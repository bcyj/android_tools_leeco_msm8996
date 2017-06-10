/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __MATRIX_UTILS_H__
#define __MATRIX_UTILS_H__

#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>


void eye(int n, double **x);
void multmat(double **m1, int r1, int c1, double **m2, int r2, int c2,
  double **z);
void transmat(double **x, int r, int c, double **xt);
void eigen(double **x, double **v, double *l, int n);

#endif /* __MATRIX_UTILS_H__ */
