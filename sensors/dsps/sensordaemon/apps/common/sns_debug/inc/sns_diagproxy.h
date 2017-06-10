#ifndef SNS_DIAGPROXY_H
#define SNS_DIAGPROXY_H

/*============================================================================
@file
sns_diagproxy.h

@brief
Contains defines for Diag command implementation
on Apps processor.

Copyright (c) 2010,2013-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

/*=====================================================================
                 INCLUDE FILES
=======================================================================*/
#include "sns_common.h"
#include "comdef.h"
#include "diagpkt.h"
#include <stddef.h>

/*===========================================================================
                  DEFINES
============================================================================*/
/*! Packet type values for the different messages */
#define SNS_DIAG_QMI_MSG 1
#define SNS_DIAG_SENSOR1_MSG 2

#define SNS_DIAG_SELF_TEST_MSG 10

/*! Packet type 2 Sensor Header value defines */
#define SNS_DIAG_SENSOR1_OPEN 1
#define SNS_DIAG_SENSOR1_WRITE 2
#define SNS_DIAG_SENSOR1_CLOSE 3

/*===========================================================================
                  STRUCTURES
============================================================================*/
#ifdef _WIN32
#pragma pack(push, 1)
#endif  /* _WIN32 */

/* Request packet format from PC to Phone */
typedef PACK(struct)
{
   diagpkt_subsys_header_type header;
   uint8_t                    pkt_type;
   uint8_t                    payload[1];
} sns_diag_handle_req_s;


/* Sensor Header for packet type 1 - QMI Messages to SMR */
typedef PACK(struct)
{
  uint8_t srvc_num;
  uint16_t msg_id;
  uint8_t msg_type;
  uint8_t src_module;
  uint8_t dest_module;
  uint8_t priority;
  uint8_t txn_id;
  uint8_t ext_client_id;
  uint16_t qmi_msg_size;
} sns_diag_qmi_msg_header_s;

/* Sensors Message Body for Packet Type 1 */
typedef PACK(struct)
{
  byte qmi_msg[1];
} sns_diag_qmi_msg_body_s;

/* Packet type 1 packet define */
typedef PACK(struct)
{
  sns_diag_qmi_msg_header_s msg_hdr;
  sns_diag_qmi_msg_body_s   msg_body;
} sns_diag_qmi_msg_s;

/* Sensor Header for Packet Type 2 - Requests to Sensor1 API */
typedef uint8_t sns_diag_cm_header_type;

/* Sensor1_Write request message body */
typedef PACK(struct)
{
  uint32_t cbdata;
  uint8_t srvc_num;
  int16_t msg_id;
  int8_t msg_type;
  uint8_t txn_id;
  uint16_t msg_size;
  byte qmi_msg[1];
} sns_diag_cm_write_msgbody_s;

/* Sensor1_Close request message body */
typedef PACK(struct)
{
  uint32_t cbdata;
} sns_diag_cm_close_msgbody_s;


/*! Sensor1 Request */
typedef PACK(struct)
{
  sns_diag_cm_header_type cm_fn;
  PACK(union)
  {
    sns_diag_cm_write_msgbody_s write;
    sns_diag_cm_close_msgbody_s close;
  } msgbody;
  /* Dont add any items to the end of structure. Write msg_body has a
   * variable length buffer and needs to be at the end of struct 
   */
} sns_diag_cm_msg_body_s;

/* Delayed response DIAG header */
typedef PACK(struct)
{
  uint8_t command_code;
  uint8_t subsys_id;
  uint16_t subsys_cmd_code;
  uint32_t status;  
  uint16_t delayed_rsp_id;
  uint16_t rsp_cnt; /* 0, means one response and 1, means two responses */
} sns_diag_subsys_hdr_v2_type;

/* Packet Type 2 (Sensor1 Request) Immediate Response */
typedef PACK(struct)
{
  sns_diag_subsys_hdr_v2_type diag_hdr;
  int32_t sensor1_api_ret_value;
  uint32_t cb_data;
  uint32_t context;
  uint16_t delayed_rsp_id;
  uint8_t txn_id;
} sns_diag_sensor1_immediate_response_msg_s;

/* Packet type 2 (Sensor1 Request) Delayed Response  */
typedef PACK(struct)
{
  sns_diag_subsys_hdr_v2_type hdr;
  uint16_t delayed_rsp_id;
  uint8_t txn_id;
  uint32_t cb_data;
  uint32_t context;
  uint8_t srvc_num;
  uint16_t msg_id;
  uint8_t msg_type;
  uint16_t msg_size;
  uint8_t msg;
} sns_diag_sensor1_delayed_rsp_msg_s;

/* Packet type 1 (Request through SMR) Immediate Response */
typedef PACK(struct)
{
  sns_diag_subsys_hdr_v2_type diag_hdr;
  int32_t smr_ret_val;
} sns_diag_smr_immediate_response_msg_s;

#ifdef _WIN32
#pragma pack(pop)
#endif /* _WIN32 */

/*===========================================================================
                 FUNCTIONS (Needed for unit test suite)
============================================================================*/
/* Diag Init function */
sns_err_code_e sns_diag_init(void);

/* Diag DeInit function */
void sns_diag_deinit(void);

#endif /* SNS_DIAGPROXY_H */
