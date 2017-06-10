/*
 * Copyright (c) 2014, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#include <sys/statfs.h>
#include "mmi_module.h"

#define SDCARD1_PATH "/storage/sdcard1"

/**
* Defined case run in mmi mode,this mode support UI.
*
*/
static int emmc_info(char *buf, int size, uint64_t * target_size) {
    int ret = FAILED;
    char tmp[256] = { 0 };
    double total_size = 0;

    if(buf == NULL || target_size == NULL)
        return ret;

    if(!read_file(SYS_MMCBLK0_SIZE, tmp, sizeof(tmp))) {
        total_size = atoi(tmp) * BLOCK_SIZE / SIZE_1G;
        ALOGI("emmc size:%4.3f G", total_size);
        snprintf(buf, size, "emmc capacity = %4.3f G \n", total_size);
        strlcat(buf, "emmc = deteced \n", size);
        *target_size = (int) total_size;

        ret = SUCCESS;
    } else {
        strlcpy(buf, "emmc = not deteced \n", size);
    }

    return ret;
}

static int sdcard_info(char *buf, int size, uint64_t * target_size) {
    struct statfs st;
    int ret = FAILED;
    char tmp[256] = { 0 };

    if(buf == NULL || target_size == NULL)
        return ret;

    if(check_file_exist(DEV_MMCBLK1)) {

        if(statfs(SDCARD1_PATH, &st) < 0) {
            ALOGE("Fail to calculate the capacity of sdcard\n");
            strlcpy(buf, "sdcard = not deteced \n", size);

        } else {
            uint64_t capacity = (uint64_t) st.f_blocks * (uint64_t) st.f_bsize;
            uint64_t free_space = (uint64_t) st.f_bfree * (uint64_t) st.f_bsize;

            *target_size = capacity / SIZE_1G;

            snprintf(buf, size, "sdcard_capacity = %" PRIu64 " \n", capacity);
            snprintf(tmp, sizeof(tmp), "sdcard_used_space = %" PRIu64 " \n", free_space);
            strlcat(buf, tmp, size);
            snprintf(tmp, sizeof(tmp), "sdcard_free_space = %" PRIu64 " \n", capacity - free_space);
            strlcat(buf, tmp, size);
            strlcat(buf, "sdcard = detected \n", size);
        }
        ret = SUCCESS;
    } else {
        strlcat(buf, "sdcard = not deteced \n", size);
    }

    return ret;
}

static int32_t module_init(const mmi_module_t * module, hash_map < string, string > &params) {
    ALOGI("%s start ", __FUNCTION__);

    if(module == NULL) {
        ALOGE("%s NULL point  received ", __FUNCTION__);
        return FAILED;
    }

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
    uint64_t target_size = 0, min_limit = 0, max_limit = 0;

    if(!module || !cmd) {
        ALOGE("%s NULL point  received ", __FUNCTION__);
        return FAILED;
    }
    ALOGI("%s start.command : %s", __FUNCTION__, cmd);

    /**Run test */
    if(!strcmp(params["type"].c_str(), "emmc")) {
        ret = emmc_info(buf, sizeof(buf), &target_size);
    } else if(!strcmp(params["type"].c_str(), "sdcard")) {
        ret = sdcard_info(buf, sizeof(buf), &target_size);
    } else {
        ALOGE("FFBM STORAGE: Unknow storage device");
        return FAILED;
    }
    ALOGI("%s start.command : target_size:%llu", __FUNCTION__, target_size);

    if(!strcmp(cmd, SUBCMD_MMI)) {
        module->cb_print(params[KEY_MODULE_NAME].c_str(), SUBCMD_MMI, buf, strlen(buf), PRINT_DATA);
    } else if(!strcmp(cmd, SUBCMD_PCBA)) {
        min_limit = string_to_long(params[KEY_MIN_LIMINT]);
        max_limit = string_to_long(params[KEY_MAX_LIMINT]);
        if(max_limit != 0) {
            if(target_size > min_limit && target_size < max_limit)
                ret = SUCCESS;
        }

        ALOGI("%s target_size : %llu limit[%llu,%llu] GB", __FUNCTION__, target_size, min_limit, max_limit);
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
    .name = "Storage",
    .author = "Qualcomm Technologies, Inc.",
    .methods = &module_methods,
    .module_handle = NULL,
    .supported_cmd_list = NULL,
    .supported_cmd_list_size = 0,
    .cb_print = NULL, /**it is initialized by mmi agent*/
    .run_pid = -1,
};
