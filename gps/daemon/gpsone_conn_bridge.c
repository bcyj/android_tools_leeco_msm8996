/* Copyright (c) 2010, 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * Copyright (c) 2012, 2014 Qualcomm Atheros, Inc.
 * All Rights Reserved.
 * Qualcomm Atheros Confidential and Proprietary
 */
#include <stdlib.h>
#include <string.h>
#include "gpsone_daemon_dbg.h"
#include "gpsone_thread_helper.h"
#include "gpsone_conn_bridge.h"
#include "gpsone_conn_bridge_proc.h"
#include "gpsone_glue_pipe.h"
#include "gpsone_daemon_manager_handler.h"

#ifdef _ANDROID_

#define RX_PIPENAME "/data/misc/location/gpsone_d/gpsone_pipe_rx"
#define TX_PIPENAME "/data/misc/location/gpsone_d/gpsone_pipe_tx"

#else

#define RX_PIPENAME "/tmp/gpsone_pipe_rx"
#define TX_PIPENAME "/tmp/gpsone_pipe_tx"

#endif

void conn_bridge_dbg(struct gpsone_conn_bridge_obj * p)
{
    struct ctrl_msgbuf *pmsg;

    if (p == NULL) {
        GPSONE_DMN_DBG("%s:%d] gpsone_conn_bridge_obj is NULL\n", __func__, __LINE__);
        return;
    }

    pmsg = (struct ctrl_msgbuf *) (p->data);

    GPSONE_DMN_DBG("%s:%d] session-%d thelper-%lx fwd_rx-%d fwd_tx-%d rx-%d tx-%d sock-%d unblock-%d surfix-%d\n",
        __func__, __LINE__, pmsg->session_handle, (long) &(p->thelper), p->fwd_rx_pipe, p->fwd_tx_pipe,
        p->rx_pipe, p->tx_pipe, p->socket_inet, p->unblock_flag, p->surfix/*, p->rx_buf, p->tx_buf*/);
}

/*===========================================================================
FUNCTION    conn_bridge_proc_init

DESCRIPTION
   initialization for connection bridge before parent thread is going.

   context - not used

DEPENDENCIES
   None

RETURN VALUE
   0: success

SIDE EFFECTS
   N/A

===========================================================================*/
static int conn_bridge_proc_init(void *context)
{
    int result;
    struct gpsone_conn_bridge_obj * p_conn_bridge_obj = (struct gpsone_conn_bridge_obj *) context;
    struct ctrl_msgbuf *pmsg = (struct ctrl_msgbuf *) (p_conn_bridge_obj->data);
    char rx_pipename[sizeof(RX_PIPENAME) + 16], tx_pipename[sizeof(TX_PIPENAME) + 16];
    int surfix;

    gpsone_daemon_manager_add_conn_handle(p_conn_bridge_obj);
    surfix = gpsone_daemon_manager_get_session_surfix(p_conn_bridge_obj);
    snprintf( rx_pipename, sizeof(RX_PIPENAME) + 15, RX_PIPENAME "%d", (int) surfix);
    snprintf( tx_pipename, sizeof(TX_PIPENAME) + 15, TX_PIPENAME "%d", (int) surfix);

    GPSONE_DMN_DBG("%s:%d - %d] rx_pipename %s, tx_pipename %s\n", __func__, __LINE__, surfix, rx_pipename, tx_pipename);
    result = gpsone_conn_bridge_connect(p_conn_bridge_obj,
        pmsg->cmsg.cmsg_connect,
        rx_pipename, tx_pipename);

    if (result < 0) {
      GPSONE_DMN_PR_ERR("%s:%d] error. result < 0. result = %d\n", __func__, __LINE__, result);
      del_conn_handle(p_conn_bridge_obj);
      return result;
    }

    GPSONE_DMN_DBG("%s:%d - %d] result = %d\n", __func__, __LINE__, surfix, result);

    conn_bridge_dbg(p_conn_bridge_obj);

    return result;
}

/*===========================================================================
FUNCTION    conn_bridge_proc_pre

DESCRIPTION
   This function is called after the conn_bridge_proc_init, but before the
   loop for conn_bridge_proc. It runs in parrallel with the parent thread.

   context - the pointer to the connection bridge object

DEPENDENCIES
   None

RETURN VALUE
   0: success

SIDE EFFECTS
   N/A

===========================================================================*/
static int conn_bridge_proc_pre(void *context)
{
    conn_bridge_dbg(context);

    return 0;
}


/*===========================================================================
FUNCTION    conn_bridge_proc

DESCRIPTION
   This is the task loop for connection bridge

   context - the pointer to the connection bridge object

DEPENDENCIES
   None

RETURN VALUE
   0: success

SIDE EFFECTS
   N/A

===========================================================================*/
static int conn_bridge_proc(void *context)
{
    int result;
    struct gpsone_conn_bridge_obj * p_conn_bridge_obj = (struct gpsone_conn_bridge_obj *) context;

    conn_bridge_dbg(p_conn_bridge_obj);

    GPSONE_DMN_DBG("%s:%d - %d]\n", __func__, __LINE__, gpsone_daemon_manager_get_session_surfix(p_conn_bridge_obj));
    result = gpsone_conn_bridge_task(p_conn_bridge_obj);
    GPSONE_DMN_DBG("%s:%d - %d] result = %d\n", __func__, __LINE__, gpsone_daemon_manager_get_session_surfix(p_conn_bridge_obj), result);
    return result;
}

