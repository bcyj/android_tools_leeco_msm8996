/* =======================================================================
                              wavfile.cpp
DESCRIPTION
  Defines the functions to parse wav wav  format files
Copyright (c) 2009-2014 QUALCOMM Technologies Inc, All Rights Reserved.
QUALCOMM Technologies Proprietary and Confidential.

$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/WAVParserLib/main/latest/src/wavfile.cpp#42 $
$DateTime: 2014/02/07 02:53:30 $
$Change: 5240686 $
========================================================================== */

//=============================================================================
// INCLUDES
//=============================================================================
#include "parserinternaldefs.h"
#include "parserdatadef.h"
#include <stdio.h>
#include "wavfile.h"
#include "wavformatparser.h"
#include "filebase.h"
#include "MMMemory.h"

#ifdef FEATURE_FILESOURCE_DRM_DCF
#include "IxStream.h"
#endif
#ifdef FEATURE_FILESOURCE_WAVADPCM
//=============================================================================
// FUNCTION DEFINITIONS
//=============================================================================

//=============================================================================
// FUNCTION: Constructor
//
// DESCRIPTION:
//  Create an instance of wavformatparser, and init the class attributes
//
// PARAMETERS
//
//
//
//
// RETURN:
//  None
//
// SIDE EFFECTS
//  None
//=============================================================================
//
WAVFile::WAVFile()
{
  InitData();
}

WAVFile::WAVFile(const FILESOURCE_STRING &filename, unsigned char *pFileBuf,
                 uint64 bufSize)
{
  InitData();
  if (pFileBuf && bufSize)
  {
    m_pFileBuf = pFileBuf;
    m_FileBufSize = bufSize;
    m_WAVFilePtr = OSCL_FileOpen (pFileBuf, bufSize);
    m_fileSize = bufSize;
  }
  else
  {
    m_filename = filename;
//#ifdef FEATURE_FILESOURCE_FILE_IO_CACHING
#if 0 //Disabling as the buffer siz has to be decided for caching
    /* Calling with 10K cache  buffer size */
    m_WAVFilePtr = OSCL_FileOpen(filename, (OSCL_TCHAR *) _T("rb"), FILE_READ_BUFFER_SIZE_FOR_WAV );
#else
    m_WAVFilePtr = OSCL_FileOpen(filename, (OSCL_TCHAR *) _T("rb"),WAV_CACHE_SIZE);
#endif
    m_fileSize = OSCL_FileSize( m_filename );
  }
  if(m_WAVFilePtr != NULL)
  {
    if(ParseWAVHeader())
    {
      _fileErrorCode = PARSER_ErrorNone;
      _success = true;
    }
  }
}

WAVFile::WAVFile(IxStream* pixstream)
{
  InitData();
#ifdef FEATURE_FILESOURCE_DRM_DCF
  m_pIxStream = pixstream;
  m_WAVFilePtr = OSCL_FileOpen(m_pIxStream);
  if(m_WAVFilePtr != NULL)
  {
    if(m_pIxStream)
    {
      (void)m_pIxStream->Size(&m_fileSize);
    }
    if(ParseWAVHeader())
    {
      _fileErrorCode = PARSER_ErrorNone;
      _success = true;
    }
  }
#else
  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
    "WAVFile::WAVFile (ixstream not implemented) %p", pixstream);
#endif
}
#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
WAVFile::WAVFile(video::iStreamPort* pport)
{
  InitData();
  m_pPort = pport;
  m_WAVFilePtr = OSCL_FileOpen(pport);
  if(m_WAVFilePtr != NULL)
  {
    if(m_pPort)
    {
      int64 noffset = 0;
      if(video::iStreamPort::DS_SUCCESS == m_pPort->GetContentLength(&noffset))
      {
        m_fileSize = (uint64)noffset;
      }
    }
    if(ParseWAVHeader())
    {
      _fileErrorCode = PARSER_ErrorNone;
      _success = true;
    }
  }
}
#endif

