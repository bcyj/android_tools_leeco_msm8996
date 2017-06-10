/* -*- Mode: C++ -*-
  =============================================================================
  FILE: qcpparser.cpp

  SERVICES: AUDIO

  DESCRIPTION: defines functions that parse QCP files

  Copyright (c) 2009-2015 QUALCOMM Technologies Inc, All Rights Reserved.
  QUALCOMM Technologies Proprietary and Confidential.

  $Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/QCPParserLib/main/latest/src/qcpparser.cpp#37 $
  $DateTime: 2014/02/07 02:53:30 $
  $Change: 5240686 $

  ========================================================================== */


//=============================================================================
// INCLUDES
//=============================================================================

#include "qcpparser.h"
#include "MMMemory.h"

/* Maximum number of frames that need to be parsed to calculate approximate
   duration value */
#define MAX_QCELP_FRAMES 256

/* Maximum file size value.
   This value will be used as upper boundary. If input file size is more than
   this value, then Parser will return approximate duration value. */
#define MAX_QCELP_FILE_SIZE (64 * 1024 * 1024)
//=============================================================================
// FUNCTION DEFINITONS
//=============================================================================
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
   return uint16((p_value[0]) | ((p_value[1]) << 8) );
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
// FUNCTION: Constructor
//
// DESCRIPTION:
//  Create an instance of qcpparser, and init the class attributes
//
// PARAMETERS
//  pUData : This is the interface to the component services environment
//  fsize : size of the file
//  FilePtr: pointer to the file interface
//
// RETURN:
//  None
//
// SIDE EFFECTS
//  None
//=============================================================================
//
qcpParser::qcpParser(void* pUserData,
                     uint64 fsize,
                     OSCL_FILE *FilePtr){
  m_pUserData = pUserData;
  m_nFileSize = fsize;
  m_CurrentParserState = PARSER_IDLE;
  m_QCPFilePtr = FilePtr;
  m_nCurrOffset = 0;
  m_psimple_seektable = NULL;
  m_pseek = NULL;
  m_seek_function_defined = false;
  m_qcp_format = QCP_FORMAT_UNKNOWN;
  m_dataBuffer = NULL;
  m_qcp_duration = 0;
  (void) memset(&m_qcp_tech_metadata, 0, sizeof(tech_data_qcp));
  (void) memset(&m_audio_track,0,sizeof(AudioTrack));
  (void) memset(m_ReadBuffer,0,sizeof(uint8)*QCP_READ_BUFFER_SIZE);
  (void) memset(&m_qcp_header_qcph,0,sizeof(qcp_header_qcph));
  (void) memset(&m_qcp_audio_info,0,sizeof(qcp_audio_info));
  m_hOutputModeEnum = FILE_SOURCE_MEDIA_OUTPUT_BYTES_STREAM;
  m_nCurrTime = 0;
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
qcpParser::~qcpParser()
{
   if(m_psimple_seektable)
   {
      MM_Delete( m_psimple_seektable);
      m_psimple_seektable = NULL;
   }
   if(m_pseek)
   {
      MM_Delete( m_pseek);
      m_pseek = NULL;
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
//  qcpErrorType
//
// SIDE EFFECTS
//  None
//=============================================================================
//
PARSER_ERRORTYPE qcpParser::StartParsing(void){
  PARSER_ERRORTYPE retError = PARSER_ErrorNone;
  if(!m_pUserData)
  {
     MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                   "StartParsing QCP_INVALID_USER_DATA");
    retError = PARSER_ErrorDefault;
    return retError;
  }
  if(PARSER_ErrorNone !=  (retError = parse_file_header()))
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                  "Corrupt Header type QCP_PARSE_ERROR");
  }
  /**
   * In case VRAT chunk is not available, m_qcp_format will be UNKNOWN.
   * m_nCurrOffset value will be not updated properly in this case, since
   * header parsing will be failed. Instead of returning from in-between
   * the function, returns at last so all other variable update properly
   */
   //don't check return type as we can play the clip even if there is no seek function defined.
  (void)get_seek_function_defined();
  if(retError == PARSER_ErrorNone)
  {
    parse_qcp_file_header();
    parse_qcp_audio_data();
    m_nCurrOffset = m_audio_track.start;
  }
  return retError;
}

//=============================================================================
// FUNCTION: parse_qcp_file_header
//
// DESCRIPTION:
//  parse qcp file header
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

void qcpParser::parse_qcp_file_header()
{
  m_qcp_header_qcph.nChannels = 1;
  m_qcp_header_qcph.nSampleRate = m_qcp_tech_metadata.Sampling_rate;
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
//  qcpErrorType
//
// SIDE EFFECTS
//  None
//=============================================================================
//
PARSER_ERRORTYPE qcpParser::GetCurrentSample(uint8* dataBuffer,
                                             uint32 nMaxBufSize,
                                             uint32 *nBytesNeeded)
{
  if( (!dataBuffer) || (!nMaxBufSize) ||  (!nBytesNeeded) )
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
                   "GetCurrentSample QCP_INVALID_USER_DATA");
    return PARSER_ErrorInvalidParam;
  }

  uint32 nNumBytesRequest = *nBytesNeeded;
  uint32 nBytesRead = 0;
  m_dataBuffer = dataBuffer;
  if(m_nCurrOffset >= m_nFileSize)
  {
    *nBytesNeeded  = 0;
    MM_MSG_PRIO(MM_FILE_OPS,
        MM_PRIO_HIGH,
        "GetCurrentSample QCP_END_OF_FILE");
    return PARSER_ErrorEndOfFile;
  }
  if(m_hOutputModeEnum == FILE_SOURCE_MEDIA_OUTPUT_SINGLE_AUDIO_FRAME)
  {
    uint32 framesize = 0;
    uint32 frametime = 0;
    nBytesRead = QCPCallbakGetData (m_nCurrOffset, QCP_FRAME_HEADER_SIZE,
                                    m_dataBuffer, nMaxBufSize, m_pUserData );
    if(nBytesRead)
    {
      /* In some erroneous clips, frameSize calculation is not possible.
         In such scenarios, Parser will return EOF directly irrespective of current location.*/
      if(IAUDIO_SUCCESS == parse_frame_header(m_dataBuffer,&framesize,&frametime) && framesize)
      {
        m_nCurrTime+=frametime;
        nBytesRead = QCPCallbakGetData (m_nCurrOffset, framesize,
                                        m_dataBuffer, nMaxBufSize,
                                        m_pUserData );
        m_nCurrOffset += nBytesRead;
        *nBytesNeeded = nBytesRead;
      }
      else
      {
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                  "parse_frame_header failed...");
        //this will trigger EOF
        *nBytesNeeded = 0;
        return PARSER_ErrorEndOfFile;
      }
    }
  }
  else
  {
    nBytesRead = QCPCallbakGetData (m_nCurrOffset, nNumBytesRequest,
                                    m_dataBuffer, nMaxBufSize, m_pUserData );
    if(!(nBytesRead))
    {
      m_CurrentParserState = PARSER_READ_FAILED;
      *nBytesNeeded  = 0;
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                  "GetCurrentSample QCP_READ_FAILURE");
    return PARSER_ErrorReadFail;
    }
    m_nCurrOffset += nBytesRead;
    *nBytesNeeded = nBytesRead;
  }
  m_CurrentParserState = PARSER_READY;
  return PARSER_ErrorNone;

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

void qcpParser::init_file_position(){

  m_nCurrOffset = 0;
  m_nCurrOffset += m_audio_track.start;;

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
void qcpParser::set_newfile_position(uint64 file_position)
{
  m_nCurrOffset = file_position;
}

//=============================================================================
// FUNCTION: parse_qcp_audio_data
//
// DESCRIPTION:
//  parse qcp audio data information
//
// PARAMETERS
//  m_qcp_audio_info - pointer to the qcp audio info structure
//
// RETURN:
//  None
//
// SIDE EFFECTS
//  None
//=============================================================================
//

void qcpParser::parse_qcp_audio_data()
{
  m_qcp_audio_info.dwSuggestedBufferSize = QCP_MAX_BUFFER_SIZE;
}

//=============================================================================
// FUNCTION: GetQCPHeader
//
// DESCRIPTION:
//  Parse the qcp file header
//
// PARAMETERS
//  pQcpHdrPtr - pointer to qcp header structure
//
// RETURN:
//  qcpErrorType
//
// SIDE EFFECTS
//  None
//=============================================================================
//
PARSER_ERRORTYPE qcpParser::GetQCPHeader(qcp_header_qcph* pQcpHdrPtr)
{

  PARSER_ERRORTYPE retError = PARSER_ErrorNone;

  if(!pQcpHdrPtr)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                   "GetQCPHeader QCP_INVALID_USER_DATA");
    retError = PARSER_ErrorInvalidParam;
    return retError;
  }
  if(NULL == &m_qcp_header_qcph)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                  "GetQCPHeader,NULL QCP Header,QCP_PARSE_ERROR");
    retError = PARSER_ErrorDefault;
    return retError;
  }
  pQcpHdrPtr->nChannels = m_qcp_header_qcph.nChannels;
  pQcpHdrPtr->nSampleRate = m_qcp_header_qcph.nSampleRate;
  return retError;
}
//=============================================================================
// FUNCTION: get_qcp_subtype
//
// DESCRIPTION:
// This function returns qcp subformat specific information
//
// PARAMETERS
//  None
//
// RETURN:
//  qcp_boolean TRUE or FALSE
//
// SIDE EFFECTS
//  None
//=============================================================================
//

