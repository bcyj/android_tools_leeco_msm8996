/******************************************************************************

  @file    platform_call_arb_SM.c
  @brief   Platform Call Arbitration State Machine Module

  DESCRIPTION
  Implememtation of the Platform Arbitration State Machine.
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
/*Including necessary header files*/
#include <pthread.h>
#include <errno.h>
#include <sys/stat.h>

/*Header File Declarations*/
#include "portbridge_common.h"
#include "portbridge_core.h"
#include "platform_call_arb.h"
#include "platform_call_arb_kevents.h"

#undef LOGD
#undef LOGE
#undef LOGI
#undef LOGV

#define LOGD ALOGD
#define LOGI ALOGI
#define LOGE ALOGE
#define LOGV ALOGV


/*The pipe for the events posted by the rmnet to the platform*/
static int pb_rmnet_to_platform_pipe[2];
/*The pipe for the core events posted to the platform*/
static int pb_core_to_platform_pipe[2];

/*The mutexes associated with the above two pipes*/
static pthread_mutex_t post_platform_event_mutex;
static pthread_mutex_t post_ipc_plat_event_mutex;

/*The Platform state variable*/
EMB_DATA_CALL_STATE_S pb_platform_state;
RMNETSTATE_EVENT_E initial_rmnet_state;

/*The thread running the Platform State Machine*/
static pthread_t pb_PLATFORM_SM_thread;

/*Platform State Machine - Debugging Purposes*/
char *PLATFORM_STATE_STR[EMB_STATE_MAX] = {
    "EMB_DATA_CALL_ERROR",
    "EMB_DATA_CALL_IDLE",
    "EMB_DATA_CALL_CONNECTING",
    "EMB_DATA_CALL_CONNECTED",
    "EMB_DATA_CALL_DISCONNECTING",
    "EMB_DATA_CALL_RECONNECTING"
};

/*Platform Events - Debugging Purposes*/
char *PLATFORM_EVENT_STR[PLATFORM_EVENT_MAX] = {
    "PLATFORM_EVENT_ERROR",
    "PLATFORM_EVENT_RMNET_UP",
    "PLATFORM_EVENT_RMNET_DOWN",
    "PLATFORM_EVENT_DUN_INITIATED",
    "PLATFORM_EVENT_DUN_TERMINATED"
};

/*===========================================================================

FUNCTION     : post_core_event_to_platform

DESCRIPTION  : This is used to write the cire event to the platform (IPC) pipe

DEPENDENCIES : None

RETURN VALUE : None

============================================================================*/
void post_core_event_to_platform(int event)
{
    ipc_event_msg_m msg;

    msg.event.p_event = event;

    pthread_mutex_lock (&post_ipc_plat_event_mutex);

    port_log_low("Posting Core to Platform Event : %s in state %s",
            PLATFORM_EVENT_STR[msg.event.p_event],
            PLATFORM_STATE_STR[pb_platform_state]);
    if(write(pb_core_to_platform_pipe[WRITE_FD_FOR_PIPE], &msg, sizeof(msg)) < 0){
        port_log_err("Error while writing to IPC pipe!");
    }

    pthread_mutex_unlock (&post_ipc_plat_event_mutex);
}

/*===========================================================================

FUNCTION     : post_rmnet_event_to_platform

DESCRIPTION  : Posts an rmnet event to the platform internal pipe

DEPENDENCIES : None

RETURN VALUE : None

============================================================================*/
void post_rmnet_event_to_platform (PLATFORM_EVENT_E event)
{
    platform_event_msg_m msg;

    pthread_mutex_lock (&post_platform_event_mutex);
    msg.event = event;

    port_log_low("Posting RMNET to Platform Event : %s in state %s",
            PLATFORM_EVENT_STR[msg.event],
            PLATFORM_STATE_STR[pb_platform_state]);
    if(write(pb_rmnet_to_platform_pipe[WRITE_FD_FOR_PIPE], &msg, sizeof(msg)) < 0){
        port_log_err("Error while writing to IPC pipe!");
    }
    pthread_mutex_unlock (&post_platform_event_mutex);
}

