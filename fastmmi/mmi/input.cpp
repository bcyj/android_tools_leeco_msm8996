/*
 * Copyright (c) 2013-2015, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#include "input.h"
#include "utils.h"
#include "layout.h"
#include "input_listener.h"
#include "module.h"
#include "mmi.h"

static sem_t g_sem_runable;
static runnable_queue_t g_runnable_queue;
static pthread_mutex_t runnable_mutex;

/**Only support one listener at one time*/
static input_listener *g_input_listener = NULL;
static pthread_mutex_t g_listener_mutex;
static union vkey_map g_key_map[MAX_KEYMAP_LINES];
static input_adjust_t g_input_adjust;

void register_input_listener(input_listener * listener) {

    if(listener == NULL)
        return;

    pthread_mutex_lock(&g_listener_mutex);
    /**Clean before register */
    if(g_input_listener != NULL)
        delete g_input_listener;

    g_input_listener = listener;
    pthread_mutex_unlock(&g_listener_mutex);
}

void unregister_input_listener() {
    pthread_mutex_lock(&g_listener_mutex);
    /**Clean */
    if(g_input_listener != NULL) {
        delete g_input_listener;

        g_input_listener = NULL;
    }
    pthread_mutex_unlock(&g_listener_mutex);
}

static bool invoke_listener(input_event ev) {
    bool ret = true;

    pthread_mutex_lock(&g_listener_mutex);
    if(g_input_listener != NULL)
        ret = g_input_listener->dispatch_event(ev);
    pthread_mutex_unlock(&g_listener_mutex);

    return ret;
}

static void enqueue_runnable_locked(runnable_queue_t * queue, runnable_t * r) {

    pthread_mutex_lock(&queue->lock);
    queue->queue.push_back(r);
    pthread_mutex_unlock(&queue->lock);
}

static void dequeue_runnable_locked(runnable_queue_t * queue, runnable_t ** r) {
    pthread_mutex_lock(&queue->lock);
    if(!queue->queue.empty()) {
        *r = queue->queue.front();
        queue->queue.pop_front();
    }
    pthread_mutex_unlock(&queue->lock);
}


static void process_touch_up() {
    layout *curlay = acquire_cur_layout();

    if(curlay->touch_list.size() > 0) {
        point_t tail_point = curlay->touch_list.back();

        ALOGI("check (%d,%d) in btn_list=%d\n", tail_point.x, tail_point.y, curlay->button_list.size());

        /**Button clicked*/
        list < button * >::iterator iter;
        for(iter = curlay->button_list.begin(); iter != curlay->button_list.end(); iter++) {
            button *btn = (button *) (*iter);
            rect_t *rect = btn->get_rect();

            if(btn->get_visibility() && !btn->get_disabled() && is_point_in_rect(tail_point, btn->get_rect())) {
                ALOGD("[%s] rect[%d,%d,%d,%d]\n", __FUNCTION__, rect->x, rect->y, rect->w, rect->h);
                if(btn->get_cb() != NULL) {
                    runnable_t *r = new runnable_t;

                    r->cb = btn->get_cb();
                    r->module = curlay->module;
                    enqueue_runnable_locked(&g_runnable_queue, r);
                    sem_post(&g_sem_runable);
                }
                break;
            }
        }

        /**List view item clicked*/
        if(curlay->m_listview != NULL && curlay->m_listview->get_cb() != NULL
           && curlay->m_listview->get_items() != NULL && is_point_in_rect(tail_point, curlay->m_listview->get_rect())) {

            list < item_t * >::iterator iter;
            for(iter = curlay->m_listview->get_items()->begin(); iter != curlay->m_listview->get_items()->end(); iter++) {
                item_t *obj = (item_t *) (*iter);

                if(is_point_in_rect(tail_point, &obj->rect)) {

                    runnable_t *r = new runnable_t;

                    r->cb = curlay->m_listview->get_cb();
                    r->module = obj->mod;
                    enqueue_runnable_locked(&g_runnable_queue, r);
                    sem_post(&g_sem_runable);
                }
            }
        }
    }

    curlay->touch_list.clear();

    release_cur_layout();
}

