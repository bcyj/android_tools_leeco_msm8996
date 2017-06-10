/*
 * Copyright (c) 2012-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef _CABL_H
#define _CABL_H

#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include <sys/types.h>
#include <cutils/properties.h>

#include <linux/msm_mdp.h>
#include <linux/fb.h>

#include "abl_driver.h"

#include "ScreenRefresher.h"
#include "ConfigParameters.h"
#include "PPDaemonUtils.h"

#define CMD_DEBUG_PREFIX "debug:"
#define CMD_DEBUG_CABL_ON "debug:cabl:on"
#define CMD_DEBUG_CABL_OFF "debug:cabl:off"

enum cabl_status {
    CABL_OFF = false,
    CABL_ON  = true
};

class CABL {
public:
    bool mEnable;
    bool mDebug;
    CABLHWParams mOEM;
    volatile cabl_status eStatus;
    bool mHistStatus;
    bool mPowerLevelChange;
    int32_t mPowerSaveLevel;
    int32_t mPrevNotify;
    uint32_t mUserBLLevel;
    uint32_t mScale;
    struct fb_cmap mColorMap;
    struct hw_info_hist mHistInfo;
    bl_oem_api mOEMParams;
    pthread_t mControlThreadID;
    pthread_t mWorkerThreadID;
    pthread_mutex_t mCABLCtrlLock;
    pthread_mutex_t mCABLLock;
    pthread_cond_t  mCABLCond;
    char mPanelMode;
    ScreenRefresher *mRefresher;
    int32_t auto_adjust_quality_lvl();
    int32_t ConfigureABLParameters(bl_oem_api*,uint32_t,uint32_t);
    void initHW();
    CABL() : mEnable(false), mDebug(false), eStatus(CABL_OFF), mHistStatus(0),
            mPowerLevelChange(false), mPowerSaveLevel(ABL_QUALITY_HIGH),
            mPrevNotify(NOTIFY_TYPE_SUSPEND), mScale(1024), mRefresher(NULL) {
        /* initialize the cabl thread sync variables */
        pthread_mutex_init(&mCABLCtrlLock, NULL);
        pthread_mutex_init(&mCABLLock, NULL);
        pthread_cond_init(&mCABLCond, NULL);
        /* set the flags based on property */
        char property[PROPERTY_VALUE_MAX];
        if (property_get("ro.qualcomm.cabl", property, NULL) > 0 && (atoi(property) == 1)) {
            mEnable = true;
        }
        if (property_get("debug.cabl.logs", property, 0) > 0 && (atoi(property) > 0 )) {
            mDebug = true;
        }
    }
    ~CABL() {
        pthread_mutex_destroy(&mCABLCtrlLock);
        pthread_mutex_destroy(&mCABLLock);
        pthread_cond_destroy(&mCABLCond);
    }
    int32_t start_cabl();
    void stop_cabl();
    int Notify(uint32_t notification_type);
    inline uint32_t get_scale() {
        return mScale;
    }
};

#endif /* _CABL_H */
