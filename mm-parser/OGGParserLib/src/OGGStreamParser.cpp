/* =======================================================================
                              OGGStreamParser.cpp
DESCRIPTION
  Meaningful description of the definitions contained in this file.
  Description must specify if the module is portable specific, mobile
  specific, or common to both, and it should alert the reader if the
  module contains any conditional definitions which tailors the module to
  different targets.

Copyright 2009-2015 QUALCOMM Technologies Incorporated, All Rights Reserved.
QUALCOMM Technologies Proprietary and Confidential.
========================================================================== */

/* =======================================================================
                             PERFORCE HEADER
$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/OGGParserLib/main/latest/src/OGGStreamParser.cpp#38 $
========================================================================== */
#include "parserdatadef.h"
#include "OGGStreamParser.h"
#include "string.h"
#include "MMDebugMsg.h"
#include "MMMemory.h"
#include "MMMalloc.h"
#include "math.h"
#include <stdio.h>

#ifdef FEATURE_FILESOURCE_OGG_PARSER
//! This macro is used to call same Seek API internally by updating seek time
#define OGG_SEEK_DELTA (3000)
/*! ======================================================================
@brief    OGGStreamParser constructor

@detail    Instantiates OGGStreamParser to parse OGG stream.

@param[in]  pUData    APP's UserData.This will be passed back when this parser invokes the callbacks.
@return    N/A
@note      None
========================================================================== */
OGGStreamParser::OGGStreamParser(void* pUData,uint64 fsize,bool bAudio)
{
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"OGGStreamParser");
  m_pUserData = pUData;
  m_nCurrOffset = 0;
  m_nFileSize = fsize;
  m_nOggDataBufferSize = 0;
  m_nOggDataBufferFillSize = 0;
  m_nAudioStreams = m_nVideoStreams = 0;
  m_nClipDuration = 0;
  m_nstreams      = 0;
  m_eParserState = OGGSTREAM_IDLE;
  m_pDataBuffer = NULL;
  m_pOggAudioCodecInfo = NULL;
  m_pOggVideoCodecInfo = NULL;
  m_pOggIndex  = NULL;
  m_bOggMetaDataParsed = false;
  m_bParsedLastPage = false;
  m_bPlayAudio = bAudio;
  m_pOggPage = NULL;
#ifdef FEATURE_FILESOURCE_FLAC_PARSER
  m_pFlacParser = NULL;
#endif
  m_nStartGranulePosn = 0;
  m_nEndGranulePosn = 0;
  m_nDataPageOffset = 0;
  m_nMetaData = 0;
  m_pDataCache = NULL;
  m_bUserInitiatedRW = false;
  m_pOggMetaData = NULL;
  m_nDataCacheFileOffset = 0;
  m_pDataBufferOffset = 0;
  m_nCurrentTimeStampMs = 0;
  m_bTimeStampValid = true;
  InitCrcComputeTables();
}
/*! ======================================================================
@brief    OGGStreamParser destructor

@detail    OGGStreamParser destructor

@param[in] N/A
@return    N/A
@note      None
========================================================================== */
OGGStreamParser::~OGGStreamParser()
{
  #ifdef OGG_PARSER_DEBUG
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"~OGGStreamParser");
  #endif
  if(m_pDataCache)
  {
    MM_Free(m_pDataCache);
  }
  if(m_pOggAudioCodecInfo)
  {
    for(uint32 i = 0; i< m_nAudioStreams;i++)
    {
      if(m_pOggAudioCodecInfo[i].IdentificationHdr)
      {
        MM_Free(m_pOggAudioCodecInfo[i].IdentificationHdr);
      }
      if(m_pOggAudioCodecInfo[i].CommentHdr)
      {
        MM_Free(m_pOggAudioCodecInfo[i].CommentHdr);
      }
      if(m_pOggAudioCodecInfo[i].SetupHdr)
      {
        MM_Free(m_pOggAudioCodecInfo[i].SetupHdr);
      }
      if(m_pOggAudioCodecInfo[i].pCodecHeader)
      {
        MM_Free(m_pOggAudioCodecInfo[i].pCodecHeader);
      }
    }
    MM_Free(m_pOggAudioCodecInfo);
  }
  if(m_pOggVideoCodecInfo)
  {
    for(uint32 i = 0; i< m_nVideoStreams;i++)
    {
      if(m_pOggVideoCodecInfo[i].IdentificationHdr)
      {
        MM_Free(m_pOggVideoCodecInfo[i].IdentificationHdr);
      }
      if(m_pOggVideoCodecInfo[i].CommentHdr)
      {
        MM_Free(m_pOggVideoCodecInfo[i].CommentHdr);
      }
      if(m_pOggVideoCodecInfo[i].SetupHdr)
      {
        MM_Free(m_pOggVideoCodecInfo[i].SetupHdr);
      }
      if(m_pOggVideoCodecInfo[i].pCodecHeader)
      {
        MM_Free(m_pOggVideoCodecInfo[i].pCodecHeader);
      }
    }
    MM_Free(m_pOggVideoCodecInfo);
  }
  if(m_pOggPage)
  {
    MM_Free(m_pOggPage);
  }
#ifdef FEATURE_FILESOURCE_FLAC_PARSER
  if(m_pFlacParser)
  {
    MM_Delete(m_pFlacParser);
  }
#endif
  if(m_pOggMetaData)
  {
    for(int i = 0 ; i < m_nMetaData; i++)
    {
      if(m_pOggMetaData[i].pMetaData)
      {
        MM_Free(m_pOggMetaData[i].pMetaData);
      }
    }
    MM_Free(m_pOggMetaData);
  }
#ifdef FEATURE_OGGPARSER_BUILD_SEEK_INDEX
  FreeOGGIndexInfo();
#endif
}
/*! ======================================================================
@brief  Starts Parsing the OGG stream.

@detail    This function starts parsing the OGG stream.
        Upon successful parsing, user can retrieve total number of streams and
        stream specific information.

@param[in] N/A
@return    OGGSTREAM_SUCCESS is parsing is successful otherwise returns appropriate error.
@note      StartParsing needs to be called before retrieving any elementary stream specific information.
========================================================================== */
OGGStreamStatus OGGStreamParser::StartParsing(void)
{
  OGGStreamStatus retError = OGGSTREAM_CORRUPT_DATA;
  bool bContinue = true;
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"StartParsing");
  //Allocate buffers to hold more than a page for caching data locally
  m_pDataCache = (uint8*)MM_Malloc(MAX_PAGE_SIZE * 4);
  m_nDataCacheFileOffset = 0;
  m_nOggDataBufferFillSize = 0;
  if(m_pDataCache)
  {
    m_pDataBuffer = m_pDataCache;
    m_nOggDataBufferSize = MAX_PAGE_SIZE * 4;
    m_pDataBufferOffset = m_nOggDataBufferSize;
  }

  m_pOggPage = (OggPage*)MM_Malloc(sizeof(OggPage));
  if(m_pOggPage)
  {
    memset(m_pOggPage,0,sizeof(OggPage));
  }
  if ((!m_pOggPage) || (!m_pDataBuffer))
  {
    bContinue = false;
  }
  /* Start parsing OGG Stream */
  while(bContinue                    &&
       (m_nCurrOffset < m_nFileSize) &&
       (!m_bOggMetaDataParsed   ||
        !m_nDataPageOffset) )
  {
#ifdef OGG_PARSER_DEBUG
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
                 "StartParsing m_nCurrOffset %llu", m_nCurrOffset);
#endif
    retError = ParseOGGPage(m_nCurrOffset,m_pOggPage);

    // Even if Parser state is not proper, try to parse next page
    if((m_eParserState == OGGSTREAM_PAGE_CRC_ERROR) ||
       (m_eParserState ==  OGGSTREAM_CORRUPT_DATA))
    {
        m_nCurrOffset = m_pOggPage->PageEndOffset;
        continue;
    }
    else if(retError == OGGSTREAM_SUCCESS)
    {
#ifdef FEATURE_OGGPARSER_BUILD_SEEK_INDEX
      if( (m_pOggIndex) && (m_pOggPage->Granule) && (!m_pOggPage->ContFlag) )
      {
        if(IndexOGGPage(m_pOggPage,m_pOggPage->Codec) != OGGSTREAM_SUCCESS)
        {
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"StartParsing IndexOGGPage failed..Deleting IndexInfo....");
          FreeOGGIndexInfo();
        }
      }
#endif
      if(!m_bOggMetaDataParsed || !m_nDataPageOffset)
      {
        m_nCurrOffset = m_pOggPage->PageEndOffset;
      }
    }
    else
    {
      bContinue = false;
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
      "StartParsing ParseOGGPage returned failure @ offset %llu",m_nCurrOffset);
    }
  }//while(bContinue...
  m_pDataBufferOffset =  m_nOggDataBufferSize;
  m_nDataCacheFileOffset = 0;
  m_bParsedLastPage = false;
  if( (bContinue) && (m_bOggMetaDataParsed) )
  {
    MM_MSG_PRIO3(MM_FILE_OPS, MM_PRIO_HIGH,
      "MetaData parsed successfully:Total#Streams %d #AudioStreams %d #VideoStreams %d",
      m_nstreams,m_nAudioStreams,m_nVideoStreams);
    retError = OGGSTREAM_SUCCESS;
    m_eParserState = OGGSTREAM_READY;
#ifndef FEATURE_OGGPARSER_FAST_START_UP
    m_bParsedLastPage = false;
    m_nCurrOffset = m_nDataPageOffset;
#else
    if(m_bOggMetaDataParsed == true)
    {
    m_nCurrOffset = m_nDataPageOffset;
    }
#endif
    m_nStartGranulePosn = 0;
    m_nEndGranulePosn = 0;
    //Prepare codec header for each stream
    for(int i =0; i< m_nAudioStreams; i++)
    {
      uint32 size = 0;
      size+= m_pOggAudioCodecInfo[i].nIdentificationHdrSize;
      size+= m_pOggAudioCodecInfo[i].nCommentHdrSize;
      size+= m_pOggAudioCodecInfo[i].nSetupHdrSize;
      if( (size) && (m_pOggAudioCodecInfo[i].Audio_Codec == OGG_AUDIO_CODEC_VORBIS) )
      {
        m_pOggAudioCodecInfo[i].pCodecHeader =
          (uint8*)MM_Malloc(size);
        if(m_pOggAudioCodecInfo[i].pCodecHeader)
        {
          uint32 nwriteindex = 0;
          if(m_pOggAudioCodecInfo[i].IdentificationHdr)
          {
            memcpy(m_pOggAudioCodecInfo[i].pCodecHeader+nwriteindex,
                   m_pOggAudioCodecInfo[i].IdentificationHdr,
                   m_pOggAudioCodecInfo[i].nIdentificationHdrSize);
            nwriteindex += m_pOggAudioCodecInfo[i].nIdentificationHdrSize;
          }
          if(m_pOggAudioCodecInfo[i].CommentHdr)
          {
            memcpy(m_pOggAudioCodecInfo[i].pCodecHeader+nwriteindex,
                   m_pOggAudioCodecInfo[i].CommentHdr,
                   m_pOggAudioCodecInfo[i].nCommentHdrSize);
            nwriteindex += m_pOggAudioCodecInfo[i].nCommentHdrSize;
          }
          if(m_pOggAudioCodecInfo[i].SetupHdr)
          {
            memcpy(m_pOggAudioCodecInfo[i].pCodecHeader+nwriteindex,
                   m_pOggAudioCodecInfo[i].SetupHdr,
                   m_pOggAudioCodecInfo[i].nSetupHdrSize);
            nwriteindex += m_pOggAudioCodecInfo[i].nSetupHdrSize;
          }
        }//if(m_pOggAudioCodecInfo[i].pCodecHeader)
      }//if(size)
    }//for(int i =0; i< m_nAudioStreams; i++)

    for(int i =0; i< m_nVideoStreams; i++)
    {
      uint32 size = 0;
      size+= m_pOggVideoCodecInfo[i].nIdentificationHdrSize;
      size+= m_pOggVideoCodecInfo[i].nCommentHdrSize;
      size+= m_pOggVideoCodecInfo[i].nSetupHdrSize;
      if(size)
      {
        m_pOggVideoCodecInfo[i].pCodecHeader =
          (uint8*)MM_Malloc(size);
        if(m_pOggVideoCodecInfo[i].pCodecHeader)
        {
          uint32 nwriteindex = 0;
          if(m_pOggVideoCodecInfo[i].IdentificationHdr)
          {
            memcpy(m_pOggAudioCodecInfo[i].pCodecHeader+nwriteindex,
                   m_pOggVideoCodecInfo[i].IdentificationHdr,
                   m_pOggVideoCodecInfo[i].nIdentificationHdrSize);
            nwriteindex+= m_pOggVideoCodecInfo[i].nIdentificationHdrSize;
          }
          if(m_pOggVideoCodecInfo[i].CommentHdr)
          {
            memcpy(m_pOggVideoCodecInfo[i].pCodecHeader+nwriteindex,
                   m_pOggVideoCodecInfo[i].CommentHdr,
                   m_pOggVideoCodecInfo[i].nCommentHdrSize);
            nwriteindex+= m_pOggVideoCodecInfo[i].nCommentHdrSize;
          }
          if(m_pOggVideoCodecInfo[i].SetupHdr)
          {
            memcpy(m_pOggVideoCodecInfo[i].pCodecHeader+nwriteindex,
                   m_pOggVideoCodecInfo[i].SetupHdr,
                   m_pOggVideoCodecInfo[i].nSetupHdrSize);
            nwriteindex+= m_pOggVideoCodecInfo[i].nSetupHdrSize;
          }
        }//if(m_pOggVideoCodecInfo[i].pCodecHeader)
      }//if(size)
    }//for(int i =0; i< m_nVideoStreams; i++)

  }//if( (bContinue) && (m_bOggMetaDataParsed) )
  else
  {
    retError = OGGSTREAM_CORRUPT_DATA;
  }
  return retError;
}
/* =============================================================================
FUNCTION:
 OGGStreamParser::ParseOGGPage

DESCRIPTION:
Parses OGG Page from the given offset

INPUT/OUTPUT PARAMETERS:

RETURN VALUE:
OGGSTREAM_SUCCESS if successful in parsing the page otherwise, returns appropriate error.

SIDE EFFECTS:
  None.
=============================================================================*/
OGGStreamStatus  OGGStreamParser::ParseOGGPage(uint64 offset,OggPage* pOggPage)
{
#ifdef OGG_PARSER_DEBUG
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"ParseOGGPage");
#endif
  OGGStreamStatus retError = OGGSTREAM_INVALID_PARAM;
  uint32 localOffset = 0;

  if(m_nDataCacheFileOffset <= offset &&
     offset < (m_nDataCacheFileOffset + m_nOggDataBufferFillSize) &&
     m_pDataBufferOffset != m_nOggDataBufferSize)
  {
     m_pDataBuffer = m_pDataCache + (offset - m_nDataCacheFileOffset);
     m_pDataBufferOffset = uint32(m_pDataBuffer - m_pDataCache);
  }
  else
  {
     m_pDataBufferOffset = m_nOggDataBufferSize;
  }



  if(m_pDataCache && pOggPage && m_nOggDataBufferSize)
  {
    //! Check Max available data
    uint32 nAvailData  = (uint32)(m_nFileSize - offset);
    uint32 nSizeNeeded = FILESOURCE_MIN(m_nOggDataBufferSize, nAvailData);
    //! For Last few pages, Max data need not be "MAX_PAGE_SIZE" value.
    //! Check against Max available data and take minimum value
    uint32 nMaxData    = FILESOURCE_MIN(nSizeNeeded, MAX_PAGE_SIZE);
    retError = OGGSTREAM_SUCCESS;
    memset(pOggPage,0,sizeof(OggPage));
#ifdef OGG_PARSER_DEBUG
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,"ParseOGGPage nSizeNeeded %d",nSizeNeeded);
#endif

    if(m_nOggDataBufferFillSize <= m_pDataBufferOffset||
        (m_nOggDataBufferFillSize - m_pDataBufferOffset < nMaxData
         && m_nOggDataBufferFillSize == m_nOggDataBufferSize)||
        (offset >= m_nFileSize))
    {
      //memset(m_pDataCache,0,m_nOggDataBufferSize);
      if(offset >= m_nFileSize)
      {
        m_bParsedLastPage = true;
        m_nOggDataBufferFillSize = 0;
        m_nDataCacheFileOffset = 0;
        m_pDataBufferOffset = 0;
        retError = OGGSTREAM_READ_ERROR;
        m_eParserState = OGGSTREAM_READ_ERROR;
#ifdef OGG_PARSER_DEBUG
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
                 "ParseOGGPage failed due to offset exceeds %llu",m_nFileSize);
#endif
        return retError;
      }
      else if( (nSizeNeeded + offset) > m_nFileSize )
      {
        nSizeNeeded = (int)(m_nFileSize - offset);
#ifdef OGG_PARSER_DEBUG
        MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,"ParseOGGPage Updated nSizeNeeded %d",nSizeNeeded);
#endif
        memset(m_pDataCache + nSizeNeeded, 0, m_nOggDataBufferSize - nSizeNeeded);
      }
      if(!OGGStreamCallbakGetData (offset, nSizeNeeded, m_pDataCache,
                                   m_nOggDataBufferSize, m_pUserData) )
      {
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,"ParseOGGPage read failed..");
        m_bParsedLastPage = true;
        m_nOggDataBufferFillSize = 0;
        m_nDataCacheFileOffset = 0;
        m_pDataBufferOffset = 0;
        retError = OGGSTREAM_READ_ERROR;
        m_eParserState = OGGSTREAM_READ_ERROR;
      }
      else
      {
        m_eParserState = OGGSTREAM_READY;
        m_nOggDataBufferFillSize = nSizeNeeded;
        m_nDataCacheFileOffset = offset;
        m_pDataBufferOffset = 0;
        m_pDataBuffer = m_pDataCache;
      }
    }
    if(m_eParserState != OGGSTREAM_READ_ERROR)
    {
      //Make sure there is page SYNC
      //if(memcmp(m_pDataBuffer+localOffset,OGG_PAGE_MAGIC_NUMBER,PAGE_MAGIC_NUMBER_SIZE) != 0)
      if(m_pDataBuffer[localOffset] != 'O' ||
         m_pDataBuffer[localOffset + 1] != 'g'||
         m_pDataBuffer[localOffset + 2] != 'g'||
         m_pDataBuffer[localOffset + 3] != 'S')

      {
        uint64 nNextPageOffset = 0;
        nNextPageOffset = FindNextPageOffset(offset);
        pOggPage->PageEndOffset = nNextPageOffset;
        retError = OGGSTREAM_CORRUPT_DATA;
        m_eParserState = OGGSTREAM_CORRUPT_DATA;
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,"ParseOGGPage Failed to locate OGG Page SYNC!!!");
      }
      else
      {
        pOggPage->PageOffset = offset;
        localOffset  += PAGE_MAGIC_NUMBER_SIZE;
#ifdef OGG_PARSER_DEBUG
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"ParseOGGPage Located OGG Page SYNC");
#endif
        //Ensure correct OGG bitstream format version
        if(m_pDataBuffer[localOffset]!= OGG_STREAM_VERSION_FORMAT )
        {
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,"ParseOGGPage Invalid Version#");
          retError = OGGSTREAM_CORRUPT_DATA;
          m_eParserState = OGGSTREAM_CORRUPT_DATA;
        }
        else
        {
          m_pOggPage->nPageSegmentsCorruptSample = 0;
          pOggPage->Version = m_pDataBuffer[localOffset];
          //Parse rest of the page
          localOffset++;
          pOggPage->HdrType = m_pDataBuffer[localOffset];
          uint8 contpkt = m_pDataBuffer[localOffset] & OGG_CONT_PAGE;
          uint8 bospage = m_pDataBuffer[localOffset] & OGG_BOS_PAGE;
          uint8 eospage = m_pDataBuffer[localOffset] & OGG_EOS_PAGE;
          pOggPage->ContFlag = contpkt;
          pOggPage->BOSFlag = bospage;
          pOggPage->EOSFlag = eospage;
          localOffset++;
#ifdef OGG_PARSER_DEBUG
          MM_MSG_PRIO3(MM_FILE_OPS, MM_PRIO_HIGH,
                       "ParseOGGPage contpkt %d bospage %d eospage %d",
                       contpkt,bospage,eospage);
#endif
          pOggPage->Granule = (uint64)(m_pDataBuffer[localOffset++]);
          pOggPage->Granule |= (uint64)(m_pDataBuffer[localOffset++]) << 8;
          pOggPage->Granule |= (uint64)(m_pDataBuffer[localOffset++]) << 16;
          pOggPage->Granule |= (uint64)(m_pDataBuffer[localOffset++]) << 24;
          pOggPage->Granule |= (uint64)(m_pDataBuffer[localOffset++]) << 32;
          pOggPage->Granule |= (uint64)(m_pDataBuffer[localOffset++]) << 40;
          pOggPage->Granule |= (uint64)(m_pDataBuffer[localOffset++]) << 48;
          pOggPage->Granule |= (uint64)(m_pDataBuffer[localOffset++]) << 56;

#ifdef OGG_PARSER_DEBUG
          MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
                      "ParseOGGPage pOggPage->Granule %llu",pOggPage->Granule);
#endif

          pOggPage->SerialNo = (uint32)(m_pDataBuffer[localOffset++]);
          pOggPage->SerialNo |= (uint32)(m_pDataBuffer[localOffset++]) << 8;
          pOggPage->SerialNo |= (uint32)(m_pDataBuffer[localOffset++]) << 16;
          pOggPage->SerialNo |= (uint32)(m_pDataBuffer[localOffset++]) << 24;

          pOggPage->SeqNo = (uint32)(m_pDataBuffer[localOffset++]);
          pOggPage->SeqNo |= (uint32)(m_pDataBuffer[localOffset++]) << 8;
          pOggPage->SeqNo |= (uint32)(m_pDataBuffer[localOffset++]) << 16;
          pOggPage->SeqNo |= (uint32)(m_pDataBuffer[localOffset++]) << 24;

          localOffset += FOURCC_SIGNATURE_BYTES;

          pOggPage->PageSegments = m_pDataBuffer[localOffset];
#ifdef OGG_PARSER_DEBUG
          MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
                     "ParseOGGPage page_segments %lu", pOggPage->PageSegments);
