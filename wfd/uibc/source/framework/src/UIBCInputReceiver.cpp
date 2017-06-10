/*==============================================================================
 *  @file UIBCInputReceiver.cpp
 *
 *  @par  DESCRIPTION:
 *        Definition of the UIBC Input Receiver(Wifi Display Source)
 *        Contains implementation to receive UIBC inputs over TCP link and
 *        it interfaces with modules that parser/inject those inputs
 *
 *
 *  Copyright (c) 2011 - 2013 Qualcomm Technologies, Inc. All Rights Reserved.
 *  Qualcomm Technologies Proprietary and Confidential.
==============================================================================*/

/*==============================================================================

                      EDIT HISTORY FOR FILE

$Header: //source/qcom/qct/multimedia2/Video/wfd/uibc/main/latest/source/src/UIBCInputReceiver.cpp#2 $
$DateTime: 2012/02/10 05:45:30 $
$Changes:$

===============================================================================*/

#include "UIBCInputReceiver.h"
#include "UIBCInputInjector.h"
#include "UIBCInputParser.h"
#include "UIBCHIDInjector.h"

#include "MMThread.h"
#include "MMSignal.h"
#include "MMTimer.h"
#include "MMDebugMsg.h"

#ifndef WIN32
#include <arpa/inet.h>
#include <sys/socket.h>
#include <linux/tcp.h>
#include <sys/ioctl.h>
#include <errno.h>
#else
#include <Winsock.h>
#define SD_RECEIVE      0x00
#define SD_SEND         0x01
#define SD_BOTH         0x02
#endif

typedef union address
{
    struct sockaddr sa;
    struct sockaddr_in sa_in;
    struct sockaddr_in6 sa_in6;
    struct sockaddr_storage sa_stor;
}
address_t;

/*Static member definitions*/
//user data for m_pFetchDataSignal
const uint32 UIBCInputReceiver::FETCH_DATA_SIGNAL  = 0;
//user data for m_pExitSignal
const uint32 UIBCInputReceiver::THREAD_EXIT_SIGNAL = 1;
//user data for m_pStopSignal
const uint32 UIBCInputReceiver::FETCH_DATA_STOP_SIGNAL = 2;
const uint32 UIBCInputReceiver::ACCEPT_CONNECTION_SIGNAL = 3;

/** @brief     UIBCInputReceiver construtor
  *
  * @param[in] None
  *
  * @return    None
  */
UIBCInputReceiver::UIBCInputReceiver():
m_bListenForConn(false),
m_eUIBCState(UIBC_NO_INIT),
m_eUIBCInputInjectMode(USER),
m_pUIBCThread(NULL),
m_pSignalQ(NULL),
m_pFetchDataSignal(NULL),
m_pStopSignal(NULL),
m_pExitSignal(NULL),
m_pAcceptSignal(NULL),
m_pAttachVMCB(NULL),
m_pSendEventCB(NULL),
m_pClientData(NULL),
m_nSocket(-1),
m_nTCPDataSocket(-1),
m_UIBCPacket(NULL),
m_packetReadIndex(-1),
m_packetWriteIndex(-1),
m_pInjector(NULL),
m_pParser(NULL),
m_pHIDInjector(NULL)
{
  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "UIBCInputReceiver:constructor");
  m_UIBCPacket = new uint8[UIBC_PACKET_MAXIMUM_SIZE];
  memset (m_UIBCPacket,0,UIBC_PACKET_MAXIMUM_SIZE);
  memset (&m_pNegotiatedCapability, 0, sizeof(WFD_uibc_capability_t));
  Init();
  m_pInjector = MM_New_Args(UIBCInputInjector,());
  if (m_pInjector == NULL)
  {
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
      "UIBCInputReceiver:Constructor, can't create Injector");
    return;
  }
  m_pParser   = MM_New_Args(UIBCInputParser,());
  if (m_pParser == NULL)
  {
    if (m_pInjector != NULL)
    {
      MM_Delete(m_pInjector);
    }
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
      "UIBCInputReceiver:Constructor, can't create Injector");
    return;
  }
}

/** @brief     UIBCInputReceiver destrutor
  *
  * @param[in] None
  *
  * @return    None
  */
