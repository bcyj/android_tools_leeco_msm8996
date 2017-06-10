/*========================================================================

*//** @file jpege_engine_sw_quant_zigzag.c
  This file contains the implementations for JPEG Encode Quant and Zigzag.

@par EXTERNALIZED FUNCTIONS
  jpege_engine_sw_quant_zigzag
  jpege_engine_sw_zigzag

@par INITIALIZATION AND SEQUENCING REQUIREMENTS
  (none)

Copyright (C) 2009-2011 Qualcomm Technologies, Inc.
All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.

*//*====================================================================== */

/*========================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/still/linux/main/latest/src/jpege_engine_sw_quant_zigzag.c#2 $

when       who     what, where, why
--------   ---     -------------------------------------------------------
07/13/10   zhiminl Added jpege_engine_sw_zigzag().
04/08/09   zhiminl Replaced zigzag table with zigzag offset table.
04/06/09   zhiminl Split jpege_engine_sw_quant_zigzag() from jpege_engine_sw.c.

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

/*------------------------------------------------------------------------
 *                          Static Variable Definitions
 * ----------------------------------------------------------------------- */

/*------------------------------------------------------------------------
 *                          Static Function Declarations and Definitions
 * ----------------------------------------------------------------------- */

/*------------------------------------------------------------------------
 *                          Externalized Function Definitions
 * ----------------------------------------------------------------------- */

void jpege_engine_sw_quant_zigzag
(
    const int16_t *quantInput,        /* pointer of the input 64 coefficients */
          int16_t *zigzagOutput,      /* pointer of the output 64 coefficients in zigzag order */
    const int16_t *zigzagOffsetTable, /* pointer of the zigzag offset table */
    const int16_t *quantTable         /* pointer of the quantization table */
)
{
    const int32_t rnd_value = FIX_POINT_ROUNDING_NUM;
    uint32_t      i         = JBLOCK_SIZE_MINUS_1;

    /* Shall scan zigzagOffsetTable decrementally, that is from 63 down to 0,
     * also adjust the pointers of quantInput and quantTable accordingly.
     */
    quantInput += JBLOCK_SIZE_MINUS_1;
    quantTable += JBLOCK_SIZE_MINUS_1;

    do
    {
        /* Quant first */
        const int32_t tmpOutput = (int32_t)((*quantInput) * (*quantTable));
        int16_t offset;

        /* Zigzag */
        if (tmpOutput >= 0)
        {
            /* positive value or zero */
            /* lint -e{639} Strong type mismatch for type 'uint16_t' in binary operation */
            zigzagOutput[i] = (int16_t)((tmpOutput + rnd_value) >> FIX_POINT_FACTOR);
        }
        else
        {
            /* negtive value */
            /* lint -e{639} Strong type mismatch for type 'uint16_t' in binary operation */
            zigzagOutput[i] = -(int16_t)(((-tmpOutput) + rnd_value) >> FIX_POINT_FACTOR);
        }
        /* Get the offset of the next coefficient in zigzag order */
        offset = (zigzagOffsetTable[i] / sizeof(int16_t));
        /* Go to the next coefficient */
        quantInput += offset;
        quantTable += offset;
    }
    while (i-- != 0);
} /* jpege_engine_sw_quant_zigzag */

void jpege_engine_sw_zigzag
(
    const int16_t *quantInput,        /* pointer of the input 64 coefficients */
          int16_t *zigzagOutput,      /* pointer of the output 64 coefficients in zigzag order */
    const int16_t *zigzagOffsetTable, /* pointer of the zigzag offset table */
    const int16_t *signTable          /* pointer of the sign table */
)
{
    uint32_t  i = JBLOCK_SIZE_MINUS_1;

    /* Shall scan zigzagOffsetTable decrementally, that is from 63 down to 0,
     * also adjust the pointers of quantInput and quantTable accordingly.
     */
    quantInput += JBLOCK_SIZE_MINUS_1;
    signTable  += JBLOCK_SIZE_MINUS_1;

    do
    {
        int16_t offset;

        /* Zigzag */
        zigzagOutput[i] = (int16_t)((*quantInput) * (*signTable));
        /* Get the offset of the next coefficient in zigzag order */
        offset = (zigzagOffsetTable[i] / sizeof(int16_t));
        /* Go to the next coefficient */
        quantInput += offset;
        signTable  += offset;
}
    while (i-- != 0);
} /* jpege_engine_sw_zigzag */

