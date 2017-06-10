/**
 *  Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 *  Qualcomm Technologies Proprietary and Confidential.
 *
 *  Overview: This file defines various generic ioctls for getting/setting
 *            various parameters or executing fm commands
 *
 **/

#include "QcomFmIoctlsInterface.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <cutils/properties.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <math.h>

int QcomFmIoctlsInterface :: start_fm_patch_dl
(
    UINT fd
)
{
    int ret;
    int init_success = 0;
    char versionStr[MAX_VER_STR_LEN] = {'\0'};
    char prop_value[PROPERTY_VALUE_MAX] = {'\0'};
    struct v4l2_capability cap;

    ret = ioctl(fd, VIDIOC_QUERYCAP, &cap);
    if(ret == IOCTL_SUCC) {
        ret = snprintf(versionStr, MAX_VER_STR_LEN, "%d", cap.version);
        if(ret >= MAX_VER_STR_LEN) {
            return FM_FAILURE;
        }else {
            ret = property_set(FM_VERSION_PROP, versionStr);
            if(ret != PROP_SET_SUCC)
               return FM_FAILURE;
            ret = property_set(FM_MODE_PROP, "normal");
            if(ret != PROP_SET_SUCC)
               return FM_FAILURE;
            ret = property_set(FM_INIT_PROP, "0");
            if(ret != PROP_SET_SUCC)
               return FM_FAILURE;
            ret = property_set(SCRIPT_START_PROP, SOC_PATCH_DL_SCRPT);
            if(ret != PROP_SET_SUCC)
               return FM_FAILURE;
            for(int i = 0; i < INIT_LOOP_CNT; i++) {
                property_get(FM_INIT_PROP, prop_value, NULL);
                if (strcmp(prop_value, "1") == 0) {
                    init_success = 1;
                    break;
                }else {
                    usleep(INIT_WAIT_TIMEOUT);
                }
            }
            if(!init_success) {
                property_set(SCRIPT_STOP_PROP, SOC_PATCH_DL_SCRPT);
                return FM_FAILURE;
            }else {
                return FM_SUCCESS;
            }
        }
    }else {
        return FM_FAILURE;
    }
}

int  QcomFmIoctlsInterface :: close_fm_patch_dl
(
    void
)
{
    int ret;

    ret = property_set(SCRIPT_STOP_PROP, SOC_PATCH_DL_SCRPT);
    if(ret != PROP_SET_SUCC) {
        return FM_FAILURE;
    }else {
        return FM_SUCCESS;
    }
}

int  QcomFmIoctlsInterface :: get_cur_freq
(
    UINT fd, long &freq
)
{
    int ret;
    struct v4l2_frequency channel;

    channel.type = V4L2_TUNER_RADIO;
    ret = ioctl(fd, VIDIOC_G_FREQUENCY, &channel);

    if(ret < IOCTL_SUCC) {
        return FM_FAILURE;
    }else {
        freq = (channel.frequency / TUNE_MULT);
        return FM_SUCCESS;
    }
}

int  QcomFmIoctlsInterface :: set_freq
(
    UINT fd, ULINT freq
)
{
    int ret;
    struct v4l2_frequency channel;

    channel.type = V4L2_TUNER_RADIO;
    channel.frequency = (freq * TUNE_MULT);

    ret = ioctl(fd, VIDIOC_S_FREQUENCY, &channel);
    if(ret < IOCTL_SUCC) {
        return FM_FAILURE;
    }else {
        return FM_SUCCESS;
    }
}

int  QcomFmIoctlsInterface :: set_control
(
    UINT fd, UINT id, int val
)
{
    int ret;
    struct v4l2_control control;

    control.value = val;
    control.id = id;

    ret = ioctl(fd, VIDIOC_S_CTRL, &control);
    if(ret < IOCTL_SUCC) {
        return FM_FAILURE;
    }else {
        return FM_SUCCESS;
    }
}

int  QcomFmIoctlsInterface :: set_calibration
(
    UINT fd
)
{
    int ret;
    FILE *cal_fp;
    struct v4l2_ext_control ext_ctl;
    struct v4l2_ext_controls v4l2_ctls;
    char cal_data[CAL_DATA_SIZE] = {0};

    cal_fp = fopen(CALIB_DATA_NAME, "r");
    if(cal_fp != NULL) {
       if(fread(&cal_data[0], 1, CAL_DATA_SIZE, cal_fp)
           < CAL_DATA_SIZE) {
           fclose(cal_fp);
           return FM_FAILURE;
       }
       fclose(cal_fp);
       ext_ctl.id = V4L2_CID_PRV_SET_CALIBRATION;
       ext_ctl.string = cal_data;
       ext_ctl.size = CAL_DATA_SIZE;
       v4l2_ctls.ctrl_class = V4L2_CTRL_CLASS_USER;
       v4l2_ctls.count = 1;
       v4l2_ctls.controls = &ext_ctl;
       ret = ioctl(fd, VIDIOC_S_EXT_CTRLS, &v4l2_ctls);
       if(ret < IOCTL_SUCC) {
           return FM_FAILURE;
       }else {
           return FM_SUCCESS;
       }
    }else {
        return FM_FAILURE;
    }
}

