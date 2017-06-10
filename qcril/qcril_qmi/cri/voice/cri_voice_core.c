/******************************************************************************
  ---------------------------------------------------------------------------

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

#include "cri_voice_call_info.h"
#include "cri_voice_settings.h"
#include "cri_voice_utils.h"
#include "cri_rule_handler.h"
//#include "qcril_qmi_nas.h"
//#include "qcril_qmi_voice.h"
//#include "qcrili.h"
//#include "qcril_pbm.h"
#include "cri_voice_ind_hdlr.h"
#include "cri_voice_dial.h"

boolean settings_inited;
cri_voice_cache_type call_info;
cri_voice_settings_type settings;
cri_voice_qmi_client_info_type qmi_client_info;

cri_core_error_type cri_voice_core_init(hlos_ind_cb_type hlos_ind_cb_func_ptr)
{
    QCRIL_LOG_FUNC_ENTRY();
    cri_voice_qmi_client_init(&qmi_client_info, hlos_ind_cb_func_ptr);

    cri_voice_cache_init(&call_info);

    if (!settings_inited)
    {
        cri_voice_settings_init(&settings);
        settings_inited = TRUE;
    }
    QCRIL_LOG_FUNC_RETURN();
    return 0;
}

cri_core_error_type cri_voice_reset()
{
    cri_voice_qmi_client_deinit(&qmi_client_info);
    // reset the cache
    cri_voice_cache_reset(&call_info);
    return 0;
}

cri_voice_cache_type* cri_voice_core_get_call_info()
{
    return &call_info;
}

util_list_info_type* cri_voice_core_get_call_list()
{
    return call_info.call_list_ptr;
}

cri_voice_settings_type* cri_voice_core_get_settings()
{
    return &settings;
}

cri_voice_qmi_client_info_type* cri_voice_core_get_qmi_client_info()
{
    return &qmi_client_info;
}

void cri_voice_core_unsol_ind_handler(int qmi_service_client_id,
                               unsigned long message_id,
                               void *ind_data,
                               int ind_data_len)
{
    switch(message_id)
    {
        case QMI_VOICE_ALL_CALL_STATUS_IND_V02:
            cri_voice_ind_hdlr_all_call_status_ind(
                qmi_service_client_id,
                (voice_all_call_status_ind_msg_v02*) ind_data );
            break;

        case QMI_VOICE_INFO_REC_IND_V02:
            cri_voice_ind_hdlr_info_rec_ind(
                qmi_service_client_id,
                (voice_info_rec_ind_msg_v02*) ind_data );
            break;

        case QMI_VOICE_CONFERENCE_INFO_IND_V02:
            cri_voice_ind_hdlr_conference_info_ind(
                qmi_service_client_id,
                (voice_conference_info_ind_msg_v02*) ind_data );
            break;

        case QMI_VOICE_OTASP_STATUS_IND_V02:
            cri_voice_ind_hdlr_otasp_status_ind(
                qmi_service_client_id,
                (voice_otasp_status_ind_msg_v02*) ind_data );
            break;

        case QMI_VOICE_PRIVACY_IND_V02:
            cri_voice_ind_hdlr_privacy_ind(
                qmi_service_client_id,
                (voice_privacy_ind_msg_v02*) ind_data );
            break;

        case QMI_VOICE_EXT_BRST_INTL_IND_V02:
            cri_voice_ind_hdlr_ext_brst_intl_ind(
                qmi_service_client_id,
                (voice_ext_brst_intl_ind_msg_v02*) ind_data );
            break;

        case QMI_VOICE_SUPS_NOTIFICATION_IND_V02:
            cri_voice_ind_hdlr_sups_notification_ind(
                qmi_service_client_id,
                (voice_sups_notification_ind_msg_v02*) ind_data );
            break;

        default:
            //no action
            break;
    }
}

void cri_voice_core_async_resp_handler(int qmi_service_client_id,
                                unsigned long message_id,
                                void *resp_data,
                                int resp_data_len,
                                cri_core_context_type cri_core_context)
{
    QCRIL_LOG_FUNC_ENTRY();

    if(resp_data && resp_data_len)
    {
        if (QMI_VOICE_DIAL_CALL_RESP_V02 == message_id)
        {
            cri_voice_dial_resp_handler(qmi_service_client_id, resp_data, cri_core_context);
        }
        else
        {
            cri_core_error_type cri_core_error = QMI_ERR_NONE_V01;
            switch(message_id)
            {
                case QMI_VOICE_END_CALL_RESP_V02:
                    cri_core_error = cri_core_retrieve_err_code(QMI_NO_ERR, &((voice_end_call_resp_msg_v02*)resp_data)->resp);
                    break;
                case QMI_VOICE_ANSWER_CALL_RESP_V02:
                    cri_core_error = cri_core_retrieve_err_code(QMI_NO_ERR, &((voice_answer_call_resp_msg_v02*)resp_data)->resp);
                    break;
                case QMI_VOICE_MANAGE_CALLS_RESP_V02:
                    cri_core_error = cri_core_retrieve_err_code(QMI_NO_ERR, &((voice_manage_calls_resp_msg_v02*)resp_data)->resp);
                    break;

                default:
                    //no action
                    break;

            }
            cri_rule_handler_rule_check(cri_core_context, cri_core_error, NULL);
        }
    }

    QCRIL_LOG_FUNC_RETURN();
}

