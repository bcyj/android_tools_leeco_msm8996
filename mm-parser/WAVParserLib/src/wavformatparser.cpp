/* =======================================================================
                              wavformatparser.cpp
DESCRIPTION
  Defines the functions to parse wav wav  format files
Copyright (c) 2009-2015 QUALCOMM Technologies Inc, All Rights Reserved.
QUALCOMM Technologies Proprietary and Confidential.

$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/WAVParserLib/main/latest/src/wavformatparser.cpp#37 $
$DateTime: 2014/02/07 02:53:30 $
$Change: 5240686 $
=============================================================================*/

//=============================================================================
//   INCLUDES AND VARIABLE DEFINITIONS
//=============================================================================
#include "wavformatparser.h"
#include "filebase.h"

#define G711_FRAME_BOUNDARY 80
#ifdef FEATURE_FILESOURCE_WAVADPCM
//=============================================================================
// Forward Declaration
//=============================================================================
class WAVfile;

//=============================================================================
// GLOBALS
//=============================================================================


//=============================================================================
// CONSTANTS
//=============================================================================

//=============================================================================
// FUNCTION: Constructor
//
// DESCRIPTION:
//  Create an instance of wavformatParser, and init the class attributes
//
// PARAMETERS
// >>pUData : Context data
// >>fsize : size of the file
//
//
// RETURN:
// None
//
// SIDE EFFECTS
// None
//=============================================================================
//
wavformatParser::wavformatParser(void* pUserData,
                                 uint64 fsize,
                                 OSCL_FILE *FilePtr)
{
  m_pUserData = pUserData;
  m_BytesAfterDecode = 0;
  m_nFileSize = fsize;
  m_CurrentParserState = PARSER_IDLE;
  m_WAVFilePtr = FilePtr;
  m_nCurrOffset = 0;
  m_duration = 0;
  m_maxBufferSize = 0;
  m_nCurrStartTime = 0;
  m_wav_format = WAV_UNKNOWN;
  (void) memset(m_ReadBuffer,0, sizeof(uint8)*WAV_READ_BUFFER_SIZE);
  (void) memset(&m_audio_track,0, sizeof(AudioTrack));
  (void) memset(&m_wav_header_wavh,0,sizeof(wav_header_wavh));
  (void) memset(&m_wav_audio_info,0,sizeof(wav_audio_info));
  (void) memset(&m_header_info,0,sizeof(m_header_info));
  (void) memset(&m_ima_adpcm_data,0,sizeof(ima_adpcm_data));
  (void) memset(&m_wav_tech_metadata,0,sizeof(tech_data_wav));
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
wavformatParser::~wavformatParser()
{
}



//=============================================================================
// FUNCTION: little_endian
//
// DESCRIPTION:
//  Converts little endian value to big endian
//
// PARAMETERS
//  uint16 value
//
// RETURN:
//  uint16 value
//=============================================================================
//
static uint16 little_endian (uint16 value)
{
  const uint8 * const p_value = reinterpret_cast<const uint8 *> (&value);
  return uint16((p_value[0]) | ( (p_value[1]) << 8) );
}

//=============================================================================
// FUNCTION: little_endian
//
// DESCRIPTION:
//  Converts little endian value to big endian
//
// PARAMETERS
//  uint16 value
//
// RETURN:
//  uint16 value
//=============================================================================
//
static uint32 little_endian (uint32 value)
{
  const uint8 * const p_value = reinterpret_cast<const uint8 *> (&value);
  return (static_cast<uint32> (p_value[0])
      | (static_cast<uint32> (p_value[1]) << 8)
      | (static_cast<uint32> (p_value[2]) << 16)
      | (static_cast<uint32> (p_value[3]) << 24));
}

//=============================================================================
// FUNCTION: StartParsing
//
// DESCRIPTION:
//  Starts the file parsing
//
// PARAMETERS
//  None
//
// RETURN:
//  wavErrorType
//
// SIDE EFFECTS
//  None
//=============================================================================
//
PARSER_ERRORTYPE wavformatParser::StartParsing(void)
{
  PARSER_ERRORTYPE status = PARSER_ErrorStreamCorrupt;
  if(PARSER_ErrorNone == parse_file_header())
  {
    parse_wav_file_header();
    parse_wav_audio_data();
    m_nCurrOffset += m_audio_track.start;
    status = PARSER_ErrorNone;
  }
  return status;
}

//=============================================================================
// FUNCTION: parse_wav_file_header
//
// DESCRIPTION:
//  parse wav file header
//
// PARAMETERS
//  wav_header_wavh - pointer to the wav header structure
//
// RETURN:
//  None
//
// SIDE EFFECTS
//  None
//=============================================================================
//

void wavformatParser::parse_wav_file_header(){

  m_wav_header_wavh.nSampleRate = m_header_info.sample_rate;

}


//=============================================================================
// FUNCTION: parse_wav_audio_data
//
// DESCRIPTION:
//  parse wav audio data information
//
// PARAMETERS
//  m_wav_audio_info - pointer to the wav audio info structure
//
// RETURN:
//  None
//
// SIDE EFFECTS
//  None
//=============================================================================
//

void wavformatParser::parse_wav_audio_data(){

  m_wav_audio_info.dwSuggestedBufferSize = (uint32)m_maxBufferSize;
  m_wav_audio_info.nBlockSize = m_wav_tech_metadata.block_align;

}

//=============================================================================
// FUNCTION: GetWAVHeader
//
// DESCRIPTION:
//  Parse the wav file header
//
// PARAMETERS
// <>pWavHdrPtr - pointer to wav header structure
//
// RETURN:
//  wavErrorType
//
// SIDE EFFECTS
//  None
//=============================================================================
//

PARSER_ERRORTYPE wavformatParser::GetWAVHeader(wav_header_wavh* pWavHdrPtr)
{
  PARSER_ERRORTYPE retError = PARSER_ErrorNone;

  if(!pWavHdrPtr)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
               "GetWAVHeader WAV_INVALID_USER_DATA");
    retError = PARSER_ErrorInvalidParam;
    return retError;
  }
  if(NULL == &m_wav_header_wavh)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
       "GetWAVHeader,NULL WAV Header,WAV_PARSE_ERROR");
    retError = PARSER_ErrorDefault;
    return retError;
  }
  pWavHdrPtr->nChannels = m_wav_tech_metadata.channels;
  pWavHdrPtr->nSampleRate = m_wav_tech_metadata.sample_rate;
  return retError;
}

