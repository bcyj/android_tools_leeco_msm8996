/*   Copyright (c) 2012 Qualcomm Atheros, Inc.
     All Rights Reserved.
     Qualcomm Atheros Confidential and Proprietary
 */

#include <stdlib.h>
#include "qmi_client.h"
#include "qmi_idl_lib.h"
#include "qmi_cci_target_ext.h"

#if defined( _ANDROID_)
#include "qmi_cci_target.h"
#include "qmi_cci_common.h"
#define LOG_NDEBUG 0
#endif //_ANDROID_

#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "gpsone_glue_qmi.h"
#include "gpsone_qmi_msg.h"
#include "gpsone_daemon_dbg.h"
#include "gpsone_glue_msg.h"

//Different wait times for off-target testing
#ifdef DEBUG_X86

// timeout in ms before send_msg_sync should return
#define BIT_CLIENT_ACK_TIMEOUT (5000)

// timeout in ms before a sync request should return
#define BIT_CLIENT_SYNC_REQ_TIMEOUT (5000)

#else

// timeout in ms before send_msg_sync should return
#define BIT_CLIENT_ACK_TIMEOUT (1000)

// timeout in ms before a sync request should return
#define BIT_CLIENT_SYNC_REQ_TIMEOUT (1000)

#endif //DEBUG_X86

//timeout in ms to wait for the service to come up
#define BIT_CLIENT_SERVICE_TIMEOUT_UNIT  (4000)
#define BIT_CLIENT_SERVICE_TIMEOUT_TOTAL  (40000)

/* table to relate the respInd Id with its size */
typedef struct
{
  uint32_t indId;
  size_t   indSize;
}bitClientIndTableStructT;

static bitClientIndTableStructT bitClientIndTable[]= {

  // open req ind
  { QMI_BIT_OPEN_REQ_V01, sizeof(bit_open_req_msg_v01)},

  // close req Ind
  { QMI_BIT_CLOSE_REQ_V01, sizeof(bit_close_req_msg_v01)},

  { QMI_BIT_CONNECT_REQ_V01, sizeof(bit_connect_req_msg_v01)},

  { QMI_BIT_DISCONNECT_REQ_V01, sizeof(bit_disconnect_req_msg_v01)},

  { QMI_BIT_SEND_REQ_V01, sizeof(bit_send_req_msg_v01)},

  { QMI_BIT_READY_TO_RECEIVE_REQ_V01, sizeof(bit_ready_to_receive_req_msg_v01)},

  { QMI_BIT_DATA_RECEIVED_STATUS_REQ_V01, sizeof(bit_data_received_status_req_msg_v01)},

  { QMI_BIT_SET_DORMANCY_REQ_V01, sizeof(bit_set_dormancy_req_msg_v01)},

  { QMI_BIT_GET_LOCAL_HOST_INFO_REQ_V01, sizeof(bit_get_local_host_info_req_msg_v01)}

  };

qmi_client_type g_userHandle;
void *pClientCookie;
//Defined and intialized in gpsone_bit_forward_qmi.c
int g_bit_forward_qmi_msgqid = -1;
/*===========================================================================
 *
 *                          FUNCTION DECLARATION
 *
 *==========================================================================*/

/** bitClientGetSizeByIndId
 *  @brief this function gets the size and the type (event,
 *         response)of the indication structure from its ID
 *  @param [in]  indId  ID of the indication
 *  @param [out] size   size of the indications
 *
 *  @return true if the ID was found, false otherwise */
static bool bitClientGetSizeByIndId (uint32_t indId, size_t *pIndSize)
{
  size_t idx = 0, indTableSize = 0;
  indTableSize = (sizeof(bitClientIndTable)/sizeof(bitClientIndTableStructT));
  for(idx=0; idx<indTableSize; idx++ )
  {
    if(indId == bitClientIndTable[idx].indId)
    {
      // found
      *pIndSize = bitClientIndTable[idx].indSize;

       GPSONE_DMN_DBG("%s:%d]: resp ind Id %d size = %d\n", __func__, __LINE__,
                    indId, (uint32_t)*pIndSize);
      return true;
    }
  }

  // Id not found
  GPSONE_DMN_PR_ERR("%s:%d]: indId %d not found\n", __func__, __LINE__, indId);
  return false;
}

/** convertQmiResponseToBitStatus
 *  @brief This function converts the erro codes from
    QMI format to BIT QMI Client format
 *  @param [in]  BIT Acknowledgement from modem
 *
 *  @return Status in BIT QMI Client format */
static bitClientStatusEnumType convertQmiResponseToBitStatus(
  bit_ack_msg_v01 *pResponse)
{
  bitClientStatusEnumType status =  eBIT_CLIENT_FAILURE_INTERNAL;

  // if result == SUCCESS don't look at error code
  if(pResponse->resp.result == QMI_RESULT_SUCCESS_V01 )
  {
    status  = eBIT_CLIENT_SUCCESS;
  }
  else
  {
    switch(pResponse->resp.error)
    {
      case QMI_ERR_MALFORMED_MSG_V01:
      case QMI_ERR_INVALID_ARG_V01:
        status = eBIT_CLIENT_FAILURE_INVALID_PARAMETER;
        break;

      case QMI_ERR_DEVICE_IN_USE_V01:
        status = eBIT_CLIENT_FAILURE_ENGINE_BUSY;
        break;

      default:
        status = eBIT_CLIENT_FAILURE_INTERNAL;
        break;
    }
  }
  GPSONE_DMN_DBG("%s:%d]: result = %d, error = %d, status = %d\n",
                __func__, __LINE__, pResponse->resp.result,
                pResponse->resp.error, status);
  return status;
}

