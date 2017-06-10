/******************************************************************************
  ---------------------------------------------------------------------------

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

#include <stdlib.h>
#include <ctype.h>

#include "cri_nas_core.h"
#include "cri_nas_utils.h"
#include "cri_utils.h"
#include "cri_nas_rules.h"
#include "cri_dms_core.h"
#include "cri_dms_utils.h"
#include "cri_rule_handler.h"

int nas_client_id;

static uint32_t is_voice_rte_changed;
static uint32_t is_data_rte_changed;

static qmi_error_type_v01 cri_nas_init_client_state();
static void cri_nas_cleanup_client_state();
static qmi_error_type_v01 cri_nas_core_toggle_indication_subscription(int is_subscribe);
static qmi_error_type_v01 cri_nas_core_fetch_update_sys_info();
static qmi_error_type_v01 cri_nas_core_fetch_update_sig_info();

static inline int cri_nas_core_is_contains_cs(nas_service_domain_enum_type_v01 domain_cap);
static inline int cri_nas_core_is_contains_ps(nas_service_domain_enum_type_v01 domain_cap);
static inline int cri_nas_core_is_radio_if_same_as_rte(nas_radio_if_enum_v01 radio_if,
                                                           cri_nas_rte_type rte);
static inline cri_nas_rte_reg_confidence_type cri_nas_core_convert_reg_status_to_confidence(
                                                  cri_nas_tech_reg_status_type reg_status);

static void cri_nas_core_calculate_gsm_voice_reg_status();
static void cri_nas_core_calculate_wcdma_voice_reg_status();
static void cri_nas_core_calculate_tdscdma_voice_reg_status();
static void cri_nas_core_calculate_lte_voice_reg_status();
static void cri_nas_core_calculate_cdma_voice_reg_status();

static void cri_nas_core_calculate_gsm_data_reg_status();
static void cri_nas_core_calculate_wcdma_data_reg_status();
static void cri_nas_core_calculate_tdscdma_data_reg_status();
static void cri_nas_core_calculate_lte_data_reg_status();
static void cri_nas_core_calculate_cdma_data_reg_status();
static void cri_nas_core_calculate_hdr_data_reg_status();

static void cri_nas_calculate_voice_rte();
static void cri_nas_calculate_data_rte();

static void cri_nas_update_voice_network_info();
static void cri_nas_update_data_network_info();
static void cri_nas_update_nw_selection_info();
static void cri_nas_core_nw_scan_resp_handler(int qmi_service_client_id,
                                        void *nw_scan_resp_msg,
                                        cri_core_context_type cri_core_context);
static cri_core_error_type cri_nas_get_current_mcc_mnc(char *mcc, char *mnc);
static void cri_nas_handle_centralized_short_long_eons(char *mcc_str, char *mnc_str, char *short_eons, char *long_eons,uint32_t plmn_rat);




int cri_nas_core_retrieve_client_id()
{
    return nas_client_id;
}

qmi_error_type_v01 cri_nas_core_init_client(hlos_ind_cb_type hlos_ind_cb)
{
    qmi_error_type_v01 client_init_error;

    client_init_error = QMI_ERR_INTERNAL_V01;

    nas_client_id = cri_core_create_qmi_service_client(QMI_NAS_SERVICE,
                                                        hlos_ind_cb);
    if(QMI_INTERNAL_ERR != nas_client_id)
    {
        client_init_error = cri_nas_init_client_state();
    }

    return client_init_error;
}

void cri_nas_core_release_client(int qmi_service_client_id)
{
    cri_nas_cleanup_client_state();
    cri_core_release_qmi_service_client(qmi_service_client_id);
    nas_client_id = NIL;
}

qmi_error_type_v01 cri_nas_init_client_state()
{
    int iter_i;

    iter_i = 0;

    cri_nas_core_sys_info =
        util_memory_alloc(sizeof(*cri_nas_core_sys_info));
    cri_nas_core_managed_roaming_info =
        util_memory_alloc(sizeof(*cri_nas_core_managed_roaming_info));
    cri_nas_core_nas_reject_info =
        util_memory_alloc(sizeof(*cri_nas_core_nas_reject_info));

    for(iter_i = 0; iter_i < CRI_NAS_RTE_SERVICE_MAX; iter_i++)
    {
        cri_nas_rte_gsm_reg_info[iter_i] =
            util_memory_alloc(sizeof(cri_nas_rte_gsm_reg_info_type));
    }

    for(iter_i = 0; iter_i < CRI_NAS_RTE_SERVICE_MAX; iter_i++)
    {
        cri_nas_rte_wcdma_reg_info[iter_i] =
            util_memory_alloc(sizeof(cri_nas_rte_wcdma_reg_info_type));
    }

    for(iter_i = 0; iter_i < CRI_NAS_RTE_SERVICE_MAX; iter_i++)
    {
        cri_nas_rte_tdscdma_reg_info[iter_i] =
            util_memory_alloc(sizeof(cri_nas_rte_tdscdma_reg_info_type));
    }

    for(iter_i = 0; iter_i < CRI_NAS_RTE_SERVICE_MAX; iter_i++)
    {
        cri_nas_rte_lte_reg_info[iter_i] =
            util_memory_alloc(sizeof(cri_nas_rte_lte_reg_info_type));
    }

    for(iter_i = 0; iter_i < CRI_NAS_RTE_SERVICE_MAX; iter_i++)
    {
        cri_nas_rte_cdma_reg_info[iter_i] =
            util_memory_alloc(sizeof(cri_nas_rte_cdma_reg_info_type));
    }

    for(iter_i = 0; iter_i < CRI_NAS_RTE_SERVICE_MAX; iter_i++)
    {
        cri_nas_rte_hdr_reg_info[iter_i] =
            util_memory_alloc(sizeof(cri_nas_rte_hdr_reg_info_type));
    }

//    cri_nas_core_toggle_indication_subscription(TRUE);
    cri_nas_enable_networking_indications(FALSE,FALSE);
    cri_nas_core_fetch_update_sys_info();

    return QMI_ERR_NONE_V01;
}

void cri_nas_cleanup_client_state()
{
    int iter_i;

    iter_i = 0;

    cri_nas_core_toggle_indication_subscription(FALSE);

    if(cri_nas_core_sys_info)
    {
       util_memory_free((void**) &cri_nas_core_sys_info);
    }

    if(cri_nas_core_managed_roaming_info)
    {
       util_memory_free((void**) &cri_nas_core_managed_roaming_info);
    }

    if(cri_nas_core_nas_reject_info)
    {
       util_memory_free((void**) &cri_nas_core_nas_reject_info);
    }

    for(iter_i = 0; iter_i < CRI_NAS_RTE_SERVICE_MAX; iter_i++)
    {
        if(cri_nas_rte_gsm_reg_info[iter_i])
        {
            util_memory_free((void**) &cri_nas_rte_gsm_reg_info[iter_i]);
        }
    }

    for(iter_i = 0; iter_i < CRI_NAS_RTE_SERVICE_MAX; iter_i++)
    {
        if(cri_nas_rte_wcdma_reg_info[iter_i])
        {
            util_memory_free((void**) &cri_nas_rte_wcdma_reg_info[iter_i]);
        }
    }

    for(iter_i = 0; iter_i < CRI_NAS_RTE_SERVICE_MAX; iter_i++)
    {
        if(cri_nas_rte_tdscdma_reg_info[iter_i])
        {
            util_memory_free((void**) &cri_nas_rte_tdscdma_reg_info[iter_i]);
        }
    }

    for(iter_i = 0; iter_i < CRI_NAS_RTE_SERVICE_MAX; iter_i++)
    {
        if(cri_nas_rte_lte_reg_info[iter_i])
        {
            util_memory_free((void**) &cri_nas_rte_lte_reg_info[iter_i]);
        }
    }

    for(iter_i = 0; iter_i < CRI_NAS_RTE_SERVICE_MAX; iter_i++)
    {
        if(cri_nas_rte_cdma_reg_info[iter_i])
        {
            util_memory_free((void**) &cri_nas_rte_cdma_reg_info[iter_i]);
        }
    }

    for(iter_i = 0; iter_i < CRI_NAS_RTE_SERVICE_MAX; iter_i++)
    {
        if(cri_nas_rte_hdr_reg_info[iter_i])
        {
            util_memory_free((void**) &cri_nas_rte_hdr_reg_info[iter_i]);
        }
    }
}

qmi_error_type_v01 cri_nas_core_toggle_indication_subscription(int is_subscribe)
{
    qmi_error_type_v01 err_code;
    nas_indication_register_req_msg_v01 nas_indication_register_req_msg;
    nas_indication_register_resp_msg_v01 nas_indication_register_resp_msg;

    err_code = QMI_ERR_INTERNAL_V01;
    memset(&nas_indication_register_req_msg,
           NIL,
           sizeof(nas_indication_register_req_msg));
    memset(&nas_indication_register_resp_msg,
           NIL,
           sizeof(nas_indication_register_resp_msg));

    nas_indication_register_req_msg.req_serving_system_valid = TRUE;
    nas_indication_register_req_msg.req_serving_system = FALSE;

    nas_indication_register_req_msg.sys_info_valid = TRUE;
    nas_indication_register_req_msg.sys_info = is_subscribe;

    nas_indication_register_req_msg.network_reject_valid = TRUE;
    nas_indication_register_req_msg.network_reject.reg_network_reject = is_subscribe;
    nas_indication_register_req_msg.network_reject.suppress_sys_info = TRUE;

    nas_indication_register_req_msg.reg_managed_roaming_valid = TRUE;
    nas_indication_register_req_msg.reg_managed_roaming = is_subscribe;

    err_code = cri_core_qmi_send_msg_sync(cri_nas_core_retrieve_client_id(),
                                          QMI_NAS_INDICATION_REGISTER_REQ_MSG_V01,
                                          &nas_indication_register_req_msg,
                                          sizeof(nas_indication_register_req_msg),
                                          &nas_indication_register_resp_msg,
                                          sizeof(nas_indication_register_resp_msg),
                                          CRI_CORE_MINIMAL_TIMEOUT);

    if(CRI_ERR_NONE_V01 != cri_core_retrieve_err_code(err_code,
                                                      &nas_indication_register_resp_msg.resp))
    {
        err_code = QMI_ERR_INTERNAL_V01;
    }

    return err_code;
}

void cri_nas_fillup_mcc_mnc_helper(char *src_arr, char *dest_str)
{
    int iter_i=0,len=0;

    if( NULL != src_arr && NULL != dest_str )
    {
        for( iter_i=0; iter_i<NAS_MCC_MNC_MAX_V01; iter_i++ )
        {
            if( isdigit(src_arr[iter_i]) )
            {
                dest_str[iter_i] = src_arr[iter_i];
                len++;
            }
        }
        dest_str[len] = '\0';
    }
}


qmi_error_type_v01 cri_nas_core_fetch_update_sys_info()
{
    qmi_error_type_v01 err_code;
    nas_get_sys_info_resp_msg_v01 *nas_get_sys_info_resp_msg;

    err_code = QMI_ERR_INTERNAL_V01;
    nas_get_sys_info_resp_msg = NULL;

    nas_get_sys_info_resp_msg = util_memory_alloc(sizeof(*nas_get_sys_info_resp_msg));

    if(nas_get_sys_info_resp_msg)
    {
        err_code = cri_core_qmi_send_msg_sync(cri_nas_core_retrieve_client_id(),
                                              QMI_NAS_GET_SYS_INFO_REQ_MSG_V01,
                                              NULL,
                                              NIL,
                                              nas_get_sys_info_resp_msg,
                                              sizeof(*nas_get_sys_info_resp_msg),
                                              CRI_CORE_MINIMAL_TIMEOUT);

        if(CRI_ERR_NONE_V01 == cri_core_retrieve_err_code(err_code,
                                                          &nas_get_sys_info_resp_msg->resp))
        {
            if(cri_nas_core_sys_info)
            {
                CRI_STORE_OPT_TLV(cri_nas_core_sys_info->cdma_srv_status_info,
                                  nas_get_sys_info_resp_msg->cdma_srv_status_info);
                CRI_STORE_OPT_TLV(cri_nas_core_sys_info->hdr_srv_status_info,
                                  nas_get_sys_info_resp_msg->hdr_srv_status_info);
                CRI_STORE_OPT_TLV(cri_nas_core_sys_info->gsm_srv_status_info,
                                  nas_get_sys_info_resp_msg->gsm_srv_status_info);
                CRI_STORE_OPT_TLV(cri_nas_core_sys_info->wcdma_srv_status_info,
                                  nas_get_sys_info_resp_msg->wcdma_srv_status_info);
                CRI_STORE_OPT_TLV(cri_nas_core_sys_info->tdscdma_srv_status_info,
                                  nas_get_sys_info_resp_msg->tdscdma_srv_status_info);
                CRI_STORE_OPT_TLV(cri_nas_core_sys_info->lte_srv_status_info,
                                  nas_get_sys_info_resp_msg->lte_srv_status_info);
                CRI_STORE_OPT_TLV(cri_nas_core_sys_info->cdma_sys_info,
                                  nas_get_sys_info_resp_msg->cdma_sys_info);
                CRI_STORE_OPT_TLV(cri_nas_core_sys_info->hdr_sys_info,
                                  nas_get_sys_info_resp_msg->hdr_sys_info);
                CRI_STORE_OPT_TLV(cri_nas_core_sys_info->gsm_sys_info,
                                  nas_get_sys_info_resp_msg->gsm_sys_info);
                CRI_STORE_OPT_TLV(cri_nas_core_sys_info->wcdma_sys_info,
                                  nas_get_sys_info_resp_msg->wcdma_sys_info);
                CRI_STORE_OPT_TLV(cri_nas_core_sys_info->tdscdma_sys_info,
                                  nas_get_sys_info_resp_msg->tdscdma_sys_info);
                CRI_STORE_OPT_TLV(cri_nas_core_sys_info->lte_sys_info,
                                  nas_get_sys_info_resp_msg->lte_sys_info);
                CRI_STORE_OPT_TLV(cri_nas_core_sys_info->gsm_sys_info3,
                                  nas_get_sys_info_resp_msg->gsm_sys_info3);
                CRI_STORE_OPT_TLV(cri_nas_core_sys_info->wcdma_sys_info3,
                                  nas_get_sys_info_resp_msg->wcdma_sys_info3);
                CRI_STORE_OPT_TLV(cri_nas_core_sys_info->voice_support_on_lte,
                                  nas_get_sys_info_resp_msg->voice_support_on_lte);
                CRI_STORE_OPT_TLV(cri_nas_core_sys_info->lte_cell_status,
                                  nas_get_sys_info_resp_msg->lte_cell_status);

                cri_nas_core_calculate_reg_status_for_all_rtes();
                cri_nas_calculate_voice_rte();
                cri_nas_update_voice_network_info();
                cri_nas_calculate_data_rte();
                cri_nas_update_data_network_info();
                // TODO: find out a better way of calling this function.
                cri_nas_update_nw_selection_info();
            }
            else
            {
                err_code = QMI_ERR_INTERNAL_V01;
            }
        }
        util_memory_free((void**)&nas_get_sys_info_resp_msg);
    }

    return err_code;
}


void cri_nas_core_sys_info_ind_handler(int qmi_service_client_id,
                                           nas_sys_info_ind_msg_v01 *ind_msg)
{
    if(ind_msg)
    {
        if(cri_nas_core_sys_info)
        {
            CRI_STORE_OPT_TLV(cri_nas_core_sys_info->cdma_srv_status_info,
                              ind_msg->cdma_srv_status_info);
            CRI_STORE_OPT_TLV(cri_nas_core_sys_info->hdr_srv_status_info,
                              ind_msg->hdr_srv_status_info);
            CRI_STORE_OPT_TLV(cri_nas_core_sys_info->gsm_srv_status_info,
                              ind_msg->gsm_srv_status_info);
            CRI_STORE_OPT_TLV(cri_nas_core_sys_info->wcdma_srv_status_info,
                              ind_msg->wcdma_srv_status_info);
            CRI_STORE_OPT_TLV(cri_nas_core_sys_info->tdscdma_srv_status_info,
                              ind_msg->tdscdma_srv_status_info);
            CRI_STORE_OPT_TLV(cri_nas_core_sys_info->lte_srv_status_info,
                              ind_msg->lte_srv_status_info);
            CRI_STORE_OPT_TLV(cri_nas_core_sys_info->cdma_sys_info,
                              ind_msg->cdma_sys_info);
            CRI_STORE_OPT_TLV(cri_nas_core_sys_info->hdr_sys_info,
                              ind_msg->hdr_sys_info);
            CRI_STORE_OPT_TLV(cri_nas_core_sys_info->gsm_sys_info,
                              ind_msg->gsm_sys_info);
            CRI_STORE_OPT_TLV(cri_nas_core_sys_info->wcdma_sys_info,
                              ind_msg->wcdma_sys_info);
            CRI_STORE_OPT_TLV(cri_nas_core_sys_info->tdscdma_sys_info,
                              ind_msg->tdscdma_sys_info);
            CRI_STORE_OPT_TLV(cri_nas_core_sys_info->lte_sys_info,
                              ind_msg->lte_sys_info);
            CRI_STORE_OPT_TLV(cri_nas_core_sys_info->gsm_sys_info3,
                              ind_msg->gsm_sys_info3);
            CRI_STORE_OPT_TLV(cri_nas_core_sys_info->wcdma_sys_info3,
                              ind_msg->wcdma_sys_info3);
            CRI_STORE_OPT_TLV(cri_nas_core_sys_info->voice_support_on_lte,
                              ind_msg->voice_support_on_lte);
            CRI_STORE_OPT_TLV(cri_nas_core_sys_info->lte_cell_status,
                              ind_msg->lte_cell_status);

            cri_nas_core_calculate_reg_status_for_all_rtes();
            cri_nas_calculate_voice_rte();
            cri_nas_update_voice_network_info();
            cri_nas_calculate_data_rte();
            cri_nas_update_data_network_info();
            // TODO: find out a better way of calling this function.
            cri_nas_update_nw_selection_info();
        }
    }
}

void cri_nas_core_update_gsm_sig_info(int8_t gsm_sig_info)
{
    if(cri_nas_rte_gsm_reg_info[CRI_NAS_RTE_SERVICE_VOICE])
    {
        cri_nas_rte_gsm_reg_info[CRI_NAS_RTE_SERVICE_VOICE]->gsm_sig_info = gsm_sig_info;
    }
    if(cri_nas_rte_gsm_reg_info[CRI_NAS_RTE_SERVICE_DATA])
    {
        cri_nas_rte_gsm_reg_info[CRI_NAS_RTE_SERVICE_DATA]->gsm_sig_info = gsm_sig_info;
    }
}

void cri_nas_core_update_wcdma_sig_info(nas_common_sig_info_param_type_v01 *wcdma_sig_info)
{
    if(cri_nas_rte_wcdma_reg_info[CRI_NAS_RTE_SERVICE_VOICE])
    {
        memcpy(&cri_nas_rte_wcdma_reg_info[CRI_NAS_RTE_SERVICE_VOICE]->wcdma_sig_info,
               wcdma_sig_info,
               sizeof(cri_nas_rte_wcdma_reg_info[CRI_NAS_RTE_SERVICE_VOICE]->wcdma_sig_info));
    }
    if(cri_nas_rte_wcdma_reg_info[CRI_NAS_RTE_SERVICE_DATA])
    {
        memcpy(&cri_nas_rte_wcdma_reg_info[CRI_NAS_RTE_SERVICE_DATA]->wcdma_sig_info,
               wcdma_sig_info,
               sizeof(cri_nas_rte_wcdma_reg_info[CRI_NAS_RTE_SERVICE_DATA]->wcdma_sig_info));
    }
}

void cri_nas_core_update_tdscdma_sig_info(nas_tdscdma_sig_info_type_v01 *tdscdma_sig_info)
{
    if(cri_nas_rte_tdscdma_reg_info[CRI_NAS_RTE_SERVICE_VOICE])
    {
        memcpy(&cri_nas_rte_tdscdma_reg_info[CRI_NAS_RTE_SERVICE_VOICE]->tdscdma_sig_info,
               tdscdma_sig_info,
               sizeof(cri_nas_rte_tdscdma_reg_info[CRI_NAS_RTE_SERVICE_VOICE]->tdscdma_sig_info));
    }
    if(cri_nas_rte_tdscdma_reg_info[CRI_NAS_RTE_SERVICE_DATA])
    {
        memcpy(&cri_nas_rte_tdscdma_reg_info[CRI_NAS_RTE_SERVICE_DATA]->tdscdma_sig_info,
               tdscdma_sig_info,
               sizeof(cri_nas_rte_tdscdma_reg_info[CRI_NAS_RTE_SERVICE_DATA]->tdscdma_sig_info));
    }
}

void cri_nas_core_update_lte_sig_info(nas_lte_sig_info_type_v01 *lte_sig_info)
{
    if(cri_nas_rte_lte_reg_info[CRI_NAS_RTE_SERVICE_VOICE])
    {
        memcpy(&cri_nas_rte_lte_reg_info[CRI_NAS_RTE_SERVICE_VOICE]->lte_sig_info,
               lte_sig_info,
               sizeof(cri_nas_rte_lte_reg_info[CRI_NAS_RTE_SERVICE_VOICE]->lte_sig_info));
    }
    if(cri_nas_rte_lte_reg_info[CRI_NAS_RTE_SERVICE_DATA])
    {
        memcpy(&cri_nas_rte_lte_reg_info[CRI_NAS_RTE_SERVICE_DATA]->lte_sig_info,
               lte_sig_info,
               sizeof(cri_nas_rte_lte_reg_info[CRI_NAS_RTE_SERVICE_DATA]->lte_sig_info));
    }
}

void cri_nas_core_update_cdma_sig_info(nas_common_sig_info_param_type_v01 *cdma_sig_info)
{
    if(cri_nas_rte_cdma_reg_info[CRI_NAS_RTE_SERVICE_VOICE])
    {
        memcpy(&cri_nas_rte_cdma_reg_info[CRI_NAS_RTE_SERVICE_VOICE]->cdma_sig_info,
               cdma_sig_info,
               sizeof(cri_nas_rte_cdma_reg_info[CRI_NAS_RTE_SERVICE_VOICE]->cdma_sig_info));
    }
    if(cri_nas_rte_cdma_reg_info[CRI_NAS_RTE_SERVICE_DATA])
    {
        memcpy(&cri_nas_rte_cdma_reg_info[CRI_NAS_RTE_SERVICE_DATA]->cdma_sig_info,
               cdma_sig_info,
               sizeof(cri_nas_rte_cdma_reg_info[CRI_NAS_RTE_SERVICE_DATA]->cdma_sig_info));
    }
}

void cri_nas_core_update_hdr_sig_info(nas_hdr_sig_info_type_v01 *hdr_sig_info)
{
    if(cri_nas_rte_hdr_reg_info[CRI_NAS_RTE_SERVICE_VOICE])
    {
        memcpy(&cri_nas_rte_hdr_reg_info[CRI_NAS_RTE_SERVICE_VOICE]->hdr_sig_info,
               hdr_sig_info,
               sizeof(cri_nas_rte_hdr_reg_info[CRI_NAS_RTE_SERVICE_VOICE]->hdr_sig_info));
    }
    if(cri_nas_rte_hdr_reg_info[CRI_NAS_RTE_SERVICE_DATA])
    {
        memcpy(&cri_nas_rte_hdr_reg_info[CRI_NAS_RTE_SERVICE_DATA]->hdr_sig_info,
               hdr_sig_info,
               sizeof(cri_nas_rte_hdr_reg_info[CRI_NAS_RTE_SERVICE_DATA]->hdr_sig_info));
    }
}


qmi_error_type_v01 cri_nas_core_fetch_update_sig_info()
{
    qmi_error_type_v01 err_code;
    nas_get_sig_info_resp_msg_v01 *nas_get_sig_info_resp_msg;

    err_code = QMI_ERR_INTERNAL_V01;
    nas_get_sig_info_resp_msg = NULL;

    nas_get_sig_info_resp_msg = util_memory_alloc(sizeof(*nas_get_sig_info_resp_msg));

    if(nas_get_sig_info_resp_msg)
    {
        err_code = cri_core_qmi_send_msg_sync(cri_nas_core_retrieve_client_id(),
                                              QMI_NAS_GET_SIG_INFO_REQ_MSG_V01,
                                              NULL,
                                              NIL,
                                              nas_get_sig_info_resp_msg,
                                              sizeof(*nas_get_sig_info_resp_msg),
                                              CRI_CORE_MINIMAL_TIMEOUT);

        if(CRI_ERR_NONE_V01 == cri_core_retrieve_err_code(err_code,
                                                          &nas_get_sig_info_resp_msg->resp))
        {
            if(nas_get_sig_info_resp_msg->gsm_sig_info_valid)
            {
                cri_nas_core_update_gsm_sig_info(nas_get_sig_info_resp_msg->gsm_sig_info);
            }
            if(nas_get_sig_info_resp_msg->wcdma_sig_info_valid)
            {
                cri_nas_core_update_wcdma_sig_info(&nas_get_sig_info_resp_msg->wcdma_sig_info);
            }
            if(nas_get_sig_info_resp_msg->tdscdma_sig_info_valid)
            {
                cri_nas_core_update_tdscdma_sig_info(&nas_get_sig_info_resp_msg->tdscdma_sig_info);
            }
            if(nas_get_sig_info_resp_msg->lte_sig_info_valid)
            {
                cri_nas_core_update_lte_sig_info(&nas_get_sig_info_resp_msg->lte_sig_info);
            }
            if(nas_get_sig_info_resp_msg->cdma_sig_info_valid)
            {
                cri_nas_core_update_cdma_sig_info(&nas_get_sig_info_resp_msg->cdma_sig_info);
            }
            if(nas_get_sig_info_resp_msg->hdr_sig_info_valid)
            {
                cri_nas_core_update_hdr_sig_info(&nas_get_sig_info_resp_msg->hdr_sig_info);
            }
        }

        util_memory_free((void**)&nas_get_sig_info_resp_msg);
    }

    return err_code;
}

void cri_nas_core_sig_info_ind_handler(int qmi_service_client_id,
                                           nas_sig_info_ind_msg_v01 *ind_msg)
{
    if(ind_msg)
    {
        if(ind_msg->gsm_sig_info_valid)
        {
            cri_nas_core_update_gsm_sig_info(ind_msg->gsm_sig_info);
        }
        if(ind_msg->wcdma_sig_info_valid)
        {
            cri_nas_core_update_wcdma_sig_info(&ind_msg->wcdma_sig_info);
        }
        if(ind_msg->tdscdma_sig_info_valid)
        {
            cri_nas_core_update_tdscdma_sig_info(&ind_msg->tdscdma_sig_info);
        }
        if(ind_msg->lte_sig_info_valid)
        {
            cri_nas_core_update_lte_sig_info(&ind_msg->lte_sig_info);
        }
        if(ind_msg->cdma_sig_info_valid)
        {
            cri_nas_core_update_cdma_sig_info(&ind_msg->cdma_sig_info);
        }
        if(ind_msg->hdr_sig_info_valid)
        {
            cri_nas_core_update_hdr_sig_info(&ind_msg->hdr_sig_info);
        }
    }
}

void cri_nas_core_managed_roaming_ind_handler(int qmi_service_client_id,
                                                  nas_managed_roaming_ind_msg_v01 *ind_msg)
{
    if(ind_msg)
    {
        if(cri_nas_core_managed_roaming_info)
        {
            CRI_STORE_OPT_TLV(cri_nas_core_managed_roaming_info->radio_if,
                              ind_msg->radio_if);

            cri_nas_core_calculate_reg_status_for_all_rtes();
            cri_nas_calculate_voice_rte();
            cri_nas_calculate_data_rte();
        }
    }
}

void cri_nas_core_nas_reject_ind_handler(int qmi_service_client_id,
                                                 nas_network_reject_ind_msg_v01 *ind_msg)
{
    if(ind_msg)
    {
        if(cri_nas_core_nas_reject_info)
        {
            CRI_STORE_MAND_TLV(cri_nas_core_nas_reject_info->radio_if,
                               ind_msg->radio_if);
            CRI_STORE_MAND_TLV(cri_nas_core_nas_reject_info->reject_srv_domain,
                               ind_msg->reject_srv_domain);
            CRI_STORE_MAND_TLV(cri_nas_core_nas_reject_info->rej_cause,
                               ind_msg->rej_cause);
            CRI_STORE_OPT_TLV(cri_nas_core_nas_reject_info->plmn_id,
                              ind_msg->plmn_id);
            CRI_STORE_OPT_TLV(cri_nas_core_nas_reject_info->csg_id,
                              ind_msg->csg_id);

            cri_nas_core_calculate_reg_status_for_all_rtes();
            cri_nas_calculate_voice_rte();
            cri_nas_calculate_data_rte();
        }
    }
}


void cri_nas_core_system_selection_ind_handler(int qmi_service_client_id,
                                                 nas_system_selection_preference_ind_msg_v01 *ind_msg)
{

    if( ind_msg )
    {

        CRI_STORE_OPT_TLV(cri_nas_sys_sel_pref_info.mode_pref,
                          ind_msg->mode_pref);

        CRI_STORE_OPT_TLV(cri_nas_sys_sel_pref_info.net_sel_pref,
                          ind_msg->net_sel_pref);

        CRI_STORE_OPT_TLV(cri_nas_sys_sel_pref_info.roam_pref,
                          ind_msg->roam_pref);

        CRI_STORE_OPT_TLV(cri_nas_sys_sel_pref_info.srv_reg_restriction,
                          ind_msg->srv_reg_restriction);

    }

}


cri_core_error_type cri_nas_core_set_pref_mode_resp_handler(
                        int qmi_service_client_id,
                        nas_set_system_selection_preference_resp_msg_v01 *qmi_resp_msg,
                        cri_core_context_type cri_core_context)
{
    cri_core_error_type cri_core_error;
    UTIL_LOG_MSG("cri_nas_core_set_pref_mode_resp_handler enter\n");
    cri_core_error = QMI_ERR_NONE_V01;

    if(qmi_resp_msg)
    {
        cri_core_error = cri_core_retrieve_err_code(QMI_ERR_NONE_V01, &qmi_resp_msg->resp);
        if ( cri_core_error == CRI_ERR_NONE_V01 )
        {
            cri_rule_handler_rule_check(cri_core_context, cri_core_error, qmi_resp_msg);
        }
        else
        {
            cri_nas_nw_selection_set_state(CRI_NAS_NW_SELECT_ROLLBACK);
        }
    }

    return cri_core_error;
}



void cri_nas_core_async_resp_handler(int qmi_service_client_id,
                                     unsigned long message_id,
                                     void *resp_data,
                                     int resp_data_len,
                                     cri_core_context_type cri_core_context)
{

    UTIL_LOG_MSG("entry");

    switch(message_id)
    {
        case QMI_NAS_SET_SYSTEM_SELECTION_PREFERENCE_RESP_MSG_V01:
            cri_nas_core_set_pref_mode_resp_handler(qmi_service_client_id,
                                                    resp_data,
                                                    cri_core_context);
        break;

        case QMI_NAS_PERFORM_NETWORK_SCAN_RESP_MSG_V01:
            cri_nas_core_nw_scan_resp_handler(qmi_service_client_id,
                                            resp_data,
                                            cri_core_context);
        break;


        default:
            //no action
            break;
    }

    UTIL_LOG_MSG("exit");

}

void cri_nas_core_unsol_ind_handler(int qmi_service_client_id,
                                        unsigned long message_id,
                                        void *ind_data,
                                        int ind_data_len)
{
    UTIL_LOG_MSG("entry %d", message_id);

    switch(message_id)
    {
        case QMI_NAS_SYS_INFO_IND_MSG_V01:
            cri_nas_core_sys_info_ind_handler(qmi_service_client_id,
                                                  ind_data);
            break;

        case QMI_NAS_SIG_INFO_IND_MSG_V01:
            cri_nas_core_sig_info_ind_handler(qmi_service_client_id,
                                                  ind_data);
            break;

        case QMI_NAS_MANAGED_ROAMING_IND_MSG_V01:
            cri_nas_core_managed_roaming_ind_handler(qmi_service_client_id,
                                                         ind_data);
            break;

        case QMI_NAS_NETWORK_REJECT_IND_V01:
            cri_nas_core_nas_reject_ind_handler(qmi_service_client_id,
                                                        ind_data);
            break;

        case QMI_NAS_SYSTEM_SELECTION_PREFERENCE_IND_MSG_V01:
            cri_nas_core_system_selection_ind_handler(qmi_service_client_id,
                                                        ind_data);
            break;

        case QMI_NAS_CURRENT_PLMN_NAME_IND_V01:
            // TODO: add logic to update operator name
            break;

        case QMI_NAS_NETWORK_TIME_IND_MSG_V01:
            // TODO:
            break;

        default:
            //no action
            break;
    }

    // check rule handler
    cri_rule_handler_rule_check(NIL,
                                QMI_ERR_NONE_V01,
                                NULL);

    cri_nas_send_update_to_clients(qmi_service_client_id,
                                    message_id,
                                    ind_data,
                                    ind_data_len);
    UTIL_LOG_MSG("exit");
}

int is_3gpp(cri_nas_rte_type rte)
{
    int ret = FALSE;
    if ( rte == CRI_NAS_RTE_GSM
            || rte == CRI_NAS_RTE_WCDMA
            || rte == CRI_NAS_RTE_LTE
            || rte == CRI_NAS_RTE_TDSCDMA )
    {
        ret = TRUE;
    }
    return ret;
}


void cri_nas_send_update_to_clients(int qmi_service_client_id,
                                        unsigned long message_id,
                                        void *ind_data,
                                        int ind_data_len)
{
    hlos_ind_cb_type hlos_ind_cb;

    hlos_ind_cb = cri_core_retrieve_hlos_ind_cb(qmi_service_client_id);
    if(hlos_ind_cb)
    {
        (*hlos_ind_cb) (message_id, NULL, NIL);
    }
    else
    {
        UTIL_LOG_MSG("No registered HLOS ind handler");
    }

}

uint8_t cri_nas_is_current_plmn(uint32_t mcc, uint32_t mnc)
{
    uint8_t ret = FALSE;
    char curr_mcc[CRI_NAS_MCC_MNC_MAX+1];
    char curr_mnc[CRI_NAS_MCC_MNC_MAX+1];

    cri_nas_get_current_mcc_mnc(curr_mcc,curr_mnc);

    if ( mcc == (uint32_t) atoi(curr_mcc) && mnc == (uint32_t) atoi(curr_mnc))
    {
        ret = TRUE;
    }
    return ret;
}


cri_nas_nw_select_state_type cri_nas_nw_selection_get_state()
{
    return cri_nas_nw_selection_info.final_state;
}


uint8_t cri_nas_is_nw_selection_in_progress()
{
    uint8_t ret = TRUE;
    switch(cri_nas_nw_selection_info.final_state )
    {
        case CRI_NAS_NW_SELECT_NONE:
            ret = FALSE;
        default:
            break;
    }

    return ret;
}


cri_core_error_type cri_nas_nw_selection_set_state(cri_nas_nw_select_state_type state)
{
    cri_core_error_type ret_val = CRI_ERR_INTERNAL_V01;

    switch(state)
    {

        case CRI_NAS_NW_SELECT_SETTING_PREF:
            cri_nas_nw_selection_info.final_state = CRI_NAS_NW_SELECT_SETTING_PREF;
            ret_val = CRI_ERR_NONE_V01;
            break;

        case CRI_NAS_NW_SELECT_SETTING_PREF_REGD:
            switch(cri_nas_nw_selection_info.curr_state )
            {
                case CRI_NAS_NW_SELECT_SETTING_PREF:
                case CRI_NAS_NW_SELECT_AWAITING_REG:
                case CRI_NAS_NW_SELECT_SETTING_PREF_LIMITED_REGD:
                    cri_nas_nw_selection_info.final_state = CRI_NAS_NW_SELECT_DONE;
                    ret_val = CRI_ERR_NONE_V01;
                    break;

                default:
                    break;
            }
            break;

        case CRI_NAS_NW_SELECT_SETTING_PREF_LIMITED_REGD:
            // TODO: Do we need to implement logic of waiting for 10sec for regd ?
            cri_nas_nw_selection_info.final_state = CRI_NAS_NW_SELECT_DONE;
            ret_val = CRI_ERR_NONE_V01;
            break;

        case CRI_NAS_NW_SELECT_AWAITING_REG:
            if ( cri_nas_nw_selection_info.curr_state == CRI_NAS_NW_SELECT_SETTING_PREF ||
                cri_nas_nw_selection_info.curr_state == CRI_NAS_NW_SELECT_SETTING_PREF_LIMITED_REGD )
            {
                cri_nas_nw_selection_info.final_state = CRI_NAS_NW_SELECT_AWAITING_REG;
                ret_val = CRI_ERR_NONE_V01;
            }
            break;

        case CRI_NAS_NW_SELECT_CANCELLING_REQ:
            switch ( cri_nas_nw_selection_info.curr_state )
            {
                case CRI_NAS_NW_SELECT_SETTING_PREF:        // fallthrough
                case CRI_NAS_NW_SELECT_SETTING_PREF_LIMITED_REGD:        // fallthrough
                case CRI_NAS_NW_SELECT_SETTING_PREF_REGD:   // fallthrough
                case CRI_NAS_NW_SELECT_AWAITING_REG:
                    cri_nas_nw_selection_info.final_state = CRI_NAS_NW_SELECT_NONE;
                    memset(&cri_nas_nw_selection_info,NIL,sizeof(cri_nas_nw_selection_info));
                    cri_nas_enable_networking_indications(is_screen_state_off,FALSE);
                    ret_val = CRI_ERR_NONE_V01;
                    break;

                default:
                    break;
            }
            break;

        case CRI_NAS_NW_SELECT_DONE:
            switch(cri_nas_nw_selection_info.curr_state)
            {
                case CRI_NAS_NW_SELECT_SETTING_PREF:
                case CRI_NAS_NW_SELECT_AWAITING_REG:
                case CRI_NAS_NW_SELECT_SETTING_PREF_REGD:
                case CRI_NAS_NW_SELECT_SETTING_PREF_LIMITED_REGD:
                    cri_nas_nw_selection_info.final_state = CRI_NAS_NW_SELECT_NONE;
                    memset(&cri_nas_nw_selection_info,NIL,sizeof(cri_nas_nw_selection_info));
                    cri_nas_enable_networking_indications(is_screen_state_off,FALSE);
                    ret_val = CRI_ERR_NONE_V01;
                    break;

                default:
                    break;
            }
            break;
        case CRI_NAS_NW_SELECT_ROLLBACK:
            cri_nas_nw_selection_info.final_state = CRI_NAS_NW_SELECT_NONE;
            memset(&cri_nas_nw_selection_info,NIL,sizeof(cri_nas_nw_selection_info));
            cri_nas_enable_networking_indications(is_screen_state_off,FALSE);
            ret_val = CRI_ERR_NONE_V01;
            break;

        default:
        break;
    }

    if ( ret_val == CRI_ERR_NONE_V01 )
    {
        cri_nas_nw_selection_info.curr_state = cri_nas_nw_selection_info.final_state;
    }

    return ret_val;
}



inline int cri_nas_core_is_contains_cs(nas_service_domain_enum_type_v01 domain_cap)
{
    int ret_code;

    ret_code = FALSE;

    switch(domain_cap)
    {
        case SYS_SRV_DOMAIN_CS_ONLY_V01:
        case SYS_SRV_DOMAIN_CS_PS_V01:
            ret_code = TRUE;
            break;

        default: //no action
            break;
    }

    return ret_code;
}

inline int cri_nas_core_is_contains_ps(nas_service_domain_enum_type_v01 domain_cap)
{
    int ret_code;

    ret_code = FALSE;

    switch(domain_cap)
    {
        case SYS_SRV_DOMAIN_PS_ONLY_V01:
        case SYS_SRV_DOMAIN_CS_PS_V01:
            ret_code = TRUE;
            break;

        default: //no action
            break;
    }

    return ret_code;
}

inline int cri_nas_core_is_radio_if_same_as_rte(nas_radio_if_enum_v01 radio_if,
                                                    cri_nas_rte_type rte)
{
    int ret_code;

    ret_code = FALSE;

    if(NAS_RADIO_IF_CDMA_1X_V01 == radio_if && CRI_NAS_RTE_CDMA == rte)
    {
        ret_code = TRUE;
    }
    else if(NAS_RADIO_IF_CDMA_1XEVDO_V01 == radio_if && CRI_NAS_RTE_HDR == rte)
    {
        ret_code = TRUE;
    }
    else if(NAS_RADIO_IF_GSM_V01 == radio_if && CRI_NAS_RTE_GSM == rte)
    {
        ret_code = TRUE;
    }
    else if(NAS_RADIO_IF_UMTS_V01 == radio_if && CRI_NAS_RTE_WCDMA == rte)
    {
        ret_code = TRUE;
    }
    else if(NAS_RADIO_IF_TDSCDMA_V01 == radio_if && CRI_NAS_RTE_TDSCDMA == rte)
    {
        ret_code = TRUE;
    }
    else if(NAS_RADIO_IF_LTE_V01 == radio_if && CRI_NAS_RTE_LTE == rte)
    {
        ret_code = TRUE;
    }

    return ret_code;
}

inline cri_nas_rte_reg_confidence_type cri_nas_core_convert_reg_status_to_confidence(
                                                  cri_nas_tech_reg_status_type reg_status)
{
    cri_nas_rte_reg_confidence_type reg_confidence;

    reg_confidence = CRI_NAS_RTE_REG_CONFIDENCE_UNKNOWN;

    switch(reg_status)
    {
        case CRI_NAS_TECH_REG_STATUS_REGISTERED_HOME:
        case CRI_NAS_TECH_REG_STATUS_REGISTERED_ROAMING:
            reg_confidence = CRI_NAS_RTE_REG_CONFIDENCE_FULL;
            break;

        case CRI_NAS_TECH_REG_STATUS_NOT_REGISTERED_LTD_SRV:
        case CRI_NAS_TECH_REG_STATUS_DENIED_LTD_SRV:
            reg_confidence = CRI_NAS_RTE_REG_CONFIDENCE_LIMITED;
            break;

        default:
            reg_confidence = CRI_NAS_RTE_REG_CONFIDENCE_NONE;
            break;
    }

    return reg_confidence;
}

void cri_nas_core_srv_status_check(int is_voice,
                                   nas_service_status_enum_type_v01 srv_status,
                                   int roam_status,
                                   cri_nas_rte_common_reg_info_type *reg_info)
{
    if(reg_info)
    {
        switch(srv_status) //basic service status check
        {
            case NAS_SYS_SRV_STATUS_SRV_V01:
                reg_info->reg_status = CRI_NAS_TECH_REG_STATUS_REGISTERED_HOME;
                if(roam_status)
                {
                    reg_info->reg_status = CRI_NAS_TECH_REG_STATUS_REGISTERED_ROAMING;
                    reg_info->roam_status = roam_status;
                }
                break;

            case NAS_SYS_SRV_STATUS_LIMITED_V01:
            case NAS_SYS_SRV_STATUS_LIMITED_REGIONAL_V01:
                if(is_voice)
                {
                    reg_info->reg_status = CRI_NAS_TECH_REG_STATUS_NOT_REGISTERED_LTD_SRV;
                }
                else
                {
                    reg_info->reg_status = CRI_NAS_TECH_REG_STATUS_NOT_REGISTERED_SEARCHING;
                }
                break;

            case NAS_SYS_SRV_STATUS_NO_SRV_V01:
                reg_info->reg_status = CRI_NAS_TECH_REG_STATUS_NOT_REGISTERED_SEARCHING;
                break;

            case NAS_SYS_SRV_STATUS_PWR_SAVE_V01:
                reg_info->reg_status = CRI_NAS_TECH_REG_STATUS_NOT_REGISTERED_NOT_SEARCHING;
                break;

            default:
                break;
        }
    }
}

void cri_nas_core_domain_cap_check(int is_voice,
                                   nas_service_domain_enum_type_v01 srv_domain,
                                   nas_service_domain_enum_type_v01 srv_cap,
                                   cri_nas_rte_common_reg_info_type *reg_info)
{
    if(reg_info)
    {
        if(is_voice)
        {
            switch(reg_info->reg_status)
            {
                case CRI_NAS_TECH_REG_STATUS_REGISTERED_HOME:
                case CRI_NAS_TECH_REG_STATUS_REGISTERED_ROAMING:
                case CRI_NAS_TECH_REG_STATUS_NOT_REGISTERED_LTD_SRV:
                    if(FALSE == cri_nas_core_is_contains_cs(srv_domain) ||
                       FALSE == cri_nas_core_is_contains_cs(srv_cap))
                    {
                        reg_info->reg_status =
                            CRI_NAS_TECH_REG_STATUS_NOT_REGISTERED_NOT_SEARCHING;
                    }
                    break;

                default:
                    break;
            }
        }
        else
        {
            switch(reg_info->reg_status)
            {
                case CRI_NAS_TECH_REG_STATUS_REGISTERED_HOME:
                case CRI_NAS_TECH_REG_STATUS_REGISTERED_ROAMING:
                case CRI_NAS_TECH_REG_STATUS_NOT_REGISTERED_LTD_SRV:
                    if(FALSE == cri_nas_core_is_contains_ps(srv_domain) ||
                       FALSE == cri_nas_core_is_contains_ps(srv_cap))
                    {
                        reg_info->reg_status =
                            CRI_NAS_TECH_REG_STATUS_NOT_REGISTERED_NOT_SEARCHING;
                    }
                    break;

                default:
                    break;
            }
        }
    }
}

int cri_nas_core_managed_roaming_check(cri_nas_rte_type rte,
                                       cri_nas_rte_common_reg_info_type *reg_info)
{
    int is_managed_roaming;

    is_managed_roaming = FALSE;

    if(reg_info)
    {
        switch(reg_info->reg_status) //managed roaming check
        {
            case CRI_NAS_TECH_REG_STATUS_NOT_REGISTERED_LTD_SRV:
            case CRI_NAS_TECH_REG_STATUS_NOT_REGISTERED_SEARCHING:
            case CRI_NAS_TECH_REG_STATUS_NOT_REGISTERED_NOT_SEARCHING:
                if(cri_nas_core_managed_roaming_info &&
                   cri_nas_core_managed_roaming_info->radio_if_valid)
                {
                    if(TRUE == cri_nas_core_is_radio_if_same_as_rte(
                                                    cri_nas_core_managed_roaming_info->radio_if,
                                                    rte)
                       )
                    {
                        reg_info->reg_status = CRI_NAS_TECH_REG_STATUS_DENIED_LTD_SRV;
                        reg_info->reg_reject_reason = CRI_NAS_MANAGED_ROAMING_CAUSE;
                        is_managed_roaming = TRUE;
                    }
                }
                break;

            default:
                break;
        }
    }

    return is_managed_roaming;
}

void cri_nas_core_reg_reject_check(int is_voice,
                                   cri_nas_rte_type rte,
                                   cri_nas_rte_common_reg_info_type *reg_info)
{
    if(reg_info && cri_nas_core_nas_reject_info)
    {
        if((TRUE == cri_nas_core_is_radio_if_same_as_rte(
                                        cri_nas_core_nas_reject_info->radio_if,
                                        rte))
           )
        {
            if(is_voice)
            {
                if(TRUE == cri_nas_core_is_contains_cs(
                        cri_nas_core_nas_reject_info->reject_srv_domain))
                {
                    switch(reg_info->reg_status)
                    {
                        case CRI_NAS_TECH_REG_STATUS_NOT_REGISTERED_LTD_SRV:
                            reg_info->reg_status = CRI_NAS_TECH_REG_STATUS_DENIED_LTD_SRV;
                            break;

                        default:
                            reg_info->reg_status = CRI_NAS_TECH_REG_STATUS_DENIED;
                            break;
                    }
                    reg_info->reg_reject_reason = cri_nas_core_nas_reject_info->rej_cause;
                }
            }
            else
            {
                if(TRUE == cri_nas_core_is_contains_ps(
                        cri_nas_core_nas_reject_info->reject_srv_domain))
                {
                    reg_info->reg_status = CRI_NAS_TECH_REG_STATUS_DENIED;
                    reg_info->reg_reject_reason = cri_nas_core_nas_reject_info->rej_cause;
                }
            }
        }
    }
}

void cri_nas_core_calculate_reg_status(int is_voice,
                                       cri_nas_rte_type rte,
                                     nas_service_status_enum_type_v01 srv_status,
                                     nas_service_domain_enum_type_v01 srv_domain,
                                     nas_service_domain_enum_type_v01 srv_cap,
                                     int is_ignore_domain_cap,
                                     int roam_status,
                                     nas_common_network_id_type_v01 *network_id,
                                     cri_nas_rte_common_reg_info_type *reg_info)
{
    int is_managed_roaming;

    is_managed_roaming = FALSE;

    if(reg_info)
    {
//        memset(reg_info, NIL, sizeof(*reg_info));

        cri_nas_core_srv_status_check(is_voice,
                                      srv_status,
                                      roam_status,
                                      reg_info);

        if(FALSE == is_ignore_domain_cap) //domain and capability check
        {
            cri_nas_core_domain_cap_check(is_voice,
                                          srv_domain,
                                          srv_cap,
                                          reg_info);
        }

        if(TRUE == is_voice)
        {
            is_managed_roaming = cri_nas_core_managed_roaming_check(rte,
                                                                    reg_info);
        }


        if(FALSE == is_managed_roaming) //nas reject check
        {
            cri_nas_core_reg_reject_check(is_voice,
                                          rte,
                                          reg_info);
        }

        reg_info->reg_confidence = cri_nas_core_convert_reg_status_to_confidence(
                                                                            reg_info->reg_status);

        if(network_id) //filling mcc mnc
        {
            cri_nas_fillup_mcc_mnc_helper(network_id->mcc,reg_info->mcc);
            cri_nas_fillup_mcc_mnc_helper(network_id->mnc,reg_info->mnc);
        }
    }
}


void cri_nas_core_calculate_gsm_reg_status()
{
    nas_service_status_enum_type_v01 srv_status;
    nas_service_domain_enum_type_v01 srv_domain;
    nas_service_domain_enum_type_v01 srv_cap;
    int roam_status;
    nas_common_network_id_type_v01 *network_id;

    srv_status = NAS_SYS_SRV_STATUS_NO_SRV_V01;
    srv_domain = SYS_SRV_DOMAIN_NO_SRV_V01;
    srv_cap = SYS_SRV_DOMAIN_NO_SRV_V01;
    roam_status = FALSE;
    network_id = NULL;

    if(cri_nas_core_sys_info)
    {
        if(cri_nas_core_sys_info->gsm_srv_status_info_valid)
        {
            srv_status = cri_nas_core_sys_info->gsm_srv_status_info.srv_status;
        }

        if(cri_nas_core_sys_info->gsm_sys_info_valid)
        {
            if(cri_nas_core_sys_info->gsm_sys_info.common_sys_info.srv_domain_valid)
            {
                srv_domain = cri_nas_core_sys_info->gsm_sys_info.common_sys_info.srv_domain;
            }
            if(cri_nas_core_sys_info->gsm_sys_info.common_sys_info.srv_capability_valid)
            {
                srv_cap = cri_nas_core_sys_info->gsm_sys_info.common_sys_info.srv_capability;
            }
            if(cri_nas_core_sys_info->gsm_sys_info.common_sys_info.roam_status_valid)
            {
                roam_status = cri_nas_core_sys_info->gsm_sys_info.common_sys_info.roam_status;
            }
            if(cri_nas_core_sys_info->gsm_sys_info.threegpp_specific_sys_info.network_id_valid)
            {
                network_id =
                    &cri_nas_core_sys_info->gsm_sys_info.threegpp_specific_sys_info.network_id;
            }
        }

        if(cri_nas_rte_gsm_reg_info[CRI_NAS_RTE_SERVICE_VOICE])
        {
            cri_nas_core_calculate_reg_status(TRUE,
                                            CRI_NAS_RTE_GSM,
                                            srv_status,
                                            srv_domain,
                                            srv_cap,
                                            FALSE,
                                            roam_status,
                                            network_id,
                                            &cri_nas_rte_gsm_reg_info[CRI_NAS_RTE_SERVICE_VOICE]->reg_info);

            cri_nas_rte_gsm_reg_info[CRI_NAS_RTE_SERVICE_VOICE]->reg_info.tech = CRI_NAS_TECH_GSM;

            if(cri_nas_core_sys_info->gsm_sys_info3_valid)
            {
                cri_nas_rte_gsm_reg_info[CRI_NAS_RTE_SERVICE_VOICE]->cell_access_status = cri_nas_core_sys_info->gsm_sys_info3.cs_bar_status;
            }
        }
        if(cri_nas_rte_gsm_reg_info[CRI_NAS_RTE_SERVICE_DATA])
        {
            cri_nas_core_calculate_reg_status(FALSE,
                                                CRI_NAS_RTE_GSM,
                                                        srv_status,
                                                        srv_domain,
                                                        srv_cap,
                                                        FALSE,
                                                        roam_status,
                                                        network_id,
                                                        &cri_nas_rte_gsm_reg_info[CRI_NAS_RTE_SERVICE_DATA]->reg_info);


            if(cri_nas_core_sys_info->gsm_sys_info_valid)
            {
                if(cri_nas_core_sys_info->gsm_sys_info.gsm_specific_sys_info.egprs_supp_valid &&
                   TRUE == cri_nas_core_sys_info->gsm_sys_info.gsm_specific_sys_info.egprs_supp)
                {
                    cri_nas_rte_gsm_reg_info[CRI_NAS_RTE_SERVICE_DATA]->reg_info.tech = CRI_NAS_TECH_EDGE;
                }
                else
                {
                    cri_nas_rte_gsm_reg_info[CRI_NAS_RTE_SERVICE_DATA]->reg_info.tech = CRI_NAS_TECH_GPRS;
                }

            }

            if(cri_nas_core_sys_info->gsm_sys_info3_valid)
            {
                cri_nas_rte_gsm_reg_info[CRI_NAS_RTE_SERVICE_DATA]->cell_access_status = cri_nas_core_sys_info->gsm_sys_info3.ps_bar_status;
            }
        }
    }
}

void cri_nas_core_calculate_hs_variant(uint8_t hs_call_status_valid,
                                        nas_hs_support_enum_type_v01 hs_call_status,
                                        uint8_t hs_ind_valid,
                                        nas_hs_support_enum_type_v01 hs_ind,
                                        cri_nas_tech_type *tech)
{
    nas_hs_support_enum_type_v01 loc_hs_ser;

    loc_hs_ser = NAS_HS_SUPPORT_ENUM_TYPE_MIN_ENUM_VAL_V01;

    if(tech)
    {
        if ( hs_ind_valid && hs_call_status_valid )
        {
          if( hs_ind > hs_call_status )
          {
            loc_hs_ser = hs_ind;
          }
          else
          {
            loc_hs_ser = hs_call_status;
          }
        }
        else if ( hs_ind_valid )
        {
          loc_hs_ser = hs_ind;
        }
        else if ( hs_call_status_valid )
        {
          loc_hs_ser = hs_call_status;
        }

        switch ( loc_hs_ser )
        {
          case SYS_HS_IND_HSDPA_SUPP_CELL_V01:
              *tech = CRI_NAS_TECH_HSDPA;
              break;

          case SYS_HS_IND_HSUPA_SUPP_CELL_V01:
              *tech = CRI_NAS_TECH_HSUPA;
              break;

          case SYS_HS_IND_HSDPA_HSUPA_SUPP_CELL_V01:
              *tech = CRI_NAS_TECH_HSPA;
              break;

          case SYS_HS_IND_HSDPAPLUS_SUPP_CELL_V01:
          case SYS_HS_IND_HSDPAPLUS_64QAM_SUPP_CELL_V01:
          case SYS_HS_IND_DC_HSDPAPLUS_SUPP_CELL_V01:
          case SYS_HS_IND_HSDPAPLUS_HSUPA_SUPP_CELL_V01:
          case SYS_HS_IND_DC_HSDPAPLUS_HSUPA_SUPP_CELL_V01:
          case SYS_HS_IND_HSDPAPLUS_64QAM_HSUPA_SUPP_CELL_V01:
              *tech = CRI_NAS_TECH_HSPA_PLUS;
              break;

          default:
              break;
        }
    }
}

void cri_nas_core_calculate_wcdma_reg_status()
{
    nas_service_status_enum_type_v01 srv_status;
    nas_service_domain_enum_type_v01 srv_domain;
    nas_service_domain_enum_type_v01 srv_cap;
    int roam_status;
    nas_common_network_id_type_v01 *network_id;

    srv_status = NAS_SYS_SRV_STATUS_NO_SRV_V01;
    srv_domain = SYS_SRV_DOMAIN_NO_SRV_V01;
    srv_cap = SYS_SRV_DOMAIN_NO_SRV_V01;
    roam_status = FALSE;
    network_id = NULL;

    if(cri_nas_core_sys_info)
    {
        if(cri_nas_core_sys_info->wcdma_srv_status_info_valid)
        {
            srv_status = cri_nas_core_sys_info->wcdma_srv_status_info.srv_status;
        }

        if(cri_nas_core_sys_info->wcdma_sys_info_valid)
        {
            if(cri_nas_core_sys_info->wcdma_sys_info.common_sys_info.srv_domain_valid)
            {
                srv_domain = cri_nas_core_sys_info->wcdma_sys_info.common_sys_info.srv_domain;
            }
            if(cri_nas_core_sys_info->wcdma_sys_info.common_sys_info.srv_capability_valid)
            {
                srv_cap = cri_nas_core_sys_info->wcdma_sys_info.common_sys_info.srv_capability;
            }
            if(cri_nas_core_sys_info->wcdma_sys_info.common_sys_info.roam_status_valid)
            {
                roam_status = cri_nas_core_sys_info->wcdma_sys_info.common_sys_info.roam_status;
            }
            if(cri_nas_core_sys_info->wcdma_sys_info.threegpp_specific_sys_info.network_id_valid)
            {
                network_id =
                    &cri_nas_core_sys_info->wcdma_sys_info.threegpp_specific_sys_info.network_id;
            }
        }

        if(cri_nas_rte_wcdma_reg_info[CRI_NAS_RTE_SERVICE_VOICE])
        {
            cri_nas_core_calculate_reg_status(TRUE,
                                                CRI_NAS_RTE_WCDMA,
                                                        srv_status,
                                                        srv_domain,
                                                        srv_cap,
                                                        FALSE,
                                                        roam_status,
                                                        network_id,
                                                        &cri_nas_rte_wcdma_reg_info[CRI_NAS_RTE_SERVICE_VOICE]->reg_info);

            cri_nas_rte_wcdma_reg_info[CRI_NAS_RTE_SERVICE_VOICE]->reg_info.tech = CRI_NAS_TECH_WCDMA;

            if(cri_nas_core_sys_info->wcdma_sys_info3_valid)
            {
                cri_nas_rte_wcdma_reg_info[CRI_NAS_RTE_SERVICE_VOICE]->cell_access_status = cri_nas_core_sys_info->wcdma_sys_info3.cs_bar_status;
            }
        }

        if(cri_nas_rte_wcdma_reg_info[CRI_NAS_RTE_SERVICE_DATA])
        {
            cri_nas_core_calculate_reg_status(FALSE,
                                                CRI_NAS_RTE_WCDMA,
                                                        srv_status,
                                                        srv_domain,
                                                        srv_cap,
                                                        FALSE,
                                                        roam_status,
                                                        network_id,
                                                        &cri_nas_rte_wcdma_reg_info[CRI_NAS_RTE_SERVICE_DATA]->reg_info);

            cri_nas_rte_wcdma_reg_info[CRI_NAS_RTE_SERVICE_DATA]->reg_info.tech = CRI_NAS_TECH_WCDMA;
            if(cri_nas_core_sys_info->wcdma_sys_info_valid)
            {
                cri_nas_core_calculate_hs_variant(cri_nas_core_sys_info->wcdma_sys_info.wcdma_specific_sys_info.hs_call_status_valid,
                                                  cri_nas_core_sys_info->wcdma_sys_info.wcdma_specific_sys_info.hs_call_status,
                                                  cri_nas_core_sys_info->wcdma_sys_info.wcdma_specific_sys_info.hs_ind_valid,
                                                  cri_nas_core_sys_info->wcdma_sys_info.wcdma_specific_sys_info.hs_ind,
                                                  &cri_nas_rte_wcdma_reg_info[CRI_NAS_RTE_SERVICE_DATA]->reg_info.tech);
            }

            if(cri_nas_core_sys_info->wcdma_sys_info3_valid)
            {
                cri_nas_rte_wcdma_reg_info[CRI_NAS_RTE_SERVICE_DATA]->cell_access_status = cri_nas_core_sys_info->wcdma_sys_info3.ps_bar_status;
            }
        }
    }
}

void cri_nas_core_calculate_tdscdma_reg_status()
{
    nas_service_status_enum_type_v01 srv_status;
    nas_service_domain_enum_type_v01 srv_domain;
    nas_service_domain_enum_type_v01 srv_cap;
    int roam_status;
    nas_common_network_id_type_v01 *network_id;

    srv_status = NAS_SYS_SRV_STATUS_NO_SRV_V01;
    srv_domain = SYS_SRV_DOMAIN_NO_SRV_V01;
    srv_cap = SYS_SRV_DOMAIN_NO_SRV_V01;
    roam_status = FALSE;
    network_id = NULL;

    if(cri_nas_core_sys_info)
    {
        if(cri_nas_core_sys_info->tdscdma_srv_status_info_valid)
        {
            srv_status = cri_nas_core_sys_info->tdscdma_srv_status_info.srv_status;
        }

        if(cri_nas_core_sys_info->tdscdma_sys_info_valid)
        {
            if(cri_nas_core_sys_info->tdscdma_sys_info.common_sys_info.srv_domain_valid)
            {
                srv_domain = cri_nas_core_sys_info->tdscdma_sys_info.common_sys_info.srv_domain;
            }
            if(cri_nas_core_sys_info->tdscdma_sys_info.common_sys_info.srv_capability_valid)
            {
                srv_cap = cri_nas_core_sys_info->tdscdma_sys_info.common_sys_info.srv_capability;
            }
            if(cri_nas_core_sys_info->tdscdma_sys_info.common_sys_info.roam_status_valid)
            {
                roam_status = cri_nas_core_sys_info->tdscdma_sys_info.common_sys_info.roam_status;
            }
            if(cri_nas_core_sys_info->tdscdma_sys_info.threegpp_specific_sys_info.network_id_valid)
            {
                network_id =
                    &cri_nas_core_sys_info->tdscdma_sys_info.threegpp_specific_sys_info.network_id;
            }
        }

        if(cri_nas_rte_tdscdma_reg_info[CRI_NAS_RTE_SERVICE_VOICE])
        {
            cri_nas_core_calculate_reg_status(TRUE,
                                                CRI_NAS_RTE_TDSCDMA,
                                                        srv_status,
                                                        srv_domain,
                                                        srv_cap,
                                                        FALSE,
                                                        roam_status,
                                                        network_id,
                                                        &cri_nas_rte_tdscdma_reg_info[CRI_NAS_RTE_SERVICE_VOICE]->reg_info);

            cri_nas_rte_tdscdma_reg_info[CRI_NAS_RTE_SERVICE_VOICE]->reg_info.tech = CRI_NAS_TECH_TDSCDMA;

            if(cri_nas_core_sys_info->tdscdma_sys_info_valid &&
               cri_nas_core_sys_info->tdscdma_sys_info.tdscdma_specific_sys_info.cs_bar_status_valid)
            {
                cri_nas_rte_tdscdma_reg_info[CRI_NAS_RTE_SERVICE_VOICE]->cell_access_status = cri_nas_core_sys_info->tdscdma_sys_info.tdscdma_specific_sys_info.cs_bar_status;
            }
        }

        if(cri_nas_rte_tdscdma_reg_info[CRI_NAS_RTE_SERVICE_DATA])
        {
            cri_nas_core_calculate_reg_status(FALSE,
                                                CRI_NAS_RTE_TDSCDMA,
                                                        srv_status,
                                                        srv_domain,
                                                        srv_cap,
                                                        FALSE,
                                                        roam_status,
                                                        network_id,
                                                        &cri_nas_rte_tdscdma_reg_info[CRI_NAS_RTE_SERVICE_DATA]->reg_info);

            cri_nas_rte_tdscdma_reg_info[CRI_NAS_RTE_SERVICE_DATA]->reg_info.tech = CRI_NAS_TECH_TDSCDMA;
            if(cri_nas_core_sys_info->tdscdma_sys_info_valid)
            {
                cri_nas_core_calculate_hs_variant(cri_nas_core_sys_info->tdscdma_sys_info.tdscdma_specific_sys_info.hs_call_status_valid,
                                                  cri_nas_core_sys_info->tdscdma_sys_info.tdscdma_specific_sys_info.hs_call_status,
                                                  cri_nas_core_sys_info->tdscdma_sys_info.tdscdma_specific_sys_info.hs_ind_valid,
                                                  cri_nas_core_sys_info->tdscdma_sys_info.tdscdma_specific_sys_info.hs_ind,
                                                  &cri_nas_rte_tdscdma_reg_info[CRI_NAS_RTE_SERVICE_DATA]->reg_info.tech);
            }

            if(cri_nas_core_sys_info->tdscdma_sys_info_valid &&
               cri_nas_core_sys_info->tdscdma_sys_info.tdscdma_specific_sys_info.ps_bar_status_valid)
            {
                cri_nas_rte_tdscdma_reg_info[CRI_NAS_RTE_SERVICE_DATA]->cell_access_status = cri_nas_core_sys_info->tdscdma_sys_info.tdscdma_specific_sys_info.ps_bar_status;
            }
        }
    }
}

void cri_nas_core_calculate_lte_reg_status()
{
    nas_service_status_enum_type_v01 srv_status;
    nas_service_domain_enum_type_v01 srv_domain;
    nas_service_domain_enum_type_v01 srv_cap;
    int roam_status;
    nas_common_network_id_type_v01 *network_id;
    int is_ignore_domain_cap;

    srv_status = NAS_SYS_SRV_STATUS_NO_SRV_V01;
    srv_domain = SYS_SRV_DOMAIN_NO_SRV_V01;
    srv_cap = SYS_SRV_DOMAIN_NO_SRV_V01;
    roam_status = FALSE;
    network_id = NULL;
    is_ignore_domain_cap = FALSE;

    if(cri_nas_core_sys_info)
    {
        if(cri_nas_core_sys_info->lte_srv_status_info_valid)
        {
            srv_status = cri_nas_core_sys_info->lte_srv_status_info.srv_status;
        }

        if(cri_nas_core_sys_info->lte_sys_info_valid)
        {
            if(cri_nas_core_sys_info->lte_sys_info.common_sys_info.srv_domain_valid)
            {
                srv_domain = cri_nas_core_sys_info->lte_sys_info.common_sys_info.srv_domain;
            }
            if(cri_nas_core_sys_info->lte_sys_info.common_sys_info.srv_capability_valid)
            {
                srv_cap = cri_nas_core_sys_info->lte_sys_info.common_sys_info.srv_capability;
            }
            if(cri_nas_core_sys_info->voice_support_on_lte_valid)
            {
                is_ignore_domain_cap = cri_nas_core_sys_info->voice_support_on_lte;
            }
            if(cri_nas_core_sys_info->lte_sys_info.common_sys_info.roam_status_valid)
            {
                roam_status = cri_nas_core_sys_info->lte_sys_info.common_sys_info.roam_status;
            }
            if(cri_nas_core_sys_info->lte_sys_info.threegpp_specific_sys_info.network_id_valid)
            {
                network_id =
                    &cri_nas_core_sys_info->lte_sys_info.threegpp_specific_sys_info.network_id;
            }
        }

        if(cri_nas_rte_lte_reg_info[CRI_NAS_RTE_SERVICE_VOICE])
        {
            cri_nas_core_calculate_reg_status(TRUE,
                                                CRI_NAS_RTE_LTE,
                                                        srv_status,
                                                        srv_domain,
                                                        srv_cap,
                                                        is_ignore_domain_cap,
                                                        roam_status,
                                                        network_id,
                                                        &cri_nas_rte_lte_reg_info[CRI_NAS_RTE_SERVICE_VOICE]->reg_info);

            cri_nas_rte_lte_reg_info[CRI_NAS_RTE_SERVICE_VOICE]->reg_info.tech = CRI_NAS_TECH_LTE;

            if(cri_nas_core_sys_info->lte_cell_status_valid)
            {
                cri_nas_rte_lte_reg_info[CRI_NAS_RTE_SERVICE_VOICE]->cell_access_status = cri_nas_core_sys_info->lte_cell_status;
            }
        }

        if(cri_nas_rte_lte_reg_info[CRI_NAS_RTE_SERVICE_DATA])
        {
            cri_nas_core_calculate_reg_status(FALSE,
                                                CRI_NAS_RTE_LTE,
                                                        srv_status,
                                                        srv_domain,
                                                        srv_cap,
                                                        FALSE,
                                                        roam_status,
                                                        network_id,
                                                        &cri_nas_rte_lte_reg_info[CRI_NAS_RTE_SERVICE_DATA]->reg_info);

            cri_nas_rte_lte_reg_info[CRI_NAS_RTE_SERVICE_DATA]->reg_info.tech = CRI_NAS_TECH_LTE;

            if(cri_nas_core_sys_info->lte_cell_status_valid)
            {
                cri_nas_rte_lte_reg_info[CRI_NAS_RTE_SERVICE_DATA]->cell_access_status = cri_nas_core_sys_info->lte_cell_status;
            }
        }


    }
}

void cri_nas_core_calculate_cdma_variant(uint8_t p_rev_in_use_valid,
                                        uint8_t p_rev_in_use,
                                        cri_nas_tech_type *tech)
{
    if(tech)
    {
        if ( p_rev_in_use_valid )
        {
            if ( p_rev_in_use >= 6 )
            {
                *tech = CRI_NAS_TECH_1xRTT;
            }
            else if ( p_rev_in_use >= 4  )
            {
                *tech = CRI_NAS_TECH_IS95B;
            }
            else
            {
                *tech = CRI_NAS_TECH_IS95A;
            }
        }
    }
}

void cri_nas_core_calculate_cdma_reg_status()
{
    nas_service_status_enum_type_v01 srv_status;
    nas_service_domain_enum_type_v01 srv_domain;
    nas_service_domain_enum_type_v01 srv_cap;
    int roam_status;
    nas_common_network_id_type_v01 *network_id;

    srv_status = NAS_SYS_SRV_STATUS_NO_SRV_V01;
    srv_domain = SYS_SRV_DOMAIN_NO_SRV_V01;
    srv_cap = SYS_SRV_DOMAIN_NO_SRV_V01;
    roam_status = FALSE;
    network_id = NULL;

    if(cri_nas_core_sys_info)
    {
        if(cri_nas_core_sys_info->cdma_srv_status_info_valid)
        {
            srv_status = cri_nas_core_sys_info->cdma_srv_status_info.srv_status;
        }

        if(cri_nas_core_sys_info->cdma_sys_info_valid)
        {
            if(cri_nas_core_sys_info->cdma_sys_info.common_sys_info.srv_domain_valid)
            {
                srv_domain = cri_nas_core_sys_info->cdma_sys_info.common_sys_info.srv_domain;
            }
            if(cri_nas_core_sys_info->cdma_sys_info.common_sys_info.srv_capability_valid)
            {
                srv_cap = cri_nas_core_sys_info->cdma_sys_info.common_sys_info.srv_capability;
            }
            if(cri_nas_core_sys_info->cdma_sys_info.common_sys_info.roam_status_valid)
            {
                roam_status = cri_nas_core_sys_info->cdma_sys_info.common_sys_info.roam_status;
            }
            if(cri_nas_core_sys_info->cdma_sys_info.cdma_specific_sys_info.network_id_valid)
            {
                network_id =
                    &cri_nas_core_sys_info->cdma_sys_info.cdma_specific_sys_info.network_id;
            }
        }

        if(cri_nas_rte_cdma_reg_info[CRI_NAS_RTE_SERVICE_VOICE])
        {
            cri_nas_core_calculate_reg_status(TRUE,
                                                CRI_NAS_RTE_CDMA,
                                                        srv_status,
                                                        srv_domain,
                                                        srv_cap,
                                                        FALSE,
                                                        roam_status,
                                                        network_id,
                                                        &cri_nas_rte_cdma_reg_info[CRI_NAS_RTE_SERVICE_VOICE]->reg_info);

            cri_nas_rte_cdma_reg_info[CRI_NAS_RTE_SERVICE_VOICE]->reg_info.tech = CRI_NAS_TECH_IS95A;
            if(cri_nas_core_sys_info->cdma_sys_info_valid)
            {
                cri_nas_core_calculate_cdma_variant(cri_nas_core_sys_info->cdma_sys_info.cdma_specific_sys_info.p_rev_in_use_valid,
                                                    cri_nas_core_sys_info->cdma_sys_info.cdma_specific_sys_info.p_rev_in_use,
                                                    &cri_nas_rte_cdma_reg_info[CRI_NAS_RTE_SERVICE_VOICE]->reg_info.tech);
            }
        }

        if(cri_nas_rte_cdma_reg_info[CRI_NAS_RTE_SERVICE_DATA])
        {
            cri_nas_core_calculate_reg_status(FALSE,
                                                CRI_NAS_RTE_CDMA,
                                                        srv_status,
                                                        srv_domain,
                                                        srv_cap,
                                                        FALSE,
                                                        roam_status,
                                                        network_id,
                                                        &cri_nas_rte_cdma_reg_info[CRI_NAS_RTE_SERVICE_DATA]->reg_info);

            cri_nas_rte_cdma_reg_info[CRI_NAS_RTE_SERVICE_DATA]->reg_info.tech = CRI_NAS_TECH_IS95A;
            if(cri_nas_core_sys_info->cdma_sys_info_valid)
            {
                cri_nas_core_calculate_cdma_variant(cri_nas_core_sys_info->cdma_sys_info.cdma_specific_sys_info.p_rev_in_use_valid,
                                                    cri_nas_core_sys_info->cdma_sys_info.cdma_specific_sys_info.p_rev_in_use,
                                                    &cri_nas_rte_cdma_reg_info[CRI_NAS_RTE_SERVICE_DATA]->reg_info.tech);
            }
        }
    }
}

void cri_nas_core_calculate_hdr_variant_and_reg_status(uint8_t hdr_personality_valid,
                                                       nas_hdr_personality_enum_type_v01 hdr_personality,
                                                       uint8_t hdr_active_prot_valid,
                                                       nas_hdr_active_prot_enum_type_v01 hdr_active_prot,
                                                       cri_nas_rte_common_reg_info_type *reg_info)
{
    if(reg_info)
    {
        if(hdr_personality_valid && hdr_active_prot_valid)
        {
            switch (hdr_personality)
            {
                case NAS_SYS_PERSONALITY_EHRPD_V01:
                    reg_info->tech = CRI_NAS_TECH_EHRPD;
                    break;

                default:
                    switch (hdr_active_prot)
                    {
                        case NAS_SYS_ACTIVE_PROT_HDR_REL0_V01:
                            reg_info->tech = CRI_NAS_TECH_EVDO_0;
                            break;

                        case NAS_SYS_ACTIVE_PROT_HDR_RELA_V01:
                            reg_info->tech = CRI_NAS_TECH_EVDO_A;
                            break;

                        case NAS_SYS_ACTIVE_PROT_HDR_RELB_V01:
                            reg_info->tech = CRI_NAS_TECH_EVDO_B;
                            break;

                        case NAS_SYS_ACTIVE_PROT_NONE_V01:
                        default:
                            reg_info->reg_status = CRI_NAS_TECH_REG_STATUS_NOT_REGISTERED_NOT_SEARCHING;
                            break;
                    }
                    break;
            }
        }
        else
        {
            reg_info->reg_status = CRI_NAS_TECH_REG_STATUS_NOT_REGISTERED_NOT_SEARCHING;
        }
    }
}

void cri_nas_core_calculate_hdr_reg_status()
{
    nas_service_status_enum_type_v01 srv_status;
    nas_service_domain_enum_type_v01 srv_domain;
    nas_service_domain_enum_type_v01 srv_cap;
    int roam_status;

    srv_status = NAS_SYS_SRV_STATUS_NO_SRV_V01;
    srv_domain = SYS_SRV_DOMAIN_NO_SRV_V01;
    srv_cap = SYS_SRV_DOMAIN_NO_SRV_V01;
    roam_status = FALSE;

    if(cri_nas_core_sys_info)
    {
        if(cri_nas_core_sys_info->hdr_srv_status_info_valid)
        {
            srv_status = cri_nas_core_sys_info->hdr_srv_status_info.srv_status;
        }

        if(cri_nas_core_sys_info->hdr_sys_info_valid)
        {
            if(cri_nas_core_sys_info->hdr_sys_info.common_sys_info.srv_domain_valid)
            {
                srv_domain = cri_nas_core_sys_info->hdr_sys_info.common_sys_info.srv_domain;
            }
            if(cri_nas_core_sys_info->hdr_sys_info.common_sys_info.srv_capability_valid)
            {
                srv_cap = cri_nas_core_sys_info->hdr_sys_info.common_sys_info.srv_capability;
            }
            if(cri_nas_core_sys_info->hdr_sys_info.common_sys_info.roam_status_valid)
            {
                roam_status = cri_nas_core_sys_info->hdr_sys_info.common_sys_info.roam_status;
            }
        }

        if(cri_nas_rte_hdr_reg_info[CRI_NAS_RTE_SERVICE_DATA])
        {
            cri_nas_core_calculate_reg_status(FALSE,
                                                CRI_NAS_RTE_HDR,
                                                        srv_status,
                                                        srv_domain,
                                                        srv_cap,
                                                        FALSE,
                                                        roam_status,
                                                        NULL,
                                                        &cri_nas_rte_hdr_reg_info[CRI_NAS_RTE_SERVICE_DATA]->reg_info);

            if(cri_nas_core_sys_info->hdr_sys_info_valid)
            {
                cri_nas_core_calculate_hdr_variant_and_reg_status(cri_nas_core_sys_info->hdr_sys_info.hdr_specific_sys_info.hdr_personality_valid,
                                                                  cri_nas_core_sys_info->hdr_sys_info.hdr_specific_sys_info.hdr_personality,
                                                                  cri_nas_core_sys_info->hdr_sys_info.hdr_specific_sys_info.hdr_active_prot_valid,
                                                                  cri_nas_core_sys_info->hdr_sys_info.hdr_specific_sys_info.hdr_active_prot,
                                                                  &cri_nas_rte_hdr_reg_info[CRI_NAS_RTE_SERVICE_DATA]->reg_info);
            }
        }
    }
}


void cri_nas_calculate_voice_rte()
{

    cri_nas_tech_reg_status_type gsm_reg_status;
    cri_nas_tech_reg_status_type wcdma_reg_status;
    cri_nas_tech_reg_status_type tdscdma_reg_status;
    cri_nas_tech_reg_status_type lte_reg_status;
    cri_nas_tech_reg_status_type cdma_reg_status;

    gsm_reg_status = CRI_NAS_TECH_REG_STATUS_UNKNOWN;
    wcdma_reg_status = CRI_NAS_TECH_REG_STATUS_UNKNOWN;
    tdscdma_reg_status = CRI_NAS_TECH_REG_STATUS_UNKNOWN;
    lte_reg_status = CRI_NAS_TECH_REG_STATUS_UNKNOWN;
    cdma_reg_status = CRI_NAS_TECH_REG_STATUS_UNKNOWN;

    if(cri_nas_rte_gsm_reg_info[CRI_NAS_RTE_SERVICE_VOICE] &&
       cri_nas_rte_wcdma_reg_info[CRI_NAS_RTE_SERVICE_VOICE] &&
       cri_nas_rte_tdscdma_reg_info[CRI_NAS_RTE_SERVICE_VOICE] &&
       cri_nas_rte_lte_reg_info[CRI_NAS_RTE_SERVICE_VOICE] &&
       cri_nas_rte_cdma_reg_info[CRI_NAS_RTE_SERVICE_VOICE])
    {
        gsm_reg_status = cri_nas_rte_gsm_reg_info[CRI_NAS_RTE_SERVICE_VOICE]->reg_info.reg_status;
        wcdma_reg_status = cri_nas_rte_wcdma_reg_info[CRI_NAS_RTE_SERVICE_VOICE]->reg_info.reg_status;
        tdscdma_reg_status = cri_nas_rte_tdscdma_reg_info[CRI_NAS_RTE_SERVICE_VOICE]->reg_info.reg_status;
        lte_reg_status = cri_nas_rte_lte_reg_info[CRI_NAS_RTE_SERVICE_VOICE]->reg_info.reg_status;
        cdma_reg_status = cri_nas_rte_cdma_reg_info[CRI_NAS_RTE_SERVICE_VOICE]->reg_info.reg_status;

        if(CRI_NAS_TECH_REG_STATUS_REGISTERED_HOME == wcdma_reg_status ||
           CRI_NAS_TECH_REG_STATUS_REGISTERED_ROAMING == wcdma_reg_status)
        {
            cri_nas_core_rte_info.cri_nas_voice_rte = CRI_NAS_RTE_WCDMA;
            cri_nas_core_rte_info.cri_nas_voice_rte_confidence = CRI_NAS_RTE_REG_CONFIDENCE_FULL;
        }
        else if(CRI_NAS_TECH_REG_STATUS_REGISTERED_HOME == tdscdma_reg_status ||
           CRI_NAS_TECH_REG_STATUS_REGISTERED_ROAMING == tdscdma_reg_status)
        {
            cri_nas_core_rte_info.cri_nas_voice_rte = CRI_NAS_RTE_TDSCDMA;
            cri_nas_core_rte_info.cri_nas_voice_rte_confidence = CRI_NAS_RTE_REG_CONFIDENCE_FULL;
        }
        else if(CRI_NAS_TECH_REG_STATUS_REGISTERED_HOME == gsm_reg_status ||
           CRI_NAS_TECH_REG_STATUS_REGISTERED_ROAMING == gsm_reg_status)
        {
            cri_nas_core_rte_info.cri_nas_voice_rte = CRI_NAS_RTE_GSM;
            cri_nas_core_rte_info.cri_nas_voice_rte_confidence = CRI_NAS_RTE_REG_CONFIDENCE_FULL;
        }
        else if(CRI_NAS_TECH_REG_STATUS_REGISTERED_HOME == cdma_reg_status ||
           CRI_NAS_TECH_REG_STATUS_REGISTERED_ROAMING == cdma_reg_status)
        {
            cri_nas_core_rte_info.cri_nas_voice_rte = CRI_NAS_RTE_CDMA;
            cri_nas_core_rte_info.cri_nas_voice_rte_confidence = CRI_NAS_RTE_REG_CONFIDENCE_FULL;
        }
        else if(CRI_NAS_TECH_REG_STATUS_REGISTERED_HOME == lte_reg_status ||
           CRI_NAS_TECH_REG_STATUS_REGISTERED_ROAMING == lte_reg_status)
        {
            cri_nas_core_rte_info.cri_nas_voice_rte = CRI_NAS_RTE_LTE;
            cri_nas_core_rte_info.cri_nas_voice_rte_confidence = CRI_NAS_RTE_REG_CONFIDENCE_FULL;
        }
        else if(CRI_NAS_TECH_REG_STATUS_DENIED_LTD_SRV == wcdma_reg_status)
        {
            cri_nas_core_rte_info.cri_nas_voice_rte = CRI_NAS_RTE_WCDMA;
            cri_nas_core_rte_info.cri_nas_voice_rte_confidence = CRI_NAS_RTE_REG_CONFIDENCE_LIMITED;
        }
        else if(CRI_NAS_TECH_REG_STATUS_DENIED_LTD_SRV == tdscdma_reg_status)
        {
            cri_nas_core_rte_info.cri_nas_voice_rte = CRI_NAS_RTE_TDSCDMA;
            cri_nas_core_rte_info.cri_nas_voice_rte_confidence = CRI_NAS_RTE_REG_CONFIDENCE_LIMITED;
        }
        else if(CRI_NAS_TECH_REG_STATUS_DENIED_LTD_SRV == gsm_reg_status)
        {
            cri_nas_core_rte_info.cri_nas_voice_rte = CRI_NAS_RTE_GSM;
            cri_nas_core_rte_info.cri_nas_voice_rte_confidence = CRI_NAS_RTE_REG_CONFIDENCE_LIMITED;
        }
        else if(CRI_NAS_TECH_REG_STATUS_DENIED_LTD_SRV == lte_reg_status)
        {
            cri_nas_core_rte_info.cri_nas_voice_rte = CRI_NAS_RTE_LTE;
            cri_nas_core_rte_info.cri_nas_voice_rte_confidence = CRI_NAS_RTE_REG_CONFIDENCE_LIMITED;
        }
        else if(CRI_NAS_TECH_REG_STATUS_NOT_REGISTERED_LTD_SRV == wcdma_reg_status)
        {
            cri_nas_core_rte_info.cri_nas_voice_rte = CRI_NAS_RTE_WCDMA;
            cri_nas_core_rte_info.cri_nas_voice_rte_confidence = CRI_NAS_RTE_REG_CONFIDENCE_LIMITED;
        }
        else if(CRI_NAS_TECH_REG_STATUS_NOT_REGISTERED_LTD_SRV == tdscdma_reg_status)
        {
            cri_nas_core_rte_info.cri_nas_voice_rte = CRI_NAS_RTE_TDSCDMA;
            cri_nas_core_rte_info.cri_nas_voice_rte_confidence = CRI_NAS_RTE_REG_CONFIDENCE_LIMITED;
        }
        else if(CRI_NAS_TECH_REG_STATUS_NOT_REGISTERED_LTD_SRV == gsm_reg_status)
        {
            cri_nas_core_rte_info.cri_nas_voice_rte = CRI_NAS_RTE_GSM;
            cri_nas_core_rte_info.cri_nas_voice_rte_confidence = CRI_NAS_RTE_REG_CONFIDENCE_LIMITED;
        }
        else if(CRI_NAS_TECH_REG_STATUS_NOT_REGISTERED_LTD_SRV == lte_reg_status)
        {
            cri_nas_core_rte_info.cri_nas_voice_rte = CRI_NAS_RTE_LTE;
            cri_nas_core_rte_info.cri_nas_voice_rte_confidence = CRI_NAS_RTE_REG_CONFIDENCE_LIMITED;
        }
        else if(CRI_NAS_TECH_REG_STATUS_DENIED == wcdma_reg_status)
        {
            cri_nas_core_rte_info.cri_nas_voice_rte = CRI_NAS_RTE_WCDMA;
            cri_nas_core_rte_info.cri_nas_voice_rte_confidence = CRI_NAS_RTE_REG_CONFIDENCE_NONE;
        }
        else if(CRI_NAS_TECH_REG_STATUS_DENIED == tdscdma_reg_status)
        {
            cri_nas_core_rte_info.cri_nas_voice_rte = CRI_NAS_RTE_TDSCDMA;
            cri_nas_core_rte_info.cri_nas_voice_rte_confidence = CRI_NAS_RTE_REG_CONFIDENCE_NONE;
        }
        else if(CRI_NAS_TECH_REG_STATUS_DENIED == gsm_reg_status)
        {
            cri_nas_core_rte_info.cri_nas_voice_rte = CRI_NAS_RTE_GSM;
            cri_nas_core_rte_info.cri_nas_voice_rte_confidence = CRI_NAS_RTE_REG_CONFIDENCE_NONE;
        }
        else if(CRI_NAS_TECH_REG_STATUS_DENIED == lte_reg_status)
        {
            cri_nas_core_rte_info.cri_nas_voice_rte = CRI_NAS_RTE_LTE;
            cri_nas_core_rte_info.cri_nas_voice_rte_confidence = CRI_NAS_RTE_REG_CONFIDENCE_NONE;
        }
        else if(CRI_NAS_TECH_REG_STATUS_NOT_REGISTERED_SEARCHING == wcdma_reg_status)
        {
            cri_nas_core_rte_info.cri_nas_voice_rte = CRI_NAS_RTE_WCDMA;
            cri_nas_core_rte_info.cri_nas_voice_rte_confidence = CRI_NAS_RTE_REG_CONFIDENCE_NONE;
        }
        else if(CRI_NAS_TECH_REG_STATUS_NOT_REGISTERED_SEARCHING == tdscdma_reg_status)
        {
            cri_nas_core_rte_info.cri_nas_voice_rte = CRI_NAS_RTE_TDSCDMA;
            cri_nas_core_rte_info.cri_nas_voice_rte_confidence = CRI_NAS_RTE_REG_CONFIDENCE_NONE;
        }
        else if(CRI_NAS_TECH_REG_STATUS_NOT_REGISTERED_SEARCHING == gsm_reg_status)
        {
            cri_nas_core_rte_info.cri_nas_voice_rte = CRI_NAS_RTE_GSM;
            cri_nas_core_rte_info.cri_nas_voice_rte_confidence = CRI_NAS_RTE_REG_CONFIDENCE_NONE;
        }
        else if(CRI_NAS_TECH_REG_STATUS_NOT_REGISTERED_SEARCHING == cdma_reg_status)
        {
            cri_nas_core_rte_info.cri_nas_voice_rte = CRI_NAS_RTE_CDMA;
            cri_nas_core_rte_info.cri_nas_voice_rte_confidence = CRI_NAS_RTE_REG_CONFIDENCE_NONE;
        }
        else if(CRI_NAS_TECH_REG_STATUS_NOT_REGISTERED_SEARCHING == lte_reg_status)
        {
            cri_nas_core_rte_info.cri_nas_voice_rte = CRI_NAS_RTE_LTE;
            cri_nas_core_rte_info.cri_nas_voice_rte_confidence = CRI_NAS_RTE_REG_CONFIDENCE_NONE;
        }
        else if(CRI_NAS_TECH_REG_STATUS_NOT_REGISTERED_NOT_SEARCHING == wcdma_reg_status)
        {
            cri_nas_core_rte_info.cri_nas_voice_rte = CRI_NAS_RTE_WCDMA;
            cri_nas_core_rte_info.cri_nas_voice_rte_confidence = CRI_NAS_RTE_REG_CONFIDENCE_NONE;
        }
        else if(CRI_NAS_TECH_REG_STATUS_NOT_REGISTERED_NOT_SEARCHING == tdscdma_reg_status)
        {
            cri_nas_core_rte_info.cri_nas_voice_rte = CRI_NAS_RTE_TDSCDMA;
            cri_nas_core_rte_info.cri_nas_voice_rte_confidence = CRI_NAS_RTE_REG_CONFIDENCE_NONE;
        }
        else if(CRI_NAS_TECH_REG_STATUS_NOT_REGISTERED_NOT_SEARCHING == gsm_reg_status)
        {
            cri_nas_core_rte_info.cri_nas_voice_rte = CRI_NAS_RTE_GSM;
            cri_nas_core_rte_info.cri_nas_voice_rte_confidence = CRI_NAS_RTE_REG_CONFIDENCE_NONE;
        }
        else if(CRI_NAS_TECH_REG_STATUS_NOT_REGISTERED_NOT_SEARCHING == cdma_reg_status)
        {
            cri_nas_core_rte_info.cri_nas_voice_rte = CRI_NAS_RTE_CDMA;
            cri_nas_core_rte_info.cri_nas_voice_rte_confidence = CRI_NAS_RTE_REG_CONFIDENCE_NONE;
        }
        else if(CRI_NAS_TECH_REG_STATUS_NOT_REGISTERED_NOT_SEARCHING == lte_reg_status)
        {
            cri_nas_core_rte_info.cri_nas_voice_rte = CRI_NAS_RTE_LTE;
            cri_nas_core_rte_info.cri_nas_voice_rte_confidence = CRI_NAS_RTE_REG_CONFIDENCE_NONE;
        }
        else
        {
            cri_nas_core_rte_info.cri_nas_voice_rte = CRI_NAS_RTE_WCDMA;
            cri_nas_core_rte_info.cri_nas_voice_rte_confidence = CRI_NAS_RTE_REG_CONFIDENCE_NONE;
        }

        UTIL_LOG_MSG("voice rte %d, confd %d",
                     cri_nas_core_rte_info.cri_nas_voice_rte,
                     cri_nas_core_rte_info.cri_nas_voice_rte_confidence);

        UTIL_LOG_MSG("voice reg status GSM %d, WCDMA %d, TDSCDMA %d, LTE %d, CDMA %d",
                     gsm_reg_status,
                     wcdma_reg_status,
                     tdscdma_reg_status,
                     lte_reg_status,
                     cdma_reg_status);
    }
}


void cri_nas_update_voice_network_info()
{

    char lac[CRI_NAS_LAC_MAX+1];
    char cid[CRI_NAS_CID_MAX+1];
    char psc[CRI_NAS_PSC_MAX + 1];
    char sid[CRI_NAS_SID_MAX + 1];
    char nid[CRI_NAS_NID_MAX + 1];
    char tac[CRI_NAS_TAC_MAX + 1];


    if ( cri_nas_core_rte_info.cri_nas_voice_rte == CRI_NAS_RTE_GSM )
    {
        if ( cri_nas_core_sys_info->gsm_sys_info.threegpp_specific_sys_info.lac_valid == TRUE )
        {
            snprintf(lac,
                    sizeof(lac),
                "%d",
                 cri_nas_core_sys_info->gsm_sys_info.threegpp_specific_sys_info.lac);

            strlcpy(cri_nas_rte_gsm_reg_info[CRI_NAS_RTE_SERVICE_VOICE]->lac,
                    lac,
                    sizeof(cri_nas_rte_gsm_reg_info[CRI_NAS_RTE_SERVICE_VOICE]->lac));
        }

        if ( cri_nas_core_sys_info->gsm_sys_info.threegpp_specific_sys_info.cell_id_valid == TRUE )
        {
            snprintf(cid,
                    sizeof(cid),
                 "%d",
                 cri_nas_core_sys_info->gsm_sys_info.threegpp_specific_sys_info.cell_id);

            strlcpy(cri_nas_rte_gsm_reg_info[CRI_NAS_RTE_SERVICE_VOICE]->cid,
                    cid,
                    sizeof(cri_nas_rte_gsm_reg_info[CRI_NAS_RTE_SERVICE_VOICE]->cid));
        }
    }

    if ( cri_nas_core_rte_info.cri_nas_voice_rte == CRI_NAS_RTE_WCDMA )
    {
        if ( cri_nas_core_sys_info->wcdma_sys_info.threegpp_specific_sys_info.lac_valid == TRUE )
        {
            snprintf(lac,
                     sizeof(lac),
                "%d",
                 cri_nas_core_sys_info->wcdma_sys_info.threegpp_specific_sys_info.lac);

            strlcpy(cri_nas_rte_wcdma_reg_info[CRI_NAS_RTE_SERVICE_VOICE]->lac,
                    lac,
                    sizeof(cri_nas_rte_wcdma_reg_info[CRI_NAS_RTE_SERVICE_VOICE]->lac));
        }

        if ( cri_nas_core_sys_info->wcdma_sys_info.threegpp_specific_sys_info.cell_id_valid == TRUE )
        {
            snprintf(cid,
                     sizeof(cid),
                 "%d",
                 cri_nas_core_sys_info->wcdma_sys_info.threegpp_specific_sys_info.cell_id);

            strlcpy(cri_nas_rte_wcdma_reg_info[CRI_NAS_RTE_SERVICE_VOICE]->cid,
                    cid,
                    sizeof(cri_nas_rte_wcdma_reg_info[CRI_NAS_RTE_SERVICE_VOICE]->cid));
        }
        if ( cri_nas_core_sys_info->wcdma_sys_info.wcdma_specific_sys_info.psc_valid == TRUE )
        {
            snprintf(psc,
                     sizeof(psc),
                 "%d",
                 cri_nas_core_sys_info->wcdma_sys_info.wcdma_specific_sys_info.psc);

            strlcpy(cri_nas_rte_wcdma_reg_info[CRI_NAS_RTE_SERVICE_VOICE]->psc,
                    psc,
                    sizeof(cri_nas_rte_wcdma_reg_info[CRI_NAS_RTE_SERVICE_VOICE]->psc));
        }
    }

    if ( cri_nas_core_rte_info.cri_nas_voice_rte == CRI_NAS_RTE_CDMA )
    {
        if ( cri_nas_core_sys_info->cdma_sys_info.cdma_specific_sys_info.cdma_sys_id_valid == TRUE )
        {
            snprintf(sid,
                     sizeof(sid),
                "%d",
                 cri_nas_core_sys_info->cdma_sys_info.cdma_specific_sys_info.cdma_sys_id.sid);

            strlcpy(cri_nas_rte_cdma_reg_info[CRI_NAS_RTE_SERVICE_VOICE]->sid,
                    sid,
                    sizeof(cri_nas_rte_cdma_reg_info[CRI_NAS_RTE_SERVICE_VOICE]->sid));

            snprintf(nid,
                     sizeof(nid),
                "%d",
                 cri_nas_core_sys_info->cdma_sys_info.cdma_specific_sys_info.cdma_sys_id.nid);

            strlcpy(cri_nas_rte_cdma_reg_info[CRI_NAS_RTE_SERVICE_VOICE]->nid,
                    nid,
                    sizeof(cri_nas_rte_cdma_reg_info[CRI_NAS_RTE_SERVICE_VOICE]->nid));
        }
    }

    if ( cri_nas_core_rte_info.cri_nas_voice_rte == CRI_NAS_RTE_LTE)
    {
        if ( cri_nas_core_sys_info->lte_sys_info.threegpp_specific_sys_info.lac_valid == TRUE )
        {
            snprintf(lac,
                     sizeof(lac),
                "%d",
                 cri_nas_core_sys_info->lte_sys_info.threegpp_specific_sys_info.lac);

            strlcpy(cri_nas_rte_lte_reg_info[CRI_NAS_RTE_SERVICE_VOICE]->lac,
                    lac,
                    sizeof(cri_nas_rte_lte_reg_info[CRI_NAS_RTE_SERVICE_VOICE]->lac));
        }

        if ( cri_nas_core_sys_info->lte_sys_info.threegpp_specific_sys_info.cell_id_valid == TRUE )
        {
            snprintf(cid,
                     sizeof(cid),
                 "%d",
                 cri_nas_core_sys_info->lte_sys_info.threegpp_specific_sys_info.cell_id);

            strlcpy(cri_nas_rte_lte_reg_info[CRI_NAS_RTE_SERVICE_VOICE]->cid,
                    cid,
                    sizeof(cri_nas_rte_lte_reg_info[CRI_NAS_RTE_SERVICE_VOICE]->cid));
        }
        if ( cri_nas_core_sys_info->lte_sys_info.lte_specific_sys_info.tac_valid == TRUE )
        {
            snprintf(tac,
                     sizeof(tac),
                 "%d",
                 cri_nas_core_sys_info->lte_sys_info.lte_specific_sys_info.tac);

            strlcpy(cri_nas_rte_lte_reg_info[CRI_NAS_RTE_SERVICE_VOICE]->tac,
                    psc,
                    sizeof(cri_nas_rte_lte_reg_info[CRI_NAS_RTE_SERVICE_VOICE]->tac));
        }
    }

}




void cri_nas_calculate_data_rte()
{
    cri_nas_tech_reg_status_type gsm_reg_status;
    cri_nas_tech_reg_status_type wcdma_reg_status;
    cri_nas_tech_reg_status_type tdscdma_reg_status;
    cri_nas_tech_reg_status_type lte_reg_status;
    cri_nas_tech_reg_status_type cdma_reg_status;
    cri_nas_tech_reg_status_type hdr_reg_status;
    cri_data_system_status_info_type* cri_data_system_status_info;

    gsm_reg_status = CRI_NAS_TECH_REG_STATUS_UNKNOWN;
    wcdma_reg_status = CRI_NAS_TECH_REG_STATUS_UNKNOWN;
    tdscdma_reg_status = CRI_NAS_TECH_REG_STATUS_UNKNOWN;
    lte_reg_status = CRI_NAS_TECH_REG_STATUS_UNKNOWN;
    cdma_reg_status = CRI_NAS_TECH_REG_STATUS_UNKNOWN;
    hdr_reg_status = CRI_NAS_TECH_REG_STATUS_UNKNOWN;
    cri_data_system_status_info = NULL;

    if(cri_nas_rte_gsm_reg_info[CRI_NAS_RTE_SERVICE_DATA] &&
       cri_nas_rte_wcdma_reg_info[CRI_NAS_RTE_SERVICE_DATA] &&
       cri_nas_rte_tdscdma_reg_info[CRI_NAS_RTE_SERVICE_DATA] &&
       cri_nas_rte_lte_reg_info[CRI_NAS_RTE_SERVICE_DATA] &&
       cri_nas_rte_cdma_reg_info[CRI_NAS_RTE_SERVICE_DATA] &&
       cri_nas_rte_hdr_reg_info[CRI_NAS_RTE_SERVICE_DATA])
    {
        cri_data_system_status_info = cri_data_retrieve_data_system_status();
        if(cri_data_system_status_info && cri_data_system_status_info->is_dsd)
        {
            switch(cri_data_system_status_info->data_rte)
            {
                case CRI_NAS_RTE_GSM:
                    cri_nas_rte_gsm_reg_info[CRI_NAS_RTE_SERVICE_DATA]->reg_info.reg_status =
                  (cri_nas_rte_gsm_reg_info[CRI_NAS_RTE_SERVICE_DATA]->reg_info.reg_status ==
                   CRI_NAS_TECH_REG_STATUS_REGISTERED_ROAMING) ?
                        CRI_NAS_TECH_REG_STATUS_REGISTERED_ROAMING :
                        CRI_NAS_TECH_REG_STATUS_REGISTERED_HOME;

                    cri_nas_rte_gsm_reg_info[CRI_NAS_RTE_SERVICE_DATA]->reg_info.tech =
                        cri_data_system_status_info->data_tech;
                    gsm_reg_status =
                        cri_nas_rte_gsm_reg_info[CRI_NAS_RTE_SERVICE_DATA]->reg_info.reg_status;
                    break;

                case CRI_NAS_RTE_WCDMA:
                    cri_nas_rte_wcdma_reg_info[CRI_NAS_RTE_SERVICE_DATA]->reg_info.reg_status =
                  (cri_nas_rte_wcdma_reg_info[CRI_NAS_RTE_SERVICE_DATA]->reg_info.reg_status ==
                   CRI_NAS_TECH_REG_STATUS_REGISTERED_ROAMING) ?
                        CRI_NAS_TECH_REG_STATUS_REGISTERED_ROAMING :
                        CRI_NAS_TECH_REG_STATUS_REGISTERED_HOME;

                    cri_nas_rte_wcdma_reg_info[CRI_NAS_RTE_SERVICE_DATA]->reg_info.tech =
                        cri_data_system_status_info->data_tech;
                    wcdma_reg_status =
                        cri_nas_rte_wcdma_reg_info[CRI_NAS_RTE_SERVICE_DATA]->reg_info.reg_status;
                    break;

                case CRI_NAS_RTE_TDSCDMA:
                    cri_nas_rte_tdscdma_reg_info[CRI_NAS_RTE_SERVICE_DATA]->reg_info.reg_status =
                  (cri_nas_rte_tdscdma_reg_info[CRI_NAS_RTE_SERVICE_DATA]->reg_info.reg_status ==
                   CRI_NAS_TECH_REG_STATUS_REGISTERED_ROAMING) ?
                        CRI_NAS_TECH_REG_STATUS_REGISTERED_ROAMING :
                        CRI_NAS_TECH_REG_STATUS_REGISTERED_HOME;

                    cri_nas_rte_tdscdma_reg_info[CRI_NAS_RTE_SERVICE_DATA]->reg_info.tech =
                        cri_data_system_status_info->data_tech;
                    tdscdma_reg_status =
                  cri_nas_rte_tdscdma_reg_info[CRI_NAS_RTE_SERVICE_DATA]->reg_info.reg_status;
                    break;

                case CRI_NAS_RTE_LTE:
                    cri_nas_rte_lte_reg_info[CRI_NAS_RTE_SERVICE_DATA]->reg_info.reg_status =
                  (cri_nas_rte_lte_reg_info[CRI_NAS_RTE_SERVICE_DATA]->reg_info.reg_status ==
                   CRI_NAS_TECH_REG_STATUS_REGISTERED_ROAMING) ?
                        CRI_NAS_TECH_REG_STATUS_REGISTERED_ROAMING :
                        CRI_NAS_TECH_REG_STATUS_REGISTERED_HOME;

                    cri_nas_rte_lte_reg_info[CRI_NAS_RTE_SERVICE_DATA]->reg_info.tech =
                        cri_data_system_status_info->data_tech;
                    lte_reg_status =
                  cri_nas_rte_lte_reg_info[CRI_NAS_RTE_SERVICE_DATA]->reg_info.reg_status;
                    break;

                case CRI_NAS_RTE_CDMA:
                    cri_nas_rte_cdma_reg_info[CRI_NAS_RTE_SERVICE_DATA]->reg_info.reg_status =
                  (cri_nas_rte_cdma_reg_info[CRI_NAS_RTE_SERVICE_DATA]->reg_info.reg_status ==
                   CRI_NAS_TECH_REG_STATUS_REGISTERED_ROAMING) ?
                        CRI_NAS_TECH_REG_STATUS_REGISTERED_ROAMING :
                        CRI_NAS_TECH_REG_STATUS_REGISTERED_HOME;

                    cri_nas_rte_cdma_reg_info[CRI_NAS_RTE_SERVICE_DATA]->reg_info.tech =
                        cri_data_system_status_info->data_tech;
                    cdma_reg_status =
                  cri_nas_rte_cdma_reg_info[CRI_NAS_RTE_SERVICE_DATA]->reg_info.reg_status;
                    break;

                case CRI_NAS_RTE_HDR:
                    cri_nas_rte_hdr_reg_info[CRI_NAS_RTE_SERVICE_DATA]->reg_info.reg_status =
                  (cri_nas_rte_hdr_reg_info[CRI_NAS_RTE_SERVICE_DATA]->reg_info.reg_status ==
                   CRI_NAS_TECH_REG_STATUS_REGISTERED_ROAMING) ?
                        CRI_NAS_TECH_REG_STATUS_REGISTERED_ROAMING :
                        CRI_NAS_TECH_REG_STATUS_REGISTERED_HOME;

                    cri_nas_rte_hdr_reg_info[CRI_NAS_RTE_SERVICE_DATA]->reg_info.tech =
                        cri_data_system_status_info->data_tech;
                    hdr_reg_status =
                  cri_nas_rte_hdr_reg_info[CRI_NAS_RTE_SERVICE_DATA]->reg_info.reg_status;
                    break;

                case CRI_NAS_RTE_UNKNOWN:
                default:
                    gsm_reg_status =
                  (cri_nas_rte_gsm_reg_info[CRI_NAS_RTE_SERVICE_DATA]->reg_info.reg_status !=
                   CRI_NAS_TECH_REG_STATUS_REGISTERED_ROAMING
                   &&
                   cri_nas_rte_gsm_reg_info[CRI_NAS_RTE_SERVICE_DATA]->reg_info.reg_status !=
                   CRI_NAS_TECH_REG_STATUS_REGISTERED_HOME) ?
                        cri_nas_rte_gsm_reg_info[CRI_NAS_RTE_SERVICE_DATA]->reg_info.reg_status :
                        CRI_NAS_TECH_REG_STATUS_NOT_REGISTERED_NOT_SEARCHING;

                    wcdma_reg_status =
                  (cri_nas_rte_wcdma_reg_info[CRI_NAS_RTE_SERVICE_DATA]->reg_info.reg_status !=
                   CRI_NAS_TECH_REG_STATUS_REGISTERED_ROAMING
                   &&
                   cri_nas_rte_wcdma_reg_info[CRI_NAS_RTE_SERVICE_DATA]->reg_info.reg_status !=
                   CRI_NAS_TECH_REG_STATUS_REGISTERED_HOME) ?
                        cri_nas_rte_wcdma_reg_info[CRI_NAS_RTE_SERVICE_DATA]->reg_info.reg_status :
                        CRI_NAS_TECH_REG_STATUS_NOT_REGISTERED_NOT_SEARCHING;

                    tdscdma_reg_status =
                  (cri_nas_rte_tdscdma_reg_info[CRI_NAS_RTE_SERVICE_DATA]->reg_info.reg_status !=
                   CRI_NAS_TECH_REG_STATUS_REGISTERED_ROAMING
                   &&
                   cri_nas_rte_tdscdma_reg_info[CRI_NAS_RTE_SERVICE_DATA]->reg_info.reg_status !=
                   CRI_NAS_TECH_REG_STATUS_REGISTERED_HOME) ?
                        cri_nas_rte_tdscdma_reg_info[CRI_NAS_RTE_SERVICE_DATA]->reg_info.reg_status :
                        CRI_NAS_TECH_REG_STATUS_NOT_REGISTERED_NOT_SEARCHING;

                    lte_reg_status =
                  (cri_nas_rte_lte_reg_info[CRI_NAS_RTE_SERVICE_DATA]->reg_info.reg_status !=
                   CRI_NAS_TECH_REG_STATUS_REGISTERED_ROAMING
                   &&
                   cri_nas_rte_lte_reg_info[CRI_NAS_RTE_SERVICE_DATA]->reg_info.reg_status !=
                   CRI_NAS_TECH_REG_STATUS_REGISTERED_HOME) ?
                        cri_nas_rte_lte_reg_info[CRI_NAS_RTE_SERVICE_DATA]->reg_info.reg_status :
                        CRI_NAS_TECH_REG_STATUS_NOT_REGISTERED_NOT_SEARCHING;

                    cdma_reg_status =
                  (cri_nas_rte_cdma_reg_info[CRI_NAS_RTE_SERVICE_DATA]->reg_info.reg_status !=
                   CRI_NAS_TECH_REG_STATUS_REGISTERED_ROAMING
                   &&
                   cri_nas_rte_cdma_reg_info[CRI_NAS_RTE_SERVICE_DATA]->reg_info.reg_status !=
                   CRI_NAS_TECH_REG_STATUS_REGISTERED_HOME) ?
                        cri_nas_rte_cdma_reg_info[CRI_NAS_RTE_SERVICE_DATA]->reg_info.reg_status :
                        CRI_NAS_TECH_REG_STATUS_NOT_REGISTERED_NOT_SEARCHING;

                    hdr_reg_status =
                  (cri_nas_rte_hdr_reg_info[CRI_NAS_RTE_SERVICE_DATA]->reg_info.reg_status !=
                   CRI_NAS_TECH_REG_STATUS_REGISTERED_ROAMING
                   &&
                   cri_nas_rte_hdr_reg_info[CRI_NAS_RTE_SERVICE_DATA]->reg_info.reg_status !=
                   CRI_NAS_TECH_REG_STATUS_REGISTERED_HOME) ?
                        cri_nas_rte_hdr_reg_info[CRI_NAS_RTE_SERVICE_DATA]->reg_info.reg_status :
                        CRI_NAS_TECH_REG_STATUS_NOT_REGISTERED_NOT_SEARCHING;
                    break;
            }
        }
        else
        {

            gsm_reg_status = cri_nas_rte_gsm_reg_info[CRI_NAS_RTE_SERVICE_DATA]->reg_info.reg_status;
            wcdma_reg_status = cri_nas_rte_wcdma_reg_info[CRI_NAS_RTE_SERVICE_DATA]->reg_info.reg_status;
            tdscdma_reg_status = cri_nas_rte_tdscdma_reg_info[CRI_NAS_RTE_SERVICE_DATA]->reg_info.reg_status;
            lte_reg_status = cri_nas_rte_lte_reg_info[CRI_NAS_RTE_SERVICE_DATA]->reg_info.reg_status;
            cdma_reg_status = cri_nas_rte_cdma_reg_info[CRI_NAS_RTE_SERVICE_DATA]->reg_info.reg_status;
            hdr_reg_status = cri_nas_rte_hdr_reg_info[CRI_NAS_RTE_SERVICE_DATA]->reg_info.reg_status;
        }

        if(CRI_NAS_TECH_REG_STATUS_REGISTERED_HOME == lte_reg_status ||
           CRI_NAS_TECH_REG_STATUS_REGISTERED_ROAMING == lte_reg_status)
        {
            cri_nas_core_rte_info.cri_nas_data_rte = CRI_NAS_RTE_LTE;
            cri_nas_core_rte_info.cri_nas_data_rte_confidence = CRI_NAS_RTE_REG_CONFIDENCE_FULL;
        }
        else if(CRI_NAS_TECH_REG_STATUS_REGISTERED_HOME == hdr_reg_status ||
           CRI_NAS_TECH_REG_STATUS_REGISTERED_ROAMING == hdr_reg_status)
        {
            cri_nas_core_rte_info.cri_nas_data_rte = CRI_NAS_RTE_HDR;
            cri_nas_core_rte_info.cri_nas_data_rte_confidence = CRI_NAS_RTE_REG_CONFIDENCE_FULL;
        }
        else if(CRI_NAS_TECH_REG_STATUS_REGISTERED_HOME == wcdma_reg_status ||
           CRI_NAS_TECH_REG_STATUS_REGISTERED_ROAMING == wcdma_reg_status)
        {
            cri_nas_core_rte_info.cri_nas_data_rte = CRI_NAS_RTE_WCDMA;
            cri_nas_core_rte_info.cri_nas_data_rte_confidence = CRI_NAS_RTE_REG_CONFIDENCE_FULL;
        }
        else if(CRI_NAS_TECH_REG_STATUS_REGISTERED_HOME == tdscdma_reg_status ||
           CRI_NAS_TECH_REG_STATUS_REGISTERED_ROAMING == tdscdma_reg_status)
        {
            cri_nas_core_rte_info.cri_nas_data_rte = CRI_NAS_RTE_TDSCDMA;
            cri_nas_core_rte_info.cri_nas_data_rte_confidence = CRI_NAS_RTE_REG_CONFIDENCE_FULL;
        }
        else if(CRI_NAS_TECH_REG_STATUS_REGISTERED_HOME == gsm_reg_status ||
           CRI_NAS_TECH_REG_STATUS_REGISTERED_ROAMING == gsm_reg_status)
        {
            cri_nas_core_rte_info.cri_nas_data_rte = CRI_NAS_RTE_GSM;
            cri_nas_core_rte_info.cri_nas_data_rte_confidence = CRI_NAS_RTE_REG_CONFIDENCE_FULL;
        }
        else if(CRI_NAS_TECH_REG_STATUS_REGISTERED_HOME == cdma_reg_status ||
           CRI_NAS_TECH_REG_STATUS_REGISTERED_ROAMING == cdma_reg_status)
        {
            cri_nas_core_rte_info.cri_nas_data_rte = CRI_NAS_RTE_CDMA;
            cri_nas_core_rte_info.cri_nas_data_rte_confidence = CRI_NAS_RTE_REG_CONFIDENCE_FULL;
        }
        else if(CRI_NAS_TECH_REG_STATUS_DENIED == lte_reg_status)
        {
            cri_nas_core_rte_info.cri_nas_data_rte = CRI_NAS_RTE_LTE;
            cri_nas_core_rte_info.cri_nas_data_rte_confidence = CRI_NAS_RTE_REG_CONFIDENCE_NONE;
        }
        else if(CRI_NAS_TECH_REG_STATUS_DENIED == wcdma_reg_status)
        {
            cri_nas_core_rte_info.cri_nas_data_rte = CRI_NAS_RTE_WCDMA;
            cri_nas_core_rte_info.cri_nas_data_rte_confidence = CRI_NAS_RTE_REG_CONFIDENCE_NONE;
        }
        else if(CRI_NAS_TECH_REG_STATUS_DENIED == tdscdma_reg_status)
        {
            cri_nas_core_rte_info.cri_nas_data_rte = CRI_NAS_RTE_TDSCDMA;
            cri_nas_core_rte_info.cri_nas_data_rte_confidence = CRI_NAS_RTE_REG_CONFIDENCE_NONE;
        }
        else if(CRI_NAS_TECH_REG_STATUS_DENIED == gsm_reg_status)
        {
            cri_nas_core_rte_info.cri_nas_data_rte = CRI_NAS_RTE_GSM;
            cri_nas_core_rte_info.cri_nas_data_rte_confidence = CRI_NAS_RTE_REG_CONFIDENCE_NONE;
        }
        else if(CRI_NAS_TECH_REG_STATUS_NOT_REGISTERED_SEARCHING == lte_reg_status)
        {
            cri_nas_core_rte_info.cri_nas_data_rte = CRI_NAS_RTE_LTE;
            cri_nas_core_rte_info.cri_nas_data_rte_confidence = CRI_NAS_RTE_REG_CONFIDENCE_NONE;
        }
        else if(CRI_NAS_TECH_REG_STATUS_NOT_REGISTERED_SEARCHING == hdr_reg_status)
        {
            cri_nas_core_rte_info.cri_nas_data_rte = CRI_NAS_RTE_HDR;
            cri_nas_core_rte_info.cri_nas_data_rte_confidence = CRI_NAS_RTE_REG_CONFIDENCE_NONE;
        }
        else if(CRI_NAS_TECH_REG_STATUS_NOT_REGISTERED_SEARCHING == wcdma_reg_status)
        {
            cri_nas_core_rte_info.cri_nas_data_rte = CRI_NAS_RTE_WCDMA;
            cri_nas_core_rte_info.cri_nas_data_rte_confidence = CRI_NAS_RTE_REG_CONFIDENCE_NONE;
        }
        else if(CRI_NAS_TECH_REG_STATUS_NOT_REGISTERED_SEARCHING == tdscdma_reg_status)
        {
            cri_nas_core_rte_info.cri_nas_data_rte = CRI_NAS_RTE_TDSCDMA;
            cri_nas_core_rte_info.cri_nas_data_rte_confidence = CRI_NAS_RTE_REG_CONFIDENCE_NONE;
        }
        else if(CRI_NAS_TECH_REG_STATUS_NOT_REGISTERED_SEARCHING == gsm_reg_status)
        {
            cri_nas_core_rte_info.cri_nas_data_rte = CRI_NAS_RTE_GSM;
            cri_nas_core_rte_info.cri_nas_data_rte_confidence = CRI_NAS_RTE_REG_CONFIDENCE_NONE;
        }
        else if(CRI_NAS_TECH_REG_STATUS_NOT_REGISTERED_SEARCHING == cdma_reg_status)
        {
            cri_nas_core_rte_info.cri_nas_data_rte = CRI_NAS_RTE_CDMA;
            cri_nas_core_rte_info.cri_nas_data_rte_confidence = CRI_NAS_RTE_REG_CONFIDENCE_NONE;
        }
        else if(CRI_NAS_TECH_REG_STATUS_NOT_REGISTERED_NOT_SEARCHING == lte_reg_status)
        {
            cri_nas_core_rte_info.cri_nas_data_rte = CRI_NAS_RTE_LTE;
            cri_nas_core_rte_info.cri_nas_data_rte_confidence = CRI_NAS_RTE_REG_CONFIDENCE_NONE;
        }
        else if(CRI_NAS_TECH_REG_STATUS_NOT_REGISTERED_NOT_SEARCHING == hdr_reg_status)
        {
            cri_nas_core_rte_info.cri_nas_data_rte = CRI_NAS_RTE_HDR;
            cri_nas_core_rte_info.cri_nas_data_rte_confidence = CRI_NAS_RTE_REG_CONFIDENCE_NONE;
        }
        else if(CRI_NAS_TECH_REG_STATUS_NOT_REGISTERED_NOT_SEARCHING == wcdma_reg_status)
        {
            cri_nas_core_rte_info.cri_nas_data_rte = CRI_NAS_RTE_WCDMA;
            cri_nas_core_rte_info.cri_nas_data_rte_confidence = CRI_NAS_RTE_REG_CONFIDENCE_NONE;
        }
        else if(CRI_NAS_TECH_REG_STATUS_NOT_REGISTERED_NOT_SEARCHING == tdscdma_reg_status)
        {
            cri_nas_core_rte_info.cri_nas_data_rte = CRI_NAS_RTE_TDSCDMA;
            cri_nas_core_rte_info.cri_nas_data_rte_confidence = CRI_NAS_RTE_REG_CONFIDENCE_NONE;
        }
        else if(CRI_NAS_TECH_REG_STATUS_NOT_REGISTERED_NOT_SEARCHING == gsm_reg_status)
        {
            cri_nas_core_rte_info.cri_nas_data_rte = CRI_NAS_RTE_GSM;
            cri_nas_core_rte_info.cri_nas_data_rte_confidence = CRI_NAS_RTE_REG_CONFIDENCE_NONE;
        }
        else if(CRI_NAS_TECH_REG_STATUS_NOT_REGISTERED_NOT_SEARCHING == cdma_reg_status)
        {
            cri_nas_core_rte_info.cri_nas_data_rte = CRI_NAS_RTE_CDMA;
            cri_nas_core_rte_info.cri_nas_data_rte_confidence = CRI_NAS_RTE_REG_CONFIDENCE_NONE;
        }
        else
        {
            cri_nas_core_rte_info.cri_nas_data_rte = CRI_NAS_RTE_LTE;
            cri_nas_core_rte_info.cri_nas_data_rte_confidence = CRI_NAS_RTE_REG_CONFIDENCE_NONE;
        }

        UTIL_LOG_MSG("data rte %d, confd %d",
                     cri_nas_core_rte_info.cri_nas_data_rte,
                     cri_nas_core_rte_info.cri_nas_data_rte_confidence);

        UTIL_LOG_MSG("data reg status, GSM %d, WCDMA %d, TDSCDMA %d, LTE %d, CDMA %d, HDR %d",
                     gsm_reg_status,
                     wcdma_reg_status,
                     tdscdma_reg_status,
                     lte_reg_status,
                     cdma_reg_status,
                     hdr_reg_status);
    }
}


void cri_nas_update_data_network_info()
{

    char lac[CRI_NAS_LAC_MAX+1];
    char cid[CRI_NAS_CID_MAX+1];
    char psc[CRI_NAS_PSC_MAX + 1];
    char sid[CRI_NAS_SID_MAX + 1];
    char nid[CRI_NAS_NID_MAX + 1];
    char tac[CRI_NAS_TAC_MAX + 1];

    if ( cri_nas_core_rte_info.cri_nas_voice_rte == CRI_NAS_RTE_GSM )
    {
        if ( cri_nas_core_sys_info->gsm_sys_info.threegpp_specific_sys_info.lac_valid == TRUE )
        {
            snprintf(lac,
                     sizeof(lac),
                "%d",
                 cri_nas_core_sys_info->gsm_sys_info.threegpp_specific_sys_info.lac);

            strlcpy(cri_nas_rte_gsm_reg_info[CRI_NAS_RTE_SERVICE_DATA]->lac,
                    lac,
                    sizeof(cri_nas_rte_gsm_reg_info[CRI_NAS_RTE_SERVICE_DATA]->lac));
        }

        if ( cri_nas_core_sys_info->gsm_sys_info.threegpp_specific_sys_info.cell_id_valid == TRUE )
        {
            snprintf(cid,
                     sizeof(cid),
                 "%d",
                 cri_nas_core_sys_info->gsm_sys_info.threegpp_specific_sys_info.cell_id);

            strlcpy(cri_nas_rte_gsm_reg_info[CRI_NAS_RTE_SERVICE_DATA]->cid,
                    cid,
                    sizeof(cri_nas_rte_gsm_reg_info[CRI_NAS_RTE_SERVICE_DATA]->cid));
        }
    }

    if ( cri_nas_core_rte_info.cri_nas_voice_rte == CRI_NAS_RTE_WCDMA )
    {
        if ( cri_nas_core_sys_info->wcdma_sys_info.threegpp_specific_sys_info.lac_valid == TRUE )
        {
            snprintf(lac,
                     sizeof(lac),
                "%d",
                 cri_nas_core_sys_info->wcdma_sys_info.threegpp_specific_sys_info.lac);

            strlcpy(cri_nas_rte_wcdma_reg_info[CRI_NAS_RTE_SERVICE_DATA]->lac,
                    lac,
                    sizeof(cri_nas_rte_wcdma_reg_info[CRI_NAS_RTE_SERVICE_DATA]->lac));
        }

        if ( cri_nas_core_sys_info->wcdma_sys_info.threegpp_specific_sys_info.cell_id_valid == TRUE )
        {
            snprintf(cid,
                     sizeof(cid),
                 "%d",
                 cri_nas_core_sys_info->wcdma_sys_info.threegpp_specific_sys_info.cell_id);

            strlcpy(cri_nas_rte_wcdma_reg_info[CRI_NAS_RTE_SERVICE_DATA]->cid,
                    cid,
                    sizeof(cri_nas_rte_wcdma_reg_info[CRI_NAS_RTE_SERVICE_DATA]->cid));
        }
        if ( cri_nas_core_sys_info->wcdma_sys_info.wcdma_specific_sys_info.psc_valid == TRUE )
        {
            snprintf(psc,
                     sizeof(psc),
                 "%d",
                 cri_nas_core_sys_info->wcdma_sys_info.wcdma_specific_sys_info.psc);

            strlcpy(cri_nas_rte_wcdma_reg_info[CRI_NAS_RTE_SERVICE_DATA]->psc,
                    psc,
                    sizeof(cri_nas_rte_wcdma_reg_info[CRI_NAS_RTE_SERVICE_DATA]->psc));
        }
    }

    if ( cri_nas_core_rte_info.cri_nas_voice_rte == CRI_NAS_RTE_CDMA )
    {
        if ( cri_nas_core_sys_info->cdma_sys_info.cdma_specific_sys_info.cdma_sys_id_valid == TRUE )
        {
            snprintf(sid,
                     sizeof(sid),
                "%d",
                 cri_nas_core_sys_info->cdma_sys_info.cdma_specific_sys_info.cdma_sys_id.sid);

            strlcpy(cri_nas_rte_cdma_reg_info[CRI_NAS_RTE_SERVICE_DATA]->sid,
                    sid,
                    sizeof(cri_nas_rte_cdma_reg_info[CRI_NAS_RTE_SERVICE_DATA]->sid));

            snprintf(nid,
                     sizeof(nid),
                "%d",
                 cri_nas_core_sys_info->cdma_sys_info.cdma_specific_sys_info.cdma_sys_id.nid);

            strlcpy(cri_nas_rte_cdma_reg_info[CRI_NAS_RTE_SERVICE_DATA]->nid,
                    nid,
                    sizeof(cri_nas_rte_cdma_reg_info[CRI_NAS_RTE_SERVICE_DATA]->nid));
        }
    }

    if ( cri_nas_core_rte_info.cri_nas_voice_rte == CRI_NAS_RTE_LTE)
    {
        if ( cri_nas_core_sys_info->lte_sys_info.threegpp_specific_sys_info.lac_valid == TRUE )
        {
            snprintf(lac,
                     sizeof(lac),
                "%d",
                 cri_nas_core_sys_info->lte_sys_info.threegpp_specific_sys_info.lac);

            strlcpy(cri_nas_rte_lte_reg_info[CRI_NAS_RTE_SERVICE_DATA]->lac,
                    lac,
                    sizeof(cri_nas_rte_lte_reg_info[CRI_NAS_RTE_SERVICE_DATA]->lac));
        }

        if ( cri_nas_core_sys_info->lte_sys_info.threegpp_specific_sys_info.cell_id_valid == TRUE )
        {
            snprintf(cid,
                     sizeof(cid),
                 "%d",
                 cri_nas_core_sys_info->lte_sys_info.threegpp_specific_sys_info.cell_id);

            strlcpy(cri_nas_rte_lte_reg_info[CRI_NAS_RTE_SERVICE_DATA]->cid,
                    cid,
                    sizeof(cri_nas_rte_lte_reg_info[CRI_NAS_RTE_SERVICE_DATA]->cid));
        }
        if ( cri_nas_core_sys_info->lte_sys_info.lte_specific_sys_info.tac_valid == TRUE )
        {
            snprintf(tac,
                     sizeof(tac),
                 "%d",
                 cri_nas_core_sys_info->lte_sys_info.lte_specific_sys_info.tac);

            strlcpy(cri_nas_rte_lte_reg_info[CRI_NAS_RTE_SERVICE_DATA]->tac,
                    psc,
                    sizeof(cri_nas_rte_lte_reg_info[CRI_NAS_RTE_SERVICE_DATA]->tac));
        }
    }

}


void* cri_nas_retrieve_voice_rte(cri_nas_rte_type *rte)
{
    void *voice_rte_info;

    voice_rte_info = NULL;

    if(rte)
    {
        *rte = CRI_NAS_RTE_UNKNOWN;
        if(cri_nas_rte_gsm_reg_info[CRI_NAS_RTE_SERVICE_VOICE] &&
           cri_nas_rte_wcdma_reg_info[CRI_NAS_RTE_SERVICE_VOICE] &&
           cri_nas_rte_tdscdma_reg_info[CRI_NAS_RTE_SERVICE_VOICE] &&
           cri_nas_rte_lte_reg_info[CRI_NAS_RTE_SERVICE_VOICE] &&
           cri_nas_rte_cdma_reg_info[CRI_NAS_RTE_SERVICE_VOICE])
        {
            *rte = cri_nas_core_rte_info.cri_nas_voice_rte;
            switch(cri_nas_core_rte_info.cri_nas_voice_rte)
            {
                case CRI_NAS_RTE_GSM:
                    voice_rte_info = cri_nas_rte_gsm_reg_info[CRI_NAS_RTE_SERVICE_VOICE];
                    break;

                case CRI_NAS_RTE_WCDMA:
                    voice_rte_info = cri_nas_rte_wcdma_reg_info[CRI_NAS_RTE_SERVICE_VOICE];
                    break;

                case CRI_NAS_RTE_TDSCDMA:
                    voice_rte_info = cri_nas_rte_tdscdma_reg_info[CRI_NAS_RTE_SERVICE_VOICE];
                    break;

                case CRI_NAS_RTE_LTE:
                    voice_rte_info = cri_nas_rte_lte_reg_info[CRI_NAS_RTE_SERVICE_VOICE];
                    break;

                case CRI_NAS_RTE_CDMA:
                    voice_rte_info = cri_nas_rte_cdma_reg_info[CRI_NAS_RTE_SERVICE_VOICE];
                    break;

                default:
                    break;
            }
            UTIL_LOG_MSG("voice rte %d",
                         *rte);
        }
    }

    return voice_rte_info;
}

void* cri_nas_retrieve_data_rte(cri_nas_rte_type *rte)
{
    void *data_rte_info;

    data_rte_info = NULL;

    if(rte)
    {
        *rte = CRI_NAS_RTE_UNKNOWN;
        if(cri_nas_rte_gsm_reg_info[CRI_NAS_RTE_SERVICE_DATA] &&
           cri_nas_rte_wcdma_reg_info[CRI_NAS_RTE_SERVICE_DATA] &&
           cri_nas_rte_tdscdma_reg_info[CRI_NAS_RTE_SERVICE_DATA] &&
           cri_nas_rte_lte_reg_info[CRI_NAS_RTE_SERVICE_DATA] &&
           cri_nas_rte_cdma_reg_info[CRI_NAS_RTE_SERVICE_DATA] &&
           cri_nas_rte_hdr_reg_info[CRI_NAS_RTE_SERVICE_DATA])
        {
            *rte = cri_nas_core_rte_info.cri_nas_data_rte;
            switch(cri_nas_core_rte_info.cri_nas_data_rte)
            {
                case CRI_NAS_RTE_GSM:
                    data_rte_info = cri_nas_rte_gsm_reg_info[CRI_NAS_RTE_SERVICE_DATA];
                    break;

                case CRI_NAS_RTE_WCDMA:
                    data_rte_info = cri_nas_rte_wcdma_reg_info[CRI_NAS_RTE_SERVICE_DATA];
                    break;

                case CRI_NAS_RTE_TDSCDMA:
                    data_rte_info = cri_nas_rte_tdscdma_reg_info[CRI_NAS_RTE_SERVICE_DATA];
                    break;

                case CRI_NAS_RTE_LTE:
                    data_rte_info = cri_nas_rte_lte_reg_info[CRI_NAS_RTE_SERVICE_DATA];
                    break;

                case CRI_NAS_RTE_CDMA:
                    data_rte_info = cri_nas_rte_cdma_reg_info[CRI_NAS_RTE_SERVICE_DATA];
                    break;

                case CRI_NAS_RTE_HDR:
                    data_rte_info = cri_nas_rte_hdr_reg_info[CRI_NAS_RTE_SERVICE_DATA];
                    break;

                default:
                    break;
            }
            UTIL_LOG_MSG("data rte %d",
                         *rte);
        }
    }

    return data_rte_info;
}

void cri_nas_core_calculate_reg_status_for_all_rtes()
{
    cri_nas_core_calculate_gsm_reg_status();
    cri_nas_core_calculate_wcdma_reg_status();
    cri_nas_core_calculate_tdscdma_reg_status();
    cri_nas_core_calculate_lte_reg_status();
    cri_nas_core_calculate_cdma_reg_status();
    cri_nas_core_calculate_hdr_reg_status();
}


cri_nas_tech_reg_status_type cri_nas_retrieve_reg_state(uint8_t is_voice)
{
    void *reg_info;
    cri_nas_rte_type rte;
    cri_nas_tech_reg_status_type reg_status;
    cri_nas_rte_reg_confidence_type reg_confidence;

    cri_nas_rte_gsm_reg_info_type *gsm_reg_info;
    cri_nas_rte_wcdma_reg_info_type *wcdma_reg_info;
    cri_nas_rte_tdscdma_reg_info_type *tdscdma_reg_info;
    cri_nas_rte_lte_reg_info_type *lte_reg_info;
    cri_nas_rte_cdma_reg_info_type *cdma_reg_info;
    cri_nas_rte_hdr_reg_info_type *hdr_reg_info;

    if ( is_voice )
    {
        reg_info = cri_nas_retrieve_voice_rte(&rte);
    }
    else
    {
        reg_info = cri_nas_retrieve_data_rte(&rte);
    }
    if(reg_info)
    {
        switch(rte)
        {
            case CRI_NAS_RTE_GSM:
                gsm_reg_info = (cri_nas_rte_gsm_reg_info_type*)reg_info;
                reg_status = gsm_reg_info->reg_info.reg_status;
                reg_confidence = gsm_reg_info->reg_info.reg_confidence;
                break;

            case CRI_NAS_RTE_WCDMA:
                wcdma_reg_info = (cri_nas_rte_wcdma_reg_info_type*)reg_info;
                reg_status = wcdma_reg_info->reg_info.reg_status;
                reg_confidence = wcdma_reg_info->reg_info.reg_confidence;
                break;

            case CRI_NAS_RTE_TDSCDMA:
                tdscdma_reg_info = (cri_nas_rte_tdscdma_reg_info_type*)reg_info;
                reg_status = tdscdma_reg_info->reg_info.reg_status;
                reg_confidence = tdscdma_reg_info->reg_info.reg_confidence;
                break;

            case CRI_NAS_RTE_LTE:
                lte_reg_info = (cri_nas_rte_lte_reg_info_type*)reg_info;
                reg_status = lte_reg_info->reg_info.reg_status;
                reg_confidence = lte_reg_info->reg_info.reg_confidence;
                break;

            case CRI_NAS_RTE_CDMA:
                cdma_reg_info = (cri_nas_rte_cdma_reg_info_type*)reg_info;
                reg_status = cdma_reg_info->reg_info.reg_status;
                reg_confidence = cdma_reg_info->reg_info.reg_confidence;
                break;

            case CRI_NAS_RTE_HDR:
                hdr_reg_info = (cri_nas_rte_hdr_reg_info_type*)reg_info;
                reg_status = hdr_reg_info->reg_info.reg_status;
                reg_confidence = hdr_reg_info->reg_info.reg_confidence;
                break;

            default:
                break;
        }
    }

    return reg_status;
}

cri_nas_tech_reg_status_type cri_nas_is_considered_registered()
{
    uint8_t ret_val = FALSE;
    cri_nas_tech_reg_status_type voice_reg_status = CRI_NAS_TECH_REG_STATUS_UNKNOWN;
    cri_nas_tech_reg_status_type data_reg_status = CRI_NAS_TECH_REG_STATUS_UNKNOWN;

    voice_reg_status = cri_nas_retrieve_reg_state(TRUE);
    if ( voice_reg_status == CRI_NAS_TECH_REG_STATUS_REGISTERED_HOME ||
            voice_reg_status == CRI_NAS_TECH_REG_STATUS_REGISTERED_ROAMING )
    {
        ret_val = TRUE;
    }

    if ( !ret_val )
    {
        data_reg_status = cri_nas_retrieve_reg_state(FALSE);
        if ( data_reg_status == CRI_NAS_TECH_REG_STATUS_REGISTERED_HOME ||
                data_reg_status == CRI_NAS_TECH_REG_STATUS_REGISTERED_ROAMING )
        {
            ret_val = TRUE;
        }
    }
    return ret_val;
}

cri_nas_tech_reg_status_type cri_nas_is_considered_limited_service(void)
{
    uint8_t ret_val = FALSE;
    cri_nas_tech_reg_status_type voice_reg_status = CRI_NAS_TECH_REG_STATUS_UNKNOWN;
    cri_nas_tech_reg_status_type data_reg_status = CRI_NAS_TECH_REG_STATUS_UNKNOWN;

    voice_reg_status = cri_nas_retrieve_reg_state(TRUE);
    if ( voice_reg_status == CRI_NAS_TECH_REG_STATUS_NOT_REGISTERED_LTD_SRV ||
            voice_reg_status == CRI_NAS_TECH_REG_STATUS_DENIED_LTD_SRV)
    {
        ret_val = TRUE;
    }

    if ( !ret_val )
    {
        data_reg_status = cri_nas_retrieve_reg_state(FALSE);
        if ( data_reg_status == CRI_NAS_TECH_REG_STATUS_NOT_REGISTERED_LTD_SRV ||
                data_reg_status == CRI_NAS_TECH_REG_STATUS_DENIED_LTD_SRV )
        {
            ret_val = TRUE;
        }
    }
    return ret_val;
}


uint8_t cri_nas_nw_selection_is_plmn_same_as_requested()
{
    uint8_t ret = FALSE;
    uint8_t is_nw_sel = FALSE;
    uint8_t is_registered = FALSE;
    is_nw_sel = cri_nas_is_nw_selection_in_progress();
    uint32_t req_mcc;
    uint32_t req_mnc;
    char mcc[CRI_NAS_MCC_MNC_MAX+1];
    char mnc[CRI_NAS_MCC_MNC_MAX+1];

    if( is_nw_sel )
    {
        req_mcc = cri_nas_nw_selection_info.mcc;
        req_mnc = cri_nas_nw_selection_info.mnc;
        cri_nas_get_current_mcc_mnc(mcc,mnc);
        if ( (req_mcc == (uint32_t) atoi(mcc)) &&
            (req_mnc == (uint32_t) atoi(mnc)) )
        {
            ret = TRUE;
        }
    }
    return ret;
}


void cri_nas_update_nw_selection_info()
{
    uint8_t is_nw_sel = FALSE;
    uint8_t is_registered = FALSE;
    uint8_t is_limited_srv = FALSE;
    uint8_t is_same_plmn = FALSE;
    cri_nas_nw_select_state_type nw_sel_state;


    // TODO: do we need to consider no_change_valid TLV?
    is_nw_sel = cri_nas_is_nw_selection_in_progress();
    if ( is_nw_sel ) // nw_selection in progress.
    {
        is_registered = cri_nas_is_considered_registered();
        if ( is_registered )
        {
            if ( cri_nas_nw_selection_info.is_automatic == FALSE )
            {
                is_same_plmn = cri_nas_nw_selection_is_plmn_same_as_requested();
                if ( is_same_plmn )
                {
                    cri_nas_nw_selection_set_state(CRI_NAS_NW_SELECT_SETTING_PREF_REGD);
                }
            }
            else
            {
                    cri_nas_nw_selection_set_state(CRI_NAS_NW_SELECT_SETTING_PREF_REGD);
            }
        }
        else
        {
            is_limited_srv = cri_nas_is_considered_limited_service();
            if ( is_limited_srv )
            {
                cri_nas_nw_selection_set_state(CRI_NAS_NW_SELECT_SETTING_PREF_LIMITED_REGD);
            }
        }
    }
}



cri_core_error_type cri_nas_get_nw_selection_preferrence(uint32_t *nw_sel_pref)
{
    cri_core_error_type ret_val = CRI_ERR_NONE_V01;

    if ( cri_nas_sys_sel_pref_info.net_sel_pref_valid)
    {
        *nw_sel_pref = cri_nas_sys_sel_pref_info.net_sel_pref;
    }
    else
    {
        ret_val = cri_nas_fetch_system_selection_preference();
        if ( ret_val == CRI_ERR_NONE_V01 )
        {
            if ( cri_nas_sys_sel_pref_info.net_sel_pref_valid)
            {
                *nw_sel_pref = cri_nas_sys_sel_pref_info.net_sel_pref;
            }
        }
    }
    return ret_val;
}


cri_core_error_type cri_nas_get_roaming_preferrence(uint32_t *roaming_pref)
{
    cri_core_error_type ret_val = CRI_ERR_NONE_V01;

    if ( cri_nas_sys_sel_pref_info.roam_pref_valid )
    {
        if ( cri_nas_sys_sel_pref_info.roam_pref == NAS_ROAMING_PREF_OFF_V01 )
        {
            *roaming_pref = CRI_NAS_ROAMING_PREF_OFF;
        }
        else if (cri_nas_sys_sel_pref_info.roam_pref == NAS_ROAMING_PREF_ANY_V01)
        {
            *roaming_pref = CRI_NAS_ROAMING_PREF_ANY;
        }
    }
    else
    {
        ret_val = cri_nas_fetch_system_selection_preference();
        if ( ret_val == CRI_ERR_NONE_V01 )
        {
            if ( cri_nas_sys_sel_pref_info.roam_pref_valid )
            {
                if ( cri_nas_sys_sel_pref_info.roam_pref == NAS_ROAMING_PREF_OFF_V01 )
                {
                    *roaming_pref = CRI_NAS_ROAMING_PREF_OFF;
                }
                else if (cri_nas_sys_sel_pref_info.roam_pref == NAS_ROAMING_PREF_ANY_V01)
                {
                    *roaming_pref = CRI_NAS_ROAMING_PREF_ANY;
                }
            }
        }
    }
    return ret_val;
}


cri_core_error_type cri_nas_get_mode_preferrence(uint32_t *mode_pref)
{
    cri_core_error_type ret_val = CRI_ERR_NONE_V01;
    *mode_pref = 0;
    if ( cri_nas_sys_sel_pref_info.mode_pref_valid )
    {
        *mode_pref = cri_nas_sys_sel_pref_info.mode_pref;
    }
    else
    {
        ret_val = cri_nas_fetch_system_selection_preference();
        if ( ret_val == CRI_ERR_NONE_V01 )
        {
            if ( cri_nas_sys_sel_pref_info.mode_pref_valid )
            {
                *mode_pref = cri_nas_sys_sel_pref_info.mode_pref;
            }
        }
    }
    return ret_val;
}


cri_core_error_type cri_nas_fetch_system_selection_preference()
{
    cri_core_error_type ret_val;

    ret_val = CRI_ERR_INTERNAL_V01;

    nas_get_system_selection_preference_resp_msg_v01 qmi_response;
    memset(&qmi_response,
            NIL,
            sizeof(qmi_response));

    ret_val = cri_core_qmi_send_msg_sync(nas_client_id,
                                        QMI_NAS_GET_SYSTEM_SELECTION_PREFERENCE_REQ_MSG_V01,
                                        NULL,
                                        NIL,  // empty request payload
                                        &qmi_response,
                                        sizeof(qmi_response),
                                        CRI_CORE_MAX_TIMEOUT);

    if(CRI_ERR_NONE_V01 != ret_val)
    {
        UTIL_LOG_MSG("QMI Error - %d for get system selection pref mode\n",ret_val);
    }
    else
    {
        CRI_STORE_OPT_TLV(cri_nas_sys_sel_pref_info.mode_pref,
                          qmi_response.mode_pref);

        CRI_STORE_OPT_TLV(cri_nas_sys_sel_pref_info.net_sel_pref,
                          qmi_response.net_sel_pref);

        CRI_STORE_OPT_TLV(cri_nas_sys_sel_pref_info.roam_pref,
                          qmi_response.roam_pref);

        CRI_STORE_OPT_TLV(cri_nas_sys_sel_pref_info.srv_reg_restriction,
                          qmi_response.srv_reg_restriction);
    }

    return ret_val;
}


uint32_t cri_nas_is_auto_mode_enabled(uint32_t pref_mode)
{
    return (pref_mode & (1<<CRI_NAS_RAT_PRL_BIT));
}

uint32_t cri_nas_set_auto_mode(uint32_t pref_mode)
{
    pref_mode =  pref_mode | (1<<CRI_NAS_RAT_PRL_BIT);
    return pref_mode;
}

uint32_t cri_nas_get_qmi_pref_mode(uint32_t pref_mode)
{
    return (pref_mode & ~(1<<CRI_NAS_RAT_PRL_BIT));
}


cri_core_error_type cri_nas_set_pref_mode_request_handler( cri_core_context_type cri_core_context,
                                                            uint32_t pref_mode_to_be_set,
                                                            uint32_t roaming_pref,
                                                            void *event_data,
                                                            hlos_resp_cb_type hlos_resp_cb,
                                                            uint32_t *is_changed)
{
    cri_core_error_type ret_val;
    uint32_t curr_pref_mode;
    uint32_t curr_roaming_pref;
    uint32_t qmi_pref_mode;
    uint32_t is_auto;
    nas_set_system_selection_preference_req_msg_v01 nas_set_pref_mode_req_msg;
    cri_nas_rules_pref_mode_rule_data_type *rule_data;
    cri_rule_handler_user_rule_info_type user_rule_info;

    ret_val = QMI_ERR_INTERNAL_V01;
    memset(&nas_set_pref_mode_req_msg,
            NIL,
            sizeof(nas_set_pref_mode_req_msg));

    is_auto = cri_nas_is_auto_mode_enabled(pref_mode_to_be_set);
    qmi_pref_mode = cri_nas_get_qmi_pref_mode(pref_mode_to_be_set);
    ret_val = cri_nas_get_mode_preferrence(&curr_pref_mode);
    ret_val = cri_nas_get_roaming_preferrence(&curr_roaming_pref);

    if ( ret_val == CRI_ERR_NONE_V01 &&
        ( curr_pref_mode != qmi_pref_mode ||
        curr_roaming_pref != roaming_pref ) )
    {
        nas_set_pref_mode_req_msg.mode_pref_valid = TRUE;
        nas_set_pref_mode_req_msg.mode_pref = qmi_pref_mode;

        if ( is_auto )
        {
            nas_set_pref_mode_req_msg.gw_acq_order_pref_valid = TRUE;
            nas_set_pref_mode_req_msg.gw_acq_order_pref = NAS_GW_ACQ_ORDER_PREF_AUTOMATIC_V01;
        }

        nas_set_pref_mode_req_msg.roam_pref_valid = TRUE;
        if ( roaming_pref == CRI_NAS_ROAMING_PREF_OFF )
        {
            nas_set_pref_mode_req_msg.roam_pref = NAS_ROAMING_PREF_OFF_V01;
        }
        else if ( roaming_pref == CRI_NAS_ROAMING_PREF_ANY )
        {
            nas_set_pref_mode_req_msg.roam_pref = NAS_ROAMING_PREF_ANY_V01;
        }

        rule_data = (cri_nas_rules_pref_mode_rule_data_type*)
                    util_memory_alloc(sizeof(*rule_data));
        if(rule_data)
        {
            rule_data->pref_mode = qmi_pref_mode;

            memset(&user_rule_info, 0, sizeof(user_rule_info));
            user_rule_info.rule_data = (void *)rule_data;
            user_rule_info.rule_check_handler = cri_nas_rules_pref_mode_rule_check_handler;
            user_rule_info.rule_data_free_handler = cri_nas_rules_pref_mode_rule_data_free_handler;
            ret_val = cri_core_qmi_send_msg_async(cri_core_context,
                                                nas_client_id,
                                                QMI_NAS_SET_SYSTEM_SELECTION_PREFERENCE_REQ_MSG_V01,
                                                &nas_set_pref_mode_req_msg,
                                                sizeof(nas_set_pref_mode_req_msg),
                                                sizeof(nas_set_system_selection_preference_resp_msg_v01),
                                                event_data,
                                                hlos_resp_cb,
                                                CRI_CORE_MINIMAL_TIMEOUT,
                                                &user_rule_info
                                                );

            if(CRI_ERR_NONE_V01 != ret_val)
            {
                UTIL_LOG_MSG("QMI Error - %d for set preferred networks\n",ret_val);
                *is_changed = FALSE;
            }
            else
            {
                *is_changed = TRUE;
            }
        }
    }
    else
    {
        *is_changed = FALSE;
    }

    return ret_val;
}


cri_core_error_type cri_nas_get_current_mcc_mnc(char *mcc, char *mnc)
{
    void *reg_info;
    cri_nas_rte_type rat;
    cri_nas_rte_gsm_reg_info_type *gsm_reg_info;
    cri_nas_rte_wcdma_reg_info_type *wcdma_reg_info;
    cri_nas_rte_cdma_reg_info_type *cdma_reg_info;
    cri_nas_rte_hdr_reg_info_type *hdr_reg_info;
    cri_nas_rte_lte_reg_info_type *lte_reg_info;
    cri_nas_rte_tdscdma_reg_info_type *tdscdma_reg_info;

    reg_info = cri_nas_retrieve_voice_rte(&rat);

    if ( rat == CRI_NAS_RTE_UNKNOWN )
    {
        return CRI_ERR_INTERNAL_V01;
    }

    switch(rat)
    {
        case CRI_NAS_RTE_CDMA:
            cdma_reg_info = (cri_nas_rte_cdma_reg_info_type*)reg_info;
            strlcpy(mcc, cdma_reg_info->reg_info.mcc,sizeof(mcc));
            strlcpy(mnc, cdma_reg_info->reg_info.mnc,sizeof(mnc));
            break;

        case CRI_NAS_RTE_HDR:
            hdr_reg_info = (cri_nas_rte_hdr_reg_info_type*)reg_info;
            strlcpy(mcc, hdr_reg_info->reg_info.mcc,sizeof(mcc));
            strlcpy(mnc, hdr_reg_info->reg_info.mnc,sizeof(mnc));
            break;

        case CRI_NAS_RTE_GSM:
            gsm_reg_info = (cri_nas_rte_gsm_reg_info_type*)reg_info;
            strlcpy(mcc, gsm_reg_info->reg_info.mcc,sizeof(mcc));
            strlcpy(mnc, gsm_reg_info->reg_info.mnc,sizeof(mnc));
            break;

        case CRI_NAS_RTE_WCDMA:
            wcdma_reg_info = (cri_nas_rte_wcdma_reg_info_type*)reg_info;
            strlcpy(mcc, wcdma_reg_info->reg_info.mcc,sizeof(mcc));
            strlcpy(mnc, wcdma_reg_info->reg_info.mnc,sizeof(mnc));
            break;

        case CRI_NAS_RTE_TDSCDMA:
            tdscdma_reg_info = (cri_nas_rte_tdscdma_reg_info_type*)reg_info;
            strlcpy(mcc, tdscdma_reg_info->reg_info.mcc,sizeof(mcc));
            strlcpy(mnc, tdscdma_reg_info->reg_info.mnc,sizeof(mnc));
            break;

        case CRI_NAS_RTE_LTE:
            lte_reg_info = (cri_nas_rte_lte_reg_info_type*)reg_info;
            strlcpy(mcc, lte_reg_info->reg_info.mcc,sizeof(mcc));
            strlcpy(mnc, lte_reg_info->reg_info.mnc,sizeof(mnc));
            break;

        default:
            break;

    }
    return CRI_ERR_NONE_V01;

}


cri_nas_rte_type cri_nas_get_current_rat()
{
    cri_nas_rte_type curr_rat;
    cri_nas_retrieve_voice_rte(&curr_rat);
    return curr_rat;
}


uint32_t cri_nas_convert_qmi_rat_to_cri_rat(uint32_t rat)
{

    uint32_t qmi_rat;

    switch(rat)
    {
        case NAS_RADIO_IF_NO_SVC_V01:
            qmi_rat = CRI_NAS_RTE_UNKNOWN;
            break;

        case NAS_RADIO_IF_GSM_V01:
            qmi_rat = CRI_NAS_RTE_GSM;
            break;

        case NAS_RADIO_IF_UMTS_V01:
            qmi_rat = CRI_NAS_RTE_WCDMA;

            break;

        case NAS_RADIO_IF_TDSCDMA_V01:
            qmi_rat = CRI_NAS_RTE_TDSCDMA;

            break;

        case NAS_RADIO_IF_LTE_V01:
            qmi_rat = CRI_NAS_RTE_LTE;
            break;

        case NAS_RADIO_IF_CDMA_1X_V01:
            qmi_rat = CRI_NAS_RTE_CDMA;
            break;

        case NAS_RADIO_IF_CDMA_1XEVDO_V01:
            qmi_rat = CRI_NAS_RTE_HDR;
            break;

        default:
            qmi_rat = CRI_NAS_RTE_UNKNOWN;
            break;
    }
    return qmi_rat;
}



uint32_t cri_nas_convert_rat_to_qmi_rat(uint32_t rat)
{

    uint32_t qmi_rat;

    switch(rat)
    {

        case CRI_NAS_RTE_UNKNOWN:
            qmi_rat = NAS_RADIO_IF_NO_SVC_V01;
            break;

        case CRI_NAS_RTE_GSM:
            qmi_rat = NAS_RADIO_IF_GSM_V01;
            break;

        case CRI_NAS_RTE_WCDMA:
            qmi_rat = NAS_RADIO_IF_UMTS_V01;

            break;

        case CRI_NAS_RTE_TDSCDMA:
            qmi_rat = NAS_RADIO_IF_TDSCDMA_V01;

            break;

        case CRI_NAS_RTE_LTE:
            qmi_rat = NAS_RADIO_IF_LTE_V01;
            break;

        case CRI_NAS_RTE_CDMA:
            qmi_rat = NAS_RADIO_IF_CDMA_1X_V01;
            break;

        case CRI_NAS_RTE_HDR:
            qmi_rat = NAS_RADIO_IF_CDMA_1XEVDO_V01;
            break;

        default:
            qmi_rat = NAS_RADIO_IF_NO_SVC_V01;
            break;
    }
    return qmi_rat;
}


cri_core_error_type cri_nas_get_operator_name_request_handler
(
    char *long_eons,
    char *short_eons,
    char *mcc_str,
    char *mnc_str
)
{
    void *reg_info;
    cri_core_error_type ret_val = CRI_ERR_NONE_V01;
    cri_nas_rte_type plmn_rat;

    plmn_rat = cri_nas_get_current_rat();

    if ( plmn_rat != CRI_NAS_RTE_UNKNOWN )
    {
        cri_nas_get_current_mcc_mnc(mcc_str, mnc_str);

        cri_nas_handle_centralized_short_long_eons(mcc_str,
                                                    mnc_str,
                                                    short_eons,
                                                    long_eons,
                                                    plmn_rat);
    }
    else
    {
        ret_val = CRI_ERR_NO_NETWORK_FOUND_V01;
    }

    return ret_val;

}




int cri_nas_check_centralized_eons_support_status()
{
    qmi_client_error_type qmi_transport_error;
    nas_get_centralized_eons_support_status_resp_msg_v01 qmi_response;
    memset(&qmi_response, NIL, sizeof(qmi_response));

    qmi_transport_error = cri_core_qmi_send_msg_sync( nas_client_id,
                                                    QMI_NAS_GET_CENTRALIZED_EONS_SUPPORT_STATUS_REQ_MSG_V01,
                                                    NULL,
                                                    NIL,
                                                    &qmi_response,
                                                    sizeof(qmi_response),
                                                    CRI_CORE_MAX_TIMEOUT );

    if( TRUE == qmi_response.centralized_eons_supported_valid && TRUE == qmi_response.centralized_eons_supported )
    {
        UTIL_LOG_MSG("centralized eons supported");
        return TRUE;
    }
    else
    {
        UTIL_LOG_MSG("centralized eons not supported");
        return FALSE;
    }

}



cri_core_error_type cri_nas_get_plmn_name_from_modem
(
    char *mcc_str,
    char *mnc_str,
    char *short_eons,
    char *long_eons,
    uint32_t plmn_rat
)
{
    int ret_val = CRI_ERR_INTERNAL_V01;
    uint32_t is_3gpp_rat;
    uint32_t mcc,mnc;
    uint32_t qmi_plmn_rat;

    int is_spn_present;
    int is_plmn_name_present;
    int prefer_spn;

    nas_get_plmn_name_req_msg_v01  get_plmn_req;
    nas_get_plmn_name_resp_msg_v01 get_plmn_resp;

    nas_get_3gpp2_subscription_info_req_msg_v01 cdma_subscription_info_req;
    nas_get_3gpp2_subscription_info_resp_msg_v01 cdma_subscription_info_resp;

    qmi_client_error_type qmi_client_error = QMI_NO_ERR;

    // TODO: check with Sai on how to deal with spn or plmn name.
    prefer_spn = FALSE;

    if( !mcc_str || !mnc_str || !short_eons || !long_eons )
    {
        UTIL_LOG_MSG( "Invalid args. NULL ptr passed.");
        return CRI_ERR_MISSING_ARG_V01;
    }

    memset(&get_plmn_req,NIL,sizeof(get_plmn_req));
    memset(&get_plmn_resp,NIL,sizeof(get_plmn_resp));

    memset(&cdma_subscription_info_req, NIL, sizeof(cdma_subscription_info_req));
    memset(&cdma_subscription_info_resp, NIL, sizeof(cdma_subscription_info_resp));

    memset(short_eons, NIL, CRI_NAS_OPERATOR_MAX_EONS_LEN);
    memset(long_eons, NIL, CRI_NAS_OPERATOR_MAX_EONS_LEN);

    mcc = atoi(mcc_str);
    mnc = atoi(mnc_str);
    is_3gpp_rat = is_3gpp(plmn_rat);
    qmi_plmn_rat = cri_nas_convert_rat_to_qmi_rat(plmn_rat);

    if ( is_3gpp_rat )
    { // 3gpp

        get_plmn_req.plmn.mcc = mcc;
        get_plmn_req.plmn.mnc = mnc;

        if( NAS_MCC_MNC_MAX_V01 != strlen(mnc_str) )
        {
            get_plmn_req.mnc_includes_pcs_digit_valid = TRUE;
            get_plmn_req.mnc_includes_pcs_digit = TRUE;
        }

        get_plmn_req.always_send_plmn_name_valid = TRUE;
        get_plmn_req.always_send_plmn_name = TRUE;

        get_plmn_req.suppress_sim_error_valid = TRUE;
        get_plmn_req.suppress_sim_error = TRUE;

        // Pass the RAT from network scan response if found valid.
        if(qmi_plmn_rat != NAS_RADIO_IF_NO_SVC_V01 )
        {
            get_plmn_req.rat_valid = TRUE;
            get_plmn_req.rat = qmi_plmn_rat;
        }
        else
        {
            UTIL_LOG_MSG("Unknown RAT\n");
            get_plmn_req.rat_valid = FALSE;
        }

    qmi_client_error = cri_core_qmi_send_msg_sync( nas_client_id,
                                                QMI_NAS_GET_PLMN_NAME_REQ_MSG_V01,
                                                (void*) &get_plmn_req,
                                                sizeof( get_plmn_req ),
                                                (void*) &get_plmn_resp,
                                                sizeof( get_plmn_resp ),
                                                CRI_CORE_MAX_TIMEOUT);


    UTIL_LOG_MSG("qmi_client_error: %d,\n",(int)qmi_client_error );

    if( QMI_ERR_NONE_V01 == qmi_client_error )
    {
        if( TRUE == get_plmn_resp.eons_plmn_name_3gpp_valid )
        {
            is_spn_present = (get_plmn_resp.eons_plmn_name_3gpp.spn_len > NIL);
            is_plmn_name_present = ((get_plmn_resp.eons_plmn_name_3gpp.plmn_short_name_len > NIL) || (get_plmn_resp.eons_plmn_name_3gpp.plmn_long_name_len > NIL));

            if ( is_plmn_name_present == FALSE && is_spn_present == TRUE )
            {
                prefer_spn = TRUE;
            }

            if( TRUE == prefer_spn )
            {
                if( NAS_CODING_SCHEME_CELL_BROADCAST_GSM_V01 == get_plmn_resp.eons_plmn_name_3gpp.spn_enc )
                {
                cri_nas_convert_gsm8bit_alpha_string_to_utf8( (const char*)get_plmn_resp.eons_plmn_name_3gpp.spn,
                                                                get_plmn_resp.eons_plmn_name_3gpp.spn_len,
                                                                short_eons );
                cri_nas_convert_gsm8bit_alpha_string_to_utf8( (const char*)get_plmn_resp.eons_plmn_name_3gpp.spn,
                                                                get_plmn_resp.eons_plmn_name_3gpp.spn_len,
                                                                long_eons );
                UTIL_LOG_MSG("spn is 7-bit Unpacked data");
                }
            }
            else
            {
                if( NAS_CODING_SCHEME_CELL_BROADCAST_GSM_V01 == get_plmn_resp.eons_plmn_name_3gpp.plmn_short_name_enc )
                {
                    cri_nas_convert_gsm8bit_alpha_string_to_utf8( (const char*)get_plmn_resp.eons_plmn_name_3gpp.plmn_short_name,
                                                                get_plmn_resp.eons_plmn_name_3gpp.plmn_short_name_len,
                                                                short_eons );
                    UTIL_LOG_MSG("plmn short name is 7-bit Unpacked data\n");
                }
                else
                {
                    cri_nas_decode_operator_name_in_little_endian(short_eons,
                                                                CRI_NAS_OPERATOR_MAX_EONS_LEN,
                                                                get_plmn_resp.eons_plmn_name_3gpp.plmn_short_name_enc,
                                                                (unsigned char*)get_plmn_resp.eons_plmn_name_3gpp.plmn_short_name,
                                                                get_plmn_resp.eons_plmn_name_3gpp.plmn_short_name_len);
                    UTIL_LOG_MSG("short eons derived from plmn %s\n",short_eons);
                }

                    if( NAS_CODING_SCHEME_CELL_BROADCAST_GSM_V01 == get_plmn_resp.eons_plmn_name_3gpp.plmn_long_name_enc )
                    {
                        cri_nas_convert_gsm8bit_alpha_string_to_utf8( (const char*)get_plmn_resp.eons_plmn_name_3gpp.plmn_long_name,
                                                                    get_plmn_resp.eons_plmn_name_3gpp.plmn_long_name_len,
                                                                    long_eons );
                        UTIL_LOG_MSG("plmn long name is 7-bit Unpacked data\n");
                    }
                    else
                    {
                        cri_nas_decode_operator_name_in_little_endian(long_eons,
                                                                    CRI_NAS_OPERATOR_MAX_EONS_LEN,
                                                                    get_plmn_resp.eons_plmn_name_3gpp.plmn_long_name_enc,
                                                                    (unsigned char*)get_plmn_resp.eons_plmn_name_3gpp.plmn_long_name,
                                                                    get_plmn_resp.eons_plmn_name_3gpp.plmn_long_name_len);
                        UTIL_LOG_MSG("long eons derived from plmn %s\n",long_eons);
                    }
                }
            }
        }
    }
    else
    {
    // 3gpp2

        memset(&cdma_subscription_info_req,0,sizeof(cdma_subscription_info_req));

        cdma_subscription_info_req.nam_id = 0xFF; // current NAM
        cdma_subscription_info_req.get_3gpp2_info_mask_valid = TRUE;
        cdma_subscription_info_req.get_3gpp2_info_mask = QMI_NAS_GET_3GPP2_SUBS_INFO_NAM_NAME_V01;

        qmi_client_error = cri_core_qmi_send_msg_sync( nas_client_id,
                                                   QMI_NAS_GET_3GPP2_SUBSCRIPTION_INFO_REQ_MSG_V01,
                                                   (void*) &cdma_subscription_info_req,
                                                   sizeof( cdma_subscription_info_req ),
                                                   (void*) &cdma_subscription_info_resp,
                                                   sizeof( cdma_subscription_info_resp ),
                                                   CRI_CORE_MINIMAL_TIMEOUT );
        if ( qmi_client_error == QMI_ERR_NONE_V01 )
        {
            if ( cdma_subscription_info_resp.nam_name_valid == TRUE )
            {
                memcpy(long_eons,
                      cdma_subscription_info_resp.nam_name,
                      cdma_subscription_info_resp.nam_name_len);
                memcpy(short_eons,
                    cdma_subscription_info_resp.nam_name,
                    cdma_subscription_info_resp.nam_name_len);
            }
        }
    }

    return qmi_client_error;

}




void cri_nas_handle_centralized_short_long_eons(char *mcc_str, char *mnc_str, char *short_eons, char *long_eons,uint32_t plmn_rat)
{
  char * internal_long_name;
  char * internal_short_name;

  if( mcc_str && mnc_str && short_eons && long_eons )
  {
      cri_nas_get_plmn_name_from_modem (mcc_str, mnc_str, short_eons, long_eons, plmn_rat);

      if( cri_nas_is_operator_name_empty_or_white_space( long_eons, CRI_NAS_OPERATOR_MAX_EONS_LEN ) &&
          cri_nas_is_operator_name_empty_or_white_space( short_eons, CRI_NAS_OPERATOR_MAX_EONS_LEN))
      {
        UTIL_LOG_MSG("Received empty operator name\n");
      }
  }
  else
  {
    UTIL_LOG_MSG("Null pointer passed");
  }

}



void* cri_nas_core_nw_scan_helper(void *nw_scan_resp_msg)
{
    nas_perform_network_scan_resp_msg_v01 *qmi_response;
    qmi_response = (nas_perform_network_scan_resp_msg_v01*)nw_scan_resp_msg;

    int iter_i;
    int    iter_j;
    int  nw_cnt = 0;
    int  network_status;
    uint8_t nw_status;
    uint8_t plmn_rat;
    int rat;
    qmi_client_error_type qmi_client_error;
    nas_3gpp_network_info_type_v01 * qmi_nw_info;
    char qmi_nw_info_mcc_str[NAS_MCC_MNC_MAX_V01+1];
    char qmi_nw_info_mnc_str[NAS_MCC_MNC_MAX_V01+1];
    char long_eons[CRI_NAS_OPERATOR_MAX_EONS_LEN+1];
    char short_eons[CRI_NAS_OPERATOR_MAX_EONS_LEN+1];

    cri_nas_nw_scan_resp_type *nw_scan_result;

    nw_scan_result = util_memory_alloc(sizeof(*nw_scan_result));

    if ( qmi_response )
    {
        if ( qmi_response->nas_3gpp_network_info_valid && nw_scan_result )
        {
            qmi_nw_info = qmi_response->nas_3gpp_network_info;
            UTIL_LOG_MSG("Network Info (long/short/mcc_mnc/status)");
            UTIL_LOG_MSG("Number of Scanned Network(s) : %d",qmi_response->nas_3gpp_network_info_len);
            nw_scan_result->nw_scan_info_len = 0;
            for ( iter_i = 0; iter_i < (int)qmi_response->nas_3gpp_network_info_len; iter_i++ )
            {
                memset(qmi_nw_info_mcc_str,NIL,sizeof(qmi_nw_info_mcc_str));
                memset(qmi_nw_info_mnc_str,NIL,sizeof(qmi_nw_info_mnc_str));

                snprintf( qmi_nw_info_mcc_str, NAS_MCC_MNC_MAX_V01+1, "%03d", (int)qmi_nw_info->mobile_country_code );
                if ( (qmi_nw_info->mobile_network_code > 99) ||  (qmi_response->mnc_includes_pcs_digit_valid && qmi_response->mnc_includes_pcs_digit[iter_i].mnc_includes_pcs_digit) )
                {
                    snprintf( qmi_nw_info_mnc_str, NAS_MCC_MNC_MAX_V01+1, "%03d", (int)qmi_nw_info->mobile_network_code );
                }
                else
                {
                    snprintf( qmi_nw_info_mnc_str, NAS_MCC_MNC_MAX_V01+1, "%02d", (int)qmi_nw_info->mobile_network_code );
                }

                if( TRUE == cri_nas_check_centralized_eons_support_status() )
                {
                    if ( qmi_response->nas_network_radio_access_technology_valid )
                    {
                      plmn_rat = qmi_response->nas_network_radio_access_technology[iter_i].rat;
                      plmn_rat = cri_nas_convert_qmi_rat_to_cri_rat(plmn_rat);
                      UTIL_LOG_MSG ("RAT received in perform_nw_scan response %d", (int) plmn_rat);
                    }
                    cri_nas_handle_centralized_short_long_eons(qmi_nw_info_mcc_str, qmi_nw_info_mnc_str, short_eons, long_eons, plmn_rat);

                }
                else
                {
                    // TODO: use nitz information
                }

                if ( *(long_eons) && !*(short_eons) )
                {
                    UTIL_LOG_MSG("Filling short eons with long eons\n");
                    strlcpy( short_eons, long_eons, sizeof(short_eons));
                }

                if ( *(short_eons) && !*(long_eons) )
                {
                    UTIL_LOG_MSG("Filling long eons with short eons\n");
                    strlcpy( long_eons, short_eons, sizeof(long_eons));
                }

                nw_status = qmi_nw_info->network_status;

                if ( (nw_status & CRI_NAS_NW_SCAN_RES_ENTRY_CUR_SERVING) )
                {
                    network_status = CRI_NAS_NW_SCAN_RES_ENTRY_CUR_SERVING;
                }
                else if ( nw_status & CRI_NAS_NW_SCAN_RES_ENTRY_FORBIDDEN )
                {
                    network_status = CRI_NAS_NW_SCAN_RES_ENTRY_FORBIDDEN;
                }
                else if ( nw_status & CRI_NAS_NW_SCAN_RES_ENTRY_AVAILABLE )
                {
                    network_status = CRI_NAS_NW_SCAN_RES_ENTRY_AVAILABLE;
                }

                if( qmi_response->nas_network_radio_access_technology_valid )
                {
//                    rat = qmi_response->nas_network_radio_access_technology[nw_cnt].rat;
                    switch(qmi_response->nas_network_radio_access_technology[nw_cnt].rat)
                    {
                        case NAS_RADIO_IF_GSM:
                            rat = CRI_NAS_RTE_GSM;
                            break;

                        case NAS_RADIO_IF_UMTS:
                            rat = CRI_NAS_RTE_WCDMA;
                            break;

                        case NAS_RADIO_IF_LTE:
                            rat = CRI_NAS_RTE_LTE;
                            break;

                        case NAS_RADIO_IF_TDSCDMA:
                            rat = CRI_NAS_RTE_TDSCDMA;
                            break;

                        default:
                            rat = CRI_NAS_RTE_UNKNOWN;
                            break;

                    }

                }

                UTIL_LOG_MSG("\nNetwork #%d - %s/%s/%d/%d\n",
                                iter_i+1,
                                long_eons,
                                short_eons,
                                network_status,
                                rat);
                strlcpy(nw_scan_result->long_eons[iter_i],
                        long_eons,
                        sizeof(nw_scan_result->long_eons[iter_i]));
                strlcpy(nw_scan_result->short_eons[iter_i],
                        short_eons,
                        sizeof(nw_scan_result->short_eons[iter_i]));

                strlcpy(nw_scan_result->mcc[iter_i],
                        qmi_nw_info_mcc_str,
                        sizeof(nw_scan_result->mcc[iter_i]));
                strlcpy(nw_scan_result->mnc[iter_i],
                        qmi_nw_info_mnc_str,
                        sizeof(nw_scan_result->mnc[iter_i]));
                nw_scan_result->rat[iter_i] = rat;
                nw_scan_result->network_status[iter_i] = network_status;
                nw_scan_result->nw_scan_info_len++;
                qmi_nw_info++;
            }
        }

        if ( qmi_response )
        {
//            util_memory_free( &qmi_response );
        }
    }
    return nw_scan_result;
}



void cri_nas_core_nw_scan_resp_handler(int qmi_service_client_id,
                                        void *nw_scan_resp_msg,
                                        cri_core_context_type cri_core_context)
{
    cri_core_error_type cri_core_error;
    nas_perform_network_scan_resp_msg_v01 *resp_msg;
    cri_nas_nw_scan_resp_type *nw_scan_result;

    cri_core_error = QMI_ERR_NONE_V01;
    resp_msg = (nas_perform_network_scan_resp_msg_v01*)nw_scan_resp_msg;

    nw_scan_result = cri_nas_core_nw_scan_helper(nw_scan_resp_msg);

    if(nw_scan_resp_msg)
    {
        cri_core_error = cri_core_retrieve_err_code(QMI_NO_ERR, &resp_msg->resp);
        cri_rule_handler_rule_check(cri_core_context, cri_core_error, nw_scan_result);
    }
}



cri_core_error_type cri_nas_core_network_scan_request_handler(cri_core_context_type cri_core_context,
                                                            void *hlos_cb_data,
                                                            hlos_resp_cb_type hlos_resp_cb)

{
    qmi_error_type_v01 ret_val;
    ret_val = QMI_ERR_INTERNAL_V01;

    ret_val =  cri_core_qmi_send_msg_async(cri_core_context,
                                        nas_client_id,
                                        QMI_NAS_PERFORM_NETWORK_SCAN_REQ_MSG_V01,
                                        NULL,
                                        NIL,
                                        sizeof(nas_perform_network_scan_resp_msg_v01),
                                        hlos_cb_data,
                                        hlos_resp_cb,
                                        CRI_CORE_MAX_TIMEOUT,
                                        NULL);

    if(QMI_ERR_NONE_V01 != ret_val)
    {
        UTIL_LOG("cri_nas_core_network_scan_request_handler error - %d\n", ret_val);
    }
    return ret_val;

}




int cri_nas_change_screen_state(uint8_t is_screen_off)
{
    int ret_val;
    is_screen_state_off = is_screen_off;
    ret_val = cri_nas_enable_networking_indications(is_screen_off, cri_nas_is_nw_selection_in_progress());
    return ret_val;
}

cri_core_error_type cri_nas_core_limit_sys_info_indication(uint8_t flag)
{
    cri_core_error_type ret_val = CRI_ERR_NONE_V01;
    qmi_error_type_v01 error_code = QMI_ERR_NONE_V01;
    nas_limit_sys_info_ind_reporting_req_msg_v01 req_msg;
    nas_limit_sys_info_ind_reporting_resp_msg_v01 resp_msg;

    memset(&req_msg, NIL, sizeof(req_msg));
    memset(&req_msg, NIL, sizeof(req_msg));

    req_msg.limit_sys_info_chg_rpt = flag ? (NAS_LIMIT_BY_SRV_STATUS_V01 || NAS_LIMIT_BY_SRV_DOMAIN_V01) : NIL;

    error_code = cri_core_qmi_send_msg_sync(cri_nas_core_retrieve_client_id(),
                                          QMI_NAS_LIMIT_SYS_INFO_IND_REPORTING_REQ_MSG_V01,
                                          &req_msg,
                                          sizeof(req_msg),
                                          &resp_msg,
                                          sizeof(resp_msg),
                                          CRI_CORE_MINIMAL_TIMEOUT);

    ret_val == cri_core_retrieve_err_code(error_code,
                                          &resp_msg.resp);
    return ret_val;
}


int cri_nas_enable_networking_indications(uint8_t is_screen_off,
                                            uint8_t is_nw_sel_in_progress)
{
    uint32_t iter_i;
    uint8_t always_enable_action = FALSE; // enable when online
    uint8_t enable_indications = TRUE;
    cri_core_error_type cri_error;
    dms_operating_mode_enum_v01 opr_mode;

    nas_config_sig_info2_req_msg_v01 sig_info2_req_msg;
    nas_config_sig_info2_resp_msg_v01 sig_info2_resp_msg;
    nas_indication_register_req_msg_v01 nwreg_indications_qmi_request;
    nas_indication_register_resp_msg_v01 nwreg_indications_qmi_response;
    nas_limit_sys_info_ind_reporting_req_msg_v01 sys_info_report_req_msg;

    memset(&sig_info2_req_msg,NIL,sizeof(sig_info2_req_msg));
    memset(&sig_info2_resp_msg,NIL,sizeof(sig_info2_resp_msg));
    memset(&nwreg_indications_qmi_request,NIL,sizeof(nwreg_indications_qmi_request));

    // if screen is off, disable indications.
    if ( is_screen_off )
    {
        enable_indications = FALSE;
    }

    if ( cri_dms_utils_is_valid_operating_mode())
    {
        cri_dms_utils_get_current_operating_mode(&opr_mode);
        if ( opr_mode == DMS_OP_MODE_ONLINE_V01 )
        {
            always_enable_action = TRUE;
        }
    }

    sig_info2_req_msg.cdma_ecio_delta_valid = TRUE;
    sig_info2_req_msg.cdma_ecio_delta = CRI_NAS_ECIO_DELTA;

    sig_info2_req_msg.cdma_rssi_delta_valid = TRUE;
    sig_info2_req_msg.cdma_rssi_delta = CRI_NAS_RSSI_DELTA;

    sig_info2_req_msg.gsm_rssi_delta_valid = TRUE;
    sig_info2_req_msg.gsm_rssi_delta = CRI_NAS_RSSI_DELTA;

    sig_info2_req_msg.hdr_ecio_delta_valid = TRUE;
    sig_info2_req_msg.hdr_ecio_delta = CRI_NAS_ECIO_DELTA;

    sig_info2_req_msg.hdr_io_delta_valid = TRUE;
    sig_info2_req_msg.hdr_io_delta = CRI_NAS_IO_DELTA;

    sig_info2_req_msg.hdr_rssi_delta_valid = TRUE;
    sig_info2_req_msg.hdr_rssi_delta = CRI_NAS_RSSI_DELTA;

    sig_info2_req_msg.hdr_sinr_delta_valid = TRUE;
    sig_info2_req_msg.hdr_sinr_delta = CRI_NAS_SINR_DELTA;

    sig_info2_req_msg.lte_rsrp_delta_valid = TRUE;
    sig_info2_req_msg.lte_rsrp_delta = CRI_NAS_RSRP_DELTA;

    sig_info2_req_msg.lte_rsrq_delta_valid = TRUE;
    sig_info2_req_msg.lte_rsrq_delta = CRI_NAS_RSRQ_DELTA;

    sig_info2_req_msg.lte_rssi_delta_valid = TRUE;
    sig_info2_req_msg.lte_rssi_delta = CRI_NAS_RSSI_DELTA;

    sig_info2_req_msg.lte_snr_delta_valid = TRUE;
    sig_info2_req_msg.lte_snr_delta = CRI_NAS_SNR_DELTA;

    sig_info2_req_msg.wcdma_ecio_delta_valid = TRUE;
    sig_info2_req_msg.wcdma_ecio_delta = CRI_NAS_ECIO_DELTA;

    sig_info2_req_msg.wcdma_rssi_delta_valid = TRUE;
    sig_info2_req_msg.wcdma_rssi_delta_valid = CRI_NAS_RSSI_DELTA;

    sig_info2_req_msg.tdscdma_ecio_delta_valid = TRUE;
    sig_info2_req_msg.tdscdma_ecio_delta = CRI_NAS_ECIO_DELTA;

    sig_info2_req_msg.tdscdma_rscp_delta_valid = TRUE;
    sig_info2_req_msg.tdscdma_rscp_delta = CRI_NAS_RSCP_DELTA;

    sig_info2_req_msg.tdscdma_rssi_delta_valid = TRUE;
    sig_info2_req_msg.tdscdma_rssi_delta = CRI_NAS_RSSI_DELTA;

    sig_info2_req_msg.tdscdma_sinr_delta_valid = TRUE;
    sig_info2_req_msg.tdscdma_sinr_delta = CRI_NAS_SINR_DELTA;


    cri_error = cri_core_qmi_send_msg_sync( nas_client_id,
                                           QMI_NAS_CONFIG_SIG_INFO2_REQ_MSG_V01,
                                           (void*) &sig_info2_req_msg,
                                           sizeof( sig_info2_req_msg),
                                           (void*) &sig_info2_resp_msg,
                                           sizeof( sig_info2_resp_msg),
                                           CRI_CORE_MINIMAL_TIMEOUT );

    // TODO: get clarification from sai on whether to register for registartion reject cause or not.

    if ( cri_error == CRI_ERR_NONE_V01 )
    {
        // TODO: get clarification on why we need always indications.
        nwreg_indications_qmi_request.reg_sys_sel_pref_valid = TRUE;
        nwreg_indications_qmi_request.reg_sys_sel_pref       = TRUE; // always get sys sel pref

        nwreg_indications_qmi_request.dual_standby_pref_valid = TRUE;
        nwreg_indications_qmi_request.dual_standby_pref       = TRUE;

        nwreg_indications_qmi_request.subscription_info_valid = TRUE;
        nwreg_indications_qmi_request.subscription_info       = TRUE;

        nwreg_indications_qmi_request.reg_network_time_valid = TRUE;
        nwreg_indications_qmi_request.reg_network_time       = always_enable_action; // only when online?

        nwreg_indications_qmi_request.reg_rtre_cfg_valid = TRUE;
        nwreg_indications_qmi_request.reg_rtre_cfg       = TRUE;

        nwreg_indications_qmi_request.reg_embms_status_valid = TRUE;
        nwreg_indications_qmi_request.reg_embms_status       = always_enable_action;

        nwreg_indications_qmi_request.sig_info_valid = TRUE;
        nwreg_indications_qmi_request.sig_info       = enable_indications;

        nwreg_indications_qmi_request.err_rate_valid = TRUE;
        nwreg_indications_qmi_request.err_rate       = enable_indications;

        nwreg_indications_qmi_request.req_serving_system_valid = TRUE;
        nwreg_indications_qmi_request.req_serving_system       = FALSE; // always suppress serving system as we have sys info.

        nwreg_indications_qmi_request.reg_current_plmn_name_valid = TRUE;
        nwreg_indications_qmi_request.reg_current_plmn_name       = always_enable_action; // while online?

        nwreg_indications_qmi_request.sys_info_valid = TRUE;
        nwreg_indications_qmi_request.sys_info       = TRUE;

        // enable RF band indications only for LTE.
        nwreg_indications_qmi_request.reg_rf_band_info_valid = TRUE;
        if ( cri_nas_sys_sel_pref_info.mode_pref_valid &&
            (cri_nas_sys_sel_pref_info.mode_pref && (1 << QMI_NAS_RAT_MODE_PREF_LTE_BIT_V01)) )
        {
            nwreg_indications_qmi_request.reg_rf_band_info = TRUE;
        }
        else
        {
            nwreg_indications_qmi_request.reg_rf_band_info = FALSE;
        }

        cri_error = cri_core_qmi_send_msg_sync( nas_client_id,
                                               QMI_NAS_INDICATION_REGISTER_REQ_MSG_V01,
                                               (void*) &nwreg_indications_qmi_request,
                                               sizeof( nwreg_indications_qmi_request ),
                                               (void*) &nwreg_indications_qmi_response,
                                               sizeof( nwreg_indications_qmi_response ),
                                               CRI_CORE_MINIMAL_TIMEOUT);

        if ( enable_indications && CRI_ERR_NONE_V01 == cri_error )
        {
            // need to fetch all information which is disabled as part of screen off.
            cri_nas_core_fetch_update_sys_info();
            cri_nas_core_fetch_update_sig_info();
        }

        // if nw_selection in progress, or enable indication true don't limit sys info indication
        if ( is_nw_sel_in_progress || enable_indications )
        {
            cri_nas_core_limit_sys_info_indication(FALSE);
        }
        else
        {
            cri_nas_core_limit_sys_info_indication(TRUE);
        }
    }

    return cri_error;
}


cri_core_error_type cri_nas_network_selection_handler(cri_core_context_type cri_core_context, const cri_nas_nw_selection_request_type *selection_info, void *user_data, hlos_resp_cb_type hlos_resp_cb)
{
    uint32_t mcc;
    uint32_t mnc;
    uint32_t rat;
    uint8_t is_autoselection;
    uint8_t nw_sel_state = FALSE;
    const struct timeval nw_sel_timeout = { 60 , 0 }; // 60 second
    cri_core_error_type ret_val = QMI_ERR_INTERNAL_V01;
    nas_set_system_selection_preference_req_msg_v01 req_msg;
    nas_set_system_selection_preference_resp_msg_v01 resp_msg;
    cri_nas_rules_generic_rule_data_type *cri_nas_rules_generic_rule_data;
    cri_rule_handler_user_rule_info_type user_rule_info;

    memset(&req_msg, NIL, sizeof(req_msg));
    memset(&resp_msg, NIL, sizeof(resp_msg));

    is_autoselection = selection_info->is_automatic;

    if ( !is_autoselection )
    {
        mcc = selection_info->mcc;
        mnc = selection_info->mnc;
        rat = selection_info->rat;
    }

    cri_nas_rules_generic_rule_data = (cri_nas_rules_generic_rule_data_type*) util_memory_alloc(sizeof(*cri_nas_rules_generic_rule_data));
    if(cri_nas_rules_generic_rule_data)
    {
        cri_nas_rules_generic_rule_data->dummy_var = 0x00FF;

        if ( !is_autoselection && ( mcc == NIL || mnc == NIL ) )
        {
            return CRI_ERR_INVALID_ARG_V01;
        }

        nw_sel_state = cri_nas_is_nw_selection_in_progress();
        // make sure no other nw_selection request pending
        if ( nw_sel_state )
        {
            // TODO: how to differentiate the response from different requests
            // already a request pending, cancel currently on-going request
            cri_nas_nw_selection_set_state(CRI_NAS_NW_SELECT_CANCELLING_REQ);
        }

        req_msg.net_sel_pref_valid = TRUE;

        if ( is_autoselection )
        {
            req_msg.net_sel_pref.net_sel_pref = NAS_NET_SEL_PREF_AUTOMATIC_V01;
            cri_nas_nw_selection_info.is_automatic = TRUE;
        }
        else
        {
            req_msg.net_sel_pref.net_sel_pref = NAS_NET_SEL_PREF_MANUAL_V01;
            req_msg.net_sel_pref.mcc = mcc;
            req_msg.net_sel_pref.mnc = mnc;
            req_msg.mnc_includes_pcs_digit_valid = TRUE;
            req_msg.mnc_includes_pcs_digit = (mnc > 99 ) ? TRUE : FALSE;
            if ( rat != CRI_NAS_TECH_UNKNOWN )
            {
                req_msg.rat_valid = TRUE;
                req_msg.rat = cri_nas_convert_rat_to_qmi_rat(rat);
            }

            // update cache
            cri_nas_nw_selection_info.is_automatic = FALSE;
            cri_nas_nw_selection_info.mcc = mcc;
            cri_nas_nw_selection_info.mnc = mnc;
            cri_nas_nw_selection_info.rat = cri_nas_convert_rat_to_qmi_rat(rat);
        }

        cri_nas_nw_selection_set_state(CRI_NAS_NW_SELECT_SETTING_PREF);

        // enable indications
        cri_nas_enable_networking_indications(is_screen_state_off,TRUE);

        user_rule_info.rule_data = (void *)cri_nas_rules_generic_rule_data;
        user_rule_info.rule_check_handler = cri_nas_nw_selection_rule_check_handler;
        user_rule_info.rule_data_free_handler = cri_nas_rules_generic_rule_data_free_handler;
        ret_val = cri_core_qmi_send_msg_async(cri_core_context,
                                            nas_client_id,
                                            QMI_NAS_SET_SYSTEM_SELECTION_PREFERENCE_REQ_MSG_V01,
                                            &req_msg,
                                            sizeof(req_msg),
                                            sizeof(resp_msg),
                                            user_data,
                                            hlos_resp_cb,
                                            CRI_CORE_MAX_TIMEOUT,
                                            &user_rule_info
                                            );
    }

    if(CRI_ERR_NONE_V01 != ret_val)
    {
        if(cri_nas_rules_generic_rule_data)
        {
            cri_nas_rules_generic_rule_data_free_handler(cri_nas_rules_generic_rule_data);
        }
        cri_nas_nw_selection_set_state(CRI_NAS_NW_SELECT_ROLLBACK);
    }

    return ret_val;

}

void cri_nas_core_update_data_system_status()
{
    cri_nas_calculate_data_rte();
    cri_nas_send_update_to_clients(nas_client_id,
                                    QMI_NAS_SYS_INFO_IND_MSG_V01,
                                    NULL,
                                    NIL);
}
