/******************************************************************************
  ---------------------------------------------------------------------------

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

#include "cri_voice_qmi_client.h"
#include "voice_service_v02.h"
#include "cri_voice_temp_defs.h"
//cri_voice_qmi_client_info_type qmi_client_info;

static void cri_voice_qmi_client_set_voice_client(cri_voice_qmi_client_info_type* qmi_client_info_ptr, int qmi_id);
static void cri_voice_qmi_client_set_csvt_client(cri_voice_qmi_client_info_type* qmi_client_info_ptr, int qmi_id);
static void cri_voice_voice_ind_registrations();
static void cri_voice_qmi_client_reset_client_info();

#define CRI_VOICE_INVALID_CLIENT_ID -1

int cri_voice_qmi_client_get_voice_client(const cri_voice_qmi_client_info_type* qmi_client_info_ptr)
{
    if (qmi_client_info_ptr)
    {
//        QCRIL_LOG_FUNC_RETURN_WITH_RET(qmi_client_info_ptr->qmi_voice_client_id);
        return qmi_client_info_ptr->qmi_voice_client_id;
    }
    else
    {
        return CRI_VOICE_INVALID_CLIENT_ID;
    }
}
int cri_voice_qmi_client_get_csvt_client(const cri_voice_qmi_client_info_type* qmi_client_info_ptr)
{
    if (qmi_client_info_ptr)
    {
//        QCRIL_LOG_FUNC_RETURN_WITH_RET(qmi_client_info_ptr->qmi_csvt_client_id);
        return qmi_client_info_ptr->qmi_voice_client_id;
    }
    else
    {
        return CRI_VOICE_INVALID_CLIENT_ID;
    }

}

// TODO: add params to specify what indications are interested
// TODO: combine csvt, csvt may fail on init but voice must success
qmi_error_type_v01 cri_voice_qmi_client_init(cri_voice_qmi_client_info_type* qmi_client_info_ptr, hlos_ind_cb_type hlos_ind_cb_ptr)
{
    qmi_error_type_v01 client_init_error = QMI_ERR_INTERNAL_V01;

    if (qmi_client_info_ptr)
    {
        int client_id = cri_core_create_qmi_service_client(QMI_VOICE_SERVICE, hlos_ind_cb_ptr);
        if (QMI_INTERNAL_ERR != client_id)
        {
            cri_voice_qmi_client_set_voice_client(qmi_client_info_ptr, client_id);
            cri_voice_voice_ind_registrations(qmi_client_info_ptr);
            client_init_error = QMI_ERR_NONE_V01;
        }
    }

    return client_init_error;
}

// TODO: combine csvt, csvt may fail on init but voice must success
void cri_voice_qmi_client_deinit(cri_voice_qmi_client_info_type* qmi_client_info_ptr)
{
    if (qmi_client_info_ptr)
    {
        cri_core_release_qmi_service_client(cri_voice_qmi_client_get_voice_client(qmi_client_info_ptr));
        cri_core_release_qmi_service_client(cri_voice_qmi_client_get_csvt_client(qmi_client_info_ptr));
        cri_voice_qmi_client_reset_client_info(qmi_client_info_ptr);
    }
}

hlos_ind_cb_type cri_voice_qmi_client_get_hlos_ind_cb(const cri_voice_qmi_client_info_type* qmi_client_info_ptr)
{
    if (qmi_client_info_ptr)
    {
        return cri_core_retrieve_hlos_ind_cb(cri_voice_qmi_client_get_voice_client(qmi_client_info_ptr));
    }
    else
    {
        return NULL;
    }
}

void cri_voice_qmi_client_set_voice_client(cri_voice_qmi_client_info_type* qmi_client_info_ptr, int qmi_id)
{
    if (qmi_client_info_ptr)
    {
        qmi_client_info_ptr->qmi_voice_client_id = qmi_id;
    }
}

void cri_voice_qmi_client_set_csvt_client(cri_voice_qmi_client_info_type* qmi_client_info_ptr, int qmi_id)
{
    if (qmi_client_info_ptr)
    {
        qmi_client_info_ptr->qmi_csvt_client_id = qmi_id;
    }
}

void cri_voice_qmi_client_reset_client_info(cri_voice_qmi_client_info_type* qmi_client_info_ptr)
{
    if (qmi_client_info_ptr)
    {
        cri_voice_qmi_client_set_voice_client(qmi_client_info_ptr, CRI_VOICE_INVALID_CLIENT_ID);
        cri_voice_qmi_client_set_csvt_client (qmi_client_info_ptr, CRI_VOICE_INVALID_CLIENT_ID);
    }
}

void cri_voice_voice_ind_registrations(cri_voice_qmi_client_info_type* qmi_client_info_ptr)
{
    if (!qmi_client_info_ptr)
    {
        return;
    }


    QCRIL_LOG_FUNC_ENTRY();

    voice_indication_register_req_msg_v02  indication_req;
    voice_indication_register_resp_msg_v02 indication_resp_msg;

    int voice_client_id = cri_voice_qmi_client_get_voice_client(qmi_client_info_ptr);


    memset(&indication_req, 0, sizeof(indication_req));
    memset(&indication_resp_msg, 0, sizeof(indication_resp_msg));
    indication_req.reg_voice_privacy_events_valid = TRUE;
    indication_req.reg_voice_privacy_events = 0x01;
    if ( QMI_ERR_NONE_V01 != cri_core_qmi_send_msg_sync( voice_client_id,
                                                         QMI_VOICE_INDICATION_REGISTER_REQ_V02,
                                                         (void *)&indication_req,
                                                         sizeof(indication_req),
                                                         &indication_resp_msg,
                                                         sizeof(indication_resp_msg),
                                                         CRI_CORE_MINIMAL_TIMEOUT )
        )
    {
        QCRIL_LOG_INFO("Voice_privacy events indication register failed!");
    }
    else
    {
        QCRIL_LOG_INFO("Voice_privacy events registration error code: %d", indication_resp_msg.resp.error);
    }

    memset(&indication_req, 0, sizeof(indication_req));
    memset(&indication_resp_msg, 0, sizeof(indication_resp_msg));
    indication_req.ext_brst_intl_events_valid = TRUE;
    indication_req.ext_brst_intl_events = 0x01;
    if ( QMI_ERR_NONE_V01 != cri_core_qmi_send_msg_sync( voice_client_id,
                                                         QMI_VOICE_INDICATION_REGISTER_REQ_V02,
                                                         (void *)&indication_req,
                                                         sizeof(indication_req),
                                                         &indication_resp_msg,
                                                         sizeof(indication_resp_msg),
                                                         CRI_CORE_MINIMAL_TIMEOUT )
        )
    {
        QCRIL_LOG_INFO("Extended_burst events indication register failed!");
    }
    else
    {
        QCRIL_LOG_INFO("Extended_burst events registration error code: %d", indication_resp_msg.resp.error);
    }

    memset(&indication_req, 0, sizeof(indication_req));
    memset(&indication_resp_msg, 0, sizeof(indication_resp_msg));
    indication_req.speech_events_valid = TRUE;
    indication_req.speech_events = 0x01;
    if ( QMI_ERR_NONE_V01 != cri_core_qmi_send_msg_sync( voice_client_id,
                                                         QMI_VOICE_INDICATION_REGISTER_REQ_V02,
                                                         (void *)&indication_req,
                                                         sizeof(indication_req),
                                                         &indication_resp_msg,
                                                         sizeof(indication_resp_msg),
                                                         CRI_CORE_MINIMAL_TIMEOUT )
        )
    {
        QCRIL_LOG_INFO("Speech events indication register failed!");
    }
    else
    {
        QCRIL_LOG_INFO("Speech events registration error code: %d", indication_resp_msg.resp.error);
    }

    //if (qcril_qmi_voice_info.jbims)
    if (1)
    {
        memset(&indication_req, 0, sizeof(indication_req));
        memset(&indication_resp_msg, 0, sizeof(indication_resp_msg));
        indication_req.handover_events_valid = TRUE;
        indication_req.handover_events = 0x01;
        indication_req.conference_events_valid = TRUE;
        indication_req.conference_events = 0x01;
        if ( QMI_ERR_NONE_V01 != cri_core_qmi_send_msg_sync( voice_client_id,
                                                             QMI_VOICE_INDICATION_REGISTER_REQ_V02,
                                                             (void *)&indication_req,
                                                             sizeof(indication_req),
                                                             &indication_resp_msg,
                                                             sizeof(indication_resp_msg),
                                                             CRI_CORE_MINIMAL_TIMEOUT )
            )
        {
            QCRIL_LOG_INFO("Handover events indication register failed!");
        }
        else
        {
            QCRIL_LOG_INFO("Handover events registration error code: %d", indication_resp_msg.resp.error);
        }
    }

    QCRIL_LOG_FUNC_RETURN();
}
