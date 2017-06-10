/* =======================================================================
                              PSHeaderParser.cpp
DESCRIPTION
  Meaningful description of the definitions contained in this file.
  Description must specify if the module is portable specific, mobile
  specific, or common to both, and it should alert the reader if the
  module contains any conditional definitions which tailors the module to
  different targets.

Copyright (c) 2009-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
========================================================================== */

/* =======================================================================
                             PERFORCE HEADER
$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/MP2ParserLib/main/latest/src/PSHeaderParser.cpp#24 $
========================================================================== */
#include "MP2StreamParser.h"
#include "string.h"
#include "MMDebugMsg.h"
#include "MMMemory.h"
#include "MMMalloc.h"

#include "math.h"
/*! ======================================================================
@brief  Parses program stream pack header.

@detail    Starts parsing pack header from current byte offset.

@param[in] N/A

@return    MP2STREAM_SUCCESS is parsing is successful otherwise returns appropriate error.
@note      None.
========================================================================== */
MP2StreamStatus MP2StreamParser::parsePackHeader(uint64& nOffset,bool /*bParseMultiplePESPkts*/,
                                                 uint32 /*trackId*/,uint8* /*dataBuffer*/,
                                                 uint32 /*nMaxBufSize*/, int32* /*nBytesRead*/)
{
  MP2StreamStatus retError = MP2STREAM_SUCCESS;
  bool bContinue = true;
  if(!readMpeg2StreamData (nOffset, MPEG2_PACK_HDR_BYTES,
                           m_pDataBuffer, m_nDataBufferSize,
                           m_pUserData) )
  {
    retError = m_eParserState;
    bContinue = false;
  }
#ifdef MPEG2_PARSER_DEBUG
  MM_MSG_PRIO1(MM_FILE_OPS,MM_PRIO_HIGH,"parsePackHeader nOffset %llu",nOffset);
#endif

  if(bContinue)
  {
    m_currPackHeader.pack_start_code = MP2_PACK_START_CODE;
    m_currPackHeader.noffset = nOffset;
    m_nInitialPacksParsed++;
    //parse pack header fields from bytes read in
    int localoffset = 4;//we have already read in pack start code

    /*Check if Mpeg1 PS or Mpeg2 PS */

    if((m_pDataBuffer[4] & 0xf0) == 0x20)
    {
      m_sContext.bMPEG1Video = true;
    }

    if(m_sContext.bMPEG1Video)
    {
      nOffset += MPEG1_PACK_HDR_BYTES;
      uint8 firstpart = (uint8)((m_pDataBuffer[localoffset] & 0x0E)<<4);//get ms 3 bits
      localoffset++;
      uint16 secondpart = (uint16)(((m_pDataBuffer[localoffset] << 8) |
                                    (m_pDataBuffer[localoffset+1] & 0xFE) )>>1);
      localoffset+=2;
      uint16 thirdpart = (uint16)(((m_pDataBuffer[localoffset] << 8) |
                                   (m_pDataBuffer[localoffset+1] & 0xFE) )>>1);
      localoffset+=2;
      m_currPackHeader.scr_val = (double)make33BitValue(firstpart,secondpart,thirdpart);
      uint16 muxRate1 = ((m_pDataBuffer[localoffset] & 0x7F) << 1);
      muxRate1 = (uint16)(( (muxRate1 << 8)|(m_pDataBuffer[localoffset+1]) ));
      localoffset+=2;
      uint8 muxRate2 = (m_pDataBuffer[localoffset] & 0xFE) >> 1;
      m_currPackHeader.program_mux_rate = make22BitValue(muxRate1,muxRate2);
      localoffset++;
    }
    else
    {
      nOffset += MPEG2_PACK_HDR_BYTES;
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

      //get top 2 bits out of 9 bit scr extension
      uint8 scrextpart1     = (uint8)((m_pDataBuffer[localoffset++] & 0x03)<<6);
      //get lower 7 bits out of 9 bit scr extension
      uint8 scrextpart2     = (m_pDataBuffer[localoffset++] & 0xFE);
      m_currPackHeader.scr_extension = make9BitValue(scrextpart1,scrextpart2);

      m_currPackHeader.scr_base = (double)make33BitValue(firstpart,secondpart,thirdpart);
      m_currPackHeader.scr_base *= 300;

      m_currPackHeader.scr_val = m_currPackHeader.scr_base + m_currPackHeader.scr_extension;
      m_currPackHeader.scr_val = m_currPackHeader.scr_val /27000.0;    //27 MHz clock

#ifdef MPEG2_PARSER_DEBUG
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
                   "parsePackHeader scr_val %f ",m_currPackHeader.scr_val);
#endif
      //get top 16 bits from program mux rate
      uint16 prog_mux_rate_part1 = (uint16)((m_pDataBuffer[localoffset]<<8) |
                                             m_pDataBuffer[localoffset+1]);
      localoffset += 2;
      //get ls 6 bits from program mux rate
      uint8 prog_mux_rate_part2 = (m_pDataBuffer[localoffset] & 0xFC);
      prog_mux_rate_part2 = (uint8)(prog_mux_rate_part2 >> 2);
      prog_mux_rate_part2 &= 0x3F;
      localoffset++;
      m_currPackHeader.program_mux_rate = ((uint32)prog_mux_rate_part1)<<6;
      m_currPackHeader.program_mux_rate |= prog_mux_rate_part2;


      uint8* stuffing_val = m_pDataBuffer+MPEG2_PACK_HDR_BYTES-1;
      m_currPackHeader.pack_stuffing_length = (*stuffing_val)&(0x07);
      nOffset += m_currPackHeader.pack_stuffing_length;
      retError = MP2STREAM_SUCCESS;
    }
  }
  return retError;
}
/*! ======================================================================
@brief  Inspects Data content from given offset to determine total number of
        streams that exists in given program stream.

@detail    Inspects Data content from given offset to determine total number of
        streams that exists in given program stream

@param[in] N/A

@return    Number of streams from given program stream.
@note      Please note that offset being passed should point after
           target system fixed header..
========================================================================== */
uint8 MP2StreamParser::getNumberOfStreamsFromTargetHeader(int bytesRemaining, uint64 nStartOffset)
{
  uint8 nStreams = 0;
  bool bContinue = true;
  while( (bytesRemaining) > 0 && (bContinue ==true))
  {
    if(!readMpeg2StreamData (nStartOffset, SYS_HDR_STREAM_ID_INFO_BYTES,
                             m_pDataBuffer, m_nDataBufferSize,
                             m_pUserData ) )
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,"getNumberOfStreamsFromTargetHeader READ Failed!!");
      bContinue = false;
      continue;
    }
    nStartOffset += SYS_HDR_STREAM_ID_INFO_BYTES;
    bytesRemaining -= SYS_HDR_STREAM_ID_INFO_BYTES;
    if(m_pDataBuffer[0]& 0x01)//need to check if byte needs to be 0x01 or not
    {
      nStreams++;
    }
  }
  return nStreams;
}

