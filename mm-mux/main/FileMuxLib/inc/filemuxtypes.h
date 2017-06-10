#ifndef __FILEMUX_TYPES_H__
#define __FILEMUX_TYPES_H__

/* =======================================================================
                              filemuxtypes.h
========================================================================== */

/**
 *
 * Copyright (c) 2011 - 2014 Qualcomm Technologies, Inc., All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 * =======================================================================
                             Edit History

$Header: //depot/asic/msmshared/users/sateesh/multimedia2/Video/wfd-source/mm-mux/main/FileMuxLib/inc/filemuxtypes.h#2 $


========================================================================== */

/* =======================================================================
**               Includes and Public Data Declarations
** ======================================================================= */

/* ==========================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */

#include "AEEStdDef.h"
#include "AEEstd.h"

namespace video
{
  class iStreamPort;
}



typedef void * VIDEO_HANDLE;

/* =======================================================================

                        DATA DECLARATIONS

========================================================================== */

/* -----------------------------------------------------------------------
** Constant / Define Declarations
** ----------------------------------------------------------------------- */

#define MUX_MAX_MEDIA_STREAMS 6

#define FS_FILENAME_MAX_LENGTH_P 256

#ifndef MAX
   #define  MAX( x, y ) ( ((x) > (y)) ? (x) : (y) )
#endif

#ifndef MIN
   #define  MIN( x, y ) ( ((x) < (y)) ? (x) : (y) )
#endif

//! Macros to check the status of the MUX
#define MUX_SUCCEEDED(x) (x == muxbase::MUX_SUCCESS)
#define MUX_FAILED(x) (x != muxbase::MUX_SUCCESS)

/* This feature is process the audio/video samples in the caller context
   also to write 64K chunks into the file in another thread context
*/

//#define FILEMUX_WRITE_ASYNC
//#define FILEMUX_WIN32
/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */
/* This enumerated type is used to select a file format. */
typedef enum {
  MUX_FMT_MP4,          /* ISO/IEC 14496-1          */
  MUX_FMT_QCP,         /*....EVRC audio............*/
  MUX_FMT_AAC,         /*....AAC(ADIF ADTS) audio............*/
  MUX_FMT_AMR,           /*.....AMR format .............*/
  MUX_FMT_WAV,        /*WAV format..............*/
  MUX_FMT_EVRCB,      /*.....EVRCB format....*/
  MUX_FMT_EVRCWB,     /*.....EVRCWB format....*/
  MUX_FMT_MP2,          /* MP2 systems spec format */
  MUX_FMT_SECURE_MP2,          /* MP2 systems spec format */
  MUX_FMT_INVALID       /* invalid file format      */
} MUX_fmt_type;


/* This enumerated type is used to select qcp sub.format
based on the differnet rates */
typedef enum {
  MUX_QCP_RATE_INVALID,
  MUX_QCP_RATE_EIGHTH,
  MUX_QCP_RATE_QUARTER,
  MUX_QCP_RATE_HALF,
  MUX_QCP_RATE_FULL,
  MUX_QCP_RATE_MAX       /*....AAC(ADIF ADTS) audio............*/
} MUX_qcp_rate;


/*!

*@brief Enumeration to differentiate among metadata such as Author,
         Title etc.
 @detail Each Meta Data Type has some more information or properties
         associated with it. Read the documentation for each property
         carefully to pass the pMetaProperties in the correct format
*/

typedef enum

