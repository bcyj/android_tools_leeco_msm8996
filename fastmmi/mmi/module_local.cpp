/*
 * Copyright (c) 2014-2015, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#include "common.h"
#include "module.h"
#include "utils.h"
#include "draw.h"
#include "input.h"
#include "mmi.h"

static pthread_t g_module_tid;
static sem_t g_local_module_sem;

static const char *local_module_name[] = {
    LOCAL_LCD,
    LOCAL_TOUCH,
    LOCAL_KEY,
    LOCAL_KEY_HEADSET
};

static key_map_t key_map[] = {
    {KEY_STR_HOME, NULL, KEY_HOME, false, false},
    {KEY_STR_MENU, NULL, KEY_MENU, false, false},
    {KEY_STR_BACK, NULL, KEY_BACK, false, false},
    {KEY_STR_VOLUMEDOWN, NULL, KEY_VOLUMEDOWN, false, false},
    {KEY_STR_VOLUMEUP, NULL, KEY_VOLUMEUP, false, false},
    {KEY_STR_POWER, NULL, KEY_POWER, false, false},
    {KEY_STR_SNAPSHOT, NULL, KEY_CAMERA_SNAPSHOT, false, false},
};

/**Check if test locally */
bool is_local_module(module_info * mod) {
    bool ret = false;
    uint32_t i = 0;

    if(mod == NULL)
        return false;

    for(i = 0; i < sizeof(local_module_name) / sizeof(char *); i++) {
        if(!strcmp(local_module_name[i], mod->config_list[KEY_LIB_NAME].c_str())) {
            ret = true;
            break;
        }
    }

    return ret;
}

/**Change the operation permission for PASS,FAIL button: 
*  enable = 1: enable operation for the buttons(both pass and fail button).
*  enable = 0: disable operation for the buttons(both pass and fail button).
**/
static void btn_enable(layout * lay, bool enable) {

    if(lay == NULL) {
        ALOGE("%s:No layout\n", __FUNCTION__);
        return;
    }

    button *pass_btn = lay->find_button_by_name(KEY_PASS);
    button *fail_btn = lay->find_button_by_name(KEY_FAIL);

    if(pass_btn != NULL) {
        if(enable)
            pass_btn->set_disabled(false);
        else
            pass_btn->set_disabled(true);
    }

    if(fail_btn != NULL) {
        if(enable)
            fail_btn->set_disabled(false);
        else
            fail_btn->set_disabled(true);
    }
}

/**Change the visibility for PASS,FAIL,DISPLAY button: 
*  visible = 1: dispaly button visible,pass and fail button invisible.
*  visible = 0: pass and fail button visible, display button invisible.
**/
static void change_visibility(layout * lay, int visible) {

    if(lay == NULL) {
        ALOGE("%s:No layout\n", __FUNCTION__);
        return;
    }

    button *pass_btn = lay->find_button_by_name(KEY_PASS);
    button *fail_btn = lay->find_button_by_name(KEY_FAIL);
    button *display_btn = lay->find_button_by_name(KEY_DISPLAY);

    if(pass_btn != NULL) {
        if(visible)
            pass_btn->set_visibility(false);
        else
            pass_btn->set_visibility(true);
    }

    if(fail_btn != NULL) {
        if(visible)
            fail_btn->set_visibility(false);
        else
            fail_btn->set_visibility(true);
    }

    if(display_btn != NULL) {
        if(visible)
            display_btn->set_visibility(true);
        else
            display_btn->set_visibility(false);
    }
}

/**========================================*/
/**LCD test*/
void do_next(void *m) {
    sem_post(&g_local_module_sem);
}

void launch_lcd_function(module_info * mod) {

    hash_map < string, string > params;
    int color = 0x00000000;

    /*Get the layout */
    layout *lay = g_layout_map[mod->config_list[KEY_LAYOUT]];

    if(lay == NULL) {
        ALOGE("[%s] No layout\n", mod->module);
        return;
    }
    /**switch layout to pcba and initial the module*/
    switch_cur_layout_locked(lay, mod);

    button *display_btn = lay->find_button_by_name(KEY_DISPLAY);

    if(display_btn == NULL)
        return;

    parse_parameter(mod->config_list["parameter"], params);
    if(!strcmp(params["color"].c_str(), "red")) {
        color = COLOR_RED;
    } else if(!strcmp(params["color"].c_str(), "blue")) {
        color = COLOR_BLUE;
    } else if(!strcmp(params["color"].c_str(), "green")) {
        color = COLOR_GREEN;
    } else if(!strcmp(params["color"].c_str(), "black")) {
        color = COLOR_BLACK;
    }

    change_visibility(lay, 1);

    display_btn->set_color(color);
    invalidate();
}

