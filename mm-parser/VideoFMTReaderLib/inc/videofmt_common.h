#ifndef VIDEOFMT_COMMON_H
#define VIDEOFMT_COMMON_H
/*===========================================================================

        V I D E O   F O R M A T S   C O M M O N   D E F I N I T I O N S

DESCRIPTION
  This header file contains all the definitions common among the various video
  format services modules.

  Copyright(c) 2008-2014 by Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
===========================================================================*/

/*===========================================================================

                      EDIT HISTORY FOR FILE

  This section contains comments describing changes made to this file.
  Notice that changes are listed in reverse chronological order.

  $Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/VideoFMTReaderLib/main/latest/inc/videofmt_common.h#46 $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
09/23/08   ps   Done Changes for more than 4GB card support
07/08/08   shiven  Adding support for loci atom
08/28/07   rpw     Added EVRC-B and EVRC-WB support
04/28/08   sanal   Adding length field, Merging changes for cprt
                   atom suppport.
11/05/07   dhaval  Added copyright atom support
07/03/07   jk      Added 'output_unit_size' for EFS throughput control.
06/19/07   kadhir  Added FTL and USB as storage media
02/19/06   vs      Merged code for support to enable/disable audio/video truncation
                   from 6550/4.5 branch.
12/18/06   Gopi    Added support to enable/disable audio/video truncation.
07/14/06   jdas    Added featurized clipinfo stored as skip atom in recorded clip
06/20/06   Gopi    Added support for dynamically updating titl,descp
                   auth atoms.
                   Code is featurized in videoeng,cameraservices under
                   FEATURE_VIDEO_SETATOMS_DYNAMIC.
                   In videofmt support has been added as an additional API.
                   Also videofmt is feature free file hence didn't featurize
                   code.
05/17/06   jdas    Added profile_comp field in video_fmt_stream_video_subtype
02/12/06   dbv     Added ability to parse avcc atom irrespective of its size
12/09/05   rpw     Added AMR-WB and AMR-WB+ support
11/14/05   Shahed New implementation of camcorder space status error/warning msgs.
11/11/05   Shahed Undo SPACE error/warning changes.
10/08/05   Shahed  Added new members in video_fmt_status_type enum for detailed
                   SPACE warning/error message support.
05/24/05   jk      Added support for a very long clip to the writer.
05/12/05   rpw     Added movie duration limit feature.
02/07/05   rpw     Added still image support.
                   Added audio only support.
                   Added NTT DoCoMo DRM atom support.
                   Added capability of disabling interlacing.
                   Added file size limit feature.
                   Fixed file limit reached logic.
                   Now collapsing all 'mdat' atoms together when recording.
                   Added mode selection to select bitrate for AMR recording.
                   Replaced all TASKLOCK/TASKFREE with REX critical sections.
12/08/04   rpw     Added ability to read/write language code
                   from 'mdhd' atom.
                   Added ability to read width/height and origin
                   information from 'tkhd' atom.
11/03/04   jk      Added video_fmt_mp4w_seek to expose pointer to stream
                   buffer with its size to the client,
                   video_fmt_mp4w_write_footer to write fixed stream footer.
                   Implemented one sample delay before writing chunk out
                   to append fixed footer to the last sample.
09/08/04   rpw     Added option to force 'stsz' table generation, even for
                   streams with fixed-size samples (SKM requirement).
07/14/04   rpw     Added timed text encoding support.
09/16/03   rpw     Moved "sampling_frequency" from aac_params structure to
                   its parent structure, and added "num_channels" field too.
09/05/03   enj     Removing FEATURE_VIDEOFMT (i.e. permanently enabling it)
08/04/03   rpw     Reformatted code in file reader.
                   Renamed most instances of "video_fmt_mp4"
                   to "video_fmt_mp4r".
                   Added file writer under "video_fmt_mp4w" - most of the
                   code came from the video encoder engine.
07/31/03   rpw     Added "sampling_frequency" parsing for AAC audio.
07/24/03   rpw     Added support for H263SampleEntry.
07/24/03   rpw     Added VIDEO_FMT_MP4R_MAX_DEC_SPECIFIC_INFO in place of
                   magic number for size of decoder specific information
                   cached.
07/14/03    ny     Addition of video_fmt_mp4_amr_params_type and addition
                   of amr_parms to video_fmt_stream_audio_subtype
06/23/03   rpw     Replaced FEATURE_MP4_DECODER with FEATURE_VIDEOFMT.
06/11/03   rpw     Added parsing of MPEG-4 AAC audio bitstream headers.
05/23/03   rpw     Added timed text atom minimal support.
05/22/03   rpw     Added interface for finding nearest sync sample at or
                   after any given sample in a stream.
                   Added stream byte offset to sample information.
05/22/03   rpw     Fixed typo in 3GPP2 EVRC sample entry atom names.
03/18/03   rpw     Merged in changes from MPEG4_FILE_FORMAT branch.
02/25/03   rpw     Added method of getting sample information (timestamps,
                   sample sizes, sync samples, etc.)
02/24/03   rpw     Added VIDEO_FMT_IO_DONE and associated data structure.
                   This callback status is given at the end of a read stream
                   requests, instead of VIDE_FMT_CONTINUE, which was used
                   previously, and indicates the total number of bytes read.
11/04/02   rpw     Created file.

===========================================================================*/

/* <EJECT> */
/*===========================================================================

                      INCLUDE FILES FOR MODULE

===========================================================================*/
#include "AEEStdDef.h"              /* Definitions for byte, word, etc.        */

#include "video_common.h"       /* Pull in definitions common to all video */

/* <EJECT> */
/*===========================================================================

                        DATA DECLARATIONS

===========================================================================*/

/* This is the maximum number of streams that can be accessed through video
** format services.
*/
#define VIDEO_FMT_MAX_MEDIA_STREAMS       6

/* This is the maximum amount of decoder specific information cached for the
** client in the video_fmt_dec_specific_info_type structure.
*/
#define VIDEO_FMT_MAX_DEC_SPECIFIC_INFO   128

#define VIDEO_FMT_MAX_SERVER_DATA         2
#define VIDEO_FMT_MAX_RUNS                100
#define MAX_NUM_IND_SUBSTREAM             8

/* definition of the various types of atoms */
#define MOOV_TYPE 0x6D6F6F76  /* 'moov' */
#define MVHD_TYPE 0x6D766864  /* 'mvhd' */
#define TRAK_TYPE 0x7472616B  /* 'trak' */
#define TKHD_TYPE 0x746B6864  /* 'tkhd' */
#define TREF_TYPE 0x74726566  /* 'tref' */
#define HINT_TYPE 0x68696E74  /* 'hint' */
#define DPND_TYPE 0x64706E64  /* 'dpnd' */
#define IPIR_TYPE 0x69706972  /* 'ipir' */
#define MPOD_TYPE 0x6D706F64  /* 'mpod' */
#define SYNC_TYPE 0x73796E63  /* 'sync' */
#define MDIA_TYPE 0x6D646961  /* 'mdia' */
#define MDHD_TYPE 0x6D646864  /* 'mdhd' */
#define HDLR_TYPE 0x68646C72  /* 'hdlr' */
#define MINF_TYPE 0x6D696E66  /* 'minf' */
#define DINF_TYPE 0x64696E66  /* 'dinf' */
#define DREF_TYPE 0x64726566  /* 'dref' */
#define STBL_TYPE 0x7374626C  /* 'stbl' */
#define STSD_TYPE 0x73747364  /* 'stsd' */
#define MP4V_TYPE 0x6D703476  /* 'mp4v' */
#define ENCV_TYPE 0x656E6376  /* 'encv' */
#define ENCA_TYPE 0x656E6361  /* 'enca' */
#define SINF_TYPE 0x73696E66  /* 'sinf' */
#define FRMA_TYPE 0x66726D61  /* 'frma' */
#define SCHM_TYPE 0x7363686D  /* 'schm' */
#define SCHI_TYPE 0x73636869  /* 'schi' */
#define PSSH_TYPE 0x70737368  /* 'pssh' */
#define SAIZ_TYPE 0x7361697A  /* 'saiz' */
#define SAIO_TYPE 0x7361696F  /* 'saio' */
#define TENC_TYPE 0x74656E63  /* 'tenc' */
#define ODKM_TYPE 0x6F646B6D  /* 'odkm' */
#define OHDR_TYPE 0x6F686472  /* 'ohdr' */
#define ODAF_TYPE 0x6F646166  /* 'odaf' */
#define MDRI_TYPE 0X6D647269  /* 'mdri' */
#define MP4A_TYPE 0x6D703461  /* 'mp4a' */
#define WAVE_TYPE 0x77617665  /* 'wave' */
#define SAMR_TYPE 0x73616D72  /* 'samr' */
#define DAMR_TYPE 0x64616D72  /* 'damr' */
#define SEVC_TYPE 0x73657663  /* 'sevc' */
#define EVRC_TYPE 0x65767263  /* 'evrc' TEMPORARY support */
#define DEVC_TYPE 0x64657663  /* 'devc' */
#define SECB_TYPE 0x73656362  /* 'secb' */
#define DECB_TYPE 0x64656362  /* 'decb' */
#define SECW_TYPE 0x73656377  /* 'secw' */
#define DECW_TYPE 0x64656377  /* 'decw' */
#define SQCP_TYPE 0x73716370  /* 'sqcp' */
#define DQCP_TYPE 0x64716370  /* 'dqcp' */
#define S263_TYPE 0x73323633  /* 's263' */
#define H263_TYPE 0x48323633  /* 'h263' */
#define D263_TYPE 0x64323633  /* 'd263' */
#define BITR_TYPE 0x62697472  /* 'bitr' */
#define TX3G_TYPE 0x74783367  /* 'tx3g' */
#define MP4S_TYPE 0x6D703473  /* 'mp4s' */
#define ESDS_TYPE 0x65736473  /* 'esds' */
#define AVC1_TYPE 0x61766331  /* 'avc1' */
#define AVC2_TYPE 0x61766332  /* 'avc2' */
#define AVC3_TYPE 0x61766333  /* 'avc3' */
#define AVC4_TYPE 0x61766334  /* 'avc4' */
#define AVCC_TYPE 0x61766343  /* 'avcC' */
#define MVCC_TYPE 0x6D766343  /* 'mvcC' */
#define VWID_TYPE 0x76776964  /* 'vwid' */
#define BTRT_TYPE 0x62747274  /* 'btrt' */
#define STCO_TYPE 0x7374636F  /* 'stco' */
#define CO64_TYPE 0x636F3634  /* 'co64' */
#define STSC_TYPE 0x73747363  /* 'stsc' */
#define STSZ_TYPE 0x7374737A  /* 'stsz' */
#define STTS_TYPE 0x73747473  /* 'stts' */
#define CTTS_TYPE 0x63747473  /* 'ctts' */
#define STSS_TYPE 0x73747373  /* 'stss' */
#define FREE_TYPE 0x66726565  /* 'free' */
#define SKIP_TYPE 0x736B6970  /* 'skip' */
#define UDTA_TYPE 0x75647461  /* 'udta' */
#define  DRM_TYPE 0x64726D20  /* 'drm ' */
#define FTYP_TYPE 0x66747970  /* 'ftyp' */
#define CPRT_TYPE 0x63707274  /* 'cpry' */
#define AUTH_TYPE 0x61757468  /* 'auth' */
#define TITL_TYPE 0x7469746C  /* 'titl' */
#define DSCP_TYPE 0x64736370  /* 'dscp' */
#define VINF_TYPE 0x76696E66  /* 'vinf' */
#define CVER_TYPE 0x63766572  /* 'cver' */
#define RAND_TYPE 0x72616E64  /* 'rand' */
#define RQMT_TYPE 0x72716D74  /* 'rqmt' */
#define MIDI_TYPE 0x6D696469  /* 'midi' */
#define LINK_TYPE 0x6C696E6B  /* 'link' */
#define MVEX_TYPE 0x6D766578  /* 'mvex' */
#define MEHD_TYPE 0x6D656864  /* 'mehd' */
#define TREX_TYPE 0x74726578  /* 'trex' */
#define MOOF_TYPE 0x6D6F6F66  /* 'moof' */
#define MFHD_TYPE 0x6D666864  /* 'mfhd' */
#define TRAF_TYPE 0x74726166  /* 'traf' */
#define TFHD_TYPE 0x74666864  /* 'tfhd' */
#define TFDT_TYPE 0x74666474  /* 'tfdt' */
#define TRUN_TYPE 0x7472756E  /* 'trun' */
#define MDAT_TYPE 0x6D646174  /* 'mdat' */
#define MFRA_TYPE 0x6D667261  /* 'mfra' */
#define MFRO_TYPE 0x6D66726F  /* 'mfro' */
#define TFRA_TYPE 0x74667261  /* 'tfra' */
#define SAWB_TYPE 0x73617762  /* 'sawb' */
#define SAWP_TYPE 0x73617770  /* 'sawp' */
#define DAWP_TYPE 0x64617770  /* 'damp' */
#define SECB_TYPE 0x73656362  /* 'secb' */
#define DECB_TYPE 0x64656362  /* 'decb' */
#define SECC_TYPE 0x73656377  /* 'secw' */
#define DECC_TYPE 0x64656377  /* 'decw' */
#define AUDIO_HANDLER    0x736F756E /* 'soun' */
#define VIDE0_HANDLER    0x76696465 /* 'vide' */
#define TEXT_HANDLER     0x74657874 /* 'text' */
#define SUBTITLE_HANDLER 0x73756274 /* 'subt' */
#define RTNG_TYPE 0x72746E67 /*'rtng'*/
#define GNRE_TYPE 0x676E7265 /*'gnre'*/
#define PERF_TYPE 0x70657266 /*'perf'*/
#define CLSF_TYPE 0x636C7366 /*'clsf'*/
#define KYWD_TYPE 0x6B797764 /*'kywd'*/
#define LOCI_TYPE 0x6C6F6369 /*'loci'*/
#define YRRC_TYPE 0x79727263 /* 'yrrc'*/
#define META_TYPE 0x6D657461 /*'meta'*/
#define ALBM_TYPE 0x616C626D /*'album'*/
#define SIDX_TYPE 0x73696478 /* 'sidx'*/
#define DASH_TYPE 0x64617368 /* 'dash'*/
#define MSDH_TYPE 0x6D736468 /* 'msdh'*/
#define STPP_TYPE 0x73747070 /* 'stpp'*/
#define SUBS_TYPE 0x73756273 /* 'subs'*/
#define GEOT_TYPE 0xA978797A /* '@xyz'*/
#define AC_3_TYPE 0x61632D33 /* 'ac-3'*/
#define EC_3_TYPE 0x65632D33 /* 'ec-3'*/
#define DAC3_TYPE 0x64616333 /* 'dac3'*/
#define DEC3_TYPE 0x64656333 /* 'dec3'*/
#define DTSC_TYPE 0x64747363 /* 'dtsc'*/
#define DTSH_TYPE 0x64747368 /* 'dtsh'*/
#define DTSL_TYPE 0x6474736C /* 'dtsl'*/
#define DTSE_TYPE 0x64747365 /* 'dtse'*/
#define DDTS_TYPE 0x64647473 /* 'ddts'*/
#define HVC1_TYPE 0x68766331 /* 'hvc1'*/
#define HEV1_TYPE 0x68657631 /* 'hev1'*/
#define HVCC_TYPE 0x68766343 /* 'hvcC'*/
#define QT_TYPE   0x71742020 /* 'qt..'*/
#define MP3_TYPE  0x2E6D7033 /*.mp3*/
#define MSOU_TYPE 0x6D730075 /*ms\0u*/

