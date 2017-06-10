/*
* Copyright (c) 2014-2015 Qualcomm Technologies, Inc. All Rights Reserved.
* Qualcomm Technologies Proprietary and Confidential.
*
* Not a Contribution.
* Apache license notifications and license are retained
* for attribution purposes only.
*/

 /*
  * Copyright (C) 2008 The Android Open Source Project
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

#include <hardware/hardware.h>
#include <utils/Timers.h>
#include "mmi_module.h"
#include "sensors_extension.h"

/**
* Defined case run in mmi mode,this mode support UI.
*
*/
static const char *subcmd_calibration = "calibration";

static const char *extra_cmd_list[] = {
    subcmd_calibration,
};

static struct sensors_poll_device_1_ext_t *device = NULL;
static struct sensor_t const *sensor_list = NULL;
static int dev_count = 0;
static int cur_sensor_type = 0;
static char cur_module_name[32];
static int calibration_result = FAILED;
static mutex_locker g_mutex;

static char const *get_sensor_name(int type) {
    switch (type) {
    case SENSOR_TYPE_ACCELEROMETER:
        return "Acc";
    case SENSOR_TYPE_MAGNETIC_FIELD:
        return "Mag";
    case SENSOR_TYPE_ORIENTATION:
        return "Ori";
    case SENSOR_TYPE_GYROSCOPE:
        return "Gyr";
    case SENSOR_TYPE_LIGHT:
        return "Lux";
    case SENSOR_TYPE_PRESSURE:
        return "Bar";
    case SENSOR_TYPE_TEMPERATURE:
        return "Tmp";
    case SENSOR_TYPE_PROXIMITY:
        return "Prx";
    case SENSOR_TYPE_GRAVITY:
        return "Grv";
    case SENSOR_TYPE_LINEAR_ACCELERATION:
        return "Lac";
    case SENSOR_TYPE_ROTATION_VECTOR:
        return "Rot";
    case SENSOR_TYPE_RELATIVE_HUMIDITY:
        return "Hum";
    case SENSOR_TYPE_AMBIENT_TEMPERATURE:
        return "Tam";
    }
    return "ukn";
}

static int get_sensor_type(const char *name) {
    int sensor_type = 0;

    if(!strncmp(name, "acc", 3))
        sensor_type = SENSOR_TYPE_ACCELEROMETER;
    else if(!strncmp(name, "mag", 3))
        sensor_type = SENSOR_TYPE_MAGNETIC_FIELD;
    else if(!strncmp(name, "ori", 3))
        sensor_type = SENSOR_TYPE_ORIENTATION;
    else if(!strncmp(name, "gyr", 3))
        sensor_type = SENSOR_TYPE_GYROSCOPE;
    else if(!strncmp(name, "lig", 3))
        sensor_type = SENSOR_TYPE_LIGHT;
    else if(!strncmp(name, "bar", 3))
        sensor_type = SENSOR_TYPE_PRESSURE;
    else if(!strncmp(name, "tmp", 3))
        sensor_type = SENSOR_TYPE_TEMPERATURE;
    else if(!strncmp(name, "pro", 3))
        sensor_type = SENSOR_TYPE_PROXIMITY;
    else if(!strncmp(name, "grv", 3))
        sensor_type = SENSOR_TYPE_GRAVITY;
    else if(!strncmp(name, "lac", 3))
        sensor_type = SENSOR_TYPE_LINEAR_ACCELERATION;
    else if(!strncmp(name, "rot", 3))
        sensor_type = SENSOR_TYPE_ROTATION_VECTOR;
    else if(!strncmp(name, "hum", 3))
        sensor_type = SENSOR_TYPE_RELATIVE_HUMIDITY;
    else if(!strncmp(name, "tam", 3))
        sensor_type = SENSOR_TYPE_AMBIENT_TEMPERATURE;

    return sensor_type;
}

