/*
 * Copyright (c) 2014, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#ifndef __COMPONENT__
#define __COMPONENT__
#include "common.h"
extern "C" {
#include <minui.h>
}

typedef struct {
    gr_surface surface;
    int img_w;
    int img_h;
} image_t;

#endif
