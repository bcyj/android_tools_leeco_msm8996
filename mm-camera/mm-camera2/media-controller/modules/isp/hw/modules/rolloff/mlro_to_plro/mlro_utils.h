/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/

/*==============================================================================
 * Function:           bubblesort
 *
 * Description: Subroutine for sorting 1-D array elements
 *
 * Parameters: double *x    --->   input one-dimensional array
 *             int n        --->   size of input array
 *             int *m       --->   indices of sorted elements
 *============================================================================*/
#ifndef __MLRO_UTILS_H__
#define __MLRO_UTILS_H__

void bubblesort(double *x, int n, int *m);
double absmax( double *x, int n);

#endif /* __MLRO_UTILS_H__ */
