/*============================================================================

   Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __ISPIF_UTIL_H__
#define __ISPIF_UTIL_H__

#include "ispif_interface.h"
#include "camera_dbg.h"

#define ISPIF_MAX_CLIENT_NUM 4
#define ISPIF_MAX_OBJ 2


typedef struct {
  uint8_t client_idx;
  uint8_t used;
  struct msm_ispif_params ispif_params;
  uint8_t pending;
} ispif_ctrl_t;

typedef struct {
  uint32_t csid_version;
  uint8_t ref_count;
  ispif_ctrl_t ispif_ctrl[INTF_MAX];
  pthread_mutex_t session_mutex[ISPIF_MAX_OBJ];
  int8_t session_lock_owner;
} ispif_t;

typedef struct {
  uint8_t obj_idx_mask;
  uint8_t client_idx;
  uint32_t handle;
  mctl_ops_t *ops;
  uint8_t my_comp_id;
  uint32_t stream_mask;
  uint32_t channel_interface_mask;
  uint32_t channel_stream_info;
} ispif_client_t;

typedef struct {
  pthread_mutex_t mutex;
  uint32_t ispif_handle_cnt;
  ispif_client_t client[ISPIF_MAX_CLIENT_NUM];
  ispif_t ispif_obj[ISPIF_MAX_OBJ];
} ispif_comp_root_t;

int ispif_process_init(ispif_client_t *ispif_client,
  ispif_t *ispif_obj);
int ispif_set_csi_params(ispif_client_t *ispif_client,
  ispif_t *ispif_obj, ispif_set_data_t *ispif_set);
int ispif_set_format(ispif_client_t *ispif_client,
  ispif_t *ispif_obj, ispif_set_data_t *ispif_set);
int ispif_process_cfg(ispif_client_t *ispif_client,
  ispif_t *ispif_obj);
int ispif_process_start_on_frame_boundary(ispif_client_t *ispif_client,
  ispif_t *ispif_obj);
int ispif_process_stop_on_frame_boundary(ispif_client_t *ispif_client,
  ispif_t *ispif_obj);
int ispif_process_stop_immediately(ispif_client_t *ispif_client,
  ispif_t *ispif_obj);
int ispif_process_release(ispif_client_t *ispif_client, ispif_t *ispif_obj);

#endif

