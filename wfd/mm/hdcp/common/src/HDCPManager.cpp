/*==============================================================================
*       HDCPManager.cpp
*
*  DESCRIPTION:
*       Manager class for HDCP
*
*
*  Copyright (c) 2012-2014 by Qualcomm Technologies, Inc. All Rights Reserved.
*  Qualcomm Technologies Proprietary and Confidential.
*===============================================================================
*/
/*==============================================================================
                             Edit History
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
#include "HDCPManager.h"
#include "MMDebugMsg.h"
#include "MMThread.h"
#include "MMTimer.h"
#include "MMCriticalSection.h"
#include "MMDebugMsg.h"
#include "MMSignal.h"

#ifdef WFD_HDCP_ENABLED
#include "DX_Hdcp_Receiver.h"
#include "DX_Hdcp_Repeater.h"
#include "DX_Hdcp_Errors.h"
#include "DX_VOS_Errors.h"
#include "hdcpmgr_api.h"
#include "DxTypes.h"
#include "DX_Hdcp_Types.h"
#endif

#include <stdlib.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <linux/if.h>

#define HDCP_DEBUG

#define SINK_VIDEO_TRACK_ID  0
#define SINK_AUDIO_TRACK_ID  1

const int HDCPManager::m_nHDCPManCloseEvent = 1;
const int HDCPManager::m_nHDCPManListenEvent = 2;
const int HDCPManager::m_nHDCPManConnectEvent = 3;
const int HDCPManager::m_nHDCPManDisconnectEvent = 4;

#define HDCPMAN_STACK_SIZE 65536

MM_HANDLE HDCPManager::m_hCriticalSect;

#define HDCPMAN_CRITICAL_SECT_ENTER     if(m_hCriticalSect)                   \
                                        {                                     \
                                   MM_CriticalSection_Enter(m_hCriticalSect); \
                                        }                                     \


#define HDCPMAN_CRITICAL_SECT_LEAVE     if(m_hCriticalSect)                   \
                                        {                                     \
                                   MM_CriticalSection_Leave(m_hCriticalSect); \
                                        }                                     \



/* =============================================================================

                              DATA DECLARATIONS

================================================================================
*/
/* -----------------------------------------------------------------------------
** Constant / Define Declarations
** -------------------------------------------------------------------------- */

/* ----------------------------------------------------------------------------
** Type Declarations
** -------------------------------------------------------------------------- */

/* -----------------------------------------------------------------------------
** Global Constant Data Declarations
** -------------------------------------------------------------------------- */
#define IP_ADDR_SIZE 4
#define SINK_HDCP_STATCK_SIZE 8*1024

static int localPort;
static unsigned char localIPAddr[IP_ADDR_SIZE];

// keeps track of topology update status
HDCPStatus HDCPManager::m_eTopologyStatus;
// keeps track of propagation status from DX
bool HDCPManager::m_bPropagationStatus = false;


/* =============================================================================
*                       Local Function Definitions
* =========================================================================== */

/* -----------------------------------------------------------------------------
** Local Data Declarations
** -------------------------------------------------------------------------- */


/* =============================================================================
**                          Macro Definitions
** ========================================================================== */
#define UNUSED(x) ((void)x)

/* =============================================================================
**                            Function Definitions
** ========================================================================== */


HDCPManager::HDCPManager()
{
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_DEBUG, "HDCPManager:constructor");
    m_eState = HDCP_STATE_DEINIT;
    initData();
    /**-------------------------------------------------------------------------
      If critical section create fails use other wait schemes
    ----------------------------------------------------------------------------
    */
    if(0 != MM_CriticalSection_Create(&m_hCriticalSect))
    {
        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR,
            "HDCPManager CriticalSect failed %p Try other options!!!",
            m_hCriticalSect);
        m_hCriticalSect = NULL;
    }
    return;
}

HDCPManager::~HDCPManager()
{
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_DEBUG, "HDCPManager:destructor");
    /**-------------------------------------------------------------------------
      If critical section create fails use other wait schemes
    ----------------------------------------------------------------------------
    */
    if(m_hCriticalSect)
    {
        if(0 != MM_CriticalSection_Release(m_hCriticalSect))
        {
            MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR,
                "HDCPManager CriticalSect Release failed %p !!!",
                m_hCriticalSect);
            m_hCriticalSect = NULL;
        }
    }
    return;
}

