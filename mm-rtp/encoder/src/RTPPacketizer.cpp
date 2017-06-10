/*==============================================================================
*        @file RTPPacketizer.cpp
*
*  @par DESCRIPTION:
*       This is the implementation of the RTPPacketizer class.
*
*
*  Copyright (c) 2011 - 2013 Qualcomm Technologies, Inc. All Rights Reserved.
*  Qualcomm Technologies Proprietary and Confidential.
==============================================================================*/

/*==============================================================================

                      EDIT HISTORY FOR FILE

  $Header:

================================================================================
*/
#include "MMThread.h"
#include "MMTimer.h"
#include "MMMemory.h"
#include "MMSignal.h"
#include "MMTime.h"
#include "MMDebugMsg.h"
#include "MMCriticalSection.h"
#include "RTPPacketizer.h"
#include "RTPPacketTransmit.h"
#include <stdlib.h>
#include <unistd.h>
#include <threads.h>
#ifdef ENABLE_RTP_STATS
#include <cutils/properties.h>
#endif
boolean bFileDumpEnable;
#define RTP_MSG_PRIO(a,b,c) //MM_MSG_PRIO(a,b,c)
#define RTP_MSG_PRIO1(a,b,c,d) //MM_MSG_PRIO1(a,b,c,d)
#define RTP_MSG_PRIO2(a,b,c,d,e) //MM_MSG_PRIO2(a,b,c,d,e)
#define RTP_MEMCPY_OPTIMIZE_
static unsigned int nPacketCount;
unsigned int ngSleepTime;
#define RTP_HDR_SIZE 12
#define RTP_TCP_SIZE_FIELD_LEN 2
#define TS_PKT_SIZE 188
#define MAX_TS_PKTS 7
#define RTP_BUFFERING_DURATION 250
#define MIN(x,y)  ((x) < (y) ? (x) : (y))
#define MAX(x,y)  ((x) < (y) ? (y) : (x))

#define MM_RTP_THREAD_PRIORITY -2

// WFD:STATISTICS -- start
static FILE *fp = NULL;
extern bool bRTPDumpEnable;
// WFD:STATISTICS -- end

    const uint32 CRTPPacketizer::m_RTPEncodeEvent = 0;   //! Event-process incomingdata
    const uint32 CRTPPacketizer::m_RTPCloseEvent = 1;    //! Event-close the thred
    const uint32 CRTPPacketizer::m_RTPBufAvailEvent = 0; //! Event-buffer is available
    const uint32 CRTPPacketizer::m_RTPStackSize = 16384; //! Stack size for rtp Thread

/*==============================================================================

         FUNCTION:         CRTPPacketizer

         DESCRIPTION:
*//**       @brief         CRTPPacketizer constructor
*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         Port Number
                           Destination IP address as uint32
                           Stream bitrate

*//*     RETURN VALUE:
*//**       @return
                           None


@par     SIDE EFFECTS:
                           None

*//*==========================================================================*/

CRTPPacketizer::CRTPPacketizer(uint32 nPortNum, uint32 nDestIPAddr,
                               uint32 nStreamBitrate, uint8 bRtpPortTypeUdp)
{
    _success          = true;
    m_pRTPSignalQ     = NULL;
    m_pRTPThread      = NULL;
    m_RTPEncodeSignal = NULL;
    m_RTPCloseSignal  = NULL;
    m_RTPPacketBuf    = NULL;
    m_bFrameStart = true;
    m_nCurrTimestamp = 0;
    m_nBaseTimestamp = 0;
    m_bTCP = false;
    MM_Time_GetTime(&m_nBaseTimestamp);
    /**-------------------------------------------------------------------------
       Create the signal queue for RTP packetization thread.
    ----------------------------------------------------------------------------
    */
    if ( 0 != MM_SignalQ_Create( &m_pRTPSignalQ ) )
    {
        RTP_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
           "CRTPPacketizer::CRTPPacketizer Signal Q create failed!!!");
        m_pRTPSignalQ = NULL;
        _success = false;
    }
    if(_success)
    {
        RTP_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
           "CRTPPacketizer::CRTPPacketizer  SignalQ create success!!!");
    }
    /**-------------------------------------------------------------------------
       Create the signal for encoding the payload.
    ----------------------------------------------------------------------------
    */
    if(_success && ( 0 != MM_Signal_Create( m_pRTPSignalQ,
                                   (void *) &m_RTPEncodeEvent,
                                   NULL,
                                   &m_RTPEncodeSignal ) ) )
    {
        RTP_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
           "CRTPPacketizer::CRTPPacketizer Signal  create failed!!!");
        _success = false;
        m_RTPEncodeSignal = NULL;
    }
    if(_success)
    {
        RTP_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
           "CRTPPacketizer::CRTPPacketizer encode Signal create success!!!");
    }
    /**-------------------------------------------------------------------------
       Create the signal for ending the encoding.
    ----------------------------------------------------------------------------
    */
    if(_success && ( 0 != MM_Signal_Create( m_pRTPSignalQ,
                                   (void *) &m_RTPCloseEvent,
                                   NULL,
                                   &m_RTPCloseSignal ) ) )
    {
        RTP_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
           "CRTPPacketizer::CRTPPacketizer Signal create failed!!!");
        _success = false;
        m_RTPEncodeSignal = NULL;
    }
    if(_success)
    {
        RTP_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
           "CRTPPacketizer::CRTPPacketizer close Signal create success!!!");
    }

    /**-------------------------------------------------------------------------
       Create the thread for RTP packetization.
    ----------------------------------------------------------------------------
    */
    if ( _success &&( 0 != MM_Thread_CreateEx( 20,
                                   0,
                                   CRTPPacketizer::RTPThreadEntry,
                                   this,
                                   CRTPPacketizer::m_RTPStackSize,
                                   "RTPPacketize", &m_pRTPThread) ) )
    {
        RTP_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR,
           "CRTPPacketizer::CRTPPacketizer Thread create failed %d!!!",m_pRTPThread);
        _success = false;
        m_pRTPThread = NULL;
    }
    if(_success)
    {
        RTP_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
           "CRTPPacketizer::CRTPPacketizer Thread create success!!!");
    }
    /**-------------------------------------------------------------------------
       Initialize the RTP packetization parameters
    ----------------------------------------------------------------------------
    */
    InitRTPPacketParameters();
    m_bEOF = false;
    m_sStreamHandler.bFlushing = false;
    m_sStreamHandler.nFlushThreshold =0;
    m_sStreamHandler.nSize = ((nStreamBitrate / 8) * RTP_BUFFERING_DURATION)
        /1000;

    m_sStreamHandler.nSize +=(TS_PKT_SIZE - (
                            m_sStreamHandler.nSize % TS_PKT_SIZE));

    if(!OpenStreamHandler())
    {
        RTP_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
           "CRTPPacketizer::CRTPPacketizer Stream Handle create failed!!!");
        _success = false;
    }

    /**-------------------------------------------------------------------------
         Create an instance of the transmitter class.
    ----------------------------------------------------------------------------
    */
    pTransmitter = NULL;
    if(_success)
    {
        pTransmitter = new CRTPPacketTransmit(nPortNum, 0, nDestIPAddr, bRtpPortTypeUdp);
    }

    if(!pTransmitter)
    {
        RTP_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
         "CRTPPacketizer::CRTPPacketizer Transmitter Object create failed!!!");
        _success = false;
    }

    m_bTCP = bRtpPortTypeUdp ? false : true;


    /**-------------------------------------------------------------------------
         Allocate a buffer to pack RTP data
    ----------------------------------------------------------------------------
    */
        uint32 nSize = (uint32)(TS_PKT_SIZE * MAX_TS_PKTS + sizeof(uint32) * 4);
        RTP_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR,
         "CRTPPacketizer::CRTPPacketizer RTB Buf size = %d!!!",nSize );
