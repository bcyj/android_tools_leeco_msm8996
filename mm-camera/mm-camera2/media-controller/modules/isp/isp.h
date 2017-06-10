/*============================================================================
Copyright (c) 2013-2015 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

#ifndef __ISP_H__
#define __ISP_H__

#include "cam_intf.h"
#include "mct_controller.h"
#include "modules.h"
#include "isp_def.h"
#include "isp_event.h"
#include "isp_ops.h"
#include "isp_hw.h"
#include "q3a_stats_hw.h"
#include "isp_buf_mgr.h"
#include "isp_zoom.h"
#include "isp_tintless.h"
#include"isp_resource_mgr.h"
#include "isp.h"

#define UNPACK_STREAM_ID(identity) (identity & 0x0000FFFF)
#define UNPACK_SESSION_ID(identity) ((identity & 0xFFFF0000) >> 16)

#define ISP_MAX_RDIS    3
#define ISP_MAX_HW    2

#define ISP_MAX_NATIVE_BUF_NUM 4
#define ISP_SRC_PORTS_3A_NUM 4
#define ISP_SRC_PORTS_BUF_NUM 10
#define ISP_SINK_PORTS_NUM 8

/*Forward declarations*/
struct _isp_t;
struct _isp_session_t;

typedef struct {
  uint32_t session_id;
  uint32_t stream_id;
} isp_add_stream_t;

typedef enum {
  ISP_SRC_PORT_DATA,    /* data port for buffer */
  ISP_SRC_PORT_3A,      /* 3A port */
  ISP_SRC_PORT_MAX      /* max src ports per stream */
} isp_src_port_type_t;

typedef struct {
  int hw_idx;        /* hw idx, 0 or 1 */
  int frame_based;   /* 1: frame based, 0 : line based */
  //int num_wms;     /* num WMs */
  int num_rdi;       /* num RDIs used. Each RDI needs one CID */
  int cids[ISP_MAX_RDIS];
  int rdi[ISP_MAX_RDIS];
} isp_stream_rdi_info_t;

typedef struct {
  int frame_based; /* frame based or line based */
  int num_hw;      /* num hws used for this stream */
  int hw_idx[ISP_MAX_HW];
} isp_stream_pix_info_t;

typedef enum {
  ISP_STREAM_STATE_INITIAL,  /* initial */
  ISP_STREAM_STATE_CREATED,  /* created */
  ISP_STREAM_STATE_ASSOCIATED_WITH_SINK_PORT,  /* linked with sink port */
  ISP_STREAM_STATE_USER_CFG, /* received user straem config */
  ISP_STREAM_STATE_HW_CFG, /* hw configured, such as pix/rdi are decided */
  ISP_STREAM_STATE_STARTING, /* starting */
  ISP_STREAM_STATE_ACTIVE,   /* got start ack already */
  ISP_STREAM_STATE_STOPPING, /* stopping */
  ISP_STREAM_STATE_MAX,
} isp_stream_state_t;

typedef struct {
  sensor_out_info_t sensor_cfg;
  ispif_out_info_t ispif_out_cfg;
  uint32_t vfe_output_mask; /* hi 16 bits - VFE1, lo 16 bits VFE0 */
  uint32_t meta_use_out_mask;
  uint32_t vfe_mask;        /* which vfe associated */
} isp_stream_cfg_t;

typedef struct {
  uint32_t saturation;
  boolean is_init_setting;
} isp_saturation_setting_t;

typedef enum {
  ISP_CHANNEL_STATE_INITIAL,  /* initial */
  ISP_CHANNEL_STATE_CREATED,  /* created */
  ISP_CHANNEL_STATE_USER_CFG, /* received user stream config */
  ISP_CHANNEL_STATE_HW_CFG, /* hw configured, such as pix/rdi are decided */
  ISP_CHANNEL_STATE_ACTIVE,   /* got start ack already */
  ISP_CHANNEL_STATE_STOPPING, /* stopping */
  ISP_CHANNEL_STATE_MAX,
} isp_channel_state_t;

typedef enum {
  ISP_CHANNEL_TYPE_IMAGE,
  ISP_CHANNEL_TYPE_META_DATA,
  ISP_CHANNEL_TYPE_MAX
}isp_channel_type_t;

