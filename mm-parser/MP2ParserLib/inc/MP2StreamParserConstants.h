#ifndef MP2_STREAM_CONSTANTS_DEFN
#define MP2_STREAM_CONSTANTS_DEFN

/* =======================================================================
                              MP2StreamParserConstants.h
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
$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/MP2ParserLib/main/latest/inc/MP2StreamParserConstants.h#44 $
========================================================================== */

#include "AEEStdDef.h"

//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
/*Following are the constants to be used when parsing MPEG2 Transport stream*/
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
#define MP2_TS_CACHE_SIZE 128000

/* Minimum number of TS Packets that are parsed to get PMT.
   If PMT is not found within this range, report failure.*/
#define MIN_TS_PACKETS (10000)
//EACH TS packet is 188 bytes long
const uint8 TS_PKT_SIZE                    = 188;
//Initial 4 bytes bytes
const uint8 TS_INIT_BYTES                  = 4;
//EACH TS packet has 4 bytes header
const uint8 TS_PKT_HDR_BYTES               = 4;
//EACH M2TS packet has 4 bytes extra header
const uint8 M2TS_PKT_EXTRA_HDR_BYTES       = 4;
//Each PSI table has 4 bytes CRC in it
const uint8 CRC_BYTES                      = 4;
//Number of bytes in Program map section header
const uint8 PROG_MAP_SECT_HDR_BYTES        = 9;
//Number of bytes including section header and CRC bytes
const uint8 SECTION_HDR_CRC_BYTES          = 9;
//Number of bytes describing PID that carries program definition.
const uint8 PROGRAM_NO_PID_BYTES           = 4;
//A PSI table can have at the max 1024 bytes
const uint16 MAX_PSI_SECTION_BYTES         = 1024;
//A section can have at the max, 1021 bytes.
const uint16 MAX_SECT_LENGTH               = 1021;
//Private data can be at the max 4096 bytes.
const uint16 MAX_PVT_SECTION_DATA_BYTES    = 4096;
//Default time scale to be used for elementary streams
const uint32 MPEG2_STREAM_TIME_SCALE       = 1000;
//EACH TS packet starts with '0x47'
const uint8  TS_PKT_SYNC_BYTE              = 0x47;
const uint32 PES_PKT_START_CODE            = 0x000001;
const uint32 PES_PTS_DELTA_DISCONTINUITY   = 10;

/* Continuity counter value goes from 0-15,
   it wraps around after reacing 15 */
const uint8  MAX_CONTINUITY_COUNTER_VALUE  = 15;
/*
REFER TO PID TABLE, Table 2-3, page 19
Indicates type of data stored in TS packet payload
*/
const uint16 TS_PROG_ASSOCIATION_TBL_PID   = 0x0000;
const uint16 TS_CONDITIONAL_ACCESS_TBL_PID = 0x0001;
const uint16 TS_DESC_TBL_PID               = 0x0002;
const uint16 TS_RESERVED_PID_START         = 0x0003;
const uint16 TS_RESERVED_PID_END           = 0x000F;
const uint16 TS_GENERAL_PURPOSE_PID_START  = 0x0010;
const uint16 TS_GENERAL_PURPOSE_PID_END    = 0x1FFE;
const uint16 TS_NULL_PKT_PID               = 0x1FFF;
const uint16 TS_NW_INFO_PID                = 0x0010;
const uint16 TS_SDT_BAT_ST_PID             = 0x0011;
const uint16 TS_EIT_ST_PID                 = 0x0012;
const uint16 TS_RST_ST_PID                 = 0x0013;
const uint16 TS_TDT_TOT_ST_PID             = 0x0014;
const uint16 TS_NETWORK_SYNCHRONIZATION    = 0x0015;
const uint16 TS_RESERVED_FUTURE_PID_START  = 0x0016;
const uint16 TS_RESERVED_FUTURE_PID_END    = 0x001B;
const uint32 AVC_START_CODE_PREFIX_32BIT   = 0x00000001;
const uint32 AVC_START_CODE_PREFIX_24BIT   = 0x000001;
const uint32 AVC_START_CODE_SIZE           = 4;

const uint8 NAL_UNIT_TYPE_SPS             = 7;
const uint8 NAL_UNIT_TYPE_PPS             = 8;
const uint8 NAL_UNIT_TYPE_IDR             = 5;
const uint8 NAL_UNIT_TYPE_NON_IDR         = 1;

