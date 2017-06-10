/*
 * Copyright (c) 2014, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#include "mmi_module.h"

/**
* Defined case run in mmi mode,this mode support UI.
*
*/
static exec_cmd_t execmd;

#define PCBA_REPLY_STR "Test finished, please check the result\n"

static int32_t module_run_mmi(const mmi_module_t * module, hash_map < string, string > &params) {
    ALOGI("%s start ", __FUNCTION__);
    const char *args[16];
    int i = 0;
    int ret = FAILED;
    char result[SIZE_2K] = { 0 };
    char soundCardInfo[256] = { 0 };
    char sound_card[256] = { 0 };
    char ftm_config_file[100] = { "/etc/ftm_test_config_" };
    FILE *fp = NULL;

    if((fp = fopen("/proc/asound/cards", "r")) != NULL) {
        while(fgets(soundCardInfo, sizeof(soundCardInfo), fp) != NULL)
            sscanf(soundCardInfo, "%*s%*s%*s%*s%s", sound_card);
        fclose(fp);
    }

    strlcat(ftm_config_file, sound_card, sizeof(ftm_config_file));

    args[i++] = AUDIO_FTM;
    args[i++] = "-c";

    if((fp = fopen(ftm_config_file, "rb"))) {
        args[i++] = ftm_config_file;
        fclose(fp);
    } else
        args[i++] = AUDIO_FTM_CONFIG;

    args[i++] = "-tc";
    args[i++] = params["tc"].c_str();
    args[i++] = "-d";
    args[i++] = params["duration"].c_str();
    args[i++] = "-v";
    args[i++] = params["volume"].c_str();

    if(!params["file"].empty()) {
        args[i++] = "-file";
        args[i++] = params["file"].c_str();
    }

    if(!params["fl"].empty()) {
        args[i++] = "-fl";
        args[i++] = params["fl"].c_str();
    }
    if(!params["fh"].empty()) {
        args[i++] = "-fh";
        args[i++] = params["fh"].c_str();
    }
    args[i] = NULL;

    memset(&execmd, 0, sizeof(exec_cmd_t));
    execmd.cmd = (char *) AUDIO_FTM;
    execmd.pid = -1;
    execmd.result = result;
    execmd.size = sizeof(result);
    execmd.params = (char **) args;

    ret = exe_cmd(NULL, &execmd);

    sleep(1);

    const char *args_play[12] = { AUDIO_FTM, "-c", AUDIO_FTM_CONFIG,
        "-tc", "8", "-d", params["duration"].c_str(), "-v", params["volume"].c_str(), "-file",
        params["file"].c_str(), NULL
    };

    if(!strncmp(params["tc"].c_str(), "1", strlen(params["tc"].c_str()))) {
        memset(&execmd, 0, sizeof(exec_cmd_t));
        execmd.cmd = (char *) AUDIO_FTM;
        execmd.pid = -1;
        execmd.result = result;
        execmd.size = sizeof(result);
        execmd.params = (char **) args_play;
        ret = exe_cmd(NULL, &execmd);
    }

    return ret;
}

/**
* Defined case run in PCBA mode, fully automatically.
*
*/
static int32_t module_run_pcba(const mmi_module_t * module, hash_map < string, string > &params) {
    ALOGI("%s start", __FUNCTION__);
    char result[SIZE_2K] = { 0 };
    const char *args[16];
    int i = 0;

   /**Before start kill the previous*/
    if(execmd.pid > 0) {
        kill(execmd.pid, SIGTERM);
        usleep(100);
        kill_proc(execmd.pid);
    }
    args[i++] = AUDIO_FTM;
    args[i++] = "-c";
    args[i++] = AUDIO_FTM_CONFIG;
    args[i++] = "-tc";
    args[i++] = params["tc"].c_str();
    args[i++] = "-d";
    args[i++] = params["duration"].c_str();
    args[i++] = "-v";
    args[i++] = params["volume"].c_str();

    if(!params["file"].empty()) {
        args[i++] = "-file";
        args[i++] = params["file"].c_str();
    }

    if(!params["fl"].empty()) {
        args[i++] = "-fl";
        args[i++] = params["fl"].c_str();
    }
    if(!params["fh"].empty()) {
        args[i++] = "-fh";
        args[i++] = params["fh"].c_str();
    }
    args[i] = NULL;

    memset(&execmd, 0, sizeof(exec_cmd_t));
    execmd.cmd = (char *) AUDIO_FTM;
    execmd.pid = -1;
    execmd.result = result;
    execmd.size = sizeof(result);
    execmd.params = (char **) args;

    exe_cmd(NULL, &execmd);
    module->cb_print(params[KEY_MODULE_NAME].c_str(), SUBCMD_PCBA, PCBA_REPLY_STR, strlen(PCBA_REPLY_STR), PRINT);

    return ERR_UNKNOW;
}

static int32_t module_init(const mmi_module_t * module, hash_map < string, string > &params) {
    ALOGI("%s start ", __FUNCTION__);
    int ret = FAILED;

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

    kill(execmd.pid, SIGTERM);
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
    .name = "Audio",
    .author = "Qualcomm Technologies, Inc.",
    .methods = &module_methods,
    .module_handle = NULL,
    .supported_cmd_list = NULL,
    .supported_cmd_list_size = 0,
    .cb_print = NULL, /**it is initialized by mmi agent*/
    .run_pid = -1,
};
