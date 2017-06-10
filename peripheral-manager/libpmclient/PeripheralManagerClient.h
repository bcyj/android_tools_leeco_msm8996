/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#ifndef __PM_IPMCLIENT_H__
#define __PM_IPMCLIENT_H__

#include <list>
#include <binder/MemoryHeapBase.h>
#include <binder/IServiceManager.h>
#include <binder/IPCThreadState.h>

#include "IPeripheralManager.h"
#include "pm-service.h"

using namespace std;

namespace android {

class EventNotifier: public BnPeriperalManagerCb {
  public:
    EventNotifier(pm_client_notifier callback, void *clientData);
    // these are static callback functions
    virtual void notifyCallback(int32_t event);

  private:
    // The callback function pointer, that client gives, in order to be
    // notified for events later
    pm_client_notifier mClientCb;
    // Client managed structure. We return it with the callback, so clients can
    // determine what is this callback for
    void *mClientData;
};

struct PeripheralManagerClient;

class ServerDiedNotifier : public IBinder::DeathRecipient {
  public:
    ServerDiedNotifier(struct PeripheralManagerClient *client);
  protected:
    virtual void binderDied(const wp<IBinder> &who);
  private:
    struct PeripheralManagerClient *mClient;
};

class ServerQuitNotifier : public IBinder::DeathRecipient {
  public:
    ServerQuitNotifier(Mutex *lock);
  protected:
    virtual void binderDied(const wp<IBinder> &who);
  private:
    // To be posted when binder detect that service is no longer a live
    mutable Mutex *mDone;
};

struct PeripheralManagerClient {
    // Returned from server side, to identify clients
    int64_t id;
    // Periperal device devName
    String8 devName;
    // Name of the client
    String8 clientName;
    // This state track whether client have done successfully connect
    // to server. It will be used to reconnect after server crashed and restarted.
    bool connected;
    sp<IPeriperalManagerCb> notifier;
    sp<IPeripheralManager> ops;
    sp<ServerDiedNotifier> recipient;

    mutable Mutex mLock;
};

}; // namespace android
#endif
