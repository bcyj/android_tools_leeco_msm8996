/******************************************************************************
  @file    qmi_atcop_srvc.c
  @brief   The QMI ATCOP service layer.

  DESCRIPTION
  QMI ATCOP service routines.

  INITIALIZATION AND SEQUENCING REQUIREMENTS
  qmi_atcop_srvc_init_client() needs to be called before sending or receiving of any
  QoS service messages

  ---------------------------------------------------------------------------
  Copyright (c) 2010-2014 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/


#include "qmi_i.h"
#include "qmi_service.h"
#include "qmi_atcop_srvc.h"
#include "qmi_util.h"

#define QMI_ATCOP_STD_MSG_SIZE QMI_MAX_STD_MSG_SIZE

/*Request Response TLVS*/
#define QMI_ATCOP_CMD_INFO_TLV        0x01
#define QMI_ATCOP_AT_CMD_LIST_TLV     0x01
#define QMI_ATCOP_AT_RESP_INFO_TLV    0x01

#define QMI_ATCOP_AT_FWD_URC_LEN      0x01

#define QMI_ATCOP_AT_CMD_HDL_TLV      0x01
/*MSG IDS*/
#define QMI_ATCOP_REGISTER_AT_CMD_FWD_MSD_ID          0x0020
#define QMI_ATCOP_REGISTER_AT_CMD_FWD_RESP_MSG_ID     0x0022
#define QMI_ATCOP_AT_CMD_URC_FWD_REQ_MSG_ID           0x0024
/*Indication Message IDs*/
#define QMI_ATCOP_ABORT_IND_MSG_ID                      0x0023
#define QMI_ATCOP_AT_FWD_IND_MSG_ID                     0x0021

/*Indication TLV IDs*/
#define QMI_ATCOP_FWD_CMD_AT_CMD_INFO_TLV                0x01
#define QMI_ATCOP_FWD_CMD_AT_TOKEN_INFO_TLV              0x10
#define QMI_ATCOP_FWD_CMD_AT_STATE_INFO_TLV              0x11
/*Nested AT FWD TLVs*/
#define QMI_ATCOP_FWD_CMD_TOKEN_TLV                      0x10

#define QMI_ATCOP_FWD_CMD_V_VAL_TLV                      0x10
#define QMI_ATCOP_FWD_CMD_Q_VAL_TLV                      0x11
#define QMI_ATCOP_FWD_CMD_S3_VAL_TLV                     0x12
#define QMI_ATCOP_FWD_CMD_S4_VAL_TLV                     0x13
#define QMI_ATCOP_FWD_CMD_CLIR_VAL_TLV                   0x14
#define QMI_ATCOP_FWD_CMD_COLP_VAL_TLV                   0x15
#define QMI_ATCOP_FWD_CMD_CMEE_VAL_TLV                   0x16
#define QMI_ATCOP_FWD_CMD_CCUG_VAL_TLV                   0x17
#define QMI_ATCOP_FWD_CMD_CMEC_VAL_TLV                   0x18
#define QMI_ATCOP_FWD_CMD_CMEC_VAL_4_TLV                 0x19


