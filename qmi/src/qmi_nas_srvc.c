/******************************************************************************
  @file    qmi_nas_srvc.c
  @brief   The QMI NAS service layer.

  DESCRIPTION
  QMI NAS service routines.

  INITIALIZATION AND SEQUENCING REQUIREMENTS
  qmi_nas_srvc_init_client() needs to be called before sending or receiving of any
  NAS service messages

  ---------------------------------------------------------------------------
  Copyright (c) 2008,2014 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/


#include "qmi_i.h"
#include "qmi_service.h"
#include "qmi_nas_srvc.h"
#include "qmi_util.h"

#define QMI_EAP_STD_MSG_SIZE                    QMI_MAX_STD_MSG_SIZE
#define QMI_NAS_STD_MSG_SIZE                    QMI_MAX_STD_MSG_SIZE
#define QMI_NAS_EVENT_REPORT_TLV_MAX_SIZE       7 * sizeof(char)

/*Indication message IDs*/
#define QMI_NAS_EVENT_REPORT_IND_MSG_ID           0x0002
#define QMI_NAS_SERVING_SYSTEM_IND_MSG_ID         0x0024

/*TLV IDs*/
#define QMI_NAS_SIGNAL_STRENGTH_IND_TLV_ID        0x10

/*Message Ids*/
#define     QMI_NAS_EVENT_REPORT_MSG_ID               0x0002
#define     QMI_NAS_INDICATION_REGISTER_MSG_ID        0x0003
#define     QMI_NAS_GET_SERVING_SYSTEM_MSG_ID         0x0024
#define     QMI_NAS_INIT_PS_ATTACH_DETACH_MSG_ID      0x0023


/*req resp tlv ids*/
#define     QMI_NAS_SERVING_SYSTEM_TLV_ID             0x01
#define     QMI_NAS_ROAMING_INDICATOR_TLV_ID          0x10
#define     QMI_NAS_DS_CAPABILITIES_TLV_ID            0x11
#define     QMI_NAS_CURRENT_PLMN_TLV_ID               0x12
#define     QMI_NAS_PS_ATTACH_ACTION_TLV_ID           0x10
#define     QMI_NAS_SYS_SEL_PREF_IND_TLV_ID           0x10
#define     QMI_NAS_DDTM_EVENTS_IND_TLV_ID            0x12
#define     QMI_NAS_SERVING_SYSTEM_IND_TLV_ID         0x13

#define WRITE_8_BIT_SIGNED_VAL(dest,val) \
 do {*(char *)dest = (char) val; \
     dest = (char *)dest + 1;} while (0)

static int nas_service_initialized = FALSE;


