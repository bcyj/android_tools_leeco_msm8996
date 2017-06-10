/* =======================================================================
mp3parser.cpp
DESCRIPTION
It has the definitions for the functions related to mp3parser class.
These functions will be used to parse metadata of MP3 files and to extract
frame data.

EXTERNALIZED FUNCTIONS
List functions and a brief description that are exported from this file

INITIALIZATION AND SEQUENCING REQUIREMENTS
Detail how to initialize and use this service.  The sequencing aspect
is only needed if the order of operations is important.

Copyright 2009-2015 Qualcomm Technologies, Inc., All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
========================================================================== */
/* =======================================================================
                               Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/MP3ParserLib/main/latest/src/mp3parser.cpp#70 $
$DateTime: 2014/02/07 02:53:30 $
$Change: 5240686 $

========================================================================== */

#include "mp3parser.h"
#include "mp3vbrheader.h"
#include "seektable.h"
#include "filebase.h"
#include "seek.h"
#include "MMMemory.h"
#include "MMDebugMsg.h"

#define MIN_SEEK_DELTA (100)
const uint8 mp3Parser::m_header_size = 4;

//=============================================================================
// CONSTANTS
//=============================================================================
class mp3metadata;
class mp3vbrheader;
//=============================================================================
// FUNCTION: Constructor
//
// DESCRIPTION:
//  Create an instance of mp3formatparser, and init the class attributes
//
// PARAMETERS
//  pUData : This is the interface to the component services environment
//  fsize : AEE_SUCCESS - success
//                 AEE_EBADPARM - failure
//
// RETURN:
//  None
//
// SIDE EFFECTS
//  None
//=============================================================================
//
mp3Parser::mp3Parser(void* pUserData, uint64 fsize,OSCL_FILE *FilePtr,
                     bool bHTTPStreaming)
{
  // Preffered number of buffers for MP3
  m_pref_buf_num = 4;
  // length of sync word of MP3 audio frame
  m_sync_len = 2;
  // maximum possible length of a MP3 frame
  m_max_frame_size = 3000;
  // maximum number of frames that can be send in one o/p buffer
  m_max_frames_in_output = 1;
  m_pUserData = pUserData;
  m_nFileSize = fsize;
  m_CurrentParserState = PARSER_IDLE;
  m_MP3FilePtr = FilePtr;
  m_nCurrOffset = 0;
  m_psimple_seektable = NULL;
  m_pseek = NULL;
  m_bLAMETagAvailable      = false;
  m_parse_file_header_done = false;
  m_seek_function_defined  = false;
  m_id3tagfound = false;
  m_id3tagparsed = false;
  m_firstFrame = false;
  m_isFirstTSValid = false;
  m_firstFrameTS = 0;
  m_baseTS = 0;
  m_SeekDone = false;
  m_vbr_header = NULL;
  m_metadata = NULL;
  m_duration = 0;
  m_is_vbr = false;
  m_dataBuffer = NULL;
  m_hFrameOutputModeEnum  = FILE_SOURCE_MEDIA_OUTPUT_BYTES_STREAM;
  m_hHeaderOutputModeEnum = FILE_SOURCE_MEDIA_RETAIN_AUDIO_HEADER;
  m_nCurrentTime = 0;
  m_nNextFrameTime = 0;
  m_bHTTPStreaming = bHTTPStreaming;
  m_ulEncoderDelay = 0;
  m_ulPaddingDelay = 0;
  m_nFracResidualTs= 0;
  m_FrameTimeinUS  = 0;
  memset (&m_audio_track, 0, sizeof(m_audio_track));
  memset(&m_mp3_header_mp3h,0,sizeof(mp3_header_mp3h));
  memset(&m_mp3_audio_info,0,sizeof(mp3_audio_info));
  memset (&m_header_info, 0, sizeof(m_header_info));
  memset (&m_mpeg1_tag_info, 0, sizeof(mpeg1_tag));
  memset (&m_ReadBuffer, 0, MP3_READ_BUFFER_SIZE);
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
mp3Parser::~mp3Parser()
{
  if(m_psimple_seektable)
  {
    MM_Delete( m_psimple_seektable);
    m_psimple_seektable = NULL;
  }
  if(m_pseek)
  {
    MM_Delete( m_pseek);
    m_pseek=NULL;
  }
  if(m_metadata)
  {
    MM_Delete(m_metadata);
  }
  if(m_vbr_header)
  {
    MM_Delete(m_vbr_header);
    m_vbr_header = 0;
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
//  amrErrorType
//
// SIDE EFFECTS
//  None
//=============================================================================
//
PARSER_ERRORTYPE mp3Parser::StartParsing(void)
{
  PARSER_ERRORTYPE status = PARSER_ErrorDefault;
  status = parse_file_header();
  if(PARSER_ErrorNone == status)
  {
    m_mp3_header_mp3h.nSampleRate          = m_header_info.samplerate;
    m_mp3_audio_info.dwSuggestedBufferSize = m_max_frame_size;
    update_mpeg1_tag_info();
    m_nCurrOffset += m_audio_track.start;
    m_CurrentParserState = PARSER_READY;
    return PARSER_ErrorNone;
  }
  if(PARSER_ErrorDataUnderRun == status)
    m_CurrentParserState = PARSER_UNDERRUN;
  return status;
}

//=============================================================================
// FUNCTION: update_mpeg1_tag_info
//
// DESCRIPTION:
//  updates mpeg1 tag information
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
void mp3Parser::update_mpeg1_tag_info()
{
  switch (m_header_info.layer)
  {
    case MP3_LAYER_3:
      m_mpeg1_tag_info.fwHeadLayer = MP3_ACM_MPEG_LAYER3;

      switch (m_header_info.extension)
      {
        case MP3_EXT_INTENSITY_OFF_MS_OFF:
          m_mpeg1_tag_info.fwHeadModeExt = 0;
          break;
        case MP3_EXT_INTENSITY_ON_MS_OFF:
          m_mpeg1_tag_info.fwHeadModeExt = 2;
          break;
        case MP3_EXT_INTENSITY_OFF_MS_ON:
          m_mpeg1_tag_info.fwHeadModeExt = 4;
          break;
        case MP3_EXT_INTENSITY_ON_MS_ON:
          m_mpeg1_tag_info.fwHeadModeExt = 8;
          break;
      }
      break;
    default:
      break;
  }

  m_mpeg1_tag_info.dwHeadBitrate = m_header_info.bitrate;

  switch (m_header_info.channel)
  {
    case MP3_CHANNEL_STEREO:
      m_mp3_header_mp3h.nChannels = 2;
      m_mpeg1_tag_info.fwHeadMode = MP3_ACM_MPEG_STEREO;
      m_mpeg1_tag_info.fwHeadModeExt = 0;
      break;
    case MP3_CHANNEL_JOINT_STEREO:
      m_mp3_header_mp3h.nChannels = 2;
      m_mpeg1_tag_info.fwHeadMode = MP3_ACM_MPEG_JOINTSTEREO;
      break;
    case MP3_CHANNEL_DUAL:
      m_mp3_header_mp3h.nChannels = 2;
      m_mpeg1_tag_info.fwHeadMode = MP3_ACM_MPEG_DUALCHANNEL;
      m_mpeg1_tag_info.fwHeadModeExt = 0;
      break;
    case MP3_CHANNEL_SINGLE:
      m_mp3_header_mp3h.nChannels = 1;
      m_mpeg1_tag_info.fwHeadMode = MP3_ACM_MPEG_SINGLECHANNEL;
      m_mpeg1_tag_info.fwHeadModeExt = 0;
      break;
  }

  switch (m_header_info.emphasis)
  {
    case MP3_EMPHASIS_NONE:
      m_mpeg1_tag_info.wHeadEmphasis = 1;
      break;
    case MP3_EMPHASIS_50_15_MS:
      m_mpeg1_tag_info.wHeadEmphasis = 2;
      break;
    case MP3_EMPHASIS_RESERVED:
      m_mpeg1_tag_info.wHeadEmphasis = 3;
      break;
    case MP3_EMPHASIS_CCITT_J17:
      m_mpeg1_tag_info.wHeadEmphasis = 4;
      break;
  }

  if (TRUE == m_header_info.is_private)
  {
    m_mpeg1_tag_info.fwHeadFlags |= MP3_ACM_MPEG_PRIVATEBIT;
  }

  if (TRUE == m_header_info.copyright_present)
  {
    m_mpeg1_tag_info.fwHeadFlags |= MP3_ACM_MPEG_COPYRIGHT;
  }

  if (TRUE == m_header_info.is_original)
  {
    m_mpeg1_tag_info.fwHeadFlags |= MP3_ACM_MPEG_ORIGINALHOME;
  }

  if (TRUE == m_header_info.is_original)
  {
    m_mpeg1_tag_info.fwHeadFlags |= MP3_ACM_MPEG_PROTECTIONBIT;
  }

  m_mpeg1_tag_info.fwHeadFlags |= MP3_ACM_MPEG_ID_MPEG1;

  m_mpeg1_tag_info.dwPTSLow = 0;

  m_mpeg1_tag_info.dwPTSHigh = 0;

}

//=============================================================================
// FUNCTION: GetMP3Header
//
// DESCRIPTION:
//  Parse the mp3 file header
//
// PARAMETERS
//  pMp3HdrPtr - pointer to mp3 header structure
//
// RETURN:
//  mp3ErrorType
//
// SIDE EFFECTS
//  None
//=============================================================================
PARSER_ERRORTYPE mp3Parser::GetMP3Header(mp3_header_mp3h* pMp3HdrPtr)
{
  PARSER_ERRORTYPE retError = PARSER_ErrorNone;

  if(!pMp3HdrPtr)
  {
    //MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
    //   "GetAVIHeader AMR_INVALID_USER_DATA");
    retError = PARSER_ErrorInvalidParam;
    return retError;
  }
  if(NULL == &m_mp3_header_mp3h)
  {
    //MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
    //   "GetMP3Header,NULL MP3 Header,MP3_PARSE_ERROR");
    retError = PARSER_ErrorDefault;
    return retError;
  }
  pMp3HdrPtr->nChannels = m_mp3_header_mp3h.nChannels;
  pMp3HdrPtr->nSampleRate = m_mp3_header_mp3h.nSampleRate;
  return retError;
}
//=============================================================================
// FUNCTION: GetMP3DecodeInfo
//
// DESCRIPTION:
//  Get MP3 Decode specific Information
//
// PARAMETERS
// <> mpeg1_tag structure pointer
//
// RETURN:
//  mp3ErrorType
//
// SIDE EFFECTS
//  None
//=============================================================================
PARSER_ERRORTYPE mp3Parser::GetMP3DecodeInfo(mpeg1_tag *p_mpeg1_tag_info)
{
  if(NULL != &m_mpeg1_tag_info)
  {
    memcpy(p_mpeg1_tag_info,&m_mpeg1_tag_info,sizeof(m_mpeg1_tag_info));
    return PARSER_ErrorNone;
  }
  return PARSER_ErrorDefault;
}

//=============================================================================
// FUNCTION: GetAudioInfo
//
// DESCRIPTION:
// This function returns audio format specific information
//
// PARAMETERS
//  pAudioInfo - pointer to the amr_audio_info structure
//
// RETURN:
//  amrErrorType
//
// SIDE EFFECTS
//  None
//=============================================================================
PARSER_ERRORTYPE mp3Parser::GetAudioInfo(mp3_audio_info* pAudioInfo)
{
  PARSER_ERRORTYPE retError = PARSER_ErrorNone;

  if(NULL == &m_mp3_audio_info)
  {
    retError = PARSER_ErrorDefault;
    return retError;
  }
  if(NULL == pAudioInfo)
  {
    retError = PARSER_ErrorInvalidParam;
    return retError;
  }
  memset(pAudioInfo,0,sizeof(mp3_audio_info));
  memcpy(pAudioInfo,&m_mp3_audio_info,sizeof(mp3_audio_info));
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
uint64 mp3Parser::GetClipDurationInMsec()
{
  uint64 uDuration = 0;
  uint32 noEntries = 50;
  if (m_duration)
  {
    return m_duration;
  }


  /* If file size info is not known, we will not calculate the duration info*/
  if(MAX_FILE_SIZE == m_nFileSize)
      return uDuration;

  if(!m_seek_function_defined)
  {
    if(m_psimple_seektable == NULL)
    {
      m_psimple_seektable = MM_New_Args(simple_seektable,(noEntries));
    }

    if(m_pseek == NULL && m_psimple_seektable != NULL)
    {
      m_pseek = MM_New_Args(seek,(m_psimple_seektable
                         ,this
                         ,m_MP3FilePtr
                         ,MP3_FILE_HEADER_SIZE
                         ,m_audio_track.start
                         ,m_audio_track.end));
    }
    if( NULL != m_pseek)
    {
      m_duration = uDuration = m_pseek->get_duration();

      if(!uDuration)
      {
        return 0;
      }

      if(m_pseek->set_mode(PREGENERATE_TABLE))
      {
        return 0;
      }
    }
    return uDuration;
  }
  else
  {
    (void)get_duration(&uDuration);
    m_duration = uDuration;
    return uDuration;
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
//  frame :  A byte pointer to frame_size bytes, the frame header to be parsed
//              Note: the entire frame header is expected, thus it must be
//              at least  <em>frame_header_size</em> in length.
//  frame_size : frame size determine from parsing the frame header
//  frame_time : frame duration in miliseconds determine from parsing frame
//               header
//
// RETURN VALUE
//  AEE_SUCCESS : Frame header was valid, and frame size is returned
//  AEE_EBADPARM : Invalid input parameters
//  AEE_EBADSTATE : Invalid format parser state
//  AEE_EFAILED : Frame header was INVALID, frame size is not valid
//
// SIDE EFFECTS
//  None
//=============================================================================
IAudioReturnType mp3Parser::parse_frame_header (/*in*/uint8* frame,
                                                /*rout*/uint32* frame_size,
                                                /*out*/uint32* frame_time)
{
  IAudioReturnType result = IAUDIO_FAILURE;

  // Validate input parameters
  if((NULL == frame)||(NULL == frame_size)||(NULL == frame_time))
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
                  "mp3parser::parse_frame_header: frame/frame_time/frame_size is NULL!");
    result = IAUDIO_FAILURE;
  }
  else
  {
    // Initialize the output variables
    *frame_size = 0;
    *frame_time = 0;
    // Check for format parser state
    if (!m_parse_file_header_done)
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
                    "mp3parser::parse_frame_header:m_parse_file_header_done is FALSE");
      result = IAUDIO_FAILURE;
    }
    else
    {
      struct tech_data_mp3 header_info;
      memset (&header_info, 0, sizeof(header_info));
      // Parse the frame header
      if (!parse_mp3_frame_header(frame,header_info) &&
          (PARSER_READY != m_CurrentParserState ||
          (m_header_info.layer == header_info.layer &&
           m_header_info.version ==  header_info.version &&
           m_header_info.samplerate == header_info.samplerate) ) )
      {
        // Successfully parsed the frame header - copy the frame size
        *frame_size = calc_frame_length(header_info);
        // Account for loss of precision in frame length calculation
        /*if ((0 != (*frame_size)))
        {
         (*frame_size)--;
        }*/
        // Update the frame time
        *frame_time = (uint32)calc_frame_time(header_info);
        result = IAUDIO_SUCCESS;
     }
     else
     {
#ifdef FEATURE_FILESOURCE_MP3_PARSER_DEBUG
       MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
                    "mp3parser::parse_frame_header:parse_mp3_header failed...");
#endif
       result = IAUDIO_FAILURE;
     }
    }
  }
  return result;
}
//=============================================================================
// FUNCTION calc_frame_time
//
// DESCRIPTION
//  This function calculates the duration of an MP3 frame based on information
//  stored in the given MP3 header.
//
// PARAMETERS
//  header_info: MP3 header info
//
// RETURN VALUE
//  The frame time in miliseconds
//
// SIDE EFFECTS
//  None.
//=============================================================================
uint64 mp3Parser::calc_frame_time (const struct tech_data_mp3 &header_info)
{
   // Validate header info
   if (0 == header_info.samplerate) {
      //DBG(HIGH,"mp3formatparser::calc_frame_time:"
         //" samplerate is 0");
      return 0;
   }

   float  fFrameTime = 0;

   // Calculate frame time
   fFrameTime = ((MP3_SAMPLES_TABLE
                [(m_header_info.version == MP3_VER_1 ? 0 : 1)]
                [header_info.layer] * (float)1000))
                / header_info.samplerate;

   //! Update fractional time in micro-sec units
   m_nFracResidualTs = (uint32)(fFrameTime - (uint64)fFrameTime)*1000;

   return (uint64)fFrameTime;
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
//  amrErrorType
//
// SIDE EFFECTS
//  None
//=============================================================================
//
PARSER_ERRORTYPE mp3Parser::GetCurrentSample(uint8* dataBuffer,
                                             uint32 nMaxBufSize,
                                             uint32 *nBytesNeeded,
                                             bool bUpdateTime)
{
  if( (!dataBuffer) || (!nMaxBufSize) ||  (!nBytesNeeded) )
  {
    //MM_MSG_PRIO(MM_FILE_OPS,
    //    MM_PRIO_HIGH,
    //  "GetCurrentSample AMR_INVALID_USER_DATA");
    return PARSER_ErrorInvalidParam;
  }

  if((m_nFileSize != MAX_FILE_SIZE) &&
     (m_nFileSize <= m_nCurrOffset + MIN_FRAME_HEADER_SIZE))
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR, "GetCurrentSample EOF reached");
    *nBytesNeeded = 0;
    return PARSER_ErrorEndOfFile;
  }

  /* Initialize the variables*/
  bool bEndOfData = false;
  uint32 nNumBytesRequest = nMaxBufSize;
  uint32 nBytesRead = 0;
  tech_data_mp3 header_info;
  bool frameStartFound = false;
  uint32 nBytesSkipped = 0;
  uint32 nFrameBufLen = 0;
  m_dataBuffer = dataBuffer;
  memset(&header_info, 0 , sizeof(tech_data_mp3));

  //If frame header is located, points to the frame time
  uint32 nframeTime = 0;
  uint32 nframeLen = 0;
  /* This flag will be marked as false always. If ID3 tags are found in middle,
     then this flag will be marked as true. */
  m_firstFrame = false;
  while(false == frameStartFound)
  {
    /* If there is no data available, then read data from the file*/
    if(!nBytesRead)
    {
      m_nCurrOffset += nBytesSkipped;
      nBytesRead = MP3CallbakGetData (m_nCurrOffset, nNumBytesRequest,
                                      m_dataBuffer, nMaxBufSize, m_pUserData,
                                      bEndOfData);
      nBytesSkipped = 0;
    }
    if(!nBytesRead && true == bEndOfData)
    {
      *nBytesNeeded  = 0;
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL, "GetCurrentSample MP3_READ_FAILURE");
      return PARSER_ErrorEndOfFile;
    }
    else if(!nBytesRead)
    {
      *nBytesNeeded  = 0;
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH, "GetCurrentSample MP3_DATA_UNDERRUN");
      return PARSER_ErrorDataUnderRun;
    }

    /* Calc frame length in both cbr and vbr clips and update output buf size accordingly.
       Both Samplerate and bitrate values should be non-zero */
     while(nBytesRead - nFrameBufLen > MIN_FRAME_HEADER_SIZE &&
           IAUDIO_SUCCESS == parse_frame_header(dataBuffer + nBytesSkipped + nFrameBufLen,
                                                   &nframeLen,&nframeTime) )
    {
      if(nMaxBufSize < nframeLen)
      {
         MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL, "GetCurrentSample: Buf size is smaller than frame size!");
         return PARSER_ErrorInsufficientBufSize;
      }
      //! During resync scenario, validate next frame also
      if (nBytesSkipped)
      {
        uint32 ulNextFrameLen  = 0;
        uint32 ulNextFrameTime = 0;
        //! If complete frame is available, then validate next frame
        if (nBytesRead > MIN_FRAME_HEADER_SIZE + nframeLen)
        {
          if (IAUDIO_SUCCESS !=
              parse_frame_header(dataBuffer + nBytesSkipped + nFrameBufLen + nframeLen,
                                 &ulNextFrameLen,&ulNextFrameTime) )
          {
            nBytesSkipped++;
            nBytesRead--;
            continue;
          }
        }
        else
        {
          uint64 ullOffset = m_nCurrOffset;
          memcpy(dataBuffer, m_dataBuffer+nBytesSkipped, nBytesRead);
          ullOffset += (nBytesSkipped + nBytesRead);
          nBytesRead += MP3CallbakGetData (ullOffset, nMaxBufSize - nBytesSkipped,
                                           m_dataBuffer, nMaxBufSize,
                                           m_pUserData, bEndOfData);
          nBytesSkipped = 0;
          if (IAUDIO_SUCCESS !=
              parse_frame_header(dataBuffer + nframeLen,
                                 &ulNextFrameLen,&ulNextFrameTime) )
          {
            nBytesSkipped++;
            nBytesRead--;
            continue;
          }
        }
      }

      /* Check whether enough data is available in the buffer or not, before updating nBytesRead param*/
      if((nBytesRead - nFrameBufLen) < nframeLen)
      {
        /* If single frame is also not found, then we will return data underrun in case of streaming and
           end of file in case of local playback. We will not provide incomplete frames to the upper layer*/
        if(false == frameStartFound)
        {
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH, "GetCurrentSample: Complete frame is not available in the o/p buf!!");
          *nBytesNeeded  = 0;
          if(true == bEndOfData)
            return PARSER_ErrorEndOfFile;
          else
            return PARSER_ErrorDataUnderRun;
        }
        break;
      }

      /* Set flag to indicate that, at least one frame was found */
      frameStartFound = true;
      nFrameBufLen += nframeLen;
