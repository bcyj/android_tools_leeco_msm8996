/*========================================================================


*//** @file jpegd_engine_sw_idct.c

@par EXTERNALIZED FUNCTIONS
  (none)

@par INITIALIZATION AND SEQUENCING REQUIREMENTS
  (none)

Copyright (c) 1991-1998, Thomas G. Lane
See the IJG_README.txt file for more details.
Copyright (C) 2008-2009 Qualcomm Technologies, Inc.
All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.

*//*====================================================================== */

/*========================================================================
                             Edit History

$Header:

when       who     what, where, why
--------   ---     -------------------------------------------------------
10/15/09   mingy   Modified iDCT function signature.
                   Added ARM_ARCH_7A definition so that ArmV6 won't
                   pick it up.
10/10/09   mingy   Added iDCT coefficients derivations.
05/05/09   mingy   Replace clamp function with _CLAMP MACRO.
07/07/08   vma     Created file.

========================================================================== */

/* =======================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */
#include "jpeg_buffer_private.h"
#include "jpegd_engine_sw.h"
#include "jpegd.h"
#include "jpegerr.h"

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */
/* -----------------------------------------------------------------------
** Global Object Definitions
** ----------------------------------------------------------------------- */
/* -----------------------------------------------------------------------
** Local Object Definitions
** ----------------------------------------------------------------------- */
/* -----------------------------------------------------------------------
** Forward Declarations
** ----------------------------------------------------------------------- */
/* =======================================================================
**                       Macro/Constant Definitions
** ======================================================================= */

/***************************************************************************
        8 x 8         4 x 4
C1 = Cos(Pi*1/16)               = 0.9807852804
C2 = Cos(Pi*2/16) = Cos(Pi*1/8) = 0.9238795325
C3 = Cos(Pi*3/16)               = 0.8314696123
C4 = Cos(Pi*4/16) = Cos(Pi*2/8) = 0.7071067812
C5 = Cos(Pi*5/16)               = 0.5555702330
C6 = Cos(Pi*6/16) = Cos(Pi*3/8) = 0.3826834324
C7 = Cos(Pi*7/16)               = 0.1950903220

The following coefficents are in Q13 format:
eg: 0.298631336 * 2^13 = 2446
**************************************************************************/

#define IDCT_COEF_0_298631336     2446     //(-C1+C3+C5-C7)/C4
#define IDCT_COEF_0_390180644     3196     //       (C3-C5)/C4
#define IDCT_COEF_0_541196100     4433     //            C6/C4
#define IDCT_COEF_0_765366865     6270     //       (C2-C6)/C4
#define IDCT_COEF_0_899976223     7373     //       (C3-C7)/C4
#define IDCT_COEF_1_175875602     9633     //            C3/C4
#define IDCT_COEF_1_501321110     12299    // (C1+C3-C5-C7)/C4
#define IDCT_COEF_1_847759065     15137    //       (C2+C6)/C4
#define IDCT_COEF_1_961570560     16069    //       (C3+C5)/C4
#define IDCT_COEF_2_053119869     16819    // (C1+C3-C5+C7)/C4
#define IDCT_COEF_2_562915447     20995    //       (C1+C3)/C4
#define IDCT_COEF_3_072711026     25172    // (C1+C3+C5-C7)/C4
#define IDCT_COEF_1_000000000     8192
#define ONE                       1

#define _CLAMP(x) ( ((x) & 0xFFFFFF00) ? ((~(x)>>31) & 0xFF) : (x))

/* =======================================================================
**                          Function Definitions
** ======================================================================= */