typedef struct {
  //int idx;   /* index in the stream array */
  void *session; /* session ptr */
  isp_channel_state_t state;
  uint32_t session_id;
  uint32_t channel_id;
  isp_channel_type_t channel_type;
  isp_stream_cfg_t cfg;
  mct_stream_info_t stream_info;        /* stream parameters from MCT */
  mct_port_t *sink_port; /* the data sink port */
  mct_port_t *src_ports[ISP_SRC_PORT_MAX]; /* src ports */
  //isp_stream_state_t stream_state;
  boolean use_native_buf;
  uint8_t current_num_bufs;/*Buffers in use for deferred alloc use case*/
  uint8_t total_num_bufs; /*Total buffers in deferred buf alloc use case*/
  //int link_cnt;
  uint32_t user_stream_idx_mask;
  int channel_idx; /* array index */
  boolean is_encoder;
  boolean dis_enable;
  boolean divert_to_3a;
  sensor_meta_t meta_info;
  int streamon_cnt;
  uint32_t bufq_handle;
} isp_channel_t;

typedef struct {
  //int idx;   /* index in the stream array */
  void *session; /* session ptr */
  isp_stream_state_t state;
  uint32_t session_id;
  uint32_t stream_id;
  isp_stream_cfg_t cfg;
  mct_stream_info_t stream_info;        /* stream parameters from MCT */
  mct_port_t *sink_port; /* the data sink port */
  mct_port_t *src_ports[ISP_SRC_PORT_MAX]; /* src ports */
  //isp_stream_state_t stream_state;
  //boolean use_native_buf;
  //uint8_t num_bufs;
  int link_cnt;
  /* for sychronization in case ispif outputs two stripes */
  //uint32_t split_sync_mask;
  //void *hw_stream_ptr;
  int stream_idx; /* array index */
  boolean is_encoder;
  boolean dis_enable;
  boolean divert_to_3a;
  //int shared_streamon_cnt;
  //uint32_t bufq_handle;
  /* adding channel related new fields */
  uint32_t channel_idx_mask;
  sensor_meta_t meta_info;
} isp_stream_t;

typedef enum {
  ISP_ASYNC_COMMAND_UV_SUBSAMPLE, /* uv subsampling - uint32_t */
  ISP_ASYNC_COMMAND_STREAMON,     /* isp_session_cmd_streamon_t */
  ISP_ASYNC_COMMAND_STRAEMOFF,    /* */
  ISP_ASYNC_COMMAND_SET_HW_PARAM, /* isp_buffered_hw_params_t */
  ISP_ASYNC_COMMAND_WM_BUS_OVERFLOW_RECOVERY, /* isp_wm_bus_overflow */
  ISP_ASYNC_COMMAND_EXIT,         /* exit the thraed loop */
  ISP_ASYNC_COMMAND_MAX           /* not used */
} isp_async_cmd_id_t;

typedef struct {
  void *isp;
  void *isp_sink_port; /* isp_port_t */
  uint32_t session_id;
  uint32_t stream_id;
  mct_event_t *event;
  void *session; /* isp_session_t */
  boolean sync_cmd;
} isp_session_cmd_streamon_t;

typedef struct {
  void *isp;
  void *isp_sink_port; /* isp_port_t */
  uint32_t session_id;
  uint32_t stream_id;
  mct_event_t *event;
  void *session; /* isp_session_t */
  boolean sync_cmd;
} isp_session_cmd_streamoff_t;

typedef struct {
  void *isp;
  void *session; /* isp_session_t */
} isp_session_cmd_set_hw_params_t;

typedef struct {
  void *isp_hw;
  void *session;
} isp_session_cmd_wm_bus_overflow_t;

typedef struct {
  isp_async_cmd_id_t cmd_id;
  union {
    uint32_t uv_subsample_enable;
    isp_session_cmd_streamon_t streamon;
    isp_session_cmd_streamoff_t streamoff;
    isp_session_cmd_set_hw_params_t set_hw_params;
    isp_session_cmd_wm_bus_overflow_t wm_recovery;
  };
} isp_async_cmd_t;

typedef enum {
  ISP_UV_SWITCH_STATE_IDLE,
  ISP_UV_SWITCH_STATE_ENQUEUED,
  ISP_UV_SWITCH_STATE_IN_PROGRESS,
  ISP_UV_SWITCH_STATE_FINISHING,
  ISP_UV_SWITCH_STATE_MAX
} isp_uv_switch_state_t;

