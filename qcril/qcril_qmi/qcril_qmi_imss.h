/******************************************************************************
  @file    qcril_qmi_imss.h
  @brief   qcril qmi - IMS Setting

  DESCRIPTION
    Handles RIL requests, Callbacks, indications for QMI IMS Setting.

  ---------------------------------------------------------------------------

  Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

  ---------------------------------------------------------------------------
******************************************************************************/

#ifndef QCRIL_QMI_IMSS_H
#define QCRIL_QMI_IMSS_H

#include "qmi.h"
#include "qmi_client.h"
#include "qcrili.h"

void qcril_qmi_imss_init(void);

void qcril_qmi_imss_info_lock(void);

void qcril_qmi_imss_info_unlock(void);

void qcril_qmi_imss_get_ims_reg_config(void);

void qcril_qmi_imss_request_set_ims_registration
(
   const qcril_request_params_type *const params_ptr,
   qcril_request_return_type *const ret_ptr /*!< Output parameter */
);

void qcril_qmi_imss_request_set_ims_srv_status(
   const qcril_request_params_type *const params_ptr,
   qcril_request_return_type *const ret_ptr /*!< Output parameter */
);

void qcril_qmi_imss_request_query_vt_call_quality
(
   const qcril_request_params_type *const params_ptr,
   qcril_request_return_type *const ret_ptr /*!< Output parameter */
);

void qcril_qmi_imss_request_set_vt_call_quality
(
   const qcril_request_params_type *const params_ptr,
   qcril_request_return_type *const ret_ptr /*!< Output parameter */
);

void qcril_qmi_imss_request_get_wifi_calling_status
(
   const qcril_request_params_type *const params_ptr,
   qcril_request_return_type *const ret_ptr /*!< Output parameter */
);

void qcril_qmi_imss_request_set_wifi_calling_status
(
   const qcril_request_params_type *const params_ptr,
   qcril_request_return_type *const ret_ptr /*!< Output parameter */
);

void qcril_qmi_imss_command_cb
(
   qmi_client_type              user_handle,
   unsigned int                 msg_id,
   void                        *resp_c_struct,
   unsigned int                 resp_c_struct_len,
   void                        *resp_cb_data,
   qmi_client_error_type        transp_err
);

void qcril_qmi_imss_command_cb_helper
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
);

#endif
