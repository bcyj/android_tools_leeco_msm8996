#ifndef MP2_STREAM_CONSTANTS_DEFN
#define MP2_STREAM_CONSTANTS_DEFN

/* =======================================================================
                              MP2StreamMuxConstants.h
DESCRIPTION
  Meaningful description of the definitions contained in this file.
  Description must specify if the module is portable specific, mobile
  specific, or common to both, and it should alert the reader if the
  module contains any conditional definitions which tailors the module to
  different targets.

Copyright (c) 2011 Qualcomm Technologies, Inc., All Rights Reserved
Qualcomm Technologies Proprietary and Confidential.
========================================================================== */

/* =======================================================================
                             PERFORCE HEADER
$Header: //source/qcom/qct/multimedia2/Video/Source/FileMux/MP2BaseFileLib/main/latest/inc/MP2StreamMuxConstants.h
========================================================================== */

#include "AEEStdDef.h"

//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
/*Following are the constants to be used when Muxing MPEG2 Transport stream*/
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

//EACH TS packet is 188 bytes long
const uint8 TS_PKT_SIZE                    = 188;
//EACH TS packet has 4 bytes header
const uint8 MUX_TS_PKT_HDR_BYTES               = 4;
//Each PSI table has 4 bytes CRC in it
const uint8 MUX_CRC_BYTES                      = 4;
//Number of bytes in Program map section header
const uint8 MUX_PROG_MAP_SECT_HDR_BYTES        = 9;
//Number of bytes including section header and CRC bytes
const uint8 MUX_SECTION_HDR_CRC_BYTES          = 9;
//Number of bytes describing PID that carries program definition.
const uint8 MUX_PROGRAM_NO_PID_BYTES           = 4;
//A PSI table can have at the max 1024 bytes
const uint16 MUX_MAX_PSI_SECTION_BYTES         = 1024;
//A section can have at the max, 1021 bytes.
const uint16 MUX_MAX_SECT_LENGTH               = 1021;
//Private data can be at the max 4096 bytes.
const uint16 MUX_MAX_PVT_SECTION_DATA_BYTES    = 4096;
//Default time scale to be used for elementary streams
const uint32 MUX_MPEG2_STREAM_TIME_SCALE       = 1000;
//EACH TS packet starts with '0x47'
const uint8  TS_PKT_SYNC_BYTE              = 0x47;
//sateesh need to make sure this is correct
const uint8 PES_PKT_SIZE      = 128;
const uint32 MUX_PES_PKT_START_CODE            = 0x000001;
/*
REFER TO PID TABLE, Table 2-3, page 19
Indicates type of data stored in TS packet payload
*/
const uint16 MUX_TS_PROG_ASSOCIATION_TBL_PID   = 0x0000;
const uint16 MUX_TS_CONDITIONAL_ACCESS_TBL_PID = 0x0001;
const uint16 MUX_TS_DESC_TBL_PID               = 0x0002;
const uint16 MUX_TS_RESERVED_PID_START         = 0x0003;
const uint16 MUX_TS_RESERVED_PID_END           = 0x000F;
const uint16 MUX_TS_GENERAL_PURPOSE_PID_START  = 0x0010;
const uint16 MUX_TS_GENERAL_PURPOSE_PID_END    = 0x1FFE;
const uint16 MUX_TS_NULL_PKT_PID               = 0x1FFF;
const uint16 MUX_TS_NW_INFO_PID                = 0x0010;
const uint16 MUX_TS_SDT_BAT_ST_PID             = 0x0011;
const uint16 MUX_TS_EIT_ST_PID                 = 0x0012;
const uint16 MUX_TS_RST_ST_PID                 = 0x0013;
const uint16 MUX_TS_TDT_TOT_ST_PID             = 0x0014;
const uint16 MUX_TS_NETWORK_SYNCHRONIZATION    = 0x0015;
const uint16 MUX_TS_RESERVED_FUTURE_PID_START  = 0x0016;
const uint16 MUX_TS_RESERVED_FUTURE_PID_END    = 0x001B;
/*
Adaption Field Control Values,page 20, Table 2-5
Indicates whether TS packet header will be followed by
adaptionfield data and/or payload.
*/
const uint8  MUX_TS_ADAPTION_FILED_RESERVED          = 0;
const uint8  MUX_TS_ADAPTION_FILED_ABSENT_PYLD_ONLY  = 1;
const uint8  MUX_TS_ADAPTION_FILED_PRESENT_NO_PYLD   = 2;
const uint8  MUX_TS_ADAPTION_FILED_DATA_PRSENT       = 3;

