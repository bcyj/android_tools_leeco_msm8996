/******************************************************************************
  ---------------------------------------------------------------------------

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

#ifndef CRI_DATA
#define CRI_DATA

#include "utils_common.h"
#include "cri_core.h"
#include "cri_nas.h"
#include "data_system_determination_v01.h"

typedef struct cri_data_system_status_info_type
{
    int is_dsd;
    uint32_t data_rte;
    uint32_t data_tech;
 }cri_data_system_status_info_type;


qmi_error_type_v01 cri_data_init_client(hlos_ind_cb_type hlos_ind_cb);
void cri_data_release_client(int qmi_service_client_id);
void cri_data_async_resp_handler(int qmi_service_client_id,
                                unsigned long message_id,
                                void *resp_data,
                                int resp_data_len,
                                cri_core_context_type cri_core_context);
void cri_data_unsol_ind_handler(int qmi_service_client_id,
                                unsigned long message_id,
                                void *ind_data,
                                int ind_data_len);

cri_data_system_status_info_type* cri_data_retrieve_data_system_status();

#endif
