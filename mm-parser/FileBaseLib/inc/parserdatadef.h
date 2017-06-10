#ifndef _FILE_SOURCE_HELPER_INTERNAL_H_
#define _FILE_SOURCE_HELPER_INTERNAL_H_
/*=======================================================================
                             parserdatadef.h
DESCRIPTION
  Definiton of internal data types used by demuxer module.

  Copyright (c) 2010-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
*======================================================================== */
/* =======================================================================
                             Edit History
$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/FileBaseLib/main/latest/inc/parserdatadef.h#58 $$
$DateTime: 2014/05/13 22:50:58 $$
$Change: 5885284 $$
========================================================================== */
/*********Include all required header files**************************/
#include <wchar.h>
#include "AEEStdDef.h"

#if defined(__QNX__) || defined(__QNXINTO__) || defined (_ANDROID_)
#include "AEEstd.h"
#else
#include "AEEStd.h"
#endif

/********************************************************************/

//!  Maximum number of valid (audio,video and text) tracks in a clip
#define FILE_SOURCE_MAX_NUM_TRACKS          12
//! Header size(s) for various Audio Formats
#define EVRC_SAMPLE_HEADER_SIZE             1
#define QCELP_SAMPLE_HEADER_SIZE            1
#define AMR_SAMPLE_HEADER_SIZE              1
#define AMRWBP_SAMPLE_HEADER_SIZE           8
//! conversion factor to get time in milli seconds
#define MILLISEC_TIMESCALE_UNIT             1000
#define MICROSEC_TIMESCALE_UNIT             (1000*1000)
#define NANOSEC_TIMESCALE_UNIT              ((uint64)(1000 * 1000 * 1000))
#define TIMESCALE_BASE                      1000
#define FOURCC_SIGNATURE_BYTES              4

#define UNUSED_PARAM(x) ((void)x);

#define DEFAULT_ATOM_SIZE 8
#define DEFAULT_ATOM_VERSION 0
#define DEFAULT_ATOM_VERSION_SIZE 1
#define DEFAULT_ATOM_FLAG_SIZE 3
#define DEFAULT_MAX_URI_SIZE 256
#define ATOM_VERSION_ONE 1

/*!
 * ID3 tag specific define
 * ID3v2.3.0 & ID3v2.3.4
 */
#define ID3V2_HEADER_SIZE         10
#define AAC_FORMAT_BUFF_SIZE      4096

/*!
 * Possible values for frame text encoding
 */
//// [ISO-8859-1]. Terminated with $00.
#define ID3V2_FRAME_TEXT_ENCODING_ISO_8859_1 0x00
// [UTF-16] encoded Unicode [UNICODE] with BOM. All strings in the same frame
// SHALL have the same byteorder. Terminated with $00 00.
#define ID3V2_FRAME_TEXT_ENCODING_UTF_16     0x01
// [UTF-16] encoded Unicode [UNICODE] without BOM. Terminated with $00 00.
#define ID3V2_FRAME_TEXT_ENCODING_UTF_16BE   0x02
// [UTF-8] encoded Unicode [UNICODE]. Terminated with $00.
#define ID3V2_FRAME_TEXT_ENCODING_UTF_8      0x03

/* Frame size is not available for ADIF clips.
   This special value will be helpful to differentiate
   frame Length calculated from LOAS and ADTS clips.
   For ADIF clips, Parser will not validate two frames. */
#define AAC_ADIF_FRAME_SIZE       (0xFFFFFFFF)

//This value is equivalent to one ADIF frame header size.
//Parser uses same value as min file size for both MP3 and AAC file formats.
#define MIN_FILE_SIZE         64

/*! Parser will use this value in streaming scenarios where content length is
  ! not available. This max value will help parsers not to calculate duration
  ! value.
*/
#define MAX_FILE_SIZE ((uint64 )-1)

/*! Upper boundary to the file Size. This upper boundary will be helpful in
  ! Audio file formats where duration information is not available. Parser
  ! will calculate average duration, if input file/buffer size is greater than
  ! this predefined value.
*/
#define FILE_UPPER_BOUNDARY (40 * 1000 * 1000)//40MB

/* Number of frames that will be parsed to calculate average duration value.*/
#define MAX_FRAMES    (150)

/*! Minimum Frame Size used
  ! For low resolution clips, max frame size calculated based on Height and
  ! Width fields sometimes resulting in reallocation buffer request.
  ! To avoid this, minimum frame size is calculated by using VGA resolution.
  ! For resolutions lower than VGA, Parser will use the below value as Max
  ! Buffer Size required.
  ! VGA resolution is 640X480.
*/
#define MIN_MEDIA_FRAME_SIZE (640 * 480 * 0.75)

