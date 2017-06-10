/******************************************************************************

                        QTI_RMNET_QMI.C

******************************************************************************/

/******************************************************************************

  @file    qti_rmnet_qmux.c
  @brief   Qualcomm Tethering Interface for RMNET tethering. This file
           has functions which interact with QXMUD APIs for RMNET tethering.

  DESCRIPTION
  Has functions which interact with QXMUD APIs for RMNET tethering.

  ---------------------------------------------------------------------------
  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------


******************************************************************************/


/******************************************************************************

                      EDIT HISTORY FOR FILE

when       who        what, where, why
--------   ---        -------------------------------------------------------
01/22/14   sb        Initial version for Fusion

******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/stat.h>
#include <ctype.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <asm/types.h>
#include <fcntl.h>

#include "qti.h"
#include "wireless_data_administrative_service_v01.h"
#include "data_filter_service_v01.h"
#include "wireless_data_service_v01.h"


static  qti_rmnet_param        * rmnet_state_config;
qmi_client_os_params           qti_dfs_os_params;

/*===========================================================================
                               FUNCTION DEFINITIONS
/*=========================================================================*/

/*===========================================================================

FUNCTION SET_VALUE()

DESCRIPTION

  This function
  - allows modifying of values present in QMI message TLVs

DEPENDENCIES
  None.

RETURN VALUE
  QTI_SUCCESS on success
  QTI_FAILURE on failure


SIDE EFFECTS
  None

/*=========================================================================*/

static void set_value
(
  void * data,
  int bytes_processed,
  int size,
  uint32_t change_value
)
{
  uint32_t * value = NULL;
/*------------------------------------------------------------------------*/

  value = malloc(sizeof(uint32));
  if (value == NULL)
  {
    LOG_MSG_ERROR("Could not allocate memory in set_value of size %d ",
                  sizeof(uint32), 0, 0);
    return ;
  }
  *value = change_value;
  memcpy(((uint8_t *)data + bytes_processed),(uint8_t *)value,size);
  free(value);
  return;
}

/*===========================================================================

FUNCTION GET_VALUE()

DESCRIPTION

  This function
  - gets the value of TLV present in QMI message

DEPENDENCIES
  None.

RETURN VALUE
  QTI_SUCCESS on success
  QTI_FAILURE on failure


SIDE EFFECTS
  None

/*=========================================================================*/
static int get_value
(
  void * data,
  int bytes_processed,
  int size
)
{
  int value;
/*------------------------------------------------------------------------*/
  void * value_data = NULL;
  value_data = malloc(size);
  if (value_data == NULL)
  {
    LOG_MSG_ERROR("Could not allocate memory in get_value of size %d ",
                  size, 0, 0);
    return QTI_FAILURE ;
  }
  memcpy((uint8_t *)value_data,((uint8_t *)data + bytes_processed),size);
  value = *(int *)value_data;
  free(value_data);
  return value;
}

/*===========================================================================

FUNCTION QTI_RMNET_TRANSLATE_EP_INFO_PROCESS_REQ()

DESCRIPTION

  This function
  - processes QMI message before sending it to modem.
  - change the USB EP ID to PCIe EP ID


DEPENDENCIES
  None.

RETURN VALUE
  QTI_SUCCESS on success
  QTI_FAILURE on failure


SIDE EFFECTS
  None

/*=========================================================================*/
static int qti_rmnet_translate_ep_info_process_req
(
  qti_qmux_msg_s *qmux_msg,
  uint32_t        qmux_msg_len,
  uint8_t         ep_info_tlv_t
)
{
  int        count;
  void       * data = NULL;
  int        bytes_processed = 0;
  uint8_t    tlv_type;
  uint16_t   tlv_length;
  int        usb_ep_id;
  int        usb_ep_type;
  int        modem_ep_id;
  int        modem_ep_type;
  int        ret_val;
/*-------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------
  Get the Modem EP ID
--------------------------------------------------------------------------*/
  modem_ep_type = rmnet_state_config->modem_iface_ep.ep_type;
  modem_ep_id = rmnet_state_config->modem_iface_ep.peripheral_iface_id;

/*--------------------------------------------------------------------------
  Process the QMI message and translate USB EP ID to Modem EP ID
--------------------------------------------------------------------------*/

  count = (uint16)qmux_msg->sdu.qmux.msg.msg_length;
  LOG_MSG_INFO1(" QMI message length %d", count, 0, 0);
  data = malloc(count);
  if (data == NULL)
  {
    LOG_MSG_ERROR("Could not allocate memory for QMI message in TX ",
                   0, 0, 0);
    return QTI_FAILURE;
  }

  memcpy((uint8_t *)data,((uint8_t *)qmux_msg + QTI_QMUX_SDU_MESSAGE_HDR_LEN_BYTES),count);

  while (bytes_processed < count)
  {
    tlv_type = (uint8)get_value(data, bytes_processed,sizeof(uint8));
    LOG_MSG_INFO1("TLV type tx %d", tlv_type, 0, 0);
    bytes_processed += sizeof(uint8);

    tlv_length = (uint16)get_value(data, bytes_processed,sizeof(uint16));
    LOG_MSG_INFO1("TLV length tx %d", tlv_length, 0, 0);
    bytes_processed += sizeof(uint16);

    if(tlv_type == ep_info_tlv_t)
    {
      usb_ep_type = (uint32)get_value(data, bytes_processed, sizeof(uint32));
      LOG_MSG_INFO1("USB EP type %d", usb_ep_type, 0, 0);
      LOG_MSG_INFO1("Modem EP type %d", modem_ep_type, 0, 0);
      set_value(data, bytes_processed, sizeof(uint32), modem_ep_type);
      bytes_processed += sizeof(uint32);

      usb_ep_id = (uint32)get_value(data, bytes_processed, sizeof(uint32));
      LOG_MSG_INFO1("USB EP ID %d", usb_ep_id, 0, 0);
      LOG_MSG_INFO1("Modem EP ID %d", modem_ep_id, 0, 0);
      set_value(data, bytes_processed, sizeof(uint32), modem_ep_id);
      bytes_processed += sizeof(uint32);
    }
    else
    {
      bytes_processed += tlv_length;
    }
  }

  memcpy(((uint8_t *)qmux_msg + QTI_QMUX_SDU_MESSAGE_HDR_LEN_BYTES),(uint8_t *)data,count);

  free(data);

  return QTI_SUCCESS;

}

/*===========================================================================

FUNCTION QTI_RMNET_WDA_SET_DATA_FORMAT_PROCESS_RESP()

DESCRIPTION

  This function
  - processes QMI WDA response message before sending it to the host.
  - set MRU/MTU on USB and MHI interfaces based on aggregation size
    returned in the WDA set data format response message


DEPENDENCIES
  None.

RETURN VALUE
  QTI_SUCCESS on success
  QTI_FAILURE on failure


SIDE EFFECTS
  None

/*=========================================================================*/