boolean qcpParser::get_qcp_subtype()
{
  return m_qcp_tech_metadata.SubType;
}




//=============================================================================
// FUNCTION: GetQCPDecodeInfo
//
// DESCRIPTION:
//  Parse the qcp decode information
//
// PARAMETERS
//  pQCPDecodeinfo - pointer to qcp decode info structure
//
// RETURN:
//  qcpErrorType
//
// SIDE EFFECTS
//  None
//=============================================================================
//
PARSER_ERRORTYPE qcpParser::GetQCPDecodeInfo(qcp_decode_info* pQCPDecodeinfo)
{
  pQCPDecodeinfo->codec_version = m_qcp_tech_metadata.Codec_version;
  switch(m_qcp_tech_metadata.Format_type)
  {
    case QCP_FULLRATE_FIX:
    case QCP_FULLRATE_VAR:
    {
      pQCPDecodeinfo->cdma_rate = 1;
      break;
    }
    case QCP_HALFRATE_FIX:
    case QCP_HALFRATE_VAR:
    {
      pQCPDecodeinfo->cdma_rate = 2;
      break;
    }
    default:
    {
      pQCPDecodeinfo->cdma_rate = 0;
      //RETAILMSG (ZONE_ERROR, (TEXT(
                 //"CDSAudioParserAdaptor::getformatmediatype: QCPFORMAT : cdma rate not handled\r\n")));
    }
  }
  return PARSER_ErrorNone;
}

//=============================================================================
// FUNCTION: GetAudioInfo
//
// DESCRIPTION:
// This function returns audio format specific information
//
// PARAMETERS
//  pAudioInfo - pointer to the qcp_audio_info structure
//
// RETURN:
//  qcpErrorType
//
// SIDE EFFECTS
//  None
//=============================================================================
//
PARSER_ERRORTYPE qcpParser::GetAudioInfo(qcp_audio_info* pAudioInfo){

  PARSER_ERRORTYPE retError = PARSER_ErrorNone;

  if(NULL == &m_qcp_audio_info)
  {
    retError = PARSER_ErrorDefault;
    return retError;
  }
  if(NULL == pAudioInfo)
  {
    retError = PARSER_ErrorInvalidParam;
    return retError;
  }
  (void) memset(pAudioInfo,0,sizeof(qcp_audio_info));
  (void) memcpy(pAudioInfo,&m_qcp_audio_info,sizeof(qcp_audio_info));
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
uint64 qcpParser::GetClipDurationInMsec()
{

  uint32 noEntries = 50;

  uint64 uDuration = 0;
  if (m_qcp_duration)
  {
    uDuration = m_qcp_duration;
  }
  else if(!m_seek_function_defined)
  {
    if(m_nFileSize > MAX_QCELP_FILE_SIZE)
    {
      (void)GetApproxDuration(&uDuration);
      m_qcp_duration = uDuration;
    }
    else
    {
      if(m_psimple_seektable == NULL)
      {
        m_psimple_seektable = MM_New_Args(simple_seektable,(noEntries));
        (void)OSCL_FileSeek(m_QCPFilePtr, m_audio_track.start, SEEK_CUR);
      }

      if(m_pseek == NULL && m_psimple_seektable != NULL)
      {
        m_pseek = MM_New_Args(seek,(m_psimple_seektable
                           ,this
                           ,m_QCPFilePtr
                           ,QCP_FRAME_HEADER_SIZE
                           ,m_audio_track.start
                           ,m_audio_track.end));
      }

      if( NULL != m_pseek)
      {
        m_qcp_duration = uDuration = m_pseek->get_duration();
        m_pseek->set_mode(PREGENERATE_TABLE);
      }
    }
  }
  else
  {
    uint64 fileduration = 0;
    if(PARSER_ErrorNone != calculate_get_duration (&fileduration))
    {
      return uDuration;
    }
    m_qcp_duration = uDuration = fileduration;
  }
  return uDuration;
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
//  QCP_SUCCESS Function succesfully calculated the playback duration
//  AEE_UNSUPPORTED: Function is not implemented for this audio format
//  AEE_EBADSTATE: Data not available to perform the calculation
//
// SIDE EFFECTS
//  None
//=============================================================================
//
PARSER_ERRORTYPE qcpParser::calculate_get_duration(uint64* puDuration)
{
  PARSER_ERRORTYPE result = PARSER_ErrorDefault;
  //! If packet count and file format are known, then calc duration
  if (QCP_FORMAT_UNKNOWN != m_qcp_format && m_qcp_tech_metadata.size_in_packet)
  {
    result = PARSER_ErrorNone;
    // *time  = (Total Number of packets*20e-03)  > Irrespective of CBR/VBR
    // as each packet is of duration 20ms
    *puDuration = (int64)((int64)(m_qcp_tech_metadata.size_in_packet) *
                          (int64)QCP_FRAME_DURATION);
  }
  return result;
}

//=============================================================================
// FUNCTION : read_riff_chunk
//
// DESCRIPTION
//  verifies RIFF chunk ina  qcp file
//
// PARAMETERS
//
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
PARSER_ERRORTYPE qcpParser::read_riff_chunk(uint32 *chunk_size)
{
  PARSER_ERRORTYPE result = PARSER_ErrorNone;

  uint8 buffer[QCP_RIFF_HEADER_SIZE] = {0};
  int buf_len = QCP_RIFF_HEADER_SIZE;
  uint32 offset = 0;
  uint32 bytes_read = 0;

  if(PARSER_ErrorNone != OSCL_FileSeek(m_QCPFilePtr,offset,SEEK_SET))
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                   "qcpformatparser::read_riff_chunk: file seek failed");
    result = PARSER_ErrorReadFail;
    return result;
  }

  bytes_read = OSCL_FileRead(buffer,buf_len,1,m_QCPFilePtr);
  if( !(bytes_read))
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                   "qcpformatparser::read_riff_chunk : file read failed");
    result = PARSER_ErrorReadFail;
    return result;
  }

  if (bytes_read != QCP_RIFF_HEADER_SIZE)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                   "qcpformatparser::read_riff_chunk:could not read from File!");
    result = PARSER_ErrorReadFail;
    return result;
  }

  //check for "RIFF" ChunkID
  if (strncmp ((char *)buffer, "RIFF", QCP_FIELD_OFFSET)) {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                     "qcpformatparser::read_riff_chunk: no 'RIFF' ID not wave chunk");
    result = PARSER_ErrorStreamCorrupt;
    return result;
  }

  offset += QCP_FIELD_OFFSET;
  // Read RIFF chunk size.  If it indicates RIFF is smaller than the
  // actual file size, overwrite the file size.
  uint32 riff_size;
  (void) memmove(&riff_size, buffer + offset, QCP_FIELD_OFFSET);
  riff_size = little_endian (riff_size);

  offset += QCP_FIELD_OFFSET;

  if (strncmp ((char *)buffer + offset, "QLCM", QCP_FIELD_OFFSET))
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                   "qcpformatparser::read_riff_chunk: no 'QLCM'tag,not qcp chunk");
    result = PARSER_ErrorStreamCorrupt;
    return result;
  }

  *chunk_size = riff_size;
  return result;
}

