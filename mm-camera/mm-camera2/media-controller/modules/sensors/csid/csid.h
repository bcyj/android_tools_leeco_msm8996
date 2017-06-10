/*============================================================================

  Copyright (c) 2012 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __CSID_H__
#define __CSID_H__

#include <media/msm_cam_sensor.h>

typedef struct {
  int                            fd;
  uint32_t                       csid_version;
  struct msm_camera_csid_params  cur_csid_params;
  struct csi_lane_params_t      *csi_lane_params;
} sensor_csid_data_t;

#endif
