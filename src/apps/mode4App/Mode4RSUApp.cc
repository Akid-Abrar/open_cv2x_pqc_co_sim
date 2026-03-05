#include "apps/mode4App/Mode4RSUApp.h"
#include "common/LteControlInfo.h"
#include "stack/phy/packet/cbr_m.h"
#include <sstream>
#include "apps/mode4App/IcaWarn_m.h"          // the new message
#include <nlohmann/json.hpp>
using nlohmann::json;
#include <chrono>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <dirent.h>
#include <sys/stat.h>

#include "veins/modules/mobility/traci/TraCIMobility.h"
#include "veins/base/modules/BaseMobility.h"
#include "veins/base/utils/Coord.h"
#include "apps/mode4App/IcaSpdu_m.h"

Define_Module(Mode4RSUApp);
//using namespace lte::apps::mode4App;



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

int Mode4RSUApp::getNumVehicles() const
{
    auto ueList = binder_->getUeList();
    if (!ueList) return 0;

    int count = 0;
    for (auto* info : *ueList) {
        if (!info) continue;
        // RSU itself is also registered as a UE, so skip our own id
        if (info->id == nodeId_) continue;
        ++count;
    }
    return count;
}

// helper: JSON -> IcaWarn
static IcaWarn* makeIcaWarnFromJson(const json& j)
{
    auto* w = new IcaWarn("IcaWarn");
    w->setMsgCnt( j.value("msgCnt", 0) );

    if (j.contains("id") && j["id"].is_string())
    {
        const std::string idHex = j["id"].get<std::string>();
        w->setTempId(idHex.c_str());
    }

    const auto& iid = j["intersectionID"];
    w->setIntersectionId( iid.value("id", 0) );

    const auto& ln = j["laneNumber"];
    std::string choice = ln.value("choice", "lane");
    int val = ln.value("value", -1);
    if (choice == "approach") { w->setApproach(val); w->setLane(-1); }
    else                      { w->setLane(val);     w->setApproach(-1); }

    w->setEventFlag( j.value("eventFlag", 0) );

    if (j.contains("partOne")) {
        const auto& p1 = j["partOne"];
        if (p1.contains("lat")) w->setLat( p1["lat"].get<long long>() );
        if (p1.contains("lon")) w->setLon( p1["lon"].get<long long>() );
    }

    w->setGenTime(simTime());
    return w;
}

// Canonical hex body for ICA signing/verifying (RSU and UE must match 1:1)
static std::string icaBodyHex(const IcaWarn& w)
{
    std::ostringstream os;
    os << w.getMsgCnt() << ','
       << w.getIntersectionId() << ','
       << w.getApproach() << ','
       << w.getLane() << ','
       << w.getEventFlag() << ','
       << w.getSrcX() << ','        // OMNeT world X
       << w.getSrcY() << ','        // OMNeT world Y
       << w.getLat() << ','         // optional coarse position (kept as is)
       << w.getLon() << ','
       << w.getTempId();            // hex string for TID

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

void Mode4RSUApp::openNonBlockingUdp_(int port)
{
    sockFd_ = ::socket(AF_INET, SOCK_DGRAM, 0);
    if (sockFd_ < 0) throw cRuntimeError("RSU: socket() failed: %s", strerror(errno));

    int reuse = 1;
    ::setsockopt(sockFd_, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    memset(&sockAddr_, 0, sizeof(sockAddr_));
    sockAddr_.sin_family = AF_INET;
    sockAddr_.sin_addr.s_addr = htonl(INADDR_ANY);   // listen on 0.0.0.0
    sockAddr_.sin_port = htons(port);

    if (::bind(sockFd_, (sockaddr*)&sockAddr_, sizeof(sockAddr_)) < 0)
        throw cRuntimeError("RSU: bind(%d) failed: %s", port, strerror(errno));

    // non-blocking
    int flags = fcntl(sockFd_, F_GETFL, 0);
    fcntl(sockFd_, F_SETFL, flags | O_NONBLOCK);
}

static void cleanSimulationLogs()
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
                if (ent->d_name[0] == '.') continue;  // skip . and ..
                std::string fpath = dir + "/" + ent->d_name;
                ::remove(fpath.c_str());
            }
            ::closedir(d);
        }
    } else {
        ::mkdir(dir.c_str(), 0755);
    }
}

