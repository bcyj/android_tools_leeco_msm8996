/*
* Copyright (c) 2014-2015 Qualcomm Technologies, Inc. All Rights Reserved.
* Qualcomm Technologies Proprietary and Confidential.
*
* Not a Contribution.
* Apache license notifications and license are retained
* for attribution purposes only.
*/

 /*
  * Copyright (C) 2009 The Android Open Source Project
  *
  * Licensed under the Apache License, Version 2.0 (the "License");
  * you may not use this file except in compliance with the License.
  * You may obtain a copy of the License at
  *
  *      http://www.apache.org/licenses/LICENSE-2.0
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  */

#include <hardware/lights.h>
#include <hardware/hardware.h>
#include "mmi_module.h"

/**
* Defined case run in mmi mode,this mode support UI.
*
*/
static const char str_led_on[] = "Do you see LED light on?";
static const char str_led_off[] = "Do you see LED light off?";
static const char str_btn_on[] = "Do you see button light on?";
static const char str_btn_off[] = "Do you see button light off?";
static const char str_lcd[] = "Do you see LCD backlight brightness change?";

enum {
    LIGHT_INDEX_BACKLIGHT = 0,
    LIGHT_INDEX_KEYBOARD = 1,
    LIGHT_INDEX_BUTTONS = 2,
    LIGHT_INDEX_BATTERY = 3,
    LIGHT_INDEX_NOTIFICATIONS = 4,
    LIGHT_INDEX_ATTENTION = 5,
    LIGHT_INDEX_BLUETOOTH = 6,
    LIGHT_INDEX_WIFI = 7,
    LIGHT_COUNT,
};

struct Devices {
    light_device_t *lights[LIGHT_COUNT];
};

static Devices *devices = NULL;
static light_device_t *dev = NULL;
static int light_device = 0;
static hash_map < string, string > *paras;

static int32_t module_stop(const mmi_module_t * module);

static light_device_t *get_device(hw_module_t * module, char const *name) {
    int err;
    hw_device_t *device;

    err = module->methods->open(module, name, &device);
    if(err == 0) {
        return (light_device_t *) device;
    } else {
        return NULL;
    }
}

static int hal_init(void) {
    int ret = FAILED;
    hw_module_t *hw_module = NULL;

    devices = (struct Devices *) malloc(sizeof(struct Devices));
    if(devices == NULL) {
        ALOGE("Light FFBM : fail to malloc for devices");
        return ret;
    }

    ret = hw_get_module(LIGHTS_HARDWARE_MODULE_ID, (hw_module_t const **) &hw_module);
    if(ret == SUCCESS) {
        devices->lights[LIGHT_INDEX_BACKLIGHT]
            = get_device(hw_module, LIGHT_ID_BACKLIGHT);
        devices->lights[LIGHT_INDEX_KEYBOARD]
            = get_device(hw_module, LIGHT_ID_KEYBOARD);
        devices->lights[LIGHT_INDEX_BUTTONS]
            = get_device(hw_module, LIGHT_ID_BUTTONS);
        devices->lights[LIGHT_INDEX_BATTERY]
            = get_device(hw_module, LIGHT_ID_BATTERY);
        devices->lights[LIGHT_INDEX_NOTIFICATIONS]
            = get_device(hw_module, LIGHT_ID_NOTIFICATIONS);
        devices->lights[LIGHT_INDEX_ATTENTION]
            = get_device(hw_module, LIGHT_ID_ATTENTION);
        devices->lights[LIGHT_INDEX_BLUETOOTH]
            = get_device(hw_module, LIGHT_ID_BLUETOOTH);
        devices->lights[LIGHT_INDEX_WIFI]
            = get_device(hw_module, LIGHT_ID_WIFI);
    } else {
        ALOGE("Light FFBM : hw_get_module() call failed \n");
        free(devices);
        devices = NULL;
    }

    return ret;
}

