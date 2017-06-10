#ifndef __WFD_MM_SOURCE_AC3_ENCODE_H__
#define __WFD_MM_SOURCE_AC3_ENCODE_H__

/*==============================================================================
*       WFDMMSourceAC3Encode.h
*
*  DESCRIPTION:
*       Class declaration WFDMMSourceAC3Encode.
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

#include "WFDMMSourceAudioEncode.h"
#include "WFDDDPWrapper.h"

//! WFDMMSourceAC3Encode class
//! Abstract interface for complete frame audio encode
class WFDMMSourceAC3Encode:public WFDMMSourceAudioEncode
{
public:
    WFDMMSourceAC3Encode();
    virtual ~WFDMMSourceAC3Encode();
    virtual OMX_ERRORTYPE  Configure(audioConfigType *pCfg);
    virtual OMX_ERRORTYPE  Encode(uint8* pInBuffer,
                                  uint32 nInBufSize,
                                  uint8* pOutBuffer,
                                  uint32 nOutBufSize,
                                  uint32 *pEncodedLen);
    virtual uint32  GetInputFrameSize();

private:
    void                          *mhAC3Handle;
    wfdAC3WrapperCreateFnType      pFnCreate;
    wfdAC3WrapperConfigureFnType   pFnConfigure;
    wfdAC3WrapperGetFrameLenFnType pFnGetFrameLen;
    wfdAC3WrapperEncodeFnType      pFnEncode;
    wfdAC3WrapperDestroyFnType     pFnDestroy;
    void                          *mhLibHandle;
};
#endif /*__WFD_MM_SOURCE_AC3_ENCODE_H__*/

