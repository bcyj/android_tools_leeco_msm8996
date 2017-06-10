
/***************************************************************************
 *                             WFD_Hdcp_SourceCp.cpp
 * DESCRIPTION
 *  HDCP Wrapper for WFD Source content protection
 *
 * Copyright (c) 2011 - 2014  Qualcomm Technologies, Inc. All Rights Reserved
 * Qualcomm Technologies Confidential and Proprietary
 ***************************************************************************/

/***************************************************************************
                              Edit History
  $Header:$
  $DateTime:$
  $Change:$
 ***************************************************************************/

/***************************************************************************
 *                   I N C L U D E-F I L E S
 ***************************************************************************/
#include "WFD_HdcpCP.h"
#ifdef WFD_HDCP_ENABLED
#include "DX_Hdcp_Transmitter.h"
#include "DX_Hdcp_Receiver.h"
#include "DxTypes.h"
#endif //#ifdef WFD_HDCP_ENABLED


#include "MMDebugMsg.h"
#include <stdlib.h>
#include <stdio.h>
#include "MMTimer.h"
#include "MMCriticalSection.h"

/***************************************************************************
 *                   M A C R O - D E F I N I T I O N S
 ***************************************************************************/
//#define ENCRYPT_STATISTICS
#define WFD_HDCP_BAIL_ON_ERROR(_e_)\
  if ( 0 != (_e_))\
    {\
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_FATAL, "DX_HDCP Error Code %x", _e_);\
      goto ERROR_BAIL;\
    }
#define BIT0       0x00000001
#define IP_ADDR_SIZE                 4
#define HDCP_CP_THREAD_STACK_SIZE    16384
#define HDCP_CP_SIGNAL_WAIT_TIME     10000
#define HDCP_CP_CONNECTION_TIMEOUT   10000
#define HDCP_CO_CONNECTION_RETRY_CNT 4
#define HDCP_CO_RECONNECTION_RETRY_CNT 4
#define HDCP_CP_THREAD_PRIORITY      -14
#define HDCP_CIPHER_TIMEOUT 5000 //Wait for 5 seconds for CIPHER_ENABLED event
#define WFD_HDCP_OMX_ERROR_UNDEFINED         (OMX_S32)0x80001001 //same as OMX_ErrorUndefined
#define MAP_HDCP_VERSION(_Ver_) (((_Ver_) == (BIT0)) ? (DX_HDCP_VERSION_2_0) : (DX_HDCP_VERSION_2_1))
#define UNUSED(x) ((void)x)

static const unsigned long WFD_HDCP_CP_THREAD_EXIT_EVENT = 0;
static const unsigned long WFD_HDCP_CP_SESSION_CONNECT_EVENT= 1;
static const unsigned long WFD_HDCP_CP_NOTIFICATION_EVENT  = 2;

void * CWFD_HdcpCp::m_pAppData;
eventHandlerType CWFD_HdcpCp::m_pEventHandlerFn;
unsigned long CWFD_HdcpCp::m_uModuleId;
HDCP_SESSION_STATUSTYPE CWFD_HdcpCp::m_eHdcpSessionStatus;
WFD_HDCP_REPEAT_CAPABILITY CWFD_HdcpCp::m_eHdcpStreamType;
bool CWFD_HdcpCp::m_bHdcpCipherstatus;
CWFD_HdcpCp* CWFD_HdcpCp::s_pHdcpCtx;




/***************************************************************************
 *                  C L A S S - D E F I N I T I O N S
 ***************************************************************************/

/**!
 * @brief      CTOR
 * @details
 *
 */

CWFD_HdcpCp::CWFD_HdcpCp(void *pClientData,
                               unsigned long ulIpAddr,
                               int nCtrlPort,
                               unsigned char ucPrimStreamType,
                               unsigned char ucSecStreamType,
                               WFD_HDCP_DEVICETYPE eDeviceType,
                               unsigned char ucHdcpVer):
