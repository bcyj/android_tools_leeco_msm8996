#ifndef _OGG_STREAM_PARSER_STATUS_H
#define _OGG_STREAM_PARSER_STATUS_H

/* =======================================================================
                              OGGStreamParserStatus.h
DESCRIPTION
  Meaningful description of the definitions contained in this file.
  Description must specify if the module is portable specific, mobile
  specific, or common to both, and it should alert the reader if the
  module contains any conditional definitions which tailors the module to
  different targets.

Copyright 2009-2013 QUALCOMM Technologies Incorporated, All Rights Reserved.
QUALCOMM Technologies Proprietary and Confidential.
========================================================================== */

/* =======================================================================
                             PERFORCE HEADER
$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/OGGParserLib/main/latest/inc/OGGStreamParserStatus.h#4 $
========================================================================== */

typedef enum _Status
{
  OGGSTREAM_DEFAULT_ERROR,
  OGGSTREAM_CORRUPT_DATA,
  OGGSTREAM_PAGE_CRC_ERROR,
  OGGSTREAM_INVALID_PARAM,
  OGGSTREAM_OUT_OF_MEMORY,
  OGGSTREAM_PARSE_ERROR,
  OGGSTREAM_DATA_UNDER_RUN,
  OGGSTREAM_IDLE,
  OGGSTREAM_INIT,
  OGGSTREAM_READY,
  OGGSTREAM_SEEKING,
  OGGSTREAM_READ_ERROR,
  OGGSTREAM_SUCCESS,
  OGGSTREAM_EOF,
  OGGSTREAM_FAIL
}OGGStreamStatus;

#endif//#ifndef _OGG_STREAM_PARSER_STATUS_H

