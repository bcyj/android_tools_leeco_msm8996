/*
**
** Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved
** Qualcomm Technologies Confidential and Proprietary.
**
*/

#define LOG_TAG "DAP_HAL"
//#define LOG_NDEBUG 0 /* Enable logging */
#include <sys/ioctl.h>
#include <cutils/log.h>
#include <stdlib.h>
#include <errno.h>
#include "sound/devdep_params.h"
#include "dap_hal_api.h"
#include "dap_hal_util.h"
#include "dap_hw_info.h"

#define DAP_HAL_INSTANCE_BASE 100
#define MAX_DAP_INSTANCES 5
#define MAX_LEN 256

dap_handle_t dap_open() {
    ALOGD("%s\n", __func__);
    if (dap_util_initialize() == 0) {
        return dap_util_get_handle();
    } else {
        ALOGE("%s: cannot open dap\n", __func__);
        return NULL;
    }
}

void dap_close(dap_handle_t handle) {
    pthread_mutex_lock(&lock);
    dap_util_close(handle);
    pthread_mutex_unlock(&lock);
}

int dap_command(dap_handle_t handle, audio_devices_t device_id,
                dap_cmd_t command, int32_t data) {
    int fd = -1, be_id = -1;
    struct dolby_param_data param_data;
    int ret = -EINVAL;

    if (!dap_util_get_fd_by_handle(handle, &fd)) {
        ALOGE("%s: Invalid dap handle %ld\n", __func__, (unsigned long)handle);
        return ret;
    }
    if (dap_util_is_hw_unsupported_device(device_id)) {
        ALOGE("%s: device not handled in DSP %x",__func__, device_id);
        return 0;
    }
    if (!dap_util_get_be_id_by_device_id(device_id, &be_id)) {
        ALOGE("%s: Invalid device_id = 0x%x command %d\n", __func__, device_id, command);
        return ret;
    }
    param_data.device_id = device_id;
    param_data.be_id = be_id;
    param_data.param_id = command;
    param_data.data = &data;
    param_data.length = sizeof(int32_t);
    ALOGV("%s: device_id=%d be_id=%d command=%d data=%d\n", __func__,
          device_id, be_id, command, data);
    pthread_mutex_lock(&lock);
    ret = ioctl(fd, SNDRV_DEVDEP_DAP_IOCTL_DAP_COMMAND, &param_data);
    pthread_mutex_unlock(&lock);
    return ret;
}


int dap_set_param(dap_handle_t handle, audio_devices_t device_id,
                  int32_t param_id, const int32_t* data, int32_t length) {
    int fd = -1, be_id = -1;
    struct dolby_param_data param_data;
    int ret = -EINVAL;

    if (data == NULL) {
        ALOGE("%s: Invalid data\n", __func__);
        return ret;
    }

    if (!dap_util_get_fd_by_handle(handle, &fd)) {
        ALOGE("%s: Invalid dap handle %ld\n", __func__, (unsigned long)handle);
        return ret;
    }
    if (dap_util_is_hw_unsupported_device(device_id)) {
        ALOGE("%s: device not handled in DSP %x",__func__, device_id);
        return 0;
    }
    if (!dap_util_get_be_id_by_device_id(device_id, &be_id)) {
        ALOGE("%s: Invalid device_id = 0x%x\n", __func__, device_id);
        return ret;
    }
    param_data.device_id = device_id;
    param_data.be_id = be_id;
    param_data.param_id = param_id;
    param_data.data = data;
    param_data.length = length;
    ALOGV("%s: device_id=%d be_id=%d param_id=%d data=%p len=%d\n", __func__,
          device_id, be_id, param_id, data, length);
    pthread_mutex_lock(&lock);
    ret = ioctl(fd, SNDRV_DEVDEP_DAP_IOCTL_SET_PARAM,
                &param_data);
    pthread_mutex_unlock(&lock);
    return ret;

}

