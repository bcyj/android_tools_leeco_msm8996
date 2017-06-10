/******************************************************************************
  ---------------------------------------------------------------------------

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

#ifndef CRI_WMS_CORE
#define CRI_WMS_CORE

#include "utils_common.h"
#include "cri_core.h"
#include "wireless_messaging_service_v01.h"
#include "cri_wms.h"


int cri_wms_core_retrieve_client_id();
qmi_error_type_v01 cri_wms_core_init_client(hlos_ind_cb_type hlos_ind_cb);
void cri_wms_core_release_client(int qmi_service_client_id);
void cri_wms_core_unsol_ind_handler(int qmi_service_client_id,
                                        unsigned long message_id,
                                        void *ind_data,
                                        int ind_data_len);
void cri_wms_core_async_resp_handler(int qmi_service_client_id,
                                     unsigned long message_id,
                                     void *resp_data,
                                     int resp_data_len,
                                     cri_core_context_type cri_core_context);
qmi_error_type_v01 cri_wms_core_send_gw_sms(cri_core_context_type cri_core_context,
                                            char *destination_number,
                                            char *message_content,
                                            void *hlos_cb_data,
                                            hlos_resp_cb_type hlos_resp_cb,
                                            cri_wms_mo_pp_sms_type concatenated,
                                            int seg_number,
                                            int total_segments);

qmi_error_type_v01 cri_wms_core_send_cdma_sms(cri_core_context_type cri_core_context,
                                            char *destination_number,
                                            char *message_content,
                                            void *hlos_cb_data,
                                            hlos_resp_cb_type hlos_resp_cb);

#endif
