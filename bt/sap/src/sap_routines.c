/******************************************************************************NS

  @file    sap_routines.c
  @brief

  DESCRIPTION

******************************************************************************/
/*===========================================================================

  Copyright (c) 2011 Qualcomm Technologies, Inc. All Rights Reserved

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
#include <sys/stat.h>
#include <sys/socket.h>
#include <utils/Log.h>
#include <stdlib.h>
#include <errno.h>
#include <arpa/inet.h>

#include "sap_types.h"
#undef LOGD
#undef LOGE
#undef LOGI
#undef LOGV

#define LOGD ALOGD
#define LOGI ALOGI
#define LOGE ALOGE
#define LOGV ALOGV

/*
This list denotes the expected number of params for the different
SAP requests. These constants are used to validate the SAP requests.
If the count is -1, It indicates that the number of parameters are
variable and can't be validated in these cases.
TODO: have a proper structure for mapping table.
*/
const int sap_param_count[] =
{
        1, /*CONNECT_REQ*/
        2, /*CONNECT_RESP*/
        0,/*DISCONNECT_REQ*/
        0, /*DISCONNECT_RESP*/
        1, /*DISCONNECT_IND*/
        1, /*TRANSFER_APDU_REQ*/        /*It can have either comandAPDU or comdAPDU7816*/
        -1,/*TRANSFER_APDU_RESP*/
        0,/*TRANSFER_ATR_REQ*/
        -1, /*TRANSFER_ATR_RESP*/
        0, /*POWER_SIM_OFF_REQ*/
        1, /*POWER_SIM_OFF_RESP*/
        0, /*POWER_SIM_ON_REQ*/
        1, /*POWER_SIM_ON_RESP*/
        0, /*RESET_SIM_REQ*/
        1, /*RESET_SIM_RESP*/
        0, /*TRANSFER_CARD_READER_STATUS_REQ*/
        -1, /*TRANSFER_CARD_READER_STATUS_RESP*/
        1, /*STATUS_IND*/
        0, /*ERROR_RESP*/
};

/*
This list denotes the expected  length for the different parameters
used in SAP requests. These constants are used to validate the SAP parameters.
If the length is -1, It indicates that the length of parameter is
variable and can't be validated in these cases.
TODO: have a proper structure for mapping table.
*/
const int sap_param_length[] =
{
        2, /*Max Msg size*/
        1, /*Connection Stat*/
        1,  /*Result code*/
        1, /*Disconnection type*/
        -1, /*commandapdu*/
        -1, /*ResponseApdu*/
        -1, /*ATR*/
        1, /*Card Reader Stat*/
        1, /*Stat change*/
        1, /*Transport protocol*/
        -1, /*Invalid Entry*/
        -1, /*Invalid Entry*/
        -1, /*Invalid Entry*/
        -1, /*Invalid Entry*/
        -1, /*Invalid Entry*/
        -1, /*Invalid Entry*/
        -1, /*commandapdu7816*/
};

/*
 * form_sap_param
 */
void form_sap_param(sap_param* a_param, sap_param_id a_id, unsigned short a_length, unsigned char *a_value)
{
        if(a_value)
        {
            LOGV("->%s address: %x, id:%d, len:%d, value:%d ", __func__, (intptr_t)a_param, a_id, a_length, (*(intptr_t*)a_value));
            a_param->id = a_id;
            a_param->length = htons(a_length);

            LOGV("Copying the values\n");
            memcpy(a_param->value, a_value, a_length);
        }
        else
        {
            LOGV("->%s address: %x, id:%d, len:%d ", __func__, (intptr_t)a_param, a_id, a_length);
        }

        LOGV("<-%s returning %d", __func__, 0);
}

