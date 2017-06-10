/******************************************************************************NS

  @file    qmi_uim_handle.c
  @brief

  DESCRIPTION

******************************************************************************/
/*===========================================================================

  Copyright (c) 2011-2012 Qualcomm Technologies, Inc. All Rights Reserved

  Qualcomm Technologies Proprietary

  Export of this technology or software is regulated by the U.S. Government.
  Diversion contrary to U.S. law prohibited.

  All ideas, data and information contained in or disclosed by
  this document are confidential and proprietary information of
  Qualcomm Technologies, Inc. and all rights therein are expressly reserved.
  By accepting this material the recipient agrees that this material
  and the information contained therein are held in confidence and in
  trust and will not be used, copied, reproduced in whole or in part,
  nor its contents revealed in any manner to others without the express
  written permission of Qualcomm Technologies, Inc.

===========================================================================*/
#include <utils/Log.h>
#include "sap_types.h"
#include "qmi_uim_handler.h"
#include <cutils/properties.h>

#undef LOGD
#undef LOGE
#undef LOGI
#undef LOGV

#define LOGD ALOGD
#define LOGI ALOGI
#define LOGE ALOGE
#define LOGV ALOGV

/* Android system property for fetching the modem type */
#define QMI_UIM_PROPERTY_BASEBAND               "ro.baseband"

/* Android system property values for various modem types */
#define QMI_UIM_PROP_BASEBAND_VALUE_SVLTE_1     "svlte1"
#define QMI_UIM_PROP_BASEBAND_VALUE_SVLTE_2A    "svlte2a"
#define QMI_UIM_PROP_BASEBAND_VALUE_CSFB        "csfb"
#define QMI_UIM_PROP_BASEBAND_VALUE_SGLTE       "sglte"
#define QMI_UIM_PROP_BASEBAND_VALUE_SGLTE2      "sglte2"
#define QMI_UIM_PROP_BASEBAND_VALUE_MSM         "msm"
#define QMI_UIM_PROP_BASEBAND_VALUE_APQ         "apq"
#define QMI_UIM_PROP_BASEBAND_VALUE_MDMUSB      "mdm"
#define QMI_UIM_PROP_BASEBAND_VALUE_MDM2USB     "mdm2"
#define QMI_UIM_PROP_BASEBAND_VALUE_DSDA        "dsda"
#define QMI_UIM_PROP_BASEBAND_VALUE_DSDA_2      "dsda2"
#define BT_PB_SAP_PROP_TARGET_PLATFORM          "ro.board.platform"
#define BT_PB_SAP_PROP_TARGET_PLATFORM_MSM8994  "msm8994"
#define BT_PB_SAP_PROP_TARGET_PLATFORM_MSM8916  "msm8916" /* For 8916, 8939 */
#define BT_PB_SAP_PROP_TARGET_PLATFORM_MSM8909  "msm8909"

qmi_client_type uim_qmi_client;
int qmi_handle;

/*Global Response structures
These are global as it has to be accessed from
callbacks*/
uim_sap_request_resp_msg_v01 sap_resp;
uim_sap_request_req_msg_v01 sap_req;

