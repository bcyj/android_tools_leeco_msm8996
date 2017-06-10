/*==============================================================================
*       WFDDDPWrapper.cpp
*
*  DESCRIPTION:
*       WFDDDPWrapper Functions
*
*
*  Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
*  Qualcomm Technologies Proprietary and Confidential.
*===============================================================================
*/
/*==============================================================================
                             Edit History
================================================================================
   When            Who           Why
-----------------  ------------  -----------------------------------------------
05/22/2014         SK            InitialDraft
================================================================================
*/

/*==============================================================================
**               Includes and Public Data Declarations
**==============================================================================
*/

/* =============================================================================

                     INCLUDE FILES FOR MODULE

================================================================================
*/
#include "WFDDDPWrapper.h"
#include "WFDMMLogs.h"
#ifdef DOLBY_ENCODER_ENABLE
#include "WFDMMAC3Encode.h"
#endif
#include "MMMemory.h"

#ifdef __cplusplus
extern "C" {
#endif
#define WFD_AC3WRAPPER_CREATE_FN      "wfdAC3WrapperCreate"
#define WFD_AC3WRAPPER_CONFIGURE_FN   "wfdAC3WrapperConfigure"
#define WFD_AC3WRAPPER_GETFRAMELEN_FN "wfdAC3WrapperGetFrameLen"
#define WFD_AC3WRAPPER_ENCODE_FN      "wfdAC3WrapperEncode"
#define WFD_AC3WRAPPER_DESTROY_FN     "wfdAC3WrapperDestroy"
#define UNUSED(x) ((void)x)

void* wfdAC3WrapperCreate()
{
#ifdef DOLBY_ENCODER_ENABLE
    WFDMMAC3Encode *pAC3 = MM_New (WFDMMAC3Encode);

    return (void*)pAC3;
#else
    WFDMMLOGE("Dolby resources not available");
    return NULL;
#endif
}

OMX_ERRORTYPE wfdAC3WrapperConfigure(void *hHandle,
                                     audioConfigType *pCfg)
{
    UNUSED(hHandle);
    UNUSED(pCfg);
#ifdef DOLBY_ENCODER_ENABLE
    if(!hHandle)
    {
        return OMX_ErrorBadParameter;
    }
    WFDMMAC3Encode *pAC3 = (WFDMMAC3Encode*)hHandle;

    return pAC3->Configure(pCfg);
#else
    WFDMMLOGE("Dolby resources not available");
    return OMX_ErrorInsufficientResources;
#endif
}


uint32 wfdAC3WrapperGetFrameLen(void *hHandle)
{
    UNUSED(hHandle);
#ifdef DOLBY_ENCODER_ENABLE
    if(!hHandle)
    {
        return OMX_ErrorBadParameter;
    }
    WFDMMAC3Encode *pAC3 = (WFDMMAC3Encode*)hHandle;

    return pAC3->GetInputFrameSize();
#else
    WFDMMLOGE("Dolby resources not available");
    return OMX_ErrorInsufficientResources;
#endif
}

OMX_ERRORTYPE wfdAC3WrapperEncode(void  *hHandle,
                                                   uint8* pInBuffer,
                                                   uint32 nInBufSize,
                                                   uint8* pOutBuffer,
                                                   uint32 nOutBufSize,
                                                   uint32 *pEncodedLen)
{
    UNUSED(hHandle);
    UNUSED(pInBuffer);
    UNUSED(nInBufSize);
    UNUSED(pOutBuffer);
    UNUSED(nOutBufSize);
    UNUSED(pEncodedLen);
#ifdef DOLBY_ENCODER_ENABLE
    if(!hHandle)
    {
        return OMX_ErrorBadParameter;
    }
    WFDMMAC3Encode *pAC3 = (WFDMMAC3Encode*)hHandle;

    return pAC3->Encode(pInBuffer,nInBufSize,pOutBuffer,
                        nOutBufSize,pEncodedLen);
#else
    WFDMMLOGE("Dolby resources not available");
    return OMX_ErrorInsufficientResources;
#endif
}

OMX_ERRORTYPE wfdAC3WrapperDestroy(void *hHandle)
{
    UNUSED(hHandle);
#ifdef DOLBY_ENCODER_ENABLE
    if(!hHandle)
    {
        return OMX_ErrorBadParameter;
    }
    WFDMMAC3Encode *pAC3 = (WFDMMAC3Encode*)hHandle;

    delete(pAC3);
    return OMX_ErrorNone;
#else
    WFDMMLOGE("Dolby resources not available");
    return OMX_ErrorInsufficientResources;
#endif
}
#ifdef __cplusplus
}
#endif