void form_sap_ipc_msg(sap_ipc_msg *ipc_msg, sap_ipc_msg_type ipc_msg_type, sap_msg_id a_id,
                                                unsigned short a_count, unsigned int len)
{
        unsigned int i =0;
        unsigned char *dst;
        LOGV("->%s", __func__);
        ipc_msg->msg_type = ipc_msg_type;
        /* actual message length will be total size - SAP IPC header size */
        ipc_msg->msg_len = len - SAP_IPC_HEADER_SIZE;

        ipc_msg->msg.id = a_id;
        ipc_msg->msg.param_count = a_count;

        dst = (unsigned char*)&ipc_msg->msg;
        LOGV("->SAP PACKET");
        for(i=0; i<ipc_msg->msg_len; i++)
                LOGV("%x ", dst[i]);
        LOGV("<-SAP_PACKET");

        LOGV("<-%s", __func__);
}

void form_sap_ipc_ctrl_msg(sap_ipc_msg *ipc_msg, sap_ipc_msg_type ipc_msg_type,
                              sap_ipc_ctrl_msg_type ctrl_msg, unsigned int len)
{
        LOGV("->%s", __func__);
        ipc_msg->msg_type = ipc_msg_type;
        ipc_msg->msg_len = len - SAP_IPC_HEADER_SIZE;
        ipc_msg->ctrl_msg = ctrl_msg;

        LOGV("<-%s", __func__);
}

sap_ipc_msg* alloc_sap_ipc_msg(const int a_count, int *size_array, int *len)
{
        int total_size = SAP_HEADER_SIZE + SAP_IPC_HEADER_SIZE;
        int i;
        sap_ipc_msg *ipc_msg =  NULL;

        LOGV("->%s \n", __func__);
        for (i = 0; i<a_count; i++)
        {
                total_size += SAP_PARAM_HEADER_SIZE + size_array[i] + FOUR_BYTE_PADDING(size_array[i]);
                LOGV("padding for param[%d]: %d\n", i, FOUR_BYTE_PADDING(size_array[i]));
        }
        ipc_msg = (sap_ipc_msg*)malloc(total_size);
        LOGV("%s: Total size allocated: %d\n", __func__, total_size);

        if(ipc_msg != NULL) {
                /*Initialize the memory*/
                memset(ipc_msg, 0x00, total_size);
                *len = total_size;
        } else {
                LOGE("Memory overflow\n");
                len =0;
        }

        LOGV("<-%s returns %x",__func__, ipc_msg);
        return ipc_msg;
}

void notify_connection_status(int a_socket, sap_state_type sap_state)
{
        sap_ipc_msg ipc_msg;
        int ret = 0;
        unsigned short int len = SAP_IPC_HEADER_SIZE + SAP_IPC_CTRL_MSG_SIZE;
        if(sap_state == DISCONNECTED) {
                form_sap_ipc_ctrl_msg(&ipc_msg, SAP_IPC_MSG_CTRL_RESPONSE, SAP_CRTL_MSG_DISCONNECTED_RESP , len);
        }
        else if(sap_state == CONNECTED_READY) {
                form_sap_ipc_ctrl_msg(&ipc_msg, SAP_IPC_MSG_CTRL_RESPONSE, SAP_CRTL_MSG_CONNECTED_RESP, len);
        }
        else
            return;

        ret = send (a_socket, &ipc_msg, len, 0);
        if (ret < 0) {
                LOGE("Not able to send mesg to remote dev: %s", strerror(errno));
        }
        LOGV("<-%s returns %d\n", __func__, ret);
}

