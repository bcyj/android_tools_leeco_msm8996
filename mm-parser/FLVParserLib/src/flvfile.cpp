/* =======================================================================
                              FLVfile.cpp
DESCRIPTION
Flash File class definition

EXTERNALIZED FUNCTIONS
None

INITIALIZATION AND SEQUENCING REQUIREMENTS
  Detail how to initialize and use this service.  The sequencing aspect
  is only needed if the order of operations is important.

Copyright (c) 2012-2014 QUALCOMM Technologies Inc, All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
========================================================================== */

/* =======================================================================
                             PERFORCE HEADER
$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/FLVParserLib/main/latest/src/flvfile.cpp#8 $
========================================================================== */

/* ==========================================================================

                     INCLUDE FILES FOR MODULE
========================================================================== */
#include "flvparser.h"
#include "flvfile.h"
#include "MMMalloc.h"
#include "atomdefs.h"
#include "utf8conv.h"

#define FLV_FILE_CACHING_SIZE 128000

/* ==========================================================================

                DEFINITIONS AND DECLARATIONS FOR MODULE

This section contains definitions for constants, macros, types, variables
and other items needed by this module.

========================================================================== */
/* -----------------------------------------------------------------------
** Constant / Define Declarations
** ----------------------------------------------------------------------- */
/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */
/* -----------------------------------------------------------------------
** Local Object Definitions
** ----------------------------------------------------------------------- */
/* -----------------------------------------------------------------------
** Forward Declarations
** ----------------------------------------------------------------------- */
/* =======================================================================
                            Function Definitions
** ======================================================================= */

/* ======================================================================
FUNCTION
  FLVCallbakGetData

DESCRIPTION
  Its a callback function from the SDK to get the FLV data for decoding.
  This is not implemented by the SDK. It should be implemented by the app
  that calls the SDK.

ARGUMENTS
  ullOffset           Offset of the requested data (from beginning),
  ulNumBytesRequest   Size of the requested data (in bytes).
  pucData             Pointer to data buffer.
  ulMaxBufSize        Maximum data buffer size.
  pUserData           Client user data information.

DEPENDENCIES
  Used by FLV Parser.

RETURN VALUE
  The number of bytes actually being filled in

SIDE EFFECTS
  Detail any side effects.
========================================================================== */
uint32 FLVCallbakGetData ( uint64 ullOffset,
                           uint32 ulNumBytesRequest,
                           uint8* pucData,
                           uint32 ulMaxBufSize,
                           void*  pUserData )
{
  if(pUserData)
  {
    FLVFile* pFLV = (FLVFile*)pUserData;
    return ( pFLV->FileGetData( ullOffset,
                                ulNumBytesRequest,
                                ulMaxBufSize,
                                pucData ) );
  }
  return 0;
}
/* ======================================================================
FUNCTION:
  FLVFile::FileGetData

DESCRIPTION:
  To read the data from the file
INPUT/OUTPUT PARAMETERS:
  nOffset         : Offset from which data is being requested
  nNumBytesRequest: Total number of bytes requested
  ppData          : Buffer to be used for reading the data

RETURN VALUE:
 Number of bytes read

SIDE EFFECTS:
  None.
======================================================================*/
uint32 FLVFile::FileGetData(uint64 nOffset, uint32 nNumBytesRequest,
                            uint32 nMaxSize, uint8* pData  )
{
  uint32 nRead = 0;
  if( (m_pFilePtr != NULL)&& (!m_bMediaAbort) )
  {
    nRead = FileBase::readFile(m_pFilePtr, pData, nOffset,
                               FILESOURCE_MIN(nNumBytesRequest,
                                              nMaxSize));
  }
  return nRead;
}

