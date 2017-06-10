/*
 * DESCRIPTION
 * This file contains the context for running the AD thread and algorithm.
 *
 * Copyright (c) 2012-2015 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 */

#include "AD.h"
#include "DaemonContext.h"

int ad_init_data_cnt[AD_INIT_PARAM_CNT] = {33, 33, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 256, 256, 1, 33, 1, 1};
int ad_cfg_data_cnt[AD_CFG_PARAM_CNT] = {1, 33, 1, 1, 1, 1, 2, 4, 1, 1, 1};

int AD::Notify(uint32_t notification_type) {

    pthread_mutex_lock(&mADLock);
    if (notification_type == NOTIFY_TYPE_UPDATE) {
        mADUpdateType = NOTIFY_TYPE_UPDATE;
        pthread_cond_signal(&mADCond);
    }
    else if (notification_type == NOTIFY_TYPE_SUSPEND) {
        mADUpdateType = NOTIFY_TYPE_SUSPEND;
    }
    pthread_mutex_unlock(&mADLock);

    if (notification_type == NOTIFY_TYPE_SUSPEND) {
        pthread_mutex_lock(&mLightSensor->mALSLock);
        pthread_cond_signal(&mLightSensor->mALSCond);
        pthread_mutex_unlock(&mLightSensor->mALSLock);
    }

    if (notification_type == NOTIFY_TYPE_BL_UPDATE) {
        if (mRefresher) {
            LOGD_IF(mRefresher->mDebug, "Calling refresh()");
            mRefresher->Refresh(AD_REFRESH_CNT, AD_REFRESH_INTERVAL);
        }
    }

    return 0;
}

void AD::ProcessADWork() {
    int ret = -1, tmp_ALS = 0;

    if(mADSupported ) {
        ret = ADSetupMode();
        if(ret) {
            LOGE("%s(): ADSetupMode() Failed : Return Value = %d",__func__, ret);
            goto exit_ad_off;
        } else  {
            LOGD_IF(mDebug, "ADSetup successful");
        }

        ret = mLightSensor->ALSRegister();
        bIsFirstRun = true;
        if( ret ) {
            LOGE("%s(): ALSRegister() Failed : Return Value = %d",__func__, ret);
            goto exit_ad_off;
        } else {
            LOGD_IF(mDebug, "ALSRegister successful");
        }

        pthread_mutex_lock(&mADLock);
        mStatus = AD_ON;
        pthread_mutex_unlock(&mADLock);

        pthread_mutex_lock(&mADLock);
        while(AD_ON == mStatus) {

            if (mADUpdateType != NOTIFY_TYPE_UPDATE) {
                if (!bIsFirstRun) {
                    bIsFirstRun = true;
                    pthread_cond_wait(&mADCond, &mADLock);
                    if (mStatus == AD_OFF) {
                        pthread_mutex_unlock(&mADLock);
                        LOGD_IF(mDebug, "AD worker signalled with status set to OFF");
                        ret = 0;
                        goto exit_cleanup;
                    }
                }
            }
            pthread_mutex_unlock(&mADLock);

            ret = mLightSensor->ALSRun(bIsFirstRun);
            if(ret) {
                LOGE("%s(): ALSRun() Failed : Return Value = %d",__func__, ret);
                goto exit_cleanup;
            } else {
                LOGD_IF(mDebug, "ALSRun successful");
            }

            if (bIsFirstRun) {
                if (mLightSensor->mALSPaused)
                    ADUpdateAL(mLastManualInput, AD_REFRESH_CNT);
                else
                    ADUpdateAL(START_ALS_VALUE, AD_REFRESH_CNT);
            }

            pthread_mutex_lock(&mADLock);
            while(AD_ON == mStatus) {
                if ( mADUpdateType != NOTIFY_TYPE_UPDATE){
                    break;
                }
                pthread_mutex_unlock(&mADLock);
                tmp_ALS = mLightSensor->ALSReadSensor();
                mALSValue = (tmp_ALS > 0) ? tmp_ALS : 1;

                if(ADThresholdCheck(mPrevALSValue, mALSValue) || bIsFirstRun){
                    if (!mLightSensor->mALSPaused) {
                        mPrevALSValue = mALSValue;
                        ADUpdateAL(mALSValue, AD_REFRESH_CNT);
                    }
                }

                pthread_mutex_lock(&mADLock);
                if (bIsFirstRun) {
                    bIsFirstRun = false;
                    break;
                }
            }
            pthread_mutex_unlock(&mADLock);

            ret = mLightSensor->ALSDeRegister();
            if (ret) {
                LOGE("%s(): ALSDeRegister() Failed : Return Value = %d",__func__, ret);
                goto exit_cleanup;
            }

            pthread_mutex_lock(&mADLock);
        }
        pthread_mutex_unlock(&mADLock);

        if (!bIsNativeALS) {
           ret = mLightSensor->ALSCleanup();
           if (ret) {
              LOGE("%s(): ALSCleanup() Failed : Return Value = %d",__func__, ret);
              return;
           }
        }

        ret = ADCleanup();
        if (ret) {
            LOGE("%s(): ADCleanup() Failed : Return Value = %d",__func__, ret);
        }

    }
    return;
exit_dereg:
    mLightSensor->ALSDeRegister();
exit_cleanup:
    if (!bIsNativeALS)
        mLightSensor->ALSCleanup();
exit_ad_off:
    ret = ADCleanup();
    if (ret)
        LOGE("ADCleanup failed, ret %d", ret);
    close(mDisplayFd);
    mDisplayFd = -1;
    pthread_mutex_lock(&mADLock);
    mStatus = AD_OFF;
    pthread_mutex_unlock(&mADLock);
    return;

}