sap_internal_error send_status_ind(int a_socket, sap_status_change a_stat)
{
        int size_array[1] = {sap_param_length[STATUS_CHANGE]};
        unsigned char value[1];
        sap_ipc_msg* ipc_msg;
        sap_param *param;
        int len, ret;

        LOGV("->%s", __func__);
        value[0] = (unsigned char)a_stat;

        ipc_msg = alloc_sap_ipc_msg(sap_param_count[STATUS_IND], size_array, &len);
        LOGV("msg: %x", ipc_msg);
        if(ipc_msg == NULL) {
                return SAP_ERR_MEM_LOW;
        }
        /*Fill all the the params */
        param = (sap_param*)ipc_msg->msg.params;
        LOGV("%x", param);
        form_sap_param(param, STATUS_CHANGE, size_array[0], &value[0]);

        form_sap_ipc_msg(ipc_msg, SAP_IPC_MSG_SAP_RESPONSE, STATUS_IND, sap_param_count[STATUS_IND], len);

        ret = send(a_socket, ipc_msg, len, 0);
        if (ret < 0) {
                LOGE("Not able to send mesg to remote dev: %s", strerror(errno));
                free(ipc_msg);
                return SAP_ERR_SND_FAIL;
        }

        if(a_stat == CARD_RESET) {
                sim_card_state = SIM_CARD_POWERED_ON;
                /* notify SAP BT App that SAP profile connection established */
                notify_connection_status(a_socket,CONNECTED_READY);
        }
        else if(a_stat == CARD_REMOVED) {
                sim_card_state = SIM_CARD_REMOVED;
        }
        else if(a_stat == CARD_NOT_ACCESSIBLE) {
                sim_card_state = SIM_CARD_NOT_ACCESSIBLE;
        }

        LOGV("<-%s returns %d\n", __func__, ret);
        set_sap_state (CONNECTED_READY);
        free(ipc_msg);
        return SAP_ERR_NONE;
}

sap_internal_error send_conn_resp(int a_socket, sap_connection_status a_stat, unsigned short a_max_msg_sz)
{
        int size_array[2] = {sap_param_length[CONNECTION_STATUS], sap_param_length[MAX_MSG_SIZE] };
        sap_ipc_msg* ipc_msg;
        sap_param *param;
        int len, i = 0, ret;
        unsigned short n_val;
        int param_count;
        unsigned char value[3], *p;

        LOGV("->%s", __func__);
        value[0] = (unsigned char)a_stat;

        param_count = (a_stat == CONN_MAX_SIZE_NOT_SUPPORTED) ? 2 : 1;

        ipc_msg = alloc_sap_ipc_msg(param_count, size_array, &len);
        if(ipc_msg == NULL) {
                return SAP_ERR_MEM_LOW;
        }

        param = (sap_param*)ipc_msg->msg.params;
        p = ipc_msg->msg.params;
        form_sap_param(param, CONNECTION_STATUS, size_array[0], &value[0]);

        if (a_stat == CONN_MAX_SIZE_NOT_SUPPORTED) {
                /*Form the supported size in the network byte order*/
                n_val = htons(*((unsigned short*)&a_max_msg_sz));
                memcpy(&value[1], (unsigned char*)&n_val, sap_param_length[MAX_MSG_SIZE]);

                p += SAP_PARAM_HEADER_SIZE + size_array[0] +  FOUR_BYTE_PADDING(size_array[0]);
                param = (sap_param*)p;
                form_sap_param(param, MAX_MSG_SIZE, size_array[1], &value[1]);
        }

        form_sap_ipc_msg(ipc_msg, SAP_IPC_MSG_SAP_RESPONSE, CONNECT_RESP, param_count, len);

        ret = send (a_socket, ipc_msg, len, 0);

        if (ret < 0) {
                LOGE("Not able to send mesg to remote dev: %s", strerror(errno));
                free(ipc_msg);
                return SAP_ERR_SND_FAIL;
        }
        LOGV("<-%s returns %d\n", __func__, ret);
        free(ipc_msg);

        return SAP_ERR_NONE;
}

sap_internal_error send_disconn_resp(int a_socket )
{
        sap_ipc_msg ipc_msg;
        int ret = 0;
        unsigned short int len = SAP_HEADER_SIZE + SAP_IPC_HEADER_SIZE;
        form_sap_ipc_msg(&ipc_msg, SAP_IPC_MSG_SAP_RESPONSE, DISCONNECT_RESP, 0, len);
        ret = send (a_socket, &ipc_msg, len, 0);
        if (ret < 0) {
                LOGE("Not able to send mesg to remote dev: %s", strerror(errno));
                return SAP_ERR_SND_FAIL;
        }
        LOGV("<-%s returns %d\n", __func__, ret);
        set_sap_state (DISCONNECTED);
        /* notify SAP BT App that SAP profile connection released */
        notify_connection_status(a_socket, DISCONNECTED);

        return SAP_ERR_NONE;
}