#define DOUBLEWORD_LENGTH (0x4L)
#define QUADWORD_LENGTH   (0x8L)
#define MAX_STRING_LENGTH (0x200L)
/*! File signature bytes Sizes
  ! Add new Signature Bytes in increasing order and update the macro
  ! "FILE_FORMAT_BUF_SIZE" with the largest ENUM value. These ENUMs should
  ! not be featurized irrespective of whether file format is supported or not.
  ! Else compilation issues may come. For 3gp file format, Atom Size (4Bytes)
  ! + Atom Type (4Bytes). So we need to read 8bytes. Same is the case with some
  ! other file formats as AVI, QCP and WAV where we have to read 12bytes.
*/
typedef enum
{
  MPEG2_TS_SIGNATURE_BYTES       =      1,  /* Signature bytes: 0x47 */
  AC3_FILE_SIGNATURE_BYTES       =      2,  /* Signature bytes: 0x77 0x0B */
  MP3_SYNC_SIGNATURE_BYTES       =      2,  /* Signature bytes: 0xFF0E*/
  MPEG2_PS_SIGNATURE_BYTES       =      3,  /* Signature bytes: 0x000001 */
  FLV_SIGNATURE_BYTES            =      3,  /* Signature bytes: "FLV"  */
  OGG_FILE_SIGNATURE_BYTES       =      4,  /* Signature bytes: "OggS" */
  FLAC_FILE_SIGNATURE_BYTES      =      4,  /* Signature bytes: "fLaC" */
  MKV_FILE_SIGNATURE_BYTES       =      4,  /* Signature bytes: 0x1A45DFA3 */
  AMR_FILE_SIGNATURE_BYTES       =      6,  /* Signature bytes: "#!AMR\n" */
  EVRC_WB_FILE_SIGNATURE_BYTES   =      8,  /* Signature bytes: "#!EVCWB\n" */
  QRAW_FILE_SIGNATURE_BYTES      =      8,  /* Signature bytes: "QRAWxxxx */
  AMR_WB_FILE_SIGNATURE_BYTES    =      9,  /* Signature bytes: "#!AMR-WB\n" */
  EVRC_B_FILE_SIGNATURE_BYTES    =      9,  /* Signature bytes: "#!EVRC-B\n" */
  AAC_FILE_SIGNATURE_BYTES       =      10, /* Signature bytes: AAC file header for ADTS/ADIF/LOAS*/
  AVI_FILE_SIGNATURE_BYTES       =      12, /* Signature bytes: "RIFF"<4Bytes>"AVI " */
  WAVADPCM_FILE_SIGNATURE_BYTES  =      12, /* Signature bytes: "RIFF"<4bytes>"WAVE" */
  QCP_FILE_SIGNATURE_BYTES       =      12, /* Signature bytes: "RIFF"<4bytes>"QLCM" */
  ASF_FILE_SIGNATURE_BYTES       =      16, /* Signature bytes: 0x3026b275,
                                               0x8e66cf11, 0xa6d900aa, 0x0062ce6c */
  MP4_3GPP2_FILE_SIGNATURE_BYTES =      20, /* Signature bytes: <4Bytes Size><4Bytes Type>
                                               Type has to be one of the :
                                               "ftyp", "moov", "mdat" */
  DTS_FILE_SIGNATURE_BYTES       =      64, /* Signature bytes: 0x7FFE8001
                                               Some of the WAV clips also carry
                                               DTS data. In that case, we need
                                               validate WAVE header before DTS
                                               sync marker. */
  MP3_FILE_WITH_ID3_TAG_BYTES    =    8202, /* Signature Bytes: Assuming ID3 size 4k(Max)
                                               ID3-1(4096)|ID3-2(4096)|FrameHeader(10)*/
  MAX_SIGNATURE_BYTES            =    MP3_FILE_WITH_ID3_TAG_BYTES
} FILE_SIGNATURE_BYTES;

/*  Read 1024 bytes into local cache. This is sufficient to validate most
 *  of the containers. Depending on necessity, extend this buffer size.
 */
#define FILE_FORMAT_BUF_SIZE           (0x400L)

#define VIDEO_MAX_LANGUAGE_BYTES            4
/* 3 byte GPS latitude */
#define QTV_MAX_GPS_LATITUDE                3
/* 3 byte GPS longitude */
#define QTV_MAX_GPS_LONGITUDE               3
/* 3 byte GPS time */
#define QTV_MAX_GPS_TIME                    3
#define MAX_MEDIA_HANDLES                   3

//! Macros to check the status of the FileSource
#define FILE_SOURCE_SUCCEEDED(x) (x == FILE_SOURCE_SUCCESS)
#define FILE_SOURCE_FAILED(x)    (x != FILE_SOURCE_SUCCESS)

//! Define MAX macro
#define  FILESOURCE_MAX( x, y ) ( ((x) > (y)) ? (x) : (y) )
//!Define MIN macro
#define  FILESOURCE_MIN( x, y ) ( ((x) < (y)) ? (x) : (y) )

//!Define ABS for 32 bit variables
#define  FILESOURCE_ABS32( x )  ( ( (int32)(x) > 0 ) ? (x) : -(x) )

//! Reverse bytes order
#define REVERSE_BYTES_ORDER(x) ((((uint32)(((uint8 *) &x) [0])) << 24) \
                              | (((uint32)(((uint8 *) &x) [1])) << 16) \
                              | (((uint32)(((uint8 *) &x) [2])) << 8) \
                              | ((uint32) (((uint8 *) &x) [3])))
enum DataT
  {
    /* Atom Types */
    DATA_ATOM_NONE = 0,
    DATA_ATOM_FTYP = 1,             /* File Type Atom            */
    DATA_ATOM_DCMD = 2,             /* Dcmd DRM Atom             */
    DATA_ATOM_UDTA_CPRT = 3,        /* Copyright Atom            */
    DATA_ATOM_UDTA_AUTH = 4,        /* Author Atom               */
    DATA_ATOM_UDTA_TITL = 5,        /* Title Atom                */
    DATA_ATOM_UDTA_DSCP = 6,        /* Description Atom          */
    DATA_ATOM_UDTA_MIDI = 7,        /* Midi Atom                 */
    DATA_ATOM_UDTA_LINK = 8,        /* Link Atom                 */
    DATA_TEXT_TKHD_ORIGIN_X =9,     /* Text track origin_x       */
    DATA_TEXT_TKHD_ORIGIN_Y =10,    /* Text track origin_y       */
    DATA_TEXT_TKHD_WIDTH =11,       /* Text track width          */
    DATA_TEXT_TKHD_HEIGHT =12,      /* Text track height         */
    DATA_MEDIA_INFO=13,             /* Media information         */
    DATA_ATOM_UDTA_RTNG = 14,       /* Rating Atom               */
    DATA_ATOM_UDTA_PERF = 15,       /* Performance Atom          */
    DATA_ATOM_UDTA_CLSF = 16,       /* Classification Atom       */
    DATA_ATOM_UDTA_KYWD = 17,       /* Keyword Atom              */
    DATA_ATOM_UDTA_LOCI = 18,       /* Location Atom             */
    DATA_ATOM_UDTA_GNRE = 19,       /* Genre Atom                */
    DATA_ATOM_UDTA_META = 20,       /* Meta Atom                 */
    DATA_ATOM_UDTA_ALBUM = 21,      /* Album Atom                */
    DATA_ATOM_UUID = 22,            /* UUID Atom                 */
    DATA_ATOM_TX3G = 23,            /* Text TX3G Atom            */
    DATA_ATOM_STPP = 24,            /* Text STPP Atom            */
    DATA_ATOM_SCHM = 25,
    DATA_ATOM_FRMA = 26,
    DATA_ATOM_OHDR = 27,
    DATA_ATOM_ODAF = 28,
    DATA_ATOM_MDRI = 29,
    DATA_ATOM_SUBS = 30,            /* Subs Atom                */
    DATA_ATOM_PSSH = 31,            /* PSSH Atom                */
    DATA_ATOM_SCHI = 32,            /* Schi Atom                */
    DATA_ATOM_SINF = 33,            /* Sinf Atom                */
    DATA_ATOM_TENC = 34,            /* Tenc Atom                */
    /* Here we are returning the offset of the atom including the header.*/
    DATA_ATOM_SCHM_OFFSET = 35,
    DATA_ATOM_FRMA_OFFSET = 36,
    DATA_ATOM_OHDR_OFFSET = 37,
    DATA_ATOM_ODAF_OFFSET = 38,
    DATA_ATOM_MDRI_OFFSET = 39,
    DATA_ATOM_UDTA_YEAR   = 40,
  } ;

  enum EncryptionTypeT
  {
    // type of encryption
    ENCRYPT_NONE = 0,
    ENCRYPT_OMA_DRM_V2,
    ENCRYPT_WM_DRM,
    ENCRYPT_UNSUPPORTED
  } ;

