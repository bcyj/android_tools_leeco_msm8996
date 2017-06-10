/* =======================================================================
                              MKAVFile.cpp
DESCRIPTION
  Meaningful description of the definitions contained in this file.
  Description must specify if the module is portable specific, mobile
  specific, or common to both, and it should alert the reader if the
  module contains any conditional definitions which tailors the module to
  different targets.  Include any initialization and synchronizing
  requirements.

EXTERNALIZED FUNCTIONS
  List functions and a brief description that are exported from this file

INITIALIZATION AND SEQUENCING REQUIREMENTS
  Detail how to initialize and use this service.  The sequencing aspect
  is only needed if the order of operations is important.

Copyright (c) 2011-2015 Qualcomm Technologies Inc, All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
========================================================================== */

/* =======================================================================
                             PERFORCE HEADER
$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/MKAVParserLib/main/latest/src/mkavfile.cpp#48 $
========================================================================== */

/* ==========================================================================

                     INCLUDE FILES FOR MODULE
========================================================================== */
#include "mkavparser.h"
#include "mkavfile.h"
#include "MMMalloc.h"
#include "atomdefs.h"
#include "utf8conv.h"
#include "mkavparserconstants.h"
#include "zrex_string.h"
#include "parserdatadef.h"
#include "parserinternaldefs.h"
#define MKAV_FILE_CACHING_SIZE 128000
#define MIN_AAC_CODEC_HDR_SIZE 2

#ifdef FEATURE_FILESOURCE_MKV_PARSER
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
  MKAVFileCallbakGetData

DESCRIPTION
  Its a callback function from the SDK to get the MKV data for decoding.
  This is not implemented by the SDK. It should be implemented by the app
  that calls the SDK.

ARGUMENTS
  ullOffset           Offset of the requested data (from beginning),
  ulNumBytesRequest   Size of the requested data (in bytes).
  pucDataBuf          Pointer to data buffer.
  ulMaxBufSize        Maximum data buffer size.
  pUserData           Client user data information.

DEPENDENCIES
  Used by MKV Parser.

RETURN VALUE
  The number of bytes actually being filled in

SIDE EFFECTS
  Detail any side effects.