//=============================================================================
// FUNCTION: GetAudioInfo
//
// DESCRIPTION:
// This function returns audio format specific information
//
// PARAMETERS
// <>pAudioInfo - pointer to the wav_audio_info structure
//
// RETURN:
//  wavErrorType
//
// SIDE EFFECTS
//  None
//=============================================================================
//
PARSER_ERRORTYPE wavformatParser::GetAudioInfo(wav_audio_info* pAudioInfo){

  PARSER_ERRORTYPE retError = PARSER_ErrorNone;

  if(NULL == &m_wav_audio_info)
  {
    retError = PARSER_ErrorDefault;
    return retError;
  }
  if(NULL == pAudioInfo)
  {
    retError = PARSER_ErrorInvalidParam;
    return retError;
  }
  (void) memset(pAudioInfo,0,sizeof(wav_audio_info));
  (void) memcpy(pAudioInfo,&m_wav_audio_info,sizeof(wav_audio_info));
  /* Currently parser providing max size based on the size required to store
     1second worth of data, now made changes to provide only 100ms worth of
     data. */
  if (m_wav_tech_metadata.bytes_rate)
  {
    pAudioInfo->dwSuggestedBufferSize = m_wav_tech_metadata.bytes_rate/10;
  }
  else
  {
    pAudioInfo->dwSuggestedBufferSize = (m_wav_tech_metadata.sample_rate *
                                        m_wav_tech_metadata.channels) *
                                       (m_wav_tech_metadata.bits_per_sample/8);
    pAudioInfo->dwSuggestedBufferSize /= 10;
  }
#ifdef _ANDROID_
  //! In LA, this value is hardcoded to 8192 always.
  pAudioInfo->dwSuggestedBufferSize = 8192;
#endif
  if((m_wav_tech_metadata.format == WAV_IMA_ADPCM) &&
     (pAudioInfo->dwSuggestedBufferSize < m_maxBufferSize))
  {
    pAudioInfo->dwSuggestedBufferSize = m_maxBufferSize;
  }
  return retError;
}

//=============================================================================
// FUNCTION: GetClipDurationInMsec
//
// DESCRIPTION:
//  This function gives the total duration of a file
//
// PARAMETERS
//  None
//
// RETURN:
//  uint64 - file duration in ms
//
// SIDE EFFECTS
//  None
//=============================================================================
//
uint64 wavformatParser::GetClipDurationInMsec()
{
  if (0 == m_duration)
  {
    (void)get_duration(&m_duration);
  }
  return m_duration;
}

//=============================================================================
// FUNCTION: GetCurrentSample
//
// DESCRIPTION:
//  This function returns the current sample data
//
// PARAMETERS
// <>dataBuffer - pointer to the data sample
// >>nMaxBufSize - maximum buffer size
// >>nBytesNeeded - no of bytes needed