//=============================================================================
// FUNCTION : read_fmt_chunk
//
// DESCRIPTION
//  reads "fmt" chunk in a .qcn file and retrieves meta data
//
// PARAMETERS
//  file : IFilePort interface to data to be parsed.
//  chunk_size : size of the fmt chunk
//
// RETURN VALUE
//  An indicator of success or failure of the method is returned.
//
// SIDE EFFECTS
//  None
//
//=============================================================================
//
PARSER_ERRORTYPE qcpParser::read_fmt_chunk(uint32 *pulChunkSize)
{

  PARSER_ERRORTYPE result = PARSER_ErrorNone;
  uint32 ulBytesRead = 0;
  uint32 loop = 0;
  uint8 EVRC_FLAG = 0;    // Flag for EVRC
  uint8 QCELP_FLAG = 0;  // Flag for QCELP
  uint8 EVRC_ID[8]  = {0x91,0xEF,0x73,0x6A,0x51,0x00,0xCE,0xB4};
  uint8 QCELP_ID[8] = {0xBA,0x91,0x00,0x80,0x5F,0xB4,0xB9,0x7E};

  if(PARSER_ErrorNone != OSCL_FileSeek(m_QCPFilePtr,QCP_RIFF_HEADER_SIZE,SEEK_SET))
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                     "qcpformatparser::read_fmt_chunk: file seek failed");
    result = PARSER_ErrorReadFail;
    return result;
  }

  uint8 ucHeaderBuffer[QCP_FMT_HEADER_SIZE] = {0};
  uint32 ulBuf_len = QCP_FMT_HEADER_SIZE;
  uint32 ulOffset = 0;

  ulBytesRead = OSCL_FileRead(ucHeaderBuffer,ulBuf_len,1,m_QCPFilePtr);
  if(!(ulBytesRead))
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                   "qcpformatparser::read_fmt_chunk : file read failed");
    result = PARSER_ErrorReadFail;
    return result;
  }

  if (ulBytesRead != QCP_FMT_HEADER_SIZE)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                   "qcpformatparser::read_fmt_chunk: could not read from File!");
    result = PARSER_ErrorReadFail;
    return result;
  }

  //Reading "fmt" chunk
  //check for "fmt" ChunkID
  if (strncmp ((char*)ucHeaderBuffer, "fmt ", QCP_FIELD_OFFSET))
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                     "qcpformatparser::read_fmt_chunk: no 'fmt ' ID");
    result = PARSER_ErrorStreamCorrupt;
    return result;
  }
  //DBG(LOW,vastr("FMT Chunk is = 0x%x%x%x%x",ucHeaderBuffer[0],ucHeaderBuffer[1],ucHeaderBuffer[2],ucHeaderBuffer[3]));

  ulOffset += QCP_FIELD_OFFSET;

  // Read "fmt" chunk size.
  uint32 ulFmt_size;
  uint32 ulLoopCount = 0;
  (void) memmove(&ulFmt_size, ucHeaderBuffer + ulOffset, QCP_FIELD_OFFSET);
  ulFmt_size = little_endian (ulFmt_size);
  //DBG(LOW,vastr("FMT Chunk size is = 0x%x",fmt_size));
  uint8* pucBuffer = NULL;
  if (ulFmt_size > m_nFileSize)
  {
    // Error condition, RIFF size exceeds file size, something
    // is wrong here. Mostly the file is corrupt.
    result = PARSER_ErrorInHeaderParsing;
    return result;
  }
  // Allocate memory for reading data from IFilePort
  pucBuffer = (uint8 *) MM_Malloc(ulFmt_size * sizeof(uint8));
  if (NULL == pucBuffer)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                     "qcpformatparser::read_fmt_chunk: not enough data available!");
    result = PARSER_ErrorMemAllocFail;
    return result;
  }
  //seek and read actual "fmt" contents
  if(PARSER_ErrorNone != OSCL_FileSeek(m_QCPFilePtr,QCP_RIFF_HEADER_SIZE + QCP_CHUNK_HEADER_SIZE,SEEK_SET))
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                   "qcpformatparser::read_fmt_chunk: file seek failed");
    result = PARSER_ErrorReadFail;
    MM_Free(pucBuffer);
    pucBuffer = NULL;
    return result;
  }

  ulBuf_len = ulFmt_size;

  ulBytesRead = OSCL_FileRead(pucBuffer,ulBuf_len,1,m_QCPFilePtr);
  if(!(ulBytesRead))
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                   "qcpformatparser::read_fmt_chunk : file read failed");
    result = PARSER_ErrorReadFail;
    MM_Free(pucBuffer);
    pucBuffer = NULL;
    return result;
  }

  if (ulBytesRead != ulBuf_len)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                     "qcpformatparser::read_fmt_chunk:could not read from File !");
    result = PARSER_ErrorReadFail;
    MM_Free(pucBuffer);
    pucBuffer = NULL;
    return result;
  }


  ulOffset -= 4;     // Now ulOffset = 8 Byte & pointing towards 'major' uint8

  m_qcp_tech_metadata.type = AUDIO_QCP;
  m_qcp_tech_metadata.major = (uint8)(*(pucBuffer + ulOffset));  // read major flag (1 Byte)
  ulOffset += 1;
  //DBG(LOW,vastr("major flag is  = 0x%x",m_qcp_tech_metadata.major));

  m_qcp_tech_metadata.minor = (uint8)*(pucBuffer + ulOffset);  // read minor flag (1 Byte)
  ulOffset += 1;  // ulOffset now pointing to "Codec_guid_codecID1"
  //DBG(LOW,vastr("minor flag is  = 0x%x",m_qcp_tech_metadata.minor));

  memcpy(&m_qcp_tech_metadata.Codec_guid_codecID1,pucBuffer+ulOffset,(sizeof(uint32)));
  m_qcp_tech_metadata.Codec_guid_codecID1 = little_endian (m_qcp_tech_metadata.Codec_guid_codecID1);
  //DBG(LOW,vastr("Codec_guid_codecID1 = 0x%x",m_qcp_tech_metadata.Codec_guid_codecID1));

  if(m_qcp_tech_metadata.Codec_guid_codecID1 == 0xE689D48D)
  {
    EVRC_FLAG = 1;
  }
  else if((m_qcp_tech_metadata.Codec_guid_codecID1 == 0x5E7F6D41) || (m_qcp_tech_metadata.Codec_guid_codecID1 == 0x5E7F6D42))
  {
    QCELP_FLAG = 1;
  }
  ulOffset += QCP_FIELD_OFFSET; // ulOffset now pointing to "Codec_guid_codecID2"
  memcpy(&m_qcp_tech_metadata.Codec_guid_codecID2,pucBuffer+ulOffset,(sizeof(uint16)));
  m_qcp_tech_metadata.Codec_guid_codecID2 = little_endian (m_qcp_tech_metadata.Codec_guid_codecID2);
  ulOffset += 2;  //AS Codec_guid_codecID2 SIZE is 2 Bytes
  //DBG(LOW,vastr("Codec_guid_codecID2 = 0x%x",m_qcp_tech_metadata.Codec_guid_codecID2));

  if(m_qcp_tech_metadata.Codec_guid_codecID2 == 0x9076)
  {
    EVRC_FLAG = (EVRC_FLAG & 1);
  }
  else if (m_qcp_tech_metadata.Codec_guid_codecID2 == 0xB115)
  {
    QCELP_FLAG = (QCELP_FLAG & 1);
  }
  else
  {
    EVRC_FLAG  = (EVRC_FLAG & 0);
    QCELP_FLAG = (QCELP_FLAG & 0);
  }
  memcpy(&m_qcp_tech_metadata.Codec_guid_codecID3,pucBuffer+ulOffset,(sizeof(uint16)));
  m_qcp_tech_metadata.Codec_guid_codecID3 = little_endian (m_qcp_tech_metadata.Codec_guid_codecID3);

  ulOffset += 2;  //AS Codec_guid_codecID3 SIZE is 2 Bytes

  if(m_qcp_tech_metadata.Codec_guid_codecID3 == 0x46B5)
  {
    EVRC_FLAG = (EVRC_FLAG & 1);
  }
  else if (m_qcp_tech_metadata.Codec_guid_codecID3 ==0x11D0)
  {
    QCELP_FLAG = (QCELP_FLAG & 1);
  }
  else
  {
    EVRC_FLAG = (EVRC_FLAG & 0);
    QCELP_FLAG = (QCELP_FLAG & 0);
  }
  //DBG(LOW,vastr("Codec_guid_codecID3 = 0x%x",m_qcp_tech_metadata.Codec_guid_codecID3));

  for(loop = 0; loop < 8; loop++)
  {
    m_qcp_tech_metadata.Codec_guid_codecID4[loop] = (uint8)*(pucBuffer + ulOffset);  // read major flag (1 Byte)
    if(m_qcp_tech_metadata.Codec_guid_codecID4[loop] == EVRC_ID[loop])
    {
      EVRC_FLAG = (EVRC_FLAG & 1);
    }
    else if (m_qcp_tech_metadata.Codec_guid_codecID4[loop] ==QCELP_ID[loop])
    {
      QCELP_FLAG = (QCELP_FLAG & 1);
    }
    else
    {
      EVRC_FLAG = (EVRC_FLAG & 0);
      QCELP_FLAG = (QCELP_FLAG & 0);
    }
    ulOffset += 1;
  }
  //DBG(LOW,vastr("EVRC_FLAG  = 0x%x",EVRC_FLAG));
  //DBG(LOW,vastr("QCELP_FLAG = 0x%x",QCELP_FLAG));
  if(((EVRC_FLAG == 1) ||(QCELP_FLAG == 1)) && (m_qcp_tech_metadata.major == 1) && (m_qcp_tech_metadata.minor == 0))
  {
    if (EVRC_FLAG == 1)
    {
      m_qcp_tech_metadata.SubType = TRUE;     // Confirming a Valid EVRCfile
    }
    else if (QCELP_FLAG == 1)
         {
           m_qcp_tech_metadata.SubType = FALSE;     // Confirming a Valid QCELP file
         }
    }
  else
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                     "qcpformatparser::read_fmt_chunk: This is Not a Valid QCP File");
    result = PARSER_ErrorUnsupportedCodecType;
    MM_Free(pucBuffer);
    pucBuffer = NULL;
    return result;
  }
  memcpy(&m_qcp_tech_metadata.Codec_version,pucBuffer+ulOffset,(sizeof(uint16)));
  m_qcp_tech_metadata.Codec_version = little_endian (m_qcp_tech_metadata.Codec_version);

  ulOffset += 2;  //AS Codec_version SIZE is 2 Bytes
  //DBG(LOW,vastr("codec version is  = 0x%x",m_qcp_tech_metadata.Codec_version));
  //DBG(LOW,vastr("codec name is  = %s",(pucBuffer+ulOffset)));
  ulOffset += 80;      //As codec name is 80 Bytes


  memcpy(&m_qcp_tech_metadata.Average_bps,pucBuffer+ulOffset,(sizeof(uint16)));
  m_qcp_tech_metadata.Average_bps = little_endian (m_qcp_tech_metadata.Average_bps);

  ulOffset += 2;  //AS Average_bps SIZE is 2 Bytes
  //DBG(LOW,vastr("Average bit rate is  = %d",m_qcp_tech_metadata.Average_bps));

  memcpy(&m_qcp_tech_metadata.Packet_size,pucBuffer+ulOffset,(sizeof(uint16)));
  m_qcp_tech_metadata.Packet_size = little_endian (m_qcp_tech_metadata.Packet_size);
  ulOffset += 2;  //AS Packet_size SIZE is 2 Bytes
  //DBG(LOW,vastr("Maximum Packet size  is  = %d",m_qcp_tech_metadata.Packet_size));

  memcpy(&m_qcp_tech_metadata.Block_size,pucBuffer+ulOffset,(sizeof(uint16)));
  m_qcp_tech_metadata.Block_size = little_endian (m_qcp_tech_metadata.Block_size);
  ulOffset += 2;  //AS Block_size SIZE is 2 Bytes
  //DBG(LOW,vastr("Block_size  is  = %d",m_qcp_tech_metadata.Block_size));

  memcpy(&m_qcp_tech_metadata.Sampling_rate,pucBuffer+ulOffset,(sizeof(uint16)));
  m_qcp_tech_metadata.Sampling_rate = little_endian (m_qcp_tech_metadata.Sampling_rate);
  ulOffset += 2;  //AS Sampling_rate SIZE is 2 Bytes
  //DBG(LOW,vastr("Sampling Rate  is  = %d",m_qcp_tech_metadata.Sampling_rate));

  memcpy(&m_qcp_tech_metadata.Sample_size,pucBuffer+ulOffset,(sizeof(uint16)));
  m_qcp_tech_metadata.Sample_size = little_endian (m_qcp_tech_metadata.Sample_size);
  ulOffset += 2;  //AS Sample_size SIZE is 2 Bytes
  //DBG(LOW,vastr("Sampling Size  is  = %d",m_qcp_tech_metadata.Sample_size));

  memcpy(&m_qcp_tech_metadata.Num_rates,pucBuffer+ulOffset,(sizeof(uint32)));
  m_qcp_tech_metadata.Num_rates = little_endian (m_qcp_tech_metadata.Num_rates);
  ulOffset += 4;  //AS Num_rates SIZE is 4 Bytes
  //DBG(LOW,vastr("Number of possible rate octet  is  = %d",m_qcp_tech_metadata.Num_rates));

  //DBG(LOW,vastr("Rate-map-table   is  = "));
  for(ulLoopCount = 0;ulLoopCount < 16; ulLoopCount++)
  {
    m_qcp_tech_metadata.rate_map_entry[ulLoopCount] = pucBuffer[ulOffset + ulLoopCount];
    //DBG(LOW,vastr("%x",m_qcp_tech_metadata.rate_map_entry[ulLoopCount]));
  }

  ulOffset += 16;
  ulOffset += 4*5;
  *pulChunkSize = QCP_CHUNK_HEADER_SIZE + ulFmt_size;
  //DBG(LOW,vastr("Total Size of FMT chunk including header  is  = %d",*pulChunkSize));
  MM_Free(pucBuffer);
  pucBuffer = NULL;
  return result;
}

