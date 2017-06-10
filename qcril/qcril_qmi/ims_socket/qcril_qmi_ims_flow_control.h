/*!
  @file
  qcril_qmi_ims_flow_control.h

  @brief

*/

/*===========================================================================

  Copyright (c) 2015 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

===========================================================================*/

#ifndef QCRIL_QMI_IMS_FLOW_CONTROL_H
#define QCRIL_QMI_IMS_FLOW_CONTROL_H


#include "ril.h"
#include "qcrili.h"
#include "qcril_qmi_voice.h"
#include "qcril_qmi_imsa.h"

typedef enum qcril_qmi_ims_flow_control_req_type
{
  QCRIL_QMI_IMS_FLOW_CONTROL_HANDLE_REQ = 0,
  QCRIL_QMI_IMS_FLOW_CONTROL_REQ_COMPLETE,
  QCRIL_QMI_IMS_FLOW_CONTROL_HANDLE_NEXT_REQ,
  QCRIL_QMI_IMS_FLOW_CONTROL_CLEAR_LIST
}qcril_qmi_ims_flow_control_req_type;

typedef struct qcril_ims_flow_control_list_struct
{
  struct qcril_ims_flow_control_list_struct *next;
  struct qcril_ims_flow_control_list_struct *prev;
  boolean data_must_be_freed;
  qcril_qmi_ims_flow_control_req_type       req_type;
  qcril_evt_e_type                          event_id;
  void                                      *data;
  size_t                                    datalen;
  RIL_Token                                 t;
} qcril_ims_flow_control_list_type;

typedef struct
{
    pthread_t                         ims_flow_control_thread_id;
    pthread_mutex_t                   ims_flow_control_mutex;
    pthread_mutex_t                   list_mutex;
    qcril_ims_flow_control_list_type  list;
    fd_set                            readFds;
    int                               fdWakeupRead;
    int                               fdWakeupWrite;
} qcril_ims_flow_control_type;

typedef enum qcril_qmi_ims_flow_control_fw_request_state
{
  QCRIL_QMI_IMS_REQ_STATE_NONE = 0,
  QCRIL_QMI_IMS_REQ_STATE_IN_PROGRESS, //request already posted to event queue
  QCRIL_QMI_IMS_REQ_STATE_IN_QUEUE,    //request waiting in flow control queue
}qcril_qmi_ims_flow_control_fw_request_state;

//Structure for storing req contents
typedef struct qcril_qmi_ims_flow_control_fw_request_holder
{
  RIL_Token token;        //Android token
  int       req_id;       //Android request
  void      *payload;     //Android request payload
  size_t    payload_len;
  qcril_qmi_ims_flow_control_fw_request_state  req_state;         //Request state
}qcril_qmi_ims_flow_control_fw_request_holder;

typedef enum qcril_qmi_ims_flow_control_fw_request_action
{
  QCRIL_QMI_IMS_NONE = 0,
  QCRIL_QMI_IMS_SEND_SUCCESS_RESP, //Immediately send success resp
  QCRIL_QMI_IMS_SEND_FAILURE_RESP, //Immediately send failure resp
  QCRIL_QMI_IMS_WAIT_FOR_RESP      //Wait for the response of pending request
}qcril_qmi_ims_flow_control_fw_request_action;

//Structure to hold requests from same group
typedef struct qcril_qmi_ims_flow_control_fw_request_list
{
  qcril_qmi_ims_flow_control_fw_request_holder *req_node;         //Request received fro processing
  qcril_qmi_ims_flow_control_fw_request_action action_on_dup_req; //Action to be taken when a duplicate req received
  int timer;                                                      //Timer to wait for processing next req after receiving the resp for this req
  struct qcril_qmi_ims_flow_control_fw_request_list *next;        //Next req to be processed when resp recieved for this request
}qcril_qmi_ims_flow_control_fw_request_list;

//Structure to hold requests from different groups
typedef struct qcril_qmi_ims_flow_control_fw_req_overview
{
  int req_token;                                          //Internal token for search purpose
  qcril_qmi_ims_flow_control_fw_request_list *list_head;  //Request list for different kind of groups
  struct qcril_qmi_ims_flow_control_fw_req_overview *next;
}qcril_qmi_ims_flow_control_fw_req_overview;

