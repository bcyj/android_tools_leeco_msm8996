/* =======================================================================
                              PESParser.cpp
DESCRIPTION
  Meaningful description of the definitions contained in this file.
  Description must specify if the module is portable specific, mobile
  specific, or common to both, and it should alert the reader if the
  module contains any conditional definitions which tailors the module to
  different targets.

Copyright (c) 2009-2015 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
========================================================================== */

/* =======================================================================
                             PERFORCE HEADER
$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/MP2ParserLib/main/latest/src/PESParser.cpp#100 $
========================================================================== */
#include "MP2StreamParser.h"
#include "string.h"
#include "MMDebugMsg.h"
#include "MMMemory.h"
#include "MMMalloc.h"
#include "math.h"
#include "H264HeaderParser.h"
#include "parserinternaldefs.h"

/*! ======================================================================
@brief  Get Available TimeStamp from the last PES packet

@detail    Starts parsing PCI packet encountered in TS or PS

@param[in] N/A

@return    MP2STREAM_SUCCESS is parsing is successful otherwise returns appropriate error.
@note      None.
========================================================================== */

void MP2StreamParser::GetClipDurationFromPTS(uint32 trackId)
{
  bool bParseMultiplePESPkts = true;
  MP2StreamStatus retError = MP2STREAM_DEFAULT_ERROR;
  bool bContinue = true;
  if(!m_sContext.bGetLastPTS)
  {
    return;
  }
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW,"GetClipDurationFromPTS");
  uint32 ulChunkCount=1;
  uint64 ullOffset = 0;
  uint32 valCode =0;
  int32* nBytesRead=NULL;
  uint8 *ucTempBuf=(uint8*)MM_Malloc(MPEG2_FILE_READ_CHUNK_SIZE);

  if (NULL == ucTempBuf)
  {
    return;
  }

  // We will start scanning for PTS value from
  // the end of the file and break whenever we
  // find the last PTS in the file
  while(ulChunkCount*MPEG2_FILE_READ_CHUNK_SIZE < m_nFileSize)
  {
    if((ullOffset == 0) || (ullOffset >= MPEG2_FILE_READ_CHUNK_SIZE))
    {
      if (!readMpeg2StreamData(
            m_nFileSize -(ulChunkCount*MPEG2_FILE_READ_CHUNK_SIZE),
            MPEG2_FILE_READ_CHUNK_SIZE,ucTempBuf, MPEG2_FILE_READ_CHUNK_SIZE,
            m_pUserData ) )
      {
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
            "GetClipDurationFromPTS File Read Failure");
        break;
      }
      else
      {
        ullOffset = MPEG2_FILE_READ_CHUNK_SIZE-1;
      }
    }
    uint32 codeVal = getBytesValue((int)4,&ucTempBuf[ullOffset]);
    while(codeVal != MP2_PACK_START_CODE && ullOffset > 0)
    {
      ullOffset--;
      codeVal = getBytesValue((int)4,&ucTempBuf[ullOffset]);
    }
    if(ullOffset==0)
    {
      //No Start code in this chunk
      ulChunkCount++;
    }
    else
    {
      uint64 ullAbsOffset = m_nFileSize - ulChunkCount*MPEG2_FILE_READ_CHUNK_SIZE+ullOffset;
      retError = parsePackHeader(ullAbsOffset,false,
                                 0/*Track ID does not matter.*/,
                                 ucTempBuf,
                                 MPEG2_FILE_READ_CHUNK_SIZE,
                                 nBytesRead);
      if(MP2STREAM_SUCCESS == retError)
      {
        //look ahead to see if there is system header ahead
        if(!readMpeg2StreamData (ullAbsOffset,
                                 (uint32)2*sizeof(MP2_SYS_HDR_START_CODE),
                                 m_pDataBuffer, m_nDataBufferSize,
                                 m_pUserData) )
        {
          retError = m_eParserState;
          bContinue = false;
        }
        if(bContinue)
        {
          uint32 codeVal = getBytesValue((int)4,m_pDataBuffer);
          if( codeVal == MP2_SYS_HDR_START_CODE)
          {
            MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW,
                         "SystemTargetHeader ullOffset %llu",ullOffset);
            ullAbsOffset += sizeof(MP2_SYS_HDR_START_CODE);
            uint16 usLen = getBytesValue(sizeof(uint16),m_pDataBuffer+4);
            //! Do not parse system target header while calculating duration
            //! This will lead to infinite loop.
            //retError = parseSystemTargetHeader(trackId, ullAbsOffset);
            //if(retError == MP2STREAM_SUCCESS)
            {
              ullAbsOffset += usLen + 2;
              //Now read ahead to see if there is a PES packet ahead
              if(!readMpeg2StreamData (ullAbsOffset, 4,
                                       m_pDataBuffer, m_nDataBufferSize,
                                       m_pUserData) )
              {
                retError = m_eParserState;
                bContinue = false;
              }
            }
          }//if( codeVal == MP2_SYS_HDR_START_CODE)

          uint32 valCode = 0;
          while( isPESPacket(m_pDataBuffer,&valCode) &&
                 (retError == MP2STREAM_SUCCESS) )
          {
            //There is a PES packet,parse it before moving onto next pack
            retError = parsePESPacket(ullAbsOffset,valCode,trackId,
                ucTempBuf,MPEG2_FILE_READ_CHUNK_SIZE,nBytesRead);

            //Make sure we are nor reading beyond file size
            if(ullAbsOffset >= m_nFileSize)
            {
              retError = MP2STREAM_EOF;
              MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
                  "parsePackHeader EOF detected....");
              break;
            }
          }//while( isPESPacket(m_pDataBuffer,&valCode)&&
        }//if(bContinue)
      }
      else
      {
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
            "parsePackHeader Failed !");
      }
      if ( 0!= m_currPESPkt.pts)
      {
        double nBaseTime;
        // Found the last PES packet with a TS
        m_nEndPESPktTS = (uint64)m_currPESPkt.pts;
        // Adjust with BaseTime
        GetBaseTime(trackId, &nBaseTime);
        m_nEndPESPktTS -= (uint64)nBaseTime;
        MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
            "GetClipDurationFromPTS m_nEndPESPktTS=%llu",
            m_nEndPESPktTS);
        break;
      }
      else
      {
        // NO TS in this PES packet, go back to previous PES packet
        MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
            "GetClipDurationFromPTS Error Calculated m_nEndPESPktTS=%llu",
            m_nEndPESPktTS);
        ullOffset = ullOffset - 1;
      }
    }
  }
  m_sContext.bGetLastPTS = false;
  if(ucTempBuf!=NULL)
  {
    MM_Free(ucTempBuf);
    ucTempBuf = NULL;
  }
  return;
}
/*! ======================================================================
@brief  Get Available TimeStamp from the last PES packet in a TS stream

@detail    Starts parsing from the end of file for a PES packet with
           non-zero PTS value

@param[in] Track ID

@return    Void.
@note      None.
========================================================================== */
void MP2StreamParser::TSGetClipDurationFromPTS(uint32 trackId)
{
  bool bParseMultiplePESPkts = true;
  MP2StreamStatus retError = MP2STREAM_DEFAULT_ERROR;
  bool bContinue = true;
  if(!m_sContext.bGetLastPTS)
  {
    return;
  }
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW,"GetClipDurationFromPTS");
  uint32 ulChunkCount=0;
  uint64 ullOffset = 0;
  uint32 valCode =0;
  int32* nBytesRead=NULL;
  uint8 *ucTempBuf=(uint8*)MM_Malloc(MPEG2_FILE_READ_CHUNK_SIZE);

  if (NULL == ucTempBuf)
  {
    return;
  }

  // We will start scanning for PTS value from
  // the end of the file and break whenever we
  // find the last PTS in the file
  while(ulChunkCount*MPEG2_FILE_READ_CHUNK_SIZE < m_nFileSize)
  {
    if((ullOffset == 4) || (ullOffset >= MPEG2_FILE_READ_CHUNK_SIZE))
    {
      uint32 ulBytesRead = \
        readMpeg2StreamData(
                m_nFileSize - ((ulChunkCount+1)*MPEG2_FILE_READ_CHUNK_SIZE)+4,
                MPEG2_FILE_READ_CHUNK_SIZE,
                ucTempBuf, m_nDataBufferSize,
                m_pUserData);
      if (!ulBytesRead)
      {
        MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
            "GetClipDurationFromPTS File Read Failure m_nFileSize=%llu", m_nFileSize);
        break;
      }
      else
      {
        ullOffset = ulBytesRead-1;
      }
    }
    uint32 val = 0;
    while(!isPESPacket(&ucTempBuf[ullOffset],&val) && ullOffset > 4)
    {
      ullOffset--;
    }
    if(ullOffset==4)
    {
      //No Start code in this chunk
      ulChunkCount++;
    }
    else
    {
      uint64 ullAbsOffset = m_nFileSize -
        (ulChunkCount+1)*MPEG2_FILE_READ_CHUNK_SIZE+ullOffset;
      retError = parsePESPacket(ullAbsOffset,val,trackId,
          ucTempBuf,MPEG2_FILE_READ_CHUNK_SIZE,nBytesRead);

      if ( 0!= m_currPESPkt.pts)
      {
        double nBaseTime;
        // Found the last PES packet with a TS
        m_nEndPESPktTS = (uint64)m_currPESPkt.pts;
        // Adjust with BaseTime
        GetBaseTime(trackId, &nBaseTime);
        m_nEndPESPktTS -= (uint64)nBaseTime;
        MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
            "GetClipDurationFromPTS m_nEndPESPktTS=%llu",
            m_nEndPESPktTS);
        m_sContext.bGetLastPTS = false;
        break;
      }
      else
      {
        // NO TS in this PES packet, go back to previous PES packet
        MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
            "GetClipDurationFromPTS Error Calculated m_nEndPESPktTS=%llu",
            m_nEndPESPktTS);
        ullOffset = ullOffset - 1;
      }
      //Make sure we are nor reading beyond file size
      if(ullAbsOffset >= m_nFileSize)
      {
        retError = MP2STREAM_EOF;
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
            "EOF detected....");
        break;
      }
    }
  }

  if(ucTempBuf!=NULL)
  {
    MM_Free(ucTempBuf);
    ucTempBuf = NULL;
  }
  return;
}
/*! ======================================================================
@brief  Get Available TimeStamp from the first PES packet in FWD direction

@detail    Starts parsing from the position specified for a PES packet with
           non-zero PTS value
@param[in] trackId Identifies elementary stream to demultiplex from
                   current pes packet
@param[in] ullStartPos Absolute offset from where to start looking for
                       PES packet
@param[out] pullStartOffset Start offset of PES packet
@param[out] pullEndOffset   End offset of PES packet
@param[out] *pullTimeStamp TS from PES packet

@return    Void.
@note      None.
========================================================================== */
void MP2StreamParser::GetPTSFromNextPES(uint32 ulTrackId,
    uint64 ullStartPos,uint64 *pullStartOffset, uint64 *pullEndOffset,
    uint64 *pullTimeStamp)
{
   MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
      "Shantanu GetPTSFromLastPES trackId=%lu ",ulTrackId);
  uint64 ullAbsOffset=0;
  uint64 ullOffset=0;
  uint32 ulTempTrackid=0xFFFFFFFF;
  uint8 *pucTempBuf=(uint8*)MM_Malloc(MPEG2_FILE_READ_CHUNK_SIZE);
  int32* pslBytesRead=NULL;
  if(pucTempBuf)
  {
    uint32 ulBytesRead = readMpeg2StreamData( ullStartPos,
                                              MPEG2_FILE_READ_CHUNK_SIZE,
                                              pucTempBuf,
                                              MPEG2_FILE_READ_CHUNK_SIZE,
                                              m_pUserData);
    if (!ulBytesRead)
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
        "GetPTSFromLastPES File Read Failure ullAvailOffset");
    }
    else
    {
      ullOffset = ulBytesRead;
    }
    while(ullAbsOffset < ullOffset)
    {
      uint32 val = 0;
      while( (!isPESPacket(&pucTempBuf[ullAbsOffset],&val))  &&
             (ullAbsOffset<ullOffset))
      {
        if((pucTempBuf[ullAbsOffset] == 0x47)&& (!m_bProgramStream))
        {
          // Check for Payload Unit Start Indicator
          if(pucTempBuf[ullAbsOffset+1] & 0x40)
          {
            ulTempTrackid =  (pucTempBuf[ullAbsOffset+1] & 0x1F)<< 8;
            ulTempTrackid |=  pucTempBuf[ullAbsOffset+2];
          }
        }
        ullAbsOffset++;
      }
      if(ullAbsOffset < ullOffset)
      {
        uint64 ullTempOffset = ullAbsOffset+ullStartPos;
        parsePESPacket(ullTempOffset,val,ulTrackId,
            pucTempBuf,MPEG2_FILE_READ_CHUNK_SIZE,pslBytesRead);
        if(m_bProgramStream)
        {
          ulTempTrackid = (uint32)m_currPESPkt.trackid;
        }
        MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_MEDIUM,
            "m_currPESPkt.pts=%f ulTempTrackid=%lu",
            m_currPESPkt.pts, ulTempTrackid);
        if ( (0!= m_currPESPkt.pts) &&
             (ulTrackId == ulTempTrackid))
        {
          // Found the last PES packet with a TS
          if(pullTimeStamp)
          {
            *pullTimeStamp = uint64(m_currPESPkt.pts);
          }
          if(pullStartOffset)
          {
            *pullStartOffset = m_currPESPkt.noffset;
          }
          if(pullEndOffset)
          {
            *pullEndOffset = m_currPESPkt.noffset+m_currPESPkt.packet_length;
          }
          MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_MEDIUM,
              "GetPTSFromLastPES m_currPESPkt.pts=%f m_currPESPkt.nOffset=%llu",
              m_currPESPkt.pts, m_currPESPkt.noffset);
          break;
        }
        else
        {
          ullAbsOffset++;
        }
      }
      else
      {
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
            "Exhausted all bytes without finding PES for track!");
        GetPTSFromNextPES(ulTrackId,
            ullStartPos+MPEG2_FILE_READ_CHUNK_SIZE,pullStartOffset,
            pullEndOffset, pullTimeStamp);
      }
    }
  }
  else
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                "Malloc for temp buffer failed");
  }
}