//=============================================================================
// FUNCTION : read_vrat_chunk
//
// DESCRIPTION
//  reads "fact" chunk in a .qcp file
//
// PARAMETERS
//  file : IFilePort interface to data to be parsed.
//  offset : offset of fact chunk from start of file
//  chunk_size : size of fact chunk
//
// RETURN VALUE
//  An indicator of success or failure of the method is returned.
//
// SIDE EFFECTS
//  None
//
//=============================================================================
//

PARSER_ERRORTYPE qcpParser::read_vrat_chunk( uint32 *offset, uint32 *chunk_size )
{
  PARSER_ERRORTYPE result = PARSER_ErrorNone;
  uint32 bytes_read = 0;
  int seek_pos = (int ) *offset;
  bool b_vrat_exist = true;

  if(PARSER_ErrorNone != OSCL_FileSeek(m_QCPFilePtr,seek_pos,SEEK_SET))
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                     "qcpformatparser::read_vrat_chunk: file seek failed");
    result = PARSER_ErrorReadFail;
    return result;
  }
  uint8 header_buffer[QCP_VRAT_HEADER_SIZE] = {0};
  int buf_len = QCP_VRAT_HEADER_SIZE;
  uint32 local_offset = 0;

  bytes_read = OSCL_FileRead(header_buffer,buf_len,1,m_QCPFilePtr);
  if(!(bytes_read))
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                   "qcpformatparser::read_vrat_chunk : file read failed");
    result = PARSER_ErrorReadFail;
    return result;
  }

   //Reading "vrat" chunk
   //check for "vrat" ChunkID
  if (strncmp ((char*)header_buffer, "vrat", QCP_FIELD_OFFSET))
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"NO VRAT chunk detected...");
    *chunk_size = 0;
    b_vrat_exist = false;
  }
  else if (bytes_read != QCP_VRAT_HEADER_SIZE)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
      "qcpformatparser::read_vrat_chunk: could not read from IFileport1!");
    result = PARSER_ErrorReadFail;
    return result;
  }
  uint32 temp_var = 0;
  uint32 vrat_size = 0;

  if(b_vrat_exist)
  {
    local_offset += QCP_FIELD_OFFSET;
    // Read "vrat" chunk size.
    (void) memmove(&vrat_size, header_buffer + local_offset, QCP_FIELD_OFFSET);
    vrat_size = little_endian (vrat_size);
    local_offset += QCP_FIELD_OFFSET;

    (void) memmove(&temp_var, header_buffer + local_offset, QCP_FIELD_OFFSET);
    m_qcp_tech_metadata.var_rate_flag = little_endian (temp_var);
    local_offset += QCP_FIELD_OFFSET;
    if(m_qcp_tech_metadata.var_rate_flag == 0)
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
                   "qcpformatparser::read_vrat_chunk: It is a Fixed Rate File!");
    }
    else if ((m_qcp_tech_metadata.var_rate_flag > 0) && (m_qcp_tech_metadata.var_rate_flag < 0xFFFF0000))
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
                   "qcpformatparser::read_vrat_chunk: It is a Variable Rate File!");
    }
    else if (m_qcp_tech_metadata.var_rate_flag >= 0xFFFF0001)
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                   "qcpformatparser::read_vrat_chunk: Var_Rate_Flag is not defined !");
      return PARSER_ErrorStreamCorrupt;
    }
  }//if(b_vrat_exist)

  // Setting qcp_format_type Flag here
  if (m_qcp_tech_metadata.type == AUDIO_QCP)   // varifying valid QCP file
  {
    // Check full rate
    if((m_qcp_tech_metadata.Average_bps == 13000) || (m_qcp_tech_metadata.Average_bps == 14399))
    {
      if(m_qcp_tech_metadata.var_rate_flag == 1)
      {
        m_qcp_format = (qcp_format_type) 3;// VBR FULL RATE
      }
      else
      {
        m_qcp_format = (qcp_format_type) 1;// CBR FULL RATE
      }
    }
    else  if((m_qcp_tech_metadata.Average_bps == 6800) || (m_qcp_tech_metadata.Average_bps == 7199))    // Half Rate
    {
      if(m_qcp_tech_metadata.var_rate_flag == 1)
      {
        m_qcp_format = (qcp_format_type) 4;// VBR HALF RATE
      }
      else
      {
        m_qcp_format = (qcp_format_type) 2;// CBR HALF RATE
      }
    }
    else
    {
      m_qcp_format = (qcp_format_type) 6;   // valid QCP file with some other bit rate other than 13k,6.8kbps
    }
  }
  else
  {
    if(m_qcp_tech_metadata.major == 2)
    {
      m_qcp_format = (qcp_format_type) 5;   // SMV
    }
    else
    {
      m_qcp_format = (qcp_format_type) 0;   // QCP_FORMAT_UNKNOWN
    }
  }
  m_qcp_tech_metadata.Format_type = m_qcp_format;
  if(b_vrat_exist)
  {
    (void) memmove(&temp_var, header_buffer + local_offset, QCP_FIELD_OFFSET);
    m_qcp_tech_metadata.size_in_packet  = little_endian (temp_var);
    local_offset += QCP_FIELD_OFFSET;
    //DBG(LOW,vastr("Total number of packets in data chunk are : %d",m_qcp_tech_metadata.size_in_packet));
    *chunk_size = QCP_CHUNK_HEADER_SIZE + vrat_size;
    //DBG(LOW,vastr("Total Size of vrat chunk including header  is  = %d",*chunk_size));
  }
  return result;
}


