/* =======================================================================
mp3vbrheader.cpp
DESCRIPTION
It has the definitions for the functions related to XING and VBRI variable bit
rate parameters.
These functions will be used to parse metadata of MP3 files and to extract
frame data.

EXTERNALIZED FUNCTIONS
List functions and a brief description that are exported from this file

INITIALIZATION AND SEQUENCING REQUIREMENTS
Detail how to initialize and use this service.  The sequencing aspect
is only needed if the order of operations is important.

Copyright 2011-2014 Qualcomm Technologies, Inc., All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
========================================================================== */
/* =======================================================================
                               Edit History

$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/MP3ParserLib/main/latest/src/mp3vbrheader.cpp#22 $
$DateTime: 2012/07/25 21:19:29 $
$Change: 2631309 $

========================================================================== */

#include "parserdatadef.h"
#include "mp3vbrheader.h"
#include "mp3parser.h"
#include "MMMemory.h"

//=============================================================================
// GLOBALS
//=============================================================================

//=============================================================================
// CONSTANTS
//=============================================================================

// maximum size of xing vbr header
const int mp3xingheader::m_max_xing_header_size = 120;

// maximum size of vbri vbr header
// (toc entries field is not considered in this size)
const int mp3vbriheader::m_max_vbri_header_size = 26;

// offset within first MP3 audio frame from where the VBRI header starts
const uint8 mp3vbriheader::m_vbri_header_offset = 32;


//=============================================================================
// FUNCTION DEFINATONS
//=============================================================================

//=============================================================================
// FUNCTION DEFINATONS of mp3vbrheader class
//=============================================================================

//=============================================================================
// FUNCTION : parse_mp3_vbr_header
//
// DESCRIPTION
//  Finds and parses VBR header if present in given file
//
// PARAMETERS
//  file : IFilePort1 interface to input stream
//  first_sync_offset : start position of first frame in given input file
//  hdr_info : MP3 header information
//  vbr_header_info (out) : interface for accessing vbr header information
//                          obtained after parsing
//
// RETURN VALUE
//  MP3_SUCCESS : success
//  MP3_INVALID_USER_DATA : Invalid parameters
//  MP3_FAILURE : general failure
//
// SIDE EFFECTS
//  None
//=============================================================================
//
PARSER_ERRORTYPE mp3vbrheader::parse_mp3_vbr_header (
                          OSCL_FILE *m_MP3FilePtr ,
                          uint64 first_sync_offset,
                          const struct tech_data_mp3 &hdr_info,
                          mp3vbrheader** vbr_header_info) {


   if (NULL == m_MP3FilePtr) {
      //DBG(HIGH,
         //"mp3vbrheader::parse_mp3_vbr_header: file parameter was NULL!");
      return PARSER_ErrorDefault;
   }

   if (NULL == vbr_header_info) {
      //DBG(HIGH,
         //"mp3vbrheader::parse_mp3_vbr_header:  parameter was NULL!");
      return PARSER_ErrorInvalidParam;
   }

   // Initialize output parameters
   *vbr_header_info = NULL;

   PARSER_ERRORTYPE result = PARSER_ErrorNone;

   // Check if input data has Xing header
   result = mp3xingheader::parse_mp3_xing_header( m_MP3FilePtr, first_sync_offset,
                           hdr_info, vbr_header_info);

   if (PARSER_ErrorNone != result) {
      // Check if input data has VBRI header
      result = mp3vbriheader::parse_mp3_vbri_header( m_MP3FilePtr,
                              first_sync_offset, vbr_header_info);
   }


   return result;

}


//=============================================================================
// FUNCTION : read_big_endian_value
//
// DESCRIPTION
//  Converts from big endian to native format (Intel=little endian) and return
//  as uint32 (32bit)
//
// PARAMETERS
//  buffer : input big endian bitstream
//  size : bumber of bits to be converted to little endian
//
// RETURN VALUE
//  little endian 32 bit value
//
// SIDE EFFECTS
//  None
//=============================================================================
//
uint32 mp3vbrheader::read_big_endian_value(const uint8* buffer,
                                           uint32 size) const
{

   uint32 result = 0;

   // big endian extract (most significant byte first)
   // (will work on little and big-endian computers)
   uint32 byte_shifts_num = size - 1;

   for (uint32 count=0; count < size; count++)
   {
       // the bit shift will do the correct byte order for you
       result |= buffer[count] << (8*byte_shifts_num--);
   }

   return result;

}