/*
* Initialize the class members to the default values.
*/
void WAVFile::InitData()
{
  memset(&m_audsampleinfo,0,sizeof(m_audsampleinfo));
  m_SEEK_DONE = false;
  m_uSeektime = 0;
  m_validDataOffset = 0;
  m_DecodedBytes = 0;
  _fileErrorCode = PARSER_ErrorDefault;
  _success = false;
  m_bMediaAbort = false;
  m_pIxStream = NULL;
  m_filename = NULL;
  m_WAVFilePtr = NULL;
#ifdef FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD
  m_pPort = NULL;
#endif
  m_fileSize         = 0;
  m_pFileBuf         = NULL;
  m_FileBufSize      = 0;
  m_bStreaming       = FALSE;
  m_pimaAdpcmDecoder = NULL;
  m_pwavformatParser = NULL;
  m_BlockSize        = 0;
  m_wavformat        = 0;
  m_pDataBuffer      = NULL;
  m_nDataBufSize     = 0;
  //! By default do not use PCM related config enums.
  //! Let user decides to enable one of the macros
  //! FILE_SOURCE_MEDIA_DISABLE_PCM_SAMPLE_UPGRADE or
  //! FILE_SOURCE_MEDIA_ENABLE_PCM_SAMPLE_UPGRADE
  m_eConfigItem      = FILE_SOURCE_MEDIA_OUTPUT_BYTES_STREAM;
}

void WAVFile::SetCriticalSection(MM_HANDLE pcriticalsection)
{
  if(m_WAVFilePtr)
  {
    m_WAVFilePtr->pCriticalSection = pcriticalsection;
  }
}
uint32 WAVFile::GetAudioBitsPerSample(int)
{
  uint32 bitspersample = 0;
  tech_data_wav techdata;
  memset(&techdata,0,sizeof(tech_data_wav));
  if(m_pwavformatParser)
  {
    if(PARSER_ErrorNone == m_pwavformatParser->GetFormatChunk(&techdata))
    {
      bitspersample = 16;
      if( (techdata.format != WAV_IMA_ADPCM) && (techdata.format != WAV_UNKNOWN) )
      {
        bitspersample = techdata.bits_per_sample;
      }
    }
  }
  return bitspersample;
}

//=============================================================================
// FUNCTION: Destructor
//
// DESCRIPTION:
//  Free any resources allocated
//
// PARAMETERS
//  None
//
// RETURN:
//  None
//
// SIDE EFFECTS
//  None
//=============================================================================
//

WAVFile::~WAVFile()
{
  if(m_WAVFilePtr!=NULL)
  {
    (void) OSCL_FileClose(m_WAVFilePtr);
    m_WAVFilePtr = NULL;
  }

  if(m_pwavformatParser)
  {
    MM_Delete( m_pwavformatParser);
    m_pwavformatParser = NULL;
  }
  if(m_pimaAdpcmDecoder)
  {
    MM_Delete(m_pimaAdpcmDecoder);
  }
  if(m_pDataBuffer)
  {
    MM_Free(m_pDataBuffer);
    m_pDataBuffer = NULL;
  }
}

/* ======================================================================
FUNCTION
  WAVCallbakGetData

DESCRIPTION
  Its a callback function from the SDK to get the WAV data for decoding.
  This is not implemented by the SDK. It should be implemented by the app
  that calls the SDK.

ARGUMENTS
  ullOffset           Offset of the requested data (from beginning),
  ulNumBytesRequest   Size of the requested data (in bytes).
  pucData             Pointer to data buffer.
  ulMaxBufSize        Maximum data buffer size.
  pUserData           Client user data information.

DEPENDENCIES
  Used by WAV Parser.

RETURN VALUE
  The number of bytes actually being filled in

SIDE EFFECTS
  Detail any side effects.
========================================================================== */

uint32 WAVCallbakGetData (uint64 ullOffset,
                          uint32 ulNumBytesRequest,
                          uint8* pucData,
                          uint32 ulMaxBufSize,
                          void*  pUserData )
{
  uint32 nRead = 0;
  if(pUserData)
  {
    WAVFile *pWAVFile = (WAVFile *)pUserData;
    nRead = pWAVFile->FileGetData( ullOffset, ulNumBytesRequest,
                                   ulMaxBufSize, pucData );
  }
  return nRead;
}

