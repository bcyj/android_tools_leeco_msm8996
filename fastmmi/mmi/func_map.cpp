/*
 * Copyright (c) 2014-2015, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#include "common.h"
#include "func_map.h"
#include "mmi.h"
#include "draw.h"
#include "utils.h"
#include "module.h"
#include "lang.h"

static sem_t g_sem_confirm;
static sem_t g_sem_mod_complete;
static bool is_ok = true;
static reboot_opt_t reboot_opt = REBOOT_NONE;

void switch_module(void *m) {

    module_info *mod = (module_info *) m;
    layout *lay = NULL;

    if(mod == NULL || mod->config_list[KEY_LAYOUT].empty()) {
        ALOGE("No param error");
        return;
    }
    lay = g_layout_map[mod->config_list[KEY_LAYOUT]];
    if(lay == NULL) {
        ALOGE("No layout for :%s", mod->config_list[KEY_LAYOUT].c_str());
        return;
    }

    lay = switch_cur_layout_locked(lay, mod);
    ALOGI("[%s] layout:%s,it is path:%s,btn(%d)", mod->module, mod->config_list[KEY_LAYOUT].c_str(),
          lay->get_layout_path(), lay->button_list.size());


    if(!strcmp(MAIN_MODULE, mod->module)) {
        update_main_status();
    } else {
        initial_screen(mod);
        module_exec_ui(mod);
    }
    invalidate();

    ALOGI("== in %s ==\n", mod->module);
}

static void do_page_up(void *m) {
    layout *curlay = acquire_cur_layout();

    if(curlay->m_listview != NULL) {
        curlay->m_listview->page_up();
    }
    invalidate();
    release_cur_layout();
}

static void do_page_down(void *m) {
    layout *curlay = acquire_cur_layout();

    if(curlay->m_listview != NULL) {
        curlay->m_listview->page_down();
    }
    invalidate();
    release_cur_layout();
}

static void do_show_fail(void *m) {
    layout *curlay = acquire_cur_layout();
    button *btn = NULL;

    if(curlay != NULL && curlay->m_listview != NULL) {
        ALOGI("show only fail cases");
        curlay->m_listview->set_show_fail(1);

        btn = curlay->find_button_by_name(KEY_MAIN_ALL);
        if(btn != NULL)
            btn->set_color(0x000000ff);

        btn = curlay->find_button_by_name(KEY_MAIN_FAILED);
        if(btn != NULL)
            btn->set_color(0x666666ff);
    }
    invalidate();
    release_cur_layout();
}

static void do_show_all(void *m) {
    layout *curlay = acquire_cur_layout();
    button *btn = NULL;

    if(curlay->m_listview != NULL) {
        ALOGI("showall cases");
        curlay->m_listview->set_show_fail(0);

        btn = curlay->find_button_by_name(KEY_MAIN_ALL);
        if(btn != NULL)
            btn->set_color(0x666666ff);

        btn = curlay->find_button_by_name(KEY_MAIN_FAILED);
        if(btn != NULL)
            btn->set_color(0x000000ff);
    }
    invalidate();
    release_cur_layout();
}

static void process_exit(void *m, int result) {

    if(m == NULL)
        return;

    module_info *mod = (module_info *) m;

    time(&mod->last_time);
    mod->duration = difftime(mod->last_time, mod->start_time);
    mod->result = result;
    flush_result();
    module_cleanup(mod);
    ALOGI("[%s]  endup with result =%d ", mod->module, result);
    launch_main();
    usleep(100);
    sem_post(&g_sem_mod_complete);
}
static void do_pass(void *m) {
    process_exit(m, SUCCESS);
}

static void do_fail(void *m) {
    process_exit(m, FAILED);
}

static void do_exit(void *m) {
    launch_main();
}

static bool pop_confirm(void *m, char *lay_name, const char *note) {

    if(m == NULL || lay_name == NULL) {
        ALOGE("[%s] NULL param ", __FUNCTION__);
        return false;
    }
    sem_init(&g_sem_confirm, 0, 0);
    module_info *mod = (module_info *) m;

    layout *lay = g_layout_map[(string) lay_name];

    if(lay == NULL) {
        ALOGE("No layout for :%s", lay_name);
        return false;
    }
    switch_cur_layout_locked(lay, mod);
    button *display = lay->find_button_by_name(KEY_DISPLAY);

    if(display != NULL && note != NULL) {
        display->set_text(note);
    }
    invalidate();

    /**waiting for confirm from user*/
    sem_wait(&g_sem_confirm);

    /**switch back to the previous screen*/
    lay = g_layout_map[mod->config_list[KEY_LAYOUT]];
    if(lay == NULL) {
        ALOGE("No layout for :%s", mod->config_list[KEY_LAYOUT].c_str());
        return false;
    }
    switch_cur_layout_locked(lay, mod);
    invalidate();

    return is_ok;
}