// RETURN:
//  wavErrorType
//
// SIDE EFFECTS
//  None
//=============================================================================
//
PARSER_ERRORTYPE wavformatParser::GetCurrentSample(uint8* dataBuffer,
                                                   uint32 nMaxBufSize,
                                                   uint32 *nBytesNeeded)
{
  PARSER_ERRORTYPE  retType = PARSER_ErrorNone;
  uint32 nBytesRead = 0;

  //Validate input parameters
  if( (!dataBuffer) || (!nMaxBufSize) ||  (!nBytesNeeded) )
  {
    MM_MSG_PRIO(MM_FILE_OPS,MM_PRIO_FATAL,"GetCurrentSample WAV_INVALID_USER_DATA");
    return PARSER_ErrorInvalidParam;
  }
  uint32 nNumBytesRequest = *nBytesNeeded;
  *nBytesNeeded = 0;
  /*
  * Check for EOF.
  * Under normal scenario, m_nCurrOffset will never be > m_nFileSize,
  * but to avoid read failure, check for >=
  */
  if( (m_nCurrOffset >= m_nFileSize) && (retType == PARSER_ErrorNone) )
  {
    MM_MSG_PRIO(MM_FILE_OPS,MM_PRIO_HIGH,"GetCurrentSample encountered EOF");
    *nBytesNeeded = 0;
    retType = PARSER_ErrorEndOfFile;
  }
  /*
  *If number of bytes left in audio chunk < the amount of data being asked,
  *adjust number of bytes to read.
  */
  else if( ( (m_audio_track.end - m_nCurrOffset) < nNumBytesRequest) &&
           (retType == PARSER_ErrorNone) )
  {
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
                 "GetCurrentSample nNumBytesRequest %lu", nNumBytesRequest);
    if((m_audio_track.end - m_nCurrOffset) >= m_wav_tech_metadata.block_align)
    {
      if(m_wav_tech_metadata.format!= WAV_IMA_ADPCM)
      {
        if(m_wav_tech_metadata.block_align)
        {
          nNumBytesRequest = (uint32)(m_audio_track.end - m_nCurrOffset) -
                           ((uint32)(m_audio_track.end - m_nCurrOffset) % m_wav_tech_metadata.block_align);
        }
        else
        {
          nNumBytesRequest = (uint32)(m_audio_track.end - m_nCurrOffset);
        }
      }
      else
      {
        nNumBytesRequest = m_wav_tech_metadata.block_align;
      }
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW,
                 "GetCurrentSample m_nCurrOffset %llu ",m_nCurrOffset);
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW,
                   "GetCurrentSample Updated nNumBytesRequest %lu",
                   nNumBytesRequest);
    }
    else
    {
      MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_MEDIUM,
                  "GetCurrentSample @ EOF m_nCurrOffset %llu m_nFileSize %llu",
                  m_nCurrOffset, m_nFileSize);
      nNumBytesRequest = 0;
      *nBytesNeeded = 0;
      retType = PARSER_ErrorEndOfFile;
    }
  }

  if((m_nCurrOffset >= m_audio_track.start) &&
     (m_wav_tech_metadata.channels) &&
     (m_wav_tech_metadata.sample_rate) &&
     (m_wav_tech_metadata.bits_per_sample))
  {
    uint64 ullRelativeOffset = m_nCurrOffset - m_audio_track.start;
    m_nCurrStartTime = (uint64)((MILLISEC_TIMESCALE_UNIT * ullRelativeOffset*8) /
                       (m_wav_tech_metadata.channels *
                        m_wav_tech_metadata.sample_rate *
                       (m_wav_tech_metadata.bits_per_sample)) );
  }
  if(retType == PARSER_ErrorNone)
  {
    MM_MSG_PRIO1(MM_FILE_OPS,MM_PRIO_LOW,
                 "GetCurrentSample m_nCurrOffset to read %llu", m_nCurrOffset);
    nBytesRead = WAVCallbakGetData (m_nCurrOffset, nNumBytesRequest,
                                    dataBuffer, nMaxBufSize, m_pUserData );
    if(!(nBytesRead))
    {
      //Read failure will trigger end of track from upper layers.
      m_CurrentParserState = PARSER_READ_FAILED;
      *nBytesNeeded  = 0;
      MM_MSG_PRIO(MM_FILE_OPS,MM_PRIO_FATAL,"GetCurrentSample WAV_READ_FAILURE");
      retType = PARSER_ErrorReadFail;
    }
    else
    {
      m_nCurrOffset += nBytesRead;
      switch(m_wav_tech_metadata.format)
      {
        case WAV_IMA_ADPCM:
          *nBytesNeeded = m_BytesAfterDecode;
        break;
        case WAV_PCM:
        case WAV_MULTICHANNEL_PCM:
        case WAV_ALAW:
        case WAV_ULAW:
        case WAV_GSM:
          *nBytesNeeded = nBytesRead;
        break;
        default:
          *nBytesNeeded = 0;
      }
      m_CurrentParserState = PARSER_READY;
    }
  }//if(retType == PARSER_ErrorNone)
  return retType;
}


//=============================================================================
// FUNCTION: get_wav_subtype
//
// DESCRIPTION:
//  This function returns the sub type of wav format
//
// PARAMETERS
//<> m_wavformat - pointer to wav format sub type
//
// RETURN:
//  None
//
// SIDE EFFECTS
//  None
//=============================================================================

void wavformatParser::get_wav_subtype(uint32 *m_wavformat)
{
  *m_wavformat = m_wav_tech_metadata.format;
}


//=============================================================================
// FUNCTION: ActualBytesDecoded
//
// DESCRIPTION:
//  This function updates the no of bytes decoded
//
// PARAMETERS
//>> t_DecodeBytes - Total bytes decoded
//
// RETURN:
//  None
//
// SIDE EFFECTS
//  None
//=============================================================================

void wavformatParser::ActualBytesDecoded(uint32 t_DecodeBytes)
{
  m_BytesAfterDecode = t_DecodeBytes;
}
//=============================================================================
// FUNCTION: init_file_position
//
// DESCRIPTION:
//  This function initialise the file offset to reset position
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

