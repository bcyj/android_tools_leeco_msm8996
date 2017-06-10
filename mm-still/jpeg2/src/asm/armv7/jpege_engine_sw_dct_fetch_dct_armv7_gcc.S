/*========================================================================

*//** @file jpege_engine_sw_fetch_dct.S
  This file contains the implementations for JPEG Encode DCT
  in ARM/NEON assembly.

@par EXTERNALIZED FUNCTIONS
  jpege_engine_sw_fdct_block
  jpege_engine_sw_fetch_dct_block_luma
  jpege_engine_sw_fetch_dct_block_chroma
  jpege_engine_sw_fetch_dct_block_luma_packed
  jpege_engine_sw_fetch_dct_block_chroma_packed
  jpege_engine_sw_fdct_u8


@par INITIALIZATION AND SEQUENCING REQUIREMENTS
  (none)

Copyright (C) 2009-12, 2014 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.

*//*====================================================================== */

/*========================================================================

                             Edit History

$Header: //source/qcom/qct/multimedia2/still/v3/utils/gcc/main/latest/src/jpege_engine_sw_dct_fetch_dct_armv7_gcc.S#1 $

when       who     what, where, why
--------   ---     -------------------------------------------------------
07/15/12   mingy   Added jpege_engine_sw_fdct_u8() function with stride
                   support.
04/15/12  csriniva Added support for YUY2/UYVY VeNum Implementation
10/20/10   mingy   Updated to be compilable with gcc.
09/08/09   zhiminl Added jpege_engine_sw_fetch_dct_block_luma() and
                   jpege_engine_sw_fetch_dct_block_chroma().
07/24/09   zhiminl Fast DCT algorithm based on ARMv7 SIMD.
04/09/09   zhiminl Fast DCT algorithm based on ARMv6 SIMD.

========================================================================== */

/*------------------------------------------------------------------------
 *                          Private Constants
 * ----------------------------------------------------------------------- */

.equ DCTSIZE,           8                 @ The basic DCT block is 8x8
.equ DCTSIZE2,          64                @ DCTSIZE squared,
                                          @ # of elements in a block

/* CCk = sqrt(1 / 8)cos((k*pi) / 16) in Q15, where k = 0,
 * CCk = sqrt(2 / 8)cos((k*pi) / 16) in Q15, where k = 1, 2, ..., 7.
 */
.equ CC0,               11585             @ 0x2D41, sqrt(1 / 8) * FIX(1.0000000000)
.equ CC1,               16069             @ 0x3EC5, sqrt(2 / 8) * FIX(0.9807852804)
.equ CC2,               15137             @ 0x3B21, sqrt(2 / 8) * FIX(0.9238795325)
.equ CC3,               13623             @ 0x3537, sqrt(2 / 8) * FIX(0.8314696123)
.equ CC4,               11585             @ 0x2D41, sqrt(2 / 8) * FIX(0.7071067812)
.equ CC5,               9102              @ 0x238E, sqrt(2 / 8) * FIX(0.5555702330)
.equ CC6,               6270              @ 0x187E, sqrt(2 / 8) * FIX(0.3826834324)
.equ CC7,               3196              @ 0x0C7C, sqrt(2 / 8) * FIX(0.1950903220)

/* The maximum sum of 8 pixel is 255 * 8 * sqrt(1 / 8) in PASS1, which
 * is 10 bits, the DCT kernel is in Q15, so the PASS1 8-point DCT
 * results could be 25 bits at most, right shift 12 bits to save as
 * signed 16-bit integer in Q3.
 *
 * In PASS2, the final results are also saved as signed 16-bit integer
 * in Q3 by right shift 15 bits.
 */
.equ DCT_PASS1_BITS,    12
.equ DCT_PASS2_BITS,    15

/* Scale down the final result by 8 (right shift 3 bits)
 */
.equ DCT_PASS2_BITS_SCALE_DOWN,   18

/*------------------------------------------------------------------------
 *                          EXPORTS
 * ----------------------------------------------------------------------- */

    .global jpege_engine_sw_fdct_block
    .global jpege_engine_sw_fetch_dct_block_luma
    .global jpege_engine_sw_fetch_dct_block_chroma
    .global jpege_engine_sw_fetch_dct_block_luma_packed
    .global jpege_engine_sw_fetch_dct_block_chroma_packed
    .global jpege_engine_sw_fdct_u8

/*------------------------------------------------------------------------
 *                          Macro Definitions
 * ----------------------------------------------------------------------- */

/*========================================================================

 MACRO               TRANSPOSE8x8

 DESCRIPTION         Transpose an 8 x 8 x 16-bit matrix in place.

 REGISTER INPUTS     Q1-Q8 containing the 8 x 8 x 16-bit matrix

 REGISTER OUTPUTS    Q1-Q8 containing the transpose of the matrix

 REGISTERS AFFECTED  Q1-Q8

========================================================================== */
    .macro  TRANSPOSE8x8
    VSWP      d3, d10                     @ q1 (d2, d3), q5 (d10, d11)
    VSWP      d9, d16                     @ q4 (d8, d9), q8 (d16, d17)
    VSWP      d7, d14                     @ q3 (d6, d7), q7 (d14, d15)
    VSWP      d5, d12                     @ q2 (d4, d5), q6 (d12, d13)

    VTRN.32   q1, q3
    VTRN.32   q2, q4
    VTRN.32   q5, q7
    VTRN.32   q6, q8

    VTRN.16   q1, q2
    VTRN.16   q3, q4
    VTRN.16   q5, q6
    VTRN.16   q7, q8
    .endm                                 @ end of TRANSPOSE8x8

/*========================================================================

 MACRO               BUTTERFLY8

 DESCRIPTION         Calculate t0, t1, t2, t3, t4, t5, t6, t7 for each row
                     of an 8 x 8 x 16-bit matrix in place:
                     t0 = x0 - x7
                     t1 = x1 - x6
                     t2 = x2 - x5
                     t3 = x3 - x4
                     t4 = x0 + x7
                     t5 = x1 + x6
                     t6 = x2 + x5
                     t7 = x3 + x4

 REGISTER INPUTS     Q1-Q8 containing the 8 x 8 x 16-bit matrix
                     with each row as [x7, x6, x5, x4, x3, x2, x1, x0]

 REGISTER OUTPUTS    Q1-Q8 containing the calculation results
                     with each row as [t0, t1, t2, t3, t7, t6, t5, t4]

 REGISTERS AFFECTED  Q0-Q8, Q13-Q15
                     Q0, Q13, Q14, Q15 are used as scratch registers

========================================================================== */
    .macro BUTTERFLY8

    VREV64.16 d0,  d3                     @ d0  = [0x00, x4, 0x00, x5, 0x00, x6, 0x00, x7] of row0
    VREV64.16 d1,  d2                     @ d1  = [0x00, x0, 0x00, x1, 0x00, x2, 0x00, x3] of row0
    VREV64.16 d26, d5                     @ d26 = [0x00, x4, 0x00, x5, 0x00, x6, 0x00, x7] of row1
    VREV64.16 d27, d4                     @ d27 = [0x00, x0, 0x00, x1, 0x00, x2, 0x00, x3] of row1
    VREV64.16 d28, d7                     @ d28 = [0x00, x4, 0x00, x5, 0x00, x6, 0x00, x7] of row2
    VREV64.16 d29, d6                     @ d29 = [0x00, x0, 0x00, x1, 0x00, x2, 0x00, x3] of row2
    VREV64.16 d30, d9                     @ d30 = [0x00, x4, 0x00, x5, 0x00, x6, 0x00, x7] of row3
    VREV64.16 d31, d8                     @ d31 = [0x00, x0, 0x00, x1, 0x00, x2, 0x00, x3] of row3

    VSUB.S16  d3, d1, d3                  @ d3  = [t0, t1, t2, t3] of row0
    VADD.S16  d2, d2, d0                  @ d2  = [t7, t6, t5, t4] of row0
    VSUB.S16  d5, d27, d5                 @ d5  = [t0, t1, t2, t3] of row1
    VADD.S16  d4, d4, d26                 @ d4  = [t7, t6, t5, t4] of row1
    VSUB.S16  d7, d29, d7                 @ d7  = [t0, t1, t2, t3] of row2
    VADD.S16  d6, d6, d28                 @ d6  = [t7, t6, t5, t4] of row2
    VSUB.S16  d9, d31, d9                 @ d9  = [t0, t1, t2, t3] of row3
    VADD.S16  d8, d8, d30                 @ d8  = [t7, t6, t5, t4] of row3

    VREV64.16 d0,  d11                    @ d0  = [0x00, x4, 0x00, x5, 0x00, x6, 0x00, x7] of row4
    VREV64.16 d1,  d10                    @ d1  = [0x00, x0, 0x00, x1, 0x00, x2, 0x00, x3] of row4
    VREV64.16 d26, d13                    @ d26 = [0x00, x4, 0x00, x5, 0x00, x6, 0x00, x7] of row5
    VREV64.16 d27, d12                    @ d27 = [0x00, x0, 0x00, x1, 0x00, x2, 0x00, x3] of row5
    VREV64.16 d28, d15                    @ d28 = [0x00, x4, 0x00, x5, 0x00, x6, 0x00, x7] of row6
    VREV64.16 d29, d14                    @ d29 = [0x00, x0, 0x00, x1, 0x00, x2, 0x00, x3] of row6
    VREV64.16 d30, d17                    @ d30 = [0x00, x4, 0x00, x5, 0x00, x6, 0x00, x7] of row7
    VREV64.16 d31, d16                    @ d31 = [0x00, x0, 0x00, x1, 0x00, x2, 0x00, x3] of row7

    VSUB.S16  d11, d1,  d11               @ d11 = [t0, t1, t2, t3] of row4
    VADD.S16  d10, d10, d0                @ d10 = [t7, t6, t5, t4] of row4
    VSUB.S16  d13, d27, d13               @ d13 = [t0, t1, t2, t3] of row5
    VADD.S16  d12, d12, d26               @ d12 = [t7, t6, t5, t4] of row5
    VSUB.S16  d15, d29, d15               @ d15 = [t0, t1, t2, t3] of row6
    VADD.S16  d14, d14, d28               @ d14 = [t7, t6, t5, t4] of row6
    VSUB.S16  d17, d31, d17               @ d17 = [t0, t1, t2, t3] of row7
    VADD.S16  d16, d16, d30               @ d16 = [t7, t6, t5, t4] of row7

    .endm                                 @ end of BUTTERFLY8

