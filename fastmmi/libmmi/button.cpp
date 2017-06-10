/*
 * Copyright (c) 2014, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#include <string.h>
#include "button.h"

button::button():m_cb(NULL), m_visibility(true) {
    strlcpy(m_name, "undefined", sizeof(m_name));
    m_text[0] = '\0';
    m_image[0] = '\0';
    set_color(255, 255, 255, 0);
    m_disabled = false;
}

button::button(const char *name, rect_t rect, const char *str, cb_t cb):m_cb(cb), m_visibility(true) {
    m_rect = rect;
    if(str != NULL)
        strlcpy(m_text, str, sizeof(m_text));

    if(name != NULL)
        strlcpy(m_name, name, sizeof(m_name));

    m_image[0] = '\0';
    set_color(255, 255, 255, 0);
    m_disabled = false;
}

char *button::get_text() {
    return m_text;
}

rect_t *button::get_rect() {
    return &m_rect;
}

void button::set_rect(rect_t * rect) {
    m_rect.x = rect->x;
    m_rect.y = rect->y;
    m_rect.w = rect->w;
    m_rect.h = rect->h;
}
void button::set_rect(int x, int y, int w, int h) {
    m_rect.x = x;
    m_rect.y = y;
    m_rect.w = w;
    m_rect.h = h;
}

void button::set_color(char r, char g, char b, char a) {
    m_color.r = r;
    m_color.g = g;
    m_color.b = b;
    m_color.a = a;
}

void button::set_color(color_t * color) {
    m_color.r = color->r;
    m_color.g = color->g;
    m_color.b = color->b;
    m_color.a = color->a;
}

void button::set_color(int color) {
    m_color.r = (char) ((color & 0xff000000) >> 24);
    m_color.g = (char) ((color & 0x00ff0000) >> 16);
    m_color.b = (char) ((color & 0x0000ff00) >> 8);
    m_color.a = (char) (color & 0x000000ff);
}

color_t *button::get_color() {
    return &m_color;
}

void button::set_text(const char *str) {
    strlcpy(m_text, str, sizeof(m_text));
}

void button::set_name(const char *str) {
    strlcpy(m_name, str, sizeof(m_name));
}

char *button::get_name() {
    return m_name;
}

cb_t button::get_cb() {
    return m_cb;
}
void button::set_cb(cb_t cb) {
    m_cb = cb;
}

bool button::get_visibility() {
    return m_visibility;
}

void button::set_visibility(bool visible) {
    m_visibility = visible;
}

char *button::get_image() {
    return m_image;
}
void button::set_image(char *image) {
    strlcpy(m_image, image, sizeof(m_image));
}

bool button::get_disabled() {
    return m_disabled;
}
void button::set_disabled(bool disable) {
    m_disabled = disable;
    /**store the color*/
    if(disable) {
        m_color_bak.r = m_color.r;
        m_color_bak.g = m_color.g;
        m_color_bak.b = m_color.b;
        m_color_bak.a = m_color.a;

        set_color(0xccccccff);
    } else {
        set_color(&m_color_bak);
    }
}
