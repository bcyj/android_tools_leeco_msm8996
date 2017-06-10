/* Copyright (c) 2010 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * Copyright (c) 2012 Qualcomm Atheros, Inc.
 * All Rights Reserved.
 * Qualcomm Atheros Confidential and Proprietary
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "gpsone_daemon_dbg.h"
#include "gpsone_ctrl_msg.h"
#include "gpsone_daemon_manager_handler.h"
#include "gpsone_conn_bridge.h"
#include "gpsone_udp_modem.h"
#include "gpsone_bit_forward.h"
#include "gpsone_net.h"

static int dorm_resp_needed;
static unsigned char udp_server_created = 0;
pthread_mutex_t conn_list_mutex = PTHREAD_MUTEX_INITIALIZER;

struct conn_list
{
    void * conn_bridge_handle;
    struct conn_list * next;
};

struct conn_list * conn_list_head;

/*===========================================================================
FUNCTION    init_conn_handle_list

DESCRIPTION
   This function initializes the link list which stores handlers for all
   connections

DEPENDENCIES
   None

RETURN VALUE

SIDE EFFECTS
   N/A

===========================================================================*/
void init_conn_handle_list(void)
{
    conn_list_head = NULL;
    return;
}

/*===========================================================================
FUNCTION    remove_conn_handle_list

DESCRIPTION
   This function clean the link list which stores handlers for all
   connnections

DEPENDENCIES
   None

RETURN VALUE

SIDE EFFECTS
   N/A

===========================================================================*/
void remove_conn_handle_list(void)
{
    struct conn_list * p = conn_list_head;
    void * curr_p;
    while (p) {
        curr_p= p;
        p = p->next;
        free(curr_p);
    }
    return;
}

/*===========================================================================
FUNCTION    set_conn_surfix

DESCRIPTION
   This function set a connection surfix to the link list

    conn_bridge_handle - The handle to the connection

DEPENDENCIES
   None

RETURN VALUE
   0: success

SIDE EFFECTS
   N/A

===========================================================================*/
int set_conn_surfix(void * conn_bridge_handle)
{
    struct gpsone_conn_bridge_obj * p_conn_bridge_obj;

    int surfix;
    int surfix_avail;
    struct conn_list * p;

    surfix = 0;

    do {
        p = conn_list_head;
        surfix_avail = surfix;
        while(p) {
            if (p->conn_bridge_handle != conn_bridge_handle) {
                p_conn_bridge_obj = (struct gpsone_conn_bridge_obj *) p->conn_bridge_handle;
                if (p_conn_bridge_obj->surfix == surfix_avail) {
                    surfix_avail = -1;
                    break;
                }
            }
            p = p->next;
        }
        surfix ++;
    } while (surfix_avail == -1);

    p_conn_bridge_obj = (struct gpsone_conn_bridge_obj *) conn_bridge_handle;
    p_conn_bridge_obj->surfix = surfix_avail;

    return 0;
}

/*===========================================================================
FUNCTION    gpsone_daemon_manager_add_conn_handle

DESCRIPTION
   This function add a connection handle to the link list

    conn_bridge_handle - The new handle to be added

DEPENDENCIES
   None

RETURN VALUE
   0: success
   -1: failure

SIDE EFFECTS
   N/A

===========================================================================*/
int gpsone_daemon_manager_add_conn_handle(void * conn_bridge_handle)
{
    struct conn_list * p;
    struct conn_list * p_tail;

    pthread_mutex_lock(&conn_list_mutex);

    p = (struct conn_list *) malloc(sizeof(struct conn_list));

    if (!p) {
        GPSONE_DMN_PR_ERR("%s:%d] Out of memory\n", __func__, __LINE__);
        return -1;
    }

    p->conn_bridge_handle = conn_bridge_handle;
    p->next = NULL;

    if (conn_list_head == NULL) {
        GPSONE_DMN_DBG("%s:%d] adding the first handle %lx\n", __func__, __LINE__, (long) p);
        conn_list_head = p;
    } else {
        p_tail = conn_list_head;
        while (p_tail->next) {
            GPSONE_DMN_DBG("%s:%d] more handle in the list: %lx\n", __func__, __LINE__, (long) p_tail);
            p_tail = p_tail->next;
        }
        GPSONE_DMN_DBG("%s:%d] adding handle %lx to the tail\n", __func__, __LINE__, (long) p);
        p_tail->next = p;
    }

    set_conn_surfix(conn_bridge_handle);

    pthread_mutex_unlock(&conn_list_mutex);
    return 0;
}