/*========================================================================

 MACRO               DCT1D t0123, t7654, row, shift

 DESCRIPTION         1D 8-point DCT in place.
                     The macro is used to calculate row0/column0 (Q1),
                     row1/column1 (Q2), row2/column2 (Q3).

 REGISTER INPUTS     t0123 - the doubleword register containing
                             [t0, t1, t2, t3] from MSHW to LSHW
                     t7654 - the doubleword register containing
                             [t7, 56, t5, t4] from MSHW to LSHW
                     shift - constant specifying the # of bits RIGHT
                             shifted when saving DCT results
                     Q9-Q12 containing DCT kernel from MSHW to LSHW
                     Q9    - [c60, c40, c20, c00, c70, c50, c30, c10]
                     Q10   - [c61, c41, c21, c00, c71, c51, c31, c11]
                     Q11   - [c62, c42, c22, c00, c72, c52, c32, c12]
                     Q12   - [c63, c43, c23, c00, c73, c53, c33, c13]

 REGISTER OUTPUTS    row   - the quadword register holding DCT results
                             as [s7, s6, s5, s4, s3, s2, s1, s0]

 REGISTERS AFFECTED  Q1-Q3, Q13-Q14
                     Q13, Q14 are used as scratch registers

========================================================================== */
    .macro  DCT1D t0123, t7654, row, shift

    /*--------------------------------------------------------------------
     * Calculate s0, s1, s2, s3, s4, s5, s6, s7 of the row
     * si = [ci0, ci1, ci2, ci3] * [t4, t5, t6, t7], i = 0, 2, 4, 6
     * si = [ci0, ci1, ci2, ci3] * [t0, t1, t2, t3], i = 1, 3, 5, 7
     * ------------------------------------------------------------------- */
    VMULL.S16 q13, d18, \t7654[0]         @ q13  = [c60, c40, c20, c00] * t4
    VMULL.S16 q14, d19, \t0123[3]         @ q14  = [c70, c50, c30, c10] * t0

    VMLAL.S16 q13, d20, \t7654[1]         @ q13 += [c61, c41, c21, c00] * t5
    VMLAL.S16 q14, d21, \t0123[2]         @ q14 += [c71, c51, c31, c11] * t1

    VMLAL.S16 q13, d22, \t7654[2]         @ q13 += [c62, c42, c22, c00] * t6
    VMLAL.S16 q14, d23, \t0123[1]         @ q14 += [c72, c52, c32, c12] * t2

    VMLAL.S16 q13, d24, \t7654[3]         @ q13 += [c63, c43, c23, c00] * t7
                                          @ q13  = [s6, s4, s2, s0] in Q15/Q18 (32-bit)
    VMLAL.S16 q14, d25, \t0123[0]         @ q14 += [c73, c53, c33, c13] * t3
                                          @ q14  = [s7, s5, s4, s2] in Q15/Q18 (32-bit)

    VSHR.S32  q13, q13, #(\shift)         @ q13 = [s6, s4, s2, s0] in Q3 (16-bit)
    VSHR.S32  q14, q14, #(\shift)         @ q14 = [s7, s5, s4, s2] in Q3 (16-bit)

    VSHL.S32  q14, q14, #16               @ d28 = [s3, 0x0000, s1, 0x0000] in 16-bit
                                          @ d29 = [s7, 0x0000, s5, 0x0000] in 16-bit
    VADD.S16  \row, q13, q14              @ done with the row

    .endm                                 @ end of DCT1D

/*========================================================================

 MACRO               DCT1Dx row, shift, nextrow

 DESCRIPTION         1D 8-point DCT in place. Basically this macro
                     does the same thing as macro DCT1D. It is defined
                     here due to the limitation of VeNum multiply
                     instructions which can only access registers D0-D7
                     (Q0-Q3) for 16-bit scalars. So Q0 (D0, D1) is
                     reserved here for this purpose.
                     The macro is used to calculate row3/column3 (Q4),
                     row4/column4 (Q5), row5/column5 (Q6), row6/column6 (Q7)
                     and row7/column7 (Q8).

 REGISTER INPUTS     Q0      - [t0, t1, t2, t3, t7, 56, t5, t4]
                               of current row from MSHW to LSHW
                     nextrow - the next row to be loaded as early as possible
                     shift   - constant specifying the # of bits RIGHT
                               shifted when saving DCT results
                     Q9-Q12 containing DCT kernel from MSHW to LSHW
                     Q9    - [c60, c40, c20, c00, c70, c50, c30, c10]
                     Q10   - [c61, c41, c21, c00, c71, c51, c31, c11]
                     Q11   - [c62, c42, c22, c00, c72, c52, c32, c12]
                     Q12   - [c63, c43, c23, c00, c73, c53, c33, c13]

 REGISTER OUTPUTS    row   - the quadword register holding DCT results
                             as [s7, s6, s5, s4, s3, s2, s1, s0]
                     Q0    - [t0, t1, t2, t3, t7, 56, t5, t4] of nextrow

 REGISTERS AFFECTED  Q0, Q4-Q8, Q13-Q14
                     Q13, Q14 are used as scratch registers

========================================================================== */
    .macro  DCT1Dx row, shift, nextrow

    /*--------------------------------------------------------------------
     * Calculate s0, s1, s2, s3, s4, s5, s6, s7 of the row
     * si = [ci0, ci1, ci2, ci3] * [t4, t5, t6, t7], i = 0, 2, 4, 6
     * si = [ci0, ci1, ci2, ci3] * [t0, t1, t2, t3], i = 1, 3, 5, 7
     * ------------------------------------------------------------------- */
    VMULL.S16 q13, d18, d0[0]             @ q13  = [c60, c40, c20, c00] * t4
    VMULL.S16 q14, d19, d1[3]             @ q14  = [c70, c50, c30, c10] * t0

    VMLAL.S16 q13, d20, d0[1]             @ q13 += [c61, c41, c21, c00] * t5
    VMLAL.S16 q14, d21, d1[2]             @ q14 += [c71, c51, c31, c11] * t1

    VMLAL.S16 q13, d22, d0[2]             @ q13 += [c62, c42, c22, c00] * t6
    VMLAL.S16 q14, d23, d1[1]             @ q14 += [c72, c52, c32, c12] * t2

    VMLAL.S16 q13, d24, d0[3]             @ q13 += [c63, c43, c23, c00] * t7
                                          @ q13  = [s6, s4, s2, s0] in Q15/Q18 (32-bit)
    VMLAL.S16 q14, d25, d1[0]             @ q14 += [c73, c53, c33, c13] * t3
                                          @ q14  = [s7, s5, s4, s2] in Q15/Q18 (32-bit)

    VMOV.S16  q0, \nextrow                @ d0  = [t7, t6, t5, t4] of the next row
                                          @ d1  = [t0, t1, t2, t3] of the next row

    VSHR.S32  q13, q13, #(\shift)         @ q13 = [s6, s4, s2, s0] in Q3  (16-bit)
    VSHR.S32  q14, q14, #(\shift)         @ q14 = [s7, s5, s4, s2] in Q3  (16-bit)

    VSHL.S32  q14, q14, #16               @ d28 = [s3, 0x0000, s1, 0x0000] in 16-bit
                                          @ d29 = [s7, 0x0000, s5, 0x0000] in 16-bit
    VADD.S16  \row, q13, q14              @ done with the row

    .endm                                 @ end of DCT1Dx

/*========================================================================

 MACRO               DCT2D dc dctOutput

 DESCRIPTION         Perform forward 2D DCT on a 8x8 pixel block.

 REGISTER INPUTS     Q1-Q8 containing the 8 x 8 x 16-bit pixel matrix
                     Q1:  first  [x7, x6, x5, x4, x3, x2, x1, x0] of row0,
                          second [t0, t1, t2, t3, t7, t6, t5, t4] of row0,
                          then 1D DCT results [s7, s6, s5, s4, s3, s2, s1, s0] of row0,
                          finally 2D DCT results for column0
                     Q2:  first  [x7, x6, x5, x4, x3, x2, x1, x0] of row1,
                          second [t0, t1, t2, t3, t7, t6, t5, t4] of row1,
                          then 1D DCT results [s7, s6, s5, s4, s3, s2, s1, s0] of row1,
                          finally 2D DCT results for column1
                     Q3:  first  [x7, x6, x5, x4, x3, x2, x1, x0] of row2,
                          second [t0, t1, t2, t3, t7, t6, t5, t4] of row2,
                          then 1D DCT results [s7, s6, s5, s4, s3, s2, s1, s0] of row2
                          finally 2D DCT results for column2
                     Q4:  first  [x7, x6, x5, x4, x3, x2, x1, x0] of row3,
                          second [t0, t1, t2, t3, t7, t6, t5, t4] of row3,
                          then 1D DCT results [s7, s6, s5, s4, s3, s2, s1, s0] of row3,
                          finally 2D DCT results for column3
                     Q5:  first  [x7, x6, x5, x4, x3, x2, x1, x0] of row4,
                          second [t0, t1, t2, t3, t7, t6, t5, t4] of row4,
                          then 1D DCT results [s7, s6, s5, s4, s3, s2, s1, s0] of row4,
                          finally 2D DCT results for column4
                     Q6:  first  [x7, x6, x5, x4, x3, x2, x1, x0] of row5,
                          second [t0, t1, t2, t3, t7, t6, t5, t4] of row5,
                          then 1D DCT results [s7, s6, s5, s4, s3, s2, s1, s0] of row5,
                          finally 2D DCT results for column5
                     Q7:  first  [x7, x6, x5, x4, x3, x2, x1, x0] of row6,
                          second [t0, t1, t2, t3, t7, t6, t5, t4] of row6,
                          then 1D DCT results [s7, s6, s5, s4, s3, s2, s1, s0] of row6,
                          finally 2D DCT results for column6
                     Q8:  first  [x7, x6, x5, x4, x3, x2, x1, x0] of row7,
                          second [t0, t1, t2, t3, t7, t6, t5, t4] of row7,
                          then 1D DCT results [s7, s6, s5, s4, s3, s2, s1, s0] of row7
                          finally 2D DCT results for column7
                     Q9-Q12 containing DCT kernel from MSHW to LSHW
                     Q9:  D18 = [c60, c40, c20, c00], D19 = [c70, c50, c30, c10]
                     Q10: D20 = [c61, c41, c21, c00], D21 = [c71, c51, c31, c11]
                     Q11: D22 = [c62, c42, c22, c00], D23 = [c72, c52, c32, c12]
                     Q12: D24 = [c63, c43, c23, c00], D25 = [c73, c53, c33, c13]

                     dc        - DC level shift calculation
                     dctOutput - pointer of 8x8 2D DCT results

 REGISTER OUTPUTS    Q1-Q8 containing the 2D DCT results transposed in Q3

 REGISTERS AFFECTED  dc, dctOutput, Q0-Q15
                     Q0, Q13, Q14, Q15 are used as scratch registers

========================================================================== */
    .macro  DCT2D dc, dctOutput

    /*--------------------------------------------------------------------
     * PASS1: 1D 8-point DCT processing begins
     * ------------------------------------------------------------------- */
    /*--------------------------------------------------------------------
     * PASS1: calculate t0, t1, t2, t3, t4, t5, t6, t7 for all rows
     * ------------------------------------------------------------------- */
    BUTTERFLY8                            @ q1 - q8
    /*--------------------------------------------------------------------
     * PASS1 : process all rows
     * ------------------------------------------------------------------- */
    VMOV.S16  q0, q4                      @ d0 = [t7, t6, t5, t4] of row3
                                          @ d1 = [t0, t1, t2, t3] of row3
                                          @ load row3 (q4) as early as possible
    DCT1D     d3, d2, q1, DCT_PASS1_BITS  @ process row0 (q1) in place
    DCT1D     d5, d4, q2, DCT_PASS1_BITS  @ process row1 (q2) in place
    DCT1D     d7, d6, q3, DCT_PASS1_BITS  @ process row2 (q3) in place
    DCT1Dx    q4, DCT_PASS1_BITS, q5      @ process row3 (q4) in place
                                          @ load row4 (q5) as early as possible
    DCT1Dx    q5, DCT_PASS1_BITS, q6      @ process row4 (q5) in place
                                          @ load row5 (q6) as early as possible
    DCT1Dx    q6, DCT_PASS1_BITS, q7      @ process row5 (q6) in place
                                          @ load row6 (q7) as early as possible
    DCT1Dx    q7, DCT_PASS1_BITS, q8      @ process row6 (q7) in place
                                          @ load row7 (q8) as early as possible
    DCT1Dx    q8, DCT_PASS1_BITS, #0      @ process row7 (q8) in place
    /*--------------------------------------------------------------------
     * PASS1: 1D 8-point DCT processing ends
     * ------------------------------------------------------------------- */

    /*--------------------------------------------------------------------
     * Transpose the intermediate 1D-DCT results
     * ------------------------------------------------------------------- */
    TRANSPOSE8x8                          @ transpose the data in q1 - q8

    /*--------------------------------------------------------------------
     * PASS2: 1D 8-point DCT processing begins
     * ------------------------------------------------------------------- */
    /*--------------------------------------------------------------------
     * PASS2: calculate t0, t1, t2, t3, t4, t5, t6, t7 for all columns
     * ------------------------------------------------------------------- */
    BUTTERFLY8                            @ q1 - q8
    /*--------------------------------------------------------------------
     * PASS2 : process all columns
     * ------------------------------------------------------------------- */
    VMOV.S16  q0, q4                      @ d0 = [t7, t6, t5, t4] of column3
                                          @ d1 = [t0, t1, t2, t3] of column3
                                          @ load column3 (q4) as early as possible
    DCT1D     d3, d2, q1, DCT_PASS2_BITS  @ process column0 (q1)
    DCT1D     d5, d4, q2, DCT_PASS2_BITS  @ process column1 (q2)
    VMOV.32   \dc, d2[0]                  @ dc = d2[0]

    DCT1D     d7, d6, q3, DCT_PASS2_BITS  @ process column2 (q3)
    SUB       \dc, \dc, #(128 * DCTSIZE2) @ dc -= (128 * DCTSIZE2)

    DCT1Dx    q4, DCT_PASS2_BITS, q5      @ process column3 (q4)
                                          @ load column4 (q5) as early as possible
    VMOV.32   d2[0], \dc                  @ d2[0] = dc

    DCT1Dx    q5, DCT_PASS2_BITS, q6      @ process column4 (q5)
                                          @ load column5 (q6) as early as possible
    DCT1Dx    q6, DCT_PASS2_BITS, q7      @ process column5 (q6)
                                          @ load column6 (q7) as early as possible
    DCT1Dx    q7, DCT_PASS2_BITS, q8      @ process column6 (q7)
                                          @ load column7 (q8) as early as possible
    DCT1Dx    q8, DCT_PASS2_BITS, #0      @ process column7 (q8)
    /*--------------------------------------------------------------------
     * PASS2: 1D 8-point DCT processing ends
     * ------------------------------------------------------------------- */

    /*--------------------------------------------------------------------
     * Done: save the 2D-DCT results
     * ------------------------------------------------------------------- */
    VST4.16   {d2[0],  d4[0],  d6[0],  d8[0]},  [\dctOutput]!
    VST4.16   {d10[0], d12[0], d14[0], d16[0]}, [\dctOutput]!
    VST4.16   {d2[1],  d4[1],  d6[1],  d8[1]},  [\dctOutput]!
    VST4.16   {d10[1], d12[1], d14[1], d16[1]}, [\dctOutput]!
    VST4.16   {d2[2],  d4[2],  d6[2],  d8[2]},  [\dctOutput]!
    VST4.16   {d10[2], d12[2], d14[2], d16[2]}, [\dctOutput]!
    VST4.16   {d2[3],  d4[3],  d6[3],  d8[3]},  [\dctOutput]!
    VST4.16   {d10[3], d12[3], d14[3], d16[3]}, [\dctOutput]!

    VST4.16   {d3[0],  d5[0],  d7[0],  d9[0]},  [\dctOutput]!
    VST4.16   {d11[0], d13[0], d15[0], d17[0]}, [\dctOutput]!
    VST4.16   {d3[1],  d5[1],  d7[1],  d9[1]},  [\dctOutput]!
    VST4.16   {d11[1], d13[1], d15[1], d17[1]}, [\dctOutput]!
    VST4.16   {d3[2],  d5[2],  d7[2],  d9[2]},  [\dctOutput]!
    VST4.16   {d11[2], d13[2], d15[2], d17[2]}, [\dctOutput]!
    VST4.16   {d3[3],  d5[3],  d7[3],  d9[3]},  [\dctOutput]!
    VST4.16   {d11[3], d13[3], d15[3], d17[3]}, [\dctOutput]!

    .endm                                 @ end of DCT2D

