/******************************************************************************
  @file    qmi_eap_srvc.c
  @brief   The QMI EAP service layer.

  DESCRIPTION
  QMI EAP service routines.

  INITIALIZATION AND SEQUENCING REQUIREMENTS
  qmi_eap_srvc_init_client() needs to be called before sending or receiving of any
  QoS service messages

  ---------------------------------------------------------------------------
  Copyright (c) 2008-2014 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

#include <stdlib.h>
#include <memory.h>
#include "qmi_i.h"
#include "qmi_service.h"
#include "qmi_eap_srvc.h"
#include "qmi_util.h"

#define QMI_EAP_STD_MSG_SIZE   QMI_MAX_STD_MSG_SIZE
#define QMI_EAP_MAX_MSG_SIZE   QMI_MAX_MSG_SIZE


/*Indication Message Ids*/

#define QMI_EAP_AUTH_SESSION_RESULT_IND_MSG_ID                0x0022
#define QMI_EAP_AKA_RESULT_IND_MSG_ID                         0x0026
#define QMI_EAP_NOTIF_CODE_IND_MSG_ID                         0x0029
#define QMI_EAP_ERROR_CODE_IND_MSG_ID                         0x002A
#define QMI_EAP_AUTH_REJ_IND_MSG_ID                           0x002B

/*MSG IDS*/

#define QMI_EAP_AUTH_RESET_REQ_MSG_ID                         0x0000
#define QMI_EAP_AUTH_INDICATION_REGISTER_MSG_ID               0x0003
#define QMI_EAP_AUTH_START_EAP_SESSION_REQ_MSG_ID             0x0020
#define QMI_EAP_AUTH_SEND_PACKET_REQ_MSG_ID                   0x0021
#define QMI_EAP_AUTH_END_EAP_SESSION_REQ_MSG_ID               0x0024
#define QMI_EAP_AUTH_GET_SESSION_KEYS_REQ_MSG_ID              0x0023
#define QMI_EAP_AUTH_INITIATE_AKA_ALGORITHM_MSG_ID            0x0025
#define QMI_EAP_AUTH_BIND_SUBSCRIPTION_MSG_ID                 0x0027
#define QMI_EAP_AUTH_GET_BIND_SUBSCRIPTION_MSG_ID             0x0028


/*Req Resp TLV IDS*/
#define QMI_EAP_METHOD_MASK_REQ_TLV_ID              0x10
#define QMI_EAP_AUTH_START_EAP_USER_ID_TLV_ID       0x11
#define QMI_EAP_AUTH_START_EAP_META_ID_TLV_ID       0x12
#define QMI_EAP_AUTH_START_EAP_SIM_AKA_ALGO_TLV_ID  0x13

#define QMI_EAP_SEND_PKT_REQ_TLV_ID                 0x01
#define QMI_EAP_SEND_PKT_RESP_TLV_ID                0x01
#define QMI_EAP_GET_SESSION_KEYS_RESP_TLV_ID        0x01
#define QMI_EAP_SESSTION_RESULT_IND_TLV_ID          0x01
#define QMI_EAP_AKA_ALGORITHM_VERSION_TLV_ID        0x01
#define QMI_EAP_AKA_V1_OR_V2_AUTH_PARAMS_TLV_ID     0x10
#define QMI_EAP_AKA_HANDLE_TLV_ID                   0x01
#define QMI_EAP_AKA_RESULT_IND_TLV_ID               0x01
#define QMI_EAP_AKA_V1_OR_V2_AUTH_IND_TLV_ID        0x10
#define QMI_EAP_BIND_SUBSCRIPTION_TLV_ID            0x01
#define QMI_EAP_GET_BIND_SUBSCRIPTION_TLV_ID        0x10

#define QMI_EAP_SESSION_ERR_NOTIF_TLV_ID            0x10
#define QMI_EAP_SESSION_ERR_CODE_TLV_ID             0x11
#define QMI_EAP_AUTH_REJ_NOTIF_TLV_ID               0x12
#define QMI_EAP_NOTIF_CODE_IND_TLV_ID               0x01
#define QMI_EAP_ERROR_CODE_IND_TLV_ID               0x01

static int eap_service_initialized = FALSE;