// ======================================================================
//FUNCTION:
//  WAVFile::FileGetData
//
//DESCRIPTION:
//  To read the data from the file
//INPUT/OUTPUT PARAMETERS:
//  nOffset         : Offset from which data is being requested
//  nNumBytesRequest: Total number of bytes requested
//  ppData          : Buffer to be used for reading the data
//
//RETURN VALUE:
// Number of bytes read
//
//SIDE EFFECTS:
//  None.
//======================================================================
uint32 WAVFile::FileGetData(  uint64 nOffset,
                              uint32 nNumBytesRequest,
                              uint32 nMaxSize,
                              uint8 *pData  )
{
  uint32 nRead = 0;
  uint8 *decode_inbuffer;
  if (m_WAVFilePtr != NULL)
  {
    if(!m_bMediaAbort)
    {
      switch(m_wavformat)
      {
        case WAV_IMA_ADPCM:
          decode_inbuffer = (uint8 *) MM_Malloc(m_BlockSize*sizeof(uint8));
          nRead = FileBase::readFile(m_WAVFilePtr, decode_inbuffer, nOffset,
                                     FILESOURCE_MIN(m_BlockSize, nMaxSize));

          if(nRead != 0)
          {
            uint8 *pAdpcm = decode_inbuffer;
            uint32 nAdpcmLength = nRead;
            uint32 rAdpcmUsed;
            int16 *pSamples = reinterpret_cast< short *> (pData);
            // Making sure nSampleLength is under MAX_BUFF size reported by parser
            uint32 nSamplesLength = FILESOURCE_MIN(nNumBytesRequest,
                                          m_pwavformatParser->m_maxBufferSize);
            uint32 rSamplesWritten;
            uint16 nBlockSize = m_BlockSize;
            // call decode function to decode adpcm samples
            m_pimaAdpcmDecoder->wav_parser_adpcm_dec_Process ( pAdpcm,
                                          nAdpcmLength, rAdpcmUsed, pSamples,
                                          nSamplesLength,rSamplesWritten,
                                          nBlockSize );

            m_DecodedBytes = (rSamplesWritten)*WAV_PCM_SAMPLE_SIZE;
            m_DecodedBytes = (m_DecodedBytes-m_validDataOffset);
            m_validDataOffset = 0;//resets the offset value
            m_pwavformatParser->ActualBytesDecoded(m_DecodedBytes);
          }
          MM_Free(decode_inbuffer);
          break;

        case WAV_PCM:
        case WAV_MULTICHANNEL_PCM:
        case WAV_ALAW:
        case WAV_ULAW:
        case WAV_GSM:
          nRead = FileBase::readFile(m_WAVFilePtr, pData,
                                     nOffset, FILESOURCE_MIN(nNumBytesRequest,nMaxSize));
          break;
      }
    }
    else
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR, "Breaking, user Abort is true.");
    }
  }
  return nRead;
}


//===========================================================================
//
//FUNCTION
//  WAVFile::ParseWAVHeader
//
//DESCRIPTION
//  creates instance of WAVparser and calls start on parsing WAV file
//
//DEPENDENCIES
//  None
//
//INPUT PARAMETERS:
//->None
//->
//
//RETURN VALUE
//  true or false
//
//SIDE EFFECTS
// None
//===========================================================================*/

