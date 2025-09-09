//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

/**
 * Mode4App is a new application developed to be used with Mode 4 based simulation
 * Author: Brian McCarthy
 * Email: b.mccarthy@cs.ucc.ie
 */

#include "apps/mode4App/Mode4App.h"
#include "common/LteControlInfo.h"
#include "stack/phy/packet/cbr_m.h"



//#include "stack/phy/layer/LtePhyBase.h"
//#include "common/LteCommon.h"
//#include "veins/modules/mobility/traci/TraCIMobility.h"
//#include "veins/base/modules/BaseMobility.h"


Define_Module(Mode4App);

//using namespace lte::apps::mode4App;

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

        // Now, print the lengths using EV_INFO
        EV_FATAL << "--- PQC Key Information ---" << endl;
        EV_FATAL << "Falcon-512 Public Key Length: " << keyPair.pubKeyLength << " bytes" << endl;
        EV_FATAL << "Falcon-512 Public Key Length after string: " << strlen(keyPair.pubHex.c_str()) << " bytes" << endl;
        EV_FATAL << "---------------------------" << endl;
        Cert.setSubjectId(getParentModule()->getParentModule()->getFullName());
//        Cert.setAlgoName("Falcon-512");
        Cert.setAlgoName("Dilithium 2");
        //Cert.setPublicKeyHex(keyPair.pubHex.c_str());
//        std::vector<uint8_t> pubKeyBinary = pqcdsa::fromHex(keyPair.pubHex);
//        Cert.setPublicKeyArraySize(keyPair.pubKeyLength);
//        for (size_t i = 0; i < 897; ++i) {
//            Cert.setPublicKey(i, pubKeyBinary[i]); // Set each byte of the publicKey array
//        }
        auto pkBytes = pqcdsa::fromHex(keyPair.pubHex);            // binary vector
        Cert.setPublicKeyArraySize(pkBytes.size());
        for (size_t i = 0; i < pkBytes.size(); ++i)
            Cert.setPublicKey(i, pkBytes[i]);
        Cert.setNotBefore(0);
        Cert.setNotAfter(9223372036854775807LL);
        // --- END ADDED ---

        // Schedule the first BSM/SPDU transmission
        bsmSeq = 0;
        sendEvt = new cMessage("sendSPDU");
        //double startTime = par("startTime").doubleValue();
        scheduleAt(simTime() + 1, sendEvt); // Start sending after 1 second


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
        warnVerified_ = registerSignal("warnVerified");

        double delay = 0.001 * intuniform(0, 1000, 0);
        //scheduleAt((simTime() + delay).trunc(SIMTIME_MS), selfSender_);
    }
}