static const int ims_flow_control_family_ring_incall_req[] =
{
  QCRIL_EVT_IMS_SOCKET_REQ_ANSWER,
  QCRIL_EVT_IMS_SOCKET_REQ_HANGUP,
  QCRIL_EVT_IMS_SOCKET_REQ_HANGUP_WAITING_OR_BACKGROUND,
  QCRIL_EVT_IMS_SOCKET_REQ_HANGUP_FOREGROUND_RESUME_BACKGROUND,
  QCRIL_EVT_IMS_SOCKET_REQ_SWITCH_WAITING_OR_HOLDING_AND_ACTIVE,
  QCRIL_EVT_IMS_SOCKET_REQ_CONFERENCE,
//  QCRIL_EVT_IMS_SOCKET_REQ_EXPLICIT_CALL_TRANSFER,
//  QCRIL_EVT_IMS_SOCKET_REQ_UDUB,
  QCRIL_EVT_IMS_SOCKET_REQ_MODIFY_CALL_INITIATE,
  QCRIL_EVT_IMS_SOCKET_REQ_MODIFY_CALL_CONFIRM,
  QCRIL_EVT_IMS_SOCKET_REQ_ADD_PARTICIPANT,
  QCRIL_EVT_IMS_SOCKET_REQ_CALL_DEFLECTION,
  QCRIL_EVT_IMS_SOCKET_REQ_HOLD,
  QCRIL_EVT_IMS_SOCKET_REQ_RESUME
};

static const int ims_flow_control_family_ring_common_ss_req[] =
{
  QCRIL_EVT_IMS_SOCKET_REQ_QUERY_CLIP,
  QCRIL_EVT_IMS_SOCKET_REQ_GET_CLIR,
  QCRIL_EVT_IMS_SOCKET_REQ_SET_CLIR,
  QCRIL_EVT_IMS_SOCKET_REQ_QUERY_CALL_FORWARD_STATUS,
  QCRIL_EVT_IMS_SOCKET_REQ_SET_CALL_FORWARD_STATUS,
  QCRIL_EVT_IMS_SOCKET_REQ_QUERY_CALL_WAITING,
  QCRIL_EVT_IMS_SOCKET_REQ_SET_CALL_WAITING,
  QCRIL_EVT_IMS_SOCKET_REQ_SUPP_SVC_STATUS,
  QCRIL_EVT_IMS_SOCKET_REQ_GET_COLR,
  QCRIL_EVT_IMS_SOCKET_REQ_SET_COLR
};

static const int ims_flow_control_family_ring_dtmf_req[] =
{
  QCRIL_EVT_IMS_SOCKET_REQ_DTMF,
  QCRIL_EVT_IMS_SOCKET_REQ_DTMF_START,
  QCRIL_EVT_IMS_SOCKET_REQ_DTMF_STOP
};

typedef struct qcril_qmi_ims_family_ring_list
{
  uint8_t max_req;
  int timer;
  qcril_qmi_ims_flow_control_fw_request_action action_on_dup_req;
  int *ims_flow_control_family_ring;
  struct qcril_qmi_ims_family_ring_list *next;
}qcril_qmi_ims_family_ring_list_type;

//IMS flow control init routine
void qcril_ims_flow_control_pre_init();

//Extern routine to queue request recieved on IMS socket
void qcril_qmi_ims_flow_control_event_queue
(
  qcril_qmi_ims_flow_control_req_type req_type,
  qcril_data_src_e_type data_src,
  qcril_evt_e_type event_id,
  void *data,
  size_t datalen,
  RIL_Token t
);

//routine to process queued flow control requests
void qcril_qmi_ims_flow_control_main();

void qcril_ims_flow_control_process_request
(
  qcril_evt_e_type   event_id,
  void               *data,
  size_t             datalen,
  RIL_Token          t
);

void qcril_ims_flow_control_request_complete
(
  qcril_evt_e_type   event_id,
  RIL_Token          token
);

void qcril_ims_flow_control_handle_next_request
(
  qcril_evt_e_type   event_id,
  RIL_Token          token
);

void qcril_ims_flow_control_clear_list();

//IMS flow control request holder node
qcril_qmi_ims_flow_control_fw_request_holder *qcril_qmi_ims_flow_control_fw_create_node();

//Check for similar req exists or not
void qcril_qmi_ims_flow_control_fw_check_req_from_family_ring
(
  qcril_qmi_ims_flow_control_fw_request_holder *req_node,
  qcril_qmi_ims_flow_control_fw_request_list **orig_req_list
);

//Queue current request to flow control fw
void qcril_qmi_ims_flow_control_add_req_node
(
  qcril_qmi_ims_flow_control_fw_request_holder *req_node,
  qcril_qmi_ims_flow_control_fw_request_list **req_list_head
);

qcril_qmi_ims_flow_control_fw_request_list *
qcril_qmi_ims_flow_control_get_req_list_entry
(
  RIL_Token         token,
  qcril_evt_e_type  req_id
);

#endif /* QCRIL_QMI_IMS_FLOW_CONTROL_H */