{

    /*
    @brief This will be a string mentioning the title of the content
    @detail
       Fill the properties structure as mentioned below.
       pMetaProperties[0] = Size of the string of ascii characters
                            including NULL termination
       pMetaProperties[1] = pointer to the string.
    */
    FILEMUX_MD_TITLE,

    /*
    @brief This will be a string mentioning the Author of the content
    @detail
       Fill the properties structure as mentioned below.
       pMetaProperties[0] = Size of the string of ascii characters
                            including NULL termination
       pMetaProperties[1] = pointer to the string.
    */
    FILEMUX_MD_AUTHOR,

    /*
    @brief This will be a string having some description of the content
    @detail
       Fill the properties structure as mentioned below.
       pMetaProperties[0] = Size of the string of ascii characters
       pMetaProperties[1] = pointer to the string.
    */
    FILEMUX_MD_DESCRIPTION,

    /*
    @brief This will be a string with the rating of the content
    @detail
       Fill the properties structure as mentioned below.
       pMetaProperties[0] = Size of the string of ascii characters
                            including NULL termination
       pMetaProperties[1] = pointer to the string.
    */
    FILEMUX_MD_RATING,

    /*
    @brief This will be a string with the copyright information
           of the content
    @detail
       Fill the properties structure as mentioned below.
       pMetaProperties[0] = Size of the string of ascii characters
                            including NULL termination
       pMetaProperties[1] = pointer to the string.
    */
    FILEMUX_MD_COPYRIGHT,

    /*
    @brief This will be a string with the version information
           of the content
    @detail
       Fill the properties structure as mentioned below.
       pMetaProperties[0] = Size of the string of ascii characters
                            including NULL termination
       pMetaProperties[1] = pointer to the string.
    */
    FILEMUX_MD_VERSION,

    /*
    @brief This will be a string with the date of creation
           of the content
    @detail
       Fill the properties structure as mentioned below.
       pMetaProperties[0] = Size of the string of ascii characters
                            including NULL termination
       pMetaProperties[1] = pointer to the string.
    */
    FILEMUX_MD_CREATION_DATE,

    /*
    @brief This will be the detail on the performance
    @detail
       Fill the properties structure as mentioned below.
       pMetaProperties[0] = Size of the string of ascii characters
                            including NULL termination
       pMetaProperties[1] = pointer to the string.
    */
    FILEMUX_MD_PERFORMER,

    /*
    @brief This will be the detail on the genre
    @detail
       Fill the properties structure as mentioned below.
       pMetaProperties[0] = Size of the string of ascii characters
                            including NULL termination
       pMetaProperties[1] = pointer to the string.
    */
    FILEMUX_MD_GENRE,

    /*
    @brief This will be the detail on content classification
    @detail
       Fill the properties structure as mentioned below.
       pMetaProperties[0] = Size of the string of ascii characters
                            including NULL termination
       pMetaProperties[1] = pointer to the string.
    */
    FILEMUX_MD_CLASSIFICATION,

    /*
    @brief This will be the detail on keywords that can be
           associated with content
    @detail
       Fill the properties structure as mentioned below.
       pMetaProperties[0] = Size of the string of ascii characters
                            including NULL termination
       pMetaProperties[1] = pointer to the string.
    */
    FILEMUX_MD_KEYWORD,

    /*
    @brief This will be the location information
    @detail
       Fill the properties structure as mentioned below.
       pMetaProperties[0] = Size of the string of ascii characters
       pMetaProperties[1] = Pointer to string on place of creation.
       pMetaProperties[2] = Size of the string of ascii characters
       pMetaProperties[3] = Pointer to the astronomical body name
       pMetaProperties[4] = Role
                            0: Invalid
                            1: Shooting Location
                            2: real location
                            3: fictional location

       pMetaProperties[5] = Logitude Reference
                            0: Invalid
                            1: East
                            2: West
       pMetaProperties[6] = Longitude Degrees
       pMetaProperties[7] = Longitude Minutes
       pMetaProperties[8] = Longitude Seconds

       pMetaProperties[9] = Lattitude Reference
                            0: Invalid Latitude
                            1: North
                            1: South

       pMetaProperties[10] = Lattitude Degrees
       pMetaProperties[11] = Lattitude Minutes
       pMetaProperties[12] = Lattitude Seconds
       pMetaProperties[13] = Altitude Reference
                             0: Invalid
                             1: Above Sea Level
                             2: Below Sea Level
       pMetaProperties[14] = Meters above/below sea level
       pMetaProperties[15] = Millimieters above/below meters
                             specified
       pMetaProperties[16] = nSize of Additional Description
                             Set to 0 for no description
       pMetaProperties[17] = Pointer to Additional Description
    */
    FILEMUX_MD_LOCATION,

    /*
    @brief This will mention the Artist
    @detail
       Fill the properties structure as mentioned below.
       pMetaProperties[0] = Size of the string of ascii characters
                            including NULL termination
       pMetaProperties[1] = pointer to the string.
    */
    FILEMUX_MD_ARTIST,

    /*
    @brief This is to add the Album information
    @detail
       Fill the properties structure as mentioned below.
       pMetaProperties[0] = Size of the string of ascii characters
                            including NULL termination
       pMetaProperties[1] = pointer to the string.
    */
    FILEMUX_MD_ALBUM,

    /*
    @brief Conveys the track num for the content
    @detail
       Fill the properties structure as mentioned below.
       pMetaProperties[0] = Track Num
    */
    FILEMUX_MD_TRACKNUM,

    /*
    @brief This is to add the general information
    @detail
       Fill the properties structure as mentioned below.
       pMetaProperties[0] = Size of the string of ascii characters
                            including NULL termination
       pMetaProperties[1] = pointer to the string.
    */
    FILEMUX_MD_INFO,

    /*
    @brief This is to add the owner information
    @detail
       Fill the properties structure as mentioned below.
       pMetaProperties[0] = Size of the string of ascii characters
                            including NULL termination
       pMetaProperties[1] = pointer to the string.
    */
    FILEMUX_MD_OWNER,

    /*
    @brief This is to add the some user data related to the content
    @detail
    User can have some useful information to be added other than
    that specified in standards. This allows to add some user information
    with a fourcc code specified by user.
       Fill the properties structure as mentioned below.
       pMetaproperties[0] = a fourcc code for users data in ascii
       pMetaProperties[1] = Size of the proprietary data
       pMetaProperties[2] = pointer to the data.
    */
    FILEMUX_MD_TAGGED_USER_DATA ,

    /*
    @brief This is to add the some proprietary user data which
    is tagged by the user with a GUID.
    @detail
       The data is identified with a GUID related to that particular
       information. If GUID is not present or set to NULL it will be
       added as a generic user information in the formats which supports
       general information.
       Not all formats support this.
       Note: This config has to be made before first frame data
             or frame header is sent to filemux. This call will
             be rejected if made after filemux receives any
             media data.

       Fill the properties structure as mentioned below.
       pMetaProperties[0] = Size of GUID
       pMetaProperties[1] = GUID
       pMetaProperties[2] = Size of the proprietary data
       pMetaProperties[3] = pointer to the data.
    */
    FILEMUX_MD_USER_PROPRIETARY_DATA,

    FILEMUX_MD_META_MAX = 0xffffffff

} FileMuxMetaDataType;