#ifndef ARM_ARCH_7A
/**************************************************************************
FUNCTION     jpegd_engine_sw_idct_1x1
---------------------------------------------------------------------------
DESCRIPTION  IDCT on one block 1xx coefficients, output is clamped
---------------------------------------------------------------------------
INPUT  VALUE jpegd_coeff_t *: coefficent buffer of size 8x8
             jpegd_sample_t *: sample buffer of size 1x1
---------------------------------------------------------------------------
RETURN VALUE none
---------------------------------------------------------------------------
NOTES        none
**************************************************************************/
void jpegd_engine_sw_idct_1x1(jpegd_coeff_t * coeffPtr,
                              jpegd_sample_t * samplePtr,
                              int32_t stride)
{
    stride = 0;

    samplePtr[0] = _CLAMP((((coeffPtr[0]) + (ONE<<2)) >> 3 ) +128);
}
#endif /* !ARM_ARCH_7A */


#ifndef ARM_ARCH_7A
/**************************************************************************
FUNCTION     jpegd_engine_sw_idct_2x2
---------------------------------------------------------------------------
DESCRIPTION  IDCT on one block 2x2 coefficients, output is clamped
---------------------------------------------------------------------------
INPUT  VALUE jpegd_coeff_t *: coefficent buffer of size 8x8
             jpegd_sample_t *: sample buffer of size 2x2
---------------------------------------------------------------------------
RETURN VALUE none
---------------------------------------------------------------------------
NOTES        none
**************************************************************************/
void jpegd_engine_sw_idct_2x2(jpegd_coeff_t * coeffPtr,
                              jpegd_sample_t * samplePtr,
                              int32_t stride)
{
    /**********************************************************************
     * local variable
     *********************************************************************/
    jpegd_coeff_t *  inputPtr  = coeffPtr;       // local copy of input ptr
    jpegd_sample_t * outputPtr = samplePtr;      // local copy of input ptr

    int32_t a0, a1, a2, a3;

    stride = 0;

    /**********************************************************************
     * first process column-wise
     *********************************************************************/
    a0 = (int32_t)(inputPtr[JPEGD_DCTSIZE * 0]) +
         (int32_t)(inputPtr[JPEGD_DCTSIZE * 1]);
    a2 = (int32_t)(inputPtr[JPEGD_DCTSIZE * 0]) -
         (int32_t)(inputPtr[JPEGD_DCTSIZE * 1]);

    a1 = (int32_t)(inputPtr[JPEGD_DCTSIZE * 0 + 1 ] ) +
         (int32_t)(inputPtr[JPEGD_DCTSIZE * 1 + 1 ] );
    a3 = (int32_t)(inputPtr[JPEGD_DCTSIZE * 0 + 1 ] ) -
         (int32_t)(inputPtr[JPEGD_DCTSIZE * 1 + 1 ] );

    /**********************************************************************
     * now do row-wise process
     *********************************************************************/
    outputPtr[JPEGD_DCTSIZE * 0 + 0] = _CLAMP(((a0 + a1 + (ONE<<2)) >> 3) +128);
    outputPtr[JPEGD_DCTSIZE * 0 + 1] = _CLAMP(((a0 - a1 + (ONE<<2)) >> 3) +128);
    outputPtr[JPEGD_DCTSIZE * 1 + 0] = _CLAMP(((a2 + a3 + (ONE<<2)) >> 3) +128);
    outputPtr[JPEGD_DCTSIZE * 1 + 1] = _CLAMP(((a2 - a3 + (ONE<<2)) >> 3) +128);
}
#endif /* !ARM_ARCH_7A */