static int qti_rmnet_wda_set_data_format_process_resp
(
  void      *msg,
  uint32_t  qmux_msg_len,
  int       message_id
)
{
  int                                   ret_val;
  uint32_t                              dl_max_size;
  uint32_t                              ul_max_size;
  uint32_t                              ul_aggr_prot;
  uint32_t                              dl_aggr_prot;
  uint32_t                              dl_max_datagrams;
  uint32_t                              ul_max_datagrams;
  qmi_idl_service_object_type           wda_service_object;
  wda_set_data_format_resp_msg_v01      wda_set_data_format_resp;

/*------------------------------------------------------------------------*/
  wda_service_object = wda_get_service_object_v01();

  if ( wda_service_object == NULL )
  {
    LOG_MSG_ERROR(" wda_service_object  is NULL ", 0, 0, 0);
    return QTI_FAILURE;
  }

  memset ( &wda_set_data_format_resp , 0,
           sizeof(wda_set_data_format_resp_msg_v01));

  ret_val = qti_rmnet_qmi_message_decode(
              wda_service_object,
              QMI_IDL_RESPONSE,
              message_id,
              msg,
              qmux_msg_len,
              &wda_set_data_format_resp,
              sizeof(wda_set_data_format_resp_msg_v01));

  if (ret_val == QTI_FAILURE)
  {
    LOG_MSG_ERROR(" WDA_SET_DATA_FORMAT_RESP_MSG decode failure ",
                   0, 0, 0);
    return QTI_FAILURE;
  }

  if ( wda_set_data_format_resp.ul_data_aggregation_protocol_valid)
  {
    ul_aggr_prot = wda_set_data_format_resp.ul_data_aggregation_protocol;
    LOG_MSG_INFO1("ul_aggr_prot rx from modem %d ", ul_aggr_prot, 0, 0);
  }

  if ( wda_set_data_format_resp.dl_data_aggregation_protocol_valid )
  {
    dl_aggr_prot = wda_set_data_format_resp.dl_data_aggregation_protocol;
    LOG_MSG_INFO1("dl_aggr_prot rx from modem %d ", dl_aggr_prot, 0, 0);
  }

  if ( wda_set_data_format_resp.ul_data_aggregation_max_size_valid )
  {
    ul_max_size = wda_set_data_format_resp.ul_data_aggregation_max_size;
    LOG_MSG_INFO1("ul_max_size rx from modem %d ", ul_max_size, 0, 0);
  }

  if ( wda_set_data_format_resp.dl_data_aggregation_max_size_valid )
  {
    dl_max_size = wda_set_data_format_resp.dl_data_aggregation_max_size;
    LOG_MSG_INFO1("dl_max_size rx from modem %d ", dl_max_size, 0, 0);
  }

  if ( wda_set_data_format_resp.ul_data_aggregation_max_datagrams_valid )
  {
    ul_max_datagrams = wda_set_data_format_resp.ul_data_aggregation_max_datagrams;
    LOG_MSG_INFO1("ul_max_datagrams rx from modem %d", ul_max_datagrams, 0, 0);
  }

  if ( wda_set_data_format_resp.dl_data_aggregation_max_datagrams_valid )
  {
    dl_max_datagrams = wda_set_data_format_resp.dl_data_aggregation_max_datagrams;
    LOG_MSG_INFO1("dl_max_datagrams rx from modem %d", dl_max_datagrams, 0, 0);
  }

  ds_system_call("ifconfig rmnet_mhi0 down",
                 strlen("ifconfig rmnet_mhi0 down"));

  ds_system_call("ifconfig usb_rmnet0 down",
                 strlen("ifconfig usb_rmnet0 down"));

/*---------------------------------------------------------------------------
   Set UL aggr size as MRU on USB interface and MTU on MHI interface
---------------------------------------------------------------------------*/

  if (ul_aggr_prot > 0 &&
      ul_max_datagrams == 1)
  {
    ret_val = qti_rmnet_ph_set_mru(DEFAULT_MTU_MRU_VALUE);
    if (ret_val)
    {
      LOG_MSG_ERROR("Failed to set peripheral interface MRU to %d",
                     DEFAULT_MTU_MRU_VALUE, 0, 0);
    }

    ret_val = qti_rmnet_modem_set_mtu(DEFAULT_MTU_MRU_VALUE);
    if (ret_val)
    {
      LOG_MSG_ERROR("Failed to set modem interface MTU to %d",
                    DEFAULT_MTU_MRU_VALUE, 0, 0);
    }
  }
  else if (ul_aggr_prot > 0 &&
           ul_max_datagrams > 1 &&
           ul_max_size > DEFAULT_MTU_MRU_VALUE)
  {
    ret_val = qti_rmnet_ph_set_mru(ul_max_size);
    if (ret_val)
    {
      LOG_MSG_ERROR("Failed to set peripheral interface MRU to %d",
                    ul_max_size, 0, 0);
    }

    ret_val = qti_rmnet_modem_set_mtu(ul_max_size);
    if (ret_val)
    {
      LOG_MSG_ERROR("Failed to set modem interface MTU to %d",
                     ul_max_size, 0, 0);
    }
  }

/*---------------------------------------------------------------------------
   Set DL aggr size as MTU on USB interface and MRU on MHI interface
---------------------------------------------------------------------------*/

  if (dl_aggr_prot > 0 &&
      dl_max_datagrams == 1)
  {
    ret_val = qti_rmnet_ph_set_mtu(DEFAULT_MTU_MRU_VALUE);
    if (ret_val)
    {
      LOG_MSG_ERROR("Failed to set peripheral interface MTU to %d",
                     DEFAULT_MTU_MRU_VALUE, 0, 0);
    }

    ret_val = qti_rmnet_modem_set_mru(DEFAULT_MTU_MRU_VALUE);
    if (ret_val)
    {
      LOG_MSG_ERROR("Failed to set modem interface MRU to %d",
                    DEFAULT_MTU_MRU_VALUE, 0, 0);
    }
  }
  else if (dl_aggr_prot > 0 &&
           dl_max_datagrams > 1 &&
           dl_max_size > DEFAULT_MTU_MRU_VALUE)
  {
    ret_val = qti_rmnet_ph_set_mtu(dl_max_size);
    if (ret_val)
    {
      LOG_MSG_ERROR("Failed to set peripheral interface MTU to %d",
                     dl_max_size, 0, 0);
    }

    ret_val = qti_rmnet_modem_set_mru(dl_max_size);
    if (ret_val)
    {
      LOG_MSG_ERROR("Failed to set modem interface MRU to %d",
                    dl_max_size, 0, 0);
    }
  }


  ds_system_call("ifconfig rmnet_mhi0 up",
                 strlen("ifconfig rmnet_mhi0 up"));

  ds_system_call("ifconfig usb_rmnet0 up",
                 strlen("ifconfig usb_rmnet0 up"));


  return QTI_SUCCESS;
}

