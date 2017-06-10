/******************************************************************************
  ---------------------------------------------------------------------------

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

#include "cri_dms_utils.h"
#include "cri_dms_core.h"
#include "cri_core.h"
#include "device_management_service_v01.h"

int dms_client_id;

cri_dms_operating_mode modem_operating_mode;

qmi_error_type_v01 cri_dms_utils_init_client(hlos_ind_cb_type hlos_ind_cb)
{
    qmi_error_type_v01 client_init_error;

    client_init_error = QMI_ERR_INTERNAL_V01;

    dms_client_id = cri_core_create_qmi_service_client(QMI_DMS_SERVICE, hlos_ind_cb);
    if(QMI_INTERNAL_ERR != dms_client_id)
    {
        client_init_error = QMI_ERR_NONE_V01;
        cri_dms_init_client_state();
    }

    return client_init_error;
}

void cri_dms_indication_subscription(uint32_t is_subscribe)
{
    cri_core_error_type ret_val;
    dms_set_event_report_req_msg_v01 dms_set_event_report_req_msg;
    dms_set_event_report_resp_msg_v01 dms_set_event_report_resp_msg;

    memset(&dms_set_event_report_req_msg, NIL, sizeof(dms_set_event_report_req_msg_v01));
    memset(&dms_set_event_report_resp_msg, NIL, sizeof(dms_set_event_report_resp_msg_v01));

    dms_set_event_report_req_msg.report_oprt_mode_state_valid = TRUE;
    dms_set_event_report_req_msg.report_oprt_mode_state = is_subscribe;

    ret_val = cri_core_qmi_send_msg_sync(dms_client_id,
                                        QMI_DMS_SET_EVENT_REPORT_REQ_V01,
                                        (void *)&dms_set_event_report_req_msg,
                                        sizeof(dms_set_event_report_req_msg_v01),
                                        &dms_set_event_report_resp_msg,
                                        sizeof(dms_set_event_report_resp_msg_v01),
                                        CRI_CORE_MINIMAL_TIMEOUT);

}

qmi_error_type_v01 cri_dms_init_client_state()
{
    cri_core_error_type ret_val;
    dms_set_operating_mode_req_msg_v01 set_opr_mode_req_msg;
    dms_set_operating_mode_resp_msg_v01 set_opr_mode_resp_msg;
    memset(&set_opr_mode_req_msg,
            NIL,
            sizeof(set_opr_mode_req_msg));

        memset(&set_opr_mode_resp_msg,
            NIL,
            sizeof(set_opr_mode_resp_msg));

    ret_val = QMI_ERR_NONE_V01;
    set_opr_mode_req_msg.operating_mode = DMS_OP_MODE_LOW_POWER_V01;

    cri_dms_indication_subscription(TRUE);

#if 0 // TODO: check with sai on whether to use cri set modem api or sync api.do we really need this?
    // as part of init, CRI DMS should move modem to offline state and then online state.
    ret_val = cri_core_qmi_send_msg_sync(dms_client_id,
                                        QMI_DMS_SET_OPERATING_MODE_REQ_V01,
                                        &set_opr_mode_req_msg,
                                        sizeof(set_opr_mode_req_msg),
                                        &set_opr_mode_resp_msg,
                                        sizeof(set_opr_mode_resp_msg),
                                        CRI_CORE_MINIMAL_TIMEOUT);
#endif // 0

    return ret_val;

}

void cri_dms_utils_release_client(int qmi_service_client_id)
{
    cri_dms_indication_subscription(FALSE);
    cri_core_release_qmi_service_client(qmi_service_client_id);
    dms_client_id = NIL;
}


void cri_dms_utils_update_operating_mode(dms_operating_mode_enum_v01 opr_mode)
{
    modem_operating_mode.is_valid = TRUE;
    modem_operating_mode.current_operating_mode = opr_mode;
}

void cri_dms_utils_get_current_operating_mode(dms_operating_mode_enum_v01 *opr_mode)
{
    if ( modem_operating_mode.is_valid )
    {
        *opr_mode = modem_operating_mode.current_operating_mode;
    }
}

uint32_t cri_dms_utils_is_valid_operating_mode()
{
    return modem_operating_mode.is_valid;
}



