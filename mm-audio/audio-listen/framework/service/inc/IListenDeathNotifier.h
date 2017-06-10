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
 * IListenDeathNotifier
 *
 * Inherited by IListenReceiver in order for Listen client to be notified when
 * Listen Native Service dies.
 * Each Listen Native Receiver is add to this DeathNotifies Obituary list so that
 * all active Listen Native Receiver can have there notification callback functions
 * call with an event denoting that Listen Service thread has died.
 * It is the responsibility of the Listen client code (in Java) to clean up ("release")
 * Listen objects when it gets this notification.
 */
#ifndef ANDROID_ILISTENDEATHNOTIFIER_H
#define ANDROID_ILISTENDEATHNOTIFIER_H

#include <utils/threads.h>
#include <utils/SortedVector.h>

#include <IListenService.h>

namespace android {

class IListenDeathNotifier: virtual public RefBase
{
public:
    IListenDeathNotifier() { addObitRecipient(this); }
    virtual ~IListenDeathNotifier() { removeObitRecipient(this); }

    virtual void died() = 0;
    static const sp<IListenService>& getListenService();

private:
    IListenDeathNotifier &operator=(const IListenDeathNotifier &);
    IListenDeathNotifier(const IListenDeathNotifier &);

    static void addObitRecipient(const wp<IListenDeathNotifier>& recipient);
    static void removeObitRecipient(const wp<IListenDeathNotifier>& recipient);

    class DeathNotifier: public IBinder::DeathRecipient
    {
    public:
        DeathNotifier() {}
        virtual ~DeathNotifier();

        virtual void binderDied(const wp<IBinder>& who);
    };

    friend class DeathNotifier;

    static  Mutex                                    sServiceLock;
    static  sp<IListenService>                       sListenService;
    static  sp<DeathNotifier>                        sDeathNotifier;
    static  SortedVector< wp<IListenDeathNotifier> > sObitRecipients;
};

}; // namespace android

#endif // ANDROID_ILISTENDEATHNOTIFIER_H
