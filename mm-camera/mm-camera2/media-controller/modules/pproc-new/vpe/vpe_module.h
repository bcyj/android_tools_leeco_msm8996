/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef VPE_MODULE_H
#define VPE_MODULE_H

#include <pthread.h>
#include "modules.h"
#include "mct_queue.h"
#include "mct_list.h"
#include "media_controller.h"
#include "mct_port.h"
#include "mct_object.h"
#include "cam_types.h"
#include "mct_module.h"
#include "mct_pipeline.h"
#include "mct_stream.h"
#include "camera_dbg.h"
#include "vpe_thread.h"
#include "vpe_hardware.h"

#define READ_FD   0
#define WRITE_FD  1

#define VPE_MODULE_MAX_STREAMS   32
#define VPE_MODULE_MAX_SESSIONS  4

#define MODULE_VPE_MIN_NUM_PP_BUFS 2

/* macros for unpacking identity */
#define VPE_GET_STREAM_ID(identity) (identity & 0xFFFF)
#define VPE_GET_SESSION_ID(identity) ((identity & 0xFFFF0000) >> 16)

/* forward declaration of thread data structures */
typedef struct _vpe_thread_msg_t  vpe_thread_msg_t;

typedef enum {
  VPE_DIVERT_UNPROCESSED,
  VPE_DIVERT_PROCESSED
} vpe_divert_type_t;

typedef enum {
  VPE_PRIORITY_REALTIME,
  VPE_PRIORITY_OFFLINE
} vpe_priority_t;

/* aysnchronous events are the event processed by
  vpe_thread from the event_queue */
typedef enum {
  VPE_MODULE_EVENT_PROCESS_BUF,
  VPE_MODULE_EVENT_DIVERT_BUF,
  // currently not used
  VPE_MODULE_EVENT_DOWNSTREAM_ACK
} vpe_module_event_type_t;

typedef struct _vpe_module_event_process_buf_data_t {
  isp_buf_divert_t       isp_buf_divert;
  uint32_t               proc_identity;
  vpe_hardware_params_t  hw_params;
} vpe_module_event_process_buf_data_t;

typedef struct _vpe_module_event_divert_buf_data_t {
  isp_buf_divert_t  isp_buf_divert;
  uint32_t          div_identity;
} vpe_module_event_divert_buf_data_t;

// not used
typedef struct _vpe_module_event_downstream_ack_data_t {
  isp_buf_divert_ack_t  isp_buf_divert_ack;
  uint32_t              identity; //not used
} vpe_module_event_downstream_ack_data_t;

/* key to find the ACK from list */
typedef struct _vpe_module_ack_key_t {
  uint32_t identity;
  int buf_idx;
} vpe_module_ack_key_t;

/* data structure for events stored in vpe queue */
typedef struct _vpe_module_async_event_t {
  vpe_module_event_type_t type;
  /* invalid bit used for stream-off handling */
  boolean invalid;
  /* indicates if vpe hw processing is needed for this event */
  boolean hw_process_flag;
  /* key to find corresponding ack */
  vpe_module_ack_key_t ack_key;

  /* event specific data based on type */
  union {
    vpe_module_event_process_buf_data_t     process_buf_data;
    vpe_module_event_divert_buf_data_t      divert_buf_data;
  } u;
} vpe_module_event_t;

/* data structure to store the ACK in list */
typedef struct _vpe_module_ack_t {
  isp_buf_divert_ack_t isp_buf_divert_ack;
  int32_t ref_count;
  int32_t frame_id;
  struct timeval in_time, out_time;
} vpe_module_ack_t;

/* queue for handling async events */
typedef struct _vpe_module_event_queue_t {
  mct_queue_t       *q;
  pthread_mutex_t   mutex;
} vpe_module_event_queue_t;

/* list for storing ACKs */
typedef struct _vpe_module_ack_list_t {
  mct_list_t        *list;
  uint32_t           size;
  pthread_mutex_t    mutex;
} vpe_module_ack_list_t;

typedef struct _vpe_module_stream_params_t vpe_module_stream_params_t;

/* stream specific parameters */
typedef struct _vpe_module_stream_params_t {
  vpe_hardware_params_t        hw_params;
  uint32_t                     identity;
  pproc_divert_info_t          div_info;
  vpe_priority_t               priority;
  cam_stream_type_t            stream_type;
  boolean                      is_stream_on;
  uint32_t                     unproc_div_identity;
  uint32_t                     proc_div_identity;
  pthread_mutex_t              mutex;
  boolean                      hfr_skip_required;
  int32_t                      hfr_skip_count;
  uint32_t                     frame_offset;
  /* this pointer is used to access a stream which is
     linked and can receive buffers for this stream.
     This is used when ISP send buffer on identity which
     is not on */
  vpe_module_stream_params_t  *linked_stream_params;
} vpe_module_stream_params_t;

/* session specific parameters */
typedef struct _vpe_module_session_params_t {
  vpe_module_stream_params_t   *stream_params[VPE_MODULE_MAX_STREAMS];
  int32_t                       stream_count;
  /* vpe_hardware_params_t         hw_params; */
  uint32_t                      session_id;
  cam_hfr_mode_t                hfr_mode;
} vpe_module_session_params_t;

typedef struct _vpe_module_buffer_info {
  int fd;
  uint32_t index;
  uint32_t offset;
  uint8_t native_buff;
  uint8_t processed_divert;
} vpe_module_buffer_info_t;

