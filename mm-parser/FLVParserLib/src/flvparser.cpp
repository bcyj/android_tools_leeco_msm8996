/* =======================================================================
                              FLVParser.cpp
DESCRIPTION
  Meaningful description of the definitions contained in this file.
  Description must specify if the module is portable specific, mobile
  specific, or common to both, and it should alert the reader if the
  module contains any conditional definitions which tailors the module to
  different targets.

Copyright (c) 2012-2014 Qualcomm Technologies Inc, All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
========================================================================== */

/* =======================================================================
                             PERFORCE HEADER
$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/FLVParserLib/main/latest/src/flvparser.cpp#9 $
========================================================================== */
#include "flvparserconstants.h"
#include "flvparser.h"
#include "string.h"
#include "MMDebugMsg.h"
#include "MMMemory.h"
#include "MMMalloc.h"
#include "math.h"
#include "parserdatadef.h"
#include <stdio.h>
#include "parserdatadef.h"
#define DEF_DATA_BUF_SIZE    16000

extern uint32 FLVCallbakGetData (uint64, uint32,unsigned char*,uint32,void*);

/*! ======================================================================
@brief    FLVParser constructor

@detail	Instantiates FLVParser to MKAV file.

@param[in]  pUData    APP's UserData.This will be passed back when this parser invokes the callbacks.
@return    N/A
@note      None
========================================================================== */
FLVParser::FLVParser(void* pUData,uint64 fsize,bool bAudio)
{
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,"FLVParser");
  InitData();
  m_pUserData   = pUData;
  m_ullFileSize = fsize;
  m_bPlayAudio  = bAudio;
}
/*! ======================================================================
@brief    InitData

@detail   Initializes class members to their default values

@param[in] None
@return    none
@note      None
========================================================================== */
void FLVParser::InitData()
{
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW,"InitData");
  m_pFLVHdr              = NULL;
  m_pUserData            = NULL;
  m_pucDataBuffer        = NULL;
  m_pAudioInfo           = NULL;
  m_pVideoInfo           = NULL;
  m_pMetadataInfo        = NULL;
  m_pIndexTable          = NULL;
  m_pucSeekDataBuffer    = NULL;
  m_ullCurrOffset        = 0;
  m_ullFileSize          = 0;
  m_ullClipDuration      = 0;
  m_ulDataBufSize        = 0;
  m_ulMaxIndexTableEntry = 0;
  m_ullIndexTableDelta   = 0;
  m_ucNoStreams          = 0;
  m_ucNoAudioStreams     = 0;
  m_ucNoVideoStreams     = 0;
  m_bPlayAudio           = false;
}

/*! ======================================================================
@brief    FLVParser destructor

@detail   FLVParser destructor

@param[in] N/A
@return    N/A
@note      None
========================================================================== */
FLVParser::~FLVParser()
{
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,"~FLVParser");
  if(m_pucDataBuffer)
  {
    MM_Free(m_pucDataBuffer);
  }
  if (m_pucSeekDataBuffer)
  {
    MM_Free(m_pucSeekDataBuffer);
  }
  if(m_pFLVHdr)
  {
    MM_Free(m_pFLVHdr);
  }
  if (m_pMetadataInfo)
  {
    MM_Free(m_pMetadataInfo);
  }
  if (m_pIndexTable)
  {
    MM_Free(m_pIndexTable);
  }
  if (m_pAudioInfo)
  {
    if (m_pAudioInfo->pucCodecConfigBuf)
    {
      MM_Free(m_pAudioInfo->pucCodecConfigBuf);
    }
    MM_Free(m_pAudioInfo);
  }
  if (m_pVideoInfo)
  {
    if (m_pVideoInfo->psCodecConfig)
    {
      if (m_pVideoInfo->psCodecConfig->pPicParam)
      {
        uint32 ulIndex = 0;
        for(;ulIndex < m_pVideoInfo->psCodecConfig->ucNumPicParams; ulIndex++)
        {
          if (m_pVideoInfo->psCodecConfig->pPicParam[ulIndex].pucNALBuf)
          {
            MM_Free(m_pVideoInfo->psCodecConfig->pPicParam[ulIndex].pucNALBuf);
          }
        }
        MM_Free(m_pVideoInfo->psCodecConfig->pPicParam);
      }
      if (m_pVideoInfo->psCodecConfig->pSeqParam)
      {
        uint32 ulIndex = 0;
        for(;ulIndex < m_pVideoInfo->psCodecConfig->ucNumSeqParams; ulIndex++)
        {
          if (m_pVideoInfo->psCodecConfig->pSeqParam[ulIndex].pucNALBuf)
          {
            MM_Free(m_pVideoInfo->psCodecConfig->pSeqParam[ulIndex].pucNALBuf);
          }
        }
        MM_Free(m_pVideoInfo->psCodecConfig->pSeqParam);
      }
      MM_Free(m_pVideoInfo->psCodecConfig);
    }
    MM_Free(m_pVideoInfo);
  }
}

/*! ======================================================================
@brief  Starts Parsing the FLV file.

@detail	This function starts parsing the mkav file.
        Upon successful parsing, user can retrieve total number of streams and
        stream specific information.

@param[in] N/A
@return    FLVPARSER_SUCCESS is parsing is successful,
           otherwise returns appropriate error.
@note      StartParsing needs to be called before retrieving any elementary
           stream specific information.
========================================================================== */
PARSER_ERRORTYPE FLVParser::StartParsing(void)
{
  PARSER_ERRORTYPE retError = PARSER_ErrorDefault;
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM, "FLVParser::StartParsing");

  m_pucDataBuffer = (uint8*)MM_Malloc(DEF_DATA_BUF_SIZE);
  if(m_pucDataBuffer)
  {
    memset(m_pucDataBuffer,0,DEF_DATA_BUF_SIZE);
    m_ulDataBufSize = DEF_DATA_BUF_SIZE;
    //make sure file/stream starts with valid FLV header
    if(PARSER_ErrorNone == ParseFLVHeader(m_ullCurrOffset) )
    {
      //parses if audio is present in the clip.
      //If audio is not present, ParseAudioInfo will return SUCCESS.
      if((!m_pFLVHdr->ucAudioPresentFlag) ||
         (PARSER_ErrorNone == ParseAudioInfo(m_ullCurrOffset)) )
      {
        //parses if video is present in the clip.
        //If video is not present, ParseVideoInfo will return SUCCESS.
        if((!m_pFLVHdr->ucVideoPresentFlag) ||
           (PARSER_ErrorNone == ParseVideoInfo(m_ullCurrOffset)) )
        {
          if( (m_pAudioInfo || m_pVideoInfo) )
          {
             MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,
                         "FLVParser::StartParsing FLVPARSER_READY");
             retError        = PARSER_ErrorNone;
             m_ullCurrOffset = m_pFLVHdr->ulDataStartOffset;

             if ((m_bPlayAudio) && (m_pAudioInfo))
             {
               m_ullCurrOffset = m_pAudioInfo->ulAudStartOffset;
             }
             else if ((!m_bPlayAudio) && (m_pVideoInfo))
             {
               m_ullCurrOffset = m_pVideoInfo->ulVidStartOffset;
             }
          } // if( (m_pFLVHdr)&& (m_pAudioInfo || m_pVideoInfo) )
        } // if(PARSER_ErrorNone == ParseVideoInfo(m_nCurrOffset) )
      } // if(PARSER_ErrorNone == ParseAudioInfo(m_nCurrOffset) )
    } // if(PARSER_ErrorNone == ParseFLVHeader(m_nCurrOffset) )
  }//if(m_pDataBuffer)
  else
  {
    retError = PARSER_ErrorMemAllocFail;
  }
  return retError;
}

/* =============================================================================
FUNCTION:
 FLVParser::GetTrackWholeIDList

DESCRIPTION:
Returns trackId list for all the tracks in given clip.

INPUT/OUTPUT PARAMETERS:

RETURN VALUE:

SIDE EFFECTS:
  None.
=============================================================================*/
uint32 FLVParser::GetTrackWholeIDList(uint32* idList)
{
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,"FLVParser::GetTrackWholeIDList");
  if(!idList)
  {
    return 0;
  }
  int ntracks = GetTotalNumberOfTracks();
  int nindex = 0;
  if(ntracks)
  {
    if(m_pAudioInfo)
    {
      idList[nindex++]= m_pAudioInfo->ucTrackId;
    }
    if(m_pVideoInfo)
    {
      idList[nindex++]= m_pVideoInfo->ulTrackId;
    }
  }
  return ntracks;
}
/* =============================================================================
FUNCTION:
 FLVParser::GetTrackCodecType

DESCRIPTION:
Returns track codec type for given track id

INPUT/OUTPUT PARAMETERS:

RETURN VALUE:
 Success if track id is valid otherwise, returns invalid parameter.
SIDE EFFECTS:
  None.
=============================================================================*/
FLVMediaCodecType FLVParser::GetTrackCodecType(uint32 ulTrackID)
{
  FLVMediaCodecType eCodecType = FLV_UNKNOWN_CODEC;

  if(GetTotalNumberOfTracks())
  {
    if((m_pAudioInfo) && (m_pAudioInfo->ucTrackId == ulTrackID))
    {
      eCodecType = m_pAudioInfo->eAudioCodec;
    }
    if((m_pVideoInfo) && (m_pVideoInfo->ulTrackId == ulTrackID))
    {
      eCodecType = m_pVideoInfo->eVideoCodec;
    }
  }
  MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_LOW,"GetTrackCodecType for track %lu is %d",
               ulTrackID, eCodecType);
  return eCodecType;
}
/* =============================================================================
FUNCTION:
 FLVParser::GetTrackType

DESCRIPTION:
Returns track type for given track id

INPUT/OUTPUT PARAMETERS:

RETURN VALUE:
 Success if track id is valid otherwise, returns invalid parameter.
SIDE EFFECTS:
  None.
=============================================================================*/
FLVTrackType  FLVParser::GetTrackType(uint32 ulTrackID)
{
  FLVTrackType eTrackType = FLV_TRACK_TYPE_UNKNOWN;

  if(GetTotalNumberOfTracks())
  {
    if((m_pAudioInfo) && (m_pAudioInfo->ucTrackId == ulTrackID))
    {
      eTrackType = FLV_TRACK_AUDIO;
    }
    if((m_pVideoInfo) && (m_pVideoInfo->ulTrackId == ulTrackID))
    {
      eTrackType = FLV_TRACK_VIDEO;
    }
  }
  MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_LOW,"GetTrackType for track %lu is %d",
               ulTrackID, eTrackType);
  return eTrackType;
}

