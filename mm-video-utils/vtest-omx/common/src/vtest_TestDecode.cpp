/*-------------------------------------------------------------------
Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential
--------------------------------------------------------------------*/

#include "vtest_ComDef.h"
#include "vtest_Debug.h"
#include "vtest_TestDecode.h"
#include "vtest_Time.h"
#include "vtest_Decoder.h"
#include "vtest_DecoderFileSource.h"
#include "vtest_DecoderFileSink.h"
#include "vtest_NativeWindow.h"
#include "vtest_MdpOverlaySink.h"
#include "vtest_BufferManager.h"

#undef LOG_TAG
#define LOG_TAG "VTEST_TEST_DECODE"

namespace vtest {

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
TestDecode::TestDecode()
    : ITestCase(),
      m_pSource(NULL),
      m_pSink(NULL),
      m_pDecoder(NULL),
      m_pBufferManager(NULL),
      m_pCrypto(NULL) {}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
TestDecode::~TestDecode() {}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE TestDecode::Finish() {

    OMX_ERRORTYPE result = ITestCase::Finish();

    VTEST_MSG_HIGH("Deleting all objects");
    if (m_pSink) {
        delete m_pSink;
        m_pSink = NULL;
    }

    if (m_pDecoder) {
        delete m_pDecoder;
        m_pDecoder = NULL;
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
OMX_ERRORTYPE TestDecode::Execute(CodecConfigType *pConfig,
        DynamicConfigType *pDynamicConfig, OMX_S32 nTestNum) {

    (void)pDynamicConfig; (void)nTestNum;
    OMX_ERRORTYPE result = OMX_ErrorNone;

    OMX_TICKS nStartTime = 0;
    OMX_TICKS nEndTime;
    OMX_TICKS nRunTimeSec;
    OMX_TICKS nRunTimeMillis;

    m_pCrypto = new Crypto();
    m_pBufferManager = new BufferManager();
    m_pSource = new DecoderFileSource(m_pCrypto);
    m_pDecoder = new Decoder(pConfig);

    switch (pConfig->eSinkType) {
        case NativeWindow_Sink:
            m_pSink = new NativeWindow(
                    pConfig->bRotateDisplay, pConfig->nOutputColorFormat);
            break;
        case MDPOverlay_Sink:
            m_pSink = new MdpOverlaySink(pConfig->bRotateDisplay);
            break;
        case File_Sink:
            m_pSink = new DecoderFileSink(m_pCrypto);
            break;
        default:
            m_pSink = NULL;
            return OMX_ErrorBadParameter;
    }

    if (!m_pCrypto || !m_pBufferManager || !m_pSource || !m_pDecoder || !m_pSink) {
        return OMX_ErrorInsufficientResources;
    }

    if (pConfig->bSecureSession) {
        result = m_pCrypto->Init();
        FAILED1(result, "Crypto Init failed");
    }

    result = m_pSource->Configure(pConfig, m_pBufferManager, NULL, m_pDecoder);
    FAILED1(result, "Could not configure Source");

    result = m_pDecoder->Configure(pConfig, m_pBufferManager, m_pSource, m_pSink);
    FAILED1(result, "Could not configure Decoder");

    result = m_pSink->Configure(pConfig, m_pBufferManager, m_pDecoder, NULL);
    FAILED1(result, "Could not configure Sink");

    result = m_pBufferManager->SetupBufferPool(m_pSource, m_pDecoder);
    FAILED1(result, "Could not create buffer pool between Source and Decoder");

    result = m_pBufferManager->SetupBufferPool(m_pDecoder, m_pSink);
    if (result != OMX_ErrorNone) {
        m_pBufferManager->FreeBuffers(m_pDecoder, PORT_INDEX_IN);
        VTEST_MSG_ERROR("Could not create buffer pool between Decoder and Sink");
        return result;
    }

    nStartTime = Time::GetTimeMicrosec();
    OMX_S32 executionResult = 0;

    executionResult = (OMX_S32)m_pSink->Start();

    executionResult |= (OMX_S32)m_pSink->Finish();

    executionResult |= (OMX_S32)m_pSink->Stop();
    result = (OMX_ERRORTYPE)executionResult;

    m_pBufferManager->FreeBuffers(m_pDecoder, PORT_INDEX_IN);
    m_pBufferManager->FreeBuffers(m_pDecoder, PORT_INDEX_OUT);

    if (pConfig->bSecureSession) {
        m_pCrypto->Terminate();
    }

    FAILED1(result, "Test execution failed");

    //==========================================
    // Compute stats
    nEndTime = Time::GetTimeMicrosec();
    nRunTimeMillis = (nEndTime - nStartTime) / 1000;   // convert to millis
    nRunTimeSec = nRunTimeMillis / 1000;               // convert to seconds

    VTEST_MSG_PROFILE("Time = %d millis", (int)nRunTimeMillis);
    return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE TestDecode::ValidateAssumptions(CodecConfigType *pConfig,
        DynamicConfigType *pDynamicConfig) {

    (void)pDynamicConfig;
    /* Overwrite the input width and height with Adaptive width and height
     * if enabled.
     */
    if (pConfig->ePlaybackMode == AdaptiveSmoothStreaming) {
        pConfig->nFrameWidth = pConfig->nAdaptiveWidth;
        pConfig->nFrameHeight = pConfig->nAdaptiveHeight;
    }
    return OMX_ErrorNone;
}

} // namespace vtest