/*
  @brief An enum definition for text coding type.
  @detail
      User can specify the text coding to be performed while encoding
      the string into the file. By default UTF8 is taken.
*/

typedef enum
{
    FILEMUX_UTF8,
    FILEMUX_UTF16,
    FILEMUX_ENCODING_MAX = 0xffffffff
}FileMuxTextCodingType;

/*
  @brief This stucture is used to pass meta data
*/

typedef struct metaData
{
    FileMuxMetaDataType   eType;          //!Type of meta data
    FileMuxTextCodingType eCoding;        //!Text coding to be used
    unsigned long         lang;           //!ISO 639-2T code for language
    unsigned long         nDataSize;      //!Size of data related to this tag
    unsigned long        *pMetaProperties;//!Data related to the Tag
}FileMuxMetaData;

/*
  @brief This stucture is used to get statistics of file mux
*/

typedef struct muxStatistics
{
    uint64 recordedTime;
    uint64 timeCanRecord;
    uint64 spaceConsumed;
    uint64 spaceLeft;
}FileMuxStatistics;

typedef struct
{
  uint32 sample;            /* sample number (first sample is zero)                    */
  uint32 size;              /* size of sample in bytes                                 */
  uint32 offset;            /* offset of sample in stream, in bytes                    */
  uint32 time;              /* composition time of sample, in the media timescale      */
  uint32 decode_time;       /* decode time of sample, in the media timescale  */
  uint32 delta;             /* difference between composition time of this sample and  */
                            /* the next sample, in the media timescale                 */
  uint32 sync;              /* Indication if sample is a random access point           */
                            /* (non-zero) or not (zero)                                */
  uint32 extra_data_size;   /* size of any inband extra data included with buffer      */
  uint32 extra_data_offset; /* starting offset of the extra data                       */
  uint8* extra_data_ptr;    /* Extra data start location                               */
  uint32 fmt_pvtdata_size;  /* size of any inband pvt data included with buffer        */
  uint32 fmt_pvtdata_offset;/* starting offset of the pvt data                         */
  uint8* fmt_pvtdata_ptr;   /* Extra data start location                               */
  uint32 sample_desc_index; /* Sample Description retrieved from the stsc atom         */
} MUX_sample_info_type;

/* This enumerated type lists the different text justification settings. */
typedef enum {
    MUX_JUSTIFY_LEFT_OR_TOP,
    MUX_JUSTIFY_CENTER,
    MUX_JUSTIFY_RIGHT_OR_BOTTOM
} MUX_justify_type;


typedef struct mux_font_type_t {
  int16                         font_id;
  const char                    *font_name;
} MUX_font_type;

/* This enumerated type lists the different types of scrolling that are
** defined for timed text.
*/
typedef enum {
    MUX_SCROLL_VERT_UP,
    MUX_SCROLL_HORZ_LEFT,
    MUX_SCROLL_VERT_DOWN,
    MUX_SCROLL_HORZ_RIGHT
} MUX_scroll_type;

/* This structure provides information necessary to construct a sample entry
** atom for a timed text track.
*/
typedef struct MUX_text_type_t {
  boolean                    scroll_in;
  boolean                    scroll_out;
  MUX_scroll_type      scroll_type;
  boolean                    continuous_karaoke;
  boolean                    write_vertically;
  MUX_justify_type     horz_justify;
  MUX_justify_type     vert_justify;
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
  const MUX_font_type  *fonts;
} MUX_text_type;