/*! ======================================================================
@brief  Parses program stream system target header.

@detail    Starts parsing system target header from program stream.

@param[in] N/A

@return    MP2STREAM_SUCCESS is parsing is successful otherwise returns appropriate error.
@note      None.
========================================================================== */
MP2StreamStatus MP2StreamParser::parseSystemTargetHeader(uint32 /*trackId*/,
                                                         uint64& nOffset)
{
  MP2StreamStatus retError = MP2STREAM_DEFAULT_ERROR;
  bool bContinue = true;
  int localoffset = 0;
  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
               "parseSystemTargetHeader nOffset %llu", nOffset);

  if(!readMpeg2StreamData (nOffset, 2,
                           m_pDataBuffer, m_nDataBufferSize,
                           m_pUserData) )
  {
    retError = m_eParserState;
    bContinue = false;
  }
  uint8 tmpAudioBound = 0;
  uint8 tmpVideoBound = 0;
  uint16 Length    = (uint16)((m_pDataBuffer[localoffset]<<8) |
                              m_pDataBuffer[localoffset+1]);

  /* If system header is already parsed, then update Offset field and do not
     parse the system header details again. It will avoid overhead processing.
  */
  if(m_currPackHeader.sys_header)
  {
    nOffset += Length +  sizeof(Length);
    return MP2STREAM_SUCCESS;
  }
  system_header* trg_sys_hdr = (system_header*)MM_Malloc(sizeof(system_header));
  if(!trg_sys_hdr)
  {
    retError = MP2STREAM_OUT_OF_MEMORY;
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                "parseSystemTargetHeader sys_header allocation failed");
  }
  else
  {
    memset(trg_sys_hdr,0,sizeof(system_header));
    trg_sys_hdr->sys_header_start_code = MP2_SYS_HDR_START_CODE;
    trg_sys_hdr->header_length = (uint16)((m_pDataBuffer[localoffset]<<8) |
                                           m_pDataBuffer[localoffset+1]);
    trg_sys_hdr->noffset = nOffset - sizeof(MP2_SYS_HDR_START_CODE);
    nOffset += 2*(sizeof(uint8));
#ifdef MPEG2_PARSER_DEBUG
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,"parseSystemTargetHeader size %u",
                 trg_sys_hdr->header_length);
