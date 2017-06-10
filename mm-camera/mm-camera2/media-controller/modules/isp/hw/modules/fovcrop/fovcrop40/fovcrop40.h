/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __FOVCROP40_H__
#define __FOVCROP40_H__

#include "camera_dbg.h"
#include "isp_event.h"
#include "isp_hw_module_ops.h"
#include "isp_pipeline.h"
#include "isp_pipeline_util.h"
#include "fovcrop40_reg.h"
#include "chromatix.h"

typedef struct {
  ISP_FOV_CropConfigCmdType reg_cmd;
  uint8_t hw_update_pending;
  uint8_t is_used; /* is this entry used or not */
  crop_window_info_t crop_window;
}isp_fov_entry_t;

typedef struct {
  int fd;
  uint32_t scaler_crop_request[ISP_PIX_PATH_MAX];
  isp_fov_entry_t fov[ISP_PIX_PATH_MAX];
  isp_pixel_window_info_t scaler_output[ISP_PIX_PATH_MAX];
  uint8_t fov_update;
  uint8_t fov_enable;
  uint8_t fov_trigger_enable;
  cam_streaming_mode_t old_streaming_mode;
  uint8_t hw_update_pending;
  isp_ops_t ops;
  isp_notify_ops_t *notify_ops;
}isp_fov_mod_t;

typedef enum {
  ENCODER,
  VIEWFINER,
}crop_type;


#endif //__FOVCROP40_H__
