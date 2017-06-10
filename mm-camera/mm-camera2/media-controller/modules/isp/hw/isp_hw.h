/*============================================================================
Copyright (c) 2013-2015 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

#ifndef __ISP_HW_H__
#define __ISP_HW_H__

#include <fcntl.h>
#include <poll.h>
#include <semaphore.h>
#include "cam_intf.h"
#include "mct_controller.h"
#include "modules.h"
#include "isp_def.h"
#include "isp_event.h"
#include "isp_hw_module_ops.h"
#include "eztune_vfe_diagnostics.h"
#include "q3a_stats_hw.h"

#define ISP_MSM8960V1    0x3030b
#define ISP_MSM8960V2    0x30408
#define ISP_MSM8930      0x3040f
#define ISP_MSM8974_V1   0x10000018
#define ISP_MSM8974_V2   0x1001001A
#define ISP_MSM8974_V3   0x1001001B
#define ISP_MSM8226_V1   0x20000013
#define ISP_MSM8226_V2   0x20010014
#define ISP_MSM8610      0x3050A
#define ISP_MSM8916      0x10030000
#define ISP_MSM8939      0x10040000
#define ISP_MSM8909      0x30600
#define ISP_REVISION_V1  1
#define ISP_REVISION_V2  2
#define ISP_REVISION_V3  3

#define ISP_MAX_THREAD_POIING_FDS 2
#define ISP40_NUM_REG_DUMP 576 /* 0x900(hex) = 2304 / 4(4 byte per register) */
#define ISP32_NUM_REG_DUMP 576

#define ISP_LIMIT_FPS_3p0 (3.0 * Q8)
#define ISP_LIMIT_FPS_4p5 (4.5 * Q8)
#define ISP_LIMIT_FPS_7p5 (7.5 * Q8)
#define ISP_LIMIT_FPS_3p9 (3.9 * Q8)

#define VFE32_BURST_LEN 2
#define VFE32_UB_SIZE 1024 /* 1024 * 128 bits = 16KB */
#define VFE32_UB_SIZE_32KB 2048
#define VFE32_STATS_BURST_LEN 2

#define VFE40_BURST_LEN_1 1
#define VFE40_BURST_LEN_2 2
#define VFE40_UB_SIZE_24KB 1536 /* 1536 * 128 bits = 24KB */
#define VFE40_UB_SIZE_32KB 2048 /* 2048 * 128 bits = 32KB */
#define VFE40_UB_SIZE_48KB 3072 /* 3072 * 128 bits = 48KB */
#define VFE40_STATS_BURST_LEN_1 1
#define VFE40_STATS_BURST_LEN_2 2
#define VFE40_UB_SLICING_POLICY_DEFAULT 0
#define VFE40_UB_SLICING_POLICY_EQUAL 1
#define VFE44_BURST_LEN 3
#define VFE44_STATS_BURST_LEN 2
#define VFE44_UB_SIZE 2048

/* root struct for isp pix interface */
typedef enum {
  ISP_HW_PIPELINE,
  ISP_HW_AXI,
  ISP_HW_MAX,
} isp_hw_interface_type_t;

/* dump type definition*/
typedef enum {
  ISP_HW_DUMP_REG,
  ISP_HW_DUMP_DMI_16BIT,
  ISP_HW_DUMP_DMI_32BIT,
  ISP_HW_DUMP_DMI_64BIT,
  ISP_HW_DUMP_MAX,
} isp_dump_type_t;

/* ISP HW structures defined here */
typedef enum {
  ISP_HW_STATE_INVALID,   /* initial state */
  ISP_HW_STATE_DEV_OPEN,      /* ISP in idle state */
  ISP_HW_STATE_IDLE,      /* ISP in idle state */
  ISP_HW_STATE_ACTIVE,    /* ISP HW in use */
  ISP_HW_STATE_STREAM_STOPPING,
  ISP_HW_STATE_MAX        /* max state num */
} isp_hw_state_t;