void uim_qmi_client_ind_cb
(
  qmi_client_type                user_handle,
  unsigned long                  msg_id,
  unsigned char                  *ind_buf,
  int                            ind_buf_len,
  void                           *ind_cb_data
)
{
        int i, len;
        uint32_t decoded_payload_len = 0;
        void* decoded_payload = NULL;
        uim_card_state_enum_v01 card_state;
        qmi_client_error_type qmi_err = QMI_NO_ERR;

        LOGV("%s: callback invoked\n", __func__);
        qmi_idl_get_message_c_struct_len((qmi_idl_service_object_type)ind_cb_data,
                                     QMI_IDL_INDICATION,
                                     (uint16_t) msg_id,
                                     &decoded_payload_len);
        if(decoded_payload_len)
        {
                LOGV("Allocating mem for decoded data");
                decoded_payload = malloc(decoded_payload_len);
                if ( decoded_payload != NULL )
                {
                        memset( decoded_payload, 0, decoded_payload_len);
                }
        }

        if (decoded_payload)
        {
            if( decoded_payload_len )
                qmi_err = qmi_client_message_decode(user_handle,
                                                QMI_IDL_INDICATION,
                                                msg_id,
                                                ind_buf,
                                                ind_buf_len,
                                                decoded_payload,
                                                (int)decoded_payload_len);

            if (qmi_err == QMI_NO_ERR) {
                     if (msg_id == QMI_UIM_STATUS_CHANGE_IND_V01) {
                             uim_status_change_ind_msg_v01 *stat =   (uim_status_change_ind_msg_v01*)decoded_payload;

                             if (stat->card_status_valid) {
                                     len =  stat->card_status.card_info_len;
                                     LOGV("%s: Card status info len: %d\n", __func__, len);
                                     for(i=0; i<len; i++) {
                                             LOGV("%s: Card status changed slot-%d: %d\n", __func__, i, stat->card_status.card_info[i].card_state);
                                     }
                                     /*get card state of slot-1*/
                                     card_state = stat->card_status.card_info[SIM_CARD_SLOT_1].card_state;
                                     /*UIM supports only below two card events in the connected state.
                                       It doesn't support CARD_INSERTED event*/
                                     if (card_state == UIM_CARD_STATE_ERROR_V01) {
                                            switch(stat->card_status.card_info[SIM_CARD_SLOT_1].error_code) {
                                                case UIM_CARD_ERROR_CODE_SAP_CONNECTED_V01:
                                                    LOGV("%s: Ignore Error: UIM_CARD_ERROR_CODE_SAP_CONNECTED:\n", __func__);
                                                    break;
                                                default:
                                                    send_card_status(CARD_STATUS_ERROR);
                                                    break;
                                            }
                                     }
                                     else if (card_state == UIM_CARD_STATE_ABSENT_V01) {
                                             send_card_status(CARD_STATUS_REMOVED);
                                     }
                             }
                     }
             }
             else {
                     LOGE("Indication decode failed\n");
             }
             if(decoded_payload_len) {
                     free(decoded_payload);
             }
        }
}

void clear_request_packet(void *buf, size_t size)
{
        LOGV("->%s", __func__);
        if(buf == NULL)
               return;

        memset(buf, 0, size);
        LOGV("<-%s", __func__);
}

