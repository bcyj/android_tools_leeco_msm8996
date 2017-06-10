/**********************************************************************
* Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.     *
* Qualcomm Technologies Proprietary and Confidential.                              *
**********************************************************************/

#ifndef __CAMERA_PLUGIN_INTF_H__
#define __CAMERA_PLUGIN_INTF_H__

/*===========================================================================
 *                         INCLUDE FILES
 *===========================================================================*/
#include "camera.h"
#include "tgtcommon.h"

/*===========================================================================
 *                         DATA DEFINITIONS
 *===========================================================================*/

typedef enum {
  CAM_OEM_PLUGIN_PROC_DIS_NOT_USED,
  CAM_OEM_PLUGIN_PROC_DIS_OFFSET,
  CAM_OEM_PLUGIN_PROC_DIS_MAX_NUM
} camera_plugin_proc_dis_t;

typedef enum {
  CAM_OEM_PLUGIN_PROC_3A_NOT_USED,
  CAM_OEM_PLUGIN_PROC_3A_SET_VFE_AEC_PARAMS,
  CAM_OEM_PLUGIN_PROC_3A_SET_VFE_AWB_PARMS,
  CAM_OEM_PLUGIN_PROC_3A_SET_VFE_ASD_PARMS,
  CAM_OEM_PLUGIN_PROC_3A_SET_SENSOR_DIG_GAIN,
  CAM_OEM_PLUGIN_PROC_3A_SET_FLASH_PARMS,
  CAM_OEM_PLUGIN_PROC_3A_VFE_TRIGGER_UPDATE_FOR_3A,
  CAM_OEM_PLUGIN_PROC_3A_MAX_NUM
} camera_plugin_proc_3a_t;

typedef enum {
  CAM_OEM_PLUGIN_PROC_VFE_NOT_USED,
  CAM_OEM_PLUGIN_PROC_MOD_UPDATE,
  CAM_OEM_PLUGIN_PROC_REG_UPDATE,
  CAM_OEM_PLUGIN_PROC_VFE_MAX_NUM
} camera_plugin_proc_vfe_t;

typedef enum {
  CAM_OEM_PLUGIN_PROC_ZOOM_NOT_USED,
  CAM_OEM_PLUGIN_PROC_ZOOM_SET_ZOOM_TABLE,
  CAM_OEM_PLUGIN_PROC_ZOOM_MAX_NUM
} camera_plugin_proc_zoom_t;

typedef enum {
  CAM_OEM_PLUGIN_PROC_SENSOR_NOT_USED,
  CAM_OEM_PLUGIN_PROC_SENSOR_I2C_READ,
  CAM_OEM_PLUGIN_PROC_SENSOR_I2C_WRITE,
  CAM_OEM_PLUGIN_PROC_SENSOR_GPIO_WRITE,
  CAM_OEM_PLUGIN_PROC_SENSOR_EEPROM_WRITE,
  CAM_OEM_PLUGIN_PROC_SENSOR_MAX_NUM
} camera_plugin_proc_sensor_t;


typedef enum {
  CAM_OEM_PLUGIN_SENSOR_NOTIFY_NOTUSED,
  CAM_OEM_PLUGIN_SENSOR_NOTIFY_FD,
  CAM_OEM_PLUGIN_SENSOR_NOTIFY_MAX_NUM
} camera_plugin_sensor_notify_type_t;

typedef struct {
  camera_plugin_sensor_notify_type_t type;
  union {
	int sensor_ioctl_fd;
  };
} camera_plugin_sensor_notify_t;
typedef enum {
  CAM_PLUGIN_SENSOR_GET_NOTUSED,
  CAM_PLUGIN_SENSOR_GET_DIM_INFO,
  //SENSOR_GET_CHROMATIX_PTR,
  CAM_PLUGIN_SENSOR_GET_CAMIF_CFG,
  CAM_PLUGIN_SENSOR_GET_OUTPUT_CFG,
  //SENSOR_GET_SENSOR_MODE_AEC_INFO,
  //SENSOR_GET_DIGITAL_GAIN,
  //SENSOR_GET_SENSOR_MAX_AEC_INFO,
  CAM_PLUGIN_SENSOR_GET_PREVIEW_FPS_RANGE,
  //SENSOR_GET_PENDING_FPS,
  //SENSOR_GET_CHROMATIX_TYPE,
  CAM_PLUGIN_SENSOR_GET_MAX_SUPPORTED_HFR_FPS,
  CAM_PLUGIN_SENSOR_GET_CUR_FPS,
  CAM_PLUGIN_SENSOR_GET_LENS_INFO,
  //SENSOR_GET_CSI_PARAMS,
  CAM_PLUGIN_SENSOR_GET_CUR_RES,
  CAM_PLUGIN_SENSOR_MAX_NUM
} camera_plugin_sensor_get_type_t;

