/******************************************************************************

  @file    sap_server.c
  @brief

  DESCRIPTION

******************************************************************************/
/*===========================================================================

  Copyright (c) 2011-2013 Qualcomm Technologies, Inc. All Rights Reserved

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
/*Including necessary header files*/
#include <utils/Log.h>
#include <stdlib.h>

#include <time.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <sys/prctl.h>
#include <sys/types.h>
#include <signal.h>
#include <cutils/sockets.h>
#include <sys/un.h>
#include <sys/ioctl.h>

#include "comdef.h"

//Local includes
#include "sap_types.h"
#include "qmi_uim_handler.h"
#include "sap_authorize.h"
#undef LOGD
#undef LOGE
#undef LOGI
#undef LOGV

#define LOGD ALOGD
#define LOGI ALOGI
#define LOGE ALOGE
#define LOGV ALOGV


static int local_sk;
int conn_sk;
int disconn_sk;
static time_t disconnect_initiated_time;
static pthread_t read_ongoing_status_tid;

static int server_evt_pipe[2];
/* flag to determine if there was a error_resp sent
 * due to invalid packet
 */
static boolean error_occured = FALSE;

/* Mutex for controlling the sap state updates
*/
static pthread_mutex_t state_mutex;
sap_state_type sap_state = DISCONNECTED;

sap_sim_card_state sim_card_state = SIM_CARD_NOT_ACCESSIBLE;

void set_sap_state(sap_state_type a_sap_state)
{
        pthread_mutex_lock(&state_mutex);
        sap_state = a_sap_state;
        pthread_mutex_unlock(&state_mutex);
}


void get_bdaddr_as_string(const bdaddr_t *ba, char *str) {
        const uint8_t *b = (const uint8_t *)ba;
        snprintf(str, BTADDR_SIZE, "%2.2X:%2.2X:%2.2X:%2.2X:%2.2X:%2.2X",
                b[5], b[4], b[3], b[2], b[1], b[0]);
}


boolean validate_param_header (unsigned char* a_param_hdr_buf, sap_param_id a_id)
{
        unsigned short len = ntohs(*((unsigned short*)&a_param_hdr_buf[2]));

        LOGV("->%s: exp_id:%d, id: %d, len: %d ", __func__, a_id, (sap_param_id)a_param_hdr_buf[0], len);

        if ((a_param_hdr_buf[0] >= 10) && (a_param_hdr_buf[0] <= 15)) {
                LOGE("Error while validating: Invalid exp_id\n");
                return FALSE;
        }

        if ((a_id != (sap_param_id)a_param_hdr_buf[0]) || (
                (sap_param_length[a_id] != -1 ) && (len != sap_param_length[a_id] )) ) {
                LOGE("Error while validating\n");
                return FALSE;
        }
        LOGV("<-%s: %d", __func__, TRUE);
        return TRUE;
}

void clear_contents(int a_remote_sk)
{
        int len = 0;

        LOGV("->%s", __func__);

        if ((ioctl(a_remote_sk, FIONREAD, &len)) < 0) {
            LOGE("->%s: IOCTL failed", __func__);
        } else if (len > 0) {
            LOGV("->%s: No. of Bytes received in IOCTL: %d", __func__, len);

            char *dummy_buf = malloc(sizeof(char) * (len + 1));
            if (dummy_buf == NULL) {
               LOGE("%s:Failed to allocate the dummy buffer", __func__);
               return;
            }

            len = recv(a_remote_sk, dummy_buf, len, 0);

            LOGV("%s:Read %d bytes from the socket", __func__, len);

            if (dummy_buf != NULL) {
                free(dummy_buf);
                dummy_buf = NULL;
            }
        } else {
            LOGV("%s:All sockets contents are read: %s", __func__, strerror(errno));
        }
}

void *read_ongoing_status_thread(void *arg)
{
    int retval;
    sap_status_change status = UNKNOWN_ERROR;
    LOGD("%s: ", __func__);
    do {
            retval = qmi_sap_connect(&status);
            if(retval < 0) {
                    LOGE("%s Error with QMI connect: %s\n", __func__, strerror(errno));
                    return NULL;
            }
            LOGD("%s qmi_sap_connect returned status = %d\n", __func__, status);
    } while (status == CARD_BUSY);
    if( status == CARD_RESET) {
        send_status_ind(conn_sk, status);
    }
    LOGE("exsting from %s\n", __func__);
    pthread_exit(0);

    return NULL;
}