m_ucHdcpSessionID(0),
m_ucHdcpClientID(0),
m_ulPeerIpAddr(ulIpAddr),
m_ulHdcpCtrlPort(nCtrlPort),
m_ucHdcpAudioStreamID(0),
m_ucHdcpVideoStreamID(0),
m_ucHdcpPrimaryStreamType(ucPrimStreamType),
m_ucHdcpSecondaryStreamType(ucSecStreamType),
m_pClientData(pClientData),
m_hHdcpCpThread(NULL),
m_bThreadRunning(false),
m_bHdcpSessionConnect(true),
m_bHdcpCipherEventSupported(false),
m_nEncryptFrameSize(0),
m_nMaxFrameSize(0),
m_nEncryptFrames(0),
m_nEncryptTime(0),
m_nMaxEncryptTime(0),
m_nCipherWaitStart(0),
m_nCipherWaitEnd(0),
m_bKeepGoing(true)
{
    UNUSED(eDeviceType);
    UNUSED(ucHdcpVer);
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "CWFD_HdcpCp:Constructor");
    HDCP_CP_ERRORTYPE ulStatus = QC_HDCP_CP_ErrorUndefined;
    HDCP_CP_ERRORTYPE ulRetVal = QC_HDCP_CP_ErrorUndefined;
    int nRetVal = 1; // 0 is success else failure
    // Store HDCP Session Ctx
    s_pHdcpCtx = static_cast<CWFD_HdcpCp*>(this);
    m_eHdcpSessionStatus = HDCP_STATUS_FAIL;

    /* HDCP Session init*/
    ulStatus = WFD_HdcpSessionInit();
    if( QC_HDCP_CP_ErrorNone == ulStatus )
    {
#ifdef WFD_HDCP_ENABLED
      // Check DX HDCP Version
      m_bHdcpCipherEventSupported = WFD_HDCP_Events_Supported();
#endif
      // Create session
      ulStatus = WFD_HdcpCreateSession();
      if ( QC_HDCP_CP_ErrorNone == ulStatus)
      {
        //Open Stream
        ulStatus = WFD_HdcpOpenStream(VIDEO_STREAMID, STREAM_VIDEO);
        if(m_ucHdcpSecondaryStreamType == STREAM_AUDIO)
        {
          ulRetVal = WFD_HdcpOpenStream(AUDIO_STREAMID, STREAM_AUDIO);
          MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,"CWFD_HdcpSrcCp : OpenAudioStream");
        }
        if( QC_HDCP_CP_ErrorNone != ulStatus ||
          ((QC_HDCP_CP_ErrorNone != ulRetVal)&&
            m_ucHdcpSecondaryStreamType == STREAM_AUDIO))
        {
          MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_FATAL, "HDCP Open Stream Failed err"\
                           "code video = %d Audio err = %d",ulStatus, ulRetVal);
          // Close Session
          ulStatus = WFD_HdcpCloseSession();
          // Deinit
          ulStatus = WFD_HdcpSessionDeInit();
          // Notify Status
          m_eHdcpSessionStatus = HDCP_STATUS_FAIL;
        }
        else
        {
          m_eHdcpSessionStatus = HDCP_STATUS_INIT_COMPLETE;
          MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "HDCP_SRC_STATUS_INIT_COMPLETE");
        }
      }
      else
      {
        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_FATAL, "Create Session failed errcode %d ",ulStatus);
        //De-init the session
        ulStatus = WFD_HdcpSessionDeInit();
        if ( QC_HDCP_CP_ErrorNone == ulStatus)
        {
          MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "HDCP SESSION DE-INIT!!");
        }
        else
        {
          MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_FATAL,
                      "HDCP SESSION DE-INIT FAILED ErroCode %d ", ulStatus);
        }
        m_eHdcpSessionStatus = HDCP_STATUS_FAIL;
      }// if HDCP create session
    }
    else
    {
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_FATAL, "Unable to INIT HDCP session!! errorcode %d",ulStatus);
      m_eHdcpSessionStatus = HDCP_STATUS_FAIL;
    }// if HDCP Init

    //If HDCP Session initlization success, create a thread for HDCP Listen
    //and event handling.
    if ( ( HDCP_STATUS_INIT_COMPLETE == m_eHdcpSessionStatus ) &&
         ( 0 == MM_Thread_CreateEx( /*MM_Thread_DefaultPriority*/HDCP_CP_THREAD_PRIORITY,
                                    0,
                                    WFD_HdcpCp_ThreadEntry,
                                   (void *)this,
                                    HDCP_CP_THREAD_STACK_SIZE,
                                    "HDCP_WORKER",
                                    &m_hHdcpCpThread ) ) )
    {
      m_eHdcpStreamType = HDCP_STREAM_DATA_REPEATABLE;
    }
    nRetVal = MM_CriticalSection_Create(&m_encryptCriticalSectionHandle);
    if(nRetVal)
    {
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_FATAL, "Critical section creation failure %d",nRetVal);
      m_encryptCriticalSectionHandle = NULL;
      m_eHdcpSessionStatus = HDCP_STATUS_FAIL;
    }
}

/***************************************************************************
 * @brief      DTOR
 * @details
 *
 ***************************************************************************/

CWFD_HdcpCp::~CWFD_HdcpCp(void)
{
  /*De Init */
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "CWFD_HdcpCp:Destructor");
#ifdef ENCRYPT_STATISTICS
    if(m_nEncryptFrames)
    {
        MM_MSG_PRIO5(MM_GENERAL, MM_PRIO_HIGH,
            "Total HDCP Statistics: Avge Encrypt time %lld, frame size %lld,"
            "Maximum Encrypt time %lld, maximum frame size %lld, "
            "total frames %lld",
            (m_nEncryptTime / m_nEncryptFrames),
            (m_nEncryptFrameSize / m_nEncryptFrames),
            m_nMaxEncryptTime, m_nMaxFrameSize,
            m_nEncryptFrames);
    }


#endif
    int exitCode = 0, nRetVal = 1;
    unsigned long ulStatus = 0;
    //Stop thread
    m_bHdcpSessionConnect = false;
    m_bThreadRunning = false;
    m_bHdcpCipherEventSupported = false;

    // close stream
    ulStatus  = WFD_HdcpCloseStream(STREAM_VIDEO);
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_LOW, "WFD_HdcpCloseStream Primary Stream"\
                                                  "Close errCode %lu",ulStatus);
    if(m_ucHdcpSecondaryStreamType == STREAM_AUDIO)
    {
      ulStatus = WFD_HdcpCloseStream(STREAM_AUDIO);
    }
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_LOW, "WFD_HdcpCloseStream Secondary Stream"\
                                                  "Close errCode %lu",ulStatus);
    // disconnect
    ulStatus = WFD_HdcpDisconnect();
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_LOW, "WFD_HdcpDisconnect errCode %ld",ulStatus);
    // close session
    ulStatus = WFD_HdcpCloseSession();
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_LOW, "WFD_HdcpCloseSession errCode %ld",ulStatus);
    // de-init
    ulStatus = WFD_HdcpSessionDeInit();
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_LOW, "WFD_HdcpSessionDeInit errCode %ld",ulStatus);

    MM_Thread_Join(m_hHdcpCpThread,&exitCode);
    if ( NULL != m_hHdcpCpThread )
    {
      MM_Thread_Release(m_hHdcpCpThread);
    }

    nRetVal = MM_CriticalSection_Release(m_encryptCriticalSectionHandle);
    if(nRetVal)
    {
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR, "WFD_HdcpSessionDeInit Critical"\
        "section errCode %d",nRetVal);
    }
    s_pHdcpCtx = NULL;
}

/***************************************************************************
 * @brief      Init
 * @details    Init a HDCP Source CP session
 * @param[in]  EventCb Event notification callback
 * @return     RETURN '0' if SUCCESS
 *             HDCP_CP_ERROR code in FAILURE
 ***************************************************************************/