static int atcop_service_initialized = FALSE;
/*====================================
           Utility fns
======================================*/
/*===========================================================================
  FUNCTION  qmi_atcop_reg_at_format_cmd_list_tlv
===========================================================================*/
/*!
@brief
    Formats the command list TLV for qmi_atcop_reg_at_command_fwd_req
@return
  None.

@note
  None.
*/
/*=========================================================================*/
static int
qmi_atcop_reg_at_format_cmd_list_tlv
(
  qmi_atcop_at_cmd_fwd_req_type     *cmd_fwd_req,
  unsigned char                     *param_ptr,
  int                               *param_len
)
{
  int i, num_cmds;

  if(!cmd_fwd_req || !param_ptr || !param_len)
  {
    QMI_ERR_MSG_0("qmi_atcop_reg_at_format_cmd_list_tlv: Bad Input Received\n");
    return QMI_INTERNAL_ERR;
  }

  num_cmds = cmd_fwd_req->num_of_cmds;

  for (i = 0; i < num_cmds; i++)
  {
    int at_cmd_len;
    INIT_ENVELOPE_TLV_HDR (param_ptr, *param_len);

    /*calculate the length of the value field*/
    at_cmd_len = (int)strlen((const char *)cmd_fwd_req->qmi_atcop_at_cmd_fwd_req_type[i].at_cmd_name);
    tlv_length = (int)(2 * sizeof(unsigned char)) + at_cmd_len; //2 here corresponds to size of
                                                      // abort field + size of length[at cmd field]
    /* Subtract tlv_length from param_buf_len */
    *param_len -= tlv_length;

    /* See if buffer is big enough */
    if (*param_len < 0)
    {
      QMI_ERR_MSG_1 ("qmi_atcop_reg_at_format_cmd_list_tlv: Will Overflow ParamBuf, %d .\n", QMI_ATCOP_MAX_PARAM_BUF_SIZE);
      return QMI_INTERNAL_ERR;
    }

    WRITE_8_BIT_VAL(param_ptr,cmd_fwd_req->qmi_atcop_at_cmd_fwd_req_type[i].abort_flag);
    WRITE_8_BIT_VAL(param_ptr,at_cmd_len);
    memcpy(param_ptr, cmd_fwd_req->qmi_atcop_at_cmd_fwd_req_type[i].at_cmd_name, (size_t)at_cmd_len);
    param_ptr += at_cmd_len;

    SET_ENVELOPE_TLV_HDR (QMI_ATCOP_CMD_INFO_TLV);

  }

  return QMI_NO_ERR;
}
/*===========================================================================
  FUNCTION  qmi_atcop_process_at_cmd_fwd_state_info
===========================================================================*/
/*!
@brief
    Processes the at command indications state information received from modem
    and places them in users c data structure.
@return
  None.

@note
  None.
*/
/*=========================================================================*/
static int
qmi_atcop_process_at_cmd_fwd_state_info
(
  unsigned char                   *rx_msg_buf,
  int                              rx_msg_len,
  qmi_atcop_indication_data_type  *ind_data
)
{
  unsigned long type;
  unsigned long length;
  unsigned char *value_ptr;
  qmi_atcop_at_cmd_fwd_ind_type   *at_cmd_fwd;

  if(rx_msg_buf == NULL || ind_data == NULL)
  {
    QMI_ERR_MSG_0("qmi_atcop_process_at_cmd_fwd_state_info: Bad Input Received\n");
    return QMI_INTERNAL_ERR;
  }

  at_cmd_fwd = &ind_data->at_cmd_fwd_type;

  /* Initialize CMEC value 4 present flag to FALSE */
  at_cmd_fwd->cmec_val.cmec_val4_is_valid = FALSE;

  while (rx_msg_len > 0)
  {
    if (qmi_util_read_std_tlv (&rx_msg_buf,
                                  &rx_msg_len,
                                  &type,
                                  &length,
                                  &value_ptr) < 0)
    {
      return QMI_INTERNAL_ERR;
    }

    /* Now process TLV */
    switch (type)
    {
      case QMI_ATCOP_FWD_CMD_V_VAL_TLV:
      {
        at_cmd_fwd->param_mask |= QMI_ATCOP_STATE_INFO_V_VAL;
        READ_32_BIT_VAL(value_ptr,at_cmd_fwd->v_val);
      }
      break;
      case QMI_ATCOP_FWD_CMD_Q_VAL_TLV:
      {
        at_cmd_fwd->param_mask |= QMI_ATCOP_STATE_INFO_Q_VAL;
        READ_32_BIT_VAL(value_ptr,at_cmd_fwd->q_val);
      }
      break;
      case QMI_ATCOP_FWD_CMD_S3_VAL_TLV:
      {
        at_cmd_fwd->param_mask |= QMI_ATCOP_STATE_INFO_S3_VAL;
        READ_32_BIT_VAL(value_ptr,at_cmd_fwd->s3_val);
      }
      break;
      case QMI_ATCOP_FWD_CMD_S4_VAL_TLV:
      {
        at_cmd_fwd->param_mask |= QMI_ATCOP_STATE_INFO_S4_VAL;
        READ_32_BIT_VAL(value_ptr,at_cmd_fwd->s4_val);
      }
      break;
      case QMI_ATCOP_FWD_CMD_CLIR_VAL_TLV:
      {
        at_cmd_fwd->param_mask |= QMI_ATCOP_STATE_INFO_CLIR_VAL;
        READ_32_BIT_VAL(value_ptr,at_cmd_fwd->clir_val);
      }
      break;
      case QMI_ATCOP_FWD_CMD_COLP_VAL_TLV:
      {
        at_cmd_fwd->param_mask |= QMI_ATCOP_STATE_INFO_COLP_VAL;
        READ_32_BIT_VAL(value_ptr,at_cmd_fwd->colp_val);
      }
      break;
      case QMI_ATCOP_FWD_CMD_CMEE_VAL_TLV:
      {
        at_cmd_fwd->param_mask |= QMI_ATCOP_STATE_INFO_CMEE_VAL;
        READ_32_BIT_VAL(value_ptr,at_cmd_fwd->cmee_val);
      }
      break;
      case QMI_ATCOP_FWD_CMD_CCUG_VAL_TLV:
      {
        at_cmd_fwd->param_mask |= QMI_ATCOP_STATE_INFO_CCUG_VAL;
        READ_32_BIT_VAL(value_ptr,at_cmd_fwd->ccug_val.ccug_val1);
        READ_32_BIT_VAL(value_ptr,at_cmd_fwd->ccug_val.ccug_val2);
        READ_32_BIT_VAL(value_ptr,at_cmd_fwd->ccug_val.ccug_val3);
      }
      break;
      case QMI_ATCOP_FWD_CMD_CMEC_VAL_TLV:
      {
        at_cmd_fwd->param_mask |= QMI_ATCOP_STATE_INFO_CMEC_VAL;
        READ_32_BIT_VAL(value_ptr,at_cmd_fwd->cmec_val.cmec_val1);
        READ_32_BIT_VAL(value_ptr,at_cmd_fwd->cmec_val.cmec_val2);
        READ_32_BIT_VAL(value_ptr,at_cmd_fwd->cmec_val.cmec_val3);
      }
      break;
      case QMI_ATCOP_FWD_CMD_CMEC_VAL_4_TLV:
      {
        /* Will never get cmec 4, without getting 1,2 and 3, so no separate
        ** param mask needed
        */
        READ_32_BIT_VAL(value_ptr,at_cmd_fwd->cmec_val.cmec_val4);
        at_cmd_fwd->cmec_val.cmec_val4_is_valid = TRUE;
      }
      break;
      default:
         QMI_ERR_MSG_1 ("qmi_atcop_process_at_cmd_fwd_state_info: TLV type %ld Not handled\n",type);
      break;
    }
  }
  return QMI_NO_ERR;
}
/*===========================================================================
  FUNCTION  qmi_atcop_process_at_fwd_ind
===========================================================================*/
/*!
@brief
    Processes the at command indications received from modem and places them
    in users c data structure.
@return
  None.

@note
  None.
*/
/*=========================================================================*/
static int
qmi_atcop_process_at_fwd_ind
(
  unsigned char                     *rx_msg_buf,
  int                               rx_msg_len,
  qmi_atcop_indication_data_type    *ind_data
)
{
  unsigned long type;
  unsigned long length;
  unsigned char *value_ptr;
  int rc;
  qmi_atcop_at_cmd_fwd_ind_type   *at_cmd_fwd;


  if(rx_msg_buf == NULL || ind_data == NULL)
  {
    QMI_ERR_MSG_0("qmi_atcop_process_at_fwd_ind: Bad Input Received\n");
    return QMI_INTERNAL_ERR;
  }

  at_cmd_fwd = &ind_data->at_cmd_fwd_type;

  memset(at_cmd_fwd,0,sizeof(qmi_atcop_at_cmd_fwd_ind_type));

  /*Initialize Param Mask*/
  at_cmd_fwd ->param_mask           = 0;

  while (rx_msg_len > 0)
  {
    if (qmi_util_read_std_tlv (&rx_msg_buf,
                                  &rx_msg_len,
                                  &type,
                                  &length,
                                  &value_ptr) < 0)
    {
      return QMI_INTERNAL_ERR;
    }

    /* Now process TLV */
    switch (type)
    {
      case QMI_ATCOP_FWD_CMD_AT_CMD_INFO_TLV:
      {
         /* Read in value */
        int at_cmd_len;

        READ_32_BIT_VAL (value_ptr,at_cmd_fwd->at_hndl);
        READ_32_BIT_VAL (value_ptr,at_cmd_fwd->op_code);
	{ /*lint --e{774} */
        READ_8_BIT_VAL (value_ptr,at_cmd_len);
        if (at_cmd_len > QMI_ATCOP_AT_CMD_NAME_MAX_LEN)
        {
          QMI_ERR_MSG_0("qmi_atcop_process_at_fwd_ind: Length Not Supported\n");
          return QMI_INTERNAL_ERR;
        }
	}

        memcpy(at_cmd_fwd->at_name, value_ptr, (size_t)at_cmd_len);
        at_cmd_fwd->at_name[at_cmd_len] = '\0';
      }
      break;

      case QMI_ATCOP_FWD_CMD_AT_TOKEN_INFO_TLV:
      {
        int i = 0;
        unsigned long tmp_type;
        unsigned long tmp_length;
        unsigned char *tmp_value_ptr;
        int tmp_msg_len = (int) length;

        while (tmp_msg_len > 0)
        {
          if (qmi_util_read_std_tlv (&value_ptr,
                                        &tmp_msg_len,
                                        &tmp_type,
                                        &tmp_length,
                                        &tmp_value_ptr) < 0)
          {
            return QMI_INTERNAL_ERR;
          }

          if (tmp_type != QMI_ATCOP_FWD_CMD_AT_TOKEN_INFO_TLV )
          {
             QMI_ERR_MSG_1("qmi_atcop_process_at_fwd_ind: Ignoring unknown TLV received = %d\n",(int) tmp_type);
             continue;
          }
          if (i < QMI_ATCOP_MAX_TOKENS)
          {
            at_cmd_fwd->tokens[i] = (unsigned char *)tmp_value_ptr;
            i++;
          }
          else
          {
            QMI_ERR_MSG_1("qmi_atcop_process_at_fwd_ind: Max number of tokens allowed %d already received. Cannot Handle more\n",QMI_ATCOP_MAX_TOKENS);
            break;
          }

        }/*While Loop*/
        at_cmd_fwd->num_tokens = (unsigned short)i;
      }/*Case*/
      break;

      case QMI_ATCOP_FWD_CMD_AT_STATE_INFO_TLV:
      {
        if ((rc = qmi_atcop_process_at_cmd_fwd_state_info(value_ptr,(int)length,ind_data)) < 0)
        {
          QMI_ERR_MSG_0("qmi_atcop_process_at_fwd_ind: Processing State Info Failed.\n");
          return rc;
        }
      }
      break;

      default:
        QMI_ERR_MSG_1 ("qmi_atcop_process_at_fwd_ind: TLV type %ld Not handled\n",type);
      break;
    }
  }
  return QMI_NO_ERR;
}