/*! ======================================================================
@brief  Get Available TimeStamp from the last PES packet in a TS stream

@detail    Starts parsing from the end of file for a PES packet with
           non-zero PTS value
@param[in] trackId Identifies elementary stream to demultiplex from
                   current pes packet
@param[in] ullAvailOffset Bytes of data available to parse
@param[out] *ullTimeStamp TS from Last PES packet

@return    Void.
@note      None.
========================================================================== */
MP2StreamStatus MP2StreamParser::GetPTSFromLastPES(uint32 ultrackId,
    uint64 ullAvailOffset, uint64 *pullTimeStamp, uint64* /*pullEndOffset*/)
{
  MP2StreamStatus eRetError = MP2STREAM_FAIL;
  MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_MEDIUM,
      "GetPTSFromLastPES trackId=%lu ullAvailOffset=%llu",
      ultrackId, ullAvailOffset);
  uint64 ullOffset = 0;
  uint32 valCode =0;
  int32* psiBytesRead=NULL;
  uint32 ulTempTrackid=0xFFFFFFFF;
  uint8 *pucTempBuf=(uint8*)MM_Malloc(MPEG2_FILE_READ_CHUNK_SIZE);
  if(pucTempBuf)
  {
    bool bFound=false;
    if(pullTimeStamp)
    *pullTimeStamp = 0;

    // We will start scanning for PTS value from
    // the end of the file and break whenever we
    // find the last PTS in the file
    uint64 ullAbsOffset = ullAvailOffset;
    while(ullAbsOffset > 0)
    {
      if((ullOffset == 0) || (ullOffset >= MPEG2_FILE_READ_CHUNK_SIZE))
      {
        if(ullAbsOffset > MPEG2_FILE_READ_CHUNK_SIZE)
        {
          ullAbsOffset = ullAbsOffset - MPEG2_FILE_READ_CHUNK_SIZE;
        }
        else
        {
          ullAbsOffset = 0;
        }
        uint32 ulBytesRead = readMpeg2StreamData( ullAbsOffset,
                                                  MPEG2_FILE_READ_CHUNK_SIZE,
                                                  pucTempBuf,
                                                  m_nDataBufferSize,
                                                  m_pUserData);
        if (!ulBytesRead)
        {
          MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
              "GetPTSFromLastPES File Read Failure ullAvailOffset=%llu",
              ullAvailOffset);
          break;
        }
        else
        {
          ullOffset = ulBytesRead;
        }
      }
      while( (!isPESPacket(&pucTempBuf[ullOffset-1],&valCode)) &&
             (ullOffset > 0))
      {
        ullOffset--;
      }
      if(!m_bProgramStream)
      {
        // Get PID for Transport stream
        uint64 ullTempOffset=ullOffset;
        while ((pucTempBuf[ullTempOffset] != 0x47) &&
               (ullTempOffset))
        {
          ullTempOffset--;
        }
        // In TS packet header 10th bit is
        // payload_unit_start_indicator
        if((pucTempBuf[ullTempOffset+1] & 0x40) &&
           (0x47 == pucTempBuf[ullTempOffset+1]) )
        {
          // TS packet PID = 13 bits from bit 12 to bit 24
          ulTempTrackid =  (pucTempBuf[ullTempOffset+1] & 0x1F)<< 8;
          ulTempTrackid |=  pucTempBuf[ullTempOffset+2];
        }
      }
      if(ullOffset!=0)
      {
        uint64 ullTempOffset = ullAbsOffset+ullOffset-1;
        eRetError = parsePESPacket(ullTempOffset,valCode,ultrackId,
            pucTempBuf,MPEG2_FILE_READ_CHUNK_SIZE,psiBytesRead);
        if(m_bProgramStream)
        {
          // Get Track ID for Program stream
          ulTempTrackid = m_currPESPkt.trackid;
        }

        if ( (0!= m_currPESPkt.pts) &&
            (ultrackId == ulTempTrackid))
        {
          // Found the last PES packet with a TS
          if(pullTimeStamp)
          {
            *pullTimeStamp = uint64(m_currPESPkt.pts);
          }
          MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_MEDIUM,
              "GetPTSFromLastPES m_currPESPkt.pts=%f m_currPESPkt.nOffset=%llu",
              m_currPESPkt.pts, m_currPESPkt.noffset);
          break;
        }
        else
        {
          // NO TS in this PES packet, go back to previous PES packet
          ullOffset = ullOffset - 1;
        }
      }
    }
    if(pucTempBuf!=NULL)
    {
      MM_Free(pucTempBuf);
      pucTempBuf = NULL;
    }
  }
  return eRetError;
}
/*! ======================================================================
@brief  Parses PES packet encountered in TS or PS

@detail    Starts parsing PES packet encountered in TS or PS

@param[in] N/A

@return    MP2STREAM_SUCCESS is parsing is successful otherwise returns appropriate error.
@note      None.
========================================================================== */
MP2StreamStatus MP2StreamParser::parsePESPacket(uint64& nOffset,uint32 valCode,
                                                uint32 trackId,uint8* dataBuffer,
                                                uint32 nMaxBufSize, int32* nBytesRead)
{
  MP2StreamStatus retError = MP2STREAM_DEFAULT_ERROR;
  memset(&m_currPESPkt,0,sizeof(PESPacket));
  int localoffset = 0;
  bool bOk = true;
#ifdef MPEG2_PARSER_DEBUG
  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW,"parsePESPacket nOffset %llu",nOffset);
