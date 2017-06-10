/*==============================================================================
 *  @file MMAdaptor.cpp
 *
 *  @par DESCRIPTION:
 *       Session Manager, MM Module Interface
 *
 *
 *  Copyright (c) 2012 - 2014 by Qualcomm Technologies, Incorporated.
 *  All Rights Reserved.
 *  Qualcomm Technologies Proprietary and Confidential.
==============================================================================*/

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_NDEBUG 0
#define LOG_NDDEBUG 0

#include <utils/Log.h>
#ifndef WFD_ICS
#include "common_log.h"
#endif
#include <cutils/properties.h>
#include "SessionManager.h"
#include "MMAdaptor.h"
#include "RTSPSession.h"
#include "MMCapability.h"
#include "wfd_netutils.h"
#include "wifidisplay.h"

//Generic Data buffer type must have same structure every layer
typedef struct mmGenericDataBuf
{
    char *pData;
    unsigned int len;
}mmGenericDataBufType;

typedef struct mmSessionInfo
{
    int height;
    int width;
    int hdcp;
    long producer;
    //Add audio params as needed
}mmSessionInfo;

/* Initialize static member variables */
WFD_MM_HANDLE MMAdaptor::mmHandle = NULL;

bool MMAdaptor::mmCreated = false;
bool MMAdaptor::mmHdcpCreated = false;
DeviceType MMAdaptor::mmDevType = UNKNOWN;

WFD_MM_callbacks_t MMAdaptor::mmCallbacks = {
                                             MMAdaptor::capability_change_cb,
                                             MMAdaptor::idr_trigger_cb,
                                             MMAdaptor::av_format_change_cb,
                                             MMAdaptor::event_update_cb
                                            };

MMEventStatusType convertWfdToMMStatus(WFD_status_t status){

  switch (status) {
  case WFD_STATUS_SUCCESS:
    return MM_STATUS_SUCCESS;
  case WFD_STATUS_FAIL:
    return MM_STATUS_FAIL;
  case WFD_STATUS_NOTSUPPORTED:
    return MM_STATUS_NOTSUPPORTED;
  case WFD_STATUS_BADPARAM:
    return MM_STATUS_BADPARAM;
  case WFD_STATUS_MEMORYFAIL:
    return MM_STATUS_MEMORYFAIL;
  case WFD_STATUS_RUNTIME_ERROR:
    return MM_STATUS_RUNTIME_ERROR;
  case WFD_STATUS_READY:
    return MM_STATUS_READY;
  case WFD_STATUS_CONNECTED:
    return MM_STATUS_CONNECTED;
  case WFD_STATUS_PROXY_OPENED:
    return MM_STATUS_SUCCESS;
  case WFD_STATUS_PROXY_CLOSED:
    return MM_STATUS_SUCCESS;
  case WFD_STATUS_INVALID:
    return MM_STATUS_INVALID;
  default:
    return MM_STATUS_INVALID;
  }
}

MMAdaptor::MMAdaptor() {
}

MMAdaptor::~MMAdaptor() {
}


/**
 * Query MM library and update MMCapability
 * @param pLocalMMCapability
 * @param devType
 *
 * @return bool
 */
bool MMAdaptor::updateLocalMMCapability(MMCapability* pLocalMMCapability, DeviceType devType) {

    MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"Updating local MM capability");
    wfd_mm_get_local_capability(mmHandle, pLocalMMCapability->pCapability, deviceType_conversion(devType));

    return true;
}


bool MMAdaptor::getNegotiatedCapability(MMCapability* pLocalMMCapability,
                                        MMCapability* pPeerMMCapability,
                                        MMCapability* pNegotiatedMMCapability,
                                        MMCapability* pCommonCapability)
{

    wfd_mm_get_negotiated_capability(mmHandle,
                                     pLocalMMCapability->pCapability,
                                     pPeerMMCapability->pCapability,
                                     pNegotiatedMMCapability->pCapability,
                                     pCommonCapability->pCapability);
    return true;
}


/**
 * Get proposed capability from MM lib for capability
 * renegotiation
 * @param pLocalMMCapability
 * @param pPeerMMCapability
 * @param pNegotiatedMMCapability
 *
 * @return bool
 */
