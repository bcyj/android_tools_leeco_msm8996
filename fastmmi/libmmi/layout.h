/*
 * Copyright (c) 2014, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#ifndef __LAYOUT__
#define __LAYOUT__

#include "common.h"
#include "button.h"
#include "listview.h"
#include "textview.h"

class layout {
  public:
    list < button * >button_list;
    list < textview * >textview_list;
    list < point_t > touch_list;
    list < point_t > trace;
    listview *m_listview = NULL;
      layout();
      layout(const char *path);
    void add_button_locked(button * btn);
    void add_textview_locked(textview * tv);
    void add_listview_locked(listview * view);

    void lock_touch_queue(bool);
    void enqueue_touch_locked(point_t cb);
    button *find_button_by_name(const char *name);
    bool delete_btn_by_name(const char *name);
    void set_button_color_by_name(const char *name, color_t * color);
    textview *find_textview_by_name(const char *name);
    bool delete_btn_by_point(point_t point, const char *name);

    module_info *module;
    void clear_locked();
    char *get_layout_path();

  private:
    char layout_path[128];
    mutex_locker m_lock;
};

#endif