HDCP_CP_ERRORTYPE CWFD_HdcpCp::WFD_HdcpSessionInit( /*EventCbFunction EventCb*/ )
{
  HDCP_CP_ERRORTYPE eError = QC_HDCP_CP_ErrorUndefined;

#ifdef WFD_HDCP_ENABLED
  uint32_t  ulStatus = 0;
  EDxHdcpVersion eHDCPVersion = DX_HDCP_VERSION_UNKNOWN;
  /* Init the current Transmitter */
  ulStatus = DX_HDCP_Tsmt_Init(/*EventCb*/NotifyHdcpCpEvents);
  MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "DX HDCP Init Status :%ld",ulStatus);


  eError = (HDCP_CP_ERRORTYPE)ulStatus;
#endif //WFD_HDCP_ENABLED

  return eError;
}

/***************************************************************************
 * @brief      DeInit
 * @details    De-init a HDCP Source CP session
 * @return     RETURN '0' if SUCCESS
 *             HDCP_CP_ERROR code in FAILURE
 ***************************************************************************/

HDCP_CP_ERRORTYPE CWFD_HdcpCp::WFD_HdcpSessionDeInit(void)
{
  HDCP_CP_ERRORTYPE eError = QC_HDCP_CP_ErrorUndefined;
#ifdef WFD_HDCP_ENABLED
  uint32_t ulStatus = 0;
  /* Close the current Transmitter */
  ulStatus = DX_HDCP_Tsmt_Close();
  MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,"WFD_HdcpSessionDeInit(): ErrorStatus(%lu)",ulStatus);
  eError = (HDCP_CP_ERRORTYPE)ulStatus;
#endif //WFD_HDCP_ENABLED
  return eError;
}

/***************************************************************************
 * @brief      Configure Callback
 * @details    Set the callback
 * @return     RETURN '0' if SUCCESS
 *             Non Zero on FAILURE
 ***************************************************************************/
int CWFD_HdcpCp::WFD_HdcpCpConfigureCallback(eventHandlerType pCallback, unsigned long moduleId, void *pData)
{
  UNUSED(pCallback);
  UNUSED(moduleId);
  UNUSED(pData);
#ifdef WFD_HDCP_ENABLED
  if(NULL != pCallback)
  {
     m_pEventHandlerFn = pCallback;
  }
  else
  {
     MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,"WFD_HdcpCpConfigureCallback(): Callback is NULL");
     return 1;
  }
  m_uModuleId = moduleId;
  m_pAppData = pData;
  MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,"WFD_HdcpCpConfigureCallback : Callback set, module id = %lu",m_uModuleId);
#endif //WFD_HDCP_ENABLED
  return 0;
}

/***************************************************************************
 * @brief      CTOR
 * @details    CTOR to create & init a HDCP Source CP session
 * @param[in]  ulIpAddr HDCP Transmitter peer IP Address
 * @param[in]  usPort   Control port on which HDCP Transmitter( WFD-SRC)
 *             & HDCP Receiver ( WFD-SINK) communicate session message
 * @return     RETURN '0' if SUCCESS
 *             HDCP_CP_ERROR code in FAILURE
 ***************************************************************************/

HDCP_CP_ERRORTYPE CWFD_HdcpCp::WFD_HdcpDataEncrypt (
  unsigned char ucStreamType,
  unsigned char*  aPESPvtHeader,
  unsigned char * pucPESClrPayload,
  unsigned char * pucPESEncrytPayload,
  unsigned long  ulPayloadLen )
{
  int nRetVal = 1;
  HDCP_CP_ERRORTYPE eError = QC_HDCP_CP_ErrorUndefined;
  UNUSED(ucStreamType);
  UNUSED(aPESPvtHeader);
  UNUSED(pucPESClrPayload);
  UNUSED(pucPESEncrytPayload);
  UNUSED(ulPayloadLen);
#ifdef WFD_HDCP_ENABLED
  if( HDCP_STATUS_SUCCESS != m_eHdcpSessionStatus )
  {
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR,"WFD_HdcpDataEncrypt- Session status is not successful = %d",m_eHdcpSessionStatus);
    return QC_HDCP_CP_ErrorUndefined;
  }
  if(true == CWFD_HdcpCp::m_bHdcpCipherEventSupported)
  {
    if(false == CWFD_HdcpCp::m_bHdcpCipherstatus)
    {
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "Cipher is disabled");
      if(m_nCipherWaitStart != 0)//First CIPHER_DISABLED event received
      {
          unsigned long currTime;
          MM_Time_GetTime(&currTime);
          if(currTime - m_nCipherWaitStart > HDCP_CIPHER_TIMEOUT)
          {
              MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
              "Cipher is not enabled still, irrecoverable error!");
              /*--------------------------------------------------------------------------
               Set flag to indicate irrecoverable error to prevent any further
               encrypt calls from source modules
              ----------------------------------------------------------------------------
              */
              m_bKeepGoing = false;
          }
      }
      return QC_HDCP_CP_ErrorCipherDisabled;
    }
  }
  else
  {
   MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM, " Old HDCP: don't use new events");
  }
  if ( (NULL != pucPESClrPayload )   &&
       (NULL != pucPESEncrytPayload) &&
       (NULL != aPESPvtHeader))
  {
    uint32_t  ulStatus = 0;
    bool bConnectStatus = false;
    DxStreamID_t ucHdcpStreamID =0;
    if( STREAM_AUDIO == ucStreamType )
    {
      ucHdcpStreamID = m_ucHdcpAudioStreamID;
    }
    else
    {
      ucHdcpStreamID = m_ucHdcpVideoStreamID;
    }
    /* Start encrypt the content */
    MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_HIGH,"WFD_HdcpDataEncrypt- params = %u, %lu",ucHdcpStreamID,ulPayloadLen);
#ifdef ENCRYPT_STATISTICS
    unsigned long beforeEncryptTime;
    unsigned long afterEncryptTime;
    MM_Time_GetTime(&beforeEncryptTime);