/*===========================================================================
  FUNCTION  qmi_nas_get_serving_system_info
===========================================================================*/
/*!
@brief
  This function will decode TLV's dealing with serving system info.  It
  is used both by the get serving system info cmd response as well as the
  serving system indication.

@return
  QMI_INTERNAL_ERR if an error was encountered, QMI_NO_ERR otherwise.

*/
/*=========================================================================*/
static int
qmi_nas_get_serving_system_info
(
  unsigned char                     *rx_buf,
  int                               rx_buf_len,
  qmi_nas_serving_system_info_type  *ss_info
)
{
  unsigned long type;
  unsigned long length;
  unsigned char *value_ptr;

  /* Initialize param_mask to 0 */
  ss_info->param_mask = 0;

  /* Cycle through return message reading all TLV's */
  while (rx_buf_len > 0)
  {
    if (qmi_util_read_std_tlv (&rx_buf,
                                  &rx_buf_len,
                                  &type,
                                  &length,
                                  &value_ptr) < 0)
    {
      return QMI_INTERNAL_ERR;
    }

    /* Now process TLV */
    switch (type)
    {
      case QMI_NAS_SERVING_SYSTEM_TLV_ID:
      {
        unsigned char tmp;
        int i = 0;
        READ_8_BIT_VAL(value_ptr,tmp);
        ss_info->reg_state = (qmi_nas_registration_state) tmp;
        READ_8_BIT_VAL(value_ptr,tmp);
        ss_info->cs_attach_state = (qmi_nas_cs_attach_state) tmp;
        READ_8_BIT_VAL(value_ptr,tmp);
        ss_info->ps_attach_state = (qmi_nas_ps_attach_state) tmp;
        READ_8_BIT_VAL(value_ptr,tmp);
        ss_info->network_type = (qmi_nas_reg_network_type) tmp;
        READ_8_BIT_VAL(value_ptr,ss_info->num_radio_interfaces);

        for (i = 0; (i < ss_info->num_radio_interfaces) && (i < QMI_NAS_MAX_RADIO_IFACES); i++ )
        {
          unsigned char temp;
          READ_8_BIT_VAL(value_ptr,temp);
          ss_info->radio_if[i] = (qmi_nas_radio_interface) temp;
        }
      }
      break;


      case QMI_NAS_ROAMING_INDICATOR_TLV_ID:
      {
        ss_info->param_mask |= QMI_NAS_ROAMING_INDICATOR_PARAM_TYPE;
        READ_8_BIT_VAL (value_ptr,ss_info->roaming_indicator);
      }
      break;

      case QMI_NAS_CURRENT_PLMN_TLV_ID:
      {
        unsigned int net_desc_len;
        ss_info->param_mask |= QMI_NAS_CURRENT_PLMN_PARAM_TYPE;
        READ_16_BIT_VAL (value_ptr,ss_info->current_plmn.mobile_country_code);
        READ_16_BIT_VAL (value_ptr,ss_info->current_plmn.mobile_network_code);
        READ_8_BIT_VAL (value_ptr,net_desc_len);

        if (net_desc_len > 0)
        {
          size_t cpy_len = (net_desc_len < QMI_NAS_MAX_STR_LEN)
                           ? net_desc_len : (QMI_NAS_MAX_STR_LEN - 1);
          memcpy (ss_info->current_plmn.network_desc, (void *)value_ptr, cpy_len);
          ss_info->current_plmn.network_desc[cpy_len] = '\0';
        }
        else
        {
          ss_info->current_plmn.network_desc[0] = '\0';
        }
      }
      break;

      case QMI_NAS_DS_CAPABILITIES_TLV_ID:
      {
        int i;
        ss_info->param_mask |= (unsigned short)QMI_NAS_DS_CAPABILITY_PARAM_TYPE;
        READ_8_BIT_VAL(value_ptr,ss_info->ds_capabilites.num_capabilities);
        for (i = 0;  (i < ss_info->ds_capabilites.num_capabilities) &&
                     (i < (unsigned short)QMI_NAS_DS_CAPABILITY_MAX); i++)
        {
          unsigned char tmp;
          READ_8_BIT_VAL(value_ptr,tmp);
          ss_info->ds_capabilites.capabilities[i] = (qmi_nas_ds_capability_type) tmp;
        }
      }
      break;


      default:
        QMI_DEBUG_MSG_1 ("qmi_nas_get_serving_system_info: Unknown TLV ID=%x\n",(unsigned)type);
      break;

    } /* switch */
  } /* while */

  return QMI_NO_ERR;
}

/*===========================================================================
  FUNCTION  qmi_nas_srvc_indication_cb
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

  - Side Effects
*/
/*=========================================================================*/