int  QcomFmIoctlsInterface :: get_control
(
    UINT fd, UINT id, long &val
)
{
    int ret;
    struct v4l2_control control;

    control.id = id;
    ret = ioctl(fd, VIDIOC_G_CTRL, &control);
    if(ret < IOCTL_SUCC) {
        return FM_FAILURE;
    }else {
        val = control.value;
        return FM_SUCCESS;
    }
}

int  QcomFmIoctlsInterface :: start_search
(
    UINT fd, UINT dir
)
{
    int ret;
    struct v4l2_hw_freq_seek hw_seek;

    hw_seek.seek_upward = dir;
    hw_seek.type = V4L2_TUNER_RADIO;

    ret = ioctl(fd, VIDIOC_S_HW_FREQ_SEEK, &hw_seek);
    if(ret < IOCTL_SUCC) {
        return FM_FAILURE;
    }else {
        return FM_SUCCESS;
    }
}

int  QcomFmIoctlsInterface :: set_band
(
    UINT fd, ULINT low, ULINT high
)
{
    int ret;
    struct v4l2_tuner tuner;

    tuner.index = 0;
    tuner.signal = 0;
    tuner.rangelow = (low * TUNE_MULT);
    tuner.rangehigh = (high * TUNE_MULT);

    ret = ioctl(fd, VIDIOC_S_TUNER, &tuner);
    ret = set_control(fd, V4L2_CID_PRV_REGION, 0);
    if(ret < IOCTL_SUCC) {
        return FM_FAILURE;
    }else {
        return FM_SUCCESS;
    }
}

int QcomFmIoctlsInterface :: get_rmssi
(
    UINT fd, long &rmssi
)
{
    struct v4l2_tuner tuner;
    int ret;

    tuner.index = 0;
    tuner.signal = 0;
    ret = ioctl(fd, VIDIOC_G_TUNER, &tuner);
    if(ret < IOCTL_SUCC) {
        ret = FM_SUCCESS;
    }else {
        rmssi = tuner.signal;
        ret = FM_SUCCESS;
    }
    return ret;
}

int  QcomFmIoctlsInterface :: get_upperband_limit
(
    UINT fd, ULINT &freq
)
{
    int ret;
    struct v4l2_tuner tuner;

    ret = ioctl(fd, VIDIOC_G_TUNER, &tuner);
    if(ret < IOCTL_SUCC) {
        return FM_FAILURE;
    }else {
        freq = (tuner.rangehigh / TUNE_MULT);
        return FM_SUCCESS;
    }
}

int  QcomFmIoctlsInterface :: get_lowerband_limit
(
    UINT fd, ULINT &freq
)
{
    int ret;
    struct v4l2_tuner tuner;

    ret = ioctl(fd, VIDIOC_G_TUNER, &tuner);
    if(ret < IOCTL_SUCC) {
        return FM_FAILURE;
    }else {
        freq = (tuner.rangelow / TUNE_MULT);
        return FM_SUCCESS;
    }
}

int  QcomFmIoctlsInterface :: set_audio_mode
(
    UINT fd, enum AUDIO_MODE mode
)
{
    int ret;
    struct v4l2_tuner tuner;

    tuner.index = 0;
    ret = ioctl(fd, VIDIOC_G_TUNER, &tuner);
    if(ret < IOCTL_SUCC) {
        return FM_FAILURE;
    }else {
        tuner.audmode = mode;
        ret = ioctl(fd, VIDIOC_S_TUNER, &tuner);
        if(ret < IOCTL_SUCC) {
            return FM_FAILURE;
        }else {
            return FM_SUCCESS;
        }
    }
}

int  QcomFmIoctlsInterface :: get_buffer
(
     UINT fd, char *buff, UINT len, UINT index
)
{
    int ret;
    struct v4l2_buffer v4l2_buf;

    if((len < STD_BUF_SIZE) || (buff == NULL)) {
        return FM_FAILURE;
    }else {
        memset(&v4l2_buf, 0, sizeof(v4l2_buf));
        v4l2_buf.index = index;
        v4l2_buf.type = V4L2_BUF_TYPE_PRIVATE;
        v4l2_buf.length = STD_BUF_SIZE;
        v4l2_buf.m.userptr = (ULINT)buff;
        ret = ioctl(fd, VIDIOC_DQBUF, &v4l2_buf);
        if(ret < IOCTL_SUCC) {
            return FM_FAILURE;
        }else {
            return v4l2_buf.bytesused;
        }
    }
}