typedef enum {
  /* not used */
  CAMERA_PLUGIN_FEATURE_NOT_USED,
  CAMERA_PLUGIN_FEATURE_DIS_PLUGIN,
  CAMERA_PLUGIN_FEATURE_STATS_NOTIFY,
  CAMERA_PLUGIN_FEATURE_EXT_3A_ALGORITHM,
  CAMERA_PLUGIN_FEATURE_EXT_SENSOR_CTRL,
  CAMERA_PLUGIN_FEATURE_EXT_VFE_CTRL,
  CAMERA_PLUGIN_FEATURE_EXT_ZOOM_TABLE,
  CAM_OEM_PLUGIN_FEATURE_MAX_NUM
} camera_plugin_supported_feature_t;

typedef struct {
  /* x offset */
  int32_t  x;
  /* y offset */
  int32_t  y;
  /* unique frame id */
  uint32_t frame_id;
} dis_position_type_t;

typedef struct {
  camera_plugin_proc_dis_t type;
  int status; /* 0: success, negative: error */
  union {
	  dis_position_type_t dis_offset;
  };
} camera_plugin_process_dis_t;

typedef struct {
  uint32_t module_type;
  uint32_t len;
  void *reg_update_data;
} camera_plugin_vfe_reg_update_data_t;

#define CAMERA_PLUGIN_VFE_REG_UPDATE_MAX_ENTRIES 32

typedef struct {
  int num_entries;
  camera_plugin_vfe_reg_update_data_t *entry;
} camera_plugin_vfe_reg_update_t;

typedef struct {
  camera_plugin_proc_vfe_t type;
  union {
	camera_plugin_vfe_reg_update_t reg_update;
  };
} camera_plugin_process_vfe_t;

typedef struct {
  uint32_t num_entry;
  uint32_t *camera_zoom_table;
} camera_plugin_zoom_table_t;

typedef struct {
  camera_plugin_proc_zoom_t type;
  union {
	camera_plugin_zoom_table_t zoom_table;
  };
} camera_plugin_process_zoom_t;

typedef struct {
   /* ioctl type */
  uint32_t type;
  /* data len */
  uint32_t len;
  /* data buffer */
  uint8_t *data;
} cam_oem_mctl_event_t;

typedef enum {
  /* not used */
  CAM_OEM_PLUGIN_DIS_EVENT_NOT_USED,
	/* tell oem plugin the dis offset wait option used in mctl */
  CAM_OEM_PLUGIN_DIS_EVENT_DIS_ENABLE,
	/* give oem plugin the dis frame info with margin */
  CAM_OEM_PLUGIN_DIS_EVENT_SET_FRAME_INFO,
	/* give oem plugin the VFE frame is ready */
  CAM_OEM_PLUGIN_DIS_EVENT_FRAME_READY,
	/* max num event types */
  CAM_OEM_PLUGIN_DIS_EVENT_MAX_NUM
} camera_plugin_dis_notify_type_t;

typedef enum {
  /* not used */
  CAM_OEM_PLUGIN_ISP_TIMING_NOT_USED,
  /* notify oem plugin that MCTL received STRAEMON */
  CAM_OEM_PLUGIN_ISP_TIMING_STREAMON,
  /* notify oem plugin that MCTL received STRAEMOFF */
  CAM_OEM_PLUGIN_ISP_TIMING_STREAMOFF,
  /* notify oem plugin that MCTL received RSEET ACK */
  CAM_OEM_PLUGIN_ISP_TIMING_RESET_ACK_RECEIVED,
  /* SOF ACK received */
  CAM_OEM_PLUGIN_ISP_TIMING_VFE_SOF_ACK,
  /* Start ack */
  CAM_OEM_PLUGIN_ISP_TIMING_VFE_START_ACK,
  /* Stop ack */
  CAM_OEM_PLUGIN_ISP_TIMING_VFE_STOP_ACK,
  /* max num event types */
  CAM_OEM_PLUGIN_ISP_TIMING_MAX_NUM
} camera_plugin_isp_timing_type_t;

typedef enum {
  /* frame only waits till the next frame comes */
  CAM_WAIT_DIS_OFFSET_TILL_NEXT_FRAME,
  /* frame waits till receiving dis offset */
  CAM_WAIT_DIS_OFFSET_FOREVER
} camera_plugin_dis_wait_option_t;

typedef struct {
  /* 1: enable, 0: disable */
  int enable;
  /* the image type of the frame */
  unsigned short image_type;
  /* dis offset wait option used in mctl pp module */
  camera_plugin_dis_wait_option_t dis_wait_option;
} camera_plugin_dis_enable_t;

typedef struct {
  /* the image type of this frame */
  unsigned short image_type;
  /* width */
  int width;
  /* height */
  int height;
  /* margin, 0 - no margin, 10 - 10 % margin, etc. */
  frame_margin_t margin;
  /* frame format such as NV12, NV21, etc.*/
  cam_format_t format;
  /* frame buffer offset configure */
  cam_frame_len_offset_t frame_offset;
} camera_plugin_dis_frame_info_t;