static void
qmi_nas_srvc_indication_cb
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
  qmi_nas_indication_id_type      ind_id = QMI_NAS_SRVC_INVALID_IND_MSG;
  qmi_nas_indication_data_type    ind_data;
  qmi_nas_indication_hdlr_type    user_ind_hdlr;

  unsigned char tmp;
  unsigned long type;
  unsigned long length;
  unsigned char *value_ptr;

  /* Make sure that the user indication handler isn't NULL */
  if (user_ind_msg_hdlr == NULL)
  {
    return;
  }

  switch (msg_id)
  {

    case QMI_NAS_EVENT_REPORT_IND_MSG_ID:
    {
      ind_id = QMI_NAS_SRVC_EVENT_REPORT_IND_MSG;
      if (qmi_util_read_std_tlv (&rx_msg_buf,
                                    &rx_msg_len,
                                    &type,
                                    &length,
                                    &value_ptr) < 0)
      {
        return ;
      }

      if (type != QMI_NAS_SIGNAL_STRENGTH_IND_TLV_ID)
      {
        QMI_ERR_MSG_1 ("qmi_nas_srvc_indication_cb::Invalid TLV received %lx \n ",type);
        return;
      }

      READ_8_BIT_VAL (value_ptr,ind_data.signal_strength.sig_strength);
      READ_8_BIT_VAL (value_ptr,tmp);
      ind_data.signal_strength.radio_if = (qmi_nas_radio_interface) tmp;
    }
    break;

    case QMI_NAS_SERVING_SYSTEM_IND_MSG_ID:
    {
      ind_id = QMI_NAS_SRVC_SERVING_SYSTEM_IND_MSG;

      if (qmi_nas_get_serving_system_info (rx_msg_buf,
                                           rx_msg_len,
                                           &ind_data.serving_system) < 0)
      {
        QMI_ERR_MSG_0 ("qmi_nas_srvc_indication_cb: Failure in get serving system IND data\n ");
        return;
      }
    }
    break;

    default:
      QMI_ERR_MSG_1 ("qmi_nas_srvc_indication_cb::Invalid indication msg_id received %lx\n ",msg_id);
    break;
  }/*Switch*/

  /* If we got a valid/supported indication, report it */
  if (ind_id != QMI_NAS_SRVC_INVALID_IND_MSG)
  {
    /* Get properly cast pointer to user indication handler */
    /*lint -e{611} */
    user_ind_hdlr = (qmi_nas_indication_hdlr_type) user_ind_msg_hdlr;

    /* Call user registered handler */
    user_ind_hdlr (user_handle,
                   service_id,
                   user_ind_msg_hdlr_user_data,
                   ind_id,
                   &ind_data);
  }
}

