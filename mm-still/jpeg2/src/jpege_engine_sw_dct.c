/*========================================================================

*//** @file jpege_engine_sw_dct.c

@par EXTERNALIZED FUNCTIONS
  jpege_engine_sw_fdct_block

@par INITIALIZATION AND SEQUENCING REQUIREMENTS
  (none)

Copyright (c) 1991-1998, Thomas G. Lane
See the IJG_README.txt file for more details.
Copyright (C) 2008-2011 Qualcomm Technologies, Inc.
All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.

*//*====================================================================== */

/*========================================================================
                             Edit History

$Header:

when       who     what, where, why
--------   ---     -------------------------------------------------------
04/06/09   zhiminl Removed jpege_engine_sw_dct_wrapper().
10/14/08   vma     Optimizations.
07/05/08   vma     Created file.

========================================================================== */
#include "jpege_engine_sw.h"

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */

/*
 * Macros for handling fixed-point arithmetic; these are used by many
 * but not all of the DCT modules.
 *
 * All values are expected to be of type int32_t.
 * Fractional constants are scaled left by CONST_BITS bits.
 * CONST_BITS is defined within each module using these macros,
 * and may differ from one module to the next.
 */

#define ONE ((int32_t) 1)
#define CONST_SCALE ( 1 << 13) //0000 0000 0000 0000 0010 0000 0000 0000

#define CONST_BITS  13
#define PASS1_BITS  2

/*
 * We assume that right shift corresponds to signed division by 2 with
 * rounding towards minus infinity.  This is correct for typical "arithmetic
 * shift" instructions that shift in copies of the sign bit.  But some
 * C compilers implement >> with an unsigned shift.  For these machines you
 * must define RIGHT_SHIFT_IS_UNSIGNED.
 * RIGHT_SHIFT provides a proper signed right shift of an int32_t quantity.
 * It is only applied with constant shift counts.                           */

#define RIGHT_SHIFT(x,shft) ((x) >> (shft))

/*
 * Descale and correctly round an int32_t value that's scaled by N bits.
 * We assume RIGHT_SHIFT rounds towards minus infinity, so adding
 * the fudge factor is correct for either sign of X.
 */

#define DESCALE(x,n)  RIGHT_SHIFT((x) + (ONE << ((n)-1)), n)

#define MULTIPLY(var,const)  ((var) * (const))

#define DCTSIZE         8   /* The basic DCT block is 8x8 samples */
#define DCTSIZE2        64  /* DCTSIZE squared; # of elements in a block */
#define CENTERJSAMPLE   128

#define GETJSAMPLE(value)  ((int16_t) (value))

/*
 * Some C compilers fail to reduce "FIX(constant)" at compile time, thus
 * causing a lot of useless floating-point operations at run time.
 * To get around this we use the following pre-calculated constants.
 * If you change CONST_BITS you may want to add appropriate values.

 */

/*if CONST_BITS == 13 */
#define FIX_0_298631336  ((int32_t)  2446)  /* FIX(0.298631336) */
#define FIX_0_390180644  ((int32_t)  3196)  /* FIX(0.390180644) */
#define FIX_0_541196100  ((int32_t)  4433)  /* FIX(0.541196100) */
#define FIX_0_765366865  ((int32_t)  6270)  /* FIX(0.765366865) */
#define FIX_0_899976223  ((int32_t)  7373)  /* FIX(0.899976223) */
#define FIX_1_175875602  ((int32_t)  9633)  /* FIX(1.175875602) */
#define FIX_1_501321110  ((int32_t)  12299) /* FIX(1.501321110) */
#define FIX_1_847759065  ((int32_t)  15137) /* FIX(1.847759065) */
#define FIX_1_961570560  ((int32_t)  16069) /* FIX(1.961570560) */
#define FIX_2_053119869  ((int32_t)  16819) /* FIX(2.053119869) */
#define FIX_2_562915447  ((int32_t)  20995) /* FIX(2.562915447) */
#define FIX_3_072711026  ((int32_t)  25172) /* FIX(3.072711026) */

/*
 * Multiply an int32_t variable by an int32_t constant to yield an int32_t result.
 * For 8-bit samples with the recommended scaling, all the variable
 * and constant values involved are no more than 16 bits wide, so a
 * 16x16->32 bit multiply can be used instead of a full 32x32 multiply.
 */

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
**                          Macro Definitions
** ======================================================================= */

/* =======================================================================
**                          Function Definitions
** ======================================================================= */

