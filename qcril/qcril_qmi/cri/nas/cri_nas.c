/******************************************************************************
  ---------------------------------------------------------------------------

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

#include "cri_nas.h"
#include "cri_nas_core.h"

qmi_error_type_v01 cri_nas_init_client(hlos_ind_cb_type hlos_ind_cb)
{
    return cri_nas_core_init_client(hlos_ind_cb);
}

void cri_nas_release_client(int qmi_service_client_id)
{
    cri_nas_core_release_client(qmi_service_client_id);
}

void cri_nas_async_resp_handler(int qmi_service_client_id,
                                unsigned long message_id,
                                void *resp_data,
                                int resp_data_len,
                                cri_core_context_type cri_core_context)
{
    cri_nas_core_async_resp_handler(qmi_service_client_id,
                                    message_id,
                                    resp_data,
                                    resp_data_len,
                                    cri_core_context);
}

void cri_nas_unsol_ind_handler(int qmi_service_client_id,
                                unsigned long message_id,
                                void *ind_data,
                                int ind_data_len)
{
    cri_nas_core_unsol_ind_handler(qmi_service_client_id,
                                   message_id,
                                   ind_data,
                                   ind_data_len);
}

void cri_nas_update_data_system_status()
{
    cri_nas_core_update_data_system_status();
}
