/*
 * Copyright (c) 2012-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef _AD_H
#define _AD_H

#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>
#include <poll.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <cutils/properties.h>
#include <cutils/log.h>

#include "common_log.h"
#include <cutils/sockets.h>
#include <linux/msm_mdp.h>
#include <linux/fb.h>

#include "lib-postproc.h"
#include "ScreenRefresher.h"
#include "PPDaemonUtils.h"
#include "DummySensor.h"
#include "Calib.h"

#ifdef ALS_ENABLE
#include "LightSensor.h"
#endif

#define AD_DEFAULT_MODE_PROP "ro.qcom.ad.default.mode"
#define AD_CALIB_DATA_PATH_PROP "ro.qcom.ad.calib.data"
#define AD_SENSOR_PROP "ro.qcom.ad.sensortype"

#define CALIB_INIT 0x1
#define CALIB_CFG 0x2
#define AD_PARAM_SEPARATOR ";"
#define AD_DATA_SEPARATOR ","
#define AD_INIT_PARAM_CNT 23
#define AD_CFG_PARAM_CNT 11
#define AD_MAX_DATA_CNT 256
#define BL_LIN_LUT_SIZE 256
#define BL_ATT_ALPHA_BASE 1024
#define ASYM_COL_AL_LUT_SIZE 33
#define START_ALS_VALUE 1000
#define AD_ENABLE_PRI 1
#define AD_ENABLE_WB 2
#define AD_REFRESH_CNT 256
#define AD_REFRESH_INTERVAL 16
#define AD_STRLIMT_MIN 0
#define AD_STRLIMT_MAX 255
#define AD_ASSERT_SLIDER_MAX 255
#define AD_ASSERT_SLIDER_MIN 0

#define CONTROL_DISABLE 0x0
#define CONTROL_ENABLE 0x1
#define CONTROL_PAUSE 0x2
#define CONTROL_RESUME 0x3

#define CALIB_READY(x) (x == (CALIB_INIT | CALIB_CFG))

#ifdef _ANDROID_
#define FRAMEBUFFER_NODE "/dev/graphics/fb"
#else
#define FRAMEBUFFER_NODE "/dev/fb"
#endif

enum ad_status {
    AD_OFF,
    AD_ON,
    AD_CALIB_OFF,
    AD_CALIB_ON
};

enum ad_mode {
    ad_mode_auto_bl,
    ad_mode_auto_str,
    ad_mode_calib
};

enum panel_mode {
    MIPI_DSI_VIDEO = '8',
    MIPI_DSI_CMD = '9',
};

struct ad_default_params {
    struct mdss_ad_init init;
    struct mdss_ad_cfg cfg;
};

enum ad_debug_level {
    LOG_SILENT,   /*no debug logs*/
    LOG_DEBUG_1,  /*level 1 debug logs*/
    LOG_DEBUG_2   /*debug 2 debug logs*/
};

class DaemonContext;

class AD {
    int mDisplayFd;
    int mEnable;
    int mADUpdateType;
    int mStatus;
    int mMode;
    bool mADSupported;
    int mALSValue;
    int mLastSentALSValue;
    int mPrevALSValue;
    bool bIsFirstRun;
    bool bIsNativeALS;
    ALS *mLightSensor;
    pthread_t mADThread;
    pthread_mutex_t mADLock;
    pthread_cond_t  mADCond;

    uint32_t mBLLinLUT[BL_LIN_LUT_SIZE];
    uint32_t mBLLinInvLUT[BL_LIN_LUT_SIZE];
    uint32_t mBLAttLUT[AD_BL_ATT_LUT_LEN];
    struct ad_default_params mDefADConfig;
    uint32_t mADCalibStatus;

    int ADParseCalibData(struct ad_default_params* params);
    int ADPrintCalibData(struct ad_default_params* params);
    void ProcessADWork();
    static void *ad_thrd_func(void *obj) {
        reinterpret_cast<AD *>(obj)->ProcessADWork();
        return NULL;
    }