/*'----: Free form metadata atom'*/
#define FREE_FORM     0x2D2D2D2D /* '----' */
#define ILST_TYPE     0x696C7374 /* 'ilst' */
#define MEAN_TYPE     0x6D65616E /* 'mean' */
#define NAME_TYPE     0x6E616D65 /* 'name' */
#define DATA_TYPE     0x64617461 /* 'data' */
#define IALB_TYPE     0xA9616C62 /* '@alb' */
#define IART_TYPE     0xA9415254 /* '@ART' */
#define ICMT_TYPE     0xA9636D74 /* '@cmt' */
#define ICVR_TYPE     0x636F7672 /* 'covr' */
#define IDAT_TYPE     0xA9646179 /* '@day' */
#define IGEN_TYPE     0xA967656E /* '@gen' */
#define INAM_TYPE     0xA96E616D /* '@nam' */
#define IWRT_TYPE     0xA9777274 /* '@wrt' */
#define DISK_TYPE     0x6469736B /* 'disk' */
#define TRKN_TYPE     0x74726B6E /* 'trkn' */
#define CPIL_TYPE     0x6370696C /* 'cpil' */
#define ALB_ART_TYPE  0x61415254 /* 'aART' */
#define ID32_TYPE     0x49443332 /* 'ID32' */

#define PLAYING_AUDIO    0x01
#define PLAYING_VIDEO    0x02
#define PLAYING_TEXT     0x04

/* This enumerated type lists the different types of media streams. */
typedef enum {
  VIDEO_FMT_STREAM_UNK,    /* unknown type of stream      */
  VIDEO_FMT_STREAM_VIDEO,  /* video_fmt_stream_video_type */
  VIDEO_FMT_STREAM_AUDIO,  /* video_fmt_stream_audio_type */
  VIDEO_FMT_STREAM_TEXT,   /* vidoe_fmt_stream_text_type  */
  VIDEO_FMT_STREAM_DATA,   /* video_fmt_stream_data_type  */
  VIDEO_FMT_STREAM_INVALID /* invalid type of stream      */
} video_fmt_stream_type;

/* This enumerated type lists all the possible status conditions used by the
** video format services.  For each status, the comments below indicate which
** field of the "info" union parameters to the "video_fmt_status_cb_func_type"
** callback contains additional information ("NULL" indicates there is no
** additional information).
*/
typedef enum {
  VIDEO_FMT_ALLOC,          /* alloc           */
  VIDEO_FMT_REALLOC,        /* realloc         */
  VIDEO_FMT_FREE,           /* free            */
  VIDEO_FMT_GET_DATA,       /* get_data        */
  VIDEO_FMT_PUT_DATA,       /* put_data        */
  VIDEO_FMT_GET_META_DATA,  /* get_meta_data   */
  VIDEO_FMT_PUT_META_DATA,  /* put_meta_data   */
  VIDEO_FMT_CONTINUE,       /* cont            */
  VIDEO_FMT_INFO,           /* info            */
  VIDEO_FMT_WRITER_READY,   /* writer_ready    */
  VIDEO_FMT_UUID,           /* uuid_atom       */
  VIDEO_FMT_UDTA_CHILD,     /* udta child atom */
  VIDEO_FMT_TEXT,           /* text atom tx3g/stpp */
  VIDEO_FMT_IO_DONE,        /* io_done         */
  VIDEO_FMT_SPACE_CHECK,    /* space_check     */
  VIDEO_FMT_SPACE_CONSUMED, /* space_consumed  */
  VIDEO_FMT_HINT,           /* hint            */
  VIDEO_FMT_DONE,           /* done            */
  VIDEO_FMT_LIMIT_NEAR,     /* NULL            */
  VIDEO_FMT_LIMIT_IMMINENT, /* NULL            */
  VIDEO_FMT_LIMIT_REACHED,  /* done            */
  VIDEO_FMT_FAILURE,        /* NULL            */
  VIDEO_FMT_BUSY,           /* NULL            */
  VIDEO_FMT_FRAGMENT,       /* info            */
  VIDEO_FMT_FRAGMENT_SIZE,
  VIDEO_FMT_FRAGMENT_PEEK,
  VIDEO_FMT_ABS_FILE_OFFSET,
  VIDEO_FMT_DATA_INCOMPLETE,
  VIDEO_FMT_INIT,
  VIDEO_FMT_DATA_CORRUPT,
  VIDEO_FMT_FILESIZE,       /* To calculate file Size */
  VIDEO_FMT_UDTA_ATOM_REQ,
  VIDEO_FMT_CLIPINFO_REQ,   /* clipinfo        */
  VIDEO_FMT_STATUS_INVALID  /* NULL            */
} video_fmt_status_type;

/* This enumerated type lists the different types of data streams.  */
typedef enum {
  VIDEO_FMT_STREAM_DATA_BIFS,  /* Binary Format for Scenes (MPEG-4) */
  VIDEO_FMT_STREAM_DATA_OD,    /* Object Descriptors (MPEG-4)       */
  VIDEO_FMT_STREAM_DATA_SKT_TEXT,   /* SKT text                          */
  VIDEO_FMT_STREAM_DATA_UNK    /* Unknown type of data              */
} video_fmt_stream_data_type;

/* This structure provides user information contained in the file.
*/
typedef struct {
  uint64  creation_time;        /* creation time of presentation           */
  uint64  mod_time;             /* last time the presentation was modified */
  uint32  movie_timescale;      /* time units per sec for whole movie      */
  uint64 total_movie_duration; /* length (in timescale) of movie ('mvhd') */
  uint32  next_track_id;        /* id of next track to be added to movie   */
  uint8   no_rand_access;       /* file random access denied               */
  uint8   video_only_allowed;   /* allow video-only                        */
  uint8   audio_only_allowed;   /* allow audio-only                        */
  uint64  fragment_file_total_movie_duration;
                      /* ength (in timescale) of fragmented movie ('mehd') */
} video_fmt_file_info_type;

typedef struct {
  uint32 data_format;           /* format of encoded data                      */
  uint32 scheme_type;           /* protection scheme type                      */
  uint32 scheme_version;        /* protection scheme version                   */
  uint8  EncryptionMethod;
  uint8  PaddingScheme;         /* Padding type                                */
  uint64 PlaintextLength;       /* Plaintext content length in bytes           */
  uint16 ContentIDLength;       /* Length of ContentID field in bytes          */
  uint16 RightsIssuerURLLength; /* Rights Issuer URL field length in bytes     */
  uint16 TextualHeadersLength;  /* Length of the TextualHeaders array in bytes */
  uint8  SelectiveEncryption;
  uint8  KeyIndicatorLength;
  uint8  IVLength;
} video_fmt_pdcf_type;

/* This structure stores the view information */
typedef struct
{
  uint16  view_id;
  uint16  view_order_index;
  uint8   base_view_type;
  uint16  num_ref_views;
  uint16* ref_view_id;//array of num_ref_views elements
}video_fmt_view_info;

/* This structure stores the vwid atom information */
typedef struct
{
  uint8  min_temporal_id;
  uint8  max_temporal_id;
  uint16 num_views;
  video_fmt_view_info* p_view_info;
}video_fmt_vwid_info;


/* This structure contains type-specific information about a
** video stream.
*/
typedef struct {
  video_fmt_stream_video_type format;
  uint16                      width;            /* frame width in pixels  */
  uint16                      height;           /* frame height in pixels */
  float                       frame_rate;       /* frames per second      */
  uint16                      iframe_interval;  /* I-frame interval       */
  uint8                       profile;          /* codec feature set      */
  uint8                       level;            /* subdivision of level   */
  uint8                       profile_comp;     /* Profile compatibility */
  uint16                      nViews;
  boolean                     complete_representation;
  boolean                     explicit_au_track;
  video_fmt_vwid_info         vwid_info;
  video_fmt_pdcf_type         pdcf_info;
} video_fmt_stream_video_subtype;

/* This structure contains AAC stream header information needed by the AAC
** decoder.
*/
typedef struct {
  uint8    audio_object_type;
  uint8    channel_configuration;
  uint8    sbr_present_flag;
  uint8    ps_present_flag;
  uint8    ep_config;
  boolean  aac_section_data_resilience_flag;
  boolean  aac_scalefactor_data_resilience_flag;
  boolean  aac_spectral_data_resilience_flag;
} video_fmt_aac_params_type;

/* This structure contains AC3/EC3 bit stream information needed by the AC3/EC3
 ** decoder.
 */
typedef struct{
  uint8  ucFscod;           /* Sampling frequency code   */
  uint8  ucBsid;            /* Bit stream identification */
  uint8  ucBsmod;           /* Bit stream mode           */
  uint8  ucAcmod;           /* Audio coding mode         */
  uint8  ucLfeon;           /* Lower frequency effect ON */
  uint8  ucNumDepSubs;      /* No# of dependent sub streams that are
                             * are associated with independent sub stream */
  uint8  ucNumChannels;
  uint16 usChLocation;      /* Channel locations         */
}video_fmt_dd_bsi_info_type;

