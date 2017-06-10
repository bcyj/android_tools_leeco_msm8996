#ifndef VIDEO_COMMON_H
#define VIDEO_COMMON_H
/*===========================================================================

              V I D E O   C O M M O N   D E F I N I T I O N S

DESCRIPTION
  This header file contains all the definitions common among the various video
  modules, such as the video encoder interface and video encode engine.

  Copyright(c) 2008-2013 by Qualcomm Technologies, Inc. All Rights Reserved
  Qualcomm Technologies Proprietary and Confidential.
===========================================================================*/

/*===========================================================================

                      EDIT HISTORY FOR FILE

  This section contains comments describing changes made to this file.
  Notice that changes are listed in reverse chronological order.

  $Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/VideoFMTReaderLib/main/latest/inc/video_common.h#18 $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
06/29/07   kadhir  Added max and dynamic bitrate support for camcorder recording
11/09/06   jk      Added VIDEO_ALLOC and MM_Free.
04/03/06   wyh     Remove old video types that are no longer valid.
01/18/06   sagars  Make parameter filename in video_handle_type_filename
                   const char*.
12/14/05   sagars  Added video_handle_type_fname to remove compile time
                   dependency of QVP codec library on FS_FILENAME_MAX_LENGTH_P.
12/09/05   rpw     Added AMR-WB and AMR-WB+ support
11/14/05   Shahed New implementation of camcorder space status error/warning msgs.
11/11/05   Shahed Undo SPACE error/warning changes.
11/08/05   Shahed  Added new members in video_status_type enum for detailed
                   SPACE warning/error message support.
07/18/05   jk      Added VIDEO_ENC_FOOTER.
                   Added variable bitrate audio support.
02/07/05   rpw     Added still image support.
                   Added audio only support.
                   Added NTT DoCoMo DRM atom support.
                   Added capability of disabling interlacing.
                   Added file size limit feature.
                   Fixed file limit reached logic.
                   Now collapsing all 'mdat' atoms together when recording.
                   Added mode selection to select bitrate for AMR recording.
                   Replaced all TASKLOCK/TASKFREE with REX critical sections.
11/18/04   jk      Changed #ifdef T_MSM6550A to #ifdef T_MSM6550
10/21/04   jk      Revived 3 quality profiles conditionally.
10/01/04   jk      Added VIDEO_ENC_PARTIAL_FRAME, VIDEO_ENC_HEADER status
                   type and created video_frame_type.
                   Added VIDEO_ENC_USER_DEFINED profile type and removed
                   3 quality profiles.
04/21/04   rpw     Added VIDEO_FMT_BRAND_SKM brand type.
10/27/03   rpw     Added support for KDDI fragmented file version of 3G2.
09/16/03   rpw     Replaced VIDEO_FMT_STREAM_AUDIO_MP3 with
                   VIDEO_FMT_STREAM_AUDIO_MPEG1_L3 and
                   VIDEO_FMT_STREAM_AUDIO_MPEG2_L3.
08/04/03   rpw     Moved to services/videofmt from services/camcorder.
07/31/03   rpw     Added video_fmt_brand_type.
07/29/03   rpw     Added VIDEO_FMT_STREAM_AUDIO_EVRC_PV to distinguish
                   between EVRC streams in 3GPP2 and PacketVideo's variant
                   which uses AudioSampleEntry with a special OTI value.
06/03/03   rpw     Added code to recognize MPEG-4 video short header format
                   as H.263 content.  Added type for MPEG-4 AAC audio.
05/07/03   rpw     Added status codes VIDEO_LIMIT_NEAR and
                   VIDEO_LIMIT_REACHED for use by the engine to indicate that
                   it is about to or has reached resource limits during
                   recording.
03/20/03   sj      Updated video_profile_type to support Rate Control
02/17/03   rpw     Expanded QCELP audio into full- and half-rate options
11/04/02   rpw     Created file.

===========================================================================*/

