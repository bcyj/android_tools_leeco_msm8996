/* aec_module.h
 *
 * Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __AEC_MODULE_H__
#define __AEC_MODULE_H__

#include "q3a_module.h"
#include "aec.h"

#define AEC_MAX_BG_STATS_NUM    3888 /* 72x54, usually we only use 64x48 */
#define AEC_MAX_YUV_STATS_NUM   256  /* 16x16 */
#define AEC_MAX_HIST_STATS_NUM  256

#define AEC_OBJECT_CAST(obj) ((aec_object_t *)(obj))

#define AEC_LOCK(obj) \
  pthread_mutex_lock(AEC_OBJECT_CAST(obj)->obj_lock)

#define AEC_UNLOCK(obj) \
  pthread_mutex_unlock(AEC_OBJECT_CAST(obj)->obj_lock)

#define AEC_INITIALIZE_LOCK(obj) \
  pthread_mutex_init(&(AEC_OBJECT_CAST(obj)->obj_lock), NULL)

#define AEC_DESTROY_LOCK(obj) \
  pthread_mutex_destroy(&(AEC_OBJECT_CAST(obj)->obj_lock));

mct_module_t *aec_module_init(const char *name);
void         aec_module_deinit(mct_module_t *module);
mct_port_t   *aec_module_get_port(mct_module_t *aec_module,
  unsigned int sessionid);

#endif /* __AEC_MODULE_H__ */