typedef struct {
  uint16                     usBitRate;        /* data-rate in kbit/s                     */
  uint8                      ucNumIndSubs;     /* No# of sub-stream present in bit-stream */
  uint8                      ucEC3ExtTypeA;    /* Flag for EC3 extension type A           */
  uint8                      ucComplexIdxTypeA;/* Complexity index of type A              */
  video_fmt_dd_bsi_info_type sBSIInfo[MAX_NUM_IND_SUBSTREAM]; /* bit-stream information   */
}video_fmt_dd_params_type;

/* This structure contains DTS bit stream information needed by the DTS
 ** decoder.
 */
typedef struct{
  uint32 ulMaxSamplingFrequency;  /* Maximum sampling frequency stored stream */
  uint32 ulMaxBitRate;            /* Peak bit rate in bit per second          */
  uint32 ulAvgBitRate;            /* Average bit rate in bit per second       */
  uint8  ucPcmSampleDepth;        /* Bit depth of rendered audio.             */
  uint8  ucFrameDuration;         /* No# of audio sample decoded in AU        */
  uint8  ucStreamConstruction;    /* Information about extensions             */
  uint8  ucCoreLEFPresent;        /* LEF channel presence in core             */
  uint8  ucCoreLayout;            /* Channel layout with core sub stream      */
  uint16 usCoreSize;              /* Size of core sub-stream AU in bytes      */
  uint8  ucStereoDownMix;         /* Indication of stereo down mix in stream  */
  uint8  ucRepresentationType;    /* Special properties of audio presentation */
  uint16 usChannelLayout;         /* Channel layout of core and sub-stream    */
  uint8  ucMultiAssetFlag;        /* Indicate multiple asset presence         */
  uint8  ucLBRDurationMod;        /* Indicate LBR coding bandwidth            */
}video_fmt_dts_params_type;

/* This structure contains AMR/EVRC/QCELP-specific information needed by
** the media player.
*/
typedef struct {
  uint16   mode_set;
  uint8    mode_change_period;
  uint8    frames_per_sample;
} video_fmt_audio_params_type;

/* This structure contains type-specific information about an
** audio stream.
*/
typedef struct {
  video_fmt_stream_audio_type    format;
  video_fmt_aac_params_type      aac_params;
  video_fmt_dd_params_type       dd_params;
  video_fmt_dts_params_type      dts_params;
  video_fmt_audio_params_type    audio_params;
  uint32                         sampling_frequency;
  uint8                          num_channels;
  video_fmt_pdcf_type            pdcf_info;
} video_fmt_stream_audio_subtype;

/* This structure contains type-specific information about a text stream. */
typedef struct {
  video_fmt_stream_text_type format; /* indicate text track format */
  int cur_atom_index; /* index of current tx3g atom being parsed   */
} video_fmt_stream_text_subtype;

/* This structure contains type-specific information about a data stream. */
typedef struct {
  video_fmt_stream_data_type format;
} video_fmt_stream_data_subtype;

/* This union combines together the different type-specific media stream
** information structures.
*/
typedef union {
  video_fmt_stream_video_subtype video; /* VIDEO_FMT_STREAM_VIDEO */
  video_fmt_stream_audio_subtype audio; /* VIDEO_FMT_STREAM_AUDIO */
  video_fmt_stream_text_subtype  text;  /* VIDEO_FMT_STREAM_TEXT  */
  video_fmt_stream_data_subtype  data;  /* VIDEO_FMT_STREAM_DATA  */
} video_fmt_stream_subinfo_type;

/*
 ** This structure characterizes the reference tracks when included in a stream
 */
typedef struct {
  uint32    ref_atom;
  uint32    track_count;
  uint32    track_id [VIDEO_FMT_MAX_MEDIA_STREAMS];
} video_fmt_ref_track_type;

typedef struct {
  uint16                content_version;    /* track content version       */
  uint8                 no_rand_access;     /* track random access denied  */
} video_fmt_udata_type;

/*
 ** This structure contain NAL unit length & data information
 */
typedef struct {
  //! NAL unit length
  uint16 len;
  //! NAL unit data
  byte  *data;
} video_fmt_nalu_data_type;

/*  This structure contain NALU array of particular NALU type
 */
typedef struct {
  //! 1: NALU of given type available in NALU array
  //! 0: Additional NALU of given type will also available in stream
  uint8  ucArrayCompleteness;
  //! Indicate type of NALU i.e. VPS, SPS, PPS, APS or SEI
  uint8  ucNALUType;
  //! Indicate number of NALU of indicated type available
  uint16 usNumNALU;
  //! Store NALU length & contain its data
  video_fmt_nalu_data_type *pNALU;
} video_fmt_nalu_array_type;

/*
 ** This structure retains the value of memory pointer, memory allocated
 ** and boolean value for whether memory has been allocated or not
 ** for the avcc/mvcc/hvcc atom whose size might exceed the maximum size of
 ** ping pong buffers
 */
typedef struct
{
    boolean memory_allocated;
    uint8 *memory_ptr;
    uint32 allocated_size;
} video_fmt_mp4r_data_alloc_type;

/*
** This structure stores the mvcc atom information.
*/
typedef struct
{
  uint8 configurationVersion;
  uint8 MVCProfileIndication;
  uint8 profile_compatibility;
  uint8 MVCLevelIndication;
  /*complete_representation is set on a minimal set of tracks that contain
  *a portion of the original encoded stream.
  *Other tracks may be removed from the file without loss of
  *any portion of the original encoded bitstream, and,
  *once the set of tracks has been reduced to only those in the complete subset,
  *any further removal of a track removes a portion of the encoded information
  */
  uint8 complete_representation;
  /*explicit_au_track is set on a track that is ‘complete’;
  *it is not necessary to determine the view dependencies,
  *nor calculate whether views not present in this track
  *must be found from other tracks
  */
  uint8 explicit_au_track;
  uint8                       len_minus_one;
  uint8                       num_seq_param;
  uint8                       num_pic_param;
  video_fmt_nalu_data_type  *seq_param_set;
  video_fmt_nalu_data_type  *pic_param_set;
} video_fmt_mp4r_mvcc_info;

/*
** This structure stores the avcc atom information.
*/
typedef struct
{
  uint8 configurationVersion;
  uint8 AVCProfileIndication;
  uint8 profile_compatibility;
  uint8 AVCLevelIndication;
  uint8 len_minus_one;
  uint8 num_seq_param;
  uint8 num_pic_param;
  video_fmt_nalu_data_type  *seq_param_set;
  video_fmt_nalu_data_type  *pic_param_set;
} video_fmt_mp4r_avcc_info;

/* This structure contains extra type-specific information about
** a H264 video stream.
*/
typedef struct {
  uint32                      horiz_resolution;
  uint32                      verti_resolution;
  uint16                      frame_count;
  uint16                      depth;
  video_fmt_mp4r_data_alloc_type avcc_alloc; /* Dynamic allocation for avcc atom*/
  video_fmt_mp4r_data_alloc_type mvcc_alloc; /* Dynamic allocation for mvcc atom*/
  video_fmt_mp4r_data_alloc_type vwid_alloc; /* Dynamic allocation for vwid atom*/
  video_fmt_mp4r_avcc_info*  avcc_info;
  video_fmt_vwid_info*       vwid_info;
  video_fmt_mp4r_mvcc_info*  mvcc_info;
  boolean bAvcParseDone;//This will be set to true once we are done with parsing AVC atom.
} video_fmt_h264_dec_info_type;

/*
 ** This structure stores the hvcC atom information.
 */
typedef struct{
  //! Indicate configuration version
  uint8 ucConfigVersion;
  //! Indicate profile space
  uint8 ucProfileSpace;
  //! Indicate Tier flag
  uint8 ucTierFlag;
  //! Indicate profile IDC
  uint8 ucProfileIDC;
  //! Indicate profile compatibility indications
  uint32 ulProfileCompatibilityFlags;
  //! Indicate constraint indication flags
  uint64 usContraintIndicatorFlags;
  //! Indicate Level IDC
  uint8 ucLevelIDC;
  //! Indicate minimum spatial segmentation IDC
  uint8 ucSpatialSegIDC;
  //! +1 indicate the length in bytes of the NALUnitLength in an HEVC sample
  uint8 ucLenSizeMinusOne;
  //!Average frame rate in units of frames/(256 sec)
  uint16 usAvgFrameRate;
  //!Number of array of NALU
  uint8 ucNumOfNALArray;
  //! Total length of NALU in NALU array
  uint32 ulNaluDataLength;
  //!Array of NALU
  video_fmt_nalu_array_type* pArrayNALU;
}video_fmt_mp4r_hvcc_info;

/*
 ** This structure contains extra type-specific information about
 ** a HEVC video stream.
 */
typedef struct{
  //! Flag to indicate HEVC atom parsing are done
  boolean bHEVCParseDone;
  //! Horizontal resolution of the image in pixels-per-inch
  uint32 ulHorizResolution;
  //! Vertical resolution of the image in pixels-per-inch
  uint32 ulVertResolution;
  //! Number of compressed frame stored in each sample
  uint16 usFrameCount;
  //! Takes -0x0018 images are in color with no ALPHA
  uint16 usDepth;
  //! Dynamic allocation for hvcC atom
  video_fmt_mp4r_data_alloc_type sHVCCAtomAlloc;
  //! hvcC atom specific information
  video_fmt_mp4r_hvcc_info *pHVCCInfo;
}video_fmt_hevc_dec_info_type;

/* The info[] array contains information specific to the decoder */
typedef struct {
  uint8            obj_type;
  uint8            stream_type;
  uint32           max_buffer_size;         /* decoder max buffer size     */
  uint32           maxbitrate;              /* maximum bitrate             */
  uint32           avgbitrate;              /* average bitrate             */
  uint8            info[VIDEO_FMT_MAX_DEC_SPECIFIC_INFO];
                                            /* decSpecificInfo             */
  video_fmt_h264_dec_info_type h264_info;   /* H264 specific info          */
  video_fmt_hevc_dec_info_type sHEVCInfo;   /* HEVC codec specific info    */
} video_fmt_dec_specific_info_type;

typedef struct {
  uint64           first_frame;
  uint32           trun_count;
  uint32           sample_description_index;
  uint64           first_timestamp;
} video_fmt_track_frag_info_type;

/* This structure provides information about one particular media stream in a
** file.
*/
typedef struct {
  uint32           stream_num;     /* stream num in file (0,1,..)          */
  uint32           track_id;       /* stream identification                */
  video_fmt_stream_type  type;     /* media type                           */
  video_fmt_stream_subinfo_type subinfo;    /* type-specific media info    */
  uint64           frames;         /* length in frames                     */
  uint64           bytes;          /* length in bytes                      */
  uint32           largest;        /* byte size of largest frame           */
  boolean          largest_found;   /* flag to check whether already computed
                                       the largest or not */
  uint32           header;         /* # bytes in decSpecificInfo           */
  uint32           media_timescale;/* media-specific time units per second */
  uint64           media_duration; /* in time units using media_timescale  */
  uint64           creation_time;  /* creation time of the stream          */
  uint64           mod_time;       /* last time the stream was modified    */
  boolean          fixed_size;
  boolean          fixed_rate;
  boolean          inter_frames;   /* TRUE only if stream has inter-frames */
  video_fmt_dec_specific_info_type dec_specific_info;
  video_fmt_udata_type      user_data;      /* info gathered from 'udta'   */
  video_fmt_ref_track_type  ref_track;      /* tracks referenced by stream */
  uint32                    fragment_number;
  uint64                    fragment_offset;  /* Position of the fragment for repositioning */
  video_fmt_track_frag_info_type  track_frag_info;
  uint32 prevReqSampleNum;         /* Sample number whose abs_file_offset has been found*/
  uint32           rotation_degrees;/* Derived for composition matrix         */
  uint32           tkhd_width;     /* width from 'tkhd' (video/text only)    */
  uint32           tkhd_height;    /* height from 'tkhd' (video/text only)   */
  uint32           tkhd_origin_x;  /* origin_x from 'tkhd' (video/text only) */
  uint32           tkhd_origin_y;  /* origin_y from 'tkhd' (video/text only) */
  uint16           media_language; /* packed ISO-639-2/T language code       */
} video_fmt_stream_info_type;

