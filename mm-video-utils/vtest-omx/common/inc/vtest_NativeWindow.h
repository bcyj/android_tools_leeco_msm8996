/* ========================================================================= *
  Copyright (c) 2012-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
* ========================================================================= */
#ifndef _VTEST_NATIVE_WINDOW_H
#define _VTEST_NATIVE_WINDOW_H

#include <utils/RefBase.h>
#include <SurfaceComposerClient.h>
#include <Surface.h>
#include <ISurfaceComposer.h>
#include <DisplayInfo.h>
#include "OMX_Core.h"
#include "OMX_QCOMExtns.h"
#include <gralloc_priv.h>
#include "vtest_DecoderFileSink.h"

using namespace android;

/*
 * This class wraps the android NativeWindow API and provides simplified
 * interface to allocate and display buffers.
 */
namespace vtest {

class NativeWindow : virtual public ISource {

public:
    NativeWindow(OMX_BOOL bRotate,
            OMX_U32 nColorFormat, OMX_BOOL bShowVideo = OMX_TRUE);
    ~NativeWindow();

    virtual PortBufferCapability GetBufferRequirements(OMX_U32 ePortIndex);
    virtual OMX_ERRORTYPE Start();
    virtual OMX_ERRORTYPE Configure(CodecConfigType *pConfig,
            BufferManager *pBufManager, ISource *pSource, ISource *pSink);
    virtual OMX_ERRORTYPE SetBuffer(
            BufferInfo *pBuffer, ISource *pSource);
    virtual OMX_ERRORTYPE AllocateBuffers(BufferInfo **pBuffers,
            OMX_U32 nWidth, OMX_U32 nHeight, OMX_U32 nBufferCount,
            OMX_U32 nBufferSize, OMX_U32 ePortIndex, OMX_U32 nBufferUsage = 0);
    virtual OMX_ERRORTYPE FreeAllocatedBuffers(BufferInfo **pBuffers,
            OMX_U32 nBufferCount, OMX_U32 ePortIndex);
    virtual OMX_ERRORTYPE PortReconfig(OMX_U32 ePortIndex,
            OMX_U32 nWidth, OMX_U32 nHeight, const OMX_CONFIG_RECTTYPE& sRect);

private:
    NativeWindow();
    static const OMX_U32 DEFAULT_DISPLAY_WIDTH = 800;
    static const OMX_U32 DEFAULT_DISPLAY_HEIGHT = 480;

    enum Rotation {
        ROTATION_0 = 0,
        ROTATION_90 = 90
    };

    virtual OMX_ERRORTYPE ThreadRun(OMX_PTR pThreadData);
    virtual OMX_ERRORTYPE FreeBuffer(
            BufferInfo *pBuffer, OMX_U32 ePortIndex);
    OMX_ERRORTYPE QueryAllocatedNativeBuffer(OMX_U32 nIndex, ANativeWindowBuffer *& nativeBuf);
    OMX_ERRORTYPE RegisterBuffer(OMX_U8 *pBufferHandle, OMX_BUFFERHEADERTYPE *pBuffer);
    OMX_ERRORTYPE CancelBuffer(OMX_BUFFERHEADERTYPE *bufHdr);
    OMX_ERRORTYPE DequeueBuffer(OMX_BUFFERHEADERTYPE *& bufHdr);
    OMX_ERRORTYPE EnqueueBuffer(OMX_BUFFERHEADERTYPE *bufHdr);
    OMX_ERRORTYPE FindBuffer(OMX_BUFFERHEADERTYPE *bufHdr, ANativeWindowBuffer **buf);

    void Cleanup();

    //get scaled size and position based on video size and orientation
    void FixScaling(OMX_U32 transform,
            OMX_U32& w, OMX_U32& h, OMX_U32& x, OMX_U32& y);

    class BufMap {
    public:
        BufMap(const OMX_U32 limit);
        ~BufMap();

        OMX_ERRORTYPE add(ANativeWindowBuffer *);
        //associate nativeBuffers with bufHdrs
        OMX_ERRORTYPE map(OMX_U8 *, OMX_BUFFERHEADERTYPE *);
        OMX_ERRORTYPE removeLast(ANativeWindowBuffer *&);
        //find nativebuffer corresponding to bufHdr
        ANativeWindowBuffer* find(OMX_BUFFERHEADERTYPE *);
        //find bufHdr corresponding to nativebuffer
        OMX_BUFFERHEADERTYPE* find(ANativeWindowBuffer *);

        OMX_U32 GetSize() const { return mCount; }
        ANativeWindowBuffer * operator[](OMX_U32 i) { return mNativeList[i]; }
        OMX_U32 mCurrentCount;

    private:
        ANativeWindowBuffer  **mNativeList;
        OMX_BUFFERHEADERTYPE **mBufHdrList;
        OMX_U32 mCount;
        OMX_U32 mLimit;
    };

private:

    sp<SurfaceComposerClient>   m_pSurfaceComposer;
    sp<Surface>                 m_pSurface;
    sp<SurfaceControl>          m_pControl;
    sp<ANativeWindow>           m_pNativeWindow;

    //background
    sp<SurfaceControl>          m_pBackgroundControl;

    OMX_U32 m_nFrames;
    OMX_U32 m_nFrameWidth;
    OMX_U32 m_nFrameHeight;
    OMX_U32 m_nDisplayWidth;
    OMX_U32 m_nDisplayHeight;
    OMX_U32 m_nMinBufferCount;
    OMX_U32 m_nColorFormat;
    Rotation m_eRotation;
    BufMap *mBufMap;
    OMX_U32 m_nBuffersWithNativeWindow;
    SignalQueue *m_pFreeBufferQueue;
    OMX_BOOL m_bFreeBuffers;
    OMX_BOOL m_bShowVideo;
};

static inline OMX_U32 GetGrallocFormat(OMX_U32 nFormat) {

    switch (nFormat) {
        case OMX_COLOR_FormatYUV420Planar:
              /* Cr Cb will be swapped, mdp limitation
               * can be fixed in color convertors if req.
               * This will require Input width and height to
               * be already aligned to hw requirements.
               */
              return HAL_PIXEL_FORMAT_YV12;
        case OMX_COLOR_Format32bitARGB8888:
            return HAL_PIXEL_FORMAT_RGBA_8888;
        default:
            return nFormat;
    }
}

}

#endif //#ifndef _VTEST_NATIVE_WINDOW_H

