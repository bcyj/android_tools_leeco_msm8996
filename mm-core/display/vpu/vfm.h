/* Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

/*****************************************************************************
    vfm.h
    Video Flow Manager class declaration
*****************************************************************************/

#ifndef VFM_H
#define VFM_H
#include <sys/types.h>
#include <utils/Mutex.h>
#include "vfm_defs.h"
#include "vfm_cmds.h"
#include "vfm_utils.h"

//using namespace android;
namespace vpu {

//Forward declaration
class VFM;
class VidFlow;

/*!
  @brief         Returns the VFM object

  @param[in]     None

  @return        VFM object

  @note    Synchronous function
 */
extern "C" VFM* getVfmObject();

/*!
  @brief         Since its a singleton object, does nothing.

  @param[in]     VFM object

  @return        None

  @note    Synchronous function
 */
extern "C" void deleteVfmObject(VFM* object);

class VFM {
public:
    virtual ~VFM();

    // To be called during construction
    virtual status_t init();

    // To be called during construction to register notifyCb_t
    virtual status_t init(notifyCb_t*);

/*!
  @brief         Analyzes the layer list and marks the layers that will be
                 processed by VFM. Upon success, LayerList->Layer->
                 canVpuProcessLayer, Layer->reservePrevPipes and
                 Layer->sDestPipes  will be set appropriately

  @param[inout]  layerList pointer to the LayerList structure

  @param[in]     DISPLAY_ID corresponding to this LayerList

  @return        status_t Error.

  @note    Synchronous function
 */
    virtual status_t setupVpuSession(DISPLAY_ID, LayerList* layerList);

/*!
  @brief         Sets the MDSS pipe information to VFM

  @param[inout]  layerList pointer to the LayerList structure

  @param[in]     DISPLAY_ID corresponding to this LayerList

  @return        status_t Error.

  @note    Synchronous function
 */
    virtual status_t prepare(DISPLAY_ID, LayerList* layerList);

/*!
  @brief         Queues the buffers to VFM

  @param[in]     layerList pointer to the LayerList structure

  @param[in]     DISPLAY_ID corresponding to this LayerList

  @return        status_t Error.

  @note    Synchronous function
 */
    virtual status_t draw(DISPLAY_ID, LayerList* layerList);

/*!
  @brief         Setting properties

  @param[in]     command - key

  @param[in]     settting - value

  @return        status_t Error.

  @note    Synchronous function
 */
    virtual status_t processCommand(const uint32_t command,
                        const Parcel* inParcel, Parcel* outParcel);
/*
    Set the display attributes - fps and resolution of the display
*/
    virtual status_t setDisplayAttr(DISPLAY_ID, DispAttr_t&);

    //returns the singleton instance
    //TBD: N Not thread safe
    static VFM* getInstance();

private:
    //Singleton
    explicit VFM();
    VFM(VFM const&);
    void operator=(VFM const&);

    typedef enum {
        NO_VIEW,
        SINGLE_VIEW,
        MULTI_VIEW,
        MAIN_PIP_SWAP
    } VIEW_TYPE;

    typedef enum {
        DRV_UNINIT       = 0x00,
        DRV_INIT         = 0x01,
        VIDFLOW_INIT     = 0x02,
        MARK_VPU_LAYERS    = 0x03,
        PREPARE_VPU_LAYERS = 0x04,
        DRAW_VPU_LAYERS   = 0x05,
    } STATE;

    class LayerStats {
        public:
            int32_t mIdx;
            int32_t mMediaSessId;
            int32_t mPriority;
            int32_t mVidFlowExists;
            int32_t mNewlyCreatedVidFlow;
            int32_t mMarkedToClose;
    };

    struct DisplayAtrribs_t {
        int32_t width;
        int32_t height;
        int32_t fp100s;
    };

    // Returns the VPU capabilties
    status_t getCapabilities( VPUCapabilities& );

    status_t setupVpuSessionPrimary(LayerList* layerList);

    status_t preparePrimary(LayerList* layerList);

    status_t drawPrimary(LayerList* layerList);

    status_t setLayerStats(LayerList* layerList);

    status_t markMissingVidFlows(LayerList* layerList);

    status_t createVidFlowsIfReqd(LayerList *layerList);

    status_t updateVidFlows(LayerList *layerList);

    status_t closeMissingVidFlows(LayerList *layerList);

    status_t setVpuLayerFlag(LayerList *layerList);

    status_t setDestPipes(LayerList* layerList);

    status_t qBufs(LayerList* layerList);

    status_t processCommand(const uint32_t command,
                        const vfmCmdInArg_t& inArg, vfmCmdOutArg_t& outArg);

    status_t assertChecks();
    void dumpLayerStats();
    void clearLayerStats();
    void setDisplayAttr(const vfmInArgSetDisplayAttr_t&);

    bool isMediaSessIdPresent(int32_t mediaSessId);

    status_t closeVidFlow(VidFlow *vidFlow);

    int32_t doesVideoFlowExist(int32_t mediaSessId);

    VidFlow* getNextAvailableVidFlow();

    VidFlow* getVidFlow(int32_t mediaSessId);
    VidFlow* getVidFlowByFlowId(int32_t vidFlowId);

    inline void updateViewType();

    /* opens the VPU driver */
    status_t initVpu();

    /* callback to notify events */
    notifyCb_t mNotifyCb;

    //Local book keeping of Video flows
    int32_t  mActiveVidFlows;
    VidFlow* mVidFlow[MAX_VPU_SESSIONS];

    //Event thread
    EventThread* mEventThread;

    //Total number of vpu sessions
    int mTotalNumVpuSessions;

    //Display attributes
    DisplayAtrribs_t dispAttr[DISP_MAX];

    /* loglevels: 1: Debug (generally config) 2: All */
    int32_t mDebugLogs;
    int32_t logAll() { return  (mDebugLogs >= 2);}
    int32_t isDebug() { return (mDebugLogs >= 1); }

    //To query VPU capabilities etc
    int mFd;

    //Singleton instance
    static VFM* sInstance;
    static Mutex mLock;

    int32_t      mNumPotentialVpuLayers;
    LayerStats   mLayerStats[MAX_VPU_SESSIONS];

    VIEW_TYPE    mViewType;
    STATE        mState;
    /* Local flag to keep track for Prepare Draw API sequence */
    int32_t      mDebugPrepareDrawSeq;
    int32_t      mDisableVfm;
};

}; //namespace VFM

#endif /* end of include guard: VFM_H */
