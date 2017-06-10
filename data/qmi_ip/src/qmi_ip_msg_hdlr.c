/******************************************************************************

                        QMI_IP_MSG_HDLR.C

******************************************************************************/

/******************************************************************************

  @file    qmi_ip_msg_hdlr.c
  @brief   Qualcomm mapping interface over IP Message handler

  DESCRIPTION


  ---------------------------------------------------------------------------
  Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------


******************************************************************************/

/******************************************************************************

                      EDIT HISTORY FOR FILE

  $Id:$

when       who        what, where, why
--------   ---        -------------------------------------------------------
08/30/13   tw         Initial version

******************************************************************************/

/*===========================================================================
                              INCLUDE FILES
===========================================================================*/

#include "qmi_ip.h"

#define QMI_PLATFORM_NUM_CHARS_PER_BYTE (3) /* 2 hex chars per byte + space */
#define QMI_PLATFORM_NUM_BYTES_PER_LINE (16)

#define QMI_PLATFORM_GET_HEX_CHAR(x)  \
  (((x) > 0x0F) ? '*' : hex_digit_to_char_tbl[(x)])

static const char hex_digit_to_char_tbl[] =
{
  '0', '1', '2', '3', '4', '5', '6', '7',
  '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
};

/*===========================================================================
                              FUNCTION DEFINITIONS
===========================================================================*/

int proccess_nas_message(unsigned char**, unsigned long, int, unsigned long tx_id);
void disable_embms(void);

/*===========================================================================
  FUNCTION  DISABLE_EMBMS
===========================================================================*/
/*!
@brief

  - Disable the eMBMS connection

@return

@note

  - Dependencies
    - SSL link to GW active

  - Side Effects
    - None
*/
/*=========================================================================*/
void disable_embms(void){
  unsigned char msg_ptr[5];
  unsigned char *tmp_msg_ptr;
  unsigned char *tlv_msg_ptr;
  unsigned char *tlv_tmp_msg_ptr;
  int ret, tlv_size;
  int msg_ptr_len;

  memset((char*)msg_ptr,0,5);
  tmp_msg_ptr = msg_ptr;

  /* QMI_NAS_CONFIG_EMBMS */
  tlv_size = 2;
  tlv_msg_ptr = malloc(tlv_size);
  if (tlv_msg_ptr == NULL){
      LOG_MSG_ERROR("MALLOC failure",0,0,0);
      return;
  }
  tlv_tmp_msg_ptr = tlv_msg_ptr;
  WRITE_8_BIT_VAL(tlv_tmp_msg_ptr, 0);
  ret = qmi_util_write_std_tlv(&tmp_msg_ptr, &msg_ptr_len, 0x01, tlv_size,
                               (void *)tlv_msg_ptr);

  ret = proccess_nas_message((unsigned char** )&msg_ptr, QMI_NAS_CONFIG_EMBMS,
                             msg_ptr_len, QMI_IND_TX_ID);
  if (ret == -1) {
      LOG_MSG_ERROR("Processing of message failed", 0,0,0);
  }

  free(tlv_msg_ptr);
}

qmi_error_type_v01 pasword_reset(void);
/*==========================================================================

FUNCTION FACTORY_RESET()

DESCRIPTION

 Resets the ODU password to the default password.

DEPENDENCIES
  ODU password file exists.

RETURN VALUE
  None.


SIDE EFFECTS
  None

==========================================================================*/
qmi_error_type_v01 pasword_reset(void){
  FILE *fp;

  if((fp = fopen(PASSWORD_FILE, "r")) == NULL){
      LOG_MSG_ERROR("File does not exist",0,0,0);
      return QMI_ERR_NO_EFFECT_V01;
  }
  fclose(fp);

  if (remove(PASSWORD_FILE) != 0) {
      LOG_MSG_ERROR("Unable to delete file",0,0,0);
      return QMI_ERR_INTERNAL_V01;
  }
  else
      LOG_MSG_ERROR("File deleted successfully",0,0,0);

  return QMI_NO_ERR;
}

