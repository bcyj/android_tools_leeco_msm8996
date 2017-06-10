/*!
  @file
  qcril_qmi_npb_encode.h

  @brief

*/

/*===========================================================================

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

===========================================================================*/

#ifndef QCRIL_QMI_NPB_ENCODE_H
#define QCRIL_QMI_NPB_ENCODE_H

#include <comdef.h>
#include <pb.h>

boolean qcril_qmi_encode_npb(pb_ostream_t *os, const pb_field_t pb_fields[], void *msg);

#endif
