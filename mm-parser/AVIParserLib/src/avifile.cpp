/* =======================================================================
                              avifile.cpp
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

Copyright 2011-2014 QUALCOMM Technologies Incorporated, All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
========================================================================== */

/* =======================================================================
                             PERFORCE HEADER
$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/AVIParserLib/main/latest/src/avifile.cpp#157 $
========================================================================== */

/* ==========================================================================

                     INCLUDE FILES FOR MODULE
========================================================================== */
#include "parserdatadef.h"
#include "parserinternaldefs.h"
#ifdef FEATURE_FILESOURCE_AVI

#ifdef FEATURE_FILESOURCE_DIVX_DRM
  #include "DrmTypes.h"
  #include "DrmApi.h"
  #ifndef FILESOURCE_LEGACY_DIVX_DRM
    #include "DrmApiExt.h"
    #define MAX_JIT_BUFFER_SIZE 1024
    uint8_t  DecryptJITBuffer[MAX_JIT_BUFFER_SIZE];
    #ifdef PLATFORM_LTK
      uint8_t* pDecryptContext;
      uint32_t nDRMContextLength;
    #endif
  #endif
#endif//#ifdef FEATURE_FILESOURCE_DIVX_DRM

#include "avifile.h"
#include "aviparser.h"
#include "aviconstants.h"

//! ADTS can require up to 9bytes for ADTS header
#define AAC_ADTS_MAX_HEADER_SIZE (9)
#ifdef FEATURE_FILESOURCE_DIVX_DRM
  #ifndef FILESOURCE_LEGACY_DIVX_DRM
    #ifdef PLATFORM_LTK
    drmErrorCodes_t SETUP_DECRYPT_BLOCK()
    {
      nDRMContextLength = 0;
      drmInitSystemEx(drmContextRoleDecryption,NULL,&nDRMContextLength);
      pDecryptContext = (uint8_t*)MM_Malloc( nDRMContextLength  );
      drmInitSystemEx(drmContextRoleDecryption,pDecryptContext,&nDRMContextLength);
      drmErrorCodes_t ret = drmInitPlayback(pDecryptContext,NULL);
      return drmCommitPlayback(pDecryptContext);
    }
    drmErrorCodes_t DECRYPT_VIDEO_CHUNK(uint8_t* buf,uint32_t size)
    {
      return drmDecryptVideoEx(pDecryptContext,buf, size);
    }
    void RELEASE_DECRYPT_BLOCK()
    {
      (void)drmFinalizePlayback(pDecryptContext);
      if(pDecryptContext)
      {
        MM_Free(pDecryptContext);
      }
      pDecryptContext = NULL;
      nDRMContextLength = 0;
    }
    #endif//#ifdef PLATFORM_LTK
  #endif//#ifndef FILESOURCE_LEGACY_DIVX_DRM
#endif//#ifdef FEATURE_FILESOURCE_DIVX_DRM
/* ==========================================================================

                DEFINITIONS AND DECLARATIONS FOR MODULE

This section contains definitions for constants, macros, types, variables
and other items needed by this module.

========================================================================== */

/* -----------------------------------------------------------------------
** Constant / Define Declarations
** ----------------------------------------------------------------------- */
//#define __TEST

#ifdef FEATURE_FILESOURCE_FILE_IO_CACHING
#define FILE_READ_BUFFER_SIZE_FOR_AVI 10240
#endif

#define ALLOW_REVERSE_4CC 1

#ifdef FEATURE_FILESOURCE_DIVX_DRM
PARSER_ERRORTYPE MAP_DRM_ERROR_2_PARSER_ERROR_CODE(drmErrorCodes_t error_code)
{
  if(error_code == DRM_NOT_AUTHORIZED)
  {
    return PARSER_ErrorDRMAuthorization;
  }
  if(error_code == DRM_NOT_REGISTERED)
  {
    return PARSER_ErrorDRMDeviceNotRegistered;
  }
  if(error_code == DRM_RENTAL_EXPIRED)
  {
    return PARSER_ErrorDRMRentalCountExpired;
  }
  return PARSER_ErrorDRMPlaybackError;
}
#endif

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
  AVICheckAvailableData

DESCRIPTION
  Its a callback function from the SDK to get the amount of AVI data available.
  This is not implemented by the SDK. It should be implemented by the app that calls the SDK.

ARGUMENTS
  u32UserData             Extra info From App. Given by user in aviin_open

DEPENDENCIES
  Used by AVI Parser.

RETURN VALUE
  The number of bytes available

SIDE EFFECTS
  Detail any side effects.

========================================================================== */
avi_int64 AVICheckAvailableData (void* pUserData)
{
  if(pUserData)
  {
    AVIFile *pAviFile = (AVIFile *)pUserData;
    return(pAviFile->CheckAvailableData());
  }
  return 0;
}

/* ======================================================================
FUNCTION
  AVICallbakGetData

DESCRIPTION
  Its a callback function from the SDK to get the AVI data for decoding.
  This is not implemented by the SDK. It should be implemented by the app
  that calls the SDK.

ARGUMENTS
  ullOffset           Offset of the requested data (from beginning),
  ulNumBytesRequest   Size of the requested data (in bytes).
  pucData             Pointer to data buffer.
  ulMaxBufSize        Maximum data buffer size.
  pUserData           Client user data information.

DEPENDENCIES
  Used by AVI Parser.

RETURN VALUE
  The number of bytes actually being filled in

SIDE EFFECTS
  Detail any side effects.
========================================================================== */
avi_int32 AVICallbakGetData (uint64 ullOffset,
                             uint32 ulNumBytesRequest,
                             uint8* pucData,
                             uint32 ulMaxBufSize,
                             void*  pUserData)
{
  if(pUserData)
  {
    AVIFile *pAviFile = (AVIFile *)pUserData;
    return(pAviFile->FileGetData( ullOffset,
                                  ulNumBytesRequest,
                                  ulMaxBufSize, pucData));
  }
  return 0;
}
/* ======================================================================
FUNCTION:
  AVIFile::CheckAvailableData

DESCRIPTION:
  Returns amount of data available(valid only when valid IStreamPort exists)

INPUT/OUTPUT PARAMETERS:

RETURN VALUE:
 Number of bytes available from IStreamPort

SIDE EFFECTS:
  None.
======================================================================*/
avi_uint64 AVIFile::CheckAvailableData()
{
  int64 nOffset = 0;
#if defined(FEATURE_FILESOURCE_PSEUDO_STREAM) || defined(FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD)
  if(m_pStreamPort)
  {
    bool bend = false;
    (void)m_pStreamPort->GetAvailableOffset(&nOffset,&bend);
  }
#endif
  return nOffset;
}
/* ======================================================================
FUNCTION:
  AVIFile::FileGetData

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
uint32 AVIFile::FileGetData(  uint64 nOffset,
                              uint32 nNumBytesRequest,
                              avi_uint32 nMaxSize,
                              uint8* pData  )
{
  uint32 nRead = 0;
  if (m_AviFilePtr != NULL)
  {
    if(!m_bMediaAbort)
    {
      nRead = FileBase::readFile(m_AviFilePtr, pData, nOffset, FILESOURCE_MIN(nNumBytesRequest,nMaxSize));
    }
    else
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR, "Breaking, user Abort is true.");
    }
  }
  return nRead;
}
/* ==============================================================================
FUNCTION:
  AVIFile::getFileSize

DESCRIPTION:
  Returns the size of the file

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
  File size

SIDE EFFECTS:
  None.
==============================================================================*/
uint64 AVIFile::getFileSize()
{
  return m_fileSize;
}

/* =============================================================================
FUNCTION:
  AVIFile::getCodecName

DESCRIPTION:
  Returns the name of the codec

INPUT/OUTPUT PARAMETERS:
  codecName          Buffer to copy the codec name into
  bufLen             Length of buffer to store the codec name
  mediaType          Specifies which codec name to get, audio or video

RETURN VALUE:
  None

SIDE EFFECTS:
  None
==============================================================================*/
void AVIFile::getCodecName(char *codecName,uint32 bufLen,uint32 trackId)
{
  CHUNK_t chunkType;
  int min = 0;
  avi_video_info pvInfo;
  avi_audio_info paInfo;

  if(!m_pAVIParser || (!codecName)|| (!bufLen))
  {
    return;
  }
  if(m_pAVIParser->GetTrackChunkType(trackId,&chunkType)==AVI_SUCCESS)
  {
    switch(chunkType)
    {

    case AVI_CHUNK_AUDIO:
      if(m_pAVIParser->GetAudioInfo(trackId,&paInfo)==AVI_SUCCESS)
      {
        if(paInfo.strnAudio.streamName)
        {
          min = (paInfo.strnAudio.streamNameSize <= bufLen)?paInfo.strnAudio.streamNameSize:bufLen;
          memcpy(codecName,paInfo.strnAudio.streamName,min);
        }
      }
      break;

    case AVI_CHUNK_VIDEO:
      if(m_pAVIParser->GetVideoInfo(trackId,&pvInfo)==AVI_SUCCESS)
      {
        if(pvInfo.strnVideo.streamName)
        {
          min = (pvInfo.strnVideo.streamNameSize <= bufLen)?pvInfo.strnVideo.streamNameSize:bufLen;
          memcpy(codecName,pvInfo.strnVideo.streamName,min);
        }
      }
      break;

    default:
      break;
    }
  }
}

/* ======================================================================
FUNCTION:
  AVIFile::AVIFile

DESCRIPTION:
  default constructor

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
AVIFile::AVIFile() // Default Constructor
{
  InitData();
}
/* ======================================================================
FUNCTION:
  AVIFile::InitData

DESCRIPTION:
  Initializer function for class members

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
void AVIFile::InitData()
{
  _success = false;
  _fileErrorCode = PARSER_ErrorDefault;
  m_playAudio = false;
  m_playVideo = false;
  m_playText = false;
  m_corruptFile = false;
  m_bVideoReposPending = false;
  m_bAudioReposPending = false;
  m_nNumStreams = 0;
  m_bMediaAbort = false;
  m_pAVIParser = NULL;
  m_bAudioFrameBoundary = false;
#ifdef FEATURE_FILESOURCE_DIVX_DRM
  m_pClipDrmInfo = NULL;
  drmContext = NULL;
  drmContextLength = 0;
  m_pDRMExDDInfo = NULL;
  memset(drm_frame_info,0,DIVX_DRM_FRAME_DRM_INFO_SIZE);
#endif

  m_aviParseLastStatus = AVI_SUCCESS;
  m_bframesMayPresent = FALSE;
  m_repositionProgress = FALSE;
  m_videoFramesReadAhead.firstVideoFrame = FALSE;
  m_videoFramesReadAhead.currentSampleIndex = 0;
  m_videoFramesReadAhead.validSampleIndex = 0;
  m_videoFramesReadAhead.allocatedForReadAhead = FALSE;
  /* Initializing read ahead buffer
     This is to adjust the time stamps for Mpeg4 if there are any B-Frames
  */
  for(uint8 index = 0; index<AVI_MAX_VIDEO_FRAMES_READ_AHEAD; index++)
  {
    m_avi_video_samples[index].vop_type = NO_VOP;
    m_avi_video_samples[index].bVopCount = 0;
    m_avi_video_samples[index].size = 0;

    m_avi_video_samples[index].m_sampleInfo.time = 0;
    m_avi_video_samples[index].m_sampleInfo.size = 0;
    m_avi_video_samples[index].m_sampleInfo.sync = 0;
    m_avi_video_samples[index].m_sampleInfo.delta = 0;

    if( m_avi_video_samples[index].buff != NULL)
    {
      m_avi_video_samples[index].buff = NULL;
    }
  }
  //Initialized audio, video and binary streamIDs.
  m_nSelectedAudioStreamId = -1;
  m_nSelectedVideoStreamId = -1;
  m_nSelectedTextStreamId = -1;

  memset(m_sampleInfo, 0, sizeof(m_sampleInfo));
  memset(m_nDecodedDataSize, 0, sizeof(m_nDecodedDataSize));
  memset(m_nLargestFrame, 0, sizeof(m_nLargestFrame));

  //Initializing largest sample size for Audio and Video to 0
  m_audioLargestSize = 0;
  m_videoLargestSize = 0;
  m_pFileBuf = NULL;
  m_FileBufSize = 0;
  m_fileSize = 0;
  m_filename = (OSCL_TCHAR*) _T("");
  m_bStreaming = FALSE;
  m_AviFilePtr = NULL;
  m_pAACAudioInfo = NULL;
  memset(&m_hMP3Sync,0,MP3_MIN_FRAME_HEADER_SIZE);
  m_bSetSync = false;
  //by default, parser won't process anything for the data being read for audio AU
  m_hFrameOutputModeEnum = FILE_SOURCE_MEDIA_OUTPUT_BYTES_STREAM;
  //by default, parser won't look for any codec speicific frame header from the data being read for AU.
  m_hHeaderOutputModeEnum = FILE_SOURCE_MEDIA_RETAIN_AUDIO_HEADER;

#if defined(FEATURE_FILESOURCE_PSEUDO_STREAM) || defined(FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD)
  m_bEndOfData = FALSE;
  m_minOffsetRequired = 0;
  m_bEndOfData = FALSE;
  bHttpStreaming = FALSE;
  m_pStreamPort = NULL;
  bHttpStreaming = false;
  m_wBufferOffset = 0;
  bGetMetaDataSize = TRUE;
  bIsMetaDataParsed = FALSE;
  m_HttpDataBufferMinOffsetRequired.Offset = 0;
  m_HttpDataBufferMinOffsetRequired.bValid = FALSE;
  parserState = PARSER_IDLE;
  memset(m_maxPlayableTime, 0, sizeof(m_maxPlayableTime));
#endif //FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
}
/* ======================================================================
FUNCTION:
  SetAudioOutputMode

DESCRIPTION:
  Called by user to set output mode specified by henum

INPUT/OUTPUT PARAMETERS:
  henum-Output mode

RETURN VALUE:
 FILE_SOURCE_SUCCESS if successful in setting output mode else returns FILE_SOURCE_FAIL

SIDE EFFECTS:
  None.
======================================================================*/
FileSourceStatus AVIFile::SetAudioOutputMode(FileSourceConfigItemEnum henum)
{
  FileSourceStatus status = FILE_SOURCE_FAIL;
  /*
  * FILE_SOURCE_MEDIA_OUTPUT_BYTES_STREAM means there won't be any processing done on data read from AU.
  */
  if((henum == FILE_SOURCE_MEDIA_OUTPUT_BYTES_STREAM)||
     (henum == FILE_SOURCE_MEDIA_OUTPUT_SINGLE_AUDIO_FRAME))
  {
    m_hFrameOutputModeEnum = henum;
    status = FILE_SOURCE_SUCCESS;
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM, "AudioOutputMode %d",henum);
  }
  else if( (henum == FILE_SOURCE_MEDIA_STRIP_AUDIO_HEADER)||
           (henum == FILE_SOURCE_MEDIA_RETAIN_AUDIO_HEADER) )
  {
    m_hHeaderOutputModeEnum = henum;
    status = FILE_SOURCE_SUCCESS;
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM, "HeaderOutputMode %d",henum);
  }
  else
  {
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_FATAL, "SetAudioOutputMode failed for configItem %d",henum);
  }
  return status;
}
/* ======================================================================
FUNCTION:
  GetAudioOutputMode

DESCRIPTION:
  Called by user to check what audio audio output mode is set

INPUT/OUTPUT PARAMETERS:

RETURN VALUE:
 FILE_SOURCE_SUCCESS if successful in retrieving output mode else returns FILE_SOURCE_FAIL

SIDE EFFECTS:
  None.
======================================================================*/
FileSourceStatus AVIFile::GetAudioOutputMode(bool* bret, FileSourceConfigItemEnum henum)
{
  FileSourceStatus status = FILE_SOURCE_FAIL;
  //if our mode(frame/header) is same as henum,
  if(bret && (henum == m_hFrameOutputModeEnum))
  {
    status = FILE_SOURCE_SUCCESS;
    *bret = true;
  }
  else if (bret && (henum == m_hHeaderOutputModeEnum) )
  {
    status = FILE_SOURCE_SUCCESS;
    *bret = true;
  }
  else
  {
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH, "GetAudioOutputMode configItem %d not set...",henum);
  }
  return status;
}
#if defined(FEATURE_FILESOURCE_PSEUDO_STREAM) || defined(FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD)
/* ======================================================================
FUNCTION:
  AVIFile::AVIFile

DESCRIPTION:
  constructor for supporting playback from StreamPort.

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
AVIFile::AVIFile( video::iStreamPort* pPort,
                  bool bPlayVideo,
                  bool bPlayAudio)
{
  InitData();
  m_pStreamPort = pPort;
  m_playAudio = bPlayAudio;
  m_playVideo = bPlayVideo;
  bHttpStreaming = true;
  if(!(m_playAudio || m_playVideo))
  {
    //we don't play anything except audio/video.
    //Do not create parser handle.
    _fileErrorCode = PARSER_ErrorNone;
    _success = true;
    m_playText = true;
  }
#if defined(FEATURE_FILESOURCE_PSEUDO_STREAM) || defined(FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD)
  if (pPort)
  {
     int64 nStreamSize = 0;
     int64 nDownloadedBytes = 0;
     if(video::iStreamPort::DS_SUCCESS == pPort->GetContentLength(&nStreamSize))
     {
       m_fileSize = (uint64)nStreamSize;
     }
     else
     {
       m_fileSize = MAX_FILE_SIZE;
     }
     m_AviFilePtr = OSCL_FileOpen(pPort);
     /*if bEndofData is true,then OEM has indicated that the entire clip is available*/
     m_pStreamPort->GetAvailableOffset(&nDownloadedBytes, &m_bEndOfData);
     m_wBufferOffset = (uint64)nDownloadedBytes;
  }
#endif //FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
  if(!m_AviFilePtr )
  {
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_ERROR, "Failed to create m_AviFilePtr");
    _success = false;
    return;
  }
  if(!m_fileSize)
  {
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_ERROR, "Invalid file size %llu",m_fileSize);
    _success = false;
    return;
  }

  if(m_AviFilePtr)
  {
    (void)parseHTTPStream();
  }
}
#endif
/* ======================================================================
FUNCTION:
  AVIFile::AVIFile

DESCRIPTION:
  local file playback Constructor

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
AVIFile::AVIFile( const FILESOURCE_STRING &filename
                  ,unsigned char *pFileBuf
                  ,uint32 bufSize
                  ,bool bPlayVideo
                  ,bool bPlayAudio
#if defined(FEATURE_FILESOURCE_PSEUDO_STREAM) || defined(FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD)
                 ,bool bHttpStream
                 ,uint64 wBufferOffset
#endif //FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
                )
{
  InitData();
  m_playAudio = bPlayAudio;
  m_playVideo = bPlayVideo;
  if(!(m_playAudio || m_playVideo))
  {
    //we don't play anything except audio/video.
    //Do not create parser handle.
    _fileErrorCode = PARSER_ErrorNone;
    _success = true;
    m_playText = true;
  }
  if (pFileBuf && bufSize)
  {
    m_pFileBuf = pFileBuf;
    m_FileBufSize = bufSize;
    m_fileSize = bufSize;
    m_filename = (OSCL_TCHAR*) _T("");
    m_bStreaming = FALSE;
    m_AviFilePtr = OSCL_FileOpen (pFileBuf, bufSize);
  }
#if defined(FEATURE_FILESOURCE_PSEUDO_STREAM) || defined(FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD)
#endif //FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
  else
  {
    m_filename = filename;
    m_pFileBuf = NULL;
    m_FileBufSize = 0;
    m_bStreaming = FALSE;
#ifdef FEATURE_FILESOURCE_FILE_IO_CACHING
    /* Calling with 10K cache  buffer size */
    m_AviFilePtr = OSCL_FileOpen(filename, (OSCL_TCHAR *) _T("rb"), FILE_READ_BUFFER_SIZE_FOR_AVI );
#else
    m_AviFilePtr = OSCL_FileOpen(filename, (OSCL_TCHAR *) _T("rb"));