/*===========================================================================
  FUNCTION  qmi_atcop_srvc_indication_cb
===========================================================================*/
/*!
@brief
  This is the callback function that will be called by the generic
  services layer to report asynchronous indications.  This function will
  process the indication TLV's and then call the user registered
  functions with the indication data.

@return
  None.

@note

  - Dependencies
    - qmi_connection_init() must be called for the associated port first.

  - Side Effects
    - Talks to modem processor
*/
/*=========================================================================*/

static void
qmi_atcop_srvc_indication_cb
(
  int                   user_handle,
  qmi_service_id_type   service_id,
  unsigned long         msg_id,
  void                                *user_ind_msg_hdlr,
  void                                *user_ind_msg_hdlr_user_data,
  unsigned char         *rx_msg_buf,
  int                   rx_msg_len
)
{
  qmi_atcop_indication_id_type      ind_id;
  qmi_atcop_indication_data_type    ind_data;
  qmi_atcop_indication_hdlr_type    user_ind_hdlr;

  unsigned long type;
  unsigned long length;
  unsigned char *value_ptr;


  /* Make sure that the user indication handler isn't NULL */
  if (user_ind_msg_hdlr == NULL)
  {
    QMI_ERR_MSG_0 ("qmi_atcop_srvc_indication_cb: User Indication Handler "
                   "not present, must be present to receive AT command's\n");
    return;
  }

  memset(&ind_data, 0, sizeof(ind_data));

  /* Get properly cast pointer to user indication handler */
  /*lint -e{611} */
  user_ind_hdlr = (qmi_atcop_indication_hdlr_type) user_ind_msg_hdlr;

  switch (msg_id)
  {
    case QMI_ATCOP_ABORT_IND_MSG_ID:
    {
      ind_id = QMI_ATCOP_SRVC_ABORT_MSG_IND_TYPE;

      if (qmi_util_read_std_tlv (&rx_msg_buf,
                                    &rx_msg_len,
                                    &type,
                                    &length,
                                    &value_ptr) < 0)
      {
        return ;
      }

      if (type != QMI_ATCOP_AT_CMD_HDL_TLV)
      {
        QMI_ERR_MSG_1 ("qmi_atcop_srvc_indication_cb::Invalid TLV received %lx \n ",type);
        return;
      }

      READ_32_BIT_VAL (value_ptr,ind_data.at_hndl);
    }
    break;

    case QMI_ATCOP_AT_FWD_IND_MSG_ID:
    {
      ind_id = QMI_ATCOP_SRVC_AT_FWD_MSG_IND_TYPE;

      if ( qmi_atcop_process_at_fwd_ind(rx_msg_buf,rx_msg_len,&ind_data) < 0)
      {
        QMI_ERR_MSG_0("qmi_atcop_process_at_fwd_ind: Processing CMD FWD IND Failed.\n");
        return;
      }
    }
    break;
    default:
    {
      QMI_ERR_MSG_0("qmi_atcop_process_at_fwd_ind: Invalid Indication Received.\n");
      return;
    }
  }/*Switch*/

  /* Call user registered handler */
  user_ind_hdlr (user_handle,
                 service_id,
                 user_ind_msg_hdlr_user_data,
                 ind_id,
                 &ind_data);

}


