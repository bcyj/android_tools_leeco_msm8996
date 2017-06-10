/*==============================================================================
*  @file wifidisplay.cpp
*
*  @par DESCRIPTION:
*       Wifi Display Native APIs
*       Interface between SM-B and SM-A JNI
*
*
*  Copyright (c) 2012 - 2014 by Qualcomm Technologies, Incorporated.
*  All Rights Reserved.
*  Qualcomm Technologies Proprietary and Confidential.
==============================================================================*/


/*==============================================================================

                      EDIT HISTORY FOR FILE

  $Header:


when        who         what, where, why
--------    ------      --------------------------------------------------------

================================================================================
*/

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_NDEBUG 0
#define LOG_NDDEBUG 0

#include <utils/Log.h>
#ifndef WFD_ICS
#include "common_log.h"
#endif
#include "wifidisplay.h"

#include "SessionManager.h"
#include "WFDSession.h"
#include "RTSPSession.h"
#include "MMAdaptor.h"
#include "UIBCAdaptor.h"
#include "mdp_version.h"
#ifdef WFD_ICS
#include <surfaceflinger/Surface.h>
#include <surfaceflinger/ISurface.h>
#else
#include <Surface.h>
#endif

#define WIFIDISPLAY_STRING_ARR_SIZE 50
#define WIFIDISPLAY_STRING_SIZE     256
#define WIFIDISPLAY_SEGMENTED_MESSAGE_SIZE (WIFIDISPLAY_STRING_SIZE - 1)

using namespace android;

static stringarray_callback gCallback = NULL;

/* TODO*/
static Surface* gSurface = NULL;

/*
 * Method:    enableWfd
 * This should be called first before calling any other WFD calls.
 */
int enableWfd (WfdDevice* thisDevice, stringarray_callback cb)
{
    MM_Debug_Initialize();
    MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"enableWfd called");
    if (cb != NULL) {
        gCallback = cb;
    }

    SessionManager *pSM = SessionManager::Instance();

    if (!pSM)
        return false;

    DeviceType tp = SOURCE;
    switch (thisDevice->deviceType) {
    case 0:
        tp = SOURCE;
        break;
    case 1:
        tp = PRIMARY_SINK;
        break;
    case 2:
        tp = SECONDARY_SINK;
        break;
    default:
        MM_MSG_PRIO1(MM_GENERAL,MM_PRIO_ERROR,"Unknown devType: %d", thisDevice->deviceType);
        return false;
    }

    Device *dev = new Device(string(thisDevice->macaddress), tp);
    dev->ipAddr  = string(thisDevice->ipaddress);
    dev->coupledPeerMacAddr = string(thisDevice->peermac);
    if (thisDevice->SMControlPort <= 0 ||
        thisDevice->SMControlPort > UINT16_MAX)
    {
      MM_MSG_PRIO2(MM_GENERAL,MM_PRIO_ERROR,"Invalid SM Port: %d, Using default value %u",
           thisDevice->SMControlPort, SM_CONTROL_PORT);
      dev->sessionMngtControlPort = SM_CONTROL_PORT;
    }
    else
    {
      dev->sessionMngtControlPort = (unsigned short)thisDevice->SMControlPort;
    }

    MM_MSG_PRIO1(MM_GENERAL,MM_PRIO_HIGH,"enable WFD for this device %s", dev->macAddr.c_str());
    pSM->enableWfd(dev);

    return true;
}

/*
 * Method:    getResolution
 *To retrieve the current resolution of the WFD session
 */
int getResolution(int32_t* pWidth, int32_t* pHeight)
{
    return RTSPSession::getNegotiatedResolution(pWidth, pHeight);
}

/*
 * Method:    setUIBC
 *@Brief: To set UIBC parameters during WFD session using M14 message
 *@param : Session ID
 *@return : success/failure
*/
int setUIBC(int sessionId)
{
    return RTSPSession::setUIBC(sessionId);
}

/*
 * Method:    getCfgItems
 *@Brief: To get configurable items parsed from cfg fule
 *@param : cfgItems, len
 *@return : success/failure
*/
int getCfgItems(int** cfgItems, size_t* len)
{
    return RTSPSession::getCfgItems(cfgItems,len);
}

