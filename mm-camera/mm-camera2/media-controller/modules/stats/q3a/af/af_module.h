/* af_module.h
 *
 * Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#ifndef __AF_MODULE_H__
#define __AF_MODULE_H__

#include "q3a_module.h"
#include "af.h"

#define AF_OBJECT_CAST(obj) ((af_object_t *)(obj))

#define AF_LOCK(obj) \
  pthread_mutex_lock(&(AF_OBJECT_CAST(obj)->obj_lock))

#define AF_UNLOCK(obj) \
  pthread_mutex_unlock(&(AF_OBJECT_CAST(obj)->obj_lock))

#define AF_INITIALIZE_LOCK(obj) \
  pthread_mutex_init(&(AF_OBJECT_CAST(obj)->obj_lock), NULL)

#define AF_DESTROY_LOCK(obj) \
  pthread_mutex_destroy(&(AF_OBJECT_CAST(obj)->obj_lock));

/** _af_object:
 *    @obj_lock:    synchronization mechanism to protect concurrent access
 *                  to the af object
 *    @af:          pointer to the af module
 *    @cb:          the af port's callback function
 *    @output:      the data to be sent upstream
 *    @port:        the private af port data
 *    @af_ops:      structure with pointers to the api library functions
 *    @thread_data: the thread data for the af thread
 *
 * This structure describes the AF object that will be used to handle the
 * AF operations
 **/
typedef struct _af_object {
  pthread_mutex_t  obj_lock;

  void             *af;
  af_callback_func cb;
  af_output_data_t output;
  void             *port;
  af_ops_t         af_ops;
  void             *thread_data;
} af_object_t;

mct_module_t *af_module_init(const char *name);
void       af_module_deinit(mct_module_t *module);
mct_port_t *af_module_get_port(mct_module_t *af_module, unsigned int sessionid);

#endif /* __AF_MODULE_H__ */
