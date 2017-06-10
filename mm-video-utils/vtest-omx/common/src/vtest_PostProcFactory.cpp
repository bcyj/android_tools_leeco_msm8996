/*-------------------------------------------------------------------
Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential
--------------------------------------------------------------------*/

#include "vtest_IPostProc.h"
#include "vtest_Debug.h"
#include "vtest_ComDef.h"

#ifdef GPU_PP_ENABLED
#include "vtest_GpuPostProc.h"
#endif
#ifdef C2DCC_PP_ENABLED
#include "vtest_C2dCCPostProc.h"
#endif
#ifdef MMCC_PP_ENABLED
#include "vtest_MmCCPostProc.h"
#endif

namespace vtest {

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
IPostProc *CreatePostProcModule(PostProcType ePostProcType) {

    IPostProc *pPostProcModule = NULL;

    switch (ePostProcType) {
#ifdef GPU_PP_ENABLED
        case GpuPostProcessing:
            pPostProcModule = new GpuPostProc();
            break;
#endif
#ifdef C2DCC_PP_ENABLED
        case C2dColorConversion:
            pPostProcModule = new C2dCCPostProc();
            break;
#endif
#ifdef MMCC_PP_ENABLED
        case MmColorConversion:
            pPostProcModule = new MmCCPostProc();
            break;
#endif
        case DefaultMemcopy:
            pPostProcModule = new IPostProc();
            break;
        default:
            VTEST_MSG_ERROR("Invalid PostProcType : %d", (int)ePostProcType);
    }
    return pPostProcModule;
}

} // namespace vtest
