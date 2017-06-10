/*
 * Copyright (c) 2015, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#include <linux/input.h>
#include "mmi_module.h"
#include "minui.h"

static key_map_t key_map[] = {
    {KEY_STR_HEADPHONE_INSERT, NULL, SW_HEADPHONE_INSERT, false, false},
    {KEY_STR_MICROPHONE_INSERT, NULL, SW_MICROPHONE_INSERT, false, false},
    {KEY_STR_HANGUP, NULL, KEY_MEDIA, false, false},
};

static mmi_module_t *g_module = NULL;
static char module_name[32] = { 0 };

static bool pcba_success = false;
static sem_t semaphore_headset_success;

static int32_t module_stop(const mmi_module_t * module);

/**
* Defined case run in mmi mode,this mode support UI.
* @return, 0 -success; -1
*/
static int key_input_callback(int fd, uint32_t revents, void *data) {
    struct input_event ev;
    char tmp[SIZE_512] = { 0 };
    char buf[SIZE_512] = { 0 };
    int retval = 0;
    unsigned int i = 0;

    retval = ev_get_input(fd, revents, &ev);
    if(retval < 0)
        return -1;

    if(ev.type == EV_KEY) {
        for(i = 0; i < (sizeof(key_map) / sizeof(key_map_t)); i++) {
            if((ev.code == key_map[i].key_code) && (ev.value == 0))
                key_map[i].tested = true;
        }
    } else if(ev.type == EV_SW) {
        for(i = 0; i < (sizeof(key_map) / sizeof(key_map_t)); i++) {
            if((ev.code == key_map[i].key_code) && (ev.value == 0))
                key_map[i].tested = true;
        }
    }
    for(i = 0; i < (sizeof(key_map) / sizeof(key_map_t)); i++) {
        if((key_map[i].exist) && (key_map[i].tested))
            pcba_success = true;
        else if((key_map[i].exist) && !(key_map[i].tested)) {
            pcba_success = false;
            break;
        }
    }

    if(pcba_success) {
        for(i = 0; i < (sizeof(key_map) / sizeof(key_map_t)); i++) {
            if(key_map[i].exist && key_map[i].tested) {
                snprintf(buf, sizeof(buf), "%s = detected\n", key_map[i].key_name);
                strncat(tmp, buf, sizeof(buf));
            }
        }
        snprintf(buf, sizeof(buf), "Key PCBA test Pass\n%s", tmp);
        g_module->cb_print(module_name, SUBCMD_PCBA, buf, strlen(buf), PRINT_DATA);
        sem_post(&semaphore_headset_success);
    } else if(((ev.type == EV_KEY) || (ev.type == EV_SW)) && (ev.value == 0)) {
        for(i = 0; i < (sizeof(key_map) / sizeof(key_map_t)); i++) {
            if(key_map[i].exist && key_map[i].tested) {
                snprintf(buf, sizeof(buf), "%s = detected\n", key_map[i].key_name);
                strncat(tmp, buf, sizeof(buf));
            } else if(key_map[i].exist && !key_map[i].tested) {
                snprintf(buf, sizeof(buf), "%s = not detected\n", key_map[i].key_name);
                strncat(tmp, buf, sizeof(buf));
            }
        }
        snprintf(buf, sizeof(buf), "Key PCBA test Fail\n%s", tmp);
        g_module->cb_print(module_name, SUBCMD_PCBA, buf, strlen(buf), PRINT_DATA);
    }

    return 0;
}

static void *run_test(void *mod) {
    signal(SIGUSR1, signal_handler);
    if(mod == NULL)
        return NULL;

    while(!pcba_success) {
        if(!ev_wait(-1))
            ev_dispatch();
    }

    return NULL;
}

static int init(hash_map < string, string > &params) {
    unsigned int i = 0;
    char tmp[SIZE_512] = { 0 };
    char buf[SIZE_512] = { 0 };

    pcba_success = false;

    for(i = 0; i < (sizeof(key_map) / sizeof(key_map_t)); i++) {
        key_map[i].tested = false;
        if(strstr(params["keys"].c_str(), key_map[i].key_name) != NULL) {
            key_map[i].exist = true;
        }
    }

    if(sem_init(&semaphore_headset_success, 0, 0) != 0) {
        ALOGE("HEADSET FFBM :semaphore_headset_success creation failed \n");
        return FAILED;
    }

    for(i = 0; i < (sizeof(key_map) / sizeof(key_map_t)); i++) {
        if(key_map[i].exist) {
            snprintf(buf, sizeof(buf), "%s = not detected\n", key_map[i].key_name);
            strncat(tmp, buf, sizeof(buf));
        }
    }
    snprintf(buf, sizeof(buf), "Key PCBA test Fail\n%s", tmp);
    g_module->cb_print(module_name, SUBCMD_PCBA, buf, strlen(buf), DATA);

    return SUCCESS;
}