/*===========================================================================
  FUNCTION  qmi_eap_srvc_indication_cb
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
qmi_eap_srvc_indication_cb
(
  int                   user_handle,
  qmi_service_id_type   service_id,
  unsigned long         msg_id,
  void                  *user_ind_msg_hdlr,
  void                  *user_ind_msg_hdlr_user_data,
  unsigned char         *rx_msg_buf,
  int                   rx_msg_len
)
{
  qmi_eap_indication_id_type      ind_id;
  qmi_eap_indication_data_type    ind_data;
  qmi_eap_indication_hdlr_type    user_ind_hdlr;

  unsigned long type;
  unsigned long length;
  unsigned char *value_ptr;

  /* Make sure that the user indication handler isn't NULL */
  if (user_ind_msg_hdlr == NULL)
  {
    return;
  }

  memset(&ind_data, 0, sizeof(ind_data));

  /* Get properly cast pointer to user indication handler */
  /*lint -e{611} */
  user_ind_hdlr = (qmi_eap_indication_hdlr_type) user_ind_msg_hdlr;

  switch (msg_id)
  {

    case QMI_EAP_AUTH_SESSION_RESULT_IND_MSG_ID:
      {
        unsigned char temp_8bit;
        ind_id = QMI_EAP_SRVC_SESSION_RESULT_IND_MSG;

        if (qmi_util_read_std_tlv (&rx_msg_buf,
                                   &rx_msg_len,
                                   &type,
                                   &length,
                                   &value_ptr) < 0)
        {
          return ;
        }

        if (type != QMI_EAP_SESSTION_RESULT_IND_TLV_ID)
        {
          QMI_ERR_MSG_1 ("qmi_eap_srvc_indication_cb::Invalid TLV received %lx \n ",type);
          return;
        }

        READ_8_BIT_VAL (value_ptr,temp_8bit);
        ind_data.auth_result = (qmi_eap_auth_result_ind_param_type) temp_8bit;


      }
      break;

    case QMI_EAP_AKA_RESULT_IND_MSG_ID:
      {
        ind_id = QMI_EAP_AKA_RESULT_IND_MSG;

        while(rx_msg_len > 0)
        {
          if (qmi_util_read_std_tlv (&rx_msg_buf,
                                     &rx_msg_len,
                                     &type,
                                     &length,
                                     &value_ptr) < 0)
          {
            return;
          }

          switch(type)
          {
            case QMI_EAP_AKA_RESULT_IND_TLV_ID:
              {
                READ_32_BIT_VAL (value_ptr,ind_data.aka_result.aka_handle);
                READ_8_BIT_VAL (value_ptr,ind_data.aka_result.status);
              }
              break;

            case QMI_EAP_AKA_V1_OR_V2_AUTH_IND_TLV_ID:
              {
                size_t copy_len = 0;
                int parse_success = FALSE;

                do
                {
                   if (length < 2) // at least two bytes for read digest_len and aka_data_len
                   {
                      break;
                   }

                   READ_8_BIT_VAL (value_ptr,
                                   ind_data.aka_result.v1_or_v2_auth_params.digest_len);

                   if (length < 2+(unsigned long) ind_data.aka_result.v1_or_v2_auth_params.digest_len)
                   {
                      break;
                   }

                   copy_len = (size_t)(ind_data.aka_result.v1_or_v2_auth_params.digest_len >
                     QMI_EAP_MAX_STR_LEN ? QMI_EAP_MAX_STR_LEN :
                     ind_data.aka_result.v1_or_v2_auth_params.digest_len);

                   if (copy_len !=
                       ind_data.aka_result.v1_or_v2_auth_params.digest_len)
                   {
                     QMI_DEBUG_MSG_1 ("qmi_eap_srvc_indication_cb: "
                                      "modem provided digest length exceeds the "
                                      "max supported, truncating it to %d ",
                                      QMI_EAP_MAX_STR_LEN);
                   }
                   memcpy(ind_data.aka_result.v1_or_v2_auth_params.digest,
                          value_ptr,
                          copy_len);

                   value_ptr += ind_data.aka_result.v1_or_v2_auth_params.digest_len;

                   READ_8_BIT_VAL (value_ptr,
                                   ind_data.aka_result.v1_or_v2_auth_params.aka_data_len);

                   copy_len = (size_t)(ind_data.aka_result.v1_or_v2_auth_params.aka_data_len >
                     QMI_EAP_MAX_STR_LEN ? QMI_EAP_MAX_STR_LEN :
                     ind_data.aka_result.v1_or_v2_auth_params.aka_data_len);

                   if (copy_len !=
                       ind_data.aka_result.v1_or_v2_auth_params.aka_data_len)
                   {
                     QMI_DEBUG_MSG_1 ("qmi_eap_srvc_indication_cb: "
                                      "modem provided aka data length exceeds the"
                                      "max supported, truncating it to %d ",
                                      QMI_EAP_MAX_STR_LEN);
                   }

                   if (length < 2+ (unsigned long) (ind_data.aka_result.v1_or_v2_auth_params.digest_len+copy_len))
                   {
                      break;
                   }

                   memcpy(ind_data.aka_result.v1_or_v2_auth_params.aka_data,
                          value_ptr,
                          copy_len);

                   parse_success = TRUE;
                } while (FALSE);

                if (parse_success)
                {
                   ind_data.aka_result.param_mask |= QMI_EAP_AKA_V1_OR_V2_AUTH_RESP_PARAMS;
                }
                else
                {
                   QMI_ERR_MSG_0("qmi_eap_srvc_indication_cb: "
                                 "length read from std tlv could not be parsed"
                                 "for QMI_EAP_AKA_V1_OR_V2_AUTH_IND_TLV_ID");
                }

              }
              break;

            default:
              {
                QMI_ERR_MSG_1 ("qmi_eap_srvc_indication_cb: "
                               "unknown indication TLV type = %x",(unsigned int)type);
              }
              break;
          }
        }/*while*/
      }
      break;

    case QMI_EAP_NOTIF_CODE_IND_MSG_ID:
      {
        ind_id = QMI_EAP_NOTIF_CODE_IND_MSG;

        if (qmi_util_read_std_tlv (&rx_msg_buf,
                                   &rx_msg_len,
                                   &type,
                                   &length,
                                   &value_ptr) < 0)
        {
          return ;
        }

        if (type != QMI_EAP_NOTIF_CODE_IND_TLV_ID)
        {
          QMI_ERR_MSG_1 ("qmi_eap_srvc_indication_cb::Invalid TLV received %lx \n ",type);
          return;
        }

        READ_16_BIT_VAL (value_ptr,ind_data.eap_notif_code);
      }
      break;

    case QMI_EAP_ERROR_CODE_IND_MSG_ID:
      {
        ind_id = QMI_EAP_ERROR_CODE_IND_MSG;

        if (qmi_util_read_std_tlv (&rx_msg_buf,
                                   &rx_msg_len,
                                   &type,
                                   &length,
                                   &value_ptr) < 0)
        {
          return ;
        }

        if (type != QMI_EAP_ERROR_CODE_IND_TLV_ID)
        {
          QMI_ERR_MSG_1 ("qmi_eap_srvc_indication_cb::Invalid TLV received %lx \n ",type);
          return;
        }

        READ_16_BIT_VAL (value_ptr,ind_data.eap_err_code);
      }
      break;

    case QMI_EAP_AUTH_REJ_IND_MSG_ID:
      {
        ind_id = QMI_EAP_AUTH_REJ_IND_MSG;
      }
      break;

    default:
      return;
  }

  /* Call user registered handler */
  user_ind_hdlr (user_handle,
                 service_id,
                 user_ind_msg_hdlr_user_data,
                 ind_id,
                 &ind_data);
  return;
}
/*===========================================================================
  FUNCTION  qmi_eap_srvc_async_cb
===========================================================================*/
/*!
@brief
  This is the callback function that will be called by the generic
  services layer to report asynchronous responses.  This function will
  process the asynchronous TLV's and then call the user registered
  functions with the reply data

@return
  0 if abort operation was sucessful, < 0 if not.  If return code is
  QMI_INTERNAL_ERR, then the qmi_err_code will be valid and will
  indicate which QMI error occurred.

@note

  - Dependencies
    - qmi_connection_init() must be called for the associated port first.

  - Side Effects
    - Talks to modem processor
*/
/*=========================================================================*/
static void
qmi_eap_srvc_async_cb
(
  int                     user_handle,
  qmi_service_id_type     service_id,
  unsigned long           msg_id,
  int                     rsp_rc,
  int                     qmi_err_code,
  unsigned char           *reply_msg_data,
  int                     reply_msg_size,
  void                    *srvc_async_cb_data,
  void                    *user_async_cb_fn,
  void                    *user_async_cb_data
)
{
  qmi_eap_user_async_cb_type user_cb;
  qmi_eap_async_rsp_id_type    rsp_id;
  qmi_eap_async_rsp_data_type  rsp_data;

  (void) srvc_async_cb_data;

  /* Now set get response data based on which type of response it was */
  switch (msg_id)
  {
    case QMI_EAP_AUTH_SEND_PACKET_REQ_MSG_ID:
      {
        /* set response ID, and call function to handle start network
        ** return data
        */
        rsp_id = QMI_EAP_SEND_EAP_PKT_RSP_ID;
        if (rsp_rc == QMI_NO_ERR)
        {
          unsigned long type;
          unsigned long length;
          unsigned char *value_ptr;

          while (reply_msg_size > 0)
          {
            if (qmi_util_read_std_tlv (&reply_msg_data,
                                          &reply_msg_size,
                                          &type,
                                          &length,
                                          &value_ptr) < 0)
            {
              return;
            }

            switch (type)
            {
              case QMI_EAP_SEND_PKT_RESP_TLV_ID:
                {
                  rsp_data.eap_send_pkt_resp.resp_data = (void *) value_ptr;
                  rsp_data.eap_send_pkt_resp.length    =  length;
                }
                break;
              default:
                {
                  QMI_ERR_MSG_1 ("qmi_eap_srvc_async_cb: "
                                 "unknown response TLV type = %x",(unsigned int)type);
                }
                break;
            }
          }
        }
      }/*Case St-EAP_SEND_PKT_MSG_ID*/
      break;

    default:
      {
        QMI_ERR_MSG_1 ("qmi_eap_srvc_async_cb: "
                       "Discarding unknown async reply type received = %d\n",(int)msg_id);
        return;
      }


  }/*Switch Msg ID*/


  /* If the reply indicates that the command was aborted, don't
  ** call the user callback
  */
  if ((rsp_rc == QMI_SERVICE_ERR) &&
      (qmi_err_code == QMI_SERVICE_ERR_ABORTED))
  {
    QMI_DEBUG_MSG_2 ("Discarding aborted reply, msg_id=%x, user=%x",(unsigned int) msg_id,
                     (unsigned int) user_handle);
  }

  else
  {
    /*lint -e{611} */
    user_cb = (qmi_eap_user_async_cb_type) user_async_cb_fn;

    /* Call the user callback */
    user_cb (user_handle,
             service_id,
             rsp_rc,
             qmi_err_code,
             user_async_cb_data,
             rsp_id,
             &rsp_data);
  }

 return;
}

