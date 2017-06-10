/*
 * DESCRIPTION
 * This file runs the daemon for postprocessing features.
 * It listens to the socket for client connections and controls the features
 * based on commands it received from the clients.
 *
 * Copyright (c) 2012-2015 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 */

#include "DaemonContext.h"
#include <time.h>
#include <mode_manager.h>
#include <utility.h>
#include <dlfcn.h>
#include "ScreenRefresher.h"

int32_t DaemonContext::reply(bool result, const int32_t& fd) {
    int32_t ret = 1;
    ssize_t ret_val;
    char buf[32];
    memset(buf, 0x00, sizeof(buf));

    if (fd < 0)
        return 0;

    if (result)
        snprintf(buf, sizeof(buf), "Success\n");
    else
        snprintf(buf, sizeof(buf), "Failure\n");
    ret_val = write(fd, buf, strlen(buf));
    if(ret_val == -1) {
        LOGE("Failed to reply back: %s", strerror(errno));
        ret = -1;
    }
    return ret;
}

int32_t DaemonContext::reply(const char *reply, const int32_t& fd) {
    int32_t ret = 1;
    ssize_t ret_val;
    if (NULL == reply) {
        LOGE("Reply string is NULL!");
        return -1;
    }

    if (fd < 0)
        return 0;

    ret_val = write(fd, reply, strlen(reply));
    if(ret_val == -1) {
        LOGE("Failed to reply back: %s", strerror(errno));
        ret = -1;
    }
    return ret;
}

int DaemonContext::ProcessCABLMsg(char* buf, const int32_t& fd) {
    int32_t result = 1;

    LOGD_IF(mDebug, "PROCESS_CABL_MSG(): Command : %s", buf);
    if(!strncmp(buf, CMD_CABL_ON, strlen(CMD_CABL_ON))) {
        pthread_mutex_lock(&mCABLOpLock);
        if(bAbaEnabled){
            mABA->SetCABLFeature();
            result = mABA->CABLControl(true);
            pthread_mutex_lock(&mCtrlLock);
            if (!result)
                mCtrlStatus |= aba_cabl_bit;
            pthread_mutex_unlock(&mCtrlLock);
        } else {
            result = mCABL->start_cabl();
            pthread_mutex_lock(&mCtrlLock);
            if (!result)
                mCtrlStatus |= cabl_bit;
            pthread_mutex_unlock(&mCtrlLock);
        }
        pthread_mutex_unlock(&mCABLOpLock);
        reply(!result, fd);
    } else if (!strncmp(buf, CMD_CABL_OFF, strlen(CMD_CABL_OFF))) {
        pthread_mutex_lock(&mCABLOpLock);
        if(bAbaEnabled){
            result = mABA->CABLControl(false);
        } else {
            mCABL->stop_cabl();
            result = 0;
            //Removing CABL bits from control status
            pthread_mutex_lock(&mCtrlLock);
            mCtrlStatus &= ~cabl_bit;
            pthread_mutex_unlock(&mCtrlLock);
        }
        pthread_mutex_unlock(&mCABLOpLock);
        reply(!result, fd);
    } else if (!strncmp(buf, CMD_CABL_SET, strlen(CMD_CABL_SET))) {
        char *c_lvl = NULL, *tmp = NULL, *bufr;
        tmp = strtok_r(buf, " ", &bufr);
        while (tmp != NULL) {
            c_lvl = tmp;
            tmp = strtok_r(NULL, " ", &bufr);
        }
        if (c_lvl == NULL) {
            LOGE("Invalid quality level!");
            reply(false, fd);
            return -1;
        }
        if (!strcmp(c_lvl, CABL_LVL_AUTO)) {
            pthread_mutex_lock(&mCABLOpLock);
            if(bAbaEnabled){
                mABA->SetQualityLevel(ABL_QUALITY_HIGH);
                result = AbaSetQualityLevel(mABA->GetHandle(),
                (AbaQualityLevelType) ABL_QUALITY_HIGH);
            } else {
                pthread_mutex_lock(&mCABL->mCABLLock);
                mCABL->mPowerSaveLevel = ABL_QUALITY_HIGH;
                mCABL->mPowerLevelChange = true;
                result = 0;
                pthread_mutex_unlock(&mCABL->mCABLLock);
            }
            if (!result) {
                property_set("hw.cabl.level", c_lvl);
                if(bAbaEnabled){
                    mABA->SetDefaultQualityMode(AUTO_QL_MODE);
                } else {
                    pthread_mutex_lock(&mCABL->mCABLLock);
                    mCABL->mOEMParams.default_ql_mode = AUTO_QL_MODE;
                    pthread_mutex_unlock(&mCABL->mCABLLock);
                }
            }
            pthread_mutex_unlock(&mCABLOpLock);
        } else {
            if(bAbaEnabled){
                CABLQualityLevelType i_lvl = (CABLQualityLevelType) ql_string2int(c_lvl);
                if((int)i_lvl < 0){
                    LOGE("Invalid quality level!");
                    result = -1;
                } else {
                    if (i_lvl == mABA->GetQualityLevel()) {
                        LOGD_IF(mDebug, "ABA CABL Power level has not changed!");
                        pthread_mutex_lock(&mCABLOpLock);
                        mABA->SetDefaultQualityMode(USER_QL_MODE);
                        pthread_mutex_unlock(&mCABLOpLock);
                        result = 0;
                    } else {
                        pthread_mutex_lock(&mCABLOpLock);
                        result = AbaSetQualityLevel(mABA->GetHandle(), (AbaQualityLevelType) i_lvl);
                        if (!result) {
                            property_set("hw.cabl.level", c_lvl);
                            mABA->SetQualityLevel((CABLQualityLevelType) i_lvl);
                            mABA->SetDefaultQualityMode(USER_QL_MODE);
                        }
                        pthread_mutex_unlock(&mCABLOpLock);
                    }
                }
            } else {
                int i_lvl = ql_string2int(c_lvl);
                if(i_lvl < 0){
                    LOGE("Invalid quality level!");
                    result = -1;
                } else {
                    pthread_mutex_lock(&mCABLOpLock);
                    pthread_mutex_lock(&mCABL->mCABLLock);
                    if (i_lvl == mCABL->mPowerSaveLevel) {
                        LOGD_IF(mDebug, "CABL Power level has not changed!");
                        mCABL->mOEMParams.default_ql_mode = USER_QL_MODE;
                        result = 0;
                    } else {
                        property_set("hw.cabl.level", c_lvl);
                        mCABL->mPowerSaveLevel = i_lvl;
                        mCABL->mOEMParams.default_ql_mode = USER_QL_MODE;
                        mCABL->mPowerLevelChange = true;
                        result = 0;
                    }
                    pthread_mutex_unlock(&mCABL->mCABLLock);
                    pthread_mutex_unlock(&mCABLOpLock);
                }
            }
        }
        reply(!result, fd);
    } else if (!strncmp(buf, CMD_CABL_SUPPORT, strlen(CMD_CABL_SUPPORT))) {
        if (display_pp_cabl_supported() > 0)
            result = 0;
        reply(!result, fd);
    } else if (!strncmp(buf, CMD_CABL_STATUS, strlen(CMD_CABL_STATUS))) {
        char status[32];
        memset(status, 0x00, sizeof(status));
        if (bAbaEnabled){
            if (mABA) {
                pthread_mutex_lock(&mABA->mAbaCtrlLock);
                if (mABA->IsABAStatusON(ABA_FEATURE_CABL))
                    snprintf(status, sizeof(status), "running\n");
                else
                    snprintf(status, sizeof(status), "stopped\n");
                pthread_mutex_unlock(&mABA->mAbaCtrlLock);
            } else
                snprintf(status, sizeof(status), "stopped\n");
        } else {
            if (mCABL && mCABL->eStatus == CABL_ON)
                snprintf(status, sizeof(status), "running\n");
            else
                snprintf(status, sizeof(status), "stopped\n");
        }
        reply(status, fd);
        result = 0;
    } else if (!strncmp(buf, CMD_CABL_GET, strlen(CMD_CABL_GET))) {
        char level[20];
        int lvl = 0;
        memset(level, 0, sizeof(level));

        pthread_mutex_lock(&mCABLOpLock);
        if ((bAbaEnabled) && (mABA)) {
            lvl = mABA->GetQualityLevel();
        } else if (mCABL) {
            pthread_mutex_lock(&mCABL->mCABLLock);
            lvl = mCABL->mPowerSaveLevel;
            pthread_mutex_unlock(&mCABL->mCABLLock);
        }
        pthread_mutex_unlock(&mCABLOpLock);

        if (lvl == ABL_QUALITY_LOW)
            snprintf(level, sizeof(level), "%s", CABL_LVL_LOW);
        else if (lvl == ABL_QUALITY_NORMAL)
            snprintf(level, sizeof(level), "%s", CABL_LVL_MEDIUM);
        else if (lvl == ABL_QUALITY_HIGH)
            snprintf(level, sizeof(level), "%s", CABL_LVL_HIGH);
        else if (lvl == ABL_QUALITY_AUTO)
            snprintf(level, sizeof(level), "%s", CABL_LVL_AUTO);
        else
            snprintf(level, sizeof(level), "%s", "failed");

        reply(level, fd);
    } else if (!strncmp(buf, CMD_CABL_SCALE, strlen(CMD_CABL_SCALE))) {
        char str[20];
        int scale = 0;
        memset(str, 0, sizeof(str));

        if (bAbaEnabled && mABA) {
            scale = (int)mABA->GetScale();
        } else if (mCABL) {
            scale = (int)mCABL->get_scale();
        }

        if ((scale > 1024) || (scale < 0))
            snprintf(str, sizeof(str), "%s", "failed");
        else
            snprintf(str, sizeof(str), "scale:%d:", scale);
        reply(str, fd);
    }
    return result;
}

