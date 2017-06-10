/*!
  @file
  qcril_uim_remote_server_msg_meta.h

  @brief

*/

/*===========================================================================

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
===========================================================================*/

#ifndef QCRIL_UIM_REMOTE_SERVER_MSG_META_H
#define QCRIL_UIM_REMOTE_SERVER_MSG_META_H

#include "sap-api.pb.h"
#include "qcrili.h"
#include "qcril_uim_remote_server_misc.h"
#include "qcril_qmi_npb_utils.h"

size_t qcril_uim_remote_server_get_msg_size(MsgId msg_id, MsgType msg_type);
const pb_field_t* qcril_uim_remote_server_get_msg_pb_fields(MsgId msg_id, MsgType msg_type);
const char* qcril_uim_remote_server_get_msg_log_str(MsgId msg_id, MsgType msg_type);
qcril_evt_e_type qcril_uim_remote_server_get_msg_event(MsgId msg_id, MsgType msg_type);
boolean qcril_uim_remote_server_is_msg_valid(MsgId msg_id, MsgType msg_type);

#endif /* QCRIL_UIM_REMOTE_SERVER_MSG_META_H */