/*===========================================================================
  FUNCTION  PRINT_QMI_MSG
===========================================================================*/
/*!
@brief

  - Print the QMI message in hex

@return

@note

*/
/*=========================================================================*/
void print_qmi_msg (
  unsigned char           *msg,
  int                     msg_len
)
{
  int i, j;
  char buff[QMI_PLATFORM_NUM_CHARS_PER_BYTE*QMI_PLATFORM_NUM_BYTES_PER_LINE+1] = "";
  unsigned char upper_half;
  unsigned char lower_half;
  const unsigned char *data = msg;

  if (NULL == msg)
  {
    return;
  }

  for (i = 1, j = 0; i <= msg_len; ++i, ++data)
  {
    upper_half = (*data) >> 4;
    lower_half = (*data) & 0x0F;
    buff[j++]  = QMI_PLATFORM_GET_HEX_CHAR(upper_half);
    buff[j++]  = QMI_PLATFORM_GET_HEX_CHAR(lower_half);
    buff[j++]  = ' ';

    if (i % QMI_PLATFORM_NUM_BYTES_PER_LINE == 0)
    {
      buff[j] = '\0';
      LOG_MSG_ERROR ("%s\n", buff,0,0);
      j = 0;
    }
  }

  /* Print any remaining data */
  if (j > 0)
  {
    buff[j] = '\0';
    LOG_MSG_ERROR ("%s\n", buff,0,0);
  }
}

/*===========================================================================
  FUNCTION  PROCESS_WDS_MESSAGE
===========================================================================*/
/*!
@brief

  Proccess the wds messages from the GW.

@return

  - Calls corresponding dsi_netctrl function based off wds message

@note

  - Dependencies
    - SSL link to GW active

  - Side Effects
    - None
*/
/*=========================================================================*/

int proccess_wds_message(unsigned char **msg,
                         unsigned long message_id,
                         int msg_len,
                         unsigned long tx_id)
{
    int             ret;
    unsigned long   type;
    unsigned char   *value_ptr = NULL;
    unsigned char   temp_char;
    unsigned long   value_len;
    int             error_code = QMI_ERR_NONE_V01;
    int             tmp_msg_len;

    unsigned char *msg_ptr;
    unsigned char *tmp_msg_ptr;
    int tlv_len;
    unsigned char *tlv_msg_ptr;
    unsigned char *tlv_tmp_msg_ptr;
    int msg_ptr_len, resp_len;
    unsigned char *resp_msg_ptr;

    if(message_id == QMI_WDS_START_NETWORK_INTERFACE) {
        //if the network tech pref is anything but eMBMS do not forward it to modem
        tmp_msg_ptr = *msg;
        tmp_msg_len = msg_len;

        while (tmp_msg_len >0)
        {
            if (( ret = qmi_util_read_std_tlv(&tmp_msg_ptr, &tmp_msg_len, &type, &value_len,
                                              &value_ptr)) ==  QMI_INTERNAL_ERR){
                error_code = QMI_ERR_MISSING_ARG_V01;
                break;
            }

            if(type == QMI_WDS_NW_IF_XTENDED_TECH_PREF_REQ_TLV_ID){
                error_code = QMI_ERR_NONE_V01;
                break;
            }
            else
                error_code = QMI_ERR_INVALID_TECH_PREF_V01;
        }
    }
    else if ((message_id == QMI_WDS_EMBMS_TMGI_ACTIVATE || message_id == QMI_WDS_EMBMS_TMGI_ACT_DEACT)
             && embms_call_state && !netmgr_status) {
        error_code = QMI_ERR_INTERNAL_V01;
    }

    if (error_code == QMI_ERR_NONE_V01) {
        msg_ptr = *msg;
        ret =  qmi_client_send_raw_msg_sync (qmi_ip_conf.qmi_ip_v4_wds_handle, message_id,
                                            (unsigned char *)((unsigned char *)msg_ptr),
                                             msg_len, msg_ptr, QMI_MAX_STD_MSG_SIZE,
                                             &resp_len, QMI_TIMEOUT);
        if (ret == QMI_NO_ERR){
            msg_ptr_len = QMI_HEADER_SIZE + resp_len;
            resp_msg_ptr = malloc(msg_ptr_len);
            if (resp_msg_ptr == NULL){
                LOG_MSG_ERROR("MALLOC failure",0,0,0);
                return QMI_IP_ERROR;
            }
            tmp_msg_ptr = resp_msg_ptr;

            WRITE_QMI_HEADER(tmp_msg_ptr, msg_ptr_len, QMI_WDS_SERVICE_TYPE, message_id,
                             QMI_RESP_CTRL_FLAGS, tx_id, msg_ptr_len - QMI_HEADER_SIZE);
            memcpy(tmp_msg_ptr, msg_ptr, resp_len);

            ret = SSL_write(ssl, resp_msg_ptr, msg_ptr_len);
            CHK_SSL(ret);

            free(resp_msg_ptr);
            return TRUE;
        }
        else
            error_code = QMI_ERR_INTERNAL_V01;
    }

    //Something was wrong with the msg, send error back to GW
    msg_ptr_len = QMI_HEADER_SIZE + QMI_ERROR_SIZE;
    msg_ptr = malloc(msg_ptr_len);
    if (msg_ptr == NULL){
        LOG_MSG_ERROR("MALLOC failure",0,0,0);
        return QMI_IP_ERROR;
    }
    tmp_msg_ptr = msg_ptr;

    WRITE_QMI_HEADER(tmp_msg_ptr, msg_ptr_len, QMI_WDS_SERVICE_TYPE, message_id, QMI_RESP_CTRL_FLAGS, tx_id, msg_ptr_len - QMI_HEADER_SIZE);

    //write TLV
    tlv_len = QMI_ERROR_TLV;
    tlv_msg_ptr = malloc(tlv_len);
    if (tlv_msg_ptr == NULL){
        LOG_MSG_ERROR("MALLOC failure",0,0,0);
        free(msg_ptr);
        return QMI_IP_ERROR;
    }
    tlv_tmp_msg_ptr = tlv_msg_ptr;

    WRITE_16_BIT_VAL(tlv_tmp_msg_ptr, QMI_RESULT_FAILURE_V01);
    WRITE_16_BIT_VAL(tlv_tmp_msg_ptr, error_code);

    if (tlv_len > 0 && qmi_util_write_std_tlv(&tmp_msg_ptr, &msg_ptr_len, QMI_IP_RESULT_CODE, tlv_len, (void *)tlv_msg_ptr) < 0)
    {
        LOG_MSG_INFO2("write TLV failed",0,0,0);
    }

    ret = SSL_write(ssl, msg_ptr, msg_ptr_len);
    CHK_SSL(ret);

    free(msg_ptr);
    free(tlv_msg_ptr);
    return TRUE;
}

