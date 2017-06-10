/*
 * Copyright (c) 2014-2015, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#ifndef __LIBbutton__
#define __LIBbutton__
#include "view.h"

class button {
  public:
    button();
    button(const char *name, rect_t rect, const char *str, cb_t cb);

    char *get_name();
    void set_name(const char *str);

    char *get_text();
    void set_text(const char *str);

    rect_t *get_rect();
    void set_rect(rect_t * rect);
    void set_rect(int x, int y, int w, int h);

    void set_color(char r, char g, char b, char a);
    void set_color(color_t * color);
    void set_color(int color);
    color_t *get_color();

    cb_t get_cb();
    void set_cb(cb_t cb);

    bool get_visibility();
    void set_visibility(bool visible);

    char *get_image();
    void set_image(char *image);

    bool get_disabled();
    void set_disabled(bool disable);

  protected:
    rect_t m_rect;
    char m_name[NAME_MAX];
    char m_text[NAME_MAX];
    color_t m_color;
    color_t m_color_bak;
    cb_t m_cb;
    bool m_visibility;
    bool m_disabled;
    char m_image[NAME_MAX];
};
#endif
