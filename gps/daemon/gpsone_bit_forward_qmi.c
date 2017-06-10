/*   Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
     Qualcomm Technologies Proprietary and Confidential.

     Copyright (c) 2012, 2014 Qualcomm Atheros, Inc.
     All Rights Reserved.
     Qualcomm Atheros Confidential and Proprietary
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <arpa/inet.h> /* for inet_ntoa */
#include <netdb.h>     /* for gethostbyname */
#include <unistd.h> /* for usleep*/

#include "gpsone_daemon_dbg.h"
#include "gpsone_ctrl_msg.h"
#include "gpsone_qmi_msg.h"
#include "gpsone_glue_msg.h"
#include "gpsone_glue_pipe.h"
#include "gpsone_glue_qmi.h"
#include "gpsone_conn_bridge_proc.h"
#include "gpsone_daemon_manager_handler.h"
#include "gpsone_udp_modem_proc.h"
#include "gpsone_thread_helper.h"

#ifdef _ANDROID_
#define GPSONE_QMI_Q_PATH "/data/misc/location/gpsone_d/gpsone_qmi_q"
#else
#define GPSONE_QMI_Q_PATH "/tmp/gpsone_qmi_q"
#endif

#define GPSONE_QMI_REQ_INVALID 0
#define GPSONE_QMI_REQ_VALID 1
#define GPSONE_QMI_MAX_RETRY_ATTEMPTS 3
#define GPSONE_QMI_RETRY_INTERVAL (300000L) //3 ms

const char * global_gpsone_qmi_q_path = GPSONE_QMI_Q_PATH;
static gpsone_bit_session_handle_type global_session_count = 1;
extern const char * global_gpsone_ctrl_q_path;

//Local function declarations
static void gpsone_bit_forward_qmi_open_handler (int daemon_manager_client_msgqid, struct qmi_msgbuf * p_qmsgbuf);
static void gpsone_bit_forward_qmi_connect_handler (int daemon_manager_client_msgqid, struct qmi_msgbuf * p_qmsgbuf);
static void gpsone_bit_forward_qmi_ready_to_receive_handler (int daemon_manager_client_msgqid, struct qmi_msgbuf * p_qmsgbuf);
static void gpsone_bit_forward_qmi_data_received_status_handler (int daemon_manager_client_msgqid, struct qmi_msgbuf * p_qmsgbuf);
static void gpsone_bit_forward_qmi_send_handler (int daemon_manager_client_msgqid, struct qmi_msgbuf * p_qmsgbuf);
static void gpsone_bit_forward_dmn_notify_handler (struct qmi_msgbuf * p_qmsgbuf);
static void gpsone_bit_forward_qmi_trigger_data_push(gpsone_bit_session_handle_type session_handle);
static void gpsone_bit_forward_qmi_data_ready_handler(gpsone_bit_session_handle_type session_handle);
static void gpsone_bit_forward_qmi_get_local_host_info_handler(int daemon_manager_client_msgqid, struct qmi_msgbuf * p_qmsgbuf);
static void gpsone_bit_forward_qmi_set_dormancy_handler(int daemon_manager_client_msgqid, struct qmi_msgbuf * p_qmsgbuf);
static void gpsone_bit_forward_qmi_disconnect_handler(int daemon_manager_client_msgqid, struct qmi_msgbuf * p_qmsgbuf);
static void gpsone_bit_forward_qmi_close_handler(int daemon_manager_client_msgqid, struct qmi_msgbuf * p_qmsgbuf);
static void gpsone_bit_forward_qmi_ioctl_handler(struct bit_msg_notify *p_msg_notify);

typedef struct gpsone_bit_forward_qmi_msgQ_t {
   int bit_forward_qmi_msgqid;              /* BIT QMI Message Queue identifier (pipe-based) */
   int daemon_manager_client_msgqid;
} gpsone_bit_forward_qmi_msgQ_t;

typedef struct gpsone_bit_forward_qmi_control_t {
   gpsone_bit_forward_qmi_msgQ_t msgQRef;
   struct gpsone_thelper ctl_task_helper;       /* BIT QMI Control Task thread state */
} gpsone_bit_forward_qmi_control_t;

gpsone_bit_forward_qmi_control_t* g_gpsone_bit_forward_qmi_control = NULL;

/*===========================================================================
FUNCTION    bit_forward_qmi_proc_init

DESCRIPTION
   This is the initialization function for bit_forward_qmi. The parent thread
   will wait until this function returns.

   context - no use

DEPENDENCIES
   None

RETURN VALUE
   0: success
   -1: failure

SIDE EFFECTS
   N/A

===========================================================================*/
static int bit_forward_qmi_proc_init(void *context)
{

//    gpsone_glue_msgflush(bit_forward_qmi_msgqid);
    GPSONE_DMN_DBG("%s:%d] \n", __func__, __LINE__);
    return 0;
}

/*===========================================================================
FUNCTION    bit_forward_qmi_proc_pre

DESCRIPTION
   This function is executed before the task loop and after the bit_forward_qmi
   initialization. This function will be executed in parallel with the parent
   thread

   context - no use.

DEPENDENCIES
   None

RETURN VALUE
   0: success

SIDE EFFECTS
   N/A

===========================================================================*/
static int bit_forward_qmi_proc_pre(void *context)
{
    return 0;
}

/*===========================================================================
FUNCTION    bit_forward_qmi_proc_post

DESCRIPTION
   This function will be called after the task loop is finished.

    context - used to retireve msgQRef

DEPENDENCIES
   None

RETURN VALUE
   0: success

SIDE EFFECTS
   N/A

===========================================================================*/
static int bit_forward_qmi_proc_post(void *context)
{
    gpsone_bit_forward_qmi_msgQ_t * msgQRef = (gpsone_bit_forward_qmi_msgQ_t *)context;
    GPSONE_DMN_DBG("%s:%d] bit_forward_qmi_msgqid: %d \n", __func__, __LINE__,msgQRef->bit_forward_qmi_msgqid);
    gpsone_glue_msgremove(global_gpsone_qmi_q_path, msgQRef->bit_forward_qmi_msgqid );
    return 0;
}

/*===========================================================================
FUNCTION   bit_forward_qmi_proc

DESCRIPTION
   This is the main task loop for bit_forward_qmi. It will receive messages from
   queue, and then distribute to the corresponding handlers.

    context - used to retireve msgQRef

DEPENDENCIES
   None

RETURN VALUE
   0: success
   -1: failure

SIDE EFFECTS
   N/A

===========================================================================*/
static int bit_forward_qmi_proc(void *context)
{
    int length, sz;
    struct qmi_msgbuf * p_qmsgbuf;

    sz = sizeof(struct qmi_msgbuf);
    p_qmsgbuf = (struct qmi_msgbuf *) malloc(sz);

    if (!p_qmsgbuf) {
        GPSONE_DMN_PR_ERR("%s:%d] Out of memory\n", __func__, __LINE__);
        return -1;
    }

    gpsone_bit_forward_qmi_msgQ_t * msgQRef = (gpsone_bit_forward_qmi_msgQ_t *)context;
    length = gpsone_glue_msgrcv(msgQRef->bit_forward_qmi_msgqid, p_qmsgbuf, sz);
    if (length < 0) {
        GPSONE_DMN_PR_ERR("%s:%d] msgrcv failed fd = %d, result = %d\n", __func__, __LINE__, msgQRef->bit_forward_qmi_msgqid, length);
        return -1;
    }

    GPSONE_DMN_DBG("%s:%d] received qmi_type = %d\n", __func__, __LINE__, p_qmsgbuf->qmi_msg_type);
    switch(p_qmsgbuf->qmi_msg_type) {
        case QMI_BIT_OPEN:
            gpsone_bit_forward_qmi_open_handler(msgQRef->daemon_manager_client_msgqid, p_qmsgbuf);
            break;

        case QMI_BIT_CONNECT:
            gpsone_bit_forward_qmi_connect_handler(msgQRef->daemon_manager_client_msgqid, p_qmsgbuf);
            break;

        case QMI_BIT_READY_TO_RECEIVE :
            gpsone_bit_forward_qmi_ready_to_receive_handler
                (msgQRef->daemon_manager_client_msgqid,
                 p_qmsgbuf);
            break;

        case QMI_BIT_SEND:
            gpsone_bit_forward_qmi_send_handler(msgQRef->daemon_manager_client_msgqid, p_qmsgbuf);
            break;

        case QMI_BIT_DATA_RECEIVED_STATUS:
            gpsone_bit_forward_qmi_data_received_status_handler(msgQRef->daemon_manager_client_msgqid, p_qmsgbuf);
            break;

        case QMI_BIT_GET_LOCAL_HOST_INFO :
            gpsone_bit_forward_qmi_get_local_host_info_handler(msgQRef->daemon_manager_client_msgqid, p_qmsgbuf);
            break;

        case QMI_BIT_SET_DORMANCY :
            gpsone_bit_forward_qmi_set_dormancy_handler(msgQRef->daemon_manager_client_msgqid, p_qmsgbuf);
            break;

        case QMI_BIT_DISCONNECT:
            gpsone_bit_forward_qmi_disconnect_handler(msgQRef->daemon_manager_client_msgqid, p_qmsgbuf);
            break;

        case QMI_BIT_CLOSE:
            gpsone_bit_forward_qmi_close_handler(msgQRef->daemon_manager_client_msgqid, p_qmsgbuf);
            break;

        case QMI_BIT_NOTIFY:
            gpsone_bit_forward_dmn_notify_handler(p_qmsgbuf);
            break;

        default:
            GPSONE_DMN_PR_ERR("%s:%d] unsupported qmi_type = %d\n",
                __func__, __LINE__, p_qmsgbuf->qmi_msg_type);
            break;
    }

    GPSONE_DMN_DBG("%s:%d] done qmi_type = %d\n", __func__, __LINE__, p_qmsgbuf->qmi_msg_type);
    free(p_qmsgbuf);
    return 0;
}


