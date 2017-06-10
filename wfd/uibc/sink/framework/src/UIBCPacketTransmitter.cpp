 /*==============================================================================
   *       UIBCPacketTransmitter.cpp
   *
   *  DESCRIPTION:
   *       Transmit the UIBC packets  to source device
   *
   *
   *  Copyright (c) 2011 - 2014 Qualcomm Technologies, Inc. All Rights Reserved.
   *  Qualcomm Technologies Proprietary and Confidential.
   *==============================================================================*/

/* =======================================================================
                             Edit History
========================================================================== */

/* =======================================================================
**               Includes and Public Data Declarations
** ======================================================================= */

/* ==========================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */
#define LOG_NDEBUG 0
#define LOG_NDDEBUG 0

#include <sys/socket.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <errno.h>
#include <linux/tcp.h>
#include "MMDebugMsg.h"
#include "MMThread.h"
#include "MMSignal.h"
#include "MMTimer.h"
#include "MMMalloc.h"
#include "UIBCPacketTransmitter.h"


/* ==========================================================================

                        DATA DECLARATIONS

========================================================================== */
/* -----------------------------------------------------------------------
** Constant / Define Declarations
** ----------------------------------------------------------------------- */

typedef union address
{
    struct sockaddr sa;
    struct sockaddr_in sa_in;
    struct sockaddr_in6 sa_in6;
    struct sockaddr_storage sa_stor;
}
address_t;

/* -----------------------------------------------------------------------
** Forward Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */


/* -----------------------------------------------------------------------
** Global Constant Data Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Global Data Declarations
** ----------------------------------------------------------------------- */

/* =======================================================================
**                          Macro Definitions
** ======================================================================= */
#define UIBC_TRANSIMITTER_THREAD_STACK_SIZE 16384

/* -----------------------------------------------------------------------
** Local Object Definitions
** ----------------------------------------------------------------------- */
const uint32 UIBCPacketTransmitter::THREAD_CONNECT_SIGNAL = 0;       //user data for m_pConnectSignal
const uint32 UIBCPacketTransmitter::THREAD_EXIT_SIGNAL = 1;        //user data for m_pExitSignal
const uint32 UIBCPacketTransmitter::THREAD_SEND_EVENT_SIGNAL = 2;  //user data for m_pSendEventSignal
const uint32 UIBCPacketTransmitter::THREAD_STOP_SEND_SIGNAL = 3;   //user data for m_pStopSendSignal


/* =======================================================================
**                            Function Definitions
** ======================================================================= */

/*==========================================================================
   FUNCTION     : constructor

   DESCRIPTION: constructor of the class

   PARAMETERS :  serverPort[in]:port number on which source accpets connection
                           sourceIPaddr[in]:id address of source

   Return Value  :
  ===========================================================================*/

UIBCPacketTransmitter::UIBCPacketTransmitter( uint16 sourcePort, uint32 sourceIPaddr, uint32 negotiated_height, uint32  negotiated_width )
  :m_nSourcePort(sourcePort),
   m_nSourceIPaddr(sourceIPaddr),
   m_negotiated_height(negotiated_height),
   m_negotiated_width(negotiated_width),
   m_eUIBCState(UIBC_NO_INIT),
   m_nSocket(-1),
   m_pUIBCThread(NULL),
   m_pSignalQ(NULL),
   m_pConnectSignal(NULL),
   m_pExitSignal(NULL),
   m_pSendEventSignal(NULL),
   m_pStopSendSingal(NULL),
   m_pConnectTimer(NULL),
   m_pUIBCPacketizer(NULL),
   m_pUIBCBuffer(NULL)
{
  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "UIBCPacketTransmitter:constructor");
  init();
}

UIBCPacketTransmitter::~UIBCPacketTransmitter()
{
  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "UIBCPacketTransmitter:destructor");
  release();
}


/*==========================================================================
   FUNCTION     : start

   DESCRIPTION: Initiate the connection

   PARAMETERS :  none

   Return Value  : return UIBC_SINK_SUCCESS for success else error code
  ===========================================================================*/
UIBC_sink_status_t UIBCPacketTransmitter::start()
{
  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "UIBCPacketTransmitter:start");
  init();
  UIBC_sink_status_t nStatus = UIBC_SINK_SUCCESS;

  if( m_eUIBCState == UIBC_NO_INIT || m_eUIBCState == UIBC_CONNECTED)
  {
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR, "UIBCPacketTransmitter:Invalid state %d for start", m_eUIBCState);
    nStatus = UIBC_SINK_ERROR_INVALID;
  }
  else
  {
    //sending start signal to thread
    if( 0 != MM_Signal_Set(m_pConnectSignal) )
    {
      nStatus = UIBC_SINK_ERROR_UNKNOWN;
    }
  }

  return nStatus;
}