static void launch_local_lcd(module_info * mod) {
    /*Get the layout */
    layout *lay = g_layout_map[mod->config_list[KEY_LAYOUT]];

    if(lay == NULL) {
        ALOGE("[%s] No layout\n", mod->module);
        return;
    }

    button *display_btn = lay->find_button_by_name(KEY_DISPLAY);

    if(display_btn == NULL)
        return;

    change_visibility(lay, 1);

    display_btn->set_color(COLOR_RED);
    invalidate();
    sem_wait(&g_local_module_sem);

    display_btn->set_color(COLOR_GREEN);
    invalidate();
    sem_wait(&g_local_module_sem);

    display_btn->set_color(COLOR_BLUE);
    invalidate();
    sem_wait(&g_local_module_sem);

    display_btn->set_color(COLOR_WHITE);
    invalidate();
    sem_wait(&g_local_module_sem);

    display_btn->set_color(COLOR_BLACK);
    invalidate();
    sem_wait(&g_local_module_sem);

    change_visibility(lay, 0);

    invalidate();
}

/**KEY test*/
static void launch_local_key(module_info * mod) {
    unsigned int i = 0;

    hash_map < string, string > params;

    /**Get the layout */
    layout *lay = g_layout_map[mod->config_list[KEY_LAYOUT]];

    if(lay == NULL) {
        ALOGE("[%s] No layout\n", mod->module);
        return;
    }

    parse_parameter(mod->config_list["parameter"], params);
    for(i = 0; i < (sizeof(key_map) / sizeof(key_map_t)); i++) {
        if(strstr(params["keys"].c_str(), key_map[i].key_name) != NULL) {
            key_map[i].exist = true;
        }
    }

    color_t color = { 0, 125, 125, 255 };
    for(i = 0; i < (sizeof(key_map) / sizeof(key_map_t)); i++) {
        if(key_map[i].exist) {
            lay->set_button_color_by_name(key_map[i].key_name, &color);
        } else {
            key_map[i].key_btn = lay->find_button_by_name(key_map[i].key_name);
            if(key_map[i].key_btn != NULL)
                key_map[i].key_btn->set_visibility(false);
        }
    }

    invalidate();

      /**Register call back */
    input_listener *lis = new input_listener_key(mod, lay, NULL);

    if(lis != NULL)
        register_input_listener(lis);
}

/**TOUCH test*/
void launch_touch_function(module_info * mod) {
    /*Get the layout */
    layout *lay = g_layout_map[mod->config_list[KEY_LAYOUT]];

    if(lay == NULL) {
        ALOGE("[%s] No layout\n", mod->module);
        return;
    }

    /**switch layout to pcba and initial the module*/
    switch_cur_layout_locked(lay, mod);

    button *display_btn = lay->find_button_by_name(KEY_DISPLAY);

    if(display_btn == NULL)
        return;

    change_visibility(lay, 1);

    display_btn->set_color(COLOR_RED);
    invalidate();
    sem_wait(&g_local_module_sem);

    display_btn->set_color(COLOR_GREEN);
    invalidate();
    sem_wait(&g_local_module_sem);

    display_btn->set_color(COLOR_BLUE);
    invalidate();
}

void touch_complete_cb(void *m) {

    if(m == NULL)
        return;

    ALOGI("Touch complete callback invoke");
    module_info *mod = (module_info *) m;

    mod->result = SUCCESS;
    sem_post(&g_local_module_sem);
}