/*===========================================================================
FUNCTION    gpsone_bit_forward_qmi_init

DESCRIPTION
   This function will create a thread context and msgQ for bit_forward_qmi module

DEPENDENCIES
   None

RETURN VALUE
   0: success

SIDE EFFECTS
   N/A

===========================================================================*/

int gpsone_bit_forward_qmi_init(void){

    int result;

    g_gpsone_bit_forward_qmi_control = (gpsone_bit_forward_qmi_control_t*)calloc(1, sizeof(*g_gpsone_bit_forward_qmi_control));
    if( NULL == g_gpsone_bit_forward_qmi_control )
    {
       GPSONE_DMN_PR_ERR("%s:%d Error: calloc error]\n", __func__, __LINE__);
       return -1;
    }

    g_gpsone_bit_forward_qmi_control->msgQRef.bit_forward_qmi_msgqid =
        gpsone_glue_msgget(global_gpsone_qmi_q_path, O_RDWR);
    if (g_gpsone_bit_forward_qmi_control->msgQRef.bit_forward_qmi_msgqid < 0) {
        GPSONE_DMN_PR_ERR("%s:%d] msgget failed result = %d\n", __func__, __LINE__,
                          g_gpsone_bit_forward_qmi_control->msgQRef.bit_forward_qmi_msgqid);
        return -1;
    }


    result = gpsone_launch_thelper( &g_gpsone_bit_forward_qmi_control->ctl_task_helper ,
        bit_forward_qmi_proc_init,
        bit_forward_qmi_proc_pre,
        bit_forward_qmi_proc,
        bit_forward_qmi_proc_post,
        &g_gpsone_bit_forward_qmi_control->msgQRef );
    if (result != 0) {
        GPSONE_DMN_PR_ERR("%s:%d]\n", __func__, __LINE__);
        return -1;
    }
    return 0;
}
/*===========================================================================
FUNCTION    gpsone_bit_forward_register

DESCRIPTION
   This function will register daemon to modem through QMI BIT API

DEPENDENCIES
   None

RETURN VALUE
   0: success

SIDE EFFECTS
   N/A

===========================================================================*/
int gpsone_bit_forward_register(void)
{
    bitClientStatusEnumType status;
    GPSONE_DMN_DBG("%s:%d]\n", __func__, __LINE__);

    g_gpsone_bit_forward_qmi_control->msgQRef.daemon_manager_client_msgqid
        = gpsone_glue_msgget(global_gpsone_ctrl_q_path, O_RDWR);

    status = gpsone_glue_qmi_init(g_gpsone_bit_forward_qmi_control->msgQRef.bit_forward_qmi_msgqid);

    GPSONE_DMN_DBG("%s:%d] status = %d\n", __func__, __LINE__, status);
    return 0;
}

/*===========================================================================
FUNCTION    gpsone_bit_forward_deregister

DESCRIPTION
   This function will deregister daemon from modem through QMI BIT API

DEPENDENCIES
   None

RETURN VALUE
   0: success

SIDE EFFECTS
   N/A

===========================================================================*/
int gpsone_bit_forward_deregister(void)
{
    gpsone_bit_status_e_type status;

    GPSONE_DMN_DBG("%s:%d]\n", __func__, __LINE__);

    gpsone_glue_piperemove(global_gpsone_ctrl_q_path,
                           g_gpsone_bit_forward_qmi_control->msgQRef.daemon_manager_client_msgqid);
    g_gpsone_bit_forward_qmi_control->msgQRef.daemon_manager_client_msgqid = -1;
    //TODO: Do we need to call qmi_client_release() here since this function does not
    //seem to be invoked by the daemone_mgr in the target build

    GPSONE_DMN_DBG("%s:%d] status = %d\n", __func__, __LINE__, status);
    return 0;
}

/*===========================================================================
FUNCTION    gpsone_bit_forward_notify

DESCRIPTION
   This function sends notification to QMI BIT thread from the Daemon Manager

   session_handle - unique session handle for each connection
   transaction_id - unique transaction id for each transaction
   event_payload  - event_payload to QMI BIT thread

DEPENDENCIES
   None

RETURN VALUE
   transaction_id

SIDE EFFECTS
   N/A

===========================================================================*/
int gpsone_bit_forward_notify(gpsone_bit_session_handle_type session_handle,
                              uint32 transaction_id,
                              gpsone_bit_event_payload_type * event_payload)
{
    struct qmi_msgbuf qbitfwd_msg_notify;
    GPSONE_DMN_DBG("%s:%d] , session_handle = %d, transaction_id = %d\n", __func__, __LINE__,  session_handle, transaction_id);
    qbitfwd_msg_notify.qmi_msg_type = QMI_BIT_NOTIFY;
    qbitfwd_msg_notify.qmsg.qbitfwd_msg_notify.transaction_id = transaction_id;
    qbitfwd_msg_notify.qmsg.qbitfwd_msg_notify.session_handle = session_handle;

    qbitfwd_msg_notify.qmsg.qbitfwd_msg_notify.event_payload = *event_payload;
    gpsone_glue_msgsnd(g_gpsone_bit_forward_qmi_control->msgQRef.bit_forward_qmi_msgqid,
                        &qbitfwd_msg_notify, sizeof(qbitfwd_msg_notify));
    return transaction_id;
}

/*===========================================================================
FUNCTION    gpsone_bit_forward_qmi_open_handler

DESCRIPTION
   This is the BIT QMI handler function for OPEN request from the modem. This
   function will send the open request to daemon manager

    daemon_manager_client_msgqid - msqQ Id for daemon manager
    p_qmsgbuf                    - open request msg

DEPENDENCIES
   None

RETURN VALUE
   none

SIDE EFFECTS
   N/A

===========================================================================*/
void gpsone_bit_forward_qmi_open_handler (int daemon_manager_client_msgqid, struct qmi_msgbuf * p_qmsgbuf)
{
    struct ctrl_msgbuf cmsgbuf;
    bitClientStatusEnumType rc = eBIT_CLIENT_SUCCESS;
    //First send ack for the open request to QBC
    bitClientReqUnionType req_union;
    bit_resp_msg_v01 open_req_ack;
    GPSONE_DMN_DBG("%s:%d] transaction_id = %d\n", __func__, __LINE__,
                   p_qmsgbuf->qmsg.qbitfwd_msg_open.transaction_id);
    open_req_ack.resp.result = QMI_RESULT_SUCCESS;
    open_req_ack.resp.error = QMI_ERR_NONE_V01;
    open_req_ack.transaction_id = p_qmsgbuf->qmsg.qbitfwd_msg_open.transaction_id;
    req_union.pReqAck = &open_req_ack;
    rc = gpsone_glue_qmi_send_req(QMI_BIT_OPEN_RESP_V01, req_union);
    if(rc != eBIT_CLIENT_SUCCESS)
    {
      GPSONE_DMN_PR_ERR("%s:%d] status %d\n", __func__, __LINE__,
               rc );
      return;
    }
    cmsgbuf.session_handle = 0;
    cmsgbuf.transaction_id = p_qmsgbuf->qmsg.qbitfwd_msg_open.transaction_id ;
    cmsgbuf.ctrl_type = GPSONE_BIT_OPEN;
    GPSONE_DMN_DBG("%s:%d] daemon_manager_client_msgqid = %d\n", __func__, __LINE__,
                   g_gpsone_bit_forward_qmi_control->msgQRef.daemon_manager_client_msgqid);
    gpsone_glue_msgsnd(daemon_manager_client_msgqid, &cmsgbuf, sizeof(cmsgbuf));
}
/*===========================================================================
FUNCTION    gpsone_bit_forward_dmn_notify_handler

DESCRIPTION
   This is the BIT QMI handler function for NOTIFY response from the Daemon mgr,
   connection bridge etc. This function will convey the notify responses to
   the modem (QBC)

   p_qmsgbuf                    - notify response msg

DEPENDENCIES
   None

RETURN VALUE
   none

SIDE EFFECTS
   N/A

===========================================================================*/

