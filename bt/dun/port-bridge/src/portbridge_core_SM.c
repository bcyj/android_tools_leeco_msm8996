/******************************************************************************

  @file    portbridge_core_SM.c
  @brief   Portbridge Core State Machine Module

  DESCRIPTION
  Implememtation of the Portbridge Core State Machine.

 ******************************************************************************/
/*===========================================================================

  Copyright (c) 2010,2012-2013  Qualcomm Technologies, Inc. All Rights Reserved

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
/*Including necessary header files*/
#include <pthread.h>
#include <errno.h>

/*Header File Declarations*/
#include "portbridge_common.h"
#include "portbridge_core.h"
#include "portbridge_ext_host_mon.h"
#include "portbridge_core_xfer.h"
#include "platform_call_arb.h"
#include "platform_call_arb_kevents.h"

#include<sys/time.h>

/*The pipe for the events posted by the ext host to core*/
static int pb_exthost_to_core_pipe[2];
/*The pipe for the events platform posts to the core*/
static int pb_platform_to_core_pipe[2];

/*The mutexes associated with the above two pipes*/
static pthread_mutex_t post_dun_event_mutex;
static pthread_mutex_t post_ipc_core_event_mutex;

/*The thread running the Core State Machine*/
static pthread_t pb_CORE_SM_thread;

/* To track the pb_start_xfer_threads is successful or not */
static boolean threads_started = FALSE;

/*The DUN state variable*/
DUN_STATE_S pb_dun_state;


/*The condition variable and associated mutex used for signalling
 * to the data thread that the AT cmd can be passed onto the modem. */
pthread_mutex_t pb_signal_ext_host_mutex;
pthread_cond_t pb_signal_ext_host;
pthread_mutex_t fd_close_mutex;

/*The time interval for which the CORE SM waits for the PLATFORM SM
 * to give the go-ahead to bring up the DUN call*/
#define WAIT_FOR_PLATFORM_EVENT 20
#undef LOGD
#undef LOGE
#undef LOGI
#undef LOGV

#define LOGD ALOGD
#define LOGI ALOGI
#define LOGE ALOGE
#define LOGV ALOGV


/*Core State Machine - Debugging Purposes*/
char *DUN_STATE_STR[DUN_STATE_MAX] = {
    "DUN_STATE_ERROR",
    "DUN_STATE_DISCONNECTED",
    "DUN_STATE_IDLE",
    "DUN_STATE_CONNECTING",
    "DUN_STATE_CONNECTED"
};

/*Core Events - Debugging Purposes*/
char *DUN_EVENT_STR[DUN_EVENT_MAX] = {
    "DUN_EVENT_ERROR",
    "DUN_EVENT_EXT_HOST_CON",
    "DUN_EVENT_EXT_HOST_DISCON",
    "DUN_EVENT_START_CALL",
    "DUN_EVENT_STOP_CALL",
    "DUN_EVENT_READY_TO_CONNECT"
};

/*===========================================================================

FUNCTION     : notify_connection_status

DESCRIPTION  : his function is used to notify the DUN profile connection
status to the DUN application layer.

DEPENDENCIES : None

RETURN VALUE : None

============================================================================*/
void notify_connection_status(int a_socket, DUN_STATE_S dun_state)
{
    dun_ipc_msg ipc_msg;
    int ret = 0;
    unsigned short int len = DUN_IPC_HEADER_SIZE + DUN_IPC_CTRL_MSG_SIZE;
    ipc_msg.msg_type = DUN_IPC_MSG_CTRL_RESPONSE;
    ipc_msg.msg_len  = DUN_IPC_CTRL_MSG_SIZE;
    if(dun_state == DUN_STATE_DISCONNECTED) {
        ipc_msg.ctrl_msg  = DUN_CRTL_MSG_DISCONNECTED_RESP;
    }
    else if(dun_state == DUN_STATE_CONNECTED) {
        ipc_msg.ctrl_msg  = DUN_CRTL_MSG_CONNECTED_RESP;
    }
    else
        return;

    ret = send (a_socket, &ipc_msg, len, 0);
    if (ret < 0) {
        port_log_err("Not able to send mesg to remote dev: %s", strerror(errno));
    }
    port_log_err("<-%s returns %d\n", __func__, ret);
}

/*===========================================================================

FUNCTION     : event_watchdog

DESCRIPTION  : This function respawns the process on not receiving a platform
event on timeout.

DEPENDENCIES : None

RETURN VALUE : None

============================================================================*/

