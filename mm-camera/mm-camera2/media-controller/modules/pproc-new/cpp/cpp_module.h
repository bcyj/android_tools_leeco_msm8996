/*============================================================================

  Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef CPP_MODULE_H
#define CPP_MODULE_H

#include <pthread.h>
#include <math.h>
#include "modules.h"
#include "chromatix.h"
#include "chromatix_common.h"
#include "mct_queue.h"
#include "mct_list.h"
#include "media_controller.h"
#include "mct_port.h"
#include "mct_object.h"
#include "cam_types.h"
#include "cam_intf.h"
#include "mct_module.h"
#include "mct_pipeline.h"
#include "mct_stream.h"
#include "camera_dbg.h"
#include "cpp_port.h"
#include "cpp_thread.h"
#include "cpp_hardware.h"

#define READ_FD   0
#define WRITE_FD  1

#define CPP_MODULE_MAX_STREAMS      16
#define CPP_MODULE_MAX_SESSIONS     4

#define CPP_OUTPUT_DUPLICATION_EN   1

#define MODULE_CPP_MIN_NUM_PP_BUFS  1

/* macros for unpacking identity */
#define CPP_GET_STREAM_ID(identity) ((identity) & 0xFFFF)
#define CPP_GET_SESSION_ID(identity) (((identity) & 0xFFFF0000) >> 16)

// Debug mask
#define PPROC_DEBUG_MASK_CPP                 (1<<2)

/* forward declaration of thread data structures */
typedef struct _cpp_thread_msg_t  cpp_thread_msg_t;
extern volatile uint32_t gCamCppLogLevel;

typedef enum {
  CPP_DIVERT_UNPROCESSED,
  CPP_DIVERT_PROCESSED
} cpp_divert_type_t;

typedef enum {
  CPP_PRIORITY_REALTIME,
  CPP_PRIORITY_OFFLINE
} cpp_priority_t;

/* aysnchronous events are the event processed by
  cpp_thread from the event_queue */
typedef enum {
  CPP_MODULE_EVENT_PROCESS_BUF,
  CPP_MODULE_EVENT_DIVERT_BUF,
} cpp_module_event_type_t;

typedef struct _cpp_module_event_process_buf_data_t {
  isp_buf_divert_t       isp_buf_divert;
  uint32_t               proc_identity;
  boolean                proc_div_required;
  uint32_t               proc_div_identity;
  cpp_hardware_params_t  hw_params;
} cpp_module_event_process_buf_data_t;

typedef struct _cpp_module_event_divert_buf_data_t {
  isp_buf_divert_t  isp_buf_divert;
  uint32_t          div_identity;
} cpp_module_event_divert_buf_data_t;

/* key to find the ACK from list */
typedef struct _cpp_module_ack_key_t {
  uint32_t identity;
  int buf_idx;
  int channel_id;
  int frame_id;
  void *meta_data;
} cpp_module_ack_key_t;

typedef struct _cpp_module_hw_cookie_t {
  cpp_module_ack_key_t  key;
  boolean               proc_div_required;
  uint32_t              proc_div_identity;
  void                 *meta_data;
} cpp_module_hw_cookie_t;

/* data structure for events stored in cpp queue */
typedef struct _cpp_module_async_event_t {
  cpp_module_event_type_t type;
  /* invalid bit used for stream-off handling */
  boolean invalid;
  /* indicates if cpp hw processing is needed for this event */
  boolean hw_process_flag;
  /* key to find corresponding ack */
  cpp_module_ack_key_t ack_key;

  /* event specific data based on type */
  union {
    cpp_module_event_process_buf_data_t     process_buf_data;
    cpp_module_event_divert_buf_data_t      divert_buf_data;
  } u;
} cpp_module_event_t;

/* data structure to store the ACK in list */
typedef struct _cpp_module_ack_t {
  isp_buf_divert_ack_t isp_buf_divert_ack;
  int32_t ref_count;
  int32_t frame_id;
  struct timeval in_time, out_time;
} cpp_module_ack_t;

/* queue for handling async events */
typedef struct _cpp_module_event_queue_t {
  mct_queue_t       *q;
  pthread_mutex_t   mutex;
} cpp_module_event_queue_t;

/* list for storing ACKs */
typedef struct _cpp_module_ack_list_t {
  mct_list_t        *list;
  uint32_t           size;
  pthread_mutex_t    mutex;
} cpp_module_ack_list_t;

/* Information about frame skipping for HFR */
typedef struct _cpp_module_hfr_skip_info_t {
  boolean      skip_required;
  int32_t      skip_count;
  uint32_t     frame_offset;
  float        output_fps;
  float        input_fps;
} cpp_module_hfr_skip_info_t;

