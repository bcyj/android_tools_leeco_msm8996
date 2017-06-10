/* Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

/*****************************************************************************
    This file contains defines Video Flow class. Video flow converts layer
    and metadata into input and output config of VPU. VideoFlow provides an
    interface between "layer" and vpu "session". VideoFlow also keeps
    association between post processing configurations and vpu sessions.
*****************************************************************************/
#include <sys/types.h>
#include "vfm_defs.h"
#include "vfm_videoflow.h"
#include "vfm_utils.h"

namespace vpu {

/*****************************************************************************
            Public Methods
*****************************************************************************/
//ctor
VidFlow::VidFlow(int flowId, EventThread* evtThrd, const notifyCb_t& notifyCb,
                int enableDebugLogs)
     : mFlowId(flowId),
       mEventThread(evtThrd),
       mNotifyCb(notifyCb),
       mDebugLogs(enableDebugLogs)
{
    ALOGD_IF(isDebug(), "%s :E flowId: %d", __func__, mFlowId);

    int32_t dpy = DISP_PRIMARY;
    mDispAttr[DISP_PRIMARY].width    = 1920;
    mDispAttr[DISP_PRIMARY].height   = 1080;
    mDispAttr[DISP_PRIMARY].fp100s   = 60 * 100;

    reset();
}

VidFlow::~VidFlow()
{
}

/*!
    VidFlow is not created or destroyed when a new layer becomes visible
    or goes invisible. Only init() is called again when a new layer becomes
    visible and teardown() is called when an existing layer goes invisible
    1. Detect the video flow mode - VCAP tunneling, VFM tunneling, SF
    2. For each of the modes, read the configuration from layerList
        or metadata. This might differ for each mode
    3. Call vpuSessoin API to set the configuration
*/
status_t VidFlow::init(Layer* layer)
{
    status_t err = NO_ERROR;
    InputCfg        inputCfg;
    OutputCfg       outputCfg;

    mEnable10bitBWC = getProperty("persist.vfm.10bit.enable");
    ALOGD_IF(isDebug(), "%s :E persist.vfm.10bit.enable: %d", __func__,
                        mEnable10bitBWC);
    if(isVcapSource(layer->handle)){
        mVpuSess = new VPUSessionVcapTunnel(mFlowId, mEventThread, mNotifyCb,
                                                mDebugLogs);
        mSrc    = INPUT_SRC_VCAP_TUNNEL;
        /* For vcap tunneling, only dummy buffers are queued in the draw call */
        mQMode  = NO_Q_ON_DRAW;
    }
    else if(isVfmTunnel(layer->handle)){
        mVpuSess = new VPUSessionVfmTunnel(mFlowId, mEventThread, mNotifyCb,
                                                mDebugLogs);
        mSrc    = INPUT_SRC_VFM_TUNNEL;
        /* For vfm tunneling, only dummy buffers are queued in the draw call */
        mQMode  = NO_Q_ON_DRAW;
    }
    else{
        mVpuSess = new VPUSessionSfBuffers(mFlowId, mEventThread, mNotifyCb,
                                                mDebugLogs);
        mSrc    = INPUT_SRC_SF_QBUF;
        mQMode  = Q_ON_DRAW;
    }

    err = mVpuSess->init();
    /* if VPU session cannot be created, then delete it */
    if(NO_ERROR != err){
        delete mVpuSess;
        return err;
    }
    mState = INIT_DONE;
    //TODO: notify that new VpuSess is created so that VPU pp is reconfigured

    //Extract configuration from layer structure and metadata
    inputCfg.inSrc.eInputSrcType = mSrc;
    layer2InOutCfg(layer, inputCfg, outputCfg);

    err = mVpuSess->setInOutCfg(inputCfg, outputCfg);
    // If session can't be setup, close vpu session, reset vidFlow
    if(NO_ERROR != err){
        reset();
        delete mVpuSess;
        /* Not an error condition. Logging as debug info here */
        ALOGD_IF(isDebug(), "Failed to setup VPU session for %s, %s",
            inputCfg.dump().string(), outputCfg.dump().string());
        return err;
    }

    mState       = RUNNING;
    mMediaSessId = getMediaSessId(layer->handle);

    layer->vpuOutPixFmt = outputCfg.pixFmt;

    ALOGD_IF(isDebug(), "%s: Video flow for mediasession Id %x setup",
                __func__, mMediaSessId);

    return err;
}

/*
    This function gets config from existing VpuSession, and updates the
    config from the updated layer and then sets config to VpuSession
*/
status_t VidFlow::updateLayer(Layer* layer)
{
    status_t err = NO_ERROR;
    InputCfg        inputCfg;
    OutputCfg       outputCfg;
    ALOGD_IF(isDebug(), "%s :E", __func__);

    if(RUNNING == mState){
        /* Query the current config before updating the same */
        mVpuSess->getInOutCfg(inputCfg, outputCfg);

        //Extract configuration from layer structure and metadata
        inputCfg.inSrc.eInputSrcType = mSrc;
        layer2InOutCfg(layer, inputCfg, outputCfg);

        err = mVpuSess->setInOutCfg(inputCfg, outputCfg);
        // If the new configuration cannot be set, close vpu session
        //  reset vidFlow
        if(NO_ERROR != err){
            /* Not an error condition. Logging as debug info here */
            ALOGD_IF(isDebug(), "Failed to reconfigure VPU session %d for"
                "%s, %s", mFlowId, inputCfg.dump().string(),
                outputCfg.dump().string());
            reset();
            delete mVpuSess;
            return err;
        }

        layer->vpuOutPixFmt = outputCfg.pixFmt;
    }
    return err;
}

//! queues the buffer to vpuSession if its in Q_ON_DRAW mode
status_t VidFlow::qBuf(private_handle_t* handle, int32_t& releaseFenceFd)
{
    status_t err = NO_ERROR;
    ALOGD_IF(isDebug(), "%s :E", __func__);

    //Nothing to be done if Queuing mode is NO_Q_ON_DRAW
    if(NO_Q_ON_DRAW == mQMode){
        /* set releaseFenceFd = -1 as this buffer is not queued to vpu driver*/
        releaseFenceFd = -1;
        return NO_ERROR;
    }

    switch(mState) {
        case RUNNING:
        {
            err = mVpuSess->qBuf(handle, releaseFenceFd);
            ERR_CHK((NO_ERROR != err), err);
            break;
        }
        default:
            ALOGE("%s: Invalid state", __func__);
            return INVALID_OPERATION;
    }
    return err;
}

/*!
  @brief         Tearsdown the Video Flow

  @param[out]    None.

  @param[in]     None.

  @return        status_t Error.

  @note    Synchronous function
 */
status_t VidFlow::teardown()
{
    status_t err = NO_ERROR;
    ALOGD_IF(isDebug(), "%s :E", __func__);

    switch(mState){
        case INIT_DONE:
        case RUNNING:
            //teardown is called explicitly to catch the error
            err = mVpuSess->teardown();
            delete mVpuSess;
            mVpuSess = NULL;

            reset();
            ALOGD_IF(isDebug(), "%s: session teardown %d", __func__, mFlowId);
            break;

        default:
            break;
    }
    return err;
}

status_t VidFlow::processCommand(const uint32_t command,
        const vfmCmdInArg_t& inArg, vfmCmdOutArg_t& outArg)
{
    ALOGD_IF(logAll(), "%s: flowId: %d cmd: %d", __func__, mFlowId, command);
    switch(command){
        case VFM_CMD_SET_DISPLAY_ATTRIBUTES:
           setDisplayAttr(inArg.iaDispAttr);
           return NO_ERROR;
        break;
    }
    if(mVpuSess){
        return mVpuSess->processCommand(command, inArg, outArg);
    }
    return NO_INIT;
}

/*****************************************************************************
              PRIVATE METHODS
*****************************************************************************/
//! Initializes all the members and releases any acquired resources
status_t VidFlow::reset()
{
    status_t err = NO_ERROR;
    ALOGD_IF(isDebug(), "%s :E", __func__);

    //Note: donot reset flowId and mDebugLogs here
    mState        = UN_INIT;
    mQMode        = Q_ON_DRAW;
    mMediaSessId  = -1;

    return err;
}

//! Factory for layer2InOutCfg functionality
status_t VidFlow::layer2InOutCfg(Layer* layer, InputCfg& inCfg,
                                    OutputCfg& outCfg)
{
    if(INPUT_SRC_VCAP_TUNNEL == mSrc) {
        return layer2InOutCfgVcapTunnel(layer, inCfg, outCfg);
    }
    else if(INPUT_SRC_VFM_TUNNEL == mSrc) {
        return layer2InOutCfgVfmTunnel(layer, inCfg, outCfg);
    }
    else /* if(SRC_SF == mSrc) */ {
        return layer2InOutCfgSfBuffers(layer, inCfg, outCfg);
    }
}

status_t VidFlow::layer2InOutCfgVcapTunnel(Layer* layer, InputCfg& inCfg,
                                            OutputCfg& outCfg)
{
    status_t err = NO_ERROR;
    ALOGD_IF(isDebug(), "%s :E", __func__);

    /* For Vcap tunneling, dummy buffer is queued to SF. Hence the actual
        buffer geometry is communicated through metadata */
    metadata2InBufGeometry(layer->handle, inCfg);
    layer2InInterlace(layer, inCfg);
    layer2In3D(layer, inCfg);
    layer2InColorSpace(layer, inCfg);
    layer2InFramerate(layer, inCfg);
    /* Vcap tunneling requires vcap pipe information as well */
    layer2InSrcPipes(layer, inCfg);
    //TODO: Secure session
    //layer2SecureSess(layer, inCfg);

    layer2OutTgtRect(layer, outCfg);

    /* Obtained from side-channel binder call for view modes */
    getSrcCrop(inCfg);
    getDestRect(outCfg);

    getOutFramerate(inCfg, outCfg);
    getOutPixFmt(inCfg, outCfg);
    getOutColorspace(inCfg, outCfg);
    getOutBufferDim(inCfg, outCfg);
    getOut3DFormat(outCfg);
    getOutField(outCfg);

    return err;
}

status_t VidFlow::layer2InOutCfgVfmTunnel(Layer* layer, InputCfg& inCfg,
                                            OutputCfg& outCfg)
{
    status_t err = NO_ERROR;
    ALOGD_IF(isDebug(), "%s :E", __func__);

    /* Read the configuration from metadata of dummy buffer only while
        setting up the session. Subsequently, the change in source
        characteristics is communicated via the metadata of the actual
        buffer queued via binder. */
    if(mState < RUNNING){
        //Buffer geometry metadata includes pixel format as well
        metadata2InBufGeometry(layer->handle, inCfg);
        layer2InInterlace(layer, inCfg);
        layer2In3D(layer, inCfg);
        layer2InColorSpace(layer, inCfg);
        layer2InFramerate(layer, inCfg);
        //Time stamp is not retrieved from the dummy buffer
        //TODO: Secure session
        //layer2SecureSess(layer, inCfg);
    }

    //Output dimension is received from SF/HWC every time
    layer2OutTgtRect(layer, outCfg);

    /* Obtained from side-channel binder call for view modes */
    getSrcCrop(inCfg);
    getDestRect(outCfg);

    getOutFramerate(inCfg, outCfg);
    getOutPixFmt(inCfg, outCfg);
    getOutColorspace(inCfg, outCfg);
    getOutBufferDim(inCfg, outCfg);
    getOut3DFormat(outCfg);
    getOutField(outCfg);

    return err;
}

status_t VidFlow::layer2InOutCfgSfBuffers(Layer* layer, InputCfg& inCfg,
                                            OutputCfg& outCfg)
{
    status_t err = NO_ERROR;
    ALOGD_IF(isDebug(), "%s :E", __func__);

    layer2InBufGeometry(layer, inCfg);
    layer2InPixFmt(layer, inCfg);
    layer2InInterlace(layer, inCfg);
    layer2In3D(layer, inCfg);
    layer2InColorSpace(layer, inCfg);
    layer2InFramerate(layer, inCfg);
    //TODO: Secure session
    //layer2SecureSess(layer, inCfg);

    layer2OutTgtRect(layer, outCfg);

    /* Obtained from side-channel binder call for view modes */
    getSrcCrop(inCfg);
    getDestRect(outCfg);

    getOutFramerate(inCfg, outCfg);
    getOutPixFmt(inCfg, outCfg);
    getOutColorspace(inCfg, outCfg);
    getOutBufferDim(inCfg, outCfg);
    getOut3DFormat(outCfg);
    getOutField(outCfg);

    return err;
}

void VidFlow::getSrcCrop(InputCfg& inCfg){
    //TODO: Overwrite the source crop if it is set from side-channel view mode
    //config command
}

void VidFlow::getDestRect(OutputCfg& outCfg){
    //Dest rect can be set from side-channel config command
    //Else, dest rect = tgt rect
    outCfg.dstRect = outCfg.tgtRect;
}

void VidFlow::getOutFramerate(InputCfg& inCfg, OutputCfg& outCfg){
    int32_t dpy = DISP_PRIMARY;
    outCfg.framerate = mDispAttr[dpy].fp100s;
}

void VidFlow::getOutPixFmt(InputCfg& inCfg, OutputCfg& outCfg){
    //Out pixel format is same as input pixel format
    outCfg.pixFmt = inCfg.pixFmt;

    //8092 MDSS does not support VENUS_NV12.
    if (mEnable10bitBWC){
        outCfg.pixFmt = HAL_PIXEL_FORMAT_YCbCr_422_I_10BIT_COMPRESSED;
        ALOGD_IF(isDebug(), "%s: 10 bit BWC chosen ", __func__);
    } else {
        outCfg.pixFmt = HAL_PIXEL_FORMAT_RGB_888;
        ALOGD_IF(isDebug(), "%s: RGB888 out format ", __func__);
    }
}

void VidFlow::getOutColorspace(InputCfg& inCfg, OutputCfg& outCfg){
    outCfg.colorspace = inCfg.colorspace;
}

void VidFlow::getOutBufferDim(InputCfg& inCfg, OutputCfg& outCfg){
    int32_t dpy = DISP_PRIMARY;
    int32_t pixelDimension = (outCfg.tgtRect.right - outCfg.tgtRect.left) *
                            (outCfg.tgtRect.bottom - outCfg.tgtRect.top);
//      If tgtRect <= 1080p, sDim = 1080p so that 4k is not allocated
//      un-necessarily
    if(pixelDimension <= 1920 * 1080){
        outCfg.sDim.width = 1920;
        outCfg.sDim.height = 1080;
    }else if (pixelDimension <= mDispAttr[dpy].width * mDispAttr[dpy].height){
        outCfg.sDim.width = mDispAttr[dpy].width;
        outCfg.sDim.height = mDispAttr[dpy].height;
    }else{
        outCfg.sDim.width = outCfg.tgtRect.right - outCfg.tgtRect.left;
        outCfg.sDim.height = outCfg.tgtRect.bottom - outCfg.tgtRect.top;
    }
}

void VidFlow::getOut3DFormat(OutputCfg& outCfg){
    /* TODO: This should be set by binder api? */
    outCfg.is3d = 0;
}

void VidFlow::getOutField(OutputCfg& outCfg){
    /* output is alway progressive */
    outCfg.eField = VFM_FIELD_ORDER_NONE;
}

void VidFlow::setDisplayAttr(const vfmInArgSetDisplayAttr_t& iaDisplayAttr)
{
    ALOGD_IF(isDebug(), "%s: dpy: %d w: %d h: %d fps: %d", __func__,
        iaDisplayAttr.dpy, iaDisplayAttr.width, iaDisplayAttr.height,
        iaDisplayAttr.fp100s);
    mDispAttr[iaDisplayAttr.dpy].width = iaDisplayAttr.width;
    mDispAttr[iaDisplayAttr.dpy].height = iaDisplayAttr.height;
    mDispAttr[iaDisplayAttr.dpy].fp100s = iaDisplayAttr.fp100s;
    return;
}

}; //namespace vpu