//=============================================================================
// FUNCTION DEFINATONS of mp3xingheader class
//=============================================================================

//=============================================================================
// FUNCTION : parse_mp3_xing_header
//
// DESCRIPTION
//  Finds and parses XING VBR header if present in given file
//
// PARAMETERS
//  file : IFilePort1 interface to input stream
//  first_sync_offset : start position of first frame in given input file
//  hdr_info : MP3 header information
//  vbr_header_info (out) : interface for accessing vbr header information
//                          obtained after parsing
//
// RETURN VALUE
//  MP3_SUCCESS : success
//  MP3_INVALID_USER_DATA : Invalid parameters
//  MP3_RESOURCENOTFOUND : Xing header not found
//  AEE_ENOMEMORY : No memory
//  MP3_FAILURE : general failure
//
// SIDE EFFECTS
//  None
//=============================================================================
//
PARSER_ERRORTYPE mp3xingheader::parse_mp3_xing_header (
                           OSCL_FILE *m_MP3FilePtr,
                           uint64 first_sync_offset,
                           const struct tech_data_mp3 &hdr_info,
                           mp3vbrheader** vbr_header_info) {

   PARSER_ERRORTYPE result = PARSER_ErrorNone;

   // Initialize output parameters
   *vbr_header_info = NULL;

   uint8 buffer[mp3xingheader::m_max_xing_header_size] = {0};

   uint32 sideinfo_size  = 0;

   if (MP3_LAYER_3 == hdr_info.layer)
   {
      // Get the side info size if given MP3 data is Layer 3
      sideinfo_size = MP3_SIDEINFO_SIZES[hdr_info.version]
                      [(hdr_info.channel == MP3_CHANNEL_SINGLE ? 1 : 0)];
   }

   // Calculate start position of Xing header within first audio frame
   uint64 offset = first_sync_offset + mp3Parser::m_header_size + sideinfo_size;

   // Seek to the calculated offset and read data from IFilePort1

   uint32 bytes_read = 0;

   int buf_len = m_max_xing_header_size;

   if(PARSER_ErrorNone !=
      OSCL_FileSeek(m_MP3FilePtr, offset, SEEK_SET))
   {
      //DBG(HIGH,"mp3xingheader::parse_mp3_vbr_header: file seek failed");
      return PARSER_ErrorReadFail;
   }

   // Read data from file
   bytes_read = OSCL_FileRead(buffer,buf_len,1,m_MP3FilePtr);
   if(!bytes_read)
   {
      //DBG(HIGH,"mp3xingheader::parse_mp3_vbr_header: file read failed");
      return PARSER_ErrorReadFail;
   }

   // Check if sufficient data is read
   if(bytes_read < XING_ID_SIZE) {
       //DBG(HIGH,"mp3xingheader::parse_mp3_vbr_header:"
           //" failed to read required amount of data");
      return PARSER_ErrorDefault;
   }

   //check for id of xing header
   if ( 0 != std_memcmp (buffer, "Xing", XING_ID_SIZE)) {
      //DBG(HIGH,"mp3xingheader::parse_mp3_vbr_header: not a Xing header");
      return PARSER_ErrorUnsupported;
   }

   // Parse the remaining fields of Xing header
   mp3xingheader* temp = MM_New_Args(mp3xingheader, ( buffer, bytes_read, result));

   if (NULL == temp) {
      //DBG(HIGH,"mp3xingheader::parse_mp3_vbr_header:"
          //" failed to create instance of mp3xingheader");
      return PARSER_ErrorInsufficientBufSize;
   }

   if (PARSER_ErrorNone != result) {
      MM_Delete( temp);
      return result;
   }

   // if Xing header is parsed successfully return the mp3xingheader instance
   *vbr_header_info = (mp3vbrheader*)temp;


   return result;

}