/*===========================================================================

FUNCTION QTI_RMNET_PROCESS_QMI_TX_TO_MODEM()

DESCRIPTION

  This function
  - processes QMI messages from peripheral interface to modem.
  - processes WDA set data format request.
  - processes WDS bind MUX data port request.

DEPENDENCIES
  None.

RETURN VALUE
  QTI_SUCCESS on success
  QTI_FAILURE on failure


SIDE EFFECTS
  None

=========================================================================*/
int qti_rmnet_process_qmi_tx_to_modem
(
  void             *data,
  uint32_t         qmux_msg_len
)
{
  int                          ret = QTI_SUCCESS;
  boolean                      ret_val;
  int                          svc_type;
  int                          message_id;
  qti_qmux_msg_s               *qmux_msg;

/*-------------------------------------------------------------------------*/

  qmux_msg = (qti_qmux_msg_s*)data;

  if(qmux_msg == NULL)
  {
    LOG_MSG_ERROR("Invalid QMUX message", 0, 0, 0);
    return QTI_FAILURE;
  }

  svc_type = (uint8)qmux_msg->qmux_hdr.svc_type;
  message_id = (uint16)qmux_msg->sdu.qmux.msg.msg_id;

  LOG_MSG_INFO1("Received QMI message of service type %d message id %d",
                svc_type,
                message_id,
                0);

  switch (svc_type)
  {
     case WDA_SERVICE_ID:
       ret_val = qti_rmnet_process_wda_message_to_modem (data,
                                                         qmux_msg_len,
                                                         message_id);
       if (ret_val != QTI_SUCCESS)
       {
         LOG_MSG_ERROR("Failed to process WDA service msg %d", message_id, 0, 0);
         ret = QTI_FAILURE;
       }
       break;

     case WDS_SERVICE_ID:
       ret_val = qti_rmnet_process_wds_message_to_modem (data,
                                                         qmux_msg_len,
                                                         message_id);
       if (ret_val != QTI_SUCCESS)
       {
         LOG_MSG_ERROR("Failed to process WDS service msg %d", message_id, 0, 0);
         ret = QTI_FAILURE;
       }
       break;

    case QMI_QOS_SERVICE_ID:
      if (message_id == QMI_QOS_BIND_DATA_PORT)
      {
         LOG_MSG_INFO1("Got QMI QOS bind data port req", 0, 0, 0);
         ret_val = qti_rmnet_translate_ep_info_process_req(qmux_msg,
                                                           qmux_msg_len,
                                                           QMI_QOS_BIND_DATA_PORT_REQ_DATA_EP_ID_TYPE);
         if (ret_val != QTI_SUCCESS)
         {
           LOG_MSG_ERROR("Failed to parse QMI QOS bind data port req", 0, 0, 0);
           ret = QTI_FAILURE;
         }
      }
      break;

    case DFS_SERVICE_ID:
      ret_val = qti_rmnet_process_dfs_message_to_modem(data,
                                                       qmux_msg_len,
                                                       message_id);
      if (ret_val != QTI_SUCCESS)
      {
        LOG_MSG_ERROR("Failed to process DFS service msg %d", message_id, 0, 0);
        ret = QTI_FAILURE;
      }
      break;

    default:
     break;
  }

  return ret;

}


/*===========================================================================

FUNCTION QTI_RMNET_PROCESS_QMI_RX_FROM_MODEM()

DESCRIPTION

  This function
  - receives all messages from modem and passes onto USB interface.
  - processes  WDA set data format response message.

DEPENDENCIES
  None.

RETURN VALUE
  QTI_SUCCESS on success
  QTI_FAILURE on failure


SIDE EFFECTS
  None

=========================================================================*/


int qti_rmnet_process_qmi_rx_from_modem
(
  void            *data,
  uint32_t        qmux_msg_len
)
{

  int                          ret = QTI_SUCCESS;
  boolean                      ret_val;
  int                          svc_type;
  int                          message_id;
  qti_qmux_msg_s               *qmux_msg;

/*-------------------------------------------------------------------------*/

  qmux_msg = (qti_qmux_msg_s*)data;
  if(qmux_msg == NULL)
  {
    LOG_MSG_ERROR("Invalid QMUX message", 0, 0, 0);
    return QTI_FAILURE;
  }

  svc_type = (uint8)qmux_msg->qmux_hdr.svc_type;
  message_id = (uint16)qmux_msg->sdu.qmux.msg.msg_id;

  LOG_MSG_INFO1("Received QMI message of service type %d", svc_type, 0, 0);

  switch (svc_type)
  {
     case WDA_SERVICE_ID:
       if (message_id == WDA_SET_DATA_FORMAT_MESSAGE_ID)
       {
         if( DS_TARGET_FUSION4_5_PCIE == rmnet_state_config->target)
         {
           LOG_MSG_INFO1("**** Got WDA SET DATA FORMAT message ****", 0, 0, 0);
           ret_val = qti_rmnet_wda_set_data_format_process_resp(data,
                                                                qmux_msg_len,
                                                                message_id);
           if (ret_val != QTI_SUCCESS)
           {
             LOG_MSG_ERROR("Failed to process WDS service msg %d",
                           message_id, 0, 0);
             ret = QTI_FAILURE;
           }
         }
       }
       break;

    case WDS_SERVICE_ID:
      ret_val = qti_rmnet_process_wds_message_from_modem(data,
                                                         qmux_msg_len,
                                                         message_id);
      if (ret_val != QTI_SUCCESS)
      {
        LOG_MSG_ERROR("Failed to process WDS service msg %d", message_id, 0, 0);
        ret = QTI_FAILURE;
      }
      break;

    case DFS_SERVICE_ID:
      ret_val = qti_rmnet_process_dfs_message_from_modem(data,
                                                         qmux_msg_len,
                                                         message_id);
      if (ret_val != QTI_SUCCESS)
      {
        LOG_MSG_ERROR("Failed to process DFS service msg %d", message_id, 0, 0);
        ret = QTI_FAILURE;
      }
      break;

    default:
      break;
  }

  return ret;

}

/*===========================================================================

FUNCTION QTI_RMNET_DFS_IND_CB()

DESCRIPTION
  Handles DFS client indications

DEPENDENCIES
  None.

RETURN VALUE
  QTI_SUCCESS on success
  QTI_FAILURE on failure


SIDE EFFECTS
  None

=========================================================================*/
void qti_rmnet_dfs_ind_cb
(
  qmi_client_type               user_handle,      /* QMI user handle       */
  unsigned int                  msg_id,           /* Indicator message ID  */
  void                          *ind_buf,         /* Raw indication data   */
  int                           ind_buf_len,      /* Raw data length       */
  void                         *ind_cb_data      /* User call back handle */
)
{
  qmi_client_error_type                 qmi_err;
  dfs_low_latency_traffic_ind_msg_v01   low_latency_traffic_status;

/*-----------------------------------------------------------------------*/

  LOG_MSG_INFO1("Received DFS IND", 0, 0, 0);

  if (ind_buf == NULL ||
      rmnet_state_config->qti_dfs_handle != user_handle)
  {
    LOG_MSG_ERROR("Invalid indication ind_buf=0x%x",
                  ind_buf,
                  0,
                  0);
    return;
  }

  LOG_MSG_INFO1("Received DFS IND id %d", msg_id, 0, 0);

  switch (msg_id)
  {
    case QMI_DFS_LOW_LATENCY_TRAFFIC_IND_V01:
      qmi_err = qmi_client_message_decode(user_handle,
                                          QMI_IDL_INDICATION,
                                          msg_id,
                                          ind_buf,
                                          ind_buf_len,
                                          &low_latency_traffic_status,
                                          sizeof(low_latency_traffic_status));
      if(QMI_NO_ERR != qmi_err)
      {
        LOG_MSG_ERROR("Invalid filter mode ind msg error %d",
                      qmi_err, 0, 0);
        return;
      }

      if (!low_latency_traffic_status.traffic_start)
      {
        qti_rmnet_modem_change_sleep_state(TRUE);
      }
      else
      {
        qti_rmnet_modem_change_sleep_state(FALSE);
      }
      break;

    default:
      LOG_MSG_INFO1("No handler for DFS IND id %d", msg_id, 0, 0);
  }
  return;
}