sap_internal_error send_disconn_ind(int a_socket, sap_disconnect_type a_type)
{
        int size_array[1] = {sap_param_length[DISCONNECTION_TYPE]};
        unsigned char value[1];
        sap_ipc_msg* ipc_msg;
        sap_param *param;
        int len, ret = 0;
        value[0] = (unsigned char)a_type;

        LOGV("->%s", __func__);
        ipc_msg = alloc_sap_ipc_msg(sap_param_count[DISCONNECT_IND], size_array, &len);
        LOGV("ipc_msg: %x", ipc_msg);
        if(ipc_msg == NULL){
                return SAP_ERR_MEM_LOW;
        }

        /*Fill all the params*/
        param = (sap_param*)ipc_msg->msg.params;
        LOGV("%x", param);
        form_sap_param(param, DISCONNECTION_TYPE, size_array[0], &value[0]);

        form_sap_ipc_msg(ipc_msg, SAP_IPC_MSG_SAP_RESPONSE, DISCONNECT_IND, sap_param_count[DISCONNECT_IND], len);

        ret = send(a_socket, ipc_msg, len, 0);
        if (ret < 0) {
                LOGE("Not able to send mesg to remote dev: %s", strerror(errno));
                free(ipc_msg);
                return SAP_ERR_SND_FAIL;
        }

        LOGV("<-%s returns %d\n", __func__, ret);

        if(a_type == IMMEDIATE_DISCONN) {
                set_sap_state (DISCONNECTED);
                /* notify SAP BT App that SAP profile connection released */
                notify_connection_status(a_socket, DISCONNECTED);
        }

        free(ipc_msg);
        return SAP_ERR_NONE;
}

sap_internal_error send_tx_apdu_resp(int a_socket, sap_result_code a_result, unsigned char* a_resp_apdu, int a_apdu_len)
{
        int size_array[2], len;
        unsigned char value[1], *p;
        sap_ipc_msg *ipc_msg = NULL;
        sap_param* param;
        int param_count, ret = 0;

        size_array[0] = sap_param_length[RESULT_CODE];
        size_array[1] = a_apdu_len;

        param_count = (a_result == ERR_NONE) ? 2 : 1;

        ipc_msg = alloc_sap_ipc_msg(param_count, size_array, &len);
        if(ipc_msg == NULL) {
                return SAP_ERR_MEM_LOW;
        }

        param = (sap_param*)ipc_msg->msg.params;
        p = (unsigned char*)ipc_msg->msg.params;
        form_sap_param(param, RESULT_CODE, size_array[0], (unsigned char*)&a_result);

        if (a_result == ERR_NONE) {
                p += SAP_PARAM_HEADER_SIZE + size_array[0] +  FOUR_BYTE_PADDING(size_array[0]);
                param = (sap_param*)p;
                form_sap_param(param, RESPONSE_APDU, size_array[1], a_resp_apdu);
        }

        form_sap_ipc_msg(ipc_msg, SAP_IPC_MSG_SAP_RESPONSE, TRANSFER_APDU_RESP, param_count, len);

        ret = send(a_socket, ipc_msg, len, 0);
        if (ret < 0) {
                LOGE("Not able to send mesg to remote dev: %s", strerror(errno));
                free(ipc_msg);
                return SAP_ERR_SND_FAIL;
        }

        set_sap_state (CONNECTED_READY);
        LOGV("<-%s returns %d\n", __func__, ret);
        free(ipc_msg);
        return SAP_ERR_NONE;
}


