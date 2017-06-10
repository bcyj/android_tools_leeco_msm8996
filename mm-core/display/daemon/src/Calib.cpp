/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
@file  Calib.cpp
@brief Interface for Display Calibration Manager

GENERAL DESCRIPTION

  Interface implementation of Display Calibration Manager.

  Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

EXTERNALIZED FUNCTIONS
  None

INITIALIZATION AND SEQUENCING REQUIREMENTS

*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/


#include "utils/Log.h"
#include "common_log.h"
#include "Calib.h"
#include "diagplatform.h"

int DCM::DCMControl(bool flag){
    int ret = -1;
    pthread_mutex_lock(&mDCMOpLock);
    if (flag){
        if (mStatus == DCM_ON) {
            LOGD("DCM is already on !");
            goto end;
        }
        ret = dcm_os_intf_control(TRUE);
        if (!ret){
            mStatus = DCM_ON;
        } else {
            LOGE("Unable to initialize DCM");
        }

    } else {
        if (mStatus == DCM_OFF) {
            LOGD("DCM is already off !");
            goto end;
        }
        ret = dcm_os_intf_control(FALSE);
        if (!ret) {
            mStatus = DCM_OFF;
        } else {
            LOGE("Unable to deinitialize DCM");
        }
    }
end:
    pthread_mutex_unlock(&mDCMOpLock);
    return ret;
}

