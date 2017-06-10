// -*- Mode: C++ -*-
//=============================================================================
// FILE: aacparser.cpp
//
// SERVICES: AUDIO
//
// DESCRIPTION: defines functions that parse AACParser files
//
// Copyright (c) 2009-2015 QUALCOMM Technologies Incorporated.
// All Rights Reserved.
// Qualcomm Technologies Proprietary and Confidential.

//$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/AACParserLib/main/latest/src/aacparser.cpp#77 $
//$DateTime: 2014/02/07 02:53:30 $
//$Change: 5240686 $

//=============================================================================
#include "parserdatadef.h"
#include "aacparser.h"
#include "seektable.h"
#include "filebase.h"
#include "seek.h"
#include "MMDebugMsg.h"
#include "MMMemory.h"

//=============================================================
// CONSTANTS
//=============================================================

//=============================================================
// FUNCTION DECLARATIONS
//=============================================================
//=============================================================================
// FUNCTION: Constructor
//
// DESCRIPTION:
//  Create an instance of aacparser, and init the class attributes
//
// PARAMETERS
//  pUserData : This is the interface to the component services environment
//  fsize : AAC_SUCCESS - success
//                 AAC_INVALID_PARM - failure
//
// RETURN:
//  None
//
// SIDE EFFECTS
//  None
//=============================================================================
//

aacParser::aacParser(void *pUserData,
                     uint64 fsize,
                     OSCL_FILE *FilePtr,
                     bool bHttpStreaming)
{
#ifdef FEATURE_FILESOURCE_AAC_PARSER_DEBUG
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"aacParser::aacParser");
#endif
  m_bHTTPStreaming = bHttpStreaming;
  m_pUserData = pUserData;
  m_nFileSize = fsize;
  m_CurrentParserState = PARSER_IDLE;
  m_AACFilePtr = FilePtr;
  m_nCurrOffset = 0;
  m_filereadpos = 0;
  pseek = NULL;
  psimple_seektable = NULL;
  m_aac_format = AAC_FORMAT_UNKNOWN;
  m_seek_function_defined = false;
  aac_file_format = false;
  m_id3tagfound = false;
  m_firstFrame = false;
  m_isFirstTSValid = false;
  m_firstFrameTS = 0;
  m_baseTS       = 0;
  m_SeekDone     = false;
  m_id3tagparsed = false;
  m_n_adif_hdr_len = 0;
  memset(&m_aac_header_aach,0,sizeof(aac_header_aach));
  memset(&m_aac_audio_info,0,sizeof(aac_audio_info));
  memset(&m_audio_track,0,sizeof(AudioTrack));
  memset(&m_ReadBuffer,0,AAC_READ_BUFFER_SIZE);
  m_aac_metadata = NULL;
  m_hFrameOutputModeEnum  = FILE_SOURCE_MEDIA_OUTPUT_BYTES_STREAM;
  m_hHeaderOutputModeEnum = FILE_SOURCE_MEDIA_RETAIN_AUDIO_HEADER;
  m_nCurrentTime = 0;
  m_nNextFrameTime = 0;
  m_bEndOfData = 0;
  m_aac_duration = 0;
  m_bFixedADTSHdrSet = false;
  m_bCRCPresent = false;
  memset(m_adts_fix_hdr,0,AAC_ADTS_FIX_HDR_SIZE);
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
aacParser::~aacParser()
{
#ifdef FEATURE_FILESOURCE_AAC_PARSER_DEBUG
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"aacParser::~aacParser");
#endif
  if(m_aac_metadata)
  {
    MM_Delete(m_aac_metadata);
    m_aac_metadata = NULL;
  }
  if(psimple_seektable)
  {
    MM_Delete( psimple_seektable);
    psimple_seektable = NULL;
  }
  if(pseek)
  {
    MM_Delete( pseek);
    pseek=NULL;
  }
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
//  aacErrorType
//
// SIDE EFFECTS
//  None
//=============================================================================
//
PARSER_ERRORTYPE aacParser::StartParsing(void)
{
#ifdef FEATURE_FILESOURCE_AAC_PARSER_DEBUG
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"aacParser::StartParsing");
#endif
  PARSER_ERRORTYPE retError = PARSER_ErrorStreamCorrupt;
  if(!m_pUserData)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
               "StartParsing AAC_INVALID_USER_DATA");
    return PARSER_ErrorDefault;
  }
  if(PARSER_ErrorNone == (retError = parse_file_header()))
  {
#ifdef FEATURE_FILESOURCE_AAC_PARSER_DEBUG
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"parse_file_header returned AAC_SUCCESS");
#endif
    parse_aac_file_header();
    parse_aac_audio_data();
    m_nCurrOffset += m_audio_track.start;
    retError = PARSER_ErrorNone;
  }
  if(retError == PARSER_ErrorNone)
    m_CurrentParserState = PARSER_READY;
  else if(retError == PARSER_ErrorDataUnderRun)
    m_CurrentParserState = PARSER_UNDERRUN;
  return retError;
}

//=============================================================================
// FUNCTION: parse_aac_file_header
//
// DESCRIPTION:
//  parse aac file header
//
// PARAMETERS
//  aac_header_aach - pointer to the aac header structure
//
// RETURN:
//  None
//
// SIDE EFFECTS
//  None
//=============================================================================
//

void aacParser::parse_aac_file_header()
{
#ifdef FEATURE_FILESOURCE_AAC_PARSER_DEBUG
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"parse_aac_file_header");
#endif
  if(m_aac_metadata)
  {
    m_aac_header_aach.nChannels = m_aac_metadata->m_aac_tech_metadata.channel;
    m_aac_header_aach.nSampleRate = AAC_SAMPLING_FREQUENCY_TABLE[m_aac_metadata->m_aac_tech_metadata.sample_rate];
    m_aac_header_aach.nBitRate = m_aac_metadata->m_aac_tech_metadata.bit_rate;
  }
}
//=============================================================================
// FUNCTION : parse_frame_header
//
// DESCRIPTION
//  Parse a frame header according to audio specific format and return the
//  calculated frame size and frame duration. Input buffer (frame) must contain
//  an entire frame header, otherwise an error will be returned
//
// PARAMETERS
//  frame :  A uint8 pointer to frame_size bytes, the frame header to be parsed
//              Note: the entire frame header is expected, thus it must be
//              at least  <em>frame_header_size</em> in length.
//  frame_size : frame size determine from parsing the frame header
//  frame_time : frame duration in miliseconds determine from parsing frame
//               header
//
// RETURN VALUE
//  AAC_SUCCESS : Frame header was valid, and frame size is returned
//  AAC_INVALID_PARM : Invalid input parameters
//  AEE_EBADSTATE : Invalid format parser state
//  AAC_FAILURE : Frame header was INVALID, frame size is not valid
//
// SIDE EFFECTS
//  None
//=============================================================================
//
IAudioReturnType aacParser::parse_frame_header (uint8* frame,
                                                uint32* frame_size,
                                                uint32* frame_time)
{
  IAudioReturnType result = IAUDIO_FAILURE;
#ifdef FEATURE_FILESOURCE_AAC_PARSER_DEBUG
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"parse_frame_header");
#endif
  // Validate input parameters
  if( (!frame)|| (!frame_size) || (!frame_time) )
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,"parse_frame_header INVALID USER DATA");
    result = IAUDIO_FAILURE;
  }
  else
  {
    // Initialize the output variables
    *frame_size = 0;
    *frame_time = 0;
    if (AAC_FORMAT_UNKNOWN == m_aac_format)
    {
      // parse_file_header API is not called yet - parse_frame_header can be
      // called only after AAC format parser successfully parses the file header
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,"parse_frame_header AAC_FAILURE");
      result = IAUDIO_FAILURE;
    }
    else
    {
      result = IAUDIO_FAILURE;
      if(parse_aac_frame_header(frame,m_aac_format,frame_size,frame_time) == PARSER_ErrorNone)
      {
        result = IAUDIO_SUCCESS;
      }
    }
  }
  return result;
}

//=============================================================================
// FUNCTION : parse_aac_frame_header
//
// DESCRIPTION
//  Parse a frame header according to audio specific format and return the
//  calculated frame size.
//
// PARAMETERS
//  frame : input bitstream
//  format : AAC File format
//  header : AAC Frame header info
//
// RETURN VALUE
//  1: Frame header was valid, and frame size is returned
//  0: Frame header was INVALID, frame size is not valid
//
// SIDE EFFECTS
//  None
//=============================================================================
//
PARSER_ERRORTYPE aacParser::parse_aac_frame_header (uint8 *frame,
                                                    aac_format_type format,
                                                    uint32* frame_size,
                                                    uint32* frame_time)
{
#ifdef FEATURE_FILESOURCE_AAC_PARSER_DEBUG
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"parse_aac_frame_header");
#endif
  PARSER_ERRORTYPE result = PARSER_ErrorNone;
  *frame_size =0;

  switch (format)
  {
    case AAC_FORMAT_ADTS:
    {
#ifdef FEATURE_FILESOURCE_AAC_PARSER_DEBUG
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"AAC_FORMAT_ADTS calling parse_adts_frame_header");
#endif
      result = parse_adts_frame_header(frame,frame_size,frame_time);
      break;
    }

    case AAC_FORMAT_ADIF:
    case AAC_FORMAT_RAW:
    case AAC_FORMAT_LOAS:
    {
      result = PARSER_ErrorUnsupportedCodecType;
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"parse_aac_frame_header not supported for non ADTS...");
      break;
    }
    case AAC_FORMAT_UNKNOWN:
    default:
    {
      result = PARSER_ErrorDefault;
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,"parse_aac_frame_header AAC_PARSE_ERROR");
      break;
    }
  }
  return result;
}

//=============================================================
// FUNCTION : parse_adts_frame_header
//
// DESCRIPTION
//  Parse a frame header according to audio specific format and return the calculated frame size.
//  AudioBuffer must contain an entire frame header, otherwise an error will be returned.
//  This function must not consume data from the AudioBuffer
//  (ie. does not update the AudioBuffer's pointers).
//
// PARAMETERS
//  frame : input bitstream
//  header : aac Frame header info
//
// RETURN VALUE
//  AAC_SUCCESS: Frame header was valid, and frame size is returned
//  AAC_FAILURE: Frame header was INVALID, frame size is not valid
//
// SIDE EFFECTS
//  None
//=============================================================
PARSER_ERRORTYPE aacParser::parse_adts_frame_header (uint8*  frame,
                                                     uint32* frame_size,
                                                     uint32* frame_time)
{
#ifdef FEATURE_FILESOURCE_AAC_PARSER_DEBUG
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"parse_adts_frame_header");
#endif
  PARSER_ERRORTYPE reterror = PARSER_ErrorDefault;
  uint16 uData = (uint16)((frame[1] << 8) + frame[0]);

  // Verify sync word and layer field.
  if (ADTS_HEADER_MASK_RESULT != (uData & ADTS_HEADER_MASK))
  {
    //MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"parse_adts_frame_header no adts mask...");
    return PARSER_ErrorStreamCorrupt;
  }

  // Check whether CRC data is present or not.
  if(0 == (frame[1] & 0x01))
  {
    m_bCRCPresent = true;
  }
  else
  {
    m_bCRCPresent = false;
  }

  // Extract frame length from the frame header
  uint64 frameLength
                 = (static_cast<uint64> (frame [3] & 0x03) << 11)
                 | (static_cast<uint64> (frame [4]) << 3)
                 | (static_cast<uint64> (frame [5] & 0xE0) >> 5);
#ifdef FEATURE_FILESOURCE_AAC_PARSER_DEBUG
  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,"frameLength %llu", frameLength);
#endif
  // Verify we have a valid frame length
  if (0 == frameLength)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                "parse_adts_frame_header AAC_CORRUPTED_FILE");
    return PARSER_ErrorStreamCorrupt;
  }

  // parser framework handles max frame length of 32 bits
  *frame_size = (uint32)(frameLength & 0xFFFF);

  // Extract sampling frequency
  const uint8 samplingFrequencyIndex = ((frame [2] >> 2) & 0x0F);

  //make sure sampling freq. index is within the range
  if(samplingFrequencyIndex <= 12)
  {
    //layer should be always be 00
    if((frame [1] & 0x06) == 0x00)
    {
      // Calculate playback time given the the sampling freq
      *frame_time = (AAC_SAMPLES_PER_DATA_BLOCK * 1000)/
                              AAC_SAMPLING_FREQUENCY_TABLE[samplingFrequencyIndex];
      //if fixed header is not set, store the fixed header
      if(!m_bFixedADTSHdrSet)
      {
        //store 24 bits as it is
        memcpy(m_adts_fix_hdr,frame,AAC_ADTS_FIX_HDR_SIZE-1);
        //only 4 bits are part of fixed header
        uint8 byte = frame[3];
        byte &= 0xF0;
        m_adts_fix_hdr[3] = byte;
        m_bFixedADTSHdrSet = true;
        reterror = PARSER_ErrorNone;
      }
      else
      {
        //compare this header against fixed header to detect false sync..
        if(memcmp(frame,m_adts_fix_hdr,AAC_ADTS_FIX_HDR_SIZE-1) == 0)
        {
          //first 24 bits match, now compare the remaining 4 bits against the
          //fix header..
          if(m_adts_fix_hdr[3] == (frame[3] & 0xF0))
          {
            reterror = PARSER_ErrorNone;
          }
        }
      }
    }//if((frame [1] & 0x06) == 0x00)
  }//if(samplingFrequencyIndex <= 12)
  return reterror;
}

