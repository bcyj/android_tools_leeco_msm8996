/*
 * Copyright (c) 2014, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#include "mmi_module.h"

#define MAX_SIM_NUM 3
#define JUSTIFY_KEY "card state Present"

static exec_cmd_t execmd;
static pthread_mutex_t g_mutex;
static int sim_state[MAX_SIM_NUM];

static int prepare_params() {
    FILE *fp = NULL;
    char str[256] = "card status";

    fp = fopen(SIM_INPUT, "w");
    if(fp == NULL)
        return -1;

    fwrite(str, 1, strlen(str), fp);
    fclose(fp);
    /*output file empty */
    fp = fopen(SIM_OUTPUT, "w");
    if(fp == NULL)
        return -1;
    fclose(fp);
    return 0;
}

static void parse_sim_state() {
    char line[1024] = { 0, };
    bool found = false;
    int index = 0;

    FILE *file = fopen(SIM_OUTPUT, "r");

    if(file == NULL) {
        ALOGE("%s,open fail", SIM_OUTPUT);
        return;
    }

    while(fgets(line, sizeof(line), file) != NULL) {

        if(line[0] == '#') {
            continue;
        }

        if(found) {
            found = false;
            ALOGE("found one card : %s\n", line);
            if(strstr(line, JUSTIFY_KEY) != NULL) {
                sim_state[index] = 1;
            }
        }

        if(!strncmp(line, "CARD", 4)) {
            ALOGI("FFBM SIM: card found : %s\n", line);
            get_device_index(line, "CARD", &index);
            found = true;
            continue;
        }
    }

    fclose(file);
}

static void *thread_test(void *) {
    signal(SIGUSR1, signal_handler);

    int ret = FAILED;
    char buf[SIZE_1K] = { 0 };

    if(!check_file_exist(BIN_RIL_TEST)) {
        return NULL;
    }

    char *args[4] = { (char *) BIN_RIL_TEST, (char *) SIM_INPUT_PARAM, (char *) SIM_OUTPUT_PARAM, NULL };
    execmd.cmd = BIN_RIL_TEST;
    execmd.params = args;
    execmd.pid = -1;
    execmd.result = buf;
    execmd.size = sizeof(buf);
    exe_cmd(NULL, &execmd);
    return NULL;
}

static void start_test(const mmi_module_t * module) {
    ALOGI("%s,start\n", __FUNCTION__);
    int ret = -1;

    prepare_params();
    ret = pthread_create((pthread_t *) & module->run_pid, NULL, thread_test, NULL);
    if(ret < 0)
        return;

    /**waiting sometime and then kill the process*/
    usleep(1000 * 3000);

    if(execmd.pid > 0)
        kill_proc(execmd.pid);

    kill_thread(module->run_pid);
    /**parse output file*/
    parse_sim_state();
    ALOGI("%s,finished\n", __FUNCTION__);
}

static int32_t module_init(const mmi_module_t * module, hash_map < string, string > &params) {
    ALOGI("%s start ", __FUNCTION__);
    if(module == NULL) {
        ALOGE("%s NULL point  received ", __FUNCTION__);
        return FAILED;
    }
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

    pthread_kill(module->run_pid, SIGUSR1);
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
    ALOGI("%s start.command : %s", __FUNCTION__, cmd);

    pthread_mutex_lock(&g_mutex);
    start_test(module);
    pthread_mutex_unlock(&g_mutex);

    for(int i = 0; i < MAX_SIM_NUM; i++)
        ALOGI("%s card[%d] status: [%s]", __FUNCTION__, i, sim_state[i] ? "present" : "absent");

    if(!strcmp(params["sub"].c_str(), "0")) {
        if(sim_state[0])
            ret = SUCCESS;
        snprintf(buf, sizeof(buf), "status = %s \n", sim_state[0] ? "detected" : "not detected");
    } else if(!strcmp(params["sub"].c_str(), "1")) {
        if(sim_state[1])
            ret = SUCCESS;
        snprintf(buf, sizeof(buf), "status = %s  \n", sim_state[1] ? "detected" : "not detected");
    }

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
    .name = "Simcard",
    .author = "Qualcomm Technologies, Inc.",
    .methods = &module_methods,
    .module_handle = NULL,
    .supported_cmd_list = NULL,
    .supported_cmd_list_size = 0,
    .cb_print = NULL, /**it is initialized by mmi agent*/
    .run_pid = -1,
};