static int key_callback(int type, int code, int value) {
    int down = ! !value;

    if(type == EV_KEY) {
          /**Key Down*/
        if(code == KEY_BACK && down)
            ALOGI("KEYBACK down, return to main");

      /**Other keys*/
        if(down) {
            ALOGI("key:%d press down", code);
        } else {
            ALOGI("key:%d release", code);
        }

    }
    return 0;
}

static int touch_callback(int type, int code, int value) {

    static point_t last_point(-1, -1);
    static int x_last = -1;
    static int y_last = -1;

    int x = x_last;
    int y = y_last;

    if(type == EV_ABS) {
        if(code == ABS_X || code == ABS_MT_POSITION_X) {
            x_last = x = value;
        } else if(code == ABS_Y || code == ABS_MT_POSITION_Y) {
            y_last = y = value;
        } else if(code == ABS_MT_TRACKING_ID && value == 0xffffffff) {
            /**Touch up*/
            process_touch_up();
        }

    } else if(type == EV_SYN && code == SYN_REPORT) {

        point_t cur_point(x, y);

        if(!(last_point == cur_point)) {
            last_point = cur_point;

            layout *curlay = acquire_cur_layout();

            curlay->touch_list.push_back(last_point);
            release_cur_layout();
        }
    }

    return 0;
}

/**Handle virtual keys*/
static int vkey_to_keycode(union vkey_map *key_map, int x, int y) {
    int i;

    for(i = 0; i < MAX_KEYMAP_LINES; i++) {
        if((abs(x - key_map[i].map.center_x) <= (key_map[i].map.width >> 1)) &&
           (abs(y - key_map[i].map.center_y) <= (key_map[i].map.height >> 1))) {
            return key_map[i].map.key_code;
        }
    }

    return -1;
}

static int hook_vkey(input_event * ev, void *data) {

    static int x_last = -1;
    static int y_last = -1;
    static point_t last_point(-1, -1);
    int key_code = -1;

    int x = x_last;
    int y = y_last;
    point_t cur_point(-1, -1);

    if(ev->type == EV_ABS) {
        if(ev->code == ABS_X || ev->code == ABS_MT_POSITION_X) {
            x_last = x = ev->value;
        } else if(ev->code == ABS_Y || ev->code == ABS_MT_POSITION_Y) {
            y_last = y = ev->value;
        }
    } else if(ev->type == EV_SYN && ev->code == SYN_REPORT) {
        point_t cur_point(x, y);

        if(!(last_point == cur_point)) {
            last_point = cur_point;
            key_code = vkey_to_keycode((union vkey_map *) data, cur_point.x, cur_point.y);
            if(key_code > 0) {
                ALOGI("Got virtual keycode:%d\n", key_code);
                ev->type = EV_KEY;
                ev->code = key_code;
                ev->value = 1;
            }
        }
    }

    return 0;
}

static void adjust_ev(input_event * ev) {
    if(ev != NULL && ev->type == EV_ABS) {

        if(ev->code == ABS_X || ev->code == ABS_MT_POSITION_X) {
            ev->value = (ev->value * g_input_adjust.lcd_x) / (g_input_adjust.ts_x_max - g_input_adjust.ts_x_min);
        } else if(ev->code == ABS_Y || ev->code == ABS_MT_POSITION_Y) {
            ev->value = (ev->value * g_input_adjust.lcd_y) / (g_input_adjust.ts_y_max - g_input_adjust.ts_y_min);
        }
    }
}