static void touch_mode_edges(layout * lay) {
    int w = gr_fb_width();
    int h = gr_fb_height();
    int row = 20 - 1;
    int col = 10 - 1;
    rect_t rect;
    button *btn;

    for(int i = 0; i < row; i += 2) {
        rect.x = 0;
        rect.y = i * h / row;
        rect.w = w / col;
        rect.h = h / row;

        btn = new button(KEY_BTN, rect, "", NULL);

        btn->set_color(0, 125, 125, 255);
        lay->add_button_locked(btn);
    }
    for(int i = 0; i < row; i += 2) {
        rect.x = 8 * w / col;
        rect.y = i * h / row;
        rect.w = w / col;
        rect.h = h / row;

        btn = new button(KEY_BTN, rect, "", NULL);

        btn->set_color(0, 125, 125, 255);
        lay->add_button_locked(btn);
    }
    for(int i = 0; i < col; i += 2) {
        rect.x = i * w / col;
        rect.y = 0;
        rect.w = w / col;
        rect.h = h / row;

        btn = new button(KEY_BTN, rect, "", NULL);

        btn->set_color(0, 125, 125, 255);
        lay->add_button_locked(btn);
    }
    for(int i = 0; i < col; i += 2) {
        rect.x = i * w / col;
        rect.y = 18 * h / row;
        rect.w = w / col;
        rect.h = h / row;

        btn = new button(KEY_BTN, rect, "", NULL);

        btn->set_color(0, 125, 125, 255);
        lay->add_button_locked(btn);
    }
}

static void set_btn(layout * lay, int w, int h, int x, int y) {
    int row = 20 - 1;
    int col = 10 - 1;

    rect_t rect;
    button *btn;

    if((x == 0) || (y == 0) || (x == w) || (y == h)) {
        if((x == 0) && (y == 0)) {
            rect.x = 0;
            rect.y = 0;
            rect.w = w / col;
            rect.h = h / row;

            btn = new button(KEY_BTN, rect, "", NULL);

            btn->set_color(0, 125, 125, 255);
            lay->add_button_locked(btn);
        }
        if((x == 0) && (y == h)) {
            rect.x = 0;
            rect.y = h - (h / row);
            rect.w = w / col;
            rect.h = h / row;

            btn = new button(KEY_BTN, rect, "", NULL);

            btn->set_color(0, 125, 125, 255);
            lay->add_button_locked(btn);
        }
        if((x == w) && (y == 0)) {
            rect.x = w - (w / col);
            rect.y = 0;
            rect.w = w / col;
            rect.h = h / row;

            btn = new button(KEY_BTN, rect, "", NULL);

            btn->set_color(0, 125, 125, 255);
            lay->add_button_locked(btn);

        }
        if((x == w) && (y == h)) {
            rect.x = w - (w / col);
            rect.y = h - (h / row);
            rect.w = w / col;
            rect.h = h / row;

            btn = new button(KEY_BTN, rect, "", NULL);

            btn->set_color(0, 125, 125, 255);
            lay->add_button_locked(btn);
        }
    } else {
        rect.x = x - (w / (2 * col));
        rect.y = y - (h / (2 * row));
        rect.w = w / col;
        rect.h = h / row;

        btn = new button(KEY_BTN, rect, "", NULL);

        btn->set_color(0, 125, 125, 255);
        lay->add_button_locked(btn);
    }
}

static void touch_mode_diagonal(layout * lay) {
    int w = gr_fb_width();
    int h = gr_fb_height();

    for(int i = 0; i < 9; i++) {
        set_btn(lay, w, h, w * i / 8, h * i / 8);
        set_btn(lay, w, h, w * i / 8, h * (8 - i) / 8);
    }
}


static void touch_mode_full(layout * lay) {
    int w = gr_fb_width();
    int h = gr_fb_width();
    int row = 20 - 1;
    int col = 10 - 1;
    rect_t rect;
    button *btn;

    for(int i = 0; i < row; i += 2) {
        for(int j = 0; j < col; j += 2) {
            rect.x = j * w / col;
            rect.y = i * h / row;
            rect.w = w / col;
            rect.h = h / row;

            btn = new button(KEY_BTN, rect, "", NULL);
            btn->set_color(0, 125, 125, 255);
            lay->add_button_locked(btn);
        }
    }
}

