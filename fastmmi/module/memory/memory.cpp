/*
 * Copyright (c) 2014, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#include "mmi_module.h"

static int32_t module_init(const mmi_module_t * module, hash_map < string, string > &params) {
    ALOGI("%s start", __FUNCTION__);
    if(module == NULL) {
        ALOGE("%s NULL point  received ", __FUNCTION__);
        return FAILED;
    }
    return SUCCESS;
}

static int32_t module_deinit(const mmi_module_t * module) {
    ALOGI("%s start.", __FUNCTION__);
    return SUCCESS;
}

static int32_t module_stop(const mmi_module_t * module) {
    ALOGI("%s start.", __FUNCTION__);
    if(module == NULL) {
        ALOGE("%s NULL point  received ", __FUNCTION__);
        return FAILED;
    }

    kill_thread(module->run_pid);
    return SUCCESS;
}

/**
* Before call Run function, caller should call module_init first to initialize the module.
* the "cmd" passd in MUST be defined in cmd_list ,mmi_agent will validate the cmd before run.
*
*/
static int32_t module_run(const mmi_module_t * module, const char *cmd, hash_map < string, string > &params) {

    int ret = FAILED, n = 0;
    char buf[SIZE_1K] = { 0 };
    char mem[256] = { 0 };
    char *memtotal, *x;
    uint64_t total = 0, min_limit = 0, max_limit = 0;

    if(!module || !cmd) {
        ALOGE("%s NULL point  received ", __FUNCTION__);
        return FAILED;
    }
    ALOGI("%s start.command : %s", __FUNCTION__, cmd);

    if(!read_file(PROC_MEMINFO, buf, sizeof(buf))) {
        memtotal = strstr(buf, "MemTotal");
        if(memtotal) {
            x = strstr(memtotal, ": ");
            if(x) {
                x += 2;
                n = 0;
                while(*x) {
                    if(*x == '\n')
                        break;

                    if(!isspace(*x) && isdigit(*x))
                        mem[n++] = *x;
                    x++;
                }
                mem[n] = '\0';
            }
        }

        total = string_to_long(mem) / SIZE_1K;
    }


    if(!strcmp(cmd, SUBCMD_MMI)) {
        snprintf(buf, sizeof(buf), "MemTotal = %" PRIu64 " MB \n", total);
        module->cb_print(params[KEY_MODULE_NAME].c_str(), SUBCMD_MMI, buf, strlen(buf), PRINT_DATA);
    } else if(!strcmp(cmd, SUBCMD_PCBA)) {
        min_limit = string_to_long(params[KEY_MIN_LIMINT]);
        max_limit = string_to_long(params[KEY_MAX_LIMINT]);
        if(max_limit != 0) {
            if(total > min_limit && total < max_limit)
                ret = SUCCESS;
        }

        ALOGI("%s memTotal : %llu limit[%llu,%llu] kB", __FUNCTION__, total, min_limit, max_limit);
        snprintf(buf, sizeof(buf), "MemTotal = %" PRIu64 " MB \n", total);
        module->cb_print(params[KEY_MODULE_NAME].c_str(), SUBCMD_PCBA, buf, strlen(buf), PRINT_DATA);
    }

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
    .name = "Memory",
    .author = "Qualcomm Technologies, Inc.",
    .methods = &module_methods,
    .module_handle = NULL,
    .supported_cmd_list = NULL,
    .supported_cmd_list_size = 0,
    .cb_print = NULL, /**it is initialized by mmi agent*/
    .run_pid = -1,
};