/*==========================================================================
   FUNCTION     : stop

   DESCRIPTION: Stops the UIBC thread

   PARAMETERS :  none

   Return Value  : return UIBC_SINK_SUCCESS for success else error code
  ===========================================================================*/
UIBC_sink_status_t UIBCPacketTransmitter::stop()
{
  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "UIBCPacketTransmitter:stop");

  UIBC_sink_status_t nStatus = UIBC_SINK_SUCCESS;

  if( m_eUIBCState == UIBC_CONNECTED )
  {
    //sending stop signal to thread
    if( 0 != MM_Signal_Set(m_pStopSendSingal) )
    {
      nStatus = UIBC_SINK_ERROR_UNKNOWN;
    }
  }
  else
  {
     MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR,
               "UIBCInputReceiver:Invalid component state %d for Stop",
               m_eUIBCState);

     nStatus = UIBC_SINK_ERROR_INVALID;
  }

  return nStatus;
}

/*==========================================================================
   FUNCTION     : sendEvent

   DESCRIPTION: Queues the uibc event that need to sent to src

   PARAMETERS :  event - pointer to uibc event

   Return Value  : return UIBC_SINK_SUCCESS for success else error code
  ===========================================================================*/

UIBC_sink_status_t UIBCPacketTransmitter::sendEvent(WFD_uibc_event_t* event)
{
  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "UIBCPacketTransmitter:sendEvent");

  UIBC_sink_status_t nStatus = UIBC_SINK_SUCCESS;

  if( m_eUIBCState == UIBC_CONNECTED && event != NULL)
  {
    uibc_events_list_link_t* pLink = (uibc_events_list_link_t*)MM_Malloc(sizeof(uibc_events_list_link_t));

    memcpy(&(pLink->event), event, sizeof(WFD_uibc_event_t));

    qmm_ListPushFront( &m_hEventsList, &(pLink->link));

    MM_Signal_Set(m_pSendEventSignal);
  }
  else
  {
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR, "UIBCPacketTransmitter:sendEvent Invalid parm for state %d", m_eUIBCState);
    nStatus = UIBC_SINK_ERROR_INVALID;
  }

  return nStatus;
}


/*==========================================================================
  FUNCTION     : UIBCThreadEntry

  DESCRIPTION:  UIBC Packet transmitter worker thread entry function

  PARAMETERS :  ptr[in]:

  Return Value  : 0 for success

 ===========================================================================*/
int UIBCPacketTransmitter::uibcThreadEntry( void* ptr )
{
  UIBCPacketTransmitter* uibcPacketTransmitter = (UIBCPacketTransmitter*)ptr;
  uibcPacketTransmitter->uibcThreadWorker();

  return 0;
}

/*==========================================================================
  FUNCTION     : UIBCPacketTransmitter

  DESCRIPTION:  Worker method for UIBCPacketTransmitter thread.
                          stops execution only after sending THREAD_EXIT_SIGNAL

  PARAMETERS :  None

  Return Value  : None
 ===========================================================================*/