bool WAVFile::ParseWAVHeader()
{
  bool ret = false;
  //Creates wavformatParser object
  m_pwavformatParser = MM_New_Args(wavformatParser,(this,m_fileSize,m_WAVFilePtr));

  if(m_pwavformatParser)
  {
    if( PARSER_ErrorNone !=(m_pwavformatParser->StartParsing()) )
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                  "WAVFile::ParseWAVHeadera failed..");
      ret = false;
    }
    else
    {
      ret = true;
      m_pwavformatParser->get_wav_subtype(&m_wavformat);
      if(m_wavformat == WAV_IMA_ADPCM && m_pimaAdpcmDecoder == NULL)
      {
        // Create CAdpcmDecoderLib object for ima_adpcm decoding
        m_pimaAdpcmDecoder = MM_New_Args(CAdpcmDecoderLib,());
        if (NULL == m_pimaAdpcmDecoder)
        {
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                         "WAVFile::ParseWAVHeadera:Failed to create CAdpcmDecoderLib object");
          ret = false;
        }
        int trackID = 0;
        uint32 t_Channels = 0;
        //Get No of channels
        t_Channels = GetNumAudioChannels(trackID);
        //Initialise the no of channels for decoding
        m_pimaAdpcmDecoder->wav_parser_adpcm_dec_Init((uint16)t_Channels);
      }
    }
  }
  return ret;
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
PARSER_ERRORTYPE WAVFile::getNextMediaSample(uint32 /*ulTrackID*/, uint8 *pucDataBuf,
                                             uint32 *pulBufSize, uint32 &/*rulIndex*/)
{
  uint32 ulOutDataSize       = 0;
  uint8* pDataBuffer         = pucDataBuf;
  PARSER_ERRORTYPE retStatus = PARSER_ErrorDefault;
  tech_data_wav  WavTechData;
  memset(&WavTechData, 0, sizeof(tech_data_wav));

  /* Validate input params and class variables */
  if((NULL == pulBufSize) || (NULL == pucDataBuf) ||
     (0 == *pulBufSize)   || (NULL == m_pwavformatParser))
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
               "getNextMediaSample invalid input params!!");
    return PARSER_ErrorInvalidParam;
  }
  (void)m_pwavformatParser->GetFormatChunk(&WavTechData);
  if(FILE_SOURCE_MEDIA_ENABLE_PCM_SAMPLE_UPGRADE == m_eConfigItem)
  {
    if (NULL == m_pDataBuffer)
    {
      /* Allocate internal buffer based on output bufferSize. */
      if (8 == WavTechData.bits_per_sample)
      {
        m_nDataBufSize = FILESOURCE_MIN(m_nDataBufSize, *pulBufSize / 2);
      }
      else if (24 == WavTechData.bits_per_sample)
      {
        m_nDataBufSize = FILESOURCE_MIN(m_nDataBufSize, (*pulBufSize * 3)/ 4);
      }

      m_pDataBuffer = (uint8*)MM_Malloc(m_nDataBufSize);
      if(!m_pDataBuffer)
      {
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,
                    "getNextMediaSample malloc failed!!");
        return PARSER_ErrorMemAllocFail;
      }
    }
    ulOutDataSize = m_nDataBufSize;
    pDataBuffer   = m_pDataBuffer;
  }
  else
  {
    /* If up-sampling is not required, then update with buffer size already
       stored as part of Max Buf Size calculation */
    m_nDataBufSize = *pulBufSize;
    ulOutDataSize  = *pulBufSize;
    /* Provide data at the frame boundary, Frame means sample from each channel*/
    m_nDataBufSize -= (m_nDataBufSize %
                      (WavTechData.bits_per_sample * WavTechData.channels / 8));
  }

  retStatus = m_pwavformatParser->GetCurrentSample(pDataBuffer,
                                                   m_nDataBufSize,
                                                   &ulOutDataSize);
  if (PARSER_ErrorNone == retStatus)
  {
    bool bUpgrade = false;
    *pulBufSize = ulOutDataSize;
    /* If this configuration is set, then Parser will upgrade 24bit sample
       data to 32bit samples. It is required for QNX for now.*/
    if(FILE_SOURCE_MEDIA_ENABLE_PCM_SAMPLE_UPGRADE == m_eConfigItem)
    {
      bUpgrade = true;
      ConvertPCMSampleData(m_pDataBuffer, pucDataBuf, ulOutDataSize, pulBufSize,
                           (uint8)WavTechData.bits_per_sample, bUpgrade);
    }
  }
  else
  {
    *pulBufSize = 0;
    retStatus   = PARSER_ErrorEndOfFile;
  }
  if(m_SEEK_DONE == true)
  {
    m_audsampleinfo.btimevalid = true;
  }
  else
  {
    m_audsampleinfo.btimevalid = false;
  }
#ifdef _ANDROID_
  m_audsampleinfo.time       = m_pwavformatParser->GetCurSampleTime();
  m_audsampleinfo.btimevalid = true;
#endif
  m_SEEK_DONE = false;
  return retStatus;
}

// ======================================================================
//FUNCTION:
//  WAVFile::getMediaTimestampForCurrentSample
//
//DESCRIPTION:
//  gets time stamp of current sample of the track.
//
//INPUT/OUTPUT PARAMETERS:
//>>id: sample ID
//
//RETURN VALUE:
// time stamp in track time scale unit
//
//SIDE EFFECTS:
//  None.
//======================================================================
uint64 WAVFile::getMediaTimestampForCurrentSample(uint32 /*id*/)
{
  uint64 nTimeStamp = 0;
  if(m_pwavformatParser)
  {
    nTimeStamp = m_audsampleinfo.time;
  }
  return nTimeStamp;
}

