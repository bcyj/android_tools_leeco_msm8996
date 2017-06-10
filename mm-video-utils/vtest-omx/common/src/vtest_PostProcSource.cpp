/* ========================================================================= *
  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
* ========================================================================= */

#include "vtest_Debug.h"
#include "vtest_PostProcSource.h"
#include "vtest_ISource.h"
#include "vtest_Time.h"
#include "gralloc_priv.h"
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>

#undef LOG_TAG
#define LOG_TAG "VTEST_POSTPROC_SOURCE"

namespace vtest {

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
PostProcSource::PostProcSource(Crypto *pCrypto)
    : ISource(),
      m_nFrames(0),
      m_nFramerate(0),
      m_nFrameWidth(0),
      m_nFrameHeight(0),
      m_bSecureSession(OMX_FALSE),
      m_pCrypto(pCrypto),
      m_pBufferQueueOut(new SignalQueue(100, sizeof(OMX_BUFFERHEADERTYPE*))),
      m_nInputBufferSize(0),
      m_nOutputBufferSize(0),
      m_pFreeBufferQueue(new SignalQueue(32, sizeof(BufferInfo*))),
      m_pPostProcModule(NULL),
      m_pStats(new Stats()) {

    snprintf(m_pName, PROPERTY_FILENAME_MAX, "PostProcSource");
    VTEST_MSG_LOW("%s created", Name());
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
PostProcSource::~PostProcSource() {

    if (m_pBufferQueueOut != NULL) {
        delete m_pBufferQueueOut;
        m_pBufferQueueOut = NULL;
    }

    if (m_pFreeBufferQueue != NULL) {
        delete m_pFreeBufferQueue;
        m_pFreeBufferQueue = NULL;
    }

    if (m_pPostProcModule != NULL) {
        m_pPostProcModule->Terminate();
        delete m_pPostProcModule;
        m_pPostProcModule = NULL;
    }

    if (m_pStats != NULL) {
        delete m_pStats;
        m_pStats = NULL;
    }
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
PortBufferCapability PostProcSource::GetBufferRequirements(OMX_U32 ePortIndex) {

    PortBufferCapability sBufCap;

    Mutex::Autolock autoLock(m_pLock);
    memset(&sBufCap, 0, sizeof(PortBufferCapability));

    if ((ePortIndex != PORT_INDEX_IN) && (ePortIndex != PORT_INDEX_OUT)) {
        VTEST_MSG_ERROR("Error: invalid port selection");
        return sBufCap;
    }

    sBufCap.bAllocateBuffer = OMX_FALSE;
    sBufCap.bUseBuffer = OMX_TRUE;
    sBufCap.pSource = this;
    sBufCap.ePortIndex = ePortIndex;
    sBufCap.nWidth = m_nFrameWidth;
    sBufCap.nHeight = m_nFrameHeight;

    OMX_U32 nPostProcBufSize = 0;
    if (m_pPostProcModule != NULL) {
        m_pPostProcModule->GetBufferRequirements(ePortIndex, &nPostProcBufSize);
    }

    sBufCap.nMinBufferSize = nPostProcBufSize;
    sBufCap.nMinBufferCount = 1;
    sBufCap.nBufferUsage = m_bSecureSession ? (GRALLOC_USAGE_PRIVATE_MM_HEAP |
            GRALLOC_USAGE_PROTECTED) : (GRALLOC_USAGE_PRIVATE_IOMMU_HEAP);
    sBufCap.nBufferUsage |= GRALLOC_USAGE_PRIVATE_UNCACHED;
    sBufCap.nExtraBufferCount = 0;
    return sBufCap;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE PostProcSource::Stop() {

    {
        Mutex::Autolock autoLock(m_pLock);
        m_bThreadStop = OMX_TRUE;
        m_pBufferQueueOut->WakeAll();
    }
    return ISource::Stop();
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE PostProcSource::Configure(CodecConfigType *pConfig,
        BufferManager *pBufManager, ISource *pSource, ISource *pSink) {

    OMX_ERRORTYPE result = OMX_ErrorNone;

    result = ISource::Configure(pConfig, pBufManager, pSource, pSink);
    if (result != OMX_ErrorNone) {
        VTEST_MSG_ERROR("DecoderFileSink configure failed");
        return result;
    }

    Mutex::Autolock autoLock(m_pLock);

    m_nFrames = pConfig->nFrames;
    m_nFramerate = pConfig->nFramerate;
    m_nFrameWidth = pConfig->nFrameWidth;
    m_nFrameHeight = pConfig->nFrameHeight;
    m_bSecureSession = pConfig->bSecureSession;

    m_pPostProcModule = CreatePostProcModule(pConfig->ePostProcType);
    if (m_pPostProcModule == NULL) {
        VTEST_MSG_ERROR("Could not create PostProcModule");
        return OMX_ErrorBadParameter;
    }

    PostProcSession pSession;
    memset((void*)&pSession, 0, sizeof(struct PostProcSession));
    pSession.nFrameWidth = m_nFrameWidth;
    pSession.nFrameHeight = m_nFrameHeight;
    pSession.bSecureSession = m_bSecureSession;
    pSession.nInputColorFormat = pConfig->nInputColorFormat;
    pSession.nOutputColorFormat = pConfig->nOutputColorFormat;

    result = m_pPostProcModule->Init(&pSession);
    return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE PostProcSource::SetBuffer(
        BufferInfo *pBuffer, ISource *pSource) {

    OMX_ERRORTYPE result = OMX_ErrorNone;
    {
        Mutex::Autolock autoLock(m_pLock);
        /* TODO : give all buffers back to Buffer Manager */
        if (m_bThreadStop) {
            result = m_pFreeBufferQueue->Push(&pBuffer, sizeof(BufferInfo **));
            return OMX_ErrorNoMore;
        }
    }

    result = ISource::SetBuffer(pBuffer, pSource);
    if (result != OMX_ErrorNone) {
        return result;
    }

    if (pSource == m_pSource) {

        VTEST_MSG_LOW("In queue push (%p %p)",
                pBuffer->pHeaderIn, pBuffer->pHeaderOut);
        result = m_pBufferQueue->Push(&pBuffer, sizeof(BufferInfo **));
        FAILED1(result, "Error: In queue push failed");
    } else if (pSource == m_pSink) {

        VTEST_MSG_LOW("Out queue push (%p %p)",
                pBuffer->pHeaderIn, pBuffer->pHeaderOut);
        result = m_pBufferQueueOut->Push(&pBuffer, sizeof(BufferInfo **));
        FAILED1(result, "Error: Out queue push failed");
    } else {
        VTEST_MSG_ERROR("Invalid source : %s", pSource->Name());
        result = OMX_ErrorBadParameter;
    }
    return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE PostProcSource::UseBuffers(BufferInfo **pBuffers,
        OMX_U32 nWidth, OMX_U32 nHeight, OMX_U32 nBufferCount,
        OMX_U32 nBufferSize, OMX_U32 ePortIndex) {

    (void)nWidth, (void)nHeight;
    {
        Mutex::Autolock autoLock(m_pLock);

        if (ePortIndex == PORT_INDEX_IN) {
            m_nInputBufferSize = nBufferSize;
        } else if (ePortIndex == PORT_INDEX_OUT) {
            m_nOutputBufferSize = nBufferSize;
        }

        VTEST_MSG_HIGH("PostProcSource: %s size %u count %u", OMX_PORT_NAME(ePortIndex),
                (unsigned int)nBufferSize, (unsigned int)nBufferCount);

        for (OMX_U32 i = 0; i < nBufferCount; i++) {

            BufferInfo *pBuffer = &((*pBuffers)[i]);

            OMX_BUFFERHEADERTYPE **pHeaderPtr = pBuffer->GetHeaderPtr(ePortIndex);
            *pHeaderPtr =
                (OMX_BUFFERHEADERTYPE*)calloc(sizeof(OMX_BUFFERHEADERTYPE), 1);
            if (*pHeaderPtr == NULL) {
                VTEST_MSG_ERROR("Error allocating pHeader");
                return OMX_ErrorInsufficientResources;
            }

            (*pHeaderPtr)->pAppPrivate = this;
            private_handle_t* pHandle = (private_handle_t *)pBuffer->pHandle;

            if (!m_bSecureSession) {
                (*pHeaderPtr)->pBuffer = (OMX_U8 *)mmap(NULL, nBufferSize,
                        PROT_READ | PROT_WRITE, MAP_SHARED, pHandle->fd, 0);
                if ((*pHeaderPtr)->pBuffer == MAP_FAILED) {
                    VTEST_MSG_ERROR("Failed to mmap ion memory, errno is %s", strerror(errno));
                    VTEST_MSG_ERROR("Size allocated in handle : %d vs size in BufferManager : %d",
                            pHandle->size, nBufferSize);
                    return OMX_ErrorInsufficientResources;
                }
            } else {
                (*pHeaderPtr)->pBuffer = (OMX_U8 *)(unsigned long)pHandle->fd;
            }

            (*pHeaderPtr)->nAllocLen = nBufferSize;
            // for NativeWindow case, supply all headers
            pBuffer->pHeaderIn = (pBuffer->pHeaderIn == NULL)
                ? *pHeaderPtr : pBuffer->pHeaderIn;
            pBuffer->pHeaderOut = (pBuffer->pHeaderOut == NULL)
                ? *pHeaderPtr : pBuffer->pHeaderOut;

            VTEST_MSG_HIGH("%s use_ct=%u sz=%u handle=0x%x hdr=(%p %p)",
                OMX_PORT_NAME(ePortIndex), (unsigned int)i + 1, (unsigned int)nBufferSize,
                (unsigned int)pBuffer->pHandle, pBuffer->pHeaderIn, pBuffer->pHeaderOut);
        }
    }
    return OMX_ErrorNone;
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE PostProcSource::PortReconfig(OMX_U32 ePortIndex,
        OMX_U32 nWidth, OMX_U32 nHeight, const OMX_CONFIG_RECTTYPE& sRect) {

    OMX_ERRORTYPE result = OMX_ErrorNone;

    VTEST_MSG_LOW("PortReconfig port %s", OMX_PORT_NAME(ePortIndex));
    if (m_nFrameWidth != nWidth || m_nFrameHeight != nHeight) {
        VTEST_MSG_LOW("PortReconfig WxH (%uX%u) ==> WxH (%uX%u)",
                (unsigned int)m_nFrameWidth, (unsigned int)m_nFrameHeight,
                (unsigned int)nWidth, (unsigned int)nHeight);
        VTEST_MSG_ERROR("Port reconfig currently not supported for post proc!");
        return OMX_ErrorUnsupportedSetting;
    } else {
        /* Still report the crop to Native Window but do not change the
         * input width/height. */
        VTEST_MSG_LOW("PortReconfig crop ([%u %u] [%u %u])",
                (unsigned int)sRect.nLeft, (unsigned int)sRect.nTop, (unsigned int)(sRect.nLeft + sRect.nWidth),
                (unsigned int)(sRect.nTop + sRect.nHeight));
    }

    if (m_pSink != NULL) {
        result = m_pSink->PortReconfig(ePortIndex, nWidth, nHeight, sRect);
    }
    return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE PostProcSource::ThreadRun(OMX_PTR pThreadData) {

    (void)pThreadData;
    OMX_ERRORTYPE result = OMX_ErrorNone;
    BufferInfo *pBufferIn = NULL;
    BufferInfo *pBufferOut = NULL;

    for (OMX_U32 i = 1; !m_bThreadStop; i++) {

        OMX_BUFFERHEADERTYPE *pHeaderIn = NULL, *pHeaderOut = NULL;

        result = m_pBufferQueue->Pop(&pBufferIn, sizeof(BufferInfo **), 0);
        VTEST_MSG_LOW("Input queue pop %u of %u (qsize %u)",
                      (unsigned int)i, (unsigned int)m_nFrames, (unsigned int)m_pBufferQueue->GetSize());

        if ((pBufferIn == NULL) || (result != OMX_ErrorNone)) {
            /* Can only happen if stop is called or someone else ran into an
             * error */
            VTEST_MSG_HIGH("Stopping thread");
            result = OMX_ErrorNone;
            continue;
        }

        pHeaderIn = pBufferIn->pHeaderIn;

        if (m_bThreadStop) {
            continue;
        }

        result = m_pBufferQueueOut->Pop(&pBufferOut, sizeof(BufferInfo **), 0);
        VTEST_MSG_LOW("Output queue pop %u of %u (qsize %u)",
                      (unsigned int)i, (unsigned int)m_nFrames, (unsigned int)m_pBufferQueueOut->GetSize());

        if ((pBufferOut == NULL) || (result != OMX_ErrorNone)) {
            /* Can only happen if stop is called or someone else ran into an
             * error */
            VTEST_MSG_HIGH("Stopping thread");
            result = OMX_ErrorNone;
            continue;
        }

        pHeaderOut = pBufferOut->pHeaderOut;
        pHeaderOut->nTimeStamp = pHeaderIn->nTimeStamp;
        pHeaderOut->nFlags = pHeaderIn->nFlags;
        pHeaderOut->nFilledLen = pHeaderIn->nFilledLen;

        if (pHeaderIn->nFlags & OMX_BUFFERFLAG_EOS) {
            VTEST_MSG_HIGH("Got EOS for frame : %u", (unsigned int)i);
            m_bThreadStop = OMX_TRUE;
        }

        if (pHeaderIn->nFilledLen > 0) {
            Stats::AutoStats sAutoStats(m_pStats);
            result = m_pPostProcModule->Perform(pBufferIn, pBufferOut);
            FAILED2(result, SetError(), "Error while PostProcessing buffer : %p", pHeaderIn);
        } else  {
            VTEST_MSG_HIGH("Skipping frame because of 0 length %u...", (unsigned int)i);
        }

        m_pPostProcModule->UpdateOutputBufferGeometry(pBufferOut);
        m_pSource->SetBuffer(pBufferIn, this);
        m_pSink->SetBuffer(pBufferOut, this);
    }

    //clean up
    while(m_pBufferQueue->GetSize() > 0) {
        VTEST_MSG_LOW("Cleanup: In q-wait (qsize %u)", (unsigned int)m_pBufferQueue->GetSize());
        m_pBufferQueue->Pop(&pBufferIn, sizeof(BufferInfo **), 0);
        m_pSource->SetBuffer(pBufferIn, this);
    }

    while(m_pBufferQueueOut->GetSize() > 0) {
        VTEST_MSG_LOW("Cleanup: Out q-wait (qsize %u)", (unsigned int)m_pBufferQueueOut->GetSize());
        m_pBufferQueueOut->Pop(&pBufferOut, sizeof(BufferInfo **), 0);
        m_pSink->SetBuffer(pBufferOut, this);
    }

    VTEST_MSG_HIGH("Thread is exiting...");
    return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE PostProcSource::FreeBuffer(
        BufferInfo *pBuffer, OMX_U32 ePortIndex) {

    OMX_ERRORTYPE result = OMX_ErrorNone;
    OMX_U32 nBufferSize = 0;

    OMX_BUFFERHEADERTYPE **pHeaderPtr = pBuffer->GetHeaderPtr(ePortIndex);
    if ((*pHeaderPtr == NULL) || ((*pHeaderPtr)->pAppPrivate != this)) {
        VTEST_MSG_MEDIUM("Not my pHeaderPtr : %p", *pHeaderPtr);
        return result;
    }

    VTEST_MSG_MEDIUM("Freeing buffer %p 0x%lu", *pHeaderPtr, pBuffer->pHandle);

    if (ePortIndex == PORT_INDEX_IN) {
        nBufferSize = m_nInputBufferSize;
    } else if (ePortIndex == PORT_INDEX_OUT) {
        nBufferSize = m_nOutputBufferSize;
    }

    if (!m_bSecureSession) {
        if (munmap((*pHeaderPtr)->pBuffer, nBufferSize)) {
            VTEST_MSG_ERROR("munmap failed for header : %p", *pHeaderPtr);
        }
    }

    free(*pHeaderPtr);
    *pHeaderPtr = NULL;
    return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
PostProcSource::Stats::Stats()
    : m_nMax(0),
        m_nMin(0),
        m_nAvg(0),
        m_nTotal(0),
        m_nCount(0) {}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
PostProcSource::Stats::~Stats() {

    VTEST_MSG_HIGH(
            "Stats are as follows:\nTotal : %u ms\nFrameCount : %u\nAvg : %u ms\nMax : %u ms\nMin : %u ms",
            m_nTotal, m_nCount, m_nAvg, m_nMax, m_nMin);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
PostProcSource::Stats::AutoStats::AutoStats(Stats *pStats)
    : m_pStats(pStats),
        m_nEndTime(0),
        m_nRunTimeMillis(0) {

    VTEST_MSG_HIGH("Autostats: Start");
    m_nStartTime = Time::GetTimeMicrosec();
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
PostProcSource::Stats::AutoStats::~AutoStats() {

    m_nEndTime = Time::GetTimeMicrosec();
    m_nRunTimeMillis = (m_nEndTime - m_nStartTime) / 1000;   // convert to millis
    m_pStats->m_nCount++;
    if (m_pStats->m_nCount == 1) {
        m_pStats->m_nMin = m_nRunTimeMillis;
        m_pStats->m_nMax = m_nRunTimeMillis;
    } else {
        m_pStats->m_nMin = MIN(m_pStats->m_nMin, m_nRunTimeMillis);
        m_pStats->m_nMax = MAX(m_pStats->m_nMax, m_nRunTimeMillis);
    }
    m_pStats->m_nTotal += m_nRunTimeMillis;
    m_pStats->m_nAvg = m_pStats->m_nTotal / m_pStats->m_nCount;
    VTEST_MSG_HIGH("AutoStats: End: Runtime : %dms", (int)m_nRunTimeMillis);
}

} // namespace vtest