/*===========================================================================
  FUNCTION  PROCESS_NAS_MESSAGE
===========================================================================*/
/*!
@brief

  Proccess the nas messages from the GW.

@return

  - Calls corresponding NAS function based off message

@note

  - Dependencies
    - SSL link to GW active

  - Side Effects
    - None
*/
/*=========================================================================*/

//needs to be made into generic function for any QMI service
int proccess_nas_message(unsigned char **msg,
                         unsigned long message_id,
                         int msg_len,
                         unsigned long tx_id)
{
  int            ret;
  unsigned char *msg_ptr;
  unsigned char *resp_msg_ptr;
  unsigned char *tmp_msg_ptr;
  unsigned char *tlv_msg_ptr;
  unsigned char *tlv_tmp_msg_ptr;
  int            msg_ptr_len;
  int            resp_len, tlv_len;

  msg_ptr = *msg;
  int *qmi_error_code;
  ret =  qmi_client_send_raw_msg_sync (qmi_nas_handle, message_id,
                                      (unsigned char *)((unsigned char *)msg_ptr),
                                       msg_len, msg_ptr, QMI_MAX_STD_MSG_SIZE,
                                       &resp_len, QMI_TIMEOUT);

  if (ret == QMI_NO_ERR)
  {
    msg_ptr_len = QMI_HEADER_SIZE + resp_len;
    resp_msg_ptr = malloc(msg_ptr_len);
    if (resp_msg_ptr == NULL){
        LOG_MSG_ERROR("MALLOC failure",0,0,0);
        return QMI_IP_ERROR;
    }
    tmp_msg_ptr = resp_msg_ptr;

    WRITE_QMI_HEADER(tmp_msg_ptr, msg_ptr_len, QMI_NAS_SERVICE_TYPE, message_id,
                     QMI_RESP_CTRL_FLAGS, tx_id, msg_ptr_len - QMI_HEADER_SIZE);

    memcpy(tmp_msg_ptr, msg_ptr, resp_len);
  }
  else
  {
    LOG_MSG_ERROR("qmi_client_send_raw_msg_sync error %d",ret,0,0);
    /* if qmi_client_send_raw_msg_sync fails we still need to send a
    response back to the GW */
    msg_ptr_len = QMI_HEADER_SIZE + 7;
    resp_msg_ptr = malloc(msg_ptr_len);

    if (resp_msg_ptr == NULL){
        LOG_MSG_ERROR("MALLOC failure",0,0,0);
        return QMI_IP_ERROR;
    }
    tmp_msg_ptr = resp_msg_ptr;

    WRITE_QMI_HEADER(tmp_msg_ptr, msg_ptr_len, QMI_NAS_SERVICE_TYPE,
                     message_id, QMI_RESP_CTRL_FLAGS, tx_id,
                     msg_ptr_len - QMI_HEADER_SIZE);

    tlv_len = QMI_ERROR_TLV;
    tlv_msg_ptr = malloc(tlv_len);
    if (tlv_msg_ptr == NULL){
        free(resp_msg_ptr);
        return QMI_IP_ERROR;
    }
    tlv_tmp_msg_ptr = tlv_msg_ptr;

    WRITE_16_BIT_VAL(tlv_tmp_msg_ptr, QMI_RESULT_FAILURE_V01);
    WRITE_16_BIT_VAL(tlv_tmp_msg_ptr, QMI_ERR_UNKNOWN_V01);

    if (qmi_util_write_std_tlv(&tmp_msg_ptr, &msg_ptr_len,
                               QMI_IP_RESULT_CODE, tlv_len,
                               (void *)tlv_msg_ptr) < 0)
    {
      LOG_MSG_INFO2("NAS return message write TLV failed",0,0,0);
    }

    free(tlv_msg_ptr);
  }

  /* Check is needed since a nas message is sent to disable embms when SSL
  goes down if dsi_net_mode is connected */
  if (ssl != NULL) {
      ret = SSL_write(ssl, resp_msg_ptr, msg_ptr_len);
      CHK_SSL(ret);
  }

  free(resp_msg_ptr);
}