static int sensor_enable(int sensor_type, int delay, bool enable) {
    int err = FAILED;

    if(sensor_list == NULL || sensor_type <= 0) {
        ALOGE("Invalid sensor number %d passed to initSensor", sensor_type);
        return FAILED;
    }

    for(int i = 0; i < dev_count; i++) {
        if(sensor_list[i].type == sensor_type) {

            if(enable)
                device->setDelay((sensors_poll_device_t *) device, sensor_list[i].handle, ms2ns(delay));

            ALOGI("Activating/Deactiveating sensor : %s", get_sensor_name(sensor_type));
            err = device->activate((sensors_poll_device_t *) device, sensor_list[i].handle, enable);
            if(err != SUCCESS) {
                ALOGE("activate() for '%s'failed (%s)\n", sensor_list[i].name, strerror(-err));
            }
            break;
        }
    }

    return err;
}


static int do_calibration(int sensor_type) {
    int i = 0;
    int ret = FAILED;
    bool found = false;
    struct cal_cmd_t para;

    memset(&para, 0, sizeof(cal_cmd_t));
    for(i = 0; i < dev_count; i++) {
        if(sensor_list[i].type == sensor_type) {
            switch (sensor_list[i].type) {
            case SENSOR_TYPE_ACCELEROMETER:
                found = true;
                para.axis = 3;
                para.save = 1;
                para.apply_now = 1;
                break;
            case SENSOR_TYPE_PROXIMITY:
                found = true;
                para.axis = 2;
                para.save = 1;
                para.apply_now = 1;
                break;
            default:
                break;
            }
            break;
        }
    }

    if(found && !sensor_enable(sensor_type, 0, false)) {
        ret =
            device->calibrate(reinterpret_cast < struct sensors_poll_device_1_ext_t *>(device),
                              sensor_list[i].handle, &para);
    }

    return ret;
}


static int test_event(sensors_event_t * event, hash_map < string, string > &params) {
    int err = FAILED;
    double x_min, x_max, y_min, y_max, z_min, z_max, v_min, v_max;

    if(event != NULL) {
        switch (event->type) {
        case SENSOR_TYPE_ACCELEROMETER:
        case SENSOR_TYPE_MAGNETIC_FIELD:
        case SENSOR_TYPE_ORIENTATION:
        case SENSOR_TYPE_GYROSCOPE:
        case SENSOR_TYPE_GRAVITY:
        case SENSOR_TYPE_LINEAR_ACCELERATION:
        case SENSOR_TYPE_ROTATION_VECTOR:

            x_min = atof(params["x_min_limit"].c_str());
            x_max = atof(params["x_max_limit"].c_str());
            y_min = atof(params["y_min_limit"].c_str());
            y_max = atof(params["y_max_limit"].c_str());
            z_min = atof(params["z_min_limit"].c_str());
            z_max = atof(params["z_max_limit"].c_str());

            ALOGI("limitation for(%s):x(%5.5f,%5.5f),y(%5.5f,%5.5f),z(%5.5f,%5.5f). data(%5.5f,%5.5f,%5.5f)",
                  get_sensor_name(event->type), x_min, x_max, y_min, y_max, z_min, z_max, event->data[0],
                  event->data[1], event->data[2]);
            if(inside_float(event->data[0], x_min, x_max) && inside_float(event->data[1], y_min, y_max)
               && inside_float(event->data[2], z_min, z_max))
                err = SUCCESS;
            break;

        case SENSOR_TYPE_LIGHT:
        case SENSOR_TYPE_PRESSURE:
        case SENSOR_TYPE_TEMPERATURE:
        case SENSOR_TYPE_PROXIMITY:
        case SENSOR_TYPE_RELATIVE_HUMIDITY:
        case SENSOR_TYPE_AMBIENT_TEMPERATURE:
            v_min = atof(params["min_limit"].c_str());
            v_max = atof(params["max_limit"].c_str());

            ALOGI("limitation(%s):value(%5.5f,%5.5f). data(%5.5f)", get_sensor_name(event->type), v_min, v_max,
                  event->data[0]);

            if(inside_float(event->data[0], v_min, v_max))
                err = SUCCESS;

            break;
        default:
            ALOGE("FFBM SENSOR: Data received, but sensor is unknown... returning");
            break;
        }
    }

    return err;
}


