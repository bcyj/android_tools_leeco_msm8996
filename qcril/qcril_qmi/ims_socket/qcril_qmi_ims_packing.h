/*!
  @file
  qcril_qmi_ims_packing.h

  @brief

*/

/*===========================================================================

  Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

  Export of this technology or software is regulated by the U.S. Government.
  Diversion contrary to U.S. law prohibited.

  All ideas, data and information contained in or disclosed by
  this document are confidential and proprietary information of
  Qualcomm Technologies, Inc. and all rights therein are expressly reserved.
  By accepting this material the recipient agrees that this material
  and the information contained therein are held in confidence and in
  trust and will not be used, copied, reproduced in whole or in part,
  nor its contents revealed in any manner to others without the express
  written permission of Qualcomm Technologies, Inc.

===========================================================================*/

#ifndef QCRIL_QMI_IMS_PACKING_H
#define QCRIL_QMI_IMS_PACKING_H

#include "qcrili.h"

size_t qcril_qmi_ims_pack_msg_tag(uint32_t token, Ims__MsgType type, Ims__MsgId message_id, Ims__Error error, uint8_t *buf, size_t buf_size);
Ims__MsgTag* qcril_qmi_ims_unpack_msg_tag(const uint8_t *buf);

size_t qcril_qmi_ims_pack_msg(const void *msg, Ims__MsgType type, Ims__MsgId message_id, uint8_t *buf, size_t max_buf_size);
void qcril_qmi_ims_parse_packed_msg(Ims__MsgType type, Ims__MsgId message_id, const uint8_t *packed_msg, size_t packed_msg_size,
                                     void** unpacked_msg, size_t*unpacked_msg_size_ptr, qcril_evt_e_type* event_ptr);

#endif /* QCRIL_QMI_IMS_PACKING_H */