void wavformatParser::init_file_position()
{
  m_nCurrOffset = 0;
  m_nCurrOffset += m_audio_track.start;
}

//=============================================================================
// FUNCTION: set_newfile_position
//
// DESCRIPTION:
//
// PARAMETERS
// >>file_position - new file position
//
// RETURN:
//  None
//
// SIDE EFFECTS
//  None
//=============================================================================
void wavformatParser::set_newfile_position(::uint64 file_position)
{
  m_nCurrOffset = (uint64) file_position;
}


//=============================================================================
// FUNCTION : get_duration
//
// DESCRIPTION
//  Calculates Total Playback duration from data found by init_parser.
//  This function is only implemented when the seek_function_defined is set to TRUE
//
// PARAMETERS
//  time  Total playback time for the audio content in milliseconds
//
// RETURN VALUE
//  WAV_SUCCESS Function succesfully calculated the playback duration
//  WAV_UNSUPPORTED: Function is not implemented for this audio format
//  WAV_UNKNOWN_ERROR: Data not available to perform the calculation
//
// SIDE EFFECTS
//  None
//=============================================================================
//
PARSER_ERRORTYPE  wavformatParser::get_duration (/*rout*/ ::uint64* time)
{
  PARSER_ERRORTYPE result = PARSER_ErrorNone;

  if (WAV_UNKNOWN == m_wav_format)
  {
    //parse_file_header API is not called yet -
    //get_duration API can be called only
    //after wave format parser successfully parses the file header
    MM_MSG_PRIO(MM_FILE_OPS,MM_PRIO_FATAL,
                   "wavformatparser::get_duration:Bad state, wav format not set!");
    result = PARSER_ErrorUnknownCodecType;
    return result;
  }
  else
  {
    uint16 bitspersample = m_wav_tech_metadata.bits_per_sample;
    uint64 playback_time = 0;
    if ((0 == m_wav_tech_metadata.channels) ||
        (0 == m_wav_tech_metadata.sample_rate) ||
        (0 == bitspersample))
    {
      result = PARSER_ErrorStreamCorrupt;
      return result;
    }
    playback_time = (m_audio_track.size * MILLISECONDS);
    if (bitspersample)
    {
      playback_time = (playback_time * 8) /
                      (m_wav_tech_metadata.channels *
                       m_wav_tech_metadata.sample_rate* bitspersample);
    }
    else
    {
      playback_time = playback_time /
                      (m_wav_tech_metadata.channels *
                       m_wav_tech_metadata.bytes_rate);
    }
    *time = playback_time;
  }
  return result;
}

