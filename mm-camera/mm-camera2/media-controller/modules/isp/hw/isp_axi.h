/*============================================================================
Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

#ifndef __ISP_AXI_H__
#define __ISP_AXI_H__

#include "isp_def.h"

#define ISP_AXI_STREAM_MAX      7


typedef enum {
  ISP_AXI_STREAM_STATE_INVALID,
  ISP_AXI_STREAM_STATE_CFG,
  ISP_AXI_STREAM_STATE_CFG_START_PENDING,
  ISP_AXI_STREAM_STATE_STARTING,
  ISP_AXI_STREAM_STATE_ACTIVE,
  ISP_AXI_STREAM_STATE_STOPPING,
  ISP_AXI_STREAM_STATE_MAX
} isp_axi_stream_state_t;

typedef struct {
  void *vaddr;
  int fd;
  struct v4l2_buffer buffer;
  struct v4l2_plane planes[VIDEO_MAX_PLANES];
} isp_axi_buf_t;

typedef struct {
  isp_axi_stream_state_t state;
  isp_hwif_output_cfg_t cfg;
  uint32_t axi_stream_handle;
  uint32_t buf_handle;
  uint32_t divert_event_id;
  uint32_t vt_enable;
} isp_axi_stream_t;

typedef struct {
  union {
    struct msm_vfe_axi_stream_request_cmd stream_request_cmd;
    struct msm_vfe_axi_stream_release_cmd stream_release_cmd;
    struct msm_isp_buf_request buf_request_cmd; /* request bufs */
    struct msm_isp_qbuf_info qbuf_cmd;     /* qbuf to kernel */
    struct msm_isp_event_data buf_event;
    struct msm_vfe_axi_stream_cfg_cmd stream_start_stop_cmd; /* start/stop */
    struct msm_vfe_input_cfg vfe_input_cfg;
    struct msm_vfe_axi_halt_cmd halt_cmd;
    struct msm_vfe_axi_reset_cmd reset_cmd;
    struct msm_vfe_axi_restart_cmd restart_cmd;
  } u;
} isp_hw_axi_work_struct_t;

typedef struct {
  int fd;
  uint32_t isp_version;
  isp_axi_stream_t streams[ISP_AXI_STREAM_MAX];
  isp_ops_t axi_ops;
  void *parent;
  isp_hw_axi_work_struct_t work_struct;
  struct msm_vfe_axi_stream_update_cmd update_cmd;
  void *buf_mgr;
  int dev_idx;
  uint8_t hw_update_pending;
} isp_axi_t;

typedef enum {
  ISP_AXI_NOTIFY_INVALID,
  ISP_AXI_NOTIFY_PIX_SOF,
  ISP_AXI_NOTIFY_WM_BUS_OVERFLOW,
  ISP_AXI_NOTIFY_MAX
} isp_axi_notify_param_t;

typedef enum {
  ISP_AXI_SET_PARAM_INVALID,
  ISP_AXI_SET_STREAM_CFG,        /* isp_hwif_output_cfg_t */
  ISP_AXI_SET_STREAM_UNCFG,      /* isp_hwif_output_cfg_t */
  ISP_AXI_SET_PARAM_FRAME_SKIP,  /* isp_param_frame_skip_pattern_t */
  ISP_AXI_SET_STREAM_UPDATE,     /* isp_hwif_output_cfg_t */
  ISP_AXI_SET_MAX_NUM,
} isp_axi_set_param_id_t;

typedef enum {
  ISP_AXI_ACTION_CODE_INVALID,
  ISP_AXI_ACTION_CODE_STREAM_START,
  ISP_AXI_ACTION_CODE_STREAM_STOP,
  ISP_AXI_ACTION_CODE_HALT,
  ISP_AXI_ACTION_CODE_RESET,
  ISP_AXI_ACTION_CODE_RESTART,
  ISP_AXI_ACTION_CODE_STREAM_DIVERT_ACK,
  ISP_AXI_ACTION_CODE_HW_UPDATE,
  ISP_AXI_ACTION_CODE_MAX_NUM,
} isp_axi_action_code_t;

int isp_axi_init(void *ctrl, void *in_params, void *parent);
int isp_axi_destroy(void *ctrl);
int isp_axi_set_params(void *ctrl, uint32_t params_id, void *in_params,
  uint32_t in_params_size);
int isp_axi_get_params(void *ctrl, uint32_t params_id, void *in_params,
  uint32_t in_params_size, void *out_params, uint32_t out_params_size);
int isp_axi_action(void *ctrl, uint32_t action_code, void *action_data,
  uint32_t action_data_size);

#endif /* __ISP_AXI_H__ */