HDCPStatusType HDCPManager::initializeHDCPManager()
{
#ifdef WFD_HDCP_ENABLED
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "HDCPManager Initialize");
    HDCPMAN_CRITICAL_SECT_ENTER
    if(m_eState != HDCP_STATE_DEINIT)
    {
        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR,
                    "HDCP Manager Invalid State %d",m_eState);
        HDCPMAN_CRITICAL_SECT_LEAVE
        return HDCP_INVALID_STATE;
    }

    /**-------------------------------------------------------------------------
       Create the signal queue for HDCPMan thread.
    ----------------------------------------------------------------------------
    */
    if ( 0 != MM_SignalQ_Create( &m_pHDCPManSignalQ ) )
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
           "HDCPManager Signal Q create failed!!!");
        m_pHDCPManSignalQ = NULL;
        m_eState = HDCP_STATE_ERROR;
        HDCPMAN_CRITICAL_SECT_LEAVE
        return HDCP_FAIL;
    }
    /**-------------------------------------------------------------------------
       Create the signal for making connection for HDCP
    ----------------------------------------------------------------------------
    */
    if( 0 != MM_Signal_Create( m_pHDCPManSignalQ,
                                   (void *) &m_nHDCPManConnectEvent,
                                   NULL,
                                   &m_pConnectSignal ) )
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
           "HDCPManager Signal create failed!!!");
        m_pConnectSignal = NULL;
        m_eState = HDCP_STATE_ERROR;
        HDCPMAN_CRITICAL_SECT_LEAVE
        return HDCP_FAIL;
    }

    /**-------------------------------------------------------------------------
       Create the signal for Accepting incoming connection.
    ----------------------------------------------------------------------------
    */
    if( 0 != MM_Signal_Create( m_pHDCPManSignalQ,
                                   (void *) &m_nHDCPManListenEvent,
                                   NULL,
                                   &m_pListenSignal ) )
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
           "HDCPManager Signal create failed!!!");
        m_pListenSignal = NULL;
        m_eState = HDCP_STATE_ERROR;
        HDCPMAN_CRITICAL_SECT_LEAVE
        return HDCP_FAIL;
    }

    /**-------------------------------------------------------------------------
       Create the signal for Accepting incoming connection.
    ----------------------------------------------------------------------------
    */
    if( 0 != MM_Signal_Create( m_pHDCPManSignalQ,
                                   (void *) &m_nHDCPManDisconnectEvent,
                                   NULL,
                                   &m_pDisconnectSignal ) )
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
           "HDCPManager Signal create failed!!!");
        m_pDisconnectSignal = NULL;
        m_eState = HDCP_STATE_ERROR;
        HDCPMAN_CRITICAL_SECT_LEAVE
        return HDCP_FAIL;
    }

    /**-------------------------------------------------------------------------
       Create the signal for exiting session.
    ----------------------------------------------------------------------------
    */
    if( 0 != MM_Signal_Create( m_pHDCPManSignalQ,
                                   (void *) &m_nHDCPManCloseEvent,
                                   NULL,
                                   &m_pHDCPManCloseSignal ) )
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
           "HDCPManager Signal create failed!!!");
        m_pHDCPManCloseSignal = NULL;
        m_eState = HDCP_STATE_ERROR;
        HDCPMAN_CRITICAL_SECT_LEAVE
        return HDCP_FAIL;
    }

    /**-------------------------------------------------------------------------
       Create the thread for HDCP connect/Listen.
    ----------------------------------------------------------------------------
    */
    if (0 != MM_Thread_CreateEx(0,
                                0,
                                HDCPManager::threadEntry,
                                this,
                                HDCPMAN_STACK_SIZE,
                                "HDCPMan", &m_pHDCPManThread) )
    {
        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR,
           "HDCPManager Thread create failed %p!!!",m_pHDCPManThread);
        m_pHDCPManThread = NULL;
        m_eState = HDCP_STATE_ERROR;
        HDCPMAN_CRITICAL_SECT_LEAVE
        return HDCP_FAIL;
    }
    m_eState = HDCP_STATE_INIT;

    m_bAudioCodecInfoAvailable = false;
    m_bVideoCodecInfoAvailable = false;
    m_nStreamType = 0;
    m_eTopologyStatus = HDCP_IN_PROGRESS;
    m_bPropagationStatus = false;
    HDCPMAN_CRITICAL_SECT_LEAVE
    return HDCP_SUCCESS;
#else
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "HDCP Libraries not present");
    return HDCP_FAIL;
#endif
}

HDCPStatusType HDCPManager::deinitializeHDCPManager()
{
#ifdef WFD_HDCP_ENABLED
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                         "HDCPManager deinitializeHDCPManager");
    HDCPMAN_CRITICAL_SECT_ENTER
    if(m_eState == HDCP_STATE_DEINIT)
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
            "HDCPManager deinitializeHDCPManager Already Done");
        HDCPMAN_CRITICAL_SECT_LEAVE
        return HDCP_SUCCESS;
    }

    switch(m_eState)
    {
        case HDCP_STATE_PROCESSING:
        {
            /**-----------------------------------------------------------------
              In the middle of processing a sample. Wait for it to finish
            --------------------------------------------------------------------
            */
            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                         "HDCPMan Wait for sample to process");
            while(m_eState == HDCP_STATE_PROCESSING)
            {
                MM_Timer_Sleep(1);
            }
        }
        /** No Break is Intentional */
        case HDCP_STATE_CONNECTED:
        case HDCP_STATE_CONNECTING:
        {
            teardownHDCPSession();
            /**-----------------------------------------------------------------
             Wait for HDCP Session to disconnect
            --------------------------------------------------------------------
            */
            while(m_eState != HDCP_STATE_INIT)
            {
                MM_Timer_Sleep(1);
            }
        }
        /** No Break is Intentional */
        case HDCP_STATE_INIT:
        {
            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                        "HDCPMAnager wait for thread to exit");
            /**-----------------------------------------------------------------
               If thread has been started make the thread exit
            --------------------------------------------------------------------
            */
            if(m_pHDCPManCloseSignal && m_pHDCPManThread)
            {
                int exitCode = 0;
                MM_Signal_Set( m_pHDCPManCloseSignal );
                MM_Thread_Join(m_pHDCPManThread, &exitCode );
                MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                                                  "HDCPManager Thread Join-ed");
            }

            if(m_pHDCPManThread)
            {
                MM_Thread_Release(m_pHDCPManThread);
                MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                                                "HDCPManager Thread Released");
                m_pHDCPManThread = NULL;
            }

            if(m_pHDCPManCloseSignal)
            {
                MM_Signal_Release(m_pHDCPManCloseSignal);
                m_pHDCPManCloseSignal = NULL;
            }
            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                        "HDCPManager Close Signal Released");

            if(m_pListenSignal)
            {
                MM_Signal_Release(m_pListenSignal);
                m_pListenSignal = NULL;
            }
            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                        "HDCPManager Listen Signal Released");

            if(m_pConnectSignal)
            {
                MM_Signal_Release(m_pConnectSignal);
                m_pConnectSignal = NULL;
            }
            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                        "HDCPManager Connect Signal Released");
            if(m_pDisconnectSignal)
            {
                MM_Signal_Release(m_pDisconnectSignal);
                m_pDisconnectSignal = NULL;
            }
            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                        "HDCPManager Disconnect Signal Released");

            if(m_pHDCPManSignalQ)
            {
                MM_SignalQ_Release(m_pHDCPManSignalQ);
                m_pHDCPManSignalQ = NULL;
            }

            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                        "HDCPManager SignalQ Released");


            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                        "HDCPMAnager all threads and resources cleared");
        }
        default:
            ;
    }
    m_eState = HDCP_STATE_DEINIT;
    m_bAudioCodecInfoAvailable = false;
    m_bVideoCodecInfoAvailable = false;
    m_nStreamType = 0;
    m_eTopologyStatus = HDCP_INVALID_STATE;
    m_bPropagationStatus = false;
    HDCPMAN_CRITICAL_SECT_LEAVE

    return HDCP_SUCCESS;
#else
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "HDCP Libraries not present");
    return HDCP_FAIL;
#endif
}

