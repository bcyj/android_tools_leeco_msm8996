/*============================================================================
Copyright (c) 2013-2015 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#ifndef __ISP_DEF_H__
#define __ISP_DEF_H__

#include <media/msmb_isp.h>
#include <media/msmb_ispif.h>
#include "modules.h"

#define CDBG_DUMP(fmt, args...) do{}while(0)

/* bit 16 - 31 used for isp version
 * bit 0 -15 used for isp revision */
#define GET_ISP_MAIN_VERSION(ver) ((ver & 0xFFFF0000) >> 16)
#define GET_ISP_SUB_VERSION(ver) (ver & 0xFFFF)
#define SET_ISP_VERSION(main_ver, sub_ver) \
(((main_ver & 0xFFFF) << 16) | (sub_ver & 0xFFFF))

#define Q14   0x00004000
#define Q13   0x00002000
#define Q12   0x00001000
#define Q12_2 0x00002000
#define Q12_4 0x00004000
#define Q11   0x00000800
#define Q10   0x00000400
#define Q8    0x00000100
#define Q7    0x00000080
#define Q6    0x00000040
#define Q4    0x00000010

#define ISP_MAX_NUM                 2
#define ISP_MAX_SESSIONS            4
#define ISP_MAX_STREAMS             8
#define ISP_SINK_PORT_CFG_ENTRY_MAX 2
#define MAX_STREAM_PLANES           8
#define ISP_SUBDEV_NAME_LEN         32
#define ISP_SD_NODE_ID_MAX_NUM      VFE_MAX
#define ISP_METADATA_GAMMATBL_SIZE  64
#define ISP_MAX_NUM_SCALER          2
#define ISP_LOW_LIGHT_LUX_IDX       300
#define ISP_UV_SUB_SWITCH_CNT       6

#define ISP_PADDING(width, pad_bytes) (((width)+pad_bytes-1)&~(pad_bytes-1))
/*#define ISPIF_DEBUG */
/*#define ISPIF_UTIL_DEBUG */
/*#define MODULE_ISPIF_DEBUG */
/*#define PORT_ISPIF_DEBUG */

/*Macros to enable logging */
/*#define MODULE_ISP_DEBUG 1 */
/*#define PORT_ISP_DEBUG 1 */
/*#define ISP_DEBUG 1 */
/*#define ISP_UTIL_DEBUG 1 */

typedef enum {
  MARGIN_P_4 = 4,
  MARGIN_P_5 = 5,
  MARGIN_P_6 = 6,
  MARGIN_P_8 = 8,
  MARGIN_P_10 = 10,
  MARGIN_INVALID
} isp_dis__margin_t;

typedef enum {
  ISP_INTF_PIX,
  ISP_INTF_RDI0,
  ISP_INTF_RDI1,
  ISP_INTF_RDI2,
  ISP_INTF_MAX
} isp_intf_type_t;

typedef enum {
  ISP_PIX_PATH_ENCODER,    /* encoder type */
  ISP_PIX_PATH_VIEWFINDER, /* preview type */
  ISP_PIX_PATH_MAX,        /* max paths */
} isp_pix_scaler_path_t;

typedef enum {
  ISP_RDI_CH_0,
  ISP_RDI_CH_1,
  ISP_RDI_CH_2,
  ISP_RDI_CH_MAX
} isp_rdi_ch_t;

typedef enum {
  ISP_MODE_2D = 0,
  ISP_MODE_3D = 1,
  ISP_MODE_MAX
} isp_mode_t;

typedef struct {
  uint32_t stats_mask; // to set stats_mask |= 1 << MSM_ISP_STATS_<type>
                       // to reset stats_mask &= 1 << MSM_ISP_STATS_<type>
  aec_config_t aec_config; /*include ae or bg*/
  awb_config_t awb_config;
  af_config_t  af_config;
}isp_stats_config_t;

typedef struct {
  int enabled;
  unsigned long duration;
} camif_strobe_info_t;

typedef struct {
  uint32_t first_pixel;
  uint32_t last_pixel;
  uint32_t first_line;
  uint32_t last_line;
}isp_pixel_line_info_t;

typedef struct {
  uint32_t width;
  uint32_t height;
  float scaling_factor;
}isp_pixel_window_info_t;

typedef struct {
  int width;
  int height;
  isp_pixel_line_info_t pix_line;
} camera_size_t;

typedef struct {
  /* from isp */
  isp_pixel_line_info_t pix_line; /* first line, pix, last line, pix */
  uint32_t x;                     /* x coordinate */
  uint32_t y;                     /* y coordinate */
  uint32_t crop_factor;           /* Q12 crop factor */
}isp_zoom_info_t;

typedef struct {
  uint8_t num_planes;
  uint32_t strides[MAX_STREAM_PLANES];
  uint32_t scanline[MAX_STREAM_PLANES];
} isp_plane_info_t;