/* =============================================================================
FUNCTION:
 FLVParser::GetTrackBufferSize

DESCRIPTION:
Returns the buffer size needed for given track id

INPUT/OUTPUT PARAMETERS:

RETURN VALUE:
 returns buffer size needed.
SIDE EFFECTS:
  None.
=============================================================================*/
uint32  FLVParser::GetTrackBufferSize(uint32 ulTrackId)
{
  uint32 ulBufSize = 0;
  if ((m_pVideoInfo) && (m_pVideoInfo->ulTrackId == ulTrackId) )
  {
    //! Init with highest possible values
    uint32 ulHeight = 1080;
    uint32 ulWidth  = 1920;
    if(m_pMetadataInfo)
    {
      ulHeight = m_pMetadataInfo->ulVidFrameHeight;
      ulWidth  = m_pMetadataInfo->ulVidFrameWidth;
    }
    ulBufSize = (uint32)((ulHeight * ulWidth) * 0.75);
  }
  else if (m_pAudioInfo && m_pAudioInfo->ucTrackId == ulTrackId)
  {
    ulBufSize = DEF_AUDIO_BUFF_SIZE;
  }

  MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_MEDIUM,
               "FLVParser::GetTrackBufferSize for track %lu returns %lu",
               ulTrackId, ulBufSize);
  return ulBufSize;
}

/* ============================================================================
  @brief  Function to return clip duration value

  @details    Function to return clip duration value

  @param[in]      None

  @return     clip Duration value in milli-sec.
  @note       None.
============================================================================ */
uint64 FLVParser::GetClipDurationInMsec()
{
  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
               "FLVParser::GetClipDurationInMsec %llu", m_ullClipDuration);
  return m_ullClipDuration;
}

/* ============================================================================
  @brief  Reads one Sample/Frame at a time.

  @details    This function is used to read one media sample/frame at a time
              from input file/stream.

  @param[in]      trackId       Track Id number.
  @param[in/out]  pucDataBuf    Buffer to read sample.
  @param[in]      ulBufSize     Size of the input buffer.
  @param[in/out]  plBytesRead   Number of bytes read into buffer.
  @param[in/out]  psampleInfo   Structure to store sample properties.

  @return     PARSER_ErrorNone indicating sample read successfully.
              Else, it will report corresponding error.
  @note       BufferSize should be more than maximum frame size value.
============================================================================ */
PARSER_ERRORTYPE FLVParser::GetCurrentSample(uint32               ulTrackId,
                                          uint8*                  pucDataBuf,
                                          uint32                  ulMaxBufSize,
                                          int32*                  plBytesRead,
                                          FLVStreamSampleInfo*    pSampleInfo)
{
  PARSER_ERRORTYPE retError = PARSER_ErrorDefault;
  FLVTrackType eTrackType   = GetTrackType(ulTrackId);
  FLVTagInfo tagInfo;

  //Validate input parameters
  if( (FLV_TRACK_TYPE_UNKNOWN != eTrackType )&& (plBytesRead) &&
      (pucDataBuf) && pSampleInfo )
  {
    memset(&tagInfo, 0, sizeof(FLVTagInfo));
    retError = ParseFLVTag(m_ullCurrOffset,&tagInfo);
    while(PARSER_ErrorNone == retError)
    {
      //! Check if tag parsed is matched with destination requirements
      if(((TAG_TYPE_AUDIO  == tagInfo.ucTagType ) &&
          (FLV_TRACK_AUDIO == eTrackType )) ||
         ((TAG_TYPE_VIDEO  == tagInfo.ucTagType ) &&
          (FLV_TRACK_VIDEO == eTrackType )))
      {
        uint8* pDataBuf     = pucDataBuf;
        uint32 ulDataRead   = 0;
        uint64 ullTagOffset = 0;

        //! Check if input buffer size is sufficient to read sample data
        if (ulMaxBufSize < tagInfo.ulPayloadSize)
        {
          *plBytesRead = tagInfo.ulPayloadSize;
          retError     = PARSER_ErrorInsufficientBufSize;
          break;
        }

        /* If codec type is AVC/H264, Parser need to replace NALU size with
           start code (0x00 00 00 01). To do this change, Parser will
           allocate extra memory inside and do the memcopy operations.
           There is one buffer to process metadata, Same buffer is used for
           this purpose as well. */
        if ((FLV_TRACK_VIDEO == eTrackType) && (m_pVideoInfo) &&
            (FLV_H264 == m_pVideoInfo->eVideoCodec) &&
            (m_pVideoInfo->psCodecConfig))
        {
          if (m_ulDataBufSize < ulMaxBufSize)
          {
            if (!m_pucDataBuffer)
            {
              m_pucDataBuffer = (uint8*)MM_Malloc(ulMaxBufSize);
              m_ulDataBufSize = ulMaxBufSize;
            }
            else
            {
              m_pucDataBuffer = (uint8*)MM_Realloc(m_pucDataBuffer, ulMaxBufSize);
              m_ulDataBufSize = ulMaxBufSize;
            }
          }
          //! Read frame data into class variable
          if (m_pucDataBuffer)
          {
            pDataBuf = m_pucDataBuffer;
          }
          else
          {
            MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                        "FLVParser::GetCurrentSample malloc failed");
            retError = PARSER_ErrorMemAllocFail;
            break;
          }
        }
        //! Tag Data start offset
        ullTagOffset = tagInfo.ullOffset + tagInfo.ucTagHeaderSize;
        //! Read media sample data from file system/stream cache
        ulDataRead = FLVCallbakGetData(ullTagOffset,
                                       tagInfo.ulPayloadSize,
                                       pDataBuf, ulMaxBufSize,
                                       m_pUserData);
        if(ulDataRead == tagInfo.ulPayloadSize)
        {
          uint32 ulNALLenMinusOne = 4;
          *plBytesRead = tagInfo.ulPayloadSize;
          pSampleInfo->ullSampleTime = tagInfo.ulTime |
                              (tagInfo.ucExtendedTimeStamp << 24);
          pSampleInfo->ullSampleTime+= tagInfo.ulCompositionTime;
          pSampleInfo->ullSize       = tagInfo.ulPayloadSize;
          pSampleInfo->bsync         = true;

          //! Call utility function to replace NALUnit sizes with start code
          if ((FLV_TRACK_VIDEO == eTrackType) && (m_pVideoInfo) &&
              (FLV_H264 == m_pVideoInfo->eVideoCodec) &&
              (m_pVideoInfo->psCodecConfig))
          {
            pSampleInfo->bsync = false;
            ulNALLenMinusOne = m_pVideoInfo->psCodecConfig->ucNALLengthMinusOne;
            ulNALLenMinusOne++;
            //! Check if it is key frame or not
            (void)IsKeyFrame(pDataBuf, &tagInfo, ulDataRead);
            *plBytesRead = UpdateAVC1SampleWithStartCode(ulNALLenMinusOne,
                                                         ulDataRead,
                                                         pucDataBuf,
                                                         pDataBuf);
            if (FLV_KEY_FRAME == tagInfo.eFrameType)
            {
              pSampleInfo->bsync = true;
            }
          }

          //! Update Seek Index Table
          UpdateIndexTable(pSampleInfo->ullSampleTime, &tagInfo);
          m_ullCurrOffset = NEXT_TAG_OFFSET(tagInfo.ullOffset,
                                            tagInfo.ulTagDataSize);
          break;
        }
        else
        {
          retError = PARSER_ErrorEndOfFile;
          MM_MSG_PRIO(MM_FILE_OPS,MM_PRIO_HIGH,"GetCurrentSample Read failed");
          break;
        }
      }
      m_ullCurrOffset = NEXT_TAG_OFFSET(tagInfo.ullOffset,
                                        tagInfo.ulTagDataSize);
      retError        = ParseFLVTag(m_ullCurrOffset,&tagInfo);
    }//while(PARSER_ErrorNone == retError)
  }//if( (tracktype != FLV_TRACK_TYPE_UNKNOWN)&& nBytesRead && dataBuffer && psampleInfo )
  return retError;
}