#endif
    m_fileSize = OSCL_FileSize( m_filename );
  }

  if(m_AviFilePtr)
  {
    if(!ParseMetaData())
    {
      return;
    }

#ifdef __TEST
    if(m_playAudio||m_playVideo)
    {
      FILE*afp;
      FILE* vfp;
      afp = fopen("audioStream_avi.strm", "w");
      vfp = fopen("videoStream_avi.strm", "w");
      int max_video_count = 5;
      int max_audio_count = 5;

      uint8 buff[200000];
      uint32 index = 0;
      int count = 0;
      int nSize = 0;
      int trid = -1;
      CHUNK_t chunkType;
      for(int track = 0; track < m_nNumStreams && (trid < 0); track++)
      {
        // Get the chunk type corresponding to track number.
        m_pAVIParser->GetTrackChunkType(track,&chunkType);

        switch(chunkType)
        {
          case AVI_CHUNK_VIDEO:
          {
            trid = track;
            break;
          }
        }
      }
      if(trid < 0)
      {
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR, "AVIFile::AVIFile error occured while retrieving video track id");
        return;
      }
      bool bFirst = true;
      while(1)
      {
        //getNextMediaSample(uint32 id, uint8 *buf, uint32 size, uint32 &index);
        //if(m_playAudio)
        //{
        //  nSize = getNextMediaSample(trid,buff,200000,index);
        //  //if(max_audio_count > 0)
        //  {
        //    fwrite(buff, 1, nSize, afp);
        //    max_audio_count--;
        //  }
        //  MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_ERROR,
        //         "AVIFile::getNextMediaSample retrieved audio chunk %d nSize %d",count++,nSize);
        //}
        //else
        if(m_playVideo)
        {
          if(bFirst)
          {
            uint8 vcodec = getTrackOTIType(trid);
            if(MPEG4_VIDEO != vcodec)
            {
              int wid = getTrackVideoFrameWidth(trid);
              int ht = getTrackVideoFrameHeight(trid);
              fwrite(&wid, 1, sizeof(int), vfp);
              fwrite(&ht, 1, sizeof(int), vfp);
            }
            else
            {
              uint8* vol  = getTrackDecoderSpecificInfoContent(trid);
              uint32 size = getTrackDecoderSpecificInfoSize(trid);
              fwrite(vol, 1, size, vfp);
              int code  = MPEG4_VOP_HEADER_CODE;
              fwrite(&code, 1, sizeof(int), vfp);
            }
            bFirst = false;
            continue;
          }
          else
          {
            nSize = getNextMediaSample(trid,buff,200000,index);
            if(nSize)
            {
              fwrite(&nSize, 1, sizeof(int), vfp);
              //if(max_video_count > 0)
              {
                fwrite(buff, 1, nSize, vfp);
                max_video_count--;
              }
              MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_ERROR,"AVIFile::getNextMediaSample retrieved video chunk %d nSize %d",count++,nSize);
            }
          }
        }

        if(!nSize)
        {
          break;
        }
      }
      if(vfp)
      {
        fclose(vfp);
      }
    }
