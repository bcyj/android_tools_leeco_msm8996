/*
**
** Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved
** Qualcomm Technologies Confidential and Proprietary.
**
*/
#define LOG_TAG "DAP_UTIL"
//#define LOG_NDEBUG 0
//#define LOG_NDDEBUG 0
#include <cutils/log.h>
#include <tinyalsa/asoundlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <cutils/properties.h>
#include <stdlib.h>
#include "dap_hal_util.h"
#include "dap_hw_info.h"
#include "dap_hal_api.h"
#include "sound/devdep_params.h"

#define MAX_DAP_INSTANCES 5
#define DAP_HAL_INSTANCE_BASE 100
static int dap_instances[MAX_DAP_INSTANCES] = {-1};
static int dap_count = 0;

typedef void* dap_handle_t;
pthread_once_t once = PTHREAD_ONCE_INIT;

static void dap_init_once() {
    init_status = -1;

    pthread_mutex_init(&lock, NULL);
    if (dap_util_open() != NULL)
       init_status = 0;
}

int dap_util_initialize() {
    pthread_once(&once, dap_init_once);
    return init_status;
}

static void get_device_name(char* name, size_t len) {
    snprintf(name, len, "/dev/snd/hwC%uD%u", CARD_NO, DEVICE_ID);
    ALOGV("%s: device_name=%s\n", __func__, name);
}

static bool get_fd_idx_by_handle(dap_handle_t handle, int* fd, int* index) {
    *index = (unsigned long)handle - DAP_HAL_INSTANCE_BASE;
    if (*index < 0 || *index > (MAX_DAP_INSTANCES - 1)) {
        ALOGE("%s: Invalid dap handle\n", __func__);
        return false;
    }
    *fd = dap_instances[*index];
    ALOGV("%s handle=%ld fd=%d index=%d\n", __func__, (unsigned long)handle, *fd, *index);
    return true;
}

static dap_handle_t dap_util_insert(int fd) {
    int i = 0;
    dap_handle_t handle = NULL;

    for (i = 0; i < MAX_DAP_INSTANCES; i++) {
        if (dap_instances[i] == -1) {
            dap_instances[i] =  fd;
            dap_count++;
            handle = (void*)((unsigned long) (i + DAP_HAL_INSTANCE_BASE));
            ALOGV("%s: Inserting at index=%d handle=%ld\n", __func__, i, (unsigned long)handle);
            break;
        }
    }
    if (i == MAX_DAP_INSTANCES)
        ALOGE("%s Max DAP instances exceeded!\n", __func__);
    return handle;
}

dap_handle_t dap_util_open()
{
    int fd = -1;
    char dev_name[256];
    dap_handle_t handle = NULL;

    get_device_name((char*)dev_name, sizeof(dev_name));
    ALOGV("%s Opening device %s\n", __func__, dev_name);
    fd = open(dev_name, O_RDWR);
    if (fd < 0) {
        ALOGE("%s: cannot open device '%s'\n", __func__, dev_name);
        return handle;
    }
    ALOGV("%s success", __func__);
    handle = dap_util_insert(fd);

    if (handle != NULL) {
       dap_util_set_bypass_type(fd);
    }
    return handle;
}

void dap_util_close(dap_handle_t handle) {
    int fd = -1, index = 0;

    if (!get_fd_idx_by_handle(handle, &fd, &index)) {
        ALOGE("%s: Invalid dap handle\n", __func__);
        return;
    }
    close(fd);
    ALOGV("dap_util_close success\n");
    dap_instances[index] = -1;
    dap_count--;
}

bool dap_util_get_fd_by_handle(dap_handle_t handle, int* fd) {
    int index = (unsigned long)handle - DAP_HAL_INSTANCE_BASE;

    if (index < 0 || index > (MAX_DAP_INSTANCES - 1)) {
        ALOGE("%s: Invalid dap handle\n", __func__);
        return false;
    }
    *fd = dap_instances[index];
    ALOGV("%s handle=%ld fd=%d index=%d\n", __func__, (unsigned long)handle, *fd, index);
    return true;
}

bool dap_util_get_be_id_by_device_id(int device_id, int* be_id) {
    bool ret = false;
    int i = 0;
    int cnt = device_be_id_map_len;

    if (device_id == AUDIO_DEVICE_NONE) {
       ALOGV("%s Device id is AUDIO_DEVICE_NONE\n", __func__);
       return true;
    }
    for (i=0; i < cnt; i++) {
        if (msm_device_id_to_be_id[i][0] == device_id) {
            *be_id = msm_device_id_to_be_id[i][1];
            ALOGV("%s: device_id = 0x%x be_id = %d\n", __func__, device_id, *be_id);
            ret = true;
            break;
        }
    }
    return ret;
}