UIBCInputReceiver::~UIBCInputReceiver()
{
  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "UIBCInputReceiver:destructor");

  CloseDataSource();

  if(m_UIBCPacket)
  {
    delete [] m_UIBCPacket;
    m_UIBCPacket = NULL;
  }
  if (m_pInjector)
  {
    MM_Delete(m_pInjector);
    m_pInjector = NULL;
  }
  if (m_pParser)
  {
    MM_Delete(m_pParser);
    m_pParser = NULL;
  }
#ifdef HID_USE_KERNEL
  if (m_pHIDInjector)
  {
    MM_Delete(m_pHIDInjector);
    m_pHIDInjector = NULL;
  }
#endif

}

/** @brief     UIBCInputReceiver RegisterCallback
  *            This function is used only for select platforms where UIBC
  *            Event routing is done through application
  *
  * @param[in] wfd_uibc_attach_cb : Callback function to attach the thread
  *                                 in which this module runs to the platform
  *                                 environment.
  *       [in] wfd_uibc_send_event_cb : Callback function to Send UIBC events
  *                                     back to the application
  *       [in] wfd_uibc_hid_event_cb : Callback function to Send HID events
  *                                     back to the application
  *       [in] void*              : ClientData passed from the Client, may be
  *                                 useful while executing the callback.
  *
  *
  * @return    None
  */
void UIBCInputReceiver::RegisterCallback(wfd_uibc_attach_cb     Attach,
                                         wfd_uibc_send_event_cb SendEvent,
                                         wfd_uibc_hid_event_cb sendHID,
                                         void*                  ClientData)
{
  m_pAttachVMCB = Attach;
  m_pSendEventCB = SendEvent;
  m_pClientData = ClientData;
  m_pSendHIDEventCB = sendHID;
  MM_MSG_PRIO3(MM_GENERAL, MM_PRIO_DEBUG, "Registercallback %p %p %p",Attach,SendEvent,sendHID);
}

/** @brief     UIBCInputReceiver UibcThreadEntry
  *            UIBC Input Receiver worker thread entry function
  *
  * @param [in] void* : Receiver Handle
  *
  * @return    Success
  */

int UIBCInputReceiver::UibcThreadEntry( void* ptr )
{
  UIBCInputReceiver* Receiver = (UIBCInputReceiver*)ptr;
  Receiver->UibcThreadWorker();
  return 0;
}

/** @brief     UIBCInputReceiver UibcThreadWorker
  *            Worker method for UIBC Input Receiver thread.
  *            stops execution only after sending THREAD_EXIT_SIGNAL
  *
  * @param     None
  *
  * @return    None
  */

