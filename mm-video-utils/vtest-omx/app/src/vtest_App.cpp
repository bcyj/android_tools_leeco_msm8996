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

#include <stdlib.h>
#include "vtest_Script.h"
#include "vtest_Debug.h"
#include "vtest_ComDef.h"
#include "vtest_ITestCase.h"
#include "vtest_TestCaseFactory.h"

void RunTest(OMX_STRING pTestName, OMX_STRING pConfigFile,
        OMX_S32 nSession, OMX_S32 *nPass, OMX_S32 *nFail) {

    OMX_ERRORTYPE result = OMX_ErrorNone;

    if (!nPass || !nFail) {
        return;
    }

    for (OMX_S32 i = 0; i < nSession; i++) {

        vtest::ITestCase *pTest =
            vtest::TestCaseFactory::AllocTest(pTestName);
        if (pTest == NULL) {
            VTEST_MSG_CONSOLE("Unable to alloc test %d : %s", (int)i + 1, pTestName);
            break;
        }

        VTEST_MSG_CONSOLE("Running test %d", (int)i + 1);
        result = pTest->Start(pConfigFile, i);
        if (result != OMX_ErrorNone) {
            VTEST_MSG_CONSOLE("Error starting test");
            ++*nFail;
        } else {
            result = pTest->Finish();
            if (result != OMX_ErrorNone) {
                VTEST_MSG_CONSOLE("Test %d failed", (int)i + 1);
                ++*nFail;
            } else {
                ++*nPass;
            }
        }
        vtest::TestCaseFactory::DestroyTest(pTest);
    }
}

void RunScript(OMX_STRING pScriptFile, OMX_S32 *nPass, OMX_S32 *nFail) {

    OMX_ERRORTYPE result = OMX_ErrorNone;
    vtest::Script script;
    script.Configure(pScriptFile);
    vtest::TestDescriptionType testDescription;

    while (1) {
        result = script.NextTest(&testDescription);
        if (result == OMX_ErrorNoMore) {
            break;
        }
        if (result != OMX_ErrorNone) {
            VTEST_MSG_CONSOLE("Error parsing vtest script");
            break;
        }
        RunTest((OMX_STRING)testDescription.cTestName,
                testDescription.cConfigFile, testDescription.nSession, nPass, nFail);
    }
}

int main(int argc, char *argv[]) {

    OMX_Init();
    OMX_S32 nPass = 0;
    OMX_S32 nFail = 0;

    if (argc == 2) {
        OMX_STRING pScriptFile = (OMX_STRING)argv[1];
        RunScript(pScriptFile, &nPass, &nFail);
    } else if (argc == 4) {
        OMX_STRING pTestName = (OMX_STRING)argv[1];
        OMX_STRING pConfigFile = (OMX_STRING)argv[2];
        OMX_STRING pNumSession = (OMX_STRING)argv[3];
        OMX_S32 nSession = atoi((char *)pNumSession);
        RunTest(pTestName, pConfigFile, nSession, &nPass, &nFail);
    } else {
        VTEST_MSG_CONSOLE("invalid number of command args %d", argc);
        VTEST_MSG_CONSOLE("./mm-vidc-omx-test ENCODE/DECODE/TRANSCODE Config.cfg 1");
    }

    VTEST_MSG_CONSOLE("passed %d out of %d tests", (int)nPass, (int)(nPass + nFail));
    VTEST_MSG_CONSOLE("/*******************TestApp Exiting*******************/");
    OMX_Deinit();
}
