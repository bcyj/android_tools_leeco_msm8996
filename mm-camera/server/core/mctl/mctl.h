/*============================================================================
   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

#ifndef __MCTL_H
#define __MCTL_H

#include <pthread.h>
#include "cam_list.h"
#include "camera.h"

#include "chromatix.h"
#include "stats_proc_interface.h"
#include "vfe_interface.h"
#include "camif_interface.h"
#include "axi_interface.h"
#include "sensor_interface.h"
#include "actuator_interface.h"
#include "eeprom_interface.h"
#include "flash_interface.h"
#include "effects.h"
#include "zoom.h"
#include "bestshot.h"
#include "stereocam.h"
#include "features_hdr.h"
#include "live_snapshot.h"
#include "csi_interface.h"
#include "ispif_interface.h"
#include "config_interface.h"
#include "camera_plugin_intf.h"

#include "mctl_pp.h"
#include "mctl_stats.h"
#include "af_tuning.h"

#ifdef FEATURE_GYRO
#include "dsps_hw_interface.h"
#include "dsps_hw.h"
#endif

#define MCTL_CONFIG_MAX_LEN       512*5 /* 2.5k buf */

#define MSM_MAX_DEV_INST          16

#define MCTL_AEC_NUM_REGIONS      256
#define MCTL_DEFAULT_DIG_GAIN_ADJ 1.0
#define MCTL_DEFAULT_DIG_GAIN     1.0

#define V4L2_DEV_STRAEMON_BIT_P        (1<<0)
#define V4L2_DEV_STRAEMON_BIT_V        (1<<1)
#define V4L2_DEV_STRAEMON_BIT_S        (1<<2)
#define V4L2_DEV_STRAEMON_BIT_T        (1<<3)
#define V4L2_DEV_STREAMON_BIT_R        (1<<4)
#define V4L2_DEV_STREAMON_BIT_RDI      (1<<5)
#define V4L2_DEV_STREAMON_BIT_AEC      (1<<6)
#define V4L2_DEV_STREAMON_BIT_AWB      (1<<7)
#define V4L2_DEV_STREAMON_BIT_AF       (1<<8)
#define V4L2_DEV_STREAMON_BIT_IHIST    (1<<9)
#define V4L2_DEV_STREAMON_BIT_ISP_OUT1 (1<<10)
#define V4L2_DEV_STREAMON_BIT_ISP_OUT2 (1<<11)
#define V4L2_DEV_STREAMON_BIT_RDI1     (1<<12)
#define V4L2_DEV_STREAMON_BIT_RDI2     (1<<13)

#define V4L2_MAX_VIDEO    4

#define MCTL_YUV_SENSOR_VFE_HW_SEL_RULE_FIRST_IN_FIRST_SERVE    0
#define MCTL_YUV_SENSOR_VFE_HW_SEL_ALWAYS_RDI                   1

#define MCTL_MAX_PARALLEL_PP_PIPES        3

/* definition of camera error state */
typedef enum {
  ERROR_NONE    = (0<<0),
  ERROR_CAMIF   = (1<<0),
  ERROR_INVALID = 0xFFFFFFFF
} error_state_t;

typedef enum {
  MCTL_CTRL_STATE_NULL = 0,      /* initial state */
  MCTL_CTRL_STATE_INITED,        /* mctl node opened */
  MCTL_CTRL_STATE_INITED_SENSOR, /* mctl sensor initted and thraed running */
  MCTL_CTRL_STATE_COMP_INIT_ERR, /* components open/init error */
  MCTL_CTRL_STATE_SERVICE_READY, /* MCTL has components ready */
  MCTL_CTRL_STATE_MAX_NUM
} mctl_ctrl_state_enum_t;

typedef enum {
  MCTL_THREAD_OPEN_REQUESTED,
  MCTL_THREAD_OPEN_FAILED,
  MCTL_THREAD_RUNNING,
  MCTL_THREAD_CLOSED,
  MCTL_THREAD_INVALID
} mctl_thread_status;

