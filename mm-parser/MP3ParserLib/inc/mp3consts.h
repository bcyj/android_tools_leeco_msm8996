// -*- Mode: C++ -*-
//=============================================================================
// FILE: mp3consts.h
//
// SERVICES: Audio
//
// DESCRIPTION:
/// @file: mp3consts.h
/// Declaration of constants required fro MP3 Format Parser Implementation
///
/// Copyright (c) 2009-2013 Qualcomm Technologies, Inc.
/// All Rights Reserved.
/// Qualcomm Technologies Proprietary and Confidential.
//=============================================================================

//=============================================================================
//                      EDIT HISTORY FOR FILE
//
//  This section contains comments describing changes made to this file.
//  Notice that changes are listed in reverse chronological order.
//
//$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/MP3ParserLib/main/latest/inc/mp3consts.h#9 $
//$DateTime: 2013/08/24 08:24:59 $
//$Change: 4329860 $
//
//when            what, where, why
//--------   ---     ----------------------------------------------------------
//01/14/09      Created the file

//=============================================================================

#ifndef _MP3CONSTS_H_
#define _MP3CONSTS_H_

//=============================================================================
// INCLUDES
//=============================================================================


//=============================================================================
// CONSTANTS
//=============================================================================
// Count of frames parsed in parse file header to check if requested
// MP3 format is supported or not
#define MP3_PARSE_FRAME_COUNT 3

#define MP3_FIND_SYNC_SIZE 9

// max size of data read from IFilePort1 interface in parse_file_header API
#define MAX_READ_LENGTH 2000

#define MP3_FILE_HEADER_SIZE 4

//Position of the LAME tags(Xing or Info)
#define MP3_MONO_XING_TAG_POS   (21)
#define MP3_STEREO_XING_TAG_POS (36)
#define LAME_TAG_POS_FOR_MONO   (141)
#define LAME_TAG_POS_FOR_STEREO (156)
/* LAME tag contains other metadata fields.
   Encoder delay parameter will be available at offset # 21.
   LAME structure:
   Encoder short version string                   9bytes  0
   Info tag version[4MSB] & VBR method[4LSB]      1       9
   Low Pass Filter Value                          1       10
   Replay Gain                                    8       11
   Encoding Flag [4MSB] & ATH Type[4LSB]          1       19
   If an ABR file{specified bitrate}              1       20
      else{minimal bitrate}
   Encoder delays                                 3       21
   Misc                                           1       24
   MP3 gain                                       1       25
   Preset and surround information                2       26
   Music Length                                   4       28
   Music CRC                                      2       30
   CRC-16 of Info Tag                             2       32

*/
#define MP3_ENCODER_DELAY_OFFSET    (21)

// file i/o buffer size
#define MP3_READ_BUFFER_SIZE 1024

#define MP3_SAMPLE_RATE 4000

#define MP3_ACM_MPEG_LAYER1             (0x0001)
#define MP3_ACM_MPEG_LAYER2             (0x0002)
#define MP3_ACM_MPEG_LAYER3             (0x0004)
#define MP3_ACM_MPEG_STEREO             (0x0001)
#define MP3_ACM_MPEG_JOINTSTEREO        (0x0002)
#define MP3_ACM_MPEG_DUALCHANNEL        (0x0004)
#define MP3_ACM_MPEG_SINGLECHANNEL      (0x0008)
#define MP3_ACM_MPEG_PRIVATEBIT         (0x0001)
#define MP3_ACM_MPEG_COPYRIGHT          (0x0002)
#define MP3_ACM_MPEG_ORIGINALHOME       (0x0004)
#define MP3_ACM_MPEG_PROTECTIONBIT      (0x0008)
#define MP3_ACM_MPEG_ID_MPEG1           (0x0010)

#endif // _MP3CONSTS_H_
