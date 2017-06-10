/*!
  @file
  qcril_qmi_ims_socket.h

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

#ifndef QCRIL_QMI_IMS_SOCKET_H
#define QCRIL_QMI_IMS_SOCKET_H

#ifdef  __cplusplus
extern "C" {
#endif

#include "qcrili.h"

#include "ril.h"

#include "qcril_qmi_nas.h"

#define QCRIL_QMI_IMS_SOCKET_MAX_BUF_SIZE (1024*8)
#define QCRIL_QMI_IMS_SOCKET_MAX_THREAD_NAME_SIZE 50

void qcril_qmi_ims_socket_init();
void qcril_qmi_ims_socket_send(RIL_Token token, Ims__MsgType type, Ims__MsgId message_id, Ims__Error error, const void* msg, int msg_len);
void qcril_qmi_ims_socket_send_empty_payload_unsol_resp(Ims__MsgId msg_id);

#ifdef  __cplusplus
}
#endif

#endif /* QCRIL_QMI_IMS_SOCKET_H */
