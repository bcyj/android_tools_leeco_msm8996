/*
**
** Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
** Qualcomm Technologies Confidential and Proprietary.
**
** Not a Contribution, Apache license notifications and
** license are retained for attribution purposes only.
**/
/*
** Copyright 2010, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#define LOG_NDEBUG 0
#define LOG_TAG "IListenDeathNotifier"
#include <utils/Log.h>

#include <binder/IServiceManager.h>
#include <binder/IPCThreadState.h>

#include <IListenDeathNotifier.h>

namespace android {

// client singleton for binder interface to Listen Service
Mutex IListenDeathNotifier::sServiceLock;
sp<IListenService> IListenDeathNotifier::sListenService;
sp<IListenDeathNotifier::DeathNotifier> IListenDeathNotifier::sDeathNotifier;
SortedVector< wp<IListenDeathNotifier> > IListenDeathNotifier::sObitRecipients;

// establish binder interface to ListenService
const sp<IListenService>& IListenDeathNotifier::getListenService()
{
    ALOGV("getListenService");
    Mutex::Autolock _l(sServiceLock);
    if (sListenService == 0) {
        sp<IServiceManager> sm;
        sp<IBinder> binder;
        do {
            sm = defaultServiceManager();
            if (sm != 0) {
               binder = sm->getService(String16("listen.service"));
               if (binder != 0) {
                  break;
               } else {
                  ALOGE("getService instance of listen.service failed");
               }
            } else {
                ALOGE("defaultServiceManager failed");
            }
            ALOGV("Listen service not published, waiting...");
            usleep(500000); // 0.5 s
        } while (true);

        if (sDeathNotifier == NULL) {
           sDeathNotifier = new DeathNotifier();
       }
       binder->linkToDeath(sDeathNotifier);
       sListenService = interface_cast<IListenService>(binder);
    }
    ALOGE_IF(sListenService == 0, "no listen service!?");
    return sListenService;
}

void IListenDeathNotifier::addObitRecipient(const wp<IListenDeathNotifier>& recipient)
{
    ALOGV("addObitRecipient");
    Mutex::Autolock _l(sServiceLock);
    sObitRecipients.add(recipient);
}

void IListenDeathNotifier::removeObitRecipient(const wp<IListenDeathNotifier>& recipient)
{
    ALOGV("removeObitRecipient");
    Mutex::Autolock _l(sServiceLock);
    sObitRecipients.remove(recipient);
}

void IListenDeathNotifier::DeathNotifier::binderDied(const wp<IBinder>& who) {
    ALOGV("Listen server died");

    // Need to do this with the lock held
    SortedVector< wp<IListenDeathNotifier> > list;
    {
        Mutex::Autolock _l(sServiceLock);
        sListenService.clear();
        list = sObitRecipients;
    }

    // Notify application when Listen server dies.
    // Don't hold the static lock during callback in case app
    // makes a call that needs the lock.
    size_t count = list.size();
    for (size_t iter = 0; iter < count; ++iter) {
        sp<IListenDeathNotifier> notifier = list[iter].promote();
        if (notifier != 0) {
            notifier->died();
        }
    }
}

IListenDeathNotifier::DeathNotifier::~DeathNotifier()
{
    ALOGV("DeathNotifier::~DeathNotifier");
    Mutex::Autolock _l(sServiceLock);
    sObitRecipients.clear();
    if (sListenService != 0) {
        sp<IBinder> pBinder = sListenService->asBinder();
        if (pBinder != NULL)
            pBinder->unlinkToDeath(this);
    }
}

}; // namespace android
