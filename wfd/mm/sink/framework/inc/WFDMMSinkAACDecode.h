#ifndef __WFD_MM_SINK_AAC_DECODE_H__
#define __WFD_MM_SINK_AAC_DECODE_H__
#if !defined (USE_OMX_AAC_CODEC) && !defined (USE_AUDIO_TUNNEL_MODE)
/*==============================================================================
*       WFDMMSinkAACDecode.h
*
*  DESCRIPTION:
*       Class declaration WFDMMSinkAACDecode.
*
*
*  Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
*  Qualcomm Technologies Proprietary and Confidential.
*===============================================================================
*/
/*==============================================================================
                             Edit History
================================================================================
   When            Who           Why
-----------------  ------------  -----------------------------------------------
05/15/2013         SK            InitialDraft
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

#include "OMX_Core.h"
#include "OMX_IVCommon.h"
#include "MMCriticalSection.h"
#include "WFDMMSinkCommon.h"
#include "wdsm_mm_interface.h"
#include "WFDMMSinkAudioDecode.h"
#include <aacdecoder_lib.h>
class WFDMMSinkAACDecode: public WFDMMSinkAudioDecode
{
public:

    WFDMMSinkAACDecode();

    virtual ~WFDMMSinkAACDecode();

    virtual OMX_ERRORTYPE  Configure(audioConfigType *pCfg);

    virtual OMX_ERRORTYPE  Decode(OMX_BUFFERHEADERTYPE *pBuffer);

private:
    OMX_ERRORTYPE          createResources();
    OMX_ERRORTYPE          destroyResources();
    OMX_ERRORTYPE          recoverAACDecoder();

    OMX_ERRORTYPE          deinitialize();

    void                   InitData();

    bool                   GenerateAACHeaderFromADTS(uint8* pAdts,
                                                     uint32 nAdtsLen,
                                                     uint8 *pAacHeader,
                                                     uint32 *nAACHdrLen);

    audioConfigType        mCfg;
    HANDLE_AACDECODER      mhAACDec;
    void                  *mpAACStreamInfo;
    MM_HANDLE              mhCritSect;
    bool                   mbAudioCodecHdrSet;

};
#endif//!defined (USE_OMX_AAC_CODEC) && !defined (USE_AUDIO_TUNNEL_MODE)
#endif /*__WFD_MM_SINK_AAC_DECODE_H__*/