int AD::ADSetupMode() {
    int ret = -1;
    struct msmfb_mdp_pp pp;
    struct fb_var_screeninfo info;
    struct mdss_ad_init *init = &pp.data.ad_init_cfg.params.init;

    LOGD_IF(mDebug, "%s: Entering", __func__);
    if(mADSupported) {
        mADCalibStatus = 0;

        pp.op = mdp_op_ad_cfg;
        pp.data.ad_init_cfg.ops = MDP_PP_OPS_ENABLE | MDP_PP_AD_INIT | mFlags;
        pp.data.ad_init_cfg.params.init = mDefADConfig.init;

        if (ioctl(mDisplayFd, FBIOGET_VSCREENINFO, &info) == -1) {
            LOGE("Error getting FB Info : %s", strerror(errno));
            ret = errno;
            return ret;
        }else if (info.xres > UINT16_MAX || info.yres > UINT16_MAX) {
            LOGE("Unsupported screen resolution (%dx%d)\n",
                                    info.xres, info.yres);
            return -1;
        }
        init->frame_w = (uint16_t) info.xres;
        init->frame_h = (uint16_t) info.yres;

        LOGD_IF(mDebug, "Calling ioctl for AD init!");
        if (ioctl(mDisplayFd, MSMFB_MDP_PP, &pp)) {
            if (!IsPuEnabled())
                LOGE("ad init ioctl failed, err: %s", strerror(errno));
            ret = errno;
            goto exit;
        } else {
            mADCalibStatus |= CALIB_INIT;
            ret = 0;
        }

        pp.op = mdp_op_ad_cfg;
        pp.data.ad_init_cfg.ops = MDP_PP_OPS_ENABLE | MDP_PP_AD_CFG | mFlags;
        pp.data.ad_init_cfg.params.cfg = mDefADConfig.cfg;
        pp.data.ad_init_cfg.params.cfg.mode = mMode;

        LOGD_IF(mDebug, "Calling ioctl for AD config!");

        //set the ad_cfg_calib if assertiveness is valid
        ret = ADCalcCalib();
        if (ret == SUCCESS) {
            pp.data.ad_init_cfg.params.cfg.calib[0] = mAD_calib_a;
            pp.data.ad_init_cfg.params.cfg.calib[2] = mAD_calib_c;
            pp.data.ad_init_cfg.params.cfg.calib[3] = mAD_calib_d;
        }

        if (ioctl(mDisplayFd, MSMFB_MDP_PP, &pp)) {
            if (!IsPuEnabled())
                LOGE("ad config ioctl failed, err: %s", strerror(errno));
            ret = errno;
        } else {
            mADCalibStatus |= CALIB_CFG;
            ret = 0;
        }
        LOGD_IF(mDebug, "%s: Calibration status = %d",__func__, mADCalibStatus);
    }
exit:
    LOGD_IF(mDebug, "%s: Exiting ret = %d", __func__, ret);
    return ret;
}

int AD::ADCleanup() {
    int ret = -1;
    struct msmfb_mdp_pp pp;

    if(mADSupported) {
        pp.op = mdp_op_ad_cfg;
        pp.data.ad_init_cfg.ops = MDP_PP_OPS_DISABLE | MDP_PP_AD_CFG;
        pp.data.ad_init_cfg.params.cfg = mDefADConfig.cfg;
        pp.data.ad_init_cfg.params.cfg.bl_ctrl_mode = MDSS_AD_BL_CTRL_MODE_DIS;

        LOGD_IF(mDebug, "%s:Calling ioctl for AD config!",__func__);
        if (ioctl(mDisplayFd, MSMFB_MDP_PP, &pp)) {
            LOGE("ad config ioctl failed, err: %s", strerror(errno));
            ret = errno;
        } else {
            mADCalibStatus = 0;
            ret = 0;
        }

        LOGD_IF(mDebug, "%s: Cleanup status = %d",__func__, ret);
    }
    return ret;
}

int AD::ADSetCalibMode(int mode) {
    int ret = 0;
    struct msmfb_mdp_pp pp;
    struct mdss_calib_cfg *calib = &pp.data.mdss_calib_cfg;

    if(mADSupported) {
        pp.op = mdp_op_calib_mode;
        calib->ops = MDP_PP_OPS_ENABLE;
        calib->calib_mask = mode;

        LOGD_IF(mDebug, "Calling mdss_calib_cfg ioctl!");
        if (ioctl(mDisplayFd, MSMFB_MDP_PP, &pp)) {
            LOGE("mdss_calib_cfg ioctl failed, err: %s", strerror(errno));
            ret = errno;
        }
    }
    return ret;
}

int AD::ADSetCalibBL(int bl) {
    int ret = 0;
    struct msmfb_mdp_pp pp;
    struct mdss_ad_input *input = &pp.data.ad_input;

    if(mADSupported) {
        pp.op = mdp_op_ad_input;
        input->mode = MDSS_AD_MODE_CALIB;
        input->in.calib_bl = bl;

        LOGD_IF(mDebug, "Calling ad_set_calib_bl ioctl!");
        if (ioctl(mDisplayFd, MSMFB_MDP_PP, &pp)) {
            LOGE("ad_set_calib_bl ioctl failed, err: %s", strerror(errno));
            ret = errno;
        }
    }
    return ret;
}

static int32_t set_backlight_level(uint32_t backlight) {
    int32_t bytes, ret = 0;
    ssize_t ret_val;
    char buffer[MAX_BACKLIGHT_LEN];
    memset(buffer, 0, MAX_BACKLIGHT_LEN);
    bytes = snprintf(buffer, MAX_BACKLIGHT_LEN, "%d\n", backlight);
    if (bytes < 0) {
        LOGE("Failed to write backlight to buffer");
        return ret;
    }
    ret_val = write(BLFd, buffer, bytes);
    lseek(BLFd, 0, SEEK_SET);
    if (ret_val <= 0) {
        LOGE("Failed to write to BLFd.");
        ret = -1;
    }
    return ret;
}

