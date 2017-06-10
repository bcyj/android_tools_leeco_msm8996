/*============================================================================
  @file fixed_point.c

  Fixed point utility source file

  Copyright (c) 2010-2011 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Qualcomm Technologies Confidential and Proprietary
============================================================================*/

/* $Header: //source/qcom/qct/core/sensors/dsps/common/main/latest/util/mathtools/src/fixed_point.c#3 $ */
/* $DateTime: 2011/04/29 10:33:24 $ */

/*============================================================================
  EDIT HISTORY

  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.

  when        who  what, where, why
  ----------  ---  -----------------------------------------------------------
  2011-04-29  agk  Removed compiler warning for function isResultOK
  2011-02-28  jtl  Added function comments.
  2011-01-24  ad   initial version

============================================================================*/

#include "fixed_point.h"


#ifdef CODE_PROFILING
extern int32_t isResultOK( void );
#else
static int32_t isResultOK( void )
{
  return 1;
}
#endif


/*===========================================================================

  FUNCTION: sqrtFxQ16

  ===========================================================================*/
/*!
  @brief Computes the square root.

  @param[i] val: Q16 value of which to compute the square root.

  @return
  Q16 representation of the square root.

*/
/*=========================================================================*/
uint32_t sqrtFxQ16(uint32_t val)
{
  uint32_t sqrtVal = 0, temp;
  int32_t i;

  for (i=0; i<32; i+=2)
  {
    temp = (0x40000000l >> i);
    if((temp + sqrtVal) <= val)
    {
      val -= (temp + sqrtVal);
      sqrtVal = ((sqrtVal >> 1) | temp);
    }
    else
    {
      sqrtVal = (sqrtVal >> 1);
    }
  }

  sqrtVal += (sqrtVal<val);

  return (sqrtVal << (FX_QFACTOR>>1));
}

/* arcTan[i] and r[i] = K[i]/K tables for i = 0:16 needed for CORDIC rotations */
static const uint16_t arctanTableQ16[17] = {0xC910,0x76B2,0x3EB7,0x1FD6,0x0FFB,0x07FF,
                                            0x0400,0x0200,0x0100,0x0080,0x0040,0x0020,
                                            0x0010,0x0008,0x0004,0x0002,0x0001};
static const uint32_t rTableQ16[17]      = {0x9B75,0xDBD9,0xF5CC,0xFD5D,0xFF56,0xFFD5,
                                            0xFFF5,0xFFFD,0xFFFF,0x10000,0x10000,0x10000,
                                            0x10000,0x10000,0x10000,0x10000};

/* constant 1/K = 1/1.6468 needed for CORDIC rotations */
#define FX_INVK_Q16 (0x9B75)

/*===========================================================================

  FUNCTION: cordic

  ===========================================================================*/
/*!
  @brief A COordinate Rotation DIgital Computer implementaion used to compute
         trigonometric functions.

  @param[io] x:

  @param[io] y:

  @param[io] z:

  @param[i] vecmode:

  @return
  None.

  @note
*/
/*=========================================================================*/
static void cordic(int32_t* x0, int32_t* y0, int32_t* z0, int32_t vecmode)
{
  int32_t x, y, z, x_old, i, temp, neg;

  x = *x0;
  y = *y0;
  z = *z0;

  for (i = 0; i < FX_QFACTOR; i++)
  {
    temp = FX_MUL_Q16(rTableQ16[i], vecmode);
    neg = (temp >= 0 && y < temp) || (temp < 0 && z >= 0);

    x_old = x;
    x += FX_NEG((y>>i),neg);
    y -= FX_NEG((x_old>>i),neg);
    z += FX_NEG(arctanTableQ16[i],neg);
  }

  *x0 = x;
  *y0 = y;
  *z0 = z;
}

/*===========================================================================

  FUNCTION: aSinCordic

  ===========================================================================*/
/*!
  @brief

  @param[i] sinTheta:

  @return

  @note
*/
/*=========================================================================*/
static int32_t aSinCordic(int32_t sinTheta)
{
  int32_t x, y, z;

  /* does not converge near sin_theta=1, so use
     2nd order approximation in this region
     0xFFB6 (Q16) = 0.998871 (decimal) */
  if (sinTheta >= 0xFFB6)
  {
    return FX_PIOVER2_Q16 - sqrtFxQ16((FX_ONE_Q16-sinTheta)<<1);
  }

  x = FX_INVK_Q16;
  y = 0;
  z = 0;

  cordic(&x, &y, &z, sinTheta);
  return FX_NEG(z, (sinTheta > 0));
}

/*===========================================================================

  FUNCTION: aTan2Cordic

  ===========================================================================*/
/*!
  @brief

  @param[i] im:

  @param[i] re:

  @return
  Arctangent.

  @note
*/
/*=========================================================================*/
static int32_t aTan2Cordic(int32_t im, int32_t re)
{
  int32_t x, y, z;

  if (re == 0 || FX_DIV_Q16(im, re) > 0x4dba76d5)
  {
    return FX_PIOVER2_Q16;
  }
  else if (im == 0 || FX_DIV_Q16(re, im) > 0x4dba76d5)
  {
    return 0;
  }

  x = re;
  y = im;
  z = 0;

  cordic(&x, &y, &z, 0);
  return z;
}

/*===========================================================================

  FUNCTION: sinCosCordic

  ===========================================================================*/
