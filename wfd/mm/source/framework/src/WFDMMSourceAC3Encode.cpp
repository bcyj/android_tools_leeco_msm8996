/*==============================================================================
*       WFDMMSourceAC3Encode.cpp
*
*  DESCRIPTION:
*       AC3 encode wrapper class
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
04/02/2014         SK            InitialDraft
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

#include "WFDMMLogs.h"
#include "WFDMMSourceAC3Encode.h"
#include "OMX_Core.h"
#include "dlfcn.h"


/*==============================================================================

         FUNCTION:         WFDMMSourceAC3Encode

         DESCRIPTION:
*//**       @brief         WFDMMSourceAC3Encode contructor
*//**
@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         None

*//*     RETURN VALUE:
*//**       @return
                           None
@par     SIDE EFFECTS:
                           None
*//*==========================================================================*/
WFDMMSourceAC3Encode::WFDMMSourceAC3Encode()
{
    mhAC3Handle       = NULL;
    pFnCreate         = NULL;
    pFnConfigure      = NULL;
    pFnGetFrameLen    = NULL;
    pFnEncode         = NULL;
    pFnDestroy        = NULL;
    /*--------------------------------------------------------------------------
      LOAD AC3 wrapper library and functions
    ----------------------------------------------------------------------------
    */
    mhLibHandle = dlopen("libwfdac3wrapper.so", RTLD_NOW);
    if(!mhLibHandle)
    {
        WFDMMLOGE("AC3 Wrapper Library Not Present");
        return;
    }

    pFnCreate         = (wfdAC3WrapperCreateFnType)
                            dlsym(mhLibHandle, WFD_AC3WRAPPER_CREATE_FN);
    pFnConfigure      = (wfdAC3WrapperConfigureFnType)
                            dlsym(mhLibHandle, WFD_AC3WRAPPER_CONFIGURE_FN);
    pFnGetFrameLen    = (wfdAC3WrapperGetFrameLenFnType)
                            dlsym(mhLibHandle, WFD_AC3WRAPPER_GETFRAMELEN_FN);
    pFnEncode         = (wfdAC3WrapperEncodeFnType)
                            dlsym(mhLibHandle, WFD_AC3WRAPPER_ENCODE_FN);
    pFnDestroy        = (wfdAC3WrapperDestroyFnType)
                            dlsym(mhLibHandle, WFD_AC3WRAPPER_DESTROY_FN);

    if(!pFnCreate || !pFnConfigure || !pFnGetFrameLen || !pFnEncode
       || !pFnDestroy)
    {
        WFDMMLOGE("Failed to Load one or more AC3 Wrapper Functions");
        return;
    }

    mhAC3Handle = pFnCreate();

    if(!mhAC3Handle)
    {
        WFDMMLOGE("Failed to Create AC3 wrapper Instance");
    }
}

/*==============================================================================

         FUNCTION:          ~WFDMMSourceAC3Encode

         DESCRIPTION:
*//**       @brief          AC3 decoder destructor
*//**
@par     DEPENDENCIES:
                            None
*//*
         PARAMETERS:
*//**       @param          None

*//*     RETURN VALUE:
*//**       @return
                            None
@par     SIFE EFFECTS:
                            None
*//*==========================================================================*/
WFDMMSourceAC3Encode::~WFDMMSourceAC3Encode()
{
    if(mhAC3Handle && pFnDestroy)
    {
        pFnDestroy(mhAC3Handle);
    }

    if(mhLibHandle)
    {
        dlclose(mhLibHandle);
    }
    WFDMMLOGH("WFDMMSourceAC3Encode Destructor");
}

/*==============================================================================

         FUNCTION:         Configure

         DESCRIPTION:
*//**       @brief         Configures the renderer
*//**
@par     DEPENDENCIES:
                           None
*//*
         PARAMETERS:
*//**       @param         pCfg        - Configuration Parameters

*//*     RETURN VALUE:
*//**       @return
                           OMX_ERRORTYPE
@par     SIDE EFFECTS:
                           None
*//*==========================================================================*/
OMX_ERRORTYPE WFDMMSourceAC3Encode::Configure(audioConfigType *pCfg)
{
    if(mhAC3Handle && pFnConfigure)
    {
        return pFnConfigure(mhAC3Handle,
                            pCfg);
    }
    return OMX_ErrorInsufficientResources;
}


/*==============================================================================

         FUNCTION:          Encode

         DESCRIPTION:
*//**       @brief          Encodes a frame
*//**
@par     DEPENDENCIES:
                            None
*//*
         PARAMETERS:
*//**       @param          None

*//*     RETURN VALUE:
*//**       @return
                            None
@par     SIFE EFFECTS:
                            None
*//*==========================================================================*/
OMX_ERRORTYPE WFDMMSourceAC3Encode::Encode(uint8* pInBuffer,
                                           uint32 nInBufSize,
                                           uint8* pOutBuffer,
                                           uint32 nOutBufSize,
                                           uint32 *pEncodedLen)
{
    if(mhAC3Handle && pFnEncode)
    {
        return pFnEncode(mhAC3Handle,
                         pInBuffer,
                         nInBufSize,
                         pOutBuffer,
                         nOutBufSize,
                         pEncodedLen);
    }
    return OMX_ErrorInsufficientResources;
}

/*==============================================================================

         FUNCTION:          GetInputFrameSize

         DESCRIPTION:
*//**       @brief          returns recommended frame length
*//**
@par     DEPENDENCIES:
                            None
*//*
         PARAMETERS:
*//**       @param          None

*//*     RETURN VALUE:
*//**       @return
                            None
@par     SIFE EFFECTS:
                            None
*//*==========================================================================*/
uint32 WFDMMSourceAC3Encode::GetInputFrameSize()
{
    if(mhAC3Handle && pFnGetFrameLen)
    {
        return pFnGetFrameLen(mhAC3Handle);
    }
    return 0;
}

