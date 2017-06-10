/*
 * Copyright (c) 2014-2015, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#include "layout.h"

layout::layout() {
}

layout::layout(const char *path) {
    if(path != NULL) {
        strlcpy(layout_path, path, sizeof(layout_path));
    }
}

void layout::add_listview_locked(listview * view) {
    if(view == NULL)
        return;

    mutex_locker::autolock _L(m_lock);

    if(view != m_listview) {
        delete(m_listview);
        m_listview = view;
    }
}
void layout::add_button_locked(button * btn) {
    if(btn == NULL)
        return;

    mutex_locker::autolock _L(m_lock);
    button_list.push_back(btn);
}

void layout::add_textview_locked(textview * tv) {
    if(tv == NULL)
        return;

    mutex_locker::autolock _L(m_lock);

    textview_list.push_back(tv);
}

textview *layout::find_textview_by_name(const char *name) {
    list < textview * >::iterator iter;
    for(iter = textview_list.begin(); iter != textview_list.end(); iter++) {
        textview *obj = (textview *) (*iter);

        if(obj != NULL && !strcmp(obj->get_name(), name))
            return obj;
    }
    return NULL;
}

button *layout::find_button_by_name(const char *name) {

    mutex_locker::autolock _L(m_lock);

    list < button * >::iterator iter;
    for(iter = button_list.begin(); iter != button_list.end(); iter++) {
        button *obj = (button *) (*iter);

        if(obj != NULL && !strcmp(obj->get_name(), name)) {
            return obj;
        }
    }

    return NULL;
}

void layout::set_button_color_by_name(const char *name, color_t * color) {

    mutex_locker::autolock _L(m_lock);

    list < button * >::iterator iter;
    for(iter = button_list.begin(); iter != button_list.end(); iter++) {
        button *obj = (button *) (*iter);

        if(obj != NULL && !strcmp(obj->get_name(), name))
            obj->set_color(color);
    }

}

bool layout::delete_btn_by_name(const char *name) {
    int ret = false;

    mutex_locker::autolock _L(m_lock);

    list < button * >::iterator iter;
    for(iter = button_list.begin(); iter != button_list.end(); iter++) {
        button *obj = (button *) (*iter);

        if(obj != NULL && !strcmp(obj->get_name(), name)) {
            delete obj;

            button_list.erase(iter);
            ret = true;
            break;
        }
    }

    return ret;
}

bool layout::delete_btn_by_point(point_t point, const char *name) {
    int ret = false;

    mutex_locker::autolock _L(m_lock);

    list < button * >::iterator iter;
    for(iter = button_list.begin(); iter != button_list.end(); iter++) {
        button *tmp = *iter;

        if(tmp != NULL && tmp->get_visibility() && is_point_in_rect(point, tmp->get_rect())
           && !strcmp(tmp->get_name(), name)) {
            delete tmp;

            button_list.erase(iter);
            ret = true;
            break;
        }
    }

    return ret;
}

void layout::enqueue_touch_locked(point_t point) {

    mutex_locker::autolock _L(m_lock);

    touch_list.push_back(point);
}
void layout::clear_locked() {

    ALOGI("%s\n", __FUNCTION__);
    mutex_locker::autolock _L(m_lock);

    /*Delete button */
    while(button_list.begin() != button_list.end()) {
        button *tmp = *button_list.begin();

        if(tmp != NULL)
            delete tmp;

        button_list.erase(button_list.begin());
    }

    /*textview_list button */
    while(textview_list.begin() != textview_list.end()) {
        textview *tmp = *textview_list.begin();

        if(tmp != NULL)
            delete tmp;

        textview_list.erase(textview_list.begin());
    }

    /*touch_list button */
    touch_list.clear();
    trace.clear();

    if(m_listview != NULL) {
        m_listview->clean_items();
        delete(m_listview);
        m_listview = NULL;
    }

}

char *layout::get_layout_path() {
    return layout_path;
}