const uint8 MPEG2_I_FRAME_TYPE             = 1;
const uint8 MPEG2_P_FRAME_TYPE             = 2;
const uint8 MPEG2_B_FRAME_TYPE             = 3;

// VC1 Codes
// VC-1 Compressed Video Bit-stream
// Advanced Profile Picture Type VLC
//110  I
//0    P
//10   B
//1110 BI
const uint8 VC1_SEQ_START_CODE                = 0xF;
const uint8 VC1_FRAME_START_CODE              = 0xD;
const uint8 VC1_I_FRAME_TYPE                  = 0x6;
const uint8 VC1_P_FRAME_TYPE                  = 0x0;
const uint8 VC1_B_FRAME_TYPE                  = 0x2;
const uint8 VC1_BI_FRAME_TYPE                 = 0xE;

/*
Adaption Field Control Values,page 20, Table 2-5
Indicates whether TS packet header will be followed by
adaptation-field data and/or payload.
*/
const uint8  TS_ADAPTION_FILED_RESERVED          = 0;
const uint8  TS_ADAPTION_FILED_PRESENT_NO_PYLD   = 2;
const uint8  TS_ADAPTION_FILED_DATA_PRSENT       = 3;
const uint8  TS_ADAPTION_FILED_ABSENT_PYLD_ONLY  = 1;

const uint8  TS_ADPT_PLYD_MAX_LEN    = 182;
const uint8  TS_ADPT_NOPLYD_MAX_LEN  = 183;
/*
TABLE IDs, Table 2-26, page 44,
Identifies the content of transport stream PSI section.
*/
//porgram association section
const uint8 TS_PSI_PA_TABLE_ID = 0x00;
//conditional access section
const uint8 TS_PSI_CA_TABLE_ID = 0x01;
//program map section
const uint8 TS_PSI_PM_TABLE_ID = 0x02;
//description section
const uint8 TS_PSI_DS_TABLE_ID = 0x03;
//scne description
const uint8 TS_PSI_SD_TABLE_ID = 0x04;
//object description
const uint8 TS_PSI_OD_TABLE_ID = 0x05;
//reserved range
const uint8 TS_PSI_RES_TABLE_START_ID = 0x06;
const uint8 TS_PSI_RES_TABLE_END_ID = 0x37;
//defined in ISO/IEC 13818-6
const uint8 TS_PSI_ISO_IEC_13818_6_START_ID = 0x38;
const uint8 TS_PSI_ISO_IEC_13818_6_END_ID = 0x3F;
//User private range
const uint8 TS_PSI_PVT_START_ID = 0x40;
const uint8 TS_PSI_PVT_END_ID = 0xFE;
//Forbidden section
const uint8 TS_PSI_FORBIDDEN_ID = 0xFF;
//Program Descriptor header Length
const uint8 TS_DESC_HEADER_LEN = 0x02;
//!Elementary Descriptor header length for TS
const uint8 TS_ELEMSTREAM_DESC_HEADER_LEN = 0x05;

//!Elementary Descriptor header length for MPG/PS
const uint8 PS_ELEMSTREAM_DESC_HEADER_LEN = 0x04;
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
/* Following are the constants to be used when MPEG2 Program stream*/
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

/* Start/End codes to identify MPEG2 Program Stream*/
const uint32 MP2_PROGRAM_END_CODE         = 0x000001B9;
const uint32 MP2_PACK_START_CODE          = 0x000001BA;
const uint32 MP2_SYS_HDR_START_CODE       = 0x000001BB;
const uint16 MAX_INIT_PACKS_TO_PARSE      = 0x64;
const uint16 MAX_INIT_TSPKTS_TO_PARSE     = 0x1F4;
const uint16 MAX_PS_PACK_SIZE             = 0x800;

//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
/* Following are the constants to be used when parsing PES packets*/
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