/*
 * Method:    setSurface
 *
 */
void setSurface(void *surface)
{
    MM_MSG_PRIO1(MM_GENERAL,MM_PRIO_HIGH,"setSurface called with value %p", surface);
    if (gSurface == NULL) {
        MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"gSurface was null");
    }
    gSurface = (Surface*) surface;
    RTSPSession::setSinkSurface(surface);
}

/*
 * Class:     com_qualcomm_wfd_WFDNative
 * Method:    disableWfd
 * Signature: ()Z
 */
int disableWfd ()
{
    MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"disableWfd called");

    SessionManager *pSM = SessionManager::Instance();

    if (pSM) {
        pSM->disableWfd();
        SessionManager::DeleteInstance();
        pSM = NULL;
    }

    if (gCallback != NULL)
        gCallback = NULL;

    gSurface = NULL;
    MM_Debug_Deinitialize();
    return true;
}

/*
 * Method:    startWfdSession
 *
 */
void startWfdSession (WfdDevice* pD)
{

    SessionManager *pSM = SessionManager::Instance();

    if (pSM) {
        if (false == pSM->startWfdSession(pD)){
            MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"Error in startWfdSession" );
            eventError("StartSessionFail");
        }
    }
}

void stopWfdSession (int sid)
{
  SessionManager *pSM = SessionManager::Instance();
  if (pSM)
  {
    pSM->stopWfdSession(sid);
  }
  else
  {
    MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"pSM is NULL");
  }
}

/*
 * Method:    play
 *
 */
void play_rtsp ( int sessionId, unsigned char secureFlag)
{
    MM_MSG_PRIO2(MM_GENERAL,MM_PRIO_HIGH,"Play RTSP session.  Session id=%d, secureFlag = %d", sessionId, secureFlag);
    if (secureFlag) {
        MMAdaptor::streamPlay();
    } else {
        RTSPSession::streamControl(sessionId, PLAY);
    }
}

/*
 * Method:    pause
 *
 */
void pause_rtsp (int sessionId, unsigned char secureFlag)
{
    MM_MSG_PRIO2(MM_GENERAL,MM_PRIO_HIGH,"Pause RTSP session.  sessionId=%d, secureFlag = %d", sessionId, secureFlag);
    if (secureFlag) {
        MMAdaptor::streamPause();
    } else {
        RTSPSession::streamControl(sessionId, PAUSE);
    }
}

/*
 * Method:    teardown
 *
 */
void teardown_rtsp   ( int sid, int isRTSP)
{
    if (isRTSP) {
    MM_MSG_PRIO1(MM_GENERAL,MM_PRIO_HIGH,"Teardown RTSP session.  session id=%d", sid);
    RTSPSession::streamControl(sid, TEARDOWN);
    }
    else {
     RTSPSession::Cleanup();
    }
}

/*
 * Method:    standby
 *
 */
int standby_rtsp (int sessionId)
{
    MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"standby called");
    return RTSPSession::standby(sessionId);
}

/*
 * Method:    enableUIBC
 *
 */
int enableUIBC (int sessionId)
{
    // send RTSP request to enable UIBC
    MM_MSG_PRIO1(MM_GENERAL,MM_PRIO_HIGH,"Enable UIBC.  RTSP sessionId=%d", sessionId);
    return RTSPSession::uibcControl(sessionId, true);
}

/*
 * Method:    disableUIBC
 *
 */
int disableUIBC (int sessionId)
{
    // send RTSP request to disable UIBC
    MM_MSG_PRIO1(MM_GENERAL,MM_PRIO_HIGH,"Disable UIBC.  RTSP sessionId=%d", sessionId);
    return RTSPSession::uibcControl(sessionId, false);
}

/*
 * Method:    startUIBC
 *
 */
void startUIBC ()
{
    // call MM lib to start UIBC
    UIBCAdaptor::startUIBC();
}

/*
 * Method:    stopUIBC
 *
 */
void stopUIBC  ()
{
    // UIBC Adaptor to stop UIBC
    UIBCAdaptor::stopUIBC();
}