int input_callback(int fd, uint32_t revents, void *data) {
    struct input_event ev;
    int retval;

    retval = ev_get_input(fd, revents, &ev);
    if(retval < 0)
        return -1;

    /**Adjust the value to match LCD resolution*/
    adjust_ev(&ev);

   /**Convert virtual key to KEY code*/
    hook_vkey(&ev, &g_key_map);

    /**Call listener, if return False mean stop here,
  * if return true mean continue process event.
  */
    if(!invoke_listener(ev))
        return 0;

    if(ev.type == EV_KEY) {
        key_callback(ev.type, ev.code, ev.value);
    } else if(ev.type == EV_ABS || ev.type == EV_SYN) {
        touch_callback(ev.type, ev.code, ev.value);
    }

    return 0;
}


static int parse_vkey_map(char *tp_name, union vkey_map *key_map) {
    char path[PATH_MAX];
    char *buffer;
    off_t length;
    int fd_map;
    char *str;
    char *save_ptr1, *save_ptr2;
    char *main_token;
    char *sub_token;
    char *substr;
    int val;
    int i, j;

    strlcpy(path, "/sys/board_properties/virtualkeys.", sizeof(path));
    strlcat(path, tp_name, sizeof(path));
    fd_map = open(path, O_RDONLY | O_CLOEXEC);
    if(fd_map < 0) {
        ALOGE("could not open virtual key map file:%s (%s)\n", path, strerror(errno));
        return -1;
    }

    length = lseek(fd_map, 0, SEEK_END);
    if(length < 0) {
        ALOGE("could not seek to the end of file:%s (%s)\n", path, strerror(errno));
        close(fd_map);
        return -1;
    }
    lseek(fd_map, 0, SEEK_SET);
    buffer = (char *) malloc((size_t) length);
    if(buffer == NULL) {
        ALOGE("malloc %ld bytes failed:(%s)\n", length, strerror(errno));
        close(fd_map);
        return -1;
    }

    if(read(fd_map, buffer, length) <= 0) {
        ALOGE("read from virtual key map file failed.(%s)\n", strerror(errno));
        close(fd_map);
        free(buffer);
        return -1;
    }
    buffer[length - 1] = '\0';
    /* Parse the virtual key map finally */
    for(str = buffer, i = 0;; str = NULL) {
        main_token = strtok_r(str, "\n", &save_ptr1);
        if(main_token == NULL)
            break;
        if(i >= MAX_KEYMAP_LINES) {
            ALOGE("lines exceeds max supported soft keys\n");
            break;
        }
        /* The comment line starts with '#' */
        if(main_token[0] == '#')
            continue;
        for(j = 0, substr = main_token;; substr = NULL, j++) {
            sub_token = strtok_r(substr, ":", &save_ptr2);
            if(sub_token == NULL)
                break;
            val = strtol(sub_token, NULL, 0);
            key_map[i].vkey_map_value[j] = val;
        }
        i++;
    }

    free(buffer);
    close(fd_map);
    return 0;
}