int DaemonContext::ProcessPPMsg(const char* buf, const int32_t& fd) {
    int32_t result = 1;

    LOGD_IF(mDebug, "PROCESS_PP_MSG(): Command :%s", buf);

    if (!strncmp(buf, CMD_PP_ON, strlen(CMD_PP_ON))) {
        pthread_mutex_lock(&mPostProcOpLock);
        result = mPostProc.start_pp();
        pthread_mutex_unlock(&mPostProcOpLock);
        reply(!result, fd);
    } else if (!strncmp(buf, CMD_PP_OFF, strlen(CMD_PP_OFF))) {
        pthread_mutex_lock(&mPostProcOpLock);
        mPostProc.stop_pp();
        pthread_mutex_unlock(&mPostProcOpLock);
        reply(true, fd);
    } else if (!strncmp(buf, CMD_PP_SET_HSIC, strlen(CMD_PP_SET_HSIC))) {
        int32_t intensity;
        float hue, sat, contrast;
        /* start the postproc module */
        sscanf(buf, CMD_PP_SET_HSIC "%f %f %d %f", &hue,
                &sat, &intensity, &contrast);
        pthread_mutex_lock(&mPostProcOpLock);
        mPostProc.start_pp();
        result = mPostProc.set_hsic(hue, sat, intensity, contrast);
        if (result){
            LOGE("Failed to set PA data");
            pthread_mutex_unlock(&mPostProcOpLock);
            reply(!result, fd);
            return result;
        }

        result = mPostProc.save_pa_data(hue, sat, intensity, contrast);
        if (result){
            LOGE("Failed to save PA data");
        }

        pthread_mutex_unlock(&mPostProcOpLock);
        reply(!result, fd);
    } else if (!strncmp(buf, CMD_POSTPROC_STATUS, strlen(CMD_POSTPROC_STATUS))) {
        char buf[32];
        memset(buf, 0x00, sizeof(buf));
        if (mPostProc.mStatus == PP_ON)
            snprintf(buf, sizeof(buf), "Running\n");
        else
            snprintf(buf, sizeof(buf), "Stopped\n");
        reply(buf, fd);
    } else if (!strncmp(buf, CMD_PP_MEM_SUPPORT, strlen(CMD_PP_MEM_SUPPORT))) {
        int disp_id;
        result = sscanf(buf, CMD_PP_MEM_SUPPORT ":%d", &disp_id);
        if (result != 1)
            result = 1;
        else if (display_pp_pa_v2_supported() > 0)
            result = 0;
        reply(!result, fd);
    } else
        LOGE("%s: Unsupported command", __func__);

    return result;
}

