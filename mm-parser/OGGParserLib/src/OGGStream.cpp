/* =======================================================================
                              OGGStream.cpp
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

Copyright 2009-2014 QUALCOMM Technologies Incorporated, All Rights Reserved.
QUALCOMM Technologies Proprietary and Confidential.
========================================================================== */

/* =======================================================================
                             PERFORCE HEADER
$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/OGGParserLib/main/latest/src/OGGStream.cpp#33 $
========================================================================== */

/* ==========================================================================

                     INCLUDE FILES FOR MODULE
========================================================================== */
#include "OGGStreamParser.h"
#include "OGGStream.h"
#include "MMMalloc.h"
#include "atomdefs.h"
#include "utf8conv.h"
#ifdef FEATURE_FILESOURCE_OGG_PARSER
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
  OGGStreamCallbakGetData

DESCRIPTION
  Its a callback function from OGG Parser to read the data.
  This is not implemented by the parser.
  It should be implemented by the app that calls into the parser.

ARGUMENTS
  nOffset                 Offset of the requested data (from beginning),
  nNumBytesRequest        Size of the requested data (in bytes).
  ppData                  Pointer to the buffer for filling in the OGG data
  u32UserData             Extra info From App. Given by user in aviin_open

DEPENDENCIES

RETURN VALUE
  The number of bytes actually being filled in

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
uint32 OGGStreamCallbakGetData (uint64 nOffset, uint32 nNumBytesRequest,
                                 unsigned char* pData, uint32  nMaxSize,
                                 void*  pClientData )
{
  uint32 ulDataRead = 0;
  if(pClientData)
  {
    OGGStream* pOGGStream = (OGGStream*)pClientData;
    ulDataRead = pOGGStream->FileGetData(nOffset, nNumBytesRequest, nMaxSize,
                                         pData);
  }
  return ulDataRead;
}
/* ======================================================================
FUNCTION:
  OGGStream::FileGetData

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
uint32 OGGStream::FileGetData(uint64 nOffset, uint32 nNumBytesRequest,
                               uint32 nMaxSize, uint8* pData  )
{
  uint32 nRead = 0;
  if( (m_pFilePtr != NULL)&& (!m_bMediaAbort) )
  {
    nRead = readFile(m_pFilePtr, pData, nOffset,
                     FILESOURCE_MIN(nNumBytesRequest,nMaxSize));
  }
  return nRead;
}
/* ======================================================================
FUNCTION:
  OGGStream::OGGStream

DESCRIPTION:
  OGGStream constructor

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
  None.

SIDE EFFECTS:
  None.
======================================================================*/
OGGStream::OGGStream(const FILESOURCE_STRING filename
                     ,unsigned char* /*pFileBuf*/
                     ,uint32 /*bufSize*/
                     ,bool bPlayVideo
                     ,bool bPlayAudio
                    )
{
  InitData();
  m_playAudio = bPlayAudio;
  m_playVideo = bPlayVideo;
  m_pFilePtr = OSCL_FileOpen(filename, (OSCL_TCHAR *) _T("rb"));
  m_fileSize = OSCL_FileSize(filename);
  m_pOGGStreamParser = MM_New_Args(OGGStreamParser,(this, m_fileSize,
                                                    bPlayAudio));
  (void)ParseMetaData();
}
#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
OGGStream::OGGStream(video::iStreamPort* pport , bool bPlayVideo, bool bPlayAudio)
{
  InitData();
  m_fileSize    = MAX_FILE_SIZE;
  m_pPort       = pport;
  m_playAudio   = bPlayAudio;
  m_playVideo   = bPlayVideo;
  m_pFilePtr    = OSCL_FileOpen(pport);
  int64 noffset = 0;
  if(m_pPort)
  {
    if(video::iStreamPort::DS_SUCCESS == m_pPort->GetContentLength(&noffset) &&
       (noffset))
    {
      m_fileSize = (uint64)noffset;
    }
  }
  m_pOGGStreamParser = MM_New_Args(OGGStreamParser,(this, m_fileSize,
                                                    bPlayAudio));
  (void)ParseMetaData();
}
#endif
/* ======================================================================
FUNCTION:
  OGGStream::InitData

DESCRIPTION:
  Initializes all the data members to default

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
  None.

SIDE EFFECTS:
  None.
======================================================================*/
void OGGStream::InitData()
{
  m_playAudio       = false;
  m_playVideo       = false;
  m_bStreaming      = false;
  m_nNumStreams     = 0;

  memset(&m_sampleInfo,0,(FILE_MAX_MEDIA_STREAMS * sizeof(file_sample_info_type)) );

  m_fileSize = 0;
  m_pFilePtr = NULL;
  m_pOGGStreamParser = NULL;
  m_pIndTrackIdTable = NULL;
  _success = false;
  _fileErrorCode = PARSER_ErrorDefault;
  m_bMediaAbort = false;
#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
  m_pPort = NULL;
#endif
}
void OGGStream::SetCriticalSection(MM_HANDLE pcriticalsection)
{
  if(m_pFilePtr)
  {
    m_pFilePtr->pCriticalSection = pcriticalsection;
  }
}
/* ======================================================================
FUNCTION:
  OGGStream::~OGGStream

DESCRIPTION:
  OGGStream constructor

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
  None.

SIDE EFFECTS:
  None.
======================================================================*/
OGGStream::~OGGStream()
{
  if(m_pFilePtr)
  {
      OSCL_FileClose(m_pFilePtr);
  }
  if(m_pOGGStreamParser)
  {
    MM_Delete(m_pOGGStreamParser);
  }
  if(m_pIndTrackIdTable)
  {
    MM_Free(m_pIndTrackIdTable);
  }
}
/* ======================================================================
FUNCTION:
  OGGStream::ParseMetaData

DESCRIPTION:
  Starts parsing the MPEG2 transport stream.

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
  None.

SIDE EFFECTS:
  None.
======================================================================*/
bool OGGStream::ParseMetaData()
{
  bool bRet = false;
  if(m_pOGGStreamParser
#ifdef FEATURE_OGG_AUDIO_ONLY
     && m_playAudio
#endif
     )
  {
    if(m_pOGGStreamParser->StartParsing() == OGGSTREAM_SUCCESS)
    {
      bRet = true;
      _success = true;
      _fileErrorCode = PARSER_ErrorNone;
      m_nNumStreams = m_pOGGStreamParser->GetTotalNumberOfTracks();
      if(m_nNumStreams)
      {
        uint32* idlist = (uint32*)MM_Malloc(m_nNumStreams * sizeof(uint32) );
        m_pIndTrackIdTable = (OggTrackIdToIndexTable*)MM_Malloc(m_nNumStreams * sizeof(OggTrackIdToIndexTable));
        if(m_pIndTrackIdTable && idlist)
        {
          memset(m_pIndTrackIdTable,0,m_nNumStreams * sizeof(OggTrackIdToIndexTable));
          if(m_pOGGStreamParser->GetTrackWholeIDList(idlist) == m_nNumStreams)
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
        {
          MM_Free(idlist);
        }
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
PARSER_ERRORTYPE OGGStream::getNextMediaSample(uint32 ulTrackID,
                                               uint8 *pucDataBuf,
                                               uint32 *pulBufSize,
                                               uint32& /*rulIndex*/)
{
  int32 nBytes = 0;
  PARSER_ERRORTYPE retError = PARSER_ErrorDefault;
  OGGStreamStatus eStatus = OGGSTREAM_FAIL;

  /* Validate input params and class variables */
  if(NULL == m_pOGGStreamParser || NULL == pulBufSize || NULL == pucDataBuf ||
     0 == *pulBufSize)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
               "getNextMediaSample invalid input params!!");
    return PARSER_ErrorInvalidParam;
  }
  uint32 ulSerialNum = m_pOGGStreamParser->GetTrackSerialNo(ulTrackID);

#ifdef OGG_PARSER_DEBUG
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"OGGStream::getNextMediaSample");
#endif
  nBytes = (int32)*pulBufSize;
  eStatus = m_pOGGStreamParser->GetCurrentSample(ulSerialNum, pucDataBuf,
                                                 *pulBufSize, &nBytes);
  if(OGGSTREAM_SUCCESS == eStatus)
    retError = PARSER_ErrorNone;
  /* Apart from success, all other errors will be treated as EOF only.
     If streaming support is added, then we need to change this. */
  else
    retError = PARSER_ErrorEndOfFile;
  if(OGGSTREAM_READ_ERROR == eStatus )
  {
      //In case of read error  in local playback  we assume that the end of stream
      //has arrived  or data is no longer received. Here we return 0 bytes and
      //this will be intepreted as media_end in upper layers.
  }
  if(nBytes > 0 && OGGSTREAM_SUCCESS == eStatus)
  {
    bool  bError     = false;
    bool  bTimeValid = false;
    uint8 index      = MapTrackIdToIndex(&bError, ulTrackID);
    if(!bError)
    {
      m_sampleInfo[index].num_frames = 1;
      m_sampleInfo[index].nGranule   = 0;
      eStatus = m_pOGGStreamParser->GetCurrentSampleTimeStamp(ulSerialNum,
                                    &(m_sampleInfo[index].time),
                                    &(m_sampleInfo[index].nGranule),
                                    &bTimeValid);
      if(eStatus == OGGSTREAM_SUCCESS && bTimeValid)
      {
        m_sampleInfo[index].btimevalid = true;
      }
      else
      {
        m_sampleInfo[index].btimevalid = false;
      }
      m_sampleInfo[index].sample++;
      m_sampleInfo[index].size = nBytes;
      m_sampleInfo[index].sync = 1;
    }
  }
#ifdef OGG_PARSER_DEBUG
  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
    "getNextMediaSample returing nBytesRead %lu",nBytes);
#endif
  *pulBufSize = nBytes;
  return retError;
}
/* ======================================================================
FUNCTION:
  OGGStream::getMediaTimestampForCurrentSample

DESCRIPTION:
  gets time stamp of current sample of the track.

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 time stamp in track time scale unit

SIDE EFFECTS:
  None.
======================================================================*/
uint64 OGGStream::getMediaTimestampForCurrentSample(uint32 id)
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
//Need to be implemented when we add support for VIDEO as skipping
//few frames for audio does not really make sense as each frame is only few
//milliseconds
/* ======================================================================
FUNCTION:
  OGGStream::resetPlayback

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
uint64 OGGStream::resetPlayback(uint64 repos_time,
                                uint32 id,
                                bool /*bSetToSyncSample*/,
                                bool *bError,
                                uint64 currentPosTimeStamp)
{
  uint64 newTS = 0;

  bool bforward = (repos_time > currentPosTimeStamp)?1:0;
  ogg_stream_sample_info ogg_sampleInfo;
  memset(&ogg_sampleInfo,0,sizeof(ogg_stream_sample_info));

  MM_MSG_PRIO3(MM_FILE_OPS, MM_PRIO_HIGH,
               "resetPlayback id %lu repos_time %llu currentPosTimeStamp %llu",
               id, repos_time, currentPosTimeStamp);

  if(bError && m_pOGGStreamParser)
  {
    uint32 ulSerialNum = m_pOGGStreamParser->GetTrackSerialNo(id);
    *bError = true;
    if(OGGSTREAM_SUCCESS == m_pOGGStreamParser->Seek
       (ulSerialNum,repos_time,currentPosTimeStamp,&ogg_sampleInfo,bforward,false,0)
      )
    {
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,"Seek Succeed, new TS %llu",ogg_sampleInfo.ntime);
      *bError = false;
      _fileErrorCode = PARSER_ErrorNone;
      bool bMapError = false;
      uint8 index = MapTrackIdToIndex(&bMapError,id);
      if(!bMapError)
      {
        m_sampleInfo[index].num_frames = 1;
        m_sampleInfo[index].time = ogg_sampleInfo.ntime;
        m_sampleInfo[index].sample =  ogg_sampleInfo.nsample;
        m_sampleInfo[index].size = ogg_sampleInfo.nsize;
        m_sampleInfo[index].sync = 1;
        newTS = m_sampleInfo[index].time;
      }
      else
      {
        MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_FATAL,
                     "!!!MapTrackIdToIndex failed for trackid %lu!!!",id);
      }
    }
    else
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"Seek Failed");
     _fileErrorCode = PARSER_ErrorSeekFail;
    }
  }
  return newTS;
}

