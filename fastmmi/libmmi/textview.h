/*
 * Copyright (c) 2014, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __LIBtextview__
#define __LIBtextview__

#include "view.h"

class textview {

  public:
    textview();
    textview(const char *name, rect_t * rect, const char *str);
      textview(const char *name, rect_t * rect, const char *str, int bold);

    char *get_text(void);
    void set_text(const char *str);
    void append_text(const char *str);
    int get_bold(void);
    void set_bold(int bold);

    char *get_name();
    void set_name(const char *name);
    rect_t *get_rect();
    void set_rect(rect_t * rect);
    void set_rect(int x, int y, int w, int h);

  private:
      rect_t m_rect;
    int m_bold;
    char m_name[NAME_MAX];
    /*support multi-line */
    char m_text[SIZE_8K];
};

#endif
