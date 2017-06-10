/*   Copyright (c) 2012 Qualcomm Atheros, Inc.
     All Rights Reserved.
     Qualcomm Atheros Confidential and Proprietary
 */

#include <string.h>
#include <stdbool.h>
#include "qmi_idl_lib.h"
#include "qmi_csi.h"
#include "bearer_independent_transport_v01.h"
#include "../gpsone_daemon_dbg.h"
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include "unistd.h"

/* This struct can hold more information
   about the client that is connecting to the service
   if there is any reason to keep state information
   about clients */
typedef struct {
  qmi_client_handle clnt;
}client_info_type;

/* This struct holds the arguments that need to be passed to the send_indications function */
typedef struct {
  client_info_type *clnt;
  qmi_req_handle req_handle;
  int msg_id;
  bit_ack_msg_v01 ind_struct;
}ind_type;

/* This struct is used to hold context of the server.  Cleanup is used
   to signal the reader thread to clean up and die, num_requests just tracks
   how many requests the service has handled, and service_handle is passed
   into the qmi_csi_handle_event function
   An instance of this structure is passed to the qmi_csi_register function as
   the service_cookie, and this structure can be tailored to a specific services needs */
typedef struct {
  volatile int cleanup;
  qmi_csi_service_handle service_handle;
  int num_requests;
}service_context_type;

/* This struct holds the arguments that need to be passed to the service reader thread */
typedef struct {
  service_context_type *service_cookie;
  qmi_csi_os_params *os_params;
}reader_args_type;

static service_context_type service_cookie;

static qmi_client_handle client_list[10] = {0};
static int g_transaction_id = 0;

void add_client(qmi_client_handle handle)
{
  uint32_t i = 0;
  int32_t first_index = -1;
  for(i = 0; i < sizeof(client_list) / sizeof(client_list[0]); i++)
  {
    // Find first empty bucket
    if(client_list[i] == NULL && first_index == -1)
    {
      first_index = i;
    }
    // Client has already been added
    if(client_list[i] == handle)
    {
      return;
    }
  }

  // Not already in the list and we found an empty bucket.
  if(first_index >= 0)
  {
    GPSONE_DMN_DBG("add_client: handle: %p idx: %d\n", handle, first_index);
    client_list[first_index] = handle;
  }
  else
  {
    GPSONE_DMN_DBG("add_client: NO CLIENT ADDED\n");
  }
}

void remove_client(qmi_client_handle handle)
{
  uint32_t i = 0;
  for(i = 0; i < sizeof(client_list) / sizeof(client_list[0]); i++)
  {
    if(client_list[i] == handle)
    {
      GPSONE_DMN_DBG("remove_client: handle: %p idx: %d\n", handle, i);
      client_list[i] = NULL;
    }
  }
}

int broadcast_ind(uint32_t indId, void* source, uint32_t msgSize)
{
  uint32_t i = 0;
  for(i = 0; i < sizeof(client_list) / sizeof(client_list[0]); i++)
  {
    if(client_list[i] != NULL)
    {
      qmi_csi_error retVal = qmi_csi_send_ind(
                                  client_list[i], indId,
                                  source, msgSize);

      if( QMI_CSI_NO_ERR != retVal )
      {
        GPSONE_DMN_PR_ERR("qmi_csi_send_ind returned error for broadcast idx: %d handle: %p ind: %d error: %d\n", i, client_list[i], indId, retVal);
        return -1;
      }
    }
  }

  return 0;
}