int DaemonContext::ProcessADMsg(const char* buf, const int32_t& fd){
    int32_t result = 0;

    LOGD_IF(mDebug, "PROCESS_AD_MSG(): Command : %s", buf);

    if (!strncmp(buf, CMD_AD_CALIB_ON, strlen(CMD_AD_CALIB_ON))) {
        pthread_mutex_lock(&mADOpLock);
        if (mAD.ADStatus() == AD_ON) {
            /* stop ad first before turning on ad_calib mode */
            result = mAD.ADControl(this, false);
            if (result) {
                pthread_mutex_unlock(&mADOpLock);
                reply(!result, fd);
                return result;
            }
        }
        result = mAD.ADControl(this, true, ad_mode_calib, MDP_LOGICAL_BLOCK_DISP_0);
        pthread_mutex_unlock(&mADOpLock);
        reply(!result, fd);
    } else if (!strncmp(buf, CMD_AD_CALIB_OFF, strlen(CMD_AD_CALIB_OFF))) {
        pthread_mutex_lock(&mADOpLock);
        if ( mAD.ADStatus() == AD_CALIB_ON ) {
            result = mAD.ADControl(this, false);
        }
        pthread_mutex_unlock(&mADOpLock);
        reply(!result, fd);
    } else if (!strncmp(buf, CMD_AD_ON, strlen(CMD_AD_ON))) {
        int int_mode;
        ad_mode mode;
        int display_id = MDP_LOGICAL_BLOCK_DISP_0;
        uint32_t flag = 0;
        char c_mode[32];
        sscanf(buf, CMD_AD_ON ";" "%d" ";" "%d" ";" "%u", &int_mode, &display_id, &flag);
        if (int_mode > ad_mode_calib || int_mode < ad_mode_auto_bl) {
            LOGE("Invalid input, unsupported mode %d\n", int_mode);
            result = -1;
            reply(!result, fd);
            return result;
        } else {
            mode = (ad_mode) int_mode;
        }
        if(!mSplitDisplay && flag != 0) {
           LOGE("Invalid input, target doesn't support AD split mode");
           result = -1;
           reply(!result, fd);
           return result;
        }
        pthread_mutex_lock(&mADOpLock);
        if (mAD.IsADInputValid(mode, display_id)) {
            if (mAD.ADStatus() == AD_CALIB_ON) {
            /* stop ad_calib first before turning on ad mode */
                result = mAD.ADControl(this, false);
                if (result) {
                    pthread_mutex_unlock(&mADOpLock);
                    reply(!result, fd);
                    return result;
                }
            }
            if (mAD.ADStatus() == AD_ON && (mode != mAD.ADMode() || mAD.mFlags != flag)) {
                result = mAD.ADControl(this, false);
                if (result) {
                    pthread_mutex_unlock(&mADOpLock);
                    reply(!result, fd);
                    return result;
                }
            }
            mAD.mFlags = (MDSS_PP_SPLIT_MASK & flag);
            result = mAD.ADControl(this, true, mode, display_id);
            pthread_mutex_lock(&mCtrlLock);
            if (!result) {
                snprintf(c_mode, (size_t) sizeof(c_mode), "%d", mode);
                property_set("hw.ad.mode",c_mode);
                mCtrlStatus |= ad_bit;
            }
            pthread_mutex_unlock(&mCtrlLock);
        } else
            result = -1;
        pthread_mutex_unlock(&mADOpLock);
        reply(!result, fd);
    } else if (!strncmp(buf, CMD_AD_OFF, strlen(CMD_AD_OFF))) {
        pthread_mutex_lock(&mADOpLock);
        if (mAD.ADStatus() == AD_ON) {
            result = mAD.ADControl(this, false);
            pthread_mutex_lock(&mCtrlLock);
            if (!result)
                mCtrlStatus &= ~ad_bit;
            pthread_mutex_unlock(&mCtrlLock);
        } else {
            LOGD_IF(mDebug, "AD is already off!");
        }
        pthread_mutex_unlock(&mADOpLock);
        reply(!result, fd);
    } else if (!strncmp(buf, CMD_AD_INIT, strlen(CMD_AD_INIT))) {
        char *params = NULL;
        params = strchr(buf, ';');
        if (params == NULL) {
            LOGE("Invalid format for input command");
            result = -1;
        } else {
            params = params + 1;
            if (mAD.ADStatus() == AD_CALIB_ON) {
                pthread_mutex_lock(&mADOpLock);
                result = mAD.ADInit(params);

                if (mAD.mRefresher) {
                    LOGD_IF(mAD.mRefresher->mDebug, "Calling refresh()");
                    mAD.mRefresher->Refresh(1, AD_REFRESH_INTERVAL);
                }

                if (mAD.ADLastSentALSValue() >= 0)
                    mAD.ADUpdateAL(mAD.ADLastSentALSValue(), AD_REFRESH_CNT);
                pthread_mutex_unlock(&mADOpLock);
            } else {
                LOGE("AD calibration mode is not ON!!");
                result = -1;
            }
        }
        reply(!result, fd);
    } else if (!strncmp(buf, CMD_AD_CFG, strlen(CMD_AD_CFG))) {
        char *params = NULL;
        params = strchr(buf, ';');
        if (params == NULL) {
            LOGE("Invalid format for input command");
            result = -1;
        } else {
            params = params + 1;
            if (mAD.ADStatus() == AD_CALIB_ON) {
                pthread_mutex_lock(&mADOpLock);
                result = mAD.ADConfig(params);

                if (mAD.mRefresher) {
                    LOGD_IF(mAD.mRefresher->mDebug, "Calling refresh()");
                    mAD.mRefresher->Refresh(1, AD_REFRESH_INTERVAL);
                }

                if (mAD.ADLastSentALSValue() >= 0)
                    mAD.ADUpdateAL(mAD.ADLastSentALSValue(), AD_REFRESH_CNT);
                pthread_mutex_unlock(&mADOpLock);
            } else {
                LOGE("AD calibration mode is not ON!!");
                result = -1;
            }
        }
        reply(!result, fd);
    } else if (!strncmp(buf, CMD_AD_INPUT, strlen(CMD_AD_INPUT))) {
        int32_t lux_value;
        uint32_t enableManualInput = 0;
        sscanf(buf, CMD_AD_INPUT ";" "%d" ";" "%u", &lux_value, &enableManualInput);
        if ((mAD.ADStatus() == AD_CALIB_ON) || (mAD.ADStatus() == AD_ON)) {
            pthread_mutex_lock(&mADOpLock);
            if (enableManualInput == 1) {
                mAD.ADControl(this, CONTROL_PAUSE);
                mAD.mLastManualInput = lux_value;
            } else {
                mAD.ADControl(this, CONTROL_RESUME);
            }

            result = mAD.ADUpdateAL(lux_value, AD_REFRESH_CNT);

            pthread_mutex_unlock(&mADOpLock);
        } else {
            LOGE("AD is not ON!!");
            result = -1;
        }
        reply(!result, fd);
    } else if (!strncmp(buf, CMD_AD_ASSERTIVENESS, strlen(CMD_AD_ASSERTIVENESS)) ||
            !strncmp(buf, CMD_AD_STRLIMIT, strlen(CMD_AD_STRLIMIT))) {
        pthread_mutex_lock(&mADOpLock);
        if (mAD.ADStatus() == AD_ON) {
            int32_t value;
            if (!strncmp(buf, CMD_AD_ASSERTIVENESS, strlen(CMD_AD_ASSERTIVENESS))) {
                sscanf(buf, CMD_AD_ASSERTIVENESS";" "%d", &value);
                if (mAD.mAssertivenessSliderValue == value) {
                    LOGD_IF(mDebug, "Input assertiveness is the same as current one!");
                    pthread_mutex_unlock(&mADOpLock);
                    reply(!result, fd);
                    return result;
                }
                mAD.mAssertivenessSliderValue = value;
            } else if (!strncmp(buf, CMD_AD_STRLIMIT, strlen(CMD_AD_STRLIMIT))) {
                sscanf(buf, CMD_AD_STRLIMIT";" "%d", &value);
                if ((value >= AD_STRLIMT_MIN) && (value <= AD_STRLIMT_MAX)) {
                    if (mAD.ADGetStrLimit() == value) {
                        LOGD_IF(mDebug, "Strength limit is the same as current one!");
                        pthread_mutex_unlock(&mADOpLock);
                        reply(!result, fd);
                        return result;
                    }
                    mAD.ADSetStrLimit((uint8_t) value);
                } else {
                    LOGE("AD strength limit out of range!");
                    pthread_mutex_unlock(&mADOpLock);
                    result = -1;
                    reply(!result, fd);
                    return result;
                }
            }

            result = mAD.ADSetupMode();
            if (result)
                LOGE("Failed to set the assertiveness!");

            if (mAD.mRefresher) {
                LOGD_IF(mAD.mRefresher->mDebug, "Calling refresh()");
                mAD.mRefresher->Refresh(1, AD_REFRESH_INTERVAL);
            }

            result = mAD.ADUpdateAL(((mAD.ADLastSentALSValue() > 0) ? mAD.ADLastSentALSValue() : START_ALS_VALUE), AD_REFRESH_CNT);

        } else {
            LOGE("AD is not ON, cannot set assertiveness or strength limit");
            result = -1;
        }
        pthread_mutex_unlock(&mADOpLock);
        reply(!result, fd);
    } else if (!strncmp(buf, CMD_AD_SUPPORT, strlen(CMD_AD_SUPPORT))) {
        if (display_pp_ad_supported() > 0)
            result = 0;
        else
            result = -1;
        reply(!result, fd);
    } else if (!strncmp(buf, CMD_AD_STATUS, strlen(CMD_AD_STATUS))) {
        char ad_status[32];
        memset(ad_status, 0x00, sizeof(ad_status));
        int def_status = 0, def_mode = -1;
        pthread_mutex_lock(&mADOpLock);
        def_status = mAD.ADStatus();
        if (def_status == AD_ON || def_status == AD_CALIB_ON)
            def_mode = mAD.ADMode();
        pthread_mutex_unlock(&mADOpLock);
        snprintf(ad_status, sizeof(ad_status), "%d;%d", def_status, def_mode);
        reply(ad_status, fd);
        result = 0;
    } else if (!strncmp(buf, CMD_AD_RANGE, strlen(CMD_AD_RANGE))) {
        char range[32];
        memset(range, 0, sizeof(range));
        snprintf(range, sizeof(range), "range:%d:%d", AD_ASSERT_SLIDER_MAX,
                                                        AD_ASSERT_SLIDER_MIN);
        reply(range, fd);
    } else if (!strncmp(buf, CMD_AD_GET, strlen(CMD_AD_GET))) {
        int val = -1;
        char response[20];
        memset(response, 0, sizeof(response));
        if(mAD.ADStatus() == AD_ON) {
            pthread_mutex_lock(&mADOpLock);
            val = mAD.mAssertivenessSliderValue;
            pthread_mutex_unlock(&mADOpLock);
            snprintf(response, sizeof(response), "strength:%d:", val);
        } else
            snprintf(response, sizeof(response), "failed");
        reply(response, fd);
    } else {
        LOGE("Unsupported AD message.");
        result = -1;
    }

    return result;
}