static void launch_local_touch(module_info * mod) {

    if(mod == NULL) {
        ALOGE("touch test mode not set ");
        return;
    }
    hash_map < string, string > params;

    /**Get the layout */
    layout *lay = g_layout_map[mod->config_list[KEY_LAYOUT]];

    if(lay == NULL) {
        ALOGE("[%s] No layout\n", mod->module);
        return;
    }

    change_visibility(lay, 1);

    /**Register call back */
    input_listener *lis = new input_listener_touch(mod, lay, touch_complete_cb);

    if(lis != NULL)
        register_input_listener(lis);

   /**build touch mode*/
    ALOGE("[%s] mod:%s\n", mod->module, mod->config_list["parameter"].c_str());

    parse_parameter(mod->config_list["parameter"], params);
    if(!strcmp(params["mode"].c_str(), "edges")) {
        touch_mode_edges(lay);
    } else if(!strcmp(params["mode"].c_str(), "full")) {
        touch_mode_full(lay);
    } else if(!strcmp(params["mode"].c_str(), "diagonal")) {
        touch_mode_edges(lay);
        touch_mode_diagonal(lay);
    }
    invalidate();

   /**Complete test*/
    sem_wait(&g_local_module_sem);
    ALOGI("touch test complete exit ");
    btn_enable(lay, false);
    change_visibility(lay, 0);
    invalidate();
    usleep(1000 * 1000);
    btn_enable(lay, true);
}

static void launch_local_headset(module_info * mod) {
    button *headphone_btn = NULL;
    button *microphone_btn = NULL;
    button *hangup_btn = NULL;

    hash_map < string, string > params;

    /**Get the layout */
    layout *lay = g_layout_map[mod->config_list[KEY_LAYOUT]];

    if(lay == NULL) {
        ALOGE("[%s] No layout\n", mod->module);
        return;
    }

    headphone_btn = lay->find_button_by_name(KEY_STR_HEADPHONE_INSERT);
    microphone_btn = lay->find_button_by_name(KEY_STR_MICROPHONE_INSERT);
    hangup_btn = lay->find_button_by_name(KEY_STR_HANGUP);

    color_t color = { 0, 125, 125, 255 };

    parse_parameter(mod->config_list["parameter"], params);

    lay->set_button_color_by_name(KEY_STR_HEADPHONE_INSERT, &color);
    lay->set_button_color_by_name(KEY_STR_MICROPHONE_INSERT, &color);
    lay->set_button_color_by_name(KEY_STR_HANGUP, &color);

    if(strstr(params["keys"].c_str(), "headphone") == NULL) {
        if(headphone_btn != NULL)
            headphone_btn->set_visibility(false);
    }
    if(strstr(params["keys"].c_str(), "microphone") == NULL) {
        if(microphone_btn != NULL)
            microphone_btn->set_visibility(false);
    }
    if(strstr(params["keys"].c_str(), "hangup") == NULL) {
        if(hangup_btn != NULL)
            hangup_btn->set_visibility(false);
    }

    invalidate();

      /**Register call back */
    input_listener *lis = new input_listener_key(mod, lay, NULL);

    if(lis != NULL)
        register_input_listener(lis);
}

/***/
static void *launch_local_module(void *mod) {

    if(mod == NULL)
        return NULL;

    /**Init sem*/
    sem_init(&g_local_module_sem, 0, 0);
    module_info *m = (module_info *) mod;

    ALOGD("=====launch local module: %s\n", m->module);
    if(!strcmp(LOCAL_LCD, m->config_list[KEY_LIB_NAME].c_str())) {
        launch_local_lcd(m);
    } else if(!strcmp(LOCAL_KEY, m->config_list[KEY_LIB_NAME].c_str())) {
        launch_local_key(m);
    } else if(!strcmp(LOCAL_TOUCH, m->config_list[KEY_LIB_NAME].c_str())) {
        launch_local_touch(m);
    } else if(!strcmp(LOCAL_KEY_HEADSET, m->config_list[KEY_LIB_NAME].c_str())) {
        launch_local_headset(m);
    }
    sem_close(&g_local_module_sem);
    return NULL;
}

/**Local module implementation, include LCD,KEY,TOUCH*/
int create_module_thread(module_info * mod) {
    int retval = -1;

    retval = pthread_create(&g_module_tid, NULL, launch_local_module, (void *) mod);
    if(retval < 0)
        return -1;

    return 0;
}
