/*
 * Copyright (c) 2014-2015, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#include "input_listener.h"
#include "draw.h"

bool input_listener_key::dispatch_event(input_event ev) {

    layout *lay = this->get_lay();
    char btn_name[64] = { 0 };

    __u16 type = ev.type;
    __u16 code = ev.code;
    __u32 value = ev.value;

    int down = ! !value;

    if(type == EV_KEY) {
        switch (code) {
        case KEY_BACK:
            strlcpy(btn_name, KEY_STR_BACK, sizeof(btn_name));
            break;

        case KEY_HOME:
        case KEY_HOMEPAGE:
            strlcpy(btn_name, KEY_STR_HOME, sizeof(btn_name));
            break;

        case KEY_MENU:
            strlcpy(btn_name, KEY_STR_MENU, sizeof(btn_name));
            break;

        case KEY_VOLUMEUP:
            strlcpy(btn_name, KEY_STR_VOLUMEUP, sizeof(btn_name));
            break;

        case KEY_VOLUMEDOWN:
            strlcpy(btn_name, KEY_STR_VOLUMEDOWN, sizeof(btn_name));
            break;

        case KEY_CAMERA_SNAPSHOT:
            strlcpy(btn_name, KEY_STR_SNAPSHOT, sizeof(btn_name));
            break;

        case KEY_POWER:
            strlcpy(btn_name, KEY_STR_POWER, sizeof(btn_name));
            break;

        case KEY_MEDIA:        //Hangup key
        case BTN_1:
            strlcpy(btn_name, KEY_STR_HANGUP, sizeof(btn_name));
            break;

        default:
            break;
        }
    }
    if(type == EV_SW) {
        switch (code) {
        case SW_HEADPHONE_INSERT:
            strlcpy(btn_name, KEY_STR_HEADPHONE_INSERT, sizeof(btn_name));
            break;
        case SW_MICROPHONE_INSERT:
            strlcpy(btn_name, KEY_STR_MICROPHONE_INSERT, sizeof(btn_name));
            break;
        default:
            break;
        }
    }
    button *btn = lay->find_button_by_name(btn_name);

    if(btn != NULL) {
        if(down) {
            ALOGI("[%s]key:%d press down", __FUNCTION__, code);
            btn->set_color(255, 0, 0, 125);
        } else {
            ALOGI("[%s]key:%d release", __FUNCTION__, code);
            btn->set_color(255, 0, 0, 255);
        }
        invalidate();
    }

    if(code == KEY_BACK)
        return false;

    return true;
}