========================================================================== */
uint32 MKAVFileCallbakGetData (uint64 ullOffset, uint32 ulNumBytesRequest,
                               uint8* pucDataBuf, uint32  ulMaxBufSize,
                               void*  pUserData)
{
  uint32 ulDataRead = 0;
  if(pUserData)
  {
    MKAVFile* pMKAVFile = (MKAVFile*)pUserData;
    ulDataRead = pMKAVFile->FileGetData(ullOffset, ulNumBytesRequest,
                                        ulMaxBufSize, pucDataBuf);
  }
  return ulDataRead;
}
/* ======================================================================
FUNCTION
  MKAVCheckAvailableData

DESCRIPTION
  Its a callback function from MKAV Parser to determine how much data is available.
  This is not implemented by the parser.It should be implemented by the app that calls into the parser.

ARGUMENTS
  noffset                 Absolute file offset available
  bend                    Will be set to true if entire file is available
  u32UserData             Extra info From App. Given by user

DEPENDENCIES

RETURN VALUE
  True if successful in retrieving available offset, else returns false;

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
bool MKAVCheckAvailableData(uint64* noffset,bool* bend,void* pUserData)
{
  bool bret = false;
  if(pUserData)
  {
    MKAVFile* pMKAVFile = (MKAVFile*)pUserData;
    bret = pMKAVFile->CheckAvailableDataSize(noffset,bend);
  }
  return bret;
}
/* ======================================================================
FUNCTION:
  MKAVFile::FileGetData

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
uint32 MKAVFile::FileGetData(uint64 nOffset, uint32 nNumBytesRequest,
                               uint32 nMaxSize, uint8* pData  )
{
  uint32 nRead = 0;
  if( (m_pFilePtr != NULL)&& (!m_bMediaAbort) )
  {
    nRead = FileBase::readFile(m_pFilePtr, pData, nOffset,
                               FILESOURCE_MIN(nNumBytesRequest,nMaxSize));
  }
  return nRead;
}
/* ======================================================================
FUNCTION:
  MKAVFile::CheckAvailableDataSize

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
bool MKAVFile::CheckAvailableDataSize(uint64* offset,bool* bend)
{
  bool bret = false;
  if(offset && bend)
  {
    bret = true;
#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
    if(m_pPort && m_bHttpStreaming)
    {
      bret = false;
      if(m_pPort->GetAvailableOffset((int64*)offset,bend) ==
         video::iStreamPort::DS_SUCCESS)
      {
        bret = true;
      }
    }
    else
#endif
    {
      *offset = m_ullFileSize;
    }
  }
  return bret;
}
/* ======================================================================
FUNCTION:
  MKAVFile::MKAVFile

DESCRIPTION:
  MKAVFile constructor

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
  None.

SIDE EFFECTS:
  None.
======================================================================*/
MKAVFile::MKAVFile(const FILESOURCE_STRING filename
                     ,unsigned char* /*pFileBuf*/
                     ,uint32 /*bufSize*/
                     ,bool bPlayVideo
                     ,bool bPlayAudio
                    )
{
  InitData();
  m_playAudio = bPlayAudio;
  m_playVideo = bPlayVideo;
  m_pFilePtr = OSCL_FileOpen(filename, (OSCL_TCHAR *) _T("rb"),MKAV_FILE_CACHING_SIZE);
  m_ullFileSize = OSCL_FileSize(filename);
  m_pMKAVParser = MM_New_Args(MKAVParser,(this,m_ullFileSize,bPlayAudio,false));
  (void)ParseMetaData();
}
/* ======================================================================
FUNCTION:
  MKAVFile::MKAVFile

DESCRIPTION:
  MKAVFile constructor

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
  None.

SIDE EFFECTS:
  None.
======================================================================*/
#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
MKAVFile::MKAVFile(video::iStreamPort* pport , bool bPlayVideo, bool bPlayAudio)
{
  InitData();
  m_bHttpStreaming = true;
  m_pPort = pport;
  m_playAudio = bPlayAudio;
  m_playVideo = bPlayVideo;
  m_pFilePtr = OSCL_FileOpen(pport);

  m_ullFileSize = MAX_FILE_SIZE;
  if(m_pPort)
  {
    int64 noffset = 0;

    if(video::iStreamPort::DS_SUCCESS == m_pPort->GetContentLength(&noffset))
    {
      m_ullFileSize = (uint64)noffset;
    }
  }
  if(m_playAudio || m_playVideo)
  {
    m_pMKAVParser = MM_New_Args(MKAVParser,(this,m_ullFileSize,bPlayAudio,true));
    (void)parseHTTPStream();
  }
}
#endif
/* ======================================================================
FUNCTION:
  MKAVFile::InitData

DESCRIPTION:
  Initializes all the data members to default

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
  None.

SIDE EFFECTS:
  None.
======================================================================*/
void MKAVFile::InitData()
{
  m_playAudio   = false;
  m_playVideo   = false;
  m_playText    = false;
  m_nNumStreams = 0;

  memset(&m_sampleInfo,0,(FILE_MAX_MEDIA_STREAMS * sizeof(file_sample_info_type)) );
  memset(&m_nDecodedDataSize,0,(FILE_MAX_MEDIA_STREAMS * sizeof(uint32)) );
  memset(&m_nLargestFrame,0,(FILE_MAX_MEDIA_STREAMS * sizeof(uint32)) );

  m_ullFileSize      = 0;
  m_pFilePtr         = NULL;
  m_pMKAVParser      = NULL;
  m_pIndTrackIdTable = NULL;
  _success           = false;
  m_bHttpStreaming   = false;
  _fileErrorCode     = PARSER_ErrorDefault;
  m_bMediaAbort      = false;
#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
  m_pPort            = NULL;
#endif
#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
   bIsMetaDataParsed = false;
   m_nAvailableOffset = 0;
#endif//#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
}
/* ======================================================================
FUNCTION:
  MKAVFile::SetCriticalSection

DESCRIPTION:
  Sets the critical section for IStreamPort based playback

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
  None.

SIDE EFFECTS:
  None.
======================================================================*/
void MKAVFile::SetCriticalSection(MM_HANDLE pcriticalsection)
{
  if(m_pFilePtr)
  {
    m_pFilePtr->pCriticalSection = pcriticalsection;
  }
}
/* ======================================================================
FUNCTION:
  MKAVFile::~MKAVFile

DESCRIPTION:
  MKAVFile constructor

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
  None.

SIDE EFFECTS:
  None.
======================================================================*/
MKAVFile::~MKAVFile()
{
  if(m_pMKAVParser)
  {
    MM_Delete(m_pMKAVParser);
    m_pMKAVParser = NULL;
  }
  if(m_pIndTrackIdTable)
  {
    MM_Free(m_pIndTrackIdTable);
    m_pIndTrackIdTable = NULL;
  }
  if (m_pFilePtr)
  {
    OSCL_FileClose(m_pFilePtr);
    m_pFilePtr= NULL;
  }

}
/* ======================================================================
FUNCTION:
  MKAVFile::ParseMetaData

DESCRIPTION:
  Starts parsing the MPEG2 transport stream.

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
  None.

SIDE EFFECTS:
  None.
======================================================================*/
bool MKAVFile::ParseMetaData()
{
  bool bRet = false;
  if(m_pMKAVParser)
  {
    if(m_pMKAVParser->StartParsing() == MKAV_API_SUCCESS)
    {
      bRet = true;
      _success = true;
      _fileErrorCode = PARSER_ErrorNone;
      m_nNumStreams = m_pMKAVParser->GetTotalNumberOfTracks();
      if(m_nNumStreams)
      {
        uint32* idlist = (uint32*)MM_Malloc(m_nNumStreams * sizeof(uint32) );
        m_pIndTrackIdTable = (MKAVTrackIdToIndexTable*)MM_Malloc(m_nNumStreams * sizeof(MKAVTrackIdToIndexTable));
        if(m_pIndTrackIdTable && idlist)
        {
          memset(m_pIndTrackIdTable,0,m_nNumStreams * sizeof(MKAVTrackIdToIndexTable));
          if(m_pMKAVParser->GetTrackWholeIDList(idlist) == m_nNumStreams)
          {
            for(uint32 i = 0; i < m_nNumStreams; i++)
            {
              m_pIndTrackIdTable[i].index = (uint8)i;
              m_pIndTrackIdTable[i].bValid = true;
              m_pIndTrackIdTable[i].trackId = idlist[i];
            }
          }
        }
        if(idlist)
          MM_Free(idlist);
      }
    }
  }
  return bRet;
}
/* ======================================================================
FUNCTION   : getNextMediaSample
DESCRIPTION: gets next sample of the given track.

INPUT/OUTPUT PARAMETERS:
@param[in] ulTrackID  TrackID requested
@param[in] pucDataBuf DataBuffer pointer to fill the frame(s)
@param[in/out]
           pulBufSize Amount of data request /
                      Amount of data filled in Buffer
@param[in] rulIndex   Index

RETURN VALUE:
 PARSER_ErrorNone in Successful case /
 Corresponding error code in failure cases

SIDE EFFECTS:
  None.
======================================================================*/
PARSER_ERRORTYPE MKAVFile::getNextMediaSample(uint32 ulTrackID, uint8 *pucDataBuf,
                                              uint32 *pulBufSize, uint32& /*rulIndex*/)
{
  int32 lBufFilledLen  = 0;
  bool bError   = true;
  uint8 ucIndex = MapTrackIdToIndex(&bError, ulTrackID);
  MKAV_API_STATUS eStatus = MKAV_API_INVALID_PARAM;

  /* Validate input params and class variables */
  if(NULL == m_pMKAVParser || NULL == pulBufSize || NULL == pucDataBuf ||
     0 == *pulBufSize)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
               "getNextMediaSample invalid input params!!");
    return PARSER_ErrorInvalidParam;
  }
  if (bError)
  {
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_FATAL,
                 "getNextMediaSample MapTrackIdToIndex failed for trackid %lu",
                 ulTrackID);
    return PARSER_ErrorInvalidTrackID;
  }

#ifdef MKAV_PARSER_DEBUG
  MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_HIGH,
              "MKAVFile::getNextMediaSample for id %lu, Sample num %lu",
              ulTrackID, m_sampleInfo[ucIndex].sample);
