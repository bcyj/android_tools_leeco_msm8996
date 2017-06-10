#ifndef AMRWB_HEADERS_H
#define AMRWB_HEADERS_H

/* =======================================================================
                              amrwbHeaders.h
DESCRIPTION

  Copyright (c) 2009-2013 QUALCOMM Technologies Inc, All Rights Reserved.
  QUALCOMM Technologies Proprietary and Confidential.
========================================================================== */

/* =======================================================================
                             PERFORCE HEADER
$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/AMRWBParserLib/main/latest/inc/amrwbheaders.h#7 $
========================================================================== */

/*
* Headers as defined in AMRWB specification.
* Refer to AMRWB specification for more information
* about each header and it's individual field.
*/
#include "amrwbconstants.h"

typedef struct amrwb_header_amrwbh_t
{
  uint16 nChannels;      ///< number of channels
  uint32 nSampleRate;    ///< sample rate
} amrwb_header_amrwbh;

typedef struct amrwb_audio_info_t
{
  int32 dwSuggestedBufferSize; // Suggested buffer size for o/p buffer
} amrwb_audio_info;

typedef struct amrwb_header {
  uint8 frame_type; //frame type
  uint8 frame_quality_indicator; //frame quality indicator
  uint32 frame_size; //size of frame
  uint32 frame_time; //duration of frame
}amrwb_header_type;

typedef enum{
    AMRWB_UNKNOWN
   ,AMRWB_SC_FS //AMRWB Wide band File Storage
   ,AMRWB_MC_FS //AMRWB Wide band Multichannel
}amrwb_format_type;

namespace AUDIO_AMRWB_FS {
   const unsigned char FRAME_TYPE_MASK = 0x78;
   const int FRAME_TYPE_BYTE_OFFSET = 3;
   const unsigned char FRAME_QUALITY_MASK = 0x04;
   const int FRAME_QUALITY_BYTE_OFFSET = 2;
}

#endif

