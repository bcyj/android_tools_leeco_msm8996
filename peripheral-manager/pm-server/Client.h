/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __PM_CLIENT_H__
#define __PM_CLIENT_H__

#include <utils/RefBase.h>
#include <utils/String8.h>

#include "pm-service.h"
#include "IPeripheralManager.h"

namespace android {

#define EVENT_INVALID       ((enum pm_event) -1)

// This is a class represent PM Service view for APP Client it is used
// in the peripheral manager server context, it is used to hold clients data
class Client {
  public:
    // Constructor used for binder/APP clients
    Client(const sp<IPeriperalManagerCb> &notifier, void *service, const String8 &name);
    ~Client();
    // Client recipient notification
    class ClientDiedNotifier: public IBinder::DeathRecipient {
      public:
        ClientDiedNotifier(void *client, void *service);
      protected:
        // Invoked by binder when client crash. Implemented by Server
        virtual void binderDied(const wp<IBinder> &who);
      private:
        void *mClient;
        // Peripheral manager instance
        void *mService;
    };

    // Send to APP client Peripheral power state change events
    void eventSend(enum pm_event event);
    // Send to APP client Peripheral power state change events
    void eventReceived(enum pm_event event);
    bool isAckExpected();

    void setIsVoter(bool isVoter);
    bool getIsVoter();
    const char* getName();

  private:
    // Callback function which we receive from client, in order to eventSend for
    // events in the future
    sp<IPeriperalManagerCb> mClientCb;
    // Death eventNotifier
    sp<Client::ClientDiedNotifier> mDeathNotifier;
    // Is this client voting for peripheral power up?
    bool mIsVoter;
    // Are we waiting for an acknowledge from the client?
    enum pm_event mAckExpected;
    //Name of the client
    String8 mName;
};

}; // namespace android
#endif
