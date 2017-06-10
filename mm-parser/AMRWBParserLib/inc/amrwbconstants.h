#ifndef AMRWB_CONSTANTS_H
#define AMRWB_CONSTANTS_H

/* =======================================================================
                              amrwbConstants.h
DESCRIPTION

  Copyright (c) 2009-2013 QUALCOMM Technologies Inc, All Rights Reserved.
  QUALCOMM Technologies Proprietary and Confidential.
========================================================================== */

/* =======================================================================
                             PERFORCE HEADER
$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/AMRWBParserLib/main/latest/inc/amrwbconstants.h#8 $
========================================================================== */

// default output buffer size
#define AMRWB_DEFAULT_BUF_SIZE 800

// file i/o buffer size
#define AMRWB_READ_BUFFER_SIZE 1024

//AMRWB File Header size
#define AMRWB_FILE_HEADER_SIZE 9

#define AMRWB_FRAME_HEADER_SIZE 1

#define AMRWB_CACHE_SIZE (AMRWB_DEFAULT_BUF_SIZE * 5)
//AMRWB sampling rate is fixed and it is 16000 samples per second
//(refer 3.1 section of rfc4867)
#define AMRWB_SAMPLE_RATE 16000

#define AMRWB_FRAME_TIME 20

#define AMRWB_FRAME_SIZE 20

#endif
