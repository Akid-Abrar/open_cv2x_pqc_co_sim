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

#ifndef _LTE_MODE4APP_H_
#define _LTE_MODE4APP_H_

#include "apps/mode4App/Mode4BaseApp.h"
#include "apps/alert/AlertPacket_m.h"
#include "corenetwork/binder/LteBinder.h"

// --- AKID ---
#include "apps/mode4App/BSM_m.h"
#include "apps/mode4App/Certificate_m.h"
#include "apps/mode4App/SPDU_m.h"
#include "apps/mode4App/pqcdsa.h"
#include "apps/mode4App/IcaWarn_m.h"


class Mode4App : public Mode4BaseApp {

public:
    ~Mode4App() override;

protected:
    //sender
    int size_;
    int nextSno_;
    int priority_;
    int duration_;
    simtime_t period_;

    simsignal_t sentMsg_;
    simsignal_t delay_;
//    simsignal_t rcvdMsg_;
    simsignal_t cbr_;

    simtime_t entryTime;
    simsignal_t lifetimeSignal;
    simsignal_t received_;
    simsignal_t verified_;

    // PDR tracking for ICA
    int    lastIcaSeq_        = -1;  // last received seq
    long   icaExpected_       = 0;   // received + missed
    long   icaReceived_       = 0;
    double lastIcaDist_       = 0;   // last known distance to RSU

    simsignal_t warnReceived_;
    simsignal_t warnVerified_;
    simsignal_t warnExpected_;
    simsignal_t warnPdrSample_;
    simsignal_t warnPdrDistance_;
    simsignal_t rxWarnDist_;
    simsignal_t icaVerifyMs_ = SIMSIGNAL_NULL;
    simsignal_t icaDelayMs_  = SIMSIGNAL_NULL;

    cMessage *selfSender_;

    LteBinder* binder_;
    MacNodeId nodeId_;

    pqcdsa::KeyPair keyPair;
    Certificate     Cert;

    cMessage* sendEvt = nullptr;
    int       bsmSeq  = 0;

   int numInitStages() const { return inet::NUM_INIT_STAGES; }

   void initialize(int stage);

   void handleLowerMessage(cMessage* msg);

   void finish();

   void handleSelfMessage(cMessage* msg);

   void sendLowerPackets(cPacket* pkt);

   void generateAndSendSPDU();

};

#endif
