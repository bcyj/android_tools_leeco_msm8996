/*!
  @file
  qcril_qmi_ims_msg_meta.h

  @brief

*/

/*===========================================================================

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

===========================================================================*/

#ifndef QCRIL_QMI_IMS_MSG_META_H
#define QCRIL_QMI_IMS_MSG_META_H

#include "imsIF.pb.h"
#include "qcrili.h"

size_t qcril_qmi_ims_get_msg_size(ims_MsgId msg_id, ims_MsgType msg_type);
const pb_field_t* qcril_qmi_ims_get_msg_pb_fields(ims_MsgId msg_id, ims_MsgType msg_type);
const char* qcril_qmi_ims_get_msg_log_str(ims_MsgId msg_id, ims_MsgType msg_type);
qcril_evt_e_type qcril_qmi_ims_get_msg_event(ims_MsgId msg_id, ims_MsgType msg_type);

#endif /* QCRIL_QMI_IMS_MSG_META_H */