int dap_get_param(dap_handle_t handle, audio_devices_t device_id,
                  int32_t param_id, int32_t* data, int32_t* length) {
    int fd = -1, be_id = -1;
    int ret = -EINVAL;
    struct dolby_param_data param_data;

    if ((data == NULL) || (length == NULL)) {
        ALOGE("%s: Invalid data/ len\n", __func__);
        return ret;
    }

    if (!dap_util_get_fd_by_handle(handle, &fd)) {
        ALOGE("%s: Invalid dap handle\n", __func__);
        return ret;
    }
    if (dap_util_is_hw_unsupported_device(device_id)) {
        ALOGE("%s: device not handled in DSP %x",__func__, device_id);
        return 0;
    }
    if (!dap_util_get_be_id_by_device_id(device_id, &be_id)) {
        ALOGE("%s: Invalid device_id = 0x%x\n", __func__, device_id);
        return ret;
    }
    param_data.device_id = device_id;
    param_data.be_id = be_id;
    param_data.param_id = param_id;
    param_data.data = data;
    param_data.length = *length;
    ALOGV("%s: device_id=%d be_id=%d param_id=%d data=%p len=%d\n", __func__,
          device_id, be_id, param_id, data, *length);
    pthread_mutex_lock(&lock);
    ret = ioctl(fd, SNDRV_DEVDEP_DAP_IOCTL_GET_PARAM,
                &param_data);
    pthread_mutex_unlock(&lock);
    if (ret == 0) {
        *length = param_data.length;
    }
    return ret;

}

int dap_get_visualizer(dap_handle_t handle, int32_t* data, int32_t* length) {
    int fd = -1, be_id = -1;
    int ret = -EINVAL;
    struct dolby_param_data param_data;

    if ((data == NULL) || (length == NULL)) {
        ALOGE("%s: Invalid data/ len\n", __func__);
        return ret;
    }

    if (!dap_util_get_fd_by_handle(handle, &fd)) {
        ALOGE("%s: Invalid dap handle\n", __func__);
        return ret;
    }
    param_data.data = data;
    param_data.length = *length;
    ALOGV("%s: data=%p len=%d\n", __func__,data, *length);
    pthread_mutex_lock(&lock);
    ret  = ioctl(fd, SNDRV_DEVDEP_DAP_IOCTL_GET_VISUALIZER,
                 &param_data);
    pthread_mutex_unlock(&lock);
    if (ret == 0) {
        *length = param_data.length;
    }
    return ret;
}

int dap_hal_set_hw_info(int32_t hw_info, void* data) {
    dap_hal_hw_info_t hal_hw_info = hw_info;
    int ret = 0, i = 0;
    int fd = -1;
    int num_endpoint = 0;
    int dev_arr[32];
    dap_handle_t handle = NULL;
    struct dolby_param_license *dolby_license;
    bool bypass = false;

    if (data == NULL) {
        ALOGE("%s: Invalid data\n", __func__);
        return -EINVAL;
    }

    if (hal_hw_info == SND_CARD) {
        snd_card = *((int*)data);
        ALOGV("%s: snd_card=%d\n", __func__, snd_card);
        return ret;
    } else if (hal_hw_info == DEVICE_BE_ID_MAP) {
        dap_util_set_be_id_map(data);
        ALOGV("%s: Set be id map\n", __func__);
        return ret;
    }

    if (dap_util_initialize() == 0) {
        handle = dap_util_get_handle();
    } else {
        ALOGE("%s: cannot open dap\n", __func__);
        return -EINVAL;
    }

    if (!dap_util_get_fd_by_handle(handle, &fd)) {
        ALOGE("%s: Invalid dap handle %ld\n", __func__, (unsigned long)handle);
        return -EINVAL;
    }
    if (hal_hw_info == DMID) {
        dolby_license = ((struct dolby_param_license *)data);
        ALOGE("%s: dmid=%d\n key %x0x", __func__, dolby_license->dmid, dolby_license->license_key);
        pthread_mutex_lock(&lock);
        ret = ioctl(fd, SNDRV_DEVDEP_DAP_IOCTL_DAP_LICENSE, dolby_license);
        pthread_mutex_unlock(&lock);
    } else if (hal_hw_info == HW_ENDPOINT) {
        endpoint = *((int*)data);
        ALOGV("%s: endpoint=0x%x\n", __func__, endpoint);
        dap_util_get_endpoint(endpoint, &num_endpoint, dev_arr);
        for (i = 0; i < num_endpoint; i++) {
            ret = dap_command(handle, dev_arr[i], DAP_CMD_SET_ACTIVE_DEVICE, dev_arr[i]);
        }
    } else if (hal_hw_info == DAP_BYPASS) {
        // TODO: what about soft bypass we would still send parameters.
        bypass = *((int*)data);
        ALOGE("%s: bypass=0x%x\n", __func__, bypass);
        ret = dap_command(handle, AUDIO_DEVICE_OUT_DEFAULT, DAP_CMD_SET_BYPASS,
                          bypass);
    } else {
        ALOGE("%s: Invalid hw info param %d\n", __func__, hw_info);
        ret = -EINVAL;
    }
    return ret;
}
