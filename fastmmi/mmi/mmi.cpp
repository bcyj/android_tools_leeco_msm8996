/*
 * Copyright (c) 2013-2015, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#include "utils.h"
#include "config.h"
#include "input.h"
#include "draw.h"
#include "mmi.h"
#include "module.h"
#include "controller.h"
#include "lang.h"

static const char *boot_mode_string[] = {
    "normal",
    "ffbm-00",
    "ffbm-01",
    "ffbm-02"
};

static const char *test_mode_string[] = {
    "none",
    "pcba",
    "ui",
};

/**global export var*/
char g_res_file[PATH_MAX] = { 0 };

/*
 * This is the layout pool,module could find
 * it's own layout via "layout" param
 * Key:    module name
 * Value:  layout
 */
hash_map < string, layout * >g_layout_map;

/*
 * used to store all modules
 * key:module name
 */
hash_map < string, module_info * >g_modules_map;
/*
 * used to store diag,debug module
 */
hash_map < string, module_info * >g_controller_map;

/*
 * g_ordered_modules is the same thing as g_modules,
 * the different is sorted or not
 */
static list < module_info * >g_ordered_modules;

/*
 * g_nodup_modules: it is a duplicate hast_set from g_ordered_modules with group the module with
 * the same lib_name.
 * Key: lib_name
 * Value: module_info
 */
static hash_map < string, module_info * >g_nodup_modules_map;

/*
 * used to store modules and it's sock id
 * key:module name
 */
hash_map < string, int >g_sock_client_map;

sem_t g_data_print_sem;
sem_t g_result_sem;

/* used for clients that launched successfully;
 * Its content is part of g_modules
 */
static list < module_info * >g_clients;
static pthread_t g_input_waiting_tid;
static pthread_t g_input_handle_tid;
static pthread_t g_draw_tid;
static pthread_t g_accept_tid;
static pthread_t g_waiting_event_tid;
static pthread_t g_msg_handle_tid;
static sem_t g_sem_exit;
static sem_t g_sem_accept_ready;
static int g_max_fd = -1;

static msg_queue_t g_msg_queue;
static sem_t g_msg_sem;

static layout *g_cur_layout = NULL;
static pthread_mutex_t g_cur_layout_mutex;
static module_info *g_main_module = NULL;
static module_mode_t g_test_mode = TEST_MODE_NONE;

module_info *get_main_module() {
    return g_main_module;
}

void set_main_module(module_info * mod) {
    g_main_module = mod;
}

layout *acquire_cur_layout() {
    pthread_mutex_lock(&g_cur_layout_mutex);
    return g_cur_layout;
}

void release_cur_layout() {
    pthread_mutex_unlock(&g_cur_layout_mutex);
}

layout *switch_cur_layout_locked(layout * lay, module_info * mod) {

    acquire_cur_layout();
    ALOGI("switch layout from:%p to %p \n", g_cur_layout, lay);
    if(lay != NULL) {
        g_cur_layout = lay;
        g_cur_layout->module = mod;
    }
    release_cur_layout();
    return g_cur_layout;
}

void set_boot_mode(boot_mode_type mode) {
    int fd;
    const char *dev = NULL;

    module_info *mod = get_main_module();

    if(mod == NULL || mod->config_list["misc_dev"].empty()) {
        ALOGE("No main module error");
        return;
    }

    dev = mod->config_list["misc_dev"].c_str();
    fd = open(dev, O_WRONLY);
    if(fd < 0) {
        ALOGE("open misc fail");
    } else {
        if(write(fd, boot_mode_string[mode], strlen(boot_mode_string[mode])) != strlen(boot_mode_string[mode])) {
            ALOGE("write misc fail ");
        }
        fsync(fd);
        close(fd);
    }
}

