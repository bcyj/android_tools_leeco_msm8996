/*=====================================================================
  @file sensorrdiag.c

  @brief
    This contains main implementation of receiving and processing
    Remote Diag commands on Apps processor.

  Copyright (c) 2010-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
====================================================================*/

/*====================================================================
  INCLUDE FILES
====================================================================*/
#include "sensor1.h"
#define SNS_SMR_C

#ifdef SNS_LA
#  include "comdef.h"
#  include "diag_lsm.h"
#endif

#include "sns_common.h"
#include "sns_common_v01.h"
#include "qmi_client.h"
#include "diagcmd.h"
#include "sensorrdiag.h"
#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include <errno.h>
#include <inttypes.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "sensorrdiag"
#include <utils/Log.h>

/*====================================================================
  DEFINES
====================================================================*/

#define SNS_DIAG_REMOTE_CLIENT    0x0003

#define SNS_RDIAG_MAX_CLIENTS      20
#define SNS_RDIAG_MAX_TXNS         50

#define SNS_RDIAG_APPS_SENSOR1_MSG_OPEN_HDR_SIZE \
         ( sizeof(sns_rdiag_cm_header_type) )
#define SNS_RDIAG_APPS_SENSOR1_MSG_OPEN_BODY_SIZE  0

#define SNS_RDIAG_APPS_SENSOR1_MSG_WRITE_HDR_SIZE \
         ( sizeof(sns_rdiag_cm_header_type) )
#define SNS_RDIAG_APPS_SENSOR1_MSG_WRITE_MIN_BODY_SIZE \
         ( sizeof(sns_rdiag_cm_write_msgbody_s) - 1)

#define SNS_RDIAG_APPS_SENSOR1_MSG_CLOSE_HDR_SIZE \
         ( sizeof(sns_rdiag_cm_header_type) )
#define SNS_RDIAG_APPS_SENSOR1_MSG_CLOSE_BODY_SIZE \
         ( sizeof(sns_rdiag_cm_close_msgbody_s) )

#define SNS_RDIAG_APPS_SENSOR1_RESP_MSG_SIZE \
         ( sizeof(sns_rdiag_sensor1_immediate_res_msg_s) )

/*====================================================================
  STRUCTURE DEFINES
====================================================================*/

/* Struct storing delayed rsp ID for a request */
typedef struct
{
  uint8_t is_valid;
  uint16_t delay_rsp_id;
  int64_t  timestamp; /* Last message sent or received with this txn id */
} delay_rsp_id_association_s;

/*====================================================================
  FUNCTION DECLARATIONS
====================================================================*/

PACK(void *)sns_rdiag_request_handler_apps(PACK(void *), uint16_t);

/*====================================================================
  GLOBAL VARIABLES
====================================================================*/

/* Mutex/cond used to confirm response in callback function */
pthread_mutex_t  rdiag_mutex;
pthread_cond_t   rdiag_cond;

uint8_t          delay_res_flag;  // flags for delay response
uint8_t          rdiag_exit_flag;  // flags for remote diag exit condition

/* Table of handlers to register with DIAG  */
static const diagpkt_user_table_entry_type sns_rdiag_tbl[] =
  {
    /* subsys_cmd_code lo, subsys_cmd_code hi, call back function */
    {SNS_DIAG_REMOTE_CLIENT, SNS_DIAG_REMOTE_CLIENT, sns_rdiag_request_handler_apps},
  };

/* Array used for associating cb_data (a.k.a client_id) with context
 * handle from sensor1_open Client ID starting from 1 */
sensor1_handle_s *client_association_arr[SNS_RDIAG_MAX_CLIENTS + 1];

/* Array used for associating txn_id with delayed rsp Id from DIAG */
delay_rsp_id_association_s delay_rsp_id_association_arr[SNS_RDIAG_MAX_TXNS];

/*====================================================================
  FUNCTIONS
====================================================================*/

/*====================================================================
  FUNCTION: sns_rdiag_get_client_id
====================================================================*/
/*
  @brief
  Assigns a client ID to the Diag client.

  @return
  Integer value representing the Client ID.
  Zero value indicates error
  Positive value indicates Client ID.
*/
static uint32_t
sns_rdiag_get_client_id()
{
  uint8_t i;

  /* Check for a free context handle value */
  for(i = 1; i <= SNS_RDIAG_MAX_CLIENTS; i++)
  {
    if(NULL == client_association_arr[i])
    {
      return i;
    }
  }

  return 0;
}

/*====================================================================
  FUNCTION: sns_rdiag_free_client_id
====================================================================*/
/*
  @brief
  Frees a allocated client Identifier number.

  @param[i] id: Client ID

  @return
  No return value
*/
static void
sns_rdiag_free_client_id(uint32_t id)
{
  client_association_arr[id] = NULL;
  return;
}

/*============================================================================

  FUNCTION:   sns_rdiag_curr_ts

  ==========================================================================*/
/*!
  @brief Return the current LA timestamp (in ns).

  @return Apps timestamp; nanoseconds since last epoch.
*/
/*==========================================================================*/
static int64_t
sns_rdiag_curr_ts()
{
  struct timespec ts_apps;

  if( 0 != clock_gettime( CLOCK_REALTIME, &ts_apps ) )
  {
    LOGE("%s: Error with clock_gettime %i", __func__, errno );
    return 0;
  }

  return ((int64_t)ts_apps.tv_sec * 1000000000) + ts_apps.tv_nsec;
}

