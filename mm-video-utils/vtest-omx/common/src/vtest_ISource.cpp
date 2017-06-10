/*-------------------------------------------------------------------
Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential
--------------------------------------------------------------------*/

#include "vtest_Debug.h"
#include "vtest_BufferManager.h"
#include "vtest_ISource.h"

#undef LOG_TAG
#define LOG_TAG "VTEST_ISOURCE"

namespace vtest {

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
ISource::ISource()
    : m_nInBufferCount(0),
      m_nInBytes(0),
      m_pThread(new Thread()),
      m_bThreadStop(OMX_FALSE),
      m_pSink(NULL),
      m_pSource(NULL),
      m_pBufferManager(NULL),
      m_pLock(new Mutex()),
      m_pBufferQueue(new SignalQueue(100, sizeof(OMX_BUFFERHEADERTYPE*))),
      m_bSourceStop(OMX_FALSE),
      m_bSourceResult(OMX_ErrorNone) {

    memset(m_pName, 0, sizeof(m_pName));
    snprintf(m_pName, PROPERTY_FILENAME_MAX, "NAME NOT SET");
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
ISource::~ISource() {

    if (m_pThread != NULL) {
        delete m_pThread;
        m_pThread = NULL;
    }

    if (m_pLock != NULL) {
        delete m_pLock;
        m_pLock = NULL;
    }

    if (m_pBufferQueue != NULL) {
        delete m_pBufferQueue;
        m_pBufferQueue = NULL;
    }
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE ISource::Start() {

    OMX_ERRORTYPE result = OMX_ErrorNone;
    {
        Mutex::Autolock autoLock(m_pLock);

        // duplicate starts ok, as any source can start
        if (m_pThread->Started()) {
            return result;
        }

        VTEST_MSG_MEDIUM("%s, thread start...", Name());
        result = m_pThread->Start(
            this,   // threaded object
            this,   // thread data
            0);     // thread priority
        FAILED1(result, "Failed to start thread");
    }

    if (m_pSource != NULL) {
        result = m_pSource->Start();
        FAILED1(result, "Error: cannot start source %s!", m_pSource->Name());
    }

    if (m_pSink != NULL) {
        result = m_pSink->Start();
        FAILED1(result, "Error: cannot start sink %s!", m_pSink->Name());
    }

    // Queue buffers to start chain
    // by convention, have each block enqueue to its output port to avoid duplicate enqueuing
    if (m_pSink != NULL) {
        OMX_U32 nBufferCount;
        BufferInfo *pBuffers = NULL;
        result = m_pBufferManager->GetBuffers(
            this, PORT_INDEX_OUT, &pBuffers, &nBufferCount);
        FAILED1(result, "%s: Error setting port buffers", Name());

        for (OMX_U32 i = 0; i < nBufferCount; i++) {
            VTEST_MSG_HIGH("%s: queuing buffer %p", Name(), &pBuffers[i]);
            SetBuffer(&pBuffers[i], m_pSink);
            FAILED1(result, "%s: Error setting port buffer %p", Name(), &pBuffers[i]);
        }
    }
    return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE ISource::Finish() {

    OMX_ERRORTYPE result = OMX_ErrorNone;

    if (!m_pThread->Started()) {
        VTEST_MSG_HIGH("%s: thread already done", Name());
        return m_bSourceResult;
    }

    VTEST_MSG_MEDIUM("%s: thread join...", Name());
    m_pThread->Join(&result);

    m_bSourceResult = (OMX_ERRORTYPE)((OMX_S32)result | (OMX_S32)m_bSourceResult);
    if (m_bSourceResult != OMX_ErrorNone) {
        VTEST_MSG_ERROR("Error: %s thread execution error", Name());
    }
    return m_bSourceResult;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE ISource::Stop() {

    OMX_S32 executionResult = 0;

    {
        Mutex::Autolock autoLock(m_pLock);

        if (m_bSourceStop) {
            return m_pThread->GetStatus();
        }

        VTEST_MSG_HIGH("%s: Stop", Name());
        m_bThreadStop = OMX_TRUE;
        m_pBufferQueue->WakeAll();
    }

    VTEST_MSG_MEDIUM("%s: finishing...", Name());

    executionResult = (OMX_S32)Finish();
    VTEST_MSG_MEDIUM("%s: execution result : %d", Name(), executionResult);
    {
        Mutex::Autolock autoLock(m_pLock);
        m_bSourceStop = OMX_TRUE;
    }

    if (m_pSource != NULL) {
        executionResult |= m_pSource->Stop();
    }

    if (m_pSink != NULL) {
        executionResult |= m_pSink->Stop();
    }
    return (OMX_ERRORTYPE)executionResult;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE ISource::Configure(CodecConfigType *pConfig,
        BufferManager *pBufManager, ISource *pSource, ISource *pSink) {

    (void) pConfig;
    OMX_ERRORTYPE result = OMX_ErrorNone;
    Mutex::Autolock autoLock(m_pLock);

    if (pBufManager == NULL ||
        (pSource == NULL && pSink == NULL)) {
        VTEST_MSG_ERROR("Error: BufferManager or source/sink not set");
        return OMX_ErrorBadParameter;
    }

    m_pBufferManager = pBufManager;
    m_pSource = pSource;
    m_pSink = pSink;
    return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE ISource::SetBuffer(BufferInfo *pBuffer, ISource *pSource) {

    Mutex::Autolock autoLock(m_pLock);

    if (m_bThreadStop) {
        return OMX_ErrorNoMore;
    }

    if (pSource == NULL) {
        FAILED1(OMX_ErrorBadParameter, "Source is null");
    }

    if (pBuffer != NULL) {
        //non-shared headers have some fields copied...
        if (pBuffer->pHeaderIn != pBuffer->pHeaderOut) {
            if (pSource == m_pSource) {
                pBuffer->pHeaderIn->nFilledLen = pBuffer->pHeaderOut->nFilledLen;
                pBuffer->pHeaderIn->nOffset = pBuffer->pHeaderOut->nOffset;
                pBuffer->pHeaderIn->nTimeStamp = pBuffer->pHeaderOut->nTimeStamp;
                pBuffer->pHeaderIn->nFlags = pBuffer->pHeaderOut->nFlags;
            } else {
                pBuffer->pHeaderOut->nFilledLen = pBuffer->pHeaderIn->nFilledLen;
                pBuffer->pHeaderOut->nOffset = pBuffer->pHeaderIn->nOffset;
                pBuffer->pHeaderOut->nTimeStamp = pBuffer->pHeaderIn->nTimeStamp;
                pBuffer->pHeaderOut->nFlags = pBuffer->pHeaderIn->nFlags;
            }
        }

        // track input bytes
        if(pSource == m_pSource || m_pSource == NULL) {
            m_nInBufferCount++;
            m_nInBytes += pBuffer->pHeaderIn->nFilledLen;
            VTEST_MSG_LOW("%s received: buf_ct %u byte_ct %u",
                          Name(), (unsigned int)m_nInBufferCount, (unsigned int)m_nInBytes);
        }
    }
    return OMX_ErrorNone;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE ISource::AllocateBuffers(BufferInfo **pBuffers,
        OMX_U32 nWidth, OMX_U32 nHeight, OMX_U32 nBufferCount,
        OMX_U32 nBufferSize, OMX_U32 ePortIndex, OMX_U32 nBufferUsage) {

    (void)pBuffers, (void)nWidth, (void)nHeight, (void)nBufferCount;
    (void)nBufferSize, (void)ePortIndex, (void)nBufferUsage;
    Mutex::Autolock autoLock(m_pLock);
    VTEST_MSG_ERROR("%s: Error, unimplemented allocate", Name());
    return OMX_ErrorUndefined;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE ISource::UseBuffers(BufferInfo **pBuffers,
        OMX_U32 nWidth, OMX_U32 nHeight, OMX_U32 nBufferCount,
        OMX_U32 nBufferSize, OMX_U32 ePortIndex) {

    (void)nWidth, (void)nHeight, (void)nBufferSize;
    Mutex::Autolock autoLock(m_pLock);
    VTEST_MSG_LOW("%s: sharing buffer-headers with %s on %s port",
                  Name(),
                  (ePortIndex == PORT_INDEX_IN ? m_pSource->Name() : m_pSink->Name()),
                  OMX_PORT_NAME(ePortIndex));

    // By default use the allocating nodes buffer-headers
    for (OMX_U32 i = 0; i < nBufferCount; i++) {
        BufferInfo *pBuffer = &((*pBuffers)[i]);
        if (ePortIndex == PORT_INDEX_OUT) {
            pBuffer->pHeaderOut = pBuffer->pHeaderIn;
        } else {
            pBuffer->pHeaderIn = pBuffer->pHeaderOut;
        }
    }
    return OMX_ErrorNone;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE ISource::FreeAllocatedBuffers(
        BufferInfo **pBuffers, OMX_U32 nBufferCount, OMX_U32 ePortIndex) {

    OMX_ERRORTYPE result = OMX_ErrorNone;

    if (pBuffers == NULL) {
        return result;
    }

    VTEST_MSG_MEDIUM("%s, freeing allocated buffers on %s port",
                     Name(), OMX_PORT_NAME(ePortIndex));
    for (OMX_U32 i = 0; i < nBufferCount; i++) {
        BufferInfo *pBuffer = &((*pBuffers)[i]);
        VTEST_MSG_LOW("%s, free-alloc 0x%lx (%p %p)",Name(),
                         pBuffer->pHandle, pBuffer->pHeaderIn, pBuffer->pHeaderOut);
        result = FreeBuffer(pBuffer, ePortIndex);
        pBuffer->pHandle = 0;
    }
    return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE ISource::FreeUsedBuffers(
        BufferInfo **pBuffers, OMX_U32 nBufferCount, OMX_U32 ePortIndex) {

    OMX_ERRORTYPE result = OMX_ErrorNone;

    VTEST_MSG_MEDIUM("%s, freeing used buffer on %s port",
                     Name(), OMX_PORT_NAME(ePortIndex));
    for (OMX_U32 i = 0; i < nBufferCount; i++) {
        BufferInfo *pBuffer = &((*pBuffers)[i]);
        VTEST_MSG_LOW("%s, free-use 0x%lx (%p %p)",Name(),
                      pBuffer->pHandle, pBuffer->pHeaderIn, pBuffer->pHeaderOut);
        result = FreeBuffer(pBuffer, ePortIndex);
    }
    return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE ISource::PortReconfig(OMX_U32 ePortIndex,
        OMX_U32 nWidth, OMX_U32 nHeight, const OMX_CONFIG_RECTTYPE& sRect) {

    (void)nWidth, (void)nHeight, (void)sRect;
    VTEST_MSG_MEDIUM("%s: PortReconfig %s",
            Name(), OMX_PORT_NAME(ePortIndex));
    return OMX_ErrorNone;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_STRING ISource::Name() {
    return m_pName;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_S32 ISource::OutputBufferCount() {
    return (m_pSink != NULL ? m_pSink->m_nInBufferCount : m_nInBufferCount);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_S32 ISource::OutputByteCount() {
    return (m_pSink != NULL ? m_pSink->m_nInBytes : m_nInBytes);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
ISource* ISource::Source() {
    return m_pSource;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
ISource* ISource::Sink() {
    return m_pSink;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void ISource::SetError() {

    /* This API is only to be used when YOU run into an error,
     * then it is your responsibility to notify everyone else.
     * DO NOT use this API when someone else runs into an error
     * and SetBuffer on them fails. It should do the same work
     * as an EOS sent from first node in the chain.
     * When calling this API, make sure the error is propagated
     * to your thread or called from the context of that thread,
     * so that it can be reported to the app.
     * The other sources will NOT report your error for you.
     * */
    VTEST_MSG_MEDIUM("%s:", Name());
    {
        Mutex::Autolock autoLock(m_pLock);
        m_bSourceResult = OMX_ErrorUndefined;
    }

    StopThreadOnError();
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void ISource::StopThreadOnError() {

    VTEST_MSG_MEDIUM("%s:", Name());
    {
        Mutex::Autolock autoLock(m_pLock);
        m_bThreadStop = OMX_TRUE;
        m_pBufferQueue->WakeAll();
    }

    if ((m_pSource != NULL) && (!m_pSource->ThreadStoppedOnError())) {
        m_pSource->StopThreadOnError();
    }

    if ((m_pSink != NULL) && (!m_pSink->ThreadStoppedOnError())) {
        m_pSink->StopThreadOnError();
    }
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_BOOL ISource::ThreadStoppedOnError() {

    {
        Mutex::Autolock autoLock(m_pLock);
        return m_bThreadStop;
    }
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE ISource::ThreadRun(OMX_PTR pThreadData) {
    (void)pThreadData;

    m_bSourceResult = OMX_ErrorUndefined;
    return m_bSourceResult;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE ISource::FreeBuffer(
        BufferInfo *pBuffer, OMX_U32 ePortIndex) {

    (void)pBuffer, (void)ePortIndex;
    VTEST_MSG_MEDIUM("%s: unimplemented FreeBuffer", Name());
    return OMX_ErrorNone;
}

} // namespace vtest