/*===========================================================================

FUNCTION     : close_plat_pipes

DESCRIPTION  : Closes all the pipes associated with the PLAT SM

DEPENDENCIES : None

RETURN VALUE : None
============================================================================*/

static void close_plat_pipes(void)
{

    close(pb_rmnet_to_platform_pipe[READ_FD_FOR_PIPE]);
    close(pb_rmnet_to_platform_pipe[WRITE_FD_FOR_PIPE]);
    close(pb_core_to_platform_pipe[READ_FD_FOR_PIPE]);
    close(pb_core_to_platform_pipe[WRITE_FD_FOR_PIPE]);

}

/*===========================================================================

FUNCTION     : emb_data_call_process_state_idle

DESCRIPTION  : State during which the embedded call is not active

DEPENDENCIES : None

RETURN VALUE : None

============================================================================*/
static void emb_data_call_process_state_idle(platform_event_msg_m msg)
{
    port_log_low("Processing the Event : %s in the State : %s",
            PLATFORM_EVENT_STR[msg.event] ,
            PLATFORM_STATE_STR[pb_platform_state]);

    switch(msg.event){

        case PLATFORM_EVENT_DUN_INITIATED:
            post_platform_event_to_core(DUN_EVENT_READY_TO_CONNECT);
            break;

        case PLATFORM_EVENT_DUN_TERMINATED:
            pb_platform_state = EMB_DATA_CALL_CONNECTING;
            pb_enable_embedded_data_call();
            break;

        case PLATFORM_EVENT_RMNET_UP:
            pb_platform_state = EMB_DATA_CALL_CONNECTED;
            break;

        case PLATFORM_EVENT_RMNET_DOWN:

        case PLATFORM_EVENT_ERROR:
            port_log_dflt("Ignoring event %s in %s \n",PLATFORM_EVENT_STR[msg.event],
                    PLATFORM_STATE_STR[pb_platform_state]);
            break;

        default:
            port_log_dflt("Invalid event %d in %s \n", msg.event,
                    PLATFORM_STATE_STR[pb_platform_state]);
            break;
    }

    port_log_dflt("Moved to state(%s) \n", PLATFORM_STATE_STR[pb_platform_state]);
    return;
}

/*===========================================================================

FUNCTION     : emb_data_call_process_state_connected

DESCRIPTION  : State during which the Embedded call is connected

DEPENDENCIES : None

RETURN VALUE : None

============================================================================*/
static void emb_data_call_process_state_connected(platform_event_msg_m msg)
{
    port_log_low("Processing the Event : %s in the State : %s",
            PLATFORM_EVENT_STR[msg.event] ,
            PLATFORM_STATE_STR[pb_platform_state]);

    switch(msg.event){

        case PLATFORM_EVENT_DUN_INITIATED:
            pb_platform_state = EMB_DATA_CALL_DISCONNECTING;
            pb_disable_embedded_data_call();
            break;

        case PLATFORM_EVENT_RMNET_DOWN:
            pb_platform_state = EMB_DATA_CALL_IDLE;
            pb_disable_embedded_data_call();
            break;

        case PLATFORM_EVENT_DUN_TERMINATED:

        case PLATFORM_EVENT_RMNET_UP:

        case PLATFORM_EVENT_ERROR:
            port_log_dflt("Ignoring event %s in %s \n",PLATFORM_EVENT_STR[msg.event],
                    PLATFORM_STATE_STR[pb_platform_state]);
            break;

        default:
            port_log_dflt("Invalid event %d in %s \n", msg.event,
                    PLATFORM_STATE_STR[pb_platform_state]);
            break;
    }

    port_log_dflt("Moved to state(%s) \n", PLATFORM_STATE_STR[pb_platform_state]);
    return;
}

