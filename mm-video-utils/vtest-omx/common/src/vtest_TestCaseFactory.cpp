/*-------------------------------------------------------------------
Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential
--------------------------------------------------------------------*/

#include "vtest_TestCaseFactory.h"
#include "vtest_ITestCase.h"
#include "vtest_Thread.h"
#include "vtest_Debug.h"
#include "vtest_Parser.h"

#include "vtest_TestEncode.h"
#include "vtest_TestDecode.h"
#include "vtest_TestTranscode.h"
#include "vtest_TestPostProc.h"

namespace vtest {

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
ITestCase* TestCaseFactory::AllocTest(OMX_STRING pTestName) {

    ITestCase *pTest = NULL;

    if (Parser::StringICompare(pTestName, (OMX_STRING)"ENCODE") == 0) {
        pTest = new TestEncode;
    } else if (Parser::StringICompare(pTestName, (OMX_STRING)"DECODE") == 0) {
        pTest = new TestDecode;
    } else if (Parser::StringICompare(pTestName, (OMX_STRING)"TRANSCODE") == 0) {
        pTest = new TestTranscode;
    } else if (Parser::StringICompare(pTestName, (OMX_STRING)"POSTPROC") == 0) {
        pTest = new TestPostProc;
    } else {
        VTEST_MSG_ERROR("Invalid test name");
    }
    return pTest;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE TestCaseFactory::DestroyTest(ITestCase *pTest) {

    OMX_ERRORTYPE result = OMX_ErrorNone;

    if (pTest) {
        delete pTest;
    } else {
        VTEST_MSG_ERROR("NULL param");
        result = OMX_ErrorBadParameter;
    }
    return result;
}

} // namespace vtest
