/*****************************************************************************
* Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved. *
* Qualcomm Technologies Proprietary and Confidential. *
*****************************************************************************/

#include "QIPerf.h"
#include "dlfcn.h"
#include "jpeglog.h"

/** img_perf_lock_handle_create
 *
 * Creates new performance handle
 *
 * Returns new performance handle
 **/
void* img_perf_handle_create()
{
  img_perf_handle_t *perf_handle;
  char qcopt_lib_path[PATH_MAX] = {0};

  perf_handle = calloc(1, sizeof(img_perf_handle_t));
  if (!perf_handle) {
    QIDBG_ERROR("%s:%d Not enough memory\n", __func__, __LINE__);
    return NULL;
  }

  if (!property_get("ro.vendor.extension_library", qcopt_lib_path, NULL)) {
    QIDBG_ERROR("%s:%d Cannot get performance lib name\n", __func__, __LINE__);
    free(perf_handle);
    return NULL;
  }

  dlerror();
  perf_handle->instance = dlopen(qcopt_lib_path, RTLD_NOW);
  if (!perf_handle->instance) {
    QIDBG_ERROR("%s:%d Unable to open %s: %s\n", __func__, __LINE__,
      qcopt_lib_path, dlerror());
    free(perf_handle);
    return NULL;
  }

  perf_handle->perf_lock_acq = dlsym(perf_handle->instance, "perf_lock_acq");

  if (!perf_handle->perf_lock_acq) {
    QIDBG_ERROR("%s:%d Unable to get perf_lock_acq function handle\n", __func__,
      __LINE__);
    if (dlclose(perf_handle->instance)) {
      QIDBG_ERROR("%s:%d Error occurred while closing qc-opt library\n",
        __func__, __LINE__);
    }
    free(perf_handle);
    return NULL;
  }

  perf_handle->perf_lock_rel = dlsym(perf_handle->instance, "perf_lock_rel");

  if (!perf_handle->perf_lock_rel) {
    QIDBG_ERROR("%s:%d Unable to get perf_lock_rel function handle\n", __func__,
      __LINE__);
    if (dlclose(perf_handle->instance)) {
      QIDBG_ERROR("%s:%d Error occurred while closing qc-opt library\n",
        __func__, __LINE__);
    }
    free(perf_handle);
    return NULL;
  }

  return perf_handle;
}

/** img_perf_handle_destroy
 *    @p_perf: performance handle
 *
 * Destoyes performance handle
 *
 * Returns None.
 **/
void img_perf_handle_destroy(void* p_perf)
{
  img_perf_handle_t *perf_handle = (img_perf_handle_t*)p_perf;

  if (!perf_handle || !perf_handle->instance) {
    QIDBG_ERROR("%s:%d Null pointer detected perf_handle %p instance %p\n",
      __func__, __LINE__, perf_handle, perf_handle->instance);
    return;
  }

  if (dlclose(perf_handle->instance)) {
    QIDBG_ERROR("%s:%d Error occurred while closing qc-opt library\n",
      __func__, __LINE__);
  }

  free(perf_handle);
}

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
  size_t perf_lock_params_size, int32_t duration)
{
  img_perf_handle_t *perf_handle = (img_perf_handle_t*)p_perf;
  img_perf_lock_handle_t *lock_handle;

  if (!perf_handle || !p_perf_lock_params || !perf_handle->perf_lock_acq) {
    QIDBG_ERROR("%s:%d Null pointer detected perf_handle %p lock_params %p\n",
      __func__, __LINE__, perf_handle, p_perf_lock_params);
    return NULL;
  }

  lock_handle = calloc(1, sizeof(img_perf_lock_handle_t));
  if (!lock_handle) {
    QIDBG_ERROR("%s:%d Not enough memory\n", __func__, __LINE__);
    return NULL;
  }

  lock_handle->instance = perf_handle->perf_lock_acq(lock_handle->instance,
    duration, p_perf_lock_params, perf_lock_params_size);

  return lock_handle;
}

/** img_perf_lock_end
 *    @p_perf: performance handle
 *    @p_perf_lock: performance lock handle
 *
 * Locks performance with specified parameters
 *
 * Returns None.
 **/
void img_perf_lock_end(void* p_perf, void* p_perf_lock)
{
  img_perf_handle_t *perf_handle = (img_perf_handle_t*)p_perf;
  img_perf_lock_handle_t *lock_handle = (img_perf_lock_handle_t*)p_perf_lock;

  if (!perf_handle || !lock_handle || !perf_handle->perf_lock_rel) {
    QIDBG_ERROR("%s:%d Null pointer detected perf_handle %p lock_handle %p\n",
      __func__, __LINE__, perf_handle, lock_handle);
    return;
  }

  perf_handle->perf_lock_rel(lock_handle->instance);

  free(lock_handle);
}
