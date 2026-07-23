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
#include <openssl/evp.h>
#include <array>
#include <cstring>


Define_Module(Mode4App);

// Compute HashedId8 = last 8 bytes of SHA-256 over canonical cert serialization
// per IEEE 1609.2 Section 6.3.29
static std::array<uint8_t,8> computeHashedId8(const Certificate& c) {
    std::ostringstream os;
    os << (int)c.getVersion() << '|' << (int)c.getCertType() << '|'
       << (int)c.getIssuerType() << '|' << c.getSubjectId() << '|'
       << (int)c.getAppPermPsid() << '|' << c.getAlgoName();
    for (size_t i = 0; i < c.getPublicKeyArraySize(); i++)
        os << (int)c.getPublicKey(i) << ',';
    std::string s = os.str();

    uint8_t hash[32];
    unsigned int len = 0;
    EVP_Digest(s.data(), s.size(), hash, &len, EVP_sha256(), nullptr);

    std::array<uint8_t,8> id8;
    std::memcpy(id8.data(), hash + 24, 8);  // last 8 bytes
    return id8;
}

static std::string getLogDirectory()
{
    // Get configuration name from OMNeT++ to create scenario-specific log directory
    const char* configName = cSimulation::getActiveSimulation()->getEnvir()->getConfigEx()->getActiveConfigName();
    if (configName && strlen(configName) > 0) {
        return std::string("simulation_logs_") + configName;
    }
    return "simulation_logs";
}