//=============================================================================
// FUNCTION : read_labl_chunk
//
// DESCRIPTION
//  reads "labl" chunk in a .qcp file
//
// PARAMETERS
//  file : IFilePort interface to data to be parsed.
//  offset : offset of fact chunk from start of file
//  chunk_size : size of fact chunk
//
// RETURN VALUE
//  An indicator of success or failure of the method is returned.
//
// SIDE EFFECTS
//  None
//
//=============================================================================
//
PARSER_ERRORTYPE qcpParser::read_labl_chunk( uint32 *offset, uint32 *chunk_size )
{
  PARSER_ERRORTYPE result = PARSER_ErrorNone;
  uint32 bytes_read = 0;
  int seek_pos = (int) *offset;

  if(PARSER_ErrorNone != OSCL_FileSeek(m_QCPFilePtr,seek_pos,SEEK_SET))
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                     "qcpformatparser::read_labl_chunk: file seek failed");
    result = PARSER_ErrorReadFail;
    return result;
  }

  uint8 header_buffer[QCP_LABL_HEADER_SIZE] = {0};
  int buf_len = QCP_LABL_HEADER_SIZE;
  uint32 local_offset = 0;

  bytes_read = OSCL_FileRead(header_buffer,buf_len,1,m_QCPFilePtr);
  if(!(bytes_read))
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                     "qcpformatparser::read_labl_chunk : file read failed");
    result = PARSER_ErrorReadFail;
    return result;
  }

   //check for "labl" ChunkID
  if (strncmp ((char*)header_buffer, "labl", QCP_FIELD_OFFSET))
  {
    //DBG(LOW,"qcpformatparser::read_qcp_chunk: no 'labl' chunk");
    *chunk_size = 0;
    result = PARSER_ErrorNone;
    return result;
  }
  else if (bytes_read != QCP_LABL_HEADER_SIZE)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
      "qcpformatparser::read_labl_chunk: could not read from File!");
    result = PARSER_ErrorReadFail;
    return result;
  }

  local_offset += QCP_FIELD_OFFSET;

  // Read "labl" chunk size.
  uint32 labl_size;
  (void) memmove(&labl_size, header_buffer + local_offset, QCP_FIELD_OFFSET);
  labl_size = little_endian (labl_size);

  *chunk_size = QCP_CHUNK_HEADER_SIZE + labl_size;
  uint32 loop = 0;
  for(loop=0;loop<48;loop++)
  {
    m_qcp_tech_metadata.label[loop] = (uint8)*(header_buffer + local_offset);  // read label (1 Byte)
    offset += 1;
  }
  return result;
}

//=============================================================================
// FUNCTION : read_offs_chunk
//
// DESCRIPTION
//  reads "offs" chunk in a .qcp file
//
// PARAMETERS
//  file : IFilePort interface to data to be parsed.
//  offset : offset of fact chunk from start of file
//  chunk_size : size of fact chunk
//
// RETURN VALUE
//  An indicator of success or failure of the method is returned.
//
// SIDE EFFECTS
//  None
//
//=============================================================================
//
PARSER_ERRORTYPE qcpParser::read_offs_chunk(uint32 *offset, uint32 *chunk_size)
{
  PARSER_ERRORTYPE result = PARSER_ErrorNone;
  uint32 bytes_read = 0;
  int seek_pos = (int) *offset;
  if(PARSER_ErrorNone != OSCL_FileSeek(m_QCPFilePtr,seek_pos,SEEK_SET))
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                   "qcpformatparser::read_offs_chunk: file seek failed");
    result = PARSER_ErrorReadFail;
    return result;
  }

  uint8 header_buffer[QCP_OFFS_HEADER_SIZE] = {0};
  int buf_len = QCP_OFFS_HEADER_SIZE;
  uint32 local_offset = 0;

  bytes_read = OSCL_FileRead(header_buffer,buf_len,1,m_QCPFilePtr);
  if(!(bytes_read))
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                   "qcpformatparser::read_offs_chunk : file read failed");
    result = PARSER_ErrorReadFail;
    return result;
  }

  //check for "offs" ChunkID
  if (strncmp ((char*)header_buffer, "offs", QCP_FIELD_OFFSET))
  {
    //DBG(LOW,"qcpformatparser::read_qcp_chunk: no 'offs' chunk");
    //std_memmove(m_qcp_tech_metadata.label, header_buffer + 8, (QCP_OFFS_HEADER_SIZE-8)); // Filling zero in 'label' field in metadata
    result = PARSER_ErrorNone;
    *chunk_size = 0;
    return result;
  }
  else if (bytes_read != QCP_OFFS_HEADER_SIZE)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
      "qcpformatparser::read_offs_chunk: could not read from IFileport1!");
    result = PARSER_ErrorReadFail;
    return result;
  }

  local_offset += QCP_FIELD_OFFSET;

  // Read "offs" chunk size.
  uint32 temp_var;
  (void) memmove(&temp_var, header_buffer + local_offset, QCP_FIELD_OFFSET);
  temp_var = little_endian (temp_var);

  *chunk_size = QCP_CHUNK_HEADER_SIZE + temp_var;
  //DBG_RETURN(result);
  //return result;
  local_offset += QCP_FIELD_OFFSET;

  (void) memmove(&temp_var, header_buffer + local_offset, QCP_FIELD_OFFSET);
  m_qcp_tech_metadata.offs_step_size  = little_endian (temp_var);
  local_offset += QCP_FIELD_OFFSET;

  (void) memmove(&temp_var, header_buffer + local_offset, QCP_FIELD_OFFSET);
  m_qcp_tech_metadata.offs_num_offset  = little_endian (temp_var);
  local_offset += QCP_FIELD_OFFSET;

  (void) memmove(&temp_var, header_buffer + local_offset, QCP_FIELD_OFFSET);
  m_qcp_tech_metadata.offs_offset  = little_endian (temp_var);
  local_offset += QCP_FIELD_OFFSET;
  return result;
}

