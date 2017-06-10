/*============================================================================
@file
sns_diagproxy.c

@brief
Contains main implementation of receiving and processing
Diag commands on Apps processor.

Copyright (c) 2010-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

/*=====================================================================
  INCLUDE FILES
  =======================================================================*/
#include "sensor1.h"

/* Header files to include when not unit testing */
#include "msg.h"

#include "diagpkt.h"
#include "diagcmd.h"
#include "log.h"
#include "log_codes.h"
#include "qmi_idl_lib.h"
#include "sns_common.h"

#ifdef SNS_BLAST
# include "sns_debug_str_mdm.h"
#else
# include "sns_debug_str.h"
#endif

#include "sns_diagproxy.h"
#include "sns_debug_api.h"
#include "sns_log_api.h"
#include "sns_memmgr.h" /* For malloc calls */
#include "sns_smr_util.h"

#ifdef SNS_LA
# include <unistd.h>
# undef LOG_TAG
# include "diag_lsm.h"
# undef LOG_TAG
# define LOG_TAG "Sensors"
#endif

#include <stddef.h>


/*=======================================================================
  DEFINES
  ========================================================================*/

#define SNS_DIAG_MODEM_CLIENT            0x0001
#define SNS_DIAG_APPS_CLIENT             0x0002
#define SNS_DIAG_RESET_CLIENT            0x4003

/* Max Diag clients for Client Manager */
#define SNS_DIAG_MAX_CLIENTS 20

/* Maximum number of transactions possible (for all the clients through DIAG commands) */
#define SNS_DIAG_MAX_TXNS 128

/* Maximum size of a response message from the sensor1 API */
#define SNS_DIAG_RESP_MSG_MAX_LEN 2000

/* Header and body sizes (in Sensor Payload) */
#define SNS_DIAG_APPS_QMI_MSG_HDR_SIZE (sizeof(sns_diag_qmi_msg_header_s))

#define SNS_DIAG_APPS_SENSOR1_MSG_OPEN_HDR_SIZE \
  (sizeof(sns_diag_cm_header_type))
#define SNS_DIAG_APPS_SENSOR1_MSG_OPEN_BODY_SIZE 0

#define SNS_DIAG_APPS_SENSOR1_MSG_WRITE_HDR_SIZE \
  (sizeof(sns_diag_cm_header_type))
#define SNS_DIAG_APPS_SENSOR1_MSG_WRITE_MIN_BODY_SIZE \
  (sizeof(sns_diag_cm_write_msgbody_s) - 1)

#define SNS_DIAG_APPS_SENSOR1_MSG_CLOSE_HDR_SIZE \
  (sizeof(sns_diag_cm_header_type))
#define SNS_DIAG_APPS_SENSOR1_MSG_CLOSE_BODY_SIZE \
  (sizeof(sns_diag_cm_close_msgbody_s))

#define SNS_DIAG_APPS_SENSOR1_RESP_MSG_SIZE \
  (sizeof(sns_diag_sensor1_immediate_response_msg_s))

#define SNS_DIAG_APPS_SMR_RESP_MSG_SIZE \
  (sizeof(sns_diag_smr_immediate_response_msg_s))

#if defined( SNS_BLAST )
# define SNS_DBG_MOD_DIAG    SNS_DBG_MOD_MDM_DIAG
#else
# define SNS_DBG_MOD_DIAG    SNS_DBG_MOD_APPS_DIAG
#endif

/*===========================================================================
  STRUCTURE DEFINES
  ============================================================================*/
/* Struct storing delayed rsp ID for a request */
typedef struct
{
  uint8_t is_valid;
  uint16_t delay_rsp_id;
} delay_rsp_id_association_s;

/*===========================================================================
  LOCAL FUNCTION PROTOTYPES
  ============================================================================*/
/* Rd callback function for sensor1 API */
void sns_diag_clientmgr_notify_data_cb_apps(intptr_t cbdata,
                                            sensor1_msg_header_s *msg_hdr_ptr,
                                            sensor1_msg_type_e msg_type,
                                            void *msg_ptr
                                            );

/* Function on apps processor registered with DIAG for handling commands */
PACK(void *)sns_diag_request_handler_apps(PACK(void *)req_pkt_ptr,
                                          uint16_t pkt_len);

/* Function handling the Packet type 1 (intended for SMR) DIAG requests */
void* sns_diag_qmi_msg_handler_apps(byte *req_pkt_ptr,
                                    uint16_t pkt_len);

/* Function constructing immediate response for "Packet Type 1" requests */
sns_err_code_e sns_diag_fill_smr_immediate_resp(int32_t smr_ret_value,
                                                uint8_t rsp_cnt);

/* Function handling the Packet type 2 (intended for sensor1 API) DIAG requests */
void* sns_diag_sensor1_msg_handler_apps(byte *req_pkt_ptr,
                                        uint16_t pkt_len);

/* Function constructing immediate response for "Packet type 2" requests */
sns_err_code_e sns_diag_fill_sensor1_immediate_resp(int32_t sensor1_api_ret_value,
                                                    intptr_t cb_data,
                                                    sensor1_handle_s* context,
                                                    uint16_t delayed_rsp_id,
                                                    uint8_t txn_id,
                                                    uint8_t rsp_cnt);

/* Helper functions */
uint32_t sns_diag_get_client_id( void );
void sns_diag_free_client_id(uint32_t id);
sns_err_code_e sns_diag_assign_txn_id(uint8_t *txn_id);
void sns_diag_clear_txn_id(uint8_t txn_id);
void sns_diag_assign_delayed_rsp_id(uint8_t txn_id,
                                    sns_diag_sensor1_immediate_response_msg_s* rsp);
sns_err_code_e sns_diag_get_delay_rsp_id(uint8_t txn_id,uint16_t* delay_rsp_id);

/* Function which tests Sensor Diag Handler functionality */
void sns_diag_self_test( void );

/*===========================================================================
  GLOBAL VARIABLES
  ============================================================================*/
/* Table of handlers to register with DIAG */
static const diagpkt_user_table_entry_type sns_diag_tbl[] =
  {
    /* susbsys_cmd_code lo , susbsys_cmd_code hi , call back function */
    #if defined( SNS_BLAST )
    {SNS_DIAG_MODEM_CLIENT, SNS_DIAG_MODEM_CLIENT, sns_diag_request_handler_apps},
    #else
    {SNS_DIAG_APPS_CLIENT, SNS_DIAG_APPS_CLIENT, sns_diag_request_handler_apps},
    #endif
  };