void UIBCInputReceiver::UibcThreadWorker()
{
  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
              "UIBCInputReceiver:UibcThreadWorker started");

  boolean isRunning = TRUE;

    uint32 *pEvent = NULL;
    int bTimedOut = 0;
    int32 waitTime = 0;

  while(isRunning == TRUE)
  {
    pEvent = NULL;
    bTimedOut = 0;

    /*There should not be any wait if the component is fetching data*/
    waitTime = ( m_eUIBCState == UIBC_FETCHING_DATA ) ? 1:UIBC_SOCKET_RECEIVE_TIME_OUT;
    /* Peeking signal queue to check whether any signal is set */
    if ( 0 == MM_SignalQ_TimedWait( m_pSignalQ,
                                    waitTime,
                                    (void **) &pEvent,
                                    &bTimedOut ) )
    {
      if( !bTimedOut)
      {
        switch( *pEvent )
        {
        case ACCEPT_CONNECTION_SIGNAL:
            {
              if(m_eUIBCState == UIBC_INITIALIZED)
              {
                if( m_nTCPDataSocket < 0)
                {
                  //Accept TCP connection
                  m_bListenForConn = true;
                  m_nTCPDataSocket = AcceptTCPConnection(m_nSocket);
                  if(m_nTCPDataSocket < 0)
                  {
                    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                      "UIBCInputReceiver:Error in accepting TCP connection");
                    continue;
                  }
                  SetSocketOptions(m_nTCPDataSocket);
                }
              }
             break;
           }
        case FETCH_DATA_SIGNAL:
          {
            if (m_pAttachVMCB)
            {
              if (m_pAttachVMCB(TRUE,m_pUIBCThread))
              {
                MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,"Attaching to VM succeeded");
              }
              else
              {
                MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,"Attaching to VM failed");
              }
          }

            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_DEBUG,
                        "UIBCInputReceiver:FETCH_DATA_SIGNAL received");
            m_eUIBCState = UIBC_FETCHING_DATA;
            break;
          }
        case FETCH_DATA_STOP_SIGNAL:
          {
            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                        "UIBCInputReceiver:FETCH_DATA_STOP_SIGNAL received");
            m_eUIBCState = UIBC_STOPPED;
            break;
          }
        case THREAD_EXIT_SIGNAL:
          {
            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                        "UIBCInputReceiver:THREAD_EXIT_SIGNAL received");
            isRunning = FALSE;
            if (m_pAttachVMCB)
            {
              if (!m_pAttachVMCB(FALSE,m_pUIBCThread))
              MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                          "UIBCInputReceiver:Detach thread failed");
            }
            break;
          }
        default:
          {
            MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR,
                         "UIBCInputReceiver:invalid signal %ld", *pEvent);
          }
        }
      }
      else
      {
        //No signal is received. Fetch data
        if( m_eUIBCState == UIBC_FETCHING_DATA)
        {
          fd_set fdReadSet;
          FD_ZERO(&fdReadSet);
          FD_SET(m_nTCPDataSocket, &fdReadSet);
          struct timeval timeout;
          timeout.tv_sec = 0;//inseconds
          timeout.tv_usec = 10;//in micro seconds
          int32 selectStatus = select(m_nTCPDataSocket+1,
                               &fdReadSet, NULL, NULL, &timeout);
          if(FD_ISSET(m_nTCPDataSocket, &fdReadSet))
          {
            if (selectStatus > 0)
            {
              if ( 0 != FetchData())
              {
                MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                              "UIBCInputReceiver:Error in fetchData.");
              }
            }
            else
            {
              MM_MSG_PRIO1(MM_GENERAL,MM_PRIO_HIGH,"Select unsuccessful because of %s",
                           strerror(errno));
            }
          }
        }
      }
    } //if (signal q wait)
  }

  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
              "UIBCInputReceiver:Exiting UibcThreadWorker thread");

  m_eUIBCState = UIBC_NO_INIT;
  MM_Thread_Exit( m_pUIBCThread, 0 );
}

/** @brief     UIBCInputReceiver FetchData
  *            receives UIBC packets and send them to UIBC parser
  *
  * @param     None
  *
  * @return    returns "0" incase of success
  */

int32 UIBCInputReceiver::FetchData()
{
  int32 nStatus = -1;

  int32 nBytes = ReceiveUIBCPacket(m_UIBCPacket);

  if( nBytes > 0 )
  {
    //Parse and Inject Input
    nStatus = ParseAndInjectInput();
  }
#if 0 // temporarily compiled out
#ifndef WIN32
  else if( errno == EAGAIN || errno == EWOULDBLOCK )
  {
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR,
                 "UIBCInputReceiver:No data is available for %d ms",
                 UIBC_SOCKET_RECEIVE_TIME_OUT);
  }
#endif
  else
  {
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR,
                 "UIBCInputReceiver:Error %d in receiving data",
                 errno);
    return nStatus;
  }
#endif
  return !nStatus;

}

/** @brief     UIBCInputReceiver Init
  *            Prepares WFD Source for listening to UIBC data:
  *            Creates the Thread and the necessary signals and moves the
  *            UIBC state to Initialized if everything is successful
  *
  * @param     None
  *
  * @return    None
  */

void UIBCInputReceiver::Init()
{
  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "UIBCInputReceiver:init");
  if (m_eUIBCState == UIBC_NO_INIT)
  {

  //creating signal queue
  if((MM_SignalQ_Create(&m_pSignalQ) != 0) ||
     (MM_Signal_Create( m_pSignalQ, (void *) &FETCH_DATA_SIGNAL, NULL, &m_pFetchDataSignal) != 0) ||
     (MM_Signal_Create( m_pSignalQ, (void *) &FETCH_DATA_STOP_SIGNAL, NULL, &m_pStopSignal) != 0) ||
     (MM_Signal_Create( m_pSignalQ, (void *) &ACCEPT_CONNECTION_SIGNAL, NULL, &m_pAcceptSignal) != 0) ||
     (MM_Signal_Create( m_pSignalQ, (void *) &THREAD_EXIT_SIGNAL, NULL, &m_pExitSignal) != 0)
    )
  {
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                "UIBCInputReceiver:error in creating signal queue");
    return;
  }

  int32 threadStatus = MM_Thread_CreateEx(MM_Thread_DefaultPriority,
                                          0, UibcThreadEntry, (void *) this,
                                          UIBC_THREAD_STACK_SIZE,
                                          "UIBCInputReceiver",
                                          &m_pUIBCThread);
  if(threadStatus != 0)
  {
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR,
                 "UIBCInputReceiver:Error in creating thread = %d",
                 threadStatus);
    return;
  }

    m_eUIBCState = UIBC_INITIALIZED;
  }
}

