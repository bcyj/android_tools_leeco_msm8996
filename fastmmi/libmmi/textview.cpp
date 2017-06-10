/*
 * Copyright (c) 2014, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#include "textview.h"
#include <string.h>

textview::textview():m_bold(0) {
    m_rect.x = 0;
    m_rect.y = 0;
    m_rect.w = 0;
    m_rect.h = 0;

    strlcpy(m_text, "", sizeof(m_text));
    strlcpy(m_name, "undefined", sizeof(m_name));
}

textview::textview(const char *name, rect_t * rect, const char *str):m_bold(0) {
    set_rect(rect);

    if(str != NULL)
        strlcpy(m_text, str, sizeof(m_text));

    if(name != NULL)
        strlcpy(m_name, name, sizeof(m_name));
}

textview::textview(const char *name, rect_t * rect, const char *str, int bold):m_bold(bold) {
    set_rect(rect);

    if(str != NULL)
        strlcpy(m_text, str, sizeof(m_text));

    if(name != NULL)
        strlcpy(m_name, name, sizeof(m_name));
}

char *textview::get_text() {
    return m_text;
}

char *textview::get_name() {
    return m_name;
}
void textview::set_name(const char *name) {

    if(name != NULL)
        strlcpy(m_name, name, sizeof(m_name));
}

void textview::set_text(const char *str) {

    if(str != NULL)
        strlcpy(m_text, str, sizeof(m_text));
}

void textview::append_text(const char *str) {

    if(str != NULL)
        strlcat(m_text, str, sizeof(m_text));
}

int textview::get_bold() {
    return m_bold;
}

void textview::set_bold(int bold) {
    m_bold = bold;
}

void textview::set_rect(rect_t * rect) {
    m_rect.x = rect->x;
    m_rect.y = rect->y;
    m_rect.w = rect->w;
    m_rect.h = rect->h;
}
void textview::set_rect(int x, int y, int w, int h) {
    m_rect.x = x;
    m_rect.y = y;
    m_rect.w = w;
    m_rect.h = h;
}
rect_t *textview::get_rect() {
    return &m_rect;
}