/**Flush result file*/
void flush_result() {
    char tmp[SIZE_1K] = { 0 };
    FILE *fp = NULL;

    if(g_main_module == NULL)
        return;

    /*Get the layout */
    layout *lay = g_layout_map[g_main_module->config_list[KEY_LAYOUT]];

    if(lay == NULL || lay->m_listview == NULL) {
        ALOGE(" No layout\n");
        return;
    }
    list < item_t * >*item_list = lay->m_listview->get_items();

    fp = fopen(g_res_file, "w+");
    if(fp == NULL)
        return;

    ALOGI("start to update result : %s", g_res_file);

    list < item_t * >::iterator iter;
    for(iter = item_list->begin(); iter != item_list->end(); iter++) {
        item_t *obj = (item_t *) (*iter);

        /**if the module already tested*/
        if(obj != NULL && obj->mod != NULL && obj->mod->result != ERR_UNKNOW) {
            snprintf(tmp, sizeof(tmp), "\n[%s]\n%s = %lu\n%s = %s\n%s = %3.4f\n", obj->mod->module, KEY_TIMESTAMP_WORDS,
                     obj->mod->last_time, KEY_RESULT_WORDS, obj->mod->result == SUCCESS ? KEY_PASS : KEY_FAIL,
                     KEY_TESTTIME_WORDS, obj->mod->duration);
            strlcat(tmp, obj->mod->data, sizeof(tmp));
            fwrite(tmp, sizeof(char), strlen(tmp), fp);
        }
    }

    if(fp != NULL)
        fclose(fp);
}

/**Restore latest result */
static int restore_result(char *filepath) {
    char module[1024] = { 0, };
    char line[1024] = { 0, };
    char indicator = '=';
    module_info *cur_mod = NULL;

    if(filepath == NULL)
        return -1;

    FILE *file = fopen(filepath, "r");

    if(file == NULL) {
        ALOGE("%s open failed\n", filepath);
        return -1;
    }

    ALOGD("Parse result file: %s\n", filepath);
    while(fgets(line, sizeof(line), file) != NULL) {
        char name[1024] = { 0, }, value[1024] = {
        0,};

        if(line[0] == '#')
            continue;

        if(line[0] == '[') {
            parse_module(line, module, sizeof(module));
            cur_mod = g_modules_map[(string) module];
            ALOGD("[%s]\n", module);
            if(cur_mod != NULL)
                memset(cur_mod->data, 0, sizeof(cur_mod->data));    //initial the data field
            continue;
        }

        if(module[0] != '\0' && cur_mod != NULL) {
            parse_value(line, indicator, name, sizeof(name), value, sizeof(value));
            char *pname = trim(name);
            char *pvalue = trim(value);

            if(*pname != '\0' && *pvalue != '\0') {

                if(!strcmp(pname, KEY_TIMESTAMP_WORDS)) {
                    cur_mod->last_time = string_to_ulong(pvalue);
                } else if(!strcmp(pname, KEY_RESULT_WORDS)) {
                    if(!strcmp(pvalue, KEY_PASS))
                        cur_mod->result = SUCCESS;
                    else
                        cur_mod->result = FAILED;
                } else if(!strcmp(pname, KEY_TESTTIME_WORDS)) {
                    cur_mod->start_time = cur_mod->last_time;
                } else {
                    strlcat(cur_mod->data, line, sizeof(cur_mod->data));
                }
            }
        }
    }

    fclose(file);
    return 0;
}

static module_info *get_module_by_name(char *module_name) {
    module_info *mod = NULL;

    mod = g_modules_map[(string) module_name];
    if(mod == NULL)
        mod = g_controller_map[(string) module_name];

    return mod;
}

/**set the module fd*/
static void module_set_fd(char *module, int fd) {
    module_info *mi = NULL;

    if(module == NULL || (mi = get_module_by_name(module)) == NULL) {
        ALOGE("module:%s not in config file\n", module);
        return;
    }

    string lib_name = mi->config_list[KEY_LIB_NAME];

    if(!lib_name.empty()) {
        /* set all module in the same lib_name to the same FD */
        hash_map < string, module_info * >::iterator p;
        for(p = g_modules_map.begin(); p != g_modules_map.end(); p++) {
            module_info *mod = (module_info *) (p->second);

            if(mod != NULL && !mod->config_list[KEY_LIB_NAME].empty()
               && !mod->config_list[KEY_LIB_NAME].compare(lib_name)) {
                ALOGI("==== Set module: %s fd=%d\n", mod->module, fd);
                mod->socket_fd = fd;
                g_sock_client_map[lib_name] = fd;
            }
        }
    } else {
        /* if no lib_name just set the FD */
        ALOGI("==== Set module: %s fd=%d\n", mi->module, fd);
        mi->socket_fd = fd;
        g_sock_client_map[mi->module] = fd;
    }

    g_clients.push_back(mi);
    if(fd > g_max_fd)
        g_max_fd = fd;

    ALOGI("==== Get Hello from %s fd=%d\n", module, fd);
}

