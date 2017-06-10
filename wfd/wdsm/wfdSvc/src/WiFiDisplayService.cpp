/*==============================================================================
*       WiFiDisplayService.cpp
*
*  DESCRIPTION:
*       Service side implementation of WiFiDisplayService
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

#include "WiFiDisplayService.h"
#include <utils/Log.h>
#include <binder/IServiceManager.h>
#include <cutils/properties.h>
#include "wifidisplay.h"
#include <gui/IGraphicBufferProducer.h>
#include <gui/Surface.h>

#ifdef LOG_TAG
#undef LOG_TAG
#define LOG_TAG "WiFiDisplayService"
#endif

WiFiDisplayService* WiFiDisplayService::sWiFiDisplayService = NULL;
static const char* wfdPermission = "com.qualcomm.permission.wfd.QC_WFD";

WiFiDisplayService::WiFiDisplayService()
{
    ALOGE("Bon jour from WiFiDisplayService");
    videoSurface = NULL;
}

WiFiDisplayService::~WiFiDisplayService()
{
    ALOGE("Adios from ~WiFiDisplayService");
}

void WiFiDisplayService::publishService()
{
    if(sWiFiDisplayService == NULL)
    {
        sWiFiDisplayService = new WiFiDisplayService();
    }

    defaultServiceManager()->addService(String16("wfdservice"),
                            sWiFiDisplayService);

    if(defaultServiceManager()->checkService(
                     String16("wfdservice"))== NULL)
    {
        ALOGE("Adding WiFiDisplayService Failed!!");
    }
    else
    {
        ALOGE("Added WiFiDisplayService");
    }

}

int WiFiDisplayService::enableWfdService(
    const sp<IWiFiDisplayListener>& listener,
                          WfdDevice *localDevice)
{
    ALOGE("WiFiDisplayService::enableWfdService check permission");
    bool allowed = checkCallingPermission(String16(wfdPermission));

    if(!allowed)
    {
        ALOGE("Caller requires %s to interact with WiFiDisplayService",
            wfdPermission);
        return -1;
    }

    mListener = listener;
    registerUibcCallbacks(NULL,&sendGenericEvent,&sendHIDEvent);
    return enableWfd(localDevice,&StringArrayCallback);
}

int WiFiDisplayService::disableWfdService()
{
    if(videoSurface != NULL)
    {
        videoSurface.clear();
        videoSurface = NULL;
    }
    return disableWfd();
}

int WiFiDisplayService::startWfdSessionService(WfdDevice *peerDevice)
{
    int res = -1;
    startWfdSession(peerDevice);
    return 0;
}

int WiFiDisplayService::stopWfdSessionService(int sessId)
{
    int res = -1;
    stopWfdSession(sessId);
    return 0;
}

int WiFiDisplayService::play(int sessId, bool secureFlag )
{
    play_rtsp(sessId,secureFlag);
    return 0;
}

int WiFiDisplayService::pause(int sessId, bool secureFlag )
{
    pause_rtsp(sessId,secureFlag);
    return 0;
}

int WiFiDisplayService::standby(int sessId)
{
    return standby_rtsp(sessId);
}

int WiFiDisplayService::teardown(int sessId, bool isRTSP)
{
    teardown_rtsp(sessId,isRTSP);
    return 0;
}

int WiFiDisplayService::setRtpTransportType(int transportType)
{
    setRtpTransport(transportType);
    return 0;
}

int WiFiDisplayService::queryTCPSupport()
{
    queryTCPTransportSupport();
    return 0;
}

int WiFiDisplayService::setDecoderLatencyValue(int latency)
{
    setDecoderLatency(latency);
    return 0;
}

int WiFiDisplayService::tcpPlaybackControlCmd(int cmdType, int cmdVal)
{
    tcpPlaybackControl(cmdType,cmdVal);
    return 0;
}

int WiFiDisplayService::negotiateRtpTransportType(int TransportType,
             int BufferLenMs,int portNum)
{
    negotiateRtpTransport(TransportType,BufferLenMs,portNum);
    return 0;
}

int WiFiDisplayService::executeRuntimeCmd(int cmd)
{
    return executeRuntimeCommand(cmd);
}

int WiFiDisplayService::setVideoSurface(const sp<IGraphicBufferProducer>& surface)
{
    if(videoSurface != NULL)
    {
        //Release reference to previous surface, if any
        videoSurface.clear();
        videoSurface = NULL;
    }

    if(surface != NULL)
    {
        ALOGE("Instantiating a new surface");
        videoSurface = new Surface(surface);
    }

    if(videoSurface != NULL)
    {
        setSurface(videoSurface.get());
    }
    else
    {
        setSurface(NULL);
    }
    return 0;
}

int WiFiDisplayService::getConfigItems(int** configItems, size_t* length)
{
    return getCfgItems(configItems,length);
}

int WiFiDisplayService::setUIBCSession(int sessId)
{
    return setUIBC(sessId);
}

int WiFiDisplayService::enableUIBCSession(int sessId)
{
    return enableUIBC(sessId);
}

int WiFiDisplayService::disableUIBCSession(int sessId)
{
    return disableUIBC(sessId);
}

int WiFiDisplayService::startUIBCDataPath()
{
    startUIBC();
    return 0;
}

int WiFiDisplayService::stopUIBCDataPath()
{
    stopUIBC();
    return 0;
}

int WiFiDisplayService::transmitUIBCEvent(const WFD_uibc_event_t& event)
{
    return sendUIBCEvent(const_cast<WFD_uibc_event_t*>(&event));
}

int WiFiDisplayService::setAVMode(int mode)
{
    return setAVPlaybackMode(static_cast<AVPlaybackMode>(mode));
}

int WiFiDisplayService::setBitrateValue(int value)
{
    setBitrate(value);
    return 0;
}

int WiFiDisplayService::setSessionResolution(int formatType, int value,
                                                int* resParams, int len __unused)
{
    return setResolution(formatType,value,resParams);
}

int WiFiDisplayService::getSessionResolution(int32_t* width, int32_t* height)
{
    return getResolution(width,height);
}


int WiFiDisplayService::getCommonResolutionBitmap(uint32_t** bitmap,
                                                int32_t* numProf)
{
    if(bitmap && numProf)
    {
        *bitmap = NULL;
        *bitmap = getCommonResolution(numProf);
        if(bitmap)
        {
            return 0;
        }
    }
    return -1;
}

int WiFiDisplayService::getNegotiatedResolutionBitmap(uint32_t** bitmap)
{
    if(bitmap)
    {
        *bitmap = NULL;
        *bitmap = getNegotiatedResolution();
        return 0;
    }
    return -1;
}

void WiFiDisplayService::StringArrayCallback(const char* eName,
                                int numObjects, char strArray[][256])
{
    ALOGD("StringArrayCallback eName= %s  numObjects=%d", eName, numObjects);
    for (int i=0; i<numObjects; i++)
    {
        ALOGE("\t strArray[%d] = \"%s\"", i, strArray[i]);
    }

    if(numObjects >= 4 && !strcmp(eName,MM_UPDATE_EVENT))
    {
        //Make sure that is a MM_UPDATE_EVENT with the requisite number of objects
        //and then check if its a MMStreamStarted event to handle it a bit differently
        if(!strcmp(strArray[0],"MMStreamStarted"))
        {
            ALOGE("Received MMStreamStarted with IGBP");
            long surface = atol((char*)strArray[numObjects - 1]);
            const sp<IGraphicBufferProducer>& surfaceProducer
                (reinterpret_cast<IGraphicBufferProducer*>(surface));
            Parcel obj;
            ALOGE("Writing surface %p to parcel", surfaceProducer.get());
            obj.writeStrongBinder(surfaceProducer->asBinder());
            sWiFiDisplayService->mListener->notify(eName,numObjects,strArray,&obj);
        }
    }
    else//Normal callbacks
    {
        sWiFiDisplayService->mListener->notify(eName,numObjects,strArray);
    }
}

boolean WiFiDisplayService::sendGenericEvent(WFD_uibc_event_t * ev,void * pClientData)
{
    sWiFiDisplayService->mListener->notifyUIBCGenericEvent(ev,pClientData );
    return true;
}

boolean WiFiDisplayService::sendHIDEvent(uint8* HIDPack, uint8 len, HIDDataType type)
{
    sWiFiDisplayService->mListener->notifyUIBCHIDEvent(HIDPack, len,type);
    return true;
}
