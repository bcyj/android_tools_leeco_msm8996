/******************************************************************************
  @file    qcril_qmi_pdc.h
  @brief   qcril qmi - PDC

  DESCRIPTION
    Handles RIL requests, Callbacks, indications for QMI NAS.

  ---------------------------------------------------------------------------

  Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/


#ifndef QCRIL_QMI_PDC_H
#define QCRIL_QMI_PDC_H


/*===========================================================================

                           INCLUDE FILES

============================================================================*/

#include "comdef.h"
#include "qmi_client.h"
#include <libxml/parser.h>
#include <libxml/xpath.h>

/*===========================================================================

                        DEFINITIONS AND TYPES

 ===========================================================================*/


#define QCRIL_MBN_FILE_PATH_LEN             255
#define QCRIL_DUMP_FILE_PATH_LEN            255

#define QCRIL_DUMP_FILE_PREFIX              "/data/misc/radio/validation_result_sub"

void qcril_qmi_pdc_set_modem_test_mode
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
);

void qcril_qmi_pdc_query_modem_test_mode
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
);

void qcril_qmi_pdc_get_available_configs
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
);

qmi_client_error_type qcril_qmi_pdc_init
(
  void
);

void qcril_qmi_pdc_load_configuration
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
);
void qcril_qmi_pdc_select_configuration
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
);

void qcril_qmi_pdc_activate_configuration
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
);

void qcril_qmi_pdc_deactivate_current_config
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
);

void qcril_qmi_pdc_cleanup_loaded_configs
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
);

void qcril_qmi_pdc_select_configs
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
);

void qcril_qmi_pdc_get_meta_info_of_config
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
);

void qcril_qmi_pdc_delete_configuration
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
);

void qcril_qmi_pdc_list_configuration
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
);

void qcril_qmi_pdc_deactivate_configuration
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
);

void qcril_qmi_pdc_deactivate_configs
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
);

void qcril_qmi_pdc_get_qc_version_of_file
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
);

void qcril_qmi_pdc_get_qc_version_of_configid
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
);

void qcril_qmi_pdc_validate_config
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
);

void qcril_qmi_pdc_parse_diff_result
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
);

void qcril_qmi_pdc_get_oem_version_of_file
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
);

void qcril_qmi_pdc_get_oem_version_of_configid
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
);

void qcril_qmi_pdc_activate_configs
(
  const qcril_request_params_type *const params_ptr,
  qcril_request_return_type *const ret_ptr
);

void qcril_qmi_pdc_unsol_ind_cb
(
  qmi_client_type       user_handle,
  unsigned long         msg_id,
  unsigned char         *ind_buf,
  int                   ind_buf_len,
  void                  *ind_cb_data
);

void qcril_qmi_pdc_retrieve_current_mbn_info
(
    void
);

void qcril_qmi_mbn_diff_send_unsol_msg
(
  int result,
  int index,
  xmlChar* id,
  xmlChar* ref_val,
  xmlChar* dev_val
);

#endif