int DaemonContext::ProcessSVIMsg(const char* buf, const int32_t& fd){
    int32_t result = 1;

    LOGD_IF(mDebug,"PROCESS_SVI_MSG(): Command : %s", buf);
    if (!strncmp(buf, CMD_SVI_ON, strlen(CMD_SVI_ON))) {
        LOGD_IF(mDebug, "%s:  Command received %s", __FUNCTION__, buf);
        mABA->SetSVIFeature();

        pthread_mutex_lock(&mSVIOpLock);
        result = mABA->SVIControl(true);
        pthread_mutex_lock(&mCtrlLock);

        if (!result) {
            mCtrlStatus |= aba_svi_bit;
        }

        pthread_mutex_unlock(&mCtrlLock);
        pthread_mutex_unlock(&mSVIOpLock);
        reply(!result, fd);
    } else if (!strncmp(buf, CMD_SVI_OFF, strlen(CMD_SVI_OFF))) {
        pthread_mutex_lock(&mSVIOpLock);
        LOGD_IF(mDebug, "%s:  Command received %s", __FUNCTION__, buf);
        result = mABA->SVIControl(false);
        pthread_mutex_unlock(&mSVIOpLock);
        reply(!result, fd);
    } else if (!strncmp(buf, CMD_SVI_SUPPORT, strlen(CMD_SVI_SUPPORT))) {
        if (display_pp_svi_supported() > 0)
            result = 0;
        reply(!result, fd);
    } else if (!strncmp(buf, CMD_SVI_SET, strlen(CMD_SVI_SET))) {
        int strength;
        AbaStatusType retval;
        pthread_mutex_lock(&mABA->mAbaCtrlLock);
        if (!mABA->IsABAStatusON(ABA_FEATURE_SVI)) {
            pthread_mutex_unlock(&mABA->mAbaCtrlLock);
            return 1;
        }
        pthread_mutex_unlock(&mABA->mAbaCtrlLock);
        sscanf(buf, CMD_SVI_SET ":" "%d", &strength);
        if ((strength < MIN_SVI_STRENGTH) || (strength > MAX_SVI_STRENGTH))
            result = 1;
        else {
            result = 0;
            SVIDynamicConfigType dynamicConfig;
            memset(&dynamicConfig, 0x0, sizeof(dynamicConfig));
            mABA->GetSVIDynConfigOemParams(dynamicConfig);
            dynamicConfig.uBrightnessStrengthFactor = strength;
            retval = AbaDynamicConfigUpdate(mABA->GetHandle(), &dynamicConfig,
                sizeof(dynamicConfig), ABA_FEATURE_SVI);
            if (retval != ABA_STATUS_SUCCESS)
                result = 1;
            else
                mABA->SetSVIDynConfigOemParams(dynamicConfig);
        }
        reply(!result, fd);
    } else if (!strncmp(buf, CMD_SVI_STATUS, strlen(CMD_SVI_STATUS))) {
        char status[32];
        memset(status, 0x00, sizeof(status));
        pthread_mutex_lock(&mABA->mAbaCtrlLock);
        if (mABA->IsABAStatusON(ABA_FEATURE_SVI))
            snprintf(status, sizeof(status), "running\n");
        else
            snprintf(status, sizeof(status), "stopped\n");
        pthread_mutex_unlock(&mABA->mAbaCtrlLock);
        reply(status, fd);
        result = 0;
    } else if (!strncmp(buf, CMD_SVI_RANGE, strlen(CMD_SVI_RANGE))) {
        char range[32];
        memset(range, 0, sizeof(range));
        snprintf(range, sizeof(range), "range:%d:%d:", MAX_SVI_STRENGTH,
                                                        MIN_SVI_STRENGTH);
        reply(range, fd);
    } else if (!strncmp(buf, CMD_SVI_GET, strlen(CMD_SVI_GET))) {
        int strength = 0;
        char str[20];
        memset(str, 0, sizeof(str));
        pthread_mutex_lock(&mABA->mAbaCtrlLock);
        if (mABA->IsABAStatusON(ABA_FEATURE_SVI)) {
            strength = (int)mABA->GetSVIStrength();
            snprintf(str, sizeof(str), "strength:%d:", strength);
        } else
            snprintf(str, sizeof(str), "failed");
        pthread_mutex_unlock(&mABA->mAbaCtrlLock);
        reply(str, fd);
    }
    return result;
}

void DaemonContext::ProcessControlWork() {
    LOGD_IF(mDebug, "Starting control loop");
    int ret = 0, tmp_ctrl_status = 0;
    int screenOn = screenStatus;
    uint32_t update_notify = NOTIFY_UPDATE_INIT;

    /* Call INIT before other NOTIFY UPDATE ioctl calls */
    ret = ioctl(FBFd, MSMFB_NOTIFY_UPDATE, &update_notify);
    if (ret) {
        LOGE("%s: Notify INIT call failed! Control thread exiting", __func__);
        return;
    }

    pthread_mutex_lock(&mCtrlLock);
    /* assume we are holding lock when loop evaluates */
    while ((mCtrlStatus & cabl_bit) || (mCtrlStatus & aba_bit) ||
        (mCtrlStatus & ad_bit)) {
        if (mCtrlStatus != tmp_ctrl_status) {
            if (tmp_ctrl_status & cabl_bit && !(mCtrlStatus & cabl_bit)){
                mCABL->Notify(NOTIFY_TYPE_SUSPEND);
            }
            else if (!(tmp_ctrl_status & cabl_bit) && (mCtrlStatus & cabl_bit)
                                                        && screenStatus == 1) {
                mCABL->Notify(NOTIFY_TYPE_UPDATE);
            }

            if (tmp_ctrl_status & aba_bit && !(mCtrlStatus & aba_bit)){
                if(bAbaEnabled && mABA)
                    mABA->Notify(NOTIFY_TYPE_SUSPEND);
            }
            else if (!(tmp_ctrl_status & aba_bit) && (mCtrlStatus & aba_bit)
                                                        && screenStatus == 1) {
                if (bAbaEnabled && mABA)
                    mABA->Notify(NOTIFY_TYPE_UPDATE);
            }
            if (tmp_ctrl_status & ad_bit && !(mCtrlStatus & ad_bit))
                mAD.Notify(NOTIFY_TYPE_SUSPEND);
            else if (!(tmp_ctrl_status & ad_bit) && (mCtrlStatus & ad_bit)
                                                        && screenOn == 1) {
                mAD.Notify(NOTIFY_TYPE_UPDATE);
            }
            tmp_ctrl_status = mCtrlStatus;
        }

        //Updating mCtrlStatus based on ABA status
        if ((bAbaEnabled) && (mABA)) {
            pthread_mutex_lock(&mABA->mAbaCtrlLock);
            if (!mABA->IsABAStatusON(ABA_FEATURE_CABL))
                mCtrlStatus &= ~aba_cabl_bit;
            if (!mABA->IsABAStatusON(ABA_FEATURE_SVI))
                mCtrlStatus &= ~aba_svi_bit;
            pthread_mutex_unlock(&mABA->mAbaCtrlLock);
        }

        pthread_mutex_unlock(&mCtrlLock);
        if (screenStatus == 0) {
            update_notify = NOTIFY_UPDATE_START;
            ret =  ioctl(FBFd, MSMFB_NOTIFY_UPDATE, &update_notify);
            if (ret != 0) {
                if (errno != ETIMEDOUT)
                    LOGE("MSMFB_NOTIFY_UPDATE start ioctl failed");
                pthread_mutex_lock(&mCtrlLock);
                continue;
            }
            /*
             * Notify screen refresher first in case of update, as other
             * features would be calling refresh and it might reject if not
             * notified of update apriori
             */
            if ((update_notify == NOTIFY_TYPE_UPDATE) &&
                    ScreenRefresher::isActive()) {
                ScreenRefresher *pRefresher = ScreenRefresher::getInstance();
                if (pRefresher)
                    pRefresher->Notify(update_notify);
                ScreenRefresher::deleteInstance();
            }
            if (tmp_ctrl_status & cabl_bit){
                if(mCABL->Notify(update_notify) == -1) {
                    pthread_mutex_lock(&mCtrlLock);
                    continue;
                }
            } else if ((tmp_ctrl_status & aba_bit) && (bAbaEnabled)){
                if(mABA && (mABA->Notify(update_notify) == -1)){
                    pthread_mutex_lock(&mCtrlLock);
                    continue;
                }
            }
            /* Notify screen refresher last in case of suspend */
            if ((update_notify == NOTIFY_TYPE_SUSPEND) &&
                    ScreenRefresher::isActive()) {
                ScreenRefresher *pRefresher = ScreenRefresher::getInstance();
                if (pRefresher)
                    pRefresher->Notify(update_notify);
                ScreenRefresher::deleteInstance();
            }
            if (update_notify == NOTIFY_TYPE_UPDATE) {
                screenStatus = 1;
                screenOn = 1;
            } else if (update_notify == NOTIFY_TYPE_SUSPEND) {
                screenOn = 0;
            }
            if (tmp_ctrl_status & ad_bit)
                mAD.Notify(update_notify);
        }
        else if (screenStatus == 1) {
            update_notify = NOTIFY_UPDATE_STOP;
            ret =  ioctl(FBFd, MSMFB_NOTIFY_UPDATE, &update_notify);
            if (ret != 0) {
                if (errno != ETIMEDOUT)
                    LOGE("MSMFB_NOTIFY_UPDATE stop ioctl failed");
                pthread_mutex_lock(&mCtrlLock);
                continue;
            }
            screenStatus = 0;
            if (update_notify == NOTIFY_TYPE_SUSPEND){
                screenOn = 0;
            }
            if (tmp_ctrl_status & cabl_bit){
                mCABL->Notify(update_notify);
            } else if (tmp_ctrl_status & aba_bit) {
                if(bAbaEnabled && mABA)
                    mABA->Notify(update_notify);
            }
            if (tmp_ctrl_status & ad_bit)
                mAD.Notify(update_notify);
            if (ScreenRefresher::isActive()) {
                ScreenRefresher *pRefresher = ScreenRefresher::getInstance();
                if (pRefresher)
                    pRefresher->Notify(update_notify);
                ScreenRefresher::deleteInstance();
            }
        }
        pthread_mutex_lock(&mCtrlLock);
    }
    if (mCtrlStatus != tmp_ctrl_status) {
        if (tmp_ctrl_status & cabl_bit && !(mCtrlStatus & cabl_bit)){
            mCABL->Notify(NOTIFY_TYPE_SUSPEND);
        } else if ((tmp_ctrl_status & aba_bit) && !(mCtrlStatus & aba_bit)) {
            if (bAbaEnabled && mABA)
                mABA->Notify(NOTIFY_TYPE_SUSPEND);
        }
        if (tmp_ctrl_status & ad_bit && !(mCtrlStatus & ad_bit))
            mAD.Notify(NOTIFY_TYPE_SUSPEND);
        tmp_ctrl_status = mCtrlStatus;
    }
    // Removing ctrl_bit here so that new control worker thread can be created.
    mCtrlStatus &= ~ctrl_bit;
    pthread_mutex_unlock(&mCtrlLock);
    if (IsPuEnabled()) {
        void *libhandle = NULL;
        int (*sendAckToPU)() = NULL;
        libhandle = dlopen("libqdutils.so", RTLD_NOW);
        if (libhandle) {
            LOGD("%s: Successfully loaded libqdutils.so", __func__);
            *(void **)&sendAckToPU = dlsym(libhandle, "setPartialUpdateState");
            if (sendAckToPU == NULL) {
                LOGE("%s: Could not find setPartialUpdateState", __func__);
            } else {
                ret = sendAckToPU();
                if (ret < 0)
                    LOGE("%s: Failed to ACK PP Off to PU", __func__);
            }
            dlclose(libhandle);
        } else
            LOGE("%s: Not able to load libqdutils.so", __func__);
    }
    LOGD_IF(mDebug, "Exiting control loop");
    return;
}