typedef struct _vpe_module_stream_buff_info {
  uint32_t identity;
  uint32_t num_buffs;
  mct_list_t *buff_list;
} vpe_module_stream_buff_info_t;

typedef struct _vpe_module_ctrl_t {
  mct_module_t                *p_module;
  vpe_module_event_queue_t    realtime_queue;
  vpe_module_event_queue_t    offline_queue;
  vpe_module_ack_list_t       ack_list;
  pthread_t                   vpe_thread;
  pthread_cond_t              th_start_cond;
  boolean                     vpe_thread_started;
  pthread_mutex_t             vpe_mutex;
  int                         pfd[2];
  int32_t                     session_count;
  vpe_hardware_t              *vpehw;
  vpe_module_session_params_t *session_params[VPE_MODULE_MAX_SESSIONS];
} vpe_module_ctrl_t;

mct_module_t* vpe_module_init(const char *name);
void vpe_module_deinit(mct_module_t *mod);
static void vpe_module_set_mod (mct_module_t *module, unsigned int module_type,
  unsigned int identity);
static boolean vpe_module_query_mod(mct_module_t *module, void *query_buf,
  unsigned int sessionid);
static boolean vpe_module_start_session(mct_module_t *module,
  unsigned int sessionid);
static boolean vpe_module_stop_session(mct_module_t *module,
  unsigned int sessionid);
static vpe_module_ctrl_t* vpe_module_create_vpe_ctrl(void);
static int32_t vpe_module_destroy_vpe_ctrl(vpe_module_ctrl_t *ctrl);

int32_t vpe_module_post_msg_to_thread(mct_module_t *module,
  vpe_thread_msg_t msg);

static int32_t vpe_module_send_ack_event_upstream(vpe_module_ctrl_t *ctrl,
  isp_buf_divert_ack_t isp_ack);

int32_t vpe_module_process_downstream_event(mct_module_t* module,
  mct_event_t *event);
int32_t vpe_module_process_upstream_event(mct_module_t* module,
  mct_event_t *event);
int32_t vpe_module_send_event_downstream(mct_module_t* module,
   mct_event_t* event);
int32_t vpe_module_send_event_upstream(mct_module_t* module,
   mct_event_t* event);
int32_t vpe_module_notify_add_stream(mct_module_t* module, mct_port_t* port,
  mct_stream_info_t* stream_info);
int32_t vpe_module_notify_remove_stream(mct_module_t* module,
  uint32_t identity);
int32_t vpe_module_do_ack(vpe_module_ctrl_t *ctrl,
  vpe_module_ack_key_t key);
int32_t vpe_module_invalidate_queue(vpe_module_ctrl_t* ctrl,
  uint32_t identity);
int32_t vpe_module_put_new_ack_in_list(vpe_module_ctrl_t *ctrl,
  vpe_module_ack_key_t key, int32_t buf_dirty, int32_t ref_count);
int32_t vpe_module_enq_event(mct_module_t* module,
  vpe_module_event_t* event, vpe_priority_t prio);

/* ----------------------- utility functions -------------------------------*/

mct_port_t* vpe_module_find_port_with_identity(mct_module_t *module,
  mct_port_direction_t dir, uint32_t identity);
vpe_module_ack_t* vpe_module_find_ack_from_list(vpe_module_ctrl_t *ctrl,
  vpe_module_ack_key_t key);
cam_streaming_mode_t vpe_module_get_streaming_mode(mct_module_t *module,
  uint32_t identity);
int32_t vpe_module_get_params_for_identity(vpe_module_ctrl_t* ctrl,
  uint32_t identity, vpe_module_session_params_t** session_params,
  vpe_module_stream_params_t** stream_params);
void vpe_module_dump_stream_params(vpe_module_stream_params_t* stream_params,
  const char* func, int line);
boolean vpe_module_util_map_buffer_info(void *d1, void *d2);
boolean vpe_module_util_free_buffer_info(void *d1, void *d2);
boolean vpe_module_util_create_hw_stream_buff(void *d1, void *d2);
boolean vpe_module_invalidate_q_traverse_func(void* qdata, void* input);


/* vpe_decide_hfr_skip:
 *
 * Decides if a frame needs to be skipped. if @frame_id is 0, its not skipped.
 * @count number of frames are skipped after the 0th frame.
 * The pattern repeats.
 *
 **/
#define vpe_decide_hfr_skip(frame_id, count) \
  ((((frame_id) % ((count)+1)) == 0) ? FALSE : TRUE)

int32_t vpe_module_handle_buf_divert_event(mct_module_t* module,
  mct_event_t* event);

int32_t vpe_module_handle_isp_out_dim_event(mct_module_t* module,
  mct_event_t* event);

int32_t vpe_module_handle_stream_crop_event(mct_module_t* module,
  mct_event_t* event);

int32_t vpe_module_handle_dis_update_event(mct_module_t* module,
  mct_event_t* event);

int32_t vpe_module_handle_set_parm_event(mct_module_t* module,
  mct_event_t* event);

int32_t vpe_module_handle_streamon_event(mct_module_t* module,
  mct_event_t* event);

int32_t vpe_module_handle_streamoff_event(mct_module_t* module,
  mct_event_t* event);

int32_t vpe_module_handle_stream_cfg_event(mct_module_t* module,
  mct_event_t* event);

int32_t vpe_module_handle_div_info_event(mct_module_t* module,
  mct_event_t* event);

/* -------------------------------------------------------------------------*/

#endif