/*=============================================================================
  CALLBACK FUNCTION bit_connect_cb
=============================================================================*/
/*!
@brief
  This callback function is called by the QCSI infrastructure when
  infrastructure receives a request from the client( user of the service).

@param[in]   client_handle       Opaque handle used by the infrastructure to
                                 identify different services.
@param[in]   service_cookie      Service specific data.Service cookie is
                                 registered with the infrastructure during
                                 service registration(qmi_csi_register).
@param[out]  connection_handle   Services should provide this handle to
                                 facilitate the infrastructure to communicate
                                 to each individual service.

@retval    QMI_CSI_NO_ERR        Success
@retval    QMI_CSI_CONN_REFUSED  This error occurs when  limit on MAX number of
                                 clients a service can support is reached.

*/
/*=========================================================================*/
static qmi_csi_cb_error bit_connect_cb
(
  qmi_client_handle         client_handle,
  void                      *service_cookie,
  void                      **connection_handle
  )
{
  /* This is where the client handle can be stored if the service wants
         to keep some state information about clients. The connection handle can also
         be modified to facilitate communication with the infrastructure */
  client_info_type *clnt_info;
  /* For any service where it is anticipated that there could be more than one client
         connected at any given time the clients should be stored in some kind of data structure
         to facilitate cleanup and the sending of indications. */
  clnt_info = malloc(sizeof(client_info_type)); /* Freed in ping_disconnect_cb */
  if(!clnt_info)
    return QMI_CSI_CB_CONN_REFUSED;
  clnt_info->clnt = client_handle;
  *connection_handle = clnt_info;
  GPSONE_DMN_DBG("bit_connect_cb received, returning no error - handle: %p\n", client_handle);

  add_client(client_handle);

  return QMI_CSI_CB_NO_ERR;
}

/*=============================================================================
  CALLBACK FUNCTION bit_disconnect_cb
=============================================================================*/
/*!
@brief
  This callback function is called by the QCSI infrastructure when the last
  client(user of service ) deregisters with the  QCSI infrastructure.

@param[in]  connection_handle      Service handle used by the infrastructure
                                   to communicate to each individual service.
@param[in]  service_cookie         Service specific data.Service cookie is
                                   registered with the infrastructure during
                                   service registration(qmi_csi_register).
@retval    QMI_CSI_NO_ERR          Success
@retval    QMI_CSI_CONN_BUSY       This error occurs when service might be busy
                                   processing other pending requests.
@retval    QMI_CSI_INVALID_HANDLE  This error occurs when the connection
                                   handle passed is invalid.
*/
/*=========================================================================*/
static void bit_disconnect_cb
(
  void                      *connection_handle,
  void                      *service_cookie
 )
{
  client_info_type* clt_info = connection_handle;
  GPSONE_DMN_DBG("bit_disconnect_cb received, returning no error - handle: %p\n", clt_info->clnt);
  remove_client(clt_info->clnt);

  /* Free up memory for the client */
  if(connection_handle)
    free(connection_handle); /* Malloc in ping_connect_cb */


  return;
}

