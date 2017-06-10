/**********************************************************************
 * Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved. *
 * Qualcomm Technologies Proprietary and Confidential.                 *
 **********************************************************************/

#ifndef __MODULE_HDR_DBG_H__
#define __MODULE_HDR_DBG_H__

/** MODULE_HDR_NAME:
 *
 * Defines hdr module name
 *
 * Returns hdr module name
 **/
#define MODULE_HDR_NAME "hdr"

/** IDBG_LOG_LEVEL:
 *
 * Specify debug level for current module
 **/
#undef IDBG_LOG_LEVEL
#define IDBG_LOG_LEVEL 2

/** IDBG_LOG_TAG:
 *
 * Specify debug tag for current module
 **/
#undef IDBG_LOG_TAG
#define IDBG_LOG_TAG MODULE_HDR_NAME

#include "camera_dbg.h"
#include "img_dbg.h"

#endif //__MODULE_HDR_DBG_H__
