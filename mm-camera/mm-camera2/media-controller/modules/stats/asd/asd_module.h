/* asd_module.h
 *
 * Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __ASD_MODULE_H__
#define __ASD_MODULE_H__

#include "mct_list.h"
#include "mct_module.h"
#include "mct_pipeline.h"

#include "mct_port.h"
#include "aec_module.h"
#include "awb_module.h"
#include "asd.h"

#define ASD_MAX_BG_STATS_NUM  3888  /*72x54, usually we only use 64x48*/
#define ASD_MAX_YUV_STATS_NUM 256   /*16x16*/
#define ASD_MAX_HIST_STATS_NUM 256

#define ASD_OBJECT_CAST(obj) ((asd_object_t *)(obj))

#define ASD_LOCK(obj) \
  pthread_mutex_lock(ASD_OBJECT_CAST(obj)->obj_lock)

#define ASD_UNLOCK(obj) \
  pthread_mutex_unlock(ASD_OBJECT_CAST(obj)->obj_lock)

#define ASD_INITIALIZE_LOCK(obj) \
  pthread_mutex_init(&(ASD_OBJECT_CAST(obj)->obj_lock), NULL)

#define ASD_DESTROY_LOCK(obj) \
  pthread_mutex_destroy(&(ASD_OBJECT_CAST(obj)->obj_lock));


/**
 *
 **/
typedef struct _asd_object {
  pthread_mutex_t          obj_lock;

  /* typecase to asd_algorithm_internal_control_t */
  void                     *asd;
  stats_t                  stats;
  asd_output_data_t        output;
  void                    *thread_data;
  asd_ops_t                asd_ops;

} asd_object_t;

mct_module_t* asd_module_init(const char *name);
void asd_module_deinit(mct_module_t *mod);
mct_port_t *asd_module_get_port(mct_module_t *asb_module, unsigned int sessionid);
#endif /* __ASD_MODULE_H__ */