/**
* Defined case run in mmi mode,this mode support UI.
* @return, 0 -success; -1
*/
static int32_t module_run_mmi(const mmi_module_t * module, hash_map < string, string > &params) {
    ALOGI("[%s]start", __FUNCTION__);
    if(module == NULL) {
        ALOGE("%s NULL point  received ", __FUNCTION__);
        return FAILED;
    }
    return ERR_UNKNOW;
}

/**
* Defined case run in PCBA mode, fully automatically.
*
*/
static int32_t module_run_pcba(const mmi_module_t * module, hash_map < string, string > &params) {
    ALOGI("[%s] start", __FUNCTION__);
    int ret = FAILED;
    char tmp[SIZE_512] = { 0 };
    char buf[SIZE_512] = { 0 };
    unsigned int i = 0;
    struct timespec time_sec;
    int sem_status;

    if(module == NULL) {
        ALOGE("%s NULL point  received ", __FUNCTION__);
        return FAILED;
    }

    ret = init(params);
    if(ret != SUCCESS) {
        ALOGE("%s key pcba test initialization failed", __FUNCTION__);
        return FAILED;
    }

    ret = pthread_create((pthread_t *) & module->run_pid, NULL, run_test, (void *) module);
    if(ret < 0) {
        ALOGE("%s:Can't create pthread: %s\n", __FUNCTION__, strerror(errno));
        return FAILED;
    }

    if(clock_gettime(CLOCK_REALTIME, &time_sec) == -1)
        ALOGI("get clock_gettime error");
    time_sec.tv_sec += atoi(params["timeout"].c_str());

    sem_status = sem_timedwait(&semaphore_headset_success, &time_sec);
    pthread_kill(module->run_pid, SIGUSR1);
    if(sem_status == -1) {
        ALOGI("%s detect key timeout, fail to test key in pcba", __FUNCTION__);
        return FAILED;
    } else
        return SUCCESS;

    return ERR_UNKNOW;
}

static int32_t module_init(const mmi_module_t * module, hash_map < string, string > &params) {
    ALOGI("[%s]start", __FUNCTION__);
    if(module == NULL) {
        ALOGE("%s NULL point  received ", __FUNCTION__);
        return FAILED;
    }

    g_module = (mmi_module_t *) module;
    strncpy(module_name, params[KEY_MODULE_NAME].c_str(), sizeof(module_name));
    ev_init(key_input_callback, NULL);

    return SUCCESS;
}

static int32_t module_deinit(const mmi_module_t * module) {
    ALOGI("[%s] start.", __FUNCTION__);
    if(module == NULL) {
        ALOGE("%s NULL point  received ", __FUNCTION__);
        return FAILED;
    }
    return SUCCESS;
}

static int32_t module_stop(const mmi_module_t * module) {
    ALOGI("[%s] start.", __FUNCTION__);
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
* Attention: the UI mode running in MMI application, no need implementation in module.
*/
static int32_t module_run(const mmi_module_t * module, const char *cmd, hash_map < string, string > &params) {
    ALOGI("%s start.command : %s", __FUNCTION__, cmd);
    int ret = -1;

    if(!module || !cmd) {
        ALOGE("%s NULL point  received ", __FUNCTION__);
        return FAILED;
    }

    if(!strncmp(cmd, SUBCMD_PCBA, strlen(cmd)))
        ret = module_run_pcba(module, params);
    else {
        ALOGE("%s Invalid command: %s  received ", __FUNCTION__, cmd);
        ret = FAILED;
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
    .name = "Headset",
    .author = "Qualcomm Technologies, Inc.",
    .methods = &module_methods,
    .module_handle = NULL,
    .supported_cmd_list = NULL,
    .supported_cmd_list_size = 0,
    .cb_print = NULL, /**it is initialized by mmi agent*/
    .run_pid = -1,
};