typedef enum {
  MCTL_POLL_SLOT_CONFIG_FD,
  MCTL_POLL_SLOT_DAEMON_PIPE,
  MCTL_POLL_SLOT_EZTUNE_PIPE_READ_FD,
  MCTL_POLL_SLOT_EZTUNE_CLIENT_FD,
  MCTL_POLL_SLOT_EZTUNE_PREV_PIPE_FD,
  MCTL_POLL_SLOT_EZTUNE_PREV_CLIENT_FD,
  MCTL_POLL_SLOT_DOMAIN_SOCKET_FD,
  MCTL_POLL_SLOT_PP_NODE_FD,
  MCTL_POLL_SLOT_MAX
} mctl_poll_slot_idx_t;

typedef struct {
  cam_stream_info_def_t user[MSM_MAX_DEV_INST];
  cam_stream_info_def_t mctl[MSM_MAX_DEV_INST];
  cam_stream_info_def_t stats[MSM_MAX_DEV_INST];
} cam_stream_info_t;

typedef struct {
  mm_camera_wdn_start_type wdn_start;
} mctl_pp_wdn_status_t;

typedef struct {
  mm_camera_hdr_start_type hdr_start;
} mctl_pp_hdr_status_t;

typedef struct {
  int socket_fd;
  cam_sock_packet_t buf_packet;
} mctl_domain_socket_t;

typedef struct {
  int magic_num;             /* unique per video node */
  uint8_t use_pix_interface; /*if TRUE need VFE pix. If FALSE RDI only*/
  uint8_t default_vfe_idx;   /* default VFE idx from camera plugin */
  uint32_t op_mode;          /* set it when getting S_CTRL of DEV_OP */
  uint32_t streamon_mask;    /* set it when receiving stream on */
  uint32_t evt_mask;
  mctl_pp_buf_info_t mctl_buf_info[MSM_V4L2_EXT_CAPTURE_MODE_MAX];
  mctl_pp_buf_info_t user_buf_info[MSM_V4L2_EXT_CAPTURE_MODE_MAX];
  mctl_pp_buf_info_t user_hist_buf_info;
  /* Stores the information for a particular stream when user
   * sets the stream parameters. This will be used when the user
   * turns on the particular stream.*/
  cam_stream_info_t      strm_info;
  cam_format_t           prev_format;
  cam_format_t           enc_format;
  cam_format_t           thumb_format;
  cam_format_t           main_img_format;
  cam_format_t           rdi0_format;
  cam_format_t           rdi1_format;
  cam_pad_format_t       padding_format;
  mctl_pp_wdn_status_t   wdn_status;
  mctl_pp_hdr_status_t   hdr_status;
  mctl_domain_socket_t   socket_info;
  /* pp_dev_name is used for mctl video streaming */
  char mctl_pp_dev_name[MAX_DEV_NAME_LEN];
  int                    def_pp_idx;
  uint32_t               user_bundle_mask;
  uint32_t               streamon_bundle_mask;
  uint32_t               streamoff_bundle_mask;
} v4l2_video_ctrl;

typedef struct {
  int is_auto_wb;
  int wb_temperature;
  int brightness;
  int contrast;
  int ev_num;
  int saturation;
} v4l2_ctrl_t;

/* Config child communication pipe end characteristics */
typedef enum {
  READ_END,
  WRITE_END,
  CONFG_OPS_MAX,
}conf_child_pipe_ops;

/* Config child communication pipe Behaviour */
typedef enum {
  PIPE_IN,
  PIPE_OUT,
  CONF_PIPE_MAX,
}conf_child_comm_pipe;

/* Config Child Thread FDs*/
typedef enum {
  STEREO_ANALYSIS,
  STEREO_DISPATCH,
  CONF_CHILD_MAX,
}conf_child_threads;

typedef struct {
  stats_buffers_type_t *p_buf_data;
  stats_proc_interface_t intf;
  uint8_t sof_update_needed;
  float digital_gain;
  int RS_stats_ready;
  int CS_stats_ready;
  vfe_stats_output_t vfe_stats_out;
}stats_proc_ctrl_t;

