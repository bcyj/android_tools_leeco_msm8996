/*
 * Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#define LOG_TAG "PerMgrSrv"

#include "PeripheralManagerServer.h"
#include "Client.h"

namespace android {


Client::Client(const sp<IPeriperalManagerCb> &notifier, void *service, const String8 &name) {

    mClientCb = notifier;
    mIsVoter = false;
    mName = name;
    mAckExpected = EVENT_INVALID;
    mDeathNotifier = new ClientDiedNotifier(this, service);
    mClientCb->asBinder()->linkToDeath(mDeathNotifier);
}

Client::~Client() {

    mClientCb->asBinder()->unlinkToDeath(mDeathNotifier);
}

void Client::eventSend(enum pm_event event) {

    mAckExpected = event;
    mClientCb->notifyCallback((int32_t)event);
}

void Client::eventReceived(enum pm_event event) {

    if (event == EVENT_INVALID) {

        if (mAckExpected != EVENT_INVALID) {
            // Event wait canceled
        }

        // Just forget what you waiting, timeout happen
        mAckExpected = EVENT_INVALID;
        return;
    }

    if (mAckExpected == event) {
        mAckExpected = EVENT_INVALID;
        return;
    }

    // Probably callback received too late, or it is from a listener only
    // in both cases it is ignored
}

bool Client::isAckExpected() {

    return mAckExpected != EVENT_INVALID;
}

void Client::setIsVoter(bool isVoter) {

    mIsVoter = isVoter;
}

bool Client::getIsVoter() {

    return mIsVoter;
}

const char* Client::getName() {
    return mName.string();
}

Client::ClientDiedNotifier::ClientDiedNotifier(void *client, void *service) {

    mClient = client;
    mService = service;
}

void Client::ClientDiedNotifier::binderDied(const wp<IBinder> &who) {
    const wp<IBinder> unused = who;
    PeripheralManagerServer *service;
    Client *client;

    client = reinterpret_cast <Client *>(mClient);
    service = reinterpret_cast <PeripheralManagerServer *>(mService);

    service->clientDied(client);
}

}; // namespace android
