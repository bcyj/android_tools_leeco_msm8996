/*-------------------------------------------------------------------
Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential
--------------------------------------------------------------------*/

#ifndef _VTEST_CASE_FACTORY_H
#define _VTEST_CASE_FACTORY_H

#include "OMX_Core.h"
#include "vtest_ComDef.h"

namespace vtest {

class ITestCase;

/**
 * @brief A test case class factory. All test cases created/destroyed through this class.
 */
class TestCaseFactory {

public:
    /**
     * @brief Allocates a test case object
     */
    static ITestCase* AllocTest(OMX_STRING pTestName);

    /**
     * @brief Destroys a test case object
     */
    static OMX_ERRORTYPE DestroyTest(ITestCase *pTest);

private:
    TestCaseFactory() { }
    ~TestCaseFactory() { }
};

} // namespace vtest

#endif // #ifndef  _VTEST_CASE_FACTORY_H
