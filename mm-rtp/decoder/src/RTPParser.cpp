/*==============================================================================
*        @file RTPParser.cpp
*
*  @par DESCRIPTION:
*       This is the implementation of the RTPParser class.
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

#define MIN(x,y)  ((x) < (y) ? (x) : (y))
#define ABSDIFF(x,y) ((x) > (y) ? (x - y) : (y - x))

#include <arpa/inet.h>
#include "MMDebugMsg.h"
#include "RTPParser.h"
#include "MMTimer.h"
#include "MMCriticalSection.h"
#include "MMSignal.h"
#include <cutils/properties.h>
#define RTP_DUMP_TS_DATA

extern bool bRTPDumpEnable;
#define PAT_PID  0
#define WRAP_AROUND_MAGIC_NUM 95443717
#define WRAPAROUND_THRESHOLD  100000

namespace android
{
/*Static member definitions*/
   const uint32 RTPParser::DATA_AVAILABLE_SIGNAL  = 1; //user data for m_pDataAvailableSignal

   RTPParser::RTPParser(RTPPayloadType payloadType, bool tcp)
      :mPayloadType(payloadType),
       m_bIsTCP(tcp),
       m_TimeStampCS(NULL),
       m_pSignalRtpParserQ(NULL),
       m_pDataAvailableSignal(NULL)
   {
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "RTPParser:constructor");
      init();

      //Reset time control variables
      m_nPCRPID   = 0;
      m_nPMTPID   = 0;

      m_nPrevPCRMs = 0;
      m_nPCRBaseMs = 0;
      m_nLocalBaseTime = 0;
      m_nPCRConsistency = 0;
      m_bPATParsed = false;
      m_bPMTParsed = false;
      m_bPCRBaseTaken = false;
      m_bLocalBaseTimeTaken = false;
      m_bBaseTimeAdjusted = false;
      m_nPCRPID = 0;
      m_nPMTPID = 0;
      m_nLocalPrevTime = 0;
      m_nDelayedPCR = 0;
      m_nPCRJumpValue = 0;


#ifdef RTP_DUMP_TS_DATA
      m_pDumpTimer = NULL;
      m_nDuration = 10000;

      /*Timer allows periodic checking for persist.sys.enable_RTPDumps property
        to enable runtime enablement of RTPDumps. Duration is set to 10 seconds*/
      if(MM_Timer_Create(m_nDuration,1,checkForRTPDumpStatus,(void *)this,&m_pDumpTimer) != 0)
      {
        MM_MSG_PRIO(MM_GENERAL,MM_PRIO_ERROR,"Failed to create timer for runtime RTPDumps");
      }

      FILE *pDumpEnableFile = NULL;
      pDumpEnableFile = fopen("/data/media/dumpenable", "rb");

      if(pDumpEnableFile || bRTPDumpEnable)
      {
        m_pDumpFile = fopen("/data/media/rtpdump.ts", "ab");
        if(m_pDumpFile == NULL)
        {
          MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "RTPParser:Dump file fopen failed");
        }

        if(pDumpEnableFile)
        {
          fclose(pDumpEnableFile);
        }
      }
#endif
   }
   RTPParser::~RTPParser()
   {
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "RTPParser:destructor");
      if(m_sTSBufferHandler.pBuffer != NULL)
      {
         MM_Free(m_sTSBufferHandler.pBuffer);
      }

      if(mIsReorderingEnabled)
      {
        for(int i = 0; i < RTP_REORDER_PACKETS_QUEUE_LENGTH ; i++)
        {
           if(mReorderPackets[i] != NULL)
           {
              MM_Free(mReorderPackets[i]);
           }
        }
      }
    MM_CriticalSection_Release(m_TimeStampCS);

    if(NULL != m_pDataAvailableSignal )
    {
      MM_Signal_Release(m_pDataAvailableSignal);
    }

     //releasing signal queue
    if(NULL != m_pSignalRtpParserQ  )
    {
      MM_SignalQ_Release(m_pSignalRtpParserQ);
    }
#ifdef RTP_DUMP_TS_DATA
      if(m_pDumpFile != NULL)
      {
        fclose(m_pDumpFile);
      }

      if(m_pDumpTimer != NULL)
      {
        MM_Timer_Release(m_pDumpTimer);
        m_pDumpTimer = NULL;
      }