//=============================================================================
// FUNCTION: parse_aac_audio_data
//
// DESCRIPTION:
//  parse aac audio data information
//
// PARAMETERS
//  m_aac_audio_info - pointer to the aac audio info structure
//
// RETURN:
//  None
//
// SIDE EFFECTS
//  None
//=============================================================================
//
void aacParser::parse_aac_audio_data()
{
#ifdef FEATURE_FILESOURCE_AAC_PARSER_DEBUG
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"parse_aac_audio_data");
#endif
  m_aac_audio_info.dwSuggestedBufferSize = AAC_MAX_FRAME_SIZE;
}

//=============================================================================
// FUNCTION: GetAACHeader
//
// DESCRIPTION:
//  Parse the aac file header
//
// PARAMETERS
//  pAacHdrPtr - pointer to aac header structure
//
// RETURN:
//  aacErrorType
//
// SIDE EFFECTS
//  None
//=============================================================================
//

PARSER_ERRORTYPE aacParser::GetAACHeader(aac_header_aach* pAacHdrPtr)
{
  PARSER_ERRORTYPE retError = PARSER_ErrorDefault;
  aac_header_aach *p_aac_header_aach;
  p_aac_header_aach = &m_aac_header_aach;

  if(!pAacHdrPtr)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,"GetAACHeader AAC_INVALID_USER_DATA");
    retError = PARSER_ErrorInvalidParam;
    return retError;
  }
  if(NULL == p_aac_header_aach)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,"GetAACHeader AAC_PARSE_ERROR");
    retError = PARSER_ErrorDefault;
    return retError;
  }
  memset(pAacHdrPtr,0,sizeof(aac_header_aach));
  memcpy(pAacHdrPtr,p_aac_header_aach,sizeof(aac_header_aach));
  retError = PARSER_ErrorNone;
  return retError;
}
//=============================================================================
// FUNCTION: GetTrackDecoderSpecificInfoContent
//
// DESCRIPTION:
//  COnstructs and returns
// AAC Audio config header using Helper function in FileBase
//
// OUT PARAMETERS:
// pBuf - pointer to audio config header
// pSize - Size of such audio config header
//
// RETURN:
//  aacErrorType
//
// SIDE EFFECTS
//  None
//=============================================================================
//
PARSER_ERRORTYPE  aacParser::GetTrackDecoderSpecificInfoContent(uint8* pBuf,
                                                                uint8* pSize)
{
  PARSER_ERRORTYPE retVal = PARSER_ErrorDefault;
  if(pSize)
  {
    if(MAKE_AAC_AUDIO_CONFIG(pBuf,
                          m_aac_metadata->m_aac_tech_metadata.audio_object,
                          m_aac_metadata->m_aac_tech_metadata.sample_rate,
                          m_aac_metadata->m_aac_tech_metadata.channel,
                          pSize))
    {
      retVal = PARSER_ErrorNone;
    }
  }
  return retVal;
}

//=============================================================================
// FUNCTION: GetAACDecodeInfo
//
// DESCRIPTION:
//  Parse the aac decode information
//
// PARAMETERS
//  pAACDecodeinfo - pointer to aac decode info structure
//
// RETURN:
//  aacErrorType
//
// SIDE EFFECTS
//  None
//=============================================================================
//
PARSER_ERRORTYPE aacParser::GetAACDecodeInfo(aac_decode_info* pAACDecodeinfo)
{
  PARSER_ERRORTYPE returnval = PARSER_ErrorDefault;
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"GetAACDecodeInfo");
  if(m_aac_metadata)
  {
    pAACDecodeinfo->audio_object = m_aac_metadata->m_aac_tech_metadata.audio_object;
    pAACDecodeinfo->aac_subformat_type = (uint8)m_aac_metadata->m_aac_tech_metadata.format_type;
    returnval = PARSER_ErrorNone;
  }
  return returnval;
}

//=============================================================================
// FUNCTION: GetAudioInfo
//
// DESCRIPTION:
// This function returns audio format specific information
//
// PARAMETERS
//  pAudioInfo - pointer to the aac_audio_info structure
//
// RETURN:
//  aacErrorType
//
// SIDE EFFECTS
//  None
//=============================================================================
//
PARSER_ERRORTYPE aacParser::GetAudioInfo(aac_audio_info* pAudioInfo)
{
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"GetAudioInfo");
  PARSER_ERRORTYPE retError = PARSER_ErrorDefault;
  aac_audio_info* p_aac_audio_info;
  p_aac_audio_info = &m_aac_audio_info;
  if(NULL == p_aac_audio_info)
  {
    retError = PARSER_ErrorInvalidParam;
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,"GetAudioInfo AAC_PARSE_ERROR");
  }
  else
  {
    memset(pAudioInfo,0,sizeof(aac_audio_info));
    memcpy(pAudioInfo,p_aac_audio_info,sizeof(aac_audio_info));
    retError = PARSER_ErrorNone;
  }
  return retError;
}

/* ============================================================================
   FUNCTION: GetApproxDuration

   DESCRIPTION:
    This function returns the average duration value

   PARAMETERS
    None

   RETURN:
    aacErrorType

   SIDE EFFECTS
    None
//========================================================================== */

PARSER_ERRORTYPE aacParser::GetApproxDuration(uint64 *pDuration)
{
  PARSER_ERRORTYPE retVal = PARSER_ErrorDefault;
#ifdef FEATURE_FILESOURCE_AAC_PARSER_DEBUG
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"GetApproxDuration");
#endif

  uint32 nNumBytesRequest = AAC_READ_BUFFER_SIZE;
  uint32 nBytesRead       = 0;
  uint32 nFrameSize       = 0;
  uint32 nFrameTime       = 0;
  uint32 nBytesSkipped    = 0;
  uint32 nFrameBufLen     = 0;
  uint64 nTotalFrameSize  = 0;
  uint64 nTotalFrameTime  = 0;
  uint32 nFramesParsed    = 0;
  uint64 nCurrOffset      = m_nCurrOffset;
  uint8* dataBuffer       = (uint8*)m_ReadBuffer;
  while(nFramesParsed < MAX_FRAMES)
  {
    if(!nBytesRead)
    {
      nCurrOffset += nFrameBufLen + nBytesSkipped;
      nBytesRead = AACCallbakGetData (nCurrOffset, nNumBytesRequest,
                                      dataBuffer, AAC_READ_BUFFER_SIZE,
                                      m_pUserData, m_bEndOfData);
      nBytesSkipped = 0;
      nFrameBufLen  = 0;
      if(nBytesRead <= AAC_FRAME_HEADER_SIZE)
      {
        if(nFramesParsed)
        {
          MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
                      "GetApproxDuration done with %lu frames", nFramesParsed);
          break;
        }
        //check for EOF in case of local playback
        if(m_bEndOfData == true)
        {
          nBytesRead = 0;
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
                      "GetApproxDuration detected end of ADTS/ADIF AAC track");
          return PARSER_ErrorEndOfFile;
        }
        else
        {
          nBytesRead = 0;
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
                      "GetApproxDuration detected Data Underrun!!");
          return PARSER_ErrorDataUnderRun;
        }
      }
    }//if(!nBytesRead)

    uint32 nConsumedDataLen = nFrameBufLen + nBytesSkipped;
    while((nBytesRead > nConsumedDataLen) &&
          ((nBytesRead - nConsumedDataLen) > AAC_FRAME_HEADER_SIZE))
    {
      if(IAUDIO_SUCCESS == parse_frame_header(dataBuffer + nBytesSkipped +
                                              nFrameBufLen, &nFrameSize,
                                              &nFrameTime) )
      {
        nFrameBufLen    += nFrameSize;
        nTotalFrameSize += nFrameSize;
        nTotalFrameTime += nFrameTime;
        nFramesParsed++;
      }
      else
      {
        nBytesSkipped++;
      }
      nConsumedDataLen = nFrameBufLen + nBytesSkipped;
    }//while(nBytesRead - frameBufLen > AAC_FRAME_HEADER_SIZE && ....
    nBytesRead = 0;

  }//while(nFramesParsed < MAX_FRAMES)

  uint64 nAvgFrameSize = nTotalFrameSize / nFramesParsed;
  uint64 nAvgFrameTime = nTotalFrameTime / nFramesParsed;
  uint64 nFrameCount   = m_nFileSize / nAvgFrameSize;
  if(pDuration)
  {
    *pDuration = nAvgFrameTime * nFrameCount;
    retVal = PARSER_ErrorNone;
  }

  return retVal;
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
uint64 aacParser::GetClipDurationInMsec()
{
  uint32 noEntries = 50;
  uint64 uDuration = m_aac_duration;
  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
               "GetClipDurationInMsec m_seek_function_defined %d",
               m_seek_function_defined);

  /* If file size is not known, then duration will not be calculated */
  if(MAX_FILE_SIZE == m_nFileSize)
    return uDuration;

  /* If duration is already calculated, return that value */
  if (m_aac_duration)
  {
    return m_aac_duration;
  }

  if( (!m_seek_function_defined) && ((m_aac_format != AAC_FORMAT_ADIF) &&
      (m_aac_format != AAC_FORMAT_LOAS)))
  {
    uint8* dataBuffer  = (uint8*)m_ReadBuffer;
    uint32 nBytesRead  = 0;

    /* If input file is more than upper boundary that parser can support, then
       Parser will provide average duration value and disable seek opertion.
       This is to reduce the time taken to report OPEN_COMPLETE. This will be
       done irrespective of source type: Local Playback or Streaming. */
    if((m_nFileSize < FILE_UPPER_BOUNDARY) && (!m_bHTTPStreaming))
    {
      /* This is special to check whether complete file is downloaded or not.
         If EndOfData flag is set to false, parser will calculate average
         duration and disable seek operation. */
      nBytesRead = AACCallbakGetData (m_nFileSize - 10, 10,
                                      dataBuffer, AAC_READ_BUFFER_SIZE,
                                      m_pUserData, m_bEndOfData);
      MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_HIGH,
                   "dataRead at the end of file is %lu, m_bEndOfData %d",
                   nBytesRead, m_bEndOfData);
    }

    if((0 == nBytesRead) || (FALSE == m_bEndOfData))
    {
      (void)GetApproxDuration(&uDuration);
    }
    else
    {
      if(psimple_seektable == NULL)
      {
        psimple_seektable = MM_New_Args(simple_seektable,(noEntries));
        (void)OSCL_FileSeek(m_AACFilePtr,0,SEEK_SET);
        (void)OSCL_FileSeek(m_AACFilePtr,(uint32)m_audio_track.start,SEEK_CUR);
      }

      if(pseek == NULL && psimple_seektable != NULL)
      {
        pseek = MM_New_Args(seek,(psimple_seektable
                         ,this
                         ,m_AACFilePtr
                         ,AAC_FRAME_HEADER_SIZE
                         ,m_audio_track.start
                         ,m_audio_track.end));
      }

      if(NULL != pseek)
      {
        uDuration = pseek->get_duration();
        if(!uDuration)
        {
          uDuration = 0;
        }
        if(pseek->set_mode(PREGENERATE_TABLE))
        {
          uDuration = 0;
        }
      }
    }
  }
  m_aac_duration = uDuration;

  return uDuration;
}
//=============================================================================
// FUNCTION: GetCurrentSample
//
// DESCRIPTION:
//  This function returns the current sample data
//
// PARAMETERS
//  dataBuffer - pointer to the data sample
//  nMaxBufSize - maximum buffer size
//  nBytesNeeded - no of bytes needed