typedef struct _cpp_module_frame_hold_t {
  /* frame hold flag, if flag is TRUE, then isp_buf holds frame */
  boolean          is_frame_hold;
  /* Store frame until DIS event for this frame is received */
  unsigned int     identity;
  isp_buf_divert_t isp_buf;
} cpp_module_frame_hold_t;

typedef struct _cpp_module_dis_hold_t {
  /* DIS hold flag, if is_valid is TRUE, then dis_frame_id has valid frame id */
  boolean      is_valid;
  /* Store frame until DIS event for this frame is received */
  unsigned int dis_frame_id;
} cpp_module_dis_hold_t;

typedef struct _cpp_module_stream_params_t cpp_module_stream_params_t;

/* stream specific parameters */
typedef struct _cpp_module_stream_params_t {
  cpp_hardware_params_t        hw_params;
  uint32_t                     identity;
  cpp_divert_info_t           *div_config;
  cpp_priority_t               priority;
  cam_stream_type_t            stream_type;
  boolean                      is_stream_on;
  pthread_mutex_t              mutex;
  cpp_module_hfr_skip_info_t   hfr_skip_info;
  modulesChromatix_t          module_chromatix;
  /* linked stream is the stream which shares same ISP input buffer.
     This is usually mapped on same port */
  cpp_module_stream_params_t  *linked_stream_params;
} cpp_module_stream_params_t;

/* session specific parameters */
typedef struct _cpp_module_session_params_t {
  cpp_module_stream_params_t   *stream_params[CPP_MODULE_MAX_STREAMS];
  int32_t                       stream_count;
  cpp_hardware_params_t         hw_params;
  uint32_t                      session_id;
  cam_hfr_mode_t                hfr_mode;
  cpp_params_aec_trigger_info_t aec_trigger;
  /* DIS enable flag to be used for frame hold */
  int32_t                       dis_enable;
  /* Latest frame id received from DIS crop event */
  cpp_module_dis_hold_t         dis_hold;
  /* Hold frame until DIS crop is received for this frame */
  cpp_module_frame_hold_t       frame_hold;
  ez_pp_params_t                diag_params;
  cam_fps_range_t               fps_range;
  pthread_mutex_t               dis_mutex;
  mct_sensor_format_t           sensor_format;
} cpp_module_session_params_t;

typedef struct _cpp_module_buffer_info {
  int fd;
  uint32_t index;
  uint32_t offset;
  uint8_t native_buff;
  uint8_t processed_divert;
} cpp_module_buffer_info_t;

typedef struct _cpp_module_stream_buff_info {
  uint32_t identity;
  uint32_t num_buffs;
  mct_list_t *buff_list;
} cpp_module_stream_buff_info_t;

/* list for storing clock_rates of different streams */
typedef struct _cpp_module_clk_rate_list_t {
  mct_list_t        *list;
  uint32_t           size;
  pthread_mutex_t    mutex;
} cpp_module_clk_rate_list_t;

typedef struct _cpp_module_stream_clk_rate_t {
  uint32_t identity;
  uint64_t total_load;
} cpp_module_stream_clk_rate_t;


typedef struct _cpp_module_ctrl_t {
  mct_module_t                *p_module;
  cpp_module_event_queue_t    realtime_queue;
  cpp_module_event_queue_t    offline_queue;
  cpp_module_ack_list_t       ack_list;
  pthread_t                   cpp_thread;
  pthread_cond_t              th_start_cond;
  boolean                     cpp_thread_started;
  pthread_mutex_t             cpp_mutex;
  int                         pfd[2];
  int32_t                     session_count;
  cpp_hardware_t              *cpphw;
  cpp_module_clk_rate_list_t  clk_rate_list;
  unsigned long               clk_rate;
  cpp_module_session_params_t *session_params[CPP_MODULE_MAX_SESSIONS];
} cpp_module_ctrl_t;

mct_module_t* cpp_module_init(const char *name);
void cpp_module_deinit(mct_module_t *mod);
static void cpp_module_set_mod (mct_module_t *module, unsigned int module_type,
  unsigned int identity);
static boolean cpp_module_query_mod(mct_module_t *module, void *query_buf,
  unsigned int sessionid);
static boolean cpp_module_start_session(mct_module_t *module,
  unsigned int sessionid);
static boolean cpp_module_stop_session(mct_module_t *module,
  unsigned int sessionid);
static cpp_module_ctrl_t* cpp_module_create_cpp_ctrl(void);
static int32_t cpp_module_destroy_cpp_ctrl(cpp_module_ctrl_t *ctrl);

