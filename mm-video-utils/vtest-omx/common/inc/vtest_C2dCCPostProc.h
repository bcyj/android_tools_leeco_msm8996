/*-------------------------------------------------------------------
Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential
--------------------------------------------------------------------*/

#ifndef _VTEST_C2DCC_POSTPROC_H
#define _VTEST_C2DCC_POSTPROC_H

#include "vtest_IPostProc.h"
#include "vtest_Debug.h"
#include "vtest_ComDef.h"
#include "C2DColorConverter.h"

namespace vtest {

typedef android::createC2DColorConverter_t* C2dCC_Open;
typedef android::destroyC2DColorConverter_t* C2dCC_Close;

class C2dCCPostProc : virtual public IPostProc {

public:
    C2dCCPostProc();
    ~C2dCCPostProc();

    virtual OMX_ERRORTYPE Init(PostProcSession *pSession);
    virtual void Terminate();
    virtual OMX_ERRORTYPE Perform(BufferInfo *pBufferIn, BufferInfo *pBufferOut);
    virtual void GetBufferRequirements(OMX_U32 ePortIndex, OMX_U32 *nBufferSize);

private:
    OMX_ERRORTYPE LoadC2dCCLib();
    void UnloadC2dCCLib();
    android::ColorConvertFormat GetC2DFormat(OMX_U32 nFormat);

private:
    C2dCC_Open m_fC2dCCOpen;
    C2dCC_Close m_fC2dCCClose;
};

} // namespace vtest

#endif // #ifndef _VTEST_C2DCC_POSTPROC_H