#endif
          localOffset++;
          uint32 size = localOffset + pOggPage->PageSegments;

          //Store offsets relative to start of page offset as opposed
          //to absolute file offset.
          //pOggPage->SegmentTableOffset = pOggPage->PageOffset + localOffset;
          pOggPage->SegmentTableOffset = localOffset;

          // If the new Page has no packets continuing from a previous corrupt
          // page we can move state to READY
          if(m_eParserState == OGGSTREAM_PAGE_CRC_ERROR &&!pOggPage->ContFlag)
          {
            m_eParserState = OGGSTREAM_READY;
          }

          for(uint32 i = 0; i < pOggPage->PageSegments; i++)
          {
            size += m_pDataBuffer[localOffset];

            if(m_eParserState == OGGSTREAM_PAGE_CRC_ERROR &&
               m_pDataBuffer[localOffset] < 0xff)
            {
               // If page has a packet continuing from a
               // previous corrupt page, we need to discard it.
               // Here we will mark how many segments corresponds to
               // packet in previous page.
               pOggPage->nPageSegmentsCorruptSample = i + 1;

               // Now that we have found that current page has a packet
               // starting in it, we can change the status from CRC error
               // and alow the new packet to be decoded. If no new packet
               // start in this page whole page has to be discarded.
               m_eParserState = OGGSTREAM_READY;
            }

            localOffset++;
          }
          offset += size;
          m_pDataBufferOffset += size;
          pOggPage->PageEndOffset = offset;


          //Now that we have the Page available in buffer and Page size known
          //We can validate the page for bitstream errors

          if(!CheckPageCRC(m_pDataBuffer, size))
          {
            MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,"ParseOGGPage CRC check failed");

            // See if a valid page starts from the size calculated.
            if(memcmp(m_pDataBuffer+size,OGG_PAGE_MAGIC_NUMBER,PAGE_MAGIC_NUMBER_SIZE) != 0)
            {
              uint64 nNextPageOffset;

              MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,"ParseOGGPage:Locate OGG Page SYNC after corrupt Page!!!");

              // Send the valid offset and try to find the next page. Add PAGE_MAGIC_NUMBER_SIZE
              // to offset of current page in order not to find 0 offset.
              nNextPageOffset = FindNextPageOffset(offset);
              if(nNextPageOffset != (uint64)-1)
              {
                pOggPage->PageEndOffset = nNextPageOffset;
              }
              else
              {
                MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,"ParseOGGPage Unable to find Page End Offset.. Abort!!!");
                m_eParserState = OGGSTREAM_CORRUPT_DATA;
                return OGGSTREAM_CORRUPT_DATA;
              }

            }
            else
            {
              pOggPage->PageEndOffset = offset;
            }
            m_eParserState = OGGSTREAM_PAGE_CRC_ERROR;
            return OGGSTREAM_PAGE_CRC_ERROR;
          }
          m_eParserState = OGGSTREAM_READY;
          if(pOggPage->BOSFlag)
          {
            retError = ParseBOSPage(localOffset,pOggPage);
          }
          ogg_media_codec_type codec = GetTrackType(pOggPage->SerialNo);
          pOggPage->Codec = codec;

          if( (!pOggPage->BOSFlag) && (retError == OGGSTREAM_SUCCESS) )
          {
            if( (!m_nAudioStreams) && (!m_nVideoStreams) )
            {
              /*
              *As BOS flag is not set, we would have already known total number
              *of streams that exist in the file.
              *If none of those streams are classified as audio/video,
              *there is nothing in the file that can be played.
              *Just set m_bOggMetaDataParsed to true to stop parsing furhter
              *and playback would stop gracefully as there won't be any
              *audio/video track.
              */
              m_bOggMetaDataParsed = true;
              retError = OGGSTREAM_SUCCESS;
            }
            else if(!m_bOggMetaDataParsed && IsMetaDataParsingDone(pOggPage) )
            {
              m_bOggMetaDataParsed = true;
            }
#ifdef FEATURE_OGGPARSER_BUILD_SEEK_INDEX
            if( (!m_pOggIndex)         &&
                (!m_nDataPageOffset)   &&
                (pOggPage->Granule)    &&
                (!m_bOggMetaDataParsed)   )
            {
              MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"ParseOGGPage Allocating m_pOggIndex");
              m_pOggIndex = (ogg_index*)MM_Malloc(sizeof(ogg_index));
              if(m_pOggIndex)
              {
                memset(m_pOggIndex,0,sizeof(ogg_index));
                if(m_nAudioStreams)
                {
                  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
                    "ParseOGGPage Allocating m_pOggIndex->pAudioIndex m_nAudioStreams %d",m_nAudioStreams);
                  m_pOggIndex->pAudioIndex = (ogg_stream_index**)MM_Malloc(sizeof(ogg_stream_index*) * m_nAudioStreams);
                  if(m_pOggIndex->pAudioIndex)
                  {
                    m_pOggIndex->nAudioStreamsIndexed = m_nAudioStreams;
                    memset(m_pOggIndex->pAudioIndex,0,sizeof(ogg_stream_index*) * m_nAudioStreams);
                    for(int i =0;i<m_nAudioStreams;i++)
                    {
                      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"ParseOGGPage Allocating m_pOggIndex->pAudioIndex[i]");
                      m_pOggIndex->pAudioIndex[i] = (ogg_stream_index*)MM_Malloc(sizeof(ogg_stream_index));
                      if(m_pOggIndex->pAudioIndex[i])
                      {
                        memset(m_pOggIndex->pAudioIndex[i],0,sizeof(ogg_stream_index));
                        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"ParseOGGPage Allocating m_pOggIndex->pAudioIndex[i]->pPagesInfo");
                        m_pOggIndex->pAudioIndex[i]->pPagesInfo =
                          (ogg_stream_index_page_info*)MM_Malloc(sizeof(ogg_stream_index_page_info) * DEFAULT_OGG_PAGES_TO_INDEX);
                        if(m_pOggIndex->pAudioIndex[i]->pPagesInfo)
                        {
                          m_pOggIndex->pAudioIndex[i]->nPagesAllocated = DEFAULT_OGG_PAGES_TO_INDEX;
                          memset(m_pOggIndex->pAudioIndex[i]->pPagesInfo,0,
                                 sizeof(ogg_stream_index_page_info) * DEFAULT_OGG_PAGES_TO_INDEX);
                        }
                      }
                    }//for(int i =0;i<m_nAudioStreams;i++)
                  }//if(m_pOggIndex->pAudioIndex)
                }//if(m_nAudioStreams)
              }//if(m_pOggIndex) `
            }//if( (!m_pOggIndex) && (!m_nDataPageOffset) && (pOggPage->Granule) && (!m_bOggMetaDataParsed) )
#endif
          }//if( (!pOggPage->BOSFlag) && (retError == OGGSTREAM_SUCCESS) )

          if( (codec != OGG_UNKNOWN_AUDIO_VIDEO_CODEC) &&
              (!m_bOggMetaDataParsed
              || (!m_nDataPageOffset))               &&
              (!pOggPage->BOSFlag)                   &&
              (retError == OGGSTREAM_SUCCESS)
            )
          {
            switch(codec)
            {
              /*
              *If we need to parse codec headers,
              *call corresponding codec header parsing function.
              *Update the granule position for given codec
              */
              case OGG_AUDIO_CODEC_VORBIS:
              {
                bool bDataPage = true;
                if(
                    (m_pDataBuffer[localOffset] == VORBIS_COMMENT_HDR_BYTE)&&
                    (memcmp(m_pDataBuffer+localOffset+1,VORBIS_CODEC,VORBIS_CODEC_SIGN_SIZE) == 0) )
                {
                  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"ParseOGGPage Encountered VORBIS_COMMENT_HDR");
                  retError = ParseCommentHdr(localOffset,OGG_AUDIO_CODEC_VORBIS);
                  bDataPage = false;
                }//if(m_pDataBuffer[localOffset] == VORBIS_COMMENT_HDR_BYTE)

                if(
                    (m_pDataBuffer[localOffset] == VORBIS_SETUP_HDR_BYTE) &&
                    (memcmp(m_pDataBuffer+localOffset+1,VORBIS_CODEC,VORBIS_CODEC_SIGN_SIZE) == 0) )
                {
                  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"ParseOGGPage Encountered VORBIS_SETUP_HDR");
                  retError = ParseSetupHdr(localOffset,OGG_AUDIO_CODEC_VORBIS,pOggPage);
                  bDataPage = false;
                }//if(m_pDataBuffer[localOffset] == VORBIS_SETUP_HDR_BYTE)
                if ( (!m_nDataPageOffset) && (bDataPage) && (!pOggPage->ContFlag) )
                {
                  m_nDataPageOffset = pOggPage->PageOffset;
                  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
                               "UPDATED m_nDataPageOffset %llu",
                               m_nDataPageOffset);
                }
                UpdateGranulePosition(pOggPage);
              }//case OGG_AUDIO_CODEC_VORBIS:
              break;
  #ifdef FEATURE_FILESOURCE_OGG_THEORA_CODEC
              case OGG_VIDEO_CODEC_THEORA:
              {
                bool bDataPage = true;
                if(
                    (m_pDataBuffer[localOffset] == THEORA_COMMENT_HDR_BYTE)&&
                    (memcmp(m_pDataBuffer+localOffset+1,THEORA_CODEC,THEORA_CODEC_SIGN_SIZE) == 0) )
                {
                  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"ParseOGGPage Encountered THEORA_COMMENT_HDR");
                  retError = ParseCommentHdr(localOffset,OGG_VIDEO_CODEC_THEORA);
                  bDataPage = false;
                }//if(m_pDataBuffer[localOffset] == THEORA_COMMENT_HDR_BYTE)..
                if(
                    (m_pDataBuffer[localOffset] == THEORA_SETUP_HDR_BYTE)&&
                    (memcmp(m_pDataBuffer+localOffset+1,THEORA_CODEC,THEORA_CODEC_SIGN_SIZE) == 0) )
                {
                  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"ParseOGGPage Encountered THEORA_SETUP_HDR");
                  retError = ParseSetupHdr(localOffset,OGG_VIDEO_CODEC_THEORA,pOggPage);
                  bDataPage = false;
                }//if(m_pDataBuffer[localOffset] == THEORA_SETUP_HDR_BYTE)..
                if( (!m_nDataPageOffset) && (bDataPage) && (!pOggPage->ContFlag) )
                {
                  m_nDataPageOffset = pOggPage->PageOffset;
                  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,"UPDATED m_nDataPageOffset %d",m_nDataPageOffset);
                }
                UpdateGranulePosition(pOggPage);
              }
              break;
  #endif
#ifdef FEATURE_FILESOURCE_FLAC_PARSER
              case OGG_AUDIO_CODEC_FLAC:
              {
#ifdef OGG_PARSER_DEBUG
                MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"ParseOGGPage Encountered FLAC non BOS Page");
#endif
                bool bInitialParsingDone = false;
                if(m_pFlacParser)
                {
                  uint64 tmpLocalOffset = pOggPage->PageOffset + localOffset;
                  if(FLACPARSER_SUCCESS == m_pFlacParser->StartParsing(tmpLocalOffset) )
                  {
                    bInitialParsingDone = IsMetaDataParsingDone(pOggPage);
#ifdef OGG_PARSER_DEBUG
                    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"ParseOGGPage parsed FLAC non BOS Page Successfully");
#endif
                  }
                }
                if( (!m_nDataPageOffset)                           &&
                    ((m_pOggPage->Granule)||(bInitialParsingDone)) &&
                    (!pOggPage->ContFlag) )
                {
                  if(bInitialParsingDone)
                  {
                    m_nDataPageOffset = pOggPage->PageEndOffset;
                  }
                  else
                  {
                    m_nDataPageOffset = pOggPage->PageOffset;
                  }
                  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
                           "UPDATED m_nDataPageOffset %llu",m_nDataPageOffset);
                }
                UpdateGranulePosition(pOggPage);
              }
              break;
