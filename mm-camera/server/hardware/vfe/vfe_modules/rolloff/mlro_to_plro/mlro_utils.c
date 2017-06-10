/*==============================================================================
   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
==============================================================================*/
#include "camera_dbg.h"
#include "mlro_utils.h"
#ifndef ABS
#define ABS(x)            (((x) < 0) ? -(x) : (x))
#endif
/*==============================================================================
 * Function:           bubblesort
 *
 * Description: Subroutine for sorting 1-D array elements
 *
 * Parameters: double *x    --->   input one-dimensional array
 *             int n        --->   size of input array
 *             int *m       --->   indices of sorted elements
 *============================================================================*/
void bubblesort(double *x, int n, int *m)
{
  int i, j, t1;
  double t2;

  for(i = 0; i < n; i++)
    m[i] = i;

  for(i = 0; i < n; i++) {
    for(j = 0; j <n-1; j++) {
      if(x[j] < x[j+1]) {
        t2 = x[j+1];
        x[j+1] = x[j];
        x[j] = t2;
        t1 = m[j+1];
        m[j+1] = m[j];
        m[j] = t1;
      }
    }
  }
} /* bubblesort */

/*==============================================================================
 * Function:           absmax
 *
 * Description:
 *
 * Parameters: double *x    --->   input one-dimensional array
 *             int n        --->   size of input array
 *============================================================================*/
double absmax(double *x, int n)
{
  int j, *l;
  int index = 0;
  double *y;

  l = (int *)malloc(n * sizeof(int));
  if (NULL == l) {
    free(l);
    CDBG("%s: Error mem alloc for l.\n", __func__);
    return -1;
  }

  y = (double *)malloc(n * sizeof(double));
  if (NULL == y) {
    free(l);
    free(y);
    CDBG("%s: Error mem alloc for y.\n", __func__);
    return -1;
  }
  for(j = 0; j < n; j++)
    y[j] = ABS(x[j]);
  bubblesort(y, n, l);
  index = l[0];

  free(l);
  free(y);
  return ABS(x[index]);
}