/*===========================================================================

FUNCTION     : emb_data_call_process_state_connecting

DESCRIPTION  : State during which the Embedded call is being brought up

DEPENDENCIES : None

RETURN VALUE : None

============================================================================*/
static void emb_data_call_process_state_connecting(platform_event_msg_m msg)
{
    port_log_low("Processing the Event : %s in the State : %s",
            PLATFORM_EVENT_STR[msg.event] ,
            PLATFORM_STATE_STR[pb_platform_state]);

    switch(msg.event){

        case PLATFORM_EVENT_DUN_INITIATED:
            pb_platform_state = EMB_DATA_CALL_DISCONNECTING;
            pb_disable_embedded_data_call();
            break;

        case PLATFORM_EVENT_RMNET_UP:
            pb_platform_state = EMB_DATA_CALL_CONNECTED;
            break;

        case PLATFORM_EVENT_RMNET_DOWN:
            pb_platform_state = EMB_DATA_CALL_IDLE;
            break;

        case PLATFORM_EVENT_DUN_TERMINATED:

        case PLATFORM_EVENT_ERROR:
            port_log_dflt("Ignoring event %s in %s \n",PLATFORM_EVENT_STR[msg.event],
                    PLATFORM_STATE_STR[pb_platform_state]);
            break;

        default:
            port_log_dflt("Invalid event %d in %s \n", msg.event,
                    PLATFORM_STATE_STR[pb_platform_state]);
            break;
    }

    port_log_dflt("Moved to state(%s) \n", PLATFORM_STATE_STR[pb_platform_state]);
    return;
}

/*===========================================================================

FUNCTION     : emb_data_call_process_state_disconnecting

DESCRIPTION  : State during which the Embedded call needs to be
in-activated to set up DUN call

DEPENDENCIES : None

RETURN VALUE : None

============================================================================*/
static void emb_data_call_process_state_disconnecting(platform_event_msg_m msg)
{

    port_log_low("Processing the Event : %s in the State : %s",
            PLATFORM_EVENT_STR[msg.event] ,
            PLATFORM_STATE_STR[pb_platform_state]);

    switch(msg.event){

        case PLATFORM_EVENT_DUN_TERMINATED:
            pb_platform_state = EMB_DATA_CALL_RECONNECTING;
            break;

        case PLATFORM_EVENT_RMNET_DOWN:
            pb_platform_state = EMB_DATA_CALL_IDLE;
            post_platform_event_to_core(DUN_EVENT_READY_TO_CONNECT);
            break;

        case PLATFORM_EVENT_RMNET_UP:

        case PLATFORM_EVENT_DUN_INITIATED:

        case PLATFORM_EVENT_ERROR:
            port_log_dflt("Ignoring event %s in %s \n",PLATFORM_EVENT_STR[msg.event],
                    PLATFORM_STATE_STR[pb_platform_state]);
            break;

        default:
            port_log_dflt("Invalid event %d in %s \n", msg.event,
                    PLATFORM_STATE_STR[pb_platform_state]);
            break;
    }

    port_log_dflt("Moved to state(%s) \n", PLATFORM_STATE_STR[pb_platform_state]);
    return;
}