/* Array used for associating cb_data (a.k.a client_id) with context
 * handle from sensor1_open Client ID's start from 1 */
sensor1_handle_s *client_association_arr[SNS_DIAG_MAX_CLIENTS+1];

/* Array used for associating txn_id with delayed rsp Id from DIAG */
delay_rsp_id_association_s delay_rsp_id_association_arr[SNS_DIAG_MAX_TXNS];

/*===========================================================================
  FUNCTIONS
  ============================================================================*/
/*===========================================================================

  FUNCTION:   sns_diag_init

  ===========================================================================*/
/*!
  @brief
  This function performs the initialization with DIAG.

  @param[i]
  No input parameters.

  @return
  sns_err_code_e: SNS_SUCCESS if the initialization had no errors.
  All other values indicate an error has occurred.
*/
/*=========================================================================*/
sns_err_code_e sns_diag_init(void)
{
  /* Calling LSM init - Has to be done on Apps processor
   * Step not required on modem processor
   */
#ifdef SNS_LA
  boolean sns_diag_init_b = FALSE;
  SNS_PRINTF_STRING_LOW_0(SNS_DBG_MOD_DIAG,"In sns_diag_init function");
  /* Required only for Apps processor */
  sns_diag_init_b = Diag_LSM_Init(NULL);
  if (!sns_diag_init_b)
  {
    SNS_PRINTF_STRING_FATAL_0(SNS_DBG_MOD_DIAG,
                              "sns_diag_init: Diag_LSM_Init() failed.");
    return SNS_ERR_FAILED;
  }

#endif
  /* Registering diag packet for delayed responses with sensors subsystem id.
   * To execute on QXDM :: "send_data 128 64 2 0 <Sensors Payload>"
   */
  DIAGPKT_DISPATCH_TABLE_REGISTER_V2_DELAY(DIAG_SUBSYS_CMD_VER_2_F,
                                           DIAG_SUBSYS_SENSORS,
                                           sns_diag_tbl);

  return SNS_SUCCESS;
} // end of fn sns_diag_init


/*===========================================================================

  FUNCTION:   sns_diag_deinit

  ===========================================================================*/
/*!
  @brief
  This function deinitializes with DIAG.

  @param[i]
  No input parameters.

  @return
  No return value.
*/
/*=========================================================================*/
void sns_diag_deinit(void)
{
#ifdef SNS_LA
  SNS_PRINTF_STRING_LOW_0(SNS_DBG_MOD_DIAG,"In sns_diag_deinit function");

  /* Call the DeInit function.
   * Clean up before exiting
   */
  Diag_LSM_DeInit();
#endif
}


/*===========================================================================

  FUNCTION:   sns_diag_request_handler_apps

  ===========================================================================*/
/*!
  @brief
  Handles a diag request packet.

  @detail
  The diag packet sent will have the following
  format.

  function  ||                     header                     ||   byte1          ||    byte 2..n ||
  _________ ||________________________________________________||__________________||______________||
  send_data || Command_type| Subsystem| SNS Subsystem| Unusued||   Packet Type    ||  Sensors Hdr ||
            ||      128    |    64    |    2 = APPS  |   0    || 1 = Msg thru SMR ||              ||
            ||                                                || 2 = Msg thru     ||              ||
            ||                                                ||     sensor1      ||              ||

  ||     byte n..     ||
  ||__________________||
  ||    Sensors       ||
  ||    Payload       ||
  ||                  ||
  ||                  ||

  @param[i] req_pkt_ptr     : A pointer to the diag command packet
  @param[i] pkt_len         : Length of the command packet

  @return
  Immediate response if any. In this implementation NULL Pointer
  is returned as we are committing our response to DIAG within the
  handler function.
*/
/*=========================================================================*/
PACK(void *)sns_diag_request_handler_apps(PACK(void *)req_pkt_ptr,
                                          uint16_t pkt_len)
{
  sns_diag_handle_req_s  *sns_diag_req_ptr = (sns_diag_handle_req_s *)req_pkt_ptr;
  void *sns_diag_resp_ptr;
  uint16_t payload_size = 0;

  SNS_PRINTF_STRING_LOW_1(SNS_DBG_MOD_DIAG,
                             "SNS DIAG: Rcd diag request, pkt_len = %d",
                             pkt_len);
  if ( (sns_diag_req_ptr == NULL) ||
       (pkt_len <= (sizeof(sns_diag_req_ptr->header) +
                    sizeof(sns_diag_req_ptr->pkt_type)) )
       )
  {
    SNS_PRINTF_STRING_ERROR_1(SNS_DBG_MOD_DIAG,
                              "Diag Request Invalid!Pkt Length = %d",
                              pkt_len);
    return NULL;
  }

  /* Payload size to send to handler =
   * Payload size - Diag header field length - Pkt Type field length */
  payload_size = ( pkt_len -
                   (sizeof(sns_diag_req_ptr->header)) -
                   sizeof(sns_diag_req_ptr->pkt_type) );

  switch( sns_diag_req_ptr->pkt_type )
  {
    /* Packet Type 1 Diag Request received */
    case SNS_DIAG_QMI_MSG:
      SNS_PRINTF_STRING_MEDIUM_0(SNS_DBG_MOD_DIAG,
                                 "Received Packet Type 1 (QMI Message)");
      sns_diag_resp_ptr =
        sns_diag_qmi_msg_handler_apps(sns_diag_req_ptr->payload,
                                      payload_size);
      break;

      /* Packet Type 1 Diag Request received */
    case SNS_DIAG_SENSOR1_MSG:
      SNS_PRINTF_STRING_MEDIUM_0(SNS_DBG_MOD_DIAG,
                                 "Received Packet Type 2 (Client Manager)");
      SNS_PRINTF_STRING_FATAL_1(SNS_DBG_MOD_DIAG,
                                "SNS DIAG: Payload Size passed to ACM %d",payload_size);
      sns_diag_resp_ptr =
        sns_diag_sensor1_msg_handler_apps(sns_diag_req_ptr->payload,
                                          payload_size);
      break;

      /* Self Test option (Used for testing) */
    case SNS_DIAG_SELF_TEST_MSG:
      SNS_PRINTF_STRING_MEDIUM_0(SNS_DBG_MOD_DIAG,
                                 "Received Packet Type 10 (Test logging)");
      sns_diag_self_test();

      /* Dont know what the Sensors Payload is! */
    default:
      SNS_PRINTF_STRING_MEDIUM_0(SNS_DBG_MOD_DIAG,
                                 "Received unknown Packet Type");
      sns_diag_resp_ptr = NULL;
  } // end of switch

  return sns_diag_resp_ptr;
} // end of funtion sns_diag_request_handler_apps

