/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
@file  Calib.h
@brief Interface for Display Calibration Manager

GENERAL DESCRIPTION

  Interface implementation of  Display Calibration Manager.

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

EXTERNALIZED FUNCTIONS
  None

INITIALIZATION AND SEQUENCING REQUIREMENTS

*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/


#ifndef __CALIB_H__
#define __CALIB_H__

#include <stdlib.h>
#include <pthread.h>
#include <cutils/properties.h>

#define LOG_TAG "PPDaemon"

/*=========================================================================*/
/*Interface API declarations*/
/*=========================================================================*/
enum dcm_status {
     DCM_OFF = false,
     DCM_ON  = true
};


class DCM {
    pthread_mutex_t mDCMOpLock;
public:
    bool mDebug;
    bool mStatus;
    DCM() : mDebug(false), mStatus(DCM_OFF) {
        char property[PROPERTY_VALUE_MAX];
        if (property_get("debug.calib.logs", property, 0) > 0 && (atoi(property) == 1)) {
                mDebug = true;
        }
        pthread_mutex_init(&mDCMOpLock, NULL);
    }

    ~DCM() {
        pthread_mutex_destroy(&mDCMOpLock);
    }

    int DCMControl(bool flag);
};

#endif //__CALIB_H__
