/*
 * DESCRIPTION
 * This file holds utility functions as a PPDaemon library. Any class
 * can include this file for common interaction with pp-daemon.
 *
 * Copyright (c) 2012-2015 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 */

#include "PPDaemonUtils.h"
#include <assert.h>
#include <dlfcn.h>

display_hw_info gHwInfo;

void inspectHW() {
    if (display_pp_get_hw_revision(&gHwInfo.hwRevision))
        gHwInfo.hwRevision = MDP_HW_REV_UNKNOWN;
    if (display_pp_get_max_mixer_width(&gHwInfo.maxMixerWidth))
        gHwInfo.maxMixerWidth = DEFAULT_MAX_MIXER_WIDTH;

    if (gHwInfo.hwRevision >= MDP_HW_REV_V5_0) {
        gHwInfo.nPriDisplayHistBins = 256;
        gHwInfo.nPriDisplayHistComp = 1;
        gHwInfo.nPriDisplayHistBlock = MDP_LOGICAL_BLOCK_DISP_0;
        gHwInfo.isMDP5= 1;
    } else if (gHwInfo.hwRevision >= MDP_HW_REV_V4_2) {
        gHwInfo.nPriDisplayHistBins = 128;
        gHwInfo.nPriDisplayHistComp = 3;
        gHwInfo.nPriDisplayHistBlock = MDP_BLOCK_DMA_P;
    } else if (gHwInfo.hwRevision > MDP_HW_REV_V3_0) {
        gHwInfo.nPriDisplayHistBins = 32;
        gHwInfo.nPriDisplayHistComp = 3;
        gHwInfo.nPriDisplayHistBlock = MDP_BLOCK_DMA_P;
    } else {
        gHwInfo.nPriDisplayHistBins = 0;
        gHwInfo.nPriDisplayHistComp = 0;
        gHwInfo.nPriDisplayHistBlock = 0;
    }
    assert(gHwInfo.nPriDisplayHistBins < UINT16_MAX);
}

bool IsPuEnabled() {
    bool pu_enabled = false;
    int temp = 0, ret = 0;
    char buffer[SYSFS_FILE_LEN + 1] = {0};

    if (PUFd < 0) {
        LOGD("PUFd is not open for reading %d", PUFd);
        return pu_enabled;
    }
    memset(buffer, 0, SYSFS_FILE_LEN);
    ret = pread(PUFd, buffer, SYSFS_FILE_LEN, 0);
    if (ret > 0) {
        buffer[SYSFS_FILE_LEN] = '\0';
        temp = atoi(&buffer[0]);
        LOGD("PU Status from pu node: %d", temp);
        if (!(temp == 0 || temp == 1))
            LOGE("Unsupported dyn_pu sysfs node value: %d", pu_enabled);
        else
            pu_enabled = temp;
    }
    return pu_enabled;
}

bool isHDMIPrimary (void) {

    char isPrimaryHDMI = '0';
    /* read HDMI sysfs nodes */
    FILE *fp = fopen(HDMI_PRIMARY_NODE, "r");

    if (fp) {
        fread(&isPrimaryHDMI, 1, 1, fp);
        if (isPrimaryHDMI == '1'){
            /* HDMI is primary */
            LOGD("%s: HDMI is primary display", __FUNCTION__);
            fclose(fp);
            return true;
        } else {
            /* Should never happen */
            LOGE("%s: HDMI_PRIMARY_NODE is: %c", __FUNCTION__, isPrimaryHDMI);
            fclose(fp);
            return false;
        }
    } else {
        /* HDMI_PRIMARY_NODE not present */
        LOGD("%s: HDMI is not primary display", __FUNCTION__);
        return false;
    }
}


void free_cmap(struct fb_cmap *cmap)
{

    if(cmap == NULL){
        LOGE("%s: Colormap struct NULL", __FUNCTION__);
        return;
    }

    if (cmap->red)
        free(cmap->red);
    if (cmap->green)
        free(cmap->green);
    if (cmap->blue)
        free(cmap->blue);
}