sap_internal_error send_tx_atr_resp(int a_socket, sap_result_code a_result, unsigned char* a_resp_atr, int a_atr_len)
{
        int size_array[2], len;
        unsigned char value[1], *p;
        sap_ipc_msg *ipc_msg = NULL;
        sap_param* param;
        int param_count, ret = 0;

        size_array[0] = sap_param_length[RESULT_CODE];
        size_array[1] = a_atr_len;

        param_count = (a_result == ERR_NONE) ? 2 : 1;
        ipc_msg = alloc_sap_ipc_msg(param_count, size_array, &len);
        if(ipc_msg == NULL) {
                return SAP_ERR_MEM_LOW;
        }

        param = (sap_param*)ipc_msg->msg.params;
        p = ipc_msg->msg.params;
        form_sap_param(param, RESULT_CODE, size_array[0], (unsigned char*)&a_result);

        if (a_atr_len != 0) {
                p += SAP_PARAM_HEADER_SIZE + size_array[0] +  FOUR_BYTE_PADDING(size_array[0]);
                param = (sap_param*)p;
                LOGV("ATR: param address: %x", param);
                form_sap_param(param, ATR, size_array[1], a_resp_atr);
        }

        form_sap_ipc_msg(ipc_msg, SAP_IPC_MSG_SAP_RESPONSE, TRANSFER_ATR_RESP, param_count, len);

        ret = send(a_socket, ipc_msg, len, 0);

        if (ret < 0) {
                LOGE("Not able to send mesg to remote dev: %s", strerror(errno));
                free(ipc_msg);
                return SAP_ERR_SND_FAIL;
        }

        set_sap_state (CONNECTED_READY);
        LOGV("<-%s returns %d\n", __func__, ret);
        free(ipc_msg);
        return SAP_ERR_NONE;
}

sap_internal_error send_sim_off_resp(int a_socket, sap_result_code a_result)
{
        int size_array[1] = {sap_param_length[RESULT_CODE]};
        unsigned char value[1];
        sap_ipc_msg* ipc_msg;
        sap_param *param;
        int len, ret = 0;
        value[0] = (unsigned char)a_result;

        LOGV("->%s", __func__);
        ipc_msg = alloc_sap_ipc_msg(sap_param_count[POWER_SIM_OFF_RESP], size_array, &len);
        LOGV("ipc_msg: %x", ipc_msg);
        if(ipc_msg == NULL) {
                return SAP_ERR_MEM_LOW;
        }
        /*Fill all the the params*/
        param = (sap_param*)ipc_msg->msg.params;
        LOGV("%x", param);
        form_sap_param(param, RESULT_CODE, size_array[0], &value[0]);

        form_sap_ipc_msg(ipc_msg, SAP_IPC_MSG_SAP_RESPONSE, POWER_SIM_OFF_RESP, sap_param_count[POWER_SIM_OFF_RESP], len);

        ret = send(a_socket, ipc_msg, len, 0);
        if (ret < 0) {
                LOGE("Not able to send mesg to remote dev: %s", strerror(errno));
                free(ipc_msg);
                return SAP_ERR_SND_FAIL;
        }
        set_sap_state (CONNECTED_READY);
        LOGV("<-%s returns %d\n", __func__, ret);
        free(ipc_msg);
        return SAP_ERR_NONE;
}

sap_internal_error send_sim_on_resp(int a_socket, sap_result_code a_result)
{
        int size_array[1] = {sap_param_length[RESULT_CODE]};
        unsigned char value[1];
        sap_ipc_msg* ipc_msg;
        sap_param *param;
        int len, ret =0;
        value[0] = (unsigned char)a_result;

        LOGV("->%s", __func__);
        ipc_msg = alloc_sap_ipc_msg(sap_param_count[POWER_SIM_ON_RESP], size_array, &len);
        LOGV("ipc_msg: %x", ipc_msg);
        if (ipc_msg == NULL) {
                return SAP_ERR_MEM_LOW;
        }
        /*Fill all the the params*/
        param = (sap_param*)ipc_msg->msg.params;
        LOGV("%x", param);
        form_sap_param(param, RESULT_CODE, size_array[0], &value[0]);

        form_sap_ipc_msg(ipc_msg, SAP_IPC_MSG_SAP_RESPONSE, POWER_SIM_ON_RESP, sap_param_count[POWER_SIM_ON_RESP], len);

        ret = send(a_socket, ipc_msg, len, 0);
        if (ret < 0) {
                LOGE("Not able to send mesg to remote dev: %s", strerror(errno));
                free(ipc_msg);
                return SAP_ERR_SND_FAIL;
        }
        set_sap_state (CONNECTED_READY);
        LOGV("<-%s returns %d\n", __func__, ret);
        free(ipc_msg);
        return SAP_ERR_NONE;
}