/*!
*@brief Structure to store DRM key information.
*/
struct DRMKey
{
  //! Pointer to DRM key
  void   *pKey;
  //! Size (in bytes) of the DRM key
  uint32  dwSize;
};

/** The PARSER_ERRORTYPE enumeration defines the standard Parser
 *  Errors.  These errors should cover most of the common
 *  failure cases.  On need basis we can add errors, if there
 *  are some errors specific to file formats.
 */
typedef enum PARSER_ERRORTYPE
{
  PARSER_ErrorNone = 0,

  /** This error used to indicate unknown errors. This one will
   *  be used as Default error to intialize the Error ENUM
   *  variable. */
  PARSER_ErrorDefault = (int32) 0x80001000,

  /** There were invalid input Params to perform */
  PARSER_ErrorInvalidParam = (int32) 0x80001001,

  /** Invalid TrackID */
  PARSER_ErrorInvalidTrackID = (int32) 0x80001002,

  /** Memory allocation is failed */
  PARSER_ErrorMemAllocFail = (int32) 0x80001003,

  /** Insufficient Buffer Size */
  PARSER_ErrorInsufficientBufSize = (int32) 0x80001004,

  /** Functionality is not supported. For eg, in some clips
      seek may not be supported. */
  PARSER_ErrorUnsupported = (int32) 0x80001005,

  /** ZERO Sample Size is found */
  PARSER_ErrorZeroSampleSize = (int32) 0x80001006,

  /** Data Under-run case. It occurs in streaming scenario. In
   *  seek operations also, this error will be used to indicate
   *  data under run. File-Source will use this map to the
   *  appropriate error code. */
  PARSER_ErrorDataUnderRun = (int32) 0x80001007,

  /** File/Buf Read operation Failure. This error is also used
   *  for File Seek operation Failure as well. */
  PARSER_ErrorReadFail = (int32) 0x80001008,

  /** End of File is reached. */
  PARSER_ErrorEndOfFile = (int32) 0x80001009,

  /** Stream/Track is found to be corrupt */
  PARSER_ErrorStreamCorrupt = (int32) 0x8000100A,

  /** Unsupported/Unknown Codec types */
  PARSER_ErrorUnsupportedCodecType = (int32) 0x8000100B,

  /** Error while parsing header of file format*/
  PARSER_ErrorInHeaderParsing = (int32) 0x8000100C,

  /** Functionality has not yet implemented */
  PARSER_ErrorNotImplemented = (int32) 0x8000100D,

  /** Codec Type is unknown */
  PARSER_ErrorUnknownCodecType = (int32) 0x8000100E,

  /** I/p File contains ZERO tracks */
  PARSER_ErrorZeroTracks = (int32) 0x8000100F,

  /** DRM Authorization Error */
  PARSER_ErrorDRMAuthorization = (int32) 0x80001010,

  /** DRM Device is not registered */
  PARSER_ErrorDRMDeviceNotRegistered = (int32) 0x80001011,

  /** DRM Rental Count is expired */
  PARSER_ErrorDRMRentalCountExpired = (int32) 0x80001012,

  /** DRM Memory Allocation is failed */
  PARSER_ErrorDRMMemAllocFail = (int32) 0x80001013,

  /** DRM playback general Error */
  PARSER_ErrorDRMPlaybackError = (int32) 0x80001014,

  /** Codec Info is parsed */
  PARSER_ErrorCodecInfo = (int32) 0x80001015,

  /** Data is at Fragment Boundary  */
  PARSER_ErrorDataFragment = (int32) 0x80001016,

  /** Error while Parser does not have enough data in seek case.*/
  PARSER_ErrorSeekUnderRunInFragment = (int32) 0x80001017,

  /** Error during Seek Operation */
  PARSER_ErrorSeekFail = (int32) 0x80001018

} PARSER_ERRORTYPE;


/** The ParserStatusCode enumeration defines the standard Parser
 *  status info.
 */
typedef enum ParserStatusCode
{
  //Initial Parser state when parser handle is created.
  PARSER_IDLE,
  //Parser state when parsing begins.
  PARSER_INIT,
  //Parser state when parsing is successful.
  PARSER_READY,
  //Parser is seeking to a given timestamp.
  PARSER_SEEK,
  //Read failed
  PARSER_READ_FAILED,
  //Data being read is corrupted.
  PARSER_FAILED_CORRUPTED_FILE,
  //Parser state it fails to allocate memory,
  //that is, when malloc/new fails.
  PARSER_NO_MEMORY,
  //Parser is in Pause state
  PARSER_PAUSE,
  //Parser is in Resume state
  PARSER_RESUME,
  //Parser initialization failed
  PARSER_FAILED,
  //Parser state if requested amount of data is
  // not downloaded in PD or PS cases
  PARSER_UNDERRUN
} ParserStatusCode;