void gpsone_bit_forward_dmn_notify_handler (struct qmi_msgbuf * p_qmsgbuf)
{
    bitClientStatusEnumType rc = eBIT_CLIENT_SUCCESS;
    bitClientReqUnionType req_union;
    uint32                  reqId;
    GPSONE_DMN_DBG("%s:%d] Event: %d Result: %d \n", __func__, __LINE__,
                   p_qmsgbuf->qmsg.qbitfwd_msg_notify.event_payload.event,
                   p_qmsgbuf->qmsg.qbitfwd_msg_notify.event_payload.result );
    switch (p_qmsgbuf->qmsg.qbitfwd_msg_notify.event_payload.event) {
    case GPSONE_BIT_EVENT_OPEN_RESULT:
        {
        bit_open_status_ind_msg_v01 open_status_ind;
        GPSONE_DMN_DBG("%s:%d] NOTIFY OPEN_RESULT\n", __func__, __LINE__);
        reqId = QMI_BIT_OPEN_STATUS_IND_V01;
    if(p_qmsgbuf->qmsg.qbitfwd_msg_notify.event_payload.result == GPSONE_BIT_STATUS_SUCCESS)      {
       open_status_ind.status = (qmi_response_type_v01 ){QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01};
    } else {
      open_status_ind.status = (qmi_response_type_v01 ){QMI_RESULT_FAILURE_V01,QMI_ERR_GENERAL_V01};
    }
        open_status_ind.transaction_id =
         p_qmsgbuf->qmsg.qbitfwd_msg_notify.transaction_id;
        req_union.pOpenStatusInd = &open_status_ind;
        rc = gpsone_glue_qmi_send_req(reqId, req_union);
        if(rc != eBIT_CLIENT_SUCCESS){
            GPSONE_DMN_PR_ERR("%s:%d] status %d\n", __func__, __LINE__,
                   rc );
        }
        }
        break;

    case GPSONE_BIT_EVENT_DISCONNECT_RESULT:
        {
        bit_disconnect_status_ind_msg_v01 disconnect_status_ind;
        GPSONE_DMN_DBG("%s:%d] NOTIFY DISCONNECT_RESULT\n", __func__, __LINE__);
        reqId = QMI_BIT_DISCONNECT_STATUS_IND_V01 ;
    if(p_qmsgbuf->qmsg.qbitfwd_msg_notify.event_payload.result == GPSONE_BIT_STATUS_SUCCESS)      {
       disconnect_status_ind.status = (qmi_response_type_v01 ){QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01};
    } else {
      disconnect_status_ind.status = (qmi_response_type_v01 ){QMI_RESULT_FAILURE_V01,QMI_ERR_GENERAL_V01};
    }
        disconnect_status_ind.transaction_id =
         p_qmsgbuf->qmsg.qbitfwd_msg_notify.transaction_id;
        disconnect_status_ind.session_handle =
            p_qmsgbuf->qmsg.qbitfwd_msg_notify.session_handle;
        req_union.pDisconnectStatusInd = &disconnect_status_ind;
        rc = gpsone_glue_qmi_send_req(reqId, req_union);
        if(rc != eBIT_CLIENT_SUCCESS){
            GPSONE_DMN_PR_ERR("%s:%d] status %d\n", __func__, __LINE__,
                   rc );
        }
        }
        break;

    case GPSONE_BIT_EVENT_CLOSE_RESULT:
        {
        bit_close_status_ind_msg_v01 close_status_ind;
        GPSONE_DMN_DBG("%s:%d] NOTIFY CLOSE\n", __func__, __LINE__);
        reqId = QMI_BIT_CLOSE_STATUS_IND_V01;
    if(p_qmsgbuf->qmsg.qbitfwd_msg_notify.event_payload.result == GPSONE_BIT_STATUS_SUCCESS)      {
       close_status_ind.status = (qmi_response_type_v01 ){QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01};
    } else {
      close_status_ind.status = (qmi_response_type_v01 ){QMI_RESULT_FAILURE_V01,QMI_ERR_GENERAL_V01};
    }
        close_status_ind.transaction_id =
         p_qmsgbuf->qmsg.qbitfwd_msg_notify.transaction_id;
        req_union.pCloseStatusInd = &close_status_ind;
        rc = gpsone_glue_qmi_send_req(reqId, req_union);
        if(rc != eBIT_CLIENT_SUCCESS){
            GPSONE_DMN_PR_ERR("%s:%d] status %d\n", __func__, __LINE__,
                   rc );
        }
        }
        break;

    case GPSONE_BIT_EVENT_CONNECT_RESULT:

        {
        GPSONE_DMN_DBG("%s:%d] NOTIFY CONNECT\n", __func__, __LINE__);
        bit_connect_status_ind_msg_v01 connect_status_ind;
        reqId = QMI_BIT_CONNECT_STATUS_IND_V01;
        if(GPSONE_BIT_STATUS_SUCCESS == p_qmsgbuf->qmsg.qbitfwd_msg_notify.event_payload.result){
          connect_status_ind.status= (qmi_response_type_v01 ){QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01};
        } else if(GPSONE_BIT_STATUS_NOT_IMPLEMENTED ==
                  p_qmsgbuf->qmsg.qbitfwd_msg_notify.event_payload.result ) {
      connect_status_ind.status = (qmi_response_type_v01 ){QMI_RESULT_FAILURE_V01,QMI_ERR_NOT_SUPPORTED_V01};
    } else {
            connect_status_ind.status = (qmi_response_type_v01 ){QMI_RESULT_FAILURE_V01,QMI_ERR_GENERAL_V01};
        }
         connect_status_ind.transaction_id =
             p_qmsgbuf->qmsg.qbitfwd_msg_notify.transaction_id;
         connect_status_ind.session_handle =
             p_qmsgbuf->qmsg.qbitfwd_msg_notify.session_handle;
         connect_status_ind.session_handle_valid = GPSONE_QMI_REQ_VALID;
         connect_status_ind.transaction_id =
             p_qmsgbuf->qmsg.qbitfwd_msg_notify.transaction_id;
        req_union.pConnectStatusInd = &connect_status_ind;
        rc = gpsone_glue_qmi_send_req(reqId, req_union);
        if(rc != eBIT_CLIENT_SUCCESS){
            GPSONE_DMN_PR_ERR("%s:%d] status %d\n", __func__, __LINE__,
                   rc );
        }
        }
        break;

    case GPSONE_BIT_EVENT_SEND_RESULT:
        GPSONE_DMN_DBG("%s:%d] NOTIFY SEND\n", __func__, __LINE__);
        break;

    case GPSONE_BIT_EVENT_DATA_READY:
        GPSONE_DMN_DBG("%s:%d] DATA_READY\n", __func__, __LINE__);
        gpsone_bit_forward_qmi_data_ready_handler(p_qmsgbuf->qmsg.qbitfwd_msg_notify.session_handle);
        break;

    case GPSONE_BIT_EVENT_IOCTL_RESULT:
        GPSONE_DMN_DBG("%s:%d] NOTIFY IOCTL\n", __func__, __LINE__);
        gpsone_bit_forward_qmi_ioctl_handler(&p_qmsgbuf->qmsg.qbitfwd_msg_notify);
        break;

    case GPSONE_BIT_EVENT_NETWORK_STATUS:
        GPSONE_DMN_DBG("%s:%d] NOTIFY NETWORK\n", __func__, __LINE__);
        break;

    case GPSONE_BIT_EVENT_NONE:
        GPSONE_DMN_DBG("%s:%d] NOTIFY NONE\n", __func__, __LINE__);
        break;

    default:
        GPSONE_DMN_DBG("%s:%d] NOTIFY default\n", __func__, __LINE__);
        break;
    }
}


