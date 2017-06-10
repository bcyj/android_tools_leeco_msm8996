/*==============================================================================
*        @file RTPDataSource.cpp
*
*  @par DESCRIPTION:
*       This is the implementation of the RTPDataSource class.
*
*
*  Copyright (c) 2012 - 2014 by Qualcomm Technologies, Inc. All Rights Reserved.
*  Qualcomm Technologies Proprietary and Confidential.
==============================================================================*/

/*==============================================================================

                      EDIT HISTORY FOR FILE

  $Header:

================================================================================
*/
#include "MMThread.h"
#include "MMSignal.h"
#include "MMTimer.h"
#include "MMDebugMsg.h"

#include "RTPDataSource.h"
#include "RTPParser.h"
#include <threads.h>

#include <media/stagefright/MediaErrors.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <ctype.h>

//Debug Feature: Need to remove
//read from /data/play_local.ts if it is present
#define RTP_READ_FROM_FILE

typedef union address
{
    struct sockaddr sa;
    struct sockaddr_in sa_in;
    struct sockaddr_in6 sa_in6;
    struct sockaddr_storage sa_stor;
}
address_t;

namespace android
{
  /*Static member definitions*/
  const uint32 RTPDataSource::FETCH_DATA_SIGNAL  = 0; //user data for m_pFetchDataSignal
  const uint32 RTPDataSource::THREAD_EXIT_SIGNAL = 1; //user data for m_pExitSignal
  const uint32 RTPDataSource::THREAD_PAUSE_SIGNAL = 2; //user data for m_pPauseSignal
  const uint32 RTPDataSource::THREAD_RESUME_SIGNAL = 3; //user data for m_pResumeSignal
  RTPDataSource::RTPDataSource(int32 port, int32 payloadType, boolean isTCPConnection, int rtpSock)
    :m_eRTPState(RTP_NO_INIT),
     m_pRTPPacket(NULL),
     m_pRTPParser(NULL),
     m_pRTPThread(NULL),
     m_pSignalQ(NULL),
     m_pFetchDataSignal(NULL),
     m_pExitSignal(NULL),
     m_pPauseSignal(NULL),
     m_pResumeSignal(NULL),
     m_bTCPConnection(isTCPConnection),
     m_nRTPSocket(-1),
     m_nTCPDataSocket(-1),
     m_nCacheSize(0),
     m_pLocalFile(NULL)
  {
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "RTPDataSource:constructor");
    init(port, payloadType, rtpSock, isTCPConnection);
  }

  RTPDataSource::~RTPDataSource()
  {
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "RTPDataSource:destructor");
    closeDataSource();
  }

  void RTPDataSource::updateRTPPortVars()
  {
    m_pRTPParser->updateRTPPortVars();
  }

  /*==========================================================================
    FUNCTION     : readAt

    DESCRIPTION:  Provides the RTP packets payload

    PARAMETERS : offset:offset of the data
                           data:output put buffer to hold data
                           size:requested number of bytes

    Return Value  : number of bytes read
   ===========================================================================*/
  ssize_t RTPDataSource::readAt(int64 offset, void *data, size_t size)
  {
    ssize_t nBytes = 0;
    if(m_eRTPState == RTP_FETCHING_DATA)
    {
#ifdef RTP_READ_FROM_FILE
      if(m_pLocalFile)
      {
        //LOGE("Reading from file offset %lld", offset);
        fseek(m_pLocalFile, offset, SEEK_SET);
        return fread( data, 1, size, m_pLocalFile);
      }
      else
#endif /*RTP_READ_FROM_FILE*/
      {
        int64 numBytesAvailable = m_pRTPParser->getNumBytesAvailable();

        /*Don't call RTPParser read, if complete requested data is not available as RTPParser read is blocking call
              */
        if((int64)(offset + size) <= numBytesAvailable || numBytesAvailable >= TS_PKT_LEN)
        {
          nBytes = m_pRTPParser->read(offset, data, size);
        }
        else
        {
          MM_MSG_PRIO3(MM_GENERAL, MM_PRIO_ERROR, "RTPDataSource:readAt data is not available,"
                                                   "requested offset %lld, size %d, Downloaded bytes %lld",
                                                   offset, size, numBytesAvailable);
        }
      }
    }
    else
    {
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR, "RTPDataSource:readAt invalid state %d", m_eRTPState);
    }
    return nBytes;
  }

  /*==========================================================================
    FUNCTION     : rtpThreadEntry

    DESCRIPTION:  RTP Datasource worker thread entry function

   ===========================================================================*/
  int RTPDataSource::rtpThreadEntry( void* ptr )
  {
    RTPDataSource* rtpDataSource = (RTPDataSource*)ptr;
    rtpDataSource->rtpThreadWorker();
    return 0;
  }

  /*==========================================================================
    FUNCTION     : rtpThreadWorker

    DESCRIPTION:  Worker method for RTP DataSource thread.
                            stops execution only after sending THREAD_EXIT_SIGNAL
   ===========================================================================*/
  void RTPDataSource::rtpThreadWorker()
  {
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "RTPDataSource:RTP worker thread started");

    int tid = androidGetTid();
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,"WFDD: RTPThread priority b4 %d ", androidGetThreadPriority(tid));
    androidSetThreadPriority(0,-19);
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,"WFDD:RTPThread priority after%d ", androidGetThreadPriority(tid));

    boolean isRunning = TRUE;
    while(isRunning)
    {
      uint32 *pEvent = NULL;
      int bTimedOut = 0;

      /*There should not be any wait if the component is fetching data*/
      int32 waitTime = ( m_eRTPState == RTP_FETCHING_DATA ) ? 0 : RTP_SOCKET_RECEIVE_TIME_OUT;

      /* Peeking singal queue to check whether any signal is set */
      if ( 0 == MM_SignalQ_TimedWait( m_pSignalQ, waitTime, (void **) &pEvent, &bTimedOut ) )
      {
        if( !bTimedOut)
        {
          switch( *pEvent )
          {
            case FETCH_DATA_SIGNAL:
            {
              MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "RTPDataSource:FETCH_DATA_SIGNAL received");
              m_eRTPState = RTP_FETCHING_DATA;
              break;
            }
            case THREAD_EXIT_SIGNAL:
            {
              MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "RTPDataSource:THREAD_EXIT_SIGNAL received");
              isRunning = FALSE;
              break;
            }
          case THREAD_PAUSE_SIGNAL:
            {
              MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "RTPDataSource:THREAD_PAUSE_SIGNAL received");
              break;

            }
          case THREAD_RESUME_SIGNAL:
            {
              MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "RTPDataSource:THREAD_RESUME_SIGNAL received");
              break;

            }

            default:
            {
              MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR, "RTPDataSource:invalid signal %u", *pEvent);
            }
          }
         }
         else
         {
           //No signal is received. Fetch data
           if( m_eRTPState == RTP_FETCHING_DATA)
           {
             if ( 0 != fetchData())
             {
               MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "RTPDataSource:Error in fetchData. Stopping RTP thread");
               isRunning = false;
             }
           }
         }
      }
    }

    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "RTPDataSource:Exiting RTP thread");
    m_eRTPState = RTP_STOPPED;
    MM_Thread_Exit( m_pRTPThread, 0 );
  }

  /*==========================================================================
     FUNCTION     : fetchData

     DESCRIPTION: receives RTP packets and send them to RTP parser

     Return Value: returns "0" incase of success
     ===========================================================================*/
  int32 RTPDataSource::fetchData()
  {
    int32 nStatus = -1;
    if( m_bTCPConnection && m_nTCPDataSocket < 0)
    {
      //Accpet TCP connection
      m_nTCPDataSocket = acceptTCPConnection(m_nRTPSocket);
      if(m_nTCPDataSocket < 0)
      {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "RTPDataSource:Error in accepting TCP connection");
        return nStatus;
      }
      setSocketOptions(m_nTCPDataSocket);
    }

    int32 nBytes = receiveRTPPacket(m_pRTPPacket);

    if( nBytes > 0 )
    {
      m_pRTPParser->processRTPPacket(m_pRTPPacket, nBytes);
    }
    else if( errno == EAGAIN || errno == EWOULDBLOCK )
    {
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR, "RTPDataSource:No data is available for %d ms", RTP_SOCKET_RECEIVE_TIME_OUT);
    }
    else
    {
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR, "RTPDataSource:Error %d in receiving data", errno);
      return nStatus;
    }

    nStatus = 0;
    return nStatus;
  }

  /*==========================================================================
    FUNCTION     : init

    DESCRIPTION: Prepares RTP data source for reading

   ===========================================================================*/
  void RTPDataSource::init(int32 port, int32 payloadType, int rtpSock, bool tcp)
  {
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "RTPDataSource:init");

    m_nRTPSocket = rtpSock > 0? rtpSock : createSocket(port);
    if(m_nRTPSocket < 0)
    {
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "RTPDataSource:Error in creating socket");
      return;
    }

    setSocketOptions(m_nRTPSocket);

    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "RTPDataSource:creating RTPParser for payload type %d", payloadType);
    m_pRTPParser = RTPParser::create(payloadType, tcp);

    if(m_pRTPParser == NULL)
    {
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR, "RTPDataSource:error in creating RTP parser for payload type %d",payloadType);
      return;
    }

    //creating signal queue
    if( (MM_SignalQ_Create(&m_pSignalQ) != 0) ||
          (MM_Signal_Create( m_pSignalQ, (void *) &FETCH_DATA_SIGNAL, NULL, &m_pFetchDataSignal) != 0) ||
        (MM_Signal_Create( m_pSignalQ, (void *) &THREAD_EXIT_SIGNAL, NULL, &m_pExitSignal) != 0) ||
        (MM_Signal_Create( m_pSignalQ, (void *) &THREAD_PAUSE_SIGNAL, NULL, &m_pPauseSignal) != 0) ||
        (MM_Signal_Create( m_pSignalQ, (void *) &THREAD_RESUME_SIGNAL, NULL, &m_pResumeSignal) != 0)
        )
    {
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "RTPDataSource:error in creating signal queue");
      return;
    }

