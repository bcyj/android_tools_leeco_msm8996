/*
 * Copyright (c) 2013-2015, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __INPUT__
#define __INPUT__

#include "view.h"
#include "input_listener.h"

#define MAX_KEYMAP_LINES 10

struct vkey_map_struct {
    uint32_t version;
    uint32_t key_code;
    uint32_t center_x;
    uint32_t center_y;
    uint32_t width;
    uint32_t height;
};

union vkey_map {
    struct vkey_map_struct map;
    int vkey_map_value[sizeof(struct vkey_map_struct) / sizeof(int)];
};

typedef struct adjust_value {
    uint32_t valid;
    uint32_t lcd_x;
    uint32_t lcd_y;
    uint32_t ts_x_max;
    uint32_t ts_x_min;
    uint32_t ts_y_max;
    uint32_t ts_y_min;
} input_adjust_t;

int input_callback(int fd, short revents, void *data);
void *input_waiting_thread(void *);
void *input_handle_thread(void *);

void register_input_listener(input_listener * listener);
void unregister_input_listener();

#endif
