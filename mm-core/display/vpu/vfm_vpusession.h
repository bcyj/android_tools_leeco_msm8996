/* Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
 /*****************************************************************************
    This file declares VPU Session class. Each VPUSession object manages one
    VPU session.
 *****************************************************************************/

#ifndef VFM_VPU_SESSION_H
#define VFM_VPU_SESSION_H

#include <sys/types.h>
#include <utils/Log.h>
#include <utils/Trace.h>
#include <utils/Looper.h>
#include <utils/Thread.h>
#include "vfm_defs.h"
#include "vfm_utils.h"
#include "vfm_bufqueue.h"
#include "vfm_vpuinterface.h"

using namespace android;

namespace vpu {

//This is just number of driver buffer slots and not actual data buffers
#define NUM_DRIV_IN_BUFS (32)

#define NUM_DRIV_OUT_BUFS (3)

/* Forward declarations */

class VPUSession {
public:
    VPUSession(int vpuSessionId, EventThread*, const notifyCb_t&,
                int32_t enableDebugLogs);

    //Attaches to the VPU session on this call
    virtual status_t init();

    //Configures all the input and output config
    virtual void getInOutCfg(InputCfg& inCfg, OutputCfg& outCfg){
        inCfg = mInCfg;
        outCfg = mOutCfg;
    }

    //Configures all the input and output config
    virtual status_t setInOutCfg(const InputCfg&, const OutputCfg&);

    //To be called at every prepare call
    virtual status_t getDestPipes(DestinationPipes& sDestPipes);

    //To be called at every prepare call
    virtual status_t setDestPipes(const DestinationPipes& sDestPipes);

    //Corresponds to STREAM_OFF and close down of the session
    virtual status_t teardown();

    virtual status_t qBuf(const private_handle_t* handle, int32_t&
                            releaseFenceFd);
    //TBD:KW explicit dequeue is not required due to fence operation
    //dequeueBuffer();

    virtual status_t processCommand(const uint32_t command,
        const vfmCmdInArg_t& inArg, vfmCmdOutArg_t& outArg);

    //Returns true when a VPU session is available
    virtual int32_t isAvailable() { return mState & SESSION_ATTACHED ;}

    //Returns true when a VPU session has started
    virtual int32_t isActive() {
        return (mState & STREAMON_IN_PORT) && (mState & STREAMON_OUT_PORT);
    }

    virtual int getFd() { return mFd; }
    virtual int getSessId() { return mSessId; }
    virtual ~VPUSession();

protected:
    status_t                initStats();
    String8                 dumpStats();
    struct Stats {
        int32_t             mFirstFrameQTime;
        int32_t             mPrevFrameQTime;
        int32_t             mPrevFrameDQTime;
        int32_t             mEnable;
        int32_t             mLoopback;
        int32_t             mNumBufsQueuedIn;
        int32_t             mNumBufsQueuedToDriver;
        int32_t             mNumBufsDequeuedFromDriver;
        int32_t             mNumDequeueEventsFromDriver;
    }mStats;

    class FrameRepeat {
    public:
        FrameRepeat(){
            mInitialBuffering   = 0;
            mFrameCount         = 0;
            mFrameRateRatio     = 0;
            mToggle             = 0;
        }

        status_t    setInitBufDelay(int32_t initBufferDelay){
            mInitialBuffering = initBufferDelay;
            return NO_ERROR;
        }

        status_t    setFrameRate(int32_t inFp100s, int32_t
                                outFp100s){
            mInFp100s       = inFp100s;
            mOutFp100s      = outFp100s;
            mFrameRateRatio = (float)outFp100s / inFp100s;
            mToggle         = 0;
            ALOGD("%s: infps: %d outfps: %d ratio: %f",
                    __func__, inFp100s, outFp100s, mFrameRateRatio);
            return NO_ERROR;
        }

        status_t    getRepeatCount(struct timeval& rpt){
            int32_t repeatCount = 1;
            /* Range is defined to be able to handle fractional fps */
            if((1.9 <= mFrameRateRatio) && (mFrameRateRatio < 2.1)){
                repeatCount = 2;
            }
            if((2.45 <= mFrameRateRatio) && (mFrameRateRatio < 2.6)){
                if(mToggle){
                    repeatCount = 2;
                    mToggle = 0;
                }else{
                    repeatCount = 3;
                    mToggle = 1;
                }
            }
            if(0 == mFrameCount){
                /* The first 32 bits indicate initial buffering */
                /* and the last 32 bits indicate actual repeat */
                rpt.tv_sec = mInitialBuffering;
                rpt.tv_usec = repeatCount;
            }else{
                rpt.tv_sec = 0;
                rpt.tv_usec = repeatCount;
            }
            mFrameCount++;

            return NO_ERROR;
        }

        status_t    getInitBufferCount() { return mInitialBuffering; }

    private:
        int32_t             mInitialBuffering;
        int32_t             mFrameCount;
        int32_t             mInFp100s;
        int32_t             mOutFp100s;
        float               mFrameRateRatio;
        int32_t             mToggle;
    };

    /* loglevels: 1: Debug (generally config) 2: All */
    int32_t logAll() { return  (mDebugLogs >= 2);}
    int32_t isDebug() { return (mDebugLogs >= 1); }

    FrameRepeat             mFrameRepeat;
    int32_t                 mFd;           //<! session specific fd
    BufQ*                   mBufQ;
    int32_t                 mDebugLogs;
    InputCfg                mInCfg;
    int32_t                 mSessId;       //<! Vpu session/channel number
    int32_t                 mDbgOutFrmNum;
    int32_t                 mFrmRptEnable;
    int32_t                 mVpuInitBufDelay;
    int32_t                 mVpuOutBuffers;
    int32_t                 mDbgDumpPixelVal;
    int32_t                 mDbgBufQ;
    int32_t                 mDbgDisableTimestamp;

private:
    //TBD: Hide evil constructors
    VPUSession();

