/* ========================================================================= *
  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
* ========================================================================= */

#include "vtest_C2dCCPostProc.h"
#include <dlfcn.h>
#include "gralloc_priv.h"
#include "graphics.h"

#undef LOG_TAG
#define LOG_TAG "VTEST_C2DCC_POSTPROC"

#define SymC2dCCLib "libc2dcolorconvert.so"
#define SymC2dCCOpen "createC2DColorConverter"
#define SymC2dCCClose "destroyC2DColorConverter"

using namespace android;

namespace vtest {

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
C2dCCPostProc::C2dCCPostProc()
    : m_fC2dCCOpen(NULL),
        m_fC2dCCClose(NULL) {

    snprintf(m_pName, PROPERTY_FILENAME_MAX, "C2dCCPostProc");
    VTEST_MSG_LOW("%s created", Name());
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
C2dCCPostProc::~C2dCCPostProc() {

    Terminate();
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE C2dCCPostProc::Init(PostProcSession *pSession) {

    OMX_ERRORTYPE result = OMX_ErrorNone;

    if (pSession == NULL) {
        FAILED1(OMX_ErrorBadParameter, "PostProcSession structure is null");
    }

    m_nFrameWidth = pSession->nFrameWidth;
    m_nFrameHeight = pSession->nFrameHeight;
    m_bSecureSession = pSession->bSecureSession;
    m_nInputColorFormat = pSession->nInputColorFormat;
    m_nOutputColorFormat = pSession->nOutputColorFormat;

    if (m_bSecureSession) {
        FAILED1(OMX_ErrorUnsupportedSetting, "Secure session not supported");
    }

    result = LoadC2dCCLib();
    FAILED1(result, "C2dCCPostProcLib loading failed");

    m_pSessionHandle = (void*)m_fC2dCCOpen(m_nFrameWidth, m_nFrameHeight,
            m_nFrameWidth, m_nFrameHeight,
            GetC2DFormat(m_nInputColorFormat), GetC2DFormat(m_nOutputColorFormat), 0, 0);
    if (m_pSessionHandle == NULL) {
        result = OMX_ErrorBadParameter;
        VTEST_MSG_ERROR("C2DCC Open failed");
    }
    return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void C2dCCPostProc::Terminate() {

    if (m_fC2dCCClose) {
        m_fC2dCCClose((C2DColorConverterBase*)m_pSessionHandle);
        m_pSessionHandle = NULL;
    }
    UnloadC2dCCLib();
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE C2dCCPostProc::Perform(BufferInfo *pBufferIn, BufferInfo *pBufferOut) {

    OMX_ERRORTYPE result = OMX_ErrorNone;

    if (m_pSessionHandle == NULL) {
        FAILED1(OMX_ErrorUndefined, "Invalid method handle to C2dCC Perform");
    }

    if (!pBufferIn || !pBufferOut) {
        FAILED1(OMX_ErrorBadParameter, "Bad parameters: %p, %p", pBufferIn, pBufferOut);
    }

    private_handle_t *pHandleIn = (private_handle_t *)pBufferIn->pHandle;
    private_handle_t *pHandleOut = (private_handle_t *)pBufferOut->pHandle;
    OMX_BUFFERHEADERTYPE *pHeaderIn = pBufferIn->pHeaderIn;
    OMX_BUFFERHEADERTYPE *pHeaderOut = pBufferOut->pHeaderOut;

    if (!pHandleIn || !pHandleOut || !pHeaderIn || !pHeaderOut) {
        FAILED1(OMX_ErrorBadParameter, "Bad parameters: %p, %p, %p, %p",
                pHandleIn, pHandleOut, pHeaderIn, pHeaderOut);
    }

    return (OMX_ERRORTYPE)((C2DColorConverterBase*)m_pSessionHandle)->convertC2D(
            pHandleIn->fd, pHeaderIn->pBuffer, pHeaderIn->pBuffer,
            pHandleOut->fd, pHeaderOut->pBuffer, pHeaderOut->pBuffer);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void C2dCCPostProc::GetBufferRequirements(OMX_U32 ePortIndex, OMX_U32 *nBufferSize) {

    if (!nBufferSize) {
        VTEST_MSG_ERROR("nBufferSize is NULL");
        return;
    }

    *nBufferSize = 0x1000;
    if (m_pSessionHandle) {
        C2DBuffReq pBufReq;
        if (!((C2DColorConverterBase*)m_pSessionHandle)->getBuffReq(ePortIndex, &pBufReq)) {
            *nBufferSize = pBufReq.size;
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE C2dCCPostProc::LoadC2dCCLib() {

    OMX_ERRORTYPE result = OMX_ErrorNone;

    VTEST_MSG_HIGH("Loading C2dCC lib");

    m_pLibHandle = dlopen(SymC2dCCLib, RTLD_NOW);
    if (m_pLibHandle == NULL) {
        VTEST_MSG_ERROR("Failed to open %s, error : %s", SymC2dCCLib, dlerror());
        return OMX_ErrorUndefined;
    }

    m_fC2dCCOpen = (C2dCC_Open)dlsym(m_pLibHandle, SymC2dCCOpen);
    if (m_fC2dCCOpen == NULL) {
        VTEST_MSG_ERROR("Failed to find symbol for C2dCCOpen: %s", dlerror());
        result = OMX_ErrorUndefined;
    }

    if (result == OMX_ErrorNone) {
        m_fC2dCCClose = (C2dCC_Close)dlsym(m_pLibHandle, SymC2dCCClose);
        if (m_fC2dCCClose == NULL) {
            VTEST_MSG_ERROR("Failed to find symbol for C2dCCClose: %s", dlerror());
            result = OMX_ErrorUndefined;
        }
    }

    if (result != OMX_ErrorNone) {
        UnloadC2dCCLib();
    }
    return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void C2dCCPostProc::UnloadC2dCCLib() {

    if (m_pLibHandle) {
        dlclose(m_pLibHandle);
        m_pLibHandle = NULL;
    }

    m_fC2dCCOpen = NULL;
    m_fC2dCCClose = NULL;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
ColorConvertFormat C2dCCPostProc::GetC2DFormat(OMX_U32 nFormat)
{
    switch (nFormat) {
        case QOMX_COLOR_FORMATYUV420PackedSemiPlanar32m:
            return NV12_128m;
        case OMX_COLOR_FormatYUV420Planar:
            return YCbCr420P;
        case HAL_PIXEL_FORMAT_YV12:
            return YCrCb420P;
        default:
            VTEST_MSG_ERROR("Unsupported color format : %d", nFormat);
            return (ColorConvertFormat)-1;
    }
}

} // namespace vtest