HDCPStatusType HDCPManager::registerCallback
(
    HDCPEventCbType pCb,
    void *pUsrData
)
{
    if(!pCb)
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
               "HDCPManager registerCallback bad params");
        return HDCP_BAD_PARAMS;
    }

    m_pEventCb = pCb;
    m_pUsrData = pUsrData;
    return HDCP_SUCCESS;
}


HDCPStatusType HDCPManager::setupHDCPSession
(
    HDCPModeType eMode,
    char *pIpAddr,
    unsigned short portNum
)
{
    UNUSED(eMode);
    UNUSED(pIpAddr);
    UNUSED(portNum);
#ifdef WFD_HDCP_ENABLED
    MM_MSG_PRIO3(MM_GENERAL, MM_PRIO_HIGH,
                    "HDCPManager Setup HDCP Session %d %s %d",
                    eMode, pIpAddr, portNum);
    HDCPMAN_CRITICAL_SECT_ENTER

    if(m_eState != HDCP_STATE_INIT)
    {
        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR,
                    "HDCP Manager Invalid State %d",m_eState);
        HDCPMAN_CRITICAL_SECT_LEAVE
        return HDCP_INVALID_STATE;
    }

    if(!portNum)
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                     "HDCP Manager Invalid Port Num");

        HDCPMAN_CRITICAL_SECT_LEAVE

        return HDCP_BAD_PARAMS;
    }
    m_eTopologyStatus = HDCP_IN_PROGRESS;
    m_bPropagationStatus = false;
    m_nRetryCount = 0;

    if(pIpAddr)
    {
        m_ipAddr = inet_addr(pIpAddr);


        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,
                     "IP address received %d",m_ipAddr);

        if((signed int)m_ipAddr == -1) m_ipAddr = 0;

        if(m_ipAddr)
        {
            m_ipAddrArray[3] = (uint8_t)(m_ipAddr >> 24);
            m_ipAddrArray[2] = (uint8_t)(m_ipAddr >> 16);
            m_ipAddrArray[1] = (uint8_t)(m_ipAddr >> 8);
            m_ipAddrArray[0] = (uint8_t)(m_ipAddr);
        }
    }
    m_nPortNum = portNum;
    /**-------------------------------------------------------------------------
     If local device is a transmitter we need IP address
    ----------------------------------------------------------------------------
    */
    if(eMode == HDCP_MODE_TX)
    {
        if(!pIpAddr || !portNum)
        {
            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                         "HDCP Manager Invalid IP Addr/Port Num");
            HDCPMAN_CRITICAL_SECT_LEAVE

            return HDCP_BAD_PARAMS;
        }
    }

    /**-------------------------------------------------------------------------
     Now we'll attempt to make a connection asynch.ly
    ----------------------------------------------------------------------------
    */
    m_eState = HDCP_STATE_CONNECTING;

    if(eMode == HDCP_MODE_TX)
    {
        if(m_pConnectSignal)
        {
            MM_Signal_Set(m_pConnectSignal);
        }
    }
    else if(eMode == HDCP_MODE_RX)
    {
        if(m_nPortNum && !m_ipAddr)
        {
            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                                    "Setup HDCP, Find local IpAddr");
            /**-----------------------------------------------------------------
              For Listen we need the local Ip address of the device
            --------------------------------------------------------------------
            */ 
            struct ifreq ifr;
            int s;
            unsigned int addr;

            memset(&ifr, 0, sizeof(struct ifreq));
            strlcpy(ifr.ifr_name, "p2p0", IFNAMSIZ);

            ifr.ifr_name[IFNAMSIZ-1] = 0;

            if((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
            {
                MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,
                               "SetupHDCPSession :Error in creating socket");
            }
            else
            {
                if (ioctl(s, SIOCGIFADDR, &ifr) < 0)
                {
                    MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,
                               "SetupHDCPSession :Error in getting local IP");
                }
                else
                {
                    addr = ((struct sockaddr_in *)&ifr.ifr_addr)
                                                           ->sin_addr.s_addr;
                    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR,
                                 "SetupHDCPSession :addr[%x]",addr);

                    m_ipAddrArray[3]= static_cast<unsigned char>((addr >> 24) & 0xff);
                    m_ipAddrArray[2]= static_cast<unsigned char>((addr >> 16) & 0xff);
                    m_ipAddrArray[1]= static_cast<unsigned char>((addr >> 8) & 0xff);
                    m_ipAddrArray[0]= static_cast<unsigned char>(addr & 0xff);
                    MM_MSG_PRIO4(MM_GENERAL, MM_PRIO_HIGH,
                            "SetupHDCPSession :local IP Address %d.%d.%d.%d",
                             m_ipAddrArray[0], m_ipAddrArray[1],
                             m_ipAddrArray[2], m_ipAddrArray[3]);
                    m_ipAddr = (((unsigned int)m_ipAddrArray[3])  << 24) |
                               (((unsigned int)m_ipAddrArray[2])  << 16) |
                               (((unsigned int)m_ipAddrArray[1])  << 8)  |
                               ((unsigned int)m_ipAddrArray[0]);
               }
           }
        }

        if(!m_ipAddr)
        {
            m_ipAddr = inet_addr("127.0.0.1");
            m_ipAddrArray[3] = (uint8_t)(m_ipAddr >> 24);
            m_ipAddrArray[2] = (uint8_t)(m_ipAddr >> 16);
            m_ipAddrArray[1] = (uint8_t)(m_ipAddr >> 8);
            m_ipAddrArray[0] = (uint8_t)(m_ipAddr);
        }
        if(m_pListenSignal)
        {
            MM_Signal_Set(m_pListenSignal);
        }
    }
    m_eMode = eMode;
    HDCPMAN_CRITICAL_SECT_LEAVE
    return HDCP_SUCCESS;
#else
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "HDCP Libraries not present");
    return HDCP_FAIL;
#endif
}

void HDCPManager::initData()
{
    m_pHDCPManCloseSignal = NULL;
    m_pHDCPManThread = NULL;
    m_pHDCPManSignalQ = NULL;
    m_pHDCPManCloseSignal = NULL;
    m_pListenSignal = NULL;
    m_pConnectSignal = NULL;
    m_pDisconnectSignal = NULL;
    m_nPortNum = 0;
    m_ipAddr = 0;
    m_hCriticalSect = NULL;
    fp = NULL;
    m_eTopologyStatus = HDCP_INVALID_STATE;
    m_bAudioCodecInfoAvailable = false;
    m_bVideoCodecInfoAvailable = false;
    m_nAudioStreamType = 0;
    m_nVideoStreamType = 0;
    m_nStreamType = 0;
    m_bPropagationStatus= false;
    m_nRetryCount = 0;

}

