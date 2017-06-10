/*===========================================================================*]
[* Copyright (c) 2009-2013 QUALCOMM Technologies Incorporated.               *]
[* All Rights Reserved.                                                      *]
[* Qualcomm Technologies Confidential and Proprietary.                       *]
[*===========================================================================*]
[*===========================================================================*]
[* FILE NAME:   AdpcmDecoder.h                     TYPE: C-header file       *]
[* DESCRIPTION: Contains the defnition of AdpcmDecoder.cpp file.             *]
[* REVISION HISTORY:                                                         *]
[*   when           who     what, where, why                                 *]
[*   --------       ---     -------------------------------------------------*]
[*    3/22/2009:     Honghao      Initial revision                           *]
$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/WAVParserLib/main/latest/inc/CAdpcmDecoderLib.h#7 $
$DateTime: 2013/04/04 03:45:52 $
$Change: 3571381 $
[*===========================================================================*/
#ifndef _CADPCMDECODERLIB_H_
#define _CADPCMDECODERLIB_H_
#include <AEEStdDef.h>
#include "parserdatadef.h"
#include "parserinternaldefs.h"
#ifdef FEATURE_FILESOURCE_WAVADPCM
/*============================================================================
  Constants and Macros
============================================================================*/

/*===========================================================================
      Typedefs
============================================================================*/

class CAdpcmDecoderLib {

private:

    struct adpcm_state {
        int16	nValPrev;	/* Previous output value */
        int8	nIndex;		/* Index into stepsize table */
    };

private:

    static const int8 wav_parser_rgIndexTable[16];       /* The index table */
    static const uint16 wav_parser_rgStepsizeTable[89];   /* The stepsize table */

private:
    uint16 m_nNoOfChannels;     /* number of channels */
    uint32 m_ValidDataOffset;
    bool m_bDataReady;

private:

/*==========================================================================*/
/* FUNCTION: adpcm_decode                                                   */
/*                                                                          */
/* DESCRIPTION: decode a block of ADPCM data                                */
/*                                                                          */
/* INPUTS: pIndata: input adpcm data                                        */
/*         pOutData: output buffer for                                      */
/*         nLen: the number of samples to decode                            */
/*         pStateL: the state of the left channel adpcm                     */
/*         pStateR: the state of the right channel adpcm,                   */
/* OUTPUTS:                                                                 */
/*==========================================================================*/
    void wav_parser_adpcm_decode_block (
        uint8 *pIndata,
        int16 *pOutData,
        uint16 nLen,
        struct adpcm_state *pStateL,
        struct adpcm_state *pStateR
        );

public:
       CAdpcmDecoderLib();
       ~CAdpcmDecoderLib();

/*==========================================================================*/
/* FUNCTION: Process                                                        */
/*                                                                          */
/* DESCRIPTION: decode blocks of ADPCM data                                 */
/*                                                                          */
/* INPUTS: pAdpcm: This points to a buffer containing ADPCM blocks to be    */
/*         decoded.                                                         */
/*         nAdpcmLength: This indicates the number of bytes from the ADPCM  */
/*         buffer to be decoded.                                            */
/*         rAdpcmUsed: This indicates how many bytes of input are used      */
/*         pSamples: This points to a buffer where decoded PCM data will    */
/*         be stored. If NULL, no decoding is done, and return false        */
/*         nSamplesLength: This is the size of the given samples buffer,    */
/*         in samples.                                                      */
/*         rSamplesWritten: This indicates the number of samples outputted  */
/*         nBlockSize:  This indicates the desired number of bytes per ADPCM*/
/*         block.  It may be any number between 5 and 4095.                 */
/* OUTPUTS: return true if all adpcm blocks are decoded, return false       */
/*          otherwise                                                       */
/*==========================================================================*/
    PARSER_ERRORTYPE wav_parser_adpcm_dec_Process (
        uint8 *pAdpcm,
        uint32 nAdpcmLength,
        uint32 &rAdpcmUsed,
        int16 *pSamples,
        uint32 nSamplesLength,
        uint32 &rSamplesWritten,
        uint16 nBlockSize
        );

/*==========================================================================*/
/* FUNCTION: Init                                                           */
/*                                                                          */
/* DESCRIPTION: Initialize the adpcm decoder                                */
/*                                                                          */
/* INPUTS: nNoOfChannels: number of channels for decoding                   */
/* OUTPUTS:                                                                 */
/*==========================================================================*/
    void wav_parser_adpcm_dec_Init (uint16 nNoOfChannels);
    void SetDataOffset(uint32 valid_data_offset);
};

#endif //FEATURE_FILESOURCE_WAVADPCM
#endif //_CADPCMDECODERLIB_H_

