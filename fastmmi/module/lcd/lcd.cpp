/*
 * Copyright (c) 2014, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#include "mmi_module.h"

#define SYS_LCD "/sys/class/graphics/fb0/msm_fb_panel_info"
#define JUSTIFY_KEY "alive"

static const char str_pcba_pass[] = "Test pass: LCD panel status is alive";

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
    char lcd_info[256] = { 0 };

    if(module == NULL) {
        ALOGE("%s NULL point  received ", __FUNCTION__);
        return FAILED;
    }

    if((read_file(SYS_LCD, lcd_info, sizeof(lcd_info)) == SUCCESS)
       && (strstr(lcd_info, JUSTIFY_KEY) != NULL)) {
        module->cb_print(NULL, SUBCMD_PCBA, lcd_info, strlen(lcd_info), PRINT);
        return SUCCESS;
    }

    module->cb_print(NULL, SUBCMD_PCBA, "Test fail\n", 10, PRINT);
    return FAILED;
}

static int32_t module_init(const mmi_module_t * module, hash_map < string, string > &params) {
    ALOGI("[%s]start", __FUNCTION__);
    if(module == NULL) {
        ALOGE("%s NULL point  received ", __FUNCTION__);
        return FAILED;
    }
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
    .name = "Key",
    .author = "Qualcomm Technologies, Inc.",
    .methods = &module_methods,
    .module_handle = NULL,
    .supported_cmd_list = NULL,
    .supported_cmd_list_size = 0,
    .cb_print = NULL, /**it is initialized by mmi agent*/
    .run_pid = -1,
};