#endif

              default:
                break;
            }//switch(codec)
          }//if(codec != OGG_UNKNOWN_AUDIO_VIDEO_CODEC)&& (!m_bOggMetaDataParsed)....
        }//end of else of if(m_pDataBuffer[localOffset]!= OGG_STREAM_VERSION_FORMAT )
      }//end of else of if(memcmp(m_pDataBuffer+localOffset,OGG_PAGE_MAGIC_NUMBER,PAGE_MAGIC_NUMBER_SIZE) != 0) )
    }//end of else of if(!OGGStreamCallbakGetData (offset, nSizeNeeded, m_pDataBuffer, m_nOggDataBufferSize, (uint32) m_pUserData) )
  }//if(m_pDataBuffer && pOggPage && m_nOggDataBufferSize)
  return retError;
}
#ifdef FEATURE_OGGPARSER_BUILD_SEEK_INDEX
/* =============================================================================
FUNCTION:
 OGGStreamParser::IndexOGGPage

DESCRIPTION:
Indexes given Ogg page

INPUT/OUTPUT PARAMETERS:

RETURN VALUE:
None

SIDE EFFECTS:
  None.
=============================================================================*/
OGGStreamStatus  OGGStreamParser::IndexOGGPage(OggPage* pPage,ogg_media_codec_type codecType)
{
  OGGStreamStatus retError = OGGSTREAM_SUCCESS;
#ifdef OGG_PARSER_DEBUG
  MM_MSG_PRIO(MM_FILE_OPS,MM_PRIO_HIGH,"IndexOGGPage");
#endif
  switch(codecType)
  {
    case OGG_AUDIO_CODEC_VORBIS:
    case OGG_AUDIO_CODEC_FLAC:
    {
      retError = OGGSTREAM_INVALID_PARAM;
      //Make sure index pointer exists
      if(m_pOggIndex && m_pOggIndex->pAudioIndex && m_pOggIndex->nAudioStreamsIndexed)
      {
#ifdef OGG_PARSER_DEBUG
        MM_MSG_PRIO(MM_FILE_OPS,MM_PRIO_HIGH,"OGG_AUDIO_CODEC_VORBIS");
#endif
        int nAudioIndexToUse = -1;
        int audioInfoIndexToUse = -1;
        //Locate unused audio index pointer/index
        for(int i = 0; i < m_pOggIndex->nAudioStreamsIndexed; i++)
        {
          if( (m_pOggIndex->pAudioIndex[i])                              &&
              (m_pOggIndex->pAudioIndex[i]->bInUse)                      &&
              (m_pOggIndex->pAudioIndex[i]->pPagesInfo)                  &&
              (m_pOggIndex->pAudioIndex[i]->serialNo == pPage->SerialNo)
            )
          {
            nAudioIndexToUse = i;
#ifdef OGG_PARSER_DEBUG
            MM_MSG_PRIO(MM_FILE_OPS,MM_PRIO_HIGH,"IndexOGGPage located existing audio index pointer");
#endif
            break;
          }
          if( (m_pOggIndex->pAudioIndex[i])            &&
              (!m_pOggIndex->pAudioIndex[i]->bInUse)   &&
              (m_pOggIndex->pAudioIndex[i]->pPagesInfo)
            )
          {
            nAudioIndexToUse = i;
#ifdef OGG_PARSER_DEBUG
        MM_MSG_PRIO(MM_FILE_OPS,MM_PRIO_HIGH,"IndexOGGPage located unused audio index pointer");
#endif
            break;
          }
        }//for(int i = 0; i < m_pOggIndex->nAudioStreamsIndexed; i++)

        for(int k =0; (m_pOggAudioCodecInfo) && (k < m_nAudioStreams); k++)
        {
          if( m_pOggAudioCodecInfo[k].SerialNo == pPage->SerialNo )
          {
           audioInfoIndexToUse = k;
           break;
          }
        }

        if( (nAudioIndexToUse != -1) &&(audioInfoIndexToUse != -1) )
        {
          bool bOK = true;
          ogg_stream_index* pIndexPtr = m_pOggIndex->pAudioIndex[nAudioIndexToUse];

          pIndexPtr->bInUse = true;
          pIndexPtr->serialNo = pPage->SerialNo;
          pIndexPtr->streamCodecType = codecType;
          //Make sure there is a room to index given page
          if(pIndexPtr->nPagesIndexed >= pIndexPtr->nPagesAllocated)
          {
            MM_MSG_PRIO(MM_FILE_OPS,MM_PRIO_HIGH,"IndexOGGPage need to reallocate");
            ogg_stream_index_page_info* pTemp = (ogg_stream_index_page_info*)
              MM_Realloc(pIndexPtr->pPagesInfo,
                        sizeof(ogg_stream_index_page_info) * pIndexPtr->nPagesAllocated *2);
            if(!pTemp)
            {
              MM_MSG_PRIO(MM_FILE_OPS,MM_PRIO_FATAL,"IndexOGGPage realloc failed!!!");
              bOK = false;
              retError = OGGSTREAM_OUT_OF_MEMORY;
            }
            else
            {
              pIndexPtr->pPagesInfo = pTemp;
              pIndexPtr->nPagesAllocated = pIndexPtr->nPagesAllocated *2;
            }
          }//if(pIndexPtr->nPagesIndexed >= pIndexPtr->nPagesAllocated)
          if(bOK)
          {
            uint64 nStartGranule = 0;

            pIndexPtr->pPagesInfo[pIndexPtr->nPagesIndexed].nPageEndGranule =
              pPage->Granule;

            pIndexPtr->pPagesInfo[pIndexPtr->nPagesIndexed].nPageEndOffset =
              pPage->PageEndOffset;

            pIndexPtr->pPagesInfo[pIndexPtr->nPagesIndexed].nPageStartOffset =
                pPage->PageOffset;

            if(pIndexPtr->nPagesIndexed)
            {
              nStartGranule =  pIndexPtr->pPagesInfo[pIndexPtr->nPagesIndexed-1].nPageEndGranule;
            }
            pIndexPtr->pPagesInfo[pIndexPtr->nPagesIndexed].nTS =
            (uint64)( ( (float)nStartGranule / (float) m_pOggAudioCodecInfo[audioInfoIndexToUse].SamplingFrequency)*1000);

#ifdef OGG_PARSER_DEBUG

            MM_MSG_PRIO4(MM_FILE_OPS,MM_PRIO_HIGH,
              "Audio Index Ptr# %d Serial# %d nPagesIndex# %d nPagesAllocated# %d",
                         nAudioIndexToUse,
                         pIndexPtr->serialNo,
                         (pIndexPtr->nPagesIndexed+1),
                         pIndexPtr->nPagesAllocated);
            MM_MSG_PRIO1(MM_FILE_OPS,MM_PRIO_HIGH,"nPageStartOffset %d",
                         pIndexPtr->pPagesInfo[pIndexPtr->nPagesIndexed].nPageStartOffset);
            MM_MSG_PRIO1(MM_FILE_OPS,MM_PRIO_HIGH,"nPageEndOffset# %d",
                         pIndexPtr->pPagesInfo[pIndexPtr->nPagesIndexed].nPageEndOffset);
            MM_MSG_PRIO1(MM_FILE_OPS,MM_PRIO_HIGH,"nPageEndGranule# %d",
                         pIndexPtr->pPagesInfo[pIndexPtr->nPagesIndexed].nPageEndGranule);
            MM_MSG_PRIO1(MM_FILE_OPS,MM_PRIO_HIGH,"TS# %d",
                         pIndexPtr->pPagesInfo[pIndexPtr->nPagesIndexed].nTS);
#endif
            pIndexPtr->nPagesIndexed++;

            retError = OGGSTREAM_SUCCESS;
          }//if(bOK)
        }//if( (nIndexToUse != -1) &&(audioInfoIndexToUse != -1) )
      }//if(m_pOggIndex && m_pOggIndex->pAudioIndex && m_pOggIndex->nAudioStreamsIndexed)
    }//case OGG_AUDIO_CODEC_VORBIS:
    break;
  }//switch(codecType)
  return retError;
}
/* =============================================================================
FUNCTION:
 OGGStreamParser::FreeOGGIndexInfo

DESCRIPTION:
Frees up all the indexing structure

INPUT/OUTPUT PARAMETERS:

RETURN VALUE:
None

SIDE EFFECTS:
  None.
=============================================================================*/
void OGGStreamParser::FreeOGGIndexInfo()
{
  MM_MSG_PRIO(MM_FILE_OPS,MM_PRIO_HIGH,"FreeOGGIndexInfo");
  for(int i = 0; m_pOggIndex && i < m_pOggIndex->nAudioStreamsIndexed;i++)
  {
    if( (m_pOggIndex->pAudioIndex) && (m_pOggIndex->pAudioIndex[i]) && (m_pOggIndex->pAudioIndex[i]->pPagesInfo))
    {
      MM_Free(m_pOggIndex->pAudioIndex[i]->pPagesInfo);
    }
    MM_Free(m_pOggIndex->pAudioIndex[i]);
  }
  for(int i = 0; m_pOggIndex && i < m_pOggIndex->nVideoStreamsIndexed;i++)
  {
    if( (m_pOggIndex->pVideoIndex) && (m_pOggIndex->pVideoIndex[i]) && (m_pOggIndex->pVideoIndex[i]->pPagesInfo))
    {
      MM_Free(m_pOggIndex->pVideoIndex[i]->pPagesInfo);
    }
    MM_Free(m_pOggIndex->pVideoIndex[i]);
  }
  if(m_pOggIndex)
  {
    if(m_pOggIndex->pAudioIndex)
    {
      MM_Free(m_pOggIndex->pAudioIndex);
    }
    if(m_pOggIndex->pVideoIndex)
    {
      MM_Free(m_pOggIndex->pVideoIndex);
    }
    MM_Free(m_pOggIndex);
    m_pOggIndex = NULL;
  }
}
/*! ======================================================================
@brief  Repositions given track to specified time using OGG index

@detail    Repositions given track to specified time using OGG index built during
        parsing time.

@param[in]
 trackid: Identifies the track to be repositioned.
 nReposTime:Target time to seek to
 nCurrPlayTime: Current playback time
 sample_info: Sample Info to be filled in if seek is successful
 canSyncToNonKeyFrame: When true, video can be repositioned to non key frame
 nSyncFramesToSkip:Number of sync samples to skip. Valid only if > or < 0.

@return    OGGSTREAM_SUCCESS if successful other wise returns OGGSTREAM_FAIL
@note      None.
========================================================================== */
OGGStreamStatus OGGStreamParser::SearchOGGIndex(uint32 trackid,
                                                uint64 nReposTime,
                                                uint32 nCurrPlayTime,
                                                ogg_stream_sample_info* sample_info,
                                                bool bForward,
                                                bool canSyncToNonKeyFrame,
                                                int nSyncFramesToSkip)
{
  MM_MSG_PRIO3(MM_FILE_OPS, MM_PRIO_HIGH,"SearchOGGIndex id %d nReposTime %d nCurrPlayTime %d",trackid,nReposTime,nCurrPlayTime);
  MM_MSG_PRIO3(MM_FILE_OPS, MM_PRIO_HIGH,"SearchOGGIndex bForward %d canSyncToNonKeyFrame %d nSyncFramesToSkip %d",bForward,canSyncToNonKeyFrame,nSyncFramesToSkip);
  OGGStreamStatus retError = OGGSTREAM_FAIL;
  bool bValid = false;
  bool bAudio = false;
  uint32 trackIndex = 0;

  //Determine whether given track-id is a valid trackid
  for(uint32 i = 0; m_pOggAudioCodecInfo && (i< m_nAudioStreams);i++)
  {
    if(m_pOggAudioCodecInfo[i].SerialNo == trackid)
    {
#ifdef FEATURE_OGGPARSER_FLAC_FAST_START_UP
      if(m_pOggAudioCodecInfo[i].Audio_Codec != OGG_AUDIO_CODEC_FLAC)
#endif
      {
        bValid = true;
        bAudio = true;
        trackIndex = i;
      }
      break;
    }
  }
  for(uint32 i = 0; m_pOggVideoCodecInfo && (i< m_nVideoStreams);i++)
  {
    if( m_pOggVideoCodecInfo[i].SerialNo == trackid  )
    {
      bValid = true;
      bAudio = false;
      trackIndex = i;
      break;
    }
  }
  if(bValid && sample_info && m_pOggIndex)
  {
    //Make sure we have index table for audio tracks
    if(bAudio && m_pOggIndex->pAudioIndex)
    {
      memset(sample_info,0,sizeof(ogg_stream_sample_info));
      int nAudioIndexPtrToUse = -1;
      //Make sure we have built audio index for given trackid
      for(int i =0; i < m_pOggIndex->nAudioStreamsIndexed;i++)
      {
        if(m_pOggIndex->pAudioIndex[i]->serialNo == trackid)
        {
          nAudioIndexPtrToUse = i;
          break;
        }
      }
      if(nAudioIndexPtrToUse != -1)
      {
        //Make sure we have indexed pages for given audio trackid
        if(
            (m_pOggIndex->pAudioIndex[nAudioIndexPtrToUse])                &&
            (m_pOggIndex->pAudioIndex[nAudioIndexPtrToUse]->nPagesIndexed) &&
            (m_pOggIndex->pAudioIndex[nAudioIndexPtrToUse]->pPagesInfo)
          )
        {
          int  nPageIndexToUse = -1;
          if(bForward)
          {
            ogg_stream_index* pAIndexPtr = m_pOggIndex->pAudioIndex[nAudioIndexPtrToUse];
            for( uint32 j = 0; j <  pAIndexPtr->nPagesIndexed; j++)
            {
              //Try to locate OGG page with TS >= requested time
              if(pAIndexPtr->pPagesInfo[j].nTS >= nReposTime)
              {
                nPageIndexToUse = j;
                break;
              }
              //If we are on last indexed page,use it as long as
              //page TS is > current playback time
              if(
                  (pAIndexPtr->pPagesInfo[j].nTS <= nReposTime)&&
                  (j == (pAIndexPtr->nPagesIndexed -1))        &&
                  (pAIndexPtr->pPagesInfo[j].nTS > nCurrPlayTime)
                )
              {
                nPageIndexToUse = j;
                break;
              }
            }//for( int j = 0; j <  pAIndexPtr->nPagesIndexed; j++)
          }//if(bForward)
          else
          {
            ogg_stream_index* pAIndexPtr = m_pOggIndex->pAudioIndex[nAudioIndexPtrToUse];
            for( uint32 j = 0; j <  pAIndexPtr->nPagesIndexed; j++)
            {
              if(j == pAIndexPtr->nPagesIndexed -1)
              {
                MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"Reached to last indexed page for RW,failing the RWD...");
                break;
              }
              //Try to locate 2 consecutive ogg pages which TS satisfies following condition
              if(
                  (pAIndexPtr->pPagesInfo[j].nTS <= nReposTime) &&
                  (pAIndexPtr->pPagesInfo[j+1].nTS > nReposTime)
                )
              {
                nPageIndexToUse = j;
                break;
              }
            }//for( int j = 0; j <  pAIndexPtr->nPagesIndexed; j++)
          }//end of else of if(bForward)
          if(nPageIndexToUse != -1)
          {
            ogg_stream_index* pAIndexPtr = m_pOggIndex->pAudioIndex[nAudioIndexPtrToUse];
            retError = OGGSTREAM_SUCCESS;
            if(nPageIndexToUse)
            {
              m_nStartGranulePosn = pAIndexPtr->pPagesInfo[nPageIndexToUse-1].nPageEndGranule;
            }
            else
            {
              m_nStartGranulePosn = 0;
            }
            m_nEndGranulePosn = pAIndexPtr->pPagesInfo[nPageIndexToUse].nPageEndGranule;
            m_nCurrOffset = pAIndexPtr->pPagesInfo[nPageIndexToUse].nPageStartOffset;
            sample_info->ntime = pAIndexPtr->pPagesInfo[nPageIndexToUse].nTS;
            sample_info->bsync = 1;
            sample_info->noffset = (uint32)m_nCurrOffset;
            //Flush out m_pOggPage as we need to start parsing afresh from m_nCurrOffset;
            memset(m_pOggPage,0,sizeof(OggPage));
            MM_MSG_PRIO5(MM_FILE_OPS, MM_PRIO_HIGH,"Located indexed page for trackid %d TS %d PageOffset %d PageEndOffset %d PageEndGranule %d",
                         trackid,
                         pAIndexPtr->pPagesInfo[nPageIndexToUse].nTS,
                         pAIndexPtr->pPagesInfo[nPageIndexToUse].nPageStartOffset,
                         pAIndexPtr->pPagesInfo[nPageIndexToUse].nPageEndOffset,
                         pAIndexPtr->pPagesInfo[nPageIndexToUse].nPageEndGranule);
          }//if(nPageIndexToUse != -1)
        }//Make sure we have indexed pages for given audio trackid
      }//if(nAudioIndexPtrToUse != -1)
    }//if(bAudio && m_pOggIndex->pAudioIndex)
  }//if(bValid && sample_info && m_pOggIndex)
  return retError;
}
#endif
/* =============================================================================
FUNCTION:
 OGGStreamParser::UpdateGranulePosition

DESCRIPTION:
Updated the granule position from the current parsed ogg page

INPUT/OUTPUT PARAMETERS:

RETURN VALUE:
None

SIDE EFFECTS:
  None.
=============================================================================*/
void OGGStreamParser::UpdateGranulePosition(OggPage* pOggPage)
{
#ifdef OGG_PARSER_DEBUG
  MM_MSG_PRIO(MM_FILE_OPS,MM_PRIO_HIGH,"UpdateGranulePosition");
#endif
  if(pOggPage)
  {
    for(int i =0; (m_pOggAudioCodecInfo) && (i < m_nAudioStreams); i++)
    {
      if( m_pOggAudioCodecInfo[i].SerialNo == pOggPage->SerialNo )
      {
        m_pOggAudioCodecInfo[i].EndGranulePosn = pOggPage->Granule;

#if !defined(FEATURE_OGGPARSER_FAST_START_UP)
        if( (m_pOggAudioCodecInfo[i].SamplingFrequency) &&
            ((m_pOggAudioCodecInfo[i].Audio_Codec == OGG_AUDIO_CODEC_VORBIS)
#if !defined(FEATURE_OGGPARSER_FLAC_FAST_START_UP)
            ||
            (m_pOggAudioCodecInfo[i].Audio_Codec == OGG_AUDIO_CODEC_FLAC)
#endif
            )
          )
        {
          uint64 ts = (uint64)
                           (
                             ((float)m_pOggAudioCodecInfo[i].EndGranulePosn /
                             (float)m_pOggAudioCodecInfo[i].SamplingFrequency ) *
                             (1000)
                           );
          m_nClipDuration = (ts > m_nClipDuration)?ts:m_nClipDuration;
        }
#endif//#ifndef FEATURE_OGGPARSER_FAST_START_UP

#if defined(FEATURE_OGGPARSER_FAST_START_UP) || defined(FEATURE_OGGPARSER_FLAC_FAST_START_UP)
  #ifndef FEATURE_OGGPARSER_FAST_START_UP
        if( (m_pOggAudioCodecInfo[i].Audio_Codec == OGG_AUDIO_CODEC_FLAC) &&
            (m_pFlacParser) )
   #endif

        {
           OggPage hTempOggPage;
           memset(&hTempOggPage, 0, sizeof(OggPage));
          /** When Vorbis and FLAC arein OGG container timing information is
           present only in OGG page Granule position, even in flac StreamInfo
           number of samples will be set to 0 **/

           /* If file size is not known in advance, Parser will not calculate
              duration.*/
          if((MAX_FILE_SIZE != m_nFileSize) &&
             FindLastValidPage(pOggPage->SerialNo,m_nFileSize,&hTempOggPage) == true)
          {
              m_nClipDuration = (uint64)
                           (
                             ((float)hTempOggPage.Granule /
                             (float)m_pOggAudioCodecInfo[i].SamplingFrequency ) *
                             (1000)
                           );
          }
        }
#endif//#ifdef  FEATURE_OGGPARSER_FAST_START_UP
        break;
      }//if( m_pOggAudioCodecInfo[i].SerialNo == pOggPage->SerialNo )
    }//for(int i =0; (m_pOggAudioCodecInfo) && (i < m_nAudioStreams); i++)
    for(int i =0; (m_pOggVideoCodecInfo) && (i < m_nVideoStreams); i++)
    {
      if( m_pOggVideoCodecInfo[i].SerialNo == pOggPage->SerialNo )
      {
        m_pOggVideoCodecInfo[i].EndGranulePosn = pOggPage->Granule;
#ifndef FEATURE_OGGPARSER_FAST_START_UP
        uint64 theorats = (uint64)
          (m_pOggVideoCodecInfo[i].EndGranulePosn *
           m_pOggVideoCodecInfo[i].TimeInMsecPerFrame);
        m_nClipDuration = (theorats > m_nClipDuration)?theorats:m_nClipDuration;
#endif
        break;
      }
    }//for(int i =0; (m_pOggVideoCodecInfo) && (i < m_nVideoStreams); i++)
#ifdef OGG_PARSER_DEBUG
    MM_MSG_PRIO1(MM_FILE_OPS,MM_PRIO_HIGH,"Updated Clip Duration= %llu",
                 m_nClipDuration);
#endif
  }//if(pOggPage)
}
/* =============================================================================
FUNCTION:
 OGGStreamParser::ParseBOSPage

DESCRIPTION:
Parses OGG Page when BOS flag is set

INPUT/OUTPUT PARAMETERS:

RETURN VALUE:
OGGSTREAM_SUCCESS if successful in parsing the page otherwise, returns appropriate error.

SIDE EFFECTS:
  None.
=============================================================================*/
OGGStreamStatus OGGStreamParser::ParseBOSPage(uint32& localOffset,OggPage* pOggPage)
{
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"ParseBOSPage");
  OGGStreamStatus retError = OGGSTREAM_INVALID_PARAM;
  if(pOggPage)
  {
    retError = OGGSTREAM_SUCCESS;
    if(pOggPage->BOSFlag)
    {
      m_nstreams++;
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,"ParseBOSPage Encountered new Stream m_nstreams %d",m_nstreams);

      //Check what kind of codec exist
      if(m_pDataBuffer[localOffset] == VORBIS_IDENT_HDR_BYTE)
      {
        localOffset++;
        //Make sure VORBIS codec is specified
        if( memcmp(m_pDataBuffer+localOffset,VORBIS_CODEC,VORBIS_CODEC_SIGN_SIZE) == 0)
        {
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"ParseBOSPage Encountered VORBIS_IDENT_HDR");
          retError = ParseVorbisIdentificationHdr(localOffset,pOggPage);
        }//if( memcmp(m_pDataBuffer+localOffset,VORBIS_CODEC,VORBIS_CODEC_SIGN_SIZE) == 0)
      }//if(m_pDataBuffer[localOffset] == VORBIS_IDENT_HDR_BYTE)
  #ifdef FEATURE_FILESOURCE_OGG_THEORA_CODEC
      if(m_pDataBuffer[localOffset] == THEORA_IDENT_HDR_BYTE)
      {
        localOffset++;
        if( memcmp(m_pDataBuffer+localOffset,THEORA_CODEC,THEORA_CODEC_SIGN_SIZE) == 0)
        {
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"ParseOGGPage Encountered THEORA_IDENT_HDR");
          retError = ParseTheoraIdentificationHdr(localOffset,pOggPage);
        }//if( memcmp(m_pDataBuffer+localOffset,THEORA_CODEC,THEORA_CODEC_SIGN_SIZE) == 0)
      }//if(m_pDataBuffer[localOffset] == THEORA_IDENT_HDR_BYTE)
  #endif
#ifdef FEATURE_FILESOURCE_FLAC_PARSER
      if(m_pDataBuffer[localOffset] == FLAC_IDENT_HDR_BYTE)
      {
        localOffset++;
        if( memcmp(m_pDataBuffer+localOffset,FLAC_CODEC,FLAC_CODEC_SIGN_SIZE) == 0)
        {
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"ParseOGGPage Encountered FLAC_IDENT_HDR");
          retError = ParseFlacIdentificationHdr(localOffset,pOggPage);
        }//if( memcmp(m_pDataBuffer+localOffset,FLAC_CODEC,FLAC_CODEC_SIGN_SIZE) == 0)
      }//if(m_pDataBuffer[localOffset] == FLAC_IDENT_HDR_BYTE)