/* ======================================================================
FUNCTION:
  GetClipMetaData

DESCRIPTION:
  Provides different metadata fields info in the o/p buffer

INPUT/OUTPUT PARAMETERS:
  pucDataBuf   : Buffer to store metadata string in Wchar format.
  pulDatabufLen: Buffer Size.
  ienumData    : Enum to indicate which metadata field requested.

RETURN VALUE:
 PARSER_ErrorNone if Success
 Error status, if failure

SIDE EFFECTS:
  None.
======================================================================*/
PARSER_ERRORTYPE OGGStream::GetClipMetaData(wchar_t *pucDataBuf,
                                            uint32 *pulDatabufLen,
                                            FileSourceMetaDataType ienumData)
{
  int nMetaIndex = MAX_FIELDNAMES_SUPPORTED;
  if((NULL == pulDatabufLen) || (NULL == m_pOGGStreamParser))
    return PARSER_ErrorInvalidParam;

  uint32 ulMetaDataLen = 0;

  m_eEncodeType = FS_ENCODING_TYPE_UNKNOWN;
  switch (ienumData)
  {
    case FILE_SOURCE_MD_TITLE:
      nMetaIndex = TAG_OGG_TITLE;
      break;
    case FILE_SOURCE_MD_VERSION:
      nMetaIndex = TAG_OGG_VERSION;
      break;
    case FILE_SOURCE_MD_ALBUM:
      nMetaIndex = TAG_OGG_ALBUM;
      break;
    case FILE_SOURCE_MD_TRACK_NUM:
      nMetaIndex = TAG_OGG_TRACKNUMBER;
      break;
    case FILE_SOURCE_MD_ARTIST:
      nMetaIndex = TAG_OGG_ARTIST;
      break;
    case FILE_SOURCE_MD_PERFORMANCE:
      nMetaIndex = TAG_OGG_PERFORMER;
      break;
    case FILE_SOURCE_MD_COPYRIGHT:
      nMetaIndex = TAG_OGG_COPYRIGHT;
      break;
    case FILE_SOURCE_MD_DESCRIPTION:
      nMetaIndex = TAG_OGG_DESCRIPTION;
      break;
    case FILE_SOURCE_MD_GENRE:
      nMetaIndex = TAG_OGG_GENRE;
      break;
    case FILE_SOURCE_MD_CREATION_DATE:
      nMetaIndex = TAG_OGG_DATE;
      break;
    case FILE_SOURCE_MD_COMPOSER:
      nMetaIndex = TAG_OGG_COMPOSER;
    break;
    case FILE_SOURCE_MD_ALBUM_ARTIST:
      nMetaIndex = TAG_OGG_ALBUMARTIST;
      break;
    case FILE_SOURCE_MD_ANDROID_LOOP:
      nMetaIndex = TAG_OGG_LOOP;
      break;
    default:
      nMetaIndex = MAX_FIELDNAMES_SUPPORTED;
      break;
  }

  if (MAX_FIELDNAMES_SUPPORTED != nMetaIndex)
  {
    m_pOGGStreamParser->GetClipMetaData(nMetaIndex, NULL, &ulMetaDataLen);
    ulMetaDataLen++;
    if ((pucDataBuf) && (*pulDatabufLen >= ulMetaDataLen))
    {
      //! Initialize memory with ZERO
      memset(pucDataBuf, 0, *pulDatabufLen);
      m_pOGGStreamParser->GetClipMetaData(nMetaIndex, (uint8*)pucDataBuf,
                                          &ulMetaDataLen);
      m_eEncodeType = FS_TEXT_ENC_UTF8;
    }
    else if (pucDataBuf)
    {
      return PARSER_ErrorInsufficientBufSize;
    }
    /* It means there is no metadata field for this enum type. Two characters
       are to store NULL characters at the end. */
    if (2 == ulMetaDataLen)
    {
      ulMetaDataLen = 0;
    }
  }
  *pulDatabufLen = ulMetaDataLen * (uint32)sizeof(wchar_t);

  return PARSER_ErrorNone;
}