HDCPStatusType HDCPManager::teardownHDCPSession
(
)
{
#ifdef WFD_HDCP_ENABLED
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                 "HDCPManager teardowHDCPSession");
    HDCPMAN_CRITICAL_SECT_ENTER

    if(m_eState == HDCP_STATE_INIT ||
       m_eState == HDCP_STATE_DEINIT)
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                "HDCPManager teardown: Already In Init Return");
        HDCPMAN_CRITICAL_SECT_LEAVE
        return HDCP_SUCCESS;
    }

    if(m_pDisconnectSignal)
    {
        m_eState = HDCP_STATE_DISCONNECTING;
        m_eTopologyStatus = HDCP_UPSTREAM_CLOSE;
        unsigned long nStatus;

        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
          "HDCPManager:teardownHDCPSession Dx Close Session");

        nStatus = DX_HDCP_Rpt_Close_Session();

        if(nStatus)
        {
           MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR,
            "HDCPManager:teardownHDCPSession DxClose Session failed %lu",
             nStatus);
        }

        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
            "HDCPManager:teardownHDCPSession Dx Close");

        nStatus = DX_HDCP_Rpt_Close();

        if(nStatus)
        {
           MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR,
              "HDCPManager:teardownHDCPSession DxClose failed %lu",
               nStatus);
        }

        MM_Signal_Set(m_pDisconnectSignal);
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                "HDCPManager Waiting for disconnect");
        while(m_eState != HDCP_STATE_INIT)
        {
            MM_Timer_Sleep(5);
        }
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                "HDCPManager Waiting for disconnect Done!!");
    }
    else
    {
        m_eState = HDCP_STATE_INIT;
    }
    m_bAudioCodecInfoAvailable = false;
    m_bVideoCodecInfoAvailable = false;
    m_nStreamType = 0;
    m_bPropagationStatus= false;
    m_nRetryCount = 0;

    HDCPMAN_CRITICAL_SECT_LEAVE
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                "HDCPManager teardowHDCPSession Done");
    return HDCP_SUCCESS;
#else
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "HDCP Libraries not present");
    return HDCP_FAIL;
#endif
}


/*==============================================================================

         FUNCTION:         threadEntry

         DESCRIPTION:
*//**       @brief         Entry function to HDCP Thread

*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         void pointer to the instance

*//*     RETURN VALUE:
*//**       @return
                           0


@par     SIDE EFFECTS:
                           None

*//*==========================================================================*/
int HDCPManager::threadEntry( void* ptr )
{
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "HDCPManager::ThreadEntry");
    HDCPManager* pThis = (HDCPManager *) ptr;

    if ( NULL != pThis )
    {
        pThis->HDCPManThread();
    }
    return 0;
}

/*==============================================================================

         FUNCTION:         HDCPManThread

         DESCRIPTION:
*//**       @brief         Actual processing function for HDCP thread

*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         None

*//*     RETURN VALUE:
*//**       @return
                           None


@par     SIDE EFFECTS:
                           None

*//*==========================================================================*/
void HDCPManager::HDCPManThread( void )
{
    bool bRunning = true;
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "HDCPManager::HDCPManThread");
    unsigned int *pEvent = NULL;