/* This structure contains info about an MP4 file atom being parsed. */
typedef struct {
  uint32  type;     /* 4-letter printable atom type code, or beginning of */
                    /*   UUID, if not a type defined in ISO/IEC 14496-1   */
  uint8   uuid[12]; /* Remainder of atom type UUID - always               */
                    /*   0x00110010800000AA00389B71 for types defined     */
                    /*   in ISO/IEC 14496-1                               */
  uint32  size;     /* size of atom contents, in bytes                    */
  uint64  offset;   /* Offset of atom contents, in MP4 file               */
  uint64  parent;   /* Offset of atom's parent's contents - zero          */
                    /*   for atoms not contained by other atoms           */
  boolean user;     /* For convenience - TRUE if atom type is a           */
                    /*   user-defined type (one not defined in            */
                    /*   ISO/IEC 14496-1                                  */
  boolean contain;  /* TRUE if this atom can contain other atoms.         */
} video_fmt_mp4_atom_type;

/* This enumerated type lists the different kinds of data units the
** application layer can ask for when demuxing media stream data.
*/
typedef enum {
  VIDEO_FMT_DATA_UNIT_BYTE,
  VIDEO_FMT_DATA_UNIT_FRAME,
  VIDEO_FMT_DATA_UNIT_INVALID
} video_fmt_data_unit_type;

/* This structure contains information about a single sample (video frame or
** audio packet) in a video file stream.
*/
typedef struct {
  uint64 sample; /* sample number (first sample is zero)                    */
  uint32 size;   /* size of sample in bytes                                 */
  uint64 offset; /* offset of sample in stream, in bytes                    */
  uint64 time;   /* decode time of sample, in the media timescale      */
  uint64 decode_time;   /* composition time of sample, in the media timescale  */
  uint64 delta;  /* difference between composition time of this sample and  */
                 /* the next sample, in the media timescale                 */
  uint32 sync;   /* Indication if sample is a random access point           */
                 /* (non-zero) or not (zero)                                */
  uint32 sample_desc_index; /* Sample Description retrieved from the        */
                            /* stsc atom                                    */
  uint32  aux_sample_info_size ;   /* size of Auxiliary sample in bytes     */
  uint64  aux_sample_info_offset;  /* offset of Auxiliary sample in bytes   */
} video_fmt_sample_info_type;


#if defined(T_WINNT)
   #pragma pack(push, videofmtcommon_h, 1)
#endif

typedef struct {
    uint64 access_point_time;
    uint32 moof_offset;
    uint8 traf_number;
    uint8 trun_number;
    uint8 sample_number;
} video_fmt_tfra_entry_type;

#if defined(T_WINNT)
   #pragma pack(pop, videofmtcommon_h)
#endif


/* This structure contains information about an entry in a dynamic highlight
** (karaoke) text modifier for a timed text stream sample.
*/
typedef struct {
  uint32 end_time;
  uint16 start_char;
  uint16 end_char;
} video_fmt_karaoke_box_type;

/* This structure contains information about a style record entry in a style
** text modifier for a timed text stream sample.
*/
typedef struct {
  uint16   start_char; /* starting offset in string   */
  uint16   end_char;   /* ending offset in string     */
  uint16   font_id;
  boolean  bold;
  boolean  italic;
  boolean  underline;
  uint8    font_size;
  uint32   text_color; /* 0xrrggbbaa where rr=red,    */
                       /* gg=green, bb=blue, aa=alpha */
} video_fmt_text_style_type;


#define VIDEO_FMT_MAX_FRAMEDROP          100
#define VIDEO_FMT_SERSOR_MODEL_SIZE      31

/* This structure contains frame dropping information; Each bit of
data field represents one frame; If the bit is set i.e 1 then the
frame is dropped by the RC. Otherwise, the frame is encoded */
typedef struct video_fmt_framedrop_list_type_t {
    struct video_fmt_framedrop_list_type_t    *next;
    uint8                                     data[VIDEO_FMT_MAX_FRAMEDROP];
    uint32                                    used_bits;
                            /* No of bits used of data field */
}video_fmt_framedrop_list_type;

/* Structure used to store Qcom platform specific information which need to
** be stored as "skip" atom at the end of recording session in a
** recorded movie file.
*/
typedef struct {
  int8                         *build_id;        /* for example: m6550B-SMAAR-4435 */
  uint16                       build_id_len;     /* length of build_id */
  uint16                       vfe_dsp_id;       /* front end dsp id */
  uint16                       backend_dsp_id;   /* backend dsp id */
  uint8                        qdspmodule;       /* qdsp module number */
  char                         sersor_model[VIDEO_FMT_SERSOR_MODEL_SIZE];
                                                 /* Sensor model name */

  /* The following are the parameters passed while initilization of videoengine */
  uint8                        file_format;      /* video_fmt_type i.e. MP4, AVI, ...   */
  uint8                        file_brand;       /* video_fmt_brand_type i.e. 3GP, AMC, ...   */
  uint8                        video_format;     /* video_fmt_stream_video_type i.e. MPEG-4, ...     */
  uint8                        audio_format;     /* video_fmt_stream_audio_type i.e. EVRC, AMR, ...  */
  uint16                       frame_width;      /* frame width          */
  uint16                       frame_height;     /* frame height         */
  uint16                       video_time_increment;   /* ticks per frame      */
  uint16                       video_time_resolution;  /* ticks per second     */
  uint8                        profile;          /* video_profile_type i.e. low, medium, high        */
  int32                        user_bitrate;     /* user specified b.r.  */
  int16                        rotation_flag;    /* rotation value       */
  uint8                        method;           /* video_method_type i.e. mem, efs, client  */
  boolean                      short_header;     /* H.263 compatibility  */
  boolean                      fixed_fps_mode;   /* fixed frames per sec */
  uint32                       movie_size_limit; /* if >0, max file size */
  uint32                       text_size;        /* size of "text"       */
  uint8                        text_type;        /* video_eng_text_input_type format of "text"     */
  const char                   *text;            /* optional timed text  */

  uint8                        audio_channel;     /* video_eng_audio_channel_type */
  uint8                        audio_sample_rate; /* video_eng_audio_sample_rate_type */
  uint8                        audio_profile;     /* video_eng_audio_profile_type */

  uint32                       bufferCount;
  boolean                      reorder_atom;     /* atom reorder according to KDDI */
  /* The following are encoder init information */
  uint32                       enc_pkt_size;     /* DSP output frame buffer size */
  uint32                       ved_buf_size;     /* Video enc output buffer size */
  uint32                       enc_init_inter_memory_size; /* encoder init internal mem size */

  /* The following are from video_config_type to store encoder configuration */
  boolean                      still_image;      /* discard frames > 1   */
  uint16                       audio_rate_set;   /* i.e. AMR bitrate     */
  boolean                      no_interlace;     /* don't mix vid/aud    */
  uint32                       movie_duration_limit; /* if >0, max dur   */
  boolean                      include_drm;      /* Include "drm " atom  */
  uint16                       drm_distribution_rights;
  uint16                       title_lang;
  uint16                       author_lang;
  uint16                       description_lang;
  uint16                       copyright_lang;
  uint16                       loci_lang;
  /* The following are from video_enc_config_type structure to keep encoder
  ** configuration inforamtion
  */
  int32                        target_bitrate;   /* Target bit rate              */
  int32                        intra_period;     /* Intra frame period in ms     */
  int32                        first_frame_delta;/* First frame delta            */
                                          /* ignored if <= 0              */
  uint8                        time_unit;        /* video_enc_time_type Unit of time passed in       */
                                          /* video_encode_frame           */
  uint8                        rc_ctl;           /*video_enc_rc_type Rate control flag            */

  /* The following from video_enc_raw_config_type structure */
  uint32                       adv_config_cookie;     /* This cookie enables this config */
  uint16                       quantizer_scale_intra;        /* quant_scale for I-VOPs  */
  uint16                       quantizer_scale_inter;        /* quant_scale for P-VOPs  */
  uint8                        frac_pixel_enable;            /* Enable fractional pixel search  */

  uint16                       rcw_overflow_factor;          /* RC Window Overflow Factor  */
  uint16                       pframe_overflow_factor;       /* P-Frame Overflow Factor    */
  uint16                       loose_pframe_overflow_factor; /* Loose P-Frame Overflow Factor */
  uint16                       max_qp_frame_range_down;      /* max. qp range down from frame qp         */
  uint16                       max_qp_frame_range_up;        /* max. qp range up from frame qp           */
  uint32                       log2_nc_square;               /* log2 of square of number coefficients    */
  int32                        alpha;                        /* alpha in rate control parmetric equation */
  int32                        beta;                         /* beta in rate control parmetric equation  */

  uint16                       quantizer_scale_intra_low;    /* Lower bound for quant_scale for I-VOPs   */
  uint16                       quantizer_scale_intra_high;   /* Higher bound for quant_scale for I-VOPs  */
  uint16                       quantizer_scale_inter_low;    /* Lower bound for quant_scale for P-VOPs   */
  uint16                       quantizer_scale_inter_high;   /* Higher bound for quant_scale for P-VOPs  */

  /* The following from video_enc_raw_dsp_cfg_type structure to store
  ** advanced DSP selection cmd configs
  */
  uint32                       dsp_cfg_cookie;               /* This cookie enables this config          */
  uint16                       intra_16x16_bias;
  uint16                       intra_4x4_bias;
  uint16                       inter_one_mv_bias;
  uint16                       inter_four_mv_bias;
  uint16                       four_mv_threshold;    /* Threshold to decide between 1MV and 4MV  */
  uint8                        search_type;          /* MB search, Block search or both          */
  uint8                        use_16x16_srch_min;   /* use MB min as anchor for block search    */
  uint8                        use_16x16_anch_pnt;   /* use MB anchor as Block anchor            */
  uint8                        ac_pred_flag;
  uint8                        rounding_ctrl;
  uint8                        video_hw_me_profile;  /* HW ME profiling flag                     */
  uint8                        search_method;        /* search_method_raw_type */

  uint8                        luma_filter; /* video_enc_raw_lf_type Placeholder Luma Filter            */

  uint8                        save_location; /* storage medium used for movie file        */
  uint64                       free_space;    /* Free space available in the medium chosen */

  /* if video_method_type type is VIDEO_METHOD_MEM, then it will
  ** store memory length
  */
  uint32                       bytes_available;
  /* This represents the space left after the end of recording session */
  uint64                       space_left;

  /* The following represents the frame statistics of each frame whether
  ** it is being dropped or encodded
  */
  uint32                                frame_dropped; /* total no. of frame dropped */
  uint32                                frame_encoded; /* total no. of frame encodded */
  video_fmt_framedrop_list_type         *framedrop_head;
  video_fmt_framedrop_list_type         *framedrop_current;

  char                                  *err_msg;
  uint16                                err_msg_size;
} video_fmt_clipinfo_type;

/* This union defines either a UTF-8 or UTF-16 string, terminated with a NULL
** character.
*/
typedef union video_fmt_utf_string_type_t {
  const char *utf8;
  const short *utf16;
} video_fmt_utf_string_type;

/* This structure describes a string along with the language in which it is
** written.
**
** language - This is the packed ISO-639-2/T language code describing the
** language in which the string is encoded.
**
** isUTF16 - if set, the string is UTF-16, the "utf16" field of the
** "string" union is used, and the string should begin with byte order mark
** (0xFEFF).  Otherwise, the string is UTF-8, and the "utf8" field of the
** "string" union is used.
**
** string - this contains the string, encoded either in UTF-8 or UTF-16,
** terminated with a NULL character.
*/
typedef struct video_fmt_language_coded_string_type_t {
  uint16                     language;
  boolean                    isUTF16;
  video_fmt_utf_string_type  string;
  uint32                     length;
} video_fmt_language_coded_string_type;

typedef struct video_fmt_user_proprietary_data_type_t {
  boolean                     data_valid;
  uint32                      nSize;
  uint8                      *pData;
}video_fmt_user_proprietary_data_type;

