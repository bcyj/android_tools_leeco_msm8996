/*
**
** Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved
** Qualcomm Technologies Confidential and Proprietary.
**
*/
#ifndef DAP_HW_INFO_H
#define DAP_HW_INFO_H

#include <system/audio.h>

static int snd_card = 0;
static int endpoint = -1;
#define NO_COLS 2
#define CARD_NO 0
#ifdef PLATFORM_MSM8916
#define DEVICE_ID 9
#else
#define DEVICE_ID 3
#endif
#define FLAGS PCM_OUT
static int32_t dmid = 0;
int (*msm_device_id_to_be_id)[NO_COLS];
int device_be_id_map_len;

static int msm_hw_unsupported_device_id[] = {
                  AUDIO_DEVICE_OUT_USB_ACCESSORY,
		  AUDIO_DEVICE_OUT_USB_DEVICE,
		  AUDIO_DEVICE_OUT_BLUETOOTH_A2DP,
		  AUDIO_DEVICE_OUT_BLUETOOTH_A2DP_HEADPHONES,
		  AUDIO_DEVICE_OUT_BLUETOOTH_A2DP_SPEAKER
		  };

#endif
