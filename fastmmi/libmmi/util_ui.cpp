/*
 * Copyright (c) 2014, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#include "utils.h"
#include "view.h"

static hash_map < string, image_t * >image_map;

image_t *load_image(string image_name) {

    if(image_map[image_name] != NULL) {
        return image_map[image_name];
    } else {
        gr_surface surface;
        string path = "/etc/mmi/" + image_name;
        int retval = res_create_display_surface(path.c_str(), &surface);

        if(retval < 0) {
            ALOGE("Failed to load %s, ret=%d.\n", image_name.c_str(), retval);
            return NULL;
        }
        image_t *image = (image_t *) malloc(sizeof(image_t));

        if(image == NULL)
            return NULL;
        image->surface = surface;
        image->img_w = gr_get_width(surface);
        image->img_h = gr_get_height(surface);
        image_map[image_name] = image;
        return image;
    }
}

image_t *load_image(char *image_name) {

    if(image_name == NULL)
        return NULL;
    else
        return load_image((string) image_name);
}

bool is_point_in_rect(int x, int y, rect_t * rect) {
    if(rect == NULL)
        return false;

    return (x >= rect->x && x <= rect->x + rect->w && y >= rect->y && y <= rect->y + rect->h);
}

bool is_point_in_rect(point_t point, rect_t * rect) {
    if(rect == NULL)
        return false;

    return (point.x >= rect->x && point.x <= rect->x + rect->w && point.y >= rect->y && point.y <= rect->y + rect->h);
}