//=============================================================================
// FUNCTION : mp3xingheader
//
// DESCRIPTION
//  constrctor; Parses the Xing VBR header
//
// PARAMETERS
//  buffer : buffer containing raw data of Xing VBR header
//  buf_len : length of input buffer
//  result (out) : O/p parameter that is updated with result of parsing
//
// RETURN VALUE
//  None
//
// SIDE EFFECTS
//  None
//=============================================================================
//
mp3xingheader::mp3xingheader (const uint8* buffer,
                              uint32 buf_len, PARSER_ERRORTYPE &result)
{
   (void) std_memset (&m_xing_header, 0, sizeof(m_xing_header));

   uint32 read_position = 4;

   // get flags (mandatory in XING header)
   m_xing_header.flags = read_big_endian_value(buffer+read_position, 4);

   read_position += 4;

   // get total frames
   if (read_position + 4 <= buf_len &&
      (m_xing_header.flags & MP3_XING_FRAMES_FLAG)) {
      m_xing_header.frames = read_big_endian_value(buffer+read_position, 4);
      read_position += 4;
   }

   // get total bytes
   if (read_position +4 <= buf_len &&
      (m_xing_header.flags & MP3_XING_BYTES_FLAG)) {
      m_xing_header.bytes = read_big_endian_value(buffer+read_position, 4);
      read_position += 4;
   }

   // get TOC entries
   if (read_position + 100 <= buf_len &&
      (m_xing_header.flags & MP3_XING_TOC_FLAG)) {
      for(int count=0;count<MP3_XING_TOC_LEN; count++) {
         m_xing_header.toc_entries[count] = (uint8) read_big_endian_value
                                            (buffer+read_position, 1);
         read_position++;
      }
   }

   // get quality indicator
   if (read_position +4 <= buf_len &&
      (m_xing_header.flags & MP3_XING_QUAL_IND)) {
      m_xing_header.quality_ind = read_big_endian_value
                                  (buffer+read_position, 4);
      read_position += 4;
   }

   result = PARSER_ErrorNone;
}


//=============================================================================
// FUNCTION : ~mp3xingheader
//
// DESCRIPTION
//  destructor; Frees resources acquired by mp3xingheader instance
//
// PARAMETERS
//  None
//
// RETURN VALUE
//  None
//
// SIDE EFFECTS
//  None
//=============================================================================
//
mp3xingheader::~mp3xingheader ()
{

   //DBG_ENTER(~mp3xingheader);

}


//=============================================================================
// FUNCTION : get_seek_function_defined
//
// DESCRIPTION
//  Returns true if VBR header has enough information for calculation of
//  duration and seek positions
//
// PARAMETERS
//  seek_function_defined : true if optional duration and seek functions
//                          are provided
//
// RETURN VALUE
//  MP3_SUCCESS : success
//  MP3_INVALID_USER_DATA : Invalid parameters
//  MP3_FAILURE : general failure
//
// SIDE EFFECTS
//  None
//=============================================================================
//
PARSER_ERRORTYPE mp3xingheader::get_seek_function_defined
                           (bool &seek_function_defined) const {

   //DBG_ENTER(get_seek_function_defined);

   PARSER_ERRORTYPE result = PARSER_ErrorNone;

   uint32 required_flags = MP3_XING_TOC_FLAG | MP3_XING_FRAMES_FLAG |
                           MP3_XING_BYTES_FLAG;

   // update the output parameter if information is available in
   // Xing VBR header
   // Seek can be supported only when TOC entries and
   // information for calculation total playback duration is avialable
   if ((m_xing_header.flags & required_flags) == required_flags) {
      seek_function_defined = true;
   }
   else {
      seek_function_defined = false;
   }

   return result;

}

//=============================================================================
// FUNCTION : get_seek_position
//
// DESCRIPTION
//  Calculates seek position based on time.this function is only implemented
//  when the seek_function_defined is set to TRUE
//
// PARAMETERS
//  time : Presentation time to seek to
//  playback_duration : Total playback duration
//  file_position (out) : File position corresponding to the seek time provided
//
// RETURN VALUE
//  MP3_SUCCESS : success
//  AEE_UNSUPPORTED : Function is not implemented for this audio format
//  MP3_FAILURE : general failure
//
// SIDE EFFECTS
//  None
//=============================================================================
//
PARSER_ERRORTYPE mp3xingheader::get_seek_position (uint64 time,
                                                   uint64 playback_duration,
                                                   uint64* file_position) const
{

   //DBG_ENTER(get_seek_position);

   PARSER_ERRORTYPE result = PARSER_ErrorNone;

   // Validate input parameters
   if (NULL == file_position) {
      result = PARSER_ErrorInvalidParam;
      //DBG(HIGH,"mp3xingheader::get_seek_position:"
         //" file_position parameter was NULL!");
      return result;
   }

   // Initialize output parameters
   *file_position = 0;

  int percent_x10; // percentage into file to move, factored by 10 (000-999)
  int percent_a;   // percentage into file to move, 0-99 (fraction truncated)
  int factor_a;    // lower factor based on 0-99 percent from TOC
  int factor_b;    // upper factor based on 0-99 percent from TOC
  int factor_x;    // estimated factor between factor_a and factor_b

   // Calculate percentage into song, factored by 10
   percent_x10 =  ((((int)time) * 1000) / (int)playback_duration);
   if(percent_x10 >= 990) {
     // Set percent_a and factor_b to max values
     percent_a = 99;
     factor_b = 256;
   } else {
     // Calculate percent_a and retrieve factor_b
     percent_a = percent_x10 / 10;
     factor_b = m_xing_header.toc_entries [(percent_a) +1];
   }

   factor_a = m_xing_header.toc_entries [percent_a];

   // Factor_x is an approximate gradient between factor_a and factor_b based
   // on percentage differences of factor_a and factor_b.  Factor_x is scaled
   // by 10 for greater precision.  The value is between 0 and 2560.
   factor_x = factor_a * 10 + (factor_b - factor_a) *
              (percent_x10 - percent_a * 10);

   // return approximate destination position into file
   *file_position =  ((uint64)factor_x *
                    (m_xing_header.bytes/ 2560));

   return result;

}