void Mode4RSUApp::initialize(int stage)
{
    Mode4BaseApp::initialize(stage);
    if (stage==inet::INITSTAGE_LOCAL){
        cleanSimulationLogs();

        binder_ = getBinder();
        cModule *ue = getParentModule();
        nodeId_ = binder_->registerNode(ue, UE, 0);
        binder_->setMacNodeId(nodeId_, nodeId_);

        // after your existing signal registrations…
        keyPair_ = pqcdsa::generateKeyPair();
        std::string tag   = pqcdsa::algoTagFromKey(keyPair_.pubHex);
        std::string label = pqcdsa::prettyNameFromTag(tag);
        cert_.setAlgoName(label.c_str());
        EV_FATAL << "Public Key Length: " << keyPair_.pubKeyLength << " bytes" << endl;
        auto pkBytes = pqcdsa::fromHex(keyPair_.pubHex);

        cert_.setSubjectId(getParentModule()->getParentModule()->getFullName());
        cert_.setAlgoName(label.c_str());   // use auto-detected algo, not hardcoded
        cert_.setPublicKeyArraySize(pkBytes.size());
        for (size_t i = 0; i < pkBytes.size(); ++i) cert_.setPublicKey(i, pkBytes[i]);
        cert_.setVersion(3);
        cert_.setCertType(0);           // explicit
        cert_.setIssuerType(1);         // self-signed
        cert_.setAppPermPsid(0x40);     // ICA PSID
        cert_.setValidityStart(0);
        cert_.setValidityDuration(9223372036854775807LL);


        // ICA socket reading disabled — not using ICA right now
        // int port = par("socketPort");
        // openNonBlockingUdp_(port);
        // sockPollEvt_ = new cMessage("sockPoll");
        // scheduleAt(simTime() + par("socketPollInterval"), sockPollEvt_);

    } else if (stage == inet::INITSTAGE_APPLICATION_LAYER) {
        EV_INFO << "RSU Application started, listening for signed BSMs...\n";
        rsuReceivedMsg = registerSignal("rsuReceivedMsg");
        rsuVerifiedMsg = registerSignal("rsuVerifiedMsg");
        cbr_ = registerSignal("cbr");
        numBroadcasted = registerSignal("numBroadcasted");
        icaSignMs = registerSignal("icaSignMs");
        verifyTimeMs_ = registerSignal("verifyTimeMs");

    }
}

// Do not delete. Need this for simple payload
//void Mode4RSUApp::broadcastIca(IcaWarn* w)
//{
//    // 1) read RSU position from its BaseMobility submodule
//    auto* mobMod = getParentModule()->getSubmodule("veinsmobility");
//    veins::BaseMobility* bm = mobMod ? check_and_cast<veins::BaseMobility*>(mobMod) : nullptr;
//    if (bm) {
//        const veins::Coord pos = bm->getPositionAt(simTime());   // <-- FIX
//        constexpr double SCALE = 1e6; // fixed-point scale to reuse int64 lat/lon
//        w->setLat(static_cast<long long>(std::llround(pos.x * SCALE)));
//        w->setLon(static_cast<long long>(std::llround(pos.y * SCALE)));
//    } else {
//        w->setLat(0);
//        w->setLon(0);
//    }
//
//    // 2) attach sidelink control and send
//    auto ci = new FlowControlInfoNonIp();
//    ci->setDirection(D2D_MULTI);
//    ci->setPriority(par("slPriority"));
//    ci->setLcid(par("slLcid"));
//    ci->setDuration(par("slDurationMs").intValue());
//    ci->setCreationTime(simTime());
//    ci->setSrcAddr(nodeId_);
//
//    w->setControlInfo(ci);
//    w->setTimestamp(simTime());
//    w->setByteLength(64);
//
//    Mode4BaseApp::sendLowerPackets(w);   // takes ownership
//    emit(numBroadcasted, 1);
//}