#endif
  }
}
/* ======================================================================
FUNCTION:
  AVIFile::SetCriticalSection

DESCRIPTION:
  Sets the critical section to be used in oscl file io

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
void AVIFile::SetCriticalSection(MM_HANDLE pcriticalsection)
{
  if(m_AviFilePtr)
  {
    m_AviFilePtr->pCriticalSection = pcriticalsection;
  }
}
/* ======================================================================
FUNCTION:
  AVIFile::GetLastRetrievedSampleOffset

DESCRIPTION:
  Returns the offset of the last retrieved sample

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
uint64 AVIFile::GetLastRetrievedSampleOffset(uint32 trackid)
{
  uint64 offset = 0;
  CHUNK_t chunkType;
  if(m_pAVIParser)
  {
    if(m_pAVIParser->GetTrackChunkType(trackid,&chunkType)==AVI_SUCCESS)
    {
      offset = m_pAVIParser->GetLastRetrievedSampleOffset(trackid);
    }
  }
  MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM, "GetLastRetrievedSampleOffset %llu",
               offset);
  return offset;
}
#ifdef FEATURE_FILESOURCE_DIVX_DRM

/* ======================================================================
FUNCTION:
  AVIFile::initDivXDrmSystem

DESCRIPTION:
 Initializes the DivX DRM System.

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
bool AVIFile::initDivXDrmSystem()
{
  bool bRet = true;
  if(m_playVideo && IsDRMProtection() && (!drmContext) )
  {
    drmErrorCodes_t result;
    avi_uint8 rentalMessageFlag = 0;
    avi_uint8 useLimit;
    avi_uint8 useCount;
    avi_uint8 cgmsaSignal;
    avi_uint8 acptbSignal;
    avi_uint8 digitalProtectionSignal;
#ifndef FILESOURCE_LEGACY_DIVX_DRM
    avi_uint8 ictSignal;
#endif
    int drm_size = 0;
#ifdef FILESOURCE_LEGACY_DIVX_DRM
    result = drmInitSystem( NULL,&drmContextLength );
#else
    result = drmInitSystemEx(drmContextRoleAuthentication,NULL,&drmContextLength);
#endif

    drmContext = (uint8_t*)MM_Malloc( drmContextLength  );

    if(!drmContext)
    {
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_FATAL, "drmInitSystem memory allocation failed..");
      _fileErrorCode = PARSER_ErrorDRMMemAllocFail;
      return false;
    }
#ifdef FILESOURCE_LEGACY_DIVX_DRM
    result = drmInitSystem( drmContext,&drmContextLength );
#else
    result = drmInitSystemEx(drmContextRoleAuthentication,drmContext,&drmContextLength);
#endif
    if ( DRM_SUCCESS!= result )
    {
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_FATAL, "drmInitSystem failed result %d",result);
      _fileErrorCode = MAP_DRM_ERROR_2_PARSER_ERROR_CODE(result);
      return false;
    }
#ifndef FILESOURCE_LEGACY_DIVX_DRM
    for ( int i = 0; i < SET_RANDOM_SAMPLE_SEED_MIN; i++ )
    {
      drmSetRandomSample( drmContext );
    }
#endif

    avi_uint8* drm_info = GetDRMInfo(&drm_size);
    result = drmInitPlayback( drmContext,drm_info );
    if ( DRM_SUCCESS!= result )
    {
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_FATAL, "drmInitPlayback failed result %d",result);
      _fileErrorCode = MAP_DRM_ERROR_2_PARSER_ERROR_CODE(result);
      return false;
    }
#ifndef FILESOURCE_LEGACY_DIVX_DRM
  #ifdef PLATFORM_LTK
    (void)SETUP_DECRYPT_BLOCK();
  #endif
#endif
    result = drmQueryRentalStatus( drmContext,
                                   &rentalMessageFlag,
                                   &useLimit,
                                   &useCount );
    if ( DRM_SUCCESS!= result )
    {
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_FATAL, "drmQueryRentalStatus failed result %d",result);
      _fileErrorCode = MAP_DRM_ERROR_2_PARSER_ERROR_CODE(result);
      return false;
    }
#ifndef FILESOURCE_LEGACY_DIVX_DRM
    for ( int i = 0; i < SET_RANDOM_SAMPLE_SEED_MIN; i++ )
#else
    for ( int i = 0; i < 3; i++ )
#endif
    {
      drmSetRandomSample( drmContext );
    }
    result = drmQueryCgmsa( drmContext,&cgmsaSignal );
    if ( DRM_SUCCESS!= result )
    {
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_FATAL, "drmQueryCgmsa failed result %d",result);
      _fileErrorCode = MAP_DRM_ERROR_2_PARSER_ERROR_CODE(result);
      return false;
    }
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM, "cgmsa signal %d",cgmsaSignal);

    result = drmQueryAcptb( drmContext,&acptbSignal );
    if ( DRM_SUCCESS!= result )
    {
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_FATAL, "drmQueryAcptb failed result %d",result);
      _fileErrorCode = MAP_DRM_ERROR_2_PARSER_ERROR_CODE(result);
      return false;
    }
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM, "acptb signal %d",acptbSignal);

    result = drmQueryDigitalProtection( drmContext, &digitalProtectionSignal );
    if ( DRM_SUCCESS!= result )
    {
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_FATAL, "drmQueryDigitalProtection failed result %d",result);
      _fileErrorCode = MAP_DRM_ERROR_2_PARSER_ERROR_CODE(result);
      return false;
    }
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM, "digital protection signal %d",digitalProtectionSignal);
#ifndef FILESOURCE_LEGACY_DIVX_DRM
    result = drmQueryIct( drmContext, &ictSignal );
    if ( DRM_SUCCESS!= result )
    {
       MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_FATAL, "drmQueryIct failed result %d",result);
       _fileErrorCode = MAP_DRM_ERROR_2_PARSER_ERROR_CODE(result);
       return false;
    }
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM, "ict signal %d",ictSignal);
#endif
    if(rentalMessageFlag == 0)
    {
      result = drmCommitPlayback( drmContext );
      if ( DRM_SUCCESS != result )
      {
        MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_FATAL, "drmCommitPlayback failed result %d",result);
        _fileErrorCode = MAP_DRM_ERROR_2_PARSER_ERROR_CODE(result);
        return false;
      }
    }
    m_pClipDrmInfo = MM_Malloc( sizeof(ClipDrmInfoT));
    if(m_pClipDrmInfo)
    {
      memset(m_pClipDrmInfo,0,sizeof(ClipDrmInfoT));
      ClipDrmInfoT* clipdrminfo = (ClipDrmInfoT*)m_pClipDrmInfo;
      clipdrminfo->isRental = false;
      if(rentalMessageFlag > 0)
      {
        clipdrminfo->isRental = true;
      }
      clipdrminfo->useLimit = useLimit;
      clipdrminfo->useCounter = useCount;

      clipdrminfo->cgmsaSignal = cgmsaSignal;
      clipdrminfo->acptbSignal = acptbSignal;
      clipdrminfo->digitalProtectionSignal = digitalProtectionSignal;
#ifndef FILESOURCE_LEGACY_DIVX_DRM
      clipdrminfo->ictSignal = ictSignal;
#endif
    }
  }
  else if(m_playVideo && IsDRMProtection())
  {
    if( (_fileErrorCode == PARSER_ErrorDRMAuthorization)       ||
        (_fileErrorCode == PARSER_ErrorDRMDeviceNotRegistered) ||
        (_fileErrorCode == PARSER_ErrorDRMRentalCountExpired)  ||
        (_fileErrorCode == PARSER_ErrorDRMPlaybackError) )
    {
      bRet = false;
    }
  }
  return bRet;
}
/* ======================================================================
FUNCTION:
  AVIFile::CommitDivXPlayback

DESCRIPTION:
 Commit the DivX Playback.

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
bool AVIFile::CommitDivXPlayback()
{
  bool bRet = false;
  if(drmContext)
  {
    drmErrorCodes_t result = drmCommitPlayback( drmContext );
    if ( DRM_SUCCESS == result )
    {
      bRet = true;
    }
    else
    {
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_FATAL, "drmCommitPlayback failed result %d",result);
      _fileErrorCode = MAP_DRM_ERROR_2_PARSER_ERROR_CODE(result);
    }
  }
  return bRet;
}
/* ======================================================================
FUNCTION:
  AVIFile::GetDRMContextInfo

DESCRIPTION:
 Retrieves the context info to be used for decrypting audio/video.

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
avi_uint8* AVIFile::GetDRMContextInfo(avi_uint32* length)
{
  MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_LOW, "AVIFile::GetDRMContextInfo m_playVideo %d m_playAudio %d",m_playVideo,m_playAudio);
  if(!length)
  {
    return NULL;
  }
  *length = drmContextLength;
  return drmContext;
}
/* ======================================================================
FUNCTION:
  AVIFile::SetDRMContextInfo

DESCRIPTION:
 Sets the context info to be used for decrypting audio/video.

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
void AVIFile::SetDRMContextInfo(avi_uint8 *context,avi_uint32 length)
{
  drmContext = context;
  drmContextLength = length;
  MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_LOW, "AVIFile::SetDRMContextInfo m_playVideo %d m_playAudio %d",m_playVideo,m_playAudio);
}
/* ======================================================================
FUNCTION:
  AVIFile::CopyDRMContextInfo

DESCRIPTION:
 Copies the DRM context info from 'ptr' in this object for decrypting audio/video.

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
void AVIFile::CopyDRMContextInfo(void* ptr)
{
  MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_LOW, "AVIFile::CopyDRMContextInfo m_playVideo %d m_playAudio %d",m_playVideo,m_playAudio);
  if(ptr)
  {
    AVIFile* refAviHnd = (AVIFile*)ptr;
    avi_uint32 drmln = 0;
    avi_uint8* drmctx = refAviHnd->GetDRMContextInfo(&drmln);
    SetDRMContextInfo(drmctx,drmln);
  }
}
/* ======================================================================
FUNCTION:
  AVIFile::GetClipDrmInfo

DESCRIPTION:
 Retrieves clip specifc DRM information.

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
void AVIFile::GetClipDrmInfo(void* ptr)
{
  MM_MSG_PRIO(MM_GENERAL, MM_PRIO_LOW, "AVIFile::GetClipDrmInfo");
  if(ptr)
  {
    ClipDrmInfoT* clip_drmInfo = (ClipDrmInfoT*)ptr;
    memset(clip_drmInfo,0,sizeof(ClipDrmInfoT));
    if(m_pClipDrmInfo)
    {
      memcpy(clip_drmInfo,(ClipDrmInfoT*)m_pClipDrmInfo,sizeof(ClipDrmInfoT) );
    }
    else
    {
      MM_MSG_PRIO(MM_GENERAL, MM_PRIO_LOW, "AVIFile::GetClipDrmInfo m_pClipDrmInfo is NULL");
    }
  }
}
#endif
/* ======================================================================
FUNCTION:
  AVIFile::ParseMetaData

DESCRIPTION:
  To begin parsing avi file and update media information such as
  total number of tracks, if drm is present etc.

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
bool AVIFile::ParseMetaData()
{
  bool nRet = false;
  CHUNK_t chunkType;
  avi_video_info videoInfo;
  avi_audiotrack_summary_info audioSummaryInfo;
  avi_uint8 track;
  aviErrorType retError;

#ifndef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
  bool bHttpStreaming = false;
#endif

  //check if we have already done with the initial parsing?
  if(m_playAudio && (m_nSelectedAudioStreamId >= 0) && _success && (_fileErrorCode == PARSER_ErrorNone))
  {
    return true;
  }
  if(m_playVideo && (m_nSelectedVideoStreamId >= 0) && _success && (_fileErrorCode == PARSER_ErrorNone))
  {
    return true;
  }

  if(!m_pAVIParser)
  {
    m_pAVIParser =
      MM_New_Args(aviParser,(this,m_fileSize,(m_playAudio)?false:true,(m_playVideo)?false:true, bHttpStreaming));
  }
  if(m_pAVIParser)
  {
    if( (retError = m_pAVIParser->StartParsing()) == AVI_SUCCESS)
    {
      #ifdef FEATURE_FILESOURCE_DIVX_DRM
      if(!initDivXDrmSystem())
      {
        //Appropriate error code is set in initDivXDrmSystem.
        _success = false;
        return false;
      }
      #else
       MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW,
                  "ParseMetaData->FEATURE_FILESOURCE_DIVX_DRM is not defined..");
      #endif
      _fileErrorCode = PARSER_ErrorNone;
      m_nNumStreams = m_pAVIParser->GetTotalNumberOfTracks();
      for(track = 0; track < m_nNumStreams; track++)
      {
        // Get the chunk type corresponding to track number.
        if(m_pAVIParser->GetTrackChunkType(track,&chunkType)!=AVI_SUCCESS)
        {
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH, "ParseMetaData::unknown chunk type");
        }

        switch(chunkType)
        {
          case AVI_CHUNK_AUDIO:
            // Get the audioInfo.
            if(m_pAVIParser->GetAudioTrackSummaryInfo(track,&audioSummaryInfo)==AVI_SUCCESS)
            {
              if( (m_playAudio)&&(m_nSelectedAudioStreamId < 0) )
              {
                //Only audio instance should select the audio track
                m_nSelectedAudioStreamId = track;
                _success = true;
              }
            }
            break;

          case AVI_CHUNK_VIDEO:
            // Get videoInfo,and initialize video.
            if(m_pAVIParser->GetVideoInfo(track,&videoInfo)!=AVI_SUCCESS)
            {
              break;
            }
            if( (m_playVideo)&&(m_nSelectedVideoStreamId < 0))
            {
              //Only video instance should select the video track.
              m_nSelectedVideoStreamId = track;
              _success = true;
            }
            break;
          case AVI_CHUNK_BITMAP_CAPTION:
            if( (m_playText)&&(m_nSelectedTextStreamId < 0))
            {
              m_nSelectedTextStreamId = track;
              _success = true;
            }
            break;
          default:
            break;
        }//switch(chunkType)
      }//for(track = 0; track < m_nNumStreams; track++)
#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
      if( (_fileErrorCode == PARSER_ErrorNone) && (bHttpStreaming) )
      {
        _success = true;
        //send parser ready event
        MM_MSG_PRIO(MM_GENERAL, MM_PRIO_LOW, "Common::PARSER_READY");
        sendParserEvent(PARSER_READY);
      }
#endif
    }//if(m_pAVIParser->StartParsing() == AVI_SUCCESS)
    else
    {
#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
      //When return error is not SUCCESS & streaming & currentOffset is more than downloaded offset but
      //within filesize
      if((bHttpStreaming) && ((m_pAVIParser->GetLastOffsetRead()) > m_wBufferOffset) &&
        ((m_pAVIParser->GetLastOffsetRead()) < m_fileSize) )
      {
        _success = false;
        retError = AVI_DATA_UNDERRUN;
        MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
                    "AVIFile:ParseMetaData aviParsing failed.. due to underrun retError %d",retError);
      }
      else
#endif
      {
        MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_FATAL,
                    "AVIFile:ParseMetaData aviParing failed..retError %d",retError);
      }
    }
  }//if(m_pAVIParser)

 if(_success)
 {
   nRet = true;
 }
 return nRet;
}
/* ======================================================================
FUNCTION:
  AVIFile::~AVIFile()

DESCRIPTION:
  destructor

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
AVIFile::~AVIFile()
{
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW, "AVIFile::~AVIFile");
  if (m_AviFilePtr != NULL)
  {
    OSCL_FileClose(m_AviFilePtr);
    m_AviFilePtr = NULL;
  }
  if (m_pAVIParser != NULL)
  {
    MM_Delete(m_pAVIParser);
    m_pAVIParser = NULL;
  }
  if(m_pAACAudioInfo)
  {
    MM_Free(m_pAACAudioInfo);
  }
#ifdef FEATURE_FILESOURCE_DIVX_DRM
  if(m_playVideo && drmContext)
  {
    drmFinalizePlayback(drmContext);
    MM_Free(drmContext);
    drmContext = NULL;
  }
  #ifndef FILESOURCE_LEGACY_DIVX_DRM
    #ifdef PLATFORM_LTK
      RELEASE_DECRYPT_BLOCK();
    #endif
  #endif
  if(m_pClipDrmInfo)
  {
    MM_Free(m_pClipDrmInfo);
    m_pClipDrmInfo = NULL;
  }
#endif
  for(uint8 index = 0; index<AVI_MAX_VIDEO_FRAMES_READ_AHEAD; index++)
  {
    if( m_avi_video_samples[index].buff != NULL)
    {
      MM_Free(m_avi_video_samples[index].buff);
      m_avi_video_samples[index].buff = NULL;
    }
  }
}

/* ======================================================================
FUNCTION:
  getNextMediaSample

DESCRIPTION:
  gets next sample of the given track.

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
PARSER_ERRORTYPE AVIFile::getNextMediaSample(uint32 id, uint8 *buf,
                                             uint32 *pulBufSize, uint32 &index)
{
  CHUNK_t Type;
  uint32 nOutDataSize = 0;
  PARSER_ERRORTYPE retError = PARSER_ErrorDefault;
  uint64 temp_time;
  uint8 noOfBVops = 0;
  uint32 width = 0;
  uint32 height = 0;
  uint8 readIndex = 0;
  uint8 noOfBVopsInSameChunk = 0;
  uint8 totalNoOfBVops = 0;

  /* Validate input params and class variables */
  if(NULL == pulBufSize || NULL == buf || 0 == *pulBufSize)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
               "getNextMediaSample invalid input params!!");
    return PARSER_ErrorInvalidParam;
  }

  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,"AVIFile::GetNextMediaSample");
  if( (m_pAVIParser->GetTrackChunkType(id,&Type) == AVI_SUCCESS) && (id < FILE_MAX_MEDIA_STREAMS) )
  {
    switch(Type)
    {
      case AVI_CHUNK_VIDEO:
        /* This piece of code is applicable to MPEG4 video only. in future if we
           need to add for H264 then we may have to remove this check.
        */
        if(m_bframesMayPresent)
        {
          /* We need to allocate this memory once for the playback. This is
             to read ahead to know whether the following frame is B-Frame or not.
          */
          if(m_videoFramesReadAhead.allocatedForReadAhead == FALSE)
          {
            width = getTrackVideoFrameWidth(id);
            height = getTrackVideoFrameHeight(id);
            for(readIndex = 0; readIndex<AVI_MAX_VIDEO_FRAMES_READ_AHEAD; readIndex++)
            {
              m_avi_video_samples[readIndex].buff = (uint8*)MM_Malloc((int)(width * height *1.5));
              if(m_avi_video_samples[readIndex].buff == NULL)
              {
                MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                                  "AVIFile::GetNextMediaSample memory allocation failed for size %lu",(width * height *2));
                return PARSER_ErrorMemAllocFail;
              }
            }
            m_videoFramesReadAhead.allocatedForReadAhead = TRUE;
          }
          /* If we are reading for the first time just read the frame */
          if(m_videoFramesReadAhead.firstVideoFrame == FALSE)
          {
            nOutDataSize = *pulBufSize;
            uint8 *tmpBuf = m_avi_video_samples[m_videoFramesReadAhead.currentSampleIndex].buff;
            retError = getNextAVIMediaSample(id, tmpBuf, &nOutDataSize, index);
            if(nOutDataSize > 0 && PARSER_ErrorNone == retError)
            {
              //Fill in sample information.
              m_avi_video_samples[m_videoFramesReadAhead.currentSampleIndex].m_sampleInfo.time = m_sampleInfo[id].time;
              m_avi_video_samples[m_videoFramesReadAhead.currentSampleIndex].m_sampleInfo.btimevalid = true;
              m_avi_video_samples[m_videoFramesReadAhead.currentSampleIndex].m_sampleInfo.delta = m_sampleInfo[id].delta;
              m_avi_video_samples[m_videoFramesReadAhead.currentSampleIndex].m_sampleInfo.sample = m_sampleInfo[id].sample;
              m_avi_video_samples[m_videoFramesReadAhead.currentSampleIndex].size = nOutDataSize;
              m_avi_video_samples[m_videoFramesReadAhead.currentSampleIndex].vop_type =
                                whichVop( tmpBuf, nOutDataSize, &noOfBVopsInSameChunk);
              if(
                (MPEG4_I_VOP == m_avi_video_samples[m_videoFramesReadAhead.validSampleIndex].vop_type)||
                (H264_I_VOP == m_avi_video_samples[m_videoFramesReadAhead.validSampleIndex].vop_type)
              )
              {
                m_avi_video_samples[m_videoFramesReadAhead.validSampleIndex].m_sampleInfo.sync = 1;
              }
              else
              {
                m_avi_video_samples[m_videoFramesReadAhead.validSampleIndex].m_sampleInfo.sync = 0;
              }
              m_avi_video_samples[m_videoFramesReadAhead.currentSampleIndex].bVopCount = noOfBVopsInSameChunk;
              MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_MEDIUM,"AVIFile::GetNextMediaSample vop type  %d  no. of Bvops  %d ",
                           m_avi_video_samples[m_videoFramesReadAhead.currentSampleIndex].vop_type,
                           noOfBVopsInSameChunk);
              MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_MEDIUM,
                              "AVIFile::GetNextMediaSample Reading first video frame VIDEO nTimeStamp %llu nOutDataSize %ld",
                            m_sampleInfo[id].time,nOutDataSize);
            }
            else
            {
              return retError;
            }
          }
          /* If we already read a frame from the file and that frame is a B-Frame then send that frame no need to read
             any frames ahead
             Here we already computed the time stamp of the B-frame
          */
          if(m_avi_video_samples[m_videoFramesReadAhead.currentSampleIndex].vop_type == MPEG4_B_VOP)
          {
            /* copy the frame into the supplied buffer */
            memcpy(buf,m_avi_video_samples[m_videoFramesReadAhead.currentSampleIndex].buff,
                   m_avi_video_samples[m_videoFramesReadAhead.currentSampleIndex].size);
            m_videoFramesReadAhead.currentSampleIndex = (m_videoFramesReadAhead.currentSampleIndex + 1) %
                                                        AVI_MAX_VIDEO_FRAMES_READ_AHEAD;
            MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_MEDIUM,
                            "AVIFile::GetNextMediaSample Video nTimeStamp this is B-Frame %llu nSize %lu",
                            m_avi_video_samples[(m_videoFramesReadAhead.currentSampleIndex +
                            (AVI_MAX_VIDEO_FRAMES_READ_AHEAD -1)) % AVI_MAX_VIDEO_FRAMES_READ_AHEAD].m_sampleInfo.time ,
                            m_avi_video_samples[(m_videoFramesReadAhead.currentSampleIndex +
                            (AVI_MAX_VIDEO_FRAMES_READ_AHEAD -1)) % AVI_MAX_VIDEO_FRAMES_READ_AHEAD].size);

            *pulBufSize = m_avi_video_samples[(m_videoFramesReadAhead.currentSampleIndex +
                      (AVI_MAX_VIDEO_FRAMES_READ_AHEAD -1)) % AVI_MAX_VIDEO_FRAMES_READ_AHEAD].size;
            return PARSER_ErrorNone;
          }

          /* Read the next frame */
          m_videoFramesReadAhead.validSampleIndex = (m_videoFramesReadAhead.validSampleIndex + 1) % AVI_MAX_VIDEO_FRAMES_READ_AHEAD;
          uint8 *tmpBufPtr = m_avi_video_samples[m_videoFramesReadAhead.validSampleIndex].buff;
          nOutDataSize = *pulBufSize;
          retError = getNextAVIMediaSample(id, tmpBufPtr, &nOutDataSize, index);
          if(nOutDataSize > 0)
          {
            //Fill in sample information.
            m_avi_video_samples[m_videoFramesReadAhead.validSampleIndex].m_sampleInfo.time = m_sampleInfo[id].time;
            m_avi_video_samples[m_videoFramesReadAhead.validSampleIndex].m_sampleInfo.btimevalid = true;
            m_avi_video_samples[m_videoFramesReadAhead.validSampleIndex].m_sampleInfo.delta = m_sampleInfo[id].delta;
            m_avi_video_samples[m_videoFramesReadAhead.validSampleIndex].m_sampleInfo.sample = m_sampleInfo[id].sample;
            m_avi_video_samples[m_videoFramesReadAhead.validSampleIndex].size = nOutDataSize;
            m_avi_video_samples[m_videoFramesReadAhead.validSampleIndex].vop_type =
                    whichVop( tmpBufPtr, nOutDataSize, &noOfBVopsInSameChunk);
            if(
                (MPEG4_I_VOP == m_avi_video_samples[m_videoFramesReadAhead.validSampleIndex].vop_type)||
                (H264_I_VOP == m_avi_video_samples[m_videoFramesReadAhead.validSampleIndex].vop_type)
              )
            {
              m_avi_video_samples[m_videoFramesReadAhead.validSampleIndex].m_sampleInfo.sync = 1;
            }
            else
            {
              m_avi_video_samples[m_videoFramesReadAhead.validSampleIndex].m_sampleInfo.sync = 0;
            }
            m_avi_video_samples[m_videoFramesReadAhead.validSampleIndex].bVopCount = noOfBVopsInSameChunk;
            MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_MEDIUM,"AVIFile::GetNextMediaSample vop type  %d  no. of Bvops  %d ", m_avi_video_samples[m_videoFramesReadAhead.validSampleIndex].vop_type, noOfBVopsInSameChunk);
          }
          else
          {
            // Failed to read so decrementing the write pointer by 1
            m_videoFramesReadAhead.validSampleIndex = (m_videoFramesReadAhead.validSampleIndex + (AVI_MAX_VIDEO_FRAMES_READ_AHEAD -1)) % AVI_MAX_VIDEO_FRAMES_READ_AHEAD;
            *pulBufSize = 0;
            return retError;
          }
          m_videoFramesReadAhead.firstVideoFrame = TRUE;
          /* After I or P see how many B-Frames are following this is required to compute the
             TS for the current I/P frame and the consecutive B-Frames */
          for(readIndex=0; readIndex<AVI_MAX_VIDEO_FRAMES_READ_AHEAD; readIndex++)
          {
            if(m_avi_video_samples[m_videoFramesReadAhead.validSampleIndex].vop_type == MPEG4_B_VOP)
            {
              /* Increment the B-VoP counter */
              ++noOfBVops;
              MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,"AVIFile::GetNextMediaSample consecutive B-Frame count %d",noOfBVops);
              m_videoFramesReadAhead.validSampleIndex = (m_videoFramesReadAhead.validSampleIndex + 1) % AVI_MAX_VIDEO_FRAMES_READ_AHEAD;
              nOutDataSize = *pulBufSize;
              avi_video_sample *pVideoSamplePtr = &m_avi_video_samples[m_videoFramesReadAhead.validSampleIndex];
              retError = getNextAVIMediaSample(id, pVideoSamplePtr->buff, &nOutDataSize, index);
              if(nOutDataSize > 0)
              {
                pVideoSamplePtr->m_sampleInfo.time = m_sampleInfo[id].time;
                pVideoSamplePtr->m_sampleInfo.btimevalid = true;
                pVideoSamplePtr->m_sampleInfo.delta = m_sampleInfo[id].delta;
                pVideoSamplePtr->m_sampleInfo.sample = m_sampleInfo[id].sample;
                pVideoSamplePtr->size = nOutDataSize;
                pVideoSamplePtr->vop_type = whichVop(pVideoSamplePtr->buff, nOutDataSize,
                                                     &noOfBVopsInSameChunk);
                if(
                    (MPEG4_I_VOP == pVideoSamplePtr->vop_type)||
                    (H264_I_VOP  == pVideoSamplePtr->vop_type)
                  )
                  {
                    pVideoSamplePtr->m_sampleInfo.sync = 1;
                  }
                  else
                  {
                    pVideoSamplePtr->m_sampleInfo.sync = 0;
                 }
                pVideoSamplePtr->bVopCount = noOfBVopsInSameChunk;
                MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_MEDIUM,"AVIFile::GetNextMediaSample vop type  %d  no. of Bvops  %d ",
                             pVideoSamplePtr->vop_type, noOfBVopsInSameChunk);
                continue;
              }
              else
              {
                // Failed to read so decrementing the write pointer by 1
                m_videoFramesReadAhead.validSampleIndex = (m_videoFramesReadAhead.validSampleIndex +
                                                           (AVI_MAX_VIDEO_FRAMES_READ_AHEAD -1)) % AVI_MAX_VIDEO_FRAMES_READ_AHEAD;
                *pulBufSize = nOutDataSize;
                return retError;
              }
            }
            else
            {
              break;
            }
          }
          // This is to take care of the B-Frames in the current access unit and following in a seperate access unit
          totalNoOfBVops = m_avi_video_samples[m_videoFramesReadAhead.currentSampleIndex].bVopCount + noOfBVops;
          /* If we encounter a clip where there are 4 or more consecutive B-Frames we don't support so
             we need to return with an error
          */
          if(totalNoOfBVops == 4)
          {
            MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,"AVIFile::GetNextMediaSample we don't support clips with more than 3 consecutive B-Frames  %d",totalNoOfBVops);
            return PARSER_ErrorEndOfFile;
          }
          switch (totalNoOfBVops)
          {
            case 0:
              // There are no B-Frames following this P/I frame so no need to compute any new TS
              memcpy(buf,m_avi_video_samples[m_videoFramesReadAhead.currentSampleIndex].buff,m_avi_video_samples[m_videoFramesReadAhead.currentSampleIndex].size);
              m_videoFramesReadAhead.currentSampleIndex = (m_videoFramesReadAhead.currentSampleIndex + 1) % AVI_MAX_VIDEO_FRAMES_READ_AHEAD;
              nOutDataSize = m_avi_video_samples[(m_videoFramesReadAhead.currentSampleIndex + (AVI_MAX_VIDEO_FRAMES_READ_AHEAD -1)) % AVI_MAX_VIDEO_FRAMES_READ_AHEAD].size;
              MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_MEDIUM,
                            "AVIFile::GetNextMediaSample Video nTimeStamp %llu nSize %lu",
                            m_avi_video_samples[(m_videoFramesReadAhead.currentSampleIndex + (AVI_MAX_VIDEO_FRAMES_READ_AHEAD -1)) % AVI_MAX_VIDEO_FRAMES_READ_AHEAD].m_sampleInfo.time, nOutDataSize);
              break;
            case 1:
              // one B-frame is following P/I frame
              memcpy(buf,m_avi_video_samples[m_videoFramesReadAhead.currentSampleIndex].buff,m_avi_video_samples[m_videoFramesReadAhead.currentSampleIndex].size);
              // Recalculate the new time stamps for P/I and B-Frame
              temp_time = m_avi_video_samples[m_videoFramesReadAhead.currentSampleIndex].m_sampleInfo.time;
              m_avi_video_samples[m_videoFramesReadAhead.currentSampleIndex].m_sampleInfo.time += m_avi_video_samples[m_videoFramesReadAhead.currentSampleIndex].m_sampleInfo.delta;
              // Only one B-Frame and it is following in a seperate access unit
              if(m_avi_video_samples[m_videoFramesReadAhead.currentSampleIndex].bVopCount == 0)
              {
              m_avi_video_samples[(m_videoFramesReadAhead.currentSampleIndex + 1) % AVI_MAX_VIDEO_FRAMES_READ_AHEAD].m_sampleInfo.time = temp_time;
              }
              //Increment the read index
              m_videoFramesReadAhead.currentSampleIndex = (m_videoFramesReadAhead.currentSampleIndex + 1) % AVI_MAX_VIDEO_FRAMES_READ_AHEAD;
              nOutDataSize = m_avi_video_samples[(m_videoFramesReadAhead.currentSampleIndex + (AVI_MAX_VIDEO_FRAMES_READ_AHEAD -1)) % AVI_MAX_VIDEO_FRAMES_READ_AHEAD].size;
              MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_MEDIUM,
                            "AVIFile::GetNextMediaSample Video nTimeStamp %llu nSize %lu",
                            m_avi_video_samples[(m_videoFramesReadAhead.currentSampleIndex + (AVI_MAX_VIDEO_FRAMES_READ_AHEAD -1)) % AVI_MAX_VIDEO_FRAMES_READ_AHEAD].m_sampleInfo.time, nOutDataSize);
              break;
            case 2:
              // two B-frames are following this current P/I frame
              memcpy(buf,m_avi_video_samples[m_videoFramesReadAhead.currentSampleIndex].buff,m_avi_video_samples[m_videoFramesReadAhead.currentSampleIndex].size);
              // Recalculate the new time stamps for P/I and the consecutive two B-Frame
              temp_time = m_avi_video_samples[m_videoFramesReadAhead.currentSampleIndex].m_sampleInfo.time;
              m_avi_video_samples[m_videoFramesReadAhead.currentSampleIndex].m_sampleInfo.time += m_avi_video_samples[m_videoFramesReadAhead.currentSampleIndex].m_sampleInfo.delta * totalNoOfBVops;
              // Two B-Frames and two are following in a seperate access units
              if(m_avi_video_samples[m_videoFramesReadAhead.currentSampleIndex].bVopCount == 0)
              {
              m_avi_video_samples[(m_videoFramesReadAhead.currentSampleIndex + 2) % AVI_MAX_VIDEO_FRAMES_READ_AHEAD].m_sampleInfo.time = m_avi_video_samples[(m_videoFramesReadAhead.currentSampleIndex + 1) % AVI_MAX_VIDEO_FRAMES_READ_AHEAD].m_sampleInfo.time;
              m_avi_video_samples[(m_videoFramesReadAhead.currentSampleIndex + 1) % AVI_MAX_VIDEO_FRAMES_READ_AHEAD].m_sampleInfo.time = temp_time;
              }
              //Increment the read index
              m_videoFramesReadAhead.currentSampleIndex = (m_videoFramesReadAhead.currentSampleIndex + 1) % AVI_MAX_VIDEO_FRAMES_READ_AHEAD;
              nOutDataSize = m_avi_video_samples[(m_videoFramesReadAhead.currentSampleIndex + (AVI_MAX_VIDEO_FRAMES_READ_AHEAD -1)) % AVI_MAX_VIDEO_FRAMES_READ_AHEAD].size;
              MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_MEDIUM,
                            "AVIFile::GetNextMediaSample Video nTimeStamp %llu nSize %lu",
                            m_avi_video_samples[(m_videoFramesReadAhead.currentSampleIndex + (AVI_MAX_VIDEO_FRAMES_READ_AHEAD -1)) % AVI_MAX_VIDEO_FRAMES_READ_AHEAD].m_sampleInfo.time, nOutDataSize);
              break;
            case 3:
              // two B-frames are following this current P/I frame
              memcpy(buf,m_avi_video_samples[m_videoFramesReadAhead.currentSampleIndex].buff,m_avi_video_samples[m_videoFramesReadAhead.currentSampleIndex].size);
              // Recalculate the new time stamps for P/I and the consecutive three B-Frame
              temp_time = m_avi_video_samples[m_videoFramesReadAhead.currentSampleIndex].m_sampleInfo.time;
              m_avi_video_samples[m_videoFramesReadAhead.currentSampleIndex].m_sampleInfo.time += m_avi_video_samples[m_videoFramesReadAhead.currentSampleIndex].m_sampleInfo.delta * totalNoOfBVops;
              // Three B-Frames and two are following in a seperate access units
              if(m_avi_video_samples[m_videoFramesReadAhead.currentSampleIndex].bVopCount == 0)
              {
              m_avi_video_samples[(m_videoFramesReadAhead.currentSampleIndex + 3) % AVI_MAX_VIDEO_FRAMES_READ_AHEAD].m_sampleInfo.time = m_avi_video_samples[(m_videoFramesReadAhead.currentSampleIndex + 2) % AVI_MAX_VIDEO_FRAMES_READ_AHEAD].m_sampleInfo.time;
              m_avi_video_samples[(m_videoFramesReadAhead.currentSampleIndex + 2) % AVI_MAX_VIDEO_FRAMES_READ_AHEAD].m_sampleInfo.time = m_avi_video_samples[(m_videoFramesReadAhead.currentSampleIndex + 1) % AVI_MAX_VIDEO_FRAMES_READ_AHEAD].m_sampleInfo.time;
              m_avi_video_samples[(m_videoFramesReadAhead.currentSampleIndex + 1) % AVI_MAX_VIDEO_FRAMES_READ_AHEAD].m_sampleInfo.time = temp_time;
              }
              // Three B-Frames are there and two are in the same access unit and one in a seperate access unit
              else if(m_avi_video_samples[m_videoFramesReadAhead.currentSampleIndex].bVopCount == 2)
              {
                m_avi_video_samples[(m_videoFramesReadAhead.currentSampleIndex + 1) % AVI_MAX_VIDEO_FRAMES_READ_AHEAD].m_sampleInfo.time = m_avi_video_samples[(m_videoFramesReadAhead.currentSampleIndex + 1) % AVI_MAX_VIDEO_FRAMES_READ_AHEAD].m_sampleInfo.time + m_avi_video_samples[(m_videoFramesReadAhead.currentSampleIndex + 1) % AVI_MAX_VIDEO_FRAMES_READ_AHEAD].m_sampleInfo.delta;
              }
              //Increment the read index
              m_videoFramesReadAhead.currentSampleIndex = (m_videoFramesReadAhead.currentSampleIndex + 1) % AVI_MAX_VIDEO_FRAMES_READ_AHEAD;
              nOutDataSize = m_avi_video_samples[(m_videoFramesReadAhead.currentSampleIndex + (AVI_MAX_VIDEO_FRAMES_READ_AHEAD -1)) % AVI_MAX_VIDEO_FRAMES_READ_AHEAD].size;
              MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_MEDIUM,
                            "AVIFile::GetNextMediaSample Video nTimeStamp %llu nSize %lu",
                            m_avi_video_samples[(m_videoFramesReadAhead.currentSampleIndex + (AVI_MAX_VIDEO_FRAMES_READ_AHEAD -1)) % AVI_MAX_VIDEO_FRAMES_READ_AHEAD].m_sampleInfo.time, nOutDataSize);
              break;
            default :
              break;
          }
        }
        else
        {
          //other than Mpeg4 & Divx we don't support B-Frames
          nOutDataSize = *pulBufSize;
          retError = getNextAVIMediaSample(id, buf, &nOutDataSize, index);
        }
        break;
      case AVI_CHUNK_AUDIO:
      {
        nOutDataSize = *pulBufSize;
        retError = getNextAVIMediaSample(id, buf, &nOutDataSize, index);
      }
      break;
      case AVI_CHUNK_BITMAP_CAPTION:
      {
        nOutDataSize = *pulBufSize;
        retError = getNextAVIMediaSample(id, buf, &nOutDataSize, index);
      }
      break;
      default:
        break;
    }
  }
  m_repositionProgress = false;
  *pulBufSize = nOutDataSize;
  if(nOutDataSize)
  {
    return PARSER_ErrorNone;
  }
  else
  {
    return PARSER_ErrorEndOfFile;
  }
}