#endif
  mkav_stream_sample_info sampleinfo;
  memset(&sampleinfo,0,sizeof(mkav_stream_sample_info));
  eStatus = m_pMKAVParser->GetCurrentSample(ulTrackID, pucDataBuf, *pulBufSize,
                                            &lBufFilledLen, &sampleinfo);
  if(lBufFilledLen > 0 && MKAV_API_SUCCESS == eStatus)
  {
    m_sampleInfo[ucIndex].num_frames = 1;
    m_sampleInfo[ucIndex].sample     = sampleinfo.nsample;
    m_sampleInfo[ucIndex].size       = sampleinfo.nsize;
    m_sampleInfo[ucIndex].sync       = 1;
    m_sampleInfo[ucIndex].offset     = (uint32)sampleinfo.noffset;
    if(sampleinfo.ntime > m_sampleInfo[ucIndex].time)
    {
      m_sampleInfo[ucIndex].delta = sampleinfo.ntime -
                                   m_sampleInfo[ucIndex].time;
    }
    m_sampleInfo[ucIndex].time       = sampleinfo.ntime;
    m_sampleInfo[ucIndex].btimevalid = true;
    #ifdef MKAV_PARSER_DEBUG
    MM_MSG_PRIO4(MM_FILE_OPS, MM_PRIO_HIGH,
       "getNextMediaSample Sample# %lu TIME %llu SampleSize %lu TrackId %lu",
       m_sampleInfo[ucIndex].sample, m_sampleInfo[ucIndex].time,
       sampleinfo.nsize, ulTrackID);
    #endif
  }
  else if(MKAV_API_INSUFFICIENT_BUFFER == eStatus)
  {
    *pulBufSize = lBufFilledLen;
    return PARSER_ErrorInsufficientBufSize;
  }
  else if(MKAV_API_DATA_UNDERRUN == eStatus)
  {
    *pulBufSize = 0;
    return PARSER_ErrorDataUnderRun;
  }

  *pulBufSize = lBufFilledLen;
  /* If error is other than success, report EOF straightaway. */
  if(MKAV_API_SUCCESS != eStatus)
    return PARSER_ErrorEndOfFile;
  else
    return PARSER_ErrorNone;
}
/* ======================================================================
FUNCTION:
  MKAVFile::getMediaTimestampForCurrentSample

DESCRIPTION:
  gets time stamp of current sample of the track.

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 time stamp in track time scale unit

SIDE EFFECTS:
  None.
======================================================================*/
uint64 MKAVFile::getMediaTimestampForCurrentSample(uint32 id)
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
  MKAVFile::skipNSyncSamples

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
  MKAVFile::randomAccessDenied

DESCRIPTION:
  gets if repositioning is allowed or not.

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 boolean value to indicate whether seek is allowed or not.

SIDE EFFECTS:
  None.