// RETURN:
//  aacErrorType
//
// SIDE EFFECTS
//  None
//=============================================================================
//
PARSER_ERRORTYPE aacParser::GetCurrentSample(uint8* dataBuffer,
                                             uint32 nMaxBufSize,
                                             uint32 *nBytesNeeded,
                                             bool bUpdateTime)
{
  PARSER_ERRORTYPE retVal = PARSER_ErrorDefault;
#ifdef FEATURE_FILESOURCE_AAC_PARSER_DEBUG
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"GetCurrentSample");
#endif
  if( (!dataBuffer) || (!nMaxBufSize) ||  (!nBytesNeeded) )
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,"GetCurrentSample AAC_INVALID_USER_DATA");
    retVal = PARSER_ErrorInvalidParam;
  }
  else
  {
    memset(dataBuffer,0,nMaxBufSize);
    uint32 nNumBytesRequest = *nBytesNeeded;
    uint32 nBytesRead = 0;
    bool bFeedStreamOfBytes = true;
    m_firstFrame = false;
    //We can only output ADTS on frame boundary
#ifdef OUTPUT_AAC_ON_FRAME_BOUNDARY
    if(m_aac_format == AAC_FORMAT_ADTS)
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"GetCurrentSample AAC_FORMAT_ADTS outputting on frame boundary..");
      bFeedStreamOfBytes = false;
      //If frame header is located, points to the frame size
      uint32 frameSize = 0;
      //If frame header is located, points to the frame time
      uint32 frameTime = 0;
      //Keep track of junk bytes, if available, between two valid ADTS frames
      uint32 nBytesSkipped = 0;
      //When collecting more than one frame, denotes the number of bytes that have been accumulated so far..
      uint32 frameBufLen = 0;
      //Set to true if valid frame header is located
      bool frameStartFound = false;
      while(!frameStartFound)
      {
        if(!nBytesRead)
        {
          m_nCurrOffset += (uint64)nBytesSkipped;
          nBytesRead = AACCallbakGetData (m_nCurrOffset, nNumBytesRequest,
                                          dataBuffer, nMaxBufSize, m_pUserData,
                                          m_bEndOfData);
          nBytesSkipped = 0;
          if((nBytesRead <= AAC_FRAME_HEADER_SIZE) ||
             (m_nNextFrameTime >= m_aac_duration))
          {
            //check for EOF in case of local playback
            if(m_bEndOfData == true)
            {
              nBytesRead = *nBytesNeeded = 0;
              MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
                          "GetCurrentSample detected end of ADTS/ADIF AAC track");
              return PARSER_ErrorEndOfFile;
            }
            else
            {
              nBytesRead = *nBytesNeeded = 0;
              MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
                          "GetCurrentSample detected Data Underrun!!");
              return PARSER_ErrorDataUnderRun;
            }
          }
        }//if(!nBytesRead)

        while(nBytesRead - frameBufLen > AAC_FRAME_HEADER_SIZE &&
              IAUDIO_SUCCESS == parse_frame_header(dataBuffer + nBytesSkipped + frameBufLen,
                                                   &frameSize,&frameTime) )
        {
          if(nMaxBufSize < frameSize)
          {
            MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL, "GetCurrentSample: Buf size is smaller than frame size!");
            return PARSER_ErrorInsufficientBufSize;
          }
          if(nBytesRead - frameBufLen < frameSize)
          {
            /* If single frame is also not found, then we will return data underrun in case of streaming and
               end of file in case of local playback. We will not provide incomplete frames to the upper layer*/
            if(frameStartFound == false)
            {
              MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH, "GetCurrentSample: Complete frame is not available in the o/p buf!!");
              nBytesRead = *nBytesNeeded  = 0;
              if(m_bEndOfData == true)
                return PARSER_ErrorEndOfFile;
              else
                return PARSER_ErrorDataUnderRun;
            }
            break;
          }
          frameStartFound = true;
          frameBufLen += frameSize;
          retVal = PARSER_ErrorNone;
#ifdef HLS_PRIVATE_TIMESTAMP
          /* If ID3 tag is available, then only one frame will be given in output buf */
          if(m_id3tagfound == true)
          {
            m_firstFrame = true;
            m_filereadpos = m_nCurrOffset + uint64(nBytesSkipped);
            /* This function is called to update media info of the AACParser with
               the first frame after ID3 tags */
            (void)parse_frame_metadata();
            m_id3tagfound = false;
            metadata_id3v2_type *id3V2 = m_aac_metadata->get_id3v2(0);
            /* If it is first frame and timestamp is valid then update the class param accordingly */
            if(id3V2)
            {
              m_firstFrameTS = id3V2->private_tag.timeStamp;
              m_nCurrentTime = m_nNextFrameTime = m_firstFrameTS;
              m_isFirstTSValid = id3V2->private_tag.isTsValid;
            }

            /* If Single frame output mode is set, then don't break this while
               loop. Following "if" will do the needful things on this frame
               and breaks the while loop. User may requested to strip header
               info, which will be taken care in the next "if" condition.*/
            if(FILE_SOURCE_MEDIA_OUTPUT_SINGLE_AUDIO_FRAME !=
               m_hFrameOutputModeEnum)
            {
              break;
            }
          }
#endif
          /*If parser is configured to output one frame at a time,
          we need to break of the loop and stop collecting more frames..*/
          if(m_hFrameOutputModeEnum == FILE_SOURCE_MEDIA_OUTPUT_SINGLE_AUDIO_FRAME)
          {
            if(bUpdateTime)
            {
              m_nCurrentTime = m_nNextFrameTime;
              m_nNextFrameTime += frameTime;
            }
            /*If parser is configured to strip ADTS header, knock off the header
              before outputting frame. If CRC data is there, we need to srip that
              section as well.*/
            if(m_hHeaderOutputModeEnum == FILE_SOURCE_MEDIA_STRIP_AUDIO_HEADER)
            {
              if(frameSize > AAC_ADTS_HEADER_SIZE)
              {
                uint32 skipDataLen = AAC_ADTS_HEADER_SIZE;
                if(m_bCRCPresent)
                {
                  skipDataLen += AAC_CRC_DATA_LEN;
                }
                memmove(dataBuffer,dataBuffer + skipDataLen,(frameSize-skipDataLen));
                frameSize = frameSize - skipDataLen;
                frameBufLen -= (skipDataLen);
              }
            }
            break;
          }
          /*
          If this is the first sample pulled after seek is done, we will give out only one frame..
          */
          if(true == m_SeekDone)
          {
            m_SeekDone = false;
            break;
          }
        }//while(nBytesRead - frameBufLen > AAC_FRAME_HEADER_SIZE && ....
#ifdef HLS_PRIVATE_TIMESTAMP
        //! Check whether "ID3" tag is found or not
        if(!frameStartFound && !memcmp("ID3", dataBuffer+frameBufLen, 3))
        {
          uint64 prev_startOffset = m_audio_track.start;
          m_audio_track.start = m_nCurrOffset;
          PARSER_ERRORTYPE result = parse_id3();
          if(PARSER_ErrorNone != result)
          {
            m_audio_track.start = prev_startOffset;
            MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL, "GetCurrentSample: ID3 Parsing is failed");
            return result;
          }
          m_nCurrOffset = m_audio_track.start;

          /* Retain the start value which is available in the file. If seek is called, this
             start value will be used to calculate relative file offset value. So, we need to
             update the start offset value with the prev value. */
          m_audio_track.start = prev_startOffset;
          nBytesRead = 0;
          nBytesSkipped = 0;
          frameBufLen   = 0;
          m_id3tagfound = true;
          continue;
        }
#endif
        if(frameStartFound == false)
        {
          nBytesRead--;
          nBytesSkipped++;
          continue;
        }
      }//while(!frameStartFound)

      /* If there is some junk data at the start of the buffer, we will skip that data
      and make sure the buffer starts with framesync word */
      if(nBytesSkipped)
      {
        memcpy(dataBuffer, dataBuffer + nBytesSkipped, frameBufLen);
        m_nCurrOffset += (uint64)nBytesSkipped;
      }

      /* Update output buffer size param */
      *nBytesNeeded = frameBufLen;
#ifdef HLS_PRIVATE_TIMESTAMP
      /* Skipping TAG header info which is of 128 bytes. */
      if( (nBytesRead - frameBufLen) >= 3 &&
          !memcmp("TAG", dataBuffer+frameBufLen, 3))
      {
        frameBufLen += 128;
      }
#endif

    /* Update cur offset value to the start of next frame */
    m_nCurrOffset += uint64(frameBufLen);

    /*
    * If parser is configured to o/p one frame and being asked to strip
    * ADTS header, add header size to point to the next frame header.
    * Also add CRC Data Length, if CRC field is present.
    */
    if((m_hFrameOutputModeEnum == FILE_SOURCE_MEDIA_OUTPUT_SINGLE_AUDIO_FRAME)&&
      (m_hHeaderOutputModeEnum == FILE_SOURCE_MEDIA_STRIP_AUDIO_HEADER))
    {
      m_nCurrOffset += AAC_ADTS_HEADER_SIZE;
      if(m_bCRCPresent)
      {
        m_nCurrOffset += AAC_CRC_DATA_LEN;
      }
    }
#ifdef HLS_PRIVATE_TIMESTAMP
    /* If the current offset starts with ID3 tag.*/
    if((nBytesRead - frameBufLen) >= 3 &&
       !memcmp("ID3", dataBuffer+frameBufLen, 3))
    {
      uint64 prev_startOffset = m_audio_track.start;
      m_audio_track.start = m_nCurrOffset;
      PARSER_ERRORTYPE result = parse_id3();
      if(PARSER_ErrorNone == result)
      {
          m_nCurrOffset = m_audio_track.start;
          m_id3tagfound = true;
      }
      /* Retain the start value which is available in the file. Even if seek is called, this
         start value will not be affected */
      m_audio_track.start = prev_startOffset;
      }
#endif
    }//if(m_aac_format == AAC_FORMAT_ADTS)
#endif
    //Check if we need to supply buffer worth of data
    if(bFeedStreamOfBytes)
    {
      if( (m_nFileSize - m_nCurrOffset) < nNumBytesRequest)
      {
        nNumBytesRequest = (uint32)(m_nFileSize - m_nCurrOffset);
      }
      if(nNumBytesRequest)
      {
        //check if we have loas and have been asked to o/p one frame
        if((m_aac_format == AAC_FORMAT_LOAS) &&
           (m_hFrameOutputModeEnum == FILE_SOURCE_MEDIA_OUTPUT_SINGLE_AUDIO_FRAME) )
        {
          uint32 framesize = 0;
          uint32 framehdr = 0;
          if(PARSER_ErrorNone == parse_loas_file_header(m_nCurrOffset,&framesize,&framehdr,NULL))
          {
            MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW,"GetCurrentSample bFeedStreamOfBytes is TRUE");
            nBytesRead = AACCallbakGetData (m_nCurrOffset, framesize,
                                            dataBuffer, nMaxBufSize, m_pUserData, m_bEndOfData);
            retVal = PARSER_ErrorNone;
            if( !nBytesRead && m_bEndOfData == true)
            {
              *nBytesNeeded  = nBytesRead = 0;
              MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"GetCurrentSample AAC_END_OF_FILE");
              retVal = PARSER_ErrorEndOfFile;
            }
            else if(!nBytesRead)
            {
              *nBytesNeeded  = nBytesRead = 0;
              MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"GetCurrentSample AAC_DATA_UNDERRUN");
              retVal = PARSER_ErrorDataUnderRun;
            }
            else
            {
              m_nCurrOffset += nBytesRead;
              *nBytesNeeded = nBytesRead;
              //check if we have been asked to strip the audio header(loas header)
              if(m_hHeaderOutputModeEnum == FILE_SOURCE_MEDIA_STRIP_AUDIO_HEADER)
              {
                memmove(dataBuffer,dataBuffer+framehdr,(framesize-framehdr));
                nBytesRead -= framehdr;
                *nBytesNeeded = nBytesRead;
              }
            }
          }//if(PARSER_ErrorNone == parse_loas_file_header(m_nCurrOffset,&framesize,&frametime,NULL))
        }
        else
        {
          //check if we have adif stream and have parsed adif header successfully
          //and have been asked to strip the adif header.
          //by default, parser will start o/p adif header when data starts flowing.
          if( (m_n_adif_hdr_len)                &&
              (m_aac_format == AAC_FORMAT_ADIF) &&
              (m_nCurrOffset < m_n_adif_hdr_len)&&
              (m_hHeaderOutputModeEnum == FILE_SOURCE_MEDIA_STRIP_AUDIO_HEADER)
            )
          {
            m_nCurrOffset+= m_n_adif_hdr_len;
            MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW,"GetCurrentSample adjusting m_nCurrOffset for ADIF...");
          }
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW,"GetCurrentSample bFeedStreamOfBytes is TRUE");
          nBytesRead = AACCallbakGetData (m_nCurrOffset, nNumBytesRequest,
                                          dataBuffer, nMaxBufSize, m_pUserData, m_bEndOfData);
          retVal = PARSER_ErrorNone;
          if( !nBytesRead && m_bEndOfData == true)
          {
            *nBytesNeeded  = nBytesRead = 0;
            MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                     "GetCurrentSample AAC_END_OF_FILE");
            retVal = PARSER_ErrorEndOfFile;
          }
          else if(!nBytesRead)
          {
            *nBytesNeeded  = nBytesRead = 0;
            MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                         "GetCurrentSample AAC_DATA_UNDERRUN");
            retVal = PARSER_ErrorDataUnderRun;
          }
          m_nCurrOffset += nBytesRead;
          *nBytesNeeded = nBytesRead;
        }
      }
      else
      {
        *nBytesNeeded  = nBytesRead = 0;
        retVal = PARSER_ErrorEndOfFile;
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
                       "GetCurrentSample AAC_END_OF_FILE");
      }
    }
  }