int DaemonContext::ProcessDebugMsg(char* buf){
    LOGD("ProcessDebugMsg(): Command : %s", buf);

    if (!strncmp(buf, CMD_DEBUG_DAEMON_ON, strlen(CMD_DEBUG_DAEMON_ON))) {
        mDebug = true;
    } else if (!strncmp(buf, CMD_DEBUG_DAEMON_OFF, strlen(CMD_DEBUG_DAEMON_OFF))) {
        mDebug = false;
    } else if (!strncmp(buf, CMD_DEBUG_CABL_ON, strlen(CMD_DEBUG_CABL_ON))) {
        if(bAbaEnabled){
            mABA->SetDebugLevel(true);
        } else {
            mCABL->mDebug = true;
        }
    } else if (!strncmp(buf, CMD_DEBUG_CABL_OFF, strlen(CMD_DEBUG_CABL_OFF))) {
        if(bAbaEnabled){
            mABA->SetDebugLevel(false);
        } else {
            mCABL->mDebug = false;
        }

    } else if (!strncmp(buf, CMD_DEBUG_AD_ON, strlen(CMD_DEBUG_AD_ON))) {
        mAD.mDebug = true;
    } else if (!strncmp(buf, CMD_DEBUG_AD_OFF, strlen(CMD_DEBUG_AD_OFF))) {
        mAD.mDebug = false;
    } else if (!strncmp(buf, CMD_DEBUG_PP_ON, strlen(CMD_DEBUG_PP_ON))) {
        mPostProc.mDebug = true;
    } else if (!strncmp(buf, CMD_DEBUG_PP_OFF, strlen(CMD_DEBUG_PP_OFF))) {
        mPostProc.mDebug = false;
    } else {
        LOGE("Unsupported debug message.");
        return -1;
    }
    return 0;
}

int32_t DaemonContext::PPFeatureControl(int enable, uint32_t features)
{
    char cmd[SHORT_CMD_LEN];
    int32_t ret = 0;
    if (enable) { /* enable PP features */
        if (features & ad_bit) {
            memset(cmd, 0, sizeof(cmd));
            snprintf(cmd, SHORT_CMD_LEN, "%s;%d",
                    CMD_AD_ON, ad_mode_auto_str);
            ret = ProcessCommand(cmd, SHORT_CMD_LEN, -1);
            if (ret)
                LOGE("Unable to process command: %s, ret = %d", cmd, ret);
        }
        if (features & cabl_bit) {
            memset(cmd, 0, sizeof(cmd));
            snprintf(cmd, SHORT_CMD_LEN, "%s", CMD_CABL_ON);
            ret = ProcessCommand(cmd, SHORT_CMD_LEN, -1);
            if (ret)
                LOGE("Unable to process command: %s, ret = %d", cmd, ret);
        }
        if (features & aba_cabl_bit) {
            memset(cmd, 0, sizeof(cmd));
            snprintf(cmd, SHORT_CMD_LEN, "%s", CMD_CABL_ON);
            ret = ProcessCommand(cmd, SHORT_CMD_LEN, -1);
            if (ret)
                LOGE("Unable to process command: %s, ret = %d", cmd, ret);
        }
        if (features & aba_svi_bit) {
            memset(cmd, 0, sizeof(cmd));
            snprintf(cmd, SHORT_CMD_LEN, "%s", CMD_SVI_ON);
            ret = ProcessCommand(cmd, SHORT_CMD_LEN, -1);
            if (ret)
                LOGE("Unable to process command: %s, ret = %d", cmd, ret);
        }
    } else { /* disable PP features */
        if (features == 0) {
            /* If no feature is on, ack should be send to HWC right away */
            void *libhandle = NULL;
            int (*sendAckToPU)() = NULL;
            libhandle = dlopen("libqdutils.so", RTLD_NOW);
            if (libhandle) {
                LOGD("%s: Successfully loaded libqdutils.so", __func__);
                *(void **)&sendAckToPU = dlsym(libhandle, "setPartialUpdateState");
                if (sendAckToPU == NULL) {
                    LOGE("%s: Could not find setPartialUpdateState", __func__);
                } else {
                    ret = sendAckToPU();
                    if (ret < 0)
                        LOGE("%s: Failed to ACK PP Off to PU", __func__);
                }
                dlclose(libhandle);
            } else
                LOGE("%s: Not able to load libqdutils.so", __func__);
        }

        if (features & ad_bit) {
            memset(cmd, 0, sizeof(cmd));
            snprintf(cmd, SHORT_CMD_LEN, "%s", CMD_AD_OFF);
            ret = ProcessCommand(cmd, SHORT_CMD_LEN, -1);
            if (ret)
                LOGE("Unable to process command: %s, ret = %d", cmd, ret);
        }
        if (features & cabl_bit) {
            memset(cmd, 0, sizeof(cmd));
            snprintf(cmd, SHORT_CMD_LEN, "%s", CMD_CABL_OFF);
            ret = ProcessCommand(cmd, SHORT_CMD_LEN, -1);
            if (ret)
                LOGE("Unable to process command: %s, ret = %d", cmd, ret);
        }
        if (features & aba_cabl_bit) {
            memset(cmd, 0, sizeof(cmd));
            snprintf(cmd, SHORT_CMD_LEN, "%s", CMD_CABL_OFF);
            ret = ProcessCommand(cmd, SHORT_CMD_LEN, -1);
            if (ret)
                LOGE("Unable to process command: %s, ret = %d", cmd, ret);
        }
        if (features & aba_svi_bit) {
            memset(cmd, 0, sizeof(cmd));
            snprintf(cmd, SHORT_CMD_LEN, "%s", CMD_SVI_OFF);
            ret = ProcessCommand(cmd, SHORT_CMD_LEN, -1);
            if (ret)
                LOGE("Unable to process command: %s, ret = %d", cmd, ret);
        }
    }
    return ret;
}

