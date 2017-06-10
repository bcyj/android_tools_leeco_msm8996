/* Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
 /*****************************************************************************
    This file contains declares Video Flow class. Video flow converts layer
    and metadata into input and output config of VPU. VideoFlow provides an
    interface between "layer" and vpu "session". VideoFlow also keeps
    association between post processing configurations and vpu sessions.
 *****************************************************************************/

#ifndef VFM_VIDEO_FLOW_H
#define VFM_VIDEO_FLOW_H
#include <sys/types.h>
#include "vfm_defs.h"
#include "vfm_vpusession.h"

//using namespace android;
namespace vpu {

class VidFlow {
public:
    VidFlow(int flowId, EventThread*, const notifyCb_t&, int enableDebugLogs);
    virtual ~VidFlow();

    virtual status_t init(Layer*);

    //To be called in subsequent updates
    virtual status_t updateLayer(Layer*);

    //TODO: 2  Main-PiP swap
    //virtual status_t setLayer(Layer* inLayer, Layer* outLayer);
    //virtual status_t setLayerInCfg(Layer*);
    //virtual status_t setLayerOutCfg(Layer*);

    //To be called at every draw call
    virtual status_t qBuf(private_handle_t* handle, int32_t& releaseFenceFd);

    //To be called when a previously existing layer is dropped
    virtual status_t teardown();

    virtual status_t getDestPipes(DestinationPipes& sDestPipes){
        return mVpuSess->getDestPipes(sDestPipes);
    }
    virtual status_t setDestPipes(const DestinationPipes& sDestPipes){
        return mVpuSess->setDestPipes(sDestPipes);
    }
    virtual status_t processCommand(const uint32_t command,
        const vfmCmdInArg_t& inArg, vfmCmdOutArg_t& outArg);
    void setDisplayAttr(const vfmInArgSetDisplayAttr_t&);

    virtual int32_t isAvailable() { return mState!= RUNNING;}
    virtual int32_t isActive() { return mState == RUNNING; }
    virtual inline int32_t vidFlowId() { return mFlowId; }
    //Valid only if isActive() returns "true"
    virtual inline int32_t mediaSessId() { return mMediaSessId; }

private:
    //TBD: Hide evil constructors
    VidFlow();

    //VideoFlow State machine STATES
    typedef enum {
        UN_INIT    = 0x00,
        INIT_DONE  = 0x01,
        RUNNING    = 0x02,
    } STATE;

    //Whether buffers will be queued on Draw call
    typedef enum {
        Q_ON_DRAW,
        NO_Q_ON_DRAW
    } QUEUE_MODE;

    struct DisplayAtrribs_t {
        int32_t width;
        int32_t height;
        int32_t fp100s;
    };

    /* loglevels: 1: Debug (generally config) 2: All */
    int32_t logAll() { return  (mDebugLogs >= 2);}
    int32_t isDebug() { return (mDebugLogs >= 1); }

    //resets all the members except mFlowId and mDebugLogs
    status_t reset();

    status_t layer2InOutCfg(Layer*, InputCfg&, OutputCfg&);
    status_t layer2InOutCfgVcapTunnel(Layer*, InputCfg&, OutputCfg&);
    status_t layer2InOutCfgVfmTunnel(Layer*, InputCfg&, OutputCfg&);
    status_t layer2InOutCfgSfBuffers(Layer*, InputCfg&, OutputCfg&);

    void getSrcCrop(InputCfg&);
    void getDestRect(OutputCfg&);
    void getOutFramerate(InputCfg&, OutputCfg&);
    void getOutPixFmt(InputCfg& inCfg, OutputCfg& outCfg);
    void getOutColorspace(InputCfg& inCfg, OutputCfg& outCfg);
    void getOutBufferDim(InputCfg& inCfg, OutputCfg& outCfg);
    void getOut3DFormat(OutputCfg& outCfg);
    void getOutField(OutputCfg& outCfg);

    VPUSession* mVpuSess;

    //State variable for state machine implementation
    STATE       mState;

    //Data buffer queue mode
    QUEUE_MODE  mQMode;

    //TODO: Updated during setLayer()
    int32_t     mMediaSessId;

    //A unique identifier
    int32_t      mFlowId;
    EventThread* mEventThread;
    notifyCb_t   mNotifyCb;
    INPUT_SOURCE_TYPE mSrc;
    int32_t      mEnable10bitBWC;
    //Whether to enable debug logs
    int32_t      mDebugLogs;

    //Display attributes
    DisplayAtrribs_t mDispAttr[DISP_MAX];
};

}; //namespace VPU

#endif /* end of include guard: VFM_VIDEO_FLOW_H */
