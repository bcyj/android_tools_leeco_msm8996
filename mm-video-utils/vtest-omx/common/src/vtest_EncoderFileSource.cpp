/*-------------------------------------------------------------------
Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential

Copyright (c) 2010 The Linux Foundation. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of The Linux Foundation nor
      the names of its contributors may be used to endorse or promote
      products derived from this software without specific prior written
      permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NON-INFRINGEMENT ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
--------------------------------------------------------------------------*/

#include "venc/inc/omx_video_common.h"
#include "vtest_ComDef.h"
#include "vtest_Debug.h"
#include "vtest_Thread.h"
#include "vtest_Sleeper.h"
#include "vtest_File.h"
#include "vtest_Time.h"
#include "vtest_SignalQueue.h"
#include "vtest_BufferManager.h"
#include "vtest_EncoderFileSource.h"
#include "vt_ion_allocator.h"

#undef LOG_TAG
#define LOG_TAG "VTEST_ENCODER_FILE_SOURCE"

namespace vtest {

static const OMX_S32 MAX_BUFFER_ASSUME = 16;

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
EncoderFileSource::EncoderFileSource(Crypto *pCrypto)
    : ISource(),
      m_nFrames(0),
      m_nFramerate(0),
      m_nFrameWidth(0),
      m_nFrameHeight(0),
      m_nBuffers(0),
      m_nDVSXOffset(0),
      m_nDVSYOffset(0),
      m_pFile(NULL),
      m_bIsProfileMode(OMX_FALSE),
      m_bSecureSession(OMX_FALSE),
      m_bIsVTPath(OMX_FALSE),
      m_pInCopyBuf(NULL),
      m_nInCopyBufSize(0),
      m_pCrypto(pCrypto),
      m_bMetaMode(OMX_FALSE),
      m_eYuvColorSpace(ITUR601) {

    snprintf(m_pName, PROPERTY_FILENAME_MAX, "EncoderFileSource");
    VTEST_MSG_LOW("%s: created...", Name());
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
EncoderFileSource::~EncoderFileSource() {

    VTEST_MSG_LOW("start");
    if (m_pFile != NULL) {
        (void)m_pFile->Close();
        delete m_pFile;
    }
    if (m_pInCopyBuf != NULL) {
        delete m_pInCopyBuf;
    }
    VTEST_MSG_LOW("done");
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
PortBufferCapability EncoderFileSource::GetBufferRequirements(OMX_U32 ePortIndex) {

    PortBufferCapability sBufCap;

    Mutex::Autolock autoLock(m_pLock);
    memset(&sBufCap, 0, sizeof(PortBufferCapability));

    if (ePortIndex == PORT_INDEX_OUT) {
        if (m_bMetaMode) {
            sBufCap.bAllocateBuffer = OMX_TRUE;
            sBufCap.bUseBuffer = OMX_FALSE;
        } else {
            sBufCap.bAllocateBuffer = OMX_FALSE;
            sBufCap.bUseBuffer = OMX_TRUE;
        }
        sBufCap.pSource = this;
        sBufCap.ePortIndex = ePortIndex;
        sBufCap.nMinBufferSize = 0x1000;
        sBufCap.nMinBufferCount = 1;
        sBufCap.nExtraBufferCount = 0;
        sBufCap.nBufferUsage = 0;
    } else {
        VTEST_MSG_ERROR("Error: invalid port selection");
    }
    return sBufCap;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE EncoderFileSource::Configure(CodecConfigType *pConfig,
        BufferManager *pBufManager, ISource *pSource, ISource *pSink) {

    OMX_ERRORTYPE result = OMX_ErrorNone;

    result = ISource::Configure(pConfig, pBufManager, pSource, pSink);
    if (result != OMX_ErrorNone) {
        VTEST_MSG_ERROR("EncoderFileSource configure failed");
        return result;
    }

    Mutex::Autolock autoLock(m_pLock);

    OMX_STRING pFileName = pConfig->cInFileName;
    m_bSecureSession = pConfig->bSecureSession;
    m_bMetaMode = pConfig->bMetaMode;
    m_eYuvColorSpace = pConfig->eYuvColorSpace;

    VTEST_MSG_HIGH("MetaMode %s", m_bMetaMode ? "Enabled" : "Disabled");
    VTEST_MSG_MEDIUM("YuvColorSpace : 0x%x", m_eYuvColorSpace);

    if (pConfig->nFrames > 0 &&
        pConfig->nFramerate > 0 &&
        pConfig->nFrameWidth > 0 &&
        pConfig->nFrameHeight > 0 &&
        pConfig->nDVSXOffset >= 0 &&
        pConfig->nDVSYOffset >= 0 &&
        pConfig->nInBufferCount > 0 &&
        pConfig->nInBufferCount <= MAX_BUFFER_ASSUME) {
        m_nFrames = pConfig->nFrames;
        m_nFramerate = pConfig->nFramerate;
        m_nFrameWidth = pConfig->nFrameWidth;
        m_nFrameHeight = pConfig->nFrameHeight;
        m_nBuffers = pConfig->nInBufferCount;
        m_nDVSXOffset = pConfig->nDVSXOffset;
        m_nDVSYOffset = pConfig->nDVSYOffset;
        m_bIsProfileMode = pConfig->bProfileMode;

        if (pFileName != NULL) {
            m_pFile = new File();
            if (m_pFile != NULL) {
                result = m_pFile->Open(pFileName, OMX_TRUE);
                if (result != OMX_ErrorNone) {
                    VTEST_MSG_ERROR("Failed to open file");
                }
            } else {
                VTEST_MSG_ERROR("Failed to allocate file");
                result = OMX_ErrorInsufficientResources;
            }
        }
    } else {
        VTEST_MSG_ERROR("bad params");
        result = OMX_ErrorBadParameter;
    }

    return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE EncoderFileSource::SetBuffer(
        BufferInfo *pBuffer, ISource *pSource) {

    OMX_ERRORTYPE result = OMX_ErrorNone;

    result = ISource::SetBuffer(pBuffer, pSource);
    if (result != OMX_ErrorNone) {
        return result;
    }

    VTEST_MSG_LOW("queue push (%p %p)", pBuffer->pHeaderIn, pBuffer->pHeaderOut);
    result = m_pBufferQueue->Push(&pBuffer, sizeof(BufferInfo **));
    return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE EncoderFileSource::AllocateBuffers(BufferInfo **pBuffers,
        OMX_U32 nWidth, OMX_U32 nHeight, OMX_U32 nBufferCount,
        OMX_U32 nBufferSize, OMX_U32 ePortIndex, OMX_U32 nBufferUsage) {

    (void)nWidth, (void)nHeight, (void)nBufferUsage;
    OMX_ERRORTYPE result = OMX_ErrorNone;

    Mutex::Autolock autoLock(m_pLock);
    VTEST_MSG_HIGH("alloc %s size %u count %u",
            OMX_PORT_NAME(ePortIndex), (unsigned int)nBufferSize, (unsigned int)nBufferCount);

    if (ePortIndex != PORT_INDEX_OUT) {
        return OMX_ErrorBadParameter;
    }

    if (pBuffers == NULL) {
        VTEST_MSG_ERROR("%s - invalid input", __FUNCTION__);
        return OMX_ErrorBadParameter;
    }

    *pBuffers = new BufferInfo[nBufferCount];
    memset(*pBuffers, 0, sizeof(BufferInfo) * nBufferCount);

    for (OMX_U32 i = 0; i < nBufferCount; i++) {

        BufferInfo *pBuffer = &((*pBuffers)[i]);

        result = AllocateBuffer(pBuffer, ePortIndex, nBufferSize);
        FAILED1(result, "error allocating buffer");

        VTEST_MSG_HIGH("%s alloc_ct=%u sz=%u handle=%p hdr=(%p %p)",
                OMX_PORT_NAME(ePortIndex), (unsigned int)i+1, (unsigned int)nBufferSize,
                (void *)pBuffer->pHandle, pBuffer->pHeaderIn, pBuffer->pHeaderOut);
    }
    return result;
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE EncoderFileSource::ChangeFrameRate(OMX_S32 nFramerate) {

    OMX_ERRORTYPE result = OMX_ErrorNone;
    Mutex::Autolock autoLock(m_pLock);

    if (nFramerate > 0) {
        m_nFramerate = nFramerate;
    } else {
        VTEST_MSG_ERROR("bad frame rate");
        result = OMX_ErrorBadParameter;
    }
    return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE EncoderFileSource::ThreadRun(OMX_PTR pThreadData) {

    (void)pThreadData;
    OMX_ERRORTYPE result = OMX_ErrorNone;
    OMX_BUFFERHEADERTYPE *pHeader = NULL;
    BufferInfo *pBuffer = NULL;
    OMX_S32 nFrameBytes = m_nFrameWidth * m_nFrameHeight * 3 / 2;
    OMX_TICKS nTimeStamp = 0;

    for (OMX_S32 i = 1; i <= m_nFrames && !m_bThreadStop; i++) {
        // Since frame rate can change at any time, let's make sure that we use
        // the same frame rate for the duration of this loop iteration
        OMX_S32 nFramerate = m_nFramerate;

        // If in live mode we deliver frames in a real-time fashion
        if (m_bIsProfileMode) {
            VTEST_MSG_LOW("delaying frame %u ms", (unsigned int)(1000 / nFramerate));
            Sleeper::Sleep(1000 / nFramerate);
        }

        result = m_pBufferQueue->Pop(&pBuffer, sizeof(BufferInfo**), 0);
        VTEST_MSG_LOW("queue pop %u of %u...", (unsigned int)i, (unsigned int)m_nFrames);

        if ((pBuffer == NULL) || (result != OMX_ErrorNone)) {
            /* Can only happen if stop is called or someone else ran into an
             * error */
            VTEST_MSG_HIGH("Stopping thread");
            result = OMX_ErrorNone;
            continue;
        }

        pHeader = pBuffer->pHeaderOut;
        pHeader->nOffset = 0;
        pHeader->nTimeStamp = 0;
        pHeader->nFlags = 0;
        pHeader->nFilledLen = 0;

        result = FillBuffer(pHeader, nFrameBytes);
        if (result != OMX_ErrorNone) {
            SetError();
            pHeader->nFlags = OMX_BUFFERFLAG_EOS;
            m_pSink->SetBuffer(pBuffer, this);
            continue;
        }

        // set the EOS flag if this is the last frame
        if (i >= m_nFrames) {
            pHeader->nFlags = OMX_BUFFERFLAG_EOS;
            VTEST_MSG_HIGH("enable OMX_BUFFERFLAG_EOS on frame %u", (unsigned int)i);
            m_bThreadStop = OMX_TRUE;
        }

        pHeader->nFilledLen = nFrameBytes;
        if (m_bIsProfileMode) {
            nTimeStamp = (OMX_TICKS)Time::GetTimeMicrosec();
        } else {
            nTimeStamp = nTimeStamp + (OMX_TICKS)(1000000 / nFramerate);
        }
        pHeader->nTimeStamp = nTimeStamp;

        VTEST_MSG_MEDIUM("delivering frame %u", (unsigned int)i);
        pHeader->nOffset = ((m_nFrameWidth * m_nDVSYOffset) + m_nDVSXOffset) * 3 / 2;
        m_pSink->SetBuffer(pBuffer, this);
    } // end for-loop

    //clean up
    while(m_pBufferQueue->GetSize() > 0) {
        VTEST_MSG_LOW("cleanup: q-wait (qsize %u)", (unsigned int)m_pBufferQueue->GetSize());
        m_pBufferQueue->Pop(&pBuffer, sizeof(BufferInfo **), 0);
        m_pSink->SetBuffer(pBuffer, this);
    }

    VTEST_MSG_HIGH("thread exiting...");
    return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE EncoderFileSource::FillBuffer(
        OMX_BUFFERHEADERTYPE *pHeader, OMX_S32 nFrameBytes) {

    OMX_ERRORTYPE result = OMX_ErrorNone;
    OMX_S32 nBytesRead;
    OMX_U8 *buffer;

    if (m_pFile == NULL) {
        VTEST_MSG_ERROR("EncoderFileSource file is null");
        return OMX_ErrorBadParameter;
    }

    if (nFrameBytes > (OMX_S32)pHeader->nAllocLen) {
        VTEST_MSG_ERROR("EncoderFileSource buffer too small");
        return OMX_ErrorBadParameter;
    }

    // allocate scratch buffer for secure sessions
    if (m_bSecureSession && m_pInCopyBuf == NULL) {
        m_nInCopyBufSize = pHeader->nAllocLen;
        m_pInCopyBuf = new OMX_U8[m_nInCopyBufSize];
        VTEST_MSG_HIGH("EncoderFileSource secure buf alloc %p (%u bytes)",
            m_pInCopyBuf, (unsigned int)m_nInCopyBufSize);
    }

    if (m_bSecureSession) {
        VTEST_MSG_LOW("select sec buf %p", m_pInCopyBuf);
        buffer = m_pInCopyBuf;
    } else {
        buffer = pHeader->pBuffer;
    }

    result = m_pFile->Read(buffer, m_nFrameWidth,
            m_nFrameHeight, &nBytesRead, 0);
    if (result != OMX_ErrorNone || nFrameBytes != nBytesRead) {
        VTEST_MSG_HIGH("YUV file too small. Seeking to start.");
        result = m_pFile->SeekStart(0);
        result = m_pFile->Read(buffer, m_nFrameWidth,
                m_nFrameHeight, &nBytesRead, 0);
    }
    VTEST_MSG_LOW("EncoderFileSource: read %u bytes", (unsigned int)nBytesRead);

    if ((m_bSecureSession) && (m_pCrypto != NULL)) {
        unsigned long nBufferFd = -1;
        if (m_bMetaMode) {
            struct ion_buffer_data *pIonBuffer =
                (struct ion_buffer_data *)pHeader->pAppPrivate;
            if (pIonBuffer) {
                nBufferFd = pIonBuffer->fd_data.fd;
            }
        } else {
            struct pmem *pPmem = (struct pmem *)pHeader->pInputPortPrivate;
            if (pPmem != NULL && pPmem->fd >= 0) {
                nBufferFd = pPmem->fd;
            }
        }
        result = m_pCrypto->Copy(SAMPLECLIENT_COPY_NONSECURE_TO_SECURE,
                m_pInCopyBuf, nBufferFd, m_nInCopyBufSize);
    }
    return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE EncoderFileSource::AllocateBuffer(
        BufferInfo *pBuffer, OMX_U32 ePortIndex, OMX_U32 nBufferSize) {

    OMX_ERRORTYPE result = OMX_ErrorNone;
    OMX_S32 nFds = 1;
    OMX_S32 nInts = 3;

    OMX_BUFFERHEADERTYPE **pHeaderPtr = pBuffer->GetHeaderPtr(ePortIndex);
    *pHeaderPtr = (OMX_BUFFERHEADERTYPE*)calloc(sizeof(OMX_BUFFERHEADERTYPE), 1);
    if (*pHeaderPtr == NULL) {
        VTEST_MSG_ERROR("Error allocating pHeader");
        return OMX_ErrorInsufficientResources;
    }

    if (vt_ion_allocate(m_bSecureSession,
                &((*pHeaderPtr)->pAppPrivate), nBufferSize) != 0) {
        VTEST_MSG_ERROR("Error allocating ion buffer of size : %u",
                (unsigned int)nBufferSize);
        free(*pHeaderPtr);
        *pHeaderPtr = NULL;
        return OMX_ErrorInsufficientResources;
    }

    struct ion_buffer_data *pIonBuffer =
        (struct ion_buffer_data *)(*pHeaderPtr)->pAppPrivate;
    (*pHeaderPtr)->nAllocLen = nBufferSize;
    (*pHeaderPtr)->pBuffer = (OMX_U8 *)pIonBuffer->data;

    BufferHandle pMetaHandle = (BufferHandle)calloc((
                sizeof(struct NativeHandle)+ sizeof(OMX_S32)*(nFds + nInts)), 1);
    if (pMetaHandle == NULL) {
        VTEST_MSG_ERROR("Error allocating MetaHandle");
        vt_ion_free((*pHeaderPtr)->pAppPrivate);
        free(*pHeaderPtr);
        *pHeaderPtr = NULL;
        return OMX_ErrorInsufficientResources;
    }

    pMetaHandle->version = sizeof(NativeHandle);
    pMetaHandle->numFds = nFds;
    pMetaHandle->numInts = nInts;
    pMetaHandle->data[0] = pIonBuffer->fd_data.fd;
    pMetaHandle->data[1] = 0; //offset
    pMetaHandle->data[2] = nBufferSize;
    pMetaHandle->data[3] = m_eYuvColorSpace;
    pBuffer->pHandle = (unsigned long)pMetaHandle;
    VTEST_MSG_MEDIUM("Allocated buffer of size : %u, fd : %u, data : %p",
            (unsigned int)nBufferSize, (unsigned int)pIonBuffer->fd_data.fd,
            pIonBuffer->data);
    return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE EncoderFileSource::FreeBuffer(
        BufferInfo *pBuffer, OMX_U32 ePortIndex) {

    (void)ePortIndex;
    OMX_ERRORTYPE result = OMX_ErrorNone;

    VTEST_MSG_MEDIUM("FreeBuffer: handle : %p pHeader: (%p %p)",
            (void *)pBuffer->pHandle, pBuffer->pHeaderIn, pBuffer->pHeaderOut);

    if (pBuffer->pHandle == 0) {
        return result;
    }

    BufferHandle pMetaHandle = (BufferHandle)pBuffer->pHandle;
    free(pMetaHandle);
    pMetaHandle = NULL;

    OMX_BUFFERHEADERTYPE **pHeaderPtr = pBuffer->GetHeaderPtr(ePortIndex);
    vt_ion_free((*pHeaderPtr)->pAppPrivate);
    free(*pHeaderPtr);
    *pHeaderPtr = NULL;
    FAILED(result, "Error freeing %p", (void *)pBuffer->pHandle);
    return result;
}


} // namespace vtest
