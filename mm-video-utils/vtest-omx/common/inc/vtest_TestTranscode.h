/*-------------------------------------------------------------------
Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential
--------------------------------------------------------------------*/

#ifndef _VTEST_TEST_TRANSCODE_H
#define _VTEST_TEST_TRANSCODE_H

#include <vector>
#include "OMX_Core.h"
#include "vtest_Crypto.h"
#include "vtest_BufferManager.h"
#include "vtest_ITestCase.h"
#include "vtest_ISource.h"

using std::vector;

namespace vtest {

/**
 * @brief Test case for bitrate and frame rate analysis
 */
class TestTranscode : public ITestCase {

public:
    /**
     * @brief Constructor
     */
    TestTranscode();

    /**
     * @brief Destructor
     */
    virtual ~TestTranscode();

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

#endif // #ifndef _VTEST_TEST_TRANSCODE_H