//=============================================================================
// FUNCTION : get_total_frames
//
// DESCRIPTION
//  Returns total frame count information stored in the VBR header
//
// PARAMETERS
//  frame_bytes (out) : Total number of frames
//
// RETURN VALUE
//  MP3_SUCCESS : success
//  MP3_FAILURE : general failure
//
// SIDE EFFECTS
//  None
//=============================================================================
//
PARSER_ERRORTYPE mp3xingheader::get_total_frames (uint32 &total_frames) const
{

   //DBG_ENTER(get_total_frames);

   PARSER_ERRORTYPE result = PARSER_ErrorNone;

   // Initialize output parameters
   total_frames = 0;

   // Update the output parameter if information is available in
   // Xing VBR header
   if (m_xing_header.flags & MP3_XING_FRAMES_FLAG) {
      total_frames = m_xing_header.frames;
   }
   else {
      result = PARSER_ErrorDefault;
   }

   return result;

}

//=============================================================================
// FUNCTION : get_total_bytes
//
// DESCRIPTION
//  Returns total bytes information stored in the VBR header
//
// PARAMETERS
//  frame_bytes (out) : Total bytes
//
// RETURN VALUE
//  MP3_SUCCESS : success
//  MP3_FAILURE : general failure
//
// SIDE EFFECTS
//  None
//=============================================================================
//
PARSER_ERRORTYPE mp3xingheader::get_total_bytes (uint32 &total_bytes) const
{

   //DBG_ENTER(get_total_bytes);

   PARSER_ERRORTYPE result = PARSER_ErrorNone;

   // Initialize output parameters
   total_bytes = 0;

   // Update the output parameter if information is available
   // in Xing VBR header
   if (m_xing_header.flags & MP3_XING_BYTES_FLAG) {
      total_bytes = m_xing_header.bytes;
   }
   else {
      result = PARSER_ErrorDefault;
   }

   return result;

}

//=============================================================================
// FUNCTION : show
//
// DESCRIPTION
//  Output class attributes for debugging. These functions should compile into
//  no-ops when DEBUG is not defined
//
// PARAMETERS
//  str : may include caller information
//
// RETURN VALUE
//  void
//
// SIDE EFFECTS
//  None
//=============================================================================
//
void mp3xingheader::show(const char* str) {

#ifndef _DEBUG
   // Debug is turned off, do nothing
   (void) str;
#else
   // output debug info for this class
   //DBG(LOW,vastr("====== %s ======", str));
   //Xing header info
   //DBG(LOW,vastr("m_xing_header.flags = %0x",m_xing_header.flags));
   //DBG(LOW,vastr("m_xing_header frames= %ld",m_xing_header.frames));
   //DBG(LOW,vastr("m_xing_header bytes= %ld",m_xing_header.bytes));
   //DBG(LOW,vastr("m_xing_header quality_ind= %d",m_xing_header.quality_ind));
   // TOC entries info
   //for (int count = 0; MP3_XING_TOC_LEN > count; count++) {
      //DBG(LOW,vastr("m_xing_header toc_entries[%d] = %d",count,
          //m_xing_header.toc_entries[count]));
   //}
   //DBG(LOW,      "================");
#endif

}