/*===========================================================================
  FUNCTION  qmi_atcop_srvc_init
===========================================================================*/
/*!
@brief
  This function is a callback that will be called once during client
  initialization

@return
  None.

@note

  - Dependencies
    - None.

  - Side Effects
    - None.
*/
/*=========================================================================*/
int qmi_atcop_srvc_init (void)
{
  int  rc = QMI_NO_ERR;
  if (!atcop_service_initialized)
  {

    rc = qmi_service_set_srvc_functions (QMI_ATCOP_SERVICE,
                                         qmi_atcop_srvc_indication_cb);
    if (rc != QMI_NO_ERR)
    {
      QMI_ERR_MSG_1("qmi_atcop_srvc_init: set srvc functions returns err=%d\n",rc);
    }
    else
    {
      QMI_DEBUG_MSG_0("qmi_atcop_srvc_init: ATCOP successfully initialized");
      atcop_service_initialized = TRUE;
    }
  }
  else
  {
    QMI_ERR_MSG_0 ("qmi_atcop_srvc_init: Init failed, ATCOP already initialized");
  }
  return rc;
}


/*===========================================================================
  FUNCTION  qmi_atcop_srvc_release
===========================================================================*/
/*!
@brief
  This function is a callback that will be called once during client
  release

@return
  None.

@note

  - Dependencies
    - None.

  - Side Effects
    - None.
*/
/*=========================================================================*/
int qmi_atcop_srvc_release (void)
{
  int  rc = QMI_NO_ERR;
  if (atcop_service_initialized)
  {

    rc = qmi_service_set_srvc_functions (QMI_ATCOP_SERVICE,NULL);
    if (rc != QMI_NO_ERR)
    {
      QMI_ERR_MSG_1("qmi_atcop_srvc_release: set srvc functions returns err=%d\n",rc);
    }
    else
    {
      QMI_DEBUG_MSG_0("qmi_atcop_srvc_release: ATCOP successfully released");
      atcop_service_initialized = FALSE;
    }
  }
  else
  {
    QMI_ERR_MSG_0 ("qmi_atcop_srvc_release: Release failed, ATCOP not initialized");
  }
  return rc;
}

