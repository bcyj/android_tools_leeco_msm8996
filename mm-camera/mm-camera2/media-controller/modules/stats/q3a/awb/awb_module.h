/* awb_module.h
 *
 * Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __AWB_MODULE_H__
#define __AWB_MODULE_H__

#include "mct_list.h"
#include "mct_module.h"
#include "mct_port.h"
#include "stats_module.h"
#include "awb.h"

#if 0
typedef boolean (* awb_set_parameter_func)(awb_set_parameter_t *param,
  void *awb_obj);

typedef boolean (* awb_get_parameters_func)(awb_get_parameter_t *param,
  void *awb_obj);

typedef void    (* awb_process_func)(stats_t *stats,
  void *awb_obj, awb_output_data_t *output);

#endif
typedef void    (* awb_callback_func)(awb_output_data_t *output,
                    void *port);
/**
 *
 **/
typedef struct _awb_object {
  pthread_mutex_t          obj_lock;
  void                     *awb;
  stats_t                  stats;

  awb_output_data_t        output;

  /* q3a_thread_data_t */
  void                    *thread_data;
  awb_ops_t                awb_ops;
} awb_object_t;

#define AWB_OBJECT_CAST(obj) ((awb_object_t *)(obj))

#define AWB_LOCK(obj) \
  pthread_mutex_lock(&(AWB_OBJECT_CAST(obj)->obj_lock))

#define AWB_UNLOCK(obj) \
  pthread_mutex_unlock(AWB_OBJECT_CAST(obj)->obj_lock)

#define AWB_INITIALIZE_LOCK(obj) \
  pthread_mutex_init(&(AWB_OBJECT_CAST(obj)->obj_lock), NULL)

#define AWB_DESTROY_LOCK(obj) \
  pthread_mutex_destroy(&(obj->obj_lock));


mct_module_t* awb_module_init(const char *name);
void awb_module_deinit(mct_module_t *mod);
mct_port_t *awb_module_get_port(mct_module_t *awb_module, unsigned int sessionid);

#endif /* __AWB_MODULE_H__ */
