#ifndef __WFD_MM_SOURCE_AAC_ENCODE_H__
#define __WFD_MM_SOURCE_AAC_ENCODE_H__

/*==============================================================================
*       WFDMMSourceAACEncode.h
*
*  DESCRIPTION:
*       Class declaration WFDMMSourceAACEncode.
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

#define AAC_FRAME_SIZE 1024
//! WFDMMSourceAudioEncode class
//! Abstract interface for complete frame audio encode
class WFDMMSourceAACEncode:public WFDMMSourceAudioEncode
{
public:
    WFDMMSourceAACEncode();
    virtual ~WFDMMSourceAACEncode();
    virtual OMX_ERRORTYPE  Configure(audioConfigType *pCfg);
    virtual OMX_ERRORTYPE  Encode(uint8* pInBuffer,
                                  uint32 nInBufSize,
                                  uint8* pOutBuffer,
                                  uint32 nOutBufSize,
                                  uint32 *pEncodedLen);
    virtual uint32  GetInputFrameSize(){return (uint32)AAC_FRAME_SIZE;};

private:
    OMX_ERRORTYPE ConfigureAACEncoder();
    OMX_ERRORTYPE destroyResources();
    OMX_ERRORTYPE createResources();
    OMX_ERRORTYPE deinitialize();
    void          InitData();
    void         *m_hAACEncoder;
    void         *m_pAACInfo;
    audioConfigType mCfg;
};

#endif /*__WFD_MM_SOURCE_AAC_ENCODE_H__*/