/*========================================================================

 MACRO               DCT2D_dstStride dc dctOutput dstStride

 DESCRIPTION         Perform forward 2D DCT on a 8x8 pixel block.

 REGISTER INPUTS     Q1-Q8 containing the 8 x 8 x 16-bit pixel matrix
                     Q1:  first  [x7, x6, x5, x4, x3, x2, x1, x0] of row0,
                          second [t0, t1, t2, t3, t7, t6, t5, t4] of row0,
                          then 1D DCT results [s7, s6, s5, s4, s3, s2, s1, s0] of row0,
                          finally 2D DCT results for column0
                     Q2:  first  [x7, x6, x5, x4, x3, x2, x1, x0] of row1,
                          second [t0, t1, t2, t3, t7, t6, t5, t4] of row1,
                          then 1D DCT results [s7, s6, s5, s4, s3, s2, s1, s0] of row1,
                          finally 2D DCT results for column1
                     Q3:  first  [x7, x6, x5, x4, x3, x2, x1, x0] of row2,
                          second [t0, t1, t2, t3, t7, t6, t5, t4] of row2,
                          then 1D DCT results [s7, s6, s5, s4, s3, s2, s1, s0] of row2
                          finally 2D DCT results for column2
                     Q4:  first  [x7, x6, x5, x4, x3, x2, x1, x0] of row3,
                          second [t0, t1, t2, t3, t7, t6, t5, t4] of row3,
                          then 1D DCT results [s7, s6, s5, s4, s3, s2, s1, s0] of row3,
                          finally 2D DCT results for column3
                     Q5:  first  [x7, x6, x5, x4, x3, x2, x1, x0] of row4,
                          second [t0, t1, t2, t3, t7, t6, t5, t4] of row4,
                          then 1D DCT results [s7, s6, s5, s4, s3, s2, s1, s0] of row4,
                          finally 2D DCT results for column4
                     Q6:  first  [x7, x6, x5, x4, x3, x2, x1, x0] of row5,
                          second [t0, t1, t2, t3, t7, t6, t5, t4] of row5,
                          then 1D DCT results [s7, s6, s5, s4, s3, s2, s1, s0] of row5,
                          finally 2D DCT results for column5
                     Q7:  first  [x7, x6, x5, x4, x3, x2, x1, x0] of row6,
                          second [t0, t1, t2, t3, t7, t6, t5, t4] of row6,
                          then 1D DCT results [s7, s6, s5, s4, s3, s2, s1, s0] of row6,
                          finally 2D DCT results for column6
                     Q8:  first  [x7, x6, x5, x4, x3, x2, x1, x0] of row7,
                          second [t0, t1, t2, t3, t7, t6, t5, t4] of row7,
                          then 1D DCT results [s7, s6, s5, s4, s3, s2, s1, s0] of row7
                          finally 2D DCT results for column7
                     Q9-Q12 containing DCT kernel from MSHW to LSHW
                     Q9:  D18 = [c60, c40, c20, c00], D19 = [c70, c50, c30, c10]
                     Q10: D20 = [c61, c41, c21, c00], D21 = [c71, c51, c31, c11]
                     Q11: D22 = [c62, c42, c22, c00], D23 = [c72, c52, c32, c12]
                     Q12: D24 = [c63, c43, c23, c00], D25 = [c73, c53, c33, c13]

                     dc        - DC level shift calculation
                     dctOutput - pointer of 8x8 2D DCT results
                     dstStride - the output stride in number of bytes

 REGISTER OUTPUTS    Q1-Q8 containing the 2D DCT results transposed in Q3

 REGISTERS AFFECTED  dc, dctOutput, dstStride, Q0-Q15
                     Q0, Q13, Q14, Q15 are used as scratch registers

========================================================================== */
    .macro  DCT2D_dstStride dc, dctOutput, dstStride

    SUB R5, dstStride, #8  @ R5 = dstStride - 4 * sizeof(int16_t)

    /*--------------------------------------------------------------------
     * PASS1: 1D 8-point DCT processing begins
     * ------------------------------------------------------------------- */
    /*--------------------------------------------------------------------
     * PASS1: calculate t0, t1, t2, t3, t4, t5, t6, t7 for all rows
     * ------------------------------------------------------------------- */
    BUTTERFLY8                            @ q1 - q8
    /*--------------------------------------------------------------------
     * PASS1 : process all rows
     * ------------------------------------------------------------------- */
    VMOV.S16  q0, q4                      @ d0 = [t7, t6, t5, t4] of row3
                                          @ d1 = [t0, t1, t2, t3] of row3
                                          @ load row3 (q4) as early as possible
    DCT1D     d3, d2, q1, DCT_PASS1_BITS  @ process row0 (q1) in place
    DCT1D     d5, d4, q2, DCT_PASS1_BITS  @ process row1 (q2) in place
    DCT1D     d7, d6, q3, DCT_PASS1_BITS  @ process row2 (q3) in place
    DCT1Dx    q4, DCT_PASS1_BITS, q5      @ process row3 (q4) in place
                                          @ load row4 (q5) as early as possible
    DCT1Dx    q5, DCT_PASS1_BITS, q6      @ process row4 (q5) in place
                                          @ load row5 (q6) as early as possible
    DCT1Dx    q6, DCT_PASS1_BITS, q7      @ process row5 (q6) in place
                                          @ load row6 (q7) as early as possible
    DCT1Dx    q7, DCT_PASS1_BITS, q8      @ process row6 (q7) in place
                                          @ load row7 (q8) as early as possible
    DCT1Dx    q8, DCT_PASS1_BITS, #0      @ process row7 (q8) in place
    /*--------------------------------------------------------------------
     * PASS1: 1D 8-point DCT processing ends
     * ------------------------------------------------------------------- */

    /*--------------------------------------------------------------------
     * Transpose the intermediate 1D-DCT results
     * ------------------------------------------------------------------- */
    TRANSPOSE8x8                          @ transpose the data in q1 - q8

    /*--------------------------------------------------------------------
     * PASS2: 1D 8-point DCT processing begins
     * ------------------------------------------------------------------- */
    /*--------------------------------------------------------------------
     * PASS2: calculate t0, t1, t2, t3, t4, t5, t6, t7 for all columns
     * ------------------------------------------------------------------- */
    BUTTERFLY8                            @ q1 - q8
    /*--------------------------------------------------------------------
     * PASS2 : process all columns
     * ------------------------------------------------------------------- */
    VMOV.S16  q0, q4                      @ d0 = [t7, t6, t5, t4] of column3
                                          @ d1 = [t0, t1, t2, t3] of column3
                                          @ load column3 (q4) as early as possible
    DCT1D     d3, d2, q1, DCT_PASS2_BITS_SCALE_DOWN  @ process column0 (q1)
    DCT1D     d5, d4, q2, DCT_PASS2_BITS_SCALE_DOWN  @ process column1 (q2)
    VMOV.32   \dc, d2[0]                  @ dc = d2[0]

    DCT1D     d7, d6, q3, DCT_PASS2_BITS_SCALE_DOWN  @ process column2 (q3)
    SUB       \dc, \dc, #(128 * DCTSIZE2) @ dc -= (128 * DCTSIZE2)

    DCT1Dx    q4, DCT_PASS2_BITS_SCALE_DOWN, q5      @ process column3 (q4)
                                          @ load column4 (q5) as early as possible
    VMOV.32   d2[0], \dc                  @ d2[0] = dc

    DCT1Dx    q5, DCT_PASS2_BITS_SCALE_DOWN, q6      @ process column4 (q5)
                                          @ load column5 (q6) as early as possible
    DCT1Dx    q6, DCT_PASS2_BITS_SCALE_DOWN, q7      @ process column5 (q6)
                                          @ load column6 (q7) as early as possible
    DCT1Dx    q7, DCT_PASS2_BITS_SCALE_DOWN, q8      @ process column6 (q7)
                                          @ load column7 (q8) as early as possible
    DCT1Dx    q8, DCT_PASS2_BITS_SCALE_DOWN, #0      @ process column7 (q8)
    /*--------------------------------------------------------------------
     * PASS2: 1D 8-point DCT processing ends
     * ------------------------------------------------------------------- */

    /*--------------------------------------------------------------------
     * Done: save the 2D-DCT results
     * Store 4 int16_t output, increment the output address automatically,
     * store another 4 int16_t output, increment the output by
     * dstStride - 4 * sizeof(int16_t)
     * ------------------------------------------------------------------- */
    VST4.16   {d2[0],  d4[0],  d6[0],  d8[0]},  [\dctOutput]!
    VST4.16   {d10[0], d12[0], d14[0], d16[0]}, [\dctOutput], R5
    VST4.16   {d2[1],  d4[1],  d6[1],  d8[1]},  [\dctOutput]!
    VST4.16   {d10[1], d12[1], d14[1], d16[1]}, [\dctOutput], R5
    VST4.16   {d2[2],  d4[2],  d6[2],  d8[2]},  [\dctOutput]!
    VST4.16   {d10[2], d12[2], d14[2], d16[2]}, [\dctOutput], R5
    VST4.16   {d2[3],  d4[3],  d6[3],  d8[3]},  [\dctOutput]!
    VST4.16   {d10[3], d12[3], d14[3], d16[3]}, [\dctOutput], R5

    VST4.16   {d3[0],  d5[0],  d7[0],  d9[0]},  [\dctOutput]!
    VST4.16   {d11[0], d13[0], d15[0], d17[0]}, [\dctOutput], R5
    VST4.16   {d3[1],  d5[1],  d7[1],  d9[1]},  [\dctOutput]!
    VST4.16   {d11[1], d13[1], d15[1], d17[1]}, [\dctOutput], R5
    VST4.16   {d3[2],  d5[2],  d7[2],  d9[2]},  [\dctOutput]!
    VST4.16   {d11[2], d13[2], d15[2], d17[2]}, [\dctOutput], R5
    VST4.16   {d3[3],  d5[3],  d7[3],  d9[3]},  [\dctOutput]!
    VST4.16   {d11[3], d13[3], d15[3], d17[3]}, [\dctOutput], R5

    .endm                                 @ end of DCT2D_dstStride

