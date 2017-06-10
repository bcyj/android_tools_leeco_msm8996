#ifndef MP3_HEADERS_H
#define MP3_HEADERS_H

/* =======================================================================
                              mp3Headers.h
DESCRIPTION

Copyright 2009-2013 Qualcomm Technologies, Inc., All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
========================================================================== */

/* =======================================================================
                             PERFORCE HEADER
$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/MP3ParserLib/main/latest/inc/mp3headers.h#10 $
========================================================================== */

/*
* Headers as defined in MP3 specification.
* Refer to MP3 specification for more information
* about each header and it's individual field.
*/
#include "mp3consts.h"
#include "parserdatadef.h"

typedef struct mp3_header_mp3h_t
{
  uint16 nChannels;       ///< number of channels
  uint32 nSampleRate;    ///< sample rate
} mp3_header_mp3h;

typedef struct mp3_audio_info_t
{
  uint32 dwSuggestedBufferSize; // Suggested buffer size for o/p buffer
} mp3_audio_info;

struct mpeg1_tag
{
  uint16  fwHeadLayer;
  uint16  fwHeadMode;
  uint16  fwHeadModeExt;
  uint16  wHeadEmphasis;
  uint16  fwHeadFlags;
  uint32  dwHeadBitrate;
  uint32  dwPTSLow;
  uint32  dwPTSHigh;
};

// size of side information (only for Layer III)
// 1. index = LSF, 2. index = mono
const uint32 MP3_SIDEINFO_SIZES[4][2] =
{
   {17,9},   // MPEG 2.5
   {0,0},    // reserved
   {17,9},   // MPEG 2
   {32,17},  // MPEG 1
};
#endif
