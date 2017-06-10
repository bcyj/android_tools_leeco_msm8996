/*
 *
 * Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Confidential and Proprietary.
 *
 * Not a Contribution, Apache license notifications and
 * license are retained for attribution purposes only.
 */
/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/*
 * IListenClientDeathNotifier
 *
 * Inherited by IListenReceiver in order for Listen Service to be notified when
 * Listen Receiver dies.
 */
#ifndef ANDROID_ILISTENCLIENTDEATHNOTIFIER_H
#define ANDROID_ILISTENCLIENTDEATHNOTIFIER_H

#include <utils/threads.h>
#include <utils/Errors.h>  // for status_t

#include <IListenReceiver.h>

namespace android {

// forward declaration
class ListenService;

class IListenClientDeathNotifier: virtual public RefBase
{
public:
    IListenClientDeathNotifier();
    virtual ~IListenClientDeathNotifier();

    virtual void died() = 0;

private:
    IListenClientDeathNotifier &operator=(const IListenClientDeathNotifier &);
    IListenClientDeathNotifier(const IListenClientDeathNotifier &);

protected:
    virtual status_t setListenClientContext(const sp<IListenReceiver>& receiver) = 0;
    virtual status_t clearListenClientContext(const sp<IListenReceiver>& receiver) = 0;

    /* static */ void setObitRecipient(ListenService *);
    /* static */ void clearObitRecipient();

protected:
    class DeathNotifier: public IBinder::DeathRecipient
    {
      public:
        DeathNotifier() {}
        virtual ~DeathNotifier();

        virtual void binderDied(const wp<IBinder>& who);
    };

    friend class DeathNotifier;

protected:
    // leave the Lock static so that only one DeathNotifier is cleaning up service any one time
    static Mutex               sReceiverLock;
    // Only ListenService is the receipient of obituary events so it can be static
    static ListenService *     sObitRecipient;
    static SortedVector< sp<IListenReceiver> > sListenReceivers;
    static sp<DeathNotifier>   sDeathNotifier;
};

}; // namespace android

#endif // ANDROID_ILISTENCLIENTDEATHNOTIFIER_H