//=============================================================================
// FUNCTION : read_data_chunk
//
// DESCRIPTION
//  reads "data" chunk in a wave file and gets the data chunk size
//
// PARAMETERS
//  file : IFilePort interface to data to be parsed.
//  offset : offset of fact chunk from start of file
//  chunk_size : size of fact chunk
//
// RETURN VALUE
//  An indicator of success or failure of the method is returned.
//
// SIDE EFFECTS
//  None
//
//=============================================================================
//
PARSER_ERRORTYPE qcpParser::read_data_chunk( uint32 *offset, uint32 *chunk_size )
{
  PARSER_ERRORTYPE result = PARSER_ErrorNone;

  uint32 bytes_read = 0;
  int seek_pos = (int) *offset;

  if(PARSER_ErrorNone != OSCL_FileSeek(m_QCPFilePtr,seek_pos,SEEK_SET))
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                   "qcpformatparser::read_data_chunk: file seek failed");
    result = PARSER_ErrorReadFail;
    return result;
  }

  uint8 header_buffer[QCP_CHUNK_HEADER_SIZE] = {0};
  int buf_len = QCP_CHUNK_HEADER_SIZE;
  uint32 local_offset = 0;

  bytes_read = OSCL_FileRead(header_buffer,buf_len,1,m_QCPFilePtr);
  if(!(bytes_read))
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                   "qcpformatparser::read_data_chunk : file read failed");
    result = PARSER_ErrorReadFail;
    return result;
  }

  if (bytes_read != QCP_CHUNK_HEADER_SIZE)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                "qcpformatparser::read_data_chunk:could not read from File!");
    result = PARSER_ErrorReadFail;
    return result;
  }

  //Reading "data" chunk
  //check for "data" ChunkID
  if (strncmp ((char*)header_buffer, "data", QCP_FIELD_OFFSET))
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                "qcpformatparser::read_data_chunk: no 'data' chunk");
    result = PARSER_ErrorUnsupportedCodecType;
    return result;
  }

  local_offset += QCP_FIELD_OFFSET;

  // Read "data" chunk size.
  uint32 data_size;
  (void) memmove(&data_size, header_buffer + local_offset, QCP_FIELD_OFFSET);
  data_size = little_endian (data_size);
  m_qcp_tech_metadata.data_chunk_length = data_size;
  *chunk_size = QCP_CHUNK_HEADER_SIZE + data_size;
  //DBG(LOW,vastr("Total Size of DATA chunk   is  = %d",data_size));
  return result;
}

//=============================================================================
// FUNCTION : read_cnfg_chunk
//
// DESCRIPTION
//  reads Optional "[cnfg]" chunk in a qcp file and gets the cnfg chunk size
//
// PARAMETERS
//  file : IFilePort interface to data to be parsed.
//  offset : offset of cnfg chunk from start of file
//  chunk_size : size of cnfg chunk
//
// RETURN VALUE
//  An indicator of success or failure of the method is returned.
//
// SIDE EFFECTS
//  None
//
//=============================================================================
//
PARSER_ERRORTYPE qcpParser::read_cnfg_chunk( uint32 *offset, uint32 *chunk_size )
{
  PARSER_ERRORTYPE result = PARSER_ErrorNone;

  uint32 bytes_read = 0;
  int seek_pos = (int) *offset;


  if (m_nFileSize <= (uint64) seek_pos)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
                   "qcpformatparser::read_cnfg_chunk: Already End of the file");
    result = PARSER_ErrorNone;
    return result;
  }
  if(PARSER_ErrorNone != OSCL_FileSeek(m_QCPFilePtr,seek_pos,SEEK_SET))
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                   "qcpformatparser::read_cnfg_chunk: file seek failed");
    result = PARSER_ErrorReadFail;
    return result;
  }

  uint8 header_buffer[QCP_CHUNK_HEADER_SIZE] = {0};
  int buf_len = QCP_CHUNK_HEADER_SIZE;
  uint32 local_offset = 0;

  bytes_read = OSCL_FileRead(header_buffer,buf_len,1,m_QCPFilePtr);

  if(!(bytes_read))
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                   "qcpformatparser::read_cnfg_chunk : file read failed");
    result = PARSER_ErrorReadFail;
    return result;
  }
  if (bytes_read != QCP_CHUNK_HEADER_SIZE)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                   "qcpformatparser::read_cnfg_chunk:could not read from IFileport1!");
    result = PARSER_ErrorNone;
    return result;
  }

  //Reading "cnfg" chunk
  //check for "cnfg" ChunkID
  if (strncmp ((char*)header_buffer, "cnfg", QCP_FIELD_OFFSET))
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                   "qcpformatparser::read_cnfg_chunk: 'cnfg' chunk not found");
    result = PARSER_ErrorNone;
    return result;
  }

  local_offset += QCP_FIELD_OFFSET;

  // Read "cnfg" chunk size.
  uint32 data_size;
  (void) memmove(&data_size, header_buffer + local_offset, QCP_FIELD_OFFSET);
  data_size = little_endian (data_size);

  // Read "cnfg" config-word.
  uint16 config_word = 0;
  (void) memmove(&config_word, header_buffer + local_offset, (int) data_size);
  config_word = little_endian (config_word);
  m_qcp_tech_metadata.config_word = config_word;
  *chunk_size = QCP_CHUNK_HEADER_SIZE + data_size;
  return result;
}