qmi_csi_cb_error bit_handle_req(
   client_info_type         *clnt_info,
   qmi_req_handle           req_handle,
   int                      msg_id,
   void                     *req_c_struct,
   int                      req_c_struct_len,
   void                     *service_cookie)
{
   bit_ack_msg_v01 bitResp;

   qmi_csi_cb_error rc = QMI_CSI_CB_INTERNAL_ERR;
   qmi_csi_error resp_err;

   // send response [SUCCESS]
   bitResp.resp.error = QMI_NO_ERR;
   bitResp.resp.result = QMI_RESULT_SUCCESS;
   GPSONE_DMN_DBG("bit_handle_req, sending response: %d\n", msg_id);
   resp_err = qmi_csi_send_resp( req_handle, msg_id, &bitResp, sizeof(bit_ack_msg_v01) );

   if(resp_err != QMI_CSI_NO_ERR)
   {
     GPSONE_DMN_PR_ERR("bit_handle_req: qmi_csi_send_resp returned error: %d\n",resp_err);
   }
   else
   {
     rc = QMI_CSI_CB_NO_ERR;
   }
   switch(msg_id)
   {
     case QMI_BIT_SERVICE_READY_IND_V01:
       GPSONE_DMN_DBG("bit_handle_req: received BIT_SERVICE_READY_IND\n");
             sleep(1);
          bit_open_req_msg_v01 openReq;
          memset(&openReq,0, sizeof(openReq));
       openReq.transaction_id = ++g_transaction_id;
       resp_err = qmi_csi_send_ind(clnt_info->clnt,
                                   QMI_BIT_OPEN_REQ_V01,
                                   &openReq,
                                   sizeof(openReq));
       if(resp_err != QMI_CSI_NO_ERR)
       {
         GPSONE_DMN_PR_ERR("qmi_csi_send_ind returned error for QMI_BIT_OPEN_REQ_V01: %d\n",resp_err);
         rc = QMI_CSI_CB_INTERNAL_ERR;
       }
       break;

     case QMI_BIT_OPEN_RESP_V01:
       {
         bit_resp_msg_v01 *openReqAck = (bit_resp_msg_v01 *)req_c_struct;
         GPSONE_DMN_DBG("%s:%d] received QMI_BIT_OPEN_RESP_V01. tr-id - %d, result - %d\n", __func__, __LINE__, openReqAck->transaction_id,
                        openReqAck->resp.error );
       }
       break;

     case QMI_BIT_OPEN_STATUS_IND_V01:
       {
         bit_open_status_ind_msg_v01 *openStatusInd = (bit_open_status_ind_msg_v01 *)req_c_struct;
         char * server_ip_host_name = (char *) "127.0.0.1";
         GPSONE_DMN_DBG("%s:%d] received QMI_BIT_OPEN_STATUS_IND_V01. tr-id - %d, result - %d\n",
                         __func__, __LINE__, openStatusInd->transaction_id, openStatusInd->status.result );

         //Sending Connect Request
         if(QMI_RESULT_SUCCESS_V01 == openStatusInd->status.result) {
           sleep(1);
           bit_connect_req_msg_v01 connectReq;
           memset(&connectReq,0, sizeof(connectReq));
           connectReq.transaction_id = ++g_transaction_id;
           connectReq.link = BIT_ENUM_LINK_TCP_V01;
           connectReq.protocol = BIT_ENUM_PROTOCOL_AGPS_SUPL_V01;
           connectReq.protocol_valid = 1;
           connectReq.host_info.ipv4_addr = inet_addr(server_ip_host_name);
           connectReq.host_info.ipv4_port = 8009;
           connectReq.host_info.validity_mask = BIT_MASK_IPV4_ADDR_AND_PORT_V01;
           connectReq.host_info_valid = 1;
           resp_err = qmi_csi_send_ind(clnt_info->clnt,
                                       QMI_BIT_CONNECT_REQ_V01 ,
                                       &connectReq,
                                       sizeof(connectReq));
           if(resp_err != QMI_CSI_NO_ERR)
           {
             GPSONE_DMN_PR_ERR("qmi_csi_send_ind returned error for QMI_BIT_OPEN_REQ_V01: %d\n",resp_err);
             rc = QMI_CSI_CB_INTERNAL_ERR;
           }
         }
       }
       break;

    case QMI_BIT_CONNECT_RESP_V01:
       {
         bit_resp_msg_v01 *connectReqAck = (bit_resp_msg_v01 *)req_c_struct;
         GPSONE_DMN_DBG("%s:%d] received QMI_BIT_CONNECT_RESP_V01. tr-id - %d, result - %d\n",
                        __func__, __LINE__, connectReqAck->transaction_id,connectReqAck->resp.result );
       }
       break;

     case QMI_BIT_CONNECT_STATUS_IND_V01 :
        {
          bit_connect_status_ind_msg_v01 *connectStatus = (bit_connect_status_ind_msg_v01 *)req_c_struct;
          GPSONE_DMN_DBG("%s:%d] received QMI_BIT_CONNECT_STATUS_IND_V01. tr-id - %d, result - %d, err - %d"
                         "session_hdle- %d session_hdle_valid - %d\n",
                         __func__, __LINE__, connectStatus->transaction_id,connectStatus->status.result,
                          connectStatus->status.error, (int) connectStatus->session_handle,
                         connectStatus->session_handle_valid );

          //Send RTR = true at this point
          if(QMI_RESULT_SUCCESS_V01 == connectStatus->status.result )
          {
            sleep(1);
            bit_ready_to_receive_req_msg_v01 readyToReceiveReq;
            memset(&readyToReceiveReq,0, sizeof(readyToReceiveReq));
            readyToReceiveReq.transaction_id = ++g_transaction_id;
            readyToReceiveReq.rtr = 1;
            readyToReceiveReq.max_recv_payload_size = BIT_CONST_PAYLOAD_LEN_MAX_V01;
            readyToReceiveReq.max_recv_payload_size_valid = 1;
            readyToReceiveReq.session_handle = connectStatus->session_handle;

            resp_err = qmi_csi_send_ind(clnt_info->clnt,
                                        QMI_BIT_READY_TO_RECEIVE_REQ_V01 ,
                                        &readyToReceiveReq,
                                        sizeof(readyToReceiveReq));
            if(resp_err != QMI_CSI_NO_ERR)
            {
              GPSONE_DMN_PR_ERR("qmi_csi_send_ind returned error for READY_TO_RECEIVE_REQ: %d\n",resp_err);
              rc = QMI_CSI_CB_INTERNAL_ERR;
            }
            else {
              GPSONE_DMN_DBG("%s:%d]READY_TO_RECEIVE_REQ transmit was successfull\n",
                             __func__, __LINE__);
            }
          }
        }
        break;

     case QMI_BIT_READY_TO_RECEIVE_RESP_V01 :
        {
          bit_session_resp_msg_v01 *readyToReceiveReqAck = (bit_session_resp_msg_v01 *)req_c_struct;
          GPSONE_DMN_DBG("%s:%d] received QMI_BIT_READY_TO_RECEIVE_RESP_V01. tr-id - %d, result - %d, err - %d"
                         "session_hdle- %d \n", __func__, __LINE__, readyToReceiveReqAck->transaction_id,
                         readyToReceiveReqAck->resp.result,readyToReceiveReqAck->resp.error,
                         (int) readyToReceiveReqAck->session_handle);

          if(QMI_RESULT_SUCCESS_V01 == readyToReceiveReqAck->resp.result )
          {
            sleep(1);
            char * testDataPayload = (char *) "TS_ECHO 0; PLEASE ECHO 8009 BACK";
            int testDataPayloadLen = strlen(testDataPayload) + 1;
            bit_send_req_msg_v01 sendReq;
            memset(&sendReq,0, sizeof(sendReq));
            sendReq.transaction_id = ++g_transaction_id;
            sendReq.session_handle = readyToReceiveReqAck->session_handle;
            memcpy(&sendReq.payload, testDataPayload, testDataPayloadLen);
            sendReq.payload_len = testDataPayloadLen;
            resp_err = qmi_csi_send_ind(clnt_info->clnt,
                                        QMI_BIT_SEND_REQ_V01 ,
                                        &sendReq,
                                        sizeof(sendReq));
            if(resp_err != QMI_CSI_NO_ERR)
            {
              GPSONE_DMN_PR_ERR("qmi_csi_send_ind returned error for QMI_BIT_SEND_REQ_V01: %d\n",resp_err);
              rc = QMI_CSI_CB_INTERNAL_ERR;
            }
            else {
              GPSONE_DMN_DBG("%s:%d]QMI_BIT_SEND_REQ_V01 transmit was successfull\n",
                             __func__, __LINE__);
            }
          }
        }
        break;


     case QMI_BIT_SEND_RESP_V01 :
        {
          bit_session_resp_msg_v01 *sendReqAck = (bit_session_resp_msg_v01 *)req_c_struct;
          GPSONE_DMN_DBG("%s:%d] received QMI_BIT_SEND_RESP_V01. tr-id - %d, sess_hdle - %d, result - %d\n",
                         __func__, __LINE__, sendReqAck->transaction_id,(int)sendReqAck->session_handle,
                         sendReqAck->resp.result );
        }
        break;

     case QMI_BIT_SEND_STATUS_IND_V01 :
        {
          bit_send_status_ind_msg_v01 *sendStatusInd = (bit_send_status_ind_msg_v01 *)req_c_struct;
          GPSONE_DMN_DBG("%s:%d] received SEND_STATUS_IND. tr-id - %d, result - %d, err - %d"
                         "session_hdle- %d \n", __func__, __LINE__, sendStatusInd->transaction_id,
                         sendStatusInd->status.result,sendStatusInd->status.error,
                         (int) sendStatusInd->session_handle);

        }
        break;


     case QMI_BIT_DATA_RECEIVED_IND_V01 :
        {
          bit_data_received_ind_msg_v01 *dataReceivedInd = (bit_data_received_ind_msg_v01 *)req_c_struct;
          GPSONE_DMN_DBG("%s:%d] received QMI_BIT_DATA_RECEIVED_IND_V01.session_hdle- %d seq_num - %d "
                         "payload_received: %s payload_len: %d\n",
                         __func__, __LINE__, (int)dataReceivedInd->session_handle, (int)dataReceivedInd->seq_num,
                         dataReceivedInd->payload, dataReceivedInd->payload_len);
          //Send DataReceivedStatus
          if(dataReceivedInd->payload_len > 0 )
          {
            sleep(1);
            bit_data_received_status_req_msg_v01 dataReceivedStatusReq;
            memset(&dataReceivedStatusReq,0, sizeof(dataReceivedStatusReq));
            dataReceivedStatusReq.transaction_id = ++g_transaction_id;
            dataReceivedStatusReq.seq_num = dataReceivedInd->seq_num;
            dataReceivedStatusReq.resp = (qmi_response_type_v01 ){QMI_RESULT_SUCCESS_V01,QMI_ERR_NONE_V01};
            dataReceivedStatusReq.max_recv_payload_size = (BIT_CONST_PAYLOAD_LEN_MAX_V01/2);
            dataReceivedStatusReq.max_recv_payload_size_valid = 1;
            dataReceivedStatusReq.session_handle = dataReceivedInd->session_handle;

            resp_err = qmi_csi_send_ind(clnt_info->clnt,
                                        QMI_BIT_DATA_RECEIVED_STATUS_REQ_V01 ,
                                        &dataReceivedStatusReq,
                                        sizeof(dataReceivedStatusReq));
            if(resp_err != QMI_CSI_NO_ERR)
            {
              GPSONE_DMN_PR_ERR("qmi_csi_send_ind returned error for DATA_RECEIVED_STATUS_REQ: %d\n",resp_err);
              rc = QMI_CSI_CB_INTERNAL_ERR;
            }
            else {
              GPSONE_DMN_DBG("%s:%d]DATA_RECEIVED_STATUS_REQ transmit was successfull\n",
                             __func__, __LINE__);
            }
          }
        }
        break;

     case QMI_BIT_DATA_RECEIVED_STATUS_RESP_V01 :
        {
          bit_session_resp_msg_v01 *dataReceivedStatusAck = (bit_session_resp_msg_v01 *)req_c_struct;
          GPSONE_DMN_DBG("%s:%d] received DATA_RECEIVED_STATUS_RESP. result - %d session_hdle- %d\n",
                         __func__, __LINE__, dataReceivedStatusAck->resp.result,
                          (int)dataReceivedStatusAck->session_handle );
          //Send get_local_host_info_req
          if(QMI_RESULT_SUCCESS_V01 == dataReceivedStatusAck->resp.result )
          {
            sleep(1);
            bit_get_local_host_info_req_msg_v01 getLocalHostInfoReq;
            memset(&getLocalHostInfoReq,0, sizeof(getLocalHostInfoReq));
            getLocalHostInfoReq.transaction_id = ++g_transaction_id;
            getLocalHostInfoReq.session_handle = dataReceivedStatusAck->session_handle;
            resp_err = qmi_csi_send_ind(clnt_info->clnt,
                                        QMI_BIT_GET_LOCAL_HOST_INFO_REQ_V01 ,
                                        &getLocalHostInfoReq,
                                        sizeof(getLocalHostInfoReq));
            if(resp_err != QMI_CSI_NO_ERR)
            {
              GPSONE_DMN_PR_ERR("qmi_csi_send_ind returned error for GET_LOCAL_HOST_INFO_REQ: %d\n",resp_err);
              rc = QMI_CSI_CB_INTERNAL_ERR;
            }
            else {
              GPSONE_DMN_DBG("%s:%d]GET_LOCAL_HOST_INFO_REQ transmit was successfull\n",
                             __func__, __LINE__);
            }
          }

        }
        break;

     case QMI_BIT_GET_LOCAL_HOST_INFO_RESP_V01 :
        {
          bit_session_resp_msg_v01 *getHostInfoReqAck = (bit_session_resp_msg_v01 *)req_c_struct;
          GPSONE_DMN_DBG("%s:%d] received GET_LOCAL_HOST_INFO_RESP. tr-id - %d, sess_hdle - %d, result - %d\n",
                         __func__, __LINE__, getHostInfoReqAck->transaction_id,(int)getHostInfoReqAck->session_handle,
                         getHostInfoReqAck->resp.result );
        }
        break;


     case QMI_BIT_GET_LOCAL_HOST_INFO_STATUS_IND_V01 :
        {
          bit_get_local_host_info_status_ind_msg_v01 *getLocalHostInfoInd
            = (bit_get_local_host_info_status_ind_msg_v01 *)req_c_struct;
          GPSONE_DMN_DBG("%s:%d] received GET_LOCAL_HOST_INFO_STATUS_IND. result - %d session_hdle- %d"
                         "host_info_valid %d ipv4_addr %d\n",
                         __func__, __LINE__, getLocalHostInfoInd->status.result,
                          (int)getLocalHostInfoInd->session_handle,
                         getLocalHostInfoInd->local_host_info_valid,
                         getLocalHostInfoInd->local_host_info.ipv4_addr );
          //Send set dormancy request
          if(QMI_RESULT_SUCCESS_V01 == getLocalHostInfoInd->status.result )
          {
            sleep(1);
            bit_set_dormancy_req_msg_v01 bitSetDormancyReq;
            memset(&bitSetDormancyReq,0, sizeof(bitSetDormancyReq));
            bitSetDormancyReq.transaction_id = ++g_transaction_id;
            bitSetDormancyReq.session_handle = getLocalHostInfoInd->session_handle;
            bitSetDormancyReq.dormancy_state = false;
            resp_err = qmi_csi_send_ind(clnt_info->clnt,
                                        QMI_BIT_SET_DORMANCY_REQ_V01 ,
                                        &bitSetDormancyReq,
                                        sizeof(bitSetDormancyReq));
            if(resp_err != QMI_CSI_NO_ERR)
            {
              GPSONE_DMN_PR_ERR("qmi_csi_send_ind returned error for SET_DORMANCY_REQ: %d\n",resp_err);
              rc = QMI_CSI_CB_INTERNAL_ERR;
            }
            else {
              GPSONE_DMN_DBG("%s:%d]SET_DORMANCY_REQ transmit was successfull\n",
                             __func__, __LINE__);
            }
          }

        }
        break;

     case QMI_BIT_SET_DORMANCY_RESP_V01 :
        {
          bit_session_resp_msg_v01 *setDormancyReqAck = (bit_session_resp_msg_v01 *)req_c_struct;
          GPSONE_DMN_DBG("%s:%d] received SET_DORMANCY_RESP. tr-id - %d, sess_hdle - %d, result - %d\n",
                         __func__, __LINE__, setDormancyReqAck->transaction_id,(int)setDormancyReqAck->session_handle,
                         setDormancyReqAck->resp.result );
          //Send Disconnect here as in simulation we will not get SET_DORMANCY_STATUS_IND due to stubbed out DS code
          if(QMI_RESULT_SUCCESS_V01 == setDormancyReqAck->resp.result)
          {
            sleep(1);
            bit_disconnect_req_msg_v01 bitDisconnectReq;
            memset(&bitDisconnectReq,0, sizeof(bitDisconnectReq));
            bitDisconnectReq.transaction_id = ++g_transaction_id;
            bitDisconnectReq.session_handle = setDormancyReqAck->session_handle;
            resp_err = qmi_csi_send_ind(clnt_info->clnt,
                                        QMI_BIT_DISCONNECT_REQ_V01 ,
                                        &bitDisconnectReq,
                                        sizeof(bitDisconnectReq));
            if(resp_err != QMI_CSI_NO_ERR)
            {
              GPSONE_DMN_PR_ERR("qmi_csi_send_ind returned error for DISCONNECT_REQ: %d\n",resp_err);
              rc = QMI_CSI_CB_INTERNAL_ERR;
            }
            else {
              GPSONE_DMN_DBG("%s:%d]DISCONNECT_REQ transmit was successfull\n",
                             __func__, __LINE__);
            }
          }

        }
        break;

     case QMI_BIT_SET_DORMANCY_STATUS_IND_V01 :
        {
          bit_set_dormancy_status_ind_msg_v01 *setDormancyStatusInd = (bit_set_dormancy_status_ind_msg_v01 *)req_c_struct;
          GPSONE_DMN_DBG("%s:%d] received SET_DORMANCY_STATUS. tr-id - %d, sess_hdle - %d, result - %d\n",
                         __func__, __LINE__, setDormancyStatusInd->transaction_id,(int)setDormancyStatusInd->session_handle,
                         setDormancyStatusInd->status.result );
        }
        break;

     case QMI_BIT_DISCONNECT_RESP_V01 :
        {
          bit_session_resp_msg_v01 *disconnectReqAck = (bit_session_resp_msg_v01 *)req_c_struct;
          GPSONE_DMN_DBG("%s:%d] received DISCONNECT_RESP. tr-id - %d, sess_hdle - %d, result - %d\n",
                         __func__, __LINE__, disconnectReqAck->transaction_id,(int)disconnectReqAck->session_handle,
                        disconnectReqAck->resp.result );
        }
        break;

     case QMI_BIT_DISCONNECT_STATUS_IND_V01 :
        {
          bit_disconnect_status_ind_msg_v01 *disconnectStatusInd = (bit_disconnect_status_ind_msg_v01 *)req_c_struct;
          GPSONE_DMN_DBG("%s:%d] received DISCONNECT_STATUS_IND. tr-id - %d, sess_hdle - %d, result - %d\n",
                         __func__, __LINE__, disconnectStatusInd->transaction_id,(int)disconnectStatusInd->session_handle,
                         disconnectStatusInd->status.result );
          //Send Close
          if(QMI_RESULT_SUCCESS_V01 == disconnectStatusInd->status.result)
          {
            sleep(1);
            bit_close_req_msg_v01 bitCloseReq;
            memset(&bitCloseReq,0, sizeof(bitCloseReq));
            bitCloseReq.transaction_id = ++g_transaction_id;
            resp_err = qmi_csi_send_ind(clnt_info->clnt,
                                        QMI_BIT_CLOSE_REQ_V01 ,
                                        &bitCloseReq,
                                        sizeof(bitCloseReq));
            if(resp_err != QMI_CSI_NO_ERR)
            {
              GPSONE_DMN_PR_ERR("qmi_csi_send_ind returned error for CLOSE_REQ: %d\n",resp_err);
              rc = QMI_CSI_CB_INTERNAL_ERR;
            }
            else {
              GPSONE_DMN_DBG("%s:%d]CLOSE_REQ transmit was successfull\n",
                             __func__, __LINE__);
            }
          }

        }
        break;

     case QMI_BIT_CLOSE_RESP_V01 :
        {
          bit_resp_msg_v01 *closeReqAck = (bit_resp_msg_v01 *)req_c_struct;
          GPSONE_DMN_DBG("%s:%d] received CLOSE_RESP. tr-id - %d, result - %d\n",
                         __func__, __LINE__, closeReqAck->transaction_id,closeReqAck->resp.result );
        }
        break;

     case QMI_BIT_CLOSE_STATUS_IND_V01 :
        {
          bit_close_status_ind_msg_v01 *closeStatusInd = (bit_close_status_ind_msg_v01 *)req_c_struct;
          GPSONE_DMN_DBG("%s:%d] received CLOSE_STATUS_IND. tr-id - %d, result - %d\n",
                         __func__, __LINE__, closeStatusInd->transaction_id,closeStatusInd->status.result );
        }
        break;

     default:
     {
       GPSONE_DMN_PR_ERR("%s:%d]got unrecognized req: %d\n",__func__, __LINE__, msg_id);
       break;
     }
   }
  return rc;
}


