/*
 * Copyright (c) 2014-2015, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#include <string.h>
#include "listview.h"

listview::listview() {
    m_item_num = 10;
    m_cb = NULL;
    m_index = 0;
    m_page = 0;
    m_max_page = 0;
}

void listview::set_name(const char *str) {
    if(str != NULL)
        strlcpy(m_name, str, sizeof(m_name));
}

char *listview::get_name() {
    return m_name;
}

rect_t *listview::get_rect() {
    return &m_rect;
}

void listview::set_rect(rect_t * rect) {
    m_rect = *rect;
}
void listview::set_rect(int x, int y, int w, int h) {
    m_rect.x = x;
    m_rect.y = y;
    m_rect.w = w;
    m_rect.h = h;
}

cb_t listview::get_cb() {
    return m_cb;
}
void listview::set_cb(cb_t cb) {
    m_cb = cb;
}

void listview::page_up() {

    if(m_page <= 0)
        return;

    m_page--;

    int h = m_rect.h / m_item_num;

    list < item_t * >::iterator iter;
    for(iter = m_items.begin(); iter != m_items.end(); iter++) {
        item_t *obj = (item_t *) (*iter);

        if(obj->hidden == 0) {
            obj->rect.y = obj->rect.y + h * m_item_num;
        }
    }
}

void listview::page_down() {

    if(m_page >= m_max_page - 1)
        return;

    m_page++;

    int h = m_rect.h / m_item_num;

    list < item_t * >::iterator iter;
    for(iter = m_items.begin(); iter != m_items.end(); iter++) {
        item_t *obj = (item_t *) (*iter);

        if(obj->hidden == 0) {
            obj->rect.y = obj->rect.y - h * m_item_num;
        }
    }
}

int listview::getIndex() {
    return m_index;
}

void listview::setIndex(int index = 0) {
    m_index = index;
}

void listview::set_show_fail(int fail) {
    int i = 0;
    int h = m_rect.h / m_item_num;

    list < item_t * >::iterator iter;
    for(iter = m_items.begin(); iter != m_items.end(); iter++) {
        item_t *obj = (item_t *) (*iter);

        if(fail == 1 && obj->mod->result != FAILED) {
            obj->hidden = 1;
            obj->rect.x = 0;
            obj->rect.y = 0;
            obj->rect.h = 0;
            obj->rect.w = 0;
        } else {
            obj->hidden = 0;
            obj->rect.x = m_rect.x;
            obj->rect.y = m_rect.y + (i++) * h;
            obj->rect.h = h;
            obj->rect.w = m_rect.w;
        }
    }

    /**Update Max page num*/
    m_max_page = i / m_item_num;
    if(i % m_item_num != 0)
        m_max_page = m_max_page + 1;

    m_page = 0;

    ALOGI("new max page: %d", m_max_page);
}

int listview::get_item_num() {
    return m_item_num;
}

void listview::set_item_num(int itemNumInPage) {
    m_item_num = itemNumInPage;

    /*update max page num */
    m_max_page = m_items.size() / m_item_num;
    if(m_items.size() % m_item_num != 0)
        m_max_page = m_max_page + 1;

}

list < item_t * >*listview::get_items() {
    return &m_items;
}

void listview::clean_items() {
        /**Clean Items if any*/
    list < item_t * >::iterator iter;
    for(iter = m_items.begin(); iter != m_items.end(); iter++) {
        item_t *obj = (item_t *) (*iter);

        if(obj != NULL)
            delete obj;
    }

}

void listview::set_items(list < module_info * >*items) {

    int i = 0;
    int h = m_rect.h / m_item_num;
    int w = m_rect.w;

    if(items != NULL) {
        /**Clean items before initial*/
        clean_items();

        list < module_info * >::iterator iter;
        for(iter = items->begin(); iter != items->end(); iter++) {
            module_info *obj = (module_info *) (*iter);

            //skip "MMI" module
            if(!strcmp("MMI", obj->module))
                continue;

            //skip "disabled" modules
            if(obj != NULL && obj->config_list[KEY_ENABLE].compare("1"))
                continue;

            item_t *item = (item_t *) malloc(sizeof(item_t));

            if(obj == NULL || item == NULL)
                continue;

            item->mod = obj;
            item->rect.h = h;
            item->rect.w = w;
            item->rect.x = m_rect.x;
            item->rect.y = m_rect.y + i * h;
            item->index = i + 1;
            item->hidden = 0;

            ALOGI("[%s](%d,%d)", item->mod->module, item->rect.x, item->rect.y);
            //update index
            i++;

            m_items.push_back(item);
        }
    }

    m_max_page = m_items.size() / m_item_num;
    if(m_items.size() % m_item_num != 0)
        m_max_page = m_max_page + 1;

    m_page = 0;
    ALOGI("m_max_page:%d", m_max_page);
}

int listview::get_page() {
    return m_page;
}

int listview::get_fail_count() {
    int num = 0;

    list < item_t * >::iterator iter;
    for(iter = m_items.begin(); iter != m_items.end(); iter++) {
        item_t *obj = (item_t *) (*iter);

        if(obj != NULL && obj->mod != NULL && obj->mod->result == FAILED)
            num++;
    }

    return num;
}

int listview::get_pass_count() {
    int num = 0;

    list < item_t * >::iterator iter;
    for(iter = m_items.begin(); iter != m_items.end(); iter++) {
        item_t *obj = (item_t *) (*iter);

        if(obj != NULL && obj->mod != NULL && obj->mod->result == SUCCESS)
            num++;
    }

    return num;
}

void listview::reset_result() {
    list < item_t * >::iterator iter;
    for(iter = m_items.begin(); iter != m_items.end(); iter++) {
        item_t *obj = (item_t *) (*iter);

        if(obj != NULL && obj->mod != NULL) {
            obj->mod->result = ERR_UNKNOW;
            obj->mod->start_time = 0;
            obj->mod->duration = 0;
            memset(obj->mod->data, 0, sizeof(obj->mod->data));
        }
    }
}