sap_internal_error process_conn_req(int a_remote_sk, unsigned char a_param_cnt)
{
        int retval;
        unsigned short max_msg_sz;
        unsigned char param_hdr_buf[SAP_PARAM_HEADER_SIZE];
        unsigned char param_pl[4];
        sap_status_change status = UNKNOWN_ERROR;
        sap_connection_status conn_status = CONN_ERR;
        unsigned short size;
        sap_internal_error err = SAP_ERR_NONE;

        if (a_param_cnt != sap_param_count[CONNECT_REQ]) {
                return SAP_ERR_ILLEGAL_ARG;
        }

        /*Read the params*/
        retval = recv(a_remote_sk, &param_hdr_buf, SAP_PARAM_HEADER_SIZE, 0);
        if (retval < 0) {
                LOGE("%s: Not able to read from the socket: %s", __func__, strerror(errno));
                return SAP_ERR_RCV_FAIL;
        }

        if (retval != SAP_PARAM_HEADER_SIZE) {
            LOGE("%s: No of bytes read by the buffer(%d) is not equal to requested number of bytes %s",
            __func__, retval, strerror(errno));
            return SAP_ERR_RCV_FAIL;
        }

        if (!validate_param_header(param_hdr_buf, MAX_MSG_SIZE)) {
                return SAP_ERR_ILLEGAL_ARG;
        }

        retval = recv(a_remote_sk, param_pl, (sap_param_length[MAX_MSG_SIZE]+FOUR_BYTE_PADDING(sap_param_length[MAX_MSG_SIZE])), 0);

        if (retval != (sap_param_length[MAX_MSG_SIZE]+FOUR_BYTE_PADDING(sap_param_length[MAX_MSG_SIZE]))) {
            LOGE("%s: No of bytes read by the buffer(%d) is not equal to requested number of bytes %s",
            __func__, retval, strerror(errno));
            return SAP_ERR_RCV_FAIL;
        }
        max_msg_sz = ntohs(*((unsigned short*)&param_pl));

        LOGV("%s: Client asking for %d max_msg size", __func__,  max_msg_sz);

        if(max_msg_sz > SAP_MAX_MSG_LEN) {
                conn_status = CONN_MAX_SIZE_NOT_SUPPORTED;
                size = SAP_MAX_MSG_LEN;
                err = SAP_ERR_UNDEFINED;
        }
        else {
                conn_status = CONN_OK;
                size = 0;
        }
        /*Process through QMI*/
        if (conn_status == CONN_OK) {
                retval = init_qmi_uim();
                if (retval < 0) {
                         LOGE("Error while initializing QMI: %s\n", strerror(errno));
                         return SAP_ERR_QMI;
                }
                retval = qmi_sap_connect(&status);
                if(retval < 0) {
                         LOGE("Error with QMI connect: %s\n", strerror(errno));
                         return SAP_ERR_QMI;
                }
                else if (status == CARD_BUSY ) {
                         conn_status = CONN_ONGOING_CALL;
                         /*  Set the sap state to connecting as it is still
                             trying to connect */
                         set_sap_state(CONNECTING);
                }
       }
       err = send_conn_resp(a_remote_sk, conn_status, size);
       if (err != SAP_ERR_NONE) {
               LOGE("Error in sending the Status Ind: %s\n", strerror(errno));
               return SAP_ERR_SND_FAIL;
       }

       if ( conn_status == CONN_ONGOING_CALL ) {
          /* Create a new thread to check for call status */
          LOGD("Call in progress, creating a thread to poll for call status change\n");
          if (pthread_create(&read_ongoing_status_tid, NULL, &read_ongoing_status_thread, NULL)) {
              LOGE("failed to create thread for call status polling, error: %s\n", strerror(errno));
          }
        }

        if( status == CARD_RESET) {
                err = send_status_ind(conn_sk, status);
                LOGV("%s: send status: %d", __func__, err);
        } else if (conn_status == CONN_ONGOING_CALL) {
                err = SAP_ERR_NONE;
        } else {
                err = SAP_ERR_QMI;
        }

        LOGV("<-%s", __func__);
        return err;
}