/** bitClientHandleOpenReq
 *  @brief Handles an Open request from modem QBC
 *  @param [in] ind_buf
 *  @param [in] ind_buf_len
 *  @return true if transmit of request to BIT QMI Shim was
 *          successfull
*/
static bool bitClientHandleOpenReq
(
 const void*     ind_buf,
 uint32_t        ind_buf_len
)
{
  // process the open request
   struct qmi_msgbuf qmiOpenReq;
   bit_open_req_msg_v01 *openReq = (bit_open_req_msg_v01 *)ind_buf;

   GPSONE_DMN_DBG ("%s:%d]: len = %d tr-id = %d\n",
                 __func__, __LINE__, ind_buf_len, openReq->transaction_id);

   qmiOpenReq.qmi_msg_type = QMI_BIT_OPEN;
   memcpy(&qmiOpenReq.qmsg.qbitfwd_msg_open, openReq,
          sizeof(qmiOpenReq.qmsg.qbitfwd_msg_open));
   GPSONE_DMN_DBG("%s:%d] g_bit_forward_qmi_msgqid = %d\n", __func__, __LINE__, g_bit_forward_qmi_msgqid);
   gpsone_glue_msgsnd(g_bit_forward_qmi_msgqid , &qmiOpenReq, sizeof(qmiOpenReq));
   return true;
}

/** bitClientHandleConnectReq
 *  @brief Handles a Connect request from modem QBC
 *  @param [in] ind_buf
 *  @param [in] ind_buf_len
 *  @return true if transmit of request to BIT QMI Shim was
 *          successfull
*/
static bool bitClientHandleConnectReq
(
 const void*     ind_buf,
 uint32_t        ind_buf_len
)
{
  // process the connect request
   struct qmi_msgbuf qmiConnectReq ;
   bit_connect_req_msg_v01 *connectReq = (bit_connect_req_msg_v01 *)ind_buf;

   GPSONE_DMN_DBG ("%s:%d]: len = %d tr-id = %d\n",
                 __func__, __LINE__, ind_buf_len, connectReq->transaction_id );

   qmiConnectReq.qmi_msg_type = QMI_BIT_CONNECT;
   memcpy(&qmiConnectReq.qmsg.qbitfwd_msg_connect, connectReq,
          sizeof(qmiConnectReq.qmsg.qbitfwd_msg_connect));
   GPSONE_DMN_DBG("%s:%d] g_bit_forward_qmi_msgqid = %d\n", __func__, __LINE__, g_bit_forward_qmi_msgqid);
   gpsone_glue_msgsnd(g_bit_forward_qmi_msgqid , &qmiConnectReq, sizeof(qmiConnectReq));
   return true;
}

/** bitClientHandleReadyToReceiveReq
 *  @brief Handles a ReadyToReceive request from modem QBC
 *  @param [in] ind_buf
 *  @param [in] ind_buf_len
 *  @return true if transmit of request to BIT QMI Shim was
 *          successfull
*/
static bool bitClientHandleReadyToReceiveReq
(
 const void*     ind_buf,
 uint32_t        ind_buf_len
)
{
   struct qmi_msgbuf qmiReadyToReceive;
   bit_ready_to_receive_req_msg_v01 *readyToReceiveReq =
     (bit_ready_to_receive_req_msg_v01 *)ind_buf;

   GPSONE_DMN_DBG ("%s:%d]: len = %d tr-id = %d\n",
                 __func__, __LINE__, ind_buf_len, readyToReceiveReq->transaction_id );

   qmiReadyToReceive.qmi_msg_type = QMI_BIT_READY_TO_RECEIVE;
   memcpy(&qmiReadyToReceive.qmsg.qbitfwd_msg_ready_to_receive , readyToReceiveReq,
          sizeof(qmiReadyToReceive.qmsg.qbitfwd_msg_ready_to_receive));
   GPSONE_DMN_DBG("%s:%d] g_bit_forward_qmi_msgqid = %d\n", __func__, __LINE__, g_bit_forward_qmi_msgqid);
   gpsone_glue_msgsnd(g_bit_forward_qmi_msgqid , &qmiReadyToReceive, sizeof(qmiReadyToReceive));
   return true;
}

/** bitClientHandleDataReceivedStatusReq
 *  @brief Handles a DataReceivedStatus request from modem QBC
 *  @param [in] ind_buf
 *  @param [in] ind_buf_len
 *  @return true if transmit of request to BIT QMI Shim was
 *          successfull
*/
static bool bitClientHandleDataReceivedStatusReq
(
 const void*     ind_buf,
 uint32_t        ind_buf_len
)
{
   struct qmi_msgbuf qmiDataReceivedStatus;
   bit_data_received_status_req_msg_v01 *dataReceivedStatusReq =
     (bit_data_received_status_req_msg_v01 *)ind_buf;
   GPSONE_DMN_DBG ("%s:%d]: len = %d tr-id = %d\n",
                 __func__, __LINE__, ind_buf_len, dataReceivedStatusReq->transaction_id );
   qmiDataReceivedStatus.qmi_msg_type = QMI_BIT_DATA_RECEIVED_STATUS;
   memcpy(&qmiDataReceivedStatus.qmsg.qbitfwd_msg_data_received_status , dataReceivedStatusReq,
          sizeof(qmiDataReceivedStatus.qmsg.qbitfwd_msg_data_received_status));
   gpsone_glue_msgsnd(g_bit_forward_qmi_msgqid , &qmiDataReceivedStatus, sizeof(qmiDataReceivedStatus));
   return true;
}