// ======================================================================
//FUNCTION:
//  WAVFile::resetPlayback
//
//DESCRIPTION:
//  resets the playback time to given time(pos) for a track.
//  Also tells if we need to goto closest sync sample or not.
//
//INPUT/OUTPUT PARAMETERS:
//>>repos_time: seek position in terms of time
//>>id: sample ID
//>>bSetToSyncSample: When true, audio can be repositioned to non key frame
//<<bError: error type will be filled in this parameter
//>>currentPosTimeStamp: current plyabck time
//RETURN VALUE:
// new reposition time
//
//SIDE EFFECTS:
//  None.
//======================================================================
//
uint64 WAVFile::resetPlayback(  uint64 repos_time, uint32 id, bool /*bSetToSyncSample*/,
                                bool* /*bError*/, uint64 currentPosTimeStamp)
{
   uint32 ValidDataOffset = 0;
   if(m_pwavformatParser)
   {
     m_uSeektime =m_pwavformatParser->Seek(repos_time,&ValidDataOffset );
     if(!m_uSeektime)
     {
       MM_MSG_PRIO3(MM_FILE_OPS,
                       MM_PRIO_HIGH,
                       " WAVFile::resetPlayback %ld repos_time %llu current TS %llu",
                       id,repos_time,currentPosTimeStamp);
       m_pwavformatParser->init_file_position();
     }
     if(m_wavformat == WAV_IMA_ADPCM)
     {
       //sets the data offset in decoder
       m_validDataOffset = ValidDataOffset;
       m_pimaAdpcmDecoder->SetDataOffset(ValidDataOffset);
     }
     m_audsampleinfo.time = m_uSeektime;
     m_SEEK_DONE = true;
    _fileErrorCode = PARSER_ErrorNone;
   }
   return m_uSeektime;
}
//Queries wav parser to determine if seek is supported or not.
//Non zero return indicates unsupported seek.
uint8  WAVFile::randomAccessDenied()
{
  uint8 nSeekDenied = 1;
  if(m_pwavformatParser)
  {
    nSeekDenied = m_pwavformatParser->RandomAccessDenied();
  }
  return nSeekDenied;
}
//======================================================================
//FUNCTION:
//  WAVFile::getMovieDuration
//
//DESCRIPTION:
//  gets movie duration in movie timescale unit.
//
//INPUT/OUTPUT PARAMETERS:
//  None.
//
//RETURN VALUE:
// returns duration of playback
//
//SIDE EFFECTS:
//  None.
//======================================================================
uint64 WAVFile::getMovieDuration() const
{

  uint64 nDuration = 0;

  if(!m_pwavformatParser)
  {
     return 0;
  }

  nDuration = m_pwavformatParser->GetClipDurationInMsec();

  return nDuration;

}

// ======================================================================
//FUNCTION:
//  WAVFile::getMovieTimescale
//
//DESCRIPTION:
//  gets movie timescale
//
//INPUT/OUTPUT PARAMETERS:
//  None.
//
//RETURN VALUE:
// return time scale
//
//SIDE EFFECTS:
//  None.
//======================================================================
uint32 WAVFile::getMovieTimescale() const
{
  uint32 uTime = 0;
  if(m_pwavformatParser)
  {
    uTime = WAV_STREAM_TIME_SCALE;
  }
  return uTime;
}

// ======================================================================
//FUNCTION:
//  WAVFile::getTrackWholeIDList
//
//DESCRIPTION:
//  gets list of track IDs
//
//INPUT/OUTPUT PARAMETERS:
//<<ids: track ID will be filled into this pointer
//
//RETURN VALUE:
// returns no of tracks
//
//SIDE EFFECTS:
//  None.
//======================================================================
uint32 WAVFile::getTrackWholeIDList(uint32 *ids)
{
  uint32 nTracks = 0;

  if((m_pwavformatParser)&&(ids))
  {
    nTracks = AUDIO_WAV_MAX_TRACKS;
    *ids = nTracks;
  }
  return nTracks;
}

// ======================================================================
//FUNCTION:
//  WAVFile::getTrackMediaDuration
//
//DESCRIPTION:
//  gets track duration in track time scale unit
//
//INPUT/OUTPUT PARAMETERS:
//>>id: track id no
//
//RETURN VALUE:
// returns track playback duration
//
//SIDE EFFECTS:
//  None.
//======================================================================*/
uint64 WAVFile::getTrackMediaDuration(uint32 /*id*/)
{
  uint64 nTrackDuration = 0;
  if( m_pwavformatParser )
  {
    nTrackDuration = m_pwavformatParser->GetClipDurationInMsec();
  }
  return nTrackDuration;
}


