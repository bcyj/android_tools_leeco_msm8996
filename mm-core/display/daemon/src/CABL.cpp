/*
 * DESCRIPTION
 * This file contains the context for running the CABL thread and algorithm.
 * This CABL context is similar to ABL in the AbaContext, but they are
 * separate versions of the context and algorithm.
 *
 * Copyright (c) 2012-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 */

#include "CABL.h"

void CABL::initHW() {
    /* Hardware capacity determination*/
    mHistInfo.hist_block = gHwInfo.nPriDisplayHistBlock;
    mHistInfo.hist_bins = gHwInfo.nPriDisplayHistBins;
    mHistInfo.hist_components = gHwInfo.nPriDisplayHistComp;
}

int32_t CABL::auto_adjust_quality_lvl(){
    int32_t result = 0;
    char lvl[MAX_CMD_LEN];
    char property[PROPERTY_VALUE_MAX];
    if (property_get(YUV_INPUT_STATE_PROP, property, NULL) > 0) {
        if ((atoi(property) == 1) && (mPowerSaveLevel != mOEMParams.video_quality_lvl)) {
            mPowerSaveLevel = mOEMParams.video_quality_lvl;
            LOGD_IF(mDebug, "Power saving level: %d", mPowerSaveLevel);
            pthread_mutex_lock(&mCABLLock);
            result = abl_change_quality_level(&mOEMParams, mPowerSaveLevel);
            pthread_mutex_unlock(&mCABLLock);
        }else if ((atoi(property) == 0) && (mPowerSaveLevel != mOEMParams.ui_quality_lvl)) {
            mPowerSaveLevel = mOEMParams.ui_quality_lvl;
            LOGD_IF(mDebug, "Power saving level: %d", mPowerSaveLevel);
            pthread_mutex_lock(&mCABLLock);
            result = abl_change_quality_level(&mOEMParams, mPowerSaveLevel);
            pthread_mutex_unlock(&mCABLLock);
        }
    }
    return result;
}

int is_backlight_modified(CABL *cabl, uint32_t *old_lvl)
{
    int ret;
    uint32_t temp_lvl = get_backlight_level();

    struct fb_cmap cmap;
    if (temp_lvl != *old_lvl) {
        ret = 1;
        LOGD_IF(cabl->mDebug, "The BL level changed,");
        *old_lvl = temp_lvl;
        LOGD_IF(cabl->mDebug, "The BL level changed to %u", temp_lvl);
        /* Reset the orig level only if > than the min level */
        if (temp_lvl >= cabl->mOEMParams.bl_min_level) {
            cabl->mOEMParams.orig_level = temp_lvl;
            abl_change_orig_bl_level(&cabl->mOEMParams);
            //set the new restore level
            cabl->mUserBLLevel = temp_lvl;
        }
    } else {
        ret = 0;
    }
    return ret;
}