#endif

  if(!readMpeg2StreamData (nOffset, PES_PKT_START_CODE_STREAM_ID_SIZE,
                           m_pDataBuffer, m_nDataBufferSize,
                           m_pUserData) )
  {
    retError = m_eParserState;
    bOk = false;
  }

  if(bOk)
  {
    m_currPESPkt.noffset = nOffset;
    //Update the overall offset as we have read in PES_PKT_START_CODE_STREAM_ID_SIZE bytes
    nOffset += PES_PKT_START_CODE_STREAM_ID_SIZE;

    m_currPESPkt.start_code_prefix = PES_PKT_START_CODE;
    //PES_PKT_START_CODE consists of 24 bits, 3 bytes, so update localoffset.
    localoffset += 3;

    //Next byte is stream-id, which is passed in, so update localoffset.
    m_currPESPkt.trackid = (uint8)valCode;
    localoffset++;

    //Next 2 bytes give pes packet length
    m_currPESPkt.packet_length = ((uint32)(m_pDataBuffer[localoffset])<<8) | (uint32)m_pDataBuffer[localoffset+1];
  #ifdef MPEG2_PARSER_DEBUG
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,"parsePESPacket Length %lu",m_currPESPkt.packet_length);
  #endif
    localoffset = (int)(localoffset + sizeof(uint16));

    switch(valCode)
    {
      case PROG_STREAM_MAP_ID:
      {
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,"parsePESPacket encountered PSM packet");
        if (m_sContext.pProgramStreamMap)
        {
          nOffset += m_currPESPkt.packet_length;
        }
        else
        {
          retError = parseProgStreamMap(nOffset);
        }
        break;
      }
      case PADDING_STREAM_ID:
      case ECM_STREAM_ID:
      case EMM_STREAM_ID:
      case PROG_STREAM_DIRECTORY_ID:
      case DSMCC_STREAM_ID:
      case H222_TYPE_E_STREAM_ID:
      {
        retError = MP2STREAM_SUCCESS;
        nOffset += m_currPESPkt.packet_length;
        if( m_eParserState == MP2STREAM_SEEKING)
        {
          retError = MP2STREAM_SKIP_PES_PKT;
        }
        break;
      }
      case PRIVATE_STREAM2_ID:
      {
        //Read in next byte to get substream-id
        if(!readMpeg2StreamData ( nOffset, (uint32)sizeof(PCI_PKT_SUBSTREAM_ID),
                                  m_pDataBuffer, m_nDataBufferSize,
                                  m_pUserData) )
        {
          retError = m_eParserState;
        }
        if(PCI_PKT_SUBSTREAM_ID == m_pDataBuffer[0])
        {
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,"parsePESPacket encountered PCI packet");
          retError = parsePCIPacket(nOffset+sizeof(PCI_PKT_SUBSTREAM_ID),m_currPESPkt.packet_length);
        }
        else if(DSI_PKT_SUBSTREAM_ID == m_pDataBuffer[0])
        {
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,"parsePESPacket encountered DSI packet");
          retError = parseDSIPacket(nOffset+sizeof(PCI_PKT_SUBSTREAM_ID),m_currPESPkt.packet_length);
        }
        else
        {
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,"parsePESPacket encountered non PCI/DSI packet");
          if(!m_bProgramStream)
          {
            retError = MP2STREAM_SUCCESS;
            nOffset = m_currPESPkt.noffset + TS_PKT_SIZE ;
          }
        }
        if(m_bProgramStream)
        {
          nOffset += m_currPESPkt.packet_length;
        }
        break;
      }
      default:
      {
        if( m_eParserState == MP2STREAM_SEEKING)
        {
          nOffset += m_currPESPkt.packet_length;
          retError = MP2STREAM_SKIP_PES_PKT;
          break;
        }
         if( ((valCode >= AUDIO_STREAM_ID_START) && (valCode <= AUDIO_STREAM_ID_END)) ||
             ((valCode >= RES_DATA_STREAM_START_ID) && (valCode <= RES_DATA_STREAM_END_ID)) ||
             (valCode == PRIVATE_STREAM1_ID) )
         {
           #ifdef MPEG2_PARSER_DEBUG
             MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,"AUDIO STREAM ID %d",m_currPESPkt.trackid);
           #endif
           if(!nBytesRead)
           {
             int32 nRead = 0;
             nBytesRead = &nRead;
           }
           retError = parseElementaryStream(nOffset,trackId,TRACK_TYPE_AUDIO,dataBuffer,nMaxBufSize,nBytesRead);
           m_sContext.ulDataRead = *nBytesRead;
           if(retError != MP2STREAM_SUCCESS)
           {
             return retError;
           }
           //! Parse audio metadata only if PTS is valid. This is done to avoid
           //! using of partial PES Packets to update track properties.
           if((m_sContext.bInitialParsingPending) &&
              (m_currPESPkt.pts_dts_flag))
           {
             //! Update Track base time if it is not updated already
             if(!m_sContext.bAudioRefTS)
             {
               m_sContext.bAudioRefTS     = true;
               m_sContext.sllAudioRefTime = (int64)m_currPESPkt.pts;
               m_nFirstAudioPTS = m_currPESPkt.pts;
             }
             (void)parseAudioMetaData(&m_sContext);
           }
         }
         else if( (valCode >= VIDEO_STREAM_ID_START) && (valCode <= VIDEO_STREAM_ID_END))
         {
           #ifdef MPEG2_PARSER_DEBUG
             MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,"VIDEO STREAM ID %d",m_currPESPkt.trackid);
           #endif
           if(!nBytesRead)
           {
             int32 nRead = 0;
             nBytesRead = &nRead;
           }
           retError = parseElementaryStream(nOffset,trackId,TRACK_TYPE_VIDEO,dataBuffer,nMaxBufSize,nBytesRead);
           m_sContext.ulDataRead = *nBytesRead;
           if(retError != MP2STREAM_SUCCESS)
           {
             return retError;
           }
           //! Parse audio metadata only if PTS is valid. This is done to avoid
           //! using of partial PES Packets to update track properties.
           if((m_sContext.bInitialParsingPending) &&
              (m_currPESPkt.pts_dts_flag))
           {
             //! Update Track base time if it is not updated already
             if(!m_sContext.bVideoRefTS)
             {
               m_sContext.bVideoRefTS     = true;
               m_sContext.sllVideoRefTime = (int64)m_currPESPkt.pts;
             }
             (void)parseVideoMetaData(&m_sContext);
           }
         }
         else
         {
           if(m_bProgramStream)
             nOffset += m_currPESPkt.packet_length;
           else
             retError = MP2STREAM_SUCCESS;
         }
        break;
      }
    }
  }
  return retError;
}
/*! ======================================================================
@brief  Parses PCI packet encountered in TS or PS

@detail    Starts parsing PCI packet encountered in TS or PS

@param[in] N/A

@return    MP2STREAM_SUCCESS is parsing is successful otherwise returns appropriate error.
@note      None.
========================================================================== */
MP2StreamStatus  MP2StreamParser::parsePCIPacket(uint64 nOffset,uint32 /*nLength*/)
{
  MP2StreamStatus retError = MP2STREAM_DEFAULT_ERROR;
  pci_pkt* tmpPciPkt = m_pCurrVOBUPCIPkt;

  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,"parsePCIPacket nOffset %llu",nOffset);
  if(!m_pFirstVOBUPCIPkt)
  {
    m_pFirstVOBUPCIPkt = (pci_pkt*)MM_Malloc(sizeof(pci_pkt));
    tmpPciPkt = m_pFirstVOBUPCIPkt;
  }
  else if(!m_pCurrVOBUPCIPkt)
  {
    m_pCurrVOBUPCIPkt = (pci_pkt*)MM_Malloc(sizeof(pci_pkt));
    tmpPciPkt = m_pCurrVOBUPCIPkt;
  }
  if(tmpPciPkt)
  {
    memset(tmpPciPkt,0,sizeof(pci_pkt));
    if(!readMpeg2StreamData ( nOffset, PCI_PKT_INFO_BYTES,
                              m_pDataBuffer, m_nDataBufferSize,
                              m_pUserData) )
    {
      retError = m_eParserState;
    }
    else
    {
      tmpPciPkt->noffset = m_currPESPkt.noffset;
      int readindex = 0;
      nOffset += PCI_PKT_INFO_BYTES;

      //Read block number
      tmpPciPkt->blockno = getBytesValue((int)4,m_pDataBuffer+readindex);
      readindex += 4;

      //Read APS and skip 2 reserved bytes
      tmpPciPkt->flags_aps = (uint16)getBytesValue(2,m_pDataBuffer+readindex);
      readindex += 2;

      //read bit-mask for prohibited user options
      tmpPciPkt->bitmask_puo = (uint16)getBytesValue(4,m_pDataBuffer+readindex);
      readindex += 4;

      //read in start and end time
      tmpPciPkt->start_pts = getBytesValue(4,m_pDataBuffer+readindex);
      tmpPciPkt->start_pts /= 90;
      readindex += 4;

      tmpPciPkt->end_pts = getBytesValue(4,m_pDataBuffer+readindex);
      tmpPciPkt->end_pts /= 90;
      readindex += 4;

      //skip next 8 bytes to get international standard recording code
      readindex += (2*4);

      memcpy(&tmpPciPkt->recording_code,m_pDataBuffer+readindex,(sizeof(uint8)*32));
      MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_HIGH,"parsePCIPacket current VOBU:start PTS %lu end PTS %lu",tmpPciPkt->start_pts,tmpPciPkt->end_pts);
      retError = MP2STREAM_SUCCESS;
      if(!m_pCurrVOBUPCIPkt)
      {
        m_pCurrVOBUPCIPkt = (pci_pkt*)MM_Malloc(sizeof(pci_pkt));
        if(m_pCurrVOBUPCIPkt != NULL)
        {
          memset(m_pCurrVOBUPCIPkt,0,sizeof(pci_pkt));
          memcpy(m_pCurrVOBUPCIPkt,m_pFirstVOBUPCIPkt,sizeof(pci_pkt));
        }
        else
        {
          retError = MP2STREAM_OUT_OF_MEMORY;
        }
      }
    }
  }
  else
  {
    retError = MP2STREAM_OUT_OF_MEMORY;
  }
  return retError;
}
/*! ======================================================================
@brief  Parses DSI packet encountered in TS or PS

@detail    Starts parsing DSI packet encountered in TS or PS

@param[in] N/A

@return    MP2STREAM_SUCCESS is parsing is successful otherwise returns appropriate error.
@note      None.
========================================================================== */
MP2StreamStatus  MP2StreamParser::parseDSIPacket(uint64 nOffset,uint32 nLength)
{
  MP2StreamStatus retError = MP2STREAM_DEFAULT_ERROR;
  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,"parseDSIPacket nOffset %llu",nOffset);
  dsi_pkt* tmpDsiPkt = m_pCurrVOBUDSIPkt;
  if(!m_pFirstVOBUDSIPkt)
  {
    m_pFirstVOBUDSIPkt = (dsi_pkt*)MM_Malloc(sizeof(dsi_pkt));
    tmpDsiPkt = m_pFirstVOBUDSIPkt;
  }
  else if(!m_pCurrVOBUDSIPkt)
  {
    m_pCurrVOBUDSIPkt = (dsi_pkt*)MM_Malloc(sizeof(dsi_pkt));
    tmpDsiPkt = m_pCurrVOBUDSIPkt;
  }
  if(tmpDsiPkt)
  {
    memset(tmpDsiPkt,0,sizeof(dsi_pkt));
    tmpDsiPkt->noffset = m_currPackHeader.noffset;
    uint32 nBytesToRead = nLength;
    if(nLength < DSI_PKT_INFO_BYTES)
    {
      nBytesToRead = DSI_PKT_INFO_BYTES;
    }
    if(!readMpeg2StreamData ( nOffset, nBytesToRead,
                              m_pDataBuffer, m_nDataBufferSize,
                              m_pUserData) )
    {
      retError = m_eParserState;
    }
    else
    {
      //Skip SCR bytes and start parsing
      int readindex = 4;

      //Update the offset with total number of bytes read
      nOffset += DSI_PKT_INFO_BYTES;

      //Read block number
      tmpDsiPkt->blockno = getBytesValue(4,m_pDataBuffer+readindex);
      readindex += 4;

      tmpDsiPkt->vobu_ea = getBytesValue(4,m_pDataBuffer+readindex);
      readindex += 4;

      tmpDsiPkt->first_ref_frame_end_block = getBytesValue(4,m_pDataBuffer+readindex);
      readindex += 4;

      tmpDsiPkt->second_ref_frame_end_block = getBytesValue(4,m_pDataBuffer+readindex);
      readindex += 4;

      tmpDsiPkt->third_ref_frame_end_block = getBytesValue(4,m_pDataBuffer+readindex);
      readindex += 4;

      tmpDsiPkt->vobu_vob_idn = (uint16)getBytesValue(2,m_pDataBuffer+readindex);
      readindex += 2;

      //Go to start and end PTM offset pointer
      readindex = DSI_PKT_START_END_PTM_OFFSET;

      tmpDsiPkt->vob_v_s_ptm = getBytesValue(4,m_pDataBuffer+readindex);
      tmpDsiPkt->vob_v_s_ptm /= 90;
      m_nClipStartTime = tmpDsiPkt->vob_v_s_ptm;
      readindex += 4;

      tmpDsiPkt->vob_v_e_ptm = getBytesValue(4,m_pDataBuffer+readindex);
      tmpDsiPkt->vob_v_e_ptm /= 90;

      if (tmpDsiPkt->vob_v_e_ptm < m_nEndPESPktTS)
      {
        m_nClipDuration = m_nEndPESPktTS;
      }
      else if (tmpDsiPkt->vob_v_e_ptm >= m_nClipDuration)
      {
        m_nClipDuration = tmpDsiPkt->vob_v_e_ptm;
      }
      readindex += 4;

      //Go to next video vobu offset pointer
      readindex = NEXT_VOBU_OFFSET_INDEX;

      tmpDsiPkt->next_vobu_offset = getBytesValue(4,m_pDataBuffer+readindex);
      if( (tmpDsiPkt->next_vobu_offset & 0x80000000) &&
          (NO_VIDEO_VOBU_CODE != tmpDsiPkt->next_vobu_offset) )
      {
        tmpDsiPkt->next_vobu_offset &= 0x0EFFFFFF;
        tmpDsiPkt->end_offset_curr_vobu += tmpDsiPkt->next_vobu_offset * MAX_PS_PACK_SIZE;
        tmpDsiPkt->next_vobu_offset_valid = true;
      }
      readindex += 4;

      //Go to previous video vobu offset pointer
      readindex = PRV_VOBU_OFFSET_INDEX;
      tmpDsiPkt->prv_vobu_offset = getBytesValue(4,m_pDataBuffer+readindex);
      if( (tmpDsiPkt->prv_vobu_offset & 0x80000000) &&
          (NO_VIDEO_VOBU_CODE != tmpDsiPkt->prv_vobu_offset) )
      {
        tmpDsiPkt->prv_vobu_offset_valid = true;
        tmpDsiPkt->prv_vobu_offset &= 0x0EFFFFFF;
      }
      readindex += 4;
#ifdef MPEG2_PARSER_DEBUG
      MM_MSG_PRIO3(MM_FILE_OPS, MM_PRIO_HIGH,"parseDSIPacket DSI_GI VOB:Start V PTS %lu End V PTS %lu tmpDsiPkt->end_offset_curr_vobu %llu",tmpDsiPkt->vob_v_s_ptm,tmpDsiPkt->vob_v_e_ptm,tmpDsiPkt->end_offset_curr_vobu);
      MM_MSG_PRIO5(MM_FILE_OPS, MM_PRIO_HIGH,
        "parseDSIPacket current VOBU# %d EndAddr %lu #1Ref Frame %lu #2Ref Frame %lu #3Ref Frame %lu",
                      tmpDsiPkt->vobu_vob_idn,tmpDsiPkt->vobu_ea,
                      tmpDsiPkt->first_ref_frame_end_block,
                      tmpDsiPkt->second_ref_frame_end_block,
                      tmpDsiPkt->third_ref_frame_end_block);
      MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_HIGH,"parseDSIPacket:Next/Prv VOBU valid %d/%d",tmpDsiPkt->next_vobu_offset_valid,tmpDsiPkt->prv_vobu_offset_valid);
      MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_HIGH,"parseDSIPacket:Offset to Next/Prv VOBU with Video: %llu, %llu",tmpDsiPkt->next_vobu_offset,tmpDsiPkt->prv_vobu_offset);
