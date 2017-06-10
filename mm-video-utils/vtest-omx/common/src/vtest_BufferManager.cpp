/*-------------------------------------------------------------------
Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential
--------------------------------------------------------------------*/

#include "vtest_Debug.h"
#include "vtest_BufferManager.h"
#include "vtest_ISource.h"

#undef LOG_TAG
#define LOG_TAG "VTEST_BUFFER_MANAGER"

namespace vtest {

BufferManager::BufferManager() {}

BufferManager::~BufferManager() {

    VTEST_MSG_LOW("~BufferManager start");
    for (int i = m_pBufferPools.size() - 1; i >= 0; i--) {

        BufferPool *pBufferPool = m_pBufferPools[i];

        if (pBufferPool->pAllocateNode) {
            delete pBufferPool->pAllocateNode;
            pBufferPool->pAllocateNode = NULL;
        }
        if (pBufferPool->pUseNode) {
            delete pBufferPool->pUseNode;
            pBufferPool->pUseNode = NULL;
        }
        if (pBufferPool->pLock) {
            delete pBufferPool->pLock;
            pBufferPool->pLock = NULL;
        }

        m_pBufferPools.pop_back();
        delete pBufferPool;
    }
    VTEST_MSG_LOW("~BufferManager done");
}

OMX_ERRORTYPE BufferManager::GetBuffers(ISource *pNode,
        OMX_U32 ePortIndex, BufferInfo **pBuffers, OMX_U32 *nBufferCount) {

    OMX_ERRORTYPE result = OMX_ErrorNone;
    VTEST_MSG_MEDIUM("GetBuffers %s %s",
                     pNode->Name(), OMX_PORT_NAME(ePortIndex));

    BufferPool *pBufferPool = GetBufferPool(pNode, ePortIndex);
    if (pBufferPool == NULL) {
        VTEST_MSG_HIGH("Did not find BufferPool for %s on %s",
                            pNode->Name(), OMX_PORT_NAME(ePortIndex));
        return OMX_ErrorBadParameter;
    }

    Mutex::Autolock autoLock(pBufferPool->pLock);
    *pBuffers = pBufferPool->pBuffers;
    *nBufferCount = pBufferPool->nBufferCount;
    VTEST_MSG_LOW("%s: GetBuffers buf=%p ctr=%u",
                  pNode->Name(),*pBuffers,(unsigned int)*nBufferCount);
    return result;
}

OMX_ERRORTYPE BufferManager::GetBuffer(ISource *pNode,
        OMX_U32 ePortIndex, OMX_BUFFERHEADERTYPE *pHeader,
        BufferInfo **pBuffer) {

    OMX_ERRORTYPE result = OMX_ErrorNone;

    BufferPool *pBufferPool = GetBufferPool(pNode, ePortIndex);
    if (pBufferPool == NULL) {
        VTEST_MSG_ERROR("Error did not find BufferPool for %s on %s",
                            pNode->Name(), OMX_PORT_NAME(ePortIndex));
        return OMX_ErrorBadParameter;
    }

    Mutex::Autolock autoLock(pBufferPool->pLock);

    for (OMX_U32 i = 0; i < pBufferPool->nBufferCount; i++) {

        if (pBufferPool->pBuffers[i].pHeaderIn == pHeader ||
            pBufferPool->pBuffers[i].pHeaderOut == pHeader) {
            *pBuffer = &pBufferPool->pBuffers[i];
            break;
        }
    }
    VTEST_MSG_LOW("%s: GetBuffer hdr: %p, 0x%lx (%p %p)",
                  pNode->Name(), pHeader, (*pBuffer)->pHandle,
                  (*pBuffer)->pHeaderIn, (*pBuffer)->pHeaderOut);
    return result;
}

OMX_ERRORTYPE BufferManager::FreeBuffers(ISource *pNode, OMX_U32 ePortIndex) {

    VTEST_MSG_MEDIUM("");
    BufferPool *pBufferPool = GetBufferPool(pNode, ePortIndex);
    if (pBufferPool == NULL) {
        VTEST_MSG_ERROR("Could not find Buffer Pool for this source %s on this port : %u",
                pNode->Name(), (unsigned int)ePortIndex);
        return OMX_ErrorBadParameter;
    }

    VTEST_MSG_MEDIUM("pBuffers : 0x%p", pBufferPool->pBuffers);
    if (pBufferPool->pBuffers != NULL) {

            VTEST_MSG_MEDIUM("Freeing %u Buffers for %s ---- %s",
                    (unsigned int)pBufferPool->nBufferCount,
                    pBufferPool->pAllocateNode->pSource->Name(),
                    pBufferPool->pUseNode->pSource->Name());

        for (OMX_U32 i = 0; i < pBufferPool->nBufferCount; i++) {

            VTEST_MSG_LOW("FreeBuffer: 0x%lu pHeader: (%p %p)",
                    pBufferPool->pBuffers[i].pHandle,
                    pBufferPool->pBuffers[i].pHeaderIn,
                    pBufferPool->pBuffers[i].pHeaderOut);
        }
        pBufferPool->pAllocateNode->pSource->FreeAllocatedBuffers(
                &pBufferPool->pBuffers, pBufferPool->nBufferCount,
                pBufferPool->pAllocateNode->ePortIndex);
        pBufferPool->pUseNode->pSource->FreeUsedBuffers(
                &pBufferPool->pBuffers, pBufferPool->nBufferCount,
                pBufferPool->pUseNode->ePortIndex);

        /* Cannot hold this lock while calling FreeBuffers on nodes
         * because it can result in a deadlock with the openmax
         * callback thread and its lock(example, while freeing the
         * last buffer it wants to send flush done and at the same
         * time we get an FBD which calls getBuffer for the other
         * pool */
        Mutex::Autolock autoLock(pBufferPool->pLock);
        delete[] pBufferPool->pBuffers;
        pBufferPool->pBuffers = NULL;
    }
    return OMX_ErrorNone;
}

OMX_ERRORTYPE BufferManager::SetupBufferPool(ISource *pNodeSrc, ISource *pNodeSink) {

    OMX_ERRORTYPE result = OMX_ErrorNone;
    BufferPool *pBufferPool = NULL;
    PortBufferCapability sPortCapSrc = pNodeSrc->GetBufferRequirements(PORT_INDEX_OUT);
    PortBufferCapability sPortCapSink = pNodeSink->GetBufferRequirements(PORT_INDEX_IN);
    OMX_U32 nWidth = 0;
    OMX_U32 nHeight = 0;
    OMX_U32 nBufferCount = 0;
    OMX_U32 nBufferSize = 0;
    OMX_U32 nBufferUsage = 0;
    BufferInfo *pBuffers = NULL;

    // sanity checks
    if (pNodeSrc->Sink() != pNodeSink || pNodeSrc != pNodeSink->Source()) {
        VTEST_MSG_HIGH("ERROR: cannot allocate unlinked objects %s and %s",
                       pNodeSrc->Name(), pNodeSink->Name());
        return OMX_ErrorInvalidState;
    }

    if ((!sPortCapSrc.nWidth && !sPortCapSink.nWidth)
        || (!sPortCapSrc.nHeight && !sPortCapSink.nHeight)) {

        VTEST_MSG_ERROR(
                "BufferManager not configured properly for nodes : %s and %s, width : %u or height : %u is 0",
                pNodeSrc->Name(), pNodeSink->Name(),
                (unsigned int)sPortCapSrc.nWidth, (unsigned int)sPortCapSrc.nHeight);
        return OMX_ErrorBadParameter;
    }

    nWidth = MAX(sPortCapSrc.nWidth, sPortCapSink.nWidth);
    nHeight = MAX(sPortCapSrc.nHeight, sPortCapSink.nHeight);
    nBufferCount = MAX(sPortCapSrc.nMinBufferCount, sPortCapSink.nMinBufferCount)
        + sPortCapSrc.nExtraBufferCount + sPortCapSink.nExtraBufferCount;
    nBufferSize = MAX(sPortCapSrc.nMinBufferSize, sPortCapSink.nMinBufferSize);
    nBufferUsage = sPortCapSrc.nBufferUsage | sPortCapSink.nBufferUsage;

    PortBufferCapability *pPortCapForSamePool;
    pPortCapForSamePool = sPortCapSrc.bUseSameBufferPool ? &sPortCapSrc :
                        sPortCapSink.bUseSameBufferPool ? &sPortCapSink : NULL;

    if ((pPortCapForSamePool != NULL)
            && (pPortCapForSamePool->pSource->Source() != NULL)
            && (pPortCapForSamePool->pSource->Sink() != NULL)) {

        PortBufferCapability sPortCapOtherPort;
        sPortCapOtherPort = pPortCapForSamePool->pSource->GetBufferRequirements(
                (pPortCapForSamePool->ePortIndex == PORT_INDEX_IN) ? PORT_INDEX_OUT
                : PORT_INDEX_IN);
        if ((pPortCapForSamePool->bAllocateBuffer && sPortCapOtherPort.bAllocateBuffer)
            && (!pPortCapForSamePool->bUseBuffer && !sPortCapOtherPort.bUseBuffer)
            && (pPortCapForSamePool->nWidth == sPortCapOtherPort.nWidth)
            && (pPortCapForSamePool->nHeight == sPortCapOtherPort.nHeight)
            && (pPortCapForSamePool->nMinBufferSize ==
                                        sPortCapOtherPort.nMinBufferSize)
            && (pPortCapForSamePool->nMinBufferCount ==
                                        sPortCapOtherPort.nMinBufferCount)
            && (pPortCapForSamePool->nExtraBufferCount ==
                                        sPortCapOtherPort.nExtraBufferCount)
            && (pPortCapForSamePool->nBufferUsage ==
                                        sPortCapOtherPort.nBufferUsage)
            && (pPortCapForSamePool->bUseSameBufferPool ==
                                        sPortCapOtherPort.bUseSameBufferPool)) {
            VTEST_MSG_HIGH("Node : %s wants to use same buffer pool on both ports",
                    pPortCapForSamePool->pSource->Name());

            /* Make sure the buffer count is max of what is calculated + both source and sink req*/
            VTEST_MSG_HIGH("Old Buffer count : %d", nBufferCount);
            nBufferCount = MAX(nBufferCount,
                    MAX(pPortCapForSamePool->pSource->Source()->GetBufferRequirements(PORT_INDEX_OUT).nMinBufferCount,
                    pPortCapForSamePool->pSource->Sink()->GetBufferRequirements(PORT_INDEX_IN).nMinBufferCount));
            VTEST_MSG_HIGH("Adjusted buffer count to accomodate for other port source, count : %d", nBufferCount);

            /* We are trying to allocate for pPortCapForSamePool->ePortIndex,
             * so check if there is already a buffer pool for the other port
             * (sPortCapOtherPort.ePortIndex). */
            OMX_U32 nBufCountTemp = 0;
            BufferInfo *pBufTemp = NULL;
            result = GetBuffers(pPortCapForSamePool->pSource,
                    sPortCapOtherPort.ePortIndex, &pBufTemp, &nBufCountTemp);
            if (result == OMX_ErrorNone) {
                if (nBufCountTemp != nBufferCount) {
                    VTEST_MSG_ERROR(
                            "Buffer counts for same pool allocation on both ports don't match : (%u) vs (%u)",
                            (unsigned int)nBufCountTemp, (unsigned int)nBufferCount);
                    return OMX_ErrorUndefined;
                }
                VTEST_MSG_HIGH("Same pool allocation succeeded!");
                pBuffers = new BufferInfo[nBufferCount];
                memset(pBuffers, 0, sizeof(BufferInfo) * nBufferCount);
                memcpy(pBuffers, pBufTemp, sizeof(BufferInfo) * nBufferCount);
            }
        } else {
            VTEST_MSG_ERROR(
                    "BufferManager not configured properly for node : %s, to use same buffer pool",
                    pPortCapForSamePool->pSource->Name());
                    return OMX_ErrorUndefined;
        }
    }

    pBufferPool = GetBufferPool(pNodeSrc, pNodeSink);
    if (pBufferPool == NULL) {
        VTEST_MSG_HIGH("Allocating buffer pool for: %s (source) ==> %s (sink)",
            pNodeSrc->Name(), pNodeSink->Name());

        pBufferPool = new BufferPool();
        memset(pBufferPool, 0, sizeof(struct BufferPool));
        pBufferPool->pLock = new Mutex();
        m_pBufferPools.push_back(pBufferPool);
    } else {
        // this is a re-allocation, verify a clean up has been done
        if (pBufferPool->pBuffers != NULL) {
            VTEST_MSG_HIGH("ERROR: buffer pool re-allocation memory leak!!!");
            return OMX_ErrorInvalidState;
        }

        VTEST_MSG_HIGH("Re-Allocating buffer pool for: %s (source) ==> %s (sink)",
            pNodeSrc->Name(), pNodeSink->Name());
    }

    Mutex::Autolock autoLock(pBufferPool->pLock);

    pBufferPool->pAllocateNode = new Node();
    pBufferPool->pUseNode = new Node();
    if (sPortCapSrc.bAllocateBuffer && sPortCapSink.bUseBuffer) {
        pBufferPool->pAllocateNode->pSource = pNodeSrc;
        pBufferPool->pAllocateNode->ePortIndex = PORT_INDEX_OUT;
        pBufferPool->pAllocateNode->sPortCaps = sPortCapSrc;
        pBufferPool->pUseNode->pSource = pNodeSink;
        pBufferPool->pUseNode->ePortIndex = PORT_INDEX_IN;
        pBufferPool->pUseNode->sPortCaps = sPortCapSink;
    } else if (sPortCapSink.bAllocateBuffer && sPortCapSrc.bUseBuffer) {
        pBufferPool->pAllocateNode->pSource = pNodeSink;
        pBufferPool->pAllocateNode->ePortIndex = PORT_INDEX_IN;
        pBufferPool->pAllocateNode->sPortCaps = sPortCapSink;
        pBufferPool->pUseNode->pSource = pNodeSrc;
        pBufferPool->pUseNode->ePortIndex = PORT_INDEX_OUT;
        pBufferPool->pUseNode->sPortCaps = sPortCapSrc;
    } else {
        VTEST_MSG_ERROR(
            "BufferManager not configured properly for nodes : %s and %s",
            pNodeSrc->Name(), pNodeSink->Name());
        return OMX_ErrorBadParameter;
    }

    pBufferPool->nWidth = nWidth;
    pBufferPool->nHeight = nHeight;
    pBufferPool->nBufferCount = nBufferCount;
    pBufferPool->nBufferSize = nBufferSize;
    pBufferPool->nBufferUsage = nBufferUsage;

    VTEST_MSG_HIGH("BufferPool: %s (alloc) ==> %s (use)",
            pBufferPool->pAllocateNode->pSource->Name(),
            pBufferPool->pUseNode->pSource->Name());
    VTEST_MSG_MEDIUM("BufferPool: count %u size %u usage 0x%x",
            (unsigned int)pBufferPool->nBufferCount, (unsigned int)pBufferPool->nBufferSize,
            (unsigned int)pBufferPool->nBufferUsage);
    VTEST_MSG_MEDIUM("BufferPool: height %u width %u",
            (unsigned int)pBufferPool->nHeight, (unsigned int)pBufferPool->nWidth);
    VTEST_MSG_LOW("BufferPool: src.buf_ct %u sink.buf_ct %u src.buf_xtr 0x%x sink.buf_xtr %u",
        (unsigned int)sPortCapSrc.nMinBufferCount, (unsigned int)sPortCapSink.nMinBufferCount,
        (unsigned int)sPortCapSrc.nExtraBufferCount, (unsigned int)sPortCapSink.nExtraBufferCount);

    /* pBuffers will be NULL always except when a node wants to use same buffer
     * pool between both ports */
    if (pBuffers == NULL) {
        result = pBufferPool->pAllocateNode->pSource->AllocateBuffers(&pBufferPool->pBuffers,
                pBufferPool->nWidth, pBufferPool->nHeight, pBufferPool->nBufferCount,
                pBufferPool->nBufferSize, pBufferPool->pAllocateNode->ePortIndex,
                pBufferPool->nBufferUsage);
        FAILED1(result, "Allocate buffer on node %s failed",
                pBufferPool->pAllocateNode->pSource->Name())
    } else {
        pBufferPool->pBuffers = pBuffers;
    }
    result = pBufferPool->pUseNode->pSource->UseBuffers(&pBufferPool->pBuffers,
            pBufferPool->nWidth, pBufferPool->nHeight, pBufferPool->nBufferCount,
            pBufferPool->nBufferSize, pBufferPool->pUseNode->ePortIndex);
    FAILED1(result, "Usebuffer on node %s failed",
            pBufferPool->pUseNode->pSource->Name())

    // ensure headers are available for both nodes
    for (OMX_U32 i = 0; i < pBufferPool->nBufferCount; i++) {

        if (pBufferPool->pBuffers[i].pHandle == 0 ||
            pBufferPool->pBuffers[i].pHeaderIn == NULL ||
            pBufferPool->pBuffers[i].pHeaderOut == NULL) {
            VTEST_MSG_ERROR("Error: missing info, headers: (%p %p) handle: 0x%lx",
                    pBufferPool->pBuffers[i].pHeaderIn,
                    pBufferPool->pBuffers[i].pHeaderOut,
                    pBufferPool->pBuffers[i].pHandle);
            result = OMX_ErrorBadParameter;
            break;
        }
    }

    return result;
}

BufferPool* BufferManager::GetBufferPool(ISource *pNode1, ISource *pNode2) {

    BufferPool *pBufferPool = NULL;
    for (OMX_U32 i = 0; i < m_pBufferPools.size(); i++) {

        pBufferPool = m_pBufferPools[i];
        Mutex::Autolock autoLock(pBufferPool->pLock);

        if ((pBufferPool->pAllocateNode->pSource == pNode1 &&
             pBufferPool->pUseNode->pSource == pNode2) ||
            (pBufferPool->pAllocateNode->pSource == pNode2 &&
             pBufferPool->pUseNode->pSource == pNode1)) {
            return pBufferPool;
        }
    }
    return NULL;
}

BufferPool* BufferManager::GetBufferPool(ISource *pNode, OMX_U32 ePortIndex) {

    BufferPool *pBufferPool = NULL;
    for (OMX_U32 i = 0; i < m_pBufferPools.size(); i++) {

        pBufferPool = m_pBufferPools[i];
        Mutex::Autolock autoLock(pBufferPool->pLock);

        if (pBufferPool->pAllocateNode) {
            if ((pBufferPool->pAllocateNode->pSource == pNode) &&
                (pBufferPool->pAllocateNode->ePortIndex == ePortIndex)) {
                return pBufferPool;
            }
        }
        if (pBufferPool->pUseNode) {
            if ((pBufferPool->pUseNode->pSource == pNode) &&
                (pBufferPool->pUseNode->ePortIndex == ePortIndex)) {
                return pBufferPool;
            }
        }
    }
    VTEST_MSG_HIGH("No BufferPool for %s on %s",
                    pNode->Name(), OMX_PORT_NAME(ePortIndex));
    return NULL;
}

BufferPool* BufferManager::GetBufferPool(ISource *pNode, BufferInfo *pBuffer) {

    BufferPool *pBufferPool = NULL;
    for (OMX_U32 i = 0; i < m_pBufferPools.size(); i++) {

        pBufferPool = m_pBufferPools[i];
        Mutex::Autolock autoLock(pBufferPool->pLock);

        if (pNode == pBufferPool->pAllocateNode->pSource ||
            pNode == pBufferPool->pUseNode->pSource) {
            for (OMX_U32 j = 0; j < pBufferPool->nBufferCount; j++) {
                if (pBuffer == &pBufferPool->pBuffers[j]) {
                    return pBufferPool;
                }
            }
        }
    }
    VTEST_MSG_ERROR("Error no BufferPool for %s buffer %p",
                    pNode->Name(), pBuffer);
    return NULL;
}

}