/*! =========================================================================
@brief  Repositions given track to specified time

@detail Seeks given track in forward/backward direction to specified time

@param[in]
 trackid: Identifies the track to be repositioned.
 nReposTime:Target time to seek to
 nCurrPlayTime: Current playback time
 sample_info: Sample Info to be filled in if seek is successful
 canSyncToNonKeyFrame: When true, video can be repositioned to non key frame
 nSyncFramesToSkip:Number of sync samples to skip. Valid only if > or < 0.

@return    FLVPARSER_SUCCESS if successful other wise returns FLVPARSER_FAIL
@note      None.
=============================================================================*/
PARSER_ERRORTYPE FLVParser::Seek(uint32 ulTrackid,
                                 uint64 ullReposTime,
                                 uint64 ullCurrPlayTime,
                                 FLVStreamSampleInfo* pSampleInfo,
                                 bool bForward,
                                 bool canSyncToNonKeyFrame,
                                 int nSyncFramesToSkip)
{
  MM_MSG_PRIO3(MM_FILE_OPS, MM_PRIO_MEDIUM,
               "FLVParser::Seek id %lu nReposTime %llu nCurrPlayTime %llu",
               ulTrackid, ullReposTime, ullCurrPlayTime);
  MM_MSG_PRIO3(MM_FILE_OPS, MM_PRIO_LOW,
               "FLVParser::Seek bForward %d canSyncToNonKeyFrame %d nSyncFramesToSkip %d",
               bForward, canSyncToNonKeyFrame, nSyncFramesToSkip);

  PARSER_ERRORTYPE retError  = PARSER_ErrorDefault;
  uint64           ullOffset = m_ullCurrOffset;
  uint64           ullTime   = 0;
  FLVTrackType     eTrackType= GetTrackType(ulTrackid);
  uint32           ulBufSize = (uint32)MIN_MEDIA_FRAME_SIZE;
  FLVTagInfo       TagInfo;
  FLVIndexTable    prevIndexEntry;
  memset(&prevIndexEntry, 0, sizeof(FLVIndexTable));
  memset(&TagInfo, 0, sizeof(FLVTagInfo));

  //! Calculate Buffer Size based on height and width
  if (m_pMetadataInfo)
  {
    ulBufSize = (uint32)(m_pMetadataInfo->ulVidFrameHeight *
                         m_pMetadataInfo->ulVidFrameWidth  * 0.75);
  }
  /* If reposition timestamp is ZERO, the seek to first frame. */
  if (0 == ullReposTime)
  {
    m_ullCurrOffset = m_pFLVHdr->ulDataStartOffset;
    //! If input track id is audio, then update startOffset value according
    if ((m_bPlayAudio) && (m_pAudioInfo) && (FLV_TRACK_AUDIO == eTrackType))
    {
      m_ullCurrOffset = m_pAudioInfo->ulAudStartOffset;
    }
    //! If input track id is video, then update startOffset value according
    else if ((!m_bPlayAudio) && (m_pVideoInfo) &&
             (FLV_TRACK_VIDEO == eTrackType))
    {
      m_ullCurrOffset = m_pVideoInfo->ulVidStartOffset;
    }
    return PARSER_ErrorNone;
  }

  TagInfo.ullOffset = m_ullCurrOffset;
  //! If index table is available, use that to go to nearest entry
  retError  = GetClosestTagPosn(&TagInfo, bForward, ullReposTime);
  ullOffset = TagInfo.ullOffset;

  MM_MSG_PRIO3(MM_FILE_OPS, MM_PRIO_MEDIUM,
               "FLVParser::Seek for track %lu starts @ %llu, direction %d",
               ulTrackid, ullOffset, bForward);

  /* This buffer is used to check whether current frame is key frame or not.
     FLV Parser currently supports H264 codec only. */
  if ((!m_pucSeekDataBuffer) &&
      (FLV_TRACK_VIDEO == eTrackType) && (m_pVideoInfo) &&
      (FLV_H264 == m_pVideoInfo->eVideoCodec))
  {
    m_pucSeekDataBuffer = (uint8*)MM_Malloc(ulBufSize);
  }
  /* Parse Tags sequentially. */
  while(PARSER_ErrorNone == retError)
  {
    /* Check whether frame is key frame or not for H264 codec.
       If Frame type is KEY Frame, it need not be correct.
       Ensure that frame is Key frame or not. */
    if ((m_pucSeekDataBuffer) && (TAG_TYPE_VIDEO == TagInfo.ucTagType) &&
        (FLV_KEY_FRAME == TagInfo.eFrameType))
    {
      uint64 ullTagOffset = TagInfo.ullOffset + TagInfo.ucTagHeaderSize;
      //! Read media sample data from file system/stream cache
      uint32 ulDataRead = FLVCallbakGetData(ullTagOffset,
                                            TagInfo.ulPayloadSize,
                                            m_pucSeekDataBuffer, ulBufSize,
                                            m_pUserData);
      retError = IsKeyFrame(m_pucSeekDataBuffer, &TagInfo, ulDataRead);
    }
    if((TAG_TYPE_AUDIO == TagInfo.ucTagType && FLV_TRACK_AUDIO == eTrackType)||
       (TAG_TYPE_VIDEO == TagInfo.ucTagType && FLV_TRACK_VIDEO == eTrackType))
    {
      //! Calculate sample time and update entry in Index table
      ullTime  = TagInfo.ulTime | (TagInfo.ucExtendedTimeStamp << 24);
      ullTime += TagInfo.ulCompositionTime;
      UpdateIndexTable(ullTime, &TagInfo);

      bool bFoundSample = CheckCurrentSample(TagInfo, prevIndexEntry, bForward,
                                             ullReposTime, pSampleInfo);

      //! If closest seek entry is found, break the loop
      if (bFoundSample)
      {
        break;
      }

      /* Store most recent entry details, Parser will use these details to
         seek to the closest entry. */
      if ((TAG_TYPE_AUDIO == TagInfo.ucTagType ) ||
          (FLV_KEY_FRAME  == TagInfo.eFrameType))
      {
        prevIndexEntry.ullTagOffset = ullOffset;
        prevIndexEntry.ullTimeStamp = ullTime;
        prevIndexEntry.ucTagType   = TagInfo.ucTagType;
        MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_LOW,
                     "FLVParser::Seek prev entry time %llu offset %llu",
                     ullTime, ullOffset);
      }
    }
    //! Depending on Seek direction update offset value
    if (bForward)
      ullOffset = NEXT_TAG_OFFSET(TagInfo.ullOffset, TagInfo.ulTagDataSize);
    else if(ullOffset > (TagInfo.ulPrevTagSize))
      ullOffset = ullOffset - (TagInfo.ulPrevTagSize + sizeof(uint32));
    else
      break;
    //! Parse Next Tag
    retError = ParseFLVTag(ullOffset,&TagInfo);
  }
  /* If required element is not found, then update seek time and position to
     the most recent entry found. */
  if ((PARSER_ErrorNone != retError) && (prevIndexEntry.ullTagOffset))
  {
    m_ullCurrOffset              = prevIndexEntry.ullTagOffset;
    pSampleInfo->ullSampleTime   = prevIndexEntry.ullTimeStamp;
    pSampleInfo->ullSampleOffset = prevIndexEntry.ullTagOffset;
    retError = PARSER_ErrorNone;
  }
  return retError;
}

/* =============================================================================
FUNCTION:
 FLVParser::GetVideoWidth

DESCRIPTION:
Returns video width for given id
INPUT/OUTPUT PARAMETERS:

RETURN VALUE:
 Video width
SIDE EFFECTS:
  None.
=============================================================================*/
uint32 FLVParser::GetVideoWidth(uint32 /*ulTrackID*/)
{
  uint32 ulWidth = 0;
  if (m_pMetadataInfo && m_pMetadataInfo->ulVidFrameWidth)
  {
    ulWidth = m_pMetadataInfo->ulVidFrameWidth;
  }
  return ulWidth;
}
/* =============================================================================
FUNCTION:
 FLVParser::GetVideoHeight

DESCRIPTION:
Returns video height for given id
INPUT/OUTPUT PARAMETERS:

RETURN VALUE:
 Video height
SIDE EFFECTS:
  None.
=============================================================================*/
uint32 FLVParser::GetVideoHeight(uint32 /*ulTrackID*/)
{
  uint32 ulHeight = 0;
  if (m_pMetadataInfo && m_pMetadataInfo->ulVidFrameHeight)
  {
    ulHeight = m_pMetadataInfo->ulVidFrameHeight;
  }
  return ulHeight;
}
/* =============================================================================
FUNCTION:
 FLVParser::GetAudioSamplingFrequency

DESCRIPTION:
Returns sampling frequency for given id
INPUT/OUTPUT PARAMETERS:

RETURN VALUE:
 Sampling frequency
SIDE EFFECTS:
  None.
=============================================================================*/
uint32 FLVParser::GetAudioSamplingFrequency(uint32 ulTrackID)
{
  uint32 ulSampFreq = 0;
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"GetAudioSamplingFrequency");
  if(GetTotalNumberOfTracks())
  {
    if((m_pAudioInfo) && (m_pAudioInfo->ucTrackId == ulTrackID))
    {
      ulSampFreq = m_pAudioInfo->ulSamplingRate;
    }
  }
  return ulSampFreq;
}
/* =============================================================================
FUNCTION:
 FLVParser::GetNumberOfAudioChannels

DESCRIPTION:
Returns number of channels for given id
INPUT/OUTPUT PARAMETERS:

RETURN VALUE:
 Number of channels
SIDE EFFECTS:
  None.
=============================================================================*/
uint8  FLVParser::GetNumberOfAudioChannels(uint32 ulTrackID)
{
  uint8 ucNumChannels = 0;
  if(GetTotalNumberOfTracks())
  {
    if((m_pAudioInfo) && (m_pAudioInfo->ucTrackId == ulTrackID))
    {
      ucNumChannels = m_pAudioInfo->ucNumChannels;
    }
  }
  return ucNumChannels;
}

/* =============================================================================
FUNCTION:
 FLVParser::GetVideoFrameRate

DESCRIPTION:
Returns video frame rate for given id
INPUT/OUTPUT PARAMETERS:

RETURN VALUE:
 Returns video frame rate for given id
SIDE EFFECTS:
  None.
=============================================================================*/
float FLVParser::GetVideoFrameRate(uint32 /*ulTrackID*/)
{
  float fVidFrameRate = 0.0;
  if (m_pMetadataInfo && m_pMetadataInfo->fVidFrameRate)
  {
    fVidFrameRate = m_pMetadataInfo->fVidFrameRate;
  }
  return fVidFrameRate;
}

/* ============================================================================
  @brief  Copies codec config data into output buffer.

  @details    This function is used to copy codec config data.

  @param[in]      trackId       Track Id number.
  @param[in/out]  pCodecBuf     Codec Config buffer.
  @param[in]      pulBufSize    Size of the input buffer.

  @return     PARSER_ErrorNone indicating sample read successfully.
              Else, it will report corresponding error.
  @note       BufferSize should be sufficient to copy the codec config data.
============================================================================ */
PARSER_ERRORTYPE FLVParser::GetCodecHeader(uint32 ulTrackId, uint8* pCodecBuf,
                                           uint32 *pulBufSize)
{
  PARSER_ERRORTYPE eRetStatus = PARSER_ErrorInvalidParam;
  if ((!pCodecBuf) || (!pulBufSize))
  {
    return eRetStatus;
  }
  eRetStatus = PARSER_ErrorInsufficientBufSize;
  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW,"GetCodecHeader for trackId %lu",
               ulTrackId);
  //! If codec type is H264, copy codec config data in SPS/PPS format
  if((m_pVideoInfo) && (m_pVideoInfo->ulTrackId == ulTrackId) &&
     (FLV_H264 == m_pVideoInfo->eVideoCodec ) &&
     (m_pVideoInfo->psCodecConfig) &&
     (*pulBufSize >= m_pVideoInfo->ulCodecConfigSize))
  {
    AVCCodecBuf* pCodecConfig = m_pVideoInfo->psCodecConfig;
    uint32       ulStartCode  = 0x01000000;
    uint32       ulCount      = 0;
    uint32       ulFilledLen  = 0;

    for ( ; ulCount < pCodecConfig->ucNumSeqParams; ulCount++)
    {
      //! copy start code
      memcpy(pCodecBuf + ulFilledLen, &ulStartCode, sizeof(uint32));
      ulFilledLen += FOURCC_SIGNATURE_BYTES;

      //! Copy sequence parameters into o/p buf
      if (pCodecConfig->pSeqParam && pCodecConfig->pSeqParam[ulCount].pucNALBuf)
      {
        memcpy(pCodecBuf + ulFilledLen,
               pCodecConfig->pSeqParam[ulCount].pucNALBuf,
               pCodecConfig->pSeqParam[ulCount].usNALLength);
        ulFilledLen += pCodecConfig->pSeqParam[ulCount].usNALLength;
      }
    }
    for (ulCount = 0; ulCount < pCodecConfig->ucNumPicParams; ulCount++)
    {
      //! copy start code
      memcpy(pCodecBuf + ulFilledLen, &ulStartCode, sizeof(uint32));
      ulFilledLen += FOURCC_SIGNATURE_BYTES;

      //! Copy sequence parameters into o/p buf
      if (pCodecConfig->pPicParam && pCodecConfig->pPicParam[ulCount].pucNALBuf)
      {
        memcpy(pCodecBuf + ulFilledLen,
               pCodecConfig->pPicParam[ulCount].pucNALBuf,
               pCodecConfig->pPicParam[ulCount].usNALLength);
        ulFilledLen += pCodecConfig->pPicParam[ulCount].usNALLength;
      }
    }
    eRetStatus = PARSER_ErrorNone;
  }//if(m_pVideoInfo && (m_pVideoInfo->nTrackId == id)  )
  else if(m_pAudioInfo && (m_pAudioInfo->ucTrackId == ulTrackId))
  {
    if (*pulBufSize >= m_pAudioInfo->ulCodecConfigSize)
    {
      memcpy(pCodecBuf, m_pAudioInfo->pucCodecConfigBuf,
             m_pAudioInfo->ulCodecConfigSize);
      eRetStatus = PARSER_ErrorNone;
    }
    else
    {
      *pulBufSize = m_pAudioInfo->ulCodecConfigSize;
    }
  }
  else if (m_pVideoInfo)
  {
    *pulBufSize = m_pVideoInfo->ulCodecConfigSize;
  }

  return eRetStatus;
}