void registerUibcCallbacks(wfd_uibc_attach_cb Attach,
                           wfd_uibc_send_event_cb SendEvent,
                           wfd_uibc_hid_event_cb sendHIDEvent)
{
  UIBCAdaptor::registerUIBCCallbacks(Attach,SendEvent,sendHIDEvent);
}

/*
 * Method:    sendUIBCKeyEvent
 * arg event is of type WFD_uibc_event_t
 */
 int sendUIBCEvent
  (WFD_uibc_event_t* event)
{
    // send the captured Android event to peer device through MM lib

    return UIBCAdaptor::sendUIBCEvent(event);
}


int getRTSPSessionId()
{
   SessionManager *pSM = SessionManager::Instance();
    if (pSM)
        return pSM->getRTSPSessionId();
    else
        return 0;
}

void setCapability (int capType, void *value)
{
    SessionManager *pSM = SessionManager::Instance();
  if (pSM)
    pSM->setUserCapability(capType, value);

}

int setResolution (int type, int value, int* resParams)
{
  return RTSPSession::setResolution(type,value,resParams);
}

void setDecoderLatency (int latency)
{
  return RTSPSession::setDecoderLatency(latency);
}

void setRtpTransport (int transportType)
{
  return RTSPSession::setTransport(transportType);
}

void queryTCPTransportSupport ()
{
  return RTSPSession::queryTCPTransportSupport();
}

void tcpPlaybackControl(int cmdType, int cmdVal)
{
  return RTSPSession::sendBufferingControlRequest(cmdType, cmdVal);
}

void negotiateRtpTransport(int TransportType,int BufferLenMs, int portNum)
{
  return RTSPSession::sendTransportChangeRequest(TransportType, BufferLenMs, portNum);
}

void setBitrate(int value)
{
  MMAdaptor::setBitRate(value);
}

int setAVPlaybackMode (AVPlaybackMode mode)
{
  if (RTSPSession::setAVPlaybackMode(mode))
    return 1;
  else
    return 0;
}

int executeRuntimeCommand (int cmd)
{
  return MMAdaptor::executeRuntimeCommand(cmd);
}

uint32_t* getCommonResolution(int* numProf)
{
  return RTSPSession::getCommonResloution(numProf);
}

uint32_t* getNegotiatedResolution()
{
  return RTSPSession::getNegotiatedResloution();
}


/** =======================================================================
*                Callback functions
** ======================================================================= */


/**
 * Function to send notification to RTSPAdaptor with one string argument.
 * message is comprised of: eventName, eventObjectArray={single_string_argument}.
 */
static void singlestring_callback(const char* eName, const char* str_argument)
{
    const int numObjects = 1;
    char strarray[numObjects][WIFIDISPLAY_STRING_SIZE];
    size_t len = strlen(str_argument) + 1;
    strlcpy(strarray[0], str_argument,len);
    if (gCallback != NULL)
        gCallback(eName, numObjects, strarray);
    else
        MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"singlestring_callback: Callback is NULL");
}


void eventError(const char* reason)
{
    singlestring_callback(ERROR_EVENT, reason);
}

void eventServiceEnabled()
{
    singlestring_callback(SERVICE_CHANGED_EVENT, WFD_ENABLED);
}

void eventServiceDisabled()
{
    singlestring_callback(SERVICE_CHANGED_EVENT, WFD_DISABLED);
}