#endif
   }

   /*==========================================================================
      FUNCTION     : create

      DESCRIPTION: Factory function for RTPParser. Creates the RTPParser based on the payload type


      PARAMETERS :  payloadType(specified in tables 4,5 of RFC 3551(RTP Profile for Audio and Video Conferences with Minimal Control))

      Return Value  : Returns the RTP parser object created
     ===========================================================================*/
   RTPParser* RTPParser::create(int32 payloadType, bool tcp)
   {
     MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "RTPParser:create");
     switch( payloadType )
     {
        case RTP_PAYLOAD_MPEG2TS:
        {
          MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH, "RTPParser:creating RTP Parser for RTP_PAYLOAD_MPEG2TS");

          //ToDo:Need to change to MM_New_Args.
          //MM_New_Args is giving compiler error as _ANDROID_ is not defined
          return MM_New_Args(RTPParser, (RTP_PAYLOAD_MPEG2TS, tcp));
        }

        default:
        {
          MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR, "RTPParser:Invalid/Unsupported RTP payload type - %d", payloadType);
          return NULL;
        }
     }
   }


   /*==========================================================================
      FUNCTION     : read

      DESCRIPTION: Reads the RTP payload packets stored. If the requested data is not available,caller will be blocked for maximum of
                            <RTP_PAYLOAD_MAX_WAIT_TIME> micro seconds and return the available data

      PARAMETERS :  offset:offset of the data
                             data:output put buffer to hold data
                             size:requested number of bytes

      Return Value  : Returns number of bytes read
     ===========================================================================*/
   ssize_t RTPParser::read(int64 offset, void *data, size_t size)
   {

      uint64 requestedOffset = offset;
      uint64 readRequestTime = getCurrentSystemTimeMicroS();
      int nBytes = 0;
      while(size > 0)
      {
         if(offset < m_sTSBufferHandler.headOffset)
         {
           MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR, "Data at requested offset %lld is already overwritten",offset);
           return nBytes;
         }
         else if(offset > m_sTSBufferHandler.tailOffset)
         {
           if(nBytes > TS_PKT_LEN)
           {
             MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM, "RTPParser:Read. One TS packet available already return");
             break;
           }
           uint64 readRequestTimeNow = getCurrentSystemTimeMicroS();
           int64 timeDiff = (readRequestTimeNow - readRequestTime);

           if(timeDiff > RTP_PAYLOAD_MAX_WAIT_TIME)
           {
             MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR, "RTPParser:read() requested data is not available.Returning %d bytes", nBytes);
             return nBytes;
           }

           //wait time in micro seconds
           int64_t waitTimeUs = RTP_PAYLOAD_MAX_WAIT_TIME - timeDiff;
           MM_MSG_PRIO3(MM_GENERAL, MM_PRIO_MEDIUM, "RTPParser:Requested offset %lld is more than tail offset %lld,"
                                                   "hence waiting for %lld milli seconds",
                                                   offset, m_sTSBufferHandler.tailOffset, waitTimeUs/1000);
         //Putting the thread in wait state,till the data is available or timeout happens
           int32 waitTime = (int32)(waitTimeUs/1000);
           uint32 *pEvent = NULL;
           int bTimedOut = 0;
           if ( 0 == MM_SignalQ_TimedWait( m_pSignalRtpParserQ, waitTime, (void **) &pEvent, &bTimedOut ) )
           {
               //how much it waited can be found out from system time logs
            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM, "RTPParser:read() MM_SignalQ_TimedWait success");
            continue;
           }
           else
           {
               MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "RTPParser:read() MM_SignalQ_TimedWait returned error");
               return 0 ;
           }
         }
         int64_t offsetIndex =  offset % m_sTSBufferHandler.nBufsize;

         int32_t availableLength = static_cast<int32_t>(MIN((int64_t)size, m_sTSBufferHandler.tailOffset - offset + 1));
         //choosing the length that can go in one memcpy
         int32_t chunk = static_cast<int32_t>(MIN(availableLength, (int64_t)(m_sTSBufferHandler.nBufsize - offsetIndex)));

         memcpy(data, m_sTSBufferHandler.pBuffer + offsetIndex, chunk);

         //LOGI("RTPParser:read:inside read copied %d bytes from offset %lld", chunk, offset);
         data = (void*)((uint8_t*)data + chunk);
         offset += chunk;
         size -= chunk;
         nBytes += chunk;
      }

      int64 currentMaxOffset = requestedOffset + nBytes - 1;
      if(currentMaxOffset > m_nMaxOffsetRead)
      {
        m_nMaxOffsetRead = currentMaxOffset;
        //MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "RTPDataSource:readAt updating maxoffset %lld", m_nMaxOffsetRead);
      }
      //LOGI("RTPParser:read:copied %d bytes from offset %lld.Total bytes read %lld", nBytes, requestedOffset, mNumReadBytes);

      return nBytes;
   }

   /*==========================================================================
      FUNCTION     : processRTPPacket

      DESCRIPTION: parses RTP packet and stores the TS packets in it.

      PARAMETERS :  pRTPPacket: pointer to RTP packet buffer
                           packetLength: length of rtp packet

      Return Value  :
     ===========================================================================*/
   RTPParserStatus RTPParser::processRTPPacket(uint8_t* pRTPPacket,size_t packetLength)
   {
     if(isStatisticsEnabled)
     {
       mTotalPackets++;
       if((mTotalPackets % 1000) == 0)
       {
         printStatistics();
       }
     }

     RTP_Packet_struct rtpPacketStruct;

     RTPParserStatus nStatus = parseRTPHeader(pRTPPacket, packetLength, &rtpPacketStruct);

     if(nStatus != RTP_PARSER_SUCCESS)
     {
       MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "RTPParser:Dropping invalid packet received");
       mInvalidPackets++;
       return nStatus;
     }

     uint16_t payloadType = rtpPacketStruct.payloadType;
     uint32_t ssrcId = rtpPacketStruct.ssrcID;
     uint16_t seqNum = rtpPacketStruct.seqNum;
     uint32_t timeStamp = rtpPacketStruct.timeStamp;

     if(payloadType != RTP_PAYLOAD_MPEG2TS)
     {
       MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_ERROR, "RTPParser:Invalid payload type %d,expected is %d",payloadType, RTP_PAYLOAD_MPEG2TS);
       mInvalidPackets++;
       return RTP_PARSER_ERROR_INVALID_PACKET;
     }

     PCRCheckAndTimeUpdate(rtpPacketStruct);
     //mPrevSeqNumber holds the sequence number of the last packet stored in circular buffer
     uint16_t expectedSeqNum = (uint16_t)(mPrevSeqNum + 1);

     if(mSSRCId == -1)
     {
       //first packet
       MM_MSG_PRIO3(MM_GENERAL, MM_PRIO_HIGH, "RTPParser:First packet SSRC ID %u,sequence number %u,timestamp %u",
                                               ssrcId, seqNum, timeStamp);
       mSSRCId = ssrcId;
       expectedSeqNum = seqNum;
     }
     else if(mSSRCId != ssrcId)
     {
       MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_ERROR, "RTPParser:Discarding packet as SSRC id is not matching."
                                               "SSRC id is %u but received %u", mSSRCId, ssrcId);
       mInvalidPackets++;
       return RTP_PARSER_ERROR_INVALID_PACKET;
     }
     else if(expectedSeqNum != seqNum)
     {
       if(seqNum == getMaxSequenceNumber(expectedSeqNum,seqNum))
       {
         if(!mIsReorderingEnabled)
         {
           /*Number of dropped packets will be calculated in enquepacket if reordering is enabled*/
           MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_HIGH, "RTPParser:Mismatch in seqence numbers.Expected sequence number is %u, but received %u",
                                                   expectedSeqNum, seqNum);
           uint16_t seqNumDiff = (seqNum > expectedSeqNum) ? (uint16_t)(seqNum - expectedSeqNum) : (uint16_t)(seqNum + 65536 - expectedSeqNum);
           mSequenceNumMismatches += seqNumDiff;
         }
       }
       else
       {
         //current RTP packet sequence is smaller than previously stored,hence consirering it as OOO packet
         MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_HIGH, "RTPParser:Out of order packet is received."
                                                "Previously stored packet sequence number is %u, but received %u",
                                                mPrevSeqNum, seqNum);
         mOutOfOrderPackets++;
         return RTP_PARSER_ERROR_OOO_PACKET;
       }
     }

     /*Over write circular buffer only when complete data is read*/
     if(isUnReadDataOverWritten(&rtpPacketStruct) == TRUE)
     {
       MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_ERROR, "RTPDataSource:Demuxer is too slow in reading. Discarding RTP packets."
                                                "maximum offset read %lld, Max offset available %lld",
                                                 m_nMaxOffsetRead,  m_sTSBufferHandler.tailOffset);
       return RTP_PARSER_SUCCESS;
     }

#ifdef RTP_USE_RECVMSG
     mPrevSeqNum = seqNum;
     processRecvDataVector(packetLength - RTP_PACKET_HEADER_LENGTH);
     //updateRTPTimeStamp(timeStamp);
