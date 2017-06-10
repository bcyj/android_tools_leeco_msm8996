/*
 * Copyright (c) 2014, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#include "mmi_module.h"

/**
* Defined case run in mmi mode,this mode support UI.
*
*/
struct audio_params {
    char *sz_testcase;
    char *sz_duration;
    char *sz_volume;
};

static struct audio_params g_audio_params;
static const char next_tune[] = "next_tune";

static const char *extra_cmd_list[] = {
    next_tune
};

static int g_pid = -1;

static void local_exe_cmd(const char *cmd, char *result, int size) {
    char line[SIZE_1K];
    char ps[SIZE_1K] = { 0 };
    char name[SIZE_1K] = { 0 };
    char value[SIZE_1K] = { 0 };
    char indicator = ':';
    FILE *pp;

    strlcpy(ps, cmd, sizeof(ps));
    if((pp = popen(ps, "r")) != NULL) {
        while(fgets(line, sizeof(line), pp) != NULL) {
            char *p = &line[0];

            parse_nv_by_indicator(p, indicator, name, sizeof(name), value, sizeof(value));
            char *pname = trim(name);
            char *pvalue = trim(value);

            if(*pname != '\0' && *pvalue != '\0') {
                if(!strncmp(pname, "result", 6)) {
                    strlcpy(result, pvalue, size);
                    break;
                }
            }
        }
        ALOGE("close the pipe \n");
        pclose(pp);
        pp = NULL;
    } else {
        ALOGE("popen %s error\n", ps);
    }
}

static void fm_disable(void) {
    char fm_test_cmd[SIZE_512] = { 0 };
    char result[256] = { 0 };

    snprintf(fm_test_cmd, sizeof(fm_test_cmd), "%s %s", FM_APP, " disable");
    ALOGE("cmd:%s \n", fm_test_cmd);
    local_exe_cmd(fm_test_cmd, result, sizeof(result));
}

static int fm_tune(hash_map < string, string > &params) {
    char fm_test_cmd[SIZE_512] = { 0 };
    char result[SIZE_2K] = { 0 };
    int ret = 0;

    snprintf(fm_test_cmd, sizeof(fm_test_cmd), "%s %s %s", FM_APP, " tune ", (char *) params["tune"].c_str());
    ALOGE("cmd:%s \n", fm_test_cmd);
    local_exe_cmd(fm_test_cmd, result, sizeof(result));
    ret = atoi(result);
    return ret;
}

static int fm_seeknext(const mmi_module_t * mod) {
    char fm_test_cmd[SIZE_512] = { 0 };
    char result[SIZE_2K] = { 0 };
    int tune = 0;

    snprintf(fm_test_cmd, sizeof(fm_test_cmd), "%s %s", FM_APP, " seeknext");
    ALOGE("cmd:%s \n", fm_test_cmd);
    mod->cb_print(NULL, SUBCMD_MMI, "search_tune", 11, PRINT);
    local_exe_cmd(fm_test_cmd, result, sizeof(result));
    ALOGE("fm_seeknext result:%s \n", result);
    tune = atoi(result);
    snprintf(result, sizeof(result), "%s:%d.%dMHz", "playing_tune", tune / 1000, (tune % 1000) / 100);
    mod->cb_print(NULL, SUBCMD_MMI, result, strlen(result), PRINT);
    sleep(2);
    return 0;
}

static int fm_enable(bool enable, const mmi_module_t * mod) {
    char fm_test_cmd[SIZE_512] = { 0 };
    char result[256] = { 0 };
    int ret = FAILED;

    snprintf(fm_test_cmd, sizeof(fm_test_cmd), "%s %s", FM_APP, " enable");
    ALOGE("cmd:%s \n", fm_test_cmd);
    if(enable)
        mod->cb_print(NULL, SUBCMD_MMI, "FM enable", 9, PRINT);

    local_exe_cmd(fm_test_cmd, result, sizeof(result));
    ret = atoi(result);

    ALOGE("enable result:%s \n", result);
    return ret;
}