/** bitClientHandleSetDormancyReq
 *  @brief Handles a DataReceivedStatus request from modem QBC
 *  @param [in] ind_buf
 *  @param [in] ind_buf_len
 *  @return true if transmit of request to BIT QMI Shim was
 *          successfull
*/
static bool bitClientHandleSetDormancyReq
(
 const void*     ind_buf,
 uint32_t        ind_buf_len
)
{
   struct qmi_msgbuf qmiSetDormancyReq;
   bit_set_dormancy_req_msg_v01 *bitSetDormancyReq=
     (bit_set_dormancy_req_msg_v01 *)ind_buf;
   GPSONE_DMN_DBG ("%s:%d]: len = %d tr-id = %d\n",
                 __func__, __LINE__, ind_buf_len, bitSetDormancyReq->transaction_id );
   qmiSetDormancyReq.qmi_msg_type = QMI_BIT_SET_DORMANCY;
   memcpy(&qmiSetDormancyReq.qmsg.qbitfwd_msg_set_dormancy , bitSetDormancyReq,
          sizeof(qmiSetDormancyReq.qmsg.qbitfwd_msg_set_dormancy));
   gpsone_glue_msgsnd(g_bit_forward_qmi_msgqid , &qmiSetDormancyReq, sizeof(qmiSetDormancyReq));
   return true;
}

/** bitClientHandleDisconnectReq
 *  @brief Handles a Disconnect request from modem QBC
 *  @param [in] ind_buf
 *  @param [in] ind_buf_len
 *  @return true if transmit of request to BIT QMI Shim was
 *          successfull
*/
static bool bitClientHandleDisconnectReq
(
 const void*     ind_buf,
 uint32_t        ind_buf_len
)
{
   struct qmi_msgbuf qmiDisconnectReq;
   bit_disconnect_req_msg_v01 *bitDisconnectReq=
     (bit_disconnect_req_msg_v01 *)ind_buf;
   GPSONE_DMN_DBG ("%s:%d]: len = %d tr-id = %d\n",
                 __func__, __LINE__, ind_buf_len, bitDisconnectReq->transaction_id );
   qmiDisconnectReq.qmi_msg_type = QMI_BIT_DISCONNECT;
   memcpy(&qmiDisconnectReq.qmsg.qbitfwd_msg_disconnect , bitDisconnectReq,
          sizeof(qmiDisconnectReq.qmsg.qbitfwd_msg_disconnect));
   gpsone_glue_msgsnd(g_bit_forward_qmi_msgqid , &qmiDisconnectReq, sizeof(qmiDisconnectReq));
   return true;
}

/** bitClientHandleCloseReq
 *  @brief Handles a Close request from modem QBC
 *  @param [in] ind_buf
 *  @param [in] ind_buf_len
 *  @return true if transmit of request to BIT QMI Shim was
 *          successfull
*/
static bool bitClientHandleCloseReq
(
 const void*     ind_buf,
 uint32_t        ind_buf_len
)
{
   struct qmi_msgbuf qmiCloseReq;
   bit_close_req_msg_v01 *bitCloseReq=
     (bit_close_req_msg_v01 *)ind_buf;
   GPSONE_DMN_DBG ("%s:%d]: len = %d tr-id = %d\n",
                 __func__, __LINE__, ind_buf_len, bitCloseReq->transaction_id );
   qmiCloseReq.qmi_msg_type = QMI_BIT_CLOSE;
   memcpy(&qmiCloseReq.qmsg.qbitfwd_msg_close , bitCloseReq,
          sizeof(qmiCloseReq.qmsg.qbitfwd_msg_close));
   gpsone_glue_msgsnd(g_bit_forward_qmi_msgqid , &qmiCloseReq, sizeof(qmiCloseReq));
   return true;
}

/** bitClientHandleGetLocalHostInfoReq
 *  @brief Handles a GetLocalHostInfo request from modem QBC
 *  @param [in] ind_buf
 *  @param [in] ind_buf_len
 *  @return true if transmit of request to BIT QMI Shim was
 *          successfull
*/
static bool bitClientHandleGetLocalHostInfoReq
(
 const void*     ind_buf,
 uint32_t        ind_buf_len
)
{
   struct qmi_msgbuf qmiGetLocalHostInfo;
   bit_get_local_host_info_req_msg_v01 *getLocalHostInfoReq =
     (bit_get_local_host_info_req_msg_v01 *)ind_buf;
   GPSONE_DMN_DBG ("%s:%d]: len = %d tr-id = %d\n",
                 __func__, __LINE__, ind_buf_len, getLocalHostInfoReq->transaction_id );
   qmiGetLocalHostInfo.qmi_msg_type = QMI_BIT_GET_LOCAL_HOST_INFO;
   memcpy(&qmiGetLocalHostInfo.qmsg.qbitfwd_msg_get_local_host_info , getLocalHostInfoReq,
          sizeof(qmiGetLocalHostInfo.qmsg.qbitfwd_msg_get_local_host_info));
   gpsone_glue_msgsnd(g_bit_forward_qmi_msgqid , &qmiGetLocalHostInfo, sizeof(qmiGetLocalHostInfo));
   return true;
}