/*===========================================================================

FUNCTION QTI_RMNET_DFS_RELEASE()

DESCRIPTION
  Release the DFS QMI client

DEPENDENCIES
  None.

RETURN VALUE
  QTI_SUCCESS on success
  QTI_FAILURE on failure


SIDE EFFECTS
  None

=========================================================================*/

int qti_rmnet_dfs_release()
{

  qmi_client_error_type                 qmi_error;

/*------------------------------------------------------------------------*/

  qmi_error = qmi_client_release(rmnet_state_config->qti_dfs_handle);
  if (qmi_error != QMI_NO_ERR)
  {
    LOG_MSG_ERROR("Can not init DFS client %d", qmi_error, 0, 0);
    return QTI_FAILURE;
  }

  LOG_MSG_INFO1("Successfully deregistered DFS client", 0, 0, 0);

  qti_rmnet_modem_change_sleep_state(TRUE);

  return QTI_SUCCESS;
}

/*===========================================================================

FUNCTION QTI_RMNET_DFS_INIT()

DESCRIPTION
  Initializes a DFS QMI client

DEPENDENCIES
  None.

RETURN VALUE
  QTI_SUCCESS on success
  QTI_FAILURE on failure


SIDE EFFECTS
  None

=========================================================================*/

int qti_rmnet_dfs_init()
{

  qmi_idl_service_object_type           qti_dfs_service_object;
  qmi_client_error_type                 qmi_error, qmi_err_code = QMI_NO_ERR;
  dfs_indication_register_req_msg_v01   dfs_ind_reg_request;
  dfs_indication_register_resp_msg_v01  dfs_ind_reg_response;
  int                                   ret;
/*---------------------------------------------------------------------------*/

  LOG_MSG_INFO1("qti_dfs_init", 0, 0, 0);
/*-----------------------------------------------------------------------------
  Obtain a DFS client for QTI
  - get the service object
  - obtain the client
------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
  Get the service object
------------------------------------------------------------------------------*/
  qti_dfs_service_object = dfs_get_service_object_v01();
  if (qti_dfs_service_object == NULL)
  {
    LOG_MSG_ERROR("QTI DFS service object not available",
                   0, 0, 0);
    return QTI_FAILURE;
  }

  memset(&qti_dfs_os_params, 0, sizeof(qmi_client_os_params));
/*-----------------------------------------------------------------------------
  Client init
------------------------------------------------------------------------------*/
  qmi_error = qmi_client_init_instance(qti_dfs_service_object,
                                       QMI_CLIENT_INSTANCE_ANY,
                                       (void *)qti_rmnet_dfs_ind_cb,
                                       NULL,
                                       &qti_dfs_os_params,
                                       QTI_QMI_MSG_TIMEOUT_VALUE,
                                       &(rmnet_state_config->qti_dfs_handle));

  if (qmi_error != QMI_NO_ERR)
  {
    LOG_MSG_ERROR("Can not init DFS client %d", qmi_error, 0, 0);
    return QTI_FAILURE;
  }

  LOG_MSG_INFO1("Successfully registered DFS client", 0, 0, 0);

  memset(&dfs_ind_reg_request, 0, sizeof(dfs_indication_register_req_msg_v01));
  memset(&dfs_ind_reg_response, 0, sizeof(dfs_indication_register_resp_msg_v01));

  dfs_ind_reg_request.report_low_latency_traffic_valid = TRUE;
  dfs_ind_reg_request.report_low_latency_traffic = TRUE;

  qmi_error = qmi_client_send_msg_sync(rmnet_state_config->qti_dfs_handle,
                                QMI_DFS_INDICATION_REGISTER_REQ_V01,
                                &dfs_ind_reg_request,
                                sizeof(dfs_indication_register_req_msg_v01),
                                &dfs_ind_reg_response,
                                sizeof(dfs_indication_register_resp_msg_v01),
                                QTI_QMI_MSG_TIMEOUT_VALUE);

  if (qmi_error != QMI_NO_ERR ||
      dfs_ind_reg_response.resp.result != QMI_RESULT_SUCCESS_V01 )
  {
    LOG_MSG_ERROR("DFS ind registration failed %d %d",
                  qmi_error,
                  dfs_ind_reg_response.resp.error,
                  0);
    return QTI_FAILURE;
  }

  LOG_MSG_INFO1("Successfully registered for low latency ind", 0, 0, 0);

  return QTI_SUCCESS;
}


/*===========================================================================

FUNCTION QTI_RMNET_QMI_INIT()

DESCRIPTION

  This function
  - initializes RmNet state for QMI message processing

DEPENDENCIES
  None.

RETURN VALUE
  QTI_SUCCESS on success
  QTI_FAILURE on failure


SIDE EFFECTS
  None

=========================================================================*/
int qti_rmnet_qmi_init
(
   qti_rmnet_param       * rmnet_state
)
{
  rmnet_state_config = rmnet_state;

  return QTI_SUCCESS;
}



/*===========================================================

FUNCTION QTI_RMNET_PROCESS_WDA_MESSAGE_TO_MODEM()

DESCRIPTION
  This function does EP translation for all the WDA messages to modem.

DEPENDENCIES
  None.

RETURN VALUE
  0 on success
  -1 on failure

SIDE EFFECTS
  None

============================================================*/

