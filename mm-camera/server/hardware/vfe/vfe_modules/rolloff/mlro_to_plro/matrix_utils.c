/*==============================================================================
   Copyright (c) 2011 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
==============================================================================*/
#include "camera_dbg.h"
#include "matrix_utils.h"

/*==============================================================================
 * Function:           eye
 *
 * Description: constructs an nxn identity matrix
 *============================================================================*/
void eye(int n, double **x)
{
  int i, j;

  for(i = 0; i < n; i++)
    for(j = 0; j < n; j++)
      x[i][j] = 0;

  for(i = 0; i < n; i++)
    x[i][i] = 1.0;

  return;
} /* eye */

/*==============================================================================
 * Function:           multmat
 *
 * Description: multiplies two matrices m1 and m2
 *============================================================================*/
void multmat(double **m1, int r1, int c1, double **m2, int r2, int c2,
  double **z)
{
  int i, j, k;

  CDBG("%s: r1=%d, c1=%d, r2=%d, c2=%d\n", __func__, r1, c1, r2, c2);
  if (c1 != r2) {
    CDBG_ERROR("%s: m1 and m2 not compatible for matrix multiplication.\n",
      __func__);
    return;
  }

  for(i = 0; i < r1; i++) {
    for(j = 0; j < c2; j++) {
      z[i][j]=0;
      for(k = 0; k < c1; k++) {
        z[i][j] += m1[i][k]*m2[k][j];
      }
    }
  }

  return;
} /* multmat */

/*==============================================================================
 * Function:           transmat
 *
 * Description: computes the transpose of a matrix
 *============================================================================*/
void transmat(double **x, int r, int c, double **xt)
{
  int i, j;

  for(i = 0; i < r; i++)
    for(j = 0; j < c; j++)
      xt[j][i] = x[i][j];
} /* transmat */

/*==============================================================================
 * Function:           eigen
 *
 * Description:
 *============================================================================*/
void eigen(double **x, double **v, double *l, int n)
{
  int i, j, k, p;
  double c, s, t, t1, t2;

  eye(n, v);

  for(k = 0; k < 5; k++) {
    for(i = 0; i < n-1; i++) {
      for(j = i+1; j < n; j++) {
        /* parameters for givens rotation */
        if(x[i][j] != 0) {
          t = (x[i][i] - x[j][j]) / (2* x[i][j]);
          if(t>=0)
            t = 1 / (t + sqrt(1 + t*t));
          else
            t = -1/(-t + sqrt(1 + t*t));
          c = 1 / sqrt(1 + t*t);
          s = t * c;
        } else {
          c = 1;
          s = 0;
        }

        /* apply givens rotation */
        for(p = 0; p < n; p++) {
          t1 = x[p][i];
          t2 = x[p][j];
          x[p][i] = (t1 * c) + (t2 * s);
          x[p][j] = (-t1 * s) + (t2 * c);
        }

        for(p = 0; p < n; p++) {
          t1 = x[i][p];
          t2 = x[j][p];
          x[i][p] = (t1 * c) + (t2 * s);
          x[j][p] = (-t1 * s) + (t2 * c);
        }

        /* compute eigen vectors */
        for(p = 0; p < n; p++) {
          t1 = v[p][i];
          t2 = v[p][j];
          v[p][i] = (t1 * c) + (t2 * s);
          v[p][j] = (-t1 * s) + (t2 * c);
        }
      }
    }
  }
  for(i = 0; i < n; i++)
    l[i] = x[i][i];
} /* eigen */
