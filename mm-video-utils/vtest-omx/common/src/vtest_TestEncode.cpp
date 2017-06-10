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

#include "vtest_ComDef.h"
#include "vtest_Debug.h"
#include "vtest_Time.h"
#include "vtest_TestEncode.h"
#include "vtest_BufferManager.h"
#include "vtest_Encoder.h"
#include "vtest_EncoderFileSource.h"
#include "vtest_EncoderFileSink.h"
#include "vtest_MdpSource.h"

namespace vtest {

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
TestEncode::TestEncode()
    : ITestCase(),    // invoke the base class constructor
      m_pSource(NULL),
      m_pSink(NULL),
      m_pEncoder(NULL),
      m_pBufferManager(NULL),
      m_pCrypto(NULL)  {}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
TestEncode::~TestEncode() {}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE TestEncode::Finish() {

    OMX_ERRORTYPE result = ITestCase::Finish();

    VTEST_MSG_HIGH("Deleting all objects");
    if (m_pSink) {
        delete m_pSink;
        m_pSink = NULL;
    }

    if (m_pEncoder) {
        delete m_pEncoder;
        m_pEncoder = NULL;
    }

    if (m_pSource) {
        delete m_pSource;
        m_pSource = NULL;
    }

    if (m_pBufferManager) {
        delete m_pBufferManager;
        m_pBufferManager = NULL;
    }

    if (m_pCrypto) {
        delete m_pCrypto;
        m_pCrypto = NULL;
    }
    return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE TestEncode::Execute(CodecConfigType *pConfig,
        DynamicConfigType *pDynamicConfig, OMX_S32 nTestNum) {

    (void)pDynamicConfig; (void)nTestNum;
    OMX_ERRORTYPE result = OMX_ErrorNone;

    OMX_TICKS nStartTime = 0;
    OMX_TICKS nEndTime;
    OMX_TICKS nRunTimeMillis;

    m_pCrypto = new Crypto();
    m_pBufferManager = new BufferManager();

    if(!pConfig->bMdpFrameSource) {
        m_pSource = new EncoderFileSource(m_pCrypto);
    } else {
        m_pSource = new MdpSource(pConfig);
    }

    m_pEncoder = new Encoder(pConfig);
    m_pSink = new EncoderFileSink(m_pCrypto);

    if (!m_pCrypto || !m_pBufferManager || !m_pSource || !m_pEncoder || !m_pSink) {
        return OMX_ErrorInsufficientResources;
    }

    if (pConfig->bSecureSession) {
        result = m_pCrypto->Init();
        FAILED1(result, "Crypto Init failed");
    }

    result = m_pSource->Configure(pConfig, m_pBufferManager, NULL, m_pEncoder);
    FAILED1(result, "Could not configure Source");

    result = m_pEncoder->Configure(pConfig, m_pBufferManager, m_pSource, m_pSink);
    FAILED1(result, "Could not configure Encoder");

    result = m_pSink->Configure(pConfig, m_pBufferManager, m_pEncoder, NULL);
    FAILED1(result, "Could not configure Sink");

    result = m_pBufferManager->SetupBufferPool(m_pSource, m_pEncoder);
    FAILED1(result, "Could not create buffer pool between Source and Encoder");

    result = m_pBufferManager->SetupBufferPool(m_pEncoder, m_pSink);
    if (result != OMX_ErrorNone) {
        m_pBufferManager->FreeBuffers(m_pEncoder, PORT_INDEX_IN);
        VTEST_MSG_ERROR("Could not create buffer pool between Encoder and Sink");
        return result;
    }

    nStartTime = Time::GetTimeMicrosec();
    OMX_S32 executionResult = 0;

    executionResult = (OMX_S32)m_pSink->Start();

    executionResult |= (OMX_S32)m_pSink->Finish();

    executionResult |= (OMX_S32)m_pSink->Stop();
    result = (OMX_ERRORTYPE)executionResult;

    m_pBufferManager->FreeBuffers(m_pEncoder, PORT_INDEX_IN);
    m_pBufferManager->FreeBuffers(m_pEncoder, PORT_INDEX_OUT);

    if (pConfig->bSecureSession) {
        m_pCrypto->Terminate();
    }

    FAILED1(result, "Test execution failed");

    //==========================================
    // Compute stats
    nEndTime = Time::GetTimeMicrosec();
    nRunTimeMillis = (nEndTime - nStartTime) / 1000;   // convert to millis
    if (nRunTimeMillis == 0) {
        VTEST_MSG_ERROR("DIV_) warning, forcing to valid value");
        nRunTimeMillis = -1000000;
    }

    VTEST_MSG_PROFILE(
        "Time = %d millis, Encoded = %d, Dropped = %d",
        (int)nRunTimeMillis,
        (int)m_pSink->OutputBufferCount(),
        (int)(pConfig->nFrames - m_pSink->OutputBufferCount()));

    VTEST_MSG_PROFILE(
        "Bitrate = %d, InputFPS = %d, OutputFPS = %d",
        (int)((8 * m_pSink->OutputByteCount() * 1000)/ nRunTimeMillis),
        (int)((pConfig->nFrames * 1000) / nRunTimeMillis),
        (int)((m_pSink->OutputBufferCount() * 1000) / nRunTimeMillis));

    VTEST_MSG_PROFILE("Avg encode time = %d millis per frame",
                          (int)(nRunTimeMillis / pConfig->nFrames));

    if (pConfig->eControlRate != OMX_Video_ControlRateDisable &&
            pConfig->bProfileMode == OMX_TRUE) {
        static const double errorThreshold = .15; // error percentage threshold

        OMX_S32 nBitrateDelta = (OMX_S32)(pConfig->nBitrate -
                ((8 * m_pSink->OutputByteCount() * 1000) / nRunTimeMillis));
        if (nBitrateDelta < 0) {
            nBitrateDelta = -nBitrateDelta;
        }
        if ((double)nBitrateDelta > pConfig->nBitrate * errorThreshold) {
            FAILED1(OMX_ErrorUndefined,
                    "Test failed with bitrate %d. bitrate delta is %d max allowed is approx %d",
                    (int)pConfig->nBitrate, (int)nBitrateDelta,
                    (int)(pConfig->nBitrate * errorThreshold));
        }

        OMX_S32 nFramerateDelta =
            (OMX_S32)(pConfig->nFramerate - ((pConfig->nFrames * 1000) / nRunTimeMillis));
        if (nFramerateDelta < 0) {
            nFramerateDelta = -nFramerateDelta;
        }
        if ((double)nFramerateDelta > pConfig->nFramerate * errorThreshold) {
            FAILED1(OMX_ErrorUndefined,
                    "Test failed with frame rate %d. frame rate delta is %d max allowed is approx %d",
                    (int)pConfig->nFramerate, (int)nFramerateDelta,
                    (int)(((pConfig->nFrames * 1000) / nRunTimeMillis) * errorThreshold));
        }
    }
    return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE TestEncode::ValidateAssumptions(CodecConfigType *pConfig,
        DynamicConfigType *pDynamicConfig) {

    (void)pConfig; (void)pDynamicConfig;

    /* Encoder cannot be in MetaMode because the buffers need to be mapped in
     * the venus SMMU to get a hint assigned without which the secure copy will
     * fail. */
    if (pConfig->bSecureSession) {
        pConfig->bMetaMode = OMX_FALSE;
    }

    if (pConfig->bMetaMode) {
        pConfig->eMetaBufferType = CameraSource;
    }
    return OMX_ErrorNone;
}

} // namespace vtest