/* ======================================================================
FUNCTION:
  FLVFile::FLVFile

DESCRIPTION:
  FLV constructor

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
  None.

SIDE EFFECTS:
  None.
======================================================================*/
FLVFile::FLVFile(const FILESOURCE_STRING filename
                     ,unsigned char* /*pFileBuf*/
                     ,uint32 /*bufSize*/
                     ,bool bPlayVideo
                     ,bool bPlayAudio
                    )
{
  InitData();
  m_playAudio = bPlayAudio;
  m_playVideo = bPlayVideo;
  m_pFilePtr = OSCL_FileOpen(filename, (OSCL_TCHAR *) _T("rb"),
                             FLV_FILE_CACHING_SIZE);
  m_fileSize = OSCL_FileSize(filename);
  m_pFLVParser = MM_New_Args(FLVParser,(this, m_fileSize, bPlayAudio));
  (void)ParseMetaData();
  m_bIsMetaDataParsed = true;
}

/* ======================================================================
FUNCTION:
  FLVFile::FLVFile

DESCRIPTION:
  FLV constructor for iStreamPort Interface

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
  None.

SIDE EFFECTS:
  None.
======================================================================*/
#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
FLVFile::FLVFile(video::iStreamPort* pport , bool bPlayVideo, bool bPlayAudio)
{
  InitData();
  m_pPort          = pport;
  m_playAudio      = bPlayAudio;
  m_playVideo      = bPlayVideo;
  m_bHttpStreaming = true;
  m_pFilePtr = OSCL_FileOpen(pport);
  m_fileSize = MAX_FILE_SIZE;
  if(m_pPort)
  {
    int64 sllOffset = 0;
    if(video::iStreamPort::DS_SUCCESS == m_pPort->GetContentLength(&sllOffset))
    {
      m_fileSize = (uint64)sllOffset;
    }
  }
  m_pFLVParser = MM_New_Args(FLVParser,(this, m_fileSize, bPlayAudio));
  (void)parseHTTPStream();
}

/* ======================================================================
FUNCTION:
  FLVFile::parseHTTPStream

DESCRIPTION:
  Function used to parse HTTP stream

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
  None.

SIDE EFFECTS:
  None.
======================================================================*/
bool FLVFile::parseHTTPStream ( void )
{
  if(m_pFLVParser && m_bHttpStreaming && (!m_bIsMetaDataParsed) )
  {
    m_bIsMetaDataParsed = ParseMetaData();
  }//if(m_pFLVParser && (!m_bIsMetaDataParsed) )
  return m_bIsMetaDataParsed;
}
#endif

/* ======================================================================
FUNCTION:
  FLVFile::InitData

DESCRIPTION:
  Initializes all the data members to default

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
  None.

SIDE EFFECTS:
  None.
======================================================================*/
void FLVFile::InitData()
{
  m_playAudio         = false;
  m_playVideo         = false;
  m_playText          = false;
  m_corruptFile       = false;
  m_bIsMetaDataParsed = false;
  m_bMediaAbort       = false;
  m_nNumStreams       = 0;
  m_fileSize          = 0;
  m_pFilePtr          = NULL;
  m_pFLVParser        = NULL;
  m_pIndTrackIdTable  = NULL;
  m_bHttpStreaming    = false;
  _success            = false;
  _fileErrorCode      = PARSER_ErrorDefault;
  memset(&m_sampleInfo, 0, (FILE_MAX_MEDIA_STREAMS *
                            sizeof(file_sample_info_type)));
#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
  m_pPort            = NULL;
#endif
}

/* ======================================================================
FUNCTION:
  FLVFile::SetCriticalSection

DESCRIPTION:
  Critical section will be created.

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
  None.

SIDE EFFECTS:
  None.
======================================================================*/
void FLVFile::SetCriticalSection(MM_HANDLE pcriticalsection)
{
  if(m_pFilePtr)
  {
    m_pFilePtr->pCriticalSection = pcriticalsection;
  }
}
/* ======================================================================
FUNCTION:
  FLVFile::~FLVFile

DESCRIPTION:
  FLV destructor

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
  None.

SIDE EFFECTS:
  None.
======================================================================*/
FLVFile::~FLVFile()
{
  if(m_pFLVParser)
  {
    MM_Delete(m_pFLVParser);
  }
  if(m_pIndTrackIdTable)
  {
    MM_Free(m_pIndTrackIdTable);
  }
}