#ifndef _ANDROID_
  if ((m_nCurrOffset >= m_nFileSize) && (m_bEndOfData))
  {
    retVal = PARSER_ErrorEndOfFile;
  }
#endif
  return retVal;
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

void aacParser::init_file_position()
{
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"init_file_position");
  m_nCurrOffset = 0;
  m_nCurrOffset += m_audio_track.start;
  //m_nCurrOffset += AAC_FILE_HEADER_SIZE;
}

//=============================================================================
// FUNCTION: set_newfile_position
//
// DESCRIPTION:
//
// PARAMETERS
//  file_position - new file position
//
// RETURN:
//  None
//
// SIDE EFFECTS
//  None
//=============================================================================
void aacParser::set_newfile_position(uint64 file_position)
{
  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,"set_newfile_position new position %llu",file_position);
  m_nCurrOffset = (uint64) file_position;
}
//=============================================================================
// FUNCTION: RandomAccessDenied
//
// DESCRIPTION:
//  Determines if seek will be supported for given clip or not.
//
// PARAMETERS
//
// RETURN:
//  qcpErrorType
//
// SIDE EFFECTS
//  None
//=============================================================================
uint8  aacParser::RandomAccessDenied()
{
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"aacParser checking if seek is supported...");
  uint8 nSeekDenied = 1;
  //If seek function is defined, parser can handle the seek.
  if(m_seek_function_defined)
  {
    nSeekDenied = 0;
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"aacParser can support the seek!!!");
    // Denied seek in case of AAC-ADIF CBR format only if seek time is nonZERO!!
  }
  else
  {
    /* Even if seek function is not defined, ADTS can be seeked using seek
       lib. Check if aac format is ADTS and file size within seekable range. */
    if((m_aac_format == AAC_FORMAT_ADTS) &&
       (m_nFileSize < FILE_UPPER_BOUNDARY))
    {
      nSeekDenied = 0;
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"aacParser can support the seek!!!");
    }
  }
  if(nSeekDenied)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"aacParser can't support the seek..");
  }
  return nSeekDenied;
}
//=============================================================================
// FUNCTION: Seek
//
// DESCRIPTION:
//  This function will return the corresponding new file position for the given input time
//
// PARAMETERS
//  nReposTime - time to seek
//
// RETURN:
//  aacErrorType
//
// SIDE EFFECTS
//  None
//=============================================================================
//
PARSER_ERRORTYPE aacParser::Seek(uint64 nReposTime, uint64 *nSeekedTime)
{
  PARSER_ERRORTYPE status = PARSER_ErrorNone;
  bool isDataAvailable = false;
  bool isID3TagAvailable = false;
  uint64 prevFilePosition = m_nCurrOffset;
  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,"Seek nReposTime %llu",nReposTime);
  uint64 time = nReposTime;
  uint64 file_position;

  if(nReposTime == 0)
  {
    set_newfile_position(m_audio_track.start);
    m_nCurrentTime = nReposTime;
    m_nNextFrameTime = m_nCurrentTime;
    return PARSER_ErrorNone;
  }

  if( m_aac_format == AAC_FORMAT_ADIF )
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR,"seek to non-zero TS is not allowed on AAC-ADIF!!!");
    return PARSER_ErrorNotImplemented;
  }
  /* Check whether seek Requsted time is less than the first frame's TS */
  if(m_firstFrameTS - m_baseTS > nReposTime)
    time = 0;
  /* If firstFrameTS is non-zero value, update seektime requested accordingly */
  else if(nReposTime >= m_firstFrameTS - m_baseTS)
    time = nReposTime - (m_firstFrameTS - m_baseTS);

  if(m_seek_function_defined)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"Seek m_seek_function_defined");
    status = get_seek_position ( time, &file_position);
    if(PARSER_ErrorNone == status)
    {
      set_newfile_position(file_position);
      m_nCurrentTime = time;
      m_nNextFrameTime = m_nCurrentTime;
    }
  }
  else
  {
    if(NULL != pseek)
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"Seek pseek->process_seek");
      time = pseek->process_seek(time);
      m_nCurrentTime = time;
      m_nNextFrameTime = m_nCurrentTime;
    }
    else
    {
      status = get_accurate_seek_position(time, &file_position, nSeekedTime);
      /* In the above function, we will parse frame by frame to seek to the
         requested Timestamp. So, we have already parsed the data that will be
         returned as part of getNextMediaSample after seek is returned as successful*/
      isDataAvailable = true;
      if(PARSER_ErrorNone == status)
      {
        set_newfile_position(file_position);

        /* Update output seek time according to the first frame's timestamp */
        *nSeekedTime += (m_firstFrameTS - m_baseTS);

        /* If seek is successful, then we will not use id3 tags that are found at the
           start of the playback. By marking id3tagfound flag as false, we will ensure
           that id3 tags are not used when seek is called */
          m_id3tagfound = false;

        /* Mark seekDone flag. Parser will provide single frame as output through
           getNextMediaSample API since there is timestamp associated with it. */
        m_SeekDone = true;
        m_nCurrentTime = *nSeekedTime;
        m_nNextFrameTime = m_nCurrentTime;
      }
    }
  }

  /* We will verify whether next frame is available or not,
     only in case of ADTS format */
  if(false == isDataAvailable && PARSER_ErrorNone == status &&
     (m_aac_format == AAC_FORMAT_ADTS))
  {
    uint32 loopCount = 1;
    uint8  *nDataBuf = 0;
    uint32 nDataBufSize = m_aac_audio_info.dwSuggestedBufferSize;
    uint32 nBytesNeeded = 0;

    /* New file position after seek has been successful. This one will be helpful,
       if seek table has been created and seek is done by using seek table. By updating
       file_position variable, we can retain the new file position after seek */
    file_position = m_nCurrOffset;

    /* If ID3 tag is available, make this flag as false. Now, we will not be able to parse
       ID3 tag info and we will parse only frame data. */
    if(m_id3tagfound == true)
    {
      m_id3tagfound = false;
      isID3TagAvailable = true;
    }

    /* Mark seekDone flag as true, so that parser will search for one complete frame */
    m_SeekDone = true;

    do
    {
      if(nDataBuf)
      {
        MM_Free(nDataBuf);
        nDataBuf = 0;
        nDataBufSize = m_aac_audio_info.dwSuggestedBufferSize * loopCount;
      }
      nDataBuf = (uint8 *)MM_Malloc(nDataBufSize);
      if(!nDataBuf)
      {
        return PARSER_ErrorMemAllocFail;
      }
      nBytesNeeded = nDataBufSize;
      status = GetCurrentSample(nDataBuf, nDataBufSize, &nBytesNeeded,false);
    } while(PARSER_ErrorInsufficientBufSize == status && loopCount < 10);

    /* Reset id3tag flag */
    m_id3tagfound = isID3TagAvailable;

    if(PARSER_ErrorNone == status)
    {
      /* Reset file position to the calculated offset*/
      set_newfile_position(file_position);
      *nSeekedTime = time;
      *nSeekedTime += (m_firstFrameTS - m_baseTS);

      /* If seek is successful, then we will not use id3 tags that are found at
         the start of the playback. By marking id3tagfound flag as false, parser
          will ensure that id3 tags are not used when seek is called */
      m_id3tagfound = false;

      /* Mark SeekDone flag as true, so that parser will return only one
         complete frame as output through getNextMediaSample API */
      m_SeekDone = true;
    }
    else
    {
      /* Revert the Current Offset position to the value, before seek is called*/
      m_nCurrOffset = prevFilePosition;
      m_SeekDone = false;
    }
    /* Free the memory allocated to buffer */
    if(nDataBuf)
      MM_Free(nDataBuf);
  }
  return status;
}

//=============================================================
// FUNCTION : get_seek_position
//
// DESCRIPTION
//  Calculates seek position based on time. This function is only used
//  when the seek_function_defined is set to TRUE
//
// PARAMETERS
//  time  Presentation time to seek to
//  file_position  File position corresponding to the seek time provided
//
// RETURN VALUE
//  AAC_SUCCESS: Function succesfully calculated the playback duration
//  AEE_UNSUPPORTED: Function is not implemented for this audio format
//  AEE_EBADSTATE: Data not available to perform the calculation
//
// SIDE EFFECTS
//  None
//=============================================================
//
PARSER_ERRORTYPE aacParser::get_seek_position (uint64 time,
                                               uint64* file_position)
{
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"get_seek_position");
  // Validate input parameters
  if(NULL == file_position)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,"get_seek_position AAC_INVALID_PARM");
    return PARSER_ErrorInvalidParam;
  }

  if (AAC_FORMAT_UNKNOWN == m_aac_format)
  {
    // parse_file_header API is not called yet - parse_frame_header can be
    // called only after AAC format parser successfully parses the
    // file header
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,"get_seek_position AAC_FAILURE");
    return PARSER_ErrorUnknownCodecType;
  }

  if(!m_seek_function_defined)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"m_seek_function_defined is FALSE get_seek_position returning AAC_UNSUPPORTED");
    return PARSER_ErrorUnsupported;
  }

  if(m_aac_metadata->m_aac_tech_metadata.bit_rate == 0)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"m_aac_tech_metadata.bit_rate 0 get_seek_position returning AAC_UNSUPPORTED");
    return PARSER_ErrorStreamCorrupt;
  }

  uint64 bit_rate = m_aac_metadata->m_aac_tech_metadata.bit_rate;

  if (bit_rate > 1000)
  {
    // This arrangement of math operations takes care of underflow errors
    *file_position = (uint64)(time*(bit_rate/1000))/8;
  }
  else
  {
    // This arrangement of math operations takes care of overflow errors
    *file_position = (uint64)(time*(bit_rate/8))/1000;
  }
  MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
      "get_seek_position updated new file position to %llu",(*file_position));

  return PARSER_ErrorNone;
}

//=============================================================================
// FUNCTION: is_aac_format
//
// DESCRIPTION:
//  check the given file aac or not
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
bool aacParser::is_aac_format()
{
  bool bRet = false;
  if(aac_file_format)
  {
    bRet = true;
  }
  return bRet;
}
//=============================================================
// FUNCTION : parse_adts_file_header
//
// DESCRIPTION
//  Parse a file header according to audio specific format store the required parameters
//
// PARAMETERS
//  file : IFilePort interface to data to be parsed.
//  m_nFileSize : Total length of the file
//
// RETURN VALUE
//  AAC_SUCCESS: File header was parsed successfully
//  AAC_FAILURE: File  header was INVALID
//
// SIDE EFFECTS
//  None
//=============================================================
//
PARSER_ERRORTYPE aacParser::parse_adts_file_header ( )
{

#ifdef FEATURE_FILESOURCE_AAC_PARSER_DEBUG
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"parse_adts_file_header");
#endif
  PARSER_ERRORTYPE result = PARSER_ErrorNone;
  // This is a cache of the sampling frequency of the audio stream, in
  // Hertz.
  unsigned long samplingFrequency = 0;
  tech_data_aac aac_tech_metadata;
  aac_tech_metadata.type = AUDIO_AAC;
  int iframe_count=0;
  // start parsing the file header
  // Attempt to parse the input as an ADTS stream.  For every ADTS frame,
  // verify the sync word, read the frame length, update the max packet size
  // variable, and skip to the beginning of the next frame.
  int64 pos = m_filereadpos;

  /* Reset channels parameter */
  aac_tech_metadata.channel = 0;

  for(iframe_count=0;iframe_count<AAC_ADTS_FRAME_PARSE_COUNT;iframe_count++)
  {
    uint8 buffer[AAC_ADTS_HEADER_SIZE] = {0};
    // Seek to the required position and read 1 ADTS frame header
    uint32 nBytesRead = seekandreadfile(AAC_ADTS_HEADER_SIZE, pos, buffer,AAC_ADTS_HEADER_SIZE);
    if(nBytesRead < AAC_ADTS_HEADER_SIZE)
    {
      /* We need to have at least one frame worth of data */
      if(!iframe_count)
      {
        if(m_bEndOfData == true)
          result = PARSER_ErrorReadFail;
        else
          result = PARSER_ErrorDataUnderRun;
      }
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,"parse_adts_file_header seekandreadfile failed..");
      break;
    }
    uint16 uData = (uint16)((buffer[1] << 8) + buffer[0]);
    // Verify sync word and layer field.
    if (ADTS_HEADER_MASK_RESULT != (uData & ADTS_HEADER_MASK))
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
              "parse_adts_file_header sync word/layer verification failed...");
      result = PARSER_ErrorInHeaderParsing;
      break;
    }
    // Parse and verify frame length.
    uint64 frameLength
          = (static_cast<uint64> (buffer [3] & 0x03) << 11)
          | (static_cast<uint64> (buffer [4]) << 3)
          | (static_cast<uint64> (buffer [5] & 0xE0) >> 5);