#endif
    if(m_encryptCriticalSectionHandle)
    {
      nRetVal = MM_CriticalSection_Enter(m_encryptCriticalSectionHandle);
      if(nRetVal)
      {
        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR,"WFD_HdcpDataEncrypt- Critical section Enter failed = %d",nRetVal);
        return QC_HDCP_CP_InsufficientResources;
      }
    }
    else
    {
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,"WFD_HdcpDataEncrypt- Critical section m_encryptCriticalSectionHandle is NULL");
      return QC_HDCP_CP_InsufficientResources;
    }
  do{
    ulStatus = DX_HDCP_Tsmt_Encrypt2( m_ucHdcpSessionID,
                                      ucHdcpStreamID,
                                      aPESPvtHeader,
                                      (uint64_t)pucPESClrPayload,
                                      (uint64_t)pucPESEncrytPayload,
                                      ulPayloadLen,
                                      0,
                                      DX_HDCP_BUFFER_TYPE_SHARED_MEMORY_HANDLE,
                                      DX_HDCP_BUFFER_TYPE_SHARED_MEMORY_HANDLE);

    bConnectStatus = false;
    if(DX_HDCP_NO_ACTIVE_CONNECTION == ulStatus && m_bHdcpSessionConnect
      && HDCP_STATUS_SUCCESS == m_eHdcpSessionStatus)
    {
       MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,"WFD_HdcpDataEncrypt DX_HDCP_NO_ACTIVE_CONNECTION");
       // Close the current session
       HDCP_CP_ERRORTYPE eErrValue = QC_HDCP_CP_ErrorUndefined;
       eErrValue = WFD_HdcpCpCleanUp(this);
       if( QC_HDCP_CP_ErrorNone != eErrValue )
       {
         if (m_encryptCriticalSectionHandle)
         {
           nRetVal = MM_CriticalSection_Leave(m_encryptCriticalSectionHandle);
           if(nRetVal)
           {
             MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR,"WFD_HdcpDataEncrypt- Critical section Leave failed = %d",nRetVal);
             return QC_HDCP_CP_InsufficientResources;
           }
         }
         else
         {
           MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,"WFD_HdcpDataEncrypt- Critical section m_encryptCriticalSectionHandle is NULL");
           return QC_HDCP_CP_InsufficientResources;
         }
         if(m_pEventHandlerFn)
         {
             m_pEventHandlerFn(m_pAppData, m_uModuleId, WFDMMSRC_ERROR,(OMX_ERRORTYPE)WFD_HDCP_OMX_ERROR_UNDEFINED, 0);
         }
         return QC_HDCP_CP_ErrorUndefined;
       }
       // Reconnect
       MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,"WFD_HdcpDataEncrypt b4 WFD_HdcpCpReconnect");
       eErrValue = WFD_HdcpCpReconnect(this);
       MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,"WFD_HdcpDataEncrypt after WFD_HdcpCpReconnect");
       if( QC_HDCP_CP_ErrorNone != eErrValue )
       {
         if (m_encryptCriticalSectionHandle)
         {
           nRetVal = MM_CriticalSection_Leave(m_encryptCriticalSectionHandle);
           if(nRetVal)
           {
             MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR,"WFD_HdcpDataEncrypt- Critical section Leave failed = %d",nRetVal);
             return QC_HDCP_CP_InsufficientResources;
           }
         }
         else
         {
           MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,"WFD_HdcpDataEncrypt- Critical section m_encryptCriticalSectionHandle is NULL");
           return QC_HDCP_CP_InsufficientResources;
         }
         if(m_pEventHandlerFn)
         {
             m_pEventHandlerFn(m_pAppData, m_uModuleId, WFDMMSRC_ERROR,(OMX_ERRORTYPE)WFD_HDCP_OMX_ERROR_UNDEFINED, 0);
         }
         return QC_HDCP_CP_ErrorUndefined;
       }
       if( HDCP_STATUS_INIT_COMPLETE == m_eHdcpSessionStatus )
       {
         //Connect to downstream devices.
         MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH ,"WFD_HdcpDataEncrypt b4 WFD_HdcpSessionConnect");
         if ( QC_HDCP_CP_ErrorNone == WFD_HdcpSessionConnect(HDCP_CO_RECONNECTION_RETRY_CNT) )
         {
           if(m_eHdcpSessionStatus != HDCP_STATUS_UNRECOVERABLE_ERROR)
           {
            m_eHdcpSessionStatus = HDCP_STATUS_SUCCESS;
            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,"WFD_HdcpSessionConnect(): SUCCESS!!");
           }
           else
           {
            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "Ignore HDCP Connect Succes due to unrecoverable error");
           }
           bConnectStatus = true;
         }
         else
         {
           m_eHdcpSessionStatus = HDCP_STATUS_FAIL;
           bConnectStatus = false;
           if(m_encryptCriticalSectionHandle)
           {
             nRetVal = MM_CriticalSection_Leave(m_encryptCriticalSectionHandle);
             if(nRetVal)
             {
               MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR,"WFD_HdcpDataEncrypt- Critical section Leave failed = %d",nRetVal);
               return QC_HDCP_CP_InsufficientResources;
             }
           }
           else
           {
             MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,"WFD_HdcpDataEncrypt- Critical section m_encryptCriticalSectionHandle is NULL");
             return QC_HDCP_CP_InsufficientResources;
           }
           MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,"WFD_HdcpSessionConnect(): FAILURE!!");
           if(m_pEventHandlerFn)
           {
               m_pEventHandlerFn(m_pAppData, m_uModuleId, WFDMMSRC_ERROR,(OMX_ERRORTYPE)WFD_HDCP_OMX_ERROR_UNDEFINED, 0);
           }
           return QC_HDCP_CONNECTION_TIMEOUT;
         }
       }
    }
  }while(bConnectStatus);

    if(m_encryptCriticalSectionHandle)
    {
      nRetVal = MM_CriticalSection_Leave(m_encryptCriticalSectionHandle);
      if(nRetVal)
      {
        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR,"WFD_HdcpDataEncrypt- Critical section Leave failed = %d",nRetVal);
        return QC_HDCP_CP_InsufficientResources;
      }
    }
    else
    {
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,"WFD_HdcpDataEncrypt- Critical section m_encryptCriticalSectionHandle is NULL");
      return QC_HDCP_CP_InsufficientResources;
    }