void DaemonContext::ProcessPollWork() {
    int32_t ret = -1, index = -1;
    ssize_t len;
    char buffer[SYSFS_FILE_LEN + 1] = {0};
    char ADFilePath[MAX_FB_NAME_LEN];
    char cmd[SHORT_CMD_LEN];
    bool ad_enabled = false;
    int pu_enabled = false;
    struct pollfd poll_fd[2];

    LOGD_IF(mDebug, "%s() Entering", __func__);
    if (PUFd > 0) {
        poll_fd[DYN_PU_SYSFS].fd = PUFd; //file descriptor
        poll_fd[DYN_PU_SYSFS].events = POLLPRI | POLLERR; //requested events
    }

    if (mAD.isADEnabled() == AD_ENABLE_WB) {
        ret = SelectFB(MDP_LOGICAL_BLOCK_DISP_2, &index);
        if (ret == 0 && index >= 0) {
            memset(buffer, 0, sizeof(buffer));
            memset(ADFilePath, 0, sizeof(ADFilePath));
            snprintf(ADFilePath, sizeof(ADFilePath), "/sys/class/graphics/fb%d/ad", index);
            LOGD_IF(mDebug, "Polling on %s", ADFilePath);
            ad_fd = open(ADFilePath, O_RDONLY);
            if (ad_fd < 0) {
                LOGE("Unable to open fd%d/ad node  err:  %s", index, strerror(errno));
            } else {
                poll_fd[AD_WB_SYSFS].fd = ad_fd; //file descriptor
                poll_fd[AD_WB_SYSFS].events = POLLPRI | POLLERR; //requested events
            }
        }
    }

    if ((PUFd < 0) && (ad_fd < 0)) {
        LOGE("No valid fd to pull, returning");
        return;
    }

    pu_enabled = IsPuEnabled();
    if (pu_enabled) {
        mEnabledFeatures = mCtrlStatus;
        ret = PPFeatureControl(false, mEnabledFeatures);
        if (ret)
            LOGE("Unable to process command: %s, ret = %d", cmd, ret);
        PuEnabled = true;
    } else {
        PuEnabled = false;
        ret = PPFeatureControl(true, mEnabledFeatures);
        if (ret)
            LOGE("Unable to process command: %s, ret = %d", cmd, ret);
    }
    while (!sigflag) {
        if ((ret = poll(poll_fd, 2, -1)) < 0) {//infinite timeout
            LOGE("Error in polling sysfs nodes: %s.", strerror(errno));
            break;
        } else {
            if (poll_fd[AD_WB_SYSFS].revents & POLLPRI) {
                memset(buffer, 0, SYSFS_FILE_LEN);
                len = pread(ad_fd, buffer, SYSFS_FILE_LEN, 0);
                if (len > 0) {
                    buffer[SYSFS_FILE_LEN] = '\0';
                    ad_enabled = atoi(&buffer[0]) == 1;
                    LOGD_IF(mDebug, "Requested AD Status from AD node: %d", ad_enabled);
                    //turn on/off ad according to the value of ad_enabled
                    memset(cmd, 0, sizeof(cmd));
                    if (ad_enabled)
                        snprintf(cmd, SHORT_CMD_LEN, "%s;%d;%d",
                                CMD_AD_ON, ad_mode_auto_str, MDP_LOGICAL_BLOCK_DISP_2);
                    else
                        snprintf(cmd, SHORT_CMD_LEN, "%s", CMD_AD_OFF);
                    ret = ProcessCommand(cmd, SHORT_CMD_LEN, -1);
                    if (ret)
                        LOGE("Unable to process command for AD, ret = %d", ret);

                } else if (len == 0) {
                    LOGE_IF(mDebug, "No data to read from AD node.");
                } else {
                    LOGE("Unable to read AD node, %s", strerror(errno));
               }
            }
            if (poll_fd[DYN_PU_SYSFS].revents & POLLPRI) {
                pu_enabled = IsPuEnabled();
                LOGD_IF(mDebug, "Requested PU Status from pu node: %d", pu_enabled);
                if (pu_enabled) { /* PU enabled, disable PP features */
                    /* cache the PP feature status */
                    mEnabledFeatures = mCtrlStatus;
                    ret = PPFeatureControl(false, mEnabledFeatures);
                    if (ret)
                        LOGE("Unable to process command: %s, ret = %d", cmd, ret);
                    PuEnabled = true;
                } else { /* PU is disabled, turn PP features back on */
                    PuEnabled = false;
                    ret = PPFeatureControl(true, mEnabledFeatures);
                    if (ret)
                        LOGE("Unable to process command: %s, ret = %d", cmd, ret);
                }
            }
        }
    }

    LOGD_IF(mDebug, "Closing the ad node.");
    close(ad_fd);

    LOGD_IF(mDebug, "%s() Exiting", __func__);
    return;
}

using namespace qmode;
int DaemonContext::LoadQdcmLibrary() {
    mQdcmLib = dlopen("libmm-qdcm.so", RTLD_NOW);
    ModeManager* (*factory)() = NULL;
    int ret = 0;

    if (mQdcmLib)
        *(void **)&factory = dlsym(mQdcmLib, "getObject");
    else {
        LOGE("%s: QDCM library loading failed", __func__);
        return -EFAULT;
    }

    if (factory)
        mQdcmMode = factory();
    else
        mQdcmMode = NULL;

    if (mQdcmMode) {
        LOGD("%s: QDCM library loaded successfully mQdcmLib = %p, \
            factory = %p, mQdcmMode = %p", __func__, mQdcmLib, factory,
            mQdcmMode);
        ret = 0;
        bQdcmLoaded = true;
    } else {
        LOGE("%s: Failed creating mode manager instance", __func__);
        ret = -EFAULT;
    }

    return ret;
}

void DaemonContext::UnloadQdcmLibrary() {
    void (*destroy)(ModeManager*) = NULL;

    if (mQdcmLib)
        *(void **)&destroy = dlsym(mQdcmLib, "deleteObject");

    if (destroy) {
        destroy(mQdcmMode);
        mQdcmMode = NULL;
    }

    if (mQdcmLib) {
        dlclose(mQdcmLib);
        mQdcmLib = NULL;
    }
    bQdcmLoaded = false;
}