sap_internal_error process_tx_apdu_req(int a_remote_sk, unsigned char a_param_cnt)
{
        int retval, i, array_size;
        unsigned char param_hdr_buf[SAP_PARAM_HEADER_SIZE];
        sap_internal_error err = SAP_ERR_NONE;
        /*There can be atmost two params */
        if (a_param_cnt > sap_param_count[TRANSFER_APDU_REQ]) {
                return SAP_ERR_ILLEGAL_ARG;
        }

        /*Read the commandAPDU*/
        retval = recv(a_remote_sk, &param_hdr_buf, SAP_PARAM_HEADER_SIZE, 0);
        if (retval < 0) {
                LOGE("%s: Not able to read from the socket: %s", __func__, strerror(errno));
                /*Should take care of informing to parent*/
                return SAP_ERR_RCV_FAIL;
        }

        if (retval != SAP_PARAM_HEADER_SIZE) {
            LOGE("%s: No of bytes read by the buffer(%d) is not equal to requested number of bytes %s",
            __func__, retval, strerror(errno));
            return SAP_ERR_RCV_FAIL;
        }

        if (!validate_param_header(param_hdr_buf, COMMAND_APDU) && !validate_param_header(param_hdr_buf, COMMAND_APDU_7816))    {
                return SAP_ERR_ILLEGAL_ARG;
        }

        clear_request_packet(&sap_req, sizeof(sap_req));
        sap_req.apdu_len = ntohs(*(unsigned short*)&param_hdr_buf[2]);
        retval = recv(a_remote_sk, sap_req.apdu, (sap_req.apdu_len+FOUR_BYTE_PADDING(sap_req.apdu_len)), 0);

        if (retval != (sap_req.apdu_len+FOUR_BYTE_PADDING(sap_req.apdu_len))) {
            LOGE("%s: No of bytes read by the buffer(%d) is not equal to requested number of bytes %s",
            __func__, retval, strerror(errno));
            return SAP_ERR_RCV_FAIL;
        }

        LOGV("apdu len: %d\n",  sap_req.apdu_len);
        LOGV("APDU: Start<");

        array_size = sizeof(sap_req.apdu)/sizeof(sap_req.apdu[0]);
        for(i =0; i<(int)sap_req.apdu_len && i<array_size; i++){
                LOGV("%x ", sap_req.apdu[i]);
        }
        LOGV("APDU: Stop>");

        retval = qmi_sap_transfer_apdu();

        if(retval < 0) {
                err = SAP_ERR_QMI;
        }

        return err;
}


sap_internal_error process_disconn_req(unsigned char a_param_cnt)
{
        sap_internal_error err = SAP_ERR_NONE;
        LOGV("->%s", __func__);

        if (a_param_cnt != sap_param_count[DISCONNECT_REQ]) {
                return SAP_ERR_ILLEGAL_ARG;
        }

        /*Process the discconection using QMI*/
        if( qmi_sap_disconnect(UIM_SAP_DISCONNECT_GRACEFULL_V01, TRUE) < 0 ) {
                err = SAP_ERR_QMI;
        } else {
                /*emulate the send failure, so that
                there will be graceful disconnection of
                the session*/
                err = SAP_ERR_SND_FAIL;
        }

        LOGV("<-%s", __func__);
        return err;
}


sap_internal_error process_tx_atr_req(unsigned char a_param_cnt)
{
        sap_internal_error err = SAP_ERR_NONE;

        LOGV("->%s", __func__);
        if (a_param_cnt != sap_param_count[TRANSFER_ATR_REQ]) {
                return SAP_ERR_ILLEGAL_ARG;
        }

        /*Process the request*/
        if (qmi_sap_transfer_atr() < 0) {
                err = SAP_ERR_QMI;
        }

        LOGV("<-%s", __func__);
        return err;
}


sap_internal_error process_sim_off_req(unsigned char a_param_cnt)
{
        sap_internal_error err = SAP_ERR_NONE;

        if (a_param_cnt != sap_param_count[POWER_SIM_OFF_REQ]) {
                return SAP_ERR_ILLEGAL_ARG;
        }

        if (qmi_sap_power_off_sim() < 0) {
                err = SAP_ERR_QMI;
        }

        return err;
}

sap_internal_error process_sim_on_req(unsigned char a_param_cnt)
{
        sap_internal_error err = SAP_ERR_NONE;

        if (a_param_cnt != sap_param_count[POWER_SIM_OFF_REQ]) {
                return SAP_ERR_ILLEGAL_ARG;
        }

        if (qmi_sap_power_on_sim() < 0) {
                err = SAP_ERR_QMI;
        }

        return err;
}

