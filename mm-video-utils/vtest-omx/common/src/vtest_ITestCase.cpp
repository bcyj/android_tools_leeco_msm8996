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

#include "vtest_ITestCase.h"
#include "vtest_Thread.h"
#include "vtest_Debug.h"
#include "vtest_Config.h"
#include "vtest_Parser.h"

namespace vtest {

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
ITestCase::ITestCase()
    : m_pThread(new Thread()),
      m_nTestNum(0) {}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
ITestCase::~ITestCase() {

    if (m_pThread) {
        delete m_pThread;
    }
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE ITestCase::Start(
        OMX_STRING pConfigFileName, OMX_S32 nTestNum) {

    OMX_ERRORTYPE result = OMX_ErrorNone;

    if (!m_pThread) {
        return OMX_ErrorInsufficientResources;
    }
    Config config;
    config.GetCodecConfig(&m_sConfig);
    config.GetDynamicConfig(&m_sDynamicConfig);

    m_nTestNum = nTestNum;
    result = config.Parse(pConfigFileName, &m_sConfig, &m_sDynamicConfig);
    FAILED1(result, "Error parsing config file");

    result = ValidateAssumptions(&m_sConfig, &m_sDynamicConfig);
    FAILED1(result, "Invalid config. Assumptions not validated");

    VTEST_MSG_MEDIUM("Starting test thread...");
    result = m_pThread->Start(ThreadEntry, this, 0);
    return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE ITestCase::Finish() {

    OMX_ERRORTYPE result = OMX_ErrorNone;

    if (m_pThread) {

        VTEST_MSG_MEDIUM("waiting for testcase thread to finish...");

        // wait for thread to exit
        m_pThread->Join(&result);
        FAILED(result, "test case thread execution error");
    }
    return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE ITestCase::ThreadEntry(OMX_PTR pThreadData) {

    OMX_ERRORTYPE result = OMX_ErrorNone;

    ITestCase *pTest = (ITestCase *)pThreadData;
    result = pTest->Execute(&pTest->m_sConfig, &pTest->m_sDynamicConfig,
                            pTest->m_nTestNum);
    return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE ITestCase::ValidateAssumptions(CodecConfigType *pConfig,
        DynamicConfigType *pDynamicConfig) {

    (void)pConfig; (void)pDynamicConfig;
    OMX_ERRORTYPE result = OMX_ErrorNone;
    return result;
}

} // namespace vtest