/*=============================================================================
  CALLBACK FUNCTION bit_handle_req_cb
=============================================================================*/
/*!
@brief
   This callback is invoked when the infrastructure receives an incoming message.
   The infrastructure decodes the data and gives it to the services

@param[in]  connection_handle      Service handle used by the infrastructure
                                   to communicate to each individual service.
@param[in]  req_handle             Opaque handle provided by the infrastructure
                                   to specify a particular transaction and
                                   message.

@param[in]  msg_id                 Message Id pertaining to a particular
                                   message.
@param[in]  req_c_struct           C struct with the decoded data.
@param[in]  req_c_struct_len       Length of the c struct.
@param[in]  service_cookie         Service specific data.Service cookie is
                                   registered with the infrastructure during
                                   service registration(qmi_csi_register).


@retval    QMI_CSI_NO_ERR          Success
@retval    QMI_IDL_...             Error, see error codes defined in qmi.h
*/
/*=========================================================================*/
static qmi_csi_cb_error bit_handle_req_cb
(
  void                     *connection_handle,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                     *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_cookie
)
{
  qmi_csi_cb_error rc = QMI_CSI_CB_INTERNAL_ERR;
  client_info_type *clnt_info = (client_info_type*)connection_handle;
  /* At this point the clnt_info can be tested to verify it is valid
         and any other tests or operations that a particular service would
         like to do on every client request */

  /* A request is being handled, increment the service_cookie num_requests */
  ((service_context_type*)service_cookie)->num_requests++;
  GPSONE_DMN_DBG("bit_handle_req_cb : received Message ID: %d\n",msg_id);
  rc = bit_handle_req(clnt_info,req_handle,msg_id,req_c_struct,req_c_struct_len,service_cookie);

  return rc;
}/* handle_req */

/*=============================================================================
  FUNCTION qmi_bit_register_service
=============================================================================*/
void *qmi_bit_register_service(qmi_csi_os_params *os_params)
{
  qmi_idl_service_object_type bit_service_object = bit_get_service_object_v01();
  qmi_csi_error rc = QMI_CSI_INTERNAL_ERR;
    GPSONE_DMN_DBG("%s:%d] qmi_bit_register_service: registering qmiBitService\n", __func__, __LINE__);
  rc = qmi_csi_register(bit_service_object, bit_connect_cb,
      bit_disconnect_cb, bit_handle_req_cb, &service_cookie, os_params,
      &service_cookie.service_handle);
  GPSONE_DMN_DBG("qmi_bit_register_service: qmi_csi_register returned %d\n", rc);
  if(rc != QMI_NO_ERR)
    return NULL;

  return service_cookie.service_handle;
}