#ifdef HLS_PRIVATE_TIMESTAMP
      /* If we got ID3 tags in the middle of the file, we will provide only one frame as output, else
         we will provide integer number of frames data in the buffer */
      if(true == m_id3tagfound)
      {
        m_firstFrame = true;

        /* Update class variable with first frame's header info */
        parse_mp3_frame_header(dataBuffer + nBytesSkipped, header_info);
        memcpy(&m_header_info, &header_info, sizeof(header_info));

        /* Reset id3 tag flag */
        m_id3tagfound = false;
        uint32 ulTotalEntries = m_metadata->getTotalID3V2Entries();
        for (uint32 ulIndex = 0; ulIndex < ulTotalEntries; ulIndex++)
        {
          metadata_id3v2_type *pid3V2 = m_metadata->get_id3v2(ulIndex);
          /* If it is first frame and timestamp is valid then update
             the class param accordingly */
          if((pid3V2) && pid3V2->private_tag.isTsValid)
          {
            m_firstFrameTS = pid3V2->private_tag.timeStamp;
            m_nCurrentTime = m_nNextFrameTime = m_firstFrameTS;
            m_isFirstTSValid = pid3V2->private_tag.isTsValid;
            break;
          }
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
          //! If Fractional time crosses 1000, then it means 1ms worth is reached.
          //! Update output sample time and reset the fractional time unit
          if (m_FrameTimeinUS >= 1000)
          {
            m_nCurrentTime++;
            m_nNextFrameTime++;
            m_FrameTimeinUS -= 1000;
          }
          m_nNextFrameTime += nframeTime;
        }
        //! Update fractional part in class variable
        m_FrameTimeinUS += m_nFracResidualTs;
        break;
      }
      /* If seek is done, then also we will provide only one complete frame, in that case also
         we have timestamp to provide along with frame data.*/
      if(true == m_SeekDone)
      {
        m_SeekDone = false;
        break;
      }
    }