/* ============================================================================
  @brief  Calculates codec config data buffer size

  @details    This function is used to calculate codec config data length

  @param[in]      trackId       Track Id number.

  @return     Codec config data length .
  @note       None.
============================================================================ */
uint32 FLVParser::GetCodecHeaderSize(uint32 ulTrackId)
{
  uint32 ulCodecDataSize = 0;
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW,"GetCodecHeaderSize");

  if ((m_pVideoInfo) && (m_pVideoInfo->ulCodecConfigSize) &&
      (m_pVideoInfo->ulTrackId == ulTrackId))
  {
    ulCodecDataSize = m_pVideoInfo->ulCodecConfigSize;
  }
  //! Calculate codec config data only if it is not calculated already
  else if((m_pVideoInfo) && (m_pVideoInfo->ulTrackId == ulTrackId) &&
          (FLV_H264 == m_pVideoInfo->eVideoCodec ) &&
          (m_pVideoInfo->psCodecConfig))
  {
    AVCCodecBuf *pCodecConfig = m_pVideoInfo->psCodecConfig;
    uint32 ulCount = 0;
    //! Size to indicate the start offsets for all
    ulCodecDataSize = (pCodecConfig->ucNumPicParams +
                       pCodecConfig->ucNumSeqParams) * FOURCC_SIGNATURE_BYTES;
    for ( ; ulCount < pCodecConfig->ucNumSeqParams; ulCount++)
    {
      if (!pCodecConfig->pSeqParam)
      {
        break;
      }
      ulCodecDataSize += pCodecConfig->pSeqParam[ulCount].usNALLength;
    }
    for (ulCount = 0 ; ulCount < pCodecConfig->ucNumPicParams; ulCount++)
    {
      if (!pCodecConfig->pPicParam)
      {
        break;
      }
      ulCodecDataSize += pCodecConfig->pPicParam[ulCount].usNALLength;
    }
    m_pVideoInfo->ulCodecConfigSize = ulCodecDataSize;
  }//if(m_pVideoInfo && (m_pVideoInfo->nTrackId == id) )
  else if(m_pAudioInfo && (m_pAudioInfo->ucTrackId == ulTrackId))
  {
    ulCodecDataSize = m_pAudioInfo->ulCodecConfigSize;
  }
  MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_MEDIUM,
               "GetCodecHeaderSize for track %lu is %lu",
               ulTrackId, ulCodecDataSize);
  return ulCodecDataSize;
}

/* =============================================================================
FUNCTION:
 FLVParser::ParseFLVHeader

DESCRIPTION:
parses and stores FLV header information
INPUT/OUTPUT PARAMETERS:

RETURN VALUE:
 PARSER_ErrorNone if successful in parsing
 otherwise returns appropriate error code
SIDE EFFECTS:
  None.
=============================================================================*/
PARSER_ERRORTYPE  FLVParser::ParseFLVHeader(uint64 ullOffset)
{
  PARSER_ERRORTYPE retError = PARSER_ErrorDefault;
  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,"ParseFLVHeader @ %llu", ullOffset);

  if(m_pucDataBuffer)
  {
    bool bok = true;
    if(FLVCallbakGetData( ullOffset, 16,
                          m_pucDataBuffer,DEF_DATA_BUF_SIZE,
                          m_pUserData))
    {
      if(!m_pFLVHdr)
      {
        m_pFLVHdr = (FLVHeader*)MM_Malloc(sizeof(FLVHeader));
        if(m_pFLVHdr)
        {
          //parse the FLV header
          memset(m_pFLVHdr,0,sizeof(FLVHeader));
          if(!memcmp(m_pucDataBuffer,"FLV",FLV_SIG_BYTES_LENGTH) )
          {
            memcpy(&(m_pFLVHdr->ucFLVVersionInfo),
                   m_pucDataBuffer+FLV_SIG_BYTES_LENGTH, FLV_VERSION_NO_BYTES);
            m_ullCurrOffset += (FLV_SIG_BYTES_LENGTH+FLV_VERSION_NO_BYTES);
            uint8 nvalbyte = m_pucDataBuffer[m_ullCurrOffset];
            m_pFLVHdr->ucAudioPresentFlag = (uint8)((nvalbyte & 0x04)>>2);
            m_pFLVHdr->ucVideoPresentFlag = (uint8)(nvalbyte & 0x01);
            m_ullCurrOffset++;
            memcpy(&(m_pFLVHdr->ulDataStartOffset),
                   m_pucDataBuffer + m_ullCurrOffset, sizeof(uint32));
            FS_REVERSE_ENDIAN((uint8*)&(m_pFLVHdr->ulDataStartOffset),
                               sizeof(uint32));
            m_ullCurrOffset +=sizeof(uint32);
            retError = PARSER_ErrorNone;
          }
        }
      }
    }
    else
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                  "ParseFLVHeader FLVCallbakGetData failed");
      bok = false;
    }
  }//if(m_pDataBuffer)
  if (PARSER_ErrorNone == retError)
  {
    (void)ParseMetaData(m_ullCurrOffset);
  }
  return retError;
}
/* =============================================================================
FUNCTION:
 FLVParser::ParseFLVTag

DESCRIPTION:
parses FLV tag information
INPUT/OUTPUT PARAMETERS:

RETURN VALUE:
 PARSER_ErrorNone if successful in parsing,
 otherwise returns appropriate error code
SIDE EFFECTS:
  None.
=============================================================================*/
PARSER_ERRORTYPE  FLVParser::ParseFLVTag(uint64 ullOffset, FLVTagInfo* ptag)
{
  PARSER_ERRORTYPE retError = PARSER_ErrorDefault;
  uint32 ulIndex = 0;

  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,"ParseFLVTag @ %llu", ullOffset);
  if (m_ullFileSize <= ullOffset)
  {
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,"ParseFLVTag EOF reached @ %llu",
                 ullOffset);
    return PARSER_ErrorEndOfFile;
  }

  if(m_pucDataBuffer)
  {
    FLVTagInfo* pmytag       = ptag;
    uint32 ulExtraHeaderSize = 0;
    FLVTagInfo taginfo;
    if(!pmytag)
    {
      pmytag = &taginfo;
    }
    memset(pmytag, 0, sizeof(FLVTagInfo));
    /* Read double the header size. After Fixed header, we have some optional
       header based on codec type. To handle that case, read double the header
       size. */
    uint32 ulDataRead = FLVCallbakGetData(ullOffset, (2 * FLV_TAG_HDR_SIZE),
                                          m_pucDataBuffer, DEF_DATA_BUF_SIZE,
                                          m_pUserData);
    if((2 * FLV_TAG_HDR_SIZE) == ulDataRead )
    {
      pmytag->ullOffset = ullOffset;

      memcpy(&(pmytag->ulPrevTagSize), m_pucDataBuffer + ulIndex, sizeof(uint32));
      FS_REVERSE_ENDIAN((uint8*)&(pmytag->ulPrevTagSize), FOURCC_SIGNATURE_BYTES);
      ulIndex += FOURCC_SIGNATURE_BYTES;

      memcpy(&(pmytag->ucTagType), m_pucDataBuffer + ulIndex, sizeof(uint8));
      ulIndex++;

      memcpy(&(pmytag->ulTagDataSize),m_pucDataBuffer+ulIndex,sizeof(uint8)*3);
      FS_REVERSE_ENDIAN((uint8*)&(pmytag->ulTagDataSize), 3);
      ulIndex+=3;

      memcpy(&(pmytag->ulTime), m_pucDataBuffer + ulIndex, sizeof(uint8)*3);
      FS_REVERSE_ENDIAN((uint8*)&(pmytag->ulTime), 3);
      ulIndex+=3;

      memcpy(&(pmytag->ucExtendedTimeStamp),m_pucDataBuffer + ulIndex,
             sizeof(uint8));
      ulIndex++;

      memcpy(&(pmytag->ulStreamId), m_pucDataBuffer + ulIndex, sizeof(uint8)*3);
      FS_REVERSE_ENDIAN((uint8*)&(pmytag->ulStreamId), 3);
      ulIndex+=3;
      retError = PARSER_ErrorNone;

      //! Copy Header byte, first byte in data indicates Header
      pmytag->ucCodecHeader = m_pucDataBuffer[ulIndex];

      //adjust payload start offset based on audio/video tag
      if(TAG_TYPE_AUDIO == pmytag->ucTagType )
      {
        uint8 ucCodecHeaderByte = (m_pucDataBuffer[ulIndex] & 0xF0) >> 4;
        FLVMediaCodecType eAudioCodec =
                           MapHeaderToAudioCodecType(ucCodecHeaderByte);
        //! By default, one byte is used for Audio Tag Header
        ulExtraHeaderSize = 1;

        /* If Codec is AAC, one more extra byte is provided to indicate AAC
           frame type. */
        if (FLV_AAC == eAudioCodec)
        {
          pmytag->ucAACFrameType = m_pucDataBuffer[ulIndex + 1];
          ulExtraHeaderSize++;
        }
      }
      else if(TAG_TYPE_VIDEO == pmytag->ucTagType )
      {
        //! FLV contains extra header data at frame
        ulExtraHeaderSize = UpdateVideoProperties(m_pucDataBuffer + ulIndex,
                                                  ulDataRead - ulIndex, pmytag);
      }

      //! Update Tag Header size and payload size fields
      pmytag->ucTagHeaderSize = (uint8)(ulIndex + ulExtraHeaderSize);
      pmytag->ulPayloadSize   = pmytag->ulTagDataSize - ulExtraHeaderSize;
    }
    else
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
                  "ParseFLVTag FLVCallbakGetData failed...");
      retError = PARSER_ErrorEndOfFile;
    }
  }//if(m_pDataBuffer)
  return retError;
}