int AD::ADControl(DaemonContext *ctx, int enableBit, ad_mode mode, int display_id) {
    int ret = -1;
    void *term;
    int index = -1;

    if(mADSupported) {
        LOGD_IF(mDebug, "%s: Entering", __func__);

        if (enableBit == CONTROL_PAUSE) {
            ret = mLightSensor->ALSPauseControl(true);
        } else if (enableBit == CONTROL_RESUME) {
            ret = mLightSensor->ALSPauseControl(false);
        } else if (enableBit == CONTROL_ENABLE) {
            if (mStatus == AD_ON ) {
                LOGD_IF(mDebug, "AD is already on !");
                return 0;
            } else if (mStatus == AD_CALIB_ON) {
                LOGD_IF(mDebug, "AD calibration is in progress!");
                return ret;
            }

            if (mEnable) {
                if (mDisplayFd >= 0) {
                    LOGE("Unexpected state, mDisplayFd should have been closed, mDisplayFd = %d", mDisplayFd);
                    return ret;
                }

                ret = ctx->SelectFB(display_id, &index);
                if (ret || index < 0) {
                    LOGE("Cannot identify framebuffer for display = %d, index = %d", display_id, index);
                    return ret;
                } else {
                    char FbPath[MAX_FB_NAME_LEN];
                    snprintf(FbPath, sizeof(FbPath), "%s%d", FRAMEBUFFER_NODE, index);
                    mDisplayFd = open(FbPath, O_RDWR);
                    if (mDisplayFd < 0) {
                        LOGE("Cannot open framebuffer, err: %s", strerror(errno));
                        ret = errno;
                        return ret;
                    }
                }

                if (mRefresher) {
                    LOGD_IF(mDebug, "%s(): Calling mRefresher->control(true) in AD enable",
                            __func__);
                    mRefresher->Control(1);
                }
                switch (mode) {
                    case ad_mode_auto_bl:
                    case ad_mode_auto_str:
                        //Parse calibrationd data file
                        ret = ADParseCalibData(&mDefADConfig);
                        if (ret) {
                            LOGE("%s(): parsing ad calib file failed with ret = %d", __func__, ret);
                            break;
                        }

                        if (mode != ad_mode_auto_bl)
                            mDefADConfig.cfg.bl_ctrl_mode = MDSS_AD_BL_CTRL_MODE_DIS;
                        else {
                            mDefADConfig.cfg.bl_ctrl_mode = MDSS_AD_BL_CTRL_MODE_EN;
                            mPrevBL = get_backlight_level();
                            LOGD_IF(mDebug, "Backlight before entering AD auto bl mode: %u.", mPrevBL);
                        }

                        if (mDebug >= LOG_DEBUG_2)
                            ret = ADPrintCalibData(&mDefADConfig);

                        ret = ADRun(mode);
                        if (ret) {
                            LOGE("%s: ADRun() failed, ret = %d", __func__, ret);
                        }
                        if (mode == ad_mode_auto_str) {
                            mPrevBL = get_backlight_level();
                            if (!mPrevBL) {
                                LOGE("%s(): Failed to get_backlight_level", __func__);
                            } else {
                                LOGD_IF(mDebug, "AD AUTO STR MODE ON: trigger set_backlight to %u.", mPrevBL);
                                if (set_backlight_level(mPrevBL))
                                    LOGE("%s(): Failed to set the backlight", __func__);
                            }
                        }
                        break;
                    case ad_mode_calib:
                        ret = ADSetCalibMode(1);
                        mStatus = AD_CALIB_ON;
                        mADCalibStatus = 0;
                        break;

                    default:
                        break;
                }

                if (ret) {
                    LOGE("%s: Could not enable AD mode = %d, ret = %d, closing display Fd!!", __func__, mode, ret);
                    if (mRefresher) {
                        LOGD_IF(mDebug, "%s(): Disabling ScreenRefresher in AD enable err case",
                                __func__);
                        mRefresher->Control(0);
                    }
                    close(mDisplayFd);
                    mDisplayFd = -1;
                }
            } else {
                LOGD("Assertive display is not enabled!");
            }
        } else if (enableBit == CONTROL_DISABLE) {
            if (mStatus == AD_OFF) {
                LOGD("AD is already off!");
                return 0;
            } else if (mStatus == AD_CALIB_OFF) {
                LOGD_IF(mDebug, "AD calibration is already off!");
                return ret;
            }

            if (mStatus == AD_ON) {
                mStatus = AD_OFF;

                pthread_mutex_lock(&mADLock);
                pthread_cond_signal(&mADCond);
                pthread_mutex_unlock(&mADLock);

                pthread_mutex_lock(&mLightSensor->mALSLock);
                pthread_cond_signal(&mLightSensor->mALSCond);
                pthread_mutex_unlock(&mLightSensor->mALSLock);

                ret = pthread_join(mADThread, &term);
                if (mMode == ad_mode_auto_bl) {
                    /* reset the backlight scale when exiting mode 0*/
                    LOGD_IF(mDebug, "Reset backlight to %u.", mPrevBL);
                    ret = set_backlight_level(mPrevBL);
                    if (ret)
                        LOGE("%s(): Failed to reset the backlight", __func__);
                }
                if (mMode == ad_mode_auto_str) {
                    mPrevBL = get_backlight_level();
                    if (!mPrevBL) {
                        LOGE("%s(): Failed to get_backlight_level", __func__);
                    } else {
                        LOGD_IF(mDebug, "AD AUTO STR MODE OFF: trigger set_backlight to %u.", mPrevBL);
                        if (set_backlight_level(mPrevBL))
                            LOGE("%s(): Failed to set the backlight", __func__);
                    }
                }

            } else if (mStatus == AD_CALIB_ON) {
                ret = ADSetCalibMode(0);
                ret = ADCleanup();
                if( ret ) {
                    LOGE("%s(): Failed : Return Value = %d",__func__, ret);
                    return ret;
                }
                mStatus = AD_CALIB_OFF;
                ret = 0;
            } else {
                LOGE("Unexpected state, mStatus = %d, mDisplayFd = %d", mStatus, mDisplayFd);
            }

            if (mRefresher && (mStatus == AD_CALIB_OFF || mStatus == AD_OFF)) {
                LOGD_IF(mRefresher->mDebug, "Calling mRefresher->control(true)");
                mRefresher->Control(0);
            }

            if (mDisplayFd < 0) {
                LOGE("Unexpected state, invalid mDisplayFd, mDisplayFd = %d", mDisplayFd);
            } else {
                close(mDisplayFd);
            }

            mDisplayFd = -1;

        }
    }
    return ret;
}