/* Worker thread */
void *worker(void *data) {
    CABL *cabl = (CABL *) data;
    uint32_t old_level, set_ratio, bl_scale_ratio = 0;
    int32_t ret;

    if (NULL == cabl)
    {
        LOGE("cabl object passed to worker thread is null");
        return NULL;
    }

    old_level = cabl->mUserBLLevel;
    set_ratio = BL_SCALE_MAX;
    if (cabl->mRefresher) {
        LOGD_IF(cabl->mDebug, "Calling mRefresher->control(true)");
        cabl->mRefresher->Control(1);
    }

    LOGD_IF(cabl->mDebug, "Starting worker thread");
    while (true) {
        pthread_mutex_lock(&cabl->mCABLLock);
        if (cabl->eStatus != CABL_ON) {
            pthread_mutex_unlock(&cabl->mCABLLock);
            break;
        }
        if (cabl->mPowerLevelChange) {
            ret = abl_change_quality_level(&cabl->mOEMParams,
                cabl->mPowerSaveLevel);
            cabl->mPowerLevelChange = false;
            if (ret)
                LOGE("%s: CABL Quality change failed to set %d",__func__,
                    cabl->mPowerSaveLevel);
        }
        pthread_mutex_unlock(&cabl->mCABLLock);

        if (AUTO_QL_MODE == cabl->mOEMParams.default_ql_mode) {
            /* Set the power saving level according to the input format*/
            ret = cabl->auto_adjust_quality_lvl();
            if (ret) {
                LOGE("adjust_quality_level failed");
            }
        }

        pthread_mutex_lock(&cabl->mCABLLock);
        while ((cabl->mHistStatus == 0) && (cabl->eStatus == CABL_ON)) {
            pthread_cond_wait(&cabl->mCABLCond, &cabl->mCABLLock);
        }
        if (cabl->eStatus == CABL_OFF) {
            pthread_mutex_unlock(&cabl->mCABLLock);
            break;
        }
        /* Set LUT */
        if (cabl->mHistStatus == 1) {
            pthread_mutex_unlock(&cabl->mCABLLock);
            is_backlight_modified(cabl, &old_level);
            ret = abl_calc_lut((int32_t *)&bl_scale_ratio, FBFd,
                &cabl->mColorMap);
            if (ret != 0) {
                if ((ret != -ENODATA) && (ret != -ETIMEDOUT) && (ret != -EPERM))
                    LOGE("abl_calc_lut() failed with ret = %d", ret);
                /* If caused by stopped histogram sleep to yield to control */
                if (ret == -EPERM || ret == -ENODATA)
                    usleep(16666);
                continue;
            }
            if (bl_scale_ratio > 1024) {
                LOGE("%s: Possible Integer overlow! scale=%u, capped to 1024",
                        __func__, bl_scale_ratio);
                bl_scale_ratio = 1024;
            }

            if (bl_scale_ratio != set_ratio) {
                LOGD_IF(cabl->mDebug,
                        "User level = %03u Set Ratio = %03u Scale Ratio = %04u",
                        cabl->mUserBLLevel, set_ratio, bl_scale_ratio);
                /*Check to see if BL modified during calculation*/
                if (is_backlight_modified(cabl, &old_level)) {
                    /*Revert ABL calculations as not applying LUT*/
                    abl_revert_calc();
                    continue;
                }
                ret = abl_set_lut(FBFd, &cabl->mColorMap);
                if (ret != 0) {
                    LOGE("abl_set_lut() failed");
                    abl_revert_calc();
                    continue;
                }
                set_backlight_scale(bl_scale_ratio,
                    cabl->mOEMParams.bl_min_level);
                set_ratio = bl_scale_ratio;
                cabl->mScale = bl_scale_ratio;
                if (cabl->mRefresher) {
                    LOGD_IF(cabl->mDebug, "Calling screenRefresh");
                    cabl->mRefresher->Refresh(1, 16);
                }
            }
            abl_update_history();
        } else {
            pthread_mutex_unlock(&cabl->mCABLLock);
        }
    }
    abl_exit(FBFd, &cabl->mColorMap);
    if (cabl->mRefresher) {
        LOGD_IF(cabl->mDebug, "Calling mRefresher->control(false)");
        cabl->mRefresher->Control(0);
    }
    pthread_exit(NULL);

    return NULL;
}

int CABL::Notify(uint32_t notification_type) {
    int32_t ret = 0;
    LOGD_IF(mDebug, "Starting %s ", __func__);
    pthread_mutex_lock(&mCABLLock);
    if (mHistStatus == 0 && (notification_type == NOTIFY_TYPE_BL_UPDATE || notification_type == NOTIFY_TYPE_UPDATE)) {
        uint32_t level = get_backlight_level();
        if (level <= mOEMParams.bl_min_level) {
            LOGE_IF(mDebug, "New BL level %d lower than min level %u,"
                    " Skip this update for calc",
                    level, mOEMParams.bl_min_level);
            if (mPrevNotify == NOTIFY_TYPE_SUSPEND)
                ret = -1;
            goto exit_early;
        }
        if (eStatus == CABL_OFF) {
            pthread_mutex_unlock(&mCABLLock);
            return ret;
        }
        LOGD_IF(mDebug, "Start notification received, start histogram");
        ret = startHistogram();
        if (0 == ret) {
            mHistStatus = 1;
            pthread_cond_signal(&mCABLCond);
        }
    } else if (mHistStatus == 1 &&
        (notification_type == NOTIFY_TYPE_SUSPEND ||
        notification_type == NOTIFY_TYPE_NO_UPDATE)) {
        LOGD_IF(mDebug, "Stop notification received, stop histogram");
        if(!stopHistogram(mHistStatus))
            mHistStatus = 0;
        if (notification_type == NOTIFY_TYPE_SUSPEND)
            abl_reset_history();

    }

    if (notification_type == NOTIFY_TYPE_BL_UPDATE) {
        if (mRefresher) {
            mRefresher->Control(1);
            LOGE_IF(mDebug, "Calling refresh()");
            mRefresher->Refresh(1, 16);
            mRefresher->Control(0);
        }
    }
exit_early:
    mPrevNotify = notification_type;
    pthread_mutex_unlock(&mCABLLock);
    return ret;
}