#endif
      retError = MP2STREAM_SUCCESS;
      if(!m_pCurrVOBUDSIPkt)
      {
        m_pCurrVOBUDSIPkt = (dsi_pkt*)MM_Malloc(sizeof(dsi_pkt));
        if(m_pCurrVOBUDSIPkt != NULL)
        {
          memset(m_pCurrVOBUDSIPkt,0,sizeof(dsi_pkt));
          memcpy(m_pCurrVOBUDSIPkt,m_pFirstVOBUDSIPkt,sizeof(dsi_pkt));
        }
        else
        {
          retError = MP2STREAM_OUT_OF_MEMORY;
        }
      }
    }
  }
  else
  {
    retError = MP2STREAM_OUT_OF_MEMORY;
  }
  return retError;
}
/*! ======================================================================
@brief  Parses elementary stream from current PES packet.

@detail    Demultiplexes elementary stream identified by 'id' from current PES packet.

@param[in/out] nOffset Points to the offset in current PES packet.
@param[in] id          Identifies elementary stream to demultiplex from current pes packet

@return    MP2STREAM_SUCCESS is parsing is successful otherwise returns appropriate error.
@note      None.
========================================================================== */
MP2StreamStatus MP2StreamParser::parseElementaryStream(uint64& nOffset,
                                                       uint32 id,
                                                       track_type StrmType,
                                                       uint8* dataBuffer,
                                                       uint32 nMaxBufSize,
                                                       int32* pnBytesRead)
{
  MP2StreamStatus retError = MP2STREAM_DEFAULT_ERROR;
  bool bContinue = true;
  int localoffset = 0;
  uint16 total_variable_bytes_consumed = 0;
  uint8 nBytesConsumedInPESExtensionHdr = 0;

  MP2TransportStreamPkt* pTSPkt  = &m_sContext.sTSPacket;

#ifdef MPEG2_PARSER_DEBUG
  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW,"parseElementaryStream nOffset %llu",nOffset);
