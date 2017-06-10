/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __PPROC_CAPS_H__
#define __PPROC_CAPS_H__

#include "cam_intf.h"

#define CAPS_DENOISE      (0x00000001 << 0)
#define CAPS_SCALE        (0x00000001 << 1)
#define CAPS_SHARPENING   (0x00000001 << 2)
#define CAPS_CROP         (0x00000001 << 3)
#define CAPS_ROTATION     (0x00000001 << 4)
#define CAPS_FLIP         (0x00000001 << 5)
#define CAPS_COLOR_CONV   (0x00000001 << 6)

#define ROTATION_90       (0x00000001 << 0)
#define ROTATION_180      (0x00000001 << 1)
#define ROTATION_270      (0x00000001 << 2)

typedef struct {
  float min_scale_factor;
  float max_scale_factor;
} caps_scale_t;

typedef struct {
  cam_format_t src_fmt;
  cam_format_t dest_fmt;
} caps_color_conv_t;

typedef struct {
  uint8_t h_flip;
  uint8_t v_flip;
} caps_flip_t;

typedef struct {
  int32_t min_value;
  int32_t max_value;
  int32_t def_value;
  int32_t step;
} caps_sharpness_t;

typedef struct {
  uint32_t caps_mask;

  /* supported capabilities for each capability */
  caps_scale_t      caps_scale;
  uint32_t          caps_rotation;
  caps_color_conv_t caps_color_conv;
  caps_flip_t       caps_flip;
  caps_sharpness_t  caps_sharpness;
} pproc_caps_t;

#endif