static char * uim_find_modem_port( char * prop_value_ptr)
{
        char  * qmi_modem_port_ptr = QMI_PORT_RMNET_1;
        char prop_value[PROPERTY_VALUE_MAX];

        /* Sanity check */
        if (prop_value_ptr == NULL)
        {
                LOGE("%s", "NULL prop_value_ptr, using default port");
                return qmi_modem_port_ptr;
        }

        LOGI("Baseband property value read: %s\n", prop_value_ptr);

        /* Map the port based on the read property */
        if ((strcmp(prop_value_ptr, QMI_UIM_PROP_BASEBAND_VALUE_SVLTE_1)  == 0) ||
                        (strcmp(prop_value_ptr, QMI_UIM_PROP_BASEBAND_VALUE_SVLTE_2A) == 0) ||
                        (strcmp(prop_value_ptr, QMI_UIM_PROP_BASEBAND_VALUE_CSFB)     == 0))
        {
                qmi_modem_port_ptr = QMI_PORT_RMNET_SDIO_0;
        }
        else if ((strcmp(prop_value_ptr, QMI_UIM_PROP_BASEBAND_VALUE_MDMUSB) == 0) ||
                        (strcmp(prop_value_ptr, QMI_UIM_PROP_BASEBAND_VALUE_SGLTE2) == 0))
        {
                qmi_modem_port_ptr = QMI_PORT_RMNET_USB_0;
        }
        else if (strcmp(prop_value_ptr, QMI_UIM_PROP_BASEBAND_VALUE_MDM2USB) == 0)
        {
                qmi_modem_port_ptr = QMI_PORT_RMNET_MHI_0;
        }
        else if ((strcmp(prop_value_ptr, QMI_UIM_PROP_BASEBAND_VALUE_MSM)    == 0) ||
                        (strcmp(prop_value_ptr, QMI_UIM_PROP_BASEBAND_VALUE_APQ)    == 0) ||
                        (strcmp(prop_value_ptr, QMI_UIM_PROP_BASEBAND_VALUE_SGLTE) == 0))
        {
                memset(prop_value, 0x00, sizeof(prop_value));
                property_get(BT_PB_SAP_PROP_TARGET_PLATFORM, prop_value, "");
                if ((strcmp(BT_PB_SAP_PROP_TARGET_PLATFORM_MSM8994, prop_value) == 0)
                    || (strcmp(BT_PB_SAP_PROP_TARGET_PLATFORM_MSM8916, prop_value) == 0)
                    || (strcmp(BT_PB_SAP_PROP_TARGET_PLATFORM_MSM8909, prop_value) == 0))
                    qmi_modem_port_ptr = QMI_PORT_RMNET_0;
                else
                    qmi_modem_port_ptr = QMI_PORT_RMNET_1;
        }
        else if (strcmp(prop_value_ptr, QMI_UIM_PROP_BASEBAND_VALUE_DSDA) == 0)
        {
                /* If it is a DSDA configuration, use the existing API */
                qmi_modem_port_ptr = (char *)QMI_PLATFORM_INTERNAL_USE_PORT_ID;
        }
        else if (strcmp(prop_value_ptr, QMI_UIM_PROP_BASEBAND_VALUE_DSDA_2) == 0)
        {
                /* If it is a DSDA2 configuration, use the existing API */
                qmi_modem_port_ptr = (char *)QMI_PLATFORM_INTERNAL_USE_PORT_ID;
        }
        else
        {
                LOGE("%s", "Property value does not match, using default port");
        }

        LOGI("QMI port found for modem: %s\n", qmi_modem_port_ptr);

        return qmi_modem_port_ptr;
} /* uim_find_modem_port */

int init_qmi_uim()
{
        qmi_client_error_type qmi_client_err;
        qmi_idl_service_object_type uim_service;
        uim_event_reg_req_msg_v01 evt_req;
        uim_event_reg_resp_msg_v01 evt_resp;
        char prop_value[PROPERTY_VALUE_MAX];
        char *qmi_modem_port = NULL;

        qmi_handle = qmi_init(NULL, NULL);
        if( qmi_handle < 0 ) {
                LOGE("%s: Error while initializing qmi:\n", __func__);
                return -1;
        }

        uim_service = uim_get_service_object_v01();

        if(uim_service == NULL) {
                LOGE("%s: Not able to get the service handle\n", __func__);
                return -1;
        }
        /* Find out the modem type */
        memset(prop_value, 0x00, sizeof(prop_value));
        property_get(QMI_UIM_PROPERTY_BASEBAND, prop_value, "");

        /* Map to a respective QMI port */
        qmi_modem_port = uim_find_modem_port(prop_value);

        if(qmi_modem_port == NULL) {
                LOGE("%s: qmi_modem_port is NULL \n", __func__);
                return -1;
        }

        qmi_client_err = qmi_client_init((const char *)qmi_modem_port, uim_service, uim_qmi_client_ind_cb, uim_service, &uim_qmi_client);
        if(qmi_client_err != QMI_NO_ERR){
                LOGE("%s :Error while Initializing qmi_client_init : %d\n", __func__, qmi_client_err);
                return -1;
        }

        clear_request_packet(&evt_req, sizeof(evt_req));
        /*Registering for the SIM card status events*/
        evt_req.event_mask = 0x01;
        qmi_client_err = qmi_client_send_msg_sync(uim_qmi_client,
                        QMI_UIM_EVENT_REG_REQ_V01, &evt_req, sizeof(evt_req), &evt_resp, sizeof(evt_resp), SAP_QMI_TIMEOUT);

        if(qmi_client_err != QMI_NO_ERR){
                LOGE( "%s: Error:  %d\n", __func__, qmi_client_err);
                return -1;
        }

        if (evt_resp.event_mask_valid && (evt_resp.event_mask | 0x01)) {
                LOGE("%s: Succesfully registered for Card events\n", __func__);
        }

        LOGV("<-%s", __func__);
        return 0;
}