sap_internal_error process_sim_reset_req(unsigned char a_param_cnt)
{
        sap_internal_error err = SAP_ERR_NONE;

        if (a_param_cnt != sap_param_count[RESET_SIM_REQ]) {
                return SAP_ERR_ILLEGAL_ARG;
        }

        if (qmi_sap_reset_sim() < 0) {
                err = SAP_ERR_QMI;
        }

        return err;
}


sap_internal_error process_tx_card_stat_req(unsigned char a_param_cnt)
{
        sap_internal_error err = SAP_ERR_NONE;

        if (a_param_cnt != sap_param_count[TRANSFER_CARD_READER_STATUS_REQ]) {
                return SAP_ERR_ILLEGAL_ARG;
        }

        if (qmi_sap_get_card_reader_stat() < 0) {
                err = SAP_ERR_QMI;
        }

        return err;
}

void signal_handler(int sig)
{
       sap_internal_error err = SAP_ERR_NONE;
       err = send_disconn_ind(disconn_sk, IMMEDIATE_DISCONN);

       if (err != SAP_ERR_NONE) {
           LOGI("%s: Error while sending the diconn_ind\n", __func__);
       }

       qmi_sap_disconnect(UIM_SAP_DISCONNECT_IMMEDIATE_V01, FALSE);

       return;
}

sap_internal_error handle_ctrl_request(int a_remote_sk, sap_ipc_ctrl_msg_type ctrl_msg)
{
        sap_internal_error err = SAP_ERR_NONE;

        switch(ctrl_msg) {
                case SAP_CRTL_MSG_DISCONNECT_REQ:
                        LOGV("%s: SAP Control DISCONNECT_REQ\n", __func__);
                        err =  send_disconn_ind(a_remote_sk, GRACEFUL_DISCONN);
                        if (err == SAP_ERR_NONE) {
                                set_sap_state (DISCONNECTING);
                                disconnect_initiated_time = time(&disconnect_initiated_time);
                        }
                        signal(SIGALRM, signal_handler);
                        disconn_sk = a_remote_sk;
                        /*Adding 3 secs to avoid collision with the disconnect request
                        from the server in case it comes */
                        alarm(DISCONNECT_TIMEOUT + 3);
                        break;
                case SAP_CRTL_MSG_DISCONNECT_REQ_IMM:
                        LOGV("%s: SAP Control DISCONNECT_REQ_IMM \n", __func__);
                        err = SAP_ERR_QMI;
                        break;
                default:
                        err = SAP_ERR_ILLEGAL_ARG;
                        LOGV(" Invalid control Request: Req:%d", ctrl_msg);
                        break;
        }
        return err;
}

