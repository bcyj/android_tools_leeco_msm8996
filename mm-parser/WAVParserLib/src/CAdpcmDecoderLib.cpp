/*===========================================================================*]
[* Copyright (c) 2009-2015 QUALCOMM Technologies Incorporated.               *]
[* All Rights Reserved.                                                      *]
[* Qualcomm Technologies Confidential and Proprietary.                       *]
[*===========================================================================*]
[*===========================================================================*]
[* FILE NAME: AdpcmDecoder.cpp                                               *]
[* DESCRIPTION:                                                              *]
[*    Contains the functions to decode adpcm input.                          *]
[*  FUNCTION LIST:                                                           *]
[*    Process: decode adpcm input                                            *]
[*    adpcm_decode_block: decode a block of adpcm data                       *]
[*    Init: initialize the adpcm decoder                                     *]
[*===========================================================================*]
[*  REVISION HISTORY:                                                        *]
[*  $Header                                                                  *]
[*  when       who      what, where, why                                     *]
[*  --------   ---      -----------------------------------------------------*]
[*    3/22/2009:     Honghao      Initial revision                           *]

$Header: //source/qcom/qct/multimedia2/Video/Source/FileDemux/WAVParserLib/main/latest/src/CAdpcmDecoderLib.cpp#16 $
$DateTime: 2013/04/04 03:45:52 $
$Change: 3571381 $

[*===========================================================================*/

/*============================================================================
INCLUDE FILES FOR MODULE
=============================================================================*/
#include "CAdpcmDecoderLib.h"
#include "parserdatadef.h"
#include "parserinternaldefs.h"
#include "MMDebugMsg.h"
#ifdef FEATURE_FILESOURCE_WAVADPCM
#ifdef __qdsp6__
#include <q6protos.h>
#include <q6types.h>
#endif

/*============================================================================
Constants and Macros
=============================================================================*/

const int8 CAdpcmDecoderLib::wav_parser_rgIndexTable [] = {
        -1, -1, -1, -1, 2, 4, 6, 8,
        -1, -1, -1, -1, 2, 4, 6, 8,
};
const uint16 CAdpcmDecoderLib::wav_parser_rgStepsizeTable [] = {
        7, 8, 9, 10, 11, 12, 13, 14, 16, 17,
        19, 21, 23, 25, 28, 31, 34, 37, 41, 45,
        50, 55, 60, 66, 73, 80, 88, 97, 107, 118,
        130, 143, 157, 173, 190, 209, 230, 253, 279, 307,
        337, 371, 408, 449, 494, 544, 598, 658, 724, 796,
        876, 963, 1060, 1166, 1282, 1411, 1552, 1707, 1878, 2066,
        2272, 2499, 2749, 3024, 3327, 3660, 4026, 4428, 4871, 5358,
        5894, 6484, 7132, 7845, 8630, 9493, 10442, 11487, 12635, 13899,
        15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767
};



CAdpcmDecoderLib::CAdpcmDecoderLib()
{
    m_nNoOfChannels = 1;     /* number of channels */
    m_ValidDataOffset = 0;
    m_bDataReady = true;
}

CAdpcmDecoderLib::~CAdpcmDecoderLib()
{

}

