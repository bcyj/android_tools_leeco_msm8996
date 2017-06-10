/*============================================================================
   Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#ifndef __C2D_API_H__
#define __C2D_API_H__

#include <pthread.h>
#include "tgtcommon.h"
#include "c2d2.h"
#include "c2dExt.h"
#include "cam_list.h"

#define ENABLE_C2D_LOGGING

#define C2D_CMD_IMAGE_DRAW                        (VPE_CMD_MAX + 1)
#define C2D_CMD_IMAGE_GEOCORRECT                  (VPE_CMD_MAX + 2)
#define C2D_CMD_MAX                               (VPE_CMD_MAX + 3)

typedef enum {
  C2D_STATE_NULL,
  C2D_STATE_INIT,
  C2D_STATE_START,
  C2D_STATE_DOING,
  C2D_STATE_STOP,
  C2D_STATE_MAX
} c2d_state_type_t;

typedef enum {
  C2D_EVENT_DO_PP = 0,
  C2D_EVENT_ACK,
  C2D_EVENT_START,
  C2D_EVENT_STOP,
} c2d_event_type_t;

typedef enum {
  C2D_IMAGE_DRAW,
  C2D_IMAGE_GEOCORRECT,
  C2D_IMAGE_INVALID,
} c2d_process_mode_t;

/* Set following params in order */
typedef enum {
  /* One Time Configs */
  C2D_SET_INPUT_CFG,       /* Mandatory */
  C2D_SET_OUTPUT_CFG,      /* Mandatory */
  /* Every CMD Configs */
  C2D_SET_PROCESS_MODE,    /* Mandatory */
  C2D_SET_ROTATION_CFG,    /* Optional  */
  C2D_SET_INPUT_BUF_CFG,   /* Mandatory */
  C2D_SET_OUTPUT_BUF_CFG,  /* Mandatory */
  C2D_SET_CROP_CFG,        /* Optional  */
  C2D_SET_CB_DATA,         /* Mandatory */
} c2d_set_type_t;

typedef struct {
  void *ptr;
  C2D_STATUS (*c2dCreateSurface)(uint32_t *surface_id, uint32_t surface_bits,
    C2D_SURFACE_TYPE surface_type, void *surface_definition);
  C2D_STATUS (*c2dUpdateSurface)(uint32_t surface_id, uint32_t surface_bits,
    C2D_SURFACE_TYPE surface_type, void *surface_definition);
  C2D_STATUS (*c2dDraw)(uint32_t target_id, uint32_t target_config,
    C2D_RECT *target_scissor, uint32_t target_mask_id,
    uint32_t target_color_key, C2D_OBJECT *objects_list, uint32_t num_objects);
  C2D_STATUS (*c2dLensCorrection)(uint32_t targetSurface,
    C2D_LENSCORRECT_OBJECT *sourceObject);
  C2D_STATUS (*c2dFinish)(uint32_t target_id);
  C2D_STATUS (*c2dDestroySurface)(uint32_t surface_id);
  C2D_STATUS (*c2dMapAddr)(int mem_fd, void *hostptr, uint32_t len,
    uint32_t offset, uint32_t flags, void **gpuaddr);
  C2D_STATUS (*c2dUnMapAddr)(void *gpuaddr);
} c2d_lib_t;

typedef struct {
  struct msm_pp_frame frame;
  union {
    uint32_t sp_gAddr;
    uint32_t mp_gAddr[MAX_PLANES];
  };
} c2d_buf_t;

typedef struct {
  struct cam_list list;
  struct {
    int fd;
    uint32_t vAddr;
    uint32_t gpuAddr;
  } data;
} c2d_gpu_addr_list_t;

typedef struct {
  uint32_t x;
  uint32_t y;
  uint32_t width;
  uint32_t height;
} c2d_roi_cfg_t;

typedef struct {
  uint32_t id;
  c2d_buf_t buf;
  c2d_roi_cfg_t roi_cfg;
  C2D_YUV_SURFACE_DEF surfaceDef;
  C2D_SURFACE_BITS surface_type;
} c2d_surface_param_t;

typedef struct {
  camera_rotation_type rotation;
  c2d_surface_param_t src;
  c2d_surface_param_t dst;
} c2d_persist_params_t;

typedef struct {
  uint32_t target_id;
  uint32_t target_config;
  C2D_RECT *target_scissor;
  uint32_t target_mask_id;
  uint32_t target_color_key;
  C2D_OBJECT draw_obj;
} c2d_draw_params_t;

typedef struct {
  c2d_process_mode_t mode;
  c2d_surface_param_t src;
  c2d_surface_param_t dst;
  union {
    c2d_draw_params_t draw_params;
    C2D_LENSCORRECT_OBJECT lens_correct_obj;
  } mode_data;
} c2d_cmd_params_t;

typedef struct {
  struct cam_list list;
  c2d_cmd_params_t params;
  pp_frame_data_t cb_data;
} c2d_cmd_list_t;

typedef struct {
  uint32_t width;
  uint32_t height;
  plane_info_t plane[MAX_PLANES];
  cam_format_t cam_fmt;
} c2d_dimension_cfg_t;

typedef enum {
  C2D_CMD_DRAW,
  C2D_CMD_EXIT
} c2d_thread_msg_type;

typedef struct {
  c2d_thread_msg_type type;
  void *data;
  unsigned int len;
} c2d_thread_msg_t;

typedef struct {
  int c2d_in_pipe_fds[2];
  int c2d_out_pipe_fds[2];
  int is_c2d_thread_ready;
  int stop_requested;
  int sent_ack_to_pp;
  pthread_mutex_t mutex;
  pthread_cond_t cond_v;
  pthread_t pid;
} c2d_thread_data_t;

typedef struct {
  c2d_thread_data_t thread_data;
  c2d_state_type_t state;
  c2d_lib_t *c2d_lib;
  c2d_persist_params_t persist_params;
  c2d_cmd_list_t cmd_list;
  c2d_cmd_list_t *current_cmd;
  mctl_ops_t ops;
  c2d_gpu_addr_list_t g_list;
  /* Handle corresponding to this c2d object.
   * Can be used to lookup the correct object
   * (in case of multiple simultaneous C2D Client instances.) */
  uint32_t handle;
} c2d_ctrl_info_t;

typedef struct {
  uint32_t handle;
  c2d_ctrl_info_t obj;
} c2d_intf_t;

uint32_t c2d_interface_create(module_ops_t *c2d_ops);
int c2d_init(uint32_t handle, mctl_ops_t *ops, void *init_data);
int c2d_set_params(uint32_t handle, int type, void *param_in, void *param_out);
int c2d_get_params(uint32_t handle, int type, void *param, int param_len);
int c2d_process(uint32_t handle, int event, void *data);
int c2d_release(uint32_t handle);
void c2d_abort(uint32_t handle);

#endif /* __C2D_API_H__ */