/*===========================================================================
  FUNCTION  qmi_eap_srvc_init
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
int qmi_eap_srvc_init (void)
{
  int  rc = QMI_NO_ERR;
  if (!eap_service_initialized)
  {

    rc = qmi_service_set_srvc_functions (QMI_EAP_SERVICE,
                                         qmi_eap_srvc_indication_cb);
    if (rc != QMI_NO_ERR)
    {
      QMI_ERR_MSG_1("qmi_eap_srvc_init: set srvc functions returns err=%d\n",rc);
    }
    else
    {
      QMI_DEBUG_MSG_0("qmi_eap_srvc_init: EAP successfully initialized");
      eap_service_initialized = TRUE;
    }
  }
  else
  {
    QMI_ERR_MSG_0 ("qmi_eap_srvc_init: Init failed, EAP already initialized");
  }
  return rc;
}


/*===========================================================================
  FUNCTION  qmi_eap_srvc_release
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
int qmi_eap_srvc_release (void)
{
  int  rc = QMI_NO_ERR;
  if (eap_service_initialized)
  {

    rc = qmi_service_set_srvc_functions (QMI_EAP_SERVICE,NULL);
    if (rc != QMI_NO_ERR)
    {
      QMI_ERR_MSG_1("qmi_eap_srvc_release: set srvc functions returns err=%d\n",rc);
    }
    else
    {
      QMI_DEBUG_MSG_0("qmi_eap_srvc_release: EAP successfully released");
      eap_service_initialized = FALSE;
    }
  }
  else
  {
    QMI_ERR_MSG_0 ("qmi_eap_srvc_release: Release failed, EAP not initialized");
  }
  return rc;
}


/*===========================================================================
  FUNCTION  qmi_eap_srvc_init_client
===========================================================================*/
/*!
@brief
  This function is called to initialize the EAP service.  This function
  must be called prior to calling any other EAP service functions.
  For the time being, the indication handler callback and user data
  should be set to NULL until this is implemented.  Also note that this
  function may be called multiple times to allow for multiple, independent
  clients.

@return
  Returns a user handle if success, This user handle can be used for other
  operations using this service.

@note

  - Dependencies
    - qmi_connection_init() must be called for the associated port first.

  - Side Effects
    - Talks to modem processor
*/
/*=========================================================================*/

qmi_client_handle_type
qmi_eap_srvc_init_client
(
  const char                    *dev_id,
  qmi_eap_indication_hdlr_type  user_ind_msg_hdlr,
  void                          *user_ind_msg_hdlr_user_data,
  int                           *qmi_err_code
)
{
  qmi_client_handle_type client_handle;
  qmi_connection_id_type conn_id;


  if ((conn_id = QMI_PLATFORM_DEV_NAME_TO_CONN_ID(dev_id)) == QMI_CONN_ID_INVALID)
  {
    return QMI_INTERNAL_ERR;
  }


  /* Call common service layer initialization function */
  /*lint -e{611} */
  client_handle =  qmi_service_init (conn_id,
                                   QMI_EAP_SERVICE,
                                   (void *) user_ind_msg_hdlr,
                                   user_ind_msg_hdlr_user_data,
                                   qmi_err_code);



  return client_handle;
}



/*===========================================================================
  FUNCTION  qmi_eap_srvc_release_client
===========================================================================*/
/*!
@brief
  This function is called to release a client created by the
  qmi_eap_srvc_init_client() function.  This function should be called
  for any client created when terminating a client process, especially
  if the modem processor is not reset.  The modem side QMI server has
  a limited number of clients that it will allocate, and if they are not
  released, we will run out.

@return
  0 if abort operation was sucessful, < 0 if not.  If return code is
  QMI_INTERNAL_ERR, then the qmi_err_code will be valid and will
  indicate which QMI error occurred.

@note

  - Dependencies
    - qmi_connection_init() must be called for the associated port first.

  - Side Effects
    - Talks to modem processor
*/
/*=========================================================================*/

int
qmi_eap_srvc_release_client
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
  FUNCTION  qmi_eap_reset