void Mode4RSUApp::broadcastIca(IcaWarn* w)
{
    // 1) stamp RSU position into the payload (srcX/srcY in OMNeT coords)
    veins::Coord pos(0,0,0);
    if (auto* mobMod = getParentModule()->getSubmodule("veinsmobility")) {
        if (auto* bm = dynamic_cast<veins::BaseMobility*>(mobMod))
            pos = bm->getPositionAt(simTime());
    }
    w->setSrcX(pos.x);
    w->setSrcY(pos.y);

    // 2) build canonical body and sign with RSU’s private key
    const std::string bodyHex = icaBodyHex(*w);
    auto start_time = std::chrono::high_resolution_clock::now();
    const std::string sigHex  = pqcdsa::sign(bodyHex, keyPair_.privHex);
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration_time = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();
    emit(icaSignMs, duration_time/1000.0);

    // 3) assemble IcaSpdu (IEEE 1609.2 Ieee1609Dot2Data + payload + signature + cert)
    auto* spdu = new IcaSpdu("ICA_SPDU");
    spdu->setProtocolVersion(3);
    spdu->setPsid(0x40);               // ICA PSID
    spdu->setGenerationTime((int64_t)(simTime().dbl() * 1e6));
    spdu->setGenLocation_lat((int32_t)(pos.x * 1000));
    spdu->setGenLocation_lon((int32_t)(pos.y * 1000));
    spdu->setSignerType(1);            // certificate included
    spdu->setWarn(*w);                 // deep copy of the warn payload
    spdu->setCert(cert_);

    auto sigBytes = pqcdsa::fromHex(sigHex);
    spdu->setSignatureArraySize(sigBytes.size());
    for (size_t i = 0; i < sigBytes.size(); ++i)
        spdu->setSignature(i, sigBytes[i]);

    // 4) attach sidelink flow control
    auto ci = new FlowControlInfoNonIp();
    ci->setDirection(D2D_MULTI);
    ci->setPriority(par("slPriority"));
    ci->setLcid(par("slLcid"));
    ci->setDuration(par("slDurationMs").intValue());
    ci->setCreationTime(simTime());
    ci->setSrcAddr(nodeId_);
    spdu->setControlInfo(ci);

    spdu->setTimestamp(simTime());
    // 1609.2 wrapper (28) + IcaWarn payload (64) + cert overhead (105)
    long icaWrapperAndPayload = 28 + 64 + 105;
    long icaDerPkSize = spdu->getCert().getPublicKeyArraySize();
    std::string icaAlgo(spdu->getCert().getAlgoName());
    bool icaIsEcdsa = (icaAlgo.find("ECDSA") != std::string::npos
                    || icaAlgo.find("ecdsa") != std::string::npos
                    || icaAlgo.find("P-256") != std::string::npos
                    || icaAlgo.find("P-384") != std::string::npos);
    long icaRawPkSize = icaIsEcdsa ? (icaDerPkSize - 26) : icaDerPkSize;
    spdu->setByteLength(icaWrapperAndPayload + spdu->getSignatureArraySize() + icaRawPkSize);

    // 5) send
    Mode4BaseApp::sendLowerPackets(spdu);
    emit(numBroadcasted, long(1));

    delete w; // we copied it into spdu
}

void Mode4RSUApp::socketRead()
{
    if (sockFd_ < 0) return;

    for (;;) {
        char buf[2048];
        sockaddr_in src{};
        socklen_t slen = sizeof(src);
        ssize_t n = ::recvfrom(sockFd_, buf, sizeof(buf), 0, (sockaddr*)&src, &slen);
        if (n <= 0) {
            if (errno == EWOULDBLOCK || errno == EAGAIN || errno == EINTR) break;
            break;
        }

        try {
            auto j = json::parse(buf, buf + n);
            auto* w = makeIcaWarnFromJson(j);
            broadcastIca(w);              // takes ownership
        }
        catch (...) {
            EV_WARN << "[RSU] received malformed ICA JSON\n";
        }
    }
}

void Mode4RSUApp::handleSelfMessage(cMessage* msg)
{
    // ICA socket reading disabled
    // if (msg == sockPollEvt_) {
    //     socketRead();
    //     scheduleAt(simTime() + par("socketPollInterval"), sockPollEvt_);
    //     return;
    // }
    // RSU has no other timers
    delete msg;
}