/*===========================================================================
FUNCTION    gpsone_bit_forward_qmi_connect_handler

DESCRIPTION
   This is the BIT QMI handler function for CONNECT request from the modem. This
   function will send the connect request to daemon manager

    daemon_manager_client_msgqid - msqQ Id for daemon manager
    p_qmsgbuf                    - connect request msg

DEPENDENCIES
   None

RETURN VALUE
   None

SIDE EFFECTS
   N/A

===========================================================================*/
static void gpsone_bit_forward_qmi_connect_handler
 (int daemon_manager_client_msgqid,
  struct qmi_msgbuf * p_qmsgbuf
 )
{
    struct ctrl_msgbuf cmsgbuf;
    struct hostent *p_hostent;

    GPSONE_DMN_DBG("%s:%d] transaction_id = %d\n", __func__, __LINE__,p_qmsgbuf->qmsg.qbitfwd_msg_connect.transaction_id );
    GPSONE_DMN_DBG("%s:%d] global_session_count = %d\n", __func__, __LINE__, (int) global_session_count);
    GPSONE_DMN_DBG("%s:%d] connecting to addr type: %d port: %d\n", __func__, __LINE__,
                   (int) p_qmsgbuf->qmsg.qbitfwd_msg_connect.host_info.validity_mask,
                   (int)  p_qmsgbuf->qmsg.qbitfwd_msg_connect.host_info.ipv4_port);

    cmsgbuf.session_handle = global_session_count ++;
    cmsgbuf.transaction_id = p_qmsgbuf->qmsg.qbitfwd_msg_connect.transaction_id;

    bitClientStatusEnumType rc = eBIT_CLIENT_SUCCESS;
    bitClientReqUnionType req_union;
    bit_resp_msg_v01 connect_req_ack;
    connect_req_ack.resp.result = QMI_RESULT_SUCCESS;
    connect_req_ack.resp.error = QMI_ERR_NONE_V01;
    connect_req_ack.transaction_id = p_qmsgbuf->qmsg.qbitfwd_msg_connect.transaction_id;
    req_union.pReqAck = &connect_req_ack;

    if(GPSONE_QMI_REQ_VALID == p_qmsgbuf->qmsg.qbitfwd_msg_connect.protocol_valid) {
        cmsgbuf.cmsg.cmsg_connect.is_supl =
            p_qmsgbuf->qmsg.qbitfwd_msg_connect.protocol != BIT_ENUM_PROTOCOL_ANY_V01 ? 1 : 0;
    }
    cmsgbuf.cmsg.cmsg_connect.is_udp  =
        p_qmsgbuf->qmsg.qbitfwd_msg_connect.link == BIT_ENUM_LINK_UDP_V01?1 : 0;

    if((GPSONE_QMI_REQ_VALID == p_qmsgbuf->qmsg.qbitfwd_msg_connect.host_info_valid)) {
        if ( BIT_MASK_HOST_URL_AND_PORT_V01 ==
             (p_qmsgbuf->qmsg.qbitfwd_msg_connect.host_info.validity_mask &
              BIT_MASK_HOST_URL_AND_PORT_V01)){
            GPSONE_DMN_DBG("%s:%d] host name %s\n", __func__, __LINE__,
                           p_qmsgbuf->qmsg.qbitfwd_msg_connect.host_info.url);
            cmsgbuf.cmsg.cmsg_connect.ip_port = p_qmsgbuf->qmsg.qbitfwd_msg_connect.host_info.url_port;
            p_hostent =
                gethostbyname2(p_qmsgbuf->qmsg.qbitfwd_msg_connect.host_info.url, AF_INET);
            if (p_hostent == NULL) {
                GPSONE_DMN_PR_ERR("%s:%d] gethostbyname2 for ip v4 returned null, trying ipv6!\n", __func__, __LINE__);
                p_hostent =
                    gethostbyname2(p_qmsgbuf->qmsg.qbitfwd_msg_connect.host_info.url, AF_INET6);
                if (p_hostent == NULL) {
                    GPSONE_DMN_PR_ERR("%s:%d] gethostbyname2 for ip v6 returned null could not resolve hostname\n", __func__, __LINE__);
                    connect_req_ack.resp.result = QMI_RESULT_FAILURE_V01 ;
                    connect_req_ack.resp.error = QMI_ERR_GENERAL_V01;
                } else {
                    cmsgbuf.cmsg.cmsg_connect.ip_addr.type = GPSONE_BIT_IP_V6;
                    GPSONE_DMN_DBG("%s:%d] gethostbyname returned 0x%s from %s\n", __func__, __LINE__,
                                   p_hostent->h_addr, p_qmsgbuf->qmsg.qbitfwd_msg_connect.host_info.url);
                    memcpy((void *)&cmsgbuf.cmsg.cmsg_connect.ip_addr.addr.v6_addr,
                            p_hostent->h_addr,
                            p_hostent->h_length);
                    //TODO: do we convert the ipv6 to host byte order too??
                }
            } else {
                cmsgbuf.cmsg.cmsg_connect.ip_addr.type = GPSONE_BIT_IP_V4;
                GPSONE_DMN_DBG("%s:%d] gethostbyname returned 0x%s from %s\n", __func__, __LINE__,
                               p_hostent->h_addr, p_qmsgbuf->qmsg.qbitfwd_msg_connect.host_info.url);
                memcpy((void *)&cmsgbuf.cmsg.cmsg_connect.ip_addr.addr.v4_addr,
                        p_hostent->h_addr,
                        p_hostent->h_length);
    #ifndef DEBUG_X86
                cmsgbuf.cmsg.cmsg_connect.ip_addr.addr.v4_addr = ntohl(cmsgbuf.cmsg.cmsg_connect.ip_addr.addr.v4_addr);
    #endif
            }
        } else if( BIT_MASK_IPV4_ADDR_AND_PORT_V01 ==
             (p_qmsgbuf->qmsg.qbitfwd_msg_connect.host_info.validity_mask &
              BIT_MASK_IPV4_ADDR_AND_PORT_V01)) {
                uint32 ipv4_addr = p_qmsgbuf->qmsg.qbitfwd_msg_connect.host_info.ipv4_addr;
                GPSONE_DMN_DBG("%s:%d] v4_addr: %d.%d.%d.%d", __func__, __LINE__,
                         (unsigned char)(ipv4_addr>>24),
                         (unsigned char)(ipv4_addr>>16),
                         (unsigned char)(ipv4_addr>>8),
                         (unsigned char)(ipv4_addr));
                cmsgbuf.cmsg.cmsg_connect.ip_addr.type = GPSONE_BIT_IP_V4;
                cmsgbuf.cmsg.cmsg_connect.ip_addr.addr.v4_addr =
                    p_qmsgbuf->qmsg.qbitfwd_msg_connect.host_info.ipv4_addr;
                cmsgbuf.cmsg.cmsg_connect.ip_port =
                    p_qmsgbuf->qmsg.qbitfwd_msg_connect.host_info.ipv4_port;
        } else {
            cmsgbuf.cmsg.cmsg_connect.ip_addr.type = GPSONE_BIT_IP_V6;
            memcpy(&cmsgbuf.cmsg.cmsg_connect.ip_addr.addr.v6_addr,
                &p_qmsgbuf->qmsg.qbitfwd_msg_connect.host_info.ipv6_addr,
                   sizeof(cmsgbuf.cmsg.cmsg_connect.ip_addr.addr.v6_addr));
            cmsgbuf.cmsg.cmsg_connect.ip_port =
                p_qmsgbuf->qmsg.qbitfwd_msg_connect.host_info.ipv6_port;
        }
        cmsgbuf.ctrl_type = GPSONE_BIT_CONNECT;
        GPSONE_DMN_DBG("%s:%d] daemon_manager_client_msgqid = %d\n", __func__, __LINE__,
                   g_gpsone_bit_forward_qmi_control->msgQRef.daemon_manager_client_msgqid);
        gpsone_glue_msgsnd(g_gpsone_bit_forward_qmi_control->msgQRef.daemon_manager_client_msgqid,
                            &cmsgbuf, sizeof(cmsgbuf));

    }
    else
    {
      GPSONE_DMN_PR_ERR("%s:%d] No valid Host info has been provided\n", __func__, __LINE__);
      connect_req_ack.resp.result = QMI_RESULT_FAILURE_V01 ;
      connect_req_ack.resp.error = QMI_ERR_GENERAL_V01;
    }
   rc = gpsone_glue_qmi_send_req(QMI_BIT_CONNECT_RESP_V01 , req_union);
   if(rc != eBIT_CLIENT_SUCCESS)
   {
     GPSONE_DMN_PR_ERR("%s:%d] status %d\n", __func__, __LINE__,
              rc );
   }
}
/*===========================================================================
FUNCTION    gpsone_bit_forward_qmi_ready_to_receive_handler

DESCRIPTION
   This is the BIT QMI handler function for RTR request from the modem. This
   function will update the RTR state info in BIT QMI context

    daemon_manager_client_msgqid - msqQ Id for daemon manager
    p_qmsgbuf                    - rtr request msg

DEPENDENCIES
   None

RETURN VALUE
   None

SIDE EFFECTS
   N/A

===========================================================================*/
static void gpsone_bit_forward_qmi_ready_to_receive_handler
 (int daemon_manager_client_msgqid,
  struct qmi_msgbuf * p_qmsgbuf
 )
{
    void * conn_bridge_handle;
    struct gpsone_qmi_session_context *p_qmi_session_context;
    bit_ready_to_receive_req_msg_v01 *p_ready_to_receive_req =
        &p_qmsgbuf->qmsg.qbitfwd_msg_ready_to_receive;
    GPSONE_DMN_DBG("%s:%d] transaction_id = %d sess_hndle %d rtr: %d payload_sz %d payload_vld %d\n",
                    __func__, __LINE__,
                   p_ready_to_receive_req->transaction_id,
                   (int)p_ready_to_receive_req->session_handle,
                   p_ready_to_receive_req->rtr,
                   p_ready_to_receive_req->max_recv_payload_size,
                   p_ready_to_receive_req->max_recv_payload_size_valid );

    bitClientStatusEnumType rc = eBIT_CLIENT_SUCCESS;
    bitClientReqUnionType req_union;
    bit_session_resp_msg_v01 ready_to_receive_req_ack;

    ready_to_receive_req_ack.resp.result = QMI_RESULT_SUCCESS;
    ready_to_receive_req_ack.resp.error = QMI_ERR_NONE_V01;
    ready_to_receive_req_ack.transaction_id =
        p_ready_to_receive_req->transaction_id;
    ready_to_receive_req_ack.session_handle = p_ready_to_receive_req->session_handle;
    req_union.pReqSessionAck = &ready_to_receive_req_ack;

    conn_bridge_handle =
        gpsone_daemon_manager_get_conn_handle((int)p_ready_to_receive_req->session_handle);
    if (!conn_bridge_handle) {
        GPSONE_DMN_PR_ERR("%s:%d] session_handle %d not found\n",
                          __func__, __LINE__, (int)p_ready_to_receive_req->session_handle);

        ready_to_receive_req_ack.resp.result = QMI_RESULT_FAILURE_V01 ;
        ready_to_receive_req_ack.resp.error = QMI_ERR_GENERAL_V01;
    } else {

        p_qmi_session_context =
            &(((struct gpsone_conn_bridge_obj *) conn_bridge_handle)->qmi_session_context);

        if(p_qmi_session_context) {
            p_qmi_session_context->ready_to_receive = p_ready_to_receive_req->rtr;
            if( p_qmi_session_context->ready_to_receive == false) {
                if(GPSONE_QMI_REQ_VALID == p_ready_to_receive_req->max_recv_payload_size_valid ) {
                    p_qmi_session_context->max_recv_payload_size =
                        p_ready_to_receive_req->max_recv_payload_size;
                } else {
                    GPSONE_DMN_PR_ERR("%s:%d] payload_size_valid set to false\n",
                                      __func__, __LINE__);
                    ready_to_receive_req_ack.resp.result = QMI_RESULT_FAILURE_V01 ;
                    ready_to_receive_req_ack.resp.error = QMI_ERR_GENERAL_V01;
                }
            }
        } else {
            GPSONE_DMN_PR_ERR("%s:%d] p_qmi_session_context not found\n",
                          __func__, __LINE__);
            ready_to_receive_req_ack.resp.result = QMI_RESULT_FAILURE_V01 ;
            ready_to_receive_req_ack.resp.error = QMI_ERR_GENERAL_V01;
        }
    }

   rc = gpsone_glue_qmi_send_req(QMI_BIT_READY_TO_RECEIVE_RESP_V01 , req_union);
   if(rc != eBIT_CLIENT_SUCCESS)
   {
     GPSONE_DMN_PR_ERR("%s:%d] status %d\n", __func__, __LINE__,
              rc );
   } else {
    if(QMI_RESULT_SUCCESS == ready_to_receive_req_ack.resp.result) {
     gpsone_bit_forward_qmi_trigger_data_push(p_ready_to_receive_req->session_handle);
    }
   }
}
/*===========================================================================
FUNCTION    gpsone_bit_forward_qmi_data_received_status_handler

DESCRIPTION
   This is the BIT QMI handler function for Data Received Status
   update from the modem. This function will update the data received
   state info in BIT QMI context

   daemon_manager_client_msgqid - msqQ Id for daemon manager
   p_qmsgbuf                    - Data Received Status msg

DEPENDENCIES
   None

RETURN VALUE
   None

SIDE EFFECTS
   N/A

===========================================================================*/
static void gpsone_bit_forward_qmi_data_received_status_handler
 (int daemon_manager_client_msgqid,
  struct qmi_msgbuf * p_qmsgbuf
 )
{
    void * conn_bridge_handle;
    struct gpsone_qmi_session_context *p_qmi_session_context;
    bit_data_received_status_req_msg_v01 *p_data_received_status_req =
        &p_qmsgbuf->qmsg.qbitfwd_msg_data_received_status;
    //We want to use only the lower bits of incoming sess handle
    gpsone_bit_session_handle_type local_session_handle = p_data_received_status_req->session_handle;
    boolean need_to_trigger_data_push = false;

    GPSONE_DMN_DBG("%s:%d] transaction_id = %u sess_hndle %llu result: %d error %d "
                   "seq_num %llu payload_sz_vld %d payload_sz %u\n",
                    __func__, __LINE__,
                   p_data_received_status_req->transaction_id,
                   p_data_received_status_req->session_handle,
                   p_data_received_status_req->resp.result,
                   p_data_received_status_req->resp.error,
                   p_data_received_status_req->seq_num,
                   (int)p_data_received_status_req->max_recv_payload_size_valid,
                   p_data_received_status_req->max_recv_payload_size);
    bitClientStatusEnumType rc = eBIT_CLIENT_SUCCESS;
    bitClientReqUnionType req_union;
    bit_session_resp_msg_v01 data_received_status_ack;

    data_received_status_ack.resp.result = QMI_RESULT_SUCCESS;
    data_received_status_ack.resp.error = QMI_ERR_NONE_V01;
    data_received_status_ack.transaction_id = p_data_received_status_req->transaction_id;
    data_received_status_ack.session_handle = p_data_received_status_req->session_handle;
    req_union.pReqSessionAck = &data_received_status_ack;

    conn_bridge_handle =
        gpsone_daemon_manager_get_conn_handle(local_session_handle);
    if (!conn_bridge_handle) {
        GPSONE_DMN_PR_ERR("%s:%d] session_handle %d not found\n",
                          __func__, __LINE__, local_session_handle);

        data_received_status_ack.resp = (qmi_response_type_v01) {QMI_RESULT_FAILURE_V01, QMI_ERR_GENERAL_V01};
    } else {

        p_qmi_session_context =
            &(((struct gpsone_conn_bridge_obj *) conn_bridge_handle)->qmi_session_context);
        if(p_qmi_session_context) {
            GPSONE_DMN_DBG("%s:%d]Session state data_ready: %d bytes_left: %ld retry_attempts %d\n",
                           __func__, __LINE__, p_qmi_session_context->notify_data_ready_received,
                           p_qmi_session_context->bytes_leftover,p_qmi_session_context->retry_attempts);
            if(QMI_RESULT_SUCCESS_V01 == p_data_received_status_req->resp.result ) {
                //Data push was successfull so set retry_attempts to 0, update session context
                // and continue with data push
                p_qmi_session_context->retry_attempts = 0;
                if( GPSONE_QMI_REQ_VALID == p_data_received_status_req->max_recv_payload_size_valid) {
                    p_qmi_session_context->max_recv_payload_size =
                        p_data_received_status_req->max_recv_payload_size;
                } else
                {
                    GPSONE_DMN_PR_ERR("%s:%d] payload_size_valid flag set to false\n",
                          __func__, __LINE__);
                }

                if((p_qmi_session_context->bytes_leftover > 0 )||
                   (true == p_qmi_session_context->notify_data_ready_received )) {
                    //Kick off a data_push operation if there are bytes left or more data became available
                    need_to_trigger_data_push = true;
                }
            } else {
                //The last packet transmission to modem has failed. Increment retry_attempts
                ++p_qmi_session_context->retry_attempts;
                if(p_qmi_session_context->retry_attempts <= GPSONE_QMI_MAX_RETRY_ATTEMPTS)
                {
                    //Kick off a data_push operation if retry attempts less than threshold
                    need_to_trigger_data_push = true;
                } else {
                    GPSONE_DMN_PR_ERR("%s:%d] max re-try attempts reached. Abandoning data push\n",
                          __func__, __LINE__);
                    p_qmi_session_context->retry_attempts = 0;

                    data_received_status_ack.resp = (qmi_response_type_v01){QMI_RESULT_FAILURE_V01, QMI_ERR_GENERAL_V01};
                }
            }

        } else {
            GPSONE_DMN_PR_ERR("%s:%d] p_qmi_session_context not found\n",
                          __func__, __LINE__);
            data_received_status_ack.resp = (qmi_response_type_v01){QMI_RESULT_FAILURE_V01, QMI_ERR_GENERAL_V01};
        }
    }

   rc = gpsone_glue_qmi_send_req(QMI_BIT_DATA_RECEIVED_STATUS_RESP_V01 , req_union);
   if(rc != eBIT_CLIENT_SUCCESS)
   {
     GPSONE_DMN_PR_ERR("%s:%d] status %d\n", __func__, __LINE__,
              rc );
   }
   //Set of the next data inject operation after sending the Ack for the last Data Received Status Req
   if (true == need_to_trigger_data_push) {
    gpsone_bit_forward_qmi_trigger_data_push(p_data_received_status_req->session_handle);
   }
}
/*===========================================================================
FUNCTION    gpsone_bit_forward_qmi_get_local_host_info_handler

DESCRIPTION
   This is the BIT QMI handler function for Get Local Host Info
   request from the modem. This function will transalate and forward
   this request to the Daemon Mgr

   daemon_manager_client_msgqid - msqQ Id for daemon manager
   p_qmsgbuf                    - Get Local Host Info request msg

DEPENDENCIES
   None

RETURN VALUE
   None

SIDE EFFECTS
   N/A

===========================================================================*/
void gpsone_bit_forward_qmi_get_local_host_info_handler (int daemon_manager_client_msgqid, struct qmi_msgbuf * p_qmsgbuf)
{
    struct ctrl_msgbuf cmsgbuf;
    int result;
    bitClientStatusEnumType rc = eBIT_CLIENT_SUCCESS;
    bitClientReqUnionType req_union;
    bit_session_resp_msg_v01 get_local_host_info_req_ack;
    GPSONE_DMN_DBG("%s:%d] transaction_id = %d session_handle = %d\n", __func__, __LINE__,
                   p_qmsgbuf->qmsg.qbitfwd_msg_get_local_host_info.transaction_id,
                   (int)p_qmsgbuf->qmsg.qbitfwd_msg_get_local_host_info.session_handle );
    get_local_host_info_req_ack.resp =  (qmi_response_type_v01) {QMI_RESULT_SUCCESS, QMI_ERR_NONE_V01};
    get_local_host_info_req_ack.transaction_id =
        p_qmsgbuf->qmsg.qbitfwd_msg_get_local_host_info.transaction_id;
    get_local_host_info_req_ack.session_handle = p_qmsgbuf->qmsg.qbitfwd_msg_get_local_host_info.session_handle;
    req_union.pReqSessionAck = &get_local_host_info_req_ack;

    cmsgbuf.session_handle = (uint16) p_qmsgbuf->qmsg.qbitfwd_msg_get_local_host_info.session_handle ;
    cmsgbuf.transaction_id = p_qmsgbuf->qmsg.qbitfwd_msg_get_local_host_info.transaction_id ;
    cmsgbuf.ctrl_type = GPSONE_BIT_GET_LOCAL_IP_ADDR;
    cmsgbuf.cmsg.cmsg_ioctl.reserved = -1;

    result = gpsone_glue_msgsnd(daemon_manager_client_msgqid, &cmsgbuf, sizeof(cmsgbuf));

    if (result < 0) {
        GPSONE_DMN_DBG("%s:%d] msg send fail\n", __func__, __LINE__);
        get_local_host_info_req_ack.resp =  (qmi_response_type_v01) {QMI_RESULT_FAILURE_V01 , QMI_ERR_GENERAL_V01 };
    } else {
        GPSONE_DMN_DBG("%s:%d] msg send success\n", __func__, __LINE__);
    }
    rc = gpsone_glue_qmi_send_req(QMI_BIT_GET_LOCAL_HOST_INFO_RESP_V01 , req_union);
    if(rc != eBIT_CLIENT_SUCCESS)
    {
      GPSONE_DMN_PR_ERR("%s:%d] status %d\n", __func__, __LINE__,
               rc );
      return;
    }
}
/*===========================================================================
FUNCTION    gpsone_bit_forward_qmi_set_dormancy_handler

DESCRIPTION
   This is the BIT QMI handler function for Enable/Disable Dormancy
   request from the modem. This function will transalate and forward
   this request to the Daemon Mgr

   daemon_manager_client_msgqid - msqQ Id for daemon manager
   p_qmsgbuf                    - Enable/Disable Dormancy request msg

DEPENDENCIES
   None

RETURN VALUE
   None

SIDE EFFECTS
   N/A

===========================================================================*/
void  gpsone_bit_forward_qmi_set_dormancy_handler (int daemon_manager_client_msgqid, struct qmi_msgbuf * p_qmsgbuf)
{
    struct ctrl_msgbuf cmsgbuf;
    int result;
    bitClientStatusEnumType rc = eBIT_CLIENT_SUCCESS;
    bitClientReqUnionType req_union;
    bit_session_resp_msg_v01 set_dormancy_req_ack;
    GPSONE_DMN_DBG("%s:%d] transaction_id = %d session_handle = %d Dormancy = %d\n",
                   __func__, __LINE__,
                   p_qmsgbuf->qmsg.qbitfwd_msg_set_dormancy.transaction_id,
                   (int)p_qmsgbuf->qmsg.qbitfwd_msg_set_dormancy.session_handle,
                   p_qmsgbuf->qmsg.qbitfwd_msg_set_dormancy.dormancy_state );
    set_dormancy_req_ack.resp =  (qmi_response_type_v01) {QMI_RESULT_SUCCESS, QMI_ERR_NONE_V01};
    set_dormancy_req_ack.transaction_id =
        p_qmsgbuf->qmsg.qbitfwd_msg_set_dormancy.transaction_id;
    set_dormancy_req_ack.session_handle = p_qmsgbuf->qmsg.qbitfwd_msg_set_dormancy.session_handle;
    req_union.pReqSessionAck = &set_dormancy_req_ack;

    cmsgbuf.session_handle = (uint16) p_qmsgbuf->qmsg.qbitfwd_msg_set_dormancy.session_handle ;
    cmsgbuf.transaction_id = p_qmsgbuf->qmsg.qbitfwd_msg_set_dormancy.transaction_id ;
    cmsgbuf.cmsg.cmsg_ioctl.reserved = -1;

    if(false == p_qmsgbuf->qmsg.qbitfwd_msg_set_dormancy.dormancy_state) {
        cmsgbuf.ctrl_type = GPSONE_BIT_FORCE_DORMANCY;
    } else {
        cmsgbuf.ctrl_type = GPSONE_BIT_UNFORCE_DORMANCY;
    }
    result = gpsone_glue_msgsnd(daemon_manager_client_msgqid, &cmsgbuf, sizeof(cmsgbuf));

    if (result < 0) {
        GPSONE_DMN_DBG("%s:%d] msg send fail\n", __func__, __LINE__);
        set_dormancy_req_ack.resp =  (qmi_response_type_v01) {QMI_RESULT_FAILURE_V01 , QMI_ERR_GENERAL_V01 };
    } else {
        GPSONE_DMN_DBG("%s:%d] msg send success\n", __func__, __LINE__);
    }
    rc = gpsone_glue_qmi_send_req(QMI_BIT_SET_DORMANCY_RESP_V01 , req_union);
    if(rc != eBIT_CLIENT_SUCCESS)
    {
      GPSONE_DMN_PR_ERR("%s:%d] status %d\n", __func__, __LINE__,
               rc );
      return;
    }
}
/*===========================================================================
FUNCTION    gpsone_bit_forward_qmi_disconnect_handler

DESCRIPTION
   This is the BIT QMI handler function for Disconnect
   request from the modem. This function will transalate and forward
   this request to the Daemon Mgr

   daemon_manager_client_msgqid - msqQ Id for daemon manager
   p_qmsgbuf                    - Disconnect request msg

DEPENDENCIES
   None

RETURN VALUE
   None

SIDE EFFECTS
   N/A

===========================================================================*/
void  gpsone_bit_forward_qmi_disconnect_handler (int daemon_manager_client_msgqid, struct qmi_msgbuf * p_qmsgbuf)
{
    struct ctrl_msgbuf cmsgbuf;
    int result;
    bitClientStatusEnumType rc = eBIT_CLIENT_SUCCESS;
    bitClientReqUnionType req_union;
    bit_session_resp_msg_v01 disconnect_req_ack;
    GPSONE_DMN_DBG("%s:%d] transaction_id = %d session_handle = %d \n",
                   __func__, __LINE__,
                   p_qmsgbuf->qmsg.qbitfwd_msg_disconnect.transaction_id,
                   (int)p_qmsgbuf->qmsg.qbitfwd_msg_disconnect.session_handle);
    disconnect_req_ack.resp =  (qmi_response_type_v01) {QMI_RESULT_SUCCESS, QMI_ERR_NONE_V01};
    disconnect_req_ack.transaction_id =
        p_qmsgbuf->qmsg.qbitfwd_msg_disconnect.transaction_id;
    disconnect_req_ack.session_handle = p_qmsgbuf->qmsg.qbitfwd_msg_disconnect.session_handle;
    req_union.pReqSessionAck = &disconnect_req_ack;
    cmsgbuf.session_handle = (uint16)p_qmsgbuf->qmsg.qbitfwd_msg_disconnect.session_handle;
    cmsgbuf.transaction_id = p_qmsgbuf->qmsg.qbitfwd_msg_disconnect.transaction_id;
    cmsgbuf.ctrl_type = GPSONE_BIT_DISCONNECT;
    result = gpsone_glue_msgsnd(daemon_manager_client_msgqid, &cmsgbuf, sizeof(cmsgbuf));

    if (result < 0) {
        GPSONE_DMN_DBG("%s:%d] msg send fail\n", __func__, __LINE__);
        disconnect_req_ack.resp =  (qmi_response_type_v01) {QMI_RESULT_FAILURE_V01 , QMI_ERR_GENERAL_V01 };
    } else {
        GPSONE_DMN_DBG("%s:%d] msg send success\n", __func__, __LINE__);
    }
    rc = gpsone_glue_qmi_send_req(QMI_BIT_DISCONNECT_RESP_V01, req_union);
    if(rc != eBIT_CLIENT_SUCCESS)
    {
      GPSONE_DMN_PR_ERR("%s:%d] status %d\n", __func__, __LINE__,
               rc );
      return;
    }
}
/*===========================================================================
FUNCTION    gpsone_bit_forward_qmi_close_handler

DESCRIPTION
   This is the BIT QMI handler function for CLOSE
   request from the modem. This function will transalate and forward
   this request to the Daemon Mgr

   daemon_manager_client_msgqid - msqQ Id for daemon manager
   p_qmsgbuf                    - CLOSE request msg

DEPENDENCIES
   None

RETURN VALUE
   None

SIDE EFFECTS
   N/A

===========================================================================*/
void  gpsone_bit_forward_qmi_close_handler (int daemon_manager_client_msgqid, struct qmi_msgbuf * p_qmsgbuf)
{
    struct ctrl_msgbuf cmsgbuf;
    int result;
    bitClientStatusEnumType rc = eBIT_CLIENT_SUCCESS;
    bitClientReqUnionType req_union;
    bit_resp_msg_v01 close_req_ack;
    GPSONE_DMN_DBG("%s:%d] transaction_id = %d \n",
                   __func__, __LINE__,
                   p_qmsgbuf->qmsg.qbitfwd_msg_close.transaction_id);
     close_req_ack.resp =  (qmi_response_type_v01) {QMI_RESULT_SUCCESS, QMI_ERR_NONE_V01};
     close_req_ack.transaction_id =
        p_qmsgbuf->qmsg.qbitfwd_msg_set_dormancy.transaction_id;
    req_union.pReqAck = &close_req_ack;

    cmsgbuf.session_handle = 0;
    cmsgbuf.transaction_id = 0;
    cmsgbuf.ctrl_type = GPSONE_BIT_CLOSE;
    result = gpsone_glue_msgsnd(daemon_manager_client_msgqid, &cmsgbuf, sizeof(cmsgbuf));

    if (result < 0) {
        GPSONE_DMN_DBG("%s:%d] msg send fail\n", __func__, __LINE__);
        close_req_ack.resp =  (qmi_response_type_v01) {QMI_RESULT_FAILURE_V01 , QMI_ERR_GENERAL_V01 };
    } else {
        GPSONE_DMN_DBG("%s:%d] msg send success\n", __func__, __LINE__);
    }
    rc = gpsone_glue_qmi_send_req(QMI_BIT_CLOSE_RESP_V01 , req_union);
    if(rc != eBIT_CLIENT_SUCCESS)
    {
      GPSONE_DMN_PR_ERR("%s:%d] status %d\n", __func__, __LINE__,
               rc );
      return;
    }
}


