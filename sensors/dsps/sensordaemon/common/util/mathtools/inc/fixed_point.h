#ifndef FIXED_POINT_H
#define FIXED_POINT_H

/*============================================================================
  @file fixed_point.h

  Fixed point utility header file

  Copyright (c) 2010-2014 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Qualcomm Technologies Confidential and Proprietary
============================================================================*/

/* $Header: //components/dev/ssc.adsp/2.6/gulansey.ssc.adsp.2.6.gulansey_sam31/common/util/mathtools/inc/fixed_point.h#1 $ */
/* $DateTime: 2014/09/30 13:32:03 $ */
/* $Author: gulansey $ */

/*============================================================================
  EDIT HISTORY

  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.

  when        who  what, where, why
  ----------  ---  -----------------------------------------------------------
  2014-07-10  sa   Use single precision for float
  2011-01-24  ad   initial version

============================================================================*/

#include <stdint.h>
#ifdef QDSP6
  #include <hexagon_protos.h>
#endif /* QDSP6 */

#ifdef _WIN32
//suppress warning C4293 on visual studio platform
#pragma warning( disable : 4293 )
#endif /* _WIN32 */

typedef int32_t q16_t;

/* compute the absolute value of a 32-bit integer */
#define FX_ABS(x)            ((x<0)?(-(x)):(x))
#define FX_MIN(x,y)          (((x)<(y))?(x):(y))
#define FX_MAX(x,y)          (((x)>(y))?(x):(y))
#define FX_BOUND(y,x,z)      (FX_MIN(FX_MAX((x),(y)),(z)))
#define FX_NEG(x,flag)       ((flag)?(-(x)):(x))
#define FX_ONE(q)            ((int32_t) 1<<q)
#define FX_HALF(q)           (FX_ONE(q)>>1)
#define FX_SIGN32(x)         (((x)>>31) & 1)

/* generic fixed point macros */
#define FX_FLTTOFIX(f,q)     ((int32_t)( (f)*(1<<(q))+(((f)>(0))?(0.5f):(-0.5f))))
#define FX_FIXTOFLT(i,q)     (((double)(i))/((double)(1<<(q))))
#define FX_FIXTOFLT_SP(i,q)  (((float)(i))/((float)(1<<(q))))  // Single Precision
#define FX_TRUNC(a,q)        ((a)&(0xFFFFFFFF<<(q)))
#define FX_ROUND(a,q)        (FX_TRUNC((a)+FX_HALF(q),q))

/* convert a from q1 format to q2 format */
#define FX_CONV(a,q1,q2)     (((q2)>(q1))?(a)<<((q2)-(q1)):(a)>>((q1)-(q2)))

#define FX_ADD(a,b,q1,q2,q3) (FX_CONV((a),(q1),(q3))+FX_CONV((b),(q2),(q3)))
#define FX_SUB(a,b,q1,q2,q3) (FX_CONV((a),(q1),(q3))-FX_CONV((b),(q2),(q3)))
#define FX_MUL(a,b,q1,q2,q3) ((int32_t)(FX_CONV((((int64_t)(int32_t)(a))*((int64_t)(int32_t)(b))),((q1)+(q2)),(q3))))
#define FX_DIV(a,b,q1,q2,q3) ((int32_t)(FX_CONV(((int64_t)(int32_t)(a)),(q1),(q2)+(q3))/((int64_t)(int32_t)(b))))

/* Q16 fixed point macros */
#define FX_QFACTOR           (16)
#define FX_MAX_Q16           ((int32_t) 0x7FFFFFFF)
#define FX_MIN_Q16           ((int32_t) 0x80000000)

#define FX_FLTTOFIX_Q16(d)   (FX_FLTTOFIX(d,FX_QFACTOR))
#define FX_FIXTOFLT_Q16(a)   (FX_FIXTOFLT(a,FX_QFACTOR))
#define FX_FIXTOFLT_Q16_SP(a) (FX_FIXTOFLT_SP(a,FX_QFACTOR))  // Single Precision
#define FX_TRUNC_Q16(a)      (FX_TRUNC(a,FX_QFACTOR))
#define FX_ROUND_Q16(a)      (FX_ROUND(a,FX_QFACTOR))

/* convert a from q1 format to q2 format */
#define FX_CONV_Q16(a,q1)    (FX_CONV(a,q1,FX_QFACTOR))

#define FX_MUL_Q16(a,b)      ((int32_t)FX_MUL(a,b,FX_QFACTOR,FX_QFACTOR,FX_QFACTOR))
#define FX_DIV_Q16(a,b)      ((int32_t)FX_DIV(a,b,FX_QFACTOR,FX_QFACTOR,FX_QFACTOR))

#define PI                   (3.14159265358979323846f)
#define FX_PIOVER4_Q16       (FX_FLTTOFIX_Q16(0.25f*PI))
#define FX_PIOVER2_Q16       (FX_FLTTOFIX_Q16(0.5f*PI))
#define FX_PI_Q16            (FX_FLTTOFIX_Q16(1.0f*PI))
#define FX_3PIOVER2_Q16      (FX_FLTTOFIX_Q16(1.5f*PI))
#define FX_2PI_Q16           (FX_FLTTOFIX_Q16(2.0f*PI))
#define FX_ONE_Q16           (FX_ONE(FX_QFACTOR))
#define FX_HALF_Q16          (FX_HALF(FX_QFACTOR))

#define G                    (9.80665f) // accel due to gravity at sea level
#define FX_G_Q16             (FX_FLTTOFIX_Q16(G))

uint32_t sqrtFxQ16(uint32_t val);
void sinCosTanFxQ16(int32_t theta, int32_t *sin, int32_t *cos, int32_t *tan);
int32_t arcTan2FxQ16(int32_t num, int32_t den);
int32_t arcCosFxQ16(int32_t cosTheta);
int32_t arcSinFxQ16(int32_t sinTheta);

#endif /* FIXED_POINT_H */