#endif

    if(!readMpeg2StreamData (nOffset, Length,
                             m_pDataBuffer, m_nDataBufferSize,
                             m_pUserData) )
    {
      retError = m_eParserState;
      bContinue = false;
    }
    nOffset += Length;

    //get top 15 bits from rate bound
    uint16 rate_bound_part1 = (uint16)((m_pDataBuffer[localoffset]<<8) |
                                        m_pDataBuffer[localoffset + 1]);
    localoffset += 2;
    //knock off marker bit and get the 15 bits of interest.
    rate_bound_part1 = (uint16)(rate_bound_part1<<1);
    //get ls 7 bits from rate bound
    uint8 rate_bound_part2 = (m_pDataBuffer[localoffset] & 0xFE);
    localoffset++;
    trg_sys_hdr->rate_bound = make22BitValue(rate_bound_part1,rate_bound_part2);
    if(trg_sys_hdr->rate_bound && (m_nClipDuration == 0) )
    {
      //rate bound is measured in RATE_MEASURED_UNITS bytes/seconds
      m_nClipDuration = m_nFileSize * MPEG2_STREAM_TIME_SCALE;
      m_nClipDuration = m_nClipDuration/
                         (RATE_MEASURED_UNITS * trg_sys_hdr->rate_bound);
    }

    trg_sys_hdr->audio_bound = (uint8)((m_pDataBuffer[localoffset] & 0xFC)>>2);
    trg_sys_hdr->fixed_flag  = m_pDataBuffer[localoffset] & 0x02 >> 1;
    trg_sys_hdr->csps_flag   = m_pDataBuffer[localoffset++] & 0x01;

    if ((m_pDataBuffer[localoffset] & 0x80) >> 7)
       trg_sys_hdr->sys_audio_lock_flag = true;
    if ((m_pDataBuffer[localoffset] & 0x40) >> 6)
      trg_sys_hdr->sys_video_lock_flag = true;
    trg_sys_hdr->video_bound         =  m_pDataBuffer[localoffset++] & 0x1F;
    if((m_pDataBuffer[localoffset++]& 0x80) >> 7)
      trg_sys_hdr->packet_restriction_flag = true;
    uint16 bytesRemaining = (uint16)(trg_sys_hdr->header_length - SYS_HDR_BYTES);

    MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_HIGH,
                 "parseSystemTargetHeader video_bound %d audio_bound %d",
                 trg_sys_hdr->audio_bound, trg_sys_hdr->video_bound);

    //We encountered system target header for the first time
    //allocate memory for storing stream information
    m_sContext.usNumStreams = (uint16)(trg_sys_hdr->video_bound + trg_sys_hdr->audio_bound);
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
                 "parseSystemTargetHeader #Total Streams %lu",m_sContext.usNumStreams);
    if(m_sContext.usNumStreams)
    {
      m_sContext.pStreamInfo = (stream_info*)
                       MM_Malloc(m_sContext.usNumStreams * sizeof(stream_info));
      if(!m_sContext.pStreamInfo)
      {
        bContinue = false;
        retError  = MP2STREAM_OUT_OF_MEMORY;
      }
      else
      {
        memset(m_sContext.pStreamInfo,0, sizeof(stream_info)* m_sContext.usNumStreams);
      }
    }
    uint16 usIndex = 0;
    stream_info* pStreamInfo = m_sContext.pStreamInfo;
    while( (bytesRemaining >= SYS_HDR_STREAM_ID_INFO_BYTES) && (bContinue ==true))
    {
      //! Sometimes we may have more than required streams info.
      //! These extra streams are not related to audio/video metadata.
      //! Parser need not to store these streams details.
      if (usIndex >= m_sContext.usNumStreams )
      {
        break;
      }

      bytesRemaining = (uint16)(bytesRemaining - SYS_HDR_STREAM_ID_INFO_BYTES);
      if(pStreamInfo)
      {
        pStreamInfo[usIndex].stream_media_type = TRACK_TYPE_UNKNOWN;
        pStreamInfo[usIndex].stream_id  = m_pDataBuffer[localoffset++];
        pStreamInfo[usIndex].buffer_bound_scale = false;
        if((m_pDataBuffer[localoffset] & 0x20)>>5)
          pStreamInfo[usIndex].buffer_bound_scale = true;
        pStreamInfo[usIndex].buffer_size_bound = (uint16)
          ((m_pDataBuffer[localoffset] & 0x1F<<8) |
           m_pDataBuffer[localoffset + 1]);
        localoffset += 2;

        if(!pStreamInfo[usIndex].buffer_bound_scale)
        {
          pStreamInfo[usIndex].buffer_size =
          pStreamInfo[usIndex].buffer_size_bound * 128;
        }
        else
        {
          pStreamInfo[usIndex].buffer_size =
          pStreamInfo[usIndex].buffer_size_bound * 1024;
        }
      }
      usIndex++;
    }//while( (bytesRemaining) > 0 && (bContinue ==true))

    retError = MP2STREAM_SUCCESS;
    if(m_currPackHeader.sys_header == NULL)
    {
      m_currPackHeader.sys_header = (system_header*)
                                    MM_Malloc(sizeof(system_header));
      if(m_currPackHeader.sys_header)
      {
        memcpy(m_currPackHeader.sys_header,trg_sys_hdr,sizeof(system_header));
      }
    }
    MM_Free(trg_sys_hdr);
  }//end of else of if(!trg_sys_hdr)

  return retError;
}