/** bitClientHandleSendReq
 *  @brief Handles a Send request from modem QBC
 *  @param [in] ind_buf
 *  @param [in] ind_buf_len
 *  @return true if transmit of request to BIT QMI Shim was
 *          successfull
*/
static bool bitClientHandleSendReq
(
 const void*     ind_buf,
 uint32_t        ind_buf_len
)
{
   struct qmi_msgbuf qmiSendReq;
   bit_send_req_msg_v01 *sendReq = ( bit_send_req_msg_v01 *)ind_buf;
   GPSONE_DMN_DBG ("%s:%d]: len = %d tr-id = %d\n",
                 __func__, __LINE__, ind_buf_len, sendReq->transaction_id );
   qmiSendReq.qmi_msg_type = QMI_BIT_SEND;
   memcpy(&qmiSendReq.qmsg.qbitfwd_msg_send , sendReq,
          sizeof(qmiSendReq.qmsg.qbitfwd_msg_send));
   GPSONE_DMN_DBG("%s:%d] g_bit_forward_qmi_msgqid = %d\n", __func__, __LINE__, g_bit_forward_qmi_msgqid);
   gpsone_glue_msgsnd(g_bit_forward_qmi_msgqid , &qmiSendReq, sizeof(qmiSendReq));
   return true;
}

/** bitClientHandleIndication
 *  @brief looks at each indication and calls the appropriate
 *         handler
 *  @param [in] indId
 *  @param [in] indBuffer
 *  @param [in] indSize
 *  @return true if indication was validated; else false */

static bool bitClientHandleIndication(
  uint32_t        indId,
  void*           indBuffer,
  size_t          indSize
 )
{
  bool status = false;
  switch(indId)
  {
    // handle the indications
    case QMI_BIT_OPEN_REQ_V01 :
    {
      status = bitClientHandleOpenReq(indBuffer, indSize);
      break;
    }

   case QMI_BIT_CONNECT_REQ_V01:
    {
     status = bitClientHandleConnectReq(indBuffer, indSize);
     break;
    }

    case QMI_BIT_READY_TO_RECEIVE_REQ_V01 :
     {
      status = bitClientHandleReadyToReceiveReq(indBuffer, indSize);
      break;
     }

    case QMI_BIT_SEND_REQ_V01 :
    {
     status = bitClientHandleSendReq(indBuffer, indSize);
     break;
    }
    case QMI_BIT_DATA_RECEIVED_STATUS_REQ_V01 :
    {
     status = bitClientHandleDataReceivedStatusReq(indBuffer, indSize);
     break;
    }

    case QMI_BIT_GET_LOCAL_HOST_INFO_REQ_V01 :
    {
     status = bitClientHandleGetLocalHostInfoReq(indBuffer, indSize);
     break;
    }

    case QMI_BIT_SET_DORMANCY_REQ_V01 :
    {
     status = bitClientHandleSetDormancyReq(indBuffer, indSize);
     break;
    }

    case QMI_BIT_DISCONNECT_REQ_V01 :
    {
     status = bitClientHandleDisconnectReq(indBuffer, indSize);
     break;
    }

    case QMI_BIT_CLOSE_REQ_V01 :
    {
     status = bitClientHandleCloseReq(indBuffer, indSize);
     break;
    }

    default:
      GPSONE_DMN_PR_ERR("%s:%d]: unknown ind id %d\n", __func__, __LINE__,
                   (uint32_t)indId);
      status = false;
      break;
  }
  return status;
}


/** bitClientErrorCb
 *  @brief handles the QCCI error events, this is called by the
 *         QCCI infrastructure when the service is no longer
 *         available.
 *  @param [in] user handle
 *  @param [in] error
 *  @param [in] *err_cb_data
 */

static void bitClientErrorCb
(
  qmi_client_type user_handle,
  qmi_client_error_type error,
  void *err_cb_data
)
{

   GPSONE_DMN_DBG("%s:%d]: Service Error %d received, pCallbackData = %p\n",
      __func__, __LINE__, error, err_cb_data);

  //TODO: need to restart the GPS daemon at this point. Verify if this is for modem restart?
  GPSONE_DMN_PR_ERR("%s:%d] we exit...", __func__, __LINE__);
  exit(1);
}


/** bitClientIndCb
 *  @brief handles the indications sent from the service
 *  @param [in] user handle
 *  @param [in] msg_id
 *  @param [in] ind_buf
 *  @param [in] ind_buf_len
 *  @param [in] ind_cb_data */