/** @brief     UIBCInputReceiver Start
  *            Starts fetching data from remote socket
  *
  * @param     None
  *
  * @return    TRUE if start is successful
  */

boolean UIBCInputReceiver::Start()
{
  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "UIBCInputReceiver:Start");
  Init();


  if(m_eUIBCState == UIBC_NO_INIT || m_eUIBCState == UIBC_FETCHING_DATA)
  {
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR,
                 "UIBCInputReceiver:Invalid component state %d for start",
                 m_eUIBCState);
    return FALSE;
  }

  //sending signal to fetch data
  MM_Signal_Set(m_pFetchDataSignal);
  return TRUE;
}

/** @brief     UIBCInputReceiver Stop
  *            Stops UIBC Receiver Thread execution
  *
  * @param     None
  *
  * @return    TRUE if start is successful
  */

boolean UIBCInputReceiver::Stop()
{
  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "UIBCInputReceiver:Stop");

  //sending stop signal to thread
  if( m_eUIBCState != UIBC_NO_INIT && m_eUIBCState != UIBC_STOPPED)
  {
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
      "UIBCInputReceiver:Sending UIBC FETCH DATA STOP signal");
    MM_Signal_Set(m_pStopSignal);
    return TRUE;
  }
  MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR,
               "UIBCInputReceiver:Invalid component state %d for Stop",
               m_eUIBCState);

  return FALSE;
}

/** @brief      UIBCInputReceiver SetUIBCCapability
  *             Stores the UIBC Capability, may be used for sanity checks
  *
  * @param [in] WFD_uibc_capability_t* : Negotiated UIBC capability handle
  *
  * @return     None
  */

void UIBCInputReceiver::SetUIBCCapability(WFD_uibc_capability_t* pNegCap)
{
  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_LOW, "UIBCInputReceiver:SetUIBCCapability");

  memcpy(&m_pNegotiatedCapability,pNegCap,sizeof (WFD_uibc_capability_t));
  m_nSocket = CreateSocket(pNegCap->port_id);
  if(m_nSocket < 0)
  {
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                "UIBCInputReceiver:SetUIBCCapability Error creating socket");
    return;
  }
  if(pNegCap->config.category & HIDC)//UIBC session supports HID
  {
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
           "UIBCInputReceiver:UIBC session supports HID");
#ifdef HID_USE_KERNEL
    if(!m_pHIDInjector)
    {
      m_pHIDInjector= MM_New(UIBCHIDInjector);
      if (m_pHIDInjector == NULL)
      {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
          "UIBCInputReceiver:SetUIBCCapability, can't create HIDInjector");
        return;
      }
    }
    else
    {
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
           "UIBCInputReceiver:SetUIBCCapability HIDInjector already created");
    }
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
           "UIBCInputReceiver:HID events will be injected through kernel");
#endif
  }
  int32 nRetStatus = -1;
  nRetStatus = SetSocketOptions(m_nSocket);
  if (nRetStatus != 0)
  {
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                "SetUIBCCapability Error in setting socket options");
  }
  MM_Signal_Set(m_pAcceptSignal);

}

/** @brief      UIBCInputReceiver GetUIBCCapability
  *             Retrieves the Negotiated UIBC Capability
  *
  * @param [in/out] WFD_uibc_capability_t* : Negotiated UIBC capability struct
  *
  * @return     None
  */

void UIBCInputReceiver::GetUIBCCapability(WFD_uibc_capability_config_t* pNegCap)
{
  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_LOW, "UIBCInputReceiver:GetUIBCCapability");

  if(m_eUIBCState != UIBC_INITIALIZED)
  {
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR,
      "UIBCInputReceiver:Invalid component state %d for start",
      m_eUIBCState);
  }
  if (pNegCap != NULL)
  {
    memcpy(pNegCap,&m_pNegotiatedCapability.config,sizeof(WFD_uibc_capability_config_t));
  }
}

