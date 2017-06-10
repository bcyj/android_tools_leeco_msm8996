/*========================================================================

*//** @file jpege_engine_sw_huff_bs.c

@par EXTERNALIZED FUNCTIONS
  jpege_engine_sw_pack_bs
  jpege_engine_sw_huff_encode_dc
  jpege_engine_sw_huff_encode_ac

@par INITIALIZATION AND SEQUENCING REQUIREMENTS
  (none)

Copyright (C) 2009 Qualcomm Technologies, Inc.
All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.

*//*====================================================================== */

/*========================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/still/linux/main/latest/src/jpege_engine_sw_huff_bs.c#5 $

when       who     what, where, why
--------   ---     -------------------------------------------------------
06/17/09   zhiminl Fixed compiler warnings.
05/07/09   zhiminl Split jpege_engine_sw_huff_encode_ac() from
                   jpege_engine_sw_huff.c.
05/04/09   zhiminl Split jpege_engine_sw_pack_bs() from jpege_engine_sw_huff.c.
04/14/09   zhiminl Added internal bitstream pack buffer.
04/06/09   zhiminl Split dc and runlength encode from jpege_engine_sw_huff.c.

========================================================================== */

/*------------------------------------------------------------------------
 *                          Include Files
 * ----------------------------------------------------------------------- */

#include "jpege_engine_sw.h"

/*------------------------------------------------------------------------
 *                          Preprocessor Definitions and Constants
 * ----------------------------------------------------------------------- */

/*------------------------------------------------------------------------
 *                          Type Declarations
 * ----------------------------------------------------------------------- */

/*------------------------------------------------------------------------
 *                          Global Data Definitions
 * ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
 *                          Forward Declarations
 * ----------------------------------------------------------------------- */
void jpege_engine_sw_pack_bs (uint32_t, uint32_t, bitstream_t *);

/*------------------------------------------------------------------------
 *                          Static Variable Definitions
 * ----------------------------------------------------------------------- */

/*------------------------------------------------------------------------
 *                          Static Function Declarations and Definitions
 * ----------------------------------------------------------------------- */

static uint32_t jpege_engine_sw_huff_encode_ssss
(
    int32_t   value,
    uint32_t  numCategories,
    uint32_t *valueComplement
)
{
    uint32_t magnitude, category, bitMask;
    uint32_t complement;

    if (value >= 0)
    {
        magnitude = value;
        complement = value;
    }
    else
    {
        magnitude = -value;
        /* compute 1's complement, as it's -ve. */
        complement = value - 1;
    }

    /* bitMask to check if (numCategories - 1)th bit is 1 */
    bitMask = (1 << (numCategories - 2));

    /* if the (category)th bit is 1, category is i */
    for (category = (numCategories - 1); category > 0; category--)
    {
        /* if (category)th bit is 1. Quit for loop */
        if ((magnitude & bitMask) != 0)
        {
            break;
        }
        /* Change bitMask to check next bit */
        bitMask = bitMask >> 1;
    }
    /* At the end of the loop, category has appropriate value */

    /* Now setup bitMask to extract only category number of bits */
    bitMask = (bitMask << 1) - 1;

    /* Extract category number of bits from 1's complement */
    complement = (complement & bitMask);
    *valueComplement = complement;

    return category;
} /* jpege_engine_sw_huff_encode_ssss() */

static void jpege_engine_sw_runlength
(
    int32_t              runLength,
    int32_t              termSymbol,
    huff_lookup_table_t *p_acHuffLookupTable,
    bitstream_t         *p_bitstream
)
{
    uint32_t runSize;
    uint32_t acCategory, acComplement;
    uint32_t codeWord, codeLength;
    uint32_t xcodeWord, xcodeLength;

    /* Encode ZRL */
    while (runLength > 15)
    {
        jpege_engine_sw_pack_bs((p_acHuffLookupTable + ZRL)->codeWord,
                                (p_acHuffLookupTable + ZRL)->codeLength,
                                p_bitstream);
        runLength -= 16;
    }

    /* Encode general runlength */
    acCategory = jpege_engine_sw_huff_encode_ssss(termSymbol, NUM_AC_CATEGORIES, &acComplement);

    /* runSize = (R,S), Runlength being the upper nibble and Size
     * being the lower nibble.
     */
    runSize = runLength << 4 | acCategory;

    /*---------------------------------------------------------------
     Find codeword corresponding to runSize
    ---------------------------------------------------------------*/
    codeWord   = (p_acHuffLookupTable + runSize)->codeWord;
    codeLength = (p_acHuffLookupTable + runSize)->codeLength;

    /*---------------------------------------------------------------
     Pack codeword and 1's complement of AC Value into bitAssembly buffer
    ---------------------------------------------------------------*/
    if (acCategory < (NUM_AC_CATEGORIES - 1))
    {
        xcodeWord   = (codeWord << acCategory) | acComplement;
        xcodeLength = (codeLength + acCategory);
        jpege_engine_sw_pack_bs(xcodeWord, xcodeLength, p_bitstream);
    }
    else
    {
        jpege_engine_sw_pack_bs(codeWord, codeLength, p_bitstream);
        jpege_engine_sw_pack_bs(acComplement, acCategory, p_bitstream);
    }
} /* jpege_engine_sw_runlength() */

