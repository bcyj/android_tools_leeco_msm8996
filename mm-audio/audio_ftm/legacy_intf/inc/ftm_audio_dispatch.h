#ifndef FTM_AUDIO_DISPATCH_H
#define FTM_AUDIO_DISPATCH_H

/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

                      F T M    A U D I O    D I S P A T C H

GENERAL DESCRIPTION
  This is the header file for the embedded FTM Audio Commands

*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/*===========================================================================
Copyright (c) 2011 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.

                        EDIT HISTORY FOR MODULE

  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.

$Header: //source/qcom/qct/multimedia2/Audio/drivers/ftm/8x60/linux/rel/1.0/inc/ftm_audio_dispatch.h#1 $ $DateTime: 2011/04/05 20:05:46 $ $Author: zhongl $

when       who    what, where, why
--------   ---    ----------------------------------------------------------
===========================================================================*/

#include "DALSYS_common.h"
#include "diagpkt.h"

/*===========================================================================

                              DEFINITIONS

===========================================================================*/

typedef struct
{
  diagpkt_cmd_code_type         cmd_code;
  diagpkt_subsys_id_type        subsys_id;
  diagpkt_subsys_cmd_code_type  subsys_cmd_code;
}__attribute__((packed)) ftm_audio_diagpkt_subsys_header_type;

typedef struct
{
  uint16 cmd_id;            /* command id (required) */
  uint16 cmd_data_len;      /* request pkt data length, excluding the diag and ftm headers
                             (optional, set to 0 if not used)
                          */
  uint16 cmd_rsp_pkt_size;  /* rsp pkt size, size of response pkt if different then req pkt
                             (optional, set to 0 if not used)
                          */
}__attribute__((packed)) ftm_audio_cmd_header_type;

typedef struct
{
  ftm_audio_diagpkt_subsys_header_type  diag_hdr;
  ftm_audio_cmd_header_type             ftm_hdr;
}__attribute__((packed)) ftm_audio_composite_cmd_header_type;


#define PCM_LOOPBACK_ON             0xA9
#define PCM_LOOPBACK_OFF            0xA8

#define FTM_SND_DEVICE_NOT_PRESENT  99
#define FTM_SOUND_DEVICE_ARRAY_SIZE 32

#define LOWEST_FREQUENCY            50

typedef enum
{
  FTM_AUDIO_COMMAND_SUCCESS,
  FTM_AUDIO_COMMAND_FAILURE,
  FTM_AUDIO_DEVICE_NOT_PRESENT,
  FTM_AUDIO_PCM_BUFFER_EXCEEDED
} ftm_audio_rsp_status_type;

typedef enum
{
  FTM_AUDIO_SET_PATH,
  FTM_AUDIO_SET_VOLUME,
  FTM_AUDIO_DSP_LOOPBACK,
  FTM_AUDIO_PCM_LOOPBACK,
  FTM_AUDIO_TONES_PLAY,
  FTM_AUDIO_TONES_STOP,
  FTM_AUDIO_NS_CONTROL,
  FTM_AUDIO_PCM_ENABLE,
  FTM_AUDIO_GET_PCM_SAMPLES,
  FTM_AUDIO_PCM_DISABLE,
  FTM_MIC_GAIN_OFFSET,
  FTM_AUDIO_MP3_PLAY
} ftm_audio_subcmd_type;

typedef struct
{
  uint16                  hi_freq;
  uint16                  lo_freq;
  uint16                  snd_method;
}__attribute__((packed)) ftm_audio_tone_type;

typedef struct
{
  dword                   data_offset;
  word                    data_size;
}__attribute__((packed)) ftm_audio_pcm_req_type;

typedef union
{
  boolean                 on_off;
  uint16                  device;
  uint8                   volume;
  uint16                  num_pcm_buffers;
  sint15                  mic_gain_offset;
  ftm_audio_tone_type     tone_params;
  ftm_audio_pcm_req_type  pcm_req;
}__attribute__((packed)) ftm_audio_params;

typedef struct
{
  ftm_audio_composite_cmd_header_type composite_header;
  ftm_audio_params              audio_params;
}__attribute__((packed)) ftm_audio_pkt_type;

typedef struct
{
    ftm_audio_diagpkt_subsys_header_type  header ;
    char                        result ;
}__attribute__((packed)) ftm_audio_response_type;

typedef struct
{
  ftm_audio_composite_cmd_header_type header;
  dword                           data_offset;
  word                            data_size;
}__attribute__((packed)) ftm_audio_get_pcm_req_type;

typedef struct
{
  ftm_audio_composite_cmd_header_type    header ;
  char                          result ;
  dword                         data_offset;
  word                          data_size;
  uint8                         pcm_data[1];
}__attribute__((packed)) ftm_audio_get_pcm_rsp_type;


#endif /* FTM_AUDIO_DISPATCH_H */