static void print_event(sensors_event_t * event, const mmi_module_t * mod, bool is_pcba) {
    int err = FAILED;
    char print_buf[256];

    if(event == NULL || mod == NULL)
        return;

    switch (event->type) {
    case SENSOR_TYPE_ACCELEROMETER:
    case SENSOR_TYPE_MAGNETIC_FIELD:
    case SENSOR_TYPE_ORIENTATION:
    case SENSOR_TYPE_GYROSCOPE:
    case SENSOR_TYPE_GRAVITY:
    case SENSOR_TYPE_LINEAR_ACCELERATION:
    case SENSOR_TYPE_ROTATION_VECTOR:

        snprintf(print_buf,
                 sizeof(print_buf),
                 "%s\nx = %5.5f\ny = %5.5f\nz = %5.5f\n", (char *) get_sensor_name(event->type),
                 event->data[0], event->data[1], event->data[2]);

        break;
    case SENSOR_TYPE_LIGHT:
    case SENSOR_TYPE_PRESSURE:
    case SENSOR_TYPE_TEMPERATURE:
    case SENSOR_TYPE_PROXIMITY:
    case SENSOR_TYPE_RELATIVE_HUMIDITY:
    case SENSOR_TYPE_AMBIENT_TEMPERATURE:
        snprintf(print_buf, sizeof(print_buf), "%s\nvalue = %5.5f\n",
                 (char *) get_sensor_name(event->type), event->data[0]);

        break;
    default:
        ALOGE("FFBM SENSOR: Data received, but sensor is unknown... returning");
        break;
    }


    if(event->type == SENSOR_TYPE_ACCELEROMETER || event->type == SENSOR_TYPE_PROXIMITY) {
        strlcat(print_buf, "calibration result = ", sizeof(print_buf));
        strlcat(print_buf, calibration_result == 0 ? "success \n" : "failed \n", sizeof(print_buf));
    }

    if(!is_pcba)
        mod->cb_print(cur_module_name, SUBCMD_MMI, print_buf, strlen(print_buf), PRINT_DATA);
    else
        mod->cb_print(cur_module_name, SUBCMD_PCBA, print_buf, strlen(print_buf), PRINT_DATA);

}

static void get_sensor_data(int sensor_type, sensors_event_t * event) {
    int n = 0;

    if(device == NULL || event == NULL)
        return;

    while(1) {
        n = device->poll((sensors_poll_device_t *) device, event, 1);
        if(n > 0 && event->type == sensor_type)
            break;
    }
}

static void *run_test(void *mod) {
    signal(SIGUSR1, signal_handler);
    sensors_event_t event;
    mmi_module_t *module = (mmi_module_t *) mod;

    if(module == NULL) {
        ALOGE("%s NULL for cb function ", __FUNCTION__);
        return NULL;
    }

    while(1) {
        get_sensor_data(cur_sensor_type, &event);
        print_event(&event, module, false);
    }

    return NULL;
}

static int32_t module_run_calibration(const mmi_module_t * module, hash_map < string, string > &params) {
    int ret = FAILED;

    mutex_locker::autolock _L(g_mutex);
    cur_sensor_type = get_sensor_type(params["type"].c_str());
    calibration_result = do_calibration(cur_sensor_type);

    sensor_enable(cur_sensor_type, atoi(params["delay"].c_str()), true);

    return calibration_result;
}

static int32_t module_run_mmi(const mmi_module_t * module, hash_map < string, string > &params) {
    ALOGI("%s start ", __FUNCTION__);
    int ret = FAILED;

    mutex_locker::autolock _L(g_mutex);
    cur_sensor_type = get_sensor_type(params["type"].c_str());
    calibration_result = do_calibration(cur_sensor_type);
    strlcpy(cur_module_name,params[KEY_MODULE_NAME].c_str(),sizeof(cur_module_name));

    ret = sensor_enable(cur_sensor_type, atoi(params["delay"].c_str()), true);
    if(ret != SUCCESS) {
        ALOGE("FFBM SENSOR : fail to initialize");
        return ret;
    }

    ret = pthread_create((pthread_t *) & module->run_pid, NULL, run_test, (void *) module);
    if(ret < 0) {
        ALOGE("%s:Can't create pthread: %s\n", __FUNCTION__, strerror(errno));
        return FAILED;
    }

    return SUCCESS;
}

