#ifndef WAV_CONSTANTS_H
#define WAV_CONSTANTS_H


/* =======================================================================
                              wavconstants.h
DESCRIPTION

Copyright 2011 Qualcomm Technologies, Inc., All Rights Reserved

$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/WAVParserLib/main/latest/inc/wavconstants.h#7 $
$DateTime: 2011/11/28 23:18:09 $
$Change: 2065783 $

========================================================================== */


//=============================================================================
// CONSTANTS
//=============================================================================
#define WAV_MIN_BUFFER_SIZE 256

#define WAV_MAX_BUFFER_SIZE 1024

#define WAV_FILE_RIFF_CHUNK_SIZE 12

#define WAV_CHUNK_HEADER_SIZE 8

#define WAV_PREF_BUFF_NUM 4

#define WAV_FIELD_OFFSET 4

#define WAV_PCM_SAMPLE_SIZE 2

#define AUDIO_WAVE 1

#define MILLISECONDS 1000

#define WAV_PCM_WAVFORMATEXTENSIBLE 0xFFFE

// file i/o buffer size
#define WAV_READ_BUFFER_SIZE 1024
#define WAV_CACHE_SIZE (WAV_MAX_BUFFER_SIZE *5)

#endif