static int32_t cpp_module_send_ack_event_upstream(cpp_module_ctrl_t *ctrl,
  isp_buf_divert_ack_t isp_ack);

int32_t cpp_module_post_msg_to_thread(mct_module_t *module,
  cpp_thread_msg_t msg);

int32_t cpp_module_process_downstream_event(mct_module_t* module,
  mct_event_t *event);
int32_t cpp_module_process_upstream_event(mct_module_t* module,
  mct_event_t *event);
int32_t cpp_module_send_event_downstream(mct_module_t* module,
   mct_event_t* event);
int32_t cpp_module_send_event_upstream(mct_module_t* module,
   mct_event_t* event);
int32_t cpp_module_notify_add_stream(mct_module_t* module, mct_port_t* port,
  mct_stream_info_t* stream_info);
int32_t cpp_module_notify_remove_stream(mct_module_t* module,
  uint32_t identity);
int32_t cpp_module_do_ack(cpp_module_ctrl_t *ctrl,
  cpp_module_ack_key_t key);
int32_t cpp_module_invalidate_queue(cpp_module_ctrl_t* ctrl,
  uint32_t identity);
int32_t cpp_module_put_new_ack_in_list(cpp_module_ctrl_t *ctrl,
  cpp_module_ack_key_t key, int32_t buf_dirty, int32_t ref_count,
  isp_buf_divert_t *isp_buf);
int32_t cpp_module_enq_event(mct_module_t* module,
  cpp_module_event_t* event, cpp_priority_t prio);
int32_t cpp_module_set_clock_freq(cpp_module_ctrl_t *ctrl,
  cpp_module_stream_params_t *stream_params, uint32_t stream_event);

/* ----------------------- utility functions -------------------------------*/

mct_port_t* cpp_module_find_port_with_identity(mct_module_t *module,
  mct_port_direction_t dir, uint32_t identity);
cpp_module_ack_t* cpp_module_find_ack_from_list(cpp_module_ctrl_t *ctrl,
  cpp_module_ack_key_t key);
cam_streaming_mode_t cpp_module_get_streaming_mode(mct_module_t *module,
  uint32_t identity);
int32_t cpp_module_get_params_for_identity(cpp_module_ctrl_t* ctrl,
  uint32_t identity, cpp_module_session_params_t** session_params,
  cpp_module_stream_params_t** stream_params);
void cpp_module_dump_stream_params(cpp_module_stream_params_t* stream_params,
  const char* func, int line);
boolean cpp_module_util_map_buffer_info(void *d1, void *d2);
boolean cpp_module_util_free_buffer_info(void *d1, void *d2);
boolean cpp_module_util_create_hw_stream_buff(void *d1, void *d2);
boolean cpp_module_invalidate_q_traverse_func(void* qdata, void* input);
boolean cpp_module_release_ack_traverse_func(void* data, void* userdata);
boolean cpp_module_key_list_free_traverse_func(void* data, void* userdata);
int32_t cpp_module_update_hfr_skip(cpp_module_stream_params_t *stream_params);
int32_t cpp_module_set_output_duplication_flag(
  cpp_module_stream_params_t *stream_params);
int32_t cpp_port_get_linked_identity(mct_port_t *port, uint32_t identity,
  uint32_t *linked_identity);
pproc_divert_info_t *cpp_module_get_divert_info(uint32_t *identity_list,
  uint32_t identity_list_size, cpp_divert_info_t *cpp_divert_info);
int32_t cpp_module_set_divert_cfg_identity(uint32_t key_identity,
  uint32_t new_identity, cpp_divert_info_t *cpp_divert_info);
int32_t cpp_module_set_divert_cfg_entry(uint32_t identity,
  pproc_cfg_update_t update_mode, pproc_divert_info_t *divert_info,
  cpp_divert_info_t *cpp_divert_info);
int32_t cpp_module_util_post_diag_to_bus(mct_module_t *module,
  ez_pp_params_t *cpp_params, uint32_t identity);
int32_t cpp_module_util_update_session_diag_params(mct_module_t *module,
  cpp_hardware_params_t* hw_params);
int64_t cpp_module_get_total_load_by_value(cpp_module_ctrl_t *ctrl);
cpp_module_stream_clk_rate_t *
  cpp_module_find_clk_rate_by_identity(cpp_module_ctrl_t *ctrl,
    uint32_t identity);
static void get_cpp_loglevel();
mct_stream_t* cpp_module_util_find_parent(uint32_t identity,
  mct_module_t* module);
/* -------------------------------------------------------------------------*/

#endif