static void invalidate_button(reboot_opt_t opt) {

    int color_org = 0x7D7D7Dff;
    int color = 0x007D7Dff;

    layout *lay = g_layout_map[LAYOUT_REBOOT];

    if(lay == NULL) {
        ALOGE("No layout for :%s", LAYOUT_CONFIRM);
        return;
    }

    button *btn_poweroff = lay->find_button_by_name(KEY_REBOOT_POWEROFF);
    button *btn_ffbm = lay->find_button_by_name(KEY_REBOOT_FFBM);
    button *btn_android = lay->find_button_by_name(KEY_REBOOT_ANDROID);

    if(btn_android == NULL || btn_poweroff == NULL || btn_ffbm == NULL)
        return;

    if(opt == REBOOT_POWEROFF) {
        btn_poweroff->set_color(color);
        btn_ffbm->set_color(color_org);
        btn_android->set_color(color_org);
    } else if(opt == REBOOT_FFBM) {
        btn_poweroff->set_color(color_org);
        btn_ffbm->set_color(color);
        btn_android->set_color(color_org);
    } else if(opt == REBOOT_ANDROID) {
        btn_poweroff->set_color(color_org);
        btn_ffbm->set_color(color_org);
        btn_android->set_color(color);
    }

    invalidate();
}
static void onchange_reboot_android(void *m) {
    ALOGI("[%s] start,", __FUNCTION__);
    reboot_opt = REBOOT_ANDROID;
    invalidate_button(REBOOT_ANDROID);
}


static void onchange_reboot_ffbm(void *m) {
    ALOGI("[%s] start,", __FUNCTION__);
    reboot_opt = REBOOT_FFBM;
    invalidate_button(REBOOT_FFBM);
}

static void onchange_poweroff(void *m) {
    ALOGI("[%s] start,", __FUNCTION__);
    reboot_opt = REBOOT_POWEROFF;
    invalidate_button(REBOOT_POWEROFF);
}


static void do_ok(void *m) {
    ALOGI("[%s] start,", __FUNCTION__);
    is_ok = true;
    sem_post(&g_sem_confirm);
}

static void do_cancel(void *m) {
    ALOGI("[%s] start,", __FUNCTION__);
    is_ok = false;
    sem_post(&g_sem_confirm);
}

static void *process_runall(void *m) {
    ALOGI("[%s] start,", __FUNCTION__);

    if(m == NULL)
        return NULL;

    module_info *mod = (module_info *) m;

    sem_init(&g_sem_mod_complete, 0, 0);
    layout *lay = g_layout_map[mod->config_list[KEY_LAYOUT]];

    if(lay == NULL) {
        ALOGE("No layout for :%s", mod->config_list[KEY_LAYOUT].c_str());
        return NULL;
    }

    button *btn = lay->find_button_by_name(KEY_MAIN_RUNALL);

    if(btn != NULL) {
        if(btn->get_disabled()) {
            ALOGI("The previous RUN ALL action is not finished, please wait");
            return NULL;
        } else {
            btn->set_disabled(true);
        }
    }
   /**start all automation cases*/
    start_all(true);
    ALOGI("%s,start manual test.", __FUNCTION__);

  /**start manually test cases*/
    if(lay != NULL && lay->m_listview != NULL && lay->m_listview->get_items() != NULL) {

        list < item_t * >*items = lay->m_listview->get_items();
        list < item_t * >::iterator iter;
        for(iter = items->begin(); iter != items->end(); iter++) {
            item_t *item = (item_t *) (*iter);
            module_info *tmod = item->mod;

            if(tmod != NULL && !tmod->config_list[KEY_AUTOMATION].compare("0")) {
                usleep(100);
                switch_module(tmod);
                ALOGI("%s:testing %s", __FUNCTION__, tmod->module);
                sem_wait(&g_sem_mod_complete);
                ALOGI("%s:testing %s, finished", __FUNCTION__, tmod->module);
            }
        }
    }

    if(btn != NULL)
        btn->set_disabled(false);

    return NULL;
}

