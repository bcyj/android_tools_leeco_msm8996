/*
 * Copyright (c) 2012-2015 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef _DAEMONCONTEXT_H
#define _DAEMONCONTEXT_H

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

#include <poll.h>

#include "AD.h"
#include "AbaContext.h"
#include "CABL.h"
#include "PostProc.h"
#include "Calib.h"

#define TOTAL_FB_NUM 3
#define PRIMARY_PANEL_TYPE_CNT 5
#define EXTERNAL_PANEL_TYPE_CNT 1
#define WRITEBACK_PANEL_TYPE_CNT 1
#define HDMI_PANEL "hdmi panel"
#define LVDS_PANEL "lvds panel"
#define DTV_PANEL "dtv panel"
#define MIPI_DSI_VIDEO_PANEL "mipi dsi video panel"
#define MIPI_DSI_CMD_PANEL "mipi dsi cmd panel"
#define WB_PANEL "writeback panel"
#define EDP_PANEL "edp panel"

#define DAEMON_SOCKET "pps"
#define CONN_TIMEOUT 300000 /* (5*60*1000) millisecs = 5 minutes */

#define CMD_DEBUG_PREFIX "debug:"
#define CMD_DEBUG_CABL_ON "debug:cabl:on"
#define CMD_DEBUG_CABL_OFF "debug:cabl:off"
#define CMD_DEBUG_AD_ON "debug:ad:on"
#define CMD_DEBUG_AD_OFF "debug:ad:off"
#define CMD_DEBUG_PP_ON "debug:pp:on"
#define CMD_DEBUG_PP_OFF "debug:pp:off"
#define CMD_DEBUG_DAEMON_ON "debug:daemon:on"
#define CMD_DEBUG_DAEMON_OFF "debug:daemon:off"

#define CMD_AD_PREFIX "ad:"
#define CMD_AD_ON "ad:on"
#define CMD_AD_OFF "ad:off"
#define CMD_AD_STATUS "ad:query:status"
#define CMD_AD_CALIB_ON "ad:calib:on"
#define CMD_AD_CALIB_OFF "ad:calib:off"
#define CMD_AD_INIT "ad:init"
#define CMD_AD_CFG "ad:config"
#define CMD_AD_INPUT "ad:input"
#define CMD_AD_ASSERTIVENESS "ad:assertiveness"
#define CMD_AD_STRLIMIT "ad:strlim"
#define CMD_BL_SET "bl:set"
#define CMD_AD_SUPPORT "ad:support"
#define CMD_AD_RANGE "ad:range"
#define CMD_AD_GET "ad:query:assertiveness"

#define CMD_SVI_PREFIX "svi:"
#define CMD_SVI_ON "svi:on"
#define CMD_SVI_OFF "svi:off"
#define CMD_SVI_SUPPORT "svi:support"
#define CMD_SVI_STATUS "svi:status"
#define CMD_SVI_RANGE "svi:range"
#define CMD_SVI_GET "svi:get:strength"
#define CMD_SVI_SET "svi:set:strength"

#define OP_INDEX 0
#define DISPLAY_INDEX 1
#define FEATURE_INDEX 2
#define FLAG_INDEX 3
#define DATA_INDEX 4

#define CMD_PP_ON "pp:on"
#define CMD_PP_OFF "pp:off"
#define CMD_PP_SET_HSIC "pp:set:hsic"
#define CMD_POSTPROC_STATUS "pp:query:status:postproc"
#define PP_CFG_FILE_PROP "hw.pp.cfg"
#define PP_CFG_FILE_PATH "/data/misc/display/pp_data.cfg"
#define CMD_PP_PREFIX "pp:"
#define CMD_PP_MEM_SUPPORT "pp:mem:support"

#define CMD_CABL_PREFIX "cabl:"
#define CMD_CABL_ON "cabl:on"
#define CMD_CABL_OFF "cabl:off"
#define CMD_CABL_SET "cabl:set"
#define CMD_CABL_STATUS "cabl:status"
#define CMD_CABL_SUPPORT "cabl:support"
#define CMD_CABL_GET "cabl:get"
#define CMD_CABL_SCALE "cabl:scale"


#define CMD_GET "get"
#define CMD_SET "set"

#define PRIMARY_DISPLAY 0
#define SECONDARY_DISPLAY 1
#define WIFI_DISPLAY 2

#define FEATURE_PCC "pcc"
#define MIN_PCC_PARAMS_REQUIRED 37
#define TOKEN_PARAMS_DELIM ":;"

#define CMD_DCM_ON "dcm:on"
#define CMD_DCM_OFF "dcm:off"

