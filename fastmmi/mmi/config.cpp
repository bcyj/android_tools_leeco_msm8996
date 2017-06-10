/*
 * Copyright (c) 2014-2015, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlmemory.h>
extern "C" {
#include <minui.h>
}
#include "common.h"
#include "utils.h"
#include "config.h"
#include "button.h"
#include "layout.h"
#include "config.h"
#include "func_map.h"
#include "view.h"
#include "mmi.h"
#include "lang.h"
#include "graphics.h"

int load_config(const char *path, hash_map < string, module_info * >*modules, list < module_info * >*clients) {
    char module[1024] = { 0, };
    char line[1024] = { 0, };
    char indicator = '=';
    module_info *cur_mod = NULL;

    FILE *file = fopen(path, "r");

    if(file == NULL) {
        ALOGE("%s open failed\n", path);
        return CONFIG_NOT_FOUND_ERR;
    }

    modules->clear();
    clients->clear();

    ALOGD("Loading config: %s\n", path);

    while(fgets(line, sizeof(line), file) != NULL) {
        char name[1024] = { 0, }, value[1024] = {
        0,};

        if(line[0] == '#')
            continue;

        if(line[0] == '[') {
            parse_module(line, module, sizeof(module));
            cur_mod = new module_info(module);
            // add module into global list
            (*modules)[(string) module] = cur_mod;
            clients->push_back(cur_mod);
            ALOGD("[%s]\n", module);
            continue;
        }

        if(module[0] != '\0') {
            parse_value(line, indicator, name, sizeof(name), value, sizeof(value));
            char *pname = trim(name);
            char *pvalue = trim(value);

            if(*pname != '\0' && *pvalue != '\0') {

                /*Initial language */
                if(!strcmp(pname, KEY_STR_LANGUAGE)) {
                    load_lang(pvalue);
                }

                /*initial font size */
                if(!strcmp(pname, KEY_FONTSIZE)) {
                    set_font_size(pvalue);
                }

                cur_mod->config_list[(string) pname] = (string) pvalue;
                ALOGI("[%s] %s:%s \n", module, pname, pvalue);
            }
        }
    }

    fclose(file);
    return CONFIG_SUCCESS;
}

int parse_module(const char *line, char *module, int module_len) {
    if(line == NULL || module == NULL)
        return -1;
    string input(line);
    int startIndex = input.find_first_of('[');
    int endIndex = input.find_first_of(']');

    if(startIndex >= 0 && endIndex >= 0 && endIndex > startIndex + 1) {
        if(endIndex - startIndex - 1 < module_len) {
            strlcpy(module, line + startIndex + 1, module_len);
            module[endIndex - startIndex - 1] = '\0';
            return 0;
        } else
            return -1;
    } else
        return -1;
}

static void parse_button(xmlNodePtr node, button * btn) {
    xmlAttrPtr attr;
    rect_t rect;

    attr = node->properties;
    while(attr != NULL) {
        char *value = (char *) xmlGetProp(node, (const xmlChar *) attr->name);

        if(value != NULL) {
            if(!xmlStrcmp(attr->name, (const xmlChar *) "name")) {
                btn->set_name(value);
            } else if(!xmlStrcmp(attr->name, (const xmlChar *) "text"))
                btn->set_text(get_string(value));
            else if(!xmlStrcmp(attr->name, (const xmlChar *) "image")) {
                btn->set_image(value);
            } else if(!xmlStrcmp(attr->name, (const xmlChar *) "onclick")) {
                btn->set_cb(get_cb(value));
            } else if(!xmlStrcmp(attr->name, (const xmlChar *) "color"))
                btn->set_color(strtol(value, NULL, 0));
            else if(!xmlStrcmp(attr->name, (const xmlChar *) "x_abs"))
                rect.x = atoi(value);
            else if(!xmlStrcmp(attr->name, (const xmlChar *) "y_abs"))
                rect.y = atoi(value);
            else if(!xmlStrcmp(attr->name, (const xmlChar *) "w_abs"))
                rect.w = atoi(value);
            else if(!xmlStrcmp(attr->name, (const xmlChar *) "h_abs"))
                rect.h = atoi(value);
            else if(!xmlStrcmp(attr->name, (const xmlChar *) "x_rel")) {
                rect.x = atoi(value) * gr_fb_width() / 100;
            } else if(!xmlStrcmp(attr->name, (const xmlChar *) "y_rel")) {
                rect.y = atoi(value) * gr_fb_height() / 100;
            } else if(!xmlStrcmp(attr->name, (const xmlChar *) "w_rel")) {
                rect.w = atoi(value) * gr_fb_width() / 100;
            } else if(!xmlStrcmp(attr->name, (const xmlChar *) "h_rel")) {
                rect.h = atoi(value) * gr_fb_height() / 100;
            } else if(!xmlStrcmp(attr->name, (const xmlChar *) "visibility")) {
                if(!strcmp("invisible", value))
                    btn->set_visibility(false);
                else
                    btn->set_visibility(true);
            }

            xmlFree(value);
        }

        attr = attr->next;
    }
    btn->set_rect(&rect);
}

