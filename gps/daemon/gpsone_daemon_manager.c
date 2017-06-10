/* Copyright (c) 2010, 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * Copyright (c) 2012, 2014 Qualcomm Atheros, Inc.
 * All Rights Reserved.
 * Qualcomm Atheros Confidential and Proprietary
 */
#include <stdio.h>
#include <stdlib.h>

#include <linux/stat.h>
#include <fcntl.h>
#include <linux/types.h>

#include "gpsone_daemon_dbg.h"
#include "gpsone_thread_helper.h"
#include "gpsone_ctrl_msg.h"
#include "gpsone_daemon_manager_handler.h"
#include "gpsone_glue_msg.h"
#include "gpsone_glue_rpc.h"

#ifdef _ANDROID_

#define GPSONE_LOC_API_Q_PATH "/data/misc/location/gpsone_d/gpsone_loc_api_q"
#define GPSONE_CTRL_Q_PATH "/data/misc/location/gpsone_d/gpsone_ctrl_q"
#define GPSONE_LOC_API_RESP_Q_PATH "/data/misc/location/gpsone_d/gpsone_loc_api_resp_q"

#else

#define GPSONE_LOC_API_Q_PATH "/tmp/gpsone_loc_api_q"
#define GPSONE_CTRL_Q_PATH "/tmp/gpsone_ctrl_q"
#define GPSONE_LOC_API_RESP_Q_PATH "/tmp/gpsone_loc_api_resp_q"

#endif

int daemon_manager_msgqid = -1;
const char * global_gpsone_loc_api_q_path = GPSONE_LOC_API_Q_PATH;
const char * global_gpsone_ctrl_q_path = GPSONE_CTRL_Q_PATH;
const char * global_gpsone_loc_api_resp_q_path = GPSONE_LOC_API_RESP_Q_PATH;

/*===========================================================================
FUNCTION    daemon_manager_proc_init

DESCRIPTION
   This is the initialization function for daemon manager. The parent thread
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
static int daemon_manager_proc_init(void *context)
{
    daemon_manager_msgqid = gpsone_glue_msgget(global_gpsone_ctrl_q_path, O_RDWR);
    if (daemon_manager_msgqid < 0) {
        GPSONE_DMN_PR_ERR("%s:%d] msgget failed result = %d\n", __func__, __LINE__, daemon_manager_msgqid);
        return -1;
    }

//    gpsone_glue_msgflush(daemon_manager_msgqid);
    GPSONE_DMN_DBG("%s:%d] daemon_manager_msgqid = %d\n", __func__, __LINE__, daemon_manager_msgqid);
    return 0;
}

/*===========================================================================
FUNCTION    daemon_manager_proc_pre

DESCRIPTION
   This function is executed before the task loop and after the daemon manager
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
static int daemon_manager_proc_pre(void *context)
{
    return 0;
}

/*===========================================================================
FUNCTION    daemon_manager_proc

DESCRIPTION
   This is the task loop for daemon manager. It will receive messages from
   queue, and then distribute to the corresponding handlers.

    context - no use

DEPENDENCIES
   None

RETURN VALUE
   0: success
   -1: failure

SIDE EFFECTS
   N/A

===========================================================================*/
static int daemon_manager_proc(void *context)
{
    static uint32 local_transaction_id = -1;
    int length, sz;
    struct ctrl_msgbuf * p_cmsgbuf;

    sz = sizeof(struct ctrl_msgbuf);
    p_cmsgbuf = (struct ctrl_msgbuf *) malloc(sz);

    if (!p_cmsgbuf) {
        GPSONE_DMN_PR_ERR("%s:%d] Out of memory\n", __func__, __LINE__);
        return -1;
    }

    length = gpsone_glue_msgrcv(daemon_manager_msgqid, p_cmsgbuf, sz);
    if (length < 0) {
        GPSONE_DMN_PR_ERR("%s:%d] msgrcv failed fd = %d, result = %d\n", __func__, __LINE__, daemon_manager_msgqid, length);
        return -1;
    }

    GPSONE_DMN_DBG("%s:%d] received ctrl_type = %d\n", __func__, __LINE__, p_cmsgbuf->ctrl_type);
    switch(p_cmsgbuf->ctrl_type) {
        case GPSONE_BIT_OPEN:
            gpsone_daemon_manager_open_handler(p_cmsgbuf, length);
            break;

        case GPSONE_BIT_CLOSE:
            gpsone_daemon_manager_close_handler(p_cmsgbuf, length);
            break;

        case GPSONE_BIT_CONNECT:
            gpsone_daemon_manager_connect_handler(p_cmsgbuf, length);
            break;

        case GPSONE_BIT_DISCONNECT:
            gpsone_daemon_manager_disconnect_handler(p_cmsgbuf, length);
            break;

        case GPSONE_UNBLOCK:
            GPSONE_DMN_DBG("%s:%d] GPSONE_UNBLOCK\n", __func__, __LINE__);
            break;

        case GPSONE_FORCE_DORMANCY_STATUS:
            /* this message is from data stack callback, so there is no transaction_id */
            p_cmsgbuf->transaction_id = local_transaction_id;
            gpsone_daemon_manager_dormancy_status_handler(p_cmsgbuf, length);
            break;

        case GPSONE_BIT_FORCE_DORMANCY:
            local_transaction_id = p_cmsgbuf->transaction_id;
            gpsone_daemon_manager_force_dormancy_handler(p_cmsgbuf, length);
            break;

        case GPSONE_BIT_UNFORCE_DORMANCY:
            local_transaction_id = p_cmsgbuf->transaction_id;
            gpsone_daemon_manager_unforce_dormancy_handler(p_cmsgbuf, length);
            break;

        case GPSONE_BIT_GET_LOCAL_IP_ADDR:
            gpsone_daemon_manager_get_local_ip_addr_handler(p_cmsgbuf, length);
            break;

        default:
            GPSONE_DMN_PR_ERR("%s:%d] unsupported ctrl_type = %d\n",
                __func__, __LINE__, p_cmsgbuf->ctrl_type);
            break;
    }

    GPSONE_DMN_DBG("%s:%d] done ctrl_type = %d\n", __func__, __LINE__, p_cmsgbuf->ctrl_type);
    free(p_cmsgbuf);
    return 0;
}

