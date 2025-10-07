#include "apps/mode4App/Mode4App.h"
#include "common/LteControlInfo.h"
#include "stack/phy/packet/cbr_m.h"

#include "veins/modules/mobility/traci/TraCIMobility.h"
#include "veins/base/modules/BaseMobility.h"
#include "veins/base/utils/Coord.h"
#include "apps/mode4App/IcaSpdu_m.h"
#include <chrono>
#include <fstream>
#include <iomanip>
#include <sstream>


Define_Module(Mode4App);

static inline void appendCsv(const std::string& path,
                             const std::string& header,
                             const std::vector<std::string>& cols)
{
    // Write header once if file is new/empty
    bool writeHeader = false;
    {
        std::ifstream fin(path, std::ios::in | std::ios::binary);
        writeHeader = !fin.good() || fin.peek() == std::ifstream::traits_type::eof();
    }

    std::ofstream f(path, std::ios::out | std::ios::app);
    if (!f.is_open()) return;

    if (writeHeader) f << header << '\n';

    // Simple CSV join (fields we use are numeric or hex strings, so no quoting needed)
    for (size_t i = 0; i < cols.size(); ++i) {
        if (i) f << ',';
        f << cols[i];
    }
    f << '\n';
}


static std::string icaBodyHex(const IcaWarn& w)
{
    std::ostringstream os;
    os << w.getMsgCnt() << ','
       << w.getIntersectionId() << ','
       << w.getApproach() << ','
       << w.getLane() << ','
       << w.getEventFlag() << ','
       << w.getSrcX() << ','
       << w.getSrcY() << ','
       << w.getLat() << ','
       << w.getLon() << ','
       << w.getTempId();

    return pqcdsa::toHex(reinterpret_cast<const uint8_t*>(os.str().data()), os.str().size());
}

static veins::Coord getNodePositionNow(cModule* context, simtime_t t) {
    for (cModule* m = context; m; m = m->getParentModule()) {
        if (auto* mob = m->getSubmodule("veinsmobility")) {
            if (auto* tm = dynamic_cast<veins::TraCIMobility*>(mob))
                return tm->getPositionAt(t);
            if (auto* bm = dynamic_cast<veins::BaseMobility*>(mob))
                return bm->getPositionAt(t);
        }
    }
    return veins::Coord(0,0,0); // fallback if not found
}

void Mode4App::initialize(int stage)
{
    Mode4BaseApp::initialize(stage);
    if (stage==inet::INITSTAGE_LOCAL){

        binder_ = getBinder();
        cModule *ue = getParentModule();
        nodeId_ = binder_->registerNode(ue, UE, 0);
        binder_->setMacNodeId(nodeId_, nodeId_);
    } else if (stage==inet::INITSTAGE_APPLICATION_LAYER) {

        keyPair = pqcdsa::generateKeyPair();

        std::string tag   = pqcdsa::algoTagFromKey(keyPair.pubHex);
        std::string label = pqcdsa::prettyNameFromTag(tag);
        Cert.setAlgoName(label.c_str());

        EV_FATAL << "--- PQC Key Information ---" << endl;
        EV_FATAL << "Public Key Length: " << keyPair.pubKeyLength << " bytes" << endl;
        EV_FATAL << "Public Key Length after string: " << strlen(keyPair.pubHex.c_str()) << " bytes" << endl;
        EV_FATAL << "---------------------------" << endl;
        Cert.setSubjectId(getParentModule()->getParentModule()->getFullName());

        auto pkBytes = pqcdsa::fromHex(keyPair.pubHex);
        Cert.setPublicKeyArraySize(pkBytes.size());
        for (size_t i = 0; i < pkBytes.size(); ++i)
            Cert.setPublicKey(i, pkBytes[i]);
        Cert.setNotBefore(0);
        Cert.setNotAfter(9223372036854775807LL);

        bsmSeq = 0;
        sendEvt = new cMessage("sendSPDU");
        scheduleAt(simTime() + 1, sendEvt);


        selfSender_ = NULL;
        nextSno_ = 0;

        selfSender_ = new cMessage("selfSender");

        size_ = par("packetSize");
        period_ = par("period");
        priority_ = par("priority");
        duration_ = par("duration");

        sentMsg_ = registerSignal("sentMsg");
        delay_ = registerSignal("delay");
        verified_ = registerSignal("verified");
        cbr_ = registerSignal("cbr");


        entryTime = simTime();
        lifetimeSignal = registerSignal("lifetime");
        received_ = registerSignal("received");

        warnReceived_ = registerSignal("warnReceived");
        warnVerified_     = registerSignal("warnVerified");
        warnExpected_ = registerSignal("warnExpected");
        warnPdrSample_ = registerSignal("warnPdrSample");
        warnPdrDistance_ = registerSignal("warnPdrDistance");
        rxWarnDist_ = registerSignal("rxWarnDist");
        icaVerifyMs_ = registerSignal("icaVerifyMs");
        icaDelayMs_  = registerSignal("icaDelayMs");
    }
}

