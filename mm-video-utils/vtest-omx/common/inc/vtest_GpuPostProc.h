/*-------------------------------------------------------------------
Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential
--------------------------------------------------------------------*/

#ifndef _VTEST_GPU_POSTPROC_H
#define _VTEST_GPU_POSTPROC_H

#include "vtest_IPostProc.h"
#include "vtest_Debug.h"
#include "vtest_ComDef.h"
#include <utils/RefBase.h>
#include <SurfaceComposerClient.h>
#include <Surface.h>
#include <ISurfaceComposer.h>
#include <DisplayInfo.h>
#include "gpupostprocessing.h"

namespace vtest {

typedef gppResult(*GpuPostProc_Init)(
        gpuPostProcessingHandle **pGppHandle, bool secureSession);
typedef gppResult(*GpuPostProc_Terminate)(gpuPostProcessingHandle *pGppHandle);
typedef gppResult(*GpuPostProc_Perform)(gpuPostProcessingHandle *pGppHandle,
        ANativeWindowBuffer *pInputNativeBuffer, ANativeWindowBuffer *pOutputNativeBuffer);

class GpuPostProc : virtual public IPostProc {

public:
    GpuPostProc();
    ~GpuPostProc();

    virtual OMX_ERRORTYPE Init(PostProcSession *pSession);
    virtual void Terminate();
    virtual OMX_ERRORTYPE Perform(BufferInfo *pBufferIn, BufferInfo *pBufferOut);

private:
    OMX_ERRORTYPE LoadGpuPostProcLib();
    void UnloadGpuPostProcLib();

private:
    GpuPostProc_Init m_fGpuPostProcInit;
    GpuPostProc_Terminate m_fGpuPostProcTerminate;
    GpuPostProc_Perform m_fGpuPostProcPerform;
};

} // namespace vtest

#endif // #ifndef _VTEST_GPU_POSTPROC_H
