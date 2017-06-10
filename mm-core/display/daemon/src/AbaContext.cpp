/*
 * DESCRIPTION
 * This file runs the ABA algorithm, which contains both CABL and SVI.
 * It interacts with the pp-daemon in order to run on its own.
 *
 * Copyright (c) 2012-2015 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 */

#include "AbaContext.h"
#define  MAX_CHECKSUM_LENGTH 20

static void initialize_cmap(struct fb_cmap *cmap);
static void free_hist(struct mdp_histogram_data *hist)
{

    if(hist == NULL){
        LOGE("%s: Histogram data NULL", __FUNCTION__);
        return;
    }

    if (hist->c0)
        free(hist->c0);
    if (hist->c1)
        free(hist->c1);
    if (hist->c2)
        free(hist->c2);
    if (hist->extra_info)
        free(hist->extra_info);
}

static int read_hist(int32_t fb, struct mdp_histogram_data *hist)
{
    int32_t ret;

    if(hist == NULL){
        LOGE("%s: Histogram data is null!", __FUNCTION__);
        return -1;
    }

    ret = ioctl(fb, MSMFB_HISTOGRAM, hist);
    if ((ret != 0) && (errno != ENODATA) && (errno != ETIMEDOUT)
        && (errno != EPERM) && (errno != EPROTO))
        /* ENODATA or ETIMEDOUT indicates a valid histogram failure */
        LOGD("%s: MSMFB_HISTOGRAM failed: %d:%s", __FUNCTION__,
                                                    errno, strerror(errno));

    if (ret == 0)
        return 0;
    else
        return -errno;
}

static int32_t copyHistogram(struct mdp_histogram_data *hist, uint32_t *outhist)
{

    uint32_t offset, size;
    int32_t ret = 0;

    offset = hist->bin_cnt;
    size = hist->bin_cnt * 4;
    switch (gHwInfo.nPriDisplayHistComp) {
    case 3:
        memcpy(outhist + offset * 2, hist->c2, size);
    case 2:
        memcpy(outhist + offset, hist->c1, size);
    case 1:
        memcpy(outhist, hist->c0, size);
        break;
    default:
        ret = -1;
    }
    return ret;
}

static void conv3channel(struct fb_cmap *cmap, uint32_t *outmap)
{
    int32_t i;
    __u16 *r, *g, *b;

    if ((cmap == NULL) || (cmap == NULL)) {
        LOGE("%s: Invalid parameters passed", __FUNCTION__);
        return;
    }

    /* map LUT */
    r = cmap->red;
    g = cmap->green;
    b = cmap->blue;
    for (i = 0; i < BL_LUT_SIZE; i++) {
        r[i] = (outmap[i] & 0xFF);
        g[i] = ((outmap[i] >> 8) & 0xFF);
        b[i] = ((outmap[i] >> 16) & 0xFF);
    }
    return;

}

static int32_t outCmap(int32_t fb, uint32_t *outcmap)
{
    struct msmfb_mdp_pp lut;
    struct fb_cmap cmap;
    uint32_t *cmap_data;
    int32_t i, ret = -1;

    switch (gHwInfo.nPriDisplayHistBins) {
    default:
        initialize_cmap(&cmap);
        conv3channel(&cmap, outcmap);
        lut.op = mdp_op_lut_cfg;
        lut.data.lut_cfg_data.lut_type = mdp_lut_rgb;
        lut.data.lut_cfg_data.data.rgb_lut_data.lut_type = mdp_rgb_lut_hist;
        lut.data.lut_cfg_data.data.rgb_lut_data.flags = MDP_PP_OPS_WRITE |
                                                        MDP_PP_OPS_ENABLE;
        lut.data.lut_cfg_data.data.rgb_lut_data.cmap = cmap;
        break;
    case 256:
        cmap_data = (uint32_t *)malloc(BL_LUT_SIZE * sizeof(uint32_t));
        if (!cmap_data)
            goto err_mem;

        for (i = 0; (uint32_t)i < BL_LUT_SIZE; i++)
            cmap_data[i] = outcmap[i] & 0xFF;

        lut.op = mdp_op_lut_cfg;
        lut.data.lut_cfg_data.lut_type = mdp_lut_hist;
        lut.data.lut_cfg_data.data.hist_lut_data.block =
                                                gHwInfo.nPriDisplayHistBlock;
        lut.data.lut_cfg_data.data.hist_lut_data.ops = MDP_PP_OPS_WRITE |
                                                        MDP_PP_OPS_ENABLE;
        lut.data.lut_cfg_data.data.hist_lut_data.len = BL_LUT_SIZE;
        lut.data.lut_cfg_data.data.hist_lut_data.data = cmap_data;
        break;
    }

    ret = ioctl(fb, MSMFB_MDP_PP, &lut);
err_mem:
    free_cmap(&cmap);
    if (cmap_data)
        free(cmap_data);
    return ret;
}

int calculateDiffALS(int currALS, int prevALS){
    int ret = 0;
    float calcVal;
    if (!prevALS)
        return (currALS > 0) ? 1 : 0;
    calcVal = (float) (prevALS - currALS);
    calcVal = fabsf(calcVal / ((float)prevALS));
    if(calcVal > SVI_ALS_RATIO_THR)
        return 1;
    if (abs(prevALS - currALS) > SVI_ALS_LIN_THR)
        ret = 1;
    return ret;
}