/*===========================================================================
FUNCTION    del_conn_handle

DESCRIPTION
   This function remove a connection handle from the link list

    conn_bridge_handle - connection handle that is to be removed

DEPENDENCIES
   None

RETURN VALUE
   0: success

SIDE EFFECTS
   N/A

===========================================================================*/
void del_conn_handle(void * conn_bridge_handle)
{
    struct conn_list * p;
    struct conn_list * p_last;

    pthread_mutex_lock(&conn_list_mutex);

    p = conn_list_head;
    p_last = NULL;
    while (p) {
        if (p->conn_bridge_handle == conn_bridge_handle) {
            break;
        }
        p_last = p;
        p = p->next;
    }
    if (p) {
        GPSONE_DMN_DBG("%s:%d] handle %lx found\n", __func__, __LINE__, (long) p);
        if (p_last == NULL) {
            conn_list_head = p->next;
        } else {
            p_last->next = p->next;
        }
        free(p);
    } else {
        GPSONE_DMN_PR_ERR("%s:%d] handle not found\n", __func__, __LINE__);
    }

    pthread_mutex_unlock(&conn_list_mutex);
    return;
}

/*===========================================================================
FUNCTION    gpsone_daemon_manager_get_session_handle

DESCRIPTION
   This function will take connection bridge handle as input, and return the
   session id of a connection

    conn_bridge_handle - connection bridge handle

DEPENDENCIES
   None

RETURN VALUE
   session_handle

SIDE EFFECTS
   N/A

===========================================================================*/
gpsone_bit_session_handle_type gpsone_daemon_manager_get_session_handle(void * conn_bridge_handle)
{
    struct gpsone_conn_bridge_obj * p_conn_bridge_obj;
    struct ctrl_msgbuf *pmsg;

    pthread_mutex_lock(&conn_list_mutex);
    p_conn_bridge_obj = (struct gpsone_conn_bridge_obj *) conn_bridge_handle;
    pmsg = (struct ctrl_msgbuf *) p_conn_bridge_obj->data;
    pthread_mutex_unlock(&conn_list_mutex);
    return pmsg->session_handle;
}

/*===========================================================================
FUNCTION    gpsone_daemon_manager_get_session_surfix

DESCRIPTION
   This function will take connection bridge handle as input, and return the
   surfix reserved for this connection

    conn_bridge_handle - connection bridge handle

DEPENDENCIES
   None

RETURN VALUE
   surfix

SIDE EFFECTS
   N/A

===========================================================================*/
int gpsone_daemon_manager_get_session_surfix(void * conn_bridge_handle)
{
    struct gpsone_conn_bridge_obj * p_conn_bridge_obj;
    int surfix;

    pthread_mutex_lock(&conn_list_mutex);
    p_conn_bridge_obj = (struct gpsone_conn_bridge_obj *) conn_bridge_handle;
    surfix = p_conn_bridge_obj->surfix;
    pthread_mutex_unlock(&conn_list_mutex);
    return surfix;
}

/*===========================================================================
FUNCTION    gpsone_daemon_manager_get_conn_handle

DESCRIPTION
   This function takes session handle as input and return the connection
   handle

DEPENDENCIES
   None

RETURN VALUE
   connection handle which is a void pointer

SIDE EFFECTS
   N/A

===========================================================================*/
void * gpsone_daemon_manager_get_conn_handle(gpsone_bit_session_handle_type session_handle)
{
    struct conn_list * p;
    void * conn_bridge_handle = NULL;
    struct gpsone_conn_bridge_obj * p_conn_bridge_obj;
    struct ctrl_msgbuf *pmsg;

    pthread_mutex_lock(&conn_list_mutex);
    p = conn_list_head;
    while (p) {
        p_conn_bridge_obj = (struct gpsone_conn_bridge_obj *) p->conn_bridge_handle;
        pmsg = (struct ctrl_msgbuf *) p_conn_bridge_obj->data;
        if (pmsg->session_handle == session_handle) {
            conn_bridge_handle = p->conn_bridge_handle;
            break;
        }
        p = p->next;
    }
    pthread_mutex_unlock(&conn_list_mutex);
    return conn_bridge_handle;
}