//=============================================================================
// FUNCTION : get_seek_position
//
// DESCRIPTION
//  Calculates seek position based on time. This function is only implemented
//  when the seek_function_defined is set to TRUE
//
// PARAMETERS
//  time  Presentation time to seek to
//  file_position  File position corresponding to the seek time provided
//
// RETURN VALUE
//  WAV_SUCCESS: Function succesfully calculated the playback duration
//  WAV_UNSUPPORTED: Function is not implemented for this audio format
//  WAV_UNKNOWN_ERROR: Data not available to perform the calculation
//
// SIDE EFFECTS
//  None
//=============================================================================
//
PARSER_ERRORTYPE  wavformatParser::get_seek_position (/*in*/ ::uint64 time,
                                                      /*rout*/ ::uint32* file_position)
{
  (void)time;

  PARSER_ERRORTYPE result = PARSER_ErrorNone;

  if (WAV_UNKNOWN == m_wav_format)
  {
    MM_MSG_PRIO(MM_FILE_OPS,MM_PRIO_FATAL,
                   "wavformatparser::get_seek_position: Bad state, wav format not set!");
    result = PARSER_ErrorUnknownCodecType;
    return result;
  }
  else
  {
    if (m_wav_tech_metadata.type == AUDIO_WAVE)
    {
      uint32 playback_position = 0;
      // If bits_per_sample is valid then use it to calculate pb_position otherwise use
      // bytes_rate to calculate pb_position. In GSM-FR clip rate is fixed so encoded
      // bits_per_sample will be 0. If bits_par_sample value is '0' then following
      // calculation used to calculate pb_position.
      // As bytes_rate * 8 = bit_rate = sample_rate * bits_per_sample
      // pb_position = (time(ms) * no_channels * bytes_rate)/1000;
      if(m_wav_tech_metadata.bits_per_sample)
      {
        playback_position = (uint32)((
          time * m_wav_tech_metadata.channels *
          m_wav_tech_metadata.sample_rate *
          m_wav_tech_metadata.bits_per_sample) /
          (8 * MILLISECONDS));
      }
      else
      {
        playback_position = (uint32)((
          time * m_wav_tech_metadata.channels *
          m_wav_tech_metadata.bytes_rate) /(MILLISECONDS));
      }

      if( (m_wav_tech_metadata.format == WAV_ALAW)||
          (m_wav_tech_metadata.format == WAV_ULAW) )
      {
        //adjust position to multiple of 80 bytes for a-law/mu-law
        if((playback_position % G711_FRAME_BOUNDARY) != 0)
        {
          MM_MSG_PRIO1(MM_FILE_OPS,MM_PRIO_MEDIUM,
               "update playback pos on g711 frame boundary, offset update %lu",
               playback_position);
          playback_position +=
            (G711_FRAME_BOUNDARY - (playback_position % G711_FRAME_BOUNDARY)) ;
          MM_MSG_PRIO1(MM_FILE_OPS,MM_PRIO_MEDIUM,
                       "updated playback_position on g711 frame boundary %lu",
                       playback_position);
        }
      }
      else if(m_wav_tech_metadata.format == WAV_GSM)
      {
        if(m_wav_tech_metadata.block_align)
        {
          if((playback_position % m_wav_tech_metadata.block_align) != 0)
          {
            MM_MSG_PRIO1(MM_FILE_OPS,MM_PRIO_MEDIUM,
                   "updating playback_position on block align for gsm-fr, file posn update %lu",
                   playback_position);
            playback_position +=
            (m_wav_tech_metadata.block_align - (playback_position % m_wav_tech_metadata.block_align)) ;
            MM_MSG_PRIO1(MM_FILE_OPS,MM_PRIO_MEDIUM,
                     "updated playback_position on block align for gsm-fr %lu",
                     playback_position);
          }
        }
      }
      //check if position is word aligned
      else if ((playback_position % 2) != 0)
      {
        playback_position -= 1;
      }
      *file_position = playback_position;
    }
    else
    {
      MM_MSG_PRIO(MM_FILE_OPS,MM_PRIO_FATAL,
                     "wavformatparser::get_seek_position: Bad state, wav format not set!");
      result = PARSER_ErrorUnsupportedCodecType;
    }
  }
  return result;
}
//=============================================================================
// FUNCTION: RandomAccessDenied
//
// DESCRIPTION:
//Returns non zero if seek is not supported by the parser
//
// PARAMETERS
//
// RETURN:
//
// SIDE EFFECTS
//  None
//=============================================================================
//
uint8 wavformatParser::RandomAccessDenied()
{
  uint8 seekDenied = 0;
  // In case of GSM_FR audio format bit-rate is fix(FR-> Fix Rate), so encoder
  // puts bits_per_sample value zero always. So random access denied will be
  // decided based on bytes_rate & bits_per_sample
  if( ( m_wav_tech_metadata.bytes_rate == 0 )&& (m_wav_tech_metadata.bits_per_sample == 0 ))
  {
    seekDenied = 1;
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW,"Random Access Denied %d",seekDenied);
  }
  return seekDenied;
}
//=============================================================================
// FUNCTION: Seek
//
// DESCRIPTION:
//  This function will return the corresponding new file position for the given input time
//
// PARAMETERS
// <>nReposTime - time to seek
//
// RETURN:
//  wavErrorType
//
// SIDE EFFECTS
//  None
//=============================================================================
//
uint64 wavformatParser::Seek(uint64 nReposTime, uint32 *t_dataoffset)
{
  ::uint64 time = nReposTime;
  ::uint32 file_position;
  uint32 valid_data_offset = 0;
  (void) get_seek_position ( time, &file_position);
  if(m_wav_tech_metadata.format == WAV_IMA_ADPCM )
  {
    calc_redundant_pcm(&file_position, &valid_data_offset);
    *t_dataoffset = valid_data_offset;
  }
  file_position += (uint32)m_audio_track.start;
  set_newfile_position(file_position);
  return time;
}


//=============================================================================
// FUNCTION : calc_redundant_pcm
//
// DESCRIPTION
//  calculate the excess adpcm samples shall be decoded in decoder
//
// PARAMETERS
//  data_offset : size of the excess pcm samples
//  file_position:  File position corresponding to the seek time provided
// RETURN VALUE
//  An indicator of success or failure of the method is returned.
//
// SIDE EFFECTS
//  None
//
//=============================================================================

PARSER_ERRORTYPE wavformatParser::calc_redundant_pcm(::uint32* file_position,
                                                     ::uint32* data_offset)
{
  PARSER_ERRORTYPE result = PARSER_ErrorNone;
  uint32 playback_position = *file_position;
  if (playback_position%(m_wav_tech_metadata.block_align) != 0)
  {
    *data_offset = playback_position%(m_wav_tech_metadata.block_align);
    uint32 temp_file_pos;
    temp_file_pos = playback_position/(uint32)(m_wav_tech_metadata.block_align);
    temp_file_pos *= (uint32)(m_wav_tech_metadata.block_align);
    *file_position = temp_file_pos;
  }
  return result;
}