// ======================================================================
//FUNCTION:
//  WAVFile::getTrackMediaTimescale
//
//DESCRIPTION:
//  gets track time scale
//
//INPUT/OUTPUT PARAMETERS:
//>>id: track id no.
//
//RETURN VALUE:
// return media time scale
//
//SIDE EFFECTS:
//  None.
//======================================================================
uint32 WAVFile::getTrackMediaTimescale(uint32 /*id*/)
{
   uint32 uTime = 0;
   if(m_pwavformatParser)
   {
     uTime = WAV_STREAM_TIME_SCALE;
   }
   return uTime;
}

// ======================================================================
//FUNCTION:
//  WAVFile::getTrackAudioSamplingFreq
//
//DESCRIPTION:
//  gets audio track's sampling frequency
//
//INPUT/OUTPUT PARAMETERS:
//>>id: track ID
//
//RETURN VALUE:
// returns the sampling frequency of track
//
//SIDE EFFECTS:
//  None.
//======================================================================
uint32 WAVFile::getTrackAudioSamplingFreq(uint32 /*id*/)
{
  wav_header_wavh swav_header_wavh;
  uint32 nSamplingFreq = 0;
  if(m_pwavformatParser)
  {
    if(PARSER_ErrorNone == m_pwavformatParser->GetWAVHeader(&swav_header_wavh))
    {
      nSamplingFreq = swav_header_wavh.nSampleRate;
    }
  }
  return nSamplingFreq;
}
/* ======================================================================
FUNCTION:
  WAVFile::GetAudioChannelMask

DESCRIPTION:
  returns the channel mask.

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
uint32 WAVFile::GetAudioChannelMask(int /*id*/)
{
  tech_data_wav wav_data;
  memset(&wav_data, 0, sizeof(tech_data_wav));
  if(m_pwavformatParser)
  {
    if(PARSER_ErrorNone == m_pwavformatParser->GetFormatChunk(&wav_data))
      return wav_data.channel_mask;
  }
  return 0;
}
// ======================================================================
//FUNCTION:
//  WAVFile::GetNumAudioChannels
//
//DESCRIPTION:
//  returns number of audio channels
//
//INPUT/OUTPUT PARAMETERS:
//>>id: track ID.
//
//RETURN VALUE:
// returns no of audio channels
//
//SIDE EFFECTS:
//  None.
//======================================================================
uint32 WAVFile::GetNumAudioChannels(int /*id*/)
{
  uint32 nChannels = 0;
  wav_header_wavh swav_header_wavh;

  if(m_pwavformatParser)
  {
    if(PARSER_ErrorNone == m_pwavformatParser->GetWAVHeader(&swav_header_wavh))
    {
      nChannels = swav_header_wavh.nChannels;
    }
  }
  return nChannels;
}



// ======================================================================
//FUNCTION:
// WAVFile::getTrackMaxBufferSizeDB
//
//DESCRIPTION:
//  gets maximum buffer size to play the track
//
//INPUT/OUTPUT PARAMETERS:
//>>id: Track ID.
//
//RETURN VALUE:
// returns the buffer size
//
//SIDE EFFECTS:
//  None.
//======================================================================
int32 WAVFile::getTrackMaxBufferSizeDB(uint32 /*id*/)
{
  wav_audio_info audioInfo;
  tech_data_wav  WavTechData;
  int32          slBufferSize = 0;

  //memset the structure
  memset(&WavTechData, 0, sizeof(WavTechData));
  memset(&audioInfo, 0, sizeof(wav_audio_info));

  if(m_pwavformatParser)
  {
    (void)m_pwavformatParser->GetFormatChunk(&WavTechData);
    if(PARSER_ErrorNone == (m_pwavformatParser->GetAudioInfo(&audioInfo)))
    {
      slBufferSize = audioInfo.dwSuggestedBufferSize;
      m_BlockSize  = audioInfo.nBlockSize;
    }
    else
    {
      slBufferSize = WAV_DEFAULT_AUDIO_BUF_SIZE;
    }
  }
  m_nDataBufSize = (uint32)slBufferSize;

  if(FILE_SOURCE_MEDIA_ENABLE_PCM_SAMPLE_UPGRADE == m_eConfigItem)
  {
    if (24 == WavTechData.bits_per_sample)
    {
      slBufferSize   = (slBufferSize * 4) / 3;
    }
    else if (8 == WavTechData.bits_per_sample)
    {
      slBufferSize   = (slBufferSize * 2);
    }
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
                 "getTrackMaxBufferSizeDB value for 24/8bit PCM clips is %ld",
                 slBufferSize);
  }
  return slBufferSize;
}