int AD::ADRun(ad_mode mode) {
    int ret = -1;

    if(mADSupported) {
        mMode = mode;
        pthread_attr_t attr;
        pthread_attr_init(&attr);

        ret = pthread_create(&mADThread, &attr, &AD::ad_thrd_func, this);
        if (ret) {
            LOGE("AD: Failed to start the ad_thread thread");
        }
        pthread_attr_destroy(&attr);
    }
    return ret;
}

void AD::ADSetLightSensor() {
    if (mADSupported) {
        char property[PROPERTY_VALUE_MAX];
        if (property_get(AD_SENSOR_PROP, property, 0) > 0) {
            int type = atoi(property);
            if (type == 0) {
                mLightSensor  = new ALS();
            } else if (type == 1) {
                mLightSensor = new DummySensor();
            } else if (type == 2) {
                if (getNativeLightSensor(&mLightSensor)) {
                    LOGE("%s: Light Sensor not acquired", __func__);
                    mLightSensor = new ALS();
                } else
                    bIsNativeALS = true;
#ifdef ALS_ENABLE
            } else if (type == 3) {
                mLightSensor  = new LightSensor();
#endif
            } else {
                LOGE("Invalid choice for sensor type, initializing the default sensor class!");
                mLightSensor  = new ALS();
            }
        } else {
#ifdef ALS_ENABLE
            mLightSensor  = new LightSensor();
#else
            if (getNativeLightSensor(&mLightSensor)) {
                LOGE("%s: Light Sensor not acquired", __func__);
                mLightSensor = new ALS();
            } else
                bIsNativeALS = true;
#endif
        }
    }
}

void AD::ADRemoveLightSensor() {
    if (mADSupported) {
        if (isNativeLightSensor()) {
            removeNativeLightSensor(&mLightSensor);
        } else {
            delete mLightSensor;
        }
        mLightSensor = NULL;
    }
}

void AD::ADSetScreenRefresher() {
    if(mEnable == AD_ENABLE_PRI) {
        mRefresher = ScreenRefresher::getInstance();
    }
}

void AD::ADRemoveScreenRefresher() {
    ScreenRefresher::deleteInstance();
    mRefresher = NULL;
}

int AD::ADInit(char* initial) {
    int ret = -1;
    char* params[AD_INIT_PARAM_CNT];
    char* data[AD_MAX_DATA_CNT];
    int i = 0, j = 0, index = 0;
    uint32_t* bl_lm;
    uint32_t* bl_lm_inv;
    uint32_t* bl_att;
    struct fb_var_screeninfo info;

    if(mADSupported) {
        struct msmfb_mdp_pp pp;
        struct mdss_ad_init *init = &pp.data.ad_init_cfg.params.init;

        pp.op = mdp_op_ad_cfg;
        pp.data.ad_init_cfg.ops = MDP_PP_OPS_ENABLE | MDP_PP_AD_INIT;
        init->bl_lin_len = BL_LIN_LUT_SIZE;
        init->bl_att_len = AD_BL_ATT_LUT_LEN;
        init->alpha_base = BL_ATT_ALPHA_BASE;

        bl_lm = (uint32_t*) malloc(BL_LIN_LUT_SIZE * sizeof(uint32_t));
        if (bl_lm == NULL)
            return ret;

        bl_lm_inv = (uint32_t*) malloc(BL_LIN_LUT_SIZE * sizeof(uint32_t));
        if (bl_lm_inv == NULL) {
            free(bl_lm);
            return ret;
        }

        bl_att = (uint32_t*) malloc(AD_BL_ATT_LUT_LEN * sizeof(uint32_t));
        if (bl_att == NULL) {
            free(bl_lm);
            free(bl_lm_inv);
            return ret;
        }

        /* Initialization parsing*/
        memset(params, 0, sizeof(params));
        ret = tokenize_params(initial, AD_PARAM_SEPARATOR, AD_INIT_PARAM_CNT, params, &index);
        if(ret){
            LOGE("tokenize_params failed!");
            goto err;
        }

        if (index != AD_INIT_PARAM_CNT){
            LOGE("invalid number of params reqiuired = %d != given = %d",
                    AD_INIT_PARAM_CNT, index);
            goto err;
        }

        for (i = 0; i < AD_INIT_PARAM_CNT; i++) {
            memset(data, 0, sizeof(data));
            index = 0;
            ret = tokenize_params(params[i], AD_DATA_SEPARATOR, ad_init_data_cnt[i], data, &index);
            if(ret){
                LOGE("tokenize_params failed!");
                goto err;
            }
            if (index != ad_init_data_cnt[i]){
                LOGE("invalid number of params reqiuired = %d != given = %d",
                        ad_init_data_cnt[i], index);
                goto err;
            }

            switch (i) {
                case 0:
                    for (j = 0; j < ad_init_data_cnt[i]; j++)
                        init->asym_lut[j] = atoi(data[j]);
                    break;
                case 1:
                    for (j = 0; j < ad_init_data_cnt[i]; j++)
                        init->color_corr_lut[j] = atoi(data[j]);
                    break;
                case 2:
                    for (j = 0; j < ad_init_data_cnt[i]; j++)
                        init->i_control[j] = (uint8_t) atoi(data[j]);
                    break;
                case 3:
                    init->black_lvl = (uint8_t) atoi(data[0]);
                    break;
                case 4:
                    init->white_lvl = (uint16_t) atoi(data[0]);
                    break;
                case 5:
                    init->var = (uint8_t) atoi(data[0]);
                    break;
                case 6:
                    init->limit_ampl = (uint8_t) atoi(data[0]);
                    break;
                case 7:
                    init->i_dither = (uint8_t) atoi(data[0]);
                    break;
                case 8:
                    init->slope_max = (uint8_t) atoi(data[0]);
                    break;
                case 9:
                    init->slope_min = (uint8_t) atoi(data[0]);
                    break;
                case 10:
                    init->dither_ctl = (uint8_t) atoi(data[0]);
                    break;
                case 11:
                    init->format = (uint8_t) atoi(data[0]);
                    break;
                case 12:
                    init->auto_size = (uint8_t) atoi(data[0]);
                    break;
                case 13:
                    init->frame_w = (uint16_t) atoi(data[0]);
                    break;
                case 14:
                    init->frame_h = (uint16_t) atoi(data[0]);
                    break;
                case 15:
                    init->logo_v = (uint8_t) atoi(data[0]);
                    break;
                case 16:
                    init->logo_h = (uint8_t) atoi(data[0]);
                    break;
                case 17:
                    for (j = 0; j < ad_init_data_cnt[i]; j++)
                        bl_lm[j] = (uint32_t) atoi(data[j]);
                    init->bl_lin = bl_lm;
                    break;
                case 18:
                    for (j = 0; j < ad_init_data_cnt[i]; j++)
                        bl_lm_inv[j] = (uint32_t) atoi(data[j]);
                    init->bl_lin_inv = bl_lm_inv;
                    break;
                case 19:
                    init->alpha = (uint32_t) atoi(data[0]);
                    break;
                case 20:
                    for (j = 0; j < ad_init_data_cnt[i]; j++)
                        bl_att[j] = (uint32_t) atoi(data[j]);
                    init->bl_att_lut = bl_att;
                    break;
                case 21:
                    mAD_ALS_offset = (uint16_t) atoi(data[0]);
                    break;
                case 22:
                    mAD_ALS_thresh = (float) atof(data[0]);
                    break;
                default:
                    break;
            }
        }

        if (ioctl(mDisplayFd, FBIOGET_VSCREENINFO, &info) == -1) {
            LOGE("Error getting FB Info : %s", strerror(errno));
            ret = errno;
            goto err;
        }else if (info.xres > UINT16_MAX || info.yres > UINT16_MAX) {
            LOGE("Unsupported screen resolution (%dx%d)\n",
                                    info.xres, info.yres);
            ret = -1;
            goto err;
        }
        init->frame_w = (uint16_t) info.xres;
        init->frame_h = (uint16_t) info.yres;
        LOGD_IF(mDebug, "Calling ioctl for AD init!");
        if (ioctl(mDisplayFd, MSMFB_MDP_PP, &pp)) {
            LOGE("ad_init ioctl failed, err: %s", strerror(errno));
            ret = errno;
        } else {
            mADCalibStatus |= CALIB_INIT;
        }

err:
        free(bl_lm);
        free(bl_lm_inv);
        free(bl_att);

    }
    return ret;
}