const uint32 PES_PKT_START_CODE_STREAM_ID_SIZE = 6;
const uint8  PES_PKT_FIXED_HDR_BYTES           = 3;
const uint8  PES_PKT_PTS_BYTES                 = 5;
const uint8  PES_PKT_PTS_DTS_BYTES             = 10;
const uint8  PES_PKT_ESCR_BYTES                = 6;
const uint8  PES_PKT_ES_RATE_BYTES             = 3;
const uint8  PES_PKT_DSM_TRICK_MODE_BYTES      = 1;
const uint8  PES_PKT_COPY_INFO_BYTES           = 1;
const uint8  PES_PKT_PES_CRC_BYTES             = 2;
const uint8  PES_EXTN_FIXED_HDR_BYTES          = 1;
const uint8  PES_PKT_EXTN_BYTES                = 24;
const uint8  PES_EXTN_PVT_DATA_BYTES           = 16;
const uint8  PES_EXTN_PACK_FIELD_LEN_BYTES     = 1;
const uint8  PES_EXTN_PKT_SEQ_COUNTER_BYTES    = 2;
const uint8  PES_EXTN_P_STD_BUFFER_BYTES       = 2;
const uint8  PES_EXTN_FLAG2_BYTES              = 2;
const uint8  PES_EXTN_PVT_DATA_LEN             = 16;
const uint8  PES_PKT_STD_BUFFER_SIZE           = 2;
const uint8  RATE_MEASURED_UNITS               = 50;
const uint32 TS_PES_PKT_TOTAL_HEADER_SIZE      = 175;

const uint8  PES_PKT_PTS_FLAG                  = 0x02;
const uint8  PES_PKT_PTS_DTS_FLAG              = 0x03;
//Presentation Control Information Packet(PCI packet)
const uint8  PCI_PKT_SUBSTREAM_ID              = 0x00;
//Data Search Information Packet(DSI packet)
const uint8  DSI_PKT_SUBSTREAM_ID              = 0x01;
const uint16 PCI_PKT_INFO_BYTES                = 50;
const uint16 DSI_PKT_START_END_PTM_OFFSET      = 44;
const uint16 NEXT_VOBU_OFFSET_INDEX            = 234;
const uint16 PRV_VOBU_OFFSET_INDEX             = 398;
const uint16 DSI_PKT_INFO_BYTES                = 402;
const uint32 NO_VIDEO_VOBU_CODE                = 0xbfffffff;
const uint32 NO_VOBU_WITHIN_CELL_SPAN          = 0x3fffffff;
//------------------------------------------------------------------------
/* Stream-Ids,Table 2-18, page 34,Identifies the content of PES packet.*/
//------------------------------------------------------------------------
const uint32 PROG_STREAM_MAP_ID           = 0xBC;
const uint32 PRIVATE_STREAM1_ID           = 0xBD;
const uint32 PADDING_STREAM_ID            = 0xBE;
const uint32 PRIVATE_STREAM2_ID           = 0xBF;
const uint32 AUDIO_STREAM_ID_START        = 0xC0;
const uint32 AUDIO_STREAM_ID_END          = 0xDF;
const uint32 VIDEO_STREAM_ID_START        = 0xE0;
const uint32 VIDEO_STREAM_ID_END          = 0xEF;
const uint32 ECM_STREAM_ID                = 0xF0;
const uint32 EMM_STREAM_ID                = 0xF1;
const uint32 DSMCC_STREAM_ID              = 0xF2;
const uint32 ISO_OEC_13522_STREAM_ID      = 0xF3;
const uint32 H222_TYPE_A_STREAM_ID        = 0xF4;
const uint32 H222_TYPE_B_STREAM_ID        = 0xF5;
const uint32 H222_TYPE_C_STREAM_ID        = 0xF6;
const uint32 H222_TYPE_D_STREAM_ID        = 0xF7;
const uint32 H222_TYPE_E_STREAM_ID        = 0xF8;
const uint32 ANCILLARY_STREAM_ID          = 0xF9;
const uint32 SL_PACKETIZED_STREAM_ID      = 0xFA;
const uint32 FLEX_MUX_STREAM_ID           = 0xFB;
const uint32 RES_DATA_STREAM_START_ID     = 0xFC;
const uint32 RES_DATA_STREAM_END_ID       = 0xFE;
const uint32 PROG_STREAM_DIRECTORY_ID     = 0xFF;
const uint8  MPEG2_PACK_HDR_BYTES         = 0x0E;
const uint8  MPEG1_PACK_HDR_BYTES         = 0x0C;
const uint8  SYS_HDR_BYTES                = 0x06;
const uint8  SYS_HDR_STREAM_ID_INFO_BYTES = 0x03;

