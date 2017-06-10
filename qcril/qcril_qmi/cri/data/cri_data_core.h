/******************************************************************************
  ---------------------------------------------------------------------------

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

#ifndef CRI_DATA_CORE
#define CRI_DATA_CORE

#include "utils_common.h"
#include "cri_core.h"
#include "data_system_determination_v01.h"
#include "cri_data.h"


int cri_data_core_retrieve_client_id();
qmi_error_type_v01 cri_data_core_init_client(hlos_ind_cb_type hlos_ind_cb);
void cri_data_core_release_client(int qmi_service_client_id);
void cri_data_core_unsol_ind_handler(int qmi_service_client_id,
                                        unsigned long message_id,
                                        void *ind_data,
                                        int ind_data_len);
void cri_data_core_async_resp_handler(int qmi_service_client_id,
                                     unsigned long message_id,
                                     void *resp_data,
                                     int resp_data_len,
                                     cri_core_context_type cri_core_context);

cri_data_system_status_info_type* cri_data_core_retrieve_data_system_status();

#endif