int32_t DaemonContext::ProcessCommand(char *buf, const int32_t len, const int32_t& fd) {
    int result = 0;
    int ret = 0;

    if (NULL == buf) {
        LOGE("Command string is NULL!");
        result = -1;
        return result;
    }

    LOGD_IF(mDebug, "Command received: %s ", buf);
    if (PuEnabled) {
        LOGD("Partial update is enabled, cannot support PP features!");
        result = -1;
        return result;
    }
    if (!strncmp(buf, CMD_SET, strlen(CMD_SET))) {
        result = ProcessSetMsg(&buf[0]);
        reply(!result, fd);
    } else if (!strncmp(buf, CMD_DCM_ON, strlen(CMD_DCM_ON))) {
        result = -1;
        if(!mDCM)
            mDCM = new DCM();
        if (mDCM) {
            result = mDCM->DCMControl(true);
        }
        reply(!result, fd);
    } else if (!strncmp(buf, CMD_DCM_OFF, strlen(CMD_DCM_OFF))) {
        if (mDCM && (mDCM->mStatus == DCM_ON)) {
            result = mDCM->DCMControl(false);
            delete mDCM;
            mDCM = NULL;
        } else if (!mDCM) {
            LOGD("DCM is already off !");
            result = 0;
        } else {
            result = -1;
        }

        reply(!result, fd);
    } else if (!strncmp(buf, CMD_PP_PREFIX, strlen(CMD_PP_PREFIX))) {
        result = ProcessPPMsg(&buf[0], fd);
    } else if (!strncmp(buf, CMD_CABL_PREFIX, strlen(CMD_CABL_PREFIX))) {
        StartAlgorithmObjects();
        if (mCABL || (bAbaEnabled && mABA))
            result = ProcessCABLMsg(&buf[0], fd);
        else {
            /* Denote process command failure if not support API */
            if (strncmp(buf, CMD_CABL_SUPPORT, strlen(CMD_CABL_SUPPORT)))
                result = -1;
            reply(false, fd);
        }
    } else if (!strncmp(buf, CMD_SVI_PREFIX, strlen(CMD_SVI_PREFIX))) {
        StartAlgorithmObjects();
        if(bAbaEnabled && mABA)
            result = ProcessSVIMsg(&buf[0], fd);
        else {
            /* Denote process command failure if not support API */
            if (strncmp(buf, CMD_SVI_SUPPORT, strlen(CMD_SVI_SUPPORT)))
                result = -1;
            reply(false, fd);
        }
    } else if (!strncmp(buf, CMD_AD_PREFIX, strlen(CMD_AD_PREFIX))) {
        result = ProcessADMsg(&buf[0], fd);
    } else if (!strncmp(buf, CMD_OEM_PREFIX, strlen(CMD_OEM_PREFIX))) {
        /* pass the command to the OEM module */
        pp_oem_message_handler(buf, len, fd);
    } else if (!strncmp(buf, CMD_BL_SET, strlen(CMD_BL_SET))) {
        int32_t backlight;
        sscanf(buf, CMD_BL_SET ";" "%d", &backlight);
        if (mAD.ADStatus() == AD_CALIB_ON) {
            pthread_mutex_lock(&mADOpLock);
            result = mAD.ADSetCalibBL(backlight);
            pthread_mutex_unlock(&mADOpLock);
        } else {
            LOGE("AD calib is not on, start AD calib first!");
        }
        reply(!result, fd);
    } else if (!strncmp(buf, CMD_QDCM_PREFIX, strlen(CMD_QDCM_PREFIX))) {
        if(!bQdcmLoaded) {
            result = LoadQdcmLibrary();
            if (result)
                reply(!result, fd);
        }
        //will be replying within the function depending on the cases.
        if (!result)
            result = mQdcmMode->requestRoute(buf, len, fd);
        // call invalidate for forcing frame update
        if (!strncmp(buf, CMD_QDCM_PA_PREFIX, strlen(CMD_QDCM_PA_PREFIX))) {
            ScreenRefresher *pRefresher = ScreenRefresher::getInstance();
            if (pRefresher) {
                pRefresher->Control(1);
                pRefresher->Refresh(1, 16);
                pRefresher->Control(0);
                ScreenRefresher::deleteInstance();
            }
        }
    } else if (!strncmp(buf, CMD_DEBUG_PREFIX, strlen(CMD_DEBUG_PREFIX))) {
        result = ProcessDebugMsg(&buf[0]);
        reply(!result, fd);
    } else {
        LOGE("Unknown command for pp daemon: %s", buf);
        result = -1;
    }
    ret = result;
    pthread_mutex_lock(&mCtrlLock);
    if (!result && !(mCtrlStatus & ctrl_bit) &&
        ((mCtrlStatus & cabl_bit) || (mCtrlStatus & aba_bit) ||
            (mCtrlStatus & ad_bit))) {
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
        ret = pthread_create(&mControlThrdId, &attr, &DaemonContext::control_thrd_func, this);
        if (ret)
            LOGE("Failed to start the control thread, status = %d", ret);
        else
            mCtrlStatus |= ctrl_bit;
        pthread_attr_destroy(&attr);
    }
    if (!result && (mCtrlStatus & ctrl_bit) &&
        !(mCtrlStatus & cabl_bit) && !(mCtrlStatus & aba_bit) &&
            !(mCtrlStatus & ad_bit)) {
            void *term;
            uint32_t update_cancel = NOTIFY_UPDATE_DEINIT;
            pthread_mutex_unlock(&mCtrlLock);
            result = ioctl(FBFd, MSMFB_NOTIFY_UPDATE, &update_cancel);
            /*
             * Error is not harmful as failure to deinit would only cause
             * additional delay during exit and will not fail anything more
             */
            if (result)
                LOGD("%s: Not able to cancel update notify %d", __func__, result);
            pthread_join(mControlThrdId, &term);
            pthread_mutex_lock(&mCtrlLock);
            LOGD_IF(mDebug, "control thread terminated");
    }
    pthread_mutex_unlock(&mCtrlLock);
    return ret;
}

int32_t DaemonContext::getListenFd() {
    /* trying to open a socket */
    mListenFd = android_get_control_socket(DAEMON_SOCKET);
    if (mListenFd < 0) {
        LOGE("Obtaining listener socket %s failed", DAEMON_SOCKET);
        return -1;
    }
    LOGD_IF(mDebug, "Acquired the %s socket", DAEMON_SOCKET);
    /* listen to the opened socket */
    if (listen(mListenFd, mNumConnect) < 0) {
        LOGE("Unable to listen on fd '%d' for socket %s",
                                  mListenFd, DAEMON_SOCKET);
        return -1;
    }
    return 0;
}

void DaemonContext::StartAlgorithmObjects() {

    if(mABA || mCABL)
        return;

    if ((2 == display_pp_cabl_supported()) || (mBootStartAbaCabl)
        || (1 == display_pp_svi_supported()) || (mBootStartAbaSvi)) {
        mABA = new AbaContext();
        mABA->initHW();
    } else if ((1 == display_pp_cabl_supported()) || (mBootStartCABL)) {
        mCABL = new CABL();
        mCABL->initHW();
    }
}

void DaemonContext::StopAlgorithmObjects() {
    if ((bAbaEnabled) && (mABA)) {
        delete mABA;
    } else if (mCABL){
        delete mCABL;
    }
}

int DaemonContext::SelectFB(int display_id, int *fb_idx) {
    int ret = -1, index = -1, i = 0, j = 0;
    ssize_t len;
    int fd_type[TOTAL_FB_NUM] = {-1, -1, -1};
    char fb_type[MAX_FB_NAME_LEN];
    char msmFbTypePath[MAX_FB_NAME_LEN];
    const char* logical_display_0[PRIMARY_PANEL_TYPE_CNT] = {HDMI_PANEL,
        LVDS_PANEL, MIPI_DSI_VIDEO_PANEL, MIPI_DSI_CMD_PANEL, EDP_PANEL};
    const char* logical_display_1[EXTERNAL_PANEL_TYPE_CNT] = {DTV_PANEL};
    const char* logical_display_2[WRITEBACK_PANEL_TYPE_CNT] = {WB_PANEL};

    for (i = 0; i < TOTAL_FB_NUM; i++) {
        memset(fb_type, 0, sizeof(fb_type));
        snprintf(msmFbTypePath, sizeof(msmFbTypePath),
        "/sys/class/graphics/fb%d/msm_fb_type", i);
        fd_type[i] = open(msmFbTypePath, O_RDONLY);
        if (fd_type[i] >= 0 ) {
            len = read(fd_type[i], fb_type, sizeof(fb_type));
            if (len == 0) {
                continue;
            } else if (len < 0) {
                ret = errno;
                LOGE("Unable to read fb type file at fd_type[%d], err:  %s", i, strerror(errno));
                break;
            }

            if (display_id == MDP_LOGICAL_BLOCK_DISP_0) {
                for(j = 0; j < PRIMARY_PANEL_TYPE_CNT; j++) {
                    if(!strncmp(fb_type, logical_display_0[j] , strlen(logical_display_0[j]))) {
                        index = i;
                        ret = 0;
                        goto exit;
                    }
                }
            } else if (display_id == MDP_LOGICAL_BLOCK_DISP_1) {
                for(j = 0; j < EXTERNAL_PANEL_TYPE_CNT; j++) {
                    if(!strncmp(fb_type, logical_display_1[j], strlen(logical_display_1[j]))) {
                        index = i;
                        ret = 0;
                        goto exit;
                    }
                }
            } else if (display_id == MDP_LOGICAL_BLOCK_DISP_2) {
                for(j = 0; j < WRITEBACK_PANEL_TYPE_CNT; j++) {
                    if(!strncmp(fb_type, logical_display_2[j], strlen(logical_display_2[j]))) {
                        index = i;
                        ret = 0;
                        goto exit;
                    }
                }
            } else {
                LOGE("Unsupported display_id %d", display_id);
                ret = -1;
                break;
            }
        } else {
            LOGE("Unable to open fb type file  err:  %s", strerror(errno));
        }
    }

exit:
    for (i = 0; i < TOTAL_FB_NUM; i++) {
        if (fd_type[i] >= 0) {
            close(fd_type[i]);
        }
    }

    *fb_idx = index;
    return ret;
}

