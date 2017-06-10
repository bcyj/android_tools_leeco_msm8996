#ifndef FLAC_PARSER_CONSTANTS_DEFN
#define FLAC_PARSER_CONSTANTS_DEFN

/* =======================================================================
                              OGGStreamParserConstants.h
DESCRIPTION
  Meaningful description of the definitions contained in this file.
  Description must specify if the module is portable specific, mobile
  specific, or common to both, and it should alert the reader if the
  module contains any conditional definitions which tailors the module to
  different targets.

  Copyright(c) 2009-2015 by Qualcomm Technologies, Inc. All Rights Reserved
  Qualcomm Technologies Proprietary and Confidential.
========================================================================== */

/* =======================================================================
                             PERFORCE HEADER
$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/FlacParserLib/main/latest/inc/FlacParserConstants.h#10 $
========================================================================== */

#include "AEEStdDef.h"

#define FLAC_FAST_STARTUP
#define FLAC_APPROX_SEEK_MULTIPASS

#ifdef FLAC_APPROX_SEEK_MULTIPASS
#define FLAC_APPROX_SEEK_MAX_PASS 4
#define FLAC_APPROX_SEEK_MAX_DEVIATION_MS 2000
#endif

#define FLAC_VALID_FRAME_DETECT_THRESHOLD_MS       8000
#define FLAC_SIGNATURE_BYTES                       "fLaC"
#define FLAC_SIGNATURE_SIZE                        4

#define FLAC_METADATA_BLOCK_TYPE_BYTES             1
#define FLAC_METADATA_BLOCK_SIZE_BYTES             3

#define METADATA_BLOCK_STREAMINFO_TYPE             0x00
#define METADATA_BLOCK_PADDING_TYPE                0x01
#define METADATA_BLOCK_APPLICATION_TYPE            0x02
#define METADATA_BLOCK_SEEKTABLE_TYPE              0x03
#define METADATA_BLOCK_VORBIS_COMMENT_TYPE         0x04
#define METADATA_BLOCK_CUESHEET_TYPE               0x05
#define METADATA_BLOCK_PICTURE_TYPE                0x06
#define METADATA_BLOCK_UNKNOWN_TYPE_BEG            0x07
#define METADATA_BLOCK_UNKNOWN_TYPE_END            0x7F
#define MD5_SIG_SIZE                               16

#define FLAC_FRAME_HDR_SYNC_CODE                   0x3FFE

#define FLAC_FRAME_HDR_FIXED_BLOCKSIZE             0x00
#define FLAC_FRAME_HDR_VARIABLE_BLOCKSIZE          0x01
#define FLAC_PARSER_BUFFER_SIZE                   (65536)
#define FLAC_PARSER_SUPERSET_BUFFER_SIZE           260000
#define FLAC_PARSER_BLOCK_LEN_BUFFER_THRESHOLD     14000

#define SEEK_POINT_SIZE_IN_BYTES                   18

#define FLAC_MAX_HEADER_BYTES                      16

#define FLAC_MAX_FIELDNAMES_SUPPORTED  17

#endif//#ifndef FLAC_PARSER_CONSTANTS_DEFN