typedef struct {
  isp_uv_switch_state_t switch_state;
  uint32_t        min_wait_cnt;
  uint32_t        curr_enable;
  long        trigger_A; /*Trigger point for CDS hystersis*/
  long        trigger_B; /*Trigger point for CDS hystersis*/
} isp_uv_subsample_t;

typedef struct {
  struct _isp_t *isp;
  struct _isp_session_t *session;
  pthread_t        thread;
  boolean          thread_started;
  mct_queue_t      task_q;
  pthread_mutex_t  task_q_mutex; /* for lock/unlock the task_q operation */
  sem_t            task_q_sem;   /* for enqueue cmd to the task */
  pthread_mutex_t  sync_mutex;   /* for serialize the sync command */
  sem_t            sync_sem;     /* for blocking command */
  int              sync_ret;     /* sync return value */
  boolean          wait_hw_update_done; /* wait hw update down */
  sem_t            hw_wait_sem;  /* sem for waitomng hw update done */
} isp_async_task_t;

typedef enum {
  ISP_UV_SUBSAMPLE_OFF,
  ISP_UV_SUBSAMPLE_ON,
  ISP_UV_SUBSAMPLE_AUTO,
} isp_uv_subsample_mode_t;

typedef struct _isp_session_t {
  void *isp_data; /* isp_data_t ptr */
  isp_stream_t streams[ISP_MAX_STREAMS];  /* place holder for ISP
                                           * received MCT streams */
  isp_channel_t channel[ISP_MAX_STREAMS]; /* ISP's own stream channel
                                           * for stream hw configuration.
                                           * one channel maps to one actual
                                           * hardware stream */
  uint32_t session_id;  /* session id */
  int num_stream;
  uint32_t active_count;
  int session_idx;
  uint32_t vfe_ids;
  uint32_t sof_frame_id;
  uint32_t vfe_updating_mask; /* each bit in the mask indicates which
                               * hw is currently applying hw_update */
  modulesChromatix_t chromatix;
  int ion_fd;
  uint32_t hal_bundling_mask;
  uint32_t streamon_bundling_mask;
  uint32_t streamoff_bundling_mask;
  isp_stats_config_t stats_config;
  isp_saved_params_t saved_params;
  isp_hw_pending_update_params_t pending_update_params;
  isp_uv_subsample_t uv_subsample_ctrl;
  boolean use_pipeline;
  isp_hfr_param_t hfr_param;
  isp_dis_param_t dis_param;
  q3a_ihist_stats_t ihist_stats;
  uint8_t ihist_update;
  uint8_t isp_fast_aec_mode;
  boolean flash_streamon;
  int32_t zoom_val;
  int32_t pproc_zoom_val;
  isp_zoom_session_t *zoom_session;
  int zoom_stream_cnt;
  uint8_t num_src_data_port;
  isp_async_task_t async_task;
  int32_t recording_hint;
  isp_uv_subsample_mode_t uv_subsample_mode;
  isp_buffered_hw_params_t buffered_hw_params;
  isp_tintless_session_t *tintless_session;
  uint32_t sof_id[VFE_MAX];
  isp_hw_zoom_param_t temp_zoom_params[2]; /* To save ROI params in dual ISP*/
  uint32_t temp_frame_id; /* To synchronize ROI calculation & params update in dual ISP*/
  int temp_zoom_roi_hw_id_mask;
  mct_bracket_ctrl_t bracket_info;
  boolean is_session_active;
  pthread_mutex_t  state_mutex;
  isp_reg_update_info_t reg_update_info;
} isp_session_t;

typedef struct {
  uint32_t input_format;
} isp_fetch_eng_info_t;

typedef struct {
  isp_input_type_t input_type;
  union {
    //isp_sink_port_sensor_cfg_t sensor;
    isp_fetch_eng_info_t fetch_eng_info;
  };
  isp_stream_t *streams[ISP_MAX_STREAMS];
  ispif_src_port_caps_t caps; /* ispif src cap is isp sink cap */
  int num_streams;
} isp_sink_port_t;

typedef struct {
  isp_src_port_type_t port_type;
  ispif_src_port_caps_t caps;
  isp_stream_t *streams[ISP_MAX_STREAMS]; /* num of streams using this port */
  int num_streams;
  cam_streaming_mode_t streaming_mode;
} isp_src_port_t;