/* ======================================================================
FUNCTION:
  FLVFile::ParseMetaData

DESCRIPTION:
  Starts parsing the Flash Video File.

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
  None.

SIDE EFFECTS:
  None.
======================================================================*/
bool FLVFile::ParseMetaData()
{
  bool bRet = false;
  if(m_pFLVParser)
  {
    if(m_pFLVParser->StartParsing() == PARSER_ErrorNone)
    {
      bRet = true;
      _success = true;
      _fileErrorCode = PARSER_ErrorNone;
      m_nNumStreams = m_pFLVParser->GetTotalNumberOfTracks();
      if(m_nNumStreams)
      {
        uint32* idlist = (uint32*)MM_Malloc(m_nNumStreams * sizeof(uint32) );
        m_pIndTrackIdTable = (FLVTrackIdToIndexTable*)MM_Malloc(
                               m_nNumStreams * sizeof(FLVTrackIdToIndexTable));
        if(m_pIndTrackIdTable && idlist)
        {
          memset(m_pIndTrackIdTable, 0,
                 m_nNumStreams * sizeof(FLVTrackIdToIndexTable));
          if(m_pFLVParser->GetTrackWholeIDList(idlist) == m_nNumStreams)
          {
            for(uint32 i = 0; i < m_nNumStreams; i++)
            {
              m_pIndTrackIdTable[i].index   = (uint8)i;
              m_pIndTrackIdTable[i].bValid  = true;
              m_pIndTrackIdTable[i].trackId = idlist[i];
            }
          }
        }
        if(idlist)
        {
          MM_Free(idlist);
        }
      }
    }
  }
  return bRet;
}
/* ======================================================================
FUNCTION:
  FLVFile::getNextMediaSample

DESCRIPTION:
  gets next sample of the given track.

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 size of sample

SIDE EFFECTS:
  None.
======================================================================*/
PARSER_ERRORTYPE FLVFile::getNextMediaSample(uint32 ulTrackId, uint8 *pucBuf,
                                             uint32* pulBufSize,
                                             uint32& /*ucrIndex*/)
{
  int32 nBytes  = 0;
  uint8 ucIndex = 0;
  PARSER_ERRORTYPE eRetStatus = PARSER_ErrorDefault;

  //! Validate input params
  if ((!m_pFLVParser) || (!pucBuf) || (!pulBufSize) || (0 == *pulBufSize))
  {
    eRetStatus = PARSER_ErrorInvalidParam;
  }
  else
  {
    bool bError = false;
    eRetStatus  = PARSER_ErrorNone;
    ucIndex     = MapTrackIdToIndex(&bError,ulTrackId);
    if (bError)
    {
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                   "FLVFile::getNextMediaSample failed for trackid %lu",
                   ulTrackId);
      eRetStatus = PARSER_ErrorInvalidTrackID;
    }
  }
  if(PARSER_ErrorNone == eRetStatus)
  {
    MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_MEDIUM,
                 "FLVFile::getNextMediaSample for track %lu, size %lu",
                 ulTrackId, *pulBufSize);

    FLVStreamSampleInfo sampleinfo;
    memset(&sampleinfo,0,sizeof(FLVStreamSampleInfo));
    eRetStatus = m_pFLVParser->GetCurrentSample(ulTrackId, pucBuf, *pulBufSize,
                                                &nBytes, &sampleinfo);
    if(nBytes > 0 && PARSER_ErrorNone == eRetStatus)
    {
      if(sampleinfo.ullSampleTime > m_sampleInfo[ucIndex].time)
      {
        m_sampleInfo[ucIndex].delta = (sampleinfo.ullSampleTime -
                                       m_sampleInfo[ucIndex].time);
      }
      m_sampleInfo[ucIndex].num_frames = 1;
      m_sampleInfo[ucIndex].sample     = sampleinfo.ulSampleNum;
      m_sampleInfo[ucIndex].size       = sampleinfo.ullSize;
      m_sampleInfo[ucIndex].sync       = sampleinfo.bsync;
      m_sampleInfo[ucIndex].offset     = sampleinfo.ullSampleOffset;
      m_sampleInfo[ucIndex].time       = sampleinfo.ullSampleTime;
      MM_MSG_PRIO4(MM_FILE_OPS, MM_PRIO_MEDIUM,
                   "FLVFile::getNextMediaSample Sample# %lu TIME %llu \
                    SampleSize %ld TrackId %lu",
                   m_sampleInfo[ucIndex].sample,
                   m_sampleInfo[ucIndex].time, nBytes, ulTrackId);

    } //if(nBytes > 0 && PARSER_ErrorNone == eRetStatus)
    *pulBufSize = nBytes;
  }
  return eRetStatus;
}