int AD::ADConfig(char* config) {
    int ret = -1;
    char* params[AD_CFG_PARAM_CNT];
    char* data[AD_MAX_DATA_CNT];
    int i = 0, j = 0, index = 0;

    if(mADSupported) {
        struct msmfb_mdp_pp pp;
        struct mdss_ad_cfg *cfg = &pp.data.ad_init_cfg.params.cfg;

        pp.op = mdp_op_ad_cfg;
        pp.data.ad_init_cfg.ops = MDP_PP_AD_CFG;

        /* Calibration parsing*/
        memset(params, 0, sizeof(params));
        ret = tokenize_params(config, AD_PARAM_SEPARATOR, AD_CFG_PARAM_CNT, params, &index);
        if(ret){
            LOGE("tokenize_params failed!");
            goto err;
        }

        if (index != AD_CFG_PARAM_CNT){
            LOGE("invalid number of params reqiuired = %d != given = %d",
                    AD_CFG_PARAM_CNT, index);
            goto err;
        }

        for (i = 0; i < AD_CFG_PARAM_CNT; i++) {
            memset(data, 0, sizeof(data));
            index = 0;
            ret = tokenize_params(params[i], AD_DATA_SEPARATOR, ad_cfg_data_cnt[i], data, &index);
            if(ret){
                LOGE("tokenize_params failed!");
                goto err;
            }
            if (index != ad_cfg_data_cnt[i]){
                LOGE("invalid number of params reqiuired = %d != given = %d",
                        ad_cfg_data_cnt[i], index);
                goto err;
            }

            switch (i) {
                case 0:
                    mMode = atoi(data[0]);
                    cfg->mode = mMode;
                    break;
                case 1:
                    for (j = 0; j < ad_cfg_data_cnt[i]; j++)
                        cfg->al_calib_lut[j] = atoi(data[j]);
                    break;
                case 2:
                    cfg->backlight_min = (uint16_t) atoi(data[0]);
                    break;
                case 3:
                    cfg->backlight_max = (uint16_t) atoi(data[0]);
                    break;
                case 4:
                    cfg->backlight_scale = (uint16_t) atoi(data[0]);
                    break;
                case 5:
                    cfg->amb_light_min = (uint16_t) atoi(data[0]);
                    break;
                case 6:
                    for (j = 0; j < ad_cfg_data_cnt[i]; j++)
                        cfg->filter[j] = (uint16_t) atoi(data[j]);
                    break;
                case 7:
                    for (j = 0; j < ad_cfg_data_cnt[i]; j++)
                        cfg->calib[j] = (uint16_t) atoi(data[j]);
                    break;
                case 8:
                    cfg->strength_limit = (uint8_t) atoi(data[0]);
                    break;
                case 9:
                    cfg->t_filter_recursion = (uint8_t) atoi(data[0]);
                    break;
                case 10:
                    cfg->stab_itr = (uint16_t) atoi(data[0]);
                    break;
                default:
                    break;
            }
            if (cfg->mode != ad_mode_auto_str) {
                ret = -1;
                LOGE_IF(mDebug, "Unsupported AD mode: %d ! ", cfg->mode);
                goto err;
            }
        }

        LOGD_IF(mDebug, "Calling ioctl for AD cfg!");
        if (ioctl(mDisplayFd, MSMFB_MDP_PP, &pp)) {
            LOGE("ad_config ioctl failed, err: %s", strerror(errno));
            ret = errno;
        } else {
            mADCalibStatus |= CALIB_CFG;
        }

    }
err:
    return ret;
}