/* This enumerated type lists the different types of video streams. */
typedef enum {
  MUX_STREAM_VIDEO_NONE,        /* flag indicating no video       */
  MUX_STREAM_VIDEO_MPEG4,       /* ISO/IEC 14496-2                */
  MUX_STREAM_VIDEO_H263,        /* H.263                          */
  MUX_STREAM_VIDEO_H264,        /* H.264                          */
  MUX_STREAM_VIDEO_JPEG,        /* SKT-MOD JPEG                   */
  MUX_STREAM_VIDEO_BMP,         /* SKT-MOD BMP                    */
  MUX_STREAM_VIDEO_STILL_IMAGE, /* PV Specific Still Image        */
  MUX_STREAM_VIDEO_UNK,         /* unknown type of video          */
  MUX_STREAM_VIDEO_INVALID      /* invalid video stream type      */
} MUX_stream_video_type;

typedef struct MUX_AVC_TimingHRD_params
{
    uint32  nAVCTimeScale;
    uint32  nNumUnitsInTick;
    bool    bHrdManagementValid;
    bool    bPictureAndTimingInfo;
    bool    bFixedFrameRate;
    bool    bTemporalPocFlag;
    bool    bPictureToDisplayConversionFlag;
}MUX_AVC_TimingHRD_params_type;

typedef struct {
  uint32                      data_format;      /* format of encoded data */
  uint32                      scheme_type;      /* protection scheme type */
  uint32                      scheme_version;   /* protection scheme version */
  uint8                       EncryptionMethod;
  uint8	                      PaddingScheme;		// Padding type
  uint64	              PlaintextLength;	// Plaintext content length in bytes
  uint16	              ContentIDLength;	// Length of ContentID field in bytes
  uint16	              RightsIssuerURLLength;	// Rights Issuer URL field length in bytes
  uint16	              TextualHeadersLength;	// Length of the TextualHeaders array in bytes
  uint8                       SelectiveEncryption;
  uint8                       KeyIndicatorLength;
  uint8                       IVLength;
} MUX_pdcf_type;


typedef struct {
  MUX_stream_video_type       format;
  uint16                      width;            /* frame width in pixels  */
  uint16                      height;           /* frame height in pixels */
  uint16                      frame_rate;       /* frames per second      */
  uint16                      iframe_interval;  /* I-frame interval       */
  uint8                       profile;          /* codec feature set      */
  uint8                       level;            /* subdivision of level   */
  uint8                       profile_comp;     /* Profile compatibility */
  MUX_pdcf_type         pdcf_info;
} MUX_stream_video_subtype;


/* This enumerated type lists the different types of audio streams. */
typedef enum {
  MUX_STREAM_AUDIO_NONE,           /* flag indicating no audio       */
  MUX_STREAM_AUDIO_QCELP13K_FULL,  /* PureVoice QCELP-13K fixed full */
  MUX_STREAM_AUDIO_QCELP13K_HALF,  /* PureVoice QCELP-13K fixed half */
  MUX_STREAM_AUDIO_EVRC,           /* Enhanced Variable Rate Codec   */
  MUX_STREAM_AUDIO_EVRC_PV,        /* EVRC, PacketVideo variant      */
  MUX_STREAM_AUDIO_AMR,            /* GSM Adaptive Multi Rate codec  */
  MUX_STREAM_AUDIO_MPEG1_L3,       /* MPEG-1, Layer 3 (MP3) codec    */
  MUX_STREAM_AUDIO_MPEG2_L3,       /* MPEG-2, Layer 3 (MP3) codec    */
  MUX_STREAM_AUDIO_MPEG4_AAC,      /* MPEG-4 Advanced Audio Codec    */
  MUX_STREAM_AUDIO_PUREVOICE,      /* 3GPP2 QCELP                    */
  MUX_STREAM_AUDIO_AMR_WB,         /* AMR Wideband codec (TS 26.171) */
  MUX_STREAM_AUDIO_AMR_WB_PLUS,    /* Extended AMR WB (TS 26.290)    */
  MUX_STREAM_AUDIO_EVRC_B,         /* EVRC-B type codec */
  MUX_STREAM_AUDIO_EVRC_WB,        /* EVRC-WB type codec */
  MUX_STREAM_AUDIO_PCM,            /* Raw audio formats              */
  MUX_STREAM_AUDIO_AC3,            /* Dolby digital codec            */
  MUX_STREAM_AUDIO_UNK,            /* Unknown type of audio          */
  MUX_STREAM_AUDIO_INVALID         /* invalid audio stream type      */
} MUX_stream_audio_type;

typedef struct {
  uint8    audio_object_type;
  uint8    channel_configuration;
  uint8    sbr_present_flag;
  uint8    ep_config;
  boolean  aac_section_data_resilience_flag;
  boolean  aac_scalefactor_data_resilience_flag;
  boolean  aac_spectral_data_resilience_flag;
} MUX_aac_params_type;

