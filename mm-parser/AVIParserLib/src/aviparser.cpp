
/* =======================================================================
                              aviParser.cpp
DESCRIPTION

Copyright (c) 2010-2015 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
========================================================================== */

/* =======================================================================
                             PERFORCE HEADER
$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/AVIParserLib/main/latest/src/aviparser.cpp#136 $
========================================================================== */

#include "aviparser.h"

#ifdef FEATURE_FILESOURCE_AVI

#include <assert.h>
#include "MMDebugMsg.h"

#include "qcplayer_oscl_utils.h"
#include "MMMalloc.h"

#define HTONS(x) \
        ((unsigned short)((((unsigned short)(x) & 0x00ff) << 8) | \
                         (((unsigned short)(x) & 0xff00) >> 8)))
/* =============================================================================
FUNCTION:
  aviParser::ValidateChunkSize

DESCRIPTION:
  Validates child chunk has size lesser than parent chunk

INPUT/OUTPUT PARAMETERS:

RETURN VALUE:
  bool, true in case of error and false if no error

SIDE EFFECTS:
  None.
=============================================================================*/
bool aviParser::ValidateChunkSize( uint32 ulChildChunkSize , uint32 ulParentChunkSize)
{
  bool bRetError = false;
  if (ulChildChunkSize > ulParentChunkSize)
  {
    bRetError = true;
  }
  return bRetError;
}

/* =============================================================================
FUNCTION:
 ascii_2_short_int


DESCRIPTION:
 Helper function to convert ascii characters into short int.

INPUT/OUTPUT PARAMETERS:

RETURN VALUE:

SIDE EFFECTS:
  None.
=============================================================================*/
avi_uint16 ascii_2_short_int(avi_uint16* ptr, bool *bResult = NULL)
{
  if(*ptr == 0)
    return (avi_uint16)-1;
  avi_uint16 no = 0;
  char* data = (char*)ptr;
  bool bOK = false;
  if(bResult)
  {
    *bResult = false;
  }

  for(unsigned int i = 0; (data!= 0) && i < sizeof(avi_uint16);i++)
  {
    if((data[i] >= '0')&&(data[i]<='9'))
    {
      no = ( (10*no) + (data[i]-'0'));
      if(bResult)
      {
        *bResult = true;
      }
      bOK = true;
    }
  }
  if(! bOK)
  {
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,"ascii_2_short_int could not locate ASCII CHARS *ptr %x",(*ptr));
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,"trackId returned not correct,PLEASE CHECK..");
    no = (avi_uint16)-1;
  }
  return no;
}

/* =============================================================================
FUNCTION:
  aviParser::aviParser

DESCRIPTION:
  Class constructor

INPUT/OUTPUT PARAMETERS:

RETURN VALUE:

SIDE EFFECTS:
  None.
=============================================================================*/
aviParser::aviParser(void* pUserData,avi_uint64 fileSize,
                     bool bDiscardAudIndex,
                     bool bDiscardVidIndex,bool bHttpStreaming)
{
#ifdef FEATURE_FILESOURCE_AVI_PARSER_DEBUG
  MM_MSG_PRIO3(MM_FILE_OPS, MM_PRIO_LOW,
                "aviParser::aviParser fileSize %d bDiscardAudIndex %d bDiscardVidIndex %d",
                fileSize,bDiscardAudIndex,bDiscardVidIndex);
#endif
  m_bDRMProtection = false;
  m_bSeekDenied = true;
  m_nNumOfRiff = 0;
  m_pUserData = pUserData;
  m_nCurrentChunkDataSize = 0;
  m_nCurrOffset = m_nMoviOffset = m_nIdx1Offset = m_nStartOfMovi = m_nLastOffsetRead = m_nSampleInfoOffset =  0;
  m_CurrentParserState = AVI_PARSER_IDLE;
  m_nFileSize = fileSize;
  m_bDiscardAudioIndex = bDiscardAudIndex;
  m_bDiscardVideoIndex = bDiscardVidIndex;
  memset(&m_hAviSummary,0,sizeof(avi_summary_info));
  m_nBytesToBeAdjustedForMOVI = 0;
  m_bByteAdjustedForMOVI = m_bisAVIXpresent = false;
  m_VolSize = 0;
  memset(&m_AviClipMetaInfo,0,sizeof(avi_info_struct));
  memset(&m_AviSTRDHeader,0,sizeof(m_AviSTRDHeader));
  m_nMoviSize = m_nIdx1Size = 0;
  m_nAdjustedMoviOffset = 0;
  m_nAdjustedIdx1Offset = 0;
  m_nCurrentSampleInfoOffsetInIdx1 = 0;
  m_nCurrAudioSampleInIdx1 = 0;
  m_nCurrVideoSampleInIdx1 = 0;
  m_pIdx1SeekCache = NULL;
  m_pMultipleRiff = NULL;
  m_pWmaAudioInfo = NULL;
  m_bHttpStreaming = bHttpStreaming;

  for(int k = 0; k < AVI_MAX_TRACKS;k++)
  {
    memset(&m_hAviSummary.stream_index[k],0,sizeof(m_hAviSummary.stream_index[k]));
    memset(&m_base_indx_tbl[k],0,sizeof(m_base_indx_tbl[k]));

    m_AviVOLHeader[k] = (memory_struct*)MM_Malloc(sizeof(memory_struct));
    if(m_AviVOLHeader[k])
    {
      memset(m_AviVOLHeader[k],0,sizeof(memory_struct));
    }
  }

  for(int j = 0; j < AVI_MAX_VIDEO_TRACKS;j++)
  {
    memset(&m_nCurrVideoFrameCount[j],0,sizeof(m_nCurrVideoFrameCount[j]));
    memset(&m_hAviSummary.video_info[j],0,sizeof(m_hAviSummary.video_info[j]));
  }
  for(int k = 0; k < AVI_MAX_AUDIO_TRACKS; k++)
  {
    memset(&m_nCurrAudioPayloadSize[k],0,sizeof(m_nCurrAudioPayloadSize[k]));
    memset(&m_nCurrAudioFrameCount[k] ,0,sizeof(m_nCurrAudioFrameCount[k]));
    memset(&m_hAviSummary.audio_info[k] ,0,sizeof(m_hAviSummary.audio_info[k]));
    memset(&m_nParserAudSampleEndTime[k],0,sizeof(m_nParserAudSampleEndTime[k]));
  }
}
/* =============================================================================
FUNCTION:
 aviParser::~aviParser

DESCRIPTION:
  Class destructor

INPUT/OUTPUT PARAMETERS:

RETURN VALUE:

SIDE EFFECTS:
  None.
=============================================================================*/
aviParser::~aviParser()
{
#ifdef FEATURE_FILESOURCE_AVI_PARSER_DEBUG
  if(m_hAviSummary.pIdx1Table)
  {
    MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_LOW,
                  "aviParser::~aviParser #Audio entries allocated %d #Video entries allocated %d",
                  m_hAviSummary.pIdx1Table->nAudioEntriesAllocated,m_hAviSummary.pIdx1Table->nVideoEntriesAllocated);
  }
#endif
  if(m_hAviSummary.pIdx1Table)
  {
    if( m_hAviSummary.pIdx1Table->pAudioEntries )
    {
      MM_Free(m_hAviSummary.pIdx1Table->pAudioEntries);
      m_hAviSummary.pIdx1Table->pAudioEntries = NULL;
    }
    if( m_hAviSummary.pIdx1Table->pKeyVideoEntries )
    {
      MM_Free(m_hAviSummary.pIdx1Table->pKeyVideoEntries);
      m_hAviSummary.pIdx1Table->pKeyVideoEntries = NULL;
    }
    MM_Free(m_hAviSummary.pIdx1Table);
    m_hAviSummary.pIdx1Table = NULL;
  }
  for(int i =0; i < AVI_MAX_VIDEO_TRACKS;i++)
  {
    if(m_hAviSummary.video_info[i].strnVideo.streamName)
    {
      MM_Free(m_hAviSummary.video_info[i].strnVideo.streamName);
      m_hAviSummary.video_info[i].strnVideo.streamName = NULL;
    }
    if(m_hAviSummary.video_info[i].strfVideo.pExtraData)
    {
      MM_Free(m_hAviSummary.video_info[i].strfVideo.pExtraData);
    }
  }
  for(int i =0; i < AVI_MAX_AUDIO_TRACKS;i++)
  {
    if(m_hAviSummary.audio_info[i].strnAudio.streamName)
    {
      MM_Free(m_hAviSummary.audio_info[i].strnAudio.streamName);
      m_hAviSummary.audio_info[i].strnAudio.streamName = NULL;
    }
    if(m_hAviSummary.audio_info[i].strfAudio.extra)
    {
      MM_Free(m_hAviSummary.audio_info[i].strfAudio.extra);
      m_hAviSummary.audio_info[i].strfAudio.extra = NULL;
    }
  }
  for(int k = 0; k < AVI_MAX_TRACKS; k++)
  {
    if(m_AviVOLHeader[k])
    {
      if(m_AviVOLHeader[k]->pMemory)
      {
        MM_Free(m_AviVOLHeader[k]->pMemory);
      }
      MM_Free(m_AviVOLHeader[k]);
    }
  }
  for(int l = 0; l < AVI_MAX_TRACKS; l++)
  {
    if(m_base_indx_tbl[l].pIndxSuperIndexEntry)
    {
      MM_Free(m_base_indx_tbl[l].pIndxSuperIndexEntry);
    }
    if(m_base_indx_tbl[l].pIXIndexChunk)
    {
      for(avi_uint32 m = 0 ; m < m_base_indx_tbl[l].nEntriesInUse ; m++)
      {
        if(m_base_indx_tbl[l].pIXIndexChunk[m].pIndxChunkIndexEntry)
        {
          MM_Free(m_base_indx_tbl[l].pIXIndexChunk[m].pIndxChunkIndexEntry);
        }
      }
      MM_Free(m_base_indx_tbl[l].pIXIndexChunk);
      m_base_indx_tbl[l].pIXIndexChunk = NULL;
    }
  }
  if(m_pIdx1SeekCache && m_pIdx1SeekCache->pMemory)
  {
    MM_Free(m_pIdx1SeekCache->pMemory);
    MM_Free(m_pIdx1SeekCache);
    m_pIdx1SeekCache = NULL;
  }
  if(m_pWmaAudioInfo)
  {
    MM_Free(m_pWmaAudioInfo);
    m_pWmaAudioInfo = NULL;
  }
  if(m_pMultipleRiff)
  {
    MM_Free(m_pMultipleRiff);
    m_pMultipleRiff = NULL;
  }
  if(m_hAviSummary.avih)
  {
    MM_Free(m_hAviSummary.avih);
    m_hAviSummary.avih = NULL;
  }
  if(m_AviSTRDHeader.drm_info)
  {
    MM_Free(m_AviSTRDHeader.drm_info);
  }
  if(m_AviClipMetaInfo.ArchLocn.Ptr){
      MM_Free(m_AviClipMetaInfo.ArchLocn.Ptr);
  }
  if(m_AviClipMetaInfo.Artist.Ptr){
      MM_Free(m_AviClipMetaInfo.Artist.Ptr);
  }
  if(m_AviClipMetaInfo.Commissioned.Ptr){
    MM_Free(m_AviClipMetaInfo.Commissioned.Ptr);
  }
  if(m_AviClipMetaInfo.Comments.Ptr){
    MM_Free(m_AviClipMetaInfo.Comments.Ptr);
  }
  if(m_AviClipMetaInfo.Copyright.Ptr){
    MM_Free(m_AviClipMetaInfo.Copyright.Ptr);
  }
  if(m_AviClipMetaInfo.CreateDate.Ptr){
    MM_Free(m_AviClipMetaInfo.CreateDate.Ptr);
  }
  if(m_AviClipMetaInfo.Genre.Ptr){
    MM_Free(m_AviClipMetaInfo.Genre.Ptr);
  }
  if(m_AviClipMetaInfo.Keyword.Ptr){
    MM_Free(m_AviClipMetaInfo.Keyword.Ptr);
  }
  if(m_AviClipMetaInfo.Name.Ptr){
    MM_Free(m_AviClipMetaInfo.Name.Ptr);
  }
  if(m_AviClipMetaInfo.Product.Ptr){
    MM_Free(m_AviClipMetaInfo.Product.Ptr);
  }
  if(m_AviClipMetaInfo.Subject.Ptr){
    MM_Free(m_AviClipMetaInfo.Subject.Ptr);
  }
  if(m_AviClipMetaInfo.Software.Ptr){
    MM_Free(m_AviClipMetaInfo.Software.Ptr);
  }
  if(m_AviClipMetaInfo.Source.Ptr){
    MM_Free(m_AviClipMetaInfo.Source.Ptr);
  }

  memset(&m_AviClipMetaInfo,0,sizeof(m_AviClipMetaInfo));
  m_CurrentParserState = AVI_PARSER_IDLE;
}
/* =============================================================================
FUNCTION:
  aviParser::parseAVIH

DESCRIPTION:
  Reads and parses AVI header.

INPUT/OUTPUT PARAMETERS:

RETURN VALUE:

SIDE EFFECTS:
  None.
=============================================================================*/
aviErrorType aviParser::parseAVIH(avi_uint64* offset, uint32 ulMaxSize)
{
  unsigned char* byteData = m_ReadBuffer;
  avi_uint32 nSize = 0;

  aviErrorType retError = AVI_READ_FAILURE;
  avi_uint64 offsetAtTheEndOfAVIH = 0;

#ifdef FEATURE_FILESOURCE_AVI_PARSER_DEBUG
  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW,
                "parseAVIH Offset to start %d",*offset);
#endif
  if(!parserAVICallbakGetData(*offset,2 * sizeof(fourCC_t),
    m_ReadBuffer,AVI_READ_BUFFER_SIZE,m_pUserData, &retError))
  {
    return retError;
  }
  *offset+= (2 *sizeof(fourCC_t));
  if(!memcmp(byteData,AVI_AVIH_FOURCC,sizeof(fourCC_t)))
  {
    memcpy(&nSize,byteData+4,sizeof(fourCC_t));
    bool bRetError = ValidateChunkSize(nSize, ulMaxSize);
    if(bRetError)
    {
      retError = AVI_CORRUPTED_FILE;
      return retError;
    }

    offsetAtTheEndOfAVIH = (*offset + nSize);
    //Read and parse AVIH
    if(!parserAVICallbakGetData(*offset,10 * sizeof(avi_uint32),
      m_ReadBuffer,AVI_READ_BUFFER_SIZE,m_pUserData, &retError))
    {
      return retError;
    }
    *offset+= (10 * sizeof(avi_uint32));
    if(m_hAviSummary.avih)
    {
      //Ideally,this should never happen.Print F3 for debug purpose.
      MM_Free(m_hAviSummary.avih);
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,
                   "parseAVIH freeing existing avih..");
    }
    m_hAviSummary.avih = (avi_mainheader_avih*)MM_Malloc(sizeof(avi_mainheader_avih));
    if(!m_hAviSummary.avih)
    {
       MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                   "parseAVIH Malloc failed..AVI_OUT_OF_MEMORY");
      return AVI_OUT_OF_MEMORY;
    }
    memset(m_hAviSummary.avih,0,sizeof(avi_mainheader_avih));

    memcpy(&m_hAviSummary.avih->dwMicroSecPerFrame,byteData,sizeof(avi_uint32));
    memcpy(&m_hAviSummary.avih->dwMaxBytesPerSec,byteData+4,sizeof(avi_uint32));
    memcpy(&m_hAviSummary.avih->dwPaddingGranularity,byteData+8,sizeof(avi_uint32));
    memcpy(&m_hAviSummary.avih->dwFlags,byteData+12,sizeof(avi_uint32));
    memcpy(&m_hAviSummary.avih->dwTotalFrames,byteData+16,sizeof(avi_uint32));
    memcpy(&m_hAviSummary.avih->dwInitialFrames,byteData+20,sizeof(avi_uint32));
    memcpy(&m_hAviSummary.avih->dwStreams,byteData+24,sizeof(avi_uint32));
    memcpy(&m_hAviSummary.avih->dwSuggestedBufferSize,byteData+28,sizeof(avi_uint32));
    memcpy(&m_hAviSummary.avih->dwWidth,byteData+32,sizeof(avi_uint32));
    memcpy(&m_hAviSummary.avih->dwHeight,byteData+36,sizeof(avi_uint32));

    for(int i = 0; i < 4; i++)
    {
      if(!parserAVICallbakGetData(*offset,
                            sizeof(avi_uint32),
                            m_ReadBuffer,AVI_READ_BUFFER_SIZE,
                            m_pUserData, &retError))
      {
        return retError;
      }
      memcpy(&m_hAviSummary.avih->dwReserved[i],byteData,sizeof(avi_uint32));
      *offset+= sizeof(avi_uint32);
    }
    retError = AVI_SUCCESS;
  }
  else
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                "parseAVIH AVI_CORRUPTED_FILE");
    retError = AVI_CORRUPTED_FILE;
  }
  if(*offset != offsetAtTheEndOfAVIH)
  {
    *offset = offsetAtTheEndOfAVIH;
  }
  return retError;
}
/* =============================================================================
FUNCTION:
  aviParser::GetAudioTrackSummaryInfo

DESCRIPTION:
  Returns audio track summary info for given trackId.

INPUT/OUTPUT PARAMETERS:

RETURN VALUE:

SIDE EFFECTS:
  None.
=============================================================================*/
aviErrorType aviParser::GetAudioTrackSummaryInfo(avi_uint32 trackId,avi_audiotrack_summary_info* info)
{
  aviErrorType retError = AVI_INVALID_USER_DATA;
  if( (!info) || (m_hAviSummary.n_streams <= trackId) )
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                "GetAudioTrackSummaryInfo AVI_INVALID_USER_DATA");
    return AVI_INVALID_USER_DATA;
  }
  for(int i = 0; i< AVI_MAX_TRACKS; i++)
  {
    if( (m_hAviSummary.stream_index[i].index == trackId)&&
        (m_hAviSummary.stream_index[i].type == AVI_CHUNK_AUDIO))
    {
      info->audioBytesPerSec = m_hAviSummary.audio_info[m_hAviSummary.stream_index[i].audioIndex].strfAudio.nAvgBytesPerSec;
      info->audioFrequency=m_hAviSummary.audio_info[m_hAviSummary.stream_index[i].audioIndex].strfAudio.nSamplesPerSec;
      info->nBlockAlign=m_hAviSummary.audio_info[m_hAviSummary.stream_index[i].audioIndex].strfAudio.nBlockAlign;
      info->nChannels=m_hAviSummary.audio_info[m_hAviSummary.stream_index[i].audioIndex].strfAudio.nChannels;
      info->trackID = (avi_uint8)m_hAviSummary.stream_index[i].index;
      info->wFormatTag=m_hAviSummary.audio_info[m_hAviSummary.stream_index[i].audioIndex].strfAudio.wFormatTag;
      info->nbitsPerAudioSample = m_hAviSummary.audio_info[m_hAviSummary.stream_index[i].audioIndex].strfAudio.wBitsPerSample;

      /* There is no documentation available on how to determine if an audio
         stream is VBR. However, if dwSampleSize = 0, that means each sample
         is of variable size. We assume here that it is made VBR by varying
         the size of each sample and not the duration (or both)
       */
      if (m_hAviSummary.audio_info[m_hAviSummary.stream_index[i].audioIndex].strhAudio.dwSampleSize == 0)
      {
        info->isVbr = true;
        //There are some restrictions on MP3, doing additional check here
        if (info->wFormatTag == AVI_AUDIO_MP3 && info->nBlockAlign != 576 && info->nBlockAlign != 1152)
        {
          info->isVbr = false;
        }
      }
      else
      {
        info->isVbr = false;
      }
      return AVI_SUCCESS;
    }
  }
  return retError;
}
/* =============================================================================
FUNCTION:
  aviParser::GetWMAExtraInfo

DESCRIPTION:
  Returns audio track summary info for given trackId.

INPUT/OUTPUT PARAMETERS:

RETURN VALUE:

SIDE EFFECTS:
  None.
=============================================================================*/
aviErrorType aviParser::GetWMAExtraInfo(uint32 trackId,avi_header_strf_wma_extra* info)
{
  aviErrorType retError = AVI_UNKNOWN_ERROR;
  if(info && m_pWmaAudioInfo)
  {
    memcpy(info,m_pWmaAudioInfo,sizeof(avi_header_strf_wma_extra));
    retError = AVI_SUCCESS;
  }
  return retError;
}
/* =============================================================================
FUNCTION:
  aviParser::parseSTRD

DESCRIPTION:
 Reads and parse STRD header.

INPUT/OUTPUT PARAMETERS:

RETURN VALUE:

SIDE EFFECTS:
  None.
=============================================================================*/
aviErrorType aviParser::parseSTRD(avi_uint64* offset, uint32 ulMaxSize)
{
  unsigned char* byteData = m_ReadBuffer;
  avi_uint32 nSize = 0;
  avi_uint32 nVersion = 0;
  avi_uint32 length = 0;
  avi_uint64 drmOffset = 0;
  aviErrorType retError = AVI_READ_FAILURE;
  bool isDivxCodec = false;

#ifdef FEATURE_FILESOURCE_AVI_PARSER_DEBUG
  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW,
                "parseSTRD Offset to start %d",*offset);
#endif

  if(!parserAVICallbakGetData(*offset,3 * sizeof(avi_uint32),m_ReadBuffer,
                              AVI_READ_BUFFER_SIZE,m_pUserData, &retError))
  {
    return retError;
  }
#ifdef FEATURE_FILESOURCE_AVI_DIVX_PARSER
  if ( (!memcmp(&m_hAviSummary.video_info->strhVideo.fccHandler,"divx",sizeof(fourCC_t))) ||
       (!memcmp(&m_hAviSummary.video_info->strhVideo.fccHandler,"DIVX",sizeof(fourCC_t))) ||
       (!memcmp(&m_hAviSummary.video_info->strhVideo.fccHandler,"dx50",sizeof(fourCC_t))) ||
       (!memcmp(&m_hAviSummary.video_info->strhVideo.fccHandler,"DX50",sizeof(fourCC_t))) ||
       (!memcmp(&m_hAviSummary.video_info->strhVideo.fccHandler,"div3",sizeof(fourCC_t))) ||
       (!memcmp(&m_hAviSummary.video_info->strhVideo.fccHandler,"DIV3",sizeof(fourCC_t))) ||
       (!memcmp(&m_hAviSummary.video_info->strhVideo.fccHandler,"div4",sizeof(fourCC_t))) ||
       (!memcmp(&m_hAviSummary.video_info->strhVideo.fccHandler,"DIV4",sizeof(fourCC_t)))
       )
  {
    isDivxCodec = true;
  }
#endif

  if(isDivxCodec)
  {
    m_bDRMProtection = true;
    *offset+= (3 *sizeof(avi_uint32));
    memcpy(&length,byteData,sizeof(avi_uint32));
    memcpy(&nVersion,byteData+sizeof(avi_uint32),sizeof(avi_uint32));
    memcpy(&nSize,byteData+(2*sizeof(avi_uint32)),sizeof(avi_uint32));
    bool bRetError = ValidateChunkSize(nSize, ulMaxSize);
    if(bRetError)
    {
      retError = AVI_CORRUPTED_FILE;
      return retError;
    }

    drmOffset = *offset;

    m_AviSTRDHeader.version = nVersion;
    m_AviSTRDHeader.drm_size = nSize;
    m_AviSTRDHeader.drm_offset = (int)drmOffset;
    if(m_AviSTRDHeader.drm_info)
    {
      MM_Free(m_AviSTRDHeader.drm_info);
    }
    /* Check whether DRM size value is less than STRD atom size or not. */
    if( m_AviSTRDHeader.drm_size < (int)length)
    {
      m_AviSTRDHeader.drm_info = (avi_uint8*)MM_Malloc(nSize);
      if(!m_AviSTRDHeader.drm_info)
      {
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                     "Memory allocation failed for DRM Info chunk");
        return AVI_OUT_OF_MEMORY;
      }
      if(!parserAVICallbakGetData(*offset,nSize,m_AviSTRDHeader.drm_info,
                                  m_AviSTRDHeader.drm_size,m_pUserData,
                                  &retError))
      {
        return retError;
      }
    }
    else
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                  "DRM info size is greater than STRD chunk Size");
      m_AviSTRDHeader.drm_size = 0;
      return AVI_PARSE_ERROR;
    }
    retError = AVI_SUCCESS;
  }
  else
  {
    *offset+= sizeof(avi_uint32);
    memcpy(&nSize,byteData,sizeof(avi_uint32));
    bool bRetError = ValidateChunkSize(nSize, ulMaxSize);
    if(bRetError)
    {
      retError = AVI_CORRUPTED_FILE;
      return retError;
    }
    retError = AVI_SUCCESS;
  }
  *offset+= nSize;
  return retError;
}
/* =============================================================================
FUNCTION:
  aviParser::parseSTRL

DESCRIPTION:
 Reads and parse STRL header.

INPUT/OUTPUT PARAMETERS:

RETURN VALUE:

SIDE EFFECTS:
  None.
=============================================================================*/
aviErrorType aviParser::parseSTRL(avi_uint64* offset,avi_uint32 nListSize)
{
  unsigned char* byteData = m_ReadBuffer;
  avi_uint32 nSize = 0;
  fourCC_t fccType = 0;
  aviErrorType retError = AVI_READ_FAILURE;

  /*
  * We would have already read STRL fourCC as that makes us call parseSTRL.
  * Hence subtract fourCC_t size from the total size.
  * This is used if we encounter any error in parsing STRL as we can skip entire
  * STRL with out parsing its children.
  */
  avi_uint64 offsetAtTheEndOfSTRL = (*offset + (nListSize - sizeof(fourCC_t)) );

  #ifdef FEATURE_FILESOURCE_AVI_PARSER_DEBUG
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW,
                "parseSTRL Offset to start %d",*offset);
  #endif
  if(!parserAVICallbakGetData(*offset,3 * sizeof(fourCC_t),
    m_ReadBuffer,AVI_READ_BUFFER_SIZE,m_pUserData, &retError))
  {
    return retError;
  }
  *offset+= (3 *sizeof(fourCC_t));
  if(memcmp(byteData,AVI_STRH_FOURCC,sizeof(fourCC_t)))
  {
    retError = AVI_CORRUPTED_FILE;
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                "parseSTRL AVI_CORRUPTED_FILE");
    return retError;
  }
  memcpy(&nSize,byteData+4,sizeof(fourCC_t));
  bool bRetError = ValidateChunkSize(nSize, nListSize);
  if(bRetError)
  {
    retError = AVI_CORRUPTED_FILE;
    return retError;
  }

  memcpy(&fccType,byteData+8,sizeof(fourCC_t));

  if(!memcmp(&fccType,"vids",sizeof(fourCC_t)) && (m_hAviSummary.n_streams < AVI_MAX_TRACKS) )
  {
    m_hAviSummary.n_streams++;
    if(m_hAviSummary.n_video_tracks >= AVI_MAX_VIDEO_TRACKS)
    {
       MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW, "Not parsing Video/Subtitle Track as # video tracks > %d",AVI_MAX_VIDEO_TRACKS);
      //Skip rest of the STRL and continue parsing..
      *offset = offsetAtTheEndOfSTRL;
      return AVI_SUCCESS;
    }

    /*
    * Video and Caption track are marked as "vids".
    * Need to look at "biCompression" to distinguish.
    * For time being, we will treat it as video track and
    * once we have biCompression, we will adjust the stream information.
    */
    avi_video_info tempInfo;
    memset(&tempInfo,0,sizeof(avi_video_info));
    retError = parseVideoSTRH(offset,nSize,&tempInfo);
    if(retError == AVI_SUCCESS)
    {
      m_hAviSummary.n_video_tracks++;
      m_hAviSummary.stream_index[m_hAviSummary.nNextAvailTrackIndex].videoIndex =
      m_hAviSummary.nCurrVideoTrackInfoIndex;
      m_hAviSummary.stream_index[m_hAviSummary.nNextAvailTrackIndex].bParsed = true;
      m_hAviSummary.nCurrVideoTrackInfoIndex++;

      #ifdef FEATURE_FILESOURCE_AVI_PARSER_DEBUG
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW, "parseSTRL VIDEO STRL");
      #endif
      m_hAviSummary.stream_index[m_hAviSummary.nNextAvailTrackIndex].index =
      m_hAviSummary.nNextAvailTrackIndex;
      m_hAviSummary.stream_index[m_hAviSummary.nNextAvailTrackIndex].type=   AVI_CHUNK_VIDEO;
      m_hAviSummary.nNextAvailTrackIndex++;
    }//if(retError == AVI_SUCCESS)
    while ((*offset < m_nFileSize) && (retError == AVI_SUCCESS) &&
           (*offset < offsetAtTheEndOfSTRL))
    {
      if(!parserAVICallbakGetData(*offset, 2*sizeof(fourCC_t),
         m_ReadBuffer, AVI_READ_BUFFER_SIZE,m_pUserData, &retError))
      {
        return retError;
      }
      *offset+=sizeof(fourCC_t);
      if(!memcmp(byteData,AVI_STRF_FOURCC,sizeof(fourCC_t) ) )
      {
        retError = parseVideoSTRF(offset,&tempInfo,nListSize);
        if (retError != AVI_SUCCESS)
        {
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW, "parseSTRL VIDEO STRF Failed..");
        }
        else
        {
          //check if we really have video track or subtitle track
          if( (( !memcmp(&tempInfo.strfVideo.biCompression,"DXSB",sizeof(avi_int32)) ) ||
              ( !memcmp(&tempInfo.strfVideo.biCompression,"DXSA",sizeof(avi_int32)) )) &&
              (m_hAviSummary.nCurrTextTrackInfoIndex < AVI_MAX_VIDEO_TRACKS) )
          {
            //The one we thought as VIDEO is actually sub-title.
            //Update our records to reflect this information.
            uint8 tempId = m_hAviSummary.nNextAvailTrackIndex -1;
            m_hAviSummary.stream_index[tempId].type = AVI_CHUNK_BITMAP_CAPTION;
            m_hAviSummary.stream_index[tempId].videoIndex = 0;
            m_hAviSummary.stream_index[tempId].bParsed = false;
            m_hAviSummary.nCurrTextTrackInfoIndex++;
            m_hAviSummary.n_text_tracks++;
            #ifdef FEATURE_FILESOURCE_AVI_PARSER_DEBUG
              MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW,"parseSTRL AVI_CHUNK_BITMAP_CAPTION STRL");
            #endif
            if (m_hAviSummary.nCurrTextTrackInfoIndex <= AVI_MAX_VIDEO_TRACKS)
            {
              memcpy(&m_hAviSummary.text_info[m_hAviSummary.nCurrTextTrackInfoIndex-1], &tempInfo,sizeof(avi_video_info));
            }
          }
          else
          {
            //It's a Video track, copy over strh/strf/strd/strn if any.
            memcpy(&m_hAviSummary.video_info[m_hAviSummary.nCurrVideoTrackInfoIndex-1], &tempInfo,sizeof(avi_video_info));
          }
        }
      }
      else if(!memcmp(byteData,AVI_STRN_FOURCC,sizeof(fourCC_t) ) )
      {
        retError = parseVideoSTRN(offset,&tempInfo, nListSize);
        if (retError != AVI_SUCCESS)
        {
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW, "parseSTRL VIDEO STRN Failed..");
        }
        else
        {
          if( (tempInfo.strnVideo.streamNameSize && tempInfo.strnVideo.streamName)&&
              (memcmp(&tempInfo.strfVideo.biCompression,"DXSB",sizeof(avi_int32))!= 0) )
          {
            //It's a Video track, copy over strh/strf/strd/strn if any.
            memcpy(&m_hAviSummary.video_info[m_hAviSummary.nCurrVideoTrackInfoIndex-1], &tempInfo,sizeof(avi_video_info));
          }
        }
      }
      else if(!memcmp(byteData,AVI_STRD_FOURCC,sizeof(fourCC_t)))
      {
        //Clip has 'strd' chunk,meaning it's DRM protected.
        if( (retError = parseSTRD(offset, nListSize))!= AVI_SUCCESS )
        {
          if(tempInfo.strfVideo.pExtraData)
          {
            MM_Free(tempInfo.strfVideo.pExtraData);
            tempInfo.strfVideo.pExtraData = NULL;
          }
          MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_FATAL,
            "StartParsing parseSTRD failed..retError %d",retError);
          return retError;
        }
        #ifdef FEATURE_FILESOURCE_AVI_PARSER_DEBUG
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW,"StartParsing done parsing STRD");
        #endif
      }
      // No need to parse index if the m_bDiscardVideoIndex variable is set
      else if(!memcmp(byteData,AVI_INDX_FOURCC,sizeof(fourCC_t)) && !m_bDiscardVideoIndex)
      {
        memcpy(&nSize, byteData + sizeof(avi_uint32), sizeof(avi_uint32));
        bool bRetError = ValidateChunkSize(nSize, nListSize);
        if(bRetError)
        {
          retError = AVI_CORRUPTED_FILE;
          return retError;
        }
        retError = parseINDX(*offset);
        *offset +=sizeof(fourCC_t) + nSize;
      }
      else
      {
        memcpy(&nSize, byteData + sizeof(avi_uint32), sizeof(avi_uint32));
        bool bRetError = ValidateChunkSize(nSize, nListSize);
        if(bRetError)
        {
          retError = AVI_CORRUPTED_FILE;
          return retError;
        }
        *offset +=sizeof(fourCC_t) + nSize;
      }
    }//while ((offset < m_nFileSize) && (retError == AVI_SUCCESS))
  }//if(!memcmp(&fccType,"vids",sizeof(fourCC_t)))
  else if(!memcmp(&fccType,"auds",sizeof(fourCC_t)) && (m_hAviSummary.n_streams < AVI_MAX_TRACKS) )
  {
    m_hAviSummary.n_streams++;
    m_hAviSummary.n_audio_tracks++;
    if(m_hAviSummary.n_audio_tracks > AVI_MAX_AUDIO_TRACKS)
    {
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW, "Not parsing Audio Track as # audio tracks > %d",AVI_MAX_AUDIO_TRACKS);
      //Skip rest of the STRL and continue parsing..
      *offset = offsetAtTheEndOfSTRL;
      return AVI_SUCCESS;
    }
    else
    {
      m_hAviSummary.stream_index[m_hAviSummary.nNextAvailTrackIndex].audioIndex =
      m_hAviSummary.nCurrAudioTrackInfoIndex;
      m_hAviSummary.stream_index[m_hAviSummary.nNextAvailTrackIndex].bParsed = true;

      retError = parseAudioSTRH(offset,nSize);

    }
    m_hAviSummary.stream_index[m_hAviSummary.nNextAvailTrackIndex].index =
                                (avi_uint32)m_hAviSummary.nNextAvailTrackIndex;
    m_hAviSummary.stream_index[m_hAviSummary.nNextAvailTrackIndex].type= AVI_CHUNK_AUDIO;
    m_hAviSummary.nNextAvailTrackIndex++;

    while((*offset < m_nFileSize) && (retError == AVI_SUCCESS) &&
          (*offset < offsetAtTheEndOfSTRL))
    {
      if(!parserAVICallbakGetData(*offset, 2*sizeof(fourCC_t),
        m_ReadBuffer, AVI_READ_BUFFER_SIZE,m_pUserData, &retError))
      {
        return retError;
      }
      *offset+=sizeof(fourCC_t);
      if(!memcmp(byteData,AVI_STRF_FOURCC,sizeof(fourCC_t) ) )
      {
        retError = parseAudioSTRF(offset, nListSize);
        if (retError != AVI_SUCCESS)
        {
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW, "parseSTRL AUDIO STRF Failed..");
        }
      }
      else if(!memcmp(byteData,AVI_STRN_FOURCC,sizeof(fourCC_t) ) )
      {
        retError = parseAudioSTRN(offset, nListSize);
        if (retError != AVI_SUCCESS)
        {
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW, "parseSTRL AUDIO STRN Failed..");
        }
      }
      else if(!memcmp(byteData,AVI_STRD_FOURCC,sizeof(fourCC_t)))
      {
        //Clip has 'strd' chunk,meaning it's DRM protected.
        if( (retError = parseSTRD(offset, nListSize))!= AVI_SUCCESS )
        {
          MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_FATAL,
                      "StartParsing parseSTRD failed..retError %d",retError);
          return retError;
        }
        #ifdef FEATURE_FILESOURCE_AVI_PARSER_DEBUG
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW,"StartParsing done parsing STRD");
        #endif
      }
      // No need to parse index if the m_bDiscardAudioIndex variable is set
      else if(!memcmp(byteData,AVI_INDX_FOURCC,sizeof(fourCC_t)) && !m_bDiscardAudioIndex)
      {
        memcpy(&nSize, byteData + sizeof(avi_uint32), sizeof(avi_uint32));
        retError = parseINDX(*offset);
        *offset +=sizeof(fourCC_t) + nSize;
      }
      else
      {
        memcpy(&nSize, byteData + sizeof(avi_uint32), sizeof(avi_uint32));
        *offset +=sizeof(fourCC_t) + nSize;
      }
    }//while ((offset < m_nFileSize) && (retError == AVI_SUCCESS))
    if(AVI_SUCCESS == retError)
    {
      /*
      * FOR CBR AUDIO, we need nAvgBytesPerSec to be nonzero.
      * If it's zero, signal the failure here itself as we won't be able
      * to calculate time stamp correctly.
      *
      * FOR VBR, we need dwScale,dwRate and nBlockAlign to be non-zero.
      * if any one of them is 0,signal the failure here itself as we won't be able
      * to calculate time stamp correctly.
      */
      retError = AVI_CORRUPTED_FILE;
      int audid = AVI_MAX_TRACKS;
      if( (m_hAviSummary.nNextAvailTrackIndex-1) < AVI_MAX_TRACKS)
      {
        audid = (int)m_hAviSummary.stream_index[m_hAviSummary.nNextAvailTrackIndex-1].index;
      }
      else
      {
        return AVI_INVALID_USER_DATA;
      }
      avi_audiotrack_summary_info audsuminfo;
      memset(&audsuminfo,0,sizeof(avi_audiotrack_summary_info));
      if(GetAudioTrackSummaryInfo(audid,&audsuminfo) == AVI_SUCCESS)
      {
        if((audsuminfo.isVbr) &&
            ((!m_hAviSummary.audio_info[m_hAviSummary.nCurrAudioTrackInfoIndex].strhAudio.dwScale) ||
             (!m_hAviSummary.audio_info[m_hAviSummary.nCurrAudioTrackInfoIndex].strhAudio.dwRate)  ||
             (!m_hAviSummary.audio_info[m_hAviSummary.nCurrAudioTrackInfoIndex].strfAudio.nBlockAlign))
          )
        {
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL, "VBR AUDIO:AVI_CORRUPTED_FILE can't get dwScale|dwRate|nBlockAlign");
        }
        else if( (!audsuminfo.isVbr) &&
                 (!m_hAviSummary.audio_info[m_hAviSummary.nCurrAudioTrackInfoIndex].strfAudio.nAvgBytesPerSec) )
        {
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL, "CBR AUDIO:AVI_CORRUPTED_FILE can't get nAvgBytesPerSec");
        }
        else
        {
          retError = AVI_SUCCESS;
          m_hAviSummary.nCurrAudioTrackInfoIndex++;
          #ifdef FEATURE_FILESOURCE_AVI_PARSER_DEBUG
            MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW, "parseSTRL AUDIO STRL");
          #endif
        }
      }
    }
    else
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW, "parseSTRL AUDIO STRL Failed..");
    }
    return retError;
  }
  else
  {
    if(m_hAviSummary.n_streams < AVI_MAX_TRACKS)
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
                     "parseSTRL encountered non 'auds'/'vids'");
      m_hAviSummary.stream_index[m_hAviSummary.nNextAvailTrackIndex].index =
      (avi_uint32)m_hAviSummary.nNextAvailTrackIndex;
      m_hAviSummary.stream_index[m_hAviSummary.nNextAvailTrackIndex].type = AVI_CHUNK_UNKNOWN;
      m_hAviSummary.nNextAvailTrackIndex++;
    }
    //Skip rest of the STRL and continue parsing..
    *offset = offsetAtTheEndOfSTRL;
    retError = AVI_SUCCESS;
  }
  return retError;
}
/* =============================================================================
FUNCTION:
  aviParser::parseVideoSTRH

DESCRIPTION:
 Reads and parse video track STRH header.

INPUT/OUTPUT PARAMETERS:

RETURN VALUE:

SIDE EFFECTS:
  None.
=============================================================================*/
aviErrorType aviParser::parseVideoSTRH(avi_uint64* offset,
                                       avi_uint64 nMaxSize,
                                       avi_video_info* streaminfo)
{
  unsigned char* byteData = m_ReadBuffer;
  aviErrorType retError = AVI_READ_FAILURE;

  if((!streaminfo)||(!offset))
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                "parseVideoSTRH AVI_INVALID_USER_DATA");
    return AVI_INVALID_USER_DATA;
  }
  avi_uint64 offsetAtStartOfVideoSTRH = *offset;

  #ifdef FEATURE_FILESOURCE_AVI_PARSER_DEBUG
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW,
                  "parseVideoSTRH Offset to start %d",*offset);
  #endif
  if(!parserAVICallbakGetData(*offset,sizeof(fourCC_t),m_ReadBuffer,
    AVI_READ_BUFFER_SIZE,m_pUserData, &retError))
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                "parseVideoSTRH AVI_READ_FAILURE");
    return AVI_READ_FAILURE;
  }
  *offset+=sizeof(fourCC_t);

  memcpy(&(streaminfo->strhVideo.fccType),"vids",sizeof(fourCC_t));
  memcpy(&(streaminfo->strhVideo.fccHandler),byteData,sizeof(fourCC_t));

  if(!parserAVICallbakGetData(*offset,sizeof(avi_int32),m_ReadBuffer,
    AVI_READ_BUFFER_SIZE,m_pUserData, &retError))
  {
    return retError;
  }
  *offset+=sizeof(avi_int32);
  memcpy(&(streaminfo->strhVideo.dwFlags),byteData,sizeof(avi_int32));

  if(!parserAVICallbakGetData(*offset,(2*sizeof(avi_int16)),
    m_ReadBuffer,AVI_READ_BUFFER_SIZE,m_pUserData, &retError))
  {
    return retError;
  }
  *offset+=(2*sizeof(avi_int16));
  memcpy(&(streaminfo->strhVideo.wPriority),byteData,sizeof(avi_int16));
  memcpy(&(streaminfo->strhVideo.wLanguage),byteData+sizeof(avi_int16),sizeof(avi_int16));

  if(!parserAVICallbakGetData(*offset,(8*sizeof(avi_int32)),
    m_ReadBuffer,AVI_READ_BUFFER_SIZE,m_pUserData, &retError))
  {
    return retError;
  }
  *offset+=(8*sizeof(avi_int32));
  int tempOffset = 0;

  memcpy(&(streaminfo->strhVideo.dwInitialFrames),
         byteData+tempOffset,sizeof(avi_int32));
  tempOffset+=sizeof(avi_int32);

  memcpy(&(streaminfo->strhVideo.dwScale),
         byteData+tempOffset,sizeof(avi_int32));
  tempOffset+=sizeof(avi_int32);

  memcpy(&(streaminfo->strhVideo.dwRate),
         byteData+tempOffset,sizeof(avi_int32));
  tempOffset+=sizeof(avi_int32);

  memcpy(&(streaminfo->strhVideo.dwStart),
         byteData+tempOffset,sizeof(avi_int32));
  tempOffset+=sizeof(avi_int32);
  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
               "parseVideoSTRH dwStart %d",streaminfo->strhVideo.dwStart);

  memcpy(&(streaminfo->strhVideo.dwLength),
         byteData+tempOffset,sizeof(avi_int32));
  tempOffset+=sizeof(avi_int32);
  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
               "parseVideoSTRH dwLength %d",streaminfo->strhVideo.dwLength);

  memcpy(&(streaminfo->strhVideo.dwSuggestedBufferSize),
         byteData+tempOffset,sizeof(avi_int32));
  tempOffset+=sizeof(avi_int32);

  memcpy(&(streaminfo->strhVideo.dwQuality),
         byteData+tempOffset,sizeof(avi_int32));
  tempOffset+=sizeof(avi_int32);

  memcpy(&(streaminfo->strhVideo.dwSampleSize),
         byteData+tempOffset,sizeof(avi_int32));

  if(!parserAVICallbakGetData(*offset,(4*sizeof(avi_int16)),
    m_ReadBuffer,AVI_READ_BUFFER_SIZE,m_pUserData, &retError))
  {
    return retError;
  }
  *offset+=(4*sizeof(avi_int16));
  tempOffset = 0;

  memcpy(&(streaminfo->strhVideo.rcFrame_left),
         byteData+tempOffset,sizeof(avi_int16));
  tempOffset+=sizeof(avi_int16);

  memcpy(&(streaminfo->strhVideo.rcFrame_top),
         byteData+tempOffset,sizeof(avi_int16));
  tempOffset+=sizeof(avi_int16);

  memcpy(&(streaminfo->strhVideo.rcFrame_right),
         byteData+tempOffset,sizeof(avi_int16));
  tempOffset+=sizeof(avi_int16);

  memcpy(&(streaminfo->strhVideo.rcFrame_bottom),
         byteData+tempOffset,sizeof(avi_int16));
  streaminfo->strhVideo.present = 1;

  retError = AVI_SUCCESS;

  if(*offset != (offsetAtStartOfVideoSTRH + nMaxSize - sizeof(fourCC_t) ))
  {
    *offset = (offsetAtStartOfVideoSTRH + nMaxSize - sizeof(fourCC_t));
  }
  return retError;
}
/* =============================================================================
FUNCTION:
  aviParser::parseAudioSTRH

DESCRIPTION:
 Reads and parse audio track STRH header.

INPUT/OUTPUT PARAMETERS:

RETURN VALUE:

SIDE EFFECTS:
  None.
=============================================================================*/
aviErrorType aviParser::parseAudioSTRH(avi_uint64* offset,avi_uint64 nMaxSize)
{
  unsigned char* byteData = m_ReadBuffer;
  avi_uint64 offsetAtStartOfAudioSTRH = *offset;
  aviErrorType retError = AVI_READ_FAILURE;

  #ifdef FEATURE_FILESOURCE_AVI_PARSER_DEBUG
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW,
                "parseAudioSTRH Offset to start %d",*offset);
  #endif
  if(!parserAVICallbakGetData(*offset,sizeof(fourCC_t),
    m_ReadBuffer,AVI_READ_BUFFER_SIZE,m_pUserData, &retError))
  {
    return retError;
  }
  *offset+=sizeof(fourCC_t);

  memcpy(&(m_hAviSummary.audio_info[m_hAviSummary.nCurrAudioTrackInfoIndex].strhAudio.fccType),
         "auds",sizeof(fourCC_t));
  memcpy(&(m_hAviSummary.audio_info[m_hAviSummary.nCurrAudioTrackInfoIndex].strhAudio.fccHandler),
         byteData,sizeof(fourCC_t));

  if(!parserAVICallbakGetData(*offset,sizeof(avi_int32),
    m_ReadBuffer,AVI_READ_BUFFER_SIZE,m_pUserData, &retError))
  {
    return retError;
  }
  *offset+=sizeof(avi_int32);
  memcpy(&(m_hAviSummary.audio_info[m_hAviSummary.nCurrAudioTrackInfoIndex].strhAudio.dwFlags),
         byteData,sizeof(avi_int32));

  if(!parserAVICallbakGetData(*offset,(2*sizeof(avi_int16)),
    m_ReadBuffer,AVI_READ_BUFFER_SIZE,m_pUserData, &retError))
  {
    return retError;
  }
  *offset+=(2*sizeof(avi_int16));
  memcpy(&(m_hAviSummary.audio_info[m_hAviSummary.nCurrAudioTrackInfoIndex].strhAudio.wPriority),
         byteData,sizeof(avi_int16));
  memcpy(&(m_hAviSummary.audio_info[m_hAviSummary.nCurrAudioTrackInfoIndex].strhAudio.wLanguage),
         byteData+sizeof(avi_int16),sizeof(avi_int16));

  if(!parserAVICallbakGetData(*offset,(8*sizeof(avi_int32)),
    m_ReadBuffer,AVI_READ_BUFFER_SIZE,m_pUserData, &retError))
  {
    return retError;
  }
  *offset+=(8*sizeof(avi_int32));
  int tempOffset = 0;

  memcpy(&(m_hAviSummary.audio_info[m_hAviSummary.nCurrAudioTrackInfoIndex].strhAudio.dwInitialFrames),
         byteData+tempOffset,sizeof(avi_int32));
  tempOffset+=sizeof(avi_int32);

  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
               "parseAudioSTRH dwInitialFrames %d",
               m_hAviSummary.audio_info[m_hAviSummary.nCurrAudioTrackInfoIndex].strhAudio.dwInitialFrames);

  memcpy(&(m_hAviSummary.audio_info[m_hAviSummary.nCurrAudioTrackInfoIndex].strhAudio.dwScale),
         byteData+tempOffset,sizeof(avi_int32));
  tempOffset+=sizeof(avi_int32);

  memcpy(&(m_hAviSummary.audio_info[m_hAviSummary.nCurrAudioTrackInfoIndex].strhAudio.dwRate),
         byteData+tempOffset,sizeof(avi_int32));
  tempOffset+=sizeof(avi_int32);

  memcpy(&(m_hAviSummary.audio_info[m_hAviSummary.nCurrAudioTrackInfoIndex].strhAudio.dwStart),
         byteData+tempOffset,sizeof(avi_int32));
  tempOffset+=sizeof(avi_int32);
  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
               "parseAudioSTRH dwStart %d",
               m_hAviSummary.audio_info[m_hAviSummary.nCurrAudioTrackInfoIndex].strhAudio.dwStart);

  memcpy(&(m_hAviSummary.audio_info[m_hAviSummary.nCurrAudioTrackInfoIndex].strhAudio.dwLength),
         byteData+tempOffset,sizeof(avi_int32));
  tempOffset+=sizeof(avi_int32);

  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
               "parseAudioSTRH dwLength %d",
               m_hAviSummary.audio_info[m_hAviSummary.nCurrAudioTrackInfoIndex].strhAudio.dwLength);

  memcpy(&(m_hAviSummary.audio_info[m_hAviSummary.nCurrAudioTrackInfoIndex].strhAudio.dwSuggestedBufferSize),
         byteData+tempOffset,sizeof(avi_int32));
  tempOffset+=sizeof(avi_int32);

  memcpy(&(m_hAviSummary.audio_info[m_hAviSummary.nCurrAudioTrackInfoIndex].strhAudio.dwQuality),
         byteData+tempOffset,sizeof(avi_int32));
  tempOffset+=sizeof(avi_int32);

  memcpy(&(m_hAviSummary.audio_info[m_hAviSummary.nCurrAudioTrackInfoIndex].strhAudio.dwSampleSize),
         byteData+tempOffset,sizeof(avi_int32));

  if(!parserAVICallbakGetData(*offset,(4*sizeof(avi_int16)),
    m_ReadBuffer,AVI_READ_BUFFER_SIZE,m_pUserData, &retError))
  {
    return retError;
  }
  *offset+=(4*sizeof(avi_int16));
  tempOffset = 0;

  memcpy(&(m_hAviSummary.audio_info[m_hAviSummary.nCurrAudioTrackInfoIndex].strhAudio.rcFrame_left),
         byteData+tempOffset,sizeof(avi_int16));
  tempOffset+=sizeof(avi_int16);

  memcpy(&(m_hAviSummary.audio_info[m_hAviSummary.nCurrAudioTrackInfoIndex].strhAudio.rcFrame_top),
         byteData+tempOffset,sizeof(avi_int16));
  tempOffset+=sizeof(avi_int16);

  memcpy(&(m_hAviSummary.audio_info[m_hAviSummary.nCurrAudioTrackInfoIndex].strhAudio.rcFrame_right),
         byteData+tempOffset,sizeof(avi_int16));
  tempOffset+=sizeof(avi_int16);

  memcpy(&(m_hAviSummary.audio_info[m_hAviSummary.nCurrAudioTrackInfoIndex].strhAudio.rcFrame_bottom),
         byteData+tempOffset,sizeof(avi_int16));
  m_hAviSummary.audio_info[m_hAviSummary.nCurrAudioTrackInfoIndex].strhAudio.present = 1;

  if(*offset != (offsetAtStartOfAudioSTRH + nMaxSize - sizeof(fourCC_t) ))
  {
    *offset = (offsetAtStartOfAudioSTRH + nMaxSize - sizeof(fourCC_t));
  }

  retError = AVI_SUCCESS;
  return retError;
}
/* =============================================================================
FUNCTION:
  aviParser::parseAudioSTRF

DESCRIPTION:
 Reads and parse audio track STRF header.

INPUT/OUTPUT PARAMETERS:

RETURN VALUE:

SIDE EFFECTS:
  None.
=============================================================================*/
aviErrorType aviParser::parseAudioSTRF(avi_uint64* offset, uint32 ulMaxSize)
{
  unsigned char* byteData = m_ReadBuffer;
  avi_uint32 nSize = 0;
  aviErrorType retError = AVI_CORRUPTED_FILE;
  avi_uint64 offsetAtTheEndOfAudioSTRF = 0;

  #ifdef FEATURE_FILESOURCE_AVI_PARSER_DEBUG
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW,
                  "parseAudioSTRF Offset to start %d",*offset);
  #endif

  if(!parserAVICallbakGetData(*offset,sizeof(avi_uint32),
    m_ReadBuffer,AVI_READ_BUFFER_SIZE,m_pUserData, &retError))
  {
    return retError;
  }
  *offset+=sizeof(avi_uint32);

  memcpy(&nSize,byteData,sizeof(avi_uint32));
  bool bRetError = ValidateChunkSize(nSize, ulMaxSize);
  if(bRetError)
  {
    retError = AVI_CORRUPTED_FILE;
    return retError;
  }
  offsetAtTheEndOfAudioSTRF = (*offset + nSize);

  if(!parserAVICallbakGetData(*offset,(2*sizeof(avi_int16)),
    m_ReadBuffer,AVI_READ_BUFFER_SIZE,m_pUserData, &retError))
  {
    return retError;
  }
  *offset+=(2*sizeof(avi_int16));

  memcpy(&(m_hAviSummary.audio_info[m_hAviSummary.nCurrAudioTrackInfoIndex].strfAudio.wFormatTag),
         byteData,sizeof(avi_int16));

  memcpy(&(m_hAviSummary.audio_info[m_hAviSummary.nCurrAudioTrackInfoIndex].strfAudio.nChannels),
         byteData+sizeof(avi_int16),sizeof(avi_int16));

  if(!parserAVICallbakGetData(*offset,(2*sizeof(avi_int32)),
    m_ReadBuffer,AVI_READ_BUFFER_SIZE,m_pUserData, &retError))
  {
    return retError;
  }
  *offset+=(2*sizeof(avi_int32));
  memcpy(&(m_hAviSummary.audio_info[m_hAviSummary.nCurrAudioTrackInfoIndex].strfAudio.nSamplesPerSec),
         byteData,sizeof(avi_int32));

  memcpy(&(m_hAviSummary.audio_info[m_hAviSummary.nCurrAudioTrackInfoIndex].strfAudio.nAvgBytesPerSec),
         byteData+sizeof(avi_int32),sizeof(avi_int32));


  if(!parserAVICallbakGetData(*offset,(2*sizeof(avi_int16)),
    m_ReadBuffer,AVI_READ_BUFFER_SIZE,m_pUserData, &retError))
  {
    return retError;
  }
  *offset+=(2*sizeof(avi_int16));

  memcpy(&(m_hAviSummary.audio_info[m_hAviSummary.nCurrAudioTrackInfoIndex].strfAudio.nBlockAlign),
         byteData,sizeof(avi_int16));

  memcpy(&(m_hAviSummary.audio_info[m_hAviSummary.nCurrAudioTrackInfoIndex].strfAudio.wBitsPerSample),
         byteData+sizeof(avi_int16),sizeof(avi_int16));

  if(nSize > MIN_AUDIO_STRF_HDR_SIZE)
  {
    if(!parserAVICallbakGetData(*offset,sizeof(avi_int16),
      m_ReadBuffer,AVI_READ_BUFFER_SIZE,m_pUserData, &retError))
    {
      return retError;
    }
    *offset+=sizeof(avi_int16);

    memcpy(&(m_hAviSummary.audio_info[m_hAviSummary.nCurrAudioTrackInfoIndex].strfAudio.cbSize),
           byteData,sizeof(avi_int16));

    if(m_hAviSummary.audio_info[m_hAviSummary.nCurrAudioTrackInfoIndex].strfAudio.cbSize)
    {
      m_hAviSummary.audio_info[m_hAviSummary.nCurrAudioTrackInfoIndex].strfAudio.extra = (avi_uint8*)MM_Malloc((sizeof(avi_uint8))*(m_hAviSummary.audio_info[m_hAviSummary.nCurrAudioTrackInfoIndex].strfAudio.cbSize));

      //Check if it is a valid pointer
      if(m_hAviSummary.audio_info[m_hAviSummary.nCurrAudioTrackInfoIndex].strfAudio.extra)
      {
        for(int i=0; i < m_hAviSummary.audio_info[m_hAviSummary.nCurrAudioTrackInfoIndex].strfAudio.cbSize; i++)
        {
          if(!parserAVICallbakGetData(*offset,sizeof(avi_uint8),
            m_ReadBuffer,AVI_READ_BUFFER_SIZE,m_pUserData, &retError))
          {
            return retError;
          }
          *offset+=sizeof(avi_uint8);
          memcpy(&(m_hAviSummary.audio_info[m_hAviSummary.nCurrAudioTrackInfoIndex].strfAudio.extra[i]),
                 byteData,sizeof(avi_uint8));
        }
        retError = parseSTRFExtraData(m_hAviSummary.audio_info[m_hAviSummary.nCurrAudioTrackInfoIndex].strfAudio.extra,m_hAviSummary.audio_info[m_hAviSummary.nCurrAudioTrackInfoIndex].strfAudio.cbSize);
        if(retError != AVI_SUCCESS)
        {
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,"parseSTRFExtraData failed");
          return retError;
        }
      }
      else
      {
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                     "parseAudioSTRF Malloc failed");
        return AVI_OUT_OF_MEMORY;
      }
    }
  }
  else
  {
    m_hAviSummary.audio_info[m_hAviSummary.nCurrAudioTrackInfoIndex].strfAudio.cbSize = 0;
  }
  retError = AVI_SUCCESS;
  *offset = offsetAtTheEndOfAudioSTRF;

  return retError;
}
/* =============================================================================
FUNCTION:
  aviParser::parseSTRFExtraData

DESCRIPTION:
  Parse track STRF header extra info.

INPUT/OUTPUT PARAMETERS:

RETURN VALUE:

SIDE EFFECTS:
  None.
=============================================================================*/
aviErrorType aviParser::parseSTRFExtraData(avi_uint8* byteData,uint16 size)
{
  aviErrorType retError = AVI_UNKNOWN_ERROR;
  avi_audiotrack_summary_info audioSummaryInfo;
  uint32 trackid = 0;
  uint32 audioIndex = 0;

  if(byteData)
  {
    for(int i=0;i<AVI_MAX_TRACKS;i++)
    {
      if(m_hAviSummary.stream_index[i].type == AVI_CHUNK_AUDIO)
      {
        trackid = m_hAviSummary.stream_index[i].index;
        audioIndex = m_hAviSummary.stream_index[i].audioIndex;

        if(m_hAviSummary.audio_info[audioIndex].strfAudio.wFormatTag == AVI_AUDIO_WMA_STD)
        {
          m_pWmaAudioInfo = (avi_header_strf_wma_extra*) MM_Malloc (sizeof(avi_header_strf_wma_extra));
          if(m_pWmaAudioInfo)
          {
            memset(m_pWmaAudioInfo,0,sizeof(avi_header_strf_wma_extra));
            //First 4 bytes are samples/block
            memcpy(&(m_pWmaAudioInfo->nEncodeOpt),byteData+4,sizeof(avi_uint16));
            if(GetAudioTrackSummaryInfo(trackid,&audioSummaryInfo) == AVI_SUCCESS)
            {
              if(audioSummaryInfo.nChannels == 2)
              {
                m_pWmaAudioInfo->dwChannelMask = 3;
              }
              else if(audioSummaryInfo.nChannels == 1)
              {
                m_pWmaAudioInfo->dwChannelMask = 4;
              }
            }
            retError = AVI_SUCCESS;
          }
          else
          {
            retError = AVI_OUT_OF_MEMORY;
          }
        }
        else if(m_hAviSummary.audio_info[audioIndex].strfAudio.wFormatTag == AVI_AUDIO_WMA_PRO)
        {
          m_pWmaAudioInfo = (avi_header_strf_wma_extra*) MM_Malloc (sizeof(avi_header_strf_wma_extra));
          if(m_pWmaAudioInfo)
          {
            memset(m_pWmaAudioInfo,0,sizeof(avi_header_strf_wma_extra));
            //First 2 bytes reserved
            memcpy(&(m_pWmaAudioInfo->dwChannelMask),byteData+2,sizeof(avi_uint32));
            //Next 4 bytes are reserved DWORDs
            memcpy(&(m_pWmaAudioInfo->nAdvancedEncodeOpt2),byteData+ 10,sizeof(avi_uint32));
            memcpy(&(m_pWmaAudioInfo->nEncodeOpt),byteData+14,sizeof(avi_uint16));
            memcpy(&(m_pWmaAudioInfo->nAdvancedEncodeOpt),byteData+16,sizeof(avi_uint16));
            retError = AVI_SUCCESS;
          }
          else
          {
            retError = AVI_OUT_OF_MEMORY;
          }
        }
        else
        {
          retError = AVI_SUCCESS;
        }
        break;
      }
    }
  }
  return retError;
}
/* =============================================================================
FUNCTION:
  aviParser::parseVideoSTRF

DESCRIPTION:
 Reads and parse video track STRF header.

INPUT/OUTPUT PARAMETERS:

RETURN VALUE:

SIDE EFFECTS:
  None.
=============================================================================*/
aviErrorType aviParser::parseVideoSTRF(avi_uint64* offset,avi_video_info* streaminfo,
    uint32 ulMaxSize)
{
  unsigned char* byteData = m_ReadBuffer;
  avi_uint32 nSize = 0;
  aviErrorType retError = AVI_CORRUPTED_FILE;
  avi_uint64 offsetAtTheEndOfVideoSTRF = 0;

  #ifdef FEATURE_FILESOURCE_AVI_PARSER_DEBUG
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW,
                  "parseVideoSTRF Offset to start %d",*offset);
  #endif

  if((!streaminfo)||(!offset))
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                "parseVideoSTRF AVI_INVALID_USER_DATA");
    return AVI_INVALID_USER_DATA;
  }
  //read 4 bytes to get the size of STRF
  if(!parserAVICallbakGetData(*offset,sizeof(avi_uint32),
    m_ReadBuffer,AVI_READ_BUFFER_SIZE,m_pUserData, &retError))
  {
    return retError;
  }
  *offset+=sizeof(avi_uint32);

   memcpy(&nSize,byteData,sizeof(avi_uint32));
   offsetAtTheEndOfVideoSTRF = (*offset + nSize);

  bool bRetError = ValidateChunkSize(nSize, ulMaxSize);
  if(bRetError)
  {
    retError = AVI_CORRUPTED_FILE;
    return retError;
  }

  //Parse STRF header now
  //biSize is same as STRF size found earlier, no need to read, just increment the offset by 4
  streaminfo->strfVideo.biSize = nSize;
  *offset+=sizeof(avi_uint32);

  if(!parserAVICallbakGetData(*offset,(2*sizeof(avi_int32)),
    m_ReadBuffer,AVI_READ_BUFFER_SIZE,m_pUserData, &retError))
  {
    return retError;
  }
  *offset+=(2*sizeof(avi_int32));

  memcpy(&(streaminfo->strfVideo.biWidth),byteData,sizeof(avi_int32));

  memcpy(&(streaminfo->strfVideo.biHeight),
       byteData+sizeof(avi_int32),sizeof(avi_int32));

  if(!parserAVICallbakGetData(*offset,(2*sizeof(avi_int16)),
    m_ReadBuffer,AVI_READ_BUFFER_SIZE,m_pUserData, &retError))
  {
    return retError;
  }
  *offset+=(2*sizeof(avi_int16));

  memcpy(&(streaminfo->strfVideo.biPlanes),byteData,sizeof(avi_int16));

  memcpy(&(streaminfo->strfVideo.biBitCount),
       byteData+sizeof(avi_int16),sizeof(avi_int16));


  if(!parserAVICallbakGetData(*offset,(6*sizeof(avi_int32)),
    m_ReadBuffer,AVI_READ_BUFFER_SIZE,m_pUserData, &retError))
  {
    return retError;
  }
  *offset+=(6*sizeof(avi_int32));

  memcpy(&(streaminfo->strfVideo.biCompression),byteData,sizeof(avi_int32));

  memcpy(&(streaminfo->strfVideo.biSizeImage),
       byteData+sizeof(avi_int32),sizeof(avi_int32));

  memcpy(&(streaminfo->strfVideo.biXPelsPerMeter),
       byteData+(2*sizeof(avi_int32)),sizeof(avi_int32));
  memcpy(&(streaminfo->strfVideo.biYPelsPerMeter),
       byteData+(3*sizeof(avi_int32)),sizeof(avi_int32));
  memcpy(&(streaminfo->strfVideo.biClrUsed),
       byteData+(4*sizeof(avi_int32)),sizeof(avi_int32));
  memcpy(&(streaminfo->strfVideo.biClrImportant),
       byteData+(5*sizeof(avi_int32)),sizeof(avi_int32));
  retError = AVI_SUCCESS;

  //check if there is any extra data?
  if(offsetAtTheEndOfVideoSTRF > *offset)
  {
    avi_int32 nread = parserAVICallbakGetData(*offset,
    (avi_uint32)(offsetAtTheEndOfVideoSTRF - *offset),
    m_ReadBuffer,AVI_READ_BUFFER_SIZE,m_pUserData, &retError);
    //Store the extra data as it might be needed to configure decoder.
    if(nread)
    {
      streaminfo->strfVideo.pExtraData = (avi_uint8*)MM_Malloc(nread);
      if(streaminfo->strfVideo.pExtraData)
      {
        memcpy(streaminfo->strfVideo.pExtraData,m_ReadBuffer,nread);
        streaminfo->strfVideo.nExtraData = nread;
      }
    }//if(nread)
  }//if(offsetAtTheEndOfVideoSTRF > *offset)

  *offset = offsetAtTheEndOfVideoSTRF;
  return retError;
}
/* =============================================================================
FUNCTION:
  aviParser::parseVideoSTRN

DESCRIPTION:
 Reads and parse video track STRN header.

INPUT/OUTPUT PARAMETERS:

RETURN VALUE:

SIDE EFFECTS:
  None.
=============================================================================*/
aviErrorType aviParser::parseVideoSTRN(avi_uint64* offset,avi_video_info* streaminfo,
    uint32 ulMaxSize)
{
  unsigned char* byteData = m_ReadBuffer;
  avi_uint32 nSize = 0;
  aviErrorType retError = AVI_CORRUPTED_FILE;
  avi_uint64 offsetAtTheEndOfVideoSTRN = 0;

  #ifdef FEATURE_FILESOURCE_AVI_PARSER_DEBUG
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW,
                  "parseVideoSTRN Offset to start %d",*offset);
  #endif

  if((!streaminfo)||(!offset))
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                "parseVideoSTRN AVI_INVALID_USER_DATA");
    return AVI_INVALID_USER_DATA;
  }
  if(!parserAVICallbakGetData(*offset,sizeof(avi_uint32),
    m_ReadBuffer,AVI_READ_BUFFER_SIZE,m_pUserData, &retError))
  {
    return retError;
  }
  *offset+=sizeof(avi_uint32);
  memcpy(&nSize,byteData,sizeof(avi_uint32));
  /* make sure nSize is even number (16 bits boundary) */
  bool bRetError = ValidateChunkSize(nSize, ulMaxSize);
  if(bRetError)
  {
    retError = AVI_CORRUPTED_FILE;
    return retError;
  }

  nSize = nSize + (nSize %2);
  offsetAtTheEndOfVideoSTRN = (*offset + nSize);

  if( memcmp(&(streaminfo->strfVideo.biCompression),"DXSB",sizeof(avi_int32)) != 0)
  {
    #ifdef FEATURE_FILESOURCE_AVI_PARSER_DEBUG
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW, "parseVideoSTRN nSize %d",nSize);
    #endif
    retError = AVI_SUCCESS;
    if(nSize > 0)
    {
      if(streaminfo->strnVideo.streamName)
      {
        MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW,
                       "parseVideoSTRN existing streamName is not NULL video trackid %d",
                       m_hAviSummary.nCurrVideoTrackInfoIndex);
        MM_Free(streaminfo->strnVideo.streamName);
      }
      streaminfo->strnVideo.streamName =
        (avi_uint8*)MM_Malloc(nSize+1);
      if(!streaminfo->strnVideo.streamName)
      {
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                       "parseVideoSTRN AVI_OUT_OF_MEMORY");
        return AVI_OUT_OF_MEMORY;
      }
      if(!parserAVICallbakGetData(*offset,nSize,
        streaminfo->strnVideo.streamName,nSize,m_pUserData,
        &retError))
      {
        return retError;
      }
      memcpy(streaminfo->strnVideo.streamName+nSize,"\0",1);
      streaminfo->strnVideo.streamNameSize = nSize;

      //*offset+=nSize;
      retError = AVI_SUCCESS;
    }//if(nSize > 0)
  }//if(!memcmp(&streaminfo.strfVideo.biCompression,"DXSB",sizeof(avi_int32)))
  else
  {
    retError = AVI_SUCCESS;
  }

  *offset = offsetAtTheEndOfVideoSTRN;
  return retError;
}
/* =============================================================================
FUNCTION:
  aviParser::parseAudioSTRN

DESCRIPTION:
 Reads and parse audio track STRN header.

INPUT/OUTPUT PARAMETERS:

RETURN VALUE:

SIDE EFFECTS:
  None.
=============================================================================*/
aviErrorType aviParser::parseAudioSTRN(avi_uint64* offset, uint32 ulMaxSize)
{
  unsigned char* byteData = m_ReadBuffer;
  avi_uint32 nSize = 0;
  aviErrorType retError = AVI_CORRUPTED_FILE;
  avi_uint64 offsetAtTheEndOfAudioSTRN = 0;

  #ifdef FEATURE_FILESOURCE_AVI_PARSER_DEBUG
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW,
                  "parseAudioSTRN Offset to start %d",*offset);
  #endif
  if(!parserAVICallbakGetData(*offset,sizeof(avi_uint32),
    m_ReadBuffer,AVI_READ_BUFFER_SIZE,m_pUserData, &retError))
  {
    return retError;
  }
  *offset+=sizeof(avi_uint32);
  memcpy(&nSize,byteData,sizeof(avi_uint32));
  /* make sure nSize is even number (16 bits boundary) */
  nSize = nSize + (nSize %2);
  bool bRetError = ValidateChunkSize(nSize, ulMaxSize);
  if(bRetError)
  {
    retError = AVI_CORRUPTED_FILE;
    return retError;
  }
  offsetAtTheEndOfAudioSTRN = (*offset + nSize);
  retError = AVI_SUCCESS;

  #ifdef FEATURE_FILESOURCE_AVI_PARSER_DEBUG
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW,
                   "parseAudioSTRN nSize %d",nSize);
  #endif

  if(nSize > 0)
  {
    if(m_hAviSummary.audio_info[m_hAviSummary.nCurrAudioTrackInfoIndex].strnAudio.streamName)
    {
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW,
                    "parseAudioSTRN existing streamName is not NULL audio trackid %d",
                    m_hAviSummary.nCurrAudioTrackInfoIndex);
      MM_Free(m_hAviSummary.audio_info[m_hAviSummary.nCurrAudioTrackInfoIndex].strnAudio.streamName);
    }
    m_hAviSummary.audio_info[m_hAviSummary.nCurrAudioTrackInfoIndex].strnAudio.streamName =
      (avi_uint8*)MM_Malloc(nSize+1);

    if(!m_hAviSummary.audio_info[m_hAviSummary.nCurrAudioTrackInfoIndex].strnAudio.streamName)
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                   "parseAudioSTRN AVI_OUT_OF_MEMORY");
      return AVI_OUT_OF_MEMORY;
    }
    if(!parserAVICallbakGetData(*offset,
                          nSize,
                          m_hAviSummary.audio_info[m_hAviSummary.nCurrAudioTrackInfoIndex].strnAudio.streamName,
                          nSize,
                          m_pUserData, &retError))
    {
      return retError;
    }
    memcpy(m_hAviSummary.audio_info[m_hAviSummary.nCurrAudioTrackInfoIndex].strnAudio.streamName+nSize,
      "\0",1);
    m_hAviSummary.audio_info[m_hAviSummary.nCurrAudioTrackInfoIndex].strnAudio.streamNameSize = nSize;

    //*offset+=nSize;
    retError = AVI_SUCCESS;
  }//if(nSize > 0)
  *offset = offsetAtTheEndOfAudioSTRN;
  return retError;
}
/* =============================================================================
FUNCTION:
  aviParser::parseINDX

DESCRIPTION:
 Reads and parse index header

INPUT/OUTPUT PARAMETERS:

RETURN VALUE:

SIDE EFFECTS:
  None.
=============================================================================*/
aviErrorType aviParser::parseINDX(avi_uint64 ulloffset)
{
  aviErrorType retError = AVI_SUCCESS;
  avi_int32     cb = 0;
  avi_uint16    wLongsPerEntry = 0; // size of each entry in aIndex array
  char          bIndexSubType = '\0';   // must be 0
  char          bIndexType = '\0';      // one of AVI_INDEX_* codes
  avi_uint32    nEntriesInUse = 0;  //
  avi_uint32    dwChunkId = 0;      // '##dc' OR '##db' OR ...
  avi_uint32    dwReserved[3] = {0};
  unsigned char* byteData = m_ReadBuffer;
  avi_uint16 trackId,cType;
  uint64 tempVal = 0;
  uint32 ulCount = 0;

  m_bSeekDenied = false;

  if(!parserAVICallbakGetData(ulloffset,AVI_READ_BUFFER_SIZE,
                              m_ReadBuffer,AVI_READ_BUFFER_SIZE,
                              m_pUserData, &retError))
  {
    return retError;
  }
  memcpy(&cb,byteData,sizeof(avi_int32));
  ulCount += sizeof(avi_int32);

  memcpy(&wLongsPerEntry,byteData+ulCount,sizeof(avi_uint16));
  ulCount += sizeof(avi_uint16);

  memcpy(&bIndexSubType,byteData+ulCount,sizeof(char));
  ulCount += (sizeof(char));
  memcpy(& bIndexType,byteData+ulCount,sizeof(char));
  ulCount += (sizeof(char));

  //Format of Base index table and Super Index table is identical.
  //Need to re-write some of the following stuff when 'indx' is really
  //an AVI_INDEX_OF_CHUNKS.
  if (AVI_INDEX_OF_CHUNKS == bIndexType)
  {
    return AVI_PARSE_ERROR;
  }

  memcpy(&nEntriesInUse,byteData+ulCount,sizeof(avi_uint32));
  //check nEntriesInUse which should be less than INDX table size
  if(nEntriesInUse * sizeof(avi_indx_super_index_entry) > cb)
  {
    return AVI_CORRUPTED_FILE;
  }
  ulCount += (sizeof(avi_uint32));
  memcpy(&dwChunkId,byteData+ulCount,sizeof(avi_uint32));
  ulCount += (sizeof(avi_uint32));
  memcpy(&dwReserved[0],byteData+ulCount,sizeof(avi_uint32));
  ulCount += (sizeof(avi_uint32));
  memcpy(&dwReserved[1],byteData+ulCount,sizeof(avi_uint32));
  ulCount += (sizeof(avi_uint32));
  memcpy(&dwReserved[2],byteData+ulCount,sizeof(avi_uint32));
  ulCount += (sizeof(avi_uint32));

  ulloffset += ulCount;

  memcpy(&trackId,(unsigned char*)&dwChunkId,sizeof(avi_uint16));
  trackId = ascii_2_short_int(&trackId);
  memcpy(&cType,((unsigned char*)&dwChunkId)+2,sizeof(avi_uint16));

  if(trackId < AVI_MAX_TRACKS)
  {
    m_base_indx_tbl[trackId].isAvailable = true;
    m_base_indx_tbl[trackId].cb = cb;
    m_base_indx_tbl[trackId].wLongsPerEntry = wLongsPerEntry;
    m_base_indx_tbl[trackId].bIndexSubType = bIndexSubType;
    m_base_indx_tbl[trackId].bIndexType = bIndexType;
    m_base_indx_tbl[trackId].nEntriesInUse = nEntriesInUse;
    m_base_indx_tbl[trackId].dwChunkId = dwChunkId;
    m_base_indx_tbl[trackId].dwReserved[0] = dwReserved[0];
    m_base_indx_tbl[trackId].dwReserved[1] = dwReserved[1];
    m_base_indx_tbl[trackId].dwReserved[2] = dwReserved[2];

#ifdef FEATURE_FILESOURCE_AVI_PARSER_DEBUG
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW,
        "m_base_indx_tbl[trackId].isAvailable = %lu",
        (uint32)m_base_indx_tbl[trackId].isAvailable);
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW,
        "m_base_indx_tbl[trackId].cb = %lu",
        (uint32)m_base_indx_tbl[trackId].cb);
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW,
        "m_base_indx_tbl[trackId].wLongsPerEntry = %lu",
        (uint32) m_base_indx_tbl[trackId].wLongsPerEntry);
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW,
        "m_base_indx_tbl[trackId].bIndexType = %lu",
        (uint32)m_base_indx_tbl[trackId].bIndexType);
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW,
        "m_base_indx_tbl[trackId].dwChunkId = %lu",
        (uint32)m_base_indx_tbl[trackId].dwChunkId);
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW,
        "m_base_indx_tbl[trackId].nEntriesInUse = %lu",
        (uint32)m_base_indx_tbl[trackId].nEntriesInUse);
#endif

    if(AVI_INDEX_OF_INDEXES == m_base_indx_tbl[trackId].bIndexType)
    {
      m_base_indx_tbl[trackId].pIndxSuperIndexEntry = (avi_indx_super_index_entry*)
        MM_Malloc(nEntriesInUse * sizeof(avi_indx_super_index_entry));
      m_base_indx_tbl[trackId].pIXIndexChunk = (avi_std_ix_tbl*)
        MM_Malloc(nEntriesInUse * sizeof(avi_std_ix_tbl));

      if((!m_base_indx_tbl[trackId].pIndxSuperIndexEntry) ||
         (!m_base_indx_tbl[trackId].pIXIndexChunk))
      {
        return AVI_OUT_OF_MEMORY;
      }
      memset(m_base_indx_tbl[trackId].pIndxSuperIndexEntry, 0 ,
             (nEntriesInUse* sizeof(avi_indx_super_index_entry)));
      memset(m_base_indx_tbl[trackId].pIXIndexChunk, 0 ,
             (nEntriesInUse* sizeof(avi_std_ix_tbl)));

      uint32 nEntry = 0;
      ulCount = 0;
      while( (nEntry < nEntriesInUse) && (retError == AVI_SUCCESS))
      {
        // We need to reach the buffer again each time as
        // parseIX will modify the global buffer m_ReadBuffer
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW,
                "Trying to read 1024 bytes from the file.");
        if(!parserAVICallbakGetData(ulloffset,sizeof(avi_uint64)+
              sizeof(avi_uint32) + sizeof(avi_uint32),
              m_ReadBuffer, AVI_READ_BUFFER_SIZE,m_pUserData,
              &retError))
        {
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                      "aviParser::parseINDX parserAVICallbakGetData failed");
          return retError;
        }
        else
        {
          byteData = m_ReadBuffer;
          ulCount = 0;
        }
        memcpy(&tempVal,byteData+ulCount,sizeof(avi_uint64));
        m_base_indx_tbl[trackId].pIndxSuperIndexEntry[nEntry].qwOffset = tempVal;
        ulCount += sizeof(avi_uint64);

        memcpy(&tempVal,byteData+ulCount,sizeof(avi_uint32));
        m_base_indx_tbl[trackId].pIndxSuperIndexEntry[nEntry].dwSize = (avi_uint32)tempVal;
        ulCount += sizeof(avi_uint32);

        memcpy(&tempVal,byteData+ulCount,sizeof(avi_uint32));
        m_base_indx_tbl[trackId].pIndxSuperIndexEntry[nEntry].dwDuration = (avi_uint32)tempVal;
        ulCount += sizeof(avi_uint32);

        ulloffset += ulCount;
        retError = parseIX(&m_base_indx_tbl[trackId].pIXIndexChunk[nEntry],
                           m_base_indx_tbl[trackId].pIndxSuperIndexEntry[nEntry].qwOffset);
        nEntry++;
      }
    }
  }
  return retError;
}
/* =============================================================================
FUNCTION:
  aviParser::parseIX

DESCRIPTION:
 Reads and parse indx chunk.

INPUT/OUTPUT PARAMETERS:

RETURN VALUE:

SIDE EFFECTS:
  None.
=============================================================================*/
aviErrorType aviParser::parseIX(avi_std_ix_tbl* pIXTable, avi_uint64 ulloffset)
{
  aviErrorType retError = AVI_SUCCESS;
  avi_int32     cb = 0;
  avi_uint16    wLongsPerEntry = 0; // size of each entry in aIndex array
  char          bIndexSubType = '\0';   // must be 0
  char          bIndexType = '\0';      // one of AVI_INDEX_* codes
  avi_uint32    nEntriesInUse = 0;  //
  avi_uint32    dwChunkId = 0;      // '##dc' OR '##db' OR ...
  avi_uint32    dwReserved = 0;
  avi_uint32    dwReserved_arr[3] = {0};
  unsigned char* byteData = m_ReadBuffer;
  uint32 ulCount = 0;
  avi_uint64 qwBaseOffset;

  fourCC_t fourcc;

  if(!parserAVICallbakGetData(ulloffset,AVI_READ_BUFFER_SIZE,
                              m_ReadBuffer,AVI_READ_BUFFER_SIZE,
                              m_pUserData, &retError))
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                "aviParser::parseIX parserAVICallbakGetData failed");
    return retError;
  }
  memcpy(&fourcc,&byteData[ulCount],sizeof(fourCC_t));
  ulCount += sizeof(fourCC_t);

  memcpy(&cb,&byteData[ulCount],sizeof(avi_int32));
  ulCount += sizeof(avi_int32);

  memcpy(&wLongsPerEntry,&byteData[ulCount],sizeof(avi_uint16));
  ulCount += sizeof(avi_uint16);

  memcpy(&bIndexSubType,&byteData[ulCount],sizeof(char));
  ulCount += (sizeof(char));

  memcpy(& bIndexType,&byteData[ulCount],sizeof(char));
  ulCount += (sizeof(char));

  if(bIndexType == AVI_INDEX_OF_INDEXES)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM, "parseIX : Found AVI_INDEX_OF_INDEXES");
    //nEntriesInUse
    memcpy(&nEntriesInUse,&byteData[ulCount],sizeof(avi_uint32));
    ulCount += (sizeof(avi_uint32));
    //dwChunkId
    memcpy(&dwChunkId,&byteData[ulCount],sizeof(avi_uint32));
    ulCount += (sizeof(avi_uint32));
    //dwReserved_arr[0]
    memcpy(&dwReserved_arr[0],&byteData[ulCount],sizeof(avi_uint32));
    ulCount += (sizeof(avi_uint32));
    //dwReserved_arr[1]
    memcpy(&dwReserved_arr[1],&byteData[ulCount],sizeof(avi_uint32));
    ulCount += (sizeof(avi_uint32));
    //dwReserved_arr[2]
    memcpy(&dwReserved_arr[2],&byteData[ulCount],sizeof(avi_uint32));
    ulCount += (sizeof(avi_uint32));

    ulloffset += ulCount;
  }
  else
  {
    //Either Standard AVI Index of Chunks or Index of Fields.
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,
        "parseIX : Standard AVI Index of Chunks or Index of Fields");
    //nEntriesInUse
    memcpy(&nEntriesInUse,&byteData[ulCount],sizeof(avi_uint32));
    ulCount += (sizeof(avi_uint32));
    //dwChunkId
    memcpy(&dwChunkId,&byteData[ulCount],sizeof(avi_uint32));
    ulCount += (sizeof(avi_uint32));
    //qwBaseOffset
    memcpy(&qwBaseOffset,&byteData[ulCount],sizeof(avi_uint64));
    ulCount += (sizeof(avi_uint64));
    //dwReserved
    memcpy(&dwReserved,&byteData[ulCount],sizeof(avi_uint32));
    ulCount += (sizeof(avi_uint32));

    ulloffset += ulCount;
  }

  avi_uint16 trackId,cType;

  memcpy(&trackId,(unsigned char*)&dwChunkId,sizeof(avi_uint16));
  trackId = ascii_2_short_int(&trackId);
  memcpy(&cType,((unsigned char*)&dwChunkId)+2,sizeof(avi_uint16));

  pIXTable->fcc = fourcc;
  pIXTable->cb = cb;
  pIXTable->wLongsPerEntry = wLongsPerEntry;
  pIXTable->bIndexSubType = bIndexSubType;
  pIXTable->bIndexType = bIndexType;
  pIXTable->nEntriesInUse = nEntriesInUse;
  pIXTable->dwChunkId = dwChunkId;

#ifdef FEATURE_FILESOURCE_AVI_PARSER_DEBUG
  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW, "trackID = %lu",
      (uint32)trackId);
  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW, "pIXTable->fcc = %lu",
      (uint32)pIXTable->fcc);
  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW, "pIXTable->cb = %lu",
      (uint32)pIXTable->cb);
  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW, "pIXTable->wLongsPerEntry = %lu",
      (uint32) pIXTable->wLongsPerEntry);
  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW, "pIXTable->bIndexSubType = %c",
      pIXTable->bIndexSubType);
  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW, "pIXTable->bIndexType = %c",
      pIXTable->bIndexType);
  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW, "pIXTable->nEntriesInUse = %lu",
      (uint32)pIXTable->nEntriesInUse);
  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW, "pIXTable->dwChunkId = %lu",
      (uint32)pIXTable->dwChunkId);
#endif

  if(bIndexType != AVI_INDEX_OF_CHUNKS)
  {
    //No implementation currently
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_FATAL,
                 "parseIX ERROR bIndexType = %c", bIndexType);
    m_bSeekDenied = true;
  }
  else
  {
    //Store Standard Index of Chunks for given stream#.
    pIXTable->qwBaseOffset = qwBaseOffset;
    pIXTable->pIndxChunkIndexEntry = (avi_indx_chunk_index_entry*)
      MM_Malloc(nEntriesInUse * sizeof(avi_indx_chunk_index_entry));

    if(!pIXTable->pIndxChunkIndexEntry)
    {
      return AVI_OUT_OF_MEMORY;
    }

    avi_uint32 nEntry = 0;
    while(nEntry < nEntriesInUse)
    {
      avi_uint32 tempVal =0;
      if (ulCount >= AVI_READ_BUFFER_SIZE -8 )
      {
        // We have already used at least AVI_READ_BUFFER_SIZE-8
        // bytes of data that we had read at the beginning of the
        // function.  And we need at least 8 more bytes.
        // Get More data.
        if(!parserAVICallbakGetData(ulloffset,AVI_READ_BUFFER_SIZE,
           m_ReadBuffer,AVI_READ_BUFFER_SIZE,m_pUserData, &retError))
        {
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                      "aviParser::parseIX parserAVICallbakGetData failed");
          return retError;
        }
        else
        {
          byteData = m_ReadBuffer;
          ulCount= 0;
        }
      }
      memcpy(&tempVal,&byteData[ulCount],sizeof(avi_int32));
      ulloffset += (sizeof(avi_uint32));
      ulCount += (sizeof(avi_uint32));

      /*
      * dwOffset + qwBaseOffset points to actual media data.
      * Need to go back by 8 bytes to point to data chunk header
      */
      pIXTable->pIndxChunkIndexEntry[nEntry].dwOffset = (qwBaseOffset + tempVal) -
                                                        (sizeof(fourCC_t) + sizeof(uint32));

      memcpy(&tempVal,&byteData[ulCount],sizeof(avi_int32));
      ulloffset += (sizeof(avi_uint32));
      ulCount += (sizeof(avi_uint32));


      //Mask 31st bit to get actual size.
      pIXTable->pIndxChunkIndexEntry[nEntry].dwSize = tempVal & 0x7FFFFFFF;

      //bit 31 is set if this is not a key frame.
      pIXTable->pIndxChunkIndexEntry[nEntry].bKeyFrame = (tempVal & 0x80000000)?0:1;

      nEntry++;
    }
  }//if(bIndexType == AVI_INDEX_OF_CHUNKS)
  return retError;
}
/* =============================================================================
FUNCTION:
  aviParser::updateSampleRetrievedOffset

DESCRIPTION:
 Update the offset in IDX1 to match the current audio/video sample retrieved.
 This is used when seeking the stream.

INPUT/OUTPUT PARAMETERS:

RETURN VALUE:

SIDE EFFECTS:
  None.
=============================================================================*/
aviErrorType aviParser::updateSampleRetrievedOffset(CHUNK_t chunktype,avi_uint32 trid)
{
  aviErrorType retError = AVI_SUCCESS;
  unsigned char* byteData = m_ReadBuffer;
  //avi_uint32 nSize = 0;

  avi_uint16 trackId = 0;
  avi_uint16 cType = 0;
  avi_uint32 dwFlags = 0;
  avi_uint64 dwOffset = 0;
  avi_uint32 dwSize = 0;

  if(!m_nCurrentSampleInfoOffsetInIdx1)
  {
     MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                    "updateSampleRetrievedOffset IDX1 does not exist!!");
     return retError;
  }
  while(true)
  {
    if(!parserAVICallbakGetData(m_nCurrentSampleInfoOffsetInIdx1,
      4 * sizeof(avi_uint32),m_ReadBuffer,AVI_READ_BUFFER_SIZE,
      m_pUserData, &retError))
    {
      return retError;
    }
    #ifdef FEATURE_FILESOURCE_AVI_PARSER_DEBUG
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW,
                    "updateSampleRetrievedOffset m_nCurrentSampleInfoOffsetInIdx1 %d",
                     m_nCurrentSampleInfoOffsetInIdx1);
    #endif
    m_nCurrentSampleInfoOffsetInIdx1 += (4 *sizeof(avi_uint32));
    memcpy(&trackId,byteData,sizeof(avi_uint16));
    trackId = ascii_2_short_int(&trackId);
    memcpy(&cType,byteData+2,sizeof(avi_uint16));
    memcpy(&dwFlags,byteData+sizeof(avi_uint32),sizeof(avi_uint32));
    memcpy(&dwOffset,byteData+(2*sizeof(avi_uint32)),sizeof(avi_uint32));
    memcpy(&dwSize,byteData+(3*sizeof(avi_uint32)),sizeof(avi_uint32));

    if(!m_bByteAdjustedForMOVI)
    {
      if( m_nMoviOffset != dwOffset )
      {
        m_nBytesToBeAdjustedForMOVI = (avi_uint32)dwOffset;
        m_bByteAdjustedForMOVI = true;
        #ifdef FEATURE_FILESOURCE_AVI_PARSER_DEBUG
          MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW,
                        "updateSampleRetrievedOffset adjusted m_nBytesToBeAdjustedForMOVI %d",
                        m_nBytesToBeAdjustedForMOVI);
        #endif
      }
    }
    if( (trackId == trid) && (trid < AVI_MAX_VIDEO_TRACKS) )
    {
      if( (chunktype == AVI_CHUNK_VIDEO) && (trid < AVI_MAX_VIDEO_TRACKS) )
      {
        m_nCurrVideoSampleInIdx1++;
        #ifdef FEATURE_FILESOURCE_AVI_PARSER_DEBUG
          MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW,
                        "updateSampleRetrievedOffset m_nCurrVideoSampleInIdx1%d", m_nCurrVideoSampleInIdx1);
        #endif
        if(m_nCurrVideoFrameCount[trid] == m_nCurrVideoSampleInIdx1)
        {
          #ifdef FEATURE_FILESOURCE_AVI_PARSER_DEBUG
            MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW,
                          "updateSampleRetrievedOffset located current VIDEO sample in IDX1 %d", m_nCurrVideoSampleInIdx1);
          #endif
          break;
        }
      }//if(chunktype == AVI_CHUNK_VIDEO)
      if( (chunktype == AVI_CHUNK_AUDIO) && (trid < AVI_MAX_AUDIO_TRACKS) )
      {
        m_nCurrAudioSampleInIdx1++;
        #ifdef FEATURE_FILESOURCE_AVI_PARSER_DEBUG
          MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW,
                        "updateSampleRetrievedOffset m_nCurrAudioSampleInIdx1%d", m_nCurrAudioSampleInIdx1);
        #endif
        if(m_nCurrAudioFrameCount[trid] == m_nCurrAudioSampleInIdx1)
        {
          #ifdef FEATURE_FILESOURCE_AVI_PARSER_DEBUG
            MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW,
                          "updateSampleRetrievedOffset located current AUDIO sample in IDX1 %d", m_nCurrAudioSampleInIdx1);
          #endif
          break;
        }
      }//if(chunktype == AVI_CHUNK_AUDIO)
    }//if(trackId == trid)
  }//while(true)
  return retError;
}
/* =============================================================================
FUNCTION:
  aviParser::SkipToNextValidMediaChunk

DESCRIPTION:
 Scans IDX1 table to locate first valid media chunk

INPUT/OUTPUT PARAMETERS:

RETURN VALUE:

SIDE EFFECTS:
  None.
=============================================================================*/
avi_uint64 aviParser::skipToNextValidMediaChunk(avi_uint64 offset)
{
  unsigned char* byteData = m_ReadBuffer;
  avi_uint16 cType = 0;
  avi_uint16 trackId = 0;
  avi_uint32 dwFlags = 0;
  avi_uint64 dwOffset = 0;
  avi_uint32 dwSize = 0;
  avi_uint64 nOffsetRet;
  aviErrorType retError = AVI_SUCCESS;
  int nItr = 0;

  do
  {
    if(!parserAVICallbakGetData(offset,4 * sizeof(avi_uint32),
      m_ReadBuffer,AVI_READ_BUFFER_SIZE,m_pUserData,&retError))
    {
      return retError;
    }
    memcpy(&trackId,byteData,sizeof(avi_uint16));
    trackId = ascii_2_short_int(&trackId);
    memcpy(&cType,byteData+2,sizeof(avi_uint16));
    memcpy(&dwFlags,byteData+sizeof(avi_uint32),sizeof(avi_uint32));
    memcpy(&dwOffset,byteData+(2*sizeof(avi_uint32)),sizeof(avi_uint32));
    memcpy(&dwSize,byteData+(3*sizeof(avi_uint32)),sizeof(avi_uint32));
    offset += (4 *sizeof(avi_uint32));
    nItr++;
  }while (!( (!memcmp(&cType,"wb",sizeof(avi_uint16))) || (!memcmp(&cType,"dc",sizeof(avi_uint16))) || (!memcmp(&cType,"db",sizeof(avi_uint16))) || (!memcmp(&cType,"dd",sizeof(avi_uint16))) ));
  if(nItr > 1)
  {
    m_nCurrentSampleInfoOffsetInIdx1 = m_nIdx1Offset + ( (nItr-1) * (4 * sizeof(avi_uint32)) );
    m_nAdjustedIdx1Offset = m_nCurrentSampleInfoOffsetInIdx1;
  }
  else
  {
    m_nCurrentSampleInfoOffsetInIdx1 = m_nIdx1Offset;
    m_nAdjustedIdx1Offset = 0;
  }
  if(m_bByteAdjustedForMOVI)
  {
    nOffsetRet = m_nStartOfMovi + dwOffset - m_nBytesToBeAdjustedForMOVI;
  }
  else
  {
    nOffsetRet = dwOffset;
  }

  MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_MEDIUM,
                "SkipToNextValidMediaChunk nOffsetRet %llu m_nCurrOffset %llu", nOffsetRet,m_nCurrOffset);

   MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_MEDIUM,
                "SkipToNextValidMediaChunk m_nCurrentSampleInfoOffsetInIdx1 %llu m_nIdx1Offset %llu",
                m_nCurrentSampleInfoOffsetInIdx1,m_nIdx1Offset);

   MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_MEDIUM,
                "SkipToNextValidMediaChunk m_nAdjustedIdx1Offset %llu m_nIdx1Offset %llu",
                m_nAdjustedIdx1Offset,m_nIdx1Offset);
  return nOffsetRet;
}
/* =============================================================================
FUNCTION:
  aviParser::parseIDX1

DESCRIPTION:
 Reads and parse index header

INPUT/OUTPUT PARAMETERS:

RETURN VALUE:

SIDE EFFECTS:
  None.
=============================================================================*/
aviErrorType aviParser::parseIDX1(avi_uint64* offset)
{
  unsigned char* byteData = m_ReadBuffer;
  avi_uint32 nSize = 0;
  avi_uint16 cType = 0;
  avi_uint16 trackId = 0;
  avi_uint32 dwFlags = 0;
  avi_uint64 dwOffset = 0;
  avi_uint32 dwSize = 0;
  aviErrorType retError = AVI_SUCCESS;
  uint32 ulCount = 0;

  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW, "Entered parseIDX1");
  if(!parserAVICallbakGetData(*offset,AVI_READ_BUFFER_SIZE,
    m_ReadBuffer,AVI_READ_BUFFER_SIZE,m_pUserData,&retError))
  {
    return retError;
  }

  memcpy(&nSize,&byteData[ulCount],sizeof(avi_uint32));
  *offset += sizeof(avi_uint32);
  ulCount += sizeof(avi_uint32);

  bool bOK = false;
  bool bTkID = false;
  bool bFirst = true;
  CHUNK_t Type;

  if(GetTotalNumberOfTracks())
  {
    while(!bOK)
    {
      if (ulCount >= AVI_READ_BUFFER_SIZE)
      {
        // Data in the buffer has already been consumed, get more data
        if(!parserAVICallbakGetData(*offset,AVI_READ_BUFFER_SIZE,
              m_ReadBuffer,AVI_READ_BUFFER_SIZE,m_pUserData,
              &retError))
        {
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                      "aviParser::parseIDX1 parserAVICallbakGetData failed");
          return retError;
        }
        else
        {
          byteData = m_ReadBuffer;
          ulCount= 0;
        }
      }
      memcpy(&trackId,&byteData[ulCount],sizeof(avi_uint16));
      trackId = ascii_2_short_int(&trackId, &bTkID);

      if(GetTrackChunkType(trackId,&Type)!=AVI_SUCCESS)
      {
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH, "parseIDX1::unknown chunk type");
      }

      // ulCount is not being updated here as we will take care of it later
      // depending on the result of data verification which is being done
      // in around 30 lines from here. That verification adds a sort of
      // error tolerance.
      memcpy(&cType,&byteData[ulCount+sizeof(avi_uint16)],sizeof(avi_uint16));
      memcpy(&dwFlags,&byteData[ulCount+sizeof(avi_uint32)],sizeof(avi_uint32));
      memcpy(&dwOffset,&byteData[ulCount+(2*sizeof(avi_uint32))],sizeof(avi_uint32));
      memcpy(&dwSize,&byteData[ulCount+(3*sizeof(avi_uint32))],sizeof(avi_uint32));

      if(bFirst)
      {
        if(( ( (!memcmp(&cType,"dc",sizeof(avi_uint16))) || (!memcmp(&cType,"db",sizeof(avi_uint16))) ||
              (!memcmp(&cType,"wb",sizeof(avi_uint16))) || (!memcmp(&cType,"dd",sizeof(avi_uint16)))
            ) && (((Type == AVI_CHUNK_VIDEO)||(Type ==AVI_CHUNK_AUDIO)) && (bTkID == true))) ||
            (!memcmp(&byteData[ulCount],AVI_JUNK_FOURCC,sizeof(fourCC_t))) ||
            (!memcmp(&byteData[ulCount],AVI_RES_FOURCC,3*sizeof(char))))
        {
            if(!m_bByteAdjustedForMOVI)
            {
              if( m_nStartOfMovi != dwOffset )
              {
                m_nBytesToBeAdjustedForMOVI = (avi_uint32)dwOffset;
                m_bByteAdjustedForMOVI = true;
                #ifdef FEATURE_FILESOURCE_AVI_PARSER_DEBUG
                  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW, "parseIDX1 adjusted m_nBytesToBeAdjustedForMOVI %d",
                              m_nBytesToBeAdjustedForMOVI);
                #endif
              }
            }
        }
        bFirst = false;
      }

      if( ( (!memcmp(&cType,"dc",sizeof(avi_uint16))) || (!memcmp(&cType,"db",sizeof(avi_uint16))) ||
            (!memcmp(&cType,"wb",sizeof(avi_uint16))) || (!memcmp(&cType,"dd",sizeof(avi_uint16)))
          ) && (((Type == AVI_CHUNK_VIDEO)||(Type ==AVI_CHUNK_AUDIO)) && (bTkID == true))
        )
      {
        bOK = true;
      }
      else if( (!memcmp(&byteData[ulCount],AVI_JUNK_FOURCC,sizeof(fourCC_t))) ||
               (!memcmp(&byteData[ulCount],AVI_RES_FOURCC,3*sizeof(char))) )
      {
        *offset += (4 *sizeof(avi_uint32));
        ulCount += (4 *sizeof(avi_uint32));
        nSize -= (4 *sizeof(avi_uint32));
        continue;
      }
      else
      {
        *offset += sizeof(fourCC_t);
        ulCount  += sizeof(fourCC_t);
        nSize -= sizeof(fourCC_t);
      }
    }

    //nOffset is pointing to first entry in IDX1
    m_nIdx1Offset = *offset;
    /*
    * Initialized to first entry in IDX1.
    * Variable gets updated inline with current sample being retrieved.
    * This helps to avoid scannig IDX1 from the begining when seeking.
    */
    m_nCurrentSampleInfoOffsetInIdx1 = m_nIdx1Offset;
    m_nIdx1Size = nSize;

    *offset += nSize;
    ulCount += nSize;

    #ifdef FEATURE_FILESOURCE_AVI_PARSER_DEBUG
      MM_MSG_PRIO4(MM_FILE_OPS, MM_PRIO_LOW,
                      "StartParsing done parsing IDX1 at offset %d size %d m_nCurrentSampleInfoOffsetInIdx1 %d m_nIdx1Size %d",
                      *offset,nSize,m_nCurrentSampleInfoOffsetInIdx1,m_nIdx1Size);
    #endif

#ifndef AVI_PARSER_FAST_START_UP
    (void)cacheIDX1(&m_nIdx1Offset,nSize);
#else
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,"No need to cache IDX1 for FAST_START_UP!!");
#endif
  }

  return AVI_SUCCESS;
}
/* =============================================================================
FUNCTION:
  aviParser::cacheIDX1

DESCRIPTION:
 Reads and cache in entire idx1 table to reduce seek delay.

INPUT/OUTPUT PARAMETERS:

RETURN VALUE:

SIDE EFFECTS:
  None.
=============================================================================*/
#ifndef AVI_PARSER_FAST_START_UP
aviErrorType aviParser::cacheIDX1(avi_uint64* offset,avi_uint32 idx1size)
#else
aviErrorType aviParser::cacheIDX1(avi_uint64*,avi_uint32)
#endif
{
aviErrorType retError = AVI_SUCCESS;
#ifndef AVI_PARSER_FAST_START_UP
  avi_uint16 cType = 0;
  avi_uint16 trackId = 0;
  avi_uint32 dwFlags = 0;
  avi_uint64 dwOffset = 0;
  avi_uint32 dwSize = 0;
  avi_uint32 nTotalEntries = 0;
  avi_uint32 nAudioIndex = 0;
  avi_uint32 nVideoIndex = 0;
  avi_uint64 audioBytesFromStart = 0;
  avi_uint64 tsAudio = 0;
  avi_uint64 prevTSAudio  = 0;
  bool bContinue = true;
  avi_uint64 tmpoffset = *offset;
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW, "Entered cacheIDX1");
  avi_uint32 readIndex = 0;

  int nTSIndexToStore = 0;
  int videotrackidtoCache = -1;
  int audiotrackidtoCache = -1;

  if(m_bDiscardAudioIndex == false)
  {
    //we don't parse idx1 in audio instance.
    //It will be parsed in video instance and will be pushed into aviparser audio instance.
    return retError;
  }

  if(m_hAviSummary.pIdx1Table)
  {
    //Weird,this should never happen.Print F3 for debug and Free the memory.
    MM_Free(m_hAviSummary.pIdx1Table);
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,
                   "Deleting m_hAviSummary.pIdx1Table and allocating...");
  }
  //Allocate the memory for idx1 table
  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW, "trying to allocate memory for idx1 table %d",(sizeof(avi_idx1_table)) );
  m_hAviSummary.pIdx1Table = (avi_idx1_table*)MM_Malloc(sizeof(avi_idx1_table));
  if(!m_hAviSummary.pIdx1Table)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL, "Malloc Failed for pIdx1Table-AVI_OUT_OF_MEMORY");
    bContinue = false;
    retError = AVI_OUT_OF_MEMORY;
  }
  else
  {
    //Record the start offset for the idx1 read above and it's total size.
    memset(m_hAviSummary.pIdx1Table,0,sizeof(avi_idx1_table));
    m_hAviSummary.pIdx1Table->nTotalSize = idx1size;
    m_hAviSummary.pIdx1Table->nStartOffset = *offset;

    //Each idx1 entry has a constant size.Calculate total number of entries that exist in idx1.
    nTotalEntries = idx1size/AVI_IDX1_ENTRY_SIZE;
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW, "nTotalEntries calculated %d",nTotalEntries);
    //Make sure there are non zero entries in idx1
    if(!nTotalEntries)
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW, "nTotalEntries is 0");
      bContinue = false;
    }
    /*
    * Allocate memory for storing audio index table only if clip has audio and parser is told not to
    * discard audio index entries.(AUDIO INSTANCE).
    * To save memory, we don't need to store video entries in audio instance.
    */
    if( (m_hAviSummary.n_audio_tracks)            &&
        (!m_hAviSummary.pIdx1Table->pAudioEntries)&&
        (bContinue))
    {
      /*
      * We always play the first audio track from AVI/DIVX FILE.
      * Storing each audio entry consumes too much memory.
      * We save the audio sample at every AUDIO_SAMPLE_CACHING_INTERVAL to reduce memory requirement.
      * Get the duration of the first audio track
      */
      for(int i =0; i< (int)m_hAviSummary.n_streams; i++)
      {
        if(m_hAviSummary.stream_index[i].type == AVI_CHUNK_AUDIO)
        {
          audiotrackidtoCache = i;
          MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW, "cacheIDX1 caching audio trackid %d",i);
          break;
        }
      }
      if(audiotrackidtoCache == -1)
      {
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL, "Failed to locate audio track id when n_audio_tracks > 0");
        bContinue = false;
        retError = AVI_PARSE_ERROR;
      }
      else
      {
        avi_uint64 duration = GetTrackDuration(audiotrackidtoCache);
        int nAudioEntries = (int)(duration / AUDIO_SAMPLE_CACHING_INTERVAL)+1;
        if(nAudioEntries > 0)
        {
          MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_MEDIUM,
              "trying to allocate memory for storing AudioEntries %d %d",
              nAudioEntries,(sizeof(avi_idx1_entry) * nAudioEntries));
          //make sure there are non zero audio entries to cache.
          m_hAviSummary.pIdx1Table->pAudioEntries =
          (avi_idx1_entry*)MM_Malloc(sizeof(avi_idx1_entry) * nAudioEntries);

          if(!m_hAviSummary.pIdx1Table->pAudioEntries)
          {
            MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH, "Malloc failed for audio IDX1 entries %d",nAudioEntries);
            retError = AVI_OUT_OF_MEMORY;
            bContinue = false;
          }
          else
          {
            memset(m_hAviSummary.pIdx1Table->pAudioEntries, 0, sizeof(avi_idx1_entry) * nAudioEntries);
            m_hAviSummary.pIdx1Table->nAudioEntriesAllocated = nAudioEntries;
            MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW, "ParseIDX1 nAudioEntries %d",nAudioEntries);
          }
        }
      }
    }
    /*
    * Allocate only if clip has video and parser is told not to
    * discard video index entries.(VIDEO INSTANCE)
    */
    if((m_hAviSummary.n_video_tracks)                &&
       (!m_hAviSummary.pIdx1Table->pKeyVideoEntries) &&
       (bContinue)  )
    {
      for(int i =0; i< (int)m_hAviSummary.n_streams; i++)
      {
        if(m_hAviSummary.stream_index[i].type == AVI_CHUNK_VIDEO)
        {
          videotrackidtoCache = i;
          MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW, "cacheIDX1 caching video trackid %d",i);
          break;
        }
      }
      int nVideoEntries = 0;
      if(videotrackidtoCache != -1)
      {
        nVideoEntries = m_hAviSummary.video_info[videotrackidtoCache].strhVideo.dwLength / 8;
      }
      //Make sure there are non zero video entries to cache
      if(nVideoEntries)
      {
        MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_LOW, "trying to allocate memory for storing VideoKeyEntries %d %d,",nVideoEntries,(sizeof(avi_idx1_entry) * nVideoEntries));
        m_hAviSummary.pIdx1Table->pKeyVideoEntries = (avi_idx1_entry*)MM_Malloc(sizeof(avi_idx1_entry) * nVideoEntries);
        if(!m_hAviSummary.pIdx1Table->pKeyVideoEntries)
        {
          MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW, "Malloc failed for video IDX1 entries %d",nVideoEntries);
          retError = AVI_OUT_OF_MEMORY;
          bContinue = false;
        }
        else
        {
          memset(m_hAviSummary.pIdx1Table->pKeyVideoEntries,0,sizeof(avi_idx1_entry) * nVideoEntries);
          m_hAviSummary.pIdx1Table->nVideoEntriesAllocated = nVideoEntries;
          MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW, "ParseIDX1 nVideoEntries %d",nVideoEntries);
        }
      }
    }
    //check if any error has occured above(bContinue will be false) or memory allocation fail.
    if( (!bContinue)                                         ||
        ( (!m_hAviSummary.pIdx1Table->pKeyVideoEntries)&&
        (!m_hAviSummary.pIdx1Table->pAudioEntries) )
      )
    {
      //No need to cache, just print the message
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW, "cacheIDX1 aborting caching!!!");
    }

    while( (tmpoffset < (*offset + idx1size)) && (bContinue) )
    {
      readIndex = 0;
      if(!parserAVICallbakGetData(tmpoffset,(4*sizeof(avi_uint32)),
        m_ReadBuffer,AVI_READ_BUFFER_SIZE,m_pUserData,&retError))
      {
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL, "cacheIDX1 AVI_READ_FAILURE");
        bContinue = false;
      }
      //read in idx1 entry
      memcpy(&trackId,m_ReadBuffer+readIndex,sizeof(avi_uint16));
      trackId = ascii_2_short_int(&trackId);
      readIndex+=sizeof(avi_uint16);

      memcpy(&cType,m_ReadBuffer+readIndex,sizeof(avi_uint16));
      readIndex+=sizeof(avi_uint16);

      memcpy(&dwFlags,m_ReadBuffer+readIndex,sizeof(avi_uint32));
      readIndex+=sizeof(avi_uint32);

      memcpy(&dwOffset,m_ReadBuffer+readIndex,sizeof(avi_uint32));
      readIndex+=sizeof(avi_uint32);

      memcpy(&dwSize,m_ReadBuffer+readIndex,sizeof(avi_uint32));
      readIndex+=sizeof(avi_uint32);
      tmpoffset += (4*sizeof(avi_uint32));

      //If idx1 entry denotes video chunk, check if it's a key frame
      if(  ( (!memcmp(&cType,"dc",sizeof(avi_uint16))) ||(!memcmp(&cType,"db",sizeof(avi_uint16))) ) &&
           (m_hAviSummary.pIdx1Table->pKeyVideoEntries ) && (bContinue) && (videotrackidtoCache == trackId) )
      {
        if(dwFlags & AVI_KEY_FRAME_MASK)
        {
          //video chunk contains key-frame.Make sure there is room for storage,otherwise, do the re-alloc.
          if( nVideoIndex >= m_hAviSummary.pIdx1Table->nVideoEntriesAllocated )
          {
            MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW, "cacheIDX1 doing realloc for video curr #entries ",
                            m_hAviSummary.pIdx1Table->nVideoEntriesAllocated);
            avi_idx1_entry* pTemp = NULL;
            pTemp = (avi_idx1_entry*)MM_Realloc(m_hAviSummary.pIdx1Table->pKeyVideoEntries,
                                             (sizeof(avi_idx1_entry) * (m_hAviSummary.pIdx1Table->nVideoEntriesAllocated * 2)) );
            if(!pTemp)
            {
              MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW,
                              "cacheIDX1 realloc failed for video curr #entries ",
                               m_hAviSummary.pIdx1Table->nVideoEntriesAllocated);
              MM_Free(m_hAviSummary.pIdx1Table->pKeyVideoEntries);
              m_hAviSummary.pIdx1Table->pKeyVideoEntries = NULL;
              m_hAviSummary.pIdx1Table->nVideoEntriesAllocated = 0;
              retError = AVI_OUT_OF_MEMORY;
              bContinue = false;
            }
            else
            {
              m_hAviSummary.pIdx1Table->pKeyVideoEntries = pTemp;
              m_hAviSummary.pIdx1Table->nVideoEntriesAllocated *= 2;
            }
          }
          //Check for pointer as re-alloc can fail.
          if(m_hAviSummary.pIdx1Table->pKeyVideoEntries)
          {
            m_hAviSummary.pIdx1Table->pKeyVideoEntries[nVideoIndex].chunkType = AVI_CHUNK_VIDEO;
            m_hAviSummary.pIdx1Table->nKeyVideoyEntriesInIDX1++;
            m_hAviSummary.pIdx1Table->pKeyVideoEntries[nVideoIndex].dwVidFrameCount =
                                       m_hAviSummary.pIdx1Table->nVideoEntriesInIDX1;
            m_hAviSummary.pIdx1Table->pKeyVideoEntries[nVideoIndex].dwFlags = dwFlags;

            if(m_bByteAdjustedForMOVI)
            {
              m_hAviSummary.pIdx1Table->pKeyVideoEntries[nVideoIndex].dwOffset =
              (avi_uint64)(m_nStartOfMovi + dwOffset - m_nBytesToBeAdjustedForMOVI);
            }
            else
            {
              m_hAviSummary.pIdx1Table->pKeyVideoEntries[nVideoIndex].dwOffset = dwOffset;
            }
            m_hAviSummary.pIdx1Table->pKeyVideoEntries[nVideoIndex].dwSize = dwSize;
            m_hAviSummary.pIdx1Table->pKeyVideoEntries[nVideoIndex].trackId = trackId;
            m_hAviSummary.pIdx1Table->pKeyVideoEntries[nVideoIndex].dwOffsetInIDX1 = (tmpoffset - (4*sizeof(avi_uint32)) );

            avi_video_info vinfo;
            if( (GetVideoInfo(trackId,&vinfo)==AVI_SUCCESS) && (vinfo.strhVideo.dwRate) )
            {
              //dwScale/dwRate gives framerate in fps.
              m_hAviSummary.pIdx1Table->pKeyVideoEntries[nVideoIndex].nTimeStamp =
                ((uint64)vinfo.strhVideo.dwScale * MILLISEC_TIMESCALE_UNIT *
                 m_hAviSummary.pIdx1Table->pKeyVideoEntries[nVideoIndex].dwVidFrameCount)/
                 vinfo.strhVideo.dwRate;
            }
            nVideoIndex++;
          }
        }
        m_hAviSummary.pIdx1Table->nVideoEntriesInIDX1++;
      }//if video entry
      else if( (!memcmp(&cType,"wb",sizeof(avi_uint16))) &&
               (m_hAviSummary.pIdx1Table->pAudioEntries) &&
               (bContinue)                               &&
               (trackId == audiotrackidtoCache))
      {
        //make sure there is a room to store audio entry.If not do the re-alloc.
        if( nAudioIndex >= m_hAviSummary.pIdx1Table->nAudioEntriesAllocated )
        {
          #ifdef FEATURE_FILESOURCE_AVI_PARSER_DEBUG
            MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW, "parseIDX1 doing realloc for audio curr #entries ",
                            m_hAviSummary.pIdx1Table->nAudioEntriesAllocated);
          #endif
          avi_idx1_entry* pTemp = NULL;
          pTemp = (avi_idx1_entry*)MM_Realloc(m_hAviSummary.pIdx1Table->pAudioEntries,
                                          (sizeof(avi_idx1_entry)*(m_hAviSummary.pIdx1Table->nAudioEntriesAllocated * 2)) );
          if(!pTemp )
          {
            MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW, "parseIDX1 realloc failed for audio curr #entries ",
                            m_hAviSummary.pIdx1Table->nAudioEntriesAllocated);
            MM_Free(m_hAviSummary.pIdx1Table->pAudioEntries);
            m_hAviSummary.pIdx1Table->pAudioEntries = NULL;
            m_hAviSummary.pIdx1Table->nAudioEntriesAllocated = 0;
            bContinue = false;
            retError = AVI_OUT_OF_MEMORY;
          }
          else
          {
            m_hAviSummary.pIdx1Table->pAudioEntries = pTemp;
            m_hAviSummary.pIdx1Table->nAudioEntriesAllocated *= 2;
          }
        }
        avi_audiotrack_summary_info ainfo;
        //check for pointer as re-alloc can fail
        if(m_hAviSummary.pIdx1Table->pAudioEntries)
        {
          //This should always succeed, if not there is a problem in initial parsing.
          if(GetAudioTrackSummaryInfo(trackId,&ainfo)==AVI_SUCCESS)
          {
            if( (!ainfo.isVbr)&&(ainfo.audioBytesPerSec) )
            {
              tsAudio = (avi_uint32)((float)audioBytesFromStart /
                (float)ainfo.audioBytesPerSec * 1000.0f);

              prevTSAudio = tsAudio;

              #ifdef WALK_INDEX_TABLES
                MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_LOW,
                               "parseIDX1 AUDIO TRACK IS CBR TS %d Chunk# %d ",
                               tsAudio,m_hAviSummary.pIdx1Table->nAudioEntriesInIDX1);
              #endif
            }
            else if( ainfo.isVbr && ainfo.audioFrequency )
            {
              avi_audio_info audinfo;
              if(GetAudioInfo(trackId,&audinfo)==AVI_SUCCESS)
              {
                if(audinfo.strhAudio.dwSampleSize > 0)
                {
                  prevTSAudio = tsAudio;
                  double val1 = ceil( ( (double)dwSize / ainfo.nBlockAlign) );
                  double val2 =  ((double)audinfo.strhAudio.dwScale/audinfo.strhAudio.dwRate)* 1000.f;
                  avi_uint64 duration  =  (uint64)(val1 * val2);
                  tsAudio +=  duration;
                }
                else
                {
                  tsAudio = (avi_uint64)(( ( (float)audinfo.strhAudio.dwScale/
                                             (float)audinfo.strhAudio.dwRate) * 1000.f) * (float)m_hAviSummary.pIdx1Table->nAudioEntriesInIDX1);                  \
                  prevTSAudio = tsAudio;
                }
              }
            }
            if( (prevTSAudio / AUDIO_SAMPLE_CACHING_INTERVAL) >= (uint64)nTSIndexToStore)
            {
              m_hAviSummary.pIdx1Table->pAudioEntries[nAudioIndex].nTimeStamp =
                prevTSAudio;
              m_hAviSummary.pIdx1Table->pAudioEntries[nAudioIndex].chunkType =
                AVI_CHUNK_AUDIO;
              m_hAviSummary.pIdx1Table->pAudioEntries[nAudioIndex].dwFlags =
                dwFlags;
              if(m_bByteAdjustedForMOVI)
              {
                m_hAviSummary.pIdx1Table->pAudioEntries[nAudioIndex].dwOffset =
                    (avi_uint64)(m_nStartOfMovi + dwOffset - m_nBytesToBeAdjustedForMOVI);
              }
              else
              {
                m_hAviSummary.pIdx1Table->pAudioEntries[nAudioIndex].dwOffset = dwOffset;
              }
              m_hAviSummary.pIdx1Table->pAudioEntries[nAudioIndex].dwOffsetInIDX1 =
                (tmpoffset - (4 * sizeof(avi_uint32)) );
              m_hAviSummary.pIdx1Table->pAudioEntries[nAudioIndex].dwSize = dwSize;
              m_hAviSummary.pIdx1Table->pAudioEntries[nAudioIndex].trackId = trackId;
              m_hAviSummary.pIdx1Table->pAudioEntries[nAudioIndex].dwAudFrameCount =
                              m_hAviSummary.pIdx1Table->nAudioEntriesInIDX1;
              m_hAviSummary.pIdx1Table->pAudioEntries[nAudioIndex].nTotalSizeInBytes =
                audioBytesFromStart;

              nTSIndexToStore = ((int)prevTSAudio / AUDIO_SAMPLE_CACHING_INTERVAL);
              nTSIndexToStore++;
              nAudioIndex++;
              m_hAviSummary.pIdx1Table->nCurrParsedAudioEntriesInIDX1 = nAudioIndex;

              #ifdef WALK_INDEX_TABLES
                MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_MEDIUM,"cacheIDX1, AUDIO TS indexed %d nAudioIndex %d",prevTSAudio,nAudioIndex);
              #endif
            }//if( (tsAudio / AUDIO_SAMPLE_CACHING_INTERVAL) == nTSIndexToStore)
          }//if(GetAudioTrackSummaryInfo(trackId,&ainfo)==AVI_SUCCESS)
        }//if(m_hAviSummary.pIdx1Table->pAudioEntries)
        audioBytesFromStart+=dwSize;
        m_hAviSummary.pIdx1Table->nAudioEntriesInIDX1++;
      }//else if( (!memcmp(&cType,"wb",sizeof(avi_uint16))) && (m_hAviSummary.pIdx1Table->pAudioEntries) && (bContinue) )

      m_hAviSummary.pIdx1Table->nTotalEntriesInIDX1++;
      #ifdef WALK_INDEX_TABLES
        MM_MSG_PRIO6(MM_FILE_OPS, MM_PRIO_MEDIUM, "parseIDX1 nAudioEntriesInIDX1 %d nAudioEntriesLoaded %d nVideoEntriesInIDX1 %d #KeyFrames %d nTotalEntriesInIDX1 %d nSize %d",
                        m_hAviSummary.pIdx1Table->nAudioEntriesInIDX1,
                        m_hAviSummary.pIdx1Table->nCurrParsedAudioEntriesInIDX1,
                        m_hAviSummary.pIdx1Table->nVideoEntriesInIDX1,
                        m_hAviSummary.pIdx1Table->nKeyVideoyEntriesInIDX1,
                        m_hAviSummary.pIdx1Table->nTotalEntriesInIDX1,nSize);
      #endif
    }//while( (tmpoffset < (*offset + idx1size)) && (bContinue) )
    //store the total num of audio bytes.
    m_hAviSummary.pIdx1Table->nTotalAudioBytes = audioBytesFromStart;
  }//end of else of if(!m_hAviSummary.pIdx1Table)
  if(!bContinue)
  {
    //error occured while caching the entries.
    //free up the memory, if any,allocated above.
    if(m_hAviSummary.pIdx1Table)
    {
      if(m_hAviSummary.pIdx1Table->pAudioEntries)
      {
        MM_Free(m_hAviSummary.pIdx1Table->pAudioEntries);
        m_hAviSummary.pIdx1Table->pAudioEntries = NULL;
      }
      if(m_hAviSummary.pIdx1Table->pKeyVideoEntries)
      {
        MM_Free(m_hAviSummary.pIdx1Table->pKeyVideoEntries);
        m_hAviSummary.pIdx1Table->pKeyVideoEntries = NULL;
      }
      MM_Free(m_hAviSummary.pIdx1Table);
      m_hAviSummary.pIdx1Table = NULL;
    }
  }
  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW, "leaving cacheIDX1 retError %d",retError);
#endif
  return retError;
}
/* =============================================================================
FUNCTION:
  aviParser::parseODML

DESCRIPTION:
 Reads and parse ODML header

INPUT/OUTPUT PARAMETERS:

RETURN VALUE:

SIDE EFFECTS:
  None.
=============================================================================*/
aviErrorType aviParser::parseODML(avi_uint64 offset)
{
  int nFrames = 0;
  int nSize=0;
  unsigned char* byteData = m_ReadBuffer;
  aviErrorType retError = AVI_SUCCESS;
  #ifdef FEATURE_FILESOURCE_AVI_PARSER_DEBUG
  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW,
                "parseODML Offset to start %d",offset);
  #endif
  if(!parserAVICallbakGetData(offset,(2*sizeof(avi_uint32)),
    m_ReadBuffer,AVI_READ_BUFFER_SIZE,m_pUserData,&retError))
  {
    return retError;
  }
  offset+=(2*sizeof(avi_uint32));
  memcpy(&nSize,byteData,sizeof(avi_uint32));
  memcpy(&nFrames,byteData+sizeof(avi_uint32),sizeof(avi_uint32));
  return AVI_SUCCESS;
}
/* =============================================================================
FUNCTION:
  aviParser::parseINFO

DESCRIPTION:
 Reads and parse INFO header

INPUT/OUTPUT PARAMETERS:

RETURN VALUE:

SIDE EFFECTS:
  None.
=============================================================================*/
aviErrorType aviParser::parseINFO(avi_uint64 offset,int size)
{
  unsigned char* byteData = m_ReadBuffer;
  avi_uint32 nSize = 0;
  aviErrorType retError = AVI_SUCCESS;
  fourCC_t fourcc;

  #ifdef FEATURE_FILESOURCE_AVI_PARSER_DEBUG
  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW,
                "parseINFO Offset to start %d",offset);
  #endif

  while(size > 0)
  {
    //Read INFO Chunk FourCC and Size
    if(!parserAVICallbakGetData(offset,
      (sizeof(fourCC_t)+ sizeof(avi_uint32)),m_ReadBuffer,
      AVI_READ_BUFFER_SIZE,m_pUserData,&retError))
    {
      return retError;
    }
    offset+= ((sizeof(fourCC_t)) +(sizeof(avi_uint32)));

    memcpy(&fourcc,byteData,sizeof(fourCC_t));
    memcpy(&nSize,byteData+sizeof(fourCC_t),sizeof(avi_uint32));

    //validate chunk size
    if (ValidateChunkSize(nSize,m_nFileSize))
    {
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH, "parseINFO::DATA CHUNK SIZE CORRUPTED %llu",nSize);
      return AVI_CORRUPTED_FILE;
    }

    (void)updateInfoChunkInfo(fourcc,nSize,offset);

    if(nSize %2)
    {
      //Must align to 16-bit boundary
      nSize++;
    }
    offset+= nSize;

    size -= (sizeof(uint32)+sizeof(fourCC_t)+nSize);
  }
  return AVI_SUCCESS;
}
/* =============================================================================
FUNCTION:
  aviParser::getDataFromInfoChunk

DESCRIPTION:
 Retrieves required meta data from INFO chunk

INPUT/OUTPUT PARAMETERS:

RETURN VALUE:

SIDE EFFECTS:
  None.
=============================================================================*/
PARSER_ERRORTYPE aviParser::getDataFromInfoChunk(const char* fourcc, wchar_t* ptr, avi_uint16* size)
{
  PARSER_ERRORTYPE retError = PARSER_ErrorDefault;

  if((size != NULL) && (fourcc != NULL) )
  {
    retError = PARSER_ErrorNone;
    if(!memcmp(fourcc,INFO_NAME,sizeof(fourCC_t)))
    {
      *size = m_AviClipMetaInfo.Name.nSize;
      if((ptr != NULL) && (*size>0))
      {
        memcpy(ptr,m_AviClipMetaInfo.Name.Ptr,*size);
      }
    }
    else if(!memcmp(fourcc,INFO_ARTIST,sizeof(fourCC_t)))
    {
      *size = m_AviClipMetaInfo.Artist.nSize;
      if((ptr != NULL)&& (*size>0))
      {
        memcpy(ptr,m_AviClipMetaInfo.Artist.Ptr,*size);
      }
    }
    else if(!memcmp(fourcc,INFO_COMMENTS,sizeof(fourCC_t)))
    {
      *size = m_AviClipMetaInfo.Comments.nSize;
      if((ptr != NULL)&& (*size>0))
      {
        memcpy(ptr,m_AviClipMetaInfo.Comments.Ptr,*size);
      }
    }
    else if(!memcmp(fourcc,INFO_COPYRIGHT,sizeof(fourCC_t)))
    {
      *size = m_AviClipMetaInfo.Copyright.nSize;
      if((ptr != NULL)&& (*size>0))
      {
        memcpy(ptr,m_AviClipMetaInfo.Copyright.Ptr,*size);
      }
    }
    else if(!memcmp(fourcc,INFO_CREATION_DATE,sizeof(fourCC_t)))
    {
      *size = m_AviClipMetaInfo.CreateDate.nSize;
      if((ptr != NULL)&& (*size>0))
      {
        memcpy(ptr,m_AviClipMetaInfo.CreateDate.Ptr,*size);
      }
    }
    else if(!memcmp(fourcc,INFO_SOFTWARE,sizeof(fourCC_t)))
    {
      *size = m_AviClipMetaInfo.Software.nSize;
      if((ptr != NULL)&& (*size>0))
      {
        memcpy(ptr,m_AviClipMetaInfo.Software.Ptr,*size);
      }
    }
    else
      retError = PARSER_ErrorNotImplemented;
  }
  else
  {
    retError = PARSER_ErrorInvalidParam;
  }
  return retError;
}
/* =============================================================================
FUNCTION:
  aviParser::updateInfoChunkInfo

DESCRIPTION:
 Stores the info for chunk identified by fourcc.

INPUT/OUTPUT PARAMETERS:

RETURN VALUE:

SIDE EFFECTS:
  None.
=============================================================================*/
aviErrorType aviParser::updateInfoChunkInfo(fourCC_t fourcc,avi_uint32 nsize,avi_uint64 offset)
{
  aviErrorType retError = AVI_SUCCESS;
  #ifdef FEATURE_FILESOURCE_AVI_PARSER_DEBUG
  MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_LOW,
                "updateInfoChunkInfo fourcc %d nsize %d",fourcc,nsize);
  #endif

  if(nsize > 0)
  {
    avi_info_chunk* pChunkInfo = getInfoChunkHandle(fourcc);
    if(!pChunkInfo)
    {
      //Error occured or parser does not support given fourcc.
      //Continue but just flag the message.
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
                     "updateInfoChunkInfo getInfoChunkHandle return NULL");
    }
    else
    {
      if(pChunkInfo->Ptr)
      {
        MM_Free(pChunkInfo->Ptr);
      }
      pChunkInfo->Ptr   = (char*)MM_Malloc(nsize);
      if(!pChunkInfo->Ptr)
      {
        return AVI_OUT_OF_MEMORY;
      }
      //Read 'nsize' data from file.
      if(!parserAVICallbakGetData(offset,nsize,
        (unsigned char*)pChunkInfo->Ptr,nsize,m_pUserData,&retError))
      {
        return retError;
      }
      pChunkInfo->nSize = nsize;
    }
  }
  return AVI_SUCCESS;
}
/* =============================================================================
FUNCTION:
  aviParser::getInfoChunkHandle

DESCRIPTION:
 Returns the chunk info handle for given chunk id.

INPUT/OUTPUT PARAMETERS:

RETURN VALUE:

SIDE EFFECTS:
  None.
=============================================================================*/
avi_info_chunk* aviParser::getInfoChunkHandle(fourCC_t fourcc)
{
   if(!memcmp(&fourcc,INFO_ARCHIVAL_LOCATION,sizeof(fourCC_t))){
      return &m_AviClipMetaInfo.ArchLocn;
    }
    else if(!memcmp(&fourcc,INFO_ARTIST,sizeof(fourCC_t))){
      return &m_AviClipMetaInfo.Artist;
    }
    else if(!memcmp(&fourcc,INFO_COMMISIONED,sizeof(fourCC_t))){
      return &m_AviClipMetaInfo.Commissioned;
    }
    else if(!memcmp(&fourcc,INFO_COMMENTS,sizeof(fourCC_t))){
      return &m_AviClipMetaInfo.Comments;
    }
    else if(!memcmp(&fourcc,INFO_COPYRIGHT,sizeof(fourCC_t))){
      return &m_AviClipMetaInfo.Copyright;
    }
    else if(!memcmp(&fourcc,INFO_CREATION_DATE,sizeof(fourCC_t))){
      return &m_AviClipMetaInfo.CreateDate;
    }
    else if(!memcmp(&fourcc,INFO_GENRE,sizeof(fourCC_t))){
      return &m_AviClipMetaInfo.Genre;
    }
    else if(!memcmp(&fourcc,INFO_KEYWORD,sizeof(fourCC_t))){
      return &m_AviClipMetaInfo.Keyword;
    }
    else if(!memcmp(&fourcc,INFO_NAME,sizeof(fourCC_t))){
      return &m_AviClipMetaInfo.Name;
    }
    else if(!memcmp(&fourcc,INFO_PRODUCT,sizeof(fourCC_t))){
      return &m_AviClipMetaInfo.Product;
    }
    else if(!memcmp(&fourcc,INFO_SUBJECT,sizeof(fourCC_t))){
      return &m_AviClipMetaInfo.Subject;
    }
    else if(!memcmp(&fourcc,INFO_SOURCE,sizeof(fourCC_t))){
      return &m_AviClipMetaInfo.Source;
    }
    else if(!memcmp(&fourcc,INFO_SOFTWARE,sizeof(fourCC_t))){
      return &m_AviClipMetaInfo.Software;
    }
    return NULL;
 }
/* =============================================================================
FUNCTION:
  aviParser::StartParsing

DESCRIPTION:
 Reads and parse clip meta data such as AVIH/STRL/STRF/STRN/INDX1 etc.

INPUT/OUTPUT PARAMETERS:

RETURN VALUE:

SIDE EFFECTS:
  None.
=============================================================================*/
aviErrorType aviParser::StartParsing(void)
{
  unsigned char* byteData = m_ReadBuffer;
  bool bFoundStartRiff = false;
  aviErrorType retError = AVI_CORRUPTED_FILE;
  fourCC_t nCurrFourCC = 0;
  int nFoundRiff = 0;
  int badChunkCounter = 0;
  if(!m_pUserData)
  {
     MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                  "StartParsing AVI_INVALID_USER_DATA");
    return AVI_INVALID_USER_DATA;
  }
  avi_uint64 nOffset = 0;
  avi_uint64 inMoviOffset = 0;

  #ifdef FEATURE_FILESOURCE_AVI_PARSER_DEBUG
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW, "StartParsing");
  #endif

  if(!bFoundStartRiff)
  {
    if(parserAVICallbakGetData(nOffset,12,m_ReadBuffer,
      AVI_READ_BUFFER_SIZE,m_pUserData,&retError))
    {
      nOffset+=12;
      if(!memcmp(byteData,AVI_START_BYTES,sizeof(fourCC_t)))
      {
        if(!memcmp((byteData+8),AVI_SIGNATURE_BYTES,sizeof(fourCC_t)))
        {
          bFoundStartRiff = true;
          #ifdef FEATURE_FILESOURCE_AVI_PARSER_DEBUG
            MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW, "StartParsing located RIFF AVI");
          #endif
        }
      }
    }
    if(!bFoundStartRiff)
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL, "StartParsing AVI_CORRUPTED_FILE");
      return AVI_CORRUPTED_FILE;
    }
  }
  if(bFoundStartRiff)
  {
    m_CurrentParserState = AVI_PARSER_INIT;
    retError = AVI_SUCCESS;
    while( (nOffset + sizeof(fourCC_t) < m_nFileSize) && (retError == AVI_SUCCESS) )
    {
      if(!parserAVICallbakGetData(nOffset,sizeof(fourCC_t),
        m_ReadBuffer,AVI_READ_BUFFER_SIZE,m_pUserData,&retError))
      {
        break;
      }
      nOffset+=sizeof(fourCC_t);

      if(!memcmp(byteData,AVI_LIST_FOURCC,sizeof(fourCC_t)))
      {
        //Get the list size
        if(!parserAVICallbakGetData(nOffset,sizeof(avi_uint32),
          m_ReadBuffer,AVI_READ_BUFFER_SIZE,m_pUserData,&retError))
        {
          break;
        }
        nOffset+=sizeof(avi_uint32);
        avi_uint32 nListSize = 0;
        memcpy(&nListSize,byteData,sizeof(avi_uint32));
        if(nListSize && badChunkCounter)
        {
          //Reset counter to 0 when we find chunk with valid size
          badChunkCounter = 0;
        }

        //get the next fourCC code
        if(!parserAVICallbakGetData(nOffset,sizeof(fourCC_t),
          m_ReadBuffer,AVI_READ_BUFFER_SIZE,m_pUserData,&retError))
        {
          break;
        }
        nOffset+=sizeof(fourCC_t);
        memcpy(&nCurrFourCC,byteData,sizeof(fourCC_t));

        /*
        * PARSING is STRICT,meaning when an error is reported
        * while parsing any of the following headers,playback will error out.
        * Not updating retError with return value will make it non STRICT,if needed.
        * Need to find out which error can be ignored while parsing following AVI headers.
        */

        if(!memcmp(byteData,AVI_HDRL_FOURCC,sizeof(fourCC_t)))
        {
          retError = parseHDRL(&nOffset, nListSize);
        }
        else if(!memcmp(byteData,AVI_STRL_FOURCC,sizeof(fourCC_t)))
        {
          retError = parseSTRL(&nOffset,nListSize);
        }
        else if(!memcmp(byteData,AVI_ODML_FOURCC,sizeof(fourCC_t)))
        {
          retError = parseODML(nOffset);
          nOffset+=nListSize-sizeof(fourCC_t);
        }
        else if(!memcmp(byteData,AVI_MOVI_FOURCC,sizeof(fourCC_t)))
        {
          avi_uint64 nDataOffset = (avi_uint64)AVICheckAvailableData(m_pUserData);
          if((m_bHttpStreaming)&& (nDataOffset < m_nFileSize) && (nDataOffset))
          {
            m_nMoviOffset = m_nCurrOffset = m_nSampleInfoOffset = nOffset;
            m_CurrentParserState = AVI_PARSER_READY;
            break;
          }
          inMoviOffset = nOffset;
          m_nStartOfMovi = inMoviOffset;
          retError = parseMOVI(nOffset);
          nOffset = nOffset + (nListSize - (nOffset - inMoviOffset)) - sizeof(fourCC_t);
        }
        else if(!memcmp(byteData,AVI_INFO_FOURCC,sizeof(fourCC_t)))
        {
          retError = parseINFO(nOffset,(nListSize-sizeof(fourCC_t)));
          nOffset+=nListSize-sizeof(fourCC_t);
        }
        else
        {
          //Any un-known LIST chunk should be skipped.
          fourCC_t unknownFourCC;
          memcpy(&unknownFourCC,byteData,sizeof(fourCC_t));
          MM_MSG_PRIO1(MM_FILE_OPS,
                        MM_PRIO_LOW,
                        "StartParsing encountered unknown LIST FOURCC %x",
                        unknownFourCC);
          //Skip size to go to next FOURCC.
          //nOffset+=nListSize;
          nOffset+=nListSize-sizeof(fourCC_t);
        }
      }//if(!memcmp(byteData,AVI_LIST_FOURCC,sizeof(fourCC_t)))
      else if(!memcmp(byteData,AVI_JUNK_FOURCC,sizeof(fourCC_t)))
      {
        retError = parseJUNK(&nOffset);
        continue;
      }
      else if(!memcmp(byteData,AVI_IDX1_FOURCC,sizeof(fourCC_t)))
      {
        m_bSeekDenied = false;
        //IDX1 is the last RIFF as per AVI std.
        if( (retError = parseIDX1(&nOffset))!= AVI_SUCCESS )
        {
          /*
          * IDX1 is used for repositioning and calculating audio/video
          * timestamp once seek is done.
          * So, abort the playback if IDX1 parsing fails.
          */
          MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_FATAL,
                       "StartParsing parseIDX1 failed..retError %d",retError);
          return retError;
        }

        /*
        * As per AVI specification, IDX1 is the last chunk in the file.
        * So, we can safely break from here. However, there could be few clips
        * that do not follow the specification,so continue parsing without breaking.
        */
      }
      else if(!memcmp(byteData,AVI_INDX_FOURCC,sizeof(fourCC_t)))
      {
        //Clip has 'indx' chunk.
        if( (retError = parseINDX(nOffset) ) != AVI_SUCCESS )
        {
          /*
          * INDX is used for repositioning and calculating audio/video
          * timestamp once seek is done and during playback.
          * So, abort the playback if IDX1 parsing fails.
          */
          MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_FATAL,
                       "StartParsing parseINDX failed..retError %d",retError);
          return retError;
        }
        /*
        * Done parsing INDX;Read in size now and go over it.
        */
        if(!parserAVICallbakGetData(nOffset,sizeof(avi_uint32),
          m_ReadBuffer,AVI_READ_BUFFER_SIZE,m_pUserData,&retError))
        {
          break;
        }
        nOffset+=sizeof(avi_uint32);
        int size = 0;
        memcpy(&size,byteData,sizeof(avi_uint32));
        nOffset+=size;
        #ifdef FEATURE_FILESOURCE_AVI_PARSER_DEBUG
          MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_LOW,
                        "StartParsing done parsing INDX at offset %d size %d",
                        nOffset - sizeof(avi_uint32),size);
        #endif
      }
      else if(!memcmp(byteData,AVI_START_BYTES,sizeof(fourCC_t)))
      {
        m_bisAVIXpresent = true;
        if(!m_nNumOfRiff)
        {
          retError = GetNumOfRiff(nOffset, &m_nNumOfRiff);
          if( (retError == AVI_SUCCESS) && (m_nNumOfRiff) )
          {
            m_pMultipleRiff = (avi_riff_info*)MM_Malloc((sizeof(avi_riff_info))*(m_nNumOfRiff));
          }
          else
          {
            MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,
                             "StartParsing: m_nNumOfRiff is ZERO, multiple valid RIFFs not found");
          }
        }
        if((m_pMultipleRiff) && (nFoundRiff<= m_nNumOfRiff))
        {
          if(!parserAVICallbakGetData(nOffset,sizeof(avi_uint32),
            m_ReadBuffer,AVI_READ_BUFFER_SIZE,m_pUserData,&retError))
          {
            return retError;
          }
          nOffset+=sizeof(avi_uint32);
          m_pMultipleRiff[nFoundRiff].startOffset = nOffset;
          memcpy(&m_pMultipleRiff[nFoundRiff].size,byteData,sizeof(avi_uint32));
          //Allign to 16-bit boundary
          m_pMultipleRiff[nFoundRiff].size = m_pMultipleRiff[nFoundRiff].size + ((m_pMultipleRiff[nFoundRiff].size) %2);
          nOffset+=m_pMultipleRiff[nFoundRiff].size;
          nFoundRiff++;
        }
      }
      else
      {
        //Any unknown FOURCC should be skipped...
        fourCC_t unknownFourCC;
        memcpy(&unknownFourCC,byteData,sizeof(fourCC_t));

        if( (nOffset+sizeof(avi_uint32)) >= m_nFileSize)
        {
          //Some times, a file might be missing chunk size after a fourCC.
          //If this is the last chunk, break out as no need flag an error while
          //parsing last unknown chunk.
          #ifdef FEATURE_FILESOURCE_AVI_PARSER_DEBUG
            MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_MEDIUM,
                          "StartParsing Breaking from within the loop ..nOffset %d m_nFileSize %d",nOffset,m_nFileSize);
          #endif
          break;
        }

        //read in size to skip to next FOURCC
        if(!parserAVICallbakGetData(nOffset,sizeof(avi_uint32),
          m_ReadBuffer,AVI_READ_BUFFER_SIZE,m_pUserData,&retError))
        {
          return retError;
        }
        nOffset+=sizeof(avi_uint32);
        avi_uint32 size = 0;
        memcpy(&size,byteData,sizeof(avi_uint32));
        size = size + (size %2);
        if(!size)
        {
          badChunkCounter++;
        }
        else
        {
          badChunkCounter = 0;
        }
        nOffset+=size;
        MM_MSG_PRIO3(MM_FILE_OPS,
                      MM_PRIO_LOW,
                      "StartParsing encountered unknown FOURCC %x nsize %d offset %llu",
                      unknownFourCC,size,(nOffset-8));
      }
      if((m_nMoviOffset) && (badChunkCounter >= FILESOURCE_AVI_BAD_CHUNK_LIMIT))
      {
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
                      "StartParsing reached max bad chunks, break from parsing");
        break;
      }
    }//while( (nOffset < m_nFileSize) && (retError == AVI_SUCCESS) )

    if( (retError == AVI_SUCCESS) && (m_nCurrOffset > 0) )
    {
      avi_uint64 tempOffset = m_nCurrOffset;
      if(m_nIdx1Offset)
      tempOffset = skipToNextValidMediaChunk(m_nIdx1Offset);
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW,
                      "StartParsing current m_nCurrOffset is %llu",m_nCurrOffset);
      if(tempOffset != m_nCurrOffset)
      {
        m_nCurrOffset = tempOffset;
        m_nAdjustedMoviOffset = m_nCurrOffset;
        MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW,
                      "StartParsing updated m_nCurrOffset based on SkipToNextValidMediaChunk to %llu",m_nCurrOffset);
      }
      else
      {
        m_nAdjustedMoviOffset = 0;
      }
      m_CurrentParserState = AVI_PARSER_READY;
      m_nLastOffsetRead = m_nCurrOffset;
      MM_MSG_PRIO(MM_FILE_OPS,
                   MM_PRIO_LOW,
                   "StartParsing parsing done successfully!!");
    }
    else
    {
      if(m_bHttpStreaming)
      {
        m_nLastOffsetRead = nOffset;
        MM_MSG_PRIO(MM_FILE_OPS,
                      MM_PRIO_LOW,
                      "StartParsing parsing failed.. due to data underrun");
      }
      else
      {
        MM_MSG_PRIO3(MM_FILE_OPS,
                      MM_PRIO_LOW,
                      "StartParsing parsing failed.. nOffset %llu m_nFileSize %llu retError %d",
                      nOffset,m_nFileSize,retError);
      }
    }
  }//if(bFoundStartRiff)
  return retError;
}
#ifndef AVI_PARSER_FAST_START_UP
/* =============================================================================
FUNCTION:
  aviParser::SetIDX1Cache
DESCRIPTION:
 Copies the given idx1 cache pointer into this cache pointer

INPUT/OUTPUT PARAMETERS:

RETURN VALUE:

SIDE EFFECTS:
  None.
=============================================================================*/
void  aviParser::SetIDX1Cache(void* ptr)
{
  avi_idx1_table* table_ptr = (avi_idx1_table*)ptr;
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM, "SetIDX1Cache");
  if(table_ptr && table_ptr->pAudioEntries)
  {
    m_hAviSummary.pIdx1Table = (avi_idx1_table*)MM_Malloc(sizeof(avi_idx1_table));
    if(!m_hAviSummary.pIdx1Table)
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL, "SetIDX1Cache:Malloc Failed for pIdx1Table-AVI_OUT_OF_MEMORY");
    }
    else
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM, "SetIDX1Cache pIdx1Table is allocated");
      memset(m_hAviSummary.pIdx1Table,0,sizeof(avi_idx1_table));
      m_hAviSummary.pIdx1Table->nTotalSize = table_ptr->nTotalSize;
      m_hAviSummary.pIdx1Table->nStartOffset = table_ptr->nStartOffset;
      m_hAviSummary.pIdx1Table->nTotalAudioBytes = table_ptr->nTotalAudioBytes;
      m_hAviSummary.pIdx1Table->pAudioEntries = (avi_idx1_entry*)MM_Malloc(sizeof(avi_idx1_entry) * table_ptr->nAudioEntriesAllocated);
      if(m_hAviSummary.pIdx1Table->pAudioEntries)
      {
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM, "SetIDX1Cache pIdx1Table->pAudioEntries is allocated");
        memset(m_hAviSummary.pIdx1Table->pAudioEntries, 0, sizeof(avi_idx1_entry) * table_ptr->nAudioEntriesAllocated);
        m_hAviSummary.pIdx1Table->nAudioEntriesAllocated = table_ptr->nAudioEntriesAllocated;
        m_hAviSummary.pIdx1Table->nCurrParsedAudioEntriesInIDX1 = table_ptr->nCurrParsedAudioEntriesInIDX1;
        m_hAviSummary.pIdx1Table->nTotalEntriesInIDX1= table_ptr->nTotalEntriesInIDX1;
        m_hAviSummary.pIdx1Table->nAudioEntriesInIDX1= table_ptr->nAudioEntriesInIDX1;
        memcpy(m_hAviSummary.pIdx1Table->pAudioEntries,
               table_ptr->pAudioEntries,
               (sizeof(avi_idx1_entry) * table_ptr->nAudioEntriesAllocated) );
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM, "SetIDX1Cache pAudioEntries set up successful!!");
        table_ptr->nAudioEntriesAllocated = 0;
        table_ptr->nCurrParsedAudioEntriesInIDX1 = 0;
        table_ptr->nAudioEntriesInIDX1 = 0;
        MM_Free(table_ptr->pAudioEntries);
        table_ptr->pAudioEntries = NULL;
      }
      else
      {
        MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_FATAL, "SetIDX1Cache:Malloc Failed for pAudioEntries #entries %d",table_ptr->nAudioEntriesAllocated);
        MM_Free(m_hAviSummary.pIdx1Table);
        m_hAviSummary.pIdx1Table = NULL;
        table_ptr->nAudioEntriesAllocated = 0;
        table_ptr->nCurrParsedAudioEntriesInIDX1 = 0;
        table_ptr->nAudioEntriesInIDX1 = 0;
        MM_Free(table_ptr->pAudioEntries);
        table_ptr->pAudioEntries = NULL;
      }
    }
  }
}
/* =============================================================================
FUNCTION:
  aviParser::GetIDX1Cache
DESCRIPTION:
 Returns this cache pointer

INPUT/OUTPUT PARAMETERS:
RETURN VALUE:
SIDE EFFECTS:
  None.
=============================================================================*/
void* aviParser::GetIDX1Cache()
{
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM, "GetIDX1Cache");
  return (void*)m_hAviSummary.pIdx1Table;
}
#endif
/* =============================================================================
FUNCTION:
  aviParser::GetNumOfRiff

DESCRIPTION:
 parseHDRL

INPUT/OUTPUT PARAMETERS:

RETURN VALUE:

SIDE EFFECTS:
  None.
=============================================================================*/
aviErrorType aviParser::GetNumOfRiff(avi_uint64 offset, int* m_nNumOfRiff)
{
  unsigned char* byteData = m_ReadBuffer;
  avi_uint64 riffOffset = offset - sizeof(fourCC_t);
  avi_uint32 riffSize = 0;
  aviErrorType retError = AVI_CORRUPTED_FILE;

  while(riffOffset + 2*sizeof(avi_uint32) < m_nFileSize)
  {
    if(!parserAVICallbakGetData(riffOffset,2*sizeof(avi_uint32),
      m_ReadBuffer,AVI_READ_BUFFER_SIZE,m_pUserData,&retError))
    {
      retError = AVI_READ_FAILURE;
      break;
    }
    riffOffset += 2*sizeof(avi_uint32);
    if(!memcmp(byteData,AVI_START_BYTES,sizeof(fourCC_t)))
    {
      memcpy(&riffSize,byteData+4,sizeof(avi_uint32));
      riffSize = riffSize + (riffSize %2); //Allign to 16-bit
      //Check if we have VALID chucks ahead
      if(!parserAVICallbakGetData(riffOffset,2*sizeof(avi_uint32),
        m_ReadBuffer,AVI_READ_BUFFER_SIZE,m_pUserData,&retError))
      {
        retError = AVI_READ_FAILURE;
        break;
      }
      //All valid RIFFs after the first RIFF should have a AVIX atom
      if( (!memcmp(byteData,"AVIX",sizeof(fourCC_t))) )
      {
        (*m_nNumOfRiff)++;
      }
      riffOffset += riffSize;
      retError = AVI_SUCCESS;
    }
    else
    {
      //Any unknown FOURCC should be skipped
      fourCC_t unknownFourCC;
      memcpy(&unknownFourCC,byteData,sizeof(fourCC_t));
      avi_uint32 size = 0;
      memcpy(&size,byteData + sizeof(fourCC_t),sizeof(avi_uint32));
      size = size + (size %2);
      MM_MSG_PRIO3(MM_FILE_OPS,
        MM_PRIO_LOW,
        "GetNumOfRiff encountered unknown FOURCC %x nsize %d offset %llu, Skipping it!",
        unknownFourCC,size,(riffOffset-8));
      riffOffset += size;
    }
  }
  return retError;
}
/* =============================================================================
FUNCTION:
  aviParser::parseHDRL

DESCRIPTION:
 parseHDRL

INPUT/OUTPUT PARAMETERS:

RETURN VALUE:

SIDE EFFECTS:
  None.
=============================================================================*/

aviErrorType aviParser::parseHDRL(avi_uint64* offset, uint32 ulMaxSize)
{
  unsigned char* byteData = m_ReadBuffer;
  uint32 nSize = 0;
  aviErrorType retError = AVI_CORRUPTED_FILE;

  fourCC_t nCurrFourCC = 0;
  if(!parserAVICallbakGetData(*offset - 8, sizeof(fourCC_t), m_ReadBuffer,
                              AVI_READ_BUFFER_SIZE, m_pUserData, &retError))
  {
    return AVI_READ_FAILURE;
  }
  memcpy(&nSize,byteData,sizeof(avi_uint32));
  uint64 HDRLEndOffset = *offset + nSize - sizeof(avi_uint32);

  retError = parseAVIH(offset, ulMaxSize);

  while( (*offset < m_nFileSize) && (retError == AVI_SUCCESS) &&
         (*offset < HDRLEndOffset))
  {
    if(!parserAVICallbakGetData(*offset, 3*sizeof(fourCC_t), m_ReadBuffer,
                                AVI_READ_BUFFER_SIZE, m_pUserData, &retError))
    {
      break;
    }
    *offset+=sizeof(fourCC_t);

    if(!memcmp(byteData,AVI_LIST_FOURCC,sizeof(fourCC_t)))
    {
      //Get the list size
      avi_uint32 nListSize = 0;
      memcpy(&nListSize,byteData + sizeof(avi_uint32), sizeof(avi_uint32));
      *offset+=sizeof(avi_uint32);

      //get the next fourCC code
      memcpy(&nCurrFourCC,byteData + 2*sizeof(avi_uint32), sizeof(fourCC_t));
      *offset+=sizeof(fourCC_t);

      if(!memcmp(&nCurrFourCC,AVI_STRL_FOURCC,sizeof(fourCC_t)))
      {
        retError = parseSTRL(offset,nListSize);
      }
      else
      {
        *offset += nListSize - sizeof(avi_uint32);
      }
    }
    else
    {
      memcpy(&nSize,byteData + sizeof(avi_uint32),sizeof(avi_uint32));
      *offset += sizeof(fourCC_t) + nSize;
    }
  }
  //Defensive Check
  // This prevents parsing moving beyond the HDRL atom which happens
  // because of unknown atoms and corrupt size valaues.
  if(*offset > HDRLEndOffset)
  {
    *offset = HDRLEndOffset;
  }
  return retError;
}
/* =============================================================================
FUNCTION:
  aviParser::parseMOVI

DESCRIPTION:
 parseMOVI

INPUT/OUTPUT PARAMETERS:

RETURN VALUE:

SIDE EFFECTS:
  None.
=============================================================================*/

aviErrorType aviParser::parseMOVI(avi_uint64 offset)
{
  aviErrorType retError   = AVI_CORRUPTED_FILE;
  unsigned char* byteData = m_ReadBuffer;
  uint64 ullStartOffset   = offset;
  uint32 ulMoviSize       = 0;
  uint32 uAtomSize        = 0;

  bool bOK = false;
  /* Read Movie chunk size (8 bytes). */
  if(!parserAVICallbakGetData(offset - 8, FOURCC_SIGNATURE_BYTES,
                              m_ReadBuffer,AVI_READ_BUFFER_SIZE,m_pUserData,
                              &retError))
  {
    return AVI_READ_FAILURE;
  }
  memcpy(&ulMoviSize, byteData, FOURCC_SIGNATURE_BYTES);
  //! Run the loop till we get known chunk type
  //! Break the loop, if offset exceeds MOVI
  while(!bOK && ((offset - ullStartOffset) < ulMoviSize))
  {
    avi_uint16 tkID, cType = 0;
    bool bTkID = false;
    CHUNK_t Type;
    /* Read Chunk header data (8 bytes). */
    if(!parserAVICallbakGetData(offset,2*FOURCC_SIGNATURE_BYTES,
                                m_ReadBuffer,AVI_READ_BUFFER_SIZE,m_pUserData,
                                &retError))
    {
      break;
    }
    avi_uint32 chunkSize = 0;
    //! Get chunk type and Chunk (or Atom) size
    memcpy(&cType,byteData+2,sizeof(avi_uint16));
    memcpy(&uAtomSize,byteData+4,sizeof(avi_uint32));
    if( (!memcmp(&cType,"dc",sizeof(avi_uint16))) ||
        (!memcmp(&cType,"db",sizeof(avi_uint16))) ||
        (!memcmp(&cType,"wb",sizeof(avi_uint16))) ||
        (!memcmp(&cType,"dd",sizeof(avi_uint16)))
      )
    {
      memcpy(&tkID,byteData,sizeof(avi_uint16));
      tkID = ascii_2_short_int(&tkID, &bTkID);

      memcpy(&chunkSize, byteData+4, sizeof(avi_uint32));

      //validate chunk size
      if (ValidateChunkSize(chunkSize,m_nFileSize)) {
        MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH, "parseMOVI::DATA CHUNK SIZE CORRUPTED %llu",
                chunkSize);
        return AVI_CORRUPTED_FILE;
      }

      if(GetTrackChunkType(tkID,&Type)!=AVI_SUCCESS)
      {
        //! Skip unknown chunk type by its size
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM, "parseMOVI::unknown chunk type");
        offset += (2* FOURCC_SIGNATURE_BYTES + uAtomSize);
      }
      else if((  (Type == AVI_CHUNK_VIDEO)||(Type ==AVI_CHUNK_AUDIO))
              && (bTkID == true))
      {
        bOK = true;
      }
    }
    //! If it is list/REC chunk atom, just skip 4 bytes
    //! REC chunk internally contains media data
    else if((!memcmp(byteData,AVI_LIST_FOURCC,sizeof(fourCC_t))) ||
            (!memcmp(byteData,AVI_REC_FOURCC,sizeof(fourCC_t))))
    {
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
                   "parseMOVI::LIST/REC identified @ offset %llu", offset);
      offset += FOURCC_SIGNATURE_BYTES;
    }
    else
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,
                  "parseMOVI::Non A/V data encountered");
      offset += (2 * FOURCC_SIGNATURE_BYTES + uAtomSize);
    }
  }//! while(!bOK && offset < m_nFileSize)
  m_nCurrOffset = offset;
  m_nMoviOffset = m_nCurrOffset;

  #ifdef FEATURE_FILESOURCE_AVI_PARSER_DEBUG
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW,
                  "StartParsing encountered MOVI at offset %d",
                    m_nMoviOffset);
  #endif
  /*
  * For MOVI, size of LIST includes 'MOVI' and all media payloads.
  * Since we have already read in MOVI, while adjusting the offset for next FOURCC,
  * we need to subtract 'MOVI' bytes, which is, sizeof(fourCC_t) from length.
  * This length is inclusive of any IX if available.
  */
  //offset+=nListSize-sizeof(fourCC_t);
  retError = AVI_SUCCESS;
  return retError;
 }

/* =============================================================================
FUNCTION:
  aviParser::parseJUNK

DESCRIPTION:
 parseJUNK

INPUT/OUTPUT PARAMETERS:

RETURN VALUE:

SIDE EFFECTS:
  None.
=============================================================================*/

aviErrorType aviParser::parseJUNK(avi_uint64* offset)
{
  unsigned char* byteData = m_ReadBuffer;
  aviErrorType retError = AVI_CORRUPTED_FILE;
  avi_uint32 size;
  /*
  * Read in JUNK size to skip it as there is nothing to parse in JUNK.
  */
  if(!parserAVICallbakGetData(*offset,sizeof(avi_uint32),
    m_ReadBuffer,AVI_READ_BUFFER_SIZE,m_pUserData,&retError))
  {
    return retError;
  }
  *offset+=sizeof(avi_uint32);
  memcpy(&size,byteData,sizeof(avi_uint32));

  size = size + (size %2); // Allign to 16 bit boundary

  *offset+=size;
  retError = AVI_SUCCESS;
  return retError;
}
/* =============================================================================
FUNCTION:
  aviParser::GetTrackChunkType

DESCRIPTION:
 Returns track type(audio/video) for given track trackId.

INPUT/OUTPUT PARAMETERS:

RETURN VALUE:

SIDE EFFECTS:
  None.
=============================================================================*/
aviErrorType aviParser::GetTrackChunkType(avi_uint32 trackId,CHUNK_t* type)
{
  if( (trackId < m_hAviSummary.n_streams) && (trackId < AVI_MAX_TRACKS) )
  {
    *type = m_hAviSummary.stream_index[trackId].type;
    return AVI_SUCCESS;
  }
  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_FATAL,
               "GetTrackChunkType AVI_INVALID_USER_DATA trackID=%d",trackId);
  return AVI_INVALID_USER_DATA;
}
/* =============================================================================
FUNCTION:
  aviParser::GetClipDurationInMsec

DESCRIPTION:
 Returns clip duration from the current parsed meta-data.

INPUT/OUTPUT PARAMETERS:

RETURN VALUE:

SIDE EFFECTS:
  None.
=============================================================================*/
avi_uint64 aviParser::GetClipDurationInMsec()
{
  avi_uint64 nDuration = 0;
  avi_uint64 maxDuration = 0;

  for (uint32 i =0;i<m_hAviSummary.n_streams;i++)
  {
    nDuration = GetTrackDuration(m_hAviSummary.stream_index[i].index);
    maxDuration = FILESOURCE_MAX(maxDuration, nDuration);
  }
  return maxDuration;
}
/* =============================================================================
FUNCTION:
  aviParser::GetTrackDuration

DESCRIPTION:
 Returns duration for given track.
 APP can take MAX across all available tracks to get the clip duration.

INPUT/OUTPUT PARAMETERS:

RETURN VALUE:

SIDE EFFECTS:
  None.
=============================================================================*/
avi_uint64 aviParser::GetTrackDuration(avi_uint32 trackId)
{
  avi_video_info vinfo;
  avi_audiotrack_summary_info ainfo;
  avi_audio_info audtrackInfo;
  CHUNK_t Type;
  avi_uint64 nDuration = 0;
  if(GetTrackChunkType(trackId,&Type)==AVI_SUCCESS)
  {
    switch(Type)
    {
      case AVI_CHUNK_VIDEO:
        if(GetVideoInfo(trackId,&vinfo)==AVI_SUCCESS)
        {
          if(vinfo.strhVideo.dwRate > 0)
          {
            float frate = (float)vinfo.strhVideo.dwScale/(float)vinfo.strhVideo.dwRate;
            float ulTotalFrames = (float)vinfo.strhVideo.dwLength;
            /*For VBR clips, dwDuration field gives total number of frames*/
            if(m_base_indx_tbl[trackId].isAvailable)
            {
              ulTotalFrames = 0;
              for(avi_uint32 i = 0 ; i < m_base_indx_tbl[trackId].nEntriesInUse ; i++)
              {
                ulTotalFrames += m_base_indx_tbl[trackId].pIndxSuperIndexEntry[i].dwDuration;
              }
            }
            nDuration = (avi_uint64)(frate*ulTotalFrames*1000);
            #ifdef FEATURE_FILESOURCE_AVI_PARSER_DEBUG
              MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
                            "GetTrackDuration video duration %d",nDuration);
            #endif
          }
        }
        break;
      case AVI_CHUNK_AUDIO:
        if( (GetAudioTrackSummaryInfo(trackId,&ainfo)==AVI_SUCCESS) &&
            (GetAudioInfo(trackId,&audtrackInfo)==AVI_SUCCESS) )
        {
          if(!ainfo.isVbr && (ainfo.audioBytesPerSec))
          {
            avi_uint64 ullTotalAudioBytes = 0;
            if(m_base_indx_tbl[trackId].isAvailable)
            {
              for(avi_uint32 i = 0 ; i < m_base_indx_tbl[trackId].nEntriesInUse ; i++)
              {
                ullTotalAudioBytes += m_base_indx_tbl[trackId].pIndxSuperIndexEntry[i].dwDuration;
              }
              if( ainfo.nBlockAlign)
              {
                nDuration = (avi_uint64)((float)ullTotalAudioBytes /
                    (float)(((float)ainfo.audioBytesPerSec)/
                    ainfo.nBlockAlign) * 1000.0f);
              }
              else
              {
                nDuration = (avi_uint64)((float)ullTotalAudioBytes /
                    (float)(ainfo.audioBytesPerSec) * 1000.0f);
              }
            }
            else
#ifndef AVI_PARSER_FAST_START_UP
            //Calculate the duration using the total nummber of audio bytes in the track.
            //This will result in the duration being returned as 0 till the idx1 cache is completed.
            if(m_hAviSummary.pIdx1Table && ainfo.audioBytesPerSec && m_hAviSummary.pIdx1Table->nTotalAudioBytes)
            {
              nDuration = (avi_uint64)((float)m_hAviSummary.pIdx1Table->nTotalAudioBytes /
                          (float)ainfo.audioBytesPerSec * 1000.0f);
            }
            else
#endif
            {
              float framerate =  ( (float)audtrackInfo.strhAudio.dwScale /
                                            (float)audtrackInfo.strhAudio.dwRate );
              nDuration = (avi_uint64)((framerate * 1000)* audtrackInfo.strhAudio.dwLength);
            }
            #ifdef FEATURE_FILESOURCE_AVI_PARSER_DEBUG
              MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
                            "GetTrackDuration CBR Audio:nDuration %d",nDuration);
            #endif
          }
          else if(ainfo.isVbr && ainfo.audioFrequency)
          {
            avi_uint32 ulTotalFrames = audtrackInfo.strhAudio.dwLength;
            /*For VBR clips, dwDuration field gives total number of frames*/
            if(m_base_indx_tbl[trackId].isAvailable)
            {
              ulTotalFrames = 0;
              for(avi_uint32 i = 0 ; i < m_base_indx_tbl[trackId].nEntriesInUse ; i++)
              {
                ulTotalFrames += m_base_indx_tbl[trackId].pIndxSuperIndexEntry[i].dwDuration;
              }
            }
            nDuration = (avi_uint64)(( ( (float)audtrackInfo.strhAudio.dwScale/
                                      (float)audtrackInfo.strhAudio.dwRate) * 1000.f) * (float)ulTotalFrames);

            #ifdef FEATURE_FILESOURCE_AVI_PARSER_DEBUG
              MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
                            "GetTrackDuration VBR Audio:nDuration %d",nDuration);
            #endif
          }
        }
        break;
        default:
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
                       "GetTrackDuration returing 0 for non audio/video track");
        break;
    }
  }
  return nDuration;
}
/* =============================================================================
FUNCTION:
 aviParser::GetAVIHeader

DESCRIPTION:
Returns AVIH from the clip.

INPUT/OUTPUT PARAMETERS:

RETURN VALUE:

SIDE EFFECTS:
  None.
=============================================================================*/
aviErrorType aviParser::GetAVIHeader(avi_mainheader_avih* pAviHdrPtr)
{
  if(!pAviHdrPtr)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
               "GetAVIHeader AVI_INVALID_USER_DATA");
    return AVI_INVALID_USER_DATA;
  }
  if(!m_hAviSummary.avih)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
               "GetAVIHeader,NULL AVI Header,AVI_PARSE_ERROR");
    return AVI_PARSE_ERROR;
  }
  memset(pAviHdrPtr,0,sizeof(avi_mainheader_avih));
  memcpy(pAviHdrPtr,m_hAviSummary.avih,sizeof(avi_mainheader_avih));
  return AVI_SUCCESS;
}
/* =============================================================================
FUNCTION:
 aviParser::GetAudioInfo

DESCRIPTION:
Returns audio track info for given trackId.

INPUT/OUTPUT PARAMETERS:

RETURN VALUE:

SIDE EFFECTS:
  None.
=============================================================================*/
aviErrorType aviParser::GetAudioInfo(avi_uint32 trackId,avi_audio_info* pAudioInfo)
{
  aviErrorType retError = AVI_INVALID_USER_DATA;
  if(!pAudioInfo)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
               "GetAudioInfo AVI_INVALID_USER_DATA");
    return AVI_INVALID_USER_DATA;
  }
  memset(pAudioInfo,0,sizeof(avi_audio_info));
  if(m_hAviSummary.n_streams <= trackId)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                 "GetAudioInfo AVI_INVALID_USER_DATA");
    return AVI_INVALID_USER_DATA;
  }
  for(int i = 0; i< AVI_MAX_TRACKS; i++)
  {
    if( (m_hAviSummary.stream_index[i].index == trackId)&&
        (m_hAviSummary.stream_index[i].type == AVI_CHUNK_AUDIO))
    {
      memcpy(pAudioInfo,&(m_hAviSummary.audio_info[m_hAviSummary.stream_index[i].audioIndex]),sizeof(avi_audio_info));
      retError = AVI_SUCCESS;
    }
  }
  return retError;
}
/* =============================================================================
FUNCTION:
 aviParser::GetVideoInfo

DESCRIPTION:
Returns video track info for given trackId.

INPUT/OUTPUT PARAMETERS:

RETURN VALUE:

SIDE EFFECTS:
  None.
=============================================================================*/
aviErrorType aviParser::GetVideoInfo(avi_uint32 trackId,avi_video_info* pVideoInfo)
{
  aviErrorType retError = AVI_INVALID_USER_DATA;
  if(!pVideoInfo)
  {
     MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                  "GetVideoInfo AVI_INVALID_USER_DATA");
    return AVI_INVALID_USER_DATA;
  }
  memset(pVideoInfo,0,sizeof(avi_video_info));
  if(m_hAviSummary.n_streams <= trackId)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                 "GetVideoInfo AVI_INVALID_USER_DATA");
    return AVI_INVALID_USER_DATA;
  }
  for(int i = 0; i< AVI_MAX_TRACKS; i++)
  {
    if( (m_hAviSummary.stream_index[i].index == trackId)&&
        (m_hAviSummary.stream_index[i].type == AVI_CHUNK_VIDEO))
    {
      memcpy(pVideoInfo,&(m_hAviSummary.video_info[m_hAviSummary.stream_index[i].videoIndex]),sizeof(avi_video_info));
      retError = AVI_SUCCESS;
    }
  }
  return retError;
}
/* =============================================================================
FUNCTION:
 aviParser::GetLastRetrievedSampleOffset

DESCRIPTION:
Returns the absolute file offset of the last successful retrieved sample.

INPUT/OUTPUT PARAMETERS:

RETURN VALUE:

SIDE EFFECTS:
  None.
=============================================================================*/
uint64 aviParser::GetLastRetrievedSampleOffset(uint32 /*id*/)
{
  return m_nCurrOffset;
}
/* =============================================================================
FUNCTION:
 aviParser::GetDRMInfo

DESCRIPTION:
Returns DRM header for and the size of the header.

INPUT/OUTPUT PARAMETERS:

RETURN VALUE:

SIDE EFFECTS:
  None.
=============================================================================*/
avi_uint8* aviParser::GetDRMInfo(int* size)
{
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM, "GetDRMInfo");
  if( (!size)||(!m_bDRMProtection)||(!m_AviSTRDHeader.drm_info) )
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM, "GetDRMInfo Error,returning NULL");
    return NULL;
  }
  *size = m_AviSTRDHeader.drm_size;
  return m_AviSTRDHeader.drm_info;
}
/* =============================================================================
FUNCTION:
 aviParser::GetAVIVolHeader

DESCRIPTION:
Returns VOL header for given video trackId.

INPUT/OUTPUT PARAMETERS:

RETURN VALUE:

SIDE EFFECTS:
  None.
=============================================================================*/
avi_uint8* aviParser::GetAVIVolHeader(avi_uint32   trackId)
{
  unsigned char* byteData = m_ReadBuffer;
  bool bOk = false;
  avi_int8 volhdrbuff[MAX_VOL_HDR_SIZE]={0};
  int writeOffset = 0;
  avi_uint8* retBuffer = NULL;
  avi_video_info vInfo;
  int bdivxfourcc = 1;
  aviErrorType retError = AVI_SUCCESS;

  #ifdef FEATURE_FILESOURCE_AVI_PARSER_DEBUG
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW, "GetVolHeader");
  #endif
  if( ( m_hAviSummary.n_streams <= trackId ) || (AVI_MAX_TRACKS <= trackId) )
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH, "GetAVIVolHeader AVI_INVALID_USER_DATA");
    return retBuffer;
  }
  if(trackId < AVI_MAX_TRACKS)
  {
    if(m_AviVOLHeader[trackId]->pMemory)
    {
      return m_AviVOLHeader[trackId]->pMemory;
    }
  }

  if(GetVideoInfo(trackId,&vInfo) != AVI_SUCCESS)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH, "GetAVIVolHeader GetVideoInfo failed");
    return retBuffer;
  }

  for(int i = 0; i< AVI_MAX_TRACKS; i++)
  {
    if( (m_hAviSummary.stream_index[i].index == trackId)   &&
        (m_hAviSummary.stream_index[i].type == AVI_CHUNK_VIDEO) &&
        ( (memcmp(&vInfo.strfVideo.biCompression,"DXSA",sizeof(fourCC_t))) &&
          (memcmp(&vInfo.strfVideo.biCompression,"DXSB",sizeof(fourCC_t))) ) )
    {
      bOk = true;
      break;
    }
  }
  if(!bOk)
  {
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
                   "GetAVIVolHeader was called for non Video track %d",trackId);
    return retBuffer;
  }
  bOk = false;
  avi_uint64 offset = m_nMoviOffset;
  avi_uint32 dwChunkId=0,dwSize=0;
  avi_uint16 tkId=0,cType=0;

  //Now locate the video frame for given track-id.
  while(offset < m_nFileSize)
  {
    //if read fails for any reason, we need to break out of the loop...
    if(!parserAVICallbakGetData(offset,(2*sizeof(avi_uint32)),
      m_ReadBuffer,AVI_READ_BUFFER_SIZE,m_pUserData,&retError))
    {
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
                   "GetAVIVolHeader parserAVICallbakGetData failed offset %llu",offset);
      return retBuffer;
    }

    offset += (2*sizeof(avi_uint32));
    memcpy(&dwChunkId,byteData,sizeof(avi_uint32));
    memcpy(&dwSize,byteData+sizeof(avi_uint32),sizeof(avi_uint32));

    memcpy(&tkId,(avi_uint8*)&dwChunkId,sizeof(avi_uint16));
    tkId = ascii_2_short_int(&tkId);
    //check if ascii_2_short_int could locate the valid track id in dwChunkId
    if(AVI_INVALID_TRACK_ID != tkId)
    {
      //valid track id detected, copy over chunk type
      memcpy(&cType,((unsigned char*)&dwChunkId)+2,sizeof(avi_uint16));
    }
    else
    {
      //check if we have standard index "ix##"
      if(!memcmp(&dwChunkId,AVI_IX_FOURCC,sizeof(avi_uint16)))
      {
        //copy over chunk type as this will be useful in skipping the entire chunk based on it's size.
        memcpy(&cType,((unsigned char*)&dwChunkId),sizeof(avi_uint16));
      }
    }

    if ( ((!memcmp(&cType,"dc",sizeof(avi_uint16))) || (!memcmp(&cType,"db",sizeof(avi_uint16))))&&
         (trackId==tkId) && (dwSize) )
    {
      bOk = true;
      #ifdef FEATURE_FILESOURCE_AVI_PARSER_DEBUG
        MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW,
                      "GetVolHeader located corrected video track id %d",tkId);
      #endif
      break;
    }
    else if ( (!memcmp(&cType,"wb",sizeof(avi_uint16))) ||
              (!memcmp(&cType,"dd",sizeof(avi_uint16))) ||
              (!memcmp(&cType,"dc",sizeof(avi_uint16))) ||
              (!memcmp(&cType,"sb",sizeof(avi_uint16))) ||
              (!memcmp(&cType,"ix",sizeof(avi_uint16))) ||
              (!memcmp(byteData,AVI_JUNK_FOURCC,sizeof(fourCC_t))) ||
              (!memcmp(byteData,AVI_RES_FOURCC,3*sizeof(char)))
            )
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,
                      "GetAVIVolHeader located wb/dd//dc/sb/ix or JUNK..");
      offset += dwSize;
    }
    else
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
                      "GetAVIVolHeader located bad data");
      offset -= (2*sizeof(avi_uint32));   //Undoing what we read earlier
      offset += sizeof(fourCC_t);         //Going to next 4 bytes to read fourcc
    }
    if(dwSize %2)
    {
      //Must align to 16-bit boundary
      offset++;
    }
  }

  if(!bOk)
  {
    if(offset >= m_nFileSize)
    {
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
                   "GetAVIVolHeader reached end of file,no format block detected for track-id %u",trackId);
    }
     //Could not locate video frame for given track-id.
     //Something is wrong or there is blank video track(??)
     MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
                   "GetAVIVolHeader Failed to locate VOL header for track %d",trackId);
     return retBuffer;
  }

  if( (!memcmp(&vInfo.strhVideo.fccHandler,"XVID",sizeof(fourCC_t))) ||
      (!memcmp(&vInfo.strhVideo.fccHandler,"xvid",sizeof(fourCC_t))) ||
      (!memcmp(&vInfo.strhVideo.fccHandler,"FMP4",sizeof(fourCC_t))) ||
      (!memcmp(&vInfo.strhVideo.fccHandler,"fmp4",sizeof(fourCC_t))) ||
      (!memcmp(&vInfo.strhVideo.fccHandler,"yv12",sizeof(fourCC_t))) ||
      (!memcmp(&vInfo.strhVideo.fccHandler,"MPG4",sizeof(fourCC_t))) ||
      (!memcmp(&vInfo.strhVideo.fccHandler,"mpg4",sizeof(fourCC_t))) ||
      (!memcmp(&vInfo.strhVideo.fccHandler,"wmv2",sizeof(fourCC_t))) ||
      (!memcmp(&vInfo.strhVideo.fccHandler,"WMV2",sizeof(fourCC_t))) ||
      (!memcmp(&vInfo.strhVideo.fccHandler,"YV12",sizeof(fourCC_t))) ||
      (!memcmp(&vInfo.strhVideo.fccHandler,"WMV3",sizeof(fourCC_t))) ||
      (!memcmp(&vInfo.strhVideo.fccHandler,"wmv3",sizeof(fourCC_t)))
    )
  {
    bdivxfourcc = 0;
  }
  else
  {
    bdivxfourcc = 1;
  }

  if(bdivxfourcc == 1)
  {
    if(!parserAVICallbakGetData(offset,(2*sizeof(avi_uint32)),
      m_ReadBuffer,AVI_READ_BUFFER_SIZE,m_pUserData,&retError))
    {
        return retBuffer;
    }
    offset += (2*sizeof(avi_uint32));

    int error = memcmp(byteData,&IVOP_START_CODE,sizeof(avi_uint32) );
    error += memcmp(byteData+sizeof(avi_uint32),&IVOP_VOL_START_CODE,sizeof(avi_uint32));

    if(error )
    {
     //Not sure how to locate VOL header so signal the failure.
     MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
                   "GetAVIVolHeader track %d does not start with SEQUENCE_START_CODE || START_CODE",trackId);
    }
    else
    {
      /*
      * Dump VISUAL_OBJECT_SEQUENCE_START_CODE,
      * MPEG4_PROFILE_AND_LEVEL_INDICATION,
      * MPEG4_VISUAL_OBJECT_START_CODE,
      * MPEG4_VIDEO_OBJECT_IDEN_VIDEO_SIG_TYPE
      * IVOP_START_CODE
      * IVOP_VOL_START_CODE into output buffer.
      */
      memcpy(volhdrbuff+writeOffset,&MPEG4_VISUAL_OBJECT_SEQUENCE_START_CODE,sizeof(avi_uint32));
      writeOffset += sizeof(avi_uint32);

      memcpy(volhdrbuff+writeOffset,&MPEG4_PROFILE_AND_LEVEL_INDICATION,sizeof(avi_uint8));
      writeOffset += sizeof(avi_uint8);

      memcpy(volhdrbuff+writeOffset,&MPEG4_VISUAL_OBJECT_START_CODE,sizeof(avi_uint32));
      writeOffset += sizeof(avi_uint32);

      memcpy(volhdrbuff+writeOffset,&MPEG4_VIDEO_OBJECT_IDEN_VIDEO_SIG_TYPE,sizeof(avi_uint8));
      writeOffset += sizeof(avi_uint8);

      memcpy(volhdrbuff+writeOffset,&IVOP_START_CODE,sizeof(avi_uint32));
      writeOffset += sizeof(avi_uint32);
      memcpy(volhdrbuff+writeOffset,&IVOP_VOL_START_CODE,sizeof(avi_uint32));
      writeOffset += sizeof(avi_uint32);
    }
  }

  bool bError = false;
  avi_int8 val = 0;

  /*/
  * When video fourCC is MPEG4/mpeg4, just copy the video strf as  it is
  * since there is no explivit VOL header for this codec.
  */

  if( (!memcmp(&vInfo.strhVideo.fccHandler,"MPG4",sizeof(fourCC_t))) ||
      (!memcmp(&vInfo.strhVideo.fccHandler,"mpg4",sizeof(fourCC_t)))
     )
  {
    /*
    * STRF structure stores the size of entire structure in 'biSize' field which
    * is avi_uint32. Subtract sizeof(avi_uint32) from total size to get
    * count of bytes belonging to decoder specific content.
    */
    m_VolSize = sizeof(vInfo.strfVideo)-sizeof(avi_uint32);
    //Now copy over the STRF into volhdrbuff
    memcpy(volhdrbuff,(avi_uint8*)&vInfo.strfVideo+sizeof(avi_uint32),m_VolSize);
  }
  else if( (!memcmp(&vInfo.strhVideo.fccHandler,"WMV2",sizeof(fourCC_t))) ||
           (!memcmp(&vInfo.strhVideo.fccHandler,"wmv2",sizeof(fourCC_t))) ||
           (!memcmp(&vInfo.strhVideo.fccHandler,"WMV3",sizeof(fourCC_t))) ||
           (!memcmp(&vInfo.strhVideo.fccHandler,"wmv3",sizeof(fourCC_t))) ||
           (!memcmp(&vInfo.strhVideo.fccHandler,"WVC1",sizeof(fourCC_t))) ||
           (!memcmp(&vInfo.strhVideo.fccHandler,"wvc1",sizeof(fourCC_t)))
     )
  {
    /*
    * STRF structure stores the codec configuration header as an extra data.
    * Check if there is any extra data pointed by STRF and copy if it exists.
    */
    m_VolSize = vInfo.strfVideo.nExtraData;
    //Now copy over the STRF->pExtraData into volhdrbuff
    memcpy(volhdrbuff,(avi_uint8*)vInfo.strfVideo.pExtraData,m_VolSize);
  }
  else
  {
    //if pExtraData exists, check if it has VOL header
    if(vInfo.strfVideo.nExtraData)
    {
      bool foundVOLHeader = false;
      int index = 0;
      while((index+4) < vInfo.strfVideo.nExtraData)
      {
        if(!memcmp(vInfo.strfVideo.pExtraData+index,&IVOP_VOL_START_CODE,sizeof(avi_uint32) ))
        {
          foundVOLHeader = true;
          break;
        }
        index++;
      }
      if(foundVOLHeader)
      {
        if(vInfo.strfVideo.nExtraData <= MAX_VOL_HDR_SIZE)
        {
          m_VolSize = vInfo.strfVideo.nExtraData;
          memcpy(volhdrbuff,(avi_uint8*)vInfo.strfVideo.pExtraData,m_VolSize);
        }
        else
        {
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL, "GetAVIVolHeader vol size > MAX_VOL_HDR_SIZE");
        }
      }
    }
    else
    {
      while( (!isVOLReadDone(trackId,offset,&val,&bError,NULL)) && (!bError) )
      {
        memcpy(volhdrbuff+writeOffset,&val,sizeof(avi_int8));
        writeOffset += sizeof(avi_uint8);
        offset++;
        if(writeOffset >= MAX_VOL_HDR_SIZE)
        {
          MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_FATAL,
                       "GetAVIVolHeader size=%d >MAX_VOL_HDR_SIZE=%d"
                       ,writeOffset, MAX_VOL_HDR_SIZE);
          return retBuffer;
        }
      }
      m_VolSize = writeOffset;
    }
  }

  if(trackId < AVI_MAX_TRACKS)
  {
    if((m_AviVOLHeader[trackId]) && (m_VolSize))
    {
      m_AviVOLHeader[trackId]->pMemory = (avi_uint8*)MM_Malloc(m_VolSize);
      if(!m_AviVOLHeader[trackId]->pMemory)
      {
        MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_FATAL,
                     "GetAVIVolHeader failed to allocate memory for %d bytes",m_VolSize);
        return retBuffer;
      }
      memcpy(m_AviVOLHeader[trackId]->pMemory,volhdrbuff,m_VolSize);
      m_AviVOLHeader[trackId]->nSize = m_VolSize;
      retBuffer = m_AviVOLHeader[trackId]->pMemory;
    }
  }
  return retBuffer;
}
/* =============================================================================
FUNCTION:
 aviParser::isVOLReadDone

DESCRIPTION:
Reads a byte for VOL header untill VOP HEADER CODE is encountered.

INPUT/OUTPUT PARAMETERS:

RETURN VALUE:

SIDE EFFECTS:
  None.
=============================================================================*/
bool aviParser::isVOLReadDone(avi_uint32 trackId, avi_uint64 offset, avi_int8* val,bool* bError,avi_uint8* membuf)
{
  unsigned char* byteData = m_ReadBuffer;
  bool bRet = false;
  avi_video_info vInfo;
  aviErrorType retError = AVI_SUCCESS;

  if( (!val)||(!bError) )
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                    "isVOLReadDone invalid parameter!!");
    return true;
  }
  *bError = false;
  if(membuf == NULL)
  {
    if(!parserAVICallbakGetData(offset,(4 *sizeof(avi_uint8)),
      m_ReadBuffer,AVI_READ_BUFFER_SIZE,m_pUserData,&retError))
    {
      *bError = true;
      return *bError;
    }
  }
  if(GetVideoInfo(trackId,&vInfo)==AVI_SUCCESS)
  {
    int code = MPEG4_VOP_HEADER_CODE;
    if(
        (!memcmp(&vInfo.strhVideo.fccHandler,"FMP4",sizeof(fourCC_t)))  ||
        (!memcmp(&vInfo.strhVideo.fccHandler,"fmp4",sizeof(fourCC_t)))
      )
      {
        code  = MPEG4_VOP_HEADER_CODE_FMP4;   //FMP4 clips have VolHeader ending with this code
      }
    if(!memcmp(byteData,&code,(4 *sizeof(avi_uint8))))
    {
      bRet = true;
      #ifdef FEATURE_FILESOURCE_AVI_PARSER_DEBUG
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW,"isVOLReadDone located VOP_HEADER_CODE");
      #endif
      return bRet;
    }
  }
  else
  {
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
                   "GetVideoInfo failed to return video info for the given trackId %d",trackId);
  }

  memcpy(val,byteData,sizeof(avi_int8));
  return bRet;
}
/* =============================================================================
FUNCTION:
 aviParser::GetAVIVolHeaderSize

DESCRIPTION:
Returns VOL headersize for given trackID.

INPUT/OUTPUT PARAMETERS:

RETURN VALUE:

SIDE EFFECTS:
  None.
=============================================================================*/
avi_uint32 aviParser::GetAVIVolHeaderSize(avi_uint32 trackId)
{
#ifdef FEATURE_FILESOURCE_AVI_PARSER_DEBUG
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW, "GetAVIVolHeaderSize");
#endif
  if(m_AviVOLHeader[trackId])
  {
    return m_AviVOLHeader[trackId]->nSize;
  }
  return 0;
}
/* =============================================================================
FUNCTION:
 aviParser::GetCurrentSample

DESCRIPTION:
Returns current sample for the given trackId.

INPUT/OUTPUT PARAMETERS:

RETURN VALUE:

SIDE EFFECTS:
  None.
=============================================================================*/
aviErrorType aviParser::GetCurrentSample(avi_uint32 trackId,avi_uint8* dataBuffer,
                                         avi_uint32 nMaxBufSize,
                                         avi_uint32 nBytesNeeded)
{
  aviErrorType retError = AVI_SUCCESS;

  #ifdef FEATURE_FILESOURCE_AVI_PARSER_DEBUG
    MM_MSG_PRIO3(MM_FILE_OPS, MM_PRIO_LOW,
                        "GetCurrentSample trackId %d nMaxBufSize %d nBytesNeeded %d",
                        trackId,nMaxBufSize,nBytesNeeded);
  #endif
  if( (!dataBuffer) || (!nMaxBufSize) || (!m_hAviSummary.n_streams) || (!nBytesNeeded) )
  {
    MM_MSG_PRIO(MM_FILE_OPS,
                 MM_PRIO_HIGH,
                 "GetCurrentSample AVI_INVALID_USER_DATA");
    return AVI_INVALID_USER_DATA;
  }
  if(trackId < m_hAviSummary.n_streams)
  {
    if(m_CurrentParserState != AVI_PARSER_CHUNK_DATA_START)
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
      "INVALID PARSER STATE.Please retrieve sampleInfo first before retrieving actual sample!!");
      return AVI_FAILURE;
    }
    if(nMaxBufSize < m_nCurrentChunkDataSize)
    {
      //Buffer size provided is < the chunk data size,
      //report failure and print an error message.
      MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_ERROR,
                    "Buffer size provided %d  < the current chunk data size %d",nMaxBufSize,m_nCurrentChunkDataSize);
      return AVI_INSUFFICIENT_BUFFER;
    }
    if(!parserAVICallbakGetData(m_nCurrOffset,nBytesNeeded,
      dataBuffer,nMaxBufSize,m_pUserData,&retError))
    {
      return retError;
    }
    m_nCurrOffset+=nBytesNeeded;
    if (nBytesNeeded % 2)
    {
      // must align to 16-bit boundary
      m_nCurrOffset++;
    }
    m_CurrentParserState = AVI_PARSER_CHUNK_HEADER_START;
    return AVI_SUCCESS;
  }
   MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                "GetCurrentSample AVI_INVALID_USER_DATA");
  return AVI_INVALID_USER_DATA;
}
/* =============================================================================
FUNCTION:
 aviParser::GetNextSampleInfo

DESCRIPTION:
Returns next available sample info for given trackId.
APP needs to call GetNextSampleInfo before retrieving the media sample.

INPUT/OUTPUT PARAMETERS:

RETURN VALUE:

SIDE EFFECTS:
  None.
=============================================================================*/
aviErrorType aviParser::GetNextSampleInfo(avi_uint32 trackId,
                                          avi_sample_info* sampleInfo,
                                          avi_uint32 nMaxBufSize,
                                          avi_uint16* trackIdFound)
{
  aviErrorType retError = AVI_FAILURE;

  if( (!sampleInfo) || (!trackIdFound) || (!m_hAviSummary.n_streams) )
  {
   MM_MSG_PRIO(MM_FILE_OPS,
                MM_PRIO_FATAL,
                "GetNextSampleInfo AVI_INVALID_USER_DATA");
    return AVI_INVALID_USER_DATA;
  }
  memset(sampleInfo,0,sizeof(avi_sample_info));
  *trackIdFound = 0;

  #ifdef FEATURE_FILESOURCE_AVI_PARSER_DEBUG
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW,
                  "GetNextSampleInfo trackId %d",trackId);
  #endif

  #ifdef FEATURE_FILESOURCE_AVI_PARSER_DEBUG
    MM_MSG_PRIO3(MM_FILE_OPS, MM_PRIO_LOW,
                "GetNextSampleInfo m_nCurrOffset %d m_nFileSize %d m_nIdx1Offset %d",
                m_nCurrOffset,m_nFileSize,m_nIdx1Offset);
  #endif

  if(trackId < m_hAviSummary.n_streams)
  {
    if(m_CurrentParserState == AVI_PARSER_CHUNK_DATA_START)
    {
       MM_MSG_PRIO2(MM_FILE_OPS,
                     MM_PRIO_LOW,
                     "Previous sample not retrieved before reading next sampleInfo!!current m_nCurrOffset %llu ADJUSTING TO %llu",
                     m_nCurrOffset,(m_nCurrOffset-8));
      m_nCurrOffset -= 8;
      m_nCurrentChunkDataSize = 0;
      m_CurrentParserState = AVI_PARSER_CHUNK_HEADER_START;
      //Since we went back by a sample,adjust IDX1 offset location
      if(m_nCurrentSampleInfoOffsetInIdx1 > (4 * sizeof(avi_uint32)))
      {
        m_nCurrentSampleInfoOffsetInIdx1 -= ( 4 * sizeof(avi_uint32) );
      }
      else
      {
        //If we reach here, something has gone bad, be on lookout!!!
        MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_FATAL,
                      "GetNextSampleInfo detected invalid m_nCurrentSampleInfoOffsetInIdx1 %llu ",
                      m_nCurrentSampleInfoOffsetInIdx1);
      }
    }
    if( (m_CurrentParserState != AVI_PARSER_CHUNK_HEADER_START) &&
        (m_CurrentParserState != AVI_PARSER_READY) )
    {
      /*
      * PARSER needs to be in AVI_PARSER_CHUNK_HEADER_START or AVI_PARSER_READY
      * when retrieving samples. If not, we should bail out immediately.
      */
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_FATAL,"INVALID PARSER STATE %d ",m_CurrentParserState);
      return AVI_PARSE_ERROR;
    }
    setParserState(AVI_PARSER_UPDATE_SAMPLES,&retError);
    retError = GetSampleInfo(&m_nCurrOffset,trackId,sampleInfo,nMaxBufSize, trackIdFound);
  }
  else
  {
    retError = AVI_INVALID_USER_DATA;
    MM_MSG_PRIO(MM_FILE_OPS,
                     MM_PRIO_FATAL,
                     "GetNextSampleInfo AVI_INVALID_USER_DATA");
  }
  return retError;
}
/* =============================================================================
FUNCTION:
 aviParser::GetSampleInfo

DESCRIPTION:
Returns next available sample info for given trackId.
APP needs to call GetNextSampleInfo before retrieving the media sample.

INPUT/OUTPUT PARAMETERS:

RETURN VALUE:

SIDE EFFECTS:
  None.
=============================================================================*/
aviErrorType aviParser::GetSampleInfo(avi_uint64* sampleOffset,
                                      avi_uint32 trackId,
                                      avi_sample_info* sampleInfo,
                                      avi_uint32 nMaxBufSize,
                                      avi_uint16* trackIdFound)
{
  unsigned char* byteData = m_ReadBuffer;
  bool bSampleOffsetAdjusted = false;
  avi_uint32 nSize = 0;
  avi_uint16 cType = AVI_CHUNK_UNKNOWN;
  aviErrorType retError = AVI_FAILURE;
  CHUNK_t chkType;
  *trackIdFound = 0;
  avi_uint32 nSizeToBeAdjusted = 0;
  bool bEndTrack = false;

  //Since this function can be used to either get next sample during playback or
  //calculate buffered duration during streaming, using these temp variables to calculate TS of audio and video,
  //we will update the class variables only if this function is called from GetNextSampleInfo

  bool bUpdateSamples = (m_CurrentParserState == AVI_PARSER_UPDATE_SAMPLES)? 1:0;

  avi_uint32  tempCurrAudioSampleInIdx1 = m_nCurrAudioSampleInIdx1;
  avi_uint32  tempCurrVideoSampleInIdx1 = m_nCurrVideoSampleInIdx1;
  avi_uint64  tempCurrAudioPayloadSize[AVI_MAX_AUDIO_TRACKS] ={0};
  avi_uint32  tempCurrAudioFrameCount[AVI_MAX_AUDIO_TRACKS] = {0};
  avi_uint64  tempParserAudSampleEndTime[AVI_MAX_AUDIO_TRACKS] = {0};


  for(int k = 0; k < AVI_MAX_AUDIO_TRACKS;k++)
  {
    memcpy(&tempCurrAudioPayloadSize[k],&m_nCurrAudioPayloadSize[k],sizeof(avi_uint64));
    memcpy(&tempCurrAudioFrameCount[k],&m_nCurrAudioFrameCount[k],sizeof(avi_uint32));
    memcpy(&tempParserAudSampleEndTime[k],&m_nParserAudSampleEndTime[k],sizeof(avi_uint64));
  }

  avi_uint32  tempCurrVideoFrameCount[AVI_MAX_VIDEO_TRACKS];

  for(int i = 0; i < AVI_MAX_VIDEO_TRACKS;i++)
  {
    memcpy(&tempCurrVideoFrameCount[i],&m_nCurrVideoFrameCount[i],sizeof(avi_uint32));
  }

  if(trackId < m_hAviSummary.n_streams)
  {
    while(1)
    {
      m_CurrentParserState = AVI_PARSER_CHUNK_HEADER_START;

      fourCC_t fourCCVal = 0;
      if(!parserAVICallbakGetData(*sampleOffset,sizeof(fourCC_t),
        m_ReadBuffer,AVI_READ_BUFFER_SIZE,m_pUserData,&retError))
      {
        break;
      }
      memcpy(&fourCCVal,byteData,sizeof(fourCC_t));
      *sampleOffset+=sizeof(fourCC_t);

      memcpy(trackIdFound,(avi_uint8*)&fourCCVal,sizeof(avi_uint16));
      *trackIdFound = ascii_2_short_int(trackIdFound);

      memcpy(&cType,((avi_uint8*)(&fourCCVal))+2,sizeof(avi_uint16));

      if(!parserAVICallbakGetData(*sampleOffset,sizeof(avi_uint32),m_ReadBuffer,AVI_READ_BUFFER_SIZE,m_pUserData,&retError))
      {
        break;
      }
      memcpy(&nSize,byteData,sizeof(avi_uint32));

      /* We will end track based on trackDuration only for AUDIO since it is of higher priority
      But once we go over the IDX1Offset,we should end track for both AUDIO and VIDEO */
      if( ( (getCurrentPlaybackTime(trackId) >= GetTrackDuration(trackId) )&&
            (GetTrackDuration(trackId) > 0) &&
            (GetTrackChunkType(trackId,&chkType) == AVI_SUCCESS) &&
            (chkType == AVI_CHUNK_AUDIO) ) ||
            ( (*sampleOffset >= m_nIdx1Offset) && (!m_bisAVIXpresent) && (m_nIdx1Offset) ) ||
            (*sampleOffset >= m_nFileSize) ||( (!cType) &&(!nSize)) )
      {
        //Report eof if cType & nSize reported as '0'. This is
        //possible in case of corrupted clips.
        bEndTrack = true;
        MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
                  "GetNextSampleInfo setting bEndTrack TRUE for trackid %d",trackId);
        break;
      }

      if( (!memcmp(&fourCCVal,AVI_JUNK_FOURCC,sizeof(fourCC_t))) ||
          (!memcmp(&fourCCVal,AVI_RES_FOURCC,3*sizeof(char)))  ||
          (!memcmp(&fourCCVal,AVI_IX_FOURCC,2*sizeof(char))) ||
          (!memcmp(&fourCCVal,AVI_LIST_FOURCC,sizeof(fourCC_t))) ||
          ((!memcmp(&fourCCVal,AVI_IDX1_FOURCC,sizeof(fourCC_t))) && (m_bisAVIXpresent) ) )
      {
        bool bChunkOk = false;
        /* Adding support to parse REC chunk. To skip other chunk types which can occur in
           as part of 'LIST' checking whether chunk is REC or not. After "rec ", track type and trackId
           are available. Reading that part of data as well, to check whether to parse inside REC or to
           skip the chunk. */
        if(!memcmp(&fourCCVal,AVI_LIST_FOURCC,sizeof(fourCC_t)))
        {
          if(!parserAVICallbakGetData(*sampleOffset + 4,
            2*sizeof(fourCC_t),m_ReadBuffer,AVI_READ_BUFFER_SIZE,
            m_pUserData,&retError))
          {
            break;
          }
          memcpy(&fourCCVal,byteData,sizeof(fourCC_t));
          memcpy(trackIdFound,byteData+4,sizeof(avi_uint16));
          *trackIdFound = ascii_2_short_int(trackIdFound);
          memcpy(&cType,byteData+6,sizeof(avi_uint16));

          if(!memcmp(&fourCCVal,AVI_REC_FOURCC,sizeof(fourCC_t)))
          {
            /* Skip the REC if the trackID is not matching with the trackID requested.*/
            if( ((!memcmp(&cType,"dc",sizeof(avi_uint16))) ||
                (!memcmp(&cType,"db",sizeof(avi_uint16))) ||
                (!memcmp(&cType,"wb",sizeof(avi_uint16))) ||
                (!memcmp(&cType,"dd",sizeof(avi_uint16))) ) )
            {
              bChunkOk = true;
              *sampleOffset+=8;
            }
          }
        }
        if(false == bChunkOk)
        {
          *sampleOffset+=sizeof(avi_uint32);
          sampleInfo->nSampleSize = nSize;
          if (sampleInfo->nSampleSize % 2)
          {
            // must align to 16-bit boundary
            sampleInfo->nSampleSize++;
          }
          *sampleOffset+=sampleInfo->nSampleSize;
          if(bUpdateSamples)
            m_nCurrentSampleInfoOffsetInIdx1 += ( 4 * sizeof(avi_uint32) );
        }
      }
      else if (
         (memcmp(&cType,"dc",sizeof(avi_uint16))) &&
         (memcmp(&cType,"db",sizeof(avi_uint16))) &&
         (memcmp(&cType,"wb",sizeof(avi_uint16))) &&
         (memcmp(&cType,"dd",sizeof(avi_uint16))) &&
         (memcmp(&cType,"sb",sizeof(avi_uint16))) &&
         ((nSize) && (*sampleOffset+nSize <= m_nFileSize) &&
         (memcmp(&fourCCVal,"RIFF",sizeof(fourCC_t))) )
         )
      {
        *sampleOffset+=sizeof(avi_uint32);
        sampleInfo->nSampleSize = nSize;
        if (sampleInfo->nSampleSize % 2)
        {
          // must align to 16-bit boundary
          sampleInfo->nSampleSize++;
        }
        *sampleOffset+=sampleInfo->nSampleSize;
        continue;
      }
      else if(!memcmp(&fourCCVal,"RIFF",sizeof(fourCC_t)) )
      {
        if(m_nNumOfRiff)
        {
          for(int i = 0;i<m_nNumOfRiff;i++)
          {
            //Check which RIFF atom is immediately next from the current offset
            if( (*sampleOffset + sizeof(avi_uint32) ) == m_pMultipleRiff[i].startOffset)
            {
              //Skip AVIX/LIST/MOVI/SIZE chunks till we start getting audio/video frames again
              *sampleOffset = m_pMultipleRiff[i].startOffset + (4*sizeof(avi_uint32));
            }
          }
        }
        else
        {
          *sampleOffset+=sizeof(avi_uint32);
          sampleInfo->nSampleSize = nSize;
          if (sampleInfo->nSampleSize % 2)
          {
            // must align to 16-bit boundary
            sampleInfo->nSampleSize++;
          }
          *sampleOffset+=sampleInfo->nSampleSize;
          continue;
        }
      }
      else
      {

        *sampleOffset+=sizeof(avi_uint32);

        sampleInfo->nSampleSize = nSize;

        memcpy(&cType,((avi_uint8*)(&fourCCVal))+2,sizeof(avi_uint16));

        /*
        * Go to next entry in IDX1 as we have read in the sample header.
        * Since IDX1 has all the samples from MOVI, number of samples
        * skipped here should match the current retrieved audio/video sample.
        * Although we are interested only in audio/video,
        * we need to do it here otherwise, we will have to do file read to
        * bypass non audio/video samples. This makes audio playback choppy.
        */

        if(bUpdateSamples)
          m_nCurrentSampleInfoOffsetInIdx1 += ( 4 * sizeof(avi_uint32) );

        /*
        * If we encounter any chunk which is not expected or not known,
        * we should just continue parsing by skipping such un-known/unexpected chunks.
        */
        bool bChunkOk = false;


        if( (!memcmp(&cType,"dc",sizeof(avi_uint16))) ||
              (!memcmp(&cType,"db",sizeof(avi_uint16))) ||
              (!memcmp(&cType,"wb",sizeof(avi_uint16))) ||
              (!memcmp(&cType,"dd",sizeof(avi_uint16))) ||
              (!memcmp(&cType,"sb",sizeof(avi_uint16))) )
        {
          bChunkOk = true;
        }

        if( (*trackIdFound != trackId) || (bChunkOk == false) || GetTotalNumberOfTracks() < (*trackIdFound))
        {
          /*
          * Any unknown chunk might yield invalid track-id as it may not have track-id in it.
          * Thus, if we have found some media chunk(audio/video/caption/drm) and if track id is >
          * total number of tracks, flag an error and abort.
          */
          if( GetTotalNumberOfTracks() < (*trackIdFound) && (bChunkOk) )
          {
            /*
            *Something is wrong, either seek resulted in wrong offset,
            *parser screwed up during playback or somebode else
            *might have corrupted the memory.
            */
             MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_FATAL,
                          "getNextSampleInfo *trackIdFound %d > total number of tracks %d ",(*trackIdFound),GetTotalNumberOfTracks());
             *sampleOffset += sizeof(avi_uint32);
             continue;
          }
          if(memcmp(&cType,"dd",sizeof(avi_uint16))) //Making sure its not a DRM chunk
          {
            //Skip un-known chunk and continue parsing
            if (sampleInfo->nSampleSize % 2)
            {
              // must align to 16-bit boundary
              sampleInfo->nSampleSize++;
            }
            *sampleOffset+=sampleInfo->nSampleSize;
            continue;
          }
        }

       if( (!memcmp(&cType,"dc",sizeof(avi_uint16))) ||
            (!memcmp(&cType,"db",sizeof(avi_uint16)))   )
        {
          sampleInfo->chunkType = AVI_CHUNK_VIDEO;
          if (nSize>nMaxBufSize)
          {
            *sampleOffset -= sizeof(fourCC_t);
            *sampleOffset -= sizeof(avi_uint32);
            if(bUpdateSamples)
              m_nCurrentSampleInfoOffsetInIdx1 -= ( 4 * sizeof(avi_uint32) );
            m_nLastOffsetRead = *sampleOffset;
            return AVI_INSUFFICIENT_BUFFER;
          }
        }
       if(!memcmp(&cType,"sb",sizeof(avi_uint16)))
        {
          sampleInfo->chunkType = AVI_CHUNK_BITMAP_CAPTION;
          if (nSize>nMaxBufSize)
          {
            *sampleOffset -= sizeof(fourCC_t);
            *sampleOffset -= sizeof(avi_uint32);
            if(bUpdateSamples)
              m_nCurrentSampleInfoOffsetInIdx1 -= ( 4 * sizeof(avi_uint32) );
            m_nLastOffsetRead = *sampleOffset;
            return AVI_INSUFFICIENT_BUFFER;
          }
        }
        if(!memcmp(&cType,"wb",sizeof(avi_uint16)))
        {
          sampleInfo->chunkType = AVI_CHUNK_AUDIO;
          if (nSize>nMaxBufSize)
          {
            *sampleOffset -= sizeof(fourCC_t);
            *sampleOffset -= sizeof(avi_uint32);
            if(bUpdateSamples)
              m_nCurrentSampleInfoOffsetInIdx1 -= ( 4 * sizeof(avi_uint32) );
            m_nLastOffsetRead = *sampleOffset;
            return AVI_INSUFFICIENT_BUFFER;
          }
        }//if(!memcmp(&cType,"wb",sizeof(avi_uint16)))
        if(!memcmp(&cType,"dd",sizeof(avi_uint16)))
        {
          sampleInfo->chunkType = AVI_CHUNK_DRM;
          if (nSize>nMaxBufSize)
          {
            *sampleOffset -= sizeof(fourCC_t);
            *sampleOffset -= sizeof(avi_uint32);
            if(bUpdateSamples)
              m_nCurrentSampleInfoOffsetInIdx1 -= ( 4 * sizeof(avi_uint32) );
            m_nLastOffsetRead = *sampleOffset;
            return AVI_INSUFFICIENT_BUFFER;
          }
          retError = AVI_SUCCESS;
          if(bUpdateSamples)
            m_nCurrentChunkDataSize = sampleInfo->nSampleSize;
          m_CurrentParserState = AVI_PARSER_CHUNK_DATA_START;
          return retError;
        }//if(!memcmp(&cType,"dd",sizeof(avi_uint16)))

        if(GetTrackChunkType(*trackIdFound,&chkType)==AVI_SUCCESS)
        {
          avi_int64 endTime = 0;
          switch(chkType)
          {
            case AVI_CHUNK_BITMAP_CAPTION:
              if(!parserAVICallbakGetData(*sampleOffset,
                AVI_SUBTITLE_HDR_SIZE,m_ReadBuffer,
                AVI_READ_BUFFER_SIZE,m_pUserData,&retError))
              {
                break;
              }
              sampleInfo->nTimeStamp = getTimeStampFromSubtitle(byteData, AVI_CHUNK_BITMAP_CAPTION);
              endTime = getTimeStampFromSubtitle(byteData + 13, AVI_CHUNK_BITMAP_CAPTION);
              sampleInfo->nDuration = (avi_uint32)(endTime - sampleInfo->nTimeStamp);
              break;
            case AVI_CHUNK_AUDIO:
              if(*trackIdFound < AVI_MAX_AUDIO_TRACKS)
              {
              avi_audiotrack_summary_info ainfo;
              avi_audio_info audinfo;
              if( bSampleOffsetAdjusted == true)
              {
                //Since we went back by a sample,adjust IDX1 sample count,
                //Retrieved sample count and bytes count.
                if( tempCurrAudioSampleInIdx1 > 0 )
                {
                  tempCurrAudioSampleInIdx1--;
                }
                if(tempCurrAudioPayloadSize[*trackIdFound] > nSizeToBeAdjusted)
                {
                  tempCurrAudioPayloadSize[*trackIdFound]-=nSizeToBeAdjusted;
                }
                if(tempCurrAudioFrameCount[*trackIdFound] > 0)
                {
                  tempCurrAudioFrameCount[*trackIdFound]--;
                }
                bSampleOffsetAdjusted = false;
                nSizeToBeAdjusted = 0;
              }
              tempCurrAudioSampleInIdx1++;
#ifdef FEATURE_FILESOURCE_AVI_PARSER_DEBUG
                MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
                              "Audio->GetNextSampleInfo->tempCurrAudioSampleInIdx1 %ld",
                              tempCurrAudioSampleInIdx1);
#endif

              if(GetAudioTrackSummaryInfo(*trackIdFound,&ainfo)==AVI_SUCCESS)
              {
                if( (!ainfo.isVbr)&&(ainfo.audioBytesPerSec) )
                {
                  sampleInfo->nTimeStamp = (avi_uint64)((float)tempCurrAudioPayloadSize[*trackIdFound] /
                  (float)ainfo.audioBytesPerSec * 1000.0f);

                   sampleInfo->nDuration = (avi_uint32)((avi_uint64)((float)(tempCurrAudioPayloadSize[*trackIdFound]+sampleInfo->nSampleSize) /

                  (float)ainfo.audioBytesPerSec * 1000.0f) - sampleInfo->nTimeStamp);

                   tempParserAudSampleEndTime[*trackIdFound] = sampleInfo->nTimeStamp+sampleInfo->nDuration;

                   MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_MEDIUM,
                                 "Audio->GetNextSampleInfo->CBR:nTimeStamp %llu nDuration %u",
                                 sampleInfo->nTimeStamp,sampleInfo->nDuration);
                }
                else if(ainfo.isVbr &&
                        ainfo.audioFrequency &&
                        (GetAudioInfo(*trackIdFound,&audinfo)== AVI_SUCCESS) )
                {
                  if(audinfo.strhAudio.dwSampleSize > 0 )
                  {
                    //Multiple samples will be grouped together in single chunk
                    sampleInfo->nTimeStamp = tempParserAudSampleEndTime[*trackIdFound];
                    double val1 = ceil( ( (double)sampleInfo->nSampleSize / ainfo.nBlockAlign) );
                    if(audinfo.strhAudio.dwRate)
                    {
                      double val2 =  ((double)audinfo.strhAudio.dwScale/audinfo.strhAudio.dwRate)* 1000.f;
                      sampleInfo->nDuration =  (uint32)(val1 * val2);
                      tempParserAudSampleEndTime[*trackIdFound] = sampleInfo->nTimeStamp+sampleInfo->nDuration;
                    }
                    else
                    {
                      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM, "audinfo.strhAudio.dwRate is ZERO");
                    }
                  }
                  else
                  {
                    if(audinfo.strhAudio.dwRate)
                    {
                      sampleInfo->nTimeStamp = (avi_uint64)(( ( (float)audinfo.strhAudio.dwScale/
                                              (float)audinfo.strhAudio.dwRate) * 1000.f) * (float)tempCurrAudioFrameCount[*trackIdFound]);
                      avi_uint64 nextAudioTS = (avi_uint64)(( ( (float)audinfo.strhAudio.dwScale/
                                              (float)audinfo.strhAudio.dwRate) * 1000.f) * (float)(tempCurrAudioFrameCount[*trackIdFound]+1) );
                      sampleInfo->nDuration = (avi_uint32) (nextAudioTS - sampleInfo->nTimeStamp);
                    }
                    else
                    {
                      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM, "audinfo.strhAudio.dwRate is ZERO");
                    }
                  }
                  MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_MEDIUM, "Audio->GetNextSampleInfo->VBR:nTimeStamp %llu nDuration %u",
                                   sampleInfo->nTimeStamp,sampleInfo->nDuration);
                }
              }
              tempCurrAudioPayloadSize[*trackIdFound]+=sampleInfo->nSampleSize;
              tempCurrAudioFrameCount[*trackIdFound]++;
#ifdef FEATURE_FILESOURCE_AVI_PARSER_DEBUG
              MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_MEDIUM,
                           "Audio->GetNextSampleInfo->tempCurrAudioPayloadSize[*trackIdFound] %d tempCurrAudioFrameCount[*trackIdFound] %ld",
                           tempCurrAudioPayloadSize[*trackIdFound],tempCurrAudioFrameCount[*trackIdFound]);
#endif

                if( sampleInfo->nSampleSize == 0 )
                {
                  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM, "Audio->GetNextSampleInfo encountered zero size sample");
                  continue;
                }
              }
              break;
            case AVI_CHUNK_VIDEO:
              avi_video_info vinfo;

              if(*trackIdFound < AVI_MAX_VIDEO_TRACKS)
              {
                if( bSampleOffsetAdjusted == true)
                {
                  //Since we went back by a sample,adjust IDX1 sample count,
                  //Retrieved sample count and bytes count.
                  if( tempCurrVideoSampleInIdx1 > 0 )
                  {
                    tempCurrVideoSampleInIdx1--;
                  }
                  if(tempCurrVideoFrameCount[*trackIdFound] > 0)
                  {
                    tempCurrVideoFrameCount[*trackIdFound]--;
                  }
                  bSampleOffsetAdjusted = false;
                  nSizeToBeAdjusted = 0;
                }
                tempCurrVideoSampleInIdx1++;
#ifdef FEATURE_FILESOURCE_AVI_PARSER_DEBUG
                  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
                                "Video->GetNextSampleInfo->tempCurrVideoSampleInIdx1 %d",
                                tempCurrVideoSampleInIdx1);
#endif

                if( (GetVideoInfo(*trackIdFound,&vinfo)==AVI_SUCCESS) && (vinfo.strhVideo.dwRate) )
                {
                  sampleInfo->nTimeStamp =
                    (avi_uint64)( ( (float)vinfo.strhVideo.dwScale/(float)vinfo.strhVideo.dwRate)*
                                  (float)tempCurrVideoFrameCount[*trackIdFound]*1000.0f);
                  sampleInfo->nDuration =
                    (avi_uint32) ( ( ((float)vinfo.strhVideo.dwScale / (float)vinfo.strhVideo.dwRate) ) * 1000.0f );
                  //#ifdef FEATURE_FILESOURCE_AVI_PARSER_DEBUG
                  MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_MEDIUM,
                                "Video->GetNextSampleInfo:nTimeStamp %llu nDuration %u",
                                sampleInfo->nTimeStamp,sampleInfo->nDuration);
                  //#endif
                }
                tempCurrVideoFrameCount[*trackIdFound]++;
#ifdef FEATURE_FILESOURCE_AVI_PARSER_DEBUG
                  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
                                "Video->GetNextSampleInfo->tempCurrVideoFrameCount[*trackIdFound] %d",
                                tempCurrVideoFrameCount[*trackIdFound]);
#endif
                if(sampleInfo->nSampleSize < 4)
                {
                  /* We encountered clips with video frames of size <4bytes which were leading to corrupted video,
                     so we will skip any frames which is lesser than start code size. */
                  if(sampleInfo->nSampleSize)
                    *sampleOffset+=(sampleInfo->nSampleSize +(sampleInfo->nSampleSize % 2));
                  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM, "Video->GetNextSampleInfo encountered <4 bytes size sample");
                  continue;
                }
              }
              //(void)updateSampleRetrievedOffset(AVI_CHUNK_VIDEO,*trackIdFound);
              break;
            default:
              break;
          }
        }//if(GetTrackChunkType(*trackIdFound,&chkType)==AVI_SUCCESS)

        if(bUpdateSamples )
        {
          m_nCurrentChunkDataSize = ( (sampleInfo->nSampleSize) +((sampleInfo->nSampleSize)%2));
          m_nCurrAudioSampleInIdx1 = tempCurrAudioSampleInIdx1;
          m_nCurrVideoSampleInIdx1 = tempCurrVideoSampleInIdx1;
          if ( *trackIdFound < AVI_MAX_AUDIO_TRACKS )
          {
            m_nCurrAudioPayloadSize[*trackIdFound] = tempCurrAudioPayloadSize[*trackIdFound];
            m_nCurrAudioFrameCount[*trackIdFound] = tempCurrAudioFrameCount[*trackIdFound];
            m_nParserAudSampleEndTime[*trackIdFound] = tempParserAudSampleEndTime[*trackIdFound];
          }
          /* trackIdFound should always be '1' less than max tracks */
          if (*trackIdFound < AVI_MAX_VIDEO_TRACKS )
          {
            m_nCurrVideoFrameCount[*trackIdFound] = tempCurrVideoFrameCount[*trackIdFound];
          }
        }//if(bUpdateSamples )
        m_nLastOffsetRead = *sampleOffset + m_nCurrentChunkDataSize;
        m_nSampleInfoOffset = *sampleOffset + m_nCurrentChunkDataSize;
        m_CurrentParserState = AVI_PARSER_CHUNK_DATA_START;
        retError = AVI_SUCCESS;
        break;
      }
    }//while(1)
    if( (m_nFileSize && (*sampleOffset >= m_nFileSize) ) || (bEndTrack == true) )
    {
      //check if we are done reading 'MOVI'
      #ifdef FEATURE_FILESOURCE_AVI_PARSER_DEBUG
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,
                  "GetSampleInfo AVI_END_OF_FILE");
      #endif
      /* Set Parser state as EOF when EOF is reached */
      setParserState(AVI_PARSER_END_OF_FILE,&retError);
      return AVI_END_OF_FILE;
    }
  }
  else
  {
    retError = AVI_INVALID_USER_DATA;
    MM_MSG_PRIO(MM_FILE_OPS,MM_PRIO_FATAL, "GetSampleInfo AVI_INVALID_USER_DATA");
  }
  return retError;
}
/* =============================================================================
FUNCTION:
 aviParser::GetTrackWholeIDList

DESCRIPTION:
Returns trackId list for all the tracks in given clip.

INPUT/OUTPUT PARAMETERS:

RETURN VALUE:

SIDE EFFECTS:
  None.
=============================================================================*/
avi_uint32 aviParser::GetTrackWholeIDList(avi_uint32* idList)
{
  if(!idList)
  {
    return 0;
  }
  for (avi_uint32 i = 0; i < m_hAviSummary.n_streams; i++)
  {
    (*idList) = m_hAviSummary.stream_index[i].index;
    idList++;
  }
  return m_hAviSummary.n_streams;
}
#ifdef AVI_PARSER_SEEK_SANITY_TEST
/* =============================================================================
FUNCTION:
 aviParser::doSanityCheckBeforeSeek

DESCRIPTION:
 To be used only for debugging purpose.
 Does quick sanity testing by comparing
 audio/video sample count retrieved so far against audio/video sample count
 in IDX1. Also compares the offset being used by parser against
 sample offset in IDX1.

INPUT/OUTPUT PARAMETERS:

RETURN VALUE:

SIDE EFFECTS:
  None.
=============================================================================*/
bool aviParser::doSanityCheckBeforeSeek(avi_uint32 tid,CHUNK_t tracktype,aviParserState state)
{
  MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_MEDIUM,
                   "doSanityCheckBeforeSeek tid %d tracktype %d",tid,tracktype);
  bool bok = true;
  unsigned char* byteData = m_ReadBuffer;
  avi_uint16 cType = 0;
  avi_uint16 trackId = 0;
  avi_uint32 dwFlags = 0;
  avi_uint64 dwOffset = 0;
  avi_uint32 dwSize = 0;
  aviErrorType retError = AVI_SUCCESS;

  //This read should always be xxwb,xxdc for audio/video.
  if(!parserAVICallbakGetData(m_nCurrentSampleInfoOffsetInIdx1,
    4 * sizeof(avi_uint32),m_ReadBuffer,AVI_READ_BUFFER_SIZE
    ,m_pUserData,&retError))
  {
    return false;
  }
  memcpy(&trackId,byteData,sizeof(avi_uint16));
  trackId = ascii_2_short_int(&trackId);
  memcpy(&cType,byteData+2,sizeof(avi_uint16));
  memcpy(&dwFlags,byteData+sizeof(avi_uint32),sizeof(avi_uint32));
  memcpy(&dwOffset,byteData+(2*sizeof(avi_uint32)),sizeof(avi_uint32));
  if(m_bByteAdjustedForMOVI)
  {
    dwOffset = (avi_uint64)(m_nStartOfMovi + dwOffset - m_nBytesToBeAdjustedForMOVI);
  }
  memcpy(&dwSize,byteData+(3*sizeof(avi_uint32)),sizeof(avi_uint32));

  //State and Offset/chunk size Sanity.
  if(state == AVI_PARSER_CHUNK_DATA_START)
  {
    if(m_nCurrentChunkDataSize != dwSize)
    {
      MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_MEDIUM,
                   "doSanityCheckBeforeSeek dwSize %d did not match m_nCurrentChunkDataSize %d",
                   dwSize,m_nCurrentChunkDataSize);
      bok = false;
    }
    if( m_nCurrOffset != (dwOffset - AVI_SIZEOF_FOURCC_PLUS_LENGTH) )
    {
      MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_MEDIUM,
                   "doSanityCheckBeforeSeek m_nCurrOffset %llu did not match dwOffset - AVI_SIZEOF_FOURCC_PLUS_LENGTH %llu",
                   m_nCurrOffset,(dwOffset - AVI_SIZEOF_FOURCC_PLUS_LENGTH));
      bok = false;
    }
  }
  else if(state == AVI_PARSER_CHUNK_HEADER_START)
  {
    if(m_nCurrOffset != dwOffset)
    {
      MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_MEDIUM,
                   "doSanityCheckBeforeSeek m_nCurrOffset %llu did not match dwOffset %llu",
                   m_nCurrOffset,dwOffset);
      bok = false;
    }
  }
  else
  {
    //Parser is in BAD STATE
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
                   "doSanityCheckBeforeSeek m_CurrentParserState in wrong state %d",
                   state);
    bok = false;
  }

  //Sample Count Sanity
  if(tracktype == AVI_CHUNK_AUDIO)
  {
    if(m_nCurrAudioSampleInIdx1 != m_nCurrAudioFrameCount[tid])
    {
       MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,
                   "doSanityCheckBeforeSeek AUDIO m_nCurrAudioFrameCount did not match!!!");
       bok = false;
    }
  }
  else if(tracktype == AVI_CHUNK_VIDEO)
  {
    if(m_nCurrVideoSampleInIdx1 != m_nCurrVideoFrameCount[tid])
    {
       MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,
                   "doSanityCheckBeforeSeek VIDEO m_nCurrVideoFrameCount did not match!!!");
        bok = false;
    }
  }
  if(bok)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,"doSanityCheckBeforeSeek Successful!!");
  }
  else
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,"PLEASE CHECK as doSanityCheckBeforeSeek Failed..");
  }
  return bok;
}
#endif
/* =============================================================================
FUNCTION:
 aviParser::getCurrentPlaybackTime

DESCRIPTION:
Returns the current playback time for given trackid.

INPUT/OUTPUT PARAMETERS:

RETURN VALUE:
Current Playback time in msec.

SIDE EFFECTS:
  None.
=============================================================================*/
avi_uint64 aviParser::getCurrentPlaybackTime(avi_uint32 trackid)
{
  avi_audiotrack_summary_info ainfo;
  avi_video_info vinfo;
  avi_uint64 nCurrentTime = 0;
  CHUNK_t cType;

  if(GetTrackChunkType(trackid,&cType)==AVI_SUCCESS)
  {
    switch(cType)
    {
      case AVI_CHUNK_AUDIO:
        if(GetAudioTrackSummaryInfo(trackid,&ainfo)==AVI_SUCCESS)
        {
          if( (!ainfo.isVbr)&&(ainfo.audioBytesPerSec) )
          {
            nCurrentTime = (avi_int64)((float)m_nCurrAudioPayloadSize[trackid] /
                                       (float)ainfo.audioBytesPerSec * 1000.0f);
          }
          else if(ainfo.isVbr && ainfo.audioFrequency)
          {
            avi_audio_info audinfo;
            if(GetAudioInfo(trackid,&audinfo) == AVI_SUCCESS)
            {
              if(audinfo.strhAudio.dwSampleSize > 0)
              {
                nCurrentTime =  m_nParserAudSampleEndTime[trackid];
              }
              else
              {
                nCurrentTime = (avi_uint64)(( ( (float)audinfo.strhAudio.dwScale/
                (float)audinfo.strhAudio.dwRate) * 1000.f) * (float)m_nCurrAudioFrameCount[trackid] );
              }
            }
          }//else if(ainfo.isVbr && ainfo.audioFrequency)
        }
        break;
      case AVI_CHUNK_VIDEO:
        if( (GetVideoInfo(trackid,&vinfo)==AVI_SUCCESS) && (vinfo.strhVideo.dwRate) )
        {
          nCurrentTime = (avi_int64)( ( (float)vinfo.strhVideo.dwScale/(float)vinfo.strhVideo.dwRate)*
                                        (float)m_nCurrVideoFrameCount[trackid]*1000.0f);
        }
        break;
      default:
        break;
    }
  }
  return nCurrentTime;
}
/* =============================================================================
FUNCTION:
 aviParser::seekInSuperIndex

DESCRIPTION:
 Seek the track identified by given trackid through INDX.

INPUT/OUTPUT PARAMETERS:
 trackid: Identifies the track to be repositioned.
 nReposTime:Target time to seek to
 nCurrPlayTime: Current playback time
 sample_info: Sample Info to be filled in if seek is successful
 canSyncToNonKeyFrame: When true, video can be repositioned to non key frame
 nSyncFramesToSkip:Number of sync samples to skip. Valid only if > or < 0.

RETURN VALUE:
 AVI_SUCCESS if successful otherwise returns appropriate error code.

SIDE EFFECTS:
  None.
=============================================================================*/
aviErrorType  aviParser::seekInSuperIndex(avi_uint32 ulTrackID,
                                   avi_uint64 ullReposTime,
                                   avi_idx1_entry* outputEntry,
                                   bool bSearchForward,
                                   CHUNK_t chunkType,
                                   int  nSyncFramesToSkip,
                                   bool* endOfFileReached,
                                   bool bSyncToKeyFrame)
{
  aviErrorType retError = AVI_UNKNOWN_ERROR;
  avi_audio_info ainfo;
  avi_video_info vinfo;
  avi_audiotrack_summary_info audioTrackInfo;

  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,"aviParser::seekInSuperIndex");

  if(chunkType == AVI_CHUNK_AUDIO)
  {
    if( (GetAudioInfo(ulTrackID,&ainfo) == AVI_SUCCESS) && (ainfo.strhAudio.dwRate) )
    {
      if(GetAudioTrackSummaryInfo(ulTrackID,&audioTrackInfo)== AVI_SUCCESS)
      {
        if( (AVI_INDEX_OF_INDEXES == m_base_indx_tbl[ulTrackID].bIndexType) &&
            m_base_indx_tbl[ulTrackID].pIndxSuperIndexEntry)
        {
          uint64 ullAudioTS     = 0;
          uint64 ullPrevFrameTS = 0;
          uint64 ullAudioBytesFromStart   = 0;
          uint32 ulPrevIndxChunkIndxEntry = 0;
          uint32 ulEntriesSkipped = 0;
          bool   bValidTimeStamp  = false;
          for(avi_uint32 ulSuperIndex = 0; ulSuperIndex <
              m_base_indx_tbl[ulTrackID].nEntriesInUse ; ulSuperIndex++)
          {
            if (bValidTimeStamp)
            {
              MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
                "seekInSuperIndex valid entry found in previous chunk %lu", ulSuperIndex - 1);
              break;
            }
            else if(m_base_indx_tbl[ulTrackID].pIXIndexChunk)
            {
              if (!m_base_indx_tbl[ulTrackID].pIXIndexChunk[ulSuperIndex].pIndxChunkIndexEntry)
              {
                retError = AVI_UNKNOWN_ERROR;
                break;
              }
              for(avi_uint32 ulStdIndex = 0 ; ulStdIndex <
                  m_base_indx_tbl[ulTrackID].pIXIndexChunk[ulSuperIndex].nEntriesInUse;
                  ulStdIndex++ )
              {
                if(audioTrackInfo.isVbr)
                {
                  ullAudioTS = ((uint64)ainfo.strhAudio.dwScale *  ulEntriesSkipped *
                                MILLISEC_TIMESCALE_UNIT) / ainfo.strhAudio.dwRate;
                }
                else if(audioTrackInfo.audioBytesPerSec)
                {
                  ullAudioTS = ((uint64)ullAudioBytesFromStart *MILLISEC_TIMESCALE_UNIT)
                                / audioTrackInfo.audioBytesPerSec ;
                }
                if((0!= ullPrevFrameTS)
                   && (ullReposTime >= ullPrevFrameTS)
                   && (ullAudioTS >= ullReposTime))
                {
                  // Declaring this pointer only to make code more readable, no other use.
                  avi_indx_chunk_index_entry* pCurrIndxEntry =
                    m_base_indx_tbl[ulTrackID].pIXIndexChunk[ulSuperIndex].pIndxChunkIndexEntry;
                  outputEntry->trackId = ulTrackID;
                  outputEntry->chunkType = AVI_CHUNK_AUDIO;
                  // Take data for next frame if seeking forward,
                  // !ulSuperIndex && !ulStdIndex is just defensive check for
                  // boundary cases
                  if((bSearchForward) || (!ulSuperIndex && !ulStdIndex))
                  {
                    outputEntry->dwOffset = pCurrIndxEntry[ulStdIndex].dwOffset;
                    outputEntry->dwSize = pCurrIndxEntry[ulStdIndex].dwSize;
                    outputEntry->nTimeStamp = ullAudioTS;
                    outputEntry->nTotalSizeInBytes = ullAudioBytesFromStart;
                    outputEntry->dwAudFrameCount = ulEntriesSkipped;
                  }
                  // Take data for previous frame if seeking backward,
                  else
                  {
                    //Handle boundary case, if ulStdIndex == 0
                    //then pIndxChunkIndexEntry[ulStdIndex-1] will be not valid
                    if (0 == ulStdIndex)
                    {
                      outputEntry->nTotalSizeInBytes = m_base_indx_tbl[ulTrackID].pIXIndexChunk[ulSuperIndex-1].pIndxChunkIndexEntry[ulPrevIndxChunkIndxEntry].dwSize;
                      outputEntry->dwOffset = m_base_indx_tbl[ulTrackID].pIXIndexChunk[ulSuperIndex-1].pIndxChunkIndexEntry[ulPrevIndxChunkIndxEntry].dwOffset;
                      outputEntry->dwSize = m_base_indx_tbl[ulTrackID].pIXIndexChunk[ulSuperIndex-1].pIndxChunkIndexEntry[ulPrevIndxChunkIndxEntry].dwSize;
                    }
                    else
                    {
                      outputEntry->nTotalSizeInBytes = ullAudioBytesFromStart;
                      outputEntry->dwOffset = pCurrIndxEntry[ulStdIndex-1].dwOffset;
                      outputEntry->dwSize = pCurrIndxEntry[ulStdIndex-1].dwSize;
                    }
                    outputEntry->nTimeStamp =  ullPrevFrameTS;
                    outputEntry->dwAudFrameCount = ulEntriesSkipped -1;
                  }
                  bValidTimeStamp = true;
                  retError = AVI_SUCCESS;
                    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
                        "SeekInSuperIndex matched Audio ts %llu",
                        ullAudioTS);
                  MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_MEDIUM,
                      "SeekInSuperIndex matched at super index %u, std index %u",
                      ulSuperIndex, ulStdIndex);
                  break;
                }
                ullPrevFrameTS = ullAudioTS;
                ullAudioBytesFromStart += m_base_indx_tbl[ulTrackID].pIXIndexChunk[ulSuperIndex].pIndxChunkIndexEntry[ulStdIndex].dwSize;
                ulEntriesSkipped++;
                ulPrevIndxChunkIndxEntry = ulStdIndex;
              }
            }
          }
        }
      }//if(GetAudioTrackSummaryInfo(ulTrackID,&audioTrackInfo)== AVI_SUCCESS)
    }//if( (GetAudioInfo(ulTrackID,&ainfo) == AVI_SUCCESS) && (ainfo.strhAudio.dwRate) )
  }//if(chunkType == AVI_CHUNK_AUDIO)

  if(bSearchForward)
  {
    if(chunkType == AVI_CHUNK_VIDEO)
    {
      if( (GetVideoInfo(ulTrackID,&vinfo)==AVI_SUCCESS) && (vinfo.strhVideo.dwRate) )
      {
        if( (AVI_INDEX_OF_INDEXES == m_base_indx_tbl[ulTrackID].bIndexType) &&
            m_base_indx_tbl[ulTrackID].pIndxSuperIndexEntry)
        {
          uint64 ullVideoTS = 0;
          uint32 numOfChunksToJump = (uint32)( ( (float)vinfo.strhVideo.dwRate * ullReposTime) /
                                    ( (float)vinfo.strhVideo.dwScale * 1000.0f) );
          uint32 nTotalChunksJumped = 0;
          bool bKeyFrameFound = false;
          for(avi_uint32 ulSuperIndex = 0;
              ulSuperIndex < m_base_indx_tbl[ulTrackID].nEntriesInUse;
              ulSuperIndex++)
          {
            if(m_base_indx_tbl[ulTrackID].pIXIndexChunk)
            {
              if(numOfChunksToJump <
                  (uint32)m_base_indx_tbl[ulTrackID].pIXIndexChunk[ulSuperIndex].nEntriesInUse)
              {
                if (!m_base_indx_tbl[ulTrackID].pIXIndexChunk[ulSuperIndex].pIndxChunkIndexEntry)
                {
                  retError = AVI_UNKNOWN_ERROR;
                  break;
                }
                for(avi_uint32 ulStdIndex =0;
                    ulStdIndex < m_base_indx_tbl[ulTrackID].pIXIndexChunk[ulSuperIndex].nEntriesInUse;
                    ulStdIndex++)
                {
                  //! This variable is linearly updated, it has info about total
                  //! number of chunks/frames jumped till now. Do not update it
                  //! anywhere else.
                  nTotalChunksJumped ++;
                  if(m_base_indx_tbl[ulTrackID].pIXIndexChunk[ulSuperIndex].pIndxChunkIndexEntry[ulStdIndex].bKeyFrame)
                  {
                    ullVideoTS = (uint64)( ( (float)vinfo.strhVideo.dwScale/(float)vinfo.strhVideo.dwRate)*
                                   (float)nTotalChunksJumped*1000.0f);
                    if ( ullReposTime < ullVideoTS)
                    {
                      bKeyFrameFound = true;
                      numOfChunksToJump = ulStdIndex;
                      // Key Frame found details
                      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW,
                          "seekInSuperIndex Key Frame offset=%llu",
                           m_base_indx_tbl[ulTrackID].pIXIndexChunk[ulSuperIndex].pIndxChunkIndexEntry[ulStdIndex].dwOffset);
                      break;
                    }
                    else
                    {
                      numOfChunksToJump = ulStdIndex;
                      outputEntry->trackId = ulTrackID;
                      outputEntry->chunkType = AVI_CHUNK_VIDEO;
                      outputEntry->dwOffset =
                        m_base_indx_tbl[ulTrackID].pIXIndexChunk[ulSuperIndex].pIndxChunkIndexEntry[numOfChunksToJump].dwOffset;
                      outputEntry->dwSize =
                        m_base_indx_tbl[ulTrackID].pIXIndexChunk[ulSuperIndex].pIndxChunkIndexEntry[numOfChunksToJump].dwSize;
                      outputEntry->nTimeStamp = ullVideoTS;
                      outputEntry->dwVidFrameCount = nTotalChunksJumped;
                      outputEntry->dwFlags = 0x00000010;
                      retError = AVI_SUCCESS;
                    }
                  }
                }
                if(bKeyFrameFound)
                {
                  outputEntry->trackId = ulTrackID;
                  outputEntry->chunkType = AVI_CHUNK_VIDEO;
                  outputEntry->dwOffset =
                    m_base_indx_tbl[ulTrackID].pIXIndexChunk[ulSuperIndex].pIndxChunkIndexEntry[numOfChunksToJump].dwOffset;
                  outputEntry->dwSize =
                    m_base_indx_tbl[ulTrackID].pIXIndexChunk[ulSuperIndex].pIndxChunkIndexEntry[numOfChunksToJump].dwSize;
                  outputEntry->nTimeStamp = ullVideoTS;
                  outputEntry->dwVidFrameCount = nTotalChunksJumped;
                  outputEntry->dwFlags = 0x00000010;
                  retError = AVI_SUCCESS;
                  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
                      "Forward SeekInSuperIndex matched Video TS:%llu",
                      ullVideoTS);
                  MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_MEDIUM,
                      "Fwd SeekInSuperIndex matched at super index %u, std index %u",
                      ulSuperIndex, numOfChunksToJump);
                  break;
                }
              }
              else
              {
                numOfChunksToJump -= m_base_indx_tbl[ulTrackID].pIXIndexChunk[ulSuperIndex].nEntriesInUse;
                nTotalChunksJumped += m_base_indx_tbl[ulTrackID].pIXIndexChunk[ulSuperIndex].nEntriesInUse;
                continue;
              }
            }
          }
          if (!bKeyFrameFound)
          {
            MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                "Fatal, No Key Frame found in INDX entries");
          }
          if(numOfChunksToJump)
          {
            MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW,
                "SeekInSuperIndex Out of INDX entries");
          }
        }
      }//if( (GetVideoInfo(ulTrackID,&vinfo)==AVI_SUCCESS) && (vinfo.strhVideo.dwRate) )
    }//if(chunkType == AVI_CHUNK_VIDEO)
  }//if(bSearchForward)
  else
  {
    if(chunkType == AVI_CHUNK_VIDEO)
    {
      if( (GetVideoInfo(ulTrackID,&vinfo)==AVI_SUCCESS) && (vinfo.strhVideo.dwRate) )
      {
        if( (AVI_INDEX_OF_INDEXES == m_base_indx_tbl[ulTrackID].bIndexType) &&
            m_base_indx_tbl[ulTrackID].pIndxSuperIndexEntry)
        {
          uint64 ullVideoTS = 0;
          uint32 numOfChunksToJump = (uint32)( ( (float)vinfo.strhVideo.dwRate * ullReposTime) /
                                    ( (float)vinfo.strhVideo.dwScale * 1000.0f) );
          uint32 nTotalChunksJumped = 0;
          bool bKeyFrameFound = false;
          for(avi_uint32 ulSuperIndex = 0;
              ulSuperIndex < m_base_indx_tbl[ulTrackID].nEntriesInUse;
              ulSuperIndex++)
          {
            nTotalChunksJumped += (uint32)
              m_base_indx_tbl[ulTrackID].pIXIndexChunk[ulSuperIndex].nEntriesInUse;
          }
          // Since we are going in backwards directions.
          numOfChunksToJump = nTotalChunksJumped - numOfChunksToJump;
          for(int32 ulSuperIndex = m_base_indx_tbl[ulTrackID].nEntriesInUse -1;
              ulSuperIndex >= 0;
              ulSuperIndex--)
          {
            if(m_base_indx_tbl[ulTrackID].pIXIndexChunk)
            {
              if(numOfChunksToJump <
                 (uint32)m_base_indx_tbl[ulTrackID].pIXIndexChunk[ulSuperIndex].nEntriesInUse)
              {
                if (!m_base_indx_tbl[ulTrackID].pIXIndexChunk[ulSuperIndex].pIndxChunkIndexEntry)
                {
                  retError = AVI_UNKNOWN_ERROR;
                  break;
                }
                for(int32 ulStdIndex = m_base_indx_tbl[ulTrackID].pIXIndexChunk[ulSuperIndex].nEntriesInUse-1;
                    ulStdIndex >= 0;
                    ulStdIndex--)
                {
                  //! This variable is linearly updated, it has info about total
                  //! number of chunks/frames jumped till now. Do not update it
                  //! anywhere else.
                  nTotalChunksJumped --;
                  if(m_base_indx_tbl[ulTrackID].pIXIndexChunk[ulSuperIndex].pIndxChunkIndexEntry[ulStdIndex].bKeyFrame)
                  {
                    ullVideoTS = (uint64)(((float)vinfo.strhVideo.dwScale/
                                      (float)vinfo.strhVideo.dwRate)*
                                     (float)nTotalChunksJumped*1000.0f);
                    if ( ullReposTime > ullVideoTS)
                    {
                      bKeyFrameFound = true;
                      numOfChunksToJump = ulStdIndex;
                      // Key Frame found details
                      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
                          "Key Frame found at offset = %lu", (uint32)
                           m_base_indx_tbl[ulTrackID].pIXIndexChunk[ulSuperIndex].pIndxChunkIndexEntry[ulStdIndex].dwOffset);
                      break;
                    }
                    else
                    {
                      numOfChunksToJump = ulStdIndex;
                      outputEntry->trackId = ulTrackID;
                      outputEntry->chunkType = AVI_CHUNK_VIDEO;
                      outputEntry->dwOffset = m_base_indx_tbl[ulTrackID].pIXIndexChunk[ulSuperIndex].pIndxChunkIndexEntry[numOfChunksToJump].dwOffset;
                      outputEntry->dwSize = m_base_indx_tbl[ulTrackID].pIXIndexChunk[ulSuperIndex].pIndxChunkIndexEntry[numOfChunksToJump].dwSize;
                      outputEntry->nTimeStamp = ullVideoTS;
                      outputEntry->dwVidFrameCount = nTotalChunksJumped;
                      outputEntry->dwFlags = 0x00000010;
                      retError = AVI_SUCCESS;
                    }
                  }
                }
                if(bKeyFrameFound)
                {
                  outputEntry->trackId = ulTrackID;
                  outputEntry->chunkType = AVI_CHUNK_VIDEO;
                  outputEntry->dwOffset = m_base_indx_tbl[ulTrackID].pIXIndexChunk[ulSuperIndex].pIndxChunkIndexEntry[numOfChunksToJump].dwOffset;
                  outputEntry->dwSize = m_base_indx_tbl[ulTrackID].pIXIndexChunk[ulSuperIndex].pIndxChunkIndexEntry[numOfChunksToJump].dwSize;
                  outputEntry->nTimeStamp = ullVideoTS;
                  outputEntry->dwVidFrameCount = nTotalChunksJumped;
                  outputEntry->dwFlags = 0x00000010;
                  retError = AVI_SUCCESS;
                  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
                      "Backward SeekInSuperIndex matched Video TS:%llu",
                      ullVideoTS);
                  MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_MEDIUM,
                      "Backward SeekInSuperIndex matched at super index %lu, std index %lu",
                      ulSuperIndex, numOfChunksToJump);
                  break;
                }
              }
              else
              {
                numOfChunksToJump -= m_base_indx_tbl[ulTrackID].pIXIndexChunk[ulSuperIndex].nEntriesInUse;
                nTotalChunksJumped -= m_base_indx_tbl[ulTrackID].pIXIndexChunk[ulSuperIndex].nEntriesInUse;
                continue;
              }
            }
          }
          if (!bKeyFrameFound)
          {
            MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                "No Key Frame found in seek direcn in INDX entries");
          }
          if(numOfChunksToJump)
          {
            MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW,
                "aviParser::Out of INDX entries");
          }
        }
      }//if( (GetVideoInfo(ulTrackID,&vinfo)==AVI_SUCCESS) && (vinfo.strhVideo.dwRate) )
      else
      {
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
            "Failed GetVideoInfo && vinfo.strhVideo.dwRate");
      }
    }//if(chunkType == AVI_CHUNK_VIDEO)
  }
  return retError;
}
/* =============================================================================
FUNCTION:
 aviParser::Seek

DESCRIPTION:
 Seek the track identified by given trackid.

INPUT/OUTPUT PARAMETERS:
 trackid: Identifies the track to be repositioned.
 nReposTime:Target time to seek to
 nCurrPlayTime: Current playback time
 sample_info: Sample Info to be filled in if seek is successful
 canSyncToNonKeyFrame: When true, video can be repositioned to non key frame
 nSyncFramesToSkip:Number of sync samples to skip. Valid only if > or < 0.

RETURN VALUE:
 AVI_SUCCESS if successful otherwise returns appropriate error code.

SIDE EFFECTS:
  None.
=============================================================================*/
aviErrorType  aviParser::Seek(avi_uint32 trackid,
                              avi_uint64 nReposTime,
                              avi_uint64 nCurrPlayTime,/* Playback time at UI */
                              avi_sample_info* sample_info,
                              bool bSyncToKeyFrame,
                              int  nSyncFramesToSkip)
{
  #ifdef FEATURE_FILESOURCE_AVI_PARSER_DEBUG
    MM_MSG_PRIO3(MM_FILE_OPS, MM_PRIO_LOW,
                  "Seek trackId %d nReposTime %llu canSyncToNonKeyFrame %d",
                  trackid,nReposTime,canSyncToNonKeyFrame);
  #endif
  aviErrorType retError = AVI_FAILURE;
  CHUNK_t cType;
  avi_uint64 nCurrentTime = 0;
  bool bSearchForward = false;
  aviParserState prvParserState = m_CurrentParserState;
  avi_audiotrack_summary_info ainfo;
  avi_video_info vinfo;

  if(!sample_info)
  {
     MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
                   "Seek sample_info is NULL..");
    return AVI_INVALID_USER_DATA;
  }
  memset(sample_info,0,sizeof(avi_sample_info));
  memset(&ainfo,0,sizeof(avi_audiotrack_summary_info));
  memset(&vinfo,0,sizeof(avi_video_info));

  //Make sure parser is in valid state.
  if( (m_CurrentParserState == AVI_PARSER_READ_FAILED)          ||
      (m_CurrentParserState == AVI_PARSER_FAILED_CORRUPTED_FILE)||
      (m_CurrentParserState == AVI_PARSER_SEEK))
  {
     MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
                   "Seek Invalid m_CurrentParserState %d",m_CurrentParserState);
     return AVI_FAILURE;
  }
  //Make sure given trackid is valid.

  if(trackid < m_hAviSummary.n_streams)
  {
    //Make sure duration is valid..
    avi_uint64 max_duration = GetTrackDuration(trackid);
    if((max_duration < nReposTime) && (nReposTime != 0) && (max_duration > 0) &&
       (GetTrackChunkType(trackid,&cType) == AVI_SUCCESS) && (cType == AVI_CHUNK_AUDIO))
    {
      MM_MSG_PRIO3(MM_FILE_OPS, MM_PRIO_MEDIUM,
                        "Seek Time correction TrackDuration %llu <= Reposition time %llu for trackid %d",max_duration,nReposTime,trackid);
      nReposTime = max_duration;
    }
    if(GetTrackChunkType(trackid,&cType)==AVI_SUCCESS)
    {
      m_CurrentParserState = AVI_PARSER_SEEK;

      /*
      * When nSyncFramesToSkip is 0,we need to seek based on
      * nReposTime otherwise based on number of sync frames to skip.
      * Thus, when seeking based on target seek time, nSyncFramesToSkip should be 0.
      */
      if( (nReposTime == 0 ) && ( nSyncFramesToSkip == 0 ) )
      {
        if(m_nAdjustedIdx1Offset == 0)
        {
          m_nCurrentSampleInfoOffsetInIdx1 = m_nIdx1Offset;
        }
        else
        {
          m_nCurrentSampleInfoOffsetInIdx1 = m_nAdjustedIdx1Offset;
           MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
                    "Seek updated m_nCurrentSampleInfoOffsetInIdx1 to use m_nAdjustedIdx1Offset %llu",m_nAdjustedIdx1Offset);
        }
        m_nCurrAudioSampleInIdx1 = 0;
        m_nCurrVideoSampleInIdx1 = 0;

        nCurrentTime = 0;
        if(m_nAdjustedMoviOffset == 0)
        {
          m_nCurrOffset = m_nMoviOffset;
        }
        else
        {
          m_nCurrOffset = m_nAdjustedMoviOffset;
          MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
                    "Seek updated m_nCurrOffset to use m_nAdjustedMoviOffset %llu",m_nAdjustedMoviOffset);
        }
        m_nCurrentChunkDataSize = 0;
        m_CurrentParserState = AVI_PARSER_CHUNK_HEADER_START;
        sample_info->nTimeStamp =  nCurrentTime;
        sample_info->chunkType = cType;
        retError = AVI_SUCCESS;
        sample_info->bSync = 1;
        if(sample_info->chunkType == AVI_CHUNK_AUDIO)
        {
          //AUDIO
          m_nCurrAudioPayloadSize[trackid] = 0;
          m_nCurrAudioFrameCount[trackid] = 0;
          m_nParserAudSampleEndTime[trackid] = 0;
        }
        if(sample_info->chunkType == AVI_CHUNK_VIDEO)
        {
          //VIDEO
          m_nCurrVideoFrameCount[trackid] = 0;
        }
        return retError;
      }
      if( (!m_nIdx1Offset) && (!m_base_indx_tbl[trackid].isAvailable))
      {
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
        "Seek Failed, no IDX1/INDX available");
        return AVI_FAILURE;
      }
      nCurrentTime =  nCurrPlayTime;

      MM_MSG_PRIO3(MM_FILE_OPS, MM_PRIO_MEDIUM,
                    "Seek nCurrPlayTime %llu nReposTime %llu nSyncFramesToSkip %d",
                    nCurrPlayTime,nReposTime,nSyncFramesToSkip);

      /* get sample time for this track to which parser is pointing to */
      uint64 currentSampleTime = getCurrentPlaybackTime(trackid);

      switch(cType)
      {
        case AVI_CHUNK_AUDIO:
            {
              bSearchForward = (nReposTime > currentSampleTime)?true:false;
              MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM, "Seek AUDIO bSearchForward %d",bSearchForward);
              avi_idx1_entry matchedEntry;
              memset(&matchedEntry,0,sizeof(avi_idx1_entry));
              aviErrorType result = AVI_PARSE_ERROR;
              bool endOfFileReached = false;
              #ifdef AVI_PARSER_SEEK_SANITY_TEST
                //Debug purpose
                (void)doSanityCheckBeforeSeek(trackid,AVI_CHUNK_AUDIO,prvParserState);
              #endif
              if( (m_base_indx_tbl[trackid].isAvailable) && (nSyncFramesToSkip == 0))
              {
                result = seekInSuperIndex(trackid,nReposTime,&matchedEntry,
                                      bSearchForward,AVI_CHUNK_AUDIO,
                                      nSyncFramesToSkip,&endOfFileReached,
                                      bSyncToKeyFrame);
              }
              else
#ifndef AVI_PARSER_FAST_START_UP
              if( (m_hAviSummary.pIdx1Table)                &&
                  (m_hAviSummary.pIdx1Table->pAudioEntries) &&
                  (bSyncToKeyFrame)                         &&
                  (nSyncFramesToSkip == 0) )
              {
                result = searchIDX1Cache(trackid,
                                         nReposTime,
                                         &matchedEntry,
                                         bSearchForward,
                                         AVI_CHUNK_AUDIO,
                                         nSyncFramesToSkip,
                                         &endOfFileReached);
              }
              else
#endif
              {
                  result = seekInIDX1(trackid,nReposTime,&matchedEntry,
                                      bSearchForward,AVI_CHUNK_AUDIO,
                                      nSyncFramesToSkip,&endOfFileReached,
                                      bSyncToKeyFrame);
              }
              if( ((result == AVI_SUCCESS) || (endOfFileReached)) && (matchedEntry.chunkType == AVI_CHUNK_AUDIO))
              {
                MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
                              "Matched Audio index entry TS %llu",matchedEntry.nTimeStamp);

                m_nCurrOffset = matchedEntry.dwOffset;
                m_nCurrentChunkDataSize = 0;
                m_nCurrAudioPayloadSize[trackid] = matchedEntry.nTotalSizeInBytes;
                m_nCurrAudioFrameCount[trackid] = matchedEntry.dwAudFrameCount;
                m_nCurrentSampleInfoOffsetInIdx1 = matchedEntry.dwOffsetInIDX1;

                m_CurrentParserState = AVI_PARSER_CHUNK_HEADER_START;
                MM_MSG_PRIO6(MM_FILE_OPS, MM_PRIO_MEDIUM,
                                "m_nCurrOffset %llu trackid %d m_nCurrAudioPayloadSize[trackid] %llu Chunk# %d m_CurrentParserState %d m_nCurrentSampleInfoOffsetInIdx1 %llu",
                                 m_nCurrOffset, trackid, m_nCurrAudioPayloadSize[trackid], m_nCurrAudioFrameCount[trackid],m_CurrentParserState,m_nCurrentSampleInfoOffsetInIdx1);

                sample_info->chunkType  = AVI_CHUNK_AUDIO;
                sample_info->nSampleSize= matchedEntry.dwSize;
                sample_info->nTimeStamp = (avi_uint64)matchedEntry.nTimeStamp;
                sample_info->bSync = 1;
                if(GetAudioTrackSummaryInfo(trackid,&ainfo)== AVI_SUCCESS)
                {
                  if( (!ainfo.isVbr) && (ainfo.audioBytesPerSec) )
                  {
                    sample_info->nDuration = (avi_uint32)(sample_info->nTimeStamp - (avi_int64)((float)(m_nCurrAudioPayloadSize[trackid]+ sample_info->nSampleSize)/
                                             (float)ainfo.audioBytesPerSec * 1000.0f));
                    m_nParserAudSampleEndTime[trackid] = sample_info->nTimeStamp;
                  }
                  else if(ainfo.isVbr && ainfo.audioFrequency)
                  {
                    avi_audio_info audinfo;
                    if( (GetAudioInfo(trackid,&audinfo) == AVI_SUCCESS) && (audinfo.strhAudio.dwRate) )
                    {
                      m_nParserAudSampleEndTime[trackid] = sample_info->nTimeStamp;
                      sample_info->nDuration = (avi_uint32)
                            ( ((double)audinfo.strhAudio.dwScale/audinfo.strhAudio.dwRate)* 1000.f);
                    }
                  }
                }
                retError = AVI_SUCCESS;
              }
              break;
      }
        case AVI_CHUNK_VIDEO:
            {
              bSearchForward = (nReposTime > currentSampleTime)?true:false;
              MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
                            "Seek VIDEO bSearchForward %d",bSearchForward);
              avi_idx1_entry matchedEntry;
              memset(&matchedEntry,0,sizeof(avi_idx1_entry));
              aviErrorType result = AVI_PARSE_ERROR;
              bool endOfFileReached = false;
              #ifdef AVI_PARSER_SEEK_SANITY_TEST
                //Debug purpose
                (void)doSanityCheckBeforeSeek(trackid,AVI_CHUNK_VIDEO,prvParserState);
              #endif
              if( (m_base_indx_tbl[trackid].isAvailable) && (nSyncFramesToSkip == 0) )
              {
                result = seekInSuperIndex(trackid,nReposTime,&matchedEntry,
                                      bSearchForward,AVI_CHUNK_VIDEO,
                                      nSyncFramesToSkip,&endOfFileReached,
                                      bSyncToKeyFrame);
              }
              else
#ifndef AVI_PARSER_FAST_START_UP
              if( (m_hAviSummary.pIdx1Table)                   &&
                  (m_hAviSummary.pIdx1Table->pKeyVideoEntries) &&
                  (bSyncToKeyFrame)                            &&
                  (nSyncFramesToSkip == 0) )
              {
                 result = searchIDX1Cache(trackid,
                                          nReposTime,
                                          &matchedEntry,
                                          bSearchForward,
                                          AVI_CHUNK_VIDEO,
                                          nSyncFramesToSkip,
                                          &endOfFileReached);
             }
             else
#endif
             {
                 result = seekInIDX1(trackid,nReposTime,&matchedEntry,
                                     bSearchForward,AVI_CHUNK_VIDEO,nSyncFramesToSkip,
                                     &endOfFileReached,bSyncToKeyFrame);
              }
              if( ( (result == AVI_SUCCESS) || (endOfFileReached) ) && (matchedEntry.chunkType == AVI_CHUNK_VIDEO) )
              {
                MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
                              "Matched Video index entry TS %llu",matchedEntry.nTimeStamp);
                m_nCurrOffset = matchedEntry.dwOffset;
                m_nCurrentChunkDataSize = 0;
                m_nCurrVideoFrameCount[trackid] = matchedEntry.dwVidFrameCount;
                m_CurrentParserState = AVI_PARSER_CHUNK_HEADER_START;
                m_nCurrentSampleInfoOffsetInIdx1 = matchedEntry.dwOffsetInIDX1;
                MM_MSG_PRIO5(MM_FILE_OPS, MM_PRIO_MEDIUM,
                                "m_nCurrOffset %llu trackid %d m_nCurrVideoFrameCount[trackid] %d m_CurrentParserState %d m_nCurrentSampleInfoOffsetInIdx1 %llu",
                                 m_nCurrOffset, trackid, m_nCurrVideoFrameCount[trackid],m_CurrentParserState,m_nCurrentSampleInfoOffsetInIdx1);

                sample_info->chunkType  = AVI_CHUNK_VIDEO;
                sample_info->nSampleSize= matchedEntry.dwSize;
                sample_info->nTimeStamp = (avi_uint64)matchedEntry.nTimeStamp;
                if(matchedEntry.dwFlags & AVI_KEY_FRAME_MASK)
                {
                  sample_info->bSync = 1;
                }
                if(GetVideoInfo(trackid,&vinfo)== AVI_SUCCESS)
                {
                  sample_info->nDuration  = (avi_int32)( ( (float)vinfo.strhVideo.dwScale/(float)vinfo.strhVideo.dwRate)*
                                                         (float)1000.0f );
                }
                /*
                * If clip is encrypted, we need to locate corresponding DRM chunk for the matched entry, if any.
                * If yes, need to adjust IDX1/MOVI offset so that parser will parse DRM chunk before retrieving matched video sample.
                */
                int nDrmChunkSize = 0;
                nDrmChunkSize = isCurrentFrameEncrypted(trackid,m_nCurrentSampleInfoOffsetInIdx1,m_nCurrOffset);

                if( nDrmChunkSize )
                {
                  m_nCurrentSampleInfoOffsetInIdx1 -= (4 * sizeof(avi_uint32) );
                  m_nCurrOffset -= (nDrmChunkSize+sizeof(fourCC_t)+sizeof(avi_uint32));

                  MM_MSG_PRIO3(MM_FILE_OPS, MM_PRIO_MEDIUM,
                                 "nDrmChunkSize %d updated m_nCurrOffset %llu updated m_nCurrentSampleInfoOffsetInIdx1 %llu",
                                 nDrmChunkSize, m_nCurrOffset, m_nCurrentSampleInfoOffsetInIdx1);
                }
                retError = AVI_SUCCESS;
              }
              break;
        }
        default:
          MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH, "Can't seek trackId %d",trackid);
        break;
      }
    }//if(GetTrackChunkType(trackid,&cType)==AVI_SUCCESS)
  }//if(trackid < m_hAviSummary.n_streams)
  if( retError != AVI_SUCCESS)
  {
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,"Reposition failed for trackid %d",trackid);
    m_CurrentParserState = prvParserState;
  }
  return  retError;
}
/* =============================================================================
FUNCTION:
 aviParser::isCurrentFrameEncrypted

DESCRIPTION:
Determine if current sample is encrypted or not.

trackid      : Identifies the track
offsetinidx1 : Offset of sample in idx1
movioffset   : sample offset in MOVI

INPUT/OUTPUT PARAMETERS:
  @See above.

RETURN VALUE:
 DRM chunk size for the sample identified by given trackid and offsetinidx1/movioffset

SIDE EFFECTS:
  None.
=============================================================================*/
avi_uint32 aviParser::isCurrentFrameEncrypted(avi_uint32 trid, avi_uint64 offsetinidx1, avi_uint64 movioffset)
{
  avi_uint32 drm_chunk_size = 0;
  unsigned char* byteData = m_ReadBuffer;

  avi_uint16 trackId = 0;
  avi_uint16 cType = 0;
  avi_uint32 dwFlags = 0;
  avi_uint64 dwOffset = 0;
  avi_uint32 dwSize = 0;
  aviErrorType retError = AVI_SUCCESS;
  #ifdef FEATURE_FILESOURCE_AVI_PARSER_DEBUG
    MM_MSG_PRIO3(MM_FILE_OPS,
                  MM_PRIO_MEDIUM,"isCurrentFrameEncrypted trid %ld offsetinidx1 %ld movioffset %ld",
                  trid,offsetinidx1,movioffset);
  #endif

  if(offsetinidx1)
  {
    offsetinidx1 = offsetinidx1 - (4 * sizeof(avi_uint32));

    if(!parserAVICallbakGetData(offsetinidx1,4 * sizeof(avi_uint32),
      m_ReadBuffer,AVI_READ_BUFFER_SIZE,m_pUserData,&retError))
    {
      return retError;
    }
    memcpy(&trackId,byteData,sizeof(avi_uint16));
    trackId = ascii_2_short_int(&trackId);
    memcpy(&cType,byteData+2,sizeof(avi_uint16));
    memcpy(&dwFlags,byteData+sizeof(avi_uint32),sizeof(avi_uint32));
    memcpy(&dwOffset,byteData+(2*sizeof(avi_uint32)),sizeof(avi_uint32));
    if(m_bByteAdjustedForMOVI)
    {
      dwOffset = (m_nStartOfMovi + dwOffset - m_nBytesToBeAdjustedForMOVI);
    }
    memcpy(&dwSize,byteData+(3*sizeof(avi_uint32)),sizeof(avi_uint32));

    #ifdef FEATURE_FILESOURCE_AVI_PARSER_DEBUG
      MM_MSG_PRIO4(MM_FILE_OPS,
                    MM_PRIO_MEDIUM,"isCurrentFrameEncrypted trackId %d cType %d dwFlags %d dwOffset %d",
                    trackId,cType,dwFlags,dwOffset);
    #endif
    if( (!memcmp(&cType,"dd",sizeof(avi_uint16))) && (trackId == trid) )
    {
      drm_chunk_size = dwSize;
      #ifdef FEATURE_FILESOURCE_AVI_PARSER_DEBUG
        MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
                      "isCurrentFrameEncrypted Matched sample is encrypted,located corresponding DRM chunk drm_chunk_size %d",drm_chunk_size);
      #endif
      if( (dwOffset + drm_chunk_size + sizeof(fourCC_t)+sizeof(avi_uint32)) != movioffset )
      {
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,
                      "isCurrentFrameEncrypted matched DRM chunk offset mismatch with current MOVI offset..");
        MM_MSG_PRIO3(MM_FILE_OPS, MM_PRIO_MEDIUM,
                      "isCurrentFrameEncrypted dwOffset %llu drm_chunk_size %d movioffset %llu",
                      dwOffset, drm_chunk_size, movioffset);
      }
    }
  }
  return drm_chunk_size;
}
/* =============================================================================
FUNCTION:
 aviParser::flushIdx1SeekCache

DESCRIPTION:
 Cache management routine to flush out IDX1 Seek cache.

INPUT/OUTPUT PARAMETERS:
  None

RETURN VALUE:
 NOne

 SIDE EFFECTS:
  None.
=============================================================================*/
void aviParser::flushIdx1SeekCache(void)
{
  #ifdef FEATURE_FILESOURCE_AVI_PARSER_DEBUG
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW,"flushIdx1SeekCache");
  #endif

  if(m_pIdx1SeekCache && m_pIdx1SeekCache->pMemory)
  {
    #ifdef FEATURE_FILESOURCE_AVI_PARSER_DEBUG
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW,"flushIdx1SeekCache resetting Cache structure..");
    #endif
    m_pIdx1SeekCache->nReadOffset = 0;
    m_pIdx1SeekCache->nSize = 0;
    m_pIdx1SeekCache->nStartOffset = 0;
    m_pIdx1SeekCache->nEndOffset = 0;
  }
}
/* =============================================================================
FUNCTION:
 aviParser::readFromIdx1SeekCache

DESCRIPTION:
 Cache management routine to be used to carry out Seek especially in RW
 as OSCL cache works in forward direction. We want caching to be
 in backward direction when performing Rewind to reduce the delay.
 Note: Currently, this routine can be used as a cache only for reading IDX1 in backward direction.
       To use it for general purpose, need to change bound checking on tmpEndOffset and tmpStartOffset.
       To be able to use in both direction, we need to add the direction as well.
       Don't free up the memory once RW is done. Frequent allocation/free up might result in
       Heap fragmentation subsequently as user won't be able to RW.

 nOffset          :Offset to read from.
 nNumBytesRequest : Number of bytes to read.
 ppData           : Buffer to read in data.

INPUT/OUTPUT PARAMETERS:
  @See above.

RETURN VALUE:
 Number of bytes read.

SIDE EFFECTS:
  None.
=============================================================================*/
avi_uint32 aviParser::readFromIdx1SeekCache(avi_uint64 nOffset,
                                            avi_int32 nNumBytesRequest,
                                            avi_int32 nMaxSize,
                                            unsigned char *pData)
{
  avi_uint32 nRead = 0;
  avi_uint64 tmpSize = 0;
  avi_uint64 tmpStartOffset = 0;
  avi_uint64 tmpEndOffset = 0;
  aviErrorType retError = AVI_SUCCESS;

#ifdef FEATURE_FILESOURCE_AVI_PARSER_DEBUG
  MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_MEDIUM,
                "readFromIdx1SeekCache nOffset %d nNumBytesRequest %d",
                nOffset,nNumBytesRequest);
#endif

  if(!m_pIdx1SeekCache)
  {
    m_pIdx1SeekCache = (avi_parser_seek_buffer_cache*)
                       MM_Malloc(sizeof(avi_parser_seek_buffer_cache));
    if(!m_pIdx1SeekCache)
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                   "readFromIdx1SeekCache failed to allocate memory for m_pIdx1SeekCache");
      return 0;
    }
    memset(m_pIdx1SeekCache,0,sizeof(avi_parser_seek_buffer_cache));
  }

  if(!m_pIdx1SeekCache->pMemory)
  {
    m_pIdx1SeekCache->nReadOffset = -1;
    m_pIdx1SeekCache->pMemory = (avi_uint8*)
                       MM_Malloc(AVI_PARSER_IDX1_SEEK_BUFFER_CACHE_SIZE);
    if(!m_pIdx1SeekCache->pMemory)
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                   "readFromIdx1SeekCache failed to allocate memory for m_pIdx1SeekCache->pMemory");
      MM_Free(m_pIdx1SeekCache);
      m_pIdx1SeekCache = NULL;
      return 0;
    }
  }


    if( (nOffset < m_pIdx1SeekCache->nStartOffset)                 ||
        ((nOffset+nNumBytesRequest) > m_pIdx1SeekCache->nEndOffset)||
        (m_pIdx1SeekCache->nReadOffset < 0) )
  {
    //Time to refill cache, flush curent cache state.
    flushIdx1SeekCache();
    m_pIdx1SeekCache->nReadOffset = -1;
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW,
                 "readFromIdx1SeekCache Refilling cache...");
  }

  if(m_pIdx1SeekCache->nReadOffset < 0)
  {
    //Cache is empty, fill it up.

    //Find the range to cache.
    tmpEndOffset = nOffset + nNumBytesRequest;

    //Validate end offset
    if( (m_nIdx1Offset + m_nIdx1Size) < tmpEndOffset)
    {
      MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_MEDIUM,
                    "readFromIdx1SeekCache tmpEndOffset %llu > (m_nIdx1Offset + m_nIdx1Size) %llu",
                    tmpEndOffset,(m_nIdx1Offset + m_nIdx1Size));
      tmpEndOffset = m_nIdx1Offset + m_nIdx1Size;

      if(tmpEndOffset < nOffset)
      {
        MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_FATAL,
                      "readFromIdx1SeekCache adjusted tmpEndOffset %llu < nOffset %llu",
                      tmpEndOffset,nOffset);
        //We should never come here, print error message and bail out.
        return 0;
      }
    }

    tmpStartOffset = tmpEndOffset - AVI_PARSER_IDX1_SEEK_BUFFER_CACHE_SIZE;

    #ifdef FEATURE_FILESOURCE_AVI_PARSER_DEBUG
      MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_MEDIUM,
                    "readFromIdx1SeekCache tmpStartOffset %d tmpEndOffset %d",
                    tmpStartOffset,tmpEndOffset);
    #endif

    if(tmpStartOffset < m_nIdx1Offset)
    {
      MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_MEDIUM,
                    "readFromIdx1SeekCache tmpStartOffset %llu < m_nIdx1Offset %llu",tmpStartOffset,m_nIdx1Offset);
      tmpStartOffset = m_nIdx1Offset;
      if(tmpStartOffset > nOffset)
      {
        MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_FATAL,
                      "readFromIdx1SeekCache adjusted tmpStartOffset %llu > nOffset %llu",
                      tmpStartOffset,nOffset);
        //We should never come here, print error message an bail out.
        return 0;
      }
    }
    #ifdef FEATURE_FILESOURCE_AVI_PARSER_DEBUG
      MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_MEDIUM,
                    "readFromIdx1SeekCache tmpStartOffset %d tmpEndOffset %d",tmpStartOffset,tmpEndOffset);
    #endif

    tmpSize = tmpEndOffset - tmpStartOffset;

    #ifdef FEATURE_FILESOURCE_AVI_PARSER_DEBUG
      MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_HIGH,
                    "readFromIdx1SeekCache tmpSize %d MAX SIZE %d",tmpSize,AVI_PARSER_IDX1_SEEK_BUFFER_CACHE_SIZE);
    #endif
    if(tmpSize > AVI_PARSER_IDX1_SEEK_BUFFER_CACHE_SIZE)
    {
      //Something is very bad, print an fatal error mesage and bail out.
      MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_FATAL,
                 "readFromIdx1SeekCache tmpSize %llu > MAX SIZE %d ",tmpSize,AVI_PARSER_IDX1_SEEK_BUFFER_CACHE_SIZE);
      return 0;
    }

    if(!parserAVICallbakGetData(tmpStartOffset,
                          (avi_int32)tmpSize,
                          m_pIdx1SeekCache->pMemory,
                          AVI_PARSER_IDX1_SEEK_BUFFER_CACHE_SIZE,
                          m_pUserData,
                          &retError))
    {
      //Read failed, free up memory and return 0.
      MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_FATAL,
                 "readFromIdx1SeekCache READ FAILED nOffset %llu nBytes %d ",nOffset,AVI_PARSER_IDX1_SEEK_BUFFER_CACHE_SIZE);
      flushIdx1SeekCache();
      MM_Free(m_pIdx1SeekCache->pMemory);
      MM_Free(m_pIdx1SeekCache);
      m_pIdx1SeekCache = NULL;
      return 0;
    }
    m_pIdx1SeekCache->nStartOffset = tmpStartOffset;
    m_pIdx1SeekCache->nEndOffset   = tmpEndOffset;
    m_pIdx1SeekCache->nSize        = tmpSize;
    m_pIdx1SeekCache->nReadOffset  = tmpStartOffset;

    avi_uint64 offdiff = nOffset - m_pIdx1SeekCache->nStartOffset;
    //Make sure offset to read belongs to our cached region
    if( (nOffset >= m_pIdx1SeekCache->nStartOffset) &&
        (nOffset <= m_pIdx1SeekCache->nEndOffset) &&
        (offdiff < AVI_PARSER_IDX1_SEEK_BUFFER_CACHE_SIZE))
    {
      memcpy(pData,m_pIdx1SeekCache->pMemory+offdiff,nNumBytesRequest);
      //*ppData = (m_pIdx1SeekCache->pMemory+offdiff);
      nRead = nNumBytesRequest;
      m_pIdx1SeekCache->nReadOffset = nOffset;
    }
    else
    {
      //Something is terrible, print fatal message and bail out.
       MM_MSG_PRIO3(MM_FILE_OPS, MM_PRIO_FATAL,
                    "readFromIdx1SeekCache nOffset not in cached region nOffset %llu m_pIdx1SeekCache->nStartOffset %llu m_pIdx1SeekCache->nEndOffset %llu",
                    nOffset, m_pIdx1SeekCache->nStartOffset, m_pIdx1SeekCache->nEndOffset);
      flushIdx1SeekCache();
      return 0;
    }
  }
  else
  {
    avi_uint64 offdiff = nOffset - m_pIdx1SeekCache->nStartOffset;
    if (offdiff < AVI_PARSER_IDX1_SEEK_BUFFER_CACHE_SIZE)
    {
      memcpy(pData,m_pIdx1SeekCache->pMemory+offdiff,nNumBytesRequest);
      //*ppData = (m_pIdx1SeekCache->pMemory+offdiff);
      nRead = nNumBytesRequest;
      m_pIdx1SeekCache->nReadOffset = nOffset;
    }
  }
  return nRead;
}
/* =============================================================================
FUNCTION:
 aviParser::searchIDX1Cache

DESCRIPTION:
 Seek the track identified by given trackid based on IDX1 cache

INPUT/OUTPUT PARAMETERS:
  trackid          : Identifies the track to be repositioned.
  nReposTime       : Target time to seek to.
  outputEntry      : Fills in idx1 entry information once the seek is complete.
  bSearchForward   : Direction to search in IDX1
  chunkType        : Track type
  nSyncFramesToSkip: Number of sync frames to skip in forward or backward direction.

Please note that nReposTime will be used to carry out SEEK only when nSyncFramesToSkip is 0.

RETURN VALUE:
  AVI_SUCCESS if successful otherwise returns appropriate error code.

SIDE EFFECTS:
  None.
=============================================================================*/
#ifndef AVI_PARSER_FAST_START_UP
aviErrorType aviParser::searchIDX1Cache(avi_uint32 trid,
                                       avi_uint64 nReposTime,
                                       avi_idx1_entry* outputEntry,
                                       bool bSearchForward,
                                       CHUNK_t chunkType,
                                       int  nSyncFramesToSkip,
                                       bool* endOfFileReached)
#else
aviErrorType aviParser::searchIDX1Cache(avi_uint32, avi_uint64, avi_idx1_entry*,
                                       bool, CHUNK_t, int, bool*)
#endif
{
  aviErrorType retError = AVI_PARSE_ERROR;
#ifndef AVI_PARSER_FAST_START_UP
  bool bOkToRepos = true;
  int entryIndex = 0;

  avi_uint16 cType = 0;
  avi_uint16 trackId = 0;
  avi_uint32 dwFlags = 0;
  avi_uint64 dwOffset = 0;
  avi_uint32 dwSize = 0;

  avi_uint32 audioChunksFromStart = 0;
  avi_uint64 audioBytesFromStart = 0;
  avi_uint64 tsAudio = 0;

  MM_MSG_PRIO4(MM_FILE_OPS, MM_PRIO_MEDIUM,"searchIDX1Cache trackid %d bsearchfwd %d nReposTime %d nSyncFramesToSkip %d",
               trid,bSearchForward,nReposTime,nSyncFramesToSkip);

  if(!outputEntry)
  {
    bOkToRepos = false;
  }
  if((chunkType == AVI_CHUNK_AUDIO)&&(bOkToRepos))
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,"searchIDX1Cache seeking audio track");
    memset(outputEntry,0,sizeof(avi_idx1_entry));
    if( (!m_hAviSummary.pIdx1Table->pAudioEntries) ||
        (!m_hAviSummary.pIdx1Table->nCurrParsedAudioEntriesInIDX1) )
    {
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,"No Audio Entries exist for trackid %d",trid);
      bOkToRepos = false;
    }
    /* If Idx1 table does not contain any entries, then this else condition will be skipped */
    else
    {
      avi_uint32 nCacheIndex = (avi_uint32)(nReposTime / AUDIO_SAMPLE_CACHING_INTERVAL);
      if(nCacheIndex > m_hAviSummary.pIdx1Table->nCurrParsedAudioEntriesInIDX1)
      {
        //Go to last entry if nCacheIndex is outside current parsed audio entries.
        nCacheIndex = m_hAviSummary.pIdx1Table->nCurrParsedAudioEntriesInIDX1 -1;
        MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,"searchIDX1Cache updated nCacheIndex %d",nCacheIndex);
      }
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,"searchIDX1Cache nCacheIndex %d",nCacheIndex);
      if( m_hAviSummary.pIdx1Table->pAudioEntries[nCacheIndex].nTimeStamp > nReposTime )
      {
        while(m_hAviSummary.pIdx1Table->pAudioEntries[nCacheIndex].nTimeStamp > nReposTime)
        {
          nCacheIndex--;
        }
      }
      if(!bSearchForward)
        nCacheIndex++;
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,"searchIDX1Cache updated nCacheIndex for REWIND CASE %d",nCacheIndex);
      bool bFirstEntry = true;
      if(bSearchForward && bOkToRepos)
      {
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,"searchIDX1Cache seeking in FORWARD dirn");
        for(avi_uint64 itr = m_hAviSummary.pIdx1Table->pAudioEntries[nCacheIndex].dwOffsetInIDX1;
            itr < (m_nIdx1Offset+m_nIdx1Size) ; )
        {
          if(!parserAVICallbakGetData(itr,4 * sizeof(avi_uint32),
            m_ReadBuffer,AVI_READ_BUFFER_SIZE,m_pUserData,&retError))
          {
            return retError;
          }
          itr += (4 *sizeof(avi_uint32));
          if(bFirstEntry)
          {
            trackId = m_hAviSummary.pIdx1Table->pAudioEntries[nCacheIndex].trackId;
            cType = m_hAviSummary.pIdx1Table->pAudioEntries[nCacheIndex].chunkType;
            dwFlags = m_hAviSummary.pIdx1Table->pAudioEntries[nCacheIndex].dwFlags;
            dwOffset= m_hAviSummary.pIdx1Table->pAudioEntries[nCacheIndex].dwOffset;
            audioChunksFromStart = m_hAviSummary.pIdx1Table->pAudioEntries[nCacheIndex].dwAudFrameCount;
            audioBytesFromStart = m_hAviSummary.pIdx1Table->pAudioEntries[nCacheIndex].nTotalSizeInBytes;
            tsAudio = (avi_uint64)m_hAviSummary.pIdx1Table->pAudioEntries[nCacheIndex].nTimeStamp;
            dwSize = m_hAviSummary.pIdx1Table->pAudioEntries[nCacheIndex].dwSize;
          }
          else
          {
            memcpy(&trackId,m_ReadBuffer,sizeof(avi_uint16));
            trackId = ascii_2_short_int(&trackId);
            memcpy(&cType,m_ReadBuffer+2,sizeof(avi_uint16));
            memcpy(&dwFlags,m_ReadBuffer+sizeof(avi_uint32),sizeof(avi_uint32));
            memcpy(&dwOffset,m_ReadBuffer+(2*sizeof(avi_uint32)),sizeof(avi_uint32));
            if(m_bByteAdjustedForMOVI)
            {
              dwOffset = (m_nStartOfMovi + dwOffset - m_nBytesToBeAdjustedForMOVI);
            }
            memcpy(&dwSize,m_ReadBuffer+(3*sizeof(avi_uint32)),sizeof(avi_uint32));
          }
          if( (trackId == trid) && ( (!memcmp(&cType,"wb",sizeof(avi_uint16)))||(bFirstEntry) ) )
          {
            avi_audio_info audstrminfo;
            avi_audiotrack_summary_info ainfo;
            memset(&audstrminfo,0,sizeof(avi_audio_info));
            memset(&ainfo,0,sizeof(avi_audiotrack_summary_info));

            if( (GetAudioTrackSummaryInfo(trackId,&ainfo)==AVI_SUCCESS) &&
                (GetAudioInfo(trackId,&audstrminfo) == AVI_SUCCESS) )
            {
              if(audstrminfo.strhAudio.dwSampleSize > 0)
              {
                if(audstrminfo.strfAudio.nBlockAlign && audstrminfo.strhAudio.dwRate)
                {
                  double val1 = ceil( ( (double)dwSize / audstrminfo.strfAudio.nBlockAlign) );
                  double val2 =  ((double)audstrminfo.strhAudio.dwScale/audstrminfo.strhAudio.dwRate)* 1000.f;
                  tsAudio += (uint32)(val1 * val2);
                }
              }//if(audstrminfo.strhAudio.dwSampleSize > 0)
              else
              {
                tsAudio = (avi_uint64)(( ( (float)audstrminfo.strhAudio.dwScale/
                                        (float)audstrminfo.strhAudio.dwRate) * 1000.f) * (float)audioChunksFromStart);
              }
            }//if( (GetAudioTrackSummaryInfo(trackId,&ainfo)==AVI_SUCCESS) &&...
            MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,"searchIDX1Cache tsAudio %d",tsAudio);
            outputEntry->trackId = trid;
            outputEntry->dwFlags = dwFlags;
            outputEntry->dwOffset = dwOffset;
            outputEntry->dwSize = dwSize;
            outputEntry->dwOffsetInIDX1 = (itr - (4 *sizeof(avi_uint32)));
            outputEntry->dwAudFrameCount = audioChunksFromStart;
            outputEntry->nTimeStamp = tsAudio;
            outputEntry->nTotalSizeInBytes = audioBytesFromStart;
            outputEntry->chunkType = AVI_CHUNK_AUDIO;
            m_nCurrAudioSampleInIdx1 = outputEntry->dwAudFrameCount;

            if( (tsAudio >= nReposTime) )
            {
              retError = AVI_SUCCESS;
              /*
              * Located the correct audio entry/chunk.
              * Load IDX1 entry values into output idx1 entry and return SUCCESS;
              */
              MM_MSG_PRIO5(MM_FILE_OPS, MM_PRIO_LOW,
                              "searchIDX1Cache Matched Audio Entry: trackid %d dwFlags %d dwOffset %d dwSize %d dwOffsetInIDX1 %d",
                              trid,dwFlags,dwOffset,dwSize,outputEntry->dwOffsetInIDX1);

              MM_MSG_PRIO3(MM_FILE_OPS, MM_PRIO_LOW,
                            "searchIDX1Cache Matched Audio Entry: dwAudFrameCount %d TS %d nTotalSizeInBytes %d",
                            outputEntry->dwAudFrameCount,outputEntry->nTimeStamp,audioBytesFromStart);

              return retError;
            }//if(tsAudio >= nReposTime)
            else
            {
              audioBytesFromStart += dwSize;
              audioChunksFromStart++;
              bFirstEntry = false;
            }//end of else of if(tsAudio >= nReposTime)
          }//if( (trackId == trid) && ( (!memcmp(&cType,"wb",sizeof(avi_uint16)))||(bFirstEntry) ) )
        }//for(avi_uint64 itr = offset; itr < (m_nIdx1Offset+m_nIdx1Size) ; )
        *endOfFileReached = true;
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"searchIDX1Cache end of file reached while seeking");
      }//if(bSearchForward && bOkToRepos)
      else if( (!bSearchForward)&& bOkToRepos )
      {
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,"searchIDX1Cache seeking in BACKWARD dirn");
        for(avi_uint64 itr = m_hAviSummary.pIdx1Table->pAudioEntries[nCacheIndex].dwOffsetInIDX1;
            itr > m_nIdx1Offset; )
        {
          if(!parserAVICallbakGetData(itr,4 * sizeof(avi_uint32),
            m_ReadBuffer,AVI_READ_BUFFER_SIZE,m_pUserData, &retError))
          {
            return retError;
          }
          itr -= (4 *sizeof(avi_uint32));
          if(bFirstEntry)
          {
            trackId = m_hAviSummary.pIdx1Table->pAudioEntries[nCacheIndex].trackId;
            cType = m_hAviSummary.pIdx1Table->pAudioEntries[nCacheIndex].chunkType;
            dwFlags = m_hAviSummary.pIdx1Table->pAudioEntries[nCacheIndex].dwFlags;
            dwOffset= m_hAviSummary.pIdx1Table->pAudioEntries[nCacheIndex].dwOffset;
            audioChunksFromStart = m_hAviSummary.pIdx1Table->pAudioEntries[nCacheIndex].dwAudFrameCount;
            audioBytesFromStart = m_hAviSummary.pIdx1Table->pAudioEntries[nCacheIndex].nTotalSizeInBytes;
            tsAudio = (avi_uint64)m_hAviSummary.pIdx1Table->pAudioEntries[nCacheIndex].nTimeStamp;
            dwSize = m_hAviSummary.pIdx1Table->pAudioEntries[nCacheIndex].dwSize;
          }
          else
          {
            memcpy(&trackId,m_ReadBuffer,sizeof(avi_uint16));
            trackId = ascii_2_short_int(&trackId);
            memcpy(&cType,m_ReadBuffer+2,sizeof(avi_uint16));
            memcpy(&dwFlags,m_ReadBuffer+sizeof(avi_uint32),sizeof(avi_uint32));
            memcpy(&dwOffset,m_ReadBuffer+(2*sizeof(avi_uint32)),sizeof(avi_uint32));
            if(m_bByteAdjustedForMOVI)
            {
              dwOffset = (m_nStartOfMovi + dwOffset - m_nBytesToBeAdjustedForMOVI);
            }
            memcpy(&dwSize,m_ReadBuffer+(3*sizeof(avi_uint32)),sizeof(avi_uint32));
          }

          if( (trackId == trid) && ( (!memcmp(&cType,"wb",sizeof(avi_uint16)))||(bFirstEntry) ) )
          {
            avi_audio_info audstrminfo;
            avi_audiotrack_summary_info ainfo;
            memset(&audstrminfo,0,sizeof(avi_audio_info));
            memset(&ainfo,0,sizeof(avi_audiotrack_summary_info));
            if(!bFirstEntry)
            {
              audioBytesFromStart -= dwSize;
              audioChunksFromStart--;
              if( (GetAudioTrackSummaryInfo(trackId,&ainfo)==AVI_SUCCESS)&&
                  (GetAudioInfo(trackId,&audstrminfo) == AVI_SUCCESS) )
              {
                if( (!ainfo.isVbr)&&(ainfo.audioBytesPerSec) )
                {
                  tsAudio = (avi_uint32)((float)audioBytesFromStart /
                                        (float)ainfo.audioBytesPerSec * 1000.0f);
                }
                if( ainfo.isVbr )
                {
                  if(audstrminfo.strhAudio.dwSampleSize > 0)
                  {
                    if(audstrminfo.strfAudio.nBlockAlign && audstrminfo.strhAudio.dwRate)
                    {
                      double val1 = ceil( ( (double)dwSize / audstrminfo.strfAudio.nBlockAlign) );
                      double val2 =  ((double)audstrminfo.strhAudio.dwScale/audstrminfo.strhAudio.dwRate)* 1000.f;
                      tsAudio -=  (uint32)(val1 * val2);
                    }
                  }
                  else
                  {
                    tsAudio = (avi_uint64)(( ( (float)audstrminfo.strhAudio.dwScale/
                                            (float)audstrminfo.strhAudio.dwRate) * 1000.f) * (float)audioChunksFromStart);
                  }
                }
              }//if( (GetAudioTrackSummaryInfo(trackId,&ainfo)==AVI_SUCCESS)&&..
            }//if(!bFirstEntry)
            bFirstEntry = false;
            MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,"searchIDX1Cache tsAudio %d",tsAudio);

            if(tsAudio <= nReposTime)
            {
              retError = AVI_SUCCESS;
              /*
              * Located the correct audio entry/chunk.
              * Load IDX1 entry values into output idx1 entry and return SUCCESS;
              */
              outputEntry->trackId = trid;
              outputEntry->dwFlags = dwFlags;
              outputEntry->dwOffset = dwOffset;
              outputEntry->dwSize = dwSize;
              outputEntry->dwOffsetInIDX1 = (itr + (4 *sizeof(avi_uint32)));

              outputEntry->dwAudFrameCount = audioChunksFromStart;
              outputEntry->nTimeStamp = tsAudio;
              outputEntry->nTotalSizeInBytes = audioBytesFromStart;
              outputEntry->chunkType = AVI_CHUNK_AUDIO;
              m_nCurrAudioSampleInIdx1 = outputEntry->dwAudFrameCount;
              MM_MSG_PRIO5(MM_FILE_OPS, MM_PRIO_LOW,
                              "searchIDX1Cache Matched Audio Entry: trackid %d dwFlags %d dwOffset %d dwSize %d dwOffsetInIDX1 %d",
                              trid,dwFlags,dwOffset,dwSize,outputEntry->dwOffsetInIDX1);

              MM_MSG_PRIO3(MM_FILE_OPS, MM_PRIO_LOW,
                            "searchIDX1Cache Matched Audio Entry: dwAudFrameCount %d TS %d nTotalSizeInBytes %d",
                            outputEntry->dwAudFrameCount,outputEntry->nTimeStamp,audioBytesFromStart);

              return retError;
            }//if(tsAudio <= nReposTime)
          }//if( (trackId == trid) && ( (!memcmp(&cType,"wb",sizeof(avi_uint16)))||(bFirstEntry) ) )
        }//for(avi_uint64 itr = offset; itr < (m_nIdx1Offset+m_nIdx1Size) ; )
      }//end of else if( (!bSearchForward)&& bOkToRepos )
    }
  }//if((chunkType == AVI_CHUNK_AUDIO)&&(bOkToRepos))

  if((chunkType == AVI_CHUNK_VIDEO)&&(bOkToRepos))
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,"searchIDX1Cache seeking video track");
    memset(outputEntry,0,sizeof(avi_idx1_entry));
    if( (!m_hAviSummary.pIdx1Table) ||
        (!m_hAviSummary.pIdx1Table->pKeyVideoEntries) ||
        (!m_hAviSummary.pIdx1Table->nKeyVideoyEntriesInIDX1) )
    {
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,"No Video Entries exist for trackid %d",trid);
      bOkToRepos = false;
    }
    /* If stream contains Idx1 entries then only do following operations */
    else if(bSearchForward)
    {
      //user wants to do FWD
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,"searchIDX1Cache seeking video track FFW");
      avi_uint64 distanceFromLastEntry  = (avi_uint64)
      m_hAviSummary.pIdx1Table->pKeyVideoEntries[m_hAviSummary.pIdx1Table->nKeyVideoyEntriesInIDX1-1].nTimeStamp - nReposTime;
      avi_uint64 distanceFromFirstEntry = nReposTime - (avi_uint64)m_hAviSummary.pIdx1Table->pKeyVideoEntries[0].nTimeStamp;

      if(distanceFromFirstEntry <= distanceFromLastEntry)
      {
        //we have to start searching in fwd direction from first video frame
        entryIndex = 0;
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,"searchIDX1Cache seeking video track searching in fwd dirn from first entry");
        for(avi_uint32 j = entryIndex; (j < (m_hAviSummary.pIdx1Table->nKeyVideoyEntriesInIDX1)) && (bOkToRepos);j++)
        {
          memcpy(outputEntry,&(m_hAviSummary.pIdx1Table->pKeyVideoEntries[j]),sizeof(avi_idx1_entry));
          if( (trid == m_hAviSummary.pIdx1Table->pKeyVideoEntries[j].trackId)&&
              (m_hAviSummary.pIdx1Table->pKeyVideoEntries[j].nTimeStamp >= nReposTime))
          {
            MM_MSG_PRIO6(MM_FILE_OPS, MM_PRIO_MEDIUM,
                        "searchIDX1Cache matched video entry %d video frame# %d dwOffset %d dwOffsetInIDX1 %d dwSize %d TS %d",
                        j,
                        outputEntry->dwVidFrameCount,
                        outputEntry->dwOffset,
                        outputEntry->dwOffsetInIDX1,
                        outputEntry->dwSize,
                        outputEntry->nTimeStamp);
            m_nCurrVideoSampleInIdx1 = outputEntry->dwVidFrameCount;
            retError = AVI_SUCCESS;
            break;
          }
        }//for(avi_uint32 j = entryIndex; (j < (m_hAviSummary.pIdx1Table->nKeyVideoyEntriesInIDX1-1)) && (bOkToRepos);j++)
        if(retError != AVI_SUCCESS)
        {
          *endOfFileReached = true;
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"searchIDX1Cache end of file reached while seeking");
        }
      }//if(distanceFromFirstEntry <= distanceFromLastEntry)
      else
      {
        //we have to start searching in the backward dirn from the last entry
        entryIndex = m_hAviSummary.pIdx1Table->nKeyVideoyEntriesInIDX1-1;
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,"searchIDX1Cache seeking video track searching in backward dirn from last entry");
        for(int j = entryIndex; (j >= 0) && (bOkToRepos);j--)
        {
          memcpy(outputEntry,&(m_hAviSummary.pIdx1Table->pKeyVideoEntries[j]),sizeof(avi_idx1_entry));
          if((j == 0)&& (trid == m_hAviSummary.pIdx1Table->pKeyVideoEntries[j].trackId))
          {
            //This is weird as nReposTime > current playback time, so unless there is some error,
            //we will never hit this condition.print a message and seek to 0th audio sample.
            MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_FATAL,
                        "searchIDX1Cache unexpected condition reached entryIndex %d , nReposTime %d",
                         entryIndex,nReposTime);
            MM_MSG_PRIO6(MM_FILE_OPS, MM_PRIO_HIGH,
                        "searchIDX1Cache matched video entry %d video frame# %d dwOffset %d dwOffsetInIDX1 %d dwSize %d TS %d",
                        j,
                        outputEntry->dwVidFrameCount,
                        outputEntry->dwOffset,
                        outputEntry->dwOffsetInIDX1,
                        outputEntry->dwSize,
                        outputEntry->nTimeStamp);
            m_nCurrVideoSampleInIdx1 = outputEntry->dwVidFrameCount;
            retError = AVI_SUCCESS;
            break;
          }
          else if( (trid == m_hAviSummary.pIdx1Table->pKeyVideoEntries[j].trackId)          &&
                   (m_hAviSummary.pIdx1Table->pKeyVideoEntries[j].nTimeStamp >= nReposTime) &&
                   (m_hAviSummary.pIdx1Table->pKeyVideoEntries[j-1].nTimeStamp < nReposTime)
                 )
          {
            MM_MSG_PRIO6(MM_FILE_OPS, MM_PRIO_MEDIUM,
                         "searchIDX1Cache matched video entry %d video frame# %d dwOffset %d dwOffsetInIDX1 %d dwSize %d TS %d",
                         j,
                         outputEntry->dwVidFrameCount,
                         outputEntry->dwOffset,
                         outputEntry->dwOffsetInIDX1,
                         outputEntry->dwSize,
                         outputEntry->nTimeStamp);
            m_nCurrVideoSampleInIdx1 = outputEntry->dwVidFrameCount;
            retError = AVI_SUCCESS;
            break;
          }
        }//for(int j = entryIndex; (j >= 0) && (bOkToRepos);j--)
        if(retError != AVI_SUCCESS)
        {
          *endOfFileReached = true;
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"searchIDX1Cache end of file reached while seeking");
        }
      }//end of else of if(distanceFromFirstEntry <= distanceFromLastEntry)
    }//if(bSearchForward)
    else
    {
      //user wants to do RWD
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,"searchIDX1Cache seeking video track RW");
      avi_uint64 distanceFromlastEntry = (avi_uint64)
      m_hAviSummary.pIdx1Table->pKeyVideoEntries[m_hAviSummary.pIdx1Table->nKeyVideoyEntriesInIDX1-1].nTimeStamp - nReposTime;
      avi_uint64 distanceFromFirstEntry = nReposTime - (avi_uint64)m_hAviSummary.pIdx1Table->pKeyVideoEntries[0].nTimeStamp;

      if(distanceFromlastEntry <= distanceFromFirstEntry)
      {
        //we have to start searching in backward direction from last video entry
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,"searchIDX1Cache seeking video track searching from last entry in backward dirn");
        entryIndex = m_hAviSummary.pIdx1Table->nKeyVideoyEntriesInIDX1-1;
        for(int j = entryIndex; (j >= 0) && (bOkToRepos);j--)
        {
          if( (trid == m_hAviSummary.pIdx1Table->pKeyVideoEntries[j].trackId)&&
              (m_hAviSummary.pIdx1Table->pKeyVideoEntries[j].nTimeStamp <= nReposTime))
          {
            memcpy(outputEntry,&(m_hAviSummary.pIdx1Table->pKeyVideoEntries[j]),sizeof(avi_idx1_entry));
            MM_MSG_PRIO6(MM_FILE_OPS, MM_PRIO_MEDIUM,
                         "searchIDX1Cache matched video entry %d video frame# %d dwOffset %d dwOffsetInIDX1 %d dwSize %d TS %d",
                         j,
                         outputEntry->dwVidFrameCount,
                         outputEntry->dwOffset,
                         outputEntry->dwOffsetInIDX1,
                         outputEntry->dwSize,
                         outputEntry->nTimeStamp);
            m_nCurrVideoSampleInIdx1 = outputEntry->dwVidFrameCount;
            retError = AVI_SUCCESS;
            break;
          }
        }//for(int j = entryIndex; (j >= 0) && (bOkToRepos);j--)
      }//if(distanceFromlastEntry <= distanceFromFirstEntry)
      else
      {
        //we have to start searching from the first entry in forward direction
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,"searchIDX1Cache seeking video track searching from first entry in fwd dirn");
        entryIndex =  0;
        for(avi_uint32 j = entryIndex; (j < (m_hAviSummary.pIdx1Table->nKeyVideoyEntriesInIDX1)) && (bOkToRepos);j++)
        {

          if ((j == m_hAviSummary.pIdx1Table->nKeyVideoyEntriesInIDX1 - 1 )&&
              (trid == m_hAviSummary.pIdx1Table->pKeyVideoEntries[j].trackId))
          {
            memcpy(outputEntry,&(m_hAviSummary.pIdx1Table->pKeyVideoEntries[j]),sizeof(avi_idx1_entry));
            MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,
                       "This case was probably hit because there was only 1 entry in the table");
            // or we are rewinding to a point which is beyond the last IDX entry
            MM_MSG_PRIO6(MM_FILE_OPS, MM_PRIO_MEDIUM,
                         "searchIDX1Cache matched video entry %d video frame# %d dwOffset %d dwOffsetInIDX1 %d dwSize %d TS %d",
                         j,
                         outputEntry->dwVidFrameCount,
                         outputEntry->dwOffset,
                         outputEntry->dwOffsetInIDX1,
                         outputEntry->dwSize,
                         outputEntry->nTimeStamp);
             m_nCurrVideoSampleInIdx1 = outputEntry->dwVidFrameCount;
             retError = AVI_SUCCESS;
             break;

          }
          else if( (trid == m_hAviSummary.pIdx1Table->pKeyVideoEntries[j].trackId)          &&
                 (m_hAviSummary.pIdx1Table->pKeyVideoEntries[j].nTimeStamp <= nReposTime) &&
                 (m_hAviSummary.pIdx1Table->pKeyVideoEntries[j+1].nTimeStamp > nReposTime) )
          {
            memcpy(outputEntry,&(m_hAviSummary.pIdx1Table->pKeyVideoEntries[j]),sizeof(avi_idx1_entry));
            MM_MSG_PRIO6(MM_FILE_OPS, MM_PRIO_MEDIUM,
                         "searchIDX1Cache matched video entry %d video frame# %d dwOffset %d dwOffsetInIDX1 %d dwSize %d TS %d",
                         j,
                         outputEntry->dwVidFrameCount,
                         outputEntry->dwOffset,
                         outputEntry->dwOffsetInIDX1,
                         outputEntry->dwSize,
                         outputEntry->nTimeStamp);
                         m_nCurrVideoSampleInIdx1 = outputEntry->dwVidFrameCount;
             retError = AVI_SUCCESS;
             break;
          }
        }//for(avi_uint32 j = entryIndex; (j < (m_hAviSummary.pIdx1Table->nKeyVideoyEntriesInIDX1-1)) && (bOkToRepos);j++)
      }//end of else of if(distanceFromlastEntry <= distanceFromFirstEntry)
    }//end of else of if(bSearchForward)
  }//if((chunkType == AVI_CHUNK_VIDEO)&&(bOkToRepos))
#endif
  return retError;
}
/* =============================================================================
FUNCTION:
 aviParser::seekInIDX1

DESCRIPTION:
 Seek the track identified by given trackid.

INPUT/OUTPUT PARAMETERS:
  trackid          : Identifies the track to be repositioned.
  nReposTime       : Target time to seek to.
  outputEntry      : Fills in idx1 entry information once the seek is complete.
  bSearchForward   : Direction to search in IDX1
  chunkType        : Track type
  nSyncFramesToSkip: Number of sync frames to skip in forward or backward direction.

Please note that nReposTime will be used to carry out SEEK only when nSyncFramesToSkip is 0.

RETURN VALUE:
  AVI_SUCCESS if successful otherwise returns appropriate error code.

SIDE EFFECTS:
  None.
=============================================================================*/
aviErrorType aviParser::seekInIDX1(avi_uint32 trid,
                                   avi_uint64 nReposTime,
                                   avi_idx1_entry* outputEntry,
                                   bool bSearchForward,
                                   CHUNK_t chunkType,
                                   int  nSyncFramesToSkip,
                                   bool* endOfFileReached,
                                   bool bSyncToKeyFrame)
{
  aviErrorType retError = AVI_PARSE_ERROR;
  unsigned char* byteData = m_ReadBuffer;

  avi_uint16 cType = 0;
  avi_uint16 trackId = 0;
  avi_uint32 dwFlags = 0;
  avi_uint64 dwOffset = 0;
  avi_uint32 dwSize = 0;

  avi_uint32 audioChunksFromStart = 0;
  avi_uint64 audioBytesFromStart = 0;
  avi_uint64 tsAudio = 0;

  avi_uint32 videoChunksFromStart = 0;
  avi_uint64 tsVideo = 0;

  avi_uint64 offset = m_nCurrentSampleInfoOffsetInIdx1;
  avi_audiotrack_summary_info ainfo;
  avi_audio_info audinfo;
  avi_video_info vinfo;
  int nVideoSyncSkipped = 0;
  int nAudioSyncSkipped = 0;

  if(chunkType == AVI_CHUNK_AUDIO)
  {
    audioChunksFromStart = m_nCurrAudioFrameCount[trid];
    audioBytesFromStart  = m_nCurrAudioPayloadSize[trid];
  }
  if(chunkType == AVI_CHUNK_VIDEO)
  {
    videoChunksFromStart = m_nCurrVideoFrameCount[trid];
  }

  #ifdef FEATURE_FILESOURCE_AVI_PARSER_DEBUG
    MM_MSG_PRIO4(MM_FILE_OPS, MM_PRIO_MEDIUM,
                 "seekInIDX1 trackid %d nReposTime %d nSyncFramesToSkip %d m_nCurrentSampleInfoOffsetInIdx1 %d",
                 trid,nReposTime,nSyncFramesToSkip,m_nCurrentSampleInfoOffsetInIdx1);
    MM_MSG_PRIO3(MM_FILE_OPS, MM_PRIO_MEDIUM,
                 "seekInIDX1 m_nCurrAudioSampleInIdx1 %d m_nCurrVideoSampleInIdx1 %d bSearchForward %d",
                 m_nCurrAudioSampleInIdx1,m_nCurrVideoSampleInIdx1,bSearchForward);
    MM_MSG_PRIO3(MM_FILE_OPS, MM_PRIO_MEDIUM,
                 "seekInIDX1 m_nCurrAudioFrameCount[trid] %d m_nCurrAudioPayloadSize[trid] %d m_nCurrVideoFrameCount[trid] %d",
                 m_nCurrAudioFrameCount[trid],m_nCurrAudioPayloadSize[trid],m_nCurrVideoFrameCount[trid]);
  #endif

  //Make sure Parser is in valid state to carry out Seek.
  if(m_CurrentParserState != AVI_PARSER_SEEK &&
     AVI_PARSER_END_OF_FILE != m_CurrentParserState)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                 "seekInIDX1 called when parser is not in AVI_PARSER_SEEK!!");
    return AVI_FAILURE;
  }
  if(!outputEntry)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL, "seekInIDX1 Failure!!");
    return AVI_FAILURE;
  }
  memset(outputEntry,0,sizeof(avi_idx1_entry));

  /*
  * bSearchForward is based on Current playback time and Reposition time.
  * Thus, is valid only when nSyncFramesToSkip is 0.
  * When nSyncFramesToSkip > 0 and nReposTime is 0,we need skip in forward direction.
  */
  if( (bSearchForward && (nSyncFramesToSkip == 0)) ||
      ((nSyncFramesToSkip > 0)&&(nReposTime == 0)) )
  {
    /*
    * In case of VBR audio, we keep record of the end time stamp of
    * the sample being consumed.
    * That time stamp becomes the start time of the sample pointed by idx1 entry
    * in following iteration.
    */
    bool bRetrieveEndTSFromSampleConsumed = true;
    for(avi_uint64 itr = offset; itr < (m_nIdx1Offset+m_nIdx1Size) ; )
    {
      bool bAdjustVideo = false;
      bool bAdjustAudio = false;

      if(!parserAVICallbakGetData(offset,4 * sizeof(avi_uint32),
        m_ReadBuffer,AVI_READ_BUFFER_SIZE,m_pUserData,&retError))
      {
        return retError;
      }
      offset += (4 *sizeof(avi_uint32));

      memcpy(&trackId,byteData,sizeof(avi_uint16));
      trackId = ascii_2_short_int(&trackId);
      memcpy(&cType,byteData+2,sizeof(avi_uint16));
      memcpy(&dwFlags,byteData+sizeof(avi_uint32),sizeof(avi_uint32));
      memcpy(&dwOffset,byteData+(2*sizeof(avi_uint32)),sizeof(avi_uint32));
      if(m_bByteAdjustedForMOVI)
      {
        dwOffset = (m_nStartOfMovi + dwOffset - m_nBytesToBeAdjustedForMOVI);
      }
      memcpy(&dwSize,byteData+(3*sizeof(avi_uint32)),sizeof(avi_uint32));

      if(  ( (!memcmp(&cType,"dc",sizeof(avi_uint16))) ||(!memcmp(&cType,"db",sizeof(avi_uint16))) ) &&
           (trackId == trid) )
      {
        if( (GetVideoInfo(trid,&vinfo)==AVI_SUCCESS) && (vinfo.strhVideo.dwRate) )
        {
          bAdjustVideo = true;
          tsVideo =    (avi_uint64)( ( (float)vinfo.strhVideo.dwScale/(float)vinfo.strhVideo.dwRate)*
                                     (float)videoChunksFromStart*1000.0f);

          #ifdef FEATURE_FILESOURCE_AVI_PARSER_DEBUG
            MM_MSG_PRIO4(MM_FILE_OPS, MM_PRIO_LOW,"seekInIDX1 VIDEO chunk# %d TS %d Size %d dwFlags %u",
                          videoChunksFromStart,tsVideo,dwSize,dwFlags);
          #endif
          /*
           * When bSyncToKeyFrame is TRUE, parser is been asked to sync to key frame.
           * (dwFlags & AVI_KEY_FRAME_MASK) makes sure current video frame is a
           * sync frame.
           * When bSyncToKeyFrame is FALSE, parser can seek to non key frame
           * provided it's timestamp match the fw/rw logic.
           *
           */
          if( (dwFlags & AVI_KEY_FRAME_MASK) || (!bSyncToKeyFrame) )
          {
            if(dwFlags & AVI_KEY_FRAME_MASK)
            {
              nVideoSyncSkipped++;
            }
            #ifdef FEATURE_FILESOURCE_AVI_PARSER_DEBUG
              MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW,"seekInIDX1 nVideoSyncSkipped %ld", nVideoSyncSkipped);
            #endif
            m_nCurrVideoSampleInIdx1 = videoChunksFromStart;
            outputEntry->trackId = trid;
            outputEntry->dwFlags = dwFlags;
            outputEntry->dwOffset = dwOffset;
            outputEntry->dwSize = dwSize;
            /*
            * OSCL Cache works in fordward direction.
            * Thus, after consuming an entry, 'offset' is
            * pointing to an entry which will be consumed in next iteration.
            * We need to subtract size of(idx1 entry) from 'offset' to
            * pick the one which is consumed.
            */
            outputEntry->dwOffsetInIDX1 = (offset - (4 *sizeof(avi_uint32)));
            outputEntry->dwVidFrameCount = videoChunksFromStart;
            outputEntry->nTimeStamp = tsVideo;
            outputEntry->chunkType = AVI_CHUNK_VIDEO;

            if( ((tsVideo >= nReposTime) &&(nSyncFramesToSkip == 0))  ||
                ((nVideoSyncSkipped == nSyncFramesToSkip)&&(nSyncFramesToSkip > 0))   )
            {
              /*
              * Located the correct video entry/chunk.
              * Load IDX1 entry values into output idx1 entry and return SUCCESS;
              */
              retError = AVI_SUCCESS;

              MM_MSG_PRIO5(MM_FILE_OPS, MM_PRIO_LOW,
              "seekInIDX1 Matched Video Entry: trackid %d dwFlags %d dwOffset %llu dwSize %d dwOffsetInIDX1 %llu",
              trid,dwFlags,dwOffset,dwSize,outputEntry->dwOffsetInIDX1);

              MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_LOW,
              "seekInIDX1 Matched Video Entry: dwVidFrameCount %d TS %llu",
              outputEntry->dwVidFrameCount,outputEntry->nTimeStamp);

              MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_LOW,
                            "seekInIDX1 nVideoSyncSkipped %d nSyncFramesToSkip %d",
                            nVideoSyncSkipped,nSyncFramesToSkip);

              return retError;
            }//if(tsVideo >= nReposTime)
          }//if(dwFlags & AVI_KEY_FRAME_MASK)
        }//if( (GetVideoInfo(trid,&vinfo)==AVI_SUCCESS) && (vinfo.strhVideo.dwRate) )
        else
        {
          MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_FATAL,
                        "seekInIDX1 GetVideoInfo failed for trackid %d",trid);
          return AVI_PARSE_ERROR;
        }
      }//if video chunk
      else if( (!memcmp(&cType,"wb",sizeof(avi_uint16))) && (trackId == trid) && (trid < AVI_MAX_AUDIO_TRACKS)  )
      {
        if( (GetAudioTrackSummaryInfo(trackId,&ainfo)==AVI_SUCCESS) &&
            (GetAudioInfo(trackId,&audinfo)==AVI_SUCCESS) )
        {
          bAdjustAudio = true;
          nAudioSyncSkipped++;
          #ifdef FEATURE_FILESOURCE_AVI_PARSER_DEBUG
            MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW,"seekInIDX1 nAudioSyncSkipped %ld", nAudioSyncSkipped);
          #endif

          if( (!ainfo.isVbr)&&(ainfo.audioBytesPerSec) )
          {
            tsAudio = (avi_uint32)((float)audioBytesFromStart / (float)ainfo.audioBytesPerSec * 1000.0f);
            #ifdef FEATURE_FILESOURCE_AVI_PARSER_DEBUG
              MM_MSG_PRIO3(MM_FILE_OPS, MM_PRIO_LOW,
                            "seekInIDX1 AUDIO TRACK IS CBR TS %d Chunk# %d Size %d",
                            tsAudio,audioChunksFromStart,dwSize);
            #endif
          }
          else if( ainfo.isVbr && ainfo.audioFrequency )
          {
            if(audinfo.strhAudio.dwSampleSize > 0)
            {
              if(bRetrieveEndTSFromSampleConsumed)
              {
                //Retrieve the end time timestamp of the sample consumed during playback
                //which is same as start time stamp of the sample pointed by idx1 entry
                //read above
                tsAudio = m_nParserAudSampleEndTime[trid];
                bRetrieveEndTSFromSampleConsumed = false;
              }
            }
            else
            {
              tsAudio = (avi_uint64)(( ( (float)audinfo.strhAudio.dwScale/
                                      (float)audinfo.strhAudio.dwRate) * 1000.f) * (float)audioChunksFromStart );
            }
            if(audinfo.strhAudio.dwSampleSize > 0)
            {
              if(audinfo.strfAudio.nBlockAlign && audinfo.strhAudio.dwRate)
              {
                double val1 = ceil( ( (double)dwSize / audinfo.strfAudio.nBlockAlign) );
                double val2 =  ((double)audinfo.strhAudio.dwScale/audinfo.strhAudio.dwRate)* 1000.f;
                tsAudio += (uint32)(val1 * val2);
              }
            }
            #ifdef FEATURE_FILESOURCE_AVI_PARSER_DEBUG
              MM_MSG_PRIO3(MM_FILE_OPS, MM_PRIO_LOW,
                            "seekInIDX1 AUDIO TRACK IS VBR TS %d Chunk# %d Size %d",
                            tsAudio,audioChunksFromStart,dwSize);
            #endif
          }
          m_nCurrAudioSampleInIdx1 = audioChunksFromStart;
          outputEntry->trackId = trid;
          outputEntry->dwFlags = dwFlags;
          outputEntry->dwOffset = dwOffset;
          outputEntry->dwSize = dwSize;
          /*
          * OSCL Cache works in fordward direction.
          * Thus, after consuming an entry, 'offset' is
          * pointing to an entry which will be consumed in next iteration.
          * We need to subtract size of(idx1 entry) from 'offset' to
          * pick the one which is consumed.
          */
          outputEntry->dwOffsetInIDX1 = (offset - (4 *sizeof(avi_uint32)));
          outputEntry->dwAudFrameCount = audioChunksFromStart;
          outputEntry->nTimeStamp = tsAudio;
          outputEntry->nTotalSizeInBytes = audioBytesFromStart;
          outputEntry->chunkType = AVI_CHUNK_AUDIO;

          if( (tsAudio >= nReposTime) ||
              ((nAudioSyncSkipped == nSyncFramesToSkip)&&(nSyncFramesToSkip > 0))   )
          {
            /*
            * Located the correct audio entry/chunk.
            * Load IDX1 entry values into output idx1 entry and return SUCCESS;
            */
            retError = AVI_SUCCESS;

            MM_MSG_PRIO5(MM_FILE_OPS, MM_PRIO_LOW,
            "seekInIDX1 Matched Audio Entry: trackid %d dwFlags %d dwOffset %llu dwSize %d dwOffsetInIDX1 %llu",
            trid,dwFlags,dwOffset,dwSize,outputEntry->dwOffsetInIDX1);

            MM_MSG_PRIO3(MM_FILE_OPS, MM_PRIO_LOW,
            "seekInIDX1 Matched Audio Entry: dwAudFrameCount %d TS %llu nTotalSizeInBytes %llu",
            outputEntry->dwAudFrameCount,outputEntry->nTimeStamp,audioBytesFromStart);

            MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_LOW,
                          "seekInIDX1 nAudioSyncSkipped %d nSyncFramesToSkip %d",
                          nAudioSyncSkipped,nSyncFramesToSkip);
            return retError;
          }
        }
        else
        {
          MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_FATAL,
                        "seekInIDX1 GetAudioTrackSummaryInfo failed for trackid %d ",trid);
          return AVI_PARSE_ERROR;
        }
      }
      if(bAdjustVideo)
      {
        videoChunksFromStart++;
      }
      if(bAdjustAudio)
      {
        audioChunksFromStart++;
        audioBytesFromStart += dwSize;
      }
      itr+=(4 *sizeof(avi_uint32));
    }//for(avi_uint64 itr = offset; itr < (m_nIdx1Offset+m_nIdx1Size) ; )
    *endOfFileReached = true;
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"seekInIDX1 end of file reached while seeking");
  }
  else if( ((bSearchForward == false)&&(nSyncFramesToSkip == 0)) ||
           ((nSyncFramesToSkip < 0)&&(nReposTime == 0)) )
  {
    /*
    * bSearchForward is based on Current playback time and Reposition time.
    * Thus, is valid only when nSyncFramesToSkip is 0.
    * When nSyncFramesToSkip < 0 and nReposTime is 0,we need skip in backward direction.
    */

    /*
    * m_nCurrentSampleInfoOffsetInIdx1 is incremented once the
    * sample info/sample is retrieved to point to the next IDX1 entry.
    * Since we read in backward direction when doing rewind,we need to
    * adjust video/audio frame count.
    * The first TS that we will encounter below while doing rewind for audio/video,
    * should be the one that parser has already spitted out.
    * So go to previous entry to match with current audio/video frame count.
    */
    /* If we have not read single entry, then m_nCurrentSampleInfoOffsetInIdx1 value will not be
       updated. In such cases, this check will ensure offset value will not be less than first IDX1
       entry offset */
    if(offset > m_nIdx1Offset)
    {
      offset -= (4 *sizeof(avi_uint32));
    }

    MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_LOW,
                    "bInitialBackWardDirection is TRUE, old offset %llu updated offset %llu",
                    (offset+(4 *sizeof(avi_uint32))),offset);

    bool bRetrieveEndTSFromSampleConsumed = true;
    for(avi_uint64 itr = offset; itr >= m_nIdx1Offset ; )
    {

      if(!readFromIdx1SeekCache(offset,4 * sizeof(avi_uint32),AVI_READ_BUFFER_SIZE,m_ReadBuffer))
      {
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                     "seekInIDX1 AVI_READ_FAILURE");
        retError = AVI_READ_FAILURE;
        flushIdx1SeekCache();
        return retError;
      }
        offset -= (4 *sizeof(avi_uint32));

      memcpy(&trackId,byteData,sizeof(avi_uint16));
      trackId = ascii_2_short_int(&trackId);
      memcpy(&cType,byteData+2,sizeof(avi_uint16));
      memcpy(&dwFlags,byteData+sizeof(avi_uint32),sizeof(avi_uint32));
      memcpy(&dwOffset,byteData+(2*sizeof(avi_uint32)),sizeof(avi_uint32));
      if(m_bByteAdjustedForMOVI)
      {
        dwOffset = (m_nStartOfMovi + dwOffset - m_nBytesToBeAdjustedForMOVI);
      }
      memcpy(&dwSize,byteData+(3*sizeof(avi_uint32)),sizeof(avi_uint32));

      if(  ( (!memcmp(&cType,"dc",sizeof(avi_uint16))) ||(!memcmp(&cType,"db",sizeof(avi_uint16))) ) &&
           (trackId == trid) )
      {
        if(videoChunksFromStart)
        {
          /*As we go backwards in RW, we should decrement videoChunksFromStart since it is already incremented
          in GetNextSampleInfo during playback.*/
          videoChunksFromStart--;
        }
        else
        {
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW,"SeekInIDX1 videoChunksFromStart is 0 ");
        }
        if( (GetVideoInfo(trid,&vinfo)==AVI_SUCCESS) && (vinfo.strhVideo.dwRate) )
        {
          tsVideo =    (avi_uint64)( ( (float)vinfo.strhVideo.dwScale/(float)vinfo.strhVideo.dwRate)*
                                     (float)videoChunksFromStart*1000.0f);

          #ifdef FEATURE_FILESOURCE_AVI_PARSER_DEBUG
            MM_MSG_PRIO4(MM_FILE_OPS, MM_PRIO_LOW,"seekInIDX1 VIDEO chunk# %d TS %d Size %d dwFlags %u",
                          videoChunksFromStart,tsVideo,dwSize,dwFlags);
          #endif

          /*
          * For some clips, first video frame is not marked as KEY Frame.
          * So, don't check for any flag if it's a first video frame(videoChunksFromStart==0).
          * If parser is being asked to sync to non key frame(!bSyncToKeyFrame),
          * don't check any key frame flag.
          */
          if( (dwFlags & AVI_KEY_FRAME_MASK)||
              (videoChunksFromStart == 0)   ||
              (!bSyncToKeyFrame)
            )
          {
            if(dwFlags & AVI_KEY_FRAME_MASK)
            {
              nVideoSyncSkipped--;
            }
            #ifdef FEATURE_FILESOURCE_AVI_PARSER_DEBUG
              MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_LOW,"seekInIDX1 nVideoSyncSkipped %ld TS %ld", nVideoSyncSkipped,tsVideo);
            #endif

            if( ((tsVideo <= nReposTime)&&(nSyncFramesToSkip==0))  ||
                ((nVideoSyncSkipped == nSyncFramesToSkip)&&(nSyncFramesToSkip < 0))
              )
            {
              flushIdx1SeekCache();
              m_nCurrVideoSampleInIdx1 = videoChunksFromStart;

              /*
              * Located the correct video entry/chunk.
              * Load IDX1 entry values into output idx1 entry and return SUCCESS;
              */
              outputEntry->trackId = trid;
              outputEntry->dwFlags = dwFlags;
              outputEntry->dwOffset = dwOffset;
              outputEntry->dwSize = dwSize;

              /*
              * Parser Cache works in backward direction when doing REWIND.
              * Thus, after consuming an entry, 'offset' is
              * pointing to an entry which will be consumed in next iteration as
              * opposed to FORWARD case where we need to subtract
              * size of(idx1 entry) from 'offset' to pick the one which is consumed.
              * Here we need to increment it.
              */
              outputEntry->dwOffsetInIDX1 = (offset + (4 *sizeof(avi_uint32)) );

              outputEntry->dwVidFrameCount = videoChunksFromStart;
              outputEntry->nTimeStamp = tsVideo;
              outputEntry->chunkType = AVI_CHUNK_VIDEO;
              retError = AVI_SUCCESS;

              MM_MSG_PRIO5(MM_FILE_OPS, MM_PRIO_LOW,
              "seekInIDX1 Matched Video Entry: trackid %d dwFlags %d dwOffset %llu dwSize %d dwOffsetInIDX1 %llu",
              trid,dwFlags,dwOffset,dwSize,outputEntry->dwOffsetInIDX1);

              MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_LOW,
              "seekInIDX1 Matched Video Entry: dwVidFrameCount %d TS %llu",
              outputEntry->dwVidFrameCount,outputEntry->nTimeStamp);
              MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_LOW,
                            "seekInIDX1 nVideoSyncSkipped %d nSyncFramesToSkip %d",
                            nVideoSyncSkipped,nSyncFramesToSkip);

              return retError;
            }//if(tsVideo >= nReposTime)
          }//if(dwFlags & AVI_KEY_FRAME_MASK)
        }//if( (GetVideoInfo(trid,&vinfo)==AVI_SUCCESS) && (vinfo.strhVideo.dwRate) )
        else
        {
          MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_FATAL,
                        "seekInIDX1 GetVideoInfo failed for trackid %d",trid);
          return AVI_PARSE_ERROR;
        }
      }//if video chunk
      else if( (!memcmp(&cType,"wb",sizeof(avi_uint16))) && (trackId == trid)&& (trid < AVI_MAX_AUDIO_TRACKS) )
      {
        /*As we go backwards in RW, we should decrement audioBytesFromStart and audioChunksFromStart
        since it is already incremented in GetNextSampleInfo during playback.*/
        if(audioBytesFromStart >= dwSize)
        {
          audioBytesFromStart -= dwSize;
        }
        else
        {
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW,"SeekInIDX1 audioBytesFromStart < dwSize ");
        }
        if(audioChunksFromStart)
        {
          audioChunksFromStart--;
        }
        else
        {
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW,"SeekInIDX1 audioChunksFromStart is 0 ");
        }
        if( (GetAudioTrackSummaryInfo(trackId,&ainfo)==AVI_SUCCESS) &&
            (GetAudioInfo(trackId,&audinfo) == AVI_SUCCESS) )
        {
          nAudioSyncSkipped--;
          #ifdef FEATURE_FILESOURCE_AVI_PARSER_DEBUG
            MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW,"seekInIDX1 nAudioSyncSkipped %ld", nAudioSyncSkipped);
          #endif

          if( (!ainfo.isVbr)&&(ainfo.audioBytesPerSec) )
          {
            tsAudio = (avi_uint32)((float)audioBytesFromStart / (float)ainfo.audioBytesPerSec * 1000.0f);
            #ifdef FEATURE_FILESOURCE_AVI_PARSER_DEBUG
              MM_MSG_PRIO3(MM_FILE_OPS, MM_PRIO_LOW,
                            "seekInIDX1 AUDIO TRACK IS CBR TS %d Chunk# %d Size %d",
                            tsAudio,audioChunksFromStart,dwSize);
            #endif
          }
          else if( ainfo.isVbr && ainfo.audioFrequency )
          {
            if(audinfo.strhAudio.dwSampleSize > 0)
            {
              if(bRetrieveEndTSFromSampleConsumed)
              {
                //Get the end time stamp of the sample that corresponds
                //to idx1 entry read above
                tsAudio = m_nParserAudSampleEndTime[trid];
                bRetrieveEndTSFromSampleConsumed = false;
              }
              //Now subtract duration based on current sample's size
              //to get this sample's start time stamp
              if(audinfo.strfAudio.nBlockAlign && audinfo.strhAudio.dwRate)
              {
                double val1 = ceil( ( (double)dwSize / audinfo.strfAudio.nBlockAlign) );
                double val2 =  ((double)audinfo.strhAudio.dwScale/audinfo.strhAudio.dwRate)* 1000.f;
                uint32 sampleduration =  (uint32)(val1 * val2);
                if(tsAudio > sampleduration)
                {
                  tsAudio -= sampleduration;
                }
              }
            }
            else
            {
              tsAudio = (avi_uint64)(( ( (float)audinfo.strhAudio.dwScale/
                                       (float)audinfo.strhAudio.dwRate) * 1000.f) * (float)audioChunksFromStart );
            }
            MM_MSG_PRIO3(MM_FILE_OPS, MM_PRIO_LOW,
                            "seekInIDX1 AUDIO TRACK IS VBR TS %llu Chunk# %d Size %d",
                            tsAudio,audioChunksFromStart,dwSize);
          }
          if( (tsAudio <= nReposTime)                                            ||
              ((nAudioSyncSkipped == nSyncFramesToSkip)&&(nSyncFramesToSkip < 0))   )
          {
            m_nCurrAudioSampleInIdx1 = audioChunksFromStart;
            flushIdx1SeekCache();
            /*
            * Located the correct audio entry/chunk.
            * Load IDX1 entry values into output idx1 entry and return SUCCESS;
            */
            outputEntry->trackId = trid;
            outputEntry->dwFlags = dwFlags;
            outputEntry->dwOffset = dwOffset;
            outputEntry->dwSize = dwSize;

            /*
            * Parser Cache works in backward direction when doing REWIND.
            * Thus, after consuming an entry, 'offset' is
            * pointing to an entry which will be consumed in next iteration as
            * opposed to FORWARD case where we need to subtract
            * size of(idx1 entry) from 'offset' to pick the one which is consumed.
            * Here we need to increment it.
            */
            outputEntry->dwOffsetInIDX1 = (offset + (4 *sizeof(avi_uint32)) );

            outputEntry->dwAudFrameCount = audioChunksFromStart;
            outputEntry->nTimeStamp = tsAudio;
            outputEntry->nTotalSizeInBytes = audioBytesFromStart;

            outputEntry->chunkType = AVI_CHUNK_AUDIO;
            retError = AVI_SUCCESS;

            MM_MSG_PRIO5(MM_FILE_OPS, MM_PRIO_LOW,
            "seekInIDX1 Matched Audio Entry: trackid %d dwFlags %d dwOffset %llu dwSize %d dwOffsetInIDX1 %llu",
            trid,dwFlags,dwOffset,dwSize,outputEntry->dwOffsetInIDX1);

            MM_MSG_PRIO3(MM_FILE_OPS, MM_PRIO_LOW,
            "seekInIDX1 Matched Audio Entry: dwAudFrameCount %d TS %llu nTotalSizeInBytes %llu",
            outputEntry->dwAudFrameCount,outputEntry->nTimeStamp,audioBytesFromStart);
            MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_LOW,
                          "seekInIDX1 nAudioSyncSkipped %d nSyncFramesToSkip %d",
                          nAudioSyncSkipped,nSyncFramesToSkip);

            return retError;
          }
        }
        else
        {
          MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_FATAL,
                        "seekInIDX1 GetAudioTrackSummaryInfo failed for trackid %d ",trid);
          return AVI_PARSE_ERROR;
        }
      }
      itr-=(4 *sizeof(avi_uint32));
    }//for(avi_uint64 itr = offset; itr >= m_nIdx1Offset ; )
    flushIdx1SeekCache();
  }
  else
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,"seekInIDX1 WEIRD CASE...PLEASE CHECK..");
    MM_MSG_PRIO3(MM_FILE_OPS, MM_PRIO_HIGH,
                  "bSearchForward %d nSyncFramesToSkip %d nReposTime %llu",
                  bSearchForward,nSyncFramesToSkip,nReposTime);
    return AVI_FAILURE;
  }
  return retError;
}

/* ======================================================================
FUNCTION:
  aviParser::randomAccessDenied

DESCRIPTION:
  gets if repositioning is allowed or not.

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
uint8 aviParser::randomAccessDenied()
{
  return (uint8)m_bSeekDenied;
}
/* =============================================================================
FUNCTION:
 aviParser::setParserState

DESCRIPTION:
 Set the Parser state.

INPUT/OUTPUT PARAMETERS:
  aviParserState          : Identifies the track to be repositioned.
  retError                : Target time to seek to.

RETURN VALUE:
  AVI_SUCCESS if successful otherwise returns appropriate error code.

SIDE EFFECTS:
  None.
=============================================================================*/
bool aviParser::setParserState(aviParserState state,aviErrorType* retError)
{
  bool retValue = true;
  if(state == AVI_PARSER_READ_FAILED)
  {
    if(m_bHttpStreaming)
    {
      *retError = AVI_DATA_UNDERRUN;
      state = AVI_PARSER_DATA_UNDERRUN;
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"setParserState AVI_PARSER_DATA_UNDERRUN");
    }
    else
    {
      *retError = AVI_READ_FAILURE;
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,"setParserState AVI_PARSER_READ_FAILED");
    }
  }
  else
  {
    *retError = AVI_SUCCESS;
  }
  m_CurrentParserState = state;
  return retValue;
}
/* =============================================================================
FUNCTION:
 aviParser::parserAVICallbakGetData

DESCRIPTION:
 Seek the track identified by given trackid.

INPUT/OUTPUT PARAMETERS:
  nOffset                 Offset of the requested data (from beginning),
  nNumBytesRequest        Size of the requested data (in bytes).
  pData                  Pointer to the buffer for filling in the ASF data
  u32UserData             Extra info From App. Given by user in aviin_open
  retError                To specify error (if any)

Please note that nReposTime will be used to carry out SEEK only when nSyncFramesToSkip is 0.

RETURN VALUE:
  AVI_SUCCESS if successful otherwise returns appropriate error code.

SIDE EFFECTS:
  None.
=============================================================================*/
avi_int32 aviParser::parserAVICallbakGetData (avi_int64         nOffset,
                                              avi_uint32        nNumBytesRequest,
                                              unsigned char     *pData,
                                              avi_int32         nMaxSize,
                                              void*             pUserData,
                                              aviErrorType*     retError)
{
  avi_int32 nread = nNumBytesRequest;
  if(!AVICallbakGetData(nOffset,nNumBytesRequest,pData,nMaxSize,
    pUserData))
  {
    if(m_nFileSize <= (avi_uint64)nOffset)
      setParserState(AVI_PARSER_END_OF_FILE,retError);
    else
      setParserState(AVI_PARSER_READ_FAILED,retError);
    nread = 0;
  }
  return nread;
}
/* =============================================================================
FUNCTION:
 aviParser::getTimeStampFromSubtitle

DESCRIPTION:
 Get timestamp from subtitle header.

INPUT/OUTPUT PARAMETERS:
  buf          : Buffer with subtitle header

RETURN VALUE:
  avi_int64 timestamp from the subtitle header.

SIDE EFFECTS:
  None.
=============================================================================*/
avi_int64 aviParser::getTimeStampFromSubtitle(avi_uint8* buf, CHUNK_t chunkType)
{
  avi_uint64 temp = 0;
  avi_uint64 timeStamp = 0;

 if(buf)
 {
   if(chunkType == AVI_CHUNK_BITMAP_CAPTION)
   {
     memcpy(&temp,buf+1,sizeof(uint16));
     timeStamp = atoi((char*)&temp) * 60 * 60 * 1000;
     memcpy(&temp,buf+4,sizeof(uint16));
     timeStamp = timeStamp + (atoi((char*)&temp) * 60 * 1000);
     memcpy(&temp,buf+7,sizeof(uint16));
     timeStamp = timeStamp + (atoi((char*)&temp) * 1000);
     memcpy(&temp,buf+10,3*sizeof(uint8));
     timeStamp = timeStamp + (atoi((char*)&temp));
   }
   else
   {
     MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"getTimeStampFromSubtitle not yet implemented");
   }
 }
 return timeStamp;
}
#endif//FEATURE_FILESOURCE_AVI