typedef enum {
  /* zero is not used enum */
  MM_ISP_CMD_NOT_USED,
  MM_ISP_CMD_NOTIFY_OPS_INIT, /* isp_pipe_notify_ops_init_t */
  MM_ISP_CMD_SET_PARAMS,      /* isp_pipe_set_params_t */
  MM_ISP_CMD_GET_PARAMS,      /* isp_pipe_get_params_t */
  MM_ISP_CMD_ACTION,          /* isp_pipe_action_t */
  MM_ISP_CMD_TIMER,
  MM_ISP_CMD_DESTROY,         /* no payload, equal to ops's destroy */
  MM_ISP_CMD_SOF_UPDATE,       /* msm_isp_event_data*/
  /* max count */
  MM_ISP_CMD_MAX
} isp_thread_pipe_cmd_t;

typedef struct {
  uint32_t params_id;
  void *in_params;
  uint32_t in_params_size;
} isp_pipe_set_params_t;

typedef struct {
  uint32_t params_id;
  void *in_params;
  uint32_t in_params_size;
  void *out_params;
  uint32_t out_params_size;
} isp_pipe_get_params_t;

typedef struct {
  uint32_t action_code;
  void *data;
  uint32_t data_size;
} isp_pipe_action_t;

typedef struct {
  void *in_params;
  isp_notify_ops_t *notify_ops;
} isp_pipe_notify_ops_init_t;

typedef void (*isp_thread_timeout_func) (void *user_data, uint32_t timeout);

typedef struct {
  uint32_t timer_num;
  struct timeval start_time;
  int32_t timeoutms;
  void *userdata;
  isp_thread_timeout_func timeout_func;
  sem_t sleep_sem;
}isp_tread_timer_t;

/** isp_saved_params_t

 *    @effect:        saved effect
 *    @contrast:      saved contrast value
 *    @bestshot:      saved bestshot value
 *
 *  This structure holds all isp parameters set by HAL upon streamon command
 **/
typedef struct {
  int32_t effect;
  int32_t contrast;
  cam_scene_mode_type bestshot;
  int32_t sce_factor;
  stats_update_mask_t stats_flag;
  awb_update_t awb_stats_update;
  aec_update_t aec_stats_update;
  q3a_ihist_stats_t ihist_stats;
  cam_flash_mode_t flash_mode;
  int32_t saturation;
  uint32_t vhdr_enable;
  int32_t sharpness;
  isp_param_frame_skip_pattern_t frame_skip;
  boolean use_bundled_frame_skip;
  isp_param_frame_skip_pattern_t bundled_frame_skip;
  uint8_t ihist_update;
  float dig_gain;
  uint32_t uv_subsample_enable;
  uint32_t vt_enable;
  boolean uv_subsample_update;
  isp_hw_set_crop_factor_t zoom_factor;
  boolean zoom_update;
  isp_flash_params_t flash_params;
  awb_update_t awb_stored_stats;
  aec_update_t aec_stored_stats;
  uint32_t longshot_enable;
  uint32_t lowpowermode_yuv_enable;
  uint32_t lowpowermode_enable;
  uint32_t lowpowermode_feature_enable;
  uint32_t lowpowermode_feature_mask;
} isp_saved_params_t;

/** isp_hw_pending_update_params_t
 *    @frame_id:        saved effect
 *    @hw_update_params:      saved contrast value
 *
 *  This structure holds all isp parameters set by HAL upon streamon command
 **/
typedef struct {
  uint32_t frame_id;    /* hw input */
  uint32_t session_id;  /* hw input for finding session */
  isp_saved_params_t hw_update_params; /* return result */
  boolean hw_fetch_pending[VFE_MAX];
  uint32_t dev_idx;
} isp_hw_pending_update_params_t;

