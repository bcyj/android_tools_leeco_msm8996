/* Copyright (c) 2010-2012, Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#include <stdlib.h>
#include <string.h>
#include <sys/uio.h>
#include <fcntl.h>
#include <sys/socket.h>
#include "signal.h"
#include "gpsone_daemon_dbg.h"
#include "gpsone_thread_helper.h"
#include "gpsone_udp_modem.h"
#include "gpsone_udp_modem_proc.h"
#include "gpsone_conn_bridge_proc.h"
#include "gpsone_glue_pipe.h"
#include "gpsone_glue_data_service.h"
#include "gpsone_daemon_manager_handler.h"
#include "gpsone_bit_forward.h"
#include "gpsone_conn_client.h"

/*===========================================================================
FUNCTION    udp_modem_proc_init

DESCRIPTION
   This function is called after the conn_bridge_proc_init, but before the
   loop for conn_bridge_proc. It runs in parrallel with the parent thread.

   It does two things:
     1) create udp socket/fd
     2) set up named pipe fd's with daemon manager thread

DEPENDENCIES
   None

RETURN VALUE
   0: success

SIDE EFFECTS
   N/A

===========================================================================*/
static int udp_modem_proc_init(void *context)
{
    struct gpsone_conn_bridge_obj * p_conn_bridge_obj = (struct gpsone_conn_bridge_obj *) context;
    struct ctrl_msgbuf *pmsg = (struct ctrl_msgbuf*) &p_conn_bridge_obj->data;
    uint16 ip_port = pmsg->cmsg.cmsg_connect.ip_port;

    gpsone_daemon_manager_add_conn_handle(p_conn_bridge_obj);

    udp_pipe_connect(p_conn_bridge_obj);

    return gpsone_udp_modem_connect(p_conn_bridge_obj, ip_port);
}

/*===========================================================================
FUNCTION    udp_modem_proc_pre

DESCRIPTION
   initialization for udp modem task before parent thread is going.


DEPENDENCIES
   None

RETURN VALUE
   0: success

SIDE EFFECTS
   N/A

===========================================================================*/
static int udp_modem_proc_pre(void *context)
{
    return 0;
}

/*===========================================================================
FUNCTION    udp_modem_proc

DESCRIPTION
   This is the task loop for the udp modem task

   context - the pointer to the connection bridge object

DEPENDENCIES
   None

RETURN VALUE
   0: success

SIDE EFFECTS
   N/A

===========================================================================*/
static int udp_modem_proc(void *context)
{
    int result;
    struct gpsone_conn_bridge_obj * p_conn_bridge_obj = (struct gpsone_conn_bridge_obj *) context;
    GPSONE_DMN_DBG("%s:%d]\n", __func__, __LINE__);
    result = gpsone_udp_modem_task(p_conn_bridge_obj);
    GPSONE_DMN_DBG("%s:%d] result = %d\n", __func__, __LINE__, result);
    return result;
}

/*===========================================================================
FUNCTION   udp_modem_proc_post

DESCRIPTION
  This function is called after the udp modem task loop.

    context - pointer to the connection bridge object


DEPENDENCIES
   None

RETURN VALUE
   0: success

SIDE EFFECTS
   N/A

===========================================================================*/
static int udp_modem_proc_post(void *context)
{
    struct gpsone_conn_bridge_obj * p_conn_bridge_obj = (struct gpsone_conn_bridge_obj *) context;

    GPSONE_DMN_DBG("%s:%d]\n", __func__, __LINE__);

    udp_pipe_disconnect(p_conn_bridge_obj);

    return gpsone_udp_modem_disconnect(p_conn_bridge_obj);
}

/*===========================================================================
FUNCTION    gpsone_udp_modem_create

DESCRIPTION
   This function will create an udp modem task instance and launch the task.

    pmsg - the message that includes the parameters from connection request
    len - len of the pmsg

DEPENDENCIES
   None

RETURN VALUE
   NULL if failed; otherwise pointer to newly created connection bridge object

SIDE EFFECTS
   N/A

===========================================================================*/
void * gpsone_udp_modem_create(struct ctrl_msgbuf *pmsg, int len)
{
    int result;
    struct gpsone_conn_bridge_obj * p_conn_bridge_obj;

    GPSONE_DMN_DBG("%s:%d]\n", __func__, __LINE__);
    p_conn_bridge_obj = (struct gpsone_conn_bridge_obj *) malloc(sizeof(struct gpsone_conn_bridge_obj) + len - 1);


    if (!p_conn_bridge_obj) {
        GPSONE_DMN_PR_ERR("%s:%d] Out of memory\n", __func__, __LINE__);
        return NULL;
    }


    memcpy((void *)&(p_conn_bridge_obj->data), (void *)pmsg, len);

    p_conn_bridge_obj->unblock_flag = 0;

    result = gpsone_launch_thelper( &p_conn_bridge_obj->thelper,
        udp_modem_proc_init,
        udp_modem_proc_pre,
        udp_modem_proc,
        udp_modem_proc_post,
        p_conn_bridge_obj);

    if (result != 0) {
        GPSONE_DMN_PR_ERR("%s:%d]\n", __func__, __LINE__);
        free(p_conn_bridge_obj);
        p_conn_bridge_obj = NULL;
    }
    GPSONE_DMN_DBG("%s:%d] p_conn_bridge_obj = %lx\n", __func__, __LINE__, (long) p_conn_bridge_obj);
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
int gpsone_udp_modem_destroy(void * conn_bridge_handle)
{
    struct gpsone_conn_bridge_obj * p_conn_bridge_obj = (struct gpsone_conn_bridge_obj *) conn_bridge_handle;

    gpsone_unblock_thelper(&p_conn_bridge_obj->thelper);

    gpsone_conn_bridge_unblock_task(p_conn_bridge_obj);

    GPSONE_DMN_DBG("%s:%d - %d]\n", __func__, __LINE__, gpsone_daemon_manager_get_session_surfix(p_conn_bridge_obj));

    shutdown(p_conn_bridge_obj->socket_inet, SHUT_RDWR);
    gpsone_client_disconnect(p_conn_bridge_obj->socket_inet);

    gpsone_join_thelper(&p_conn_bridge_obj->thelper);

    GPSONE_DMN_DBG("%s:%d - %d]\n", __func__, __LINE__, gpsone_daemon_manager_get_session_surfix(p_conn_bridge_obj));
    free(p_conn_bridge_obj);
    return 0;
}