bool MMAdaptor::getProposedCapability(MMCapability* pLocalMMCapability, MMCapability* pPeerMMCapability, MMCapability* pNegotiatedMMCapability) {

    MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"Get Proposed MMCapability from MM lib");
    #ifndef NO_MM_LIB
    if(wfd_mm_get_proposed_capability(mmHandle, pLocalMMCapability->pCapability, pPeerMMCapability->pCapability, pNegotiatedMMCapability->pCapability) != WFD_STATUS_SUCCESS)
    {
       return false ;
    }
    #endif

    #if 1
    MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"Local MMCapability:");
    pLocalMMCapability->dump();
    MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"Peer MMCapability:");
    pPeerMMCapability->dump();
    MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"Proposed MMCapability:");
    pNegotiatedMMCapability->dump();
    #endif

    return true;
}


bool MMAdaptor::createSession(MMCapability* pMMCapability, DeviceType devType) {
    if (!isMMStreamEnabled()) {
        return true;
    }

    if (!mmCreated) {
        MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"Calling wfd_mm_create_session()");
        WFD_status_t ret = wfd_mm_create_session(&mmHandle,
                                                 deviceType_conversion(devType),
                                                 pMMCapability->pCapability,
                                                 &mmCallbacks);
        MM_MSG_PRIO1(MM_GENERAL,MM_PRIO_HIGH,"Calling wfd_mm_create_session() done."
                                             "  ret=%d", ret);
        if (ret==WFD_STATUS_SUCCESS) {
            mmCreated = true;
            return true;
        }
    } else {
        MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"MM session is already created.");
    }

    return false;
}

bool MMAdaptor::createHDCPSession(DeviceType devType,
	                               MMCapability* pLocalMMCapability,
	                               MMCapability* pPeerMMCapability,
	                               MMCapability* pNegotiatedMMCapability )
{
  bool bStatus = false;
  MM_MSG_PRIO1(MM_GENERAL,MM_PRIO_HIGH,"createHDCPSession:DeviceType: %d",devType);

#ifndef NO_MM_LIB
  mmHandle = NULL;
  WFD_status_t ret = wfd_mm_create_HDCP_session(&mmHandle,
                                deviceType_conversion(devType),
                                pLocalMMCapability->pCapability,
                                pPeerMMCapability->pCapability,
                                pNegotiatedMMCapability->pCapability);
  if(ret == WFD_STATUS_SUCCESS)
  {
    mmHdcpCreated = true;
    mmDevType = devType;
    MM_MSG_PRIO1(MM_GENERAL,MM_PRIO_HIGH,"createHDCPSession: success for DeviceType %d",mmDevType);

    return true ;
  }
#endif

  return bStatus;
}

bool MMAdaptor::updateSession(MMCapability* pMMCapability) {
    if (!isMMStreamEnabled()) {
        return true;
    }

    #ifndef NO_MM_LIB
    if (mmCreated) {
        MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"Calling updateSession()");
        WFD_status_t ret = wfd_mm_update_session(mmHandle, pMMCapability->pCapability, NULL);
        MM_MSG_PRIO1(MM_GENERAL,MM_PRIO_HIGH,"Calling updateSession() done.  ret=%d", ret);
        if (ret==WFD_STATUS_SUCCESS) {
            return true;
        } else {
            MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"Failed to update MM session.");
        }
    } else {
        MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"Calling updateSession() failed.  MM hasn't been created yet.");
    }
    #endif
    return false;
}


bool MMAdaptor::destroySession() {
    if (!isMMStreamEnabled()) {
        return true;
    }

    #ifndef NO_MM_LIB
    if (mmCreated) {
        mmCreated = false;
        mmHdcpCreated = false;
        mmDevType = UNKNOWN;

        MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"Calling mm_destroy_session()");
        WFD_status_t ret = wfd_mm_destroy_session(mmHandle);
        mmHandle = NULL;
        MM_MSG_PRIO1(MM_GENERAL,MM_PRIO_HIGH,"Calling mm_destroy_session() done.  ret=%d", ret);
        if (ret==WFD_STATUS_SUCCESS) {
            return true;
        } else {
            MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"Failed to destroy MM session.");
        }
    } else {
        MM_MSG_PRIO2(MM_GENERAL,MM_PRIO_HIGH,"HDCP status mmHDCPCreated %d mmDevType %d",mmHdcpCreated,mmDevType);
        if(mmHdcpCreated)
        {
            mmHdcpCreated = false;
            wfd_mm_destroy_hdcp_session(NULL,deviceType_conversion(mmDevType));
            mmDevType = UNKNOWN;
        }
        if(mmHandle) {
            MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"Cleaning up MM");
            WFD_status_t ret = wfd_mm_destroy_session(mmHandle);
            mmHandle = NULL;
            mmDevType = UNKNOWN;
            if (ret == WFD_STATUS_SUCCESS) {
                return true;
            } else {
                MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"Failed to destroy MM session.");
            }
        }
    }
    #endif
    return false;
}

