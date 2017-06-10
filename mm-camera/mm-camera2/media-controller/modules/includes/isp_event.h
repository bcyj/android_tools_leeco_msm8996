/*============================================================================
Copyright (c) 2012 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#ifndef __ISP_EVENT_H__
#define __ISP_EVENT_H__

#include <linux/videodev2.h>
#include <media/msmb_camera.h>
#include <media/msmb_ispif.h>
#include <media/msmb_isp.h>
#include "cam_intf.h"
#include "mct_controller.h"
#include "modules.h"
#include "sensor_lib.h"

#if 0
typedef enum {
  CAMERA_WB_TYPE_MIN_MINUS_1,
  CAMERA_WB_TYPE_AUTO = 1,  /* This list must match aeecamera.h */
  CAMERA_WB_TYPE_CUSTOM,
  CAMERA_WB_TYPE_INCANDESCENT,
  CAMERA_WB_TYPE_FLUORESCENT,
  CAMERA_WB_TYPE_DAYLIGHT,
  CAMERA_WB_TYPE_CLOUDY_DAYLIGHT,
  CAMERA_WB_TYPE_TWILIGHT,
  CAMERA_WB_TYPE_SHADE,
  CAMERA_WB_TYPE_OFF,
  CAMERA_WB_TYPE_MAX_PLUS_1
} config3a_wb_type_t;
#endif
#endif /* __ISP_EVENT_H__ */

