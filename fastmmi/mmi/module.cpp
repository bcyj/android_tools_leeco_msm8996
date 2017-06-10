/*
 * Copyright (c) 2014-2015, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#include "config.h"
#include "module.h"
#include "utils.h"
#include "mmi.h"
#include "draw.h"
#include "input.h"
#include "lang.h"

static GRSurface g_surface;
static unsigned char *cam_buf[MAX_CAM_PREVIEW_BUFFERS];
static pthread_t g_cam_preview_tid = -1;
static sem_t g_cam_frame_ready;
static int buf_index = 0;
static ipc_header_t *ipc_header;

static pthread_t g_module_tid;

static const char *module_stem_32[] = {
    MODULE_CAMERA,
};

/**If it only support 32bit*/
static bool is_32bit_module(module_info * mod) {
    bool ret = false;
    uint32_t i = 0;

    if(mod == NULL)
        return false;

    for(i = 0; i < sizeof(module_stem_32) / sizeof(char *); i++) {
        if(!mod->config_list[KEY_LIB_NAME].empty()
           && !strcmp(module_stem_32[i], mod->config_list[KEY_LIB_NAME].c_str())) {
            ret = true;
            break;
        }
    }

    return ret;
}

/**If it is camera module*/
bool is_camera_module(module_info * mod) {
    return mod != NULL && !mod->config_list[KEY_LIB_NAME].empty()
        && !strcmp(MODULE_CAMERA, mod->config_list[KEY_LIB_NAME].c_str());
}

/**Check lcd module*/
bool is_lcd_pcba_function(module_info * mod) {
    hash_map < string, string > params;

    if(mod == NULL || mod->config_list[KEY_LIB_NAME].empty())
        return false;

    if(!strcmp(LOCAL_LCD, mod->config_list[KEY_LIB_NAME].c_str())) {

        parse_parameter(mod->config_list["parameter"], params);
        if(!strcmp(params["level"].c_str(), PCBA_FUNCTION)) {
            return true;
        }
    }

    return false;
}

/**Check touch module*/
bool is_touch_pcba_function(module_info * mod) {
    hash_map < string, string > params;

    if(mod == NULL || mod->config_list[KEY_LIB_NAME].empty())
        return false;

    if(!strcmp(LOCAL_TOUCH, mod->config_list[KEY_LIB_NAME].c_str())) {

        parse_parameter(mod->config_list["parameter"], params);
        if(!strcmp(params["level"].c_str(), PCBA_FUNCTION)) {
            return true;
        }
    }

    return false;
}

/**Function called by others */
void launch_main() {
       /**Unregister all listener when return to main*/
    unregister_input_listener();
    switch_module(get_main_module());
}

void initial_screen(module_info * mod) {
    button *obj = NULL;

    if(mod == NULL)
        return;

    layout *curlay = acquire_cur_layout();

    if(curlay != NULL) {
        obj = curlay->find_button_by_name(KEY_STR_TITLE);
    }

    if(obj != NULL)
        obj->set_text(get_string(mod->config_list[KEY_DISPALY_NAME]));

    textview *tv = curlay->find_textview_by_name(KEY_DISPLAY);

    if(tv != NULL) {
        tv->set_text(get_string("notice"));
    }
    release_cur_layout();
}

/**Wrapped basical function for running specified cmd*/
static void send_cmd(module_info * mod, int cmd, const char *subcmd) {

    if(mod == NULL || mod->socket_fd < 0) {
        ALOGE("%s,Invalid command", __FUNCTION__);
        return;
    }

    ALOGI("SEND COMMAND [%s]: cmd[%d], through sock:%d, to process id:%d ", mod->module, cmd, mod->socket_fd, mod->pid);
    send_cmd(mod->socket_fd, mod->module, cmd, subcmd, mod->config_list["parameter"].c_str(),
             strlen(mod->config_list["parameter"].c_str()));
}

/** RUN MMI */
static void send_run_mmi(module_info * mod) {
    send_cmd(mod, CMD_RUN, SUBCMD_MMI);
}

/**RUN PCBA*/
static void send_run_pcba(module_info * mod) {
    send_cmd(mod, CMD_RUN, SUBCMD_PCBA);
}