/*! ======================================================================
@brief  Parses program stream system target header.

@detail    Starts parsing system target header from program stream.

@param[in] N/A

@return    MP2STREAM_SUCCESS is parsing is successful otherwise returns appropriate error.
@note      None.
========================================================================== */
MP2StreamStatus MP2StreamParser::parseProgStreamMap(uint64& ullOffset)
{
  MP2StreamStatus retError = MP2STREAM_DEFAULT_ERROR;
  bool bContinue = true;
  uint32 ulIndex = 0;
#ifdef MPEG2_PARSER_DEBUG
  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,"parseProgStreamMap nOffset %llu",
               ullOffset);
#endif

  if(!readMpeg2StreamData (ullOffset, m_currPESPkt.packet_length,m_pDataBuffer,
                           m_nDataBufferSize, m_pUserData) )
  {
    retError  = m_eParserState;
    bContinue = false;
  }
  ProgramStreamMap* pPSMInfo = m_sContext.pProgramStreamMap;
  if (!pPSMInfo)
  {
    pPSMInfo = (ProgramStreamMap*)MM_Malloc(sizeof(ProgramStreamMap));
    m_sContext.pProgramStreamMap = pPSMInfo;
  }
  if (!pPSMInfo)
  {
    retError = MP2STREAM_OUT_OF_MEMORY;
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                "parseProgStreamMap m_pProgramStreamMap alloc failed");
  }
  else
  {
    memset(pPSMInfo, 0, sizeof(ProgramStreamMap));
    pPSMInfo->current_next_indicator = (m_pDataBuffer[ulIndex] & 0x80)>>7;
    pPSMInfo->version_no             = m_pDataBuffer[ulIndex++]&0x1F;
    //Reserved byte
    ulIndex++;
    pPSMInfo->program_stream_length  = (uint16)
      ((m_pDataBuffer[ulIndex]<<8) | m_pDataBuffer[ulIndex+1]);
    ulIndex += 2;
#ifdef MPEG2_PARSER_DEBUG
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,"parseProgStreamMap length %u",
                 pPSMInfo->program_stream_length);