static void initialize_cmap(struct fb_cmap *cmap)
{

    if(cmap == NULL){
        LOGE("%s: Colormap struct NULL", __FUNCTION__);
        return;
    }

    cmap->red = 0;
    cmap->green = 0;
    cmap->blue = 0;
    cmap->transp = 0;

    cmap->start = 0;
    cmap->len = BL_LUT_SIZE;
    cmap->red = (__u16 *)malloc(cmap->len * sizeof(__u16));
    if (!cmap->red) {
        LOGE("%s: can't malloc cmap red!", __FUNCTION__);
        goto fail_rest1;
    }

    cmap->green = (__u16 *)malloc(cmap->len * sizeof(__u16));
    if (!cmap->green) {
        LOGE("%s: can't malloc cmap green!", __FUNCTION__);
        goto fail_rest2;
    }

    cmap->blue = (__u16 *)malloc(cmap->len * sizeof(__u16));
    if (!cmap->blue) {
        LOGE("%s: can't malloc cmap blue!", __FUNCTION__);
        goto fail_rest3;
    }

    return;

fail_rest3:
    free(cmap->blue);
fail_rest2:
    free(cmap->green);
fail_rest1:
    free(cmap->red);
}

void AbaContext::FilteredSignal(){
    pthread_mutex_lock(&mAbaLock);
    if(mSignalToWorker == WAIT_SIGNAL) {
        if (pRefresher){
            LOGD_IF(mDebug, "%s: Calling ScreenRefresher->Refresh", __func__);
            pRefresher->Refresh(1,16);
        }
        pthread_cond_signal(&mAbaCond);
    }
    mSignalToWorker = NO_WAIT_SIGNAL;
    pthread_mutex_unlock(&mAbaLock);
}

void AbaContext::initHW() {
    uint32_t i,j;
    mAbaHwInfo.uHistogramBins = gHwInfo.nPriDisplayHistBins;
    mAbaHwInfo.uHistogramComponents = gHwInfo.nPriDisplayHistComp;
    mAbaHwInfo.uBlock = gHwInfo.nPriDisplayHistBlock;
    mAbaHwInfo.uLUTSize = BL_LUT_SIZE;
    mAbaHwInfo.uFactor = HIST_PROC_FACTOR;

    i = mAbaHwInfo.uHistogramBins;
    j = 0;
    while (i > 0) {
        j += (i&0x1)?1:0;
        i = i >> 1;
    }

    if ((mAbaHwInfo.uHistogramBins <= 256) && (1 == j)) {
        i = mAbaHwInfo.uHistogramBins;
        do {
            i = i >> 1;
            mAbaHwInfo.uFactor--;
        } while (i > 1);
    }

    mInputHistogram = (uint32_t *)malloc(mAbaHwInfo.uHistogramBins *
        mAbaHwInfo.uHistogramComponents * sizeof(uint32_t));
    if (!mInputHistogram) {
        LOGE("%s: can't malloc mInputHistogram!", __FUNCTION__);
        goto fail_hist;
    }

    minLUT = (uint32_t *) malloc((mAbaHwInfo.uLUTSize) * sizeof(uint32_t));
    if (!minLUT) {
        LOGE("%s: can't malloc minLUT!", __FUNCTION__);
        goto fail_hist;
    }

    hist.c0 = 0;
    hist.c1 = 0;
    hist.c2 = 0;
    hist.block = mAbaHwInfo.uBlock;
    hist.bin_cnt = mAbaHwInfo.uHistogramBins;
    hist.c0 = (uint32_t *)malloc(hist.bin_cnt * sizeof(uint32_t));
    if (!hist.c0) {
        LOGE("%s: can't malloc red cmap!", __FUNCTION__);
        goto fail_hist;
    }

    hist.c1 = (uint32_t *)malloc(hist.bin_cnt * sizeof(uint32_t));
    if (!hist.c1) {
        LOGE("%s: can't malloc green cmap!", __FUNCTION__);
        goto fail_hist;
    }

    hist.c2 = (uint32_t *)malloc(hist.bin_cnt * sizeof(uint32_t));
    if (!hist.c2) {
        LOGE("%s: can't malloc blue cmap!", __FUNCTION__);
        goto fail_hist;
    }

    hist.extra_info = (uint32_t *)malloc(2 * sizeof(uint32_t));
    if (!hist.extra_info) {
        LOGE("%s: can't malloc extra info!", __FUNCTION__);
        goto fail_hist;
    }
    return;

fail_hist:
    free_hist(&hist);

    if (mInputHistogram)
        free(mInputHistogram);
    if (minLUT)
        free(minLUT);

    return;
}

int AbaContext::is_backlight_modified(uint32_t *old_lvl)
{
    int ret;
    uint32_t temp_lvl = get_backlight_level();

    if (temp_lvl != *old_lvl) {
        ret = 1;
        LOGD_IF(mCablDebug, "%s: The BL level changed,", __FUNCTION__);
        *old_lvl = temp_lvl;
        LOGD_IF(mCablDebug, "%s: The BL level changed to %u", __FUNCTION__,
            temp_lvl);

        /* Reset the orig level only if > than the min level */
        if (temp_lvl >= mCablOEMParams.bl_min_level) {
            orig_level = temp_lvl;
            ret = AbaSetOriginalBacklightLevel(pHandle, orig_level);
            //set the new restore level
            mUserBLLevel = temp_lvl;
        }
    } else {
        ret = 0;
    }
    return ret;
}