/** @brief      UIBCInputReceiver CreateSocket
  *             This function creates the socket and binds it to the IP
  *
  * @param [in] port: port number of socket
  *
  * @return    Returns the FD of socket created
  */

int32 UIBCInputReceiver::CreateSocket(int16 port)
{
  int32 nSocket;
  MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR,
               "UIBCInputReceiver:Creating socket connection on TCP port %d",
               port);
  nSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  if(nSocket > 0)
  {
    struct sockaddr_in addr;
    memset(addr.sin_zero, 0, sizeof(addr.sin_zero));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    socklen_t reuseoption = 1;
    if(0 != setsockopt(static_cast<int>(nSocket),SOL_SOCKET,SO_REUSEADDR,&reuseoption,sizeof(reuseoption)))
    {
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR, "setsockopt failed because of %s",
                  strerror(errno));
      errno = 0;
#ifdef WIN32
      shutdown(nSocket,SD_BOTH);
#else
      close(nSocket);
#endif
      nSocket = -1;
    }

    address_t tAddr;
    tAddr.sa_in = addr;
    if( 0 != bind(nSocket, (const struct sockaddr *)&tAddr.sa, static_cast<socklen_t>(sizeof(addr))))
    {
      MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_ERROR,
                 "UIBCInputReceiver:Error in binding socket to port %d due to error %s",
                 port,strerror(errno));
#ifdef WIN32
      shutdown(nSocket,SD_BOTH);
#else
      close(static_cast<int>(nSocket));
#endif
      nSocket = -1;
    }
  }
  return nSocket;
}

/** @brief      UIBCInputReceiver SetSocketOptions
  *             configures the socket options required for this session
  *
  * @param [in] socket: socket descriptor
  *
  * @return    "0" on success
  */

int32 UIBCInputReceiver::SetSocketOptions(int32 socket)
{
  int32 nStatus = -1;

  MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR,
               "UIBCInputReceiver:Setting socket receive time out %d",
               UIBC_SOCKET_RECEIVE_TIME_OUT);

  struct timeval tv;
  tv.tv_sec = (UIBC_SOCKET_RECEIVE_TIME_OUT / 1000);        //time in seconds
  tv.tv_usec = (UIBC_SOCKET_RECEIVE_TIME_OUT % 1000) * 1000;
               //remaining time in micro seconds

  if( 0 != setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO,
                      reinterpret_cast<const char *>(&tv),
                      static_cast<socklen_t>(sizeof(struct timeval))) )
  {
#if 0 //temporarily compiled out
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR,
                 "UIBCInputReceiver:Error %d in setting socket receive time out ",
                 errno);
#endif
    return nStatus;
  }

  //getting the receive buffer
  int32 recvBufferSize = 0;
#ifndef WIN32
  socklen_t optionLength = static_cast<socklen_t>(sizeof(recvBufferSize));
  getsockopt(socket, SOL_SOCKET, SO_RCVBUF, &recvBufferSize, &optionLength);
#else
  uint32 optionLength = sizeof(recvBufferSize);
  getsockopt(socket, SOL_SOCKET, SO_RCVBUF,
             (char *)&recvBufferSize,
             (int*) &optionLength);
#endif
  //Half the socket buffer size is used for kernel bookkeeping
  if( recvBufferSize < (UIBC_PACKET_MAXIMUM_SIZE * 2) )
  {
    recvBufferSize = UIBC_PACKET_MAXIMUM_SIZE;

    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR,
                 "UIBCInputReceiver:Setting socket receive buffer size %d",
                 recvBufferSize);

#ifndef WIN32
    //kernel allocates double the value passed to setsockopt
    if( 0 != setsockopt(socket, SOL_SOCKET, SO_RCVBUF,
             &recvBufferSize,
             static_cast<socklen_t>(sizeof(recvBufferSize))) )
#else
    if( 0 != setsockopt(socket, SOL_SOCKET, SO_RCVBUF,
                        (const char*)&recvBufferSize,
                        sizeof(recvBufferSize)) )
#endif
    {
#if 0
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR,
                   "UIBCInputReceiver:Error %d in setting socket in receive buffer",
                   errno);