sap_internal_error handle_sap_request(int a_remote_sk, sap_msg_id a_event, unsigned char a_param_cnt)
{
        int retval = 0;
        LOGV("->%s: sap_state: %d", __func__, sap_state);

        switch(sap_state) {
                case DISCONNECTED:

                switch(a_event) {
                        case CONNECT_REQ:
                                LOGV("%s: CONNECT_REQ\n", __func__);
                                retval = process_conn_req(a_remote_sk, a_param_cnt);
                                break;
                        case DISCONNECT_REQ:
                                if (error_occured) {
                                        LOGV("%s: DISCONNECT_REQ in disconnected state\n", __func__);
                                        /* honor the disconnect request and send valid
                                         * valid response. Client might send the disconnect request
                                         * as a result of previous error response due to invalid
                                         * connection request */
                                        retval = send_disconn_resp(a_remote_sk);
                                        error_occured = FALSE;
                                }
                                break;
                        default:
                        LOGV("SM Error: State: %d :: Event:%d", sap_state, a_event);
                        retval =  SAP_ERR_ILLEGAL_ARG;
                }break;
                case CONNECTED_READY:
                case DISCONNECTING:
                        if (sap_state == DISCONNECTING){
                                /*TODO*/
                                /*fall through If the state is in Diconnecting
                                elapsed time is leass than the expected*/
                                /*if not, initiate the disconnect immediate*/
                                time_t current_time = time(&current_time);
                                if (difftime(current_time, disconnect_initiated_time) >= DISCONNECT_TIMEOUT) {
                                        alarm(0);
                                        retval = send_disconn_ind(a_remote_sk, IMMEDIATE_DISCONN);
                                        if (retval != SAP_ERR_NONE) {
                                                LOGI("%s: Error while sending the diconn_ind", __func__);
                                        }
                                        qmi_sap_disconnect(UIM_SAP_DISCONNECT_IMMEDIATE_V01, FALSE);

                                        /*Mark this as a QMI error
                                        erro case, so that it cleans up*/
                                        retval = SAP_ERR_QMI;

                                }
                        }
                        switch(a_event) {
                        case TRANSFER_APDU_REQ:
                                LOGV("%s: TRANSFER_APDU_REQ\n", __func__);
                                set_sap_state (PROCESSING_APDU);
                                retval = process_tx_apdu_req(a_remote_sk, a_param_cnt);
                                break;
                        case TRANSFER_ATR_REQ:
                                LOGV("%s: TRANSFER_ATR_REQ\n", __func__);
                                set_sap_state (PROCESSING_ATR);
                                retval = process_tx_atr_req(a_param_cnt);
                                break;
                        case POWER_SIM_OFF_REQ:
                                LOGV("%s: POWER_SIM_OFF_REQ\n", __func__);
                                set_sap_state (PROCESSING_SIM_OFF);
                                retval = process_sim_off_req(a_param_cnt);
                                break;
                        case POWER_SIM_ON_REQ:
                                LOGV("%s: POWER_SIM_ON_REQ\n", __func__);
                                set_sap_state (PROCESSING_SIM_ON);
                                retval = process_sim_on_req(a_param_cnt);
                                break;
                        case RESET_SIM_REQ:
                                LOGV("%s: RESET_SIM_REQ\n", __func__);
                                set_sap_state (PROCESSING_SIM_RESET);
                                retval = process_sim_reset_req(a_param_cnt);
                                break;
                        case TRANSFER_CARD_READER_STATUS_REQ:
                                LOGV("%s: TRANSFER_CARD_READER_STATUS_REQ\n", __func__);
                                set_sap_state (PROCESSING_TX_CARD_RDR_STATUS);
                                retval = process_tx_card_stat_req(a_param_cnt);
                                break;
                        case DISCONNECT_REQ:
                                LOGV("%s: DISCONNECT_REQ\n", __func__);
                                retval = process_disconn_req(a_param_cnt);
                                break;
                        default:
                                LOGV("SM Error: State: %d :: Event:%d", sap_state, a_event);
                                retval =  SAP_ERR_ILLEGAL_ARG;

                        } break;
                case PROCESSING_APDU:
                case PROCESSING_ATR:
                case PROCESSING_SIM_ON:
                case PROCESSING_TX_CARD_RDR_STATUS:
                        {
                                switch(a_event){
                                case DISCONNECT_REQ:
                                        LOGV("%s: DISCONNECT_REQ\n", __func__);
                                        retval = process_disconn_req(a_param_cnt);
                                        break;
                                case POWER_SIM_OFF_REQ:
                                        LOGV("%s: POWER_SIM_OFF_REQ\n", __func__);
                                        set_sap_state (PROCESSING_SIM_OFF);
                                        retval = process_sim_off_req(a_param_cnt);
                                        break;
                                case RESET_SIM_REQ:
                                        LOGV("%s: RESET_SIM_REQ\n", __func__);
                                        set_sap_state (PROCESSING_SIM_RESET);
                                        retval = process_sim_reset_req(a_param_cnt);
                                        break;
                                default:
                                        LOGV("SM Error: State: %d :: Event:%d", sap_state, a_event);
                                        retval =  SAP_ERR_ILLEGAL_ARG;
                                }
                        }break;
                case PROCESSING_SIM_OFF:
                case PROCESSING_SIM_RESET:
                        /*It is not expected to get any request
                        when these are processing*/
                default:
                        LOGV("%s: Default case\n", __func__);
                        retval =  SAP_ERR_ILLEGAL_ARG;
                        break;
        }
        if (retval == SAP_ERR_ILLEGAL_ARG) {
                /*Do a dummy socket read to clear all
                the contents*/
                clear_contents(conn_sk);
                /*Send an ERR_RESP packet to client?*/
                retval = send_error_resp(conn_sk);
                if (SAP_ERR_NONE == retval)
                        /* Set the flag to indicate the error_resp*/
                        error_occured = TRUE;
                       /*Don't need to close the connection,
                        Client may send the Disconnect request
                        after it recieves the error response*/
        }

        LOGV("%s returns %d", __func__, retval);
        return retval;
}