#ifdef ENCRYPT_STATISTICS
    MM_Time_GetTime(&afterEncryptTime);
    unsigned long diffTime = afterEncryptTime - beforeEncryptTime;
    m_nEncryptTime += diffTime;
    if( m_nMaxEncryptTime < diffTime )
    {
      m_nMaxEncryptTime = diffTime;
    }

    m_nEncryptFrameSize += ulPayloadLen;
    if( m_nMaxFrameSize < ulPayloadLen)
    {
      m_nMaxFrameSize = ulPayloadLen;
    }

    m_nEncryptFrames++;
    if( m_nEncryptFrames % 30 == 0)
    {
      MM_MSG_PRIO5(MM_GENERAL, MM_PRIO_HIGH,"HDCP Statistics: Avge Encrypt time %lld, frame size %lld,"
                                            "Maximum Encrypt time %lld, maximum frame size %lld, "
                                            "total frames %lld",
                                            (m_nEncryptTime / m_nEncryptFrames),
                                            (m_nEncryptFrameSize / m_nEncryptFrames),
                                            m_nMaxEncryptTime, m_nMaxFrameSize,
                                            m_nEncryptFrames);
    }
#endif
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,"WFD_HdcpDataEncrypt(): ErrorStatus(%ld)",ulStatus);
    eError = (HDCP_CP_ERRORTYPE)ulStatus;
  }//if != NULL
  else
  {
    eError = QC_HDCP_CP_BadParameter;
  }
#endif //WFD_HDCP_ENABLED
  return eError;
}

/***************************************************************************
 * @brief      Open Stream
 * @details    Open HDCP stream
 * @param[in]  ucContentStreamID Content stream ID
 * @param[in]  ucStreamType      Content stream type i.e. audio/video
 * @return     RETURN '0' if SUCCESS
 *             HDCP_CP_ERROR code in FAILURE
 ***************************************************************************/
HDCP_CP_ERRORTYPE CWFD_HdcpCp::WFD_HdcpOpenStream(
                                                uint32  ucContentStreamID,
                                                unsigned char ucStreamType
                                                 )
{
  HDCP_CP_ERRORTYPE eError = QC_HDCP_CP_ErrorUndefined;
#ifdef WFD_HDCP_ENABLED
  uint32_t ulStatus = 0;
  DxStreamID_t ucStreamID = 0;
  EDxHdcpStreamType DxStreamType = DX_HDCP_STREAM_TYPE_UNKNOWN;
  if( STREAM_AUDIO == ucStreamType)
  {
    DxStreamType = DX_HDCP_STREAM_TYPE_AUDIO;
  }
  else
  {
    DxStreamType = DX_HDCP_STREAM_TYPE_VIDEO;
  }
  ulStatus =  DX_HDCP_Tsmt_Open_Stream_2( m_ucHdcpSessionID,
                                        ucContentStreamID,
                                        &ucStreamID,
                                        DxStreamType );
  if( STREAM_AUDIO == ucStreamType)
  {
    m_ucHdcpAudioStreamID = ucStreamID;
  }
  else
  {
    m_ucHdcpVideoStreamID = ucStreamID;
  }
  MM_MSG_PRIO3(MM_GENERAL, MM_PRIO_HIGH,"WFD_HdcpOpenStream():ErrorStatus(%lu) stream Type =%d stream id = %d",ulStatus,ucStreamType,ucStreamID);
  eError = (HDCP_CP_ERRORTYPE)ulStatus;
#else
  UNUSED(ucContentStreamID);
  UNUSED(ucStreamType);
#endif //#ifdef WFD_HDCP_ENABLED
  return eError;
}

/***************************************************************************
 * @brief      Close Stream
 * @details    Close HDCP stream used in particular session
 * @param[in]  ucStreamType      Content stream type i.e. audio/video
 * @return     RETURN '0' if SUCCESS
 *             HDCP_CP_ERROR code in FAILURE
 ***************************************************************************/
HDCP_CP_ERRORTYPE CWFD_HdcpCp::WFD_HdcpCloseStream( unsigned char ucStreamType )
{
  HDCP_CP_ERRORTYPE eError = QC_HDCP_CP_ErrorUndefined;

#ifdef WFD_HDCP_ENABLED
  uint32_t ulStatus = 0;
  DxStreamID_t ucStreamID = 0;
  if( STREAM_AUDIO == ucStreamType )
  {
    ucStreamID = m_ucHdcpAudioStreamID;
  }
  else
  {
    ucStreamID = m_ucHdcpVideoStreamID;
  }
  ulStatus = DX_HDCP_Tsmt_Close_Stream(ucStreamID);
  MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,"WFD_HdcpCloseStream():ErrorStatus(%lu)",ulStatus);
  eError = (HDCP_CP_ERRORTYPE)ulStatus;
#else
  UNUSED(ucStreamType);
#endif //WFD_HDCP_ENABLED
  return eError;
}
 /****************************************************************************
  * @brief      Create HDCP Session
  * @details    Create a new HDCP session and connect
  * @param[in] ulTimeSout Connection time out
  * @return     RETURN '0' if SUCCESS
  *             HDCP_CP_ERROR code in FAILURE
  ****************************************************************************/
