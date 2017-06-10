#ifndef MP2_TRANSPORT_STREAM_STATUS_H
#define MP2_TRANSPORT_STREAM_STATUS_H

/* =======================================================================
                              MP2StreamParserStatus.h
DESCRIPTION
  Meaningful description of the definitions contained in this file.
  Description must specify if the module is portable specific, mobile
  specific, or common to both, and it should alert the reader if the
  module contains any conditional definitions which tailors the module to
  different targets.

Copyright (c) 2009-13 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
========================================================================== */

/* =======================================================================
                             PERFORCE HEADER
$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/MP2ParserLib/main/latest/inc/MP2StreamParserStatus.h#8 $
========================================================================== */

typedef enum _Status
{
  MP2STREAM_DEFAULT_ERROR,
  MP2STREAM_CORRUPT_DATA,
  MP2STREAM_INVALID_PARAM,
  MP2STREAM_OUT_OF_MEMORY,
  MP2STREAM_PARSE_ERROR,
  MP2STREAM_DATA_UNDER_RUN,
  MP2STREAM_IDLE,
  MP2STREAM_INIT,
  MP2STREAM_READY,
  MP2STREAM_SEEKING,
  MP2STREAM_READ_ERROR,
  MP2STREAM_SUCCESS,
  MP2STREAM_SKIP_PES_PKT,
  MP2STREAM_EOF,
  MP2STREAM_FAIL,
  MP2STREAM_INSUFFICIENT_MEMORY
}MP2StreamStatus;

#endif
