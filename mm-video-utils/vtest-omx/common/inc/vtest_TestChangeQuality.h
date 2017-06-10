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

#ifndef _VTEST_CHANGE_QUALITY_H
#define _VTEST_CHANGE_QUALITY_H

#include "OMX_Core.h"
#include "vtest_ITestCase.h"

namespace vtest {
class EncoderFileSource;       // forward declaration
class EncoderFileSink;         // forward declaration
class Encoder;          // forward declaration
class Mutex;            // forward declaration

/**
 * @brief Test case for dynamic quality configuration
 */
class TestChangeQuality : public ITestCase {
public:

    /**
     * @brief Constructor
     */
    TestChangeQuality();

    /**
     * @brief Destructor
     */
    virtual ~TestChangeQuality();

private:

    virtual OMX_ERRORTYPE Execute(CodecConfigType *pConfig,
                                  DynamicConfigType *pDynamicConfig,
                                  OMX_S32 nTestNum);

    virtual OMX_ERRORTYPE ValidateAssumptions(CodecConfigType *pConfig,
                                              DynamicConfigType *pDynamicConfig);

    static void SourceDeliveryFn(OMX_BUFFERHEADERTYPE *pBuffer);

    static void SinkReleaseFn(OMX_BUFFERHEADERTYPE *pBuffer);

    static OMX_ERRORTYPE EBD(OMX_IN OMX_HANDLETYPE hComponent,
                             OMX_IN OMX_PTR pAppData,
                             OMX_IN OMX_BUFFERHEADERTYPE *pBuffer);

    static OMX_ERRORTYPE FBD(OMX_IN OMX_HANDLETYPE hComponent,
                             OMX_IN OMX_PTR pAppData,
                             OMX_IN OMX_BUFFERHEADERTYPE *pBuffer);

private:
    EncoderFileSource *m_pSource;
    EncoderFileSink *m_pSink;
    Encoder *m_pEncoder;
    Mutex *m_pMutex;
    OMX_S32 m_nFramesCoded;
    OMX_S32 m_nFramesDelivered;
    OMX_S64 m_nBits;
};

} // namespace vtest

#endif // #ifndef _VTEST_CHANGE_QUALITY_H