===========================================================================*/
/*!
@brief
  Resets EAP service. This command resets the state of the requesting
  control point. It clears all the resources that were set up for the EAP
  session started by the control point.

@return
  If return code < 0, the operation failed.  In the failure case, if the
  return code is QMI_SERVICE_ERR, then the qmi_err_code value will give
  you the QMI error reason.  Otherwise, qmi_err_code will have meaningless
  data.

@note

  - Dependencies
    - qmi_connection_init() must be called before calling this.

  - Side Effects
    - Resets EAP service
*/
/*=========================================================================*/
int
qmi_eap_reset
(
  int                         user_handle,
  int                         *qmi_err_code
)
{
  unsigned char     msg[QMI_EAP_STD_MSG_SIZE];
  int               msg_size;
  int rc;

  rc = qmi_service_send_msg_sync (user_handle,
                                  QMI_EAP_SERVICE,
                                  QMI_EAP_AUTH_RESET_REQ_MSG_ID,
                                  QMI_SRVC_PDU_PTR(msg),
                                  0,
                                  msg,
                                  &msg_size,
                                  QMI_EAP_STD_MSG_SIZE,
                                  QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                  qmi_err_code);

  return rc;
}

/*===========================================================================
  FUNCTION  qmi_eap_auth_start_eap_session
===========================================================================*/
/*!
@brief
    This function is used to start the EAP session. This takes an optional
    eap methods mask parameter. These are the EAP methods which can be
     supported.

@return
  0 if abort operation was sucessful, < 0 if not.  If return code is
  QMI_INTERNAL_ERR, then the qmi_err_code will be valid and will
  indicate which QMI error occurred.

@note
    This function executes synchronously, there is not currently an
    asynchronous option for this functionality.

  - Dependencies
    - qmi_connection_init() must be called for the associated port first.

  - Side Effects
*/
/*=========================================================================*/
int
qmi_eap_auth_start_eap_session
(
  int                           user_handle,
  unsigned long                 eap_method_mask, /* This parameter is optional,
                                                  set this to QMI_EAP_METHOD_MASK_UNSET if not using
                                                  the method_mask*/
  int                           *qmi_err_code
)
{
  unsigned char     msg[QMI_EAP_STD_MSG_SIZE];
  int               msg_size;
  unsigned char    *tmp_msg_ptr;
  int rc;

  /* Set tmp_msg_ptr to beginning of message-specific TLV portion of
  ** message buffer
  */
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  /* Set the message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_EAP_STD_MSG_SIZE);

  if (eap_method_mask)
  {
    if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                                   &msg_size,
                                   QMI_EAP_METHOD_MASK_REQ_TLV_ID,
                                   4,
                                   (void *)&eap_method_mask) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  rc = qmi_service_send_msg_sync (user_handle,
                                  QMI_EAP_SERVICE,
                                  QMI_EAP_AUTH_START_EAP_SESSION_REQ_MSG_ID,
                                  QMI_SRVC_PDU_PTR(msg),
                                  (int)QMI_SRVC_PDU_SIZE(QMI_EAP_STD_MSG_SIZE) - msg_size,
                                  msg,
                                  &msg_size,
                                  QMI_EAP_STD_MSG_SIZE,
                                  QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                  qmi_err_code);


  return rc;

}
/*===========================================================================
  FUNCTION  qmi_eap_auth_start_eap_session_ex
===========================================================================*/
/*!
@brief
    This function is used to start the EAP session. It takes multiple optional
    EAP paramters.

@return
  0 if abort operation was sucessful, < 0 if not.  If return code is
  QMI_INTERNAL_ERR, then the qmi_err_code will be valid and will
  indicate which QMI error occurred.

@note
    This function executes synchronously, there is not currently an
    asynchronous option for this functionality.

  - Dependencies
    - qmi_connection_init() must be called for the associated port first.

  - Side Effects
*/
/*=========================================================================*/
int
qmi_eap_auth_start_eap_session_ex
(
  int                                   user_handle,
  qmi_eap_auth_start_eap_params_type   *eap_auth_start,
  int                                  *qmi_err_code
)
{
  unsigned char     msg[QMI_EAP_STD_MSG_SIZE];
  int               msg_size;
  unsigned char    *tmp_msg_ptr;
  unsigned int      i;
  unsigned char    *tmp_buf_id_ptr = NULL;
  unsigned char    *orig_tmp_buf_ptr_user_id = NULL;
  unsigned char    *orig_tmp_buf_ptr_meta_id = NULL;
  int rc = QMI_INTERNAL_ERR;

  /* validate input parameter */
  if(NULL == eap_auth_start || NULL== qmi_err_code)
  {
    QMI_ERR_MSG_0("qmi_eap_auth_start_eap_session Invalid input param\n");
    return QMI_INTERNAL_ERR;
  }
  /* Set tmp_msg_ptr to beginning of message-specific TLV portion of
  ** message buffer
  */
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  /* Set the message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_EAP_STD_MSG_SIZE);


  do
  {
    if ( QMI_EAP_AUTH_START_EAP_METHOD_MASK_PARAM & eap_auth_start->params_mask)
    {
      if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                                  &msg_size,
                                  QMI_EAP_METHOD_MASK_REQ_TLV_ID,
                                  4,
                                  (void *)&(eap_auth_start->eap_method_mask)) < 0)
      {
        break;
      }
    }

    if ( QMI_EAP_AUTH_START_EAP_USER_ID_PARAM & eap_auth_start->params_mask)
    {
      /* user_id_len == 0 regardless user_id's NULLness is OK,
         user_id's NULLness should be checked when user_id_len != 0 */
      if( 0 != eap_auth_start->user_id_len &&
         NULL == eap_auth_start->user_id
         )
      {
        QMI_ERR_MSG_2("qmi_eap_auth_start_eap_session: "
                      "invalid input param user_id_len: %d user_id: 0x%x",
                      eap_auth_start->user_id_len, eap_auth_start->user_id);
        break;
      }

      /* allocate memory */
      /* +1 is for field user_id_len */
      tmp_buf_id_ptr = (unsigned char*)malloc((size_t)eap_auth_start->user_id_len + 1);

      if(NULL == tmp_buf_id_ptr)
      {
        QMI_ERR_MSG_0 ("Allocate memory for start_eap_session user_id failed\n");
        break;
      }

      /* save the memory location */
      orig_tmp_buf_ptr_user_id = tmp_buf_id_ptr;

      /* write user_id_len */
      WRITE_8_BIT_VAL(tmp_buf_id_ptr, eap_auth_start->user_id_len);

      /* write user_id */
      memcpy(tmp_buf_id_ptr, eap_auth_start->user_id, eap_auth_start->user_id_len);

      if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                                  &msg_size,
                                  QMI_EAP_AUTH_START_EAP_USER_ID_TLV_ID,
                                  (unsigned long)eap_auth_start->user_id_len + 1,
                                  (void *)orig_tmp_buf_ptr_user_id) < 0)
      {
        break;
      }
    }

    if ( QMI_EAP_AUTH_START_EAP_META_ID_PARAM & eap_auth_start->params_mask)
    {
      /* eap_meta_id_len == 0 regardless eap_meta_id's NULLness is OK,
         eap_meta_id's NULLness should be checked when eap_meta_id_len != 0 */
      if( 0 != eap_auth_start->eap_meta_id_len &&
         NULL == eap_auth_start->eap_meta_id
         )
      {
        QMI_ERR_MSG_2("qmi_eap_auth_start_eap_session: "
                      "invalid input param meta_id_len: %d meta_id: 0x%x",
                      eap_auth_start->eap_meta_id_len, eap_auth_start->eap_meta_id);
        break;
      }
      /* allocate memory */
      /* +1 is for field eap_meta_id_len */
      tmp_buf_id_ptr = (unsigned char *)malloc((size_t)eap_auth_start->eap_meta_id_len + 1);

      if (NULL == tmp_buf_id_ptr)
      {
        QMI_ERR_MSG_0 ("Allocate memory for start_eap_session meta_id failed\n");
        break;
      }

      /* save the memory location */
      orig_tmp_buf_ptr_meta_id = tmp_buf_id_ptr;

      /* write meta_id_len */
      WRITE_8_BIT_VAL(tmp_buf_id_ptr, eap_auth_start->eap_meta_id_len);

      /* write meta_id */
      memcpy(tmp_buf_id_ptr, eap_auth_start->eap_meta_id, eap_auth_start->eap_meta_id_len);
      if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                                  &msg_size,
                                  QMI_EAP_AUTH_START_EAP_META_ID_TLV_ID,
                                  (unsigned long)eap_auth_start->eap_meta_id_len + 1,
                                  (void *)orig_tmp_buf_ptr_meta_id) < 0)
      {
        break;
      }
    }

    if ( QMI_EAP_AUTH_START_EAP_SIM_AKA_ALGO_PARAM & eap_auth_start->params_mask)
    {
      if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                                  &msg_size,
                                  QMI_EAP_AUTH_START_EAP_SIM_AKA_ALGO_TLV_ID,
                                  4,
                                  (void *)&(eap_auth_start->eap_sim_aka_algo)) < 0)
      {
        break;
      }
    }

    rc = qmi_service_send_msg_sync (user_handle,
                                    QMI_EAP_SERVICE,
                                    QMI_EAP_AUTH_START_EAP_SESSION_REQ_MSG_ID,
                                    QMI_SRVC_PDU_PTR(msg),
                                    (int)QMI_SRVC_PDU_SIZE(QMI_EAP_STD_MSG_SIZE) - msg_size,
                                    msg,
                                    &msg_size,
                                    QMI_EAP_STD_MSG_SIZE,
                                    QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                    qmi_err_code);
  } while (0);

  /* free dynamically allocated memory */
  if(
     (QMI_EAP_AUTH_START_EAP_USER_ID_PARAM & eap_auth_start->params_mask) &&
     ( NULL != orig_tmp_buf_ptr_user_id )
    )
  {
    free(orig_tmp_buf_ptr_user_id);
  }
  if(
     (QMI_EAP_AUTH_START_EAP_META_ID_PARAM & eap_auth_start->params_mask) &&
     ( NULL != orig_tmp_buf_ptr_meta_id)
    )
  {
    free (orig_tmp_buf_ptr_meta_id);
  }

  return rc;

}