//=============================================================================
// FUNCTION : show
//
// DESCRIPTION
//  Output class attributes for debugging. These functions should compile into
//  no-ops when DEBUG is not defined
//
// PARAMETERS
//  None
//
// RETURN VALUE
//  void
//
// SIDE EFFECTS
//  None
//=============================================================================
//
void mp3xingheader::show() {
   show("");
}

//=============================================================================
// FUNCTION DEFINATONS of mp3vbriheader class
//=============================================================================

//=============================================================================
// FUNCTION : parse_mp3_vbr_header
//
// DESCRIPTION
//  Finds and parses VBRI VBR header if present in given file
//
// PARAMETERS
//  file : IFilePort1 interface to input stream
//  first_sync_offset : start position of first frame in given input file
//  hdr_info : MP3 header information
//  vbr_header_info (out) : interface for accessing vbr header information
//                          obtained after parsing
//
// RETURN VALUE
//  MP3_SUCCESS : success
//  MP3_INVALID_USER_DATA : Invalid parameters
//  MP3_RESOURCENOTFOUND : Xing header not found
//  AEE_ENOMEMORY : No memory
//  MP3_FAILURE : general failure
//
// SIDE EFFECTS
//  None
//=============================================================================
//
PARSER_ERRORTYPE mp3vbriheader::parse_mp3_vbri_header (
                            OSCL_FILE *m_MP3FilePtr ,
                           uint64 first_sync_offset,
                           mp3vbrheader** vbr_header_info)
{
   PARSER_ERRORTYPE result = PARSER_ErrorNone;

   // Validate input parameters
   if (NULL == m_MP3FilePtr) {
      //DBG(HIGH,
         //"mp3vbrheader::parse_mp3_vbr_header: file parameter was NULL!");
      return PARSER_ErrorDefault;
   }

   // Validate input parameters
   if (NULL == vbr_header_info) {
      //DBG(HIGH,
         //"mp3vbrheader::parse_mp3_vbr_header:"
         //" vbr_header_info parameter was NULL!");
      return PARSER_ErrorInvalidParam;
   }

   uint8 buffer[mp3vbriheader::m_max_vbri_header_size] = {0};

   // Calculate start position of Xing header within first audio frame
   uint64 offset = first_sync_offset + mp3Parser::m_header_size +
                     mp3vbriheader::m_vbri_header_offset;

   // Seek to the calculated offset and read data from IFilePort1
   int bytes_read = 0;

   int buf_len = mp3vbriheader::m_max_vbri_header_size;

   if(PARSER_ErrorNone !=
      OSCL_FileSeek(m_MP3FilePtr, offset, SEEK_SET))
   {
      //DBG(HIGH,"mp3vbriheader::parse_mp3_vbr_header: file seek failed");
      return PARSER_ErrorReadFail;
   }

   // Read data from file to parse the header
   bytes_read = (int)OSCL_FileRead(buffer,buf_len,1,m_MP3FilePtr);
   if(!bytes_read)
   {
      //DBG(HIGH,"mp3vbriheader::parse_mp3_vbr_header: file read failed");
      return PARSER_ErrorReadFail;
   }

   // Check if sufficient data is read
   if(bytes_read < mp3vbriheader::m_max_vbri_header_size) {
      //DBG(HIGH,"mp3vbriheader::parse_mp3_vbr_header:"
          //" could not read enough data");
      return PARSER_ErrorReadFail;
   }

   //Check for id of VBRI header
   if ( 0 != std_memcmp (buffer, "VBRI", VBRI_ID_SIZE)) {
      //DBG(HIGH,"mp3vbriheader::parse_mp3_vbr_header: VBRI header not found");
      return PARSER_ErrorUnsupported;
   }

   // Parse the remaining fields of VBRI header
   mp3vbriheader* temp = MM_New_Args(mp3vbriheader,( m_MP3FilePtr, buffer,
                                                     bytes_read, result));

   if (NULL == temp) {
      //DBG(HIGH,"mp3vbriheader::parse_mp3_vbr_header:"
          //" failed to create instance of mp3vbriheader");
      return PARSER_ErrorMemAllocFail;
   }

   // if VBRI header is parsed successfully return the mp3vbriheader instance
   if (PARSER_ErrorNone == result) {
      *vbr_header_info = (mp3vbrheader*) temp;
      return PARSER_ErrorNone;
   }
   else {
      MM_Delete( temp);
   }



   return result;

}