#ifndef ARM_ARCH_7A
/**************************************************************************
FUNCTION     jpegd_engine_sw_idct_4x4
---------------------------------------------------------------------------
DESCRIPTION  IDCT on one block 4x4 coefficients, output is clamped
---------------------------------------------------------------------------
INPUT  VALUE jpegd_coeff_t *: coefficent buffer of size 8x8
             jpegd_sample_t *: sample buffer of size 4x4
---------------------------------------------------------------------------
RETURN VALUE none
---------------------------------------------------------------------------
NOTES        none
**************************************************************************/
void jpegd_engine_sw_idct_4x4(jpegd_coeff_t * coeffPtr,
                              jpegd_sample_t * samplePtr,
                              int32_t stride)
{
    /**********************************************************************
     * local variable
     *********************************************************************/
    jpegd_coeff_t *  inputPtr  = coeffPtr;       // local copy of input ptr
    jpegd_sample_t * outputPtr = samplePtr;      // local copy of input ptr

    int32_t a0, a1;
    int32_t b0, b1;
    int32_t c1, c2, c3;
    int32_t temp[JPEGD_DCTSIZE2];               // intermediat working buffer
    int32_t * tempPtr;
    uint8_t i;

    const int32_t DCTSIZE = (JPEGD_DCTSIZE>>1);

    stride = 0;

    /**********************************************************************
     * first process column-wise
     *********************************************************************/
    tempPtr = temp;
    for (i = DCTSIZE; i>0; i--)
    {
        // even part
        c2 = (int32_t)( inputPtr[JPEGD_DCTSIZE * 0] );
        c3 = (int32_t)( inputPtr[JPEGD_DCTSIZE * 2] );

        a0 = (c2 + c3) << 13;
        a1 = (c2 - c3) << 13;

        // odd part
        c2 = (int32_t)( inputPtr[JPEGD_DCTSIZE * 1] );
        c3 = (int32_t)( inputPtr[JPEGD_DCTSIZE * 3] );
        c1 = (c2 + c3) * IDCT_COEF_0_541196100;

        b0 = c1 + (c3 * (-IDCT_COEF_1_847759065));
        b1 = c1 + (c2 * ( IDCT_COEF_0_765366865));

        /******************************************************************
         * add even and odd part together;
         * and round it
         ******************************************************************/
        tempPtr[ JPEGD_DCTSIZE * 0 ] = (a0 + b1 + 1024) >> 11;
        tempPtr[ JPEGD_DCTSIZE * 1 ] = (a1 + b0 + 1024) >> 11;
        tempPtr[ JPEGD_DCTSIZE * 2 ] = (a1 - b0 + 1024) >> 11;
        tempPtr[ JPEGD_DCTSIZE * 3 ] = (a0 - b1 + 1024) >> 11;

        /******************************************************************
         * increase pointer for next column
         ******************************************************************/
        inputPtr++;
        tempPtr++;
    }


    /**********************************************************************
     * now do row-wise process
     *********************************************************************/
    // first, reset pointer to top of intermediat storage
    tempPtr = temp;
    for (i=0; i<DCTSIZE; i++)
    {
        // even
        c2 = (int32_t)( tempPtr[0] );
        c3 = (int32_t)( tempPtr[2] );

        a0 = (c2 + c3) << 13;
        a1 = (c2 - c3) << 13;

        // odd
        c2 = (int32_t)( tempPtr[1] );
        c3 = (int32_t)( tempPtr[3] );
        c1 = (c2 + c3) * IDCT_COEF_0_541196100;

        b0 = c1 + (c3 * (-IDCT_COEF_1_847759065));
        b1 = c1 + (c2 * ( IDCT_COEF_0_765366865));

        /******************************************************************
         * add even and odd part together;
         * and round it
         *****************************************************************/
        outputPtr[0] = _CLAMP(((a0 + b1 + (ONE<<17)) >> 18) + 128);
        outputPtr[1] = _CLAMP(((a1 + b0 + (ONE<<17)) >> 18) + 128);
        outputPtr[2] = _CLAMP(((a1 - b0 + (ONE<<17)) >> 18) + 128);
        outputPtr[3] = _CLAMP(((a0 - b1 + (ONE<<17)) >> 18) + 128);

        /******************************************************************
         * increase pointer for next row
         *****************************************************************/
        tempPtr     += JPEGD_DCTSIZE;
        outputPtr   += JPEGD_DCTSIZE;
    }
}
#endif /* !ARM_ARCH_7A */