//=============================================================================
// FUNCTION : read_text_chunk
//
// DESCRIPTION
//  reads Optional "[text]" chunk in a qcp file and gets the text chunk size
//
// PARAMETERS
//  file : IFilePort interface to data to be parsed.
//  offset : offset of text chunk from start of file
//  chunk_size : size of text chunk
//
// RETURN VALUE
//  An indicator of success or failure of the method is returned.
//
// SIDE EFFECTS
//  None
//
//=============================================================================
//
PARSER_ERRORTYPE qcpParser::read_text_chunk( uint32 *offset, uint32 *chunk_size )
{
  PARSER_ERRORTYPE result = PARSER_ErrorNone;
  uint32 bytes_read = 0;
  int seek_pos = (int) *offset;

  if (m_nFileSize <= (uint64) seek_pos)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
                   "qcpformatparser::read_text_chunk: Already End of the file");
    result = PARSER_ErrorNone;
    return result;
  }

  if(PARSER_ErrorNone != OSCL_FileSeek(m_QCPFilePtr,seek_pos,SEEK_SET))
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                   "qcpformatparser::read_text_chunk: file seek failed");
    result = PARSER_ErrorReadFail;
    return result;
  }

  uint8 header_buffer[QCP_CHUNK_HEADER_SIZE] = {0};
  int buf_len = QCP_CHUNK_HEADER_SIZE;
  uint32 local_offset = 0;

  bytes_read = OSCL_FileRead(header_buffer,buf_len,1,m_QCPFilePtr);
  if(!(bytes_read))
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                   "qcpformatparser::read_cnfg_chunk : file read failed");
    result = PARSER_ErrorReadFail;
    return result;
  }
  if (bytes_read != QCP_CHUNK_HEADER_SIZE)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                   "qcpformatparser::read_text_chunk:could not read from IFileport1!");
    result = PARSER_ErrorNone;
    return result;
  }

  //Reading "text" chunk
  //check for "text" ChunkID
  if (strncmp ((char*)header_buffer, "text", QCP_FIELD_OFFSET))
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW,
                   "qcpformatparser::read_text_chunk:  'text' chunk not found");
    result = PARSER_ErrorNone;
    return result;
  }

  local_offset += QCP_FIELD_OFFSET;

  // Read "text" chunk size.
  uint32 data_size = 0;
  (void) memmove(&data_size, header_buffer + local_offset, QCP_FIELD_OFFSET);
  data_size = little_endian (data_size);

  if (data_size == 0)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                   "qcpformatparser::read_text_chunk: Empty 'text' chunk");
    result = PARSER_ErrorInvalidParam;
    return result;
  }
  // Read "text" String.
  uint8 *string = NULL ;
  string = (uint8 *) MM_Malloc(data_size * sizeof(uint8));
  if (NULL == string)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                   "qcpformatparser::read_text_chunk: not enough data available!");
    result = PARSER_ErrorMemAllocFail;
    return result;
  }
  (void) memmove(string, header_buffer + local_offset, (int) data_size);
  (void) memmove(m_qcp_tech_metadata.string,string,(int) data_size);
  *chunk_size = QCP_CHUNK_HEADER_SIZE + data_size;
  if (string)
  {
     MM_Free(string);
     string = NULL;
  }
  return result;
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
//  qcpErrorType
//
// SIDE EFFECTS
//  None
//=============================================================================
//
uint64 qcpParser::Seek(uint64 nReposTime)
{

  uint64 time = nReposTime;
  uint64 file_position = 0;
  if(m_seek_function_defined)
  {
    get_seek_position ( time, &file_position);
    //add start offset with current file position
    file_position += m_audio_track.start;
    set_newfile_position(file_position);
  }
  else
  {
    if(NULL != m_pseek)
    {
      time = m_pseek->process_seek(nReposTime);
    }
  }
  m_nCurrTime = time;
  return time;
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
uint8  qcpParser::RandomAccessDenied()
{
  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"qcpParser checking if seek is supported...");
  uint8 nSeekDenied = 1;
  //If seek function is defined, parser can handle the seek.
  if(m_seek_function_defined)
  {
    nSeekDenied = 0;
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"qcpParser can support the seek!!!");
  }
  else
  {
    //Even if seek function is not defined, qcp can be seeked using seek library.
    //Make sure we know the QCP format otherwise, can't support seek using seek lib.
    if(m_qcp_format != QCP_FORMAT_UNKNOWN)
    {
      nSeekDenied = 0;
    }
    /* For larger files,Parser will not create seek table. In such cases,
       Parser will return approximately calculated value as duration.
    */
    if(m_pseek == NULL && m_psimple_seektable == NULL)
    {
      nSeekDenied = 1;
    }
  }
  if(nSeekDenied)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"qcpParser can't support the seek..");
  }
  return nSeekDenied;
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
//  QCP_SUCCESS: Function succesfully calculated the playback duration
//  QCP_UNSUPPORTED: Function is not implemented for this audio format
//  QCP_FAILURE: Data not available to perform the calculation
//
// SIDE EFFECTS
//  None
//=============================================================================
//
PARSER_ERRORTYPE qcpParser::get_seek_position(uint64 time,
                                              uint64* file_position )
{
  (void)time;

  PARSER_ERRORTYPE result = PARSER_ErrorNone;

  // Validate input parameters
  if (NULL == file_position)
  {
    result = PARSER_ErrorInvalidParam;
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                  "qcpformatparser::get_seek_position: file_position parameter was NULL!");
    return result;
  }


  if (QCP_FORMAT_UNKNOWN == m_qcp_format)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                   "qcpformatparser::get_seek_position: Bad state, qcp format not set!");
    result = PARSER_ErrorUnknownCodecType;
    return result;
  }
  else
  {
    if ((((int) m_qcp_format == 1) || ((int) m_qcp_format == 2) || ((int) m_qcp_format == 6))
               && (m_qcp_tech_metadata.var_rate_flag == 0)) //For CBR QCP
    {
      uint64 playback_position = 0;
      //typecast to uint32, possible loss of data
      playback_position = ((
                    time * ( m_qcp_tech_metadata.Packet_size)* 50) / (MILLISECONDS));
      playback_position = ((playback_position/m_qcp_tech_metadata.Packet_size)*
                           m_qcp_tech_metadata.Packet_size);     // Get Packet Allignment (A valid Packet Start)
      *file_position = playback_position;
    }
    else if ((((int) m_qcp_format == 3) || ((int) m_qcp_format == 4) || ((int) m_qcp_format == 6))
                && (m_qcp_tech_metadata.var_rate_flag == 1))     // VBR QCP file
    {
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
                     "qcpformatparser::get_seek_position: Variable Bitrate Flag is set seeking not supported!");
      result = PARSER_ErrorUnknownCodecType;
      return(result);
    }
  }

  return result;
}

//=============================================================================
// FUNCTION : get_seek_function_defined
//
// DESCRIPTION
//  Returns true if optional duration and seek functions are provided
//
// PARAMETERS
//  seek_function_defined :
//  true if optional duration and seek functions are provided
//
// RETURN VALUE
//  QCP_SUCCESS : success
//  AEE_EFAILED : failure
//
// SIDE EFFECTS
//  None
//=============================================================================
//
PARSER_ERRORTYPE qcpParser::get_seek_function_defined()
{

  PARSER_ERRORTYPE result = PARSER_ErrorNone;

  if (QCP_FORMAT_UNKNOWN == m_qcp_format)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                   "qcpformatparser::get_seek_function_defined: Bad state, format not set!");
    result = PARSER_ErrorUnknownCodecType;
    return result;
  }
  if (m_qcp_tech_metadata.var_rate_flag == 0)
  {
    m_seek_function_defined = true;// CBR
  }
  else if ((m_qcp_tech_metadata.var_rate_flag > 0) &&
          (m_qcp_tech_metadata.var_rate_flag < 0xFFFF0000))
  {
    m_seek_function_defined = false;// VBR
  }
  else if (m_qcp_tech_metadata.var_rate_flag >= 0xFFFF0001)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,
                   "qcpformatparser::get_seek_function_defined: var_rate_flag is set to a value which is not supported!");
    result = PARSER_ErrorStreamCorrupt;
    return result;
  }
  return result;
}