static void event_watchdog()
{
    port_log_high("Platform Event: DUN_READY_TO_CONNECT not received before timeout!");

    /*Stop the control and data transfer threads*/
    if(threads_started == TRUE) {
        pb_stop_xfer_threads(&pb_dun_portparams);
        threads_started = FALSE;
    }

    /*Stop the USB monitoring thread*/
    pb_stop_conn_mon_thread();

    /*Stop the rmnet monitoring thread*/
    pb_stop_rmnet_mon_thread();

    /*Kill this process*/
    raise(SIGKILL);
}

/*===========================================================================

FUNCTION     : post_platform_event_to_core

DESCRIPTION  : This is used to write to the pipe used for communication
between core and platform services module. It writes to the core
SM pipe.

DEPENDENCIES : None

RETURN VALUE : None

============================================================================*/
void post_platform_event_to_core(int event)
{
    ipc_event_msg_m msg;

    msg.event.d_event = event;

    port_log_high("Posting Platform to Core Event : %s in state %s",
            DUN_EVENT_STR[msg.event.d_event],
            DUN_STATE_STR[pb_dun_state]);

    pthread_mutex_lock (&post_ipc_core_event_mutex);

    if(write(pb_platform_to_core_pipe[WRITE_FD_FOR_PIPE], &msg, sizeof(msg)) < 0){
        port_log_err("Error while writing to IPC pipe!: %s\n", strerror(errno));
    }

    pthread_mutex_unlock (&post_ipc_core_event_mutex);
}

/*===========================================================================

FUNCTION     : post_ext_host_event_to_core

DESCRIPTION  : Posts an event to the dun event pipe

DEPENDENCIES : None

RETURN VALUE : None

============================================================================*/
void post_ext_host_event_to_core(DUN_EVENT_E event)
{
    dun_event_msg_m msg;

    msg.event = event;

    port_log_high("Posting Ext Host to Core Event : %s in state: %s",
            DUN_EVENT_STR[msg.event],
            DUN_STATE_STR[pb_dun_state]);

    pthread_mutex_lock (&post_dun_event_mutex);

    if(write(pb_exthost_to_core_pipe[WRITE_FD_FOR_PIPE], &msg, sizeof(msg)) < 0){
        port_log_err("Error while writing to dun pipe!: %s\n", strerror(errno));
    }
    pthread_mutex_unlock (&post_dun_event_mutex);
}

/*===========================================================================

FUNCTION     : close_core_pipes

DESCRIPTION  : Closes all the pipes associated with the CORE SM

DEPENDENCIES : None

RETURN VALUE : None
============================================================================*/
static void close_core_pipes(void)
{
    close(pb_platform_to_core_pipe[READ_FD_FOR_PIPE]);
    close(pb_platform_to_core_pipe[WRITE_FD_FOR_PIPE]);
    close(pb_exthost_to_core_pipe[READ_FD_FOR_PIPE]);
    close(pb_exthost_to_core_pipe[WRITE_FD_FOR_PIPE]);
}


/*===========================================================================

FUNCTION:      dun_process_state_disconnected

DESCRIPTION:   State during the external host is not connected

DEPENDENCIES : None

RETURN VALUE : None

============================================================================*/
static void dun_process_state_disconnected(dun_event_msg_m msg)
{
    port_log_high("Processing the Event : %s in the State : %s",
            DUN_EVENT_STR[msg.event] ,
            DUN_STATE_STR[pb_dun_state]);

    switch(msg.event)
    {
        case DUN_EVENT_EXT_HOST_CON:
            pb_dun_state = DUN_STATE_IDLE;
            pb_init_ports(&pb_dun_portparams);
            if( !pb_start_xfer_threads(&pb_dun_portparams)) {
                threads_started = TRUE;
            }
            break;

        case DUN_EVENT_EXT_HOST_DISCON:

        case DUN_EVENT_START_CALL:

        case DUN_EVENT_STOP_CALL:

        case DUN_EVENT_READY_TO_CONNECT:

        case DUN_EVENT_ERROR:
            port_log_dflt("Ignoring event %s in %s \n", DUN_EVENT_STR[msg.event],
                    DUN_STATE_STR[pb_dun_state]);
            break;

        default:
            port_log_dflt("Invalid event %d in %s \n", msg.event,
                    DUN_STATE_STR[pb_dun_state]);
            break;
    }

    port_log_dflt("Moved to state(%s) \n", DUN_STATE_STR[pb_dun_state]);
    return;
}