#endif

    }//if(pOggPage->BOSFlag)
  }//if(pOggPage)
  return retError;
}
/* =============================================================================
FUNCTION:
 OGGStreamParser::ParseVorbisIdentificationHdr

DESCRIPTION:
Parses VORBIS identifiction header to extract stream specific information

INPUT/OUTPUT PARAMETERS:

RETURN VALUE:
OGGSTREAM_SUCCESS if successful in parsing the header, otherwise, returns appropriate error.

SIDE EFFECTS:
  None.
=============================================================================*/
OGGStreamStatus  OGGStreamParser::ParseVorbisIdentificationHdr(uint32& localOffset,OggPage* pOggPage)
{
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"ParseVorbisIdentificationHdr");
  OGGStreamStatus retError = OGGSTREAM_INVALID_PARAM;
  if(pOggPage)
  {
    retError = OGGSTREAM_CORRUPT_DATA;
    m_nAudioStreams++;
    localOffset+= VORBIS_CODEC_SIGN_SIZE;
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"ParseVorbisIdentificationHdr Encountered VORBIS AUDIO STREAM");
    uint8 nIndexToWrite = uint8(m_nAudioStreams -1);
    bool bOK = true;
    if(!m_pOggAudioCodecInfo)
    {
      m_pOggAudioCodecInfo = (ogg_audio_info*)MM_Malloc(sizeof(ogg_audio_info));
      if(m_pOggAudioCodecInfo)
      {
        memset(m_pOggAudioCodecInfo,0,sizeof(ogg_audio_info));
      }
      else
      {
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,"ParseVorbisIdentificationHdr Malloc Fail");
        bOK = false;
      }
    }
    else if(m_pOggAudioCodecInfo)
    {
      ogg_audio_info* pTemp =
       (ogg_audio_info*)MM_Realloc(m_pOggAudioCodecInfo,m_nAudioStreams * sizeof(ogg_audio_info) );
      if(pTemp)
      {
        m_pOggAudioCodecInfo = pTemp;
        memset(m_pOggAudioCodecInfo+(m_nAudioStreams-1),0,sizeof(ogg_audio_info));
      }
      else
      {
        bOK = false;
      }
    }
    else
    {
      //Memory allocation failed, stop the parsing
      retError = OGGSTREAM_OUT_OF_MEMORY;
      m_eParserState = OGGSTREAM_OUT_OF_MEMORY;
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,"ParseVorbisIdentificationHdr Failed to Allocate memory for audio");
      bOK = false;
    }
    if(bOK)
    {
      //Skip vorbis version
      localOffset += FOURCC_SIGNATURE_BYTES;
      uint8 iden_hdr_size =
          (uint8)(pOggPage->PageEndOffset -
          (pOggPage->PageOffset + pOggPage->SegmentTableOffset + pOggPage->PageSegments));
      if(iden_hdr_size)
      {
        m_pOggAudioCodecInfo[nIndexToWrite].IdentificationHdr =
              (uint8*)MM_Malloc(iden_hdr_size);
      }
      if(m_pOggAudioCodecInfo[nIndexToWrite].IdentificationHdr)
      {
        memcpy(m_pOggAudioCodecInfo[nIndexToWrite].IdentificationHdr,
              (m_pDataBuffer+pOggPage->SegmentTableOffset + pOggPage->PageSegments),
              (iden_hdr_size));
        m_pOggAudioCodecInfo[nIndexToWrite].nIdentificationHdrSize = (uint32)iden_hdr_size;
      }
      m_pOggAudioCodecInfo[nIndexToWrite].SeqNo = pOggPage->SeqNo;
      m_pOggAudioCodecInfo[nIndexToWrite].SerialNo = pOggPage->SerialNo;
      pOggPage->Codec = OGG_AUDIO_CODEC_VORBIS;
      m_pOggAudioCodecInfo[nIndexToWrite].Audio_Codec = OGG_AUDIO_CODEC_VORBIS;
      m_pOggAudioCodecInfo[nIndexToWrite].NumberOfChannels = m_pDataBuffer[localOffset];
      localOffset++;
      memcpy(&(m_pOggAudioCodecInfo[nIndexToWrite].SamplingFrequency),
              m_pDataBuffer+localOffset,FOURCC_SIGNATURE_BYTES);
      localOffset += FOURCC_SIGNATURE_BYTES;
      memcpy(&(m_pOggAudioCodecInfo[nIndexToWrite].MaximumBitRate),
               m_pDataBuffer+localOffset,sizeof(int32));
      localOffset += FOURCC_SIGNATURE_BYTES;
      memcpy(&(m_pOggAudioCodecInfo[nIndexToWrite].NominalBitRate),
              m_pDataBuffer+localOffset,sizeof(int32));
      localOffset += FOURCC_SIGNATURE_BYTES;
      memcpy(&(m_pOggAudioCodecInfo[nIndexToWrite].MinimumBitRate),
              m_pDataBuffer+localOffset,sizeof(int32));
      localOffset += FOURCC_SIGNATURE_BYTES;
      m_pOggAudioCodecInfo[nIndexToWrite].BlockSize_0 = ( (m_pDataBuffer[localOffset]& 0xF0) >> 4);
      m_pOggAudioCodecInfo[nIndexToWrite].BlockSize_1 = m_pDataBuffer[localOffset]& 0x0f;
      localOffset++;
      m_pOggAudioCodecInfo[nIndexToWrite].FramingFlag = m_pDataBuffer[localOffset] & 0x01;
      localOffset++;
      MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_HIGH,
                   "Audio Serial# %lu Audio Seq# %lu",
                   m_pOggAudioCodecInfo[nIndexToWrite].SerialNo,
                   m_pOggAudioCodecInfo[nIndexToWrite].SeqNo);
      MM_MSG_PRIO7(MM_FILE_OPS, MM_PRIO_HIGH,
                   "#Channels %d SamplingFreq %lu Max.BitRate %ld \
                   NominalBitRate %ld Min.BitRate %ld Block_Size0 %lu \
                   BlockSize_1 %lu",
                   m_pOggAudioCodecInfo[nIndexToWrite].NumberOfChannels,
                   m_pOggAudioCodecInfo[nIndexToWrite].SamplingFrequency,
                   m_pOggAudioCodecInfo[nIndexToWrite].MaximumBitRate,
                   m_pOggAudioCodecInfo[nIndexToWrite].NominalBitRate,
                   m_pOggAudioCodecInfo[nIndexToWrite].MinimumBitRate,
                   m_pOggAudioCodecInfo[nIndexToWrite].BlockSize_0,
                   m_pOggAudioCodecInfo[nIndexToWrite].BlockSize_1 );
       retError = OGGSTREAM_SUCCESS;
    }//if(bOK)
  }//if(pOggPage)
  return retError;
}
#ifdef FEATURE_FILESOURCE_FLAC_PARSER
/* =============================================================================
FUNCTION:
 OGGStreamParser::ParseFlacIdentificationHdr

DESCRIPTION:
Parses FLAC identifiction header to extract stream specific information

INPUT/OUTPUT PARAMETERS:

RETURN VALUE:
OGGSTREAM_SUCCESS if successful in parsing the header, otherwise, returns appropriate error.

SIDE EFFECTS:
  None.
=============================================================================*/
OGGStreamStatus  OGGStreamParser::ParseFlacIdentificationHdr(uint32& localOffset,OggPage* pOggPage)
{
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"ParseFlacIdentificationHdr");
  OGGStreamStatus retError = OGGSTREAM_INVALID_PARAM;
  if(pOggPage)
  {
    retError = OGGSTREAM_CORRUPT_DATA;
    m_nAudioStreams++;
    localOffset+= FLAC_CODEC_SIGN_SIZE;
    //skip major and minor version
    localOffset+= 2;
    //! Two bytes are used to store number of headers info
    uint16 noHeaders =   uint16((m_pDataBuffer[localOffset]<<8) |
                                (m_pDataBuffer[localOffset+1]) );
    localOffset += 2;

    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,"ParseFlacIdentificationHdr Encountered FLAC AUDIO STREAM noHeaders to Follow %d",noHeaders);
    uint8 nIndexToWrite = uint8(m_nAudioStreams -1);
    bool bOK = true;
    if(!m_pOggAudioCodecInfo)
    {
      m_pOggAudioCodecInfo = (ogg_audio_info*)MM_Malloc(sizeof(ogg_audio_info));
      if(m_pOggAudioCodecInfo)
      {
        memset(m_pOggAudioCodecInfo,0,sizeof(ogg_audio_info));
      }
      else
      {
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,"ParseFlacIdentificationHdr, malloc fail");
        bOK = false;
      }
    }
    else if(m_pOggAudioCodecInfo)
    {
      ogg_audio_info* pTemp =
       (ogg_audio_info*)MM_Realloc(m_pOggAudioCodecInfo,m_nAudioStreams * sizeof(ogg_audio_info) );
      if(pTemp)
      {
        m_pOggAudioCodecInfo = pTemp;
        memset(m_pOggAudioCodecInfo+(m_nAudioStreams-1),0,sizeof(ogg_audio_info));
      }
      else
      {
        bOK = false;
      }
    }
    else
    {
      //Memory allocation failed, stop the parsing
      retError = OGGSTREAM_OUT_OF_MEMORY;
      m_eParserState = OGGSTREAM_OUT_OF_MEMORY;
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,"ParseFlacIdentificationHdr Failed to Allocate memory for audio");
      bOK = false;
    }
    if(bOK)
    {
      uint8 iden_hdr_size =
          (uint8)(pOggPage->PageEndOffset -
          (pOggPage->PageOffset + pOggPage->SegmentTableOffset + pOggPage->PageSegments));
      if(iden_hdr_size)
      {
        m_pOggAudioCodecInfo[nIndexToWrite].IdentificationHdr =
              (uint8*)MM_Malloc(iden_hdr_size);
      }
      if(m_pOggAudioCodecInfo[nIndexToWrite].IdentificationHdr)
      {
        memcpy(m_pOggAudioCodecInfo[nIndexToWrite].IdentificationHdr,
              (m_pDataBuffer+pOggPage->SegmentTableOffset + pOggPage->PageSegments),
              (iden_hdr_size));
        m_pOggAudioCodecInfo[nIndexToWrite].nIdentificationHdrSize = (uint32)iden_hdr_size;
      }
      uint64 tempoffset = pOggPage->PageOffset + localOffset;
      m_pFlacParser = MM_New_Args(FlacParser,(m_pUserData,m_nFileSize,&OGGStreamCallbakGetData));
      if( (m_pFlacParser) &&
          (m_pFlacParser->StartParsing(tempoffset) == FLACPARSER_SUCCESS) )
      {
        flac_metadata_streaminfo sInfo;
        memset(&sInfo, 0, sizeof(flac_metadata_streaminfo));
        (void)m_pFlacParser->GetFlacStreamInfo(pOggPage->SerialNo, &sInfo);
        m_pOggAudioCodecInfo[nIndexToWrite].SeqNo = pOggPage->SeqNo;
        m_pOggAudioCodecInfo[nIndexToWrite].SerialNo = pOggPage->SerialNo;
        pOggPage->Codec = OGG_AUDIO_CODEC_FLAC;
        m_pOggAudioCodecInfo[nIndexToWrite].Audio_Codec = OGG_AUDIO_CODEC_FLAC;
        m_pOggAudioCodecInfo[nIndexToWrite].SamplingFrequency = sInfo.nSamplingRate;
        m_pOggAudioCodecInfo[nIndexToWrite].NumberOfChannels  = sInfo.nChannels;
        m_pOggAudioCodecInfo[nIndexToWrite].nBitsPerSample    = sInfo.nBitsPerSample;
        retError = OGGSTREAM_SUCCESS;
        localOffset = (uint32)tempoffset;
      }
    }//if(bOK)
  }//if(pOggPage)
  return retError;
}
#endif
/* =============================================================================
FUNCTION:
 OGGStreamParser::ParseTheoraIdentificationHdr

DESCRIPTION:
Parses VORBIS identifiction header to extract stream specific information

INPUT/OUTPUT PARAMETERS:

RETURN VALUE:
OGGSTREAM_SUCCESS if successful in parsing the header, otherwise, returns appropriate error.

SIDE EFFECTS:
  None.
=============================================================================*/
OGGStreamStatus  OGGStreamParser::ParseTheoraIdentificationHdr(uint32& localOffset,OggPage* pOggPage)
{
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"ParseTheoraIdentificationHdr");
  OGGStreamStatus retError = OGGSTREAM_INVALID_PARAM;

  if(pOggPage)
  {
    retError = OGGSTREAM_CORRUPT_DATA;
  /*
   0                   1                   2                   3
   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  | 0x80          | `t'           | `h'           | `e'           |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  | `o'           | `r'           | `a'           | VMAJ          |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  | VMIN          | VREV          | FMBW                          |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  | FMBH                        | PICW...                         |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  | ...PICW       | PICH                                          |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  | PICX          | PICY          | FRN...                        |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  | ...FRN                        | FRD...                        |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  | ...FRD                        | PARN...                       |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  | ...PARN       | PARD                                          |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  | CS            | NOMBR                                         |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  | QUAL      | KFGSHIFT| PF| Res |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  */
    m_nVideoStreams++;
    bool bOK = true;
    localOffset+= THEORA_CODEC_SIGN_SIZE;
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"ParseTheoraIdentificationHdr Encountered THEORA VIDEO STREAM");

    uint8 nIndexToWrite = uint8(m_nVideoStreams -1);
    if(!m_pOggVideoCodecInfo)
    {
      m_pOggVideoCodecInfo = (ogg_video_info*)MM_Malloc(sizeof(ogg_video_info));
      if(m_pOggVideoCodecInfo)
      {
        memset(m_pOggVideoCodecInfo,0,sizeof(ogg_video_info));
      }
      else
      {
        bOK = false;
        retError = OGGSTREAM_OUT_OF_MEMORY;
      }
    }
    else if(m_pOggVideoCodecInfo)
    {
      ogg_video_info* pTemp =
        (ogg_video_info*)MM_Realloc(m_pOggVideoCodecInfo,m_nVideoStreams * sizeof(ogg_video_info) );
      if(pTemp)
      {
        m_pOggVideoCodecInfo = pTemp;
        memset(m_pOggVideoCodecInfo+(m_nVideoStreams-1),0,sizeof(ogg_video_info));
      }
      else
      {
        bOK = false;
        retError = OGGSTREAM_OUT_OF_MEMORY;
      }
    }
    else
    {
      //Memory allocation failed, stop the parsing
      retError = OGGSTREAM_OUT_OF_MEMORY;
      m_eParserState = OGGSTREAM_OUT_OF_MEMORY;
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,"ParseTheoraIdentificationHdr Failed to Allocate memory for video");
      bOK = false;
    }
    if(bOK)
    {
      uint8 iden_hdr_size =
      (uint8)(pOggPage->PageEndOffset -
      (pOggPage->PageOffset + pOggPage->SegmentTableOffset + pOggPage->PageSegments));


      if(iden_hdr_size)
      {
        m_pOggVideoCodecInfo[nIndexToWrite].IdentificationHdr =
          (uint8*)MM_Malloc(iden_hdr_size);
      }
      if(m_pOggVideoCodecInfo[nIndexToWrite].IdentificationHdr)
      {
        memcpy(m_pOggVideoCodecInfo[nIndexToWrite].IdentificationHdr,
               (m_pDataBuffer+pOggPage->SegmentTableOffset + pOggPage->PageSegments),
               (iden_hdr_size));

        m_pOggVideoCodecInfo[nIndexToWrite].nIdentificationHdrSize = (uint32)iden_hdr_size;
      }

      m_pOggVideoCodecInfo[nIndexToWrite].Vmaj = m_pDataBuffer[localOffset++];
      m_pOggVideoCodecInfo[nIndexToWrite].Vmin = m_pDataBuffer[localOffset++];
      m_pOggVideoCodecInfo[nIndexToWrite].Vrev = m_pDataBuffer[localOffset++];

      m_pOggVideoCodecInfo[nIndexToWrite].FmbW = (uint16)
      ( (m_pDataBuffer[localOffset] <<8 ) |
        (m_pDataBuffer[localOffset+1]) );
      localOffset += 2;

      m_pOggVideoCodecInfo[nIndexToWrite].FmbH = (uint16)
      ( (m_pDataBuffer[localOffset] <<8) |
        (m_pDataBuffer[localOffset+1] ));
      localOffset += 2;

      m_pOggVideoCodecInfo[nIndexToWrite].Width = (uint32)
        ((m_pDataBuffer[localOffset]<<16) |
         (m_pDataBuffer[localOffset+1]<<8) |
         (m_pDataBuffer[localOffset+2]) );
      localOffset += 3;

      m_pOggVideoCodecInfo[nIndexToWrite].Height = (uint32)
        ( (m_pDataBuffer[localOffset]<<16)  |
          (m_pDataBuffer[localOffset+1]<<8) |
          (m_pDataBuffer[localOffset+2]));
      localOffset += 3;

      m_pOggVideoCodecInfo[nIndexToWrite].PicX = m_pDataBuffer[localOffset++];
      m_pOggVideoCodecInfo[nIndexToWrite].PicY = m_pDataBuffer[localOffset++];

      uint32 frn = (uint32)((m_pDataBuffer[localOffset]<<24)  |
                            (m_pDataBuffer[localOffset+1]<<16)|
                            (m_pDataBuffer[localOffset+2]<<8) |
                            (m_pDataBuffer[localOffset+3]) );
      localOffset += 4;

      uint32 frd = (uint32)((m_pDataBuffer[localOffset]<<24)  |
                            (m_pDataBuffer[localOffset+1]<<16)|
                            (m_pDataBuffer[localOffset+2]<<8) |
                            (m_pDataBuffer[localOffset+3]) );
      localOffset += 4;
      if(frd && frn)
      {
        m_pOggVideoCodecInfo[nIndexToWrite].FrameRate =
          (float)frn/(float)frd;
        m_pOggVideoCodecInfo[nIndexToWrite].TimeInMsecPerFrame =
          (uint32)(1000.0/(float)frn);
      }
      m_pOggVideoCodecInfo[nIndexToWrite].AspectRatio_Nmr = (uint32)
                                  ((m_pDataBuffer[localOffset]<<16) |
                                   (m_pDataBuffer[localOffset+1]<<8)|
                                   (m_pDataBuffer[localOffset+2]) );
      localOffset += 3;

      m_pOggVideoCodecInfo[nIndexToWrite].AspectRatio_Dmr =(uint32)
                                  ((m_pDataBuffer[localOffset]<<16) |
                                   (m_pDataBuffer[localOffset+1]<<8)|
                                   (m_pDataBuffer[localOffset+2]) );
      localOffset += 3;

      m_pOggVideoCodecInfo[nIndexToWrite].CS = m_pDataBuffer[localOffset++];

      m_pOggVideoCodecInfo[nIndexToWrite].BitRate =(uint32)
                                  ((m_pDataBuffer[localOffset]<<16) |
                                   (m_pDataBuffer[localOffset+1]<<8)|
                                   (m_pDataBuffer[localOffset+2]) );
      localOffset += 3;

      m_pOggVideoCodecInfo[nIndexToWrite].QUAL = (uint8)
      ((m_pDataBuffer[localOffset] & 0xFC)>>2);

      m_pOggVideoCodecInfo[nIndexToWrite].Kfgshift = uint8
      (((m_pDataBuffer[localOffset]  & 0x03)<<3)|
      ((m_pDataBuffer[localOffset+1]& 0xE0)>>5) );

      m_pOggVideoCodecInfo[nIndexToWrite].PF = uint8
        ((m_pDataBuffer[localOffset+1] & 0x18)>>3);
      localOffset+= 1;

      pOggPage->Codec = OGG_VIDEO_CODEC_THEORA;
      m_pOggVideoCodecInfo[nIndexToWrite].SeqNo = pOggPage->SeqNo;
      m_pOggVideoCodecInfo[nIndexToWrite].SerialNo = pOggPage->SerialNo;
      m_pOggVideoCodecInfo[nIndexToWrite].Video_Codec = OGG_VIDEO_CODEC_THEORA;

      MM_MSG_PRIO6(MM_FILE_OPS, MM_PRIO_HIGH,
     "Width %lu Height %lu BitRate %lu FrameRate %f Aspect Rt.%lu / %lu",
      m_pOggVideoCodecInfo[nIndexToWrite].Width,
      m_pOggVideoCodecInfo[nIndexToWrite].Height,
      m_pOggVideoCodecInfo[nIndexToWrite].BitRate,
      m_pOggVideoCodecInfo[nIndexToWrite].FrameRate,
      m_pOggVideoCodecInfo[nIndexToWrite].AspectRatio_Nmr,
      m_pOggVideoCodecInfo[nIndexToWrite].AspectRatio_Dmr);
      retError = OGGSTREAM_SUCCESS;
    }//if(bOK)
  }//if(pOggPage)
  return retError;
}
/* =============================================================================
FUNCTION:
 OGGStreamParser::ParseSetupHdr

DESCRIPTION:
Parses SetUp header for given codec

INPUT/OUTPUT PARAMETERS:

RETURN VALUE:
OGGSTREAM_SUCCESS if successful in parsing the header, otherwise, returns appropriate error.

SIDE EFFECTS:
  None.
=============================================================================*/
OGGStreamStatus   OGGStreamParser::ParseSetupHdr(uint32& localOffset,
                                                 ogg_media_codec_type codec,
                                                 OggPage* pOggPage)
{
  OGGStreamStatus retError = OGGSTREAM_INVALID_PARAM;
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,"ParseSetupHdr");
  if(pOggPage)
  {
    //pOggPage contains start of setup header,
    //hNextOggPage is used to parse next ogg page
    //to see if setup header is spanning across ogg pages
    OggPage hNextOggPage;
    memset(&hNextOggPage,0,sizeof(OggPage));

    retError = OGGSTREAM_PARSE_ERROR;
    uint32 setup_hdr_size = (uint32)(pOggPage->PageEndOffset - (pOggPage->PageOffset + localOffset) );
    #ifdef OGG_PARSER_DEBUG
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,"ParseSetupHdr setup_hdr_size %lu",
                 setup_hdr_size);
    #endif
    if(m_nAudioStreams && setup_hdr_size && (codec == OGG_AUDIO_CODEC_VORBIS) )
    {
      if( m_pOggAudioCodecInfo[m_nAudioStreams-1].SetupHdr)
      {
        MM_Free( m_pOggAudioCodecInfo[m_nAudioStreams-1].SetupHdr);
        m_pOggAudioCodecInfo[m_nAudioStreams-1].nSetupHdrSize = 0;
      }
      m_pOggAudioCodecInfo[m_nAudioStreams-1].SetupHdr =
            (uint8*)MM_Malloc(setup_hdr_size);
      if( m_pOggAudioCodecInfo[m_nAudioStreams-1].SetupHdr)
      {
        memcpy(m_pOggAudioCodecInfo[m_nAudioStreams-1].SetupHdr,
               m_pDataBuffer+localOffset, setup_hdr_size);
        m_pOggAudioCodecInfo[m_nAudioStreams-1].nSetupHdrSize = setup_hdr_size;
      }
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW,
                "ParseSetupHdr Stored vorbis setup header %lu",setup_hdr_size);
      retError = OGGSTREAM_SUCCESS;

      //Setup header can span multiple ogg pages, so need keep looping untill hNextOggPage.ContFlag is true
      while( (m_nAudioStreams) || (m_nVideoStreams) )
      {
        //Now parse the next OGG page to see if it has setup header
        if(ParseOGGPage(pOggPage->PageEndOffset,&hNextOggPage) == OGGSTREAM_SUCCESS)
        {
          if(hNextOggPage.ContFlag)
          {
            //Setup header continues...
            retError = OGGSTREAM_CORRUPT_DATA;
            if(m_eParserState == OGGSTREAM_PAGE_CRC_ERROR)
            {
              //This is not a fool proof check. If there is a CRC error, ContFlag in next Page
              //might have been set by the error in bitstream. The Setup header might be self
              //contained in the Page we had. but we can't rely on that, hence the check.
              m_eParserState = OGGSTREAM_CORRUPT_DATA;
              return retError;
            }

            MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"Vorbis SETUP header begins and ends on different OGG page");
            uint64 nEndSegTableOffset =  hNextOggPage.SegmentTableOffset + hNextOggPage.PageSegments;
            //Convert nEndSegTableOffset to absolute file offset
            nEndSegTableOffset += hNextOggPage.PageOffset;
            //Get additional setup header size from hNextOggPage
            uint32 addsetupsize = (uint32)(hNextOggPage.PageEndOffset - nEndSegTableOffset);
            MM_MSG_PRIO3(MM_FILE_OPS, MM_PRIO_HIGH,
                         "Absolute nEndSegTableOffset is %llu hNextOggPage.PageOffset %llu addsetupsize %lu",
                         nEndSegTableOffset,hNextOggPage.PageOffset,addsetupsize);
            MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
                         "ParseSetupHdr New setup_hdr_size %lu",(setup_hdr_size+addsetupsize));
            //ReAllocate memory and copy setup header from both pages
            uint8* pTempPtr = (uint8*)MM_Realloc
            (m_pOggAudioCodecInfo[m_nAudioStreams-1].SetupHdr,(setup_hdr_size+addsetupsize));
            if(pTempPtr)
            {
              setup_hdr_size += addsetupsize;
              m_pOggAudioCodecInfo[m_nAudioStreams-1].SetupHdr = pTempPtr;
              memcpy(m_pOggAudioCodecInfo[m_nAudioStreams-1].SetupHdr+ (setup_hdr_size-addsetupsize),
                     m_pDataBuffer+hNextOggPage.SegmentTableOffset+hNextOggPage.PageSegments,
                     addsetupsize);
              m_pOggAudioCodecInfo[m_nAudioStreams-1].nSetupHdrSize = setup_hdr_size;
              retError = OGGSTREAM_SUCCESS;
              MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
                  "ParseSetupHdr Stored vorbis setup header after realloc %lu",
                  setup_hdr_size);
              //update the offset to point to next ogg page
              pOggPage->PageEndOffset = hNextOggPage.PageEndOffset;
            }//if(pTempPtr)
            else
            {
              retError = OGGSTREAM_OUT_OF_MEMORY;
              m_eParserState = OGGSTREAM_OUT_OF_MEMORY;
              MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_FATAL,
                           "ParseSetupHdr ReAlloc failed for %lu",
                           (setup_hdr_size+addsetupsize));
            }
          }//if(hNextOggPage.ContFlag)
          else
          {
            //end of setup header reached..break out of parsing loop
            break;
          }
        }//if(ParseOGGPage(pOggPage->PageEndOffset,&hNextOggPage) == OGGSTREAM_SUCCESS)
        else //If Page parsing is not successful, then break the loop
        {
          break;
        }
      }
    }//if(m_nAudioStreams && setup_hdr_size && (codec == OGG_AUDIO_CODEC_VORBIS) )
#ifdef FEATURE_FILESOURCE_OGG_THEORA_CODEC
    if(m_nVideoStreams && setup_hdr_size && (codec == OGG_VIDEO_CODEC_THEORA) )
    {
      m_pOggVideoCodecInfo[m_nVideoStreams-1].SetupHdr =
          (uint8*)MM_Malloc(setup_hdr_size);
      memcpy(m_pOggVideoCodecInfo[m_nVideoStreams-1].SetupHdr,
             m_pDataBuffer+localOffset, setup_hdr_size);
             m_pOggVideoCodecInfo[m_nVideoStreams-1].nSetupHdrSize = setup_hdr_size;
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,"ParseSetupHdr Stored video setup header %d",setup_hdr_size);
      retError = OGGSTREAM_SUCCESS;
    }//if(m_nVideoStreams && setup_hdr_size && (codec == OGG_VIDEO_CODEC_THEORA) )