/**RUN extra cmd*/
void send_run_extra(module_info * mod, const char *subcmd) {
    send_cmd(mod, CMD_RUN, subcmd);
}

/**Query*/
static void send_query(module_info * mod) {
    send_cmd(mod, CMD_QUERY, NULL);
}

/**INIT*/
static void send_init(module_info * mod) {
    send_cmd(mod, CMD_INIT, NULL);
}

/**DEINIT*/
static void send_deinit(module_info * mod) {
    send_cmd(mod, CMD_DEINIT, NULL);
}

/**STOP*/
static void send_stop(module_info * mod) {
    send_cmd(mod, CMD_STOP, NULL);
}

/**Fork module process*/
int fork_launch_module(module_info * mod) {
    char para[1024] = { 0 };
    char agent[64] = { 0 };
    char lib_path[64] = { 0 };
    if(mod == NULL)
        return -1;

#ifdef __LP64__
    if(is_32bit_module(mod)) {
        strlcpy(agent, MMI_AGENT32, sizeof(agent));
        strlcpy(lib_path, MMI_LIB_PATH32, sizeof(lib_path));
    } else {
        strlcpy(agent, MMI_AGENT64, sizeof(agent));
        strlcpy(lib_path, MMI_LIB_PATH64, sizeof(lib_path));
    }
#else
    strlcpy(agent, MMI_AGENT32, sizeof(agent));
    strlcpy(lib_path, MMI_LIB_PATH32, sizeof(lib_path));
#endif

    ALOGI("[ %s]fork module process by %s,%s", mod->module, agent, mod->config_list[KEY_LIB_NAME].c_str());
    snprintf(para, sizeof(para), "lib_path:%s%s;%s", lib_path,
             mod->config_list[KEY_LIB_NAME].c_str(), mod->config_list["parameter"].c_str());

    int pid = fork();

    if(pid == 0) {
        char *args[6] = { agent, "-m", mod->module, "-p", para, NULL };

        int res = execv(agent, args);

        if(res < 0) {
            mod->pid = -1;
            ALOGD("%s exec failed\n", agent);
            exit(1);
        }
    } else if(pid < 0) {
        ALOGD("fork failed\n");
        return -1;
    } else if(pid > 0) {
        mod->pid = pid;
        ALOGD("child_pid=%d\n", pid);
    }
    return 0;
}

static void *cam_start_preview(void *m) {
    ALOGI("%s\n", __FUNCTION__);
    signal(SIGUSR1, signal_handler);
    buf_index = 0;

    /**TODO prepare two buffers data */
    invalidate();
    usleep(1000 * 500);
    invalidate();
    usleep(1000 * 500);

    while(1) {

        ALOGI("%s, [ipc_sem] before wait\n", __FUNCTION__);
        sem_wait(&(ipc_header->buf_mutex_sem));

        if(ipc_header->buf_state[buf_index] != BUF_STATE_DONE) {
            ALOGE("%s, invalidate buf state?? index: %d, state: %d!!\n", __func__, buf_index,
                  ipc_header->buf_state[buf_index]);
            continue;
        }

        ALOGI("%s, displaying buffer idx: %d", __func__, buf_index);
        g_surface.data = cam_buf[buf_index];
        gr_blit_rgb(&g_surface, 0, 0, g_surface.width, g_surface.height, 0, 0);
        gr_flip();

        ipc_header->buf_state[buf_index] = BUF_STATE_EMPTY;

        /*Switch buffer */
        buf_index = (buf_index + 1) % MAX_CAM_PREVIEW_BUFFERS;
    }
    return NULL;
}

static void cam_stop_preview() {
    ALOGI("%s\n", __FUNCTION__);
    pthread_kill(g_cam_preview_tid, SIGUSR1);

    if(ipc_header != NULL)
        sem_destroy(&(ipc_header->buf_mutex_sem));

    if(cam_buf[0] != NULL) {
        munmap(cam_buf[0], g_surface.row_bytes * g_surface.height * MAX_CAM_PREVIEW_BUFFERS + sizeof(ipc_header_t));
    }
}