int qti_rmnet_process_wda_message_to_modem
(
  void           *msg,
  uint32_t        qmux_msg_len,
  int             message_id
)
{
  int                                      ret_val;
  uint32_t                                 *ep_id_ptr;
  data_ep_type_enum_v01                    *ep_type_ptr;
  int                                      mhi_ep_id;
  qmi_idl_service_object_type              wda_service_object;
  wda_set_data_format_req_msg_v01          wda_set_data_format_req;
  wda_get_data_format_req_msg_v01          wda_get_data_format_req;
  wda_set_qmap_settings_req_msg_v01        wda_set_qmap_settings_req;
  wda_get_qmap_settings_req_msg_v01        wda_get_qmap_settings_req;
/*---------------------------------------------------------------------------*/

  wda_service_object = wda_get_service_object_v01();

  if ( wda_service_object == NULL )
  {
    LOG_MSG_ERROR(" wda_service_object  is NULL ", 0, 0, 0);
    return QTI_FAILURE;
  }

  switch(message_id)
  {
    case WDA_SET_DATA_FORMAT_MESSAGE_ID:
      {
        LOG_MSG_INFO1("**** Got WDA SET DATA FORMAT message ****", 0, 0, 0);

        if( DS_TARGET_FUSION4_5_PCIE == rmnet_state_config->target)
        {
          ret_val = qti_rmnet_modem_get_ep_id (&mhi_ep_id);
          if (ret_val != QTI_SUCCESS)
          {
            LOG_MSG_ERROR("Failed to get MHI EP ID", 0, 0, 0);
            return QTI_FAILURE;
          }
          rmnet_state_config->modem_iface_ep.ep_type = DATA_EP_TYPE_PCIE;
          rmnet_state_config->modem_iface_ep.peripheral_iface_id = mhi_ep_id;
        }
        else if ( DS_TARGET_JOLOKIA== rmnet_state_config->target)
        {
          rmnet_state_config->modem_iface_ep.peripheral_iface_id =\
                    rmnet_state_config->ep_info.ph_ep_info.peripheral_iface_id;
          rmnet_state_config->modem_iface_ep.ep_type = \
                                 rmnet_state_config->ep_info.ph_ep_info.ep_type;
        }

/*-------------------------------------------------------------------
  Decode the QMI message
 --------------------------------------------------------------------*/
        memset(&wda_set_data_format_req, 0,
               sizeof(wda_set_data_format_req_msg_v01));

        ret_val = qti_rmnet_qmi_message_decode(
                    wda_service_object,
                    QMI_IDL_REQUEST,
                    message_id,
                    msg ,
                    qmux_msg_len,
                    &wda_set_data_format_req,
                    sizeof(wda_set_data_format_req_msg_v01));

        if (ret_val == QTI_FAILURE)
        {
          LOG_MSG_ERROR(" WDA_SET_DATA_FORMAT_MESSAGE decode failure ",
                        0, 0 ,0);
          return QTI_FAILURE;
        }

/*---------------------------------------------------------------------------
Only if the EP info is valid , do the translation
-----------------------------------------------------------------------------*/
        if ( wda_set_data_format_req.ep_id_valid )
        {
          rmnet_state_config->ph_iface_ep.peripheral_iface_id = \
                                         wda_set_data_format_req.ep_id.iface_id;
          rmnet_state_config->ph_iface_ep.ep_type = \
                                         wda_set_data_format_req.ep_id.ep_type;

          ep_id_ptr = &(wda_set_data_format_req.ep_id.iface_id);
          ep_type_ptr = &(wda_set_data_format_req.ep_id.ep_type);

          qti_rmnet_ep_info_translation_req ( ep_id_ptr, ep_type_ptr);

/*---------------------------------------------------------------------------
  Encode the updated EP info in QMI format
----------------------------------------------------------------------------*/
          ret_val = qti_rmnet_qmi_message_encode(
                      wda_service_object,
                      QMI_IDL_REQUEST,
                      message_id,
                      &wda_set_data_format_req,
                      sizeof(wda_set_data_format_req_msg_v01),
                      msg,
                      qmux_msg_len);

          if (ret_val == QTI_FAILURE)
          {
            LOG_MSG_ERROR(" WDA_SET_DATA_FORMAT_MESSAGE encode failure ",
                          0, 0 ,0);
            return QTI_FAILURE;
          }
        }
        else
        {
          LOG_MSG_INFO1("USB EP ID Not valid for WDA_SET_DATA_FORMAT_MESSAGE",
                        0, 0, 0);
        }
      }
      break;

    case WDA_GET_DATA_FORMAT_MSG_ID:
      {
        LOG_MSG_INFO1("Got WDA GET DATA FORMAT message", 0, 0, 0);

        memset(&wda_get_data_format_req, 0,
               sizeof(wda_get_data_format_req_msg_v01));

        ret_val = qti_rmnet_qmi_message_decode(
                    wda_service_object,
                    QMI_IDL_REQUEST,
                    message_id,
                    msg ,
                    qmux_msg_len,
                    &wda_get_data_format_req,
                    sizeof(wda_get_data_format_req_msg_v01));

        if (ret_val == QTI_FAILURE)
        {
          LOG_MSG_ERROR(" WDA_GET_DATA_FORMAT_MSG decode failure ", 0, 0 ,0);
          return QTI_FAILURE;
        }

        if ( wda_get_data_format_req.ep_id_valid )
        {
          ep_id_ptr = &(wda_get_data_format_req.ep_id.iface_id);
          ep_type_ptr = &(wda_get_data_format_req.ep_id.ep_type);

          qti_rmnet_ep_info_translation_req ( ep_id_ptr, ep_type_ptr);

          ret_val = qti_rmnet_qmi_message_encode(
                      wda_service_object,
                      QMI_IDL_REQUEST,
                      message_id,
                      &wda_get_data_format_req,
                      sizeof(wda_get_data_format_req_msg_v01),
                      msg,
                      qmux_msg_len);

          if (ret_val == QTI_FAILURE)
          {
            LOG_MSG_ERROR(" WDA_GET_DATA_FORMAT_MSG encode failure ", 0, 0 ,0);
            return QTI_FAILURE;
          }
        }
        else
        {
          LOG_MSG_INFO1("USB EP ID Not valid for WDA_GET_DATA_FORMAT_MSG",
                        0, 0, 0);
        }
      }
      break;

    case WDA_SET_QMAP_SETTINGS_MSG_ID:
      {
        LOG_MSG_INFO1("Got WDA set QMAP settings message", 0, 0, 0);

        memset(&wda_set_qmap_settings_req, 0,
               sizeof(wda_set_qmap_settings_req_msg_v01));

        ret_val = qti_rmnet_qmi_message_decode(
                    wda_service_object,
                    QMI_IDL_REQUEST,
                    message_id,
                    msg ,
                    qmux_msg_len,
                    &wda_set_qmap_settings_req,
                    sizeof(wda_set_qmap_settings_req_msg_v01));

        if (ret_val == QTI_FAILURE)
        {
          LOG_MSG_ERROR(" WDA_SET_QMAP_SETTINGS_MSG decode failure ", 0, 0 ,0);
          return QTI_FAILURE;
        }


        if ( wda_set_qmap_settings_req.ep_id_valid )
        {
          ep_id_ptr = &(wda_set_qmap_settings_req.ep_id.iface_id);
          ep_type_ptr = &(wda_set_qmap_settings_req.ep_id.ep_type);

          qti_rmnet_ep_info_translation_req ( ep_id_ptr, ep_type_ptr);

          ret_val = qti_rmnet_qmi_message_encode(
                      wda_service_object,
                      QMI_IDL_REQUEST,
                      message_id,
                      &wda_set_qmap_settings_req,
                      sizeof(wda_set_qmap_settings_req_msg_v01),
                      msg,
                      qmux_msg_len);
          if (ret_val == QTI_FAILURE)
          {
            LOG_MSG_ERROR(" WDA_SET_QMAP_SETTINGS_MSG encode failure ", 0, 0 ,0);
            return QTI_FAILURE;
          }
        }
        else
        {
          LOG_MSG_INFO1("USB EP ID Not valid for WDA_SET_QMAP_SETTINGS_MSG",
                        0 , 0, 0);
        }
      }
      break;

    case WDA_GET_QMAP_SETTINGS_MSG_ID:
      {
        LOG_MSG_INFO1("Got WDA get QMAP settings message", 0, 0, 0);

        memset(&wda_get_qmap_settings_req, 0,
               sizeof(wda_get_qmap_settings_req_msg_v01));

        ret_val = qti_rmnet_qmi_message_decode(
                    wda_service_object,
                    QMI_IDL_REQUEST,
                    message_id,
                    msg ,
                    qmux_msg_len,
                    &wda_get_qmap_settings_req,
                    sizeof(wda_get_qmap_settings_req_msg_v01));

        if (ret_val == QTI_FAILURE)
        {
          LOG_MSG_ERROR(" WDA_GET_QMAP_SETTINGS_MSG decode failure ", 0, 0 ,0);
          return QTI_FAILURE;
        }


        if ( wda_get_qmap_settings_req.ep_id_valid )
        {
          ep_id_ptr = &(wda_get_qmap_settings_req.ep_id.iface_id);
          ep_type_ptr = &(wda_get_qmap_settings_req.ep_id.ep_type);

          qti_rmnet_ep_info_translation_req ( ep_id_ptr, ep_type_ptr);

          ret_val = qti_rmnet_qmi_message_encode(
                      wda_service_object,
                      QMI_IDL_REQUEST,
                      message_id,
                      &wda_get_qmap_settings_req,
                      sizeof(wda_get_qmap_settings_req_msg_v01),
                      msg,
                      qmux_msg_len);
          if (ret_val == QTI_FAILURE)
          {
            LOG_MSG_ERROR(" WDA_GET_QMAP_SETTINGS_MSG encode failure ", 0, 0 ,0);
            return QTI_FAILURE;
          }
        }
        else
        {
          LOG_MSG_INFO1("USB EP ID Not valid for WDA_GET_QMAP_SETTINGS_MSG",
                        0 , 0, 0);
        }
      }
      break;
    default:
      break;
  }

  return QTI_SUCCESS;
}