bool MMAdaptor::streamPlay() {
    if (!isMMStreamEnabled()) {
        return true;
    }

    #ifndef NO_MM_LIB
    if (mmCreated) {
        MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"Calling wfd_mm_play()");
        WFD_status_t ret = wfd_mm_stream_play(mmHandle, WFD_STREAM_AV, &stream_play_cb);
        MM_MSG_PRIO1(MM_GENERAL,MM_PRIO_HIGH,"Calling wfd_mm_play() done.  ret=%d", ret);
        if (ret==WFD_STATUS_SUCCESS) {
            return true;
        }
    } else {
        MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"Calling wfd_mm_play() failed.  MM hasn't been created yet.");
    }
    #endif
    return false;
}

bool MMAdaptor::streamPlayPrepare() {
    if (!isMMStreamEnabled()) {
        return true;
    }

    #ifndef NO_MM_LIB
    if (mmCreated) {
        MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"Calling wfd_mm_play() in prepare");
        WFD_status_t ret = wfd_mm_stream_play(mmHandle, WFD_STREAM_AV, NULL);
        MM_MSG_PRIO1(MM_GENERAL,MM_PRIO_HIGH,"Calling wfd_mm_play() in prepare done.  ret=%d", ret);
        if (ret==WFD_STATUS_SUCCESS) {
            return true;
        }
    } else {
        MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"Calling wfd_mm_play()  in prepare failed.  MM hasn't been created yet.");
    }
    #endif
    return false;
}


bool MMAdaptor::streamControl(WFD_device_t eDeviceType,
                              WFD_MM_AV_Stream_Control_t control, int controlValue)
{
    if (!isMMStreamEnabled()) {
        return true;
    }

    #ifndef NO_MM_LIB
    if (mmCreated) {
        MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"Calling wfd_mm_av_stream_control()");
        WFD_status_t ret = wfd_mm_av_stream_control(mmHandle, eDeviceType, control, controlValue);
        MM_MSG_PRIO1(MM_GENERAL,MM_PRIO_HIGH,"Calling wfd_mm_av_stream_control() done.  ret=%d", ret);
        if (ret==WFD_STATUS_SUCCESS) {
            return true;
        }
    } else {
        MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"Calling wfd_mm_pause() failed.  MM hasn't been created yet.");
    }
    #endif
    return false;

}

bool MMAdaptor::streamPause() {
    if (!isMMStreamEnabled()) {
        return true;
    }

    #ifndef NO_MM_LIB
    if (mmCreated) {
        MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"Calling wfd_mm_pause()");
        WFD_status_t ret = wfd_mm_stream_pause(mmHandle, WFD_STREAM_AV, &stream_pause_cb);
        MM_MSG_PRIO1(MM_GENERAL,MM_PRIO_HIGH,"Calling wfd_mm_pause() done.  ret=%d", ret);
        if (ret==WFD_STATUS_SUCCESS) {
            return true;
        }
    } else {
        MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"Calling wfd_mm_pause() failed.  MM hasn't been created yet.");
    }
    #endif
    return false;
}


/**
 * Function to ask MM lib to enter standby mode
 *
 * @return bool
 */
bool MMAdaptor::standby() {
    if (!isMMStreamEnabled()) {
        return true;
    }

    #ifndef NO_MM_LIB
    if (mmCreated) {
        MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"Calling wfd_mm_stream_standby()");
        WFD_status_t ret = wfd_mm_stream_standby(mmHandle, &stream_pause_cb);
        MM_MSG_PRIO1(MM_GENERAL,MM_PRIO_HIGH,"Calling wfd_mm_stream_standby() done.  ret=%d", ret);
        if (ret==WFD_STATUS_SUCCESS) {
            return true;
        }
    } else {
        MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"Calling wfd_mm_stream_standby() failed.  MM hasn't been created yet.");
    }
    #endif
    return false;
}