void eventSessionStateChanged(int sessionState, const char* peerHandleMac, const char* sessionId)
{
    const int numObjects = 3;
    char strarray[numObjects][WIFIDISPLAY_STRING_SIZE];
    SessionManager *pSM = SessionManager::Instance();
    size_t len;
    switch (sessionState) {
    case SESSION_STOPPED:
    {
        const char* stateStr = "STOPPED";
        len = strlen(stateStr) + 1;
        strlcpy(strarray[0], stateStr,len);
        break;
    }
    case SESSION_NEGOTIATING:
    {
        const char* stateStr = "NEGOTIATING";
        len = strlen(stateStr) + 1;
        strlcpy(strarray[0], stateStr,len);
        break;
    }
    case SESSION_NEGOTIATED:
    {
        const char* stateStr = "NEGOTIATED";
        len = strlen(stateStr) + 1;
        strlcpy(strarray[0], stateStr,len);
        //Copying the Rtsp session ID
        pSM->rtspSessionId = atoi(sessionId);
        break;
    }
    case SESSION_ESTABLISHING:
    {
        const char* stateStr = "ESTABLISHING";
        len = strlen(stateStr) + 1;
        strlcpy(strarray[0], stateStr,len);
        //Copying the Rtsp session ID
        pSM->rtspSessionId = atoi(sessionId);
        break;
    }
    case SESSION_ESTABLISHED:
    {
        const char* stateStr = "ESTABLISHED";
        len = strlen(stateStr) + 1;
        strlcpy(strarray[0], stateStr,len);
        break;
    }
    case SESSION_STANDBY:
    {
        const char* stateStr = "STANDBY";
        len = strlen(stateStr) + 1;
        strlcpy(strarray[0], stateStr,len);
        break;
    }
    default:
        MM_MSG_PRIO1(MM_GENERAL,MM_PRIO_ERROR,"Unknown session state: %d", sessionState);
        eventError("Unknown session state.");
        return;
    }

    len = strlen(peerHandleMac) + 1;
    strlcpy(strarray[1], peerHandleMac,len);
    len = strlen(sessionId) + 1;
    strlcpy(strarray[2], sessionId,len);
    if (gCallback != NULL)
        gCallback(SESSION_CHANGED_EVENT, numObjects, strarray);
    else
        MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"eventSessionStateChanged: Callback is null");
}

void eventStreamControlCompleted(int cmdType, int rtspSessId)
{
    const int numObjects = 2;
    char strarray[numObjects][WIFIDISPLAY_STRING_SIZE];
    size_t len;
    snprintf(strarray[0],256, "%d", rtspSessId);
    switch (cmdType) {
    case PLAY:
    {
        const char* controlStr = "PLAY";
        len = strlen(controlStr) + 1;
        strlcpy(strarray[1], controlStr,len);
        break;
    }
    case PLAY_DONE:
    {
        const char* controlStr = "PLAY_DONE";
        len = strlen(controlStr) + 1;
        strlcpy(strarray[1], controlStr,len);
        break;
    }
    case PAUSE:
    {
        const char* controlStr = "PAUSE";
        len = strlen(controlStr) + 1;
        strlcpy(strarray[1], controlStr,len);
        break;
    }
    case PAUSE_DONE:
    {
        const char* controlStr = "PAUSE_DONE";
        len = strlen(controlStr) + 1;
        strlcpy(strarray[1], controlStr,len);
        break;
    }
    case TEARDOWN:
    {
        const char* controlStr = "TEARDOWN";
        len = strlen(controlStr) + 1;
        strlcpy(strarray[1], controlStr,len);
        break;
    }
    default:
        MM_MSG_PRIO1(MM_GENERAL,MM_PRIO_ERROR,"Unknown stream control cmd: %d", cmdType);
        eventError("Unknown stream control command.");
        return;
    }
    if (gCallback != NULL)
        gCallback(STREAM_CONTROL_EVENT, numObjects, strarray);
    else
        MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"eventStreamControlCompleted: Callback is null");
}

void eventUIBCEnabled(int rtspSessId)
{
    const int numObjects = 2;
    char strarray[numObjects][WIFIDISPLAY_STRING_SIZE];
    snprintf(strarray[0],256, "%d", rtspSessId);
    const char* UIBCStr = "ENABLED";
    size_t len = strlen(UIBCStr) + 1;
    strlcpy(strarray[1],UIBCStr,len);
    if (gCallback != NULL)
        gCallback(UIBC_CONTROL_EVENT, numObjects, strarray);
    else
        MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"eventUIBCEnabled: Callback is NULL");
}

void eventUIBCDisabled(int rtspSessId)
{
    const int numObjects = 2;
    char strarray[numObjects][WIFIDISPLAY_STRING_SIZE];
    snprintf(strarray[0],256, "%d", rtspSessId);
    const char* UIBCStr = "DISABLED";
    size_t len = strlen(UIBCStr) + 1;
    strlcpy(strarray[1],UIBCStr,len);
    if (gCallback != NULL)
        gCallback(UIBC_CONTROL_EVENT, numObjects, strarray);
    else
        MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"eventUIBCDisabled: Callback is NULL");
}

