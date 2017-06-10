//=============================================================================
// FILE: amrformatparser.cpp
//
// SERVICES: AUDIO
//
// DESCRIPTION: defines functions that parse AMRNB files
//
// Copyright (c) 2009-2015 Qualcomm Technologies Inc, All Rights Reserved.
// Qualcomm Technologies Proprietary and Confidential.

/*=========================================================================================================

                             PERFORCE HEADER

$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/AMRNBParserLib/main/latest/src/amrformatparser.cpp#31 $
$DateTime: 2014/02/07 02:53:30 $
$Change: 5240686 $

===========================================================================================================*/

//=============================================================================
#include "parserdatadef.h"
#include "amrparser.h"
#include "seektable.h"
#include "filebase.h"
#include "seek.h"
#include "MMMemory.h"

const int AMR_NB_FS_frame_size_table[] = {
   13 // FT=0
   ,14 // FT=1
   ,16 // FT=2
   ,18 // FT=3
   ,20 // FT=4
   ,21 // FT=5
   ,27 // FT=6
   ,32 // FT=7
   ,6  // FT=8  ** SID **
   ,7  // FT=9  **  SID **
   ,6  // FT=10 ** SID **
   ,6  // FT=11 ** SID **
   ,1  // FT=12 ** future use **
   ,1  // FT=13 ** future use **
   ,1  // FT=14 ** future use **
   ,1  // FT=15 ** no data **
};


const int* frame_size_lookup_table[] = {
      NULL                               // UNKNOWN
      ,&AMR_NB_FS_frame_size_table[0]    // AMR_NB_FS
      ,NULL                              // AMR_WB
      ,NULL                              // AMR_MC
      ,NULL                              // AMR_WB_MC
};