/*===========================================================================

  FUNCTION:   sns_diag_qmi_msg_handler_apps

  ===========================================================================*/
/*!
  @brief
  This function is called to handle "Packet Type 1" DIAG commands.
  The QMI messages contained in the DIAG commands are routed through the SMR.

  @param[i] req_pkt_ptr     : A pointer to the diag command packet
  @param[i] pkt_len         : Length of the command packet

  @return
  Immediate response (sns_diag_qmi_response_msg_s) if any.
  In this implementation NULL Pointer is returned as we are committing
  our response to DIAG within the
  handler function.
*/
/*=========================================================================*/
void* sns_diag_qmi_msg_handler_apps(byte *req_pkt_ptr,
                                    uint16_t pkt_len)
{
  UNREFERENCED_PARAMETER(req_pkt_ptr);
  UNREFERENCED_PARAMETER(pkt_len);
  // Type 1 DIAG commands are not supported in non-SMR builds
  return NULL;
} // end of fn sns_diag_qmi_msg_handler_apps


/*===========================================================================

  FUNCTION:   sns_diag_fill_smr_immediate_resp

  ===========================================================================*/
/*!
  @brief
  Allocates memory through DIAG api, fills up the immediate response and
  commits response.

  @param[i]smr_ret_value  : Return value from SMR function call when a
  Packet Type 1 Request is received.
  In case there were errors with the diag request
  before calling the smr API
  this field contains the corresponding smr
  error code.
  @param[i]rsp_cnt        : Number of delayed responses to follow. If its 1
  it means the immediate response is the final
  response

  @return
  Error code for the operation.
*/
/*=========================================================================*/
sns_err_code_e sns_diag_fill_smr_immediate_resp(int32_t smr_ret_value,
                                                uint8_t rsp_cnt)
{
  sns_diag_smr_immediate_response_msg_s *sns_diag_smr_response_ptr = NULL;

  /* Allocate memory */
  sns_diag_smr_response_ptr = (sns_diag_smr_immediate_response_msg_s *)
    diagpkt_subsys_alloc_v2(DIAG_SUBSYS_SENSORS,
                            SNS_DIAG_APPS_CLIENT,
                            SNS_DIAG_APPS_SMR_RESP_MSG_SIZE);
  if (sns_diag_smr_response_ptr == NULL)
  {
    SNS_PRINTF_STRING_ERROR_0(SNS_DBG_MOD_DIAG,
                              "Malloc failure: Could not allocate memory for "
                              "SMR Immediate Response");
    return SNS_ERR_NOMEM;
  }

  /* Fill out the immediate response */
  sns_diag_smr_response_ptr->smr_ret_val = smr_ret_value;

  /* If the immediate response is the final response set Response Cnt
     that in the diag header */
  if (rsp_cnt == 1)
  {
    diagpkt_subsys_set_rsp_cnt(sns_diag_smr_response_ptr,1);
  }

  /* Commit the response packet */
  (void)diagpkt_commit(sns_diag_smr_response_ptr);

  return SNS_SUCCESS;
} //end of fn sns_diag_fill_smr_immediate_resp


/*===========================================================================

  FUNCTION:   sns_diag_sensor1_msg_handler_apps

  ===========================================================================*/
