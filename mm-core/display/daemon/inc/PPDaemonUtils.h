/*
 * Copyright (c) 2012-2015 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef _PPDAEMONUTILS_H
#define _PPDAEMONUTILS_H

#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <cutils/properties.h>
#include <cutils/log.h>

#include "common_log.h"
#include <cutils/sockets.h>
#include <linux/msm_mdp.h>
#include <linux/fb.h>

#include "lib-postproc.h"
#include "abl_driver.h"
#include "als.h"

#define UINT16_MAX 65536
#define MAX_DBG_MSG_LEN 6000

#define MAX_BACKLIGHT_LEN 12

#define MAX_FB_NAME_LEN 128

#define MAX_CMD_LEN 4096
#define SHORT_CMD_LEN 64

#define YUV_INPUT_STATE_PROP "hw.cabl.yuv"

#ifndef LOGE_IF
#define LOGE_IF(cond, ...) \
    ( (CONDITION(cond)) ? ((void)LOG(LOG_ERROR, LOG_TAG, __VA_ARGS__)) : (void)0 )
#endif
#ifndef LOGD_IF
#define LOGD_IF(cond, ...) \
    ( (CONDITION(cond)) ? ((void)LOG(LOG_INFO, LOG_TAG, __VA_ARGS__)) : (void)0 )
#endif

#define LOG_TAG "PPDaemon"
#define LOG_NDDEBUG 0
#define SUCCESS 0
#define FAILED -1
#define HDMI_PRIMARY_NODE  "/sys/class/graphics/fb0/hdmi_primary"
#define BL_SCALE_MAX (1024)

#define PARTIAL_UPDATE_SYSFS "/sys/class/graphics/fb0/dyn_pu"
#define SYSFS_FILE_LEN 2

extern display_hw_info gHwInfo;
extern int FBFd;
extern int32_t BLFd;
extern int PUFd;
extern bool PuEnabled;

void inspectHW();
bool isHDMIPrimary (void);
bool IsPuEnabled();
void print_hist(struct mdp_histogram_data *hist);
void print_lut(uint32_t *lut);
int32_t stopHistogram(int debug);
int32_t startHistogram();
int set_backlight_scale(uint32_t bl_scale, uint32_t bl_min_level);
uint32_t get_backlight_level(void);
int ql_string2int(char* c_lvl);
int tokenize_params(char *inputParams, const char *delim, const int minTokenReq,
                                        char* tokenStr[], int *idx);
int getNativeLightSensor(ALS **);
bool isNativeLightSensor();
void removeNativeLightSensor(ALS **);

#endif /* _PPDAEMONUTILS_H */