#endif
  }//if(pOggPage)
  return retError;
}
/* =============================================================================
FUNCTION:
 OGGStreamParser::ParseCommentHdr

DESCRIPTION:
Parses Comment header for given codec type

INPUT/OUTPUT PARAMETERS:

RETURN VALUE:
OGGSTREAM_SUCCESS if successful in parsing the header, otherwise, returns appropriate error.

SIDE EFFECTS:
  None.
=============================================================================*/
OGGStreamStatus OGGStreamParser::ParseCommentHdr(uint32& ulOffset,
                                                 ogg_media_codec_type codec)
{
  OGGStreamStatus retError = OGGSTREAM_PARSE_ERROR;

  if( codec != OGG_UNKNOWN_AUDIO_VIDEO_CODEC )
  {
    retError = OGGSTREAM_SUCCESS;
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM, "ParseCommentHdr @ offset %lu",
                 ulOffset);
    uint32 ulCmtHdrStart = ulOffset;
    uint32 ulVendorStrLen = 0;
    uint32 ulBufIndex = ulOffset + 1;

    if(codec == OGG_AUDIO_CODEC_VORBIS)
    {
      ulBufIndex+= VORBIS_CODEC_SIGN_SIZE;
    }
    if(codec == OGG_VIDEO_CODEC_THEORA)
    {
      ulBufIndex+= THEORA_CODEC_SIGN_SIZE;
    }
    /* Comment data structure
       No. of bits        Description
          32            Vendor string len (Comment generated tool info)
          n*8           Vendor string (Comment generated tool info)
          32            Total number of comment fields
          iterate [Total fields] times
          32            Comment field length
          n*8           Comment field string (UTF8)
       Comment field string is defined as below:
       TITLE=CLIP_NAME.
       Now Parser has to look for '=' and use the data after '=' as metadata and
       data before '=' as metadata type.
    */
    //! Get Vendor string length and skip it
    memcpy(&ulVendorStrLen, m_pDataBuffer+ulBufIndex, 4 );
    ulBufIndex += 4 + ulVendorStrLen;

    uint32 ulTotalComments      = 0;
    uint32 ulCmtListStartOffset = ulBufIndex;
    memcpy(&ulTotalComments, m_pDataBuffer+ ulBufIndex, 4 );
    ulBufIndex += 4;

    //! Allocate memory to metadata structure
    if( (ulTotalComments)&& (!m_pOggMetaData) )
    {
      m_pOggMetaData = (ogg_meta_data*)
          MM_Malloc( ulTotalComments * sizeof(ogg_meta_data) );
      if(m_pOggMetaData)
      {
        memset(m_pOggMetaData,0,ulTotalComments * sizeof(ogg_meta_data) );
        m_nMetaData = (int)ulTotalComments;
      }
    }

    uint32  ulCommentLen = 0;
    uint32  ulTotalCommentStrLen = (ulTotalComments * 4);

    //! Run the loop till all the entries are parsed
    for(int nMetaInd = 0 ; nMetaInd< (int)ulTotalComments;nMetaInd++)
    {
      memcpy(&ulCommentLen, m_pDataBuffer + ulBufIndex, 4 );
      ulBufIndex += 4;

      if( (ulCommentLen)&& (m_pOggMetaData) )
      {
        uint8* pucTempPtr = (uint8*)MM_Malloc(ulCommentLen + 1);
        if(pucTempPtr)
        {
          memcpy(pucTempPtr,m_pDataBuffer + ulBufIndex, ulCommentLen);
          pucTempPtr[ulCommentLen]='\0';
          //! Look for the data after '=' (actual metadata that has to be used)
          uint8* pucActualDataPtr = (uint8*)strstr((const char*)pucTempPtr,"=");
          if(pucActualDataPtr)
          {
            int usMetaDataNameStrLen = int(pucActualDataPtr - pucTempPtr);
            uint16 usStdMetaEntryIdx = 0;
            //! Convert Metadata Name only to Upper Case
            ConvertToUpperCase(pucTempPtr, usMetaDataNameStrLen);
            while(usStdMetaEntryIdx < MAX_FIELDNAMES_SUPPORTED)
            {
              uint8* pucStdFieldString = (uint8*)&OGGFieldNames[usStdMetaEntryIdx][0];
              if(
                 (strlen((char*)pucStdFieldString) ==
                  (size_t)usMetaDataNameStrLen)&&
                 (!strncmp((char*)pucStdFieldString,(char*)pucTempPtr,
                           usMetaDataNameStrLen) ) )
              {
                //We don't want '=' from index1, which makes room for \0
                uint32 ulActualMetaDataLen= ulCommentLen - usMetaDataNameStrLen;
                if(ulActualMetaDataLen)
                {
                  m_pOggMetaData[nMetaInd].pMetaData =
                    (uint8*)MM_Malloc( ulActualMetaDataLen);
                  if(m_pOggMetaData[nMetaInd].pMetaData)
                  {
                    //! Ignore first character "="
                    memcpy(m_pOggMetaData[nMetaInd].pMetaData,
                           pucActualDataPtr + 1,
                           (ulActualMetaDataLen-1));
                    m_pOggMetaData[nMetaInd].pMetaData[ulActualMetaDataLen-1]='\0';
                    m_pOggMetaData[nMetaInd].nMetaDataFieldIndex = (uint16)usStdMetaEntryIdx;
                    m_pOggMetaData[nMetaInd].nMetaDataLength = (ulActualMetaDataLen-1);
                    m_pOggMetaData[nMetaInd].bAvailable = true;
                    break;
                  }//if(m_pOggMetaData[i].pMetaData)
                }//if(ulActualMetaDataLen)
              }//if(strlen((const char*)fname) == fieldnamelength)
              //! Increment counter
              usStdMetaEntryIdx++;
            }//while(usStdMetaEntriesIndex < MAX_FIELDNAMES_SUPPORTED)
          }//if(pucTempPtr)
          MM_Free(pucTempPtr);
        }//if(tempptr)
        ulBufIndex += ulCommentLen;
      }//if(user_comment_length)
      ulTotalCommentStrLen+= ulCommentLen ;
    }//for(uint32 ulMetaInd = 0 ; ulMetaInd< ulTotalComments;ulMetaInd++)
    //! Calculate Comment String end offset
    uint32 ulCmtEndOffset = ulCmtListStartOffset + 4 +  ulTotalCommentStrLen;

    //Add one for frame-bit
    if(codec == OGG_AUDIO_CODEC_VORBIS)
    {
       ulCmtEndOffset++;
    }
    uint32 ulCommentHdrSize = ulCmtEndOffset - ulCmtHdrStart;
    ulOffset = ulCmtEndOffset;


    //Possibility of some junk data in between the Comment header and Setup header,
    //So escape those bytes and update local offset.
    //Current page should have SETUP HDR byte and VORBIS CODEC Byte, so search for
    //SETUP HDR till current page end offset - VORBIS_CODEC_SIGN_SIZE -1 .

     for( ; ((( m_pOggPage->PageEndOffset - VORBIS_CODEC_SIGN_SIZE -1) >= ulOffset) &&
              (m_pDataBuffer[ulOffset] != VORBIS_SETUP_HDR_BYTE)); ulOffset++
         );

    MM_MSG_PRIO3(MM_FILE_OPS, MM_PRIO_LOW,
    "COMMENT_HDR vendor_length %lu user_comment_list_length %lu total_user_comment_length %lu",
    ulVendorStrLen,ulTotalComments,ulTotalCommentStrLen);

    MM_MSG_PRIO3(MM_FILE_OPS, MM_PRIO_LOW,
    "COMMENT_HDR comment_hdr_start_offset %lu comment_hdr_end_offset %lu comment_hdr_size %lu",
    ulCmtHdrStart,ulCmtEndOffset,ulCommentHdrSize);

    if( (m_nAudioStreams) &&
        (ulCommentHdrSize) &&
        (codec == OGG_AUDIO_CODEC_VORBIS) )
    {
      m_pOggAudioCodecInfo[m_nAudioStreams-1].CommentHdr =
      (uint8*)MM_Malloc(ulCommentHdrSize);
       memcpy(m_pOggAudioCodecInfo[m_nAudioStreams-1].CommentHdr,
              m_pDataBuffer+ulCmtHdrStart,
              ulCommentHdrSize);
      m_pOggAudioCodecInfo[m_nAudioStreams-1].nCommentHdrSize = ulCommentHdrSize;
    }
    if( (m_nVideoStreams) &&
        (ulCommentHdrSize) &&
        (codec == OGG_VIDEO_CODEC_THEORA) )
    {
      m_pOggVideoCodecInfo[m_nVideoStreams-1].CommentHdr =
      (uint8*)MM_Malloc(ulCommentHdrSize);
       memcpy(m_pOggVideoCodecInfo[m_nVideoStreams-1].CommentHdr,
              m_pDataBuffer+ulCmtHdrStart,
              ulCommentHdrSize);
      m_pOggVideoCodecInfo[m_nVideoStreams-1].nCommentHdrSize = ulCommentHdrSize;
    }
  }
  return retError;
}
/* =============================================================================
FUNCTION:
 OGGStreamParser::GetClipMetaData

DESCRIPTION:
Returns the clip meta data identified via nIndex

INPUT/OUTPUT PARAMETERS:

RETURN VALUE:
OGGSTREAM_SUCCESS if successful in parsing the header, otherwise, returns appropriate error.

SIDE EFFECTS:
  None.
=============================================================================*/
void OGGStreamParser::GetClipMetaData(int nMetaIndex,
                                      uint8* pMetadataValue,
                                      uint32* pMetadataLength)
{
  if(pMetadataLength)
  {
    //Count how many items are matching given index and total length
    //for all of such matching index values
    uint32 nCount = 0;
    int nMatched = 0;
    for(int nIndex =0; nIndex < (int)m_nMetaData; nIndex++)
    {
      if( (m_pOggMetaData[nIndex].nMetaDataFieldIndex == nMetaIndex) &&
          (m_pOggMetaData[nIndex].pMetaData) )
      {
        nCount += m_pOggMetaData[nIndex].nMetaDataLength;
        nMatched++;
      }
    }
    //In case of multiple match, we separate them by ~
    //Add one for '\0'
    if(nMatched > 1)
    {
      nCount = nCount + (nMatched * (int)sizeof("~")) + 1;
    }
    else
    {
      nCount++;
    }
    if(!pMetadataValue)
    {
      *pMetadataLength = nCount;
    }
    else
    {
      int nToCopy = nMatched;
      //We have sufficient buffer, start copying
      if(*pMetadataLength >= nCount)
      {
        uint32 ulWriteIndex = 0;
        for(int nIndex =0; nIndex < (int)m_nMetaData; nIndex++)
        {
          if( (m_pOggMetaData[nIndex].nMetaDataFieldIndex == nMetaIndex) &&
              (m_pOggMetaData[nIndex].pMetaData) )
          {
            memcpy(pMetadataValue+ulWriteIndex,
                   m_pOggMetaData[nIndex].pMetaData,
                   m_pOggMetaData[nIndex].nMetaDataLength);
            ulWriteIndex+=m_pOggMetaData[nIndex].nMetaDataLength;
            if(nToCopy >1)
            {
              memcpy(pMetadataValue+ulWriteIndex,"~",1);
              ulWriteIndex++;
              nToCopy--;
            }
          }
        }
        pMetadataValue[ulWriteIndex] ='\0';
      }//if(*pMetadataLength >= nCount)
    }//end of else of if(!pMetadataValue)
  }//if(pMetadataLength)
}

/* =============================================================================
FUNCTION:
 OGGStreamParser::GetTrackWholeIDList

DESCRIPTION:
Returns trackId list for all the tracks in given clip.

INPUT/OUTPUT PARAMETERS:

RETURN VALUE:

SIDE EFFECTS:
  None.
=============================================================================*/
uint32 OGGStreamParser::GetTrackWholeIDList(uint32* idList)
{
  #ifdef OGG_PARSER_DEBUG
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"GetTrackWholeIDList");
  #endif
  if(!idList)
  {
    return 0;
  }
  int nCount = 0;
  /* Instead of using Serial Num as track ID, allocate new trackId value.
     Map this trackId value with Serial Number inside OGGStreamParser object
     to process any request of OGGStream class. */
  for ( ; m_pOggVideoCodecInfo && (nCount < m_nVideoStreams); nCount++)
  {
    (*idList) = nCount;
    m_pOggVideoCodecInfo[nCount].TrackId = nCount;
    idList++;
  }
  nCount = 0;
  for (nCount = 0; m_pOggAudioCodecInfo && (nCount < m_nAudioStreams); nCount++)
  {
    (*idList) = m_nVideoStreams + nCount;
    m_pOggAudioCodecInfo[nCount].TrackId = nCount + m_nVideoStreams;
    idList++;
  }
  return GetTotalNumberOfTracks();
}
/* =============================================================================
FUNCTION:
 OGGStreamParser::GetTrackSerialNo

DESCRIPTION:
Returns track serial number for given track id

INPUT/OUTPUT PARAMETERS:

RETURN VALUE:
 Success if track id is valid otherwise, returns invalid parameter.
SIDE EFFECTS:
  None.
=============================================================================*/
uint32 OGGStreamParser::GetTrackSerialNo(uint32 id)
{
#ifdef OGG_PARSER_DEBUG
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"GetTrackSerialNo");
#endif
  uint32 ulSerialNum = 0;
  for(int i =0; m_pOggAudioCodecInfo && (i < m_nAudioStreams);i++)
  {
    if(m_pOggAudioCodecInfo[i].TrackId == id)
    {
      ulSerialNum = m_pOggAudioCodecInfo[i].SerialNo;
      break;
    }
  }
  for(int i =0; m_pOggVideoCodecInfo && (i < m_nVideoStreams);i++)
  {
    if(m_pOggVideoCodecInfo[i].TrackId == id)
    {
      ulSerialNum = m_pOggVideoCodecInfo[i].SerialNo;
      break;
    }
  }
  return ulSerialNum;
}
/* =============================================================================
FUNCTION:
 OGGStreamParser::GetTrackType

DESCRIPTION:
Returns track type for given track id

INPUT/OUTPUT PARAMETERS:

RETURN VALUE:
 Success if track id is valid otherwise, returns invalid parameter.
SIDE EFFECTS:
  None.
=============================================================================*/
ogg_media_codec_type OGGStreamParser::GetTrackType(uint32 id)
{
#ifdef OGG_PARSER_DEBUG
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"GetTrackType");
#endif
  ogg_media_codec_type codec =  OGG_UNKNOWN_AUDIO_VIDEO_CODEC;
  for(int i =0; m_pOggAudioCodecInfo && (i < m_nAudioStreams);i++)
  {
    if(m_pOggAudioCodecInfo[i].SerialNo == id)
    {
      codec = m_pOggAudioCodecInfo[i].Audio_Codec;
      break;
    }
  }
  for(int i =0; m_pOggVideoCodecInfo && (i < m_nVideoStreams);i++)
  {
    if(m_pOggVideoCodecInfo[i].SerialNo == id)
    {
      codec = m_pOggVideoCodecInfo[i].Video_Codec;
      break;
    }
  }
  return codec;
}
/* =============================================================================
FUNCTION:
  OGGStreamParser::GetClipDurationInMsec

DESCRIPTION:
 Returns clip duration from the current parsed data.

INPUT/OUTPUT PARAMETERS:

RETURN VALUE:

SIDE EFFECTS:
  None.
=============================================================================*/
uint64 OGGStreamParser::GetClipDurationInMsec()
{
  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
    "OGGParser:GetClipDurationInMsec %llu", m_nClipDuration);
  return m_nClipDuration;
}
/* =============================================================================
FUNCTION:
 OGGStreamParser::GetCurrentSample

DESCRIPTION:
Returns current sample for the given trackId.

INPUT/OUTPUT PARAMETERS:

RETURN VALUE:

SIDE EFFECTS:
  None.
=============================================================================*/
OGGStreamStatus OGGStreamParser::GetCurrentSample(uint32 trackId,uint8* dataBuffer,
                                                  uint32 nMaxBufSize, int32* nBytesRead)
{
  OGGStreamStatus retError = OGGSTREAM_DEFAULT_ERROR;
#ifdef OGG_PARSER_DEBUG
  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,"GetCurrentSample m_nCurrOffset %llu",
               m_nCurrOffset);
#endif
  bool bContinue = false;
  bool bAudio = false;
  bool bVideo = false;
  bool bSentCodecHeaders = false;
  int nIndex = 0;
  ogg_media_codec_type codec = GetTrackType(trackId);
  if(nBytesRead)
  {
    *nBytesRead = 0;
    bContinue = true;
  }
  if(m_bParsedLastPage && m_pOggPage->nPageSegmentsTransmitted == m_pOggPage->PageSegments)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"GetCurrentSample encountered OGGSTREAM_EOF");
    retError = OGGSTREAM_EOF;
    m_eParserState = OGGSTREAM_EOF;
  }
  if(
         (bContinue)                        &&
         (m_eParserState == OGGSTREAM_READY)
       )
  {
    *nBytesRead = 0;
    memset(dataBuffer,0,nMaxBufSize);
    bContinue = false;
#ifdef FEATURE_FILESOURCE_OGG_THEORA_CODEC
    if(codec == OGG_VIDEO_CODEC_THEORA)
    {
      for(int i =0; i< m_nVideoStreams;i++)
      {
        if(m_pOggVideoCodecInfo[i].SerialNo == trackId)
        {
          nIndex = i;
          bContinue = true;
          bVideo = true;
          break;
        }
      }
    }//if(codec == OGG_VIDEO_CODEC_THEORA)
#endif
    if(
        (codec == OGG_AUDIO_CODEC_VORBIS)||
        (codec == OGG_AUDIO_CODEC_FLAC) )
    {
      for(int i =0; i< m_nAudioStreams;i++)
      {
        if(m_pOggAudioCodecInfo[i].SerialNo == trackId)
        {
          nIndex = i;
          bContinue = true;
          bAudio = true;
          break;
        }
      }
    }//if(  (codec == OGG_AUDIO_CODEC_VORBIS)||...

    if(bContinue && bAudio)
    {
      if( (m_pOggAudioCodecInfo[nIndex].IdentificationHdr)     &&
          (!m_pOggAudioCodecInfo[nIndex].bSentIdentificationHdr)  )
      {

        if(m_pOggAudioCodecInfo[nIndex].Audio_Codec == OGG_AUDIO_CODEC_VORBIS)
        {
          memcpy(dataBuffer,
                 m_pOggAudioCodecInfo[nIndex].IdentificationHdr,
                 m_pOggAudioCodecInfo[nIndex].nIdentificationHdrSize);
          *nBytesRead =  m_pOggAudioCodecInfo[nIndex].nIdentificationHdrSize;
        }
#ifdef FEATURE_FILESOURCE_FLAC_PARSER
        if( (m_pOggAudioCodecInfo[nIndex].Audio_Codec == OGG_AUDIO_CODEC_FLAC)&&
            (m_pFlacParser) )
        {
          uint32 size = m_pFlacParser->GetCodecHeaderSize(m_pOggAudioCodecInfo[nIndex].SerialNo);
          if(size)
          {
            memcpy(dataBuffer,m_pFlacParser->GetCodecHeader(m_pOggAudioCodecInfo[nIndex].SerialNo),size);
            *nBytesRead = size;
          }
        }
#endif
        retError = OGGSTREAM_SUCCESS;
        m_pOggAudioCodecInfo[nIndex].bSentIdentificationHdr = true;
        bSentCodecHeaders = true;
        m_bTimeStampValid = true;
        MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
        "GetCurrentSample Sent Identification Header %lu bytes",(*nBytesRead));
      }
      else if( (m_pOggAudioCodecInfo[nIndex].CommentHdr) &&
               (!m_pOggAudioCodecInfo[nIndex].bSentCommentHdr) )

      {
        memcpy(dataBuffer ,
               m_pOggAudioCodecInfo[nIndex].CommentHdr,
               m_pOggAudioCodecInfo[nIndex].nCommentHdrSize);
        *nBytesRead =  m_pOggAudioCodecInfo[nIndex].nCommentHdrSize;
        retError = OGGSTREAM_SUCCESS;
        m_pOggAudioCodecInfo[nIndex].bSentCommentHdr = true;
        bSentCodecHeaders = true;
        m_bTimeStampValid = true;
        MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
              "GetCurrentSample Sent Comment Header %lu bytes",(*nBytesRead));
      }
      else if( (m_pOggAudioCodecInfo[nIndex].SetupHdr)        &&
               (!m_pOggAudioCodecInfo[nIndex].bSentSetupHdr)  )
      {
         memcpy(dataBuffer ,
                m_pOggAudioCodecInfo[nIndex].SetupHdr,
                m_pOggAudioCodecInfo[nIndex].nSetupHdrSize);
         *nBytesRead =  m_pOggAudioCodecInfo[nIndex].nSetupHdrSize;
         retError = OGGSTREAM_SUCCESS;
         m_pOggAudioCodecInfo[nIndex].bSentSetupHdr = true;
         bSentCodecHeaders = true;
         m_bTimeStampValid = true;
         MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
                "GetCurrentSample Sent Setup Header %lu bytes",(*nBytesRead));
      }
    }//if(bContinue && bAudio)
    if(bContinue && bVideo)
    {
      if( (m_pOggVideoCodecInfo[nIndex].IdentificationHdr)    &&
          (!m_pOggVideoCodecInfo[nIndex].bSentIdentificationHdr)   )
      {
        memcpy(dataBuffer,
               m_pOggVideoCodecInfo[nIndex].IdentificationHdr,
               m_pOggVideoCodecInfo[nIndex].nIdentificationHdrSize);
        *nBytesRead =  m_pOggVideoCodecInfo[nIndex].nIdentificationHdrSize;
        retError = OGGSTREAM_SUCCESS;
        m_pOggVideoCodecInfo[nIndex].bSentIdentificationHdr = true;
        bSentCodecHeaders = true;
        MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
        "GetCurrentSample Sent Identification Header %lu bytes",(*nBytesRead));
      }
      else if( (m_pOggVideoCodecInfo[nIndex].CommentHdr)      &&
               (!m_pOggVideoCodecInfo[nIndex].bSentCommentHdr))
      {
        memcpy(dataBuffer,
               m_pOggVideoCodecInfo[nIndex].CommentHdr,
               m_pOggVideoCodecInfo[nIndex].nCommentHdrSize);
        *nBytesRead =  m_pOggVideoCodecInfo[nIndex].nCommentHdrSize;
        retError = OGGSTREAM_SUCCESS;
        m_pOggVideoCodecInfo[nIndex].bSentCommentHdr = true;
        bSentCodecHeaders = true;
        MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
               "GetCurrentSample Sent Comment Header %lu bytes",(*nBytesRead));
      }
      else if( (m_pOggVideoCodecInfo[nIndex].SetupHdr)        &&
               (!m_pOggVideoCodecInfo[nIndex].bSentSetupHdr)  )
      {
         memcpy(dataBuffer,
                m_pOggVideoCodecInfo[nIndex].SetupHdr,
                m_pOggVideoCodecInfo[nIndex].nSetupHdrSize);
         *nBytesRead =  m_pOggVideoCodecInfo[nIndex].nSetupHdrSize;
         retError = OGGSTREAM_SUCCESS;
         m_pOggVideoCodecInfo[nIndex].bSentSetupHdr = true;
         bSentCodecHeaders = true;
         MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
                 "GetCurrentSample Sent Setup Header %lu bytes",(*nBytesRead));
      }
    }//if(bContinue && bVideo)
    if((!bSentCodecHeaders) && (bContinue) && ((bAudio) ||(bVideo)) )
    {
      bool bEverythingOK = true;
      bool bPktAcrossMultiplePage = true;
      bool bPageCRCFail = false;
      while( (bPktAcrossMultiplePage || bPageCRCFail) &&
       (!(m_bParsedLastPage && m_pOggPage->nPageSegmentsTransmitted == m_pOggPage->PageSegments)) )
      {
        if(
            ((m_pOggPage->nPageSegmentsTransmitted == m_pOggPage->PageSegments)&& (m_pOggPage->PageSegments) ) ||
            (m_pOggPage->nPageSegmentsTransmitted == 0)
          )
        {
          bEverythingOK = false;
#ifdef OGG_PARSER_DEBUG
          MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
            "GetCurrentSample starting to parse fresh OGG page @ offset %llu",
            m_nCurrOffset);
#endif
          retError = ParseOGGPage(m_nCurrOffset,m_pOggPage);

          if(m_eParserState == OGGSTREAM_PAGE_CRC_ERROR || m_eParserState == OGGSTREAM_CORRUPT_DATA ||
              (retError == OGGSTREAM_SUCCESS && m_pOggPage->Granule == 0))
          {
              // If there is a CRC error on a Page we need to discard
              // all packets in that page. Any continuing packet from
              // previous page need to be discarded.
              *nBytesRead = 0;
              bPktAcrossMultiplePage = false;
              m_nCurrOffset = m_pOggPage->PageEndOffset;
              bPageCRCFail = true;
              continue;
          }
          else if(retError == OGGSTREAM_SUCCESS)
          {
            m_pOggPage->nPageSegmentsTransmitted = m_pOggPage->nPageSegmentsCorruptSample;
            bEverythingOK = true;
            bPageCRCFail = false;
            m_nCurrOffset = m_pOggPage->PageEndOffset;
            if(m_pOggPage->SerialNo == trackId)
            {
              if(m_nEndGranulePosn != (uint64)-1 &&
                  m_nEndGranulePosn != m_pOggPage->Granule)
              {
                m_nStartGranulePosn =  m_nEndGranulePosn;
              }
              m_nEndGranulePosn   =  m_pOggPage->Granule;
#ifdef OGG_PARSER_DEBUG
              MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_HIGH,
                "GetCurrentSample m_nStartGranulePosn %llu m_nEndGranulePosn %llu",
                m_nStartGranulePosn,m_nEndGranulePosn);
#endif
              //Now that a new page is parsed we can check if a valid timestamp can be found.
              //We can find a valid timestamp if a new packet starts in this page
              // from very first segment. That is, no packet is continuing from prev page.
              //If there is a packet continuing, the timestamp is already assigned from a
              // later place in the function.

              if(m_pOggPage->ContFlag == false &&
                  m_pOggPage->PageSegments &&
                  m_nStartGranulePosn != (uint64)-1)
              {
                //Spec is not very clear if a frame which is multiple of 255 bytes can have its
                // last segment of 0 bytes start on a new page. This doesnot make much sense
                // but lets have a check anyway.
                uint8 firstSegSize = *(m_pDataBuffer+m_pOggPage->SegmentTableOffset);
                if(!m_bTimeStampValid && m_pOggAudioCodecInfo[nIndex].SamplingFrequency
                    && firstSegSize)
                {
                  //If this Page starts with a new frame assign previous Page's
                  //granule position to this frame.
                  uint32 nSampFreq = m_pOggAudioCodecInfo[nIndex].SamplingFrequency;

                  m_bTimeStampValid = true;
                  m_nCurrentTimeStampMs = (uint64)
                         (((uint64)m_nStartGranulePosn * 1000 )/
                           nSampFreq);
                }
              }

            }
#ifdef OGG_PARSER_DEBUG
            MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
            "GetCurrentSample Next OGG Page Offset %llu",m_nCurrOffset);
#endif
          }
          else
          {
             bEverythingOK = false;
             break;
          }
        }
        if( (bEverythingOK) && (m_pOggPage->SerialNo == trackId) )
        {
          uint8  segsize = 0;
          uint64 segdataoffset = 0;
          bool bFirst = true;
#ifdef OGG_PARSER_DEBUG
          MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
            "GetCurrentSample Processing parsed OGG Page;Total Page Segments %lu",m_pOggPage->PageSegments);