typedef struct {
  uint32_t session_id;
  uint32_t stream_id;
  uint32_t width;
  uint32_t height;
  cam_format_t fmt;
  isp_plane_info_t planes;
  int num_cids;
  uint8_t cid[SENSOR_CID_CH_MAX];
  cam_streaming_mode_t streaming_mode;
  cam_stream_type_t stream_type;
  uint8_t num_burst;
  uint8_t sensor_skip_cnt;
  uint32_t hfr_mode;
  uint8_t frame_base;
} isp_stream_param_t;

typedef enum{
  ISP_REG_UPDATE_STATE_NOT_REQUESTED = 0,
  ISP_REG_UPDATE_STATE_RECEIVED,
  ISP_REG_UPDATE_STATE_PENDING,
  ISP_REG_UPDATE_STATE_CONSUMED
} isp_reg_update_state_t;

typedef struct{
  unsigned int last_reg_update_frame_id;
  volatile isp_reg_update_state_t  reg_update_state;
} isp_reg_update_info_t;

typedef struct {
  int ion_fd;
  int vt_enable;
  isp_stream_param_t stream_param;
  ispif_out_info_t ispif_out_info;
  isp_out_info_t isp_out_info;
  isp_intf_type_t isp_output_interface; /* pix/RDI0/RDI1/RDI2 */
  enum msm_vfe_axi_stream_src axi_path;
  uint32_t frame_skip_period; /* frame skip period */
  /* skip how many frames every frame_skip_period */
  enum msm_vfe_frame_skip_pattern frame_skip_pattern;
  boolean need_divert;
  boolean need_uv_subsample;
  boolean use_native_buf;
  uint32_t burst_len;
} isp_hwif_output_cfg_t;

typedef struct {
  sensor_out_info_t sensor_out_info;
  ispif_out_info_t ispif_out_info;
  uint8_t is_bayer_sensor;
  cam_format_t sensor_output_fmt;
  enum msm_vfe_axi_stream_src axi_path;
  cam_hfr_mode_t hfr_mode;
} isp_pix_camif_cfg_t;

typedef struct {
  int session_id;
  int stream_id;
} isp_stream_id_t;

typedef struct {
  uint32_t max_resolution;
  uint32_t id;
  uint32_t ver;
  uint32_t max_pix_clk;
  uint32_t max_scaler_out_width;
  uint32_t max_scaler_out_height;
  float longshot_max_bandwidth;
  float zsl_max_bandwidth;
  boolean use_pix_for_SOC;
} isp_info_t;

typedef struct {
  int num_pix; /* num pix interfaces supported */
  int num_rdi; /* num rdi interfaces supported */
  int num_wms; /* num write masters */
  int num_register;
  uint32_t stats_mask;
  isp_info_t isp_info;
  uint32_t hw_ver;
} isp_hw_cap_t;

#define ISP_MAX_IMG_BUF        CAM_MAX_NUM_BUFS_PER_STREAM

typedef struct {
  void *vaddr;
  int fd;
  struct v4l2_buffer buffer;
  struct v4l2_plane planes[VIDEO_MAX_PLANES];
  unsigned long addr[VIDEO_MAX_PLANES];
  struct ion_allocation_data ion_alloc[VIDEO_MAX_PLANES];
  struct ion_fd_data fd_data[VIDEO_MAX_PLANES];
  int cached;
  boolean is_reg; /*flag to indicate buffer is already registered with kernel*/
} isp_frame_buffer_t;

typedef struct {
  uint32_t session_id;
  uint32_t stream_id;
  boolean use_native_buf;
  int num_bufs;
  isp_frame_buffer_t *buf;
} isp_mapped_buf_params_t;

typedef struct {
  /* to isp */
  uint8_t num;
  enum msm_stats_enum_type *stats_types;
}isp_stats_enable_t;

typedef struct {
  int num_streams;
  uint32_t session_id;
  uint32_t *stream_ids;
  boolean wait_for_sof; /* if set one more frame delay is added */
  uint8_t fast_aec_mode;
  uint32_t frame_id;
  boolean stop_immediately;
} start_stop_stream_t;

typedef enum {
  ISP_EFFECT_CONTRAST,
  ISP_EFFECT_HUE,
  ISP_EFFECT_SATURATION,
  ISP_EFFECT_SPECIAL,
  ISP_EFFECT_MAX
}isp_effect_type_t;

typedef struct {
  /* to isp */
  uint32_t effect_type_mask; /* bit of isp_effect_type_t */
  int32_t contrast;
  float hue;
  float saturation;
  cam_effect_mode_type spl_effect;
}isp_effects_params_t;

typedef struct {
  /* to isp */
  float asd_soft_focus_dgr;
  float bst_soft_focus_dgr;
  float ui_sharp_ctrl_factor;
  float downscale_factor;
  float portrait_severity;
}isp_sharpness_info_t;

typedef enum {
  CAMERA_FLASH_NONE,
  CAMERA_FLASH_LED,
  CAMERA_FLASH_STROBE,
}camera_flash_type;

