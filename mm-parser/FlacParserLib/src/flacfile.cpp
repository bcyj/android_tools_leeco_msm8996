/* =======================================================================
                              flacfile.cpp
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

  Copyright(c) 2009-2015 by Qualcomm Technologies, Inc. All Rights Reserved
  Qualcomm Technologies Proprietary and Confidential.
========================================================================== */

/* =======================================================================
                             PERFORCE HEADER
$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/FlacParserLib/main/latest/src/flacfile.cpp#30 $
========================================================================== */

/* ==========================================================================

                     INCLUDE FILES FOR MODULE
========================================================================== */
#include "FlacParser.h"
#include "flacfile.h"
#include "MMMalloc.h"
#ifdef FEATURE_FILESOURCE_FLAC_PARSER
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
uint32 FlacFileCallbakGetData (uint64 nOffset, uint32 nNumBytesRequest,
                               unsigned char* pData, uint32  nMaxSize,
                               void*  pClientData )
{
  if(pClientData)
  {
    flacfile* pFlacFile = (flacfile*)pClientData;
    return pFlacFile->FileGetData(nOffset, nNumBytesRequest, nMaxSize, pData);
  }
  return 0;
}
/* ======================================================================
FUNCTION:
  flacfile::FileGetData

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
uint32 flacfile::FileGetData(uint64 nOffset, uint32 nNumBytesRequest,
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
  flacfile::flacfile

DESCRIPTION:
  OGGStream constructor

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
  None.

SIDE EFFECTS:
  None.
======================================================================*/
flacfile::flacfile(const FILESOURCE_STRING filename
                   ,unsigned char* /*pFileBuf*/
                   ,uint64 /*bufSize*/)
{
  InitData();
  m_pFilePtr = OSCL_FileOpen(filename, (OSCL_TCHAR *) _T("rb"));
  uint64 size = OSCL_FileSize(filename);
  m_pFlacParser = MM_New_Args(FlacParser,(this,size,FlacFileCallbakGetData));
  (void)ParseMetaData();
}
#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
flacfile::flacfile(video::iStreamPort* pport)
{
  InitData();
  m_pPort = pport;
  m_pFilePtr = OSCL_FileOpen(pport);
  int64 noffset = 0;
  uint64 size = MAX_FILE_SIZE;
  if(m_pPort)
  {
    if(video::iStreamPort::DS_SUCCESS == m_pPort->GetContentLength(&noffset))
    {
      size = (uint64)noffset;
    }
  }
  m_pFlacParser = MM_New_Args(FlacParser,(this,size,FlacFileCallbakGetData));
  (void)ParseMetaData();
}
#endif
/* ======================================================================
FUNCTION:
  flacfile::InitData

DESCRIPTION:
  Initializes all the data members to default

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
  None.

SIDE EFFECTS:
  None.
======================================================================*/
void flacfile::InitData()
{
  memset(&m_sampleInfo,0,
         (FILE_MAX_MEDIA_STREAMS * sizeof(file_sample_info_type)) );

#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
  m_pPort           = NULL;
#endif
  m_filename        = NULL;
  m_pFileBuf        = NULL;
  m_pFilePtr        = NULL;
  m_pFlacParser     = NULL;
  _fileErrorCode    = PARSER_ErrorDefault;
  m_FileBufSize     = 0;
  m_fileSize        = 0;
  m_nNumStreams     = 0;
  m_bStreaming      = false;
  _success          = false;
  m_bMediaAbort     = false;
  m_pFlacIndTrackIdTable = NULL;
  m_bIsSingleFrameMode   = true;
  m_hFrameOutputModeEnum = FILE_SOURCE_MEDIA_OUTPUT_SINGLE_AUDIO_FRAME;
}
void flacfile::SetCriticalSection(MM_HANDLE pcriticalsection)
{
  if(m_pFilePtr)
  {
    m_pFilePtr->pCriticalSection = pcriticalsection;
  }
}
/* ======================================================================
FUNCTION:
  flacfile::~flacfile

DESCRIPTION:
  OGGStream constructor

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
  None.

SIDE EFFECTS:
  None.
======================================================================*/
flacfile::~flacfile()
{
  if(m_pFilePtr)
  {
      OSCL_FileClose(m_pFilePtr);
  }
  if(m_pFlacParser)
  {
    MM_Delete(m_pFlacParser);
  }
  if(m_pFlacIndTrackIdTable)
  {
    MM_Free(m_pFlacIndTrackIdTable);
  }
}
/* ======================================================================
FUNCTION:
  flacfile::ParseMetaData

DESCRIPTION:
  Starts parsing the MPEG2 transport stream.

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
  None.

SIDE EFFECTS:
  None.
======================================================================*/
bool flacfile::ParseMetaData()
{
  bool bRet = false;
  uint64 localoffset = 0;
  bool bOK = false;

  if(m_pFlacParser)
  {
    while(1)
    {
      if(m_pFlacParser->StartParsing(localoffset, true) == FLACPARSER_SUCCESS)
      {
        if(m_pFlacParser->IsMetaDataParsingDone())
        {
          bOK = true;
          break;
        }
      }
    }
    if(bOK)
    {
      bRet = true;
      _success = true;
      _fileErrorCode = PARSER_ErrorNone;
      m_nNumStreams = m_pFlacParser->GetTotalNumberOfAudioTracks();
      if(m_nNumStreams)
      {
        uint32* idlist = (uint32*)MM_Malloc(m_nNumStreams * sizeof(uint32) );
        m_pFlacIndTrackIdTable = (FlacTrackIdToIndexTable*)MM_Malloc(m_nNumStreams * sizeof(FlacTrackIdToIndexTable));
        if(m_pFlacIndTrackIdTable && idlist)
        {
          memset(m_pFlacIndTrackIdTable,0,m_nNumStreams * sizeof(FlacTrackIdToIndexTable));
          if(m_pFlacParser->GetTrackWholeIDList(idlist) == m_nNumStreams)
          {
            for(uint32 i = 0; i < m_nNumStreams; i++)
            {
              m_pFlacIndTrackIdTable[i].index = (uint8)i;
              m_pFlacIndTrackIdTable[i].bValid = true;
              m_pFlacIndTrackIdTable[i].trackId = idlist[i];
            }
          }
        }
        if(idlist)
        {
          MM_Free(idlist);
        }
      }
    }//if(bOK)
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
PARSER_ERRORTYPE flacfile::getNextMediaSample(uint32 ulTrackID, uint8 *pucDataBuf,
                                              uint32 *pulBufSize,
                                              uint32& /*rulIndex*/)
{
  int32 nBytes = 0;
  FlacParserStatus retError = FLACPARSER_DEFAULT_ERROR;

  /* Validate input params and class variables */
  if(NULL == m_pFlacParser || NULL == pulBufSize || NULL == pucDataBuf ||
     0 == *pulBufSize)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
               "getNextMediaSample invalid input params!!");
    return PARSER_ErrorInvalidParam;
  }

#ifdef FLAC_PARSER_DEBUG
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"flacfile::getNextMediaSample");
#endif

  nBytes = *pulBufSize;
  retError = m_pFlacParser->GetCurrentSample(ulTrackID, pucDataBuf, *pulBufSize,
                                             &nBytes, m_bIsSingleFrameMode);
  if(nBytes > 0 && FLACPARSER_SUCCESS == retError)
  {
    bool bError = false;
    uint8 index = MapTrackIdToIndex(&bError, ulTrackID);
    if(!bError)
    {
      m_sampleInfo[index].num_frames = 1;
      m_sampleInfo[index].sample++;
      m_sampleInfo[index].size = nBytes;
      m_sampleInfo[index].sync = 1;
      MM_MSG_PRIO3(MM_FILE_OPS, MM_PRIO_HIGH,
                   "getNextMediaSample Sample# %lu TIME %llu SampleSize %ld ",
                   m_sampleInfo[index].sample,m_sampleInfo[index].time,nBytes);
      if (m_bIsSingleFrameMode)
      {
        m_sampleInfo[index].btimevalid = true;
        m_pFlacParser->GetCurrentSampleTimeStamp(ulTrackID,&(m_sampleInfo[index].time));
      }

    }
    else
    {
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
                 "getNextMediaSample MapTrackIdToIndex failed for trackid %lu",
                 ulTrackID);
      return PARSER_ErrorInvalidTrackID;
    }
  }