typedef struct {
  uint32_t timer_cnt;
  pthread_t pid;
  isp_tread_timer_t *timer; /* only support one timer */
  uint32_t cmd; /* commend sending to the thread */
  int return_code; /* return code of the pipe command */
  int async_ret;
  pthread_mutex_t  cmd_mutex;
  isp_pipe_set_params_t *set_param_cmd;
  isp_pipe_get_params_t *get_param_cmd;
  isp_pipe_action_t     *action_cmd;
  isp_pipe_notify_ops_init_t *init_cmd;
  struct msm_isp_event_data sof_parm;
  sem_t sig_sem;
  int32_t pipe_fds[2];
  int poll_fd; /* non zero if is asked to poll subdev event */
  int poll_timeoutms;
  struct pollfd poll_fds[ISP_MAX_THREAD_POIING_FDS];
  uint8_t num_fds;
  void *hw_ptr;
  pthread_mutex_t  busy_mutex;
  boolean thread_busy;
  boolean wake_up_at_sof;
  sem_t thread_wait_sem; /* thread waits on this semphore */
  isp_hw_pending_update_params_t pending_update_params;
} isp_thread_t;

typedef struct {
  uint32_t lines_per_frame;
  uint32_t pixels_per_line;
  uint32_t first_pixel;
  uint32_t last_pixel;
  uint32_t first_line;
  uint32_t last_line;
  enum ISP_START_PIXEL_PATTERN pixel_pattern;
  uint32_t v4l2_fmt;
  uint32_t pixel_clock;
  uint32_t ref_count;
  uint32_t input_format;
} isp_camif_cfg_t;

typedef struct {
  uint32_t cid;
  uint32_t frame_based;
  uint32_t pixel_clock;
  uint32_t ref_count;
} isp_rdi_cfg_t;

typedef struct {
  isp_camif_cfg_t camif_cfg;
  isp_rdi_cfg_t rdi_cfg[3];
} isp_hw_input_t;

typedef struct {
  uint32_t ref_cnt;
  void *private_data;
  isp_notify_ops_t hwif_notify_ops;
  uint32_t session_id[VFE_SRC_MAX];
  int num_active_streams;
} isp_hw_pipeline_t;

typedef struct {
  uint32_t ref_cnt;
  void *private_data;
} isp_hw_axi_t;

typedef struct {
  uint32_t stream_id;
  mct_stream_info_t stream_info;
  sensor_out_info_t sensor_out_info;
  ispif_out_info_t ispif_out_info;
  isp_out_info_t isp_out_info;
  uint8_t use_pix;
  uint32_t meta_ch_idx_mask;
  boolean is_encoder;
  isp_input_type_t input_type;
  uint32_t vfe_output_mask; /* hi 16 bits - VFE1, lo 16 bits VFE0 */
  sensor_src_port_cap_t sink_cap;
  isp_mapped_buf_params_t map_buf;
  boolean need_divert;
  boolean need_uv_subsample;
  boolean use_native_buf;
  enum msm_vfe_frame_skip_pattern skip_pattern;
  uint32_t burst_len;
} isp_hw_stream_cfg_entry_t;

typedef enum {
  ISP_HW_STREAM_STATE_NULL,
  ISP_HW_STREAM_STATE_CFG, /* cfged */
  ISP_HW_STREAM_STATE_CFG_START_PENDING, /* cfged not start */
  ISP_HW_STREAM_STATE_STARTED, /* started */
  ISP_HW_STREAM_STATE_STOPPING, /* stopping */
  ISP_HW_STREAM_STATE_MAX
} isp_hw_stream_state_t;

typedef struct {
  isp_hw_stream_cfg_entry_t cfg;
  isp_hw_stream_state_t state;
} isp_hw_stream_t;

typedef struct {
  void *isp_hw;
  isp_hw_stream_t streams[ISP_MAX_STREAMS];
  int active_stream;
  int started;
  uint32_t session_id;
  int ion_fd;
  int vt_enable;
  isp_hfr_param_t hfr_param;
  boolean use_pipeline;
} isp_hw_session_t;