void cleanup_qmi_uim()
{
        qmi_client_error_type qmi_client_err;
        LOGV("->%s", __func__);

        qmi_client_err = qmi_client_release(uim_qmi_client);
        if(qmi_client_err != QMI_NO_ERR){
                LOGE("Error: while releasing qmi_client : %d\n", qmi_client_err);
        }

        qmi_handle = qmi_release(qmi_handle);
        if( qmi_handle < 0 )    {
                LOGE("Error: while releasing qmi %d\n", qmi_handle);
                //Ignore cleanup errors
        }

        LOGV("<-%s", __func__);
}

int qmi_sap_connect(sap_status_change *status)
{
        uim_sap_connection_req_msg_v01 req;
        uim_sap_connection_resp_msg_v01 resp;
        qmi_client_error_type qmi_client_err;

        LOGV("->%s", __func__);
        clear_request_packet(&req, sizeof(req));
        req.sap_connect.connect = UIM_SAP_OP_CONNECT_V01;
        req.sap_connect.slot = UIM_SLOT_1_V01;

        req.disconnect_mode_valid = 0;

        /* Set the TLV to block SAP connection only when there is on
         * going voice call and ignore on going data calls */
        req.connection_condition_valid = 1;
        req.connection_condition = UIM_SAP_CONNECTION_COND_BLOCK_VOICE_V01;

        qmi_client_err = qmi_client_send_msg_sync(uim_qmi_client,
                        QMI_UIM_SAP_CONNECTION_REQ_V01, &req, sizeof(req), &resp, sizeof(resp), SAP_QMI_TIMEOUT);

        if(qmi_client_err != QMI_NO_ERR){
                LOGE( "%s: error while requesting: %d\n", __func__, qmi_client_err);
                return -1;
        }

         LOGV("response result type: %d", resp.resp.result );
         LOGV("response error type: %d", resp.resp.error);
         LOGV("response state_valid: %d", resp.sap_state_valid);
         LOGV("response state: %d", resp.sap_state);

         if (resp.resp.result == QMI_RESULT_SUCCESS_V01) {
                 *status = CARD_RESET;
         } else {
                 if ( resp.resp.error == QMI_ERR_DEVICE_NOT_READY_V01 ) {
                         *status = CARD_BUSY;
                 }
                 else {
                          *status = CARD_REMOVED;
                 }
         }
         LOGV("<-%s", __func__);
         return 0;
}

/*
 * qmi_sap_disconnect
 */
int qmi_sap_disconnect(uim_sap_disconnect_mode_enum_v01 a_mode, boolean isSocketValid)
{

        uim_sap_connection_req_msg_v01 req;
        uim_sap_connection_resp_msg_v01 resp;
        qmi_client_error_type qmi_client_err;
        qmi_txn_handle txn_handle;
        sap_internal_error err;

        clear_request_packet(&req, sizeof(req));
        req.sap_connect.connect = UIM_SAP_OP_DISCONNECT_V01;
        req.sap_connect.slot = UIM_SLOT_1_V01;

        req.disconnect_mode_valid = 1;
        req.disconnect_mode = a_mode;

        qmi_client_err = qmi_client_send_msg_sync(uim_qmi_client,
                        QMI_UIM_SAP_CONNECTION_REQ_V01, &req, sizeof(req), &resp, sizeof(resp), SAP_QMI_TIMEOUT);

        if(qmi_client_err != QMI_NO_ERR){
                LOGE( "%s: error while requesting %d\n", __func__, qmi_client_err);
                return -1;
        }

        LOGV("response result type: %d", resp.resp.result );
        LOGV("response error type: %d", resp.resp.error);

        /*In any case clean up the QMI*/
        cleanup_qmi_uim();
        if (isSocketValid) {
                err = send_disconn_resp(conn_sk);
                LOGV("%s: send status: %d", __func__, err);
        }
        return 0;
}