#endif

  //! Reset PES Private data flag. If PES Packet is encrypted, then this flag
  //! will be set to true.
  m_currPESPkt.pes_extn_hdr.pes_extn_pvt_data_flag = false;

  if(m_sContext.bProgramStream && m_sContext.bMPEG1Video)
  {
    if(!readMpeg2StreamData ( nOffset, 8,
                              m_pDataBuffer, m_nDataBufferSize,
                              m_pUserData) )
    {
      retError = m_eParserState;
      bContinue = false;
    }
    else
    {
      if((m_pDataBuffer[localoffset] & 0xc0) == 0x40)
      {
        //STD buffer size field present, not used, skipping
        nOffset += PES_PKT_STD_BUFFER_SIZE;
        total_variable_bytes_consumed = (uint16)(total_variable_bytes_consumed+
                                                 PES_PKT_STD_BUFFER_SIZE);
      }
      if( (m_pDataBuffer[localoffset] & 0xF0) == 0x20)
      {
        //PTS present
        m_currPESPkt.pts_dts_flag = 2;
        uint8 firstpart =  (uint8)((m_pDataBuffer[localoffset++] & 0x0E) << 4);
        uint16 secondpart = (uint16)((m_pDataBuffer[localoffset] << 8) |
                                     m_pDataBuffer[localoffset+1]);
        secondpart = (secondpart & 0xFFFE)>>1;
        localoffset+=2;
        uint16 thirdpart = (uint16)((m_pDataBuffer[localoffset] << 8) |
                                    m_pDataBuffer[localoffset+1]);
        thirdpart = (thirdpart & 0xFFFE)>>1;
        localoffset+=2;
        m_currPESPkt.pts = (double)make33BitValue(firstpart,secondpart,thirdpart);
        m_currPESPkt.pts = (double)(m_currPESPkt.pts)/90.0;//90 KHz clock

        nOffset += PES_PKT_PTS_BYTES;
        total_variable_bytes_consumed = (uint16)(total_variable_bytes_consumed +
                                                 PES_PKT_PTS_BYTES);
      }
      else if( (m_pDataBuffer[localoffset] & 0xF0) == 0x30)
      {
        //PTS & DTS present
        m_currPESPkt.pts_dts_flag = 3;
        uint8 firstpart =  (uint8)((m_pDataBuffer[localoffset++] & 0x0E) << 4);
        uint16 secondpart = (uint16)((m_pDataBuffer[localoffset] << 8) |
                                     m_pDataBuffer[localoffset+1]);
        secondpart = (secondpart & 0xFFFE)>>1;
        localoffset+=2;
        uint16 thirdpart = (uint16)((m_pDataBuffer[localoffset] << 8) |
                                    m_pDataBuffer[localoffset+1]);
        thirdpart = (thirdpart & 0xFFFE)>>1;
        localoffset+=2;
        m_currPESPkt.pts = (double)make33BitValue(firstpart,secondpart,thirdpart);
        m_currPESPkt.pts = (double)(m_currPESPkt.pts)/90.0;//90 KHz clock

        //Not parsing DTS since it is not currently used.
        nOffset += PES_PKT_PTS_DTS_BYTES;
        total_variable_bytes_consumed = (uint16)(total_variable_bytes_consumed +
                                                 PES_PKT_PTS_DTS_BYTES);
      }
      int32 pes_pkt_data_bytes = m_currPESPkt.packet_length - total_variable_bytes_consumed;
      if(dataBuffer)
      {
        if( (int32)nMaxBufSize >= pes_pkt_data_bytes)
        {
          if(!readMpeg2StreamData ( nOffset, pes_pkt_data_bytes,
                                    dataBuffer, nMaxBufSize,
                                    m_pUserData) )
          {
            retError = m_eParserState;
            return retError;
          }
        }
        else
        {
          return MP2STREAM_OUT_OF_MEMORY;
        }
      }
      else
      {
        if(!readMpeg2StreamData (nOffset, pes_pkt_data_bytes,
                                 m_pDataBuffer, m_nDataBufferSize,
                                 m_pUserData) )
        {
          retError = m_eParserState;
          return retError;
        }
      }
      if(pnBytesRead)
      {
        *pnBytesRead = pes_pkt_data_bytes;
      }
      nOffset+= pes_pkt_data_bytes;
      retError = MP2STREAM_SUCCESS;
    }
  }
  else
  {

    //Read PES_PKT_FIXED_HDR_BYTES from current offset
    if(!readMpeg2StreamData (nOffset, PES_PKT_FIXED_HDR_BYTES,
                             m_pDataBuffer, m_nDataBufferSize,
                             m_pUserData) )
    {
      retError = m_eParserState;
      bContinue = false;
    }
    else
    {
      nOffset += PES_PKT_FIXED_HDR_BYTES;
      /* TS Packet PID may get updated dynamically, retain Video/Audio PID
         values as part of PES Packet structure. This PID is useful especially
         if content is encrypted. */
      m_currPESPkt.tsPID = pTSPkt->PID;
      //Parse the bytes read to proceed further
      m_currPESPkt.scrambling_control = (uint8)((m_pDataBuffer[localoffset] & 0x30)>>4);
      m_currPESPkt.pes_priority = (uint8)((m_pDataBuffer[localoffset] & 0x08)>>3);
      m_currPESPkt.data_align_indicator = (uint8)((m_pDataBuffer[localoffset] & 0x04)>>2);
      m_currPESPkt.copyright = (uint8)((m_pDataBuffer[localoffset] & 0x02)>>1);
      m_currPESPkt.original_copy = m_pDataBuffer[localoffset] & 0x01;
      localoffset++;

      m_currPESPkt.pts_dts_flag = (m_pDataBuffer[localoffset] & 0xD0)>>6;
      m_currPESPkt.escr_flag = (uint8)((m_pDataBuffer[localoffset] & 0x20)>>5);
      m_currPESPkt.es_rate_flag = (uint8)((m_pDataBuffer[localoffset] & 0x10)>>4);
      m_currPESPkt.dsm_trick_mode_flag = (uint8)((m_pDataBuffer[localoffset] & 0x08)>>3);
      m_currPESPkt.add_copy_info_flag = (uint8)((m_pDataBuffer[localoffset] & 0x04)>>2);
      m_currPESPkt.pes_crc_flag = (uint8)((m_pDataBuffer[localoffset] & 0x02)>>1);
      m_currPESPkt.pes_extn_flag = m_pDataBuffer[localoffset] & 0x01;
      MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_LOW,
        "parseElementaryStream():TS-PID %u pes_extn_flag= %d",
        m_currPESPkt.tsPID, m_currPESPkt.pes_extn_flag);
      localoffset++;
      m_currPESPkt.pes_hdr_data_length = m_pDataBuffer[localoffset++];
      localoffset++;

      //Bytes consumed here onwards as a part of PES packet are variable, so keep counting bytes consumed.
      while( (bContinue) &&
             ((uint32) (localoffset + total_variable_bytes_consumed) <
              TS_PES_PKT_TOTAL_HEADER_SIZE))
      {
        //! Audio and Video PIDs selected for playback
        uint16 usAudioPID = m_sContext.usAudioPIDSelected;
        uint16 usVideoPID = m_sContext.usVideoPIDSelected;
        if( (m_currPESPkt.pts_dts_flag & 0x03) == 0x03)
        {
          localoffset = 0;
          //parse PTS/DTS from current PES packet
          if(!readMpeg2StreamData (nOffset, PES_PKT_PTS_DTS_BYTES,
                                   m_pDataBuffer, m_nDataBufferSize,
                                   m_pUserData) )
          {
            retError = m_eParserState;
            bContinue = false;
            break;
          }
          nOffset += PES_PKT_PTS_DTS_BYTES;
          total_variable_bytes_consumed = (uint16)( total_variable_bytes_consumed +
                                                    PES_PKT_PTS_DTS_BYTES);

          //30--32
          uint8 firstpart =  (uint8)((m_pDataBuffer[localoffset++] & 0x0E) << 4);
          //29--15
          uint16 secondpart = (uint16)((m_pDataBuffer[localoffset] << 8) |
                                       m_pDataBuffer[localoffset+1]);
          secondpart = (secondpart & 0xFFFE)>>1;
          localoffset+=2;
          //0--14
          uint16 thirdpart = (uint16)((m_pDataBuffer[localoffset] << 8) |
                                      m_pDataBuffer[localoffset+1]);
          thirdpart = (thirdpart & 0xFFFE)>>1;
          localoffset+=2;

          m_currPESPkt.pts = (double)make33BitValue(firstpart,secondpart,thirdpart);
          m_currPESPkt.pts = (double)(m_currPESPkt.pts)/90.0;//90 KHz clock

          if((m_sContext.sTSPacket.PID == usAudioPID) ||
             (m_currPESPkt.trackid == usAudioPID))
          {
            /** First Frame TS may be ZERO. m_bpartialPESTS flag will be
             *  set to TRUE, once at least one PES packet is completely
             *  parsed. */
            if((!m_sContext.bAudioRefTS) && (!m_sContext.bGetLastPTS))
            {
              m_nFirstAudioPTS = m_currPESPkt.pts;
              m_sContext.sllAudioRefTime = (int64)m_nFirstAudioPTS;
              m_sContext.bAudioRefTS = true;
            }
            if((m_sContext.bGetLastPTS) && (!m_sContext.ullLastAudioTS))
            {
              m_sContext.ullLastAudioTS = (uint64)m_currPESPkt.pts;
            }
            if(!m_sContext.bInitialParsingPending)//During playback
            {
              /* Correct if discontinuity present */
              if(m_hTSDiscCorrectModeEnum ==
                 FILE_SOURCE_MEDIA_ENABLE_TS_DISCONTINUITY_CORRECTION)
              {
                correctTSDiscontinuity(id);
              }
              if(m_currPESPkt.pts >= m_sContext.sllAudioRefTime)
                m_currPESPkt.pts = m_currPESPkt.pts -
                                  (double)m_sContext.sllAudioRefTime;
              else
                m_currPESPkt.pts = 0;
            }
          }
          else if((m_sContext.sTSPacket.PID == usVideoPID) ||
                  (m_currPESPkt.trackid == usVideoPID))
          {
            /** First Frame TS may be ZERO. m_bpartialPESTS flag will be
             *  set to TRUE, once at least one PES packet is completely
             *  parsed. */
            if((!m_sContext.bVideoRefTS) && (!m_sContext.bGetLastPTS))
            {
              m_sContext.sllVideoRefTime = (int64)m_currPESPkt.pts;
              m_sContext.bVideoRefTS     = true;
            }
            if((m_sContext.bGetLastPTS) && (!m_sContext.ullLastVideoTS))
            {
              m_sContext.ullLastVideoTS = (uint64)m_currPESPkt.pts;
            }
            if(!m_sContext.bInitialParsingPending)//During playback
            {
              /* Correct if discontinuity present */
              if(m_hTSDiscCorrectModeEnum ==
                 FILE_SOURCE_MEDIA_ENABLE_TS_DISCONTINUITY_CORRECTION)
              {
                correctTSDiscontinuity(id);
              }
              if(m_currPESPkt.pts >= m_sContext.sllVideoRefTime)
                m_currPESPkt.pts = m_currPESPkt.pts - (double)m_sContext.sllVideoRefTime;
              else
                m_currPESPkt.pts = 0;
            }
          }

          if(!m_sContext.bInitialParsingPending)
          {
            //We will update our member sample info structure
            if((float)m_currPESPkt.pts != m_sampleInfo.ntime)
            {
              if(m_sampleInfo.ntime)
                m_sampleInfo.delta = (float)fabs((float(m_currPESPkt.pts) -
                                                 float(m_sampleInfo.ntime)));
              m_sampleInfo.ntime = (float)m_currPESPkt.pts;
              m_sampleInfo.noffset = m_currPESPkt.noffset;
            }
          }
          //30--32
          firstpart =  (uint8)((m_pDataBuffer[localoffset++] & 0x0E) << 4);
          //29--15
          secondpart = (uint16)((m_pDataBuffer[localoffset] << 8) |
                                m_pDataBuffer[localoffset+1]);
          secondpart = (secondpart & 0xFFFE)>>1;
          localoffset+=2;
          //0--14
          thirdpart = (uint16)((m_pDataBuffer[localoffset] << 8) |
                               m_pDataBuffer[localoffset+1]);
          thirdpart = (thirdpart & 0xFFFE)>>1;
          localoffset+=2;

          m_currPESPkt.dts = (double)make33BitValue(firstpart,secondpart,thirdpart);
          m_currPESPkt.dts = (double)(m_currPESPkt.dts)/90.0;//90 KHz clock
          MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_MEDIUM,"parseElementaryStream PTS %f DTS %f",m_currPESPkt.pts,m_currPESPkt.dts);
        }
        else if( (m_currPESPkt.pts_dts_flag & 0x02) == 0x02)
        {
          localoffset = 0;
          //parse PTS from current PES packet
          if(!readMpeg2StreamData (nOffset, PES_PKT_PTS_BYTES ,
                                   m_pDataBuffer, m_nDataBufferSize,
                                   m_pUserData) )
          {
            retError = m_eParserState;
            bContinue = false;
            break;
          }
          nOffset += PES_PKT_PTS_BYTES;
          total_variable_bytes_consumed = (uint16)(total_variable_bytes_consumed +
                                                   PES_PKT_PTS_BYTES);

          //30--32
          uint8 firstpart =  (uint8)((m_pDataBuffer[localoffset] & 0x0E) << 4);
          localoffset++;
          //29--15
          uint16 secondpart = (uint16)((m_pDataBuffer[localoffset] << 8) |
                                       m_pDataBuffer[localoffset+1]);
          secondpart = (secondpart & 0xFFFE)>>1;
          localoffset+=2;
          //0--14
          uint16 thirdpart = (uint16)((m_pDataBuffer[localoffset] << 8) |
                                      m_pDataBuffer[localoffset+1]);
          thirdpart = (thirdpart & 0xFFFE)>>1;
          localoffset+=2;

          m_currPESPkt.pts = (double)make33BitValue(firstpart,secondpart,thirdpart);
          m_currPESPkt.pts = (m_currPESPkt.pts)/90.0;//90 KHz clock

          if((m_sContext.sTSPacket.PID == usAudioPID) ||
             (m_currPESPkt.trackid == usAudioPID))
          {
            /** First Frame TS may be ZERO. m_bpartialPESTS flag will be
             *  set to TRUE, once at least one PES packet is completely
             *  parsed. */
            if((!m_sContext.bAudioRefTS) && (!m_sContext.bGetLastPTS))
            {
              m_nFirstAudioPTS = m_currPESPkt.pts;
              m_sContext.sllAudioRefTime = (int64)m_nFirstAudioPTS;
              m_sContext.bAudioRefTS = true;
            }
            if((m_sContext.bGetLastPTS) && (!m_sContext.ullLastAudioTS))
            {
              m_sContext.ullLastAudioTS = (uint64)m_currPESPkt.pts;
            }
            if(!m_sContext.bInitialParsingPending)//During playback
            {
              /* Correct if discontinuity present */
              if(m_hTSDiscCorrectModeEnum ==
                 FILE_SOURCE_MEDIA_ENABLE_TS_DISCONTINUITY_CORRECTION)
              {
                correctTSDiscontinuity(id);
              }
              if(m_currPESPkt.pts >= m_sContext.sllAudioRefTime)
                m_currPESPkt.pts = m_currPESPkt.pts - (double)m_sContext.sllAudioRefTime;
              else
                m_currPESPkt.pts = 0;
            }
          }
          else if((m_sContext.sTSPacket.PID == usVideoPID) ||
                  (m_currPESPkt.trackid == usVideoPID))
          {
            /** First Frame TS may be ZERO. m_bpartialPESTS flag will be
             *  set to TRUE, once at least one PES packet is completely
             *  parsed. */
            if( (!m_sContext.bVideoRefTS) && (!m_sContext.bGetLastPTS))
            {
              m_sContext.sllVideoRefTime = (int64)m_currPESPkt.pts;
              m_sContext.bVideoRefTS     = true;
            }
            if((m_sContext.bGetLastPTS) && (!m_sContext.ullLastVideoTS))
            {
              m_sContext.ullLastVideoTS = (uint64)m_currPESPkt.pts;
            }
            if(!m_sContext.bInitialParsingPending)//During playback
            {
              /* Correct if discontinuity present */
              if(m_hTSDiscCorrectModeEnum ==
                 FILE_SOURCE_MEDIA_ENABLE_TS_DISCONTINUITY_CORRECTION)
              {
                correctTSDiscontinuity(id);
              }
              if(m_currPESPkt.pts >= m_sContext.sllVideoRefTime)
                m_currPESPkt.pts = m_currPESPkt.pts -
                                   (double)m_sContext.sllVideoRefTime;
              else
                m_currPESPkt.pts = 0;
            }
          }

          if(!m_sContext.bInitialParsingPending)
          {
            //We will update our member sample info structure
            if((float)m_currPESPkt.pts != m_sampleInfo.ntime)
            {
              if(m_sampleInfo.ntime)
                m_sampleInfo.delta = (float)fabs((float(m_currPESPkt.pts) -
                                                 float(m_sampleInfo.ntime)));
              m_sampleInfo.ntime = (float)m_currPESPkt.pts;
              m_sampleInfo.noffset = m_currPESPkt.noffset;
            }
          }

    #ifdef MPEG2_PARSER_DEBUG
          MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,"parseElementaryStream PTS %f NO DTS",m_currPESPkt.pts);
    #endif
        }
        if(m_currPESPkt.escr_flag && bContinue)
        {
          //parse escr
          localoffset = 0;
          if(!readMpeg2StreamData (nOffset, PES_PKT_ESCR_BYTES,
                                   m_pDataBuffer, m_nDataBufferSize,
                                   m_pUserData) )
          {
            retError = m_eParserState;
            bContinue = false;
            break;
          }
          nOffset += PES_PKT_ESCR_BYTES;
          total_variable_bytes_consumed = (uint16)(total_variable_bytes_consumed +
                                                   PES_PKT_ESCR_BYTES);
          uint8 firstpart = (uint8)((m_pDataBuffer[localoffset] & 0x38)<<2);//get ms 3 bits

          //get top 2 bits out of middle 15 bits
          uint8 middle15bitspart1 = (uint8)((m_pDataBuffer[localoffset] & 0x03)<<6);
          localoffset++;

          //get remaining 13 bits out of middle 15 bits
          uint16 middle15bitspart2 = (uint16)((m_pDataBuffer[localoffset]<<8) |
                                              (m_pDataBuffer[localoffset+1]& 0xF8));
          localoffset++;
          uint16 secondpart = make15BitValue(middle15bitspart1,middle15bitspart2);

          //get top 2 bits out of lower 15 bits
          uint8 lower15bitspart1 = (uint8)((m_pDataBuffer[localoffset] & 0x03)<<6);
          localoffset++;

          //get remaining 13 bits out of middle 15 bits
          uint16 lower15bitspart2 = (uint16)((m_pDataBuffer[localoffset]<<8) |
                                             (m_pDataBuffer[localoffset+1]& 0xF8));
          localoffset++;
          uint16 thirdpart = make15BitValue(lower15bitspart1,lower15bitspart2);

          //get top 2 bits out of 9 bit escr extension
          uint8 escrextpart1     = (uint8)((m_pDataBuffer[localoffset++] & 0x03)<<6);
          //get lower 7 bits out of 9 bit escr extension
          uint8 escrextpart2     = (m_pDataBuffer[localoffset++] & 0xFE);

          m_currPESPkt.escr_extn = make9BitValue(escrextpart1,escrextpart2);
          m_currPESPkt.escr_base  = (double)make33BitValue(firstpart,secondpart,thirdpart);

          m_currPESPkt.escr_base *= 300;
          m_currPESPkt.escr_val = m_currPESPkt.escr_base + m_currPESPkt.escr_extn;
          m_currPESPkt.escr_val = m_currPESPkt.escr_val /27000.0;    //27 MHz clock
    #ifdef MPEG2_PARSER_DEBUG
          MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,"parseElementaryStream escr_val %f ",m_currPESPkt.escr_val);
    #endif
        }
        if(m_currPESPkt.es_rate_flag && bContinue)
        {
          //parse es_rate
          localoffset = 0;
          if(!readMpeg2StreamData (nOffset, PES_PKT_ES_RATE_BYTES,
                                   m_pDataBuffer, m_nDataBufferSize,
                                   m_pUserData) )
          {
            retError = m_eParserState;
            bContinue = false;
            break;
          }
          nOffset += PES_PKT_ES_RATE_BYTES;
          total_variable_bytes_consumed = (uint16)(total_variable_bytes_consumed +
                                                   PES_PKT_ES_RATE_BYTES);

          //get top 15 bits from rate bound
          uint16 es_rate_part1 = (uint16)((m_pDataBuffer[localoffset] <<8) |
                                          m_pDataBuffer[localoffset+1]);
          localoffset += 2;
          //knock off marker bit and get the 15 bits of interest.
          es_rate_part1 = (uint16)(es_rate_part1 << 1);
          //get ls 7 bits from rate bound
          uint8 es_rate_part2 = (m_pDataBuffer[localoffset] & 0xFE);
          localoffset++;
          m_currPESPkt.es_rate = make22BitValue(es_rate_part1 ,es_rate_part2);
        }
        if(m_currPESPkt.dsm_trick_mode_flag && bContinue)
        {
          //parse trick mode info
          localoffset = 0;
          if(!readMpeg2StreamData (nOffset, PES_PKT_DSM_TRICK_MODE_BYTES,
                                   m_pDataBuffer, m_nDataBufferSize,
                                   m_pUserData) )
          {
            retError = m_eParserState;
            bContinue = false;
            break;
          }
          nOffset += PES_PKT_DSM_TRICK_MODE_BYTES;
          total_variable_bytes_consumed = (uint16)(total_variable_bytes_consumed +
                                                   PES_PKT_DSM_TRICK_MODE_BYTES);
          m_currPESPkt.trick_mode_control = (m_pDataBuffer[localoffset]& 0xE0)>>5;
          switch(m_currPESPkt.trick_mode_control)
          {
            case TRICK_MODE_CONTROL_FAST_FORWARD:
            {
              m_currPESPkt.field_id = (uint8)((m_pDataBuffer[localoffset]& 0x18)>>3);
              m_currPESPkt.intra_slice_refresh = (uint8)((m_pDataBuffer[localoffset]& 0x04)>>2);
              m_currPESPkt.frequency_truncation = (uint8)(m_pDataBuffer[localoffset]& 0x01);
            }
            break;
            case TRICK_MODE_CONTROL_SLOW_MOTION:
            {
              m_currPESPkt.rep_cntrol = m_pDataBuffer[localoffset]& 0x1F;
            }
            break;
            case TRICK_MODE_CONTROL_FREEZE_FRAME:
            {
              m_currPESPkt.field_id = (uint8)((m_pDataBuffer[localoffset]& 0x18)>>3);
            }
            break;
            case TRICK_MODE_CONTROL_FAST_REVERSE:
            {
             m_currPESPkt.field_id = (uint8)((m_pDataBuffer[localoffset]& 0x18)>>3);
             m_currPESPkt.intra_slice_refresh = (uint8)((m_pDataBuffer[localoffset]& 0x04)>>2);
             m_currPESPkt.frequency_truncation = (uint8)(m_pDataBuffer[localoffset]& 0x01);
            }
            break;
            case TRICK_MODE_CONTROL_SLOW_REVERSE:
            {
              m_currPESPkt.rep_cntrol = m_pDataBuffer[localoffset]& 0x1F;
            }
            break;
            default:
              break;
          }
        }
        if(m_currPESPkt.add_copy_info_flag && bContinue)
        {
          //parse additional copy info
          localoffset = 0;
          if(!readMpeg2StreamData (nOffset, PES_PKT_COPY_INFO_BYTES,
                                   m_pDataBuffer, m_nDataBufferSize,
                                   m_pUserData) )
          {
            retError = m_eParserState;
            bContinue = false;
            break;
          }
          nOffset += PES_PKT_COPY_INFO_BYTES;
          total_variable_bytes_consumed = (uint16)(total_variable_bytes_consumed +
                                                   PES_PKT_COPY_INFO_BYTES);
          m_currPESPkt.add_copy = m_pDataBuffer[localoffset]& 0x7F;
        }
        if(m_currPESPkt.pes_crc_flag && bContinue)
        {
          //parse pes crs information
          localoffset = 0;
          if(!readMpeg2StreamData (nOffset, PES_PKT_PES_CRC_BYTES,
                                   m_pDataBuffer, m_nDataBufferSize,
                                   m_pUserData) )
          {
            retError = m_eParserState;
            bContinue = false;
            break;
          }
          nOffset += PES_PKT_PES_CRC_BYTES;
          total_variable_bytes_consumed = (uint16)(total_variable_bytes_consumed +
                                                   PES_PKT_PES_CRC_BYTES);
          m_currPESPkt.prv_pkt_pes_crc = (uint16)((m_pDataBuffer[localoffset] << 8) |
                                                  m_pDataBuffer[localoffset+1]);
        }
        if(m_currPESPkt.pes_extn_flag && bContinue)
        {
          //parse pes extension information
          localoffset = 0;
          if(!readMpeg2StreamData (nOffset, PES_EXTN_FIXED_HDR_BYTES,
                                   m_pDataBuffer, m_nDataBufferSize,
                                   m_pUserData) )
          {
            retError = m_eParserState;
            bContinue = false;
            break;
          }
          else
          {
            nOffset += PES_EXTN_FIXED_HDR_BYTES;
            total_variable_bytes_consumed = (uint16)(total_variable_bytes_consumed +
                                                     PES_EXTN_FIXED_HDR_BYTES);
            nBytesConsumedInPESExtensionHdr = (uint8)(nBytesConsumedInPESExtensionHdr +
                                                      PES_EXTN_FIXED_HDR_BYTES);
            //parse pes extension fixed header

            m_currPESPkt.pes_extn_hdr.pes_extn_pvt_data_flag    = (m_pDataBuffer[localoffset] & 0x80)?1:0;
            m_currPESPkt.pes_extn_hdr.pes_extn_pack_hdr_flag    = (m_pDataBuffer[localoffset] & 0x40)?1:0;
            m_currPESPkt.pes_extn_hdr.pes_extn_pkt_seq_cnt_flag = (m_pDataBuffer[localoffset] & 0x20)?1:0;
            m_currPESPkt.pes_extn_hdr.pes_extn_std_buffer_flag  = (m_pDataBuffer[localoffset] & 0x10)?1:0;
            m_currPESPkt.pes_extn_hdr.pes_extn_flag2            = (m_pDataBuffer[localoffset] & 0x01)?1:0;
            MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_LOW,
              "parseElementaryStream():TS-PID %u pes_extn_pvt_data_flag =%d",
              m_currPESPkt.tsPID,
              m_currPESPkt.pes_extn_hdr.pes_extn_pvt_data_flag );

            if(m_currPESPkt.pes_extn_hdr.pes_extn_pvt_data_flag && bContinue)
            {
              localoffset = 0;
              MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
             "parseElementaryStream():TS-PID %u pes_extn_pvt_data_flag is TRUE",
              m_currPESPkt.tsPID);

              if(!readMpeg2StreamData (nOffset, PES_EXTN_PVT_DATA_BYTES,
                                       m_pDataBuffer, m_nDataBufferSize,
                                       m_pUserData) )
              {
                retError = m_eParserState;
                bContinue = false;
                break;
              }
              else
              {
                nOffset += PES_EXTN_PVT_DATA_BYTES;
                total_variable_bytes_consumed = (uint16)(total_variable_bytes_consumed +
                                                         PES_EXTN_PVT_DATA_BYTES);
                nBytesConsumedInPESExtensionHdr = (uint8)(nBytesConsumedInPESExtensionHdr +
                                                          PES_EXTN_PVT_DATA_BYTES);

                localoffset = 0;
                memcpy(m_currPESPkt.pes_extn_hdr.pes_pvt_data, m_pDataBuffer,
                       PES_EXTN_PVT_DATA_BYTES);
              }
            }
            if(m_currPESPkt.pes_extn_hdr.pes_extn_pack_hdr_flag && bContinue)
            {
              localoffset = 0;
    #ifdef MPEG2_PARSER_DEBUG
              MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"parseElementaryStream pes_extn_pack_hdr_flag is TRUE");
    #endif
              if(!readMpeg2StreamData (nOffset, PES_EXTN_PACK_FIELD_LEN_BYTES,
                                       m_pDataBuffer, m_nDataBufferSize,
                                       m_pUserData) )
              {
                retError = m_eParserState;
                bContinue = false;
                break;
              }
              else
              {
                total_variable_bytes_consumed = (uint16)(total_variable_bytes_consumed +
                                                         PES_EXTN_PACK_FIELD_LEN_BYTES);
                nOffset += PES_EXTN_PACK_FIELD_LEN_BYTES;
                nBytesConsumedInPESExtensionHdr = (uint8)(nBytesConsumedInPESExtensionHdr +
                                                          PES_EXTN_PACK_FIELD_LEN_BYTES);
                localoffset = 0;
                memcpy(&m_currPESPkt.pes_extn_hdr.pack_field_length,m_pDataBuffer,1);
                if(m_bProgramStream)
                retError = parsePackHeader(nOffset);
                if(retError != MP2STREAM_SUCCESS)
                {
                   return retError;
                }
              }
            }
            if(m_currPESPkt.pes_extn_hdr.pes_extn_pkt_seq_cnt_flag && bContinue)
            {
    #ifdef MPEG2_PARSER_DEBUG
              MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,"parseElementaryStream pes_extn_pkt_seq_cnt_flag is TRUE");
    #endif
              if(!readMpeg2StreamData (nOffset, PES_EXTN_PKT_SEQ_COUNTER_BYTES,
                                       m_pDataBuffer, m_nDataBufferSize,
                                       m_pUserData) )
              {
                retError = m_eParserState;
                bContinue = false;
                break;
              }
              else
              {
                localoffset = 0;
                nOffset += PES_EXTN_PKT_SEQ_COUNTER_BYTES;
                total_variable_bytes_consumed = (uint16)
                (total_variable_bytes_consumed + PES_EXTN_PKT_SEQ_COUNTER_BYTES);
                nBytesConsumedInPESExtensionHdr =(uint8)
               (nBytesConsumedInPESExtensionHdr + PES_EXTN_PKT_SEQ_COUNTER_BYTES);
                m_currPESPkt.pes_extn_hdr.prog_seq_cnt = m_pDataBuffer[localoffset] & 0x7F;
                localoffset++;
                m_currPESPkt.pes_extn_hdr.mpeg1_mpeg2_iden = m_pDataBuffer[localoffset]&0x40;
                m_currPESPkt.pes_extn_hdr.original_stuff_length = m_pDataBuffer[localoffset]&0x3F;
                localoffset++;
    #ifdef MPEG2_PARSER_DEBUG
                MM_MSG_PRIO3(MM_FILE_OPS, MM_PRIO_HIGH,
                               "parseElementaryStream prog_seq_cnt %d mpeg1_mpeg2_iden %d original_stuff_length %d",
                               m_currPESPkt.pes_extn_hdr.prog_seq_cnt,
                               m_currPESPkt.pes_extn_hdr.mpeg1_mpeg2_iden,
                               m_currPESPkt.pes_extn_hdr.original_stuff_length);
    #endif
              }
            }
            if(m_currPESPkt.pes_extn_hdr.pes_extn_std_buffer_flag && bContinue)
            {
    #ifdef MPEG2_PARSER_DEBUG
              MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,"parseElementaryStream pes_extn_std_buffer_flag is TRUE");
    #endif
              if(!readMpeg2StreamData (nOffset, PES_EXTN_P_STD_BUFFER_BYTES,
                                       m_pDataBuffer, m_nDataBufferSize,
                                       m_pUserData) )
              {
                retError = m_eParserState;
                bContinue = false;
                break;
              }
              else
              {
                localoffset = 0;
                nOffset += PES_EXTN_P_STD_BUFFER_BYTES;
                total_variable_bytes_consumed = (uint16)
               (total_variable_bytes_consumed + PES_EXTN_P_STD_BUFFER_BYTES);
                nBytesConsumedInPESExtensionHdr = (uint8)
               (nBytesConsumedInPESExtensionHdr + PES_EXTN_P_STD_BUFFER_BYTES);
                m_currPESPkt.pes_extn_hdr.p_std_buffer_scale = m_pDataBuffer[localoffset] & 0x20;
                m_currPESPkt.pes_extn_hdr.p_std_buffer_size  = (uint16)
                              ((m_pDataBuffer[localoffset] & 0x1F)<<8);
                m_currPESPkt.pes_extn_hdr.p_std_buffer_size  = (uint16)
              (m_currPESPkt.pes_extn_hdr.p_std_buffer_size |m_pDataBuffer[localoffset+1]);

                if(!m_currPESPkt.pes_extn_hdr.p_std_buffer_scale)
                {
                  m_currPESPkt.pes_extn_hdr.p_std_buffer_size = (uint16)
                 (m_currPESPkt.pes_extn_hdr.p_std_buffer_size * 128);
                }
                else
                {
                  m_currPESPkt.pes_extn_hdr.p_std_buffer_size = (uint16)
                 (m_currPESPkt.pes_extn_hdr.p_std_buffer_size * 1024);
                }
    #ifdef MPEG2_PARSER_DEBUG
                MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_HIGH,
                               "parseElementaryStream p_std_buffer_scale %d p_std_buffer_size %d",
                               m_currPESPkt.pes_extn_hdr.p_std_buffer_scale, m_currPESPkt.pes_extn_hdr.p_std_buffer_size);
    #endif
              }
            }
            if(m_currPESPkt.pes_extn_hdr.pes_extn_flag2 && bContinue)
            {
#ifdef MPEG2_PARSER_DEBUG
              MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,"parseElementaryStream pes_extn_flag2 is TRUE");