#ifndef RTP_MEMCPY_OPTIMIZE_
    if(_success)
    {
        m_RTPPacketBuf =
            (uint8*)MM_Malloc(nSize);
    }

    if(!m_RTPPacketBuf)
    {
        RTP_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
           "CRTPPacketizer::CRTPPacketizer RTP Packet Buff alloc failed!!!");
        _success = false;
    }
#endif

    /**-------------------------------------------------------------------------
         Creata a critical section which can come quite handy
    ----------------------------------------------------------------------------
    */
    if(MM_CriticalSection_Create(&m_RTPPacketizerCS))
    {
        RTP_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
           "CRTPPacketizer::CRTPPacketizer Critical section create failed!!!");
        m_RTPPacketizerCS = NULL;
        _success = false;
    }

    if(bRTPDumpEnable)
    {
      fp = NULL;
      fp = fopen("/data/media/dump.ts","wb");
      if(!fp)
      {
         RTP_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "fopen failed");
      }
    }
    #ifdef ENABLE_RTP_STATS
    // WFD:STATISTICS -- start
    m_pStatTimer = NULL;
    memset(&rtpStats,0,sizeof(rtp_stats_struct));
    rtpStats.bRTPStat = true;
    m_nDuration= 5000;
    if(0 != MM_Timer_Create((int)m_nDuration, 1, readStatTimerHandler, (void *)(this), &m_pStatTimer))
    {
        MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"Creation of timer failed");
    }
    // WFD:STATISTICS -- end
    #endif /*ENABLE_RTP_STATS*/
}

/*==============================================================================

         FUNCTION:         CRTPPacketizer

         DESCRIPTION:
*//**       @brief         CRTPPacketizer constructor
*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         Port Number
                           Destination IP address as uint32
                           Stream bitrate

*//*     RETURN VALUE:
*//**       @return
                           None


@par     SIDE EFFECTS:
                           None

*//*==========================================================================*/