//=============================================================================
// FUNCTION : mp3vbriheader
//
// DESCRIPTION
//  constrctor; Parses the VBRI VBR header
//
// PARAMETERS
//  file : IFilePort1 interface to input stream
//  buffer : buffer containing raw data of VBRI VBR header
//  buf_len : length of input buffer
//  result (out) : O/p parameter that is updated with result of parsing
//
// RETURN VALUE
//  None
//
// SIDE EFFECTS
//  None
//=============================================================================
//
mp3vbriheader::mp3vbriheader ( OSCL_FILE *m_MP3FilePtr ,
                               uint8* buffer,
                               int buf_len, PARSER_ERRORTYPE &result)
{
  (void) std_memset (&m_vbri_header, 0, sizeof(m_vbri_header));
  if(buffer && m_MP3FilePtr && buf_len)
  {
    result = PARSER_ErrorNone;
    uint32 read_position = 4;
    m_vbri_header.version = (uint16)read_big_endian_value(buffer+read_position,2);
    read_position += 2;
    m_vbri_header.delay = (uint16)read_big_endian_value(buffer+read_position, 2);
    read_position += 2;
    m_vbri_header.quality_ind = (uint16)read_big_endian_value(buffer+read_position, 2);
    read_position += 2;
    m_vbri_header.bytes = read_big_endian_value(buffer+read_position, 4);
    read_position += 4;
    m_vbri_header.frames = read_big_endian_value(buffer+read_position, 4);
    read_position += 4;
    m_vbri_header.entry_count = (uint16)read_big_endian_value(buffer+read_position, 2);
    read_position += 2;
    m_vbri_header.scale = (uint16)read_big_endian_value(buffer+read_position, 2);
    read_position += 2;
    m_vbri_header.toc_entry_size = (uint16) read_big_endian_value(buffer+read_position, 2);
    read_position += 2;
    m_vbri_header.frames_per_entry = (uint16) read_big_endian_value(buffer+read_position, 2);
    read_position += 2;
    m_vbri_header.toc_entries = NULL;
    m_vbri_header.toc_entries = (uint16 *) MM_Malloc(sizeof(uint16)* m_vbri_header.entry_count);
    if (NULL == m_vbri_header.toc_entries)
    {
      result = PARSER_ErrorInsufficientBufSize;
      return;
    }
    int nextrabuffsize = m_vbri_header.toc_entry_size*m_vbri_header.entry_count;
    uint8* databuffer = (uint8*)MM_Malloc(nextrabuffsize);
    if(databuffer)
    {
      // read data from file to parse the header
      if(!(OSCL_FileRead(databuffer,nextrabuffsize,1,m_MP3FilePtr)))
      {
        result = PARSER_ErrorReadFail;
      }
      else
      {
        //reset read offset
        read_position = 0;
        for(int count=0;count<m_vbri_header.entry_count; count++)
        {
          m_vbri_header.toc_entries[count] = (uint16) read_big_endian_value
                                             (databuffer+read_position,
                                             m_vbri_header.toc_entry_size);
          read_position += m_vbri_header.toc_entry_size;
        }
      }
      MM_Free(databuffer);
      databuffer = NULL;
    }//if(databuffer)
  }//if(buffer && m_MP3FilePtr)
}

//=============================================================================
// FUNCTION : mp3vbriheader
//
// DESCRIPTION
//  copy constrctor;
//
// PARAMETERS
//  in_obj : mp3vbriheader instance
//
// RETURN VALUE
//  None
//
// SIDE EFFECTS
//  None
//=============================================================================
//
mp3vbriheader::mp3vbriheader (const mp3vbriheader& in_obj)
{

   //DBG_ENTER(mp3vbriheader);

   memset(&m_vbri_header,0,sizeof(mp3_vbri_header_type));

   // Copy the VBRI header info
   (void) std_memmove (&m_vbri_header, &(in_obj.m_vbri_header),
                      sizeof(m_vbri_header));

   m_vbri_header.toc_entries = (uint16 *) MM_Malloc(sizeof(uint16)*
                                                    m_vbri_header.entry_count);

   if (NULL == m_vbri_header.toc_entries)
   {
      //DBG(HIGH,"mp3vbriheader::mp3vbriheader:"
          //" failed to allocate memory for toc entries");
      return;
   }

   // Copy the TOC entries
   for(int count=0;count<m_vbri_header.entry_count; count++)
   {
      m_vbri_header.toc_entries[count] =
                                       in_obj.m_vbri_header.toc_entries[count];
   }

}