static void *server_accepting_thread(void *) {
    signal(SIGUSR1, signal_handler);

    int client_fd;
    struct sockaddr_un addr;
    socklen_t addr_size = sizeof(addr);

    int sockfd = create_socket(MMI_SOCKET);

    if(sockfd < 0)
        return NULL;

    listen(sockfd, 8);
    sem_post(&g_sem_accept_ready);

    msg_t msg;

    while(1) {
        client_fd = -1;
        memset(&msg, 0, sizeof(msg));
        if((client_fd = accept(sockfd, (struct sockaddr *) &addr, &addr_size)) < 0) {
            continue;
        }

        ALOGI("client_fd=%d connected\n", client_fd);
        TEMP_FAILURE_RETRY(recv(client_fd, &msg, sizeof(msg), MSG_WAITALL));

        /*Set to module */
        module_set_fd(msg.module, client_fd);

    }
    return NULL;
}

static int msg_waiting() {

    fd_set fds;
    int retval;


    list < module_info * >fd_lists;
    list < module_info * >::iterator iter;
    msg_t *msg = NULL;

    struct timeval tv;

    /* Wait up to 3 seconds. */
    tv.tv_sec = 3;
    tv.tv_usec = 0;

    FD_ZERO(&fds);
    fd_lists.clear();
    if(!g_clients.empty()) {
        list < module_info * >::iterator iter;
        for(iter = g_clients.begin(); iter != g_clients.end(); iter++) {
            module_info *mod = (module_info *) (*iter);

            if(mod != NULL && mod->socket_fd > 0) {
                FD_SET(mod->socket_fd, &fds);
                fd_lists.push_back(mod);
            }
        }
    }

    if(fd_lists.empty()) {
        ALOGI("fd_lists empty,waiting..\n");
        usleep(1000 * 1000);
        return -1;
    }
    retval = select(g_max_fd + 1, &fds, NULL, NULL, &tv);
    switch (retval) {
    case -1:
        ALOGE("select failed error=%d\n", strerror(errno));
        break;
    case 0:
        ALOGE("select timeout");
        break;
    default:
        int i = 0;

        for(iter = fd_lists.begin(); iter != fd_lists.end(); iter++) {
            module_info *mod = (module_info *) (*iter);

            if(mod == NULL)
                continue;

            int fd = mod->socket_fd;

            if(FD_ISSET(fd, &fds)) {
                msg = (msg_t *) zmalloc(sizeof(msg_t));
                if(msg == NULL) {
                    ALOGE("%s:out of memory, abort the current request:(%s)\n", __FUNCTION__, strerror(errno));
                    break;
                }

                int ret = TEMP_FAILURE_RETRY(recv(fd, msg, sizeof(msg_t), MSG_WAITALL));

                i++;
                ALOGD("module=%s num=%d size=%d\n", msg->module, i, ret);
                if(ret <= 0) {
                    close(mod->socket_fd);
                    mod->socket_fd = -1;
                } else {
                    enqueue_msg(&g_msg_queue, msg);
                    sem_post(&g_msg_sem);
                }
            }
        }
        break;
    }
    return 0;
}

static void *msg_waiting_thread(void *) {
    signal(SIGUSR1, signal_handler);

    while(1) {
        msg_waiting();
    }
    return NULL;
}

static void *msg_handle_thread(void *) {
    signal(SIGUSR1, signal_handler);

    msg_t *msg;
    module_info *mod;
    layout *curlay;

    while(1) {

        sem_wait(&g_msg_sem);
        dequeue_msg(&g_msg_queue, &msg);
        if(msg != NULL) {
            mod = get_module_by_name(msg->module);
            if(mod == NULL)
                continue;

            switch (msg->cmd) {
            case CMD_CTRL:
                handle_ctr_msg(msg, mod);
                break;
            case CMD_PRINT:
                handle_print(msg, mod);
                break;
            case CMD_QUERY:
                handle_query(msg, mod);
                break;
            case CMD_RUN:
                handle_run(msg, mod);
                break;
            default:
                break;
            }
        }
    }
    return NULL;
}

