/*
 * Copyright (c) 2014, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#include <libxml/parser.h>
#include <libxml/tree.h>
#include "common.h"
#include "lang.h"
#include "mmi.h"

static lang_t langs[] = {
    {"en", "/etc/mmi/strings.xml"},
    {"zh-rCN", "/etc/mmi/strings-zh-rCN.xml"},
};

static hash_map < string, string > strings_map;

static char *get_filepath(char *name) {

    if(name == NULL)
        return NULL;

    for(uint32_t i = 0; i < sizeof(langs) / sizeof(lang_t); i++) {
        if(!strcmp(name, langs[i].name))
            return langs[i].filepath;
    }

    return NULL;
}
int load_lang(char *lang) {

    xmlDocPtr doc;
    xmlNodePtr curNode;
    char *name = NULL;
    char *value = NULL;

    char *filepath = get_filepath(lang);

    if(filepath == NULL) {
        ALOGE("Invalid file name param");
        return -1;
    }

    ALOGI("start to initial language: %s, %s", lang, filepath);

    doc = xmlReadFile(filepath, "UTF-8", XML_PARSE_RECOVER);
    if(NULL == doc) {
        ALOGI("Document not parsed successfully.\n ");
        return -1;
    }

    curNode = xmlDocGetRootElement(doc);
    if(NULL == curNode) {
        ALOGI("empty document \n");
        xmlFreeDoc(doc);
        return -1;
    }


    if(xmlStrcmp(curNode->name, BAD_CAST "resources")) {
        ALOGI("document of the wrong type, root node != root \n");
        xmlFreeDoc(doc);
        return -1;
    }

    /*clear map before initialize */
    strings_map.clear();

    curNode = curNode->xmlChildrenNode;
    while(curNode != NULL) {
        if(!xmlStrcmp(curNode->name, BAD_CAST "string")) {
            name = (char *) xmlGetProp(curNode, BAD_CAST "name");
            value = (char *) xmlNodeGetContent(curNode);
            if(name != NULL && value != NULL) {
                strings_map[name] = value;
                xmlFree(value);
            }
        }
        curNode = curNode->next;
    }

    xmlFreeDoc(doc);
    return 0;
}

const char *get_string(const char *key) {
    if(key == NULL)
        return NULL;

    return get_string((string) key);
}
const char *get_string(string key) {
    return strings_map[key].c_str();
}
