/* ========================================================================= *
  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
* ========================================================================= */

#include "vtest_IPostProc.h"
#include "gralloc_priv.h"
#include <msm_media_info.h>
#include "graphics.h"
#include "qdMetaData.h"

#undef LOG_TAG
#define LOG_TAG "VTEST_IPOSTPROC"

namespace vtest {

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
IPostProc::IPostProc()
    : m_nFrameWidth(0),
        m_nFrameHeight(0),
        m_bSecureSession(OMX_FALSE),
        m_pSessionHandle(NULL),
        m_pLibHandle(NULL),
        m_nInputColorFormat(0),
        m_nOutputColorFormat(0) {

    memset(m_pName, 0, sizeof(m_pName));
    snprintf(m_pName, PROPERTY_FILENAME_MAX, "NAME NOT SET");
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
IPostProc::~IPostProc() {}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE IPostProc::Init(PostProcSession *pSession) {

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
    return OMX_ErrorNone;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void IPostProc::Terminate() {}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE IPostProc::Perform(BufferInfo *pBufferIn, BufferInfo *pBufferOut) {

    OMX_ERRORTYPE result = OMX_ErrorNone;
    OMX_BUFFERHEADERTYPE *pHeaderIn = NULL, *pHeaderOut = NULL;

    if (!pBufferIn || !pBufferOut) {
        FAILED1(OMX_ErrorBadParameter, "Bad parameters: %p, %p", pBufferIn, pBufferOut);
    }

    pHeaderIn = pBufferIn->pHeaderIn;
    pHeaderOut = pBufferOut->pHeaderOut;

    if (!pHeaderIn || !pHeaderOut) {
        FAILED1(OMX_ErrorBadParameter, "Bad parameters: %p, %p", pHeaderIn, pHeaderOut);
    }

    VTEST_MSG_HIGH("Post Process buffers : In %p, Out %p, nFilledlen %u",
            pHeaderIn, pHeaderOut, pHeaderOut->nFilledLen);

#if 0
    if (m_bSecureSession) {
        /* Cannot copy from secure to secure, need to copy into a temp
         * non-secure buffer first. */
        OMX_U8 *pTempBuffer = new OMX_U8[pHeaderOut->nFilledLen];
        if (!pTempBuffer) {
            VTEST_MSG_ERROR("Out of memory: unable to allocate temp buffer");
            return OMX_ErrorInsufficientResources;
        }

        private_handle_t* pHandleIn = (private_handle_t *)pBufferIn->pHandle;
        private_handle_t* pHandleOut = (private_handle_t *)pBufferOut->pHandle;
        if (!pHandleIn || !pHandleOut) {
            delete [] pTempBuffer;
            FAILED1(OMX_ErrorBadParameter, "Bad parameters: %p, %p", pHandleIn, pHandleOut);
        }

        result = m_pCrypto->Copy(SAMPLECLIENT_COPY_SECURE_TO_NONSECURE,
                pTempBuffer, (unsigned long)(pHandleIn->fd), pHeaderOut->nFilledLen);
        if (result != OMX_ErrorNone) {
            VTEST_MSG_ERROR("OEMCrypto_Copy from secure to non-secure failed, result is %d", result);
            delete [] pTempBuffer;
            return result;
        }

        result = m_pCrypto->Copy(SAMPLECLIENT_COPY_NONSECURE_TO_SECURE,
                pTempBuffer, (unsigned long)(pHandleOut->fd), pHeaderOut->nFilledLen);
        delete [] pTempBuffer;
        FAILED1(result, "OEMCrypto_Copy from non-secure to secure failed, result is %d", result);
    } else
#else
    {
        memcpy(pHeaderOut->pBuffer, pHeaderIn->pBuffer, pHeaderIn->nFilledLen);
    }
#endif
    return OMX_ErrorNone;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void IPostProc::GetBufferRequirements(OMX_U32 ePortIndex, OMX_U32 *nBufferSize) {

    if (!nBufferSize) {
        VTEST_MSG_ERROR("nBufferSize is NULL");
        return;
    }

    *nBufferSize = GetFrameSize(
            (ePortIndex == PORT_INDEX_IN) ? m_nInputColorFormat : m_nOutputColorFormat,
            m_nFrameWidth, m_nFrameHeight);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_STRING IPostProc::Name() {

    return m_pName;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void IPostProc::UpdateOutputBufferGeometry(BufferInfo *pBuffer) {

    if (!pBuffer) {
        VTEST_MSG_ERROR("pBuffer is NULL");
        return;
    }

    BufferDim_t dim;

    dim.sliceWidth = GetFrameStride(m_nOutputColorFormat, m_nFrameWidth);
    dim.sliceHeight = GetFrameSlice(m_nOutputColorFormat, m_nFrameHeight);
    setMetaData((private_handle_t *)pBuffer->pHandle, UPDATE_BUFFER_GEOMETRY, (void*)&dim);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_U32 IPostProc::GetFrameSize(OMX_U32 nFormat, OMX_U32 nWidth, OMX_U32 nHeight) {

    VTEST_MSG_LOW("Format is %x", nFormat);
    switch (nFormat) {
        case QOMX_COLOR_FORMATYUV420PackedSemiPlanar32m:
            return VENUS_BUFFER_SIZE(COLOR_FMT_NV12, nWidth, nHeight);
        case OMX_COLOR_FormatYUV420Planar:
            return VTEST_ALIGN((nWidth * nHeight * 3 / 2), 4096);
        case HAL_PIXEL_FORMAT_YV12:
            {
                OMX_U32 nStride = VTEST_ALIGN(nWidth, 16);
                OMX_U32 nSlice = nHeight;
                return VTEST_ALIGN(nStride * nSlice +
                        (VTEST_ALIGN(nStride / 2, 16) * (nSlice / 2)) * 2, 4096);
            }
        case OMX_COLOR_Format32bitARGB8888:
            return VTEST_ALIGN(nWidth * nHeight * 4, 4096);
        default:
            return 0x1000;
    }
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_U32 IPostProc::GetFrameStride(OMX_U32 nFormat, OMX_U32 nWidth) {

    switch (nFormat) {
        case QOMX_COLOR_FORMATYUV420PackedSemiPlanar32m:
            return VTEST_ALIGN(nWidth, 128);
        case HAL_PIXEL_FORMAT_YV12:
            return VTEST_ALIGN(nWidth, 16);
        default:
            return nWidth;
    }
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_U32 IPostProc::GetFrameSlice(OMX_U32 nFormat, OMX_U32 nHeight) {

    switch (nFormat) {
        case QOMX_COLOR_FORMATYUV420PackedSemiPlanar32m:
            return VTEST_ALIGN(nHeight, 32);
        default:
            return nHeight;
    }
}


} // namespace vtest