/*
TABLE IDs, Table 2-26, page 44,
Identifies the content of transport stream PSI section.
*/
//porgram association section
const uint8 MUX_TS_PSI_PA_TABLE_ID = 0x00;
//conditional access section
const uint8 MUX_TS_PSI_CA_TABLE_ID = 0x01;
//program map section
const uint8 MUX_TS_PSI_PM_TABLE_ID = 0x02;
//description section
const uint8 MUX_TS_PSI_DS_TABLE_ID = 0x03;
//scne description
const uint8 MUX_TS_PSI_SD_TABLE_ID = 0x04;
//object description
const uint8 MUX_TS_PSI_OD_TABLE_ID = 0x05;
//reserved range
const uint8 MUX_TS_PSI_RES_TABLE_START_ID = 0x06;
const uint8 MUX_TS_PSI_RES_TABLE_END_ID = 0x37;
//defined in ISO/IEC 13818-6
const uint8 MUX_TS_PSI_ISO_IEC_13818_6_START_ID = 0x38;
const uint8 MUX_TS_PSI_ISO_IEC_13818_6_END_ID = 0x3F;
//User private range
const uint8 MUX_TS_PSI_PVT_START_ID = 0x40;
const uint8 MUX_TS_PSI_PVT_END_ID = 0xFE;
//Forbidden section
const uint8 MUX_TS_PSI_FORBIDDEN_ID = 0xFF;

//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
/* Following are the constants to be used when MPEG2 Program stream*/
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

/* Start/End codes to identify MPEG2 Program Stream*/
const uint32 MUX_MP2_PROGRAM_END_CODE         = 0x000001B9;
const uint32 MUX_MP2_PACK_START_CODE          = 0x000001BA;
const uint32 MUX_MP2_SYS_HDR_START_CODE       = 0x000001BB;
const uint16 MUX_MAX_INIT_PACKS_TO_PARSE      = 100;
const uint16 MUX_MAX_INIT_TSPKTS_TO_PARSE     = 500;
const uint16 MUX_MAX_PS_PACK_SIZE             = 2048;

//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
/* Following are the constants to be used when parsing PES packets*/
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

