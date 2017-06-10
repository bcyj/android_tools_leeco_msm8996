/**********************************************************************
* Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved. *
* Qualcomm Technologies Proprietary and Confidential.                 *
**********************************************************************/

#ifndef __MODULE_DENOISE_DBG_H__
#define __MODULE_DENOISE_DBG_H__

/** MODULE_DENOISE_NAME:
 *
 * Defines denoise module name
 *
 * Returns denoise module name
 **/
#define MODULE_DENOISE_NAME "denoise"

/** IDBG_LOG_LEVEL:
 *
 * Specify debug level for current module
 **/
#undef IDBG_LOG_LEVEL
#define IDBG_LOG_LEVEL 1

/** IDBG_LOG_TAG:
 *
 * Specify debug tag for current module
 **/
#undef IDBG_LOG_TAG
#define IDBG_LOG_TAG MODULE_DENOISE_NAME

#include "camera_dbg.h"
#include "img_dbg.h"

#endif //__MODULE_DENOISE_DBG_H__