/*
 * APDU response callback
 */
void qmi_sap_tx_apdu_resp_cb
(
  qmi_client_type                user_handle,
  unsigned long                  msg_id,
  void                           *resp_c_struct,
  int                            resp_c_struct_len,
  void                           *resp_cb_data,
  qmi_client_error_type          transp_err
)
{
        LOGV("->%s", __func__);
        uim_sap_request_resp_msg_v01* resp = (uim_sap_request_resp_msg_v01*)resp_c_struct;
        sap_internal_error err;

        if(resp != NULL) {
                LOGV("response result type: %d", resp->resp.result );
                LOGV("response error type: %d", resp->resp.error);
                if(resp->resp.result == QMI_RESULT_SUCCESS_V01 && resp->apdu_valid) {
                        err = send_tx_apdu_resp(conn_sk, ERR_NONE, resp->apdu ,resp->apdu_len);
                }
                else {
                        err = send_tx_apdu_resp(conn_sk, ERR_UNDEFINED, NULL ,0);
                }
                LOGV("%s: send status: %d", __func__, err);
        } else {
                LOGE("%s: resp structure is NULL", __func__);
        }
}

/*
 * qmi ATR response callback
 */
void qmi_sap_tx_atr_resp_cb
(
  qmi_client_type                user_handle,
  unsigned long                  msg_id,
  void                           *resp_c_struct,
  int                            resp_c_struct_len,
  void                           *resp_cb_data,
  qmi_client_error_type          transp_err
)
{
        LOGV("-> %s", __func__);

        uim_sap_request_resp_msg_v01* resp = (uim_sap_request_resp_msg_v01*)resp_c_struct;
        sap_internal_error err;

        if (resp != NULL) {
                LOGV("response result type: %d", resp->resp.result );
                LOGV("response error type: %d", resp->resp.error);
                if(resp->resp.result == QMI_RESULT_SUCCESS_V01 && resp->ATR_value_valid) {
                        err = send_tx_atr_resp(conn_sk, ERR_NONE, resp->ATR_value ,resp->ATR_value_len);
                }
                else {
                        err = send_tx_atr_resp(conn_sk, ERR_UNDEFINED, NULL ,0);
                }
                LOGV("%s: send status : %d", __func__, err);
        } else {
                LOGE("%s: response structure is NULL", __func__);
        }
}

/*
 * qmi ATR request
 */
int qmi_sap_transfer_atr()
{

        qmi_client_error_type qmi_client_err;
        qmi_txn_handle txn_handle;

        clear_request_packet(&sap_req, sizeof(sap_req));
        sap_req.sap_request.sap_request = UIM_SAP_REQUEST_GET_ATR_V01;
        sap_req.sap_request.slot = UIM_SLOT_1_V01;

        sap_req.apdu_valid = FALSE;
        qmi_client_err = qmi_client_send_msg_async(uim_qmi_client,
                        QMI_UIM_SAP_REQUEST_REQ_V01, &sap_req, sizeof(sap_req), &sap_resp,
                        sizeof(sap_resp), qmi_sap_tx_atr_resp_cb, NULL, &txn_handle);

        if(qmi_client_err != QMI_NO_ERR){
                LOGE( "%s: error while requesting: %d\n", __func__, qmi_client_err);
                return -1;
        }

        return 0;
}