#ifdef HLS_PRIVATE_TIMESTAMP
     //! Check whether "ID3" string is available or not
    if(false == frameStartFound && !memcmp("ID3", m_dataBuffer+nFrameBufLen, 3))
    {
      uint64 prevStartOffset = m_audio_track.start;
      m_audio_track.start = m_nCurrOffset;
      PARSER_ERRORTYPE status = parse_id3(m_MP3FilePtr, m_nFileSize);
      if(PARSER_ErrorNone != status)
      {
        m_audio_track.start = prevStartOffset;
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH, "GetCurrentSample: ID3 Parsing is failed");
        return status;
      }

      m_nCurrOffset = m_audio_track.start;

      /* Retain the start value. When seek is called, Parser will do seek based on first
         frame that is available. */
      m_audio_track.start = prevStartOffset;
      nBytesRead = 0;
      nBytesSkipped = 0;
      nFrameBufLen = 0;
#ifdef FEATURE_FILESOURCE_MP3_PARSER_DEBUG
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH, "GetCurrentSample ID3 parsing is completed!!");
#endif
      m_id3tagfound = true;
    }
#endif
    if(false == frameStartFound && nBytesRead)
    {
      nBytesSkipped++;
      nBytesRead--;
    }
  }

  /* If any data is skipped at the start of the buffer, we will readjust the data in the
     output Buffer such that it starts with frame sync word. Offset parameter is also
     updated accordingly. */
  if(nBytesSkipped)
  {
    memcpy(dataBuffer, m_dataBuffer+nBytesSkipped, nFrameBufLen);
    m_nCurrOffset+= nBytesSkipped;
    m_dataBuffer = dataBuffer;
  }
  //! Update current sample for CBR clips after seek operation once
  //! For given time, Offset calculation is not accurate.
  //! After seek operation, update the current playback time.
  //! This will ensure Parser not to give different timestamps for sample at
  //! same offset.
  if ((false == bUpdateTime) && (m_seek_function_defined) && (!m_is_vbr))
  {
    m_nCurrentTime   = ((m_nCurrOffset - m_audio_track.start) * 8) /
                       (m_header_info.bitrate/1000);
    m_nNextFrameTime = m_nCurrentTime;
  }

  /* Update output buffer size param with actual frame size value */
  *nBytesNeeded = nFrameBufLen;
  /* Update cur offset value to the start of next frame */
  m_nCurrOffset += uint64(nFrameBufLen);
#ifdef HLS_PRIVATE_TIMESTAMP
  /* Skipping TAG header info which is of 128 bytes. */
  if( (nBytesRead - nFrameBufLen) >= 3 &&
      !memcmp("TAG", m_dataBuffer+nFrameBufLen, 3))
  {
    nFrameBufLen += 128;
  }

  /* If the current offset starts with ID3 tag.*/
  if((nBytesRead - nFrameBufLen) >= 3 &&
     !memcmp("ID3", m_dataBuffer+nFrameBufLen, 3))
  {
      uint64 prevStartOffset = m_audio_track.start;
      m_audio_track.start = m_nCurrOffset;
      PARSER_ERRORTYPE status = parse_id3(m_MP3FilePtr, m_nFileSize);
      if(PARSER_ErrorNone == status)
      {
        m_nCurrOffset = m_audio_track.start;
        m_id3tagfound = true;
      }

      /* If clips are playing continuously then seek will not be supported. In such cases, Parser
         instance has to be closed and re-initialize with the clip in which seek is required.
         If only one clip is playing which has multiple ID3 tags in between then by resetting the
         start value seek will not be affected. */
      m_audio_track.start = prevStartOffset;
#ifdef FEATURE_FILESOURCE_MP3_PARSER_DEBUG
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH, "GetCurrentSample ID3 parsing is completed!!");
#endif
  }
