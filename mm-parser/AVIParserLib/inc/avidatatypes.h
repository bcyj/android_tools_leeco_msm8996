#ifndef AVI_HDR_DATA_TYPES_H
#define AVI_HDR_DATA_TYPES_H

/* =======================================================================
                              aviDataTypes.h
DESCRIPTION

Copyright 2011-2014 Qualcomm Technologies, Inc., All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
========================================================================== */

/* =======================================================================
                             PERFORCE HEADER
$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/AVIParserLib/main/latest/inc/avidatatypes.h#8 $
========================================================================== */

//Data types used
typedef unsigned int        avi_uint32;
typedef int                 avi_int32;
typedef short               avi_int16;
typedef unsigned short      avi_uint16;
typedef unsigned char       avi_uint8;
typedef char                avi_int8;
typedef signed long long    avi_int64;
typedef unsigned long long  avi_uint64;
typedef avi_uint32          fourCC_t;
typedef avi_uint64          AVI_FILE_OFFSET;

//Various chunk types available in 'MOVI'
typedef enum cType
{
  AVI_CHUNK_UNKNOWN,
  AVI_CHUNK_JUNK,
  AVI_CHUNK_METADATA,
  AVI_CHUNK_DRM,
  AVI_CHUNK_AUDIO,
  AVI_CHUNK_VIDEO,
  AVI_CHUNK_BITMAP_CAPTION,
  AVI_CHUNK_TEXT_CAPTION,
  AVI_CHUNK_CHAPTER,
  AVI_CHUNK_UNCOMPRESSED,
  AVI_CHUNK_RESERVED,
} CHUNK_t;

//AVI Parser states.
typedef enum parserState
{
  //Initial Parser state when parser handle is created.
  AVI_PARSER_IDLE,
  //Parser state when parsing begins.
  AVI_PARSER_INIT,
  //Parser state when parsing is successful.
  AVI_PARSER_READY,
  //Parser is seeking to a given timestamp.
  AVI_PARSER_SEEK,
  //Read failed
  AVI_PARSER_READ_FAILED,
  //Data being read is corrupted.
  AVI_PARSER_FAILED_CORRUPTED_FILE,
  //Parser state it fails to allocate memory,
  //that is, when malloc/new fails.
  AVI_PARSER_NO_MEMORY,
  //Parser state it's ready to retrieve sample information.
  AVI_PARSER_CHUNK_HEADER_START,
  //Pasrser state when chunk data/sample payload is ready
  //to be retrieved.
  AVI_PARSER_CHUNK_DATA_START,
  //Parser state when we need more data during streaming
  AVI_PARSER_DATA_UNDERRUN,
  //Parser state when we need to update samples
  AVI_PARSER_UPDATE_SAMPLES,
  //Parser state when we reach end of file
  AVI_PARSER_END_OF_FILE
}aviParserState;

#endif
