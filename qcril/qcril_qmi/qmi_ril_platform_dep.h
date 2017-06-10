/******************************************************************************

  @file    qmi_ril_platform_dep.h
  @brief   Provides interface to functions where conditional platform
           dependency is there.

  ---------------------------------------------------------------------------

  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

  ---------------------------------------------------------------------------
******************************************************************************/

#ifndef QMI_RIL_PLATFORM_DEP_H
#define QMI_RIL_PLATFORM_DEP_H

#include "qcril_qmi_client.h"

/*===========================================================================

  FUNCTION  qcril_process_mdm_shutdown

===========================================================================*/
/*!
    @brief
    Process modem shutdown request

    @return
    0 on success
*/
/*=========================================================================*/
RIL_Errno qcril_process_mdm_shutdown
(
    void
);

/*===========================================================================

  FUNCTION  qmi_client_send_msg_async_with_shm

===========================================================================*/
/*!
    @brief
    Uses system health monitor

    @return
    0 on success
*/
/*=========================================================================*/

qmi_client_error_type
qmi_client_send_msg_async_with_shm
(
    qmi_client_type                 user_handle,
    unsigned long                   msg_id,
    void                            *req_c_struct,
    int                             req_c_struct_len,
    void                            *resp_c_struct,
    int                             resp_c_struct_len,
    qmi_client_recv_msg_async_cb    resp_cb,
    void                            *resp_cb_data,
    qmi_txn_handle                  *txn_handle
);

/*===========================================================================

  FUNCTION  qmi_client_send_msg_sync_with_shm

===========================================================================*/
/*!
    @brief
    Uses system health monitor

    @return
    0 on success
*/
/*=========================================================================*/
qmi_client_error_type
qmi_client_send_msg_sync_with_shm
(
    qmi_client_type    user_handle,
    int                msg_id,
    void               *req_c_struct,
    int                req_c_struct_len,
    void               *resp_c_struct,
    int                resp_c_struct_len,
    int                timeout_msecs
);

#endif /* QCRIL_DB_H */