/*!
*@brief AVC/HVC NALU Type information
*/
typedef enum{
  NALU_UNSPECIFIED_TYPE = 0,
  NALU_NON_IDR_TYPE,                //1 (VCL Slice)
  // NALU 2-4 are reserved in HVC
  NALU_SLICE_DATA_PART_A,           //2
  NALU_SLICE_DATA_PART_B,           //3
  NALU_SCLIE_DATA_PART_C,           //4
  NALU_IDR_TYPE,                    //5 (VCL Slice )
  NALU_SEI_TYPE,                    //6
  NALU_SPS_TYPE,                    //7
  NALU_PPS_TYPE,                    //8
  NALU_AU_DELIMITER_TYPE,           //9
  // NALU 10-11 are reserved in HVC
  NALU_END_OF_SEQ,                  //10
  NALU_END_OF_STREAM,               //11
  NALU_FILTER_DATA_TYPE,            //12
  NALU_RESERVED_TYPE                //NALU 13-23 reserved & 23-32 unspecified
}NAL_UNIT_TYPE;

/***********************************************
 * AUDIO Parser specific data definition
 ***********************************************/

//Audio parameters (samples per frame)
static const int EVRC_SAMPLES_PER_FRAME       = 160;
static const int AMR_SAMPLES_PER_FRAME        = 160;
static const int QCELP_SAMPLES_PER_FRAME      = 160;
static const int MP3_SAMPLES_PER_FRAME        = 576;
static const int AMR_WB_SAMPLES_PER_FRAME     = 320;
static const int AMR_WB_PLUS_SAMPLES_PER_FRAME= 2048;
static const int EVRC_NB_SAMPLES_PER_FRAME    = 160;
static const int EVRC_WB_SAMPLES_PER_FRAME    = 320;

/*!
 *@brief AAC File Format specific constant
 */
static const uint8 ucLATAMMask[9] = {0x2E, 'm','p','4',0x0A,'L','A','T','M'};
#define ADTS_HEADER_MASK            (0xF6FF)
#define ADTS_HEADER_MASK_RESULT     (0xF0FF)
#define LOAS_HEADER_MASK            (0xE0FF)
#define LOAS_HEADER_MASK_RESULT     (0xE056)
#define AAC_ADTS_HEADER_SIZE 7

//28 bits are fix in every ADTS frame header
#define AAC_ADTS_FIX_HDR_SIZE 4
#define AAC_SAMPLES_PER_DATA_BLOCK 1024
#define AAC_MAX_AUDIO_OBJECT 5
#define AAC_MAX_CHANNELS 48

// This look up table converts AAC sampling frequency index values into
// sampling frequencies in Hertz.
static const unsigned int AAC_SAMPLING_FREQUENCY_TABLE [] = {
96000, 88200, 64000, 48000, 44100, 32000, 24000, 22050, 16000,
12000, 11025, 8000, 7350, 0, 0, 0
};
#define AAC_SAMPLINGFREQ_TABLE_SIZE 16
/*!
 *@brief OGG File Format specific constant
 */
static const uint8 OGG_SIG_BYTES[FOURCC_SIGNATURE_BYTES] ={0x4f,0x67,0x67,0x53};

/*!
 *@brief FLAC File Format specific constant
 */
static const uint8 FLAC_SIG_BYTES[FOURCC_SIGNATURE_BYTES] ={0x66,0x4C,0x61,0x43};

/* METADATAINDEX is used to map between FileSource enum types and
   Metadata strings in MKAV file format.
*/
typedef enum
{
  TAG_OGG_TITLE,
  TAG_OGG_VERSION,
  TAG_OGG_ALBUM,
  TAG_OGG_TRACKNUMBER,
  TAG_OGG_ARTIST,
  TAG_OGG_PERFORMER,
  TAG_OGG_COPYRIGHT,
  TAG_LICENSE,
  TAG_ORGANIZATION,
  TAG_OGG_DESCRIPTION,
  TAG_OGG_GENRE,
  TAG_OGG_DATE,
  TAG_OGG_LOCATION,
  TAG_OGG_CONTACT,
  TAG_OGG_COMPOSER,
  TAG_OGG_ALBUMARTIST,
  TAG_OGG_ISRC,
  TAG_OGG_LOOP,
  MAX_FIELDNAMES_SUPPORTED
} OGGMETADATAINDEX;

/* OGG Metadata structure is used in FLAC file also. Instead of replicating
   same variables in both parsers, these enums and structures are kept in
   common header file. */
static unsigned char const OGGFieldNames[MAX_FIELDNAMES_SUPPORTED][13] =
{
  "TITLE",
  "VERSION",
  "ALBUM",
  "TRACKNUMBER",
  "ARTIST",
  "PERFORMER",
  "COPYRIGHT",
  "LICENSE",
  "ORGANIZATION",
  "DESCRIPTION",
  "GENRE",
  "DATE",
  "LOCATION",
  "CONTACT",
  "COMPOSER",
  "ALBUMARTIST",
  "ISRC",
  "ANDROID_LOOP",
};

/*!
 *@brief MP3 stream specific data definition
 */

/*!
   The following defines are used to extract all data from the MP3 header.
   Each is divided into the byte offset into the header
   the byte mask for the bits the number of bits to right-shift to
   extract a 0-based value
*/
#define MP3HDR_VERSION_OFS 1
#define MP3HDR_VERSION_M 0x18
#define MP3HDR_VERSION_SHIFT 3

#define MP3HDR_LAYER_OFS 1
#define MP3HDR_LAYER_M 0x06
#define MP3HDR_LAYER_SHIFT 1

#define MP3HDR_CRC_OFS 1
#define MP3HDR_CRC_M 0x01
#define MP3HDR_CRC_SHIFT 0

#define MP3HDR_BITRATE_OFS 2
#define MP3HDR_BITRATE_M 0xF0
#define MP3HDR_BITRATE_SHIFT 4

#define MP3HDR_SAMPLERATE_OFS 2
#define MP3HDR_SAMPLERATE_M 0x0C
#define MP3HDR_SAMPLERATE_SHIFT 2

#define MP3HDR_PADDING_OFS 2
#define MP3HDR_PADDING_M 0x02
#define MP3HDR_PADDING_SHIFT 1

#define MP3HDR_PRIVATE_OFS 2
#define MP3HDR_PRIVATE_M 0x01
#define MP3HDR_PRIVATE_SHIFT 0

#define MP3HDR_CHANNEL_OFS 3
#define MP3HDR_CHANNEL_M 0xC0
#define MP3HDR_CHANNEL_SHIFT 6