//=============================================================================
// FUNCTION : operator=
//
// DESCRIPTION
//  assignment operator;
//
// PARAMETERS
//  in_obj : mp3vbriheader instance
//
// RETURN VALUE
//  None
//
// SIDE EFFECTS
//  None
//=============================================================================
//
mp3vbriheader& mp3vbriheader::operator= (const mp3vbriheader& in_obj)
{

   //DBG_ENTER(operator=);

   // Check for self assignment
   if (&(in_obj) == this){
      return (*this);
   }

   // Copy the VBRI header info
   (void) std_memmove (&m_vbri_header, &(in_obj.m_vbri_header),
                      sizeof(m_vbri_header));

   // Copy the TOC entries
   for(int count=0;count<m_vbri_header.entry_count; count++) {
      m_vbri_header.toc_entries[count] =
                                       in_obj.m_vbri_header.toc_entries[count];
   }

   return (*this);

}

//=============================================================================
// FUNCTION : ~mp3vbriheader
//
// DESCRIPTION
//  destructor; Frees resources acquired by mp3vbriheader instance
//
// PARAMETERS
//  None
//
// RETURN VALUE
//  None
//
// SIDE EFFECTS
//  None
//=============================================================================
//
mp3vbriheader::~mp3vbriheader ()
{

   //DBG_ENTER(~mp3vbriheader);

   // Free memory allocated for TOC entries
   if (m_vbri_header.toc_entries) {
      MM_Free(m_vbri_header.toc_entries);
      m_vbri_header.toc_entries = NULL;
   }

}

//=============================================================================
// FUNCTION : get_seek_function_defined
//
// DESCRIPTION
//  Returns true if VBR header has enough information for calculation of
//  duration and seek positions
//
// PARAMETERS
//  seek_function_defined : true if optional duration and seek functions are
//                          provided
//
// RETURN VALUE
//  MP3_SUCCESS : success
//  MP3_INVALID_USER_DATA : Invalid parameters
//  MP3_FAILURE : general failure
//
// SIDE EFFECTS
//  None
//=============================================================================
//
PARSER_ERRORTYPE mp3vbriheader::get_seek_function_defined
                           (bool &seek_function_defined) const
{

   //DBG_ENTER(get_seek_function_defined);

   PARSER_ERRORTYPE result = PARSER_ErrorNone;

   // Check if sufficient information is available for
   // supporting seek and get_duration
   uint32 tmp =   (0 < m_vbri_header.frames) &&
                  (0 < m_vbri_header.bytes) &&
                  (0 < m_vbri_header.entry_count) &&
                  (0 < m_vbri_header.frames_per_entry) &&
                  (0 < m_vbri_header.toc_entry_size) &&
                  (NULL != m_vbri_header.toc_entries);

   if (tmp) {
      seek_function_defined = true;
   }
   else {
      seek_function_defined = false;
   }

   return result;

}

//=============================================================================
// FUNCTION : get_seek_position
//
// DESCRIPTION
//  Calculates seek position based on time.this function is only implemented
//  when the seek_function_defined is set to TRUE
//
// PARAMETERS
//  time : Presentation time to seek to
//  playback_duration : Total playback duration
//  file_position (out) : File position corresponding to the seek time provided
//
// RETURN VALUE
//  MP3_SUCCESS : success
//  MP3_INVALID_USER_DATA : Invalid parameters
//  MP3_FAILURE : general failure
//
// SIDE EFFECTS
//  None
//=============================================================================
//
PARSER_ERRORTYPE mp3vbriheader::get_seek_position (
                         uint64 time, uint64 playback_duration,
                         uint64* file_position) const
{

   //DBG_ENTER(get_seek_position);

   PARSER_ERRORTYPE result = PARSER_ErrorNone;

   (void) time;

   // Validate input parameters
   if (NULL == file_position) {
      result = PARSER_ErrorInvalidParam;
      //DBG(HIGH,
         //"mp3vbriheader::parse_mp3_vbr_header:"
         //" seek_function_defined parameter was NULL!");
      return result;
   }

   // Calculate duration per TOC entry
   float duration_per_entry = (float)playback_duration /
                              (float)(m_vbri_header.entry_count + 1);

   unsigned int count  = 0;

   float accumulated_time = 0;

   // Get TOC entry corrspondong to duration which is equal to
   // or greater than given seek time
   while (accumulated_time <=  time) {
      *file_position += m_vbri_header.toc_entries[count];
      accumulated_time += duration_per_entry;
      count++;
   }

   // Searched too far; correct result
   float fraction =  ((((accumulated_time - (float)time) / duration_per_entry)
                  + (1.0f/(2.0f*(float) m_vbri_header.frames_per_entry))) *
                  (float) m_vbri_header.frames_per_entry);

   *file_position -= (uint64) ((float) m_vbri_header.toc_entries [count-1] *
                     (float) (fraction) /
                     (float) m_vbri_header.frames_per_entry);

   return result;

}