/*------------------------------------------------------------------------
 *                          Externalized Function Definitions
 * ----------------------------------------------------------------------- */

void jpege_engine_sw_pack_bs
(
    uint32_t     codeWord,
    uint32_t     codeLength,
    bitstream_t *p_bitstream
)
{
    uint32_t  bitAssemblyBufferUL32 = p_bitstream->bitAssemblyBufferUL32;
    uint32_t  bitsFree              = p_bitstream->bitsFree;
    uint8_t  *nextBytePtr           = p_bitstream->nextBytePtr;

    /*-----------------------------------------------------------------------
     Pack xCodeWord into bitAssembly buffer
    -----------------------------------------------------------------------*/
    bitsFree -= codeLength;

    // Copy the codeWord to the next empty location in the bitAssembly buff
    bitAssemblyBufferUL32 |= (codeWord << bitsFree);

    /*-----------------------------------------------------------------------
     Move bytes from bitAssembly buffer to final bitstream buffer.
    -----------------------------------------------------------------------*/
    while (bitsFree <= 24)
    {
        uint8_t byteToMove;
        // At least 1 byte to move
        // byteToMove = M.S. byte from the bitAssemblyBuffer
        byteToMove = (uint8_t) (bitAssemblyBufferUL32 >> 24);
        *(nextBytePtr++) = byteToMove;

        bitAssemblyBufferUL32 <<= 8;
        bitsFree += 8;

        // Insert byte '0x00' if the byteToMove is '0xFF'
        if (byteToMove == 0xFF)
        {
            *(nextBytePtr++) = 0x0;
        }
    }

    /*-----------------------------------------------------------------------
     It is possible there is some residue (less than 8 bits) in the
     32-bit assembly buffer for the next packing.
    -----------------------------------------------------------------------*/
    p_bitstream->bitAssemblyBufferUL32 = bitAssemblyBufferUL32;
    p_bitstream->bitsFree              = bitsFree;
    p_bitstream->nextBytePtr           = nextBytePtr;
} /* jpege_engine_sw_pack_bs() */

void jpege_engine_sw_huff_encode_dc
(
    int32_t              dcDiff,
    huff_lookup_table_t *p_dcHuffLookupTable,
    bitstream_t         *p_bitstream
)
{
    uint32_t dcCategory, dcDiffComplement;
    uint32_t codeWord, codeLength;
    uint32_t xcodeWord, xcodeLength;

    dcCategory = jpege_engine_sw_huff_encode_ssss(dcDiff, NUM_DC_CATEGORIES, &dcDiffComplement);

    /*-----------------------------------------------------------------------
     Find codeword and codelength corresponding to DC Category
    -----------------------------------------------------------------------*/
    /* Do a table look-up of category to get codeWord and codeLength       */
    codeWord   = (p_dcHuffLookupTable + dcCategory)->codeWord;
    codeLength = (p_dcHuffLookupTable + dcCategory)->codeLength;

    /*-----------------------------------------------------------------------
     Pack codeword and 1's complement of DC Value into bitAssembly buffer
    -----------------------------------------------------------------------*/
    xcodeWord   = (codeWord << dcCategory) | dcDiffComplement;
    xcodeLength = (codeLength + dcCategory);
    jpege_engine_sw_pack_bs(xcodeWord, xcodeLength, p_bitstream);
} /* jpege_engine_sw_huff_encode_dc() */

void jpege_engine_sw_huff_encode_ac
(
    register int16_t             *zigzagOutput,
    register huff_lookup_table_t *p_acHuffLookupTable,
    bitstream_t                  *p_bitstream
)
{
    uint32_t currSymbolIndex = 1;
    uint32_t runLength = 0;

    /*-----------------------------------------------------------------------
     Loop to encode the 63 AC symbols.
    -----------------------------------------------------------------------*/
    uint32_t i = JBLOCK_SIZE_MINUS_1;
    while ((i--) != 0)
    {
        int16_t currSymbol = zigzagOutput[currSymbolIndex++];

        if (currSymbol == 0)
        {
            runLength++;
        }
        else
        {
            /* General Runlength Encode */
            jpege_engine_sw_runlength(runLength, currSymbol, p_acHuffLookupTable,
                                      p_bitstream);
            /* Reset runlength after encoding */
            runLength = 0;
        }
    }

    /* Encode EOB */
    if (runLength != 0)
    {
        jpege_engine_sw_pack_bs((p_acHuffLookupTable + EOB)->codeWord,
                                (p_acHuffLookupTable + EOB)->codeLength,
                                p_bitstream);
    }
} /* jpege_engine_sw_huff_encode_ac() */
