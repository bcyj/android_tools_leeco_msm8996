/* =======================================================================
                              parserinternaldefs.h
DESCRIPTION
  Meaningful description of the definitions contained in this file.
  Description must specify if the module is portable specific, mobile
  specific, or common to both, and it should alert the reader if the
  module contains any conditional definitions which tailors the module to
  different targets.  Include any initialization and synchronizing
  requirements.

EXTERNALIZED FUNCTIONS
  List functions and a brief description that are exported from this file

INITIALIZATION AND SEQUENCING REQUIREMENTS
  Detail how to initialize and use this service.  The sequencing aspect
  is only needed if the order of operations is important.

Copyright 2009,2014 Qualcomm Technologies, Inc., All Rights Reserved
========================================================================== */

/* =======================================================================
                             PERFORCE HEADER
$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/FileBaseLib/rel/1.11/inc/parserinternaldefs.h#5 $
========================================================================== */


#ifndef _FILE_SOURCE_INTERNAL_DEFS_H_
#define _FILE_SOURCE_INTERNAL_DEFS_H_

#ifndef _ANDROID_
// Pradnya
#include "target.h"
#endif

/* Enable support for file fragmentation */
#define FEATURE_FILESOURCE_FILE_FRAGMENTATION

//#define USE_EFS_FILEIO
#ifndef _ANDROID_
#define FEATURE_FILESOURCE_MP3
#define FEATURE_FILESOURCE_AAC
#define FEATURE_FILESOURCE_AMR
#define FEATURE_FILESOURCE_AMRWB
#define FEATURE_FILESOURCE_EVRC_WB
#define FEATURE_FILESOURCE_EVRC_B
#define FEATURE_FILESOURCE_QCP
#define FEATURE_FILESOURCE_WAVADPCM
#define FEATURE_FILESOURCE_3GP_PARSER
#endif

#ifdef FEATURE_FILESOURCE_FILE_FRAGMENTATION

  //#define FEATURE_FILESOURCE_PSEUDO_STREAM
  #define FEATURE_FILESOURCE_INTER_FRAG_REPOS

  #ifdef FEATURE_FILESOURCE_INTER_FRAG_REPOS
    #define FEATURE_FILESOURCE_RANDOM_ACCESS_REPOS
  #endif  /*FEATURE_FILESOURCE_INTER_FRAG_REPOS */

#endif  /* FEATURE_FILESOURCE_FILE_FRAGMENTATION */

#define FEATURE_FILESOURCE_3GPP_PROGRESSIVE_DNLD

// Pradnya
/*AVI FILE SUPPORT*/
#ifndef _ANDROID_
#ifndef T_MSM8650 /* AVI/DIVX is not needed for 8K yet */
#define FEATURE_FILESOURCE_AVI
#define FEATURE_FILESOURCE_AVI_DIVX_PARSER
#define FEATURE_FILESOURCE_DIVX_DRM
#endif

#else
#ifdef FEATURE_FILESOURCE_AVI
#define FILESOURCE_AVI_BAD_CHUNK_LIMIT 1000
#define FEATURE_FILESOURCE_AVI_DIVX_PARSER
#define FEATURE_FILESOURCE_AVI_PARSER_AUDIO_FRAME_BOUNDARY
#define AVI_PARSER_FAST_START_UP
#undef FEATURE_FILESOURCE_DIVX_DRM
#else
#undef FEATURE_FILESOURCE_DIVX_DRM
#undef FEATURE_FILESOURCE_AVI_DIVX_PARSER
#endif
/*Define this when WM DRM code is mainlined */
#undef FEATURE_FILESOURCE_WMDRM
#endif //_ANDROID_

#ifdef FEATURE_FILESOURCE_MKV_PARSER
#define FEATURE_DIVXHD_PLUS
#endif
/* enabled for MPEG-4 B-Frame support, and DivX 3.11 */
#define FEATURE_DIVX_311_ENABLE

#ifdef FEATURE_ICONTENTHANDLER
  #define FEATURE_FILESOURCE_DRM_DCF
#endif

#ifndef FEATURE_FILESOURCE_FLV
  #define FEATURE_FILESOURCE_FLV
#endif

#ifndef FEATURE_FILESOURCE_OGG_PARSER
  #define FEATURE_FILESOURCE_OGG_PARSER
#endif

//Define FLAC parser as well if OGG Parser enabled
#ifdef FEATURE_FILESOURCE_OGG_PARSER
#define FEATURE_FILESOURCE_FLAC_PARSER
#endif

#ifndef FEATURE_FILESOURCE_MP3
  #define FEATURE_FILESOURCE_MP3
#endif

#ifndef FEATURE_FILESOURCE_AC3
  #define FEATURE_FILESOURCE_AC3
#endif

#ifndef FEATURE_FILESOURCE_DTS
  #define FEATURE_FILESOURCE_DTS
#endif

// Pradnya
#ifndef _ANDROID_
#define FEATURE_FILESOURCE_MPEG2_PARSER

// Pradnya: comment out features that are not supported
/* Enable Windows Media */
#define FEATURE_FILESOURCE_WINDOWS_MEDIA
#define FEATURE_WMA
#endif // _ANDROID_

/*Enable ARM/DSP based WMA PRO decoder.*/
#define FEATURE_FILESOURCE_WMA_PRO_DSP_DECODER

/*Following defines are needed for WMA Pro decoder
  irrespective of FEATURE_FILESOURCE_WMA_PRO_DECODER
  being turned ON/OFF.
  Without following compile time switches,
  there might be a compilation problem.
*/
#ifdef FEATURE_FILESOURCE_WINDOWS_MEDIA
#define _CONSOLE
#define BITSTREAM_CORRUPTION_ASSERTS
#define WMA_DECPK_BUILD
#define WMAAPI_NO_DRM
#define WMAAPI_NO_DRM_STREAM
#define DISABLE_UES
#define _CRT_NON_CONFORMING_SWPRINTFS
#ifndef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE
#endif
/* Define Windows Media compiler switches */
#define __SUPPORT__WMVP__
#define _SLIM_C_
#define __NO_SPEECH__
#define OUTPUT_ALL_FORMATS
#define WMA_RC_NOTSUPPORTED
#define WMA_OWN_GetMoreData
#define ENABLE_LPC
#define ENABLE_ALL_ENCOPT
#define __NO_SCREEN__
#define BUILD_INTEGER
#define WMFAPI_NO_DRM
#define WMCAPI_NO_DRM
#define __STREAMING_MODE_DECODE_
#define DISABLE_OPT
#define __SLIM_WMV9__
#define OUTPUT_STANDARD_WMA_BITSTREAM
//#define FEATURE_FILESOURCE_WM_DRM_API
#endif

#endif

