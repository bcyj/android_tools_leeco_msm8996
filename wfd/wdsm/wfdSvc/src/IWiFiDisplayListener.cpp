/*==============================================================================
*       IWiFiDisplayListener.cpp
*
*  DESCRIPTION:
*       Implementation of IWiFiDisplayListener
*
*
*  Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
*  Qualcomm Technologies Proprietary and Confidential.
*===============================================================================
*/
/*==============================================================================
                             Edit History
================================================================================
   When            Who           Why
-----------------  ------------  -----------------------------------------------
11/06/2014                    InitialDraft
================================================================================
*/

/*==============================================================================
**               Includes and Public Data Declarations
**==============================================================================
*/

/* =============================================================================

                     INCLUDE FILES FOR MODULE

================================================================================
*/

#include <stdint.h>
#include <sys/types.h>

#include "IWiFiDisplayListener.h"

#define WIFIDISPLAY_STRING_ARR_SIZE 50
#define WIFIDISPLAY_STRING_SIZE     256

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "IWiFiDisplayListener"

using namespace android;

enum {
    NOTIFY = IBinder::FIRST_CALL_TRANSACTION,
    NOTIFY_UIBC_GENERIC,
    NOTIFY_UIBC_HID,
};

class BpWiFiDisplayListener: public BpInterface<IWiFiDisplayListener>
{
public:
    BpWiFiDisplayListener(const sp<IBinder>& impl)
        : BpInterface<IWiFiDisplayListener>(impl)
    {
    }

    virtual int notify(const char* eName, int numObjects,
        char strArray[][256], const Parcel* obj) {
        Parcel data, reply;
        int res = -1;
        if(!eName)//Don't even trouble the server
        {
            CHECK_TRANSACTION(res);
        }
        data.writeInterfaceToken(IWiFiDisplayListener::getInterfaceDescriptor());
        data.writeCString(eName);
        data.writeInt32(numObjects);
        bool block = false;
        for(int i = 0; i< numObjects; i++)
        {
            data.writeCString(strArray[i]);
            if(i==1 && strcmp(strArray[i],"PAUSE")==0)
            {
                //Ugly hack :(
                ALOGE("Blocking PAUSE START Callback",strArray[1]);
                block = true;
            }
        }
        if (obj && obj->dataSize() > 0)
        {
            ALOGE("Writing parcel extra with %d size",obj->dataSize());
            data.appendFrom(const_cast<Parcel*>(obj), 0, obj->dataSize());
        }
        res = remote()->transact(NOTIFY, data, &reply,block?0:IBinder::FLAG_ONEWAY);
        CHECK_TRANSACTION(res);
        return reply.readInt32();
    }

    virtual int notifyUIBCGenericEvent(WFD_uibc_event_t* ev,
        void * pClientData __unused) {
        Parcel data, reply;
        int res = -1;
        if(!ev)//Don't be inveigled
        {
            CHECK_TRANSACTION(res);
        }
        data.writeInterfaceToken(IWiFiDisplayListener::getInterfaceDescriptor());
        data.write(reinterpret_cast<void*>(ev),sizeof(*ev));
        // TODO: Client data is not delivered  as of now
        res = remote()->transact(NOTIFY_UIBC_GENERIC, data, &reply,IBinder::FLAG_ONEWAY);
        CHECK_TRANSACTION(res);
        return reply.readInt32();
    }

    virtual int notifyUIBCHIDEvent(uint8* HIDPack, uint8 len,
        HIDDataType type) {
        Parcel data, reply;
        int res = -1;
        if(!HIDPack)
        {
            CHECK_TRANSACTION(res);
        }
        data.writeInterfaceToken(IWiFiDisplayListener::getInterfaceDescriptor());
        data.writeInt32(static_cast<int32_t>(len));
        data.write(reinterpret_cast<void*>(HIDPack),len);
        data.writeInt32(static_cast<int32_t>(type));
        res = remote()->transact(NOTIFY_UIBC_HID, data, &reply,IBinder::FLAG_ONEWAY);
        CHECK_TRANSACTION(res);
        return reply.readInt32();
    }


};

IMPLEMENT_META_INTERFACE(WiFiDisplayListener, "com.qti.IWiFiDisplayListener");

// ----------------------------------------------------------------------

int BnWifiDisplayListener::onTransact(
    uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags)
{
    switch (code) {
        case NOTIFY: {
            CHECK_INTERFACE(IWiFiDisplayListener, data, reply);
            const char* eName;
            int numObjects;
            char strArray[WIFIDISPLAY_STRING_ARR_SIZE][WIFIDISPLAY_STRING_SIZE];
            eName = data.readCString();
            numObjects = data.readInt32();
            for(int i = 0; i< numObjects; i++)
            {
                strlcpy(strArray[i],data.readCString(), sizeof(strArray[i]));
            }
            Parcel obj;
            if (data.dataAvail() > 0)
            {
                ALOGE("Received an extra parcel");
                obj.appendFrom(const_cast<Parcel *>(&data), data.dataPosition(),
                    data.dataAvail());
                ALOGE("DataPosistion after appending %d is %d", data.dataAvail(), obj.dataPosition());
                obj.setDataPosition(0);//Reset it so that client is able to read off
            }
            reply->writeInt32(notify(eName,numObjects,strArray,&obj));
            return NO_ERROR;
        }
        break;

        case NOTIFY_UIBC_GENERIC: {
            CHECK_INTERFACE(IWiFiDisplayListener, data, reply);
            WFD_uibc_event_t event;
            data.read(&event, sizeof(event));
            // TODO: Client data is delivered  NULL for now
            reply->writeInt32(notifyUIBCGenericEvent(&event,NULL));
            return NO_ERROR;
        }
        break;

        case NOTIFY_UIBC_HID: {
            CHECK_INTERFACE(IWiFiDisplayListener, data, reply);
            uint8 len = static_cast<uint8>(data.readInt32());
            uint8* HIDPack = new uint8[len];
            data.read(HIDPack,len);
            HIDDataType type = static_cast<HIDDataType>(data.readInt32());
            reply->writeInt32(notifyUIBCHIDEvent(HIDPack,len,type));
            return NO_ERROR;
        }
        break;

        default:
            return BBinder::onTransact(code, data, reply, flags);
    }
}