/**
* Defined case run in PCBA mode, fully automatically.
*
*/
static int32_t module_run_pcba(const mmi_module_t * module, hash_map < string, string > &params) {
    ALOGI("%s start", __FUNCTION__);

    int ret = FAILED, tried = 3;
    sensors_event_t event;

    mutex_locker::autolock _L(g_mutex);
    cur_sensor_type = get_sensor_type(params["type"].c_str());
    strlcpy(cur_module_name,params[KEY_MODULE_NAME].c_str(),sizeof(cur_module_name));

    /**open sensor*/
    ret = sensor_enable(cur_sensor_type, atoi(params["delay"].c_str()), true);
    if(ret != SUCCESS) {
        ALOGE("FFBM SENSOR : fail to initialize");
        return FAILED;
    }

    /**Test several data, if any of them correct will returen succesful*/
    while(tried-- > 0) {
        get_sensor_data(cur_sensor_type, &event);
        ret = test_event(&event, params);
        if(ret == SUCCESS)
            break;
    }

    /**Close sensor*/
    sensor_enable(cur_sensor_type, 0, false);
    ALOGI("%s  finished", __FUNCTION__);
    print_event(&event, module, true);

    return ret;
}

static int32_t module_init(const mmi_module_t * module, hash_map < string, string > &params) {
    ALOGI("%s start ", __FUNCTION__);
    struct sensors_module_t *hal_mod = NULL;
    int err = FAILED;
    int i = 0;

    if(module == NULL) {
        ALOGE("%s NULL point  received ", __FUNCTION__);
        return FAILED;
    }

    err = hw_get_module(SENSORS_HARDWARE_MODULE_ID, (hw_module_t const **) &hal_mod);
    if(err != 0) {
        ALOGE("FFBM SENSOR: hw_get_module() failed (%s)\n", strerror(-err));
        return FAILED;
    }

    err = sensors_open_ext(&hal_mod->common, &device);
    if(err != 0) {
        ALOGE("FFBM SENSOR: sensors_open_ext() failed (%s)\n", strerror(-err));
        return FAILED;
    }

    dev_count = hal_mod->get_sensors_list(hal_mod, &sensor_list);
    for(i = 0; i < dev_count; i++) {
        ALOGI("FFBM SENSOR: Deactivating all sensor after open,current index: %d", i);
        err = device->activate((sensors_poll_device_t *) device, sensor_list[i].handle, 0);
        if(err != SUCCESS) {
            ALOGE("FFBM SENSOR: deactivate() for '%s'failed (%s)\n", sensor_list[i].name, strerror(-err));
            sensors_close_ext(device);
            return FAILED;
        }
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
    ALOGI("FFBM MMI test thread exit, disable the sensor(%s) unlock", get_sensor_name(cur_sensor_type));
    sensor_enable(cur_sensor_type, 0, false);

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
        ALOGE("%s NULL point received ", __FUNCTION__);
        return FAILED;
    }
    ALOGI("%s start.command : %s", __FUNCTION__, cmd);

    if(!strcmp(cmd, SUBCMD_MMI))
        ret = module_run_mmi(module, params);
    else if(!strcmp(cmd, SUBCMD_PCBA))
        ret = module_run_pcba(module, params);
    else if(!strcmp(cmd, subcmd_calibration))
        ret = module_run_calibration(module, params);
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
    .name = "Sensor",
    .author = "Qualcomm Technologies, Inc.",
    .methods = &module_methods,
    .module_handle = NULL,
    .supported_cmd_list = extra_cmd_list,
    .supported_cmd_list_size = sizeof(extra_cmd_list) / sizeof(char *),
    .cb_print = NULL, /**it is initialized by mmi agent*/
    .run_pid = -1,
};
