/*============================================================================
   Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#ifndef __MCTL_DIVERT_H__
#define __MCTL_DIVERT_H__

#include "camera.h"
#include "cam_list.h"
#include "mctl.h"
#include "mctl_pp.h"

int mctl_divert_set_key(void *cctrl, frame_proc_key_t fp_key);
int8_t mctl_divert_frame(void *parm1, void *parm2);
void mctl_divert_frame_done_check_eztune_fd(
  mctl_config_ctrl_t *ctrl, struct msm_cam_evt_divert_frame *div_frame);
/* Domain socket buffer helper functions */
int create_camfd_receive_socket(mctl_domain_socket_t *socket_info, int cameraId);
int mctl_divert_socket_recvmsg(int fd, mctl_config_ctrl_t *ctrl);
void mctl_divert_socket_get_buf_data(mctl_config_ctrl_t *ctrl);
void close_camfd_receive_socket(mctl_domain_socket_t *socket_info, int cameraId);

#endif /* __MCTL_PP_H__ */