static void parse_textview(xmlNodePtr node, textview * tv) {
    xmlAttrPtr attr;
    rect_t rect;

    attr = node->properties;

    while(attr != NULL) {
        char *value = (char *) xmlGetProp(node, (const xmlChar *) attr->name);

        if(value != NULL) {
            if(!xmlStrcmp(attr->name, (const xmlChar *) "name")) {
                tv->set_name(value);
            } else if(!xmlStrcmp(attr->name, (const xmlChar *) "text"))
                tv->set_text(value);
            else if(!xmlStrcmp(attr->name, (const xmlChar *) "x_abs"))
                rect.x = atoi(value);
            else if(!xmlStrcmp(attr->name, (const xmlChar *) "y_abs"))
                rect.y = atoi(value);
            else if(!xmlStrcmp(attr->name, (const xmlChar *) "w_abs"))
                rect.w = atoi(value);
            else if(!xmlStrcmp(attr->name, (const xmlChar *) "h_abs"))
                rect.h = atoi(value);
            else if(!xmlStrcmp(attr->name, (const xmlChar *) "x_rel")) {
                rect.x = atoi(value) * gr_fb_width() / 100;
            } else if(!xmlStrcmp(attr->name, (const xmlChar *) "y_rel")) {
                rect.y = atoi(value) * gr_fb_height() / 100;
            } else if(!xmlStrcmp(attr->name, (const xmlChar *) "w_rel")) {
                rect.w = atoi(value) * gr_fb_width() / 100;
            } else if(!xmlStrcmp(attr->name, (const xmlChar *) "h_rel")) {
                rect.h = atoi(value) * gr_fb_height() / 100;
            }
            xmlFree(value);
        }
        attr = attr->next;
    }
    tv->set_rect(&rect);
}

static void parse_listview(xmlNodePtr node, listview * view) {
    xmlAttrPtr attr;
    rect_t rect;

    attr = node->properties;
    while(attr != NULL) {
        char *value = (char *) xmlGetProp(node, (const xmlChar *) attr->name);

        if(value != NULL) {
            if(!xmlStrcmp(attr->name, (const xmlChar *) "name"))
                view->set_name(value);
            else if(!xmlStrcmp(attr->name, (const xmlChar *) "onclick"))
                view->set_cb(get_cb(value));
            else if(!xmlStrcmp(attr->name, (const xmlChar *) "x_abs"))
                rect.x = atoi(value);
            else if(!xmlStrcmp(attr->name, (const xmlChar *) "y_abs"))
                rect.y = atoi(value);
            else if(!xmlStrcmp(attr->name, (const xmlChar *) "w_abs"))
                rect.w = atoi(value);
            else if(!xmlStrcmp(attr->name, (const xmlChar *) "h_abs"))
                rect.h = atoi(value);
            else if(!xmlStrcmp(attr->name, (const xmlChar *) "x_rel")) {
                rect.x = atoi(value) * gr_fb_width() / 100;
            } else if(!xmlStrcmp(attr->name, (const xmlChar *) "y_rel"))
                rect.y = atoi(value) * gr_fb_height() / 100;
            else if(!xmlStrcmp(attr->name, (const xmlChar *) "w_rel"))
                rect.w = atoi(value) * gr_fb_width() / 100;
            else if(!xmlStrcmp(attr->name, (const xmlChar *) "h_rel"))
                rect.h = atoi(value) * gr_fb_height() / 100;

            xmlFree(value);
        }
        attr = attr->next;
    }
    view->set_rect(&rect);
}

int load_layout(const char *layout_path, layout * cur_layout) {
    int i, err = 0;
    char *ver, *ver_sys;
    size_t nodes_size = 0;
    xmlDocPtr doc = NULL;
    xmlNodePtr node = NULL;

    if(layout_path == NULL || cur_layout == NULL)
        return -1;

    if(access(layout_path, F_OK)) {
        ALOGE("Can't find %s\n", layout_path);
        return -1;
    }

    doc = xmlReadFile(layout_path, "UTF-8", XML_PARSE_RECOVER);
    if(doc == NULL) {
        ALOGE("%s parse failed\n", layout_path);
        return -1;
    }
    node = xmlDocGetRootElement(doc);
    if(node == NULL) {
        ALOGE("%s empty\n", layout_path);
        xmlFreeDoc(doc);
        return -1;
    }

    if(xmlStrcmp(node->name, (const xmlChar *) "layout")) {
        ALOGE("%s not a layout xml\n", layout_path);
        xmlFreeDoc(doc);
        return -1;
    }

    node = node->xmlChildrenNode;
    while(node != NULL) {

        // parse button attr
        if(!xmlStrcmp(node->name, (const xmlChar *) "button")) {
            button *btn = new button();

            parse_button(node, btn);
            cur_layout->add_button_locked(btn);
        } else if(!xmlStrcmp(node->name, (const xmlChar *) "textview")) {
            textview *tv = new textview();

            parse_textview(node, tv);
            cur_layout->add_textview_locked(tv);
        } else if(!xmlStrcmp(node->name, (const xmlChar *) "listview")) {
            listview *view = new listview();

            parse_listview(node, view);
            cur_layout->add_listview_locked(view);
        } else if(!xmlStrcmp(node->name, (const xmlChar *) "include")) {
            char xml_path[128] = { 0 };
            char *str = (char *) xmlGetProp(node, (const xmlChar *) "layout");

            if(str != NULL) {
                snprintf(xml_path, sizeof(xml_path), "%s%s", MMI_LAYOUT_BASE_DIR, str);
                load_layout(xml_path, cur_layout);
            }
        }

        node = node->next;
    }

    xmlFreeDoc(doc);
    xmlCleanupParser();

    return 0;
}