typedef struct {
  uint16   mode_set;
  uint8    mode_change_period;
  uint8    frames_per_sample;
} MUX_audio_params_type;


typedef struct {
  MUX_stream_audio_type    format;
  MUX_aac_params_type      aac_params;
  MUX_audio_params_type    audio_params;
  uint32                   sampling_frequency;
  uint8                    num_channels;
  MUX_pdcf_type            pdcf_info;
} MUX_stream_audio_subtype;

typedef struct {
  int cur_atom_index; /* index of current tx3g atom*/
} MUX_stream_text_subtype;


typedef enum {
  MUX_STREAM_DATA_BIFS,  /* Binary Format for Scenes (MPEG-4) */
  MUX_STREAM_DATA_OD,    /* Object Descriptors (MPEG-4)       */
  MUX_STREAM_DATA_SKT_TEXT,   /* SKT text                          */
  MUX_STREAM_DATA_UNK    /* Unknown type of data              */
} MUX_stream_data_type;

/* This structure contains type-specific information about a data stream. */
typedef struct {
  MUX_stream_data_type format;
} MUX_stream_data_subtype;


typedef union {
  MUX_stream_video_subtype video; /* MUX_STREAM_VIDEO */
  MUX_stream_audio_subtype audio; /* MUX_STREAM_AUDIO */
  MUX_stream_text_subtype  text;  /* MUX_STREAM_TEXT  */
  MUX_stream_data_subtype  data;  /* MUX_STREAM_DATA  */
} MUX_stream_subinfo_type;


/* This enumerated type lists the different types of media streams. */
typedef enum {
  MUX_STREAM_UNK,    /* unknown type of stream      */
  MUX_STREAM_VIDEO,  /* MUX_stream_video_type */
  MUX_STREAM_AUDIO,  /* MUX_stream_audio_type */
  MUX_STREAM_TEXT,
  MUX_STREAM_DATA,   /* MUX_stream_data_type  */
  MUX_STREAM_INVALID /* invalid type of stream      */
} MUX_stream_type;


typedef enum {
  MUX_BRAND_MP4,      /* strictly ISO/IEC 14496-1             */
  MUX_BRAND_EVRC,     /*    EVRC-QCP..........*/
  MUX_BRAND_ADTS,     /*    AAC-ADTS..........*/
  MUX_BRAND_ADIF,     /*....AAC-ADIF...........*/
  MUX_BRAND_QCELP13K_FIXED_FULL_RATE,
  MUX_BRAND_QCELP13K_FIXED_HALF_RATE,
  MUX_BRAND_QCELP13K_VAR_FULL_RATE,
  MUX_BRAND_QCELP13K_VAR_HALF_RATE,
  MUX_BRAND_AMRNB,
  MUX_BRAND_PCMLINEAR,
  MUX_BRAND_PCMALaw,
  MUX_BRAND_PCMMULaw,
  MUX_BRAND_G723,
  MUX_BRAND_G729,
  MUX_BRAND_AMC,      /* KDDI AMC (movie mail) format         */
  MUX_BRAND_3GP,      /* 3GPP (3GPP TS 26.234 v5.6.0)         */
  MUX_BRAND_3G2,      /* 3GPP2 (3GPP2 C.P0050-0)              */
  MUX_BRAND_K3G,      /* KWIF (KWISF.K-02-001)                */
  MUX_BRAND_FRAG_3G2, /* Fragmented 3G2 (KDDI Movie File      */
                            /*   Format Specification for cdma2000  */
                            /*   1x/1xEV-DO v1.1.0)                 */
  MUX_BRAND_SKM,      /* SK Telecom VOD Serv Spec 2.3 draft2  */
  MUX_BRAND_MP2TS,    /* MPEG2 TS file brand */
  MUX_BRAND_INVALID   /* invalid brand                        */
} MUX_brand_type;

typedef struct {
  uint32    ref_atom;
  uint32    track_count;
  uint32    track_id [MUX_MAX_MEDIA_STREAMS];
} MUX_ref_track_type;

typedef enum {
  MUX_ENCRYPT_TYPE_HDCP,
  MUX_ENCRYPT_TYPE_INVALID
} MUX_encrypt_type;

typedef struct {
    bool                streamEncrypted;
    MUX_encrypt_type    type;
    uint32              nEncryptVersion;
} MUX_encrypt_param_type;

typedef struct MUX_stream_create_params_type_t {
  MUX_stream_type                type;      /* video, audio, etc.          */
  MUX_stream_subinfo_type        subinfo;   /* type-specific media info    */
  uint8                          priority;  /* relative stream priority    */
  const char                     *handler;  /* name for handler reference  */
  uint32                         media_timescale; /* media-specific time   */
                                                  /* units per second      */
  MUX_ref_track_type              ref_track; /* tracks referenced by stream */
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
} MUX_stream_create_params_type;

