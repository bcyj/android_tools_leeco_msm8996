/*-------------------------------------------------------------------
Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential
--------------------------------------------------------------------*/

#ifndef _VTEST_TEST_CHAIN_H
#define _VTEST_TEST_CHAIN_H

#include "OMX_Core.h"
#include "vtest_ITestCase.h"
#include "vtest_BufferManager.h"
#include "vtest_Crypto.h"

namespace vtest {
class ISource;              // forward declaration
class BufferManager;        // forward declaration

/**
 * @brief Test case for bitrate and frame rate analysis
 */
class TestChain : public ITestCase {
public:

    /**
     * @brief Constructor
     */
    TestChain();

    /**
     * @brief Destructor
     */
    virtual ~TestChain();

private:

    virtual OMX_ERRORTYPE Execute(CodecConfigType *pConfig,
                                  DynamicConfigType *pDynamicConfig,
                                  OMX_S32 nTestNum);

    virtual OMX_ERRORTYPE ValidateAssumptions(CodecConfigType *pConfig,
                                              DynamicConfigType *pDynamicConfig);

private:
    ISource[] m_pSourceChain;
    BufferManager *m_pBufferManager;
    Crypto *m_pCrypto;
};

} // namespace vtest

#endif // #ifndef _VTEST_TEST_CHAIN_H