/*!
  @brief Computes both the sine and cosine.

  @param[i] a:

  @param[o] sinp:

  @param[o]: cosp:

  @return
  None.

  @note
*/
/*=========================================================================*/
static void sinCosCordic(int32_t a, int32_t* sinp, int32_t* cosp)
{
  int32_t z = a;
  *sinp = 0;
  *cosp = FX_INVK_Q16;
  cordic(cosp, sinp, &z, -FX_ONE_Q16);
}

/*===========================================================================

  FUNCTION: sinCosTanFxQ16

  ===========================================================================*/
/*!
  @brief

  @param[i] theta:

  @param[o] sin:

  @param[o] cos:

  @param[o] tan:

  @return
  None.

  @note
*/
/*=========================================================================*/
void sinCosTanFxQ16(int32_t theta, int32_t *sin, int32_t *cos, int32_t *tan)
{
  int32_t theta_sign, neg_sin, neg_cos;
  int32_t sin_temp, cos_temp;

  /* trivial check */
  if(theta == 0)
  {
    if (sin)
    {
      *sin = 0;
    }

    if (cos)
    {
      *cos = FX_ONE_Q16;
    }

    if (tan)
    {
      *tan = 0;
    }

    return;
  }

  /* get theta on [0, PI/2] and determine quadrant */
  theta_sign = (theta < 0);
  theta = FX_ABS(theta%FX_2PI_Q16);

  if(theta < FX_PIOVER2_Q16)
  {
    neg_sin = theta_sign;
    neg_cos = 0;
  }
  else if(theta < FX_PI_Q16)
  {
    neg_sin = theta_sign;
    neg_cos = 1;
    theta   = FX_PI_Q16-theta;
  }
  else if(theta < FX_3PIOVER2_Q16)
  {
    neg_sin = !theta_sign;
    neg_cos = 1;
    theta   -= FX_PI_Q16;
  }
  else
  {
    neg_sin = !theta_sign;
    neg_cos = 0;
    theta   = FX_2PI_Q16-theta;
  }

  sinCosCordic(theta, &sin_temp, &cos_temp);

  /* map sin and cos back to proper quadrant */
  if (sin)
  {
    *sin = FX_NEG(sin_temp, neg_sin);
  }

  if (cos)
  {
    *cos = FX_NEG(cos_temp, neg_cos);
  }

  if (tan)
  {
    /* compute tan = sin/cos */
    *tan = (cos_temp == 0) ? FX_NEG(FX_MAX_Q16, (sin_temp < 0)) : FX_DIV_Q16(sin_temp, cos_temp);
  }
}

/*===========================================================================

  FUNCTION: arcTan2FxQ16

  ===========================================================================*/
/*!
  @brief

  @param[i] num:

  @param[i] den:

  @return

  @note
*/
/*=========================================================================*/
int32_t arcTan2FxQ16(int32_t num, int32_t den)
{
  int32_t a, den_abs, num_abs;
  den_abs = FX_ABS(den);
  num_abs = FX_ABS(num);

  a = (den_abs > num_abs) ? aTan2Cordic(num_abs, den_abs) : (FX_PIOVER2_Q16 - aTan2Cordic(den_abs, num_abs));
  return (den < 0) ? (FX_PI_Q16 - FX_NEG(a, (num < 0))) : (FX_NEG(a, (num < 0)));
}

/*===========================================================================

  FUNCTION: arcCosFxQ16

  ===========================================================================*/
/*!
  @brief

  @param[i] cosTheta:

  @return

  @note
*/
/*=========================================================================*/
int32_t arcCosFxQ16(int32_t cosTheta)
{
  return FX_PIOVER2_Q16 - FX_NEG(aSinCordic(FX_ABS(cosTheta)), (cosTheta < 0));
}

/*===========================================================================

  FUNCTION: arcSinFxQ16

  ===========================================================================*/
/*!
  @brief

  @param[i] sinTheta:

  @return

  @note
*/
/*=========================================================================*/
int32_t arcSinFxQ16(int32_t sinTheta)
{
  return FX_NEG(aSinCordic(FX_ABS(sinTheta)), (sinTheta < 0));
}

/*===========================================================================

  FUNCTION: addFxQ16

  ===========================================================================*/
/*!
  @brief

  @param[o] result:

  @param[i] x:

  @param[i] y:

  @return

  @note
*/
/*=========================================================================*/
int32_t addFxQ16(int32_t *result, int32_t x, int32_t y)
{
  *result = x + y;
  return isResultOK();
}

/*===========================================================================

  FUNCTION:

  ===========================================================================*/
/*!
  @brief subFxQ16

  @param[o] result:

  @param[i] x:

  @param[i] y:

  @return

  @note
*/
/*=========================================================================*/
int32_t subFxQ16(int32_t *result, int32_t x, int32_t y)
{
  *result = x - y;
  return isResultOK();
}

/*===========================================================================

  FUNCTION: mulFxQ16

  ===========================================================================*/
/*!
  @brief

  @param[o] result:

  @param[i] x:

  @param[i] y:

  @return

  @note
*/
/*=========================================================================*/
int32_t mulFxQ16(int32_t *result, int32_t x, int32_t y)
{
  int64_t result64 = ((int64_t) x) * ((int64_t) y);
  *result = (int32_t) result64;
  return (((int32_t) ((result64 >> 32) & 0xFFFFFFFF)) == (((int32_t) (result64 & 0xFFFFFFFF))>>31));
}