/*
    if(m_eState != HDCP_STATE_INIT)
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "Invalid State Thread Exit");
        MM_Thread_Exit( m_pHDCPManThread, 0 );
        m_eState = HDCP_STATE_ERROR;
        return;
    }
*/
    while ( bRunning )
    {
        /* Wait for a signal to be set on the signal Q. */
        if ( 0 == MM_SignalQ_Wait(m_pHDCPManSignalQ, (void **) &pEvent))
        {
            switch ( *pEvent )
            {
                case m_nHDCPManConnectEvent:
                {
                    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                       "HDCPManager::HDCPManThread received ConnectEvent");
                    //TODO
                }
                break;
                case m_nHDCPManListenEvent:
                {
#ifdef WFD_HDCP_ENABLED
                   int ret = 0;
                   //if(HDCP1X_COMM_hdmi_status())
                   {
                       MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                               "HDCPManager:setupHDCPSession, Disp HDCP Manager Init");
                       ret = HDCP1X_COMM_Init(&DISPCallBack);
                       if (ret)
                       {
                           MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR,
                                                "hdcp manager init fail (%d)!",
                                                ret);
                       }
                       else
                       {
                           MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,
                                                "hdcp managet init success (%d)!",
                                                ret);
                       }
                   }

                    unsigned long nStatus;
                    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                        "HDCPManager::HDCPManThread  received ListenEvent");

                    if(!m_ipAddr || !m_nPortNum)
                    {
                        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                           "HDCPManager::HDCPManThread  Invalid IP POrt");
                        continue;
                    }

                    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                                                  "HDCPManager: Init DX");

                    nStatus = DX_HDCP_Rpt_Init(
                         (void (*)(EDxHdcpEventType, void*, void*))DXCallBack);

                     if(nStatus)
                     {
                         MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR,
                                 "HDCPManager:Rpt DX Init failed %ld",nStatus);
                         break;
                     }

                     m_bPropagationStatus = false;
                     m_nRetryCount = 0;
                     if (m_eTopologyStatus == HDCP_UPSTREAM_CLOSE)
                     {
                         /*In some use cases DX Init return little late
                           in mean time if there is a teardown request,
                           no need to proceed further.
                          */
                         MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,
                             "Disconnect issued, no need to proceed further!%d",
                              m_eTopologyStatus);
                         break;
                     }
                     m_eTopologyStatus = HDCP_IN_PROGRESS;

                     /*After DX init, send a request to display/hdmi with Request Topology
                         and we will be notified via the call back with the updated topology.
                         Wait till we recieve the CB from display after calling the send event
                         as DX update topology will be done in CB
                        */
                     if(HDCP1X_COMM_hdmi_status())
                     {
                       MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                                      "HDCPMANAGER HDMI Connected.");
                     }
                     else
                     {
                       MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                                      "HDCPMANAGER HDMI Not Connected.");
                     }
                     bool bSuccess = false;
                     // do not call DX API's
                     bool bContinueToDXCalls = false;
                     int nIteration = 0;
                     if(HDCP1X_COMM_hdmi_status())
                     {
                       MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                                      "HDCPMANAGER HDMI Connected Man thread");
                       int ret = HDCP1X_COMM_Send_hdcp2x_event(
                                      EV_REQUEST_TOPOLOGY, NULL, NULL);

                       if (ret)
                       {
                         m_eTopologyStatus = HDCP_BAD_PARAMS;
                         MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR,
                                      "hdcp manager send request fail! %d",
                                      ret);
                       }
                       else
                       {
                         m_eTopologyStatus = HDCP_IN_PROGRESS;
                         MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,
                                      "hdcp manager send request success! %d",
                                      ret);
                       }
                     }
                     else
                     {
                       bSuccess = true;
                       bContinueToDXCalls = true;
                     }

                     /* Warning!: In some use cases, DIsplay HDCP manager can give us callback little late depending on
                        when it authenticated the device and so on. So we might have to wait here till
                        we are notified on the Authentication details.
                      */
                     while (!bSuccess) // Loop here till we get the CB from display
                     {
                         // request sent for topology update. Wait till we recieve an update from the display
                         // then it is set as part of Topology update with in DX
                         MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM,
                                                  "%d: request_topology !",
                                                  m_eTopologyStatus);

                         if (m_eTopologyStatus == HDCP_SUCCESS)
                         {
                             bSuccess = true;
                             bContinueToDXCalls = true;
                             break; //
                         }
                         else if (m_eTopologyStatus == HDCP_IN_PROGRESS)
                         {
                             if (nIteration > 100)
                             {
                                 bSuccess = true;
                                 break;
                             }
                             // insteadof a tight loop sleep for 5/10 milli sec and then once again check
                             MM_Timer_Sleep(5);
                             nIteration++;
                             continue; // may be we have not recieved the CB from Display
                         }
                         else if ((m_eTopologyStatus ==  HDCP_FAIL)                       ||
                                  (m_eTopologyStatus ==  HDCP_UNAUTHENTICATED_CONNECTION) ||
                                  (m_eTopologyStatus ==  HDCP_UNAUTHORIZED_CONNECTION) ||
                                  (m_eTopologyStatus ==  HDCP_UPSTREAM_CLOSE))
                         {
                             bSuccess = true; // come out of loop
                             MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR,
                                 "Auth failure/disconnect/update topology failed %ld",
                                 nStatus);
                             break;
                         }
                         else if (m_eTopologyStatus == HDCP_BAD_PARAMS)
                         {
                             bSuccess = true;
                             //bContinueToDXCalls = true;
                             break;
                         }
                     }


                    if (!bContinueToDXCalls)
                    {
                        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR,
                           "Auth might have failed, do not continue %d",
                           bContinueToDXCalls);
                        break; // come out of it
                    }

                    while(m_eState == HDCP_STATE_CONNECTING)
                    {
                        MM_MSG_PRIO4(MM_GENERAL, MM_PRIO_HIGH,
                            "HDCPManager Lsten :local IP Address %d.%d.%d.%d",
                             m_ipAddrArray[0], m_ipAddrArray[1],
                             m_ipAddrArray[2], m_ipAddrArray[3]);

                       nStatus = DX_HDCP_Rpt_Listen(m_ipAddrArray,
                                                    (uint16_t)m_nPortNum);
                        if(nStatus)
                        {
                            MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR,
                                   "HDCPManager:Listen failed %ld",nStatus);
                        }
                        else
                        {
                            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                                   "HDCPManager:Listen SUCCESS");
                            m_eState = HDCP_STATE_CONNECTED;
                            break;
                        }
                    }
#else
                    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "HDCP Libraries not present");
#endif
                }
                break;
                case m_nHDCPManDisconnectEvent:
                {
#ifdef WFD_HDCP_ENABLED
                    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                                "HDCPManager Disconnect called");
                    m_eState = HDCP_STATE_INIT;
                    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                                "HDCPManager State changed to INIT");
                    if(HDCP1X_COMM_hdmi_status())
                    {
                      int ret = HDCP1X_COMM_Term();
                      if (ret)
                      {
                        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR,
                                                 "hdcp manager terminate fail ! %d",
                                                 ret);

                      }
                      else
                      {
                        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,
                                                 "hdcp manager terminate success! %d",
                                                 ret);
                      }
                    }

#else
                    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "HDCP Libraries not present");
#endif
                }

                break;
                case m_nHDCPManCloseEvent:
                {
                    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                                   "HDCPManager Close");
                    bRunning = false;
                    m_eState = HDCP_STATE_DEINIT;
                    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                                   "HDCPManager Closed");
                }
                break;
                default:
                {
                    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                       "HDCPManager  received Unknown Event");
                    /* Not a recognized event, ignore it. */
                }
            }
        }
    }
    MM_Thread_Exit(m_pHDCPManThread, 0);
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,"HDCPManager Exited");
    return;
}

HDCPStateType HDCPManager::getHDCPManagerState()
{
    HDCPStateType eState;
    HDCPMAN_CRITICAL_SECT_ENTER
    eState = m_eState;
    HDCPMAN_CRITICAL_SECT_LEAVE;
    return eState;
}

