#ifndef QCP_HEADERS_H
#define QCP_HEADERS_H

/*=============================================================================
 FILE: qcpheaders.h

 DESCRIPTION:
 Declarations required for QCP Format parser implementation

  Copyright (c) 2009-2013 QUALCOMM Technologies Inc, All Rights Reserved.
  QUALCOMM Technologies Proprietary and Confidential.

=============================================================================
$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/QCPParserLib/main/latest/inc/qcpheaders.h#7 $
$DateTime: 2013/04/25 02:16:26 $
$Change: 3668452 $
============================================================================ */

//=============================================================================
// Includes
//=============================================================================

//=============================================================================
// DATA TYPES
//=============================================================================

typedef struct qcp_header_qcph_t
{
  uint16 nChannels;       ///< number of channels
  uint32 nSampleRate;    ///< sample rate
} qcp_header_qcph;

typedef struct qcp_audio_info_t
{
  uint32 dwSuggestedBufferSize; // Suggested buffer size for o/p buffer
} qcp_audio_info;

//qcp decoder specific information
typedef struct qcp_decode_info_t
{
  uint32 codec_version;
  uint32 cdma_rate;
}qcp_decode_info;


//qcp format type
typedef enum{
 QCP_FORMAT_UNKNOWN,
 QCP_FULLRATE_FIX,                    // EVRC/QCELP @ 13kbps/14.4kbps :Fix Rate
 QCP_HALFRATE_FIX,                    // EVRC/QCELP @ 6.8kbps/7.2kbps :Fix Rate
 QCP_FULLRATE_VAR,                    // EVRC/QCELP @ 13kbps/14.4kbps :Var Rate
 QCP_HALFRATE_VAR,                    // EVRC/QCELP @ 6.8kbps/7.2kbps :Var Rate
 QCP_SMV,                             // SMV Codec
 QCP_OTHER                            // EVRC/QCELP @ some other Bit Rate
}qcp_format_type;


///< QCP Technical Metadata found in file header

struct tech_data_qcp
{///< fmt chunk metadata , which is encoded in 150 Bytes

  ///< SubType = true is EVRC ,   SubType = false is QCELP
  boolean  SubType;
  ///< Flags to Identify Codec Type : {major == 2 ,minor == 0 then codec = SMV}
  ///< {major == 1 , minor == 0 then codec = QCELP/EVRC}
  uint8 major;
  uint8 minor;
  ///< 5E7F6D41/5E7F6D42-B115-11D0-BA-91-00-80-5F-B4-B9-7E is QCELP
  uint8 Codec_guid_codecID4[8];
  ///< Codec Name
  uint8 Codec_name[80];
  ///< should be read as 8*[rate-size(1 Byte) , rate-octet(1 Byte)]
  /// rate-size is the size of packet , excluding the value for rate octet
  /// This is the 1st octet of a packet in data chunk
  uint8 rate_map_entry[16];
  ///< label under labl chunk
  uint8 label[48];
  ///< String contains any info defined by the application in [text] chunk
  uint8 string[100];
  ///< Bit Mapped Configuration Word of [cnfg] chunk
  uint16 config_word;
  ///< For Example E689D48D-9076-46B5-91-EF-73-6A-51-00-CE-B4 is EVRC
  uint16 Codec_guid_codecID2;
  ///< 8D7C2B75-A797-ED49-98-5E-D5-3C-8C-C7-5F-84 is SMV
  uint16 Codec_guid_codecID3;
  ///< Version number of codec {QCELP - 1 or 2},{EVRC/SMV - 1}
  uint16 Codec_version;
  ///< Average data rate in Bits per second
  uint16 Average_bps;
  ///< Size of largest packet in data chunk in Bytes
  uint16 Packet_size;
  ///< Number of Samples Encoded in every packet in data chunk
  uint16 Block_size;
  ///< Sampling Frequency in Hz
  uint16 Sampling_rate;
  ///< Bits per Sample {after decoding}
  uint16 Sample_size;
  ///< Format Type
  uint32 type;
  /// Give Information on FileType ,Bit Rate (Full Rate/Half Rate etc.)
  qcp_format_type Format_type;
  ///< Codec_guid_codecID1/2/3/4 togeather will decide the codec
  uint32 Codec_guid_codecID1;
  ///< This is the number of rate octates used in packets in data chunk
  uint32 Num_rates;
  ///< Unused data generally zero padded - kept reserved for Future use
  uint32 Unused_data[5];
  ///< Variable Rate Flag under vrat chunk
  uint32 var_rate_flag;
  ///< Total numbers of packets in data chunk
  uint32 size_in_packet;
  ///< Step size under offs flag
  uint32 offs_step_size;
  ///< Number of offsets in offs chunk
  uint32 offs_num_offset;
  ///< Absolute octet offset in this QCP file
  uint32 offs_offset;
  // length of the data chunk
  uint32 data_chunk_length;
};

#endif