void Mode4RSUApp::handleLowerMessage(cMessage* msg)
{
    if (msg->isName("CBR")) {
        Cbr* cbrPkt = check_and_cast<Cbr*>(msg);
        double channel_load = cbrPkt->getCbr();
        emit(cbr_, channel_load);
        delete cbrPkt;
        return;
    }

    emit(rsuReceivedMsg, 1);
    const char* rsuName = getParentModule()->getFullName();     // rsu[0]
//    std::string path = std::string("bsm_rx_") + rsuName + ".txt";
    std::string path = std::string("simulation_logs/appl_logs") + ".csv";

    SPDU* spdu = dynamic_cast<SPDU*>(msg);
    if (!spdu) {
        EV_DEBUG << "RSU received a non-SPDU message '" << msg->getClassName() << "', dropping.\n";
        delete msg;
        return;
    }
    const double delay_ms = (simTime() - spdu->getTimestamp()).dbl() * 1000.0;

    EV<< "CRITICAL TEST: Received Timestamp: "<< simTime().dbl() * 1000.0 << endl;
    // RSU receiver position (meters)
    veins::Coord rsu = getNodePositionNow(this, simTime());

    // Transmitter position recovered from J2735 integer fields
    const BSM& b = spdu->getBsm();
    veins::Coord tx(b.getLat() / 1000.0, b.getLon() / 1000.0, 0.0);

    // Distance in meters
    double dist_m = rsu.distance(tx);

    // Verification (must match sender's serialization exactly)
    std::ostringstream os;
    os << b.getMsgId() << ',' << b.getLat() << ',' << b.getLon() << ',' << b.getHeading_j() << ',' << b.getSpeed_j();
    std::string bsmHex = pqcdsa::toHex(reinterpret_cast<const uint8_t*>(os.str().data()), os.str().size());

    const Certificate &c = spdu->getCert();
    std::vector<uint8_t> pkBytes(c.getPublicKeyArraySize());
    for (size_t i = 0; i < pkBytes.size(); ++i) pkBytes[i] = c.getPublicKey(i);
    std::string rawPubHex = pqcdsa::toHex(pkBytes.data(), pkBytes.size());
    std::string pubKeyHex = pqcdsa::prefixKeyWithCertAlgo(rawPubHex, c.getAlgoName());

    std::vector<uint8_t> sigBytes(spdu->getSignatureArraySize());
    for (size_t i = 0; i < sigBytes.size(); ++i) sigBytes[i] = spdu->getSignature(i);
    std::string sigHex = pqcdsa::toHex(sigBytes.data(), sigBytes.size());

    auto verifyStart = std::chrono::high_resolution_clock::now();
    bool ok = pqcdsa::verify(bsmHex, sigHex, pubKeyHex);
    auto verifyEnd = std::chrono::high_resolution_clock::now();
    auto verifyUs = std::chrono::duration_cast<std::chrono::microseconds>(verifyEnd - verifyStart).count();
    emit(verifyTimeMs_, verifyUs / 1000.0);
    if (ok) emit(rsuVerifiedMsg, 1);

    EV_INFO << "RSU RX BSM#" << b.getMsgId()
            << " from " << spdu->getCert().getSubjectId()
            << "  -->  Verification: " << (ok ? "VALID" : "INVALID") << '\n';

    int sigSize  = spdu->getSignatureArraySize();
    int spduSize = spdu->getByteLength();
    int numberOfVehicles = getNumVehicles();

    // SPDU size breakdown: pk_size + sig_size + bsm_data_size + metadata_size = spdu_size
    long derPubKeySize = spdu->getCert().getPublicKeyArraySize();
    std::string algo(spdu->getCert().getAlgoName());
    bool isEcdsa = (algo.find("ECDSA") != std::string::npos
                 || algo.find("ecdsa") != std::string::npos
                 || algo.find("P-256") != std::string::npos
                 || algo.find("P-384") != std::string::npos);
    long pkSize = isEcdsa ? (derPubKeySize - 26) : derPubKeySize;
    long bsmDataSize = 1 + 4 + 4 + 2 + 4 + 4 + 2  // msgCnt+msgId+tempId+secMark+lat+lon+elev
                     + 1 + 1 + 2                    // semiMajor+semiMinor+semiMajorOrient
                     + 1 + 2 + 2 + 1                // transmission+speed_j+heading_j+angle
                     + 2 + 2 + 1 + 2                // accelLong+accelLat+accelVert+yawRate
                     + 2                             // brakes
                     + 2 + 1;                        // vehWidth+vehLength = 43 bytes
    long metadataSize = spduSize - pkSize - sigSize - bsmDataSize;

    const std::string header =
            "t,receiver,sender,msgId,lat,lon,dist_m,delay_ms,Numer of Vehicles,verified,pk_size,sig_size,bsm_data_size,metadata_size,spdu_size,Algorithm";

    std::string algoName = spdu->getCert().getAlgoName();
    std::ostringstream t;   t << std::fixed << std::setprecision(6) << simTime().dbl();
    std::ostringstream lat; lat << std::fixed << std::setprecision(6) << b.getLat() / 1000.0;
    std::ostringstream lon; lon << std::fixed << std::setprecision(6) << b.getLon() / 1000.0;
    std::ostringstream hdg; hdg << std::fixed << std::setprecision(4) << b.getHeading_j() * 0.0125;
    std::ostringstream spd; spd << std::fixed << std::setprecision(4) << b.getSpeed_j() * 0.02;
    std::ostringstream dms; dms << std::fixed << std::setprecision(3) << delay_ms;
    std::ostringstream dst; dst << std::fixed << std::setprecision(3) << dist_m;
    std::ostringstream senderName; senderName << spdu->getCert().getSubjectId();

    appendCsv(path, header, {
            t.str(),
            rsuName,
            senderName.str(),
            std::to_string(b.getMsgId()),
            lat.str(),
            lon.str(),
            //hdg.str(),
            //spd.str(),
            dst.str(),
            dms.str(),
            std::to_string(numberOfVehicles),
            (ok ? "1" : "0"),
            std::to_string(pkSize),
            std::to_string(sigSize),
            std::to_string(bsmDataSize),
            std::to_string(metadataSize),
            std::to_string(spduSize),
            algoName
        });

    delete spdu;
}

void Mode4RSUApp::finish()
{
    simtime_t endtime = simTime();
}

Mode4RSUApp::~Mode4RSUApp()
{
    // ICA socket reading disabled
    // if (sockPollEvt_) { cancelAndDelete(sockPollEvt_); sockPollEvt_ = nullptr; }
    // if (sockFd_ >= 0) { ::close(sockFd_); sockFd_ = -1; }
    binder_->unregisterNode(nodeId_);

}