typedef enum {
  ISP_PORT_STATE_CREATED,
  ISP_PORT_STATE_RESERVED,
  ISP_PORT_STATE_MAX
} isp_port_state_t;

typedef struct {
  mct_port_t *port;      /* local port */
  isp_port_state_t state;
  uint32_t session_id;
  void *isp;
  union {
    isp_src_port_t src_port;
    isp_sink_port_t sink_port;
  } u;
} isp_port_t;

/* ISP subdev information defined here */
typedef struct {
  uint32_t isp_version; /* isp version. */
  isp_hw_cap_t cap; //isp_subdev_res_t rdi_res[6];
  char subdev_name[ISP_SUBDEV_NAME_LEN];
} isp_subdev_info_t;

typedef struct {
  int num;
  int num_pix; /* total num of pix interfaces */
  int num_rdi; /* total number of RDI interfaces */
  isp_subdev_info_t sd_info[ISP_SD_NODE_ID_MAX_NUM];
} isp_subdevs_t;

typedef struct {
  isp_ops_t *hw_ops;
  isp_notify_ops_t notify_ops;
  int ref_cnt;
  pthread_mutex_t mutex;
} isp_data_hw_t;

typedef struct {
  isp_subdevs_t sd_info;
  isp_data_hw_t hw[ISP_MAX_HW];
  isp_session_t sessions[ISP_MAX_SESSIONS];
  pthread_mutex_t session_critical_section[ISP_MAX_SESSIONS];
  pthread_cond_t session_cond[ISP_MAX_SESSIONS];
  isp_zoom_t *zoom;
  isp_buf_mgr_t buf_mgr;
  isp_tintless_t *tintless;
} isp_data_t;

/* isp root struct */
typedef struct _isp_t {
  mct_module_t *module; /* isp is MCT module */
  pthread_mutex_t mutex;
  isp_data_t data;    /* isp's local data struct */
  isp_buf_mgr_t buf_mgr;
  isp_resources_t *res_mgr;
  uint32_t prev_sent_streamids[MAX_STREAMS_NUM];
  uint32_t prev_sent_streamids_cnt;
} isp_t;

int isp_create(isp_t **isp_ctrl);
int isp_destroy (isp_t *isp);
int isp_start_session(isp_t *isp, uint32_t session_id);
int isp_stop_session(isp_t *isp, uint32_t session_id);
int isp_streamon(isp_t *isp, isp_port_t *isp_sink_port,
  uint32_t session_id, uint32_t stream_id, mct_event_t *event, boolean flash_streamon);
int isp_streamoff(isp_t *isp, isp_port_t *isp_sink_port,
                 uint32_t session_id, uint32_t stream_id, mct_event_t *event);
int isp_set_chromatix(isp_t *isp, isp_port_t *isp_sink_port,
  uint32_t session_id, uint32_t stream_id, void *chromatix);
int isp_set_reload_chromatix(isp_t *isp, isp_port_t *isp_sink_port,
  uint32_t session_id, uint32_t stream_id, void *chromatix);
int isp_set_fast_aec_mode(isp_t *isp,
          uint32_t session_id, uint32_t stream_id, void *fast_aec_mode);
int isp_set_divert_to_3a(isp_t *isp, isp_port_t *isp_src_port,
  uint32_t session_id, uint32_t stream_id);
int isp_set_af_rolloff_params(isp_t *isp, isp_port_t *isp_sink_port,
  uint32_t session_id, uint32_t stream_id, void *data);
int isp_reserve_sink_port(isp_t *isp, isp_port_t *reserved_sink_port,
  ispif_src_port_caps_t *ispif_src_cap, mct_stream_info_t *stream_info,
  unsigned int session_id, unsigned int stream_id);
int isp_unreserve_sink_port(isp_t *isp, isp_port_t *isp_sink_port,
  uint32_t session_id, uint32_t stream_id);
int isp_reserve_src_port(isp_t *isp, isp_port_t *reserved_isp_src_port,
  mct_stream_info_t *stream_info, unsigned int session_id,
  unsigned int stream_id);
int isp_unreserve_src_port(isp_t *isp, isp_port_t *isp_port,
  unsigned int session_id, unsigned int stream_id);
int isp_unlink_sink_port(isp_t *isp, isp_port_t *isp_sink_port,
  mct_port_t *peer_port, uint32_t session_id, uint32_t stream_id);