/*===========================================================================

FUNCTION     : dun_process_state_idle

DESCRIPTION  : This is the initial state where there is no DUN call and
is waiting for AT command from external port

DEPENDENCIES : None

RETURN VALUE : None

============================================================================*/
static void dun_process_state_idle(dun_event_msg_m msg)
{

    port_log_high("Processing the Event : %s in the State : %s",
            DUN_EVENT_STR[msg.event] ,
            DUN_STATE_STR[pb_dun_state]);

    switch(msg.event){

        case DUN_EVENT_EXT_HOST_DISCON:
        case DUN_EVENT_ERROR:
            pb_dun_state = DUN_STATE_DISCONNECTED;
            notify_connection_status(pb_dun_portparams.conn_sk, DUN_STATE_DISCONNECTED);
            if(threads_started == TRUE) {
                pb_stop_xfer_threads(&pb_dun_portparams);
                threads_started = FALSE;
            }
            break;

        case DUN_EVENT_START_CALL:
            pb_dun_state = DUN_STATE_CONNECTING;
            post_core_event_to_platform(PLATFORM_EVENT_DUN_INITIATED);
            break;

        case DUN_EVENT_READY_TO_CONNECT:

        case DUN_EVENT_STOP_CALL:
            port_log_dflt("Ignoring event %s in %s \n", DUN_EVENT_STR[msg.event],
                DUN_STATE_STR[pb_dun_state]);
            break;

        case DUN_EVENT_EXT_HOST_CON:
            port_log_dflt("Received %s in %s state, updating the state now\n",
                DUN_EVENT_STR[msg.event], DUN_STATE_STR[pb_dun_state]);
            /* State is not correct, move to correct state now */
            post_ext_host_event_to_core(DUN_EVENT_ERROR);
            /* Send the event again after state is correct for handling */
            post_ext_host_event_to_core(DUN_EVENT_EXT_HOST_CON);
            break;

        default:
            port_log_dflt("Invalid event %d in %s \n", msg.event,
                    DUN_STATE_STR[pb_dun_state]);
            break;
    }

    port_log_dflt("Moved to state(%s)\n", DUN_STATE_STR[pb_dun_state]);
    return;
}

/*===========================================================================

FUNCTION     : dun_process_state_connecting

DESCRIPTION  : This is the state where DUN call is waiting  for the Embedded
Call to go down

DEPENDENCIES : None

RETURN VALUE : None

============================================================================*/
static void dun_process_state_connecting(dun_event_msg_m msg)
{
    int return_status;

    port_log_high("Processing the Event : %s in the State : %s",
            DUN_EVENT_STR[msg.event] ,
            DUN_STATE_STR[pb_dun_state]);

    switch(msg.event){

        case DUN_EVENT_EXT_HOST_DISCON:
        case DUN_EVENT_ERROR:
            pb_dun_state = DUN_STATE_DISCONNECTED;
            notify_connection_status(pb_dun_portparams.conn_sk, DUN_STATE_DISCONNECTED);
            if(threads_started == TRUE) {
                pb_stop_xfer_threads(&pb_dun_portparams);
                threads_started = FALSE;
            }
            post_core_event_to_platform(PLATFORM_EVENT_DUN_TERMINATED);
            break;

        case DUN_EVENT_STOP_CALL:
            pb_dun_state = DUN_STATE_IDLE;
            post_core_event_to_platform(PLATFORM_EVENT_DUN_TERMINATED);
            break;

        case DUN_EVENT_READY_TO_CONNECT:
            /*Signal the thread waiting on this event, to pass on the AT
              command to the modem*/
            pthread_mutex_lock(&pb_signal_ext_host_mutex);
            pthread_cond_signal(&pb_signal_ext_host);
            pthread_mutex_unlock(&pb_signal_ext_host_mutex);
            pb_dun_state = DUN_STATE_CONNECTED;
            notify_connection_status(pb_dun_portparams.conn_sk, DUN_STATE_CONNECTED);
            /*If throughput logging is enabled, start the thread*/
            if(THROUGHPUT_DEBUGGING) {
                pb_start_thrgpt_thread();
            }
            break;

        case DUN_EVENT_EXT_HOST_CON:

        case DUN_EVENT_START_CALL:
            port_log_dflt("Ignoring event %s in %s \n", DUN_EVENT_STR[msg.event],
                    DUN_STATE_STR[pb_dun_state]);
            break;

        default:
            port_log_dflt("Invalid event %d in %s \n", msg.event,
                    DUN_STATE_STR[pb_dun_state]);
            break;
    }

    port_log_dflt("Moved to state(%s)\n", DUN_STATE_STR[pb_dun_state]);
    return;
}


