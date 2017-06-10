/*!
  @file
  qcril_qmi_npb_utils.h

  @brief

*/

/*===========================================================================

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

===========================================================================*/

#ifndef QCRIL_QMI_NPB_UTILS_H
#define QCRIL_QMI_NPB_UTILS_H

#include <pb.h>
#include "qcrili.h"

typedef struct
{
    const pb_field_t *pos;
    void *dest_struct;
    void *cur_data_ptr;
} qcril_qmi_npb_field_iter_type;

boolean qcril_qmi_npb_field_init(qcril_qmi_npb_field_iter_type *iter, const pb_field_t *fields, void *dest_struct);

boolean qcril_qmi_npb_field_next(qcril_qmi_npb_field_iter_type *iter);

size_t qcril_qmi_npb_get_data_size(const pb_field_t fields[]);

void qcril_qmi_npb_release(const pb_field_t fields[], void *dest_struct);

#endif
