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

#include "pp_daemon.h"
#include <signal.h>
#include <time.h>
#include <getopt.h>
#include <poll.h>
#include <unistd.h>

volatile int32_t sigflag = 0;
int FBFd = -1;
int32_t BLFd = -1;
int PUFd = -1;
bool PuEnabled = false;

static void signal_handler(int32_t sig) {
    /* Dummy signal handler */
    ;
}

void openPrimaryFrameBuffer(DaemonContext &context) {
    int ret;
    int fb_idx = -1;
    char FbPath[MAX_FB_NAME_LEN];

    context.SelectFB(MDP_LOGICAL_BLOCK_DISP_0, &fb_idx);
    if (fb_idx < 0) {
        LOGE("%s: Cannot locate the primary framebuffer", __func__);
        return;
    }

    snprintf(FbPath, sizeof(FbPath), "%s%d", FRAMEBUFFER_NODE, fb_idx);
    FBFd = open(FbPath, O_RDWR);
    if (FBFd < 0)
        LOGE("%s: Cannot open framebuffer", __func__);
}

int main(int32_t argc, char** argv) {
    DaemonContext context;
    sigset_t signal_mask;
    int32_t sig;
    int ret;

    /*read the cabl version and set the property */
    char cabl_version[VERS_MAX_LEN] = {0};
    cabl_get_version(cabl_version);
    LOGD("CABL version %s", cabl_version);
    if (property_set("hw.cabl.version", cabl_version))
        LOGE("Failed to set the cabl version property");

    if (property_set(YUV_INPUT_STATE_PROP, "0"))
        LOGE("Failed to set the cabl auto level adjust property");

    inspectHW();

    bool is_hdmi_primary = isHDMIPrimary();
    if (!is_hdmi_primary) {
        if (gHwInfo.isMDP5)
            context.mPostProc.mBlockType = MDP_LOGICAL_BLOCK_DISP_0;
        else
            context.mPostProc.mBlockType = MDP_BLOCK_DMA_P;
    } else
        context.mPostProc.mBlockType = MDP_BLOCK_OVERLAY_1;

    /* Open the primary framebuffer */
    if (FBFd == -1)
        openPrimaryFrameBuffer(context);
    if (FBFd < 0)
        exit(FBFd);

    /* Open the backlight sysfs node */
    BLFd = open(SYS_BRIGHTNESS, O_RDWR);
    if (BLFd < 0) {
        BLFd = open(SYS_BRIGHTNESS_ALT, O_RDWR);
        if (BLFd < 0) {
            LOGE("Cannot open backlight");
            close(FBFd);
            exit(BLFd);
        }
    }
    /*Check whether target is using split display or not*/
    context.IsSplitDisplay(FBFd);

    /* Open the partial update sysfs node */
    PUFd = open(PARTIAL_UPDATE_SYSFS, O_RDONLY);
    if (PUFd < 0)
        LOGE("Unable to open pu node, err:  %s", strerror(errno));
    else
        PuEnabled = IsPuEnabled();

    if(context.start()) {
        LOGE("Failure to start the listener thread");
        close(FBFd);
        close(BLFd);
        close(PUFd);
        exit(-1);
    }

    if(!context.mDCM)
        context.mDCM = new DCM();
    if (context.mDCM)
        context.mDCM->DCMControl(true);

    char buf[SHORT_CMD_LEN];
    int32_t acceptFd = -1;
    int opt;
    static struct option long_options[] = {
        {"enable-cabl", no_argument, 0, 'c'},
        {"enable-ad", no_argument, 0, 'a'},
        {"enable-aba-svi", no_argument, 0, 's'},
        {"enable-aba-cabl", no_argument, 0, 'x'},
        {0, 0, 0, 0},
    };

    while ((opt = getopt_long(argc, argv, "acsx", long_options, NULL)) != -1) {
        switch (opt) {
            case 'a':
                 snprintf(buf, SHORT_CMD_LEN, "%s;%d;%d", CMD_AD_ON, ad_mode_auto_str, MDP_LOGICAL_BLOCK_DISP_0);
                 context.ProcessCommand(buf, SHORT_CMD_LEN, acceptFd);
                 break;
            case 'c':
                snprintf(buf, SHORT_CMD_LEN, "%s", CMD_CABL_ON);
                context.mBootStartCABL = true;
                context.ProcessCommand(buf, SHORT_CMD_LEN, acceptFd);
                break;
            case 's':
                snprintf(buf, SHORT_CMD_LEN, "%s", CMD_SVI_ON);
                context.mBootStartAbaSvi = true;
                context.ProcessCommand(buf, SHORT_CMD_LEN, acceptFd);
                break;
            case 'x':
                snprintf(buf, SHORT_CMD_LEN, "%s", CMD_CABL_ON);
                context.mBootStartAbaCabl = true;
                context.ProcessCommand(buf, SHORT_CMD_LEN, acceptFd);
                break;
            default:
                LOGE("Un-recognized option");
                break;
        }
    }

    // on bootup read and populate the pp data file if any
    context.mPostProc.processPPDataFile();

    sigemptyset(&signal_mask);
    sigaddset(&signal_mask, SIGTERM);
    pthread_sigmask(SIG_BLOCK, &signal_mask, NULL);

    /* Register the signal Handler */
    sigfillset(&signal_mask);
    signal(SIGTERM, &signal_handler);
    signal(SIGPIPE, SIG_IGN);
    while (1) {
        sigwait(&signal_mask, &sig);
        switch (sig) {
            case SIGTERM:
                LOGD("SIGTERM received, stopping daemon");
                sigflag = 1;
                if((2 == display_pp_cabl_supported()) && context.getABA()){
                    context.getABA()->CABLControl(false);
                    context.getABA()->SVIControl(false);
                    context.StopAlgorithmObjects();
                } else if (context.mCABL) {
                    context.mCABL->stop_cabl();
                }
                context.mPostProc.stop_pp();
                context.mAD.ADControl(&context, false);

                if (context.mDCM) {
                    context.mDCM->DCMControl(false);
                    delete context.mDCM;
                    context.mDCM = NULL;
                }
                void *term;
                pthread_join(context.mPollThrdId, &term);
                LOGD_IF(context.mDebug, "PP Poll thread terminated");

                context.UnloadQdcmLibrary();
                close(PUFd);
                close(BLFd);
                close(FBFd);
                exit(EXIT_SUCCESS);
        }
    }
    return 0;
}