int32_t AbaContext::auto_adjust_quality_lvl(){
    int32_t result = 0;
    char lvl[MAX_CMD_LEN];
    char property[PROPERTY_VALUE_MAX];
    if (property_get(YUV_INPUT_STATE_PROP, property, NULL) > 0) {
        if ((atoi(property) == 1) && (mQualityLevel != mCablOEMParams.video_quality_lvl)) {
            mQualityLevel = mCablOEMParams.video_quality_lvl;
            LOGD_IF(mCablDebug, "%s: Power saving level: %d", __FUNCTION__, mQualityLevel);
            pthread_mutex_lock(&mAbaLock);
            result = AbaSetQualityLevel(pHandle, (AbaQualityLevelType) mQualityLevel);
            if( result != ABA_STATUS_SUCCESS)
                LOGE("AbaSetQualityLevel failed with status = %d", result);
            pthread_mutex_unlock(&mAbaLock);
        }else if ((atoi(property) == 0) && (mQualityLevel != mCablOEMParams.ui_quality_lvl)) {
            mQualityLevel = mCablOEMParams.ui_quality_lvl;
            LOGD_IF(mCablDebug, "%s: Power saving level: %d", __FUNCTION__, mQualityLevel);
            pthread_mutex_lock(&mAbaLock);
            result = AbaSetQualityLevel(pHandle, (AbaQualityLevelType) mQualityLevel);
            if( result != ABA_STATUS_SUCCESS)
                LOGE("AbaSetQualityLevel failed with status = %d", result);
            pthread_mutex_unlock(&mAbaLock);
        }
    }
    return result;
}