int do_read(int s)
{
        int ret = 0, len;
        sap_internal_error retval;
        char buf[SAP_HEADER_SIZE];
        sap_msg_id type;
        unsigned char param_cnt = 0;
        char ipc_hdr[SAP_IPC_HEADER_SIZE];
        sap_ipc_msg_type ipc_msg_id;
        unsigned char sap_ipc_ctrl_msg;
        unsigned short int ipc_msg_len;
        LOGV("Reading on SAP IPC header \n");
        len = recv(s, &ipc_hdr, SAP_IPC_HEADER_SIZE, 0);
        if (len < 0) {
                LOGE("%s: Not able to read from the socket: %s", __func__, strerror(errno));
                return -1;
        }

        if (len != SAP_IPC_HEADER_SIZE) {
            LOGE("%s: No of bytes read by the buffer(%d) is not equal to requested number of bytes %s",
            __func__, len, strerror(errno));
            return 0;
        }

        LOGV("Handling SAP IPC request");
        ipc_msg_id = (sap_ipc_msg_type)ipc_hdr[0];
        ipc_msg_len = *((unsigned short int *)(&ipc_hdr[1]));
        if((ipc_msg_len <= 0) || (ipc_msg_len > SAP_MAX_MSG_LEN)) {
            LOGE("%s: Incorrect length read: %d", __func__, ipc_msg_len);
            return 0;
        }

        if(ipc_msg_id == SAP_IPC_MSG_CTRL_REQUEST) {
                len = recv(s, &buf, SAP_IPC_CTRL_MSG_SIZE, 0);
                if (len < 0) {
                        LOGE("%s: Not able to read from the socket: %s", __func__, strerror(errno));
                        return -1;
                }

                if (len != SAP_IPC_CTRL_MSG_SIZE) {
                        LOGE("%s: No of bytes read by the buffer(%d) is not equal to requested number of bytes %s",
                                __func__, len, strerror(errno));
                        return 0;
                }
                LOGV("Handling SAP control request");
                sap_ipc_ctrl_msg = (sap_msg_id)buf[0];
                retval = handle_ctrl_request(s, sap_ipc_ctrl_msg);
                if (retval == SAP_ERR_SND_FAIL || retval == SAP_ERR_RCV_FAIL ||
                                retval == SAP_ERR_QMI) {
                        /*Mark this as socket falure
                          so that it comes out of the select loop*/
                        ret = -1;
                }
                else {
                        /*Other cases are success*/
                        ret = 0;
                }
        }
        else if(ipc_msg_id == SAP_IPC_MSG_SAP_REQUEST) {
                LOGV("->%s(%x)", __func__, s);
                /*Read you get some data on the Socket*/
                len = recv(s, &buf, SAP_HEADER_SIZE, 0);
                if (len < 0) {
                        LOGE("%s: Not able to read from the socket: %s", __func__, strerror(errno));
                        return -1;
                }

                if (len != SAP_HEADER_SIZE) {
                    LOGE("%s: No of bytes read by the buffer(%d) is not equal to requested number of bytes %s",
                    __func__, len, strerror(errno));
                    return SAP_ERR_RCV_FAIL;
                }

                LOGV("Handling SAP profile request");
                type = (sap_msg_id)buf[0];
                param_cnt = buf[1];
                retval = handle_sap_request(s, type, param_cnt);

                if (retval == SAP_ERR_SND_FAIL || retval == SAP_ERR_RCV_FAIL ||
                                retval == SAP_ERR_QMI) {
                        /*Mark this as socket falure
                          so that it comes out of the select loop*/
                        ret = -1;
                }
                else {
                        /*Other cases are success*/
                        ret = 0;
                }

                LOGV("<-%s returns %d", __func__, ret);
        }
        return ret;
}

int handle_server_side_events(int remote_socket, int pipe_read_end)
{
        int ret =0;
        char buf;
        sap_internal_error err = SAP_ERR_NONE;

        ret = read(pipe_read_end, &buf, 1);
        if ( ret < 0){
                LOGE("Error in reading the pipe");
                return -1;
        }

        if (ret != 1) {
            LOGE("%s: No of bytes read by the buffer(%d) is not equal to requested number of bytes %s",
            __func__, ret, strerror(errno));
            return SAP_ERR_RCV_FAIL;
        }

        if ((int)buf == DISCONNECT_GRACEFULLY) {
                LOGE("Disconnect handling");
                err =  send_disconn_ind(remote_socket, GRACEFUL_DISCONN);
                if (err == SAP_ERR_NONE) {
                        set_sap_state (DISCONNECTING);
                        disconnect_initiated_time = time(&disconnect_initiated_time);
                } else {
                        ret = -1;
                }
        }
        else if ((int)buf == CARD_STATUS_ERROR) {
               err = send_status_ind(remote_socket, CARD_NOT_ACCESSIBLE);
               if (err != SAP_ERR_NONE) {
                      LOGE("%s: Error while sending the card status", __func__);
               }
               /* As we don't get any CARD_INSERTED events, don't need to keep
                  the connection alive.Mark this as an error case, so that there
                  will be immediate disconnection.
               */
               ret = -1;
        }
        else if ((int)buf == CARD_STATUS_REMOVED) {
               err = send_status_ind(remote_socket, CARD_REMOVED);
               if (err != SAP_ERR_NONE) {
                      LOGE("%s: Error while sending the card status", __func__);
               }
               /*Mark this as an error case, as we don't get the card inserted evetns,
               SAP server should initiate immediate disconnection*/
               ret = -1;
        }
        else {
               /*This wouldn't happen*/
               ret = -1;
        }

        return ret;
}