CRTPPacketizer::CRTPPacketizer(networkInfoType *pNetworkInfo,
                    uint32 nStreamBitrate)
{
    _success          = true;
    m_pRTPSignalQ     = NULL;
    m_pRTPThread      = NULL;
    m_RTPEncodeSignal = NULL;
    m_RTPCloseSignal  = NULL;
    m_RTPPacketBuf    = NULL;
    m_bFrameStart = true;
    m_nCurrTimestamp = 0;
    m_nBaseTimestamp = 0;
    m_bTCP = false;
    m_bEOF = false;
    m_sStreamHandler.bFlushing = false;
    m_sStreamHandler.nFlushThreshold =0;
    MM_Time_GetTime(&m_nBaseTimestamp);
    /**-------------------------------------------------------------------------
       Create the signal queue for RTP packetization thread.
    ----------------------------------------------------------------------------
    */
    if ( 0 != MM_SignalQ_Create( &m_pRTPSignalQ ) )
    {
        RTP_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
           "CRTPPacketizer::CRTPPacketizer Signal Q create failed!!!");
        m_pRTPSignalQ = NULL;
        _success = false;
    }
    if(_success)
    {
        RTP_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
           "CRTPPacketizer::CRTPPacketizer  SignalQ create success!!!");
    }
    /**-------------------------------------------------------------------------
       Create the signal for encoding the payload.
    ----------------------------------------------------------------------------
    */
    if(_success && ( 0 != MM_Signal_Create( m_pRTPSignalQ,
                                   (void *) &m_RTPEncodeEvent,
                                   NULL,
                                   &m_RTPEncodeSignal ) ) )
    {
        RTP_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
           "CRTPPacketizer::CRTPPacketizer Signal  create failed!!!");
        _success = false;
        m_RTPEncodeSignal = NULL;
    }
    if(_success)
    {
        RTP_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
           "CRTPPacketizer::CRTPPacketizer encode Signal create success!!!");
    }
    /**-------------------------------------------------------------------------
       Create the signal for ending the encoding.
    ----------------------------------------------------------------------------
    */
    if(_success && ( 0 != MM_Signal_Create( m_pRTPSignalQ,
                                   (void *) &m_RTPCloseEvent,
                                   NULL,
                                   &m_RTPCloseSignal ) ) )
    {
        RTP_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
           "CRTPPacketizer::CRTPPacketizer Signal create failed!!!");
        _success = false;
        m_RTPEncodeSignal = NULL;
    }
    if(_success)
    {
        RTP_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
           "CRTPPacketizer::CRTPPacketizer close Signal create success!!!");
    }

    /**-------------------------------------------------------------------------
       Create the thread for RTP packetization.
    ----------------------------------------------------------------------------
    */
    if ( _success &&( 0 != MM_Thread_CreateEx( 20,
                                   0,
                                   CRTPPacketizer::RTPThreadEntry,
                                   this,
                                   CRTPPacketizer::m_RTPStackSize,
                                   "RTPPacketize", &m_pRTPThread) ) )
    {
        RTP_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR,
           "CRTPPacketizer::CRTPPacketizer Thread create failed %d!!!",m_pRTPThread);
        _success = false;
        m_pRTPThread = NULL;
    }
    if(_success)
    {
        RTP_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
           "CRTPPacketizer::CRTPPacketizer Thread create success!!!");
    }
    /**-------------------------------------------------------------------------
       Initialize the RTP packetization parameters
    ----------------------------------------------------------------------------
    */
    InitRTPPacketParameters();

    m_sStreamHandler.nSize = ((nStreamBitrate / 8) * RTP_BUFFERING_DURATION)
        /1000;

    m_sStreamHandler.nSize +=(TS_PKT_SIZE - (
                            m_sStreamHandler.nSize % TS_PKT_SIZE));

    if(!OpenStreamHandler())
    {
        RTP_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
           "CRTPPacketizer::CRTPPacketizer Stream Handle create failed!!!");
        _success = false;
    }

    /**-------------------------------------------------------------------------
         Create an instance of the transmitter class.
    ----------------------------------------------------------------------------
    */
    if(_success)
    {
        pTransmitter = new CRTPPacketTransmit(
            pNetworkInfo->PortNum0,  pNetworkInfo->nSocket,
            pNetworkInfo->nIPAddress, pNetworkInfo->bRtpPortTypeUdp);
    }

    if(!pTransmitter)
    {
        RTP_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
         "CRTPPacketizer::CRTPPacketizer Transmitter Object create failed!!!");
        _success = false;
    }


    m_bTCP = pNetworkInfo->bRtpPortTypeUdp ? false : true;


    /**-------------------------------------------------------------------------
         Allocate a buffer to pack RTP data
    ----------------------------------------------------------------------------
    */
        uint32 nSize = (uint32)(TS_PKT_SIZE * MAX_TS_PKTS + sizeof(uint32) * 4);
        RTP_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR,
         "CRTPPacketizer::CRTPPacketizer RTB Buf size = %d!!!",nSize );
#ifndef RTP_MEMCPY_OPTIMIZE_
    if(_success)
    {
        m_RTPPacketBuf =
            (uint8*)MM_Malloc(nSize);
    }

    if(!m_RTPPacketBuf)
    {
        RTP_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
           "CRTPPacketizer::CRTPPacketizer RTP Packet Buff alloc failed!!!");
        _success = false;
    }
#endif
    /**-------------------------------------------------------------------------
         Creata a critical section which can come quite handy
    ----------------------------------------------------------------------------
    */
    if(MM_CriticalSection_Create(&m_RTPPacketizerCS))
    {
        RTP_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
           "CRTPPacketizer::CRTPPacketizer Critical section create failed!!!");
        m_RTPPacketizerCS = NULL;
        _success = false;
    }

    if(bRTPDumpEnable)
    {
      fp = NULL;
      fp = fopen("/data/media/dump.ts","wb");
      if(!fp)
      {
         RTP_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "fopen failed");
      }
    }
    #ifdef ENABLE_RTP_STATS
    // WFD:STATISTICS -- start
    m_pStatTimer = NULL;
    memset(&rtpStats,0,sizeof(rtp_stats_struct));
    rtpStats.bRTPStat = true;
    m_nDuration= 5000;
    if(0 != MM_Timer_Create((int) m_nDuration, 1, readStatTimerHandler, (void *)(this), &m_pStatTimer))
    {
        MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"Creation of timer failed");
    }
    // WFD:STATISTICS -- end
    #endif /*ENABLE_RTP_STATS*/
}



/*==============================================================================

         FUNCTION:         ~CRTPPacketizer

         DESCRIPTION:
*//**       @brief         CRTPPacketizer destructor
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
CRTPPacketizer::~CRTPPacketizer()
{
    // WFD:STATISTICS -- start
    #ifdef ENABLE_RTP_STATS
    if(rtpStats.bRTPStat)
    {
      RTP_MSG_PRIO1(MM_STATISTICS, MM_PRIO_MEDIUM,
        "WFD:STATISTICS: Maximum time taken in sending RTP packet is = %lu ms",
         rtpStats.nMaxTime);
    }
    if(m_pStatTimer != NULL)
    {
       MM_Timer_Release(m_pStatTimer);
    }
    m_nDuration = 0;
    #endif /* ENABLE_RTP_STATS */
    // WFD:STATISTICS -- end

    /**-------------------------------------------------------------------------
       If thread has been started make the thread exit
    ----------------------------------------------------------------------------
    */
    if(m_RTPCloseSignal && m_pRTPThread)
    {
        int exitCode = 0;
        MM_Signal_Set( m_RTPCloseSignal );
        MM_Thread_Join( m_pRTPThread, &exitCode );
    }

    /**-------------------------------------------------------------------------
       Release the packetization thread
    ----------------------------------------------------------------------------
    */
    if(m_pRTPThread)
    {
        MM_Thread_Release( m_pRTPThread );
        m_pRTPThread = NULL;
    }

    /**-------------------------------------------------------------------------
       Release the signals
    ----------------------------------------------------------------------------
    */
    if(m_RTPEncodeSignal)
    {
        MM_Signal_Release( m_RTPEncodeSignal );
        m_RTPEncodeSignal = NULL;
    }

    if(m_RTPCloseSignal)
    {
        MM_Signal_Release( m_RTPCloseSignal );
        m_RTPCloseSignal = NULL;
    }


    /**-------------------------------------------------------------------------
       Release the signalQ
    ----------------------------------------------------------------------------
    */
    if(m_pRTPSignalQ)
    {
        MM_SignalQ_Release( m_pRTPSignalQ );
        m_pRTPSignalQ = NULL;
    }

    /**-------------------------------------------------------------------------
       Kill Transmitter
    ----------------------------------------------------------------------------
    */
    if(pTransmitter)
    {
        /*MM_Delete*/delete(pTransmitter);
    }

    /**-------------------------------------------------------------------------
       Delete packet buff
    ----------------------------------------------------------------------------
    */
    if(m_RTPPacketBuf)
    {
        MM_Free(m_RTPPacketBuf);
    }

    CloseStreamHandler();

    /**-------------------------------------------------------------------------
       Finally the destroy the critical section
    ----------------------------------------------------------------------------
    */
    if(m_RTPPacketizerCS)
    {
        MM_CriticalSection_Release(m_RTPPacketizerCS);
    }

   if(bRTPDumpEnable && fp!= NULL)
   {
    fclose(fp);
   }

    return;
}