/**
 * Function to ask MM lib to exit standby mode
 *
 * @return bool
 */
bool MMAdaptor::resume() {
    if (!isMMStreamEnabled()) {
        return true;
    }

    #ifndef NO_MM_LIB
    if (mmCreated) {
        MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"Calling wfd_mm_stream_resume()");
        WFD_status_t ret = wfd_mm_stream_resume(mmHandle);
        MM_MSG_PRIO1(MM_GENERAL,MM_PRIO_HIGH,"Calling wfd_mm_stream_resume() done.  ret=%d", ret);
        if (ret==WFD_STATUS_SUCCESS) {
            return true;
        }
    } else {
        MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"Calling wfd_mm_stream_resume() failed.  MM hasn't been created yet.");
    }
    #endif
    return false;
}

/**
 * Function to ask MM lib to query last video timestamp
 *
 * @return bool
 */
uint64 MMAdaptor::getCurrentPTS() {
#ifndef NO_MM_LIB
    if (mmCreated) {
        uint64 timeStamp = 0;
        MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"Calling wfd_mm_get_current_PTS)");
        WFD_status_t ret = wfd_mm_get_current_PTS(mmHandle,&timeStamp);
        MM_MSG_PRIO2(MM_GENERAL,MM_PRIO_HIGH,"Calling wfd_mm_get_current_PTS() done.  ret=%d , timestamp = %llu", ret , timeStamp);
        if (ret==WFD_STATUS_SUCCESS) {
            return timeStamp;
        }
    } else {
        MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"Calling wfd_mm_get_current_PTS() failed.  MM hasn't been created yet.");
    }
#endif
    return -1;
}

/**
 * Send IDR frame from source
 *
 * @return bool
 */
bool MMAdaptor::sendIDRFrame() {
    if (!isMMStreamEnabled()) {
        return true;
    }
    SessionManager *pSM = SessionManager::Instance();
     if ((pSM != NULL) && (pSM->pMyDevice!=NULL))
    {
        if (pSM->pMyDevice->getDeviceType() != SOURCE)
        {
            MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"Only WFD Source can call sendIDRFrame()");
            return false;
        }
    }
    #ifndef NO_MM_LIB
    if (mmCreated) {
        MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"Calling wfd_mm_send_IDRframe()");
        WFD_status_t ret = wfd_mm_send_IDRframe(mmHandle);
        MM_MSG_PRIO1(MM_GENERAL,MM_PRIO_HIGH,"Calling wfd_mm_send_IDRframe() done.  ret=%d", ret);
        if (ret==WFD_STATUS_SUCCESS) {
            return true;
        }
    } else {
        MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"Calling wfd_mm_send_IDRframe() failed.  MM hasn't been created yet.");
    }
    #endif
    return false;
}


bool MMAdaptor::setFrameRate(int frameRate) {
    UNUSED(frameRate);
    return true;
}

bool MMAdaptor::setBitRate(int bitRate) {
    WFD_status_t retStatus = WFD_STATUS_FAIL;

    if (mmCreated)
    {
      retStatus = wfd_mm_set_bitrate(mmHandle, bitRate);
      if (retStatus == WFD_STATUS_SUCCESS)
      {
        return true;
      }
    }
    //Add eventError here:
    return false;
}

bool MMAdaptor::executeRuntimeCommand(int cmd){
   WFD_status_t retStatus = WFD_STATUS_FAIL;

    if (mmCreated)
    {
      retStatus = wfd_mm_send_runtime_command(mmHandle,(WFD_runtime_cmd_t)cmd);
      if (retStatus == WFD_STATUS_SUCCESS)
      {
        return true;
      }
    }
    return false;
}

bool MMAdaptor::getAVFormatChangeTiming(uint32* pPTS, uint32* pDTS) {
    SessionManager *pSM = SessionManager::Instance();
    if((pSM != NULL) && (pSM->pMyDevice!=NULL))
    {
        if (pSM->pMyDevice->getDeviceType() != SOURCE) {
            MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"Only WFD Source can call getAVFormatChangeTiming()");
            return false;
        }
    }
    #ifndef NO_MM_LIB
    if (mmCreated) {
        MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"Calling wfd_mm_av_format_change_timing()");
        WFD_status_t ret = wfd_mm_av_format_change_timing(mmHandle, pPTS, pDTS);
        MM_MSG_PRIO3(MM_GENERAL,MM_PRIO_HIGH,"Calling wfd_mm_av_format_change_timing() done.  ret=%d  pts=%u dts=%u", ret, *pPTS, *pDTS);
        if (ret==WFD_STATUS_SUCCESS) {
            return true;
        }
    } else {
        MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"Calling wfd_mm_av_format_change_timing() failed.  MM hasn't been created yet.");
    }
    #endif
    return false;
}


