#ifndef QMI_CSI_DEPRECATED_H
#define QMI_CSI_DEPRECATED_H
/******************************************************************************
  @file    qmi_csi_deprecated.h
  @brief   The QMI common service Interface Header file for deprecated
           functions.

  DESCRIPTION
  This header file consists of function declaration to support legacy
  QMI Interfaces.


  ---------------------------------------------------------------------------
  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
*******************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif


#include "qmi_csi.h"
/*=============================================================================
  FUNCTION  qmi_csi_get_txn_id
=============================================================================*/
/*!
@brief
  Gets a tansaction id corresponding to the request handle.

@param[in]  req_handle            Handle used by QCSI infrastructure to
                                  identify the transaction and the destination
                                  client.
@param[out] txn_id                Transaction ID corresponding to the message.

@retval     QMI_CSI_NO_ERR        Success
@retval     QMI_CSI_.....         Look into the enumeration qmi_csi_error for
                                  the error values.
@note
This API will return error once the response corresponding to the req_handle
has been sent as the transaction is not valid after the response has been sent.
*/
/*=========================================================================*/
qmi_csi_error
qmi_csi_get_txn_id
(
 qmi_req_handle     req_handle,
 unsigned int       *txn_id
);

#ifdef __cplusplus
}
#endif
#endif /* QMI_CSI_DEPRECATED_H */
