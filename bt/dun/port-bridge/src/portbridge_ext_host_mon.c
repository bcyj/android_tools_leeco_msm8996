/******************************************************************************

  @file    portbridge_ext_host_mon.c
  @brief   External Host Monitor

  DESCRIPTION
  This module monitors to listen if the external host is connected or not.

 ******************************************************************************/
/*===========================================================================

  Copyright (c) 2010,2012-2013 Qualcomm Technologies, Inc. All Rights Reserved

  Qualcomm Technologies Proprietary

  Export of this technology or software is regulated by the U.S. Government.
  Diversion contrary to U.S. law prohibited.

  All ideas, data and information contained in or disclosed by
  this document are confidential and proprietary information of
  Qualcomm Technologies, Inc. and all rights therein are expressly reserved.
  By accepting this material the recipient agrees that this material
  and the information contained therein are held in confidence and in
  trust and will not be used, copied, reproduced in whole or in part,
  nor its contents revealed in any manner to others without the express
  written permission of Qualcomm Technologies, Inc.

  ===========================================================================*/
/*Including necessary system header files*/
#include <sys/socket.h>
#include <linux/rtnetlink.h>
#include <net/if.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <pthread.h>
#include <termios.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <stdlib.h>

/*Port-bridge specific Header File Inclusions*/
#include "portbridge_common.h"
#include "portbridge_core.h"
#include "portbridge_ext_host_mon.h"
#include "portbridge_core_xfer.h"


/*The number of bytes to be read from usb netlink socket*/
#define BUF_LEN          1024
#undef LOGD
#undef LOGE
#undef LOGI
#undef LOGV

#define LOGD ALOGD
#define LOGI ALOGI
#define LOGE ALOGE
#define LOGV ALOGV


/*Thread variable for conn monitoring*/
static pthread_t pb_conn_mon_thread;
static pthread_t pb_dun_server_thread;

static int conn_mon_exit;

static int disconnect_from_server = FALSE;
/* Mutex for dun server and conn mon thread synchronization */
pthread_mutex_t signal_mutex;
pthread_mutex_t sock_mutex;
pthread_cond_t signal_cv;
/*===========================================================================

FUNCTION     : pb_ext_hos_mon_thread_exit_handler

DESCRIPTION  : This function does external host monitor thread exit handling

DEPENDENCIES : None

RETURN VALUE : None

============================================================================*/
static void pb_thread_exit_handler(int sig)
{
    if(sig == SIGUSR1)
        pthread_exit(0);

    port_log_err("Error in USB monitor exit handler! Sig value not SIGUSR1");

    return;
}

static int pb_dun_server_start(const char *name)
{
    int listen_sk;
    int conn_sk;
    struct sockaddr_un ra;
    int length = sizeof(struct sockaddr_un);
    int ret_value  = 0;

    while (1) {
        listen_sk = socket(AF_LOCAL, SOCK_STREAM, 0);
        if (listen_sk < 0) {
            port_log_err("Failed to get Local socket");
            ret_value =  -1;
            break;
        }

        port_log_high("Listing Socket %d opened. %s", listen_sk, __func__);

        if(socket_local_server_bind(listen_sk, name, ANDROID_SOCKET_NAMESPACE_ABSTRACT) < 0)
        {
            port_log_err("Failed to bind Local socket");
            ret_value = -1;
            break;
        }

        port_log_high("Listening Socket %d bound. %s", listen_sk, __func__);
        if(listen(listen_sk, DUN_NUM_CONNECTIONS) < 0)
        {
            port_log_err("Failed to listen on Local socket");
            ret_value = -1;
            break;
        }

        port_log_high("Listening on Socket %d for %d connections. %s",
                listen_sk, DUN_NUM_CONNECTIONS, __func__);

        conn_sk = accept(listen_sk, (struct sockaddr *)&ra, &length);

        if (conn_sk < 0) {
            port_log_dflt("conn_sk is not avilable %s", strerror(errno));
            if (errno == EINTR) {
                close(listen_sk);
                continue;
            } else {
                ret_value = -1;
                break;
            }
        }
        port_log_dflt("Accepted incoming DUN connection %d", conn_sk);
        ret_value = conn_sk;
        break;
    }

    if(listen_sk >= 0)
        close(listen_sk);

    return ret_value;
}