/*===========================================================================

FUNCTION     : emb_data_call_process_state_reconnecting

DESCRIPTION  : State where DUN call terminated before RMNET_was brought down,
and hence Embedded Call needs to be brought up again

DEPENDENCIES : None

RETURN VALUE : None

============================================================================*/
static void emb_data_call_process_state_reconnecting(platform_event_msg_m msg)
{
    port_log_low("Processing the Event : %s in the State : %s",
            PLATFORM_EVENT_STR[msg.event] ,
            PLATFORM_STATE_STR[pb_platform_state]);

    switch(msg.event){

        case PLATFORM_EVENT_DUN_INITIATED:
            pb_platform_state = EMB_DATA_CALL_DISCONNECTING;
            break;

        case PLATFORM_EVENT_RMNET_UP:
            pb_platform_state =  EMB_DATA_CALL_CONNECTED;
            break;

        case PLATFORM_EVENT_RMNET_DOWN:
            pb_platform_state = EMB_DATA_CALL_CONNECTING;
            pb_enable_embedded_data_call();
            break;

        case PLATFORM_EVENT_DUN_TERMINATED:

        case PLATFORM_EVENT_ERROR:
            port_log_dflt("Ignoring event %s in %s \n",PLATFORM_EVENT_STR[msg.event],
                    PLATFORM_STATE_STR[pb_platform_state]);
            break;

        default:
            port_log_dflt("Invalid event %d in %s \n", msg.event,
                    PLATFORM_STATE_STR[pb_platform_state]);
            break;
    }

    port_log_dflt("Moved to state(%s) \n", PLATFORM_STATE_STR[pb_platform_state]);
    return;
}



/*===========================================================================

FUNCTION     : process_platform_event

DESCRIPTION  : Process the Platform Service Events

DEPENDENCIES : None

RETURN VALUE : None

============================================================================*/
void process_platform_event(platform_event_msg_m msg)
{
    port_log_low("Current platform state is  %s ", PLATFORM_STATE_STR[pb_platform_state]);

    if (msg.event >= PLATFORM_EVENT_MAX) {
        port_log_err("Invalid event in Platform State Machine :%d", msg.event);
        return;
    }

    switch(pb_platform_state) {

        case EMB_DATA_CALL_IDLE:
            emb_data_call_process_state_idle(msg);
            break;

        case EMB_DATA_CALL_CONNECTING:
            emb_data_call_process_state_connecting(msg);
            break;

        case EMB_DATA_CALL_CONNECTED:
            emb_data_call_process_state_connected(msg);
            break;

        case EMB_DATA_CALL_DISCONNECTING:
            emb_data_call_process_state_disconnecting(msg);
            break;

        case EMB_DATA_CALL_RECONNECTING:
            emb_data_call_process_state_reconnecting(msg);
            break;

        default:
            port_log_dflt("Platform State Machine in invalid state: %d\n",
                    pb_platform_state);
            break;
    }
    return;
}


/*===========================================================================

FUNCTION     : main_core_sm

DESCRIPTION  : The thread which monitors the two pipes corresponding to the
PLATFORM events and passes them onto the SM.

DEPENDENCIES : None

RETURN VALUE : void function pointer to the PLATFORM SM thread

============================================================================*/