/*===========================================================================
  FUNCTION  qmi_eap_auth_send_eap_packet
===========================================================================*/
/*!
@brief
    This function is used to send an eap request packet, and receive and
    EAP response packet in reply.

@return
  In the asynchronous case, if the return code < 0, the operation failed and
  you will not get an asynchronous response.  If the operation doesn't fail
  (return code >=0), the returned value will be a transaction handle which
  can be used to cancel the transaction via the qmi_eap_delete_async_txn().

@note
    This function executes asynchronously, User should register a callback
    function to be called when this service layer receives a response for
    this request.

  - Dependencies
    - qmi_connection_init() must be called for the associated port first.

  - Side Effects
*/
/*=========================================================================*/
int
qmi_eap_auth_send_eap_packet
(
  int                           user_handle,
  qmi_eap_user_async_cb_type    user_cb,              /*User call back function*/
  void                          *user_data,           /*User data*/
  void                          *eap_request_packet,  /*Max EAP Req Message size is  QMI_EAP_MAX_PKT_LEN*/
  int                           request_pkt_length,   /*Set it to the length of the request packet.*/
  int                           *qmi_err_code
)
{
  unsigned char     *msg;
  int               msg_size;
  unsigned char    *tmp_msg_ptr;
  int rc;

  (void) qmi_err_code;

  if ((eap_request_packet == NULL) ||
      (request_pkt_length <= 0) ||
      (request_pkt_length > (int)QMI_EAP_MAX_PKT_LEN))
  {
    QMI_ERR_MSG_0 (" qmi_eap_auth_send_eap_packet:"
                   " eap_request_packet not set or length exceeds QMI_EAP_STD_MSG_SIZE\n");
    return QMI_INTERNAL_ERR;
  }

  /* Dynamically allocate message buffer, since these can be big */
  msg = (unsigned char *) malloc (QMI_EAP_MAX_MSG_SIZE);
  if (!msg)
  {
    QMI_ERR_MSG_0 ("Unable do dynamically allocate memory for EAP request\n");
    return QMI_INTERNAL_ERR;
  }


  /* Set tmp_msg_ptr to beginning of message-specific TLV portion of
  ** message buffer
  */
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  /* Set the message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_EAP_MAX_MSG_SIZE);



  if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                                 &msg_size,
                                 QMI_EAP_SEND_PKT_REQ_TLV_ID,
                                 (unsigned long)request_pkt_length,
                                 (void *)eap_request_packet) < 0)
  {
    rc = QMI_INTERNAL_ERR;
  }

  else if (user_cb)
  {
    /*lint -e{611} */
    rc = qmi_service_send_msg_async (user_handle,
                                    QMI_EAP_SERVICE,
                                    QMI_EAP_AUTH_SEND_PACKET_REQ_MSG_ID,
                                    QMI_SRVC_PDU_PTR(msg),
                                    (int)QMI_SRVC_PDU_SIZE(QMI_EAP_MAX_MSG_SIZE) - msg_size,
                                    qmi_eap_srvc_async_cb,
                                    NULL,
                                    (void *)user_cb,
                                    user_data);

  }
  else
  {
    rc = QMI_INTERNAL_ERR;
  }
  free (msg);
  return rc;
}