int32_t CABL::start_cabl() {
    LOGD_IF(mDebug, "Start_CABL E");
    pthread_mutex_lock(&mCABLCtrlLock);
    if (gHwInfo.nPriDisplayHistBins == 0) {
        LOGE("CABL not supported on this HW!");
        pthread_mutex_unlock(&mCABLCtrlLock);
        return -1;
    }

    if (!mEnable) {
        LOGE("CABL not enabled!");
        pthread_mutex_unlock(&mCABLCtrlLock);
        return -1;
    }
    pthread_mutex_lock(&mCABLLock);
    if (eStatus == CABL_ON) {
        LOGD("CABL is already on!");
        pthread_mutex_unlock(&mCABLLock);
        pthread_mutex_unlock(&mCABLCtrlLock);
        return 0;
    }
    pthread_mutex_unlock(&mCABLLock);
    /* Get the user defined power saving level */
    char property[PROPERTY_VALUE_MAX];
    if (property_get("hw.cabl.level", property, NULL) > 0) {
        int newLvl = ql_string2int(property);
        if (newLvl == ABL_QUALITY_AUTO) {
            mPowerSaveLevel = ABL_QUALITY_HIGH;
        } else if ((newLvl < 0) || (newLvl > ABL_QUALITY_MAX)) {
            mPowerSaveLevel = ABL_QUALITY_HIGH;
            LOGE("Invalid power saving level, setting hw.cabl.level to High");
        } else if (newLvl < ABL_QUALITY_MAX) {
            mPowerSaveLevel = newLvl;
        }
        LOGD_IF(mDebug, "Power saving level: %d", mPowerSaveLevel);
    }

    // Reset CABL structure to 0 to clear any garbage static data
    memset(&mColorMap, 0, sizeof(struct fb_cmap));

    /* Get the backlight level and initialize the new level to it */
    mUserBLLevel = get_backlight_level();

    /* oem initialization */
    mOEM.ConfigureABLParameters(&mOEMParams, mPowerSaveLevel, mUserBLLevel);
    set_interpolated_bl_min_level(&mOEMParams);

    // Check if initial level is more than the minimum allowed level
    // otherwise, set it to the min level
    uint32_t init_level = (mUserBLLevel >= mOEMParams.bl_min_level) ?
            mUserBLLevel : mOEMParams.bl_min_level;
    LOGD_IF(mDebug, "Original backlight level %03u "
            "Initializing with: %03u Min level: %03d",
            mUserBLLevel, init_level, mOEMParams.bl_min_level);

    /* driver initialization */
    int32_t ret = abl_init(FBFd, &mColorMap, &mOEMParams, &mHistInfo,
                                                mPowerSaveLevel, init_level);
    if (ret) {
        LOGE("abl_init() failed!");
        free_cmap(&mColorMap);
        pthread_mutex_unlock(&mCABLCtrlLock);
        return ret;
    }

    mRefresher = ScreenRefresher::getInstance();

    /* ABL Init is done, let the control
     * thread start the histogram */
    stopHistogram(mHistStatus);
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    /* update the CABL status so threads can run */
    eStatus = CABL_ON;
    /* Start the worker thread */
    int32_t err = pthread_create(&mWorkerThreadID, &attr, worker, this);
    if (err) {
        LOGE("CABL: Failed to start the control thread");
        eStatus = CABL_OFF;
        free_cmap(&mColorMap);
        pthread_attr_destroy(&attr);
        pthread_mutex_unlock(&mCABLCtrlLock);
        return -1;
    }
    pthread_attr_destroy(&attr);
    LOGD_IF(mDebug, "Start_CABL X");
    pthread_mutex_unlock(&mCABLCtrlLock);
    return 0;
}

void CABL::stop_cabl() {
    LOGD_IF(mDebug, "Stop_CABL E");
    void *ret;
    pthread_mutex_lock(&mCABLCtrlLock);
    pthread_mutex_lock(&mCABLLock);
    if (eStatus == CABL_OFF) {
        LOGD("CABL is already off!");
        pthread_mutex_unlock(&mCABLLock);
        pthread_mutex_unlock(&mCABLCtrlLock);
        return;
    }
    /* stop the threads */
    eStatus = CABL_OFF;
    pthread_mutex_unlock(&mCABLLock);
    pthread_cond_signal(&mCABLCond);
    /* signal the worker for exit */
    pthread_join(mWorkerThreadID, &ret);
    LOGD_IF(mDebug, "CABL Threads teminated, cleaning up resources");
    pthread_mutex_lock(&mCABLLock);
    mHistStatus = 0;
    pthread_mutex_unlock(&mCABLLock);
    set_backlight_scale(BL_SCALE_MAX, mOEMParams.bl_min_level); //Restore BL to 100%
    ScreenRefresher::deleteInstance();
    mRefresher = NULL;
    LOGD_IF(mDebug, "Stop_CABL X");
    pthread_mutex_unlock(&mCABLCtrlLock);
}

