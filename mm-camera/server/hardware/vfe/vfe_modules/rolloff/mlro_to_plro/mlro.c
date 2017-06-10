/*==============================================================================
   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
==============================================================================*/
#include "camera_dbg.h"
#include "mlro.h"
#include "mlro_utils.h"
#include "matrix_utils.h"

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
  double** optimcoeffs, double** optimbases, int is_bases_known)
{
  int i, j, *m = NULL, nch = 4, rc = 0;
  double a, *l = NULL, *s = NULL;
  double **v = NULL, **y = NULL, **z = NULL, *t1 = NULL, *t2 = NULL, *t3 = NULL;
  double cmin[8] = {0., -1.0, -1.0, -0.5, -0.5, -0.25, -0.25, -0.125};
  double cmax[8] = {15.99, 0.99, 0.99, 0.49, 0.49, 0.24, 0.24, 0.124};

  /* allocate scratch memory */
  z = (double **)malloc(c * sizeof(double *));
  if (NULL == z) {
    CDBG_ERROR("%s: Error mem alloc for z.\n", __func__);
    rc = -1;
    goto end;
  }
  t1 = (double *)malloc((c * c) * sizeof(double));
  if (NULL == t1) {
    CDBG_ERROR("%s: Error mem alloc for t1.\n", __func__);
    rc = -1;
    goto end;
  }
  for (i = 0; i < c; i++)
    z[i] = t1 + (i * c);

  y = (double **)malloc(r*n*4 * sizeof(double *));
  if (NULL == y) {
    CDBG_ERROR("%s: Error mem alloc for y.\n", __func__);
    rc = -1;
    goto end;
  }
  t2 = (double *)malloc((r*n*4 * c) * sizeof(double));
  if (NULL == t2) {
    CDBG_ERROR("%s: Error mem alloc for t2.\n", __func__);
    rc = -1;
    goto end;
  }
  for (i = 0; i < r*n*4; i++)
    y[i] = t2 + (i * c);

  v = (double **)malloc(c * sizeof(double *));
  if (NULL == v) {
    CDBG_ERROR("%s: Error mem alloc for v.\n", __func__);
    rc = -1;
    goto end;
  }
  t3 = (double *)malloc((c * c) * sizeof(double));
  if (NULL == t3) {
    CDBG_ERROR("%s: Error mem alloc for t3.\n", __func__);
    rc = -1;
    goto end;
  }
  for (i = 0; i < c; i++)
    v[i] = t3 + (i * c);

  s = (double *)malloc(c * sizeof(double));
  if (NULL == s) {
    CDBG_ERROR("%s: Error mem alloc for s.\n", __func__);
    rc = -1;
    goto end;
  }

  l = (double *)malloc(nbases * sizeof(double));
  if (NULL == l) {
    CDBG_ERROR("%s: Error mem alloc for l.\n", __func__);
    rc = -1;
    goto end;
  }

  m = (int *)malloc(c * sizeof(int));
  if (NULL == m) {
    CDBG_ERROR("%s: Error mem alloc for m.\n", __func__);
    rc = -1;
    goto end;
  }

  if (!is_bases_known) {
    CDBG("%s: Bases are not known. Compute them\n", __func__);
    /* form the covariance matrix and compute eigen vectors */
    transmat(x, c, r*n*nch, y);

    multmat(x, c, r*n*nch, y, r*n*nch, c, z);

    eigen(z, v, s, c);

    bubblesort(s, c, m);

    for(i = 0; i < nbases; i++) {
      for(j = 0; j < c; j++)
        optimbases[i][j] = v[j][m[i]];

      a = absmax(optimbases[i], c);
      for(j = 0; j < c; j++)
        optimbases[i][j] /= (a / 0.99);

      l[i] = 0;
      for(j = 0; j < c; j++)
        l[i] += optimbases[i][j] * optimbases[i][j];
    }
  } else {
    CDBG("%s: Bases are known. Directly use them\n", __func__);
  }

  /* compute eigen vector coefficients */
  multmat(optimbases, nbases, c, x, c, r*n*nch, optimcoeffs);

  for(i = 0; i < nbases; i++)
    for(j = 0; j < r*n*nch; j++) {
      optimcoeffs[i][j] /= l[i];

      if(optimcoeffs[i][j] < cmin[i])
        optimcoeffs[i][j] = cmin[i];

      if(optimcoeffs[i][j] > cmax[i])
        optimcoeffs[i][j] = cmax[i];
    }
  /* deallocate scratch memory */
end:
  if (m) {
    free(m);
    m = NULL;
  }
  if (l) {
    free(l);
    l = NULL;
  }
  if (s) {
    free(s);
    s = NULL;
  }
  if (t3) {
    free(t3);
    t3 = NULL;
  }
  if (v) {
    free(v);
    v = NULL;
  }
  if (t2) {
    free(t2);
    t2 = NULL;
  }
  if (y) {
    free(y);
    y = NULL;
  }
  if (t1) {
    free(t1);
    t1 = NULL;
  }
  if (z) {
    free(z);
    z = NULL;
  }

  return rc;
} /* subspaceoptim */