/* Single ABA Worker for CABL and SVI */
void AbaContext::ProcessAbaWorker() {

    AbaStatusType retval;
    AbaStateType CablState;
    AbaStateType SVIState;

    uint32_t old_level;
    uint32_t set_ratio;
    int32_t ret;
    uint32_t tmp_ALS = 1, out_ALS = 0, prop_ALS = 0;
    bool32 IsConverged = 0;
    char property[PROPERTY_VALUE_MAX];

    set_ratio = mCablOEMParams.bl_min_level;
    LOGD_IF(mDebug, "%s(): Entering ", __func__);

    pthread_mutex_lock(&mAbaLock);
    if(!mWorkerRunning)
        mWorkerRunning = true;
    else{
        pthread_mutex_unlock(&mAbaLock);
        goto exit_cleanup;
    }
    pthread_mutex_unlock(&mAbaLock);

    pthread_mutex_lock(&mAbaCtrlLock);
    if (IsABAStatusON(ABA_FEATURE_SVI) &&
                                (mALSUpdateThreadStatus == ALS_UPDATE_OFF))
            StartALSUpdaterThread();
    pthread_mutex_unlock(&mAbaCtrlLock);

    pRefresher = ScreenRefresher::getInstance();
    if (pRefresher){
        LOGD_IF(mDebug, "%s: Calling ScreenRefresher->Control(true)", __func__);
        pRefresher->Control(1);
    }

    old_level = mUserBLLevel;
    set_ratio = BL_SCALE_MAX;

    LOGD_IF(mDebug, "%s: Starting worker thread", __FUNCTION__);

    while (true) {

        pthread_mutex_lock(&mAbaCtrlLock);
        if (!IsABAStatusON(ABA_FEATURE_CABL | ABA_FEATURE_SVI)) {
            pthread_mutex_unlock(&mAbaCtrlLock);
            break;
        }
        pthread_mutex_unlock(&mAbaCtrlLock);
        LOGD_IF(mDebug, "%s: In outer", __FUNCTION__);

        pthread_mutex_lock(&mAbaLock);
        if (mHistStatus == 0) {
            mSignalToWorker = WAIT_SIGNAL;
            LOGD_IF(mDebug, "%s: Waiting for signal", __FUNCTION__);
            pthread_cond_wait(&mAbaCond, &mAbaLock);
            LOGD_IF(mDebug, "%s: AbaWorker is signalled", __FUNCTION__);
        }
        pthread_mutex_unlock(&mAbaLock);

        while (true) {

            pthread_mutex_lock(&mAbaCtrlLock);
            if (!IsABAStatusON(ABA_FEATURE_CABL | ABA_FEATURE_SVI)) {
                pthread_mutex_unlock(&mAbaCtrlLock);
                break;
            }

            //Read State for current iteration
            AbaGetState((pHandle), &CablState, ABA_FEATURE_CABL);
            AbaGetState((pHandle), &SVIState, ABA_FEATURE_SVI);

            //Print active information for CABL/SVI
            if ((CablState == ABA_STATE_ACTIVE) &&
                                    (SVIState == ABA_STATE_ACTIVE))
                LOGD_IF(mDebug, "%s: Both SVI and CABL active", __FUNCTION__);
            else if (CablState == ABA_STATE_ACTIVE)
                LOGD_IF(mDebug, "%s: CABL is only active", __FUNCTION__);
            else if (SVIState == ABA_STATE_ACTIVE)
                LOGD_IF(mDebug, "%s: SVI is only active", __FUNCTION__);

            //Cabl state processing
            switch (CablState) {
                case ABA_STATE_ACTIVE:
                    if(AUTO_QL_MODE == mCablOEMParams.default_ql_mode) {
                        ret = auto_adjust_quality_lvl();
                        if (ret)
                            LOGE("%s: adjust_quality_level failed",
                                                            __FUNCTION__);
                    }
                    break;
                case ABA_STATE_DISABLING:
                    LOGD_IF(mDebug, "%s: CABL is disabling", __FUNCTION__);
                    if (pRefresher)
                        pRefresher->Refresh(1, 16);
                    break;
                case ABA_STATE_DISABLED:
                    if (IsABAStatusON(ABA_FEATURE_CABL))
                        SetABAStatusOFF(ABA_FEATURE_CABL);
                    break;
                case ABA_STATE_INACTIVE:
                    break;
                default:
                    LOGD_IF(mDebug, "%s: Unknown CABL state", __FUNCTION__);
            }

            //SVI state processing
            switch (SVIState) {
                case ABA_STATE_ACTIVE:
                    if (mALSUpdateThreadStatus == ALS_UPDATE_OFF)
                        StartALSUpdaterThread();
                    break;
                case ABA_STATE_DISABLING:
                    LOGD_IF(mDebug, "%s: SVI is disabling", __FUNCTION__);
                    if (pRefresher)
                        pRefresher->Refresh(1, 16);
                    break;
                case ABA_STATE_DISABLED:
                    if (IsABAStatusON(ABA_FEATURE_SVI))
                        SetABAStatusOFF(ABA_FEATURE_SVI);
                    pthread_mutex_unlock(&mAbaCtrlLock);
                    StopALSUpdaterThread();
                    pthread_mutex_lock(&mAbaCtrlLock);
                    break;
                case ABA_STATE_INACTIVE:
                    break;
                default:
                    LOGD_IF(mDebug, "%s: Unknown SVI state", __FUNCTION__);
            }

            if (!IsABAStatusON(ABA_FEATURE_CABL | ABA_FEATURE_SVI)) {
                pthread_mutex_unlock(&mAbaCtrlLock);
                break;
            }
            pthread_mutex_unlock(&mAbaCtrlLock);

            pthread_mutex_lock(&mAbaLock);
            if (mHistStatus == 0) {
                pthread_mutex_unlock(&mAbaLock);
                break;
            }
            pthread_mutex_unlock(&mAbaLock);

            if (mDebug) {
                if (property_get("debug.svi.als", property, 0) > 0)
                    prop_ALS = atoi(property);
            }
            if (prop_ALS) {
                tmp_ALS = prop_ALS;
                mLastALSValue = prop_ALS;
            } else {
                pthread_mutex_lock(&mALSUpdateLock);
                tmp_ALS = mCurrALSValue;
                mLastALSValue = tmp_ALS;
                pthread_mutex_unlock(&mALSUpdateLock);
            }

            retval = AbaSetAmbientLightLevel(pHandle, tmp_ALS, &out_ALS);
            if(retval != ABA_STATUS_SUCCESS) {
                LOGE("%s: AbaSetAmbientLightLevel() failed with ret = %d",
                    __FUNCTION__, retval);
                continue;
            }
            LOGD_IF(mDebug, "%s: Input ALS = %u, Output ALS = %u", __func__,
                tmp_ALS, out_ALS);

            pthread_mutex_lock(&mAbaLock);
            if (mHistStatus == 1){
                is_backlight_modified(&old_level);
                ret = read_hist(FBFd, &hist);
                pthread_mutex_unlock(&mAbaLock);
                if (ret != 0) {
                    /* If read mentions protocol error then histogram is
                     * actually halted in kernel but we are still trying to read
                     * This is possible in MDP3 scenario where stop is called
                     * from control off and then clk regulators are turned off
                     */
                    if (ret == -EPROTO) {
                        pthread_mutex_lock(&mAbaLock);
                        mHistStatus = 0;
                        pthread_mutex_unlock(&mAbaLock);
                        break;
                    }
                    continue;
                } else if (mDebug >= 2)
                    print_hist(&hist);

                if(copyHistogram(&hist, mInputHistogram))
                    LOGE("%s: Copy Histogram failed", __FUNCTION__);
                LOGD_IF(mDebug,"%s: Backlight value= %d", __FUNCTION__,mBl_lvl);
            } else {
                pthread_mutex_unlock(&mAbaLock);
                /* Since mHistStatus is not 1 we break before calling Process */
                break;
            }

            retval = AbaProcess(pHandle, mInputHistogram, minLUT, &mBl_lvl);
            if (retval != ABA_STATUS_SUCCESS) {
                LOGE("%s: AbaProcess failed ret = %d", __FUNCTION__, retval);
                continue;
            }

            if (mBl_lvl != set_ratio) {
                LOGD_IF(mCablDebug, "User level = %03d Set Ratio = %03u \
                    Scale Ratio = %04u", mUserBLLevel, set_ratio, mBl_lvl);

                pthread_mutex_lock(&mAbaCtrlLock);
                if(IsABAStatusON(ABA_FEATURE_CABL)){
                    if (is_backlight_modified(&old_level)) {
                        pthread_mutex_unlock(&mAbaCtrlLock);
                        continue;
                    }
                }
                pthread_mutex_unlock(&mAbaCtrlLock);

                set_backlight_scale(mBl_lvl, mCablOEMParams.bl_min_level);
                set_ratio = mBl_lvl;
            }

            retval = AbaIsConverged(pHandle, &IsConverged);
            if(retval != ABA_STATUS_SUCCESS)
                IsConverged = 0;

            if(!IsConverged) {
                LOGD_IF(mDebug, "Updating LUT and calling Refresh!");
                ret = outCmap(FBFd, minLUT);
                if (ret != 0) {
                    LOGE("%s: outCmap() failed", __FUNCTION__);
                    continue;
                } else if (mDebug >= 2) {
                    /* i.   Print LUT/HIST
                     * ii.  Print CheckSum
                     * iii. Write CheckSum to a property
                     */
                    uint64_t uChecksum;
                    char sCheckSum[64]={0};

                    print_lut(minLUT);
                    AbaGetProcessInfo(pHandle,&uChecksum, sizeof(uChecksum),
                          ABA_FEATURE_SVI);
                    LOGD("***** CheckSum = 0X%X%X", (uint32_t)(uChecksum >> 32),
                          (uint32_t)(uChecksum& 0x00000000ffffffff));
                    LOGD("LuxInput = %d [0-65535], uUserContrastStrength = %d"\
                          "[0 255], uCurrentBrightnessStrength = %d [0 255],"\
                                     "uTemporalTransitionTime = %d [0 127]",
                          (uint32_t)((uChecksum & 0xffff00000000) >> 32),
                          (uint32_t)((uChecksum & 0xff0000) >> 16),
                          (uint32_t)((uChecksum & 0xff00) >> 8),
                          (uint32_t)(uChecksum & 0xff));
                    snprintf(sCheckSum, MAX_CHECKSUM_LENGTH, "0X%X%X", (uint32_t)(uChecksum >> 32),
                          (uint32_t)(uChecksum& 0x00000000ffffffff));
                    property_set(SVI_CHECKSUM_PROP, sCheckSum);
                }

                if (pRefresher)
                    pRefresher->Refresh(1,16);
            }
        }
    }

exit_cleanup:

    /* Cleanup  */
    if (pRefresher){
        LOGD_IF(mDebug, "%s: Calling ScreenRefresher->Control(0)", __func__);
        pRefresher->Control(0);
    }

    pthread_mutex_lock(&mAbaLock);
    mWorkerRunning = false;
    pthread_mutex_unlock(&mAbaLock);

    set_backlight_scale(BL_SCALE_MAX, mCablOEMParams.bl_min_level);

    //Stop the running ALS Updater Thread
    StopALSUpdaterThread();

    //Deleting instance of screen refresher
    ScreenRefresher::deleteInstance();

    LOGD_IF(mDebug, "%s(): Exiting ", __func__);
    return;
}