//=============================================================================
// FUNCTION: Constructor
//
// DESCRIPTION:
//  Create an instance of amrParser, and init the class attributes
//
// PARAMETERS
//  pUData  User data passed in
//  fsize   File Size
//  FilePtr File Ptr passed in
//
// RETURN:
//  None
//
// SIDE EFFECTS
//  None
//=============================================================================
amrParser::amrParser
(
 void *pUserData,
 uint64 fsize,
 OSCL_FILE *FilePtr
)
{
  m_pUserData = pUserData;
  m_nFileSize = fsize;
  m_CurrentParserState = PARSER_IDLE;
  m_AMRFilePtr = FilePtr;
  m_nCurrOffset = 0;
  pseek = NULL;
  psimple_seektable = NULL;
  m_dataBuffer = NULL;
  m_ullCurrPbTime = 0;
  memset(&m_amr_header_amrh,0,sizeof(amr_header_amrh));
  memset(&m_amr_audio_info,0,sizeof(amr_audio_info));
  memset(&m_amr_current_frame, 0 , sizeof(amr_header_type));
  memset(&m_ReadBuffer,0,AMR_READ_BUFFER_SIZE);
  // Default frame output mode is FILE_SOURCE_MEDIA_OUTPUT_BYTES_STREAM
  // Default frame header mode is FILE_SOURCE_MEDIA_RETAIN_AUDIO_HEADER
  m_eFrameModeCfg  = FILE_SOURCE_MEDIA_OUTPUT_BYTES_STREAM;
  m_eHeaderModeCfg = FILE_SOURCE_MEDIA_RETAIN_AUDIO_HEADER;
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
amrParser::~amrParser()
{
  if(psimple_seektable)
  {
    MM_Delete( psimple_seektable);
    psimple_seektable = NULL;
  }
  if(pseek)
  {
    MM_Delete( pseek);
    pseek = NULL;
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
PARSER_ERRORTYPE amrParser::StartParsing(void)
{
  PARSER_ERRORTYPE retError = PARSER_ErrorDefault;
  if(!m_pUserData)
  {
    MM_MSG_PRIO(
      MM_FILE_OPS,
      MM_PRIO_ERROR,
      "amrParser::StartParsing() Invalid user data!!");
    return PARSER_ErrorDefault;
  }

  parse_amr_file_header();
  parse_amr_audio_data();
  m_nCurrOffset += AMR_FILE_HEADER_SIZE;
  retError = PARSER_ErrorNone;
  return retError;
}

//=============================================================================
// FUNCTION: parse_amr_file_header
//
// DESCRIPTION:
//  parse AMR file header
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

void amrParser::parse_amr_file_header()
{
  m_amr_header_amrh.nChannels = 1;
  m_amr_header_amrh.nSampleRate = AMR_SAMPLE_RATE;
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
//  frame :      A byte pointer to frame_size bytes, the frame header to be parsed
//               Note: the entire frame header is expected, thus it must be
//               at least  <em>frame_header_size</em> in length.
//  frame_size : frame size determine from parsing the frame header
//  frame_time : frame duration in milliseconds determine from parsing frame
//               header
//
// RETURN VALUE
//  IAUDIO_SUCCESS : Frame header was valid, and frame size is returned.
//  IAUDIO_FAILURE : Frame header/input parameter was INVALID.
//
// SIDE EFFECTS
//  None
//=============================================================================
//
IAudioReturnType amrParser::parse_frame_header
(
 byte* frame,
 uint32* frame_size,
 uint32* frame_time)
{
  IAudioReturnType result = IAUDIO_FAILURE;
  // Validate input parameters
  if((NULL == frame)||(NULL == frame_size)||(NULL == frame_time))
  {
    MM_MSG_PRIO(
      MM_FILE_OPS,
      MM_PRIO_FATAL,
      "amrformatparser::parse_frame_header(): Invalid param passed in!");
    result = IAUDIO_FAILURE;
  }
  else
  {
    // Initialize the output variables
    *frame_size = 0;
    *frame_time = 0;
    // parse the frame header
    amr_header_type header;
    if ( PARSER_ErrorNone == \
      parse_amr_frame_header( frame[0], AMR_NB_FS, header ) )
    {
      // successfully parsed the frame header - copy the frame size
      *frame_size = header.frame_size;
      // update the frame time
      *frame_time = header.frame_time;
      result = IAUDIO_SUCCESS;
    }
  }
  return result;
}


//=============================================================================
// FUNCTION : parse_amr_frame_header
//
// DESCRIPTION
//  Parse a frame header according to audio specific format and return the
//  calculated frame size.
//
// PARAMETERS
//  frame : input bit stream
//  format : AMR File format
//  header : AMR Frame header info
//
// RETURN VALUE
//  PARSER_ErrorNone: Frame header was valid, and frame size/time is returned
//  PARSER_ErrorDefault: Frame header was INVALID, frame size is not valid
//
// SIDE EFFECTS
//  None
//=============================================================================
//
PARSER_ERRORTYPE amrParser::parse_amr_frame_header
(
 uint8 frame,
 amr_format_type /*format*/,
 amr_header_type &header
 )
{
  PARSER_ERRORTYPE eStatus = PARSER_ErrorNone;
  (void) memset (&header, 0, sizeof(header));
  eStatus = parse_amr_nb_fs(frame,header);
  if( PARSER_ErrorNone == eStatus )
  {
    header.frame_time = AMR_FRAME_TIME;
  }
  return eStatus;
}

//=============================================================================
// FUNCTION : parse_amr_nb_fs
//
// DESCRIPTION
// Parse a frame header of AMR_NB_FS type
//
// PARAMETERS
//  frame : input bit stream
//  header : AMR Frame header info
//
// RETURN VALUE
//  PARSER_ErrorNone: Frame header was valid, and frame size is returned
//  PARSER_ErrorDefault: Frame header was INVALID, frame size is not valid
//
// SIDE EFFECTS
//  None
//=============================================================================
//
PARSER_ERRORTYPE amrParser::parse_amr_nb_fs
(
 uint8 frame,
 amr_header_type &header
 )
{
  PARSER_ERRORTYPE eStatus = PARSER_ErrorNone;
  // Get the frame type
   header.frame_type = (uint8)( ( frame & AUDIO_AMR_NB_FS::FRAME_TYPE_MASK)
                                 >> AUDIO_AMR_NB_FS::FRAME_TYPE_BYTE_OFFSET );
   // Get the frame quality indicator
   header.frame_quality_indicator = \
      (uint8)( ( frame & AUDIO_AMR_NB_FS::FRAME_QUALITY_MASK )
                 >> AUDIO_AMR_NB_FS::FRAME_QUALITY_BYTE_OFFSET );

   // get number of bytes per frame from our lookup table
   header.frame_size = (uint32)
      frame_size_lookup_table[AMR_NB_FS][header.frame_type];
   // Store current frame information
    memcpy(&m_amr_current_frame, &header, sizeof(amr_header_type));
   // Check for valid frame type. FT > 8 & FT < 14 are invalid FT
   if( ( header.frame_type >11 ) && ( header.frame_type <15 ) )
   {
     eStatus = PARSER_ErrorDefault;
     MM_MSG_PRIO1(
       MM_FILE_OPS,
       MM_PRIO_HIGH,
       "amrParser::parse_amr_nb_fs()Invalid Frame Type = %d!!",header.frame_type);
   }

   return eStatus;
}
//=============================================================================
// FUNCTION: parse_amr_audio_data
//
// DESCRIPTION:
//  parse amr audio data information
//
// PARAMETERS
//  m_amr_audio_info - pointer to the amr audio info structure
//
// RETURN:
//  None
//
// SIDE EFFECTS
//  None
//=============================================================================
//

void amrParser::parse_amr_audio_data()
{
   m_amr_audio_info.dwSuggestedBufferSize = AMR_DEFAULT_BUF_SIZE;
}

//=============================================================================
// FUNCTION: GetAMRHeader
//
// DESCRIPTION:
//  Parse the amr file header
//
// PARAMETERS
//  pAmrHdrPtr - pointer to amr header structure
//
// RETURN:
//  amrErrorType
//
// SIDE EFFECTS
//  None
//=============================================================================
//

PARSER_ERRORTYPE amrParser::GetAMRHeader(amr_header_amrh* pAmrHdrPtr)
{
   PARSER_ERRORTYPE retError = PARSER_ErrorDefault;
   amr_header_amrh *p_amr_header_amrh;
   p_amr_header_amrh = &m_amr_header_amrh;
   if(!pAmrHdrPtr)
   {
     //MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
     //   "GetAVIHeader AMR_INVALID_USER_DATA");
     retError = PARSER_ErrorInvalidParam;
     return retError;
   }
   if(NULL == p_amr_header_amrh)
   {
     //MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
     //   "GetAMRHeader,NULL AMR Header,AMR_PARSE_ERROR");
     retError = PARSER_ErrorDefault;
     return retError;
   }
   memset(pAmrHdrPtr,0,sizeof(amr_header_amrh));
   memcpy(pAmrHdrPtr,p_amr_header_amrh,sizeof(amr_header_amrh));
   retError = PARSER_ErrorNone;
   return retError;
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
//
PARSER_ERRORTYPE amrParser::GetAudioInfo(amr_audio_info* pAudioInfo)
{
   PARSER_ERRORTYPE retError = PARSER_ErrorDefault;
   amr_audio_info* p_amr_audio_info;
   p_amr_audio_info = &m_amr_audio_info;
   if(NULL == p_amr_audio_info)
   {
      return retError;
   }
   else
   {
      //pAudioInfo = p_amr_audio_info;
      memset(pAudioInfo,0,sizeof(amr_audio_info));
      memcpy(pAudioInfo,p_amr_audio_info,sizeof(amr_audio_info));
      retError = PARSER_ErrorNone;
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
uint64 amrParser::GetClipDurationInMsec()
{
  uint32 noEntries = 50;
  uint64 uDuration = 0 ;

  if (m_nFileSize > FILE_UPPER_BOUNDARY)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
      "GetClipDuration : File Size %llu is larger, Dur calc is not supported");
  }
  else if(psimple_seektable == NULL)
  {
    psimple_seektable = MM_New_Args(simple_seektable, (noEntries));
    if(OSCL_FileSeek(m_AMRFilePtr, AMR_FILE_HEADER_SIZE, SEEK_CUR))
    {
      MM_MSG_PRIO(
        MM_FILE_OPS,
        MM_PRIO_LOW,
        "OSCL_FileSeek is successful");
    }
  }

  if(pseek == NULL && psimple_seektable != NULL)
  {
    pseek = MM_New_Args(seek,(psimple_seektable
                            ,this
                            ,m_AMRFilePtr
                            ,AMR_FRAME_HEADER_SIZE
                            ,AMR_FILE_HEADER_SIZE
                            ,m_nFileSize));
  }

  if(NULL != pseek)
  {
    uDuration = pseek->get_duration();
    if(!uDuration)
    {
      return 0;
    }
    if(pseek->set_mode(PREGENERATE_TABLE))
    {
      return 0;
    }
  }
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
//  amrErrorType
//
// SIDE EFFECTS
//  None
//=============================================================================
//
PARSER_ERRORTYPE amrParser::GetCurrentSample( uint8* pucDataBuffer,
                                              uint32 ulMaxBufSize,
                                              uint32 *pulBytesNeeded
 )
{
  if( (!pucDataBuffer) || (!ulMaxBufSize) ||  (!pulBytesNeeded) )
  {
    MM_MSG_PRIO(
      MM_FILE_OPS,
      MM_PRIO_ERROR,
      "amrParser::GetCurrentSample() Invlaid data!!");
    return PARSER_ErrorInvalidParam;
  }

  uint32 ulNumBytesRequest = *pulBytesNeeded;
  uint32 ulBytesRead = 0;

  m_dataBuffer = pucDataBuffer;
  if(m_nCurrOffset >= m_nFileSize)
  {
    *pulBytesNeeded  = 0;
    MM_MSG_PRIO(MM_FILE_OPS,
      MM_PRIO_HIGH,
      "GetCurrentSample AMR_END_OF_FILE");
    return PARSER_ErrorEndOfFile;
  }
  //Check audio output mode, if set SINGLE_AUDIO_FRAME then give
  //data frame by frame.
  if( FILE_SOURCE_MEDIA_OUTPUT_SINGLE_AUDIO_FRAME == m_eFrameModeCfg )
  {
    uint32 ulFrameSize = 0;
    uint32 ulFrameTime = 0;
    // Read frame header first which is of 1 byte long
    ulBytesRead = AMRCallbakGetData ( m_nCurrOffset,
                                     1,
                                     m_dataBuffer,
                                     ulMaxBufSize,
                                     m_pUserData );

    // Parse frame header & calculate frame_size & frame_time.
    // In case of erroneous clip, frame_size or frame_type can be corrupted
    // so return EOF irrespective of current offset position.
    if( ( ulBytesRead ) &&
        ( IAUDIO_SUCCESS == \
          parse_frame_header(m_dataBuffer, &ulFrameSize, &ulFrameTime ) ) &&
        ( ulFrameSize ) )
      {
        m_ullCurrPbTime += ulFrameTime;
        //Read data worth of full frame size
        ulBytesRead = AMRCallbakGetData ( m_nCurrOffset,
                                         ulFrameSize,
                                         m_dataBuffer,
                                         ulMaxBufSize,
                                         m_pUserData );
        m_nCurrOffset += ulBytesRead;
        *pulBytesNeeded = ulBytesRead;
      }
      else
      {
        MM_MSG_PRIO(
          MM_FILE_OPS,
          MM_PRIO_FATAL,
          "parse_frame_header failed!! Reporting EOF ");
        //this will trigger EOF
        *pulBytesNeeded = 0;
        return PARSER_ErrorEndOfFile;
      }
  }//if(m_eFrameModeCfg)
  else
  {
    ulBytesRead = AMRCallbakGetData ( m_nCurrOffset,
                                     ulNumBytesRequest,
                                     m_dataBuffer,
                                     ulMaxBufSize,
                                     m_pUserData );
    if(!ulBytesRead)
    {
      m_CurrentParserState = PARSER_READ_FAILED;
      *pulBytesNeeded  = 0;
      MM_MSG_PRIO(MM_FILE_OPS,
        MM_PRIO_FATAL,
        "GetCurrentSample AMR_READ_FAILURE");
      return PARSER_ErrorReadFail;
    }
    m_nCurrOffset += ulBytesRead;
    *pulBytesNeeded = ulBytesRead;
  }

  m_CurrentParserState = PARSER_READY;
  return PARSER_ErrorNone;
}

//=============================================================================
// FUNCTION: init_file_position
//
// DESCRIPTION:
//  This function initialize the file offset to reset position
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

void amrParser::init_file_position()
{
   m_nCurrOffset = 0;
   m_nCurrOffset += AMR_FILE_HEADER_SIZE;
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
void amrParser::set_newfile_position(uint64 file_position)
{
   m_nCurrOffset = file_position;
}

//=============================================================================
// FUNCTION: Seek
//
// DESCRIPTION:
//  This function will return the new file position for the given input time
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
//
uint64 amrParser::Seek(uint64 ullReposTime)
{
   uint64 ullTime = ullReposTime;
   if(NULL != pseek)
   {
      ullTime = pseek->process_seek(ullReposTime);
   }
   //Update current time to repositioned time
   m_ullCurrPbTime = ullTime;
   return ullTime;
}
/* ======================================================================
  FUNCTION:
    SetAudioOutputMode

  DESCRIPTION:
    Called by user to set output mode specified by eConfigParam

  INPUT/OUTPUT PARAMETERS:
    eConfigParam-Output mode

  RETURN VALUE:
   PARSER_ErrorNone if successful in setting output mode
   else returns PARSER_ErrorDefault

  SIDE EFFECTS:
    None.
======================================================================*/
PARSER_ERRORTYPE amrParser::SetAudioOutputMode
(
   FileSourceConfigItemEnum eConfigParam
 )
{
  PARSER_ERRORTYPE eStatus = PARSER_ErrorDefault;
  // Default mode is
  // FrameOutput = FILE_SOURCE_MEDIA_OUTPUT_BYTES_STREAM/
  // FrameHeader = FILE_SOURCE_MEDIA_RETAIN_AUDIO_HEADER
  switch (eConfigParam)
  {
  case FILE_SOURCE_MEDIA_OUTPUT_SINGLE_AUDIO_FRAME:
    {
      m_eFrameModeCfg = eConfigParam;
      eStatus = PARSER_ErrorNone;
    }
    break;

  case FILE_SOURCE_MEDIA_STRIP_AUDIO_HEADER:
    if (FILE_SOURCE_MEDIA_RETAIN_AUDIO_HEADER == m_eHeaderModeCfg )
    {
      m_eHeaderModeCfg = eConfigParam;
      eStatus = PARSER_ErrorNone;
    }
    break;

  default:
    {
      MM_MSG_PRIO1(MM_FILE_OPS,
       MM_PRIO_ERROR,
      "amrParser::SetAudioOutputMode(): Invalid eConfigParam %d",eConfigParam);
    }
    break;
  }//switch(eConfigParam)

  return eStatus;
}
/* ======================================================================
  FUNCTION:
    GetAudioOutputMode

  DESCRIPTION:
    Called by user to retrieve output mode specified by eConfigParam

  INPUT/OUTPUT PARAMETERS:
    pbConfigStatus-Output mode status
    eConfigParam-Output mode

  RETURN VALUE:
   PARSER_ErrorNone if successful in retrieving output mode
   else returns FILE_SPARSER_ErrorDefault

  SIDE EFFECTS:
    None.
======================================================================*/
PARSER_ERRORTYPE amrParser::GetAudioOutputMode(
  bool* pbConfigStatus,
  FileSourceConfigItemEnum eConfigParam
  )
{
  PARSER_ERRORTYPE eStatus = PARSER_ErrorDefault;
  // Default mode
  // FrameOutput = FILE_SOURCE_MEDIA_OUTPUT_BYTES_STREAM/
  // FrameHeader = FILE_SOURCE_MEDIA_RETAIN_AUDIO_HEADER
  switch (eConfigParam)
  {
  case FILE_SOURCE_MEDIA_OUTPUT_SINGLE_AUDIO_FRAME:
    if (m_eFrameModeCfg == eConfigParam)
    {
      *pbConfigStatus = true;
      eStatus = PARSER_ErrorNone;
    }
    break;

  case FILE_SOURCE_MEDIA_STRIP_AUDIO_HEADER:
    if (m_eHeaderModeCfg == eConfigParam)
    {
      *pbConfigStatus = true;
      eStatus = PARSER_ErrorNone;
    }
    break;

  default:
    {
      *pbConfigStatus = false;
      MM_MSG_PRIO1(MM_FILE_OPS,
        MM_PRIO_ERROR,
        "CAMRParser::SetAudioOutputMode(): Invalid eConfigParam %d",
        eConfigParam);
    }
    break;
  }//switch(eConfigParam)
  return eStatus;
}