/* =============================================================================
FUNCTION:
 FLVParser::ParseAudioInfo

DESCRIPTION:
parses and stores FLV audio header information
INPUT/OUTPUT PARAMETERS:

RETURN VALUE:
 PARSER_ErrorNone if successful in parsing,
 otherwise returns appropriate error code
SIDE EFFECTS:
  None.
=============================================================================*/
PARSER_ERRORTYPE  FLVParser::ParseAudioInfo(uint64 ullOffset)
{
 PARSER_ERRORTYPE retError = PARSER_ErrorDefault;
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,"FLVParser::ParseAudioInfo");
  if((m_pucDataBuffer) && (m_pFLVHdr) && (m_pFLVHdr->ucAudioPresentFlag))
  {
    bool bok = true;
    FLVTagInfo taginfo;

    memset(&taginfo, 0, sizeof(FLVTagInfo));
    while((bok)&&(!m_pAudioInfo))
    {
      //! Parse current Tag info
      retError = ParseFLVTag(ullOffset, &taginfo);
      if(PARSER_ErrorNone == retError)
      {
        if(TAG_TYPE_AUDIO == taginfo.ucTagType )
        {
          uint8 ucHeaderByte = taginfo.ucCodecHeader;
          m_pAudioInfo = (FLVAudioInfo*)MM_Malloc(sizeof(FLVAudioInfo));
          if(m_pAudioInfo)
          {
            memset(m_pAudioInfo, 0, sizeof(FLVAudioInfo));
            uint8 ucCodecHdr  = (uint8)((ucHeaderByte & 0xF0)>>4);
            uint8 nsamplerate = (uint8)((ucHeaderByte & 0x0C)>>2);

            m_pAudioInfo->eAudioCodec = MapHeaderToAudioCodecType(ucCodecHdr);
            m_pAudioInfo->ulAudStartOffset = (uint32)ullOffset;
            m_pAudioInfo->ucTrackId        = m_ucNoStreams;
            m_ucNoStreams++;
            m_ucNoAudioStreams++;

            //! Sampling rate field
            if(!nsamplerate )
            {
              m_pAudioInfo->ulSamplingRate =  5500;
            }
            else if(1 == nsamplerate )
            {
              m_pAudioInfo->ulSamplingRate =  11000;
            }
            else if(2 == nsamplerate )
            {
              m_pAudioInfo->ulSamplingRate =  22000;
            }
            else if(3 == nsamplerate )
            {
              m_pAudioInfo->ulSamplingRate =  44100;
            }

            //! bits per sample field
            if(!((ucHeaderByte & 0x02)>>1))
            {
              m_pAudioInfo->ucBitsPerSample = 8;
            }
            else
            {
              m_pAudioInfo->ucBitsPerSample = 16;
            }

            //! Number of channels
            if(!(ucHeaderByte & 0x01) )
            {
              m_pAudioInfo->ucNumChannels = 1;
            }
            else
            {
              m_pAudioInfo->ucNumChannels = 2;
            }
            //! If file format is AAC, one Extra byte is used
            if (FLV_AAC == m_pAudioInfo->eAudioCodec)
            {
              //! Check AAC Frame type
              taginfo.ucAACFrameType = taginfo.ucAACFrameType;
              //! "ZERO" indicates following data is Audio codec config info
              if (0 == taginfo.ucAACFrameType)
              {
                m_pAudioInfo->ulCodecConfigSize = taginfo.ulPayloadSize;
                m_pAudioInfo->pucCodecConfigBuf =
                  (uint8*)MM_Malloc(m_pAudioInfo->ulCodecConfigSize);
                if (m_pAudioInfo->pucCodecConfigBuf)
                {
                  uint64 ullTagOffset = taginfo.ullOffset +
                                        taginfo.ucTagHeaderSize;
                  uint32 ulDataRead = FLVCallbakGetData( ullTagOffset,
                                            taginfo.ulTagDataSize,
                                            m_pAudioInfo->pucCodecConfigBuf,
                                            m_pAudioInfo->ulCodecConfigSize,
                                            m_pUserData);
                  m_pAudioInfo->ulCodecConfigSize = ulDataRead;
                } //! if (m_pAudioInfo->pucCodecConfigBuf)
                /* For AAC, no need to send Codec config data as
                   media data explicitly in first media sample */
                m_pAudioInfo->ulAudStartOffset = (uint32)
                  NEXT_TAG_OFFSET(taginfo.ullOffset, taginfo.ulTagDataSize);
              } //! if (0 == taginfo.ucAACFrameType)
            } //! if (FLV_AAC == m_pAudioInfo->eAudioCodec)

            retError = PARSER_ErrorNone;
          }//if(m_pAudioInfo)
          else
          {
            bok      = false;
            retError = PARSER_ErrorMemAllocFail;
            MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
                         "FLVParser::ParseAudioInfo malloc failed");
          }
          break;
        }//if(taginfo.nTagType == TAG_TYPE_AUDIO)
        //! Go to end of the current Tag
        ullOffset = NEXT_TAG_OFFSET(taginfo.ullOffset, taginfo.ulTagDataSize);
      }//if(PARSER_ErrorNone == retError)
      else
      {
        MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
                  "FLVParser::ParseAudioInfo ParseFLVTag failed %d", retError);
        bok = false;
      }
    }//while((bok)&&(!m_pAudioInfo))
  }//if(m_pDataBuffer && m_pFLVHdr)

  return retError;
}

/* =============================================================================
FUNCTION:
 FLVParser::ParseVideoInfo

DESCRIPTION:
parses and stores FLV video header information
INPUT/OUTPUT PARAMETERS:

RETURN VALUE:
 PARSER_ErrorNone if successful in parsing,
 otherwise returns appropriate error code
SIDE EFFECTS:
  None.
=============================================================================*/
PARSER_ERRORTYPE  FLVParser::ParseVideoInfo(uint64 ullOffset)
{
  PARSER_ERRORTYPE retError = PARSER_ErrorDefault;
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM, "FLVParser::ParseVideoInfo");
  if((m_pucDataBuffer) && (m_pFLVHdr) && (m_pFLVHdr->ucVideoPresentFlag))
  {
    bool bok = true;
    FLVTagInfo taginfo;
    memset(&taginfo,0,sizeof(FLVTagInfo));
    while((bok)&&(!m_pVideoInfo))
    {
      retError = ParseFLVTag(ullOffset, &taginfo);
      if(PARSER_ErrorNone == retError)
      {
        if(TAG_TYPE_VIDEO == taginfo.ucTagType )
        {
          uint8 ucHeaderByte = taginfo.ucCodecHeader;
          m_pVideoInfo = (FLVVideoInfo*)MM_Malloc(sizeof(FLVVideoInfo));
          if(m_pVideoInfo)
          {
            memset(m_pVideoInfo,0,sizeof(FLVVideoInfo));
            m_pVideoInfo->ulTrackId = m_ucNoStreams;
            m_ucNoStreams++;
            m_ucNoVideoStreams++;
            uint8 ucCodecByte = (ucHeaderByte & 0x0F);
            m_pVideoInfo->eVideoCodec = MapHeaderToVideoCodecType(ucCodecByte);
            m_pVideoInfo->ulVidStartOffset = (uint32)ullOffset;
            //! Prepare Codec Config data for H264
            //! For H264, codec config data need not be send one more time
            //! Skip the tag which has codec config data
            if (FLV_H264 == m_pVideoInfo->eVideoCodec)
            {
              (void)PrepareCodecConfigforH264(&taginfo);
              m_pVideoInfo->ulVidStartOffset= (uint32)
                    NEXT_TAG_OFFSET(taginfo.ullOffset, taginfo.ulTagDataSize);
            }
            retError = PARSER_ErrorNone;
          }//if(m_pVideoInfo)
          else
          {
            MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
              "FLVParser::ParseVideoInfo malloc failed");
            retError = PARSER_ErrorMemAllocFail;
            bok      = false;
          }
          break;
        }//if(taginfo.nTagType == TAG_TYPE_VIDEO)
        //! Update offset to start of next tag
        ullOffset = NEXT_TAG_OFFSET(taginfo.ullOffset, taginfo.ulTagDataSize);
      }//if(PARSER_ErrorNone == retError)
      else
      {
        MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
                  "FLVParser::ParseVideoInfo ParseFLVTag failed %d", retError);
        bok = false;
      }
    }//while((bok)&&(!m_pVideoInfo))
  }//if(m_pDataBuffer && m_pFLVHdr)

  return retError;
}

