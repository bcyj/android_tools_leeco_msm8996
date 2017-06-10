/*==============================================================================
*       IWiFiDisplayService.cpp
*
*  DESCRIPTION:
*       Implementation of IWiFiDisplayService
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

#include "IWiFiDisplayService.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "IWiFiDisplayService"

using namespace android;

enum {
    ENABLE_WFD = IBinder::FIRST_CALL_TRANSACTION,
    DISABLE_WFD,
    START_WFD,
    STOP_WFD,
    PLAY,
    PAUSE,
    STANDBY,
    TEARDOWN,
    SET_RTP_TRANSPORT,
    QUERY_TCP_SUPPORT,
    SET_DECODER_LATENCY,
    TCP_PLAYBACK_CTRL,
    NEGOTIATE_RTP_TRANSPORT,
    EXECUTE_RUNTIME_COMMAND,
    SET_VIDEO_SURFACE,
    GET_CFG_ITEMS,
    SET_UIBC,
    ENABLE_UIBC,
    DISABLE_UIBC,
    START_UIBC,
    STOP_UIBC,
    SEND_UIBC_EVENT,
    SET_AV_MODE,
    SET_BITRATE,
    SET_RESLN,
    GET_RESLN,
    GET_COMMON_BITMAP,
    GET_NEG_BITMAP,
};

class BpWiFiDisplayService: public BpInterface<IWiFiDisplayService>
{
public:
    BpWiFiDisplayService(const sp<IBinder>& impl)
        : BpInterface<IWiFiDisplayService>(impl)
    {
    }

    virtual int enableWfdService(const sp<IWiFiDisplayListener>& listener,
                                                WfdDevice* localDevice) {
        Parcel data, reply;
        int res = -1;
        //Don't even bother calling the service
        if(!localDevice)
        {
            CHECK_TRANSACTION(res);
        }
        data.writeInterfaceToken(IWiFiDisplayService::getInterfaceDescriptor());
        data.writeStrongBinder(listener->asBinder());
        data.writeCString(localDevice->ipaddress);
        data.writeCString(localDevice->macaddress);
        data.writeCString(localDevice->peermac);
        data.writeInt32(localDevice->SMControlPort);
        data.writeInt32(localDevice->deviceType);
        data.writeInt32(localDevice->decoderLatency);
        res = remote()->transact(ENABLE_WFD, data, &reply);
        CHECK_TRANSACTION(res);
        return reply.readInt32();
    }

    virtual int disableWfdService() {
        Parcel data, reply;
        data.writeInterfaceToken(IWiFiDisplayService::getInterfaceDescriptor());
        int res = -1;
        res = remote()->transact(DISABLE_WFD, data, &reply);
        CHECK_TRANSACTION(res);
        return reply.readInt32();
    }

    virtual int startWfdSessionService(WfdDevice *peerDevice) {
        Parcel data, reply;
        data.writeInterfaceToken(IWiFiDisplayService::getInterfaceDescriptor());
        int res = -1;
        //Don't even bother calling the service
        if(!peerDevice)
        {
            CHECK_TRANSACTION(res);
        }
        data.writeCString(peerDevice->ipaddress);
        data.writeCString(peerDevice->macaddress);
        data.writeCString(peerDevice->peermac);
        data.writeInt32(peerDevice->SMControlPort);
        data.writeInt32(peerDevice->deviceType);
        data.writeInt32(peerDevice->decoderLatency);
        res = remote()->transact(START_WFD, data, &reply);
        CHECK_TRANSACTION(res);
        return reply.readInt32();
    }

    virtual int stopWfdSessionService(int sessId) {
        Parcel data, reply;
        data.writeInterfaceToken(IWiFiDisplayService::getInterfaceDescriptor());
        int res = -1;
        data.writeInt32(sessId);
        res = remote()->transact(STOP_WFD, data, &reply);
        CHECK_TRANSACTION(res);
        return reply.readInt32();
    }

    virtual int play(int sessId, bool secureFlag ) {
        Parcel data, reply;
        data.writeInterfaceToken(IWiFiDisplayService::getInterfaceDescriptor());
        int res = -1;
        data.writeInt32(sessId);
        data.writeInt32(secureFlag);
        res = remote()->transact(PLAY, data, &reply);
        CHECK_TRANSACTION(res);
        return reply.readInt32();
    }

    virtual int pause(int sessId, bool secureFlag ) {
        Parcel data, reply;
        data.writeInterfaceToken(IWiFiDisplayService::getInterfaceDescriptor());
        int res = -1;
        data.writeInt32(sessId);
        data.writeInt32(secureFlag);
        res = remote()->transact(PAUSE, data, &reply);
        CHECK_TRANSACTION(res);
        return reply.readInt32();
    }

    virtual int standby(int sessId) {
        Parcel data, reply;
        data.writeInterfaceToken(IWiFiDisplayService::getInterfaceDescriptor());
        int res = -1;
        data.writeInt32(sessId);
        res = remote()->transact(STANDBY, data, &reply);
        CHECK_TRANSACTION(res);
        return reply.readInt32();
    }

    virtual int teardown(int sessId, bool isRTSP) {
        Parcel data, reply;
        data.writeInterfaceToken(IWiFiDisplayService::getInterfaceDescriptor());
        int res = -1;
        data.writeInt32(sessId);
        data.writeInt32(isRTSP);
        res = remote()->transact(TEARDOWN, data, &reply);
        CHECK_TRANSACTION(res);
        return reply.readInt32();
    }

    virtual int setRtpTransportType(int transportType) {
        Parcel data, reply;
        data.writeInterfaceToken(IWiFiDisplayService::getInterfaceDescriptor());
        int res = -1;
        data.writeInt32(transportType);
        res = remote()->transact(SET_RTP_TRANSPORT, data, &reply);
        CHECK_TRANSACTION(res);
        return reply.readInt32();
    }

    virtual int queryTCPSupport() {
        Parcel data, reply;
        data.writeInterfaceToken(IWiFiDisplayService::getInterfaceDescriptor());
        int res = -1;
        res = remote()->transact(QUERY_TCP_SUPPORT, data, &reply);
        CHECK_TRANSACTION(res);
        return reply.readInt32();
    }

    virtual int setDecoderLatencyValue(int latency) {
        Parcel data, reply;
        data.writeInterfaceToken(IWiFiDisplayService::getInterfaceDescriptor());
        int res = -1;
        data.writeInt32(latency);
        res = remote()->transact(SET_DECODER_LATENCY, data, &reply);
        CHECK_TRANSACTION(res);
        return reply.readInt32();
    }

    virtual int tcpPlaybackControlCmd(int cmdType, int cmdVal) {
        Parcel data, reply;
        data.writeInterfaceToken(IWiFiDisplayService::getInterfaceDescriptor());
        int res = -1;
        data.writeInt32(cmdType);
        data.writeInt32(cmdVal);
        res = remote()->transact(TCP_PLAYBACK_CTRL, data, &reply);
        CHECK_TRANSACTION(res);
        return reply.readInt32();
    }

    virtual int negotiateRtpTransportType(int TransportType,
                                    int BufferLenMs,int portNum) {
        Parcel data, reply;
        data.writeInterfaceToken(IWiFiDisplayService::getInterfaceDescriptor());
        int res = -1;
        data.writeInt32(TransportType);
        data.writeInt32(BufferLenMs);
        data.writeInt32(portNum);
        res = remote()->transact(NEGOTIATE_RTP_TRANSPORT, data, &reply);
        CHECK_TRANSACTION(res);
        return reply.readInt32();
    }

    virtual int executeRuntimeCmd(int cmd) {
        Parcel data, reply;
        data.writeInterfaceToken(IWiFiDisplayService::getInterfaceDescriptor());
        int res = -1;
        data.writeInt32(cmd);
        res = remote()->transact(EXECUTE_RUNTIME_COMMAND, data, &reply);
        CHECK_TRANSACTION(res);
        return reply.readInt32();
    }

    virtual int setVideoSurface(const sp<IGraphicBufferProducer>& surface) {
        Parcel data, reply;
        data.writeInterfaceToken(IWiFiDisplayService::getInterfaceDescriptor());
        int res = -1;
        if(surface != NULL)
        {
            data.writeStrongBinder(surface->asBinder());
        }
        res = remote()->transact(SET_VIDEO_SURFACE, data, &reply);
        CHECK_TRANSACTION(res);
        return reply.readInt32();
    }

    virtual int getConfigItems(int** configItems, size_t* length) {
        Parcel data, reply;
        data.writeInterfaceToken(IWiFiDisplayService::getInterfaceDescriptor());
        int res = -1;
        if(!length || !configItems)
        {
            //Diabolical client!
            CHECK_TRANSACTION(res);
        }
        *configItems = NULL;
        res = remote()->transact(GET_CFG_ITEMS, data, &reply);
        CHECK_TRANSACTION(res);
        res = reply.readInt32();
        if(0!= res)
        {
            return res;
        }
        //Everything's hunky dory
        *length = reply.readInt32();
        size_t tempLength = *length * sizeof(**configItems);
        char* temp = new char[tempLength];
        reply.read(temp,tempLength);
        *configItems = reinterpret_cast<int*>(temp);
        return res;
    }


    virtual int setUIBCSession(int sessId) {
        Parcel data, reply;
        data.writeInterfaceToken(IWiFiDisplayService::getInterfaceDescriptor());
        int res = -1;
        data.writeInt32(sessId);
        res = remote()->transact(SET_UIBC, data, &reply);
        CHECK_TRANSACTION(res);
        return reply.readInt32();
    }

    virtual int enableUIBCSession(int sessId) {
        Parcel data, reply;
        data.writeInterfaceToken(IWiFiDisplayService::getInterfaceDescriptor());
        int res = -1;
        data.writeInt32(sessId);
        res = remote()->transact(ENABLE_UIBC, data, &reply);
        CHECK_TRANSACTION(res);
        return reply.readInt32();
    }

    virtual int disableUIBCSession(int sessId) {
        Parcel data, reply;
        data.writeInterfaceToken(IWiFiDisplayService::getInterfaceDescriptor());
        int res = -1;
        data.writeInt32(sessId);
        res = remote()->transact(DISABLE_UIBC, data, &reply);
        CHECK_TRANSACTION(res);
        return reply.readInt32();
    }

    virtual int startUIBCDataPath() {
        Parcel data, reply;
        data.writeInterfaceToken(IWiFiDisplayService::getInterfaceDescriptor());
        int res = -1;
        res = remote()->transact(START_UIBC, data, &reply);
        CHECK_TRANSACTION(res);
        return reply.readInt32();
    }

    virtual int stopUIBCDataPath() {
        Parcel data, reply;
        data.writeInterfaceToken(IWiFiDisplayService::getInterfaceDescriptor());
        int res = -1;
        res = remote()->transact(STOP_UIBC, data, &reply);
        CHECK_TRANSACTION(res);
        return reply.readInt32();
    }

    virtual int transmitUIBCEvent(const WFD_uibc_event_t& event) {
        Parcel data, reply;
        data.writeInterfaceToken(IWiFiDisplayService::getInterfaceDescriptor());
        int res = -1;
        data.write(reinterpret_cast<void*>(const_cast<WFD_uibc_event_t*>(&event)),sizeof(event));
        res = remote()->transact(SEND_UIBC_EVENT, data, &reply);
        CHECK_TRANSACTION(res);
        return reply.readInt32();
    }

    virtual int setAVMode(int mode) {
        Parcel data, reply;
        data.writeInterfaceToken(IWiFiDisplayService::getInterfaceDescriptor());
        int res = -1;
        data.writeInt32(mode);
        res = remote()->transact(SET_AV_MODE, data, &reply);
        CHECK_TRANSACTION(res);
        return reply.readInt32();
    }

    virtual int setBitrateValue(int value) {
        Parcel data, reply;
        data.writeInterfaceToken(IWiFiDisplayService::getInterfaceDescriptor());
        int res = -1;
        data.writeInt32(value);
        res = remote()->transact(SET_BITRATE, data, &reply);
        CHECK_TRANSACTION(res);
        return reply.readInt32();
    }

    virtual int setSessionResolution(int formatType, int value, int* resParams,
                                            int len) {
        Parcel data, reply;
        data.writeInterfaceToken(IWiFiDisplayService::getInterfaceDescriptor());
        int res = -1;
        data.writeInt32(formatType);
        data.writeInt32(value);
        if(resParams)
        {
            data.writeInt32(len);
            data.write(reinterpret_cast<void*>(resParams),sizeof(*resParams)*len);
        }
        res = remote()->transact(SET_RESLN, data, &reply);
        CHECK_TRANSACTION(res);
        return reply.readInt32();
    }

    virtual int getSessionResolution(int32_t* width, int32_t* height) {
        Parcel data, reply;
        data.writeInterfaceToken(IWiFiDisplayService::getInterfaceDescriptor());
        int res = -1;
        if(!width || !height)
        {
            //Diabolical caller, don't bother server
            CHECK_TRANSACTION(res);
        }
        res = remote()->transact(GET_RESLN, data, &reply);
        CHECK_TRANSACTION(res);
        res = reply.readInt32();
        *width = reply.readInt32();
        *height = reply.readInt32();
        return res;
    }

    virtual int getCommonResolutionBitmap(uint32_t** bitmap,int32_t* numProf) {
        Parcel data, reply;
        data.writeInterfaceToken(IWiFiDisplayService::getInterfaceDescriptor());
        int res = -1;
        if(!bitmap|| !numProf)
        {
            //Diabolical caller, don't bother server
            CHECK_TRANSACTION(res);
        }
        *bitmap = NULL;
        *numProf = 0;
        res = remote()->transact(GET_COMMON_BITMAP, data, &reply);
        CHECK_TRANSACTION(res);
        res = reply.readInt32();
        if(res == 0)
        {
            *numProf = reply.readInt32();
            int readLength = 4*(*numProf)*sizeof(**bitmap);
            char* temp = new char[readLength];
            reply.read(temp,readLength);
            *bitmap = reinterpret_cast<uint32_t*>(temp);
        }
        return res;
    }

    virtual int getNegotiatedResolutionBitmap(uint32_t** bitmap) {
        Parcel data, reply;
        data.writeInterfaceToken(IWiFiDisplayService::getInterfaceDescriptor());
        int res = -1;
        if(!bitmap)
        {
            //Diabolical caller, don't bother server
            CHECK_TRANSACTION(res);
        }
        *bitmap = NULL;
        res = remote()->transact(GET_NEG_BITMAP, data, &reply);
        CHECK_TRANSACTION(res);
        res = reply.readInt32();
        if(res == 0)
        {
            int readLength = 4*sizeof(**bitmap);
            char* temp = new char[readLength];
            reply.read(temp,readLength);
            *bitmap = reinterpret_cast<uint32_t*>(temp);
        }
        return res;
    }

};

IMPLEMENT_META_INTERFACE(WiFiDisplayService, "com.qti.IWiFiDisplayService");

// ----------------------------------------------------------------------

status_t BnWiFiDisplayService::onTransact(
    uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags)
{
    switch (code) {

        case ENABLE_WFD: {
            CHECK_INTERFACE(IWiFiDisplayService, data, reply);
            sp<IWiFiDisplayListener> listener(
            interface_cast<IWiFiDisplayListener>(data.readStrongBinder()));
            WfdDevice* localDev = new WfdDevice;
            if(!localDev)
            {
                reply->writeInt32(-1);
                return NO_ERROR;
            }
            strlcpy(localDev->ipaddress,data.readCString(),sizeof(localDev->ipaddress));
            strlcpy(localDev->macaddress,data.readCString(),sizeof(localDev->macaddress));
            strlcpy(localDev->peermac,data.readCString(),sizeof(localDev->peermac));
            localDev->SMControlPort = data.readInt32();
            localDev->deviceType = data.readInt32();
            localDev->decoderLatency = data.readInt32();
            reply->writeInt32(enableWfdService(listener,localDev));
            delete localDev;
            return NO_ERROR;
        }
        break;

        case DISABLE_WFD: {
            CHECK_INTERFACE(IWiFiDisplayService, data, reply);
            reply->writeInt32(disableWfdService());
            return NO_ERROR;
        }
        break;

        case START_WFD: {
            CHECK_INTERFACE(IWiFiDisplayService, data, reply);
            WfdDevice* peerDev = new WfdDevice;
            if(!peerDev)
            {
                reply->writeInt32(-1);
                return NO_ERROR;
            }
            strlcpy(peerDev->ipaddress,data.readCString(),sizeof(peerDev->ipaddress));
            strlcpy(peerDev->macaddress,data.readCString(),sizeof(peerDev->macaddress));
            strlcpy(peerDev->peermac,data.readCString(),sizeof(peerDev->peermac));
            peerDev->SMControlPort = data.readInt32();
            peerDev->deviceType = data.readInt32();
            peerDev->decoderLatency = data.readInt32();
            reply->writeInt32(startWfdSessionService(peerDev));
            delete peerDev;
            return NO_ERROR;
        }
        break;

        case STOP_WFD: {
            CHECK_INTERFACE(IWiFiDisplayService, data, reply);
            int sessId = data.readInt32();
            reply->writeInt32(stopWfdSessionService(sessId));
            return NO_ERROR;
        }
        break;

        case PLAY: {
            CHECK_INTERFACE(IWiFiDisplayService, data, reply);
            int sessId = data.readInt32();
            bool isRTSP = data.readInt32();
            reply->writeInt32(play(sessId,isRTSP));
            return NO_ERROR;
        }
        break;

        case PAUSE: {
            CHECK_INTERFACE(IWiFiDisplayService, data, reply);
            int sessId = data.readInt32();
            bool isRTSP = data.readInt32();
            reply->writeInt32(pause(sessId,isRTSP));
            return NO_ERROR;
        }
        break;

        case STANDBY: {
            CHECK_INTERFACE(IWiFiDisplayService, data, reply);
            int sessId = data.readInt32();
            reply->writeInt32(standby(sessId));
            return NO_ERROR;
        }
        break;

        case TEARDOWN: {
            CHECK_INTERFACE(IWiFiDisplayService, data, reply);
            int sessId = data.readInt32();
            bool isRTSP = data.readInt32();
            reply->writeInt32(teardown(sessId,isRTSP));
            return NO_ERROR;
        }
        break;

        case SET_RTP_TRANSPORT: {
            CHECK_INTERFACE(IWiFiDisplayService, data, reply);
            int transportType = data.readInt32();
            reply->writeInt32(setRtpTransportType(transportType));
            return NO_ERROR;
        }
        break;

        case QUERY_TCP_SUPPORT: {
            CHECK_INTERFACE(IWiFiDisplayService, data, reply);
            reply->writeInt32(queryTCPSupport());
            return NO_ERROR;
        }
        break;

        case SET_DECODER_LATENCY: {
            CHECK_INTERFACE(IWiFiDisplayService, data, reply);
            int latency = data.readInt32();
            reply->writeInt32(setDecoderLatencyValue(latency));
            return NO_ERROR;
        }
        break;

        case TCP_PLAYBACK_CTRL: {
            CHECK_INTERFACE(IWiFiDisplayService, data, reply);
            int cmdType = data.readInt32();
            int cmdVal = data.readInt32();
            reply->writeInt32(tcpPlaybackControlCmd(cmdType,cmdVal));
            return NO_ERROR;
        }
        break;

        case NEGOTIATE_RTP_TRANSPORT: {
            CHECK_INTERFACE(IWiFiDisplayService, data, reply);
            int TransportType = data.readInt32();
            int BufferLenMs = data.readInt32();
            int portNum = data.readInt32();
            reply->writeInt32(negotiateRtpTransportType(TransportType,
                            BufferLenMs,portNum));
            return NO_ERROR;
        }
        break;

        case EXECUTE_RUNTIME_COMMAND: {
            CHECK_INTERFACE(IWiFiDisplayService, data, reply);
            int cmd = data.readInt32();
            reply->writeInt32(executeRuntimeCmd(cmd));
            return NO_ERROR;
        }
        break;

        case SET_VIDEO_SURFACE: {
            CHECK_INTERFACE(IWiFiDisplayService, data, reply);
            if(data.dataAvail() > 0)
            {
                const sp<IGraphicBufferProducer> surface(
                interface_cast<IGraphicBufferProducer>(data.readStrongBinder()));
                reply->writeInt32(setVideoSurface(surface));
            }
            else
            {
                reply->writeInt32(setVideoSurface(NULL));
            }
            return NO_ERROR;
        }
        break;

        case GET_CFG_ITEMS: {
            CHECK_INTERFACE(IWiFiDisplayService, data, reply);
            int* configItems = NULL;
            size_t length = 0;
            int ret = getConfigItems(&configItems, &length);
            reply->writeInt32(ret);
            if(0 == ret)
            {
                reply->writeInt32(length);
                reply->write(reinterpret_cast<char*>(configItems),
                    length*sizeof(*configItems));
                delete[] configItems;
            }
            return NO_ERROR;
        }
        break;

        case SET_UIBC: {
            CHECK_INTERFACE(IWiFiDisplayService, data, reply);
            int sessId = data.readInt32();
            reply->writeInt32(setUIBCSession(sessId));
            return NO_ERROR;
        }
        break;

        case ENABLE_UIBC: {
            CHECK_INTERFACE(IWiFiDisplayService, data, reply);
            int sessId = data.readInt32();
            reply->writeInt32(enableUIBCSession(sessId));
            return NO_ERROR;
        }
        break;

        case DISABLE_UIBC: {
            CHECK_INTERFACE(IWiFiDisplayService, data, reply);
            int sessId = data.readInt32();
            reply->writeInt32(disableUIBCSession(sessId));
            return NO_ERROR;
        }
        break;

        case START_UIBC: {
            CHECK_INTERFACE(IWiFiDisplayService, data, reply);
            reply->writeInt32(startUIBCDataPath());
            return NO_ERROR;
        }
        break;

        case STOP_UIBC: {
            CHECK_INTERFACE(IWiFiDisplayService, data, reply);
            reply->writeInt32(stopUIBCDataPath());
            return NO_ERROR;
        }
        break;

        case SEND_UIBC_EVENT: {
            CHECK_INTERFACE(IWiFiDisplayService, data, reply);
            WFD_uibc_event_t event;
            data.read(&event, sizeof(event));
            reply->writeInt32(transmitUIBCEvent(event));
            return NO_ERROR;
        }
        break;

        case SET_AV_MODE: {
            CHECK_INTERFACE(IWiFiDisplayService, data, reply);
            int mode = data.readInt32();
            reply->writeInt32(setAVMode(mode));
            return NO_ERROR;
        }
        break;

        case SET_BITRATE: {
            CHECK_INTERFACE(IWiFiDisplayService, data, reply);
            int value = data.readInt32();
            reply->writeInt32(setBitrateValue(value));
            return NO_ERROR;
        }
        break;

        case SET_RESLN: {
            CHECK_INTERFACE(IWiFiDisplayService, data, reply);
            int32_t formatType = data.readInt32();
            int32_t value = data.readInt32();
            int32_t * resParams = NULL;
            int len = 0;
            if(data.dataAvail()>0)
            {
                len = data.readInt32();
                resParams = new int32_t[len];
                data.read(resParams,len*sizeof(*resParams));
            }
            reply->writeInt32(setSessionResolution(formatType,value, resParams,len));
            if(resParams)
            {
                delete[] resParams;
            }
            return NO_ERROR;
        }
        break;

        case GET_RESLN: {
            CHECK_INTERFACE(IWiFiDisplayService, data, reply);
            int32_t width = 0, height =0;
            int ret = getSessionResolution(&width,&height);
            reply->writeInt32(ret);
            reply->writeInt32(width);
            reply->writeInt32(height);
            return NO_ERROR;
        }
        break;

        case GET_COMMON_BITMAP: {
            CHECK_INTERFACE(IWiFiDisplayService, data, reply);
            uint32_t* bitmap = NULL;
            int32_t numProf = 0;
            int ret = getCommonResolutionBitmap(&bitmap, &numProf);
            reply->writeInt32(ret);
            if(0 == ret)
            {
                //Everything's balmy
                reply->writeInt32(numProf);
                int writeLength = 4*numProf*sizeof(*bitmap);
                reply->write(reinterpret_cast<char*>(bitmap),
                    writeLength);
                delete[] bitmap;
            }
            return NO_ERROR;
        }
        break;

        case GET_NEG_BITMAP: {
            CHECK_INTERFACE(IWiFiDisplayService, data, reply);
            uint32_t* bitmap = NULL;
            int ret = getNegotiatedResolutionBitmap(&bitmap);
            reply->writeInt32(ret);
            if(0 == ret)
            {
                //Everything's balmy
                int writeLength = 4*sizeof(*bitmap);
                reply->write(reinterpret_cast<char*>(bitmap),
                    writeLength);
                delete[] bitmap;
            }
            return NO_ERROR;
        }
        break;

        default:
            return BBinder::onTransact(code, data, reply, flags);
    }
}