int server(int remote_socket, int pipe_read_end)
{
        fd_set read_mask;
        int nfds, nfd, retval = 0;
        LOGV("->%s(%x) ", __func__, remote_socket);
        sap_internal_error err;
        pthread_mutex_init(&state_mutex, NULL);

        /*This loop need to end
        on disconnection request*/
        while (1)
        {
                FD_ZERO(&read_mask);
                FD_SET(remote_socket, &read_mask);
                FD_SET(pipe_read_end, &read_mask);
                nfds = MAX(remote_socket, pipe_read_end) + 1;
                /*Select on socket for incoming
                requests*/
                LOGV("Selecting on the remote socket\n");
                nfd = select(nfds, &read_mask, NULL, NULL, NULL );
                if(nfd < 0){
                        LOGE("Select: failed: %s", strerror(errno));
                        break;
                }
                if (FD_ISSET(pipe_read_end, &read_mask))        {
                        retval = handle_server_side_events(remote_socket, pipe_read_end);
                        if(retval < 0) {
                                LOGE("%s: Err in handling server side events: ", __func__);
                                break;
                        }
                }
                if (FD_ISSET(remote_socket, &read_mask))        {
                        LOGV("There is some data. read it\n");
                        retval = do_read(remote_socket);
                        if(retval < 0) {
                                LOGE("%s: Error from do_read: ", __func__);
                                break;
                        }
                }
        }

        if (sap_state != DISCONNECTED) {
                if (sap_state != DISCONNECTING) {
                        err = send_disconn_ind(remote_socket, IMMEDIATE_DISCONN);
                        if(err != SAP_ERR_NONE) {
                                //Ignore if this fails
                                LOGI("%s: error while sending the disconn indication", __func__);
                        }
                }
                shutdown(remote_socket, 1);
                close(remote_socket);
                qmi_sap_disconnect(UIM_SAP_DISCONNECT_IMMEDIATE_V01, FALSE);
                set_sap_state (DISCONNECTED);
        }
        pthread_mutex_destroy(&state_mutex);
        return 0;
}

void abort_handler(int sig)
{
        sap_internal_error retval;
        LOGV("->%s :%d", __func__, sig);
        close(local_sk);
        /*Make sure You disconnect and cleanup QMI SAP*/
        if(sap_state != DISCONNECTED) {
                if (sap_state != DISCONNECTING) {
                        retval = send_disconn_ind(conn_sk, IMMEDIATE_DISCONN);
                        if(retval != SAP_ERR_NONE) {
                                LOGE("%s: error while sending the disconn indication", __func__);
                        }
                }
                shutdown(conn_sk, 1);
                close(conn_sk);
                qmi_sap_disconnect(UIM_SAP_DISCONNECT_IMMEDIATE_V01, FALSE);
                set_sap_state (DISCONNECTED);
        }
        LOGV("<-%s", __func__);
}

int disconnect_sap(int fd)
{
        char buf = DISCONNECT_GRACEFULLY;

        LOGV("-> %s", __func__);
        int ret = write(fd, &buf, 1);

        LOGV("<-%s: %d :: errno:%s", __func__, ret, strerror(errno));
        return ret;
}

int send_card_status(int card_status)
{
        char buf = card_status;
        int ret;

        LOGV("-> %s", __func__);
        ret = write(server_evt_pipe[1], &buf, 1);

        LOGV("<-%s: %d :: errno:%s", __func__, ret, strerror(errno));
        return ret;
}

