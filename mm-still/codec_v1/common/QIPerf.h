/*****************************************************************************
* Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved. *
* Qualcomm Technologies Proprietary and Confidential. *
*****************************************************************************/

#ifndef __QIPERF_H__
#define __QIPERF_H__

#include "dlfcn.h"
#include <stdio.h>

/** img_perf_handle_t
 *   @instance: performance lib instance
 *   @perf_lock_acq: performance lib acquire function
 *   @perf_lock_rel: performance lib release function
 *
 *   Performance Lib Handle
 **/
typedef struct {
  void* instance;
  int32_t (*perf_lock_acq)(unsigned long handle, int32_t duration,
    int32_t list[], int32_t numArgs);
  int32_t (*perf_lock_rel)(unsigned long handle);
} img_perf_handle_t;

/** img_perf_lock_handle_t
 *   @instance: performance lock instance
 *
 *   Performance Lock Handle
 **/
typedef struct {
  int32_t instance;
} img_perf_lock_handle_t;


/** img_perf_lock_handle_create
 *
 * Creates new performance handle
 *
 * Returns new performance handle
 **/
void* img_perf_handle_create();

/** img_perf_handle_destroy
 *    @p_perf: performance handle
 *
 * Destoyes performance handle
 *
 * Returns None.
 **/
void img_perf_handle_destroy(void* p_perf);

/** img_perf_lock_start
 *    @p_perf: performance handle
 *    @p_perf_lock_params: performance lock parameters
 *    @perf_lock_params_size: size of performance lock parameters
 *    @duration: duration
 *
 * Locks performance with specified parameters
 *
 * Returns new performance lock handle
 **/
void* img_perf_lock_start(void* p_perf, int32_t* p_perf_lock_params,
  size_t perf_lock_params_size, int32_t duration);

/** img_perf_lock_end
 *    @p_perf: performance handle
 *    @p_perf_lock: performance lock handle
 *
 * Locks performance with specified parameters
 *
 * Returns None.
 **/
void img_perf_lock_end(void* p_perf, void* p_perf_lock);

#endif /* __QIPERF_H__ */