/* ======================================================================
FUNCTION:
  AVIFile::getNextAVIMediaSample

DESCRIPTION:
  gets next sample of the given track.

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 size of sample

SIDE EFFECTS:
  None.
======================================================================*/
PARSER_ERRORTYPE AVIFile::getNextAVIMediaSample(uint32 id, uint8 *buf, uint32 *size, uint32 &index)
{
  avi_uint16 trackID;
  PARSER_ERRORTYPE retError = PARSER_ErrorDefault;
  aviErrorType retVal = AVI_SUCCESS;
  avi_int32 nOutDataSize = 0;
  uint8 noOfBVopsInSameChunk = 0;
  avi_sample_info minfo;
  memset(&minfo,0,sizeof(avi_sample_info));
  AVI_VOP_TYPE vopType = NO_VOP;

#ifdef FEATURE_FILESOURCE_DIVX_DRM
  bool bDecrypt = false;
#endif

  if(!m_pAVIParser)
  {
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_FATAL,
                  "AVIFile::getNextAVIMediaSample NULL PARSER HANDLE for track id = %lu", id);
    return PARSER_ErrorDefault;
  }

  while( (!nOutDataSize) && (retVal == AVI_SUCCESS) && (id < FILE_MAX_MEDIA_STREAMS) )
  {
    retVal = m_pAVIParser->GetNextSampleInfo(id,&minfo, *size,&trackID);

#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
    if(bHttpStreaming)
    {
      if((retVal == AVI_DATA_UNDERRUN) && (!m_bEndOfData))
      {
        sendHTTPStreamUnderrunEvent();
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                   "AVIFile::getNextMediaSample underrun");
        *size = 0;
        return PARSER_ErrorDataUnderRun;
      }
    }
    else
#endif

    if(  ( (trackID != id) ||
           ( (minfo.nSampleSize == 0) && ( (retVal == AVI_SUCCESS) || (retVal == AVI_INSUFFICIENT_BUFFER) ) ) ) &&
           (minfo.chunkType != AVI_CHUNK_DRM) &&
           (retVal != AVI_END_OF_FILE ) )
    {
      continue;
    }
    if( ((retVal == AVI_SUCCESS)||(retVal == AVI_INSUFFICIENT_BUFFER)) && ((trackID == id)||(minfo.chunkType == AVI_CHUNK_DRM)) )
    {
      if((minfo.nSampleSize > *size)||(retVal == AVI_INSUFFICIENT_BUFFER))
      {
        MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_ERROR,
                     "Error:AVIFile::getNextAVIMediaSample minfo.nSampleSize %d > available buffer size %lu",minfo.nSampleSize,*size);
        if (minfo.chunkType == AVI_CHUNK_AUDIO)
        {
          //we read multiple chunks once after seek is done
          //to locate correct frame boundary.No need to set m_audioLargestSize in this scenario.
          if(!m_repositionProgress)
          {
            m_audioLargestSize = minfo.nSampleSize;
            //Make sure size is even number
            if(m_audioLargestSize % 2)
            {
              m_audioLargestSize++;
            }
            avi_audio_info audioInfo;
            if(m_pAVIParser->GetAudioInfo(id,&audioInfo)==AVI_SUCCESS)
            {
              m_audioLargestSize = FILESOURCE_MAX((uint32)audioInfo.strhAudio.dwSuggestedBufferSize,
                                                  m_audioLargestSize);
            }
          }//if(!m_repositionProgress)
        }
        else if (minfo.chunkType == AVI_CHUNK_VIDEO)
        {
          m_videoLargestSize = minfo.nSampleSize;
          avi_video_info videoInfo;
          if(m_pAVIParser->GetVideoInfo(id,&videoInfo)==AVI_SUCCESS)
          {
            m_videoLargestSize = FILESOURCE_MAX((uint32)videoInfo.strhVideo.dwSuggestedBufferSize,
                                                minfo.nSampleSize);
          }
        }
        *size = 0;
        return PARSER_ErrorInsufficientBufSize;
      }
      else
      {
        if( (retVal = m_pAVIParser->GetCurrentSample(trackID,buf, *size,minfo.nSampleSize))==AVI_SUCCESS)
        {

          if(minfo.chunkType == AVI_CHUNK_DRM)
          {
#ifdef FEATURE_FILESOURCE_DIVX_DRM
            //Next chunk is encrypted.Record frame info and continue.
            if(drmContext && drmContextLength)
            {
  #ifndef FILESOURCE_LEGACY_DIVX_DRM
              m_pDRMExDDInfo = drmGetDDInfoWriteBufferEx(drmContext);
              if(m_pDRMExDDInfo)
              {
                memcpy(m_pDRMExDDInfo,buf,DIVX_DRM_FRAME_DRM_INFO_SIZE);
              }
  #endif
              memcpy(drm_frame_info,buf,DIVX_DRM_FRAME_DRM_INFO_SIZE);
              bDecrypt = true;
              continue;
            }
#else
            nOutDataSize = 0;
            retError = PARSER_ErrorEndOfFile;
            retVal = AVI_END_OF_FILE;
#endif
          }
          else
          {
            //Fill in sample information.
            m_sampleInfo[id].time = minfo.nTimeStamp;
            m_sampleInfo[id].btimevalid = true;
            m_sampleInfo[id].size = minfo.nSampleSize;
            vopType = whichVop( buf, minfo.nSampleSize, &noOfBVopsInSameChunk);
            if( (MPEG4_I_VOP == vopType)|| (H264_I_VOP == vopType) )
            {
              m_sampleInfo[id].sync = 1;
            }
            else
            {
              m_sampleInfo[id].sync = 0;
            }
            m_sampleInfo[id].delta = minfo.nDuration;
            m_sampleInfo[id].sample = m_sampleInfo[id].sample + 1;
            nOutDataSize = minfo.nSampleSize;
            retError = PARSER_ErrorNone;
            if(minfo.chunkType == AVI_CHUNK_AUDIO)
            {
              MM_MSG_PRIO3(MM_FILE_OPS, MM_PRIO_MEDIUM,
                            "AVIFile::GetNextAVIMediaSample AUDIO nTimeStamp %llu nDuration %u nSize %u",
                             minfo.nTimeStamp,minfo.nDuration,minfo.nSampleSize);
#ifdef FEATURE_FILESOURCE_DIVX_DRM
              if(bDecrypt)
              {
                drmErrorCodes_t result = drmDecryptAudio( drmContext, buf, minfo.nSampleSize);
                if ( DRM_SUCCESS!= result )
                {
                   MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
                            "AVIFile::GetNextAVIMediaSample AUDIO frame decryption failed..");
                   nOutDataSize = 0;
                   retError = PARSER_ErrorEndOfFile;
                }
                bDecrypt = false;
              }//if(bDecrypt)
#endif//#ifdef FEATURE_FILESOURCE_DIVX_DRM
            }//if(minfo.chunkType == AVI_CHUNK_AUDIO)
            else if(minfo.chunkType == AVI_CHUNK_VIDEO)
            {
              MM_MSG_PRIO3(MM_FILE_OPS, MM_PRIO_MEDIUM,
                            "AVIFile::GetNextAVIMediaSample VIDEO nTimeStamp %llu nDuration %u nSize %u",
                            minfo.nTimeStamp,minfo.nDuration,minfo.nSampleSize);

#ifdef FEATURE_FILESOURCE_DIVX_DRM
              if(bDecrypt)
              {
                drmErrorCodes_t result = DRM_GENERAL_ERROR;
    #ifndef FILESOURCE_LEGACY_DIVX_DRM
                uint32_t nbytesadded = 0;
                memset(DecryptJITBuffer,0xff,MAX_JIT_BUFFER_SIZE);
                if(getTrackOTIType(trackID) == H264_VIDEO)
                {
                  result = drmInsertBitstreamPayloadEx(drmContext,drmStreamTypeVideoH264,DecryptJITBuffer, &nbytesadded);
                }
                else
                {
                  result = drmInsertBitstreamPayloadEx(drmContext,drmStreamTypeVideoMPEG4ASP,DecryptJITBuffer, &nbytesadded);
                }
                if(result == DRM_SUCCESS)
                {
                  #ifdef PLATFORM_LTK
                    result = DRM_GENERAL_ERROR;
                  #endif
                  if((minfo.nSampleSize + nbytesadded) < *size)
                  {
                    //This is not efficient as we have to move the data for every encrypted frame.
                    //However, drmInsertBitstreamPayloadEx knocks off frame header, so decoder can't decode
                    //unless header is in place.
                    memmove(buf+nbytesadded,buf,minfo.nSampleSize);
                    memcpy(buf,DecryptJITBuffer,nbytesadded);
                    minfo.nSampleSize+=nbytesadded;
                    nOutDataSize+=nbytesadded;
                    retError = PARSER_ErrorNone;
                    #ifdef PLATFORM_LTK
                      result = DECRYPT_VIDEO_CHUNK(buf, minfo.nSampleSize);
                      //Third party WIN32 deocder can't handle JIT frame
                      memmove(buf,buf+nbytesadded,(minfo.nSampleSize-nbytesadded));
                    #endif
                  }
                }
    #else
                result = drmDecryptVideo( drmContext, buf, minfo.nSampleSize, drm_frame_info );
    #endif//#ifndef FILESOURCE_LEGACY_DIVX_DRM
                if ( DRM_SUCCESS!= result )
                {
                   MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
                            "AVIFile::GetNextAVIMediaSample VIDEO frame decryption failed..");
                   nOutDataSize = 0;
                   retError = PARSER_ErrorEndOfFile;
                   retVal = AVI_FAILURE;
                }
                bDecrypt = false;
              }//if(bDecrypt)
#endif//#ifdef FEATURE_FILESOURCE_DIVX_DRM
            }//else if(minfo.chunkType == AVI_CHUNK_VIDEO)
          }//end of else of if(minfo.chunkType == AVI_CHUNK_DRM)
        }//if(m_pAVIParser->GetCurrentSample...
      }
    }
    if(AVI_INSUFFICIENT_BUFFER == retVal)
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                 "AVIFile::getNextAVIMediaSample encountered INSUFFICIENT_DATA_BUFFER_SIZE");
      return PARSER_ErrorInsufficientBufSize;
    }
    else if(AVI_SUCCESS != retVal)
    {
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                 "AVIFile::getNextAVIMediaSample encountered error %d..", retVal);
      *size = 0;
      return PARSER_ErrorEndOfFile;
    }
  }
  *size = nOutDataSize;
  return retError;
}
/*===========================================================================

FUNCTION:
  whichVop

DESCRIPTION:
  This function tells the application whether a the bitstream passed contains
  a VOP and if yes then what kind of VOP is it - I/P/B or S VOP.

INPUT/OUTPUT PARAMETERS:
  pBitstream - pointer to raw bitstream.
  size - size of the bitstream.

RETURN VALUE:
   I_VOP: bitstream contains an I-VOP
   P_VOP: bitstream contains an P-VOP
   B_VOP: bitstream contains an B-VOP
   S_VOP: bitstream contains an S-VOP
   NO_VOP: bitstream contains no VOP.
SIDE EFFECTS:
  None.

===========================================================================*/
AVI_VOP_TYPE AVIFile::whichVop(uint8* pBitstream, int size, uint8 * noOfBVopsInSameChunk )
{
  int i;
  int j;
  int code = 0;
  int vopType;
  AVI_VOP_TYPE  returnVopType = NO_VOP;

  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW,"AVIFile::whichVop");
  *noOfBVopsInSameChunk = 0;
  for (i=0;i<size;++i)
  {
    code <<= 8;
    code |= (0x000000FF & (pBitstream[i]));
    {
      if(MPEG4_VOP_START_CODE == code)
      {
        if((i+1)<size)
        {
          vopType = 0x000000c0 & (pBitstream[i+1]);
          if(0x00000000 == vopType) returnVopType = MPEG4_I_VOP;
          else if(0x00000040 == vopType)  returnVopType = MPEG4_P_VOP;
          else if(0x00000080 == vopType)  returnVopType = MPEG4_B_VOP;
          else if(0x000000c0 == vopType)  returnVopType = MPEG4_S_VOP;
        break;
      }
    }
  }
  }
  if(returnVopType == MPEG4_P_VOP)
  {
    if((i+2)<size)
    {
      code = 0;
      for (j=i+2;j<size;++j)
      {
        code <<= 8;
        code |= (0x000000FF & (pBitstream[j]));
        {
          if(MPEG4_VOP_START_CODE == code)
          {
            vopType = 0x000000c0 & (pBitstream[j+1]);
            if(0x00000080 == vopType) (*noOfBVopsInSameChunk) = (*noOfBVopsInSameChunk) + 1;
          }
        }
      }
    }
  }
  return returnVopType;
}

/* ======================================================================
FUNCTION:
  AVIFile::getMediaTimestampForCurrentSample

DESCRIPTION:
  gets time stamp of current sample of the track.

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 time stamp in track time scale unit

SIDE EFFECTS:
  None.
======================================================================*/
uint64 AVIFile::getMediaTimestampForCurrentSample(uint32 id)
{
  CHUNK_t Type;
  uint64 nTimeStamp = 0;
  if(!m_pAVIParser)
  {
    return AVI_PARSE_ERROR;
}
  if( ( id < m_pAVIParser->GetTotalNumberOfTracks() ) && (id < FILE_MAX_MEDIA_STREAMS))
  {
    if(m_pAVIParser->GetTrackChunkType(id,&Type) == AVI_SUCCESS)
    {
      switch(Type)
      {
        case AVI_CHUNK_VIDEO:
          if(m_bframesMayPresent && (!m_repositionProgress))
          {
            nTimeStamp = m_avi_video_samples[(m_videoFramesReadAhead.currentSampleIndex + (AVI_MAX_VIDEO_FRAMES_READ_AHEAD -1)) % AVI_MAX_VIDEO_FRAMES_READ_AHEAD].m_sampleInfo.time;
          }
          else
          {
            nTimeStamp = m_sampleInfo[id].time;
          }
          m_repositionProgress = false;
          MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM, "AVIFile::getMediaTimestampForCurrentSample Video TS %llu",nTimeStamp);
          break;
        case AVI_CHUNK_AUDIO:
          nTimeStamp = m_sampleInfo[id].time;
          MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM, "AVIFile::getMediaTimestampForCurrentSample Audio TS %llu",nTimeStamp);
          break;
        default:
          break;
      }
    }
  }
  return nTimeStamp;
}
/* ======================================================================
FUNCTION:
  AVIFile::skipNSyncSamples

DESCRIPTION:
  Skips specified sync samples in forward or backward direction.

INPUT/OUTPUT PARAMETERS:


RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
uint64 AVIFile::skipNSyncSamples(int nSyncSamplesToSkip,
                                 uint32 id,
                                 bool *bError,
                                 uint64 currentPosTimeStamp)
{
  avi_sample_info sinfo;
  memset(&sinfo,0,sizeof(avi_sample_info));

  //Do quick sanity check
  if(!bError)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL, "AVIFile::skipNSyncSamples bError is NULL");
    return 0;
  }
  if(nSyncSamplesToSkip == 0)
  {
    //??? To avoid unnecessary Seek, just return 0 and set bError to TRUE.
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
                 "AVIFile::skipNSyncSamples nSyncSamplesToSkip is 0");
    *bError = true;
    return 0;
  }
  //Now seek in parser

  if(  (!m_pAVIParser)                                     ||
       (m_pAVIParser->Seek((avi_uint8)id,
                           (avi_uint64)0,
                           currentPosTimeStamp,
                           &sinfo,false,
                           nSyncSamplesToSkip)!=AVI_SUCCESS) )
  {
    *bError = true;
    MM_MSG_PRIO3(MM_FILE_OPS,
                  MM_PRIO_FATAL,
                  "AVIFile::skipNSyncSamples failed for trackid %ld nSyncSamplesToSkip %d current TS %llu",
                  id,nSyncSamplesToSkip,currentPosTimeStamp);
    return 0;
  }
  *bError = false;
  MM_MSG_PRIO4(MM_FILE_OPS,
                MM_PRIO_MEDIUM,
                "AVIFile::skipNSyncSamples trackid %ld nSyncSamplesToSkip %d current TS %llu TS returned %llu",
                id,nSyncSamplesToSkip,currentPosTimeStamp,sinfo.nTimeStamp);

  m_sampleInfo[id].time = sinfo.nTimeStamp;
  m_sampleInfo[id].size = sinfo.nSampleSize;
  if(m_bframesMayPresent)
  {
    m_sampleInfo[id].sync = m_avi_video_samples[m_videoFramesReadAhead.currentSampleIndex].vop_type;
  }
  else
  {
    m_sampleInfo[id].sync = sinfo.bSync;
  }
  m_sampleInfo[id].delta = sinfo.nDuration;
  return m_sampleInfo[id].time;
}
/* ======================================================================
FUNCTION:
  AVIFile::resetPlayback

DESCRIPTION:
  resets the playback time/sample to zero.

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
void AVIFile::resetPlayback()
{}

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
uint64 AVIFile::resetPlayback(  uint64 repos_time, uint32 id, bool bSetToSyncSample,
                                bool *bError, uint64 currentPosTimeStamp)
{
  avi_sample_info sinfo;
  memset(&sinfo,0,sizeof(avi_sample_info));
  if(!m_pAVIParser)
  {
    *bError = true;
    _fileErrorCode = PARSER_ErrorSeekFail;
    MM_MSG_PRIO2(MM_FILE_OPS,
                  MM_PRIO_FATAL,
                  "AVIFile::resetPlayback reposition failed for trackid %lu repos_time %llu ",
                  id,repos_time);
    return 0;
  }
  if(m_pAVIParser->Seek((avi_uint8)id,
                        repos_time,
                        currentPosTimeStamp,
                        &sinfo,bSetToSyncSample,0)!=AVI_SUCCESS)
  {
    *bError = true;
    _fileErrorCode = PARSER_ErrorSeekFail;
    MM_MSG_PRIO2(MM_FILE_OPS,
                  MM_PRIO_FATAL,
                  "AVIFile::resetPlayback reposition failed for trackid %lu repos_time %llu ",
                  id,repos_time);
    return 0;
  }
  *bError = false;
  _fileErrorCode = PARSER_ErrorNone;
  MM_MSG_PRIO3(MM_FILE_OPS,
                MM_PRIO_MEDIUM,
                "AVIFile::resetPlayback trackid %lu repos_time %llu TS returned %llu",
                id,repos_time,sinfo.nTimeStamp);

  m_sampleInfo[id].time = sinfo.nTimeStamp;
  m_sampleInfo[id].size = sinfo.nSampleSize;
  if(m_bframesMayPresent)
  {
    m_sampleInfo[id].sync = m_avi_video_samples[m_videoFramesReadAhead.currentSampleIndex].vop_type;
  }
  else
  {
    m_sampleInfo[id].sync = sinfo.bSync;
  }
  m_sampleInfo[id].delta = sinfo.nDuration;

  //After seek we need to reset these variables to fill the ahead buffer with fresh data.
  m_videoFramesReadAhead.firstVideoFrame = FALSE;
  m_videoFramesReadAhead.currentSampleIndex = 0;
  m_videoFramesReadAhead.validSampleIndex = 0;
  m_repositionProgress = TRUE;

  return m_sampleInfo[id].time;
}
/* ======================================================================
FUNCTION:
  AVIFile::resetPlayback

DESCRIPTION:
  resets the playback time to zero for a track.

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
bool AVIFile::resetMediaPlayback(uint32)
{
  return 0;
}

/* ======================================================================
FUNCTION:
  AVIFile::randomAccessDenied

DESCRIPTION:
  gets if repositioning is allowed or not.

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
uint8 AVIFile::randomAccessDenied()
{
  uint8 nRet = false;
  if(m_pAVIParser)
  {
    nRet = m_pAVIParser->randomAccessDenied();
  }
  return nRet;
}
/* ======================================================================
FUNCTION:
  AVIFile::GetClipMetaData

DESCRIPTION:
  Provides different metadata fields info in the o/p buffer

INPUT/OUTPUT PARAMETERS:
  pucDataBuf and pulDatabufLen.

RETURN VALUE:
 PARSER_ErrorNone if Success
 Error status, if failure

SIDE EFFECTS:
  None.
======================================================================*/
PARSER_ERRORTYPE AVIFile::GetClipMetaData(wchar_t *pucDataBuf,
                                          uint32 *pulDatabufLen,
                                          FileSourceMetaDataType ienumData)
{
  PARSER_ERRORTYPE retError = PARSER_ErrorDefault;
  if(pulDatabufLen == NULL || m_pAVIParser == NULL)
    return PARSER_ErrorInvalidParam;

  switch(ienumData)
  {
    case FILE_SOURCE_MD_TITLE:
      retError = m_pAVIParser->getDataFromInfoChunk("INAM",pucDataBuf,(uint16*)pulDatabufLen);
      break;
    case FILE_SOURCE_MD_DESCRIPTION:
      retError = m_pAVIParser->getDataFromInfoChunk("ICMT",pucDataBuf,(uint16*)pulDatabufLen);
      break;
    case FILE_SOURCE_MD_RATING:
      retError = m_pAVIParser->getDataFromInfoChunk("IRTD",pucDataBuf,(uint16*)pulDatabufLen);
      break;
    case FILE_SOURCE_MD_COPYRIGHT:
      retError = m_pAVIParser->getDataFromInfoChunk("ICOP",pucDataBuf,(uint16*)pulDatabufLen);
      break;
    case FILE_SOURCE_MD_CREATION_DATE:
      retError = m_pAVIParser->getDataFromInfoChunk("ICRD",pucDataBuf,(uint16*)pulDatabufLen);
      break;
    case FILE_SOURCE_MD_ARTIST:
      retError = m_pAVIParser->getDataFromInfoChunk("IART",pucDataBuf,(uint16*)pulDatabufLen);
      break;
    case FILE_SOURCE_MD_SOFTWARE:
      retError = m_pAVIParser->getDataFromInfoChunk("ISFT",pucDataBuf,(uint16*)pulDatabufLen);
      break;
    default:
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW, "AVIFile::GetClipMetaData Not Implemented");
      break;
  }
  return retError;
}
/* ======================================================================
FUNCTION:
  AVIFile::getMovieDuration

DESCRIPTION:
  gets movie duration in movie timescale unit.

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
uint64 AVIFile::getMovieDuration() const
{
  avi_uint64 nDuration = 0;
  if(!m_pAVIParser)
  {
    return nDuration;
  }
  nDuration = m_pAVIParser->GetClipDurationInMsec();
  return nDuration;
}

/* ======================================================================
FUNCTION:
  AVIFile::getMovieTimescale

DESCRIPTION:
  gets movie timescale

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
uint32 AVIFile::getMovieTimescale() const
{
  uint32 nTimeScale = AVI_STREAM_TIME_SCALE;
  if(!m_pAVIParser)
  {
    return 0;
  }
  /*
  *If clip has video and is currently selcted for playback,
  *return video time scale.
  */
  //if(m_pAVIParser->m_hAviSummary.n_video_tracks && (m_nSelectedVideoStreamId >=0))
  //{
  //  return m_pAVIParser->m_hAviSummary.video_info[m_nSelectedVideoStreamId].frame_period_divisor;
  //}
  //else if(m_pAVIParser->m_hAviSummary.n_audio_tracks && (m_nSelectedAudioStreamId >=0))
  //{
  //  //If audio is selcted for playback, check if isVBR is TRUE/FALSE;
  //  if( (m_nSelectedAudioStreamId)&&
  //                                  (m_pAVIParser->m_hAviSummary.audio_info[m_nSelectedAudioStreamId].strfAudio.nBlockAlign == 1152 ||
  //                                  (m_pAVIParser->m_hAviSummary.audio_info[m_nSelectedAudioStreamId].strfAudio.nBlockAlign == 576))
  //    )
  //  {
  //    //VBR audio, timescale calculation is different than non VBR audio
  //    return m_pAVIParser->m_hAviSummary.audio_info[m_nSelectedAudioStreamId].strfAudio.nSamplesPerSec;
  //  }
  //  else
  //  {
  //    //Non VBR audio
  //    return m_pAVIParser->m_hAviSummary.audio_info[m_nSelectedAudioStreamId].strhAudio.dwRate;
  //  }
  //}
  return nTimeScale;
}