void Mode4App::handleLowerMessage(cMessage* msg)
{
    if (msg->isName("CBR")) {
        Cbr* cbrPkt = check_and_cast<Cbr*>(msg);
        double channel_load = cbrPkt->getCbr();
        emit(cbr_, channel_load);
        delete cbrPkt;
    }
//    Do not delete
//    else if (auto* w = dynamic_cast<IcaWarn*>(msg)) {
//        emit(warnReceived_, 1);
//
//        veins::Coord uePos(0,0,0);
//
//        cModule* host = getParentModule();
//        if (!host) {
//            recordScalar("hostNotFound", 1);
//        } else {
//            cModule* mob = host->getSubmodule("veinsmobility");
//            if (!mob) {
//                recordScalar("mobilitySubmoduleNotFound", 1);
//            } else {
//                if (auto* tm = dynamic_cast<veins::TraCIMobility*>(mob)) {
//                    uePos = tm->getPositionAt(simTime());
//                } else if (auto* bm = dynamic_cast<veins::BaseMobility*>(mob)) {
//                    uePos = bm->getPositionAt(simTime());
//                } else {
//                    recordScalar("mobilityTypeUnexpected", 1);
//                }
//            }
//        }
//
//        if (uePos == veins::Coord(0,0,0)) {
//            recordScalar("warnPosLookupFailed", 1);
//        }
//
//        // recover RSU "position" from w->lat/w->lon (used as fixed-point XY)
//        constexpr double SCALE = 1e6; // must match RSU side
//        veins::Coord rsuPos(
//            static_cast<double>(w->getLat()) / SCALE,
//            static_cast<double>(w->getLon()) / SCALE,
//            0.0
//        );
//
//        double dMeters = uePos.distance(rsuPos);
//        emit(rxWarnDist_, dMeters);
//        lastIcaDist_ = dMeters;
//
//        const uint8_t seq = static_cast<uint8_t>(w->getMsgCnt());
//
//        if (lastIcaSeq_ < 0) {
//            // first ever ICA
//            lastIcaSeq_  = seq;
//            icaExpected_ += 1;
//            icaReceived_ += 1;
//            emit(warnPdrSample_, 1);
//            emit(warnPdrDistance_, dMeters);
//            delete w;
//            return;
//        }
//
//        const uint8_t last = static_cast<uint8_t>(lastIcaSeq_);
//        const uint8_t diff = static_cast<uint8_t>(seq - last); // modulo-256
//
//        if (diff == 0) {
//            delete w;
//            return;
//        }
//
//        if (diff <= 128) {
//            const int missed = static_cast<int>(diff) - 1;
//            icaExpected_ += static_cast<long>(diff);
//            icaReceived_ += 1;
//
//            // account for misses at last known distance
//            for (int i = 0; i < missed; ++i) {
//                emit(warnPdrSample_, 0);
//                emit(warnPdrDistance_, lastIcaDist_);
//            }
//            // account for this received one
//            emit(warnPdrSample_, 1);
//            emit(warnPdrDistance_, dMeters);
//
//            lastIcaSeq_ = seq;  // advance baseline
//        } else {
//            // old/out-of-order (<~128 behind): ignore for loss accounting
//            // (keeps PDR stable against late frames)
//        }
//
//        delete w;
//        return;
//    }
    else if (auto* s = dynamic_cast<IcaSpdu*>(msg)) {
        // 1) distance to RSU from payload’s srcX/srcY
        emit(warnReceived_, 1);
        const char* hostName = getParentModule()->getFullName();
        //std::string path = std::string("ica_rx_") + hostName + ".txt";
        std::string path = std::string("ica_rx_") + hostName + ".csv";
        const double delay_ms = (simTime() - s->getTimestamp()).dbl() * 1000.0;

        veins::Coord uePos(0,0,0);

        cModule* host = getParentModule();
        if (!host) {
            recordScalar("hostNotFound", 1);
        } else {
            cModule* mob = host->getSubmodule("veinsmobility");
            if (!mob) {
                recordScalar("mobilitySubmoduleNotFound", 1);
            } else {
                if (auto* tm = dynamic_cast<veins::TraCIMobility*>(mob)) {
                    uePos = tm->getPositionAt(simTime());
                } else if (auto* bm = dynamic_cast<veins::BaseMobility*>(mob)) {
                    uePos = bm->getPositionAt(simTime());
                } else {
                    recordScalar("mobilityTypeUnexpected", 1);
                }
            }
        }

        if (uePos == veins::Coord(0,0,0)) {
            recordScalar("warnPosLookupFailed", 1);
        }

        const IcaWarn& w = s->getWarn();

        constexpr double SCALE = 1e6;
        veins::Coord rsuPos(
            static_cast<double>(w.getLat()) / SCALE,
            static_cast<double>(w.getLon()) / SCALE,
            0.0
        );

        double dMeters = uePos.distance(rsuPos);
        emit(rxWarnDist_, dMeters);
        lastIcaDist_ = dMeters;



//        veins::Coord uePos(0,0,0);
//        if (auto* host = getParentModule()->getParentModule()) {
//            if (auto* m = host->getSubmodule("veinsmobility")) {
//                if (auto* tm = dynamic_cast<veins::TraCIMobility*>(m))
//                    uePos = tm->getPositionAt(simTime());
//                else if (auto* bm = dynamic_cast<veins::BaseMobility*>(m))
//                    uePos = bm->getPositionAt(simTime());
//            }
//        }
//        const IcaWarn& w = s->getWarn();
//        veins::Coord rsuPos(w.getSrcX(), w.getSrcY(), 0.0);
//        const double dMeters = uePos.distance(rsuPos);
//        emit(rxWarnDist_, dMeters);
//        lastIcaDist_ = dMeters;

        veins::Coord rsuPos1(w.getSrcX(), w.getSrcY(), 0.0);
        const double dMeters1 = uePos.distance(rsuPos1);

        if (dMeters1 == dMeters) {
                recordScalar("NoSameDistance", 1);
            }

        const std::string bodyHex = icaBodyHex(w);
        std::vector<uint8_t> pkBytes(s->getCert().getPublicKeyArraySize());
        for (size_t i = 0; i < pkBytes.size(); ++i) pkBytes[i] = s->getCert().getPublicKey(i);
        const std::string rawPubHex = pqcdsa::toHex(pkBytes.data(), pkBytes.size());
        std::string pubKeyHex = pqcdsa::prefixKeyWithCertAlgo(rawPubHex, s->getCert().getAlgoName());

        std::vector<uint8_t> sigBytes(s->getSignatureArraySize());
        for (size_t i = 0; i < sigBytes.size(); ++i) sigBytes[i] = s->getSignature(i);
        const std::string sigHex = pqcdsa::toHex(sigBytes.data(), sigBytes.size());

        auto start_time = std::chrono::high_resolution_clock::now();
        const bool ok = pqcdsa::verify(bodyHex, sigHex, pubKeyHex);
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration_time = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();
        emit(icaVerifyMs_, duration_time/1000.0);
        if (ok) emit(warnVerified_, 1);

        // 3) PDR accounting with 8-bit wrap (python sends msgCnt = seq % 256)
        const int seq = w.getMsgCnt() & 0xff;
        long delta = 1;  // how many packets advanced since last (mod 256)
        if (lastIcaSeq_ >= 0) {
            delta = (seq - lastIcaSeq_ + 256) % 256;
            if (delta == 0) delta = 1; // duplicate or resync -> treat as 1 step
        }
        // expected += delta, received += 1, and mark 'misses' at last known distance
        icaExpected_ += delta;
        if (delta > 1) {
            emit(warnExpected_, delta - 1);
            for (long i = 0; i < delta - 1; ++i) {
                emit(warnPdrSample_, 0);
                emit(warnPdrDistance_, lastIcaDist_);
            }
        }
        icaReceived_ += 1;
        lastIcaSeq_ = seq;

        // 4) log one “hit” sample at this distance
        emit(warnPdrSample_, ok ? 1 : 0);     // optional: only count verified hits
        emit(warnPdrDistance_, dMeters);

        // header
        const std::string header =
            "t,host,seq,intId,lane,approach,flag,srcX,srcY,lat,lon,dist_m,delay_ms,verified,tempId";

        // row values
        std::ostringstream t;       t      << std::fixed << std::setprecision(6) << simTime().dbl();
        std::ostringstream srcX;    srcX   << std::fixed << std::setprecision(6) << w.getSrcX();
        std::ostringstream srcY;    srcY   << std::fixed << std::setprecision(6) << w.getSrcY();
        std::ostringstream lat;     lat    << w.getLat();
        std::ostringstream lon;     lon    << w.getLon();
        std::ostringstream dist;    dist   << std::fixed << std::setprecision(3) << dMeters;
        std::ostringstream dms;     dms    << std::fixed << std::setprecision(3) << delay_ms;

        appendCsv(path, header, {
            t.str(),
            hostName,
            std::to_string(w.getMsgCnt() & 0xff),
            std::to_string(w.getIntersectionId()),
            std::to_string(w.getLane()),
            std::to_string(w.getApproach()),
            std::to_string(w.getEventFlag()),
            srcX.str(),
            srcY.str(),
            lat.str(),
            lon.str(),
            dist.str(),
            dms.str(),
            (ok ? "1" : "0"),
            std::string(w.getTempId())
        });

        delete s;
        return;
    }
    else {
        SPDU* spdu = dynamic_cast<SPDU*>(msg);
        if (!spdu) {
            EV << "Received a non-SPDU message, deleting." << endl;
            delete msg;
            return;
        }
        veins::Coord rx = getNodePositionNow(this, simTime());
        simtime_t delay = simTime() - spdu->getTimestamp();

        const char* hostName = getParentModule()->getFullName();
//        std::string path = std::string("bsm_rx_") + hostName + ".txt";
        std::string path = std::string("bsm_rx_") + hostName + ".csv";
        const double delay_ms = (simTime() - spdu->getTimestamp()).dbl() * 1000.0;

        emit(delay_, delay);
        emit(received_, long(1));

        const BSM& b = spdu->getBsm();
        std::ostringstream os;
        os << b.getMsgId() << ',' << b.getLatitude() << ',' << b.getLongitude() << ',' << b.getHeading() << ',' << b.getSpeed();
        std::string bsmHex = pqcdsa::toHex(reinterpret_cast<const uint8_t*>(os.str().data()), os.str().size());

        const Certificate &c = spdu->getCert();
            std::vector<uint8_t> pkBytes(c.getPublicKeyArraySize());
            for (size_t i = 0; i < pkBytes.size(); ++i)
                pkBytes[i] = c.getPublicKey(i);
            std::string rawPubHex = pqcdsa::toHex(pkBytes.data(), pkBytes.size());
            std::string pubKeyHex = pqcdsa::prefixKeyWithCertAlgo(rawPubHex, c.getAlgoName());
            std::vector<uint8_t> sigBytes(spdu->getSignatureArraySize());
                for (size_t i = 0; i < sigBytes.size(); ++i)
                    sigBytes[i] = spdu->getSignature(i);
                std::string sigHex = pqcdsa::toHex(sigBytes.data(), sigBytes.size());

        bool ok = pqcdsa::verify(bsmHex, sigHex, pubKeyHex);
        if (ok) {
            // Only count the event if the signature was valid
            emit(verified_, long(1));
        }
        veins::Coord tx(b.getLatitude(), b.getLongitude(), 0.0);

        // Distance in meters
        double dist_m = rx.distance(tx);

        std::string algoName = spdu->getCert().getAlgoName();     // 1) Algorithm name
        int certSize = spdu->getCert().getPublicKeyArraySize();   // 2) Certificate size (approximate)
        int sigSize  = spdu->getSignatureArraySize();             // 3) Signature size
        int spduSize = spdu->getByteLength();                     // 4) Total SPDU size

        const std::string header =
            "t,host,msgId,lat,lon,heading,speed,delay_ms,dist_m,verified,cert_size,sig_size,spdu_size,Algorithm";

        std::ostringstream dst; dst << std::fixed << std::setprecision(3) << dist_m;
        std::ostringstream t;   t << std::fixed << std::setprecision(6) << simTime().dbl();
        std::ostringstream lat; lat << std::fixed << std::setprecision(6) << b.getLatitude();
        std::ostringstream lon; lon << std::fixed << std::setprecision(6) << b.getLongitude();
        std::ostringstream hdg; hdg << std::fixed << std::setprecision(6) << b.getHeading();
        std::ostringstream spd; spd << std::fixed << std::setprecision(6) << b.getSpeed();
        std::ostringstream dms; dms << std::fixed << std::setprecision(3) << delay_ms;

//        appendCsv(path, header, {
//            t.str(),
//            hostName,
//            std::to_string(b.getMsgId()),
//            lat.str(),
//            lon.str(),
//            hdg.str(),
//            spd.str(),
//            dms.str(),
//            dst.str(),
//            (ok ? "1" : "0"),
//            std::to_string(certSize),
//            std::to_string(sigSize),
//            std::to_string(spduSize),
//            algoName
//        });

        EV_INFO << "RX BSM#" << b.getMsgId() << " from " << spdu->getCert().getSubjectId() << "  -->  " << (ok ? "VALID" : "INVALID") << '\n';

        delete msg;
    }

}