/*===========================================================================
  FUNCTION  qmi_eap_auth_end_eap_session
===========================================================================*/
/*!
@brief
    This function is used to end an eap session

@return
  0 if abort operation was sucessful, < 0 if not.  If return code is
  QMI_INTERNAL_ERR, then the qmi_err_code will be valid and will
  indicate which QMI error occurred.

@note
    This function executes synchronously, there is not currently an
    asynchronous option for this functionality.

  - Dependencies
    - qmi_connection_init() must be called for the associated port first.

  - Side Effects
*/
/*=========================================================================*/
int
qmi_eap_auth_end_eap_session
(
  int                           user_handle,
  int                           *qmi_err_code
)
{
  unsigned char      msg[QMI_EAP_STD_MSG_SIZE];
  int                msg_size;
  int rc;

  rc = qmi_service_send_msg_sync (user_handle,
                                  QMI_EAP_SERVICE,
                                  QMI_EAP_AUTH_END_EAP_SESSION_REQ_MSG_ID,
                                  QMI_SRVC_PDU_PTR(msg),
                                  0,
                                  msg,
                                  &msg_size,
                                  QMI_EAP_STD_MSG_SIZE,
                                  QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                  qmi_err_code);


  return rc;
}


/*===========================================================================
  FUNCTION  qmi_eap_auth_get_session_keys
===========================================================================*/
/*!
@brief
    This function is used to query the session keys. This message should be
    used by the user after receiving the EAP session SUCCESS indiction.

@return
  0 if abort operation was sucessful, < 0 if not.  If return code is
  QMI_INTERNAL_ERR, then the qmi_err_code will be valid and will
  indicate which QMI error occurred.

@note
    This function executes synchronously, there is not currently an
    asynchronous option for this functionality.

  - Dependencies
    - qmi_connection_init() must be called for the associated port first.

  - Side Effects
*/
/*=========================================================================*/
int
qmi_eap_auth_get_session_keys
(
  int                           user_handle,
  void                          *eap_keys_resp_packet,/*Max EAP Resp Message size is  QMI_EAP_MAX_PKT_LEN*/
  int                           resp_pkt_length,        /*Set this to the length of the response buffer size.*/
  int                           *qmi_err_code
)
{
  unsigned char     msg[QMI_EAP_STD_MSG_SIZE];
  int               msg_size;
  unsigned char    *tmp_msg_ptr;
  int rc;

  /* Set tmp_msg_ptr to beginning of message-specific TLV portion of
  ** message buffer
  */
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  /* Set the message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_EAP_STD_MSG_SIZE);

  if (!eap_keys_resp_packet)
  {
    QMI_ERR_MSG_0 (" qmi_eap_auth_get_session_keys:: response_packetnot set\n");
    return QMI_INTERNAL_ERR;
  }

  rc = qmi_service_send_msg_sync (user_handle,
                                  QMI_EAP_SERVICE,
                                  QMI_EAP_AUTH_GET_SESSION_KEYS_REQ_MSG_ID,
                                  QMI_SRVC_PDU_PTR(msg),
                                  (int)QMI_SRVC_PDU_SIZE(QMI_EAP_STD_MSG_SIZE) - msg_size,
                                  msg,
                                  &msg_size,
                                  QMI_EAP_STD_MSG_SIZE,
                                  QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                  qmi_err_code);

  if (rc == QMI_NO_ERR)
  {
    unsigned long type;
    unsigned long length;
    unsigned char *value_ptr;

    /* Set tmp_msg_ptr to return data */
    tmp_msg_ptr = msg;

    while (msg_size > 0)
    {
      if (qmi_util_read_std_tlv (&tmp_msg_ptr,
                                    &msg_size,
                                    &type,
                                    &length,
                                    &value_ptr) < 0)
      {
        return QMI_INTERNAL_ERR;
      }

      switch (type)
      {
        case QMI_EAP_GET_SESSION_KEYS_RESP_TLV_ID:
          {
            if ((int)length > resp_pkt_length)
            {
              QMI_ERR_MSG_0 ("qmi_eap_auth_get_session_keys: "
                             "length of the response greater than the resp buffer size. \n");
              return QMI_INTERNAL_ERR;
            }
            memcpy((void *)eap_keys_resp_packet,(void *)value_ptr,length);
          }
          break;
        default:
          {
            QMI_ERR_MSG_1 ("qmi_eap_auth_get_session_keys: "
                           "unknown response TLV type = %x",(unsigned int)type);
          }
          break;
      }
    }
  }
  return rc;
}


/*===========================================================================
  FUNCTION  qmi_eap_delete_async_txn
===========================================================================*/
/*!
@brief
    This function is used to cancel an asynchronous transaction.  Note
    that there is an inherent race condition in that an asynchronous response
    may be in the process of being processed when this is called, and may
    still show up after this function is sucessfully called.

@return
  QMI_NO_ERR if operation is successful, QMI_INTERNAL_ERR if not.

@note


  - Side Effects: Cancels async transaction
*/
/*=========================================================================*/
int
qmi_eap_delete_async_txn
(
  int user_handle,
  int async_txn_handle
)
{
  return qmi_service_delete_async_txn (user_handle,
                                       async_txn_handle);
}

