/******************************************************************************
  ---------------------------------------------------------------------------

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

#include "cri_wms.h"
#include "cri_wms_core.h"

qmi_error_type_v01 cri_wms_init_client(hlos_ind_cb_type hlos_ind_cb)
{
    return cri_wms_core_init_client(hlos_ind_cb);
}

void cri_wms_release_client(int qmi_service_client_id)
{
    cri_wms_core_release_client(qmi_service_client_id);
}

void cri_wms_async_resp_handler(int qmi_service_client_id,
                                unsigned long message_id,
                                void *resp_data,
                                int resp_data_len,
                                cri_core_context_type cri_core_context)
{
    cri_wms_core_async_resp_handler(qmi_service_client_id,
                                    message_id,
                                    resp_data,
                                    resp_data_len,
                                    cri_core_context);
}

void cri_wms_unsol_ind_handler(int qmi_service_client_id,
                                unsigned long message_id,
                                void *ind_data,
                                int ind_data_len)
{
    cri_wms_core_unsol_ind_handler(qmi_service_client_id,
                                   message_id,
                                   ind_data,
                                   ind_data_len);
}

qmi_error_type_v01 cri_wms_send_gw_sms(cri_core_context_type cri_core_context,
                                       char *destination_number,
                                       char *message_content,
                                       void *hlos_cb_data,
                                            hlos_resp_cb_type hlos_resp_cb,
                                            cri_wms_mo_pp_sms_type concatenated,
                                            int seg_number,
                                            int total_segments)
{
    return cri_wms_core_send_gw_sms(cri_core_context,
                                    destination_number,
                                    message_content,
                                    hlos_cb_data,
                                    hlos_resp_cb,
                                    concatenated,
                                    seg_number,
                                    total_segments);
}

qmi_error_type_v01 cri_wms_send_cdma_sms(cri_core_context_type cri_core_context,
                                       char *destination_number,
                                       char *message_content,
                                       void *hlos_cb_data,
                                       hlos_resp_cb_type hlos_resp_cb)
{
    return cri_wms_core_send_cdma_sms(cri_core_context,
                                    destination_number,
                                    message_content,
                                    hlos_cb_data,
                                    hlos_resp_cb);
}