/* ======================================================================
FUNCTION:
  AVIFile::getMovieDurationMsec

DESCRIPTION:
  gets movie duration in milli secs

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
uint64 AVIFile::getMovieDurationMsec() const
{
  avi_uint64 nDuration = 0;
  if(!m_pAVIParser)
  {
    return nDuration;
  }
  nDuration = m_pAVIParser->GetClipDurationInMsec();
  return nDuration;
}
/* ======================================================================
FUNCTION:
  AVIFile::GetMaximumBitRateForTrack(uint32 trackId)

DESCRIPTION:
  Returns the max bitrate for the track identified by trackId

INPUT/OUTPUT PARAMETERS:
  TrackId

RETURN VALUE:
 Max Data Bit Rate

SIDE EFFECTS:
  None.
======================================================================*/

uint32 AVIFile::GetMaximumBitRateForTrack(uint32 /*trackId*/)
{
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW, "AVIFile::GetMaximumBitRateForTrack Not Implemented");
  return 0;
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
uint32 AVIFile::getTrackWholeIDList(uint32 *ids)
{

  if(!m_pAVIParser)
  {
    return 0;
  }
  return ((avi_uint32)m_pAVIParser->GetTrackWholeIDList((avi_uint32*)ids));
}

/* ======================================================================
FUNCTION:
  AVIFile::getTrackContentVersion

DESCRIPTION:
  gets content version number

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
int16 AVIFile::getTrackContentVersion(uint32 id)
{
  return 0;
}

/* ======================================================================
FUNCTION:
  AVIFile::trackRandomAccessDenied

DESCRIPTION:
  gets repositioning permission for the track.

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
uint8 AVIFile::trackRandomAccessDenied(uint32 /*id*/)
{
  uint8 nRet = FALSE; /* by default random access is allowed */
  return nRet;
}
/* ======================================================================
FUNCTION:
  AVIFile::getTrackVideoFrameRate

DESCRIPTION:
  gets track video (if video) frame rate

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
float AVIFile::getTrackVideoFrameRate(uint32 id)
{
  avi_video_info videoInfo;

  if(!m_pAVIParser)
  {
    return 0.0;
  }
  if(m_pAVIParser->GetVideoInfo(id,&videoInfo)==AVI_SUCCESS)
  {
    if(videoInfo.strhVideo.dwScale)
    {

      return( (float)videoInfo.strhVideo.dwRate /(float)videoInfo.strhVideo.dwScale);
      //return ((float)videoInfo.strhVideo.dwRate/1000);
    }
  }
  return 0;
}

/* ======================================================================
FUNCTION:
  AVIFile::getTrackVideoFrameWidth

DESCRIPTION:
  returns video track's frame width.

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
uint32 AVIFile::getTrackVideoFrameWidth(uint32 /*id*/)
{
  //avi_video_info videoInfo;
  avi_mainheader_avih avih;

  if(!m_pAVIParser)
  {
    return 0;
  }
  if(m_pAVIParser->GetAVIHeader(&avih)==AVI_SUCCESS)
  {
    return avih.dwWidth;
  }
  /*
  if(m_pAVIParser->GetVideoInfo(id,&videoInfo)==AVI_SUCCESS)
  {
    return videoInfo.strfVideo.biWidth;
  }
  */
  return 0;
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
uint32 AVIFile::getTrackVideoFrameHeight(uint32 /*id*/)
{
  //avi_video_info videoInfo;
  avi_mainheader_avih avih;

  if(!m_pAVIParser)
  {
    return 0;
  }
  if(m_pAVIParser->GetAVIHeader(&avih)==AVI_SUCCESS)
  {
    return avih.dwHeight;
  }
  /*
  if(m_pAVIParser->GetVideoInfo(id,&videoInfo)==AVI_SUCCESS)
  {
    return videoInfo.strfVideo.biHeight;
  }
  */
  return 0;
}

/* ======================================================================
FUNCTION:
  AVIFile::getTrackMediaDuration

DESCRIPTION:
  gets track duration in track time scale unit

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
uint64 AVIFile::getTrackMediaDuration(uint32 id)
{
  if(!m_pAVIParser)
  {
    return 0;
  }
  return m_pAVIParser->GetTrackDuration((avi_uint8)id);
}

/* ======================================================================
FUNCTION:
  AVIFile::getTrackMediaTimescale

DESCRIPTION:
  gets track time scale

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
uint32 AVIFile::getTrackMediaTimescale(uint32 id)
{
  //avi_audiotrack_summary_info audioSummaryInfo;
  //avi_video_info videoInfo;
  uint32 timeScale = AVI_STREAM_TIME_SCALE;
  //if(!m_pAVIParser)
  //{
  //  return timeScale;
  //}
  //
  //switch(m_pAVIParser->m_hAviSummary.stream_index[id].type)
  //{
  //  case AVI_CHUNK_AUDIO:
  //    if(m_pAVIParser->GetAudioTrackSummaryInfo(id,&audioSummaryInfo)==AVI_SUCCESS)
  //    {
  //      //If audio is selcted for playback, check if isVBR is TRUE/FALSE;
  //      if( (m_nSelectedAudioStreamId >=0)&& audioInfo.isVbr)
  //      {
  //        //VBR audio, timescale calculation is different than non VBR audio
  //        timeScale = m_pAVIParser->m_hAviSummary.audio_info[m_nSelectedAudioStreamId].strfAudio.nSamplesPerSec;
  //      }
  //      else
  //      {
  //        //Non VBR audio
  //        timeScale = m_pAVIParser->m_hAviSummary.audio_info[m_nSelectedAudioStreamId].strhAudio.dwRate;
  //      }
  //    }
  //  break;

  //  case AVI_CHUNK_VIDEO:
  //    if(m_pAVIParser->m_hAviSummary.n_video_tracks && (m_nSelectedVideoStreamId >=0))
  //    {
  //      timeScale = m_pAVIParser->m_hAviSummary.video_info[m_nSelectedVideoStreamId].frame_period_divisor;
  //    }
  //    break;

  //  default:
  //   break;
  //}
  return timeScale;
}

/* ======================================================================
FUNCTION:
  AVIFile::getTrackAudioSamplingFreq

DESCRIPTION:
  gets audio track's sampling frequency

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
uint32 AVIFile::getTrackAudioSamplingFreq(uint32 id)
{
  avi_audiotrack_summary_info audioSummaryInfo;
  uint32 nSamplingFreq = 0;
  CHUNK_t cType;
  if(!m_pAVIParser)
  {
    return nSamplingFreq;
  }
  if(m_pAVIParser->GetTrackChunkType(id,&cType)==AVI_SUCCESS)
  {
    switch(cType)
    {
      case AVI_CHUNK_AUDIO:
        if(m_pAVIParser->GetAudioTrackSummaryInfo(id,&audioSummaryInfo)==AVI_SUCCESS)
        {
          nSamplingFreq = audioSummaryInfo.audioFrequency;
        }
        break;
      default:
       break;
    }
  }
  return nSamplingFreq;
}
/* ======================================================================
FUNCTION:
  AVIFile::getAudioFrameDuration

DESCRIPTION:
  Returns audio frame duration.

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
long  AVIFile::getAudioFrameDuration(int trackid)
{
  long frame_duration = 0;
  avi_audiotrack_summary_info ainfo;
  avi_audio_info audtrackInfo;
  avi_audio_info aInfo ;
  CHUNK_t cType;

  if(!m_pAVIParser)
  {
    return frame_duration;
  }
  if(m_pAVIParser->GetTrackChunkType((avi_uint8)trackid,&cType)==AVI_SUCCESS)
  {
    switch(cType)
    {
      case AVI_CHUNK_AUDIO:
        if( (m_pAVIParser->GetAudioInfo((avi_uint8)trackid,&audtrackInfo)==AVI_SUCCESS) &&
            (m_pAVIParser->GetAudioTrackSummaryInfo((avi_uint8)trackid,&ainfo)==AVI_SUCCESS))
        {
          if(!ainfo.isVbr && (audtrackInfo.strhAudio.dwRate))
          {
            float framerate =  ( (float)audtrackInfo.strhAudio.dwScale /
                                             (float)audtrackInfo.strhAudio.dwRate );
            framerate *= 1000; //Convert to milliseconds.
            frame_duration = (long)framerate;
          }
          else if((audtrackInfo.strfAudio.nSamplesPerSec) && ainfo.isVbr && audtrackInfo.strhAudio.dwRate)
          {
            double framerate = ((double)audtrackInfo.strhAudio.dwScale/audtrackInfo.strhAudio.dwRate)* 1000.f;
            framerate *= 1000; //Convert to milliseconds.
            frame_duration = (long)framerate;
          }
        }
        if(m_pAVIParser->GetAudioInfo(trackid,&aInfo)==AVI_SUCCESS)
        {
          if((aInfo.strfAudio.wFormatTag == AVI_AUDIO_AC3) ||
             (aInfo.strfAudio.wFormatTag == AVI_AUDIO_PCM))
          {
            frame_duration = (long) m_sampleInfo[trackid].delta;
            MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM, "frame_duration for AC3 or PCM is %ld",frame_duration);
          }
        }
        break;
        default:
        break;
     }
  }
  return frame_duration;
}
#ifndef AVI_PARSER_FAST_START_UP
/* ======================================================================
FUNCTION:
  AVIFile::SetIDX1Cache

DESCRIPTION:
  Sets IDX1 cache in AVIParser

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
void AVIFile::SetIDX1Cache(void* ptr)
{
  if(m_pAVIParser)
  {
    m_pAVIParser->SetIDX1Cache(ptr);
  }
}
/* ======================================================================
FUNCTION:
  AVIFile::GetIDX1Cache

DESCRIPTION:
  Gets IDX1 cache from AVIParser

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
void* AVIFile::GetIDX1Cache()
{
  if(m_pAVIParser)
  {
    return m_pAVIParser->GetIDX1Cache();
  }
  return NULL;
}
#endif
/* ======================================================================
FUNCTION:
  AVIFile::getAudioSamplesPerFrame

DESCRIPTION:
  gets audio track's number of samples per frame.

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
uint32 AVIFile::getAudioSamplesPerFrame(uint32 id)
{
  uint32 dwSamplesPerFrame = 0;
  avi_audiotrack_summary_info audioSummaryInfo;
  CHUNK_t cType;

  if(!m_pAVIParser)
  {
    return dwSamplesPerFrame;
  }
  if(m_pAVIParser->GetTrackChunkType(id,&cType)==AVI_SUCCESS)
  {
    switch(cType)
    {
      case AVI_CHUNK_AUDIO:
        if(m_pAVIParser->GetAudioTrackSummaryInfo(id,&audioSummaryInfo)==AVI_SUCCESS)
        {
          if(audioSummaryInfo.audioFrequency <= 16000)
          {
            dwSamplesPerFrame = 1152;
          }
          else if(audioSummaryInfo.audioFrequency <= 32000 )
          {
            dwSamplesPerFrame = 1152;
          }
          else if(audioSummaryInfo.audioFrequency > 32000 )
          {
            dwSamplesPerFrame = 1152;
          }
        }
        break;
        default:
        break;
     }
  }
  return dwSamplesPerFrame;
}

/* ======================================================================
FUNCTION:
  AVIFile::GetAudioBitsPerSample

DESCRIPTION:
  returns bits per sample

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
uint32 AVIFile::GetAudioBitsPerSample(int id)
{
  avi_audiotrack_summary_info audioSummaryInfo;
  uint32 nbitsPerAudioSample = 0;
  CHUNK_t cType;
  if(m_pAVIParser->GetTrackChunkType(id,&cType)==AVI_SUCCESS)
  {
    switch(cType)
    {
      case AVI_CHUNK_AUDIO:
        if(m_pAVIParser->GetAudioTrackSummaryInfo(id,&audioSummaryInfo)==AVI_SUCCESS)
        {
          nbitsPerAudioSample = audioSummaryInfo.nbitsPerAudioSample;
        }
        break;

      default:
       break;
    }
  }
  return nbitsPerAudioSample;
}

/* ======================================================================
FUNCTION:
  AVIFile::GetNumAudioChannels

DESCRIPTION:
  returns number of audio channels

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
uint32 AVIFile::GetNumAudioChannels(int id)
{
  uint32 nChannels = 0;
  avi_audiotrack_summary_info audioSummaryInfo;
  CHUNK_t cType;

  if(m_pAVIParser->GetTrackChunkType(id,&cType)==AVI_SUCCESS)
  {
    switch(cType)
    {
      case AVI_CHUNK_AUDIO:
        if(m_pAVIParser->GetAudioTrackSummaryInfo(id,&audioSummaryInfo)==AVI_SUCCESS)
          nChannels = audioSummaryInfo.nChannels;
        break;

      case AVI_CHUNK_VIDEO:
       break;
      default:
        break;
    }
  }
  return nChannels;
}
/* ======================================================================
FUNCTION:
  AVIFile::GetAudioEncoderOptions

DESCRIPTION:
  returns the encoder options.

INPUT/OUTPUT PARAMETERS:
  id - track id.

RETURN VALUE:
  nEncodeOpt of type uint32

SIDE EFFECTS:
  None.
======================================================================*/
uint32 AVIFile::GetAudioEncoderOptions(int id)
{
  uint32 nEncodeOpt = 0;
  uint8 audioCodec = getTrackOTIType(id);
  if(m_pAVIParser &&
     ((audioCodec == WM_AUDIO) || (audioCodec == WM_PRO_AUDIO) ))
  {
    avi_header_strf_wma_extra wmaInfo;
    if(m_pAVIParser->GetWMAExtraInfo(id,&wmaInfo) == AVI_SUCCESS)
    {
      nEncodeOpt = (uint32)(wmaInfo.nEncodeOpt);
    }
  }
  else
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW, "AVIFile::GetAudioEncoderOptions Not Implemented");
  }
  return nEncodeOpt;
}
/* ======================================================================
FUNCTION:
  AVIFile::GetAudioAdvancedEncodeOptions

DESCRIPTION:
  returns the advanced encoder options.

INPUT/OUTPUT PARAMETERS:
  id - track id.

RETURN VALUE:
  nAdvancedEncodeOpt of type uint16

SIDE EFFECTS:
  None.
======================================================================*/
uint16 AVIFile::GetAudioAdvancedEncodeOptions(int id)
{
  uint16 nAdvancedEncodeOpt = 0;
  uint8 audioCodec = getTrackOTIType(id);
  if(m_pAVIParser &&
     ((audioCodec == WM_AUDIO) || (audioCodec == WM_PRO_AUDIO) ))
  {
    avi_header_strf_wma_extra wmaInfo;
    if(m_pAVIParser->GetWMAExtraInfo(id,&wmaInfo) == AVI_SUCCESS)
    {
      nAdvancedEncodeOpt = wmaInfo.nAdvancedEncodeOpt;
    }
  }
  else
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW, "AVIFile::GetAudioAdvancedEncodeOptions Not Implemented");
  }
  return nAdvancedEncodeOpt;
}
/* ======================================================================
FUNCTION:
  AVIFile::GetAudioAdvancedEncodeOptions2

DESCRIPTION:
  returns the advanced encoder options 2.

INPUT/OUTPUT PARAMETERS:
  id - track id.

RETURN VALUE:
  nAdvancedEncodeOpt2 of type uint32

SIDE EFFECTS:
  None.
======================================================================*/
uint32 AVIFile::GetAudioAdvancedEncodeOptions2(int id)
{
  uint32 nAdvancedEncodeOpt2 = 0;
  uint8 audioCodec = getTrackOTIType(id);
  if(m_pAVIParser &&
     ((audioCodec == WM_AUDIO) || (audioCodec == WM_PRO_AUDIO) ))
  {
    avi_header_strf_wma_extra wmaInfo;
    if(m_pAVIParser->GetWMAExtraInfo(id,&wmaInfo) == AVI_SUCCESS)
    {
      nAdvancedEncodeOpt2 =  wmaInfo.nAdvancedEncodeOpt2;
    }
  }
  else
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW, "AVIFile::GetAudioAdvancedEncodeOptions Not Implemented");
  }
  return nAdvancedEncodeOpt2;
}
/* ======================================================================
FUNCTION:
  AVIFile::GetAudioChannelMask

DESCRIPTION:
  returns the Advanced encoder options.

INPUT/OUTPUT PARAMETERS:
  id - track id.

RETURN VALUE:
  dwChannelMask of type uint32

SIDE EFFECTS:
  None.
======================================================================*/
uint32 AVIFile::GetAudioChannelMask(int id)
{
  uint32 dwChannelMask = 0;
  uint8 audioCodec = getTrackOTIType(id);
  if(m_pAVIParser &&
     ((audioCodec == WM_AUDIO) || (audioCodec == WM_PRO_AUDIO) ))
  {
    avi_header_strf_wma_extra wmaInfo;
    if(m_pAVIParser->GetWMAExtraInfo(id,&wmaInfo) == AVI_SUCCESS)
    {
      dwChannelMask = wmaInfo.dwChannelMask;
    }
  }
  else
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW, "AVIFile::GetAudioChannelMask Not Implemented");
  }
  return dwChannelMask;
}
/* ======================================================================
FUNCTION:
  AVIFile::GetFormatTag

DESCRIPTION:
  returns the format tag.

INPUT/OUTPUT PARAMETERS:
  id - trackid to get format tag for.

RETURN VALUE:
  wFormatTag of type uint16

SIDE EFFECTS:
  None.
======================================================================*/
uint16 AVIFile::GetFormatTag(int id)
{
  avi_audio_info aInfo ;
  if ( m_pAVIParser && (m_pAVIParser->GetAudioInfo(id,&aInfo)==AVI_SUCCESS))
    return aInfo.strfAudio.wFormatTag;
  else
    return 0;
}

