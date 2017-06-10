/*!
  @file
  qcril_qmi_npb_decode.h

  @brief

*/

/*===========================================================================

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

===========================================================================*/


#ifndef QCRIL_QMI_NPB_DECODE_H
#define QCRIL_QMI_NPB_DECODE_H

#include <comdef.h>
#include <pb.h>

boolean qcril_qmi_decode_npb(pb_istream_t *is, const pb_field_t pb_fields[], void* msg);

#endif
