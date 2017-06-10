/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#define LOG_TAG "PerMgrLib"

#include <utils/Log.h>
#include <stdint.h>
#include <sys/types.h>

#include "IPeripheralManager.h"

namespace android {

// ----------------------------------------------------------------------

enum {
    NOTIFY_CALLBACK = IBinder::FIRST_CALL_TRANSACTION,
};

class BpPeriperalManagerCb: public BpInterface<IPeriperalManagerCb> {
  public:
    BpPeriperalManagerCb(const sp<IBinder> &impl) : BpInterface<IPeriperalManagerCb>(impl) {}

    void notifyCallback(int32_t event) {
        Parcel data, reply;
        data.writeInterfaceToken(IPeriperalManagerCb::getInterfaceDescriptor());
        data.writeInt32(event);
        remote()->transact(NOTIFY_CALLBACK, data, &reply, IBinder::FLAG_ONEWAY);
        data.writeNoException();
    }
};

IMPLEMENT_META_INTERFACE(PeriperalManagerCb, "com.qualcomm.IPeriperalManagerCb");

status_t BnPeriperalManagerCb::onTransact(uint32_t code, const Parcel &data,
                                               Parcel *reply, uint32_t flags) {
    switch (code) {
    case NOTIFY_CALLBACK: {
        CHECK_INTERFACE(IPeriperalManagerCb, data, reply);
        int32_t event = data.readInt32();
        notifyCallback(event);
        data.readExceptionCode();
        return NO_ERROR;
    }
    break;

    default:
        return BBinder::onTransact(code, data, reply, flags);
    }
}

// ----------------------------------------------------------------------

enum {
    PM_REGISTER = IBinder::FIRST_CALL_TRANSACTION,
    PM_UNREGISTER = IBinder::FIRST_CALL_TRANSACTION + 1,
    PM_CONNECT = IBinder::FIRST_CALL_TRANSACTION + 2,
    PM_DISCONNECT = IBinder::FIRST_CALL_TRANSACTION + 3,
    PM_ACKNOWLEDGE = IBinder::FIRST_CALL_TRANSACTION + 4,
    PM_SHUTDOWN = IBinder::FIRST_CALL_TRANSACTION + 5,
};

// Client
class BpPeripheralManager: public BpInterface <IPeripheralManager> {
  public:
    BpPeripheralManager(const sp<IBinder> &impl): BpInterface
        <IPeripheralManager> (impl) {
    }

    virtual status_t registar(const String8 &devName,
                              const String8 &clientName,
                              const sp<IPeriperalManagerCb> &notifier,
                              int64_t *clientId,
                              int64_t *state) {
        Parcel data, reply;

        data.writeInterfaceToken(IPeripheralManager::getInterfaceDescriptor());
        data.writeString8(devName);
        data.writeString8(clientName);
        data.writeStrongBinder(notifier->asBinder());
        remote()->transact(PM_REGISTER, data, &reply);
        *clientId = reply.readInt64();
        *state = reply.readInt64();
        return reply.readInt32();
    }

    virtual status_t unregister(int64_t clientId) {
        Parcel data, reply;

        data.writeInterfaceToken(IPeripheralManager::getInterfaceDescriptor());
        data.writeInt64(clientId);
        remote()->transact(PM_UNREGISTER, data, &reply);
        return reply.readInt32();
    }

    virtual status_t connect(int64_t clientId) {
        Parcel data, reply;

        data.writeInterfaceToken(IPeripheralManager::getInterfaceDescriptor());
        data.writeInt64(clientId);
        remote()->transact(PM_CONNECT, data, &reply);
        return reply.readInt32();
    }

    virtual status_t disconnect(int64_t clientId) {
        Parcel data, reply;

        data.writeInterfaceToken(IPeripheralManager::getInterfaceDescriptor());
        data.writeInt64(clientId);
        remote()->transact(PM_DISCONNECT, data, &reply);
        return reply.readInt32();
    }

    virtual status_t acknowledge(int64_t clientId, int32_t event) {
        Parcel data, reply;

        data.writeInterfaceToken(IPeripheralManager::getInterfaceDescriptor());
        data.writeInt64(clientId);
        data.writeInt32(event);
        remote()->transact(PM_ACKNOWLEDGE, data, &reply);
        return reply.readInt32();
    }

    virtual status_t shutdown() {
        Parcel data, reply;

        data.writeInterfaceToken(IPeripheralManager::getInterfaceDescriptor());
        remote()->transact(PM_SHUTDOWN, data, &reply);
        return reply.readInt32();
    }
};

IMPLEMENT_META_INTERFACE(PeripheralManager, "vendor.qcom.IPeripheralManager");

// Server
status_t BnPeripheralManager::onTransact(uint32_t code, const Parcel &data,
                                         Parcel *reply, uint32_t flags) {
    switch (code) {
    case PM_REGISTER: {
        CHECK_INTERFACE(IPeripheralManager, data, reply);
        sp<IPeriperalManagerCb> notifier;
        int64_t clientId, state;
        String8 devName = data.readString8();
        String8 clientName = data.readString8();
        notifier = interface_cast <IPeriperalManagerCb> (data.readStrongBinder());
        int32_t ret = registar(devName, clientName, notifier, &clientId, &state);
        reply->writeInt64(clientId);
        reply->writeInt64(state);
        reply->writeInt32(ret);
        return NO_ERROR;
        break;
    }

    case PM_UNREGISTER: {
        CHECK_INTERFACE(IPeripheralManager, data, reply);
        int64_t clientId = data.readInt64();
        int32_t ret = unregister(clientId);
        reply->writeInt32(ret);
        return NO_ERROR;
        break;
    }

    case PM_CONNECT: {
        CHECK_INTERFACE(IPeripheralManager, data, reply);
        int64_t clientId = data.readInt64();
        int32_t ret = connect(clientId);
        reply->writeInt32(ret);
        return NO_ERROR;
        break;
    }

    case PM_DISCONNECT: {
        CHECK_INTERFACE(IPeripheralManager, data, reply);
        int64_t clientId = data.readInt64();
        int32_t ret = disconnect(clientId);
        reply->writeInt32(ret);
        return NO_ERROR;
        break;
    }

    case PM_ACKNOWLEDGE: {
        CHECK_INTERFACE(IPeripheralManager, data, reply);
        int64_t clientId = data.readInt64();
        int32_t event = data.readInt32();
        int32_t ret = acknowledge(clientId, event);
        reply->writeInt32(ret);
        return NO_ERROR;
        break;
    }

    case PM_SHUTDOWN: {
        CHECK_INTERFACE(IPeripheralManager, data, reply);
        int32_t ret = shutdown();
        reply->writeInt32(ret);
        return NO_ERROR;
        break;
    }

    default:
        return BBinder::onTransact(code, data, reply, flags);
    }
}

}; // namespace android
