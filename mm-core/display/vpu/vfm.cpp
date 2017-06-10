/* Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

/*****************************************************************************
    vfm.cpp

    Defines the Video Flow Manager class methods. VFM manages the video flow
    objects. VFM identifies appropriate layers from the combined layer list
    and assigns to corresponding video flow objects. VFM has a visibility of
    all the video flows. Any operations / intelligence required across the
    video flows should be implemented here
*****************************************************************************/
#define ATRACE_TAG (ATRACE_TAG_GRAPHICS | ATRACE_TAG_HAL)
#include <utils/Trace.h>
#include <utils/Log.h>
#include <assert.h>
#include "vfm_utils.h"
#include "vfm.h"
#include "vfm_videoflow.h"
#include "vfm_vpuinterface.h"
#include "qdMetaData.h"
#include "vfm_metadata.h"

namespace vpu {
VFM* VFM::sInstance = 0;
Mutex VFM::mLock;

//////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////

VFM::VFM()
    : mActiveVidFlows(0),
      mEventThread(NULL),
      mTotalNumVpuSessions(2),
      mViewType(NO_VIEW),
      mState(DRV_UNINIT),
      mDebugPrepareDrawSeq(0)
{
    status_t err = NO_ERROR;
    mDebugLogs  = getProperty("persist.vfm.logs");
    mDisableVfm = getProperty("persist.vfm.disable");

    ALOGD_IF(logAll(), "%s: E", __func__);

    mNotifyCb.eventHdlr = NULL;

    return;
}

VFM::~VFM()
{
    /* TBD: close all VideoFlows and close all FDs */
}

VFM* VFM::getInstance()
{
    Mutex::Autolock lock(mLock);
    if(NULL == sInstance){
        ALOGD("%s: new VFM()", __func__);
        sInstance = new VFM();
    }
    return sInstance;
}

//! Creates a VPU driver client for control access
//  Queries the maximum number of sessions that vpu driver can manage
//   and creates as many VideoFlow objects
//  Also creates Event thread to be used across all video flows
status_t VFM::init()
{
    status_t err = NO_ERROR;
    VPUCapabilities  vpuCap;

    ALOGD_IF(isDebug(), "%s: E", __func__);

    /* start event thread */
    mEventThread = new EventThread(mDebugLogs);

    err = assertChecks();
    ERR_CHK((NO_ERROR != err), err);

    if(mState < DRV_INIT){
        mFd = VpuIntf::getFd();
        if(mFd < 0){
            ALOGE("getFd failed");
            return UNKNOWN_ERROR;
        }
        mState = DRV_INIT;
    }
    if (mState < VIDFLOW_INIT){
        err = VpuIntf::querySessions( mFd, vpuCap );
        ERR_CHK_LOG((err  != NO_ERROR), err, "querySessions failed");
        mTotalNumVpuSessions = vpuCap.numSessions;

        for(int i = 0; i < mTotalNumVpuSessions; i++){
            // Video Flow ids starts from '0'
            mVidFlow[i] = new VidFlow(i, mEventThread, mNotifyCb,
                            mDebugLogs);
        }
        mState = VIDFLOW_INIT;
    }
    if(mFd > 0){
        err = VpuIntf::closeFd(mFd);
        if(err != NO_ERROR){
            ALOGE("%s: closeFd failed", __func__);
            return err;
        }
        ERR_CHK_LOG((err  != NO_ERROR), err, "");
    }

    ALOGD_IF(logAll(), "%s: X", __func__);
    return err;
}

status_t VFM::init(notifyCb_t* notifyCb)
{
    ALOGD_IF(logAll(), "%s: E", __func__);
    if(NULL != notifyCb){
        mNotifyCb = *notifyCb;
    }
    return init();
}

status_t VFM::setupVpuSession(DISPLAY_ID dispId, LayerList* layerList)
{
    status_t err = NO_ERROR;

    ALOGD_IF(logAll(), "%s: E", __func__);

    if(mDisableVfm) return NO_ERROR;

    if(mDebugPrepareDrawSeq == 0)
        mDebugPrepareDrawSeq++;
    else
        ALOGD_IF(isDebug(),"setupVpuSession out of order mDebug: %d",
            mDebugPrepareDrawSeq);

    switch(dispId){
        case DISP_PRIMARY:
          err = setupVpuSessionPrimary(layerList);
          ERR_CHK((NO_ERROR != err), err);

          break;
        default:
          ALOGE("%s: Display %d not handled", __FUNCTION__, dispId);
    }
    return err;
}

status_t VFM::prepare(DISPLAY_ID dispId, LayerList* layerList)
{
    status_t err = NO_ERROR;

    ALOGD_IF(logAll(), "%s: E", __func__);

    if(mDisableVfm) return NO_ERROR;

    if(mDebugPrepareDrawSeq == 1)
        mDebugPrepareDrawSeq++;
    else
        ALOGD_IF(isDebug(), "prepare out of order, mDebug: %d",
            mDebugPrepareDrawSeq);

    switch(dispId){
        case DISP_PRIMARY:
          err = preparePrimary(layerList);
          ERR_CHK((NO_ERROR != err), err);

          break;
        default:
          ALOGE("%s: Display %d not handled", __FUNCTION__, dispId);
    }
    return err;
}

status_t VFM::draw(DISPLAY_ID dispId, LayerList* layerList)
{
    status_t err = NO_ERROR;

    ALOGD_IF(logAll(), "%s: E", __func__);

    if(mDisableVfm) {
        ALOGD_IF(logAll(), "%s: X", __func__);
        return NO_ERROR;
    }

    if(mDebugPrepareDrawSeq == 2)
        mDebugPrepareDrawSeq = 0;
    else
        ALOGD_IF(isDebug(), "draw out of order, mDebug: %d",
            mDebugPrepareDrawSeq);

    switch(dispId){
        case DISP_PRIMARY:
          err = drawPrimary(layerList);
          ERR_CHK((NO_ERROR != err), err);

          break;
        default:
          ALOGE("%s: Display %d not handled", __FUNCTION__, dispId);
    }
    ALOGD_IF(logAll(), "%s: X", __func__);
    return err;
}

/*
    This function translates parcels to in and out args and calls local
    processCommand
*/
status_t VFM::processCommand(const uint32_t command,
                        const Parcel* inParcel, Parcel* outParcel)
{
    status_t    err = NO_ERROR;

    vfmCmdInArg_t   inArg;
    vfmCmdOutArg_t  outArg;

    ATRACE_CALL();
    ALOGD_IF(logAll(), "%s: cmd: %d ", __func__, command);
    err = unparcelInArgs(command, inArg, inParcel);
    ERR_CHK((NO_ERROR != err), err);

    err = processCommand(command, inArg, outArg);
    ERR_CHK((NO_ERROR != err), err);

    err = parcelOutArgs(command, outArg, outParcel);
    ERR_CHK((NO_ERROR != err), err);
    return err;
}

/* This function routes the command to appropriate video flow. Checks
    whether the command is for VFM_FLAG_ALL_SESSIONS or inArg.sessionId
    based or inArg.vidFlowId based
*/
status_t VFM::processCommand(const uint32_t command,
                        const vfmCmdInArg_t& inArg, vfmCmdOutArg_t& outArg)
{
    status_t err = NO_ERROR;
    VidFlow* vidFlow = NULL;

    ALOGD_IF(logAll(), "%s: cmd: %d ", __func__, command);
    if(inArg.flags & VFM_FLAG_ALL_SESSIONS) {
        for(int32_t i = 0; i < mTotalNumVpuSessions; i++){
            vidFlow = mVidFlow[i];
            err |= vidFlow->processCommand(command, inArg, outArg);
            if (NO_ERROR != err){
                ALOGE("%s: processCommand for flow id: %d failed",
                        __func__, vidFlow->vidFlowId());
            }
        }
    }
    else if(inArg.flags & VFM_FLAG_SESSION_ID) {
        //TODO: getVidFlow returns NULL if not in 'RUNNING' state to avoid any
        //error conditions. Need to remove this in order accept the settings
        //even when the VidFlow is not running
        if(NULL != (vidFlow = getVidFlow(inArg.sessionId))){
            return vidFlow->processCommand(command, inArg, outArg);
        }
        else {
            ALOGE("%s: processCommand called for non existing sessId: %d",
                    __func__, inArg.sessionId);
            return NO_INIT;
        }
    }
    else {
        //TODO: getVidFlow returns NULL if not in 'RUNNING' state to avoid any
        //error conditions. Need to remove this in order accept the settings
        //even when the VidFlow is not running
        if(NULL != (vidFlow = getVidFlowByFlowId(inArg.vidFlowId))){
            return vidFlow->processCommand(command, inArg, outArg);
        }
        else {
            ALOGE("%s: processCommand called for non existing flowId: %d",
                    __func__, inArg.vidFlowId);
            return NO_INIT;
        }
    }
    return err;
}

status_t VFM::setDisplayAttr(DISPLAY_ID dispId, DispAttr_t& dispAttr)
{
    status_t err = NO_ERROR;
    vfmCmdInArg_t   inArg;
    vfmCmdOutArg_t  outArg;

    inArg.flags = VFM_FLAG_ALL_SESSIONS;
    inArg.iaDispAttr.dpy    = dispId;
    inArg.iaDispAttr.width  = dispAttr.width;
    inArg.iaDispAttr.height = dispAttr.height;
    inArg.iaDispAttr.fp100s = dispAttr.fp100s;

    /* store display attributes locally */
    setDisplayAttr(inArg.iaDispAttr);

    /* send the attributes to vidflows */
    return processCommand(VFM_CMD_SET_DISPLAY_ATTRIBUTES, inArg, outArg);
}
//////////////////////////////////////////////////////////////////
// PRIVATE METHODS
//////////////////////////////////////////////////////////////////
status_t VFM::assertChecks()
{
    /* Assert added to ensure the vfm metadata is defined within the bounds */
    if(sizeof(VfmMetaData_t) > sizeof (VfmData_t)){
        ALOGE("%s: ASSERT ON VfmMetaData_t FAILED. CLOSING VFM", __func__);
        return BAD_VALUE;
    }
    /* Assert to ensure the vfmCmdInArg_t is defined within the bounds */
    if(sizeof(vfmCmdInArg_t) > (MAX_INT_PER_CMD_ARG * 4)){
        ALOGE("%s: ASSERT ON vfmCmdInArg_t FAILED. CLOSING VFM", __func__);
        return BAD_VALUE;
    }
    /* Assert to ensure the vfmCmdInArg_t is defined within the bounds */
    if(sizeof(vfmCmdOutArg_t) > (MAX_INT_PER_CMD_ARG * 4)){
        ALOGE("%s: ASSERT ON vfmCmdOutArg_t FAILED. CLOSING VFM", __func__);
        return BAD_VALUE;
    }
    return NO_ERROR;
}
/*
    This function identifies which layers can be processed by VPU
*/
status_t VFM::setupVpuSessionPrimary(LayerList* layerList)
{
    status_t err = NO_ERROR;

    ATRACE_CALL();
    ALOGD_IF(logAll(), "%s: E", __func__);
    //FIXME: Resetting the state everytime this call is recvd due to some bug
    mState = VIDFLOW_INIT;

    switch(mState){
        case VIDFLOW_INIT:
        case DRAW_VPU_LAYERS:  //! Previous draw successful
        {
            err = setLayerStats(layerList);
            ERR_CHK_LOG((err  != NO_ERROR), err, "setLayerStats");

            //TODO: sortLayersByPriority();
            err = createVidFlowsIfReqd(layerList);
            ERR_CHK_LOG((err  != NO_ERROR), err, "createVidFlowsIfReqd");

            err = updateVidFlows(layerList);
            ERR_CHK_LOG((err  != NO_ERROR), err, "updateVidFlows failed");

            err = setVpuLayerFlag(layerList);
            ERR_CHK_LOG((err  != NO_ERROR), err, "setVpuLayerFlag");

            //FIXME: mark all the 'stopped' vidFlows to be closed
            //All the video flows that were stopped in the previous iteration
            //due to errors have to closed in the draw call so that MDSS fetch
            //is stopped before VPU session close
            //err = moveStopToClose(layerList);
            mState = MARK_VPU_LAYERS;
            break;
        }
        case DRV_UNINIT:
        case DRV_INIT:
        case MARK_VPU_LAYERS:
        case PREPARE_VPU_LAYERS:
        default:
            ALOGD("%s Non-critical warning invalid state %d", __func__, mState);
            return INVALID_OPERATION;
    }
    dumpLayerStats();

    return err;
}
/*
    This func sets the destination pipe information to all VPU layers
*/
status_t VFM::preparePrimary(LayerList* layerList)
{
    status_t err = NO_ERROR;

    ATRACE_CALL();
    ALOGD_IF(logAll(), "%s: E", __func__);
    switch(mState) {
        case MARK_VPU_LAYERS:
        {
            {
                // Destination pipe information is not sent earlier than this
                // FIXME: If pipes cannot be allocated, close the vpuSession
                err = setDestPipes(layerList);
                ERR_CHK_LOG((err  != NO_ERROR), err, "setDestPipes failed");
            }
            mState = PREPARE_VPU_LAYERS;
            break;
        }
        case DRV_UNINIT:
        case DRV_INIT:
        case VIDFLOW_INIT:
        case PREPARE_VPU_LAYERS:
        case DRAW_VPU_LAYERS:
        default:
            ALOGD("%s Non-critical warning invalid state %d", __func__, mState);
            return INVALID_OPERATION;
    }
    return err;
}

/*
    Queues the buffers to VPU (only for non-Vcap cases)
*/
status_t VFM::drawPrimary(LayerList* layerList)
{
    status_t err = NO_ERROR;

    ATRACE_CALL();
    ALOGD_IF(logAll(), "%s: E mState: %d", __func__, mState);
    switch(mState) {
        case PREPARE_VPU_LAYERS:
        {
            if(mActiveVidFlows){
                err = qBufs(layerList);
                ERR_CHK_LOG((err  != NO_ERROR), err, "qBufs failed");

                /* Any missing layers has to be closed here to
                 * ensure MDSS pipes are reset before closing VPU session */
                err = closeMissingVidFlows(layerList);
                ERR_CHK_LOG((err  != NO_ERROR), err,
                    "closeMissingVidFlows failed");
            }
            mState = DRAW_VPU_LAYERS;
            break;
        }
        case DRV_UNINIT:
        case DRV_INIT:
        case VIDFLOW_INIT:
        case MARK_VPU_LAYERS:
        case DRAW_VPU_LAYERS:
        default:
            ALOGD("%s Non-critical warning invalid state %d", __func__, mState);
            return INVALID_OPERATION;

    }
    return err;
}

/* Identifies only those layers which needs to be processed by VFM */
status_t VFM::setLayerStats(LayerList* layerList)
{
    status_t err = NO_ERROR;
    clearLayerStats();

    ALOGD_IF(logAll(), "%s: E", __func__);
    for (uint32_t i = 0; i < layerList->numLayers ; i++) {
        Layer const* layer = &layerList->layers[i];
        private_handle_t* hnd = (private_handle_t *)layer->handle;

        /* Ignore the layers marked as skip layers */
        if(layer->inFlags & SKIP_LAYER) continue;
        /*--------------------------------------------------------*/
        /* VPU can process only Video layers. Typically HLOS      */
        /* submits YUV formats and YUV/RGB from VCAP              */
        /*--------------------------------------------------------*/
        if(isYuvBuffer(hnd) || isVcapSource(hnd))
        {
            LayerStats* lyrStats = &mLayerStats[mNumPotentialVpuLayers];
            //Reset the flags
            lyrStats->mMarkedToClose = 0;

            // store the layer indexes
            lyrStats->mIdx = i;

            //TBD: assignPriority() based on a policy

            /* Priority = Video Z-order for now
             The priority is used only while creating a new video session.
             A new video can never pre-empt an existing video session in vpu */
            lyrStats->mPriority = mNumPotentialVpuLayers;
            lyrStats->mMediaSessId = getMediaSessId(hnd);
            lyrStats->mVidFlowExists = doesVideoFlowExist(getMediaSessId(hnd));

            mNumPotentialVpuLayers++;
            ALOGD_IF(isDebug(),
                "Potential lyr sessId: %x", getMediaSessId(hnd));
        }
        /* Break if the VPU session limit is reached */
        if (mNumPotentialVpuLayers >= MAX_VPU_SESSIONS){
            break;
        }
    }
    return err;
}

//Close the video flow if active but not in the layer list
status_t VFM::markMissingVidFlows(LayerList *layerList)
{
    status_t err = NO_ERROR;

    ALOGD_IF(logAll(), "%s: E", __func__);
    for(int i = 0; i < mTotalNumVpuSessions; i++){
        //Close the video flow if it is active but not in the layer list
        if(mVidFlow[i]->isActive() &&
          (!isMediaSessIdPresent(mVidFlow[i]->mediaSessId()))){
            ALOGD_IF(isDebug(), "mMediaSessId: %d missing, marking to close",
                mVidFlow[i]->mediaSessId());
            for(int32_t j = 0; j < mNumPotentialVpuLayers; j++){
                if(mLayerStats[j].mMediaSessId ==
                    mVidFlow[i]->mediaSessId()){
                    mLayerStats[j].mMarkedToClose = 1;
                    return NO_ERROR;
                }
            }
            ALOGE("Warning: mMediaSessId: %d is missing but cannot mark for "
                    "closing", mVidFlow[i]->mediaSessId());
        }
    }
    return err;
}

//Close the video flow if active but not in the layer list
status_t VFM::closeMissingVidFlows(LayerList *layerList)
{
    status_t err = NO_ERROR;

    ALOGD_IF(logAll(), "%s: E", __func__);
    for(int i = 0; i < mTotalNumVpuSessions; i++){
        //Close the video flow if it is active but not in the layer list
        //Alternatively, can check for .mMarkedToClose flag to close
        if(mVidFlow[i]->isActive() &&
          (!isMediaSessIdPresent(mVidFlow[i]->mediaSessId()))){
            ALOGD_IF(isDebug(), "mMediaSessId: %d missing, will be closed",
                mVidFlow[i]->mediaSessId());

            err = closeVidFlow(mVidFlow[i]);
            ERR_CHK((NO_ERROR != err), err);
        }
    }
    return err;
}

//! Loops through the layer list to check if a particulary
//  mediaSessId is present
bool VFM::isMediaSessIdPresent(int32_t mediaSessId)
{
    ALOGD_IF(logAll(), "%s: E", __func__);
    for(int j = 0; j < mNumPotentialVpuLayers; j++){
        if(mediaSessId == mLayerStats[j].mMediaSessId){
            return true;
        }
    }
    return false;
}

//Closes the video flow
status_t VFM::closeVidFlow(VidFlow *vidFlow)
{
    status_t err = NO_ERROR;
    if(!vidFlow)
        return NO_ERROR;

    err = vidFlow->teardown();
    ERR_CHK((NO_ERROR != err), err);

    mActiveVidFlows--;
    updateViewType();

    return err;
}

//!Loops through all the video flows and to check if a VideoFlow
// corresponding to the mediaSessId already exists
int32_t VFM::doesVideoFlowExist(int32_t mediaSessId)
{
    status_t err = NO_ERROR;
    ALOGD_IF(logAll(), "%s: E", __func__);

    for(int i = 0; i < mTotalNumVpuSessions; i++){
        if(!mVidFlow[i]->isActive())
            continue;
        if(mediaSessId == mVidFlow[i]->mediaSessId())
            return true;
    }
    ALOGD_IF(isDebug(), "VideoFlow for %x does not exist", mediaSessId);
    return false;
}

//!Loops through all the video flows and returns the one
// that matches the mediaSessId
VidFlow* VFM::getVidFlow(int32_t mediaSessId)
{
    status_t err = NO_ERROR;
    ALOGD_IF(logAll(), "%s: E", __func__);

    for(int i = 0; i < mTotalNumVpuSessions; i++){
        if(!mVidFlow[i]->isActive())
            continue;
        if(mediaSessId == mVidFlow[i]->mediaSessId())
            return mVidFlow[i];
    }
    ALOGD_IF(isDebug(), "%s: VideoFlow for %x does not exist",
        __func__, mediaSessId);
    return NULL;
}

//!Loops through all the video flows and returns the one
// that matches the vidFlowId
VidFlow* VFM::getVidFlowByFlowId(int32_t vidFlowId)
{
    status_t err = NO_ERROR;
    ALOGD_IF(logAll(), "%s: E", __func__);

    for(int i = 0; i < mTotalNumVpuSessions; i++){
        if(!mVidFlow[i]->isActive())
            continue;
        if(vidFlowId == mVidFlow[i]->vidFlowId())
            return mVidFlow[i];
    }
    ALOGD_IF(isDebug(), "%s: VideoFlow does not exist", __func__);
    return NULL;
}

//! Loops through the potential list layer and creates videoFlows
//  if non-existent for that layer
status_t VFM::createVidFlowsIfReqd(LayerList *layerList)
{
    status_t err = NO_ERROR;

    ALOGD_IF(logAll(), "%s: E", __func__);
    for(int j = 0; j < mNumPotentialVpuLayers; j++){
        LayerStats* lyrStats = &mLayerStats[j];
        //Nothing to do if the Video Flow already exists
        if(lyrStats->mVidFlowExists)
            continue;

        VidFlow* vidFlow = getNextAvailableVidFlow();
        //Number of potential layers is greater than what h/w can handle
        if(NULL == vidFlow)
            break;

        // If a free VideoFlow is avaialble, then associate this layer with
        // that VidFlow
        Layer* layer = &layerList->layers[lyrStats->mIdx];
        ALOGD_IF(isDebug(), "%s: creating video flow for layer: %d",
                                __func__, lyrStats->mIdx);
        err = vidFlow->init(layer);
        if(NO_ERROR != err){
        /* Note: This is not an error condition. VPU does not support the
         * configuration of this layer. Donot return error */

            err = vidFlow->teardown();
            ALOGE_IF((NO_ERROR != err), "%s: vidFlow->teardown failed: %d",
                __func__, err);

            /* Donot proceed further for this layer */
            continue;
        }
        else{
            lyrStats->mNewlyCreatedVidFlow = 1;
            mActiveVidFlows++;
            updateViewType();
        }
    }
    return err;
}

/*!
    Loops through the potential layer and updates the existing videoFlows
    Very similar to setLayer calls. setLayer is for first frame of the
    session and updateLayer is for subsequent frames. If vidFlow has to
    do different book keeping for first frame and subsequent frames,
    this separation will be useful
*/
status_t VFM::updateVidFlows(LayerList *layerList)
{
    status_t err = NO_ERROR;

    ALOGD_IF(logAll(), "%s: E", __func__);
    for(int j = 0; j < mNumPotentialVpuLayers; j++){
        LayerStats* lyrStats = &mLayerStats[j];
        Layer* layer = &layerList->layers[lyrStats->mIdx];
        /* update the layer only if it is not marked for closure */
        if(lyrStats->mVidFlowExists &&
            !(lyrStats->mMarkedToClose)){
            VidFlow* vidFlow = getVidFlow(mLayerStats[j].mMediaSessId);

            if(vidFlow){
                err = vidFlow->updateLayer(layer);
            /* Note: This is not an error condition. VPU does not support the
             * updated configuration of this layer. Mark the video flow to be
             * closed */
                if(NO_ERROR != err){
                    /* Mark that an existing video flow has to be closed */
                    lyrStats->mMarkedToClose = 1;
                }
            }//if(vidFlow)
        }
    }
    return NO_ERROR;
}

/*!
    Sets VPU_LAYER flag for all the layers that vpu can handle
    Also sets the destination pipe information for already existing VidFlow
*/
status_t VFM::setVpuLayerFlag(LayerList *layerList)
{
    status_t err = NO_ERROR;
    ALOGD_IF(logAll(), "%s: E", __func__);

    for(int j = 0; j < mNumPotentialVpuLayers; j++){
        Layer* layer = &layerList->layers[mLayerStats[j].mIdx];

        /* Nothing to be done for the layer marked for closure */
        if(mLayerStats[j].mMarkedToClose)
            continue;

        if((mLayerStats[j].mVidFlowExists) ||
           (mLayerStats[j].mNewlyCreatedVidFlow)){
            layer->outFlags |= VPU_LAYER;
        }

        //Reserve Pipes flag to be set if vid flow already exists
        if(mLayerStats[j].mVidFlowExists){
            VidFlow* vidFlow = getVidFlow(mLayerStats[j].mMediaSessId);
            if(vidFlow){
                vidFlow->getDestPipes(layer->sDestPipes);
                layer->outFlags |= RESERVE_PREV_PIPES;
            }
            else{
                ALOGE("%s: Error mVidFlowExists flag set but vidFlow is NULL",
                        __func__);
            }
        }
        ALOGD_IF(logAll(), "layerId: %d, outFlag: %d",
                    mLayerStats[j].mIdx, layer->outFlags);
    }
    return err;
}

/*!
    Loops through the video flows and set the dest pipe info for
    all the existing as well as newly created pipes. Sets to
    the existing pipes as well so that any change in the pipe
    config can be updated
*/
status_t VFM::setDestPipes(LayerList* layerList)
{
    status_t err = NO_ERROR;
    ALOGD_IF(logAll(), "%s: E", __func__);

    for(int j = 0; j < mNumPotentialVpuLayers; j++){
        Layer* layer = &layerList->layers[mLayerStats[j].mIdx];

        /* Nothing to be done for the layer marked for closure */
        if(mLayerStats[j].mMarkedToClose)
            continue;

        if((mLayerStats[j].mVidFlowExists) ||
           (mLayerStats[j].mNewlyCreatedVidFlow)){
            VidFlow* vidFlow = getVidFlow(mLayerStats[j].mMediaSessId);

            if(vidFlow){
                err = vidFlow->setDestPipes(layer->sDestPipes);
                if(NO_ERROR != err){
                    //FIXME: STOP MDSS and close VPU session if
                    //VPU session is already STREAM_ON
                    //FIXME: If not stream ON, then can be closed immediately
                }
            }//if<vidFlow)
        }
    }
    ALOGD_IF(logAll(), "%s: X", __func__);
    return err;
}

//! Queues the buffers to all the active videoFlows
status_t VFM::qBufs(LayerList* layerList)
{
    status_t err = NO_ERROR;
    ALOGD_IF(logAll(), "%s: E", __func__);

    for(int j = 0; j < mNumPotentialVpuLayers; j++){
        Layer* layer = &layerList->layers[mLayerStats[j].mIdx];

        //In case of errors in qBuf, set releaseFenceFd to -1
        layer->releaseFenceFd = -1;
        if((mLayerStats[j].mVidFlowExists) ||
           (mLayerStats[j].mNewlyCreatedVidFlow)){
            VidFlow* vidFlow = getVidFlow(mLayerStats[j].mMediaSessId);

            if(vidFlow){
                err = vidFlow->qBuf(layer->handle, layer->releaseFenceFd);
                if(NO_ERROR != err){
                    //FIXME: STOP MDSS and close VPU session if
                    //VPU session is already STREAM_ON
                    //vidFlow->stop();
                }
            }//if(vidFlow)
            ALOGD_IF(logAll(), "releaseFenceFd: %d", layer->releaseFenceFd);
        }
    }

    return err;
}

//! Updates the view type based on number of active video flows
inline void VFM::updateViewType()
{
    mViewType = (mActiveVidFlows > 1) ? MULTI_VIEW: SINGLE_VIEW;
    return;
}

//Return -1 if no VideoFlowId is avaialble
VidFlow* VFM::getNextAvailableVidFlow()
{
    ALOGD_IF(logAll(), "%s: E", __func__);

    for(int i = 0; i < mTotalNumVpuSessions; i++ ){
        if(mVidFlow[i]->isAvailable()){
            return mVidFlow[i];
        }
    }
    ALOGD_IF(isDebug(), "%s: No free VidFlow", __func__);
    return NULL;
}

//Clears the layer list stats structure
void VFM::clearLayerStats()
{
    mNumPotentialVpuLayers = 0;
    for(int32_t j = 0; j < MAX_VPU_SESSIONS; j++){
       memset(&mLayerStats[j], 0, sizeof(LayerStats));
    }
}


//Logs the layer list status structure contents
void VFM::dumpLayerStats()
{
    for(int32_t j = 0; j < mNumPotentialVpuLayers; j++){
        LayerStats *ls = &mLayerStats[j];
        ALOGD_IF(isDebug(), "%s: lyrId:[%d] ms:%d p:%d hasVf:%d needVf:%d",
            __func__, ls->mIdx, ls->mMediaSessId, ls->mPriority,
            ls->mVidFlowExists, ls->mNewlyCreatedVidFlow);
    }
    return;
}
void VFM::setDisplayAttr(const vfmInArgSetDisplayAttr_t& iaDisplayAttr)
{
    dispAttr[iaDisplayAttr.dpy].width = iaDisplayAttr.width;
    dispAttr[iaDisplayAttr.dpy].height = iaDisplayAttr.height;
    dispAttr[iaDisplayAttr.dpy].fp100s = iaDisplayAttr.fp100s;
    return;
}

extern "C" VFM* getVfmObject()
{
    return VFM::getInstance();
}
extern "C" void deleteVfmObject(VFM* object)
{
}

};