#endif

          while( (m_pOggPage->nPageSegmentsTransmitted < m_pOggPage->PageSegments) &&
                 ((bFirst) || (segsize == 0xff) ) )
          {
            if(bFirst)
            {
              segdataoffset+= m_pOggPage->PageOffset;
              segdataoffset +=
                (m_pOggPage->SegmentTableOffset + m_pOggPage->PageSegments);
#ifdef OGG_PARSER_DEBUG
              MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
                "GetCurrentSample bFirst is TRUE Taking segdataoffset to start of segment data");
#endif
            }
            else
            {
              segdataoffset += segsize;
#ifdef OGG_PARSER_DEBUG
              MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
                "GetCurrentSample bFirst is FALSE, Using previous segsize to update segdataoffset");
#endif
            }
#ifdef OGG_PARSER_DEBUG
            MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_HIGH,
            "GetCurrentSample updated segdataoffset %llu nPageSegmentsTransmitted so far %lu",
             segdataoffset, m_pOggPage->nPageSegmentsTransmitted);
#endif

            memcpy(&segsize,
                   m_pDataBuffer+m_pOggPage->SegmentTableOffset+m_pOggPage->nPageSegmentsTransmitted,
                   sizeof(uint8));
#ifdef OGG_PARSER_DEBUG
            MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,"GetCurrentSample read next segsize %d",segsize);
#endif
            for(uint32 i =0; (bFirst) && (i < m_pOggPage->nPageSegmentsTransmitted);i++)
            {
              uint8 tempsegsize = 0;
              memcpy(&tempsegsize,
                     m_pDataBuffer+m_pOggPage->SegmentTableOffset+i,
                    sizeof(uint8));
              segdataoffset+= tempsegsize;
#ifdef OGG_PARSER_DEBUG
              MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,"GetCurrentSample updating segdataoffset based on tempsegsize %d",tempsegsize);
#endif
            }
#ifdef OGG_PARSER_DEBUG
            MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
             "GetCurrentSample updated segdataoffset before reading data %llu",
             segdataoffset);
#endif
            if(segsize)
            {
                if(m_nDataCacheFileOffset <= segdataoffset &&
                   (segdataoffset + segsize) < (m_nDataCacheFileOffset + m_nOggDataBufferFillSize))
                {
                    memcpy(dataBuffer+(*nBytesRead),
                           m_pDataCache + (segdataoffset - m_nDataCacheFileOffset),
                           segsize);
                    m_pOggPage->nPageSegmentsTransmitted++;
                    *nBytesRead +=  segsize;
                    retError = OGGSTREAM_SUCCESS;
#ifdef OGG_PARSER_DEBUG
                    MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_HIGH,
                      "GetCurrentSample nBytesRead %lu updated nPageSegmentsTransmitted %lu",
                      (*nBytesRead),m_pOggPage->nPageSegmentsTransmitted);
#endif
                }
                else
                {
                    if(
                      (!OGGStreamCallbakGetData (segdataoffset, segsize,
                                                 dataBuffer+(*nBytesRead),
                                                 nMaxBufSize, m_pUserData) )
                        )
                    {
                        MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_FATAL,
                        "GetCurrentSample read %lu bytes failed @ offset %llu",
                         (uint32)segsize, segdataoffset);
                        *nBytesRead = 0;
                         m_eParserState = OGGSTREAM_READ_ERROR;
                    }
                    else
                    {
                        m_pOggPage->nPageSegmentsTransmitted++;
                        *nBytesRead +=  segsize;
                        retError = OGGSTREAM_SUCCESS;
        #ifdef OGG_PARSER_DEBUG
                        MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_HIGH,
                        "GetCurrentSample nBytesRead %lu updated nPageSegmentsTransmitted %lu",
                       (*nBytesRead),m_pOggPage->nPageSegmentsTransmitted);
        #endif
                    }
                }
            }
            else
            {
               m_pOggPage->nPageSegmentsTransmitted++;
                  *nBytesRead +=  segsize;
                  retError = OGGSTREAM_SUCCESS;
            }

            bFirst = false;
            bPktAcrossMultiplePage = false;
          }//while( (bFirst) || (segsize == 0xff) )
          if( (m_pOggPage->PageSegments)                                         &&
              (m_pOggPage->nPageSegmentsTransmitted == m_pOggPage->PageSegments) &&
              (segsize == 0xff)
            )
          {
#ifdef OGG_PARSER_DEBUG
            MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"GetCurrentSample Last Page Segment Size is oxff,Need to parse next OGG Page...");
#endif
            bPktAcrossMultiplePage = true;
            //If the current packet is spanning multiple pages and it has started
            //in the current page, we can assign current page's granule position to this
            //packet. But do this only once. Remember that one packet can span multiple
            //pages not just one or two.
            if(!m_bTimeStampValid && m_pOggAudioCodecInfo[nIndex].SamplingFrequency &&
                m_nEndGranulePosn != (uint64)-1)
            {
              uint32 nSampFreq = m_pOggAudioCodecInfo[nIndex].SamplingFrequency;
              m_bTimeStampValid = true;
              m_nCurrentTimeStampMs = (uint64)
                   (((uint64)m_nEndGranulePosn * 1000 )/
                     nSampFreq);
            }
          }
        }//if(bEverythingOK)
      }//while(bPktAcrossMultiplePage)
    }//if((!bSentCodecHeaders) && (bContinue) && ((bAudio) ||(bVideo)) )
  }//if( (bContinue)  && (m_nCurrOffset < m_nFileSize)....
#ifdef OGG_PARSER_DEBUG
  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
               "GetCurrentSample returning nBytesRead %lu",(*nBytesRead));
#endif
  return retError;
}
/* =============================================================================
FUNCTION:
 OGGStreamParser::GetCurrentSampleTimeStamp

DESCRIPTION:
Returns current sample timestamp for the given trackId.

INPUT/OUTPUT PARAMETERS:

RETURN VALUE:

SIDE EFFECTS:
  None.
=============================================================================*/
OGGStreamStatus OGGStreamParser::GetCurrentSampleTimeStamp(uint32  id,
                                                           uint64* ts,
                                                           uint64* pullGranule,
                                                           bool*   isValid)
{
  OGGStreamStatus status = OGGSTREAM_FAIL;
#ifdef OGG_PARSER_DEBUG
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"GetCurrentSampleTimeStamp");
#endif
  //! Validate input parameter
  if ((!ts) || (!pullGranule) || (!isValid))
  {
    return status;
  }

#ifdef OGG_PARSER_APPROXIMATE_TIMESTAMPS
  uint64 nPrevGarnPos = m_nStartGranulePosn;
  uint64 nGranule = m_nEndGranulePosn - m_nStartGranulePosn;
  float nGranuleFrac = 0;
  *isValid = true;


  for(int i = 0; m_pOggAudioCodecInfo && (i< m_nAudioStreams);i++)
  {
    if((m_pOggAudioCodecInfo[i].SerialNo == id) && (ts) )
    {
      if(m_bTimeStampValid)
      {
        *ts               = m_nCurrentTimeStampMs;
        *pullGranule      = m_nEndGranulePosn - m_nStartGranulePosn;
        m_bTimeStampValid = false;
      }
      else
      {
        if(m_pOggPage->PageSegments && !m_bTimeStampValid)
        {
          nGranuleFrac = (float)(nGranule * m_pOggPage->nPageSegmentsTransmitted)
           / (float)m_pOggPage->PageSegments;
        }
        *ts =
        (uint32)(( ((float)nPrevGarnPos + nGranuleFrac)/
                  (float)m_pOggAudioCodecInfo[i].SamplingFrequency) * 1000);
      }
      status = OGGSTREAM_SUCCESS;
      break;
    }
  }
#else
  *isValid = false;
  for(uint32 i = 0; m_pOggAudioCodecInfo && (i< m_nAudioStreams);i++)
  {
    if((m_pOggAudioCodecInfo[i].SerialNo == id) && (ts) )
    {
      if(m_bTimeStampValid)
      {
        *ts               = m_nCurrentTimeStampMs;
        *isValid          = true;
        *pullGranule      = m_nEndGranulePosn - m_nStartGranulePosn;
        m_bTimeStampValid = false;
      }
      else
      {
        *ts          = 0;
        *pullGranule = 0;
      }
      status = OGGSTREAM_SUCCESS;
      break;
    }
  }
#endif
  for(int i = 0; m_pOggVideoCodecInfo && (i< m_nVideoStreams);i++)
  {
    if((m_pOggVideoCodecInfo[i].SerialNo == id) && (ts) )
    {
      *ts =
        (uint32)(m_nStartGranulePosn * m_pOggVideoCodecInfo[i].TimeInMsecPerFrame);
      status = OGGSTREAM_SUCCESS;
      break;
    }
  }
  return status;
}
/*! ======================================================================
@brief  Repositions given track to specified time

@detail    Seeks given track in forward/backward direction to specified time

@param[in]
 trackid: Identifies the track to be repositioned.
 nReposTime:Target time to seek to
 nCurrPlayTime: Current playback time
 sample_info: Sample Info to be filled in if seek is successful
 canSyncToNonKeyFrame: When true, video can be repositioned to non key frame
 nSyncFramesToSkip:Number of sync samples to skip. Valid only if > or < 0.

@return    OGGSTREAM_SUCCESS if successful other wise returns OGGSTREAM_FAIL
@note      None.
========================================================================== */
OGGStreamStatus OGGStreamParser::Seek(uint32 trackid, uint64 nReposTime, uint64 nCurrPlayTime,
                                      ogg_stream_sample_info* sample_info,
                                      bool bForward, bool canSyncToNonKeyFrame, int nSyncFramesToSkip)
{
  MM_MSG_PRIO3(MM_FILE_OPS, MM_PRIO_HIGH,
    "OGGStreamParser::Seek SerialNum %lu nReposTime %llu nCurrPlayTime %llu",
    trackid, nReposTime, nCurrPlayTime);
  MM_MSG_PRIO3(MM_FILE_OPS, MM_PRIO_HIGH,
    "OGGStreamParser::Seek bForward %d canSyncToNonKeyFrame %d nSyncFramesToSkip %d",
    bForward,canSyncToNonKeyFrame,nSyncFramesToSkip);
  OGGStreamStatus retError = OGGSTREAM_FAIL;
  bool bValid = false;
  bool bAudio = false;
  bool bClosest = false;
#ifdef _ANDROID_
  bClosest = true;
#endif
  uint32 trackIndex = 0;
  int    nIndex     = 0;

  //Determine whether given track-id is for audio/video
  for(; m_pOggAudioCodecInfo && (nIndex< m_nAudioStreams);nIndex++)
  {
    if(m_pOggAudioCodecInfo[nIndex].SerialNo == trackid)
    {
      bValid = true;
      bAudio = true;
      trackIndex = nIndex;
      break;
    }
  }
  for(nIndex = 0; nIndex < m_nVideoStreams; nIndex++)
  {
    if (!m_pOggVideoCodecInfo)
    {
      break;
    }
    if(m_pOggVideoCodecInfo[nIndex].SerialNo == trackid  )
    {
      bValid     = true;
      bAudio     = false;
      trackIndex = nIndex;
      break;
    }
  }
  if(bValid && sample_info)
  {
    memset(sample_info,0,sizeof(ogg_stream_sample_info));
    if(nReposTime == 0)
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"Seeking to 0 TS...");
      //Start from the begining, reset the offset and sampleinfo and return
      if(ParseOGGPage(m_nDataPageOffset,m_pOggPage) != OGGSTREAM_SUCCESS)
      {
        MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_FATAL,
          "Seek to 0 TS failed......PLEASE CHECK m_nDataPageOffset %llu",
          m_nDataPageOffset);
      }
      else
      {
        retError = OGGSTREAM_SUCCESS;
        sample_info->bsync = 1;
        sample_info->ntime = 0;
        m_nCurrOffset = m_nDataPageOffset;
        sample_info->noffset = (uint32)m_nDataPageOffset;
        m_eParserState = OGGSTREAM_READY;
        m_nStartGranulePosn = 0;
        m_nEndGranulePosn = 0;
        m_bTimeStampValid = true;
        m_nCurrentTimeStampMs = 0;
        m_bParsedLastPage = false;
        MM_MSG_PRIO3(MM_FILE_OPS, MM_PRIO_HIGH,
          "Seek sample_info->ntime %llu nReposTime %llu offset %llu ",
          sample_info->ntime,nReposTime,m_nCurrOffset);
      }
    }
    else
    {
      bool bNeedToSeek = true;
      if(m_pOggAudioCodecInfo
#if defined (FEATURE_OGGPARSER_FLAC_FAST_START_UP) || defined(FEATURE_OGGPARSER_FAST_START_UP)
#ifndef FEATURE_OGGPARSER_FAST_START_UP
      && m_pOggAudioCodecInfo[trackIndex].Audio_Codec == OGG_AUDIO_CODEC_FLAC
#endif
      )
     {
       if(nReposTime >= m_nClipDuration)
       {
          nReposTime = m_nClipDuration;
       }
       uint32 nCurrentTime = (uint32)(( ((float)m_nStartGranulePosn)/
                    (float)m_pOggAudioCodecInfo[trackIndex].SamplingFrequency) * 1000);
       //Do approximate Seek
       // If repositioning is within 4 seconds intially or within
       // 4 seconds from current location we are not doing approximate
       // seek as it can be inefficient. If seek is backward seek, we
       // will estimate anyway unless seek position is within
       // 4seconds of start of playback as it isnot possible to go
       // other that starting from beginning.
       if(!bForward ||
           (nReposTime > OGG_APPROX_LIMIT_START_RANGE &&
            nReposTime <= m_nClipDuration &&
            FILESOURCE_ABS32((int32)nReposTime - (int32)nCurrentTime) > OGG_APPROX_LIMIT_START_RANGE)
           )
       {
       //     uint32 nBytesNeed = FLAC_PARSER_BUFFER_SIZE;
          bNeedToSeek = true;
          OggPage hTempOggPage;
          OGGStreamStatus nRetError = OGGSTREAM_FAIL;
          bool bFound = false;
          uint64 nOffset = 0;
          int32  nOffsetAdjust = 0;
          float nSlope = 0;
          uint64 nLocalOffset;
          uint32 nMaxIter = OGG_APPROX_SEEK_MAX_PASS;
          uint32 nLocalTime = 0;
          uint32 nPrevOffset = 0;
          uint32 nPrevTime = 0;
          uint64 nReposAdjustTime = nReposTime - OGGPARSER_SEEK_TIME_ADJUST;


          nLocalOffset = (uint32)((float)nReposAdjustTime *
                 ((float)m_nCurrOffset / ((float)nCurrentTime + 1)));

          if(nLocalOffset >= m_nFileSize)
          {
              nLocalOffset = m_nFileSize - MAX_PAGE_SIZE;
          }
          else if(nLocalOffset < m_nDataPageOffset)
          {
              nLocalOffset = m_nDataPageOffset;
          }

          nPrevOffset = (uint32)m_pOggPage->PageOffset;
          nPrevTime = nCurrentTime;
          while(!bFound && nMaxIter)
          {
              nOffset = nLocalOffset;
              while(!bFound && nOffset < m_nFileSize)
              {
                 //Search for Oggs sync word in the buffer.
                  nOffset = FindNextPageOffset(nOffset);

                  if(nOffset == (uint64)-1)
                  {
                      break;
                  }

                  nRetError = ValidateOggPage(nOffset,&hTempOggPage);
                  if(nRetError == OGGSTREAM_SUCCESS &&
                      hTempOggPage.SerialNo == trackid)
                  {
                      bFound = true;
                      nMaxIter--;
                      if(hTempOggPage.Granule == (uint64)-1)
                      {
                          //TODO see if any sample start in this Page as well.
                          bFound = false;
                          nOffset = hTempOggPage.PageEndOffset;
                          continue;
                      }
                      nLocalOffset = nOffset;
                      nLocalTime = (uint32)(((float)hTempOggPage.Granule/
                              (float)m_pOggAudioCodecInfo[trackIndex].SamplingFrequency) * 1000);
                  }
                  //else It was not a valid page. Search again

                  nOffset += PAGE_MAGIC_NUMBER_SIZE;

              }
              if(!bFound)
              {
                  break;
              }
              if((!nMaxIter ||
                  (FILESOURCE_ABS32((int32)nLocalTime - (int32)nReposAdjustTime)
                  <= OGGPARSER_SEEK_MAX_DEVIATION_MS)))
              {
                 memcpy(m_pOggPage , &hTempOggPage, sizeof(OggPage));
                 m_nStartGranulePosn =  m_pOggPage->Granule - m_pOggPage->PageSegments;//TODO..Approximation.
                 m_nEndGranulePosn   =  m_pOggPage->Granule;
                 m_nCurrOffset = m_pOggPage->PageOffset;
                 if(nLocalTime > nReposTime)
                 {
                    //Cant go back. This is the best seek possible
                    bNeedToSeek = false;
                    sample_info->ntime = nLocalTime;
                    sample_info->bsync = 1;
                   // memcpy(m_pOggPage,&hTempOggPage,sizeof(OggPage));
                    m_bParsedLastPage = false;
                    m_eParserState = OGGSTREAM_READY;
                    retError = OGGSTREAM_SUCCESS;
                 }
                 bForward = true;
                 break;
              }
              bFound = false;

              nSlope = (float)((int32)(nLocalOffset - nPrevOffset))/
                       (((float)( (int32)(nLocalTime -
                                             nPrevTime)) /1000) + 1);
              nPrevOffset = (uint32)nLocalOffset;
              nPrevTime = nLocalTime;

              nOffsetAdjust =  (int32)(( ((float) ((int32)(nReposAdjustTime -
                                      nLocalTime) ) ) / 1000)  *nSlope);

              nLocalOffset += nOffsetAdjust;

              if(nLocalOffset >= m_nFileSize || nLocalOffset < m_nDataPageOffset)
              {
                 memcpy(m_pOggPage , &hTempOggPage, sizeof(OggPage));
                 m_nStartGranulePosn =  m_pOggPage->Granule - m_pOggPage->PageSegments;
                 m_nEndGranulePosn   =  m_pOggPage->Granule;
                 m_nCurrOffset = m_pOggPage->PageOffset;
                 if(nLocalTime > nReposTime)
                 {
                    //Cant go back. This is the best seek possible
                    bNeedToSeek = false;
                    sample_info->ntime = nLocalTime;
                    sample_info->bsync = 1;
                    m_bParsedLastPage = false;
                    m_eParserState = OGGSTREAM_READY;
                    retError = OGGSTREAM_SUCCESS;
                 }
                 bForward = true;
                 break;
              }
          }
        }
      }
#endif //FEATURE_OGGPARSER_FLAC_FAST_START_UP

#ifdef FEATURE_OGGPARSER_BUILD_SEEK_INDEX
      if(SearchOGGIndex(trackid,nReposTime,nCurrPlayTime,sample_info,
                        bForward,canSyncToNonKeyFrame,nSyncFramesToSkip) == OGGSTREAM_SUCCESS )
      {
        bNeedToSeek = false;
        retError = OGGSTREAM_SUCCESS;
      }
#endif
      if( (bAudio) && bNeedToSeek )
      {
        //Copy the current parsed OGG page into hTempOggPage
        OggPage hTempOggPage;
        memcpy(&hTempOggPage,m_pOggPage,sizeof(OggPage));

        //To keep track of previous ogg page
        OggPage hTempPrvOggPage;
        memset(&hTempPrvOggPage,0,sizeof(OggPage));
        uint64 nPrvPageStartGranule = 0;
        uint32 currts               = 0;
        uint64 nPrevPageTS          = 0;
        uint64 nCurrStartGranule    = m_nStartGranulePosn;
        uint64 nCurrEndGranule      = m_nEndGranulePosn;
        uint64 nNextPageOffset      = hTempOggPage.PageEndOffset;
        MM_MSG_PRIO4(MM_FILE_OPS, MM_PRIO_HIGH,
     "Start Seek nNextPageOffset %llu nCurrPageOffset %llu StartGranule %llu EndGranule %llu",
     nNextPageOffset,hTempOggPage.PageOffset,nCurrStartGranule,nCurrEndGranule);
        if((bForward) || (bClosest))
        {
          #ifdef OGG_PARSER_DEBUG
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"Searching in forward direction");
          #endif
          bool bOK             = true;
          bool bPrevEntryFound = false;
          while(bOK)
          {
            //Calculate the TS from the current parsed ogg page
            currts = (uint32)
              ( ( (float)nCurrStartGranule / (float)m_pOggAudioCodecInfo[trackIndex].SamplingFrequency ) *1000 );
            #ifdef OGG_PARSER_DEBUG
            MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,"Seek current parsed TS %lu",currts);
            #endif

            if(
                 ( (currts >= nReposTime)   &&
                   (!hTempOggPage.ContFlag) &&
                   (hTempOggPage.SerialNo == trackid) ) ||

                 ( ((currts > nCurrPlayTime) || bClosest)    &&
                   (m_bParsedLastPage)         &&
                   (!hTempPrvOggPage.ContFlag) &&
                   (hTempPrvOggPage.SerialNo == trackid) )
              )
            {
              /* If this flag set to true, it means Parser seeks to previous
                 entry */
              bool bUsePrev = true;
              /* If closest seek is required, then Parser checks the closest
                 entry between the previous and the current entry. If the
                 previous entry is closer, then set the flag as true.*/
              if( (bClosest) && (bPrevEntryFound) )
              {
                if ((currts > nReposTime) &&
                    (currts - nReposTime) > (nReposTime - nPrevPageTS))
                {
                  bUsePrev = false;
                }
              }
              else
              {
                bUsePrev = false;
              }
              //! If prev flag is set to true, seek to previous entry
              if( (!bUsePrev) &&
                  ((!m_bUserInitiatedRW) || (currts >= nReposTime) ))
              {
                //Desired TS belong to the current parsed ogg page
                #ifdef OGG_PARSER_DEBUG
                MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_HIGH,
                  "Current parsed TS %lu matched with nReposTime %llu",
                  currts, nReposTime);
                #endif
                m_nStartGranulePosn = nCurrStartGranule;
                m_nEndGranulePosn = nCurrEndGranule;
                m_nCurrOffset = hTempOggPage.PageOffset;
                retError = OGGSTREAM_SUCCESS;
                sample_info->bsync = 1;
                sample_info->ntime = currts;
                m_eParserState = OGGSTREAM_READY;
                m_bParsedLastPage = false;
                sample_info->noffset = (uint32)m_nCurrOffset;
                memcpy(m_pOggPage,&hTempOggPage,sizeof(OggPage));
                #ifdef OGG_PARSER_DEBUG
                MM_MSG_PRIO3(MM_FILE_OPS, MM_PRIO_HIGH,
                  "m_nStartGranulePosn %llu m_nEndGranulePosn %llu m_nCurrOffset %llu",
                  m_nStartGranulePosn,m_nEndGranulePosn,m_nCurrOffset);
                #endif
              }
              else
              {
                /*We can come here under 2 cases as mentioned below:
                *
                * Case 1:
                *Even though we are searching forward,
                *this call is a result of user initiated rewind.
                *Our goal is to find a page which satisfies following condition.
                *
                *hTempOggPage points to current parsed page and
                *starting TS on this page (currts) >= nReposTime, however,
                *hTempPrvOggPage points to a page which lies before hTempOggPage
                *and with the largest TS, which is <= nReposTime.
                *
                * Case 2:
                * While searching forward, we reached EOF/ last OGG page whose
                * TS is < nReposTime.In such case, we need to pick the ogg page
                * whose TS < nReposTime but > nCurrePlayTime
                */
                currts = (uint32)
                  (((float)nPrvPageStartGranule * MILLISEC_TIMESCALE_UNIT) /
                    (float)m_pOggAudioCodecInfo[trackIndex].SamplingFrequency);
                #ifdef OGG_PARSER_DEBUG
                MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_HIGH,
                  "Current parsed TS %lu matched with nReposTime %llu",
                  currts, nReposTime);
                #endif
                m_nStartGranulePosn = nPrvPageStartGranule;
                m_nEndGranulePosn = hTempPrvOggPage.Granule;
                m_nCurrOffset = hTempPrvOggPage.PageOffset;
                retError = OGGSTREAM_SUCCESS;
                sample_info->bsync = 1;
                sample_info->ntime = currts;
                m_eParserState = OGGSTREAM_READY;
                sample_info->noffset = (uint32)m_nCurrOffset;
                memcpy(m_pOggPage,&hTempPrvOggPage,sizeof(OggPage));
                #ifdef OGG_PARSER_DEBUG
                MM_MSG_PRIO3(MM_FILE_OPS, MM_PRIO_HIGH,
                  "m_nStartGranulePosn %llu m_nEndGranulePosn %llu m_nCurrOffset %llu",
                  m_nStartGranulePosn, m_nEndGranulePosn, m_nCurrOffset);
                #endif
              }
              break;
            }
            else
            {
              if( (!hTempOggPage.ContFlag)&& (hTempOggPage.SerialNo == trackid) )
              {
                memcpy(&hTempPrvOggPage,&hTempOggPage,sizeof(OggPage));
                nPrvPageStartGranule = nCurrStartGranule;
                nPrevPageTS          = currts;
                bPrevEntryFound      = true;
              }
              nNextPageOffset = hTempOggPage.PageEndOffset;
              bOK = false;
              //Parse the next OGG page
              #ifdef OGG_PARSER_DEBUG
              MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
                  "Seek parsing next OGG page @ offset %llu", nNextPageOffset);
              #endif
              retError = ParseOGGPage(nNextPageOffset,&hTempOggPage);
              if(m_eParserState == OGGSTREAM_PAGE_CRC_ERROR)
              {
                nNextPageOffset = hTempOggPage.PageEndOffset;
                memcpy(&hTempOggPage, &hTempPrvOggPage, sizeof(OggPage));
                hTempOggPage.PageEndOffset = nNextPageOffset;
                bOK = true;
              }

              if(retError == OGGSTREAM_SUCCESS)
              {
                if(hTempOggPage.SerialNo == trackid)
                {
                  nCurrStartGranule =  nCurrEndGranule;
                  nCurrEndGranule   =  hTempOggPage.Granule;
                }
                bOK = true;
              }
              //! This is a special case, where Parser is not able to find
              //! desired PAGE info. Call same Seek API internally by reducing,
              //! Seek time by 3sec. Instead of reporting Seek failure, this
              //! is better solution.
              else if((OGGSTREAM_READ_ERROR == m_eParserState) && (bClosest))
              {
                //! Seek to ZERO to reset offset to proper value.
                Seek(trackid, 0, nCurrPlayTime, sample_info,
                     false, canSyncToNonKeyFrame, nSyncFramesToSkip);
                nReposTime = nReposTime > OGG_SEEK_DELTA?
                             nReposTime - OGG_SEEK_DELTA: 0;
                if(Seek(trackid,nReposTime, nCurrPlayTime, sample_info,
                        false, canSyncToNonKeyFrame, nSyncFramesToSkip) ==
                   OGGSTREAM_SUCCESS)
                {
                  retError = OGGSTREAM_SUCCESS;
                  m_pDataBufferOffset = m_nOggDataBufferSize;
                  m_bTimeStampValid = false;
                  return retError;
                }
              }
            }
          }//while(bOK)
        }//if(bForward)
        else
        {
          #ifdef OGG_PARSER_DEBUG
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"Initiating USER REWIND....");
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"Trying to Seek to the begining...");
          #endif
          /*
          * Since we can't traverse OGG pages in backward direction,the only way to
          * search backward is go to the beginning and start searching forward.
          */
          if(Seek(trackid,0,nCurrPlayTime,sample_info,
               false,canSyncToNonKeyFrame,nSyncFramesToSkip) == OGGSTREAM_SUCCESS)
          {
            m_bUserInitiatedRW = true;
            #ifdef OGG_PARSER_DEBUG
            MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"Now searching in forward direction.....");
            #endif
            if( Seek(trackid,nReposTime,0,sample_info,true,
                     canSyncToNonKeyFrame,nSyncFramesToSkip) == OGGSTREAM_SUCCESS )
            {
              #ifdef OGG_PARSER_DEBUG
              MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"REWIND SUCCEED.....");
              #endif
              retError = OGGSTREAM_SUCCESS;
              m_bUserInitiatedRW = false;
            }
          }
        }
      }//if(bAudio)
    }//end of else of if(nReposTime == 0)
  }//if(bValid && sample_info)
  m_pDataBufferOffset = m_nOggDataBufferSize;
  //Timestamp valid wil be made true and calculated when next frame is delivered
  m_bTimeStampValid = false;
  return retError;
}
/* =============================================================================
FUNCTION:
 OGGStreamParser::GetVideoWidth

DESCRIPTION:
Returns video width for given id
INPUT/OUTPUT PARAMETERS:

RETURN VALUE:
 Video width
SIDE EFFECTS:
  None.
=============================================================================*/
uint32 OGGStreamParser::GetVideoWidth(uint32 ulTrackId)
{
  uint32 ulWidth = 0;
  int    nIndex  = 0;
  for ( ; m_pOggVideoCodecInfo && (nIndex < m_nVideoStreams); nIndex++)
  {
    if(m_pOggVideoCodecInfo[nIndex].TrackId == ulTrackId)
    {
      ulWidth = m_pOggVideoCodecInfo[nIndex].Width;
    }
  }
  MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_LOW,"GetVideoWidth for id %lu is %lu",
              ulTrackId, ulWidth);
  return ulWidth;
}
/* =============================================================================
FUNCTION:
 OGGStreamParser::GetVideoHeight

DESCRIPTION:
Returns video height for given id
INPUT/OUTPUT PARAMETERS:

RETURN VALUE:
 Video height
SIDE EFFECTS:
  None.
=============================================================================*/
uint32 OGGStreamParser::GetVideoHeight(uint32 ulTrackId)
{
  uint32 ulHeight = 0;
  int    nIndex   = 0;
  for ( ; m_pOggVideoCodecInfo && (nIndex < m_nVideoStreams); nIndex++)
  {
    if(m_pOggVideoCodecInfo[nIndex].TrackId == ulTrackId)
    {
      ulHeight = m_pOggVideoCodecInfo[nIndex].Height;
    }
  }
  MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_LOW,"GetVideoHeight for id %lu is %lu",
              ulTrackId, ulHeight);
  return ulHeight;
}