static void bitClientIndCb
(
 qmi_client_type                user_handle,
 unsigned int                   msg_id,
 void                           *ind_buf,
 unsigned int                   ind_buf_len,
 void                           *ind_cb_data
)
{
  size_t indSize = 0;
  qmi_client_error_type rc ;
  GPSONE_DMN_DBG("%s:%d]: Indication: msg_id=%d buf_len=%d \n",
                __func__, __LINE__, (uint32_t)msg_id, ind_buf_len);

  // check user handle
  if(g_userHandle != user_handle)
  {
    GPSONE_DMN_PR_ERR("%s:%d]: invalid user_handle got %p expected %p\n",
        __func__, __LINE__,
        user_handle, g_userHandle);
    return;
  }
  // Get the indication size
  if( true == bitClientGetSizeByIndId(msg_id, &indSize))
  {
    void *indBuffer = NULL;

    // decode the indication
    indBuffer = malloc(indSize);

    if(NULL == indBuffer)
    {
      GPSONE_DMN_PR_ERR("%s:%d]: memory allocation failed\n", __func__, __LINE__);
      return;
    }

    // decode the indication
    rc = qmi_client_message_decode(
        user_handle,
        QMI_IDL_INDICATION,
        msg_id,
        ind_buf,
        ind_buf_len,
        indBuffer,
        indSize);

    if( rc == QMI_NO_ERR )
    {
      //validate indication
      if (true == bitClientHandleIndication(msg_id, indBuffer, indSize))
      {
        GPSONE_DMN_DBG("%s:%d]: bitClientHandleIndication returned success \n",
                      __func__, __LINE__);
      }
      else // error handling indication
      {
        GPSONE_DMN_PR_ERR("%s:%d]: Error handling the indication %d\n",
                      __func__, __LINE__, (uint32_t)msg_id);
      }
    }
    else
    {
      GPSONE_DMN_PR_ERR("%s:%d]: Error decoding indication %d\n",
                    __func__, __LINE__, rc);
    }
    if(indBuffer)
    {
      free (indBuffer);
    }
  }
  else // Id not found
  {
    GPSONE_DMN_PR_ERR("%s:%d]: Error indication not found %d\n",
                  __func__, __LINE__,(uint32_t)msg_id);
  }
  return;
}


/** bitClientSendServiceReady
 *  @brief Indicates that BIT QMI Daemon is ready to BIT service
 *  @return true if transmission to modem was successfull; else false */

static bool bitClientSendServiceReady(void)
{
  bitClientStatusEnumType rc = eBIT_CLIENT_SUCCESS;
  bitClientReqUnionType serviceReadyReq;

  rc = gpsone_glue_qmi_send_req(QMI_BIT_SERVICE_READY_IND_V01 ,serviceReadyReq);

  if(rc != eBIT_CLIENT_SUCCESS)
  {
    GPSONE_DMN_PR_ERR("%s:%d] status %d\n", __func__, __LINE__,
             rc );
    return false;
  }

  return true;
}

/**  validateRequest
  @brief validates the input request
  @param [in] reqId       request ID
  @param [in] reqPayload  Union of pointers to message payload
  @param [out] ppOutData  Pointer to void *data if successful
  @param [out] pOutLen    Pointer to length of data if succesful.
  @return false on failure, true on Success
*/

static bool validateRequest(
  uint32_t                    reqId,
  const bitClientReqUnionType reqPayload,
  void                        **ppOutData,
  uint32_t                    *pOutLen )