    //APIs called by VPU session controller
    enum
    {
        FD_CLOSE                = 0,
        FD_OPEN                 = 0x0001,
        SESSION_ATTACHED        = 0x0002,
        SUBSCRIBE_TO_EVENTS     = 0x0004,
        REGISTER_TO_EVENT_THREAD = 0x0008,
        COMMIT_CONFIG           = 0x0010,
        REQ_BUFS_IN_PORT        = 0x0020,
        REQ_BUFS_OUT_PORT       = 0x0040,
        STREAMON_IN_PORT        = 0x0080,
        STREAMON_OUT_PORT       = 0x0100,
        LISTEN_TO_DEQUEUE_EVENT = 0x0200,
    };

    status_t processSetInOutCfg(const InputCfg& inCfg,
                const OutputCfg& outCfg);

    status_t setInputCfg(const InputCfg& inCfg);

    status_t setOutputCfg(const OutputCfg& outCfg);

    //Specify the input source - VCAP/HLOS qbuf
    status_t setInputSource(InputSource*);

    status_t processQBuf(const private_handle_t* handle,
                            int32_t& releaseFenceFd);

    status_t processCmdSetVpuControl(
        const vfmCmdInArg_t& inArg, vfmCmdOutArg_t& outArg);

    status_t processCmdGetVpuControl(
        const vfmCmdInArg_t& inArg, vfmCmdOutArg_t& outArg);

    //Corresponds to STREAM_ON
    virtual status_t start();

    //Corresponds to STREAM_OFF
    virtual status_t stop();

    //VpuEvent handler
    static status_t onVpuEventWrapper(void* me);
    status_t onVpuEvent();

    //Dequeue event handler
    static status_t onVpuDequeueEventWrapper(void* me, int32_t isInputPort);
    status_t onVpuDequeueEvent(int32_t isInputPort);

    OutBufPool*             mOutBufPool;
    Mutex                   mMutex;
    EventThread*            mEventThread;
    notifyCb_t              mNotifyCb;
    int32_t                 mNumDrvInBufs;
    int32_t                 mNumDrvOutBufs;
    int32_t                 mState;
    OutputCfg               mOutCfg;
    OutputDestination       mOutDest;
    private_handle_t*       mPvtHandle;
    int32_t                 mCachedReleaseFenceFd;
    int32_t                 mDebugFileDump;
};

class VPUSessionVcapTunnel : public VPUSession {
public:
    VPUSessionVcapTunnel(int vpuSessionId, EventThread* evtThread,
                const notifyCb_t& notifyCb, int32_t enableDebugLogs)
         : VPUSession(vpuSessionId, evtThread, notifyCb, enableDebugLogs){
            ALOGD_IF(isDebug(), "%s: E", __func__);
         }
    virtual ~VPUSessionVcapTunnel() {
            ALOGD_IF(isDebug(), "%s: E", __func__);
        }

    virtual status_t processCommand(const uint32_t command,
        const vfmCmdInArg_t& inArg, vfmCmdOutArg_t& outArg);

    /* For VcapTunnel, buffers are tunnelled in hw to VPU and not queued from
     * VFM */
    virtual status_t qBuf(const private_handle_t* handle){
        return INVALID_OPERATION;
    }
private:
    VPUSessionVcapTunnel();
};

class VPUSessionVfmTunnel : public VPUSession {
public:
    VPUSessionVfmTunnel(int vpuSessionId, EventThread* evtThread,
                const notifyCb_t& notifyCb, int32_t enableDebugLogs)
         : VPUSession(vpuSessionId, evtThread, notifyCb, enableDebugLogs){
            ALOGD_IF(isDebug(), "%s: E", __func__);
         }

    virtual ~VPUSessionVfmTunnel() {
            ALOGD_IF(isDebug(), "%s: E", __func__);
        }

    virtual status_t processCommand(const uint32_t command,
        const vfmCmdInArg_t& inArg, vfmCmdOutArg_t& outArg);

    virtual status_t processCmdRegBuf(
        const vfmCmdInArg_t& inArg, vfmCmdOutArg_t& outArg);

    virtual status_t processCmdDqBuf(
        const vfmCmdInArg_t& inArg, vfmCmdOutArg_t& outArg);

    virtual status_t processCmdQBuf(
        const vfmCmdInArg_t& inArg, vfmCmdOutArg_t& outArg);

    virtual status_t processCmdCancelBuf(
        const vfmCmdInArg_t& inArg, vfmCmdOutArg_t& outArg);

    /* For VfmTunnel, buffers are queued using processCommand */
    virtual status_t qBuf(const private_handle_t* handle){
        return INVALID_OPERATION;
    }
private:
    VPUSessionVfmTunnel();

};

class VPUSessionSfBuffers : public VPUSession {
public:
    VPUSessionSfBuffers(int vpuSessionId, EventThread* evtThread,
                const notifyCb_t& notifyCb, int32_t enableDebugLogs)
         : VPUSession(vpuSessionId, evtThread, notifyCb, enableDebugLogs){
            ALOGD_IF(isDebug(), "%s: E", __func__);
         }
    virtual ~VPUSessionSfBuffers() {
            ALOGD_IF(isDebug(), "%s: E", __func__);
        }

    virtual status_t processCommand(const uint32_t command,
        const vfmCmdInArg_t& inArg, vfmCmdOutArg_t& outArg);

private:
    VPUSessionSfBuffers();
};
}; //namespace VPU

#endif /* end of include guard: VFM_VPU_SESSION_H */