/*========================================================================

 MACRO               LDRDCTKernel

 DESCRIPTION         Load DCT kernel table

 REGISTER INPUTS     R12 - pointer to DCT kernel table

 REGISTER OUTPUTS    Q9-Q12 containing DCT kernel from MSHW to LSHW
                     Q9    - [c60, c40, c20, c00, c70, c50, c30, c10]
                     Q10   - [c61, c41, c21, c00, c71, c51, c31, c11]
                     Q11   - [c62, c42, c22, c00, c72, c52, c32, c12]
                     Q12   - [c63, c43, c23, c00, c73, c53, c33, c13]

 REGISTERS AFFECTED  R12, Q9-Q12
                     R12 is used as scratch register

========================================================================== */
    .macro   LDRDCTKernel

    ADRL       r12, dct_kernel_table_transposed
                                          @ r12 = dct_kernel_table_transposed
    /*--------------------------------------------------------------------
     * Load DCT kernel
     * ------------------------------------------------------------------- */
    VLD2.16   {d18, d19},  [r12]!         @ d18 = [c60, c40, c20, c00]
                                          @ d19 = [c70, c50, c30, c10]
    VLD2.16   {d20, d21},  [r12]!         @ d20 = [c61, c41, c21, c00]
                                          @ d21 = [c71, c51, c31, c11]
    VLD2.16   {d22, d23},  [r12]!         @ d22 = [c62, c42, c22, c00]
                                          @ d23 = [c72, c52, c32, c12]
    VLD2.16   {d24, d25},  [r12]!         @ d24 = [c63, c43, c23, c00]
                                          @ d25 = [c73, c53, c33, c13]

    .endm                                 @ end of LDRDCTKernel

/*========================================================================

 MACRO               LDRPixelBlock pixelBlock, rowIncrement

 DESCRIPTION         Load a 8x8 8-bit pixel block and zero expand to
                     8 x 8 x 16-bit matrix

 REGISTER INPUTS     pixelBlock   - pointer to pixel block
                     rowIncrement - row increment to the next line
                                    of the block

 REGISTER OUTPUTS    Q1-Q8 containing the 8 x 8 x 16-bit matrix
                     with each row as [x7, x6, x5, x4, x3, x2, x1, x0],
                     where each 8-bit pixel is expanded as 16-bit

 REGISTERS AFFECTED  pixelBlock, rowIncrement
                     Q1-Q8

========================================================================== */
    .macro  LDRPixelBlock pixelBlock, rowIncrement

    /*--------------------------------------------------------------------
     * Load the 8-bit pixel values from input pointer in q1-q4
     * ------------------------------------------------------------------- */
    VLD1.8    {d2},  [\pixelBlock], \rowIncrement
                                          @ d2  = [x7, x6, x5, x4, x3, x2, x1, x0] of row0
    VLD1.8    {d4},  [\pixelBlock], \rowIncrement
                                          @ d4  = [x7, x6, x5, x4, x3, x2, x1, x0] of row1
    VLD1.8    {d6},  [\pixelBlock], \rowIncrement
                                          @ d6  = [x7, x6, x5, x4, x3, x2, x1, x0] of row2
    VLD1.8    {d8},  [\pixelBlock], \rowIncrement
                                          @ d8  = [x7, x6, x5, x4, x3, x2, x1, x0] of row3
    /*--------------------------------------------------------------------
     * Zero expand the 8-bit pixel values to 16-bit in q1-q4
     * ------------------------------------------------------------------- */
    VMOVL.U8  q1, d2                      @ d2  = [0x00, x3, 0x00, x2, 0x00, x1, 0x00, x0] of row0
                                          @ d3  = [0x00, x7, 0x00, x6, 0x00, x5, 0x00, x4] of row0
    VMOVL.U8  q2, d4                      @ d4  = [0x00, x3, 0x00, x2, 0x00, x1, 0x00, x0] of row1
                                          @ d5  = [0x00, x7, 0x00, x6, 0x00, x5, 0x00, x4] of row1
    VMOVL.U8  q3, d6                      @ d6  = [0x00, x3, 0x00, x2, 0x00, x1, 0x00, x0] of row2
                                          @ d7  = [0x00, x7, 0x00, x6, 0x00, x5, 0x00, x4] of row2
    VMOVL.U8  q4, d8                      @ d8  = [0x00, x3, 0x00, x2, 0x00, x1, 0x00, x0] of row3
                                          @ d9  = [0x00, x7, 0x00, x6, 0x00, x5, 0x00, x4] of row3

    /*--------------------------------------------------------------------
     * Load the 8-bit pixel values from input pointer in q5-q8
     * ------------------------------------------------------------------- */
    VLD1.8    {d10}, [\pixelBlock], \rowIncrement
                                          @ d10 = [x7, x6, x5, x4, x3, x2, x1, x0] of row4
    VLD1.8    {d12}, [\pixelBlock], \rowIncrement
                                          @ d12 = [x7, x6, x5, x4, x3, x2, x1, x0] of row5
    VLD1.8    {d14}, [\pixelBlock], \rowIncrement
                                          @ d14 = [x7, x6, x5, x4, x3, x2, x1, x0] of row6
    VLD1.8    {d16}, [\pixelBlock], \rowIncrement
                                          @ d16 = [x7, x6, x5, x4, x3, x2, x1, x0] of row7
    /*--------------------------------------------------------------------
     * Zero expand the 8-bit pixel values to 16-bit in q5-q8
     * ------------------------------------------------------------------- */
    VMOVL.U8  q5, d10                     @ d10 = [0x00, x3, 0x00, x2, 0x00, x1, 0x00, x0] of row4
                                          @ d11 = [0x00, x7, 0x00, x6, 0x00, x5, 0x00, x4] of row4
    VMOVL.U8  q6, d12                     @ d12 = [0x00, x3, 0x00, x2, 0x00, x1, 0x00, x0] of row5
                                          @ d13 = [0x00, x7, 0x00, x6, 0x00, x5, 0x00, x4] of row5
    VMOVL.U8  q7, d14                     @ d14 = [0x00, x3, 0x00, x2, 0x00, x1, 0x00, x0] of row6
                                          @ d15 = [0x00, x7, 0x00, x6, 0x00, x5, 0x00, x4] of row6
    VMOVL.U8  q8, d16                     @ d16 = [0x00, x3, 0x00, x2, 0x00, x1, 0x00, x0] of row7
                                          @ d17 = [0x00, x7, 0x00, x6, 0x00, x5, 0x00, x4] of row7

    .endm                                 @ end of LDRPixelBlock