static void ensureSimulationLogsClean()
{
    static bool cleaned = false;
    if (cleaned) return;
    cleaned = true;

    const std::string dir = getLogDirectory();
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

        std::string algo = par("cryptoAlgo").stdstringValue();
        pqcdsa::setAlgorithm(algo);

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
        certInterval_ = par("certInterval").intValue();
        ownDigest_ = computeHashedId8(Cert);

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

        // CTAC parameters and signals
        ctacEnabled_ = par("ctacEnabled").boolValue();
        ctacCohorts_ = par("ctacCohorts").intValue();
        ctacEpoch_ = par("ctacEpoch");
        ctacAoIBound_ = par("ctacAoIBound");
        ctacCellSize_ = par("ctacCellSize").doubleValue();
        std::string ctacInactive = par("ctacInactive").stdstringValue();
        ctacCompressMode_ = (ctacInactive == "compress");
        ctacSafetyOverride_ = par("ctacSafetyOverride").boolValue();
        logV2vRx_ = par("logV2vRx").boolValue();

        bsmOpportunitySignal_ = registerSignal("bsmOpportunity");
        ctacDeferredSignal_ = registerSignal("ctacDeferred");
        ctacCompressedSignal_ = registerSignal("ctacCompressed");
        ctacOverrideAoISignal_ = registerSignal("ctacOverrideAoI");
        ctacOverrideSafetySignal_ = registerSignal("ctacOverrideSafety");
        ctrlOverheadBytesSignal_ = registerSignal("ctrlOverheadBytes");
        ctacCohortIdSignal_ = registerSignal("ctacCohortId");
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
        const std::string logDir = getLogDirectory();
        std::string path = logDir + "/ica_rx_" + std::string(hostName) + ".csv";
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
        std::string path = std::string("bsm_rx_") + hostName + ".csv";
        const double delay_ms = (simTime() - spdu->getTimestamp()).dbl() * 1000.0;

        emit(delay_, delay);
        emit(received_, long(1));

        const BSM& b = spdu->getBsm();
        std::ostringstream os;
        os << b.getMsgId() << ',' << b.getLat() << ',' << b.getLon() << ',' << b.getHeading_j() << ',' << b.getSpeed_j();
        std::string bsmHex = pqcdsa::toHex(reinterpret_cast<const uint8_t*>(os.str().data()), os.str().size());

        // Note: bsm_rx_*.csv logging is currently disabled (see commented code below line 522)
        // If re-enabled, ensure it uses SIM_LOG_DIR like other log files

        // Check SignerIdentifier type (IEEE 1609.2)
        bool ok = false;
        const Certificate* useCert = nullptr;

        if (spdu->getSignerType() == 1) {
            // signerType=certificate: use included cert and cache it
            useCert = &spdu->getCert();
            auto digest = computeHashedId8(*useCert);
            certCache_[digest] = *useCert;
        } else if (spdu->getSignerType() == 0) {
            // signerType=digest: look up in cert cache
            std::array<uint8_t,8> rxDigest;
            for (int i = 0; i < 8; i++)
                rxDigest[i] = spdu->getSignerDigest(i);
            auto it = certCache_.find(rxDigest);
            if (it != certCache_.end()) {
                useCert = &it->second;
            } else {
                EV_WARN << "RX digest SPDU but cert not in cache -- cannot verify\n";
            }
        } else {
            EV_WARN << "RX SPDU with unknown signerType=" << (int)spdu->getSignerType() << ", treating as unverified.\n";
        }

        if (useCert) {
            std::vector<uint8_t> pkBytes(useCert->getPublicKeyArraySize());
            for (size_t i = 0; i < pkBytes.size(); ++i)
                pkBytes[i] = useCert->getPublicKey(i);
            std::string rawPubHex = pqcdsa::toHex(pkBytes.data(), pkBytes.size());
            std::string pubKeyHex = pqcdsa::prefixKeyWithCertAlgo(rawPubHex, useCert->getAlgoName());
            std::vector<uint8_t> sigBytes(spdu->getSignatureArraySize());
            for (size_t i = 0; i < sigBytes.size(); ++i)
                sigBytes[i] = spdu->getSignature(i);
            std::string sigHex = pqcdsa::toHex(sigBytes.data(), sigBytes.size());

            auto verifyStart = std::chrono::high_resolution_clock::now();
            ok = pqcdsa::verify(bsmHex, sigHex, pubKeyHex);
            auto verifyEnd = std::chrono::high_resolution_clock::now();
            auto verifyUs = std::chrono::duration_cast<std::chrono::microseconds>(verifyEnd - verifyStart).count();
            emit(verifyTimeMs_, verifyUs / 1000.0);
        }
        if (ok) {
            emit(verified_, long(1));
        }
        // Recover position from fixed-point millimeters
        veins::Coord tx(b.getLat() / 1000.0, b.getLon() / 1000.0, 0.0);

        // Distance in meters
        double dist_m = rx.distance(tx);

        // ============================================================================
        // V2V Reception Logging (identical schema to RSU logging)
        // ============================================================================
        if (logV2vRx_) {
            const std::string logDir = getLogDirectory();
            const std::string v2vPath = logDir + "/v2v_logs.csv";

            int sigSize  = spdu->getSignatureArraySize();
            int spduSize = spdu->getByteLength();
            int numberOfVehicles = getNumVehicles();

            // Resolve algorithm and sender from cert (cached or included)
            bool isDigestMode = (spdu->getSignerType() == 0);
            std::string algoName = "unknown";
            std::string senderStr = "unknown";
            long pkSize = 0;
            long certMetadata = 0;
            long digestSize = 0;
            long spduOverhead = 28;

            if (useCert) {
                algoName = useCert->getAlgoName();
                senderStr = useCert->getSubjectId();
            }

            if (isDigestMode) {
                digestSize = 8;  // HashedId8
            } else if (useCert) {
                // Full cert in packet
                certMetadata = 105;  // cert fields excluding pubkey
                long derPubKeySize = useCert->getPublicKeyArraySize();
                bool isEcdsa = (algoName.find("ECDSA") != std::string::npos
                             || algoName.find("ecdsa") != std::string::npos
                             || algoName.find("P-256") != std::string::npos
                             || algoName.find("P-384") != std::string::npos);
                pkSize = isEcdsa ? (derPubKeySize - 26) : derPubKeySize;
            }

            long bsmDataSize = 1 + 4 + 4 + 2 + 4 + 4 + 2  // msgCnt+msgId+tempId+secMark+lat+lon+elev
                             + 1 + 1 + 2                    // semiMajor+semiMinor+semiMajorOrient
                             + 1 + 2 + 2 + 1                // transmission+speed_j+heading_j+angle
                             + 2 + 2 + 1 + 2                // accelLong+accelLat+accelVert+yawRate
                             + 2                             // brakes
                             + 2 + 1;                        // vehWidth+vehLength = 43 bytes

            // Note: header preserves the typo "Numer of Vehicles" for compatibility
            const std::string header =
                "t,receiver,sender,msgId,lat,lon,dist_m,delay_ms,Numer of Vehicles,verified,spdu_overhead,cert_metadata,digest_size,pk_size,sig_size,bsm_data_size,spdu_size,Algorithm,signerType";

            std::ostringstream t;   t << std::fixed << std::setprecision(6) << simTime().dbl();
            std::ostringstream lat; lat << std::fixed << std::setprecision(6) << b.getLat() / 1000.0;
            std::ostringstream lon; lon << std::fixed << std::setprecision(6) << b.getLon() / 1000.0;
            std::ostringstream dms; dms << std::fixed << std::setprecision(3) << delay_ms;
            std::ostringstream dst; dst << std::fixed << std::setprecision(3) << dist_m;

            appendCsv(v2vPath, header, {
                t.str(),
                std::string(getParentModule()->getFullName()),  // receiver = carNoIp[n]
                senderStr,
                std::to_string(b.getMsgId()),
                lat.str(),
                lon.str(),
                dst.str(),
                dms.str(),
                std::to_string(numberOfVehicles),
                (ok ? "1" : "0"),
                std::to_string(spduOverhead),
                std::to_string(certMetadata),
                std::to_string(digestSize),
                std::to_string(pkSize),
                std::to_string(sigSize),
                std::to_string(bsmDataSize),
                std::to_string(spduSize),
                algoName,
                std::to_string((int)spdu->getSignerType())
            });
        }
        // ============================================================================
        // End V2V Reception Logging
        // ============================================================================

        std::string algoName = useCert ? useCert->getAlgoName() : "unknown";
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

        EV_INFO << "RX BSM#" << b.getMsgId() << " signerType=" << (int)spdu->getSignerType() << " from " << (useCert ? useCert->getSubjectId() : "unknown") << "  -->  " << (ok ? "VALID" : "INVALID") << '\n';

        delete msg;
    }

}