/**
 * The device type definition in MM library and Session Manager
 * is different.  This function is used to convert device type
 * Session Manager into the device type defined in MM library.
 * @param devType
 *
 * @return WFD_device_t
 */
WFD_device_t MMAdaptor::deviceType_conversion(DeviceType devType) {
	switch (devType) {
		case SOURCE:
			return WFD_DEVICE_SOURCE;
		case PRIMARY_SINK:
			return WFD_DEVICE_PRIMARY_SINK;
		case SECONDARY_SINK:
			return WFD_DEVICE_SECONDARY_SINK;
        case UNKNOWN:
            return WFD_DEVICE_INVALID;
        default:
            break;
	}
	return WFD_DEVICE_INVALID;
}


void MMAdaptor::capability_change_cb(void* mm_handle) {
    UNUSED(mm_handle);
    MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"capability_change_cb()");
}

void MMAdaptor::stream_play_cb(void* mm_handle, WFD_status_t status) {
    UNUSED(mm_handle);
    UNUSED(status);
    MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"mm_play_cb()");
    SessionManager *pSM = SessionManager::Instance();
    if (pSM)
    {
      eventStreamControlCompleted(PLAY_DONE, pSM->getRTSPSessionId());
    }
    else
    {
      MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"Session Manager Instance is NULL, something fishy");
    }
}

void MMAdaptor::stream_pause_cb(void* mm_handle, WFD_status_t status) {
    UNUSED(mm_handle);
    UNUSED(status);
    MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"mm_pause_cb()");
    SessionManager *pSM = SessionManager::Instance();
    if (pSM)
    {
      eventStreamControlCompleted(PAUSE_DONE, pSM->getRTSPSessionId());
    }
    else
    {
      MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"Session Manager Instance is NULL, something fishy");
    }
}

void MMAdaptor::capability_update_cb(void* mm_handle, WFD_status_t status) {
    UNUSED(mm_handle);
    UNUSED(status);
    MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"capability_update_cb()");
}

