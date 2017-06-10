/*
 * Copyright (c) 2014, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __SYSTEM_CORE_MMI_LANG__
#define __SYSTEM_CORE_MMI_LANG__

using namespace std;

typedef struct {
    char name[NAME_MAX];
    char filepath[PATH_MAX];
} lang_t;

int load_lang(char *lang);
const char *get_string(const char *key);
const char *get_string(string key);
#endif