HDCP_CP_ERRORTYPE CWFD_HdcpCp::WFD_HdcpCreateSession()
{
  HDCP_CP_ERRORTYPE eError = QC_HDCP_CP_ErrorUndefined;
#ifdef WFD_HDCP_ENABLED
  uint32_t ulStatus = 0;
  /*! Open a new WFD SRC HDCP session */
  ulStatus = DX_HDCP_Tsmt_Open_Session((DxSessionID_t*)&m_ucHdcpSessionID);
  MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,"WFD_HdcpCreateSession():ErrorStatus(%lu)",ulStatus);
  eError = (HDCP_CP_ERRORTYPE)ulStatus;
#endif // WFD_HDCP_ENABLED
  return eError;
}

 /****************************************************************************
  * @brief		 Connect HDCP Session
  * @details	 Connect to a HDCP session
  * @param[in]  ulTimeSout Connection time out
  * @return 	 RETURN '0' if SUCCESS
  *				 HDCP_CP_ERROR code in FAILURE
  ****************************************************************************/
 HDCP_CP_ERRORTYPE CWFD_HdcpCp::WFD_HdcpSessionConnect(
   unsigned long ulRetryCnt)
{
  HDCP_CP_ERRORTYPE eError = QC_HDCP_CP_ErrorUndefined;
#ifdef WFD_HDCP_ENABLED
  uint32_t ulStatus = QC_HDCP_CP_ErrorUndefined;
  unsigned char szIpAddr[4];
  /*! Open new connection using DX_HDCP_Tsmt_Connect */
  MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,"ConnectIPAddr: %lu",m_ulPeerIpAddr);
  for(int index= 4; index > 0;index--)
  {
    szIpAddr[index-1]= (m_ulPeerIpAddr >> ((index -1)*8)) & 0xFF;
    MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_HIGH,
      "WFD_HdcpSessionConnect(): Peer IpAddr  %x  %u",
      szIpAddr[index-1],m_ulHdcpCtrlPort );
  }

  unsigned long nStartTime;
  MM_Time_GetTime(&nStartTime);
  unsigned long nTotalTime = ulRetryCnt * HDCP_CP_CONNECTION_TIMEOUT;
  unsigned long nCurrTime = 0;
  unsigned long nTempTime = 0;

  while(m_bHdcpSessionConnect && m_eHdcpSessionStatus != HDCP_STATUS_UNRECOVERABLE_ERROR)
  {
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_LOW, "trying again");

    ulStatus = DX_HDCP_Tsmt_Connect( (unsigned char *)szIpAddr,
                                    m_ulHdcpCtrlPort,
                                    m_ucHdcpSessionID,
                                   &m_ucHdcpClientID,
                                    HDCP_CP_CONNECTION_TIMEOUT );
    if(ulStatus == 0)
      break;
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,"WFD_HdcpSessionConnect():ErrorStatus(%lu)",ulStatus);

    MM_Time_GetTime(&nTempTime);

    if(nTempTime - nCurrTime < HDCP_CP_CONNECTION_TIMEOUT)
    {
      MM_Timer_Sleep(300);
    }

    MM_Time_GetTime(&nCurrTime);

    if(nCurrTime - nStartTime > nTotalTime)
    {
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "HDCP exceeded retry time... break");
      break;
    }
  }//while retry count
  eError = (HDCP_CP_ERRORTYPE)ulStatus;
#else
  UNUSED(ulRetryCnt);
#endif //#ifdef WFD_HDCP_ENABLED
  return eError;
}

 /****************************************************************************
  * @brief      Close HDCP Session
  * @details    Close HDCP session and disconnect
  * @return     RETURN '0' if SUCCESS
  *             HDCP_CP_ERROR code in FAILURE
  ****************************************************************************/
HDCP_CP_ERRORTYPE CWFD_HdcpCp::WFD_HdcpCloseSession()
{
  HDCP_CP_ERRORTYPE eError = QC_HDCP_CP_ErrorUndefined;
#ifdef WFD_HDCP_ENABLED
  uint32_t ulStatus = 0;
  /* Close the current opened session */
  ulStatus = DX_HDCP_Tsmt_Close_Session( m_ucHdcpSessionID );
  MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,"WFD_HdcpCloseSession(): ErrorStatus(%lu)",ulStatus);
  eError = (HDCP_CP_ERRORTYPE)ulStatus;
#endif //#ifdef WFD_HDCP_ENABLED
  return eError;
}

 /****************************************************************************
  * @brief      Disconnect HDCP Session
  * @details    Disconnect HDCP Session
  * @return     RETURN '0' if SUCCESS
  *             HDCP_CP_ERROR code in FAILURE
  ****************************************************************************/
HDCP_CP_ERRORTYPE CWFD_HdcpCp::WFD_HdcpDisconnect()
{
  HDCP_CP_ERRORTYPE eError = QC_HDCP_CP_ErrorUndefined;
#ifdef WFD_HDCP_ENABLED

  uint32_t ulStatus = 0;
  /* Close the current connection */
  ulStatus = DX_HDCP_Tsmt_Disconnect( m_ucHdcpClientID );
  MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,"WFD_HdcpDisconnect(): ErrorStatus(%lu)",ulStatus);
  eError = (HDCP_CP_ERRORTYPE)ulStatus;

#endif //#ifdef WFD_HDCP_ENABLED
  return eError;
}

/****************************************************************************
 * @brief		HDCP Source CP thread entry.
 * @details   This function is the entry point to HDCP CP Session handler thread
 * @param[in]	ptr This pointer
 * @return		None
 ****************************************************************************/
int CWFD_HdcpCp::WFD_HdcpCp_ThreadEntry( void* ptr )
{
  CWFD_HdcpCp* pHdcpSrcCp = (CWFD_HdcpCp*)ptr;
  if( NULL != pHdcpSrcCp)
  {
    pHdcpSrcCp->WFD_HdcpSessionHandlerThread();
  }
  return 0;
}

/****************************************************************************

 * @brief	  HDCP source CP session handler
 * @details   This function handles all the event posted to HDCP CP session thread
 * @return	  None
 ****************************************************************************/