/*!
  @brief
  This function is called to handle "Packet Type 2" DIAG commands.
  The QMI messages contained in the DIAG commands are sent to the
  Sensor1 API.

  @param[i] req_pkt_ptr     : A pointer to the diag command packet
  @param[i] pkt_len         : Length of the command packet

  @return
  Immediate response (sns_diag_sensor1_immediate_response_msg_s) if any.
  In this implementation NULL Pointer is returned as we are committing
  our response to DIAG within the
  handler function.
*/
/*=========================================================================*/
void* sns_diag_sensor1_msg_handler_apps(byte *req_pkt_ptr,
                                        uint16_t pkt_len)
{
  sns_diag_cm_msg_body_s *parsed_msg_ptr;
  /* TODO Remove later once tool is in place for filling up QMI messages */
  void* qmi_msg_ptr = NULL;
  sns_diag_sensor1_immediate_response_msg_s *sns_diag_cm_response_ptr = NULL;
  sensor1_handle_s* context_hdl;
  sensor1_msg_header_s msg_hdr;
  sensor1_error_e err_code;
  uint32_t client_id = 0;
  int32_t qmi_decode_ret_code;
  uint32_t qmi_msg_size;

  /* Input Checks */
  if ( (pkt_len == 0) || (req_pkt_ptr == NULL) )
  {
    SNS_PRINTF_STRING_ERROR_0(SNS_DBG_MOD_DIAG,
                              "SNS Packet type 2 Request Invalid (size 0 "
                              "or NULL ptr)!");
    sns_diag_fill_sensor1_immediate_resp(SENSOR1_EBAD_PARAM,0,NULL,0,0,1);
    return NULL;
  }

  parsed_msg_ptr = (sns_diag_cm_msg_body_s*) req_pkt_ptr;
  SNS_PRINTF_STRING_MEDIUM_1(SNS_DBG_MOD_DIAG,
                             "SNS Packet Type 2: cm_fn = %d",
                             parsed_msg_ptr->cm_fn);

  switch(parsed_msg_ptr->cm_fn)
  {
    case SNS_DIAG_SENSOR1_OPEN: /* Sensor1_Open function */
      SNS_PRINTF_STRING_MEDIUM_1(SNS_DBG_MOD_DIAG,
                                 "SENSOR1_Open command is rcvd,Pkt Length = %d",
                                 pkt_len);

      if ( (pkt_len < (SNS_DIAG_APPS_SENSOR1_MSG_OPEN_BODY_SIZE +
                       SNS_DIAG_APPS_SENSOR1_MSG_OPEN_HDR_SIZE)) )
      {
        SNS_PRINTF_STRING_ERROR_1(SNS_DBG_MOD_DIAG,
                                  "SENSOR1_Open Request Invalid due to pkt "
                                  "size (%d) or cb_data!",
                                  pkt_len);
        sns_diag_fill_sensor1_immediate_resp(SENSOR1_EBAD_PARAM,0,NULL,0,0,1);
        return NULL;
      }

      /* Assign a cb_data value i.e client id
       * Client ID 0 means error
       */
      client_id = sns_diag_get_client_id();
      if (client_id == 0)
      {
        SNS_PRINTF_STRING_ERROR_0(SNS_DBG_MOD_DIAG,
                                  "Packet type 2 client id could not be "
                                  "assigned");
        sns_diag_fill_sensor1_immediate_resp(SENSOR1_EFAILED,0,NULL,0,0,1);
        return NULL;
      }

      SNS_PRINTF_STRING_MEDIUM_1(SNS_DBG_MOD_DIAG,
                                 "SENSOR1_Write command, Client ID = %d",
                                 client_id);
      err_code = sensor1_open(&client_association_arr[client_id],
                              sns_diag_clientmgr_notify_data_cb_apps,
                              client_id);

      SNS_PRINTF_STRING_ERROR_1(SNS_DBG_MOD_DIAG,
                                "Packet type 2 sensor1_open error = %d",
                                err_code);
      sns_diag_fill_sensor1_immediate_resp(err_code,
                                           client_id,
                                           client_association_arr[client_id],
                                           0,
                                           0,
                                           1);
      /* Free client ID if Open had an error */
      if (err_code != SENSOR1_SUCCESS)
      {
        sns_diag_free_client_id(client_id);
      }
      break;
    case SNS_DIAG_SENSOR1_WRITE:  /* Sensor1_Write function */
      SNS_PRINTF_STRING_MEDIUM_1(SNS_DBG_MOD_DIAG,
                                 "SENSOR1_Write command rcvd, Pkt length = %d",
                                 pkt_len);

      /* Remove after testing */
      SNS_PRINTF_STRING_MEDIUM_2(SNS_DBG_MOD_DIAG,
                                 "SENSOR1_Write min body size=%d, "
                                 "msg_hdr_size=%d",
                                 SNS_DIAG_APPS_SENSOR1_MSG_WRITE_MIN_BODY_SIZE,
                                 SNS_DIAG_APPS_SENSOR1_MSG_WRITE_HDR_SIZE);

      SNS_PRINTF_STRING_MEDIUM_3(SNS_DBG_MOD_DIAG,
                                 "SENSOR1_Write cbdata=%d, msg_size=%d,"
                                 "context=%u",
                                 parsed_msg_ptr->msgbody.write.cbdata,
                                 parsed_msg_ptr->msgbody.write.msg_size,
                                 (uint32_t)(uintptr_t)client_association_arr[parsed_msg_ptr->msgbody.write.cbdata]);

      if ( (pkt_len < (SNS_DIAG_APPS_SENSOR1_MSG_WRITE_MIN_BODY_SIZE +
                       SNS_DIAG_APPS_SENSOR1_MSG_WRITE_HDR_SIZE)) ||
           (parsed_msg_ptr->msgbody.write.cbdata > SNS_DIAG_MAX_CLIENTS) ||
           (client_association_arr[parsed_msg_ptr->msgbody.write.cbdata] ==
            NULL) )
      {
        SNS_PRINTF_STRING_ERROR_1(SNS_DBG_MOD_DIAG,
                                  "Sensor1_write Request Invalid! Pkt len = %d",
                                  pkt_len);
        sns_diag_fill_sensor1_immediate_resp(SENSOR1_EBAD_PARAM,0,NULL,0,0,1);
        return NULL;
      }

      /* Obtain context handle from cb_data value (input) */
      context_hdl =
        client_association_arr[parsed_msg_ptr->msgbody.write.cbdata];

      qmi_decode_ret_code = qmi_idl_get_message_c_struct_len(
            sns_smr_get_svc_obj( parsed_msg_ptr->msgbody.write.srvc_num ),
            QMI_IDL_REQUEST, parsed_msg_ptr->msgbody.write.msg_id, &qmi_msg_size );
      if( QMI_IDL_LIB_NO_ERR != qmi_decode_ret_code )
      {
        SNS_PRINTF_STRING_ERROR_1( SNS_DBG_MOD_DIAG,
                                   "sensor1_write: qmi_idl_get_message_c_struct_len failed %i",
                                   qmi_decode_ret_code );
        sns_diag_fill_sensor1_immediate_resp(SENSOR1_EFAILED,0,NULL,0,0,1);
        return NULL;
      }

      /* Copy Message */
      if (parsed_msg_ptr->msgbody.write.msg_size != 0)
      {
        err_code = sensor1_alloc_msg_buf(context_hdl,
                                         qmi_msg_size,
                                         &qmi_msg_ptr);
      }
      else
      {
        err_code = sensor1_alloc_msg_buf(context_hdl,
                                         0,
                                         &qmi_msg_ptr);
      }

      if ( (err_code != SENSOR1_SUCCESS) || (qmi_msg_ptr == NULL) )
      {
        SNS_PRINTF_STRING_ERROR_0(SNS_DBG_MOD_DIAG,
                                  "sensor1_write: sensor1_alloc request failed");
        sns_diag_fill_sensor1_immediate_resp(SENSOR1_EFAILED,0,NULL,0,0,1);
        return NULL;
      }

      msg_hdr.service_number = parsed_msg_ptr->msgbody.write.srvc_num;
      msg_hdr.msg_id = parsed_msg_ptr->msgbody.write.msg_id;
      msg_hdr.msg_size = parsed_msg_ptr->msgbody.write.msg_size;
      SNS_PRINTF_STRING_MEDIUM_3(SNS_DBG_MOD_DIAG,
                                 "SENSOR1_Write command, Service num = %d,"
                                 "Msg ID = %d,Msg Size = %d",
                                 msg_hdr.service_number,
                                 msg_hdr.msg_id,
                                 msg_hdr.msg_size);

      /* Decode QMI Message only if QMI encoded body size is not zero */
      if (parsed_msg_ptr->msgbody.write.msg_size != 0)
      {
        SNS_PRINTF_STRING_MEDIUM_0(SNS_DBG_MOD_DIAG,
                                   "sensor1_write: Attempting to decode QMI "
                                   "Message");
        qmi_decode_ret_code =
          qmi_idl_message_decode(sns_smr_get_svc_obj(msg_hdr.service_number),
                                 QMI_IDL_REQUEST,
                                 msg_hdr.msg_id,
                                 (const void*)parsed_msg_ptr->msgbody.write.qmi_msg,
                                 parsed_msg_ptr->msgbody.write.msg_size,
                                 qmi_msg_ptr,
                                 qmi_msg_size);
        if (QMI_IDL_LIB_NO_ERR != qmi_decode_ret_code)
        {
          SNS_PRINTF_STRING_ERROR_1(SNS_DBG_MOD_DIAG,
                                    "sensor1_write: QMI Decode failed=%d",
                                    qmi_decode_ret_code);
          sns_diag_fill_sensor1_immediate_resp(SENSOR1_EFAILED,0,NULL,0,0,1);
          /* Free memory allocated using sensor1_alloc_msg_buf */
          sensor1_free_msg_buf(context_hdl,qmi_msg_ptr);
          return NULL;
        }
        msg_hdr.msg_size = qmi_msg_size;
      }


      /* Assign Txn ID to the incoming request */
      if (sns_diag_assign_txn_id(&msg_hdr.txn_id) != SNS_SUCCESS)
      {
        SNS_PRINTF_STRING_FATAL_0(SNS_DBG_MOD_DIAG,
                                  "SENSOR1_Write Could not assign TXN ID");
        sns_diag_fill_sensor1_immediate_resp(SENSOR1_EFAILED,0,NULL,0,0,1);
        sensor1_free_msg_buf(context_hdl,qmi_msg_ptr);
        return NULL;
      }
      SNS_PRINTF_STRING_MEDIUM_1(SNS_DBG_MOD_DIAG,
                                 "SENSOR1_Write command, Txn ID = %d",
                                 msg_hdr.txn_id);

      err_code = sensor1_write(context_hdl,
                               &msg_hdr,
                               (void*) qmi_msg_ptr);

      if (err_code != SENSOR1_SUCCESS)
      {
        SNS_PRINTF_STRING_ERROR_1(SNS_DBG_MOD_DIAG,
                                  "Packet type 2 sensor1_write error = %d",
                                  err_code);
        sns_diag_fill_sensor1_immediate_resp(err_code,0,NULL,0,0,1);

        /* Free memory allocated using sensor1_alloc_msg_buf */
        sensor1_free_msg_buf(context_hdl,qmi_msg_ptr);

        /* Free the assigned transaction ID */
        sns_diag_clear_txn_id(msg_hdr.txn_id);

        return NULL;
      }

      /* Fill response. Function to fill response not called because we cant
       * create the delayed rsp id unless the alloc with DIAG succeeds */
      sns_diag_cm_response_ptr = (sns_diag_sensor1_immediate_response_msg_s *)
        diagpkt_subsys_alloc_v2(DIAG_SUBSYS_SENSORS,
                                SNS_DIAG_APPS_CLIENT,
                                SNS_DIAG_APPS_SENSOR1_RESP_MSG_SIZE);
      if (sns_diag_cm_response_ptr == NULL)
      {
        SNS_PRINTF_STRING_ERROR_0(SNS_DBG_MOD_DIAG,
                                  "Malloc failure: Could not allocate memory "
                                  "for CM diag response")
          /* Free the assigned transaction ID */
          sns_diag_clear_txn_id(msg_hdr.txn_id);
        return NULL;
      }

      /* Assign the delayed response ID */
      sns_diag_assign_delayed_rsp_id(msg_hdr.txn_id,sns_diag_cm_response_ptr);

      sns_diag_cm_response_ptr->sensor1_api_ret_value =
        err_code;
      sns_diag_cm_response_ptr->cb_data               =
        parsed_msg_ptr->msgbody.write.cbdata;
      sns_diag_cm_response_ptr->context               =
        (uint32_t)(uintptr_t)client_association_arr[parsed_msg_ptr->msgbody.write.cbdata];
      sns_diag_cm_response_ptr->delayed_rsp_id        =
        sns_diag_cm_response_ptr->diag_hdr.delayed_rsp_id;
      sns_diag_cm_response_ptr->txn_id                =
        msg_hdr.txn_id;

      /* Commit the response packet */
      (void)diagpkt_commit(sns_diag_cm_response_ptr);
      SNS_PRINTF_STRING_MEDIUM_0(SNS_DBG_MOD_DIAG,
                                 "SENSOR1_Write command, Immediate rsp commited");

      break;
    case SNS_DIAG_SENSOR1_CLOSE:  /* Sensor1_Close function */
      SNS_PRINTF_STRING_MEDIUM_1(SNS_DBG_MOD_DIAG,
                                 "SENSOR1_close command rcvd, Pkt Length = %d",
                                 pkt_len);

      if ( (pkt_len < (SNS_DIAG_APPS_SENSOR1_MSG_CLOSE_BODY_SIZE +
                       SNS_DIAG_APPS_SENSOR1_MSG_CLOSE_HDR_SIZE)) ||
           (parsed_msg_ptr->msgbody.close.cbdata > SNS_DIAG_MAX_CLIENTS) ||
           (client_association_arr[parsed_msg_ptr->msgbody.close.cbdata] == NULL) )
      {
        SNS_PRINTF_STRING_ERROR_2(SNS_DBG_MOD_DIAG,
                                  "Sensor1_Close Request Invalid!,pkt len =%d,"
                                  "cb_data=%d",
                                  pkt_len,
                                  parsed_msg_ptr->msgbody.close.cbdata);
        sns_diag_fill_sensor1_immediate_resp(SENSOR1_EBAD_PARAM,0,NULL,0,0,1);
        return NULL;
      }
      SNS_PRINTF_STRING_MEDIUM_2(SNS_DBG_MOD_DIAG,
                                 "SENSOR1_close - client=%d,context=%u",
                                 parsed_msg_ptr->msgbody.close.cbdata,
                                 (uint32_t)(uintptr_t)client_association_arr[parsed_msg_ptr->msgbody.close.cbdata]);

      err_code =
        sensor1_close(client_association_arr[parsed_msg_ptr->msgbody.close.cbdata]);
      SNS_PRINTF_STRING_ERROR_1(SNS_DBG_MOD_DIAG,
                                "Packet type 2 sensor1_close error = %d",
                                err_code);

      sns_diag_fill_sensor1_immediate_resp(err_code,
                                           parsed_msg_ptr->msgbody.close.cbdata,
                                           client_association_arr[parsed_msg_ptr->msgbody.close.cbdata],
                                           0,
                                           0,
                                           1);

      /* Remove association of cb_data with context handle */
      sns_diag_free_client_id(parsed_msg_ptr->msgbody.close.cbdata);
      break;
    default:
      SNS_PRINTF_STRING_ERROR_1(SNS_DBG_MOD_DIAG,
                                "Packet type 2 Unknown Sensor1 function = %d",
                                parsed_msg_ptr->cm_fn);
  }; // end of switch

  return NULL;
} // end of function sns_diag_sensor1_msg_handler_apps


