#ifndef _FLAC_PARSER_STATUS_H
#define _FLAC_PARSER_STATUS_H

/* =======================================================================
                              FlacParserStatus.h
DESCRIPTION
  Meaningful description of the definitions contained in this file.
  Description must specify if the module is portable specific, mobile
  specific, or common to both, and it should alert the reader if the
  module contains any conditional definitions which tailors the module to
  different targets.

  Copyright(c) 2009-2013 by Qualcomm Technologies, Inc. All Rights Reserved
  Qualcomm Technologies Proprietary and Confidential.
========================================================================== */

/* =======================================================================
                             PERFORCE HEADER
$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/FlacParserLib/main/latest/inc/FlacParserStatus.h#2 $
========================================================================== */

typedef enum flac_status_t
{
  FLACPARSER_DEFAULT_ERROR,
  FLACPARSER_CORRUPT_DATA,
  FLACPARSER_INVALID_PARAM,
  FLACPARSER_OUT_OF_MEMORY,
  FLACPARSER_PARSE_ERROR,
  FLACPARSER_DATA_UNDER_RUN,
  FLACPARSER_IDLE,
  FLACPARSER_INIT,
  FLACPARSER_READY,
  FLACPARSER_SEEKING,
  FLACPARSER_READ_ERROR,
  FLACPARSER_SUCCESS,
  FLACPARSER_EOF,
  FLACPARSER_FAIL
}FlacParserStatus;

#endif//#ifndef _FLAC_PARSER_STATUS_H