void CWFD_HdcpCp::WFD_HdcpSessionHandlerThread( )
{
  bool bRunning = true;
  int bTimeOut = 0;
  m_bThreadRunning = true;
  while(m_bThreadRunning)
  {
    //Check current HDCP status, if init successful then start listen
    //MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,"WFD_HdcpSessionHandlerThread()!!");
    if( HDCP_STATUS_INIT_COMPLETE == m_eHdcpSessionStatus )
    {
      //Connect to downstream devices.
      if ( QC_HDCP_CP_ErrorNone == WFD_HdcpSessionConnect(HDCP_CO_CONNECTION_RETRY_CNT) )
      {
       if(m_eHdcpSessionStatus != HDCP_STATUS_UNRECOVERABLE_ERROR)
       {
        m_eHdcpSessionStatus = HDCP_STATUS_SUCCESS;
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,"WFD_HdcpSessionConnect(): SUCCESS!!");
       }
       else
       {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,"Ignore WFD_HdcpSessionConnect(): SUCCESS");
       }
      }
       else
       {
        m_eHdcpSessionStatus = HDCP_STATUS_FAIL;
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,"WFD_HdcpSessionConnect(): FAILURE!!");
       }
    }//if HDCP Init SUCCESS!!
    else
    {
      //Note:Need to remove this if encrypt is handled in this thread
      m_bThreadRunning = false;
    }
  }//while (m_bThreadRunning)
  // Thread Exit
  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,"WFD_HdcpSessionHandlerThread: Exiting thread");
  MM_Thread_Exit(this->m_hHdcpCpThread, 0 );
}

#ifdef WFD_HDCP_ENABLED
/****************************************************************************

 * @brief      HDCP Event Notification
 * @details    This function handles hdcp event
 * @param[in]  eHdcpEvent	  HDCP Event type
 * @param[in]  pConnectionid ConnectionId for this session
 * @param[in]  pStreamId	  StreamID for this session
 * @return     None
 ****************************************************************************/

void CWFD_HdcpCp::NotifyHdcpCpEvents(EDxHdcpEventType eHdcpEvent,
                                     void* pConnectionid,
                                     void* pStreamid)

{
    UNUSED(pConnectionid);
    UNUSED(pStreamid);
    //Capture Event and set signal to handler thread
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,"WFD_HdcpCp : Event Notified(%d)",eHdcpEvent);
    switch(eHdcpEvent)
    {
      case DX_HDCP_EVENT_UPSTREAM_CLOSE:
        {
          CWFD_HdcpCp::m_eHdcpSessionStatus = HDCP_STATUS_UPSTREAM_CLOSE;
          MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,"HDCP_UPSTREAM_CLOSE!!");
        }
        break;

      case DX_HDCP_EVENT_DOWNSTREAM_CLOSE:
        {
          MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,"HDCP_DOWNSTREAM_CLOSE!!");
          /*
             Don't report this error to UI, as the next encrypt call throws
             DX_HDCP_NO_ACTIVE_CONNECTION and we have to cleanup and reconnect
         */
	  if(m_eHdcpSessionStatus != HDCP_STATUS_SUCCESS)
	  {
             m_eHdcpSessionStatus = HDCP_STATUS_UNRECOVERABLE_ERROR;
          }
        }
        break;

      case DX_HDCP_EVENT_UNAUTHENTICATED_CONNECTION:
        {
          CWFD_HdcpCp::m_eHdcpSessionStatus = HDCP_STATUS_UNAUTHENTICATED_CONNECTION;
          MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,"HDCP_UNAUTHENTICATED_CONNECTION!!");
          if(s_pHdcpCtx)
          {
               s_pHdcpCtx->m_bKeepGoing = false;
          }
        }
        break;

      case DX_HDCP_EVENT_UNAUTHORIZED_CONNECTION:
        {
          CWFD_HdcpCp::m_eHdcpSessionStatus = HDCP_STATUS_UNAUTHORIZED_CONNECTION;
          MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,"HDCP_UNAUTHORIZED_CONNECTION!!");
          if(s_pHdcpCtx)
          {
               s_pHdcpCtx->m_bKeepGoing = false;
          }
        }
        break;

      case DX_HDCP_EVENT_REVOKED_CONNECTION:
        {
          CWFD_HdcpCp::m_eHdcpSessionStatus = HDCP_STATUS_REVOKED_CONNECTION;
          MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,"HDCP_CONNECTION_REVOKED!!");
        }
        break;

      case DX_HDCP_EVENT_INTERNAL_ERROR:
        {
          CWFD_HdcpCp::m_eHdcpSessionStatus = HDCP_STATUS_UNRECOVERABLE_ERROR;
          MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,"HDCP_INTERNAL_ERROR!!");
          if(s_pHdcpCtx)
          {
               s_pHdcpCtx->m_bKeepGoing = false;
          }
        }
        break;

      case DX_HDCP_EVENT_CIPHER_ENABLED:
        {
          CWFD_HdcpCp::m_bHdcpCipherstatus = true;
          MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,"HDCP_CIPHER_ENABLED!!");
          //Reset the time
          if(s_pHdcpCtx)
          {
            s_pHdcpCtx->m_nCipherWaitStart = 0;
            s_pHdcpCtx->m_nCipherWaitEnd   = 0;
            s_pHdcpCtx->m_bKeepGoing       = true;
          }
        }
        break;

      case DX_HDCP_EVENT_CIPHER_DISABLED:
        {
          CWFD_HdcpCp::m_bHdcpCipherstatus = false;
          MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,"HDCP_CIPHER_DISABLED!!");
          if(s_pHdcpCtx)
          {
            if(s_pHdcpCtx->m_nCipherWaitStart == 0)
            {
              //The first CIPHER_DISABLED
              MM_Time_GetTime(&(s_pHdcpCtx->m_nCipherWaitStart));
            }
            MM_Time_GetTime(&(s_pHdcpCtx->m_nCipherWaitEnd));
            if(s_pHdcpCtx->m_nCipherWaitEnd -
               s_pHdcpCtx->m_nCipherWaitStart
               >= HDCP_CIPHER_TIMEOUT)
            {
               s_pHdcpCtx->m_bKeepGoing = false;
            }
          }
        }
        break;

      default:
        {
          MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,"Unknown HDCP event!!");
        }
        break;
    }
}