static void cam_initial_preview_size(const char *dimension, GRSurface * gr) {
    if(dimension != NULL && gr != NULL) {
        if(!strcmp(dimension, KEY_SMALL)) {
            gr->width = DIMENSION_SMALL_WIDTH;
            gr->height = DIMENSION_SMALL_HEIGHT;
        } else if(!strcmp(dimension, KEY_NORMAL)) {
            gr->width = DIMENSION_NORMAL_WIDTH;
            gr->height = DIMENSION_NORMAL_HEIGHT;
        } else if(!strcmp(dimension, KEY_LARGE)) {
            gr->width = DIMENSION_LARGE_WIDTH;
            gr->height = DIMENSION_LARGE_HEIGHT;
        } else {
            gr->width = DIMENSION_SMALL_WIDTH;
            gr->height = DIMENSION_SMALL_HEIGHT;
        }
    }
}

static int cam_prepare_preview(module_info * mod) {
    ALOGI("%s\n", __FUNCTION__);
    int fd = -1;
    int retval = -1;
    pthread_t pid;
    void *map;

    hash_map < string, string > params;
    parse_parameter(mod->config_list["parameter"], params);
    cam_initial_preview_size(params[KEY_DIMENSION].c_str(), &g_surface);
    g_surface.row_bytes = g_surface.width * 4;
    g_surface.pixel_bytes = 4;
    int preview_buf_size = g_surface.row_bytes * g_surface.height;

    int mmap_size = preview_buf_size * MAX_CAM_PREVIEW_BUFFERS;

    mmap_size += sizeof(ipc_header_t);

    fd = open(TEMP_MMAP_FILE, O_CREAT | O_RDWR | O_TRUNC, 00777);
    if(fd < 0) {
        ALOGE("Could not open %s:  %s\n", TEMP_MMAP_FILE, strerror(errno));
        return -1;
    }
    lseek(fd, mmap_size - 1, SEEK_SET);
    write(fd, "", 1);

    map = mmap(NULL, mmap_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(map == MAP_FAILED) {
        ALOGE("Could not mmap %s:  %s\n", TEMP_MMAP_FILE, strerror(errno));
        close(fd);
        return -1;
    }

    ALOGI("%s, mapped buffer @%p, size: %d\n", __func__, map, mmap_size);

    for(int i = 0; i < MAX_CAM_PREVIEW_BUFFERS; i++) {
        cam_buf[i] = (unsigned char *) map + preview_buf_size * i;
    }

    /**Initial point to the first buffer*/
    g_surface.data = cam_buf[0];

    ipc_header = (ipc_header_t *) ((unsigned char *) map + preview_buf_size * MAX_CAM_PREVIEW_BUFFERS);
    memset(ipc_header, 0, sizeof(ipc_header_t));
    for(int i = 0; i < MAX_CAM_PREVIEW_BUFFERS; i++) {
        ipc_header->buf_state[i] = BUF_STATE_EMPTY;
    }
    ALOGI("%s, ipc_header offset: %d\n", __func__, (unsigned char *) ipc_header - (unsigned char *) map);

    ALOGI("%s,dimension(%d,%d) mapped address:%p", __FUNCTION__, g_surface.width, g_surface.height, g_surface.data);
    close(fd);

    sem_init(&(ipc_header->buf_mutex_sem), 1, 0);

    retval = pthread_create(&g_cam_preview_tid, NULL, cam_start_preview, NULL);
    if(retval < 0)
        return -1;

    return 0;
}

void module_cleanup(module_info * mod) {
    layout *curlay = NULL;
    char temp[8] = { 0 };
    int i = 0;
    button *btn = NULL;
    bool ret = false;

    /*Send command to stop test */
    send_stop(mod);

    if(is_camera_module(mod)) {
        usleep(500 * 1000);
        cam_stop_preview();
    }

    pthread_kill(g_module_tid, SIGUSR1);

    if(strstr(mod->config_list[KEY_PARAMETER].c_str(), "magnetic") != NULL) {
        curlay = acquire_cur_layout();
        for(i = 0; i < ROUND_ANGLE; i++) {
            snprintf(temp, sizeof(temp), "%d", i);
            if(curlay != NULL) {
                ret = curlay->delete_btn_by_name(temp);
                if(!ret) {
                    ALOGE("Fail to delete button %s", temp);
                }
            }
        }
        release_cur_layout();
    }
}

void update_main_status() {

    uint32_t fail_num = 0;
    uint32_t pass_num = 0;
    uint32_t total_num = 0;
    uint32_t remain_num = 0;
    uint32_t batt_capacity = 0;
    char status[128] = { 0 };
    char buf[32] = { 0 };

    /*Check if it is MAIN screen */
    module_info *mod = get_main_module();

    if(mod == NULL) {
        ALOGE("Not main screen OR Null point\n");
        return;
    }

    /*Get the layout */
    layout *lay = g_layout_map[mod->config_list[KEY_LAYOUT]];

    if(lay == NULL || lay->m_listview == NULL) {
        ALOGE("[%s] No layout\n", mod->module);
        return;
    }

    button *btn = lay->find_button_by_name(KEY_MAIN_STATUS);

    if(btn == NULL) {
        ALOGE("[%s] No status button\n", mod->module);
        return;
    }

    list < item_t * >*item_list = lay->m_listview->get_items();

    total_num = item_list->size();
    pass_num = lay->m_listview->get_pass_count();
    fail_num = lay->m_listview->get_fail_count();

    remain_num = total_num - pass_num - fail_num;

    read_file("/sys/class/power_supply/battery/capacity", buf, sizeof(buf));
    snprintf(status, sizeof(status), "%d P | %d F  | %d L  | %d R  | %s %% B", pass_num, fail_num, remain_num,
             total_num, buf);
    btn->set_text(status);

}

static void sensor_gyroscope_indication(module_info * mod, layout * layout) {

}

static void parse_sensors_xyz_value(const char *line, char *x, int x_len, char *y, int y_len, char *z, int z_len) {
    if(line == NULL || x == NULL || y == NULL || z == NULL)
        return;
    string input(line);
    int split_index = 0;

    split_index = input.find_first_of('x');
    if(split_index > 0) {
        if(strlen(line) - split_index - 3 < x_len) {
            strlcpy(x, line + split_index + 3, x_len);
            x[strlen(line) - split_index - 1] = '\0';
        }
    }
    split_index = input.find_first_of('y');
    if(split_index > 0) {
        if(strlen(line) - split_index - 3 < y_len) {
            strlcpy(y, line + split_index + 3, y_len);
            y[strlen(line) - split_index - 1] = '\0';
        }
    }
    split_index = input.find_first_of('z');
    if(split_index > 0) {
        if(strlen(line) - split_index - 3 < z_len) {
            strlcpy(z, line + split_index + 3, z_len);
            z[strlen(line) - split_index - 1] = '\0';
        }
    }
}

static void sensor_accelerometer_indication(module_info * mod, layout * layout) {
    button *front_btn = NULL;
    button *back_btn = NULL;
    button *left_btn = NULL;
    button *right_btn = NULL;
    button *centre_btn = NULL;
    char x[SIZE_1K] = { 0 };
    char y[SIZE_1K] = { 0 };
    char z[SIZE_1K] = { 0 };
    double x_temp = 0;
    double y_temp = 0;
    double z_temp = 0;

    front_btn = layout->find_button_by_name(KEY_STR_FRONT);
    if(front_btn != NULL)
        front_btn->set_color(COLOR_RED);
    else {
        ALOGE("Fail to get front button point for accelerometer sensor");
        return;
    }

    back_btn = layout->find_button_by_name(KEY_STR_BACK);
    if(back_btn != NULL)
        back_btn->set_color(COLOR_RED);
    else {
        ALOGE("Fail to get back button point for accelerometer sensor");
        return;
    }

    left_btn = layout->find_button_by_name(KEY_STR_LEFT);
    if(left_btn != NULL)
        left_btn->set_color(COLOR_RED);
    else {
        ALOGE("Fail to get lef button point for accelerometer sensor");
        return;
    }

    right_btn = layout->find_button_by_name(KEY_STR_RIGHT);
    if(right_btn != NULL)
        right_btn->set_color(COLOR_RED);
    else {
        ALOGE("Fail to get right button point for accelerometer sensor");
        return;
    }

    centre_btn = layout->find_button_by_name(KEY_STR_CENTER);
    if(centre_btn != NULL)
        centre_btn->set_color(COLOR_RED);
    else {
        ALOGE("Fail to get center button point for accelerometer sensor");
        return;
    }

    do {
        sem_wait(&g_data_print_sem);
        parse_sensors_xyz_value(mod->data, x, sizeof(x), y, sizeof(y), z, sizeof(z));
        x_temp = atof(x);
        y_temp = atof(y);
        z_temp = atof(z);
        if(x_temp > 4)
            left_btn->set_color(COLOR_GREEN);
        else if(x_temp < -4)
            right_btn->set_color(COLOR_GREEN);
        else if(y_temp < -3)
            front_btn->set_color(COLOR_GREEN);
        else if(y_temp > 4)
            back_btn->set_color(COLOR_GREEN);
        else if(z_temp < 0)
            centre_btn->set_color(COLOR_GREEN);
    } while(1);
}

static void sensor_light_indication(module_info * mod, layout * layout) {
    button *fb_btn = NULL;
    double temp = 0;
    double current = 0;
    char indicator = '=';
    char name[SIZE_1K] = { 0 };
    char value[SIZE_1K] = { 0 };

    fb_btn = layout->find_button_by_name(KEY_STR_INDICATION);

    if(fb_btn != NULL) {
        fb_btn->set_text("Testing...");
        fb_btn->set_color(COLOR_BLUE);
    } else {
        ALOGE("Fail to get feedback button for light sensor");
        return;
    }

    do {
        temp = current;
        sem_wait(&g_data_print_sem);
        parse_value(mod->data, indicator, name, sizeof(name), value, sizeof(value));
        current = atof(value);
    } while((temp - current < 50) || (temp == 0));

    fb_btn->set_text("Test pass");
    fb_btn->set_color(COLOR_GREEN);
}

static int convert_orientation(float mag_x, float mag_y) {
    float azimuth;
    const float rad2deg = 180 / M_PI;
    int angle = 0;

    azimuth = atan2(-(mag_x), mag_y);
    angle = (int) (azimuth * rad2deg);

    return angle;
}

static void sensor_magnetometer_indication(module_info * mod, layout * layout) {
    int w = gr_fb_width();
    int h = gr_fb_height();

    int centre_x = w / 2;
    int centre_y = 2 * h / 3;
    int radius = w / 4;
    int origin_x = 0;
    int origin_y = 0;
    int i = 0;
    int temp = 0;
    char x[SIZE_1K] = { 0 };
    char y[SIZE_1K] = { 0 };
    char z[SIZE_1K] = { 0 };
    char angle[8] = { 0 };

    double x_temp = 0;
    double y_temp = 0;
    double z_temp = 0;

    rect_t rect;
    button *btn[ROUND_ANGLE];

    for(i = 0; i < ROUND_ANGLE; i++) {
        origin_x = radius * sin((i * M_PI / 180) + (M_PI / 2));
        origin_y = radius * cos((i * M_PI / 180) + (M_PI / 2));
        rect.x = centre_x + origin_x - ROUND_POINT_HALF_SIDE;
        rect.y = centre_y - origin_y - ROUND_POINT_HALF_SIDE;
        rect.w = ROUND_POINT_SIDE;
        rect.h = ROUND_POINT_SIDE;

        snprintf(angle, sizeof(angle), "%d", i);
        btn[i] = new button(angle, rect, "", NULL);
        btn[i]->set_color(COLOR_BLUE);
        layout->add_button_locked(btn[i]);
    }

    do {
        sem_wait(&g_data_print_sem);
        parse_sensors_xyz_value(mod->data, x, sizeof(x), y, sizeof(y), z, sizeof(z));
        x_temp = atof(x);
        y_temp = atof(y);
        i = convert_orientation(x_temp, y_temp);
        if((i >= 0) && (i <= 180) && (i != temp)) {
            origin_x = radius * sin((i * M_PI / 180) + (M_PI / 2));
            origin_y = radius * cos((i * M_PI / 180) + (M_PI / 2));
            rect.x = centre_x + origin_x - POINT_HALF_SIDE;
            rect.y = centre_y - origin_y - POINT_HALF_SIDE;
            rect.w = POINT_SIDE;
            rect.h = POINT_SIDE;
            btn[i]->set_rect(&rect);
            btn[i]->set_color(COLOR_RED);

            origin_x = radius * sin((temp * M_PI / 180) + (M_PI / 2));
            origin_y = radius * cos((temp * M_PI / 180) + (M_PI / 2));
            rect.x = centre_x + origin_x - ROUND_POINT_HALF_SIDE;
            rect.y = centre_y - origin_y - ROUND_POINT_HALF_SIDE;
            rect.w = ROUND_POINT_SIDE;
            rect.h = ROUND_POINT_SIDE;
            btn[temp]->set_rect(&rect);
            btn[temp]->set_color(COLOR_GREEN);
        } else if((i < 0) && (i >= -180) && (i != temp)) {
            i = i + ROUND_ANGLE;
            origin_x = radius * sin((i * M_PI / 180) + (M_PI / 2));
            origin_y = radius * cos((i * M_PI / 180) + (M_PI / 2));
            rect.x = centre_x + origin_x - POINT_HALF_SIDE;
            rect.y = centre_y - origin_y - POINT_HALF_SIDE;
            rect.w = POINT_SIDE;
            rect.h = POINT_SIDE;
            btn[i]->set_rect(&rect);
            btn[i]->set_color(COLOR_RED);
            origin_x = radius * sin((i * M_PI / 180) + (M_PI / 2));
            origin_y = radius * cos((i * M_PI / 180) + (M_PI / 2));
            rect.x = centre_x + origin_x - ROUND_POINT_HALF_SIDE;
            rect.y = centre_y - origin_y - ROUND_POINT_HALF_SIDE;
            rect.w = ROUND_POINT_SIDE;
            rect.h = ROUND_POINT_SIDE;
            btn[temp]->set_rect(&rect);
            btn[temp]->set_color(COLOR_GREEN);
        }
        temp = i;
    } while(1);
}

static void sensor_proximity_indication(module_info * mod, layout * layout) {
    button *fb_btn = NULL;
    double temp;
    char indicator = '=';
    bool max_pass = false;
    bool min_pass = false;
    char name[1024] = { 0 };
    char value[1024] = { 0 };

    fb_btn = layout->find_button_by_name(KEY_STR_INDICATION);

    if(fb_btn != NULL) {
        fb_btn->set_text("Testing...");
        fb_btn->set_color(COLOR_BLUE);
    } else {
        ALOGE("Fail to get feedback button for proximity sensor");
        return;
    }

    do {
        sem_wait(&g_data_print_sem);
        parse_value(mod->data, indicator, name, sizeof(name), value, sizeof(value));
        temp = atof(value);
        if(temp >= 5)
            max_pass = true;
        else if(temp < 2)
            min_pass = true;
    } while(!max_pass || !min_pass);

    fb_btn->set_text("Test pass");
    fb_btn->set_color(COLOR_GREEN);
}

static void launch_module_sensor(module_info * mod) {
    /**Get the layout */
    layout *lay = g_layout_map[mod->config_list[KEY_LAYOUT]];

    if(lay == NULL) {
        ALOGE("[%s] No layout\n", mod->module);
        return;
    }

    if(strstr(mod->config_list[KEY_PARAMETER].c_str(), "gyroscope") != NULL)
        sensor_gyroscope_indication(mod, lay);
    else if(strstr(mod->config_list[KEY_PARAMETER].c_str(), "accelermeter") != NULL)
        sensor_accelerometer_indication(mod, lay);
    else if(strstr(mod->config_list[KEY_PARAMETER].c_str(), "light") != NULL)
        sensor_light_indication(mod, lay);
    else if(strstr(mod->config_list[KEY_PARAMETER].c_str(), "magnetic") != NULL)
        sensor_magnetometer_indication(mod, lay);
    else if(strstr(mod->config_list[KEY_PARAMETER].c_str(), "proximity") != NULL)
        sensor_proximity_indication(mod, lay);
}

static void launch_module_bluetooth(module_info * mod) {
    button *fb_btn = NULL;

    /**Get the layout */
    layout *lay = g_layout_map[mod->config_list[KEY_LAYOUT]];

    if(lay == NULL) {
        ALOGE("[%s] No layout\n", mod->module);
        return;
    }

    fb_btn = lay->find_button_by_name(KEY_STR_INDICATION);

    if(fb_btn != NULL) {
        fb_btn->set_text("Testing...");
        fb_btn->set_color(COLOR_BLUE);

        sem_wait(&g_result_sem);
        if(mod->result == SUCCESS) {
            fb_btn->set_text("Test pass");
            fb_btn->set_color(COLOR_GREEN);
        } else {
            fb_btn->set_text("Test fail");
            fb_btn->set_color(COLOR_RED);
        }
    }
}

static void launch_module_wifi(module_info * mod) {
    button *fb_btn = NULL;

        /**Get the layout */
    layout *lay = g_layout_map[mod->config_list[KEY_LAYOUT]];

    if(lay == NULL) {
        ALOGE("[%s] No layout\n", mod->module);
        return;
    }

    fb_btn = lay->find_button_by_name(KEY_STR_INDICATION);

    if(fb_btn != NULL) {
        fb_btn->set_text("Testing...");
        fb_btn->set_color(COLOR_BLUE);

        sem_wait(&g_result_sem);
        if(mod->result == SUCCESS) {
            fb_btn->set_text("Test pass");
            fb_btn->set_color(COLOR_GREEN);
        } else {
            fb_btn->set_text("Test fail");
            fb_btn->set_color(COLOR_RED);
        }
    }
}

static void launch_module_gps(module_info * mod) {
    button *fb_btn = NULL;

        /**Get the layout */
    layout *lay = g_layout_map[mod->config_list[KEY_LAYOUT]];

    if(lay == NULL) {
        ALOGE("[%s] No layout\n", mod->module);
        return;
    }

    fb_btn = lay->find_button_by_name(KEY_STR_INDICATION);

    if(fb_btn != NULL) {
        fb_btn->set_text("Testing...");
        fb_btn->set_color(COLOR_BLUE);

        sem_wait(&g_result_sem);
        if(mod->result == SUCCESS) {
            fb_btn->set_text("Test pass");
            fb_btn->set_color(COLOR_GREEN);
        } else {
            fb_btn->set_text("Test fail");
            fb_btn->set_color(COLOR_RED);
        }
    }
}

static void *launch_module_indication(void *mod) {
    if(mod == NULL)
        return NULL;

    module_info *m = (module_info *) mod;

    ALOGD("=====launch module indication: %s\n", m->module);
    if(!strcmp(MODULE_SENSOR, m->config_list[KEY_LIB_NAME].c_str())) {
        launch_module_sensor(m);
    } else if(!strcmp(MODULE_BLUETOOTH, m->config_list[KEY_LIB_NAME].c_str())) {
        launch_module_bluetooth(m);
    } else if(!strcmp(MODULE_WIFI, m->config_list[KEY_LIB_NAME].c_str())) {
        launch_module_wifi(m);
    } else if(!strcmp(MODULE_GPS, m->config_list[KEY_LIB_NAME].c_str())) {
        launch_module_gps(m);
    }
    return NULL;
}

/***/
int create_module_indication_thread(module_info * mod) {
    int retval = -1;

    retval = pthread_create(&g_module_tid, NULL, launch_module_indication, (void *) mod);
    if(retval < 0)
        return -1;
    return 0;
}

void module_exec_ui(module_info * mod) {

    if(mod == NULL)
        return;

    /**Record the start time*/
    time(&(mod->start_time));

    if(!is_local_module(mod)) {
     /**Special deal with Camera case*/
        if(is_camera_module(mod)) {
            cam_prepare_preview(mod);
        }
        /**Query extra command*/
        if(mod->extracmd.is_valid == false)
            send_query(mod);

        send_run_mmi(mod);
        create_module_indication_thread(mod);
    } else {
        create_module_thread(mod);
    }
}

void module_exec_pcba(module_info * mod) {

    if(mod == NULL)
        return;

    /**Record the start time*/
    time(&(mod->start_time));

    if(is_lcd_pcba_function(mod)) {
        launch_lcd_function(mod);
    } else if(is_touch_pcba_function(mod)) {
        launch_touch_function(mod);
    } else {
        send_run_pcba(mod);
    }
}
