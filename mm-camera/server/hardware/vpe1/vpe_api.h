/*============================================================================
   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#ifndef mm_vpe_api_h
#define mm_vpe_api_h

#include "camera.h"
#include "cam_list.h"
#include "tgtcommon.h"

#define VPE_FREE_NODE_MAX_NUM 16
#define SUBDEV_STR "/dev/v4l-subdev%d"

typedef enum {
  VPE_EVENT_DO_PP = 0,
  VPE_EVENT_ACK,
  VPE_EVENT_START,
  VPE_EVENT_STOP
} vpe_event_type_t;

typedef enum {
  VPE_STATE_NULL,
  VPE_STATE_INIT,
  VPE_STATE_DOING,
  VPE_STATE_DEINIT,
  VPE_STATE_MAX
} vpe_state_type_t;

typedef enum {
  VPE_PARM_PP_OPS = 0,
  VPE_PARM_CLK_RATE,
  VPE_PARM_PIPELINE_CFG,
  VPE_PARM_LOW_POWER_MODE,
} vpe_params_type_t;

typedef enum {
  VPE_FRAME_NULL = 0,
  VPE_FRAME_QUEUED,
  VPE_FRAME_DOING
} vpe_frame_state_t;

typedef struct {
  struct cam_list list;
  vpe_frame_state_t state;
  pp_frame_data_t frame;
} vpe_pp_node_t;

typedef enum {
  VPE_THREAD_CMD_INVALID,
  VPE_THREAD_CMD_EXIT,
} vpe_thread_cmd_t;

typedef enum {
  VPE_THREAD_CREATE_REQUESTED,
  VPE_THREAD_CREATE_FAIL,
  VPE_THREAD_CREATE_SUCCESS
} vpe_thread_status_t;

typedef struct {
  vpe_thread_cmd_t type;
  void *data;
} vpe_thread_msg_t;

typedef struct {
  int in_pipe_fds[2];
  int out_pipe_fds[2];
  vpe_thread_status_t thread_status;
  pthread_mutex_t mutex;
  pthread_cond_t cond_v;
  pthread_t pid;
} vpe_thread_data_t;

typedef struct {
 vpe_state_type_t vpe_state;
  struct timespec in_t;
  struct timespec out_t;
  mctl_ops_t ops;
  uint32_t handle;
  int clk_rate;
  mm_vpe_pipe_config_parm_type cfg;
  vpe_pp_node_t free_node[VPE_FREE_NODE_MAX_NUM];
  vpe_pp_node_t cmd_list;
  int low_power_mode;
  char dev_name[MAX_DEV_NAME_LEN];
  int dev_fd;
  vpe_thread_data_t thread_data;
} vpe_ctrl_type_t;

uint32_t vpe_interface_create(module_ops_t *ops, int sdev_number);

#endif /* mm_vpe_api_h */