/*===========================================================================
FUNCTION    gpsone_daemon_manager_register

DESCRIPTION
   This function will register the daemon with the BIP API

DEPENDENCIES
   None

RETURN VALUE
   0: success

SIDE EFFECTS
   N/A

===========================================================================*/
int gpsone_daemon_manager_register(void)
{
    GPSONE_DMN_DBG("%s:%d]\n", __func__, __LINE__);
    init_conn_handle_list();
    gpsone_bit_forward_register();

    gpsone_net_init();
    dorm_resp_needed = 0;
    return 0;
}

/*===========================================================================
FUNCTION    gpsone_daemon_manager_deregister

DESCRIPTION
   This function will deregister gpsone daemon from BIT API

DEPENDENCIES
   None

RETURN VALUE
   0: success

SIDE EFFECTS
   N/A

===========================================================================*/
int gpsone_daemon_manager_deregister(void)
{
    GPSONE_DMN_DBG("%s:%d]\n", __func__, __LINE__);
    remove_conn_handle_list();

    gpsone_net_release();

    gpsone_bit_forward_deregister();
    return 0;
}

/*===========================================================================
FUNCTION    gpsone_daemon_manager_force_dormancy_handler

DESCRIPTION
   This is the force dormancy handler

   pmsg - message request
   len  - length of the message

DEPENDENCIES
   None

RETURN VALUE
   0: success

SIDE EFFECTS
   N/A

===========================================================================*/
int gpsone_daemon_manager_force_dormancy_handler(struct ctrl_msgbuf *pmsg, int len)
{
    GPSONE_DMN_DBG("%s:%d]\n", __func__, __LINE__);
    if (dorm_resp_needed == 1) {
        GPSONE_DMN_PR_ERR("%s:%d] err: force dorm request pending?\n", __func__, __LINE__);
    }
    dorm_resp_needed = 1;

    if (gpsone_net_dsi_init() == -1) {
        GPSONE_DMN_PR_ERR("%s:%d] failed\n", __func__, __LINE__);
        return -1;
    }

    gpsone_net_force_dormancy();
    return 0;
}

/*===========================================================================
FUNCTION    gpsone_daemon_manager_unforce_dormancy_handler

DESCRIPTION
   This is the unforce dormancy handler

   pmsg - message request
   len  - length of the message

DEPENDENCIES
   None

RETURN VALUE
   0: success

SIDE EFFECTS
   N/A

===========================================================================*/
int gpsone_daemon_manager_unforce_dormancy_handler(struct ctrl_msgbuf *pmsg, int len)
{
    GPSONE_DMN_DBG("%s:%d]\n", __func__, __LINE__);

    if (dorm_resp_needed == 1) {
        GPSONE_DMN_PR_ERR("%s:%d] err: unforce dorm request pending?\n", __func__, __LINE__);
    }
    dorm_resp_needed = 1;

    gpsone_net_unforce_dormancy();

    return 0;
}

/*===========================================================================
FUNCTION    gpsone_daemon_manager_dormancy_status_handler

DESCRIPTION
   This is the unforce dormancy handler

   pmsg - message request
   len  - length of the message

DEPENDENCIES
   None

RETURN VALUE
   0: success

SIDE EFFECTS
   N/A

===========================================================================*/
int gpsone_daemon_manager_dormancy_status_handler(struct ctrl_msgbuf *pmsg, int len)
{
    gpsone_bit_event_payload_type event_payload={0};
    int is_net_active;

    GPSONE_DMN_DBG("%s:%d]\n", __func__, __LINE__);
    if (dorm_resp_needed == 0) {
        GPSONE_DMN_PR_ERR("%s:%d] err: dorm resp done already?\n", __func__, __LINE__);
        return -1;
    }

    is_net_active = gpsone_net_check_dormancy_status();
    switch (is_net_active) {
    case 1:
        /* after unforce dormancy, we release resources */
        gpsone_net_dsi_release();
    case 0:
        dorm_resp_needed = 0;
        event_payload.result = GPSONE_BIT_STATUS_SUCCESS;
        event_payload.event = GPSONE_BIT_EVENT_IOCTL_RESULT;
        gpsone_bit_forward_notify(0, pmsg->transaction_id, &event_payload);
        break;
    case -1:
    default:
        GPSONE_DMN_PR_ERR("%s:%d] dorm status err?\n", __func__, __LINE__);
        break;
    }

    return 0;
}