/* ======================================================================
FUNCTION:
  FLVFile::getMediaTimestampForCurrentSample

DESCRIPTION:
  gets time stamp of current sample of the track.

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 time stamp in track time scale unit

SIDE EFFECTS:
  None.
======================================================================*/
uint64 FLVFile::getMediaTimestampForCurrentSample(uint32 id)
{
  bool bError = false;
  uint64 ts = 0;
  uint8 index = MapTrackIdToIndex(&bError,id);
  if(!bError)
  {
    ts = m_sampleInfo[index].time;
  }
  return ts;
}
/* ======================================================================
FUNCTION:
  FLVFile::skipNSyncSamples

DESCRIPTION:
  Skips specified sync samples in forward or backward direction.

INPUT/OUTPUT PARAMETERS:

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
//Need to be implemented when we add support for VIDEO as skipping
//few frames for audio does not really make sense as each frame is only few
//milliseconds
/* ======================================================================
FUNCTION:
  FLVFile::resetPlayback

DESCRIPTION:
  resets the playback time to given time(pos) for a track.
  Also tells if we need to goto closest sync sample or not.

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
uint64 FLVFile::resetPlayback(uint64 ullReposTime,
                              uint32 ulTrackId,
                              bool   /*bSetToSyncSample*/,
                              bool   *bError,
                              uint64 ullCurPosTimeStamp)
{
  uint64 newTS = 0;
  PARSER_ERRORTYPE retStatus = PARSER_ErrorDefault;

  bool bForward = (ullReposTime > ullCurPosTimeStamp)?1:0;
  FLVStreamSampleInfo SampleInfo;
  memset(&SampleInfo,0,sizeof(FLVStreamSampleInfo));

  MM_MSG_PRIO3(MM_FILE_OPS, MM_PRIO_HIGH,
               "resetPlayback id %lu repos_time %llu currentPosTimeStamp %llu",
               ulTrackId, ullReposTime, ullCurPosTimeStamp);

  if(bError && m_pFLVParser)
  {
    *bError = true;
    retStatus = m_pFLVParser->Seek(ulTrackId, ullReposTime, ullCurPosTimeStamp,
                                   &SampleInfo, bForward, false, 0);
    if(PARSER_ErrorNone == retStatus)
    {
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
                   "Seek Succeed, new TS %llu", SampleInfo.ullSampleTime);
      *bError = false;
      _fileErrorCode = PARSER_ErrorNone;
      bool bMapError = false;
      uint8 ucIndex = MapTrackIdToIndex(&bMapError,ulTrackId);
      if(!bMapError)
      {
        m_sampleInfo[ucIndex].num_frames = 1;
        m_sampleInfo[ucIndex].time       = SampleInfo.ullSampleTime;
        m_sampleInfo[ucIndex].sample     =  SampleInfo.ulSampleNum;
        m_sampleInfo[ucIndex].size       = SampleInfo.ullSize;
        m_sampleInfo[ucIndex].sync       = 1;
        newTS = m_sampleInfo[ucIndex].time;
      }
      else
      {
        MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_FATAL,
                     "!!!MapTrackIdToIndex failed for trackid %lu!!!",
                     ulTrackId);
        *bError = true;
        _fileErrorCode = PARSER_ErrorInvalidTrackID;
      }
    }
    else
    {
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
                   "Seek Failed for trackId %lu", ulTrackId);
      *bError = true;
     _fileErrorCode = PARSER_ErrorSeekFail;
    }
  }
  return newTS;
}