//=============================================================================
// FUNCTION : parse_frame_header
//
// DESCRIPTION
//  Parse a packet header according to audio specific format and return the
//  calculated packet size. AudioBuffer must contain an entire frame header,
//  otherwise error will be returned.
//  This function must not consume data from the AudioBuffer
//  (ie. does not update the AudioBuffer's pointers).
//
// PARAMETERS
//  frame :  A byte pointer to frame_size bytes, the frame header to be parsed
//           Note: the entire frame header is expected, thus it must be at least
//              <em>frame_header_size</em> in length.
//  frame_size : frame size determine from parsing the frame header
//
// RETURN VALUE
//  QCP_SUCCESS: Frame header was valid, and frame size is returned
//  AEE_EFAILED: Frame header was INVALID, frame size is not valid
//
// SIDE EFFECTS
//  None
//=============================================================================
//
IAudioReturnType  qcpParser::parse_frame_header(/*in*/uint8* frame,
                                                /*rout*/uint32* frame_size,
                                                /*out*/uint32* frame_time)
{
  int loop_cnt = 0;
  IAudioReturnType result = IAUDIO_SUCCESS;

  // Validate input parameters
  if( (!frame) || (!frame_size) || (!frame_time) )
  {
    result = IAUDIO_FAILURE;
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                   "qcpformatparser::parse_frame_header: frame/frame_size/frame_time parameter is NULL!");
  }
  else
  {
    //Initialize the output variables
    *frame_size = 0;
    *frame_time = 0;

    if (QCP_FORMAT_UNKNOWN == m_qcp_format)
    {
      //parse_file_header API is not called yet - parse_frame_header can be
      //called only after QCP format parser successfully parses the file header
      MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                   "qcpformatparser::parse_frame_header: Bad state, QCP format not set!");
      result = IAUDIO_FAILURE;
    }
    else
    {
      //parse the frame header
      if( (m_qcp_tech_metadata.var_rate_flag > 0) &&
          (m_qcp_tech_metadata.var_rate_flag <0xFFFF0000) ) // For VBR
      {
        for(loop_cnt=1;loop_cnt<=15;loop_cnt+=2)
        {
          if(frame[0] == m_qcp_tech_metadata.rate_map_entry[loop_cnt])
          {
            *frame_size = (m_qcp_tech_metadata.rate_map_entry[loop_cnt-1] + 1);  // Here 1 is added for rate-octet Byte
            result = IAUDIO_SUCCESS;
          }
        }
        *frame_time = QCP_FRAME_DURATION;
      }
      else if (m_qcp_tech_metadata.var_rate_flag == 0)
      {
        // In case of CBR packet size is fixed and = max packet size in the file
        *frame_size =  (uint32) (m_qcp_tech_metadata.Packet_size );
        *frame_time =  QCP_FRAME_DURATION;
        result = IAUDIO_SUCCESS;
      }
      else if (m_qcp_tech_metadata.var_rate_flag >= 0xFFFF0001)
      {
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                     "qcpformatparser::parse_frame_header: Invalid Var-Rate-Flag");
        result = IAUDIO_FAILURE;
      }
      if (result != IAUDIO_SUCCESS)
      {
        MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                       "qcpformatparser::parse_frame_header: Failed to parse frame header");
      }
    }
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
PARSER_ERRORTYPE qcpParser::parse_file_header()
{

  PARSER_ERRORTYPE result = PARSER_ErrorNone;

  if (NULL == m_QCPFilePtr)
  {
    result = PARSER_ErrorDefault;
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                   "qcpformatparser::parse_file_header: file parameter was NULL!");
    return result;
  }
  if (QCP_FORMAT_UNKNOWN != m_qcp_format)
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                   "qcpformatparser::parse_file_header: Unknown QCP format!");
    result = PARSER_ErrorUnknownCodecType;
    return result;
  }
  uint32 offset = 0;
  uint32 chunk_size = 0;

  //check RIFF chunk first
  if (PARSER_ErrorNone != (result = read_riff_chunk(&chunk_size)))
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                   "qcpformatparser::parse_file_header: not a qcp-riff chunk");
    return result;
  }
  //DBG(LOW,vastr("RIFF Chunk Size = %x",chunk_size));
  offset += QCP_RIFF_HEADER_SIZE;

  //read "fmt" chunk, fill in tech meta data
  if (PARSER_ErrorNone != (result = read_fmt_chunk(&chunk_size)))
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                   "qcpformatparser::parse_file_header: unable to get fmt chunk info properly");
    return result;
  }
  offset += chunk_size;
  //read "vrat" chunk
  if (PARSER_ErrorNone != (result = read_vrat_chunk(&offset,&chunk_size)))
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                   "qcpformatparser::parse_file_header: unable to get vrat chunk properly");
    return result;
  }

  offset += chunk_size;
  //chunk_size = 0;
  //read "labl" chunk
  if (PARSER_ErrorNone != (result = read_labl_chunk(&offset,&chunk_size)))
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                   "qcpformatparser::parse_file_header: unable to get [labl] chunk ");
    return result;
  }

  offset += chunk_size;
  chunk_size = 0;
  //read "offs" chunk
  if (PARSER_ErrorNone != (result = read_offs_chunk(&offset,&chunk_size)))
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                   "qcpformatparser::parse_file_header: unable to get [offs] chunk ");
    return result;
  }

  offset += chunk_size;

  //read "data" chunk, point to audio data
  if (PARSER_ErrorNone != (result = read_data_chunk(&offset,&chunk_size)))
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                   "qcpformatparser::parse_file_header: unable to find data chunk properly");
    return result;
  }
  //if size_in_packet is 0 it means we do not know total number of data packets.
  //When var_rate_flag is false,content is fix rate as opposed to variable rate.
  //Packet_size when available, indicates constant size of each data packets when content is fix rate.
  if( (m_qcp_tech_metadata.size_in_packet == 0)&&
      (m_qcp_tech_metadata.var_rate_flag == 0) &&
      (m_qcp_tech_metadata.Packet_size)
    )
  {
    //calculate total number of data packets using data chunk length and constant data pkt. size
    m_qcp_tech_metadata.size_in_packet = m_qcp_tech_metadata.data_chunk_length
                                          /m_qcp_tech_metadata.Packet_size;
    MM_MSG_PRIO3(MM_FILE_OPS, MM_PRIO_HIGH,
    "parse_file_header:Calculated size_in_packet %lu using data chunk length %lu\
    and Packet_size %d",
    m_qcp_tech_metadata.size_in_packet,m_qcp_tech_metadata.data_chunk_length,
    m_qcp_tech_metadata.Packet_size);
  }

  offset += chunk_size;
  chunk_size = 0;
  //read "cnfg" chunk, point to audio data
  if (PARSER_ErrorNone != (result = read_cnfg_chunk(&offset,&chunk_size)))
  {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,
                   "qcpformatparser::parse_file_header: unable to find [cnfg] chunk");
    return result;
  }
  offset += chunk_size;
  chunk_size = 0;
  //read "text" chunk, point to audio data
  if (PARSER_ErrorNone != (result = read_text_chunk(&offset,&chunk_size))) {
    MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_LOW,
                   "qcpformatparser::parse_file_header: unable to find [text] chunk");
    return result;
  }
  offset += chunk_size;
  //DBG(LOW,vastr(" [parse_file_header] todal header size is  = %d",offset-(uint32) 1));
  //DBG(LOW,vastr(" [parse_file_header] File Type is  = %d",m_qcp_tech_metadata.type));
  //DBG(LOW,vastr(" [parse_file_header] Sampling frequency = %d",m_qcp_tech_metadata.Sampling_rate));
  //DBG(LOW,vastr(" [parse_file_header] chunk length is  = %d",m_qcp_tech_metadata.data_chunk_length));
  m_audio_track.start  = (uint64)(offset -m_qcp_tech_metadata.data_chunk_length ) ;
  m_audio_track.end   = (uint64) (offset);
  m_audio_track.size   = (uint64) (m_qcp_tech_metadata.data_chunk_length);

  return result;
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
FileSourceStatus qcpParser::SetAudioOutputMode(FileSourceConfigItemEnum henum)
{
  FileSourceStatus status = FILE_SOURCE_FAIL;
  //Make sure format is known as otherwise, parser can't output on frame boundary.
  if(QCP_FORMAT_UNKNOWN != m_qcp_format)
  {
    //Default mode is FILE_SOURCE_MEDIA_OUTPUT_BYTES_STREAM
    //We do not support changing output mode during the playback
    if((henum == FILE_SOURCE_MEDIA_OUTPUT_SINGLE_AUDIO_FRAME)&&
       (m_hOutputModeEnum == FILE_SOURCE_MEDIA_OUTPUT_BYTES_STREAM))
    {
      status = FILE_SOURCE_SUCCESS;
      m_hOutputModeEnum = henum;
    }
  }
  else
  {
     MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_FATAL,"QCP FORMAT UNKNOWN...Set ConfigEnum %d failed...",henum);
  }
  return status;
}

/* ======================================================================
FUNCTION:
  GetAudioOutputMode

DESCRIPTION:
  Called by user to check if henum was set earlier

INPUT/OUTPUT PARAMETERS:

RETURN VALUE:
 FILE_SOURCE_SUCCESS if successful in retrieving output mode else returns FILE_SOURCE_FAIL

SIDE EFFECTS:
  None.
======================================================================*/
FileSourceStatus qcpParser::GetAudioOutputMode(bool* bret, FileSourceConfigItemEnum henum)
{
  FileSourceStatus status = FILE_SOURCE_FAIL;
  //if our mode is same as henum,
  if(bret && (henum == m_hOutputModeEnum))
  {
    status = FILE_SOURCE_SUCCESS;
    *bret = true;
  }
  return status;
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

PARSER_ERRORTYPE qcpParser::GetApproxDuration(uint64 *pDuration)
{
  PARSER_ERRORTYPE retVal = PARSER_ErrorDefault;

  MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_HIGH,"GetApproxDuration");

  uint32 nBytesRead       = 0;
  uint32 nFrameSize       = 0;
  uint32 nFrameTime       = 0;
  uint64 nTotalFrameSize  = 0;
  uint32 nFramesParsed    = 0;
  uint64 nCurrOffset      = m_nCurrOffset;
  uint32 nMaxBufSize      = 1;
  uint8  ucDataBuffer[MAX_QCELP_FRAMES];
  while(nFramesParsed < MAX_QCELP_FRAMES)
  {
    nBytesRead = QCPCallbakGetData (nCurrOffset, QCP_FRAME_HEADER_SIZE,
                                    ucDataBuffer, nMaxBufSize, m_pUserData);
    if(nBytesRead)
    {
      /* Check whether frame started with sync marker or not. */
      if(IAUDIO_SUCCESS == parse_frame_header(ucDataBuffer, &nFrameSize,
                                              &nFrameTime) && nFrameSize)
      {
        nCurrOffset += nFrameSize;
        nTotalFrameSize += nFrameSize;
        nFramesParsed++;
      }
      else
      {
        nCurrOffset++;
      }
    }

  }//while(nFramesParsed < MAX_FRAMES)

  uint64 nAvgFrameSize = nTotalFrameSize / nFramesParsed;
  uint64 nFrameCount   = m_audio_track.size / nAvgFrameSize;
  if(pDuration)
  {
    *pDuration = nFrameTime * nFrameCount;
    retVal = PARSER_ErrorNone;
    MM_MSG_PRIO1(MM_FILE_OPS, MM_PRIO_HIGH,
                 "GetApproxDuration calculated is %llu", *pDuration );
  }

  return retVal;
}