/*===========================================================================

  FUNCTION:   sns_diag_fill_sensor1_immediate_resp

  ===========================================================================*/
/*!
  @brief
  Allocates memory through DIAG api, fills up the immediate response and
  commits response.

  @param[i] sensor1_api_ret_value   : Return value from sensor1 API funciton call if
                                      sensor1 API was called.
                                      In case there were errors with the diag request
                                      before calling the sensor1 API
                                      this field contains the corresponding sensor1
                                      error code.
  @param[i]            cb_data      : Contains the Client ID assigned by the Sensors
                                      Diag Handler Module
  @param[i]            context      : Context handle returned from sensor1 API
  @param[i]         delayed_rsp_id  : Delayed response identifier. This is assigned
                                      by DIAG and is used to match delayed responses
  @param[i]             txn id      : Transaction ID assigned by the Sensors
                                      Diag Handler Module
  @param[i]             rsp_cnt     : Number of delayed responses to follow. If its 1
                                      it means the immediate response is the final
                                      response

  @return
  Error code for the operation.
*/
/*=========================================================================*/
sns_err_code_e sns_diag_fill_sensor1_immediate_resp(int32_t sensor1_api_ret_value,
                                                    intptr_t cb_data,
                                                    sensor1_handle_s* context,
                                                    uint16_t delayed_rsp_id,
                                                    uint8_t txn_id,
                                                    uint8_t rsp_cnt)
{
  sns_diag_sensor1_immediate_response_msg_s *sns_diag_cm_response_ptr = NULL;

  /* Allocate memory */
  sns_diag_cm_response_ptr = (sns_diag_sensor1_immediate_response_msg_s *)
    diagpkt_subsys_alloc_v2(DIAG_SUBSYS_SENSORS,
                            SNS_DIAG_APPS_CLIENT,
                            SNS_DIAG_APPS_SENSOR1_RESP_MSG_SIZE);
  if (sns_diag_cm_response_ptr == NULL)
  {
    SNS_PRINTF_STRING_ERROR_0(SNS_DBG_MOD_DIAG,
                              "Malloc failure: Could not allocate memory for "
                              "Sensor1 Immediate Response");
    return SNS_ERR_NOMEM;
  }

  /* Fill out the immediate response */
  sns_diag_cm_response_ptr->sensor1_api_ret_value = sensor1_api_ret_value;
  sns_diag_cm_response_ptr->cb_data               = cb_data;
  // TODO: The following doesn't handle aliasing of 64-bit pointers into a
  // 32-bit space
  sns_diag_cm_response_ptr->context               = (uint32_t)(uintptr_t)context;
  sns_diag_cm_response_ptr->delayed_rsp_id        = delayed_rsp_id;
  sns_diag_cm_response_ptr->txn_id                = txn_id;

  /* If the immediate response is the final response set Response Cnt that
     in the diag header */
  if (rsp_cnt == 1)
  {
    diagpkt_subsys_set_rsp_cnt(sns_diag_cm_response_ptr,1);
  }

  /* Commit the response packet */
  (void)diagpkt_commit(sns_diag_cm_response_ptr);

  return SNS_SUCCESS;
} //end of fn sns_diag_fill_sensor1_immediate_resp


