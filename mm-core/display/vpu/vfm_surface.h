/* Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef VFM_SURFACE_H
#define VFM_SURFACE_H

#include <sys/types.h>

#include <stdlib.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <utils/Errors.h>
#include <cutils/log.h>
#include <cutils/properties.h>

#include <ui/Rect.h>
#include <ui/FramebufferNativeWindow.h>
#include <ui/PixelFormat.h>

#include <hardware/gralloc.h>
#include <hardware/hwcomposer.h>
#include <gralloc_priv.h>
#include <alloc_controller.h>
#include <memalloc.h>
#include <gr.h>
#include <linux/types.h>

#include <gui/SurfaceComposerClient.h>
#include <gui/Surface.h>

#include <gralloc_priv.h>
#include <qdMetaData.h>
#include <binder/IServiceManager.h>
#include "vfm_metadata.h"
#include "vfm_cmds.h"
#include "IQService.h"

#define MAX_NUM_VFM_BUFS (32)

using namespace android;

class VFMSurface {
public:
    VFMSurface(const int32_t id);
    VFMSurface(const android::sp<Surface>&);
    virtual ~VFMSurface();

    virtual android::status_t allocateBuffers(
                const int32_t width,
                const int32_t height,
                const int32_t pixFmt,
                const int32_t numBufs,
                const int32_t usage
                );

    /* No lock and unlock mechanism. So client has to ensure mutual
        exclusivity. This is a non-blocking call and will return
        NULL as buffer pointer when buffer is not available */
    virtual android::status_t dequeueBuf(private_handle_t**);

    /* Blocking dequeue call. Will always return a valid buffer */
    virtual android::status_t dequeueBufAndWait(private_handle_t**);

    virtual android::status_t queueBuf(private_handle_t*);

    // virtual status_t cancelBuf(private_handle_t*);

    /* its client's ownership to ensure that the buffer is not in use while
     * calling this API.
        TempFix: Close the dummy buffer before calling this API so that MDSS and
        VPU session are closed before freeing the buffers */
     virtual status_t deAllocateBuffers();
private:
    VFMSurface();

    status_t    createLocalSurface();
    status_t    allocateBuffers_l(const int32_t w, const int32_t h,
                const int32_t pixFmt, const int32_t numBufs, int32_t usage);
    status_t    registerBufs();
    status_t    registerBuf(private_handle_t* hnd, int32_t bufId);
    private_handle_t* getPvtHnd(ANativeWindowBuffer* buf);
    int32_t     getBufIdx(private_handle_t *hnd);
    int32_t     all() { return mLogLevel >= 4; }
    int32_t     dbg() { return mLogLevel >= 2; }
    inline int32_t gettimeofdayMs()
    {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        return (tv.tv_sec * 1000 + tv.tv_usec / 1000);
    }

    int32_t                 mSessionId;
    sp<qService::IQService> mBinder;
    int32_t                 mLogLevel;
    /* for allocating gralloc buffers through Surface composer client */
    int32_t                             mNumBufs;
    ANativeWindowBuffer*                buf[MAX_NUM_VFM_BUFS];
    android::sp<SurfaceComposerClient>  client;
    android::sp<SurfaceControl>         surfaceControl;
    android::sp<Surface>                sSurface;
    android::sp<ANativeWindow>          nativeWindow;
};
#endif /* end of include gaurd VFM_SURFACE_H */