/* =============================================================================
FUNCTION:
 FLVParser::ParseMetaData

DESCRIPTION:
parses and stores FLV metadata information
INPUT/OUTPUT PARAMETERS:

RETURN VALUE:
 PARSER_ErrorNone if successful in parsing,
 otherwise returns appropriate error code
SIDE EFFECTS:
  None.
=============================================================================*/
PARSER_ERRORTYPE  FLVParser::ParseMetaData(uint64 ullOffset)
{
  PARSER_ERRORTYPE retError = PARSER_ErrorDefault;
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,"FLVParser::ParseMetaData");
  if(m_pucDataBuffer && m_pFLVHdr)
  {
    bool bok = true;
    FLVTagInfo taginfo;
    memset(&taginfo,0,sizeof(FLVTagInfo));

    while((bok)&&(!m_pMetadataInfo))
    {
      retError = ParseFLVTag(ullOffset, &taginfo);
      if(PARSER_ErrorNone == retError)
      {
        ullOffset += (FLV_TAG_HDR_BYTES + sizeof(uint32));

        if((!m_pMetadataInfo)&& (TAG_TYPE_SCRIPT_DATA == taginfo.ucTagType ))
        {
          bool bMetaDataStart = false;
          //! Reset the flag, as Metadata Tag is found
          bok = false;
          m_pMetadataInfo = (FLVMetaDataInfo*)MM_Malloc(sizeof(FLVMetaDataInfo));
          MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
                       "FLVParser::ParseMetaData @ %llu", taginfo.ullOffset);
          if(m_pMetadataInfo)
          {
            memset(m_pMetadataInfo,0,sizeof(FLVMetaDataInfo));

            uint32 ulDataRead = FLVCallbakGetData(ullOffset,
                                                  taginfo.ulTagDataSize,
                                                  m_pucDataBuffer,
                                                  DEF_DATA_BUF_SIZE,
                                                  m_pUserData);
            uint32 ulIndex = 0;
            if (ulIndex < ulDataRead)
            {
              uint8  ucDataType = m_pucDataBuffer[ulIndex++];
              uint16 usStrLen   = 0;
              if (FLV_METADATA_STRG_TYPE == ucDataType)
              {
                usStrLen = (uint16)(m_pucDataBuffer[ulIndex] << 8 |
                                    m_pucDataBuffer[ulIndex+1]);
                ulIndex += 2;
              }
              if (!strncmp((char*)m_cMetadataStr[0],
                           (char*)(m_pucDataBuffer + ulIndex), usStrLen))
              {
                bMetaDataStart = true;
                ulIndex += usStrLen;
              }
            }
            while(bMetaDataStart && ulIndex < ulDataRead)
            {
              uint8  ucDataType = m_pucDataBuffer[ulIndex++];
              uint16 usStrLen   = 0;
              double fValue     = 0.0;
              if (FLV_METADATA_ECMA_ARR_TYPE == ucDataType)
              {
                uint32 ulECMACount = 0;
                memcpy(&(ulECMACount), m_pucDataBuffer+ulIndex, sizeof(uint32));
                FS_REVERSE_ENDIAN((uint8*)&(ulECMACount), sizeof(uint32));
                ulIndex += FOURCC_SIGNATURE_BYTES;
                while(ulIndex < ulDataRead)
                {
                  char*  pDataPtr      = (char*)m_pucDataBuffer + ulIndex;
                  uint32 ulDataTypeLen = (uint32)(sizeof(double));

                  /* Check if Object End is found or not. */
                  if (!memcmp(pDataPtr, SCRIPTDATA_OBJECT_END, 3))
                  {
                    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
                      "ParseMetaData End Script is found!!");
                    ulIndex += (uint32)(sizeof(SCRIPTDATA_OBJECT_END));
                    break;
                  }

                  usStrLen   = (uint16)(m_pucDataBuffer[ulIndex] << 8 |
                                        m_pucDataBuffer[ulIndex + 1]);
                  ulIndex   += 2;

                  //If String length is corrupted, then break loop!!
                  if (usStrLen > ulDataRead - ulIndex)
                  {
                    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                                "ParseMetaData length is incorrect!!");
                    break;
                  }

                  ucDataType = m_pucDataBuffer[ulIndex + usStrLen];
                  pDataPtr   = (char*)m_pucDataBuffer + ulIndex;
                  if (FLV_METADATA_DOUBLE_TYPE == ucDataType)
                  {
                    memcpy(&(fValue), m_pucDataBuffer + ulIndex + usStrLen + 1,
                           sizeof(double));
                    FS_REVERSE_ENDIAN((uint8*)&(fValue), (size_t)(sizeof(double)));
                  }
                  else if (FLV_METADATA_BOOL_TYPE == ucDataType)
                  {
                    uint8 ucVal = (m_pucDataBuffer[ulIndex]);
                    fValue = ucVal;
                    ulDataTypeLen = 1;//! Char size
                  }
                  else if(FLV_METADATA_US16_TYPE == ucDataType)
                  {
                    uint16 usValue = 0;
                    memcpy(&(usValue), m_pucDataBuffer + ulIndex + usStrLen + 1,
                           sizeof(uint16));
                    FS_REVERSE_ENDIAN((uint8*)&(usValue), 2);
                    fValue = usValue;
                    ulDataTypeLen = 2;//! Uint16/Word Size
                  }
                  else if(FLV_METADATA_DATE_TYPE == ucDataType)
                  {
                    memcpy(&(fValue), m_pucDataBuffer + ulIndex + usStrLen + 1,
                           sizeof(double));
                    FS_REVERSE_ENDIAN((uint8*)&(fValue), sizeof(double));
                    uint16 usLocalDateOffset = 0;
                    memcpy(&(usLocalDateOffset),
                           m_pucDataBuffer + ulIndex + usStrLen + 3,
                           sizeof(uint16));
                    FS_REVERSE_ENDIAN((uint8*)&(usLocalDateOffset), 2);
                  }

                  if (!strncmp(m_cMetadataStr[1], pDataPtr, usStrLen))
                  {
                    m_pMetadataInfo->ulAudioCodecId = (uint32)(fValue);
                  }
                  else if (!strncmp(m_cMetadataStr[2], pDataPtr, usStrLen))
                  {
                    m_pMetadataInfo->ulAudioDataRate = (uint32)(fValue);
                  }
                  else if (!strncmp(m_cMetadataStr[3], pDataPtr, usStrLen))
                  {
                    m_pMetadataInfo->ulAudioDelay = (uint32)(fValue);
                  }
                  else if (!strncmp(m_cMetadataStr[4], pDataPtr, usStrLen))
                  {
                    m_pMetadataInfo->ulAudioSampleRate = (uint32)(fValue);
                  }
                  else if (!strncmp(m_cMetadataStr[5], pDataPtr, usStrLen))
                  {
                    m_pMetadataInfo->ulAudioSampleSize = (uint32)(fValue);
                  }
                  else if (!strncmp(m_cMetadataStr[6], pDataPtr, usStrLen))
                  {
                    m_pMetadataInfo->bCanSeekToEnd = (boolean)(fValue);
                  }
                  else if (!strncmp(m_cMetadataStr[7], pDataPtr, usStrLen))
                  {
                    //Creation date, if required we will add it in future...
                  }
                  else if (!strncmp(m_cMetadataStr[8], pDataPtr, usStrLen))
                  {
                    m_pMetadataInfo->ulDuration = (uint32)(fValue *
                                                  MILLISEC_TIMESCALE_UNIT);
                    m_ullClipDuration = m_pMetadataInfo->ulDuration;
                  }
                  else if (!strncmp(m_cMetadataStr[9], pDataPtr, usStrLen))
                  {
                    m_pMetadataInfo->fVidFrameRate = (float)(fValue);
                  }
                  else if (!strncmp(m_cMetadataStr[10], pDataPtr, usStrLen))
                  {
                    m_pMetadataInfo->ulVidFrameHeight = (uint32)(fValue);
                  }
                  else if (!strncmp(m_cMetadataStr[11], pDataPtr, usStrLen))
                  {
                    m_pMetadataInfo->bIsStereo = (boolean)(fValue);
                  }
                  else if (!strncmp(m_cMetadataStr[12], pDataPtr, usStrLen))
                  {
                    m_pMetadataInfo->ulVidCodecId = (uint32)(fValue);
                  }
                  else if (!strncmp(m_cMetadataStr[13], pDataPtr, usStrLen))
                  {
                    m_pMetadataInfo->ulVidDataRate = (uint32)(fValue);
                  }
                  else if (!strncmp(m_cMetadataStr[14], pDataPtr, usStrLen))
                  {
                    m_pMetadataInfo->ulVidFrameWidth = (uint32)(fValue);
                  }
                  ulIndex += usStrLen + 1;//1 Extra byte is for Data type of value

                  if (FLV_METADATA_STRG_TYPE == ucDataType)
                  {
                    usStrLen = (uint16)(m_pucDataBuffer[ulIndex] << 8 |
                                        m_pucDataBuffer[ulIndex + 1]);
                    ulIndex += 2;
                    ulIndex += usStrLen;
                  }
                  else if(FLV_METADATA_DATE_TYPE == ucDataType)
                  {
                    //2 Extra bytes contain time zone difference value
                    ulIndex += ulDataTypeLen + 2;
                  }
                  else
                  {
                    ulIndex += ulDataTypeLen;
                  }
                }
              }
            }
            retError =  PARSER_ErrorNone;
            break;
          }//if(m_pMetadataInfo)
        }//if((!m_pMetadataInfo)&& (taginfo.nTagType == TAG_TYPE_SCRIPT_DATA))
        ullOffset += taginfo.ulTagDataSize;
      }
      else
      {
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
           "ParseMetaData FLVCallbakGetData failed while reading metadata");
        bok = false;
        break;
      }
    }//while((bok)&&(!m_pAudioInfo))
  }//if(m_pDataBuffer && m_pFLVHdr)
  return retError;
}

/* ============================================================================
  @brief  Map header byte with Codec Enum

  @details        This function is used to map the header byte with Codec Enum.

  @param[in]      ucHeaderByte       Header byte

  @return     Codec Type.
  @note       Nothing.
============================================================================ */
FLVMediaCodecType  FLVParser::MapHeaderToVideoCodecType(uint8 ucHeaderByte)
{
  FLVMediaCodecType eVideoCodec = FLV_UNKNOWN_CODEC;
  switch(ucHeaderByte)
  {
    case 2:
      {
        eVideoCodec = FLV_SORENSON_H263_VIDEO;
      }
      break;
    case 3:
      {
        eVideoCodec = FLV_SCREENVIDEO;
      }
      break;
    case 4:
      {
        eVideoCodec = FLV_VP6_VIDEO;
      }
      break;
    case 5:
      {
        eVideoCodec = FLV_VP6_ALPHA_CHANNEL;
      }
      break;
    case 6:
      {
        eVideoCodec = FLV_SCREENVIDEO_V2;
      }
      break;
    case 7:
      {
        eVideoCodec = FLV_H264;
      }
      break;
    default:
      {
        eVideoCodec = FLV_UNKNOWN_CODEC;
      }
      break;
  }
  return eVideoCodec;
}

/* ============================================================================
  @brief  Map header byte with Codec Enum

  @details        This function is used to map the header byte with Codec Enum.

  @param[in]      ucHeaderByte       Header byte

  @return     Codec Type.
  @note       Nothing.
============================================================================ */
FLVMediaCodecType  FLVParser::MapHeaderToAudioCodecType(uint8 ucHeaderByte)
{
  FLVMediaCodecType eAudioCodec = FLV_UNKNOWN_CODEC;
  switch(ucHeaderByte)
  {
    case 0:
      {
        eAudioCodec = FLV_UNCOMPRESSED_AUDIO;
      }
      break;
    case 1:
      {
        eAudioCodec = FLV_ADPCM_AUDIO;
      }
      break;
    case 2:
      {
        eAudioCodec = FLV_MP3_AUDIO;
      }
      break;
    case 5:
    case 6:
      {
        eAudioCodec = FLV_NELLYMOSER;
      }
      break;
    case 10:
      {
        eAudioCodec = FLV_AAC;
      }
      break;
    default:
      {
        eAudioCodec = FLV_UNKNOWN_CODEC;
      }
      break;
  }
  return eAudioCodec;
}

