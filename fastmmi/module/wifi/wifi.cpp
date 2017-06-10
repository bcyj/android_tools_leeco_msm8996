/*
 * Copyright (c) 2014, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#include "mmi_module.h"

/**
* Defined case run in mmi mode,this mode support UI.
*
*/
static const mmi_module_t *g_module = NULL;
static exec_cmd_t execmd;
static pthread_mutex_t g_mutex;

#define JUSTIFY_KEY "ESSID:"

static void cb_function(char *str, int size) {
    if(g_module != NULL)
        g_module->cb_print(NULL, SUBCMD_MMI, str, size, PRINT);
}

static void load_driver() {
    char temp[256] = { 0 };
    snprintf(temp, sizeof(temp), "insmod %s", WIFI_DRIVER);
    system(temp);
}

static void unload_driver() {
    system("rmmod wlan");
}

static void config_wlan() {
    char temp[256] = { 0 };
    snprintf(temp, sizeof(temp), "%s wlan0 up", WIFI_IFCONFIG);
    system(temp);
}

static int start_test(char *buf, uint32_t size) {
    char result[SIZE_1K] = { 0 };
    char tmp[256] = { 0 };
    int ret = FAILED;
    bool found = false;

    if(!check_file_exist(WIFI_DRIVER) || !check_file_exist(WIFI_IFCONFIG)
       || !check_file_exist(WIFI_IWLIST))
        return FAILED;

    load_driver();
    config_wlan();

    char *args[4] = { (char *) WIFI_IWLIST, (char *) "wlan0", (char *) "scan", NULL };
    execmd.cmd = WIFI_IWLIST;
    execmd.params = args;
    execmd.pid = -1;
    execmd.result = result;
    execmd.size = sizeof(result);
    ret = exe_cmd(cb_function, &execmd);
    unload_driver();

    if(ret != SUCCESS)
        return FAILED;

    char *p = result;
    char *ptr;

    while(*p != '\0') {         /*print every line of scan result information */
        ptr = tmp;
        while(*p != '\n' && *p != '\0') {
            *ptr++ = *p++;
        }

        p++;
        *ptr = '\0';

        ptr = strstr(tmp, JUSTIFY_KEY);
        if(ptr != NULL) {
            ptr = strstr(tmp, ":");
            ptr++;
            snprintf(tmp, sizeof(tmp), "AP ESSID = %s \n", ptr);
            strlcat(buf, tmp, size);
            found = true;
        }
    }
    return found ? SUCCESS : FAILED;
}

static int32_t module_init(const mmi_module_t * module, hash_map < string, string > &params) {
    ALOGI("%s start ", __FUNCTION__);

    if(module == NULL) {
        ALOGE("%s NULL point  received ", __FUNCTION__);
        return FAILED;
    }
    g_module = module;
    pthread_mutex_init(&g_mutex, NULL);

    return SUCCESS;
}

static int32_t module_deinit(const mmi_module_t * module) {
    ALOGI("%s start.", __FUNCTION__);
    if(module == NULL) {
        ALOGE("%s NULL point  received ", __FUNCTION__);
        return FAILED;
    }
    return SUCCESS;
}

static int32_t module_stop(const mmi_module_t * module) {
    ALOGI("%s start.", __FUNCTION__);
    if(module == NULL) {
        ALOGE("%s NULL point  received ", __FUNCTION__);
        return FAILED;
    }

    kill_proc(execmd.pid);
    kill_thread(module->run_pid);
    return SUCCESS;
}

/**
* Before call Run function, caller should call module_init first to initialize the module.
* the "cmd" passd in MUST be defined in cmd_list ,mmi_agent will validate the cmd before run.
*
*/
static int32_t module_run(const mmi_module_t * module, const char *cmd, hash_map < string, string > &params) {
    int ret = FAILED;
    char buf[SIZE_1K] = { 0 };

    if(!module || !cmd) {
        ALOGE("%s NULL point  received ", __FUNCTION__);
        return FAILED;
    }
    ALOGI("%s start.command :waiting ... %s", __FUNCTION__, cmd);
    g_module = module;

    pthread_mutex_lock(&g_mutex);
    ret = start_test(buf, sizeof(buf));
    pthread_mutex_unlock(&g_mutex);

    if(!strcmp(cmd, SUBCMD_MMI)) {
        module->cb_print(params[KEY_MODULE_NAME].c_str(), SUBCMD_MMI, buf, strlen(buf), PRINT_DATA);
    } else if(!strcmp(cmd, SUBCMD_PCBA)) {
        module->cb_print(params[KEY_MODULE_NAME].c_str(), SUBCMD_PCBA, buf, strlen(buf), PRINT_DATA);
    }

   /** Default RUN mmi*/
    return ret;
}

/**
* Methods must be implemented by module.
*/
static struct mmi_module_methods_t module_methods = {
    .module_init = module_init,
    .module_deinit = module_deinit,
    .module_run = module_run,
    .module_stop = module_stop,
};

/**
* Every mmi module must have a data structure named MMI_MODULE_INFO_SYM
* and the fields of this data structure must be initialize in strictly sequence as definition,
* please don't change the sequence as g++ not supported in CPP file.
*/
mmi_module_t MMI_MODULE_INFO_SYM = {
    .version_major = 1,
    .version_minor = 0,
    .name = "Wifi",
    .author = "Qualcomm Technologies, Inc.",
    .methods = &module_methods,
    .module_handle = NULL,
    .supported_cmd_list = NULL,
    .supported_cmd_list_size = 0,
    .cb_print = NULL, /**it is initialized by mmi agent*/
    .run_pid = -1,
};