#ifdef FEATURE_FILESOURCE_FLAC_PARSER
OGGStreamStatus  OGGStreamParser::GetFlacStreamInfo(uint32 id,
                                                    flac_metadata_streaminfo* pinfo)
{
  OGGStreamStatus eRet = OGGSTREAM_INVALID_PARAM;
  uint32 ulSerialNum = GetTrackSerialNo(id);
  if(pinfo)
  {
    if(m_pFlacParser)
    {
      if(FLACPARSER_SUCCESS == m_pFlacParser->GetFlacStreamInfo(ulSerialNum,pinfo))
      {
        eRet = OGGSTREAM_SUCCESS;
      }
    }
  }
  return eRet;
}
#endif

/* =============================================================================
FUNCTION:
 OGGStreamParser::GetTrackAverageBitRate

DESCRIPTION:
Returns avg. bit-rate for given id
INPUT/OUTPUT PARAMETERS:

RETURN VALUE:
 Returns bit-rate for given track id
SIDE EFFECTS:
  None.
=============================================================================*/
uint32 OGGStreamParser::GetTrackAverageBitRate(uint32 ulTrackId)
{
  uint32 ulAvgBitRate = 0;
  int    nIndex       = 0;
  for ( ; m_pOggVideoCodecInfo && (nIndex < m_nVideoStreams); nIndex++)
  {
    if(m_pOggVideoCodecInfo[nIndex].TrackId == ulTrackId)
    {
      ulAvgBitRate = m_pOggVideoCodecInfo[nIndex].BitRate;
    }
  }
  for (nIndex = 0; m_pOggAudioCodecInfo && (nIndex < m_nAudioStreams);nIndex++)
  {
    if(m_pOggAudioCodecInfo[nIndex].TrackId == ulTrackId)
    {
      ulAvgBitRate = m_pOggAudioCodecInfo[nIndex].NominalBitRate;
    }
  }
  MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_LOW,
         "GetTrackAverageBitRate for id %lu is %lu", ulTrackId, ulAvgBitRate);

  return ulAvgBitRate;
}

/* =============================================================================
FUNCTION:
  OGGStreamParser::GetAudioStreamInfo

DESCRIPTION:
  Copies Audio Info details into o/p structure given

INPUT/OUTPUT PARAMETERS:

RETURN VALUE:
 Returns bit-rate for given track id
SIDE EFFECTS:
  None.
=============================================================================*/
bool OGGStreamParser::GetAudioStreamInfo(uint32 ulTrackId,
                                         ogg_audio_info* pAudioInfo)
{
  bool bFound = false;
  int nIndex = 0;
  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW,
               "GetAudioStreamInfo for track id %lu", ulTrackId);
  for (; m_pOggAudioCodecInfo && (nIndex < m_nAudioStreams); nIndex++)
  {
    if(m_pOggAudioCodecInfo[nIndex].TrackId == ulTrackId)
    {
      memcpy(pAudioInfo, m_pOggAudioCodecInfo + nIndex,
             sizeof(ogg_audio_info));
      bFound = true;
      break;
    }
  }
  return bFound;
}

/* =============================================================================
FUNCTION:
 OGGStreamParser::GetVideoFrameRate

DESCRIPTION:
Returns video frame rate for given id
INPUT/OUTPUT PARAMETERS:

RETURN VALUE:
 Returns video frame rate for given id
SIDE EFFECTS:
  None.
=============================================================================*/
float OGGStreamParser::GetVideoFrameRate(uint32 ulTrackId)
{
  float fFrameRate = 0.0;
  int nIndex = 0;

  for ( ; m_pOggVideoCodecInfo && (nIndex < m_nVideoStreams); nIndex++)
  {
    if( (m_pOggVideoCodecInfo[nIndex].TrackId == ulTrackId) &&
        (m_pOggVideoCodecInfo[nIndex].AspectRatio_Dmr) )
    {
      fFrameRate = m_pOggVideoCodecInfo[nIndex].FrameRate;
      break;
    }
  }
  MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_LOW,"GetVideoFrameRate for id %lu is %f",
              ulTrackId, fFrameRate);
  return fFrameRate;
}
/* =============================================================================
FUNCTION:
 OGGStreamParser::GetCodecHeader

DESCRIPTION:
Returns codec specific header for given trackid
INPUT/OUTPUT PARAMETERS:

RETURN VALUE:
 Returns codec specific header for given trackid
SIDE EFFECTS:
  None.
=============================================================================*/
uint8* OGGStreamParser::GetCodecHeader(uint32 id)
{
#ifdef OGG_PARSER_DEBUG
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"GetCodecHeader");
#endif
  int nAudioIndex = -1;
  int nVideoIndex = -1;
  uint8* pHeader = NULL;
  uint32 ulSerialNum = GetTrackSerialNo(id);
  //Validate track-id and locate the index in audio/video info structure
  ogg_media_codec_type codec = GetTrackType(ulSerialNum);

  if( (codec == OGG_AUDIO_CODEC_FLAC)|| (codec == OGG_AUDIO_CODEC_VORBIS) )
  {
    int nIndex =0;
    for( ; m_pOggAudioCodecInfo && (nIndex < m_nAudioStreams); nIndex++)
    {
      if(m_pOggAudioCodecInfo[nIndex].SerialNo == ulSerialNum)
      {
        nAudioIndex = nIndex;
        break;
      }
    }
  }
  if(codec == OGG_VIDEO_CODEC_THEORA)
  {
    int nIndex =0;
    for(; m_pOggVideoCodecInfo && (nIndex < m_nVideoStreams); nIndex++)
    {
      if(m_pOggVideoCodecInfo[nIndex].SerialNo == ulSerialNum)
      {
        nVideoIndex = nIndex;
        break;
      }
    }
  }
  switch(codec)
  {
    case OGG_AUDIO_CODEC_FLAC:
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"OGG_AUDIO_CODEC_FLAC");
#ifdef FEATURE_FILESOURCE_FLAC_PARSER
      if(nAudioIndex != -1)
      {
        pHeader = m_pFlacParser->GetCodecHeader( m_pOggAudioCodecInfo[nAudioIndex].SerialNo);
      }
#endif
    }
    break;
    case OGG_AUDIO_CODEC_VORBIS:
    {
      if(nAudioIndex != -1)
      {
        pHeader = m_pOggAudioCodecInfo[nAudioIndex].pCodecHeader;
      }
    }
    break;
    case OGG_VIDEO_CODEC_THEORA:
    {
      if(nVideoIndex != -1)
      {
        pHeader = m_pOggVideoCodecInfo[nVideoIndex].pCodecHeader;
      }
    }
    break;
    case OGG_UNKNOWN_AUDIO_VIDEO_CODEC:
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"GetCodecHeaderSize OGG_UNKNOWN_AUDIO_VIDEO_CODEC");
    }
    break;
  }//switch(codec)
  return pHeader;
}
/* =============================================================================
FUNCTION:
 OGGStreamParser::GetCodecHeaderSize

DESCRIPTION:
Returns codec specific header size for given trackid
INPUT/OUTPUT PARAMETERS:

RETURN VALUE:
 Returns codec specific header size for given trackid
SIDE EFFECTS:
  None.
=============================================================================*/
uint32 OGGStreamParser::GetCodecHeaderSize(uint32 id)
{
  uint32 nsize = 0;
#ifdef OGG_PARSER_DEBUG
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"GetCodecHeaderSize");
#endif

  int nAudioIndex = -1;
  int nVideoIndex = -1;
  uint32 ulSerialNum = GetTrackSerialNo(id);
  //Validate track-id and locate the index in audio/video info structure
  ogg_media_codec_type codec = GetTrackType(ulSerialNum);

  if( (codec == OGG_AUDIO_CODEC_FLAC)|| (codec == OGG_AUDIO_CODEC_VORBIS) )
  {
    int nIndex = 0;
    for(; m_pOggAudioCodecInfo && (nIndex < m_nAudioStreams); nIndex++)
    {
      if(m_pOggAudioCodecInfo[nIndex].SerialNo == ulSerialNum)
      {
        nAudioIndex = nIndex;
        break;
      }
    }
  }
  if(codec == OGG_VIDEO_CODEC_THEORA)
  {
    int nIndex = 0;
    for(; m_pOggVideoCodecInfo && (nIndex < m_nVideoStreams); nIndex++)
    {
      if(m_pOggVideoCodecInfo[nIndex].SerialNo == ulSerialNum)
      {
        nVideoIndex = nIndex;
        break;
      }
    }
  }
  switch(codec)
  {
    case OGG_AUDIO_CODEC_FLAC:
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"OGG_AUDIO_CODEC_FLAC");
#ifdef FEATURE_FILESOURCE_FLAC_PARSER
      if( (nAudioIndex != -1) && (m_pFlacParser) )
      {
        nsize = m_pFlacParser->GetCodecHeaderSize(m_pOggAudioCodecInfo[nAudioIndex].SerialNo);
      }
#endif
    }
    case OGG_AUDIO_CODEC_VORBIS:
    {
      if( nAudioIndex != -1 )
      {
        nsize += m_pOggAudioCodecInfo[nAudioIndex].nIdentificationHdrSize;
        nsize += m_pOggAudioCodecInfo[nAudioIndex].nCommentHdrSize;
        nsize += m_pOggAudioCodecInfo[nAudioIndex].nSetupHdrSize;
      }
    }
    break;
    case OGG_VIDEO_CODEC_THEORA:
    {
      if( nVideoIndex != -1)
      {
        nsize += m_pOggVideoCodecInfo[nVideoIndex].nIdentificationHdrSize;
        nsize += m_pOggVideoCodecInfo[nVideoIndex].nCommentHdrSize;
        nsize += m_pOggVideoCodecInfo[nVideoIndex].nSetupHdrSize;
      }
    }
    break;
    case OGG_UNKNOWN_AUDIO_VIDEO_CODEC:
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"GetCodecHeaderSize OGG_UNKNOWN_AUDIO_VIDEO_CODEC");
    }
    break;
  }//switch(codec)
  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,"GetCodecHeaderSize nsize %lu",nsize);
  return nsize;
}

/* =============================================================================
FUNCTION:
 OGGStreamParser::GetTrackMaxBufferSize

DESCRIPTION:
Returns maximum buffersize for trackid
INPUT/OUTPUT PARAMETERS:

RETURN VALUE:

SIDE EFFECTS:
  None.
=============================================================================*/
uint32 OGGStreamParser::GetTrackMaxBufferSize(uint32 id)
{
  uint32 nsize = 0;
#ifdef OGG_PARSER_DEBUG
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"GetTrackMaxBufferSize");
#endif
  uint32 ulSerialNum = GetTrackSerialNo(id);
#ifdef FEATURE_FILESOURCE_FLAC_PARSER
  ogg_media_codec_type codec = GetTrackType(ulSerialNum);

  if(codec == OGG_AUDIO_CODEC_FLAC && m_pFlacParser)
  {
     nsize = m_pFlacParser->GetFlacMaxBufferSize(ulSerialNum);
  }
  else
#endif //FEATURE_FILESOURCE_FLAC_PARSER
  {
     nsize = MAX_PAGE_SIZE;
  }
  return nsize;
}


/* =============================================================================
FUNCTION:
 OGGStreamParser::IsMetaDataParsingDone

DESCRIPTION:
Returns true if all the required meta data is being parsed
INPUT/OUTPUT PARAMETERS:

RETURN VALUE:
 Returns true if all the required meta data is being parsed otherwise, returns false
SIDE EFFECTS:
  None.
=============================================================================*/
bool OGGStreamParser::IsMetaDataParsingDone(OggPage* pOggPage)
{
  bool bRet = false;
#ifdef OGG_PARSER_DEBUG
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"IsMetaDataParsingDone");
#endif
  if(pOggPage)
  {
    bRet = m_bOggMetaDataParsed;
    uint8 nAudioStreamsParsed = 0;
    uint8 nVideoStreamsParsed = 0;

    if(pOggPage->PageEndOffset == m_nFileSize)
    {
      m_bParsedLastPage = true;
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,"Reached EOF @offset %llu",
                   m_nFileSize);
    }

    for(int i =0; (m_pOggAudioCodecInfo) && (i < m_nAudioStreams); i++)
    {
      if(
          (m_pOggAudioCodecInfo[i].IdentificationHdr)                     &&
          (m_pOggAudioCodecInfo[i].CommentHdr)                            &&
          (m_pOggAudioCodecInfo[i].SetupHdr)                              &&
          (m_pOggAudioCodecInfo[i].Audio_Codec == OGG_AUDIO_CODEC_VORBIS)
  #ifndef FEATURE_OGGPARSER_FAST_START_UP
          &&
          (m_bParsedLastPage)
  #endif
        )
      {
        nAudioStreamsParsed++;
      }
#ifdef  FEATURE_FILESOURCE_FLAC_PARSER
      else if( (m_pOggAudioCodecInfo[i].Audio_Codec == OGG_AUDIO_CODEC_FLAC) &&
               (m_pFlacParser) )
      {
        if(
            (m_pFlacParser->IsMetaDataParsingDone())
  #if !defined(FEATURE_OGGPARSER_FAST_START_UP) && !defined(FEATURE_OGGPARSER_FLAC_FAST_START_UP)
            &&
            (m_bParsedLastPage)
  #endif
          )
        {
          nAudioStreamsParsed++;
        }
      }
#endif
    }//for(int i =0; (m_pOggAudioCodecInfo) && (i < m_nAudioStreams); i++)

    for(int i =0; (m_pOggVideoCodecInfo) && (i < m_nVideoStreams); i++)
    {
      if(
          (m_pOggVideoCodecInfo[i].IdentificationHdr) &&
          (m_pOggVideoCodecInfo[i].CommentHdr)        &&
          (m_pOggVideoCodecInfo[i].SetupHdr) )
      {
        nVideoStreamsParsed++;
      }
    }
    if(
        (nAudioStreamsParsed == m_nAudioStreams) &&
        (nVideoStreamsParsed == m_nVideoStreams)
      )
    {
      UpdateGranulePosition(pOggPage);
#ifdef OGG_PARSER_DEBUG
      MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_HIGH,
        "IsMetaDataParsingDone returing TRUE nAudioStreamsParsed %d nVideoStreamsParsed %d",nAudioStreamsParsed,nVideoStreamsParsed);
