/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __FOVCROP32_H__
#define __FOVCROP32_H__

#include "camera_dbg.h"
#include "isp_event.h"
#include "isp_hw_module_ops.h"
#include "isp_pipeline.h"
#include "fovcrop32_reg.h"
#include "chromatix.h"

typedef struct {
  ISP_FOV_CropConfigCmdType fov_cmd;
  int fd;
  uint8_t fov_update;
  uint8_t fov_enable;
  uint8_t fov_trigger_enable;
  cam_streaming_mode_t old_streaming_mode;
  isp_ops_t ops;
  uint8_t hw_update_pending;
  isp_notify_ops_t *notify_ops;
  crop_window_info_t output_res;  //last pix, first pix. lsst line, first line
  crop_window_info_t fov_win;
}isp_fov_mod_t;

#endif //__FOVCROP32_H__
