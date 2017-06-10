// -*- Mode: C++ -*-
//=============================================================================
// FILE: amrwbformatparser.cpp
//
// SERVICES: AUDIO
//
// DESCRIPTION: defines functions that parse AMRWBNB files
//
//Copyright (c) 2009-2015 Qualcomm Technologies Inc, All Rights Reserved.
//Qualcomm Technologies Proprietary and Confidential.

/*=========================================================================================================

                             PERFORCE HEADER

$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/AMRWBParserLib/main/latest/src/amrwbparser.cpp#27 $
$DateTime: 2014/02/07 02:53:30 $
$Change: 5240686 $

===========================================================================================================*/


//=============================================================================
#include "parserdatadef.h"
#include "amrwbparser.h"
#include "seektable.h"
#include "filebase.h"
#include "seek.h"
#include "MMMemory.h"

const int AMRWB_SC_FS_frame_size_table[] = {
    18 // FT=0
   ,24 // FT=1
   ,33 // FT=2
   ,37 // FT=3
   ,41 // FT=4
   ,47 // FT=5
   ,51 // FT=6
   ,59 // FT=7
   ,61  // FT=8
   ,6  // FT=9  ** Invalid FT **
   ,0  // FT=10 ** Invalid FT **
   ,0  // FT=11 ** Invalid FT **
   ,0  // FT=12 ** Invalid FT **
   ,0  // FT=13 ** Invalid FT **
   ,1  // FT=14 ** Speech Lost **
   ,1  // FT=15 ** No Data **
};


const int* amrwb_frame_size_lookup_table[] = {
    NULL                              // AMRWB_UNKNOWN
   ,&AMRWB_SC_FS_frame_size_table[0]    // AMR_WB_FS
   ,NULL                              // AMR_WB_MC
};



//=============================================================================
// FUNCTION: Constructor
//
// DESCRIPTION:
//  Create an instance of amrwbformatparser, and init the class attributes
//
// PARAMETERS
//  pUserData : This is the interface to the component services environment
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

amrwbParser::amrwbParser(void* pUserData,
                         uint64 fsize,
                         OSCL_FILE *FilePtr)
{
  m_pUserData = pUserData;
  m_nFileSize = fsize;
  m_CurrentParserState = PARSER_IDLE;
  m_AMRWBFilePtr = FilePtr;
  m_nCurrOffset = 0;
  pseek = NULL;
  psimple_seektable = NULL;
  m_dataBuffer = NULL;
  m_ullCurrPbTime = 0;
  memset(&m_amrwb_header_amrwbh,0,sizeof(amrwb_header_amrwbh));
  memset(&m_amrwb_audio_info,0,sizeof(amrwb_audio_info));
  memset(&m_ReadBuffer,0,AMRWB_READ_BUFFER_SIZE);
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
amrwbParser::~amrwbParser()
{

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
//  amrwbErrorType
//
// SIDE EFFECTS
//  None
//=============================================================================
PARSER_ERRORTYPE amrwbParser::StartParsing(void)
{
  PARSER_ERRORTYPE retError = PARSER_ErrorDefault;
  if(m_pUserData)
  {
    parse_amrwb_file_header();
    parse_amrwb_audio_data();
    m_nCurrOffset += AMRWB_FILE_HEADER_SIZE;
    retError = PARSER_ErrorNone;
  }
  else
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                   "StartParsing AMRWB_INVALID_USER_DATA");
    retError = PARSER_ErrorDefault;
  }
  return retError;
}

//=============================================================================
// FUNCTION: parse_amrwb_file_header
//
// DESCRIPTION:
//  parse amrwb file header
//
// PARAMETERS
//  amrwb_header_amrwbh - pointer to the amrwb header structure
//
// RETURN:
//  None
//
// SIDE EFFECTS
//  None
//=============================================================================