#endif
  m_CurrentParserState = PARSER_READY;

  /* If data is read into output buffer, then return success else return underrun*/
  if(0 != *nBytesNeeded)
  {
    /* Set EOF flag for last sample, only for non-LA targets. */
#ifndef _ANDROID_
    if (m_nCurrOffset >= m_nFileSize)
    {
      return PARSER_ErrorEndOfFile;
    }
#endif
    return PARSER_ErrorNone;
  }
  else if(true == bEndOfData)
    return PARSER_ErrorEndOfFile;
  else
    return PARSER_ErrorDataUnderRun;
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
//  amrErrorType
//
// SIDE EFFECTS
//  None
//=============================================================================
PARSER_ERRORTYPE mp3Parser::Seek(uint64 nReposTime, uint64 *nSeekedTime)
{
  PARSER_ERRORTYPE status = PARSER_ErrorNone;
  bool isDataAvailable = false;
  uint64 prevFilePosition = m_nCurrOffset;
  uint64 time = nReposTime;
  uint64 file_position;

  if(nReposTime == 0)
  {
    set_newfile_position(m_audio_track.start);
    m_nCurrentTime = nReposTime;
    m_nNextFrameTime = m_nCurrentTime;
    return PARSER_ErrorNone;
  }

  /* Check whether seek Requsted time is less than the first frame's TS */
  if(m_firstFrameTS - m_baseTS > nReposTime)
    time = 0;
  /* If firstFrameTS is non-zero value, update seektime requested accordingly */
  else if(nReposTime >= m_firstFrameTS - m_baseTS)
    time = nReposTime - (m_firstFrameTS - m_baseTS);

  if(m_seek_function_defined)
  {
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
    if(NULL != m_pseek)
    {
      time = m_pseek->process_seek(time);
      m_nCurrentTime = time;
      m_nNextFrameTime = m_nCurrentTime;
    }
    else
    {
      status = get_accurate_seek_position(time, &file_position, nSeekedTime);
      if(PARSER_ErrorNone == status)
      {
        /* In the above function, we will parse frame by frame to seek to the
           requested Timestamp. So, we have already parsed the data that will be
           returned as part of getNextMediaSample after seek is returned as successful*/
        isDataAvailable = true;
        set_newfile_position(file_position);

        /* Update output seek time according to the first frame's timestamp */
        *nSeekedTime += (m_firstFrameTS - m_baseTS);

        /* If seek is successful, then we will not use id3 tags that are found at the
           start of the playback. By marking id3tagfound flag as false, we will ensure
           that id3 tags are not used when seek is called */
        m_id3tagfound = false;
        m_SeekDone    = true;
        m_nCurrentTime = *nSeekedTime;
        m_nNextFrameTime = m_nCurrentTime;
      }
    }
  }

  if(false == isDataAvailable && PARSER_ErrorNone == status)
  {
    bool isID3TagAvailable  = false;
    uint32 loopCount    = 1;
    uint8 *nDataBuf     = 0;
    uint32 nDataBufSize = m_mp3_audio_info.dwSuggestedBufferSize;
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
      /* If bufferSize is not sufficient to allocate read one complete frame
         free the already allocated memory and allocate double the size of
          already allocated memory. */
      if(nDataBuf)
      {
        MM_Free(nDataBuf);
        nDataBuf = 0;
        nDataBufSize *= m_mp3_audio_info.dwSuggestedBufferSize * loopCount;
      }
      nDataBuf = (uint8 *)MM_Malloc(nDataBufSize);
      if(!nDataBuf)
      {
        return PARSER_ErrorMemAllocFail;
      }
      status = GetCurrentSample(nDataBuf, nDataBufSize, &nBytesNeeded, false);
      loopCount++;
    } while(PARSER_ErrorInsufficientBufSize == status && loopCount < 10);
    if(nDataBuf)
    {
      MM_Free(nDataBuf);
      nDataBuf = 0;
    }
    /* Reset id3tag flag */
    m_id3tagfound = isID3TagAvailable;
    if(PARSER_ErrorNone == status)
    {
      /* Reset file position to the calculated offset*/
      set_newfile_position(file_position);
      *nSeekedTime = time + (m_firstFrameTS - m_baseTS);

      /* If seek is successful, then we will not use id3 tags that are found at the
         start of the playback. By marking id3tagfound flag as false, we will ensure
         that id3 tags are not used when seek is called */
      m_id3tagfound = false;

      /* Mark SeekDone flag as true, so that parser will return only one complete frame
         as output through getNextMediaSample API */
      m_SeekDone = true;
    }
    else
    {
      /* Revert the Current Offset position to the value, before seek is called*/
      m_nCurrOffset = prevFilePosition;
      m_SeekDone = false;
    }
  }

  return status;
}
//=============================================================================
// FUNCTION : parse_file_header
//
// DESCRIPTION
//  Parses the file looking for file headers and other non-audio metadata.
//  Determines the start and stop file position of audio data. Does not parse
//  the audio data, just locates it.
//
// PARAMETERS
//  file : IFilePort interface to data to be parsed.
//
// RETURN VALUE
//  AEE_SUCCESS : success
//  AEE_EBADPARM : Invalid input parameters
//  AEE_EBADSTATE : Invalid format parser state
//  AEE_ENOMEMORY : No memory
//  AEE_EFAILED : unexpected failure
//  AEE_EINVALIDFORMAT : Unsupported MP3 format
//
// SIDE EFFECTS
//  None
//
// NOTE
//  Supports only MP3 audio format
//=============================================================================
PARSER_ERRORTYPE mp3Parser::parse_file_header ()
{
   PARSER_ERRORTYPE status = PARSER_ErrorNone;

   if( (NULL == m_MP3FilePtr) || (m_parse_file_header_done))
   {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"parse_file_header invliad parameters..");
      return PARSER_ErrorDefault;
   }
   // Create metadata object, if not already created
   if(!m_metadata)
   {
     m_metadata = MM_New_Args(mp3metadata,());
     if (NULL == m_metadata)
     {
       MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"parse_file_header failed to create m_metadata");
       return PARSER_ErrorMemAllocFail;
     }
     m_id3tagparsed = false;
   }
   // Do file header pre processing, if ID3 tag parsing is not yet completed due to data underrun
   if(false == m_id3tagparsed)
   {
    status = file_header_preprocessing(m_MP3FilePtr);
   }
   if(PARSER_ErrorDataUnderRun == status)
   {
     MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"Data underrun in file_header_preprocessing");
     return status;
   }
   else if (PARSER_ErrorNone != status)
   {
     MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"parse_file_header failed in file_header_preprocessing");
     MM_Delete( m_metadata);
     m_metadata = NULL;
     return status;
   }
   else
   {
     uint32 ulTotalEntries = m_metadata->getTotalID3V2Entries();
     uint32 ulIndex        = 0;
     do
     {
       metadata_id3v2_type *pid3V2 = m_metadata->get_id3v2(ulIndex);
       /* Update timestamp variables in object with the timestamps, that
          are parsed from id3 tags*/
       if ((pid3V2) &&(pid3V2->private_tag.isTsValid))
       {
         m_firstFrameTS = pid3V2->private_tag.timeStamp;
         m_isFirstTSValid = pid3V2->private_tag.isTsValid;
         /* If firstframe TS is valid, then set that as base Timestamp*/
         if(m_isFirstTSValid)
           m_baseTS = m_firstFrameTS;
         break;
       }
     } while(ulIndex++ < ulTotalEntries);
   }
   // Get start position for file header
   uint64 offset = m_audio_track.start;
   uint32 bytes_read = 0;
   bool bEndOfData = false;
   uint8 *buffer = NULL;
   buffer = (uint8 *) MM_Malloc(MAX_READ_LENGTH*sizeof(uint8));
   if (NULL == buffer)
   {
     MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,"parse_file_header buffer allocation failed..");
     MM_Delete( m_metadata);
     m_metadata = NULL;
     return PARSER_ErrorMemAllocFail;
   }

   // Read data from file to parse the header
   bytes_read = MP3CallbakGetData(offset, MAX_READ_LENGTH, buffer,
                                  MAX_READ_LENGTH,m_pUserData, bEndOfData);
   if(bytes_read < MP3_FILE_HEADER_SIZE)
   {
     MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,"parse_file_header FileRead failed..");
     MM_Free(buffer);
     buffer = NULL;
     if(bEndOfData == true)
     {
       MM_Delete( m_metadata);
       m_metadata = NULL;
       return PARSER_ErrorReadFail;
     }
     else
       return PARSER_ErrorDataUnderRun;
   }

   // File position of first frame sync word
   uint64 first_sync_offset = 0;
   int count = 0;
   uint64 read_offset = 0;
   uint32 bytes_consumed = 0;
   // Parse first few frames to check if file is a valid mp3 file and
   // the mp3 format of file is supported by this format parser
   while ( (bytes_read > (m_header_size -1)) && (MP3_PARSE_FRAME_COUNT > count) )
   {
      // Find mp3 frame sync in file
      if (find_mp3_frame_sync(buffer+read_offset, bytes_read,m_header_info))
      {
        // Get Current frame length
        bytes_consumed = calc_frame_length(m_header_info);
        if (!count)
        {
          //! Update start offset value to first frame start
          m_audio_track.start += read_offset;
          // Store the start offset of first MP3 frame
          first_sync_offset = offset + read_offset;
          // Check if vbr header is present
          if (PARSER_ErrorUnsupported == mp3vbrheader::parse_mp3_vbr_header( m_MP3FilePtr, first_sync_offset, m_header_info,&m_vbr_header))
          {
            m_is_vbr = false;
          }
          else
          {
            m_header_info.bitrate = 0;
            m_is_vbr = true;
          }
          /* If LAME tag is present, then first frame contains metadata info
             only. This is not needed to propagate to decoder. This will reduce
             the initial silence especially for gap-less playback feature. */
          if (m_bLAMETagAvailable)
          {
            m_audio_track.start += bytes_consumed;
            first_sync_offset   += bytes_consumed;
          }
        }
        if(m_is_vbr)
         {
           //We can't figure out frame boundary in case of vbr
           count = MP3_PARSE_FRAME_COUNT;
           MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"parse_file_header:Clip has variable bit-rate,parsing only the 1st frame");
           continue;
         }
         if (!bytes_consumed)
         {
           bytes_consumed = mp3Parser::m_header_size;
         }
         //! Update frame counter
         count++;
         read_offset += bytes_consumed;
         /* If sufficient data is available, then continue.
            If not, try to read more data. */
         if (bytes_read >= (bytes_consumed + mp3Parser::m_header_size))
         {
            bytes_read -= bytes_consumed;
         }
         else
         {
           // Sufficient data not available for checking more valid frames
           // Read data from file to parse the header
           offset += read_offset;
           read_offset = 0;
           bytes_read = MP3CallbakGetData(offset, MAX_READ_LENGTH, buffer,
                                          MAX_READ_LENGTH, m_pUserData, bEndOfData);
           if(!bytes_read)
           {
             MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"num of bytes read from file/buf is ZERO..");
             count = MP3_PARSE_FRAME_COUNT;
             bytes_read = 0;
             break;
           }
         }
      }
      else
      {
         read_offset++;
         bytes_read--;
      }
   }//while ( (bytes_read > (m_header_size -1)) && (MP3_PARSE_FRAME_COUNT > count) )
   MM_Free(buffer);
   buffer = NULL;
   if(count)
   {
     count = MP3_PARSE_FRAME_COUNT;
   }
   // Could not find required number of valid MP3 frames in file
   if (MP3_PARSE_FRAME_COUNT > count)
   {
#ifdef FEATURE_FILESOURCE_MP3_PARSER_DEBUG
     MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"parse_file_header could not find valid MP3 frames in file...");