#endif
      bRet = true;
    }
  }//if(pOggPage)
  return bRet;
}

/*=============================================================================
FUNCTION:
 OGGStreamParser::InitCrcComputeTables

DESCRIPTION:
 Generates the CRC tables for table-driven approach
INPUT/OUTPUT PARAMETERS:

RETURN VALUE:
 void
SIDE EFFECTS:
  None.
=============================================================================*/
void OGGStreamParser::InitCrcComputeTables(void)
{
  int i, j;
  uint32 nCrcAccum = 0;

  for (i=0;  i<256;  i++)
  {
    nCrcAccum = i << 24;

    for ( j = 0;  j < 8;  j++ )
    {
      if (MSBIT(nCrcAccum))
      {
        nCrcAccum = (nCrcAccum << 1) ^ OGG_GENERATOR_POLYNOMIAL;
      }
      else
      {
        nCrcAccum = (nCrcAccum << 1);
      }
    }

    m_aCRCTable[i] = nCrcAccum;
  }
  return;
}

/*=============================================================================
FUNCTION:
 OGGStreamParser::FindCheckSum

DESCRIPTION:
 Finds the checksum for current page
INPUT/OUTPUT PARAMETERS:

RETURN VALUE:
 CRC check sum
SIDE EFFECTS:
  None.
=============================================================================*/
uint32 OGGStreamParser::FindCheckSum(uint8 *pData,
                                     uint32 nSize)
{
   uint32 i;
   uint32 crc32 = 0;

   while(nSize--)
   {
     i = ((uint32) (crc32 >> 24) ^ * pData++) & 0xFF;
     crc32 = (crc32 << 8) ^ m_aCRCTable[i];
   }
//   crc32 = crc32 ^ 0; //XOR with 0; redundant just a description of what
                      // is mentioned in spec
   return crc32;
}


/* =============================================================================
FUNCTION:
 OGGStreamParser::IsOGGStreamValid

DESCRIPTION:
Returns true if current Page passes CRC check
INPUT/OUTPUT PARAMETERS:

RETURN VALUE:
 Returns true if current Page passes CRC check otherwise false
SIDE EFFECTS:
  None.
=============================================================================*/
bool OGGStreamParser::CheckPageCRC(uint8* pPageBuf, uint32 nSize)
{
  uint32 nStrmCheckSum;
  uint32 nCheckSum;

// Lets do the CRC check for the OGG Page
// According to spec, bytes 22 - 23 (counted from 0) gives the check sum of
// the current page.
// CRC generator polynomial is 0x04c11db7

  memcpy(&nStrmCheckSum,(pPageBuf + OGG_CHECKSUM_OFFSET),
         OGG_NUM_CHECKSUM_BYTES);

// Spec then says:
// Polynomial is divided over the entire header by making the checksum
// bytes to be 0 and
// continued to the remaining bytes in the page to find the checksum.
  pPageBuf[22] = 0;
  pPageBuf[23] = 0;
  pPageBuf[24] = 0;
  pPageBuf[25] = 0;

  nCheckSum = FindCheckSum(pPageBuf, nSize);

  pPageBuf[22] = nStrmCheckSum & 0xff;
  pPageBuf[23] = (nStrmCheckSum >> 8)& 0xff;
  pPageBuf[24] = (nStrmCheckSum  >> 16)& 0xff;
  pPageBuf[25] = ( ((uint8)(nStrmCheckSum>> 24)) & 0xff);

  if(nCheckSum != nStrmCheckSum)
  {
     //Page has erranoeus bits.CRC check failed.
     return false;
  }
  return true;
}

/* =============================================================================
FUNCTION:
 OGGStreamParser::FindNextPageOffset

DESCRIPTION:
Find the absolute offset to next page by searching the "OggS" pattern in stream.
INPUT/OUTPUT PARAMETERS:

RETURN VALUE:
 Returns offset if succesful otherwise -1
SIDE EFFECTS:
  None.
=============================================================================*/
uint64 OGGStreamParser::FindNextPageOffset(uint64 nOffsetPrevGoodPos)
{
  uint64 nOffset = nOffsetPrevGoodPos;
  //! Check the available data in file and read Max possible amount of data in
  //! one instance.
  uint32 nAvailData   = (uint32)(m_nFileSize - nOffset);
  uint32 nBytesNeeded = FILESOURCE_MIN(m_nOggDataBufferSize, nAvailData);
  uint32 nlocalOffset = 0;
  //! For Last few pages, Max data need not be "MAX_PAGE_SIZE" value.
  //! Check against Max available data and take minimum value
  uint32 nMaxData     = FILESOURCE_MIN(nBytesNeeded, MAX_PAGE_SIZE);
  //We are going to do a non progressive access to file to find last frame.
  //Lets restore after we do it.

  while(nOffset < m_nFileSize)
  {
   if(m_nDataCacheFileOffset <= nOffset &&
    nOffset < (m_nDataCacheFileOffset + m_nOggDataBufferFillSize) &&
      m_pDataBufferOffset != m_nOggDataBufferFillSize)
   {
     m_pDataBuffer = m_pDataCache + (nOffset - m_nDataCacheFileOffset);
     m_pDataBufferOffset = uint32(m_pDataBuffer - m_pDataCache);

   }
   else
   {
     m_pDataBufferOffset = m_nOggDataBufferSize;
   }
   //! Check if Max possible data is available in local cache or not
   if(m_nOggDataBufferFillSize < m_pDataBufferOffset||
       m_nOggDataBufferFillSize - m_pDataBufferOffset < nMaxData ||
       (nOffset >= m_nFileSize))
    {
      //memset(m_pDataCache,0,m_nOggDataBufferSize);
      if( (nBytesNeeded + nOffset) > m_nFileSize )
      {
        nBytesNeeded = (int)(m_nFileSize - nOffset);
        if(nBytesNeeded < PAGE_MAGIC_NUMBER_SIZE)
        {
            return (uint64)-1;
        }
        memset(m_pDataCache + nBytesNeeded, 0, m_nOggDataBufferSize - nBytesNeeded);
      }

      if(!OGGStreamCallbakGetData (nOffset, nBytesNeeded, m_pDataCache,
                                   m_nOggDataBufferSize, m_pUserData) )
      {
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,"ParseOGGPage read failed..");
        m_nOggDataBufferFillSize = 0;
        m_nDataCacheFileOffset = 0;
        m_pDataBufferOffset = 0;
        return (uint64)-1;
      }
      else
      {
        m_nOggDataBufferFillSize = nBytesNeeded;
        m_nDataCacheFileOffset = nOffset;
        m_pDataBufferOffset = 0;
        m_pDataBuffer = m_pDataCache;
      }

    }

    //if(m_nOggDataBufferFillSize - m_pDataBufferOffset < PAGE_MAGIC_NUMBER_SIZE)
    //m_nOggDataBufferFillSize - m_pDataBufferOffset - PAGE_MAGIC_NUMBER_SIZE
    //will be a large number which will lead to infinite loop in while loop
    if(m_nOggDataBufferFillSize - m_pDataBufferOffset < PAGE_MAGIC_NUMBER_SIZE)
      return (uint64) -1;

    //Correlate the complete buffer with OGG_PAGE_MAGIC_NUMBER and find the
    //matching offset.
    nlocalOffset = 0;
    while(nlocalOffset < ((m_nOggDataBufferFillSize - m_pDataBufferOffset) - PAGE_MAGIC_NUMBER_SIZE))
    {
      if(memcmp(m_pDataBuffer+nlocalOffset,OGG_PAGE_MAGIC_NUMBER,
                                            PAGE_MAGIC_NUMBER_SIZE) == 0)
      {
        #ifdef OGG_PARSER_DEBUG
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                              "Next OGG page start found after error!!!");
        #endif
        return nOffsetPrevGoodPos + nlocalOffset;
      }
      nlocalOffset++;
    }


    // Need to give some overlap to search for pattern across buffer
    // boundary
    nOffset += (m_nOggDataBufferFillSize - m_pDataBufferOffset) - (PAGE_MAGIC_NUMBER_SIZE - 1);
  }
  // Could not find a valid Ogg page marker in file after the
  // specified offset.
  return (uint64) -1;
}

#if defined (FEATURE_OGGPARSER_FAST_START_UP) || defined(FEATURE_OGGPARSER_FLAC_FAST_START_UP)
/* =============================================================================
FUNCTION:
 OGGStreamParser::FindNextPageOffset

DESCRIPTION:
Find the absolute offset to next page by searching the "OggS" pattern in stream.
INPUT/OUTPUT PARAMETERS:

RETURN VALUE:
 Returns offset if succesful otherwise -1
SIDE EFFECTS:
  None.
=============================================================================*/
bool OGGStreamParser::FindLastValidPage(uint32 trackid, uint64 fileSize, OggPage *pOggPage)
{
  uint64 nOffset = 0;
  uint64 nLocalOffset;
  uint32 nSearchLimit = (uint32)fileSize;
  OGGStreamStatus retError = OGGSTREAM_FAIL;
  bool bFound = false;
  bool bDone  = false;

  uint8* nTempDataBuffer = m_pDataBuffer;
  uint32 nTempDataCacheFileOffset = (uint32)m_nDataCacheFileOffset;
  uint32 nTempDataBufferOffset = m_pDataBufferOffset;
  uint32 nTempBufferFillSize = m_nOggDataBufferFillSize;

  if(fileSize > MAX_PAGE_SIZE)
  {
       nLocalOffset = fileSize - MAX_PAGE_SIZE;
  }
  else
  {
      nLocalOffset = 0;
  }


  while(!bFound && !bDone)
  {
      nOffset = nLocalOffset;
      while((int64)nOffset <= (int64)nSearchLimit)
      {
          //Search for Oggs sync word in the buffer.
          nOffset = FindNextPageOffset(nOffset);

          if(nOffset == (uint64)-1)
          {
              if(bFound)
              {
                  bDone = true;
                  break;
              }
              else
                  break;
          }
          retError = ValidateOggPage(nOffset,pOggPage);

          if(retError == OGGSTREAM_SUCCESS)
          {
              bFound = true;
              if(pOggPage->Granule == (uint64)-1 ||
                  pOggPage->Granule == 0  ||
                  pOggPage->SerialNo != trackid)
              {
                  //TODO see if any sample start in this Page as well.
                  bFound = false;
              }
              nOffset = pOggPage->PageEndOffset;
              continue;
          }
          //else It was not a valid page. Search again
          nOffset += PAGE_MAGIC_NUMBER_SIZE;
      }
      nSearchLimit = (uint32)nLocalOffset;
      if(nLocalOffset == 0)
      {
          break;
      }
      if(nLocalOffset < MAX_PAGE_SIZE)
      {
         nLocalOffset = 0;
      }
      else
      {
         nLocalOffset -= MAX_PAGE_SIZE;
      }
  }

  if(!OGGStreamCallbakGetData (nTempDataCacheFileOffset,
                               nTempBufferFillSize,
                               m_pDataCache,
                               m_nOggDataBufferSize, m_pUserData) )
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,"ParseOGGPage read failed..");

  }
  //Restore Cache variables to where we started with.
  m_pDataBuffer = nTempDataBuffer;
  m_nDataCacheFileOffset = nTempDataCacheFileOffset;
  m_pDataBufferOffset = nTempDataBufferOffset;
  m_nOggDataBufferFillSize = nTempBufferFillSize;

  if(bDone)
      return true;
  return false;
}


OGGStreamStatus  OGGStreamParser::ValidateOggPage(uint64 offset,OggPage* pOggPage)
{
#ifdef OGG_PARSER_DEBUG
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"ParseOGGPage");
#endif
  OGGStreamStatus retError = OGGSTREAM_INVALID_PARAM;
  uint32 localOffset = 0;

  if(m_nDataCacheFileOffset <= offset &&
     offset < (m_nDataCacheFileOffset + m_nOggDataBufferSize) &&
     m_pDataBufferOffset != m_nOggDataBufferSize)
  {
     m_pDataBuffer = m_pDataCache + offset - m_nDataCacheFileOffset;
     m_pDataBufferOffset = uint32(m_pDataBuffer - m_pDataCache);
  }
  else
  {
     m_pDataBufferOffset = m_nOggDataBufferSize;
  }



  if(m_pDataCache && pOggPage && m_nOggDataBufferSize)
  {
    //! Check Max available data
    uint32 nAvailData  = (uint32)(m_nFileSize - offset);
    uint32 nSizeNeeded = FILESOURCE_MIN(m_nOggDataBufferSize, nAvailData);
    //! For Last few pages, Max data need not be "MAX_PAGE_SIZE" value.
    //! Check against Max available data and take minimum value
    uint32 nMaxData    = FILESOURCE_MIN(nSizeNeeded, MAX_PAGE_SIZE);
    retError = OGGSTREAM_SUCCESS;
    memset(pOggPage,0,sizeof(OggPage));
#ifdef OGG_PARSER_DEBUG
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,"ParseOGGPage nSizeNeeded %d",nSizeNeeded);
#endif

    //! Compare with Max possible data available
    if(m_nOggDataBufferFillSize < m_pDataBufferOffset ||
        m_nOggDataBufferFillSize - m_pDataBufferOffset < nMaxData ||
        (offset >= m_nFileSize))
    {
      if( (nSizeNeeded + offset) > m_nFileSize )
      {
        nSizeNeeded = (uint32)(m_nFileSize - offset);
#ifdef OGG_PARSER_DEBUG
        MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,"ParseOGGPage Updated nSizeNeeded %d",nSizeNeeded);
#endif
        memset(m_pDataCache + nSizeNeeded, 0, m_nOggDataBufferSize - nSizeNeeded);
      }
      if(!OGGStreamCallbakGetData (offset, nSizeNeeded, m_pDataCache,
                                   m_nOggDataBufferSize, m_pUserData) )
      {
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,"ParseOGGPage read failed..");
        retError = OGGSTREAM_READ_ERROR;
        m_nOggDataBufferFillSize = 0;
        m_nDataCacheFileOffset = 0;
        m_pDataBufferOffset = 0;
      }
      else
      {
        m_eParserState = OGGSTREAM_READY;
        m_nOggDataBufferFillSize = nSizeNeeded;
        m_nDataCacheFileOffset = offset;
        m_pDataBufferOffset = 0;
        m_pDataBuffer = m_pDataCache;
      }
    }
    if(m_eParserState != OGGSTREAM_READ_ERROR)
    {
      //Make sure there is page SYNC
      //if(memcmp(m_pDataBuffer+localOffset,OGG_PAGE_MAGIC_NUMBER,PAGE_MAGIC_NUMBER_SIZE) != 0)
      if(m_pDataBuffer[localOffset] != 'O' ||
         m_pDataBuffer[localOffset + 1] != 'g'||
         m_pDataBuffer[localOffset + 2] != 'g'||
         m_pDataBuffer[localOffset + 3] != 'S')

      {
        retError = OGGSTREAM_CORRUPT_DATA;
        m_eParserState = OGGSTREAM_CORRUPT_DATA;
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,"ParseOGGPage Failed to locate OGG Page SYNC!!!");
      }
      else
      {
        pOggPage->PageOffset = offset;
        localOffset  += PAGE_MAGIC_NUMBER_SIZE;
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW,"ParseOGGPage Located OGG Page SYNC");
        //Ensure correct OGG bitstream format version
        if(m_pDataBuffer[localOffset]!= OGG_STREAM_VERSION_FORMAT )
        {
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,"ParseOGGPage Invalid Version#");
          retError = OGGSTREAM_CORRUPT_DATA;
          m_eParserState = OGGSTREAM_CORRUPT_DATA;
        }
        else
        {
          m_pOggPage->nPageSegmentsCorruptSample = 0;
          pOggPage->Version = m_pDataBuffer[localOffset];
          //Parse rest of the page
          localOffset++;
          pOggPage->HdrType = m_pDataBuffer[localOffset];
          uint8 contpkt = m_pDataBuffer[localOffset] & OGG_CONT_PAGE;
          uint8 bospage = m_pDataBuffer[localOffset] & OGG_BOS_PAGE;
          uint8 eospage = m_pDataBuffer[localOffset] & OGG_EOS_PAGE;
          pOggPage->ContFlag = contpkt;
          pOggPage->BOSFlag = bospage;
          pOggPage->EOSFlag = eospage;
          localOffset++;
#ifdef OGG_PARSER_DEBUG
          MM_MSG_PRIO3(MM_FILE_OPS, MM_PRIO_HIGH,"ParseOGGPage contpkt %d bospage %d eospage %d",contpkt,bospage,eospage);
#endif
          //memcpy(&pOggPage->Granule,m_pDataBuffer+localOffset,sizeof(uint64));
          pOggPage->Granule = (uint64)(m_pDataBuffer[localOffset++]);
          pOggPage->Granule |= (uint64)(m_pDataBuffer[localOffset++]) << 8;
          pOggPage->Granule |= (uint64)(m_pDataBuffer[localOffset++]) << 16;
          pOggPage->Granule |= (uint64)(m_pDataBuffer[localOffset++]) << 24;
          pOggPage->Granule |= (uint64)(m_pDataBuffer[localOffset++]) << 32;
          pOggPage->Granule |= (uint64)(m_pDataBuffer[localOffset++]) << 40;
          pOggPage->Granule |= (uint64)(m_pDataBuffer[localOffset++]) << 48;
          pOggPage->Granule |= (uint64)(m_pDataBuffer[localOffset++]) << 56;

#ifdef OGG_PARSER_DEBUG
          MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
                      "ParseOGGPage pOggPage->Granule %llu",pOggPage->Granule);
#endif
          pOggPage->SerialNo = (uint32)(m_pDataBuffer[localOffset++]);
          pOggPage->SerialNo |= (uint32)(m_pDataBuffer[localOffset++]) << 8;
          pOggPage->SerialNo |= (uint32)(m_pDataBuffer[localOffset++]) << 16;
          pOggPage->SerialNo |= (uint32)(m_pDataBuffer[localOffset++]) << 24;

          pOggPage->SeqNo = (uint32)(m_pDataBuffer[localOffset++]);
          pOggPage->SeqNo |= (uint32)(m_pDataBuffer[localOffset++]) << 8;
          pOggPage->SeqNo |= (uint32)(m_pDataBuffer[localOffset++]) << 16;
          pOggPage->SeqNo |= (uint32)(m_pDataBuffer[localOffset++]) << 24;

          localOffset += FOURCC_SIGNATURE_BYTES;

          pOggPage->PageSegments = m_pDataBuffer[localOffset];
#ifdef OGG_PARSER_DEBUG
          MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
                      "ParseOGGPage page_segments %lu",pOggPage->PageSegments);
#endif
          localOffset++;
          uint32 size = localOffset + pOggPage->PageSegments;

          //Store offsets relative to start of page offset as opposed
          //to absolute file offset.
          //pOggPage->SegmentTableOffset = pOggPage->PageOffset + localOffset;
          pOggPage->SegmentTableOffset = localOffset;

          // If the new Page has no packets continuing from a previous corrupt
          // page we can move state to READY
          for(int nIndex = 0; nIndex < (int)pOggPage->PageSegments; nIndex++)
          {
            size += m_pDataBuffer[localOffset];
            localOffset++;
          }
          offset += size;
          m_pDataBufferOffset += size;
          pOggPage->PageEndOffset = offset;


          //Now that we have the Page available in buffer and Page size known
          //We can validate the page for bitstream errors

          if(!CheckPageCRC(m_pDataBuffer, size))
          {
            MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,"ParseOGGPage CRC check failed");

            // See if a valid page starts from the size calculated.
            if(memcmp(m_pDataBuffer+size,OGG_PAGE_MAGIC_NUMBER,PAGE_MAGIC_NUMBER_SIZE) != 0)
            {
              uint64 nNextPageOffset;

              MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,"ParseOGGPage:Locate OGG Page SYNC after corrupt Page!!!");

              // Send the valid offset and try to find the next page. Add PAGE_MAGIC_NUMBER_SIZE
              // to offset of current page in order not to find 0 offset.
              nNextPageOffset = FindNextPageOffset(offset + PAGE_MAGIC_NUMBER_SIZE);
              if(nNextPageOffset != (uint64)-1)
              {
                pOggPage->PageEndOffset = nNextPageOffset;
              }
              else
              {
                MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,"ParseOGGPage Unable to find Page End Offset.. Abort!!!");
                return OGGSTREAM_CORRUPT_DATA;
              }

            }

            return OGGSTREAM_PAGE_CRC_ERROR;
          }

        }//end of else of if(m_pDataBuffer[localOffset]!= OGG_STREAM_VERSION_FORMAT )
      }//end of else of if(memcmp(m_pDataBuffer+localOffset,OGG_PAGE_MAGIC_NUMBER,PAGE_MAGIC_NUMBER_SIZE) != 0) )
    }//end of else of if(!OGGStreamCallbakGetData (offset, nSizeNeeded, m_pDataBuffer, m_nOggDataBufferSize, (uint32) m_pUserData) )
  }//if(m_pDataBuffer && pOggPage && m_nOggDataBufferSize)
  return retError;
}

#endif // defined (FEATURE_OGGPARSER_FAST_START_UP) || defined(FEATURE_OGGPARSER_FLAC_FAST_START_UP)
#endif //ifdef FEATURE_FILESOURCE_OGG_PARSER