/*====================================================================
  FUNCTION: sns_rdiag_assign_txn_id
====================================================================*/
/*
  @brief
  Assigns a transaction ID.

  @param[o] txn_id: A Pointer to the placeholder for the transaction
  id value

  @return
  Error Code for the operation.
*/
static sensor1_error_e
sns_rdiag_assign_txn_id(uint8_t *txn_id)
{
  uint8_t i, oldest_txn_id = 0;
  uint16_t  arr_size;
  int64_t  oldest_ts = 0;

  if(NULL == txn_id)
  {
    return SENSOR1_ENOMEM;
  }
  arr_size = sizeof(delay_rsp_id_association_arr);

  /* check for a free transaction ID value */
  for(i = 0; i < SNS_RDIAG_MAX_TXNS; i++)
  {
    if(delay_rsp_id_association_arr[i].is_valid == 0)
    {
      *txn_id = i;
      delay_rsp_id_association_arr[i].is_valid = 1;
      delay_rsp_id_association_arr[i].delay_rsp_id = 0;
      delay_rsp_id_association_arr[i].timestamp = sns_rdiag_curr_ts();

      return SENSOR1_SUCCESS;
    }
    else if(delay_rsp_id_association_arr[i].timestamp < oldest_ts)
    {
      oldest_ts = delay_rsp_id_association_arr[i].timestamp;
      oldest_txn_id = i;
    }
  }

  LOGE("%s: No txn ID available, using %i (last used %"PRIu64", now %"PRIu64")",
       __func__, oldest_txn_id, delay_rsp_id_association_arr[oldest_txn_id].timestamp,
       sns_rdiag_curr_ts());
  *txn_id = i;
  delay_rsp_id_association_arr[oldest_txn_id].is_valid = 1;
  delay_rsp_id_association_arr[oldest_txn_id].delay_rsp_id = 0;
  delay_rsp_id_association_arr[oldest_txn_id].timestamp = sns_rdiag_curr_ts();

  return SENSOR1_SUCCESS;
}

/*====================================================================
  FUNCTION: sns_rdiag_clear_txn_id
====================================================================*/
/*
  @brief
  Clears a transaction ID.

  @param[i] txn_id: A Pointer to the placeholder for the transaction
  id value

  @return
  No return value.
*/
static void
sns_rdiag_clear_txn_id(uint8_t txn_id)
{
  delay_rsp_id_association_arr[txn_id].is_valid = 0;
  delay_rsp_id_association_arr[txn_id].delay_rsp_id = 0;
  delay_rsp_id_association_arr[txn_id].timestamp = 0;
}

/*====================================================================
  FUNCTION: sns_rdiag_assign_delayed_rsp_id
====================================================================*/
/*
  @brief
  Clears a transaction ID.

  @param[i] txn_id : Value of the transaction ID
  @param[i] rsp    : Pointer to the immediate response message

  @return
  No return value.
*/
static void
sns_rdiag_assign_delayed_rsp_id(uint8_t txn_id,
              sns_rdiag_sensor1_immediate_res_msg_s *rsp)
{
  if(0 != delay_rsp_id_association_arr[txn_id].is_valid)
  {
    delay_rsp_id_association_arr[txn_id].delay_rsp_id =
      diagpkt_subsys_get_delayed_rsp_id( rsp);
    delay_rsp_id_association_arr[txn_id].timestamp = sns_rdiag_curr_ts();
  }
}

/*====================================================================
  FUNCTION: sns_rdiag_get_delay_rsp_id
====================================================================*/
/*
  @brief
  Gets the delayed response ID.

  @param[i] txn_id: Value of the transaction ID
  @param[o] delay_rsp_id: Response ID found.

  @return
  Error code for the operation.
*/
static sensor1_error_e
sns_rdiag_get_delay_rsp_id(uint8_t txn_id, uint16_t *delay_rsp_id)
{
  if(NULL == delay_rsp_id ||
      txn_id >= SNS_RDIAG_MAX_TXNS)
  {
    LOGE("%s: Input ERROR, txn_id = %d", __func__, txn_id);
    return SENSOR1_EFAILED;
  }

  if(0 != delay_rsp_id_association_arr[txn_id].is_valid)
  {
    delay_rsp_id_association_arr[txn_id].timestamp = sns_rdiag_curr_ts();
    *delay_rsp_id = delay_rsp_id_association_arr[txn_id].delay_rsp_id;
    return SENSOR1_SUCCESS;
  }

  return SENSOR1_EFAILED;
}

