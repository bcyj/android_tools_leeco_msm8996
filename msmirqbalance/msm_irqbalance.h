/******************************************************************************
 Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 Qualcomm Technologies Proprietary and Confidential.
 ******************************************************************************/

#ifndef MSM_IRQBALANCE_H
#define MSM_IRQBALANCE_H

#ifdef USE_ANDROID_LOG
#define LOG_TAG "MSM-irqbalance"
#include "cutils/log.h"
#ifdef LOGE
#define error(format, ...)   LOGE(format, ## __VA_ARGS__)
#define info(format, ...)   LOGI(format, ## __VA_ARGS__)
#else
#define error(format, ...)   ALOGE(format, ## __VA_ARGS__)
#define info(format, ...)   ALOGI(format, ## __VA_ARGS__)
#endif
#else
#define error(format, ...)   printf(format, ## __VA_ARGS__)
#define info(format, ...)   printf(format, ## __VA_ARGS__)
#endif

#define debug(format, ...) \
	if (debug_enabled) \
		info(format, ## __VA_ARGS__)

#endif