{
  bool noPayloadFlag = false;


  switch(reqId)
  {
    case QMI_BIT_DATA_RECEIVED_IND_V01:
    {
      *pOutLen = sizeof(bit_data_received_ind_msg_v01);
      GPSONE_DMN_DBG("%s:%d]: reqId = DATA_RECEIVED_IND\n",
                     __func__, __LINE__);
    }
    break;
    case QMI_BIT_OPEN_RESP_V01:
    {
      *pOutLen = sizeof(bit_resp_msg_v01);
      GPSONE_DMN_DBG("%s:%d]: reqId = OPEN_RESP\n",
                     __func__, __LINE__);
    }
    break;
    case QMI_BIT_OPEN_STATUS_IND_V01:
    {
      *pOutLen = sizeof(bit_open_status_ind_msg_v01);
      GPSONE_DMN_DBG("%s:%d]: reqId = OPEN_STATUS_IND\n",
                     __func__, __LINE__);

      break;
    }

    case QMI_BIT_CONNECT_STATUS_IND_V01:
    {
      *pOutLen = sizeof(bit_connect_status_ind_msg_v01);
      GPSONE_DMN_DBG("%s:%d]: reqId = CONNECT_STATUS_IND\n",
                     __func__, __LINE__);
    }
    break;

    case QMI_BIT_CONNECT_RESP_V01:
    {
      *pOutLen = sizeof(bit_resp_msg_v01);
      GPSONE_DMN_DBG("%s:%d]: reqId = CONNECT_RESP\n",
                     __func__, __LINE__);
    }
    break;

    case QMI_BIT_READY_TO_RECEIVE_RESP_V01:
    {
      *pOutLen = sizeof(bit_session_resp_msg_v01);
      GPSONE_DMN_DBG("%s:%d]: reqId = READY_TO_RECEIVE_RESP\n",
                     __func__, __LINE__);
    }
    break;

    case QMI_BIT_SEND_RESP_V01:
    {
      *pOutLen = sizeof(bit_session_resp_msg_v01);
      GPSONE_DMN_DBG("%s:%d]: reqId = SEND_RESP\n",
                     __func__, __LINE__);
    }
    break;

    case QMI_BIT_SEND_STATUS_IND_V01:
    {
      *pOutLen = sizeof(bit_send_status_ind_msg_v01);
      GPSONE_DMN_DBG("%s:%d]: reqId = SEND_STATUS_IND\n",
                     __func__, __LINE__);
    }
    break;
    case QMI_BIT_DATA_RECEIVED_STATUS_RESP_V01:
    {
      *pOutLen = sizeof(bit_session_resp_msg_v01);
      GPSONE_DMN_DBG("%s:%d]: reqId = DATA_RECEIVED_STATUS_RESP\n",
                     __func__, __LINE__);
    }
    break;

    case QMI_BIT_GET_LOCAL_HOST_INFO_RESP_V01 :
    {
      *pOutLen = sizeof(bit_session_resp_msg_v01);
      GPSONE_DMN_DBG("%s:%d]: reqId = GET_LOCAL_HOST_INFO_RESP\n",
                     __func__, __LINE__);
    }
    break;

    case QMI_BIT_SET_DORMANCY_RESP_V01 :
    {
      *pOutLen = sizeof(bit_session_resp_msg_v01);
      GPSONE_DMN_DBG("%s:%d]: reqId = SET_DORMANCY_RESP\n",
                     __func__, __LINE__);
    }
    break;


    case QMI_BIT_GET_LOCAL_HOST_INFO_STATUS_IND_V01 :
    {
      *pOutLen = sizeof(bit_get_local_host_info_status_ind_msg_v01);
      GPSONE_DMN_DBG("%s:%d]: reqId = GET_LOCAL_HOST_INFO_STATUS_IND\n",
                     __func__, __LINE__);
    }
    break;

    case QMI_BIT_SET_DORMANCY_STATUS_IND_V01 :
    {
      *pOutLen = sizeof(bit_set_dormancy_status_ind_msg_v01);
      GPSONE_DMN_DBG("%s:%d]: reqId = SET_DORMANCY_STATUS_IND\n",
                     __func__, __LINE__);
    }
    break;

    case QMI_BIT_DISCONNECT_RESP_V01 :
    {
      *pOutLen = sizeof(bit_session_resp_msg_v01);
      GPSONE_DMN_DBG("%s:%d]: reqId = DISCONNECT_RESP\n",
                     __func__, __LINE__);
    }
    break;


    case QMI_BIT_DISCONNECT_STATUS_IND_V01 :
    {
      *pOutLen = sizeof(bit_disconnect_status_ind_msg_v01);
      GPSONE_DMN_DBG("%s:%d]: reqId = DISCONNECT_STATUS_IND\n",
                     __func__, __LINE__);
    }
    break;

    case QMI_BIT_CLOSE_RESP_V01 :
    {
      *pOutLen = sizeof(bit_resp_msg_v01);
      GPSONE_DMN_DBG("%s:%d]: reqId = CLOSE_RESP\n",
                     __func__, __LINE__);
    }
    break;


    case QMI_BIT_CLOSE_STATUS_IND_V01 :
    {
      *pOutLen = sizeof(bit_close_status_ind_msg_v01);
      GPSONE_DMN_DBG("%s:%d]: reqId = CLOSE_STATUS_IND\n",
                     __func__, __LINE__);
    }
    break;

    // ALL requests with no payload
    case QMI_BIT_SERVICE_READY_IND_V01:
    {
      noPayloadFlag = true;
      GPSONE_DMN_DBG("%s:%d]: reqId = SERVICE_READY_IND\n",
                     __func__, __LINE__);
    }
    break;

    default:
      GPSONE_DMN_DBG("%s:%d]: Error unknown reqId=%d\n", __func__, __LINE__,
                    reqId);
      return false;
  }
  if(true == noPayloadFlag)
  {
    *ppOutData = NULL;
    *pOutLen = 0;
  }
  else
  {
    //set dummy pointer for request union
    *ppOutData = (void*) reqPayload.pDataReceivedInd ;
  }
  GPSONE_DMN_DBG("%s:%d]: reqId=%d, len = %d\n", __func__, __LINE__,
                reqId, *pOutLen);
  return true;
}

/** bitClientQmiCtrlPointInit
 @brief wait for the service to come up or timeout; when the
        service comes up initialize the control point and set
        internal handle and indication callback.
 @param pQmiClient,
*/

