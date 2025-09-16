#ifndef __APPS_MODE4APP_MODE4RSUAPP_H_
#define __APPS_MODE4APP_MODE4RSUAPP_H_

#include "apps/mode4App/Mode4BaseApp.h"
#include "apps/mode4App/BSM_m.h"
#include "apps/mode4App/Certificate_m.h"
#include "apps/mode4App/SPDU_m.h"
#include "apps/mode4App/pqcdsa.h"
#include "corenetwork/binder/LteBinder.h"
#include "apps/mode4App/IcaWarn_m.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

class Mode4RSUApp : public Mode4BaseApp
{
  public:
    ~Mode4RSUApp() override;

  protected:

    simsignal_t rsuReceivedMsg;
    simsignal_t rsuVerifiedMsg;
    simsignal_t cbr_;
    simsignal_t numBroadcasted;

    /* crypto */
    pqcdsa::KeyPair keyPair_;
    Certificate     cert_;
    int             warnSeq_ = 0;

    // NEW: external trigger socket
    int          sockFd_ = -1;
    sockaddr_in  sockAddr_{};
    cMessage*    sockPollEvt_ = nullptr;

    LteBinder* binder_;
    MacNodeId nodeId_;

    virtual void initialize(int stage) override;
    virtual void handleLowerMessage(cMessage* msg) override;
    virtual void handleSelfMessage(cMessage *msg) override; // Required by base class

    // Helpers
    void openNonBlockingUdp_(int port);
    void socketRead();
    void broadcastIca(IcaWarn* w);

};

#endif
