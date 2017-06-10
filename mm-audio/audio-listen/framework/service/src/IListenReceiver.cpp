/*
**
** Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
** Qualcomm Technologies Confidential and Proprietary.
**
** Not a Contribution, Apache license notifications and
** license are retained for attribution purposes only.
**/
/*
**
** Copyright 2008, The Android Open Source Project
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

#include <utils/RefBase.h>
#include <binder/IInterface.h>
#include <binder/Parcel.h>

#include <IListenReceiver.h>

#define LOG_TAG "IListenReceiver"
#define LOG_NDEBUG 0

namespace android {

enum {
    NOTIFY = IBinder::FIRST_CALL_TRANSACTION,
};

class BpListenReceiver: public BpInterface<IListenReceiver>
{
public:
    BpListenReceiver(const sp<IBinder>& impl)
        : BpInterface<IListenReceiver>(impl)
    {
       ALOGD("BnListenReceiver::constructor - impl=%p, this=%p", impl.get(), this);
    }

    ~BpListenReceiver()
    {
       ALOGD("BnListenReceiver::destructor, this=%p", this);
    }

    virtual void notify(int msg, const Parcel *obj)
    {
        Parcel data, reply;
        size_t objParcelSize = 0;
        data.writeInterfaceToken(IListenReceiver::getInterfaceDescriptor());
        data.writeInt32(msg);
        if (obj) {
           objParcelSize = obj->dataSize();
           if (obj->dataSize() > 0)
            data.appendFrom(const_cast<Parcel *>(obj), 0, objParcelSize);
        }
        ALOGD("BnListenReceiver::notify - type = %d, parcelSize = %d, should be payloadSize plus 4",
                     msg, objParcelSize);
        remote()->transact(NOTIFY, data, &reply, IBinder::FLAG_ONEWAY);
    }

};

IMPLEMENT_META_INTERFACE(ListenReceiver, "com.qualcomm.listen.IListenReceiver");

// ----------------------------------------------------------------------

status_t BnListenReceiver::onTransact(
    uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags)
{
    switch (code) {
        case NOTIFY: {
            ALOGI("BnListenReceiver::onTransact received NOTIFY");
            CHECK_INTERFACE(IListenReceiver, data, reply);
            int msg = data.readInt32();
            Parcel obj;
            if (data.dataAvail() > 0) {
                ALOGI("append %d bytes available at pos %d",
                      data.dataAvail(), data.dataPosition());
                obj.appendFrom(const_cast<Parcel *>(&data), data.dataPosition(), data.dataAvail());
            }

            notify(msg, &obj);
            return NO_ERROR;
        } break;
        default:
            ALOGI("BnListenReceiver::onTransact received unknown msg %d", code);
            return BBinder::onTransact(code, data, reply, flags);
    }
}

}; // namespace android
