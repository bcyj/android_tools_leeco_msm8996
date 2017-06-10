/* Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

/*****************************************************************************
    This file defines VPU Session objects. Each VPUSession object manages
    one VPU session.
*****************************************************************************/
#include <linux/videodev2.h>

#include "vfm_utils.h"
#include "vfm_vpusession.h"

namespace vpu {

VPUSession::VPUSession(int vpuSessId, EventThread* evtThrd,
                   const notifyCb_t& notifyCb, int enableDebugLogs)
    : mFd(-1),
      mBufQ(NULL),
      mDebugLogs(enableDebugLogs),
      mSessId(vpuSessId),
      mDbgOutFrmNum(0),
      mDbgDisableTimestamp(true),
      mOutBufPool(NULL),
      mEventThread(evtThrd),
      mNotifyCb(notifyCb),
      mNumDrvInBufs(NUM_DRIV_IN_BUFS),
      mNumDrvOutBufs(NUM_DRIV_OUT_BUFS),
      mState(FD_CLOSE),
      mPvtHandle(NULL),
      mCachedReleaseFenceFd(-1),
      mDebugFileDump(0)
{
    mDebugFileDump = getProperty("debug.vfm.filedump");
    initStats();
    mStats.mEnable = getProperty("persist.vfm.statistics");
    mStats.mLoopback = getProperty("persist.vfm.statistics.loopback");
    mStats.mFirstFrameQTime = 0;
    mStats.mPrevFrameDQTime = gettimeofdayMs();
    mStats.mPrevFrameQTime = gettimeofdayMs();

    mFrmRptEnable = getProperty("persist.vfm.frmrepeat.enable");
    mVpuInitBufDelay = getProperty("persist.vfm.initbufdelay");
    if(mVpuInitBufDelay > 0)
        mFrameRepeat.setInitBufDelay(mVpuInitBufDelay);
    mVpuOutBuffers = getProperty("persist.vfm.vpuoutbuffers");
    if(mVpuOutBuffers > 0)
        mNumDrvOutBufs = mVpuOutBuffers;
    mDbgDumpPixelVal = getProperty("persist.vfm.dbg.dumppix");
    mDbgBufQ         = getProperty("persist.vfm.dbg.bufq");

    memset(&mInCfg, 0, sizeof(InputCfg));
    memset(&mOutCfg, 0, sizeof(OutputCfg));
    return;
}

status_t VPUSession::init()
{
    status_t err = NO_ERROR;
    ALOGD_IF(isDebug(), "%s :E", __func__);

    if(!(mState & FD_OPEN)){
        mFd = VpuIntf::getFd();
        if(mFd < 0){
            ALOGE("%s: getFd failed ", __func__);
            return NO_INIT;
        }
        mState |= FD_OPEN;
    }

    if(!(mState & SESSION_ATTACHED)){
        err = VpuIntf::attachToSession(mFd, mSessId);
        if(NO_ERROR != err){
            ALOGE("Session attach failed for %d", mSessId);
            return err;
        }
        mState |= SESSION_ATTACHED;
    }
    if(!(mState & SUBSCRIBE_TO_EVENTS)){
        err = VpuIntf::subscribeToEvents(mFd);
        ERR_CHK_LOG((NO_ERROR != err), UNKNOWN_ERROR,
                        "event subscription failed");
        mState |= SUBSCRIBE_TO_EVENTS;
    }
    if(!(mState & REGISTER_TO_EVENT_THREAD)){
        EventThread::event_procs_t evtHdlr;
        evtHdlr.fd              = mFd;
        evtHdlr.notifyEvent     = onVpuEventWrapper;
        evtHdlr.notifyDequeue   = onVpuDequeueEventWrapper;
        evtHdlr.cookie          = reinterpret_cast<void *>(this);

        err = mEventThread->registerFd(evtHdlr);
        ERR_CHK_LOG((NO_ERROR != err), UNKNOWN_ERROR,
                        "Registering to event thread failed");
        mState |= REGISTER_TO_EVENT_THREAD;

        //Set timestamp control
        vfmCmdInArg_t   inArg;
        vfmCmdOutArg_t  outArg;
        int32_t*        pkt;

        inArg.iaVpuCtrlExt.type = 1;
        inArg.iaVpuCtrlExt.dataLength = 12;

        pkt = reinterpret_cast<int32_t *>(inArg.iaVpuCtrlExt.dataBuffer);
        /* VPU firmware private packet API */
#define SESSION_TIMESTAMP_CTRL_PKT_SIZE         12 //bytes
#define SESSION_TIMESTAMP_CTRL_ID               (0x30000115)
#define SESSION_TIMESTAMP_CTRL_START_DELAY      0x1
#define SESSION_TIMESTAMP_CTRL_REPEAT           0x2
#define SESSION_TIMESTAMP_CTRL_NORMAL           0x4
        pkt[0] = SESSION_TIMESTAMP_CTRL_PKT_SIZE;
        pkt[1] = SESSION_TIMESTAMP_CTRL_ID;
        pkt[2] = 0; //default
        if(mFrmRptEnable){
            pkt[2] |= SESSION_TIMESTAMP_CTRL_REPEAT;
            if(mVpuInitBufDelay > 0)
                pkt[2] |=  SESSION_TIMESTAMP_CTRL_START_DELAY;
        }
        err  = VpuIntf::setVpuControlExtended(mFd, inArg, outArg);
        ERR_CHK((NO_ERROR != err), err);
    }

    ALOGD_IF(isDebug(), "%s Success attachToSession %d", __func__, mSessId);

    return err;
}

status_t VPUSession::initStats()
{
    mStats.mEnable = 0;
    mStats.mNumBufsQueuedIn = 0;
    mStats.mNumBufsQueuedToDriver = 0;
    mStats.mNumBufsDequeuedFromDriver = 0;
    mStats.mNumDequeueEventsFromDriver = 0;

    return NO_ERROR;
}

String8 VPUSession::dumpStats()
{
    String8 str;
    if(mStats.mEnable){
        char s[256];
        snprintf(s, 256, "BufsIn: %d: BufsToDriver: %d BufsFromDriver: %d "
        "DeqEvents: %d", mStats.mNumBufsQueuedIn, mStats.mNumBufsQueuedToDriver,
        mStats.mNumBufsDequeuedFromDriver, mStats.mNumDequeueEventsFromDriver);
        str += s;
    }
        return str;
}

VPUSession::~VPUSession()
{
    teardown();
    if(mBufQ)
        delete mBufQ;
    if(mOutBufPool)
        delete mOutBufPool;
    ALOGD_IF(mStats.mEnable, "Session End: %s", dumpStats().string());
}

//Synchronous
//NOTE: InOutCfg should be atomic and no buffer should be queued to driver while
//         this call is in progress
status_t VPUSession::setInOutCfg(const InputCfg& inCfg, const OutputCfg& outCfg)
{
    return processSetInOutCfg(inCfg, outCfg);
}

status_t VPUSession::qBuf(const private_handle_t* hnd, int32_t& releaseFenceFd)
{
    if((mState & STREAMON_IN_PORT) && (mState & STREAMON_OUT_PORT)){
        processQBuf(hnd, releaseFenceFd);
    }
    if (!(mState & STREAMON_IN_PORT)){
        ALOGE("%s: QBuf called without STREAMON_IN_PORT", __func__);
    }
    if (!(mState & STREAMON_OUT_PORT)){
        ALOGE("%s: QBuf called without STREAMON_OUT_PORT", __func__);
    }
    return NO_ERROR;
}

status_t VPUSession::processCommand(const uint32_t command,
        const vfmCmdInArg_t& inArg, vfmCmdOutArg_t& outArg)
{
    ALOGD_IF(logAll(), "%s: vpuSessId: %d cmd: %d", __func__, mSessId,command);
    switch (command) {
        case VFM_CMD_SET_VPU_CONTROL:
            return processCmdSetVpuControl(inArg, outArg);
            break;
        case VFM_CMD_GET_VPU_CONTROL:
            return processCmdGetVpuControl(inArg, outArg);
            break;
        case VFM_CMD_SET_VPU_CONTROL_EXTENDED:
            return VpuIntf::setVpuControlExtended(mFd, inArg, outArg);
            break;
        case VFM_CMD_GET_VPU_CONTROL_EXTENDED:
            return VpuIntf::getVpuControlExtended(mFd, inArg, outArg);
            break;
        default:
            ALOGE("%s: Invalid command %d", __func__, command);
    }
    return NO_ERROR;
}

status_t VPUSession::processSetInOutCfg(const InputCfg& inCfg,
                                            const OutputCfg& outCfg)
{
    status_t err = NO_ERROR;

    err = setInputCfg(inCfg);
    ERR_CHK((NO_ERROR != err), err);

    err = setOutputCfg(outCfg);
    ERR_CHK((NO_ERROR != err), err);

    //Stream ON should be called after setting dest pipes info
    //Commit need not be called after every change in the config
    if(!(mState & COMMIT_CONFIG)){
        err = VpuIntf::commitConfig(mFd);
        ERR_CHK((NO_ERROR != err), err);
        mState |= COMMIT_CONFIG;
        mFrameRepeat.setFrameRate(mInCfg.framerate, mOutCfg.framerate);
    }

    return err;
}

status_t VPUSession::setInputCfg(const InputCfg& inCfg)
{
    status_t err = NO_ERROR;

    if(mInCfg == inCfg){
        ALOGD_IF(logAll(), "inCfg has not changed, skipping config");
        return NO_ERROR;
    }

    ALOGD_IF(isDebug(), "%s: previous config: %s", __func__,
        mInCfg.dump().string());
    ALOGD_IF(isDebug(), "%s: new config: %s", __func__,
        inCfg.dump().string());

    if(inCfg.inSrc != mInCfg.inSrc){
        err = VpuIntf::setInputSource(mFd, inCfg);
        ERR_CHK((NO_ERROR != err), err);
    }

    if(inCfg != mInCfg){
        //sets w, h, pixFmt, field order
        err = VpuIntf::setInputFormat(mFd, inCfg);
        ERR_CHK((NO_ERROR != err), err);

        //TODO: VpuIntf::setInputCrop(inCfg)

        //FIXME: Enable noise reduction always for non RGB cases.
        // Required for split pipe configuration in VPU firmware for
        //1080p -> 4k scaling
        if(inCfg.pixFmt != HAL_PIXEL_FORMAT_RGB_888){
            struct vpu_control vpuCtrl;
            memset(&vpuCtrl, 0, sizeof(struct vpu_control));
            vpuCtrl.control_id = VPU_CTRL_NOISE_REDUCTION;
            vpuCtrl.data.auto_manual.enable     = 1;
            vpuCtrl.data.auto_manual.auto_mode  = 0;
            vpuCtrl.data.auto_manual.value      = 0;

            err  = VpuIntf::setVpuControl(mFd, vpuCtrl);
            ERR_CHK((NO_ERROR != err), err);
        }
    }

    if(inCfg.colorspace != mInCfg.colorspace){
        err = VpuIntf::setInColorSpace(mFd, inCfg.colorspace);
        ERR_CHK((NO_ERROR != err), err);
    }

    if(inCfg.framerate != mInCfg.framerate){
        err = VpuIntf::setInputFrameRate(mFd, inCfg);
        ERR_CHK((NO_ERROR != err), err);
    }

    //Store this config locally if everything goes fine
    mInCfg = inCfg;

    return err;
}

status_t VPUSession::setOutputCfg(const OutputCfg& outCfg)
{
    status_t err = NO_ERROR;

    if(outCfg == mOutCfg){
        ALOGD_IF(isDebug(), "outCfg has not changed, skipping config");
        return NO_ERROR;
    }

    ALOGD_IF(isDebug(), "%s: previous config: %s", __func__,
        mOutCfg.dump().string());
    ALOGD_IF(isDebug(), "%s: new config: %s", __func__,
        outCfg.dump().string());

    if(outCfg != mOutCfg){
        err = VpuIntf::setOutputFormat(mFd, outCfg);
        ERR_CHK((NO_ERROR != err), err);

        //TODO: VpuIntf::setOutputCrop(mFd)
    }

    if(outCfg.colorspace != mOutCfg.colorspace){
        err = VpuIntf::setOutColorSpace(mFd, outCfg.colorspace);
        ERR_CHK((NO_ERROR != err), err);
    }

    if(outCfg.framerate != mOutCfg.framerate){
        err = VpuIntf::setOutputFrameRate(mFd, outCfg);
        ERR_CHK((NO_ERROR != err), err);
    }


    //Store settings in local for detecting whether config has updated
    mOutCfg = outCfg;

    return err;
}

/*
    Calls STREAM_ON on input and output ports. Also notifies event thread to
    listen to dequeue events
*/

status_t VPUSession::start()
{
    status_t err = NO_ERROR;

    //For non-vcap cases, need to reqbuf as well
    if((INPUT_SRC_SF_QBUF == mInCfg.inSrc.eInputSrcType) ||
        (INPUT_SRC_VFM_TUNNEL == mInCfg.inSrc.eInputSrcType)){
        if(!mBufQ){
            err = VpuIntf::reqInPortBufs(mFd, mInCfg, mNumDrvInBufs);
            ERR_CHK((NO_ERROR != err), err);
            mState |= REQ_BUFS_IN_PORT;

            mBufQ = new BufQ(mNumDrvInBufs, mDebugLogs);
        }
    }

    if(!mOutBufPool){
        v4l2bufptr          bufPtr;

        err = VpuIntf::reqOutPortBufs(mFd, mOutCfg, mNumDrvOutBufs);
        ERR_CHK((NO_ERROR != err), err);
        mState |= REQ_BUFS_OUT_PORT;

        mOutBufPool = new OutBufPool(mDebugLogs);

        err = mOutBufPool->allocBufs(mNumDrvOutBufs, mOutCfg);
        if(NO_ERROR != err){
            delete mOutBufPool;
            mOutBufPool = NULL;
            return err;
        }
        for(int32_t i = 0; i < mNumDrvOutBufs; i++){
            bufPtr = mOutBufPool->getEmptyBuf(i);

            Dim sTgtDim;
            sTgtDim.width = mOutCfg.tgtRect.right - mOutCfg.tgtRect.left;
            sTgtDim.height = mOutCfg.tgtRect.bottom - mOutCfg.tgtRect.top;

            err = VpuIntf::fillAndQOutBuf(mFd, bufPtr, mOutCfg.sDim,
                sTgtDim, mOutCfg.pixFmt, mOutCfg.eField, ns2timeval(0));
            ERR_CHK((NO_ERROR != err), err);
        }
    }

    if(!(mState & STREAMON_IN_PORT)){
        err = VpuIntf::streamOnInPort(mFd);
        ERR_CHK((NO_ERROR != err), err);
        mState |= STREAMON_IN_PORT;
    }

    if(!(mState & STREAMON_OUT_PORT)){
        err = VpuIntf::streamOnOutPort(mFd);
        ERR_CHK((NO_ERROR != err), err);
        mState |= STREAMON_OUT_PORT;
    }

    if(!(mState & LISTEN_TO_DEQUEUE_EVENT)){
        err = mEventThread->listenToDequeueEvents(mFd);
        ERR_CHK((NO_ERROR != err), err);
        mState |= LISTEN_TO_DEQUEUE_EVENT;
    }

    return err;
}

//Synchronous
status_t VPUSession::stop()
{
    status_t err = NO_ERROR;
    ALOGD_IF(isDebug(), "%s :E", __func__);

    if(mState & LISTEN_TO_DEQUEUE_EVENT){
        err = mEventThread->turnOffDequeueEvents(mFd);
        ERR_CHK((NO_ERROR != err), err);
        mState &= ~LISTEN_TO_DEQUEUE_EVENT;
    }

    if(mState & STREAMON_IN_PORT){
        err = VpuIntf::streamOffInPort(mFd);
        ERR_CHK((NO_ERROR != err), err);
        mState &= ~STREAMON_IN_PORT;
    }

    if(mState & STREAMON_OUT_PORT){
        err = VpuIntf::streamOffOutPort(mFd);
        ERR_CHK((NO_ERROR != err), err);
        mState &= ~STREAMON_OUT_PORT;
    }
    return err;
}

//Synchronous
status_t VPUSession::teardown()
{
    status_t err = NO_ERROR;
    int32_t intErr = 0;
    ALOGD_IF(isDebug(), "%s :E", __func__);

    err = stop();
    if(NO_ERROR != err) intErr |= 1;

    if(mState & REQ_BUFS_IN_PORT){
        err = VpuIntf::reqInPortBufs(mFd, mInCfg, 0);
        if(NO_ERROR != err) intErr |= 1;
        mState &= ~REQ_BUFS_IN_PORT;
    }

    if(mState & SUBSCRIBE_TO_EVENTS){
        err = VpuIntf::unSubscribeToEvents(mFd);
        if(NO_ERROR != err) intErr |= 1;
        mState &= ~SUBSCRIBE_TO_EVENTS;
    }

    if(mState & REQ_BUFS_OUT_PORT){
        err = VpuIntf::reqOutPortBufs(mFd, mOutCfg, 0);
        if(NO_ERROR != err) intErr |= 1;
        mState &= ~REQ_BUFS_OUT_PORT;
    }

    if(mState & REGISTER_TO_EVENT_THREAD){
        //Un-register from event thread
        mEventThread->unregisterFd(mFd);
        mState &= ~REGISTER_TO_EVENT_THREAD;
    }

    if(mFd > 0 ){
        err = VpuIntf::closeFd(mFd);
        if(NO_ERROR != err) intErr |= 1;
    }

    return (intErr ? UNKNOWN_ERROR : NO_ERROR);
}

status_t VPUSession::getDestPipes(DestinationPipes& sDestPipes)
{
    status_t err = NO_ERROR;
    ALOGD_IF(isDebug(), "%s :E", __func__);

    sDestPipes = mOutDest.sDestPipes;

    return err;
}

//! Set destination pipe info if vpu session is in MDSS tunnel mode
status_t VPUSession::setDestPipes(const DestinationPipes& sDestPipes)
{
    status_t err = NO_ERROR;
    ALOGD_IF(isDebug(), "%s :E", __func__);

    if(mOutDest.sDestPipes != sDestPipes) {
        ALOGD_IF(isDebug(), "%s: sDestPipes info changed: prev: %s new: %s",
            __func__,mOutDest.sDestPipes.dump().string(),
            sDestPipes.dump().string());
        err = VpuIntf::setOutputDest(mFd, sDestPipes);
        ERR_CHK_LOG((NO_ERROR != err), err, "setDestPipes failed");

        mOutDest.sDestPipes = sDestPipes;
    }

    //STREAM ON needs to be called after the destination pipes are configured
    err = start();
    ERR_CHK((NO_ERROR != err), err);
    return err;
}

//! Gets an empty buffer from local queue and sends it to the vpu
//   queues the buffer only if the handle is updated since last call
status_t VPUSession::processQBuf(const private_handle_t* handle,
                                    int32_t& releaseFenceFd)
{
    status_t err = NO_ERROR;
    ALOGD_IF(isDebug(), "%s :E", __func__);
    v4l2bufptr bufPtr;

    //Initializing releaseFenceFd in case this function exits on error
    releaseFenceFd = -1;

    ERR_CHK_LOG((NULL == handle), BAD_VALUE, "Buffer handle is NULL");

    //If same handle is queued again, do not queue to VPU. Return the
    //  cached fenceFd
    if(handle == mPvtHandle){
        releaseFenceFd = mCachedReleaseFenceFd;
        return NO_ERROR;
    }

    /* If buffer pool is not created, return without queuing */
    if(!mBufQ){
        return NO_MEMORY;
    }

    /* Dump into a file for debug puposes */
    if(mDebugFileDump){
        static int i = 0;
        int32_t w = mInCfg.sCrop.right - mInCfg.sCrop.left;
        int32_t h = mInCfg.sCrop.bottom - mInCfg.sCrop.top;
        int32_t s = mInCfg.sDim.width;

        ALOGD_IF(isDebug(), "%s: dumpFile w: %d h: %d s: %d pf: %d frmCnt: %d",
            __func__, w, h , s, mInCfg.pixFmt, i);
        dumpFile("/data/vpuSess.yuv", (char*)handle->base, w, h, s,
                    mInCfg.pixFmt, i);
    }

    //To avoid re-mmapping at the driver, obtain the corresponding buffer slot
    bufPtr = mBufQ->getEmptyBuf((unsigned long) handle->fd);
    //If not present, try to get an empty buffer which is unassigned
    if(NULL == bufPtr){
        ALOGD_IF(mDbgBufQ, "%s: buf did not find fd: %d, "
            "trying getNextUnassignedEmptyBuf", __func__, handle->fd);
        bufPtr = mBufQ->getNextUnassignedEmptyBuf();
    }
    //If not available, try to get an empty buffer even though it was prviously
    //assigned
    if(NULL == bufPtr){
        ALOGD_IF(mDbgBufQ, "%s: buf did not getNextUnassignedEmptyBuf, "
            "trying getNextEmptyBuf", __func__);
        bufPtr = mBufQ->getNextEmptyBuf();
    }
    //TODO: This case should not happen. buffers should be de-queued by now
    if(NULL == bufPtr){
        ALOGE("%s No free buffers", __func__);
        return NO_MEMORY;
    }
    mBufQ->assignUsrPtr(bufPtr, (unsigned long) handle->fd);
    mBufQ->assignPvtHandle(const_cast<private_handle_t*>(handle),
        bufPtr->index);
    ALOGD_IF(mDbgBufQ, "%s: assigning fd: %d bufId: %d", __func__, handle->fd,
            bufPtr->index);

    if(!mStats.mNumBufsQueuedIn){
        mStats.mFirstFrameQTime = gettimeofdayMs();
    }
    mStats.mNumBufsQueuedIn++;
    if(mStats.mLoopback){
        mBufQ->storeEmptyBuf(bufPtr->index);
    }else{
        int64_t timestampNs = getTimeStamp(handle);
        if(mDbgDisableTimestamp)
            timestampNs = 0;
        struct timeval timestampTv = ns2timeval(timestampNs);
        if(mFrmRptEnable){
        /* In this mode, repeat count is sent as timestamp field */
             mFrameRepeat.getRepeatCount(timestampTv);
        }
        if(mDbgDumpPixelVal &&
            (mInCfg.sDim.width * mInCfg.sDim.height > 40000)){
            int32_t last = mInCfg.sDim.width * mInCfg.sDim.height / 4 - 10000;
            ALOGD("Q bufid : %d base: %d pixels [10000]: %d, [10001]: %d "
                "[%d]: %d [%d]: %d", bufPtr->index, handle->base,
                 ((int*)(handle->base))[10000], ((int*)(handle->base))[10001],
                 last - 1, ((int*)(handle->base))[last - 1],
                 last, ((int*)(handle->base))[last] );
        }
        err = VpuIntf::fillAndQInBuf(mFd, bufPtr, mInCfg.sStride, mInCfg.sDim,
                          mInCfg.pixFmt, mInCfg.eField, timestampTv,
                          releaseFenceFd);
        ERR_CHK((NO_ERROR != err), err);
        ALOGD_IF(logAll(), "%s: tsNs: %lld, tv_sec:: %ld tv_usec: %ld",
            __func__, timestampNs, timestampTv.tv_sec, timestampTv.tv_usec);
    }
    mStats.mNumBufsQueuedToDriver++;

    ALOGD_IF(mStats.mEnable, "Q: %s", dumpStats().string());
    ALOGD_IF(mStats.mEnable, "Q: time since last queue: %d",
            gettimeofdayMs()-mStats.mPrevFrameQTime);
    mStats.mPrevFrameQTime = gettimeofdayMs();

    /* stored locally to filter repeated calls later */
    mPvtHandle = const_cast<private_handle_t *>(handle);
    mCachedReleaseFenceFd = releaseFenceFd;

    return err;
}

status_t VPUSession::processCmdSetVpuControl(const vfmCmdInArg_t& inArg,
        vfmCmdOutArg_t& outArg)
{
    status_t err = NO_ERROR;
    ALOGD_IF(isDebug(), "%s: E", __func__);

    //Any un-supported control_ids can be blocked here
    err = VpuIntf::setVpuControl(mFd, inArg.iaVpuSetCtrl);
    ERR_CHK((NO_ERROR != err), err);

    return err;
}

status_t VPUSession::processCmdGetVpuControl(const vfmCmdInArg_t& inArg,
        vfmCmdOutArg_t& outArg)
{
    status_t err = NO_ERROR;
    ALOGD_IF(isDebug(), "%s: E", __func__);

    //Any un-supported control_ids can be blocked here
    err = VpuIntf::getVpuControl(mFd, outArg.oaVpuGetCtrl);
    ERR_CHK((NO_ERROR != err), err);

    return err;
}

status_t VPUSession::onVpuDequeueEventWrapper(void *me, int32_t isInputPort)
{
    return static_cast<VPUSession *>(me)->onVpuDequeueEvent(isInputPort);
}

status_t VPUSession::onVpuDequeueEvent(int32_t isInputPort)
{
    status_t err = NO_ERROR;
    int32_t bufIndex = -1;

    ALOGD_IF(logAll(), "%s: E isInputPort: %d", __func__, isInputPort);

    if(isInputPort){
        mStats.mNumDequeueEventsFromDriver++;
        /* dequeue the buffer and release to bufferQueue */
        err = VpuIntf::dQInBuf(mFd, bufIndex);
        ERR_CHK((NO_ERROR != err), err);
        mStats.mNumBufsDequeuedFromDriver++;

        if(NO_ERROR == err){
            private_handle_t* handle = mBufQ->getPvtHandle(bufIndex);
            ALOGD_IF(mDbgBufQ, "%s: dq index: %d handle: %p base: %d",
                __func__, bufIndex, handle, handle->base);
            if(handle && mDbgDumpPixelVal &&
                (mInCfg.sDim.width * mInCfg.sDim.height > 40000)){
                int32_t last = mInCfg.sDim.width * mInCfg.sDim.height / 4 -
                            10000;
                ALOGD("DQ bufid: %d base: %d pixels [10000]: %d, [10001]: %d [%d]: %d "
                    "[%d]: %d", bufIndex, handle->base,
                 ((int*)(handle->base))[10000], ((int*)(handle->base))[10001],
                 last - 1, ((int*)(handle->base))[last - 1],
                 last, ((int*)(handle->base))[last] );

            }
            mBufQ->storeEmptyBuf(bufIndex);
        }
        ALOGD_IF(mStats.mEnable, "DeQ: %s", dumpStats().string());
        ALOGD_IF(mStats.mEnable, "DeQ: time since last dequeue: %d",
                gettimeofdayMs()-mStats.mPrevFrameDQTime);
        mStats.mPrevFrameDQTime = gettimeofdayMs();
    }
#if DBG_VPUOUT_HOST
    else{
        /* dequeue the buffer and queue it back to the driver */
        err = VpuIntf::dQOutBuf(mFd, bufIndex);
        ERR_CHK((NO_ERROR != err), err);

        if(mOutBufPool){
            void* data = mOutBufPool->getBufDataPtr(bufIndex);
            if(data){
                dumpFile("/data/vpuOut.dat", (char*) data,
                mOutCfg.tgtRect.right - mOutCfg.tgtRect.left,
                mOutCfg.tgtRect.bottom - mOutCfg.tgtRect.top,
                mOutCfg.tgtRect.right - mOutCfg.tgtRect.left,
                mOutCfg.pixFmt, mDbgOutFrmNum);
                ALOGD_IF(logAll(), "%s: out dataptr: %p", __func__, data);
            }
            /* queue the buffer back to the driver*/
            v4l2bufptr bufPtr = mOutBufPool->getEmptyBuf(bufIndex);
            if(bufPtr){
                Dim sTgtDim;
                sTgtDim.width = mOutCfg.tgtRect.right - mOutCfg.tgtRect.left;
                sTgtDim.height = mOutCfg.tgtRect.bottom - mOutCfg.tgtRect.top;
                err = VpuIntf::fillAndQOutBuf(mFd, bufPtr, mOutCfg.sDim,
                    sTgtDim, mOutCfg.pixFmt, mOutCfg.eField, 0);
                ERR_CHK((NO_ERROR != err), err);
            }
        }
    }
#endif
    return err;
}

status_t VPUSession::onVpuEventWrapper(void *me)
{
    return static_cast<VPUSession *>(me)->onVpuEvent();
}

status_t VPUSession::onVpuEvent()
{
    status_t err = NO_ERROR;
    struct v4l2_event   v4l2event;
    VFM_EVENT_TYPE      event;
    vfmEvtPayload_t        evtPayload;

    ALOGD_IF(logAll(), "%s: E", __func__);
    err = VpuIntf::dQEvent(mFd, v4l2event);
    ERR_CHK((NO_ERROR != err), err);

    /* translate V4l2_event to EVENT_TYPE */
    switch(v4l2event.type){
        default:
            event = VFM_EVT_START;
    }

    if(mNotifyCb.eventHdlr){
        mNotifyCb.eventHdlr(event, evtPayload, mSessId, mNotifyCb.cookie);
    }
    return err;
}

/*****************************************************************************
    VPUSessionVcapTunnel methods
*****************************************************************************/
status_t VPUSessionVcapTunnel::processCommand(const uint32_t command,
        const vfmCmdInArg_t& inArg, vfmCmdOutArg_t& outArg)
{
    ALOGD_IF(logAll(), "%s: vpuSessId: %d cmd: %d", __func__, mSessId,command);
    /* Invalid operations */
    if((command >= VFM_CMD_BUFFER_START) && (command <= VFM_CMD_BUFFER_END)){
        return INVALID_OPERATION;
    }

    VPUSession::processCommand(command, inArg, outArg);
    return NO_ERROR;
}

/*****************************************************************************
    VPUSessionVfmTunnel methods
*****************************************************************************/
status_t VPUSessionVfmTunnel::processCommand(const uint32_t command,
        const vfmCmdInArg_t& inArg, vfmCmdOutArg_t& outArg)
{
    status_t err = NO_ERROR;
    ALOGD_IF(logAll(), "%s: vpuSessId: %d cmd: %d", __func__, mSessId,command);

    if((command >= VFM_CMD_BUFFER_START) && (command <= VFM_CMD_BUFFER_END)){
        /* Handle the exclusive command list applicable to VFM tunnel mode */
        switch(command){
            case VFM_CMD_REGISTER_BUFFER:
                err = processCmdRegBuf(inArg, outArg);
            break;

            case VFM_CMD_DQ_BUF:
                err = processCmdDqBuf(inArg, outArg);
            break;

            case VFM_CMD_Q_BUF:
                err = processCmdQBuf(inArg, outArg);
            break;

            case VFM_CMD_CANCEL_BUF:
                err = processCmdCancelBuf(inArg, outArg);
            break;

            default:
                ALOGE("%s: Invalid command: %d", __func__, command);
        }
    }
    else {
        /* the base class processing of the commands */
        err = VPUSession::processCommand(command, inArg, outArg);
    }
    return err;
}

/* Store handle in local buffer slot and assign FD accordingly */
status_t VPUSessionVfmTunnel::processCmdRegBuf(
        const vfmCmdInArg_t& inArg, vfmCmdOutArg_t& outArg)
{
    v4l2bufptr bufPtr = NULL;
    hw_module_t const* grallocHwModule = NULL;
    gralloc_module_t const* grallocModule = NULL;
    int alignedw, alignedh;
    int err;

    ALOGD_IF(isDebug(), "RegBuf: id: %d hnd: %p", inArg.iaBufReg.id,
                inArg.iaBufReg.hnd);

    if(NULL == inArg.iaBufReg.hnd){
        ALOGE("%s: NULL handle for RegBuf: %d", __func__, inArg.iaBufReg.id);
        return BAD_VALUE;
    }

    /* Register the handle with gralloc module so that hnd->base_metadata and
     * hnd->base members are populated */
    err = hw_get_module(GRALLOC_HARDWARE_MODULE_ID, &grallocHwModule);
    if (err != 0) {
        ALOGE("%s: GRALLOC hw_get_module failed. Error No -%d\n",
            __func__, err);
        return UNKNOWN_ERROR;
    }
    grallocModule = (gralloc_module_t const *)grallocHwModule;
    grallocModule->registerBuffer(grallocModule, inArg.iaBufReg.hnd);

    if(mBufQ){
        bufPtr = mBufQ->getBufPtr(inArg.iaBufReg.id);
        if(bufPtr){
            mBufQ->assignUsrPtr(bufPtr, (unsigned long)inArg.iaBufReg.hnd->fd);
            mBufQ->assignPvtHandle(inArg.iaBufReg.hnd, inArg.iaBufReg.id);
        }
    }else {
        ALOGE("%s: BufQ not yet initialized", __func__);
        return NO_MEMORY;
    }

    return NO_ERROR;
}

/* Dequeue next available empty buffer */
status_t VPUSessionVfmTunnel::processCmdDqBuf(
        const vfmCmdInArg_t& inArg, vfmCmdOutArg_t& outArg)
{
    v4l2bufptr bufPtr = NULL;
    outArg.oaDQBuf.id = -1;
    ALOGD_IF(logAll(), "%s :E", __func__);

    if(mBufQ){
        bufPtr = mBufQ->getNextAssignedEmptyBuf();
        if(bufPtr){
            outArg.oaDQBuf.id = bufPtr->index;
            ALOGD_IF(logAll(), "DqBuf: %d", outArg.oaDQBuf.id);
        }
    }else {
        ALOGE("%s: BufQ not yet initialized", __func__);
        return NO_MEMORY;
    }
    return NO_ERROR;
}

status_t VPUSessionVfmTunnel::processCmdQBuf(
        const vfmCmdInArg_t& inArg, vfmCmdOutArg_t& outArg)
{
    status_t err = NO_ERROR;
    v4l2bufptr bufPtr = NULL;
    private_handle_t* handle = NULL;
    ALOGD_IF(logAll(), "QBuf: %d", inArg.iaQBuf.id);

    if(mBufQ){
        bufPtr = mBufQ->getBufPtr(inArg.iaQBuf.id);
        if(bufPtr){
            handle = mBufQ->getPvtHandle(inArg.iaQBuf.id);
            /* Read all the configuration from metadata */
            InputCfg inCfg;

            metadata2InBufGeometry(handle, inCfg);
            metadata2InInterlace(handle, inCfg);
            metadata2In3D(handle, inCfg);
            metadata2InColorSpace(handle, inCfg);
            metadata2InFramerate(handle, inCfg);
            //TODO: If dynamic changes are supported, call processInOutCfg
            if(!mStats.mNumBufsQueuedIn){
                mStats.mFirstFrameQTime = gettimeofdayMs();
            }
            mStats.mNumBufsQueuedIn++;

            int64_t timestampNs = getTimeStamp(handle);
            if(mDbgDisableTimestamp)
                timestampNs = 0;
            struct timeval timestampTv = ns2timeval(timestampNs);
            if(mFrmRptEnable){
            /* In this mode, repeat count is sent as timestamp field */
                 mFrameRepeat.getRepeatCount(timestampTv);
            }
            if(mDbgDumpPixelVal &&
                (mInCfg.sDim.width * mInCfg.sDim.height > 40000)){
                int32_t last = mInCfg.sDim.width * mInCfg.sDim.height / 4 -
                        10000;
                ALOGD("Q bufid : %d base: %d pixels [10000]: %d, [10001]: %d [%d]: %d "
                "[%d]: %d", bufPtr->index, handle->base,
                 ((int*)(handle->base))[10000], ((int*)(handle->base))[10001],
                 last - 1, ((int*)(handle->base))[last - 1],
                 last, ((int*)(handle->base))[last] );
            }
            err = VpuIntf::fillAndQInBuf(mFd, bufPtr, mInCfg.sStride,
                mInCfg.sDim, mInCfg.pixFmt, mInCfg.eField,
                timestampTv, outArg.oaQBuf.releaseFenceFd);
            ERR_CHK((NO_ERROR != err), err);
            mStats.mNumBufsQueuedToDriver++;
            ALOGD_IF(logAll(), "%s: tsNs: %lld, tv_sec:: %ld tv_usec: %ld",
                __func__, timestampNs, timestampTv.tv_sec,
                timestampTv.tv_usec);
            ALOGD_IF(mStats.mEnable, "Que: %s", dumpStats().string());
            ALOGD_IF(mStats.mEnable, "Q: time since last queue: %d",
                    gettimeofdayMs()-mStats.mPrevFrameQTime);
            mStats.mPrevFrameQTime = gettimeofdayMs();
            /* Mark that this buffer is with consumer so that it is not */
            /* dequeued untile consumer releases this buffer */
            mBufQ->markWithConsumer(inArg.iaQBuf.id);
        }
    }else {
        ALOGE("%s: BufQ not yet initialized", __func__);
        return NO_MEMORY;
    }

    return err;
}

status_t VPUSessionVfmTunnel::processCmdCancelBuf(
        const vfmCmdInArg_t& inArg, vfmCmdOutArg_t& outArg)
{
    ALOGD_IF(isDebug(), "CancelBuf: %d", inArg.iaCancelBuf.id);
    if(!mBufQ){
        ALOGE("%s: BufQ not yet initialized", __func__);
        return NO_MEMORY;
    }
    mBufQ->storeEmptyBuf(inArg.iaCancelBuf.id);

    return NO_ERROR;
}

/*****************************************************************************
    VPUSfBuffers methods
*****************************************************************************/
status_t VPUSessionSfBuffers::processCommand(const uint32_t command,
        const vfmCmdInArg_t& inArg, vfmCmdOutArg_t& outArg)
{
    ALOGD_IF(logAll(), "%s: vpuSessId: %d cmd: %d", __func__, mSessId,command);
    /* Invalid operations */
    if((command >= VFM_CMD_BUFFER_START) && (command <= VFM_CMD_BUFFER_END)){
        return INVALID_OPERATION;
    }

    VPUSession::processCommand(command, inArg, outArg);
    return NO_ERROR;
}

}; //namespace vpu
