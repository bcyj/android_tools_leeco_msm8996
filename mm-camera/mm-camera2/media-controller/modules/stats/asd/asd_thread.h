/* asd_thread.h
 *
 * Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __ASD_THREAD_H__
#define __ASD_THREAD_H__

#include <pthread.h>
#include <semaphore.h>
#include "asd_module.h"

typedef struct {
  pthread_mutex_t msg_q_lock;
  mct_queue_t     *msg_q;
  boolean         active;

  pthread_cond_t  thread_cond;
  pthread_mutex_t thread_mutex;
  pthread_t       thread_id;
  sem_t           sem_launch;

  mct_port_t        *asd_port;
  asd_callback_func asd_cb;
  asd_object_t      *asd_obj;

  asd_process_asd_type  process;
  asd_process_data_t process_data;
} asd_thread_data_t;

typedef enum {
  MSG_ASD_SET,
  MSG_ASD_GET,
  MSG_AEC_DATA,
  MSG_AWB_DATA,
  MSG_FACE_INFO,
  MSG_ASD_STATS,
  MSG_SOF,
  MSG_STOP_THREAD,
} asd_thread_msg_type_t;


typedef struct {
  asd_thread_msg_type_t type;

  union {
    stats_t *stats;
    asd_set_parameter_t asd_set_parm;
    asd_data_from_aec_t aec_data;
    asd_data_from_awb_t awb_data;
    asd_data_face_info_t face_data;
  }u;
} asd_thread_msg_t;

asd_thread_data_t* asd_thread_init(void);
void asd_thread_deinit(asd_thread_data_t *thread_data);
boolean asd_thread_start(asd_thread_data_t *thread_data);
boolean asd_thread_stop(asd_thread_data_t *asd_data);
boolean asd_thread_en_q_msg(void *asd_data,
  asd_thread_msg_t  *msg);

#endif /* __ASD_THREAD_H__ */