/*=========================================================================
 MACRO               LDRPixelBlockPacked pixelBlock, rowIncrement

 DESCRIPTION         Load a 8x8 8-bit pixel block and zero expand to
                     8 x 8 x 16-bit matrix for YUY2/UYVY Formats

 REGISTER INPUTS     pixelBlock   - pointer to pixel block
                     rowIncrement - row increment to the next line
                                    of the block

 REGISTER OUTPUTS    Q1-Q8 containing the 8 x 8 x 16-bit matrix
                     with each row as [x7, x6, x5, x4, x3, x2, x1, x0],
                     where each 8-bit pixel is expanded as 16-bit

 REGISTERS AFFECTED  pixelBlock, rowIncrement
                     Q1-Q8
=========================================================================*/
    .macro  LDRPixelBlockPacked pixelBlock, rowIncrement
    @---------------------------------------------------------------------
    @ Load the 8-bit pixel values from input pointer in q1-q4
    @---------------------------------------------------------------------
    VLD2.8    {d2, d3},  [\pixelBlock], \rowIncrement
                                          @ d2  = [x7, x6, x5, x4, x3, x2, x1, x0] of row0
                                          @ d3 = dummy read the chroma values
    VLD2.8    {d4, d5},  [\pixelBlock], \rowIncrement
                                          @ d4  = [x7, x6, x5, x4, x3, x2, x1, x0] of row1
                                          @ d5 = dummy read the chroma values
    VLD2.8    {d6, d7},  [\pixelBlock], \rowIncrement
                                          @ d6  = [x7, x6, x5, x4, x3, x2, x1, x0] of row2
                                          @ d7 = dummy read the chroma values
    VLD2.8    {d8, d9},  [\pixelBlock], \rowIncrement
                                          @ d8  = [x7, x6, x5, x4, x3, x2, x1, x0] of row3
                                          @ d9 = dummy read the chroma values
    @---------------------------------------------------------------------
    @ Zero expand the 8-bit pixel values to 16-bit in q1-q4
    @---------------------------------------------------------------------
    VMOVL.U8  q1, d2                      @ d2  = [0x00, x3, 0x00, x2, 0x00, x1, 0x00, x0] of row0
                                          @ d3  = [0x00, x7, 0x00, x6, 0x00, x5, 0x00, x4] of row0
    VMOVL.U8  q2, d4                      @ d4  = [0x00, x3, 0x00, x2, 0x00, x1, 0x00, x0]  of row1
                                          @ d5  = [0x00, x7, 0x00, x6, 0x00, x5, 0x00, x4] of row1
    VMOVL.U8  q3, d6                      @ d6  = [0x00, x3, 0x00, x2, 0x00, x1, 0x00, x0]  of row2
                                          @ d7  = [0x00, x7, 0x00, x6, 0x00, x5, 0x00, x4] of row2
    VMOVL.U8  q4, d8                      @ d8  = [0x00, x3, 0x00, x2, 0x00, x1, 0x00, x0]  of row3
                                          @ d9  = [0x00, x7, 0x00, x6, 0x00, x5, 0x00, x4] of row3

    @---------------------------------------------------------------------
    @ Load the 8-bit pixel values from input pointer in q5-q8
    @---------------------------------------------------------------------
    VLD2.8    {d10, d11}, [\pixelBlock], \rowIncrement
                                          @ d10 = [x7, x6, x5, x4, x3, x2, x1, x0] of row4
                                          @ d11 = dummy load chroma values
    VLD2.8    {d12, d13}, [\pixelBlock], \rowIncrement
                                          @ d12 = [x7, x6, x5, x4, x3, x2, x1, x0] of row5
                                          @ d18 = dummy read chroma values
    VLD2.8    {d14, d15}, [\pixelBlock], \rowIncrement
                                          @ d14 = [x7, x6, x5, x4, x3, x2, x1, x0] of row6
                                          @ d15 = dummy read chroma values
    VLD2.8    {d16, d17}, [\pixelBlock], \rowIncrement
                                          @ d16 = [x7, x6, x5, x4, x3, x2, x1, x0] of row7
                                          @ d17 = dummy read chroma values
    @---------------------------------------------------------------------
    @ Zero expand the 8-bit pixel values to 16-bit in q5-q8
    @---------------------------------------------------------------------
    VMOVL.U8  q5, d10                     @ d10 = [0x00, x3, 0x00, x2, 0x00, x1, 0x00, x0] of row4
                                          @ d11 = [0x00, x7, 0x00, x6, 0x00, x5, 0x00, x4] of row4
    VMOVL.U8  q6, d12                     @ d12 = [0x00, x3, 0x00, x2, 0x00, x1, 0x00, x0]  of row5
                                          @ d13 = [0x00, x7, 0x00, x6, 0x00, x5, 0x00, x4]of row5
    VMOVL.U8  q7, d14                     @ d14 = [0x00, x3, 0x00, x2, 0x00, x1, 0x00, x0]  of row6
                                          @ d15 = [0x00, x7, 0x00, x6, 0x00, x5, 0x00, x4] of row6
    VMOVL.U8  q8, d16                     @ d16 = [0x00, x3, 0x00, x2, 0x00, x1, 0x00, x0]  of row7
                                          @ d17 = [0x00, x7, 0x00, x6, 0x00, x5, 0x00, x4] of row7
    .endm                                 @ end of LDRPixelBlock

/*========================================================================

 MACRO               LDRInterleavedBlock1 interleavedBlock, rowIncrement

 DESCRIPTION         Load and deinterleave a 8x8 8-bit pixel block and
                     zero expand to 8 x 8 x 16-bit matrix

 REGISTER INPUTS     interleavedBlock - pointer to interleaved pixel block
                                        with each row as
                                        [x0, y0, x1, y1, x2, y2,....]
                     rowIncrement     - row increment to the next line
                                        of the block

 REGISTER OUTPUTS    Q1-Q8 containing the 8 x 8 x 16-bit matrix
                     with each row as [x7, x6, x5, x4, x3, x2, x1, x0],
                     where each 8-bit pixel is expanded as 16-bit

 REGISTERS AFFECTED  interleavedBlock, rowIncrement
                     Q1-Q8

========================================================================== */
    .macro  LDRInterleavedBlock1 interleavedBlock, rowIncrement

    /*--------------------------------------------------------------------
     * Load the 8-bit pixel values from input pointer in q1-q4
     * ------------------------------------------------------------------- */
    VLD2.8    {d2, d3},  [\interleavedBlock], \rowIncrement
                                          @ d2  = [x7, x6, x5, x4, x3, x2, x1, x0] of row0
                                          @ d3  = [y7, y6, y5, y4, y3, y2, y1, y0] of row0
    VLD2.8    {d4, d5},  [\interleavedBlock], \rowIncrement
                                          @ d4  = [x7, x6, x5, x4, x3, x2, x1, x0] of row1
                                          @ d4  = [y7, y6, y5, y4, y3, y2, y1, y0] of row1
    VLD2.8    {d6, d7},  [\interleavedBlock], \rowIncrement
                                          @ d6  = [x7, x6, x5, x4, x3, x2, x1, x0] of row2
                                          @ d7  = [y7, y6, y5, y4, y3, y2, y1, y0] of row2
    VLD2.8    {d8, d9},  [\interleavedBlock], \rowIncrement
                                          @ d8  = [x7, x6, x5, x4, x3, x2, x1, x0] of row3
                                          @ d9  = [y7, y6, y5, y4, y3, y2, y1, y0] of row3
    /*--------------------------------------------------------------------
     * Zero expand the 8-bit pixel values to 16-bit in q1-q4
     * ------------------------------------------------------------------- */
    VMOVL.U8  q1, d2                      @ d2  = [0x00, x3, 0x00, x2, 0x00, x1, 0x00, x0] of row0
                                          @ d3  = [0x00, x7, 0x00, x6, 0x00, x5, 0x00, x4] of row0
    VMOVL.U8  q2, d4                      @ d4  = [0x00, x3, 0x00, x2, 0x00, x1, 0x00, x0] of row1
                                          @ d5  = [0x00, x7, 0x00, x6, 0x00, x5, 0x00, x4] of row1
    VMOVL.U8  q3, d6                      @ d6  = [0x00, x3, 0x00, x2, 0x00, x1, 0x00, x0] of row2
                                          @ d7  = [0x00, x7, 0x00, x6, 0x00, x5, 0x00, x4] of row2
    VMOVL.U8  q4, d8                      @ d8  = [0x00, x3, 0x00, x2, 0x00, x1, 0x00, x0] of row3
                                          @ d9  = [0x00, x7, 0x00, x6, 0x00, x5, 0x00, x4] of row3

    /*--------------------------------------------------------------------
     * Load the 8-bit pixel values from input pointer in q5-q8
     * ------------------------------------------------------------------- */
    VLD2.8    {d10, d11}, [\interleavedBlock], \rowIncrement
                                          @ d10 = [x7, x6, x5, x4, x3, x2, x1, x0] of row4
                                          @ d11 = [y7, y6, y5, y4, y3, y2, y1, y0] of row4
    VLD2.8    {d12, d13}, [\interleavedBlock], \rowIncrement
                                          @ d12 = [x7, x6, x5, x4, x3, x2, x1, x0] of row5
                                          @ d13 = [y7, y6, y5, y4, y3, y2, y1, y0] of row5
    VLD2.8    {d14, d15}, [\interleavedBlock], \rowIncrement
                                          @ d14 = [x7, x6, x5, x4, x3, x2, x1, x0] of row6
                                          @ d15 = [y7, y6, y5, y4, y3, y2, y1, y0] of row6
    VLD2.8    {d16, d17}, [\interleavedBlock], \rowIncrement
                                          @ d16 = [x7, x6, x5, x4, x3, x2, x1, x0] of row7
                                          @ d17 = [y7, y6, y5, y4, y3, y2, y1, y0] of row7
    /*--------------------------------------------------------------------
     * Zero expand the 8-bit pixel values to 16-bit in q5-q8
     * ------------------------------------------------------------------- */
    VMOVL.U8  q5, d10                     @ d10 = [0x00, x3, 0x00, x2, 0x00, x1, 0x00, x0] of row4
                                          @ d11 = [0x00, x7, 0x00, x6, 0x00, x5, 0x00, x4] of row4
    VMOVL.U8  q6, d12                     @ d12 = [0x00, x3, 0x00, x2, 0x00, x1, 0x00, x0] of row5
                                          @ d13 = [0x00, x7, 0x00, x6, 0x00, x5, 0x00, x4] of row5
    VMOVL.U8  q7, d14                     @ d14 = [0x00, x3, 0x00, x2, 0x00, x1, 0x00, x0] of row6
                                          @ d15 = [0x00, x7, 0x00, x6, 0x00, x5, 0x00, x4] of row6
    VMOVL.U8  q8, d16                     @ d16 = [0x00, x3, 0x00, x2, 0x00, x1, 0x00, x0] of row7
                                          @ d17 = [0x00, x7, 0x00, x6, 0x00, x5, 0x00, x4] of row7

    .endm                                 @ end of LDRInterleavedBlock1

