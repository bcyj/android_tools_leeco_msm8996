/*!
  @file
  qcril_uim_remote_client_msg_meta.h

  @brief

*/

/*===========================================================================

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
===========================================================================*/

#ifndef QCRIL_UIM_REMOTE_CLIENT_MSG_META_H
#define QCRIL_UIM_REMOTE_CLIENT_MSG_META_H

#include "uim_remote_client.pb.h"
#include "qcrili.h"
#include "qcril_uim_remote_client_misc.h"
#include "qcril_qmi_npb_utils.h"

size_t qcril_uim_remote_client_get_msg_size(com_qualcomm_uimremoteclient_MessageId msg_id, com_qualcomm_uimremoteclient_MessageType msg_type);
const pb_field_t* qcril_uim_remote_client_get_msg_pb_fields(com_qualcomm_uimremoteclient_MessageId msg_id, com_qualcomm_uimremoteclient_MessageType msg_type);
const char* qcril_uim_remote_client_get_msg_log_str(com_qualcomm_uimremoteclient_MessageId msg_id, com_qualcomm_uimremoteclient_MessageType msg_type);
qcril_evt_e_type qcril_uim_remote_client_get_msg_event(com_qualcomm_uimremoteclient_MessageId msg_id, com_qualcomm_uimremoteclient_MessageType msg_type);
boolean qcril_uim_remote_client_is_msg_valid(com_qualcomm_uimremoteclient_MessageId msg_id, com_qualcomm_uimremoteclient_MessageType msg_type);

#endif /* QCRIL_UIM_REMOTE_CLIENT_MSG_META_H */