/*===========================================================================
  FUNCTION  qmi_eap_auth_initiate_aka_algorithm
===========================================================================*/
/*!
@brief
    This command initiates the AKA algorithm.

@return
  QMI_NO_ERR if operation was sucessful, < 0 if not.  If return code is
  QMI_SERVICE_ERR, then the qmi_err_code will be valid and will
  indicate which QMI error occurred.

@note
    This function executes synchronously, there is not currently an
    asynchronous option for this functionality.

  - Dependencies
    - qmi_eap_srvc_init_client() must be called for the associated port first.

  - Side Effects
*/
/*=========================================================================*/
int
qmi_eap_auth_initiate_aka_algorithm
(
  int                                    user_handle,
  qmi_eap_initiate_aka_algorithm_type   *aka_algorithm,
  unsigned long                         *aka_handle,
  int                                   *qmi_err_code
)
{

  unsigned char     msg[QMI_EAP_STD_MSG_SIZE];
  int               msg_size;
  unsigned char    *tmp_msg_ptr;
  int rc;

  /* Set tmp_msg_ptr to beginning of message-specific TLV portion of
  ** message buffer
  */
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  /* Set the message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_EAP_STD_MSG_SIZE);

  if (!aka_algorithm)
  {
    QMI_ERR_MSG_0 (" qmi_eap_auth_initiate_aka_algorithm: "
                   "AKA version not mentioned\n");
    return QMI_INTERNAL_ERR;
  }
  else if (!aka_handle)
  {
    QMI_ERR_MSG_0 (" qmi_eap_auth_initiate_aka_algorithm: "
                   "NULL aka_handle\n");
    return QMI_INTERNAL_ERR;
  }

  if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                              &msg_size,
                              QMI_EAP_AKA_ALGORITHM_VERSION_TLV_ID,
                              sizeof(unsigned char),
                              (void *)&aka_algorithm->aka_version) < 0)
  {
    return QMI_INTERNAL_ERR;
  }

  if (aka_algorithm->param_mask & QMI_EAP_AKA_V1_OR_V2_AUTH_PARAMS)
  {
    unsigned char tmp_msg_buf[2*QMI_EAP_MAX_STR_LEN + 2*sizeof(unsigned char)];
    unsigned char *tmp_msg_buf_ptr = tmp_msg_buf;
    unsigned long copy_len, tlv_len = 0;

    WRITE_8_BIT_VAL(tmp_msg_buf_ptr,
                    aka_algorithm->v1_or_v2_auth_params.rand_len);

    tlv_len++;
    copy_len = (size_t)(aka_algorithm->v1_or_v2_auth_params.rand_len >
      QMI_EAP_MAX_STR_LEN ? QMI_EAP_MAX_STR_LEN :
      aka_algorithm->v1_or_v2_auth_params.rand_len);

    if (copy_len != aka_algorithm->v1_or_v2_auth_params.rand_len)
    {
      QMI_DEBUG_MSG_1 ("qmi_eap_auth_initiate_aka_algorithm: "
                       "rand len exceeds the max, truncating it to %d ",
                       QMI_EAP_MAX_STR_LEN);
    }
    memcpy(tmp_msg_buf_ptr,
           &aka_algorithm->v1_or_v2_auth_params.rand,
           copy_len);

    tmp_msg_buf_ptr += copy_len;
    tlv_len += copy_len;

    WRITE_8_BIT_VAL(tmp_msg_buf_ptr,
                    aka_algorithm->v1_or_v2_auth_params.auth_len);

    tlv_len++;
    copy_len = (size_t)(aka_algorithm->v1_or_v2_auth_params.auth_len >
      QMI_EAP_MAX_STR_LEN ? QMI_EAP_MAX_STR_LEN :
      aka_algorithm->v1_or_v2_auth_params.auth_len);

    if (copy_len != aka_algorithm->v1_or_v2_auth_params.auth_len)
    {
      QMI_DEBUG_MSG_1 ("qmi_eap_auth_initiate_aka_algorithm: "
                       "auth len exceeds the max, truncating it to %d ",
                       QMI_EAP_MAX_STR_LEN);
    }
    memcpy(tmp_msg_buf_ptr,
           &aka_algorithm->v1_or_v2_auth_params.auth,
           copy_len);

    tmp_msg_buf_ptr += copy_len;
    tlv_len += copy_len;

    if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                                &msg_size,
                                QMI_EAP_AKA_V1_OR_V2_AUTH_PARAMS_TLV_ID,
                                tlv_len,
                                (void *)tmp_msg_buf) < 0)
    {
      return QMI_INTERNAL_ERR;
    }

  }

  rc = qmi_service_send_msg_sync (user_handle,
                                  QMI_EAP_SERVICE,
                                  QMI_EAP_AUTH_INITIATE_AKA_ALGORITHM_MSG_ID,
                                  QMI_SRVC_PDU_PTR(msg),
                                  (int)QMI_SRVC_PDU_SIZE(QMI_EAP_STD_MSG_SIZE) - msg_size,
                                  msg,
                                  &msg_size,
                                  QMI_EAP_STD_MSG_SIZE,
                                  QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                  qmi_err_code);

  if (rc == QMI_NO_ERR)
  {
    unsigned long type;
    unsigned long length;
    unsigned char *value_ptr;

    /* Set tmp_msg_ptr to return data */
    tmp_msg_ptr = msg;

    while (msg_size > 0)
    {
      if (qmi_util_read_std_tlv (&tmp_msg_ptr,
                                 &msg_size,
                                 &type,
                                 &length,
                                 &value_ptr) < 0)
      {
        return QMI_INTERNAL_ERR;
      }

      switch (type)
      {
        case QMI_EAP_AKA_HANDLE_TLV_ID:
          {
            READ_32_BIT_VAL(value_ptr, (*aka_handle));
          }
          break;

        default:
          {
            QMI_ERR_MSG_1 ("qmi_eap_auth_initiate_aka_algorithm: "
                           "unknown response TLV type = %lx", type);
          }
          break;
      }
    }
  }
  return rc;
}