void UIBCPacketTransmitter::uibcThreadWorker()
{
  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "UIBCPacketTransmitter:worker thread started");

  boolean isRunning = TRUE;
  while(isRunning)
  {
    uint32 *pEvent = NULL;

    /* Peeking singal queue to check whether any signal is set */
    if ( 0 == MM_SignalQ_Wait( m_pSignalQ, (void **) &pEvent ) )
    {
      switch( *pEvent )
      {
        case THREAD_CONNECT_SIGNAL:
        {
          if (m_eUIBCState == UIBC_INIT)
          {
            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "UIBCPacketTransmitter:THREAD_CONNECT_SIGNAL received");
            int32 connectStatus = makeConnection();
            if( connectStatus == 0 )
            {
              m_eUIBCState = UIBC_CONNECTED;
              if(m_pConnectTimer != NULL)
              {
                MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "UIBCPacketTransmitter:connection success: releasing timer");
                MM_Timer_Release(m_pConnectTimer);
                m_pConnectTimer = NULL;
              }
            }
            else //if(connectStatus == ECONNREFUSED || connectStatus == ETIMEDOUT )
            {
              int32 duration = 100;
              MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR, " UIBCPacketTransmitter:Connect failed. Trying to connect again after %d ms", duration);
              if(m_pConnectTimer == NULL)
              {
                 MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "UIBCPacketTransmitter:connection failed: creation of  timer for reconnect");
                 if(0 != MM_Timer_Create( duration, 1, connectTimerHandler, (void *)this, &m_pConnectTimer))
                 {
                     MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"Creation of timer failed");
                 }
              }
            }
          }
          else if (m_eUIBCState == UIBC_STOP)
          {
             m_eUIBCState = UIBC_CONNECTED;
          }
          break;
        }

        case THREAD_EXIT_SIGNAL:
        {
          MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "UIBCPacketTransmitter:THREAD_EXIT_SIGNAL received");
          isRunning = FALSE;
          break;
        }

        case THREAD_SEND_EVENT_SIGNAL:
        {
          MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "UIBCPacketTransmitter:THREAD_SEND_EVENT_SIGNAL received");
          processEventsList();
          break;
        }

        case THREAD_STOP_SEND_SIGNAL:
        {
          MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "UIBCPacketTransmitter:THREAD_STOP_SEND_SIGNAL received");
          m_eUIBCState = UIBC_STOP;
          break;
        }

        default:
        {
          MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR, "UIBCPacketTransmitter:Invalid signal %d", *pEvent);
        }
      }
    }
  }

  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "UIBCPacketTransmitter:Exiting UIBC thread");
  m_eUIBCState = UIBC_NO_INIT;

  MM_Thread_Exit( m_pUIBCThread, 0 );
}

/*==========================================================================
   FUNCTION     : connectTimerHandler

   DESCRIPTION: sends the connect signal to uibc thread

   PARAMETERS :  ptr[in] -

   Return Value  :
  ===========================================================================*/

 void UIBCPacketTransmitter::connectTimerHandler( void* ptr)
{
  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "UIBCPacketTransmitter:connectTimerHandler");

  UIBCPacketTransmitter* uibcPacketTransmitter = (UIBCPacketTransmitter*)ptr;
  MM_Signal_Set(uibcPacketTransmitter->m_pConnectSignal);

}



/*==========================================================================
   FUNCTION     : makeConnection

   DESCRIPTION  : Establish the TCP connection with the source
                  The Source IP is supposed to be in Network Byte Order
                  The TCP Port is in Host order

   PARAMETERS   : None

   Return Value : the status of connect call (0 for success, errno for error)
  ===========================================================================*/
int32 UIBCPacketTransmitter::makeConnection()
{
   int32 connectStatus = 0;

   struct sockaddr_in addr;
   memset(&addr, 0, sizeof(sockaddr_in));

   addr.sin_addr.s_addr = m_nSourceIPaddr;
   addr.sin_family = AF_INET;
   addr.sin_port = htons(m_nSourcePort);

   MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_ERROR, "UIBCPacketTransmitter:make connection with port %hu, IP %u", m_nSourcePort, m_nSourceIPaddr);

   address_t tAddr;
   tAddr.sa_in = addr;
   if ( 0 != connect(static_cast<int>(m_nSocket), (struct sockaddr*)&tAddr.sa, static_cast<socklen_t>(sizeof(addr))) )
   {
     connectStatus = errno;
     MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_ERROR, " UIBCPacketTransmitter:socket connect failed with error %d %s",
                                               connectStatus,strerror(errno));

     /*
          *ToDo: Temporary change to handle EINPROGRESS...blocking select for 10 seconds...
                      (Need to loop select for multiple times and accept other signals, instead of blocking the UIBC thread)
          */
     if(connectStatus == EINPROGRESS)
     {
       MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, " UIBCPacketTransmitter:making select call");

       fd_set fdWriteSet;
       FD_ZERO(&fdWriteSet);
       FD_SET(m_nSocket, &fdWriteSet);

       struct timeval timeout;
       timeout.tv_sec = 0;//inseconds
       timeout.tv_usec = 100;//in micro seconds

       int32 selectStatus = select(static_cast<int>(m_nSocket+1), NULL, &fdWriteSet, NULL, &timeout);
       MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR, " UIBCPacketTransmitter:select return status %d",selectStatus);

       if(FD_ISSET(m_nSocket, &fdWriteSet))
       {
         MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, " UIBCPacketTransmitter:socket is written");
         int errorNum = 0;
         socklen_t size = sizeof(socklen_t);
         getsockopt(m_nSocket,SOL_SOCKET,SO_ERROR,(char*)(&errorNum), &size);

         if(errorNum == 0)
         {
           MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, " UIBCPacketTransmitter:socket connection established");
           connectStatus = 0;
         }
         else
         {
           MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_ERROR, " UIBCPacketTransmitter:socket connection failed with error %d %s",
                                                    errorNum, strerror(errno));
           connectStatus = errorNum;
         }
       }
       else
       {
         MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, " UIBCPacketTransmitter:select is not successful");
       }
     }
   }
   else
   {
     MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, " UIBCPacketTransmitter:socket connect SUCCEEDED");
   }

   return connectStatus;
}