#endif
     MM_Delete( m_metadata);
     m_metadata = NULL;
     return PARSER_ErrorInHeaderParsing;
   }
   // Check if all required technical metadata is avialable
   if (0 == m_header_info.samplerate || (0 == m_header_info.bitrate && !m_is_vbr) )
   {
#ifdef FEATURE_FILESOURCE_MP3_PARSER_DEBUG
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"parse_file_header Failed to parse samplerate/bit-rate");
#endif
      MM_Delete( m_metadata);
      m_metadata = NULL;
      return PARSER_ErrorInHeaderParsing;
   }
   // Update technical metadata extracted from file and frame headers
   memcpy (&(m_metadata->m_techmetadata), &m_header_info, sizeof (m_metadata->m_techmetadata));

   // Do file header postprocessing
   status = file_header_postprocessing(m_MP3FilePtr, first_sync_offset);
   if (PARSER_ErrorNone != status)
   {
#ifdef FEATURE_FILESOURCE_MP3_PARSER_DEBUG
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,"parse_file_header Failed in file_header_postprocessing");
#endif
      MM_Delete( m_metadata);
      m_metadata = NULL;
      return status;
   }
   // Update the flag that indicates that the format parser
   // has successfully parsed the file
   m_parse_file_header_done = true;
   return status;
}
//=============================================================================
// FUNCTION : file_header_preprocessing
//
// DESCRIPTION
//  Parses the Tags present before the actual file header and returns
//  start position of actual file header
//
// PARAMETERS
//  file : IFilePort1 interface to input stream
//
// RETURN VALUE
//  AEE_SUCCESS : success
//  AEE_EFAILED : general failure
//
// SIDE EFFECTS
//  This function call parses ID3V2 tags at the begining of the file and
//  updates m_metadata
//=============================================================================
//
PARSER_ERRORTYPE mp3Parser::file_header_preprocessing (OSCL_FILE *m_MP3FilePtr)
{
   // Set the deafult audio track params for this file
   m_audio_track.start = 0;
   m_audio_track.end = m_nFileSize;
   m_audio_track.size = m_nFileSize;

   // Parse the ID3 tags
   PARSER_ERRORTYPE status = parse_id3(m_MP3FilePtr, m_nFileSize);
   //If there is no id3, check for LAME tag
   if (PARSER_ErrorDataUnderRun != status)
   {
     //If ID3 is not present, check if the clip is LAME encoded
     status = parse_LAMEtag();
   }
   /* If ID3 tag parsing is failed because of data underrun, then we will parse one more time.
      Else, we will not parse ID3 tag parsing one more time.*/
   if(PARSER_ErrorDataUnderRun != status)
    m_id3tagparsed = true;
   else
     m_id3tagparsed = false;

   //We should always return SUCCESS from header preprocessing
   //since absence of id3/LAME is valid scenario.
   return status;
}
//=============================================================================
// FUNCTION : parse_id3
//
// DESCRIPTION
//  Parses the ID3 V1 and V2 tags present at the start and end of the file
//
// PARAMETERS
//  file : IFilePort1 interface to input stream
//  length : Total length of data
//
// RETURN VALUE
//  AEE_SUCCESS : success
//  AEE_EFAILED : failure
//
// SIDE EFFECTS
//  Updates m_audio_track information and m_metadata object
//=============================================================================
//
PARSER_ERRORTYPE mp3Parser::parse_id3(OSCL_FILE *m_MP3FilePtr, uint64 length)
{
   PARSER_ERRORTYPE result = PARSER_ErrorNone;
   // Stores the intermediate results
   PARSER_ERRORTYPE status = PARSER_ErrorDefault;
   bool bID3v2postend = FALSE;
   while(m_MP3FilePtr)
   {
     //if we encounter id3v2 at the beginning of the file,
     //we need to keep parsing as there are few clips which have
     //multiple id3v2 tag at the beginning.
     bool bGotId3AtBeg = false;

     /*This flag set to false means, ID3 tags can be searched from end of file.
       But in case of streaming, Parser need not to look for ID3 tags at end.
       Update flag with HTTP stream flag properties. */
     if (FALSE == bID3v2postend)
     {
       bID3v2postend = m_bHTTPStreaming;
     }
     // Check for ID3v2 tag
     if(ID3v2::check_ID3v2_present(m_MP3FilePtr, length, m_audio_track.start,
                                   &bID3v2postend))
     {
#ifdef FEATURE_FILESOURCE_MP3_PARSER_DEBUG
       MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"ID3v2 detected..");
#endif
       // Get an object pointer of the ID3v1 parser class
       ID3v2 *mp3_ID3v2 = MM_New_Args(ID3v2,( status));
       metadata_id3v2_type id3v2;
       memset (&id3v2, 0, sizeof(id3v2));
       // Parse the ID3 tag and save the contents
       status = mp3_ID3v2->parse_ID3v2_tag(m_MP3FilePtr, m_audio_track.start,
                                           &id3v2, bID3v2postend);
       if(PARSER_ErrorReadFail == status && m_nFileSize == MAX_FILE_SIZE)
       {
           result = PARSER_ErrorDataUnderRun;
       }
       else if( (PARSER_ErrorNone == status) ||(status == PARSER_ErrorUnsupported))
       {
         m_id3tagfound = true;
         //When id3v2 is not supported, id3 lib does not
         //have capability to parse individula frames but size field is still valid
         uint64 Id3v2size = 0;
         // Parse the ID3 tag and save the contents
         (void) mp3_ID3v2->get_ID3v2_size(m_MP3FilePtr, length,
                                          m_audio_track.start, bID3v2postend,
                                          &Id3v2size);
         //Store the ID3v2 info structure with aacmetadata
         if(m_metadata)
         {
           status = m_metadata->set_id3v2(&id3v2);
         }

         if (PARSER_ErrorNone == status)
         {
           result = PARSER_ErrorNone;
           // Update the audio track information
           if(bID3v2postend)
           {
             m_audio_track.end -= Id3v2size;
           }
           else
           {
             //id3v2 detected at the begining, set bGotId3AtBeg to true
             //to parse ahead to make sure there are no subsequent id3v2
             m_audio_track.start += Id3v2size;
             bGotId3AtBeg = true;
           }
           m_audio_track.size -= Id3v2size;
         }
       }
       MM_Delete( mp3_ID3v2);
     }
     // Check for ID3v1 tag only in case of local playback
     if( result != PARSER_ErrorDataUnderRun && m_nFileSize != MAX_FILE_SIZE &&
         (false == m_bHTTPStreaming) &&
         (ID3v1::check_ID3v1_present(m_MP3FilePtr, length) ) &&
         ((m_metadata) &&(!m_metadata->m_id3v1)) )
     {
#ifdef FEATURE_FILESOURCE_MP3_PARSER_DEBUG
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"ID3v1 detected..");
#endif
        // Get an object pointer of the ID3v1 parser class
        ID3v1 *mp3_ID3v1 = MM_New_Args(ID3v1,(status));
        metadata_id3v1_type id3v1;
        memset (&id3v1, 0, sizeof(id3v1));
        // Parse the ID3 tag and save the contents
        status = mp3_ID3v1->parse_ID3v1_tag(m_MP3FilePtr, &id3v1, length);
        if (PARSER_ErrorNone == status)
        {
           //Store the ID3v1 info structure with aacmetadata
           status = m_metadata->set_id3v1(&id3v1);
           if (PARSER_ErrorNone == status)
           {
             result = PARSER_ErrorNone;
             // Update the audio track information
             m_audio_track.end -= ID3v1_SIZE;
             m_audio_track.size -= ID3v1_SIZE;
           }
        }
        MM_Delete( mp3_ID3v1);
     }
     if(!bGotId3AtBeg)
     {
       status= PARSER_ErrorNone;
#ifdef FEATURE_FILESOURCE_MP3_PARSER_DEBUG
       MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"breaking out of parse_id3 loop...");
#endif
       break;
     }
   }//while(1)
   return result;
}