/*===========================================================================
FUNCTION    gpsone_bit_forward_qmi_send_handler

DESCRIPTION
   This is the BIT QMI handler function for data SEND
   request from the modem. This function will transalate and forward
   this request to the Daemon Mgr

   daemon_manager_client_msgqid - msqQ Id for daemon manager
   p_qmsgbuf                    - SEND request msg

DEPENDENCIES
   None

RETURN VALUE
   None

SIDE EFFECTS
   N/A

===========================================================================*/
static void gpsone_bit_forward_qmi_send_handler
 (int daemon_manager_client_msgqid,
  struct qmi_msgbuf * p_qmsgbuf
 )
{
    int result = -1;
    uint32 bytes_sent=0;
    int fwd_tx_pipe;
    void * conn_bridge_handle;

    bit_send_req_msg_v01 * p_send_req = &p_qmsgbuf->qmsg.qbitfwd_msg_send;

    GPSONE_DMN_DBG("%s:%d] session_handle = %d, transaction_id = %d\n",
                    __func__, __LINE__,  (int) p_send_req->session_handle ,
                   (int) p_send_req->transaction_id);
    bitClientStatusEnumType rc = eBIT_CLIENT_SUCCESS;
    bitClientReqUnionType req_union_ack, req_union_send_status;
    bit_session_resp_msg_v01 send_req_ack;
    send_req_ack.resp.result = QMI_RESULT_SUCCESS;
    send_req_ack.resp.error = QMI_ERR_NONE_V01;
    send_req_ack.transaction_id = p_send_req->transaction_id;
    send_req_ack.session_handle = p_send_req->session_handle;
    req_union_ack.pReqSessionAck = &send_req_ack;

    conn_bridge_handle =
        gpsone_daemon_manager_get_conn_handle((int)p_send_req->session_handle);
    if (!conn_bridge_handle) {
        GPSONE_DMN_PR_ERR("%s:%d] session_handle %d not found\n",
                          __func__, __LINE__, (int)p_send_req->session_handle);

        send_req_ack.resp.result = QMI_RESULT_FAILURE_V01 ;
        send_req_ack.resp.error = QMI_ERR_GENERAL_V01;
    } else {

        fwd_tx_pipe = ((struct gpsone_conn_bridge_obj *) conn_bridge_handle)->fwd_tx_pipe;

        if (fwd_tx_pipe) {
            result = gpsone_glue_pipewrite(fwd_tx_pipe, p_send_req->payload , p_send_req->payload_len );
          if (result < 0)
          {
            GPSONE_DMN_PR_ERR("%s:%d] pipewrite failed! reason: %s\n", __func__, __LINE__, strerror(errno));
          } else {
              bytes_sent = (uint32) result;
          }

        } else {
            GPSONE_DMN_PR_ERR("%s:%d] connection bridge not ready!\n", __func__, __LINE__);
        }

        GPSONE_DMN_DBG("%s:%d] fd = %d length = %d result = %d\n", __func__, __LINE__,
                       fwd_tx_pipe, (int) p_send_req->payload_len, result);
        if (result <= 0) {
            send_req_ack.resp.result = QMI_RESULT_FAILURE_V01 ;
            send_req_ack.resp.error = QMI_ERR_GENERAL_V01;
            GPSONE_DMN_PR_ERR("%s:%d] tx failed: fd = %d length = %d result = %d\n", __func__, __LINE__,
                              fwd_tx_pipe, (int)p_send_req->payload_len, result);
        } else {
            GPSONE_DMN_DBG("%s:%d] tx successed: fd = %d length = %d result = %d\n", __func__, __LINE__,
                           fwd_tx_pipe, (int) p_send_req->payload_len, result);
        }
    }

    rc = gpsone_glue_qmi_send_req(QMI_BIT_SEND_RESP_V01 , req_union_ack);
    if(rc != eBIT_CLIENT_SUCCESS)
    {
      GPSONE_DMN_PR_ERR("%s:%d] status %d\n", __func__, __LINE__,
               rc );
    }
    //Since the legacy BIT implementation does not send a bit_notify for Send request
    //We will have to send the StatusInd to the modem at this point itself
    bit_send_status_ind_msg_v01 send_status_ind;
    send_status_ind.session_handle = p_send_req->session_handle;
    send_status_ind.transaction_id = p_send_req->transaction_id;
    send_status_ind.status = send_req_ack.resp;
    send_status_ind.bytes_sent_valid = 1;
    send_status_ind.bytes_sent = bytes_sent;
    req_union_send_status.pSendStatusInd = &send_status_ind;
    rc = gpsone_glue_qmi_send_req(QMI_BIT_SEND_STATUS_IND_V01 , req_union_send_status);
    if(rc != eBIT_CLIENT_SUCCESS)
    {
      GPSONE_DMN_PR_ERR("%s:%d] status %d\n", __func__, __LINE__,
               rc );
    }
}