#define MP3HDR_CHANNEL_EXT_OFS 3
#define MP3HDR_CHANNEL_EXT_M 0x30
#define MP3HDR_CHANNEL_EXT_SHIFT 4

#define MP3HDR_COPYRIGHT_OFS 3
#define MP3HDR_COPYRIGHT_M 0x08
#define MP3HDR_COPYRIGHT_SHIFT 3

#define MP3HDR_ORIGINAL_OFS 3
#define MP3HDR_ORIGINAL_M 0x06
#define MP3HDR_ORIGINAL_SHIFT 2

#define MP3HDR_EMPHASIS_OFS 3
#define MP3HDR_EMPHASIS_M 0x03
#define MP3HDR_EMPHASIS_SHIFT 0

// Maximum possible samplerate index
#define MP3_MAX_SAMPLERATE_INDEX 3

// Maximum possible types of MPEG version
#define MAX_VERSION_COUNT 4

// Maximum possible bitrate index
#define MP3_MAX_BITRATE_INDEX 15

// Maximum possible types of layer
#define MAX_LAYER_COUNT 4

//sampling rates : 1. index = MPEG Version ID, 2. index = sampling rate index
static const uint32 MP3_SAMPLING_RATE[MAX_VERSION_COUNT]
                                     [MP3_MAX_SAMPLERATE_INDEX] =
{
   {11025, 12000, 8000,  },    // MPEG 2.5
   {0,     0,     0,     },    // reserved
   {22050, 24000, 16000, },    // MPEG 2
   {44100, 48000, 32000  }     // MPEG 1
};

// bitrates: 1. index = LSF, 2. index = Layer, 3. index = bitrate index
static const uint32 MP3_BITRATE[2][MAX_LAYER_COUNT]
                                  [MP3_MAX_BITRATE_INDEX] =
{
   {   // MPEG 1
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, },            //reserved
      {0,32,40,48, 56, 64, 80, 96,112,128,160,192,224,256,320,},  // Layer3
      {0,32,48,56, 64, 80, 96,112,128,160,192,224,256,320,384,},  // Layer2
      {0,32,64,96,128,160,192,224,256,288,320,352,384,416,448,}   // Layer1
   },
   {   // MPEG 2, 2.5
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, },           //reserved
      {0,8,16,24,32,40,48,56,64,80,96,112,128,144,160,},         // Layer3
      {0,8,16,24,32,40,48,56,64,80,96,112,128,144,160,},         // Layer2
      {0,32,48,56,64,80,96,112,128,144,160,176,192,224,256,}     // Layer1
   }
};

// Samples per Frame: 1. index = LSF, 2. index = Layer
static const uint32 MP3_SAMPLES_TABLE[2][MAX_LAYER_COUNT] =
{
   { // MPEG 1
      0,    //reserved
      1152, // Layer3
      1152, // Layer2
      384   // Layer1
   },
   { // MPEG 2, 2.5
      0,    //reserved
      576,  // Layer3
      1152, // Layer2
      384   // Layer1
   }
};

// Samples per Frame / 8
static const uint32 MP3_COEFFICIENTS[2][MAX_LAYER_COUNT] =
{
   { // MPEG 1
      0,   // reserved
      144, // Layer3
      144, // Layer2
      12   // Layer1 (must be multiplied with 4, because of slot size)
   },
   { // MPEG 2, 2.5
      0,   // reserved
      72,  // Layer3
      144, // Layer2
      12   // Layer1 (must be multiplied with 4, because of slot size)
   }
};

// slot size per layer
static const uint32 MP3_SLOT_SIZES[MAX_LAYER_COUNT] =
{
   0,          // reserved
   1,          // Layer3
   1,          // Layer2
   4           // Layer1
};

// MP3 Version info
typedef enum
{
  MP3_VER_25,            /* MPEG Version 2.5                              */
  MP3_VER_RESERVED,      /* Reserved                                      */
  MP3_VER_2,             /* MPEG Version 2.0                              */
  MP3_VER_1,             /* MPEG Version 1.0                              */
}mp3_ver_enum_type;

// MP3 Layer info
typedef enum
{
  MP3_LAYER_RESERVED = 0,  /* Reserved                                    */
  MP3_LAYER_3,             /* MPEG Layer 3 compression                    */
  MP3_LAYER_2,             /* MPEG Layer 2 compression                    */
  MP3_LAYER_1,             /* MPEG Layer 1 compression                    */
}mp3_layer_enum_type;

/* MP3 Emphasis info
*/
typedef enum
{
  MP3_EMPHASIS_NONE,     /* Emphasis flag                                 */
  MP3_EMPHASIS_50_15_MS, /*                                               */
  MP3_EMPHASIS_RESERVED, /*                                               */
  MP3_EMPHASIS_CCITT_J17 /*                                               */
} mp3_emphasis_enum_type;

// MP3 Channel info
typedef enum
{
  MP3_CHANNEL_STEREO,       /* Stereo data                                */
  MP3_CHANNEL_JOINT_STEREO, /* Joint stereo data                          */
  MP3_CHANNEL_DUAL,         /* Dual channel (stereo) data                 */
  MP3_CHANNEL_SINGLE        /* Single channel (mono) data                 */
} mp3_channel_enum_type;

typedef enum
{
  /* For Layer 1 & 2 files */
  MP3_EXT_BAND_4_31 = 0,            /* Channel extension info, 4-31       */
  MP3_EXT_BAND_8_31 = 1,            /*                         8-31       */
  MP3_EXT_BAND_12_31 = 2,           /*                        12-31       */
  MP3_EXT_BAND_16_31 = 3,           /*                        16-31       */
  /* For Layer 3 files */
  MP3_EXT_INTENSITY_OFF_MS_OFF = 0, /* Intensity stereo off, MS off       */
  MP3_EXT_INTENSITY_ON_MS_OFF = 1,  /*                  on      off       */
  MP3_EXT_INTENSITY_OFF_MS_ON = 2,  /*                  off     on        */
  MP3_EXT_INTENSITY_ON_MS_ON = 3    /*                  on      on        */
} mp3_ext_enum_type;