#ifdef FLAC_PARSER_DEBUG
  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
               "getNextMediaSample returing nBytesRead %ld", nBytes);
#endif
  *pulBufSize = nBytes;
  if(FLACPARSER_SUCCESS == retError)
    return PARSER_ErrorNone;
  else
    return PARSER_ErrorEndOfFile;
}
/* ======================================================================
FUNCTION:
  flacfile::getMediaTimestampForCurrentSample

DESCRIPTION:
  gets time stamp of current sample of the track.

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 time stamp in track time scale unit

SIDE EFFECTS:
  None.
======================================================================*/
uint64 flacfile::getMediaTimestampForCurrentSample(uint32 id)
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
  flacfile::GetFlacCodecData

DESCRIPTION:
  Extracts the stream properties "FLAC" properties.

INPUT/OUTPUT PARAMETERS:
  ulTrackId     Track Id.
  pData         Flac Format Data structure

RETURN VALUE:
 "TRUE" in case of success, else returns false

SIDE EFFECTS:
  None.
======================================================================*/
bool flacfile::GetFlacCodecData(int ulTrackId,flac_format_data* pData)
{
  bool bRet = false;
  if((pData) && (m_pFlacParser))
  {
    flac_metadata_streaminfo pstreaminfo;
    memset(&pstreaminfo,0,sizeof(flac_metadata_streaminfo));
    if(FLACPARSER_SUCCESS ==
       m_pFlacParser->GetFlacStreamInfo(ulTrackId,&pstreaminfo))
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
  return bRet;
}
/* ======================================================================
FUNCTION:
  flacfile::GetStreamParameter

DESCRIPTION:
  Extracts the stream properties "FLAC" properties.

INPUT/OUTPUT PARAMETERS:
  ulTrackId     Track Id.
  ulParamIndex  Index "FS_IndexParamAudioFlac"
  pParamStruct  Pointer which contains structure for flac

RETURN VALUE:
 PARSER_ErrorNone in case of success, else returns corresponding error

SIDE EFFECTS:
  None.
======================================================================*/
PARSER_ERRORTYPE flacfile::GetStreamParameter( uint32 ulTrackId,
                                               uint32 ulParamIndex,
                                               void*  pParamStruct)
{
  PARSER_ERRORTYPE eError = PARSER_ErrorInvalidParam;
  if ((FS_IndexParamAudioFlac == ulParamIndex) && (pParamStruct))
  {
    bool bRet = GetFlacCodecData(ulTrackId, (FlacFormatData*)pParamStruct);
    if(bRet)
    {
      eError = PARSER_ErrorNone;
    }
  }
  return eError;
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
uint64 flacfile::resetPlayback(uint64 repos_time,
                                uint32 id,
                                bool /*bSetToSyncSample*/,
                                bool *bError,
                                uint64 currentPosTimeStamp)
{
  uint64 newTS = 0;
  bool bforward = (repos_time > currentPosTimeStamp)?1:0;
  flac_stream_sample_info flac_sampleInfo;
  memset(&flac_sampleInfo,0,sizeof(flac_stream_sample_info));

  MM_MSG_PRIO3(MM_FILE_OPS, MM_PRIO_HIGH,
             "resetPlayback id %lu, repos_time %llu, currentPosTimeStamp %llu",
             id, repos_time, currentPosTimeStamp);

  if(bError && m_pFlacParser)
  {
    *bError = true;
    if(FLACPARSER_SUCCESS == m_pFlacParser->Seek(id,repos_time,currentPosTimeStamp,&flac_sampleInfo,bforward))
    {
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
                  "Seek Succeed, new TS %llu", flac_sampleInfo.ntime);
      bool bMapError = false;
      uint8 index = MapTrackIdToIndex(&bMapError,id);
      if(!bMapError)
      {
        *bError = false;
        _fileErrorCode = PARSER_ErrorNone;
        m_sampleInfo[index].num_frames = 1;
        m_sampleInfo[index].time = flac_sampleInfo.ntime;
        m_sampleInfo[index].btimevalid = true;
        newTS = m_sampleInfo[index].time;
      }
      else
      {
        MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_FATAL,
                     "!!!MapTrackIdToIndex failed for trackid %lu!!!", id);
        _fileErrorCode = PARSER_ErrorInvalidTrackID;
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
  flacfile::GetClipMetaData

DESCRIPTION:
  Provides different metadata fields info in the o/p buffer

INPUT/OUTPUT PARAMETERS:
  pucDataBuf      Buf pointer (NULL during length calculation)
  pulDatabufLen   Buf size pointer
  ienumData       ENUM which is required

RETURN VALUE:
 PARSER_ErrorNone if Success
 Error status, if failure

SIDE EFFECTS:
  None.
======================================================================*/
PARSER_ERRORTYPE flacfile::GetClipMetaData(wchar_t *pucDataBuf,
                                          uint32 *pulDatabufLen,
                                          FileSourceMetaDataType ienumData)
{
  OGGMETADATAINDEX eMetaDataIndex = MAX_FIELDNAMES_SUPPORTED;
  PARSER_ERRORTYPE  eRetStatus     = PARSER_ErrorNone;
  if(pulDatabufLen == NULL || m_pFlacParser == NULL)
  {
    return PARSER_ErrorInvalidParam;
  }
  flac_meta_data *pTagInfo = NULL;
  m_eEncodeType = FS_ENCODING_TYPE_UNKNOWN;

  switch (ienumData)
  {
  case FILE_SOURCE_MD_TITLE:
    eMetaDataIndex     = TAG_OGG_TITLE;
    break;
  case FILE_SOURCE_MD_ARTIST:
    eMetaDataIndex     = TAG_OGG_ARTIST;
    break;
  case FILE_SOURCE_MD_ALBUM:
    eMetaDataIndex   = TAG_OGG_ALBUM;
    break;
  case FILE_SOURCE_MD_COMPOSER:
    eMetaDataIndex   = TAG_OGG_COMPOSER;
    break;
  case FILE_SOURCE_MD_GENRE:
    eMetaDataIndex   = TAG_OGG_GENRE;
    break;
  case FILE_SOURCE_MD_CREATION_DATE:
    eMetaDataIndex   = TAG_OGG_DATE;
    break;
  case FILE_SOURCE_MD_TRACK_NUM:
    eMetaDataIndex   = TAG_OGG_TRACKNUMBER;
    break;
  case FILE_SOURCE_MD_LOCATION:
    eMetaDataIndex   = TAG_OGG_LOCATION;
    break;
  case FILE_SOURCE_MD_ALBUM_ARTIST:
    eMetaDataIndex   = TAG_OGG_ALBUMARTIST;
    break;
  case FILE_SOURCE_MD_VERSION:
    eMetaDataIndex   = TAG_OGG_VERSION;
    break;
  default:
    eMetaDataIndex   = MAX_FIELDNAMES_SUPPORTED;
    break;
  }
  eRetStatus = PARSER_ErrorNone;
  if (MAX_FIELDNAMES_SUPPORTED != eMetaDataIndex )
  {
    FlacParserStatus eStatus =
     m_pFlacParser->GetClipMetaData(eMetaDataIndex, (uint8*)pucDataBuf, pulDatabufLen);
    if (FLACPARSER_SUCCESS != eStatus)
    {
      eRetStatus = PARSER_ErrorInvalidParam;
    }
    else
    {
      eRetStatus        = PARSER_ErrorNone;
      m_eEncodeType = FS_TEXT_ENC_UTF8;
    }
  }

  return eRetStatus;
}
/* ======================================================================
FUNCTION:
  flacfile::getAlbumArt

DESCRIPTION:
  Provides picture data (album art) in the o/p buffer

INPUT/OUTPUT PARAMETERS:
  pucDataBuf and pulDatabufLen.

RETURN VALUE:
 PARSER_ErrorNone if Success
 Error status, if failure

SIDE EFFECTS:
  None.
======================================================================*/
PARSER_ERRORTYPE flacfile::getAlbumArt(wchar_t *pucDataBuf, uint32 *pulDatabufLen)
{
  PARSER_ERRORTYPE eRetError = PARSER_ErrorDefault;
  FS_ALBUM_ART_METADATA *pAlbArt = (FS_ALBUM_ART_METADATA*)pucDataBuf;
  if(pulDatabufLen == NULL || m_pFlacParser == NULL)
  {
    return PARSER_ErrorInvalidParam;
  }
  eRetError = PARSER_ErrorNone;
  FlacParserStatus eStatus = m_pFlacParser->getAlbumArt(pAlbArt, pulDatabufLen);
  if (FLACPARSER_SUCCESS != eStatus)
  {
    eRetError = PARSER_ErrorInvalidParam;
  }

  return PARSER_ErrorNone;
}

/* ======================================================================
FUNCTION:
  flacfile::getMovieDurationMsec

DESCRIPTION:
  gets movie duration in milli secs

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
uint64 flacfile::getMovieDurationMsec() const
{
  uint64 nDuration = 0;
  if(!m_pFlacParser)
  {
    return nDuration;
  }
  nDuration = m_pFlacParser->GetClipDurationInMsec();
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
uint32 flacfile::getTrackWholeIDList(uint32 *ids)
{
  if(!m_pFlacParser)
  {
    return 0;
  }
  return (m_pFlacParser->GetTrackWholeIDList(ids));
}

/* ======================================================================
FUNCTION:
  flacfile::getTrackAudioSamplingFreq

DESCRIPTION:
  gets audio track's sampling frequency

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
uint32 flacfile::getTrackAudioSamplingFreq(uint32 id)
{
  uint32 ulSamplingFreq = 0;
  if(m_pFlacParser)
  {
    flac_metadata_streaminfo sInfo;
    FlacParserStatus eRet = m_pFlacParser->GetFlacStreamInfo(id, &sInfo);
    if (FLACPARSER_SUCCESS == eRet)
    {
      ulSamplingFreq = sInfo.nSamplingRate;
    }
  }
  return ulSamplingFreq;
}

/* ============================================================================
  @brief  Returns number of bits used for each audio sample

  @details    This function is used to return no. of bits used for each
              audio sample.

  @param[in]      trackId       Track Id number.

  @return     MKAV_API_SUCCESS indicating sample read successfully.
              Else, it will report corresponding error.
  @note       BufferSize should be more than maximum frame size value.
============================================================================ */
uint32 flacfile::GetAudioBitsPerSample(int trackId)
{
  uint32 ulBitWidth = 0;
  if(m_pFlacParser)
  {
    flac_metadata_streaminfo sInfo;
    FlacParserStatus eRet = m_pFlacParser->GetFlacStreamInfo(trackId, &sInfo);
    if (FLACPARSER_SUCCESS == eRet)
    {
      ulBitWidth = sInfo.nBitsPerSample;
    }
  }
  return ulBitWidth;
}

/* ======================================================================
FUNCTION:
  flacfile::GetNumAudioChannels

DESCRIPTION:
  returns number of audio channels

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
uint32 flacfile::GetNumAudioChannels(int id)
{
  uint32 ulNumChannels = 0;
  if(m_pFlacParser)
  {
    flac_metadata_streaminfo sInfo;
    FlacParserStatus eRet = m_pFlacParser->GetFlacStreamInfo(id, &sInfo);
    if (FLACPARSER_SUCCESS == eRet)
    {
      ulNumChannels = sInfo.nChannels;
    }
  }
  return ulNumChannels;
}
/* ======================================================================
FUNCTION:
  flacfile::peekCurSample

DESCRIPTION:
  gets information about current sample of a track

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
PARSER_ERRORTYPE flacfile::peekCurSample(uint32 trackid, file_sample_info_type *pSampleInfo)
{
  PARSER_ERRORTYPE reterror = PARSER_ErrorDefault;
  if((!m_pFlacParser) || (!pSampleInfo))
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,"flacfile::peekCurSample invalid argument");
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
  flacfile::getTrackMaxBufferSizeDB

DESCRIPTION:
  gets maximum buffer size to play the track

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
int32  flacfile::getTrackMaxBufferSizeDB(uint32 id)
{
  uint32 bufSize = 0;
  if(m_pFlacParser)
  {
    bufSize = m_pFlacParser->GetFlacMaxBufferSize(id);
  }
  return bufSize;
}

/* ======================================================================
FUNCTION:
  flacfile::getTrackDecoderSpecificInfoContent

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
PARSER_ERRORTYPE flacfile::getTrackDecoderSpecificInfoContent(uint32 id, uint8* buf, uint32 *pbufSize)
{
  PARSER_ERRORTYPE errorCode = PARSER_ErrorDefault;
  if( (!m_pFlacParser) || (!pbufSize) )
  {
    return errorCode;
  }
  uint32 size = m_pFlacParser->GetCodecHeaderSize(id);
  uint8* header = m_pFlacParser->GetCodecHeader(id);
  if(buf && (*pbufSize >= size) && size)
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
  flacfile::MapTrackIdToIndex

DESCRIPTION:
  Maps given track-id to track index.

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
uint8 flacfile::MapTrackIdToIndex(bool* bError,uint32 trackid)
{
  uint8 index = 0;
  if(bError)
  {
    *bError = true;
    for(uint32 i = 0; i < m_nNumStreams; i++)
    {
      if( (m_pFlacIndTrackIdTable[i].trackId == trackid) &&
        (m_pFlacIndTrackIdTable[i].bValid) )
      {
        index = m_pFlacIndTrackIdTable[i].index;
        *bError = false;
        break;
      }
    }
  }
  if(bError && *bError)
  {
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_FATAL,
                 "flacfile::MapTrackIdToIndex failed for trackid %lu",trackid);
  }
  return index;
}

/* ======================================================================
FUNCTION:
  SetAudioOutputMode

DESCRIPTION:
  Called by user to set output mode specified by henum

INPUT/OUTPUT PARAMETERS:
  henum-Output mode

RETURN VALUE:
 FILE_SOURCE_SUCCESS if successful in setting output mode
 else returns FILE_SOURCE_FAIL

SIDE EFFECTS:
  None.
======================================================================*/
FileSourceStatus flacfile::SetAudioOutputMode(FileSourceConfigItemEnum henum)
{
  FileSourceStatus status = FILE_SOURCE_FAIL;
  //! Default mode is FILE_SOURCE_MEDIA_OUTPUT_SINGLE_AUDIO_FRAME
  //! Parser do not support changing output mode during the playback
  switch (henum)
  {
    case FILE_SOURCE_MEDIA_OUTPUT_SINGLE_AUDIO_FRAME:
    case FILE_SOURCE_MEDIA_OUTPUT_BYTES_STREAM:
      m_hFrameOutputModeEnum = henum;
      m_bIsSingleFrameMode   = true;
      if (FILE_SOURCE_MEDIA_OUTPUT_BYTES_STREAM == henum)
      {
        m_bIsSingleFrameMode = false;
      }
      status = FILE_SOURCE_SUCCESS;
      break;
    default:
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR, "Unsupported enum value");
  }
  return status;
}

/* ======================================================================
FUNCTION:
  GetAudioOutputMode

DESCRIPTION:
  Called by user to retrieve output mode specified by henum

INPUT/OUTPUT PARAMETERS:
  henum-Output mode

RETURN VALUE:
 FILE_SOURCE_SUCCESS if successful in retrieving output mode
 else returns FILE_SOURCE_FAIL

SIDE EFFECTS:
  None.
======================================================================*/
FileSourceStatus flacfile::GetAudioOutputMode(bool* bret,
                                              FileSourceConfigItemEnum henum)
{
  FileSourceStatus status = FILE_SOURCE_FAIL;
  //! Default mode is FILE_SOURCE_MEDIA_OUTPUT_SINGLE_AUDIO_FRAME
  //! Parser do not support changing output mode during the playback
  switch (henum)
  {
    case FILE_SOURCE_MEDIA_OUTPUT_SINGLE_AUDIO_FRAME:
    case FILE_SOURCE_MEDIA_OUTPUT_BYTES_STREAM:
      if (m_hFrameOutputModeEnum == henum)
      {
        *bret = true;
        status = FILE_SOURCE_SUCCESS;
      }
      break;
    default:
      {
        *bret = false;
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR, "Unsupported enum value");
      }
  }
  return status;
}

#endif