/*====================================================================
  FUNCTION: rdiag_writable_cb
====================================================================*/
/*
  @brief
  This is a callback function registerd by calling sensor1_writable
*/
void
rdiag_writable_cb(intptr_t cbdata_in, uint32_t service_id)
{
  uint32_t cbdata = (uint32_t)cbdata_in;
  sensor1_handle_s *context_hdl = NULL;
  sns_rdiag_sensor1_delayed_rsp_msg_s *sensor1_delay_rsp = NULL;

  if(SNS_RDIAG_MAX_CLIENTS < cbdata)
  {
    LOGE("%s: Sensor1 Writable Callback: Input parameter err, cbdata = %i",
         __func__, cbdata);
    delay_res_flag = 2;
    return;
  }

  context_hdl = client_association_arr[cbdata];

  sensor1_delay_rsp = diagpkt_subsys_alloc_v2_delay(
        DIAG_SUBSYS_SENSORS,
        SNS_DIAG_REMOTE_CLIENT, 0,
        (sizeof(sns_rdiag_sensor1_delayed_rsp_msg_s) - 1));

  if(NULL == sensor1_delay_rsp)
  {
    LOGE("%s: No memory for delayed response", __func__);
    delay_res_flag = 2;
    return;
  }

  /* This is the final delayed response */
  diagpkt_subsys_set_rsp_cnt(sensor1_delay_rsp, 1);

  sensor1_delay_rsp->cb_data        = cbdata;
  sensor1_delay_rsp->context        = (uint32_t)(uintptr_t)context_hdl;
  sensor1_delay_rsp->srvc_num       = service_id;

  diagpkt_delay_commit((void *)sensor1_delay_rsp);

#ifdef SNS_RDIAG_DEBUG
  LOGD("%s: delay writable res committed", __func__);
#endif

  delay_res_flag = 2;
  return;
}

/*====================================================================
  FUNCTION: rdiag_notify_data_cb
====================================================================*/
/*
  @brief
  This is a callback function registered by calling sensor1_open
*/
void
rdiag_notify_data_cb(intptr_t cbdata_in,
                     sensor1_msg_header_s *msg_hdr_ptr,
                     sensor1_msg_type_e  msg_type,
                     void *msg_ptr)
{
  uint32_t cbdata = (uint32_t)cbdata_in;
  sensor1_handle_s *context_hdl = NULL;
  sns_rdiag_sensor1_delayed_rsp_msg_s *sensor1_delay_rsp = NULL;
  uint16_t delay_rsp_id;

  if(NULL == msg_hdr_ptr)
  {
    LOGE("%s: msg_hdr_ptr is NULL(Sensor1_close)", __func__);
    delay_res_flag = 1;
    return;
  }

  if(SNS_RDIAG_MAX_CLIENTS < cbdata)
  {
    LOGE("%s: Input parameters error, cbdata=%d", __func__, cbdata);
    delay_res_flag = 1;
    return;
  }

  /* Get the context handle */
  context_hdl = client_association_arr[cbdata];

  if(SENSOR1_MSG_TYPE_RESP!= msg_type &&
     SENSOR1_MSG_TYPE_RESP_INT_ERR != msg_type)
  {
    LOGE("%s: Received invalid message type in callback %i", __func__, msg_type);
    sensor1_free_msg_buf(context_hdl, msg_ptr);
    delay_res_flag = 1;
    return;
  }

  /* Get the delayed response ID for filling up the delayed res */
  if(SENSOR1_SUCCESS != sns_rdiag_get_delay_rsp_id(msg_hdr_ptr->txn_id, &delay_rsp_id))
  {
    LOGE("%s: Could not get delayed rsp_id", __func__);
    sns_rdiag_clear_txn_id(msg_hdr_ptr->txn_id);
    sensor1_free_msg_buf(context_hdl, msg_ptr);
    delay_res_flag = 1;
    return;
  }

  LOGD("%s: Retrieved delay_rsp_id = %d, txn_id = %d",
       __func__, delay_rsp_id, msg_hdr_ptr->txn_id);

  sensor1_delay_rsp = diagpkt_subsys_alloc_v2_delay(
      DIAG_SUBSYS_SENSORS,
      SNS_DIAG_REMOTE_CLIENT, delay_rsp_id,
      sizeof(sns_rdiag_sensor1_delayed_rsp_msg_s) -1 + msg_hdr_ptr->msg_size);

  if(NULL == sensor1_delay_rsp)
  {
    LOGE("%s: No memory for delayed response", __func__);
    sns_rdiag_clear_txn_id(msg_hdr_ptr->txn_id );
    sensor1_free_msg_buf(context_hdl, msg_ptr);
    delay_res_flag = 1;
    return;
  }

  /* This is the final delayed response */
  diagpkt_subsys_set_rsp_cnt(sensor1_delay_rsp, 1);

  sensor1_delay_rsp->delayed_rsp_id = delay_rsp_id;
  sensor1_delay_rsp->txn_id         = msg_hdr_ptr->txn_id;
  sensor1_delay_rsp->cb_data        = cbdata;
  sensor1_delay_rsp->context        = (uint32_t)(uintptr_t)context_hdl;
  sensor1_delay_rsp->srvc_num       = msg_hdr_ptr->service_number;
  sensor1_delay_rsp->msg_id         = msg_hdr_ptr->msg_id;
  sensor1_delay_rsp->msg_type       = msg_type;
  sensor1_delay_rsp->msg_size       = msg_hdr_ptr->msg_size;
  SNS_OS_MEMCOPY(&sensor1_delay_rsp->msg, msg_ptr, msg_hdr_ptr->msg_size);

  diagpkt_delay_commit((void *)sensor1_delay_rsp);
  sensor1_free_msg_buf(context_hdl, msg_ptr);
  sns_rdiag_clear_txn_id(msg_hdr_ptr->txn_id);

  delay_res_flag = 1;
  LOGD("%s: Delay response committed.", __func__);

  return;
}