bool dap_util_is_hw_unsupported_device(int device_id) {
    bool ret = false;
    int i = 0;
    int cnt = sizeof(msm_hw_unsupported_device_id)/ (sizeof(int));

    for (i=0; i < cnt; i++) {
        if(msm_hw_unsupported_device_id[i] == device_id) {
            ALOGV("%s: device_id = 0x%x\n", __func__, device_id);
            ret = true;
            break;
        }
    }
    return ret;
}

bool dap_util_get_endpoint(int endpoint, int *num_endpoint, int *dev_arr) {
    int idx = 0;

    if (endpoint &  AUDIO_DEVICE_OUT_EARPIECE)
        dev_arr[idx++] = AUDIO_DEVICE_OUT_EARPIECE;
    if (endpoint &  AUDIO_DEVICE_OUT_SPEAKER)
        dev_arr[idx++] =  AUDIO_DEVICE_OUT_SPEAKER;
    if (endpoint &  AUDIO_DEVICE_OUT_WIRED_HEADSET)
        dev_arr[idx++] =  AUDIO_DEVICE_OUT_WIRED_HEADSET;
    if (endpoint &  AUDIO_DEVICE_OUT_WIRED_HEADPHONE)
        dev_arr[idx++] =  AUDIO_DEVICE_OUT_WIRED_HEADPHONE;
    if (endpoint &  AUDIO_DEVICE_OUT_BLUETOOTH_SCO)
        dev_arr[idx++] =  AUDIO_DEVICE_OUT_BLUETOOTH_SCO;
    if (endpoint &  AUDIO_DEVICE_OUT_AUX_DIGITAL)
        dev_arr[idx++] =  AUDIO_DEVICE_OUT_AUX_DIGITAL;
    if (endpoint &  AUDIO_DEVICE_OUT_ANLG_DOCK_HEADSET)
        dev_arr[idx++] =  AUDIO_DEVICE_OUT_ANLG_DOCK_HEADSET;
    if (endpoint &  AUDIO_DEVICE_OUT_DGTL_DOCK_HEADSET)
        dev_arr[idx++] =  AUDIO_DEVICE_OUT_DGTL_DOCK_HEADSET;
    if (endpoint &  AUDIO_DEVICE_OUT_REMOTE_SUBMIX)
        dev_arr[idx++] =  AUDIO_DEVICE_OUT_REMOTE_SUBMIX;
    if (endpoint &  AUDIO_DEVICE_OUT_PROXY)
        dev_arr[idx++] =  AUDIO_DEVICE_OUT_PROXY;
    if (endpoint &  AUDIO_DEVICE_OUT_FM)
        dev_arr[idx++] =  AUDIO_DEVICE_OUT_FM;
    if (endpoint &  AUDIO_DEVICE_OUT_FM_TX)
        dev_arr[idx++] =  AUDIO_DEVICE_OUT_FM_TX;
    if (endpoint & AUDIO_DEVICE_NONE)
        dev_arr[idx] = AUDIO_DEVICE_NONE;
    *num_endpoint = idx;
    return true;
}

dap_handle_t dap_util_get_handle() {
     dap_handle_t handle = NULL;

     if (dap_count == 0) {
         ALOGE("%s: Dap not initialized yet\n", __func__);
         return handle;
     }
     handle = (void*)(DAP_HAL_INSTANCE_BASE);
     ALOGV("%s: Dap handle returning handle %ld\n", __func__, (unsigned long) handle);
     return handle;
}

void dap_util_set_be_id_map(void *data) {
    dap_hal_device_be_id_map_t* be_id_map;

    be_id_map = (dap_hal_device_be_id_map_t*)data;
    msm_device_id_to_be_id = be_id_map->device_id_to_be_id;
    device_be_id_map_len = be_id_map->len;
    ALOGV("%s: Set be id map len= %d\n", __func__, device_be_id_map_len);
    return;
}

int dap_util_set_bypass_type(int fd)
{
    struct dolby_param_data param_data;
    char value[PROPERTY_VALUE_MAX] = {0};
    int32_t bypass_state = 0i, ret = 0;

    property_get("audio.dolby.ds2.hardbypass", value, NULL);
    bypass_state = atoi(value) || !strncmp("true", value, 4);
    ALOGV("bypass_state %d,  atoi(value) %d , value  %s",
          bypass_state,  atoi(value), value);
    param_data.device_id = AUDIO_DEVICE_OUT_DEFAULT;
    param_data.be_id = -1;
    param_data.param_id = DAP_CMD_SET_BYPASS_TYPE;
    param_data.data = &bypass_state;
    param_data.length = sizeof(int32_t);
    ALOGV("%s: device_id=%d be_id=%d command=%d data=%d\n", __func__,
          param_data.device_id, param_data.be_id,
          param_data.param_id, bypass_state);
    pthread_mutex_lock(&lock);
    ret = ioctl(fd, SNDRV_DEVDEP_DAP_IOCTL_DAP_COMMAND, &param_data);
    pthread_mutex_unlock(&lock);
    ALOGV("%s: ret = %d", __func__, ret);
    return ret;
}