/*
 * qmi transfer apdu
 */
int qmi_sap_transfer_apdu()
{
        qmi_client_error_type qmi_client_err;
        qmi_txn_handle txn_handle;

        sap_req.sap_request.sap_request = UIM_SAP_REQUEST_SEND_APDU_V01;
        sap_req.sap_request.slot = UIM_SLOT_1_V01;

        sap_req.apdu_valid = TRUE;
        qmi_client_err = qmi_client_send_msg_async(uim_qmi_client,
                        QMI_UIM_SAP_REQUEST_REQ_V01, &sap_req, sizeof(sap_req), &sap_resp,
                        sizeof(sap_resp), qmi_sap_tx_apdu_resp_cb, NULL, &txn_handle);

        if(qmi_client_err != QMI_NO_ERR){
                LOGE("%s: error while requesting: %d\n", __func__, qmi_client_err);
                return -1;
        }

        return 0;
}

/*
 * qmi sim off response callback
 */
void qmi_sap_power_off_sim_resp_cb
(
  qmi_client_type                user_handle,
  unsigned long                  msg_id,
  void                           *resp_c_struct,
  int                            resp_c_struct_len,
  void                           *resp_cb_data,
  qmi_client_error_type          transp_err
)
{
        uim_sap_request_resp_msg_v01* resp = (uim_sap_request_resp_msg_v01*)resp_c_struct;
        sap_internal_error err;
        sap_result_code res;

        LOGV("-> %s", __func__);

        if(resp != NULL) {
                LOGV("response result type: %d", resp->resp.result );
                LOGV("response error type: %d", resp->resp.error);

                if (resp->resp.result == QMI_RESULT_SUCCESS_V01) {
                        res = ERR_NONE;
                        sim_card_state = SIM_CARD_POWERED_OFF;
                } else {
                        res = ERR_UNDEFINED;
                }
                err = send_sim_off_resp(conn_sk, res);
                LOGV("%s: send status: %d", __func__, err);
        } else {
                LOGE("%s: resp structure is NULL", __func__);
        }
}

/*
 * qmi sim off request handling
 */
int qmi_sap_power_off_sim()
{

        qmi_client_error_type qmi_client_err;
        qmi_txn_handle txn_handle;
        sap_internal_error err;

        if(sim_card_state == SIM_CARD_POWERED_OFF) {
                err = send_sim_off_resp(conn_sk, ERR_CARD_POWERED_OFF);
                LOGV("%s: send status: %d", __func__, err);
                return 0;
        }
        else if(sim_card_state == SIM_CARD_REMOVED) {
                err = send_sim_off_resp(conn_sk, ERR_CARD_REMOVED);
                LOGV("%s: send status: %d", __func__, err);
                return 0;
        }

        clear_request_packet(&sap_req, sizeof(sap_req));
        sap_req.sap_request.sap_request = UIM_SAP_REQUEST_POWER_SIM_OFF_V01;
        sap_req.sap_request.slot = UIM_SLOT_1_V01;

        sap_req.apdu_valid = FALSE;
        qmi_client_err = qmi_client_send_msg_async(uim_qmi_client,
                        QMI_UIM_SAP_REQUEST_REQ_V01, &sap_req, sizeof(sap_req), &sap_resp,
                        sizeof(sap_resp), qmi_sap_power_off_sim_resp_cb, NULL, &txn_handle);

        if(qmi_client_err != QMI_NO_ERR){
                LOGE( "%s: error while requesting: %d\n", __func__, qmi_client_err);
                return -1;
        }

        return 0;
}

/*
 * qmi sim on response callback
 */