/*!*************************************************************************
 * @brief     Timer handler for reading statistics flag from command line
 *
 * @param[in] ptr Reference to the current instance
 *
 * @return    NONE
 *
 * @note
 **************************************************************************/
#ifdef ENABLE_RTP_STATS
void CRTPPacketizer::readStatTimerHandler(void* ptr)
{
    CRTPPacketizer* rtpPkz= (CRTPPacketizer*)ptr;
    char szTemp[PROPERTY_VALUE_MAX];
    if(property_get("persist.debug.enable_rtp_stats",szTemp,"true")<0)
    {
        MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"Failed to read persist.debug.enable_rtp_stats");
        return;
    }
    if(strcmp(szTemp,"false")==0)
    {
        memset(&(rtpPkz->rtpStats),0,sizeof(rtpStats));
        rtpPkz->rtpStats.bRTPStat = false;
    }
    else
    {
        rtpPkz->rtpStats.bRTPStat = true;
    }
}
#endif


/*==============================================================================

         FUNCTION:         InitRTPPacketParameters

         DESCRIPTION:
*//**       @brief         Initialize RTP params for M2TS. Can be extebded for
                           other profiles.
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
void CRTPPacketizer::InitRTPPacketParameters()
{
    unsigned long nTimeStamp;

    (void)MM_Time_GetTime(&nTimeStamp);

    srand((unsigned int)nTimeStamp);

    m_sRTPPacketParams.txSsrc = rand();

    m_sRTPPacketParams.sequenceNo = 0;

    m_sRTPPacketParams.payloadType = 33;

    m_nRTPRefClock = 0;
    m_bFirstTimeStampCaptured = false;

    m_nOutputUnitSize = TS_PKT_SIZE * MAX_TS_PKTS;
}


/*==============================================================================

         FUNCTION:         OpenStreamHandler

         DESCRIPTION:
*//**       @brief         Initialize the buffer for caching TS packets before
                           RTP packetization.
*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         None

*//*     RETURN VALUE:
*//**       @return
                           bool


@par     SIDE EFFECTS:
                           None

*//*==========================================================================*/
bool CRTPPacketizer::OpenStreamHandler()
{
    m_sStreamHandler.nSize = MAX(m_nOutputUnitSize, m_sStreamHandler.nSize);

    m_sStreamHandler.pBuffer = (uint8*)MM_Malloc(m_sStreamHandler.nSize + RTP_HDR_SIZE);
    RTP_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR,
       "CRTPPacketizer::CRTPPacketizer Async Buf Size = %d",
                   m_sStreamHandler.nSize );
    if(!m_sStreamHandler.pBuffer)
    {
        RTP_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
           "CRTPPacketizer::OpenStreamHandler Async Buf allocation failed!!!");
        _success = false;
        return false;
    }
#ifdef RTP_MEMCPY_OPTIMIZE_
    m_sStreamHandler.pOrigBuffer = m_sStreamHandler.pBuffer;
    m_sStreamHandler.pBuffer += RTP_HDR_SIZE;
    m_RTPPacketBuf = m_sStreamHandler.pOrigBuffer;
#endif

    memset(m_sStreamHandler.pBuffer,0,m_sStreamHandler.nSize);

    m_sStreamHandler.pHead   = 0;

    m_sStreamHandler.pTail   = 0;

    m_sStreamHandler.bOverflow  = false;

    m_sStreamHandler.nBytesOverflow = 0;
    m_pBuffManSigQ  = NULL;
    m_pBuffAvailSig = NULL;

    if ( 0 != MM_SignalQ_Create( &m_pBuffManSigQ ) )
    {
      _success = false;
      RTP_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
           "CRTPPacketizer::CRTPPacketizer Async Buf handle SignalQ Create fail");
      return false;
    }

    if ((0 != MM_Signal_Create( m_pBuffManSigQ,
                                (void *) &m_RTPBufAvailEvent,
                                 NULL,
                                 &m_pBuffAvailSig )))
    {
      _success = false;
      RTP_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
          "CRTPPacketizer::CRTPPacketizer Async Buf handle SignalQ Create fail");
      return false;
    }

    return true;
}