static bitClientStatusEnumType bitClientQmiCtrlPointInit(void)
{
  qmi_client_type clnt, notifier;
  bool notifierInitFlag = false;
  // instances of this service
  qmi_service_info *pServiceInfo = NULL;
  bitClientStatusEnumType status = eBIT_CLIENT_SUCCESS;

  do
  {
    uint32_t num_services = 0, num_entries = 0;
    qmi_client_error_type rc = QMI_NO_ERR;
    bool nosignal = false;

    // Get the service object for the qmiBit Service
    qmi_idl_service_object_type bitClientServiceObject =
      bit_get_service_object_v01();

    // Verify that qmiBit_get_service_object did not return NULL
    if (NULL == bitClientServiceObject)
    {
        GPSONE_DMN_PR_ERR("%s:%d]: qmiBit_get_service_object_v02 failed\n" ,
                    __func__, __LINE__ );
       status = eBIT_CLIENT_FAILURE_INTERNAL;
       break;
    }

    // get the service addressing information
    rc = qmi_client_get_service_list( bitClientServiceObject, NULL, NULL,
                                      &num_services);
     GPSONE_DMN_DBG("%s:%d]: qmi_client_get_service_list() first try rc %d, "
             "num_services %d", __func__, __LINE__, rc, num_services);

    if (rc != QMI_NO_ERR) {
        // bummer, service list is not up.
        // We need to try again after a timed wait
        qmi_client_os_params os_params;
        int timeout = 0;

        // register for service notification
        rc = qmi_client_notifier_init(bitClientServiceObject, &os_params, &notifier);
        notifierInitFlag = (NULL != notifier);

        if (rc != QMI_NO_ERR) {
            GPSONE_DMN_PR_ERR("%s:%d]: qmi_client_notifier_init failed %d\n",
                     __func__, __LINE__, rc);
            status = eBIT_CLIENT_FAILURE_INTERNAL;
            break;
        }

        do {
            QMI_CCI_OS_SIGNAL_CLEAR(&os_params);
            /* If service is not up wait on a signal until the service is up
             * or a timeout occurs. */
            QMI_CCI_OS_SIGNAL_WAIT(&os_params, BIT_CLIENT_SERVICE_TIMEOUT_UNIT);
            nosignal = QMI_CCI_OS_SIGNAL_TIMED_OUT(&os_params);

            // get the service addressing information
            rc = qmi_client_get_service_list(bitClientServiceObject, NULL, NULL,
                                             &num_services);

            timeout += BIT_CLIENT_SERVICE_TIMEOUT_UNIT;

             GPSONE_DMN_DBG("%s:%d]: qmi_client_get_service_list() rc %d, nosignal %d, "
                     "total timeout %d", __func__, __LINE__, rc, nosignal, timeout);
        } while (timeout < BIT_CLIENT_SERVICE_TIMEOUT_TOTAL && nosignal && rc != QMI_NO_ERR);
    }

    if (0 == num_services || rc != QMI_NO_ERR) {
        if (!nosignal) {
            GPSONE_DMN_PR_ERR("%s:%d]: qmi_client_get_service_list failed even though"
                     "service is up !!!  Error %d \n", __func__, __LINE__, rc);
            status = eBIT_CLIENT_FAILURE_INTERNAL;
        } else {
            GPSONE_DMN_PR_ERR("%s:%d]: qmi_client_get_service_list failed after retries,"
                     " final Err %d", __func__, __LINE__, rc);
            status = eBIT_CLIENT_FAILURE_TIMEOUT;
        }
        break;
    }

    pServiceInfo =
      (qmi_service_info *)malloc(num_services * sizeof(qmi_service_info));

    if(NULL == pServiceInfo)
    {
      GPSONE_DMN_PR_ERR("%s:%d]: could not allocate memory for serviceInfo !!\n",
               __func__, __LINE__);

      status = eBIT_CLIENT_FAILURE_INTERNAL;
      break;
    }

    //set the number of entries to get equal to the total number of
    //services.
    num_entries = num_services;
    //populate the serviceInfo
    rc = qmi_client_get_service_list( bitClientServiceObject, pServiceInfo,
                                      &num_entries, &num_services);


     GPSONE_DMN_DBG("%s:%d]: qmi_client_get_service_list()"
                  " returned %d num_entries = %d num_services = %d\n",
                  __func__, __LINE__,
                   rc, num_entries, num_services);

    if(rc != QMI_NO_ERR)
    {
      GPSONE_DMN_PR_ERR("%s:%d]: qmi_client_get_service_list Error %d \n",
                    __func__, __LINE__, rc);

      status = eBIT_CLIENT_FAILURE_INTERNAL;
      break;
    }

    // initialize the client
    //sent the address of the first service found
    // if IPC router is present, this will go to the service instance
    // enumerated over IPC router, else it will go over the next transport where
    // the service was enumerated.
    rc = qmi_client_init(&pServiceInfo[0], bitClientServiceObject,
                         bitClientIndCb, (void *) NULL,
                         NULL, &clnt);

    if(rc != QMI_NO_ERR)
    {
      GPSONE_DMN_PR_ERR("%s:%d]: qmi_client_init error %d\n",
                    __func__, __LINE__, rc);

      status = eBIT_CLIENT_FAILURE_INTERNAL;
      break;
    }

     GPSONE_DMN_DBG("%s:%d]: before registering erro cb to"
                  "qmi_client_register_error_cb \n",
                   __func__, __LINE__);

    // register error callback
    rc  = qmi_client_register_error_cb(clnt,
        bitClientErrorCb, NULL);

    if( QMI_NO_ERR != rc)
    {
      GPSONE_DMN_PR_ERR("%s:%d]: could not register QCCI error callback error:%d\n",
                    __func__, __LINE__, rc);

      status = eBIT_CLIENT_FAILURE_INTERNAL;
      break;
    }

    // copy the clnt handle returned in qmi_client_init
    g_userHandle = clnt;
    status = eBIT_CLIENT_SUCCESS;

  } while(0);

  /* release the notifier handle */
  if(true == notifierInitFlag)
  {
    qmi_client_release(notifier);
  }

  if(NULL != pServiceInfo)
  {
    free((void *)pServiceInfo);
  }

  return status;
}

/** gpsone_one_qmi_close
  @brief Disconnects a client from the BIT service on the modem

  @return
  One of the following error codes:
  - 0 (eBIT_CLIENT_SUCCESS) - On success.
  - non-zero error code(see bitClientStatusEnumType) - On failure.
*/

bitClientStatusEnumType gpsone_glue_qmi_close(void)
{
  qmi_client_error_type rc = QMI_NO_ERR; //No error

  GPSONE_DMN_DBG("%s:%d]:\n", __func__, __LINE__ );

  // check the input handle for sanity
  if(NULL == g_userHandle)
  {
    // invalid handle
    GPSONE_DMN_PR_ERR("%s:%d]: invalid handle \n",
                  __func__, __LINE__);

    return(eBIT_CLIENT_FAILURE_INVALID_HANDLE);
  }

   GPSONE_DMN_DBG("bitClientClose releasing handle %p\n",g_userHandle );

  // NEXT call goes out to modem. We log the callflow before it
  // actually happens to ensure the this comes before resp callflow
  // back from the modem, to avoid confusing log order. We trust
  // that the QMI framework is robust.

  // release the handle
  rc = qmi_client_release(g_userHandle);
  if(QMI_NO_ERR != rc )
  {
    GPSONE_DMN_PR_ERR("%s:%d]: qmi_client_release error %d for client %p\n",
                   __func__, __LINE__, rc, g_userHandle);
    return(eBIT_CLIENT_FAILURE_INTERNAL);
  }


  // set the handle to invalid value
  g_userHandle = BIT_CLIENT_INVALID_HANDLE_VALUE;
  return eBIT_CLIENT_SUCCESS;
}

