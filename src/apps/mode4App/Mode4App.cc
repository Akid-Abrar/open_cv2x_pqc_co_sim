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
#include <dirent.h>
#include <sys/stat.h>


Define_Module(Mode4App);

static void ensureSimulationLogsClean()
{
    static bool cleaned = false;
    if (cleaned) return;
    cleaned = true;

    const std::string dir = "simulation_logs";
    struct stat st;
    if (::stat(dir.c_str(), &st) == 0 && S_ISDIR(st.st_mode)) {
        DIR* d = ::opendir(dir.c_str());
        if (d) {
            struct dirent* ent;
            while ((ent = ::readdir(d)) != nullptr) {
                if (ent->d_name[0] == '.') continue;
                std::string fpath = dir + "/" + ent->d_name;
                ::remove(fpath.c_str());
            }
            ::closedir(d);
        }
    } else {
        ::mkdir(dir.c_str(), 0755);
    }
}

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
        ensureSimulationLogsClean();

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
        Cert.setSubjectId(getParentModule()->getFullName());

        auto pkBytes = pqcdsa::fromHex(keyPair.pubHex);
        Cert.setPublicKeyArraySize(pkBytes.size());
        for (size_t i = 0; i < pkBytes.size(); ++i)
            Cert.setPublicKey(i, pkBytes[i]);
        Cert.setVersion(3);
        Cert.setCertType(0);           // explicit
        Cert.setIssuerType(1);         // self-signed
        Cert.setAppPermPsid(0x20);     // BSM
        Cert.setValidityStart(0);
        Cert.setValidityDuration(9223372036854775807LL);

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

        signatureTimeMs_  = registerSignal("signatureTimeMs");
        verifyTimeMs_      = registerSignal("verifyTimeMs");
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
        os << b.getMsgId() << ',' << b.getLat() << ',' << b.getLon() << ',' << b.getHeading_j() << ',' << b.getSpeed_j();
        std::string bsmHex = pqcdsa::toHex(reinterpret_cast<const uint8_t*>(os.str().data()), os.str().size());

        // Check SignerIdentifier type (IEEE 1609.2)
        bool ok = false;
        if (spdu->getSignerType() == 1) {
            // signerType=certificate: verify using included cert (current behavior)
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

            auto verifyStart = std::chrono::high_resolution_clock::now();
            ok = pqcdsa::verify(bsmHex, sigHex, pubKeyHex);
            auto verifyEnd = std::chrono::high_resolution_clock::now();
            auto verifyUs = std::chrono::duration_cast<std::chrono::microseconds>(verifyEnd - verifyStart).count();
            emit(verifyTimeMs_, verifyUs / 1000.0);
        } else if (spdu->getSignerType() == 0) {
            // signerType=digest: would need cached cert lookup (future)
            EV_WARN << "RX SPDU with signerType=digest (HashedId8) — cert lookup not yet implemented, treating as unverified.\n";
        } else {
            EV_WARN << "RX SPDU with unknown signerType=" << (int)spdu->getSignerType() << ", treating as unverified.\n";
        }
        if (ok) {
            emit(verified_, long(1));
        }
        // Recover position from fixed-point millimeters
        veins::Coord tx(b.getLat() / 1000.0, b.getLon() / 1000.0, 0.0);

        // Distance in meters
        double dist_m = rx.distance(tx);

        std::string algoName = spdu->getCert().getAlgoName();
        int sigSize  = spdu->getSignatureArraySize();
        int spduSize = spdu->getByteLength();

        const std::string header =
            "t,host,msgId,lat,lon,heading,speed,delay_ms,dist_m,verified,sig_size,spdu_size,Algorithm";

        std::ostringstream dst; dst << std::fixed << std::setprecision(3) << dist_m;
        std::ostringstream t;   t << std::fixed << std::setprecision(6) << simTime().dbl();
        std::ostringstream lat; lat << std::fixed << std::setprecision(6) << b.getLat() / 1000.0;
        std::ostringstream lon; lon << std::fixed << std::setprecision(6) << b.getLon() / 1000.0;
        std::ostringstream hdg; hdg << std::fixed << std::setprecision(4) << b.getHeading_j() * 0.0125;
        std::ostringstream spd; spd << std::fixed << std::setprecision(4) << b.getSpeed_j() * 0.02;
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

    // J2735 BSMcoreData fields
    bsm.setMsgCnt(bsmSeq % 128);
    {
        std::ostringstream tid;
        tid << std::hex << std::setfill('0') << std::setw(8) << (uint32_t)nodeId_;
        bsm.setTempId(tid.str().c_str());
    }
    bsm.setSecMark((uint16_t)((int64_t)(simTime().dbl() * 1000) % 60000));

    veins::Coord me = getNodePositionNow(this, simTime());
    // Store OMNeT++ coords as fixed-point millimeters (not 1e7 — that overflows
    // int32_t for simulation coords in meters, e.g. 500m * 1e7 > INT32_MAX)
    bsm.setLat((int32_t)(me.x * 1000));
    bsm.setLon((int32_t)(me.y * 1000));

    // Fetch speed and heading from TraCI mobility
    double speed   = 0.0;
    double heading = 0.0;
    for (cModule* m = this; m; m = m->getParentModule()) {
        if (auto* mob = m->getSubmodule("veinsmobility")) {
            if (auto* tm = dynamic_cast<veins::TraCIMobility*>(mob)) {
                speed   = tm->getSpeed();
                heading = tm->getHeading().getRad();
                break;
            }
        }
    }
    bsm.setSpeed_j((uint16_t)(speed / 0.02));
    bsm.setHeading_j((uint16_t)(heading / 0.0125));

    // Serialize BSM to hex (using J2735 integer fields)
    std::ostringstream os;
    os << bsm.getMsgId() << ',' << bsm.getLat() << ',' << bsm.getLon() << ',' << bsm.getHeading_j() << ',' << bsm.getSpeed_j();
    std::string bsmHex = pqcdsa::toHex(reinterpret_cast<const uint8_t*>(os.str().data()), os.str().size());

    // Sign with Falcon
    auto  sigStart = std::chrono::high_resolution_clock::now();
    std::string sigHex = pqcdsa::sign(bsmHex, keyPair.privHex);
    auto  sigEnd = std::chrono::high_resolution_clock::now();
    auto sigTime = std::chrono::duration_cast<std::chrono::microseconds>(sigEnd - sigStart).count();
    emit(signatureTimeMs_, sigTime / 1000.0);

