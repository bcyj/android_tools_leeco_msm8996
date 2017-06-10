/*-------------------------------------------------------------------
Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
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
#include "vtest_TestChangeQuality.h"
#include "vtest_Time.h"
#include "vtest_EncoderFileSource.h"
#include "vtest_EncoderFileSink.h"
#include "vtest_Encoder.h"
#include "vtest_Sleeper.h"
#include "vtest_Mutex.h"

namespace vtest {

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
TestChangeQuality::TestChangeQuality()
    : ITestCase(),    // invoke the base class constructor
      m_pSource(NULL),
      m_pSink(NULL),
      m_pEncoder(NULL),
      m_pMutex(NULL),
      m_nFramesCoded(0),
      m_nFramesDelivered(0),
      m_nBits(0) {
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
TestChangeQuality::~TestChangeQuality() {
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE TestChangeQuality::ValidateAssumptions(CodecConfigType *pConfig,
                                                     DynamicConfigType *pDynamicConfig) {
    OMX_ERRORTYPE result = OMX_ErrorNone;

    if (pConfig->eControlRate == OMX_Video_ControlRateDisable) {
        VTEST_MSG_ERROR("need to enable rate control");
        result = CheckError(OMX_ErrorUndefined);
    }

    if (pDynamicConfig->nUpdatedFrames == 0) {
        VTEST_MSG_ERROR("need to indicate number of frames to encode after quality change",
                            0, 0, 0);
        result = CheckError(OMX_ErrorUndefined);
    }

    if (pDynamicConfig->nUpdatedFramerate == 0) {
        VTEST_MSG_ERROR("need to configure the second frame rate",
                            0, 0, 0);
        result = CheckError(OMX_ErrorUndefined);
    }

    if (pDynamicConfig->nUpdatedBitrate == 0) {
        VTEST_MSG_ERROR("need to configure the second bitrate",
                            0, 0, 0);
        result = CheckError(OMX_ErrorUndefined);
    }

    return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE TestChangeQuality::Execute(CodecConfigType *pConfig,
                                         DynamicConfigType *pDynamicConfig,
                                         OMX_S32 nTestNum) {
    OMX_ERRORTYPE result = OMX_ErrorNone;

    OMX_TICKS nStartTime;
    OMX_TICKS nEndTime;
    OMX_TICKS nRunTimeSec;
    OMX_TICKS nRunTimeMillis;
    OMX_S32 nFramesCoded;
    OMX_S32 nFramesDelivered;
    OMX_S64 nBits;

    //==========================================
    // Create mutex
    if (result == OMX_ErrorNone) {
        VTEST_MSG_HIGH("Creating mutex...");
        m_pMutex = new Mutex();
    }

    //==========================================
    // Create and configure the file source (yuv reader)
    if (result == OMX_ErrorNone) {
        VTEST_MSG_HIGH("Creating source...");
        m_pSource = new EncoderFileSource(m_pSemaphore, pConfig->bProfileMode, m_bIsVTPath);
        result = CheckError(m_pSource->Configure(pConfig->nFrames + pDynamicConfig->nUpdatedFrames,  // deliver only the initial set of frames
                                                 pConfig->nFramerate,
                                                 pConfig->nFrameWidth,
                                                 pConfig->nFrameHeight,
                                                 pConfig->nInBufferCount,
                                                 SourceDeliveryFn,
                                                 pConfig->cInFileName,
                                                 pConfig->nDVSXOffset,
                                                 pConfig->nDVSYOffset,
                                                 pConfig->bProfileMode)); // live mode
    }

    //==========================================
    // Create and configure the file sink (m4v writer)
    if (result == OMX_ErrorNone) {
        VTEST_MSG_HIGH("Creating sink...");
        m_pSink = new EncoderFileSink(m_pSemaphore, pConfig->bProfileMode, m_bIsVTPath);
        result = CheckError(m_pSink->Configure(
                                pConfig->nFrames + pDynamicConfig->nUpdatedFrames,    // process the initial frames
                                pConfig->cOutFileName,                                // and updated quality frames
                                nTestNum,
                                SinkReleaseFn));
    }

    //==========================================
    // Create and configure the encoder
    if (result == OMX_ErrorNone) {
        VTEST_MSG_HIGH("Creating encoder...");
        m_pEncoder = new Encoder(EBD,
                                 FBD,
                                 NULL,
                                 this, // set the test case object as the callback app data
                                 pConfig->eCodec,
                                 pConfig->bSecureSession);
        result = CheckError(m_pEncoder->Configure(pConfig));
    }

    //==========================================
    // Go to executing state (also allocate buffers)
    if (result == OMX_ErrorNone) {
        VTEST_MSG_HIGH("Go to executing state...");
        result = CheckError(m_pEncoder->GoToExecutingState());
    }

    //==========================================
    // Get the allocated input buffers
    if (result == OMX_ErrorNone) {
        OMX_BUFFERHEADERTYPE **ppInputBuffers;
        ppInputBuffers = m_pEncoder->GetBuffers(OMX_TRUE);
        for (int i = 0; i < pConfig->nInBufferCount; i++) {
            ppInputBuffers[i]->pAppPrivate = m_pEncoder; // set the encoder as the private app data
            result = CheckError(m_pSource->SetFreeBuffer(
                                    ppInputBuffers[i])); // give ownership to source
            if (result != OMX_ErrorNone) {
                break;
            }
        }
    }

    //==========================================
    // Get the allocated output buffers
    if (result == OMX_ErrorNone) {
        OMX_BUFFERHEADERTYPE **ppOutputBuffers;
        ppOutputBuffers = m_pEncoder->GetBuffers(OMX_FALSE);
        for (int i = 0; i < pConfig->nOutBufferCount; i++) {
            ppOutputBuffers[i]->pAppPrivate = m_pEncoder; // set the encoder as the private app data
            result = CheckError(m_pEncoder->DeliverOutput(
                                    ppOutputBuffers[i])); // give ownership to encoder
            if (result != OMX_ErrorNone) {
                break;
            }
        }
    }

    //==========================================
    // Get the sink ready to write m4v output
    if (result == OMX_ErrorNone) {
        VTEST_MSG_HIGH("starting the sink thread...");
        nStartTime = Time::GetTimeMicrosec();
        result = CheckError(m_pSink->Start());
    }

    //==========================================
    // Start reading and delivering frames
    if (result == OMX_ErrorNone) {
        VTEST_MSG_HIGH("starting the source thread...");
        result = CheckError(m_pSource->Start());
    }

    (void)Sleeper::Sleep(1000 * pConfig->nFrames / pConfig->nFramerate);

    if (result == OMX_ErrorNone) {
        m_pMutex->Lock();

        //==========================================
        // Compute stats
        nEndTime = Time::GetTimeMicrosec();
        nRunTimeMillis = (nEndTime - nStartTime) / 1000;   // convert to millis
        nRunTimeSec = nRunTimeMillis / 1000;               // convert to seconds

        nFramesCoded = m_nFramesCoded;
        nFramesDelivered = m_nFramesDelivered;
        nBits = m_nBits;
        nStartTime = nEndTime;

        // reset statistics
        m_nFramesCoded = 0;
        m_nFramesDelivered = 0;
        m_nBits = 0;

        m_pMutex->UnLock();

        VTEST_MSG_PROFILE("Time = %d millis, Encoded = %d, Dropped = %d",
                              (int)nRunTimeMillis,
                              (int)nFramesCoded,
                              (int)(nFramesDelivered - nFramesCoded));

        if (nRunTimeSec > 0) { // ensure no divide by zero
            VTEST_MSG_PROFILE("Bitrate = %d, InputFPS = %d, OutputFPS = %d",
                                  (int)(nBits / nRunTimeSec),
                                  (int)(nFramesDelivered / nRunTimeSec),
                                  (int)(nFramesCoded / nRunTimeSec));
        } else {
            VTEST_MSG_PROFILE("Bitrate = %d, InputFPS = %d, OutputFPS = %d",
                                  0, 0, 0);
        }

        VTEST_MSG_PROFILE("Avg encode time = %d millis per frame",
                              (int)(nRunTimeMillis / nFramesCoded));

    }

    //==========================================
    // Change the delivery frame rate
    if (result == OMX_ErrorNone) {
        result = CheckError(m_pSource->ChangeFrameRate(
                                pDynamicConfig->nUpdatedFramerate));

        if (result != OMX_ErrorNone) {
            VTEST_MSG_ERROR("failed to change frame rate");
        }
    }

    //==========================================
    // Change the encoder quality
    if (result == OMX_ErrorNone) {
        result = CheckError(m_pEncoder->ChangeQuality(
                                pDynamicConfig->nUpdatedFramerate, pDynamicConfig->nUpdatedBitrate));

        if (result != OMX_ErrorNone) {
            VTEST_MSG_ERROR("failed to change encoder quality");
        }
    }

    //==========================================
    // Wait for the source to finish delivering all frames
    if (m_pSource != NULL) {
        VTEST_MSG_HIGH("waiting for source to finish...");
        result = CheckError(m_pSource->Finish());
        VTEST_MSG_HIGH("source is finished");
    }

    //==========================================
    // Wait for the sink to finish writing all frames
    if (m_pSink != NULL) {
        VTEST_MSG_HIGH("waiting for sink to finish...");
        result = CheckError(m_pSink->Finish());
        VTEST_MSG_HIGH("sink is finished");
    }

    //==========================================
    // Tear down the encoder (also deallocate buffers)
    if (m_pEncoder != NULL) {
        VTEST_MSG_HIGH("Go to loaded state...");
        result = CheckError(m_pEncoder->GoToLoadedState());
    }

    if (result == OMX_ErrorNone) {
        m_pMutex->Lock();

        //==========================================
        // Compute stats
        nEndTime = Time::GetTimeMicrosec();
        nRunTimeMillis = (nEndTime - nStartTime) / 1000;   // convert to millis
        nRunTimeSec = nRunTimeMillis / 1000;               // convert to seconds

        nFramesCoded = m_nFramesCoded;
        nFramesDelivered = m_nFramesDelivered;
        nBits = m_nBits;

        m_pMutex->UnLock();

        VTEST_MSG_PROFILE("Time = %d millis, Encoded = %d, Dropped = %d",
                              (int)nRunTimeMillis,
                              (int)nFramesCoded,
                              (int)(nFramesDelivered - nFramesCoded));

        if (nRunTimeSec > 0) { // ensure no divide by zero
            VTEST_MSG_PROFILE("Bitrate = %d, InputFPS = %d, OutputFPS = %d",
                                  (int)(nBits / nRunTimeSec),
                                  (int)(nFramesDelivered / nRunTimeSec),
                                  (int)(nFramesCoded / nRunTimeSec));
        } else {
            VTEST_MSG_PROFILE("Bitrate = %d, InputFPS = %d, OutputFPS = %d",
                                  0, 0, 0);
        }

        VTEST_MSG_PROFILE("Avg encode time = %d millis per frame",
                              (int)(nRunTimeMillis / nFramesCoded));
    }

    //==========================================
    // Free our helper classes
    if (m_pMutex) delete m_pMutex;
    if (m_pSource) delete m_pSource;
    if (m_pSink) delete m_pSink;
    if (m_pEncoder) delete m_pEncoder;

    return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void TestChangeQuality::SourceDeliveryFn(OMX_BUFFERHEADERTYPE *pBuffer) {
    // Deliver YUV data from source to encoder
    ((Encoder *)pBuffer->pAppPrivate)->DeliverInput(pBuffer);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void TestChangeQuality::SinkReleaseFn(OMX_BUFFERHEADERTYPE *pBuffer) {
    // Deliver bitstream buffer from sink to encoder
    ((Encoder *)pBuffer->pAppPrivate)->DeliverOutput(pBuffer);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE TestChangeQuality::EBD(OMX_IN OMX_HANDLETYPE hComponent,
                                     OMX_IN OMX_PTR pAppData,
                                     OMX_IN OMX_BUFFERHEADERTYPE *pBuffer) {
    TestChangeQuality *pTester = (TestChangeQuality *)pAppData;

    (void)pTester->m_pMutex->Lock();
    ++pTester->m_nFramesDelivered;
    (void)pTester->m_pMutex->UnLock();

    // Deliver free yuv buffer to source
    return pTester->m_pSource->SetFreeBuffer(pBuffer);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE TestChangeQuality::FBD(OMX_IN OMX_HANDLETYPE hComponent,
                                     OMX_IN OMX_PTR pAppData,
                                     OMX_IN OMX_BUFFERHEADERTYPE *pBuffer) {

    TestChangeQuality *pTester = (TestChangeQuality *)pAppData;

    (void)pTester->m_pMutex->Lock();

    // get performance data
    if (pBuffer->nFilledLen != 0) {
        // if it's only the syntax header don't count it as a frame
        if ((pBuffer->nFlags & OMX_BUFFERFLAG_CODECCONFIG) == 0) {
            ++pTester->m_nFramesCoded;
        }

        // always count the bits regarding whether or not its only syntax header
        pTester->m_nBits = pTester->m_nBits + (OMX_S32)(pBuffer->nFilledLen * 8);
    }

    (void)pTester->m_pMutex->UnLock();

    // Deliver encoded m4v output to sink for file write
    return pTester->m_pSink->Write(pBuffer);
}

} // namespace vtest