int isp_link_sink_port(isp_t *isp, isp_port_t *isp_port, mct_port_t *peer_port,
  uint32_t session_id, uint32_t stream_id);
int isp_link_src_port(isp_t *isp, isp_port_t *isp_port, mct_port_t *peer_port,
  uint32_t session_id, uint32_t stream_id);
int isp_unlink_src_port(isp_t *isp, isp_port_t *isp_src_port,
  mct_port_t *peer_port, uint32_t session_id, uint32_t stream_id);
int isp_sink_port_stream_config(isp_t *isp, isp_port_t *isp_sink_port,
  ispif_src_port_stream_cfg_t *ispif_src_stream_cfg);
int isp_set_hal_param(isp_t *isp, isp_port_t *isp_sink_port,
  uint32_t session_id, uint32_t stream_id, mct_event_t *event);
int isp_set_hal_stream_param(isp_t *isp, isp_port_t *isp_sink_port,
  uint32_t session_id, uint32_t stream_id, mct_event_t *event);
int isp_set_awb_trigger_update(isp_t *isp, isp_port_t *isp_sink_port,
  uint32_t session_id, uint32_t stream_id, void *data);
int isp_set_chroma_subsample_enable(isp_t *isp, isp_port_t *isp_sink_port,
  uint32_t session_id, uint32_t stream_id, void *data);
int isp_set_aec_trigger_update(isp_t *isp, isp_port_t *isp_sink_port,
  uint32_t session_id, uint32_t stream_id, void *data);
int isp_set_uv_subsample(isp_t *isp, uint32_t session_id,
  stats_update_t *stats_data);
int isp_buf_divert_ack(isp_t *isp, isp_port_t *isp_src_port,
  uint32_t session_id, uint32_t stream_id, isp_buf_divert_ack_t *divert_ack);
int isp_set_af_config(isp_t *isp, isp_port_t *isp_sink_port,
  uint32_t session_id, uint32_t stream_id, void *data);
int isp_save_aec_param(isp_t *isp, isp_port_t *isp_sink_port,
  uint32_t session_id, uint32_t stream_id, void *data);
int isp_save_asd_param(isp_t *isp, isp_port_t *isp_sink_port,
  uint32_t session_id, uint32_t stream_id, void *data);
int isp_set_sensor_lens_position_trigger_update(isp_t *isp,
  isp_port_t *isp_sink_port, uint32_t session_id, uint32_t stream_id,
  void *data);
int isp_get_la_gamma_tbl(isp_t *isp, isp_port_t *isp_sink_port,
  uint32_t session_id, uint32_t stream_id, void *data);
int isp_set_gain_from_sensor_req(isp_t *isp, isp_port_t *isp_sink_port,
  uint32_t session_id, uint32_t stream_id, void *data);
int isp_thread_async_task_start(isp_t *isp, isp_session_t *session);
int isp_thread_async_task_stop(isp_t *isp, isp_session_t *session);
int isp_proc_async_command(isp_t *isp, isp_session_t *session,
  isp_async_cmd_t *cmd);
int isp_proc_set_hw_params(isp_t *isp, isp_session_t *session);
int isp_enqueue_async_command(isp_t *isp, isp_session_t *session,
  isp_async_cmd_t **cmd);
int isp_meta_channel_config(isp_t *isp, uint32_t stream_id,
  uint32_t session_id, sensor_meta_t *meta_info);
int isp_video_hdr_config(isp_t *isp, uint32_t stream_id, uint32_t session_id,
  sensor_meta_t *meta_info);
int isp_send_rolloff_to_sensor(isp_t *isp, uint32_t session_id,
  uint32_t stream_id, mct_port_t *mct_port);
int isp_set_stats_config_update(isp_t *isp,
  isp_port_t *isp_sink_port, uint32_t session_id,
  uint32_t stream_id, void *data);
int isp_set_flash_mode(isp_t *isp, isp_port_t *isp_sink_port,
  uint32_t session_id, uint32_t stream_id, void *data);
int isp_update_buf_info(isp_t *isp, uint32_t session_id,
  uint32_t stream_id, mct_event_t *event);
int isp_set_cam_bracketing_ctrl(isp_t *isp, isp_port_t *isp_sink_port,
  uint32_t session_id, uint32_t stream_id, void *data);
void port_isp_destroy_ports(isp_t *isp);
#endif /* __ISP_H__ */