void Mode4App::handleSelfMessage(cMessage* msg)
{
    // This function now ONLY handles your PQC message timer
    if (msg == sendEvt) {
        emit(bsmOpportunitySignal_, 1);          // ALWAYS, before the gate

        if (!ctacEnabled_) {
            ++bsmSeq;                            // Increment ONLY when BSM generated
            generateAndSendSPDU();               // unchanged legacy path
            lastFullTx_ = simTime();
        } else {
            switch (ctacDecide()) {
                case CTAC_FULL:
                    ++bsmSeq;                    // Increment ONLY when BSM generated
                    generateAndSendSPDU();
                    lastFullTx_ = simTime();
                    break;
                case CTAC_COMPRESS:
                    ++bsmSeq;                    // Increment for compressed BSM too
                    generateAndSendCompact();    // see implementation below
                    emit(ctacCompressedSignal_, 1);
                    break;
                case CTAC_DEFER:
                    numDeferred_++;              // Count deferred, but DON'T increment bsmSeq
                    emit(ctacDeferredSignal_, 1);
                    break;
            }
        }

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
    // IEEE 1609.2 Section 6.3.12 / SAE J2945/1: alternate full cert vs digest
    bool sendFullCert = (bsmSeq % certInterval_ == 0);

    if (sendFullCert) {
        spdu->setSignerType(1);       // certificate
        spdu->setCert(Cert);
    } else {
        spdu->setSignerType(0);       // digest (HashedId8)
        for (int i = 0; i < 8; i++)
            spdu->setSignerDigest(i, ownDigest_[i]);
    }

    spdu->setBsm(bsm);
    std::vector<uint8_t> sigBytes = pqcdsa::fromHex(sigHex);
    spdu->setSignatureArraySize(sigBytes.size());
    for (size_t i = 0; i < sigBytes.size(); ++i)
        spdu->setSignature(i, sigBytes[i]);

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
    long certOrDigestSize = sendFullCert ? certSize : 8;  // HashedId8 = 8 bytes
    long totalByteLength = spduOverhead + bsmSize + spdu->getSignatureArraySize() + certOrDigestSize;
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

// ============================================================================
// CTAC (Cooperative Transmission Authority Control) Implementation
// ============================================================================

Mode4App::CtacDecision Mode4App::ctacDecide()
{
    // 1. Safety override, highest priority
    if (ctacSafetyOverride_ && safetyEventActive()) {
        emit(ctacOverrideSafetySignal_, 1);
        return CTAC_FULL;
    }
    // 2. Freshness (age-of-information) bound
    if (simTime() - lastFullTx_ >= ctacAoIBound_) {
        emit(ctacOverrideAoISignal_, 1);
        return CTAC_FULL;
    }
    // 3. Scheduled cohort turn
    long epoch = (long)floor(simTime().dbl() / ctacEpoch_.dbl());
    if (myCohort() == (int)(epoch % ctacCohorts_))
        return CTAC_FULL;

    // 4. Otherwise defer or compress
    return ctacCompressMode_ ? CTAC_COMPRESS : CTAC_DEFER;
}

int Mode4App::myCohort()
{
    // CTAC Cohort Assignment Strategy:
    // For intersection/approach scenarios, use HEADING-BASED cohorts to ensure
    // vehicles traveling in the same direction (platoon members) are in the same cohort.
    // This is critical for safety: you need to hear BSMs from vehicles directly ahead/behind.

    // Get velocity vector from TraCI mobility
    double vx = 0.0;
    double vy = 0.0;
    bool hasVelocity = false;

    for (cModule* m = this; m; m = m->getParentModule()) {
        if (auto* mob = m->getSubmodule("veinsmobility")) {
            if (auto* tm = dynamic_cast<veins::TraCIMobility*>(mob)) {
                vx = tm->getSpeed() * cos(tm->getHeading().getRad());
                vy = tm->getSpeed() * sin(tm->getHeading().getRad());
                hasVelocity = true;
                break;
            }
        }
    }

    int cohort = 0;

    if (hasVelocity && (vx != 0.0 || vy != 0.0)) {
        // Calculate heading in degrees (0° = East, 90° = North, 180° = West, 270° = South)
        double heading_rad = atan2(vy, vx);
        double heading_deg = heading_rad * 180.0 / M_PI;
        if (heading_deg < 0) heading_deg += 360.0;

        // Map to 4 cardinal directions (for K=4)
        // This ensures all vehicles on same approach/direction are in same cohort
        if (ctacCohorts_ == 4) {
            // 315-45° = East (Cohort 0)
            // 45-135° = North (Cohort 1)
            // 135-225° = West (Cohort 2)
            // 225-315° = South (Cohort 3)
            if (heading_deg >= 315.0 || heading_deg < 45.0)
                cohort = 0;  // Eastbound
            else if (heading_deg < 135.0)
                cohort = 1;  // Northbound
            else if (heading_deg < 225.0)
                cohort = 2;  // Westbound
            else
                cohort = 3;  // Southbound
        } else {
            // For other K values, distribute evenly by heading
            int sector = (int)floor(heading_deg / (360.0 / ctacCohorts_));
            cohort = sector % ctacCohorts_;
        }
    } else {
        // Fallback for stopped vehicles: use spatial hash
        veins::Coord p = getNodePositionNow(this, simTime());
        int cx = (int)floor(p.x / ctacCellSize_);
        int cy = (int)floor(p.y / ctacCellSize_);
        unsigned int h = ((unsigned int)cx * 73856093u) ^ ((unsigned int)cy * 19349663u);
        cohort = (int)(h % (unsigned int)ctacCohorts_);
    }

    emit(ctacCohortIdSignal_, cohort);
    return cohort;
}

bool Mode4App::safetyEventActive()
{
    // TODO: Hook to hard-braking or ICA events. Stub for now.
    // Future work: integrate with vehicle state (deceleration > threshold, etc.)
    return false;
}

void Mode4App::generateAndSendCompact()
{
    // For v1: Implement as a minimal BSM with digest-only cert.
    // This is a simplified version of generateAndSendSPDU that always uses digest
    // and could optionally use a reduced payload.
    // For now, defer to the standard path with certInterval forced to non-zero
    // (digest-only) to avoid transmitting full certificate.
    //
    // TODO: Implement a truly reduced payload if needed for compress mode.
    // For this version, we simply defer (no transmission).
    // This is acceptable per spec: "If time is short, implement
    // generateAndSendCompact() as a direct call to the deferral path"

    // Do nothing - effectively defers the transmission
    // In future versions, this could send a reduced BSM
}

int Mode4App::getNumVehicles() const
{
    auto ueList = binder_->getUeList();
    if (!ueList) return 0;

    int count = 0;
    for (auto* info : *ueList) {
        if (!info) continue;
        // Skip our own node id (count only other vehicles)
        if (info->id == nodeId_) continue;
        ++count;
    }
    return count;
}

// ============================================================================
// End CTAC Implementation
// ============================================================================

void Mode4App::finish()
{
    simtime_t lifetime = simTime() - entryTime;
    emit(lifetimeSignal, lifetime);
    EV_FATAL << "LIFETIME::" << lifetime << endl;

    recordScalar("icaReceived", icaReceived_);
    recordScalar("icaExpected", icaExpected_);
    const double pdr = (icaExpected_ > 0) ? (double)icaReceived_ / (double)icaExpected_ : 0.0;
    recordScalar("icaPDR", pdr);

    // CTAC control overhead (zero by design in this version - no coordination messages)
    emit(ctrlOverheadBytesSignal_, 0);

    // Log total BSMs sent by this vehicle for ground-truth PDR calculation
    const std::string logDir = getLogDirectory();
    const std::string summaryPath = logDir + "/sender_summary.csv";
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