typedef struct {
  uint32_t isp_version; /* pass in init */
  int dev_idx;       /* device index, i.e. VFE0 or VFE1, pass in init */
  isp_hw_cap_t cap;  /* hw capabilities */
  void *buf_mgr;
} isp_hw_init_params_t;

typedef struct {
  uint32_t is_kernel_dump;
  uint32_t read_type;
  uint32_t read_lengh;
  uint32_t read_bank;
  uint32_t bank_idx;
} isp_hw_read_info;

typedef struct {
  isp_hw_read_info vfe_read_info[ISP_META_MAX];
  isp_meta_t meta_data;
} isp_hw_dump_t;

typedef struct  {
  boolean has_user_dianostics;
  vfe_diagnostics_t user_vfe_diagnostics;
} isp_diag_t;

typedef struct {
  boolean enable;
  vfemodule_t module;
} isp_mod_trigger_t;

typedef struct {
  isp_hw_session_t session[ISP_MAX_SESSIONS];
  uint32_t num_active_streams;
  uint32_t is_overflow;
  uint32_t hw_update_skip;
  isp_hw_dump_t dump_info;
  isp_hw_init_params_t init_params;
  int open_cnt;
  int fd;                  /* isp file descriptor */
  isp_hw_state_t hw_state; /* ISP HW state */
  void *parent;            /* isp_data_t pointer */
  isp_ops_t hw_ops;
  isp_notify_ops_t *notify_ops;
  isp_hw_input_t input;
  isp_hw_pipeline_t  pipeline;
  isp_hw_axi_t  axi;
  isp_thread_t thread_poll;
  isp_thread_t thread_stream;
  isp_thread_t thread_hw;
  boolean use_hw_thread_for_ack;
  isp_diag_t isp_diag;
  pthread_mutex_t overflow_mutex;
  sem_t reset_done;
  int sof_subscribe_ref_cnt[ISP_INTF_MAX];
} isp_hw_t;

typedef enum {
  ISP_HW_NOTIFY_ISP_ERR_HALT_AXI,
  ISP_HW_NOTIFY_STATS,
  ISP_HW_NOTIFY_CAMIF_SOF,
  ISP_HW_NOTIFY_HW_UPDATING,
  ISP_HW_NOTIFY_BUF_DIVERT,
  ISP_HW_NOTIFY_STATS_AWB_INFO,
  ISP_HW_NOTIFY_METADATA_INFO,
  ISP_HW_NOTIFY_META_VALID,
  ISP_HW_NOTIFY_STATS_BE_CONFIG,
  ISP_HW_NOTIFY_ROLLOFF_CONFIG,
  ISP_HW_NOTIFY_ROLLOFF_GET,
  ISP_HW_NOTIFY_BG_PCA_STATS_CONFIG,
  ISP_HW_NOTIFY_FETCH_HW_UPDATE_PARAMS,
  ISP_HW_NOTIFY_CUR_ROLLOFF,
  ISP_HW_NOTIFY_ZOOM_ROI_PARAMS,
  ISP_HW_NOTIFY_ISPIF_TO_RESET,
  ISP_HW_NOTIFY_MAX
} isp_hw_notify_params_id_t;