#ifdef FEATURE_FILESOURCE_AAC_PARSER_DEBUG
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
                 "parse_adts_file_header frameLength %llu",frameLength);
#endif
    if (pos + frameLength > m_nFileSize)
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
              "parse_adts_file_header frameLength is too big,corrupted file");
      result = PARSER_ErrorInHeaderParsing;
      break;
    }
    // For the first frame, parse additional fields to build decoder
    // specific information.
    if (iframe_count == 0)
    {
      aac_tech_metadata.crc_present = buffer[3] & 0x1;

      aac_tech_metadata.is_private = buffer[2] & 0x2;

      // Parse fields from header.
      /* ID bit - MPEG identifier, set to ‘1’ if the audio data in the ADTS stream is MPEG-2 AAC (see ISO/IEC 13818-7)
         and set to ‘0’ if the audio data is MPEG-4. See also ISO/IEC 11172-3, subclause 2.4.2.3.
         profile_ObjectType - The interpretation of this data element depends on the value of the ID bit.
         If ID is equal to ‘1’ this field holds the same information as the profile field in the ADTS stream
         defined in ISO/IEC 13818-7. If ID is equal to ‘0’ this element denotes the MPEG-4 Audio Object Type (profile_ObjectType+1)
         according to the table defined in subclause 1.5.2.1.
         (buffer [1] & 0x8) value decides whether it is an MPEG2 AAC or MPEG4 AAC.
         In case of MPEG2 AAAC we call this audio_object as profile but in case of MPEG4 AAC we call this as a audio object type.
         in case of MPEG2 what ever the value that we get directly map to the profile because
         0 - AAC_PROFILE_MAIN
         1 - AAC_PROFILE_LC,
         2 - AAC_PROFILE_SSR,
         3 - AAC_PROFILE_LTP
         but in case od MPEG4 object tye mapping will be like this
         0 - Null
         1 - AAC_PROFILE_MAIN
         2 - AAC_PROFILE_LC,
         3 - AAC_PROFILE_SSR,
         4 - AAC_PROFILE_LTP
         5 - SBR
         Adding +1 to make MPEG-2 Audio audio_object type to MPEG-4 Audio Object Type (profile_ObjectType+1) to map properly
         with OMX_AUDIO_AACPROFILETYPE struct.
      */

      aac_tech_metadata.audio_object = uint8(((buffer [2] >> 6) & 0x03)+ 1);

      const uint8 samplingFrequencyIndex = uint8((buffer [2] >> 2) & 0x0F);
      const uint8 channelConfiguration = (uint8)(((buffer [2] << 2) & 0x04) |
                                                 ((buffer [3] >> 6) & 0x03) );

      aac_tech_metadata.channel = channelConfiguration;
      aac_tech_metadata.is_original = (buffer [3]  & 0x20);
      aac_tech_metadata.on_home = (buffer [3]  & 0x10);

      aac_tech_metadata.sample_rate = samplingFrequencyIndex;
      // Decode sampling frequency and convert to frame period.
      //aac_tech_metadata.sample_rate =
      samplingFrequency
               = AAC_SAMPLING_FREQUENCY_TABLE [samplingFrequencyIndex];
      if (samplingFrequency == 0)
      {
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,"parse_adts_file_header invalid samplingFrequency detected...");
        result = PARSER_ErrorInHeaderParsing;
        break;
      }
    }
    pos += frameLength;

    if(pos > (int64)m_nFileSize)
    {
      break;
    }
  }

  // The file has been parsed succefully as ADTS
  // The parameters for ADTS can now be set
  if(PARSER_ErrorNone == result)
  {
    m_aac_format = AAC_FORMAT_ADTS;

    aac_tech_metadata.format_type = m_aac_format;

    // Layer is always 0 as required by spec
    aac_tech_metadata.layer =0;

    // Set unknow values to 0 in metadata
    aac_tech_metadata.bit_rate =0;

    //Copy metadata values to class member
    m_aac_metadata->m_aac_tech_metadata.type = AUDIO_AAC;
    (void) std_memmove(&(m_aac_metadata->m_aac_tech_metadata),
                           &aac_tech_metadata,STD_SIZEOF(tech_data_aac));
  }
  return result;
}
//=============================================================
// FUNCTION : seekandreadfile
//
// DESCRIPTION
//  Seeks to the given position and then reads the desired number of bytes into the
//  buffer provided by the caller.
//
// PARAMETERS
//  length : the no. of bytes required to be read
//  position : position to start reading in the file
//  pbuffer : pointer to the start of buffer for the data to be read into
//  nMaxBufSize: Maximum size of the read buffer
//
// RETURN VALUE
//  Number of bytes read
//
// SIDE EFFECTS
//  None
//=============================================================
//
uint32 aacParser::seekandreadfile (uint32 length,
                                   int64 position,
                                   uint8 *pbuffer,
                                   uint32 nMaxBufSize)
{
#ifdef FEATURE_FILESOURCE_AAC_PARSER_DEBUG
MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"seekandreadfile");
#endif
uint32 nBytesRead = length;
  /* Since this seekandreadfile function is called only while parsing first frame header.
     In this case, we will return readFile failed even for streaming scenarios.*/
  nBytesRead = AACCallbakGetData(position, length, pbuffer, nMaxBufSize,
                                 m_pUserData, m_bEndOfData);
return nBytesRead;
}
//=============================================================
// FUNCTION : parse_generic_file_header
//
// DESCRIPTION
//
//
// PARAMETERS
//  None
//
// RETURN VALUE
//  AAC_SUCCESS : Good State
//  AEE_EBADSTATE : Bad state
//
// SIDE EFFECTS
//  Updates m_filereadpos
//=============================================================
//
PARSER_ERRORTYPE aacParser::parse_generic_file_header()
{
#ifdef FEATURE_FILESOURCE_AAC_PARSER_DEBUG
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"parse_generic_file_header");
#endif
  PARSER_ERRORTYPE result = PARSER_ErrorNone;
  m_filereadpos =0;
  // Set the deafult audio track params for this file
  m_audio_track.start = 0;
  m_audio_track.end = m_nFileSize;
  m_audio_track.size = m_nFileSize;

  result = parse_id3();
#ifdef FEATURE_FILESOURCE_AAC_PARSER_DEBUG
  if(result != PARSER_ErrorNone)
  {
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,"parse_generic_file_header failed... %d", result);
  }
#endif
  /* If ID3 tag parsing is failed because of data underrun, then we will parse one more time.
     Else, we will not parse ID3 tag parsing one more time.*/
  if(PARSER_ErrorDataUnderRun != result)
    m_id3tagparsed = true;
  else
     m_id3tagparsed = false;
  return result;
}

//=============================================================
// FUNCTION : parse_id3
//
// DESCRIPTION
//
//
// PARAMETERS
//  None
//
// RETURN VALUE
//  AAC_SUCCESS : Good State
//  AEE_EBADSTATE : Bad state
//
// SIDE EFFECTS
//  Updates m_filereadpos
//=============================================================
PARSER_ERRORTYPE aacParser::parse_id3()
{
  PARSER_ERRORTYPE status = PARSER_ErrorNone;
  // Stores the intermediate results
  PARSER_ERRORTYPE result = PARSER_ErrorNone;
  bool bID3v2postend      = FALSE;
  bool   bisID3Corrupted  = FALSE;

  uint64 Id3v2size   = 0;
  uint64 fileReadPos = 0;
  uint64 nFileSize   = m_nFileSize;
  uint8* dataBuffer  = (uint8*)m_ReadBuffer;
  uint32 nBytesRead  = 0;

  if((MAX_FILE_SIZE != m_nFileSize) && (false == m_bHTTPStreaming))
  {
    /* It is special case to check whether complete file is downloaded or not.
       If complete file is not downloaded, then dont provide filesize info to
       ID3 library. In this way, we can control ID3 library not to search for
       metadata from the end of the file. */
    nBytesRead = AACCallbakGetData (m_nFileSize - 10, 10,
                                    dataBuffer, AAC_READ_BUFFER_SIZE,
                                    m_pUserData, m_bEndOfData);
    MM_MSG_PRIO2(MM_FILE_OPS, MM_PRIO_HIGH,
                 "dataRead at the end of file is %lu, m_bEndOfData %d",
                 nBytesRead, m_bEndOfData);
    if((0 == nBytesRead) || (FALSE == m_bEndOfData))
    {
      nFileSize = MAX_FILE_SIZE;
    }
  }

  while(m_ReadBuffer)
  {
    /* This flag set to false means, ID3 tags can be searched from end of file.
       But in case of streaming, Parser need not to look for ID3 tags at end.
       Update flag with HTTP stream flag properties. */
    if(FALSE == bID3v2postend)
    {
      bID3v2postend = m_bHTTPStreaming;
    }
     //if we encounter id3v2 at the beginning of the file,
     //we need to keep parsing as there are few clips which have
     //multiple id3v2 tag at the beginning.
     bool bGotId3AtBeg = false;

    // Check for ID3v2 tag
    if(ID3v2::check_ID3v2_present(m_AACFilePtr, nFileSize,
                                  m_audio_track.start, &bID3v2postend))
    {
  #ifdef FEATURE_FILESOURCE_AAC_PARSER_DEBUG
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
                  "parse_generic_file_header located ID3v2");
  #endif
      // Get an object pointer of the ID3v1 parser class
      ID3v2 *aac_ID3v2 = MM_New_Args(ID3v2,(result));
      metadata_id3v2_type stid3v2;
      if(aac_ID3v2)
      {
        // Parse the ID3 tag and save the contents
        result = aac_ID3v2->parse_ID3v2_tag(m_AACFilePtr, m_audio_track.start,
                                            &stid3v2, bID3v2postend);
        // Parse the ID3 tag and save the contents
        if(PARSER_ErrorReadFail == result && m_nFileSize == MAX_FILE_SIZE)
        {
          status = PARSER_ErrorDataUnderRun;
        }
        else if((PARSER_ErrorNone == result) ||(PARSER_ErrorUnsupported == result))
        {
          status = PARSER_ErrorNone;
          m_id3tagfound = true;
          (void) aac_ID3v2->get_ID3v2_size(m_AACFilePtr,m_nFileSize,
                                           m_audio_track.start, bID3v2postend,
                                           &Id3v2size);
          //Store the ID3v2 info structure with aacmetadata
          (void) m_aac_metadata->set_id3v2(&stid3v2);
          // Update the size of the structures in the audio track
          if(bID3v2postend)
          {
            m_audio_track.end -= Id3v2size;
          }
          else
          {
            //id3v2 detected at the beginning, set bGotId3AtBeg to true
            //to parse ahead to make sure there are no subsequent id3v2
            bGotId3AtBeg = true;

            /* Check if ID3 tag is corrupted or not */
            fileReadPos = m_filereadpos;
            m_filereadpos = Id3v2size;
            result = parse_frame_metadata();
            if (PARSER_ErrorUnknownCodecType == result )
            {
              bisID3Corrupted = TRUE;
            }
            m_filereadpos = fileReadPos;

            /* In case of corrupted ID3 tag we need to by-pass the
            ID3 Tag and search for AAC Frame byte by byte till file size. */
            if(bisID3Corrupted)
            {
              m_filereadpos = Id3v2size;
              for(int i = 0; i <  AAC_FORMAT_BUFF_SIZE; i++)
              {
                result = parse_frame_metadata();
                if(PARSER_ErrorUnknownCodecType != result)
                {
                  // Here we got codec info, so break from while loop.
                  break;
                }
                // increment the filereadpos by AAC_FILE_HEADER_SIZE
                m_filereadpos += 1;
              }
              m_audio_track.start = m_filereadpos;

            }
            else
            {
              m_audio_track.start += Id3v2size;
              m_filereadpos += Id3v2size;
            }
          }
          m_audio_track.size -= Id3v2size;

          /* This flag set to false means, ID3 tags can be searched from end of
             file. But in case of streaming, Parser need not to look for ID3
             tags at end. Update flag with HTTP stream flag properties. */
          if (FALSE == bID3v2postend)
          {
            bID3v2postend = m_bHTTPStreaming;
          }
          if( false == ID3v2::check_ID3v2_present(m_AACFilePtr, nFileSize,
                                                  m_audio_track.start,
                                                  &bID3v2postend))
          {
            // As no next ID3v2  ahead so don't scan again. File Offset
            // will be pointing to 1st audio frame. This to break the while(1)
            // instead of running it one more time.
            bGotId3AtBeg = false;
            MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_MEDIUM,
                        "No further ID3v2 present!!");
          }
        }
        MM_Delete( aac_ID3v2);
      }
    }
    // Check for ID3v1 tag only in case of local playback
    if(nFileSize != MAX_FILE_SIZE && status != PARSER_ErrorDataUnderRun &&
       (m_bHTTPStreaming == false) &&
       ID3v1::check_ID3v1_present(m_AACFilePtr, nFileSize))
    {
  #ifdef FEATURE_FILESOURCE_AAC_PARSER_DEBUG
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
                  "parse_generic_file_header located ID3v1");
  #endif
      // Get an object pointer of the ID3v1 parser class
      ID3v1 *aac_ID3v1 = MM_New_Args(ID3v1,(result));
      metadata_id3v1_type stid3v1;
      // Parse the ID3 tag and save the contents
      result = aac_ID3v1->parse_ID3v1_tag(m_AACFilePtr,&stid3v1,m_nFileSize);
      if(PARSER_ErrorNone == result)
      {
        status = PARSER_ErrorNone;
        //Store the ID3v1 info structure with aacmetadata
        (void) m_aac_metadata->set_id3v1(&stid3v1);
        // Update the size of the structures in the audio track
        m_audio_track.end -= ID3v1_SIZE;
        m_audio_track.size -= ID3v1_SIZE;
      }
      MM_Delete( aac_ID3v1);
    }

    if(!bGotId3AtBeg)
    {
      result= PARSER_ErrorNone;
#ifdef FEATURE_FILESOURCE_AAC_PARSER_DEBUG
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
                  "breaking out of parse_id3 loop...");
