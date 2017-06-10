/*============================================================================

  Copyright (c) 2012 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __CSIPHY_H__
#define __CSIPHY_H__

#include <media/msm_cam_sensor.h>

typedef struct {
  int                              fd;
  struct msm_camera_csiphy_params  cur_csiphy_params;
  struct csi_lane_params_t        *csi_lane_params;
} sensor_csiphy_data_t;

#endif