/*===========================================================================

  FUNCTION:   sns_diag_clientmgr_notify_data_cb_apps

  ===========================================================================*/
/*!
  @brief
  This function is registered as the read callback function with Client manager
  for all DIAG clients. This function is called when client manager
  receives responses or indications for the requests.

  @param[i] cbdata         : Callback data (contains the Client ID for DIAG clients
  @param[i] msg_hdr_ptr    : Pointer to message header
  @param[i] msg_type       : Message type
  @param[i] msg_ptr        : Pointer to the response/indication

  @return
  No return value.
*/
/*=========================================================================*/
void sns_diag_clientmgr_notify_data_cb_apps(intptr_t cbdataIn,
                                            sensor1_msg_header_s *msg_hdr_ptr,
                                            sensor1_msg_type_e msg_type,
                                            void *msg_ptr
                                            )
{

  sensor1_handle_s *context_hdl;
  sns_diag_sensor1_delayed_rsp_msg_s *sensor1_delay_rsp = NULL;
  uint16_t delay_rsp_id;
  int32_t qmi_encode_ret_code;
  uint32_t encoded_length = 0;
  // We are only going to encode a response...dont need more than 200 bytes
  uint8_t encoded_arr[SNS_DIAG_RESP_MSG_MAX_LEN];
  uint32_t cbdata = (uint32_t)cbdataIn;

  SNS_PRINTF_STRING_MEDIUM_0(SNS_DBG_MOD_DIAG,
                             "Sensor1 Read Callback function called");

  /* Checks for valid input */
  if ( (msg_hdr_ptr == NULL) ||
       (cbdata > SNS_DIAG_MAX_CLIENTS) )
  {
    SNS_PRINTF_STRING_ERROR_1(SNS_DBG_MOD_DIAG,
                              "Sensor1 RdCallback: Input parameters error, "
                              "cbdata= %d",
                              cbdata);
    return;
  }

  /* Get the context handle */
  context_hdl = client_association_arr[cbdata];
  SNS_PRINTF_STRING_MEDIUM_1(SNS_DBG_MOD_DIAG,
                             "Sensor1 RdCallback: Context handle %u",
                             (uintptr_t)context_hdl);

  /* Check response type */
  if ( (msg_type != SENSOR1_MSG_TYPE_RESP) &&
       (msg_type != SENSOR1_MSG_TYPE_RESP_INT_ERR) )
  {
    SNS_PRINTF_STRING_MEDIUM_1(SNS_DBG_MOD_DIAG,
                               "Sensor1 RdCallback: Message type not "
                               "response but %d",
                               msg_type);
    SNS_PRINTF_STRING_MEDIUM_2(SNS_DBG_MOD_DIAG,
                               "Sensor1 RdCallback: Srvc ID=%d, Msg ID=%d",
                               msg_hdr_ptr->service_number,msg_hdr_ptr->msg_id);
    /* Just free the msg pointer
     * Nothing else to do
     */
    sensor1_free_msg_buf(context_hdl,msg_ptr);
    return;
  }

  /* Get the delayed response ID for filling up the delayed response */
  if (sns_diag_get_delay_rsp_id(msg_hdr_ptr->txn_id,&delay_rsp_id) !=
      SNS_SUCCESS)
  {
    /* Error could not get delayed rsp_id */
    SNS_PRINTF_STRING_FATAL_0(SNS_DBG_MOD_DIAG,
                              "Sensor1 RdCallback: Could not get delayed rsp_id");
    sns_diag_clear_txn_id(msg_hdr_ptr->txn_id);
    sensor1_free_msg_buf(context_hdl,msg_ptr);
    return;
  }

  SNS_PRINTF_STRING_HIGH_2(SNS_DBG_MOD_DIAG,
                           "Sensor1 RdCallback: Retrieved delay_rsp_id= "
                           "%d,txn_id= %d",
                           delay_rsp_id,msg_hdr_ptr->txn_id);

  /* QMI encode the delayed response only if length is greater than 0 so
   * that we know how much memory to allocate for delayed rsp with DIAG
   */
  if (msg_hdr_ptr->msg_size > 0)
  {
    SNS_PRINTF_STRING_MEDIUM_0(SNS_DBG_MOD_DIAG,
                               "Sensor1 RdCallback: Attempting to encode "
                               "response");
    qmi_encode_ret_code =
      qmi_idl_message_encode(sns_smr_get_svc_obj(msg_hdr_ptr->service_number),
                             QMI_IDL_RESPONSE,
                             msg_hdr_ptr->msg_id,
                             msg_ptr,
                             msg_hdr_ptr->msg_size,
                             encoded_arr,
                             SNS_DIAG_RESP_MSG_MAX_LEN,
                             &encoded_length);
    if (QMI_IDL_LIB_NO_ERR != qmi_encode_ret_code)
    {
      SNS_PRINTF_STRING_ERROR_1(SNS_DBG_MOD_DIAG,
                                "sensor1_write: QMI Encode failed=%d",
                                qmi_encode_ret_code);
      /* Delayed resp message will have response message without QMI encoding */
      SNS_OS_MEMCOPY(encoded_arr,msg_ptr,msg_hdr_ptr->msg_size);
      encoded_length = msg_hdr_ptr->msg_size;
    }
  } //end of if (msg_hdr_ptr->msg_size > 0)

  /* Get memory for the delayed response */
  /* NOTE:
   * 1) There is a  -1 because the delayed rsp struct has one byte allocated
   * for the message. We need to subtract that.
   */
  sensor1_delay_rsp =
    diagpkt_subsys_alloc_v2_delay(DIAG_SUBSYS_SENSORS,
                                  SNS_DIAG_APPS_CLIENT,
                                  delay_rsp_id,
                                  (sizeof(sns_diag_sensor1_delayed_rsp_msg_s)
                                   -1+encoded_length));
  if (sensor1_delay_rsp == NULL)
  {
    SNS_PRINTF_STRING_FATAL_0(SNS_DBG_MOD_DIAG,
                              "Sensor1 RdCallback: No memory for delayed "
                              "response");
    sns_diag_clear_txn_id(msg_hdr_ptr->txn_id);
    sensor1_free_msg_buf(context_hdl,msg_ptr);
    return;
  }

  /* This is the final delayed response */
  diagpkt_subsys_set_rsp_cnt(sensor1_delay_rsp,1);

  sensor1_delay_rsp->delayed_rsp_id = delay_rsp_id;
  sensor1_delay_rsp->txn_id         = msg_hdr_ptr->txn_id;
  sensor1_delay_rsp->cb_data        = cbdata;
  // TODO: The following doesn't handle aliasing of 64-bit pointers into a
  // 32-bit space
  sensor1_delay_rsp->context        = (uint32_t)(uintptr_t)context_hdl;
  sensor1_delay_rsp->srvc_num       = msg_hdr_ptr->service_number;
  sensor1_delay_rsp->msg_id         = msg_hdr_ptr->msg_id;
  sensor1_delay_rsp->msg_type       = msg_type;
  sensor1_delay_rsp->msg_size       = encoded_length;
  SNS_OS_MEMCOPY(&sensor1_delay_rsp->msg,encoded_arr,encoded_length);

  diagpkt_delay_commit((void*)sensor1_delay_rsp);

  /* Free message buffers */
  sensor1_free_msg_buf(context_hdl,msg_ptr);

  /* Free associated transaction id with delayed response ID */
  sns_diag_clear_txn_id(msg_hdr_ptr->txn_id);

  return;
} // end of function sns_diag_clientmgr_notify_data_cb_apps