int AbaContext::Notify(uint32_t notification_type) {
    int32_t ret = 0;
    uint32_t level = 0;
    LOGD_IF(mDebug, "Starting %s ", __func__);
    pthread_mutex_lock(&mAbaLock);
    if (!mHistStatus && (notification_type == NOTIFY_TYPE_UPDATE ||
        notification_type == NOTIFY_TYPE_BL_UPDATE)) {
        pthread_mutex_unlock(&mAbaLock);
        level = get_backlight_level();
        if (level <= mCablOEMParams.bl_min_level) {
            LOGE_IF(mCablDebug, "New BL level %d lower than min level %u,"
                    " Skip this update for calc",
                    level, mCablOEMParams.bl_min_level);
            if(mPrevNotify == NOTIFY_TYPE_SUSPEND)
                ret = -1;
            goto exit_early;
        }
        pthread_mutex_lock(&mAbaCtrlLock);
        if (!IsABAStatusON(ABA_FEATURE_CABL | ABA_FEATURE_SVI)) {
            pthread_mutex_unlock(&mAbaCtrlLock);
            return ret;
        }
        pthread_mutex_unlock(&mAbaCtrlLock);
        LOGD_IF(mDebug, "Start notification received, start histogram");
        ret = startHistogram();
        if (0 == ret) {
            pthread_mutex_lock(&mAbaLock);
            mHistStatus = 1;
            pthread_mutex_unlock(&mAbaLock);
            LOGD_IF(mDebug, "%s: Filtered signal called from Notify Start", __FUNCTION__);
            FilteredSignal();
        } else {
            if (IsPuEnabled())
                LOGD("%s: PU is enabled, histogram cannot be started: %d", __FUNCTION__, ret);
            else
                LOGE("%s: Histogram failed to start: %d", __FUNCTION__, ret);
        }
        if (notification_type == NOTIFY_TYPE_BL_UPDATE) {
            if (pRefresher) {
                pRefresher->Control(1);
                LOGE_IF(mDebug, "%s: Calling refresh as bl updated", __func__);
                pRefresher->Refresh(1, 16);
                pRefresher->Control(0);
            }
        }
    } else if (mHistStatus == 1 &&
        (notification_type == NOTIFY_TYPE_SUSPEND ||
        notification_type == NOTIFY_TYPE_NO_UPDATE)) {
        LOGD_IF(mDebug, "Stop notification received, stop histogram");
        if(!stopHistogram(mHistStatus)){
            mHistStatus = 0;
        }
        pthread_mutex_unlock(&mAbaLock);
        if((notification_type == NOTIFY_TYPE_SUSPEND) &&
                            (mALSUpdateThreadStatus == ALS_UPDATE_ON)) {
            StopALSUpdaterThread();
        }

    } else {
        pthread_mutex_unlock(&mAbaLock);
    }
exit_early:
    mPrevNotify = notification_type;
    return ret;
}

/* SetLightSensor() should be called only when mSVIEnable is true*/
void AbaContext::SetLightSensor() {
    int ret;
    char property[PROPERTY_VALUE_MAX];

    if (property_get(SVI_SENSOR_PROP, property, 0) > 0) {
        int type = atoi(property);
        if (type == 0) {
            mLightSensor = new ALS();
        } else if (type == 1) {
            mLightSensor = new DummySensor();
        } else if (type == 2) {
            if (getNativeLightSensor(&mLightSensor)) {
                LOGE("%s: Light Sensor not acquired", __func__);
                mLightSensor = new ALS();
            }
#ifdef ALS_ENABLE
        } else if (type == 3) {
            mLightSensor = new LightSensor();
#endif
        } else {
            LOGE("Invalid choice for sensor type, initializing the default sensor class!");
            mLightSensor = new ALS();
        }
    } else {
#ifdef ALS_ENABLE
        mLightSensor  = new LightSensor();
#else
        if (getNativeLightSensor(&mLightSensor)) {
            LOGE("%s: Light Sensor not acquired", __func__);
            mLightSensor = new ALS();
        }
#endif
    }

    // Registering the ALS
    ret = mLightSensor->ALSRegister();
    if( ret ) {
        LOGE("%s(): ALSRegister() Failed : Return Value = %d", __func__, ret);
    }
}