//    auto start_time = std::chrono::high_resolution_clock::now();
//    const bool ok = pqcdsa::verify(bodyHex, sigHex, pubKeyHex);
//    auto end_time = std::chrono::high_resolution_clock::now();
//    auto duration_time = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();
//    emit(icaVerifyMs_, duration_time/1000.0);


    // Build the SPDU packet (IEEE 1609.2 Ieee1609Dot2Data)
    SPDU* spdu = new SPDU("SPDU");
    spdu->setProtocolVersion(3);
    spdu->setPsid(0x20);
    spdu->setGenerationTime((int64_t)(simTime().dbl() * 1e6));
    spdu->setGenLocation_lat(bsm.getLat());   // already in mm fixed-point
    spdu->setGenLocation_lon(bsm.getLon());
    spdu->setSignerType(1);            // certificate included
    spdu->setBsm(bsm);
    std::vector<uint8_t> sigBytes = pqcdsa::fromHex(sigHex);
    spdu->setSignatureArraySize(sigBytes.size());
    for (size_t i = 0; i < sigBytes.size(); ++i)
        spdu->setSignature(i, sigBytes[i]);
    spdu->setCert(Cert);

    // Wire-level byte length per IEEE 1609.2 / SAE J2735.
    //
    // 1. BSM: J2735 BSMcoreData Part I — fully compliant (39 bytes standard)
    //    + msgId (4 bytes, simulation-only) = 43 bytes total
    long bsmSize = sizeof(bsm.getMsgCnt())          // msgCnt:          1
                 + sizeof(bsm.getMsgId())            // msgId:            4 (sim-only)
                 + 4                                  // tempId:           4 octets (wire)
                 + sizeof(bsm.getSecMark())          // secMark:          2
                 + sizeof(bsm.getLat())              // lat:              4
                 + sizeof(bsm.getLon())              // lon:              4
                 + sizeof(bsm.getElev())             // elev:             2
                 + sizeof(bsm.getSemiMajor())        // semiMajor:        1 (was uint16)
                 + sizeof(bsm.getSemiMinor())        // semiMinor:        1 (was uint16)
                 + sizeof(bsm.getSemiMajorOrient())  // semiMajorOrient:  2
                 + sizeof(bsm.getTransmission())     // transmission:     1
                 + sizeof(bsm.getSpeed_j())          // speed_j:          2
                 + sizeof(bsm.getHeading_j())        // heading_j:        2
                 + sizeof(bsm.getAngle())            // angle:            1
                 + sizeof(bsm.getAccelLong())        // accelLong:        2
                 + sizeof(bsm.getAccelLat())         // accelLat:         2
                 + sizeof(bsm.getAccelVert())        // accelVert:        1
                 + sizeof(bsm.getYawRate())          // yawRate:          2
                 + sizeof(bsm.getBrakes())           // brakes:           2
                 + sizeof(bsm.getVehWidth())         // vehWidth:         2
                 + sizeof(bsm.getVehLength());       // vehLength:        1
                                                      // Total:           43 (39 std + 4 sim)

    // 2. Certificate: 1609.2 explicit cert
    //    version(1) + type(1) + issuer(9) + toBeSigned[cracaId(3) + crlSeries(2)
    //    + validityPeriod(8) + appPermissions(~12) + verifyKeyIndicator(~2)]
    //    + CA signature on cert (same algo): included in certOverhead
    //    certOverhead covers all cert fields EXCEPT the raw verification key.
    //    ECDSA: certOverhead(105) + pubkey(65) = 170B (matches literature ~170B)
    //    PQC:   certOverhead(105) + pubkey(actual raw size from algorithm)
    //
    //    Internal storage uses DER (91B for ECDSA) but wire uses raw point (65B).
    //    For PQC, internal and wire sizes are the same (no DER wrapper).
    long certOverhead  = 105;
    long derPubKeySize = Cert.getPublicKeyArraySize();  // internal storage size
    // ECDSA keys are stored internally as DER SubjectPublicKeyInfo (91B for P-256),
    // but the 1609.2 wire format uses the raw uncompressed EC point (65B).
    // PQC keys (Falcon, Dilithium) have no DER wrapper — internal = wire size.
    std::string algo(Cert.getAlgoName());
    bool isEcdsa = (algo.find("ECDSA") != std::string::npos
                 || algo.find("ecdsa") != std::string::npos
                 || algo.find("P-256") != std::string::npos
                 || algo.find("P-384") != std::string::npos);
    long rawPubKeySize = isEcdsa ? (derPubKeySize - 26) : derPubKeySize;
    long certSize = certOverhead + rawPubKeySize;

    // 3. SPDU 1609.2 Ieee1609Dot2Data wrapper + HeaderInfo
    //    protocolVersion(1) + contentType(1) + hashAlgorithm(1) + psid(~3)
    //    + generationTime(8) + signerIdentifier(1) + encoding overhead(~13)
    long spduOverhead = 28;
    long totalByteLength = spduOverhead + bsmSize + spdu->getSignatureArraySize() + certSize;
    spdu->setByteLength(totalByteLength);

    EV_FATAL << "CRITICAL TEST: signature size : "<< spdu->getSignatureArraySize() <<endl;

    EV_FATAL << "CRITICAL TEST: BSM size " << bsmSize << " bytes and Certificate size is "
            << certSize <<" bytes and public key size is "<< Cert.getPublicKeyArraySize() <<endl;

    EV_FATAL << "CRITICAL TEST: fixed size is " << size_ << " bytes and calculated size is " << totalByteLength <<" bytes"<<endl;

    EV_FATAL << "CRITICAL TEST: calculating the SPDU size " << spdu->getByteLength() << " bytes." << endl;


    // --- C-V2X MODE 4 SENDING LOGIC ---
    auto lteControlInfo = new FlowControlInfoNonIp();
    lteControlInfo->setDirection(D2D_MULTI);
    lteControlInfo->setLcid(5); // Use integer value for DTCH
    lteControlInfo->setPriority(2);
    lteControlInfo->setCreationTime(simTime());
    lteControlInfo->setSrcAddr(nodeId_);
    lteControlInfo->setDuration(duration_);
    spdu->setControlInfo(lteControlInfo);
    spdu->setTimestamp(simTime());
    EV_FATAL << "CRITICAL TEST: Time of Creating the SPDU " << spdu->getTimestamp().dbl() * 1000.0 << endl;
    Mode4BaseApp::sendLowerPackets(spdu);

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

    // Log total BSMs sent by this vehicle for ground-truth PDR calculation
    const std::string summaryPath = "simulation_logs/sender_summary.csv";
    const std::string summaryHeader = "sender,total_sent";
    appendCsv(summaryPath, summaryHeader, {
        std::string(getParentModule()->getFullName()),
        std::to_string(bsmSeq)
    });

    cancelAndDelete(sendEvt);
}

Mode4App::~Mode4App()
{

    binder_->unregisterNode(nodeId_);

}


