#ifndef _MKAV_PARSER_STATUS_H
#define _MKAV_PARSER_STATUS_H

/* =======================================================================
                              MKAVParserStatus.h
DESCRIPTION
  Meaningful description of the definitions contained in this file.
  Description must specify if the module is portable specific, mobile
  specific, or common to both, and it should alert the reader if the
  module contains any conditional definitions which tailors the module to
  different targets.

Copyright (c) 2011-2013 QUALCOMM Technologies Inc, All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
========================================================================== */

/* =======================================================================
                             PERFORCE HEADER
$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/MKAVParserLib/main/latest/inc/mkavparserstatus.h#9 $
========================================================================== */

typedef enum _mkav_parser_state_
{
  MKAVPARSER_INIT,
  MKAVPARSER_LOCATE_EBML_DOC_HDR,
  MKAVPARSER_PARSE_EBML_DOC_HDR,
  MKAVPARSER_LOCATE_SEGMENT_HDR,
  MKAVPARSER_PARSE_SEGMENT_HDR,
  MKAVPARSER_PARSE_SEGMENT_INFO_HDR,
  MKAVPARSER_PARSE_SEEK_HEAD_HDR,
  MKAVPARSER_PARSE_CLUSTER_HDR,
  MKAVPARSER_PARSE_TRACKS_HDR,
  MKAVPARSER_PARSE_CUES_HDR,
  MKAVPARSER_SKIP_EBML_ID,
  MKAVPARSER_READ_DATA,
  MKAVPARSER_READY,
  MKAVPARSER_SEEKING,
  MKAVPARSER_SAMPLE_RETRIEVAL,
  MKAVPARSER_DEFAULT_ERROR,
  MKAVPARSER_OUT_OF_MEMORY,
  MKAVPARSER_DATA_UNDERRUN,
  MKAVPARSER_INSUFFICIENT_BUFFER,
  MKAVPARSER_READ_ERROR,
  MKAVPARSER_EOF,
  MKAVPARSER_PARSE_ERROR
}MKAV_PARSER_STATE;

typedef enum _api_status_
{
  MKAV_API_SUCCESS,
  MKAV_API_INVALID_PARAM,
  MKAV_API_OUT_OF_MEMORY,
  MKAV_API_INSUFFICIENT_BUFFER,
  MKAV_API_EOF,
  MKAV_API_READ_FAIL,
  MKAV_API_DATA_UNDERRUN,
  MKAV_API_FAIL
}MKAV_API_STATUS;

#endif//#ifndef _MKAV_PARSER_STATUS_H

