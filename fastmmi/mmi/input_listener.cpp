/*
 * Copyright (c) 2014, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#include "input_listener.h"

input_listener::input_listener():m_mod(NULL), m_lay(NULL), m_cb(NULL) {
}

input_listener::input_listener(module_info * mod, layout * lay, cb_t cb):m_mod(mod), m_lay(lay), m_cb(cb) {
}

input_listener::~input_listener() {
}
module_info *input_listener::get_module() {
    return m_mod;
}

void input_listener::set_module(module_info * mod) {
    m_mod = mod;
}

layout *input_listener::get_lay() {
    return m_lay;
}

void input_listener::set_lay(layout * lay) {
    m_lay = lay;
}

cb_t input_listener::get_cb() {
    return m_cb;
}
void input_listener::set_cb(cb_t cb) {
    m_cb = cb;
}
