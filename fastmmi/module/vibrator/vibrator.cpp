/*
 * Copyright (c) 2014, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#include <hardware/vibrator.h>
#include <hardware/hardware.h>
#include "mmi_module.h"

static vibrator_device_t *dev = NULL;
static const char str_vibrator_on[] = "Vibrator on\n";
static const char str_vibrator_off[] = "Vibrator off\n";

static int32_t module_stop(const mmi_module_t * module);

/**
* Defined case run in mmi mode,this mode support UI.
*
*/
static int hal_init(hash_map < string, string > &params) {
    int ret = FAILED;
    hw_module_t *hw_module = NULL;

    ret = hw_get_module(VIBRATOR_HARDWARE_MODULE_ID, (hw_module_t const **) &hw_module);
    if(ret == SUCCESS) {
        ret = vibrator_open(hw_module, &dev);
    } else {
        ALOGE("Vibrator FFBM : hw_get_module() call failed \n");
    }

    return ret;
}

static void hal_deinit(void) {
    if(dev != NULL) {
        dev->common.close((hw_device_t *) dev);
        dev = NULL;
    }
}

static void *run_test(void *mod) {
    signal(SIGUSR1, signal_handler);
    if(dev == NULL)
        return NULL;

    while(1) {
        dev->vibrator_on(dev, 200);
        usleep(800 * 1000);
    }
    return NULL;
}

static int32_t module_run_mmi(const mmi_module_t * module, hash_map < string, string > &params) {
    ALOGI("%s start ", __FUNCTION__);

    int ret = pthread_create((pthread_t *) & module->run_pid, NULL, run_test, (void *) module);

    if(ret < 0) {
        ALOGE("%s:Can't create pthread: %s\n", __FUNCTION__, strerror(errno));
        return FAILED;
    } else {
        pthread_join(module->run_pid, NULL);
    }

    return ret;
}

/**
* Defined case run in PCBA mode, fully automatically.
*
*/
static int32_t module_run_pcba(const mmi_module_t * module, hash_map < string, string > &params) {
    ALOGI("%s start", __FUNCTION__);
    if(!strncmp(params["switch"].c_str(), "on", 2)) {
        dev->vibrator_on(dev, (30 * 1000));
        module->cb_print(NULL, SUBCMD_PCBA, str_vibrator_on, strlen(str_vibrator_on), PRINT);
    } else if(!strncmp(params["switch"].c_str(), "off", 3)) {
        module_stop(module);
        module->cb_print(NULL, SUBCMD_PCBA, str_vibrator_off, strlen(str_vibrator_off), PRINT);
    }

    return ERR_UNKNOW;
}

static int32_t module_init(const mmi_module_t * module, hash_map < string, string > &params) {
    ALOGI("%s start ", __FUNCTION__);
    int ret = FAILED;

    if(module == NULL) {
        ALOGE("%s NULL point  received ", __FUNCTION__);
        return FAILED;
    }
    ret = hal_init(params);
    return ret;
}

static int32_t module_deinit(const mmi_module_t * module) {
    ALOGI("%s start.", __FUNCTION__);
    if(module == NULL) {
        ALOGE("%s NULL point  received ", __FUNCTION__);
        return FAILED;
    }

    hal_deinit();
    return SUCCESS;
}

static int32_t module_stop(const mmi_module_t * module) {
    ALOGI("%s start.", __FUNCTION__);
    if(module == NULL) {
        ALOGE("%s NULL point  received ", __FUNCTION__);
        return FAILED;
    }

    if(dev != NULL) {
        dev->vibrator_off(dev);
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

    if(!module || !cmd) {
        ALOGE("%s NULL point  received ", __FUNCTION__);
        return FAILED;
    }
    ALOGI("%s start.command : %s", __FUNCTION__, cmd);

    if(!strcmp(cmd, SUBCMD_MMI))
        ret = module_run_mmi(module, params);
    else if(!strcmp(cmd, SUBCMD_PCBA))
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
    .name = "Vibrator",
    .author = "Qualcomm Technologies, Inc.",
    .methods = &module_methods,
    .module_handle = NULL,
    .supported_cmd_list = NULL,
    .supported_cmd_list_size = 0,
    .cb_print = NULL, /**it is initialized by mmi agent*/
    .run_pid = -1,
};