void Mode4App::handleLowerMessage(cMessage* msg)
{
    if (msg->isName("CBR")) {
        Cbr* cbrPkt = check_and_cast<Cbr*>(msg);
        double channel_load = cbrPkt->getCbr();
        emit(cbr_, channel_load);
        delete cbrPkt;
    }else if (auto* w = dynamic_cast<IcaWarn*>(msg)) {
        emit(warnReceived_, 1);
        // (later) verify signatures here and emit(warnVerified_,1) if valid

        // Optional: small log
        EV_INFO << "[UE] got IcaWarn msgCnt=" << w->getMsgCnt()
                << " intId=" << w->getIntersectionId()
                << " lane=" << w->getLane()
                << " app="  << w->getApproach()
                << " flag=" << w->getEventFlag()
                << "\n";
        delete w;
        return;
    } else {
//        AlertPacket* pkt = check_and_cast<AlertPacket*>(msg);
//
//        if (pkt == 0)
//            throw cRuntimeError("Mode4App::handleMessage - FATAL! Error when casting to AlertPacket");
//
//        // emit statistics
//        simtime_t delay = simTime() - pkt->getTimestamp();
//        emit(delay_, delay);
//        emit(rcvdMsg_, (long)1);
//
//        EV << "Mode4App::handleMessage - Packet received: SeqNo[" << pkt->getSno() << "] Delay[" << delay << "]" << endl;

        SPDU* spdu = dynamic_cast<SPDU*>(msg);
        if (!spdu) {
            EV << "Received a non-SPDU message, deleting." << endl;
            delete msg;
            return;
        }
        simtime_t delay = simTime() - spdu->getTimestamp();
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
            std::string pubKeyHex = pqcdsa::toHex(pkBytes.data(), pkBytes.size());
            std::vector<uint8_t> sigBytes(spdu->getSignatureArraySize());
                for (size_t i = 0; i < sigBytes.size(); ++i)
                    sigBytes[i] = spdu->getSignature(i);
                std::string sigHex = pqcdsa::toHex(sigBytes.data(), sigBytes.size());

        bool ok = pqcdsa::verify(bsmHex, sigHex, pubKeyHex);
        if (ok) {
            // Only count the event if the signature was valid
            emit(verified_, long(1));
        }
        //bool ok = pqcdsa::verify(bsmHex, spdu->getSignatureHex(), spdu->getCert().getPublicKeyHex());

        EV_INFO << "RX BSM#" << b.getMsgId() << " from " << spdu->getCert().getSubjectId() << "  -->  " << (ok ? "VALID" : "INVALID") << '\n';

        delete msg;
    }


//    SPDU* spdu = dynamic_cast<SPDU*>(msg);
//    if (!spdu) { /* existing path */ }
//
//    // If this is a signed warning from the RSU
//    const bool isWarn = !strcmp(spdu->getName(), "WARN_SPDU");
//
//    simtime_t delay = simTime() - spdu->getTimestamp();
//    emit(delay_, delay);
//    emit(received_, long(1));
//    if (isWarn) emit(warnReceived_, long(1));
//
//    // build message string exactly like RSU did for WARNs, or like sender for BSMs
//    std::ostringstream os;
//    const BSM& b = spdu->getBsm();
//    if (isWarn)
//        os << "WARN," << b.getMsgId() << ',' << b.getLatitude() << ','
//           << b.getLongitude() << ',' << b.getHeading() << ',' << b.getSpeed();
//    else
//        os << b.getMsgId() << ',' << b.getLatitude() << ','
//           << b.getLongitude() << ',' << b.getHeading() << ',' << b.getSpeed();
//
//    std::string bsmHex = pqcdsa::toHex(reinterpret_cast<const uint8_t*>(os.str().data()), os.str().size());
//
//    // extract pubkey + sig and verify
//    const Certificate &c = spdu->getCert();
//    std::vector<uint8_t> pkBytes(c.getPublicKeyArraySize());
//    for (size_t i = 0; i < pkBytes.size(); ++i) pkBytes[i] = c.getPublicKey(i);
//    std::string pubKeyHex = pqcdsa::toHex(pkBytes.data(), pkBytes.size());
//
//    std::vector<uint8_t> sigBytes(spdu->getSignatureArraySize());
//    for (size_t i = 0; i < sigBytes.size(); ++i) sigBytes[i] = spdu->getSignature(i);
//    std::string sigHex = pqcdsa::toHex(sigBytes.data(), sigBytes.size());
//
//    bool ok = pqcdsa::verify(bsmHex, sigHex, pubKeyHex);
//    if (ok) {
//        emit(verified_, long(1));         // vehicle BSM verification (existing)
//        if (isWarn) emit(warnVerified_, long(1));  // RSU warning verification
//    }
//
//    EV_INFO << (isWarn ? "RX WARN_SPDU" : "RX BSM")
//            << " #" << b.getMsgId() << " from " << spdu->getCert().getSubjectId()
//            << " --> " << (ok ? "VALID" : "INVALID") << '\n';
//
//    delete msg;

}

//void Mode4App::handleSelfMessage(cMessage* msg)
//{
//    if (!strcmp(msg->getName(), "selfSender")){
//        // Replace method
//        AlertPacket* packet = new AlertPacket("Alert");
//        packet->setTimestamp(simTime());
//        packet->setByteLength(size_);
//        packet->setSno(nextSno_);
//
//        nextSno_++;
//
//        auto lteControlInfo = new FlowControlInfoNonIp();
//
//        lteControlInfo->setSrcAddr(nodeId_);
//        lteControlInfo->setDirection(D2D_MULTI);
//        lteControlInfo->setPriority(priority_);
//        lteControlInfo->setDuration(duration_);
//        lteControlInfo->setCreationTime(simTime());
//
//        packet->setControlInfo(lteControlInfo);
//
//        Mode4BaseApp::sendLowerPackets(packet);
//        emit(sentMsg_, (long)1);
//
//        scheduleAt(simTime() + period_, selfSender_);
//    }else if (msg == sendEvt && bsmSeq < 10) { // Send 10 BSMs
//        generateAndSendSPDU();
//        ++bsmSeq;
//        scheduleAt(simTime() + 1, sendEvt); // Schedule the next one
//        return;
//    }
//    else
//        throw cRuntimeError("Mode4App::handleMessage - Unrecognized self message");
//}

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
    bsm.setLatitude(0.0);
    bsm.setLongitude(0.0);
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
    cancelAndDelete(sendEvt);
//    cancelAndDelete(selfSender_);
}

Mode4App::~Mode4App()
{
//    if (getSimulation()->getSimulationStage() != CTX_FINISH)
//    {
//        simtime_t lifetime = simTime() - entryTime;
//        emit(lifetimeSignal, lifetime);
//    }

    binder_->unregisterNode(nodeId_);

    //cancelAndDelete(sendEvt);
    // Free the allocated memory for OQS keys
//    if (sig_ != nullptr) {
//        OQS_SIG_free(sig_);
//        free(public_key_);
//        free(secret_key_);
//    }
}


