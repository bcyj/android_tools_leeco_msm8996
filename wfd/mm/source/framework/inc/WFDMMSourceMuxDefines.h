/* =======================================================================
                             WFDMMSourceMuxDefines.h
DESCRIPTION

Header file supporting WFDMMSourceMux.cpp file

Copyright (c) 2011-2012,2014 Qualcomm Technologies, Inc., All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
========================================================================== */

/* =======================================================================
                             PERFORCE HEADER
$Header: //source/qcom/qct/multimedia2/Video/wfd/la/main/latest/framework/inc/WFDMMSourceMuxDefines.h#2 $
$DateTime: 2012/02/10 05:45:30 $
$Changes:$
========================================================================== */

#ifndef _WFD_MM_SOURCE_MUXDEF_H
#define _WFD_MM_SOURCE_MUXDEF_H
/*========================================================================

                     INCLUDE FILES FOR MODULE

==========================================================================*/
#include "OMX_Core.h"
#include "OMX_Video.h"
#include "OMX_Audio.h"
#include "QOMX_FileFormatExtensions.h"

//video related parameters.
typedef enum {
   MUX_VIDEO_NONE,        /* flag indicating no video      */
   MUX_VIDEO_MPEG4,       /* ISO/IEC 14496-2               */
   MUX_VIDEO_H263,        /* H.263                         */
   MUX_VIDEO_H264,        /* H.264                         */
   MUX_VIDEO_INVALID      /* invalid video stream type     */
} MUX_video_type;

typedef struct {
   MUX_video_type                  format;
   OMX_S16                         width;           /* frame width in pixels   */
   OMX_S16                         height;            /* frame height in pixels */
   OMX_S16                         frame_rate;      /* frames per second      */
   OMX_U32                         bitrate;
   OMX_VIDEO_H263PROFILETYPE       eH263Profile;
   OMX_VIDEO_H263LEVELTYPE         eH263Level;
   OMX_VIDEO_MPEG4PROFILETYPE      eMpeg4Profile;
   OMX_VIDEO_MPEG4LEVELTYPE        eMpeg4Level;
   OMX_VIDEO_AVCPROFILETYPE        eH264Profile;
   OMX_VIDEO_AVCLEVELTYPE          eH264Level;

} MUX_video_subtype;

//Audio related parameters.

typedef enum {
   MUX_AUDIO_NONE,           /* flag indicating no audio       */
   MUX_AUDIO_QCELP13K_FULL,  /* PureVoice QCELP-13K fixed full */
   MUX_AUDIO_QCELP13K_HALF,  /* PureVoice QCELP-13K fixed half */
   MUX_AUDIO_EVRC,           /* Enhanced Variable Rate Codec   */
   MUX_AUDIO_EVRC_PV,        /* EVRC, PacketVideo variant      */
   MUX_AUDIO_AMR,            /* GSM Adaptive Multi Rate codec  */
   MUX_AUDIO_MPEG1_L3,       /* MPEG-1, Layer 3 (MP3) codec    */
   MUX_AUDIO_MPEG2_L3,       /* MPEG-2, Layer 3 (MP3) codec    */
   MUX_AUDIO_MPEG4_AAC,      /* MPEG-4 Advanced Audio Codec    */
   MUX_AUDIO_PUREVOICE,      /* 3GPP2 QCELP                    */
   MUX_AUDIO_AMR_WB,         /* AMR Wideband codec (TS 26.171) */
   MUX_AUDIO_AMR_WB_PLUS,    /* Extended AMR WB (TS 26.290)    */
   MUX_AUDIO_EVRC_B,         /* EVRC-B type codec              */
   MUX_AUDIO_EVRC_WB,        /* EVRC-WB type codec             */
   MUX_AUDIO_PCM,            /* Raw audio formats              */
   MUX_AUDIO_AC3,            /* AC3 audio codec                */
   MUX_AUDIO_UNK,            /* Unknown type of audio          */
   MUX_AUDIO_INVALID         /* invalid audio stream type      */
} MUX_audio_type;


typedef struct {
   OMX_AUDIO_AACPROFILETYPE eAACProfile;   /**< AAC profile enumeration */
   OMX_AUDIO_AACSTREAMFORMATTYPE eAACStreamFormat; /**< AAC stream format enumeration */
   OMX_AUDIO_CHANNELMODETYPE eChannelMode;   /**< Channel mode enumeration */
   OMX_S8    sbr_present_flag;
   OMX_S8    ep_config;
   OMX_BOOL  aac_section_data_resilience_flag;
   OMX_BOOL  aac_scalefactor_data_resilience_flag;
   OMX_BOOL  aac_spectral_data_resilience_flag;
} MUX_aac_params_type;

typedef struct {
   MUX_audio_type        format;
   MUX_aac_params_type   aac_params;
   OMX_S32               sampling_frequency;
   OMX_S32               bitrate;
   OMX_S8                num_channels;
   OMX_S16               block_align;
   OMX_S16               bits_per_sample;
} MUX_audio_subtype;


typedef enum {
   MUXER_OUTPUT_FILE,
   MUXER_OUTPUT_STREAMPORT,
   MUXER_OUTPUT_RTP
}MUX_output_type;

typedef struct {
   OMX_S64 ipaddr1;
   OMX_S64 ipaddr2;
   OMX_S32 portno1;
   OMX_S32 rtcpportno1;
   OMX_S32 portno2;
   OMX_S32 rtcpportno2;
   OMX_S8 qos_dscp1;
   OMX_S8 qos_dscp2;
   OMX_U8 bPortTypeUDP;
}MUX_RTPinfo;

typedef union{
   OMX_STRING outfile;
   MUX_RTPinfo rtpInfo;
   void *portHandle;
}Mux_outputInfo;

typedef struct
{
   MUX_output_type outputType;
   Mux_outputInfo outputInfo;
}Mux_Output_object;

/**Mux_Encrypt_Param encryption params */
typedef struct
{
   OMX_BOOL           nStreamEncrypted;  /** stream is encrypted or not */
   QOMX_ENCRYPT_TYPE  nType;  /** type of Encryption */
   OMX_U32            nEncryptVersion; /** Encrypt version */
}Mux_Encrypt_Param;

struct MuxerConfigType
{
   MUX_video_subtype svideoParams;
   MUX_audio_subtype sAudioParams;

   QOMX_CONTAINER_FORMATTYPE eFormat;

   Mux_Output_object sOutputobj;

   Mux_Encrypt_Param sEncryptParam;

   OMX_S32 nAudioBufferCount;
   OMX_S32 nVideoBufferCount;
};

#endif // #ifndef _WFD_MM_SOURCE_MUXDEF_H