/*=========================================================================
 MACRO               LDRInterleavedBlock1Packed interleavedBlock, rowIncrement

 DESCRIPTION         Load and deinterleave a 8x8 8-bit pixel block and
                     zero expand to 8 x 8 x 16-bit matrix for YUY2/UYVY Format

 REGISTER INPUTS     interleavedBlock - pointer to interleaved pixel block
                                        with each row as
                                        [x0, y0, x1, y1, x2, y2,....]
                     rowIncrement     - row increment to the next line
                                        of the block

 REGISTER OUTPUTS    Q1-Q8 containing the 8 x 8 x 16-bit matrix
                     with each row as [x7, x6, x5, x4, x3, x2, x1, x0],
                     where each 8-bit pixel is expanded as 16-bit

 REGISTERS AFFECTED  interleavedBlock, rowIncrement
                     Q0-Q15
=========================================================================*/
    .macro  LDRInterleavedBlock1Packed interleavedBlock, rowIncrement
    @---------------------------------------------------------------------
    @ Load the 8-bit pixel values from input pointer in q1-q4
    @---------------------------------------------------------------------
    VLD4.8    {d0, d1, d2, d3},  [\interleavedBlock], \rowIncrement
                                          @ d0  = [x7, x6, x5, x4, x3, x2, x1, x0] of row0
                                          @ d1  = Dummy load Luma Values
                                          @ d2  = [y7, y6, y5, y4, y3, y2, y1, y0] of row0
                                          @ d3  = Dummy load Luma Values
    VLD4.8    {d4, d5, d6, d7},  [\interleavedBlock], \rowIncrement
                                          @ d4  = [x7, x6, x5, x4, x3, x2, x1, x0] of row1
                                          @ d5  = Dummy load Luma Values
                                          @ d6  = [y7, y6, y5, y4, y3, y2, y1, y0] of row1
                                          @ d7  = Dummy load Luma Values
    VLD4.8    {d8, d9, d10, d11},  [\interleavedBlock], \rowIncrement
                                          @ d8  = [x7, x6, x5, x4, x3, x2, x1, x0] of row2
                                          @ d9  = Dummy load Luma Values
                                          @ d10 = [y7, y6, y5, y4, y3, y2, y1, y0] of row2
                                          @ d11 = Dummy load Luma Values
    VLD4.8    {d12, d13, d14, d15},  [\interleavedBlock], \rowIncrement
                                          @ d12 = [x7, x6, x5, x4, x3, x2, x1, x0] of row3
                                          @ d13 = Dummy load Luma Values
                                          @ d14 = [y7, y6, y5, y4, y3, y2, y1, y0] of row3
                                          @ d15 = Dummy load Luma Values
    @---------------------------------------------------------------------
    @ Zero expand the 8-bit pixel values to 16-bit in q1-q4
    @---------------------------------------------------------------------
    VMOVL.U8  q1, d0                      @ d2  = [0x00, x3, 0x00, x2, 0x00, x1, 0x00, x0] of row0
                                          @ d3  = [0x00, x7, 0x00, x6, 0x00, x5, 0x00, x4] of row0
    VMOVL.U8  q2, d4                      @ d4  = [0x00, x3, 0x00, x2, 0x00, x1, 0x00, x0] of row1
                                          @ d5  = [0x00, x7, 0x00, x6, 0x00, x5, 0x00, x4] of row1
    VMOVL.U8  q3, d8                      @ d6  = [0x00, x3, 0x00, x2, 0x00, x1, 0x00, x0] of row2
                                          @ d7  = [0x00, x7, 0x00, x6, 0x00, x5, 0x00, x4] of row2
    VMOVL.U8  q4, d12                     @ d8  = [0x00, x3, 0x00, x2, 0x00, x1, 0x00, x0] of row3
                                          @ d9  = [0x00, x7, 0x00, x6, 0x00, x5, 0x00, x4] of row3

    @---------------------------------------------------------------------
    @ Load the 8-bit pixel values from input pointer in q5-q8
    @---------------------------------------------------------------------
    VLD4.8    {d16, d17, d18, d19}, [\interleavedBlock], \rowIncrement
                                          @ d10 = [x7, x6, x5, x4, x3, x2, x1, x0] of row4
                                          @ d18 = Dummy load Luma Values
                                          @ d11 = [y7, y6, y5, y4, y3, y2, y1, y0] of row4
                                          @ d19 = Dummy load Luma Values
    VLD4.8    {d20, d21, d22, d23}, [\interleavedBlock], \rowIncrement
                                          @ d12 = [x7, x6, x5, x4, x3, x2, x1, x0] of row5
                                          @ d18 = Dummy load Luma Values
                                          @ d13 = [y7, y6, y5, y4, y3, y2, y1, y0] of row5
                                          @ d19 = Dummy load Luma Values
    VLD4.8    {d24, d25, d26, d27}, [\interleavedBlock], \rowIncrement
                                          @ d14 = [x7, x6, x5, x4, x3, x2, x1, x0] of row6
                                          @ d18 = Dummy load Luma Values
                                          @ d15 = [y7, y6, y5, y4, y3, y2, y1, y0] of row6
                                          @ d19 = Dummy load Luma Values
    VLD4.8    {d28, d29, d30, d31}, [\interleavedBlock], \rowIncrement
                                          @ d16 = [x7, x6, x5, x4, x3, x2, x1, x0] of row7
                                          @ d18 = Dummy load Luma Values
                                          @ d17 = [y7, y6, y5, y4, y3, y2, y1, y0] of row7
                                          @ d19 = Dummy load Luma Values
    @---------------------------------------------------------------------
    @ Zero expand the 8-bit pixel values to 16-bit in q5-q8
    @---------------------------------------------------------------------
    VMOVL.U8  q5, d16                     @ d10 = [0x00, x3, 0x00, x2, 0x00, x1, 0x00, x0] of row4
                                          @ d11 = [0x00, x7, 0x00, x6, 0x00, x5, 0x00, x4] of row4
    VMOVL.U8  q6, d20                     @ d12 = [0x00, x3, 0x00, x2, 0x00, x1, 0x00, x0] of row5
                                          @ d13 = [0x00, x7, 0x00, x6, 0x00, x5, 0x00, x4] of row5
    VMOVL.U8  q7, d24                     @ d14 = [0x00, x3, 0x00, x2, 0x00, x1, 0x00, x0] of row6
                                          @ d15 = [0x00, x7, 0x00, x6, 0x00, x5, 0x00, x4] of row6
    VMOVL.U8  q8, d28                     @ d16 = [0x00, x3, 0x00, x2, 0x00, x1, 0x00, x0] of row7
                                          @ d17 = [0x00, x7, 0x00, x6, 0x00, x5, 0x00, x4] of row7
    .endm                                 @ end of LDRInterleavedBlock1Packed

/*========================================================================

 MACRO               LDRInterleavedBlock2 interleavedBlock, rowIncrement

 DESCRIPTION         Load and deinterleave a 8x8 8-bit pixel block and
                     zero expand to 8 x 8 x 16-bit matrix

 REGISTER INPUTS     interleavedBlock - pointer to interleaved pixel block
                                        with each row as
                                        [x0, y0, x1, y1, x2, y2,....]
                     rowIncrement     - row increment to the next line
                                        of the block

 REGISTER OUTPUTS    Q1-Q8 containing the 8 x 8 x 16-bit matrix
                     with each row as [y7, y6, y5, y4, y3, y2, y1, y0],
                     where each 8-bit pixel is expanded as 16-bit

 REGISTERS AFFECTED  interleavedBlock, rowIncrement
                     Q1-Q8

========================================================================== */
    .macro  LDRInterleavedBlock2 interleavedBlock, rowIncrement

    /*--------------------------------------------------------------------
     * Load the 8-bit pixel values from input pointer q1-q4
     * ------------------------------------------------------------------- */
    VLD2.8    {d2, d3},  [\interleavedBlock], \rowIncrement
                                          @ d2  = [x7, x6, x5, x4, x3, x2, x1, x0] of row0
                                          @ d3  = [y7, y6, y5, y4, y3, y2, y1, y0] of row0
    VLD2.8    {d4, d5},  [\interleavedBlock], \rowIncrement
                                          @ d4  = [x7, x6, x5, x4, x3, x2, x1, x0] of row1
                                          @ d5  = [y7, y6, y5, y4, y3, y2, y1, y0] of row1
    VLD2.8    {d6, d7},  [\interleavedBlock], \rowIncrement
                                          @ d6  = [x7, x6, x5, x4, x3, x2, x1, x0] of row2
                                          @ d7  = [y7, y6, y5, y4, y3, y2, y1, y0] of row2
    VLD2.8    {d8, d9},  [\interleavedBlock], \rowIncrement
                                          @ d8  = [x7, x6, x5, x4, x3, x2, x1, x0] of row3
                                          @ d9  = [y7, y6, y5, y4, y3, y2, y1, y0] of row3
    /*--------------------------------------------------------------------
     * Zero expand the 8-bit pixel values to 16-bit in q1-q4
     * ------------------------------------------------------------------- */
    VMOVL.U8  q1, d3                      @ d2  = [0x00, y3, 0x00, y2, 0x00, y1, 0x00, y0] of row0
                                          @ d3  = [0x00, y7, 0x00, y6, 0x00, y5, 0x00, y4] of row0
    VMOVL.U8  q2, d5                      @ d4  = [0x00, y3, 0x00, y2, 0x00, y1, 0x00, y0] of row1
                                          @ d5  = [0x00, y7, 0x00, y6, 0x00, y5, 0x00, y4] of row1
    VMOVL.U8  q3, d7                      @ d6  = [0x00, y3, 0x00, y2, 0x00, y1, 0x00, y0] of row2
                                          @ d7  = [0x00, y7, 0x00, y6, 0x00, y5, 0x00, y4] of row2
    VMOVL.U8  q4, d9                      @ d8  = [0x00, y3, 0x00, y2, 0x00, y1, 0x00, y0] of row3
                                          @ d9  = [0x00, y7, 0x00, y6, 0x00, y5, 0x00, y4] of row3

    /*--------------------------------------------------------------------
     * Load the 8-bit pixel values from input pointer in q5-q8
     * ------------------------------------------------------------------- */
    VLD2.8    {d10, d11}, [\interleavedBlock], \rowIncrement
                                          @ d10 = [x7, x6, x5, x4, x3, x2, x1, x0] of row4
                                          @ d11 = [y7, y6, y5, y4, y3, y2, y1, y0] of row4
    VLD2.8    {d12, d13}, [\interleavedBlock], \rowIncrement
                                          @ d12 = [x7, x6, x5, x4, x3, x2, x1, x0] of row5
                                          @ d13 = [y7, y6, y5, y4, y3, y2, y1, y0] of row5
    VLD2.8    {d14, d15}, [\interleavedBlock], \rowIncrement
                                          @ d14 = [x7, x6, x5, x4, x3, x2, x1, x0] of row6
                                          @ d15 = [y7, y6, y5, y4, y3, y2, y1, y0] of row6
    VLD2.8    {d16, d17}, [\interleavedBlock], \rowIncrement
                                          @ d16 = [x7, x6, x5, x4, x3, x2, x1, x0] of row7
                                          @ d17 = [y7, y6, y5, y4, y3, y2, y1, y0] of row7
    /*--------------------------------------------------------------------
     * Zero expand the 8-bit pixel values to 16-bit in q5-q8
     * ------------------------------------------------------------------- */
    VMOVL.U8  q5, d11                     @ d10 = [0x00, y3, 0x00, y2, 0x00, y1, 0x00, y0] of row4
                                          @ d11 = [0x00, y7, 0x00, y6, 0x00, y5, 0x00, y4] of row4
    VMOVL.U8  q6, d13                     @ d12 = [0x00, y3, 0x00, y2, 0x00, y1, 0x00, y0] of row5
                                          @ d13 = [0x00, y7, 0x00, y6, 0x00, y5, 0x00, y4] of row5
    VMOVL.U8  q7, d15                     @ d14 = [0x00, y3, 0x00, y2, 0x00, y1, 0x00, y0] of row6
                                          @ d15 = [0x00, y7, 0x00, y6, 0x00, y5, 0x00, y4] of row6
    VMOVL.U8  q8, d17                     @ d16 = [0x00, y3, 0x00, y2, 0x00, y1, 0x00, y0] of row7
                                          @ d17 = [0x00, y7, 0x00, y6, 0x00, y5, 0x00, y4] of row7

    .endm                                 @ end of LDRInterleavedBlock2