static void exit_handler(int num) {
    static int flag = 0;
    int status;

    if(flag == 0)
        usleep(10 * 1000);

    flag = 1;
    int pid = waitpid(-1, &status, WNOHANG);

    if(WIFEXITED(status)) {
        ALOGI("The child %d exit with code %d\n", pid, WEXITSTATUS(status));
     /**Reset pid */
        hash_map < string, module_info * >::iterator p;
        for(p = g_nodup_modules_map.begin(); p != g_nodup_modules_map.end(); ++p) {
            module_info *mod = (module_info *) (p->second);

            if(mod->pid == pid) {
                mod->pid = -1;
                break;
            }
        }
    }
    flag = 0;
}

static void init() {
    write_file(WAKE_LOCK, "mmi");
    sem_init(&g_sem_exit, 0, 0);
    sem_init(&g_msg_sem, 0, 0);
    sem_init(&g_sem_accept_ready, 0, 0);
    sem_init(&g_data_print_sem, 0, 0);
    sem_init(&g_result_sem, 0, 0);
    pthread_mutex_init(&g_cur_layout_mutex, NULL);
    init_draw();
    create_func_map();
    ALOGI("mmi gr_init complete.");
}

static void build_main_ui() {

    module_info *mod = g_modules_map[MAIN_MODULE];

    if(mod == NULL)
        return;
    /**Initial main module*/
    set_main_module(mod);

    layout *lay = g_layout_map[(string) mod->config_list[KEY_LAYOUT]];

    if(lay == NULL)
        return;

    if(lay->m_listview != NULL && g_ordered_modules.size() > 0) {
        lay->m_listview->set_items(&g_ordered_modules);
    }

    switch_cur_layout_locked(lay, mod);

    update_main_status();
    invalidate();
}

static void init_nodup_map(list < module_info * >*list_modules) {

    if(!list_modules->empty()) {
        list < module_info * >::reverse_iterator iter;

        ALOGD("total modules=%d\n", list_modules->size());
        for(iter = list_modules->rbegin(); iter != list_modules->rend(); iter++) {
            module_info *mod = (module_info *) (*iter);

            if(mod->config_list[KEY_LIB_NAME] != "")
                g_nodup_modules_map[mod->config_list[KEY_LIB_NAME]] = mod;
        }
    }
}


/**
 *   Start diag daemon for handling diag command from PC tool
 *
 */
static void launch_controller() {

    module_info *mod = g_controller_map[CLIENT_DIAG_NAME];

    if(mod == NULL)
        return;

    int pid = fork();

    if(pid == 0) {
        char *args[2] = { (char *) MMI_DIAG, NULL };

        int res = execv(MMI_DIAG, args);

        if(res < 0) {
            ALOGD("%s exec failed\n", MMI_DIAG);
            exit(1);
        }
    } else if(pid < 0) {
        ALOGD("fork failed\n");
    } else if(pid > 0) {
        mod->pid = pid;
        ALOGD("child_pid=%d\n", pid);
    }
}

static void launch_clients() {

    /* Launch clients */
    hash_map < string, module_info * >::iterator p;
    for(p = g_nodup_modules_map.begin(); p != g_nodup_modules_map.end(); ++p) {
        module_info *mod = (module_info *) (p->second);

        if(mod != NULL) {
            fork_launch_module(mod);
        }
    }
}

static int start_threads() {
    int retval = -1;

    retval = pthread_create(&g_input_waiting_tid, NULL, input_waiting_thread, NULL);
    if(retval < 0)
        return -1;

    retval = pthread_create(&g_input_handle_tid, NULL, input_handle_thread, NULL);
    if(retval < 0)
        return -1;

    retval = pthread_create(&g_draw_tid, NULL, draw_thread, NULL);
    if(retval < 0)
        return -1;

    retval = pthread_create(&g_accept_tid, NULL, server_accepting_thread, NULL);
    if(retval < 0)
        return -1;

    retval = pthread_create(&g_waiting_event_tid, NULL, msg_waiting_thread, NULL);
    if(retval < 0)
        return -1;

    retval = pthread_create(&g_msg_handle_tid, NULL, msg_handle_thread, NULL);
    if(retval < 0)
        return -1;

    sem_wait(&g_sem_accept_ready);
    return 0;
}