/*==========================================================================
   FUNCTION     : Decrypt

   DESCRIPTION  : Decrypt the encrypted content

   PARAMETERS   :

   PES private data - 128 bit private data filled in encrytion process

   msgIn            - Encrypted PES payload

   msgOut           - Encrypted PES payload

   msgLen           - Buffer length

   Return Value : return 0 for success else Dx_HDCP error code
===========================================================================*/
HDCPStatusType HDCPManager::decrypt(const unsigned char *pesPrivateData,
                                   const unsigned char *pIn,
                                   unsigned char *pOut,
                                   unsigned long nLen)
{
    UNUSED(pesPrivateData);
    UNUSED(pIn);
    UNUSED(pOut);
    UNUSED(nLen);
#ifdef WFD_HDCP_ENABLED
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_DEBUG, "HDCPManager:Decrypt");

    if(m_eState != HDCP_STATE_CONNECTED &&
       m_eState != HDCP_STATE_PROCESSING)
    {
        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR,
                     "HDCPManager:Decrypt in Invalid State %d", m_eState);
        return HDCP_INVALID_STATE;
    }

    HDCPMAN_CRITICAL_SECT_ENTER
    else
    {
        if(m_eState == HDCP_STATE_PROCESSING)
        {
            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                         "HDCPManager:Decrypt in Processing");
            while(m_eState == HDCP_STATE_PROCESSING)
            {
                MM_Timer_Sleep(1);
            }
        }
    }

    m_eState = HDCP_STATE_PROCESSING;

    /* If authentication fails, then no need to proceed with decrypt,
       report Auth failure to Client and Sink can start teardown process
     */
    if (m_eTopologyStatus == HDCP_UNAUTHENTICATED_CONNECTION)
    {
        HDCPMAN_CRITICAL_SECT_LEAVE;
        return HDCP_UNAUTHENTICATED_CONNECTION;
    }

    /* After DX Update topology, the topology information is propagated
       with in DX and DX notifies us with CIPHER_DISABLED and we should
       not call Decrypt API at that point. "m_bPropagationStatus"
       variable is set in this case. Once CIPHER_ENABLED is notified
       that means updation of topology is done properly and we should
       continue with decrypt.
     */
    while (m_bPropagationStatus)
    {
        MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_HIGH,
                     "propogation status %d mRetryCount (%d) !",
                     m_bPropagationStatus,
                     m_nRetryCount);
        // retry for some time if still didn't gothough report and error
        if (m_nRetryCount > 50)
        {
            HDCPMAN_CRITICAL_SECT_LEAVE;
            return HDCP_UNAUTHENTICATED_CONNECTION;
        }
        m_nRetryCount++;
        // sleep for 5 ms to make sure if propogation is disbaled or not
        MM_Timer_Sleep(5);
    }

    m_nRetryCount = 0;
    unsigned long nStatus = 0 ; // Success
    unsigned char priv[16];
    if( pIn && pOut && nLen )
    {
#ifdef HDCP_DEBUG
        if(pesPrivateData)
        {
            for(int i = 0; i < 16; i++)
            {
                MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,
                 "WFD_HdcpDRpt_Decrypt-Extra data-- : =%x",
                  pesPrivateData[i]);
            }
        }
#endif

        unsigned long starttime;
        unsigned long endtime;
        EDxHdcpStreamType lStream;

        if (m_nStreamType == 1)
        {
            lStream = (EDxHdcpStreamType)m_nAudioStreamType;
        }
        else
        {
            m_nVideoStreamType = ((HDCP_CODEC_AVC << 16) | DX_HDCP_STREAM_TYPE_VIDEO);
            m_bVideoCodecInfoAvailable = true;
            lStream = (EDxHdcpStreamType)m_nVideoStreamType;
        }

        MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_HIGH,
                     "HDCPManager::decrypt of Stream (%d) Codec (%d)",
                     (unsigned short)lStream,
                     (unsigned short)(lStream >> 16));


        MM_Time_GetTime(&starttime);


        nStatus = DX_HDCP_Rpt_Decrypt2(
                                       pesPrivateData,
                                      (uint64_t)pIn,
                                      (uint64_t)pOut,
                                      nLen,
                                      0,//offset 0
                                      lStream,
                                      DX_HDCP_BUFFER_TYPE_SHARED_MEMORY_HANDLE,
                                      DX_HDCP_BUFFER_TYPE_SHARED_MEMORY_HANDLE);
        MM_Time_GetTime(&endtime);
        ALOGE("HDCPManager Time Taken Decrypt = %lu", endtime-starttime);
        if(nStatus)
        {
            MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR,
               "HDCPManager:  Rpt_Decrypt failed %ld",nStatus);
            HDCPMAN_CRITICAL_SECT_LEAVE
            m_eState = HDCP_STATE_CONNECTED;
            return HDCP_FAIL;
        }
    }
    else
    {
        MM_MSG_PRIO4(MM_GENERAL, MM_PRIO_ERROR,
               "HDCPManager:Decrypt Err Invalid Args,PES Private Data"
                     " = 0x%p, pIn = 0x%p, pOut = 0x%p, pLen = %lu",
                                     pesPrivateData, pIn, pOut, nLen );
    }
    m_eState = HDCP_STATE_CONNECTED;
    HDCPMAN_CRITICAL_SECT_LEAVE
#else
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "HDCP Libraries not present");
#endif
    return HDCP_SUCCESS;
}

/*==========================================================================
   FUNCTION     : Decrypt

   DESCRIPTION  : Decrypt the encrypted content

   PARAMETERS   :

   PES private data - 128 bit private data filled in encrytion process

   msgIn            - Encrypted PES payload

   msgOut           - Encrypted PES payload

   msgLen           - Buffer length

   nStreamId        - Stream information Audio or Video

   Return Value : return 0 for success else Dx_HDCP error code
===========================================================================*/
HDCPStatusType HDCPManager::decrypt(const unsigned char *pesPrivateData,
                                   const unsigned char *pIn,
                                   unsigned char *pOut,
                                   unsigned long nLen,
                                   int nStreamId)
{
    UNUSED(pesPrivateData);
    UNUSED(pIn);
    UNUSED(pOut);
    UNUSED(nLen);
    UNUSED(nStreamId);
#ifdef WFD_HDCP_ENABLED
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_DEBUG, "HDCPManager:Decrypt");

    if(m_eState != HDCP_STATE_CONNECTED &&
       m_eState != HDCP_STATE_PROCESSING)
    {
        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR,
                     "HDCPManager:Decrypt in Invalid State %d", m_eState);
        return HDCP_INVALID_STATE;
    }

    HDCPMAN_CRITICAL_SECT_ENTER
    else
    {
        if(m_eState == HDCP_STATE_PROCESSING)
        {
            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                         "HDCPManager:Decrypt in Processing");
            while(m_eState == HDCP_STATE_PROCESSING)
            {
                MM_Timer_Sleep(1);
            }
        }
    }

    m_eState = HDCP_STATE_PROCESSING;

    /* If authentication fails, then no need to proceed with decrypt,
       report Auth failure to Client and Sink can start teardown process
     */
    if (m_eTopologyStatus == HDCP_UNAUTHENTICATED_CONNECTION)
    {
        HDCPMAN_CRITICAL_SECT_LEAVE;
        return HDCP_UNAUTHENTICATED_CONNECTION;
    }

    /* After DX Update topology, the topology information is propagated
       with in DX and DX notifies us with CIPHER_DISABLED and we should
       not call Decrypt API at that point. "m_bPropagationStatus"
       variable is set in this case. Once CIPHER_ENABLED is notified
       that means updation of topology is done properly and we should
       continue with decrypt.
     */
    while (m_bPropagationStatus)
    {
        MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_HIGH,
                     "propogation status %d mRetryCount (%d) !",
                     m_bPropagationStatus,
                     m_nRetryCount);
        // retry for some time if still didn't gothough report and error
        if (m_nRetryCount > 50)
        {
            HDCPMAN_CRITICAL_SECT_LEAVE;
            return HDCP_UNAUTHENTICATED_CONNECTION;
        }
        m_nRetryCount++;
        // sleep for 5 ms to make sure if propogation is disbaled or not
        MM_Timer_Sleep(5);
    }

    m_nRetryCount = 0;
    unsigned long nStatus = 0 ; // Success
    unsigned char priv[16];
    if( pIn && pOut && nLen )
    {
#ifdef HDCP_DEBUG
        if(pesPrivateData)
        {
            for(int i = 0; i < 16; i++)
            {
                MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,
                 "WFD_HdcpDRpt_Decrypt-Extra data-- : =%x",
                  pesPrivateData[i]);
            }
        }
