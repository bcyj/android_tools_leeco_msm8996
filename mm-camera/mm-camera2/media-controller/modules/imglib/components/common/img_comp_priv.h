/***************************************************************************
* Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved. *
* Qualcomm Technologies Proprietary and Confidential.                      *
****************************************************************************/

#ifndef __IMG_COMPONENT_PRIV_H__
#define __IMG_COMPONENT_PRIV_H__

#include "img_comp.h"
#include "img_queue.h"

/** IMG_CHK_ABORT
 *   @p: pointer to the image component
 *
 *   Check if the component can be aborted
 **/
#define IMG_CHK_ABORT(p) (p->state == IMG_STATE_STOP_REQUESTED)

/** IMG_CHK_ABORT_RET
 *   @p: pointer to the image component
 *
 *   Check if the component can be aborted and return the
 *   function
 **/
#define IMG_CHK_ABORT_RET(p) ({ \
  if (p->state == IMG_STATE_STOP_REQUESTED) \
    return 0; \
})

/** IMG_CHK_ABORT_RET_LOCKED
 *   @p: pointer to the image component
 *   @m: pointer to the mutex
 *
 *   Check if the component can be aborted with lock. If true
 *   unlock the mutex and return the function execution
 **/
#define IMG_CHK_ABORT_RET_LOCKED(p, m) ({ \
  pthread_mutex_lock(m); \
  if (p->state == IMG_STATE_STOP_REQUESTED) { \
    pthread_mutex_unlock(m); \
    return 0; \
  } \
  pthread_mutex_unlock(m); \
})

/** IMG_CHK_ABORT_UNLK_RET
 *   @p: pointer to the image component
 *   @m: pointer to the mutex
 *
 *   Check if the component can be aborted. If true unlock the
 *   mutex and return the function execution
 **/
#define IMG_CHK_ABORT_UNLK_RET(p, m) ({ \
  if (p->state == IMG_STATE_STOP_REQUESTED) { \
    pthread_mutex_unlock(m); \
    return 0; \
  } \
})

/** IMG_SEND_EVENT
 *   @p_base: pointer to the image component
 *   @evt_type: event type
 *
 *   If the callback is registered, fill the event type and
 *   issue the callback
 **/
#define IMG_SEND_EVENT(p_base, evt_type) ({ \
  img_event_t event; \
  event.type = evt_type; \
  if (p_base->p_cb) \
    p_base->p_cb(p_base->p_userdata, &event); \
  IDBG_MED("%s:%d] send event", __func__, __LINE__); \
})

/** IMG_SEND_EVENT_PYL
 *   @p_base: pointer to the image component
 *   @evt_type: event type
 *   @d_type : data type
 *   @data: pointer to the data
 *
 *   If the callback is registered, fill the event type, add the
 *   payload and issue the callback
 **/
#define IMG_SEND_EVENT_PYL(p_base, evt_type, d_type, data) ({ \
  img_event_t event; \
  event.type = evt_type; \
  event.d.d_type = data; \
  if (p_base->p_cb) \
    p_base->p_cb(p_base->p_userdata, &event); \
})

/** thread_func_t
 *   @p_data: pointer to the data
 *
 *   Thread function
 **/
typedef void *(*thread_func_t) (void *p_data);

/** img_component_t
 *   @inputQ: queue for storing the input buffers
 *   @outBufQ: queue for storing the output buffers
 *   @outputQ: queue for storing the buffers after processed
 *   @metaQ: metadata queue
 *   @mutex : component lock
 *   @cond: conditional variable for the component
 *   @threadid: Execution thread id
 *   @p_userdata: Pointer to the userdata
 *   @thread_loop: Function pointer for the execution main
 *               function
 *   @thread_exit: Flag indicates whether the thread can be
 *               exited
 *   @p_cb: Pointer to the callback function
 *   @error: Variable for storing the last error
 *   @state: State of the component
 *   @is_ready: Flag to indicate whether the flag is ready
 *   @ops: Function table to store the component operations
 *   @frame_info: Frame information for the buffers. Used for
 *              components which supports only one dimension per
 *              session.
 *   @p_core: Pointer to the derived class component
 *   @mode: Executed in sync or Async mode
 *   @img_debug_info: Info to enable additional debug options
 *   @caps: capability of the component
 *   @frame_ops: frame operations
 *
 *   If the callback is registered, fill the event type, add the
 *   payload and issue the callback
 **/
typedef struct {
  img_queue_t inputQ;
  img_queue_t outBufQ;
  img_queue_t outputQ;
  img_queue_t metaQ;
  pthread_mutex_t mutex;
  pthread_cond_t cond;
  pthread_t threadid;
  void *p_userdata;
  thread_func_t thread_loop;
  int thread_exit;
  notify_cb p_cb;
  int error;
  comp_state_t state;
  int is_ready;
  img_component_ops_t ops;
  img_frame_info_t frame_info;
  void *p_core;
  img_comp_mode_t mode;
  img_debug_info_t debug_info;
  img_caps_t caps;
  img_frame_ops_t frame_ops;
} img_component_t;

/** img_comp_create
 *   @p_comp: Pointer to the component
 *
 *   Creates the base component
 **/
int img_comp_create(img_component_t *p_comp);

#endif //__IMG_COMPONENT_PRIV_H__