//=============================================================================
// FUNCTION : read_riff_chunk
//
// DESCRIPTION
//  verifies RIFF chunk ina  wave file
//
// PARAMETERS
//  file : IFilePort interface to data to be parsed.
//  chunk_size : size of the RIFF chunk (includes all sub chunks)
//
// RETURN VALUE
//  An indicator of success or failure of the method is returned.
//
// SIDE EFFECTS
//  None
//
//=============================================================================
//
PARSER_ERRORTYPE wavformatParser::read_riff_chunk (uint32 *chunk_size)
{
  PARSER_ERRORTYPE result = PARSER_ErrorNone;

  uint8 buffer[WAV_FILE_RIFF_CHUNK_SIZE] = {0};
  int buf_len = WAV_FILE_RIFF_CHUNK_SIZE;
  uint32 offset = 0;
  uint32 bytes_read = 0;
  if (PARSER_ErrorNone != OSCL_FileSeek(m_WAVFilePtr,0,SEEK_SET))
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                  "wavformatParser::read_riff_chunk:could not seek to required position!");
    result = PARSER_ErrorReadFail;
    return result;
  }

  bytes_read = OSCL_FileRead(buffer,buf_len,1,m_WAVFilePtr);
  if (bytes_read != WAV_FILE_RIFF_CHUNK_SIZE)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                  "wavformatParser::read_riff_chunk:could not read from IFileport1!");
    result = PARSER_ErrorReadFail;
    return result;
  }

  //check for "RIFF" ChunkID
  if (strncmp ((char *)buffer, "RIFF", WAV_FIELD_OFFSET))
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                  "wavformatParser::read_riff_chunk: no 'RIFF' ID not wave chunk");
    result = PARSER_ErrorStreamCorrupt;
    return result;
  }

  offset += WAV_FIELD_OFFSET;
  // Read RIFF chunk size.  If it indicates RIFF is smaller than the
  // actual file size, overwrite the file size.
  uint32 riff_size;
  (void) memmove(&riff_size, buffer + offset, WAV_FIELD_OFFSET);
  riff_size = little_endian (riff_size);

  offset += WAV_FIELD_OFFSET;

  if (strncmp ((char *)buffer + offset, "WAVE", WAV_FIELD_OFFSET))
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                   "wavformatParser::read_riff_chunk: no 'WAVE'tag,not wave chunk");
    result = PARSER_ErrorInHeaderParsing;
    return result;
  }
  *chunk_size = riff_size;
  return result;
}


//=============================================================================
// FUNCTION : read_fmt_chunk
//
// DESCRIPTION
//  reads "fmt" chunk in a wave file and retrieves meta data
//
// PARAMETERS
//
// <>chunk_size : size of the fmt chunk
//
// RETURN VALUE
//  An indicator of success or failure of the method is returned.
//
// SIDE EFFECTS
//  None
//
//=============================================================================
//
PARSER_ERRORTYPE wavformatParser::read_fmt_chunk (uint32 fmt_size)
{
  PARSER_ERRORTYPE result = PARSER_ErrorNone;
  uint32 bytes_read = 0;

  byte header_buffer[WAV_CHUNK_HEADER_SIZE] = {0};
  uint32 offset = 0;

  byte * buffer = NULL;
  if( (fmt_size <= m_nFileSize)&& (fmt_size) )
  {
    buffer = (uint8 *) MM_Malloc(fmt_size*sizeof(uint8));
    if (NULL == buffer)
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                    "wavformatParser::read_fmt_chunk: not enough data available!");
      result = PARSER_ErrorMemAllocFail;
      MM_Free(buffer);
      return result;
    }

    bytes_read = OSCL_FileRead(buffer,fmt_size,1,m_WAVFilePtr);
    if (bytes_read != fmt_size)
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                    "wavformatParser::read_fmt_chunk:could not read fmt size worth of data");
      result = PARSER_ErrorReadFail;
      MM_Free(buffer);
      return result;
    }

    m_wav_tech_metadata.type = AUDIO_WAVE;

    const uint16 format_tag = little_endian
         (* reinterpret_cast<const uint16 *> (buffer));

    offset = 2;

    m_wav_format = (wav_format_type) format_tag;
    m_wav_tech_metadata.format = format_tag;

    m_wav_tech_metadata.channels =
        little_endian (* reinterpret_cast<const uint16 *>(buffer + offset));
    offset += 2;

    m_wav_tech_metadata.sample_rate =
        little_endian (* reinterpret_cast<const uint32 *>(buffer + offset));
    offset += WAV_FIELD_OFFSET;

    m_wav_tech_metadata.bytes_rate =
        little_endian (* reinterpret_cast<const uint32 *>(buffer + offset));
    offset += WAV_FIELD_OFFSET ;

    m_wav_tech_metadata.block_align =
        little_endian (* reinterpret_cast<const uint16 *>(buffer + offset));
    offset += 2;
    m_wav_tech_metadata.bits_per_sample =
        little_endian (* reinterpret_cast<const uint16 *>(buffer + offset));

    if (m_wav_format == WAV_IMA_ADPCM)
    {
      m_wav_tech_metadata.bits_per_sample = 16;
    }

    if(m_wav_tech_metadata.format == WAV_PCM_WAVFORMATEXTENSIBLE)
    {
      offset += 6;//(ValidBitsPerSample+wSamplesPerBlock+wReserved)
      m_wav_tech_metadata.channel_mask =
          little_endian (* reinterpret_cast<const uint32 *>(buffer + offset));
    }


    if(m_wav_tech_metadata.format == WAV_IMA_ADPCM)
    {
      offset += 2;
      //Read extra bytes
      uint16 extra_bytes_size = (* reinterpret_cast<const uint16 *>(buffer + offset));
      offset += 2;
      if(extra_bytes_size == 2)
      {
        m_wav_tech_metadata.format_data.ima_adpcm.format = WAV_IMA_ADPCM;
        m_wav_tech_metadata.format_data.ima_adpcm.samples_per_block =
        little_endian (* reinterpret_cast<const uint16 *>(buffer + offset));
        offset += 2;
      }
    }
    MM_Free(buffer);
  }//if( (fmt_size <= m_nFileSize)&& (fmt_size) )
  else
  {
    MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_FATAL," fmt_size %lu > m_nFileSize %llu",
                 fmt_size, m_nFileSize);
    result = PARSER_ErrorStreamCorrupt;
  }
  return result;
}