/*===========================================================================
  FUNCTION  qmi_atcop_srvc_init_client
===========================================================================*/
/*!
@brief
  This function is called to initialize the ATCOP service.  This function
  must be called prior to calling any other ATCOP service functions.
  Also note that this function may be called multiple times to allow
  for multiple, independent clients.

@return
  Valid user handle which is > 0, if the init client operation was successful,
  < 0 if not, If the return code is QMI_SERVICE_ERR, then the qmi_err_code will be
  valid and indicated which QMI error occured.

@note

  - Dependencies
    - qmi_connection_init() must be called for the associated port first.

  - Side Effects
    - Talks to modem processor
*/
/*=========================================================================*/

qmi_client_handle_type
qmi_atcop_srvc_init_client
(
  const char                      *dev_id,
  qmi_atcop_indication_hdlr_type  user_ind_msg_hdlr,
  void                            *user_ind_msg_hdlr_user_data,
  int                             *qmi_err_code
)
{
  qmi_client_handle_type client_handle;
  qmi_connection_id_type conn_id;

  if(!user_ind_msg_hdlr)
  {
     QMI_ERR_MSG_0("qmi_atcop_srvc_init_client: No ATCOP srvc Indication Handler \n");
     return QMI_INTERNAL_ERR;
  }

  if ((conn_id = QMI_PLATFORM_DEV_NAME_TO_CONN_ID(dev_id)) == QMI_CONN_ID_INVALID)
  {
    return QMI_INTERNAL_ERR;
  }

  /* Call common service layer initialization function */
  /*lint -e{611} */
  client_handle =  qmi_service_init (conn_id,
                                   QMI_ATCOP_SERVICE,
                                     (void *) user_ind_msg_hdlr,
                                     user_ind_msg_hdlr_user_data,
                                   qmi_err_code);
  return client_handle;
}


