#ifndef __ACDB_INCLUDES_H__
#define __ACDB_INCLUDES_H__
/*===========================================================================
    @file   acdb_includes.h

    This file contains the necessary interface to the Audio Calibration Database
    (ACDB) module.

    The Audio Calibration Database (ACDB) module provides
    calibration storage for the audio and the voice path. The ACDB
    module is coupled with Qualcomm Audio Calibration Tool (QACT) v2.x.x.
    Qualcomm recommends to always use the latest version of QACT for
    necessary fixes and compatibility.

                    Copyright (c) 2010-2012 Qualcomm Technologies, Inc.
                    All Rights Reserved.
                    Qualcomm Technologies Proprietary and Confidential.
========================================================================== */

/* ---------------------------------------------------------------------------
 * Include Files
 *--------------------------------------------------------------------------- */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

#ifndef ACDB_SIM_ENV
#include <stdbool.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/errno.h>
#include <inttypes.h>
#include <linux/msm_audio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#define TRUE  1
#define FALSE 0
typedef char char_t;
typedef bool bool_t;

#else
#include "mmdefs.h"
#endif

#ifdef _ENABLE_QC_MSG_LOG_
	#ifdef _ANDROID_
		#include <utils/Log.h>
		#include "common_log.h"
		#define DEBUG_PRINT_ERROR LOGE
		#define DEBUG_PRINT LOGI
		#define DEBUG_DETAIL LOGV
	#else
		#define DEBUG_PRINT_ERROR printf
		#define DEBUG_PRINT printf
		#define DEBUG_DETAIL printf
		#define LOGE printf
	#endif // _ANDROID_
#else
	#define DEBUG_PRINT_ERROR
	#define DEBUG_PRINT
	#define DEBUG_DETAIL
	#define LOGE
#endif // _ENABLE_QC_MSG_LOG_

//ACDB function util
#define ACDB_DEBUG_LOG  printf
#define ACDB_MALLOC     malloc
#define ACDB_MEM_FREE   free

//#define ACDB_RTC_DEBUG

//Target Version
#define ACDB_SOFTWARE_VERSION_MAJOR                      0x00000001
#define ACDB_SOFTWARE_VERSION_MINOR                      0x00000008
#define ACDBDATA_CURRENT_DATA_STRUCTURE_VERSION_MAJOR    0x00000000
#define ACDB_TARGET_VERSION_MSM8660_VERSION              0x0001130D

#define ACDB_CURRENT_TARGET_VERSION ACDB_TARGET_VERSION_MSM8660_VERSION

#endif /* __ACDB_INCLUDES_H__ */
