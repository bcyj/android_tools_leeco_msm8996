/**
 *  Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 *  Qualcomm Technologies Proprietary and Confidential.
 *
 *  Overview: This file declares generic ioctl interface for fm commands
 *
 **/

#ifndef __QCOM_FM_IOCTL_INTERFACE_H__
#define __QCOM_FM_IOCTL_INTERFACE_H__

#include "Qualcomm_FM_Const.h"

class QcomFmIoctlsInterface
{
    public:
        static int start_fm_patch_dl(UINT fd);
        static int close_fm_patch_dl(void);
        static int get_cur_freq(UINT fd, long &freq);
        static int set_freq(UINT fd, ULINT freq);
        static int set_control(UINT fd, UINT id, int val);
        static int set_calibration(UINT fd);
        static int get_control(UINT fd, UINT id, long &val);
        static int start_search(UINT fd, UINT dir);
        static int set_band(UINT fd, ULINT low, ULINT high);
        static int get_upperband_limit(UINT fd, ULINT &freq);
        static int get_lowerband_limit(UINT fd, ULINT &freq);
        static int set_audio_mode(UINT fd, enum AUDIO_MODE mode);
        static int get_buffer(UINT fd, char *buff, UINT len, UINT index);
        static int get_rmssi(UINT fd, long &rmssi);
};

#endif //__QCOM_FM_IOCTL_INTERFACE_H__