/*===========================================================================
  FUNCTION  qmi_atcop_srvc_release_client
===========================================================================*/
/*!
@brief
  This function is called to release a client created by the
  qmi_atcop_srvc_init_client() function.  This function should be called
  for any client created when terminating a client process, especially
  if the modem processor is not reset.  The modem side QMI server has
  a limited number of clients that it will allocate, and if they are not
  released, we will run out.

@return
  QMI_NO_ERR if the release client operation was sucessful, < 0 if not.
  If return code  is  QMI_SERVICE_ERR, then the qmi_err_code will be valid
  and will indicate which QMI error occurred.

@note

  - Dependencies
    - qmi_connection_init() must be called for the associated port first.

  - Side Effects
    - Talks to modem processor
*/
/*=========================================================================*/

int
qmi_atcop_srvc_release_client
(
  int      user_handle,
  int      *qmi_err_code
)
{
  int rc;
  rc = qmi_service_release (user_handle, qmi_err_code);
  return rc;
}

/*===========================================================================
  FUNCTION  qmi_atcop_reg_at_command_fwd_req
===========================================================================*/
/*!
@brief
  This command is used by the client to register any AT commands that need to
  be forwarded to it from the modem.

@return
  QMI_NO_ERR if the operation was sucessful, < 0 if not. If return code is
  QMI_SERVICE_ERR,then the qmi_err_code will be valid and will indicate which
  QMI error occurred.

@note

  - Dependencies
    - qmi_atcop_srvc_init_client() must be called before calling this.

  - Side Effects
    -
*/
/*=========================================================================*/
int
qmi_atcop_reg_at_command_fwd_req
(
  int                               client_handle,
  qmi_atcop_at_cmd_fwd_req_type     *cmd_fwd_req,
  int                               *qmi_err_code
)
{
  unsigned char     msg[QMI_ATCOP_STD_MSG_SIZE], *tmp_msg_ptr;
  int               msg_size, rc;
  unsigned char     param_buf[QMI_ATCOP_MAX_PARAM_BUF_SIZE];
  int               param_buf_size = QMI_ATCOP_MAX_PARAM_BUF_SIZE;

  if (!cmd_fwd_req || cmd_fwd_req->num_of_cmds <= 0 || cmd_fwd_req->num_of_cmds > QMI_ATCOP_MAX_AT_CMDS_REG)
  {
     QMI_ERR_MSG_0 ("qmi_atcop_reg_at_command_fwd_req: Bad Input Received.\n");
     return QMI_INTERNAL_ERR;
  }

  rc = qmi_atcop_reg_at_format_cmd_list_tlv(cmd_fwd_req,param_buf,&param_buf_size);
  if (rc < 0)
  {
    return rc;
  }

  /*Prepare the Final TLV*/
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);
  msg_size = QMI_SRVC_PDU_SIZE(QMI_ATCOP_STD_MSG_SIZE);

  if ((rc = qmi_util_write_std_tlv (&tmp_msg_ptr,
                                       &msg_size,
                                       QMI_ATCOP_AT_CMD_LIST_TLV,
                                       (unsigned long)(QMI_ATCOP_MAX_PARAM_BUF_SIZE - param_buf_size),
                                       (void *) param_buf)) < 0)
  {
    return rc;
  }

  /*Send Synchronous message*/
  rc = qmi_service_send_msg_sync (client_handle,
                                  QMI_ATCOP_SERVICE,
                                  QMI_ATCOP_REGISTER_AT_CMD_FWD_MSD_ID,
                                  QMI_SRVC_PDU_PTR(msg),
                                  (int)QMI_SRVC_PDU_SIZE(QMI_ATCOP_STD_MSG_SIZE) - msg_size,
                                  msg,
                                  &msg_size,
                                  QMI_ATCOP_STD_MSG_SIZE,
                                  QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                  qmi_err_code);

  return rc;
}