/*===========================================================================
FUNCTION    gpsone_bit_forward_qmi_trigger_data_push

DESCRIPTION
   This is the generic data push trigger handler which gets invoked when any
   of the following three events occur 1. Data Ready notification from
   connection bridge 2. RTR update from Modem QBC 3. Received Status Update
   from modem. The logic in this handler retrieves the session context info,
   analyses and determines if a data inject operation into the modem is
   warrented

   session_handle   - unique handle for the connection

DEPENDENCIES
   None

RETURN VALUE
   None

SIDE EFFECTS
   N/A

===========================================================================*/
void gpsone_bit_forward_qmi_trigger_data_push
(
  gpsone_bit_session_handle_type     session_handle
)
{
    int bytes_returned = -1;
    int fwd_rx_pipe;
    void * conn_bridge_handle;
    struct gpsone_qmi_session_context *p_qmi_session_context;
    unsigned long max_buf_size = 0;
    bitClientStatusEnumType rc = eBIT_CLIENT_SUCCESS;
    bitClientReqUnionType req_union;
    uint32                  reqId = QMI_BIT_DATA_RECEIVED_IND_V01;;
    bit_data_received_ind_msg_v01 data_received_ind;

    GPSONE_DMN_DBG("%s:%d]session_handle = %d\n", __func__, __LINE__,session_handle);

    conn_bridge_handle = gpsone_daemon_manager_get_conn_handle(session_handle);
    if (!conn_bridge_handle) {
        GPSONE_DMN_PR_ERR("%s:%d] session_handle 0x%x not found\n", __func__, __LINE__, session_handle);
        return;
    }

    p_qmi_session_context =
            &(((struct gpsone_conn_bridge_obj *) conn_bridge_handle)->qmi_session_context);

    if (!p_qmi_session_context) {
        GPSONE_DMN_PR_ERR("%s:%d] p_qmi_session_context not found\n", __func__, __LINE__);
        return;
    }

    char * p_recv_buf = p_qmi_session_context->retry_buf;
    if(false == p_qmi_session_context->ready_to_receive) {
        GPSONE_DMN_DBG("%s:%d]ready_to_receive set to false so wait\n", __func__, __LINE__);
        return;
    } else {
        max_buf_size = p_qmi_session_context->max_recv_payload_size;
    }

    GPSONE_DMN_DBG("%s:%d]Session state data_ready: %d bytes_left: %ld retry_attempts %d seq_num %ld\n",
                   __func__, __LINE__, p_qmi_session_context->notify_data_ready_received,
                   p_qmi_session_context->bytes_leftover,p_qmi_session_context->retry_attempts,
                   p_qmi_session_context->sequence_number );

    //Inject fresh data from conn_bridge buffers if data_ready or bytes left
    if((true == p_qmi_session_context->notify_data_ready_received )||
       (0 < p_qmi_session_context->bytes_leftover)) {
        fwd_rx_pipe = ((struct gpsone_conn_bridge_obj *) conn_bridge_handle)->fwd_rx_pipe;

        if (fwd_rx_pipe) {
            bytes_returned = gpsone_glue_piperead(fwd_rx_pipe, p_recv_buf, max_buf_size);
        } else {
            GPSONE_DMN_PR_ERR("%s:%d] connection bridge not ready!\n", __func__, __LINE__);
            return;
        }

        GPSONE_DMN_DBG("%s:%d] fd = %d buf_size = %d bytes_returned = %d\n", __func__, __LINE__,
                       fwd_rx_pipe, (int) max_buf_size, bytes_returned);
        if (bytes_returned > 0) {

            GPSONE_DMN_DBG("%s:%d]data received %s\n",__func__, __LINE__, p_recv_buf);
            if (ioctl(fwd_rx_pipe, FIONREAD, &p_qmi_session_context->bytes_leftover) == -1)
                  p_qmi_session_context->bytes_leftover = 0;

            GPSONE_DMN_DBG("%s:%d]p_qmi_session_context->bytes_leftover: %d\n",__func__, __LINE__,
                           (int)p_qmi_session_context->bytes_leftover);

            data_received_ind.seq_num = ++ p_qmi_session_context->sequence_number;
            if(bytes_returned > max_buf_size ) {
                GPSONE_DMN_PR_ERR("%s:%d]Error. Received more number of bytes than expected",
                                  __func__, __LINE__);
                return;
            }
            memcpy(data_received_ind.payload, p_recv_buf, bytes_returned);
            data_received_ind.payload_len = bytes_returned;
            data_received_ind.session_handle = session_handle;
            req_union.pDataReceivedInd = &data_received_ind;
            rc = gpsone_glue_qmi_send_req(reqId, req_union);
            if (rc != eBIT_CLIENT_SUCCESS) {
             GPSONE_DMN_PR_ERR("%s:%d] status %d\n", __func__, __LINE__,rc );
             return;
            } else {
             //If the transmission was successfull reset data_ready_received flag for next socket notify
             if(true == p_qmi_session_context->notify_data_ready_received)
                 p_qmi_session_context->notify_data_ready_received = false;
            }

        } else {
            if (errno == EAGAIN) {
                GPSONE_DMN_PR_ERR("%s:%d] error calling pipread EAGAIN\n", __func__, __LINE__);
                //TODO- do we need to schedule a retry attempt  ?
                bytes_returned = 0;
                p_qmi_session_context->bytes_leftover = 0;
            } else {
                GPSONE_DMN_PR_ERR("%s:%d] error calling pipread: %s\n", __func__, __LINE__, strerror(errno));

                p_qmi_session_context->bytes_leftover = 0;
            }
        }
    } else if(0 < p_qmi_session_context->retry_attempts){
        if(p_qmi_session_context->retry_attempts > GPSONE_QMI_MAX_RETRY_ATTEMPTS)
        {
            //In case we get a retry attempt request bypassing the check in data_status_req_hdlr
            GPSONE_DMN_PR_ERR("%s:%d] max re-try attempts reached. Abandoning data push\n",
                          __func__, __LINE__);
            p_qmi_session_context->retry_attempts = 0;
            return;
        }

        //Use the stored sequence number
        data_received_ind.seq_num = p_qmi_session_context->sequence_number;
        //p_recv_buf still holds the last packet
        memcpy(data_received_ind.payload, p_recv_buf, max_buf_size);
        data_received_ind.payload_len = max_buf_size;
        data_received_ind.session_handle = session_handle;
        req_union.pDataReceivedInd = &data_received_ind;
        //Carry out data push retry
        GPSONE_DMN_DBG("%s:%d] previous data pkt transmission failed. sleep for %ld ms before retyring\n",
                        __func__, __LINE__, (GPSONE_QMI_RETRY_INTERVAL/1000));
        usleep(GPSONE_QMI_RETRY_INTERVAL);
        GPSONE_DMN_DBG("%s:%d] after sleeping. \n", __func__, __LINE__);
        rc = gpsone_glue_qmi_send_req(reqId, req_union);
        if (rc != eBIT_CLIENT_SUCCESS) {
         GPSONE_DMN_PR_ERR("%s:%d] status %d\n", __func__, __LINE__,rc );
         return;
        }
    } else {
        GPSONE_DMN_DBG("%s:%d]Session state does not warrant a data push\n",
                       __func__, __LINE__);
    }
}
/*===========================================================================
FUNCTION    gpsone_bit_forward_qmi_data_ready_handler

DESCRIPTION
   This is the BIT QMI handler function for Data Ready notification from
   the Connection Bridge. This function will kick off a Data Push trigger

   session_handle                    - unique connection handle

DEPENDENCIES
   None

RETURN VALUE
   none

SIDE EFFECTS
   N/A

===========================================================================*/
void gpsone_bit_forward_qmi_data_ready_handler
(
  gpsone_bit_session_handle_type     session_handle
)
{
    void * conn_bridge_handle;
    struct gpsone_qmi_session_context *p_qmi_session_context;

    GPSONE_DMN_DBG("%s:%d]session_handle = %d\n", __func__, __LINE__,session_handle);
    conn_bridge_handle = gpsone_daemon_manager_get_conn_handle(session_handle);
    if (!conn_bridge_handle) {
        GPSONE_DMN_PR_ERR("%s:%d] session_handle 0x%x not found\n", __func__, __LINE__, session_handle);
        return;
    }
    p_qmi_session_context =
            &(((struct gpsone_conn_bridge_obj *) conn_bridge_handle)->qmi_session_context);

    if (!p_qmi_session_context) {
        GPSONE_DMN_PR_ERR("%s:%d] p_qmi_session_context not found\n", __func__, __LINE__);
        return;
    }
    //Set the Data Ready flag for this session
    p_qmi_session_context->notify_data_ready_received = true;
    gpsone_bit_forward_qmi_trigger_data_push(session_handle);
}
/*===========================================================================
FUNCTION    gpsone_bit_forward_qmi_ioctl_handler

DESCRIPTION
   This is the BIT QMI handler function for IOCTL notify response
   from the daemon manager. This function will transalate and forward
   the response to the Modem (QBC)

   p_qmsgbuf                    - IOCTL notify msg

DEPENDENCIES
   None

RETURN VALUE
   None

SIDE EFFECTS
   N/A

===========================================================================*/
void gpsone_bit_forward_qmi_ioctl_handler(struct bit_msg_notify *p_msg_notify)
{
    uint32                  reqId;
    bitClientStatusEnumType rc = eBIT_CLIENT_SUCCESS;
    bitClientReqUnionType req_union;
    GPSONE_DMN_DBG("%s:%d] sess_hndle %d tr-id %d event %d result %d\n", __func__, __LINE__,
                   p_msg_notify->session_handle,
                   p_msg_notify->transaction_id,
                   p_msg_notify->event_payload.event,
                   p_msg_notify->event_payload.result);
    //TODO: remove the hack below as currently there is no way of figuring out for
    //which ioctl this response is meant for
    if(0 != p_msg_notify->event_payload.arg.ipaddr.addr.v4_addr ) {
        //this IOCTL response is for get_host_info request
        bit_get_local_host_info_status_ind_msg_v01 get_local_host_info_status_ind;
        reqId = QMI_BIT_GET_LOCAL_HOST_INFO_STATUS_IND_V01;
        get_local_host_info_status_ind.session_handle = p_msg_notify->session_handle;
        get_local_host_info_status_ind.transaction_id = p_msg_notify->transaction_id;
        get_local_host_info_status_ind.local_host_info_valid = GPSONE_QMI_REQ_INVALID;

        if(p_msg_notify->event_payload.result == GPSONE_BIT_STATUS_SUCCESS)
        {
         get_local_host_info_status_ind.status = (qmi_response_type_v01){QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01};
         if(0 != p_msg_notify->event_payload.arg.ipaddr.addr.v4_addr) {
             get_local_host_info_status_ind.local_host_info_valid = GPSONE_QMI_REQ_VALID;
             get_local_host_info_status_ind.local_host_info.ipv4_addr = p_msg_notify->event_payload.arg.ipaddr.addr.v4_addr;
             get_local_host_info_status_ind.local_host_info.validity_mask = BIT_MASK_IPV4_ADDR_AND_PORT_V01;
         } else {
             GPSONE_DMN_PR_ERR("%s:%d] no Support yet for IPV6 for get_local_host_info\n", __func__, __LINE__);
         }

        } else {
            get_local_host_info_status_ind.status = (qmi_response_type_v01){QMI_RESULT_FAILURE_V01, QMI_ERR_GENERAL_V01};
            GPSONE_DMN_PR_ERR("%s:%d] get_local_host_info received Error from Daemon mgr\n", __func__, __LINE__);
        }
        req_union.pGetLocalHostInfoInd = &get_local_host_info_status_ind;
    } else {
        //This IOCTL response meant for force dormancy request
        bit_set_dormancy_status_ind_msg_v01 set_dormancy_status_ind;
        reqId = QMI_BIT_SET_DORMANCY_STATUS_IND_V01;
        set_dormancy_status_ind.transaction_id = p_msg_notify->transaction_id;
        set_dormancy_status_ind.session_handle = p_msg_notify->session_handle;
        if(p_msg_notify->event_payload.result == GPSONE_BIT_STATUS_SUCCESS)
        {
            set_dormancy_status_ind.status = (qmi_response_type_v01){QMI_RESULT_SUCCESS_V01, QMI_ERR_NONE_V01};

        } else {
            set_dormancy_status_ind.status = (qmi_response_type_v01){QMI_RESULT_FAILURE_V01, QMI_ERR_GENERAL_V01};
            GPSONE_DMN_PR_ERR("%s:%d] set_dormancy_status_req received Error from Daemon mgr\n", __func__, __LINE__);
        }
        req_union.pSetDormancyStatusInd = &set_dormancy_status_ind;
    }

    rc = gpsone_glue_qmi_send_req(reqId, req_union);
    if(rc != eBIT_CLIENT_SUCCESS)
    {
      GPSONE_DMN_PR_ERR("%s:%d] status %d\n", __func__, __LINE__,
               rc );
    }
}
/*===========================================================================
FUNCTION    gpsone_bit_forward_qmi_destroy

DESCRIPTION
   This function will deregister with QCCI and deallocate the global
   context for BIT Forward QMI module

DEPENDENCIES
   None

RETURN VALUE
   0: success

SIDE EFFECTS
   N/A

===========================================================================*/

int gpsone_bit_forward_qmi_destroy(void){

    int result;

    //Close the connection as a client to QCCI
    if( eBIT_CLIENT_SUCCESS != gpsone_glue_qmi_close()) {
     GPSONE_DMN_PR_ERR("%s:%d]: error %d calling gpsone_glue_qmi_close\n",
                   __func__, __LINE__);
     return -1;
    }

    if(NULL != g_gpsone_bit_forward_qmi_control) {
     int rc = gpsone_glue_msgremove(global_gpsone_qmi_q_path,
                                    g_gpsone_bit_forward_qmi_control->msgQRef.bit_forward_qmi_msgqid);

     free(g_gpsone_bit_forward_qmi_control);
     g_gpsone_bit_forward_qmi_control = NULL;

     if(rc != 0) {
      GPSONE_DMN_PR_ERR("%s:%d Error: calling gpsone_glue_msgremove for bit_forward_qmi_msgqid\n", __func__, __LINE__);
       return -1;
     }
    }

    return 0;
}