/*===========================================================================
  FUNCTION  qmi_auth_set_subscription_binding
===========================================================================*/
/*!
@brief
    This command associates the client with the requested subscription

@return
  QMI_NO_ERR if operation was sucessful, < 0 if not.  If return code is
  QMI_SERVICE_ERR, then the qmi_err_code will be valid and will
  indicate which QMI error occurred.

@note
    This function executes synchronously, there is not currently an
    asynchronous option for this functionality.

  - Dependencies
    - qmi_eap_srvc_init_client() must be called for the associated port first.

  - Side Effects
*/
/*=========================================================================*/
int
qmi_auth_set_subscription_binding
(
  int                          user_handle,
  qmi_auth_subscription_type   subscription,
  int                          *qmi_err_code
)
{
  unsigned char     msg[QMI_EAP_STD_MSG_SIZE];
  int               msg_size;
  unsigned char    *tmp_msg_ptr;
  unsigned int      tmp32;
  int rc;

  /* Set tmp_msg_ptr to beginning of message-specific TLV portion of
  ** message buffer
  */
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  /* Set the message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_EAP_STD_MSG_SIZE);

  if (subscription < QMI_AUTH_SUBS_TYPE_PRIMARY ||
      subscription >= QMI_AUTH_SUBS_TYPE_MAX)
  {
    QMI_ERR_MSG_1 ("qmi_auth_bind_subscription: invalid subscription: %d\n",
                   subscription);
    return QMI_INTERNAL_ERR;
  }
  else if (!qmi_err_code)
  {
    QMI_ERR_MSG_0 ("qmi_auth_bind_subscription: bad parameter\n");
    return QMI_INTERNAL_ERR;
  }

  tmp32 = (unsigned int) subscription;

  if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                              &msg_size,
                              QMI_EAP_BIND_SUBSCRIPTION_TLV_ID,
                              4,
                              (void *)&tmp32) < 0)
  {
    return QMI_INTERNAL_ERR;
  }

  rc = qmi_service_send_msg_sync (user_handle,
                                  QMI_EAP_SERVICE,
                                  QMI_EAP_AUTH_BIND_SUBSCRIPTION_MSG_ID,
                                  QMI_SRVC_PDU_PTR(msg),
                                  (int)QMI_SRVC_PDU_SIZE(QMI_EAP_STD_MSG_SIZE) - msg_size,
                                  msg,
                                  &msg_size,
                                  QMI_EAP_STD_MSG_SIZE,
                                  QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                  qmi_err_code);

  return rc;
}

/*===========================================================================
  FUNCTION  qmi_auth_get_bind_subscription
===========================================================================*/
/*!
@brief
    This command queries the current subscription associated with the client

@return
  QMI_NO_ERR if operation was sucessful, < 0 if not.  If return code is
  QMI_SERVICE_ERR, then the qmi_err_code will be valid and will
  indicate which QMI error occurred.

@note
    This function executes synchronously, there is not currently an
    asynchronous option for this functionality.

  - Dependencies
    - qmi_eap_srvc_init_client() must be called for the associated port first.

  - Side Effects
*/
/*=========================================================================*/
int
qmi_auth_get_bind_subscription
(
  int                          user_handle,
  qmi_auth_subscription_type   *subscription,
  int                          *qmi_err_code
)
{
  unsigned char     msg[QMI_EAP_STD_MSG_SIZE];
  int               msg_size;
  unsigned char    *tmp_msg_ptr;
  unsigned int      tmp32;
  int rc;

  /* Set tmp_msg_ptr to beginning of message-specific TLV portion of
  ** message buffer
  */
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  /* Set the message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_EAP_STD_MSG_SIZE);

  if (!subscription || !qmi_err_code)
  {
    QMI_ERR_MSG_0 ("qmi_auth_get_bind_subscription: bad parameter\n");
    return QMI_INTERNAL_ERR;
  }

  /* Initialize to invalid value */
  *subscription = QMI_AUTH_SUBS_TYPE_MAX;

  rc = qmi_service_send_msg_sync (user_handle,
                                  QMI_EAP_SERVICE,
                                  QMI_EAP_AUTH_GET_BIND_SUBSCRIPTION_MSG_ID,
                                  QMI_SRVC_PDU_PTR(msg),
                                  (int)QMI_SRVC_PDU_SIZE(QMI_EAP_STD_MSG_SIZE) - msg_size,
                                  msg,
                                  &msg_size,
                                  QMI_EAP_STD_MSG_SIZE,
                                  QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                  qmi_err_code);

  if (rc == QMI_NO_ERR)
  {
    unsigned long type;
    unsigned long length;
    unsigned char *value_ptr;

    /* Set tmp_msg_ptr to return data */
    tmp_msg_ptr = msg;

    while (msg_size > 0)
    {
      if (qmi_util_read_std_tlv (&tmp_msg_ptr,
                                 &msg_size,
                                 &type,
                                 &length,
                                 &value_ptr) < 0)
      {
        return QMI_INTERNAL_ERR;
      }

      switch (type)
      {
        case QMI_EAP_GET_BIND_SUBSCRIPTION_TLV_ID:
          {
            READ_32_BIT_VAL(value_ptr, tmp32);
            *subscription = (qmi_auth_subscription_type) tmp32;
          }
          break;
        default:
          {
            QMI_ERR_MSG_1 ("qmi_auth_get_bind_subscription: "
                           "unknown response TLV type = %x",(unsigned int)type);
          }
          break;
      }
    }
  }

  return rc;
}

/*===========================================================================
  FUNCTION  qmi_auth_indication_register
===========================================================================*/
/*!
@brief
  Register/deregister for different QMI AUTH indications. Indications include
  QMI_AUTH_EAP_NOTIFICATION_CODE_IND, QMI_AUTH_EAP_ERROR_CODE_IND,
  QMI_AUTH_EAP_AUTH_REJ_IND

@return
  QMI_NO_ERR if operation was sucessful, < 0 if not.  If return code is
  QMI_SERVICE_ERR, then the qmi_err_code will be valid and will
  indicate which QMI error occurred.

@note
    This function executes synchronously, there is not currently an
    asynchronous option for this functionality.

  - Dependencies
    - qmi_eap_srvc_init_client() must be called for the associated port first.

  - Side Effects
*/
/*=========================================================================*/
EXTERN int
qmi_auth_indication_register
(
  int                           user_handle,
  qmi_auth_indication_reg_type  *ind_reg,
  int                           *qmi_err_code
)
{
  unsigned char msg[QMI_EAP_STD_MSG_SIZE];
  int           msg_size;
  unsigned char *tmp_msg_ptr;
  int rc;

  /* Set tmp_msg_ptr to beginning of message-specific TLV portion of
  ** message buffer
  */
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  /* Set the message size to the complete buffer minus the header size */
  msg_size = QMI_SRVC_PDU_SIZE(QMI_EAP_STD_MSG_SIZE);

  if (!ind_reg || !qmi_err_code)
  {
    QMI_ERR_MSG_0 ("qmi_auth_indication_register: bad parameter(s)\n");
    return QMI_INTERNAL_ERR;
  }

  /* Construct TLV for client ID request */

  /* Register for QMI Auth notification code change indications */
  if (ind_reg->param_mask & QMI_AUTH_EAP_NOTIF_CODE_REG_PARAM_MASK)
  {
    if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                                &msg_size,
                                QMI_EAP_SESSION_ERR_NOTIF_TLV_ID,
                                1,
                                (void *)&ind_reg->report_eap_notif_code) < 0)

    {
      return QMI_INTERNAL_ERR;
    }
  }

  if (ind_reg->param_mask & QMI_AUTH_EAP_ERROR_CODE_REG_PARAM_MASK)
  {
    if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                                &msg_size,
                                QMI_EAP_SESSION_ERR_CODE_TLV_ID,
                                1,
                                (void *)&ind_reg->report_eap_err_code) < 0)

    {
      return QMI_INTERNAL_ERR;
    }
  }

  if (ind_reg->param_mask & QMI_AUTH_EAP_AUTH_REJ_REG_PARAM_MASK)
  {
    if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                                &msg_size,
                                QMI_EAP_AUTH_REJ_NOTIF_TLV_ID,
                                1,
                                (void *)&ind_reg->report_eap_auth_reject) < 0)

    {
      return QMI_INTERNAL_ERR;
    }
  }
  rc = qmi_service_send_msg_sync (user_handle,
                                  QMI_EAP_SERVICE,
                                  QMI_EAP_AUTH_INDICATION_REGISTER_MSG_ID,
                                  QMI_SRVC_PDU_PTR(msg),
                                  (int)QMI_SRVC_PDU_SIZE(QMI_EAP_STD_MSG_SIZE) - msg_size,
                                  msg,
                                  &msg_size,
                                  QMI_EAP_STD_MSG_SIZE,
                                  QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                  qmi_err_code);
  return rc;
}