const uint32 MUX_PES_PKT_START_CODE_STREAM_ID_SIZE = 6;
const uint8  MUX_PES_PKT_FIXED_HDR_BYTES           = 3;
const uint8  MUX_PES_PKT_PTS_BYTES                 = 5;
const uint8  MUX_PES_PKT_PTS_DTS_BYTES             = 10;
const uint8  MUX_PES_PKT_ESCR_BYTES                = 6;
const uint8  MUX_PES_PKT_ES_RATE_BYTES             = 3;
const uint8  MUX_PES_PKT_DSM_TRICK_MODE_BYTES      = 1;
const uint8  MUX_PES_PKT_COPY_INFO_BYTES           = 1;
const uint8  MUX_PES_PKT_PES_CRC_BYTES             = 2;
const uint8  MUX_PES_EXTN_FIXED_HDR_BYTES          = 1;
const uint8  MUX_PES_PKT_EXTN_BYTES                = 24;
const uint8  MUX_PES_EXTN_PVT_DATA_BYTES           = 16;
const uint8  MUX_PES_EXTN_PACK_FIELD_LEN_BYTES     = 1;
const uint8  MUX_PES_EXTN_PKT_SEQ_COUNTER_BYTES    = 2;
const uint8  MUX_PES_EXTN_P_STD_BUFFER_BYTES       = 2;
const uint8  MUX_PES_EXTN_FLAG2_BYTES              = 2;
const uint8  MUX_RATE_MEASURED_UNITS               = 50;
const uint32 MUX_TS_PES_PKT_TOTAL_HEADER_SIZE      = 175;
//TS_PES_PKT_TOTAL_HEADER_SIZE = TS_PKT_SIZE – ( TS_PKT_HDR_BYTES + PES_PKT_START_CODE_STREAM_ID_SIZE + PES_PKT_FIXED_HDR_BYTES);
//Presentation Control Information Packet(PCI packet)
const uint8  MUX_PCI_PKT_SUBSTREAM_ID              = 0x00;
//Data Search Information Packet(DSI packet)
const uint8  MUX_DSI_PKT_SUBSTREAM_ID              = 0x01;
const uint16 MUX_PCI_PKT_INFO_BYTES                = 50;
const uint16 MUX_DSI_PKT_START_END_PTM_OFFSET      = 44;
const uint16 MUX_NEXT_VOBU_OFFSET_INDEX            = 234;
const uint16 MUX_PRV_VOBU_OFFSET_INDEX             = 398;
const uint16 MUX_DSI_PKT_INFO_BYTES                = 402;
const uint32 MUX_NO_VIDEO_VOBU_CODE                = 0xbfffffff;
const uint32 MUX_NO_VOBU_WITHIN_CELL_SPAN          = 0x3fffffff;
//------------------------------------------------------------------------
/* Stream-Ids,Table 2-18, page 34,Identifies the content of PES packet.*/
//------------------------------------------------------------------------
const uint32 MUX_PROG_STREAM_MAP_ID           = 0xBC;
const uint32 MUX_PRIVATE_STREAM1_ID           = 0xBD;
const uint32 MUX_PADDING_STREAM_ID            = 0xBE;
const uint32 MUX_PRIVATE_STREAM2_ID           = 0xBF;
const uint32 MUX_AUDIO_STREAM_ID_START        = 0xC0;
const uint32 MUX_AUDIO_STREAM_ID_END          = 0xDF;
const uint32 MUX_VIDEO_STREAM_ID_START        = 0xE0;
const uint32 MUX_VIDEO_STREAM_ID_END          = 0xEF;
const uint32 MUX_ECM_STREAM_ID                = 0xF0;
const uint32 MUX_EMM_STREAM_ID                = 0xF1;
const uint32 MUX_DSMCC_STREAM_ID              = 0xF2;
const uint32 MUX_ISO_OEC_13522_STREAM_ID      = 0xF3;
const uint32 MUX_H222_TYPE_A_STREAM_ID        = 0xF4;
const uint32 MUX_H222_TYPE_B_STREAM_ID        = 0xF5;
const uint32 MUX_H222_TYPE_C_STREAM_ID        = 0xF6;
const uint32 MUX_H222_TYPE_D_STREAM_ID        = 0xF7;
const uint32 MUX_H222_TYPE_E_STREAM_ID        = 0xF8;
const uint32 MUX_ANCILLARY_STREAM_ID          = 0xF9;
const uint32 MUX_SL_PACKETIZED_STREAM_ID      = 0xFA;
const uint32 MUX_FLEX_MUX_STREAM_ID           = 0xFB;
const uint32 MUX_RES_DATA_STREAM_START_ID     = 0xFC;
const uint32 MUX_RES_DATA_STREAM_END_ID       = 0xFE;
const uint32 MUX_PROG_STREAM_DIRECTORY_ID     = 0xFF;
const uint8  MUX_PACK_HDR_BYTES               = 14;
const uint8  MUX_SYS_HDR_BYTES                = 8;
const uint8  MUX_SYS_HDR_STREAM_ID_INFO_BYTES = 4;

//------------------------------------------------------------------------
/* Stream-Types,Table 2-18, page 34,Identifies the content of PES packet.*/
//------------------------------------------------------------------------
const uint32 MUX_MPEG2_VIDEO_STREAM_TYPE      = 0x02;
const uint32 MUX_AVC_VIDEO_STREAM_TYPE        = 0x1B;

//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
/*Constants to be used while parsing video elementary stream data*/
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