#ifdef RTP_USE_RECVMSG
    /*only header will be stored in this buffer,payload will be stored directly to buffer given by RTPParser*/
    m_pRTPPacket = (uint8 *) MM_Malloc(RTP_PACKET_HEADER_LENGTH);
#else
    m_pRTPPacket = (uint8 *) MM_Malloc(65536);
#endif

    if(m_pRTPPacket == NULL)
    {
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "RTPDataSource:Error in allocating RTP Packet buffer");
      return;
    }

    int32 threadStatus = MM_Thread_CreateEx(MM_Thread_DefaultPriority, 0, rtpThreadEntry, (void *) this,
                                                              RTP_THREAD_STACK_SIZE, "RTPDataSource", &m_pRTPThread);
    if(threadStatus != 0)
    {
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "RTPDataSource:Error in creating thread");
      return;
    }

#ifdef RTP_READ_FROM_FILE
    m_pLocalFile = fopen("/data/play_local.ts", "rb");
#endif

    m_eRTPState = RTP_INITIALIZED;
  }

  /*==========================================================================
    FUNCTION     : start

    DESCRIPTION: Allocates the payload buffer and start looper to poll over on RTP socket

    RETURN VALUE: returns OK for success
   ===========================================================================*/
  status_t RTPDataSource::start()
  {
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "RTPDataSource:start");
    status_t nStatus = -1;

    if(m_eRTPState != RTP_INITIALIZED)
    {
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR, "RTPDataSource:Invalid component state %d for start", m_eRTPState);
      return nStatus;
    }

    int32 nBufSize = (m_nCacheSize > 0) ? m_nCacheSize : RTP_PAYLOAD_DEFAULT_BUFFER_SIZE;
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR, "RTPDataSource:payload buffer length %d",nBufSize);
    m_nCacheSize = nBufSize;

    if( m_pRTPParser->allocatePayloadBuffer(nBufSize) != 0 )
    {
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "RTPDataSource:Error at allocating payload buffer");
      return nStatus;
    }

    //sending signal to fetch data
    MM_Signal_Set(m_pFetchDataSignal);

    nStatus = OK;
    return nStatus;
  }

  /*==========================================================================
    FUNCTION     : pause

    DESCRIPTION:

    RETURN VALUE: returns OK for success
   ===========================================================================*/
  status_t RTPDataSource::pause()
  {
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "RTPDataSource:pause");
    status_t nStatus = -1;

    if(m_eRTPState != RTP_FETCHING_DATA)
    {
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR, "RTPDataSource:Invalid component state %d for pause", m_eRTPState);
      return nStatus;
    }

    //sending signal to fetch data
    MM_Signal_Set(m_pPauseSignal);

    nStatus = OK;
    return nStatus;
  }

   /*==========================================================================
    FUNCTION     : resume

    DESCRIPTION:

    RETURN VALUE: returns OK for success
   ===========================================================================*/
  status_t RTPDataSource::resume()
  {
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "RTPDataSource:resume");
    status_t nStatus = -1;

    if(m_eRTPState != RTP_INITIALIZED)
    {
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR, "RTPDataSource:Invalid component state %d for start", m_eRTPState);
      return nStatus;
    }

    //sending signal to fetch data
    MM_Signal_Set(m_pResumeSignal);

    nStatus = OK;
    return nStatus;
  }


  /*==========================================================================
    FUNCTION     : stop

    DESCRIPTION: Stops RTP Thread execution

    RETURN VALUE: returns OK for success
   ===========================================================================*/
  status_t RTPDataSource::stop()
  {
    status_t nStatus = -1;
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "RTPDataSource:stop");

    //sending exit signal to thread
    if( m_eRTPState != RTP_NO_INIT && m_eRTPState != RTP_STOPPED)
    {
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "RTPDataSource:Sending RTP Thread exit signal");
      MM_Signal_Set(m_pExitSignal);
    }

    nStatus = OK;
    return nStatus;
  }

  /*==========================================================================
     FUNCTION     : CreateSocket

     DESCRIPTION: This function creates the socket and binds it to the IP

     PARAMETERS :  port: port number of socket

     Return Value  : Returns the FD of socket created
    ===========================================================================*/
  int32 RTPDataSource::createSocket(int32 port)
  {
    int32 nSocket;
    if(m_bTCPConnection)
    {
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "RTPDataSource:Creating TCP port on %d", port);
      nSocket = socket(AF_INET, SOCK_STREAM, 0);
    }
    else
    {
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "RTPDataSource:Creating UDP port on %d", port);
      nSocket = socket(AF_INET, SOCK_DGRAM, 0);
    }

    if(nSocket > 0)
    {
      struct sockaddr_in addr;
      memset(addr.sin_zero, 0, sizeof(addr.sin_zero));
      addr.sin_family = AF_INET;
      addr.sin_addr.s_addr = INADDR_ANY;
      addr.sin_port = htons(port);

      address_t tAddr;
      tAddr.sa_in = addr;
      if( 0 != bind((int)nSocket, (const struct sockaddr *)&tAddr.sa, (int)sizeof(addr)))
      {
        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR, "RTPDataSource:Error in binding socket to port %d", port);
        close((int)nSocket);
        nSocket = -1;
      }
    }
    return nSocket;
  }

  /*==========================================================================
     FUNCTION     : setSocketOptions

     DESCRIPTION: configures the socket options required for this session

     PARAMETERS : socket:FD of socket

     Return Value  : Returns the FD of socket created
    ===========================================================================*/
  int32 RTPDataSource::setSocketOptions(int32 socket)
  {
    int32 nStatus = -1;

    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "RTPDataSource:Setting socket receive time out %d", RTP_SOCKET_RECEIVE_TIME_OUT);

    struct timeval tv;
    tv.tv_sec = (RTP_SOCKET_RECEIVE_TIME_OUT / 1000);        //time in seconds
    tv.tv_usec = (RTP_SOCKET_RECEIVE_TIME_OUT % 1000) * 1000;//remaining time in micro seconds

    if( 0 != setsockopt((int)socket, SOL_SOCKET, SO_RCVTIMEO,(struct timeval *)&tv,(socklen_t)sizeof(struct timeval)) )
    {
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR, "RTPDataSource:Error %d in setting socket receive time out ", errno);
      return nStatus;
    }

    //getting the receive buffer
    int32 recvBufferSize = 0;
    socklen_t optionLength = (socklen_t)sizeof(recvBufferSize);
    getsockopt((int)socket, SOL_SOCKET, SO_RCVBUF, &recvBufferSize, &optionLength);

    //Half the socket buffer size is used for kernel bookkeeping
    if( recvBufferSize < (RTP_SOCKET_RECEIVE_BUFFER_SIZE * 2) )
    {
      recvBufferSize = RTP_SOCKET_RECEIVE_BUFFER_SIZE;

      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "RTPDataSource:Setting socket receive buffer size %d", recvBufferSize);

      //kernel allocates double the value passed to setsockopt
      if( 0 != setsockopt((int)socket, SOL_SOCKET, SO_RCVBUF, &recvBufferSize, (socklen_t)sizeof(recvBufferSize)) )
      {
        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR, "RTPDataSource:Error %d in setting socket in receive buffer", errno);
        return nStatus;
      }
    }

    nStatus = 0;
    return nStatus;
  }

  /*==========================================================================
    FUNCTION     : receiveRTPPacket

    DESCRIPTION: receives RTP packet

    PARAMETERS :  buffer: holds RTP packet

    Return Value  : Returns "OK" if reading is success ful
   ===========================================================================*/

  int32 RTPDataSource::receiveRTPPacket(uint8* rtpBuffer)
  {
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_LOW, "RTPDataSource:Polling over socket to recv RTP packet");
    ssize_t nBytes = 0;

    int32 socket = m_nRTPSocket;
    int32 flags = 0;

    if( m_bTCPConnection )
    {
      socket = m_nTCPDataSocket;
      /*
            *Flag to wait for complete data to arrive
            *Assumed that with TCP, source will sent 7 TS packets + 12 bytes header(1328 bytes) at a time
            */
      flags |=  MSG_WAITALL;
    }