#endif
      return nStatus;
    }
  }
    int flags = 1;
    setsockopt(socket,IPPROTO_TCP,TCP_NODELAY,(char*)&flags,sizeof(socklen_t));

  nStatus = 0;
  return nStatus;
}

/** @brief      UIBCInputReceiver ReceiveUIBCPacket
  *             receives UIBC packet(from socket)
  *
  * @param [in] Buffer: Holds the UIBC Packet
  *
  * @return     Returns number of bytes if reading is successful
  */

int32 UIBCInputReceiver::ReceiveUIBCPacket(uint8* Buffer, uint32 nFetchSize)
{
  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
              "UIBCInputReceiver:Polling over socket to recv UIBC packet");
  int32 nBytes = 0;
  int32 socket = m_nTCPDataSocket;
  int32 flags  = 0;
  int32 nBuffOffset = 0;
  //ReadIndex:offset at which next read need to be done
  //WriteIndex:offset till which write has been done
  MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_ERROR,
               "UIBCInputReceiver:ReceiveUIBCPacket:start readOffset %d,write Offset %d",
                m_packetReadIndex, m_packetWriteIndex);
  if(m_packetWriteIndex > m_packetReadIndex)
  {
    nBuffOffset = m_packetWriteIndex - m_packetReadIndex + 1;
    //Some data is still pending
    memmove (Buffer, Buffer+m_packetReadIndex,nBuffOffset);
    m_packetWriteIndex = nBuffOffset - 1;
    m_packetReadIndex = -1;
    nFetchSize -=nBuffOffset;
  }
  else
  {
    m_packetWriteIndex = -1;
    m_packetReadIndex = -1;
  }
  MM_MSG_PRIO3(MM_GENERAL, MM_PRIO_ERROR,
               "UIBCInputReceiver:before recvFrom readOffset %d,write Offset %d, nFetchSize %u",
                m_packetReadIndex, m_packetWriteIndex, nFetchSize);

#ifndef WIN32
  nBytes = recvfrom(socket, Buffer + m_packetWriteIndex + 1, (int)nFetchSize,
    (int)flags, NULL, NULL);
#else
  nBytes = recvfrom(socket,(char*) (Buffer + m_packetWriteIndex + 1), nFetchSize,
    flags, NULL, NULL);
  MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_DEBUG,
               "UIBCInputReceiver:recvfrom last err %d",
               WSAGetLastError());
#endif
  MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR,
               "UIBCInputReceiver:recvfrom packet length recvd %d",
               nBytes);
  if (nBytes > 0)
  {
    m_packetWriteIndex+=nBytes;
    m_packetReadIndex = 0;
  }

  MM_MSG_PRIO3(MM_GENERAL, MM_PRIO_ERROR,
               "UIBCInputReceiver:After recvFrom readOffset %d,write Offset %d,nBytes %d",
                m_packetReadIndex, m_packetWriteIndex, nBytes);

  return nBytes;
}

/** @brief      UIBCInputReceiver AcceptTCPConnection
  *             This function listens over socket and accepts TCP connection
  *
  * @param [in] TCP Socket: Socket to listen to
  *
  * @return     Returns the accepted socket FD
  */

int32  UIBCInputReceiver::AcceptTCPConnection(int32 tcpSocket)
{
  int32 acceptedSocket = -1;
  if( 0 == listen(tcpSocket, 1) )
  {
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                "UIBCInputReceiver:Ready to accept connection on TCP port");

    //Waiting maximum 15 seconds to accept connection.
    const int32 MAX_WAIT_TIME = 100 * UIBC_SOCKET_RECEIVE_TIME_OUT;

    //Calculating maximum number of iteration as each accept call is blocked for
    //UIBC_SOCKET_RECEIVE_TIME_OUT ms
    int32 maxCount = (MAX_WAIT_TIME);
    struct sockaddr_in addr;
    memset(addr.sin_zero, 0, sizeof(addr.sin_zero));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(m_pNegotiatedCapability.port_id);
    socklen_t addrsize = static_cast<socklen_t>(sizeof(addr));

    unsigned long ioctl_arg = 1;
    ioctl(tcpSocket,FIONBIO, (char *)&ioctl_arg);
   // fcntl(tcpSocket, F_SETFL, O_NONBLOCK);
   address_t tAddr;
    for( int32 count = 0; m_bListenForConn && (count < maxCount); count++ )
    {
      tAddr.sa_in = addr;
      acceptedSocket = accept(static_cast<int>(tcpSocket),(sockaddr*) &(tAddr.sa), (socklen_t*)&addrsize);
      if( acceptedSocket >= 0)
      {
        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR,
                     "UIBCInputReceiver:Accepted connection with socket %d",
                     acceptedSocket);
        m_bListenForConn = false;
        break;
      }
#if 0
#ifndef WIN32
      else if( errno != EAGAIN && errno != EWOULDBLOCK )
      {
        break;
      }
#endif
#endif
      MM_Timer_Sleep(1000);
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                  "UIBCInputReceiver:Polling over accept");
    }
    if(acceptedSocket < 0)
    {
#if 0
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR,
                   "UIBCInputReceiver:Error in TCP accept with errno %d",
                   errno);