/* ======================================================================
FUNCTION:
  FLVFile::getMovieDurationMsec

DESCRIPTION:
  gets movie duration in milli secs

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
uint64 FLVFile::getMovieDurationMsec() const
{
  uint64 nDuration = 0;
  if(!m_pFLVParser)
  {
    return nDuration;
  }
  nDuration = m_pFLVParser->GetClipDurationInMsec();
  return nDuration;
}

/* ======================================================================
FUNCTION:
  AVIFile::getTrackWholeIDList

DESCRIPTION:
  gets list of track IDs

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
uint32 FLVFile::getTrackWholeIDList(uint32 *ids)
{
  if(!m_pFLVParser)
  {
    return 0;
  }
  return (m_pFLVParser->GetTrackWholeIDList(ids));
}

/* ======================================================================
FUNCTION:
  FLVFile::getTrackVideoFrameRate

DESCRIPTION:
  gets track video (if video) frame rate

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
float FLVFile::getTrackVideoFrameRate(uint32 id)
{
  float frate = 0.0;
  if(m_pFLVParser)
  {
    frate = m_pFLVParser->GetVideoFrameRate(id);
  }
  return frate;
}
/* ======================================================================
FUNCTION:
  FLVFile::getTrackVideoFrameWidth

DESCRIPTION:
  returns video track's frame width.

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
uint32 FLVFile::getTrackVideoFrameWidth(uint32 id)
{
  uint32 width = 0;
  if(m_pFLVParser)
  {
    width = m_pFLVParser->GetVideoWidth(id);
  }
  return width;
}
/* ======================================================================
FUNCTION:
  AVIFile::getTrackVideoFrameHeight

DESCRIPTION:
  returns video track's frame height.

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
uint32 FLVFile::getTrackVideoFrameHeight(uint32 id)
{
  uint32 height = 0;
  if(m_pFLVParser)
  {
    height = m_pFLVParser->GetVideoHeight(id);
  }
  return height;
}

/* ======================================================================
FUNCTION:
  FLVFile::getTrackAudioSamplingFreq

DESCRIPTION:
  gets audio track's sampling frequency

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
uint32 FLVFile::getTrackAudioSamplingFreq(uint32 id)
{
  uint32 freq = 0;
  if(m_pFLVParser)
  {
    freq = m_pFLVParser->GetAudioSamplingFrequency(id);
  }
  return freq;
}
/* ======================================================================
FUNCTION:
  FLVFile::GetNumAudioChannels

DESCRIPTION:
  returns number of audio channels

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
uint32 FLVFile::GetNumAudioChannels(int id)
{
  uint32 nchnls = 0;
  if(m_pFLVParser)
  {
    nchnls = m_pFLVParser->GetNumberOfAudioChannels(id);
  }
  return nchnls;
}
/* ==========================================================================
FUNCTION:
  FLVFile::peekCurSample

DESCRIPTION:
  gets information about current sample of a track

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
=============================================================================*/
PARSER_ERRORTYPE FLVFile::peekCurSample(uint32                trackid,
                                        file_sample_info_type *pSampleInfo)
{
  PARSER_ERRORTYPE reterror = PARSER_ErrorDefault;
  if((!m_pFLVParser) || (!pSampleInfo))
  {
    reterror = PARSER_ErrorDefault;
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,"FLVFile::peekCurSample invalid argument");
  }
  else
  {
    bool bError = false;
    uint8 index = MapTrackIdToIndex(&bError,trackid);
    if(!bError)
    {
      *pSampleInfo = m_sampleInfo[index];
      reterror = PARSER_ErrorNone;
    }
  }
  return reterror;
}
/* ======================================================================
FUNCTION:
  FLVFile::getTrackOTIType

DESCRIPTION:
  gets track OTI value

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
uint8 FLVFile::getTrackOTIType(uint32 id)
{
  uint8 format = 0xFF;
  if(m_pFLVParser)
  {
    FLVMediaCodecType eCodecType = m_pFLVParser->GetTrackCodecType(id);
    if(eCodecType == FLV_ADPCM_AUDIO)
    {
      format = ADPCM_AUDIO;
    }
    else if(eCodecType == FLV_MP3_AUDIO)
    {
      format = MP3_AUDIO;
    }
    else if(eCodecType == FLV_AAC)
    {
      format = MPEG4_AUDIO;
    }
    else if(eCodecType == FLV_H264)
    {
      format = H264_VIDEO;
    }
  }
  return format;
}
/* ======================================================================
FUNCTION:
  FLVFile::getTrackMaxBufferSizeDB

DESCRIPTION:
  gets maximum buffer size to play the track

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
int32  FLVFile::getTrackMaxBufferSizeDB(uint32 id)
{
  int32 bufsize = 0;
  if(m_pFLVParser)
  {
    bufsize = m_pFLVParser->GetTrackBufferSize(id);
  }
  return bufsize;
}

/* ======================================================================
FUNCTION:
  FLVFile::getTrackDecoderSpecificInfoContent

DESCRIPTION:
  this returns codec Header/size for given track id

INPUT/OUTPUT PARAMETERS:
  @in id: Track identifier
  @in buf: Buffer to copy codec header
  @in/@out: pbufSize: Size of codec header
  When buf is NULL, function returns the size of codec header into pbufSize.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
PARSER_ERRORTYPE FLVFile::getTrackDecoderSpecificInfoContent(uint32  ulTrackId,
                                                             uint8*  pucBuf,
                                                             uint32* pulBufSize)
{
  PARSER_ERRORTYPE eRetError = PARSER_ErrorNone;
  if( (!m_pFLVParser) || (!pulBufSize) )
  {
    return PARSER_ErrorDefault;
  }
  uint32 ulCodecDataSize = m_pFLVParser->GetCodecHeaderSize(ulTrackId);
  if (pucBuf)
  {
    eRetError = m_pFLVParser->GetCodecHeader(ulTrackId, pucBuf, pulBufSize);
  }

  *pulBufSize = ulCodecDataSize;
  return eRetError;
}

/* ======================================================================
FUNCTION:
  FLVFile::MapTrackIdToIndex

DESCRIPTION:
  Maps given track-id to track index.

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
uint8 FLVFile::MapTrackIdToIndex(bool* pbError,uint32 trackid)
{
  uint8 index = 0;
  if(pbError)
  {
    *pbError = true;
    for(uint32 i = 0; i < m_nNumStreams; i++)
    {
      if( (m_pIndTrackIdTable[i].trackId == trackid) &&
        (m_pIndTrackIdTable[i].bValid) )
      {
        index = m_pIndTrackIdTable[i].index;
        *pbError = false;
        break;
      }
    }
  }
  if(pbError && *pbError)
  {
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                 "FLVFile::MapTrackIdToIndex failed for trackid %lu", trackid);
  }
  return index;
}
//#if defined(FEATURE_QTV_3GPP_PROGRESSIVE_DNLD)

/////*===========================================================================
////
////FUNCTION  updateBufferWritePtr
////
////DESCRIPTION
////  Public method used to update the write buffer offset during Http streaming.
////
////===========================================================================*/
////void FLVFile::updateBufferWritePtr ( uint32 writeOffset )
////{
////  WMFDecoderEx *pDecoder = (WMFDecoderEx *)m_hASFDecoder;
////  //Executing in the UI thread context.
////
////  if(pDecoder)
////  {
////    pDecoder->wHttpDataBuffer.Offset = writeOffset;
////    pDecoder->wHttpDataBuffer.bValid = TRUE;
////  }
////
////  if((parserState == Common::PARSER_PAUSE) || (parserState == Common::PARSER_RESUME))
////  {
////     //check if we got sufficient data to start parsing the
////     //meta data.
////     sendParseHTTPStreamEvent();
////  }
////}
////
////
/////*===========================================================================
////
////FUNCTION  getMetaDataSize
////
////DESCRIPTION
////  Public method used to determine the meta-data size of the fragment.
////
////===========================================================================*/
////tWMCDecStatus FLVFile::getMetaDataSize ( void )
////{
////  tWMCDecStatus wmerr = WMCDec_Fail;
////  uint32 nHttpDownLoadBufferOffset = 0;
////  boolean bHttpDownLoadBufferOffsetValid = GetHTTPStreamDownLoadedBufferOffset(&nHttpDownLoadBufferOffset);
////  WMFDecoderEx *pDecoder = (WMFDecoderEx *)m_hASFDecoder;
////  U32_WMC nAsfHeaderSize = 0;
////
////  if( pDecoder && bHttpDownLoadBufferOffsetValid && (nHttpDownLoadBufferOffset > (MIN_OBJECT_SIZE + sizeof(U32_WMC) + 2*sizeof(U8_WMC)) ) )
////  {
////    wmerr = GetAsfHeaderSize(&m_hASFDecoder,&nAsfHeaderSize );
////  }
////  if(wmerr == WMCDec_Succeeded)
////  {
////    m_HttpDataBufferMinOffsetRequired.Offset = nAsfHeaderSize;
////    m_HttpDataBufferMinOffsetRequired.bValid = TRUE;
////    bGetMetaDataSize = FALSE;
////    return wmerr;
////  }
////  else
////  {
////    bGetMetaDataSize = TRUE;
////    return wmerr;
////  }
////
////  return WMCDec_Fail;
////}
////
/////*===========================================================================
////
////FUNCTION  parseHTTPStream
////
////DESCRIPTION
////  Public method used to parse the Http Stream.
////
////===========================================================================*/
////bool FLVFile::parseHTTPStream ( void )
////{
////
////  tWMCDecStatus wmerr = WMCDec_Succeeded;
////  bool returnStatus = true;
////  uint32 nHttpDownLoadBufferOffset = 0;
////  boolean bHttpDownLoadBufferOffsetValid = GetHTTPStreamDownLoadedBufferOffset(&nHttpDownLoadBufferOffset);
////
////
////  if(bGetMetaDataSize)
////  {
////     wmerr = getMetaDataSize();
////  }
////
////  if(wmerr != WMCDec_Succeeded)
////  {
////    //QTV_PS_PARSER_STATUS_PAUSED
////    sendParserEvent(Common::PARSER_PAUSE);
////    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH, "Parser State = Common::PARSER_PAUSE");
////    returnStatus = false;
////  }
////  else if(wmerr == WMCDec_Succeeded)
////  {
////     if((nHttpDownLoadBufferOffset >= m_HttpDataBufferMinOffsetRequired.Offset)
////        && bHttpDownLoadBufferOffsetValid && m_HttpDataBufferMinOffsetRequired.bValid)
////     {
////       if( !bIsMetaDataParsed )
////       {
////         if(ParseMetaData() == WMCDec_Succeeded)
////     {
////           //QTV_PS_PARSER_STATUS_RESUME
////           bIsMetaDataParsed = TRUE;
////           m_HttpDataBufferMinOffsetRequired.bValid = FALSE;
////           sendParserEvent(Common::PARSER_RESUME);
////           returnStatus = true;
////           MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH, "Parser State = Common::PARSER_RESUME");
////         }
////     else
////     {
////           //QTV_PS_PARSER_STATUS_PAUSED
////           sendParserEvent(Common::PARSER_PAUSE);
////           MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH, "Parser State = Common::PARSER_PAUSE");
////           returnStatus = false;
////     }
////       }
////     }
////
////     if ((parserState == Common::PARSER_RESUME) && CanPlayTracks(m_startupTime) )
////     {
////        //QTV_PS_PARSER_STATUS_READY
////        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH, "Parser State = Common::PARSER_READY");
////        sendParserEvent(Common::PARSER_READY);
////    returnStatus = true;
////      }
////      else
////      {
////        returnStatus = false;
////      }
////  }
////
////  return returnStatus;
////}
////
////
/////*===========================================================================
////
////FUNCTION  sendHTTPStreamUnderrunEvent
////
////DESCRIPTION
////  Public method used to switch contexts and notify the player about buffer-underrun.
////
////===========================================================================*/
////void FLVFile::sendHTTPStreamUnderrunEvent(void)
////{
////  QTV_HTTP_STREAM_BUFFER_UNDERRUN_EVENT_type *pEvent = QCCreateMessage(QTV_HTTP_STREAM_BUFFER_UNDERRUN_EVENT, m_pMpeg4Player);
////
////  if (pEvent)
////  {
////    pEvent->bAudio = (bool) m_playAudio;
////    pEvent->bVideo = (bool) m_playVideo;
////    pEvent->bText = (bool) m_playText;
////    QCUtils::PostMessage(pEvent, 0, NULL);
////  }
////}
/////*===========================================================================
////
////FUNCTION  GetHTTPStreamDownLoadedBufferOffset
////
////DESCRIPTION
////  Public method used to switch contexts and notify the player about buffer-underrun.
////
////===========================================================================*/
////boolean FLVFile::GetHTTPStreamDownLoadedBufferOffset(U32_WMC * pOffset)
////{
////  WMFDecoderEx *pDecoder = (WMFDecoderEx *)m_hASFDecoder;
////
////  if(pDecoder && pOffset)
////  {
////    if(m_fpFetchBufferedDataSize)
////    {
////      //Pull interface so pull dnld data size from OEM
////      m_fpFetchBufferedDataSize( 0, &(pDecoder->wHttpDataBuffer.Offset, m_QtvInstancehandle) );
////      pDecoder->wHttpDataBuffer.bValid = TRUE;
////    }
////    if( pDecoder->wHttpDataBuffer.bValid )
////    {
////      *pOffset = pDecoder->wHttpDataBuffer.Offset;
////      return TRUE;
////    }
////  }
////  return FALSE;
////}
////
///*===========================================================================
//
//FUNCTION  GetTotalAvgBitRate
//
//DESCRIPTION
//  Public method used to switch contexts and notify the player about buffer-underrun.
//
//===========================================================================*/
//bool FLVFile::GetTotalAvgBitRate(uint32 * pBitRate)
//{
//
//  return true;
//}
//
///*===========================================================================
//
//FUNCTION  CanPlayTracks
//
//DESCRIPTION
//  Public method used to switch contexts and notify the player about buffer-underrun.
//
//===========================================================================*/
//bool FLVFile::CanPlayTracks(uint32 nTotalPBTime)
//{
//    return true;
//}
//
///* ======================================================================
//FUNCTION:
//  FLVFile::GetMediaMaxTimeStampPlayable
//
//DESCRIPTION:
//  gets time stamp of current sample of the track.
//
//INPUT/OUTPUT PARAMETERS:
//  None.
//
//RETURN VALUE:
// time stamp in track time scale unit
//
//SIDE EFFECTS:
//  None.
//======================================================================*/
////tWMCDecStatus FLVFile::GetMediaMaxPlayableTime(U32_WMC *nMaxPBTime)
////{
////  uint32 nMaxPlayableTime = 0;  // default max playable time
////
////  if( (m_pStreamDecodePattern == NULL) || (nMaxPBTime == NULL) )
////  {
////    return WMCDec_InValidArguments;
////  }
////  for(uint16 i=0; i<(int)m_nNumStreams; i++)
////  {
////    if( m_maxPlayableTime[i] && (m_pStreamDecodePattern[i].tPattern != Discard_WMC) )
////    {
////      if(!nMaxPlayableTime)
////      {
////        /* initialize with valid track sample time */
////        nMaxPlayableTime = m_maxPlayableTime[i];
////        continue;
////      }
////      /* Take the MIN value to make sure all tracks are playable atleast nMaxPlayableTime */
////      nMaxPlayableTime = MIN(m_maxPlayableTime[i],nMaxPlayableTime);
////    }
////  }
////
////    *nMaxPBTime = nMaxPlayableTime;
////
////  return WMCDec_Succeeded;
////}
//#endif //  FEATURE_QTV_3GPP_PROGRESSIVE_DNLD