/* ======================================================================
FUNCTION:
  WAVFile::peekCurSample

DESCRIPTION:
  gets information about current sample of a track

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
PARSER_ERRORTYPE WAVFile::peekCurSample(uint32 /*trackid*/,
                                        file_sample_info_type *pSampleInfo)
{
  PARSER_ERRORTYPE ret = PARSER_ErrorDefault;
  if(m_pwavformatParser)
  {
    *pSampleInfo = m_audsampleinfo;
    ret = PARSER_ErrorNone;
  }
  return ret;
}
/* ======================================================================
FUNCTION:
  WAVFile::getTrackOTIType

DESCRIPTION:
  Retrieves the information regarding the audio track to determine the audio codec.

INPUT/OUTPUT PARAMETERS:
  None.

RETURN VALUE:
 none

SIDE EFFECTS:
  None.
======================================================================*/
uint8  WAVFile::getTrackOTIType(uint32)
{
  uint8 format = 0xff;
  if(m_pwavformatParser)
  {
    tech_data_wav tech_data;
    if(m_pwavformatParser->GetFormatChunk(&tech_data) == PARSER_ErrorNone)
    {
      switch(tech_data.format)
      {
        //FOR IMA_ADPCM, we output PCM, so mark audio as PCM_AUDIO
        case WAV_PCM:
        case WAV_MULTICHANNEL_PCM:
        case WAV_IMA_ADPCM:
        {
          format = PCM_AUDIO;
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW,"WAVFile::getTrackOTIType PCM_AUDIO");
        }
        break;
        case WAV_ALAW:
        {
          format = G711_ALAW_AUDIO;
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW,"WAVFile::getTrackOTIType G711_ALAW_AUDIO");
        }
        break;
        case WAV_ULAW:
        {
          format = G711_MULAW_AUDIO;
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW,"WAVFile::getTrackOTIType G711_MULAW_AUDIO");
        }
        break;
        case WAV_G_721:
        {
          format = G721_AUDIO;
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW,"WAVFile::getTrackOTIType G721_AUDIO");
        }
        break;
        case WAV_G_723:
        {
          format = G723_AUDIO;
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW,"WAVFile::getTrackOTIType G723_AUDIO");
        }
        break;
        case WAV_GSM:
        {
          format = GSM_FR_AUDIO;
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW,"WAVFile::getTrackOTIType GSM_FR_AUDIO");
        }
        break;
      }
    }
  }
  return format;
}

/* ======================================================================
FUNCTION:
  WAVFile::getTrackDecoderSpecificInfoContent

DESCRIPTION:
  gets decoder specific information.

INPUT/OUTPUT PARAMETERS:
 id: Track ID
 buf: Output buffer, decoder information will be written into this buffer
 pbufSize: size of decoder specific information

RETURN VALUE:
 Success or Fail

SIDE EFFECTS:
  None.
======================================================================*/
PARSER_ERRORTYPE WAVFile::getTrackDecoderSpecificInfoContent(uint32 /*id*/,
                                                             uint8* buf,
                                                             uint32 *pbufSize)
{
  if(m_pwavformatParser)
  {
    if(buf != NULL)
    {
      m_pwavformatParser->GetFormatChunk((tech_data_wav*)buf);
    }
    *pbufSize = (uint32)sizeof(tech_data_wav);
    return PARSER_ErrorNone;
  }
  return PARSER_ErrorDefault;
}
/* ======================================================================
FUNCTION:
  SetConfiguration

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
FileSourceStatus WAVFile::SetConfiguration(FileSourceConfigItemEnum henum)
{
  FileSourceStatus eStatus = FILE_SOURCE_FAIL;

  if((FILE_SOURCE_MEDIA_ENABLE_PCM_SAMPLE_UPGRADE  == henum) ||
     (FILE_SOURCE_MEDIA_DISABLE_PCM_SAMPLE_UPGRADE == henum) )
  {
    m_eConfigItem = henum;
    eStatus       = FILE_SOURCE_SUCCESS;
  }
  return eStatus;
}

#endif //FEATURE_FILESOURCE_WAVADPCM