void MMAdaptor::event_update_cb(WFD_MM_Event_t event,
                                WFD_status_t   status,
                                void           *pEvtData) {

    MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"event_update_cb()");

    switch (event)
    {
    case WFD_EVENT_HDCP_CONNECT_DONE:
      MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"HDCP Connect Done");
      if (status == WFD_STATUS_SUCCESS)
        eventMMUpdate(HDCP_CONNECT_SUCCESS,MM_STATUS_SUCCESS,0,0,0,0);
      else
        eventMMUpdate(HDCP_CONNECT_FAIL,MM_STATUS_FAIL,0,0,0,0);
      break;

    case WFD_EVENT_MM_AUDIO:
        if(status == WFD_STATUS_RUNTIME_ERROR ||
           status == WFD_STATUS_FAIL ||
           status == WFD_STATUS_NOTSUPPORTED ||
           status == WFD_STATUS_BADPARAM ||
           status == WFD_STATUS_MEMORYFAIL)
        {
          eventMMUpdate(MM_AUDIO_EVENT_FAILURE,convertWfdToMMStatus(status),0,0,0,0);
        }
        if(status == WFD_STATUS_PROXY_OPENED)
          eventMMUpdate(MM_AUDIO_PROXY_DEVICE_OPENED,convertWfdToMMStatus(status),0,0,0,0);
        if(status == WFD_STATUS_PROXY_CLOSED)
          eventMMUpdate(MM_AUDIO_PROXY_DEVICE_CLOSED,convertWfdToMMStatus(status),0,0,0,0);
        break;

    case WFD_EVENT_MM_VIDEO:
        if(status == WFD_STATUS_RUNTIME_ERROR ||
           status == WFD_STATUS_FAIL          ||
           status == WFD_STATUS_NOTSUPPORTED  ||
           status == WFD_STATUS_BADPARAM      ||
           status == WFD_STATUS_MEMORYFAIL)
        {
          eventMMUpdate(MM_VIDEO_EVENT_FAILURE,convertWfdToMMStatus(status),0,0,0,0);
        }
        break;
    case WFD_EVENT_MM_SESSION_EVENT:
        if (status == WFD_STATUS_SUCCESS && pEvtData) {

          mmSessionInfo *pInfo = (mmSessionInfo *)pEvtData;
          eventMMUpdate(MM_EVENT_STREAM_STARTED, convertWfdToMMStatus(status),
                        pInfo->width, pInfo->height, pInfo->hdcp,pInfo->producer);
        }
        break;
    case WFD_EVENT_MM_HDCP:
        if(status == WFD_STATUS_RUNTIME_ERROR ||
           status == WFD_STATUS_FAIL          ||
           status == WFD_STATUS_NOTSUPPORTED  ||
           status == WFD_STATUS_BADPARAM      ||
           status == WFD_STATUS_MEMORYFAIL)
        {
          eventMMUpdate(MM_HDCP_EVENT_FAILURE,convertWfdToMMStatus(status),0,0,0,0);
        }
        break;
    case WFD_EVENT_MM_RTP:
        if(status == WFD_STATUS_RUNTIME_ERROR ||
           status == WFD_STATUS_FAIL          ||
           status == WFD_STATUS_NOTSUPPORTED  ||
           status == WFD_STATUS_BADPARAM      ||
           status == WFD_STATUS_MEMORYFAIL)
        {
          eventMMUpdate(MM_RTP_EVENT_FAILURE,convertWfdToMMStatus(status),0,0,0,0);
        }
        else if(status == WFD_STATUS_READY) {
            localTransportInfo sTransportInfo = {0,0};
            WFD_transport_capability_config_t *pTransport
                  = (WFD_transport_capability_config_t *) pEvtData;

            if(!pEvtData) {
                MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"RTP STatus Update, Invalid");
                return;
            }
            MM_MSG_PRIO2(MM_GENERAL,MM_PRIO_HIGH,"MMadaptor RTP STatus Update, %d %d",
                         pTransport->port1_id, pTransport->port1_rtcp_id);
            sTransportInfo.rtpPort = pTransport->port1_id;
            sTransportInfo.rtcpPort = pTransport->port1_rtcp_id;
            RTSPSession::UpdateLocalTransportInfo(&sTransportInfo);
        }
        else if (status == WFD_STATUS_RTCP_RR_MESSAGE) {
            mmGenericDataBufType *pBufInfo = (mmGenericDataBufType*)pEvtData;
            if(pBufInfo) {
                eventMMUpdate(MM_RTP_EVENT_RTCP_RR_MESSAGE, convertWfdToMMStatus(status),
                pBufInfo->len, reinterpret_cast<long>(pBufInfo->pData),0,0);
                MM_MSG_PRIO1(MM_GENERAL,MM_PRIO_HIGH,"REceived RTCP MEssage %d", pBufInfo->len);
            }
        }
        break;
    default:
      MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"Unsupported Event from MM");
      break;
    }
}

/**
 * Callback function for MM lib to trigger IDR request on WFD
 * sink
 * @param mm_handle
 */
void MMAdaptor::idr_trigger_cb(void* mm_handle) {
    UNUSED(mm_handle);
    MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"idr_trigger_cb()");
    /* On WFD sink, MM lib uses this callback to ask WDSM to send out IDR request to WFD source through RTSP */
    RTSPSession::sendIDRRequest();
}


/**
 * Callback function for MM lib to trigger capability
 * renegotiation on WFD source
 * @param mm_handle
 */
void MMAdaptor::av_format_change_cb(void* mm_handle) {
    UNUSED(mm_handle);
    MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"av_format_change_cb()");
    RTSPSession::sendAVFormatChangeRequest();
}


bool MMAdaptor::isMMStreamEnabled() {
    char prop[PROPERTY_VALUE_MAX];
    property_get("wfd.mm.stream", prop, "enable");
    MM_MSG_PRIO1(MM_GENERAL,MM_PRIO_HIGH,"IsMMStreamEnabled: %s", prop);
    if (strcmp(prop, "disable") == 0) {
        return false;
    }
    return true;
}

