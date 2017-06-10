/*
 * Copyright (c) 2014, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#include "input_listener.h"
#include "draw.h"
bool input_listener_touch::dispatch_event(input_event ev) {

    static int x_last = -1;
    static int y_last = -1;
    static point_t last_point(-1, -1);
    char str[32] = { 0 };

    int x = x_last;
    int y = y_last;

    point_t cur_point(-1, -1);
    module_info *m = this->get_module();
    layout *lay = this->get_lay();
    cb_t cb = this->get_cb();

    __u16 type = ev.type;
    __u16 code = ev.code;
    __u32 value = ev.value;

    if(type == EV_ABS) {
        if(code == ABS_MT_POSITION_X) {
            x_last = x = value;
        } else if(code == ABS_MT_POSITION_Y) {
            y_last = y = value;
        } else if(code == ABS_MT_TRACKING_ID && value != 0xffffffff) {
            lay->trace.clear();
        }
    } else if(type == EV_SYN) {

        point_t cur_point(x, y);

        if(!(last_point == cur_point)) {
            last_point = cur_point;
            lay->trace.push_back(cur_point);

             /**Delete the touched button*/
            lay->delete_btn_by_point(cur_point, KEY_BTN);
            button *btn = lay->find_button_by_name(KEY_DISPLAY);

            if(btn != NULL) {
                snprintf(str, sizeof(str), "(%d,%d)", cur_point.x, cur_point.y);
                btn->set_text(str);
            }

            invalidate();

            /**Check if it is the last point*/
            if(lay->find_button_by_name(KEY_BTN) == NULL) {
                lay->trace.clear();
                cb(m);
            }
        }
    }

    return true;
}