void disconnect_dun()
{
    port_log_high("-> %s", __func__);
    disconnect_from_server = TRUE;
    pthread_mutex_lock(&signal_mutex);
    pthread_cond_signal(&signal_cv);
    pthread_mutex_unlock(&signal_mutex);
    port_log_high("<-%s", __func__);

    return;
}

void close_socket(int *sock)
{
    pthread_mutex_lock(&sock_mutex);
    if(*sock >= 0) {
        if(shutdown(*sock,SHUT_RDWR)) {
            port_log_err(" Socket shutdown failed %s\n", __func__);
        }
        if(!close(*sock)) {
            *sock = INVALID_SOCKET;
        }
        else {
            port_log_err(" Socket close failed %s\n",__func__);
        }
    }
    pthread_mutex_unlock(&sock_mutex);
}

static void *pb_dun_server(void *arg)
{
    struct sigaction actions;
    int authorized = 0, device_id = 0;
    dun_portparams_s *pportparams = (dun_portparams_s *) arg;

    port_log_high("DUN Server thread spawned %s", __func__);

    memset(&actions, 0, sizeof(actions));
    sigemptyset(&actions.sa_mask);
    actions.sa_flags = 0;
    actions.sa_handler = pb_thread_exit_handler;

    if (sigaction(SIGUSR1, &actions, NULL) < 0) {
        port_log_err("Error in sigaction in %s:  %s\n", __func__, strerror(errno));
        pthread_exit(NULL);
    }

    pthread_mutex_init(&signal_mutex, NULL);
    pthread_mutex_init(&sock_mutex, NULL);
    pthread_cond_init(&signal_cv, NULL);

    while (1) {

        pthread_mutex_lock(&signal_mutex);


        port_log_high("pb_dun_server_start called %s", __func__);
        pportparams->conn_sk = pb_dun_server_start(DUN_SERVER);

        if (pportparams->conn_sk < 0) {
            port_log_err("Failed to start DUN sevrer:");
            pthread_mutex_unlock(&signal_mutex);
            break;
        }
        post_ext_host_event_to_core(DUN_EVENT_EXT_HOST_CON);
        pthread_cond_wait(&signal_cv, &signal_mutex);
        pthread_mutex_unlock(&signal_mutex);
        port_log_high("pb_dun_server_start  close_socket conn_sk %s", __func__);
        close_socket(&pportparams->conn_sk);
    }

    pthread_mutex_destroy(&signal_mutex);
    pthread_mutex_destroy(&sock_mutex);
    pthread_cond_destroy(&signal_cv);

    pthread_exit(NULL);

    return NULL;
}

/*===========================================================================

FUNCTION     : pb_start_conn_mon_thread

DESCRIPTION  : Starts USB monitoring thread

DEPENDENCIES : None

RETURN VALUE : Operation status: failure (-1) / success (0)

============================================================================*/

int pb_start_conn_mon_thread(void)
{
    if (pthread_create(&pb_dun_server_thread, NULL, pb_dun_server,
                (void *)&pb_dun_portparams) != 0) {
        port_log_err("Unable to create dun server thread : %s\n",
                strerror(errno));

        return -1;
    }

    return 0;
}

/*===========================================================================

FUNCTION     : pb_stop_conn_mon_thread

DESCRIPTION  : Kills USB monitoring thread

DEPENDENCIES : None

RETURN VALUE : None

============================================================================*/

void pb_stop_conn_mon_thread(void)
{
    int status;

    if((status = pthread_kill(pb_dun_server_thread, SIGUSR1)) != 0) {
        port_log_err("Error cancelling thread %d, error = %d (%s)",
                (int)pb_dun_server_thread, status, strerror(status));
    }

    if((status = pthread_join(pb_dun_server_thread, NULL)) != 0) {
        port_log_err("Error joining thread %d, error = %d (%s)",
                (int)pb_dun_server_thread, status, strerror(status));
    }
}