#endif
      break;
    }
  }
  return status;
}
//=============================================================
// FUNCTION : getMediaTimestampForCurrentSample
//
// DESCRIPTION
//  Returns the timestamp of first frame
//
// PARAMETERS
//  None
//
// RETURN VALUE
//  uint32 : timestamp of the frame
//
// SIDE EFFECTS
//  None
//=============================================================
//
uint64 aacParser::getMediaTimestampForCurrentSample(uint32 /*id*/)
{
  m_isFirstTSValid = false;
  return (m_firstFrameTS - m_baseTS);
}

bool aacParser::GetBaseTime(uint64* nBaseTime)
{
  if(nBaseTime)
  {
    *nBaseTime = m_baseTS;
    return true;
  }
  else
    return false;
}

bool aacParser::SetBaseTime(uint64 nBaseTime)
{
  m_baseTS = nBaseTime;
  return true;
}

//=============================================================
// FUNCTION : getaacformattype
//
// DESCRIPTION
//  Parses the first few bytes of the file header to idenitfy the AAC subtype
//
// PARAMETERS
//  file_header :  buffer containing the file header
//  pFormatType : this function sets the identfied AAC subtype here
//
// RETURN VALUE
//  uint32 : the required bits
//
// SIDE EFFECTS
//  None
//=============================================================
//
aac_format_type aacParser::getaacformattype (const uint8 *file_header) const
{
  aac_format_type formattype = AAC_FORMAT_UNKNOWN;
#ifdef FEATURE_FILESOURCE_AAC_PARSER_DEBUG
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"getaacformattype");
#endif
  const uint8 uLATMMask[9] = {0x2E, 'm','p','4',0x0A,'L','A','T','M'};
  uint16 uData = (uint16)((file_header[1] << 8) + file_header[0]);
  // Check if header type is ADIF
  if(!std_memcmp("ADIF", file_header, 4))
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"getaacformattype ADIF");
    formattype = AAC_FORMAT_ADIF;
  }
  // Check if header type is LOAS
  if( (LOAS_HEADER_MASK_RESULT == (uData & LOAS_HEADER_MASK)) ||
      (!std_memcmp(uLATMMask, file_header,STD_SIZEOF(uLATMMask)) ) )
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"getaacformattype LOAS");
    formattype = AAC_FORMAT_LOAS;
  }
  // Check if header type is ADTS
  if (ADTS_HEADER_MASK_RESULT == (uData & ADTS_HEADER_MASK))
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"getaacformattype ADTS");
    formattype = AAC_FORMAT_ADTS;
  }
  return formattype;
}
//=============================================================
// FUNCTION : parse_loas_file_header
//
// DESCRIPTION
//  Parse a file header according to audio specific format store
//  the required parameters
//
// PARAMETERS
//  file : IFilePort interface to data to be parsed.
//
// RETURN VALUE
//  AAC_SUCCESS: File header was parsed successfully
//  AAC_FAILURE: File  header was INVALID
//
// SIDE EFFECTS
//  None
//=============================================================
//
PARSER_ERRORTYPE aacParser::parse_loas_file_header(uint64 noffset,
                                                   uint32* framesize,
                                                   uint32* framehdr,
                                                   uint32* /*frametime*/)
{
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"parse_loas_file_header");
  PARSER_ERRORTYPE result = PARSER_ErrorNone;
  uint8 samplingFreqIndex = 0;
  uint8 ucData,objType = 0;
  uint32 uOffset = 11;  // past sync bytes
  const uint32 uMaxSize = AAC_ADTS_HEADER_SIZE * 8;
  uint8 buffer[AAC_ADTS_HEADER_SIZE] = {0};
  uint32 buf_len = AAC_ADTS_HEADER_SIZE;
  tech_data_aac aac_tech_metadata;
  aac_tech_metadata.type = AUDIO_AAC;
  uint64 uiloasheaderpos = m_filereadpos;
  //parser configured to o/p one frame at a time
  //get the start offset passed in
  if(framesize && framehdr)
  {
    uiloasheaderpos = noffset;
  }
  uint32 nobytesRead = 0;
  while(result == PARSER_ErrorNone)
  {
    // Seek to reuqired pos and read the file header
    nobytesRead = seekandreadfile(buf_len, uiloasheaderpos, buffer,
                                  AAC_ADTS_HEADER_SIZE);
    if(nobytesRead < buf_len && m_bEndOfData == true)
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,"parse_loas_file_header seekandreadfile failed..");
      result = PARSER_ErrorReadFail;
    }
    else if(nobytesRead < buf_len)
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"parse_loas_file_header seekandreadfile underrun..");
      result = PARSER_ErrorDataUnderRun;
    }
    else
    {
      if(((buffer[0] << 3) | (buffer[1] >> 5)) == 0x2B7)
      {
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"parse_loas_file_header located 0x2B7");
        break;
      }
      if(uiloasheaderpos !=0)
      {
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"parse_loas_file_header uiloasheaderpos!= 0");
        result = PARSER_ErrorInHeaderParsing;
      }
      else
      {
        uiloasheaderpos += 10;
      }
    }
  }//while(result == PARSER_ErrorNone)

  // Check if there was a failure in finding the header sync
  if(result != PARSER_ErrorNone)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,"parse_loas_file_header parse_load_file_header failed..");
    return result;
  }
  else
  {
    bool bError = false;
    // Seek to reuqired pos and read the file header
    nobytesRead = seekandreadfile(buf_len, uiloasheaderpos, buffer,
                                  AAC_ADTS_HEADER_SIZE);
    if(nobytesRead < buf_len && m_bEndOfData == true)
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,"parse_loas_file_header seekandreadfile failed..");
      bError = true;
      result = PARSER_ErrorReadFail;
    }
    else if(nobytesRead < buf_len)
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"parse_loas_file_header seekandreadfile underrun..");
      bError = true;
      result = PARSER_ErrorDataUnderRun;
    }
    else
    {
      uint32 nframelength = getbitsfrombuffer( 13, uOffset, buffer, uMaxSize);
      uOffset += 13; // Skip frame length
      if(framesize)
      {
        *framesize = nframelength + LOAS_FIXED_SYNC_SIZE;
        if(framehdr)
        {
          *framehdr = LOAS_FIXED_SYNC_SIZE;
        }
      }
      //need to parse only when frame size is passed as NULL
      if(!framesize)
      {
        // Get useSameStreamMux
        ucData = static_cast < uint8 > ( getbitsfrombuffer ( 1, uOffset, buffer, uMaxSize ));
        ++uOffset;
        if ( ucData != 0 )
        {
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,"parse_loas_file_header useSameStreamMux parsing failed AAC_CORRUPTED_FILE");
          result = PARSER_ErrorStreamCorrupt;
          bError = true;
        }
        // Get Audio mux version
        ucData = static_cast < uint8 > ( getbitsfrombuffer ( 1, uOffset, buffer, uMaxSize ));
        ++uOffset;
        if ( ucData != 0 )
        {
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,"parse_loas_file_header Audio mux version parsing failed AAC_CORRUPTED_FILE");
          result = PARSER_ErrorStreamCorrupt;
          bError = true;
        }
        // Get allStreamsSameTimeFraming
        ucData = static_cast < uint8 > ( getbitsfrombuffer ( 1, uOffset, buffer, uMaxSize ));
        ++uOffset;
        if ( 0 == ucData )
        {
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,"parse_loas_file_header allStreamsSameTimeFraming parsing failed AAC_CORRUPTED_FILE");
          result = PARSER_ErrorStreamCorrupt;
          bError = true;
        }
        // Get numSubFrames
        ucData = static_cast < uint8 > ( getbitsfrombuffer ( 6, uOffset, buffer, uMaxSize ));
        uOffset += 6;
        if ( ucData != 0 )
        {
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,"parse_loas_file_header numSubFrames parsing AAC_CORRUPTED_FILE");
          result = PARSER_ErrorStreamCorrupt;
          bError = true;
        }
        // Get numProgram
        ucData = static_cast < uint8 > ( getbitsfrombuffer ( 4, uOffset, buffer, uMaxSize ));
        uOffset += 4;
        if ( ucData != 0 )
        {
          bError = true;
          result = PARSER_ErrorStreamCorrupt;
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,"parse_loas_file_header numProgram parsing failed AAC_CORRUPTED_FILE");
        }
        // Get numLayer
        ucData = static_cast < uint8 > ( getbitsfrombuffer ( 3, uOffset, buffer, uMaxSize ));
        uOffset += 3;
        if ( ucData != 0 )
        {
          result = PARSER_ErrorStreamCorrupt;
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,"parse_loas_file_header numLayer parsing failed AAC_CORRUPTED_FILE");
          bError = true;
        }
        objType = static_cast < uint8 > ( getbitsfrombuffer ( 5, uOffset, buffer, uMaxSize ));
          aac_tech_metadata.audio_object = objType;

        // Get Sampling frequency index
        uOffset += 5;
        samplingFreqIndex = static_cast < uint8 > ( getbitsfrombuffer ( 4, uOffset, buffer, uMaxSize ));
        uOffset += 4;
        if (( samplingFreqIndex < 3 ) || ( samplingFreqIndex > 11 ))
        {
          result = PARSER_ErrorStreamCorrupt;
          bError = true;
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,"parse_loas_file_header invalid samplingFreqIndex");
        }
        aac_tech_metadata.sample_rate = samplingFreqIndex;

        // Get Channel numbers
        aac_tech_metadata.channel =
             static_cast < uint8 > ( getbitsfrombuffer ( 4, uOffset, buffer, uMaxSize ));

        // The file has been parsed succefully as LOAS
        // The parameters for LOAS can now be set
        if( (PARSER_ErrorNone == result)&& (bError == false) )
        {
          m_aac_format = AAC_FORMAT_LOAS;
          aac_tech_metadata.format_type = m_aac_format;
          // Set unknow values to 0 in metadata
          aac_tech_metadata.bit_rate =0;
          aac_tech_metadata.layer =0;
          // Copy metadata values to class member
          m_aac_metadata->m_aac_tech_metadata.type = AUDIO_AAC;
          m_aac_metadata->m_aac_tech_metadata = aac_tech_metadata;
          // Update audio track parameters for AAC LOAS
          m_audio_track.start += uiloasheaderpos - m_filereadpos;
          m_audio_track.size -= uiloasheaderpos - m_filereadpos;
        }
      }//if(!framesize)
    }
  }
  return result;
}
//=============================================================
// FUNCTION : parse_adif_file_header
//
// DESCRIPTION
//  Parse a file header according to audio specific format
//  store the required parameters
//
// PARAMETERS
//  file : IFilePort interface to data to be parsed.
//
// RETURN VALUE
//  AAC_SUCCESS: File header was parsed successfully
//  AAC_FAILURE: File  header was INVALID
//
// SIDE EFFECTS
//  None
//=============================================================
//
PARSER_ERRORTYPE aacParser::parse_adif_file_header()
{
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"parse_adif_file_header");
  PARSER_ERRORTYPE result = PARSER_ErrorNone;
  // This is a cache of the sampling frequency of the audio stream, in
  // Hertz.
  unsigned long samplingFrequency = 0;
  uint8 samplingFreqIndex = 0,channelCfg =0;
  uint8 pucFormatData[AAC_ADIF_HEADER_SIZE] = {0};
  uint8 uBitStreamType, i, objType = 0;
  uint8 ucData, ucPfe;
  uint32 uOffset = 0, bit_rate =0;
  const uint32 uMaxSize = AAC_ADIF_HEADER_SIZE * 8;
  tech_data_aac aac_tech_metadata;
  aac_tech_metadata.type = AUDIO_AAC;
  aac_tech_metadata.channel = 0;
  // Seek to pos 4 and read the file header(skip bytes 'A','D','I','F')
  uint32 nobytesRead = seekandreadfile(AAC_ADIF_HEADER_SIZE, m_filereadpos+4,
                                       pucFormatData, AAC_ADIF_HEADER_SIZE);
  if((nobytesRead < AAC_ADIF_HEADER_SIZE) && (m_bEndOfData == true) )
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,"parse_adif_file_header seekandreadfile failed....");
    result = PARSER_ErrorReadFail;
  }
  else if(nobytesRead < AAC_ADIF_HEADER_SIZE)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,"parse_adif_file_header seekandreadfile failed....");
    result = PARSER_ErrorDataUnderRun;
  }
  else
  {
    ucData = static_cast < uint8 >
                 ( getbitsfrombuffer ( 1, uOffset, pucFormatData, uMaxSize ));
    ++uOffset;
    if ( ucData != 0 )
    {
      // Copyright present; Just discard it for now
      uOffset += 72;
    }

    aac_tech_metadata.is_original =
             (boolean) getbitsfrombuffer ( 1, uOffset, pucFormatData, uMaxSize );
    ++uOffset;

    aac_tech_metadata.on_home =
             (boolean) getbitsfrombuffer ( 1, uOffset, pucFormatData, uMaxSize );
    ++uOffset;

    uBitStreamType = static_cast < uint8 >
                   ( getbitsfrombuffer ( 1, uOffset, pucFormatData, uMaxSize ));
    ++uOffset;

    // If this stream type 0, it is CBR - We can provide seek functionality
    m_seek_function_defined = (!uBitStreamType);
    aac_tech_metadata.bit_rate = bit_rate =
             (getbitsfrombuffer(23,uOffset,pucFormatData,uMaxSize ));

    uOffset += 23;
    ucPfe = static_cast < uint8 >
                  ( getbitsfrombuffer ( 4, uOffset, pucFormatData, uMaxSize ));
    uOffset += 4;

    for ( i = 0; i <= ucPfe; i++ )
    {
      // If bit stream is not variable bit rate, skip buffer fullness
      if ( 0 == uBitStreamType )
      {
        uOffset += 20; // skip buffer fullness
      }
      uint32 unFrontChannels, unSideChannels, unBackChannels,
                        unLfeChannels, unAssocData, unValidCC;

      uOffset += 4; // Skip element_instance_tag
      // Get object_type
      aac_tech_metadata.audio_object = objType = static_cast < uint8 >
                ( getbitsfrombuffer ( 2, uOffset, pucFormatData, uMaxSize ));
      /* ID bit - MPEG identifier, set to ‘1’ if the audio data in the ADTS stream is MPEG-2 AAC (see ISO/IEC 13818-7)
         and set to ‘0’ if the audio data is MPEG-4. See also ISO/IEC 11172-3, subclauses 2.4.2.3.
         profile_ObjectType - The interpretation of this data element depends on the value of the ID bit.
         If ID is equal to ‘1’ this field holds the same information as the profile field in the ADTS stream
         defined in ISO/IEC 13818-7. If ID is equal to ‘0’ this element denotes the MPEG-4 Audio Object Type (profile_ObjectType+1)
         according to the table defined in subclause 1.5.2.1.
         (buffer [1] & 0x8) value decides whether it is an MPEG2 AAC or MPEG4 AAC.
         In case of MPEG2 AAAC we call this audio_object as profile but in case of MPEG4 AAC we call this as a audio object type.
         in case of MPEG2 what ever the value that we get directly map to the profile because
         0 - AAC_PROFILE_MAIN
         1 - AAC_PROFILE_LC,
         2 - AAC_PROFILE_SSR,
         3 - AAC_PROFILE_LTP
         but in case od MPEG4 object tye mapping will be like this
         0 - Null
         1 - AAC_PROFILE_MAIN
         2 - AAC_PROFILE_LC,
         3 - AAC_PROFILE_SSR,
         4 - AAC_PROFILE_LTP
         5 - SBR
         Adding +1 to make MPEG-2 Audio audio_object type to MPEG-4 Audio Object Type (profile_ObjectType+1) to map properly
         with OMX_AUDIO_AACPROFILETYPE struct.
      */
      aac_tech_metadata.audio_object++;
      uOffset += 2;
      // Get sampling_frequency_index
      samplingFreqIndex = static_cast<uint8>
                       (getbitsfrombuffer(4,uOffset,pucFormatData,uMaxSize));
      uOffset += 4;
      if ( samplingFreqIndex > 15 )
      {
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,"parse_adif_file_header samplingFreqIndex is invalid....");
        result = PARSER_ErrorInHeaderParsing;
        break;
      }
      else
      {
        if ( 15 == samplingFreqIndex )
        {
          samplingFrequency = getbitsfrombuffer
                             ( 24, uOffset, pucFormatData, uMaxSize );
          uOffset += 24;
        }
        else
        {
          samplingFrequency = AAC_SAMPLING_FREQUENCY_TABLE[samplingFreqIndex];
          if ( 0 == samplingFrequency )
          {
            MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,"parse_adif_file_header samplingFreqIndex is invalid....");
            result = PARSER_ErrorInHeaderParsing;
            break;
          }
        }
      }
      aac_tech_metadata.sample_rate = samplingFreqIndex;
      unFrontChannels = getbitsfrombuffer ( 4, uOffset, pucFormatData, uMaxSize );
      uOffset += 4;
      unSideChannels = getbitsfrombuffer ( 4, uOffset, pucFormatData, uMaxSize );
      uOffset += 4;
      unBackChannels = getbitsfrombuffer ( 4, uOffset, pucFormatData, uMaxSize);
      uOffset += 4;
      unLfeChannels = getbitsfrombuffer ( 2, uOffset, pucFormatData, uMaxSize);
      uOffset += 2;
      unAssocData = getbitsfrombuffer ( 3, uOffset, pucFormatData, uMaxSize );
      uOffset += 3;
      unValidCC = getbitsfrombuffer ( 4, uOffset, pucFormatData, uMaxSize );
      uOffset += 4;
      ucData = static_cast <uint8> ( unFrontChannels + unSideChannels
                                        + unBackChannels + unLfeChannels
                                        + unAssocData + unValidCC );
      channelCfg = ( ucData >= 1 && ucData <= 7 ) ? ucData : 0;

      //since each channel element can be cpe/sce,
      //To calculate total number of channels, we need to add 1 to the sum of all the channel elements.
      channelCfg++;
      aac_tech_metadata.channel = channelCfg;

      // mono_mixdown_present
      ucData = static_cast <uint8> (getbitsfrombuffer(1, uOffset, pucFormatData, uMaxSize));
      ++uOffset;

      if ( ucData != 0 )
      {
        uOffset += 4;
      }

      // stereo_mixdown_present
      ucData = static_cast <uint8> (getbitsfrombuffer(1, uOffset, pucFormatData, uMaxSize));
      ++uOffset;
      if ( ucData != 0 )
      {
        uOffset += 4;
      }
      // matrix_mixdown_idx_present
      ucData = static_cast <uint8> (getbitsfrombuffer(1, uOffset, pucFormatData, uMaxSize));
      ++uOffset;
      if ( ucData != 0 )
      {
        uOffset += 3;
      }
      uOffset += 5 * unFrontChannels + 5 * unSideChannels + 5 * unBackChannels +
               4 * unLfeChannels + 4 * unAssocData + 5 * unValidCC;

      // uint8 align
      const uint32 uRem = ( uOffset % 8 );
      if ( uRem != 0 )
      {
        uOffset += static_cast < uint32 > ( 8 - uRem );
      }

      ucData = static_cast < uint8 >
                     ( getbitsfrombuffer ( 8, uOffset, pucFormatData, uMaxSize ));
      uOffset += 8;
      uOffset += ucData * 8; // skip comment
    }// end for()
    m_n_adif_hdr_len = uOffset/8;
    m_n_adif_hdr_len+=4;

    m_aac_format = (( samplingFreqIndex >= 3 ) && ( samplingFreqIndex <= 11 ) &&
       ( channelCfg <= 6 ) && ( objType < 17 )) ? AAC_FORMAT_ADIF : AAC_FORMAT_RAW;

    // The file has been parsed succefully as ADIF
    // The parameters for ADIF can now be set
    if(PARSER_ErrorNone == result)
    {
      aac_tech_metadata.format_type = m_aac_format;
      // Set unknow values to 0 in metadata
      aac_tech_metadata.layer = 0;
      aac_tech_metadata.is_private = FALSE;
      aac_tech_metadata.crc_present = FALSE;
      aac_tech_metadata.bit_rate = bit_rate;
      // Copy metadata values to class member
      m_aac_metadata->m_aac_tech_metadata.type = AUDIO_AAC;
      (void) std_memmove(&(m_aac_metadata->m_aac_tech_metadata),
                          &aac_tech_metadata,STD_SIZEOF(tech_data_aac));

      uint64 track_len = m_audio_track.size;
      if (bit_rate < 1000)
      {
        // This arrangement of math operations takes care of underflow errors
        m_aac_duration = ((track_len*8)/(bit_rate)) * 1000;
      }
      else
      {
        // This arrangement of math operations takes care of overflow errors
        m_aac_duration = (track_len*8)/(bit_rate/1000);
      }
    }
  }//end of else of if(PARSER_ErrorNone != ret)
  return result;
}
//=============================================================
// FUNCTION : getbitsfrombuffer
//
// DESCRIPTION
//  Gets a bitfield from a buffer
//
// PARAMETERS
//  uNeededBits :  number of bits in the returned bitfield
//  uOffset : bit offset from the data pointer
//  pucInputBuffer : pointer to input data
//
// RETURN VALUE
//  bitfield : the required bits
//
// SIDE EFFECTS
//  None
//=============================================================
//
uint32 aacParser::getbitsfrombuffer (uint32 uNeededBits, uint32 uOffset,
                                     uint8 * pucInputBuffer, uint32 const uMaxSize )
{

uint32 uBitField =0, uMask;

// Make sure we are not going to have to read out of bound
if (( uMaxSize <= uOffset )||( uMaxSize < uOffset + uNeededBits)
        ||(32 <= uNeededBits))
{
return 0xFFFFFFFF; // bitfield failure when running out of data
}

// adjust to next uint8 boundary
pucInputBuffer += (uOffset/8);
uOffset %= 8;

uMask = 0xFF >> uOffset;

uint32 uLeftOffset = 8- uOffset;

// extract the low bits
uint8 uData = *(pucInputBuffer);
uBitField = static_cast < uint32 > (uData & uMask);

// If all the required bits are already available,
// position it correctly and return
if ( uNeededBits <= uLeftOffset)
{
uMask = (1 << uNeededBits) - 1;
return (( uBitField >> (uLeftOffset - uNeededBits )) & uMask );
}

uNeededBits -= uLeftOffset;

// fill with full bytes as needed
while ( uNeededBits >= 8 )
{
uBitField <<= 8;
// Lint throws 661 here because of the error check at the
// beginning of this function. We make sure there is no
// Out of bounds reference here by that error check
//lint --e{661} (Possible creation of out-of-bounds pointer)
uBitField += static_cast < uint32 > (*(++pucInputBuffer));
uNeededBits -= 8;
}

// add remaining bits
if ( uNeededBits > 0 )
{
uMask = (1 << uNeededBits) - 1;
uBitField <<= uNeededBits;
//lint --e{661,662} (Possible creation of out-of-bounds pointer)
uData = *(++pucInputBuffer);
uBitField += static_cast < uint32 >
                            ( uData >> ( 8 - uNeededBits )) & uMask;
}

return uBitField;
}
//=============================================================
// FUNCTION : parse_file_header
//
// DESCRIPTION
//  Parses the file looking for file headers and other non-audio metadata.
//  Identifies the AAC format and stores this value. For thsi AAC format, the
//  start and stop file position of audio data is determined.
//
// PARAMETERS
//  file : IFilePort interface to data to be parsed.
//
// RETURN VALUE
//  An indicator of success or failure of the method is returned.
//
// SIDE EFFECTS
//  None
//
// NOTE
//  Supports AAC-ADIF, AAC-ADTS and AAC-LOAS formats
//=============================================================
//
PARSER_ERRORTYPE aacParser::parse_file_header ()
{
#ifdef FEATURE_FILESOURCE_AAC_PARSER_DEBUG
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"parse_file_header");
#endif
  PARSER_ERRORTYPE result = PARSER_ErrorNone;

  // Validate input parameters
  if(NULL == m_AACFilePtr)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,"parse_file_header m_AACFilePtr is NULL!!");
    result = PARSER_ErrorInvalidParam;
  }
  else
  {
    if(!m_aac_metadata)
    {
      m_aac_metadata = MM_New_Args(aacmetadata,());
      if (NULL == m_aac_metadata)
      {
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,"aacParser::aacParser failed to allocate aacmetadata object");
        return PARSER_ErrorMemAllocFail;
      }
      m_id3tagparsed = false;
    }
    m_aac_format = AAC_FORMAT_UNKNOWN;
    m_aac_duration = 0;
    // Parse generic fields in the file header, if ID3 tag parsing is not yet completed due to data underrun
    if(false == m_id3tagparsed)
    {
      result = parse_generic_file_header();
    }
    if(PARSER_ErrorNone == result)
    {
       metadata_id3v2_type *stid3v2 = m_aac_metadata->get_id3v2(0);
       if(stid3v2)
       {
         /* Update timestamp variables in object with the timestamps, that
            are parsed from id3 tags*/
         m_firstFrameTS = stid3v2->private_tag.timeStamp;
         m_isFirstTSValid = stid3v2->private_tag.isTsValid;
         /* If firstframe TS is valid, then set that as base Timestamp*/
         if(m_isFirstTSValid)
           m_baseTS = m_firstFrameTS;
       }

      result = parse_frame_metadata();
    }
  }//end of else of if(NULL == m_AACFilePtr)
  if (result == PARSER_ErrorNone)
  {
    aac_file_format = true;
  }
  return result;
}
//=============================================================
// FUNCTION : parse_frame_metadata
//
// DESCRIPTION
//  Parses the first frame's properties and stores in class variable.
//
// PARAMETERS
//  file : IFilePort interface to data to be parsed.
//
// RETURN VALUE
//  An indicator of success or failure of the method is returned.
//
// SIDE EFFECTS
//  None
//
// NOTE
//  Supports AAC-ADIF, AAC-ADTS and AAC-LOAS formats
//=============================================================
PARSER_ERRORTYPE aacParser::parse_frame_metadata ()
{
  PARSER_ERRORTYPE result = PARSER_ErrorNone;
  uint8 file_header[AAC_FILE_HEADER_SIZE] = {0};
  // Seek to pos 0 and read the file header
  uint32 nobytesRead = seekandreadfile(AAC_FILE_HEADER_SIZE, m_filereadpos,
                                       file_header, AAC_FILE_HEADER_SIZE);
  if(nobytesRead < AAC_FILE_HEADER_SIZE && m_bEndOfData == true)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,"parse_file_header seekandreadfile failed");
    result = PARSER_ErrorReadFail;
  }
  else if(nobytesRead < AAC_FILE_HEADER_SIZE)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,"parse_file_header seekandreadfile underrun");
    result = PARSER_ErrorDataUnderRun;
  }
  else
  {
    aac_format_type FormatType = getaacformattype(file_header);
    switch(FormatType)
    {
      case AAC_FORMAT_ADTS:
      {
        result = parse_adts_file_header();
        break;
      }
      case AAC_FORMAT_ADIF:
      {
        result = parse_adif_file_header();
        break;
      }
      case AAC_FORMAT_LOAS:
      {
        result = parse_loas_file_header();
        break;
      }

      case AAC_FORMAT_RAW:
      case AAC_FORMAT_UNKNOWN:
      default:
      {
#ifdef FEATURE_FILESOURCE_AAC_PARSER_DEBUG
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"parse_file_header UNKNOWN_FORMAT");
#endif
        result = PARSER_ErrorUnknownCodecType;
        break;
      }
    }//switch(FormatType)
  }//end of else of if(ret != PARSER_ErrorNone)

  return result;
}
//=============================================================================
// FUNCTION is_aac_sync
//
// DESCRIPTION
//  This function parses the given data to check if it is AAC frame sync word
//
// PARAMETERS
//  frame : AAC data
//
// RETURN VALUE
//  true : given data is AAC frame sync word
//  false : given data is not AAC frame sync word
//
// SIDE EFFECTS
//  None.
//=============================================================================
//
bool aacParser::is_aac_sync(const uint8* frame) const
{
  bool bret = false;
  // Check for sync word
  if (0xFF == frame[0] && 0xF0 == (frame[1] & 0xF0))
  {
    bret = true;
  }
  return bret;
}
//=============================================================================
// FUNCTION : get_accurate_seek_position
//
// DESCRIPTION
//  Calculates seek position based on time. This function is only implemented
//  when the seek_function_defined is set to TRUE
//
// PARAMETERS
//  time  Presentation time to seek to
//  file_position  File position corresponding to the seek time provided
//  SeekedTime  Output Seek Time
//
// RETURN VALUE
//  AEE_SUCCESS : Function succesfully calculated the seek position
//  AEE_EBADPARM : Invalid input parameters
//  AEE_EBADSTATE : Invalid format parser state
//  AEE_EUNSUPPORTED : Function is not implemented for this audio format
//
// SIDE EFFECTS
//  None
//=============================================================================
PARSER_ERRORTYPE  aacParser::get_accurate_seek_position (uint64 RequestedTime,
                                                         uint64* FilePosition,
                                                         uint64* SeekedTime)
{
  PARSER_ERRORTYPE error = PARSER_ErrorNone;
  bool bEndOfData    = false;
  uint32 nMaxBufSize = m_aac_audio_info.dwSuggestedBufferSize;
  uint32 nNumBytesRequest = nMaxBufSize;
  uint32 nBytesRead = 0;
  uint8 *dataBuffer = 0;
  uint32 nBytesSkipped = 0;
  uint32 FrameBufLen = 0;
  uint32 FrameTime   = 0;
  uint32 FrameLen    = AAC_FILE_HEADER_SIZE;
  uint32 NumFramesSkipped = 0;
  uint64 ElapsedTime = 0;
  uint64 LastFrameTime = 0;
  uint64 CurrentOffset = m_audio_track.start;
  uint64 LastFrameOffset = m_audio_track.start;

  if(!FilePosition || !SeekedTime)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                "i/p var is not correct!!");
    return PARSER_ErrorInvalidParam;
  }
  *FilePosition = 0;
  *SeekedTime = 0;

  dataBuffer = (uint8 *)MM_Malloc(nMaxBufSize);
  if(0 == dataBuffer)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                "Malloc is failed in seek function");
    return PARSER_ErrorMemAllocFail;
  }

  /* Parse frame by frame till we parse the frame closest to the RequestedTime*/
  while(ElapsedTime <= RequestedTime)
  {
    bool frameStartFound = false;
    /* 1) If there is no data available, then read data from the file.

       2) If data is available, then read data only if at least one frame is
          parsed. "nBytesSkipped" will keep track of number of junk data bytes.

       3) If the data read is completely junk data, then also read data from
          new offset value. */
    if((!nBytesRead) || (nBytesRead == nBytesSkipped) ||
       ((nBytesRead >= (nBytesSkipped + FrameBufLen) && (FrameBufLen))))
    {
      CurrentOffset += (nBytesSkipped + FrameBufLen);
      nBytesRead = AACCallbakGetData (CurrentOffset, nNumBytesRequest, dataBuffer,
                                      nMaxBufSize, m_pUserData, bEndOfData);
      nBytesSkipped = 0;
      FrameBufLen   = 0;
    }
    if(nBytesRead < FrameLen && bEndOfData == true)
    {
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_ERROR,
                   "GetCurrentSample reaches EOF @ %llu", CurrentOffset);
      error = PARSER_ErrorEndOfFile;
      break;
    }
    else if(nBytesRead < FrameLen)
    {
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
                   "GetCurrentSample AAC_DATA_UNDERRUN @ %llu", CurrentOffset);
      error = PARSER_ErrorDataUnderRun;
      break;
    }

    /* Calc frame length and time and update consumed buf size accordingly */
    while(ElapsedTime <= RequestedTime && nBytesRead > FrameBufLen + nBytesSkipped &&
          is_aac_sync(dataBuffer+FrameBufLen + nBytesSkipped) &&
          parse_frame_header(dataBuffer+FrameBufLen + nBytesSkipped, &FrameLen, &FrameTime))
    {
      if(nMaxBufSize < FrameLen)
      {
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL, "GetCurrentSample: Buf size is smaller than frame size!");
        MM_Free(dataBuffer);
        /* Double the buf size */
        nMaxBufSize   *= 2;
        nBytesRead     = 0;
        dataBuffer     = 0;
        dataBuffer     = (uint8 *)MM_Malloc(nMaxBufSize);
        if(0 == dataBuffer)
        {
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                      "Malloc is failed in seek function");
          return PARSER_ErrorMemAllocFail;
        }
        break;
      }

      /* Check whether enough data is available in the buffer or not,
         before updating FrameBufLen param*/
      if((nBytesRead - FrameBufLen) < FrameLen)
      {
        nBytesRead = 0;
        break;
      }

      frameStartFound = true;
      ElapsedTime += FrameTime;
      if(ElapsedTime <= RequestedTime)
      {
        LastFrameOffset = CurrentOffset + FrameBufLen + nBytesSkipped;
        LastFrameTime = ElapsedTime;
        NumFramesSkipped++;
      }
      FrameBufLen += FrameLen;
    }
    if((nBytesRead) && (false == frameStartFound) &&
       (nBytesRead > (FrameBufLen + nBytesSkipped)))
    {
      nBytesSkipped++;
    }
  }
  /* Free the memory allocated */
  MM_Free(dataBuffer);
  dataBuffer = 0;

  /* Check the closest of the two frames w.r.t RequestedTime and seek to that loc*/
  if(error == PARSER_ErrorNone)
  {
    if((ElapsedTime - RequestedTime) < (RequestedTime - LastFrameTime))
    {
      *FilePosition = CurrentOffset + FrameBufLen + nBytesSkipped;
      *SeekedTime = ElapsedTime;
    }
    else
    {
      *FilePosition = LastFrameOffset;
      *SeekedTime = LastFrameTime;
    }
  }
  else if(PARSER_ErrorEndOfFile == error)
  {
    *FilePosition = LastFrameOffset;
    *SeekedTime   = LastFrameTime;
    error = PARSER_ErrorNone;
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_MEDIUM,
                 "Seek to last frame @ %llu", LastFrameOffset);
  }

  return error;
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
FileSourceStatus aacParser::SetAudioOutputMode(FileSourceConfigItemEnum henum)
{
  FileSourceStatus status = FILE_SOURCE_FAIL;
  //Default mode is FILE_SOURCE_MEDIA_OUTPUT_BYTES_STREAM/FILE_SOURCE_MEDIA_RETAIN_AUDIO_HEADER
  //We do not support changing output mode during the playback
  switch (henum)
  {
  case FILE_SOURCE_MEDIA_OUTPUT_SINGLE_AUDIO_FRAME:
    //parser can detect frame boundaries in case of ADTS/LOAS only
    //thus SetConfiguration for single frame should be accepeted when aac type is adts
    if( (m_hFrameOutputModeEnum == FILE_SOURCE_MEDIA_OUTPUT_BYTES_STREAM) &&
       ((m_aac_format == AAC_FORMAT_ADTS) || (m_aac_format == AAC_FORMAT_LOAS) ) )
    {
      m_hFrameOutputModeEnum = henum;
      status = FILE_SOURCE_SUCCESS;
    }
    break;
  case FILE_SOURCE_MEDIA_STRIP_AUDIO_HEADER:
    if (m_hHeaderOutputModeEnum == FILE_SOURCE_MEDIA_RETAIN_AUDIO_HEADER)
    {
      m_hHeaderOutputModeEnum = henum;
      status = FILE_SOURCE_SUCCESS;
    }
    break;
  default:
    break;
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
 FILE_SOURCE_SUCCESS if successful in retrieving output mode else returns FILE_SOURCE_FAIL

SIDE EFFECTS:
  None.
======================================================================*/
FileSourceStatus aacParser::GetAudioOutputMode(bool* bret, FileSourceConfigItemEnum henum)
{
  FileSourceStatus status = FILE_SOURCE_FAIL;
  //Default mode is FILE_SOURCE_MEDIA_OUTPUT_BYTES_STREAM/FILE_SOURCE_MEDIA_RETAIN_AUDIO_HEADER
  //We do not support changing output mode during the playback
  switch (henum)
  {
    case FILE_SOURCE_MEDIA_OUTPUT_SINGLE_AUDIO_FRAME:
      if (m_hFrameOutputModeEnum == henum)
      {
        *bret = true;
        status = FILE_SOURCE_SUCCESS;
      }
      break;
      case FILE_SOURCE_MEDIA_STRIP_AUDIO_HEADER:
      if (m_hHeaderOutputModeEnum == henum)
      {
        *bret = true;
        status = FILE_SOURCE_SUCCESS;
      }
      break;
    default:
      break;
  }
  return status;
}