/*===========================================================

FUNCTION QTI_RMNET_PROCESS_WDS_MESSAGE_TO_MODEM()

DESCRIPTION
  This function does EP translation for all the WDS messages to modem

DEPENDENCIES
  None.

RETURN VALUE
  0 on success
  -1 on failure

SIDE EFFECTS
  None

============================================================*/
int qti_rmnet_process_wds_message_to_modem
(
  void     *msg,
  uint32_t qmux_msg_len,
  int      message_id
)
{

  int                                      ret_val;
  uint32_t                                 *ep_id_ptr;
  data_ep_type_enum_v01                    *ep_type_ptr;
  qmi_idl_service_object_type              wds_service_object;
  wds_bind_mux_data_port_req_msg_v01       wds_bind_mux_data_port_req;
/*---------------------------------------------------------------------------*/

  wds_service_object = wds_get_service_object_v01();

  if ( wds_service_object == NULL )
  {
    LOG_MSG_ERROR(" wds_service_object  is NULL ", 0, 0, 0);
    return QTI_FAILURE;
  }

  switch (message_id)
  {
    case WDS_BIND_MUX_DATA_PORT:
      {
        LOG_MSG_INFO1("**** Got WDS BIND MUX DATA PORT message ****", 0, 0, 0);
/*-------------------------------------------------------------------
Decode the QMI message
 --------------------------------------------------------------------*/
        memset (&wds_bind_mux_data_port_req, 0,
                sizeof(wds_bind_mux_data_port_req_msg_v01));

        ret_val = qti_rmnet_qmi_message_decode(
                    wds_service_object,
                    QMI_IDL_REQUEST,
                    message_id,
                    msg,
                    qmux_msg_len,
                    &wds_bind_mux_data_port_req,
                    sizeof(wds_bind_mux_data_port_req_msg_v01));

        if (ret_val == QTI_FAILURE)
        {
          LOG_MSG_ERROR(" WDS_BIND_MUX_DATA_PORT decode failure ",
                        0, 0 ,0);
          return QTI_FAILURE;
        }

        if ( wds_bind_mux_data_port_req.ep_id_valid )
        {
          ep_id_ptr = &(wds_bind_mux_data_port_req.ep_id.iface_id);
          ep_type_ptr = &(wds_bind_mux_data_port_req.ep_id.ep_type);

/*---------------------------------------------------------------------------
  Only if the EP info is valid , do the translation
-----------------------------------------------------------------------------*/
          qti_rmnet_ep_info_translation_req( ep_id_ptr, ep_type_ptr);

/*---------------------------------------------------------------------------
  Encode the updated EP info in QMI format
----------------------------------------------------------------------------*/

          ret_val = qti_rmnet_qmi_message_encode(
                      wds_service_object,
                      QMI_IDL_REQUEST,
                      message_id,
                      &wds_bind_mux_data_port_req,
                      sizeof(wds_bind_mux_data_port_req_msg_v01),
                      msg,
                      qmux_msg_len);
          if (ret_val == QTI_FAILURE)
          {
            LOG_MSG_ERROR(" WDS_BIND_MUX_DATA_PORT encode failure ", 0, 0 ,0);
            return QTI_FAILURE;
          }
        }
        else
        {
          LOG_MSG_INFO1("USB EP ID Not valid for WDS_BIND_MUX_DATA_PORT",
                        0, 0, 0);
        }
      }
      break;
    default:
      break;
  }
  return QTI_SUCCESS;
}



/*===========================================================

FUNCTION QTI_RMNET_PROCESS_DFS_MESSAGE_TO_MODEM()

DESCRIPTION
  This function does EP translation for all the DFS messages to modem.

DEPENDENCIES
  None.

RETURN VALUE
  0 on success
  -1 on failure

SIDE EFFECTS
  None

============================================================*/

int qti_rmnet_process_dfs_message_to_modem
(
  void     *msg,
  uint32_t qmux_msg_len,
  int      message_id
)
{

  int                                      ret_val;
  uint32_t                                 *ep_id_ptr;
  data_ep_type_enum_v01                    *ep_type_ptr;
  qmi_idl_service_object_type              dfs_service_object;
  dfs_bind_client_req_msg_v01              dfs_bind_client_req;
/*---------------------------------------------------------------------------*/

  dfs_service_object = dfs_get_service_object_v01();

  if ( dfs_service_object == NULL )
  {
    LOG_MSG_ERROR(" dfs_service_object  is NULL ", 0, 0, 0);
    return QTI_FAILURE;
  }

  switch (message_id)
  {
    case DFS_BIND_CLIENT:
      {
        LOG_MSG_INFO1("**** Got DFS_BIND_CLIENT message ****", 0, 0, 0);
/*-------------------------------------------------------------------
Decode the QMI message
 --------------------------------------------------------------------*/
        memset (&dfs_bind_client_req, 0,
                sizeof(dfs_bind_client_req_msg_v01));

        ret_val = qti_rmnet_qmi_message_decode(
                    dfs_service_object,
                    QMI_IDL_REQUEST,
                    message_id,
                    msg,
                    qmux_msg_len,
                    &dfs_bind_client_req,
                    sizeof(dfs_bind_client_req_msg_v01));

        if (ret_val == QTI_FAILURE)
        {
          LOG_MSG_ERROR(" DFS_BIND_CLIENT decode failure ",
                        0, 0 ,0);
          return QTI_FAILURE;
        }

        if ( dfs_bind_client_req.ep_id_valid )
        {
          ep_id_ptr = &(dfs_bind_client_req.ep_id.iface_id);
          ep_type_ptr = &(dfs_bind_client_req.ep_id.ep_type);

/*---------------------------------------------------------------------------
  Only if the EP info is valid , do the translation
-----------------------------------------------------------------------------*/
          qti_rmnet_ep_info_translation_req( ep_id_ptr, ep_type_ptr);

/*---------------------------------------------------------------------------
  Encode the updated EP info in QMI format
----------------------------------------------------------------------------*/

          ret_val = qti_rmnet_qmi_message_encode(
                      dfs_service_object,
                      QMI_IDL_REQUEST,
                      message_id,
                      &dfs_bind_client_req,
                      sizeof(dfs_bind_client_req_msg_v01),
                      msg,
                      qmux_msg_len);
          if (ret_val == QTI_FAILURE)
          {
            LOG_MSG_ERROR(" DFS_BIND_CLIENT encode failure ", 0, 0 ,0);
            return QTI_FAILURE;
          }
        }
        else
        {
          LOG_MSG_INFO1("USB EP ID Not valid for DFS_BIND_CLIENT",
                        0, 0, 0);
        }
      }
      break;
    default:
      break;
  }
  return QTI_SUCCESS;


}