/*===========================================================================
FUNCTION    gpsone_daemon_manager_get_local_ip_addr_handler

DESCRIPTION
   This is the get local ip address handler

   pmsg - message request
   len  - length of the message

DEPENDENCIES
   None

RETURN VALUE
   0: success

SIDE EFFECTS
   N/A

===========================================================================*/
int gpsone_daemon_manager_get_local_ip_addr_handler(struct ctrl_msgbuf *pmsg, int len)
{
    void * conn_bridge_handle = gpsone_daemon_manager_get_conn_handle(pmsg->session_handle);

    gpsone_bit_event_payload_type event_payload={0};

    if (!conn_bridge_handle) {
        GPSONE_DMN_PR_ERR("%s:%d] session_handle 0x%x not found\n", __func__, __LINE__, pmsg->session_handle);
        return -1;
    }

    event_payload.arg.ipaddr.type = GPSONE_BIT_IP_V4;
    event_payload.arg.ipaddr.addr.v4_addr = gpsone_conn_bridge_get_ip_addr(conn_bridge_handle);
    event_payload.arg.ipaddr.type = (gpsone_bit_addr_family_e_type) 0;

    event_payload.event = GPSONE_BIT_EVENT_IOCTL_RESULT;
    event_payload.result = GPSONE_BIT_STATUS_SUCCESS;

    gpsone_bit_forward_notify(pmsg->session_handle, pmsg->transaction_id, &event_payload);

    GPSONE_DMN_DBG("%s:%d]\n", __func__, __LINE__);
    return 0;
}

/*===========================================================================
FUNCTION    gpsone_daemon_manager_open_handler

DESCRIPTION
   This is the open request handler

   pmsg - message request
   len  - length of the message

DEPENDENCIES
   None

RETURN VALUE
   0: success

SIDE EFFECTS
   N/A

===========================================================================*/
int gpsone_daemon_manager_open_handler(struct ctrl_msgbuf *pmsg, int len)
{
    gpsone_bit_event_payload_type event_payload;

    event_payload.event = GPSONE_BIT_EVENT_OPEN_RESULT;
    event_payload.result = GPSONE_BIT_STATUS_SUCCESS;
    gpsone_bit_forward_notify(pmsg->session_handle, pmsg->transaction_id, &event_payload);
    GPSONE_DMN_DBG("%s:%d]\n", __func__, __LINE__);
    return 0;
}

/*===========================================================================
FUNCTION    gpsone_daemon_manager_close_handler

DESCRIPTION
   This is the close request handler

   pmsg - message request
   len  - length of the message

DEPENDENCIES
   None

RETURN VALUE
   0: success

SIDE EFFECTS
   N/A

===========================================================================*/
int gpsone_daemon_manager_close_handler(struct ctrl_msgbuf *pmsg, int len)
{
    gpsone_bit_event_payload_type event_payload;

    event_payload.event = GPSONE_BIT_EVENT_CLOSE_RESULT;
    event_payload.result = GPSONE_BIT_STATUS_SUCCESS;
    gpsone_bit_forward_notify(pmsg->session_handle, pmsg->transaction_id, &event_payload);
    GPSONE_DMN_DBG("%s:%d]\n", __func__, __LINE__);
    return 0;
}