/*===========================================================================
  FUNCTION  PROCESS_QMI_IP_MESSAGE
===========================================================================*/
/*!
@brief

  Proccess the qmi_ip messages from the GW.

@return

  - Sets or returns the ODU mode based off qmi_ip message

@note

  - Dependencies
    - SSL link to GW active

  - Side Effects
    - None
*/
/*=========================================================================*/

int proccess_odu_message(unsigned char **msg,
                         unsigned long message_id,
                         int msg_len,
                         unsigned long tx_id)
{
    int             ret;
    unsigned long   type;
    unsigned char   *value_ptr = NULL;
    unsigned char   temp_char;
    unsigned long   value_len;
    int             error_code = QMI_ERR_NONE_V01;
    int             msg_ptr_len;
    qmi_error_type_v01 qmi_error;

    unsigned char *msg_ptr;
    unsigned char *tmp_msg_ptr;
    int            tlv_size;
    unsigned long tlv_type;
    unsigned char *tlv_msg_ptr;
    unsigned char *tlv_tmp_msg_ptr;
    unsigned char tlv_msg_array[255];
	unsigned char *tmp_tlv_msg_array_ptr;
    int reboot = FALSE;

    msg_ptr_len = QMI_HEADER_SIZE;
    memset((char*)tlv_msg_array,0,255);
	tmp_tlv_msg_array_ptr = tlv_msg_array;

    switch (message_id) {
    case QMI_IP_ODU_SET_MODE:
    {
        error_code = QMI_ERR_MISSING_ARG_V01;
        while (msg_len >0)
        {
            if ((ret = qmi_util_read_std_tlv(msg, &msg_len, &type,
                                             &value_len, &value_ptr))
                 ==  QMI_INTERNAL_ERR)
            {
                LOG_MSG_INFO1("read TLV error %d",ret,0,0);
                break;
            }

            switch (type)
            {
            case QMI_IP_ODU_MODE_TLV_ID:
                READ_8_BIT_VAL(value_ptr, temp_char);
                int mode = (int) temp_char;

                if (mode != device_mode)
                {
                    qcmap_msgr_set_odu_mode_req_msg_v01 set_mode_req;
                    qcmap_msgr_set_odu_mode_resp_msg_v01 set_mode_resp;

                    memset(&set_mode_req,0,
                           sizeof(qcmap_msgr_set_odu_mode_req_msg_v01));
                    memset(&set_mode_resp,0,
                           sizeof(qcmap_msgr_set_odu_mode_resp_msg_v01));

                    set_mode_req.mode_valid = TRUE;
                    if (mode == BRIDGE_MODE)
                        set_mode_req.mode = QCMAP_MSGR_ODU_BRIDGE_MODE_V01;
                    else
                        set_mode_req.mode = QCMAP_MSGR_ODU_ROUTER_MODE_V01;

                    qmi_error = qmi_client_send_msg_sync(qmi_ip_conf.qmi_ip_qcmap_msgr_handle,
                                       QMI_QCMAP_MSGR_SET_ODU_MODE_REQ_V01,
                                       (void*)&set_mode_req,
                                       sizeof(qcmap_msgr_set_odu_mode_req_msg_v01),
                                       (void*)&set_mode_resp,
                                       sizeof(qcmap_msgr_set_odu_mode_resp_msg_v01),
                                       QMI_TIMEOUT);

                    if ((qmi_error != QMI_NO_ERR) ||
                          (set_mode_resp.resp.result != QMI_NO_ERR))
                    {
                      LOG_MSG_ERROR("Can not set ODU mode %d : %d", qmi_error,
                                    set_mode_resp.resp.error,0);

                      if (set_mode_resp.resp.result == QMI_NO_ERR)
                          error_code = QMI_ERR_UNKNOWN_V01;
                      else
                          error_code = set_mode_resp.resp.error;
                    }
                    else
                        device_mode = mode;

                    reboot = TRUE;
                }
                else
                    error_code = QMI_ERR_NO_EFFECT_V01;
                break;
            default:
                LOG_MSG_INFO1("TLV type not supported",0,0,0);
                break;
            }
        }
        break;
    }
    case QMI_IP_ODU_GET_MODE:
    {
        qcmap_msgr_get_odu_mode_resp_msg_v01 get_mode_resp;
        memset(&get_mode_resp,0,
               sizeof(qcmap_msgr_get_odu_mode_resp_msg_v01));

        qmi_error = qmi_client_send_msg_sync(qmi_ip_conf.qmi_ip_qcmap_msgr_handle,
                           QMI_QCMAP_MSGR_GET_ODU_MODE_REQ_V01,
                           NULL,
                           0,
                           (void*)&get_mode_resp,
                           sizeof(qcmap_msgr_get_odu_mode_resp_msg_v01),
                           QMI_TIMEOUT);

        if ((qmi_error != QMI_NO_ERR) ||
              (get_mode_resp.resp.result != QMI_NO_ERR))
        {
          LOG_MSG_ERROR("Can not get ODU mode %d : %d", qmi_error,
                        get_mode_resp.resp.error,0);

          if (get_mode_resp.resp.result == QMI_NO_ERR)
              error_code = QMI_ERR_UNKNOWN_V01;
          else
              error_code = get_mode_resp.resp.error;
          break;
        }

        tlv_size = 1;
        tlv_msg_ptr = malloc(tlv_size);
        if (tlv_msg_ptr == NULL){
            LOG_MSG_ERROR("MALLOC failure",0,0,0);
            return QMI_IP_ERROR;
        }
        tlv_tmp_msg_ptr = tlv_msg_ptr;
        tlv_type = QMI_IP_ODU_MODE_TLV;
        WRITE_16_BIT_VAL(tlv_tmp_msg_ptr, get_mode_resp.mode);

        ret = qmi_util_write_std_tlv(&tmp_tlv_msg_array_ptr, &msg_ptr_len,
                                     tlv_type, tlv_size, (void *)tlv_msg_ptr);

        free(tlv_msg_ptr);
        break;
    }
    case QMI_IP_ODU_SET_PSWD:
    {
        qcmap_msgr_set_odu_pswd_req_msg_v01 set_pswd_req;
        qcmap_msgr_set_odu_pswd_resp_msg_v01 set_pswd_resp;
        char password[MAX_PSWD_SIZE];
        int password_size = 0;

        error_code = QMI_ERR_MISSING_ARG_V01;
        while (msg_len >0)
        {
            if ((ret = qmi_util_read_std_tlv(msg, &msg_len, &type, &value_len, &value_ptr))
                 ==  QMI_INTERNAL_ERR)
            {
                LOG_MSG_INFO1("read TLV error %d",ret,0,0);
                break;
            }

            switch (type)
            {
            case QMI_IP_ODU_PSWD_TLV_ID:
                while (password_size < value_len) {
                    READ_8_BIT_VAL(value_ptr, temp_char);
                    password[password_size++] = (char)temp_char;
                }

                if (password_size > MAX_PSWD_SIZE) {
                    error_code = QMI_ERR_INVALID_ARG_V01;
                    break;
                }

                memset(&set_pswd_req,0, sizeof(qcmap_msgr_set_odu_pswd_req_msg_v01));
                memset(&set_pswd_resp,0, sizeof(qcmap_msgr_set_odu_pswd_resp_msg_v01));

                set_pswd_req.password_valid = TRUE;
                memcpy(set_pswd_req.password, password, password_size);

                qmi_error = qmi_client_send_msg_sync(qmi_ip_conf.qmi_ip_qcmap_msgr_handle,
                                   QMI_QCMAP_MSGR_SET_ODU_PSWD_REQ_V01,
                                   (void*)&set_pswd_req,
                                   sizeof(qcmap_msgr_set_odu_pswd_req_msg_v01),
                                   (void*)&set_pswd_resp,
                                   sizeof(qcmap_msgr_set_odu_pswd_resp_msg_v01),
                                   QMI_TIMEOUT);

                LOG_MSG_INFO1("qmi_client_send_msg_sync(set_odu_pswd): error %d result %d",
                  qmi_error, set_pswd_resp.resp.result, 0);

                if ((qmi_error != QMI_NO_ERR) ||
                      (set_pswd_resp.resp.result != QMI_NO_ERR))
                {
                  LOG_MSG_ERROR("Can not get ODU mode %d : %d", qmi_error,
                                set_pswd_resp.resp.error,0);

                  if (set_pswd_resp.resp.result == QMI_NO_ERR)
                      error_code = QMI_ERR_UNKNOWN_V01;
                  else
                      error_code = set_pswd_resp.resp.error;
                }
                break;
            default:
                LOG_MSG_INFO1("TLV type not supported",0,0,0);
                break;
            }
        }
        break;
    }
    case QMI_IP_ODU_PSWD_RESET:
    {
        qmi_error = pasword_reset();

        if (qmi_error != QMI_NO_ERR)
        {
            LOG_MSG_ERROR("Can not complete password reset %d", qmi_error,0,0);

            error_code = QMI_ERR_NONE_V01;
        }
        else
            error_code = qmi_error;

        LOG_MSG_INFO1("Password reset successfully",0,0,0);

        //after password is reset, reboot device
        reboot = TRUE;
        break;
    }
    default:
        LOG_MSG_INFO1("Invalid QMI_IP message ID",0,0,0);
        break;
    }

    tlv_size = QMI_ERROR_TLV;
    tlv_msg_ptr = malloc(tlv_size);
    if (tlv_msg_ptr == NULL){
        LOG_MSG_ERROR("MALLOC failure",0,0,0);
        return QMI_IP_ERROR;
    }
    tlv_tmp_msg_ptr = tlv_msg_ptr;
    if (error_code == QMI_ERR_NONE_V01)
    {
        WRITE_16_BIT_VAL(tlv_tmp_msg_ptr, QMI_RESULT_SUCCESS_V01);
        WRITE_16_BIT_VAL(tlv_tmp_msg_ptr, QMI_ERR_NONE_V01);
    }
    else{
        WRITE_16_BIT_VAL(tlv_tmp_msg_ptr, QMI_RESULT_FAILURE_V01);
        WRITE_16_BIT_VAL(tlv_tmp_msg_ptr, error_code);
    }
    ret = qmi_util_write_std_tlv(&tmp_tlv_msg_array_ptr, &msg_ptr_len, QMI_IP_RESULT_CODE, tlv_size, (void *)tlv_msg_ptr);

    msg_ptr = malloc(msg_ptr_len);
    if (msg_ptr == NULL){
        LOG_MSG_ERROR("MALLOC failure",0,0,0);
        free(tlv_msg_ptr);
        return QMI_IP_ERROR;
    }
    tmp_msg_ptr = msg_ptr;

    WRITE_QMI_HEADER(tmp_msg_ptr, msg_ptr_len, QMI_IP_ODU_SERVICE_TYPE, message_id,
                     QMI_RESP_CTRL_FLAGS, tx_id, msg_ptr_len - QMI_HEADER_SIZE);

    memcpy(tmp_msg_ptr, tlv_msg_array, msg_ptr_len - QMI_HEADER_SIZE);

	ret = SSL_write(ssl, msg_ptr, msg_ptr_len);
    CHK_SSL(ret);

    free(tlv_msg_ptr);
    free(msg_ptr);

    if (reboot)
    {
        sleep(5);
        LOG_MSG_INFO1("Rebooting device",0,0,0);

        /* Shutdown SSL before rebooting */
        SSL_shutdown(ssl);

        ds_system_call("reboot", strlen("reboot"));
    }
}