/* CleanupLightSensor() should be called only when mSVIEnable is true*/
void AbaContext::CleanupLightSensor() {
    int ret;
    if (isNativeLightSensor()) {
        removeNativeLightSensor(&mLightSensor);
    } else {
        ret = mLightSensor->ALSCleanup();
        if( ret )
            LOGE("%s: ALSCleanup() Failed : Return Value = %d", __func__, ret);
        delete mLightSensor;
    }
}

/* StopALSUpdaterThread should be called only when mSVIEnable is true*/
void AbaContext::StopALSUpdaterThread(){

    LOGD_IF(mDebug, "%s(): Entering ", __func__);
    int ret = 0;

    pthread_mutex_lock(&mALSUpdateLock);
    if (mALSUpdateThreadStatus != ALS_UPDATE_ON)
        ret = 1;
    pthread_mutex_unlock(&mALSUpdateLock);
    if (ret)
        return;

    //Disabling ALS
    ret = mLightSensor->ALSDeRegister();
    if (ret) {
        LOGE("%s(): ALSDeRegister() Failed : Return Value = %d",__func__,ret);
    } else {
        pthread_mutex_lock(&mALSUpdateLock);
        bALSRunEnabled = false;
        pthread_mutex_unlock(&mALSUpdateLock);
    }
    //Stopping the ALS Updater thread
    pthread_mutex_lock(&mALSUpdateLock);
    mALSUpdateThreadStatus = ALS_UPDATE_DISABLING;
    //Setting mCurrALSValue to 1 for moving algo to CABL side
    pthread_mutex_unlock(&mALSUpdateLock);

    pthread_join(mALSUpdateThread, NULL);
    LOGD_IF(mDebug, "%s(): Exiting ", __func__);
}

/* StartALSUpdaterThread should be called only when mSVIEnable is true*/
void AbaContext::StartALSUpdaterThread(){

    LOGD_IF(mDebug, "%s(): Entering ", __func__);
    int ret_val;
    int32_t err;

    // Initialize and register the lightsensor
    SetLightSensor();

    //Enabling ALS
    ret_val = mLightSensor->ALSRun(false);
    if(ret_val) {
        LOGE("%s(): ALSRun() Failed : Return Value = %d",__func__, ret_val);
    } else{
        bALSRunEnabled = true;
    }

    //Checkiung if the ALS Updation thread is running or not
    pthread_mutex_lock(&mALSUpdateLock);
    if (mALSUpdateThreadStatus == ALS_UPDATE_ON){
        pthread_mutex_unlock(&mALSUpdateLock);
        goto exit_func;
    }
    pthread_mutex_unlock(&mALSUpdateLock);

    //Creating a separate thread for ALS Updation
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    /* update the Thread status */
    pthread_mutex_lock(&mALSUpdateLock);
    mALSUpdateThreadStatus = ALS_UPDATE_ON;
    pthread_mutex_unlock(&mALSUpdateLock);

    LOGD_IF(mDebug, "%s: Trying to create a new thread", __func__);
    err = pthread_create(&mALSUpdateThread, &attr, als_update_func, this);
    if (err) {
        LOGE("%s: ALSUpdate: Failed to start the updater thread", __FUNCTION__);
        mALSUpdateThreadStatus = ALS_UPDATE_OFF;
    }
    pthread_attr_destroy(&attr);

exit_func:

    LOGD_IF(mDebug, "%s(): Exiting ", __func__);
}

void AbaContext::ProcessAbaALSUpdater(){

    int ret_val, tmp_ALS = 1, prev_ALS = 1, diff_ALS = 0, worker_ALS = 0;

    LOGD_IF(mDebug, "%s(): Entering ", __func__);

    pthread_mutex_lock(&mALSUpdateLock);
    while(mALSUpdateThreadStatus == ALS_UPDATE_ON){
        if(bALSRunEnabled){
            worker_ALS = mLastALSValue;
            pthread_mutex_unlock(&mALSUpdateLock);

            tmp_ALS = mLightSensor->ALSReadSensor();
            tmp_ALS = (tmp_ALS > 0) ? tmp_ALS : 1;
            diff_ALS = calculateDiffALS(tmp_ALS, worker_ALS);
            if (diff_ALS)
                FilteredSignal();

            if (calculateDiffALS(tmp_ALS, prev_ALS)) {
                LOGD_IF(mDebug, "%s: New ALS value = %d", __func__, tmp_ALS);
                prev_ALS = tmp_ALS;
                pthread_mutex_lock(&mALSUpdateLock);
                mCurrALSValue = tmp_ALS;
                pthread_mutex_unlock(&mALSUpdateLock);
            }

        } else {
            pthread_mutex_unlock(&mALSUpdateLock);
            ret_val = mLightSensor->ALSRun(false);
            if(ret_val) {
                LOGE("%s(): ALSRun() Failed : Return Value = %d", __func__,
                    ret_val);
                pthread_mutex_lock(&mALSUpdateLock);
                continue;
            }
            bALSRunEnabled = true;
        }
        pthread_mutex_lock(&mALSUpdateLock);
    }
    pthread_mutex_unlock(&mALSUpdateLock);

    pthread_mutex_lock(&mALSUpdateLock);
    mALSUpdateThreadStatus = ALS_UPDATE_OFF;
    mCurrALSValue = 1;
    pthread_mutex_unlock(&mALSUpdateLock);

    pthread_mutex_lock(&mAbaCtrlLock);
    if (!IsABAStatusON(ABA_FEATURE_SVI))
        CleanupLightSensor();
    pthread_mutex_unlock(&mAbaCtrlLock);

    LOGD_IF(mDebug, "%s(): Exiting ", __func__);
    pthread_exit(NULL);
}