/*===========================================================================
  FUNCTION  qmi_nas_srvc_init
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
int qmi_nas_srvc_init (void)
{
  int  rc = QMI_NO_ERR;
  if (!nas_service_initialized)
  {

    rc = qmi_service_set_srvc_functions (QMI_NAS_SERVICE,
                                 qmi_nas_srvc_indication_cb);
    if (rc != QMI_NO_ERR)
    {
      QMI_ERR_MSG_1("qmi_nas_srvc_init: set srvc functions returns err=%d\n",rc);
    }
    else
    {
      QMI_DEBUG_MSG_0("qmi_nas_srvc_init: NAS successfully initialized");
      nas_service_initialized = TRUE;
    }
  }
  else
  {
    QMI_ERR_MSG_0 ("qmi_nas_srvc_init: Init failed, NAS already initialized");
  }
  return rc;
}


/*===========================================================================
  FUNCTION  qmi_nas_srvc_release
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
int qmi_nas_srvc_release (void)
{
  int  rc = QMI_NO_ERR;
  if (nas_service_initialized)
  {

    rc = qmi_service_set_srvc_functions (QMI_NAS_SERVICE,NULL);
    if (rc != QMI_NO_ERR)
    {
      QMI_ERR_MSG_1("qmi_nas_srvc_release: set srvc functions returns err=%d\n",rc);
    }
    else
    {
      QMI_DEBUG_MSG_0("qmi_nas_srvc_release: NAS successfully released");
      nas_service_initialized = FALSE;
    }
  }
  else
  {
    QMI_ERR_MSG_0 ("qmi_nas_srvc_release: Release failed, NAS not initialized");
  }
  return rc;
}

/*===========================================================================
  FUNCTION  qmi_nas_srvc_init_client
===========================================================================*/
/*!
@brief
  This function is called to initialize the NAS service.  This function
  must be called prior to calling any other NAS service functions.
  For the time being, the indication handler callback and user data
  should be set to NULL until this is implemented.  Also note that this
  function may be called multiple times to allow for multiple, independent
  clients.

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
qmi_client_handle_type
qmi_nas_srvc_init_client
(
   const char                   *dev_id,
  qmi_nas_indication_hdlr_type  user_ind_msg_hdlr,
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
                                   QMI_NAS_SERVICE,
                                   (void *) user_ind_msg_hdlr,
                                   user_ind_msg_hdlr_user_data,
                                   qmi_err_code);



  return client_handle;
}


/*===========================================================================
  FUNCTION  qmi_nas_srvc_release_client
===========================================================================*/
/*!
@brief
  This function is called to release a client created by the
  qmi_nas_srvc_init_client() function.  This function should be called
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
qmi_nas_srvc_release_client
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
  FUNCTION  qmi_nas_set_event_report_state
===========================================================================*/
/*!
@brief
  Set the NAS event reporting state


@return

@note

  - Dependencies
    - qmi_nas_srvc_init_client() must be called before calling this.

  - Side Effects
    - Starts event reporting
*/
/*=========================================================================*/
int
qmi_nas_set_event_report_state
(
  int                               client_handle,
  qmi_nas_event_report_state_type   report_state,
  unsigned char                     num_of_signal_strength_thresholds,
  char                              *thresholds_list,
  int                               *qmi_err_code
)
{
  unsigned char     msg[QMI_EAP_STD_MSG_SIZE],*tmp_msg_ptr;
  int               msg_size, rc;
  unsigned long     tlv_length = 0;
  char              tmp_buff[QMI_NAS_EVENT_REPORT_TLV_MAX_SIZE];
  char              *tmp_buff_ptr = tmp_buff;


  if (num_of_signal_strength_thresholds > 5 || thresholds_list == NULL)
  {
    QMI_ERR_MSG_0 ("qmi_nas_set_event_report_state::Bad Input\n");
    return QMI_INTERNAL_ERR;
  }
  /*Prepare the Value portion of the TLV*/
  WRITE_8_BIT_SIGNED_VAL(tmp_buff_ptr,report_state);
  WRITE_8_BIT_SIGNED_VAL(tmp_buff_ptr,num_of_signal_strength_thresholds);
  memcpy(tmp_buff_ptr,thresholds_list,num_of_signal_strength_thresholds);


  /*Prepare the TLV*/
  tlv_length = (2 * sizeof(char)) + ((num_of_signal_strength_thresholds) * sizeof(char));


  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  msg_size = QMI_SRVC_PDU_SIZE(QMI_EAP_STD_MSG_SIZE);

  if ((rc = qmi_util_write_std_tlv (&tmp_msg_ptr,
                                       &msg_size,
                                       QMI_NAS_SIGNAL_STRENGTH_IND_TLV_ID,
                                       tlv_length,
                                       (void *)tmp_buff)) < 0)
  {
    return QMI_INTERNAL_ERR;
  }

  rc = qmi_service_send_msg_sync (client_handle,
                                  QMI_NAS_SERVICE,
                                  QMI_NAS_EVENT_REPORT_MSG_ID,
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
  FUNCTION  qmi_nas_get_serving_system
===========================================================================*/
/*!
@brief
  This message queries for information on the system that is currently
  providing service.

@return

@note

  - Dependencies
    - qmi_nas_srvc_init_client() must be called before calling this.

  - Side Effects
    - Starts event reporting
*/
/*=========================================================================*/
int
qmi_nas_get_serving_system
(
  int                               client_handle,
  qmi_nas_serving_system_info_type  *resp_data,
  int                               *qmi_err_code
)
{
  unsigned char     msg[QMI_EAP_STD_MSG_SIZE];
  int               msg_size, rc;

  if(!resp_data)
  {
    return QMI_INTERNAL_ERR;
  }

  /*Prepare the Request Message*/
  msg_size = QMI_SRVC_PDU_SIZE(QMI_EAP_STD_MSG_SIZE);

  rc = qmi_service_send_msg_sync (client_handle,
                                  QMI_NAS_SERVICE,
                                  QMI_NAS_GET_SERVING_SYSTEM_MSG_ID,
                                  QMI_SRVC_PDU_PTR(msg),
                                  (int)QMI_SRVC_PDU_SIZE(QMI_EAP_STD_MSG_SIZE) - msg_size,
                                  msg,
                                  &msg_size,
                                  QMI_EAP_STD_MSG_SIZE,
                                  QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                  qmi_err_code);

  if (rc == QMI_NO_ERR)
  {

    if (qmi_nas_get_serving_system_info (msg,msg_size,resp_data) < 0)
    {
      QMI_ERR_MSG_0 ("qmi_nas_get_serving_system: qmi_nas_get_serving_system_info returned error");
      rc = QMI_INTERNAL_ERR;
    }

  }
  return rc;
}

/*===========================================================================
  FUNCTION  qmi_nas_initiate_ps_attach_detach
===========================================================================*/
/*!
@brief
  This function is used to initiate a PS domain attach or detach.

@return

@note

  - Dependencies
    - qmi_nas_srvc_init_client() must be called before calling this.

  - Side Effects
    - Attach or Detach is initiated
*/
/*=========================================================================*/
int
qmi_nas_initiate_ps_attach_detach
(
  int                         client_handle,
  qmi_nas_ps_attach_state     attach_detach,
  int                         *qmi_err_code
)
{
  unsigned char     msg[QMI_EAP_STD_MSG_SIZE],*tmp_msg_ptr;
  int               msg_size, rc;

  /* Do a bit of error checking first */
  if((attach_detach != QMI_NAS_PS_ATTACH) &&
     (attach_detach != QMI_NAS_PS_DETACH))
      {
        return QMI_INTERNAL_ERR;
      }

  /*Prepare the Request Message*/
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  msg_size = QMI_SRVC_PDU_SIZE(QMI_EAP_STD_MSG_SIZE);


  /* Add the attach/detach action TLV */
  if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                                  &msg_size,
                                  QMI_NAS_PS_ATTACH_ACTION_TLV_ID,
                                  1,
                                  (void *)&attach_detach) < 0)
            {
    return QMI_INTERNAL_ERR;
  }


  rc = qmi_service_send_msg_sync (client_handle,
                                  QMI_NAS_SERVICE,
                                  QMI_NAS_INIT_PS_ATTACH_DETACH_MSG_ID,
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
  FUNCTION  qmi_nas_indication_register
===========================================================================*/
/*!
@brief
  Set the NAS indication registration state for specified control point.


@return

@note

  - Dependencies
    - qmi_qos_srvc_init_client() must be called before calling this.

  - Side Effects
    - Starts event reporting
*/
/*=========================================================================*/
int
qmi_nas_indication_register
(
  int                                     client_handle,
  qmi_nas_indication_register_info_type  *ind_state,
  int                                    *qmi_err_code
)
{
  unsigned char     msg[QMI_NAS_STD_MSG_SIZE],*tmp_msg_ptr;
  int               msg_size, rc;
  unsigned long     tlv_length = 0;
  unsigned char     tmp_tlv_buf[5],*val_ptr;

  if(!ind_state)
  {
    return QMI_INTERNAL_ERR;
  }

  /* Set tmp_msg_ptr to beginning of message-specific TLV portion of
  ** message buffer
  */
  tmp_msg_ptr = QMI_SRVC_PDU_PTR(msg);

  msg_size = QMI_SRVC_PDU_SIZE(QMI_NAS_STD_MSG_SIZE);

  /* Write system selection preference TLV if appropriate */
  if(ind_state->param_mask & QMI_NAS_SYS_SEL_PREF_IND)
  {
    val_ptr = tmp_tlv_buf;
    /* Set TLV based on parameter value */
    WRITE_8_BIT_VAL (val_ptr, (int)ind_state->reg_sys_sel_pref );
    if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                                &msg_size,
                                QMI_NAS_SYS_SEL_PREF_IND_TLV_ID,
                                1,
                                (void *)tmp_tlv_buf) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }
  /* Write DDTM events TLV if appropriate */
  if(ind_state->param_mask & QMI_NAS_DDTM_EVENTS_IND)
  {
    val_ptr = tmp_tlv_buf;
    /* Set TLV based on parameter value */
    WRITE_8_BIT_VAL (val_ptr, (int)ind_state->reg_ddtm_events );
    if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                                &msg_size,
                                QMI_NAS_DDTM_EVENTS_IND_TLV_ID,
                                1,
                                (void *)tmp_tlv_buf) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }
  /* Write service system TLV if appropriate */
  if(ind_state->param_mask & QMI_NAS_SERVING_SYSTEM_IND)
  {
    val_ptr = tmp_tlv_buf;
    /* Set TLV based on parameter value */
    WRITE_8_BIT_VAL (val_ptr, (int)ind_state->reg_serving_system );
    if (qmi_util_write_std_tlv (&tmp_msg_ptr,
                                &msg_size,
                                QMI_NAS_SERVING_SYSTEM_IND_TLV_ID,
                                1,
                                (void *)tmp_tlv_buf) < 0)
    {
      return QMI_INTERNAL_ERR;
    }
  }

  /* Synchronously send message to modem processor */
  rc = qmi_service_send_msg_sync (client_handle,
                                  QMI_NAS_SERVICE,
                                  QMI_NAS_INDICATION_REGISTER_MSG_ID,
                                  QMI_SRVC_PDU_PTR(msg),
                                  (int)QMI_SRVC_PDU_SIZE(QMI_NAS_STD_MSG_SIZE) - msg_size,
                                  msg,
                                  &msg_size,
                                  QMI_NAS_STD_MSG_SIZE,
                                  QMI_SYNC_MSG_DEFAULT_TIMEOUT,
                                  qmi_err_code);

  return rc;
} /* qmi_nas_indication_register */