//=============================================================================
// FUNCTION : get_total_frames
//
// DESCRIPTION
//  Returns total frame count information stored in the VBR header
//
// PARAMETERS
//  frame_bytes (out) : Total number of frames
//
// RETURN VALUE
//  MP3_SUCCESS : success
//  MP3_FAILURE : general failure
//
// SIDE EFFECTS
//  None
//=============================================================================
//
PARSER_ERRORTYPE mp3vbriheader::get_total_frames (uint32 &total_frames) const
{

   //DBG_ENTER(get_total_frames);

   PARSER_ERRORTYPE result = PARSER_ErrorNone;

   // Initialize output parameters
   total_frames = 0;

   // Update the output parameter if information is available in
   // VBRI VBR header
   if (0 < m_vbri_header.frames)
   {
      total_frames = m_vbri_header.frames;
   }
   else
   {
      result = PARSER_ErrorStreamCorrupt;
   }

   return result;

}

//=============================================================================
// FUNCTION : get_total_bytes
//
// DESCRIPTION
//  Returns total bytes information stored in the VBR header
//
// PARAMETERS
//  frame_bytes (out) : Total bytes
//
// RETURN VALUE
//  MP3_SUCCESS : success
//  MP3_FAILURE : general failure
//
// SIDE EFFECTS
//  None
//=============================================================================
//
PARSER_ERRORTYPE mp3vbriheader::get_total_bytes (uint32 &total_bytes) const
{

   //DBG_ENTER(get_total_bytes);

   PARSER_ERRORTYPE result = PARSER_ErrorNone;

   // Initialize output parameters
   total_bytes = 0;

   // Update the output parameter if information is available in
   // VBRI VBR header
   if (0 < m_vbri_header.bytes){
      total_bytes = m_vbri_header.bytes;
   }
   else {
      result = PARSER_ErrorDefault;
   }

   return result;

}

//=============================================================================
// FUNCTION : show
//
// DESCRIPTION
//  Output class attributes for debugging. These functions should compile into
//  no-ops when DEBUG is not defined
//
// PARAMETERS
//  str : may include caller information
//
// RETURN VALUE
//  void
//
// SIDE EFFECTS
//  None
//=============================================================================
//
void mp3vbriheader::show(const char* str)
{

#ifndef _DEBUG
   // Debug is turned off, do nothing
   (void) str;
#else
   // output debug info for this class
   //DBG(LOW,vastr("====== %s ======", str));
   //VBRI header info
   //DBG(LOW,vastr("m_vbri_header.version = %0x",m_vbri_header.version));
   //DBG(LOW,vastr("m_vbri_header quality_ind= %d",m_vbri_header.quality_ind));
   //DBG(LOW,vastr("m_vbri_header delay= %ld",m_vbri_header.delay));
   //DBG(LOW,vastr("m_vbri_header frames= %ld",m_vbri_header.frames));
   //DBG(LOW,vastr("m_vbri_header bytes= %ld",m_vbri_header.bytes));
   //DBG(LOW,vastr("m_vbri_header entry_count= %ld",m_vbri_header.entry_count));
   //DBG(LOW,vastr("m_vbri_header scale= %ld",m_vbri_header.scale));
   //DBG(LOW,vastr("m_vbri_header toc_entry_size= %ld",
       //m_vbri_header.toc_entry_size));
   //DBG(LOW,vastr("m_vbri_header frames_per_entry= %ld",
       //m_vbri_header.frames_per_entry));
   //DBG(LOW,vastr("m_vbri_header frames_per_entry= %p",
       //m_vbri_header.toc_entries));
   //DBG(LOW,      "================");
#endif

}

//=============================================================================
// FUNCTION : show
//
// DESCRIPTION
//  Output class attributes for debugging. These functions should compile into
//  no-ops when DEBUG is not defined
//
// PARAMETERS
//  None
//
// RETURN VALUE
//  void
//
// SIDE EFFECTS
//  None
//=============================================================================
//
void mp3vbriheader::show()
{
   show("");
}