/*===========================================================================

FUNCTION     : dun_process_state_connected

DESCRIPTION  : This is the state where DUN call is active.

DEPENDENCIES : None

RETURN VALUE : None
============================================================================*/
static void dun_process_state_connected(dun_event_msg_m msg)
{
    port_log_high("Processing the Event : %s in the State : %s",
            DUN_EVENT_STR[msg.event] ,
            DUN_STATE_STR[pb_dun_state]);

    switch(msg.event){

        case DUN_EVENT_EXT_HOST_DISCON:
        case DUN_EVENT_ERROR:
            pb_dun_state = DUN_STATE_DISCONNECTED;
            if(threads_started == TRUE) {
                pb_stop_xfer_threads(&pb_dun_portparams);
                threads_started = FALSE;
            }
            post_core_event_to_platform(PLATFORM_EVENT_DUN_TERMINATED);
            notify_connection_status(pb_dun_portparams.conn_sk, DUN_STATE_DISCONNECTED);
            /*If thrgpt logging was enabled, stop thread*/
            if(THROUGHPUT_DEBUGGING) {
                pb_stop_thrgpt_thread();
            }
            break;

        case DUN_EVENT_STOP_CALL:
            pb_dun_state = DUN_STATE_IDLE;
            post_core_event_to_platform(PLATFORM_EVENT_DUN_TERMINATED);
            if(threads_started == TRUE) {
                pb_stop_xfer_threads(&pb_dun_portparams);
                threads_started = FALSE;
            }
            /*If thrgpt logging was enabled, stop thread*/
            if(THROUGHPUT_DEBUGGING) {
                pb_stop_thrgpt_thread();
            }
            break;

        case DUN_EVENT_EXT_HOST_CON:

        case DUN_EVENT_READY_TO_CONNECT:

        case DUN_EVENT_START_CALL:
            port_log_dflt("Ignoring event %s in %s \n", DUN_EVENT_STR[msg.event],
                    DUN_STATE_STR[pb_dun_state]);
            break;

        default:
            port_log_dflt("Invalid event %d in %s \n", msg.event,
                    DUN_STATE_STR[pb_dun_state]);
            break;
    }

    port_log_dflt("Moved to state(%s)\n", DUN_STATE_STR[pb_dun_state]);
    return;
}

/*===========================================================================

FUNCTION     : process_dun_event

DESCRIPTION  : Process the DUN events based on the current state.
Acts as a dispatcher routine based on the current state.

DEPENDENCIES : None

RETURN VALUE : None

============================================================================*/
void process_dun_event(dun_event_msg_m msg)
{
    port_log_high("Current core state is  %s ", DUN_STATE_STR[pb_dun_state]);

    if (msg.event >= DUN_EVENT_MAX) {
        port_log_err("Invalid event :%d\n", msg.event);
        return;

    }

    switch(pb_dun_state) {

        case DUN_STATE_DISCONNECTED:
            dun_process_state_disconnected(msg);
            break;

        case DUN_STATE_IDLE:
            dun_process_state_idle(msg);
            break;

        case DUN_STATE_CONNECTING:
            dun_process_state_connecting(msg);
            break;

        case DUN_STATE_CONNECTED:
            dun_process_state_connected(msg);
            break;

        default:
            port_log_dflt("Core State Machine in invalid state: %d \n",
                    pb_dun_state);
            break;
    }
    return;
}

/*===========================================================================

FUNCTION     : main_core_sm

DESCRIPTION  : The thread which monitors the two pipes corresponding to the
CORE events and passes them onto the SM.

DEPENDENCIES : None

RETURN VALUE : void function pointer to the CORE SM thread

============================================================================*/