/* ============================================================================
  @brief  Function to update Video codec properties

  @details        This function is used to map the header byte with Codec Enum.

  @param[in]      pucDataBuf       Buffer pointer
  @param[in]      ulBufSize        Buffer size
  @param[in]      pTag             FLV Tag info structure

  @return     Codec Type.
  @note       Nothing.
============================================================================ */
uint32  FLVParser::UpdateVideoProperties(uint8* pucDataBuf, uint32 ulBufSize,
                                         FLVTagInfo *pTag)
{
  uint32            ulIndex     = 0;
  uint8             ucFormatTag = 0;
  FLVMediaCodecType eVideoCodec = FLV_UNKNOWN_CODEC;

  if ((!pTag) || (!pucDataBuf) || (!ulBufSize))
  {
    return 0;
  }
  else
  {
    ucFormatTag = (pucDataBuf[ulIndex] & 0x0F);
    eVideoCodec = MapHeaderToVideoCodecType(ucFormatTag);
  }
  if( (FLV_VP6_VIDEO == eVideoCodec)||
      (FLV_VP6_ALPHA_CHANNEL == eVideoCodec))
  {
    //! VP6 contains two bytes of configuration data
    ulIndex = 2;
  }
  else if(FLV_SORENSON_H263_VIDEO == eVideoCodec)
  {
    //first byte in case of H263 contains configuration data, so skip it.
    ulIndex++;
  }
  else if(FLV_H264 == eVideoCodec)
  {
    //! First byte contains Header byte
    ulIndex++;
    //! Next byte contains AVC Packet type
    pTag->eAVCPacketType = (AVC_PACKET_TYPE)pucDataBuf[ulIndex++];

    //! 3bytes is used for Composition Timestamp
    memcpy(&(pTag->ulCompositionTime), pucDataBuf + ulIndex,
           sizeof(uint8)*3);
    FS_REVERSE_ENDIAN((uint8*)&(pTag->ulCompositionTime), 3);
    ulIndex += 3;
  }
  return ulIndex;
}

/* ============================================================================
  @brief  Function to parse codec config data for H264 codec

  @details        This function is used to parse codec config data buf.
                  It updates codec config data buffer in FLV Parser class.

  @param[in]      pTag             FLV Tag info structure

  @return     Return status
  @note       It is called only if Codec type is H264/AVC.
============================================================================ */
PARSER_ERRORTYPE FLVParser::PrepareCodecConfigforH264(FLVTagInfo *pTagInfo)
{
  PARSER_ERRORTYPE eRetStatus = PARSER_ErrorDefault;
  uint64           ullOffset  = 0;
  if ((!pTagInfo) || (!m_pucDataBuffer) || (!m_pVideoInfo))
  {
    return eRetStatus;
  }
  do
  {
    FLVMediaCodecType eCodecType = FLV_UNKNOWN_CODEC;
    ullOffset = NEXT_TAG_OFFSET(pTagInfo->ullOffset, pTagInfo->ulTagDataSize);
    if (TAG_TYPE_VIDEO == pTagInfo->ucTagType)
    {
      eCodecType = MapHeaderToVideoCodecType(pTagInfo->ucCodecHeader & 0x0F);
    }

    //! If packet type is Sequence header, then Packet contains CodecConfig
    if ((AVC_SEQ_HEADER == pTagInfo->eAVCPacketType) ||
        (FLV_H264 == eCodecType))
    {
      uint8* pTemp  = (uint8*)MM_Malloc(pTagInfo->ulPayloadSize);
      AVCCodecBuf *pCodecConfig = (AVCCodecBuf*)MM_Malloc(sizeof(AVCCodecBuf));
      m_pVideoInfo->psCodecConfig = pCodecConfig;
      //! Proceed further if memory is allocated successfully
      if ((pTemp) && (pCodecConfig))
      {
        uint32 ulIndex = 0;
        uint64 ullTagOffset = pTagInfo->ullOffset + pTagInfo->ucTagHeaderSize;
        uint32 ulDataRead = FLVCallbakGetData(ullTagOffset,
                                              pTagInfo->ulPayloadSize,
                                              pTemp, pTagInfo->ulPayloadSize,
                                              m_pUserData);
        memset(pCodecConfig, 0, sizeof(AVCCodecBuf));
        //! Ensure minimum amount of data is read into buffer
        if(ulDataRead > NALU_MIN_SIZE)
        {
          pCodecConfig->ucConfigVersion        = pTemp[ulIndex++];
          pCodecConfig->ucAVCProfile           = pTemp[ulIndex++];
          pCodecConfig->ucProfileCompatibility = pTemp[ulIndex++];
          pCodecConfig->ucAVCLevelIndication   = pTemp[ulIndex++];
          pCodecConfig->ucNALLengthMinusOne    = pTemp[ulIndex++] & 0x03;
          pCodecConfig->ucNumSeqParams         = pTemp[ulIndex++] & 0x1F;

          //! Parse Sequence Params
          if (pCodecConfig->ucNumSeqParams)
          {
            pCodecConfig->pSeqParam =
                                UpdateNALUParams(pTemp, ulIndex, ulDataRead,
                                                 pCodecConfig->ucNumSeqParams);
          }
          if(ulIndex < ulDataRead)
            pCodecConfig->ucNumPicParams = pTemp[ulIndex++];

          //! Parse Picture Params
          if (pCodecConfig->ucNumPicParams)
          {
            pCodecConfig->pPicParam =
                                UpdateNALUParams(pTemp, ulIndex, ulDataRead,
                                                 pCodecConfig->ucNumPicParams);
          }
        }
      }
      //! Free the temporary memory allocated
      if (pTemp)
      {
        MM_Free(pTemp);
      }
      //! If memory allocation is not successful, free the class memory as well
      else if(pCodecConfig)
      {
        MM_Free(pCodecConfig);
        m_pVideoInfo->psCodecConfig     = NULL;
        m_pVideoInfo->ulCodecConfigSize = 0;
      }
      eRetStatus = PARSER_ErrorNone;
      break;
    }
    else if ((TAG_TYPE_VIDEO == pTagInfo->ucTagType) &&
             (FLV_H264 == eCodecType))
    {
      break;
    }
    eRetStatus = ParseFLVTag(ullOffset, pTagInfo);
  } while (PARSER_ErrorNone == eRetStatus);

  return eRetStatus;
}

/* ============================================================================
  @brief  Function to parse NAL Unit Params

  @details        This function is used to parse Codec config data.
                  Both SPS/PPS has same structure.
                  NAL size of 2 bytes followed by NAL data.

  @param[in]      pucDataBuf       Buffer pointer
  @param[in]      ulBufSize        Buffer size
  @param[in]      pTag             FLV Tag info structure

  @return     NALUDatatype structure pointer. This will be stored in class
  @note       Nothing.
============================================================================ */
NALUDatatype* FLVParser::UpdateNALUParams(uint8* pTemp, uint32 &ulIndex,
                                          uint32 ulBufSize,
                                          uint8 ucNALUParamCount)
{
  uint32 ulCount = 0;
  NALUDatatype *pNALUParam = NULL;
  //! Validate input params
  if ((!pTemp) || (ulIndex >= ulBufSize) || (0 == ucNALUParamCount))
  {
    return NULL;
  }
  //! Allocate memory to store NALU memory
  pNALUParam = (NALUDatatype*)MM_Malloc(sizeof(NALUDatatype)*ucNALUParamCount);

  /* NALU structure is as follows.
     <uint8>  Number of Parameters sets
     for (0 to Number of parameter sets)
       <uint16> Parameter sizer
       <uint8>  Config data
     end of for loop
  */
  while((ulCount < ucNALUParamCount) && (pNALUParam) && (ulIndex < ulBufSize))
  {
    memcpy(&pNALUParam[ulCount].usNALLength, pTemp + ulIndex,
           sizeof(uint16));
    FS_REVERSE_ENDIAN((uint8*)&pNALUParam[ulCount].usNALLength, 2);
    ulIndex += 2;
    if (pNALUParam[ulCount].usNALLength)
    {
      pNALUParam[ulCount].pucNALBuf =
        (uint8*)MM_Malloc(pNALUParam[ulCount].usNALLength);
    }
    if (pNALUParam[ulCount].pucNALBuf)
    {
      memcpy(pNALUParam[ulCount].pucNALBuf, pTemp + ulIndex,
             pNALUParam[ulCount].usNALLength);
    }
    ulIndex += pNALUParam[ulCount].usNALLength;

    ulCount++;
  }
  return pNALUParam;
}

/* ============================================================================
  @brief  Function to update index table

  @details    This function is used to check if the index table is present or
              not. If index table is present, gets the closest entry and parses
              Tag at the offset value.

  @param[in/out]  pTagInfo       Structure which has current tag properties.
  @param[in]      bForward       Flag to indicate seek direction
  @param[in]      ullReposTime   Seek time requested

  @return     PARSER_ErrorNone if closest known sample is found,
              Else returns corresponding error.
  @note       None
============================================================================ */
void FLVParser::UpdateIndexTable(uint64 ullFrameTime, FLVTagInfo *pTagInfo)
{
  uint64 ulEntry = 0;
  //! Ensure that clip duration is non-zero value
  if (!m_ullClipDuration)
  {
    return;
  }

  //! If memory for Index table is not allocated already, allocate memory
  if (!m_pIndexTable)
  {
    m_pIndexTable = (FLVIndexTable*)MM_Malloc(sizeof(FLVIndexTable) *
                                                    FLV_INDEX_TABLE_ENTRIES);
    if (m_pIndexTable)
    {
      memset(m_pIndexTable, 0,
             FLV_INDEX_TABLE_ENTRIES * sizeof(FLVIndexTable));
    }
    /* Calculate delta between two successive entries.
       If delta is 100ms, then it means we will have at least one entry for
       every 100ms worth of data. */
    m_ullIndexTableDelta = m_ullClipDuration / FLV_INDEX_TABLE_ENTRIES;
    //! If duration is less than total entries in index table, then store
    //! entry for each millisec
    if (m_ullClipDuration < FLV_INDEX_TABLE_ENTRIES)
    {
      m_ullIndexTableDelta = 1;
    }
  }

  if (m_ullIndexTableDelta)
  {
    ulEntry = ullFrameTime / m_ullIndexTableDelta;
  }

  /* Store only if timestamp calculated is less than already stored
     entry timestamp. In this way, we will ensure to have lower bound
     entries in the index table.
     If codec type is H264 and previously stored entry is Key frame,
     then do not update that entry. In this way, Parser ensures to have
     nearest key frame details. */
  if ( (m_pIndexTable) &&
       ((0 == m_pIndexTable[ulEntry].ullTimeStamp)||
        (ullFrameTime >= m_pIndexTable[ulEntry].ullTimeStamp &&
         !m_pIndexTable[ulEntry].bFrameType) ))
  {
    m_pIndexTable[ulEntry].ullTagOffset = pTagInfo->ullOffset;
    m_pIndexTable[ulEntry].ullTimeStamp = ullFrameTime;
    m_pIndexTable[ulEntry].ucTagType    = pTagInfo->ucTagType;
    //! This field is applicable only for H264
    m_pIndexTable[ulEntry].bFrameType = 0;
    if ((FLV_KEY_FRAME == pTagInfo->eFrameType) &&
        (TAG_TYPE_VIDEO == pTagInfo->ucTagType))
    {
      m_pIndexTable[ulEntry].bFrameType = 1;
      MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_MEDIUM,
         "FLVParser::UpdateIndexTable Key frame with Time %llu @ offset %llu",
         ullFrameTime, pTagInfo->ullOffset);
    }

    if (m_ulMaxIndexTableEntry < ulEntry)
    {
      m_ulMaxIndexTableEntry = (uint32)ulEntry;
    }
  }
  return;
}

