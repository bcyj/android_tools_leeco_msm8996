/*
 * Copyright (c) 2014-2015, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#include "mmi.h"
#include "utils.h"
#include "module.h"
#include "draw.h"
#include "lang.h"

static void do_ctrl_stat(msg_t * resp) {
    char buf[NAME_MAX] = { 0 };
    int count = 0;

    /*Get the layout */
    layout *lay = g_layout_map[get_main_module()->config_list[KEY_LAYOUT]];

    if(lay == NULL || lay->m_listview == NULL) {
        ALOGE("[%s] No layout for MAIN\n");
        return;
    }
    count = lay->m_listview->get_fail_count();

    snprintf(buf, sizeof(buf), "%s:%d;%s:%d;", KEY_MMI_STAT, MMI_IDLE, KEY_FAIL_COUNT, count);
    ALOGI("%s:Get fail count :%d] ", __FUNCTION__, count);
    resp->result = SUCCESS;
    strlcpy(resp->msg, buf, sizeof(resp->msg));
}

static void do_ctrl_reconfig(msg_t * resp, const char *cfg) {
    ALOGI("%s:start ", __FUNCTION__);
    char buf[NAME_MAX] = { 0 };
    int num = reconfig(cfg);

    snprintf(buf, sizeof(buf), "%s:%d;", KEY_CASE_NUM, num);
    resp->result = SUCCESS;
    strlcpy(resp->msg, buf, sizeof(resp->msg));
}

static void do_ctrl_clear(msg_t * resp) {
    resp->result = clear_file(g_res_file);
}

static void do_ctrl_runcase(msg_t * resp, string module_name) {
    module_info *mod = NULL;
    layout *lay = NULL;

    resp->result = FAILED;
    ALOGI("%s:	  casename:%s] ", __FUNCTION__, module_name.c_str());

    if(module_name.empty())
        return;

    mod = g_modules_map[module_name];
    if(mod == NULL)
        return;

    lay = g_layout_map[LAYOUT_PCBA];
    if(lay == NULL) {
        ALOGE("No layout for :%s", LAYOUT_PCBA);
        return;
    }

    /**switch layout to pcba and initial the module*/
    switch_cur_layout_locked(lay, mod);

    layout *curlay = acquire_cur_layout();
    button *obj = curlay->find_button_by_name(KEY_STR_TITLE);

    if(obj != NULL)
        obj->set_text(get_string(KEY_TITLE_PCBA));
    invalidate();
    release_cur_layout();

    module_exec_pcba(mod);
    resp->result = SUCCESS;

}

static void do_ctrl_runall(msg_t * resp) {
    ALOGI("%s: start", __FUNCTION__);
    resp->result = SUCCESS;
    start_all(false);
}

static void do_ctrl_listcase(msg_t * resp, const char *filepath) {
    list < item_t * >*items;
    FILE *fd = NULL;
    char buf[64];

    resp->result = FAILED;
    ALOGI("%s:	list filepath:%s] ", __FUNCTION__, filepath);

    fd = fopen(filepath, "w");
    if(!fd)
        return;
    /*Get the layout */
    layout *lay = g_layout_map[get_main_module()->config_list[KEY_LAYOUT]];

    if(lay == NULL || lay->m_listview == NULL || lay->m_listview->get_items() == NULL) {
        ALOGE("[%s] No layout\n", get_main_module()->module);
        return;
    }

    items = lay->m_listview->get_items();
    list < item_t * >::iterator iter;
    for(iter = items->begin(); iter != items->end(); iter++) {
        item_t *obj = (item_t *) (*iter);

        memset(buf, 0, sizeof(buf));
        snprintf(buf, sizeof(buf), "%s\n", obj->mod->module);
        buf[sizeof(buf) - 1] = '\0';
        fwrite(buf, strlen(buf), 1, fd);
    }
    fclose(fd);
    fd = NULL;
    resp->result = SUCCESS;

}

static void do_ctrl_exitcase(msg_t * resp) {
    ALOGI("%s: start", __FUNCTION__);
    resp->result = SUCCESS;
}

void notify_controller(module_info * mod) {

    module_info *target_mod = NULL;
    char tmp[SIZE_1K] = { 0 };

    if(mod == NULL)
        return;

    snprintf(tmp, sizeof(tmp), "\n[%s]\nTimestamp = %lu\nResult = %s \nTestTime_Sec = %.f \n", mod->module,
             mod->last_time, mod->result == SUCCESS ? KEY_PASS : KEY_FAIL, mod->duration);
    strlcat(tmp, mod->data, sizeof(tmp));

      /**Notify diag */
    target_mod = g_controller_map[CLIENT_DIAG_NAME];
    if(target_mod != NULL) {
        send_cmd(target_mod->socket_fd, CLIENT_DIAG_NAME, CMD_RESULT, NULL, tmp, strlen(tmp));
    }

      /**Notify debug port */
    target_mod = g_controller_map[CLIENT_DEBUG_NAME];
    if(target_mod != NULL) {
        send_cmd(target_mod->socket_fd, CLIENT_DEBUG_NAME, CMD_RESULT, NULL, tmp, strlen(tmp));
    }

}

/**
*	  Handle ctrl command from client
*/
int handle_ctr_msg(msg_t * msg, module_info * mod) {

    hash_map < string, string > params;
    msg_t resp;
    int sock_fd = -1;

    if(msg == NULL || mod == NULL) {
        ALOGE("%s: Invalid param, NULL pointer", __FUNCTION__);
        return -1;
    }

    ALOGI("%s: start deal command[%s,%s,%s],send resp to:%d socket ", __FUNCTION__,
          msg->module, msg->subcmd, msg->msg, mod->socket_fd);

    sock_fd = mod->socket_fd;

    memcpy(&resp, msg, sizeof(msg_t));
    parse_parameter(msg->msg, params);

    if(!strcmp(msg->subcmd, SUBCMD_STAT)) {
        do_ctrl_stat(&resp);
    } else if(!strcmp(msg->subcmd, SUBCMD_RECONFIG)) {
        do_ctrl_reconfig(&resp, params[KEY_CFG_PATH].c_str());
    } else if(!strcmp(msg->subcmd, SUBCMD_CLEAR)) {
        do_ctrl_clear(&resp);
    } else if(!strcmp(msg->subcmd, SUBCMD_RUNCASE)) {
        do_ctrl_runcase(&resp, params[KEY_CASE_NAME]);
    } else if(!strcmp(msg->subcmd, SUBCMD_RUNALL)) {
        do_ctrl_runall(&resp);
    } else if(!strcmp(msg->subcmd, SUBCMD_LISTCASE)) {
        do_ctrl_listcase(&resp, params[KEY_TESTLIST_PATH].c_str());
    } else if(!strcmp(msg->subcmd, SUBCMD_EXITCASE)) {
        do_ctrl_exitcase(&resp);
    } else {
        resp.result = FAILED;
    }

   /**Reconfig will refresh all module_info*/
    if(!strcmp(msg->subcmd, SUBCMD_RECONFIG)) {
        sock_fd = (g_controller_map[CLIENT_DIAG_NAME])->socket_fd;
    }
    ALOGI("%s: Finished command[%s,%s,%s],send resp[result=%d,%s] to:%d socket finished", __FUNCTION__,
          msg->module, msg->subcmd, msg->msg, resp.result,resp.msg,sock_fd);

    send_msg(sock_fd, &resp);
    return 0;
}