#else/*RTP_USE_RECVMSG*/
     if(mIsReorderingEnabled &&
        ((expectedSeqNum != seqNum) || !mRTPPackets.empty()))
     {
       enquePacket(&rtpPacketStruct);
     }
     else
     {
       pushPayload(&rtpPacketStruct);
       mPrevSeqNum = seqNum;
       if(mFirstPacketBaseTimeUs== -1)
       {
         uint16 nPID = (uint16)((uint16)rtpPacketStruct.pPayloadBuffer[1] << 8) |
                        (uint16)rtpPacketStruct.pPayloadBuffer[2];

         nPID &= 0x1fff;

         //Find the first new video or audio payload
         if(//(rtpPacketStruct.pPayloadBuffer[1] & 0x40/*Payload start Indicator*/) &&
            ((nPID == 0x1011) || /*Video PID*/
             (nPID >= 0x1100 && nPID <= 0x111f)))    /*Audio PID*/
         {
           struct timespec sTime;
           clock_gettime(CLOCK_MONOTONIC, &sTime);

           uint64 currTS = ((uint64)sTime.tv_sec * 1000000/*us*/)
                                + ((uint64)sTime.tv_nsec / 1000);
           mFirstPacketBaseTimeUs = currTS;
           if(mFirstPacketBaseTimeUs == -1)
           {
             mFirstPacketBaseTimeUs -= 1;
           }
           MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,
                      "RTPDataSource: first time packet base time %lld",
                       mFirstPacketBaseTimeUs);
         }
       }
       //updateRTPTimeStamp(timeStamp);
     }