#endif
              if(!readMpeg2StreamData (nOffset, PES_EXTN_FLAG2_BYTES,
                                       m_pDataBuffer, m_nDataBufferSize,
                                       m_pUserData) )
              {
                retError = m_eParserState;
                bContinue = false;
                break;
              }
              else
              {
                localoffset = 0;
                nOffset += PES_EXTN_FLAG2_BYTES;
                total_variable_bytes_consumed =(uint16)
                (total_variable_bytes_consumed + PES_EXTN_FLAG2_BYTES);
                nBytesConsumedInPESExtensionHdr =(uint8)
                (nBytesConsumedInPESExtensionHdr + PES_EXTN_FLAG2_BYTES);
                m_currPESPkt.pes_extn_hdr.pes_extn_field_length = m_pDataBuffer[localoffset]& 0x7F;
              }
            }//if(m_currPESPkt.pes_extn_hdr.pes_extn_flag2 && bContinue)
          }
        }//if(m_currPESPkt.pes_extn_flag && bContinue)

        /*
        * m_currPESPkt.pes_hdr_data_length gives
        * total number of bytes occupied by optional fields and
        * any stuffing bytes contained in this PES pkt header.
        * Check if we have any non zero stuffing bytes to skip.
        */
        if( (m_currPESPkt.pes_hdr_data_length - total_variable_bytes_consumed) > 0)
        {
          /*
          * There are few non zero stuffing bytes.
          * Update variable bytes consumed and nOffset.
          */
          if( (m_bProgramStream) ||
              ( (!m_bProgramStream) &&
                (m_currPESPkt.pes_hdr_data_length < ( (TS_PKT_SIZE - TS_PKT_HDR_BYTES - PES_PKT_START_CODE_STREAM_ID_SIZE) - PES_PKT_FIXED_HDR_BYTES - total_variable_bytes_consumed) )))
          {
            int nPaddingBytes = m_currPESPkt.pes_hdr_data_length - total_variable_bytes_consumed;
            total_variable_bytes_consumed = (uint16)
            (total_variable_bytes_consumed + nPaddingBytes);
            nOffset += nPaddingBytes;
          }
        }
    #ifdef MPEG2_PARSER_DEBUG
        MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_HIGH,
                       "parseElementaryStream m_currPESPkt.pes_hdr_data_length %d total_variable_bytes_consumed %d ",
                       m_currPESPkt.pes_hdr_data_length,total_variable_bytes_consumed);
    #endif

        int32 pes_pkt_data_bytes = 0;
        if(m_bProgramStream)
        {
          pes_pkt_data_bytes = m_currPESPkt.packet_length - PES_PKT_FIXED_HDR_BYTES - total_variable_bytes_consumed;
        }
        else
        {
          if(pTSPkt->adaption_field_control != TS_ADAPTION_FILED_DATA_PRSENT)
          {
            pes_pkt_data_bytes = TS_PES_PKT_TOTAL_HEADER_SIZE - total_variable_bytes_consumed ;
          }
          else
          {
            if( (uint32)(total_variable_bytes_consumed + 1 +
                pTSPkt->adaption_field.adaption_field_length) < TS_PES_PKT_TOTAL_HEADER_SIZE)
            {
            pes_pkt_data_bytes = TS_PES_PKT_TOTAL_HEADER_SIZE - total_variable_bytes_consumed - 1 - pTSPkt->adaption_field.adaption_field_length ;
            total_variable_bytes_consumed = (uint16)(total_variable_bytes_consumed +
                                            (1 + pTSPkt->adaption_field.adaption_field_length));
            }
            else
            {
              bContinue = false;
              retError = MP2STREAM_SUCCESS;
              MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"no pes_pkt_data_bytes here");
            }
          }
        }
    #ifdef MPEG2_PARSER_DEBUG
        MM_MSG_PRIO3(MM_FILE_OPS, MM_PRIO_HIGH,
                        "parseElementaryStream m_currPESPkt.packet_length %lu m_currPESPkt.pes_hdr_data_length %d pes_pkt_data_bytes %ld",
                        m_currPESPkt.packet_length,m_currPESPkt.pes_hdr_data_length,pes_pkt_data_bytes);
    #endif
        if(pes_pkt_data_bytes )
        {
          //read in elementary data stream now
          if(dataBuffer)
          {
            //! Read data only for matching track ID is found
            if (((int32)nMaxBufSize >= pes_pkt_data_bytes) &&
                (m_currPESPkt.trackid == id))
            {
              bool bSkip = false;
              track_type ttype;
              memset(&ttype,0,sizeof(track_type));
              media_codec_type medcodtype;
              memset(&medcodtype,0,sizeof(media_codec_type));
              (void)GetTrackType(id,&ttype,&medcodtype);
              if ((StrmType == TRACK_TYPE_AUDIO) &&
                  (AUDIO_CODEC_AC3  == medcodtype) )
              {
                //Read substream-id
                if(!readMpeg2StreamData (nOffset, 1, //sizeof(uint8)
                                         dataBuffer, nMaxBufSize,
                                         m_pUserData) )
                {
                  retError = m_eParserState;
                  bContinue = false;
                  break;
                }
                if(( (dataBuffer[0] < 0x80) || (dataBuffer[0] > 0x87) ) )
                {
                  bSkip = true;
                }
                else
                {
                  track_type ttype;
                  memset(&ttype,0,sizeof(track_type));
                  media_codec_type medcodtype;
                  memset(&medcodtype,0,sizeof(media_codec_type));
                  //if( (StrmType == AUDIO) && (GetTrackType(id,&ttype,&medcodtype)==MP2STREAM_SUCCESS) )
                  {
                    //if(medcodtype == AUDIO_CODEC_AC3)
                    nOffset += AC3_SUBSTREAM_SYNC_INFO_BYTES;
                    pes_pkt_data_bytes -= AC3_SUBSTREAM_SYNC_INFO_BYTES;
                  }
                }
              }
              if(!readMpeg2StreamData (nOffset, pes_pkt_data_bytes,
                                       dataBuffer, nMaxBufSize,
                                       m_pUserData) )
              {
                retError = m_eParserState;
                bContinue = false;
                break;
              }
              else
              {
                if(pnBytesRead)
                {
                  if( bSkip )
                  {
                    *pnBytesRead = 0;
                  }
                  else
                  {
                    *pnBytesRead = pes_pkt_data_bytes;
                  }
                }
              }
            }
            else
            {
              if(m_currPESPkt.trackid == id)
              {
                MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_HIGH,
                  "PES remaining buf size %lu is less than packet length %lu",
                  nMaxBufSize, pes_pkt_data_bytes);
              }
              if(pnBytesRead)
                *pnBytesRead = 0;
            }
          }
          else if((int32)m_nDataBufferSize >= pes_pkt_data_bytes)
          {
            if(!readMpeg2StreamData (nOffset, pes_pkt_data_bytes,
                                     m_pDataBuffer, m_nDataBufferSize,
                                     m_pUserData) )
            {
              retError = m_eParserState;
              bContinue = false;
              break;
            }
            if(pnBytesRead)
            {
              *pnBytesRead = pes_pkt_data_bytes;
            }
          }
        }
        nOffset += pes_pkt_data_bytes;
        if(!m_bProgramStream)
          localoffset = (int)(localoffset + pes_pkt_data_bytes);

        if(bContinue)
        {
           retError = MP2STREAM_SUCCESS;
        }
        if(m_bProgramStream)
        {
          bContinue = false;
        }
      }
    }
    /* mark the flag as true, if ES parsing is successful at least once */
    if(MP2STREAM_SUCCESS == retError)
    {
      if(m_currPESPkt.packet_length > (uint32)(m_currPESPkt.pes_hdr_data_length + PES_PKT_FIXED_HDR_BYTES))
        m_currPESPkt.packet_length = m_currPESPkt.packet_length - (m_currPESPkt.pes_hdr_data_length + PES_PKT_FIXED_HDR_BYTES);
      m_bpartialPESTS = true;
    }
  }
  return retError;
}