static void enable(const mmi_module_t * module, bool on) {
    light_state_t state;

    memset(&state, 0, sizeof(light_state_t));
    /*Tell user we are waiting for valid data */

    if(light_device == LIGHT_INDEX_BUTTONS) {   //Button light test
        state.flashMode = LIGHT_FLASH_NONE;
        state.flashOnMS = 0;
        state.flashOffMS = 0;
        state.brightnessMode = BRIGHTNESS_MODE_USER;
        state.color = 0xFF020202;

        if(!on)
            state.color = 0xFF000000;
        else
            state.color = 0xFF020202;
    } else if(light_device == LIGHT_INDEX_NOTIFICATIONS) {  // LED light test
        state.flashMode = LIGHT_FLASH_NONE;
        state.flashOnMS = 0;
        state.flashOffMS = 0;
        state.brightnessMode = BRIGHTNESS_MODE_USER;

        if(!on)
            state.color = 0x000000;
        else {
            if(!strncmp((*paras)["color"].c_str(), "red", 3))
                state.color = 0xFF0000;
            else if(!strncmp((*paras)["color"].c_str(), "blu", 3))
                state.color = 0x0000FF;
            else if(!strncmp((*paras)["color"].c_str(), "gre", 3))
                state.color = 0x00FF00;
            else
                ALOGE("FFBM LIGHT: Unknow LED color");
        }
    } else if(light_device == LIGHT_INDEX_BACKLIGHT) {
        if(!on)
            state.color = 0xFF0000;
        else
            state.color = 0xFFFFFF;
    }

    devices->lights[light_device]->set_light(devices->lights[light_device], &state);

    if(light_device == LIGHT_INDEX_BUTTONS) {
        if(on)
            module->cb_print(NULL, SUBCMD_MMI, str_btn_on, strlen(str_btn_on), PRINT);
        else
            module->cb_print(NULL, SUBCMD_MMI, str_btn_off, strlen(str_btn_off), PRINT);
    } else if(light_device == LIGHT_INDEX_NOTIFICATIONS) {
        if(on)
            module->cb_print(NULL, SUBCMD_MMI, str_led_on, strlen(str_led_on), PRINT);
        else
            module->cb_print(NULL, SUBCMD_MMI, str_led_off, strlen(str_led_off), PRINT);
    } else if(light_device == LIGHT_INDEX_BACKLIGHT)
        module->cb_print(NULL, SUBCMD_MMI, str_lcd, strlen(str_lcd), PRINT);
    else
        ALOGE("FFBM LIGHT: Unknow light device");
}

static int init(hash_map < string, string > &params) {
    paras = &params;
    if(!strncmp(params["type"].c_str(), "led", 3))
        light_device = LIGHT_INDEX_NOTIFICATIONS;
    else if(!strncmp(params["type"].c_str(), "but", 3))
        light_device = LIGHT_INDEX_BUTTONS;
    else if(!strncmp(params["type"].c_str(), "lcd", 3))
        light_device = LIGHT_INDEX_BACKLIGHT;
    else {
        light_device = LIGHT_COUNT;
        ALOGE("FFBM LIGHT: Unknow light device");
        return FAILED;
    }
    return SUCCESS;
}

static void *run_test(void *mod) {
    mmi_module_t *module = (mmi_module_t *) mod;
    int delay = 0;

    delay = atoi((*paras)["delay"].c_str());

    signal(SIGUSR1, signal_handler);
    while(1) {
        enable((const mmi_module_t *) mod, true);
        usleep(1000 * delay);
        enable((const mmi_module_t *) mod, false);
        usleep(1000 * delay);
    }

    return NULL;
}

static void deinit(const mmi_module_t * mod) {
    enable(mod, false);
}

static int32_t module_run_mmi(const mmi_module_t * module, hash_map < string, string > &params) {
    ALOGI("%s start ", __FUNCTION__);
    int ret = FAILED;

    ret = init(params);
    if(ret != SUCCESS) {
        ALOGE("FFBM Light : fail to initialize");
        return ret;
    }

    ret = pthread_create((pthread_t *) & module->run_pid, NULL, run_test, (void *) module);
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
    char buf[64] = { 0 };
    if(!strncmp(params["switch"].c_str(), "on", 2)) {
        init(params);
        enable(module, true);
        snprintf(buf, sizeof(buf), "%s on\n", params["type"].c_str());
    } else if(!strncmp(params["switch"].c_str(), "off", 3)) {
        module_stop(module);
        snprintf(buf, sizeof(buf), "%s off\n", params["type"].c_str());
    }
    module->cb_print(NULL, SUBCMD_PCBA, buf, strlen(buf), PRINT);

    return ERR_UNKNOW;
}

static int32_t module_init(const mmi_module_t * module, hash_map < string, string > &params) {
    ALOGI("%s start ", __FUNCTION__);
    int ret = FAILED;

    if(module == NULL) {
        ALOGE("%s NULL point  received ", __FUNCTION__);
        return FAILED;
    }

    ret = hal_init();
    if(ret != SUCCESS) {
        ALOGE("FFBM Light : fail to initialize light HAL");
        return ret;
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

    pthread_kill(module->run_pid, SIGUSR1);
    deinit(module);
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
    .name = "Light",
    .author = "Qualcomm Technologies, Inc.",
    .methods = &module_methods,
    .module_handle = NULL,
    .supported_cmd_list = NULL,
    .supported_cmd_list_size = 0,
    .cb_print = NULL, /**it is initialized by mmi agent*/
    .run_pid = -1,
};
