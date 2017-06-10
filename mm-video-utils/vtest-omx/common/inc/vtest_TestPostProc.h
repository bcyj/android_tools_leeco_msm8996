/*-------------------------------------------------------------------
Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential
--------------------------------------------------------------------*/

#ifndef _VTEST_TEST_POSTPROC_H
#define _VTEST_TEST_POSTPROC_H

#include <vector>
#include "OMX_Core.h"
#include "vtest_ITestCase.h"
#include "vtest_BufferManager.h"
#include "vtest_Crypto.h"
#include "vtest_ISource.h"

namespace vtest {

/**
 * @brief Test case for bitrate and frame rate analysis
 */
class TestPostProc : public ITestCase {

public:
    /**
     * @brief Constructor
     */
    TestPostProc();

    /**
     * @brief Destructor
     */
    virtual ~TestPostProc();

    virtual OMX_ERRORTYPE Finish();

private:
    virtual OMX_ERRORTYPE Execute(CodecConfigType *pConfig,
            DynamicConfigType *pDynamicConfig, OMX_S32 nTestNum);

    virtual OMX_ERRORTYPE ValidateAssumptions(CodecConfigType *pConfig,
            DynamicConfigType *pDynamicConfig);

private:
    BufferManager *m_pBufferManager;
    Crypto *m_pCrypto;
    vector<ISource*> m_pSources;
};

} // namespace vtest

#endif // #ifndef _VTEST_TEST_POSTPROC_H