void do_run_all(void *m) {
    ALOGI("[%s] start,", __FUNCTION__);
    int ret = -1;
    pthread_t pid;

    ret = pthread_create(&pid, NULL, process_runall, m);
    if(ret < 0) {
        ALOGE("%s:Can't create msg handle pthread: %s\n", __FUNCTION__, strerror(errno));
    }
}

static void *process_reboot(void *m) {
    ALOGI("[%s] start,", __FUNCTION__);

    if(pop_confirm(m, LAYOUT_REBOOT, get_string(KEY_REBOOT_NOTICE))) {
        ALOGI("[%s] confirmed,", __FUNCTION__);
        if(reboot_opt == REBOOT_POWEROFF) {
            system("reboot -p");
        } else if(reboot_opt == REBOOT_FFBM) {
            system("reboot");
        } else if(reboot_opt == REBOOT_ANDROID) {
            set_boot_mode(BOOT_MODE_NORMAL);
            system("reboot");
        }
    } else {
        ALOGI("[%s] cancelled,", __FUNCTION__);
    }
    return NULL;
}

static void do_reboot(void *m) {
    ALOGI("[%s] start,", __FUNCTION__);
    int ret = -1;
    pthread_t pid;

    ret = pthread_create(&pid, NULL, process_reboot, m);
    if(ret < 0) {
        ALOGE("%s:Can't create msg handle pthread: %s\n", __FUNCTION__, strerror(errno));
    }
}

static void *process_reset(void *m) {
    ALOGI("[%s] start,", __FUNCTION__);

    if(m == NULL)
        return NULL;

    module_info *mod = (module_info *) m;
    layout *lay = g_layout_map[mod->config_list[KEY_LAYOUT]];

    if(lay == NULL) {
        ALOGE("No layout for :%s", mod->config_list[KEY_LAYOUT].c_str());
        return NULL;
    }

    if(pop_confirm(m, LAYOUT_CONFIRM, get_string(KEY_RESET_NOTICE))) {
        ALOGI("[%s] confirmed,", __FUNCTION__);
        if(lay != NULL && lay->m_listview != NULL) {
            lay->m_listview->reset_result();
        }
    } else {
        ALOGI("[%s] cancelled,", __FUNCTION__);
    }
    return NULL;
}

static void *process_report(void *m) {
    ALOGI("[%s] start,", __FUNCTION__);
    rect_t rect;
    int w = gr_fb_width();
    int h = gr_fb_height();
    int temp_x = 0;
    int temp_y = 57 * h / 100;
    module_info *mod = NULL;
    layout *lay = NULL;

    list < item_t * >*item_list = NULL;
    item_t *obj = NULL;

    if(m == NULL)
        return NULL;

    mod = (module_info *) m;

    lay = g_layout_map[mod->config_list[KEY_LAYOUT]];
    if(lay == NULL) {
        ALOGE("No layout for :%s", mod->config_list[KEY_LAYOUT].c_str());
        return NULL;
    }

    item_list = lay->m_listview->get_items();
    if(item_list == NULL) {
        ALOGE("Cannot get item list");
        return NULL;
    }

    button *btn[lay->m_listview->get_item_num()];

    sem_init(&g_sem_confirm, 0, 0);

    lay = g_layout_map[(string) LAYOUT_REPORT];
    if(lay == NULL) {
        ALOGE("No layout for :%s", LAYOUT_REPORT);
        return NULL;
    }

    switch_cur_layout_locked(lay, mod);

    list < item_t * >::iterator iter;
    for(iter = item_list->begin(); iter != item_list->end(); iter++) {
        obj = (item_t *) (*iter);

    /**if the module already tested*/
        if(obj != NULL && obj->mod != NULL && obj->mod->result == FAILED) {
            rect.x = temp_x;
            rect.y = temp_y;

            if(rect.x > w) {
                rect.x = 0;
                rect.y = temp_y + (7 * h / 100);
            }
            rect.w = 22 * w / 100;
            rect.h = 5 * h / 100;

            button *btn = new button(obj->mod->module, rect, "", NULL);

            btn->set_text(obj->mod->module);
            btn->set_color(COLOR_RED);
            lay->add_button_locked(btn);

            temp_x = rect.x + (26 * w / 100);
            temp_y = rect.y;
        }
    }

    invalidate();

    /**waiting for confirm from user*/
    sem_wait(&g_sem_confirm);

    int ret = 0;

    lay = acquire_cur_layout();
    for(iter = item_list->begin(); iter != item_list->end(); iter++) {
        obj = (item_t *) (*iter);
        if(lay != NULL && obj != NULL && obj->mod != NULL && obj->mod->result == FAILED) {
            ret = lay->delete_btn_by_name(obj->mod->module);
            if(!ret) {
                ALOGE("Fail to delete button %s", obj->mod->module);
            }
        }
    }
    release_cur_layout();

    /**switch back to the previous screen*/
    lay = g_layout_map[mod->config_list[KEY_LAYOUT]];
    if(lay == NULL) {
        ALOGE("No layout for :%s", mod->config_list[KEY_LAYOUT].c_str());
        return NULL;
    }
    switch_cur_layout_locked(lay, mod);
    invalidate();

    return NULL;
}

