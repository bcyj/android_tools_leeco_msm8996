#ifndef FLV_PARSER_CONSTANTS_DEFN
#define FLV_PARSER_CONSTANTS_DEFN

/* =======================================================================
                              FLVParserConstants.h
DESCRIPTION
Flash Parser constant declarations

Copyright (c) 2012-2014 QUALCOMM Technologies Inc, All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
========================================================================== */

/* =======================================================================
                             PERFORCE HEADER
$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/FLVParserLib/main/latest/inc/flvparserconstants.h#3 $
========================================================================== */

#define FLV_SIG_BYTES_LENGTH          3
#define FLV_VERSION_NO_BYTES          1
#define FLV_VERSION_NO_BYTES          1
#define FLV_HDR_FLAGS_INFO_BYTES      1
#define FLV_DATA_OFFSET_BYTES_LENGTH  4
#define TAG_TYPE_AUDIO                8
#define TAG_TYPE_VIDEO                9
#define TAG_TYPE_SCRIPT_DATA          18
#define FLV_STREAM_ID_BYTES           3
#define FLV_TIMESTAMP_BYTES           3
#define FLV_TAG_TYPES_BYTES           1
#define FLV_TAG_DATASIZE_BYTES        3
#define FLV_TAG_HDR_BYTES             11
#define FLV_TAG_HDR_SIZE              15

#define FLV_METADATA_DOUBLE_TYPE      0x00
#define FLV_METADATA_BOOL_TYPE        0x01
#define FLV_METADATA_STRG_TYPE        0x02
#define FLV_METADATA_DATAOBJ_TYPE     0x03
#define FLV_METADATA_US16_TYPE        0x07
#define FLV_METADATA_ECMA_ARR_TYPE    0x08
#define FLV_METADATA_STRICT_ARR_TYPE  0x0A
#define FLV_METADATA_DATE_TYPE        0x0B
#define FLV_METADATA_LONG_STRG_TYPE   0x0C
const char SCRIPTDATA_OBJECT_END[] = {0x00, 0x00, 0x09};

#define FLV_INDEX_TABLE_ENTRIES      (300)
#define NALU_MIN_SIZE                (7)
#define DEF_AUDIO_BUFF_SIZE          (128000)

#define NEXT_TAG_OFFSET(ullOffset, ulSize) \
                       (ullOffset + ulSize + FLV_TAG_HDR_SIZE)
#endif//#ifndef FLV_PARSER_CONSTANTS_DEFN