typedef struct video_fmt_location_type_t {
  boolean                               loci_valid; /* To verify that the location info is valid */
  video_fmt_language_coded_string_type  name;
  uint8                                 role;
  uint32                                longitude;
  uint32                                latitude;
  uint32                                altitude;
  video_fmt_language_coded_string_type  astr_body;
  video_fmt_language_coded_string_type  add_notes;
} video_fmt_location_type;


typedef struct video_fmt_title_type_t {
  boolean titl_valid;
  video_fmt_language_coded_string_type titl;
}video_fmt_title_type;

typedef struct video_fmt_author_type_t {
  boolean auth_valid;
  video_fmt_language_coded_string_type auth;
}video_fmt_author_type;

typedef struct video_fmt_copyright_type_t{
  boolean cprt_valid;
  video_fmt_language_coded_string_type cprt;
}video_fmt_copyright_type;

typedef struct video_fmt_performer_type_t{
  boolean perf_valid;
  video_fmt_language_coded_string_type perf;
}video_fmt_performer_type;

typedef struct video_fmt_genre_type_t{
  boolean gnre_valid;
  video_fmt_language_coded_string_type gnre;
}video_fmt_genre_type;

typedef struct video_fmt_description_type_t{
  boolean desc_valid;
  video_fmt_language_coded_string_type desc;
}video_fmt_description_type;

typedef struct video_fmt_rating_type_t {
  boolean rtng_valid;
  uint32  entity;
  uint32  criteria;
  video_fmt_language_coded_string_type rtng;
}video_fmt_rating_type;

typedef struct video_fmt_album_type_t{
  boolean albm_valid;
  boolean trknum_valid;
  video_fmt_language_coded_string_type albm;
  uint8 trackNo;
}video_fmt_album_type;

typedef struct video_fmt_classification_type_t {
  boolean clsf_valid;
  uint32 entity;
  uint16 table;
  video_fmt_language_coded_string_type clsf;
}video_fmt_classification_type;

typedef struct video_fmt_keywords_type_t {
  boolean kywd_valid;
  uint8 numKywds;
  video_fmt_language_coded_string_type *kywd;
}video_fmt_keywords_type;

typedef struct video_fmt_recorded_year_type_t {
  boolean yrrc_valid;
  uint16  year;
}video_fmt_recorded_year_type;

/* This structure provides information such as title, author, and description
** for a movie file.
*/
typedef struct video_fmt_user_data_type_t {
  boolean atom_is_valid; /* Check for the validity of atom */

  uint32 midiSize; /*if zero, format writer will exclude midi section*/
  uint8 * midiData;

  video_fmt_title_type                  title;
  video_fmt_author_type                 author;
  video_fmt_description_type            description;
  video_fmt_copyright_type              copyright;
  video_fmt_performer_type              performer;
  video_fmt_genre_type                  genre;
  video_fmt_rating_type                 rating;
  video_fmt_classification_type         clsf;
  video_fmt_keywords_type               keywords;
  video_fmt_album_type                  album;
  video_fmt_recorded_year_type          yrrc;
  video_fmt_location_type               loci;
  video_fmt_user_proprietary_data_type  data;
} video_fmt_user_data_type;

/* Forward declaration of union, necessary to compile circular structure
** references used below.
*/
union video_fmt_status_cb_info_type_t;

/* Forward declaration of structure used in parameter list of function
** declared before the structure itself.
*/
struct video_fmt_text_type_t;

/* This callback function type is used as a means for the client to tell the
** video format services to terminate the current session.  The session is not
** actually terminated until the next VIDEO_FMT_DONE status callback.
*/
typedef void (*video_fmt_end_cb_func_type) (void *server_data);

/* This callback function type is used for returning status to the client,
** and for requesting or returning more data.  The client registers this
** callback function when it initiates encoding or decoding.
*/
typedef void (*video_fmt_status_cb_func_type) (
  video_fmt_status_type                  status,
  void                                   *client_data,
  void                                   *info,
  video_fmt_end_cb_func_type             end
);

/* This callback function type is used for continuing a current video format
** services operation in progress.  It is given to the client in the server
** data of many of the status callbacks.  The "server_data" here must be the
** same as the "server_data" given to the client during the last status
** callback.
*/
typedef void (*video_fmt_continue_cb_func_type) (void *server_data);

/* This callback function type is used to ask the video format services to
** start demuxing data from one of the streams in the media file.  It is given
** to the client in the server data for the VIDEO_FMT_INFO status callback,
** when the video format services is finished decoding the media meta-data and
** is ready to begin demuxing stream data.
**
** Note that it is the responsibility of the caller to allocate enough memory
** for the buffer pointed to by the "buffer" parameter to fit the given number
** of units of stream data.  If the unit is VIDEO_FMT_DATA_UNIT_FRAME, then
** the client should make sure the buffer is at least as big as the largest
** frame size (returned in video_fmt_stream_info_type) multiplied by the
** number of frames to be read.
**
** Alternatively, the client may call the function with a "NULL" buffer
** pointer.  In this case, the video format services will count the number of
** bytes that would have been read, but not actually store them anywhere.  The
** client can use this byte count to allocate enough space to actually store
** the data, and then call the same function again, this time with a non-NULL
** buffer pointer.
**
** After the data is completely read, the video format services will return a
** status code of VIDEO_FMT_IO_DONE, and indicate the total number of bytes
** read (or would have been read, in the case where the buffer pointer is
** NULL). At this point, but not before, the client may initiate another read
** request. While reading, the client continues the reading through the
** video_fmt_continue_cb_func_type callback. At any point, the client may exit
** video format services through the video_fmt_end_cb_func_type callback.
*/
typedef void (*video_fmt_read_cb_func_type) (
  uint32                         stream_number,
  video_fmt_data_unit_type       unit,
  uint64                         offset,        /* in units              */
  uint64                         size,          /* in units              */
  uint8                          *buffer,       /* where to store stream */
  void                           *server_data,
  video_fmt_status_cb_func_type  callback_ptr,
  void                           *client_data
);

/* This callback function type is used to ask the video format services to
** return information about one or more samples in a stream.  It is given
** to the client in the server data for the VIDEO_FMT_INFO status callback,
** when the video format services is finished decoding the media meta-data and
** is ready to begin demuxing stream data.
**
** Note that it is the responsibility of the caller to allocate enough memory
** for the buffer pointed to by the "buffer" parameter to fit the given number
** of sample information structures.
**
** Alternatively, the client may call the function with a "NULL" buffer
** pointer.  In this case, the video format services will count the number of
** bytes that would have been read, but not actually store them anywhere.  The
** client can use this byte count to allocate enough space to actually store
** the data, and then call the same function again, this time with a non-NULL
** buffer pointer.
**
** After the data is completely read, the video format services will return a
** status code of VIDEO_FMT_IO_DONE, and indicate the total number of bytes
** read (or would have been read, in the case where the buffer pointer is
** NULL). At this point, but not before, the client may initiate another read
** request. While reading, the client continues the reading through the
** video_fmt_continue_cb_func_type callback. At any point, the client may exit
** video format services through the video_fmt_end_cb_func_type callback.
*/
typedef void (*video_fmt_sample_info_cb_func_type) (
  uint32                         stream_number,
  uint64                         offset,        /* in samples            */
  uint64                         size,          /* in samples            */
  video_fmt_sample_info_type     *buffer,       /* where to store info   */
  void                           *server_data,
  video_fmt_status_cb_func_type  callback_ptr,
  void                           *client_data
);

/* This callback function type is used for parsing the fragment.
** It is given to the client in the server data of the VIDEO_FMT_FRAGMENT status callback.
** The "server_data" here must be the same as the "server_data" given to the client
** during the last status callback.
*/
typedef void (*video_fmt_parse_fragment_cb_func_type) (void *server_data);


/* This callback function type is used to return the size of the fragment.
** It is given to the client in the server data of the VIDEO_FMT_FRAGMENT_SIZE
** status callback.
** The "server_data" here must be the same as the "server_data" given to the client
** during the last status callback.
*/
typedef void (*video_fmt_fragment_size_cb_func_type) (void *server_data,
                                                      uint32 fragment_number);

/* This callback function type is used to return the size of the largest frame.
** The "server_data" here must be the same as the "server_data" given to the client
** during the last status callback.
*/
typedef void (*video_fmt_largest_frame_size_cb_func_type) (void *server_data, uint32 );

typedef void (*video_fmt_abs_file_offset_cb_func_type) (
  uint32                         stream_number,
  uint64                         sampleOffset,        /* in units              */
  uint32                         sampleSize,          /* in units              */
  void                           *server_data,
  video_fmt_status_cb_func_type  callback_ptr,
  void                           *client_data
);

/* This callback function type is used to ask the video format services to
** find the nearest Access Point at or after the given timestamp.
** It is given to the client in the server data for the VIDEO_FMT_INFO
** status callback, when the video format services is finished decoding the
** media meta-data and is ready to begin demuxing stream data.
**
** After the data is completely filled in, the video format services will
** return a status code of VIDEO_FMT_IO_DONE, and indicate number of bytes
** read to be sizeof(video_fmt_sample_info_type) or zero.  If zero, this
** indicates no sync sample was found at or after the given sample.
** Otherwise, the next sync sample's information is stored in the given
** buffer.
**
** At the point the VIDEO_FMT_IO_DONE status code is returned, but not before,
** the client may initiate another request.  While reading, the client
** continues the reading through the video_fmt_continue_cb_func_type
** callback. At any point, the client may exit video format services through
** the video_fmt_end_cb_func_type callback.
*/
typedef void (*video_fmt_access_point_cb_func_type) (
  uint32                         stream_number,
  uint64                         sample_timestamp,
  int32                          skipNumber,
  boolean                        reverse,       /* TRUE if seek backward */
  video_fmt_tfra_entry_type      *buffer,       /* where to store info   */
  void                           *server_data,
  video_fmt_status_cb_func_type  callback_ptr,
  void                           *client_data
);
/* This callback function type is used to ask the video format services to
** find the nearest sync sample at or after the given sample in the given
** stream. It is given to the client in the server data for the VIDEO_FMT_INFO
** status callback, when the video format services is finished decoding the
** media meta-data and is ready to begin demuxing stream data.
**
** After the data is completely filled in, the video format services will
** return a status code of VIDEO_FMT_IO_DONE, and indicate number of bytes
** read to be sizeof(video_fmt_sample_info_type) or zero.  If zero, this
** indicates no sync sample was found at or after the given sample.
** Otherwise, the next sync sample's information is stored in the given
** buffer.
**
** At the point the VIDEO_FMT_IO_DONE status code is returned, but not before,
** the client may initiate another request.  While reading, the client
** continues the reading through the video_fmt_continue_cb_func_type
** callback. At any point, the client may exit video format services through
** the video_fmt_end_cb_func_type callback.
*/
typedef void (*video_fmt_sync_sample_cb_func_type) (
  uint32                         stream_number,
  uint64                         offset,        /* in samples            */
  boolean                        reverse,       /* TRUE if seek backward */
  video_fmt_sample_info_type     *buffer,       /* where to store info   */
  void                           *server_data,
  video_fmt_status_cb_func_type  callback_ptr,
  void                           *client_data
);

/* This callback function type is used to ask the video format services to
** start writing more data to one of the streams in the media file.  It is
** given to the client in the server data for the VIDEO_FMT_WRITER_READY
** status callback, when the video format services is finished opening a new
** movie file to be written and is ready to begin muxing stream data.
**
** If the information in the video_fmt_stream_create_params_type structure
** for the stream indicated a fixed sample size, delta time, and lack of
** inter-frames, the "sample_info" parameter may be omitted.  Otherwise, for
** every sample of data written to the stream, the client should provide one
** entry in the given sample_info array.  In this entry, all fields except
** "sample", "offset", and "time" should be specified.  These three fields are
** not necessary when writing streams.
**
** After the data is completely written, the video format services will return
** a status code of VIDEO_FMT_IO_DONE, and indicate the total number of bytes
** written. At this point, but not before, the client may initiate another
** write request.  While writing, the client continues the writing through the
** video_fmt_continue_cb_func_type callback. At any point, the client may exit
** video format services through the video_fmt_end_cb_func_type callback.
**
** When VIDEO_FMT_IO_DONE is received, if the movie being written is
** fragmented, the client may refer to the "fragment_done" flag in the
** information structure given to the callback function, to determine whether
** the current fragment has been completed.  The client is free to reorder the
** data already written out for the last fragment at this point.
**
** For timed text streams, you can use this callback function to write raw
** text sample date to the stream.  For samples constructed with the
** video_fmt_alloc_text_cb_func_type callback and (optionally) modified by the
** video_fmt_modify_*_text_cb_func_type callbacks, do not use this callback to
** write the text sample.  Use video_fmt_write_text_cb_func_type instead.
*/
typedef void (*video_fmt_write_cb_func_type) (
  uint32                            stream_number,
  uint32                            num_samples,
  const video_fmt_sample_info_type  *sample_info,
  const uint8                       *sample_data,
  void                              *server_data,
  video_fmt_status_cb_func_type     callback_ptr,
  void                              *client_data
);

