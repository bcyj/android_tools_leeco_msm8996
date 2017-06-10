/*
 * Copyright (c) 2012-2015 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef _POSTPROC_H
#define _POSTPROC_H

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

#include "PPDaemonUtils.h"

#define CMD_PP_ON "pp:on"
#define CMD_PP_OFF "pp:off"
#define CMD_PP_SET_HSIC "pp:set:hsic"
#define CMD_POSTPROC_STATUS "pp:query:status:postproc"
#define PP_CFG_FILE_PROP "hw.pp.cfg"
#define PP_CFG_FILE_PATH "/data/misc/display/pp_data.cfg"

enum pp_status {
    PP_OFF = false,
    PP_ON  = true
};

union display_pp_cfg {
    struct display_pp_pa_cfg pa_cfg;
    struct display_pp_pa_v2_cfg pa_v2_cfg;
    struct display_pp_conv_cfg conv_cfg;
};

class PostProc {
public:
    bool mEnable;
    bool mDebug;
    bool mStatus;
    uint32_t mBlockType;
    struct display_hw_info mHWInfo;
    display_pp_cfg mCurCfg;
    display_pp_cfg mNewCfg;
    PostProc() : mEnable(false), mDebug(false), mStatus(PP_OFF) {
        char property[PROPERTY_VALUE_MAX];
        if (property_get("debug.pp.logs", property, 0) > 0 && (atoi(property) == 1)) {
            mDebug = true;
        }
    }
    int get_saved_hsic_config();
    void print_values(const char *, display_pp_cfg *);
    void copy_config();
    void init_cc_matrix();
    int32_t compare_config();
    int32_t start_pp();
    int32_t stop_pp();
    int32_t set_hsic(float, float, int, float);
    int write_hsic_values();
    int parse_pa_data(float* hue, float* sat, int* intensity, float* contrast);
    int save_pa_data(float hue, float sat, int intensity, float contrast);
    int processPPDataFile();
};


#endif /* _POSTPROC_H */