typedef union MUX_utf_string_type_t {
  const char *utf8;
  const short *utf16;
} MUX_utf_string_type;

typedef struct MUX_language_coded_string_type_t {
  uint16                     language;
  boolean                    isUTF16;
  MUX_utf_string_type  string;
  uint32                     length;
} MUX_language_coded_string_type;

typedef struct MUX_title_type_t {
  boolean titl_valid;
  MUX_language_coded_string_type titl;
}MUX_title_type;

typedef struct MUX_author_type_t {
  boolean auth_valid;
  MUX_language_coded_string_type auth;
}MUX_author_type;

typedef struct MUX_copyright_type_t{
  boolean cprt_valid;
  MUX_language_coded_string_type cprt;
}MUX_copyright_type;

typedef struct MUX_performer_type_t{
  boolean perf_valid;
  MUX_language_coded_string_type perf;
}MUX_performer_type;

typedef struct MUX_genre_type_t{
  boolean gnre_valid;
  MUX_language_coded_string_type gnre;
}MUX_genre_type;

typedef struct MUX_description_type_t{
  boolean desc_valid;
  MUX_language_coded_string_type desc;
}MUX_description_type;

typedef struct MUX_rating_type_t {
  boolean rtng_valid;
  uint32  entity;
  uint32  criteria;
  MUX_language_coded_string_type rtng;
}MUX_rating_type;

typedef struct MUX_album_type_t{
  boolean albm_valid;
  boolean trknum_valid;
  MUX_language_coded_string_type albm;
  uint8 trackNo;
}MUX_album_type;

typedef struct MUX_classification_type_t {
  boolean clsf_valid;
  uint32 entity;
  uint16 table;
  MUX_language_coded_string_type clsf;
}MUX_classification_type;

typedef struct MUX_keywords_type_t {
  boolean kywd_valid;
  uint8 numKywds;
  MUX_language_coded_string_type *kywd;
}MUX_keywords_type;

typedef struct MUX_recorded_year_type_t {
  boolean yrrc_valid;
  uint16  year;
}MUX_recorded_year_type;


typedef struct MUX_user_proprietary_data_type_t {
  boolean                     data_valid;
  uint32                      nSize;
  uint8*                      pData;
 }MUX_user_proprietary_data_type;

typedef struct MUX_location_type_t {
  boolean                               loci_valid; /* To verify that the location info is valid */
  MUX_language_coded_string_type        name;
  uint8                                 role;
  uint32                                longitude;
  uint32                                latitude;
  uint32                                altitude;
  MUX_language_coded_string_type  astr_body;
  MUX_language_coded_string_type  add_notes;
} MUX_location_type;

typedef struct MUX_user_data_type_t {
  boolean atom_is_valid; /* Check for the validity of atom */
  uint32 midiSize; /*if zero, format writer will exclude midi section*/
  uint8 * midiData;
  MUX_title_type                  title;
  MUX_author_type                 author;
  MUX_description_type            description;
  MUX_copyright_type              copyright;
  MUX_performer_type              performer;
  MUX_genre_type                  genre;
  MUX_rating_type                 rating;
  MUX_classification_type         clsf;
  MUX_keywords_type               keywords;
  MUX_album_type                  album;
  MUX_recorded_year_type          yrrc;
  MUX_location_type               loci;
  MUX_user_proprietary_data_type  data;
} MUX_user_data_type;

typedef struct MUX_create_params_type_t {
  uint32                               num_streams;
  MUX_stream_create_params_type        *streams;
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
  MUX_user_data_type                   user_data;
  uint32                               file_duration_limit;
  boolean                              enable_fixupdata;
  uint32                               output_unit_size;
  uint32                               stream_bitrate;
  uint32                               frag_duration_in_msec;
  uint32                               sampling_rate;
  uint8                                sampling_rate_index;
  uint8                                num_channels;
  uint16                               block_align;
  uint16                               bits_per_sample; /// Bits per sample
  MUX_encrypt_param_type               encrypt_param;
  uint16                               pcr_pid;
  uint16                               video_pid;
  uint16                               audio_pid;
  uint16                               userdata_pid;
  uint8                                debug_flags;
} MUX_create_params_type;


typedef enum {
  MUX_METHOD_MEM,     /* recording movie file to memory buffer   */
  MUX_METHOD_EFS,     /* recording movie file to EFS             */
  MUX_METHOD_CLIENT,  /* recording movie file through client API */
  MUX_METHOD_EFS_INDIRECT, /*recording movie file to EFS
                              and client provides storage for filename     */
  MUX_METHOD_PORT,  /* recording movie file through client API */
  MUX_METHOD_TRANSCODE, /* recording movie to Transcode class buffers */
  MUX_METHOD_INVALID  /* designates an invalid type                      */
} MUX_method_type;