static int get_ts_resolution() {

    DIR *dir;
    struct dirent *de;
    int fd;
    int i = 0, j = 0;
    bool found = false;
    int max_event_index = 0;
    int index = 0;
    char buffer[80];
    unsigned long keyBitmask[BITS_TO_LONGS(KEY_MAX)];
    unsigned long absBitmask[BITS_TO_LONGS(ABS_MAX)];
    input_absinfo abs_x;
    input_absinfo abs_y;

    char filepath[256] = {
        0
    };
    dir = opendir("/dev/input");
    if(dir == 0)
        return -1;
    while((de = readdir(dir))) {
        ALOGE("/dev/input/%s\n", de->d_name);
        if(strncmp(de->d_name, "event", 5))
            continue;
        get_device_index(de->d_name, "event", &index);
        if(index > max_event_index)
            max_event_index = index;
        ALOGE("/dev/input/%s: max:%d, index:%d\n", de->d_name, max_event_index, index);
    }

    for(i = 0; i < max_event_index + 1; i++) {
        unsigned long ev_bits[BITS_TO_LONGS(EV_MAX)];

        snprintf(filepath, sizeof(filepath), "/dev/input/event%d", i);
        fd = open(filepath, O_RDONLY);
        if(fd < 0)
            continue;
        /* read the evbits of the input device */
        if(ioctl(fd, EVIOCGBIT(0, sizeof(ev_bits)), ev_bits) < 0) {
            close(fd);
            continue;
        }

        /* TODO: add ability to specify event masks. For now, just assume
         * that only EV_KEY and EV_REL event types are ever needed. */
        if(!test_bit(EV_KEY, ev_bits) && !test_bit(EV_REL, ev_bits)) {
            ALOGE("could not get EV_KEY for %d, %s\n", fd, strerror(errno));
            close(fd);
            continue;
        }

        /* read the evbits of the input device */
        if(ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(keyBitmask)), keyBitmask) < 0) {
            ALOGE("could not get keyBitmask for fd:%d, %s\n", fd, strerror(errno));
            close(fd);
            continue;
        }

        if(ioctl(fd, EVIOCGBIT(EV_ABS, sizeof(absBitmask)), absBitmask) < 0) {
            ALOGE("could not get absBitmask for fd:%d, %s\n", fd, strerror(errno));
            close(fd);
            continue;
        }
        /*See if this is a touch pad. Is this a new modern multi-touch driver */
        if(test_bit(ABS_MT_POSITION_X, absBitmask)
           && test_bit(ABS_MT_POSITION_Y, absBitmask)) {

            found = true;
            if(ioctl(fd, EVIOCGNAME(sizeof(buffer) - 1), &buffer) < 1) {
                ALOGE("could not get device name for fd:%d, %s\n", fd, strerror(errno));
                close(fd);
                continue;
            } else {
                buffer[sizeof(buffer) - 1] = '\0';
                memset(g_key_map, 0, sizeof(g_key_map));
                parse_vkey_map(buffer, g_key_map);
            }

            if(ioctl(fd, EVIOCGABS(ABS_MT_POSITION_X), &abs_x) < 0
               || ioctl(fd, EVIOCGABS(ABS_MT_POSITION_Y), &abs_y) < 0) {
                ALOGE("could not get ABS_MT_POSITION_X for fd:%d, %s\n", fd, strerror(errno));
                close(fd);
                continue;
            }
        }
        if(found) {
            g_input_adjust.ts_x_max = abs_x.maximum;
            g_input_adjust.ts_x_min = abs_x.minimum;
            g_input_adjust.ts_y_max = abs_y.maximum;
            g_input_adjust.ts_y_min = abs_y.minimum;
            g_input_adjust.lcd_x = gr_fb_width();
            g_input_adjust.lcd_y = gr_fb_height();
            g_input_adjust.valid = 1;
            ALOGI("touchscreen resolution:(%d,%d),lcd resolution(%d,%d)\n", abs_x.maximum,
                  abs_y.maximum, g_input_adjust.lcd_x, g_input_adjust.lcd_y);
            break;
        }
    }
    closedir(dir);
    if(!found)
        return -1;
    return 0;
}

void init_input() {
    pthread_mutex_init(&runnable_mutex, NULL);
    pthread_mutex_init(&g_listener_mutex, NULL);
    usleep(1000 * 1000);
    ev_init(input_callback, NULL);
    get_ts_resolution();
}

void *input_waiting_thread(void *) {
    signal(SIGUSR1, signal_handler);
    init_input();
    printf("input_waiting_thread started\n");
    while(1) {
        if(!ev_wait(-1))
            ev_dispatch();
    }
    return NULL;
}

void *input_handle_thread(void *) {
    signal(SIGUSR1, signal_handler);
    runnable_t *r;

    while(1) {
        sem_wait(&g_sem_runable);
        dequeue_runnable_locked(&g_runnable_queue, &r);
        if((r != NULL) && (r->cb != NULL) && (r->module != NULL)) {
            r->cb(r->module);
        }
    }
    return NULL;
}