/*===========================================================================
  FUNCTION  qmi_atcop_fwd_at_cmd_resp
===========================================================================*/
/*!
@brief
  This command is used by the client to send the response to an AT cmd
  previously forwarded to the client from the modem

@return
  QMI_NO_ERR if the operation was sucessful, < 0 if not. If return code is
  QMI_SERVICE_ERR,then the qmi_err_code will be valid and will indicate which
  QMI error occurred.
@note

  - Dependencies
    - qmi_atcop_srvc_init_client() must be called before calling this.

  - Side Effects
    -
*/
/*=========================================================================*/
int
qmi_atcop_fwd_at_cmd_resp
(
  int                               client_handle,
  qmi_atcop_fwd_resp_at_resp_type   *at_resp,
  int                               *qmi_err_code
)
{
  unsigned char     msg[QMI_ATCOP_STD_MSG_SIZE], *tmp_msg_ptr;
  int               msg_size, rc, at_resp_len;
  unsigned long     param_buf_len;
  unsigned char     param_buf[QMI_ATCOP_MAX_PARAM_BUF_SIZE];
  unsigned char     *param_ptr = param_buf;

  if (!at_resp)
  {
     QMI_ERR_MSG_0 ("qmi_atcop_fwd_at_cmd_resp: Bad Input Received.\n");
     return QMI_INTERNAL_ERR;
  }
  /*calculate the length of the at response*/
  if (at_resp->at_resp)
  {
    at_resp_len = (int)strlen((const char *)at_resp->at_resp);
  }
  else
  {
    at_resp_len = 0;
  }

  if(at_resp_len > QMI_ATCOP_AT_RESP_MAX_LEN)
  {
     QMI_ERR_MSG_1 ("qmi_atcop_fwd_at_cmd_resp: Cannot Handle AT Response length greater than %d .\n", QMI_ATCOP_AT_RESP_MAX_LEN);
     return QMI_INTERNAL_ERR;
  }

  if(at_resp->response > QMI_ATCOP_RESP_TYPE_MAX)
  {
     QMI_ERR_MSG_1 ("qmi_atcop_fwd_at_cmd_resp: Cannot Handle AT Response type greater than %d .\n", QMI_ATCOP_RESP_TYPE_MAX);
     return QMI_INTERNAL_ERR;
  }

  /*Length of the Value part of TLV*/
  param_buf_len = (unsigned long)(8 + at_resp_len);//Update this length,
                                                   //when TLVs change or get added
                                                   // 8 corresponds to len(handle + result + resp + resp_len)

  if ( param_buf_len > QMI_ATCOP_MAX_PARAM_BUF_SIZE)
  {
    QMI_ERR_MSG_1 ("qmi_atcop_fwd_at_cmd_resp: Will Overflow ParamBuf, %d .\n", QMI_ATCOP_MAX_PARAM_BUF_SIZE);
    return QMI_INTERNAL_ERR;
  }

  /*prepare the TLV*/
  WRITE_32_BIT_VAL(param_ptr,at_resp->at_hndl);
  WRITE_8_BIT_VAL(param_ptr,at_resp->result);
  WRITE_8_BIT_VAL(param_ptr,at_resp->response);
  WRITE_16_BIT_VAL(param_ptr,at_resp_len);

  if (at_resp_len > 0)
  {
    memcpy(param_ptr, at_resp->at_resp, (size_t)at_resp_len);
  }

  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);
  msg_size = QMI_SRVC_PDU_SIZE(QMI_ATCOP_STD_MSG_SIZE);

  if ((rc = qmi_util_write_std_tlv (&tmp_msg_ptr,
                                       &msg_size,
                                       QMI_ATCOP_AT_RESP_INFO_TLV,
                                       param_buf_len,
                                       (void *)param_buf)) < 0)
  {
    return rc;
  }

  /*Send Synchronous message*/
  rc = qmi_service_send_msg_sync (client_handle,
                                  QMI_ATCOP_SERVICE,
                                  QMI_ATCOP_REGISTER_AT_CMD_FWD_RESP_MSG_ID,
                                  QMI_SRVC_PDU_PTR(msg),
                                  (int)QMI_SRVC_PDU_SIZE(QMI_ATCOP_STD_MSG_SIZE) - msg_size,
                                  msg,
                                  &msg_size,
                                  QMI_ATCOP_STD_MSG_SIZE,
                                  QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                  qmi_err_code);

  return rc;
}

