/******************************************************************************
  ---------------------------------------------------------------------------

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

#ifndef CRI_DMS_CORE
#define CRI_DMS_CORE

#include "utils_common.h"
#include "cri_core.h"
#include "cri_dms_utils.h"
#include "device_management_service_v01.h"


cri_core_error_type cri_dms_set_modem_request_handler( cri_core_context_type cri_core_context,
                                                    uint32_t opr_mode,
                                                    void *hlos_cb_data,
                                                    hlos_resp_cb_type hlos_set_modem_resp_cb,
                                                    uint32_t *is_changed );


cri_core_error_type cri_dms_get_modem_status_request_handler(dms_operating_mode_enum_v01 *info_ptr);

cri_core_error_type cri_dms_get_modem_software_version_request_handler(char *version);


#if 0

cri_core_error_type cri_dms_get_sw_version_request_handler(dms_get_sw_version_resp_msg_v01 *qmi_response);

cri_core_error_type cri_dms_get_hw_version_request_handler(dms_get_device_hardware_rev_resp_msg_v01 *qmi_response);

cri_core_error_type cri_dms_reset_request_handler(dms_reset_resp_msg_v01 *qmi_response);

#endif

cri_core_error_type cri_dms_core_unsol_ind_handler(int qmi_service_client_id,
                                                    unsigned long message_id,
                                                    void *ind_data,
                                                    int ind_data_len);

void cri_dms_core_async_resp_handler(int qmi_service_client_id,
                                    unsigned long message_id,
                                    void *resp_data,
                                    int resp_data_len,
                                    cri_core_context_type cri_core_context);

void cri_dms_set_modem_resp_handler(int qmi_service_client_id,
                                    dms_set_operating_mode_resp_msg_v01 *set_opr_mode_resp_msg,
                                    cri_core_context_type cri_core_context);



#endif
