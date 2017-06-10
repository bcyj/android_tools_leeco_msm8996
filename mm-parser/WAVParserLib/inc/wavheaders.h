#ifndef WAV_HEADERS_H
#define WAV_HEADERS_H

/* =======================================================================
                              wavheaders.h
DESCRIPTION

Copyright 2009-2013 Qualcomm Technologies, Inc., All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.

$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/WAVParserLib/main/latest/inc/wavheaders.h#9 $
$DateTime: 2013/04/25 02:16:26 $
$Change: 3668452 $

========================================================================== */

/*
* Headers as defined in WAV specification.
* Refer to WAV specification for more information
* about each header and it's individual field.
*/

//wav format type
typedef enum{
  WAV_UNKNOWN
  ,WAV_PCM
  ,WAV_MICROSOFT_ADPCM
  ,WAV_ALAW = 6
  ,WAV_ULAW = 7
  ,WAV_IMA_ADPCM = 17
  ,WAV_YAMAHA = 20
  ,WAV_GSM =49
  ,WAV_G_721 = 64
  ,WAV_G_723 = 20
  ,WAV_MPEG = 80
  ,WAV_MULTICHANNEL_PCM = 0xfffe
}wav_format_type;


/// IMA_ADPCM type
struct  ima_adpcm_data
{
  uint8 format;           ///< format
  uint16 samples_per_block;  ///< samples per block
};

typedef struct
{
  int16 valprev; ///< Previous output value
  int8 index;    ///< Index into stepsize table
}DecoderState;

union format_spec_data
{
  uint8 format;              ///< format
  ima_adpcm_data ima_adpcm;   ///< ima-adpcm specific data
};


/// WAVE Technical Metadata found in file and frame headers
struct tech_data_wav
{
  uint32 type;  ///< first member of the union defines its type
  uint16 channels;        ///< number of channels
  uint32 sample_rate;     ///< sample rate
  uint32 bytes_rate;      ///< bit rate
  uint16 frame_size;      ///< frame size
  uint16 bits_per_sample; ///< bits per sample
  uint16 format;            ///< format
  uint16 block_align;     ///< Block align
  uint32 channel_mask;
  format_spec_data  format_data; ///<format specific data
};

typedef struct wav_header_wavh_t
{
  uint16 nChannels;       ///< number of channels
  uint32 nSampleRate;    ///< sample rate
} wav_header_wavh;

typedef struct wav_audio_info_t
{
  uint32 dwSuggestedBufferSize; // Suggested buffer size for o/p buffer
  uint16 nBlockSize;
} wav_audio_info;

#endif