/* This callback function type is used for writing a formatting and writing a
** sample to a timed text track.  The sample is deallocated automatically when
** it is written.
*/
typedef void (*video_fmt_write_text_cb_func_type) (
  uint32                            stream_number,
  void                              *handle,    /* allocated sample handle  */
  uint32                            delta,
  void                              *server_data,
  video_fmt_status_cb_func_type     callback_ptr,
  void                              *client_data
);

/* This callback function type is used to ask the video format services to
** start writing more data to the decoder specific information (header) of one
** of the streams in the media file.  It is given to the client in the server
** data for the VIDEO_FMT_WRITER_READY status callback, when the video format
** services is finished opening a new movie file to be written and is ready to
** begin muxing stream data.
**
** After the data is completely written, the video format services will return
** a status code of VIDEO_FMT_IO_DONE, and indicate the total number of bytes
** written. At this point, but not before, the client may initiate another
** write request.  While writing, the client continues the writing through the
** video_fmt_continue_cb_func_type callback. At any point, the client may exit
** video format services through the video_fmt_end_cb_func_type callback.
**
** For timed text streams, this callback function is handled differently.
** Instead of writing data into the "decoder specific information" for the
** track, the data is interpreted as the raw contents of a 'tx3g' atom to
** write to the meta data.
**
** To generate and write a 'tx3g' atom based on a "video_fmt_text_type"
** structure, do not use this callback.  Use
** video_fmt_write_header_text_cb_func_type intead.
*/
typedef void (*video_fmt_write_header_cb_func_type) (
  uint32                            stream_number,
  uint32                            num_bytes,
  const uint8                       *header_data,
  void                              *server_data,
  video_fmt_status_cb_func_type     callback_ptr,
  void                              *client_data
);

/* This callback function type is used to ask the video format services to
** generate and write a text stream sample entry ('tx3g') or header atom.
** It is given to the client in the server data for the VIDEO_FMT_WRITER_READY
** status callback, when the video format services is finished opening a new
** movie file to be written and is ready to begin muxing stream data.
**
** After the data is completely written, the video format services will return
** a status code of VIDEO_FMT_IO_DONE, and indicate the total number of bytes
** written. At this point, but not before, the client may initiate another
** write request.  While writing, the client continues the writing through the
** video_fmt_continue_cb_func_type callback. At any point, the client may exit
** video format services through the video_fmt_end_cb_func_type callback.
**
** Each call to the function writes a new ('tx3g') header, which applies to
** all samples written after the call, up until the next header (if any) is
** written.
**
** Do not use this callback to write already-prepared 'tx3g' atom contents.
** Use the video_fmt_write_header_cb_func_type callback instead.
*/
typedef void (*video_fmt_write_header_text_cb_func_type) (
  uint32                              stream_number,
  const struct video_fmt_text_type_t  *header_data,
  void                                *server_data,
  video_fmt_status_cb_func_type       callback_ptr,
  void                                *client_data
);

/* This callback function type is used to ask the video format services to
** start writing more data to append to the last sample (footer) of one
** of the streams in the media file.  It is given to the client in the server
** data for the VIDEO_FMT_WRITER_READY status callback, when the video format
** services is finished opening a new movie file to be written and is ready to
** begin muxing stream data.
**
** After the data is completely written, the video format services will return
** a status code of VIDEO_FMT_IO_DONE, and indicate the total number of bytes
** written. At this point, but not before, the client may initiate another
** write request.  While writing, the client continues the writing through the
** video_fmt_continue_cb_func_type callback. At any point, the client may exit
** video format services through the video_fmt_end_cb_func_type callback.
**
*/
typedef void (*video_fmt_write_footer_cb_func_type) (
  uint32                            stream_number,
  uint32                            num_bytes,
  const uint8                       *footer_data,
  void                              *server_data,
  video_fmt_status_cb_func_type     callback_ptr,
  void                              *client_data
);

/* This callback function type is used to ask the video format services to
** start writing a user-defined information atom to the movie file.  It is
** given to the client in the server data for the VIDEO_FMT_WRITER_READY
** status callback, when the video format services is finished opening a new
** movie file to be written and is ready to begin muxing stream data.
**
** The client provides the 16-byte UUID of the user atom, and the data to be
** written to the user-defined atom.
**
** After the data is completely written, the video format services will return
** a status code of VIDEO_FMT_IO_DONE, and indicate the total number of bytes
** written. At this point, but not before, the client may initiate another
** write request.  While writing, the client continues the writing through the
** video_fmt_continue_cb_func_type callback. At any point, the client may exit
** video format services through the video_fmt_end_cb_func_type callback.
*/
typedef void (*video_fmt_write_uuid_cb_func_type) (
  const uint8                    *uuid,  /* must point to 16-element array */
  const void                     *data,  /* data for atom contents         */
  uint32                         size,   /* size of atom contents          */
  void                           *server_data,
  video_fmt_status_cb_func_type  callback_ptr,
  void                           *client_data
);

/* This callback function type is used to ask for the pointer to the stream
** buffer with its size of the video format services. The client can directly
** access this buffer and write data in it.
**
** Input arguments of particular interests:
**  stream_number - The number of the stream whose buffer is being asked for.
**  offset        - The buffer pointer advances by offset from the current
**                  position before it is returned to the client.
**  bLarge        - Set to TRUE only if the buffer size this function
**                  previously returned was not large enough.
**                  Set to FALSE, otherwise
**  size          - Pointer where the buffer size is returned.
**                  NULL acceptable.
**
** It is given to the client in the server
** data for the VIDEO_FMT_WRITER_READY status callback, when the video format
** services is finished opening a new movie file to be written and is ready to
** begin muxing stream data.
**
*/
typedef uint8* (*video_fmt_seek_cb_func_type) (
  uint32                            stream_number,
  uint32                            offset,
  boolean                           bLarge,
  uint32                            *size,
  void                              *server_data,
  video_fmt_status_cb_func_type     callback_ptr,
  void                              *client_data
);

/* This callback function type is used to construct a new sample for a timed
** text stream.  It assembles a sample containing just the given text string,
** with no modifiers.  To add modifiers, call the
** video_fmt_modify_*_text_cb_func_type callback functions, passing the
** handle returned by this callback.  Once all modifiers are applied to a
** sample, write it to the track using the video_fmt_write_text_cb_func_type
** callback.  This automatically deallocates the sample.  If a sample needs to
** be deallocated without writing it, pass it to the
** video_fmt_free_text_cb_func_type callback.
*/
typedef void * (*video_fmt_alloc_text_cb_func_type) (
  const char                     *data,  /* string to put into sample    */
  uint16                         size,   /* size of data string in bytes */
  void                           *server_data,
  video_fmt_status_cb_func_type  callback_ptr,
  void                           *client_data
);

/* This callback function type is used to release a timed text stream sample
** without writing it to the stream.  If the sample is written to the
** video_fmt_write_text_cb_func_type callback, do not pass it to this
** callback, as the write_text callback automatically releases it after it
** is written.
*/
typedef void (*video_fmt_free_text_cb_func_type) (
  void                           *handle,    /* allocated sample handle     */
  video_fmt_status_cb_func_type  callback_ptr,
  void                           *client_data
);

/* This callback function type is used to apply a style modifier to a text
** sample constructed with the video_fmt_alloc_text_cb_func_type
** callback, before writing it to the track with the
** video_fmt_write_text_cb_func_type callback.
*/
typedef void (*video_fmt_modify_style_text_cb_func_type) (
  uint16                           num_entries,
  const video_fmt_text_style_type  *entries,
  void                             *handle,    /* allocated sample handle   */
  video_fmt_status_cb_func_type    callback_ptr,
  void                             *client_data
);

/* This callback function type is used to apply a highlight modifier to a text
** sample constructed with the video_fmt_alloc_text_cb_func_type
** callback, before writing it to the track with the
** video_fmt_write_text_cb_func_type callback.
*/
typedef void (*video_fmt_modify_hl_text_cb_func_type) (
  uint16                         start_char, /* starting offset in string   */
  uint16                         end_char,   /* ending offset in string     */
  void                           *handle,    /* allocated sample handle     */
  video_fmt_status_cb_func_type  callback_ptr,
  void                           *client_data
);

/* This callback function type is used to apply a highlight color modifier to
** a text sample constructed with the video_fmt_alloc_text_cb_func_type
** callback, before writing it to the track with the
** video_fmt_write_cb_text_func_type callback.
*/
typedef void (*video_fmt_modify_hl_color_text_cb_func_type) (
  uint32                         color,      /* 0xrrggbbaa where rr=red,    */
                                             /* gg=green, bb=blue, aa=alpha */
  void                           *handle,    /* allocated sample handle     */
  video_fmt_status_cb_func_type  callback_ptr,
  void                           *client_data
);

/* This callback function type is used to apply a dynamic highlighting
** (karaoke) modifier to a text sample constructed with the
** video_fmt_alloc_text_cb_func_type callback, before writing it to the
** track with the video_fmt_write_text_cb_func_type callback.
*/
typedef void (*video_fmt_modify_karaoke_text_cb_func_type) (
  uint32                            start_time,
  uint16                            num_entries,
  const video_fmt_karaoke_box_type  *entries,
  void                              *handle,    /* allocated sample handle  */
  video_fmt_status_cb_func_type     callback_ptr,
  void                              *client_data
);

/* This callback function type is used to apply a scroll delay modifier to a
** text sample constructed with the video_fmt_alloc_text_cb_func_type
** callback, before writing it to the track with the
** video_fmt_write_text_cb_func_type callback.
*/
typedef void (*video_fmt_modify_scroll_text_cb_func_type) (
  uint32                         scroll_delay,
  void                           *handle,    /* allocated sample handle     */
  video_fmt_status_cb_func_type  callback_ptr,
  void                           *client_data
);

/* This callback function type is used to apply a hyperlink modifier to a
** text sample constructed with the video_fmt_alloc_text_cb_func_type
** callback, before writing it to the track with the
** video_fmt_write_text_cb_func_type callback.
*/
typedef void (*video_fmt_modify_link_text_cb_func_type) (
  uint16                         start_char, /* starting offset in string   */
  uint16                         end_char,   /* ending offset in string     */
  const char                     *url,       /* URL for hyperlink           */
  uint8                          url_length, /* length of URL in bytes      */
  const char                     *alt,       /* alternate text for link     */
  uint8                          alt_length, /* length of alt text in bytes */
  void                           *handle,    /* allocated sample handle     */
  video_fmt_status_cb_func_type  callback_ptr,
  void                           *client_data
);

/* This callback function type is used to apply a text box modifier to a
** text sample constructed with the video_fmt_alloc_text_cb_func_type
** callback, before writing it to the track with the
** video_fmt_write_text_cb_func_type callback.
*/
typedef void (*video_fmt_modify_box_text_cb_func_type) (
  int16                          left,
  int16                          top,
  int16                          right,
  int16                          bottom,
  void                           *handle,    /* allocated sample handle     */
  video_fmt_status_cb_func_type  callback_ptr,
  void                           *client_data
);

/* This callback function type is used to apply a blink modifier to a
** text sample constructed with the video_fmt_alloc_text_cb_func_type
** callback, before writing it to the track with the
** video_fmt_write_text_cb_func_type callback.
*/
typedef void (*video_fmt_modify_blink_text_cb_func_type) (
  uint16                         start_char, /* starting offset in string   */
  uint16                         end_char,   /* ending offset in string     */
  void                           *handle,    /* allocated sample handle     */
  video_fmt_status_cb_func_type  callback_ptr,
  void                           *client_data
);