#endif
     return RTP_PARSER_SUCCESS;
   }

   /*==========================================================================
      FUNCTION     : PCRCheckAndTimeUpdate

      DESCRIPTION: Check if PCR base has changed

      PARAMETERS :  rtpPacketStruct: reference to RTP packet structure

      Return Value  :
     ===========================================================================*/
   void RTPParser::PCRCheckAndTimeUpdate(RTP_Packet_struct &rtpPacketStruct)
   {
     uint32 nIter = rtpPacketStruct.nPayloadSize/ TS_PKT_LEN;
     uint8  *pPktPtr = rtpPacketStruct.pPayloadBuffer;

     if(mFirstPacketBaseTimeUs)
     {
         //For all TS in this RTP payload
         for(int i = 0; i < nIter; i++)
         {
             uint16 nPID = (uint16)pPktPtr[1] << 8 |
                                   pPktPtr[2];
             nPID &= 0x1fff;

             if(!m_bPATParsed && nPID == PAT_PID)
             {
                 ParsePATPacket(pPktPtr);
             }

             if(!m_bPMTParsed && m_bPATParsed && nPID == m_nPMTPID)
             {
                 ParsePMTPacket(pPktPtr);
             }

#define PCR_CONSISTENCY_THRESHOLD 2
#define PCR_DISCONTINUITY_THRESHOLD 1000
#define PCR_DRIFT_THERSHOLD 50

             if(m_bPATParsed && m_bPMTParsed && nPID == m_nPCRPID)
             {
                 do
                 {
                     //Extract PCR from PCR PID
                     uint64 nPCR = ((uint64)pPktPtr[6]) << 25 |
                                   ((uint64)pPktPtr[7]) << 17 |
                                   ((uint64)pPktPtr[8]) << 9  |
                                   ((uint64)pPktPtr[9]) << 1  |
                                   ((uint64)pPktPtr[10]) >> 7;


                     uint32 nPCRMs = (uint64)nPCR / 90;

                     MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR, "PCR Base = %lu", nPCRMs);

                     unsigned long nCurrTime = 0;

                     MM_Time_GetTime(&nCurrTime);

                     //Check if PCR has value greater than 2^33
                     if(nPCRMs > WRAP_AROUND_MAGIC_NUM)
                     {
                         MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR, "PCR Invalid %lu", nPCRMs);
                         break;
                     }

                     if(pPktPtr[5] & 0x80)
                     {
                         MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "PCR discontinuity Indication");
                     }

                     //If PCR goes behind, make sure it is not a wrap around.
                     if(nPCRMs < m_nPrevPCRMs)
                     {
                         bool bWrapAround = false;
                         MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,"PCR behind previous check for wraparound");
                         if(nPCRMs + (WRAP_AROUND_MAGIC_NUM - m_nPrevPCRMs) < WRAPAROUND_THRESHOLD)
                         {
                             MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,"Wrap around detected");
                             m_bPCRBaseTaken = false; m_bLocalBaseTimeTaken = false;
                             bWrapAround = true;
                         }

                         MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "PCR changed base. Update base times");
                         m_nPCRJumpValue = m_nPrevPCRMs - nPCRMs
                                                   +nCurrTime - m_nLocalPrevTime;

                         MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_ERROR, "PCR Jump Value = %llu PCR Base time = %ld",
                                                   m_nPCRJumpValue, m_nPCRBaseMs);

                         m_bPCRBaseTaken = false;
                         m_bLocalBaseTimeTaken = false;
                         if(bWrapAround)
                         {
                             m_bBaseTimeAdjusted = true;
                             mFirstPacketBaseTimeUs += (int64_t)((m_nPCRJumpValue) * 1000);
                             m_nPCRJumpValue = 0;
                             m_nLastPCRPrevBase = m_nPrevPCRMs;
                             m_nStartPCRNewBase = nPCRMs;
                             m_bDiscontinuity = true;
                             MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "PCR Base changed. Adjust Basetime");
                         }
                    }

                    //Take Base times for PCR and current local time
                    if(!m_bPCRBaseTaken)
                    {
                        m_bPCRBaseTaken = true;
                        m_nPCRBaseMs = nPCRMs;
                        m_nPrevPCRMs = nPCRMs;
                        m_nPCRConsistency = 0;
                    }

                    if(!m_bLocalBaseTimeTaken)
                    {
                        MM_Time_GetTime(&m_nLocalBaseTime);
                        m_nLocalPrevTime = m_nLocalBaseTime;
                        m_bLocalBaseTimeTaken = true;
                        m_nPrevPCRMs = nPCRMs;
                        break;
                    }


                    uint32 nLocalTimeDiff = nCurrTime - m_nLocalPrevTime;
                    uint32 nPCRDiff = nPCRMs > m_nPrevPCRMs? nPCRMs - m_nPrevPCRMs : m_nPrevPCRMs - nPCRMs;
                    uint32 nTimeDiffDelta = nLocalTimeDiff > nPCRDiff ? nLocalTimeDiff - nPCRDiff : nPCRDiff - nLocalTimeDiff;

                    MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_HIGH, "PCR Diff = %lu Local Diff = %lu", nPCRDiff, nLocalTimeDiff);

                    uint32 nLocalTimeOffset = nCurrTime - m_nLocalBaseTime;
                    uint32 nPCROffset = nPCRMs - m_nPCRBaseMs;

                    if(nTimeDiffDelta > 2000)
                    {
                        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "PCR inconsistency or discontinuity");
                        m_nPCRConsistency = 0;
                    }
                    else
                    {
                        m_nPCRConsistency++;
                    }

                    if(m_nPCRConsistency > 0xfffffffe)
                    {
                        m_nPCRConsistency = PCR_CONSISTENCY_THRESHOLD + 1;
                    }

                    MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_ERROR, "PCR time curr %lu pcr %lu",
                             nLocalTimeOffset, nPCROffset);

                    if(nTimeDiffDelta > PCR_DISCONTINUITY_THRESHOLD)
                    {
                        m_nLastPCRPrevBase = m_nPrevPCRMs;
                        m_nStartPCRNewBase = nPCRMs;
                        MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_ERROR, "PCR set discontinuity before %lu after %lu",
                             m_nLastPCRPrevBase, m_nStartPCRNewBase);
                        m_bDiscontinuity = true;
                    }

                    if(m_nPCRConsistency >  PCR_CONSISTENCY_THRESHOLD)
                    {
                        if(m_nPCRJumpValue)
                        {
                            m_bBaseTimeAdjusted = true;
                            mFirstPacketBaseTimeUs += (int64_t)((m_nPCRJumpValue) * 1000);
                            m_nPCRJumpValue = 0;
                            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "PCR Base changed. Adjust Basetime");
                        }
                        if(nPCROffset > nLocalTimeOffset && nPCROffset - nLocalTimeOffset > 50)
                        {
                            m_bBaseTimeAdjusted = true;
                            mFirstPacketBaseTimeUs -= (int64_t)((nPCROffset - nLocalTimeOffset) * 1000);
                            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "PCR running ahead of current time. Adjust Basetime");
                            m_nLocalBaseTime -= (nPCROffset - nLocalTimeOffset);
                        }
                        else if(nLocalTimeOffset > nPCROffset && nLocalTimeOffset - nPCROffset > PCR_DISCONTINUITY_THRESHOLD)
                        {
                            //If PCR progression is behind by 1 seconds then it must either be a network
                            //latency or a discontinuity. Make room for both.
                            m_bBaseTimeAdjusted = true;
                            mFirstPacketBaseTimeUs += (int64_t)((nLocalTimeOffset - nPCROffset) * 1000);
                            MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_ERROR, "PCR behind current time. Adjust Basetime curr %lu pcr %lu",
                                                  nLocalTimeOffset, nPCROffset);
                            m_nLocalBaseTime += (nLocalTimeOffset - nPCROffset);
                        }
                        else if(nLocalTimeOffset > nPCROffset && nLocalTimeOffset - nPCROffset > PCR_DRIFT_THERSHOLD)
                        {
                            m_nDelayedPCR++;
                            if(m_nDelayedPCR > 10)
                            {
                                //If PCR progression is behind by 1 seconds then it must either be a network
                                //latency or a discontinuity. Make room for both.
                                m_bBaseTimeAdjusted = true;
                                mFirstPacketBaseTimeUs += (nLocalTimeOffset - nPCROffset) * 1000;
                                m_nLocalBaseTime += nLocalTimeOffset - nPCROffset;
                                MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "PCR consistently slightly behind current time. Adjust Basetime");
                                //                  m_bLocalBaseTimeTaken = false;
             //                  m_bPCRBaseTaken = false;
                                m_nDelayedPCR = 0;
                            }
                        }
                        else
                        {
                            m_nDelayedPCR = 0;
                        }
                    }
                    m_nLocalPrevTime = nCurrTime;
                    m_nPrevPCRMs     = nPCRMs;
                }while(0);
             }
         }
         pPktPtr += TS_PKT_LEN;
     }
   }

   /*==========================================================================
      FUNCTION     : IsBaseTimeAdjusted

      DESCRIPTION: Check if base time has been changed

      PARAMETERS : nDicontinuity - A discontinuity is detected or not
                   pnTsBefore - pointer to previous base of PCR
                   pnTsAfter - Pointer to new base of PCR

      Return Value  :
     ===========================================================================*/
   bool RTPParser::IsBaseTimeAdjusted(bool *bDiscontinuity, uint64 *pnTSBefore, uint64* pnTSAfter)
   {
     if(pnTSBefore && pnTSAfter && bDiscontinuity)
     {
       *pnTSBefore = m_nLastPCRPrevBase;
       *pnTSAfter  = m_nStartPCRNewBase;
       *bDiscontinuity = m_bDiscontinuity;
     }
     return m_bBaseTimeAdjusted;
   }

   /*==========================================================================
      FUNCTION     : ParsePMTPacket

      DESCRIPTION: Parses PMT table to find PCR PID

      PARAMETERS : pData - pointer to data

      Return Value  :
     ===========================================================================*/
   void RTPParser::ParsePMTPacket(uint8* pData)
   {
     if(!pData)
     {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "parsePMTPacket Invalid params");
        return;
     }

     uint32 nOffset = 5;//TS header size

     if(pData[3] & 0x20)//Adaptation field
     {
       nOffset += pData[4];
     }

     if(pData[nOffset])
     {
       //Pointer field
       #define PMT_PID_OFFSET 12
       nOffset += pData[nOffset];

       if(nOffset > TS_PKT_LEN || TS_PKT_LEN - nOffset < PMT_PID_OFFSET)
       {
         MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "PAT in sections, cant parse");
         return;
       }
     }

     //We are looking for PCR PID here.

     /*
       Table ID -                 8
       section syntax indicator - 1
       '0'                        1
       reserved                   2
       section_length             12
       program_number             16
       reserved                   2
       version number             5
       current_next_indicator     1
       section number             8
       last section number        8
       reserved                   3
       PCR_PID                   13
       */

       m_nPCRPID = ((uint16)pData[nOffset + 8]) << 8 |
                   ((uint16)pData[nOffset + 9]);
       m_nPCRPID &= 0x1fff;

       MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "Found PCR PID %lu", m_nPCRPID);

       m_bPMTParsed = true;
   }


   /*==========================================================================
      FUNCTION     : ParsePATPacket

      DESCRIPTION: Parses PAT table to find PMT Pid

      PARAMETERS : pData - pointer to data

      Return Value  :
     ===========================================================================*/
   void RTPParser::ParsePATPacket(uint8* pData)
   {
     if(!pData)
     {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "parsePATPacket Invalid params");
        return;
     }

     uint32 nOffset = 5;//TS header size + pointer

     if(pData[3] & 0x20)
     {
       nOffset += pData[4];
     }

     if(pData[nOffset])
     {
       //Pointer field
       #define PMT_PID_OFFSET 12
       nOffset += pData[nOffset];
       if(nOffset > TS_PKT_LEN || TS_PKT_LEN - nOffset < PMT_PID_OFFSET)
       {
         MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "PAT in sections, cant parse");
         return;
       }
     }
     //We are looking for PMT PID here.

     /*
       Table ID -                 8
       section syntax indicator - 1
       '0'                        1
       reserved                   2
       section_length             12
       transport stream id        16
       reserved                   2
       version number             5
       current_next_indicator     1
       section number             8
       last section number        8
       program number             16
       reserved                   3
       program map PID            13
       */

       m_nPMTPID = ((uint16)pData[nOffset + 10]) << 8 |
                   ((uint16)pData[nOffset + 11]);
       m_nPMTPID &= 0x1fff;

       MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "Found PMT PID %lu", m_nPMTPID);

       m_bPATParsed = true;

       return;
   }

   /*==========================================================================
      FUNCTION     : pushPayload

      DESCRIPTION: Extracts the payload packets from RTP packet and pushes it to the circular buffer

      PARAMETERS :  buffer: RTP packet

      Return Value  : Returns "OK" if for success else error
     ===========================================================================*/
   void RTPParser::pushPayload(RTP_Packet_struct* pRtpPacketStruct)
   {
      uint8_t* data = pRtpPacketStruct->pPayloadBuffer;

      int32_t length = (int32_t)pRtpPacketStruct->nPayloadSize;

      //LOGI("RTPParser:Received RTP packet of payload length %d",length);
#ifdef RTP_DUMP_TS_DATA
      if(m_pDumpFile != NULL)
      {
        uint8_t *dumpBuffer = data;
        fwrite(dumpBuffer, 1, length, m_pDumpFile);
      }
#endif

      while(length > 0)
      {
        //insert the data after tail
        int32_t insertIndex = static_cast<int32_t>((m_sTSBufferHandler.tailOffset + 1) % m_sTSBufferHandler.nBufsize);

        //choosing the length that can be copied in one memcpy
        int32_t chunk = MIN(length, (int32_t)(m_sTSBufferHandler.nBufsize - insertIndex));

        memcpy(m_sTSBufferHandler.pBuffer + insertIndex, data, chunk);

        //LOGI("RTPParser:stored payload length %d in buffer. head is %lld, tail is %lld", chunk, m_sTSBufferHandler.headOffset,
        //                                                                                           m_sTSBufferHandler.tailOffset);

        //updating tail
        m_sTSBufferHandler.tailOffset += chunk;

        //updating head
        if( m_sTSBufferHandler.headOffset == -1)
        {
           m_sTSBufferHandler.headOffset = 0;
        }
        else if((int64_t)insertIndex == (m_sTSBufferHandler.headOffset % (int64_t)m_sTSBufferHandler.nBufsize))//updating head as previous data is overridden
        {
           //head offset is the oldest data avaiable in buffer i.e. byte after over ridden data
           m_sTSBufferHandler.headOffset += chunk;
        }

        data += chunk;
        length -= chunk;
      }

      //signaling any thread blocked on read
      //sending signal to fetch data
    MM_Signal_Set(m_pDataAvailableSignal);
   }

   /*==========================================================================
      FUNCTION   : enquePacket

      DESCRIPTION: Stores the RTP packet in list sorted by sequence number in descending order(considering roll over)

      PARAMETERS :  pRtpPacketStruct: pointer to rtp packet struct

    ===========================================================================*/
   void RTPParser::enquePacket(RTP_Packet_struct* pRtpPacketStruct)
   {
      uint16_t seqNum = pRtpPacketStruct->seqNum;
      uint32_t timeStamp = pRtpPacketStruct->timeStamp;

      List<RTP_Packet_struct>::iterator itr = mRTPPackets.begin();

      while (itr != mRTPPackets.end())
      {
        if(seqNum == getMaxSequenceNumber((*itr).seqNum,seqNum))
        {
          break;
        }
        itr++;
      }

      //end() returns the node after the last element
      if (itr != mRTPPackets.end() && (*itr).seqNum == seqNum)
      {
        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR, "RTPParser:Dropping packet with duplicate sequence number %u", seqNum);
        return;
      }

      size_t reorderQueueLen = mRTPPackets.size();

      RTP_Packet_struct rtpPacketStruct;
      //ToDo:need to find out whether any MMOSAL API is available for memcpy
      memcpy(&rtpPacketStruct, pRtpPacketStruct, sizeof(RTP_Packet_struct));


      rtpPacketStruct.pPayloadBuffer = mReorderPackets[reorderQueueLen];
      memcpy(rtpPacketStruct.pPayloadBuffer, pRtpPacketStruct->pPayloadBuffer, rtpPacketStruct.nPayloadSize);


      //Insert function inserts the node before given itr
      mRTPPackets.insert(itr, rtpPacketStruct);
      reorderQueueLen++;

      MM_MSG_PRIO3(MM_GENERAL, MM_PRIO_DEBUG, "RTPParser:Enqueued packet with sequence number %u, payload buffer %p and size %d",
                                               seqNum, rtpPacketStruct.pPayloadBuffer, rtpPacketStruct.nPayloadSize);

      if(reorderQueueLen >= RTP_REORDER_PACKETS_QUEUE_LENGTH)
      {
        flushRTPPacketsQueue();
      }
   }

   /*==========================================================================
      FUNCTION   : flushPacketsQueue

      DESCRIPTION: Flush the rtp packets from list to circular buffer

      Return Value  : Returns "OK" if for success else error
   ===========================================================================*/
   status_t RTPParser::flushRTPPacketsQueue()
   {
     if(!mRTPPackets.empty())
     {
       //starting from end as packets are stored in descending order
       List<RTP_Packet_struct>::iterator itr = mRTPPackets.end();

       do
       {
         //end() returns the one after the last element, hence decrementing itr by one
         itr--;
         RTP_Packet_struct rtpPacket = (*itr);

         MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_DEBUG,"RTPParser:Storing packet with sequence number %u in to circular buffer",
                                                rtpPacket.seqNum);
         pushPayload(&rtpPacket);
         updateRTPTimeStamp(rtpPacket.timeStamp);

         uint16_t expectedSeqNum = (uint16_t)(mPrevSeqNum + 1);
         if(rtpPacket.seqNum != expectedSeqNum)
         {
           MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_HIGH,"RTPParser:Mismatch in seqence numbers.Expected sequence number is %u, but received %u",
                                                 expectedSeqNum, rtpPacket.seqNum);
           uint16_t seqNumDiff = (rtpPacket.seqNum > expectedSeqNum) ? (uint16_t)(rtpPacket.seqNum - expectedSeqNum) : (uint16_t)(rtpPacket.seqNum + 65536 - expectedSeqNum);
           mSequenceNumMismatches += seqNumDiff;
         }

         mPrevSeqNum = rtpPacket.seqNum;
       }while(itr != mRTPPackets.begin());
     }

     mRTPPackets.clear();
     return OK;
   }

   /*==========================================================================
      FUNCTION     : processRecvDataVector

      DESCRIPTION:

      PARAMETERS :

      Return Value  : Returns "OK" if parsing is successful else error value
     ===========================================================================*/
   status_t RTPParser::processRecvDataVector(size_t totalLengthRecvd)
   {
      if(totalLengthRecvd <= 0)
      {
         MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,"RTPParser:No data recevived. Nothing to process");
         return OK;
      }

