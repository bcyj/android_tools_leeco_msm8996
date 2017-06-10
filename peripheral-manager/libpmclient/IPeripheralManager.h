/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#ifndef __PM_IPM_H__
#define __PM_IPM_H__

#include <utils/RefBase.h>
#include <utils/Timers.h>
#include <utils/String8.h>
#include <binder/IInterface.h>
#include <binder/Parcel.h>
#include <binder/IMemory.h>

namespace android {

// ----------------------------------------------------------------------------

class IPeriperalManagerCb: public IInterface {
  public:
    DECLARE_META_INTERFACE(PeriperalManagerCb);
    // Stared by server when it have message to client
    // event - see enum pm_event
    virtual void notifyCallback(int32_t event) = 0;
};


class BnPeriperalManagerCb: public BnInterface<IPeriperalManagerCb> {
  public:
    virtual status_t onTransact(uint32_t code, const Parcel &data,
                               Parcel *reply, uint32_t flags = 0);
};

// ----------------------------------------------------------------------------

// Client
class IPeripheralManager: public IInterface {
  public:
    DECLARE_META_INTERFACE(PeripheralManager);

    // devName  - Peripheral device name , eg. "MDM9x25"
    // notifier       - Callback to be executed when server have message for client
    // clientId - return value from server, to be used as argument for
    //            following API calls
    // state    - Peripheral device state, when client is registered
    //            (enum pm_event)
    virtual status_t registar(const String8 &devName,
                              const String8 &clientName,
                              const sp<IPeriperalManagerCb> &notifier,
                              int64_t *clientId,
                              int64_t *state) = 0;

    // clientId - value returned from server in above API
    virtual status_t unregister(int64_t clientId) = 0;

    // clientId - value returned from server in above API
    virtual status_t connect(int64_t clientId) = 0;

    // clientId - value returned from server in above API
    virtual status_t disconnect(int64_t clientId) = 0;

    // clientId - value returned from server in above API
    // event      - Client acknowledge message send by serve, see class BpPeriperalManagerCb
    virtual status_t acknowledge(int64_t clientId, int32_t event) = 0;

    virtual status_t shutdown(void) = 0;

};


// Server
class BnPeripheralManager: public BnInterface <IPeripheralManager> {
  public:
    virtual status_t onTransact(uint32_t code, const Parcel &data,
                                Parcel *reply, uint32_t flags = 0);
};

}; // namespace android

#endif