/*====================================================================
  FUNCTION: confirm_response
====================================================================*/
/*
  @brief
  Wait for delayed response. Make sure to receive delayed response
  before return to DIAG.

  @return
  SENSOR1_SUCCESS or error code
*/
static sensor1_error_e
confirm_response()
{
  struct timespec  ts;
  int err = 0;

  pthread_mutex_lock(&rdiag_mutex);
  clock_gettime(CLOCK_REALTIME, &ts);

  while(0 == delay_res_flag && 0 == err)
  {
    ts.tv_nsec += 1000000000;  // wait 1sec
    err = pthread_cond_timedwait(&rdiag_cond, &rdiag_mutex, &ts);
  }

  delay_res_flag = 0;
  pthread_cond_signal(&rdiag_cond);
  pthread_mutex_unlock(&rdiag_mutex);

  if(0 != err)
  {
    LOGE("%s: Error while waiting for callback %d", __func__, err);
    return err;
  }

  return SENSOR1_SUCCESS;
}

/*====================================================================
  FUNCTION: sns_rdiag_send_immed_resp
====================================================================*/
/*
  @brief
  Allocates memory through DIAG api, fills up the immediate response
  and commits response.

  @param[i] sensor1_api_ret_value  : Return value from sensor1 API
          function call if sensor1 API was called. In case there were
          errors with the diag request before calling the sensor1 API,
          this field contains the corresponding sensor1 error code.
  @param[i] cb_data     : Contains the Client ID assigned by the
                          sensors Diag handler Module
  @param[i] context     : Context handle returned from sensor1 API
  @param[i] delayed_rsp_id  : Delayed response identifier. This is
                              assigned by DIAG and is used to match
                              delayed responses
  @param[i] txn id     : Transaction ID assigned by the Sensors Diag
                         Handler Module
  @param[i] rsp_cnt    : Number of delayed responses to follow. If
                         it is 1, it means the immediate response is
                         the final response

  @return
  Error code for the operation.
*/
static sensor1_error_e
sns_rdiag_send_immed_resp(int32_t sensor1_api_ret_value, uint32_t cb_data,
    sensor1_handle_s *context, uint16_t delayed_rsp_id, uint8_t  txn_id,
    uint8_t  rsp_cnt)
{
  sns_rdiag_sensor1_immediate_res_msg_s *sns_diag_cm_res_ptr = NULL;

  sns_diag_cm_res_ptr = (sns_rdiag_sensor1_immediate_res_msg_s *)
          diagpkt_subsys_alloc_v2(DIAG_SUBSYS_SENSORS,
                                  SNS_DIAG_REMOTE_CLIENT,
                                  SNS_RDIAG_APPS_SENSOR1_RESP_MSG_SIZE);

  if(NULL == sns_diag_cm_res_ptr)
  {
    LOGE("%s:Malloc failure: Could not allocate memory for "
         "Sensor1 Immediate Response", __func__);
    return SENSOR1_ENOMEM;
  }

  /* Fill out the immediate response */
  sns_diag_cm_res_ptr->sensor1_api_ret_value = sensor1_api_ret_value;
  sns_diag_cm_res_ptr->cb_data               = cb_data;
  sns_diag_cm_res_ptr->context               = (uint32_t)(uintptr_t)context;
  sns_diag_cm_res_ptr->delayed_rsp_id        = delayed_rsp_id;
  sns_diag_cm_res_ptr->txn_id                = txn_id;

  /* If the immediate response is the final response, Set Response
     Cnt that is in the diag header */
  if(1 == rsp_cnt)
  {
    diagpkt_subsys_set_rsp_cnt(sns_diag_cm_res_ptr, 1);
  }

  diagpkt_commit(sns_diag_cm_res_ptr);
  return SENSOR1_SUCCESS;
}

/*====================================================================
  FUNCTION: sns_rdiag_msg_handle_open
====================================================================*/
/*
  @brief
  Handle an sensor1 open request.

  @param[i] req_pkt_ptr : The request packet (sns_rdiag_cm_msg_body_s)
  @param[i] pkt_len     : Length of req_pkt_ptr

  @return
  None
*/
static void
sns_rdiag_msg_handle_open(byte *req_pkt_ptr, uint16_t pkt_len)
{
  sns_rdiag_cm_msg_body_s *parsed_msg_ptr = (sns_rdiag_cm_msg_body_s *)req_pkt_ptr;
  uint32_t client_id = 0;
  sensor1_error_e sensor1_err;

  LOGD("%s:SENSOR1_Open rcvd, pkt_len(%d)", __func__, pkt_len);

  if((pkt_len < (SNS_RDIAG_APPS_SENSOR1_MSG_OPEN_BODY_SIZE +
                  SNS_RDIAG_APPS_SENSOR1_MSG_OPEN_HDR_SIZE)))
  {
    LOGE("%s:Invalid SENSOR1_Open Request due to pkt size(%d)"
          "or cb_data", __func__, pkt_len);
    sns_rdiag_send_immed_resp(SENSOR1_EBAD_PARAM, 0, NULL, 0, 0, 1);
    return ;
  }

  // Assign a cb_data value i.e client id Client ID 0 means error
  client_id = sns_rdiag_get_client_id();
  if(0 == client_id)
  {
    LOGE("%s: Client id couldn't be assigned", __func__);
    sns_rdiag_send_immed_resp(SENSOR1_EFAILED, 0, NULL, 0, 0, 1);
    return ;
  }

  sensor1_err = sensor1_open(&client_association_arr[client_id],
                          rdiag_notify_data_cb, client_id );

  LOGE("%s:Packet type 2 sensor1_open error = %d", __func__, sensor1_err);

  sns_rdiag_send_immed_resp(sensor1_err, client_id,
      client_association_arr[client_id], 0, 0, 1 );

  /* Free client ID if Open had an error */
  if(SENSOR1_SUCCESS != sensor1_err)
  {
    sns_rdiag_free_client_id(client_id);
  }
  pthread_cond_signal(&rdiag_cond);
}