void amrwbParser::parse_amrwb_file_header()
{
  m_amrwb_header_amrwbh.nChannels = 1;
  m_amrwb_header_amrwbh.nSampleRate = AMRWB_SAMPLE_RATE;
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
//  frame_time : frame duration in milliseconds determine from parsing frame
//               header
//
// RETURN VALUE
//  IAUDIO_SUCCESS : Frame header was valid, and frame size is returned
//  IAUDIO_FAILURE : Frame header was INVALID, frame size is not valid
//
// SIDE EFFECTS
//  None
//=============================================================================

IAudioReturnType amrwbParser::parse_frame_header (byte* frame,
                                                  uint32* frame_size,
                                                  uint32* frame_time)
{
  IAudioReturnType result = IAUDIO_SUCCESS;

  // Validate input parameters
  if( (!frame) || (!frame_size) || (!frame_time) )
  {
    result = IAUDIO_FAILURE;
    MM_MSG_PRIO(
      MM_FILE_OPS,
      MM_PRIO_FATAL,
      "amrwbParser::parse_frame_header: Invalid parameter passed in!!");
  }
  else
  {
    // Initialize the output variables
    *frame_size = 0;
    *frame_time = 0;
    // parse the frame header
    amrwb_header_type header;
    if ( PARSER_ErrorNone == \
         parse_amrwb_frame_header(frame[0],AMRWB_SC_FS,header))
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
// FUNCTION : parse_amrwb_frame_header
//
// DESCRIPTION
//  Parse a frame header according to audio specific format and return the
//  calculated frame size.
//
// PARAMETERS
//  frame : input bit-stream
//  format : AMRWB File format
//  header : AMRWB Frame header info
//
// RETURN VALUE
//  PARSER_ErrorNone: Frame header was valid, and frame size & time is returned
//  PARSER_ErrorDefault: Frame header was INVALID, frame size & time is not valid
//
// SIDE EFFECTS
//  None
//=============================================================================
//
uint32 amrwbParser::parse_amrwb_frame_header (uint8 frame,
                                              amrwb_format_type /*format*/,
                                              amrwb_header_type &header) const
{
  PARSER_ERRORTYPE eStatus = PARSER_ErrorNone;
  (void) memset (&header, 0, sizeof(header));
  eStatus = parse_amrwb_fs(frame,header);
  if(PARSER_ErrorNone == eStatus)
  {
    header.frame_time = AMRWB_FRAME_TIME;
  }
  return eStatus;
}
//=============================================================================
// FUNCTION : parse_amrwb_fs
//
// DESCRIPTION
// Parse a frame header of AMRWB_FS type
//
// PARAMETERS
//  frame : input bit-stream
//  header : AMRWB Frame header info
//
// RETURN VALUE
//  PARSER_ErrorNone: Frame header was valid, and frame size & time is returned
//  PARSER_ErrorDefault: Frame header was INVALID, frame size & time is not valid
//
// SIDE EFFECTS
//  None
//=============================================================================
//
PARSER_ERRORTYPE amrwbParser::parse_amrwb_fs(uint8 frame,
                                             amrwb_header_type &header) const
{
  PARSER_ERRORTYPE eStatus = PARSER_ErrorNone;
  // Get the frame type
  header.frame_type = uint8((frame & AUDIO_AMRWB_FS::FRAME_TYPE_MASK)
                      >> AUDIO_AMRWB_FS::FRAME_TYPE_BYTE_OFFSET);
  // Get the frame quality indicator
  header.frame_quality_indicator = uint8(
                                (frame & AUDIO_AMRWB_FS::FRAME_QUALITY_MASK)
                                >> AUDIO_AMRWB_FS::FRAME_QUALITY_BYTE_OFFSET);

  // get number of bytes per frame from our lookup table
  header.frame_size = (uint32)
              amrwb_frame_size_lookup_table[AMRWB_SC_FS][header.frame_type];

  // Check for valid frame type. FT > 9 & FT < 14 are invalid FT
  if(header.frame_type > 9 &&  header.frame_type < 14)
  {
    eStatus = PARSER_ErrorDefault;
    MM_MSG_PRIO(
      MM_FILE_OPS,
      MM_PRIO_HIGH,
      "amrwbParser::parse_amr_nb_fs()Invalid Frame Type!!");

  }
  return eStatus;
}
//=============================================================================
// FUNCTION: parse_amrwb_audio_data
//
// DESCRIPTION:
//  parse amrwb audio data information
//
// PARAMETERS
//  m_amrwb_audio_info - pointer to the amrwb audio info structure
//
// RETURN:
//  None
//
// SIDE EFFECTS
//  None
//=============================================================================
//

void amrwbParser::parse_amrwb_audio_data()
{
  m_amrwb_audio_info.dwSuggestedBufferSize = AMRWB_DEFAULT_BUF_SIZE;
}


//=============================================================================
// FUNCTION: GetAMRWBHeader
//
// DESCRIPTION:
//  Parse the amrwb file header
//
// PARAMETERS
//  pAmrHdrPtr - pointer to amrwb header structure
//
// RETURN:
//  amrwbErrorType
//
// SIDE EFFECTS
//  None
//=============================================================================
//

PARSER_ERRORTYPE amrwbParser::GetAMRWBHeader(amrwb_header_amrwbh* pAmrHdrPtr)
{
  PARSER_ERRORTYPE retError = PARSER_ErrorDefault;
  amrwb_header_amrwbh *p_amrwb_header_amrwbh;
  p_amrwb_header_amrwbh = &m_amrwb_header_amrwbh;
  if(!pAmrHdrPtr)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                   "GetAMRWBHeader AMRWB_INVALID_USER_DATA");
    retError = PARSER_ErrorInvalidParam;
    return retError;
  }
  if(NULL == p_amrwb_header_amrwbh)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                   "GetAMRWBHeader,NULL AMRWB Header,AMRWB_PARSE_ERROR");
    retError = PARSER_ErrorDefault;
    return retError;
  }
  memset(pAmrHdrPtr,0,sizeof(amrwb_header_amrwbh));
  memcpy(pAmrHdrPtr,p_amrwb_header_amrwbh,sizeof(amrwb_header_amrwbh));
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
//  pAudioInfo - pointer to the amrwb_audio_info structure
//
// RETURN:
//  amrwbErrorType
//
// SIDE EFFECTS
//  None
//=============================================================================
//
PARSER_ERRORTYPE amrwbParser::GetAudioInfo(amrwb_audio_info* pAudioInfo)
{

  PARSER_ERRORTYPE retError = PARSER_ErrorDefault;
  amrwb_audio_info* p_amrwb_audio_info;
  p_amrwb_audio_info = &m_amrwb_audio_info;
  if(NULL == p_amrwb_audio_info)
  {
    return retError;
  }
  else
  {
    //pAudioInfo = p_amrwb_audio_info;
    memset(pAudioInfo,0,sizeof(amrwb_audio_info));
    memcpy(pAudioInfo,p_amrwb_audio_info,sizeof(amrwb_audio_info));
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
uint64 amrwbParser::GetClipDurationInMsec()
{
  uint32 noEntries = 50;

  uint64 uDuration = 0;

  if (m_nFileSize > FILE_UPPER_BOUNDARY)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
      "GetClipDuration : File Size %llu is larger, Dur calc is not supported");
  }
  else if(psimple_seektable == NULL)
  {
    psimple_seektable = MM_New_Args(simple_seektable,(noEntries));
    if(OSCL_FileSeek(m_AMRWBFilePtr,AMRWB_FILE_HEADER_SIZE,SEEK_CUR))
    {
      MM_MSG_PRIO(
        MM_FILE_OPS,
        MM_PRIO_LOW,
        "OSCL_FileSeek is successful");
    }
  }//if(psimple_seektable)

  if( pseek == NULL && psimple_seektable != NULL)
  {
    pseek = MM_New_Args(seek,(psimple_seektable
                            ,this
                            ,m_AMRWBFilePtr
                            ,AMRWB_FRAME_HEADER_SIZE
                            ,AMRWB_FILE_HEADER_SIZE
                            ,m_nFileSize));
  }

  if( NULL != pseek)
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
//  amrwbErrorType
//
// SIDE EFFECTS
//  None
//=============================================================================
//
PARSER_ERRORTYPE amrwbParser::GetCurrentSample(uint8* ucDataBuffer,
                                               uint32 ulMaxBufSize,
                                               uint32 *pulBytesNeeded)
{
  if( (!ucDataBuffer) || (!ulMaxBufSize) ||  (!pulBytesNeeded) )
  {
    MM_MSG_PRIO(
      MM_FILE_OPS,
      MM_PRIO_ERROR,
      "amrwbParser::GetCurrentSample() Invalid Data !!");
    return PARSER_ErrorInvalidParam;
  }

  uint32 ulNumBytesRequest = *pulBytesNeeded;
  uint32 ulBytesRead = 0;
  m_dataBuffer = ucDataBuffer;
  if(m_nCurrOffset >= m_nFileSize)
  {
    *pulBytesNeeded  = 0;
    MM_MSG_PRIO(MM_FILE_OPS,
        MM_PRIO_HIGH,
        "GetCurrentSample AMRWB_END_OF_FILE");
    return PARSER_ErrorEndOfFile;
  }
  //Check audio output mode, if set SINGLE_AUDIO_FRAME then give
  //data frame by frame.
  if( FILE_SOURCE_MEDIA_OUTPUT_SINGLE_AUDIO_FRAME == m_eFrameModeCfg )
  {
    uint32 ulFrameSize = 0;
    uint32 ulFrameTime = 0;
    // Read frame header first which is of 1 byte long
    ulBytesRead = AMRWBCallbakGetData ( m_nCurrOffset,
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
      // Update current time
      m_ullCurrPbTime += ulFrameTime;
      // Read data worth of frame size
      ulBytesRead = AMRWBCallbakGetData ( m_nCurrOffset,
                                          ulFrameSize,
                                          m_dataBuffer,
                                          ulMaxBufSize,
                                          m_pUserData );
      // Update current offset & return byte read
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
  }
  else
  {
    ulBytesRead = AMRWBCallbakGetData ( m_nCurrOffset,
                                        ulNumBytesRequest,
                                        m_dataBuffer,
                                        ulMaxBufSize,
                                        m_pUserData );
    if(!(ulBytesRead))
    {
      m_CurrentParserState = PARSER_READ_FAILED;
      *pulBytesNeeded  = 0;
      MM_MSG_PRIO(MM_FILE_OPS,
                     MM_PRIO_FATAL,
                     "GetCurrentSample AMRWB_READ_FAILURE");
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

void amrwbParser::init_file_position()
{
  m_nCurrOffset = 0;
  m_nCurrOffset += AMRWB_FILE_HEADER_SIZE;
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
void amrwbParser::set_newfile_position(uint64 file_position)
{
  m_nCurrOffset = file_position;
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
//  amrwbErrorType
//
// SIDE EFFECTS
//  None
//=============================================================================
//
uint64 amrwbParser::Seek(uint64 ullReposTime)
{
  uint64 ullTime = ullReposTime;
  if(NULL != pseek)
  {
    ullTime = pseek->process_seek(ullReposTime);
  }
  //Update current playback to repositioned time
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
PARSER_ERRORTYPE amrwbParser::SetAudioOutputMode
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
      "amrwbParser::SetAudioOutputMode(): Invalid eConfigParam %d",eConfigParam);
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
PARSER_ERRORTYPE amrwbParser::GetAudioOutputMode(
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
        "amrwbParser::SetAudioOutputMode(): Invalid eConfigParam %d",
        eConfigParam);
    }
    break;
  }//switch(eConfigParam)
  return eStatus;
}
