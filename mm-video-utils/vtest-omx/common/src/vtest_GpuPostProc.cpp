/* ========================================================================= *
  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
* ========================================================================= */

#include "vtest_GpuPostProc.h"
#include <window.h>
#include <dlfcn.h>
#include "vtest_NativeWindow.h"
#include <gralloc_priv.h>
#include "qdMetaData.h"

#undef LOG_TAG
#define LOG_TAG "VTEST_GPU_POSTPROC"

#define SymGpuPostProcLib "libgpupostprocessing.so"
#define SymGpuPostProcInit "gpuPostProcessing_Init"
#define SymGpuPostProcTerminate "gpuPostProcessing_Terminate"
#define SymGpuPostProcPerform "gpuPostProcessing_Perform"

namespace vtest {

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
GpuPostProc::GpuPostProc()
    : m_fGpuPostProcInit(NULL),
        m_fGpuPostProcTerminate(NULL),
        m_fGpuPostProcPerform(NULL) {

    snprintf(m_pName, PROPERTY_FILENAME_MAX, "GpuPostProc");
    VTEST_MSG_LOW("%s created", Name());
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
GpuPostProc::~GpuPostProc() {

    Terminate();
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE GpuPostProc::Init(PostProcSession *pSession) {

    OMX_ERRORTYPE result = OMX_ErrorNone;

    if (pSession == NULL) {
        FAILED1(OMX_ErrorBadParameter, "PostProcSession structure is null");
    }

    m_nFrameWidth = pSession->nFrameWidth;
    m_nFrameHeight = pSession->nFrameHeight;
    m_bSecureSession = pSession->bSecureSession;
    m_nInputColorFormat = pSession->nInputColorFormat;
    m_nOutputColorFormat = pSession->nOutputColorFormat;

    result = LoadGpuPostProcLib();
    FAILED1(result, "GpuPostProcLib loading failed");

    return (OMX_ERRORTYPE)m_fGpuPostProcInit(
            (gpuPostProcessingHandle**)&m_pSessionHandle, m_bSecureSession);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void GpuPostProc::Terminate() {

    if (m_fGpuPostProcTerminate) {
        m_fGpuPostProcTerminate((gpuPostProcessingHandle*)m_pSessionHandle);
        m_pSessionHandle = NULL;
    }
    UnloadGpuPostProcLib();
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE GpuPostProc::Perform(BufferInfo *pBufferIn, BufferInfo *pBufferOut) {

    gppResult result = GPP_SUCCESS;
    ANativeWindowBuffer *anw1 = NULL;
    ANativeWindowBuffer *anw2 = NULL;

    if (m_fGpuPostProcPerform == NULL) {
        FAILED1(OMX_ErrorUndefined, "Invalid method handle to Perform");
    }

    if (!pBufferIn || !pBufferOut) {
        FAILED1(OMX_ErrorBadParameter, "Bad parameters: %p, %p", pBufferIn, pBufferOut);
    }

    private_handle_t *pHandleIn = (private_handle_t *)pBufferIn->pHandle;
    private_handle_t *pHandleOut = (private_handle_t *)pBufferOut->pHandle;
    if (!pHandleIn || !pHandleOut) {
        FAILED1(OMX_ErrorBadParameter, "Bad parameters: %p, %p", pHandleIn, pHandleOut);
    }

    anw1 = new ANativeWindowBuffer_t;
    anw1->width = m_nFrameWidth;
    anw1->height = m_nFrameHeight;
    anw1->handle = (buffer_handle_t)pHandleIn;
    anw1->format = GetGrallocFormat(m_nInputColorFormat);

    anw2 = new ANativeWindowBuffer_t;
    anw2->width = m_nFrameWidth;
    anw2->height = m_nFrameHeight;
    anw2->handle = (buffer_handle_t)pHandleOut;
    anw2->format = GetGrallocFormat(m_nOutputColorFormat);

    int32_t param = 1;
    if (m_bSecureSession) {
        setMetaData(pHandleIn, MAP_SECURE_BUFFER, (void*)&param);
        setMetaData(pHandleOut, MAP_SECURE_BUFFER, (void*)&param);
    }

    result = m_fGpuPostProcPerform((gpuPostProcessingHandle*)m_pSessionHandle, anw1, anw2);

    if (m_bSecureSession) {
        param = 0;
        setMetaData(pHandleIn, MAP_SECURE_BUFFER, (void*)&param);
        setMetaData(pHandleOut, MAP_SECURE_BUFFER, (void*)&param);
    }

    delete anw1;
    delete anw2;

    if (result != GPP_SUCCESS) {
        VTEST_MSG_ERROR("Error in GpuPostProc Perform");
        return OMX_ErrorBadParameter;
    }
    return OMX_ErrorNone;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE GpuPostProc::LoadGpuPostProcLib() {

    OMX_ERRORTYPE result = OMX_ErrorNone;

    VTEST_MSG_HIGH("Loading GpuPostProc lib");

    m_pLibHandle = dlopen(SymGpuPostProcLib, RTLD_NOW);
    if (m_pLibHandle == NULL) {
        VTEST_MSG_ERROR("Failed to open %s, error : %s", SymGpuPostProcLib, dlerror());
        return OMX_ErrorUndefined;
    }

    m_fGpuPostProcInit = (GpuPostProc_Init)dlsym(m_pLibHandle, SymGpuPostProcInit);
    if (m_fGpuPostProcInit == NULL) {
        VTEST_MSG_ERROR("Failed to find symbol for GpuPostProcInit: %s", dlerror());
        result = OMX_ErrorUndefined;
    }

    if (result == OMX_ErrorNone) {
        m_fGpuPostProcTerminate = (GpuPostProc_Terminate)dlsym(m_pLibHandle, SymGpuPostProcTerminate);
        if (m_fGpuPostProcTerminate == NULL) {
            VTEST_MSG_ERROR("Failed to find symbol for GpuPostProcTerminate: %s", dlerror());
            result = OMX_ErrorUndefined;
        }
    }

    if (result == OMX_ErrorNone) {
        m_fGpuPostProcPerform = (GpuPostProc_Perform)dlsym(m_pLibHandle, SymGpuPostProcPerform);
        if (m_fGpuPostProcPerform == NULL) {
            VTEST_MSG_ERROR("Failed to find symbol for GpuPostProcPerform: %s", dlerror());
            result = OMX_ErrorUndefined;
        }
    }

    if (result != OMX_ErrorNone) {
        UnloadGpuPostProcLib();
    }
    return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void GpuPostProc::UnloadGpuPostProcLib() {

    if (m_pLibHandle) {
        dlclose(m_pLibHandle);
        m_pLibHandle = NULL;
    }

    m_fGpuPostProcInit = NULL;
    m_fGpuPostProcTerminate = NULL;
    m_fGpuPostProcPerform = NULL;
}

} // namespace vtest