/* ============================================================================
  @brief  Function to check index table and gets the closest entry

  @details    This function is used to check if the index table is present or
              not. If index table is present, gets the closest entry and parses
              Tag at the offset value.

  @param[in/out]  pTagInfo       Structure which has current tag properties.
  @param[in]      bForward       Flag to indicate seek direction
  @param[in]      ullReposTime   Seek time requested

  @return     PARSER_ErrorNone if closest known sample is found,
              Else returns corresponding error.
  @note       None
============================================================================ */
PARSER_ERRORTYPE FLVParser::GetClosestTagPosn(FLVTagInfo* pTagInfo,
                                              bool&       bForward,
                                              uint64      ullReposTime)
{
  PARSER_ERRORTYPE retError = PARSER_ErrorNone;
  uint32 ulIndex            = 0;
  uint32 ulPrevValidIndex   = 0;
  uint64 ullTime            = 0;
  uint64 ullOffset          = 0;

  //! Validate input params
  if(!pTagInfo)
  {
    return PARSER_ErrorInvalidParam;
  }

  //! Update current tag offset value
  ullOffset = pTagInfo->ullOffset;
  for ( ; (m_pIndexTable) && (ulIndex < m_ulMaxIndexTableEntry); ulIndex++)
  {
    if (m_pIndexTable[ulIndex].ullTimeStamp > ullReposTime)
    {
      /* Go to the previous entry only if forward timestamp is far away from
         seek time when compared with previous entry. */
      if ( (m_pIndexTable[ulIndex].ullTimeStamp - ullReposTime) >
           (ullReposTime - m_pIndexTable[ulPrevValidIndex].ullTimeStamp))
      {
        ullOffset = m_pIndexTable[ulPrevValidIndex].ullTagOffset;
        MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_MEDIUM,
          "FLVParser::Seek previous entry is selected @ %llu, time %llu",
          ullOffset, m_pIndexTable[ulPrevValidIndex].ullTimeStamp);
      }
      else
      {
        ullOffset = m_pIndexTable[ulIndex].ullTagOffset;
        MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_MEDIUM,
          "FLVParser::Seek forward entry is selected @ %llu, time %llu",
          ullOffset, m_pIndexTable[ulIndex].ullTimeStamp);
      }
      break;
    }
    /* If entry is valid, then update local offset parameter. */
    if (m_pIndexTable[ulIndex].ullTagOffset)
    {
      ullOffset        = m_pIndexTable[ulIndex].ullTagOffset;
      ulPrevValidIndex = ulIndex;
    }
  }

  /* Parse first tag available. Based on this tag value do seek in fwd or rew*/
  retError = ParseFLVTag(ullOffset,pTagInfo);
  ullTime  = pTagInfo->ulTime | (pTagInfo->ucExtendedTimeStamp << 24);
  ullTime += pTagInfo->ulCompositionTime;
  if (ullTime <= ullReposTime)
  {
    bForward = true;
  }
  else
  {
    bForward = false;
  }
  return retError;
}

/* ============================================================================
  @brief  Function to check whether current sample is sync sample or not.

  @details    This function is used to check if the current sample is sync
              sample or not.
              It also checks whether this is the sample required based on
              seek time requested.

  @param[in]      TagInfo        Structure which has current tag properties.
  @param[in]      prevIndexEntry Previous Sync sample properties
  @param[in]      bForward       Flag to indicate seek direction
  @param[in]      ullReposTime   Seek time requested
  @param[in/out]  pSampleInfo    Sample info structure

  @return     true if it is required sync sample, Else false
  @note       None
============================================================================ */
bool FLVParser::CheckCurrentSample(FLVTagInfo TagInfo,
                                   FLVIndexTable prevIndexEntry,
                                   bool bForward,
                                   uint64 ullReposTime,
                                   FLVStreamSampleInfo *pSampleInfo)
{
  uint64 ullTime   = 0;
  uint64 ullOffset = TagInfo.ullOffset;
  bool   bFound    = false;
  if (!pSampleInfo)
  {
    return bFound;
  }

  ullTime  = TagInfo.ulTime | (TagInfo.ucExtendedTimeStamp << 24);
  ullTime += TagInfo.ulCompositionTime;
  if ((bForward) && (ullTime >= ullReposTime))
  {
    /* Check if the current sample time stamp is farther than previous
       Seek entry point. In that case, Parser will seek to the previous
       entry. */
    if ((ullTime - ullReposTime) >
        (ullReposTime - prevIndexEntry.ullTimeStamp) &&
        (prevIndexEntry.ullTagOffset))
    {
      m_ullCurrOffset              = prevIndexEntry.ullTagOffset;
      pSampleInfo->ullSampleTime   = prevIndexEntry.ullTimeStamp;
      pSampleInfo->ullSampleOffset = m_ullCurrOffset;
      bFound = true;
    }
    else if ((TAG_TYPE_AUDIO == TagInfo.ucTagType ) ||
             (FLV_KEY_FRAME  == TagInfo.eFrameType))
    {
      m_ullCurrOffset              = ullOffset;
      pSampleInfo->ullSampleTime   = ullTime;
      pSampleInfo->ullSampleOffset = m_ullCurrOffset;
      bFound = true;
    }
  } // if ((bForward) && (ullTime >= ullReposTime))
  else if((!bForward) && (ullTime <= ullReposTime))
  {
    /* Check if the current sample time stamp is farther than previous
       Seek entry point. In that case, Parser will seek to the previous
       entry. */
    if ((ullReposTime - ullTime) >
        (prevIndexEntry.ullTimeStamp - ullReposTime) &&
        (prevIndexEntry.ullTagOffset))
    {
      m_ullCurrOffset              = prevIndexEntry.ullTagOffset;
      pSampleInfo->ullSampleTime   = prevIndexEntry.ullTimeStamp;
      pSampleInfo->ullSampleOffset = m_ullCurrOffset;
      bFound = true;
    }
    else if ((TAG_TYPE_AUDIO == TagInfo.ucTagType) ||
             (FLV_KEY_FRAME  == TagInfo.eFrameType))
    {
      m_ullCurrOffset              = ullOffset;
      pSampleInfo->ullSampleTime   = ullTime;
      pSampleInfo->ullSampleOffset = m_ullCurrOffset;
      bFound = true;
    }
  } // if((!bForward) && (ullTime <= ullReposTime))
  return bFound;
}

/* ============================================================================
  @brief  Function to check whether current sample is sync sample or not.

  @details    This function is used to check if the current sample is sync
              sample or not.

  @param[in]      pDataBuf     Buffer pointer in which media data is read.
  @param[in]      pTagInfo     Structure ptr which has current tag properties
  @param[in]      ulBufSize    Data size that read into buffer

  @return     PARSER_ErrorNone if data processed successfully, Else error
  @note       None
============================================================================ */
PARSER_ERRORTYPE FLVParser::IsKeyFrame(uint8* pDataBuf, FLVTagInfo *pTagInfo,
                                       uint32 ulBufSize)
{
  uint8  ucNALUSizeBytesLen = 4;
  uint32 ulIndex            = 0;
  //! Validate input and class params
  if ((!pDataBuf) || (!pTagInfo) || (!m_pVideoInfo)||
      (!m_pVideoInfo->psCodecConfig))
  {
    return PARSER_ErrorInvalidParam;
  }

  ucNALUSizeBytesLen = m_pVideoInfo->psCodecConfig->ucNALLengthMinusOne;
  ucNALUSizeBytesLen++;
  //! Go through the whole buffer to look for valid NALU types
  while(ulIndex < ulBufSize)
  {
    uint8  ucNALUType = 0;
    uint32 ulNALSize  = 0;
    bool bRet = GetNextH264NALUnit(ulIndex, ucNALUSizeBytesLen, pDataBuf,
                                   &ucNALUType, &ulNALSize, ulBufSize);
    if (NALU_IDR_TYPE == ucNALUType)
    {
      pTagInfo->eFrameType = FLV_KEY_FRAME;
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
        "FLVParser::IsKeyFrame true at offset %llu", pTagInfo->ullOffset);
      break;
    }
    else if (NALU_NON_IDR_TYPE == ucNALUType)
    {
      pTagInfo->eFrameType = FLV_NONKEY_FRAME;
      break;
    }
    //! If NALU is not either of the NALU types, then look for next NALU type
    if (true == bRet)
    {
      ulIndex += ulNALSize;
    }
    else
    {
      pTagInfo->eFrameType = FLV_NONKEY_FRAME;
      break;
    }
  }

  return PARSER_ErrorNone;
}

/* ============================================================================
  @brief  Function to get NALU type and size info

  @details    This function is used to get current NALU properties.

  @param[in]     ulIndex       Buf Index value to indicate amount of data read
  @param[in]     NALUSizeLen   NAL Unit Size field length in bytes
  @param[in]     pDataBuf      Buffer pointer in which media data is read
  @param[in/out] pucNALType    Variable to update NAL type
  @param[in/out] pulNALSize    Variable to store NAL Unit Size in bytes
  @param[in]     ulBufSize     Data size that read into buffer

  @return     true if sufficient data is available, Else false
  @note       None
============================================================================ */
bool FLVParser::GetNextH264NALUnit(uint32 ulIndex, uint8 ucNALUSizeLen,
                                   uint8* pDataBuf, uint8* pucNALType,
                                   uint32* pulNALSize, uint32 ulBufSize)
{
  bool   bRet = false;
  uint32 ulNALIndex = 0;
  //! Validate input parameters
  if ((!pDataBuf) || (!pucNALType) || (!pulNALSize) || (!ucNALUSizeLen) ||
      (ulIndex >= ulBufSize))
  {
    return bRet;
  }
  //! Ensure we have minimum data required
  if((ulIndex + ucNALUSizeLen + 1) > ulBufSize)
  {
    return bRet;
  }
  //! Initialize output params
  *pulNALSize = 0;
  *pucNALType = NALU_NON_IDR_TYPE;
  for (; (ulNALIndex < ucNALUSizeLen); ++ulNALIndex)
  {
    *pulNALSize = *pulNALSize << 8;
    *pulNALSize = *pulNALSize | pDataBuf[ulIndex + ulNALIndex];
  }
  //! NALUnit Size will not include NALU Size Length bytes
  *pulNALSize += ulNALIndex;
  ulIndex     += ulNALIndex;
  //! Update NAL Type field, first byte will contain NAL Type info
  *pucNALType = pDataBuf[ulIndex] & 0x1F;
  return true;
}

