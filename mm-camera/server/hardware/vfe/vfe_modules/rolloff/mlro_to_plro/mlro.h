/*==============================================================================
   Copyright (c) 2011 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
==============================================================================*/

/*==============================================================================
 * Function:           subspaceoptim
 *
 * Description: Subroutine for optimizinig set of linear bases and
 *              and coefficients to approximate mesh lens roll-off data.
 * Parameters: double** x             ---> roll-off data
 *             int n                  ---> number of roll-off tables
 *             int c                  ---> horizontal dimension of table
 *             int r                  ---> vertical dimension of table
 *             int nbases             ---> number of PCA bases vectors
 *             double** optimcoeffs   ---> optimal set of PCA coeffs
 *             double** optimbases    ---> optimal set of PCA bases
 *             is_bases_known         ---> is bases already known
 *============================================================================*/
int subspaceoptim(double** x, int n, int c, int r, int nbases,
  double** optimcoeffs, double** optimbases, int is_bases_known);