int AD::ADInput(int aml) {
    int ret = -1;
    struct msmfb_mdp_pp in;
    struct mdss_ad_input *input = &in.data.ad_input;

    if(mADSupported) {
        if(!CALIB_READY(mADCalibStatus)) {
            LOGE("%s:Init or Config not yet done, calib status = %d", __func__, mADCalibStatus);
            return -1;
        }
        /*ambient light tolenrance check */
        if (aml < AMBIENT_LIGHT_MIN || aml > AMBIENT_LIGHT_MAX) {
            LOGE("Invalid ambient light input = %d", aml);
            return -1;
        }

        mALSValue = aml;

        in.op = mdp_op_ad_input;
        in.data.ad_input.mode = mMode;
        in.data.ad_input.in.amb_light = mALSValue;
        ret = ioctl(mDisplayFd, MSMFB_MDP_PP, &in);
        if (ret) {
            if (errno != EHOSTDOWN) {
                LOGE("%s: ad_input ioctl failed, ret = %d, err: %s",__func__, ret, strerror(errno));
                ret = errno;
            } else
                ret = 0;
        } else {
            LOGD_IF(mDebug, "%s: ad_input ioctl successful!!, ALS value = %d",__func__, in.data.ad_input.in.amb_light);
            ret = 0;
        }
    }
    return ret;
}

int AD::ADUpdateAL(int aml, int refresh_cnt) {
    int ret = -1;

    if (mRefresher) {
        LOGD_IF(mRefresher->mDebug, "Calling refresh()");
        mRefresher->Refresh(refresh_cnt, AD_REFRESH_INTERVAL);
    }

    ret = ADInput(aml);
    if (ret) {
        if (ret == ETIMEDOUT)
            LOGE_IF(mDebug, "%s(): ADInput() Failed : ETIMEDOUT", __func__);
        else
            LOGE("%s(): ADInput() Failed : Return Value = %d", __func__, ret);
        if (mRefresher) {
            LOGE_IF(mRefresher->mDebug, "Cancelling refresh");
            mRefresher->Refresh(0, AD_REFRESH_INTERVAL);
        }
    } else {
        LOGD_IF(mDebug, "%s(): ADUpdateAL() successfull, ALS = %d",
                __func__, aml);
        mLastSentALSValue = aml;
    }

    return ret;
}

int AD::ADPrintCalibData(struct ad_default_params* params) {
    int ret = 0;

    if(mADSupported) {
        LOGD_IF(mDebug, "=======================Printing mdss_ad_init====================");
        int i = 0, j = 0;
        for (i = 0; i < AD_INIT_PARAM_CNT; i++) {
            switch (i) {
                case 0:
                    for (j = 0; j < ad_init_data_cnt[i]; j++)
                        LOGD_IF(mDebug, "init.asym_lut[%d]: %d", j, params->init.asym_lut[j]);
                    break;
                case 1:
                    for (j = 0; j < ad_init_data_cnt[i]; j++)
                        LOGD_IF(mDebug, "init.color_corr_lut[%d]: %d", j, params->init.color_corr_lut[j]);
                    break;
                case 2:
                    for (j = 0; j < ad_init_data_cnt[i]; j++)
                        LOGD_IF(mDebug, "init.i_control[%d]: %d", j, params->init.i_control[j]);
                    break;
                case 3:
                    LOGD_IF(mDebug, "init.black_lvl: %d", params->init.black_lvl);
                    break;
                case 4:
                    LOGD_IF(mDebug, "init.white_lvl: %d", params->init.white_lvl);
                    break;
                case 5:
                    LOGD_IF(mDebug, "init.var: %d", params->init.var);
                    break;
                case 6:
                    LOGD_IF(mDebug, "init.limit_ampl: %d", params->init.limit_ampl);
                    break;
                case 7:
                    LOGD_IF(mDebug, "init.i_dither: %d", params->init.i_dither);
                    break;
                case 8:
                    LOGD_IF(mDebug, "init.slope_max: %d", params->init.slope_max);
                    break;
                case 9:
                    LOGD_IF(mDebug, "init.slope_min: %d", params->init.slope_min);
                    break;
                case 10:
                    LOGD_IF(mDebug, "init.dither_ctl: %d", params->init.dither_ctl);
                    break;
                case 11:
                    LOGD_IF(mDebug, "init.format: %d", params->init.format);
                    break;
                case 12:
                    LOGD_IF(mDebug, "init.auto_size: %d", params->init.auto_size);
                    break;
                case 13:
                    LOGD_IF(mDebug, "init.frame_w: %d", params->init.frame_w);
                    break;
                case 14:
                    LOGD_IF(mDebug, "init.frame_h: %d", params->init.frame_h);
                    break;
                case 15:
                    LOGD_IF(mDebug, "init.logo_v: %d", params->init.logo_v);
                    break;
                case 16:
                    LOGD_IF(mDebug, "init.logo_h: %d", params->init.logo_h);
                    break;
                case 17:
                    for (j = 0; j < ad_init_data_cnt[i]; j++)
                        LOGD_IF(mDebug, "init.bl_lin[%d]: %d", j, params->init.bl_lin[j]);
                    break;
                case 18:
                    for (j = 0; j < ad_init_data_cnt[i]; j++)
                        LOGD_IF(mDebug, "init.bl_lin_inv[%d]: %d", j, params->init.bl_lin_inv[j]);
                    break;
                case 19:
                    LOGD_IF(mDebug, "init.alpha: %d", params->init.alpha);
                    break;
                case 20:
                    for (j = 0; j < ad_init_data_cnt[i]; j++)
                        LOGD_IF(mDebug, "init.bl_att_lut[%d]: %d", j, params->init.bl_att_lut[j]);
                    break;
                case 21:
                    LOGD_IF(mDebug, "mAD_ALS_offset: %d", mAD_ALS_offset);
                    break;
                case 22:
                    LOGD_IF(mDebug, "mAD_ALS_thresh : %f", mAD_ALS_thresh);
                    break;

                default:
                    break;
            }
        }

        LOGD_IF(mDebug, "===================Printing mdss_ad_cfg====================");

        for (i = 0; i < AD_CFG_PARAM_CNT; i++) {
            switch (i) {
                case 0:
                    LOGD_IF(mDebug, "cfg.mode: %d", params->cfg.mode);
                    break;
                case 1:
                    for (j = 0; j < ad_cfg_data_cnt[i]; j++)
                        LOGD_IF(mDebug, "cfg.al_calib_lut[%d]: %d", j, params->cfg.al_calib_lut[j]);
                    break;
                case 2:
                    LOGD_IF(mDebug, "cfg.backlight_min: %d", params->cfg.backlight_min);
                    break;
                case 3:
                    LOGD_IF(mDebug, "cfg.backlight_max: %d", params->cfg.backlight_max);
                    break;
                case 4:
                    LOGD_IF(mDebug, "cfg.backlight_scale: %d", params->cfg.backlight_scale);
                    break;
                case 5:
                    LOGD_IF(mDebug, "cfg.amb_light_min: %d", params->cfg.amb_light_min);
                    break;
                case 6:
                    for (j = 0; j < ad_cfg_data_cnt[i]; j++)
                        LOGD_IF(mDebug, "cfg.filter[%d]: %d", j, params->cfg.filter[j]);
                    break;
                case 7:
                    for (j = 0; j < ad_cfg_data_cnt[i]; j++)
                        LOGD_IF(mDebug, "cfg.calib[%d]: %d", j, params->cfg.calib[j]);
                    break;
                case 8:
                    LOGD_IF(mDebug, "cfg.strength_limit: %d", params->cfg.strength_limit);
                    break;
                case 9:
                    LOGD_IF(mDebug, "cfg.t_filter_recursion: %d", params->cfg.t_filter_recursion);
                    break;
                case 10:
                    LOGD_IF(mDebug, "cfg.stab_itr: %d", params->cfg.stab_itr);
                    break;
                default:
                    break;
            }

        }
    }
    return ret;
}