/* ======================================================================
FUNCTION:
  OGGStream::getMovieDurationMsec

DESCRIPTION:
  gets movie duration in milli secs

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
uint64 OGGStream::getMovieDurationMsec() const
{
  uint64 nDuration = 0;
  if(!m_pOGGStreamParser)
  {
    return nDuration;
  }
  nDuration = m_pOGGStreamParser->GetClipDurationInMsec();
  return nDuration;
}
bool OGGStream::GetFlacCodecData(int id,flac_format_data* pData)
{
  bool bRet = false;
#ifdef FEATURE_FILESOURCE_FLAC_PARSER
  if(pData)
  {
    bRet = true;
    (void)MapTrackIdToIndex(&bRet,id);
    if((!bRet) && (m_pOGGStreamParser) )//false means track id is valid
    {
      flac_metadata_streaminfo pstreaminfo;
      if(OGGSTREAM_SUCCESS == m_pOGGStreamParser->GetFlacStreamInfo(id,&pstreaminfo))
      {
        pData->nBitsPerSample = pstreaminfo.nBitsPerSample;
        pData->nChannels= pstreaminfo.nChannels;
        pData->nFixedBlockSize= pstreaminfo.nFixedBlockSize;
        pData->nMaxBlockSize= pstreaminfo.nMaxBlockSize;
        pData->nMinBlockSize= pstreaminfo.nMinBlockSize;
        pData->nMinFrameSize= pstreaminfo.nMinFrameSize;
        pData->nSamplingRate= pstreaminfo.nSamplingRate;
        pData->nTotalSamplesInStream= pstreaminfo.nTotalSamplesInStream;
        bRet = true;
      }
    }
  }
#endif
  return bRet;
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
uint32 OGGStream::getTrackWholeIDList(uint32 *ids)
{
  if(!m_pOGGStreamParser)
  {
    return 0;
  }
  return (m_pOGGStreamParser->GetTrackWholeIDList(ids));
}

/* ======================================================================
FUNCTION:
  OGGStream::getTrackVideoFrameRate

DESCRIPTION:
  gets track video (if video) frame rate

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
float OGGStream::getTrackVideoFrameRate(uint32 id)
{
  float frate = 0.0;
  if(m_pOGGStreamParser)
  {
    frate = m_pOGGStreamParser->GetVideoFrameRate(id);
  }
  return frate;
}
/* ======================================================================
FUNCTION:
  OGGStream::getTrackVideoFrameWidth

DESCRIPTION:
  returns video track's frame width.

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
uint32 OGGStream::getTrackVideoFrameWidth(uint32 id)
{
  uint32 width = 0;
  if(m_pOGGStreamParser)
  {
    width = m_pOGGStreamParser->GetVideoWidth(id);
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
uint32 OGGStream::getTrackVideoFrameHeight(uint32 id)
{
  uint32 height = 0;
  if(m_pOGGStreamParser)
  {
    height = m_pOGGStreamParser->GetVideoHeight(id);
  }
  return height;
}

/* ======================================================================
FUNCTION:
  OGGStream::getTrackAudioSamplingFreq

DESCRIPTION:
  gets audio track's sampling frequency

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
uint32 OGGStream::getTrackAudioSamplingFreq(uint32 ulTrackId)
{
  uint32 ulSamplingFreq = 0;
  if(m_pOGGStreamParser)
  {
    ogg_audio_info sAudioInfo;
    bool bStatus =
      m_pOGGStreamParser->GetAudioStreamInfo(ulTrackId, &sAudioInfo);
    if(bStatus)
    ulSamplingFreq = sAudioInfo.SamplingFrequency;
  }
  return ulSamplingFreq;
}

/* ============================================================================
  @brief  Returns number of bits used for each audio sample

  @details    This function is used to return no. of bits used for each
              audio sample.

  @param[in]      nTrackId      Track Id number.

  @return     Sample bit width is returned.
  @note       BufferSize should be more than maximum frame size value.
============================================================================ */
uint32 OGGStream::GetAudioBitsPerSample(int nTrackId)
{
  uint32 ulBitWidth = 0;
  if(m_pOGGStreamParser)
  {
    ogg_audio_info sInfo;
    memset(&sInfo, 0, sizeof(ogg_audio_info));
    (void)m_pOGGStreamParser->GetAudioStreamInfo(nTrackId, &sInfo);
    ulBitWidth = sInfo.nBitsPerSample;
  }
  return ulBitWidth;
}