/*===========================================================================
FUNCTION    conn_bridge_proc_post

DESCRIPTION
   This function is called after the connection bridge task loop.

   context - pointer to the connection bridge object

DEPENDENCIES
   None

RETURN VALUE
   0: success

SIDE EFFECTS
   N/A

===========================================================================*/
static int conn_bridge_proc_post(void *context)
{
    int result = 0;
    struct gpsone_conn_bridge_obj * p_conn_bridge_obj = (struct gpsone_conn_bridge_obj *) context;

    char rx_pipename[sizeof(RX_PIPENAME) + 16], tx_pipename[sizeof(TX_PIPENAME) + 16];
    int surfix;

    conn_bridge_dbg(p_conn_bridge_obj);

    surfix = gpsone_daemon_manager_get_session_surfix(p_conn_bridge_obj);

    snprintf( rx_pipename, sizeof(RX_PIPENAME) + 15, RX_PIPENAME "%d", (int) surfix);
    snprintf( tx_pipename, sizeof(TX_PIPENAME) + 15, TX_PIPENAME "%d", (int) surfix);

    GPSONE_DMN_DBG("%s:%d - %d]\n", __func__, __LINE__, gpsone_daemon_manager_get_session_surfix(p_conn_bridge_obj));

    gpsone_conn_bridge_disconnect(p_conn_bridge_obj, rx_pipename, tx_pipename);

    GPSONE_DMN_DBG("%s:%d - %d]\n", __func__, __LINE__, gpsone_daemon_manager_get_session_surfix(p_conn_bridge_obj));
    return result;
}

/*===========================================================================
FUNCTION    gpsone_conn_bridge_create

DESCRIPTION
   This function will create the connection bridge instance, and launch the
   task.

   pmsg - message that includes the parameter from connection request
   len  - len of pmsg

DEPENDENCIES
   None

RETURN VALUE
   NULL if failed; or pointer of the newly created connection bridge object

SIDE EFFECTS
   N/A

===========================================================================*/
void * gpsone_conn_bridge_create(struct ctrl_msgbuf *pmsg, int len)
{
    int result;
    struct gpsone_conn_bridge_obj * p_conn_bridge_obj;

    GPSONE_DMN_DBG("%s:%d]\n", __func__, __LINE__);
    p_conn_bridge_obj = (struct gpsone_conn_bridge_obj *) malloc(sizeof(struct gpsone_conn_bridge_obj) + len - 1);

    if (!p_conn_bridge_obj) {
        GPSONE_DMN_PR_ERR("%s:%d] Out of memory\n", __func__, __LINE__);
        return NULL;
    }

    //Intialize the object
    memset(p_conn_bridge_obj, 0, (sizeof(*p_conn_bridge_obj) + len - 1));
    memcpy((void *)&(p_conn_bridge_obj->data), (void *)pmsg, len);

    p_conn_bridge_obj->unblock_flag = 0;

    result = gpsone_launch_thelper( &p_conn_bridge_obj->thelper,
        conn_bridge_proc_init,
        conn_bridge_proc_pre,
        conn_bridge_proc,
        conn_bridge_proc_post,
        p_conn_bridge_obj);

    conn_bridge_dbg(p_conn_bridge_obj);

    if (result != 0) {
        GPSONE_DMN_PR_ERR("%s:%d]\n", __func__, __LINE__);
        free(p_conn_bridge_obj);
        p_conn_bridge_obj = NULL;
    }

    conn_bridge_dbg(p_conn_bridge_obj);

    return (void *) p_conn_bridge_obj;
}

/*===========================================================================
FUNCTION    gpsone_conn_bridge_destroy

DESCRIPTION
   This function will destroy the connection bridge. upon return, the task
   has been stopped

   conn_bridge_handle - handle to the connection bridge object

DEPENDENCIES
   None

RETURN VALUE
   0: success

SIDE EFFECTS
   N/A

===========================================================================*/
int gpsone_conn_bridge_destroy(void * conn_bridge_handle)
{
    struct gpsone_conn_bridge_obj * p_conn_bridge_obj = (struct gpsone_conn_bridge_obj *) conn_bridge_handle;

    conn_bridge_dbg(p_conn_bridge_obj);

    gpsone_unblock_thelper(&p_conn_bridge_obj->thelper);

    gpsone_conn_bridge_unblock_task(p_conn_bridge_obj);

    GPSONE_DMN_DBG("%s:%d - %d]\n", __func__, __LINE__, gpsone_daemon_manager_get_session_surfix(p_conn_bridge_obj));
    gpsone_join_thelper(&p_conn_bridge_obj->thelper);

    GPSONE_DMN_DBG("%s:%d - %d]\n", __func__, __LINE__, gpsone_daemon_manager_get_session_surfix(p_conn_bridge_obj));
    free(p_conn_bridge_obj);
    return 0;
}

/*===========================================================================
FUNCTION    gpsone_conn_bridge_get_ip_addr

DESCRIPTION
   This function returns the server ip address associated with the connection

   conn_bridge_handle - handle to the connection bridge object

DEPENDENCIES
   None

RETURN VALUE
   ip address

SIDE EFFECTS
   N/A

===========================================================================*/
unsigned int gpsone_conn_bridge_get_ip_addr(void * conn_bridge_handle)
{
    unsigned int ip_addr;
    struct gpsone_conn_bridge_obj * p_conn_bridge_obj = (struct gpsone_conn_bridge_obj *) conn_bridge_handle;
    GPSONE_DMN_DBG("%s:%d]\n", __func__, __LINE__);
    ip_addr = gpsone_conn_bridge_ip_addr_task(p_conn_bridge_obj);
    GPSONE_DMN_DBG("%s:%d] IP Address = %d\n", __func__, __LINE__, ip_addr);
    return ip_addr;
}

