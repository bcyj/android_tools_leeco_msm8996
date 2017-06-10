#ifndef _OMX_ALSA_UTILS_H_
#define _OMX_ALSA_UTILS_H_

/*=========================================================================
*//** @file omx_alsa_utils.h
This module contains the API to configure ALSA for OMX Test App

Copyright (c) 2012 Qualcomm Technologies, Inc.
All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
*//*=====================================================================*/
/*=========================================================================
                            Edit History

$Header:
when       who     what, where, why
--------   ---     -------------------------------------------------------
=========================================================================*/

#include <alsa_audio.h>
#include <alsa_ucm.h>
#include <hardware/audio.h>
#include "OMX_Core.h"

struct alsa_config_param{
  uint32_t channels;
  uint32_t devid;
  uint32_t volume;
};

int alsa_enable_pcm_session(FILE *logfilefd, int val, struct alsa_config_param *alsaConfigData, struct mixer **mxr, struct pcm **pcmCntxt);
int alsa_set_params(FILE *logfilefd, int channels, int samplingrate, struct pcm *pcmCntxt);
int alsa_framer_update_pcm(FILE *logfilefd, struct pcm *pcmCntxt, OMX_BUFFERHEADERTYPE *pbuff,int *spilledOverBytes,char* tempbuff);
int alsa_init_audiohardware(FILE *logfilefd);
int alsa_deinit_audiohardware(FILE *logfilefd);


#endif // _OMX_ALSA_UTILS_H_