/* ======================================================================
FUNCTION:
  AVIFile::GetBlockAlign

DESCRIPTION:
  returns the blockalign.

INPUT/OUTPUT PARAMETERS:
  id - track id to get blockalign for.

RETURN VALUE:
  nBlockAlign of type uint32

SIDE EFFECTS:
  None.
======================================================================*/
uint32 AVIFile::GetBlockAlign(int id)
{
  avi_audio_info aInfo ;
  if ( m_pAVIParser && (m_pAVIParser->GetAudioInfo(id,&aInfo)==AVI_SUCCESS))
    return aInfo.strfAudio.nBlockAlign;
  else
    return 0;
}
/* ======================================================================
FUNCTION:
  AVIFile::GetAudioVirtualPacketSize

DESCRIPTION:
  returns virtual packet size

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
uint16 AVIFile::GetAudioVirtualPacketSize(int id)
{
  avi_audio_info aInfo ;
  if ( m_pAVIParser && (m_pAVIParser->GetAudioInfo(id,&aInfo)==AVI_SUCCESS))
    return aInfo.strhAudio.dwSampleSize;
  else
    return 0;
}
/* ======================================================================
FUNCTION:
  AVIFile::GetFixedAsfAudioPacketSize

DESCRIPTION:
  returns the size of Audio ASF packet.

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
uint32 AVIFile::GetFixedAsfAudioPacketSize(int id)
{
  avi_audio_info aInfo ;
  if ( m_pAVIParser && (m_pAVIParser->GetAudioInfo(id,&aInfo)==AVI_SUCCESS))
    return aInfo.strhAudio.dwSuggestedBufferSize;
  else
    return 0;
}
/* ======================================================================
FUNCTION:
  AVIFile::peekCurSample

DESCRIPTION:
  gets information about current sample of a track

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
PARSER_ERRORTYPE AVIFile::peekCurSample(uint32 trackid, file_sample_info_type *pSampleInfo)
{
  CHUNK_t Type;
  if(!m_pAVIParser)
  {
    return PARSER_ErrorDefault;
  }
  if( ( trackid < m_pAVIParser->GetTotalNumberOfTracks() ) && (trackid < FILE_MAX_MEDIA_STREAMS))
  {
    if(m_pAVIParser->GetTrackChunkType(trackid,&Type) == AVI_SUCCESS)
    {
      switch(Type)
      {
        case AVI_CHUNK_VIDEO:
          if(m_bframesMayPresent)
          {
            *pSampleInfo = m_avi_video_samples[(m_videoFramesReadAhead.currentSampleIndex + (AVI_MAX_VIDEO_FRAMES_READ_AHEAD -1)) % AVI_MAX_VIDEO_FRAMES_READ_AHEAD].m_sampleInfo;
          }
          else
          {
            *pSampleInfo = m_sampleInfo[trackid];
          }
          break;
        case AVI_CHUNK_AUDIO:
          *pSampleInfo = m_sampleInfo[trackid];
          break;
        default:
          break;
      }
    }
    return PARSER_ErrorNone;
  }
  return PARSER_ErrorDefault;
}

/* ======================================================================
FUNCTION:
  AVIFile::getTrackOTIType

DESCRIPTION:
  gets track OTI value

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
uint8 AVIFile::getTrackOTIType(uint32 id)
{
  uint8 format = 0xFF;
  CHUNK_t cType;
  avi_audio_info aInfo;
  avi_video_info vInfo;

  if(m_pAVIParser)
  {
    if(m_pAVIParser->GetTrackChunkType(id,&cType)==AVI_SUCCESS)
    {
      switch(cType)
      {
        case AVI_CHUNK_AUDIO:
          if(m_pAVIParser->GetAudioInfo(id,&aInfo)==AVI_SUCCESS)
          {
            switch(aInfo.strfAudio.wFormatTag)
            {
              case AVI_AUDIO_DTS:
                format = (uint8)DTS_AUDIO;
                break;
              case AVI_AUDIO_MP3:
                format = (uint8)MP3_AUDIO;
                break;
              case AVI_AUDIO_AC3:
                format = (uint8)AC3_AUDIO;
                break;
              case AVI_AUDIO_PCM:
                format = PCM_AUDIO;
                break;
              case AVI_AUDIO_MP2:
                 format = (uint8)MP2_AUDIO;
                break;
              case AVI_AUDIO_AAC:
              case AVI_AUDIO_AAC_FAAD:
                if(aInfo.strfAudio.cbSize) //Extradata is present only for non-ADTS AAC clips
                {
                  format = (uint8)MPEG4_AUDIO;
                }
                else
                {
                  format = (uint8)AAC_ADTS_AUDIO;
                }
                break;
              case AVI_AUDIO_WMA_STD:
                format = (uint8)WM_AUDIO;
                break;
              case AVI_AUDIO_WMA_PRO:
                format = (uint8)WM_PRO_AUDIO;
                break;
              default:
                break;
            }
          }
          break;
        case AVI_CHUNK_VIDEO:
          if(m_pAVIParser->GetVideoInfo(id,&vInfo)==AVI_SUCCESS)
          {
            if(
                (!memcmp(&vInfo.strhVideo.fccHandler,"xvid",sizeof(fourCC_t)))    ||
                (!memcmp(&vInfo.strhVideo.fccHandler,"XVID",sizeof(fourCC_t)))    ||
                (!memcmp(&vInfo.strhVideo.fccHandler,"FMP4",sizeof(fourCC_t)))    ||
                (!memcmp(&vInfo.strhVideo.fccHandler,"fmp4",sizeof(fourCC_t)))    ||
                (!memcmp(&vInfo.strhVideo.fccHandler,"MP4V",sizeof(fourCC_t)))    ||
                (!memcmp(&vInfo.strhVideo.fccHandler,"mp4v",sizeof(fourCC_t)))    ||
                (!memcmp(&vInfo.strhVideo.fccHandler,"YV12",sizeof(fourCC_t)))    ||
                (!memcmp(&vInfo.strfVideo.biCompression,"xvid",sizeof(fourCC_t))) ||
                (!memcmp(&vInfo.strfVideo.biCompression,"XVID",sizeof(fourCC_t))) ||
                (!memcmp(&vInfo.strfVideo.biCompression,"FMP4",sizeof(fourCC_t))) ||
                (!memcmp(&vInfo.strfVideo.biCompression,"fmp4",sizeof(fourCC_t))) ||
                (!memcmp(&vInfo.strfVideo.biCompression,"MP4V",sizeof(fourCC_t))) ||
                (!memcmp(&vInfo.strfVideo.biCompression,"mp4v",sizeof(fourCC_t))) ||
                (!memcmp(&vInfo.strfVideo.biCompression,"YV12",sizeof(fourCC_t))) ||
                (!memcmp(&vInfo.strhVideo.fccHandler,"yv12",sizeof(fourCC_t)))
              )
              {
                format = (uint8)MPEG4_VIDEO;
#ifdef AVI_PARSER_B_FRAMES_TS_MANIPULATION
                m_bframesMayPresent = TRUE;
#endif
              }
            else if(
                     (!memcmp(&vInfo.strhVideo.fccHandler,"mpg4",sizeof(fourCC_t)))    ||
                     (!memcmp(&vInfo.strfVideo.biCompression,"mpg4",sizeof(fourCC_t))) ||
                     (!memcmp(&vInfo.strfVideo.biCompression,"MPEG4",sizeof(fourCC_t)+1))||
                     (!memcmp(&vInfo.strhVideo.fccHandler,"MPEG4",sizeof(fourCC_t)+1))
                   )
            {
              format = (uint8)NONSTD_MPEG4_VIDEO;
#ifdef AVI_PARSER_B_FRAMES_TS_MANIPULATION
              m_bframesMayPresent = TRUE;
#endif
            }
#ifdef FEATURE_FILESOURCE_AVI_DIVX_PARSER
            else if( (!memcmp(&vInfo.strhVideo.fccHandler,"divx",sizeof(fourCC_t))) ||
                     (!memcmp(&vInfo.strhVideo.fccHandler,"DIVX",sizeof(fourCC_t)))
                   )
            {
              format = (uint8)DIVX40_VIDEO;
#ifdef AVI_PARSER_B_FRAMES_TS_MANIPULATION
              m_bframesMayPresent = TRUE;
#endif
            }
            else if( (!memcmp(&vInfo.strhVideo.fccHandler,"dx50",sizeof(fourCC_t))) ||
                     (!memcmp(&vInfo.strhVideo.fccHandler,"DX50",sizeof(fourCC_t)))
                   )
            {
              format = (uint8)DIVX50_60_VIDEO;
#ifdef AVI_PARSER_B_FRAMES_TS_MANIPULATION
              m_bframesMayPresent = TRUE;
#endif
            }
#endif
              else if( (!memcmp(&vInfo.strhVideo.fccHandler,"h263",sizeof(fourCC_t)))   ||
                       (!memcmp(&vInfo.strfVideo.biCompression,"h263",sizeof(fourCC_t)))||
                       (!memcmp(&vInfo.strfVideo.biCompression,"H263",sizeof(fourCC_t)))||
                       (!memcmp(&vInfo.strhVideo.fccHandler,"H263",sizeof(fourCC_t)))
                     )
              {
                format = (uint8)H263_VIDEO;
              }
              else if( (!memcmp(&vInfo.strhVideo.fccHandler,"h264",sizeof(fourCC_t))) ||
                       (!memcmp(&vInfo.strfVideo.biCompression,"h264",sizeof(fourCC_t)))||
                       (!memcmp(&vInfo.strfVideo.biCompression,"H264",sizeof(fourCC_t)))||
                       (!memcmp(&vInfo.strfVideo.biCompression,"avc1",sizeof(fourCC_t)))||
                       (!memcmp(&vInfo.strfVideo.biCompression,"AVC1",sizeof(fourCC_t)))||
                       (!memcmp(&vInfo.strfVideo.biCompression,"X264",sizeof(fourCC_t)))||
                       (!memcmp(&vInfo.strhVideo.fccHandler,"H264",sizeof(fourCC_t)))
                     )
              {
                format = (uint8)H264_VIDEO;
              }
              else if( (!memcmp(&vInfo.strhVideo.fccHandler,"VP6F",sizeof(fourCC_t)))    ||
                       (!memcmp(&vInfo.strfVideo.biCompression,"VP6F",sizeof(fourCC_t))) ||
                       (!memcmp(&vInfo.strfVideo.biCompression,"vp6f",sizeof(fourCC_t))) ||
                       (!memcmp(&vInfo.strhVideo.fccHandler,"vp6f",sizeof(fourCC_t)))
                     )
              {
                format = (uint8)VP6F_VIDEO;
              }
             else if( (!memcmp(&vInfo.strhVideo.fccHandler,"FLV1",sizeof(fourCC_t))) ||
                       (!memcmp(&vInfo.strfVideo.biCompression,"FLV1",sizeof(fourCC_t))) ||
                       (!memcmp(&vInfo.strfVideo.biCompression,"flv1",sizeof(fourCC_t))) ||
                       (!memcmp(&vInfo.strhVideo.fccHandler,"flv1",sizeof(fourCC_t)))
                     )
              {
                 format = (uint8)SPARK_VIDEO;
              }
             else if( (!memcmp(&vInfo.strhVideo.fccHandler,"WMV2",sizeof(fourCC_t))) ||
                       (!memcmp(&vInfo.strfVideo.biCompression,"wmv2",sizeof(fourCC_t)))
                    )
             {
               format = (uint8) WM_VIDEO_8;
             }
             else if( (!memcmp(&vInfo.strhVideo.fccHandler,"WMV3",sizeof(fourCC_t))) ||
                      (!memcmp(&vInfo.strhVideo.fccHandler,"wmv3",sizeof(fourCC_t))) ||
                      (!memcmp(&vInfo.strfVideo.biCompression,"WMV3",sizeof(fourCC_t))) ||
                      (!memcmp(&vInfo.strfVideo.biCompression,"wmv3",sizeof(fourCC_t)))
                    )
             {
               format = (uint8) WM_VIDEO_9;
             }
             else if(  (!memcmp(&vInfo.strhVideo.fccHandler,"WVC1",sizeof(fourCC_t))) ||
                       (!memcmp(&vInfo.strhVideo.fccHandler,"wvc1",sizeof(fourCC_t)))
                    )
             {
               format = (uint8) VC1_VIDEO;
             }
             else if(  (!memcmp(&vInfo.strhVideo.fccHandler,"mpg2",sizeof(fourCC_t))) ||
                       (!memcmp(&vInfo.strhVideo.fccHandler,"MPG2",sizeof(fourCC_t))) ||
                       (!memcmp(&vInfo.strhVideo.fccHandler,"MPEG",sizeof(fourCC_t)))
                    )
             {
               format = (uint8) MPEG2_VIDEO;
             }
             else if(  (!memcmp(&vInfo.strhVideo.fccHandler,"mpg1",sizeof(fourCC_t))) ||
                       (!memcmp(&vInfo.strhVideo.fccHandler,"MPG1",sizeof(fourCC_t)))
                    )
             {
               format = (uint8) MPEG1_VIDEO;
             }

#ifdef FEATURE_DIVX_311_ENABLE
              else if(
                       (!memcmp(&vInfo.strhVideo.fccHandler,"div3",sizeof(fourCC_t))) ||
                       (!memcmp(&vInfo.strhVideo.fccHandler,"DIV3",sizeof(fourCC_t))) ||
                       (!memcmp(&vInfo.strhVideo.fccHandler,"DIV4",sizeof(fourCC_t))) ||
                       (!memcmp(&vInfo.strhVideo.fccHandler,"div4",sizeof(fourCC_t)))
                     )
              {
                format = (uint8)DIVX311_VIDEO;
              }
#endif
             else if( (!memcmp(&vInfo.strhVideo.fccHandler,"mjpg",sizeof(fourCC_t))) ||
                      (!memcmp(&vInfo.strhVideo.fccHandler,"MJPG",sizeof(fourCC_t))) ||
                      (!memcmp(&vInfo.strfVideo.biCompression,"mjpg",sizeof(fourCC_t))) ||
                      (!memcmp(&vInfo.strfVideo.biCompression,"MJPG",sizeof(fourCC_t)))
                    )
             {
               format = (uint8) MJPEG_VIDEO;
             }
          }
          break;
        case AVI_CHUNK_BITMAP_CAPTION:
          format = (uint8) AVI_BITMAP_TEXT;
          break;
        case AVI_CHUNK_TEXT_CAPTION:
          format = (uint8) AVI_SIMPLE_TEXT;
          break;
        default:
          break;
      }
    }
  }
  return format;
}

/* ======================================================================
FUNCTION:
  AVIFile::getTrackAudioFormat

DESCRIPTION:
  gets track audio format based on VideoFMT enum

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
uint8 AVIFile::getTrackAudioFormat(uint32 id)
{
  uint8 format = 0xFF;
  avi_audiotrack_summary_info audioSummaryInfo;
  CHUNK_t cType;

  if(m_pAVIParser)
  {
    if(m_pAVIParser->GetTrackChunkType(id,&cType)==AVI_SUCCESS)
    {
      switch(cType)
      {
        case AVI_CHUNK_AUDIO:
          if(m_pAVIParser->GetAudioTrackSummaryInfo(id,&audioSummaryInfo)==AVI_SUCCESS)
            format = (avi_uint8)audioSummaryInfo.wFormatTag;
        break;

        default:
        break;
      }
    }
  }
  return format;
}

/* ======================================================================
FUNCTION:
AVIFile::getLargestFrameSize

DESCRIPTION:
  gets the largest frame size in the given track
INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
uint32 AVIFile::getLargestFrameSize(uint32 /*id*/)
{
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW, "AVIFile::getLargestFrameSize Not Implemented");
  return 0;  /* not valid for WMV/WMA CODEC */
}

/* ======================================================================
FUNCTION:
  AVIFile::getFramesPerSample

DESCRIPTION:
  gets the number frames per sample of given track in video_fmt_stream_audio_subtype type.

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
uint8 AVIFile::getFramesPerSample(uint32 /*id*/)
{
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW, "AVIFile::getFramesPerSample Not Implemented");
  return 0x00;
}

/* ======================================================================
FUNCTION:
  AVIFile::getTotalNumberOfFrames

DESCRIPTION:
  gets total number of frames in a given track.

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
uint16 AVIFile::getTotalNumberOfFrames(uint32 /*id*/)
{
  avi_mainheader_avih aviHdr;
  if(!m_pAVIParser || (m_pAVIParser->GetAVIHeader(&aviHdr)!=AVI_SUCCESS) )
  {
    return 0x00;
  }
  return aviHdr.dwTotalFrames;
}


/* ======================================================================
FUNCTION:
  AVIFile::getTrackMaxBufferSizeDB

DESCRIPTION:
  gets maximum buffer size to play the track

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
int32 AVIFile::getTrackMaxBufferSizeDB(uint32 id)
{
  avi_audio_info audioInfo;
  avi_video_info videoInfo;
  CHUNK_t cType;
  int32 bufferSize = AVI_DEFAULT_AUDIO_BUF_SIZE;

  // Reset the required values
  videoInfo.strhVideo.dwSuggestedBufferSize = 0;
  audioInfo.strhAudio.dwSuggestedBufferSize = 0;
  if(!m_pAVIParser)
  {
    return 0;
  }
  if(m_pAVIParser->GetTrackChunkType(id,&cType)==AVI_SUCCESS)
  {
    switch(cType)
    {
      case AVI_CHUNK_AUDIO:
        if(m_audioLargestSize)
        {
          bufferSize = (int32)m_audioLargestSize;
        }
        else
        {
          if(m_pAVIParser->GetAudioInfo(id,&audioInfo)==AVI_SUCCESS)
          {
            bufferSize = audioInfo.strhAudio.dwSuggestedBufferSize;
            if( audioInfo.strhAudio.dwSuggestedBufferSize < audioInfo.strfAudio.nAvgBytesPerSec)
            {
              bufferSize = audioInfo.strfAudio.nAvgBytesPerSec;
            }
            if (bufferSize == 0)
            {
               bufferSize = AVI_DEFAULT_AUDIO_BUF_SIZE;
            }
            if (bufferSize % 2)
            {
              // must align to 16-bit boundary
              bufferSize++;
            }
          }
          else
          {
            bufferSize = AVI_DEFAULT_AUDIO_BUF_SIZE;
          }
        }
        break;

      case AVI_CHUNK_VIDEO:
        if (m_videoLargestSize)
        {
          bufferSize = (int32)m_videoLargestSize;
        }
        else
        {
          uint32 width = getTrackVideoFrameWidth(id);
          uint32 height = getTrackVideoFrameHeight(id);
          uint32 nsize = 0;
#ifdef FEATURE_FILESOURCE_DIVX_DRM
#ifndef  FILESOURCE_LEGACY_DIVX_DRM
          if(getTrackOTIType(id) == H264_VIDEO)
          {
            nsize = drmGetMaxBitstreamPayloadLengthEx(drmStreamTypeVideoH264);
          }
          else
          {
            nsize = drmGetMaxBitstreamPayloadLengthEx(drmStreamTypeVideoMPEG4ASP);
          }
#endif
#endif
          /* Allocate height * width * 1.2 directly for video since most
             platforms don't allow dynamic allocation to realloc when size is
             insufficient. Reducing parser input buffer size by 2 as i/p buffer
             size becoming greater than then YUV frame size for 720p/1080p  */
          bufferSize = (int32)FILESOURCE_MAX((width * height * 1.5)/2,nsize);

          (void)m_pAVIParser->GetVideoInfo(id,&videoInfo);

          /* If Calculated buffer size is less than suggested buffer size,
             then use Suggested buffer size value */
          if(bufferSize < videoInfo.strhVideo.dwSuggestedBufferSize)
          {
            bufferSize = videoInfo.strhVideo.dwSuggestedBufferSize;
          }
        }
        break;

      default:
        bufferSize = AVI_DEFAULT_AUDIO_BUF_SIZE;
        break;
    }
  }
  return bufferSize;
}