/*===========================================================================
  FUNCTION  qmi_atcop_fwd_at_urc_req
===========================================================================*/
/*!
@brief
  This command is used to send unsolicited response codes to the modem.

@return
  QMI_NO_ERR if the operation was sucessful, < 0 if not. If return code is
  QMI_SERVICE_ERR,then the qmi_err_code will be valid and will indicate which
  QMI error occurred.

@note

  - Dependencies
    - qmi_atcop_srvc_init_client() must be called before calling this.

  - Side Effects
    -
*/
/*=========================================================================*/
int
qmi_atcop_fwd_at_urc_req
(
  int                               client_handle,
  qmi_atcop_at_fwd_urc_req_type     *urc_fwd_req,
  int                               *qmi_err_code
)
{
  unsigned char     msg[QMI_ATCOP_STD_MSG_SIZE], *tmp_msg_ptr;
  int               msg_size, rc;
  size_t            at_urc_len;
  unsigned char     param_buf[QMI_ATCOP_MAX_PARAM_BUF_SIZE];
  unsigned long     param_buf_size = QMI_ATCOP_MAX_PARAM_BUF_SIZE;
  unsigned char     *param_ptr = param_buf;

  if (!urc_fwd_req || !qmi_err_code)
  {
     QMI_ERR_MSG_0 ("qmi_atcop_fwd_at_urc: Bad Input Received.\n");
     return QMI_INTERNAL_ERR;
  }

  if (urc_fwd_req->at_urc)
  {
    at_urc_len = strlen((const char *)urc_fwd_req->at_urc);
  }
  else
  {
    at_urc_len = 0;
  }

  if(at_urc_len > QMI_ATCOP_AT_URC_MAX_LEN)
  {
     QMI_ERR_MSG_0 ("qmi_atcop_fwd_at_urc:Cannot handle the input URC length");
     return QMI_INTERNAL_ERR;
  }

  if(urc_fwd_req->status > QMI_ATCOP_AT_URC_STATUS_MAX)
  {
     QMI_ERR_MSG_0 ("qmi_atcop_fwd_at_urc:Cannot handle URC status type");
     return QMI_INTERNAL_ERR;
  }

  param_buf_size = 2 + at_urc_len;/*one byte for length of urc and one byte for status type of urc*/

  if ( param_buf_size > QMI_ATCOP_MAX_PARAM_BUF_SIZE)
  {
    QMI_ERR_MSG_1 ("qmi_atcop_fwd_at_urc: Will Overflow ParamBuf, %d .\n",
                      QMI_ATCOP_MAX_PARAM_BUF_SIZE);
    return QMI_INTERNAL_ERR;
  }

  /*prepare the TLV*/
  WRITE_8_BIT_VAL(param_ptr,urc_fwd_req->status);
  WRITE_8_BIT_VAL(param_ptr,at_urc_len);
  memcpy(param_ptr, urc_fwd_req->at_urc, at_urc_len);

  /*Prepare the Final TLV*/
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);
  msg_size = QMI_SRVC_PDU_SIZE(QMI_ATCOP_STD_MSG_SIZE);

  if ((rc = qmi_util_write_std_tlv (&tmp_msg_ptr,
                                    &msg_size,
                                    QMI_ATCOP_AT_FWD_URC_LEN,
                                    param_buf_size,
                                    (void *) param_buf)) < 0)
  {
    return rc;
  }

  /*Send Synchronous message*/
  rc = qmi_service_send_msg_sync (client_handle,
                                  QMI_ATCOP_SERVICE,
                                  QMI_ATCOP_AT_CMD_URC_FWD_REQ_MSG_ID,
                                  QMI_SRVC_PDU_PTR(msg),
                                  (int)QMI_SRVC_PDU_SIZE(QMI_ATCOP_STD_MSG_SIZE) - msg_size,
                                  msg,
                                  &msg_size,
                                  QMI_ATCOP_STD_MSG_SIZE,
                                  QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                  qmi_err_code);

  return rc;
}