/*==============================================================================

         FUNCTION:         CloseStreamHandler

         DESCRIPTION:
*//**       @brief         Close the stream handler

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
void CRTPPacketizer::CloseStreamHandler()
{

    m_sStreamHandler.pBuffer = NULL;

    m_sStreamHandler.pHead   = 0;

    m_sStreamHandler.pTail   = 0;

    m_sStreamHandler.bOverflow  = false;

    m_sStreamHandler.nBytesOverflow = 0;


   if(m_pBuffAvailSig)
    {
        MM_Signal_Release(m_pBuffAvailSig);
        m_pBuffAvailSig = NULL;
    }
    if(m_pBuffManSigQ)
    {
        MM_SignalQ_Release(m_pBuffManSigQ);
        m_pBuffManSigQ = NULL;
    }

    return;
}


/*==============================================================================

         FUNCTION:         PushStream

         DESCRIPTION:
*//**       @brief         Push TS packet stream to cache.

*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         None

*//*     RETURN VALUE:
*//**       @return
                           bytes written


@par     SIDE EFFECTS:
                           None

*//*==========================================================================*/
uint32 CRTPPacketizer::PushStream(const uint8  *data, uint32 len)
{

    RTP_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM, "CRTPPacketizer::PushStream");
    uint32 chunk;
    uint8* input = (uint8*)data;
    uint32 received_len = len;


    /**-------------------------------------------------------------------------
         Push bytes while there is still data left to push.
    ----------------------------------------------------------------------------
    */

    while (len > 0)
    {
        /**---------------------------------------------------------------------
          Push either the rest of the data, or enough bytes to push the tail
          to the end of the buffer, whichever comes first.
        ------------------------------------------------------------------------
        */
    //    MM_CriticalSection_Enter(m_RTPPacketizerCS);
        chunk = MIN (len, m_sStreamHandler.nSize - m_sStreamHandler.pTail);



        /**---------------------------------------------------------------------
          Make sure not to overflow the buffer.
        ------------------------------------------------------------------------
        */
        if (m_sStreamHandler.pHead > m_sStreamHandler.pTail)
        {
            chunk = MIN (chunk, m_sStreamHandler.pHead -
                                          m_sStreamHandler.pTail - 1);
        }
        else if (m_sStreamHandler.pHead == 0)
        {
            chunk = MIN (chunk, m_sStreamHandler.nSize -
                                          m_sStreamHandler.pTail - 1);
        }

        RTP_MSG_PRIO2(MM_GENERAL, MM_PRIO_MEDIUM,
            "CRTPPacketizer::PushStream before memcpy Tail  %d   pHead  %d",
                    m_sStreamHandler.pTail, m_sStreamHandler.pHead);

        /* If chunk has bytes, move the actual data. */
        if (0 < chunk)
        {
            memcpy (m_sStreamHandler.pBuffer +
                                     m_sStreamHandler.pTail, input, chunk);
        }

        /* Update buffer state and input parameters. */
        m_sStreamHandler.pTail += chunk;
        if (m_sStreamHandler.pTail >= m_sStreamHandler.nSize)
        {
            m_sStreamHandler.pTail -= m_sStreamHandler.nSize;
        }
        input += chunk;
        len -= chunk;

        RTP_MSG_PRIO2(MM_GENERAL, MM_PRIO_MEDIUM,
           "CRTPPacketizer::PushStream after memcpy Tail ptr  %d   Head ptr  %d",
           m_sStreamHandler.pTail, m_sStreamHandler.pHead);

   //     MM_CriticalSection_Leave(m_RTPPacketizerCS);


    }
    if(/*!m_sStreamHandler.bFlushing &&*/
        GetStreamBufferOccupancy() >= (TS_PKT_SIZE * MAX_TS_PKTS))
    {
        RTP_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM,
             "CRTPPacketizer::PushStream callback to flush the data");
      //  MM_Signal_Set(m_RTPEncodeSignal);
        FlushStream();
        m_sStreamHandler.bFlushing = true;
    }
    RTP_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM,
      "CRTPPacketizer::PushStream returning from received_len - len  %d",
       received_len - len);
    return (received_len - len);
}