/*====================================================================
  FUNCTION: sns_rdiag_msg_handle_write
====================================================================*/
/*
  @brief
  Handle an sensor1 write request.

  @param[i] req_pkt_ptr : The request packet (sns_rdiag_cm_msg_body_s)
  @param[i] pkt_len     : Length of req_pkt_ptr
  @param[i] pkt_type    : 2 if message is QMI encoded; 3 if raw byte stream

  @return
  None
*/
static void
sns_rdiag_msg_handle_write(byte *req_pkt_ptr, uint16_t pkt_len, uint8_t pkt_type)
{
  sns_rdiag_cm_msg_body_s *parsed_msg_ptr = (sns_rdiag_cm_msg_body_s *)req_pkt_ptr;
  sensor1_error_e sensor1_err;
  void *qmi_msg_ptr = NULL;
  sns_rdiag_sensor1_immediate_res_msg_s *response_ptr = NULL;
  sensor1_handle_s *context_hdl = NULL;
  sensor1_msg_header_s msg_hdr;
  qmi_idl_service_object_type service = NULL;
  uint32_t ctype_len = 0;
  int32_t qmi_err;

  LOGD("%s: svc num: %i; msg_id: %i; msg type: %i; msg len: %i; txn id: %i (pkt len: %i)",
       __func__, parsed_msg_ptr->write_msgbody.srvc_num,
       parsed_msg_ptr->write_msgbody.msg_id, parsed_msg_ptr->write_msgbody.msg_type,
       parsed_msg_ptr->write_msgbody.msg_size, parsed_msg_ptr->write_msgbody.txn_id,
       pkt_len);

  if(parsed_msg_ptr->write_msgbody.msg_size !=
     pkt_len - SNS_RDIAG_APPS_SENSOR1_MSG_WRITE_MIN_BODY_SIZE - 1)
  {
    LOGE("%s: Invalid pkt_len value -> Test set-up issue (msg len: %i; pkt len: %i)",
         __func__, parsed_msg_ptr->write_msgbody.msg_size, pkt_len);
    sns_rdiag_send_immed_resp(SENSOR1_EBAD_PARAM, 0, NULL, 0, 0, 1);
    return ;
  }
  else if((pkt_len < (SNS_RDIAG_APPS_SENSOR1_MSG_WRITE_MIN_BODY_SIZE +
                  SNS_RDIAG_APPS_SENSOR1_MSG_WRITE_HDR_SIZE)) ||
    (parsed_msg_ptr->write_msgbody.cbdata > SNS_RDIAG_MAX_CLIENTS) ||
    (client_association_arr[parsed_msg_ptr->write_msgbody.cbdata] == NULL))
  {
    LOGE("%s:Invalid Sensor1_write Request. pkt_len(%d)", __func__, pkt_len);
    sns_rdiag_send_immed_resp(SENSOR1_EBAD_PARAM, 0, NULL, 0, 0, 1);
    return ;
  }

  /* Obtain context handle from cb_data value (input) */
  context_hdl =
    client_association_arr[parsed_msg_ptr->write_msgbody.cbdata];

  msg_hdr.service_number = parsed_msg_ptr->write_msgbody.srvc_num;
  msg_hdr.msg_id = parsed_msg_ptr->write_msgbody.msg_id;

  /* Copy Message */
  if(0 != parsed_msg_ptr->write_msgbody.msg_size)
  {
    if(SNS_SMR_RTB_SIZE <= msg_hdr.service_number ||
       NULL == sns_rtb[msg_hdr.service_number].svc_map.get_svc_obj)
    {
      LOGE("%s: Unsupported service %i", __func__, msg_hdr.service_number);
      sns_rdiag_send_immed_resp(SENSOR1_EFAILED, 0, NULL, 0, 0, 1);
      return ;
    }

    service = sns_rtb[msg_hdr.service_number].svc_map.get_svc_obj(
          sns_rtb[msg_hdr.service_number].svc_map.maj_ver,
          sns_rtb[msg_hdr.service_number].svc_map.min_ver,
          sns_rtb[msg_hdr.service_number].svc_map.tool_ver);

    if(NULL == service)
    {
      LOGE("%s: Unable to get service object", __func__);
      sns_rdiag_send_immed_resp(SENSOR1_EFAILED, 0, NULL, 0, 0, 1);
      return ;
    }

    qmi_err = qmi_idl_get_message_c_struct_len(service, QMI_IDL_REQUEST,
                                               msg_hdr.msg_id, &ctype_len);
    if(QMI_NO_ERR != qmi_err)
    {
      LOGE("%s: qmi message len get failed(%d)", __func__, qmi_err);
      sns_rdiag_send_immed_resp(SENSOR1_EFAILED, 0, NULL, 0, 0, 1);
      return ;
    }

    sensor1_err = sensor1_alloc_msg_buf(context_hdl, ctype_len, &qmi_msg_ptr);
  }
  else
  {
    sensor1_err = sensor1_alloc_msg_buf(context_hdl, 0, &qmi_msg_ptr);
  }

  if(SENSOR1_SUCCESS != sensor1_err|| NULL == qmi_msg_ptr)
  {
    LOGE("%s: sensor1_alloc request failed", __func__);
    sns_rdiag_send_immed_resp(SENSOR1_EFAILED, 0, NULL, 0, 0, 1);
    return ;
  }

  if(2 == pkt_type)
  {
    msg_hdr.msg_size = ctype_len;

    if(0 != parsed_msg_ptr->write_msgbody.msg_size)
    {
      qmi_err = qmi_idl_message_decode(
                    service,
                    QMI_IDL_REQUEST,
                    msg_hdr.msg_id,
                    (const void*)parsed_msg_ptr->write_msgbody.qmi_msg,
                    parsed_msg_ptr->write_msgbody.msg_size,
                    qmi_msg_ptr,
                    ctype_len);

      if(QMI_NO_ERR != qmi_err)
      {
        LOGE("%s: qmi message decode failed(%d)", __func__, qmi_err);
        sns_rdiag_send_immed_resp(SENSOR1_EFAILED, 0, NULL, 0, 0, 1);
        sensor1_free_msg_buf(context_hdl, qmi_msg_ptr);
        return ;
      }
    }
  }
  else if(3 == pkt_type)
  {
    memset(qmi_msg_ptr, 0x00, parsed_msg_ptr->write_msgbody.msg_size);

    msg_hdr.service_number = parsed_msg_ptr->write_msgbody.srvc_num;
    msg_hdr.msg_id = parsed_msg_ptr->write_msgbody.msg_id;
    msg_hdr.msg_size = parsed_msg_ptr->write_msgbody.msg_size;

    memcpy(qmi_msg_ptr, parsed_msg_ptr->write_msgbody.qmi_msg,
                        parsed_msg_ptr->write_msgbody.msg_size);
  }
  else
  {
    LOGE("%s: Invalid packet type: %i", __func__, pkt_type);
    sns_rdiag_send_immed_resp(SENSOR1_EFAILED, 0, NULL, 0, 0, 1);
    sensor1_free_msg_buf(context_hdl, qmi_msg_ptr);
    return ;
  }

  if(SENSOR1_SUCCESS != sns_rdiag_assign_txn_id(&msg_hdr.txn_id))
  {
    LOGE("%s:Sensor1_Write Couldn't assign TXN ID", __func__);
    sns_rdiag_send_immed_resp(SENSOR1_EFAILED, 0, NULL, 0, 0, 1);
    sensor1_free_msg_buf(context_hdl, qmi_msg_ptr);
    return ;
  }

  LOGD("%s:Sensor1_Write: service num(%d), Msg ID(%d), Msg size(%d), Txn id(%d)",
       __func__, msg_hdr.service_number, msg_hdr.msg_id,
       msg_hdr.msg_size, msg_hdr.txn_id);

  sensor1_err = sensor1_write(context_hdl, &msg_hdr, qmi_msg_ptr);

  LOGE("%s: Sensor1_write error = %d", __func__, sensor1_err);

  if(SENSOR1_SUCCESS != sensor1_err)
  {
    sns_rdiag_send_immed_resp( sensor1_err, 0, NULL, 0, 0, 1);
    sensor1_free_msg_buf( context_hdl, qmi_msg_ptr);
    sns_rdiag_clear_txn_id(msg_hdr.txn_id);
    return ;
  }

  /* Fill response. Function to fill response is not called because
      we can't create the delayed rsp id unless the alloc with DIAG success */
  response_ptr = (sns_rdiag_sensor1_immediate_res_msg_s *)
        diagpkt_subsys_alloc_v2(DIAG_SUBSYS_SENSORS,
                                SNS_DIAG_REMOTE_CLIENT,
                                SNS_RDIAG_APPS_SENSOR1_RESP_MSG_SIZE);
  if(NULL == response_ptr)
  {
    LOGE("%s: Could not allocate memory for CM Diag response", __func__);
    sns_rdiag_clear_txn_id(msg_hdr.txn_id);
    return ;
  }

  sns_rdiag_assign_delayed_rsp_id( msg_hdr.txn_id, response_ptr);
  response_ptr->sensor1_api_ret_value = sensor1_err;
  response_ptr->cb_data = parsed_msg_ptr->write_msgbody.cbdata;
  response_ptr->context = (uint32_t)(uintptr_t)
    client_association_arr[parsed_msg_ptr->write_msgbody.cbdata];
  response_ptr->delayed_rsp_id = response_ptr->diag_hdr.delayed_rsp_id;
  response_ptr->txn_id = msg_hdr.txn_id;

  (void)diagpkt_commit( response_ptr );
  confirm_response();
}