/* ======================================================================
FUNCTION:
  OGGStream::GetNumAudioChannels

DESCRIPTION:
  returns number of audio channels

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
uint32 OGGStream::GetNumAudioChannels(int ulTrackId)
{
  uint32 ulNumChannels = 0;
  if(m_pOGGStreamParser)
  {
    ogg_audio_info sAudioInfo;
    bool bStatus =
      m_pOGGStreamParser->GetAudioStreamInfo(ulTrackId, &sAudioInfo);
    if(bStatus)
      ulNumChannels = sAudioInfo.NumberOfChannels;
  }
  return ulNumChannels;
}
/* ======================================================================
FUNCTION:
  OGGStream::peekCurSample

DESCRIPTION:
  gets information about current sample of a track

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
PARSER_ERRORTYPE OGGStream::peekCurSample(uint32 trackid, file_sample_info_type *pSampleInfo)
{
  PARSER_ERRORTYPE reterror = PARSER_ErrorDefault;
  if((!m_pOGGStreamParser) || (!pSampleInfo))
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,"OGGStream::peekCurSample invalid argument");
  }
  else
  {
    bool bError = false;
    uint8 index = MapTrackIdToIndex(&bError,trackid);
    if(!bError)
    {
      *pSampleInfo = m_sampleInfo[index];
      m_sampleInfo[index].nGranule = 0;
      reterror = PARSER_ErrorNone;
    }
  }
  return reterror;
}
/* ======================================================================
FUNCTION:
  OGGStream::getTrackOTIType

DESCRIPTION:
  gets track OTI value

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
uint8 OGGStream::getTrackOTIType(uint32 id)
{
  uint8 format = 0xFF;
  ogg_media_codec_type codec_type;

  if(m_pOGGStreamParser)
  {
    //Convert trackID to SerialNum
    id = m_pOGGStreamParser->GetTrackSerialNo(id);
    codec_type = m_pOGGStreamParser->GetTrackType(id);
    switch(codec_type)
    {
      case OGG_AUDIO_CODEC_VORBIS:
      {
        format = (uint8)VORBIS_AUDIO;
      }
      break;

      //! FLAC in OGG container is not PORed, so disable.
#ifndef _ANDROID_
      case OGG_AUDIO_CODEC_FLAC:
      {
        format = (uint8)FLAC_AUDIO;
      }
      break;
#endif
#ifdef FEATURE_FILESOURCE_OGG_THEORA_CODEC
      case OGG_VIDEO_CODEC_THEORA:
      {
        format = (uint8)THEORA_VIDEO;
      }
      break;
#endif
      default:
      break;
    }
  }
  return format;
}
/* ======================================================================
FUNCTION:
  OGGStream::getTrackMaxBufferSizeDB

DESCRIPTION:
  gets maximum buffer size to play the track

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
int32  OGGStream::getTrackMaxBufferSizeDB(uint32 id)
{
  int32 bufsize = MAX_PAGE_SIZE;
  //ogg_media_codec_type codec_type;
  if(!m_pOGGStreamParser)
  {
    return 0;
  }
  else
  {
      bufsize = m_pOGGStreamParser->GetTrackMaxBufferSize(id);
  }
  return bufsize;
}
/* ======================================================================
FUNCTION:
  OGGStream::getTrackAverageBitrate

DESCRIPTION:
  gets track's average bitrate

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 average bitrate

SIDE EFFECTS:
  None.
======================================================================*/
int32 OGGStream::getTrackAverageBitrate(uint32 ulTrackId)
{
  uint32 bitrate = 0;
  if(m_pOGGStreamParser)
  {
    ogg_audio_info sAudioInfo;
    bool bStatus =
      m_pOGGStreamParser->GetAudioStreamInfo(ulTrackId, &sAudioInfo);
    if(bStatus)
      bitrate = sAudioInfo.NominalBitRate;
    else
      bitrate = m_pOGGStreamParser->GetTrackAverageBitRate(ulTrackId);
  }
  return bitrate;
}

