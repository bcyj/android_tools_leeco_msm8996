/*
 * Copyright (c) 2014, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __LISTVIEW__
#define __LISTVIEW__

#include "view.h"

class listview {

  public:
    listview();
    char *get_name();
    void set_name(const char *str);

    rect_t *get_rect();
    void set_rect(rect_t * rect);
    void set_rect(int x, int y, int w, int h);

    void page_up();
    void page_down();

    cb_t get_cb();
    void set_cb(cb_t cb);

    int getIndex();
    void setIndex(int index);
    int get_page(void);

    int get_item_num();
    void set_item_num(int);

    int get_show_fail();
    void set_show_fail(int);
    int get_pass_count();
    int get_fail_count();
    void reset_result();

      list < item_t * >*get_items();
    void set_items(list < module_info * >*items);
    void clean_items();

  private:
      rect_t m_rect;
    char m_name[NAME_MAX];
    int m_page;
    int m_index;
    cb_t m_cb;
    int m_item_num;
    int m_max_page;
      list < item_t * >m_items;
};

#endif
