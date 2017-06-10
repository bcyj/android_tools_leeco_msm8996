#ifndef QMI_CLIENT_DEPRECATED_H
#define QMI_CLIENT_DEPRECATED_H
/******************************************************************************
  @file    qmi_client_deprecated.h
  @brief   The QMI common client Header file for deprecated functions.

  DESCRIPTION
  This header file consists of function declaration to support legacy
  QMI Interfaces.


  ---------------------------------------------------------------------------
  Copyright (c) 2012-2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
*******************************************************************************/
/*===========================================================================

                           EDIT HISTORY FOR FILE

$Header: //source/qcom/qct/core/api/mproc/main/latest/qmi_client_deprecated.h#1 $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
01/19/12   fr      Initial Version
===========================================================================*/
#include "qmi_client.h"

#ifdef __cplusplus
extern "C" {
#endif
/*==========================================================================
  FUNCTION  qmi_client_get_async_txn_id
===========================================================================*/
/*!
@brief

  Gets a transaction id from the transaction handle.

@param[in]  user_handle   Handle used by the infrastructure to
                          identify different clients.
@param[in]   txn_handle   Handle used to identify the transaction as
                          returned by the send async functions.
@param[out]  txn_id       Transaction ID corresponding to the transaction
                          handle.

@return
  qmi_client_error_type

@note
  This API is added to support the legacy QMI messages that needs access to
  the transcation ID.
*/
/*=========================================================================*/
qmi_client_error_type
qmi_client_get_async_txn_id
(
 qmi_client_type  user_handle,
 qmi_txn_handle   async_txn_handle,
 uint32_t         *txn_id
);

#ifdef __cplusplus
}
#endif
#endif  /* QMI_CLIENT_DEPRECATED_H */