static void exe_set_audio_path(void) {
    int pid = fork();

    if(pid == 0) {
        const char *args[10] = { AUDIO_FTM, "-tc", (const char *) g_audio_params.sz_testcase,
            "-c", AUDIO_FTM_CONFIG, "-d", (const char *) g_audio_params.sz_duration,
            "-v", (const char *) g_audio_params.sz_volume, NULL
        };
        int ret = execv((char *) AUDIO_FTM, (char **) args);

        if(ret == -1) {
            ALOGE("execv fail exit: %s\n", strerror(errno));
            _exit(100);
        }
    } else if(pid > 0) {
        g_pid = pid;
        waitpid(g_pid, NULL, 0);
        g_pid = -1;
        ALOGE("FM module: wait mm audio setting finished\n");
    } else if(pid < 0) {
        perror("fork fail");
    }
}

static void *run_test(void *mod) {
    mmi_module_t *module = (mmi_module_t *) mod;

    if(module->cb_print == NULL) {
        ALOGE("%s NULL for cb function ", __FUNCTION__);
        return NULL;
    }

    signal(SIGUSR1, signal_handler);
    ((mmi_module_t *) mod)->cb_print(NULL, SUBCMD_MMI, "config_audio", 12, PRINT);
    exe_set_audio_path();

    return NULL;
}

static int32_t module_run_mmi(const mmi_module_t * module, hash_map < string, string > &params) {
    ALOGI("%s start ", __FUNCTION__);
    int ret = FAILED;
    char result[SIZE_2K] = { 0 };

    ret = pthread_create((pthread_t *) & module->run_pid, NULL, run_test, (void *) module);
    if(ret < 0) {
        ALOGE("%s:Can't create pthread: %s\n", __FUNCTION__, strerror(errno));
        return FAILED;
    } else {
        ret = fm_enable(true, module);
        if(ret == SUCCESS) {
            ret = fm_tune(params);
            if(ret == SUCCESS) {
                snprintf(result, sizeof(result), "%s:%sMHz", "playing_tune", (char *) params["tune"].c_str());
                module->cb_print(NULL, SUBCMD_MMI, result, strlen(result), PRINT);
            }
        }
        pthread_join(module->run_pid, NULL);
    }

    return SUCCESS;
}

/**
* Defined case run in PCBA mode, fully automatically.
*
*/
static int32_t module_run_pcba(const mmi_module_t * module, hash_map < string, string > &params) {
    ALOGI("%s start", __FUNCTION__);
    int ret = FAILED;

    sleep(4);
    ret = fm_enable(true, module);
    if(ret == SUCCESS)
        ret = fm_tune(params);

    fm_disable();

    if(ret == SUCCESS) {
        module->cb_print(NULL, SUBCMD_PCBA, "FM test pass\n", 13, PRINT);
        return SUCCESS;
    } else {
        module->cb_print(NULL, SUBCMD_PCBA, "FM test fail\n", 13, PRINT);
        return FAILED;
    }
}

static int32_t module_init(const mmi_module_t * module, hash_map < string, string > &params) {
    ALOGI("%s start ", __FUNCTION__);

    if(module == NULL) {
        ALOGE("%s NULL point  received ", __FUNCTION__);
        return FAILED;
    }

    memset(&g_audio_params, 0, sizeof(struct audio_params));
    g_audio_params.sz_testcase = (char *) params["tc"].c_str();
    g_audio_params.sz_duration = (char *) params["duration"].c_str();
    g_audio_params.sz_volume = (char *) params["volume"].c_str();

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

    fm_disable();

    if(g_pid > 0) {
        kill(g_pid, SIGTERM);
        g_pid = -1;
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
    else if(!strcmp(cmd, next_tune))
        ret = fm_seeknext(module);
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
    .name = "FM",
    .author = "Qualcomm Technologies, Inc.",
    .methods = &module_methods,
    .module_handle = NULL,
    .supported_cmd_list = extra_cmd_list,
    .supported_cmd_list_size = sizeof(extra_cmd_list) / sizeof(char *),
    .cb_print = NULL, /**it is initialized by mmi agent*/
    .run_pid = -1,
};