static void *main_core_sm()
{

    dun_event_msg_m dun_event_msg;
    ipc_event_msg_m ipc_event_msg;
    fd_set readset;
    int select_result;
    int max = 0;

    port_log_high("Core SM thread spawned");

    /*Initial  Core system state */
    pb_dun_state = DUN_STATE_DISCONNECTED;
    pthread_mutex_init(&fd_close_mutex, NULL);
    pthread_mutex_init(&pb_signal_ext_host_mutex, NULL);
    pthread_cond_init(&pb_signal_ext_host, NULL);

    /* Create event pipe for Core Service */
    if(0 > pipe_lock(pb_exthost_to_core_pipe)) {
        port_log_err("Error :  While creating pipe for dun events : %s \n",
                strerror(errno));
        return (void *)-1;
    }

    /*Create Pipe for inter-process Pipe communications*/
    if (0 > pipe_lock(pb_platform_to_core_pipe)) {
        port_log_err("Error : Failure in creating an interprocess pipe %s\n",
                strerror(errno));
        close(pb_exthost_to_core_pipe[READ_FD_FOR_PIPE]);
        close(pb_exthost_to_core_pipe[WRITE_FD_FOR_PIPE]);
        return (void *)-1;
    }

    while(1) {

        /*Resetting the Alarm which waits for the Platform Event*/
        alarm(0);

        FD_ZERO(&readset);
        FD_SET(pb_exthost_to_core_pipe[READ_FD_FOR_PIPE], &readset);
        FD_SET(pb_platform_to_core_pipe[READ_FD_FOR_PIPE], &readset);

        /*An alarm for 20 seconds will be set when the Core SM is waiting on
         * on the Platform SM to send back the DUN_READY_TO_CONNECT event.
         */
        if(pb_dun_state == DUN_STATE_CONNECTING) {
            /*Registering an event handler SIGKILL which will clean-up code
             * before raising SIGKILL to kill the port-bridge process
             */
            signal(SIGKILL, event_watchdog);
            alarm(WAIT_FOR_PLATFORM_EVENT);
        }

        if ( max < pb_exthost_to_core_pipe[READ_FD_FOR_PIPE]) {
            max = pb_exthost_to_core_pipe[READ_FD_FOR_PIPE];
        }

        if ( max < pb_platform_to_core_pipe[READ_FD_FOR_PIPE]) {
            max = pb_platform_to_core_pipe[READ_FD_FOR_PIPE];
        }

        select_result = select(max + 1, &readset, NULL, NULL, NULL);

        if ( select_result > 0) {

            /*Read from the dun events pipe*/
            if (FD_ISSET(pb_exthost_to_core_pipe[READ_FD_FOR_PIPE], &readset)) {
                if( read(pb_exthost_to_core_pipe[READ_FD_FOR_PIPE], &dun_event_msg,
                            sizeof(dun_event_msg)) != sizeof(dun_event_msg)) {
                    port_log_err("Error reading message from core pipe : %s\n",
                            strerror(errno));
                    close_core_pipes();
                    return (void *)-1;
                }
                process_dun_event(dun_event_msg);
            }

            /*Read from the inter-module(process) events pipe for the CORE*/
            if (FD_ISSET(pb_platform_to_core_pipe[READ_FD_FOR_PIPE], &readset)) {
                if( read(pb_platform_to_core_pipe[READ_FD_FOR_PIPE], &dun_event_msg,
                            sizeof(dun_event_msg)) != sizeof(dun_event_msg)) {
                    port_log_err("Error reading message from ipc core pipe : %s\n",
                            strerror(errno));
                    close_core_pipes();
                    return (void *)-1;
                }
                process_dun_event(dun_event_msg);
            }
        }
        else {
            port_log_err("Select func error in CORE SM thread!");
            close_core_pipes();
            return (void *)-1;
        }
    }
    pthread_mutex_destroy(&fd_close_mutex);
    pthread_mutex_destroy(&pb_signal_ext_host_mutex);
    pthread_cond_destroy(&pb_signal_ext_host);


}

/*===========================================================================

FUNCTION     : start_core_sm_thread

DESCRIPTION  : The function which spawns of the CORE SM thread

DEPENDENCIES : None

RETURN VALUE : Operation status: failure (-1) / success (0)
============================================================================*/

int start_core_sm_thread(void)
{
    if( pthread_create(&pb_CORE_SM_thread, NULL,
                main_core_sm,(void *)NULL) != 0) {
        port_log_err("Unable to create CORE SM thread : %s\n", strerror(errno));
        return -1;
    }
    return 0;
}

/*===========================================================================

FUNCTION     : stop_core_sm_thread

DESCRIPTION  : The function which kills the CORE SM thread.

DEPENDENCIES : None

RETURN VALUE : None
============================================================================*/

void stop_core_sm_thread(void)
{
    int status;

    close_core_pipes();

    if((status = pthread_kill(pb_CORE_SM_thread, SIGUSR1)) != 0) {
        port_log_err("Error cancelling thread %d, error = %d (%s)",
                (int)pb_CORE_SM_thread, status, strerror(status));
    }

    if((status = pthread_join(pb_CORE_SM_thread, NULL)) != 0) {
        port_log_err("Error joining thread %d, error = %d (%s)",
                (int)pb_CORE_SM_thread, status, strerror(status));
    }
}

/*===========================================================================

FUNCTION     : join_core_thread

DESCRIPTION  : The function which joins the CORE SM thread to the main thread

DEPENDENCIES : None

RETURN VALUE : None
============================================================================*/

void join_core_thread(void)
{
    int status;

    close_core_pipes();

    if((status = pthread_join(pb_CORE_SM_thread, NULL)) != 0) {
        port_log_err("Error joining thread %d, error = %d (%s)",
                (int)pb_CORE_SM_thread, status, strerror(status));
    }
}