/* This callback function type is used to apply a text wrap modifier to a
** text sample constructed with the video_fmt_alloc_text_cb_func_type
** callback, before writing it to the track with the
** video_fmt_write_text_cb_func_type callback.
*/
typedef void (*video_fmt_modify_wrap_text_cb_func_type) (
  boolean                        automatic,
  void                           *handle,    /* allocated sample handle     */
  video_fmt_status_cb_func_type  callback_ptr,
  void                           *client_data
);
/*
** This callback function updates the titl,auth,descp.
** Can be called by client during recording session.
*/

typedef void (*video_fmt_modify_user_atoms_cb_func_type) (
  video_fmt_user_data_type             *puser_data,
  void                                 *handle,
  video_fmt_status_cb_func_type        callback_ptr,
  void                                 *client_data
);
/* These structures each correspond to one of the structures in the union
** pointed to by "info" in the "video_fmt_status_cb_func_type" callback, for
** each of the possible values for the "status" parameter.
*/

typedef struct {
  uint32                           size;    /* number of bytes to allocate */
  void                             *ptr;    /* replace with ptr to memory  */
} video_fmt_alloc_type;

typedef struct {
  void                             *ptr;    /* pointer to memory to free   */
} video_fmt_free_type;

typedef struct {
  uint8                            *buffer;      /* where to store data    */
  uint64                           offset;       /* offset into media file */
  uint64                           num_bytes;    /* client should replace  */
                                                 /*   with actual number   */
                                                 /*   of bytes read        */
  boolean                          bEndOfData;
  video_fmt_continue_cb_func_type  callback_ptr;
  void                             *server_data;
} video_fmt_get_data_type;

typedef struct {
  const uint8                      *buffer;      /* where to retrieve data */
  uint32                           offset;       /* offset into media file */
  uint32                           num_bytes;
  video_fmt_continue_cb_func_type  callback_ptr;
  void                             *server_data;
} video_fmt_put_data_type;

typedef struct {
  uint8                            *buffer;      /* where to store data    */
  uint32                           id;           /* ID of data block       */
  uint32                           num_bytes;    /* client should replace  */
                                                 /*   with actual number   */
                                                 /*   of bytes read        */
  video_fmt_continue_cb_func_type  callback_ptr;
  void                             *server_data;
} video_fmt_get_meta_data_type;

typedef struct {
  const uint8                      *buffer;      /* where to retrieve data */
  uint32                           id;           /* ID of data block       */
                                                 /* client should set      */
                                                 /* when storing the data  */
  uint32                           num_bytes;
  video_fmt_continue_cb_func_type  callback_ptr;
  void                             *server_data;
} video_fmt_put_meta_data_type;

typedef struct {
  video_fmt_continue_cb_func_type  callback_ptr;
  void                             *server_data;
} video_fmt_continue_type;

typedef struct {
  uint32                              num_streams;
  video_fmt_stream_info_type          *streams;
  video_fmt_file_info_type            file_info;
  video_fmt_read_cb_func_type         read_cb;
  video_fmt_sample_info_cb_func_type  sample_info_cb;
  video_fmt_sync_sample_cb_func_type  sync_sample_cb;
  video_fmt_parse_fragment_cb_func_type parse_fragment_cb;
  video_fmt_fragment_size_cb_func_type fragment_size_cb;
  video_fmt_fragment_size_cb_func_type fragment_size_peek_cb;
  video_fmt_largest_frame_size_cb_func_type largest_frame_size_cb;
  uint64                              abs_file_offset;
  video_fmt_abs_file_offset_cb_func_type         abs_file_offset_cb;
  video_fmt_access_point_cb_func_type access_point_cb;
  void                                *server_data;
} video_fmt_info_type;

typedef struct {
  video_fmt_write_cb_func_type                 write_cb;
  video_fmt_write_text_cb_func_type            write_text_cb;
  video_fmt_write_header_cb_func_type          write_header_cb;
  video_fmt_write_header_text_cb_func_type     write_header_text_cb;
  video_fmt_write_footer_cb_func_type          write_footer_cb;
  video_fmt_write_uuid_cb_func_type            write_uuid_cb;
  video_fmt_seek_cb_func_type                  seek_cb;
  video_fmt_alloc_text_cb_func_type            alloc_text_cb;
  video_fmt_free_text_cb_func_type             free_text_cb;
  video_fmt_modify_style_text_cb_func_type     modify_style_text_cb;
  video_fmt_modify_hl_text_cb_func_type        modify_hl_text_cb;
  video_fmt_modify_hl_color_text_cb_func_type  modify_hl_color_text_cb;
  video_fmt_modify_karaoke_text_cb_func_type   modify_karaoke_text_cb;
  video_fmt_modify_scroll_text_cb_func_type    modify_scroll_text_cb;
  video_fmt_modify_link_text_cb_func_type      modify_link_text_cb;
  video_fmt_modify_box_text_cb_func_type       modify_box_text_cb;
  video_fmt_modify_blink_text_cb_func_type     modify_blink_text_cb;
  video_fmt_modify_wrap_text_cb_func_type      modify_wrap_text_cb;
  video_fmt_modify_user_atoms_cb_func_type     modify_user_atoms_cb;
  void                                         *server_data;
} video_fmt_writer_ready_type;

/* fragment_done - this is used only when writing data to a movie file, and
** only if the file being written is using a fragmented file format.  If TRUE,
** it indicates the end of the last fragment was written out, and the next
** data to be written out will be for the next fragment.  The client uses this
** flag to determine when it can go reorder the atoms written out in the
** previous step.
*/
typedef struct {
  uint64                               bytes;
  boolean                              fragment_done;
} video_fmt_io_done_type;

typedef struct {
  uint64                               bytes_available;
} video_fmt_space_check_type;

typedef struct {
  uint32                               bytes_consumed_by_meta_data;
} video_fmt_space_consumed_type;

typedef struct {
  video_fmt_stream_info_type *stream_info;
  video_fmt_mp4_atom_type *mp4;       /* VIDEO_FMT_MP4       */
} video_fmt_hint_type;

/* total_duration - this is returned only when completing the writing of a
** movie file.  This indicates the total duration of the movie, in the movie
** timescale.  For fragmented files, the client should write this value back
** into the movie file, in the 'mehd' atom.  The value should be written at
** byte offset 8-11 of the atom, in big-endian order:
**     offset 0-3:   'm' 'e' 'h' 'd'
**     offset 4:     <version of atom - should be 0>
**     offset 5-7:   <flags of atom - should be 0>
**     offset 8-11:  --> total movie duration  <--
** The video format services cannot write this field automatically because the
** client is free to reorder the atoms in any fragments that have been fully
** written out, and this field is located in the very first fragment, at which
** time the video format services does not know what the total duration will
** be.
*/
typedef struct {
  uint32                           total_duration;
  video_fmt_space_out_type         space_out_status; /* VIDEO_FMT_LIMIT_NEAR */
                                                     /* and VIDEO_FMT_LIMIT_IMMINENT */
                                                     /* and VIDEO_FMT_LIMIT_REACHED  */
} video_fmt_done_type;

typedef struct {
  uint64                           offset;       /* offset into media file */
  uint32                           size;         /* atom size              */
  uint32                           atom_type;    /* type of user atom      */
} video_fmt_uuid_data_type;

typedef struct {
  video_fmt_stream_text_type       format;     /* text format type        */
  uint32                           stream;     /* index of current stream */
  uint64                           offset;     /* offset into media file  */
  uint32                           size;       /* atom size               */
  uint32                           cur_index;  /* index of current atom   */
  uint32                           num_atoms;  /* total number of tx3g/stpp atoms */
} video_fmt_text_data_type;

typedef struct {
  video_fmt_clipinfo_type          *data;
}video_fmt_clip_data_type;

/*shahed-typedef enum
{
   VIDEO_FMT_SPACE_MSG_INFO_UNAVAILABLE,
   VIDEO_FMT_SPACE_MSG_SAMPLE_TABLE,
   VIDEO_FMT_SPACE_MSG_CHUNK_TABLE,
   VIDEO_FMT_SPACE_MSG_STORAGE,
   VIDEO_FMT_SPACE_MSG_MOVIE_DURATION
} video_fmt_space_out_type;*/

/* This Structure Provides information to fetch File Size which is required to
** find out whether MFRA is present or not.
*/
typedef struct {
  /* if fileSize is 0, then it is not available as in HTTP Streaming*/
  uint64                           fileSize;
} video_fmt_file_size;

/* This union is used to represent one of the many status callback information
** structures.  The status code set when each field is used is indicated.
*/
typedef union video_fmt_status_cb_info_type_t {
  video_fmt_alloc_type         alloc;         /* VIDEO_FMT_ALLOC         */
  video_fmt_free_type          free;          /* VIDEO_FMT_FREE          */
  video_fmt_get_data_type      get_data;      /* VIDEO_FMT_GET_DATA      */
  video_fmt_put_data_type      put_data;      /* VIDEO_FMT_PUT_DATA      */
  video_fmt_get_meta_data_type get_meta_data; /* VIDEO_FMT_GET_META_DATA */
  video_fmt_put_meta_data_type put_meta_data; /* VIDEO_FMT_PUT_META_DATA */
  video_fmt_continue_type      cont;          /* VIDEO_FMT_CONTINUE      */
  video_fmt_info_type          info;          /* VIDEO_FMT_INFO          */
                                              /* and VIDEO_FMT_FRAGMENT  */
  video_fmt_writer_ready_type  writer_ready;  /* VIDEO_FMT_WRITER_READY  */
  video_fmt_io_done_type       io_done;       /* VIDEO_FMT_IO_DONE       */
  video_fmt_space_check_type   space_check;   /* VIDEO_FMT_SPACE_CHECK   */
  video_fmt_space_consumed_type   space_consumed;   /* VIDEO_FMT_SPACE_CONSUMED   */
  video_fmt_uuid_data_type     uuid_atom;     /* VIDEO_FMT_UUID          */
  video_fmt_text_data_type     text_atom;     /* VIDEO_FMT_TEXT          */
  video_fmt_done_type          done;          /* VIDEO_FMT_DONE          */
  video_fmt_hint_type          hint;          /* VIDEO_FMT_HINT          */
  video_fmt_file_size          fSize;         /* VIDEO_FMT_FILESIZE      */
  video_fmt_user_data_type     user_data;     /* VIDEO_FMT_TITL_ATOM_REQ */
  video_fmt_clip_data_type     clipinfo;      /* VIDEO_FMT_CLIPINFO_REQ  */
} video_fmt_status_cb_info_type;

/* This enumerated type lists the different text justification settings. */
typedef enum {
    VIDEO_FMT_JUSTIFY_LEFT_OR_TOP,
    VIDEO_FMT_JUSTIFY_CENTER,
    VIDEO_FMT_JUSTIFY_RIGHT_OR_BOTTOM
} video_fmt_justify_type;

/* This enumerated type lists the different types of scrolling that are
** defined for timed text.
*/
typedef enum {
    VIDEO_FMT_SCROLL_VERT_UP,
    VIDEO_FMT_SCROLL_HORZ_LEFT,
    VIDEO_FMT_SCROLL_VERT_DOWN,
    VIDEO_FMT_SCROLL_HORZ_RIGHT
} video_fmt_scroll_type;

/* This structure provides information about a font for the font table of a
** timed text stream.
*/
typedef struct video_fmt_font_type_t {
  int16                         font_id;
  const char                    *font_name;
} video_fmt_font_type;

/* This structure provides information necessary to construct a sample entry
** atom for a timed text track.
*/
typedef struct video_fmt_text_type_t {
  boolean                    scroll_in;
  boolean                    scroll_out;
  video_fmt_scroll_type      scroll_type;
  boolean                    continuous_karaoke;
  boolean                    write_vertically;
  video_fmt_justify_type     horz_justify;
  video_fmt_justify_type     vert_justify;
  uint32                     back_color;    /* 0xrrggbbaa where rr=red,    */
                                            /* gg=green, bb=blue, aa=alpha */
  int16                      default_box_left;
  int16                      default_box_top;
  int16                      default_box_right;
  int16                      default_box_bottom;
  uint16                     font_id;
  boolean                    bold;
  boolean                    italic;
  boolean                    underline;
  uint8                      font_size;
  uint32                     text_color;    /* 0xrrggbbaa where rr=red,    */
                                            /* gg=green, bb=blue, aa=alpha */
  uint16                     num_fonts;
  const video_fmt_font_type  *fonts;
} video_fmt_text_type;