    int ADRun(ad_mode mode);
    int ADCleanup();
    /*Don't allow copy or assignment operator for AD class;
      if copy or assignment is needed, move the copy constructor or
      assignement operator to public and implement deep copy for AD */
    AD& operator=(const AD& ad_obj);
    AD(const AD& ad_obj);
public:
    ScreenRefresher *mRefresher;
    int mDebug;
    int mAssertivenessSliderValue; // value between 0 - 255
    uint32_t mFlags;
    uint32_t mLastManualInput;
    uint32_t mPrevBL;
    uint16_t mAD_calib_a;
    uint16_t mAD_calib_c;
    uint16_t mAD_calib_d;
    uint16_t mAD_ALS_offset;
    double mAD_ALS_thresh;
    int isADEnabled(){
        return mEnable;
    }

    int ADStatus(){
        return mStatus;
    }

    int ADMode(){
        return mMode;
    }

    int ADCurrALSValue(){
        return mALSValue;
    }

    int ADLastSentALSValue(){
        return mLastSentALSValue;
    }

    uint8_t ADGetStrLimit(){
        return mDefADConfig.cfg.strength_limit;
    }

    void ADSetStrLimit(uint8_t strLimit){
        mDefADConfig.cfg.strength_limit = strLimit;
    }

    void ADSetLightSensor();
    void ADRemoveLightSensor();
    void ADSetScreenRefresher();
    void ADRemoveScreenRefresher();
    AD() : mDisplayFd(-1), mEnable(false), mADUpdateType(0), mStatus(false), mMode(-1),
        mADSupported(false), mALSValue(0), mLastSentALSValue(-1), mPrevALSValue(0),
        bIsFirstRun(true), bIsNativeALS(false), mLightSensor(NULL),
        mRefresher(NULL), mDebug(0), mFlags(0) {
        mAssertivenessSliderValue = 128;
        mPrevBL = -1;
        mAD_calib_a = 0;
        mAD_calib_c = 0;
        mAD_calib_d = 0;
        mAD_ALS_offset = AMBIENT_LIGHT_OFFSET;
        mAD_ALS_thresh = AMBIENT_LIGHT_THR;
        mEnable = display_pp_ad_supported();
        if (mEnable) {
            mADSupported = true;
        } else
            mADSupported = false;

        if (mADSupported) {
            char property[PROPERTY_VALUE_MAX];
            if (property_get("debug.ad.logs", property, 0) > 0 && (atoi(property) >= 0)) {
                mDebug = atoi(property);
            }
            mDefADConfig.init.bl_lin_len = BL_LIN_LUT_SIZE;
            mDefADConfig.init.bl_att_len = AD_BL_ATT_LUT_LEN;
            mDefADConfig.init.alpha_base = BL_ATT_ALPHA_BASE;
            mDefADConfig.init.bl_lin = &mBLLinLUT[0];
            mDefADConfig.init.bl_lin_inv = &mBLLinInvLUT[0];
            mDefADConfig.init.bl_att_lut = &mBLAttLUT[0];

            ADSetLightSensor();
            ADSetScreenRefresher();
        }

        pthread_mutex_init(&mADLock, NULL);
        pthread_cond_init(&mADCond, NULL);
    }

    ~AD() {
        pthread_mutex_destroy(&mADLock);
        pthread_cond_destroy(&mADCond);
        ADRemoveLightSensor();
        ADRemoveScreenRefresher();
    }

    int ADControl(DaemonContext *ctx, int enableBit, ad_mode mode = ad_mode_calib, int display_id = MDP_BLOCK_MAX);
    int ADInit(char* initial);
    int ADConfig(char* config);
    int ADInput(int amlight);
    int ADUpdateAL(int amlight, int refresh_cnt);
    int ADSetCalibMode(int mode);
    int ADSetCalibBL(int bl);
    int ADSetupMode();
    int ADCalcCalib();
    int Notify(uint32_t notification_type);
    bool IsADInputValid(ad_mode mode, int display_id) {
        if (mode != ad_mode_auto_str) {
            LOGE("Invalid AD mode!");
            return false;
        }
        if (display_id < MDP_LOGICAL_BLOCK_DISP_0 || display_id > MDP_LOGICAL_BLOCK_DISP_2) {
            LOGE("Invalid AD display option!");
            return false;
        }
        return true;
    }
    bool ADThresholdCheck(int prev_als, int curr_als) {
        double diff = fabs((log(prev_als + mAD_ALS_offset) - log(curr_als + mAD_ALS_offset)) / log(2));
        return ((diff > mAD_ALS_thresh) ? 1 : 0);
    }
};

#endif /* _AD_H */
