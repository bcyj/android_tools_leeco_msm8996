/* ========================================================================= *
  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
* ========================================================================= */

#include "vtest_MmCCPostProc.h"
#include <dlfcn.h>
#include "graphics.h"

#undef LOG_TAG
#define LOG_TAG "VTEST_MMCC_POSTPROC"

#define SymMmCCLib "libmm-color-convertor.so"
#define SymMmCCConvert "convert"

namespace vtest {

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
MmCCPostProc::MmCCPostProc()
    : m_fMmCCConvert(NULL) {

    snprintf(m_pName, PROPERTY_FILENAME_MAX, "MmCCPostProc");
    VTEST_MSG_LOW("%s created", Name());
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
MmCCPostProc::~MmCCPostProc() {

    Terminate();
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE MmCCPostProc::Init(PostProcSession *pSession) {

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

    result = LoadMmCCLib();
    FAILED1(result, "MmCCPostProcLib loading failed");
    return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void MmCCPostProc::Terminate() {

    UnloadMmCCLib();
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE MmCCPostProc::Perform(BufferInfo *pBufferIn, BufferInfo *pBufferOut) {

    OMX_ERRORTYPE result = OMX_ErrorNone;

    if (!pBufferIn || !pBufferOut) {
        FAILED1(OMX_ErrorBadParameter, "Bad parameters: %p, %p", pBufferIn, pBufferOut);
    }

    OMX_BUFFERHEADERTYPE *pHeaderIn = pBufferIn->pHeaderIn;
    OMX_BUFFERHEADERTYPE *pHeaderOut = pBufferOut->pHeaderOut;

    if (!pHeaderIn || !pHeaderOut) {
        FAILED1(OMX_ErrorBadParameter, "Bad parameters: %p, %p", pHeaderIn, pHeaderOut);
    }

    ColorConvertParams inputCP;
    inputCP.width = GetFrameStride(m_nInputColorFormat, m_nFrameWidth);
    inputCP.height = GetFrameSlice(m_nInputColorFormat, m_nFrameHeight);
    inputCP.cropWidth = m_nFrameWidth;
    inputCP.cropHeight = m_nFrameHeight;
    inputCP.cropLeft = 0;
    inputCP.cropRight = m_nFrameWidth;
    inputCP.cropTop = 0;
    inputCP.cropBottom = m_nFrameHeight;
    inputCP.flags = COLOR_CONVERT_ALIGN_NONE;
    inputCP.fd = -1;
    inputCP.colorFormat = GetMmFormat(m_nInputColorFormat);
    inputCP.data = pHeaderIn->pBuffer;

    ColorConvertParams outputCP;
    outputCP.width = GetFrameStride(m_nOutputColorFormat, m_nFrameWidth);
    outputCP.height = GetFrameSlice(m_nOutputColorFormat, m_nFrameHeight);
    outputCP.cropWidth = m_nFrameWidth;
    outputCP.cropHeight = m_nFrameHeight;
    outputCP.cropLeft = 0;
    outputCP.cropRight = m_nFrameWidth;
    outputCP.cropTop = 0;
    outputCP.cropBottom = m_nFrameHeight;
    outputCP.flags = COLOR_CONVERT_ALIGN_NONE;
    outputCP.fd = -1;
    outputCP.colorFormat = GetMmFormat(m_nOutputColorFormat);;
    outputCP.data = pHeaderOut->pBuffer;;

    result = (OMX_ERRORTYPE)m_fMmCCConvert(inputCP, outputCP, NULL);
    return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE MmCCPostProc::LoadMmCCLib() {

    OMX_ERRORTYPE result = OMX_ErrorNone;

    VTEST_MSG_HIGH("Loading MmCC lib");

    m_pLibHandle = dlopen(SymMmCCLib, RTLD_NOW);
    if (m_pLibHandle == NULL) {
        VTEST_MSG_ERROR("Failed to open %s, error : %s", SymMmCCLib, dlerror());
        return OMX_ErrorUndefined;
    }

    m_fMmCCConvert = (MmCC_Convert)dlsym(m_pLibHandle, SymMmCCConvert);
    if (m_fMmCCConvert == NULL) {
        VTEST_MSG_ERROR("Failed to find symbol for MmCCOpen: %s", dlerror());
        result = OMX_ErrorUndefined;
    }

    if (result != OMX_ErrorNone) {
        UnloadMmCCLib();
    }
    return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void MmCCPostProc::UnloadMmCCLib() {

    if (m_pLibHandle) {
        dlclose(m_pLibHandle);
        m_pLibHandle = NULL;
    }

    m_fMmCCConvert = NULL;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
ColorConvertFormat MmCCPostProc::GetMmFormat(OMX_U32 nFormat) {

    switch (nFormat) {
        case QOMX_COLOR_FORMATYUV420PackedSemiPlanar32m:
            return YCbCr420SP128M;
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