/*=========================================================================
 MACRO               LDRInterleavedBlock2Packed interleavedBlock, rowIncrement

 DESCRIPTION         Load and deinterleave a 8x8 8-bit pixel block and
                     zero expand to 8 x 8 x 16-bit matrix for YUY2/UYVY Formats

 REGISTER INPUTS     interleavedBlock - pointer to interleaved pixel block
                                        with each row as
                                        [x0, y0, x1, y1, x2, y2,....]
                     rowIncrement     - row increment to the next line
                                        of the block

 REGISTER OUTPUTS    Q1-Q8 containing the 8 x 8 x 16-bit matrix
                     with each row as [y7, y6, y5, y4, y3, y2, y1, y0],
                     where each 8-bit pixel is expanded as 16-bit

 REGISTERS AFFECTED  interleavedBlock, rowIncrement
                     Q0-Q15
=========================================================================*/
    .macro  LDRInterleavedBlock2Packed interleavedBlock, rowIncrement
    @---------------------------------------------------------------------
    @ Load the 8-bit pixel values from input pointer q1-q4
    @---------------------------------------------------------------------
    VLD4.8    {d0, d1, d2, d3},  [\interleavedBlock], \rowIncrement
                                          @ d2  = [x7, x6, x5, x4, x3, x2, x1, x0] of row0
                                          @ d18 = dummy load Luma Values
                                          @ d3  = [y7, y6, y5, y4, y3, y2, y1, y0] of row0
                                          @ d19 = dummy load Luma Values
    VLD4.8    {d4, d5, d6, d7},  [\interleavedBlock], \rowIncrement
                                          @ d4  = [x7, x6, x5, x4, x3, x2, x1, x0] of row1
                                          @ d18 = dummy load Luma Values
                                          @ d5  = [y7, y6, y5, y4, y3, y2, y1, y0] of row1
                                          @ d19 = dummy load Luma Values
    VLD4.8    {d8, d9, d10, d11},  [\interleavedBlock], \rowIncrement
                                          @ d6  = [x7, x6, x5, x4, x3, x2, x1, x0] of row2
                                          @ d18 = dummy load Luma Values
                                          @ d7  = [y7, y6, y5, y4, y3, y2, y1, y0] of row2
                                          @ d19 = dummy load Luma Values
    VLD4.8    {d12, d13, d14, d15},  [\interleavedBlock], \rowIncrement
                                          @ d8  = [x7, x6, x5, x4, x3, x2, x1, x0] of row3
                                          @ d18 = dummy load Luma Values
                                          @ d9  = [y7, y6, y5, y4, y3, y2, y1, y0] of row3
                                          @ d19 = dummy load Luma Values
    @---------------------------------------------------------------------
    @ Zero expand the 8-bit pixel values to 16-bit in q1-q4
    @---------------------------------------------------------------------
    VMOVL.U8  q1, d2                      @ d2  = [0x00, y3, 0x00, y2, 0x00, y1, 0x00, y0] of row0
                                          @ d3  = [0x00, y7, 0x00, y6, 0x00, y5, 0x00, y4] of row0
    VMOVL.U8  q2, d6                      @ d4  = [0x00, y3, 0x00, y2, 0x00, y1, 0x00, y0] of row1
                                          @ d5  = [0x00, y7, 0x00, y6, 0x00, y5, 0x00, y4] of row1
    VMOVL.U8  q3, d10                     @ d6  = [0x00, y3, 0x00, y2, 0x00, y1, 0x00, y0] of row2
                                          @ d7  = [0x00, y7, 0x00, y6, 0x00, y5, 0x00, y4] of row2
    VMOVL.U8  q4, d14                     @ d8  = [0x00, y3, 0x00, y2, 0x00, y1, 0x00, y0] of row3
                                          @ d9  = [0x00, y7, 0x00, y6, 0x00, y5, 0x00, y4] of row3

    @---------------------------------------------------------------------
    @ Load the 8-bit pixel values from input pointer in q5-q8
    @---------------------------------------------------------------------
    VLD4.8    {d16, d17, d18, d19}, [\interleavedBlock], \rowIncrement
                                          @ d10 = [x7, x6, x5, x4, x3, x2, x1, x0] of row4
                                          @ d18 = dummy load Luma Values
                                          @ d11 = [y7, y6, y5, y4, y3, y2, y1, y0] of row4
                                          @ d19 = dummy load Luma Values
    VLD4.8    {d20, d21, d22, d23}, [\interleavedBlock], \rowIncrement
                                          @ d12 = [x7, x6, x5, x4, x3, x2, x1, x0] of row5
                                          @ d18 = dummy load Luma Values
                                          @ d13 = [y7, y6, y5, y4, y3, y2, y1, y0] of row5
                                          @ d19 = dummy load Luma Values
    VLD4.8    {d24, d25, d26, d27}, [\interleavedBlock], \rowIncrement
                                          @ d14 = [x7, x6, x5, x4, x3, x2, x1, x0] of row6
                                          @ d18 = dummy load Luma Values
                                          @ d15 = [y7, y6, y5, y4, y3, y2, y1, y0] of row6
                                          @ d19 = dummy load Luma Values
    VLD4.8    {d28, d29, d30, d31}, [\interleavedBlock], \rowIncrement
                                          @ d16 = [x7, x6, x5, x4, x3, x2, x1, x0] of row7
                                          @ d18 = dummy load Luma Values
                                          @ d17 = [y7, y6, y5, y4, y3, y2, y1, y0] of row7
                                          @ d19 = dummy load Luma Values
    @---------------------------------------------------------------------
    @ Zero expand the 8-bit pixel values to 16-bit in q5-q8
    @---------------------------------------------------------------------
    VMOVL.U8  q5, d18                     @ d10 = [0x00, y3, 0x00, y2, 0x00, y1, 0x00, y0] of row4
                                          @ d11 = [0x00, y7, 0x00, y6, 0x00, y5, 0x00, y4] of row4
    VMOVL.U8  q6, d22                     @ d12 = [0x00, y3, 0x00, y2, 0x00, y1, 0x00, y0] of row5
                                          @ d13 = [0x00, y7, 0x00, y6, 0x00, y5, 0x00, y4] of row5
    VMOVL.U8  q7, d26                     @ d14 = [0x00, y3, 0x00, y2, 0x00, y1, 0x00, y0] of row6
                                          @ d15 = [0x00, y7, 0x00, y6, 0x00, y5, 0x00, y4] of row6
    VMOVL.U8  q8, d30                     @ d16 = [0x00, y3, 0x00, y2, 0x00, y1, 0x00, y0] of row7
                                          @ d17 = [0x00, y7, 0x00, y6, 0x00, y5, 0x00, y4] of row7
    .endm                                 @ end of LDRInterleavedBlock2Packed

/*------------------------------------------------------------------------
 *                          Variables/Const tables
 * ----------------------------------------------------------------------- */

    .section jpeg_fetch_dct, "x"          @ AREA
    .text                                 @ |.text|, CODE, READONLY
    .align 2
    .code  32                             @ CODE32

dct_kernel_table_transposed:              @ transpose of DCT kernel table

    .hword (CC0), (CC1),  (CC2),  (CC3),  (CC4),  (CC5),  (CC6),  (CC7)
                                          @ c00 c10 c20 c30 c40 c50 c60 c70
    .hword (CC0), (CC3),  (CC6),  (-CC7), (-CC4), (-CC1), (-CC2), (-CC5)
                                          @ c00 c11 c21 c31 c41 c51 c61 c71
    .hword (CC0), (CC5),  (-CC6), (-CC1), (-CC4), (CC7),  (CC2),  (CC3)
                                          @ c00 c12 c22 c32 c42 c52 c62 c72
    .hword (CC0), (CC7),  (-CC2), (-CC5), (CC4),  (CC3),  (-CC6), (-CC1)
                                          @ c00 c13 c23 c33 c43 c53 c63 c73

/*------------------------------------------------------------------------
 *                          Function Definitions
 * ----------------------------------------------------------------------- */

/*========================================================================

 FUNCTION            : jpege_engine_sw_fdct_block

 DESCRIPTION         : Perform the forward DCT on a 8x8 block of pixels.

 C PROTOTYPE         : void jpege_engine_sw_fdct_block (
                         const uint8 *pixelBlock,
                               int16 *dctOutput)

 REGISTER INPUTS     : R0: uint8 *pixelBlock
                             pointer of the input 8x8 pixels
                       R1: int16 *dctOutput
                             pointer of the output 8x8 DCT results

 STACK ARGUMENTS     : None

 REGISTER OUTPUTS    : None

 MEMORY INPUTS       : pixelBlock - pointer to 8x8 pixels

 MEMORY OUTPUTS      : dctOutput - pointer to 8x8 DCT results in Q3

 REGISTERS AFFECTED  : R0-R2, R12, Q0-Q15
                       Q1-Q8 : DCT results transposed
                       Q9-Q12: DCT kernel from MSHW to LSHW
                       R2, R12, Q0, Q13-Q15 are used as scratch registers

 STACK USAGE         : None

 CYCLES              : 437

 NOTES               :

========================================================================== */
.type jpege_engine_sw_fdct_block, %function
jpege_engine_sw_fdct_block:

    pixelBlock            .req r0
    dctOutput             .req r1
    dc                    .req r2
    eight                 .req r3

    VPUSH     {q0-q7}
    VPUSH     {q8-q15}

    MOV eight, #8
    LDRDCTKernel                          @ load DCT kernel into q9-q12
    LDRPixelBlock pixelBlock, eight       @ load pixle values into q1-q8
    DCT2D dc, dctOutput                   @ DCT in q1-q8 and save to dctOutput

    VPOP      {q8-q15}
    VPOP      {q0-q7}
    BX        lr
                                          @ end of jpege_engine_sw_fdct_block

/*========================================================================

 FUNCTION            : jpege_engine_sw_fetch_dct_block_luma

 DESCRIPTION         : Perform the forward DCT on a 8x8 block of luma.

 C PROTOTYPE         : void jpege_engine_sw_fetch_dct_block_luma (
                         const uint8 *lumaBlock,
                               int16 *lumaDctOutput,
                               uint32 lumaWidth)

 REGISTER INPUTS     : R0: uint8 *lumaBlock
                             pointer of luma block from the input image
                       R1: int16 *lumaDctOutput
                             pointer of output 8x8 DCT results
                       R2: uint32 lumaWidth
                             luma width of the input image

 STACK ARGUMENTS     : None

 REGISTER OUTPUTS    : None

 MEMORY INPUTS       : lumaBlock - pointer to luma block

 MEMORY OUTPUTS      : lumaDctOutput - pointer to 8x8 DCT results in Q3

 REGISTERS AFFECTED  : R0-R3, R12, Q0-Q15
                       Q1-Q8 : DCT results transposed
                       Q9-Q12: DCT kernel from MSHW to LSHW
                       R3, R12, Q0, Q13-Q15 are used as scratch registers

 STACK USAGE         : None

 CYCLES              :

 NOTES               :

========================================================================== */
.type jpege_engine_sw_fetch_dct_block_luma, %function
jpege_engine_sw_fetch_dct_block_luma:

    lumaBlock             .req r0
    lumaDctOutput         .req r1
    lumaRowIncrement      .req r2         @ lumaRowIncrement = lumaWidth
    lumaDC                .req r3         @ DC level shift

    VPUSH     {q0-q7}
    VPUSH     {q8-q15}

    LDRDCTKernel                          @ load DCT kernel into q9-q12

fetch_block_luma:
    LDRPixelBlock lumaBlock, lumaRowIncrement
                                          @ load pixle values into q1-q8

dct_block_luma:
    DCT2D     lumaDC, lumaDctOutput       @ DCT in q1-q8 and save to dctOutput

    VPOP      {q8-q15}
    VPOP      {q0-q7}
    BX        lr
                                          @ end of jpege_engine_sw_fetch_dct_block_luma

/*=========================================================================

 FUNCTION            : jpege_engine_sw_fetch_dct_block_luma_packed

 DESCRIPTION         : Perform the forward DCT on a 8x8 block of YUY2/UYVY luma.

 C PROTOTYPE         : void jpege_engine_sw_fetch_dct_block_luma (
                         const uint8 *lumaBlock,
                               int16 *lumaDctOutput,
                               uint32 lumaWidth)

 REGISTER INPUTS     : R0: uint8 *lumaBlock
                             pointer of luma block from the input image
                       R1: int16 *lumaDctOutput
                             pointer of output 8x8 DCT results
                       R2: uint32 lumaWidth
                             luma width of the input image

 STACK ARGUMENTS     : None

 REGISTER OUTPUTS    : None

 MEMORY INPUTS       : lumaBlock - pointer to luma block

 MEMORY OUTPUTS      : lumaDctOutput - pointer to 8x8 DCT results in Q3

 REGISTERS AFFECTED  : R0-R3, R12, Q0-Q15
                       Q1-Q8 : DCT results transposed
                       Q9-Q12: DCT kernel from MSHW to LSHW
                       R3, R12, Q0, Q13-Q15 are used as scratch registers

 STACK USAGE         : None

 CYCLES              :

 NOTES               :

=========================================================================*/
.type jpege_engine_sw_fetch_dct_block_luma_packed, %function
jpege_engine_sw_fetch_dct_block_luma_packed:

    lumaBlock             .req r0
    lumaDctOutput         .req r1
    lumaRowIncrement      .req r2         @ lumaRowIncrement = lumaWidth
    lumaDC                .req r3         @ DC level shift

    VPUSH     {q0-q7}
    VPUSH     {q8-q15}

    LDRDCTKernel                          @ load DCT kernel into q9-q12

    LSL lumaRowIncrement, #1              @ double lumaRowIncrement for YUY2/UYVY