typedef enum {
  ISP_HW_SET_PARAM_OVERFLOW_DETECTED,
  ISP_HW_SET_PARAM_AF_ROLLOFF_PARAMS,
  ISP_HW_SET_PARAM_CHROMATIX,         /* modulesChromatix_t */
  ISP_HW_SET_PARAM_MAPPED_BUF,        /* isp_mapped_buf_params_t */
  ISP_HW_SET_PARAM_BEST_SHOT,         /* int maps to camera_bestshot_mode_type */
  ISP_HW_SET_PARAM_EFFECT,            /* isp_effects_params_t */
  ISP_HW_SET_PARAM_STATS_CFG_UPDATE,            /* isp_set_af_params_t */
  ISP_HW_SET_PARAM_STATS_ENABLE,      /* isp_stats_enable_t */
  ISP_HW_SET_PARAM_STREAM_CFG,        /* isp_hwif_output_cfg_t */
  ISP_HW_SET_PARAM_STREAM_UNCFG,      /* isp_hwif_output_cfg_t */
  ISP_HW_SET_PARAM_RELOAD_3A_TRIGGER_DATA, /* isp_3a_trigger_udpate_t */
  ISP_HW_SET_PARAM_SENSOR_LENS_POSITION_TRIGGER_UPDATE, /* lens_position_update_isp_t */
  ISP_HW_SET_PARAM_AWB_TRIGGER_UPDATE,     /* stats_update_t */
  ISP_HW_SET_PARAM_AEC_TRIGGER_UPDATE,     /* stats_update_t */
  ISP_HW_SET_FLASH_MODE,              /* cam_flash_mode_t */
  ISP_HW_SET_IHIST_LA_TRIGGER_UPDATE,
  ISP_HW_SET_PARAM_SET_UV_SUBSAMPLE,
  ISP_HW_SET_PARAM_WB_MODE,           /* int maps to config3a_wb_type_t */
  ISP_HW_SET_PARAM_SHARPNESS,         /* isp_sharpness_info_t */
  ISP_HW_SET_PARAM_FLASH,             /* isp_flash_params_t */
  ISP_HW_SET_PARAM_CONTRAST,          /* int32_t */
  ISP_HW_SET_PARAM_SATURATION,        /* int32_t */
  ISP_HW_SET_PARAM_STATS_CFG,         /* isp_stats_config_t */
  ISP_HW_SET_PARAM_FRAME_SKIP,        /* isp_hw_set_frame_skip_pattern_t */
  ISP_HW_SET_PARAM_SET_SAVED_PARAMS,  /* isp_saved_params_t */
  ISP_HW_SET_PARAM_SAVE_AEC_PARAMS,   /* isp_saved_params_t */
  ISP_HW_SET_PARAM_SAVE_ASD_PARAMS,   /* isp_saved_params_t */
  ISP_HW_SET_PARAM_CROP_FACTOR,       /* isp_hw_set_crop_factor_t */
  ISP_HW_SET_PARAM_SCE,               /* int32_t */
  ISP_HW_SET_PARAM_HW_UPDATE_SKIP,    /* uint32_t */
  ISP_HW_SET_RECORDING_HINT,          /* int32_t */
  ISP_HW_SET_PARAM_VHDR,              /* uint32_t */
  ISP_HW_SET_PARAM_RELOAD_CHROMATIX,  /* modulesChromatix_t */
  ISP_HW_SET_PARAM_TINTLESS,          /* isp_tintless_data_t*/
  ISP_HW_SET_ISP_DIAGNOSTICS,         /* int32_t */
  ISP_HW_SET_ISP_MODULE_TRIGGER,      /* int32_t */
  ISP_HW_SET_ISP_MODULE_ENABLE,       /* int32_t */
  ISP_HW_SET_PARAM_BRACKETING_DATA,   /* mct_bracket_ctrl_t */
  ISP_HW_SET_PARAM_MAX
} isp_hw_set_params_id_t;

typedef enum {
  ISP_HW_GET_CAPABILITY,
  ISP_HW_GET_3A_TRIGGER_DATA,
  ISP_HW_GET_ROLLOFF_GRID_INFO,
  ISP_HW_GET_CS_RS_CONFIG,     /* isp_cs_rs_config_t */
  ISP_HW_GET_FOV_CROP,         /* isp_hw_zoom_param_t */
  ISP_HW_GET_ROI_MAP,          /* isp_hw_zoom_parm_t */
  ISP_HW_GET_PARAM_LA_GAMMA_TBLS, /* mct_isp_table_t*/
  ISP_HW_GET_PARAM_ROLLOFF_TABLE, /* mct_event_stats_isp_rolloff_t*/
  ISP_HW_GET_PARAM_CDS_TRIGER_VAL, /*isp_uv_subsample_t*/
  ISP_HW_GET_PARAM_MAX_NUM
} isp_hw_get_params_id_t;