/*===========================================================================

  FUNCTION:   sns_diag_get_client_id

  ===========================================================================*/
/*!
  @brief
  Assigns a client ID to the Diag client.

  @param[i] None

  @return
  Integer value representing the Client ID.
  Zero value indicates error
  Positive value indicates client ID.
*/
/*=========================================================================*/
uint32_t sns_diag_get_client_id( void )
{
  uint8_t i;

  /* Check for a free context handle value */
  for(i = 1; i <= SNS_DIAG_MAX_CLIENTS; i++)
  {
    if (client_association_arr[i] == NULL)
    {
      return i;
    }
  } // end of for loop

  return 0;
} // end of function sns_diag_get_client_id


/*===========================================================================

  FUNCTION:   sns_diag_free_client_id

  ===========================================================================*/
/*!
  @brief
  Frees a allocated client Identifier number.

  @param[i] uint32: Client ID

  @return
  No return value
*/
/*=========================================================================*/
void sns_diag_free_client_id(uint32_t id)
{
  client_association_arr[id] = NULL;
  return;
} // end of function sns_diag_free_client_id


/*===========================================================================

  FUNCTION:   sns_diag_assign_txn_id

  ===========================================================================*/
/*!
  @brief
  Assigns a transaction ID.

  @param[i] txn_id: A Pointer to the placeholder for the transaction id value

  @return
  Error Code for the operation.
*/
/*=========================================================================*/
sns_err_code_e sns_diag_assign_txn_id(uint8_t *txn_id)
{
  uint8_t i;
  uint16_t arr_size;

  if (txn_id == NULL)
  {
    return SNS_ERR_NOMEM;
  }
  arr_size = sizeof(delay_rsp_id_association_arr);

  /* Check for a free transaction ID value */
  for(i = 0; i < arr_size; i++)
  {
    if (delay_rsp_id_association_arr[i].is_valid == 0)
    {
      *txn_id = i;
      delay_rsp_id_association_arr[i].is_valid = 1;
      delay_rsp_id_association_arr[i].delay_rsp_id = 0;
      SNS_PRINTF_STRING_HIGH_1(SNS_DBG_MOD_DIAG,
                               "TXN_ID assigned is: %d",
                               i);
      return SNS_SUCCESS;
    }
  } // end of for loop

  /* No transaction ID available */
  return SNS_ERR_FAILED;
} // end of function sns_diag_assign_txn_id