/*==============================================================================

         FUNCTION:         PushStreamSync

         DESCRIPTION:
*//**       @brief         Push TS packet stream to network adaptor.

*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         None

*//*     RETURN VALUE:
*//**       @return
                           bytes written


@par     SIDE EFFECTS:
                           None

*//*==========================================================================*/
uint32 CRTPPacketizer::PushStreamSync(const uint8  *data, uint32 len)
{
     if(!data || len == 0 || len > TS_PKT_SIZE * MAX_TS_PKTS)
     {
         MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR, "Invalid RTP paylaod size %d", len);
         return 0;
     }
     uint32 RTPpktsize = 0;
     uint32 nBytesWritten = 0;
     uint32 nDataOffset = RTP_HDR_SIZE;

      if(m_bTCP)
      {
          nDataOffset += RTP_TCP_SIZE_FIELD_LEN;
      }

      memcpy(m_RTPPacketBuf + nDataOffset, data, len);
      RTPpktsize = MakeRTPPacket(data, len);

      if (bRTPDumpEnable)
      {
         if (fp)
         {
            fwrite(m_RTPPacketBuf + nDataOffset, 1, len, fp);
         }
      }

      // WFD:STATISTICS -- start
      #ifdef ENABLE_RTP_STATS
      if(rtpStats.bRTPStat)
      {
         MM_Time_GetTime(&rtpStats.nStartTime);
         RTP_MSG_PRIO2(MM_STATISTICS, MM_PRIO_LOW,
                       "WFD:STATISTICS : CRTPPacketizer::PushSTreamSync len = %d StartTime = %lu",
                       len, rtpStats.nStartTime);
      }
      #endif /* ENABLE_RTP_STATS */
      // WFD:STATISTICS -- end
      if(IsOK())
      {
          nBytesWritten = pTransmitter->SendPacket(m_RTPPacketBuf, RTPpktsize);
      }

      // WFD:STATISTICS -- start
      #ifdef ENABLE_RTP_STATS
      if(rtpStats.bRTPStat)
      {
         MM_Time_GetTime(&rtpStats.nEndTime);
         RTP_MSG_PRIO2(MM_STATISTICS, MM_PRIO_LOW,
                       "WFD:STATISTICS : CRTPPacketizer::PushSTreamSync len = %d EndTime = %lu",
                       len, rtpStats.nEndTime);
         if(rtpStats.nMaxTime< (uint32)(rtpStats.nEndTime-rtpStats.nStartTime))
         {
           rtpStats.nMaxTime = rtpStats.nEndTime-rtpStats.nStartTime;
         }
         if(rtpStats.nStatCount%2000 == 0)
         {
           RTP_MSG_PRIO2(MM_STATISTICS, MM_PRIO_MEDIUM,
                         "WFD:STATISTICS:   CRTPPacketizer::FlushStream::Total time taken in\
                         transmitting packet is %lu  ms, maximum time taken is %lu ms",
            rtpStats.nEndTime-rtpStats.nStartTime, rtpStats.nMaxTime);
         }
         rtpStats.nStatCount++;
      }
      #endif /* ENABLE_RTP_STATS */
      // WFD:STATISTICS -- end

    if((int32)nBytesWritten > (int32)nDataOffset)
    {
      nBytesWritten -= nDataOffset;
    }
    return nBytesWritten;
}
/*==============================================================================

         FUNCTION:         FlushStream

         DESCRIPTION:
*//**       @brief         Flush the chache after packetization to transport
                           object

*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         None

*//*     RETURN VALUE:
*//**       @return
                           bytes written


@par     SIDE EFFECTS:
                           None

*//*==========================================================================*/
int CRTPPacketizer::FlushStream(void)
{

    uint32 len = 0;
    uint32 chunk = 0;
    uint32 write_count = 0;
    uint32 iter = 0;
    uint32 RTPpktsize = 0;

    /* Pull bytes while there is still data left to be pulled. */
    while (((len = GetStreamBufferOccupancy()) > 0))
    {

        if(len < m_nOutputUnitSize &&
           iter && !m_sStreamHandler.bOverflow)
        {
            break;
        }

        iter++;
        while (len)
        {

            if(m_sStreamHandler.pTail > m_sStreamHandler.pHead)
            {
                chunk = m_sStreamHandler.pTail - m_sStreamHandler.pHead;
            }
            else
            {
                chunk = m_sStreamHandler.nSize - m_sStreamHandler.pHead;
            }

            if(m_sStreamHandler.bOverflow)
            {
                if((m_sStreamHandler.nSize - chunk)
                                >= m_sStreamHandler.nBytesOverflow)
                {
                    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                           "CRTPPacketizer: Release caller thread from wait");
                    if(0 != MM_Signal_Set(m_pBuffAvailSig))
                    {
                        RTP_MSG_PRIO(MM_GENERAL, MM_PRIO_FATAL,
                           "CRTPPacketizer: Caller thread in indefinite wait");
                    }
                }
            }

            chunk = MIN(chunk , len);
            if (m_nOutputUnitSize > 0)
            {
                chunk = MIN (chunk, m_nOutputUnitSize);
            }

            if( chunk < m_nOutputUnitSize
                 && chunk % TS_PKT_SIZE)
            {
                chunk -= chunk % TS_PKT_SIZE;
            }

            if(!chunk)
            {
                break;
            }
            if(bRTPDumpEnable)
            {
                if (fp)
                {
                    fwrite((m_sStreamHandler.pBuffer) + m_sStreamHandler.pHead, 1, chunk, fp);
                }
            }

            m_RTPPacketBuf = ((m_sStreamHandler.pBuffer) + m_sStreamHandler.pHead) - RTP_HDR_SIZE;
            RTPpktsize =
             MakeRTPPacket((m_sStreamHandler.pBuffer) + m_sStreamHandler.pHead,
                           chunk);
           // LOGE("start Send");
           // WFD:STATISTICS -- start
           #ifdef ENABLE_RTP_STATS
           if(rtpStats.bRTPStat)
           {
              MM_Time_GetTime(&rtpStats.nStartTime);
              RTP_MSG_PRIO2(MM_STATISTICS, MM_PRIO_LOW,
                            "WFD:STATISTICS : CRTPPacketizer::FlushStream len = %d StartTime = %lu",
                            chunk, rtpStats.nStartTime);
           }
           #endif /* ENABLE_RTP_STATS */
           // WFD:STATISTICS -- end
           if(IsOK())
           {
              pTransmitter->SendPacket(m_RTPPacketBuf, RTPpktsize);
           }
          // LOGE("End Send");

           // WFD:STATISTICS -- start
           #ifdef ENABLE_RTP_STATS
           if(rtpStats.bRTPStat)
           {
              MM_Time_GetTime(&rtpStats.nEndTime);
              RTP_MSG_PRIO2(MM_STATISTICS, MM_PRIO_LOW,
                            "WFD:STATISTICS : CRTPPacketizer::FlushStream len = %d EndTime = %lu",
                            chunk, rtpStats.nEndTime);
              if(rtpStats.nMaxTime< (uint32)(rtpStats.nEndTime-rtpStats.nStartTime))
              {
                rtpStats.nMaxTime = rtpStats.nEndTime-rtpStats.nStartTime;
              }
              if(rtpStats.nStatCount%2000 == 0)
              {
                RTP_MSG_PRIO2(MM_STATISTICS, MM_PRIO_MEDIUM,
                  "WFD:STATISTICS:   CRTPPacketizer::FlushStream::Total time taken in\
                  transmitting packet is %lu  ms, maximum time taken is %lu ms",
                  rtpStats.nEndTime-rtpStats.nStartTime, rtpStats.nMaxTime);
              }
              rtpStats.nStatCount++;
           }
           #endif /* ENABLE_RTP_STATS */
           // WFD:STATISTICS -- end
          //  MM_CriticalSection_Enter(m_RTPPacketizerCS);
            /* Update buffer state and input parameters. */
            m_sStreamHandler.pHead += chunk;
            if (m_sStreamHandler.pHead >= m_sStreamHandler.nSize)
            {
                m_sStreamHandler.pHead -= m_sStreamHandler.nSize;
            }
            len -= chunk;
        }
    }
    m_sStreamHandler.pHead = 0;
    m_sStreamHandler.pTail = 0;
  //  MM_CriticalSection_Enter(m_RTPPacketizerCS);
    m_sStreamHandler.bFlushing = false;
    if(m_sStreamHandler.bOverflow)
    {
        if(m_sStreamHandler.pHead == m_sStreamHandler.pTail)
        {
            /**-----------------------------------------------------------------
            If head and tail matches here we can immediately release the
            caller thread. Otherwise caller thread would have set the signal
            again and we will wait until we finish writing those in the
            next iteration of this function.
            --------------------------------------------------------------------
            */
            RTP_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                   "CRTPPacketizer: Release caller thread from wait");
            if(0 != MM_Signal_Set(m_pBuffAvailSig))
            {
                RTP_MSG_PRIO(MM_GENERAL, MM_PRIO_FATAL,
                    "CRTPPacketizer: caller thread in indefinite wait");
            }
        }
    }

  //  MM_CriticalSection_Leave(m_RTPPacketizerCS);
    RTP_MSG_PRIO2(MM_GENERAL, MM_PRIO_ERROR,
                "CRTPPacketizer::FlushStream len = %d EndTime = %d",
                 chunk, time);
    return (int)len;
}