static void do_report(void *m) {
    ALOGI("[%s] start,", __FUNCTION__);
    int ret = -1;
    pthread_t pid;

    ret = pthread_create(&pid, NULL, process_report, m);
    if(ret < 0) {
        ALOGE("%s:Can't create msg handle pthread: %s\n", __FUNCTION__, strerror(errno));
    }
}

static void do_reset(void *m) {
    ALOGI("[%s] start,", __FUNCTION__);
    int ret = -1;
    pthread_t pid;

    ret = pthread_create(&pid, NULL, process_reset, m);
    if(ret < 0) {
        ALOGE("%s:Can't create msg handle pthread: %s\n", __FUNCTION__, strerror(errno));
    }
}

static void *process_extra_cmd(void *m) {
    if(m == NULL) {
        ALOGE("[%s] NULL param ", __FUNCTION__);
        return NULL;
    }

    module_info *mod = (module_info *) m;
    extra_cmd_t *cmd = &mod->extracmd;

    ALOGI("%s:cmd(valid:%d,size:%d)", __FUNCTION__, cmd->is_valid, cmd->size);
    if(cmd->is_valid == true && cmd->size > 0) {
        send_run_extra(mod, (cmd->cmd_list.front()).c_str());
    }

    return NULL;
}

static void do_extra_cmd(void *m) {
    ALOGI("[%s] start,", __FUNCTION__);
    int ret = -1;
    pthread_t pid;

    ret = pthread_create(&pid, NULL, process_extra_cmd, m);
    if(ret < 0) {
        ALOGE("%s:Can't create msg handle pthread: %s\n", __FUNCTION__, strerror(errno));
    }
}

static func_map_t func_list[] = {
    {"do_cancel", do_cancel},
    {"do_extra_cmd", do_extra_cmd},
    {"do_fail", do_fail},
    {"do_ok", do_ok},
    {"do_report", do_report},
    {"do_page_down", do_page_down},
    {"do_page_up", do_page_up},
    {"do_pass", do_pass},
    {"switch_module", switch_module},
    {"do_reboot", do_reboot},
    {"do_run_all", do_run_all},
    {"do_reset", do_reset},
    {"do_show_fail", do_show_fail},
    {"do_show_all", do_show_all},
    {"do_next", do_next},
    {"do_exit", do_exit},
    {"onchange_poweroff", onchange_poweroff},
    {"onchange_reboot_ffbm", onchange_reboot_ffbm},
    {"onchange_reboot_android", onchange_reboot_android},
};

static hash_map < string, cb_t > func_map;
void create_func_map() {
    uint32_t i = 0;

    for(i = 0; i < sizeof(func_list) / sizeof(func_map_t); i++) {
        func_map[(string) func_list[i].name] = func_list[i].cb;
    }
}

cb_t get_cb(string func_name) {
    return func_map[func_name];
}
cb_t get_cb(char *func_name) {
    if(func_name == NULL)
        return NULL;
    return func_map[(string) func_name];
}