#define CMD_OEM_PREFIX "oem:"
#define CMD_OEM_GET_PROFILES "oem:get:profile"
#define CMD_OEM_SET_PROFILE "oem:set:profile"

namespace qmode {
class ModeManager;
}

extern volatile int32_t sigflag;

enum sysfs_index {
    AD_WB_SYSFS,
    DYN_PU_SYSFS,
};

enum ctrl_status {
    cabl_bit = 0x01,
    ad_bit = 0x02,
    ctrl_bit = 0x04,
    aba_cabl_bit = 0x08,
    aba_svi_bit = 0x10,
    aba_bit = 0x18,
};

class DaemonContext {
    AbaContext *mABA;
    bool bAbaEnabled;
    bool bQdcmLoaded;
    pthread_t mControlThrdId;
    int screenStatus;
    int mCtrlStatus;

    int LoadQdcmLibrary();
    int ProcessADMsg (const char* buf, const int32_t& fd);
    int ProcessCABLMsg(char* buf, const int32_t& fd);
    int ProcessPPMsg (const char* buf, const int32_t& fd);
    int ProcessSVIMsg(const char* buf, const int32_t& fd);
    int ProcessDebugMsg(char* buf);
    int ProcessSetMsg(char* buf);
    int ProcessPCCMsg(char* buf);
    static void *control_thrd_func(void *obj) {
        reinterpret_cast<DaemonContext *>(obj)->ProcessControlWork();
        return NULL;
    }
    void ProcessControlWork();

    static void *poll_thrd_func(void *obj) {
        reinterpret_cast<DaemonContext *>(obj)->ProcessPollWork();
        return NULL;
    }
    void ProcessPollWork();
    void StartAlgorithmObjects();

public:
    uint32_t mEnabledFeatures;
    CABL *mCABL;
    PostProc mPostProc;
    AD mAD;
    DCM *mDCM;
    qmode::ModeManager *mQdcmMode;
    void *mQdcmLib;
    bool mDebug;
    int32_t mListenFd;
    int32_t mAcceptFd;
    int32_t mNumConnect;
    int32_t nPriPanelType;
    pthread_mutex_t mCtrlLock;
    pthread_mutex_t mCABLOpLock;
    pthread_mutex_t mPostProcOpLock;
    pthread_mutex_t mADOpLock;
    pthread_mutex_t mSVIOpLock;
    pthread_t mPollThrdId;
    int ad_fd;
    bool mSplitDisplay;
    bool mBootStartAbaCabl;
    bool mBootStartAbaSvi;
    bool mBootStartCABL;
    int32_t start();
    int32_t getListenFd();
    int32_t PPFeatureControl(int enable, uint32_t features);
    int32_t ProcessCommand(char *, const int32_t, const int32_t&);
    int32_t reply(bool, const int32_t&);
    int32_t reply(const char *, const int32_t&);
    int SelectFB(int display_id, int* idx);
    bool IsSplitDisplay(int);
    inline AbaContext* getABA(){
        return mABA;
    }
    void StopAlgorithmObjects();
    void UnloadQdcmLibrary();
    DaemonContext() : mABA(NULL), bAbaEnabled(false), bQdcmLoaded(false),
        screenStatus(0), mCtrlStatus(0), mEnabledFeatures(0), mCABL(NULL),
        mDCM(NULL), mDebug(false),mListenFd(-1), mAcceptFd(-1), mNumConnect(4),
        mSplitDisplay(0) {
        char property[PROPERTY_VALUE_MAX];
        assert(sizeof(struct sockaddr) < INT32_MAX);
        if (property_get("debug.listener.logs", property, 0) > 0 && (atoi(property) == 1)) {
            mDebug = true;
        }
        // If Aba is enabled then ABL CABL should not be enabled
        if((2 == display_pp_cabl_supported())
            || (1 == display_pp_svi_supported())) {
            bAbaEnabled = true;
        }
        mBootStartAbaCabl = false;
        mBootStartAbaSvi = false;
        mBootStartCABL = false;
        pthread_mutex_init(&mCtrlLock, NULL);
        pthread_mutex_init(&mCABLOpLock, NULL);
        pthread_mutex_init(&mPostProcOpLock, NULL);
        pthread_mutex_init(&mADOpLock, NULL);
        pthread_mutex_init(&mSVIOpLock, NULL);
        ad_fd = -1;
    }
    ~DaemonContext() {
        UnloadQdcmLibrary();
        pthread_mutex_destroy(&mCtrlLock);
        pthread_mutex_destroy(&mCABLOpLock);
        pthread_mutex_destroy(&mPostProcOpLock);
        pthread_mutex_destroy(&mADOpLock);
        pthread_mutex_destroy(&mSVIOpLock);
    }
};

#endif /* _DAEMONCONTEXT_H */