/*====================================================================
  FUNCTION: sns_rdiag_msg_handle_close
====================================================================*/
/*
  @brief
  Handle an sensor1 close request.

  @param[i] req_pkt_ptr : The request packet (sns_rdiag_cm_msg_body_s)
  @param[i] pkt_len     : Length of req_pkt_ptr

  @return
  None
*/
static void
sns_rdiag_msg_handle_close(byte *req_pkt_ptr, uint16_t pkt_len)
{
  sensor1_handle_s *context_hdl = NULL;
  sns_rdiag_cm_msg_body_s *parsed_msg_ptr = (sns_rdiag_cm_msg_body_s *)req_pkt_ptr;
  sensor1_error_e sensor1_err;

  LOGD("%s:SENSOR1_Close rcvd, pkt_len(%d)", __func__, pkt_len);

  if((pkt_len < (SNS_RDIAG_APPS_SENSOR1_MSG_CLOSE_BODY_SIZE +
                 SNS_RDIAG_APPS_SENSOR1_MSG_CLOSE_HDR_SIZE) ) ||
      (parsed_msg_ptr->close_msgbody.cbdata > SNS_RDIAG_MAX_CLIENTS) ||
      (client_association_arr[parsed_msg_ptr->close_msgbody.cbdata] == NULL) )
  {
    LOGE("%s:Invalid Sensor1_Close request, pkt len = %d, cb_data= %i",
          __func__, pkt_len, parsed_msg_ptr->close_msgbody.cbdata);
    sns_rdiag_send_immed_resp(SENSOR1_EBAD_PARAM, 0, NULL, 0, 0, 1);
    return ;
  }

  context_hdl = client_association_arr[parsed_msg_ptr->close_msgbody.cbdata];

  sensor1_err = sensor1_close(context_hdl);

  if(SENSOR1_SUCCESS != sensor1_err)
  {
    LOGE("%s: Packet type 2 sensor1_close error = %d", __func__, sensor1_err);
  }

  sns_rdiag_send_immed_resp(sensor1_err,
      parsed_msg_ptr->close_msgbody.cbdata, context_hdl, 0, 0, 1);

  /* Remove association of cb_data with context handle */
  sns_rdiag_free_client_id(parsed_msg_ptr->close_msgbody.cbdata);

  pthread_cond_signal(&rdiag_cond);
}