static void *main_platform_sm()
{
    platform_event_msg_m platform_event_msg;
    ipc_event_msg_m ipc_event_msg;
    fd_set readset;
    int select_result;
    int max = 0;

    port_log_high("Platform SM thread spawned");

    /*Initial Platform system state*/
    if (RMNETSTATE_UP == pb_rmnet_interface_check_status()) {
        pb_platform_state = EMB_DATA_CALL_CONNECTED;
    }
    else {
        pb_platform_state = EMB_DATA_CALL_IDLE;
    }


    /*Create Pipe for inter-process Pipe communications*/
    if (0 > pipe_lock(pb_core_to_platform_pipe)) {
        port_log_err("Error : Failure in creating an interprocess pipe %s\n",
                strerror(errno));
        return (void *)-1;
    }

    /* Create event pipe for Platform Service */
    if(0 > pipe_lock(pb_rmnet_to_platform_pipe)) {
        port_log_err("Error : While creating pipe for IPC : %s \n",
                strerror(errno));
        /*Close pipe created for the dun events*/
        close(pb_core_to_platform_pipe[READ_FD_FOR_PIPE]);
        close(pb_core_to_platform_pipe[WRITE_FD_FOR_PIPE]);
        return (void *)-1;
    }

    while(1) {

        FD_ZERO(&readset);
        FD_SET(pb_rmnet_to_platform_pipe[READ_FD_FOR_PIPE], &readset);
        FD_SET(pb_core_to_platform_pipe[READ_FD_FOR_PIPE], &readset);

        if ( max < pb_rmnet_to_platform_pipe[READ_FD_FOR_PIPE]) {
            max = pb_rmnet_to_platform_pipe[READ_FD_FOR_PIPE];
        }

        if ( max < pb_core_to_platform_pipe[READ_FD_FOR_PIPE]) {
            max = pb_core_to_platform_pipe[READ_FD_FOR_PIPE];
        }

        select_result = select(max + 1, &readset, NULL, NULL, NULL);

        if ( select_result > 0) {
            /*Read from the platform events pipe*/
            if (FD_ISSET(pb_rmnet_to_platform_pipe[READ_FD_FOR_PIPE], &readset)) {
                if( read(pb_rmnet_to_platform_pipe[READ_FD_FOR_PIPE], &platform_event_msg,
                            sizeof(platform_event_msg)) != sizeof(platform_event_msg)) {
                    port_log_err("Error while reading the message from platform pipe : %s\n",
                            strerror(errno));
                    close_plat_pipes();
                    return (void *)-1;
                }
                process_platform_event(platform_event_msg);
            }

            /*Read from the inter-module(process) events pipe*/
            if (FD_ISSET(pb_core_to_platform_pipe[READ_FD_FOR_PIPE], &readset)) {
                if( read(pb_core_to_platform_pipe[READ_FD_FOR_PIPE], &platform_event_msg,
                            sizeof(platform_event_msg)) != sizeof(platform_event_msg)) {
                    port_log_err("Error while reading the message from inter-process pipe : %s\n",
                            strerror(errno));
                    close_plat_pipes();
                    return (void *)-1;
                }
                process_platform_event(platform_event_msg);
            }
        }
        else {
            port_log_err("Error while reading sockets using select!");
            close_plat_pipes();
            return (void *)-1;
        }
    }
}


/*===========================================================================

FUNCTION     : start_platform_sm_thread

DESCRIPTION  : The function which spawns of the PLATFORM SM thread

DEPENDENCIES : None

RETURN VALUE : Operation status: failure (-1) / success (0)
============================================================================*/

int start_platform_sm_thread(void)
{
    if( pthread_create(&pb_PLATFORM_SM_thread, NULL,
                main_platform_sm,(void *)NULL) != 0) {
        port_log_err("Unable to create PLAT SM thread : %s\n", strerror(errno));
        return -1;
    }
    return 0;
}

/*===========================================================================

FUNCTION     : stop_platform_sm_thread

DESCRIPTION  : The function which kills the PLATFORM SM thread.

DEPENDENCIES : None

RETURN VALUE : None
============================================================================*/

void stop_platform_sm_thread(void)
{
    int status;

    close_plat_pipes();

    if((status = pthread_kill(pb_PLATFORM_SM_thread, SIGUSR1)) != 0) {
        port_log_err("Error cancelling thread %d, error = %d (%s)",
                (int)pb_PLATFORM_SM_thread, status, strerror(status));
    }

    if((status = pthread_join(pb_PLATFORM_SM_thread, NULL)) != 0) {
        port_log_err("Error joining thread %d, error = %d (%s)",
                (int)pb_PLATFORM_SM_thread, status, strerror(status));
    }
    /* Delay for graceful termination
       of data connections in case of 1x and evdo*/
    sleep(3);
    pb_enable_embedded_data_call();
}

/*===========================================================================

FUNCTION     : join_plat_thread

DESCRIPTION  : The function which joins the PLATFORM SM thread to the main thread

DEPENDENCIES : None

RETURN VALUE : None
============================================================================*/

void join_plat_thread(void)
{
    int status;

    close_plat_pipes();

    if((status = pthread_join(pb_PLATFORM_SM_thread, NULL)) != 0) {
        port_log_err("Error joining thread %d, error = %d (%s)",
                (int)pb_PLATFORM_SM_thread, status, strerror(status));
    }
}