/*===========================================================================

  FUNCTION:   sns_diag_clear_txn_id

  ===========================================================================*/
/*!
  @brief
  Clears a transaction ID.

  @param[i] txn_id: A Pointer to the placeholder for the transaction id value

  @return
  No return value.
*/
/*=========================================================================*/
void sns_diag_clear_txn_id(uint8_t txn_id)
{
  delay_rsp_id_association_arr[txn_id].is_valid = 0;
  delay_rsp_id_association_arr[txn_id].delay_rsp_id = 0;
} //end of fn sns_diag_clear_txn_id


/*===========================================================================

  FUNCTION:   sns_diag_assign_delayed_rsp_id

  ===========================================================================*/
/*!
  @brief
  Clears a transaction ID.

  @param[i] txn_id : Value of the transaction ID
  @param[i] rsp    : Pointer to the immediate response message

  @return
  No return value.
*/
/*=========================================================================*/
void sns_diag_assign_delayed_rsp_id(uint8_t txn_id,sns_diag_sensor1_immediate_response_msg_s* rsp)
{
  SNS_PRINTF_STRING_HIGH_1(SNS_DBG_MOD_DIAG,"TXN_ID: %d",txn_id);
  if (delay_rsp_id_association_arr[txn_id].is_valid != 0)
  {
    delay_rsp_id_association_arr[txn_id].delay_rsp_id =
      diagpkt_subsys_get_delayed_rsp_id(rsp);
    SNS_PRINTF_STRING_HIGH_1(SNS_DBG_MOD_DIAG,
                             "Delayed response id = %d",
                             delay_rsp_id_association_arr[txn_id].delay_rsp_id);
  }
} //end of fn sns_diag_assign_delayed_rsp_id


/*===========================================================================

  FUNCTION:   sns_diag_get_delay_rsp_id

  ===========================================================================*/
/*!
  @brief
  Gets the delayed response ID.

  @param[i] txn_id: Value of the transaction ID

  @return
  sns_err_code_e: Error code for the operation.
*/
/*=========================================================================*/
sns_err_code_e sns_diag_get_delay_rsp_id(uint8_t txn_id,uint16_t* delay_rsp_id)
{
  if ( (delay_rsp_id == NULL) ||
       (txn_id >= SNS_DIAG_MAX_TXNS) )
  {
    SNS_PRINTF_STRING_HIGH_1(SNS_DBG_MOD_DIAG,
                             "sns_diag_get_delay_rsp_id: Input ERROR, txn_id = %d",
                             txn_id);
    return SNS_ERR_FAILED;
  }
  if (delay_rsp_id_association_arr[txn_id].is_valid != 0)
  {
    *delay_rsp_id = delay_rsp_id_association_arr[txn_id].delay_rsp_id;
    return SNS_SUCCESS;
  }

  return SNS_ERR_FAILED;
} //end of fn sns_diag_get_delay_rsp_id


/*===========================================================================

  FUNCTION:   sns_diag_self_test

  ===========================================================================*/
/*!
  @brief
  Does a quick self test (prints a few strings (F3's, a log packet).

  @param[i] None

  @return
  None
*/
/*=========================================================================*/
void sns_diag_self_test( void )
{
  SNS_PRINTF_STRING_FATAL_0(SNS_DBG_MOD_DIAG,
                            "Sensors DIAG Handler Self Test BEGIN");

  /* Print sizes of our PACKED structures. The sizes are very useful when
   * composing DIAG commands. The DIAG commands should be of the correct
   * length else the software will reject them.
   * The following F3's will print sizes of structs representing the DIAG
   * commands
   */
  SNS_PRINTF_STRING_FATAL_1(SNS_DBG_MOD_DIAG,
                            "SNS DIAG: Size of diag header %d",
                            sizeof(sns_diag_handle_req_s));
  SNS_PRINTF_STRING_HIGH_1(SNS_DBG_MOD_DIAG,
                           "SNS DIAG: Size of QMI Msg Hdr %d",
                           sizeof(sns_diag_qmi_msg_header_s));
  SNS_PRINTF_STRING_MEDIUM_1(SNS_DBG_MOD_DIAG,
                             "SNS DIAG: Size of QMI Msg Hdr %d",
                             sizeof(sns_diag_qmi_msg_header_s));
  SNS_PRINTF_STRING_MEDIUM_1(SNS_DBG_MOD_DIAG,
                             "SNS DIAG: Size of CM Write Msg Body %d",
                             sizeof(sns_diag_cm_write_msgbody_s));
  SNS_PRINTF_STRING_MEDIUM_2(SNS_DBG_MOD_DIAG,
                             "SNS DIAG: Size of CM Immediate Rsp %d,Delayed Rsp %d",
                             sizeof(sns_diag_sensor1_immediate_response_msg_s),
                             sizeof(sns_diag_sensor1_delayed_rsp_msg_s));
  SNS_PRINTF_STRING_HIGH_2(SNS_DBG_MOD_DIAG,
                           "SNS DIAG: Size of QMI Rsp Msg %d,Delayed Rsp Diag Hdr %d",
                           sizeof(sns_diag_smr_immediate_response_msg_s),
                           sizeof(sns_diag_subsys_hdr_v2_type));

  /* Done! */
  SNS_PRINTF_STRING_HIGH_0(SNS_DBG_MOD_DIAG,
                           "Sensors DIAG Handler Self Test END - SUCCESS");
  return;
} // end of fn sns_diag_self_test

