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
#define LOG_TAG "IListenClientDeathNotifier"
#include <utils/Log.h>

#include <binder/IPCThreadState.h>

#include <IListenClientDeathNotifier.h>
#include <ListenService.h>

namespace android {

// client singleton for binder interface to Listen Receiver
Mutex               IListenClientDeathNotifier::sReceiverLock;
ListenService *     IListenClientDeathNotifier::sObitRecipient = NULL;
SortedVector< sp<IListenReceiver> > IListenClientDeathNotifier::sListenReceivers;

sp<IListenClientDeathNotifier::DeathNotifier>   IListenClientDeathNotifier::sDeathNotifier = 0;


IListenClientDeathNotifier::IListenClientDeathNotifier()
{
    ALOGV("constructor, this = %p", this);
    sDeathNotifier = new DeathNotifier();  // stores DeathNotifier into sp<>
    if (sDeathNotifier == 0) {
       ALOGE("constructor: ERROR DeathNotifier could not be allocated!?");
    }
}

IListenClientDeathNotifier::~IListenClientDeathNotifier() {
    ALOGV("destructor, this = %p", this);
    sListenReceivers.clear();
    if ( sDeathNotifier != 0) {
       // %%% DeathNotifier was new'ed before assigning to a sp<>
       sDeathNotifier = 0;
    }
}

void IListenClientDeathNotifier::setObitRecipient(ListenService * recipient)
{
    ALOGV("setObitRecipient");
    sObitRecipient = recipient;  // %%% will this cause a mem leak?
    ALOGV("setObitRecipient: mObitRecipient set to %p", sObitRecipient);
}

// Should be called only when ListenService is torn down
void IListenClientDeathNotifier::clearObitRecipient()
{
    ALOGV("clearObitRecipient");
    sObitRecipient = NULL;
}

// Notify Listen Service when ListenReciever client dies
// get each of the ListenReceivers from list and call method to clean up
// references to these in ListenService
void IListenClientDeathNotifier::DeathNotifier::binderDied(const wp<IBinder>& who) {
    ALOGV("binderDied: Listen Receiver died, who wp<%p>", who.unsafe_get() );

    SortedVector< sp<IListenReceiver> > receiverList;
    {
        // Need to do assignment (copy) with the lock held
        Mutex::Autolock _l(sReceiverLock);
        receiverList = sListenReceivers;
        // let this static lock be freed at the end of this block
        //     in case app makes a call that needs the lock.
    }

    size_t count = receiverList.size();
    ALOGV("binderDied: receiver list count %d", count );
    for (size_t iter = 0; iter < count; ++iter) {
        sp<IListenReceiver> spReceiver = receiverList.itemAt(iter);
        if ( (spReceiver != 0) &&
             (spReceiver->asBinder().get() == who.unsafe_get() ) ) {
           ALOGV("binderDied: cleanupDeadClient called with %p", spReceiver.get());
           sObitRecipient->cleanupDeadClient(spReceiver);
        }
    }
    ALOGV("binderDied: returns" );
}

IListenClientDeathNotifier::DeathNotifier::~DeathNotifier()
{
    ALOGV("DeathNotifier::~DeathNotifier");
}

}; // namespace android