//=============================================================================
// FUNCTION : parse_LAMEtag
//
// DESCRIPTION
//  Parses the LAME tage available in the file
//
// PARAMETERS
//  file : IFilePort1 interface to input stream
//  length : Total length of data
//
// RETURN VALUE
//  AEE_SUCCESS : success
//  AEE_EFAILED : failure
//
// SIDE EFFECTS
//  Updates m_audio_track information and m_metadata object
//=============================================================================
//
PARSER_ERRORTYPE mp3Parser::parse_LAMEtag(void)
{
  PARSER_ERRORTYPE eRetStatus = PARSER_ErrorNone;
  uint32 nNumBytesRequest     = MP3_FILE_HEADER_SIZE;
  uint32 nBytesRead           = 0;
  uint32 nMaxBufSize          = MP3_FILE_HEADER_SIZE * 8;
  uint8* pDataBuf             = (uint8 *)MM_Malloc(MP3_FILE_HEADER_SIZE * 8);
  bool bEndOfData             = false ;

  //Location of Xing and Info tags in LAME encoded clips are at 36
  //for STEREO/JSTEREO and 21 for MONO but since we do not know the channel type yet
  //we will try both
  uint64 ullOffset        = m_audio_track.start + MP3_MONO_XING_TAG_POS;
  uint64 ullLameTagOffset = m_audio_track.start + LAME_TAG_POS_FOR_MONO;
  uint32 ulCounter = 0;
  if ( pDataBuf )
  {
     while(ulCounter < 2)
     {
       ulCounter++;
       //Read 4 bytes to check if Lame tag is present
       nBytesRead = MP3CallbakGetData (ullOffset, nNumBytesRequest, pDataBuf,
                                       nMaxBufSize, m_pUserData, bEndOfData);

       if(nBytesRead)
       {
         //Lame encoded VBR clips have "Xing" tag, CBR clips will have "Info"
         if((!memcmp(pDataBuf,"Xing",MP3_FILE_HEADER_SIZE)) ||
            (!memcmp(pDataBuf,"Info",MP3_FILE_HEADER_SIZE)) )
         {
           m_bLAMETagAvailable = true;
           break;
         }
         //Not a MONO clip, let's try if it's STEREO/JSTEREO
         ullOffset = m_audio_track.start + MP3_STEREO_XING_TAG_POS;
         ullLameTagOffset = m_audio_track.start + LAME_TAG_POS_FOR_STEREO;
       }
       else if(bEndOfData == false)
       {
         m_CurrentParserState = PARSER_UNDERRUN;
         eRetStatus = PARSER_ErrorDataUnderRun;
       }
       else
       {
         m_CurrentParserState = PARSER_READ_FAILED;
         eRetStatus = PARSER_ErrorReadFail;
       }
     }/* while loop*/

     /* Encoder Delay will be stored in 3bytes.
        [xxxxxxxx] [xxxxyyyy] [yyyyyyyy]  First 12 bits are used to store
        Encoder Delay and last 12bits are used for Padding delay.*/
     if (true == m_bLAMETagAvailable)
     {
       uint32 ulDataRead  = MP3CallbakGetData (ullLameTagOffset, nMaxBufSize,
                                               pDataBuf, nMaxBufSize,
                                               m_pUserData, bEndOfData);
       if ((ulDataRead) &&
           (!memcmp(pDataBuf, "LAME", MP3_FILE_HEADER_SIZE)))
       {
         uint32 ulEncOffset = MP3_ENCODER_DELAY_OFFSET;
         m_ulEncoderDelay  = pDataBuf[ulEncOffset++] << 4;
         m_ulEncoderDelay |= (pDataBuf[ulEncOffset] >> 4);
         m_ulPaddingDelay  = (pDataBuf[ulEncOffset++] & 0xF) << 8;
         m_ulPaddingDelay |= pDataBuf[ulEncOffset];
       }
     }
     MM_Free(pDataBuf);
  }/*if pDataBuf*/

  return eRetStatus;
}
//=============================================================================
// FUNCTION : find_mp3_frame_sync
//
// DESCRIPTION
//  This function calculates the byte offset of the sync bits within an MP3
//  file given the MP3 buffer and the number of bytes contained within buffer.
//
// PARAMETERS
//  buffer: buffer containing MP3 data
//  buf_len : length of input data.
//            Length should be atleast equal to size of mp3 header
//  header_info (out) : header info extracted if sync is found
//  sync_offset (out) : offset of sync found within given buffer
//
// RETURN VALUE
//  true : sync word found
//  false : failed to find sync word
//
// SIDE EFFECTS
//  None
//=============================================================================
//
bool mp3Parser::find_mp3_frame_sync (const uint8 *buffer, uint32 buf_len,
                                     tech_data_mp3 &header_info) const
{
  bool bret = false;
  if( (buffer) && (buf_len >= m_header_size)  )
  {
    // If sync is found, check if it has valid MP3 header
    if (PARSER_ErrorNone == parse_mp3_frame_header(buffer, header_info))
    {
      //sync found
      bret = true;
    }
  }
  return bret;
}
//=============================================================================
// FUNCTION is_mp3_sync
//
// DESCRIPTION
//  This function parses the given data to check if it is MP3 frame sync word
//
// PARAMETERS
//  frame : MP3 data
//
// RETURN VALUE
//  true : given data is MP3 frame sync word
//  false : given data is not MP3 frame sync word
//
// SIDE EFFECTS
//  None.
//=============================================================================
//
bool mp3Parser::is_mp3_sync(const uint8* frame) const
{
  bool bret = false;
  // Check for sync word
  if (0xFF == frame[0] && 0xE0 == (frame[1] & 0xE0))
  {
    bret = true;
  }
  return bret;
}

//=============================================================================
// FUNCTION calc_frame_length
//
// DESCRIPTION
//  This function calculates the length of an MP3 frame based on information
//  stored in the given MP3 header.
//
// PARAMETERS
//  header_info: MP3 header info
//
// RETURN VALUE
//  The frame length in bytes
//
// SIDE EFFECTS
//  None.
//=============================================================================
//
uint32 mp3Parser::calc_frame_length (const struct tech_data_mp3 &header_info) const
{
   uint32 frame_bytes = 0;
   // Validate header info
   if (header_info.samplerate && header_info.bitrate)
   {
     if( (header_info.layer == MP3_LAYER_2)|| (header_info.layer == MP3_LAYER_3))
     {
       // Using MP3 Coefficient & MP3 Slot size to calculate proper frame bytes
       // based on different MP3 layer types.
       // frame_bytes = ((MP3_Coeff * Bit-Rate)/Sample-Rate )* SlotSize
       frame_bytes =  (MP3_COEFFICIENTS[(m_header_info.version == MP3_VER_1 ? 0 : 1)][m_header_info.layer] * header_info.bitrate
                        / header_info.samplerate)* MP3_SLOT_SIZES[m_header_info.layer];
       if(header_info.is_padding)
       {
         frame_bytes++;
       }
     }
     else if(header_info.layer == MP3_LAYER_1)
     {
       frame_bytes = (12 * header_info.bitrate / header_info.samplerate + header_info.is_padding) * 4;
     }
   }
#ifdef FEATURE_FILESOURCE_MP3_PARSER_DEBUG
   MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,"calc_frame_length frame_bytes %lu",
                frame_bytes);