/*==========================================================================
   FUNCTION     : processEventsList

   DESCRIPTION: Process the event queue and send the events

   PARAMETERS : none

   Return Value  : none
  ===========================================================================*/

void UIBCPacketTransmitter::processEventsList()
{
  //processing all the entries in the queue
  while(qmm_ListIsEmpty(&m_hEventsList) != 1)
  {
    uibc_events_list_link_t* pLink = NULL;

    qmm_ListPopRear(&m_hEventsList, (QMM_ListLinkType **)&pLink);

    if(pLink != NULL)
    {


      if(pLink->event.type == WFD_UIBC_ZOOM)
      {
         pLink->event.parms.zoom_event.coordinate_x *= static_cast<double>(m_negotiated_height);
         pLink->event.parms.zoom_event.coordinate_y *= static_cast<double>(m_negotiated_width);

      }
      if(pLink->event.type == WFD_UIBC_TOUCH)
      {
         int pointerIndex =0;
         while(pointerIndex <  pLink->event.parms.touch_event.num_pointers)
         {
             pLink->event.parms.touch_event.coordinate_x[pointerIndex] *= static_cast<double>(m_negotiated_width);
             pLink->event.parms.touch_event.coordinate_y[pointerIndex] *= static_cast<double>(m_negotiated_height);
             pointerIndex++;
         }

      }
      int32 packetLength = m_pUIBCPacketizer->constructUIBCPacket(&(pLink->event), m_pUIBCBuffer, UIBC_PACKET_MAX_LENGTH);

      if(packetLength > 0)
      {
        int32 numBytesSent = sendPacket(m_pUIBCBuffer, packetLength);
        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR, "UIBCPacketTransmitter:sendPacket:sent %d bytes", numBytesSent);
      }
      else
      {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "UIBCPacketTransmitter:Error in constructing packet");
      }

      MM_Free(pLink);
    }
  }
}

/*==========================================================================
   FUNCTION     : flushEventsList

   DESCRIPTION: flush all the entries in the event list

   PARAMETERS : none

   Return Value  : none
  ===========================================================================*/

void UIBCPacketTransmitter::flushEventsList()
{
  //free all the entries in the queue
  while(qmm_ListIsEmpty(&m_hEventsList) != 1)
  {
    uibc_events_list_link_t* pLink = NULL;

    qmm_ListPopRear(&m_hEventsList, (QMM_ListLinkType **)&pLink);

    if(pLink != NULL)
    {
      MM_Free(pLink);
    }
  }
}

/*==========================================================================
   FUNCTION     : sendPacket

   DESCRIPTION: send the packet over TCP

   PARAMETERS : pUIBCPacket[in] - pointer to uibc packet buffer
                          packetLength[in] - length of uibc packet

   Return Value  : number of bytes sent to socket
  ===========================================================================*/

int32 UIBCPacketTransmitter::sendPacket(uint8* pUIBCPacket, int32 packetLength )
{
  int32 nBytesSent = 0;

  nBytesSent = send(m_nSocket, pUIBCPacket, packetLength, 0);

  if( nBytesSent < packetLength)
  {
    int32 errorCode = errno;
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR, "UIBCPacketTransmitter:Error %d in sending packet", errorCode);
    MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_ERROR, "UIBCPacketTransmitter:Number of bytes requested for send %d, actual bytes send %d",packetLength, nBytesSent);
  }

  return nBytesSent;
}


/*==========================================================================
   FUNCTION     : init

   DESCRIPTION: Initializes the class parameters and create the thread to receive and transmit UIBC events.
                          Upon successful execution component will be moved to "UIBC_INIT" state.

   PARAMETERS :  none

   Return Value  : none
  ===========================================================================*/