//------------------------------------------------------------------------
/* Stream-Types,Table 2-18, page 34,Identifies the content of PES packet.*/
//------------------------------------------------------------------------
// SMPTE RP227 Section 4.1.1 Stream Type
const uint32 VC1_VIDEO_STREAM_TYPE      = 0xEA;
const uint32 MPEG2_VIDEO_STREAM_TYPE    = 0x02;
const uint32 AVC_VIDEO_STREAM_TYPE      = 0x1B;
const uint32 MPEG4_VIDEO_STREAM_TYPE    = 0x10;
const uint32 MPEG4_AUDIO_STREAM_TYPE    = 0x11;
const uint32 AAC_ADTS_STREAM_TYPE       = 0x0F;
const uint32 MPEG1_AUDIO_STREAM_TYPE    = 0x03;
const uint32 MPEG2_AUDIO_STREAM_TYPE    = 0x04;
const uint32 LPCM_AUDIO_STREAM_TYPE     = 0x83;
const uint32 AC3_AUDIO_STREAM_TYPE      = 0x81;
const uint32 EAC3_AUDIO_STREAM_TYPE     = 0x87;
const uint32 PES_PVT_STREAM_TYPE        = 0x06;
const uint32 USER_PVT_STREAM_TYPE       = 0xBD;

//! Below Values are used if PMT has registration descriptor "HDMV"
//! These are defined in BluRay standard
const uint32 HDMV_LPCM_STREAM_TYPE        = 0x80;
const uint32 HDMV_AC3_STREAM_TYPE         = 0x81;
const uint32 HDMV_DTS_STREAM_TYPE         = 0x82;
const uint32 HDMV_AC3_LOSSLESSSTREAM_TYPE = 0x83;
const uint32 HDMV_EAC3_STREAM_TYPE        = 0x84;
const uint32 DTS_HD_EXCEPT_XLLSTREAM_TYPE = 0x85;
const uint32 DTS_HD_STREAM_TYPE           = 0x86;
const uint32 HDMV_DRA_STREAM_TYPE         = 0x87;

//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
/*Constants to be used while parsing video elementary stream data*/
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

const uint32 VIDEO_START_CODE_PREFIX      = 0x000001;
const uint32 FULL_PICTURE_START_CODE      = 0x00000100;
const uint32 FULL_USER_DATA_START_CODE    = 0x000001B2;
const uint32 FULL_SEQUENCE_HEADER_CODE    = 0x000001B3;
const uint32 FULL_SEQUENCE_ERROR_CODE     = 0x000001B4;
const uint32 FULL_EXTENSION_START_CODE    = 0x000001B5;
const uint32 FULL_SEQUENCE_END_CODE       = 0x000001B7;
const uint32 FULL_GROUP_START_CODE        = 0x000001B8;
const uint8  PICTURE_START_CODE           = 0x00;
const uint8  USER_DATA_START_CODE         = 0xB2;
const uint8  SEQUENCE_HEADER_CODE         = 0xB3;
const uint8  SEQUENCE_ERROR_CODE          = 0xB4;
const uint8  EXTENSION_START_CODE         = 0xB5;
const uint8  SEQUENCE_END_CODE            = 0xB7;
const uint8  GROUP_START_CODE             = 0xB8;
const uint32 AFD_START_CODE               = 0x44544731;

const uint32 AVC_4BYTE_START_CODE         = 0x00000001;
const uint32 AVC_3BYTE_START_CODE         = 0x000001;

const uint32 MPEG4_VO_SEQ                 = 0x000001B0;
const uint32 MPEG4_VOP_FRAME_START_CODE   = 0x000001B6;
/* VOP_CODING_TYPE (binary)  Coding method
                      00  intra-coded (I)
                      01  predictive-coded (P)
                      10  bidirectionally-predictive-coded (B)
                      11  sprite (S) */
const uint8 MPEG4_I_FRAME = 0x0;
const uint8 MPEG4_P_FRAME = 0x1;
const uint8 MPEG4_B_FRAME = 0x2;
const uint8 MPEG4_S_FRAME = 0x3;
//------------------------------------------------------------------

const uint8  TS_REGISTRATION_DESC_TAG     = 0x05;
const uint8  TS_CA_DESC_TAG               = 0x09;
const uint8  TS_AVC_DESC_TAG              = 0x28;
const uint8  TS_DTS_DESC_TAG              = 0x7b;
const uint8  TS_DTSHD_DESC_TAG            = 0x7f;
const uint8  TS_MPEG4_AUDIO_DESC_TAG      = 0x1c;
const uint8  TS_MPEG2_AAC_AUDIO_DESC_TAG  = 0x2b;
const uint8  TS_DVD_LPCM_AUDIO_DESC_TAG   = 0x83;
const uint8  TS_ISO_639_LANG_DESC_TAG     = 0x0a;
const uint8  TS_AC3_AUDIO_DESC_TAG        = 0x81;
const uint8  TS_EAC3_AUDIO_DESC_TAG       = 0xCC;
const uint32 AC3_SYS_B_AUDIO_DESC_TAG     = 0x6A;
const uint32 EC3_SYS_B_AUDIO_DESC_TAG     = 0x7A;