#ifdef RTP_DUMP_TS_DATA
      if(m_pDumpFile != NULL)
      {
        uint8_t *dumpBuffer = m_sTSBufferHandler.pBuffer + mRecvBuffer_offset_1;
        fwrite(dumpBuffer, 1, mRecvBuffer_offset_size_1, m_pDumpFile);
        if(mRecvBuffer_offset_size_2  > 0)
        {
          dumpBuffer = m_sTSBufferHandler.pBuffer +mRecvBuffer_offset_2;
          fwrite(dumpBuffer, 1, mRecvBuffer_offset_size_2, m_pDumpFile);
        }
      }
#endif
      //updating tail offset,such that it contains the maximum data offset available
      m_sTSBufferHandler.tailOffset += totalLengthRecvd;

      //Updating recv buffer offsets for the next read
      mRecvBuffer_offset_1 =  (m_sTSBufferHandler.tailOffset + 1) % m_sTSBufferHandler.nBufsize;
      mRecvBuffer_offset_size_1 = MIN(m_sTSBufferHandler.nBufsize - mRecvBuffer_offset_1, RTP_PACKET_MAXIMUM_PAYLOAD_SIZE);

      mRecvBuffer_offset_size_2 = RTP_PACKET_MAXIMUM_PAYLOAD_SIZE - mRecvBuffer_offset_size_1;
      if(mRecvBuffer_offset_size_2  > 0)
      {
        mRecvBuffer_offset_2 =  (mRecvBuffer_offset_1 + mRecvBuffer_offset_size_1) % (m_sTSBufferHandler.nBufsize);
      }
      else
      {
        mRecvBuffer_offset_2 = -1;
      }

      //updating head offset
      if( m_sTSBufferHandler.headOffset == -1)//updating first time
      {
         m_sTSBufferHandler.headOffset = 0;
      }
      else
      {
        size_t recvBuffersLength = mRecvBuffer_offset_size_1 + mRecvBuffer_offset_size_2;

        size_t emptySpaceAvailable = 0;
        if(m_sTSBufferHandler.tailOffset < (int64_t)m_sTSBufferHandler.nBufsize)
        {
           emptySpaceAvailable = m_sTSBufferHandler.nBufsize - m_sTSBufferHandler.tailOffset - 1;
        }

        size_t overRiddenDataLength = (recvBuffersLength > emptySpaceAvailable) ? (recvBuffersLength - emptySpaceAvailable) : 0;

        m_sTSBufferHandler.headOffset += overRiddenDataLength;
      }

      //signaling any thread blocked on read
         MM_Signal_Set(m_pDataAvailableSignal);
      //LOGE("RTPParser:WFD:debug:offsets:headoffset %lld,tailoffset %lld", m_sTSBufferHandler.headOffset, m_sTSBufferHandler.tailOffset);

      return OK;

   }

   /*==========================================================================
      FUNCTION     : getRecvDataVector

      DESCRIPTION:

      PARAMETERS :

      Return Value  : Returns "OK" if parsing is successful else error value
     ===========================================================================*/
   status_t RTPParser::getRecvDataPositions(uint8_t** recvVector_1, int32* recvVector_length_1,
                                                                 uint8_t** recvVector_2, int32* recvVector_length_2)
   {
      int numVectors = 1;

      //first time invocation
      if(mRecvBuffer_offset_1 == -1)
      {
          mRecvBuffer_offset_1 = 0;
          mRecvBuffer_offset_size_1 = RTP_PACKET_MAXIMUM_PAYLOAD_SIZE;
          mRecvBuffer_offset_2 = -1;
          mRecvBuffer_offset_size_2 = 0;

          numVectors = 1;
      }
      else if(mRecvBuffer_offset_size_2 != 0)//recvBuffers already updated in processRecvDataVector
      {
         numVectors = 2;
      }


      *recvVector_1 = m_sTSBufferHandler.pBuffer + mRecvBuffer_offset_1;
      *recvVector_length_1  = (int32)mRecvBuffer_offset_size_1;

      if(numVectors == 2)
      {
        *recvVector_2 = m_sTSBufferHandler.pBuffer + mRecvBuffer_offset_2;
        *recvVector_length_2  = (int32)mRecvBuffer_offset_size_2;
      }
      else
      {
        *recvVector_2 = NULL;
        *recvVector_length_2 = 0;
      }

      //LOGE("RTPParser:WFD:debug:offsets:recvbuffer1 offset %lld,size %d.recv buffer2 offset %lld, size %d", mRecvBuffer_offset_1 ,
                                          //mRecvBuffer_offset_size_1, mRecvBuffer_offset_2, mRecvBuffer_offset_size_2);
      return OK;
   }

  /*==========================================================================
      FUNCTION     : parseRTPHeader

      DESCRIPTION: parses RTP packet header and constructs a RTPPacket based on packet and returns it.

      PARAMETERS :  buffer: RTP packet buffer
                              packetLength:length of buffer

      Return Value  :
     ===========================================================================*/
   RTPParserStatus RTPParser::parseRTPHeader(uint8_t* pRTPPacket, size_t packetLength, RTP_Packet_struct* pRTPPacketStruct)
   {
      if(pRTPPacket == NULL || pRTPPacketStruct == NULL)
      {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "RTPParser:RTPPacket is null");
        return RTP_PARSER_ERROR_INVALID_PACKET;
      }
      /**-------------------------------------------------------------------------
             ___RFC 3350 - RTP A transport protocol for real time applications___

             0                   1                   2                   3
             0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
            +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
            |V=2|P|X|  CC   |M|      PT      |       sequence number          |
            +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
            |                           timestamp                           |
            +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
            |           synchronization source (SSRC) identifier              |
            +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
            |            contributing source (CSRC) identifiers              |
            |                             ....                              |
            +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

            ----------------------------------------------------------------------------
            */

      size_t size = packetLength;
      if (size < 12)
      {
         MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR, "RTPParser:%d bytes is too short for RTP header.Minumum length is 12 bytes", size);
         return RTP_PARSER_ERROR_INVALID_PACKET;
      }

      const uint8_t *data = pRTPPacket;

   /*   if(m_bIsTCP)
      {
          data += 2;
      }*/

      //first two bits of first byte gives version,current version of RTP is 2
      if ((data[0] >> 6) != 2)
      {
         MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR, "RTPParser:Unsupported RTP version %d",(data[0]>>6));
         return RTP_PARSER_ERROR_INVALID_PACKET;
      }

      //third bit of first byte indicates the presence of padding
      if (data[0] & 0x20)
      {
        // The last octet of the padding contains a count of how many padding octets should be ignored, including itself
        size_t paddingLength = data[size - 1];
        if (paddingLength + 12 > size)
        {
            MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR, "RTPParser:header length is too small after removing %d padding bytes", paddingLength);
            return RTP_PARSER_ERROR_INVALID_PACKET;
        }
        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "RTPParser:removing %d padding bytes", paddingLength);
        size -= paddingLength;
      }

      //last 4 bits of first octet gives the number of 4 byte CSRC's that follow the fixed header
      int numCSRCs = data[0] & 0x0f;

      size_t payloadOffset = RTP_PACKET_HEADER_LENGTH + 4 * numCSRCs;

      if (size < payloadOffset)
      {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "RTPParser:Not enough data to fit the basic header and all the CSRC entries");
        return RTP_PARSER_ERROR_INVALID_PACKET;
      }

      //4th bit of the first byte indicates the presence of extension bit
      if (data[0] & 0x10) {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_HIGH,"RTPParser:Extension header is present");

        if (size < payloadOffset + 4) {
            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR,"RTPParser:Not enough data to fit the basic header, "
                                                  "all CSRC entries and the first 4 bytes of the extension header.");
            return RTP_PARSER_ERROR_INVALID_PACKET;
        }

        //Extension header data will be appended to the RTP header,following the CSRC list
        const uint8_t *extensionData = &data[payloadOffset];

        //the first 16 bits of the header extension are left open for distinguishing identifiers and the subsequent 16 bits represents extension header length
        size_t extensionLength =
            4 * ((0x00FF & extensionData[2]) << 8 | extensionData[3]);

        if (size < payloadOffset + 4 + extensionLength) {
            return RTP_PARSER_ERROR_INVALID_PACKET;
        }

        payloadOffset += 4 + extensionLength;
      }

      //second byte first bit is "marker" bit(data[1] >> 7) and the other seven bits gives the "payload type"
      pRTPPacketStruct->payloadType =   data[1] & 0x7f;

      //3rd and 4th bytes has sequence number(in network byte order i.e. big endian)
      pRTPPacketStruct->seqNum = ARRAY_NTOHS(data, 2);

      //5 to 8 bytes give time stamp in network byte order
      pRTPPacketStruct->timeStamp = ARRAY_NTOHL(data, 4);

      //9 to 13 bytes give SSRC id in network byte order
      pRTPPacketStruct->ssrcID = ARRAY_NTOHL(data, 8);

      pRTPPacketStruct->pPayloadBuffer = pRTPPacket + payloadOffset;
      pRTPPacketStruct->nPayloadSize = packetLength - payloadOffset;

      return RTP_PARSER_SUCCESS;
   }

    /*==========================================================================
      FUNCTION     : isUnReadDataOverWritten

      DESCRIPTION: Utility function to check whether any unread data will be overwritten if we push this packet in to circular buffer

      PARAMETERS :  pRTPPacket: pointer to RTP packet buffer

      Return Value  : Returns true if any unread data is overwritten if we push this packet in to circular buffer
     ===========================================================================*/

    boolean RTPParser::isUnReadDataOverWritten(RTP_Packet_struct* pRTPPacket)
    {
      boolean unReadDataOverWritten = FALSE;
      if((pRTPPacket != NULL) &&
         (m_nMaxOffsetRead >= 0) &&
         (m_sTSBufferHandler.tailOffset >= 0) &&
         (m_nMaxOffsetRead != m_sTSBufferHandler.tailOffset))
      {
        int64 maxReadOffset = m_nMaxOffsetRead % m_sTSBufferHandler.nBufsize;
        int64 tailOffset = m_sTSBufferHandler.tailOffset % m_sTSBufferHandler.nBufsize;
        int64 maxWrittenOffset = (m_sTSBufferHandler.tailOffset + pRTPPacket->nPayloadSize) % m_sTSBufferHandler.nBufsize;

        if(maxReadOffset < tailOffset)
        {
          if((maxWrittenOffset >= maxReadOffset) &&
             (maxWrittenOffset <= tailOffset))
          {
            unReadDataOverWritten = TRUE;
          }
        }
        else
        {
          if((maxWrittenOffset >= maxReadOffset) ||
             (maxWrittenOffset <= tailOffset))
          {
            unReadDataOverWritten = TRUE;
          }
        }
      }
      return unReadDataOverWritten;
    }

    /*==========================================================================
     FUNCTION     : getHeadOffset

     DESCRIPTION: Returns least offset available to be read by client.

     Return Value  : Returns least offset available to be read by client.
    ===========================================================================*/
    int64 RTPParser::getHeadOffset()
    {
       int64 availOffset = 0;
       if(m_sTSBufferHandler.headOffset)
       {
          availOffset = m_sTSBufferHandler.headOffset;
       }
       MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,"RTPParser:getHeadOffset available is %lld", availOffset);
       return availOffset;
    }

    void RTPParser::updateRTPPortVars()
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_DEBUG,
            "RTPParser:Reset base time, head & tail offsets");
        mFirstPacketBaseTimeUs = -1;
        m_sTSBufferHandler.headOffset = -1;
        m_sTSBufferHandler.tailOffset = -1;
    }

    /*==========================================================================
      FUNCTION     : getNumBytesAvailable

      DESCRIPTION: Returns the number of bytes downloaded

      Return Value  : Returns the number of bytes downloaded and "0" if no data is available
     ===========================================================================*/
   int64 RTPParser::getNumBytesAvailable()
   {
      int64 availOffset = 0;
      if(m_sTSBufferHandler.tailOffset != -1)
      {
         availOffset = m_sTSBufferHandler.tailOffset + 1;
      }
      //LOGI("RTPParser:offset available is %lld", availOffset);
      return availOffset;
   }

   void RTPParser::updateRTPTimeStamp(int64_t rtpPacketTimeStamp)
   {

       MM_CriticalSection_Enter(m_TimeStampCS);

      //converting the 90khz rtp clock value to micro seconds
      mLatestPacketTimeStamp = (rtpPacketTimeStamp * 100) / 9;
      mLatestPacketRecvdTime = getCurrentSystemTimeMicroS();
      if(mFirstPacketTimeStamp == -1)
      {
         mFirstPacketTimeStamp = mLatestPacketTimeStamp;
         MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_HIGH,"RTPParser:RTP packet timestamp%lld, system time %lld", mLatestPacketTimeStamp, mLatestPacketRecvdTime);
      }

      MM_CriticalSection_Leave(m_TimeStampCS);
      MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_HIGH,"RTPParser:RTP packet timestamp%lld, system time %lld", mLatestPacketTimeStamp, mLatestPacketRecvdTime);
   }

   /*==========================================================================
      FUNCTION     : getRTPBaseTimeUs

      DESCRIPTION: Returns the Base time for first packet

      Return Value  : Return
     ===========================================================================*/
   int64_t RTPParser::getRTPBaseTimeUs()
   {
       MM_CriticalSection_Enter(m_TimeStampCS);
       MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,"RTPParser:RTP packet first timestamp%lld", mFirstPacketBaseTimeUs);
       MM_CriticalSection_Leave(m_TimeStampCS);
       m_bBaseTimeAdjusted = false;
       m_nLastPCRPrevBase = 0;
       m_nStartPCRNewBase = 0;
       m_bDiscontinuity = false;
       return mFirstPacketBaseTimeUs;
   }


   /*==========================================================================
      FUNCTION     : getRTPReferenceTimeStamp

      DESCRIPTION: Returns the reference time relative to RTP

      Return Value  : Return
     ===========================================================================*/
   int64_t RTPParser::getRTPReferenceTimeStamp()
   {

      MM_CriticalSection_Enter(m_TimeStampCS);
      uint64 currentTime =getCurrentSystemTimeMicroS();
      int64 diffTime = currentTime - mLatestPacketRecvdTime;
      int64 refTime = mLatestPacketTimeStamp;// + diffTime - mFirstPacketTimeStamp;

      //LOGI("RTPParser:WFD:debug:RTP reference Time %lld", refTime);
     MM_CriticalSection_Leave(m_TimeStampCS);
      return refTime;
   }

   //ToDo:Change the return type
   status_t RTPParser::allocatePayloadBuffer(size_t nBufSize)
   {
     m_sTSBufferHandler.nBufsize = nBufSize;

     MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH,"RTPParser:allocating payload buffer of size %d bytes", m_sTSBufferHandler.nBufsize);
     if((m_sTSBufferHandler.pBuffer = (uint8_t*)MM_Malloc(m_sTSBufferHandler.nBufsize)) == NULL)
     {
       MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "RTPParser:Error in payload buffer memory allocation");
       return NO_MEMORY;
     }
     return OK;
   }
   void RTPParser::init()
   {
#ifdef RTP_USE_RECVMSG
     //reordering can't be happened with recvmsg as payload is stored directly in circular buffer
     mIsReorderingEnabled = false;
#else
     mIsReorderingEnabled = true;
#endif

     if(mIsReorderingEnabled)
     {
       //allocating reorder packet buffers
       for(int i = 0; i < RTP_REORDER_PACKETS_QUEUE_LENGTH ; i++)
       {
          mReorderPackets[i] = (uint8_t*)MM_Malloc(RTP_PACKET_MAXIMUM_PAYLOAD_SIZE);
       }
     }

     mSSRCId = -1;

     //initializing ring buffer parameters
     m_sTSBufferHandler.headOffset = -1;
     m_sTSBufferHandler.tailOffset = -1;
     m_sTSBufferHandler.nBufsize = 0;
     m_sTSBufferHandler.pBuffer = NULL;

     //initializing recvbuffer vector parameters
     mRecvBuffer_offset_1 = -1;
     mRecvBuffer_offset_size_1 = 0;
     mRecvBuffer_offset_2 = -1;
     mRecvBuffer_offset_size_2 = 0;

     //initializing RTP timestamps
     mFirstPacketBaseTimeUs = -1;
     mFirstPacketTimeStamp = -1;
     mLatestPacketTimeStamp = -1;
     mLatestPacketRecvdTime = -1;

     //initializing statistics parameters
     isStatisticsEnabled = true;
     mTotalPackets = 0;
     mInvalidPackets = 0;
     mOutOfOrderPackets = 0;
     mSequenceNumMismatches = 0;

     m_nMaxOffsetRead = -1;

     m_pDumpFile = NULL;
     m_TimeStampCS = NULL;

        if(MM_CriticalSection_Create(&m_TimeStampCS) != 0)
         {
            MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "RTPParser:Error in CS creation");
         }
 //creating signal queue
    if( (MM_SignalQ_Create(&m_pSignalRtpParserQ) != 0) ||
          (MM_Signal_Create( m_pSignalRtpParserQ, (void *) &DATA_AVAILABLE_SIGNAL, NULL, &m_pDataAvailableSignal) != 0))
    {
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "RTPParser:error in creating signal queue");
    }

   }

   void RTPParser::printStatistics()
   {
     if(isStatisticsEnabled)
     {
       MM_MSG_PRIO4(MM_GENERAL, MM_PRIO_HIGH, "RTPParser:statistics:totalpackets %lld, packets dropped %lld, out of orderPackets %lld, "
                                              "InvalidPackets %lld",
                                              mTotalPackets, mSequenceNumMismatches, mOutOfOrderPackets, mInvalidPackets);
       MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_HIGH, "RTPParser:head offset %lld,tail offser %lld",
                                               m_sTSBufferHandler.headOffset, m_sTSBufferHandler.tailOffset);
     }
   }

   /*==========================================================================
      FUNCTION     : getMaxSequenceNumber

      DESCRIPTION: Returns the maximum sequence numbers considering the rollover condition
                   This funciton should be used only for comparing successive or near by packets

      Return Value  : returns the maximum of given sequence number
   ===========================================================================*/
   uint16_t RTPParser::getMaxSequenceNumber(uint16_t seqNum1,uint16_t seqNum2)
   {
     const uint16_t SEQ_NUM_ROLL_OVER_DIFF = 60000;
     uint16_t seqNumDifference = (uint16_t)ABSDIFF(seqNum1, seqNum2);

     if((seqNum1 > seqNum2 && seqNumDifference < SEQ_NUM_ROLL_OVER_DIFF) ||
                                                (seqNum1 < seqNum2 && seqNumDifference > SEQ_NUM_ROLL_OVER_DIFF))
     {
       /*Comparing with SEQ_NUM_ROLL_OVER_DIFF to handle packet drops/out of order  at 16-bit boundary
               *E.g:1.Comparing packets with seq no. 65534,2 implies 65,534 pcket is received after receving packets wit seq. no.s
               *      65535,0 and 1 respectively(out of order scenario)
               *    2.Packet with seq. number 1 is received but expected is 65,535, implies 65,535 and 0 packets are dropped(packet drop scenario)
               */
       return seqNum1;
     }
     return seqNum2;
   }
uint64 RTPParser::getCurrentSystemTimeMicroS(void)
{
    uint64 readRequestTime = 0;
    MM_Time_GetTimeEx(&readRequestTime );
    return readRequestTime *1000;
}

void RTPParser::checkForRTPDumpStatus(void *ptr)
{
    RTPParser* rtpparser = (RTPParser*)ptr;
    char szTemp[PROPERTY_VALUE_MAX];
    memset(szTemp,0,sizeof(szTemp));

    if(property_get("persist.sys.enable_RTPDumps",szTemp,"0") < 0)
    {
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "RTPParser: property_get failed to fetch persist.sys.enable_RTPDumps status");
        return;
    }

    if(strcmp(szTemp,"1") == 0)
    {
        if(rtpparser->m_pDumpFile == NULL)
        {
            rtpparser->m_pDumpFile = fopen("/data/media/rtpdump.ts", "ab");
        }
    }
}

}//android namespace
