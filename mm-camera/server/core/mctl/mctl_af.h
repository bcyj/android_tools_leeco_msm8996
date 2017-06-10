/*============================================================================

   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
 
   This file defines the media/module/master controller's focus interface
   in the mm-camera server. The functionalities of this modules are:

   1. process/parse af stats events
   2. control the actuator interface 

============================================================================*/

#ifndef __MCTL_AF_H__
#define __MCTL_AF_H__

#include "mctl.h"

int8_t mctl_af_start(mctl_config_ctrl_t *ctrl,
  cam_af_focusrect_t focusrect_sel);
//void mctl_parse_AF_stats_regions(mctl_config_ctrl_t *ctrl);
//void mctl_do_af(mctl_config_ctrl_t *ctrl);
void mctl_af_stop(mctl_config_ctrl_t *ctrl);

int mctl_af_proc_MSG_ID_STATS_VFE2X_AF(mctl_config_ctrl_t *cfg_ctrl,
                                 struct msm_cam_evt_msg *adsp);
int mctl_stats_proc_MSG_ID_STATS_AF(mctl_config_ctrl_t *cfg_ctrl,
                                 struct msm_cam_evt_msg *adsp);
int8_t mctl_af_get_caf_status(mctl_config_ctrl_t *ctrl);
int mctl_af_send_focus_done_event(mctl_config_ctrl_t *ctrl, int status);

#endif /* __MCTL_AF_H__ */