sap_internal_error send_sim_reset_resp(int a_socket, sap_result_code a_result)
{
        int size_array[1] = {sap_param_length[RESULT_CODE]};
        unsigned char value[1];
        sap_ipc_msg* ipc_msg;
        sap_param *param;
        int len, ret = 0;
        value[0] = (unsigned char)a_result;

        LOGV("->%s", __func__);
        ipc_msg = alloc_sap_ipc_msg(sap_param_count[RESET_SIM_RESP], size_array, &len);
        if(ipc_msg == NULL) {
                return SAP_ERR_MEM_LOW;
        }
        LOGV("ipc_msg: %x", ipc_msg);
        /*Fill all the the params*/
        param = (sap_param*)ipc_msg->msg.params;
        LOGV("%x", param);
        form_sap_param(param, RESULT_CODE, size_array[0], &value[0]);

        form_sap_ipc_msg(ipc_msg, SAP_IPC_MSG_SAP_RESPONSE, RESET_SIM_RESP, sap_param_count[RESET_SIM_RESP], len);

        ret = send(a_socket, ipc_msg, len, 0);
        if (ret < 0) {
                LOGE("Not able to send mesg to remote dev: %s", strerror(errno));
                free(ipc_msg);
                return SAP_ERR_SND_FAIL;
        }
        set_sap_state (CONNECTED_READY);
        LOGV("<-%s returns %d\n", __func__, ret);
        free(ipc_msg);
        return SAP_ERR_NONE;
}

sap_internal_error send_tx_card_stat(int a_socket, sap_result_code a_result, unsigned char* a_status, int a_len)
{
        int size_array[2], len;
        unsigned char value[2], *p;
        sap_ipc_msg *ipc_msg = NULL;
        sap_param* param;
        int param_count, ret = 0;

        size_array[0] = sap_param_length[RESULT_CODE];
        size_array[1] = a_len;

        value[0] = a_result;

        param_count = (a_result == ERR_NONE) ? 2 : 1;
        ipc_msg = alloc_sap_ipc_msg(param_count, size_array, &len);
        if (ipc_msg == NULL) {
                return SAP_ERR_MEM_LOW;
        }

        param = (sap_param*)ipc_msg->msg.params;
        p = (unsigned char*)ipc_msg->msg.params;
        form_sap_param(param, RESULT_CODE, size_array[0], &value[0]);

        if (a_result == ERR_NONE) {
                p += SAP_PARAM_HEADER_SIZE + size_array[0] +  FOUR_BYTE_PADDING(size_array[0]);
                param = (sap_param*)p;
                form_sap_param(param, CARD_READER_STATUS, a_len, a_status);
        }

        form_sap_ipc_msg(ipc_msg, SAP_IPC_MSG_SAP_RESPONSE, TRANSFER_CARD_READER_STATUS_RESP, param_count, len);
        ret = send(a_socket, ipc_msg, len, 0);

        if (ret < 0) {
                LOGE("Not able to send mesg to remote dev: %s", strerror(errno));
                free(ipc_msg);
                return SAP_ERR_SND_FAIL;
        }
        set_sap_state (CONNECTED_READY);
        LOGV("<-%s returns %d\n", __func__, ret);
        free(ipc_msg);
        return SAP_ERR_NONE;
}

sap_internal_error send_error_resp(int a_socket ){
        int ret = 0;
        sap_ipc_msg ipc_msg;
        unsigned short int len = SAP_HEADER_SIZE + SAP_IPC_HEADER_SIZE;
        form_sap_ipc_msg(&ipc_msg, SAP_IPC_MSG_SAP_RESPONSE, ERROR_RESP, 0, len);
        ret = send (a_socket, &ipc_msg, len, 0);

        if (ret < 0) {
               LOGE("Not able to send mesg to remote dev: %s", strerror(errno));
               /*Should take care of informing to parent*/
               return SAP_ERR_SND_FAIL;
        }
        LOGV("<-%s returns %d\n", __func__, ret);
        return SAP_ERR_NONE;
}
/*EOF*/