typedef struct {
  sensor_output_t sensor_output;
  sensor_lens_info_t lens_info;
  int flash_enabled;
  /* Sensor's full size dimensions */
  uint16_t full_size_width;
  uint16_t full_size_height;
  /* Sensor's qtr size dimensions */
  uint16_t qtr_size_width;
  uint16_t qtr_size_height;
} sensor_data_ctrl_t;

typedef struct {
  uint8_t enable_histogram;
  uint32_t vfe_Ihist_data[256];
} vfe_data_t;

typedef struct {
  uint8_t af_enable;
  uint8_t af_cont_enable;
  cam_parm_info_t parm_focusrect;
  roi_info_t roiInfo;
  roi_info_t fd_roiInfo;
} af_data_ctrl_t;

enum {
  STRM_INFO_USER,
  STRM_INFO_MCTL,
  STRM_INFO_STATS,
  STRM_INFO_INVALID
};

typedef struct {
  void *ptr;
  uint32_t (*frame_proc_interface_create)(void);
  int (*frame_proc_init)(uint32_t handle,
    frame_proc_interface_input_t *init_data);
  int (*frame_proc_process)(uint32_t handle,
    frame_proc_interface_t *frame_proc_intf);
  void (*frame_proc_abort)(void);
  int (*frame_proc_set_params)(uint32_t handle,
    frame_proc_set_t *frame_proc_set, frame_proc_interface_t *frame_proc_intf);
  int (*frame_proc_destroy)(uint32_t handle);
}frame_proc_lib_t;

typedef struct {
  uint32_t handle;
  frame_proc_lib_t lib;
  frame_proc_interface_t intf;
}frame_proc_ctrl_t;

typedef struct {
  int enable;
  QISPInfo_t isp_info;
}mobicat_info_t;

typedef struct mctl_ctrl_t m_ctrl_t;

typedef struct  {
  int                    camfd;
  config_state_t         state;
  camera_mode_t          current_mode;
  camera_op_mode_t       ops_mode;
  uint8_t                vfe_reg_updated;
  uint32_t               support_3d;
  int8_t                 camera_id;
  int                    denoise_enable;
  int                    is_fd_on;
  int                    fd_mode;
  int                    num_fd;
  struct v4l2_crop       crop_info;
  /* subsidiary control blocks */
  float ui_sharp_ctrl_factor;
  effects_ctrl_t         effectCtrl;
  bestshot_ctrl_t        bestshotCtrl;
  hdr_ctrl_t             hdrCtrl;
  liveshot_ctrl_t        liveshotCtrl;
  zoom_ctrl_t            zoomCtrl;
  stereo_ctrl_t          stereoCtrl;
  mctl_pp_t              mctl_pp_ctrl[MCTL_MAX_PARALLEL_PP_PIPES];
  mctl_pp_node_obj_t     pp_node;
  int                    zoom_done_pending;
  sensor_data_ctrl_t     sensorCtrl;
  af_data_ctrl_t         afCtrl;
  sensor_mode_t          sensor_op_mode;

  uint32_t               comp_mask;
  module_ops_t           comp_ops[MCTL_COMPID_MAX];

  struct msm_ver_num_info vfe_ver;
  stats_proc_ctrl_t      stats_proc_ctrl;
  vfe_stats_rs_cs_params rs_cs_params;
  frame_proc_ctrl_t      frame_proc_ctrl;

  /* chromatix */
  chromatix_parms_type   *chromatix_ptr;
  vfe_op_mode_t          vfeMode;
  vfe_data_t             vfeData;
  uint32_t               videoHint;

  uint8_t                is_eeprom;
  af_tune_parms_t        *af_tune_ptr;

   /* dimesions */
  cam_ctrl_dimension_t   dimInfo;
  dis_ctrl_info_t        video_dis;

  struct msm_ctrl_cmd    *pendingCtrlCmd;
  struct msm_ctrl_cmd    *pendingPrepSnapCtrlCmd;
  uint32_t               ppkey;
  v4l2_ctrl_t            v4l2Ctrl;
  v4l2_video_ctrl        video_ctrl;
  int                    ion_dev_fd;
  uint8_t                eztune_preview_flag;
  current_output_info_t  curr_output_info;
  uint8_t                sensor_stream_fullsize;
  uint8_t                concurrent_enabled;
  uint32_t               channel_interface_mask;
  uint32_t               channel_stream_info;
  mctl_ops_t             ops;
  camera_hfr_mode_t      hfr_mode;
  int                    enableLowPowerMode;
  uint32_t               reconfig_vfe;
  mctl_ctrl_state_enum_t mctl_state;
  struct msm_camsensor_info cam_sensor_info;
  uint32_t               how_to_sel_vfe;
  camera_mctl_client_ops_t *p_client_ops;
  cam_3a_conv_info_t     conv_3a_info;
  uint8_t                conv_3a_info_set;
  uint8_t                preview_hfr;
  struct config_thread_arguments cfg_arg;
  struct config_interface_t *config_intf;
  camera_plugin_ops_t *camera_plugin_ops;
  camera_plugin_client_ops_t plugin_client_ops;
  camera_plugin_mctl_process_ops_t mctl_ops_for_plugin;
  m_ctrl_t                   *mctl_ctrl;
  mobicat_info_t         mobicat_info;
  int                    current_target;
} mctl_config_ctrl_t;

