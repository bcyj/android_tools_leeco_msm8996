#ifndef QMI_QOS_SRVC_I_H
#define QMI_QOS_SRVC_I_H
/******************************************************************************
  @file    qmi_qos_srvc_i.h
  @brief   QMI message library QoS service definitions for "internal" use...
  not publicly available API's

  DESCRIPTION
  This file contains common, external header file definitions for QMI
  interface QoS "internal" functionality.

  INITIALIZATION AND SEQUENCING REQUIREMENTS
  qmi_qos_srvc_init_client() must be called to create one or more clients
  qmi_qos_srvc_release_client() must be called to delete each client when 
  finished.

  $Header: //source/qcom/qct/modem/datacommon/qmimsglib/dev/work/src/qmi_qos_srvc_i.h#1 $ 
  $DateTime: 2009/07/15 10:38:12 $
  ---------------------------------------------------------------------------
  Copyright (c) 2007 - 2009 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

/************************************************************************
* Definitions associated with qmi_qos_perform_flow_operation()
************************************************************************/

#include "qmi_i.h"

#define QMI_QOS_PERFORM_FLOW_OP_REQ_OPERATIONS_PARAM           0x0001
#define QMI_QOS_PERFORM_FLOW_OP_SET_INACTIVITY_TIMER_PARAM     0x0002
#define QMI_QOS_PERFORM_FLOW_OP_PRIMARY_FLOW_OP_PARAM          0x0004

typedef struct
{

  unsigned short          params_mask;

  unsigned long           qos_identifier;

  unsigned long           requested_operation;
  unsigned long           set_inactivity_timer;
  unsigned char           primary_flow_op;
}qmi_qos_perform_flow_op_req_type;

/*RESP*/
#define QMI_QOS_PERFORM_FLOW_OP_RESP_OP_FAILURE_PARAM           0x0001
#define QMI_QOS_PERFORM_FLOW_OP_RESP_TX_QUEUE_LEVEL_PARAM       0x0002
#define QMI_QOS_PERFORM_FLOW_OP_RESP_RMAC3_INFO_PARAM           0x0004
#define QMI_QOS_PERFORM_FLOW_OP_RESP_TX_STATUS_PARAM            0x0008
#define QMI_QOS_PERFORM_FLOW_OP_RESP_INACTIVITY_TIMER_PARAM     0x0010

typedef struct
{
  unsigned char bit_number;
  int           dss_errno;
}qmi_qos_operation_failure_param_type;

typedef enum
{
  RLP_PKT_DROP_DUE_TO_SPD                 = 0x01,       //TODO:CHANGE TEMP1 and TEMP2
  RLP_PKT_NOT_DROPPED_DUE_TO_SPD_OR_MARQ  = 0x02
}qmi_qos_perform_flow_op_tx_status;

typedef struct
{
  unsigned short          params_mask;

  struct
  {
    unsigned char                           num_failures;
    qmi_qos_operation_failure_param_type    fail_info[QMI_QOS_MAX_ERR_CODES_IN_RSP];
  }op_fail_info;

  struct
  {
    unsigned long       current_new_data_cnt;
    unsigned long       wm_free_cnt;
    unsigned long       total_pending_cnt;
  }tx_queue_level;

  struct
  {
    int                 ps_headroom_payload_size;
    int                 bucket_level_payload_size;
    int                 t2p_inflow_payload_size;
  }rmac3_info;

  qmi_qos_perform_flow_op_tx_status       flow_status;
  unsigned long                           inactivity_timer;
}qmi_qos_perform_flow_op_resp_type;


/*===========================================================================
  FUNCTION  qmi_qos_perform_flow_operation
===========================================================================*/
/*!
@brief 
  This function is a request to resume one or more existing QoS flows. 
  Each QoS flow is identified with its QoS identifier

@return 

@note
  - Number of qos_identifiers for UMTS tech should always be 1
    (not more than that).
  - Dependencies
    - qmi_qos_srvc_init_client() must be called before calling this.
  - Side Effects
    - Starts event reporting
*/    
/*=========================================================================*/
EXTERN int 
qmi_qos_perform_flow_operation
(
  int                                   client_handle,
  qmi_qos_perform_flow_op_req_type      *params,
  qmi_qos_perform_flow_op_resp_type     *resp_data,
  int                                   *qmi_err_code
);

/*===========================================================================
  FUNCTION  qmi_qos_set_event_report_state
===========================================================================*/
/*!
@brief 
  Set the QoS event reporting state
     
  
@return 

@note

  - Dependencies
    - qmi_qos_srvc_init_client() must be called before calling this.

  - Side Effects
    - Starts event reporting
*/    
/*=========================================================================*/
EXTERN int
qmi_qos_set_event_report_state
(
  int                               client_handle,
  qmi_qos_event_report_state_type   *report_state,
  int                               *qmi_err_code
);


#endif /* QMI_QOS_SRVC_I_H */