/*============================================================================
Decode functions
============================================================================*/
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
void CAdpcmDecoderLib::wav_parser_adpcm_decode_block (
        uint8 *pIndata,
        int16 *pOutData,
        uint16 nLen,
        struct adpcm_state *pStateL,
        struct adpcm_state *pStateR
        )
{
    int32 *pIn = NULL;		         /* Input buffer pointer */
    uint8 nSign = 0;			     /* Current adpcm sign bit */
    int32 nDelta = 0;			     /* Current adpcm output value */
    uint16 nStep = 0;			     /* Step-size */
    int32 nValPred = 0;		         /* Predicted value */
    int32 nVpDiff = 0;			     /* Current change to nValPred */
    int32 nIndex = 0;			     /* Current step change index */
    int32 nInputbuffer = 0;          /* Place to keep next 4-bit value */
    int16 *pChannelOutput = NULL;    /* Point to left or right channel */
    int16 *pOutL = NULL;             /* Left channel pointer */
    int16 *pOutR = NULL;             /* Right channel pointer */
    int32 nIndexRight = 0;           /* Step change index for right channel */
    int32 nIndexLeft = 0;            /* Step change index for left and right channel */
    int32 nValPredRight = 0;         /* Predicated value for right channel */
    int32 nValPredLeft = 0;          /* Predicated value for left channel */
    int nWordCount = 0;              /* Loop counter */
    uint16 nSampleCount = 0;         /* Loop counter */
    boolean bCheckChannel = FALSE;   /* Used to toggle between left and right channel */
    pIn = (int32 *)pIndata;

    /* get the previous value, and index */
    nValPred = pStateL->nValPrev;
    nIndex = pStateL->nIndex;
    int wav_parser_rgStepsizeTable_len= (int)sizeof(wav_parser_rgStepsizeTable)>>1;

    if (1 == m_nNoOfChannels ) /* decode mono adpcm data */
    {
        /* in a loop, decode all the input data */
        for ( nWordCount = 0; nWordCount < nLen  ; nWordCount+=8 )
        {
            /* read 32 bits data, so there are 8 samples to decode */
            nInputbuffer = *pIn++;

             // wav_parser_rgStepsizeTable[nIndex] may go out of bound as array size is 89
            /* in a loop decode 8 samples */
            for (nSampleCount = 0; (nSampleCount < 8) && (nIndex>=0) &&
                                   (nIndex <wav_parser_rgStepsizeTable_len) ; nSampleCount++)
            {
                /* decode the lower 4 bits of the input data */
                /* Step 1 - get the delta value */
                nDelta = nInputbuffer & 0xf;
                nInputbuffer = nInputbuffer >> 4;

                /* Step 2 - Update step value */
                nStep = wav_parser_rgStepsizeTable[nIndex];

                /* Step 3 - Find new index value (for later) */
                nIndex += wav_parser_rgIndexTable[nDelta];
                if ( nIndex < 0 ) nIndex = 0;
                if ( nIndex > 88 ) nIndex = 88;

                /* Step 4 - Separate sign and magnitude */
                nSign = nDelta & 8;

                /* Step 5 - Compute difference and new predicted value and */

                /* Computes 'nVpDiff = (delta+0.5)*step/4'                 */
                /* note: nVpDiff = Q6_P_mpyacc_RR( nStep, Q6_R_asland_RI(14, nDelta, 1), nStep)>>3; */
                /* do not save number of packets for the loop */
                nVpDiff = ((((nDelta & 0x7)<<1)+1) * nStep)>>3;

                if ( nSign )
	                nValPred -= nVpDiff;
	            else
	                nValPred += nVpDiff;

                /* Step 6 - clamp output value                             */
#ifdef __qdsp6__
                nValPred = Q6_R_sath_R(nValPred);
#else
	            if ( nValPred > 32767 )
	                nValPred = 32767;
	            else if ( nValPred < -32768 )
	                nValPred = -32768;
#endif //__qdsp6__

                /* Step 7 - Output value */
                if((m_ValidDataOffset == 0) || m_bDataReady)
                {
                  *pOutData++ = (int16)nValPred;
                }
                else
                {
                  m_ValidDataOffset--;
                  if(m_ValidDataOffset == 0)
                  {
                    m_bDataReady = true;
                  }
                }


            } /* end of the loop that decode 8 adpcm samples */
        } /* end of the loop that decode all adpcm data */
    } /* end of decode mono adpcm data */
    else /* decode stereo adpcm data */
    {
        /* set the output pointer to left channel and get the right channel index and Pred value */
        pChannelOutput = pOutData;
        pOutR = pOutData+1;
        nIndexRight = pStateR->nIndex;
        nValPredRight = pStateR->nValPrev;

        /* in a loop, decode all the input data */
        for ( nWordCount = 0; nWordCount < nLen  ; nWordCount+=8 )
        {
            /* read 32 bits data, so there are 8 samples to decode */
            nInputbuffer = *pIn++;

            /* in a loop decode 8 samples */
            // wav_parser_rgStepsizeTable[nIndex] may go out of bound
            for (nSampleCount = 0; (nSampleCount < 8) && (nIndex>=0) &&
                                   (nIndex < wav_parser_rgStepsizeTable_len); nSampleCount++)
            {
                /* decode the lower 4 bits of the input data */
                /* Step 1 - get the delta value */
                nDelta = nInputbuffer & 0xf;
                nInputbuffer = nInputbuffer >> 4;

                /* Step 2 - Update step value */
                nStep = wav_parser_rgStepsizeTable[nIndex];

                /* Step 3 - Find new index value (for later) */
                nIndex += wav_parser_rgIndexTable[nDelta];
                if ( nIndex < 0 ) nIndex = 0;
                if ( nIndex > 88 ) nIndex = 88;

                /* Step 4 - Separate sign and magnitude */
                nSign = nDelta & 8;

                /* Step 5 - Compute difference and new predicted value and */

                /* Computes 'nVpDiff = (delta+0.5)*step/4'                 */
                /* note: nVpDiff = Q6_P_mpyacc_RR( nStep, Q6_R_asland_RI(14, nDelta, 1), nStep)>>3; */
                /* do not save number of packets for the loop */
                nVpDiff = ((((nDelta & 0x7)<<1)+1) * nStep)>>3;

                if ( nSign )
	                nValPred -= nVpDiff;
	            else
	                nValPred += nVpDiff;

                /* Step 6 - clamp output value                             */
#ifdef __qdsp6__
                nValPred = Q6_R_sath_R(nValPred);
#else
	            if ( nValPred > 32767 )
	                nValPred = 32767;
	            else if ( nValPred < -32768 )
	                nValPred = -32768;
#endif //__qdsp6__

                /* Step 7 - Output value */
                if((m_ValidDataOffset == 0) || m_bDataReady)
                {
                  *pChannelOutput = (int16)nValPred;
                  pChannelOutput += 2;
                }
                else
                {
                  m_ValidDataOffset--;
                  if(m_ValidDataOffset == 0)
                  {
                    m_bDataReady = true;
                  }
                }
            } /* end of the loop that decode 8 adpcm samples */
            if ( bCheckChannel) /* need to decode left channel */
            {
                pOutR = pChannelOutput;
                nValPredRight = nValPred;
                nIndexRight = nIndex;
                nValPred = nValPredLeft;
                nIndex = nIndexLeft;
                pChannelOutput = pOutL;
            }
            else /* need to decode right channel */
            {
                pOutL = pChannelOutput;
                nValPredLeft = nValPred;
                nIndexLeft = nIndex;
                nValPred = nValPredRight;
                nIndex = nIndexRight;
                pChannelOutput = pOutR;
            }
            bCheckChannel = !bCheckChannel;
        } /* end of the loop that decode all adpcm data */
    } /* end of decode stereo adpcm data */
}

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
/* OUTPUTS: return AEE_SUCCESS if all adpcm blocks are decoded, return      */
/*          AEE_EFAILED otherwise                                           */
/* Notes: the input should be 4 bit adpcm data.                             */
/*==========================================================================*/
PARSER_ERRORTYPE CAdpcmDecoderLib::wav_parser_adpcm_dec_Process (
        uint8 *pAdpcm,
        uint32 nAdpcmLength,
        uint32 &rAdpcmUsed,
        int16 *pSamples,
        uint32 nSamplesLength,
        uint32 &rSamplesWritten,
        uint16 nBlockSize
        )
{
    uint16 nBlockSamples = 0;    /* the number of samples per block */
    uint16 nMaxBlocks    = 0;    /* maximum number of blocks to be decoded */
    uint16 nNumBlocks    = 0;    /* the actual number of blocks to be decoded */
    uint16 iCount;               /* loop counter */
    uint32 nSampleIndex;         /* the start index of the pSamples for a block */
    uint32 nAdpcmIndex;          /* the start index of the adpcm input for a block */
    uint32 nAdpcmDiffIndex;      /* the index of the first 4bit data to be decoded for a block*/
    struct adpcm_state stateL;   /* adpcm state for left channel */
    struct adpcm_state stateR;   /* adpcm state for right channel */
    int16 *pOut;                 /* output pointer */
    uint16 nSamplesToDecode = 0; /* the number of samples to be decoded for a block */

    /* Initialize secondary output. */
    rAdpcmUsed = 0;
    rSamplesWritten = 0;
    PARSER_ERRORTYPE eRet = PARSER_ErrorDefault;

    /* Make sure output buffer is provided */
    if (pSamples)
    {
      /* Compute samples per block, based on block size and channel mode*/
      if (2 == m_nNoOfChannels ) /* if stereo */
      {
        if(nBlockSize < 8)
        {
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,"nBlockSize < 8 when #Channels ==2");
          eRet = PARSER_ErrorStreamCorrupt;
        }
        else
        {
          /* find the number of samples to decode for the current block */
          nSamplesToDecode = (uint16)((nBlockSize - 8) * 2);
          //nSamplesToDecode = (nBlockSize - 8);
          /* find the total number of samples per block */
          nBlockSamples = (uint16)(nSamplesToDecode + 2);
          //nBlockSamples = nSamplesToDecode + 1;
          eRet = PARSER_ErrorNone;
        }
      }
      else /* if mono */
      {
        if(nBlockSize < 4)
        {
          MM_MSG_PRIO(MM_FILE_OPS, MM_PRIO_FATAL,"nBlockSize < 4 when #Channels ==1");
          eRet = PARSER_ErrorStreamCorrupt;
        }
        else
        {
          /* find the number of samples to decode for the current block */
          nSamplesToDecode = (uint16)((nBlockSize - 4) * 2);
          /* find the total number of samples per block */
          nBlockSamples = (uint16)(nSamplesToDecode + 1);
          eRet = PARSER_ErrorNone;
        }
      }
      if(eRet == PARSER_ErrorNone)
      {
        /* Calculate the maximum number of blocks that can be decoded from input buffer */
        nMaxBlocks = (uint16)nAdpcmLength / nBlockSize;
        /* Calculate the number of blocks that can be decoded so that the decoded data fits into the output buffer */
        nNumBlocks = (uint16)(FILESOURCE_MIN ( nMaxBlocks, nSamplesLength / nBlockSize ));
      }

      /* Return AEE_SUCCESS immediately if there isn't enough room to decode even one block. */
      if (nNumBlocks < 1)
      {
        eRet = PARSER_ErrorNone;
      }
      else if(eRet == PARSER_ErrorNone)
      {
        /* In a loop, decode ADPCM blocks into PCM samples. */
        for (iCount = 0; iCount < nNumBlocks; iCount++)
        {

            /* get the index of the first output sample and first input sample to decode in this block */
            nSampleIndex = iCount * nBlockSamples;
            nAdpcmIndex = iCount * nBlockSize;

            /* Retrieve initial state sample from block header for left channel*/
            pSamples [nSampleIndex] = stateL.nValPrev
                = reinterpret_cast< int16 &> (pAdpcm [nAdpcmIndex]);

            /* Retrieve index from block header for left channel*/
            stateL.nIndex = static_cast<int8> (reinterpret_cast< int16 &>
                                             (pAdpcm [nAdpcmIndex + 2]));

            if (2 == m_nNoOfChannels ) /* if stereo mode */
            {
                /* Retrieve initial state sample from block header for right channel*/
                pSamples [nSampleIndex+1] = stateR.nValPrev
                    = reinterpret_cast< int16 &> (pAdpcm [nAdpcmIndex+4]);

                /* Retrieve index from block header for right channel*/
                stateR.nIndex = static_cast<int8> (reinterpret_cast< int16 &>
                                                (pAdpcm [nAdpcmIndex + 6]));

                /* get the index for the first byte that will be send to the adpcm_decode_block function */
                nAdpcmDiffIndex = nAdpcmIndex + 4 * (int)sizeof (int16);

                /* set the output pointer */
                pOut = pSamples + nSampleIndex + 2;
            }
            else
            {
                /* get the index for the first byte that will be send to the adpcm_decode_block function */
                nAdpcmDiffIndex = nAdpcmIndex + 2 * (int)sizeof (int16);

                /* set the output pointer */
                pOut = pSamples + nSampleIndex + 1;
            }

            /* decode the current block */
            wav_parser_adpcm_decode_block (&pAdpcm [nAdpcmDiffIndex],
                            pOut, nSamplesToDecode,
                            &stateL, &stateR);
        }
        /* Set the number of ADPCM bytes decoded, and set  */
        /* the number of samples stored in the PCM buffer. */
        rAdpcmUsed = nNumBlocks * nBlockSize;
        rSamplesWritten = nNumBlocks * nBlockSamples;
      }//else if(eRet == PARSER_ErrorNone)
    }//if (pSamples)
    return eRet;
}
/*==========================================================================*/
/* FUNCTION: Init                                                           */
/*                                                                          */
/* DESCRIPTION: Initialize the adpcm decoder: set the number of channles    */
/*                                                                          */
/* INPUTS: nNoOfChannels: number of channels for decoding                   */
/* OUTPUTS:                                                                 */
/*==========================================================================*/
void CAdpcmDecoderLib::wav_parser_adpcm_dec_Init (uint16 nNoOfChannels)
{
    m_nNoOfChannels = nNoOfChannels;

}

/*==========================================================================*/
/* FUNCTION: SetDataOffset                                                  */
/*                                                                          */
/* DESCRIPTION: This function sets the data offset in decoder               */
/*                                                                          */
/* INPUTS: valid data offset                                                */
/* OUTPUTS:                                                                 */
/*==========================================================================*/

void CAdpcmDecoderLib::SetDataOffset(uint32 valid_data_offset)
{
  m_ValidDataOffset = valid_data_offset;
  m_bDataReady = false;
}
#endif //FEATURE_FILESOURCE_WAVADPCM