int AD::ADCalcCalib() {
    int ret = FAILED;
    double temp;
    if(mAssertivenessSliderValue < 0 || mAssertivenessSliderValue > 255) {
        LOGE_IF(mDebug, "assertivenessSliderValue is not in valid range");
        return ret;
    }

    double assertiveness =
        pow(2.0, (8.0 * (mAssertivenessSliderValue - 128.0) / 255.0));

    temp = round(mDefADConfig.cfg.calib[0] * assertiveness);
    mAD_calib_a = (uint16_t) ((temp >= UINT16_MAX) ? UINT16_MAX : temp);
    temp = round(mDefADConfig.cfg.calib[2] * assertiveness);
    mAD_calib_c = (uint16_t) ((temp >= UINT16_MAX) ? UINT16_MAX : temp);
    temp = round(mDefADConfig.cfg.calib[3] * assertiveness);
    mAD_calib_d = (uint16_t) ((temp >= UINT16_MAX) ? UINT16_MAX : temp);

    return SUCCESS;
}

int AD::ADParseCalibData(struct ad_default_params* params) {
    int ret = FAILED;
    FILE *fp = NULL;
    char *line = NULL;
    char *temp_token = NULL;
    uint32_t temp;
    int i = 0, j = 0;
    int items;

    if(mADSupported) {
        /* check whehter the input is valid or not */
        if (params == NULL) {
            LOGE("Invalid input!");
        }

        char property[PROPERTY_VALUE_MAX];
        char ad_calib_file[256] = {'\0'};
        if (property_get(AD_CALIB_DATA_PATH_PROP, ad_calib_file, NULL) > 0) {
            if ( -1 == access(&ad_calib_file[0], R_OK|F_OK)) {
                LOGE("%s: No permission to access calibration data file or file does not exists!",__func__);
                return -1;
            }
        }

        LOGD_IF(mDebug, "======================Reading calib_data E=========================");
        /* open file for parsing*/
        fp = fopen(ad_calib_file, "r");
        if (!fp)
            return -1;

        line = (char *)malloc(MAX_CMD_LEN * sizeof(char));
        if (!line) {
            LOGE("Cant allocate memory");
            goto err;
        }

        while(fgets(line, (int)(MAX_CMD_LEN * sizeof(char)), fp)) {
            if (line[0] == '\r' || line[0] == '\n' || line[0] == '#')
                continue;
            if (!strncmp(line, "=version", strlen("=version")))
                goto interp_version;
            if (!strncmp(line, "=init", strlen("=init")))
                goto interp_init;
            if (!strncmp(line, "=config", strlen("=config")))
                goto interp_config;

interp_version:
            while(fgets(line, (int)(MAX_CMD_LEN * sizeof(char)), fp) && i < 1) {
                if (line[0] == '\r' || line[0] == '\n' || line[0] == '#')
                    goto interp_version;
                LOGD_IF(mDebug, "version = %s", line);
                ++i;
            }
            goto exit_version;
interp_init:
            while(fgets(line, (int)(MAX_CMD_LEN * sizeof(char)), fp) &&
                    i < AD_INIT_PARAM_CNT) {
                char *line_r;

                if (line[0] == '\r' || line[0] == '\n' || line[0] == '#')
                    goto interp_init;

                items = ad_init_data_cnt[i];
                temp_token = strtok_r(line, " ", &line_r);
                if (temp_token != NULL) {
                    j = 0;
                    do {
                        LOGD_IF((mDebug >= LOG_DEBUG_2), "%s ", temp_token);
                        switch (i) {
                            case 0:
                                mDefADConfig.init.asym_lut[j] = atoi(temp_token);
                                break;
                            case 1:
                                mDefADConfig.init.color_corr_lut[j] = atoi(temp_token);
                                break;
                            case 2:
                                mDefADConfig.init.i_control[j] = (uint8_t) atoi(temp_token);
                                break;
                            case 3:
                                mDefADConfig.init.black_lvl = (uint8_t) atoi(temp_token);
                                break;
                            case 4:
                                mDefADConfig.init.white_lvl = (uint16_t) atoi(temp_token);
                                break;
                            case 5:
                                mDefADConfig.init.var = (uint8_t) atoi(temp_token);
                                break;
                            case 6:
                                mDefADConfig.init.limit_ampl = (uint8_t) atoi(temp_token);
                                break;
                            case 7:
                                mDefADConfig.init.i_dither = (uint8_t) atoi(temp_token);
                                break;
                            case 8:
                                mDefADConfig.init.slope_max = (uint8_t) atoi(temp_token);
                                break;
                            case 9:
                                mDefADConfig.init.slope_min = (uint8_t) atoi(temp_token);
                                break;
                            case 10:
                                mDefADConfig.init.dither_ctl = (uint8_t) atoi(temp_token);
                                break;
                            case 11:
                                mDefADConfig.init.format = (uint8_t) atoi(temp_token);
                                break;
                            case 12:
                                mDefADConfig.init.auto_size = (uint8_t) atoi(temp_token);
                                break;
                            case 13:
                                mDefADConfig.init.frame_w = (uint16_t) atoi(temp_token);
                                break;
                            case 14:
                                mDefADConfig.init.frame_h = (uint16_t) atoi(temp_token);
                                break;
                            case 15:
                                mDefADConfig.init.logo_v = (uint8_t) atoi(temp_token);
                                break;
                            case 16:
                                mDefADConfig.init.logo_h = (uint8_t) atoi(temp_token);
                                break;
                            case 17:
                                mDefADConfig.init.bl_lin[j] = (uint32_t) atoi(temp_token);
                                break;
                            case 18:
                                mDefADConfig.init.bl_lin_inv[j] = (uint32_t) atoi(temp_token);
                                break;
                            case 19:
                                mDefADConfig.init.alpha = (uint32_t) atoi(temp_token);
                                break;
                            case 20:
                                mDefADConfig.init.bl_att_lut[j] = (uint32_t) atoi(temp_token);
                                break;
                            case 21:
                                mAD_ALS_offset = (uint16_t) atoi(temp_token);
                                break;
                            case 22:
                                mAD_ALS_thresh = (float) atof(temp_token);
                                break;
                            default:
                                break;
                        }
                        j++;
                        temp_token = strtok_r(NULL, " ", &line_r);
                    } while (temp_token != NULL && j < items);
                    if (j != items)
                        LOGE("not enough items (%d/%d) on input line %d", j, items, i);
                }
                i++;
            }
            goto exit_init;
interp_config:
            while(fgets(line, (int) (MAX_CMD_LEN * sizeof(char)), fp) &&
                    i < AD_CFG_PARAM_CNT) {
                char *line_r;
                if (line[0] == '\r' || line[0] == '\n' || line[0] == '#')
                    goto interp_config;

                items = ad_cfg_data_cnt[i];
                temp_token = strtok_r(line, " ", &line_r);
                if (temp_token != NULL) {
                    j = 0;
                    do {
                        temp = atoi(temp_token);
                        LOGD_IF((mDebug >= LOG_DEBUG_2), "%d ", temp);
                        switch (i) {
                            case 0:
                                mDefADConfig.cfg.mode = temp;
                                break;
                            case 1:
                                mDefADConfig.cfg.al_calib_lut[j] = temp;
                                break;
                            case 2:
                                mDefADConfig.cfg.backlight_min = (uint16_t) temp;
                                break;
                            case 3:
                                mDefADConfig.cfg.backlight_max = (uint16_t) temp;
                                break;
                            case 4:
                                mDefADConfig.cfg.backlight_scale = (uint16_t) temp;
                                break;
                            case 5:
                                mDefADConfig.cfg.amb_light_min = (uint16_t) temp;
                                break;
                            case 6:
                                mDefADConfig.cfg.filter[j] = (uint16_t) temp;
                                break;
                            case 7:
                                mDefADConfig.cfg.calib[j] = (uint16_t) temp;
                                break;
                            case 8:
                                mDefADConfig.cfg.strength_limit = (uint8_t) temp;
                                break;
                            case 9:
                                mDefADConfig.cfg.t_filter_recursion = (uint8_t) temp;
                                break;
                            case 10:
                                mDefADConfig.cfg.stab_itr = (uint16_t) temp;
                                break;
                            default:
                                break;
                        }
                        j++;
                        temp_token = strtok_r(NULL, " ", &line_r);
                    } while (temp_token != NULL && j < items);
                    if (j != items)
                        LOGE("not enough items (%d/%d) on input line %d", j, items, i);
                }
                i++;
            }
            goto exit_config;
exit_version:
            LOGD_IF(mDebug, "Finish parsing version session");
            i = 0;
            continue;
exit_init:
            LOGD_IF(mDebug, "Finish parsing init session");
            i = 0;
            continue;
exit_config:
            LOGD_IF(mDebug, "Finish parsing cfg session");
        }


        LOGD_IF(mDebug, "=================Reading calib_data X ================");

        free(line);
err:
        ret = fclose(fp);
    }
    return ret;
}