typedef struct {
  camera_plugin_dis_notify_type_t notify_type;
  union {
    camera_plugin_dis_frame_info_t frame_info;
	struct msm_pp_frame frame;
  };
} camera_plugin_dis_notify_t;

typedef struct {
  /* ioctl type */
  uint32_t type;
  /* data len */
  uint32_t len;
  /* data buffer (deep copied flat data buffer */
  uint8_t *data;
} camera_plugin_ioctl_data_t;

typedef struct {
  /* mctl's user data */
  void *user_data;
  int (*process_dis) (void *user_data,
                      camera_plugin_process_dis_t *data_dis);
  int (*process_zoom) (void *user_data,
                      camera_plugin_process_zoom_t *data_zoom);
  int (*process_vfe) (void *user_data, camera_plugin_process_vfe_t *data_vfe,
    int32_t immediate_update);
  int (*send_private_event) (void *user_data, uint32_t data_length,
    uint32_t trans_id, void *data);
  /* this is the interface for plugin to directly write to kernel */
  int (*process_sensor) (void *user_data,
  	camera_plugin_ioctl_data_t *sensor_proc_data_in, void *result);
} camera_plugin_mctl_process_ops_t;

#define MM_CAMERA_PLUGIN_RESERVED_BASE 0x00FFFFFF
typedef enum {
  MM_CAM_PLUGIN_PRIVATE_IOCTL_EVENT_SIMULTANEOUS_CAM =
    (MM_CAMERA_PLUGIN_RESERVED_BASE + 1),
  MM_CAM_PLUGIN_PRIVATE_IOCTL_EVENT_DIFF_RDI_PIX_CLK =
    (MM_CAMERA_PLUGIN_RESERVED_BASE + 2),
  MM_CAM_PLUGIN_PRIVATE_IOCTL_EVENT_MAX_NUM =
    (MM_CAMERA_PLUGIN_RESERVED_BASE + 3)
} camera_plugin_private_ioctl_reserved_event_t;

#define MM_CAMERA_PLUGIN_PRIVATE_IOCTL_EVENT_TYPE_DUAL_CAMERA 0
typedef struct {
  uint32_t client_handle;
  /* mctl calls this func to close the oem plugin */
  int (*client_init) (void *plugin_handle, uint32_t client_handle,
					   camera_plugin_mctl_process_ops_t *mctl_ops);
  void (*client_destroy) (void *plugin_handle, uint32_t client_handle);
  int (*stats_notify_to_plugin) (void *plugin_handle,
                       uint32_t client_handle,
                       int stats_type, void *stats_data);
  int (*dis_notify_to_plugin) (void *plugin_handle,
                       uint32_t client_handle,
                       camera_plugin_dis_notify_t *payload);
  int (*isp_timing_notify_to_plugin) (void *plugin_handle,
                       uint32_t client_handle,
                       camera_plugin_isp_timing_type_t timing_type, void *data);
  int (*get_sensor_parm_from_plugin) (void *plugin_handle,
                       uint32_t client_handle, uint32_t type,
                       void *params, int params_len); /* sensor_data_t */
  int (*sensor_notify_to_plugin) (void *plugin_handle,
                       uint32_t client_handle,
                       camera_plugin_sensor_notify_t *sensor_notify);
  int (*private_ioctl_to_plugin) (void *plugin_handle,
                       uint32_t client_handle,
                       camera_plugin_ioctl_data_t *data, int *status);
  int (*query_supported_feature) (void *plugin_handle,
                       uint32_t client_handle,
                       camera_plugin_supported_feature_t feature_type,
                       uint8_t *is_support);
  int (*grant_pix_interface) (void *plugin_handle,
                       uint32_t client_handle, uint8_t *grant_pix,
                       uint8_t *concurrent_enabled,
                       uint8_t *default_vfe_id,
                       struct msm_camsensor_info *sensor_info,
                       uint8_t max_hw_num);
  int (*diff_clk_for_rdi_camera) (void *plugin_handle,
                       uint32_t client_handle, uint8_t *use_diff_clock);
} camera_plugin_client_ops_t;

/* some terminology
   indication: to plugin */
typedef struct {
  /* oem plugin handle */
  void *handle;
  /* mctl calls this func to destroy/release the oem plugin */
  void (*destroy) (void *handle);
  /* mctl calls this func to open one oem plugin.
   * In case of open failure <0 value returned */
  int (*client_open) (void *handle, camera_plugin_client_ops_t *client_ops);
} camera_plugin_ops_t;

/*typedef int (*camera_plugin_create_func) (camera_plugin_ops_t *camera_ops);*/

#endif /* __CAMERA_PLUGIN_INTF_H__ */