void jpege_engine_sw_fdct_block
(
    const uint8_t *pixelDomainBlock,
    DCTELEM       *frequencyDomainBlock
)
{
    int32_t        tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7;
    int32_t        tmp10, tmp11, tmp12, tmp13;
    int32_t        z1, z2, z3, z4, z5;
    const uint8_t *inputdataptr = pixelDomainBlock;
    DCTELEM       *outputdataptr = frequencyDomainBlock;
    int            ctr;

    /* Pass 1: process rows. */
    /* Note results are scaled up by sqrt(8) compared to a true DCT; */
    /* furthermore, we scale the results by 2**PASS1_BITS. */

    for (ctr = DCTSIZE-1; ctr >= 0; ctr--)
    {
        tmp0 = inputdataptr[0] + inputdataptr[7];
        tmp7 = inputdataptr[0] - inputdataptr[7];
        tmp1 = inputdataptr[1] + inputdataptr[6];
        tmp6 = inputdataptr[1] - inputdataptr[6];
        tmp2 = inputdataptr[2] + inputdataptr[5];
        tmp5 = inputdataptr[2] - inputdataptr[5];
        tmp3 = inputdataptr[3] + inputdataptr[4];
        tmp4 = inputdataptr[3] - inputdataptr[4];

        /* Even part per LL&M figure 1 --- note that published figure is faulty;
         * rotator "sqrt(2)*c1" should be "sqrt(2)*c6".
         */

        tmp10 = tmp0 + tmp3;
        tmp13 = tmp0 - tmp3;
        tmp11 = tmp1 + tmp2;
        tmp12 = tmp1 - tmp2;

        outputdataptr[0] = (DCTELEM) ((tmp10 + tmp11) << PASS1_BITS);
        outputdataptr[4] = (DCTELEM) ((tmp10 - tmp11) << PASS1_BITS);

        z1 = MULTIPLY (tmp12 + tmp13, FIX_0_541196100);
        outputdataptr[2] = (DCTELEM) DESCALE (z1 + MULTIPLY (tmp13, FIX_0_765366865),
                                              CONST_BITS-PASS1_BITS);
        outputdataptr[6] = (DCTELEM) DESCALE (z1 + MULTIPLY (tmp12, - FIX_1_847759065),
                                              CONST_BITS-PASS1_BITS);

        /* Odd part per figure 8 --- note paper omits factor of sqrt(2).
         * cK represents cos(K*pi/16).
         * i0..i3 in the paper are tmp4..tmp7 here.
         */

        z1 = tmp4 + tmp7;
        z2 = tmp5 + tmp6;
        z3 = tmp4 + tmp6;
        z4 = tmp5 + tmp7;
        z5 = MULTIPLY (z3 + z4, FIX_1_175875602); /* sqrt(2) * c3 */

        tmp4 = MULTIPLY (tmp4, FIX_0_298631336); /* sqrt(2) * (-c1+c3+c5-c7) */
        tmp5 = MULTIPLY (tmp5, FIX_2_053119869); /* sqrt(2) * ( c1+c3-c5+c7) */
        tmp6 = MULTIPLY (tmp6, FIX_3_072711026); /* sqrt(2) * ( c1+c3+c5-c7) */
        tmp7 = MULTIPLY (tmp7, FIX_1_501321110); /* sqrt(2) * ( c1+c3-c5-c7) */
        z1 = MULTIPLY (z1, - FIX_0_899976223); /* sqrt(2) * (c7-c3) */
        z2 = MULTIPLY (z2, - FIX_2_562915447); /* sqrt(2) * (-c1-c3) */
        z3 = MULTIPLY (z3, - FIX_1_961570560); /* sqrt(2) * (-c3-c5) */
        z4 = MULTIPLY (z4, - FIX_0_390180644); /* sqrt(2) * (c5-c3) */

        outputdataptr[7] = (DCTELEM) DESCALE (tmp4 + z1 + z3 + z5, 11);
        outputdataptr[5] = (DCTELEM) DESCALE (tmp5 + z2 + z4 + z5, 11);
        outputdataptr[3] = (DCTELEM) DESCALE (tmp6 + z2 + z3 + z5, 11);
        outputdataptr[1] = (DCTELEM) DESCALE (tmp7 + z1 + z4 + z5, 11);

        inputdataptr += DCTSIZE;    /* advance pointer to next row */
        outputdataptr += DCTSIZE;   /* advance pointer to next row */
    }


    /* Pass 2: process columns.
    * We remove the PASS1_BITS scaling, but leave the results scaled up
    * by an overall factor of 8.
    */

    outputdataptr = frequencyDomainBlock;
    for (ctr = DCTSIZE-1; ctr >= 0; ctr--)
    {
        tmp0 = outputdataptr[DCTSIZE*0] + outputdataptr[DCTSIZE*7];
        tmp7 = outputdataptr[DCTSIZE*0] - outputdataptr[DCTSIZE*7];
        tmp1 = outputdataptr[DCTSIZE*1] + outputdataptr[DCTSIZE*6];
        tmp6 = outputdataptr[DCTSIZE*1] - outputdataptr[DCTSIZE*6];
        tmp2 = outputdataptr[DCTSIZE*2] + outputdataptr[DCTSIZE*5];
        tmp5 = outputdataptr[DCTSIZE*2] - outputdataptr[DCTSIZE*5];
        tmp3 = outputdataptr[DCTSIZE*3] + outputdataptr[DCTSIZE*4];
        tmp4 = outputdataptr[DCTSIZE*3] - outputdataptr[DCTSIZE*4];
        //    printf("#%x,%x,%x,%x,%x,%x,%x,%x\n",tmp0,tmp1,tmp2,tmp3,tmp4,tmp5,tmp6,tmp7);
        /* Even part per LL&M figure 1 --- note that published figure is faulty;
         * rotator "sqrt(2)*c1" should be "sqrt(2)*c6".
         */

        tmp10 = tmp0 + tmp3;
        tmp13 = tmp0 - tmp3;
        tmp11 = tmp1 + tmp2;
        tmp12 = tmp1 - tmp2;
        //    printf("@%x,%x,%x,%x\n",tmp10,tmp11,tmp12,tmp13);
        outputdataptr[DCTSIZE*0] = (DCTELEM) DESCALE (tmp10 + tmp11, PASS1_BITS);
        outputdataptr[DCTSIZE*4] = (DCTELEM) DESCALE (tmp10 - tmp11, PASS1_BITS);
        //    printf("$%x,%x\n",tmp10 - tmp11,DESCALE(tmp10 - tmp11, PASS1_BITS));
        z1 = MULTIPLY (tmp12 + tmp13, FIX_0_541196100);
        outputdataptr[DCTSIZE*2] = (DCTELEM) DESCALE (z1 + MULTIPLY (tmp13, FIX_0_765366865),
                                                CONST_BITS+PASS1_BITS);
        outputdataptr[DCTSIZE*6] = (DCTELEM) DESCALE (z1 + MULTIPLY (tmp12, - FIX_1_847759065),
                                                CONST_BITS+PASS1_BITS);

        /* Odd part per figure 8 --- note paper omits factor of sqrt(2).
         * cK represents cos(K*pi/16).
         * i0..i3 in the paper are tmp4..tmp7 here.
         */

        z1 = tmp4 + tmp7;
        z2 = tmp5 + tmp6;
        z3 = tmp4 + tmp6;
        z4 = tmp5 + tmp7;
        z5 = MULTIPLY (z3 + z4, FIX_1_175875602); /* sqrt(2) * c3 */

        tmp4 = MULTIPLY (tmp4, FIX_0_298631336); /* sqrt(2) * (-c1+c3+c5-c7) */
        tmp5 = MULTIPLY (tmp5, FIX_2_053119869); /* sqrt(2) * ( c1+c3-c5+c7) */
        tmp6 = MULTIPLY (tmp6, FIX_3_072711026); /* sqrt(2) * ( c1+c3+c5-c7) */
        tmp7 = MULTIPLY (tmp7, FIX_1_501321110); /* sqrt(2) * ( c1+c3-c5-c7) */
        z1 = MULTIPLY (z1, - FIX_0_899976223); /* sqrt(2) * (c7-c3) */
        z2 = MULTIPLY (z2, - FIX_2_562915447); /* sqrt(2) * (-c1-c3) */
        z3 = MULTIPLY (z3, - FIX_1_961570560); /* sqrt(2) * (-c3-c5) */
        z4 = MULTIPLY (z4, - FIX_0_390180644); /* sqrt(2) * (c5-c3) */


        outputdataptr[DCTSIZE*7] = (DCTELEM) DESCALE (tmp4 + z1 + z3 + z5, 15);
        outputdataptr[DCTSIZE*5] = (DCTELEM) DESCALE (tmp5 + z2 + z4 + z5,
                                                      CONST_BITS+PASS1_BITS);
        outputdataptr[DCTSIZE*3] = (DCTELEM) DESCALE (tmp6 + z2 + z3 + z5,
                                                      CONST_BITS+PASS1_BITS);
        outputdataptr[DCTSIZE*1] = (DCTELEM) DESCALE (tmp7 + z1 + z4 + z5,
                                                      CONST_BITS+PASS1_BITS);

        outputdataptr++;      /* advance pointer to next column */
    }


    /* Subtracting 128*64 from DC coeff is equivalent to subtracting 128
     * from each pixel.
     */
    *frequencyDomainBlock = (*frequencyDomainBlock) - (CENTERJSAMPLE * DCTSIZE2);
}

