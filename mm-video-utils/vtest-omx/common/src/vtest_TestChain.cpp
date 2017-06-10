/*-------------------------------------------------------------------
Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential
--------------------------------------------------------------------*/

#include "vtest_ComDef.h"
#include "vtest_Debug.h"
#include "vtest_Time.h"
#include "vtest_TestEncode.h"
#include "vtest_BufferManager.h"


namespace vtest {

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
TestChain::TestChain()
    : ITestCase(),    // invoke the base class constructor
      m_pBufferManager(NULL),
      m_pCrypto(NULL)  {

}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
TestChain::~TestChain() {
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE TestChain::ValidateAssumptions(CodecConfigType *pConfig,
                                              DynamicConfigType *pDynamicConfig) {
    OMX_ERRORTYPE result = OMX_ErrorNone;
    return result;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
OMX_ERRORTYPE TestChain::Execute(CodecConfigType *pConfig,
                                  DynamicConfigType *pDynamicConfig,
                                  OMX_S32 nTestNum) {
    return OMX_ErrorNone;
}

} // namespace vtest