int32_t AbaContext::CABLControl(bool bEnable) {
    AbaStatusType retval;
    LOGD_IF(mCablDebug, "Start_CABL E");
    pthread_mutex_lock(&mAbaCtrlLock);
    if (mAbaHwInfo.uHistogramBins== 0) {
        LOGE("%s: CABL not supported on this HW!", __FUNCTION__);
        pthread_mutex_unlock(&mAbaCtrlLock);
        return -1;
    }

    if (!mCablEnable) {
        LOGE("%s: CABL not enabled!", __FUNCTION__);
        pthread_mutex_unlock(&mAbaCtrlLock);
        return -1;
    }

    if(bEnable){ //CABL ON
        if (IsABAStatusON(ABA_FEATURE_CABL)) {
            LOGD("%s: CABL is already on!", __FUNCTION__);
            pthread_mutex_unlock(&mAbaCtrlLock);
            return 0;
        }
        retval = AbaGetDefaultParams(eCABLConfig, &mCablOEMParams,
            (uint32_t) sizeof(mCablOEMParams));
        if (retval != ABA_STATUS_SUCCESS) {
            LOGE("%s: GetDefaultParams Failed retval = %d", __FUNCTION__, retval);
            pthread_mutex_unlock(&mAbaCtrlLock);
            return retval;
        }

        /* oem specific initialization */
        mCABLCP.ConfigureCABLParameters(&mCablOEMParams);

        /* driver initialization */
        retval = AbaInit(eCABLConfig, &mCablOEMParams,
            (uint32_t) sizeof(mCablOEMParams), &mAbaHwInfo, pHandle);
        if (retval != ABA_STATUS_SUCCESS) {
            LOGE("%s: AbaInit() Failed retval = %d", __FUNCTION__, retval);
            pthread_mutex_unlock(&mAbaCtrlLock);
            return retval;
        }

        /* Get the backlight level and initialize the new level to it */
        mUserBLLevel = get_backlight_level();
        /* Following api is called only for CABL*/
        retval = AbaSetOriginalBacklightLevel(pHandle, mUserBLLevel);
        if (retval != ABA_STATUS_SUCCESS) {
            LOGE("%s: AbaSetOriginalBacklightLevel Failed retval = %d",
                __FUNCTION__, retval);
            pthread_mutex_unlock(&mAbaCtrlLock);
            return retval;
        }

        /* Get the user defined power saving level */
        char property[PROPERTY_VALUE_MAX];
        if (property_get("hw.cabl.level", property, NULL) > 0) {
            CABLQualityLevelType newLvl = (CABLQualityLevelType) ql_string2int(property);
            if (newLvl == ABL_QUALITY_AUTO) {
                mQualityLevel = ABL_QUALITY_HIGH;
            } else if ((newLvl < 0) || (newLvl > ABL_QUALITY_MAX)) {
                mQualityLevel = (CABLQualityLevelType) ABA_QUALITY_HIGH;
                LOGE("%s: Invalid power saving level, setting hw.cabl.level to High",
                        __FUNCTION__);
            } else if (newLvl < ABL_QUALITY_MAX) {
                mQualityLevel = (CABLQualityLevelType) newLvl;
            }
            LOGD_IF(mCablDebug, "Quality level: %d", mQualityLevel);
        }

        retval = AbaSetQualityLevel(pHandle,(AbaQualityLevelType) mQualityLevel);
        if (retval != ABA_STATUS_SUCCESS) {
            LOGE("%s: AbaSetQualityLevel(CABL) failed retval = %d", __FUNCTION__,
                retval);
            pthread_mutex_unlock(&mAbaCtrlLock);
            return retval;
        }

        retval = AbaActivate(pHandle,ABA_FEATURE_CABL);
        if (retval != ABA_STATUS_SUCCESS) {
            LOGE("%s: AbaActivate() failed retval = %d", __FUNCTION__, retval);
            pthread_mutex_unlock(&mAbaCtrlLock);
            return retval;
        }

        /* update the CABL status so threads can run */
        SetABAStatusON(ABA_FEATURE_CABL);
        LOGD_IF(mCablDebug, "Start_CABL sets CABL STATUS TO CABL_ON");
        /* Check if worker thread exists else start*/
        if(!mWorkerRunning){
            int err = 0;
            pthread_mutex_lock(&mAbaLock);
            if (!mHistStatus)
                err = startHistogram();
            if (0 == err)
                mHistStatus = 1;
            pthread_mutex_unlock(&mAbaLock);
            /* startHistogram failed */
            if (err) {
                if (IsPuEnabled()) {
                    LOGD("%s: PU is enabled, histogram cannot be started: %d", __FUNCTION__, err);
                    SetABAStatusOFF(ABA_FEATURE_CABL);
                    pthread_mutex_unlock(&mAbaCtrlLock);
                    return err;
                } else {
                    LOGE("%s: CABL: Histogram failed to start: = %d", __FUNCTION__, err);
                }
            }
            pthread_attr_t attr;
            pthread_attr_init(&attr);
            pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
            err = pthread_create(&mWorkerThread, &attr, aba_worker_func, this);
            if (err) {
                LOGE("%s: CABL: Failed to start worker thread", __FUNCTION__);
                SetABAStatusOFF(ABA_FEATURE_CABL);
                pthread_attr_destroy(&attr);
                pthread_mutex_unlock(&mAbaCtrlLock);
                return -1;
            }
            pthread_attr_destroy(&attr);
        }
        LOGD_IF(mCablDebug, "Start_CABL X");
        pthread_mutex_unlock(&mAbaCtrlLock);
        return 0;

    } else { //CABL Stop
        LOGD_IF(mCablDebug, "Stop_CABL E");
        if (!IsABAStatusON(ABA_FEATURE_CABL)) {
            LOGD("%s: CABL is already off!", __FUNCTION__);
            pthread_mutex_unlock(&mAbaCtrlLock);
            return 0;
        }
        AbaDisable(pHandle, ABA_FEATURE_CABL);
        pthread_mutex_lock(&mAbaLock);
        if (mSignalToWorker == WAIT_SIGNAL) {
            pthread_cond_signal(&mAbaCond);
        }
        mSignalToWorker = NO_WAIT_SIGNAL;
        pthread_mutex_unlock(&mAbaLock);
        LOGD_IF(mCablDebug, "Stopped CABL X");
    }
    pthread_mutex_unlock(&mAbaCtrlLock);
    return 0;
}