#endif

        EDxHdcpStreamType lStream = DX_HDCP_STREAM_TYPE_UNKNOWN;

        if (nStreamId == HDCP_AUDIO_TRACK_ID)
        {
            lStream = (EDxHdcpStreamType)m_nAudioStreamType;
        }
        else if (nStreamId == HDCP_VIDEO_TRACK_ID)
        {
            /*m_nVideoStreamType = ((HDCP_CODEC_AVC << 16) | DX_HDCP_STREAM_TYPE_VIDEO);
            m_bVideoCodecInfoAvailable = true;*/
            lStream = (EDxHdcpStreamType)m_nVideoStreamType;
        }

        MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_HIGH,
                     "HDCPManager::decrypt of Stream (%d) Codec (%d)",
                     (unsigned short)lStream,
                     (unsigned short)(lStream >> 16));

        nStatus = DX_HDCP_Rpt_Decrypt2(
                                      pesPrivateData,
                                      (uint64_t)pIn,
                                      (uint64_t)pOut,
                                      nLen,
                                      0,//offset 0
                                      lStream,
                                      DX_HDCP_BUFFER_TYPE_SHARED_MEMORY_HANDLE,
                                      DX_HDCP_BUFFER_TYPE_SHARED_MEMORY_HANDLE);

        if(nStatus)
        {
            MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR,
               "HDCPManager:  Rpt_Decrypt failed %ld",nStatus);
            HDCPMAN_CRITICAL_SECT_LEAVE
            m_eState = HDCP_STATE_CONNECTED;
            return HDCP_FAIL;
        }
    }
    else
    {
        MM_MSG_PRIO4(MM_GENERAL, MM_PRIO_ERROR,
               "HDCPManager:Decrypt Err Invalid Args,PES Private Data"
                     " = %p, pIn = %p, pOut = %p, pLen = %lu",
                                     pesPrivateData, pIn, pOut, nLen );
    }
    m_eState = HDCP_STATE_CONNECTED;
    HDCPMAN_CRITICAL_SECT_LEAVE
#else
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "HDCP Libraries not present");
#endif
    return HDCP_SUCCESS;
}


/*==========================================================================
   FUNCTION     : Set_Param

   DESCRIPTION  : Set Param to the HDCP Receiver

   PARAMETERS   :

         Input  : paramID

         Output : paramData returned by function

   Return Value : return 0 for success else Dx_HDCP error code
===========================================================================*/

HDCPStatusType HDCPManager::setParameter(int paramID, void *paramData)
{
    UNUSED(paramData);
    UNUSED(paramID);
 //   MM_MSG_PRIO(MM_GENERAL, MM_PRIO_DEBUG, "HDCPManager:   Set_Param");

 //   unsigned long nStatus = 0 ; // Success

//    nStatus = DX_HDCP_Rpt_Set_Parameter((EDxHdcpConfigParam)paramID,
//    paramData);

//    if(nStatus)
//    {
//        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR,
//                     "HDCPManager:   Set_Param failed %ld",nStatus);
//    }
//    return nStatus;
    return HDCP_SUCCESS;
}

/*==========================================================================
   FUNCTION     :   HDCP_Sink_Manager_CallBack

   DESCRIPTION  : Call back registered with HDCP in Init

   PARAMETERS   : HDCP Event Type

                  arg1 for passing any data

                  arg2 for passing any data

   Return Value : return 0 for success else Dx_HDCP error code
===========================================================================*/

void HDCPManager::DXCallBack(int status, void *arg1 , void *arg2)
{
    UNUSED(status);
    UNUSED(arg1);
    UNUSED(arg2);
#ifdef WFD_HDCP_ENABLED
   MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,
                  "HDCPManager: DXCallBack Entry %x", status);
   switch(status)
   {
      case DX_HDCP_EVENT_UPSTREAM_CLOSE:
         break;
      case DX_HDCP_EVENT_DOWNSTREAM_CLOSE:
         break;
      case DX_HDCP_EVENT_UNAUTHENTICATED_CONNECTION:
         break;
      case DX_HDCP_EVENT_UNAUTHORIZED_CONNECTION:
         break;
      case DX_HDCP_EVENT_REVOKED_CONNECTION:
         break;
      case DX_HDCP_EVENT_TOPOLOGY_EXCEED:
         break;
      case DX_HDCP_EVENT_INTERNAL_ERROR:
         break;
      case DX_HDCP_EVENT_CIPHER_ENABLED:
         m_bPropagationStatus = false;
         break;
      case DX_HDCP_EVENT_CIPHER_DISABLED:
        m_bPropagationStatus = true;
        break;
      default:
         MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                     "HDCPManager: DXCallBack Default Reached");
         break;
   }
#endif
}