/*===========================================================

FUNCTION QTI_RMNET_PROCESS_WDS_MESSAGE_FROM_MODEM()

DESCRIPTION
  This function does EP translation for all the WDS messages from modem

DEPENDENCIES
  None.

RETURN VALUE
  0 on success
  -1 on failure

SIDE EFFECTS
  None

============================================================*/
int qti_rmnet_process_wds_message_from_modem
(
  void        *msg,
  uint32_t    qmux_msg_len,
  int         message_id
)
{
  int                                        i;
  int                                        ret_val;
  uint32_t                                   *ep_id_ptr;
  data_ep_type_enum_v01                      *ep_type_ptr;
  uint8_t                                    thruput_info_instances;
  qmi_idl_service_object_type                wds_service_object;
  wds_start_network_interface_resp_msg_v01   wds_start_network_interface_resp;
  wds_get_last_throughput_info_resp_msg_v01  wds_get_last_throughput_info_resp;
/*---------------------------------------------------------------------------*/

  wds_service_object = wds_get_service_object_v01();

  if ( wds_service_object == NULL )
  {
    LOG_MSG_ERROR(" wds_service_object  is NULL ", 0, 0, 0);
    return QTI_FAILURE;
  }

  switch (message_id)
  {
    case WDS_START_NETWORK_INTERFACE_MSG_ID:
      {
        LOG_MSG_INFO1("Got WDS start network interface response", 0, 0, 0);

        memset (&wds_start_network_interface_resp, 0 ,
                sizeof(wds_start_network_interface_resp_msg_v01));

        ret_val = qti_rmnet_qmi_message_decode(
                    wds_service_object,
                    QMI_IDL_RESPONSE,
                    message_id,
                    msg,
                    qmux_msg_len,
                    &wds_start_network_interface_resp,
                    sizeof(wds_start_network_interface_resp_msg_v01));

        if (ret_val == QTI_FAILURE)
        {
          LOG_MSG_ERROR(" WDS_START_NETWORK_INTERFACE_MSG decode failure ",
                        0, 0 ,0);
          return QTI_FAILURE;
        }

        if ( wds_start_network_interface_resp.ep_id_valid )
        {
          ep_id_ptr = &(wds_start_network_interface_resp.ep_id.iface_id);
          ep_type_ptr = &(wds_start_network_interface_resp.ep_id.ep_type);

          qti_rmnet_ep_info_translation_resp( ep_id_ptr, ep_type_ptr);

          ret_val = qti_rmnet_qmi_message_encode(
                      wds_service_object,
                      QMI_IDL_RESPONSE,
                      message_id,
                      &wds_start_network_interface_resp,
                      sizeof(wds_start_network_interface_resp_msg_v01),
                      msg,
                      qmux_msg_len);
          if (ret_val == QTI_FAILURE)
          {
            LOG_MSG_ERROR(" WDS_START_NETWORK_INTERFACE_MSG encode failure ",
                          0, 0 ,0);
            return QTI_FAILURE;
          }
        }
        else
        {
          LOG_MSG_INFO1("USB EP ID Not valid for WDS_START_NETWORK_INTERFACE_MSG",
                        0, 0, 0);
        }
      }
      break;
    case WDS_GET_LAST_THROUGHPUT_INFO_MSG_ID:
      {
        LOG_MSG_INFO1("Got WDS get last throughput info response", 0, 0, 0);

        memset (&wds_get_last_throughput_info_resp, 0,
                sizeof(wds_get_last_throughput_info_resp_msg_v01));

        ret_val = qti_rmnet_qmi_message_decode(
                    wds_service_object,
                    QMI_IDL_RESPONSE,
                    message_id,
                    msg,
                    qmux_msg_len,
                    &wds_get_last_throughput_info_resp,
                    sizeof(wds_get_last_throughput_info_resp_msg_v01));

        if (ret_val == QTI_FAILURE)
        {
          LOG_MSG_ERROR(" QMI_WDS_GET_LAST_THROUGHPUT_INFO_RESP_MSG decode"
                        " failure ", 0, 0 ,0);
          return QTI_FAILURE;
        }

        if( wds_get_last_throughput_info_resp.throughput_info_valid )
        {
          thruput_info_instances = wds_get_last_throughput_info_resp.throughput_info_len;

          for ( i =0 ; i < thruput_info_instances ; i++)
          {
            ep_id_ptr = &(wds_get_last_throughput_info_resp.throughput_info[i].ep_id.iface_id);
            ep_type_ptr = &(wds_get_last_throughput_info_resp.throughput_info[i].ep_id.ep_type);

            qti_rmnet_ep_info_translation_resp( ep_id_ptr, ep_type_ptr);
          }

          ret_val = qti_rmnet_qmi_message_encode(
                      wds_service_object,
                      QMI_IDL_RESPONSE,
                      message_id,
                      &wds_get_last_throughput_info_resp,
                      sizeof(wds_get_last_throughput_info_resp_msg_v01),
                      msg,
                      qmux_msg_len);
          if (ret_val == QTI_FAILURE)
          {
            LOG_MSG_ERROR(" QMI_WDS_GET_LAST_THROUGHPUT_INFO_RESP_MSG "
            " encode failure ", 0, 0 ,0);
            return QTI_FAILURE;
          }
        }
        else
        {
          LOG_MSG_INFO1("Thruput info Not valid for "
                        "QMI_WDS_GET_LAST_THROUGHPUT_INFO_RESP_MSG", 0, 0, 0);
        }
      }
      break;
    default:
      break;
  }
  return QTI_SUCCESS;
}