int create_server_socket(const char* name)
{
    int sock = socket(AF_LOCAL, SOCK_STREAM, 0);
    if(sock < 0)
        return -1;

    LOGV("create_server_socket %s", name);

    if(socket_local_server_bind(sock, name, ANDROID_SOCKET_NAMESPACE_ABSTRACT) < 0)
    {
        LOGE("socket failed to create (%s)", strerror(errno));
        return -1;
    }

    if(listen(sock, 1) < 0)
    {
        LOGE("listen failed (%s)", strerror(errno));
        close(sock);
        return -1;
    }

    LOGV("created socket fd %d", sock);
    return sock;
}


/*===========================================================================

FUNCTION     : main
DEPENDENCIES : None

RETURN VALUE : None

============================================================================*/
int main(int argc, char *argv[])
{
        int pid, cid, status;
        struct sockaddr_rc ra;
        int length, retval = -1;
        char addr[BTADDR_SIZE];
        struct sigaction actions;
        memset(&actions, 0, sizeof(actions));
        sigemptyset(&actions.sa_mask);
        actions.sa_flags = 0;
        actions.sa_handler = abort_handler;

        if (argc != 2) {
                LOGE("Invalid arguements: Usage:  sapd <rfcomm_channel_no>");
                return -1;
        }

        /*Ignore the signals from child to
        prevent becoming zombie*/
        signal(SIGCHLD, SIG_IGN);
        /*Ignore the SIGPIPE signal, this signal will
          emited incase there is an error writing to
          SAP BT App socket */
        signal(SIGPIPE, SIG_IGN);
        if (sigaction(SIGTERM,&actions,NULL) < 0) {
                LOGE("Error in sigaction in %s:  %s\n", __func__, strerror(errno));
                return -1;
        }
        /*Create the pipe between parent and child
        with which all the server side events like
        disconnect,card status event can be communicated
        to child session*/
        if (pipe (server_evt_pipe))
        {
                LOGE ("Pipe Creation failed :%s\n", strerror(errno));
                return -1;
        }

        LOGV("Event pipe created\n");

        while (1) {
                length = sizeof(struct sockaddr_un);
                local_sk = create_server_socket (SAP_SERVER);
                if (local_sk == -1)
                {
                    LOGE("Fatal Error : Could not create Socket for BT SAP:\n");
                    break;
                }

                /*This is the parent daemon, which would keep listening
                for the incoming connections from SAP BT App layer */
                /*Once accepted, It forks a child which takes care of SAP request
                processing, Once the SAP disconnects, parent can accept
                further connections
                */
                conn_sk = accept(local_sk, (struct sockaddr *)&ra, &length);

                LOGV("Accepted incoming connection from SAP BT App");
                if (conn_sk < 0) {
                        close(local_sk);
                        if (errno == EBADF) {
                                LOGE("Invalid local Socket descriptor: %d err: %s\n", local_sk, strerror(errno));
                                /*This signifys that exe is aborted and should be exited*/
                                break;
                        } else {
                                /*remote socket failure case
                                skip the further part, so that SAP server
                                will be back in its wait loop*/
                                LOGE("Invalid local Socket descriptor: %d err: %s\n", local_sk, strerror(errno));
                                continue;
                        }
                }

                /*Process the request in a new child*/
                switch (cid = fork()){
                        case 0:
                                /*In child*/
                                LOGV("In Child process\n");
                                pid = getpid();
                                /*child doesn't need local socket any more*/
                                close(local_sk);
                                /*get SIGHUP on parent's death*/
                                prctl(PR_SET_PDEATHSIG, SIGHUP, 0, 0, 0);
                                /*server will handle the necessary
                                 */
                                LOGV("Starting the server in Child\n");
                                retval = server(conn_sk, server_evt_pipe[0]);
                                /*Exit from child*/
                                LOGV("Exiting from the child\n");
                                exit(0);
                                break;
                        case -1:
                                LOGE("%s: Can't fork: %s\n", __func__, strerror(errno));
                        default:
                                /*Don't need to have ref to
                                remote socket in parent*/
                                LOGV("Closing the remote socket in Parent\n");
                                close(conn_sk);
                                /*Don't need to accept futher connections?*/
                                close(local_sk);

                                LOGV("Waiting for the child to finish....\n");
                                waitpid(cid, &status, 0);
                                LOGV("Parent: Status of the child %d is %d\n", cid, status);
                                break;
                                /*Parent will go back to his main listening loop
                                for further incoming connections*/
                }
        }
        return 0;
}