/*==========================================================================
   FUNCTION     :   hdcp_mgr_notificationCB

   DESCRIPTION  : Call back to get display repeter events

   PARAMETERS   : Event Type
                   void *

   Return Value   : int
===========================================================================*/
int HDCPManager::DISPCallBack(int type, void *param)
{
    UNUSED(type);
    UNUSED(param);
#ifdef WFD_HDCP_ENABLED
    int ret = 0, *temp = NULL;
    //struct HDCP_V2V1_DS_TOPOLOGY *msg = NULL;
    HDCPMAN_CRITICAL_SECT_ENTER
    switch (type)
    {
        case EV_REQUEST_TOPOLOGY:
        case EV_SEND_TOPOLOGY:
            //msg = (struct HDCP_V2V1_DS_TOPOLOGY *)param;
            hdcp_update_topology(param);
            break;

        case EV_ERROR:
            temp = (int *)param;
            MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR,
                       "HDCPManager::DISPCallBack error = %d!",
                       *temp);
            m_eTopologyStatus = HDCP_UNAUTHENTICATED_CONNECTION;
            break;
        default :
            break;
    }
    HDCPMAN_CRITICAL_SECT_LEAVE

    return ret;
#else
    return 0;
#endif
}

/*==========================================================================
   FUNCTION     :   hdcp_update_topology

   DESCRIPTION  : Updates the topology information from Display to DX

   PARAMETERS   : void *


   Return Value : void
===========================================================================*/
void HDCPManager::hdcp_update_topology(void *msg1)
{
    UNUSED(msg1);
#ifdef WFD_HDCP_ENABLED
    struct HDCP_V2V1_DS_TOPOLOGY *msg = (struct HDCP_V2V1_DS_TOPOLOGY *)msg1;


    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,"dev_count = %d",msg->dev_count);
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,"depth = %d",msg->depth);


    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,"max_cascade_exceeded = %d",
                                           msg->max_cascade_exceeded);
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,"max_dev_exceeded = %d",
                                          msg->max_dev_exceeded);

    // CIPHER DISABLED
    if ((m_eTopologyStatus ==  HDCP_FAIL)                       ||
        (m_eTopologyStatus ==  HDCP_UNAUTHENTICATED_CONNECTION) ||
        (m_eTopologyStatus ==  HDCP_UNAUTHORIZED_CONNECTION) ||
        (m_eTopologyStatus ==  HDCP_UPSTREAM_CLOSE))
    {
       MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR,
                "HDCPManager: Rpt No need for update topo %d",
                 m_bPropagationStatus);

    }
    else
    {
        m_bPropagationStatus = true;
        /* Update the DX Topology */
        unsigned long nStatus = DX_HDCP_Rpt_Update_HDCP1_Topology(
                                 (DxHdcpReceiverId_t *)&msg->ksv_list[0],
                                  msg->dev_count,
                                  msg->depth,
                                  msg->max_cascade_exceeded,
                                  msg->max_dev_exceeded);

        if(nStatus)
        {
            MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR,
                     "HDCPManager: Rpt Update Topology failed %ld",
                     nStatus);
            m_eTopologyStatus = HDCP_FAIL;
        }
        else
        {
            MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,
                     "HDCPManager: Rpt Update Topology success %ld",
                     nStatus);
            m_eTopologyStatus = HDCP_SUCCESS;
        }
    }
#endif

}

/*==========================================================================
   FUNCTION     : setStreamType

   DESCRIPTION  : Sets the stream type, if Audio (1)/Video(0)

   PARAMETERS   : int StreamType


   Return Value : void
===========================================================================*/
void HDCPManager::setStreamType(int nStreamType)
{
   HDCPMAN_CRITICAL_SECT_ENTER
   m_nStreamType = nStreamType;
   MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,
                "HDCPManager::setStreamType mStreamType set to = %d",
                m_nStreamType);
   HDCPMAN_CRITICAL_SECT_LEAVE
}

/*==========================================================================
   FUNCTION     :   isCodecInfoSet

   DESCRIPTION  : MediaType, if Audio (1)/Video(0). Check if codec information is set or not

   PARAMETERS   : int Media Type


   Return Value : bool
===========================================================================*/
bool HDCPManager::isCodecInfoSet(int iMedia)
{
  if (iMedia == 1) // audio
  {
      if (m_bAudioCodecInfoAvailable) return true;

      return false;
  }

  if (m_bVideoCodecInfoAvailable) return true;

  return false;
}

/*==========================================================================
   FUNCTION     :   constructCodecAndStreamType

   DESCRIPTION  : Constuct the codec+stream information,
                  which is needed by DX Decrypt API

   PARAMETERS   : int Stream, char * (Codec information)


   Return Value : bool
===========================================================================*/
void HDCPManager::constructCodecAndStreamType(int nStream, char *msg2)
{
    UNUSED(nStream);
    UNUSED(msg2);
#ifdef WFD_HDCP_ENABLED
    HDCPMAN_CRITICAL_SECT_ENTER

    if ( msg2 != NULL)
    {
        MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_HIGH,
                    "HDCPManager::constructCodecAndStreamType [%d] -> [%s]",
                    nStream,msg2);

        if (nStream == 1) // Audio Stream
        {
            int a = atoi(msg2);

            if (a == 1) //WFD_AUDIO_LPCM
            {
                // then its a LPCM
                m_nAudioStreamType = ((HDCP_CODEC_LPCM << 16) | DX_HDCP_STREAM_TYPE_AUDIO);
                m_bAudioCodecInfoAvailable = true;
            }
            else if (a == 2) //WFD_AUDIO_AAC
            {
                // its AAC
                m_nAudioStreamType = ((HDCP_CODEC_AAC << 16) | DX_HDCP_STREAM_TYPE_AUDIO);
                m_bAudioCodecInfoAvailable = true;
            }
            else if (a == 3) //WFD_AUDIO_DOLBY_DIGITAL
            {
                // its AC3
                m_nAudioStreamType = ((HDCP_CODEC_AC3 << 16) | DX_HDCP_STREAM_TYPE_AUDIO);
                m_bAudioCodecInfoAvailable = true;
            }
            // we need to add other codecs here
        }
        else if (nStream == 0) // Video stream
        {
            int a = atoi((char *)msg2);

            if ( (a == 1)  ||    //WFD_VIDEO_H264
                 (a == 2))       //WFD_VIDEO_3D

            {
                m_nVideoStreamType = ((HDCP_CODEC_AVC << 16) | DX_HDCP_STREAM_TYPE_VIDEO);
                m_bVideoCodecInfoAvailable = true;
            }
        }
        else
        {
            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
               "HDCPManager::constructCodecAndStreamType Invalid Stream Type");
        }
    }
    else
    {
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
               "HDCPManager::constructCodecAndStreamType NULL");
    }
    HDCPMAN_CRITICAL_SECT_LEAVE
#endif
}