/***************************************************************************
 * @brief      Clean up the session
 * @details    Cleanup the current HDCP session in case Encrypt returned
               DX_HDCP_NO_ACTIVE_CONNECTION error
 * @param[in]  Current HDCP context
 * @return     HDCP_CP_ERROR code in FAILURE
 ***************************************************************************/
static HDCP_CP_ERRORTYPE WFD_HdcpCpCleanUp(CWFD_HdcpCp *pHDCPContext)
{
  HDCP_CP_ERRORTYPE ulStatus = QC_HDCP_CP_ErrorUndefined;
   // Close Stream(s)
   ulStatus = pHDCPContext->WFD_HdcpCloseStream(pHDCPContext->m_ucHdcpPrimaryStreamType);
   MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_LOW, "WFD_HdcpCp_CleanUp:CloseStream"\
                                    "Primary Stream Close errCode %d",ulStatus);
   if(pHDCPContext->m_ucHdcpSecondaryStreamType == STREAM_AUDIO)
   {
     ulStatus = pHDCPContext->WFD_HdcpCloseStream(pHDCPContext->m_ucHdcpSecondaryStreamType);
   }
   MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_LOW, "WFD_HdcpCp_CleanUp:CloseStream" \
                                  "Secondary Stream Close errCode %d",ulStatus);

   ulStatus = pHDCPContext->WFD_HdcpDisconnect();
   MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_LOW, "WFD_HdcpCp_CleanUp:Disconnect"\
                                                         "errCode %d",ulStatus);

   // Close Session
   ulStatus = pHDCPContext->WFD_HdcpCloseSession();
   MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_LOW, "WFD_HdcpCp_CleanUp:CloseSesion"\
                                                        "errCode %d",ulStatus);


   ulStatus = pHDCPContext->WFD_HdcpSessionDeInit();
   MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_LOW, "WFD_HdcpCp_CleanUp:DeInit"\
                                                        "errCode %d",ulStatus);


  return ulStatus;
}

/***************************************************************************
 * @brief      Reconnect the session
 * @details    Reconnect the current HDCP session in case Encrypt returned
               DX_HDCP_NO_ACTIVE_CONNECTION error
 * @param[in]  Current HDCP context
 * @return     HDCP_CP_ERROR code in FAILURE
 ***************************************************************************/
static HDCP_CP_ERRORTYPE WFD_HdcpCpReconnect(CWFD_HdcpCp *pHDCPContext)
{
  HDCP_CP_ERRORTYPE ulStatus = QC_HDCP_CP_ErrorUndefined;
  HDCP_CP_ERRORTYPE ulRetVal = QC_HDCP_CP_ErrorUndefined;
  //Init Session
  ulStatus = pHDCPContext->WFD_HdcpSessionInit();
  if( QC_HDCP_CP_ErrorNone == ulStatus )
  {
  // Create session
  ulStatus = pHDCPContext->WFD_HdcpCreateSession();
  if ( QC_HDCP_CP_ErrorNone == ulStatus)
  {
    //Open Stream
    ulStatus = pHDCPContext->WFD_HdcpOpenStream(VIDEO_STREAMID, STREAM_VIDEO);
    if(pHDCPContext->m_ucHdcpSecondaryStreamType == STREAM_AUDIO)
    {
      ulRetVal = pHDCPContext->WFD_HdcpOpenStream(AUDIO_STREAMID, STREAM_AUDIO);
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,"CWFD_HdcpSrcCp : OpenAudioStream");
    }
    if( QC_HDCP_CP_ErrorNone != ulStatus ||
      ((QC_HDCP_CP_ErrorNone != ulRetVal)&&
       (pHDCPContext->m_ucHdcpSecondaryStreamType == STREAM_AUDIO)))
    {
      MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_FATAL, "HDCP Open Stream Failed err"\
                                "Video = %d Audio err = %d",ulStatus, ulRetVal);
      // Close Session
      ulStatus = pHDCPContext->WFD_HdcpCloseSession();
      // Deinit
      ulStatus = pHDCPContext->WFD_HdcpSessionDeInit();
      // Notify Status
      pHDCPContext->m_eHdcpSessionStatus = HDCP_STATUS_FAIL;
    }
    else
    {
      pHDCPContext->m_eHdcpSessionStatus = HDCP_STATUS_INIT_COMPLETE;
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "HDCP_SRC_STATUS_INIT_COMPLETE");
    }
  }
 }
  return ulStatus;
}

/***************************************************************************
 * @brief      New hdcp events support
 * @details    Based on hdcp lib version decide if new events are supported
 * @param[in]  none
 * @return     true or false
 ***************************************************************************/
bool CWFD_HdcpCp::WFD_HDCP_Events_Supported()
{
   const char* pHdcpVersion = NULL;
   pHdcpVersion = DX_HDCP_Tsmt_Get_Version();

   if(pHdcpVersion == NULL)
   {
     return false;
   }
   int majorHigh = atoi(pHdcpVersion);

   pHdcpVersion = strchr(pHdcpVersion , '_');
   if(pHdcpVersion == NULL || std_strlen(pHdcpVersion) < 2)
   {
     return false;
   }
   pHdcpVersion++;
   int majorLow = atoi(pHdcpVersion);

   pHdcpVersion = strchr(pHdcpVersion , '_');
   if(pHdcpVersion == NULL || std_strlen(pHdcpVersion) < 2)
   {
     return false;
   }
   pHdcpVersion++;
   int minorHigh = atoi(pHdcpVersion);

   if(majorHigh < 2)
   {
    return false;
   }
   if(majorLow < 2)
   {
    return false;
   }
   if(minorHigh < 51)
   {
    return false;
   }
   return true;
}
#endif
