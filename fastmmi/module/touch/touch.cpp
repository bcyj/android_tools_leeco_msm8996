/*
 * Copyright (c) 2014, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#include <linux/input.h>
#include "mmi_module.h"

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
    DIR *dir;
    struct dirent *de;
    int fd;
    int i = 0, j = 0;
    bool ret = FAILED;
    int max_event_index = 0;
    int index = 0;
    char buffer[NAME_MAX];
    char format_buf[NAME_MAX];
    char filepath[PATH_MAX] = { 0 };
    unsigned long keyBitmask[BITS_TO_LONGS(KEY_MAX)];
    unsigned long absBitmask[BITS_TO_LONGS(ABS_MAX)];
    struct input_id id;

    if(module == NULL) {
        ALOGE("%s NULL point  received ", __FUNCTION__);
        return FAILED;
    }

    dir = opendir("/dev/input");
    if(dir == 0)
        return -1;

    while((de = readdir(dir))) {
        if(strncmp(de->d_name, "event", 5))
            continue;
        get_device_index(de->d_name, "event", &index);
        if(index > max_event_index)
            max_event_index = index;
    }

    for(i = 0; i < max_event_index + 1; i++) {
        unsigned long ev_bits[BITS_TO_LONGS(EV_MAX)];

        snprintf(filepath, sizeof(filepath), "/dev/input/event%d", i);
        fd = open(filepath, O_RDONLY);
        if(fd < 0)
            continue;
        /* read the evbits of the input device */
        if(ioctl(fd, EVIOCGBIT(0, sizeof(ev_bits)), ev_bits) < 0) {
            close(fd);
            continue;
        }

        /* TODO: add ability to specify event masks. For now, just assume
         * that only EV_KEY and EV_REL event types are ever needed. */
        if(!test_bit(EV_KEY, ev_bits) && !test_bit(EV_REL, ev_bits)) {
            ALOGE("could not get EV_KEY for %d, %s\n", fd, strerror(errno));
            close(fd);
            continue;
        }

        /* read the evbits of the input device */
        if(ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(keyBitmask)), keyBitmask) < 0) {
            ALOGE("could not get keyBitmask for fd:%d, %s\n", fd, strerror(errno));
            close(fd);
            continue;
        }

        if(ioctl(fd, EVIOCGBIT(EV_ABS, sizeof(absBitmask)), absBitmask) < 0) {
            ALOGE("could not get absBitmask for fd:%d, %s\n", fd, strerror(errno));
            close(fd);
            continue;
        }
        /*See if this is a touch pad. Is this a new modern multi-touch driver */
        if(test_bit(ABS_MT_POSITION_X, absBitmask)
           && test_bit(ABS_MT_POSITION_Y, absBitmask)) {

            ret = SUCCESS;
            if(ioctl(fd, EVIOCGNAME(sizeof(format_buf) - 1), &format_buf) < 1) {
                ALOGE("could not get device name for fd:%d, %s\n", fd, strerror(errno));
                close(fd);
                continue;
            }
            snprintf(buffer, sizeof(buffer), "name = %s\n", format_buf);

            if(ioctl(fd, EVIOCGID, &id)) {
                ALOGE("could not get device id for fd:%d, %s\n", fd, strerror(errno));
                close(fd);
                continue;
            }

            snprintf(format_buf, sizeof(format_buf), "bus = %04x\n"
                     "vendor = %04x\n"
                     "product = %04x\n" "version = %04x\n", id.bustype, id.vendor, id.product, id.version);
            strlcat(buffer, format_buf, sizeof(buffer));
            module->cb_print(params[KEY_MODULE_NAME].c_str(), SUBCMD_PCBA, buffer, strlen(buffer), PRINT_DATA);

        }
    }
    closedir(dir);

    return ret;
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

    if(!strcmp(cmd, SUBCMD_PCBA))
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