void print_hist(struct mdp_histogram_data *hist)
{
    uint32_t *h, i = 0, j = 0, buf_len = 0, sum_tot = 0, sum_loc = 0;
    char strbuf[MAX_DBG_MSG_LEN]="";
    char str[2] = {'\0', '\0'};

    if ((hist == NULL) || (hist->c0 == NULL) || (hist->c1 == NULL)
                                                || (hist->c2 == NULL)) {
        LOGE("%s: Histogram component data NULL", __FUNCTION__);
        return;
    }

    LOGD("%s: HIST Start", __FUNCTION__);
    switch (hist->bin_cnt) {
        case  32:
            // prints R histogram then G histogram and then B histogram each
            // one line containing 32 bins
            for (i = 0; i < 3; i ++) {
                if (i == 0) {
                    h = hist->c0;
                    str[0] = 'R';
                } else if (i == 1) {
                    h = hist->c1;
                    str[0] = 'G';
                } else {
                    h = hist->c2;
                    str[0] = 'B';
                }
                memset(strbuf, 0, MAX_DBG_MSG_LEN * sizeof(char));
                for (j = 0; j < 32; j++) {
                    buf_len = strlen(strbuf);
                    snprintf(strbuf + buf_len,(MAX_DBG_MSG_LEN - buf_len) *
                            sizeof(char), "%6u ,", h[j]);
                }
                LOGD("HIST %s : %s", str, strbuf);
            }
            break;
        case 256:
            // prints 16 V Histogram values per line
            h = hist->c0;
            for (i = 0; i < 256; i++)
                sum_tot += h[i];
            for (i = 0; i < 256; i += 16) {
                memset(strbuf, 0, MAX_DBG_MSG_LEN * sizeof(char));
                sum_loc = 0;
                for (j = 0; j < 16; j++) {
                    buf_len = strlen(strbuf);
                    snprintf(strbuf + buf_len, (MAX_DBG_MSG_LEN - buf_len) *
                                                sizeof(char), "%6u ", h[i + j]);
                    sum_loc += h[i + j];
                }
                LOGD("HIST: (%4u - %4u) (%3.2f%%) %s", i, i + 15,
                    ((double) sum_loc * 100) / sum_tot, strbuf);
            }
            break;
        default:
            LOGE("%s: Invalid Histogram Bin Count", __FUNCTION__);
    }
    LOGD("%s: HIST Stop", __FUNCTION__);

}

void print_lut(uint32_t *lut)
{
    uint32_t i = 0, j = 0, buf_len = 0;
    char strbuf[MAX_DBG_MSG_LEN]="";

    if (lut == NULL){
        LOGE("%s: LUT data NULL", __FUNCTION__);
        return;
    }

    // print 16 entries of LUT in one line
    LOGD("%s: LUT Start", __FUNCTION__);
    for (i = 0; i < BL_LUT_SIZE; i += 16) {
        memset(strbuf, 0, MAX_DBG_MSG_LEN * sizeof(char));
        for (j = 0; j < 16; j++) {
            buf_len = strlen(strbuf);
            snprintf(strbuf + buf_len, (MAX_DBG_MSG_LEN - buf_len) *
                                       sizeof(char), "%4u ", lut[i + j] & 0xFF);
        }
        LOGD("LUT: (%4u - %4u) %s", i, i + 15, strbuf);
    }
    LOGD("%s: LUT Stop", __FUNCTION__);
}

int32_t stopHistogram(int debug) {
    int32_t ret;
    uint32_t block = gHwInfo.nPriDisplayHistBlock;
    ret = ioctl(FBFd, MSMFB_HISTOGRAM_STOP, &block);
    if (ret < 0) {
        LOGE_IF(debug, "MSMFB_HISTOGRAM_STOP failed!");
    }
    return ret;
}

int32_t startHistogram() {
    int32_t ret;
    struct mdp_histogram_start_req req;

    req.block = gHwInfo.nPriDisplayHistBlock;
    req.frame_cnt = 1;
    req.bit_mask = 0x0;
    req.num_bins = (uint16_t) gHwInfo.nPriDisplayHistBins;

    ret = ioctl(FBFd, MSMFB_HISTOGRAM_START, &req);
    return ret;
}

int set_backlight_scale(uint32_t bl_scale, uint32_t bl_min_level) {
    int32_t ret;
    struct msmfb_mdp_pp backlight;

    backlight.op = mdp_bl_scale_cfg;
    backlight.data.bl_scale_data.min_lvl = bl_min_level;
    if (bl_scale > BL_SCALE_MAX)
        bl_scale = BL_SCALE_MAX;
    backlight.data.bl_scale_data.scale = bl_scale;
    ret = ioctl(FBFd, MSMFB_MDP_PP, &backlight);
    if (ret)
        LOGE("FAILED TO SET BACKLIGHT SCALE, %s", strerror(errno));

    return ret;
}