#ifndef RTP_USE_RECVMSG

    if(m_bTCPConnection)
    {
      //Framing is involved. 2 byte size and payload
      nBytes = recvfrom((int)socket, rtpBuffer, 2/*framingheader */,
                             (int)flags, NULL, NULL);

      uint16 nDataSize = (uint16)((uint16)(((uint16)rtpBuffer[0]) << 8) | ((uint16)rtpBuffer[1]));

      if(nDataSize <= RTP_PACKET_MAXIMUM_SIZE)
      {
         nBytes = recvfrom((int)socket, rtpBuffer, nDataSize,
                             (int)flags, NULL, NULL);
      }
    }
    else
    {
      nBytes = recvfrom((int)socket, rtpBuffer, RTP_PACKET_MAXIMUM_SIZE,
                             (int)flags, NULL, NULL);
    }

#else  /*RTP_USE_RECVMSG*/

    /*
         *With recvBuffer only header will be stored in rtpBuffer and payload will be stored at buffers provided by RTPParser
         */
    uint8* recvVector_1;
    uint8* recvVector_2;

    int32 recvVector_length_1;
    int32 recvVector_length_2;

    struct msghdr  msg;

    struct iovec   iov[3];
    size_t numRecvVectors = 1;

    memset(&msg, 0, sizeof(msg));
    memset(iov, 0, sizeof(iov));

    m_pRTPParser->getRecvDataPositions(&recvVector_1, &recvVector_length_1, &recvVector_2, &recvVector_length_2);

    iov[0].iov_base = rtpBuffer;
    iov[0].iov_len  = RTP_PACKET_HEADER_LENGTH;

    iov[1].iov_base = recvVector_1;
    iov[1].iov_len  = recvVector_length_1;

    if(recvVector_length_2 > 0)
    {
      numRecvVectors = 2;
      iov[2].iov_base = recvVector_2;
      iov[2].iov_len  = recvVector_length_2;
    }

    msg.msg_iov = iov;
    msg.msg_iovlen  = numRecvVectors + 1;

    nBytes = recvmsg(socket, &msg, flags);

    MM_MSG_PRIO4(MM_GENERAL, MM_PRIO_LOW,"RTPDataSource:recvmsg:Total length recvd %d,header length %d, IO vectors length %d,%d", nBytes,
                                                                          iov[0].iov_len, iov[1].iov_len, iov[2].iov_len);

