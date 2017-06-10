/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __DEMOSAIC_ABF_COMMON_H__
#define __DEMOSAIC_ABF_COMMON_H__

#define ABF2_CUTOFF1(c1) MAX(17, c1)
#define ABF2_CUTOFF2(c1, c2) MAX((c1-1), c2)
#define ABF2_CUTOFF3(c2, c3) MAX((c2+9), c3)
#define ABF2_MULT_NEG(c2, c3) FLOAT_TO_Q(12, 8.0/(c3 - c2))
#define ABF2_MULT_POS(c1) FLOAT_TO_Q(12, 16.0/c1)
#define ABF2_LUT(v) MIN(2047, MAX(-2047, (FLOAT_TO_Q(11, v))))
#define ABF2_SP_KERNEL(v) MIN(64, MAX(1, FLOAT_TO_Q(6, v)))

#endif //__DEMOSAIC_ABF40_H__