const uint32 MUX_VIDEO_START_CODE_PREFIX      = 0x000001;
const uint32 MUX_FULL_PICTURE_START_CODE      = 0x00000100;
const uint32 MUX_FULL_USER_DATA_START_CODE    = 0x000001B2;
const uint32 MUX_FULL_SEQUENCE_HEADER_CODE    = 0x000001B3;
const uint32 MUX_FULL_SEQUENCE_ERROR_CODE     = 0x000001B4;
const uint32 MUX_FULL_EXTENSION_START_CODE    = 0x000001B5;
const uint32 MUX_FULL_SEQUENCE_END_CODE       = 0x000001B7;
const uint32 MUX_FULL_GROUP_START_CODE        = 0x000001B8;
const uint8  MUX_PICTURE_START_CODE   	  	  = 0x00;
const uint8  MUX_USER_DATA_START_CODE 	  	  = 0xB2;
const uint8  MUX_SEQUENCE_HEADER_CODE 	  	  = 0xB3;
const uint8  MUX_SEQUENCE_ERROR_CODE  	  	  = 0xB4;
const uint8  MUX_EXTENSION_START_CODE 	  	  = 0xB5;
const uint8  MUX_SEQUENCE_END_CODE    	  	  = 0xB7;
const uint8  MUX_GROUP_START_CODE     	  	  = 0xB8;
const uint32 MUX_AFD_START_CODE     	  	  = 0x44544731;

#define VIDEO_PVTDATA_TYPE_FILLERNALU           0
#define VIDEO_PVTDATA_TYPE_FILLERNALU_ENCRYPTED 1

#define MP2_CRC_GENERATOR_POLYNOMIAL   0x04c11db7L
#define MSBIT(x)    (x & 0x80000000L )

#define MUX_MP2_NO_ADAPTATION_PAYLOAD_ONLY  1
#define MUX_MP2_ADAPTATION_ONLY             2
#define MUX_MP2_ADAPTATON_AND_PAYLOAD       3
#define MUX_MP2_PTS_DTS                     3

#define MUX_MP2_SYSTEM_REFERENCE_CLOCK      27000000L
#define MUX_MP2_PCR_BASE_DIVISOR            300
#define MUX_MP2_PTS_DTS_REFERENCE_CLOCK     90000L

#define MUX_MP2_PMT_FIX_SIZE                13
#define MUX_MP2_PTS_DELAY_CONST_90KHZ       18000 //200ms             

#define MUX_MP2_AVC_HRD_DESC_SIZE           17

//------------------------------------------------------------------

//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
/* Constants to be used while parsing audio elementary stream data*/
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

typedef struct _MUX_FRAME_SIZE_CODE_TABLE_
{
  uint8 framesizecod;
  uint16 nominal_bit_rate;
  uint16 words_per_syncframe_32;
  uint16 words_per_syncframe_44_1;
  uint16 words_per_syncframe_48;
}MUX_FRAME_CODE_TABLE;

typedef struct _MUX_NO_CHANNELS_TABLE
{
  uint8  acmod;
  char*  AudioCodingMode;
  uint8  nfchans;
}MUX_NO_CHANNELS_TABLE;

const uint8  MUX_LPCM_AUDIO_SUBSTREAM_ID_BEG   = 0xA0; //1010 0***
const uint8  MUX_LPCM_AUDIO_SUBSTREAM_ID_END   = 0xA7; //1010 0***
const uint8  MUX_AC3_AUDIO_SUBSTREAM_ID_BEG    = 0x80; //1000 0***
const uint8  MUX_AC3_AUDIO_SUBSTREAM_ID_END    = 0x87; //1000 0***
const uint16 MUX_AC3_SYNC_WORD                 = 0x0B77;
const uint8  MUX_AC3_SUBSTREAM_SYNC_INFO_BYTES = 4;
const uint8  MUX_SAMPLE_RATE_48_KHZ            = 0x00;
const uint8  MUX_SAMPLE_RATE_44_1_KHZ          = 0x01;
const uint8  MUX_SAMPLE_RATE_32_KHZ            = 0x02;
const uint8  MUX_SAMPLE_RATE_RESERVED          = 0x03;

//------------------------------------------------------------------
/*Trick mode control values, table 2-20,page 37*/
const uint8  MUX_TRICK_MODE_CONTROL_FAST_FORWARD   = 0x00;
const uint8  MUX_TRICK_MODE_CONTROL_SLOW_MOTION    = 0x01;
const uint8  MUX_TRICK_MODE_CONTROL_FREEZE_FRAME   = 0x02;
const uint8  MUX_TRICK_MODE_CONTROL_FAST_REVERSE   = 0x03;
const uint8  MUX_TRICK_MODE_CONTROL_SLOW_REVERSE   = 0x04;
#endif//MP2_STREAM_CONSTANTS_DEFN