#endif
      return acceptedSocket;
    }
  }
  return acceptedSocket;
}

/** @brief      UIBCInputReceiver CloseDataSource
  *             Performs clean up activities, Releasing thread and signals
  *
  * @param      None
  *
  * @return     None
  */

void UIBCInputReceiver::CloseDataSource()
{
  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
              "UIBCInputReceiver:Closing UIBCInputReceiver");
  m_bListenForConn = false;
  if( m_pUIBCThread != NULL )
  {

    //sending exit signal to thread
    if( m_eUIBCState != UIBC_NO_INIT)
    {
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                  "UIBCInputReceiver:Sending UIBC Thread exit signal");
      MM_Signal_Set(m_pExitSignal);
    }
    int exitCode;
    int ret;
    ret = MM_Thread_Join(m_pUIBCThread, &exitCode);
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,
                "UIBC thread exit returned %d",ret);
    ret = MM_Thread_Release(m_pUIBCThread);
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,
                "UIBC thread release returned %d",ret);
  }

  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "UIBCInputReceiver:releasing signals");
  if( m_pFetchDataSignal != NULL )
  {
    MM_Signal_Release(m_pFetchDataSignal);
  }
  if( m_pStopSignal != NULL )
  {
    MM_Signal_Release(m_pStopSignal);
  }
  if( m_pExitSignal != NULL )
  {
    MM_Signal_Release(m_pExitSignal);
  }
  if( m_pAcceptSignal != NULL )
  {
    MM_Signal_Release(m_pAcceptSignal);
  }

  //releasing signal queue
  if( m_pSignalQ != NULL )
  {
    MM_SignalQ_Release(m_pSignalQ);
  }

  if( m_nTCPDataSocket >= 0 )
  {
#ifndef WIN32
    close(m_nTCPDataSocket);
#else
    shutdown(m_nTCPDataSocket,SD_BOTH);
#endif
  }

  if( m_nSocket >= 0 )
  {
#ifndef WIN32
    close(m_nSocket);
#else
    shutdown(m_nSocket,SD_BOTH);
#endif
  }
  m_eUIBCState = UIBC_NO_INIT;

  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
              "UIBCInputReceiver:Closing UIBCInputReceiver completed");
}

/** @brief      UIBCInputReceiver ParseAndInjectInput
  *             Invokes Parser and Injector to inject remote event
  *
  * @param      None
  *
  * @return     TRUE if Input Injection and parsing was successful
  */