/* ======================================================================
FUNCTION:
  OGGStream::getTrackMinBitrate

DESCRIPTION:
  gets track's average bitrate

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 average bitrate

SIDE EFFECTS:
  None.
======================================================================*/
int32 OGGStream::getTrackMinBitrate(uint32 ulTrackId)
{
  int32 slBitRate = 0;
  if(m_pOGGStreamParser)
  {
    ogg_audio_info sAudioInfo;
    bool bStatus =
      m_pOGGStreamParser->GetAudioStreamInfo(ulTrackId, &sAudioInfo);
    if(bStatus)
      slBitRate = sAudioInfo.MinimumBitRate;
  }
  return slBitRate;
}

/* ======================================================================
FUNCTION:
  OGGStream::getTrackMaxBitrate

DESCRIPTION:
  gets track's average bitrate

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 average bitrate

SIDE EFFECTS:
  None.
======================================================================*/
int32 OGGStream::getTrackMaxBitrate(uint32 ulTrackId)
{
  int32 slBitRate = 0;
  if(m_pOGGStreamParser)
  {
    ogg_audio_info sAudioInfo;
    bool bStatus =
      m_pOGGStreamParser->GetAudioStreamInfo(ulTrackId, &sAudioInfo);
    if(bStatus)
      slBitRate = sAudioInfo.MaximumBitRate;
  }
  return slBitRate;
}

/* ======================================================================
FUNCTION:
  OGGStream::getTrackDecoderSpecificInfoContent

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
PARSER_ERRORTYPE OGGStream::getTrackDecoderSpecificInfoContent(uint32 id, uint8* buf, uint32 *pbufSize)
{
  PARSER_ERRORTYPE errorCode = PARSER_ErrorDefault;
  if( (!m_pOGGStreamParser) || (!pbufSize) )
  {
    return errorCode;
  }
  uint32 size = m_pOGGStreamParser->GetCodecHeaderSize(id);
  uint8* header = m_pOGGStreamParser->GetCodecHeader(id);
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
  OGGStream::MapTrackIdToIndex

DESCRIPTION:
  Maps given track-id to track index.

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
uint8 OGGStream::MapTrackIdToIndex(bool* bError,uint32 trackid)
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
  }
  if(bError && *bError)
  {
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_FATAL,
                "OGGStream::MapTrackIdToIndex failed for trackid %lu",trackid);
  }
  return index;
}
#endif //FEATURE_FILESOURCE_OGG_PARSER