//----------------------- END INTERNAL FUNCTIONS ----------------------------------------

/** gpsone_glue_qmi_init
  @brief Connects a BIT client to the BIT service. If the connection
         is successful, returns a handle that the BIT client uses for
         future BIT operations.

  @param [in] bit_forward_qmi_msgqid MsgQId used by the client
                               for any sub.

  @return
  One of the following error codes:
  - eBIT_CLIENT_SUCCESS  -- If the connection is opened.
  - non-zero error code(see bitClientStatusEnumType)--  On failure.
*/

bitClientStatusEnumType gpsone_glue_qmi_init (int bit_forward_qmi_msgqid)
{
  bitClientStatusEnumType status = eBIT_CLIENT_SUCCESS;
   GPSONE_DMN_DBG("%s:%d] \n", __func__, __LINE__);
  g_bit_forward_qmi_msgqid = bit_forward_qmi_msgqid;
  do
  {
    /* Initialize the QMI control point; this function will block
     * until a service is up or a timeout occurs. If the connection to
     * the service succeeds the callback data will be filled in with
     * a qmi_client value.
     */
     status = bitClientQmiCtrlPointInit();

     GPSONE_DMN_DBG ("%s:%d] bitClientQmiCtrlPointInit returned %d\n",
                    __func__, __LINE__, status);

    if(status != eBIT_CLIENT_SUCCESS)
    {
      GPSONE_DMN_PR_ERR ("%s:%d] bitClientQmiCtrlPointInit returned %d\n",
                    __func__, __LINE__, status);
      break;
    }

    if(true != bitClientSendServiceReady())
    {
      GPSONE_DMN_PR_ERR("%s:%d]: Error calling bitClientSendServiceReady\n",
                  __func__, __LINE__);

      // release the client
      gpsone_glue_qmi_close();

      status = eBIT_CLIENT_FAILURE_INTERNAL;
      break;
    }

  }while(0);

  if(eBIT_CLIENT_SUCCESS != status)
  {
    g_userHandle = BIT_CLIENT_INVALID_HANDLE_VALUE;
    GPSONE_DMN_PR_ERR("%s:%d]: Error! status = %d\n", __func__, __LINE__,status);
  }

  else
  {
     GPSONE_DMN_DBG("%s:%d]: user_handle=%p, status = %d\n",
                __func__, __LINE__, g_userHandle, status);
  }

  return(status);
}

/** gpsone_glue_qmi_send_req
  @brief Sends a message synchronously to the BIT service.
  @param [in] reqId         message ID of the request
  @param [in] reqPayload   Payload of the request, can be NULL
                            if request has no payload

  @return
  One of the following error codes:
  - 0 (eBIT_CLIENT_SUCCESS ) - On success.
  - non-zero error code (see bitClientStatusEnumType) - On failure.
*/

bitClientStatusEnumType gpsone_glue_qmi_send_req(
  uint32_t                 reqId,
  bitClientReqUnionType    reqPayload )
{
  bitClientStatusEnumType status = eBIT_CLIENT_SUCCESS;
  qmi_client_error_type rc = QMI_NO_ERR; //No error
  bit_ack_msg_v01 resp;
  uint32_t reqLen = 0;
  void *pReqData = NULL;

  GPSONE_DMN_DBG("%s:%d] Received reqId= %d \n", __func__,
                __LINE__, reqId);

  // check if we have a valid ahndle
   if(NULL == g_userHandle )
   {
     // did not find the handle in the client List
     GPSONE_DMN_PR_ERR("%s:%d]: invalid handle \n",
                   __func__, __LINE__);

     return(eBIT_CLIENT_FAILURE_INVALID_HANDLE);
   }

  // validate that the request is correct and get reqLen
  if (validateRequest(reqId, reqPayload, &pReqData, &reqLen) == false)
  {

    GPSONE_DMN_PR_ERR("%s:%d] error invalid request\n", __func__,
                __LINE__);

    return(eBIT_CLIENT_FAILURE_INVALID_PARAMETER);
  }

  GPSONE_DMN_DBG("%s:%d] sending reqId= %d, len = %d\n", __func__,
                __LINE__, reqId, reqLen);

  rc = qmi_client_send_msg_sync(
      g_userHandle,
      reqId,
      pReqData,
      reqLen,
      &resp,
      sizeof(resp),
      BIT_CLIENT_ACK_TIMEOUT);

   GPSONE_DMN_DBG("%s:%d] qmi_client_send_msg_sync returned %d resp err: %d resp result: %d\n", __func__,
                __LINE__, rc, resp.resp.error, resp.resp.result );

  if (rc != QMI_NO_ERR)
  {
    GPSONE_DMN_PR_ERR("%s:%d]: send_msg_sync error: %d\n",__func__, __LINE__, rc);
    return(eBIT_CLIENT_FAILURE_INTERNAL);
  }
  status = convertQmiResponseToBitStatus(&resp);
  return (status);
}



