#ifndef SENSORRDIAG_H
#define SENSORRDIAG_H

/*=====================================================================
  @file sensorrdiag.h

  @brief
    This contains definitions for Remote Diag command implementation on Apps processor.

  Copyright (c) 2010,2012,2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
=====================================================================*/

/*=====================================================================
   INCLUDE FILES
=====================================================================*/
#include "comdef.h"
#include "diagpkt.h"

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

/*=====================================================================
   DEFINES
=====================================================================*/

/* Packet type values for the different messages */
#define SNS_DIAG_SENSOR1_QMI_MSG     2
#define SNS_DIAG_SENSOR1_MSG         3

/* Packet type 2/3 Sensor Header value defines */
#define SNS_DIAG_SENSOR1_OPEN       1
#define SNS_DIAG_SENSOR1_WRITE      2
#define SNS_DIAG_SENSOR1_CLOSE      3
#define SNS_DIAG_SENSOR1_WRITABLE   4

/* OS MemCopy Functions adapted from sensordaemon utils */
#define SNS_OS_MEMCOPY(to, from, size)    memcpy(to, from, size)

/*=====================================================================
   STRUCTURES
=====================================================================*/

typedef PACK(struct)
{
  diagpkt_subsys_header_type  header;
  uint8_t                     pkt_type;
  uint8_t                     payload[1];
} sns_rdiag_handle_req_s;

/* Sensor Header for Packet Type 2/3 - Requests to Sensor1 API */
typedef uint8_t sns_rdiag_cm_header_type;

/* Sensor1_Write request message body */
typedef PACK(struct)
{
  uint32_t  cbdata;
  uint32_t  srvc_num;
  int32_t  msg_id;
  int8_t   msg_type;
  uint8_t  txn_id;
  uint16_t  msg_size;
  byte  qmi_msg[1];
} sns_rdiag_cm_write_msgbody_s;

/* Sensor1_Close request message body */
typedef PACK(struct)
{
  uint32_t  cbdata;
} sns_rdiag_cm_close_msgbody_s;

/* Sensor1_Writable request message body */
typedef PACK(struct)
{
  uint32_t  cbdata;
  uint32_t  srvc_num;
} sns_rdiag_cm_writable_msgbody_s;

/* Sensor1 Request */
typedef PACK(struct)
{
  sns_rdiag_cm_header_type cm_fn;
  PACK(union)
  {
    sns_rdiag_cm_write_msgbody_s  write_msgbody;
    sns_rdiag_cm_close_msgbody_s  close_msgbody;
    sns_rdiag_cm_writable_msgbody_s  writable_msgbody;
  };
  /* Don't add any items to the end of structure. Write msg_body
     has a variable length buffer and needs to be at the end
     of struct
   */
} sns_rdiag_cm_msg_body_s;

/* Delayed response DIAG header */
typedef PACK(struct)
{
  uint8_t  command_code;
  uint8_t  subsys_id;
  uint16_t  subsys_cmd_code;
  uint32_t  status;
  uint16_t  delayed_rsp_id;
  uint16_t  rsp_cnt;  /* 0, means one response and 1,
                             means two response */
} sns_rdiag_subsys_hdr_v2_type;

/* Packet Type 2/3 (Sensor1 Request) Immediate Response */
typedef PACK(struct)
{
  sns_rdiag_subsys_hdr_v2_type  diag_hdr;
  int32_t  sensor1_api_ret_value;
  uint32_t  cb_data;
  uint32_t  context;
  uint16_t  delayed_rsp_id;
  uint8_t  txn_id;
} sns_rdiag_sensor1_immediate_res_msg_s;

/* Packet type 2/3 (Sensor1 Request) Delayed Response */
typedef PACK(struct)
{
  sns_rdiag_subsys_hdr_v2_type hdr;
  uint16_t delayed_rsp_id;
  uint8_t  txn_id;
  uint32_t  cb_data;
  uint32_t  context;
  uint32_t  srvc_num;
  uint32_t  msg_id;
  uint8_t   msg_type;
  uint16_t  msg_size;
  uint8_t   msg;
} sns_rdiag_sensor1_delayed_rsp_msg_s;

#endif  // SENSORRDIAG_H