int AbaContext::SVIControl(bool bEnable) {
    int ret = -1;
    AbaStatusType retval;
    pthread_mutex_lock(&mAbaCtrlLock);

    if (bEnable) {
        if (IsABAStatusON(ABA_FEATURE_SVI)) {
            LOGD(" %s %d SVI is already on", __FUNCTION__, __LINE__);
            pthread_mutex_unlock(&mAbaCtrlLock);
            return ret;
        }

        if (mSVIEnable) {
            pthread_mutex_lock(&mAbaLock);
            SetABAStatusON(ABA_FEATURE_SVI);
            pthread_mutex_unlock(&mAbaLock);

            retval = AbaGetDefaultParams(eSVIConfig, &mSVIOEMParams,
                                        (uint32_t) sizeof(mSVIOEMParams));
            if (retval != ABA_STATUS_SUCCESS) {
                LOGE("%s: GetDefaultParams Failed retval = %d", __FUNCTION__, retval);
                pthread_mutex_unlock(&mAbaCtrlLock);
                return retval;
            }

            /* oem specific initialization */
            mSVICP.ConfigureSVIParameters(&mSVIOEMParams);

            /* driver initialization */
            retval = AbaInit(eSVIConfig, &mSVIOEMParams,
                    (uint32_t) sizeof(mSVIOEMParams), &mAbaHwInfo, pHandle);
            if (retval != ABA_STATUS_SUCCESS) {
                LOGE("%s: AbaInit() Failed retval = %d", __FUNCTION__, retval);
                pthread_mutex_unlock(&mAbaCtrlLock);
                return retval;
            }

            retval = AbaActivate(pHandle, ABA_FEATURE_SVI);
            if (retval != ABA_STATUS_SUCCESS) {
                LOGE("%s: AbaActivate(SVI) failed retval = %d", __FUNCTION__, retval);
                pthread_mutex_unlock(&mAbaCtrlLock);
                return retval;
            } else {
                LOGD_IF(mSVIDebug, "%s: Activated SVI feature", __FUNCTION__);
            }

            /* Check if worker thread is running else start */
            if(!mWorkerRunning){
                pthread_mutex_lock(&mAbaLock);
                if (!mHistStatus)
                    ret = startHistogram();
                if (0 == ret)
                    mHistStatus = 1;
                pthread_mutex_unlock(&mAbaLock);
                /* startHistogram failed*/
                if (ret) {
                    if (IsPuEnabled()) {
                        LOGD("%s: PU is enabled, histogram cannot be started: %d", __FUNCTION__, ret);
                        SetABAStatusOFF(ABA_FEATURE_SVI);
                        pthread_mutex_unlock(&mAbaCtrlLock);
                        return ret;
                    } else {
                        LOGE("%s: SVI: Histogram faile to start: %d", __FUNCTION__, ret);
                    }
                }
                pthread_attr_t attr;
                pthread_attr_init(&attr);
                pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
                ret = pthread_create(&mWorkerThread, &attr, aba_worker_func, this);
                if (ret) {
                    SetABAStatusOFF(ABA_FEATURE_SVI);
                    LOGE("%s:  Failed to start the worker thread", __FUNCTION__);
                    pthread_attr_destroy(&attr);
                    pthread_mutex_unlock(&mAbaCtrlLock);
                    return ret;
                }
                pthread_attr_destroy(&attr);
            }
        } else {
            LOGE("%s: SVI is not enabled!", __FUNCTION__);
        }
    } else {
        if (!IsABAStatusON(ABA_FEATURE_SVI)) {
            LOGD("%s: SVI is already off!", __FUNCTION__);
            pthread_mutex_unlock(&mAbaCtrlLock);
            return ret;
        }
        if (IsABAStatusON(ABA_FEATURE_SVI))
            AbaDisable(pHandle,ABA_FEATURE_SVI);
        pthread_mutex_unlock(&mAbaCtrlLock);
        pthread_mutex_lock(&mAbaLock);
        if (mSignalToWorker == WAIT_SIGNAL) {
            pthread_cond_signal(&mAbaCond);
        }
        mSignalToWorker = NO_WAIT_SIGNAL;
        pthread_mutex_unlock(&mAbaLock);
        pthread_mutex_lock(&mAbaCtrlLock);
    }
    pthread_mutex_unlock(&mAbaCtrlLock);
    return 0;
}
