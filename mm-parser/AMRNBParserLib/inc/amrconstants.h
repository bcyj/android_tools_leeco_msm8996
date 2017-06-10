#ifndef AMR_CONSTANTS_H
#define AMR_CONSTANTS_H

/* =======================================================================
                              amrConstants.h
DESCRIPTION
  Constant used in AMR file parsing.

  Copyright (c) 2009-2013 QUALCOMM Technologies Inc, All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

========================================================================== */

/* =======================================================================
                             PERFORCE HEADER
$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/AMRNBParserLib/main/latest/inc/amrconstants.h#8 $
========================================================================== */

// default output buffer size
#define AMR_DEFAULT_BUF_SIZE 800

// file i/o buffer size
#define AMR_READ_BUFFER_SIZE 1024

//AMR File Header size
#define AMR_FILE_HEADER_SIZE 6

#define AMR_FRAME_HEADER_SIZE 1

#define AMR_CACHE_SIZE (AMR_READ_BUFFER_SIZE * 5)

//AMR sampling rate is fixed and it is 8000 samples per second
//(refer 3.1 section of rfc4867)
#define AMR_SAMPLE_RATE 8000

#define AMR_FRAME_TIME 20

#define AMR_FRAME_SIZE 20

#endif