/*===========================================================

FUNCTION QTI_RMNET_PROCESS_DFS_MESSAGE_FROM_MODEM()

DESCRIPTION
  This function does EP translation for all the DFS messages from modem

DEPENDENCIES
  None.

RETURN VALUE
  0 on success
  -1 on failure

SIDE EFFECTS
  None

============================================================*/
int qti_rmnet_process_dfs_message_from_modem
(
  void        *msg,
  uint32_t    qmux_msg_len,
  int         message_id
)
{

  int                                      ret_val;
  uint32_t                                 *ep_id_ptr;
  data_ep_type_enum_v01                    *ep_type_ptr;
  qmi_idl_service_object_type              dfs_service_object;
  dfs_get_client_binding_resp_msg_v01      dfs_get_client_binding_resp;
/*---------------------------------------------------------------------------*/

  dfs_service_object = dfs_get_service_object_v01();

  if ( dfs_service_object == NULL )
  {
    LOG_MSG_ERROR(" dfs_service_object  is NULL ", 0, 0, 0);
    return QTI_FAILURE;
  }

  switch (message_id)
  {
    case DFS_GET_CLIENT_BINDING :
      {
        LOG_MSG_INFO1("Got DFS get client binding response", 0, 0, 0);

        memset ( &dfs_get_client_binding_resp, 0,
                 sizeof(dfs_get_client_binding_resp_msg_v01));

        ret_val = qti_rmnet_qmi_message_decode(
                    dfs_service_object,
                    QMI_IDL_RESPONSE,
                    message_id,
                    msg,
                    qmux_msg_len,
                    &dfs_get_client_binding_resp,
                    sizeof(dfs_get_client_binding_resp_msg_v01));

        if (ret_val == QTI_FAILURE)
        {
          LOG_MSG_ERROR(" DFS_GET_CLIENT_BINDING_RESP_MSG decode"
                        " failure ", 0, 0 ,0);
          return QTI_FAILURE;
        }

        if ( dfs_get_client_binding_resp.bound_ep_id_valid )
        {
          ep_id_ptr = &(dfs_get_client_binding_resp.bound_ep_id.iface_id);
          ep_type_ptr = &(dfs_get_client_binding_resp.bound_ep_id.ep_type);

          qti_rmnet_ep_info_translation_resp( ep_id_ptr, ep_type_ptr);

          ret_val = qti_rmnet_qmi_message_encode(
                      dfs_service_object,
                      QMI_IDL_RESPONSE,
                      message_id,
                      &dfs_get_client_binding_resp,
                      sizeof(dfs_get_client_binding_resp_msg_v01),
                      msg,
                      qmux_msg_len);
          if (ret_val == QTI_FAILURE)
          {
            LOG_MSG_ERROR(" DFS_GET_CLIENT_BINDING_RESP_MSG encode failure ",
                          0, 0 ,0);
            return QTI_FAILURE;
          }
        }
        else
        {
          LOG_MSG_INFO1("USB EP ID Not valid for "
                        "DFS_GET_CLIENT_BINDING_RESP_MSG", 0, 0, 0);
        }
      }
      break;
    default:
      break;

  }
  return QTI_SUCCESS;
}

/*===========================================================

FUNCTION QTI_RMNET_QMI_MESSAGE_DECODE()

DESCRIPTION
  This function decodes the QMI message and fills the message structure with the decoded
  values.

DEPENDENCIES
  None.

RETURN VALUE
  0 on success
  -1 on failure

SIDE EFFECTS
  None

============================================================*/
int qti_rmnet_qmi_message_decode
(
  qmi_idl_service_object_type     p_service,
  qmi_idl_type_of_message_type    req_resp_ind,
  uint16_t                        message_id,
  void                            *msg,
  uint32_t                        qmux_msg_len,
  void                            *p_dst,
  uint32_t                        dst_len
)

{
  int                      ret_val;

  ret_val = qmi_idl_message_decode(
              p_service,
              req_resp_ind,
              message_id,
              msg + QTI_QMUX_SDU_MESSAGE_HDR_LEN_BYTES,
              qmux_msg_len - QTI_QMUX_SDU_MESSAGE_HDR_LEN_BYTES,
              p_dst,
              dst_len);

  if (ret_val < QMI_IDL_LIB_NO_ERR)
  {
    LOG_MSG_ERROR("Error in decoding QMI message %d", ret_val, 0 ,0);
    return QTI_FAILURE;
  }

  return QTI_SUCCESS;
}

/*===========================================================

FUNCTION QTI_RMNET_QMI_MESSAGE_ENCODE()

DESCRIPTION
  This function encodes the contents of the QMI message structure to a QMI message
  values.

DEPENDENCIES
  None.

RETURN VALUE
  0 on success
  -1 on failure

SIDE EFFECTS
  None

============================================================*/
int qti_rmnet_qmi_message_encode
(
  qmi_idl_service_object_type           p_service,
  qmi_idl_type_of_message_type          req_resp_ind,
  uint16_t                              message_id,
  void                                  *p_src,
  uint32_t                              src_len,
  void                                  *msg,
  uint32_t                              qmux_msg_len)
{
  int                      ret_val;
  int                      msg_len;

  LOG_MSG_INFO1("qti_rmnet_qmi_message_encode", 0, 0 ,0);
  ret_val = qmi_idl_message_encode(
               p_service,
               req_resp_ind,
               message_id,
               p_src,
               src_len,
               msg + QTI_QMUX_SDU_MESSAGE_HDR_LEN_BYTES,
               qmux_msg_len - QTI_QMUX_SDU_MESSAGE_HDR_LEN_BYTES,
               &msg_len);

  if (ret_val != QMI_IDL_LIB_NO_ERR)
  {
    LOG_MSG_ERROR("Error in encoding response message. ret_val %d", ret_val, 0 ,0);
    return QTI_FAILURE;
  }
  else
  {
    LOG_MSG_INFO1("Successfully encoded response message", 0, 0 ,0);
  }
  return QTI_SUCCESS;
}

/*===========================================================

FUNCTION QTI_RMNET_EP_INFO_TRANSLATION_REQ()

DESCRIPTION
  This function replaces the peripheral device end point ID and type with the modem iface EP ID
  and type values.

DEPENDENCIES
  None.

RETURN VALUE
  void

SIDE EFFECTS
  None

============================================================*/
void  qti_rmnet_ep_info_translation_req
(
  uint32_t                *ep_id_ptr,
  data_ep_type_enum_v01   *ep_type_ptr
)
{

  LOG_MSG_INFO1("qti_rmnet_ep_info_translation_req :USB EP ID %d / EP type %d",
                *ep_id_ptr, *ep_type_ptr, 0);

/*-----------------------------------------------------------------------------
  Update the EP information with modem iface EP details
------------------------------------------------------------------------------*/
  *ep_id_ptr = rmnet_state_config->modem_iface_ep.peripheral_iface_id ;
  *ep_type_ptr = rmnet_state_config->modem_iface_ep.ep_type;

  LOG_MSG_INFO1(" qti_rmnet_ep_info_translation_req : After EP translation Modem"
                " Iface EP ID %d / EP type %d", *ep_id_ptr, *ep_type_ptr, 0);
}

/*===========================================================

FUNCTION QTI_RMNET_EP_INFO_TRANSLATION_RESP()

DESCRIPTION
  This function replaces the modem iface end point ID and type with the peripheral device EP ID
  and type values.

DEPENDENCIES
  None.

RETURN VALUE
  void

SIDE EFFECTS
  None

============================================================*/
void  qti_rmnet_ep_info_translation_resp
(
  uint32_t *ep_id_ptr,
  data_ep_type_enum_v01 *ep_type_ptr
)
{

  LOG_MSG_INFO1("qti_rmnet_ep_info_translation_resp :Modem iface ID %d / "
                " EP type %d", *ep_id_ptr, *ep_type_ptr, 0);

/*-----------------------------------------------------------------------------
  Update the EP information with peripheral EP details
------------------------------------------------------------------------------*/
  *ep_id_ptr = rmnet_state_config->ph_iface_ep.peripheral_iface_id;
  *ep_type_ptr = rmnet_state_config->ph_iface_ep.ep_type;

  LOG_MSG_INFO1(" qti_rmnet_ep_info_translation_resp : After EP translation "
                "USB EP ID %d / EP type %d", *ep_id_ptr, *ep_type_ptr, 0);

}

