/*
 * Copyright (c) 2014, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#ifndef __INPUT_LISTENER__
#define __INPUT_LISTENER__
#include "common.h"
#include <linux/input.h>
#include "layout.h"

class input_listener {
  public:
    input_listener();
    input_listener(module_info * mod, layout * lay, cb_t cb);
    /**return false mean not continue*/
    virtual bool dispatch_event(input_event ev) = 0;
    virtual ~ input_listener();
    module_info *get_module();
    void set_module(module_info * mod);

    layout *get_lay();
    void set_lay(layout * mod);

    cb_t get_cb();
    void set_cb(cb_t cb);

  private:
    module_info * m_mod;
    layout *m_lay;
    cb_t m_cb;
};

class input_listener_touch:public input_listener {
  public:
    input_listener_touch():input_listener() {
    };

  input_listener_touch(module_info * mod, layout * lay, cb_t cb):input_listener(mod, lay, cb) {
    };
    ~input_listener_touch() {
    };
    bool dispatch_event(input_event ev);
};

class input_listener_key:public input_listener {
  public:
    input_listener_key():input_listener() {
    };

  input_listener_key(module_info * mod, layout * lay, cb_t cb):input_listener(mod, lay, cb) {
    };
    ~input_listener_key() {
    };
    bool dispatch_event(input_event ev);
};

#endif
