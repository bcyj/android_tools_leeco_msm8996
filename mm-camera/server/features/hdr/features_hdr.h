/*============================================================================

   Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

============================================================================*/

#ifndef __FEATURES_HDR_H__
#define __FEATURES_HDR_H__

#include "tgtcommon.h"
#include "sensor_interface.h"
#include "frameproc_interface.h"

typedef enum {
  HDR_PARM_ALGO_ID,
  HDR_PARM_FRM_EXP,
  HDR_PARM_HW_INFO,
  HDR_PARM_FRM_USER,
  HDR_PARM_MAX
} hdr_parm_t;

typedef struct {
  uint32_t buf_size;
  uint8_t  *buf;
  int32_t  fd;
  struct ion_allocation_data ion_alloc;
  struct ion_fd_data fd_data;
} hdr_buffer_alloc_t;

typedef struct {
  struct v4l2_buffer v4l2_buf;
  struct v4l2_plane planes[VIDEO_MAX_PLANES];
  uint8_t num_planes;
} hdr_frame_t;

typedef struct {
  char node_name[MAX_DEV_NAME_LEN];
  int main_node_fd;
  int thumb_node_fd;
  cam_format_t thumb_format;
  cam_format_t main_format;
  uint32_t thumb_image_mode;
  uint32_t main_image_mode;
  struct v4l2_format thumb_fmt;
  struct v4l2_format main_fmt;
  int buf_count;
  hdr_buffer_alloc_t thumb_buf[MAX_HDR_NUM_FRAMES];
  hdr_frame_t thumb_frames[MAX_HDR_NUM_FRAMES];
  hdr_buffer_alloc_t main_buf[MAX_HDR_NUM_FRAMES];
  hdr_frame_t main_frames[MAX_HDR_NUM_FRAMES];
} hdr_buffer_t;

typedef struct {
  uint32_t current_snapshot_count;
  uint32_t hdr_main_divert_count;
  uint32_t hdr_thumb_divert_count;
  uint32_t exp_bracketing_enable;
  uint32_t hdr_enable;
  uint32_t total_frames;
  uint32_t total_hal_frames;
  uint32_t total_mctl_frames;
  char user_exp_values[MAX_EXP_BRACKETING_LENGTH];
  float algo_exp_tbl[MAX_HDR_NUM_FRAMES];
  int frm_index;
  int skip_period;
  int leading_skip;
  hdr_buffer_t buffer_info;
  struct msm_cam_evt_divert_frame hdr_main_frame[MAX_HDR_NUM_FRAMES];
  struct msm_cam_evt_divert_frame hdr_thumb_frame[MAX_HDR_NUM_FRAMES];
} hdr_ctrl_t;

void hdr_init(hdr_ctrl_t* hdr_ctrl, char *node_name);
int32_t hdr_get(hdr_parm_t type, void *cctrl);
int32_t hdr_calc_sensor_gain_upon_sof(void *cctrl);
int32_t prepare_hdr(void *ctrlblk);
int32_t destroy_hdr(void *ctrlblk);

#endif /*__FEATURES_HDR_H__*/