#ifndef ARM_ARCH_7A
/**************************************************************************
FUNCTION     jpegd_engine_sw_idct_8x8
---------------------------------------------------------------------------
DESCRIPTION  IDCT on one block 8x8 coefficients, output is clamped
---------------------------------------------------------------------------
INPUT  VALUE jpegd_coeff_t *: coefficent buffer of size 8x8
             jpegd_sample_t *: sample buffer of size 8x8
---------------------------------------------------------------------------
RETURN VALUE none
---------------------------------------------------------------------------
NOTES        none
**************************************************************************/
void jpegd_engine_sw_idct_8x8(jpegd_coeff_t * coeffPtr,
                              jpegd_sample_t * samplePtr,
                              int32_t stride)
{
    /**********************************************************************
    *  temporary variable and storage
    **********************************************************************/
    jpegd_coeff_t *  inputPtr  = coeffPtr;  // local copy of input ptr
    jpegd_sample_t * outputPtr = samplePtr; // local copy of input ptr

    int32_t a0, a1, a2, a3;
    int32_t b0, b1, b2, b3;
    int32_t c1, c2, c3, c4, c5;
    int32_t temp[JPEGD_DCTSIZE2];               // intermediat working buffer
    int32_t * tempPtr;
    int32_t i;

    stride = 0;

    /**********************************************************************
    * First process data column-wise and store them in temp
    * and scale the result by 4.
    **********************************************************************/
    tempPtr = temp;
    for (i = JPEGD_DCTSIZE; i >0; i--)        //column loop
    {
        /******************************************************************
        * if AC is not empty, do inverse dct
        * even part
        ******************************************************************/
        c2 = (int32_t)( inputPtr[ JPEGD_DCTSIZE * 2 ] );
        c3 = (int32_t)( inputPtr[ JPEGD_DCTSIZE * 6 ] );

        c1 = (c2 + c3) * IDCT_COEF_0_541196100;
        a2 = c1 + c3 * (- IDCT_COEF_1_847759065);
        a3 = c1 + c2 * IDCT_COEF_0_765366865;

        c2 = (int32_t)( inputPtr[ JPEGD_DCTSIZE * 0 ] );
        c3 = (int32_t)( inputPtr[ JPEGD_DCTSIZE * 4 ] );

        a0 = (c2 + c3) << 13;
        a1 = (c2 - c3) << 13;

        b0 = a0 + a3;
        b1 = a1 + a2;
        b2 = a1 - a2;
        b3 = a0 - a3;

        /******************************************************************
        * done with even part, now the odd part
        ******************************************************************/
        a0 = (int32_t)( inputPtr[ JPEGD_DCTSIZE * 7 ] );
        a1 = (int32_t)( inputPtr[ JPEGD_DCTSIZE * 5 ] );
        a2 = (int32_t)( inputPtr[ JPEGD_DCTSIZE * 3 ] );
        a3 = (int32_t)( inputPtr[ JPEGD_DCTSIZE * 1 ] );

        c1 = a0 + a3;
        c2 = a1 + a2;
        c3 = a0 + a2;
        c4 = a1 + a3;
        c5 = (c3 + c4) * IDCT_COEF_1_175875602;

        a0 = a0 * IDCT_COEF_0_298631336;
        a1 = a1 * IDCT_COEF_2_053119869;
        a2 = a2 * IDCT_COEF_3_072711026;
        a3 = a3 * IDCT_COEF_1_501321110;

        c1 = c1 * (-IDCT_COEF_0_899976223);
        c2 = c2 * (-IDCT_COEF_2_562915447);
        c3 = c3 * (-IDCT_COEF_1_961570560);
        c4 = c4 * (-IDCT_COEF_0_390180644);
        c3 = c3 + c5;
        c4 = c4 + c5;

        a0 += c1 + c3;
        a1 += c2 + c4;
        a2 += c2 + c3;
        a3 += c1 + c4;

        /******************************************************************
        * add even and odd part together;
        * and round it
        ******************************************************************/
        tempPtr[ JPEGD_DCTSIZE * 0 ] = (b0 + a3 + 1024) >> 11;
        tempPtr[ JPEGD_DCTSIZE * 1 ] = (b1 + a2 + 1024) >> 11;
        tempPtr[ JPEGD_DCTSIZE * 2 ] = (b2 + a1 + 1024) >> 11;
        tempPtr[ JPEGD_DCTSIZE * 3 ] = (b3 + a0 + 1024) >> 11;
        tempPtr[ JPEGD_DCTSIZE * 4 ] = (b3 - a0 + 1024) >> 11;
        tempPtr[ JPEGD_DCTSIZE * 5 ] = (b2 - a1 + 1024) >> 11;
        tempPtr[ JPEGD_DCTSIZE * 6 ] = (b1 - a2 + 1024) >> 11;
        tempPtr[ JPEGD_DCTSIZE * 7 ] = (b0 - a3 + 1024) >> 11;

        /******************************************************************
        * increase pointer for next column
        ******************************************************************/
        inputPtr++;
        tempPtr++;
    } // end column loop

    /**********************************************************************
    * now process row-wise, since column transfor might have generated lots
    * of non-empty coefficients, so no empty row check is necessary.
    **********************************************************************/
    // first, reset pointer to top of intermediat storage
    tempPtr = temp;
    for (i = 0; i < JPEGD_DCTSIZE; i++)
    {
        /******************************************************************
        * even part
        ******************************************************************/
        c2 = tempPtr[2];
        c3 = tempPtr[6];
        c1 = (c2 + c3) * IDCT_COEF_0_541196100;

        a2 = c1 + c3 * (-IDCT_COEF_1_847759065);
        a3 = c1 + c2 * IDCT_COEF_0_765366865;
        a0 = (tempPtr[0] + tempPtr[4]) << 13;
        a1 = (tempPtr[0] - tempPtr[4]) << 13;

        b0 = a0 + a3;
        b1 = a1 + a2;
        b2 = a1 - a2;
        b3 = a0 - a3;

        /******************************************************************
        * odd part
        ******************************************************************/
        a0 = tempPtr[7];
        a1 = tempPtr[5];
        a2 = tempPtr[3];
        a3 = tempPtr[1];

        c1 = a0 + a3;
        c2 = a1 + a2;
        c3 = a0 + a2;
        c4 = a1 + a3;
        c5 = (c3 + c4) * IDCT_COEF_1_175875602;

        a0 = a0 * IDCT_COEF_0_298631336;
        a1 = a1 * IDCT_COEF_2_053119869;
        a2 = a2 * IDCT_COEF_3_072711026;
        a3 = a3 * IDCT_COEF_1_501321110;

        c1 = c1 * (-IDCT_COEF_0_899976223);
        c2 = c2 * (-IDCT_COEF_2_562915447);
        c3 = c3 * (-IDCT_COEF_1_961570560);
        c4 = c4 * (-IDCT_COEF_0_390180644);

        c3 = c3 + c5;
        c4 = c4 + c5;

        a0 += c1 + c3;
        a1 += c2 + c4;
        a2 += c2 + c3;
        a3 += c1 + c4;

        /******************************************************************
        * output coefficient to output sample buffer.
        * the output is range limited [0, 0xFF]
        ******************************************************************/
        outputPtr[0] = _CLAMP(((b0 + a3 + (ONE<<17)) >> 18) +128);
        outputPtr[1] = _CLAMP(((b1 + a2 + (ONE<<17)) >> 18) +128);
        outputPtr[2] = _CLAMP(((b2 + a1 + (ONE<<17)) >> 18) +128);
        outputPtr[3] = _CLAMP(((b3 + a0 + (ONE<<17)) >> 18) +128);
        outputPtr[4] = _CLAMP(((b3 - a0 + (ONE<<17)) >> 18) +128);
        outputPtr[5] = _CLAMP(((b2 - a1 + (ONE<<17)) >> 18) +128);
        outputPtr[6] = _CLAMP(((b1 - a2 + (ONE<<17)) >> 18) +128);
        outputPtr[7] = _CLAMP(((b0 - a3 + (ONE<<17)) >> 18) +128);

        tempPtr     += JPEGD_DCTSIZE;
        outputPtr   += JPEGD_DCTSIZE;
    } // end row loop
}
#endif /* !ARM_ARCH_7A */