#endif
   return frame_bytes;
}
//=============================================================================
// FUNCTION : file_header_postprocessing
//
// DESCRIPTION
//  This function does post processing after file header is successfully parsed
//  It does following tasks -
//  1. Parser tags at the end of the file
//  2. Calculate Audio track information
//  3. Calculate Total playback duration
//  4. Calculate buffer requirements
//
// PARAMETERS
//  file : IFilePort1 interface to input stream
//  first_sync_offset : Start offset of first MP3 audio frame
//
// RETURN VALUE
//  AEE_SUCCESS : success
//  AEE_EFAILED : general failure
//
// SIDE EFFECTS
//  Parses the ID3 tags present at the end of the file and updates m_metadata
//=============================================================================
PARSER_ERRORTYPE mp3Parser::file_header_postprocessing (OSCL_FILE* /*m_MP3FilePtr*/,
                                                        uint64 first_sync_offset)
{
   PARSER_ERRORTYPE result = PARSER_ErrorNone;

   // Calculate audio track information
   (void)update_audio_track(first_sync_offset, m_audio_track);
   // Calculate total playback duration
  result = calc_playback_duration (m_duration);

  if (result == PARSER_ErrorNone)
  {
    // Determine if seek/get duration is supported
    if (0 == m_duration)
    {
      m_seek_function_defined = false;
    }
    else
    {
      if (!m_is_vbr)
      {
        // CBR file
        m_seek_function_defined = true;
      }
      else if (m_vbr_header)
      {
        // VBR file check if sufficient information is available in vbr header
        // to support get_duration and get_seek_position APIs
        (void)m_vbr_header->get_seek_function_defined(m_seek_function_defined);
      }
      else
      {
        // VBR header not avilable for VBR file
        m_seek_function_defined = false;
      }
    }
  }
  return result;
}
//=============================================================================
// FUNCTION : update_audio_track
//
// DESCRIPTION
//  This function updates the audio track information after parsing the file header
//
// PARAMETERS
//  first_sync_offset : Start offset of first MP3 audio frame
//  audio_track_info (out) : Audio track information
//
// RETURN VALUE
//  AEE_SUCCESS : success
//  AEE_EFAILED : general failure
//
// SIDE EFFECTS
//  None
//=============================================================================
//
PARSER_ERRORTYPE mp3Parser::update_audio_track (uint64 first_sync_offset,
                                                AudioTrack &audio_track_info) const
{
   PARSER_ERRORTYPE status = PARSER_ErrorDefault;

   // update the audio track start position
   audio_track_info.start = first_sync_offset;

   // update the audio track end position and length if required
   if ( m_vbr_header )
   {
     // check if track length is available in vbr header or info header
      uint32 bytes = 0;
      status = m_vbr_header->get_total_bytes(bytes);
      if (PARSER_ErrorNone == status)
      {
         audio_track_info.end = audio_track_info.start + bytes;
         audio_track_info.size = audio_track_info.end - audio_track_info.start;
      }
   }
   return status;
}
//=============================================================================
// FUNCTION parse_mp3_frame_header
//
// DESCRIPTION
//  This function parses the given MP3 frame and populates the given MP3
//  header info structure
//
// PARAMETERS
//  frame : input mp3 frame
//  hdr_info (out) : MP3 header info . Filled after parsing the frame
//
// RETURN VALUE
//  AEE_SUCCESS : Successfully parsed the frame
//  AEE_EFAILED : General failure
//
// SIDE EFFECTS
//  None.
//=============================================================================
//
PARSER_ERRORTYPE mp3Parser::parse_mp3_frame_header (const uint8* frame,
                                                    tech_data_mp3 &hdr_info) const
{
  PARSER_ERRORTYPE error = PARSER_ErrorNone;
  uint8 ucBitrateIndex   = 0;
  uint8 ucSampleIndex    = 0;

  // Check for sync word
  if (!(0xFF == frame[0] && 0xE0 == (frame[1] & 0xE0)))
  {
    error = PARSER_ErrorInHeaderParsing;
  }
  else
  {
    // extract the MP3 header information from input data
    hdr_info.version = (mp3_ver_enum_type)
      ((frame[MP3HDR_VERSION_OFS] & MP3HDR_VERSION_M) >> MP3HDR_VERSION_SHIFT);

    // Check if MPEG version of input data is supported
    if (!(MP3_VER_25 == hdr_info.version ||
          MP3_VER_2 == hdr_info.version ||
          MP3_VER_1 == hdr_info.version))
    {
#ifdef FEATURE_FILESOURCE_MP3_PARSER_DEBUG
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
        "hdr_info.version value is = %d", hdr_info.version);
#endif
      error = PARSER_ErrorInHeaderParsing;
    }
    hdr_info.layer = (mp3_layer_enum_type)
      ((frame[MP3HDR_LAYER_OFS] & MP3HDR_LAYER_M) >> MP3HDR_LAYER_SHIFT);

    // Check if MPEG layer of input data  is supported
    if ( !( ( MP3_LAYER_3 == hdr_info.layer )||
            ( MP3_LAYER_2 == hdr_info.layer )||
            ( MP3_LAYER_1 == hdr_info.layer ) ) )
    {
#ifdef FEATURE_FILESOURCE_MP3_PARSER_DEBUG
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
        "hdr_info.layer value is = %d", hdr_info.layer);
#endif
      error = PARSER_ErrorInHeaderParsing;
    }
    hdr_info.crc_present = (boolean)
      ((frame[MP3HDR_CRC_OFS] & MP3HDR_CRC_M) >> MP3HDR_CRC_SHIFT);

    ucBitrateIndex = ((frame[MP3HDR_BITRATE_OFS] &
                            MP3HDR_BITRATE_M) >> MP3HDR_BITRATE_SHIFT);

    /* If bitrate index value is ZERO, then bitrate calcualted is ZERO */
    if (MP3_MAX_BITRATE_INDEX <= ucBitrateIndex || 0 == ucBitrateIndex)
    {
#ifdef FEATURE_FILESOURCE_MP3_PARSER_DEBUG
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
        "bitrate_index value is = %d", ucBitrateIndex);
#endif
      error = PARSER_ErrorInHeaderParsing;
    }

    ucSampleIndex = (uint8)((frame[MP3HDR_SAMPLERATE_OFS] &
                             MP3HDR_SAMPLERATE_M) >> MP3HDR_SAMPLERATE_SHIFT);

    if (MP3_MAX_SAMPLERATE_INDEX <= ucSampleIndex)
    {
#ifdef FEATURE_FILESOURCE_MP3_PARSER_DEBUG
      MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
        "sample_index value is = %d", ucSampleIndex);
#endif
      error = PARSER_ErrorInHeaderParsing;
    }
  }
   if(PARSER_ErrorNone == error)
   {
     // Get the actual bitrate from bitrate index
     hdr_info.max_bitrate = 1000 *
                        MP3_BITRATE[(hdr_info.version == MP3_VER_1 ? 0 : 1)]
                        [hdr_info.layer][MP3_MAX_BITRATE_INDEX-1];

     // Get the actual bitrate from bitrate index
     hdr_info.bitrate = 1000 *
                        MP3_BITRATE[(hdr_info.version == MP3_VER_1 ? 0 : 1)]
                        [hdr_info.layer][ucBitrateIndex];

     // Get the actual samplerate from samplerate index
     hdr_info.samplerate = MP3_SAMPLING_RATE[hdr_info.version][ucSampleIndex];
     hdr_info.is_padding = (boolean)((frame[MP3HDR_PADDING_OFS] &
                                    MP3HDR_PADDING_M) >> MP3HDR_PADDING_SHIFT);
     hdr_info.is_private = (boolean)
      ((frame[MP3HDR_PRIVATE_OFS] & MP3HDR_PRIVATE_M) >> MP3HDR_PRIVATE_SHIFT);
     hdr_info.channel = (mp3_channel_enum_type)
      ((frame[MP3HDR_CHANNEL_OFS] & MP3HDR_CHANNEL_M) >> MP3HDR_CHANNEL_SHIFT);
     hdr_info.extension = (mp3_ext_enum_type)
        ((frame[MP3HDR_CHANNEL_EXT_OFS] & MP3HDR_CHANNEL_EXT_M)
        >> MP3HDR_CHANNEL_EXT_SHIFT);
     hdr_info.copyright_present = (boolean)
        ((frame[MP3HDR_COPYRIGHT_OFS] & MP3HDR_COPYRIGHT_M)
        >> MP3HDR_COPYRIGHT_SHIFT);
     hdr_info.is_original = (boolean)
        ((frame[MP3HDR_ORIGINAL_OFS] & MP3HDR_ORIGINAL_M)
        >> MP3HDR_ORIGINAL_SHIFT);
     hdr_info.emphasis = (mp3_emphasis_enum_type)
        ((frame[MP3HDR_EMPHASIS_OFS] & MP3HDR_EMPHASIS_M)
        >> MP3HDR_EMPHASIS_SHIFT);
   }
   return error;
}

uint64 mp3Parser::getMediaTimestampForCurrentSample(uint32 /*id*/)
{
  if(m_firstFrame == true && m_isFirstTSValid == true)
  {
    m_isFirstTSValid = false;
#ifdef FEATURE_FILESOURCE_MP3_PARSER_DEBUG
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
       "First frame timestamp= %lu", (uint32)m_firstFrameTS);
#endif
    return (m_firstFrameTS - m_baseTS);
  }
  return 0;
}

bool mp3Parser::GetBaseTime(uint64* nBaseTime)
{
  if(nBaseTime)
  {
    *nBaseTime = m_baseTS;
    return true;
  }
  else
    return false;
}

bool mp3Parser::SetBaseTime(uint64 nBaseTime)
{
  m_baseTS = nBaseTime;
  return true;
}