/// MP3 Technical Metadata found in file and frame headers
struct tech_data_mp3
{
  mp3_ver_enum_type version;       ///< mpeg version
  mp3_layer_enum_type layer;       ///< mpeg layer
  boolean crc_present;             ///< is CRC present in the frame headers
  uint32 bitrate;                  ///< bit rate
  uint32 max_bitrate;              ///<.max bit rate
  uint32 samplerate;               ///< sampling rate
  boolean is_padding;              ///< true if frame has padding
  boolean is_private;              ///< true if private bit is set
  mp3_channel_enum_type channel;   ///< number of channels
  mp3_ext_enum_type extension;     ///< extension
  boolean copyright_present;       ///< true if copyright bit is set
  boolean is_original;             ///< true if this an original copy
  mp3_emphasis_enum_type emphasis; ///< emphasis
};

struct AC3HeaderInfo
{
  uint8  ucBSID;
  uint32 ulFrameLen;
  uint32 ulSamplingFreq;
};
/*!
 *@brief AC3/EAC3 stream specific data definition
 */

/*!
 *@brief AC3 Frame Size Code Table
 */
static const struct
{
  uint32  ulFrameBitRate;   /*!< Nominal Bit-rate */
  uint32  ulFrameSize[3];   /*!< Words/syncframe  */
}AC3_FRAME_SIZE_CODE[38]= {
/*b-rate*//*48*//*44.1*//*32*/
  { 32  ,{  64,    69,   96 } },
  { 32  ,{  64,    70,   96 } },
  { 40  ,{  80,    87,  120 } },
  { 40  ,{  80,    88,  120 } },
  { 48  ,{  96,   104,  144 } },
  { 48  ,{  96,   105,  144 } },
  { 56  ,{  112,  121,  168 } },
  { 56  ,{  112,  122,  168 } },
  { 64  ,{  128,  139,  192 } },
  { 64  ,{  128,  140,  192 } },
  { 80  ,{  160,  174,  240 } },
  { 80  ,{  160,  175,  240 } },
  { 96  ,{  192,  208,  288 } },
  { 96  ,{  192,  209,  288 } },
  { 112 ,{  224,  243,  336 } },
  { 112 ,{  224,  244,  336 } },
  { 128 ,{  256,  278,  384 } },
  { 128 ,{  256,  279,  384 } },
  { 160 ,{  320,  348,  480 } },
  { 160 ,{  320,  349,  480 } },
  { 192 ,{  384,  417,  576 } },
  { 192 ,{  384,  418,  576 } },
  { 224 ,{  448,  487,  672 } },
  { 224 ,{  448,  488,  672 } },
  { 256 ,{  512,  557,  768 } },
  { 256 ,{  512,  558,  768 } },
  { 320 ,{  640,  696,  960 } },
  { 320 ,{  640,  697,  960 } },
  { 384 ,{  768,  835, 1152 } },
  { 384 ,{  768,  836, 1152 } },
  { 448 ,{  896,  975, 1344 } },
  { 448 ,{  896,  976, 1344 } },
  { 512 ,{ 1024, 1114, 1536 } },
  { 512 ,{ 1024, 1115, 1536 } },
  { 576 ,{ 1152, 1253, 1728 } },
  { 576 ,{ 1152, 1254, 1728 } },
  { 640 ,{ 1280, 1393, 1920 } },
  { 640 ,{ 1280, 1394, 1920 } }
};

/*!
 *@brief AC3 Sample Rate Code (2 bits)
 * For EAC3: if(fscod = '11')-> fscod2
 *   fscod2     sampling-rate(kHz)
 *    '00'      24
 *    '01'      22.05
 *    '10'      16
 *    '11'      Reserved
 */
static const uint32 AC3_FSCODE_RATE[4]= {
  /*sample rate */  /*fscod           */
  48000,            /*!<'00'          */
  44100,            /*!<'01'          */
  32000,            /*!<'10'          */
  0                 /*!<'11' Reserved */
};


/* The following two tables are required in TS Parser to parse AC3 descriptor.
   Spec "ETSI TS 102 366 V1.2.1 (2008-08)" Section A.2.4 has complete details.*/
/*!
 *@brief AC3 Bit Rate Code (5 bits)
 *  bit-rate code   bit-rate(kbit/s)
 *  00000           32
 *  00001           40
 *  00010           48
 *  00011           56
 *  00100           64
 *  00101           80
 *  00110           96
 *  00111           112
 *  01000           128
 *  01001           160
 *  01010           192
 *  01011           224
 *  01100           256
 *  01101           320
 *  01110           384
 *  01111           448
 *  10000           512
 *  10001           576
 *  10010           640
 */
static const uint32 AC3_BITRATE_CODE[19]={
   32,  40,  48,  56,
   64,  80,  96, 112,
  128, 160, 192, 224,
  256, 320, 384, 448,
  512, 576, 640
};

/* As per specification, below values are recommended.
   In case of multiple values, we use first mentioned value in
   specifications.
    "000" 48
    "001" 44,1
    "010" 32
    "011" Reserved
    "100" 48 or 44,1
    "101" 48 or 32
    "110" 44,1 or 32
    "111" 48 or 44,1 or 32
*/
static const uint32 AC3_SAMPLE_RATE[] =
{
  48000, 44100, 32000, 0,
  48000, 48000, 44100, 48000
};

/* BSMOD - Bit Stream Mode(3bit): section 5.4.2.2, A/52B-2005            */
typedef enum
{
  BSI_BSMOD_CM = 0, /*!< Main audio service: complete main(CM)           */
  BSI_BSMOD_ME,     /*!< Main audio service: music & effects(ME)         */
  BSI_BSMOD_VI,     /*!< Main audio service: visually impaired(VI)       */
  BSI_BSMOD_HI,     /*!< Main audio service: hearing imparied(HI)        */
  BSI_BSMOD_D,      /*!< Main audio service: dialogue (D)                */
  BSI_BSMOD_C,      /*!< Main audio service: commentary(C)               */
  BSI_BSMOD_E,      /*!< Main audio service: emmergency(E)               */
  BSI_BSMOD_VO_K    /*!< Main audio service: voice over or karaoke(VO_E) */
}BSI_BSMOD;