const uint32  TS_REGIS_FORMATID_DTS1      = 0x44545331;
const uint32  TS_REGIS_FORMATID_DTS2      = 0x44545332;
const uint32  TS_REGIS_FORMATID_DTS3      = 0x44545333;
const uint32  TS_REGIS_FORMATID_DTSH      = 0x44545348;

const uint32  HDMV_REG_DESCRIPTOR         = 0x48444D56; //"HDMV"
const uint32  AC3_REG_DESCRIPTOR          = 0x41432D33; //"AC-3"
const uint32  EC3_REG_DESCRIPTOR          = 0x43452D33; //"EC-3"
const uint32  EAC3_REG_DESCRIPTOR         = 0x45414333; //"EAC3"
const uint32  VC1_REG_DESCRIPTOR          = 0x56432D31; //"VC-1"

//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
/* Constants to be used while parsing audio elementary stream data*/
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
typedef struct _FRAME_SIZE_CODE_TABLE_
{
  uint8 framesizecod;
  uint16 nominal_bit_rate;
  uint16 words_per_syncframe_32;
  uint16 words_per_syncframe_44_1;
  uint16 words_per_syncframe_48;
}FRAME_CODE_TABLE;

typedef struct _NO_CHANNELS_TABLE
{
  uint8  acmod;
  const char*  AudioCodingMode;
  uint8  nfchans;
}NO_CHANNELS_TABLE;

const uint8  DVD_LPCM_FRAME_START_CODE     = 0xA0;
const uint8  LPCM_AUDIO_SUBSTREAM_ID_BEG   = 0xA0; //1010 0***
const uint8  LPCM_AUDIO_SUBSTREAM_ID_END   = 0xA7; //1010 0***
const uint8  AC3_AUDIO_SUBSTREAM_ID_BEG    = 0x80; //1000 0***
const uint8  AC3_AUDIO_SUBSTREAM_ID_END    = 0x87; //1000 0***
const uint16 AC3_SYNC_WORD                 = 0x0B77;
const uint8  AC3_SUBSTREAM_SYNC_INFO_BYTES = 4;
const uint8  SAMPLE_RATE_48_KHZ            = 0x00;
const uint8  SAMPLE_RATE_44_1_KHZ          = 0x01;
const uint8  SAMPLE_RATE_32_KHZ            = 0x02;
const uint8  SAMPLE_RATE_RESERVED          = 0x03;

const FRAME_CODE_TABLE FRAME_SIZE_CODE_TABLE [] = {
  {0x00,32,96,69,64},
  {0x01,32,96,70,64},
  {0x02,40,120,87,80},
  {0x03,40,120,88,80},
  {0x04,48,144,104,96},
  {0x05,48,144,105,96},
  {0x06,56,168,121,112},
  {0x07,56,168,122,112},
  {0x08,64,192,139,128},
  {0x09,64,192,140,128},
  {0x0A,80,240,174,160},
  {0x0B,80,240,175,160},
  {0x0C,96,288,208,192},
  {0x0D,96,288,209,192},
  {0x0E,112,336,243,224},
  {0x0F,112,336,244,224},
  {0x10,128,384,278,256},
  {0x11,128,384,279,256},
  {0x12,160,480,348,320},
  {0x13,160,480,349,320},
  {0x14,192,576,417,384},
  {0x15,192,576,418,384},
  {0x16,224,672,487,448},
  {0x17,224,672,488,448},
  {0x18,256,768,557,512},
  {0x19,256,768,558,512},
  {0x1A,320,960,696,640},
  {0x1B,320,960,697,640},
  {0x1C,384,1152,835,768},
  {0x1D,384,1152,836,768},
  {0x1E,448,1344,975,896},
  {0x1F,448,1344,976,896},
  {0x20,512,1536,1114,1024},
  {0x21,512,1536,1115,1024},
  {0x22,576,1728,1253,1152},
  {0x23,576,1728,1254,1152},
  {0x24,640,1920,1393,1280},
  {0x25,640,1920,1394,1280}
};

const NO_CHANNELS_TABLE CHANNELS_CONFIG[] = {
  {0x00,"1+1",2},
  {0x01,"1/0",1},
  {0x02,"2/0",2},
  {0x03,"3/0",3},
  {0x04,"2/1",3},
  {0x05,"3/1",4},
  {0x06,"2/2",4},
  {0x07,"3/2",5},
};

