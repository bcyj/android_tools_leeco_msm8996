/*
 * Copyright (c) 2014-2015, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#include "mmi.h"
#include "utils.h"
#include "module.h"
#include "draw.h"
#include "lang.h"
#include "controller.h"

/**
 *	  Handle print from client
 */
int handle_print(msg_t * msg, module_info * mod) {

    char tmp[256] = { 0 };
    textview *mod_tv = NULL;
    textview *pcba_tv = NULL;

    if(msg == NULL || mod == NULL)
        return -1;

    if(msg->msg_id == DATA || msg->msg_id == PRINT_DATA) {
        strlcpy(mod->data, msg->msg, sizeof(mod->data));
        sem_post(&g_data_print_sem);
    }

    if(msg->msg_id == PRINT || msg->msg_id == PRINT_DATA) {
        layout *lay = g_layout_map[(string) mod->config_list[KEY_LAYOUT]];
        layout *lay_pcba = g_layout_map[LAYOUT_PCBA];

        if(lay != NULL)
            mod_tv = lay->find_textview_by_name(KEY_DISPLAY);

        if(lay_pcba != NULL)
            pcba_tv = lay_pcba->find_textview_by_name(KEY_DISPLAY_PCBA);

        if(pcba_tv != NULL && !strcmp(msg->subcmd, SUBCMD_PCBA)) {
            snprintf(tmp, sizeof(tmp), "[%s]\n", mod->module);
            pcba_tv->append_text(tmp);
            pcba_tv->append_text(msg->msg);
        } else if(mod_tv != NULL && !strcmp(msg->subcmd, SUBCMD_MMI)) {
            mod_tv->set_text(msg->msg);
        }

        invalidate();
    }

    return 0;
}

/**
 *    Handle run response from client
 */
int handle_run(msg_t * msg, module_info * mod) {

    char temp[32] = { 0 };
    int ret, i = -1;

    if(msg == NULL || mod == NULL)
        return -1;

    ALOGI("%s,[%s]cmd(%d,%s), msg(%s) ", __FUNCTION__, msg->module, msg->cmd, msg->subcmd, msg->msg);

    if(!strcmp(msg->subcmd, SUBCMD_MMI)) {
        ALOGI("%s command: %s  received ", __FUNCTION__, msg->subcmd);
        if(!strcmp(MODULE_BLUETOOTH, mod->config_list[KEY_LIB_NAME].c_str())
           || !strcmp(MODULE_WIFI, mod->config_list[KEY_LIB_NAME].c_str())
           || !strcmp(MODULE_GPS, mod->config_list[KEY_LIB_NAME].c_str())) {
            mod->result = msg->result;
            sem_post(&g_result_sem);
        }
    } else if(!strcmp(msg->subcmd, SUBCMD_PCBA)) {
        if(msg->result != ERR_UNKNOW) {
            mod->result = msg->result;
        }
        /**update time duration*/
        time(&mod->last_time);
        mod->duration = difftime(mod->last_time, mod->start_time);
        flush_result();
        notify_controller(mod);
        invalidate();
    } else {
        ALOGE("%s Invalid command: %s  received ", __FUNCTION__, msg->subcmd);
        ret = -1;
    }

    return 0;
}

/**
 *	  Handle print from client
 */
int handle_query(msg_t * msg, module_info * mod) {
    hash_map < string, string > params;
    char temp[32] = { 0 };
    uint32_t i = -1;

    if(msg == NULL || mod == NULL)
        return -1;

    ALOGI("%s,%s ", __FUNCTION__, msg->msg);
    parse_parameter(msg->msg, params);
    string size_value = params[KEY_STR_EXTRACMDSIZE];

    if(size_value.empty() || atoi(size_value.c_str()) <= 0) {
        ALOGE("no valid extra command received, exit");
        return -1;
    }

    mod->extracmd.size = atoi(size_value.c_str());
    mod->extracmd.is_valid = true;

    for(i = 0; i < mod->extracmd.size; i++) {
        snprintf(temp, sizeof(temp), "%s%d", KEY_STR_EXTRACMD, i);
        if(params[temp] != "") {
            mod->extracmd.cmd_list.push_back(params[temp]);
            ALOGI("%s:get extra command: %s", __FUNCTION__, params[temp].c_str());
        }
    }

    /**Only update the first command to button*/
    layout *lay = g_layout_map[(string) mod->config_list[KEY_LAYOUT]];
    button *obj = lay->find_button_by_name(KEY_STR_EXTRACMD);

    if(obj != NULL) {
        obj->set_text(get_string(mod->extracmd.cmd_list.front()));
    }
    return 0;
}