/*===========================================================================
  FUNCTION  qmi_nas_indication_register_all
===========================================================================*/
/*!
@brief
  Set the NAS indication registration state for all active control points.


@return

@note

  - Dependencies
    - qmi_qos_srvc_init_client() must be called before calling this.

  - Side Effects
    - Starts event reporting
*/
/*=========================================================================*/
int
qmi_nas_indication_register_all
(
  qmi_nas_indication_register_info_type  *ind_state,
  int                                    *qmi_err_code
)
{
  unsigned int dev_id;
  int rc = 0;
  int qmi_client;

  /* List of connections supported in QMI platform layer.  This will
   * migrate to configuration database in near future. */
  static const char *dev_id_table [] =
  {
    QMI_PORT_RMNET_0,
    QMI_PORT_RMNET_1,
    QMI_PORT_RMNET_2,
    QMI_PORT_RMNET_3,
    QMI_PORT_RMNET_4,
    QMI_PORT_RMNET_5,
    QMI_PORT_RMNET_6,
    QMI_PORT_RMNET_7,
    QMI_PORT_RMNET_SDIO_0,
    QMI_PORT_RMNET_SDIO_1,
    QMI_PORT_RMNET_SDIO_2,
    QMI_PORT_RMNET_SDIO_3,
    QMI_PORT_RMNET_SDIO_4,
    QMI_PORT_RMNET_SDIO_5,
    QMI_PORT_RMNET_SDIO_6,
    QMI_PORT_RMNET_SDIO_7
  };
  #define QMI_NAS_DEV_ID_TBL_SIZE (sizeof(dev_id_table)/sizeof(dev_id_table[0]))

  if(!ind_state)
  {
    return QMI_INTERNAL_ERR;
  }
  if(!qmi_err_code)
  {
    return QMI_INTERNAL_ERR;
  }

  /* Iterate over all device connections */
  for( dev_id=0; dev_id<QMI_NAS_DEV_ID_TBL_SIZE; dev_id++)
  {
    /* Register as NAS client */
    rc = qmi_nas_srvc_init_client( dev_id_table[dev_id], NULL, NULL, qmi_err_code );
    if( 0 >= rc )
    {
      if( QMI_PORT_NOT_OPEN_ERR == rc )
      {
        /* Try next device if current not open */
        rc = QMI_NO_ERR;
        continue;
      }
      else
      {
        QMI_ERR_MSG_2 ("qmi_nas_indication_register_all: qmi_nas_srvc_init_client returned error: "
                       "rc=%d qmi_err=%d", rc, *qmi_err_code);
        break;
      }
    }
    qmi_client = rc;

    /* Configure indications */
    rc = qmi_nas_indication_register( qmi_client,ind_state, qmi_err_code );
    if( QMI_NO_ERR != rc )
    {
      QMI_ERR_MSG_2 ("qmi_nas_indication_register_all: qmi_nas_indication_register returned error: "
                     "rc=%d qmi_err=%d", rc, *qmi_err_code);
      break;
    }

    /* Release client registration */
    rc = qmi_nas_srvc_release_client( qmi_client, qmi_err_code);
    if( QMI_NO_ERR != rc )
    {
      QMI_ERR_MSG_2 ("qmi_nas_indication_register_all: qmi_nas_srvc_release_client returned error: "
                     "rc=%d qmi_err=%d", rc, *qmi_err_code);
      break;
    }
  }
  return rc;
} /* qmi_nas_indication_register_all */
