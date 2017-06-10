/* Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#define ATRACE_TAG ATRACE_TAG_GRAPHICS
#include <utils/Trace.h>
#include <vfm_surface.h>
#include "vfm_utils.h"

using namespace vpu;

VFMSurface::VFMSurface(const android::sp<Surface>& surface)
    : mLogLevel(0)
{
    mLogLevel = getProperty("persist.vfmsurface.logs");
    mSessionId = surface.get()->getSessionId();
    mBinder =
       interface_cast<qService::IQService>(defaultServiceManager()->getService(
                                           String16("display.qservice")));
}

VFMSurface::VFMSurface(const int32_t id)
    : mLogLevel(0)
{
    ATRACE_CALL();
    mLogLevel = getProperty("persist.vfmsurface.logs");
    mSessionId = id;
    mBinder =
       interface_cast<qService::IQService>(defaultServiceManager()->getService(
                                           String16("display.qservice")));

}

VFMSurface::~VFMSurface(){
    status_t err = NO_ERROR;

    ATRACE_CALL();
    // If buffers are not freed, free them here
    if(NULL != nativeWindow.get()){
        err = deAllocateBuffers();
    }

    return;
}

status_t VFMSurface::allocateBuffers(const int32_t width, const int32_t height,
                const int32_t pixFmt, const int32_t numBufs, int32_t usage)
{
    ATRACE_CALL();
    /* This implementation uses Surface composer client to allocate the
        buffers. However, gralloc can be used to directly allocate the buffers
        as demonstrated in the gralloc test app @
        vendor/qcom/proprietary/display-tests/GrallocTest */
    createLocalSurface();

    allocateBuffers_l(width, height, pixFmt, numBufs, usage);

    registerBufs();

    return NO_ERROR;
}
/*
 * Returns NO_ERROR when success.
*/
status_t VFMSurface::dequeueBuf(private_handle_t** pHnd)
{
    status_t    err = NO_ERROR;
    vfmCmdInArg_t inArg;
    vfmCmdOutArg_t outArg;
    Parcel          data, reply;

    inArg.sessionId     = mSessionId;
    inArg.flags         = VFM_FLAG_SESSION_ID;

    ATRACE_CALL();
    int32_t command = VFM_CMD_DQ_BUF;
    vfmParcelInArgs(command, inArg, data);
    err = mBinder->dispatch(command, &data, &reply);
    if(NO_ERROR != err){
        *pHnd = NULL;
        return err;
    }
    vfmUnparcelOutArgs( command, outArg,    reply);

    ALOGD_IF(all(), "%s: DeQueued bufId: %d \n", __func__, outArg.oaDQBuf.id);
    if(outArg.oaDQBuf.id < 0){
        *pHnd = NULL;
    }
    else{
        *pHnd = getPvtHnd(buf[outArg.oaDQBuf.id]);
        ALOGD_IF(all(), "%s: DeQueued bufId: %d hnd: %p\n",
            __func__, outArg.oaDQBuf.id, *pHnd);
    }
    return NO_ERROR;
}

status_t VFMSurface::dequeueBufAndWait(private_handle_t** pHnd)
{
    status_t    err = NO_ERROR;
    vfmCmdInArg_t inArg;
    vfmCmdOutArg_t outArg;
    Parcel          data, reply;

    inArg.sessionId     = mSessionId;
    inArg.flags         = VFM_FLAG_SESSION_ID;
    outArg.oaDQBuf.id = -1;

    ATRACE_CALL();
    while(outArg.oaDQBuf.id < 0)
    {
        struct timespec ts;

        int32_t command = VFM_CMD_DQ_BUF;
        vfmParcelInArgs(    command, inArg,     data);
        err = mBinder->dispatch(  command, &data,     &reply);
        if(NO_ERROR != err){
            *pHnd = NULL;
            return err;
        }
        vfmUnparcelOutArgs( command, outArg,    reply);

        if(outArg.oaDQBuf.id < 0){
            int32_t b4, after;
            b4 = gettimeofdayMs();
            ts.tv_sec = 0;
            ts.tv_nsec = 10 * 1000000;
            usleep(10*1000);
            after = gettimeofdayMs();
            ALOGD_IF(all(), "%s: DeQueued bufId: %d Trying after: %d ms \n",
                __func__, outArg.oaDQBuf.id, after - b4);
        }
    }
    {
        *pHnd = getPvtHnd(buf[outArg.oaDQBuf.id]);
        ALOGD_IF(all(), "%s: DeQueued bufId: %d hnd: %p\n",
            __func__, outArg.oaDQBuf.id, *pHnd);
    }
    return err;
}

status_t VFMSurface::queueBuf(private_handle_t* hnd)
{
    status_t    err = NO_ERROR;
    vfmCmdInArg_t inArg;
    vfmCmdOutArg_t outArg;
    Parcel data, reply;

    ATRACE_CALL();
    ALOGD_IF(all(), "%s: hnd: %p", __func__, hnd);
    if(!hnd){
      ALOGE("%s: NULL handle passed", __func__);
      return BAD_VALUE;
    }

    inArg.sessionId     = mSessionId;
    inArg.flags         = VFM_FLAG_SESSION_ID;
    inArg.iaQBuf.id     = getBufIdx(hnd);

    if(inArg.iaQBuf.id < 0){
        ALOGE("%s: Did not find hnd: %p in the list", __func__, hnd);
        return BAD_VALUE;
    }

    ALOGD_IF(all(), "%s: Queuing bufId: %d hnd: %p hnd->base: %p \n",
            __func__, inArg.iaQBuf.id, hnd, hnd->base);

    int32_t command = VFM_CMD_Q_BUF;
    vfmParcelInArgs(    command, inArg,     data);
    err = mBinder->dispatch(command, &data, &reply);
    if(NO_ERROR != err){
        return err;
    }
    err = vfmUnparcelOutArgs( command, outArg,    reply);

    return err;
}