/* This type is associated with the VIDEO_METHOD_MEM enumerated type. This
** type will be used to reference a file in the RAM region of memory.  This
** type specifies a pointer to the start of the data buffer.  The buffer must
** be in RAM.
*/
typedef struct {
  MUX_method_type  method;  /* This must be VIDEO_METHOD_MEM   */
  uint8              *buf;    /* Buffer in memory                */
  uint32             len;     /* Size of buffer, in bytes        */
} MUX_handle_type_mem;


/* This type is used to send information about the stream that is flushed
*/
typedef struct
{
  uint32 stream_id;        /**< Stream ID client requested to flush */
}MUX_flush_info_type;


/* Callback function to write the data if the output handle is of client type */
typedef uint32 (*mux_method_client_cb_func_ptr_type) (
  uint8              *buf,         /* Buffer in memory                  */
  uint32             len,          /* Size of buffer, in bytes          */
  uint32             offset,       /* Destination byte offset in file   */
  void               *client_data  /* Client data from calling function */
);

/* Callback function to flush data in another thread context */
typedef int (*Mux_flush_stream_to_file_func_type)(void);

/* Callback function to write the data if the output handle is of client type */
typedef void (*mux_write_file_cb_func_ptr_type) (
	void        *client_data  //! Client data from calling function
);

/* Client output handle type */
typedef struct {
  MUX_method_type    method;     /* This must be VIDEO_METHOD_CLIENT      */
  mux_method_client_cb_func_ptr_type  data_push_fn; /* Data push function */
  void                 *client_data;  /* Client data from calling function  */
} MUX_handle_type_client;

/* Port output handle type */
typedef struct {
  MUX_method_type    method;     /* This must be VIDEO_METHOD_CLIENT      */
  video::iStreamPort* pOputStream;  /* output port to write the data. */
  void                 *client_data;  /* Client data from calling function  */
} MUX_handle_type_port;


/* This type is associated with the VIDEO_METHOD_EFS enumerated type.  This
** type will be used to reference a file in the embedded file system (EFS). It
** specifies the complete name of a file in the EFS.
*/
typedef struct {
  MUX_method_type  method;  /* This must be VIDEO_METHOD_EFS             */
  char  filename[FS_FILENAME_MAX_LENGTH_P]; /* Name of file   */
} MUX_handle_type_efs;

/* This type is associated with the VIDEO_METHOD_EFS_INDIRECT enumerated type.
** This type will be used to reference a file in the embedded file system (EFS). It
** has a pointer to the name of file
*/
typedef struct {
  MUX_method_type  method;  /* This must be VIDEO_METHOD_EFS_INDIRECT  */
  wchar_t         *filename; /* Name of file   */
} MUX_handle_type_filename;

typedef struct {
  MUX_method_type  method;        /* This must be VIDEO_METHOD_EFS_INDIRECT  */
  void*  pTranscode;        /* pointer to an object of Transcode class  */
} MUX_handle_type_transcode;

/* output handle type */
typedef union {
  MUX_method_type         method;  /* method type id  */
  MUX_handle_type_mem     mem;     /* RAM handle           */
  MUX_handle_type_efs     efs;     /* EFS handle           */
  MUX_handle_type_client  client;  /* client handle        */
  MUX_handle_type_filename fname;    /* EFS handle for qvp   */
  MUX_handle_type_port  OputPort;
  MUX_handle_type_transcode  transcode; /* Transcode class handle */
} MUX_handle_type;

//! Generic  enum to track the status returned by interfaces for MUX Module
  typedef enum
  {
    MUX_SUCCESS             = 0,		//! Default  Success
    MUX_FAIL                = 1,		//! Default Error/Fail
    MUX_NOTAVAILABLE        = 2,		//! MUX is not intialized yet
    MUX_INVALID             = 3,		//! Invalid parameter passed to the function
    MUX_QUEUE_FULL          = 4,
    MUX_DONE                = 5,		//! When Mux has done some async op synchronously
    MUX_SPACE_LIMIT_REACHED = 6,	//! Ran out of the space
    MUX_WRITE_FAILED        = 7,		//! write failed
    MUX_OUTDATED            = 8,   //! outdated frame not processed
    MUX_SECURITY_FAULT      = 9         // security failure
  } MUX_STATUS;


typedef  int MuxCallBackStatus;
  //! Generic  status to track the status returned by interfaces for MUX Module