typedef struct {
  int ops_mode;
  mctl_config_ctrl_t *ctrl;
} pp_parms;

struct mctl_ctrl_t {
  int video_fd;
  struct config_thread_arguments cfg_arg;
  pthread_t cam_mctl_thread_id;
  pthread_cond_t cam_mctl_thread_ready_cond;
  pthread_mutex_t cam_mctl_thread_ready_mutex;
  mctl_thread_status cam_mctl_thread_status;
  mctl_config_ctrl_t* p_cfg_ctrl;
};

/* Following function loads all components.
 * It runs in qcam server context.
 */
int mctl_load_comps();
/* Following function unloads all components.
 * It runs in qcam server context.
 */
int mctl_unload_comps();

/* Following function creats a media controller object.
 * It runs in qcam server context.
 */
m_ctrl_t *mctl_create(struct config_thread_arguments* config_arg);
/* Following function deletes a media controller from memory.
 * It runs in qcam server context
 */
void mctl_delete(m_ctrl_t* pme);

/* Following function initializes mctl_pp thread, zoom, hdr etc.
 * It runs in mctl thread context.
 */
int mctl_init(m_ctrl_t* pme);
/* Following function is opposite of mctl_init.
 * It runs in mctl thread context.
 */
int mctl_deinit(m_ctrl_t* pme);

/* Following function is a superset of mctl_deinit. It undos the tasks
 * performed after successful mctl thread creation as well as mctl_deinit.
 * It runs in mctl thread context.
 */
int mctl_release(m_ctrl_t* pme);
/* this function process the commands for user app*/
int mctl_proc_v4l2_request(m_ctrl_t* pme, void *parm);
/* this function process the evt and stat messages from kernel*/
int mctl_proc_event_message(m_ctrl_t* pme, void *parm);
int mctl_open_and_init_comps(mctl_config_ctrl_t *ctrl);
/* this function is to send the ctrl cmd to done to kernel or client */
int mctl_send_ctrl_cmd_done(mctl_config_ctrl_t *ctrl,
  struct msm_ctrl_cmd *ctrlCmd, int client_only);
void mctl_stats_init_ops(void *cctrl);
void mctl_stats_deinit_ops(void *cctrl);
int mctl_init_sensor(mctl_config_ctrl_t *ctrl);
int mctl_init_camera_plugin(void* pme, camera_plugin_ops_t *camera_plugin_ops);
uint32_t config_get_inst_handle(cam_stream_info_t *strm_info,
  int search, uint32_t image_mode);
void mctl_timing_notify_to_cam_plugin(mctl_config_ctrl_t *ctrl,
  camera_plugin_isp_timing_type_t timing, void *data);
int mctl_find_resolution_image_mode (mctl_config_ctrl_t *ctrl,
	  uint32_t image_mode, uint8_t user_buf, camera_resolution_t *res);
#endif /*__MCTL_H*/