void Mode4App::handleSelfMessage(cMessage* msg)
{
    // This function now ONLY handles your PQC message timer
    if (msg == sendEvt) {
        generateAndSendSPDU();
        ++bsmSeq;
//        scheduleAt(simTime() + par("sendInterval").doubleValue(), sendEvt);
        scheduleAt(simTime() + 0.1, sendEvt);
    }
    else {
        // Delete any other unexpected self-messages
        delete msg;
    }
}

void Mode4App::generateAndSendSPDU()
{
    BSM bsm;
    bsm.setMsgId(bsmSeq);

//    cModule *carModule = getParentModule()->getParentModule();
//    cModule* mob = carModule->getSubmodule("veinsmobility");
//
//    if (mob == nullptr)
//        throw cRuntimeError("Mode4App::getCoord() - veinsmobility module not found in car module %s", carModule->getFullPath().c_str());
//
//    veins::TraCIMobility* veinsMobility = check_and_cast<veins::TraCIMobility*>(mob);
//
//    // Call the new getPosition() function
//    veins::Coord veinsPos = veinsMobility->getPosition();
//
//    // Get position, speed, and heading from Veins mobility
////    veins::Coord pos = mobility->getPosition();
//    bsm.setLatitude(veinsPos.x);
//    bsm.setLongitude(veinsPos.y);
//    double speed    = traciVehicle->getSpeed();
//    double angleDeg = traciVehicle->getAngle();
//    AKID_TEMP

    veins::Coord me = getNodePositionNow(this, simTime());
    bsm.setLatitude(me.x);
    bsm.setLongitude(me.y);
//    bsm.setLatitude(0.0);
//    bsm.setLongitude(0.0);
    double speed    = 0.0;
    double angleDeg = 0.0;
    double heading  = (90.0 - angleDeg) * M_PI / 180.0;
    bsm.setSpeed(speed);
    bsm.setHeading(heading);

    // Serialize BSM to hex
    std::ostringstream os;
    os << bsm.getMsgId() << ',' << bsm.getLatitude() << ',' << bsm.getLongitude() << ',' << bsm.getHeading() << ',' << bsm.getSpeed();
    std::string bsmHex = pqcdsa::toHex(reinterpret_cast<const uint8_t*>(os.str().data()), os.str().size());

    // Sign with Falcon
    std::string sigHex = pqcdsa::sign(bsmHex, keyPair.privHex);

    // Build the SPDU packet
    SPDU* spdu = new SPDU("SPDU");
    spdu->setBsm(bsm);
//    spdu->setSignatureHex(sigHex.c_str());
    std::vector<uint8_t> sigBytes = pqcdsa::fromHex(sigHex);
        spdu->setSignatureArraySize(sigBytes.size());
        for (size_t i = 0; i < sigBytes.size(); ++i)
            spdu->setSignature(i, sigBytes[i]);
    spdu->setCert(Cert);

    // 1. Estimate BSM size: 1x int32 (4 bytes) + 4x double (8 bytes each) = 36 bytes
    long bsmSize = sizeof(bsm.getMsgId()) + 4*(sizeof(bsm.getLatitude()));

    // 2. Calculate Certificate size from its string fields and integers (8 bytes each)
    long certSize = strlen(Cert.getSubjectId()) +
                    strlen(Cert.getAlgoName()) +
                    Cert.getPublicKeyArraySize() +
                    sizeof(Cert.getNotBefore()) +
                    sizeof(Cert.getNotAfter());


    long totalByteLength = bsmSize + spdu->getSignatureArraySize() + certSize;
    spdu->setByteLength(totalByteLength);

    EV_FATAL << "signature size : "<< spdu->getSignatureArraySize() <<endl;

    EV_FATAL << "DEBUG TEST: BSM size " << bsmSize << " bytes and Certificate size is "
            << certSize <<" bytes and public key size is "<< Cert.getPublicKeyArraySize() <<endl;

    EV_FATAL << "DEBUG TEST: fixed size is " << size_ << " bytes and calculated size is " << totalByteLength <<" bytes"<<endl;

    EV_FATAL << "DEBUG TEST: calculating the SPDU size " << spdu->getByteLength() << " bytes." << endl;


    // --- C-V2X MODE 4 SENDING LOGIC ---
    auto lteControlInfo = new FlowControlInfoNonIp();
    lteControlInfo->setDirection(D2D_MULTI);
    lteControlInfo->setLcid(5); // Use integer value for DTCH
    lteControlInfo->setPriority(1);
    lteControlInfo->setCreationTime(simTime());
    lteControlInfo->setSrcAddr(nodeId_);
    lteControlInfo->setDuration(duration_);
    spdu->setControlInfo(lteControlInfo);
    spdu->setTimestamp(simTime());
    Mode4BaseApp::sendLowerPackets(spdu);


    lteControlInfo->setSrcAddr(nodeId_);
    //        lteControlInfo->setDuration(duration_);
    // --- END C-V2X SENDING LOGIC ---

    EV_INFO << "TX BSM#" << bsmSeq << "  speed=" << speed << "  sig=" << sigHex.substr(0,12) << "...\n";
    emit(sentMsg_, (long)1);
}

void Mode4App::finish()
{
    simtime_t lifetime = simTime() - entryTime;
    emit(lifetimeSignal, lifetime);
    EV_FATAL << "LIFETIME::" << lifetime << endl;

    recordScalar("icaReceived", icaReceived_);
    recordScalar("icaExpected", icaExpected_);
    const double pdr = (icaExpected_ > 0) ? (double)icaReceived_ / (double)icaExpected_ : 0.0;
    recordScalar("icaPDR", pdr);
    cancelAndDelete(sendEvt);
}

Mode4App::~Mode4App()
{

    binder_->unregisterNode(nodeId_);

}