static void init_layout(hash_map < string, layout * >&layout_map) {

    struct dirent *de;
    DIR *dir;
    char layout_path[PATH_MAX] = { 0 };

    layout_map.clear();
    /*Initial layout */
    dir = opendir(MMI_LAYOUT_BASE_DIR);
    if(dir != 0) {
        while((de = readdir(dir))) {
            if(!strcmp(de->d_name, ".") || !strcmp(de->d_name, ".."))
                continue;

            snprintf(layout_path, sizeof(layout_path), "%s%s", MMI_LAYOUT_BASE_DIR, de->d_name);
            layout *lay = new layout(layout_path);

            if(load_layout(layout_path, lay) == 0) {
                layout_map[de->d_name] = lay;
                ALOGI("%s:Load layout(%s) success", __FUNCTION__, de->d_name);
            } else {
                if(lay != NULL)
                    delete lay;
            }
        }
        closedir(dir);
    }
}

static void init_controller() {

    g_controller_map.clear();

     /**Init Diag module*/
    module_info *mod = new module_info((char *) CLIENT_DIAG_NAME);

    g_controller_map[CLIENT_DIAG_NAME] = mod;

     /**Init debug module*/
    mod = new module_info((char *) CLIENT_DEBUG_NAME);
    g_controller_map[CLIENT_DEBUG_NAME] = mod;
}

static void init_module_mode(list < module_info * >*list_modules) {

    uint32_t i = -1;
    char boot_mode[PROPERTY_VALUE_MAX] = { 0 };

    /**Get mode from config file*/
    module_info *mod = g_modules_map[MAIN_MODULE];

    if(mod == NULL || mod->config_list[KEY_TESTMODE] == "") {
        ALOGE("Not mode configed");
        return;
    }
    const char *testmode = mod->config_list[KEY_TESTMODE].c_str();

    for(i = 0; i < sizeof(test_mode_string) / sizeof(char *); i++) {
        if(!strcmp(test_mode_string[i], testmode)) {
            g_test_mode = (module_mode_t) i;
            break;
        }
    }

   /**Get mode from misc boot mode,overwrite the config file mode*/
    property_get("ro.bootmode", boot_mode, "normal");
    for(i = 0; i < sizeof(boot_mode_string) / sizeof(char *); i++) {
        if(!strcmp(boot_mode_string[i], boot_mode)) {
            g_test_mode = (module_mode_t) i;
            break;
        }
    }
    ALOGI("fastmmi testmode :%d", g_test_mode);

    if(!list_modules->empty()) {
        list < module_info * >::iterator iter;
        for(iter = list_modules->begin(); iter != list_modules->end(); iter++) {
            module_info *mod = (module_info *) (*iter);

            if(mod != NULL)
                mod->mode = g_test_mode;
        }
    }
}

static int init_config(const char *cfg) {

    int ret = -1;

    /*Initialize configuration */
    ret = load_config(cfg, &g_modules_map, &g_ordered_modules);
    if(ret < 0)
        return -1;

    /*Load more layout */
    init_layout(g_layout_map);

    /*Initial all module running mode */
    init_module_mode(&g_ordered_modules);

    /*Initial the duplicate module map */
    init_nodup_map(&g_ordered_modules);

    /*init the result file name */
    // Res filename should not contain the extension of the config file...
    char *p = strrchr(cfg, '/');

    if(p != NULL) {
        string res_file_name = string(p);
        size_t last_index = res_file_name.find_last_of(".");

        if(last_index != string::npos) {
            res_file_name = res_file_name.substr(1, last_index - 1);
        }
        res_file_name = MMI_BASE_DIR + res_file_name;
        res_file_name += ".res";

        strlcpy(g_res_file, res_file_name.c_str(), sizeof(g_res_file));
    }
    ALOGI("test result file:%s \n", g_res_file);
    return 0;
}