#endif
    //Skip program stream descriptors
    ulIndex += pPSMInfo->program_stream_length;

    pPSMInfo->elementary_stream_map_len  = (uint16)
      ((m_pDataBuffer[ulIndex]<<8) | m_pDataBuffer[ulIndex+1]);
    ulIndex += 2;

    uint32 ulBytesRemaining = m_currPESPkt.packet_length - ulIndex;
    uint32 ulCounter        = ulIndex;
    uint16 usNumStreams     = m_sContext.usNumStreams;
    //! 4bytes at the end are used for CRC, so skip them
    ulBytesRemaining -= 4;


    while( (ulBytesRemaining >= PS_ELEMSTREAM_DESC_HEADER_LEN) &&
           (bContinue))
    {
      uint8 stream_type = m_pDataBuffer[ulIndex++];
      ulBytesRemaining--;
      uint8 stream_id   = m_pDataBuffer[ulIndex++];
      ulBytesRemaining--;
      uint16 stream_len = (uint16)((m_pDataBuffer[ulIndex]<<8) |
                                   m_pDataBuffer[ulIndex+1]);
      ulIndex += 2;
      ulBytesRemaining -= 2;
      /* Need to add support for parsing Descriptors*/
      ulBytesRemaining -= stream_len;

      /* Update track properties. */
      UpdateStreamInfo(stream_type, stream_id, &m_sContext);
    }//while( (bytesRemaining) > 4 && (bContinue ==true))
    stream_info* pStreamInfo = m_sContext.pStreamInfo;
    for (uint32 ulCount = 0; ulCount < usNumStreams; ulCount++)
    {
      if (!pStreamInfo)
      {
        break;
      }

      if( ((pStreamInfo[ulCount].stream_id & 0xB8) == 0xB8)         ||
          ((pStreamInfo[ulCount].stream_id >= AUDIO_STREAM_ID_START) &&
           (pStreamInfo[ulCount].stream_id <= AUDIO_STREAM_ID_END) ) )
      {
        if(!m_sContext.usAudioPIDSelected)
        {
          m_sContext.usAudioPIDSelected = pStreamInfo[ulCount].stream_id;
        }
        m_sContext.usNumStreamsSelected++;
      }
      else if( ((pStreamInfo[ulCount].stream_id & 0xB9) == 0xB9)    ||
               ((pStreamInfo[ulCount].stream_id >= VIDEO_STREAM_ID_START) &&
                (pStreamInfo[ulCount].stream_id <= VIDEO_STREAM_ID_END) ) )
      {
        if(!m_sContext.usVideoPIDSelected)
        {
          m_sContext.usVideoPIDSelected = pStreamInfo[ulCount].stream_id;
        }
        m_sContext.usNumStreamsSelected++;
      }
    }//! for (uint32 ulCount = 0; ulCount < m_nstreams; ulCount++)

    retError = MP2STREAM_SUCCESS;
  }//end of else of if (!m_pProgramStreamMap)

  ullOffset += m_currPESPkt.packet_length;
  return retError;
}