/* ======================================================================
FUNCTION:
  AVIFile::getTrackAverageBitrate

DESCRIPTION:
  gets track's average bitrate

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 average bitrate

SIDE EFFECTS:
  None.
======================================================================*/
int32 AVIFile::getTrackAverageBitrate(uint32 id)
{
  int32 nBitRate = 0;
  avi_audiotrack_summary_info audioSummaryInfo;
  CHUNK_t cType;

  if(!m_pAVIParser)
  {
    return nBitRate;
  }

  if(m_pAVIParser->GetTrackChunkType(id,&cType)==AVI_SUCCESS)
  {
    switch(cType)
    {
      case AVI_CHUNK_AUDIO:
        if(m_pAVIParser->GetAudioTrackSummaryInfo(id,&audioSummaryInfo)==AVI_SUCCESS)
        {
          nBitRate = audioSummaryInfo.audioBytesPerSec * 8;
        }
        break;

        case AVI_CHUNK_VIDEO:
          avi_mainheader_avih avih;
          if(m_pAVIParser->GetAVIHeader(&avih)==AVI_SUCCESS)
          {
            //Since AVI format has no direct way of getting clip video bitrate, returning dwMaxByesPerSec
            nBitRate = avih.dwMaxBytesPerSec;
          }
          break;
      default:
        break;
    }
  }
  return nBitRate;
}

/* ======================================================================
FUNCTION:
  AVIFile::getTrackMaxBitrate

DESCRIPTION:
  gets track's max bitrate

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
int32 AVIFile::getTrackMaxBitrate(uint32 id)
{
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW, "AVIFile::getTrackMaxBitRate Not Implemented");
  return 0;//getTrackAverageBitrate(id);
}
/* ======================================================================
 FUNCTION:
   AVIFile::GetTotalNumberOfAudioStreams()

 DESCRIPTION:
   returns total number of audio streams.

 INPUT/OUTPUT PARAMETERS:
   None.

 RETURN VALUE:
  none

 SIDE EFFECTS:
   None.
 ======================================================================*/
 int AVIFile::GetTotalNumberOfAudioStreams()
 {
   if(m_pAVIParser == NULL)
     return 0;
   return m_pAVIParser->GetTotalNumberOfAudioTracks();
 }
 /* ======================================================================
 FUNCTION:
   AVIFile::GetTotalNumberOfVideoStreams()

 DESCRIPTION:
   returns total number of video streams.

 INPUT/OUTPUT PARAMETERS:
   None.

 RETURN VALUE:
  none

 SIDE EFFECTS:
   None.
 ======================================================================*/
 int AVIFile::GetTotalNumberOfVideoStreams()
 {
   if(m_pAVIParser == NULL)
     return 0;
   return m_pAVIParser->GetTotalNumberOfVideoTracks();
 }
 /* ======================================================================
 FUNCTION:
   AVIFile::GetTotalNumberOfTextStreams()

 DESCRIPTION:
   returns total number of text streams.

 INPUT/OUTPUT PARAMETERS:
   None.

 RETURN VALUE:
  none

 SIDE EFFECTS:
   None.
 ======================================================================*/
 int AVIFile::GetTotalNumberOfTextStreams()
 {
  return 0;
 }

/* ======================================================================
FUNCTION:
  AVIFile::getAllowAudioOnly

DESCRIPTION:
  gets if audio only playback is allowed.

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
uint8 AVIFile::getAllowAudioOnly()
{
  return 1; /* by default allow */
}

/* ======================================================================
FUNCTION:
  AVIFile::getAllowVideoOnly

DESCRIPTION:
  gets if video only playback is allowed.

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
uint8 AVIFile::getAllowVideoOnly()
{
  return 1; /* by default allow */
}
/* ======================================================================
FUNCTION:
  AVIFile::getTrackDecoderSpecificInfoContent

DESCRIPTION:
  this returns Sequence (VOL) Header/size for given track id

INPUT/OUTPUT PARAMETERS:
  @in id: Track identifier
  @in buf: Buffer to copy VOL header
  @in/@out: pbufSize: Size of VOL header
  When buf is NULL, function returns the size of VOL header into pbufSize.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
PARSER_ERRORTYPE AVIFile::getTrackDecoderSpecificInfoContent(uint32 id, uint8* buf, uint32 *pbufSize)
{
  CHUNK_t cType;
  PARSER_ERRORTYPE errorCode = PARSER_ErrorDefault;
  if( (!m_pAVIParser) || (!pbufSize) )
  {
    return errorCode;
  }
 if(m_pAVIParser->GetTrackChunkType(id,&cType)==AVI_SUCCESS)
 {
   switch(cType)
   {
     case AVI_CHUNK_VIDEO:
       {
         uint8 vCodec = getTrackOTIType(id);
         if( (vCodec == MPEG4_VIDEO) ||
             (vCodec == NONSTD_MPEG4_VIDEO)||
             (vCodec == DIVX40_VIDEO)||
             (vCodec == DIVX50_60_VIDEO) ||
             (vCodec == WM_VIDEO_8) ||
             (vCodec == WM_VIDEO_9) )
         {
           avi_uint8* vol = m_pAVIParser->GetAVIVolHeader(id);
           avi_uint32 volsize =  m_pAVIParser->GetAVIVolHeaderSize(id);
           if(buf && (*pbufSize >= volsize) && volsize)
           {
             memcpy(buf,vol,volsize);
             *pbufSize = volsize;
             errorCode = PARSER_ErrorNone;
           }
           else if(volsize > 0)
           {
             *pbufSize = volsize;
             errorCode = PARSER_ErrorNone;
           }
         }
       }
       break;
     case AVI_CHUNK_AUDIO:
       {
         avi_audiotrack_summary_info audioSummaryInfo;
         avi_audio_info aInfo ;
         if ( (m_pAVIParser->GetAudioTrackSummaryInfo(id,&audioSummaryInfo)==AVI_SUCCESS) &&
              (m_pAVIParser->GetAudioInfo(id,&aInfo)==AVI_SUCCESS) &&
              ((aInfo.strfAudio.wFormatTag == AVI_AUDIO_AAC) ||(aInfo.strfAudio.wFormatTag == AVI_AUDIO_AAC_FAAD) ))
           {
              uint32 index =0;
              bool bError = false;
              //! If Buffer pointer is NULL or size is less than 8bytes
              if (NULL == buf || *pbufSize < AAC_ADTS_MAX_HEADER_SIZE)
              {
                *pbufSize = AAC_ADTS_MAX_HEADER_SIZE;
                errorCode = PARSER_ErrorNone;
              }
              else if(aInfo.strfAudio.cbSize)
              {
                if(pbufSize)
                {
                  *pbufSize = aInfo.strfAudio.cbSize;
                  errorCode = PARSER_ErrorNone;

                  if(buf)
                  {
                    memcpy(buf,aInfo.strfAudio.extra,aInfo.strfAudio.cbSize);
                    m_pAACAudioInfo = (avi_aac_info*)MM_Malloc(sizeof(avi_aac_info));

                    if(m_pAACAudioInfo)
                    {
                      m_pAACAudioInfo->samplingFreq = aInfo.strfAudio.nSamplesPerSec;
                      m_pAACAudioInfo->channelConfig = (avi_uint8)aInfo.strfAudio.nChannels;
                      m_pAACAudioInfo->audioObject = ( (buf [0] >> 6) & 0x03)+ 1;
                      m_pAACAudioInfo->subFormat = 3; //RAW
                    }
                    else
                    {
                      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL, "Failed to allocate memory for m_pAACAudioInfo");
                      errorCode = PARSER_ErrorMemAllocFail;
                    }
                  }
                }
                else
                {
                  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL, "pbufSize is NULL");
                  errorCode = PARSER_ErrorMemAllocFail;
                }
              }
              else
              {
                if(!m_pAACAudioInfo)
                {
                  uint8* firstSampleBuf = (uint8*)MM_Malloc(AAC_CONFIG_DATA_BUFFER_SIZE);
                  if(firstSampleBuf)
                  {
                    /*
                    *It is possible that parser might have been configured to
                    *output single audio frame and strip audio header after
                    *it has reported OPEN_COMPLETE.
                    *In such scenario, we won't be able to prepare AAC configuration
                    *header as STRIP header would have already removed what we need.
                    *
                    *We need to query output mode first.
                    *If it is configured to strip header, we need to change it to
                    *retain header and read first frame and retore strip header aagain.
                    *
                    *If not configured to strip header, we can just read frame directly.
                    */
                    bool bset = false;
                    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM, "Checking Audio Output configuration..");
                    (void)GetAudioOutputMode(&bset,FILE_SOURCE_MEDIA_OUTPUT_SINGLE_AUDIO_FRAME);
                    if(bset)
                    {
                      //configured to output single frame/buffer
                      bset = false;
                      (void)GetAudioOutputMode(&bset,FILE_SOURCE_MEDIA_STRIP_AUDIO_HEADER);
                      if(bset)
                      {
                        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM, "Changing Audio Output configuration..");
                        //configured to strip header
                        //change the configuration...
                        SetAudioOutputMode(FILE_SOURCE_MEDIA_OUTPUT_BYTES_STREAM);
                        SetAudioOutputMode(FILE_SOURCE_MEDIA_RETAIN_AUDIO_HEADER);
                      }
                    }
                    uint32 nret = AAC_CONFIG_DATA_BUFFER_SIZE;
                    errorCode = getNextMediaSample(id, firstSampleBuf, &nret, index);
                    //! Do Seek to ZERO after reading first media sample. Return
                    //! status check is not important, as Parser simply resets
                    (void)resetPlayback(0, id, true, &bError, 0);
                    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM, "nret after reading first sample %ld",nret);
                    if(bset)
                    {
                      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM, "Restoring Audio Output configuration..");
                      //restore the configuration...
                      SetAudioOutputMode(FILE_SOURCE_MEDIA_OUTPUT_SINGLE_AUDIO_FRAME);
                      SetAudioOutputMode(FILE_SOURCE_MEDIA_STRIP_AUDIO_HEADER);
                    }
                    m_pAACAudioInfo = (avi_aac_info*)MM_Malloc(sizeof(avi_aac_info));
                    if(m_pAACAudioInfo && (nret > 0) )
                    {
                      //make sure aac is ADTS and we have at least AVI_AAC_ADTS_HDR_LEN worth of bytes
                      if( (nret > AVI_AAC_ADTS_HDR_LEN) &&
                          ((firstSampleBuf[0] == 0xFF) && ((firstSampleBuf[1] & 0xF0) == 0xF0))
                        )
                      {
                        m_pAACAudioInfo->audioObject = ( (firstSampleBuf [2] >> 6) & 0x03)+ 1;
                        m_pAACAudioInfo->subFormat = 1; //ADTS
                        m_pAACAudioInfo->samplingFreq = ((firstSampleBuf [2] >> 2) & 0x0F);
                        m_pAACAudioInfo->channelConfig = ((firstSampleBuf [2] << 2) & 0x04)|
                                                         ((firstSampleBuf [3] >> 6) & 0x03);

                        if(MAKE_AAC_AUDIO_CONFIG(buf, m_pAACAudioInfo->audioObject,
                          m_pAACAudioInfo->samplingFreq, m_pAACAudioInfo->channelConfig, (uint8*)pbufSize))
                        {
                          errorCode = PARSER_ErrorNone;
                          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM, "Successful in preparing AAC configuration...");
                        }
                      }
                      else
                      {
                        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH, "AAC does not match ADTS signature....");
                      }
                    }
                    else
                    {
                      if(!m_pAACAudioInfo)
                      {
                        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL, "Failed to allocte memory for avi_aac_info");
                        errorCode = PARSER_ErrorMemAllocFail;
                      }
                    }
                    resetPlayback(0, id, false, &bError, 1);
                    MM_Free(firstSampleBuf);
                  }
                  else
                  {
                    errorCode = PARSER_ErrorInsufficientBufSize;
                  }
                }//if(!m_pAACAudioInfo)
                if( m_pAACAudioInfo &&
                    MAKE_AAC_AUDIO_CONFIG(buf, m_pAACAudioInfo->audioObject,
                                        m_pAACAudioInfo->samplingFreq,
                                        m_pAACAudioInfo->channelConfig, (uint8*)pbufSize)
                  )
                {
                  errorCode = PARSER_ErrorNone;
                  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM, "Successful in preparing AAC configuration...");
                }
              }
           }
           else if(aInfo.strfAudio.wFormatTag == AVI_AUDIO_WMA_PRO)
           {
             if(aInfo.strfAudio.cbSize)
              {
                if(pbufSize)
                {
                  *pbufSize = aInfo.strfAudio.cbSize;
                  errorCode = PARSER_ErrorNone;

                  if(buf)
                  {
                    memcpy(buf,aInfo.strfAudio.extra,aInfo.strfAudio.cbSize);
                    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM, "Successful in preparing WMA PRO configuration...");
                  }
                }
             }
           }
       }
       break;
     default:
       break;
   }
 }
 return errorCode;
}
/* ======================================================================
FUNCTION:
  AVIFile::getTrackDecoderSpecificInfoContent

DESCRIPTION:
  this returns Sequence (VOL) Header.

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
uint8 *AVIFile::getTrackDecoderSpecificInfoContent(uint32 id)
{
  CHUNK_t cType;
  if(!m_pAVIParser)
  {
    return NULL;
  }
  if(m_pAVIParser->GetTrackChunkType(id,&cType)==AVI_SUCCESS)
  {
    switch(cType)
    {
      case AVI_CHUNK_VIDEO:
        return m_pAVIParser->GetAVIVolHeader((avi_uint8)id);
     default:
       break;
    }
  }
  return NULL;
}
/* ======================================================================
FUNCTION:
  AVIFile::getTrackDecoderSpecificInfoSize

DESCRIPTION:
  this returns Sequence (VOL) Header size.

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
uint32 AVIFile::getTrackDecoderSpecificInfoSize(uint32 id)
{
 CHUNK_t cType;
 uint32 pbufSize = (sizeof(fourCC_t)/2); //Only for AAC audio

 if(!m_pAVIParser)
 {
   return 0;
 }
 if(m_pAVIParser->GetTrackChunkType(id,&cType)==AVI_SUCCESS)
 {
   switch(cType)
   {
     case AVI_CHUNK_VIDEO:
       return (m_pAVIParser->GetAVIVolHeaderSize(id));

     case AVI_CHUNK_AUDIO:
       avi_audiotrack_summary_info audioSummaryInfo;
       avi_audio_info aInfo ;
       if ( (m_pAVIParser->GetAudioTrackSummaryInfo(id,&audioSummaryInfo)==AVI_SUCCESS) &&
            (m_pAVIParser->GetAudioInfo(id,&aInfo)==AVI_SUCCESS) &&
            ((aInfo.strfAudio.wFormatTag == AVI_AUDIO_AAC) ||(aInfo.strfAudio.wFormatTag == AVI_AUDIO_AAC_FAAD) ))
       {
        MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM, "AVIFile::getTrackDecoderSpecificInfoSize pbufSize %lu", pbufSize);
        return pbufSize;
       }

     default:
       break;
   }
 }
 return 0;
}
#ifdef FEATURE_FILESOURCE_DIVX_DRM
/* =============================================================================
FUNCTION:
 AVIFile::GetDRMInfo

DESCRIPTION:
Returns DRM header and the size of the header.

INPUT/OUTPUT PARAMETERS:

RETURN VALUE:

SIDE EFFECTS:
  None.
=============================================================================*/
avi_uint8* AVIFile::GetDRMInfo(int* size)
{
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW, "AVIFile::GetDRMInfo");
  if(!m_pAVIParser)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH, "AVIFile::GetDRMInfo Error,returning NULL");
    return NULL;
  }
  return m_pAVIParser->GetDRMInfo(size);
}
/* =============================================================================
FUNCTION:
 AVIFile::IsDRMProtection

DESCRIPTION:
Returns True if file is protected else returns false.

INPUT/OUTPUT PARAMETERS:

RETURN VALUE:

SIDE EFFECTS:
  None.
=============================================================================*/
bool AVIFile::IsDRMProtection()
{
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW, "AVIFile::IsDRMProtection");
  if(!m_pAVIParser)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH, "AVIFile::IsDRMProtection Error,returning FALSE");
    return false;
  }
  return m_pAVIParser->IsDRMProtection();
}
#endif//#ifdef FEATURE_FILESOURCE_DIVX_DRM
/* =============================================================================
FUNCTION:
 AVIFile::GetDRMType

DESCRIPTION:
Returns DRM scheme used by underlying file, if any.

INPUT/OUTPUT PARAMETERS:

RETURN VALUE:

SIDE EFFECTS:
  None.
=============================================================================*/
FileSourceStatus AVIFile::GetDRMType(FileSourceDrmType& drmtype)
{
  drmtype = FILE_SOURCE_NO_DRM;
#ifdef FEATURE_FILESOURCE_DIVX_DRM
  if(IsDRMProtection())
  {
    //If file has DRM, it has to be DIVX_DRM as no other drm scheme is supported in avi/divx.
    drmtype = FILE_SOURCE_DIVX_DRM;
  }
#endif
  return FILE_SOURCE_SUCCESS;
}
/* ======================================================================
FUNCTION:
  AVIFile::SetTimeStampedSample

DESCRIPTION:
  gets closest sample's info of the closest frame of given time.

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
bool AVIFile::SetTimeStampedSample(uint32 id, uint64 TimeStamp, uint64 *newTimeStamp, boolean isRewind)
{
  //U64_WMC u64TimeReturn = 0;
  //tWMCDecStatus Ret;
  //tMediaType_WMC Type;
  //U64_WMC u64TempTimeReturn = 0;
  //uint32 StreamNum;

  //if(GetMediaTypeForStreamID(&Type, id) == WMCDec_Succeeded)
  //{
  //  if( TimeStamp && (Type==Video_WMC) )
  //  {
  //    if(isRewind)
  //    {
  //      Ret = WMCDecSeek (m_hASFDecoder, TimeStamp, &u64TempTimeReturn);

  //      if(isIndexObjectPresent())
  //      {
  //        if(Ret == WMCDec_Succeeded)
  //        {
  //          Ret = WMCDecSeekToPrevI(m_hASFDecoder, (U16_WMC)id);
  //          if(Ret == WMCDec_Succeeded)
   //         {
  //            Ret = WMCDecGetIFrameTime (m_hASFDecoder, (U16_WMC)id, u64TempTimeReturn, &u64TimeReturn, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
   //         }
  //        }//end of if(Ret == WMCDec_Succeeded)
  //      }
  //      else
  //      {
  //        if(Ret == WMCDec_Succeeded)
  //        {
  //          Ret = WMCDecSeekToPrevIFrame(m_hASFDecoder, (U16_WMC)id, &u64TimeReturn );
  //        }
  //      }
  //    }
  //    else
  //    {
  //      /* Now we can handle FF with/without Simple index object */
  //      Ret = WMCDecGetNextIFrameTime (m_hASFDecoder, (U16_WMC)id, TimeStamp, &u64TempTimeReturn, NULL, NULL, NULL, NULL );
  //      if(Ret == WMCDec_Succeeded)
  //        {
  //        Ret = WMCDecSeek (m_hASFDecoder, u64TempTimeReturn, &u64TimeReturn);
  //      }
  //    }

  //    if(Ret == WMCDec_Succeeded)
  //    {
  //      MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_MEDIUM, "SetTimeStampedSample, Time IN=%d, OUT=%d", TimeStamp,u64TimeReturn);
  //      *newTimeStamp = (uint32)u64TimeReturn;
  //      return true;
  //    }
  //  }//end of if( TimeStamp && (Type==Video_WMC) )
  //  else
  //  {
  //   if(GetStreamNumForID(&StreamNum, id) == WMCDec_Succeeded)
  //    {
  //      if( (m_bWMADecodeDone) && (!isRewind) )
  //      {
  //        //If audio is already done and user is attempting FWD, fail the repositioning.
  //        *newTimeStamp = m_sampleInfo[StreamNum].time;
  //        m_sampleInfo[StreamNum].size = 0;
  //        MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,"SetTimeStampedSample, Seek failed for track  %d as WMA track has ended.",id);
  //        return false;
  //      }
  //    }

  //    Ret = WMCDecSeek (m_hASFDecoder, TimeStamp, &u64TimeReturn);
  //    if( (Ret == WMCDec_Succeeded)|| (Ret == WMCDec_DecodeComplete) )
  //    {
  //      if(Ret == WMCDec_DecodeComplete)
  //      {
  //        *newTimeStamp = (uint32)TimeStamp;
  //        if(GetStreamNumForID(&StreamNum, id) == WMCDec_Succeeded)
  //        {
  //            m_sampleInfo[StreamNum].time = (uint32)TimeStamp;
  //          m_bWMADecodeDone = true;
  //          m_nWMATotalDummyBytesSent = WMA_TOTAL_DUMMY_BYTES_TO_SEND;
  //          m_sampleInfo[StreamNum].size = 0;
  //          MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,"SetTimeStampedSample, Seek to EOF TS %d",TimeStamp);
  //        }
  //      }//end of if(Ret == WMCDec_DecodeComplete)
  //      else
  //      {
  //        *newTimeStamp = (uint32)u64TimeReturn;
  //      }
  //      MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_MEDIUM,"SetTimeStampedSample, Time IN=%d, OUT=%d", TimeStamp, *newTimeStamp);
  //      return true;
  //    }
  //  }
  //}
  //MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR, "SetTimeStampedSample failed");
  return false;
}
#if defined(FEATURE_FILESOURCE_PSEUDO_STREAM) || defined(FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD)
/*===========================================================================

FUNCTION  sendParserEvent

DESCRIPTION
  Public method used send parser events

===========================================================================*/
void AVIFile::sendParserEvent(ParserStatusCode status)
{
  parserState = status;
}