/* ACMOD - Audio Coding Mode(3bit): section 5.4.2.3, A/52B-2005                        */
static const uint32 ACMOD_CHANNELS[8] = {2,1,2,3,3,4,4,5};
typedef enum
{
  BSI_ACMOD_CH1_CH2 = 0, /*!< Dual Mono (CH1, CH2)                                     */
  BSI_ACMOD_C,           /*!< Mono:(C)                                                 */
  BSI_ACMOD_L_R,         /*!< Stereo:(L_R)                                             */
  BSI_ACMOD_L_C_R,       /*!< Left, Centre, Right:(L_C_R)                              */
  BSI_ACMOD_L_R_S,       /*!< Left, Right, Surround :(L_R_S)                           */
  BSI_ACMOD_L_C_R_S,     /*!< Left, Centre,Right, Surround :(L_C_R_S)                  */
  ACMOD_L_R_SL_SR,       /*!< Left, Right, Surround L, Surround R:(L_R_SL_SR)          */
  ACMOD_L_C_R_SL_SR      /*!< Left, Centre, Right, Surround L, Surround R:(L_C_R_SL_SR)*/
}BSI_ACMOD;

static const uint8 CHAN_LOC[9]= \
 { 2 /* Lc/Rc pair   */,
   1 /* Lrs/Rrs pair */,
   1 /* Cs           */,
   1 /* Ts           */,
   2 /* Lsd/Rsd pair */,
   2 /* Lw/Rw pair   */,
   2 /* Lvh/Rvh pair */,
   1 /* Cvh          */,
   1 /* LFE2         */};

/* AC3/EAC3 Sync word*/
#define AUDIO_FMT_IS_AC3_SYNC(x) \
  (((((byte *)(x))[0] == 0x77) && ((((byte *)(x))[1] == 0x0B))) || \
   ((((byte *)(x))[0] == 0x0B) && ((((byte *)(x))[1] == 0x77))))

/* BSI - Bit Stream Information */
#define BSID_STANDARD         8  /* Standard ATSC A/52 bit-stream ID */
#define BSID_ANNEX_D          6  /* Annex D bit-stream ID */
#define BSID_ANNEX_E          16 /* Annex E bit-stream ID */
#define IS_DOLBY_DIGITAL(ucBSID)       ((ucBSID) <= BSID_STANDARD )
#define IS_DOLBY_DIGITAL_PLUS(ucBSID)  ((ucBSID) <= BSID_ANNEX_E && (ucBSID) > 10 )

/* DTS Specific parameters */

/* DTS Frame sync markers for different extensions.
   These frame sync markers are taken from the standard
   "DTS-HD Substream and Decoder Interface Description"
   Version 3.2
   Effective Date: August 2007
   Document no: 9302F30400
   Revision A
*/
static const uint8 DTS_SYNCWORD_MPG_PS[FOURCC_SIGNATURE_BYTES] =
                                            {0x88,0x01,0x00,0x01};
static const uint8 DTS_SYNCWORD_CORE[FOURCC_SIGNATURE_BYTES] =
                                            {0x7F, 0xFE, 0x80, 0x01};
static const uint8 DTS_SYNCWORD_XCH[FOURCC_SIGNATURE_BYTES] =
                                            {0x5A, 0x5A, 0x5A, 0x5A};
static const uint8 DTS_SYNCWORD_XXCH[FOURCC_SIGNATURE_BYTES] =
                                            {0x47, 0x00, 0x4A, 0x03};
static const uint8 DTS_SYNCWORD_X96K[FOURCC_SIGNATURE_BYTES] =
                                            {0x1D, 0x95, 0xF2, 0x62};
static const uint8 DTS_SYNCWORD_XBR[FOURCC_SIGNATURE_BYTES] =
                                            {0x65, 0x5E, 0x31, 0x5E};
static const uint8 DTS_SYNCWORD_LBR[FOURCC_SIGNATURE_BYTES] =
                                            {0x0A, 0x80, 0x19, 0x21};
static const uint8 DTS_SYNCWORD_XLL[FOURCC_SIGNATURE_BYTES] =
                                            {0x41, 0xA2, 0x95, 0x47};
static const uint8 DTS_SYNCWORD_SUBSTREAM[FOURCC_SIGNATURE_BYTES] =
                                            {0x64, 0x58, 0x20, 0x25};
static const uint8 DTS_SYNCWORD_SUBSTREAM_CORE2[FOURCC_SIGNATURE_BYTES] =
                                            {0x02, 0xB0, 0x92, 0x61};

static const uint8 DTS_SYNCWORD_CORE_LE[FOURCC_SIGNATURE_BYTES] =
                                            {0xFE, 0x7F, 0x01, 0x80};
static const uint8 DTS_SYNCWORD_LBR_LE[FOURCC_SIGNATURE_BYTES] =
                                            {0x80, 0x0A, 0x21, 0x19};
static const uint8 DTS_SYNCWORD_SUBSTREAM_LE[FOURCC_SIGNATURE_BYTES] =
                                            {0x58, 0x64, 0x25, 0x20};
//SYNC is stored in three words: 0x1fff, 0xe800, and 0x07f.
static const uint8 DTS_SYNCWORD_PCM_LE[] =
                                            {0xFF, 0x1F, 0x00, 0XE8, 0xF1, 0X07};
static const uint8 DTS_SYNCWORD_PCM[] =
                                            {0x1F, 0xFF, 0XE8, 0x00, 0X07, 0xF1};
#define DTS_14BIT_SYNC_MARKER_SIZE 6
static const uint8 DTS_HD_FRAME_SYNC_MARKER[] =
                          {'D', 'T', 'S', 'H', 'D', 'H', 'D', 'R'};
#define DTSHD_FRAME_SYNC_SIZE 8

/*!
 *@brief DTS Sample Rate Code (4 bits)
   This is for Core Audio
  0b0000 Invalid
  0b0001 8kHz
  0b0010 16kHz
  0b0011 32kHz
  0b0100 Invalid
  0b0101 Invalid
  0b0110 11.025kHz
  0b0111 22.05kHz
  0b1000 44.1kHz
  0b1001 Invalid
  0b1010 Invalid
  0b1011 12kHz
  0b1100 24kHz
  0b1101 48kHz
  0b1110 Invalid
  0b1111 Invalid
 */
static const uint32 DTS_FSCODE_RATE[16]= {
      0,  8000, 16000, 32000,
      0,     0, 11025, 22050,
  44100,     0,     0, 12000,
  24000, 48000,     0,     0
};