/* This structure contains information about a movie stream to be written in a
** movie file, to give the movie writer an idea how to write the stream.
**
** subinfo - The only fields used in this union are the format type fields,
** and the width and height for video.  Other information in the union, such
** as audio parameters, are not used and may be left uninitialized by the
** client.
**
** priority - This defines the importance of the stream relative to the other
** streams in the file.  For example, in the MP4 file format, this number is
** stored in the "streamPriority" field of the 'esds' atom for the stream, if
** the format of the stream indicates the usage of an MPEG-4 video, audio, or
** data stream.
**
** handler - This is a displayable NULL-terminated string, typically used to
** label the meta-data for the track to indicate what is supposed to handle
** the stream data.  For example, in the MP4 file format, this string is
** stored in the 'hdlr' atom for the meta-data track for the stream.  Note
** that the video format services holds onto the given pointer, rather than
** making its own internal copy.  Therefore the client should keep the string
** locked in memory until the movie file has been completely written.
**
** interlace - If set to the same number as the track's index in the list of
** tracks, this track does not force another track to be interlaced with it.
** Otherwise, the number indicates the stream number of the track to
** interlace with this track.  Interleaving a track forces the data cached in
** the other track to be packaged as a chunk and written out after every chunk
** of this track.  If enabled, the chunks of both tracks are synchronized and
** written out together.  This means the client must make sure to write out
** the data for both affected tracks at the same rate of stream time.
** Otherwise, there is a possibility that one track will have  empty chunks
** written out, or may overflow its buffer before the next chunk is written
** out.  Even if interlace mode is not used, there will still be physical
** interleaving of data if the client alternates between tracks when writing
** samples of data.  In this case, however, the interleaving will be based
** solely on the desired chunk sizes, amount of data written for each track,
** and the order in which tracks are written.
**
** interlace_rate - If this track is set up to interlace another track, this
** is the rate, in media timescale units, at which the data is interlaced.
** This will override the value of "chunk_size" as interlacing is done by
** forming chunks with sizes set according to the interlace rate.
**
** chunk_size - For file formats which group sequential samples in larger
** groups called "chunks", this is the desired target chunk size in bytes.
** If the stream is being interlaced with another stream, this value is
** ignored.  Otherwise, it should be set to a value lower than the value of
** buffer_size minus the expected largest amount of data to be written in one
** write call, to prevent a stream buffer overflow.
**
** buffer_size - This is the maximum number of bytes to allocate for caching
** bitstream data before writing it out to the file.  It must be larger than
** chunk_size plus the expected largest amount of data to be written in one
** write call, or large enough to hold the amount of data generated at the
** interlace rate, if the stream is interlaced.  Otherwise, there is a risk of
** buffer overflow when writing data to the stream.
**
** sample_size - This is the size of each sample, if the samples in this
** stream are of a constant fixed size.  Otherwise, this should be set to
** zero.
**
** sample_delta - This is the amount of time each sample adds to the stream,
** if the time distance between frames is constant (fixed rate).  Otherwise,
** this should be set to zero.
**
** max_samples, max_chunks - These define the trade-off between memory usage
** and maximum movie file size.  As the meta-data is constructed in memory as
** bitstream is written to the file, these define limits on the size of this
** meta-data.
**
** max_samples_rate - This is the number of samples per second.  This should
** be set to no less than the maximum sample rate to calculate the accurate
** maximum bit rate only if max_table_stores > 0 and fragmentation is disabled.
** If it is set too small, maximum bit rate set in the file may be smaller
** than the actual maximum bit rate but there's no other impact.  If it is set
** too large, then memory and cpu cycles are wasted. If max_table_stores == 0,
** this is not used at all.
**
** max_table_stores - This defines how many sample/chunk table blocks can be
** stored in a temporary meta data storage.  If this is 0, no sample/chunk
** table block can be stored in a temporary meta data storage and video format
** services end when sample/chunk table space in memory is used up.  If this
** is greater than 0, up to (max_table_stores + 2) sample/chunk table blocks
** can be stored in a temporary meta data storage combined.
**
** samples_out_near_threshold, chunks_out_near_threshold - These define
** threshold values under which the file writer sends indications to the
** client that it is getting near running out of sample table or chunk table
** resources, respectively.
**
** samples_out_imminent_threshold, chunks_out_imminent_threshold - These
** define threshold values under which the file writer sends indications to
** the client that it is just about to run out of sample table or chunk
** table resources, respectively (even closer than the "near" thresholds).
**
** stsc_reset_rate - For file formats such as MP4, this defines the rate, in
** seconds, at which to break off from trying to maintain a consistent ratio
** of samples to chunks.  A consistent ratio makes part of the meta-data
** smaller for formats such as MP4, but may lead to an inefficient ratio.
** Resetting the algorithm regularly sets a balance between meta-data size and
** storage efficiency.
**
** mode_change_restriction - This is used for AMR audio, and according to the
** 3GPP2 file format specification, "defines a number N, which restricts the
** mode changes only at a multiple of N frames".  This number is basically
** copied into the AMRSampleEntry atom for the stream.
**
** frames_per_sample - This is used for AMR and EVRC audio, and according to
** the 3GPP2 file format specification, "defines the number of frames to be
** considered as 'one sample' inside the 3GPP2 file".  This number is
** basically copied into the AMRSampleEntry or EVRCSampleEntry atom for the
** stream.
**
** layer - This is used for visually composed tracks (video, text, etc.)  It
** defines the stacking order of visually composed tracks in the movie.  The
** more negative a layer number is, the closer to the viewer the track is.
**
** width, height, origin_x, origin_y - These are used for visually composed
** tracks (video, text, etc.)  They define the size of the visual output for
** the track, and the offset of its top-left corner relative to the origin
** reference point (typically the top-left corner of the video track).  They
** are in units of 65536ths of a pixel (e.g. 0x00010000 == 1 pixel).
**
** language - This is the packed ISO-639-2/T language code to put into the
** 'mdhd' atom for the track.  Leave as 0 to indicate "undetermined".
**
** match_duration - If set, this stream is lengthened or shortened in order
** for its duration to match that of the stream to which it is linked.
**
** match_duration_with - If "match_duration" is set, this is the stream with
** which to link this stream in terms of duration.
*/
typedef struct video_fmt_stream_create_params_type_t {
  video_fmt_stream_type          type;      /* video, audio, etc.          */
  video_fmt_stream_subinfo_type  subinfo;   /* type-specific media info    */
  uint8                          priority;  /* relative stream priority    */
  const char                     *handler;  /* name for handler reference  */
  uint32                         media_timescale; /* media-specific time   */
                                                  /* units per second      */
  video_fmt_ref_track_type       ref_track; /* tracks referenced by stream */
  uint32                         interlace; /* other track with which to   */
                                            /* interlace data              */
  uint32                         interlace_rate;   /* in timescale units   */
  uint32                         chunk_size; /* optimal chunk size         */
                                             /* in bytes                   */
  uint32                         buffer_size; /* maximum buffer size       */
                                              /* in bytes                  */
  uint32                         max_header; /* space to reserve for head  */
  uint32                         max_footer; /* space to reserve for foot  */
  uint32                         sample_size;
  uint32                         sample_delta;
  boolean                        inter_frames; /* TRUE only if some        */
                                               /* samples can be non-sync  */
  uint32                         max_samples;
  uint32                         max_chunks;
  uint32                         max_samples_rate;
  uint32                         max_table_stores;
  uint32                         samples_out_near_threshold;
  uint32                         chunks_out_near_threshold;
  uint32                         samples_out_imminent_threshold;
  uint32                         chunks_out_imminent_threshold;
  uint32                         stsc_reset_rate;
  uint8                          mode_change_restriction;
  uint8                          frames_per_sample;
  int16                          layer;
  uint32                         width;
  uint32                         height;
  uint32                         origin_x;
  uint32                         origin_y;
  uint16                         language;
  boolean                        match_duration;
  uint32                         match_duration_with;
  uint32                         interlace_duration;
} video_fmt_stream_create_params_type;

/* This structure contains information about a movie file to be created, to
** give the movie writer an idea how to write the file.
**
** num_streams - This indicates the number of streams to make in the file.
** Due to some internal limitations, it should be no greater than
** VIDEO_FMT_MAX_MEDIA_STREAMS.
**
** major_brand - This is for specifying the major brand name of the
** specification on which the file format is based.  For example, in the 3GPP2
** file format, the brand name would be '3gp5'.
**
** version_major, version_minor - These specify the major and minor parts of
** the version number of the specification on which the file format is based.
** For example, in the 3GPP2 file format, the version number would be 0.0
** (major = 0, minor = 0).
**
** num_compat_brands, compat_brands - This is for specifying an array of
** compatible brand identification strings.  For example, in the 3GPP2 file
** format, brand identification strings are 4 characters each, and there may
** be any number of them, stored in a special 'ftyp' atom at the front of the
** file.
**
** movie_size_warning_near_threshold - This is the number of seconds, before
** the video format services estimates the space or duration limit will be
** reached, when a VIDEO_FMT_LIMIT_NEAR warning will be given.
**
** movie_size_warning_imminent_threshold - This is the number of seconds,
** before the video format services estimates the space or duration limit will
** be reached, when a VIDEO_FMT_LIMIT_IMMINENT warning will be given.
**
** fragment_size - This is normally set to 0, to indicate no file
** fragmentation.  When set to a non-zero value, it indicates a desired
** fragment size, in the movie timescale, and the writer generates a
** fragmented KDDI .3g2 file based on the specification "Movie File Format
** Specification for cdma2000 1x/1xEV-DO Version 1.1.0" by KDDI.
**
** force_stsz_table - This is to support the SKM file format, which does not
** allow an abbreviated 'stts' atom for streams with fixed-size samples.  If
** set to TRUE, a table of sample sizes is always generated in the 'stsz'
** atom, even for streams with fixed-size samples.
**
** include_drm, drm_distribution_rights - Setting "include_drm" will cause
** the writer to generate a 'drm ' atom containing a 'dcmd' atom with the
** given "drm_distribution_rights" set for the "distribution_rights" field.
** [See NTT DoCoMo Mobile MP4 Specification, v1.0.0]
**
** file_size_limit - if non-zero, indicates a maximum size for the file.
** The recording will automatically stop, with callback status of
** VIDEO_FMT_LIMIT_REACHED, as close as possible to this size without going
** over.
**
** file_duration_limit - if non-zero, indicates a maximum duration for the
** file, in the movie timescale.  The recording will automatically stop, with
** callback status of VIDEO_FMT_LIMIT_REACHED, as close as possible to this
** duration without going over.
**
** enable_fixupdata - If true truncates the video clip based on the minimum
** duration of primary elementary streams (audio/video).
**
** output_unit_size - if non-zero, indicates the maximum data size in bytes
** that videoFMT outputs per VIDEO_FMT_PUT_DATA callback and updates stream
** buffer pointers. This parameter affects the EFS write throughput.
*/
typedef struct video_fmt_create_params_type_t {
  uint32                               num_streams;
  video_fmt_stream_create_params_type  *streams;
  uint32                               movie_timescale; /* time units per  */
                                                        /* second for      */
                                                        /* whole movie     */
  const char                           *major_brand;    /* major brand id  */
  uint16                               version_major;
  uint16                               version_minor;
  uint32                               num_compat_brands;
  const char                           **compat_brands;  /* brand ids      */
  uint32                               movie_size_warning_near_threshold;
  uint32                               movie_size_warning_imminent_threshold;
  uint32                               fragment_size;
  boolean                              force_stsz_table;
  boolean                              include_drm;
  uint16                               drm_distribution_rights;
  uint32                               file_size_limit;
  boolean                              include_user_data;
  video_fmt_user_data_type             user_data;
  uint64                               file_duration_limit;
  boolean                              enable_fixupdata;
  uint32                               output_unit_size;
  uint32                               stream_bitrate;
  uint64                               frag_duration_in_msec;
} video_fmt_create_params_type;

#endif /* VIDEOFMT_COMMON_H */