/*! ======================================================================
@brief  Returns Width from video resolution reported by clip.

@detail    streamid identifies the video stream id

@param[in] N/A

@return    Video horizontal resolution if successful otherwise returns 0
@note      None.
========================================================================== */
uint32 MP2StreamParser::GetVideoWidth(uint32 ulTrackId)
{
  uint32 ulWidth = 0;
  stream_info* pStreamInfo = m_sContext.pStreamInfo;
  for(uint8 ucIndex = 0; ucIndex < m_sContext.usNumStreams; ucIndex++)
  {
    if(pStreamInfo[ucIndex].stream_id == ulTrackId)
    {
      ulWidth = pStreamInfo[ucIndex].video_stream_info.Width;
      break;
    }
  }
  return ulWidth;
}

/*! ======================================================================
@brief  Returns Height from video resolution reported by clip.

@detail    streamid identifies the video stream id

@param[in] N/A

@return    Video vertical resolution if successful otherwise returns 0
@note      None.
========================================================================== */
uint32 MP2StreamParser::GetVideoHeight(uint32 ulTrackId)
{
  uint32 ulHeight = 0;
  stream_info* pStreamInfo = m_sContext.pStreamInfo;
  for(uint8 ucIndex = 0; ucIndex < m_sContext.usNumStreams; ucIndex++)
  {
    if(pStreamInfo[ucIndex].stream_id == ulTrackId)
    {
      ulHeight = pStreamInfo[ucIndex].video_stream_info.Height;
      break;
    }
  }
  return ulHeight;
}