//=============================================================================
// FUNCTION : parse_file_header
//
// DESCRIPTION
//  Parses the file looking for file headers and other non-audio metadata.
//  Determines the start and stop file position of audio data.
//  Does not parse the audio data, just locates it.
//
// PARAMETERS
//  None
//
// RETURN VALUE
//  An indicator of success or failure of the method is returned.
//
// SIDE EFFECTS
//  None
//
//=============================================================================
//

PARSER_ERRORTYPE wavformatParser::parse_file_header ()
{
  PARSER_ERRORTYPE result = PARSER_ErrorNone;

  // Validate input parameters
  if(NULL == m_WAVFilePtr)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                   "wavformatParser::parse_file_header: file parameter was NULL");
  }

  m_wav_format = WAV_UNKNOWN;

  //save the file size
  uint64 file_length = m_nFileSize;

  //start parsing the file header
  uint32 offset = 0;
  uint32 chunk_size = 0;
  byte header_buffer[WAV_CHUNK_HEADER_SIZE] = {0};

  //check RIFF chunk first
  if (PARSER_ErrorNone != (result = read_riff_chunk(&chunk_size)))
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                   "wavformatParser::parse_file_header: not a wave chunk");
    return result;
  }

  offset += WAV_FILE_RIFF_CHUNK_SIZE;

  while(offset+WAV_CHUNK_HEADER_SIZE <= m_nFileSize)
  {
   //read chunk, fill in tech meta data
    if (PARSER_ErrorNone != OSCL_FileSeek(m_WAVFilePtr,offset,SEEK_SET))
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
              "wavformatParser::parse_file_header:could not seek to required pos!");
      result = PARSER_ErrorReadFail;
      return result;
    }
    if (OSCL_FileRead(header_buffer,WAV_CHUNK_HEADER_SIZE,1,m_WAVFilePtr) != WAV_CHUNK_HEADER_SIZE)
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
              "wavformatParser::parse_file_header: could not read WAV_CHUNK_HEADER_SIZE");
      result = PARSER_ErrorReadFail;
      return result;
    }
    //Reading chunk,check for ChunkID
    //This while is used to skip unuseful list or chunk if there are more than one list or chunk
    while ((strncmp ((char*)header_buffer, "fmt", WAV_FIELD_OFFSET-1))&&
            (strncmp ((char*)header_buffer, "fact", WAV_FIELD_OFFSET))&&
            (strncmp ((char*)header_buffer, "PAD", WAV_FIELD_OFFSET-1))&&
            (strncmp ((char*)header_buffer, "data", WAV_FIELD_OFFSET))&&
            (offset+WAV_CHUNK_HEADER_SIZE <= m_nFileSize))
    {
      uint32 list_size;
      (void) memmove(&list_size, header_buffer+ WAV_FIELD_OFFSET, WAV_FIELD_OFFSET);
      list_size = little_endian (list_size);
      //update offset to skip unused list or chunk
      offset += WAV_FIELD_OFFSET;
      offset += WAV_FIELD_OFFSET;
      offset += list_size;
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
              "wavformatParser::unknown list_size %u",list_size);

      if( PARSER_ErrorNone != OSCL_FileSeek(m_WAVFilePtr,offset,SEEK_SET))
      {
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                "wavformatParser::parse_file_header: could not seek to required position!");
        result = PARSER_ErrorReadFail;
        return result;
      }
      if (OSCL_FileRead(header_buffer,WAV_CHUNK_HEADER_SIZE,1,m_WAVFilePtr)
              != WAV_CHUNK_HEADER_SIZE)
      {
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                "wavformatParser::parse_file_header: could not read WAV_CHUNK_HEADER_SIZE");
        result = PARSER_ErrorReadFail;
        return result;
      }
    }//end of while(strncmp)
    // Read "fmt" chunk size. seek and read actual "fmt" contents
    if (!strncmp ((char*)header_buffer, "fmt", WAV_FIELD_OFFSET-1))
    {
      uint32 fmt_size;
      (void) memmove(&fmt_size, header_buffer + WAV_FIELD_OFFSET,
                          WAV_FIELD_OFFSET);
      fmt_size = little_endian (fmt_size);
      offset += 2*WAV_FIELD_OFFSET;

      if( PARSER_ErrorNone != OSCL_FileSeek(m_WAVFilePtr,offset,SEEK_SET))
      {
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                "wavformatParser::parse_file_header: can not seek to fmt position!");
        result = PARSER_ErrorReadFail;
        return result;
      }
      if (PARSER_ErrorNone != (result = read_fmt_chunk(fmt_size)))
      {
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                "wavformatParser::parse_file_header: unable to get format for fmt");
        return result;
      }
      offset += fmt_size;
    }
    // Read "fact" chunk size.
    else if (!strncmp ((char*)header_buffer, "fact", WAV_FIELD_OFFSET))
    {
      uint32 fact_size;
      (void) memmove(&fact_size, header_buffer + WAV_FIELD_OFFSET, WAV_FIELD_OFFSET);
      fact_size = little_endian (fact_size);
      offset += 2*WAV_FIELD_OFFSET + fact_size;
    }
    // Read "PAD" chunk size,omit it for PCM
    else if (!strncmp ((char*)header_buffer, "PAD", WAV_FIELD_OFFSET-1))
    {
      uint32 pad_size;
      (void) memmove(&pad_size, header_buffer + WAV_FIELD_OFFSET, WAV_FIELD_OFFSET);
      pad_size = little_endian (pad_size);
      offset += 2*WAV_FIELD_OFFSET + pad_size;
    }
    //read "data" chunk, point to audio data
    else if (!strncmp ((char*)header_buffer, "data", WAV_FIELD_OFFSET))
    {
      uint32 chunk_data_size;
      (void) memmove(&chunk_data_size, header_buffer + WAV_FIELD_OFFSET, WAV_FIELD_OFFSET);
      chunk_size = little_endian (chunk_data_size) + WAV_CHUNK_HEADER_SIZE;
      break;
    }
  }//end while{}

  chunk_size -= WAV_CHUNK_HEADER_SIZE;

  //Set maximum size of buffer for wave format
  //set buffer size equivalent to nblockalign in adpcm formats
  if(m_wav_tech_metadata.format == WAV_IMA_ADPCM)
  {
    if(m_wav_tech_metadata.channels == 1)
    {
      m_maxBufferSize = (2*(m_wav_tech_metadata.format_data.ima_adpcm.samples_per_block));
    }
    else
    {
      m_maxBufferSize = (4*(m_wav_tech_metadata.format_data.ima_adpcm.samples_per_block));
    }
    if(!m_maxBufferSize)
    {
      if(m_wav_tech_metadata.channels == 1)
      {
        m_maxBufferSize = 2 * m_wav_tech_metadata.block_align;
      }
      else
      {
        m_maxBufferSize = 4 * m_wav_tech_metadata.block_align;
      }
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
                   "wave::parse_file_header: Use blockAlign %lu as buf size",
                   m_maxBufferSize);
    }
  }
  else
  {
    // calculating max buffer size based bytes_rate retrieve from format chunk.
    // bytes_rate =  sample_rate * no_channels * bite_par_sample/8;
    // This will always make sure that parser output both sample data properly
    // in case of stereo/mono WAV file.
    if(m_wav_tech_metadata.bytes_rate)
    {
      m_maxBufferSize = m_wav_tech_metadata.bytes_rate;
    }
    else
    {
      m_maxBufferSize = (m_wav_tech_metadata.sample_rate * m_wav_tech_metadata.channels)*
                        (m_wav_tech_metadata.bits_per_sample/8);
    }
#ifdef PLATFORM_LTK
    if( (m_wav_tech_metadata.format == WAV_GSM)&&
        (m_wav_tech_metadata.block_align) )
    {
      m_maxBufferSize = m_wav_tech_metadata.block_align * 4;
    }
#endif
  }

  if (m_maxBufferSize > chunk_size)
  {
    m_maxBufferSize = chunk_size;
  }

  if (chunk_size < file_length)
  {
    offset += WAV_CHUNK_HEADER_SIZE;
    m_audio_track.start = offset;
    m_audio_track.end = offset + chunk_size;
    m_audio_track.size = chunk_size;
  }
  else
  {
    result = PARSER_ErrorStreamCorrupt;
  }
  return result;
}

//=============================================================================
// FUNCTION : GetFormatChunk
//
// DESCRIPTION
//  Retrieves the format chunk that exists in the clip.
//
// PARAMETERS
//  @out:tech_data_wav: Format structure pointer to be filled in
//
// RETURN VALUE
//  WAV_SUCCESS if successful in retireving format chunk
//  else returns appropriate error type
//
// SIDE EFFECTS
//  None
//
//=============================================================================
//
PARSER_ERRORTYPE wavformatParser::GetFormatChunk(tech_data_wav* pFormat)
{
  PARSER_ERRORTYPE vRet = PARSER_ErrorDefault;
  if(!pFormat)
  {
    vRet = PARSER_ErrorInvalidParam;
  }
  else
  {
    memset(pFormat,0,sizeof(tech_data_wav));
    memcpy(pFormat,&m_wav_tech_metadata,sizeof(tech_data_wav));
    vRet = PARSER_ErrorNone;
  }
  return vRet;
}
#endif //FEATURE_FILESOURCE_WAVADPCM
