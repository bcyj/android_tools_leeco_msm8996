/*============================================================================
   Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#ifndef __COMDEF_H
#define __COMDEF_H

#include <stdint.h>

typedef struct {
  uint32_t  in1_w;
  uint32_t  out1_w;
  uint32_t  in1_h;
  uint32_t  out1_h;
  uint32_t  in2_w;
  uint32_t  out2_w;
  uint32_t  in2_h;
  uint32_t  out2_h;
  uint8_t update_flag;
} common_crop_t;

typedef enum {
  FACE_DETECT_OFF,
  FACE_DETECT,
  FACE_RECOGNIZE,
  FACE_REGISTER,
  CLEAR_ALBUM,
}fd_mode_t;

#define TRUE (1==1)
#define FALSE (!TRUE)

#define MIN(x,y) (((x)<(y)) ? (x) : (y))
#define MAX(x,y) (((x)>(y)) ? (x) : (y))

#define ABS(x) (((x) < 0) ? -(x) : (x))
#define CLAMP(x, min, max) MAX (MIN (x, max), min)

#endif