======================================================================*/
uint8 MKAVFile::randomAccessDenied()
{
  uint8 nRet = true;
  if(m_pMKAVParser)
  {
    nRet = m_pMKAVParser->randomAccessDenied();
  }
  return nRet;
}
/* ======================================================================
FUNCTION:
  AVIFile::resetPlayback

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
uint64 MKAVFile::resetPlayback(uint64 repos_time,
                                uint32 id,
                                bool /*bSetToSyncSample*/,
                                bool *bError,
                                uint64 currentPosTimeStamp)
{
  uint64 newTS = 0;

  bool bforward = (repos_time > currentPosTimeStamp)?1:0;
  mkav_stream_sample_info mkav_sampleInfo;
  memset(&mkav_sampleInfo,0,sizeof(mkav_stream_sample_info));

  MM_MSG_PRIO3(MM_FILE_OPS, MM_PRIO_HIGH,
               "resetPlayback id %lu repos_time %llu currentPosTimeStamp %llu",
                 id, repos_time, currentPosTimeStamp);

  if(bError && m_pMKAVParser)
  {
    *bError = true;
      if(MKAV_API_SUCCESS == m_pMKAVParser->Seek(id, repos_time,
                                                 currentPosTimeStamp,
                                                 &mkav_sampleInfo, bforward,
                                                 false, 0) )
    {
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
                   "Seek Succeed, new TS %llu", mkav_sampleInfo.ntime);
      *bError = false;
      _fileErrorCode = PARSER_ErrorNone;
      bool bMapError = false;
      uint8 index = MapTrackIdToIndex(&bMapError,id);
      if(!bMapError)
      {
        m_sampleInfo[index].num_frames = 1;
        m_sampleInfo[index].time = mkav_sampleInfo.ntime;
        m_sampleInfo[index].sample =  mkav_sampleInfo.nsample;
        m_sampleInfo[index].size = mkav_sampleInfo.nsize;
        m_sampleInfo[index].sync = 1;
        newTS = m_sampleInfo[index].time;
      }
      else
      {
        MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_FATAL,
                     "!!!MapTrackIdToIndex failed for trackid %lu!!!", id);
      }
    }
    else
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"Seek Failed");
      *bError = true;
      _fileErrorCode = PARSER_ErrorSeekFail;
    }
  }
  return newTS;
}
/* ======================================================================
FUNCTION:
  MKAVFile::getMovieDurationMsec

DESCRIPTION:
  gets movie duration in milli secs

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
uint64 MKAVFile::getMovieDurationMsec() const
{
  uint64 nDuration = 0;
  if(!m_pMKAVParser)
  {
    return nDuration;
  }
  nDuration = m_pMKAVParser->GetClipDurationInMsec();
  return nDuration;
}

/* ============================================================================
  @brief  getNumTracks.

  @details    This function is used to return
              total number of tracks available in file.

  @param[in]  None

  @return     Returns the channel mask available.
  @note       None.
============================================================================ */
int32 MKAVFile::getNumTracks()
{
  if(!m_pMKAVParser)
  {
    return 0;
  }
  return (m_pMKAVParser->GetTotalNumberOfTracks());
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
uint32 MKAVFile::getTrackWholeIDList(uint32 *ids)
{
  if(!m_pMKAVParser)
  {
    return 0;
  }
  return (m_pMKAVParser->GetTrackWholeIDList(ids));
}

/* ======================================================================
FUNCTION:
  MKAVFile::getTrackVideoFrameWidth

DESCRIPTION:
  returns video track's frame width.

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
uint32 MKAVFile::getTrackVideoFrameWidth(uint32 id)
{
  uint32 ulWidth = 0;
  if(m_pMKAVParser)
  {
    ulWidth = m_pMKAVParser->GetVideoWidth(id);
  }
  return ulWidth;
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
uint32 MKAVFile::getTrackVideoFrameHeight(uint32 id)
{
  uint32 ulHeight = 0;
  if(m_pMKAVParser)
  {
    ulHeight = m_pMKAVParser->GetVideoHeight(id);
  }
  return ulHeight;
}

/* ============================================================================
  @brief  GetAudioVirtualPacketSize.

  @details    This function is used to return
              WMA codec virtual packet size value .

  @param[in]  ulTrackId           Track ID.

  @return     Returns the packet size value.
  @note       None.
============================================================================ */
uint16 MKAVFile::GetAudioVirtualPacketSize(int ulTrackId)
{
  uint16 ulEncOptions2 = 0;
  if(m_pMKAVParser)
  {
    mkav_audio_info AudioInfo;
    memset(&AudioInfo, 0, sizeof(mkav_audio_info));
    m_pMKAVParser->GetAudioTrackProperties((uint32)ulTrackId, &AudioInfo);
    ulEncOptions2 = AudioInfo.usBlockAlign;
  }
  return ulEncOptions2;
}

/* ============================================================================
  @brief  GetAudioEncoderOptions.

  @details    This function is used to return
              WMA Encoder Options value.

  @param[in]  ulTrackId           Track ID.

  @return     Returns the WMA Encoder Options value.
  @note       None.
============================================================================ */
uint32 MKAVFile::GetAudioEncoderOptions(int ulTrackId)
{
  uint32 ulEncOptions2 = 0;
  if(m_pMKAVParser)
  {
    mkav_audio_info AudioInfo;
    memset(&AudioInfo, 0, sizeof(mkav_audio_info));
    m_pMKAVParser->GetAudioTrackProperties((uint32)ulTrackId, &AudioInfo);
    ulEncOptions2 = AudioInfo.usEncoderOptions;
  }
  return ulEncOptions2;
}

/* ============================================================================
  @brief  GetAudioAdvancedEncodeOptions.

  @details    This function is used to return
              WMA Advanced Encoder Options2 value.

  @param[in]  ulTrackId           Track ID.

  @return     Returns the WMA Advanced Encoder Options2 value.
  @note       None.
============================================================================ */
uint32 MKAVFile::GetAudioAdvancedEncodeOptions2(int ulTrackId)
{
  uint32 ulEncOptions2 = 0;
  if(m_pMKAVParser)
  {
    mkav_audio_info AudioInfo;
    memset(&AudioInfo, 0, sizeof(mkav_audio_info));
    m_pMKAVParser->GetAudioTrackProperties((uint32)ulTrackId, &AudioInfo);
    ulEncOptions2 = AudioInfo.ulAdvEncoderOptions2;
  }
  return ulEncOptions2;
}

/* ============================================================================
  @brief  GetAudioAdvancedEncodeOptions.

  @details    This function is used to return
              WMA Advanced Encoder Options.

  @param[in]  ulTrackId           Track ID.

  @return     Returns the WMA Advanced Encoder Options.
  @note       None.
============================================================================ */
uint16 MKAVFile::GetAudioAdvancedEncodeOptions(int ulTrackId)
{
  uint16 usEncOptions = 0;
  if(m_pMKAVParser)
  {
    mkav_audio_info AudioInfo;
    memset(&AudioInfo, 0, sizeof(mkav_audio_info));
    m_pMKAVParser->GetAudioTrackProperties((uint32)ulTrackId, &AudioInfo);
    usEncOptions = AudioInfo.usAdvEncoderOptions;
  }
  return usEncOptions;
}

/* ============================================================================
  @brief  GetFormatTag.

  @details    This function is used to return
              WMA Format Tag type.

  @param[in]  ulTrackId           Track ID.

  @return     Returns the WMA codec format tag value.
  @note       None.
============================================================================ */
uint16 MKAVFile::GetFormatTag(int ulTrackId)
{
  uint16 usFormatTag = 0;
  if(m_pMKAVParser)
  {
    mkav_audio_info AudioInfo;
    memset(&AudioInfo, 0, sizeof(mkav_audio_info));
    m_pMKAVParser->GetAudioTrackProperties((uint32)ulTrackId, &AudioInfo);
    usFormatTag = AudioInfo.usFormatTag;
  }
  return usFormatTag;
}

/* ============================================================================
  @brief  GetBlockAlign.

  @details    This function is used to return
              WMA Codecs Block align value.

  @param[in]  ulTrackId           Track ID.

  @return     Returns the Block align value.
  @note       None.
============================================================================ */
uint32 MKAVFile::GetBlockAlign(int ulTrackId)
{
  uint32 usBlockAlign = 0;
  if(m_pMKAVParser)
  {
    mkav_audio_info AudioInfo;
    memset(&AudioInfo, 0, sizeof(mkav_audio_info));
    m_pMKAVParser->GetAudioTrackProperties((uint32)ulTrackId, &AudioInfo);
    usBlockAlign = AudioInfo.usBlockAlign;
  }
  return usBlockAlign;
}

/* ============================================================================
  @brief  GetAudioChannelMask.

  @details    This function is used to return Audio channel mask.

  @param[in]  ulTrackId           Track ID.

  @return     Returns the channel mask available.
  @note       None.
============================================================================ */
uint32 MKAVFile::GetAudioChannelMask(int ulTrackId)
{
  uint32 ulChannelMask = 0;
  if(m_pMKAVParser)
  {
    mkav_audio_info AudioInfo;
    memset(&AudioInfo, 0, sizeof(mkav_audio_info));
    m_pMKAVParser->GetAudioTrackProperties((uint32)ulTrackId, &AudioInfo);
    ulChannelMask = (uint32)AudioInfo.ullChannelMask;
  }
  return ulChannelMask;
}

/* ============================================================================
  @brief  GetAudioBitsPerSample.

  @details    This function is used to return Audio samples bit-depth.

  @param[in]  ulTrackId           Track ID.

  @return     Returns the BitDepth available.
  @note       None.
============================================================================ */
uint32 MKAVFile::GetAudioBitsPerSample(int ulTrackId)
{
  uint32 ulBitsPerSample = 0;
  if(m_pMKAVParser)
  {
    mkav_audio_info AudioInfo;
    memset(&AudioInfo, 0, sizeof(mkav_audio_info));
    m_pMKAVParser->GetAudioTrackProperties((uint32)ulTrackId, &AudioInfo);
    ulBitsPerSample = (uint32)AudioInfo.ullBitsPerSample;
  }
  return ulBitsPerSample;
}

/* ============================================================================
  @brief  getTrackAudioSamplingFreq.

  @details    This function is used to return Audio samples per sec value.

  @param[in]  ulTrackId           Track ID.

  @return     Returns the Sampling frequency value.
  @note       None.
============================================================================ */
uint32 MKAVFile::getTrackAudioSamplingFreq(uint32 ulTrackId)
{
  uint32 ulSamplingFreq = 0;
  if(m_pMKAVParser)
  {
    mkav_audio_info AudioInfo;
    memset(&AudioInfo, 0, sizeof(mkav_audio_info));
    m_pMKAVParser->GetAudioTrackProperties((uint32)ulTrackId, &AudioInfo);
    ulSamplingFreq = (uint32)AudioInfo.SamplingFrequency;
  }
  return ulSamplingFreq;
}

/* ============================================================================
  @brief  GetNumAudioChannels.

  @details    This function is used to return number of aud channels.

  @param[in]  ulTrackId           Track ID.

  @return     Returns the Num channels value.
  @note       None.
============================================================================ */
uint32 MKAVFile::GetNumAudioChannels(int ulTrackId)
{
  uint32 ulNumChannels = 0;
  if(m_pMKAVParser)
  {
    mkav_audio_info AudioInfo;
    memset(&AudioInfo, 0, sizeof(mkav_audio_info));
    m_pMKAVParser->GetAudioTrackProperties((uint32)ulTrackId, &AudioInfo);
    ulNumChannels = (uint32)AudioInfo.ullNumChannels;
  }
  return ulNumChannels;
}

/* ======================================================================
FUNCTION:
  MKAVFile::peekCurSample

DESCRIPTION:
  gets information about current sample of a track

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
PARSER_ERRORTYPE MKAVFile::peekCurSample(uint32 ulTrackid,
                                         file_sample_info_type *pSampleInfo)
{
  PARSER_ERRORTYPE reterror = PARSER_ErrorDefault;
  if((!m_pMKAVParser) || (!pSampleInfo))
  {
    reterror = PARSER_ErrorDefault;
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                "MKAVFile::peekCurSample invalid argument");
  }
  else
  {
    bool bError = false;
    uint8 ucIndex = MapTrackIdToIndex(&bError,ulTrackid);
    if(!bError)
    {
      *pSampleInfo = m_sampleInfo[ucIndex];
      reterror = PARSER_ErrorNone;
    }
  }
  return reterror;
}
/* ======================================================================
FUNCTION:
  MKAVFile::getTrackOTIType

DESCRIPTION:
  gets track OTI value

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
uint8 MKAVFile::getTrackOTIType(uint32 id)
{
  uint8 format = 0xFF;
  //mkav_media_codec_type codec_type;

  if(m_pMKAVParser)
  {
    mkav_media_codec_type codec = m_pMKAVParser->GetTrackCodecType(id);
    if((codec == MKAV_MPEG4_ISO_SIMPLE_PROFILE_CODEC)        ||
       (codec == MKAV_MPEG4_ISO_ADVANCE_SIMPLE_PROFILE_CODEC)||
       (codec == MKAV_MPEG4_ISO_ADVANCE_PROFILE_CODEC)       ||
       (codec == MKAV_MPEG4_VIDEO))
    {
      format = MPEG4_VIDEO;
    }
    else if((codec == MKAV_MPEG2_VIDEO_CODEC) ||
            (codec == MKAV_MPEG1_VIDEO_CODEC))
    {
      format = MPEG2_VIDEO;
    }
    else if(codec == MKAV_AVC1_VIDEO_CODEC)
    {
      format = H264_VIDEO;
    }
    else if (codec == MKAV_HEVC_VIDEO_CODEC)
    {
      format = HEVC_VIDEO;
    }
    else if (codec == MKAV_WMV3_VIDEO)
    {
      format = WM_VIDEO_9;
    }
    else if (codec == MKAV_WMV2_VIDEO)
    {
      format = WM_VIDEO_8;
    }
    else if (codec == MKAV_WMV1_VIDEO)
    {
      format = WM_VIDEO_7;
    }
    else if ((codec == MKAV_WVC1_VIDEO) ||
             (codec == MKAV_WMVA_VIDEO))
    {
      format = VC1_VIDEO;
    }
    else if(codec == MKAV_SPARK_VIDEO)
    {
      format = SPARK_VIDEO;
    }
    else if(codec == MKAV_SORENSON_VIDEO)
    {
      //to do no format for sorenson video yet
    }
    else if(codec == MKAV_VP8_VIDEO)
    {
      format = VP8F_VIDEO;
    }
    else if(codec == MKAV_VP9_VIDEO)
    {
      format = VP9_VIDEO;
    }
    else if(codec == MKAV_H263_VIDEO)
    {
      format = H263_VIDEO;
    }
    else if(MKAV_DIV3_VIDEO == codec)
    {
      format = DIVX311_VIDEO;
    }
    else if(MKAV_DIVX_VIDEO == codec)
    {
      format = DIVX40_VIDEO;
    }
    else if(MKAV_DIVX50_VIDEO == codec)
    {
      format = DIVX50_60_VIDEO;
    }
    else if(codec == MKAV_AAC_AUDIO)
    {
      format = MPEG4_AUDIO;
    }
    else if((codec == MKAV_AC3_AUDIO_CODEC)||(codec == MKAV_DOLBY_AC3_CODEC))
    {
      format = AC3_AUDIO;
    }
    else if(MKAV_EAC3_AUDIO_CODEC == codec)
    {
      format = EAC3_AUDIO;
    }
    else if (codec == MKAV_DTS_AUDIO_CODEC)
    {
      format = DTS_AUDIO;
    }
    else if(codec == MKAV_MP3_AUDIO)
    {
      format = MP3_AUDIO;
    }
    else if(codec == MKAV_MP2_AUDIO)
    {
      format = MP2_AUDIO;
    }
    else if(codec == MKAV_MP1_AUDIO)
    {
      format = MP1_AUDIO;
    }
    else if(codec == MKAV_VORBIS_AUDIO_CODEC)
    {
      format = VORBIS_AUDIO;
    }
    else if(codec == MKAV_WMA_AUDIO)
    {
      format = WM_AUDIO;
    }
    else if(codec == MKAV_WMA_PRO_AUDIO)
    {
      format = WM_PRO_AUDIO;
    }
    else if(codec == MKAV_WM_LOSSLESS)
    {
      format = WM_LOSSLESS;
    }
    else if(codec == MKAV_WM_SPEECH)
    {
      format = WM_SPEECH;
    }
    else if (codec == MKAV_OPUS)
    {
      format = OPUS_AUDIO;
    }
    else if(codec == MKAV_USF)
    {
      format = USF_TEXT;
    }
    else if(codec == MKAV_ASS)
    {
      format = ASS_TEXT;
    }
    else if(codec == MKAV_SSA)
    {
      format = SSA_TEXT;
    }
    else if(codec == MKAV_UTF8)
    {
      format = UTF8_TEXT;
    }
    else if(codec == MKAV_VOBSUB)
    {
      format = VOBSUB_TEXT;
    }
    else if(codec == MKAV_KARAOKE)
    {
      format = KARAOKE_TEXT;
    }
    else if(codec == MKAV_BMP)
    {
      format = BITMAP_TEXT;
    }
  }
  return format;
}
/* ======================================================================
FUNCTION:
  MKAVFile::getTrackMaxBufferSizeDB

DESCRIPTION:
  gets maximum buffer size to play the track

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
int32  MKAVFile::getTrackMaxBufferSizeDB(uint32 ulTrackId)
{
  int32 bufsize = 0;
  if(m_pMKAVParser)
  {
    bufsize = m_pMKAVParser->GetTrackBufferSize(ulTrackId);
  }
  return bufsize;
}
/* ======================================================================
FUNCTION:
  MKAVFile::getTrackAverageBitrate

DESCRIPTION:
  gets track's average bitrate

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 average bitrate

SIDE EFFECTS:
  None.
======================================================================*/
int32 MKAVFile::getTrackAverageBitrate(uint32 ulTrackId)
{
  uint32 ulBitrate = 0;
  if(m_pMKAVParser)
  {
    mkav_audio_info AudioInfo;
    memset(&AudioInfo, 0, sizeof(mkav_audio_info));
    m_pMKAVParser->GetAudioTrackProperties((uint32)ulTrackId, &AudioInfo);
    ulBitrate = AudioInfo.ulBitRate;
  }
  return ulBitrate;
}

/* ======================================================================
FUNCTION:
  MKAVFile::getTrackDecoderSpecificInfoContent

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
PARSER_ERRORTYPE MKAVFile::getTrackDecoderSpecificInfoContent(uint32 id, uint8* buf, uint32 *pbufSize)
{
  PARSER_ERRORTYPE errorCode = PARSER_ErrorDefault;
  if( (!m_pMKAVParser) || (!pbufSize) )
  {
    return PARSER_ErrorDefault;
  }
  uint32 size = m_pMKAVParser->GetCodecHeaderSize(id, m_bRawCodecData);
  uint8* header = m_pMKAVParser->GetCodecHeader(id, m_bRawCodecData);
  if(buf && (*pbufSize >= size) && size && header)
  {
    memcpy(buf,header,size);
    *pbufSize = size;
    errorCode = PARSER_ErrorNone;
  }
  else if(size > 0)
  {
    *pbufSize = size;
    errorCode = PARSER_ErrorNone;
  }
 return errorCode;
}
/* ======================================================================
FUNCTION:
  MKAVFile::GetAACAudioProfile

DESCRIPTION:
  Retrieves audio object type/audio profile

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
uint32 MKAVFile::GetAACAudioProfile(uint32 ulTrackId)
{
  uint32 ulProfileData = 0;
  if(m_pMKAVParser)
  {
    if(m_pMKAVParser->GetTrackCodecType(ulTrackId) == MKAV_AAC_AUDIO)
    {
      uint8* phdr = m_pMKAVParser->GetCodecHeader(ulTrackId);
      uint32 ulCodecHdrSize = m_pMKAVParser->GetCodecHeaderSize(ulTrackId);
      if(phdr && (ulCodecHdrSize >= MIN_AAC_CODEC_HDR_SIZE) )
      {
        uint16 usCodecData = (uint16)( (phdr[1]) << 8  |(phdr[0]) );
        ulProfileData = uint32((usCodecData & 0xF800)>>11 );
      }
    }
  }
  return ulProfileData;
}
/* ======================================================================
FUNCTION:
  MKAVFile::GetAACAudioFormat

DESCRIPTION:
  Retrieves audio stream format

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 Stream format value

SIDE EFFECTS:
  None.
======================================================================*/
uint32 MKAVFile::GetAACAudioFormat(uint32 ulTrackId)
{
  uint32 nret = 0;
  if(m_pMKAVParser)
  {
    if(m_pMKAVParser->GetTrackCodecType(ulTrackId) == MKAV_AAC_AUDIO)
    {
      nret = FILE_SOURCE_AAC_FORMAT_RAW;
    }
  }
  return nret;
}
/* ======================================================================
FUNCTION:
  MKAVFile::getTrackDecoderSpecificInfoContent

DESCRIPTION:
  this returns codec Header.

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
uint8* MKAVFile::getTrackDecoderSpecificInfoContent(uint32 id)
{
  uint8* ptr = NULL;
  if(m_pMKAVParser)
  {
    ptr = m_pMKAVParser->GetCodecHeader(id);
  }
  return ptr;
}
/* ======================================================================
FUNCTION:
  MKAVFile::getTrackDecoderSpecificInfoSize

DESCRIPTION:
  this returns codec Header size.

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
uint32 MKAVFile::getTrackDecoderSpecificInfoSize(uint32 id)
{
  uint32 nSize = 0;
  if(m_pMKAVParser)
  {
    nSize = m_pMKAVParser->GetCodecHeaderSize(id);
  }
  return nSize;
}
/* ======================================================================
FUNCTION:
  MKAVFile::MapTrackIdToIndex

DESCRIPTION:
  Maps given track-id to track index.

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
uint8 MKAVFile::MapTrackIdToIndex(bool* bError,uint32 trackid)
{
  uint8 index = 0;
  if(bError)
  {
    *bError = true;
    for(uint32 i = 0; i < m_nNumStreams; i++)
    {
      if( (m_pIndTrackIdTable[i].trackId == trackid) &&
        (m_pIndTrackIdTable[i].bValid) )
      {
        index = m_pIndTrackIdTable[i].index;
        *bError = false;
        break;
      }
    }
    if(*bError)
    {
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_FATAL,
                 "MKAVFile::MapTrackIdToIndex failed for trackid %lu",trackid);
    }
  }
  return index;
}
#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
///*===========================================================================
//
//FUNCTION  updateBufferWritePtr
//
//DESCRIPTION
//  Public method used to update the write buffer offset during Http streaming.
//
//===========================================================================*/
void MKAVFile::updateBufferWritePtr ( uint64 writeOffset )
{
  m_nAvailableOffset = writeOffset;
}
///*===========================================================================
//
//FUNCTION  parseHTTPStream
//
//DESCRIPTION
//  Public method used to parse the Http Stream.
//
//===========================================================================*/
bool MKAVFile::parseHTTPStream ( void )
{
  if(!(m_playAudio || m_playVideo))
  {
    bIsMetaDataParsed = true;
  }
  bool bRet = bIsMetaDataParsed;
  if(m_pMKAVParser && m_bHttpStreaming && (!bIsMetaDataParsed) )
  {
    if(m_pMKAVParser->ParseByteStream() == MKAV_API_SUCCESS)
    {
      bRet = true;
      _success = true;
      _fileErrorCode = PARSER_ErrorNone;
      bIsMetaDataParsed = true;
      m_nNumStreams = m_pMKAVParser->GetTotalNumberOfTracks();
      if(m_nNumStreams)
      {
        uint32* idlist = (uint32*)MM_Malloc(m_nNumStreams * sizeof(uint32) );
        m_pIndTrackIdTable = (MKAVTrackIdToIndexTable*)MM_Malloc(m_nNumStreams * sizeof(MKAVTrackIdToIndexTable));
        if(m_pIndTrackIdTable && idlist)
        {
          memset(m_pIndTrackIdTable,0,m_nNumStreams * sizeof(MKAVTrackIdToIndexTable));
          if(m_pMKAVParser->GetTrackWholeIDList(idlist) == m_nNumStreams)
          {
            for(uint32 i = 0; i < m_nNumStreams; i++)
            {
              m_pIndTrackIdTable[i].index = (uint8)i;
              m_pIndTrackIdTable[i].bValid = true;
              m_pIndTrackIdTable[i].trackId = idlist[i];
            }
          }
        }//if(m_pIndTrackIdTable && idlist)
        if(idlist)
          MM_Free(idlist);
      }//if(m_nNumStreams)
    }//if(m_pMKAVParser->StartParsing() == MKAV_API_SUCCESS)
  }//if(m_pMKAVParser && (!bIsMetaDataParsed) )
  return bRet;
}

#endif//#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD

/* ============================================================================
  @brief  SetAudioOutputMode.

  @details    This function is used to Audio o/p mode.

  @param[in]      henum               O/p mode to set.

  @return  FILE_SOURCE_SUCCESS if successful in setting output mode
           else returns FILE_SOURCE_FAIL.
  @note       None.
============================================================================ */
FileSourceStatus MKAVFile::SetAudioOutputMode(FileSourceConfigItemEnum henum)
{
  FileSourceStatus status = FILE_SOURCE_FAIL;
  if(m_pMKAVParser)
  {
    status = m_pMKAVParser->SetAudioOutputMode(henum);
  }
  return status;
}

/* ============================================================================
  @brief  GetAudioOutputMode.

  @details    This function is used to check Audio o/p mode that is set.

  @param[in]      henum               O/p mode to set.
  @param[in/out]  bret                Bool value to check whether
                                      enum set is same as queried or not.

  @return  FILE_SOURCE_SUCCESS if successful in setting output mode
           else returns FILE_SOURCE_FAIL.
  @note       None.
============================================================================ */
FileSourceStatus MKAVFile::GetAudioOutputMode(bool* bret,
                                              FileSourceConfigItemEnum henum)
{
  FileSourceStatus status = FILE_SOURCE_FAIL;
  if(m_pMKAVParser)
  {
    status = m_pMKAVParser->GetAudioOutputMode(bret,henum);
  }
  return status;
}

/* ======================================================================
FUNCTION:
  GetClipMetaData

DESCRIPTION:
  Called by user to extract metadata

INPUT/OUTPUT PARAMETERS:
  pucDataBuf            Data Buffer to store Metadata
  pulDatabufLen         Size of the data buffer
  ienumData             I Enum Data

RETURN VALUE:
 FILE_SOURCE_SUCCESS if successful in retrieving output mode
 else returns FILE_SOURCE_FAIL

SIDE EFFECTS:
  None.
======================================================================*/
PARSER_ERRORTYPE MKAVFile::GetClipMetaData(wchar_t *pucDataBuf,
                                           uint32 *pulDatabufLen,
                                           FileSourceMetaDataType ienumData)
{
  METADATAINDEX ucMetaDataIndex = MAX_INDEX, ucMetaDataIndex1 = MAX_INDEX;
  PARSER_ERRORTYPE eRetStatus = PARSER_ErrorNone;
  if(pulDatabufLen == NULL || m_pMKAVParser == NULL)
  {
    return PARSER_ErrorInvalidParam;
  }
  tag_info_type *pTagInfo = NULL;

  switch (ienumData)
  {
  case FILE_SOURCE_MD_TITLE:
    ucMetaDataIndex     = TAG_TITLE;
    break;
  case FILE_SOURCE_MD_ARTIST:
    ucMetaDataIndex     = TAG_ARTIST;
    ucMetaDataIndex1    = TAG_ARTIST1;
    break;
    case FILE_SOURCE_MD_ALBUM:
      ucMetaDataIndex   = TAG_ALBUM;
      break;
    case FILE_SOURCE_MD_COMPOSER:
      ucMetaDataIndex   = TAG_COMPOSER;
      break;
    case FILE_SOURCE_MD_GENRE:
      ucMetaDataIndex   = TAG_GENRE;
      break;
    case FILE_SOURCE_MD_AUTHOR:
      ucMetaDataIndex   = TAG_AUTHOR;
      break;
    case FILE_SOURCE_MD_REC_YEAR:
      ucMetaDataIndex   = TAG_REC_DATE;
      ucMetaDataIndex1  = TAG_REL_DATE;
    break;
    default:
      ucMetaDataIndex   = MAX_INDEX;
      ucMetaDataIndex1  = MAX_INDEX;
      break;
  }

  if (MAX_INDEX != ucMetaDataIndex )
  {
    pTagInfo = m_pMKAVParser->GetClipMetaData((uint32)ucMetaDataIndex);
  }

  if (NULL == pTagInfo && MAX_INDEX != ucMetaDataIndex1)
  {
    pTagInfo = m_pMKAVParser->GetClipMetaData((uint32)ucMetaDataIndex1);
  }

  if(pTagInfo)
  {
    if (pucDataBuf)
    {
      if((pTagInfo->pTagString && pTagInfo->ulTagStringLen > *pulDatabufLen) ||
         (sizeof(pTagInfo->ulTagBinValue) > *pulDatabufLen))
      {
        return PARSER_ErrorInsufficientBufSize;
      }
      uint8 ucTemp[16];
      uint8* pucStr   = pTagInfo->pTagString;
      uint32 ulStrLen = pTagInfo->ulTagStringLen;

      if(NULL == pTagInfo->pTagString)
      {
        memset(ucTemp, 0, sizeof(ucTemp));
#ifdef _ANDROID_
        snprintf((char*)ucTemp, 4, (char*)"%d", (int)pTagInfo->ulTagBinValue);
#else
        std_strlprintf((char*)ucTemp, 4, (char*)"%d", (int)pTagInfo->ulTagBinValue);
#endif
        pucStr = ucTemp;
        ulStrLen = 4;
      }

      //Flag at the end indicate whether input string is in UTF8 or UTF16
      //format. 'FALSE' means UTF8, 'TRUE' means UTF16
      CharToWideChar((const char*)pucStr, ulStrLen,
                     pucDataBuf, *pulDatabufLen, FALSE);
    }
    /* In MKV, text encoding is in UTF-8 format only. */
    if(pTagInfo->pTagString)
    {
      *pulDatabufLen = pTagInfo->ulTagStringLen * (int)sizeof(wchar_t);
    }
    else
    {
      *pulDatabufLen = (int)sizeof(pTagInfo->ulTagBinValue) * (int)sizeof(wchar_t);
    }
  }//if(pTagInfo)
  else if ((FILE_SOURCE_MD_ENC_DELAY == ienumData) ||
           (FILE_SOURCE_MD_SEEK_PREROLL_DELAY == ienumData))
  {
    uint32 ulTrackId;
    uint64 nDelay = 0;
    mkav_audio_info AudioInfo;
    bool bValid = false;
    char ucTempString[33];
    memset(ucTempString, 0, 33);
    memset(&AudioInfo, 0, sizeof(mkav_audio_info));

    for(uint8 i = 0; i < m_nNumStreams; i++)
    {
      ulTrackId = m_pIndTrackIdTable[i].trackId;
      bValid = m_pMKAVParser->GetAudioTrackProperties(ulTrackId, &AudioInfo);
      if (bValid)
      {
        break;
      }
    }
    if (FILE_SOURCE_MD_ENC_DELAY == ienumData)
    {
      nDelay = AudioInfo.ullEncoderDelay;
    }
    else
    {
      nDelay = AudioInfo.ullSeekPrerollDelay;
    }
#ifdef _ANDROID_
    snprintf(ucTempString, 16, (char*)"%x", (int)nDelay);
#else
    std_strlprintf(ucTempString, 16, (char*)"%x", (int)nDelay);
#endif
    if (pucDataBuf)
    {
      memcpy(pucDataBuf, ucTempString, sizeof(uint64));
    }
    *pulDatabufLen = (uint32)sizeof(uint64);
  }
  else
  {
    /* Provide some metadata fields from Segment Info data if available. */
    eRetStatus = m_pMKAVParser->GetSegmentInfo(pucDataBuf, pulDatabufLen,
                                               ienumData);
    if(PARSER_ErrorNone != eRetStatus)
    {
      *pulDatabufLen = 0;
    }
  }

  return eRetStatus;
}

/* ======================================================================
FUNCTION:
  isWebm

DESCRIPTION:
  Function to check whether i/p is WebM Complaint or not.

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 True if clip is WebM complaint or false.

SIDE EFFECTS:
  None.
======================================================================*/
bool MKAVFile::isWebm()
{
  bool bStatus = false;
  if( ( NULL != m_pMKAVParser ) &&
      ( true == m_pMKAVParser->bIsWebm() ) )
  {
     bStatus = true;
  }
  return bStatus;
}

/* ============================================================================
  @brief  getBufferedDuration.

  @details    This function is used to calculate the playback time based on the
              given Offset value.

  @param[in]      ulTrackId           Track Id.
  @param[in]      sllAvailBytes       Available offset.
  @param[in/out]  pullBufferedTime    Playback Time.

  @return  "TRUE" if successful in calculating the approximate playback time
           else returns "FALSE".
  @note       None.
=============================================================================*/
bool MKAVFile::getBufferedDuration(uint32  ulTrackId,
                                   int64   ullAvailBytes,
                                   uint64* pullBufferedTime)
{
  bool bStatus = false;
  if (m_pMKAVParser)
  {
   bStatus = m_pMKAVParser->getBufferedDuration(ulTrackId, ullAvailBytes,
                                                pullBufferedTime);
  }
  return bStatus;
}

/* ============================================================================
  @brief  GetOffsetForTime.

  @details    This function is used to calculate the approximate offset value
              based on the given Playback timestamp value.

  @param[in]      ullPBTime           Given Playback Time.
  @param[in/out]  pullFileOffset      Parameter to store o/p Offset Value.
  @param[in]      ulTrackId           Track Id.
  @param[in]      ullCurPosTimeStamp  Current Playback Time.
  @param[in]      rullReposTime       Reposition Time.

  @return  "TRUE" if successful in calculating the approximate offset value
           else returns "FALSE".
  @note       None.
=============================================================================*/
bool MKAVFile::GetOffsetForTime(uint64  ullPBTime,
                                uint64* pullFileOffset,
                                uint32  ulTrackId,
                                uint64  ullCurPosTimeStamp,
                                uint64& rullReposTime)
{
  bool bStatus = false;
  if (m_pMKAVParser)
  {
   bStatus = m_pMKAVParser->GetOffsetForTime(ullPBTime, pullFileOffset,
                                             ulTrackId, ullCurPosTimeStamp,
                                             rullReposTime);
  }
  return bStatus;
}
#endif //#ifdef FEATURE_FILESOURCE_MKV_PARSER