static void clean_resource() {

    /**Stop threads before clean module info*/
    pthread_kill(g_waiting_event_tid, SIGUSR1);
    pthread_join(g_waiting_event_tid, NULL);

    /**Clean Layout*/
    hash_map < string, layout * >::iterator p2;
    for(p2 = g_layout_map.begin(); p2 != g_layout_map.end(); ++p2) {
        layout *lay = (layout *) (p2->second);

        ALOGI("Clean layout (%s).\n", ((string) p2->first).c_str());
        if(lay != NULL) {
            lay->clear_locked();
            delete lay;

            lay = NULL;
        }
    }

     /**Clean Module info*/
    while(g_ordered_modules.begin() != g_ordered_modules.end()) {
        module_info *tmp = *g_ordered_modules.begin();

        if(tmp != NULL) {
            ALOGI("Clean module (%s).\n", tmp->module);
            delete tmp;

            g_ordered_modules.erase(g_ordered_modules.begin());
        }
    }

   /**Clean static */
    g_nodup_modules_map.clear();
    g_ordered_modules.clear();
    g_modules_map.clear();
    g_layout_map.clear();
    g_clients.clear();
}

/**autostart config **/
static bool is_autostart() {
    bool ret = false;
    char buf[64] = { 0 };
    if(!read_file(AUTOSTART_CONFIG, buf, sizeof(buf)) && !strcmp(buf, KEY_ASCII_TRUE)) {
        ret = true;
    }
    ALOGI("autoconfig:%s, autostart = %s", buf, ret == true ? "enabled" : "disabled");
    return ret;
}

/*Start all test
**if automation ==1, then only start support automation test cases
**if automation == 0, start all test cases.
**/
void start_all(bool automation) {
    list < module_info * >::iterator iter;
    for(iter = g_ordered_modules.begin(); iter != g_ordered_modules.end(); iter++) {
        module_info *mod = (module_info *) (*iter);

        if(mod != NULL && (!automation || (automation && !mod->config_list[KEY_AUTOMATION].compare("1")))) {
            module_exec_pcba(mod);
        }
    }
}

static bool check_fd_exist(int fd) {
    list < module_info * >::iterator iter;
    for(iter = g_clients.begin(); iter != g_clients.end(); iter++) {
        module_info *tmod = (module_info *) (*iter);

        if(tmod->socket_fd == fd)
            return true;
    }

    return false;
}

static void reconfig_clients() {
    ALOGI("=============Client list===============");
    list < module_info * >::iterator iter;
    for(iter = g_ordered_modules.begin(); iter != g_ordered_modules.end(); iter++) {
        module_info *mod = (module_info *) (*iter);

        if(mod != NULL && !mod->config_list[KEY_LIB_NAME].empty()
           && g_sock_client_map[mod->config_list[KEY_LIB_NAME]] > 0) {
            mod->socket_fd = g_sock_client_map[mod->config_list[KEY_LIB_NAME]];
            if(!check_fd_exist(mod->socket_fd))
                g_clients.push_back(mod);
            ALOGI("%s[%d]", mod->module, mod->socket_fd);
        }
    }

    g_clients.push_back(g_controller_map[CLIENT_DIAG_NAME]);
    ALOGI("%s[%d]", g_controller_map[CLIENT_DIAG_NAME]->module, g_controller_map[CLIENT_DIAG_NAME]->socket_fd);
}

int reconfig(const char *cfg) {
    int ret = -1;

    if(cfg == NULL)
        return -1;

    clean_resource();

    /*Initialize configuration */
    ret = init_config(cfg);
    if(ret < 0)
        return -1;

    /*Initial the MMI screen */
    build_main_ui();

    /*Reconfig client */
    reconfig_clients();

    layout *lay = g_layout_map[get_main_module()->config_list[KEY_LAYOUT]];

    if(lay == NULL || lay->m_listview == NULL)
        return -1;

    ret = pthread_create(&g_waiting_event_tid, NULL, msg_waiting_thread, NULL);
    if(ret < 0)
        return -1;

    return lay->m_listview->get_items()->size();
}

int main(int argc, char **argv) {
    signal(SIGCHLD, exit_handler);
    int ret = -1;

    ALOGI("start mmi now,enjoy it!");
    init();

    /*Initialize configuration */
    ret = init_config(MMI_CONFIG);
    if(ret < 0)
        return -1;

    /*Load controller */
    init_controller();

    /**Restore the latest result*/
    restore_result(g_res_file);

    /*Initial the MMI screen */
    build_main_ui();

    /*Start threads */
    start_threads();

    /*Launch threads */
    launch_controller();
    launch_clients();

    /*Start Background Test */
    if(is_autostart())
        start_all(true);

    sem_wait(&g_sem_exit);
    write_file(WAKE_UNLOCK, "mmi");
  out:sem_close(&g_sem_exit);
    return 0;
}