/*!
 *@brief TS DTS Sample Rate Code (4 bits)
   This is for Core Audio in Mpeg2 TS
  0     Invalid
  1     8kHz
  2     16kHz
  3     32kHz
  4     64kHz
  6     11.025kHz
  7     22.050kHz
  8     44.1kHz
  9     88.2kHz
  11    12kHz
  12    24kHz
  13    48kHz
  14    96kHz
 */
static const uint32 TS_DTS_FSCODE_RATE[15]= {
      0, 8000, 16000, 32000,
      64000, 0, 11025, 22050,
      44100, 88200, 0, 12000,
      24000, 48000, 96000
};

/*!
 *@brief TS DTSHD Sample Rate Code (4 bits)
   This is for Core Audio in Mpeg2 TS
  0     8kHz
  1     16kHz
  2     32kHz
  3     64kHz
  4*    128kHz
  5     22.050kHz
  6     44.1kHz
  7     88.2kHz
  8*    176.4kHz
  9*    352.8kHz
  10    12kHz
  11    24kHz
  12    48kHz
  13    96kHz
  14*   192kHz
  15*   384kHz
 */
//Values indicated with (*) are invalid for substream_core
static const uint32 TS_DTSHD_FSCODE_RATE[16]= {
      8000, 16000, 32000, 64000,
      0, 22050, 44100, 88200,
      0, 0, 12000, 24000,
      48000, 96000, 0, 0
};

/*!
 *@brief TS DTS Bit-Rate Code (5 bits)
0bx00101 128kbps
0bx00110 192kbps
0bx00111 224kbps
0bx01000 256kbps
0bx01001 320kbps
0bx01010 384kbps
0bx01011 448kbps
0bx01100 512kbps
0bx01101 576kbps
0bx01110 640kbps
0bx01111 768kbps
0bx10000 960kbps
0bx10001 1024kbps
0bx10010 1152kbps
0bx10011 1280kbps
0bx10100 1344kbps
0bx10101 1408kbps
0bx10110 1411.2kbps
0bx10111 1472kbps
0bx11000 1536kbps
0bx11101 OPEN
*/
// x indicates reserved bit

static const uint32 TS_DTS_BIT_RATE[21]= {
    128000,  192000,  224000,  256000,
    320000,  384000,  448000,  512000,
    576000,  640000,  768000,  960000,
    1024000, 1152000, 1280000, 1344000,
    1408000, 1411200, 1472000, 1536000,0
};

static const uint16 TS_DTS_CHANNELS[10] =
  {1,0,2,2,2,3,3,0,4,5};

//------------------------------------------------------------------
/*Trick mode control values, table 2-20,page 37*/
const uint8  TRICK_MODE_CONTROL_FAST_FORWARD   = 0x00;
const uint8  TRICK_MODE_CONTROL_SLOW_MOTION    = 0x01;
const uint8  TRICK_MODE_CONTROL_FREEZE_FRAME   = 0x02;
const uint8  TRICK_MODE_CONTROL_FAST_REVERSE   = 0x03;
const uint8  TRICK_MODE_CONTROL_SLOW_REVERSE   = 0x04;
//----------------------------------------------------------
/* BUFFER used by parser while parsing mpeg2 streams*/
const uint32 PS_DATA_BUFFER_SIZE               = 512000;
//! Minimum amount of data that needs to be read into PES buffer
//! before searching Codec config data
const uint32 MIN_DATA_READ                     = 0x800;
//----------------------------------------------------------
#define AAC_SAMPLES_PER_DATA_BLOCK 1024
#define AAC_MAX_AUDIO_OBJECT 5
#define AAC_MAX_CHANNELS 48

#define M2TS_AAC_ADTS_HDR_LEN 7
#define M2TS_MAX_ADTS_FRAMES_AT_ONCE 1

static const unsigned long PCM_SAMPLING_FREQUENCY_TABLE [] = { 0, 44100, 48000, 96000 };
static const unsigned long DVD_PCM_SAMPLING_FREQUENCY_TABLE [] = { 48000, 96000, 44100, 32000 };

#define MAX_PCM_SAMPLING_FREQ_INDEX_VALUES 4
#define M2TS_PCM_HDR_LEN 4
#define SEEK_TIMESTAMP_GAP_TOLERANCE_MS 500
#endif//MP2_STREAM_CONSTANTS_DEFN
