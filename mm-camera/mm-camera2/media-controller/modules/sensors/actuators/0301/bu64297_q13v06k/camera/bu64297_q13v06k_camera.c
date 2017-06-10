/*============================================================================

  Copyright (c) 2015 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/

#include "af_algo_tuning.h"

static af_algo_ctrl_t af_algo_data = {
#include "bu64297_q13v06k_camera.h"
};

void *bu64297_q13v06k_camera_af_algo_open_lib(void)
{
  return &af_algo_data;
}