/*==============================================================================

         FUNCTION:         GetStreamBufferOccupancy

         DESCRIPTION:
*//**       @brief         Return buffer occupancy

*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         None

*//*     RETURN VALUE:
*//**       @return
                           occupancy in the cache


@par     SIDE EFFECTS:
                           None

*//*==========================================================================*/
uint32 CRTPPacketizer::GetStreamBufferOccupancy()
{
//MM_CriticalSection_Enter(m_RTPPacketizerCS);
    uint32 len;


    RTP_MSG_PRIO2(MM_GENERAL, MM_PRIO_MEDIUM,
        "CRTPPacketizer::GetStreamBufferOccupancy   Tail=%d  pHead=%d",
         m_sStreamHandler.pTail, m_sStreamHandler.pHead);

    len = m_sStreamHandler.pTail - m_sStreamHandler.pHead;

    if (m_sStreamHandler.pTail < m_sStreamHandler.pHead)
    {
        len += m_sStreamHandler.nSize;
    }

    RTP_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM,
        "CRTPPacketizer::GetStreamBufferOccupancy  %d len", len);

 //   MM_CriticalSection_Leave(m_RTPPacketizerCS);
    return len;
}

/*==============================================================================

         FUNCTION:         Encode

         DESCRIPTION:
*//**       @brief         Public Api that is used to send the data for encode

*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         None

*//*     RETURN VALUE:
*//**       @return
                           Bytes accpeted for encode


@par     SIDE EFFECTS:
                           None

*//*==========================================================================*/
uint32 CRTPPacketizer::Encode(const uint8* pData,
                              uint32 nLen, bool bEOF)
{
    int32 num_RTP_packets;
    uint32 remBytes = 0, nBytesWritten = 0;

    if(!pData  || !nLen)
    {
        RTP_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
               "RTPPacketizer::Encode Invalid params or no Data to encode");
        return 0;
    }

    if(pData[0] != 0x47)
    {
        RTP_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
               "RTPPacketizer::Encode Data not at TS packet boundary");
        return 0;
    }

    m_bEOF = bEOF;

    if(!m_bFirstTimeStampCaptured)
    {
        MM_Time_GetTime(&m_nBaseTimestamp);
        m_bFirstTimeStampCaptured = true;
    }


    if(m_bFrameStart == true)
    {
        MM_Time_GetTime(&m_nCurrTimestamp);
        m_nCurrTimestamp -= m_nBaseTimestamp;
        m_nCurrTimestamp *= 90;//in 90000 scale
        MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_MEDIUM,
               "RTPPacketizer::RTP TS %lu len %d", m_nCurrTimestamp, nLen);
        m_bFrameStart = false;
    }

    num_RTP_packets  = (nLen / TS_PKT_SIZE);

    remBytes = nLen - (num_RTP_packets * TS_PKT_SIZE);
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM, " Num TS in RTP %d ", num_RTP_packets);
    if(num_RTP_packets > 0)
    {
        nBytesWritten = PushStreamSync(pData, nLen - remBytes);
    }

    if(bEOF)
    {
        m_bFrameStart = true;
    }
    return nBytesWritten;
}