/*===========================================================================
FUNCTION    gpsone_daemon_manager_connect_handler

DESCRIPTION
   This is the connect request handler

   pmsg - message request
   len  - length of the message

DEPENDENCIES
   None

RETURN VALUE
   0: success
   -1: failure

SIDE EFFECTS
   N/A

===========================================================================*/
int gpsone_daemon_manager_connect_handler(struct ctrl_msgbuf *pmsg, int len)
{
    int result = 0;
    void * conn_bridge_handle = NULL;
    gpsone_bit_event_payload_type event_payload;
    gpsone_bit_session_handle_type session_handle;

    GPSONE_DMN_DBG("%s:%d] session_handle is %d\n", __func__, __LINE__, (int)pmsg->session_handle);

    /*
    * if the modem tries to connect with the "special modem UDP IP and link type UDP" then we know
    * to launch the UDP conn bridge instead of a normal connection bridge task
    */
    //cmsgbuf.cmsg.cmsg_connect.is_udp  = connect_param->link_type == GPSONE_BIT_LINK_UDP? 1 : 0;
    if (pmsg->cmsg.cmsg_connect.is_udp) {
      /*
        GPSONE_DMN_DBG("%s:%d]about to launch udp server!\n", __func__, __LINE__);
        if (!udp_server_created) {
          udp_server_created = 1;
          conn_bridge_handle = gpsone_udp_modem_create(pmsg, len);
        } else {
          GPSONE_DMN_PR_ERR("%s:%d] tried to create another udp server!!!\n", __func__, __LINE__);
          return -1;
        }
      */
        GPSONE_DMN_DBG("%s:%d] udp connect, udp not implemented\n", __func__, __LINE__);
        event_payload.result = GPSONE_BIT_STATUS_NOT_IMPLEMENTED;
        event_payload.event = GPSONE_BIT_EVENT_CONNECT_RESULT;
        gpsone_bit_forward_notify(pmsg->session_handle, pmsg->transaction_id, &event_payload);
        return result;
    } else {
        int ret = gpsone_net_if_request(pmsg->cmsg.cmsg_connect.is_supl,
                                        htonl(pmsg->cmsg.cmsg_connect.ip_addr.addr.v4_addr),
                                        pmsg->cmsg.cmsg_connect.ip_addr.addr.v6_addr);
        if (ret < 0) {
          GPSONE_DMN_PR_ERR("%s:%d] network i/f bringup failed. not creating conn_bridge.\n", __func__, __LINE__);
        } else {
          conn_bridge_handle = gpsone_conn_bridge_create(pmsg, len);
        }
    }

    if (conn_bridge_handle != NULL) {
        event_payload.result = GPSONE_BIT_STATUS_SUCCESS;
        session_handle = gpsone_daemon_manager_get_session_handle(conn_bridge_handle);
    } else {
        GPSONE_DMN_DBG("%s:%d] conn_bridge launch failed. sleep for 500 ms before returning...\n", __func__, __LINE__);
        usleep(500000L);
        GPSONE_DMN_DBG("%s:%d] after 500 ms sleep. \n", __func__, __LINE__);

        result = -1;
        event_payload.result = GPSONE_BIT_STATUS_FAIL;
        session_handle = GPSONE_BIT_INVALID_SESSION_HANDLE;
    }

    event_payload.event = GPSONE_BIT_EVENT_CONNECT_RESULT;
    gpsone_bit_forward_notify(session_handle, pmsg->transaction_id, &event_payload);

    return result;
}

/*===========================================================================
FUNCTION    gpsone_daemon_manager_disconnect_handler

DESCRIPTION
   This is the disconnect handler

   pmsg - message request
   len  - length of the message

DEPENDENCIES
   None

RETURN VALUE
   0: success

SIDE EFFECTS
   N/A

===========================================================================*/
int gpsone_daemon_manager_disconnect_handler(struct ctrl_msgbuf *pmsg, int len)
{
    gpsone_bit_event_payload_type event_payload;
    void * conn_bridge_handle = gpsone_daemon_manager_get_conn_handle(pmsg->session_handle);
    struct gpsone_conn_bridge_obj *p_conn_bridge_obj = (struct gpsone_conn_bridge_obj *)conn_bridge_handle;
    struct ctrl_msgbuf *conn_pmsg = (struct ctrl_msgbuf *) p_conn_bridge_obj->data;

    GPSONE_DMN_DBG("%s:%d]\n", __func__, __LINE__);

    if (conn_bridge_handle) {
        del_conn_handle(conn_bridge_handle);

        event_payload.event = GPSONE_BIT_EVENT_DISCONNECT_RESULT;
        event_payload.result = GPSONE_BIT_STATUS_SUCCESS;

        if (!conn_pmsg->cmsg.cmsg_connect.is_udp) {
            GPSONE_DMN_DBG("%s:%d]conn bridge destroy\n", __func__, __LINE__);
            gpsone_conn_bridge_destroy(conn_bridge_handle);
            gpsone_net_if_release(conn_pmsg->cmsg.cmsg_connect.is_supl,
                                  htonl(conn_pmsg->cmsg.cmsg_connect.ip_addr.addr.v4_addr),
                                  conn_pmsg->cmsg.cmsg_connect.ip_addr.addr.v6_addr);
        }
    } else {
        event_payload.event = GPSONE_BIT_EVENT_DISCONNECT_RESULT;
        event_payload.result = GPSONE_BIT_STATUS_FAIL;

        GPSONE_DMN_PR_ERR("%s:%d] session not found 0x%x\n", __func__, __LINE__, pmsg->session_handle);
    }

    gpsone_bit_forward_notify(pmsg->session_handle, pmsg->transaction_id, &event_payload);
    return 0;
}