/*====================================================================
  FUNCTION: sns_rdiag_msg_handle_writable
====================================================================*/
/*
  @brief
  Handle an sensor1 close request.

  @param[i] req_pkt_ptr : The request packet (sns_rdiag_cm_msg_body_s)
  @param[i] pkt_len     : Length of req_pkt_ptr

  @return
  None
*/
static void
sns_rdiag_msg_handle_writable(byte *req_pkt_ptr, uint16_t pkt_len)
{
  sensor1_handle_s *context_hdl = NULL;
  sns_rdiag_cm_msg_body_s *parsed_msg_ptr = (sns_rdiag_cm_msg_body_s *)req_pkt_ptr;
  sensor1_error_e sensor1_err;

  LOGD("%s:SENSOR1_Writable rcvd, pkt_len(%d)", __func__, pkt_len);

  context_hdl =
    client_association_arr[parsed_msg_ptr->writable_msgbody.cbdata];

  sensor1_err = sensor1_writable(context_hdl, rdiag_writable_cb,
              parsed_msg_ptr->writable_msgbody.cbdata,
              (uint32_t)parsed_msg_ptr->writable_msgbody.srvc_num);

  sns_rdiag_send_immed_resp(sensor1_err,
              parsed_msg_ptr->writable_msgbody.cbdata,
              context_hdl, 0, 0, 1);

  confirm_response();
}

/*====================================================================
  FUNCTION: sns_rdiag_msg_handler
====================================================================*/
/*
  @brief
  This function is called to handle Remote Diag commands. The QMI encoded
  messages contained in the DIAG commands are sent to the Sensor1 API.

  @param[i] req_pkt_ptr     : A pointer to the diag command packet
  @param[i] pkt_len         : Length of the command packet
  @param[i] pkt_type        : 2 if message is QMI encoded; 3 if raw byte stream

  @return
  Immediate response (sns_rdiag_sensor1_immediate_res_msg_s) if
  any. In this implementation NULL pointer is returned as we are
  committing our response to DIAG within the handler function.
*/
void *
sns_rdiag_msg_handler(byte *req_pkt_ptr, uint16_t pkt_len, uint8_t pkt_type)
{
  sns_rdiag_cm_msg_body_s *parsed_msg_ptr = (sns_rdiag_cm_msg_body_s *) req_pkt_ptr;

  if(0 == pkt_len || NULL == req_pkt_ptr)
  {
    LOGE("%s:Invalid Request (size 0 or NULL ptr)", __func__);
    sns_rdiag_send_immed_resp(SENSOR1_EBAD_PARAM, 0, NULL, 0, 0, 1);
    return NULL;
  }

  switch(parsed_msg_ptr->cm_fn)
  {
    case SNS_DIAG_SENSOR1_OPEN:
      sns_rdiag_msg_handle_open(req_pkt_ptr, pkt_len);
      break;
    case SNS_DIAG_SENSOR1_WRITE:
      sns_rdiag_msg_handle_write(req_pkt_ptr, pkt_len, pkt_type);
      break;
    case SNS_DIAG_SENSOR1_CLOSE:
      sns_rdiag_msg_handle_close(req_pkt_ptr, pkt_len);
      break;
    case SNS_DIAG_SENSOR1_WRITABLE:
      sns_rdiag_msg_handle_writable(req_pkt_ptr, pkt_len);
      break;
    default:
      LOGE("%s:Unknown Sensor1 function = %d", __func__,
                 parsed_msg_ptr->cm_fn);
      break;
  }

  return NULL;
}