/*===========================================================================
FUNCTION    daemon_manager_proc_post

DESCRIPTION
   This function will be called after the task loop is finished.

    context - no use

DEPENDENCIES
   None

RETURN VALUE
   0: success

SIDE EFFECTS
   N/A

===========================================================================*/
static int daemon_manager_proc_post(void *context)
{
    GPSONE_DMN_DBG("%s:%d]\n", __func__, __LINE__);
    gpsone_glue_msgremove(global_gpsone_ctrl_q_path, daemon_manager_msgqid);
    return 0;
}

/*===========================================================================
FUNCTION    gpsone_daemon_manager_unblock_task

DESCRIPTION
   This function will unblock the daemon manager task loop by sending a message
   to the daemon manager queue.

DEPENDENCIES
   None

RETURN VALUE
   0: success

SIDE EFFECTS
   N/A

===========================================================================*/
static int gpsone_daemon_manager_unblock_task(void)
{
    struct ctrl_msgbuf cmsgbuf;
    cmsgbuf.ctrl_type = GPSONE_UNBLOCK;
    GPSONE_DMN_DBG("%s:%d]\n", __func__, __LINE__);
    gpsone_glue_msgsnd(daemon_manager_msgqid, & cmsgbuf, sizeof(cmsgbuf));
    return 0;
}

#ifdef DEBUG_X86
#ifndef FEATURE_QMI
void test_bit_main_args(int argc, char *argv[]);
int test_bit_main(int argc, char * argv[]);
#endif
#endif

/*===========================================================================
FUNCTION    main

DESCRIPTION
   This is the main for gpsone_daemon

DEPENDENCIES
   None

RETURN VALUE
   0: success

SIDE EFFECTS
   N/A

===========================================================================*/
int main(int argc, char * argv[])
{
    int result;
    struct gpsone_thelper thelper;

#ifdef FEATURE_QMI
    result = gpsone_bit_forward_qmi_init();
#else
    result = gpsone_glue_rpc_init();
#endif
    if (result != 0) {
      GPSONE_DMN_PR_ERR("%s:%d]\n", __func__, __LINE__);
      return -1;
    }
#ifdef DEBUG_X86
#ifndef FEATURE_QMI
    test_bit_main_args(argc, argv);
#endif
#endif

    result = gpsone_launch_thelper( &thelper,
        daemon_manager_proc_init,
        daemon_manager_proc_pre,
        daemon_manager_proc,
        daemon_manager_proc_post,
        NULL);
    if (result != 0) {
        GPSONE_DMN_PR_ERR("%s:%d]\n", __func__, __LINE__);
        return -1;
    }

    gpsone_daemon_manager_register();

#ifdef DEBUG_X86
#ifndef FEATURE_QMI
    test_bit_main(argc, argv);

    gpsone_daemon_manager_deregister();
    gpsone_unblock_thelper(&thelper);
    gpsone_daemon_manager_unblock_task();
#endif
#endif
    gpsone_join_thelper(&thelper);
#ifdef FEATURE_QMI
    result = gpsone_bit_forward_qmi_destroy();
    if (result != 0) {
        GPSONE_DMN_PR_ERR("%s:%d]calling gpsone_bit_forward_destroy\n", __func__, __LINE__);
        return -1;
    }
#endif
    return 0;
}

