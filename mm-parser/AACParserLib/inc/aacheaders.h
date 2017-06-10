/* =======================================================================

  FILE: aacheaders.h

  SERVICES: Audio

  DESCRIPTION:
  Declarations required for AAC Format parser implementation

  Copyright (c) 2009-2014 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.

$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/AACParserLib/main/latest/inc/aacheaders.h#11 $
$DateTime: 2012/06/26 05:10:49 $
$Change: 2535539 $

========================================================================== */

#ifndef AAC_HEADERS_H
#define AAC_HEADERS_H

#include "aacconstants.h"

typedef struct aac_header_aach_t
{
  uint16 nChannels;      ///< number of channels
  uint32 nSampleRate;    ///< sample rate
  uint32 nBitRate;       //< bit rate
} aac_header_aach;

typedef struct aac_audio_info_t
{
  int32 dwSuggestedBufferSize; // Suggested buffer size for o/p buffer
} aac_audio_info;

typedef struct aac_decode_info_t
{
  uint8 audio_object;
  uint8 aac_subformat_type;
} aac_decode_info;

/// AAC format type
typedef enum{
  AAC_FORMAT_UNKNOWN,
  AAC_FORMAT_ADTS,
  AAC_FORMAT_ADIF,
  AAC_FORMAT_RAW,
  AAC_FORMAT_LOAS,
  AAC_FORMAT_MAX
}aac_format_type;


/// AAC Technical Metadata found in file and frame headers
struct tech_data_aac
{
  boolean is_original; ///< true if this is an original copy
  boolean on_home;     ///< true if home is set
  boolean crc_present; ///< is CRC present in the frame headers
  boolean is_private;  ///< true if private bit is set
  byte channel;        ///< number of channels
  byte audio_object;   ///< audio object type
  byte layer;          ///< mpeg layer
  uint16 sample_rate;  ///< sample rate
  uint32 type;         ///< Format Type
  uint32 bit_rate;     ///< bit rate
  aac_format_type format_type; ///< AAC format type
};

#endif ///<AAC_HEADERS_H
