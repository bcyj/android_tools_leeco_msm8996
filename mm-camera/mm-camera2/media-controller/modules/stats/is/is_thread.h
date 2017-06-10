/* is_thread.h
 *
 * Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __IS_THREAD_H__
#define __IS_THREAD_H__

#include <pthread.h>
#include <semaphore.h>
#include "mct_queue.h"
#include "mct_port.h"
#include "is.h"


/** is_thread_data_t
 *    @msg_q_lock: mutex lock for protecting concurrent access to the message
 *       queue
 *    @msg_q: IS thread's message queue
 *    @active: indicates whether or not IS thread is active
 *    @thread_cond: conditional variable to wake the IS thread
 *    @thread_mutex: mutex to lock the conditional variable - needed for the
 *       signaling mechanism
 *    @thread_id: ID of the IS thread
 *    @sem_launch: semaphore used for thread synchronization
 *    @is_port: IS port
 **/
typedef struct {
  pthread_mutex_t msg_q_lock;
  mct_queue_t     *msg_q;
  boolean         active;
  pthread_cond_t  thread_cond;
  pthread_mutex_t thread_mutex;
  pthread_t       thread_id;
  sem_t           sem_launch;
  mct_port_t      *is_port;
} is_thread_data_t;


/** is_thread_msg_type_t
 **/
typedef enum {
  MSG_IS_SET,
  MSG_IS_PROCESS,
  MSG_IS_STOP_THREAD
} is_thread_msg_type_t;

/** is_thread_msg_t
 *    @type: message type
 *    @is_process_parm: parameter for is_process
 *    @is_set_parm: parameter for is_set
 *
 *  This structure is used for creating messages for the IS thread.
 **/
typedef struct {
  is_thread_msg_type_t type;

  union {
    is_process_parameter_t is_process_parm;
    is_set_parameter_t is_set_parm;
  }u;
} is_thread_msg_t;

boolean is_thread_en_q_msg(is_thread_data_t *thread_data, is_thread_msg_t *msg);
boolean is_thread_start(is_thread_data_t *thread_data);
boolean is_thread_stop(is_thread_data_t *thread_data);
is_thread_data_t* is_thread_init(void);
void is_thread_deinit(is_thread_data_t *thread_data);
#endif /* __IS_THREAD_H__ */