/*====================================================================
  FUNCTION: sns_rdiag_reqeust_handler_apps
====================================================================*/
/*
  @brief
  Handles a remote diag request packet.

  @detail
  The diag packet sent will have the following format.

 function ||             header                        ||     byte1     ||  byte 2..n ||
 _________||___________________________________________||_______________||____________||
 send_data||Command_type|Subsystem|SNS Subsystem|Unused|| Packet Type   || Sensors Hdr||
          ||   128      |   64    | 3 = REMOTE DIAG | 0 ||2/3= Msg thru ||            ||
          ||                                           ||   sensor1     ||            ||

 ||   byte n..      ||
 ||_________________||
 ||   Sensors       ||
 ||   Payload       ||
 ||                 ||
 ||                 ||

  @param[i] req_pkt_ptr    : A pointer to the diag command packet
  @param[i] pkt_len        : Length of the command packet

  @return
  Immediate response if any. In this implementation NULL pointer
  is returned as we are committing our response to DIAG within the
  handler function.
*/
PACK(void *)
sns_rdiag_request_handler_apps(PACK(void *)req_pkt_ptr, uint16_t pkt_len)
{
  sns_rdiag_handle_req_s  *sns_diag_req_ptr =
                          (sns_rdiag_handle_req_s *)req_pkt_ptr;
  void *sns_diag_resp_ptr = NULL;
  uint16_t payload_size = 0;

  LOGE("\n%s:Remote Diag Req rcvd. pkt_len: %d", __func__, pkt_len);

  if(NULL == sns_diag_req_ptr ||
     pkt_len <= sizeof(sns_diag_req_ptr->header) + sizeof(sns_diag_req_ptr->pkt_type))
  {
    LOGE("%s:Invalid Remote Diag Request!", __func__);
    return NULL;
  }

  /* Payload size to send to handler =
   Payload size - Diag header field length - Pkt Type field length */
  payload_size = pkt_len -
                 sizeof(sns_diag_req_ptr->header) - sizeof(sns_diag_req_ptr->pkt_type);

  if(SNS_DIAG_SENSOR1_QMI_MSG == sns_diag_req_ptr->pkt_type)
  {
    sns_diag_resp_ptr =
      sns_rdiag_msg_handler(sns_diag_req_ptr->payload, payload_size, 2);
  }
  else if(SNS_DIAG_SENSOR1_MSG == sns_diag_req_ptr->pkt_type)
  {
    sns_diag_resp_ptr =
      sns_rdiag_msg_handler(sns_diag_req_ptr->payload, payload_size, 3);
  }
  else
  {
    LOGE("%s: Received invalid packet type %i (len %i)", __func__,
         sns_diag_req_ptr->pkt_type, payload_size);
  }

  return sns_diag_resp_ptr;
}

/*====================================================================
  FUNCTION: register_diag
====================================================================*/
/*
  @brief Registers diag packet to receive remote diag request from QXDM

  @return
  SENSOR1_SUCCESS or error code
*/
sensor1_error_e
register_diag()
{
#ifdef SNS_LA
  boolean sns_diag_init_b = FALSE;

  sns_diag_init_b = Diag_LSM_Init(NULL);
  if(!sns_diag_init_b)
  {
    LOGE("%s:Diag_LSM_Init() failed. Exiting Sensorrdiag...", __func__);
    rdiag_exit_flag = 1;

    return SENSOR1_EFAILED;
  }
#endif

  /* Registering diag packet for delayed responses with sensors
     subsystem id.
     To execute on QXDM :: "send_data 128 64 2 0 <Sensors Payload>"
  */
  DIAGPKT_DISPATCH_TABLE_REGISTER_V2_DELAY(DIAG_SUBSYS_CMD_VER_2_F,
                                      DIAG_SUBSYS_SENSORS,
                                      sns_rdiag_tbl);

  return SENSOR1_SUCCESS;
}

/*====================================================================
  FUNCTION: main
====================================================================*/
/*
  @brief  Main function for the Sensor Remote Diag Proxy process.
  Handles starting the process, registering remote diag packet for
  delayed responses with sensors subsystem id.
  To execute on QXDM :: "send_data 128 64 3 0 <Sensors Payload>"
  Handles libsensor1 initialization.

  @param[i] argc: Count of arguments on the command line.
  @param[i] argv: Array of strings containing command line arguments.

  @return
  0 - no error
*/
int
main(int argc, char *argv[])
{
  UNREFERENCED_PARAMETER(argc);
  UNREFERENCED_PARAMETER(argv);
  int err = 0;
  int i = 0;
  sensor1_error_e error;

  LOGE("Sensorrdiag starting...");

  for(i = 0; i < SNS_RDIAG_MAX_CLIENTS; i++)
  {
    sns_rdiag_free_client_id(i);
  }

  for(i = 0; i < SNS_RDIAG_MAX_TXNS; i++)
  {
    memset(&delay_rsp_id_association_arr[i], 0x00, sizeof(delay_rsp_id_association_s));
  }

  delay_res_flag = 0;
  rdiag_exit_flag = 0;

  pthread_mutex_init(&rdiag_mutex, NULL);
  pthread_cond_init(&rdiag_cond, NULL);

  error = sensor1_init();

  if(SENSOR1_SUCCESS != error)
  {
    LOGE("%s:Sensor1_init failed %i", __func__, error);
  }

  register_diag();
  pthread_mutex_lock( &rdiag_mutex );

  while(0 == rdiag_exit_flag && 0 == err)
  {
    err = pthread_cond_wait(&rdiag_cond, &rdiag_mutex);
#ifdef SNS_RDIAG_DEBUG
    LOGV("%s: Woke up", __func__);
#endif
  }

  if(0 != err)
  {
    LOGE("%s: pthread_cond_wait err(%d)", __func__, err);
  }

  pthread_mutex_unlock(&rdiag_mutex);
  return 0;
}
