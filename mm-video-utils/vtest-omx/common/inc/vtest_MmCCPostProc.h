/*-------------------------------------------------------------------
Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential
--------------------------------------------------------------------*/

#ifndef _VTEST_MMCC_POSTPROC_H
#define _VTEST_MMCC_POSTPROC_H

#include "vtest_IPostProc.h"
#include "vtest_Debug.h"
#include "vtest_ComDef.h"
#include "MMColorConvert.h"

namespace vtest {

typedef convertFn MmCC_Convert;

class MmCCPostProc : virtual public IPostProc {

public:
    MmCCPostProc();
    ~MmCCPostProc();

    virtual OMX_ERRORTYPE Init(PostProcSession *pSession);
    virtual void Terminate();
    virtual OMX_ERRORTYPE Perform(BufferInfo *pBufferIn, BufferInfo *pBufferOut);

private:
    OMX_ERRORTYPE LoadMmCCLib();
    void UnloadMmCCLib();
    ColorConvertFormat GetMmFormat(OMX_U32 nFormat);

private:
    MmCC_Convert m_fMmCCConvert;
};

} // namespace vtest

#endif // #ifndef _VTEST_MMCC_POSTPROC_H