void UIBCPacketTransmitter::init()
{
  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "UIBCPacketTransmitter:init");

  boolean initStatus = TRUE;

  if(m_eUIBCState == UIBC_NO_INIT)
  {
    m_pUIBCPacketizer = MM_New(UIBCPacketizer);

    m_pUIBCBuffer = (uint8*)MM_Malloc(UIBC_PACKET_MAX_LENGTH);

    m_nSocket = createSocket();
    if(m_nSocket < 0)
    {
      initStatus = FALSE;
    }

    if( initStatus && (qmm_ListInit(&m_hEventsList) != QMM_LIST_ERROR_NONE) )
    {
       MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "UIBCPacketTransmitter:Error in initializing list");
       initStatus = FALSE;
    }

    if( initStatus &&
          ((MM_SignalQ_Create(&m_pSignalQ) != 0) ||
           (MM_Signal_Create( m_pSignalQ, (void *) &THREAD_CONNECT_SIGNAL, NULL, &m_pConnectSignal) != 0) ||
           (MM_Signal_Create( m_pSignalQ, (void *) &THREAD_EXIT_SIGNAL, NULL, &m_pExitSignal) != 0) ||
           (MM_Signal_Create( m_pSignalQ, (void *) &THREAD_SEND_EVENT_SIGNAL, NULL, &m_pSendEventSignal) != 0)||
           (MM_Signal_Create( m_pSignalQ, (void *) &THREAD_STOP_SEND_SIGNAL, NULL, &m_pStopSendSingal) != 0)))
    {
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "UIBCPacketTransmitter:Error in creating signal queue");
      initStatus = FALSE;
    }

    if( initStatus &&
                MM_Thread_CreateEx(MM_Thread_DefaultPriority, 0, uibcThreadEntry, (void *) this,
                                    UIBC_TRANSIMITTER_THREAD_STACK_SIZE, "UIBCPacketTransmitter", &m_pUIBCThread) != 0)
    {
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "UIBCPacketTransmitter:Error in creating thread");
      initStatus = FALSE;
    }

    if( initStatus )
    {
      m_eUIBCState = UIBC_INIT;
    }
  }
}
/*==========================================================================
   FUNCTION     : CreateSocket

   DESCRIPTION: This function creates the socket

   PARAMETERS : None

   Return Value  : Returns the FD of socket created
  ===========================================================================*/

int32 UIBCPacketTransmitter::createSocket()
{
  int nSocket;

  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "UIBCPacketTransmitter:createSocket");
  nSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  if(nSocket < 0)
  {
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR, "UIBCPacketTransmitter:Error in creating socket %d",errno);
  }
  else
  {
    //making socket calls non-blocking
    int flags = fcntl(nSocket, F_GETFL, 0);
    fcntl(nSocket, F_SETFL, flags | O_NONBLOCK);
    flags = 1;
    setsockopt(nSocket,IPPROTO_TCP,TCP_NODELAY,(char*)&flags,sizeof(socklen_t));
  }
  return nSocket;
}

/*==========================================================================
   FUNCTION     : release

   DESCRIPTION: Releases all the resources attached to UIBC

   PARAMETERS :  none

   Return Value  : none
  ===========================================================================*/

void UIBCPacketTransmitter::release()
{
  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "UIBCPacketTransmitter:release");

  if( m_pUIBCThread != NULL )
  {
    MM_Signal_Set(m_pExitSignal);

    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "UIBCPacketTransmitter:Waiting for UIBC thread exit");
    int exitCode;
    MM_Thread_Join(m_pUIBCThread, &exitCode);

    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "UIBCPacketTransmitter:Releasing UIBC thread");
    MM_Thread_Release(m_pUIBCThread);
  }

  if(m_pUIBCPacketizer != NULL)
  {
    MM_Delete(m_pUIBCPacketizer);
  }

  if(m_pUIBCBuffer != NULL)
  {
    MM_Free(m_pUIBCBuffer);
  }

  //closing socket
  if( m_nSocket >= 0)
  {
    close(m_nSocket);
  }

  flushEventsList();
  qmm_ListDeInit(&m_hEventsList);

  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "UIBCPacketTransmitter:releasing signals");
  if( m_pSendEventSignal != NULL )
  {
    MM_Signal_Release(m_pSendEventSignal);
  }

  if( m_pConnectSignal != NULL )
  {
    MM_Signal_Release(m_pConnectSignal);
  }

  if( m_pExitSignal != NULL )
  {
    MM_Signal_Release(m_pExitSignal);
  }

  if( m_pStopSendSingal!= NULL )
  {
    MM_Signal_Release(m_pStopSendSingal);
  }

  //releasing signal queue
  if( m_pSignalQ != NULL )
  {
      MM_SignalQ_Release(m_pSignalQ);
  }

  if( m_pConnectTimer != NULL )
  {
    MM_Timer_Release(m_pConnectTimer);
    m_pConnectTimer = NULL;
  }

  m_eUIBCState = UIBC_NO_INIT;

  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "UIBCPacketTransmitter:release: done");

}



