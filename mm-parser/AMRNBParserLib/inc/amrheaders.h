#ifndef AMR_HEADERS_H
#define AMR_HEADERS_H

/* =======================================================================
                              amrHeaders.h
DESCRIPTION

  Copyright (c) 2009-2013 QUALCOMM Technologies Inc, All Rights Reserved.
  QUALCOMM Technologies Proprietary and Confidential.

========================================================================== */

/* =======================================================================
                             PERFORCE HEADER
$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/AMRNBParserLib/main/latest/inc/amrheaders.h#7 $
========================================================================== */

/*
* Headers as defined in AMR specification.
* Refer to AMR specification for more information
* about each header and it's individual field.
*/
#include "amrconstants.h"

typedef struct amr_header_amrh_t
{
  uint16 nChannels;       ///< number of channels
  uint32 nSampleRate;    ///< sample rate
} amr_header_amrh;

typedef struct amr_audio_info_t
{
  int32 dwSuggestedBufferSize; // Suggested buffer size for o/p buffer
} amr_audio_info;

typedef struct amr_header {
  uint8 frame_type; //frame type
  uint8 frame_quality_indicator; //frame quality indicator
  uint32 frame_size; //size of frame
  uint32 frame_time; //duration of frame
}amr_header_type;

typedef enum{
    UNKNOWN
   ,AMR_NB_FS //AMR Narrow band File Storage
   ,AMR_WB //AMR Wide band File Storage
   ,AMR_MC //AMR Narrow band Multichannel
   ,AMR_WB_MC //AMR Wide band Multichannel
}amr_format_type;

namespace AUDIO_AMR_NB_FS {
   const unsigned char FRAME_TYPE_MASK = 0x78;
   const int FRAME_TYPE_BYTE_OFFSET = 3;
   const unsigned char FRAME_QUALITY_MASK = 0x04;
   const int FRAME_QUALITY_BYTE_OFFSET = 2;
}

#endif

