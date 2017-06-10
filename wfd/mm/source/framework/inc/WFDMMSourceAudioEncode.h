#ifndef __WFD_MM_SINK_AUDIO_ENCODE_H__
#define __WFD_MM_SINK_AUDIO_ENCODE_H__

/*==============================================================================
*       WFDMMSourceAudioEncode.h
*
*  DESCRIPTION:
*       Class declaration WFDMMSourceAudioEncode.
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
#include "AEEStdDef.h"
#include "AEEstd.h"
#include "OMX_Core.h"
#include "OMX_Audio.h"

typedef struct audioConfig
{
    OMX_AUDIO_CODINGTYPE       eAudioType;
    uint32                     nSampleRate;
    uint32                     nChannels;
    uint32                     nBitsPerSample;
    uint32                     nBitrate;
    uint32                     nSamplesPerFrame;
}audioConfigType;

//! WFDMMSourceAudioEncode class
//! Abstract interface for complete frame audio encode
class WFDMMSourceAudioEncode
{
public:
    static WFDMMSourceAudioEncode* create(OMX_AUDIO_CODINGTYPE eFmt);
    WFDMMSourceAudioEncode(){;};
    virtual ~WFDMMSourceAudioEncode();
    virtual OMX_ERRORTYPE  Configure(audioConfigType *pCfg)=0;
    virtual OMX_ERRORTYPE  Encode(uint8* pInBuffer,
                                  uint32 nInBufSize,
                                  uint8* pOutBuffer,
                                  uint32 nOutBufSize,
                                  uint32 *pEncodedLen)=0;
    virtual uint32  GetInputFrameSize() = 0;
};

#endif /*__WFD_MM_SINK_AUDIO_ENCODE_H__*/

