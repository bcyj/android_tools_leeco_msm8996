/* isp_log.h
 *
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */


// use "persist.camera.ISP.debug.mask" to control the debug mask

int ISP_MOD_COM;
#define ISP_DBG(MOD,fmt, args...) if (isp_debug_mask & (1 << MOD)) ALOGE(fmt, ##args)

uint32_t isp_debug_mask;
