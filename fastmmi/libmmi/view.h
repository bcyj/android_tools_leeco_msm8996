/*
 * Copyright (c) 2014, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#ifndef __VIEW__
#define __VIEW__
#include "common.h"
extern "C" {
#include <minui.h>
}

typedef struct {
    gr_surface surface;
    int img_w;
    int img_h;
} image_t;

typedef struct {
    uint32_t hidden;
    uint32_t index;
    rect_t rect;

    module_info *mod;
} item_t;

image_t *load_image(string image_name);
image_t *load_image(char *image_name);
bool is_point_in_rect(int x, int y, rect_t * rect);
bool is_point_in_rect(point_t point, rect_t * rect);

#endif