void qmi_sap_power_on_sim_resp_cb
(
  qmi_client_type                user_handle,
  unsigned long                  msg_id,
  void                           *resp_c_struct,
  int                            resp_c_struct_len,
  void                           *resp_cb_data,
  qmi_client_error_type          transp_err
)
{
        LOGV("-> %s", __func__);

        uim_sap_request_resp_msg_v01* resp = (uim_sap_request_resp_msg_v01*)resp_c_struct;
        sap_result_code res;
        sap_internal_error err;

        if(resp != NULL) {
                LOGV("response result type: %d", resp->resp.result );
                LOGV("response error type: %d", resp->resp.error);

                if (resp->resp.result == QMI_RESULT_SUCCESS_V01) {
                        res = ERR_NONE;
                        sim_card_state = SIM_CARD_POWERED_ON;
                } else {
                        res = ERR_UNDEFINED;
                }
                err = send_sim_on_resp(conn_sk, res);
                LOGV("%s: send status : %d", __func__, err);

        } else {
                LOGE("%s: resp structure is NULL", __func__);
        }
}

/*
 * qmi sim on request handling
 */
int qmi_sap_power_on_sim()
{

        qmi_client_error_type qmi_client_err;
        qmi_txn_handle txn_handle;
        sap_internal_error err;

        if(sim_card_state == SIM_CARD_POWERED_ON) {
                err = send_sim_on_resp(conn_sk, ERR_CARD_ALREADY_ON);
                LOGV("%s: send status: %d", __func__, err);
                return 0;
        }
        else if(sim_card_state == SIM_CARD_REMOVED) {
                err = send_sim_on_resp(conn_sk, ERR_CARD_REMOVED);
                LOGV("%s: send status: %d", __func__, err);
                return 0;
        }
        else if(sim_card_state == SIM_CARD_NOT_ACCESSIBLE) {
                err = send_sim_on_resp(conn_sk, ERR_CARD_NOT_ACCESSIBLE);
                LOGV("%s: send status: %d", __func__, err);
                return 0;
        }

        clear_request_packet(&sap_req, sizeof(sap_req));
        sap_req.sap_request.sap_request = UIM_SAP_REQUEST_POWER_SIM_ON_V01;
        sap_req.sap_request.slot = UIM_SLOT_1_V01;

        sap_req.apdu_valid = FALSE;
        qmi_client_err = qmi_client_send_msg_async(uim_qmi_client,
                        QMI_UIM_SAP_REQUEST_REQ_V01, &sap_req, sizeof(sap_req), &sap_resp,
                        sizeof(sap_resp), qmi_sap_power_on_sim_resp_cb, NULL, &txn_handle);

        if(qmi_client_err != QMI_NO_ERR){
                LOGE( "%s: error while requesting: %d\n", __func__, qmi_client_err);
                return -1;
        }

        return 0;
}

/*
 * qmi sim reset callback
 */
void qmi_sap_reset_sim_resp_cb
(
  qmi_client_type                user_handle,
  unsigned long                  msg_id,
  void                           *resp_c_struct,
  int                            resp_c_struct_len,
  void                           *resp_cb_data,
  qmi_client_error_type          transp_err
)
{
        LOGV("-> %s", __func__);

        uim_sap_request_resp_msg_v01* resp = (uim_sap_request_resp_msg_v01*)resp_c_struct;
        sap_result_code res;
        sap_internal_error err;

        if(resp != NULL) {
                LOGV("response result type: %d", resp->resp.result );
                LOGV("response error type: %d", resp->resp.error);

                if (resp->resp.result == QMI_RESULT_SUCCESS_V01) {
                        res = ERR_NONE;
                        sim_card_state = SIM_CARD_POWERED_ON;
                } else {
                        res = ERR_UNDEFINED;
                }
                err = send_sim_reset_resp(conn_sk, res);
                LOGV("%s: send status : %d", __func__, err);
        } else {
                LOGE("%s: resp structure is NULL", __func__);
        }
}