/*!
 *@brief DTS Sample Rate Code (4 bits)
   This is for LBR
  0b0000 8kHz
  0b0001 16kHz
  0b0010 32kHz
  0b0011 Invalid
  0b0100 Invalid
  0b0101 22.05kHz
  0b0110 44.1kHz
  0b0111 Invalid
  0b1000 Invalid
  0b1001 Invalid
  0b1010 12khz
  0b1011 24kHz
  0b1100 48kHz
  0b1101 Invalid
  0b1110 Invalid
  0b1111 Invalid
 */
static const uint32 DTS_LBR_FSCODE_RATE[16]= {
   8000, 16000, 32000,     0,
      0, 22050, 44100,     0,
      0,     0, 12000, 24000,
  48000,     0,     0,     0
};

/*!
 *@brief DTS Bit-Rate Code (5 bits)
0b00000 32kbps
0b00001 56kbps
0b00010 64kbps
0b00011 96kbps
0b00100 112kbps
0b00101 128kbps
0b00110 192kbps
0b00111 224kbps
0b01000 256kbps
0b01001 320kbps
0b01010 384kbps
0b01011 448kbps
0b01100 512kbps
0b01101 576kbps
0b01110 640kbps
0b01111 768kbps
0b10000 960kbps
0b10001 1024kbps
0b10010 1152kbps
0b10011 1280kbps
0b10100 1344kbps
0b10101 1408kbps
0b10110 1411.2kbps
0b10111 1472kbps
0b11000 1536kbps
0b11001 1920kbps
0b11010 2048kbps
0b11011 3072kbps
0b11100 3840kbps
0b11101 Open
0b11110 Variable
0b11111 Lossless
*/

static const uint32 DTS_BIT_RATE[32]= {
    32000,   56000,   64000,   96000,
   112000,  128000,  192000,  224000,
   256000,  320000,  384000,  448000,
   512000,  576000,  640000,  768000,
   960000, 1024000, 1152000, 1280000,
  1344000, 1408000, 1411200, 1472000,
  1536000, 1920000, 2048000, 3072000,
  3840000,       0,       0,       0
};

#define MAX_DTS_CHANNELS_INDEX 16
/* ACMOD - Audio Coding Mode(5bits).
   However more than 16 indexes are not currently defined */
static const uint16 DTS_CHANNELS[16] =
  {1,2,2,2,2,3,3,4,4,5,6,6,6,7,8,8};

/************************************************************************/
/* HDMV-LPCM codec parameters                                           */
/************************************************************************/
static const uint32 HD_LPCM_SAMPLING_FREQ[6] =
  { 0, 48000, 0, 0, 96000, 192000 };
static const uint8 HD_LPCM_CHANNEL_INFO[8] =
  { 0, 1, 2, 2, 2, 2, 4, 0 };
//! Case 2 means 20bits per sample, but takes 24bits
static const uint8 HD_LPCM_BIT_WIDTH[4] =
{ 0, 16, 24, 24 };

/***********************************************
 * VIDEO Format specific data definition
 ***********************************************/

/*!
 *@brief 3GP/MP4 File Format specific constant
 * Reference: ISO_IEC-14496-12-2008(E)/ISO_IEC-14496-14
 */
static const uint8 MP4_3GP_ATOM_FTYP[FOURCC_SIGNATURE_BYTES] ={'f','t','y','p'};
static const uint8 MP4_3GP_ATOM_MOOV[FOURCC_SIGNATURE_BYTES] ={'m','o','o','v'};
static const uint8 MP4_3GP_ATOM_MDAT[FOURCC_SIGNATURE_BYTES] ={'m','d','a','t'};
static const uint8 MP4_3GP_BRAND_K3G[FOURCC_SIGNATURE_BYTES] ={'k','3','g','1'};
static const uint8 MP4_3GP_BRAND_SKM[FOURCC_SIGNATURE_BYTES] ={'s','k','m','3'};
static const uint8 MP4_3GP_BRAND_KDDI[FOURCC_SIGNATURE_BYTES]={'k','d','d','i'};
static const uint8 MP4_3GP_BRAND_HEVC[FOURCC_SIGNATURE_BYTES]={'h','v','c','1'};
static const uint8 MP4_3GP_BRAND_DASH[FOURCC_SIGNATURE_BYTES]={'d','a','s','h'};
/*!
 *@brief 3GPP2 File Format specific constant
 * Section 8.1.1 (3GPP2 C.S0050-B v1.0)
 */
static const uint8 BRAND_3G2_REL_0[FOURCC_SIGNATURE_BYTES] ={'3','g','2','a'};
static const uint8 BRAND_3G2_REL_A[FOURCC_SIGNATURE_BYTES] ={'3','g','2','b'};
static const uint8 BRAND_3G2_REL_B[FOURCC_SIGNATURE_BYTES] ={'3','g','2','c'};

/*!
 *@brief M2TS & PS(VOB) File Format specific constant
 */
static const uint8 MPEG2_SIG_BYTES[MPEG2_PS_SIGNATURE_BYTES] ={0x00,0x00,0x01};
static const uint8 MPEG2_TS_SIG_BYTES[MPEG2_TS_SIGNATURE_BYTES] = {0x47};
static const uint8 MPEG2_TS_PKT_BYTES = 188;
static const uint8 MPEG2_M2TS_PKT_BYTES = 192;

/*!
 *@brief FLV File Format specific constant
 */
static const uint8 FLV_SIG_BYTES[] ={0x46,0x4C,0x56};
/*!
 *@brief MKV File Format specific constant
 */
static const uint8 MKV_SIG_BYTES[FOURCC_SIGNATURE_BYTES] ={0x1A,0x45,0xDF,0xA3};

/*!
 *@brief ASF(WMA/WMV) File Format specific constant
 * Reference: Advanced System Format(ASF)Specification Rev.01.20.05(06,2010)
 */
static const uint8 asfFileIdentifier[] =
  {0x30, 0x26, 0xb2, 0x75, 0x8e, 0x66, 0xcf, 0x11, 0xa6, 0xd9, 0x00, 0xaa, 0x00, 0x62, 0xce, 0x6c};

#endif