fetch_block_luma_packed:
    LDRPixelBlockPacked lumaBlock, lumaRowIncrement
                                          @ load pixle values into q1-q8
dct_block_luma_packed:
    DCT2D     lumaDC, lumaDctOutput       @ DCT in q1-q8 and save to dctOutput

    VPOP      {q8-q15}
    VPOP      {q0-q7}
    BX        lr						  @ end of jpege_engine_sw_fetch_dct_block_luma_packed

/*========================================================================

 FUNCTION            : jpege_engine_sw_fetch_dct_block_chroma

 DESCRIPTION         : Perform the forward DCT on a 16x8 block of interleaved
                       chroma.

 C PROTOTYPE         : void jpege_engine_sw_fetch_dct_block_chroma (
                         const uint8 *chromaBlock,
                               int16 *chromaDctOutput,
                               uint32 chromaWidth,
                               uint32 inputFormat)

 REGISTER INPUTS     : R0: uint8 *chromaBlock
                             pointer of chroma interleaved block from the
                             input image
                       R1: int16 *chromaDctOutput
                             pointer of the output 2 8x8 DCT results
                       R2: uint32 chromaWidth
                             chroma width of the input image
                       R3: uint32 inputFormat
                             CbCr - 0
                             CrCb - 1

 STACK ARGUMENTS     : None

 REGISTER OUTPUTS    : None

 MEMORY INPUTS       : chromaBlock - pointer to interleaved chroma block

 MEMORY OUTPUTS      : chromaDctOutput - pointer to 2 8x8 DCT results in Q3

 REGISTERS AFFECTED  : R0-R7, R12, Q0-Q15
                       Q1-Q8 : DCT results transposed
                       Q9-Q12: DCT kernel from MSHW to LSHW
                       R3-R7, R12, Q0, Q13-Q15 are used as scratch registers

 STACK USAGE         : None

 CYCLES              :

 NOTES               :

========================================================================== */
.type jpege_engine_sw_fetch_dct_block_chroma, %function
jpege_engine_sw_fetch_dct_block_chroma:

    chromaBlock           .req r0
    chromaDctOutput       .req r1
    chromaWidth           .req r2
    chromaRowIncrement    .req r2         @ chromaRowIncrement = chromaWidth * 2
    inputFormat           .req r3         @ CbCr - 0, CrCb - 1
    chromaDC              .req r4         @ DC level shift
    chromaBlock1          .req r0         @ same as chromaBlock
    chromaBlock2          .req r5         @ backup of chromaBlock
    chromaDctOutput1      .req r6
    chromaDctOutput2      .req r7

    STMFD SP!, {r4-r7}                    @ PUSH {r4-r7}
    VPUSH     {q0-q7}
    VPUSH     {q8-q15}

    CMP       inputFormat, #0             @ DCT output is always CbCr
    ADDNE     chromaDctOutput1, chromaDctOutput, #(DCTSIZE2 * 2)
                                          @ chromaDctOutput1 (r6) is Cr
                                          @ if (inputFormat == CrCb)
    MOVNE     chromaDctOutput2, chromaDctOutput
                                          @ chromaDctOutput2 (r7) is Cb
                                          @ if (inputFormat == CrCb)
    MOVEQ     chromaDctOutput1, chromaDctOutput
                                          @ chromaDctOutput1 (r6) is Cb
                                          @ if (inputFormat == CbCr)
    ADDEQ     chromaDctOutput2, chromaDctOutput, #(DCTSIZE2 * 2)
                                          @ chromaDctOutput2 (r7) is Cr
                                          @ if (inputFormat == CbCr)

    MOV       chromaBlock2, chromaBlock   @ chromaBlock2 (r5) = chromaBlock (r0)
    MOV       chromaRowIncrement, chromaWidth, LSL #1
                                          @ chromaRowIncrement = chromaWidth * 2

    LDRDCTKernel                          @ load DCT kernel into q9-q12

    /*--------------------------------------------------------------------
     * Load chroma values from chromaBlock and deinterleave, the 1st
     * deinterleaved block is in q1-q8.
     * ------------------------------------------------------------------- */
fetch_block_chroma1:
    LDRInterleavedBlock1 chromaBlock1, chromaRowIncrement

dct_block_chroma1:
    DCT2D     chromaDC, chromaDctOutput1  @ DCT in q1-q8 and save to dctOutput1

    /*--------------------------------------------------------------------
     * Load chroma values from chromaBlock and deinterleave, the 2nd
     * deinterleaved block is in q1-q8.
     * ------------------------------------------------------------------- */
fetch_block_chroma2:
    LDRInterleavedBlock2 chromaBlock2, chromaRowIncrement

dct_block_chroma2:
    DCT2D     chromaDC, chromaDctOutput2  @ DCT in q1-q8 and save to dctOutput2

    VPOP      {q8-q15}
    VPOP      {q0-q7}
    LDMFD SP!, {r4-r7}                    @ POP  {r5-r9}
    BX        lr
                                          @ end of jpege_engine_sw_fetch_dct_block_chroma
/*=========================================================================

 FUNCTION            : jpege_engine_sw_fetch_dct_block_chroma_packed

 DESCRIPTION         : Perform the forward DCT on a 16x8 block of YUY2/UYVY
                       chroma.

 C PROTOTYPE         : void jpege_engine_sw_fetch_dct_block_chroma (
                         const uint8 *chromaBlock,
                               int16 *chromaDctOutput,
                               uint32 chromaWidth,
                               uint32 inputFormat)

 REGISTER INPUTS     : R0: uint8 *chromaBlock
                             pointer of chroma interleaved block from the
                             input image
                       R1: int16 *chromaDctOutput
                             pointer of the output 2 8x8 DCT results
                       R2: uint32 chromaWidth
                             chroma width of the input image
                       R3: uint32 inputFormat
                             CbCr - 0
                             CrCb - 1

 STACK ARGUMENTS     : None

 REGISTER OUTPUTS    : None

 MEMORY INPUTS       : chromaBlock - pointer to interleaved chroma block

 MEMORY OUTPUTS      : chromaDctOutput - pointer to 2 8x8 DCT results in Q3

 REGISTERS AFFECTED  : R0-R7, R12, Q0-Q15
                       Q1-Q8 : DCT results transposed
                       Q9-Q12: DCT kernel from MSHW to LSHW
                       R3-R7, R12, Q0, Q13-Q15 are used as scratch registers

 STACK USAGE         : None

 CYCLES              :

 NOTES               :

=========================================================================*/
.type jpege_engine_sw_fetch_dct_block_chroma_packed, %function
jpege_engine_sw_fetch_dct_block_chroma_packed:

    chromaBlock           .req r0
    chromaDctOutput       .req r1
    chromaWidth           .req r2
    chromaRowIncrement    .req r2         @ chromaRowIncrement = chromaWidth * 2
    inputFormat           .req r3         @ CbCr - 0, CrCb - 1
    chromaDC              .req r4         @ DC level shift
    chromaBlock1          .req r0         @ same as chromaBlock
    chromaBlock2          .req r5         @ backup of chromaBlock
    chromaDctOutput1      .req r6
    chromaDctOutput2      .req r7
    STMFD SP!, {r4-r7}                    @ PUSH {r4-r7}
    VPUSH     {q0-q7}
    VPUSH     {q8-q15}

    MOV    chromaDctOutput1, chromaDctOutput
                                          @ chromaDctOutput1 (r6) is Cb
                                          @ if (inputFormat == CbCr)
    ADD    chromaDctOutput2, chromaDctOutput, #(DCTSIZE2 * 2)
                                          @ chromaDctOutput2 (r7) is Cr
                                          @ if (inputFormat == CbCr)

    MOV    chromaBlock2, chromaBlock      @ chromaBlock2 (r5) = chromaBlock (r0)
    MOV    chromaRowIncrement, chromaWidth, LSL #2
                                          @ chromaRowIncrement = chromaWidth * 4
    @---------------------------------------------------------------------
    @ Load chroma values from chromaBlock and deinterleave, the 1st
    @ deinterleaved block is in q1-q8.
    @---------------------------------------------------------------------
fetch_block_chroma1_packed:
    LDRInterleavedBlock1Packed chromaBlock1, chromaRowIncrement
                                          @ will corrupt q0-q15

    LDRDCTKernel                          @ load DCT kernel into q9-q12

dct_block_chroma1_packed:
    DCT2D     chromaDC, chromaDctOutput1  @ DCT in q1-q8 and save to dctOutput1

    @---------------------------------------------------------------------
    @ Load chroma values from chromaBlock and deinterleave, the 2nd
    @ deinterleaved block is in q1-q8.
    @---------------------------------------------------------------------
fetch_block_chroma2_packed:
    LDRInterleavedBlock2Packed chromaBlock2, chromaRowIncrement
                                          @ will corrupt q0-q15

    LDRDCTKernel                          @ load DCT kernel into q9-q12

dct_block_chroma2_packed:
    DCT2D     chromaDC, chromaDctOutput2  @ DCT in q1-q8 and save to dctOutput2

    VPOP      {q8-q15}
    VPOP      {q0-q7}
    LDMFD SP!, {r4-r7}                    @ POP  {r5-r9}

    BX        lr						  @ end of jpege_engine_sw_fetch_dct_block_chroma_packed

/*========================================================================

 FUNCTION            : jpege_engine_sw_fdct_u8

 DESCRIPTION         : Perform the forward DCT on a 8x8 block of pixels.

 C PROTOTYPE         : void jpege_engine_sw_fdct_u8 (
                         const uint8 *pixelBlock,
                               int16 *dctOutput,
                               uint32_t  srcStride,
                               uint32_t  dstStride)

 REGISTER INPUTS     : R0: uint8 *pixelBlock
                             pointer of the input 8x8 pixels
                       R1: int16 *dctOutput
                             pointer of the output 8x8 DCT results
                       R2: uint32_t srcStride
                             source stride in number of bytes
                       R3: uint32_t dstStride
                             destination stride in number of bytes

 STACK ARGUMENTS     : None

 REGISTER OUTPUTS    : None

 MEMORY INPUTS       : pixelBlock - pointer to 8x8 pixels

 MEMORY OUTPUTS      : dctOutput - pointer to 8x8 DCT results in Q3

 REGISTERS AFFECTED  : R0-R3, R12, Q0-Q15
                       Q1-Q8 : DCT results transposed
                       Q9-Q12: DCT kernel from MSHW to LSHW
                       R4, R12, Q0, Q13-Q15 are used as scratch registers

 STACK USAGE         : None

 CYCLES              : 437

 NOTES               :

========================================================================== */
.type jpege_engine_sw_fdct_u8, %function
jpege_engine_sw_fdct_u8:

    pixelBlock            .req r0
    dctOutput             .req r1
    srcStride             .req r2
    dstStride             .req r3

    STMFD SP!,{R4, LR}
    VPUSH     {q0-q7}
    VPUSH     {q8-q15}

    LDRDCTKernel                              @ load DCT kernel into q9-q12
    LDRPixelBlock pixelBlock, srcStride       @ load pixle values into q1-q8
    DCT2D_dstStride R4, dctOutput, dstStride  @ DCT in q1-q8 and save to dctOutput

    VPOP      {q8-q15}
    VPOP      {q0-q7}
    LDMFD SP!,{R4, PC}
                                          @ end of jpege_engine_sw_fdct_u8
.end                                      @ end of jpege_engine_sw_fetch_dct.S