/*===========================================================================

FUNCTION  sendParseHTTPStreamEvent

DESCRIPTION
  Public method used to switch contexts and call the parseHttpStream.

===========================================================================*/
void AVIFile::sendParseHTTPStreamEvent(void)
{

}

/*===========================================================================

FUNCTION  updateBufferWritePtr

DESCRIPTION
  Public method used to update the write buffer offset during Http streaming.

===========================================================================*/
void AVIFile::updateBufferWritePtr ( uint64 writeOffset )
{
  m_wBufferOffset = writeOffset;

  if((parserState == PARSER_PAUSE) || (parserState == PARSER_RESUME))
  {
     //check if we got sufficient data to start parsing the
     //meta data.
     sendParseHTTPStreamEvent();
  }
}

/*===========================================================================

FUNCTION  getMetaDataSize

DESCRIPTION
  Public method used to determine the meta-data size of the fragment. It will
  parse until it finds MOVI and sets that as m_minOffsetRequired.

===========================================================================*/
bool AVIFile::getMetaDataSize ( void )
{
  bool bRet = false;
  avi_uint64 nOffset = 12;
  avi_uint32 nListSize = 0;
  avi_uint32 nSize = 0;
  avi_uint8  readBuffer[AVI_READ_BUFFER_SIZE];
  unsigned char* byteData = readBuffer;
  aviErrorType retError = AVI_SUCCESS;
  void* pUserData = this;
  bool bfoundHDRL = false;

  while( (nOffset < m_fileSize) && (retError == AVI_SUCCESS) )
  {
    if(!AVICallbakGetData(nOffset,sizeof(fourCC_t),readBuffer,AVI_READ_BUFFER_SIZE,pUserData))
    {
      retError = AVI_DATA_UNDERRUN;
      break;
    }
    nOffset+=sizeof(fourCC_t);
    if(!memcmp(byteData,AVI_LIST_FOURCC,sizeof(fourCC_t)))
    {
      //read in size of LIST
      if(!AVICallbakGetData(nOffset,sizeof(avi_uint32),readBuffer,AVI_READ_BUFFER_SIZE,pUserData))
      {
        retError = AVI_DATA_UNDERRUN;
        break;
      }
      nOffset+=sizeof(avi_uint32);
      memcpy(&nListSize,byteData,sizeof(avi_uint32));

      if(!AVICallbakGetData(nOffset,sizeof(fourCC_t),readBuffer,AVI_READ_BUFFER_SIZE,pUserData))
      {
        m_pAVIParser->setParserState(AVI_PARSER_READ_FAILED,&retError);
        retError = AVI_READ_FAILURE;
      }
      nOffset+=sizeof(fourCC_t);
      if(!memcmp(byteData,AVI_HDRL_FOURCC,sizeof(fourCC_t)))
      {
        bfoundHDRL = true;
      }
      //Make sure we have skipped HDRL(taking care of clips that could have HDRL after movi though we
      //have not come across such clips till now)
      if( (!memcmp(byteData,AVI_MOVI_FOURCC,sizeof(fourCC_t))) && (bfoundHDRL) )
      {
        bGetMetaDataSize = FALSE;
        bRet = true;
        m_minOffsetRequired = nOffset;
        break;
      }
      else
      {
        nOffset -= sizeof(fourCC_t);
        nOffset += nListSize;
      }
    }
    else
    {
      if(!AVICallbakGetData(nOffset,sizeof(avi_uint32),readBuffer,AVI_READ_BUFFER_SIZE,pUserData))
      {
        m_pAVIParser->setParserState(AVI_PARSER_READ_FAILED,&retError);
        break;
      }
      nOffset += sizeof(avi_uint32);
      memcpy(&nSize,byteData,sizeof(avi_uint32));
      nOffset += nSize;
    }
  }
  return bRet;
}

/*===========================================================================

FUNCTION  parseHTTPStream

DESCRIPTION
  Public method used to parse the Http Stream.

===========================================================================*/
bool AVIFile::parseHTTPStream ( void )
{

  bool returnStatus = true;

  if(m_playText)
  {
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM, "parseHTTPStream reporting PARSER_READY when m_playText is true...");
    sendParserEvent(PARSER_READY);
    return returnStatus;
  }
  if(bGetMetaDataSize)
    returnStatus = getMetaDataSize();

  if(!returnStatus && (m_aviParseLastStatus == AVI_READ_FAILURE))
  {
    sendParserEvent(PARSER_PAUSE);
    MM_MSG_PRIO(MM_GENERAL, MM_PRIO_MEDIUM, "Parser State = Common::PARSER_PAUSE");
    return returnStatus;
  }

  MM_MSG_PRIO3(MM_GENERAL, MM_PRIO_MEDIUM, "parseHTTPStream: m_wBufferOffset=%llu, m_minOffsetRequired=%llu, m_playVideo=%d",
    m_wBufferOffset, m_minOffsetRequired, m_playVideo);

  if((m_wBufferOffset >= m_minOffsetRequired)
    && m_wBufferOffset && m_minOffsetRequired)
  {

    /*Parse the fragment here..*/
    if(!ParseMetaData())
    {
      //QTV_PS_PARSER_STATUS_PAUSED
      sendParserEvent(PARSER_PAUSE);
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_HIGH, "Parser State = Common::PARSER_PAUSE, m_playVideo=%d",m_playVideo);
      return false;
    }

    if ( ((parserState == PARSER_RESUME)||(parserState == PARSER_READY)) )
    {
      //QTV_PS_PARSER_STATUS_READY
      MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM, "Parser State = Common::PARSER_READY, m_playVideo=%d",m_playVideo);
      sendParserEvent(PARSER_READY);
      return true;
    }
    else
    {
      return false;
    }
  }
  else
  {
    //QTV_PS_PARSER_STATUS_PAUSED
    sendParserEvent(PARSER_PAUSE);
    MM_MSG_PRIO1(MM_GENERAL, MM_PRIO_MEDIUM, "Parser State = Common::PARSER_PAUSE, m_playVideo=%d",m_playVideo);
    return false;
  }

}

/*===========================================================================

FUNCTION  sendHTTPStreamUnderrunEvent

DESCRIPTION
  Public method used to switch contexts and notify the player about buffer-underrun.

===========================================================================*/
void AVIFile::sendHTTPStreamUnderrunEvent(void)
{

}
///*===========================================================================
//
//FUNCTION  GetHTTPStreamDownLoadedBufferOffset
//
//DESCRIPTION
//  Public method used to switch contexts and notify the player about buffer-underrun.
//
//===========================================================================*/
//boolean AVIFile::GetHTTPStreamDownLoadedBufferOffset(U32_WMC * pOffset)
//{
//  WMFDecoderEx *pDecoder = (WMFDecoderEx *)m_hASFDecoder;
//
//  if(pDecoder && pOffset)
//  {
//    if(m_fpFetchBufferedDataSize)
//    {
//      //Pull interface so pull dnld data size from OEM
//      m_fpFetchBufferedDataSize( 0, &(pDecoder->wHttpDataBuffer.Offset, m_QtvInstancehandle) );
//      pDecoder->wHttpDataBuffer.bValid = TRUE;
//    }
//    if( pDecoder->wHttpDataBuffer.bValid )
//    {
//      *pOffset = pDecoder->wHttpDataBuffer.Offset;
//      return TRUE;
//    }
//  }
//  return FALSE;
//}
//
/*===========================================================================

FUNCTION  GetTotalAvgBitRate

DESCRIPTION
  Public method used to switch contexts and notify the player about buffer-underrun.

===========================================================================*/
bool AVIFile::GetTotalAvgBitRate(uint32 * pBitRate)
{
  if(!m_nNumStreams)
  {
    return false;
  }

  //*pBitRate = 0;

  //for(uint8 index = 0; index < m_nNumStreams; index++)
  //{
  //   /* the Movie average bitrate from sum of all the tacks average bitrate */
  //   if(m_ptMediaTypeStreams)
  //   {
  //     *pBitRate = *pBitRate + getTrackAverageBitrate(m_ptMediaTypeStreams[index].wStreamId);
  //   }
  //}

  return true;
}
/*===========================================================================

FUNCTION  GetBufferedDuration

DESCRIPTION
Public method used to determine the buffered track time in ms for a track

===========================================================================*/
bool AVIFile::getBufferedDuration(uint32 id, int64 nBytes, uint64 *pBufferedTime)
{
  bool returnStatus = false;
  avi_uint16 trackID;
  aviErrorType retVal = AVI_SUCCESS;
  avi_sample_info minfo;
  memset(&minfo,0,sizeof(avi_sample_info));
  avi_uint64 prevOffset = (m_pAVIParser->GetSampleInfoOffset());
  uint64 ullAvailOffset=(uint64)nBytes;
  if(m_pStreamPort)
  {
     //Pull interface so pull dnld data size from OEM
    if(nBytes < 0)
    {
      // if buffered/downloaded bytes is not provided then get the available offset
      m_pStreamPort->GetAvailableOffset((int64*)&m_wBufferOffset, &m_bEndOfData);
      ullAvailOffset = m_wBufferOffset;
    }
  }

  while( (retVal == AVI_SUCCESS ) && (ullAvailOffset > prevOffset ) )
  {
    retVal = m_pAVIParser->GetSampleInfo(&prevOffset,id,&minfo,0,&trackID);
    prevOffset = (m_pAVIParser->GetSampleInfoOffset());
  }

  if (minfo.nTimeStamp!=0)
  {
    *pBufferedTime = minfo.nTimeStamp;
    returnStatus = true;
  }
  else
  {
    returnStatus = false;
  }
  return returnStatus;
  //return true;
}
/*! =======================================================================
   @brief         Returns absolute file offset(in bytes) associated
                  with time stamp 'pbtime'(in milliseconds).

   @param[in]     ullPBtime:Timestamp(in milliseconds) of the sample
                           that user wants to play/seek
   @param[in]     ulTrackId: Identifies elementary stream to
                  demultiplex from current pes packet
   @param[in]     ullCurrTS: Current playback TS
   @param[in]     ullReposTime: Reposition TS
   @param[out]    offset:Absolute file offset(in bytes) corresponding
                  to 'pbtime'

   @return        true if successful in retrieving the absolute
                  offset(in bytes) else returns false

   @note          This API can be called once FileSource report
                  successful OPEN_COMPLETE. In case of multiple
                  tracks audio, video & text, this API will return
                  minimum absolute offset in bytes
   ==========================================================================*/
bool AVIFile::GetOffsetForTime(uint64 ullPBTime, uint64* pullOffset,
       uint32 ulTrackId, uint64 ullCurrentPosTimeStamp,
       uint64& pullReposTime)
{
  bool bRet = false;
  aviErrorType eRetError = AVI_FAILURE;
  uint64 ullBytes=0;
  uint64 ullDuration=0;
  bool pbEndOfData=false;

  if (m_pAVIParser && pullOffset)
  {
    MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_MEDIUM,
        "GetOffsetForTime ullPBTime=%llu ulTrackId=%lu",
        ullPBTime, ulTrackId);
    *pullOffset = 0;

    uint32 ulBitRate = (uint32)getTrackAverageBitrate(ulTrackId);
    if(ulBitRate)
    {
      *pullOffset = (ulBitRate * ullPBTime)/(8 * 1000);
    }
    else if(m_fileSize)
    {
      ullDuration = m_pAVIParser->GetTrackDuration(ulTrackId);
      if(ullDuration)
      {
        uint64 ullMovieOffset = m_pAVIParser->GetMoviOffset();
        *pullOffset = ullMovieOffset + ((ullPBTime*
              (m_fileSize-ullMovieOffset))/ullDuration);
      }
    }
  }
  return bRet;
}

/*===========================================================================

FUNCTION  CanPlayTracks

DESCRIPTION
  Public method used to switch contexts and notify the player about buffer-underrun.

===========================================================================*/
bool AVIFile::CanPlayTracks(uint64 nTotalPBTime)
{
  //uint32 nMinBufferingTime = 0;
  //uint32 nMediaMaxPlayableTime = 0;
  //uint32  nTotalAvgBitRate = 0;
  //uint32 nHttpDownLoadBufferOffset = 0;
  //boolean bHttpDownLoadBufferOffsetValid = GetHTTPStreamDownLoadedBufferOffset(&nHttpDownLoadBufferOffset);

  //if( (m_HttpDataBufferMinOffsetRequired.bValid == FALSE) && GetTotalAvgBitRate( &nTotalAvgBitRate ) )
  //{
  //  /* Compute Required Buffering/Rebuffering time */

  //  (void)GetMediaMaxPlayableTime( &nMediaMaxPlayableTime );

  //  if(nTotalPBTime > nMediaMaxPlayableTime)
  //  {
  //    nMinBufferingTime = nTotalPBTime - nMediaMaxPlayableTime;
  //  }
  //  else
  //  {
  //    nMinBufferingTime = 0;
  //  }

  //  MM_MSG_PRIO3(MM_FILE_OPS, MM_PRIO_MEDIUM, " nTotalPBTime= %u, nMinBufferingTime= %u nMediaMaxPlayableTime = %u", nTotalPBTime, nMinBufferingTime,nMediaMaxPlayableTime);

  //  if( !nMinBufferingTime )
  //  {
  //    /* nBufferingTime from Mpeg4Plyer/OEM is '0', then take it as Preroll time */
  //    nMinBufferingTime = getPrerollTime();
  //    MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_HIGH, "Taking Preroll as Rebuffering Time: nTotalPBTime= %d, nMinBufferingTime= %d", nTotalPBTime, nMinBufferingTime);
  //  }

  //  /* Estimate approximate minimum offset required */
  //  m_HttpDataBufferMinOffsetRequired.Offset = nHttpDownLoadBufferOffset +  (nTotalAvgBitRate * nMinBufferingTime)/8000;
  //  m_HttpDataBufferMinOffsetRequired.bValid = TRUE;

  //  if( m_HttpDataBufferMinOffsetRequired.Offset > (GetLastPacketOffset() + GetAsfPacketSize()) )
  //  {
  //     m_HttpDataBufferMinOffsetRequired.Offset =  (GetLastPacketOffset() + GetAsfPacketSize());
  //  }

  //}

  //if((nHttpDownLoadBufferOffset >= m_HttpDataBufferMinOffsetRequired.Offset) &&
  //    bHttpDownLoadBufferOffsetValid && m_HttpDataBufferMinOffsetRequired.bValid )
  //{
  //  return true;
  //}
  //
  //MM_MSG_PRIO2(MM_GENERAL, MM_PRIO_MEDIUM,"CanPlayTracks: HttpDownLoadBufferOffset= %d, HttpDataBufferMinOffsetRequired= %d", nHttpDownLoadBufferOffset, m_HttpDataBufferMinOffsetRequired.Offset);

  return true;
}

/* ======================================================================
FUNCTION:
  AVIFile::GetMediaMaxTimeStampPlayable

DESCRIPTION:
  gets time stamp of current sample of the track.

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 time stamp in track time scale unit

SIDE EFFECTS:
  None.
======================================================================*/
//tWMCDecStatus AVIFile::GetMediaMaxPlayableTime(U32_WMC *nMaxPBTime)
//{
//  uint32 nMaxPlayableTime = 0;  // default max playable time
//
//  if( (m_pStreamDecodePattern == NULL) || (nMaxPBTime == NULL) )
//  {
//    return WMCDec_InValidArguments;
//  }
//  for(uint16 i=0; i<(int)m_nNumStreams; i++)
//  {
//    if( m_maxPlayableTime[i] && (m_pStreamDecodePattern[i].tPattern != Discard_WMC) )
//    {
//      if(!nMaxPlayableTime)
//      {
//        /* initialize with valid track sample time */
//        nMaxPlayableTime = m_maxPlayableTime[i];
//        continue;
//      }
//      /* Take the MIN value to make sure all tracks are playable atleast nMaxPlayableTime */
//      nMaxPlayableTime = MIN(m_maxPlayableTime[i],nMaxPlayableTime);
//    }
//  }
//
//    *nMaxPBTime = nMaxPlayableTime;
//
//  return WMCDec_Succeeded;
//}
#endif //  FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD

/* ======================================================================
FUNCTION:
  MP2Stream::GetAACAudioProfile

DESCRIPTION:
  Returns AAC audio profile.

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
uint32 AVIFile::GetAACAudioProfile(uint32 id)
{
  uint32 audioObject = 0;
  if(m_pAACAudioInfo)
  {
    audioObject = m_pAACAudioInfo->audioObject;
  }
  return audioObject;
}

/* ======================================================================
FUNCTION:
  MP2Stream::GetAACAudioFormat

DESCRIPTION:
  returns AAC Audio format

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
uint32 AVIFile::GetAACAudioFormat(uint32 id)
{
  uint32 audioFormat = 0;
  if(m_pAACAudioInfo)
  {
    audioFormat = m_pAACAudioInfo->subFormat;
  }
  return audioFormat;
}
/*!===========================================================================
  @brief      Get Audio/Video/Text stream parameter
  @details    This function is used to get Audio/Video stream parameter i.e.
              codec configuration, profile, level from specific parser.

  @param[in]  ulTrackId           TrackID of media
  @param[in]  ulParamIndex        Parameter Index of the structure to be
                                  filled.It is from the FS_MEDIA_INDEXTYPE
                                  enumeration.
  @param[in]  pParameterStructure Pointer to client allocated structure to
                                  be filled by the underlying parser.

  @return     PARSER_ErrorNone in case of success otherwise Error.
  @note
============================================================================*/
PARSER_ERRORTYPE AVIFile::GetStreamParameter( uint32 ulTrackId,
                                              uint32 ulParamIndex,
                                              void* pParamStruct)
{
  PARSER_ERRORTYPE eError = PARSER_ErrorInvalidParam;
  if(pParamStruct)
  {
    switch (ulParamIndex)
    {
      case FS_IndexParamAudioMP3:
      {
        FS_AUDIO_PARAM_MPGTYPE *pMPGInfo = (FS_AUDIO_PARAM_MPGTYPE*)pParamStruct;

        pMPGInfo->ulSamplingFreq    = getTrackAudioSamplingFreq(ulTrackId);
        pMPGInfo->ulBitRate         = (uint32)getTrackAverageBitrate(ulTrackId);
        pMPGInfo->usChannels        = (uint16)GetNumAudioChannels(ulTrackId);
        switch(GetFormatTag(ulTrackId))
        {
          case AVI_AUDIO_MP3:
               pMPGInfo->ucLayer = MP3_LAYER_3;
               pMPGInfo->ucVersion =  MP3_VER_1;
               break;
          case AVI_AUDIO_MP2:
               pMPGInfo->ucLayer = MP3_LAYER_2;
               pMPGInfo->ucVersion = MP3_VER_1;
               break;
          default:
               pMPGInfo->ucLayer = 0;
               pMPGInfo->ucVersion = 0;
               break;
        }//switch(GetFormatTag(ulTrackId))
        eError = PARSER_ErrorNone;
        break;
      }//case FS_IndexParamAudioMP3:
      default :
        eError = PARSER_ErrorInvalidParam;
        break;
    }//switch (ulParamIndex)
  }// if(pParamStruct)
  return eError;
}
#endif /* FEATURE_FILESOURCE_AVI */