#define    MUX_CREATE_COMPLETE            0   //! MUX create complete
#define    MUX_CREATE_FAIL                1   //! MUX create failed
#define    PROCESS_SAMPLE_COMPLETE        2   //! Process sample complete
#define    PROCESS_SAMPLE_FAIL            3   //! Process sample failed
#define    CLOSE_MUX_COMPLETE             4   //! MUX close complete
#define    CLOSE_MUX_FAIL                 5   //! MUX close failed
#define    FLUSH_COMPLETED                6   //! Flush completed
#define    FLUSH_FAILED                   7   //! Flush failed
#define    PAUSE_COMPLETED                8   //! Mux pasued
#define    PAUSE_FAILED                   9   //! failed to pause
#define    PROCESS_SAMPLE_FLUSH           10  //! Sample is flushed without processing
#define    PROCESS_HEADER_COMPLETE        11  //! Header accepted successfully
#define    PROCESS_HEADER_FAIL            12  //! Header processing fail.
#define    SPACE_LIMIT_REACHED            13  //! Space limit reached
#define    WRITE_FAILED                   14  //! write fail.
#define    MUX_STATISTICS                 15  //! recording space and time info
#define    PROCESS_SAMPLE_OUTDATED        16  //! Sample is outdated

 typedef struct Event
  {
    uint32 EventNumber;
    uint32 stream_number;
    uint32 num_samples;
    const MUX_sample_info_type *sample_info;
    const uint8  *sample_data;
  }MUX_PEventData;

typedef struct header
{
	uint32 audio_str;
	uint32 audio_header_size;
	const uint8 *audio_header;
}MUX_Header;

typedef struct footer
{
	uint32 stream_num;
	uint32 footer_length;
	const uint8 *buf_ptr;
}MUX_Footer;

typedef struct header_text
{
	uint32 stream_num;
    const MUX_text_type_t *entry;
}MUX_Header_Text;

typedef struct write_text
{
	uint32 stream_num;
	void * handle;
	uint32 delta;
}MUX_write_text;

typedef struct uuid_type
{
	const uint8 *uuid;
	const void *data;
	uint32 size;
}MUX_uuid;

typedef union {
  MUX_Header       header; /* MUX_STREAM_VIDEO */
  MUX_Footer       footer; /* MUX_STREAM_AUDIO */
  MUX_Header_Text  header_text;  /* MUX_STREAM_TEXT  */
  MUX_write_text   write_text;  /* MUX_STREAM_DATA  */
  MUX_uuid         uuid;
  void             *handle;
  uint32            duration;
} user_Data;

typedef enum {
  MUX_HEADER,          /* ISO/IEC 14496-1          */
  MUX_FOOTER,
  MUX_HEADER_TEXT,
  MUX_WRITE_TEXT,
  MUX_UUID,
  MUX_FREE_TEXT,
  MUX_MEHD_DURATION
}MUX_User_Data_Type;

typedef struct user_Data_type
{
	MUX_User_Data_Type type;
    user_Data  user_data;
}MUX_user_Data;

typedef struct MUX_OMX_Data_type
{
  void *pSample;
  MUX_sample_info_type *pSampleInfo;
  void *clientData;
}MUX_OMX_Data;

typedef struct MUX_msg_list_type_t {
    char                              *msg;
    uint16                            msg_size;
    struct MUX_msg_list_type_t        *next;
}MUX_msg_list_type;

typedef struct {
    boolean                       atom_is_valid;
    /*Title Related attributes*/
    boolean                       istitleUTF16;
    uint16                        titlelangauge;
    MUX_utf_string_type           title;
    uint8                         title_len;
    /*author Related attributes*/
    boolean                       isauthUTF16;
    uint16                        authlangauge;
    MUX_utf_string_type           author;
    uint8                         auth_len;
    /*description related attributes*/
    boolean                       isdescpUTF16;
    uint16                        descplangauge;
    MUX_utf_string_type           description;
    uint8                         descp_len;
}MUX_user_data_atom_type;

/*Callback function prototype that needs to be registered by client*/
typedef void (*MUX_user_atoms_client_cb)(
                MUX_user_data_atom_type *puseratominfo,
                void                    *client_data
);


/*! @brief MUX Notification Callback function type

    @detail  It takes status, pClientData, sampleInfo, ImediaSample as input parameters.
             status is int
             pClientData is the client data that we received at the time File Mux creation.
             sampleInfo pointer is the one to free the sample info because file Mux is done with this buffer
             ImediaSample pointer is the one to free the sample buffer because file Mux is done with this buffer

    @param[in] Params will provide the necessary information to configure the writer library.
               status - MuxCallBackStatus
               pClientData - client data that we received at the time File Mux creation.
               sampleInfo pointer is the one to free the sample info because file Mux is done with this buffer
               ImediaSample pointer is the one to free the sample buffer because file Mux is done with this buffer
*/
typedef void (*MUXCallbackFuncType)(int status,void *pClientData, void *sampleInfo, void *ImediaSample);

#endif //__FILEMUX_TYPES_H__