int qmi_sap_reset_sim()
{

        qmi_client_error_type qmi_client_err;
        qmi_txn_handle txn_handle;
        sap_internal_error err;

        if(sim_card_state == SIM_CARD_POWERED_OFF) {
                err = send_sim_reset_resp(conn_sk, ERR_CARD_POWERED_OFF);
                LOGV("%s: send status: %d", __func__, err);
                return 0;
        }
        else if(sim_card_state == SIM_CARD_REMOVED) {
                err = send_sim_reset_resp(conn_sk, ERR_CARD_REMOVED);
                LOGV("%s: send status: %d", __func__, err);
                return 0;
        }
        else if(sim_card_state == SIM_CARD_NOT_ACCESSIBLE) {
                err = send_sim_reset_resp(conn_sk, ERR_CARD_NOT_ACCESSIBLE);
                LOGV("%s: send status: %d", __func__, err);
                return 0;
        }

        clear_request_packet(&sap_req, sizeof(sap_req));
        sap_req.sap_request.sap_request = UIM_SAP_REQUEST_RESET_SIM_V01;
        sap_req.sap_request.slot = UIM_SLOT_1_V01;

        sap_req.apdu_valid = FALSE;
        qmi_client_err = qmi_client_send_msg_async(uim_qmi_client,
                        QMI_UIM_SAP_REQUEST_REQ_V01, &sap_req, sizeof(sap_req), &sap_resp,
                        sizeof(sap_resp), qmi_sap_reset_sim_resp_cb, NULL, &txn_handle);

        if(qmi_client_err != QMI_NO_ERR){
                LOGE( "%s: error while requesting: %d\n", __func__, qmi_client_err);
                return -1;
        }

        return 0;
}

/*
 * qmi card reader status callback
 */
void qmi_sap_card_reader_stat_cb
(
  qmi_client_type                user_handle,
  unsigned long                  msg_id,
  void                           *resp_c_struct,
  int                            resp_c_struct_len,
  void                           *resp_cb_data,
  qmi_client_error_type          transp_err
)
{
        LOGV("-> %s", __func__);

        uim_sap_request_resp_msg_v01* resp = (uim_sap_request_resp_msg_v01*)resp_c_struct;
        sap_result_code res;
        sap_status_change status;
        sap_internal_error err;

        if (resp != NULL) {
                LOGV("response result type: %d", resp->resp.result );
                LOGV("response error type: %d", resp->resp.error);

                if (resp->resp.result == QMI_RESULT_SUCCESS_V01) {
                        res = ERR_NONE;
                        if(resp->status_valid)  {
                                err = send_tx_card_stat(conn_sk, res, resp->status, resp->status_len);
                        }
                        else
                        {
                                /*No valid Status*/
                                err = send_tx_card_stat(conn_sk, res, NULL, 0);
                        }
                } else {
                        res = ERR_UNDEFINED;
                        err = send_tx_card_stat(conn_sk, res, NULL, 0);
                }
                LOGV("%s: send status : %d", __func__, err);
        } else {
                LOGE("%s: resp structure is NULL", __func__);
        }
}

/*
 * qmi get card status handling
 */
int qmi_sap_get_card_reader_stat()
{
        qmi_client_error_type qmi_client_err;
        qmi_txn_handle txn_handle;

        clear_request_packet(&sap_req, sizeof(sap_req));
        sap_req.sap_request.sap_request = UIM_SAP_REQUEST_CARD_READER_STATUS_V01;
        sap_req.sap_request.slot = UIM_SLOT_1_V01;

        sap_req.apdu_valid = FALSE;
        qmi_client_err = qmi_client_send_msg_async(uim_qmi_client,
                        QMI_UIM_SAP_REQUEST_REQ_V01, &sap_req, sizeof(sap_req), &sap_resp, sizeof(sap_resp),
                        qmi_sap_card_reader_stat_cb, NULL, &txn_handle);

        if(qmi_client_err != QMI_NO_ERR){
                LOGE("%s: error while requesting: %d\n", __func__, qmi_client_err);
                return -1;
        }

        return 0;
}