typedef struct {
  /* to ISP */
  camera_flash_type flash_type;
  float sensitivity_led_off;
  float sensitivity_led_low;
  float sensitivity_led_hi;
  uint32_t strobe_duration;
}isp_flash_params_t;

typedef struct {
  uint32_t session_id;
  uint32_t stream_id;
  enum msm_vfe_frame_skip_pattern pattern;
} isp_param_frame_skip_pattern_t;

typedef struct {
  cam_hfr_mode_t hfr_mode;  /* HFR mode */
  uint32_t stream_id;       /* which stream to apply HFR */
  boolean pp_do_frame_skip;
} isp_hfr_param_t;

typedef struct {
  int32_t dis_enable;
  uint32_t stream_id; /* stream id for dis */
  uint32_t width;
  uint32_t height;
} isp_dis_param_t;

typedef struct {
  uint32_t session_id;
  uint32_t col_num;
  uint32_t raw_num;
} isp_cs_rs_config_t;

typedef struct {
  uint32_t x; /* left */
  uint32_t y; /* top */
  uint32_t crop_out_x; /* width */
  uint32_t crop_out_y; /* height */
}crop_window_info_t;

#define ISP_ZOOM_MAX_ENTRY_NUM     2
typedef struct {
  uint32_t stream_id;
  cam_dimension_t dim;
  crop_window_info_t crop_win;
  isp_pixel_line_info_t roi_map_info;
} isp_hw_zoom_param_entry_t;

typedef struct {
  int num;
  isp_hw_zoom_param_entry_t entry[ISP_ZOOM_MAX_ENTRY_NUM];
  uint32_t frame_id;
  int session_id;
} isp_hw_zoom_param_t;

typedef struct {
  uint32_t session_id;          /* in param */
  uint32_t crop_factor;         /* in param */
  isp_hw_zoom_param_t hw_zoom_parm; /* out param */
} isp_hw_set_crop_factor_t;

typedef struct {
  uint32_t stream_id;
  uint32_t in_width;
  uint32_t out_width;
  uint32_t in_height;
  uint32_t out_height;
  isp_pixel_line_info_t roi_map_info;
} isp_zoom_scaling_param_entry_t;

typedef struct {
  int num;
  isp_zoom_scaling_param_entry_t entry[ISP_ZOOM_MAX_ENTRY_NUM];
} isp_zoom_scaling_param_t;

typedef struct {
  uint32_t session_id;
  uint32_t is_hw_updating;
  uint32_t dev_idx;
} isp_hw_updating_notify_t;

typedef struct {
  struct msm_isp_event_data *isp_event_data;
  boolean ack_flag;
  boolean is_buf_dirty;
  boolean use_native_buf;
} isp_frame_divert_notify_t;

typedef struct {
  boolean present;
  int32_t zoom_val;
} isp_hw_param_zoom_t;
typedef struct {
  boolean present;
  int32_t effect;
} isp_hw_param_effect_t;
typedef struct {
  boolean present;
  int32_t contrast;
} isp_hw_param_contrast_t;
typedef struct {
  boolean present;
  cam_scene_mode_type bestshot;
} isp_hw_param_bestshot_t;
typedef struct {
  boolean present;
  int32_t sce_factor;
} isp_hw_param_sce_factor_t;
typedef struct {
  boolean present;
  awb_update_t awb_stats_update;
} isp_hw_param_awb_stats_update_t;
typedef struct {
  boolean present;
  aec_update_t aec_stats_update;
} isp_hw_param_aec_stats_update_t;
typedef struct {
  boolean present;
  int32_t saturation;
} isp_hw_param_saturation_t;

typedef struct {
  boolean has_params;
  isp_hw_param_zoom_t zoom;
  isp_hw_param_effect_t effect;
  isp_hw_param_contrast_t contrast;
  /* isp_hw_param_bestshot_t bestshot; discuss with Marvin. This should not be used */
  isp_hw_param_sce_factor_t sce_factor;
  isp_hw_param_saturation_t saturation;
  isp_hw_param_awb_stats_update_t awb_stats_update;
  isp_hw_param_aec_stats_update_t aec_stats_update;
} isp_hw_params_t;

typedef struct {
  boolean in_service;
  boolean hw_update_pending;
  /*int pending_hw_count;*/
  /*isp_hw_params_t pending_params;*/
  isp_hw_params_t new_params;
} isp_buffered_hw_params_t;

typedef struct {
  uint32_t rgnHOffset;
  uint32_t rgnVOffset;
  uint32_t rgnWidth;
  uint32_t rgnHeight;
  uint32_t rgnHNum;
  uint32_t rgnVNum;
  uint16_t rMax;
  uint16_t grMax;
  uint16_t bMax;
  uint16_t gbMax;
}isp_zoom_roi_params_t;

#endif /* __ISP_DEF_H__ */
