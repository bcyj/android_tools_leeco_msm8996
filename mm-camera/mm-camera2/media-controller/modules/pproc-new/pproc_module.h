/*============================================================================

  Copyright (c) 2013,2015 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __PPROC_MODULE_H__
#define __PPROC_MODULE_H__

#include "mct_module.h"

/* macros for unpacking identity */
#define PPROC_GET_STREAM_ID(identity) ((identity) & 0xFFFF)
#define PPROC_GET_SESSION_ID(identity) (((identity) & 0xFFFF0000) >> 16)

// Debug mask
#define PPROC_DEBUG_MASK_PPROC                 (1<<0)

#ifdef CDBG
#undef CDBG
#endif
#define CDBG(fmt, args...) ALOGD_IF(gCamPprocLogLevel >= 2, fmt, ##args)

#ifdef CDBG_LOW
#undef CDBG_LOW
#endif //#ifdef CDBG_LOW
#define CDBG_LOW(fmt, args...) ALOGD_IF(gCamPprocLogLevel >= 3, fmt, ##args)

#ifdef CDBG_HIGH
#undef CDBG_HIGH
#endif //#ifdef CDBG_HIGH
#define CDBG_HIGH(fmt, args...) ALOGD_IF(gCamPprocLogLevel >= 1, fmt, ##args)

extern volatile uint32_t gCamPprocLogLevel;
mct_module_t* pproc_module_get_sub_mod(mct_module_t *module, const char *name);
static void get_pproc_loglevel();

/** pproc_module_util_check_stream
 *    @d1: mct_stream_t* pointer to the stream being checked
 *    @d2: uint32_t* pointer to identity
 *
 *  Check if the stream matches stream index or stream type.
 *
 *  Return: TRUE if stream matches.
 **/
boolean pproc_module_util_check_stream(void *d1, void *d2);

/** pproc_module_util_find_parent
 *    @identity: required identity
 *    @module: module, whichs parents will be serached
 *
 * Finds module parent (stream) with specified identity
 *
 * Returns Pointer to stream handler in case of cucess
 *   or NULL in case of failure
 **/
mct_stream_t* pproc_module_util_find_parent(uint32_t identity,
  mct_module_t* module);

#endif