status_t VFMSurface::deAllocateBuffers()
{
    status_t err = NO_ERROR;
    int32_t ret = 0;

    ATRACE_CALL();
    for(int32_t i = 0; i < mNumBufs; i++ ){
        ret = nativeWindow->cancelBuffer(nativeWindow.get(), buf[i], -1);
        if(ret){
            ALOGE("%s: cancelBuffer failed for buf: %d", __func__, i);
        }
    }

    err = native_window_api_disconnect(nativeWindow.get(),
                NATIVE_WINDOW_API_MEDIA);
    if(err != NO_ERROR){
        ALOGE("%s: native_window_api_disconnect failed", __func__);
    }

    nativeWindow = NULL;

    return err;
}
/************************
Private methods
************************/
status_t VFMSurface::createLocalSurface()
{
    client = new SurfaceComposerClient();
    client->initCheck();
    surfaceControl = client->createSurface(
                                String8("test-surface"),
                                720, //random number
                                480, //random number
                                HAL_PIXEL_FORMAT_YV12, //random value
                                0 );
    surfaceControl->isValid();
    SurfaceComposerClient::openGlobalTransaction();
    surfaceControl->setPosition(0, 0);
    surfaceControl->setLayer(100);
    /*this is dummy surface not to be displayed */
    //surfaceControl->show();
    SurfaceComposerClient::closeGlobalTransaction();
    sSurface = surfaceControl->getSurface();
    nativeWindow = sSurface;
    return NO_ERROR;
}

status_t VFMSurface::allocateBuffers_l(const int32_t w, const int32_t h,
                const int32_t pixFmt, const int32_t numBufs, int32_t usage)
{
    android::status_t err;

    err = native_window_set_scaling_mode(
            nativeWindow.get(),
            NATIVE_WINDOW_SCALING_MODE_SCALE_TO_WINDOW);
    if(err != 0)
        ALOGE("native_window_set_scaling_mode failed");

    err = native_window_api_connect(
            nativeWindow.get(),
            NATIVE_WINDOW_API_MEDIA);
    if(err != 0)
        ALOGE("native_window_api_connect failed");

    err = native_window_set_buffers_geometry(
        nativeWindow.get(),
        w, h, pixFmt );
    if(err != 0)
        ALOGE("native_window_set_buffers_geometry failed");

    err = native_window_set_usage(
        nativeWindow.get(),
        usage);

    /* no need to allocate min_undeququed as we are never queuing buffers to
        this surface/naitvewindow */
    err = native_window_set_buffer_count(
            nativeWindow.get(),
            numBufs);
    if(err !=0 )
        ALOGE("native_window_set_buffer_count failed");

    for(int32_t i = 0; i < numBufs; i++)
    {
        err = native_window_dequeue_buffer_and_wait(
                nativeWindow.get(),
                &buf[i]);
        if(err !=0 )
            ALOGE("native_window_dequeue_buffer_and_wait failed for %d buf", i);
    }

    /* store number of bufs in local variable */
    mNumBufs = numBufs;
    return NO_ERROR;
}

status_t VFMSurface::registerBufs()
{
    for(int32_t i = 0; i < mNumBufs; i++){
        registerBuf(getPvtHnd(buf[i]), i);
    }
    return NO_ERROR;
}

status_t VFMSurface::registerBuf(private_handle_t* hnd, int32_t bufId)
{
    vfmCmdInArg_t inArg;
    vfmCmdOutArg_t outArg;
    Parcel data, reply;

    ALOGD_IF(dbg(), "%s: Register bufId: %d hnd: %p hnd->base: %p size: %d\n",
                __func__, bufId, hnd, hnd->base, hnd->size);
    inArg.sessionId     = mSessionId;
    inArg.flags         = VFM_FLAG_SESSION_ID;
    inArg.iaBufReg.hnd  = hnd;
    inArg.iaBufReg.id   = bufId;

    int32_t command = VFM_CMD_REGISTER_BUFFER;
    vfmParcelInArgs(    command, inArg,     data);
    mBinder->dispatch(  command, &data,     &reply);
    vfmUnparcelOutArgs( command, outArg,    reply);

    return NO_ERROR;
}

private_handle_t* VFMSurface::getPvtHnd(ANativeWindowBuffer* buf)
{
    if(buf) {
        return static_cast<private_handle_t *>
                    (const_cast<native_handle*>(buf->handle));
    }
    return NULL;
}

//returns -1 if not found
int32_t VFMSurface::getBufIdx(private_handle_t *hnd)
{
    if(hnd){
        for(int32_t i = 0; i < mNumBufs; i++)
        {
            if(static_cast<private_handle_t*>
                (const_cast<native_handle*>(buf[i]->handle)) == hnd)
                return i;
        }
    }
    return -1;
}

