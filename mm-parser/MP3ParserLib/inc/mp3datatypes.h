#ifndef MP3_HDR_DATA_TYPES_H
#define MP3_HDR_DATA_TYPES_H

/* =======================================================================
                              mp3datatypes.h
DESCRIPTION

Copyright 2011-2013 Qualcomm Technologies, Inc., All Rights Reserved
========================================================================== */

/* =======================================================================
                             PERFORCE HEADER
$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/MP3ParserLib/main/latest/inc/mp3datatypes.h#7 $
========================================================================== */
#include "parserdatadef.h"

//Data types used
typedef unsigned int        mp3_uint32;
typedef int                 mp3_int32;
typedef short               mp3_int16;
typedef unsigned short      mp3_uint16;
typedef unsigned char       mp3_uint8;
typedef char                mp3_int8;
typedef signed long long    mp3_int64;
typedef unsigned long long  mp3_uint64;
typedef mp3_uint64          MP3_FILE_OFFSET;
typedef unsigned char       mp3_boolean;

//MP3 Parser states.
typedef enum mp3t_parserState
{
  //Initial Parser state when parser handle is created.
  MP3_PARSER_IDLE,
  //Parser state when parsing begins.
  MP3_PARSER_INIT,
  //Parser state when parsing is successful.
  MP3_PARSER_READY,
  //Parser is seeking to a given timestamp.
  MP3_PARSER_SEEK,
  //Read failed
  MP3_PARSER_READ_FAILED,
  //Data being read is corrupted.
  MP3_PARSER_FAILED_CORRUPTED_FILE,
  //Parser state it fails to allocate memory,
  //that is, when malloc/new fails.
  MP3_PARSER_NO_MEMORY,
  //Parser state if it does not have enough data to process.
  MP3_PARSER_UNDERRUN
}mp3ParserState;

// size of side information (only for Layer III)
// 1. index = LSF, 2. index = mono
const mp3_uint32 MP3_SIDEINFO_SIZES[4][2] =
{
   {17,9},   // MPEG 2.5
   {0,0},    // reserved
   {17,9},   // MPEG 2
   {32,17},  // MPEG 1
};
#endif