bool DaemonContext::IsSplitDisplay(int fb_fd) {
    struct fb_var_screeninfo info;
    struct msmfb_mdp_pp pp;

    if (ioctl(fb_fd, FBIOGET_VSCREENINFO, &info) == -1) {
        LOGE("Error getting FB Info : %s", strerror(errno));
        return false;
    }

    if ((info.xres > gHwInfo.maxMixerWidth) || display_pp_has_right_split())
        mSplitDisplay = true;

    return mSplitDisplay;
}

void *handler_thread(void *data) {
    DaemonContext *context = (DaemonContext *)data;
    if (!context) {
        LOGE("Context object passed to thread is null");
        return NULL;
    }
    /* Make this thread detached */
    pthread_detach(pthread_self());
    if (context->mAcceptFd < 0) {
        LOGE("No valid accepted connection fd!");
    }
    int32_t acceptFd = context->mAcceptFd;
    context->mAcceptFd = -1;
    LOGD_IF(context->mDebug, "Started the handler for acceptFd %d", acceptFd);

    int32_t len = 0;
    struct pollfd read_fd;
    read_fd.fd = acceptFd;
    /*
     * Polling on read_ready event (POLLIN and POLLRDNORM). If poll() returns
     * a positive number of revents, then either the socket is ready for
     * reading, or the socket has been closed with possible error revents set
     * (POLLERR, POLLHUP, POLLNVAL).
     */
    read_fd.events = POLLIN | POLLRDNORM | POLLPRI;
    read_fd.revents = 0;

    while (!sigflag && (len = poll(&read_fd, 1, CONN_TIMEOUT)) >= 0) {
        if (len == 0) {
            LOGD_IF(context->mDebug, "poll() completed with no revents on fd %d",
                    acceptFd);
            continue;
        }
        else {
            LOGD_IF(context->mDebug, "poll completed with revents %hx on fd %d",
                    read_fd.revents, acceptFd);
            char buf[MAX_CMD_LEN];
            memset(buf, 0, sizeof(buf));
            /*
             * If read() is successful, then poll() contained revents of pending
             * read (POLLIN, POLLRDNORM, POLLPRI). If read() returns no data,
             * then the revents indicate an error has occurred or the connection
             * has been closed by the client.
             */
            ssize_t ret_val = read (acceptFd, buf, sizeof(buf));
            if (ret_val == 0) {
                LOGD_IF(context->mDebug, "connection has closed on fd %d",
                        acceptFd);
                break;
            } else if (ret_val < 0) {
                LOGE("Unable to read the command %s", strerror(errno));
                break;
            }

            if ((context->mAD.isADEnabled() == AD_ENABLE_WB) &&
                    (!strncmp(buf, CMD_AD_PREFIX, strlen(CMD_AD_PREFIX)))) {
                LOGD_IF(context->mDebug, "AD on WB FB, disabling the AD on primary FB.");
                break;
            }

            if(context->ProcessCommand(buf, len, acceptFd)) {
                if (!PuEnabled)
                    LOGE("Failed to process the command!");
            }
        }
    }

    if (len < 0) {
        LOGE("poll failed on fd %d: %s", acceptFd, strerror(errno));
    }

    close(acceptFd);
    return NULL;
}

void *listener_thread(void* data) {
    DaemonContext *context = (DaemonContext *)data;
    if (NULL == context) {
        LOGE("Context object passed to thread is null");
        return NULL;
    }
    LOGD_IF(context->mDebug, "Starting the listening thread");
    pthread_detach(pthread_self());
    /* wait for the connection */
    while (!sigflag) {
        struct sockaddr addr;
        socklen_t alen;
        alen = (int) sizeof (addr);
        /* check for a new connection */
        int32_t acceptFd = accept(context->mListenFd, &addr, &alen);
        if (acceptFd >= 0) {
            LOGD_IF(context->mDebug, "Accepted connection fd %d, \
                                                creating handler", acceptFd);
            /* creating the handler thread for this connection */
            pthread_t handler;
            while (context->mAcceptFd != -1) {
                LOGD("handler has not copied the data yet... wait some time");
                sleep(1);
            }
            context->mAcceptFd = acceptFd;
            if (pthread_create(&handler, NULL, handler_thread, context)) {
                LOGE("Failed to create the handler thread for Fd %d", acceptFd);
                close(acceptFd);
                break;
            }
            sched_yield();
        } else {
            LOGE("%s: Failed to accept socket connection", strerror(errno));
        }
    }
    LOGD_IF(context->mDebug, "Closing the listening socket at fd %d",
            context->mListenFd);
    close(context->mListenFd);
    return NULL;
}

int32_t DaemonContext::start() {
    /* start listening to the socket for connections*/
    int32_t err = getListenFd();
    if (err) {
        LOGE("Failed to listen to the socket");
        return err;
    }
    pthread_t thread_id;
    /* start the listener thread to handle connection requests */
    err = pthread_create(&thread_id, NULL, listener_thread, this);
    if (err) {
        LOGE("Failed to start the listener thread");
        return err;
    }

    /* start PP polling thread, which checks the content of ad and dyn_pu file */
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    LOGD_IF(mDebug, "Creating poll thread");
    err = pthread_create(&mPollThrdId, &attr, &poll_thrd_func, this);
    if (err)
        LOGE("Failed to start the poll thread, err = %d", err);
    pthread_attr_destroy(&attr);
    return 0;
}

int DaemonContext::ProcessSetMsg(char* buf)
{
    int ret = -1;

    if (strstr(buf, FEATURE_PCC)) {
         ret = ProcessPCCMsg(&buf[0]);
        if (ret){
            LOGE("Failed to set PCC data");
        }
    } else {
        LOGE("This message is not supported currently!");
    }
    return ret;
}

int DaemonContext::ProcessPCCMsg(char* buf)
{

    char* tokens[MIN_PCC_PARAMS_REQUIRED];
    char *temp_token = NULL;
    int ret = -1, index = 0;
    int mdp_block = 0, offset = 0;
    struct display_pp_pcc_cfg pcc_cfg;
    struct display_pcc_coeff *coeff_ptr = NULL;

    memset(tokens, 0, sizeof(tokens));
    ret = tokenize_params(buf, TOKEN_PARAMS_DELIM, MIN_PCC_PARAMS_REQUIRED, tokens, &index);
    if(ret){
        LOGE("tokenize_params failed! (Line %d)", __LINE__);
        goto err;
    }

    if (index != MIN_PCC_PARAMS_REQUIRED){
        LOGE("invalid number of params reqiuired = %d != given = %d",
                MIN_PCC_PARAMS_REQUIRED, index);
        goto err;
    }

    LOGD_IF(mDebug, "tokenize_params successful with index = %d", index);

    if (tokens[DISPLAY_INDEX]) {
        switch(atoi(tokens[DISPLAY_INDEX])) {
            case PRIMARY_DISPLAY:
                mdp_block = MDP_LOGICAL_BLOCK_DISP_0;
                break;
            case SECONDARY_DISPLAY:
                mdp_block = MDP_LOGICAL_BLOCK_DISP_1;
                break;
            case WIFI_DISPLAY:
                mdp_block = MDP_LOGICAL_BLOCK_DISP_2;
                break;
            default:
                LOGE("Display option is invalid");
                goto err;
        }
    } else {
        LOGE("Display option is not provided!");
        goto err;
    }

    pcc_cfg.ops = MDP_PP_OPS_ENABLE | MDP_PP_OPS_WRITE;
    coeff_ptr = &pcc_cfg.r;
    while(offset != (MIN_PCC_PARAMS_REQUIRED - DATA_INDEX)) {
        for(index = 0; index < PCC_COEFF_PER_COLOR; index++) {
            coeff_ptr->pcc_coeff[index] = atof(tokens[index + DATA_INDEX + offset]);
        }
        offset += PCC_COEFF_PER_COLOR;
        switch(offset){
            case PCC_COEFF_PER_COLOR:
                coeff_ptr = &pcc_cfg.g;
                break;
            case (PCC_COEFF_PER_COLOR * 2):
                coeff_ptr = &pcc_cfg.b;
                break;
            default:
                break;
        }
    }

    LOGD_IF(mDebug, "Calling user space library for PCC!!");
    ret = display_pp_pcc_set_cfg(mdp_block, &pcc_cfg);
err:
    return ret;
}

