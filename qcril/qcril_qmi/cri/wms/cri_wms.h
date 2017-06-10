/******************************************************************************
  ---------------------------------------------------------------------------

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

#ifndef CRI_WMS
#define CRI_WMS

#include "utils_common.h"
#include "cri_core.h"
#include "wireless_messaging_service_v01.h"

#define CRI_WMS_MT_PP_SMS_IND 0x0001
#define CRI_WMS_MT_CB_SMS_IND 0x0002


#define CRI_WMS_DESTINATION_NUMBER_MAX_LEN (30)
#define CRI_WMS_MESSAGE_CONTENT_MAX_LEN (255)

typedef struct cri_wms_mt_pp_sms_type
{
    uint32_t transaction_id;
    wms_message_format_enum_v01 format;
    char source_number[CRI_WMS_DESTINATION_NUMBER_MAX_LEN];
    char message_content[CRI_WMS_MESSAGE_CONTENT_MAX_LEN];
}cri_wms_mt_pp_sms_type;

typedef struct cri_wms_mt_cb_sms_type
{
    wms_message_format_enum_v01 format;
    char message_content[CRI_WMS_MESSAGE_CONTENT_MAX_LEN];
}cri_wms_mt_cb_sms_type;

typedef enum
{
    CRI_WMS_MO_PP_SMS_TYPE_NOT_CONCATENATED = 0,
    CRI_WMS_MO_PP_SMS_TYPE_CONCATENATED = 1
}cri_wms_mo_pp_sms_type;

qmi_error_type_v01 cri_wms_init_client(hlos_ind_cb_type hlos_ind_cb);
void cri_wms_release_client(int qmi_service_client_id);
void cri_wms_async_resp_handler(int qmi_service_client_id,
                                unsigned long message_id,
                                void *resp_data,
                                int resp_data_len,
                                cri_core_context_type cri_core_context);
void cri_wms_unsol_ind_handler(int qmi_service_client_id,
                                unsigned long message_id,
                                void *ind_data,
                                int ind_data_len);
qmi_error_type_v01 cri_wms_send_gw_sms(cri_core_context_type cri_core_context,
                                       char *destination_number,
                                       char *message_content,
                                       void *hlos_cb_data,
                                            hlos_resp_cb_type hlos_resp_cb,
                                            cri_wms_mo_pp_sms_type concatenated,
                                            int seg_number,
                                            int total_segments);

qmi_error_type_v01 cri_wms_send_cdma_sms(cri_core_context_type cri_core_context,
                                            char *destination_number,
                                            char *message_content,
                                            void *hlos_cb_data,
                                            hlos_resp_cb_type hlos_resp_cb);






#endif
