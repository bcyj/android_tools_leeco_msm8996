/* afd_module.h
 *
 * Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __AFD_MODULE_H__
#define __AFD_MODULE_H__

#include "mct_list.h"
#include "mct_module.h"
#include "mct_pipeline.h"

#include "mct_port.h"
#include "afd.h"

#define AFD_MAX_BG_STATS_NUM  3888  /*72x54, usually we only use 64x48*/
#define AFD_MAX_YUV_STATS_NUM 256   /*16x16*/
#define AFD_MAX_HIST_STATS_NUM 256

#define AFD_OBJECT_CAST(obj) ((afd_module_object_t *)(obj))

#define AFD_LOCK(obj) \
  pthread_mutex_lock(AFD_OBJECT_CAST(obj)->obj_lock)

#define AFD_UNLOCK(obj) \
  pthread_mutex_unlock(AFD_OBJECT_CAST(obj)->obj_lock)

#define AFD_INITIALIZE_LOCK(obj) \
  pthread_mutex_init(&(AFD_OBJECT_CAST(obj)->obj_lock), NULL)

#define AFD_DESTROY_LOCK(obj) \
  pthread_mutex_destroy(AFD_OBJECT_CAST(obj)->obj_lock);

typedef int (* afd_module_set_parameters_func)(afd_set_parameter_t *param,
  void *afd_obj, afd_output_data_t *output);

typedef boolean (* afd_module_get_parameters_func)(afd_get_parameter_t *param,
  void *afd_obj);

typedef boolean    (* afd_module_process_func)(stats_t *stat,
  void *afd_obj, afd_output_data_t *output);

typedef void    (* afd_module_callback_func)(afd_output_data_t *output,
                    void *port);

/**
 *
 **/
typedef struct _afd_object {
  pthread_mutex_t          obj_lock;

  /* typecase to afd_algorithm_internal_control_t */
  void                     *afd;

  afd_module_set_parameters_func  set_parameters;
  afd_module_get_parameters_func  get_parameters;
  afd_module_process_func         process;
  afd_module_callback_func        afd_cb;

  afd_output_data_t        output;

} afd_module_object_t;

mct_module_t* afd_module_init(const char *name);
void afd_module_deinit(mct_module_t *mod);
mct_port_t *afd_module_get_port(mct_module_t *awb_module, unsigned int sessionid);
#endif /* __AFD_MODULE_H__ */
