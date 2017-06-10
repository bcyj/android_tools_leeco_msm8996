#ifndef __WFD_DDP_WRAPPER_H__
#define __WFD_DDP_WRAPPER_H__

/*==============================================================================
*       WFDDDPWrapper.h
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
06/09/2014         SK            InitialDraft
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

#include "WFDMMSourceAudioEncode.h"
#include "OMX_Core.h"

#define WFD_AC3WRAPPER_CREATE_FN      "wfdAC3WrapperCreate"
#define WFD_AC3WRAPPER_CONFIGURE_FN   "wfdAC3WrapperConfigure"
#define WFD_AC3WRAPPER_GETFRAMELEN_FN "wfdAC3WrapperGetFrameLen"
#define WFD_AC3WRAPPER_ENCODE_FN      "wfdAC3WrapperEncode"
#define WFD_AC3WRAPPER_DESTROY_FN     "wfdAC3WrapperDestroy"

typedef void*         (*wfdAC3WrapperCreateFnType)();
typedef OMX_ERRORTYPE (*wfdAC3WrapperConfigureFnType)(void *hHandle,
                                                      audioConfigType *pCfg);
typedef uint32        (*wfdAC3WrapperGetFrameLenFnType)(void *hHandle);
typedef OMX_ERRORTYPE (*wfdAC3WrapperEncodeFnType)(void  *hHandle,
                                                   uint8* pInBuffer,
                                                   uint32 nInBufSize,
                                                   uint8* pOutBuffer,
                                                   uint32 nOutBufSize,
                                                   uint32 *pEncodedLen);
typedef OMX_ERRORTYPE (*wfdAC3WrapperDestroyFnType)(void *hHandle);

#endif

