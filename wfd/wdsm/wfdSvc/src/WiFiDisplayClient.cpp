/*==============================================================================
*       WiFiDisplayClient.cpp
*
*  DESCRIPTION:
*       Implementation of WiFiDisplayClient
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

#include "WiFiDisplayClient.h"
#include <binder/IServiceManager.h>
#include <gui/IGraphicBufferProducer.h>
#include <gui/Surface.h>

#define MM_UPDATE_EVENT         "MMEvent"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "WiFiDisplayClient"

sp<IWiFiDisplayService> WiFiDisplayClient::sWiFiDisplayService = NULL;
WiFiDisplayClient* WiFiDisplayClient::spMe = NULL;
sp<WiFiDisplayClient::WiFiDisplayServiceDeathRecepient> WiFiDisplayClient::sDeathNotifier = NULL;
WiFiDisplayClient::stringarray_callback WiFiDisplayClient::clientCallback = NULL;
wfd_uibc_send_event_cb WiFiDisplayClient::genericCallback = NULL;
wfd_uibc_hid_event_cb WiFiDisplayClient::hidCallback = NULL;

const sp<IWiFiDisplayService>& WiFiDisplayClient::getWiFiDisplayService()
{
    if(sWiFiDisplayService == NULL)
    {
        sp<IBinder> binder = defaultServiceManager()->
            getService(String16("wfdservice"));
        if(binder == NULL)
        {
            ALOGE("wfdservice not available!");
            return NULL;
        }

        if(sDeathNotifier != NULL)
        {
            sDeathNotifier.clear();
            sDeathNotifier = NULL;
        }

        sDeathNotifier = new WiFiDisplayServiceDeathRecepient();
        binder->linkToDeath(sDeathNotifier);

        sWiFiDisplayService = interface_cast<IWiFiDisplayService>(binder);
    }
    return sWiFiDisplayService;
}

WiFiDisplayClient::WiFiDisplayServiceDeathRecepient::~WiFiDisplayServiceDeathRecepient()
{
    ALOGE("~WiFiDisplayServiceDeathRecepient()");
    if (sWiFiDisplayService != NULL)
    {
        ALOGE("Unlinking Death Notifier from sWiFiDisplayService");
        sWiFiDisplayService->asBinder()->unlinkToDeath(this);
    }
}

WiFiDisplayClient::~WiFiDisplayClient()
{
    ALOGE("~WiFiDisplayClient");
    spMe = NULL;
}

void WiFiDisplayClient::WiFiDisplayServiceDeathRecepient::binderDied(const wp<IBinder>& who) {
    ALOGE("WiFiDisplayService %p died!", who.unsafe_get());
    sWiFiDisplayService.clear();
    sWiFiDisplayService = NULL;
}

int WiFiDisplayClient::notify(const char* eName, int numObjects,
        char strArray[][256], const Parcel* obj)
{

    ALOGE("notify in client process");
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
            if(obj)
            {
                ALOGE("Received MMStreamStarted with IGBP");
                const sp<IGraphicBufferProducer>& surface(
                   interface_cast<IGraphicBufferProducer>(obj->readStrongBinder()));
                ALOGE("Received surface with %p", surface.get());
                //The string entry written from the service doesn't really
                //make sense in the client process, re-populate it with
                //the correct IGBP
                snprintf(strArray[4],256,"%ld", reinterpret_cast<long>
                                                (surface.get()));
                if(clientCallback)
                {
                    clientCallback(eName,numObjects,strArray);
                }
                return 0;
            }
        }
    }

    if(clientCallback)
    {
        clientCallback(eName,numObjects,strArray);
    }
    return 0;
}

int WiFiDisplayClient::notifyUIBCGenericEvent(WFD_uibc_event_t* ev,
    void * pClientData )
{
    genericCallback(ev,pClientData);
    return 0;
}

int WiFiDisplayClient::notifyUIBCHIDEvent(uint8* HIDPack, uint8 len,
    HIDDataType type)
{
    hidCallback(HIDPack,len,type);
    return 0;
}

int WiFiDisplayClient::enableWfd(WfdDevice *localDevice,stringarray_callback cb)
{
    if(spMe == NULL)
    {
        spMe = static_cast<WiFiDisplayClient*>(new WiFiDisplayClient);
    }

    clientCallback = cb;

    const sp<IWiFiDisplayService>& pWfdSvc = getWiFiDisplayService();

    if (pWfdSvc != NULL)
    {
        return pWfdSvc->enableWfdService(spMe,localDevice);
    }
    return -1;
}

int WiFiDisplayClient::disableWfd()
{
    const sp<IWiFiDisplayService>& pWfdSvc = getWiFiDisplayService();
    if (pWfdSvc != NULL)
    {
        return pWfdSvc->disableWfdService();
    }
    return -1;
}

int WiFiDisplayClient::startWfdSession(WfdDevice *peerDevice)
{
    const sp<IWiFiDisplayService>& pWfdSvc = getWiFiDisplayService();
    if (pWfdSvc != NULL)
    {
        return pWfdSvc->startWfdSessionService(peerDevice);
    }
    return -1;
}

int WiFiDisplayClient::stopWfdSession(int sessId)
{
    const sp<IWiFiDisplayService>& pWfdSvc = getWiFiDisplayService();
    if (pWfdSvc != NULL)
    {
        return pWfdSvc->stopWfdSessionService(sessId);
    }
    return -1;
}

int WiFiDisplayClient::play_rtsp(int sessId, bool secureFlag)
{
    const sp<IWiFiDisplayService>& pWfdSvc = getWiFiDisplayService();
    if (pWfdSvc != NULL)
    {
        return pWfdSvc->play(sessId,secureFlag);
    }
    return -1;
}

int WiFiDisplayClient::pause_rtsp(int sessId, bool secureFlag)
{
    const sp<IWiFiDisplayService>& pWfdSvc = getWiFiDisplayService();
    if (pWfdSvc != NULL)
    {
        return pWfdSvc->pause(sessId,secureFlag);
    }
    return -1;
}

int WiFiDisplayClient::standby_rtsp(int sessId)
{
    const sp<IWiFiDisplayService>& pWfdSvc = getWiFiDisplayService();
    if (pWfdSvc != NULL)
    {
        return pWfdSvc->standby(sessId);
    }
    return -1;
}

int WiFiDisplayClient::teardown_rtsp(int sessId, bool isRTSP)
{
    const sp<IWiFiDisplayService>& pWfdSvc = getWiFiDisplayService();
    if (pWfdSvc != NULL)
    {
        return pWfdSvc->teardown(sessId,isRTSP);
    }
    return -1;
}

int WiFiDisplayClient::setRtpTransport(int transportType)
{
    const sp<IWiFiDisplayService>& pWfdSvc = getWiFiDisplayService();
    if (pWfdSvc != NULL)
    {
        return pWfdSvc->setRtpTransportType(transportType);
    }
    return -1;
}

int WiFiDisplayClient::queryTCPTransportSupport()
{
    const sp<IWiFiDisplayService>& pWfdSvc = getWiFiDisplayService();
    if (pWfdSvc != NULL)
    {
        return pWfdSvc->queryTCPSupport();
    }
    return -1;
}

int WiFiDisplayClient::setDecoderLatency(int latency)
{
    const sp<IWiFiDisplayService>& pWfdSvc = getWiFiDisplayService();
    if (pWfdSvc != NULL)
    {
        return pWfdSvc->setDecoderLatencyValue(latency);
    }
    return -1;
}

int WiFiDisplayClient::tcpPlaybackControl(int cmdType, int cmdVal)
{
    const sp<IWiFiDisplayService>& pWfdSvc = getWiFiDisplayService();
    if (pWfdSvc != NULL)
    {
        return pWfdSvc->tcpPlaybackControlCmd(cmdType, cmdVal);
    }
    return -1;
}

int WiFiDisplayClient::negotiateRtpTransport(int TransportType,int BufferLenMs,
            int portNum)
{
    const sp<IWiFiDisplayService>& pWfdSvc = getWiFiDisplayService();
    if (pWfdSvc != NULL)
    {
        return pWfdSvc->negotiateRtpTransportType(TransportType,BufferLenMs,
                                                    portNum);
    }
    return -1;
}

int WiFiDisplayClient::executeRuntimeCommand(int cmd)
{
    const sp<IWiFiDisplayService>& pWfdSvc = getWiFiDisplayService();
    if (pWfdSvc != NULL)
    {
        return pWfdSvc->executeRuntimeCmd(cmd);
    }
    return -1;
}

int WiFiDisplayClient::setSurface(Surface* surface)
{
    const sp<IWiFiDisplayService>& pWfdSvc = getWiFiDisplayService();
    if (pWfdSvc != NULL)
    {
        if(Surface::isValid(surface))
        {
            return pWfdSvc->setVideoSurface(surface->getIGraphicBufferProducer());
        }
        else
        {
            return pWfdSvc->setVideoSurface(NULL);
        }
    }
    return -1;
}

int WiFiDisplayClient::getConfigItems(int** configItems, size_t* length)
{
    const sp<IWiFiDisplayService>& pWfdSvc = getWiFiDisplayService();
    if (pWfdSvc != NULL)
    {
        return pWfdSvc->getConfigItems(configItems,length);
    }
    return -1;
}

int WiFiDisplayClient::setUIBC(int sessId)
{
    const sp<IWiFiDisplayService>& pWfdSvc = getWiFiDisplayService();
    if (pWfdSvc != NULL)
    {
        return pWfdSvc->setUIBCSession(sessId);
    }
    return -1;
}

void WiFiDisplayClient::registerUibcCallbacks(wfd_uibc_send_event_cb sendEvent,
                                wfd_uibc_hid_event_cb sendHIDEvent)
{
    genericCallback = sendEvent;
    hidCallback = sendHIDEvent;
}

int WiFiDisplayClient::enableUIBC(int sessId)
{
    const sp<IWiFiDisplayService>& pWfdSvc = getWiFiDisplayService();
    if (pWfdSvc != NULL)
    {
        return pWfdSvc->enableUIBCSession(sessId);
    }
    return -1;
}


int WiFiDisplayClient::disableUIBC(int sessId)
{
    const sp<IWiFiDisplayService>& pWfdSvc = getWiFiDisplayService();
    if (pWfdSvc != NULL)
    {
        return pWfdSvc->disableUIBCSession(sessId);
    }
    return -1;
}

int WiFiDisplayClient::startUIBC()
{
    const sp<IWiFiDisplayService>& pWfdSvc = getWiFiDisplayService();
    if (pWfdSvc != NULL)
    {
        return pWfdSvc->startUIBCDataPath();
    }
    return -1;
}

int WiFiDisplayClient::stopUIBC()
{
    const sp<IWiFiDisplayService>& pWfdSvc = getWiFiDisplayService();
    if (pWfdSvc != NULL)
    {
        return pWfdSvc->stopUIBCDataPath();
    }
    return -1;
}

int WiFiDisplayClient::sendUIBCEvent(const WFD_uibc_event_t& event)
{
    const sp<IWiFiDisplayService>& pWfdSvc = getWiFiDisplayService();
    if (pWfdSvc != NULL)
    {
        return pWfdSvc->transmitUIBCEvent(event);
    }
    return -1;
}

int WiFiDisplayClient::setAVPlaybackMode(int mode)
{
    const sp<IWiFiDisplayService>& pWfdSvc = getWiFiDisplayService();
    if (pWfdSvc != NULL)
    {
        return pWfdSvc->setAVMode(mode);
    }
    return -1;
}

int WiFiDisplayClient::setBitrate(int value)
{
    const sp<IWiFiDisplayService>& pWfdSvc = getWiFiDisplayService();
    if (pWfdSvc != NULL)
    {
        return pWfdSvc->setBitrateValue(value);
    }
    return -1;
}

int WiFiDisplayClient::setResolution(int formatType, int value, int* resParams, int len)
{
    const sp<IWiFiDisplayService>& pWfdSvc = getWiFiDisplayService();
    if (pWfdSvc != NULL)
    {
        return pWfdSvc->setSessionResolution(formatType,value,resParams,len);
    }
    return -1;
}

int WiFiDisplayClient::getResolution(int32_t * width,int32_t * height)
{
    const sp<IWiFiDisplayService>& pWfdSvc = getWiFiDisplayService();
    if (pWfdSvc != NULL)
    {
        return pWfdSvc->getSessionResolution(width,height);
    }
    return -1;
}

int WiFiDisplayClient::getCommonResolution(uint32_t** bitmap, int32_t* numProf)
{
    const sp<IWiFiDisplayService>& pWfdSvc = getWiFiDisplayService();
    if (pWfdSvc != NULL)
    {
        return pWfdSvc->getCommonResolutionBitmap(bitmap,numProf);
    }
    return -1;
}


int WiFiDisplayClient::getNegotiatedResolution(uint32_t** bitmap)
{
    const sp<IWiFiDisplayService>& pWfdSvc = getWiFiDisplayService();
    if (pWfdSvc != NULL)
    {
        return pWfdSvc->getNegotiatedResolutionBitmap(bitmap);
    }
    return -1;
}