typedef enum {
  ISP_HW_ACTION_CODE_STREAM_START,      /* isp_action_stream_start_stop_t */
  ISP_HW_ACTION_CODE_STREAM_START_ACK,  /* isp_action_stream_start_stop_t */
  ISP_HW_ACTION_CODE_STREAM_STOP,       /* isp_action_stream_start_stop_t */
  ISP_HW_ACTION_CODE_STREAM_STOP_ACK,   /* isp_action_stream_start_stop_t */
  ISP_HW_ACTION_CODE_HALT,
  ISP_HW_ACTION_CODE_RESET,
  ISP_HW_ACTION_CODE_RESTART,
  ISP_HW_ACTION_CODE_BUF_DIVERT_ACK,    /* isp_hw_buf_divert_ack_t */
  ISP_HW_ACTION_CODE_WAKE_UP_AT_SOF,    /* null */
  ISP_HW_ACTION_CODE_MAX_NUM
} isp_hw_action_code_t;

typedef struct {
  uint32_t dev_id;      /* isp hw idx */
  uint32_t session_id;  /* session id */
  int vt_enable; /* Video call status */
  int ion_fd;
  int num_streams;      /* num streams */
  isp_hfr_param_t hfr_param;
  isp_hw_stream_cfg_entry_t entries[ISP_MAX_STREAMS];
} isp_hw_stream_cfg_t;

typedef struct {
  uint32_t session_id;
  int num_streams;
  uint32_t stream_ids[ISP_MAX_STREAMS];
} isp_hw_stream_uncfg_t;

typedef struct {
  uint32_t session_id;
  uint32_t stream_id;
  int buf_idx;
  boolean is_buf_dirty;
} isp_hw_buf_divert_ack_t;


/* thread processing functions */
int isp_hw_proc_set_params(void *ctrl, uint32_t params_id, void *in_params,
      uint32_t in_params_size);
int isp_hw_proc_get_params(void *ctrl, uint32_t params_id, void *in_params,
      uint32_t in_params_size, void *out_params, uint32_t out_params_size);
int isp_hw_proc_action(void *ctrl, uint32_t action_code, void *action_data,
      uint32_t action_data_size, int previous_ret_code);
int isp_hw_proc_init(void *ctrl, void *in_params,
      isp_notify_ops_t *notify_ops);
int isp_hw_proc_destroy(void *ctrl);

/* pix interface apis */
isp_ops_t *isp_hw_create(char *dev_name);
int isp_hw_proc_hw_update(void *ctrl, struct msm_isp_event_data *sof);
int isp_thread_start(isp_thread_t *thread_data, void *hw_ptr, int poll_fd);
int isp_hw_query_caps(const char *dev_name, uint32_t *isp_version,
      isp_hw_cap_t *cap, int id);
void isp_hw_proc_subdev_event(isp_hw_t *isp_hw, isp_thread_t *thread_data);
int isp_sem_thread_start(isp_thread_t *thread_data, void *hw_ptr);
int isp_sem_thread_stop(isp_thread_t *thread_data);
uint32_t isp_hw_find_primary_cid(sensor_out_info_t *sensor_cfg,
           sensor_src_port_cap_t *sensor_cap);
int isp_hw_proc_update_params_at_sof(isp_hw_t *isp_hw,
  struct msm_isp_event_data *sof);
int isp_hw_proc_hw_request_reg_update(isp_hw_t *isp_hw,
  void *session);
#endif /* __ISP_HW_H__ */