/*! ======================================================================
@brief  Returns audio sampling frequency for given audio track
@detail    trackd identifies the audio stream id

@param[in] N/A

@return    audio sampling frequency if successful otherwise returns 0
@note      None.
========================================================================== */
uint32 MP2StreamParser::GetAudioSamplingFrequency(uint32 ulTrackId)
{
  uint32 ulSampFreq = 0;
  stream_info* pStreamInfo = m_sContext.pStreamInfo;
  for(uint8 ucIndex = 0; ucIndex < m_sContext.usNumStreams; ucIndex++)
  {
    if(pStreamInfo[ucIndex].stream_id == ulTrackId)
    {
      ulSampFreq = pStreamInfo[ucIndex].audio_stream_info.SamplingFrequency;
      break;
    }
  }
  return ulSampFreq;
}

/*! ======================================================================
@brief  Returns number of audio channels for given audio track

@detail    trackid identifies the audio stream id

@param[in] N/A

@return    number of channels if successful otherwise returns 0
@note      None.
========================================================================== */
uint8  MP2StreamParser::GetNumberOfAudioChannels(uint32 ulTrackId)
{
  uint8 ucNumChannels = 0;
  stream_info* pStreamInfo = m_sContext.pStreamInfo;
  for(uint8 ucIndex = 0; ucIndex < m_sContext.usNumStreams; ucIndex++)
  {
    if(pStreamInfo[ucIndex].stream_id == ulTrackId)
    {
      ucNumChannels = pStreamInfo[ucIndex].audio_stream_info.NumberOfChannels;
      break;
    }
  }
  return ucNumChannels;
}
/*! ======================================================================
@brief  Returns the layer info from a MPG audio track

@detail    trackid identifies the audio stream id

@param[in] N/A

@return    Layer Info if successful otherwise returns 0
@note      None.
========================================================================== */
uint8  MP2StreamParser::GetLayer(uint32 ulTrackId)
{
  uint8 ucLayer = 0;
  stream_info* pStreamInfo = m_sContext.pStreamInfo;
  for(uint8 ucIndex = 0; ucIndex < m_sContext.usNumStreams; ucIndex++)
  {
    if(pStreamInfo[ucIndex].stream_id == ulTrackId)
    {
      ucLayer  = pStreamInfo[ucIndex].audio_stream_info.ucLayer;
      break;
    }
  }
  return ucLayer;
}
/*! ======================================================================
@brief  Returns the layer info from a MPG audio track

@detail    trackid identifies the audio stream id

@param[in] N/A

@return    Layer Info if successful otherwise returns 0
@note      None.
========================================================================== */
uint8  MP2StreamParser::GetVersion(uint32 ulTrackId)
{
  uint8 ucVersion = 0;
  stream_info* pStreamInfo = m_sContext.pStreamInfo;
  for(uint8 ucIndex = 0; ucIndex < m_sContext.usNumStreams; ucIndex++)
  {
    if(pStreamInfo[ucIndex].stream_id == ulTrackId)
    {
      ucVersion  = pStreamInfo[ucIndex].audio_stream_info.ucVersion;
      break;
    }
  }
  return ucVersion;
}

/*! ======================================================================
@brief  Returns the bit-rate for given track

@detail    trackid identifies the track

@param[in] N/A

@return    bit-rate if successful otherwise returns 0
@note      None.
========================================================================== */
uint32 MP2StreamParser::GetTrackAverageBitRate(uint32 ulTrackId)
{
  uint32 bitrate = 0;
  stream_info* pStreamInfo = m_sContext.pStreamInfo;
  for(uint8 ucIndex = 0; ucIndex < m_sContext.usNumStreams; ucIndex++)
  {
    if(pStreamInfo[ucIndex].stream_id == ulTrackId)
    {
      bitrate = pStreamInfo[ucIndex].bitRate;
      break;
    }
  }
  return bitrate;
}

/*! ======================================================================
@brief  Returns video frame rate for given track id

@detail    Returns PTS from current PES packet parsed by mpeg2 parser

@param[in] N/A

@return    Frame Rate fps for given track
@note      None.
========================================================================== */
float  MP2StreamParser::GetVideoFrameRate(uint32 ulTrackId)
{
  float frate = 0.0;
  stream_info* pStreamInfo = m_sContext.pStreamInfo;
  for(uint8 ucIndex = 0; ucIndex< m_sContext.usNumStreams; ucIndex++)
  {
    if(pStreamInfo[ucIndex].stream_id == ulTrackId)
    {
      switch(pStreamInfo[ucIndex].video_stream_info.Frame_Rate)
      {
        case FRAME_RATE_25_FPS:
          frate = 25.0;
          break;
        case FRAME_RATE_29_97_FPS:
          frate = (float)29.97;
          break;
        case FRAME_RATE_30_FPS:
          frate = (float)30;
          break;
      }
      break;
    }
  }
  return frate;
}