void eventUIBCRotate(int angle)
{
    const int numObjects = 1;
    char strarray[numObjects][WIFIDISPLAY_STRING_SIZE];
    snprintf(strarray[0],256,"%d", angle);
    if (gCallback != NULL)
    {
         MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"Sending Rotate event");
         gCallback(UIBC_ROTATE_EVENT, numObjects, strarray);
    }
}

void eventMMUpdate (MMEventType mmEvent,MMEventStatusType status, long evtData1, long evtData2, long evtData3, long evtData4)
{
  char strarray[WIFIDISPLAY_STRING_ARR_SIZE][WIFIDISPLAY_STRING_SIZE];
  switch (mmEvent)
  {
    case HDCP_CONNECT_SUCCESS:
    {
      snprintf(strarray[0],256,"HDCP_CONNECT");
      const char* success = "SUCCESS";
      size_t len = strlen(success) + 1;
      strlcpy(strarray[1],success,len);
      if (gCallback != NULL)
        gCallback(MM_UPDATE_EVENT, 2, strarray);
    }
    break;

    case HDCP_CONNECT_FAIL:
    {
      snprintf(strarray[0],256,"HDCP_CONNECT");
      const char* fail = "FAIL";
      size_t len = strlen(fail) + 1;
      strlcpy(strarray[1],fail,len);
      if (gCallback != NULL)
         gCallback(MM_UPDATE_EVENT, 2, strarray);
    }
    break;

    case HDCP_UNSUPPORTED_BY_PEER:
    {
      snprintf(strarray[0],256,"HDCP_CONNECT");
      const char* fail = "UNSUPPORTEDBYPEER";
      size_t len = strlen(fail) + 1;
      strlcpy(strarray[1],fail,len);
      if (gCallback != NULL)
         gCallback(MM_UPDATE_EVENT, 2, strarray);
    }
    break;

    case MM_VIDEO_EVENT_FAILURE:
    {
      if(status == MM_STATUS_RUNTIME_ERROR ||
         status == MM_STATUS_FAIL)
      {
        MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"eventMMUpdate: Callback is MM_VIDEO_EVENT_FAILURE-RuntimeError");
        snprintf(strarray[0],256, "RuntimeError");
      }
      else if(status == MM_STATUS_NOTSUPPORTED ||
              status == MM_STATUS_BADPARAM     ||
              status == MM_STATUS_MEMORYFAIL)
      {
        MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"eventMMUpdate: Callback is MM_VIDEO_EVENT_FAILURE-ConfigureFailure");
        snprintf(strarray[0],256, "ConfigureFailure");
      }
      const char* fail = "FAIL";
      size_t len = strlen(fail) + 1;
      strlcpy(strarray[1],fail,len);
      if (gCallback != NULL)
        gCallback(MM_VIDEO_EVENT, 2, strarray);
    }
    break;

    case MM_AUDIO_EVENT_FAILURE:
    {
      if(status == MM_STATUS_RUNTIME_ERROR ||
         status == MM_STATUS_FAIL)
      {
          MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"eventMMUpdate: Callback is MM_AUDIO_EVENT_FAILURE- RuntimeError");
        snprintf(strarray[0],256, "RuntimeError");
      }
      else if(status == MM_STATUS_NOTSUPPORTED ||
              status == MM_STATUS_BADPARAM     ||
              status == MM_STATUS_MEMORYFAIL)
      {
        MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"eventMMUpdate: Callback is MM_AUDIO_EVENT_FAILURE- ConfigureFailure");
        snprintf(strarray[0],256, "ConfigureFailure");
      }
      const char* fail = "FAIL";
      size_t len = strlen(fail) + 1;
      strlcpy(strarray[1],fail,len);
      if (gCallback != NULL)
        gCallback(MM_AUDIO_EVENT, 2, strarray);
    }
    break;

    case MM_EVENT_STREAM_STARTED:
    {
        if (status == MM_STATUS_SUCCESS && evtData1)
        {
            int nNumStrings;

            snprintf(strarray[0],256,"MMStreamStarted");
            snprintf(strarray[1],256,"%ld", evtData1);
            snprintf(strarray[2],256,"%ld", evtData2);
            snprintf(strarray[3],256,"%ld", evtData3);
            snprintf(strarray[4],256,"%ld", evtData4);
            if (gCallback != NULL)
                gCallback(MM_UPDATE_EVENT, 5, strarray);
        }
    }
    break;

    case MM_HDCP_EVENT_FAILURE:
    {
      if(status == MM_STATUS_RUNTIME_ERROR ||
         status == MM_STATUS_FAIL)
      {
	      MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"eventMMUpdate: Callback is MM_HDCP_EVENT_FAILURE- RuntimeError");
        snprintf(strarray[0],256, "RuntimeError");
      }
      else if(status == MM_STATUS_NOTSUPPORTED ||
              status == MM_STATUS_BADPARAM     ||
              status == MM_STATUS_MEMORYFAIL)
      {
        MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"eventMMUpdate: Callback is MM_HDCP_EVENT_FAILURE- ConfigureFailure");
        snprintf(strarray[0],256, "ConfigureFailure");
      }
      const char* fail = "FAIL";
      size_t len = strlen(fail) + 1;
      strlcpy(strarray[1],fail,len);
      if (gCallback != NULL)
        gCallback(MM_HDCP_EVENT, 2, strarray);
    }
    break;
    case MM_RTP_EVENT_FAILURE:
    {
      if(status == MM_STATUS_RUNTIME_ERROR ||
         status == MM_STATUS_FAIL)
      {
        MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"eventMMUpdate: Callback is MM_RTP_EVENT_FAILURE- RuntimeError");
        snprintf(strarray[0],256, "RuntimeError");
      }
      else if(status == MM_STATUS_NOTSUPPORTED ||
              status == MM_STATUS_BADPARAM     ||
              status == MM_STATUS_MEMORYFAIL)
      {
        MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"eventMMUpdate: Callback is MM_RTP_EVENT_FAILURE- ConfigureFailure");
        snprintf(strarray[0],256, "ConfigureFailure");
      }
      const char* fail = "FAIL";
      size_t len = strlen(fail) + 1;
      strlcpy(strarray[1],fail,len);
      if (gCallback != NULL)
        gCallback(MM_NETWORK_EVENT, 2, strarray);
    }
    break;

  case BUFFERING_STATUS_UPDATE:
    {
        snprintf(strarray[0],256,"BufferingUpdate");
        snprintf(strarray[1], 256,"%ld", evtData1);
        snprintf(strarray[2], 256,"%ld", evtData2);

        if (gCallback != NULL)
            gCallback(MM_NETWORK_EVENT, 3, strarray);
        else
            MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"eventUIBCDisabled: Callback is NULL");
    }
  break;

  case BUFFERING_NEGOTIATION:
    if(status != MM_STATUS_SUCCESS)
    {
        snprintf(strarray[0],256,"RtpTransportNegotiationFail");

        if (gCallback != NULL)
            gCallback(MM_NETWORK_EVENT, 1, strarray);
        else
            MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"eventUIBCDisabled: Callback is NULL");
    }
    else
    {
        snprintf(strarray[0],256,"RtpTransportNegotiationSuccess");
        snprintf(strarray[1],256,"%ld", evtData1);
        snprintf(strarray[2],256,"%ld", evtData2);

        if (gCallback != NULL)
            gCallback(MM_NETWORK_EVENT, 3, strarray);
        else
            MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR," Callback is NULL");
    }
  break;


  case BUFFERING_CONTROL_EVENT_FLUSH:
      if(status == MM_STATUS_SUCCESS)
      {
          snprintf(strarray[0], 256, "TCPPlaybackControl");
          snprintf(strarray[1], 256, "Flush");
          snprintf(strarray[2], 256, "SUCCESS");
          snprintf(strarray[3], 256,"%ld", evtData1);
          snprintf(strarray[4], 256,"%ld", evtData2);
          if (gCallback != NULL)
              gCallback(MM_NETWORK_EVENT, 5, strarray);
          else
              MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR," Callback is NULL");

      }
      else
      {
          snprintf(strarray[0], 256, "TCPPlaybackControl");
          snprintf(strarray[1], 256, "Flush");
          snprintf(strarray[2], 256, "FAIL");

          if (gCallback != NULL)
              gCallback(MM_NETWORK_EVENT, 3, strarray);
          else
              MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR," Callback is NULL");

      }
      break;
  case BUFFERING_CONTROL_EVENT_PAUSE:
      if(status == MM_STATUS_SUCCESS)
      {
          snprintf(strarray[0], 256, "TCPPlaybackControl");
          snprintf(strarray[1], 256, "Pause");
          snprintf(strarray[2], 256, "SUCCESS");
          snprintf(strarray[3], 256,"%ld", evtData1);
          snprintf(strarray[4], 256,"%ld", evtData2);
          if (gCallback != NULL)
              gCallback(MM_NETWORK_EVENT, 5, strarray);
          else
              MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR," Callback is NULL");

      }
      else
      {
          snprintf(strarray[0], 256, "TCPPlaybackControl");
          snprintf(strarray[1], 256, "Pause");
          snprintf(strarray[2], 256, "FAIL");

          if (gCallback != NULL)
              gCallback(MM_NETWORK_EVENT, 3, strarray);
          else
              MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR," Callback is NULL");

      }
      break;

  case BUFFERING_CONTROL_EVENT_PLAY:
      if(status == MM_STATUS_SUCCESS)
      {
          snprintf(strarray[0], 256, "TCPPlaybackControl");
          snprintf(strarray[1], 256, "Play");
          snprintf(strarray[2], 256, "SUCCESS");
          snprintf(strarray[3], 256,"%ld", evtData1);
          snprintf(strarray[4], 256,"%ld", evtData2);
          if (gCallback != NULL)
              gCallback(MM_NETWORK_EVENT, 5, strarray);
          else
              MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR," Callback is NULL");

      }
      else
      {
          snprintf(strarray[0], 256, "TCPPlaybackControl");
          snprintf(strarray[1], 256, "Play");
          snprintf(strarray[2], 256, "FAIL");

          if (gCallback != NULL)
              gCallback(MM_NETWORK_EVENT, 3, strarray);
          else
              MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR," Callback is NULL");

      }
      break;

  case BUFFERING_CONTROL_EVENT_STATUS:
      if(status == MM_STATUS_SUCCESS)
      {
          snprintf(strarray[0], 256, "TCPPlaybackControl");
          snprintf(strarray[1], 256, "Status");
          snprintf(strarray[2], 256, "SUCCESS");
          snprintf(strarray[3], 256,"%ld", evtData1);
          snprintf(strarray[4], 256,"%ld", evtData2);
          if (gCallback != NULL)
              gCallback(MM_NETWORK_EVENT, 5, strarray);
          else
              MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR," Callback is NULL");

      }
      else
      {
          snprintf(strarray[0], 256, "TCPPlaybackControl");
          snprintf(strarray[1], 256, "Status");
          snprintf(strarray[2], 256, "FAIL");

          if (gCallback != NULL)
              gCallback(MM_NETWORK_EVENT, 3, strarray);
          else
              MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR," Callback is NULL");

      }
      break;
  case BUFFERING_CONTROL_EVENT_DECODER_LATENCY:
      if(status == MM_STATUS_SUCCESS)
      {
          snprintf(strarray[0], 256, "setDecoderLatency");
          snprintf(strarray[1], 256, "SUCCESS");
          if (gCallback != NULL)
              gCallback(MM_NETWORK_EVENT, 2, strarray);
          else
              MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR," Callback is NULL");

      }
      else
      {
          snprintf(strarray[0], 256, "setDecoderLatency");
          snprintf(strarray[1], 256, "FAIL");
          if (gCallback != NULL)
              gCallback(MM_NETWORK_EVENT, 2, strarray);
          else
              MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR," Callback is NULL");

      }
      break;
  case MM_RTP_EVENT_TCP_SUPPORTED_BY_SINK:
      if(status == MM_STATUS_SUCCESS)
      {
          snprintf(strarray[0], 256, "TCPTransportSupport");
          snprintf(strarray[1], 256, "SUCCESS");
          if (gCallback != NULL)
              gCallback(MM_NETWORK_EVENT, 2, strarray);
          else
              MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR," Callback is NULL");

      }
      else
      {
          snprintf(strarray[0], 256, "TCPTransportSupport");
          snprintf(strarray[1], 256, "FAIL");
          if (gCallback != NULL)
              gCallback(MM_NETWORK_EVENT, 2, strarray);
          else
              MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR," Callback is NULL");

      }
      break;
  case MM_RTP_EVENT_RTCP_RR_MESSAGE:
      {
          MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"Recieved MM RTCP receiver report");
          snprintf(strarray[0], 256, "RTCPRRMessage");
          snprintf(strarray[1], 256,"%ld", evtData1); //Len of message

          if(!evtData1 || !evtData2) {
              MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"RTCP Data Size 0 or Invalid ptr");
              return;
          }

          //Need to split the RTCP message across multiple payloads.
          int numSplit = evtData1/WIFIDISPLAY_SEGMENTED_MESSAGE_SIZE;

          if(evtData1 % WIFIDISPLAY_SEGMENTED_MESSAGE_SIZE) {
            numSplit += 1;
          }

          if(numSplit > WIFIDISPLAY_STRING_ARR_SIZE - 2) {
              MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"RTCP message too large. Truncated");
              numSplit = WIFIDISPLAY_STRING_ARR_SIZE - 2;
          }
          // Split RTCP messages into units of 255 as different NULL terminated strings

          uint8 *pSrc = reinterpret_cast<uint8*>(evtData2);
          uint32 nSize = evtData1 > WIFIDISPLAY_SEGMENTED_MESSAGE_SIZE ?
                              WIFIDISPLAY_SEGMENTED_MESSAGE_SIZE : evtData1;

          for(int i = 0; i < numSplit; i++ ) {
              memcpy(strarray[2 + i], pSrc, nSize);
              strarray[2 + i][WIFIDISPLAY_SEGMENTED_MESSAGE_SIZE] = 0;
              evtData1 -= nSize;
              if(!evtData1) {
                  return;
              }
              nSize = evtData1 > WIFIDISPLAY_SEGMENTED_MESSAGE_SIZE ?
                  WIFIDISPLAY_SEGMENTED_MESSAGE_SIZE : evtData1;
          }

          if (gCallback != NULL)
              gCallback(MM_NETWORK_EVENT, 2 + numSplit, strarray);
          else
              MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR," Callback is NULL");
      }
      break;

   case  MM_AUDIO_PROXY_DEVICE_OPENED:
      MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"eventMMUpdate: Callback is MM_AUDIO_PROXY_DEVICE_OPENED");
      snprintf(strarray[0],256, "AudioProxyOpened");
      if (gCallback != NULL)
        gCallback(MM_AUDIO_EVENT, 1, strarray);
      break;

    case  MM_AUDIO_PROXY_DEVICE_CLOSED:
      MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"eventMMUpdate: Callback is MM_AUDIO_PROXY_DEVICE_CLOSED");
      snprintf(strarray[0],256, "AudioProxyClosed");
      if (gCallback != NULL)
        gCallback(MM_AUDIO_EVENT, 1, strarray);
      break;

    case MM_SESSION_AUDIO_ONLY:
      MM_MSG_PRIO(MM_GENERAL,MM_PRIO_HIGH,"eventMMUpdate: Callback is MM_SESSION_AUDIO_ONLY");
      snprintf(strarray[0],256, "AudioOnlySession");
      if (gCallback != NULL)
        gCallback(MM_AUDIO_EVENT, 1, strarray);
      break;

    default:
      break;
  }
}

void* getStreamingSurface()
{
  int32 nSleepTime = 0;
  while (gSurface == NULL && (nSleepTime < 500))//Framework yet to Set Surface
  {
     MM_Timer_Sleep(10);
     nSleepTime += 10;
  }
  if (gSurface == NULL)
     MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"WIFI_DISPLAY:: gSurface Null!");
  return gSurface;
}

/*
 * Informing sink to send a request to source
 * for generating an IDR frame.
 */
void sendIDRRequest()
{
  MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"Wifidisplay : sendIdrRequest : Called");
  RTSPSession::sendIDRRequest();
}
