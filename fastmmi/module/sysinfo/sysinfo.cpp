/*
 * Copyright (c) 2014, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#include "nv.h"
#include "mmi_module.h"

static char nv_buf[SIZE_1K] = { 0 };

static int32_t module_init(const mmi_module_t * module, hash_map < string, string > &params) {
    unsigned char tmp[256] = { 0 };
    char format_buf[256] = { 0 };

    ALOGI("%s start", __FUNCTION__);
    if(module == NULL) {
        ALOGE("%s NULL point  received ", __FUNCTION__);
        return FAILED;
    }

    /* Calling LSM init  */
    if(!Diag_LSM_Init(NULL)) {
        ALOGE("fastmmi Test App: Diag_LSM_Init() failed.");
        return -1;
    }

    ALOGI("FASTMMI Test App: Diag_LSM_Init succeeded. \n");
    /* Register the callback for the primary processor */
    register_callback();

    /**Read modem version*/
    diag_send_cmd(DIAG_EXT_BUILD_ID_F, tmp, sizeof(tmp));
    diag_ext_build_id_rsp_type *p_modem_v = (diag_ext_build_id_rsp_type *) tmp;

    snprintf(format_buf, sizeof(format_buf), "Modem = %s\n", p_modem_v->ver_strings);
    strlcat(nv_buf, format_buf, sizeof(nv_buf));

    memset(format_buf, 0, sizeof(format_buf));
    diag_nv_read(NV_UE_IMEI_I, tmp, sizeof(tmp));
    strlcat(nv_buf, "IMEI = ", sizeof(nv_buf));
    charArray_to_hexArray((char *) tmp, sizeof(tmp), 5, 8, (char *) format_buf, sizeof(format_buf), true);
    strlcat(nv_buf, format_buf, sizeof(nv_buf));
    strlcat(nv_buf, "\n", sizeof(nv_buf));

    memset(tmp, 0, sizeof(tmp));
    diag_nv_read(NV_OEM_ITEM_1_I, tmp, sizeof(tmp));
    snprintf(format_buf, sizeof(format_buf), "OEMITEM1 = %s\n", tmp);
    strlcat(nv_buf, format_buf, sizeof(nv_buf));

    memset(tmp, 0, sizeof(tmp));
    diag_nv_read(NV_OEM_ITEM_2_I, tmp, sizeof(tmp));
    snprintf(format_buf, sizeof(format_buf), "OEMITEM2 = %s\n", tmp);
    strlcat(nv_buf, format_buf, sizeof(nv_buf));

    memset(tmp, 0, sizeof(tmp));
    diag_nv_read(NV_OEM_ITEM_3_I, tmp, sizeof(tmp));
    snprintf(format_buf, sizeof(format_buf), "OEMITEM3 = %s\n", tmp);
    strlcat(nv_buf, format_buf, sizeof(nv_buf));

    return SUCCESS;
}

static int32_t module_deinit(const mmi_module_t * module) {
    ALOGI("%s start.", __FUNCTION__);
    Diag_LSM_DeInit();
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

    int ret = -1;
    char buf[SIZE_1K] = { 0 };
    unsigned char tmp[256] = { 0 };
    char format_buf[256] = { 0 };
    char software_version[PROPERTY_VALUE_MAX] = { 0 };

    if(!module || !cmd) {
        ALOGE("%s NULL point  received ", __FUNCTION__);
        return FAILED;
    }
    ALOGI("%s start.command : %s", __FUNCTION__, cmd);

    /**MMI version info*/
    snprintf(format_buf, sizeof(format_buf), "MMI = %s\n", MMI_VERSION);
    strlcat(buf, format_buf, sizeof(buf));

    property_get("ro.build.description", software_version, "normal");
    snprintf(format_buf, sizeof(format_buf), "SW version = %s\n", software_version);
    strlcat(buf, format_buf, sizeof(buf));

    strlcat(buf, nv_buf, sizeof(buf));

    if(!strcmp(cmd, SUBCMD_MMI))
        module->cb_print(params[KEY_MODULE_NAME].c_str(), SUBCMD_MMI, buf, strlen(buf), PRINT_DATA);
    else if(!strcmp(cmd, SUBCMD_PCBA))
        module->cb_print(params[KEY_MODULE_NAME].c_str(), SUBCMD_PCBA, buf, strlen(buf), PRINT_DATA);

   /** Default RUN mmi*/
    return SUCCESS;
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
    .name = "Sysinfo",
    .author = "Qualcomm Technologies, Inc.",
    .methods = &module_methods,
    .module_handle = NULL,
    .supported_cmd_list = NULL,
    .supported_cmd_list_size = 0,
    .cb_print = NULL, /**it is initialized by mmi agent*/
    .run_pid = -1,
};