uint32_t get_backlight_level(void) {
    uint32_t level = 0;
    ssize_t bytes;
    char buffer[MAX_BACKLIGHT_LEN];
    memset(buffer, 0, MAX_BACKLIGHT_LEN);
    bytes = pread(BLFd, buffer, sizeof (char) * MAX_BACKLIGHT_LEN, 0);
    if (bytes > 0)
        sscanf(buffer, "%u", &level);
    else
        LOGE("BL FD read failure: bytes = %d error = %s ", bytes,
            strerror(errno));
    return level;
}

int ql_string2int(char* c_lvl){
    int ret = -1;
    if (!strcmp(c_lvl, CABL_LVL_LOW)) {
        ret = ABL_QUALITY_LOW;
    } else if (!strcmp(c_lvl, CABL_LVL_MEDIUM)) {
        ret = ABL_QUALITY_NORMAL;
    } else if (!strcmp(c_lvl, CABL_LVL_HIGH)) {
        ret = ABL_QUALITY_HIGH;
    } else if (!strcmp(c_lvl, CABL_LVL_AUTO)) {
        ret = ABL_QUALITY_AUTO;
    }
    return ret;
}

int tokenize_params(char *inputParams, const char *delim, const int minTokenReq,
                                        char* tokenStr[], int *idx){
    char *tmp_token = NULL, *inputParams_r;
    int ret = 0, index = 0;
    if (!inputParams) {
        ret = -1;
        goto err;
    }

    tmp_token = strtok_r(inputParams, delim, &inputParams_r);
    while (tmp_token != NULL) {
        tokenStr[index++] = tmp_token;
        if (index < minTokenReq) {
            tmp_token = strtok_r(NULL, delim, &inputParams_r);
        }else{
            break;
        }
    }
    *idx = index;
err:
    return ret;
}

static int als_ref_cnt = 0;
static pthread_mutex_t als_lib_lock = PTHREAD_MUTEX_INITIALIZER;
static void* als_lib_handle = NULL;

int getNativeLightSensor(ALS **pLightSensor) {
    ALS* (*getReference)() = NULL;
    int ret = 0;

    pthread_mutex_lock(&als_lib_lock);
    if (als_lib_handle == NULL)
        als_lib_handle = dlopen("libmm-als.so", RTLD_NOW);

    if (als_lib_handle)
        *(void **)&getReference = dlsym(als_lib_handle, "getSensor");
    else
        LOGE("%s: Loading libmm-als.so failed", __func__);

    if (getReference)
        *pLightSensor = getReference();
    else
        *pLightSensor = NULL;

    if (*pLightSensor) {
        als_ref_cnt++;
        LOGD("%s: Loaded libmm-als.so ref cnt = %d", __func__, als_ref_cnt);
        ret = 0;
    } else {
        LOGE("%s: Failed creating native lib sensor instance", __func__);
        ret = -EFAULT;
    }
    pthread_mutex_unlock(&als_lib_lock);

    return ret;
}

bool isNativeLightSensor() {
    bool ret = false;
    pthread_mutex_lock(&als_lib_lock);
    if (als_lib_handle != NULL)
        ret = true;
    pthread_mutex_unlock(&als_lib_lock);
    return ret;
}

void removeNativeLightSensor(ALS **pLightSensor) {
    pthread_mutex_lock(&als_lib_lock);
    if (als_ref_cnt <= 0) {
        LOGE("%s: ERROR: Refcount already non positive: %d", __func__,
                als_ref_cnt);
        pthread_mutex_unlock(&als_lib_lock);
        return;
    }
    als_ref_cnt--;
    LOGD("%s: Removing libmm-als.so ref cnt = %d", __func__, als_ref_cnt);

    if (als_ref_cnt == 0) {
        (*pLightSensor)->ALSCleanup();
        delete *pLightSensor;
        *pLightSensor = NULL;

        if (als_lib_handle) {
            dlclose(als_lib_handle);
            als_lib_handle = NULL;
            LOGD("%s: Removed libmm-als.so completely", __func__);
        }
    }
    pthread_mutex_unlock(&als_lib_lock);
}