#endif /*RTP_USE_RECVMSG*/

    return nBytes;
  }


  /*==========================================================================
     FUNCTION     : acceptTCPConnection

     DESCRIPTION: This function listens over socket and accpets the TCP connection.

     PARAMETERS :  tcpSocket: socket FD

     Return Value  : Returns the accpected socket
    ===========================================================================*/
  int32  RTPDataSource::acceptTCPConnection(int32 tcpSocket)
  {
    int32 acceptedSocket = -1;
    if( 0 == listen((int)tcpSocket, 1) )
    {
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "RTPDataSource:Ready to accept connection on TCP port");

      //Waiting maximum 15 seconds to accept connection.
      const int32 MAX_WAIT_TIME = 15 * 1000;

      //Calculating maximum number of iteration as each accept call is blocked for "RTP_SOCKET_RECEIVE_TIME_OUT" ms
      int32 maxCount = (MAX_WAIT_TIME) / RTP_SOCKET_RECEIVE_TIME_OUT;

      for( int32 count = 0; count < maxCount; count++ )
      {
        acceptedSocket = accept((int)tcpSocket, NULL, NULL);
        if( acceptedSocket >= 0)
        {
          MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "RTPDataSource:Accepted connection with port %d", acceptedSocket);
          break;
        }
        else if( errno != EAGAIN && errno != EWOULDBLOCK )
        {
          break;
        }
        //MM_Timer_Sleep(1000);
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_LOW, "RTPDataSource:Polling over accept");
      }

      if(acceptedSocket < 0)
      {
        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR, "RTPDataSource:Error in TCP accept with errno %d", errno);
        return acceptedSocket;
      }
    }
    return acceptedSocket;
  }

  int64_t RTPDataSource::getRTPRefereceTime()
  {
    return m_pRTPParser->getRTPReferenceTimeStamp();
  }

  /*==========================================================================
     FUNCTION     : getRTPBaseTime

     DESCRIPTION: This function return baseTime for first packet

     PARAMETERS :

     Return Value  : Returns base time for 1st packet
    ===========================================================================*/
  status_t RTPDataSource::getRTPBaseTime(int64 *baseTimeUs)
  {
    status_t status = OK;
    *baseTimeUs = m_pRTPParser->getRTPBaseTimeUs();
    return status;
  }
  bool RTPDataSource::IsBaseTimeAdjusted(bool *bDiscontinuity, uint64 *pnTSBefore, uint64* pnTSAfter)
  {
    return m_pRTPParser? m_pRTPParser->IsBaseTimeAdjusted(bDiscontinuity, pnTSBefore, pnTSAfter): false;
  }

  status_t RTPDataSource::initCheck() const
  {
    status_t initStatus = OK;
    if(m_eRTPState == RTP_NO_INIT)
    {
       initStatus = NO_INIT;
    }
    return initStatus;
  }

    /*==========================================================================
      FUNCTION     : getHeadOffset

      DESCRIPTION:  It provides valid lower bound offset of buffer

      Parameter: size<out>:holds valid lower bound offset of buffer

      RETURN VALUE: returns OK for success
     ===========================================================================*/
    status_t RTPDataSource::getHeadOffset(int64 *size)
    {
       int32 nStatus = -1;

       if( m_eRTPState != RTP_NO_INIT )
       {
#ifdef RTP_READ_FROM_FILE
         if(m_pLocalFile)
         {
           *size = -1;
         }
         else
#endif /*RTP_READ_FROM_FILE*/
         {
           *size = m_pRTPParser->getHeadOffset();
         }
         nStatus = OK;
       }

       if(m_eRTPState == RTP_STOPPED)
       {
         MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "RTPDataSource:getHeadOffset returning ERROR_END_OF_STREAM") ;

         nStatus = ERROR_END_OF_STREAM;
       }
       return nStatus;
    }

  /*==========================================================================
    FUNCTION     : getSize

    DESCRIPTION:  Captures the maximum offset available at RTP datasource

    Parameter: size<out>:holds the maximum offset available at RTP datasource

    RETURN VALUE: returns OK for success
   ===========================================================================*/
  status_t RTPDataSource::getSize(int64 *size)
  {
     status_t nStatus = -1;

     if( m_eRTPState != RTP_NO_INIT )
     {
#ifdef RTP_READ_FROM_FILE
       if(m_pLocalFile)
       {
         *size = -1;
       }
       else
#endif /*RTP_READ_FROM_FILE*/
       {
         *size = m_pRTPParser->getNumBytesAvailable();
       }
       nStatus = OK;
     }

     if(m_eRTPState == RTP_STOPPED)
     {
       MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "RTPDataSource:getSize returning ERROR_END_OF_STREAM") ;

       nStatus = ERROR_END_OF_STREAM;
     }
     return nStatus;
  }

  /*==========================================================================
    FUNCTION     : setCacheSize

    DESCRIPTION: sets the size of cache to store payload packets

    PARAMETERS: cacheSize: size of cache in bytes

    RETURN VALUE: returns OK if it can set cache size
   ===========================================================================*/
  status_t RTPDataSource::setCacheSize(size_t cacheSize)
  {
    status_t nStatus = -1;
    if(m_eRTPState == RTP_INITIALIZED)
    {
      m_nCacheSize = (int32)cacheSize;
      nStatus = OK;
    }
    return nStatus;
  }

  /*==========================================================================
    FUNCTION     : closeDataSource

    DESCRIPTION: Performs clean up activities

   ===========================================================================*/
  void RTPDataSource::closeDataSource()
  {
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "RTPDataSource:Closing RTPDataSource");

    if( m_pRTPThread != NULL )
    {
      //sending exit signal to thread
      if( m_eRTPState != RTP_NO_INIT && m_eRTPState != RTP_STOPPED)
      {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "RTPDataSource:Sending RTP Thread exit signal");
        MM_Signal_Set(m_pExitSignal);
      }

      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "RTPDataSource:Waiting for RTP thread exit");
      int exitCode;
      MM_Thread_Join(m_pRTPThread, &exitCode);

      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "RTPDataSource:releasing RTP thread");
      MM_Thread_Release(m_pRTPThread);
    }

    if( m_pRTPParser != NULL )
    {
      m_pRTPParser->flushRTPPacketsQueue();
      m_pRTPParser->printStatistics();
      MM_Delete(m_pRTPParser);
    }

    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "RTPDataSource:releasing signals");
    if( m_pFetchDataSignal != NULL )
    {
      MM_Signal_Release(m_pFetchDataSignal);
    }
    if( m_pExitSignal != NULL )
    {
      MM_Signal_Release(m_pExitSignal);
    }
    if( m_pPauseSignal != NULL )
    {
      MM_Signal_Release(m_pPauseSignal);
    }
    if( m_pResumeSignal != NULL )
    {
      MM_Signal_Release(m_pResumeSignal);
    }



    //releasing signal queue
    if( m_pSignalQ != NULL )
    {
      MM_SignalQ_Release(m_pSignalQ);
    }

    if( m_pRTPPacket != NULL )
    {
      MM_Free(m_pRTPPacket);
    }

    if( m_nTCPDataSocket >= 0 )
    {
      close((int)m_nTCPDataSocket);
    }

    if( m_nRTPSocket >= 0 )
    {
      close((int)m_nRTPSocket);
    }

#ifdef RTP_READ_FROM_FILE
    if(m_pLocalFile != NULL)
    {
      fclose(m_pLocalFile);
    }
#endif
    m_eRTPState = RTP_NO_INIT;

    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "RTPDataSource:Closing RTPDataSource completed");
  }
}//android namespace