/*==============================================================================

         FUNCTION:         MakeRTPPacket

         DESCRIPTION:
*//**       @brief         Contruct the RTP packet

*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         None

*//*     RETURN VALUE:
*//**       @return
                           packet size


@par     SIDE EFFECTS:
                           None

*//*==========================================================================*/
uint32 CRTPPacketizer::MakeRTPPacket(const uint8* pData, uint32 nLen)
{
    uint8 marker_bit = 0;
    uint32 timestamp = 0;
    /**-------------------------------------------------------------------------
      ___RFC 3350 - RTP A transport protocol for real time applications___

     0                   1                   2                   3
     0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |V=2|P|X|  CC   |M|     PT      |       sequence number         |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |                           timestamp                           |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |           synchronization source (SSRC) identifier            |
    +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
    |            contributing source (CSRC) identifiers             |
    |                             ....                              |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

    V = 2 by this version of spec
    P = 1 if any padding bytes in the end. Not required for MP2TS
    X = Any header extension. Not going to be used for M2TS
    CC = Number of CSRC fields after SSRC. Not going to be used.

    M = The interpretation of the marker is defined by a profile.  It is
      intended to allow significant events such as frame boundaries to
      be marked in the packet stream.  A profile MAY define additional
      marker bits or specify that there is no marker bit by changing the
      number of bits in the payload type field (see Section 5.3).
      We will start to use this when required for M2tS or WFD. See below.

    PT = Payload type = 33 for MP2TS

    sequence number - has the usual meaning.

    timestamp = for transport stream it is the time at the which the first
    byte of the payload is being sent from transmitter.



    ____________________________________________________________________________
    RFC 2250 Part 2- Carrying transport stream in RTP defines the three of
    the above fields. Here they go..

       The RTP header fields are used as follows:

        Payload Type: Distinct payload types should be assigned for
          MPEG1 System Streams, MPEG2 Program Streams and MPEG2
          Transport Streams.  See [4] for payload type assignments.

        M bit:  Set to 1 whenever the timestamp is discontinuous
          (such as might happen when a sender switches from one data
          source to another). This allows the receiver and any
          intervening RTP mixers or translators that are synchronizing
          to the flow to ignore the difference between this timestamp
          and any previous timestamp in their clock phase detectors.

        timestamp: 32 bit 90K Hz timestamp representing the target
          transmission time for the first byte of the packet.

    ____________________________________________________________________________

    This makes things pretty simple for us. The following implementation will
    add sequence number, a random ssrc, payload type, M bit when necessary
    (can't think when we will need it) and finally timestamp
    ----------------------------------------------------------------------------
    */

    if(!pData  || !nLen)
    {
        RTP_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
               "RTPPacketizer::Encode Invalid params or no Data to encode");
        return 0;
    }

    /**-------------------------------------------------------------------------
       Start making the packet to ship.
    ----------------------------------------------------------------------------
    */

    int index = 0;

    if(m_bTCP)
    {
        uint16 nRTPPayloadLen = (uint16)(nLen + RTP_HDR_SIZE);
        m_RTPPacketBuf[index++] = (uint8)(nRTPPayloadLen >> 8);//MSB of len
        m_RTPPacketBuf[index++] = (uint8)(nRTPPayloadLen & 0xFF);//LSB of len
    }

    m_RTPPacketBuf[index++] = (uint8)(2 << 6);                      /* Version = 2 */

    m_RTPPacketBuf[index] = (uint8)(marker_bit << 7);             /* marker Bit  */

    m_RTPPacketBuf[index++] |= (uint8)m_sRTPPacketParams.payloadType;

    m_RTPPacketBuf[index++] = (uint8)(m_sRTPPacketParams.sequenceNo >> 8);/*MSB   */

    m_RTPPacketBuf[index++] = (uint8)(m_sRTPPacketParams.sequenceNo);     /*LSB   */

    m_sRTPPacketParams.sequenceNo = (uint16)(m_sRTPPacketParams.sequenceNo + 1);

    /**-------------------------------------------------------------------------
        Timestamp
    ----------------------------------------------------------------------------
    */

    m_RTPPacketBuf[index++]  = (uint8)( m_nCurrTimestamp >> 24); /* t i          */
    m_RTPPacketBuf[index++]  = (uint8)( m_nCurrTimestamp >> 16); /*    m e       */
    m_RTPPacketBuf[index++]  = (uint8)( m_nCurrTimestamp >> 8 ); /*        s t   */
    m_RTPPacketBuf[index++]  = (uint8)( m_nCurrTimestamp >> 0 ); /*           amp*/

    /**-------------------------------------------------------------------------
        TX SSRC - A random number.
    ----------------------------------------------------------------------------
    */
    m_RTPPacketBuf[index++]  = (uint8)( m_sRTPPacketParams.txSsrc >> 24);
    m_RTPPacketBuf[index++]  = (uint8)( m_sRTPPacketParams.txSsrc >> 16);
    m_RTPPacketBuf[index++] = (uint8)( m_sRTPPacketParams.txSsrc >> 8 );
    m_RTPPacketBuf[index++] = (uint8)( m_sRTPPacketParams.txSsrc >> 0 );

#ifndef RTP_MEMCPY_OPTIMIZE_
    memcpy(m_RTPPacketBuf + index, pData, nLen);
#endif
    return nLen + index;
}

/*==============================================================================

         FUNCTION:         RTPThreadEntry

         DESCRIPTION:
*//**       @brief         Entry function to RTP Thread

*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         void pointer to the const

*//*     RETURN VALUE:
*//**       @return
                           0


@par     SIDE EFFECTS:
                           None

*//*==========================================================================*/
int CRTPPacketizer::RTPThreadEntry( void* ptr )
{
    RTP_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "RTPPacketizer::RTPThreadEntry");
    CRTPPacketizer* pThis = (CRTPPacketizer *) ptr;

    if ( NULL != pThis )
    {
        pThis->RTPThread();
    }
    return 0;
}

/*==============================================================================

         FUNCTION:         RTPThread

         DESCRIPTION:
*//**       @brief         Actual processing function for RTP thread

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
void CRTPPacketizer::RTPThread( void )
{
    bool bRunning = true;
    RTP_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "RTPPacketizer::RTPThread");
    uint32 *pEvent = NULL;

    int tid = androidGetTid();
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,"RTPThread priority b4 %d ", androidGetThreadPriority(tid));
    androidSetThreadPriority(0,MM_RTP_THREAD_PRIORITY);
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,"RTPThread priority after%d ", androidGetThreadPriority(tid));

    while ( bRunning )
    {
        /* Wait for a signal to be set on the signal Q. */
        if ( 0 == MM_SignalQ_Wait( m_pRTPSignalQ, (void **) &pEvent ) )
        {
            switch ( *pEvent )
            {
                case m_RTPEncodeEvent:
                {
                    RTP_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                       "RTPPacketizer::RTPThread  received m_RTPEncodeEvent");
                    FlushStream();
                }
                break;
                case m_RTPCloseEvent:
                {
                    RTP_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,
                       "RTPPacketizer::RTPThread  received m_RTPCloseEvent");
                    bRunning = false;
                    MM_Thread_Exit( m_pRTPThread, 0 );
                }
                break;
                default:
                {
                    RTP_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,
                       "RTPPacketizer::RTPThread  received Unknown Event");
                    /* Not a recognized event, ignore it. */
                }
            }
        }
    }
    return;
}

/*==============================================================================

         FUNCTION:         IsOK

         DESCRIPTION:
*//**       @brief         Is everything fine?

*//**

@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         None

*//*     RETURN VALUE:
*//**       @return
                           bool - everything is fine
                                - something wrong


@par     SIDE EFFECTS:
                           None

*//*==========================================================================*/
bool CRTPPacketizer::IsOK()
{
    return _success;
}