/* <EJECT> */
/*===========================================================================

                      INCLUDE FILES FOR MODULE

===========================================================================*/
#ifdef __cplusplus
   extern "C" {
#endif

#include "AEEStdDef.h"              /* Definitions for byte, word, etc.        */
#include "MMDebugMsg.h"

#define VIDEO_FILE_FILENAME_MAX_LENGTH 2048 /*TODO, should be removed from our file
                                          when it gets included in next revision
                                          of OS Abstraction Layer*/

/* <EJECT> */
/*===========================================================================

                        DATA DECLARATIONS

===========================================================================*/

/* This enumerated type lists all the possible status conditions returned by
** video callback functions.
*/
typedef enum {
  VIDEO_SUCCESS,            /* Requested command was accepted        */
  VIDEO_DONE,               /* Command has been carried out          */
  VIDEO_ENC_FRAME,          /* Encoded frame data                    */
  VIDEO_ENC_PARTIAL_FRAME,  /* Encoded partial frame data            */
  VIDEO_ENC_HEADER,         /* Encoded decoder specific header       */
  VIDEO_ENC_FOOTER,         /* Encoded video footer                  */
  VIDEO_ENABLED,            /* Status is enabled                     */
  VIDEO_DISABLED,           /* Status is disabled                    */
  VIDEO_ABORTED,            /* Command was aborted                   */
  VIDEO_PAUSED,             /* Video processing is currently paused  */
  VIDEO_RESUME,             /* Video processing has resumed          */
  VIDEO_ERROR,              /* Error occurred with requested command */
  VIDEO_FAILURE,            /* Error occurred with requested command */
  VIDEO_LIMIT_NEAR,         /* Engine running low on space           */
  VIDEO_LIMIT_NEAR_SAMPLE_TABLE,
  VIDEO_LIMIT_NEAR_CHUNK_TABLE,
  VIDEO_LIMIT_NEAR_STORAGE,
  VIDEO_LIMIT_NEAR_MOVIE_DURATION,
  VIDEO_LIMIT_IMMINENT,     /* Engine about to run out of space      */
  VIDEO_LIMIT_IMMINENT_SAMPLE_TABLE,
  VIDEO_LIMIT_IMMINENT_CHUNK_TABLE,
  VIDEO_LIMIT_IMMINENT_STORAGE,
  VIDEO_LIMIT_IMMINENT_MOVIE_DURATION,
  VIDEO_LIMIT_REACHED,      /* Engine out of space - record stopped  */
  VIDEO_LIMIT_REACHED_CHUNK_TABLE,
  VIDEO_LIMIT_REACHED_SAMPLE_TABLE,
  VIDEO_LIMIT_REACHED_MOVIE_DURATION,
  VIDEO_LIMIT_REACHED_STORAGE,
  VIDEO_ALLOC,
  VIDEO_FREE
} video_status_type;

typedef enum
{
   VIDEO_FMT_SPACE_MSG_INFO_UNAVAILABLE,
   VIDEO_FMT_SPACE_MSG_SAMPLE_TABLE,
   VIDEO_FMT_SPACE_MSG_CHUNK_TABLE,
   VIDEO_FMT_SPACE_MSG_STORAGE,
   VIDEO_FMT_SPACE_MSG_MOVIE_DURATION
} video_fmt_space_out_type;

/* This enumerated type is used to select a file format. */
typedef enum {
  VIDEO_FMT_MP4,          /* ISO/IEC 14496-1                  */
  VIDEO_FMT_MP4_DASH,     /* ISO/IEC 14496-12 (third edition) */
  VIDEO_FMT_INVALID       /* invalid file format              */
} video_fmt_type;

/* This enumerated type is used to select a particular brand or
** variation in a file format.
*/
typedef enum {
  VIDEO_FMT_BRAND_MP4,      /* strictly ISO/IEC 14496-1             */
  VIDEO_FMT_BRAND_AMC,      /* KDDI AMC (movie mail) format         */
  VIDEO_FMT_BRAND_3GP,      /* 3GPP (3GPP TS 26.234 v5.6.0)         */
  VIDEO_FMT_BRAND_3G2,      /* 3GPP2 (3GPP2 C.P0050-0)              */
  VIDEO_FMT_BRAND_K3G,      /* KWIF (KWISF.K-02-001)                */
  VIDEO_FMT_BRAND_FRAG_3G2, /* Fragmented 3G2 (KDDI Movie File      */
                            /*   Format Specification for cdma2000  */
                            /*   1x/1xEV-DO v1.1.0)                 */
  VIDEO_FMT_BRAND_SKM,      /* SK Telecom VOD Serv Spec 2.3 draft2  */
  VIDEO_FMT_BRAND_INVALID   /* invalid brand                        */
} video_fmt_brand_type;

/* This enumerated type lists the different types of video streams. */
typedef enum {
  VIDEO_FMT_STREAM_VIDEO_NONE,        /* flag indicating no video       */
  VIDEO_FMT_STREAM_VIDEO_MPEG4,       /* ISO/IEC 14496-2                */
  VIDEO_FMT_STREAM_VIDEO_H263,        /* H.263                          */
  VIDEO_FMT_STREAM_VIDEO_H264,        /* H.264                          */
  VIDEO_FMT_STREAM_VIDEO_HEVC,        /* HEVC                           */
  VIDEO_FMT_STREAM_VIDEO_JPEG,        /* SKT-MOD JPEG                   */
  VIDEO_FMT_STREAM_VIDEO_BMP,         /* SKT-MOD BMP                    */
  VIDEO_FMT_STREAM_VIDEO_STILL_IMAGE, /* PV Specific Still Image        */
  VIDEO_FMT_STREAM_VIDEO_UNK,         /* unknown type of video          */
  VIDEO_FMT_STREAM_VIDEO_INVALID      /* invalid video stream type      */
} video_fmt_stream_video_type;

/* This enumerated type lists the different types of audio streams. */
typedef enum {
  VIDEO_FMT_STREAM_AUDIO_NONE,           /* flag indicating no audio       */
  VIDEO_FMT_STREAM_AUDIO_QCELP13K_FULL,  /* PureVoice QCELP-13K fixed full */
  VIDEO_FMT_STREAM_AUDIO_QCELP13K_HALF,  /* PureVoice QCELP-13K fixed half */
  VIDEO_FMT_STREAM_AUDIO_EVRC,           /* Enhanced Variable Rate Codec   */
  VIDEO_FMT_STREAM_AUDIO_EVRC_PV,        /* EVRC, PacketVideo variant      */
  VIDEO_FMT_STREAM_AUDIO_AMR,            /* GSM Adaptive Multi Rate codec  */
  VIDEO_FMT_STREAM_AUDIO_MPEG1_L3,       /* MPEG-1, Layer 3 (MP3) codec    */
  VIDEO_FMT_STREAM_AUDIO_MPEG2_L3,       /* MPEG-2, Layer 3 (MP3) codec    */
  VIDEO_FMT_STREAM_AUDIO_MPEG2_AAC,      /* MPEG-2, Advanced Audio Codec   */
  VIDEO_FMT_STREAM_AUDIO_MPEG4_AAC,      /* MPEG-4 Advanced Audio Codec    */
  VIDEO_FMT_STREAM_AUDIO_PUREVOICE,      /* 3GPP2 QCELP                    */
  VIDEO_FMT_STREAM_AUDIO_AMR_WB,         /* AMR Wideband codec (TS 26.171) */
  VIDEO_FMT_STREAM_AUDIO_AMR_WB_PLUS,    /* Extended AMR WB (TS 26.290)    */
  VIDEO_FMT_STREAM_AUDIO_EVRC_B,         /* EVRC-B type codec */
  VIDEO_FMT_STREAM_AUDIO_EVRC_WB,        /* EVRC-WB type codec */
  VIDEO_FMT_STREAM_AUDIO_AC3,            /* AC3 codec (ETSI TS 102 366)    */
  VIDEO_FMT_STREAM_AUDIO_EAC3,            /* EC3 codec (ETSI TS 102 366)           */
  VIDEO_FMT_STREAM_AUDIO_EAC3_JOC,        /* EC3-JOC codec (ETSI TS 102 366v1.3.1) */
  VIDEO_FMT_STREAM_AUDIO_DTS_CORE,       /* DTS-CORE Audio formats         */
  VIDEO_FMT_STREAM_AUDIO_DTS_HD,         /* DTS_HD audio format            */
  VIDEO_FMT_STREAM_AUDIO_DTS_HD_LOSSLESS,/* DTS-HD lossless audio format   */
  VIDEO_FMT_STREAM_AUDIO_DTS_LBR,        /* DTS-LBR(low bit rate) audio    */
  VIDEO_FMT_STREAM_AUDIO_UNK,            /* Unknown type of audio          */
  VIDEO_FMT_STREAM_AUDIO_INVALID         /* invalid audio stream type      */
} video_fmt_stream_audio_type;

/* This enumerated type lists the different types of text streams. */
typedef enum {
  VIDEO_FMT_STREAM_TEXT_NONE,        /* flag indicating no text   */
  VIDEO_FMT_STREAM_TEXT_TIMEEDTEXT,  /* 3GPP TS 26.245 Timed Text */
  VIDEO_FMT_STREAM_TEXT_SMPTE_TIMED_TEXT,    /* SMPTE ST 2052-1:2010      */
  VIDEO_FMT_STREAM_TEXT_UNK,         /* unknown type of video     */
  VIDEO_FMT_STREAM_TEXT_INVALID      /* invalid video stream type */
} video_fmt_stream_text_type;

/* These values are used to indicate rate sets for various audio formats. */
#define VIDEO_FMT_RATE_SET_AMR_1220       0      /* AMR 12.2 kbps */
#define VIDEO_FMT_RATE_SET_AMR_1020       1      /* AMR 10.2 kbps */
#define VIDEO_FMT_RATE_SET_AMR_0795       2      /* AMR 7.95 kbps */
#define VIDEO_FMT_RATE_SET_AMR_0740       3      /* AMR 7.4 kbps */
#define VIDEO_FMT_RATE_SET_AMR_0670       4      /* AMR 6.7 kbps */
#define VIDEO_FMT_RATE_SET_AMR_0590       5      /* AMR 5.9 kbps */
#define VIDEO_FMT_RATE_SET_AMR_0515       6      /* AMR 5.15 kbps */
#define VIDEO_FMT_RATE_SET_AMR_0475       7      /* AMR 4.75 kbps */

#define VIDEO_FMT_RATE_SET_AMRWB_2385     0      /* AMR-WB 23.85 kbps */
#define VIDEO_FMT_RATE_SET_AMRWB_2305     1      /* AMR-WB 23.05 kbps */
#define VIDEO_FMT_RATE_SET_AMRWB_1985     2      /* AMR-WB 19.85 kbps */
#define VIDEO_FMT_RATE_SET_AMRWB_1825     3      /* AMR-WB 18.25 kbps */
#define VIDEO_FMT_RATE_SET_AMRWB_1585     4      /* AMR-WB 15.85 kbps */
#define VIDEO_FMT_RATE_SET_AMRWB_1425     5      /* AMR-WB 14.25 kbps */
#define VIDEO_FMT_RATE_SET_AMRWB_1265     6      /* AMR-WB 12.65 kbps */
#define VIDEO_FMT_RATE_SET_AMRWB_0885     7      /* AMR-WB 8.85 kbps */
#define VIDEO_FMT_RATE_SET_AMRWB_0660     8      /* AMR-WB 6.60 kbps */

/* NOTE: AMR-WB+ is an extension to AMR-WB, so AMR-WB rate sets are included
**       in AMR-WB+, which is why we start at 9 here and not at 0.
*/
#define VIDEO_FMT_RATE_SET_AMRWBP_2400_S  9      /* AMR-WB+ 24 kbps stereo */
#define VIDEO_FMT_RATE_SET_AMRWBP_2400_M  10     /* AMR-WB+ 24 kbps mono */
#define VIDEO_FMT_RATE_SET_AMRWBP_1800_S  11     /* AMR-WB+ 18 kbps stereo */
#define VIDEO_FMT_RATE_SET_AMRWBP_1360_M  12     /* AMR-WB+ 13.6 kbps mono */
#define VIDEO_FMT_RATE_SET_AMRWBP_1360_M  12     /* AMR-WB+ 13.6 kbps mono */

/* These rate sets go with AMR-WB+ but are for the modes where the internal
** sampling frequency is not fixed at 25600Hz.  They are distinguished from
** the previous AMR-WB+ modes by the additional 'E' after 'AMRWBP'.
*/
#define VIDEO_FMT_RATE_SET_AMRWBPE_3200_S 13     /* AMR-WB+ 32 kbps stereo */
#define VIDEO_FMT_RATE_SET_AMRWBPE_3000_S 14     /* AMR-WB+ 30 kbps stereo */
#define VIDEO_FMT_RATE_SET_AMRWBPE_2960_S 15     /* AMR-WB+ 29.6 kbps stereo */
#define VIDEO_FMT_RATE_SET_AMRWBPE_2880_S 16     /* AMR-WB+ 28.8 kbps stereo */
#define VIDEO_FMT_RATE_SET_AMRWBPE_2680_S 17     /* AMR-WB+ 26.8 kbps stereo */
#define VIDEO_FMT_RATE_SET_AMRWBPE_2600_S 18     /* AMR-WB+ 26 kbps stereo */
#define VIDEO_FMT_RATE_SET_AMRWBPE_2560_S 19     /* AMR-WB+ 25.6 kbps stereo */
#define VIDEO_FMT_RATE_SET_AMRWBPE_2400_S 20     /* AMR-WB+ 24 kbps stereo */
#define VIDEO_FMT_RATE_SET_AMRWBPE_2320_S 21     /* AMR-WB+ 23.2 kbps stereo */
#define VIDEO_FMT_RATE_SET_AMRWBPE_2240_S 22     /* AMR-WB+ 22.4 kbps stereo */
#define VIDEO_FMT_RATE_SET_AMRWBPE_2120_S 23     /* AMR-WB+ 21.2 kbps stereo */
#define VIDEO_FMT_RATE_SET_AMRWBPE_2040_S 24     /* AMR-WB+ 20.4 kbps stereo */
#define VIDEO_FMT_RATE_SET_AMRWBPE_2000_S 25      /* AMR-WB+ 20 kbps stereo */
#define VIDEO_FMT_RATE_SET_AMRWBPE_1920_S 26     /* AMR-WB+ 19.2 kbps stereo */
#define VIDEO_FMT_RATE_SET_AMRWBPE_1840_S 27     /* AMR-WB+ 18.4 kbps stereo */
#define VIDEO_FMT_RATE_SET_AMRWBPE_1800_S 28     /* AMR-WB+ 18 kbps stereo */
#define VIDEO_FMT_RATE_SET_AMRWBPE_1720_S 29     /* AMR-WB+ 17.2 kbps stereo */
#define VIDEO_FMT_RATE_SET_AMRWBPE_1640_S 30     /* AMR-WB+ 16.4 kbps stereo */
#define VIDEO_FMT_RATE_SET_AMRWBPE_1600_S 31     /* AMR-WB+ 16 kbps stereo */
#define VIDEO_FMT_RATE_SET_AMRWBPE_1520_S 32     /* AMR-WB+ 15.2 kbps stereo */
#define VIDEO_FMT_RATE_SET_AMRWBPE_1440_S 33     /* AMR-WB+ 14.4 kbps stereo */
#define VIDEO_FMT_RATE_SET_AMRWBPE_1400_S 34     /* AMR-WB+ 14 kbps stereo */
#define VIDEO_FMT_RATE_SET_AMRWBPE_1280_S 35     /* AMR-WB+ 12.8 kbps stereo */
#define VIDEO_FMT_RATE_SET_AMRWBPE_1240_S 36     /* AMR-WB+ 12.4 kbps stereo */
#define VIDEO_FMT_RATE_SET_AMRWBPE_2400_M 37     /* AMR-WB+ 24 kbps mono */
#define VIDEO_FMT_RATE_SET_AMRWBPE_2080_M 38     /* AMR-WB+ 20.8 kbps mono */
#define VIDEO_FMT_RATE_SET_AMRWBPE_1920_M 39     /* AMR-WB+ 19.2 kbps mono */
#define VIDEO_FMT_RATE_SET_AMRWBPE_1680_M 40     /* AMR-WB+ 16.8 kbps mono */
#define VIDEO_FMT_RATE_SET_AMRWBPE_1520_M 41     /* AMR-WB+ 15.2 kbps mono */
#define VIDEO_FMT_RATE_SET_AMRWBPE_1360_M 42     /* AMR-WB+ 13.6 kbps mono */
#define VIDEO_FMT_RATE_SET_AMRWBPE_1200_M 43     /* AMR-WB+ 12 kbps mono */
#define VIDEO_FMT_RATE_SET_AMRWBPE_1040_M 44     /* AMR-WB+ 10.4 kbps mono */

#define VIDEO_FMT_RATE_SET_FIXED          0      /* Fixed bitrate */
#define VIDEO_FMT_RATE_SET_VAR            1      /* Variable bitrate */

/* This enumerated type is used to specify the frame type used by the video
** encoder.
*/
typedef enum {
  I_FRAME,
  P_FRAME,
  FRAME_TYPE_INVALID
} video_frame_type;

/* This enumerated type is used to specify the bit rate level used by the
** video encoder.
*/
typedef enum {
  VIDEO_ENC_LOW_RATE,          /* low rate image, smaller file size     */
  VIDEO_ENC_MED_RATE,          /* medium rate image, medium file size   */
  VIDEO_ENC_HIGH_RATE,         /* high rate image, bigger file size     */
  VIDEO_ENC_USER_DEFINED,      /* target bit rate specified by user     */

#ifndef FEATURE_CAMCORDER_DELETE_LEGACY_PROFILES
  VIDEO_ENC_LOW_QLTY,          /* low quality image, no rate control       */
  VIDEO_ENC_MED_QLTY,          /* medium quality image, no rate control    */
  VIDEO_ENC_HIGH_QLTY,         /* high quality image, no rate control      */
#endif
  VIDEO_ENC_MAX_RATE,          /* Max bit rate supported by EFS         */
  VIDEO_ENC_DYNAMIC_RATE       /* Dynamic bit rate                      */
} video_profile_type;

/* This type of callback is the standard callback type.   This callback will
** be used by all video encode engine and video encoder interface functions
** that do not require data to be passed back to the client.
*/
typedef void (*video_cb_func_ptr_type) (
  video_status_type  status,        /* Status of callback                 */
  void               *client_data   /* Client data from calling function  */
);

/* This enumerated type is used to specify the method by which movie file data
** is accessed.
*/
typedef enum {
  VIDEO_METHOD_MEM,     /* playing/recording movie file to memory buffer   */
  VIDEO_METHOD_EFS,     /* playing/recording movie file to EFS             */
  VIDEO_METHOD_CLIENT,  /* playing/recording movie file through client API */
  VIDEO_METHOD_EFS_INDIRECT, /*playing/recording movie file to EFS
                              and client provides storage for filename     */
  VIDEO_METHOD_PORT,  /* playing/recording movie file through client API */
  VIDEO_METHOD_INVALID  /* designates an invalid type                      */
} video_method_type;

/* This type is associated with the VIDEO_METHOD_MEM enumerated type. This
** type will be used to reference a file in the RAM region of memory.  This
** type specifies a pointer to the start of the data buffer.  The buffer must
** be in RAM.
*/
typedef struct {
  video_method_type  method;  /* This must be VIDEO_METHOD_MEM   */
  uint8              *buf;    /* Buffer in memory                */
  uint32             len;     /* Size of buffer, in bytes        */
} video_handle_type_mem;

/* This type is associated with the VIDEO_METHOD_EFS enumerated type.  This
** type will be used to reference a file in the embedded file system (EFS). It
** specifies the complete name of a file in the EFS.
*/
typedef struct {
  video_method_type  method;  /* This must be VIDEO_METHOD_EFS             */

  char               filename[VIDEO_FILE_FILENAME_MAX_LENGTH]; /* Name of file   */
} video_handle_type_efs;


/* This type is associated with the VIDEO_METHOD_EFS_INDIRECT enumerated type.
** This type will be used to reference a file in the embedded file system (EFS). It
** has a pointer to the name of file
*/
typedef struct {
  video_method_type  method;  /* This must be VIDEO_METHOD_EFS_INDIRECT  */
  const char         *filename; /* Name of file   */
} video_handle_type_filename;
/* Thesee types are associated with the VIDEO_METHOD_CLIENT enumerated type.
** For this type of method, the movie data will be accessed through calls to a
** registered callback function.
*/
typedef uint32 (*video_method_client_cb_func_ptr_type) (
  uint8              *buf,         /* Buffer in memory                  */
  uint32             len,          /* Size of buffer, in bytes          */
  uint32             offset,       /* Destination byte offset in file   */
  void               *client_data  /* Client data from calling function */
);
typedef struct {
  video_method_type    method;     /* This must be VIDEO_METHOD_CLIENT      */
  video_method_client_cb_func_ptr_type  data_push_fn; /* Data push function */
  void                 *client_data;  /* Client data from calling function  */
} video_handle_type_client;

/* This is the base type used to specify a movie file handle. A handle type
** contains all the necessary information needed to access the movie file
** data.  This is a union of all the supported handle types.  The actual
** handle type is determined by examining the source_type field, which must be
** the first parameter in every handle structure type.
*/
typedef union {
  video_method_type         method;  /* method type id  */
  video_handle_type_mem     mem;     /* RAM handle           */
  video_handle_type_efs     efs;     /* EFS handle           */
  video_handle_type_filename fname;    /* EFS handle for qvp   */
  video_handle_type_client  client;  /* client handle        */
} video_handle_type;

#ifdef __cplusplus
   }
#endif

#endif /* VIDEO_COMMON_H */