boolean UIBCInputReceiver::ParseAndInjectInput()
{
  while (m_packetWriteIndex > (m_packetReadIndex+ UIBC_PACKET_HEADER_LEN ))
  {
    {
       MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_ERROR, "UIBC Packet Dump Header %x %x",m_UIBCPacket[m_packetReadIndex],m_UIBCPacket[m_packetReadIndex+1]);
    }
    int32 cachedReadIndex = m_packetReadIndex;
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR,"UIBCInputReceiver:ParseNinjectInput before parsing readIndex %d",m_packetReadIndex);
  //Sanity Check
    if (((m_UIBCPacket[m_packetReadIndex]&0xE0)>>5) != UIBC_WFD_VERSION)
  {
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                "UIBCInputReceiver:ParseNinjectInput unknown version");
    return false;
  }

    boolean bIsTSPresent = (m_UIBCPacket[m_packetReadIndex] &0x10 ? TRUE:FALSE);
    m_packetReadIndex++;
    uint8  type = (m_UIBCPacket[m_packetReadIndex]&0x0F);
    m_packetReadIndex++;
    uint16 size = static_cast<uint16>(m_UIBCPacket[m_packetReadIndex]<<8|m_UIBCPacket[m_packetReadIndex+1]);

    //readIndex already moved by two
    int32 availableSize = m_packetWriteIndex - (m_packetReadIndex - 2) + 1;
    if (size > availableSize)
    {
      //Less data is available, more needs to be read
      //Update the packet read index back to original before this function
      MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_ERROR,"UIBCInputReceiver:ParseNinjectInput less data %d is available for total size %d",availableSize, size);
      m_packetReadIndex -=2;
      return FALSE;
    }
    m_packetReadIndex+=2;
  if (bIsTSPresent == TRUE)
  {
    //TODO timestamp parsing logic
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                  "UIBCInputReceiver:ParseNinjectInput timestamp received");
      m_packetReadIndex += 2;
  }

    {
       MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "******UIBC Packet Dump START********");
       for(int index=0;index<size;index++)
       {
         MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_ERROR, "UIBC Packet %d byte - %x", index, m_UIBCPacket[cachedReadIndex+index]);
       }
       MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "******UIBC Packet Dump END********");
    }
    HIDDataType usageType = HID_INVALID_DATA;
    uint8 hidType = -1;
    ssize_t len = -1;
  switch (type)
  {
  case UIBC_GENERIC_INPUT_CATEGORY_ID :
    WFD_uibc_event_t inputEvent;
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                "UIBCInputReceiver:Found Generic Input");
         m_pParser->ParseGenericInput((uint32&)m_packetReadIndex,m_UIBCPacket,&inputEvent, &m_pNegotiatedCapability);
    if (this->m_eUIBCInputInjectMode == KERNEL)
    {
      m_pInjector->InjectInput(&inputEvent);
    }
    else
    {
      m_pSendEventCB(&inputEvent,m_pClientData);
    }
         //updating readIndex, including padding bytes
         m_packetReadIndex = cachedReadIndex + size;
    break;

  case UIBC_HIDC_INPUT_CATEGORY_ID :
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                  "UIBCInputReceiver:Found HID Input");
        len = m_pParser->ParseHIDInput((uint32&)m_packetReadIndex,m_UIBCPacket,usageType,hidType);
#ifndef HID_USE_KERNEL
            m_pSendHIDEventCB(m_UIBCPacket + m_packetReadIndex,len,usageType);
            m_packetReadIndex = cachedReadIndex + size;
            break;
#else
        if(len >= 0 && m_packetReadIndex >= 0 && ((m_packetReadIndex + len)< UIBC_PACKET_MAXIMUM_SIZE) && m_pHIDInjector){
           m_pHIDInjector->set_HID_type(hidType);
           if(usageType == HID_REPORT) {
              UIBC_HID_status hidStatus = m_pHIDInjector->getStatus();
              if(hidStatus != HID_DEV_CREATED) {
                //The peer has sent reports without sending report descriptors
                //So setup the device first and then send report
                if (m_pHIDInjector->setup_uhid_device(NULL,0)) {
                   break;
                }
              }
              m_pHIDInjector->send_report(m_UIBCPacket+m_packetReadIndex,len);
              m_pSendHIDEventCB(m_UIBCPacket + m_packetReadIndex,len,HID_REPORT);
              } else if (usageType == HID_REPORT_DESCRIPTOR) {
              //setup the device
              m_pHIDInjector->setup_uhid_device(m_UIBCPacket+m_packetReadIndex,len);
              m_pSendHIDEventCB(m_UIBCPacket + m_packetReadIndex,len,HID_REPORT_DESCRIPTOR);
              } else {
                  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                    "UIBCInputReceiver:Unknown HID usage");
              }
              //updating readIndex, including padding bytes
              m_packetReadIndex = cachedReadIndex + size;
        } else {
            MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_ERROR,
               "!!!UIBCInputReceiver:Out of bound access! due to index %d or rogue length %d", m_packetReadIndex, len);
            return FALSE;
        }
#endif
    break;

  default:
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                "UIBCInputReceiver:Unknown Input category");
         //updating readIndex, including padding bytes
         m_packetReadIndex = cachedReadIndex + size;
         return FALSE;
  }
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR,"UIBCInputReceiver:ParseNinjectInput after parsing readIndex %d",m_packetReadIndex);
  } // end while
  return TRUE;
}
