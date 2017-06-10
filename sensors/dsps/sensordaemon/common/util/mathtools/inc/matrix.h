#ifndef MATRIX_H
#define MATRIX_H

/*============================================================================
  @file matrix.h

  Matrix utility header file 

  Copyright (c) 2010-2011 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Qualcomm Technologies Confidential and Proprietary
============================================================================*/

/* $Header: //source/qcom/qct/core/sensors/dsps/common/main/latest/util/mathtools/inc/matrix.h#1 $ */
/* $DateTime: 2011/01/28 12:40:41 $ */
/* $Author: agurujee $ */

/*============================================================================
  EDIT HISTORY

  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order. 

  when        who  what, where, why
  ----------  ---  -----------------------------------------------------------
  2011-01-24  ad   initial version  

============================================================================*/

#include <stdint.h>

#define MATRIX_ELEM(m,i,j) (*((m)->matrix + i*(m)->cols + j))

typedef struct
{
  int32_t *matrix;
  int32_t rows;
  int32_t cols;
} matrix_type;

int32_t matrix_mem_req(int32_t rows, int32_t cols);
void matrix_reset(matrix_type *m, int32_t rows, int32_t cols, void *mem);
void matrix_zero(matrix_type *m);
void matrix_multiply(matrix_type *result, matrix_type *A, matrix_type *B);
void matrix_copy(matrix_type *to, matrix_type *from);
int32_t vector_norm(matrix_type *m);
void vector_normalize(matrix_type *m);

#endif /* MATRIX_H */