//=============================================================================
// FUNCTION : calc_playback_duration
//
// DESCRIPTION
//  This function calculates the total playback duration
//
// PARAMETERS
//  duration (out) : Total playback duration
//
// RETURN VALUE
//  AEE_SUCCESS : successfully calculated playback duration
//  AEE_EFAILED : general failure
//
// SIDE EFFECTS
//  None
//=============================================================================
PARSER_ERRORTYPE mp3Parser::calc_playback_duration (uint64 &duration) const
{
  PARSER_ERRORTYPE result = PARSER_ErrorNone;
  duration = 0;
  uint32 frame_count = 0;

  if(m_nFileSize == MAX_FILE_SIZE)
    return result;

  if (!m_is_vbr)
  {
    //CBR file
    if (m_audio_track.end > m_audio_track.start)
    {
      uint32 frame_length = 0;
      // Calculate total number of frames in file
      // Try not to introduce much error from division.
      frame_length = calc_frame_length();
      if (frame_length == 0)
      {
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_ERROR, "Frame Length 0");
        result = PARSER_ErrorStreamCorrupt;
      }
      else
      {
        frame_count = (uint32) (m_audio_track.end - m_audio_track.start)
                               *(10)/frame_length;
        uint32 temp_num_frames = frame_count / 10;
        if (5 < (frame_count - temp_num_frames * 10))
        {
          frame_count = (frame_count / 10) + 1;
        }
        else
        {
          frame_count = frame_count / 10;
        }
      }
    }
  }
  else if (m_vbr_header)
  {
    // VBR file - vbr header is avilable
    result = m_vbr_header->get_total_frames(frame_count);
  }
  // Error conditions when frame_count cab be 0 ----
  // 1. CRB file - Audio track information is not available
  // 2. VBR file - vbr header is not avilable

  // Calculate the total playback duration from number of
  // frames present in the file
  if (PARSER_ErrorNone == result && 0 != frame_count)
  {
    uint64 temp_len_ms = frame_count * (uint64)1000;
    if (temp_len_ms < (uint64)(m_header_info.samplerate))
    {
      duration = (temp_len_ms * MP3_SAMPLES_TABLE
                     [(m_header_info.version == MP3_VER_1 ? 0 : 1)]
                     [m_header_info.layer] * 10) / (m_header_info.samplerate);
      duration = (duration / 10);
    }
    else
    {
      duration = ((temp_len_ms * MP3_SAMPLES_TABLE
                  [(m_header_info.version == MP3_VER_1 ? 0 : 1)]
                  [m_header_info.layer]) / (m_header_info.samplerate));

      /* Duration calculated based on bitrate and sampling rate are differ by
         few millisec. In LA, bitrate param is used for duration calculation.
         In order to pass CTS test cases, same approach has been used in QCOM
         MP3 Parser as well. */
#ifdef _ANDROID_
      if (m_header_info.bitrate)
      {
        duration = ((m_nFileSize - m_audio_track.start) * 8 *
                    MILLISEC_TIMESCALE_UNIT) /
                   m_header_info.bitrate;
      }
#endif
    }
  }
  else
  {
    result = PARSER_ErrorNone;
  }
  return result;
}
//=============================================================================
// FUNCTION calc_frame_length
//
// DESCRIPTION
//  This function calculates the length of an MP3 frame based on information
//  stored in the MP3's header.
//
// PARAMETERS
//  None
//
// RETURN VALUE
//  The frame length in bytes
//
// SIDE EFFECTS
//  None.
//=============================================================================
//
uint32 mp3Parser::calc_frame_length () const
{
   uint32 frame_bytes = 0;
   // Check if input MP3 stream is VBR
   if(m_is_vbr)
   {
      // Input stream is VBR MP3
      uint32 total_frames = 0;
      (void) m_vbr_header->get_total_frames (total_frames);
      uint32 total_bytes = 0;
      (void) m_vbr_header->get_total_bytes (total_bytes);
      // Calculate avarage frame length
      if (total_frames && total_bytes)
      {
         frame_bytes = total_bytes / total_frames;
      }
      else
      {
         frame_bytes = m_header_size;
      }
   }
   else
   {
      // Calculate frame length for CBR input stream
      frame_bytes = calc_frame_length (m_header_info);
   }
   return frame_bytes;
}
//=============================================================================
// FUNCTION : get_duration
//
// DESCRIPTION
//  Calculates Total Playback duration from data found by init_parser.
//  This function is only implemented when the seek_function_defined is set
//  to TRUE
//
// PARAMETERS
//  time  Total playback time for the audio content
//
// RETURN VALUE
//  AEE_SUCCESS : Function succesfully calculated the playback duration
//  AEE_EBADPARM : Invalid input parameters
//  AEE_EBADSTATE : Invalid format parser state
//  AEE_EUNSUPPORTED : Function is not implemented for this audio format
//
// SIDE EFFECTS
//  None
//=============================================================================
PARSER_ERRORTYPE  mp3Parser::get_duration (/*rout*/ uint64* time)
{
  PARSER_ERRORTYPE error = PARSER_ErrorDefault;
  // Check if get_duration is supported
  if (!m_seek_function_defined)
  {
   error = PARSER_ErrorUnsupported;
  }
  else
  {
   // Validate input parameters
   if (NULL == time)
   {
      error = PARSER_ErrorInvalidParam;
   }
   else
   {
     // Initialize the output variables
     *time = 0;
     // Check for format parser state
     if (!m_parse_file_header_done)
     {
       error = PARSER_ErrorDefault;
     }
     else
     {
       //return the duration
       *time = (int64)m_duration;
       error = PARSER_ErrorNone;
     }
   }
  }
  return error;
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
//  AEE_SUCCESS : Function succesfully calculated the seek position
//  AEE_EBADPARM : Invalid input parameters
//  AEE_EBADSTATE : Invalid format parser state
//  AEE_EUNSUPPORTED : Function is not implemented for this audio format
//
// SIDE EFFECTS
//  None
//=============================================================================
PARSER_ERRORTYPE  mp3Parser::get_seek_position (/*in*/ uint64 time,
                                                /*rout*/ uint64* file_position)
{
  PARSER_ERRORTYPE error = PARSER_ErrorDefault;
  // Validate input parameters
  if (NULL == file_position)
  {
    error = PARSER_ErrorDefault;
  }
  else
  {
    // Check for format parser state
    if (!m_parse_file_header_done)
    {
      error = PARSER_ErrorDefault;
    }
    else
    {
      error = PARSER_ErrorNone;
      // Initialize the output parameters
      *file_position = 0;

      // Check for validity of given seek time
       //Given at least MIN_SEEK_DELTA time of data after seek
      if (m_duration <= time + MIN_SEEK_DELTA)
      {
         time = m_duration - MIN_SEEK_DELTA;
      }

      if (!m_is_vbr)
      {
        //! To simplify the calculation use bitrate directly.
        //! Bitrate is in bits/sec units and time is in milli-sec units
        *file_position = m_audio_track.start +
                         (((m_header_info.bitrate / 1000) * time) / 8);
        MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_LOW,
                    "MP3:CBR clip, seek offset value %llu", *file_position);
      }
      else if (m_vbr_header)
      {
        //VBR file - vbr header is avilable
        error = m_vbr_header->get_seek_position(time,m_duration, file_position);
        /* Update the calculated file position to end of ID3 tags.
           Parser has to skip this data, as XING/VBRI tables do not have info
           about ID3 tags. */
        *file_position += m_audio_track.start;
      }
      else
      {
        //VBR file - vbr header is not avilable
        error = PARSER_ErrorDefault;
      }
    }//end of else of if (NULL == file_position)
  }//end of else of if (!m_seek_function_defined)
  return error;
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
PARSER_ERRORTYPE  mp3Parser::get_accurate_seek_position (/*in*/ uint64 RequestedTime,
                                                         /*rout*/ uint64* FilePosition,
                                                         /*rout*/ uint64* SeekedTime)
{
  PARSER_ERRORTYPE error = PARSER_ErrorNone;
  bool bEndOfData= false;
  uint32 nMaxBufSize = m_mp3_audio_info.dwSuggestedBufferSize;
  uint32 nNumBytesRequest = nMaxBufSize;
  uint32 nBytesRead = 0;
  uint8 *dataBuffer = 0;
  uint32 nBytesSkipped = 0;
  uint32 FrameBufLen = 0;
  uint32 FrameTime   = 0;
  uint32 FrameLen    = MP3_FILE_HEADER_SIZE;
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
  *SeekedTime   = 0;

  dataBuffer = (uint8 *)MM_Malloc(nMaxBufSize);
  if(NULL == dataBuffer)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                "Malloc is failed in seek function");
    return PARSER_ErrorMemAllocFail;
  }

  /* Parse frame by frame till we parse the frame closest to the RequestedTime*/
  while(ElapsedTime <= RequestedTime)
  {
    bool frameStartFound = false;
    /* If there is no data available, then read data from the file*/
    if(!nBytesRead)
    {
      CurrentOffset += (nBytesSkipped + FrameBufLen);
      nBytesRead = MP3CallbakGetData (CurrentOffset, nNumBytesRequest, dataBuffer,
                                      nMaxBufSize, m_pUserData, bEndOfData);
      nBytesSkipped = 0;
      FrameBufLen = 0;
    }
    if(nBytesRead < FrameLen && bEndOfData == true)
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL, "GetCurrentSample MP3_READ_FAILURE");
      return PARSER_ErrorEndOfFile;
    }
    else if(nBytesRead < FrameLen)
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH, "GetCurrentSample MP3_DATA_UNDERRUN");
      return PARSER_ErrorDataUnderRun;
    }

    /* Calc frame length and time and update consumed buf size accordingly */
    while(ElapsedTime <= RequestedTime && nBytesRead > FrameBufLen + nBytesSkipped &&
          is_mp3_sync(dataBuffer+FrameBufLen + nBytesSkipped) &&
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

      /* Check whether enough data is available in the buffer or not, before updating nBytesRead param*/
      if((nBytesRead - FrameBufLen) < FrameLen)
      {
        nBytesRead = 0;
        break;
      }

      /* This marks at least there is one frame available in the input file */
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
    if(nBytesRead && false == frameStartFound)
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
      *FilePosition = (CurrentOffset) + FrameBufLen + nBytesSkipped;
      *SeekedTime = ElapsedTime;
    }
    else
    {
      *FilePosition = (LastFrameOffset);
      *SeekedTime = LastFrameTime;
    }
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
FileSourceStatus mp3Parser::SetAudioOutputMode(FileSourceConfigItemEnum henum)
{
  FileSourceStatus status = FILE_SOURCE_FAIL;
  //Default mode is FILE_SOURCE_MEDIA_OUTPUT_BYTES_STREAM/FILE_SOURCE_MEDIA_RETAIN_AUDIO_HEADER
  //We do not support changing output mode during the playback
  switch (henum)
  {
  case FILE_SOURCE_MEDIA_OUTPUT_SINGLE_AUDIO_FRAME:
    //parser can detect frame boundaries in case of ADTS/LOAS only
    //thus SetConfiguration for single frame should be accepeted when aac type is adts
    if( (m_hFrameOutputModeEnum == FILE_SOURCE_MEDIA_OUTPUT_BYTES_STREAM) /* &&
       ((m_mp3_format == MP3_FORMAT_ADTS) || (m_aac_format == AAC_FORMAT_LOAS) ) */)
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
FileSourceStatus mp3Parser::GetAudioOutputMode(bool* bret, FileSourceConfigItemEnum henum)
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

