/******************************************************************************
  @file:  xtra_system_interface.c
  @brief: XTRA main thread and interface functions.

  DESCRIPTION

  XTRA Main thread and interface functions

  -----------------------------------------------------------------------------
  Copyright (c) 2013-2014 Qualcomm Technology Incoporated.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  -----------------------------------------------------------------------------
 ******************************************************************************/

#ifdef __cplusplus
extern "C"{
#endif

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/stat.h>
#include <string.h>
#include "xtra_system_interface.h"
#include "xtra.h"
#include "xtra_defines.h"
#include "xtra_log_api.h"
#include "log_util.h"
#include "platform_lib_includes.h"
#include "msg_q.h"

#ifdef USE_GLIB
#define strlcpy g_strlcpy
#endif /* USE_GLIB */

#define LOG_TAG "XTRA_CLIENT"

#define XTRA_MSG_SIZE 2

/* globals */
globals_t globals;

/* Thread id of the main message queue poller thread */
pthread_t g_thread_id;

/* Thread attributes */
pthread_attr_t g_thread_attr;


/** Synchronization mechanism **/
static pthread_mutex_t q_wait_mutex;

/** Message queue handle **/
static void * g_handle;

/** Whether system is initialized or not **/
static int isSystemInitialized = 0;

/** Local instance of registered call backs **/
XtraClientDataCallbacksType sLocalDataCallbacks = {NULL};
XtraClientTimeCallbacksType sLocalTimeCallbacks = {NULL};

/** Declarations **/
static int xtra_init(XtraClientDataCallbacksType * dataCallbacks, XtraClientTimeCallbacksType * timeCallbacks, XtraClientConfigType *pConfig);
static void stop();
static int on_xtra_data_request(void);
static int on_xtra_time_request(void);
static int on_stop_internal_request(void);

/** Local instance of this type **/
static const XtraClientInterfaceType sXtraClientInterface = {

    sizeof(XtraClientInterfaceType),
    xtra_init,
    stop,
    on_xtra_data_request,
    on_xtra_time_request
};

/** return the local instance to the requester **/
XtraClientInterfaceType const * get_xtra_client_interface() {
    return &sXtraClientInterface;
}

static void mutex_init() {
    pthread_mutex_init(&q_wait_mutex,NULL);
    XTRA_TRACE( "mutex_init: Mutexes Initialized \n");
}

static void mutex_destroy() {
    pthread_mutex_destroy(&q_wait_mutex);
    XTRA_TRACE( "mutex_destroy: Mutexes Destroyed \n");
}

static char* create_message(size_t sz, char *msg)
{
    char *buf = (char*)malloc(sz);
    if(buf != NULL) {
        memcpy(buf, msg, sz);
    }
    return buf;
}

static void free_message(void * msg)
{
    free(msg);
}

/** Creates message queue for incoming requests for data and time **/
static msq_q_err_type create_msg_q() {

    XTRA_TRACE( "create_msg_q: Creating Message Queue \n");

    return msg_q_init(&g_handle);
}

static msq_q_err_type destroy_msg_q() {
    XTRA_TRACE(" Destroying Message Queue \n");

    msg_q_flush(g_handle);

    return msg_q_destroy(&g_handle);
}

/** Registers callbacks for when data and time is ready **/
static int register_callbacks(XtraClientDataCallbacksType * dataCallbacks, XtraClientTimeCallbacksType * timeCallbacks) {

    XTRA_TRACE( "register_callbacks: Registering callbacks \n");
    if( (dataCallbacks != NULL) && (timeCallbacks!= NULL) )
    {
        sLocalDataCallbacks.xtraDataCallback = dataCallbacks->xtraDataCallback;
        sLocalTimeCallbacks.xtraTimeCallback = timeCallbacks->xtraTimeCallback;
    }
    return 0;
}


/*======
FUNCTION _sntpUpdate

DESCRIPTION
    Attemmpt to update time from NTP server.

RETURN VALUE
    OK
    FAIL
DEPENDENCIES

======*/

static int _sntpUpdate()
{
    int attempts=0;
    int server_number;

    //use last succesfull server first
    server_number=Xtra_GetLastSntpServer();

    //try each server only once
    while(attempts<AMOUNT_NTP_SERVERS)
    {
        if( Xtra_DownloadSntpTime(&globals.assist_time, globals.config.data.xtra_sntp_server_url[server_number]) < 0 )
        {
            XTRA_TRACE("sntp_client: _sntpUpdate() failed on server %s\n",globals.config.data.xtra_sntp_server_url[server_number]);
        }
        else
        {
            XTRA_TRACE( "sntp_client: _sntpUpdate() successfull on server %s\n",globals.config.data.xtra_sntp_server_url[server_number]);
            Xtra_SetLastSntpServer(server_number);
            return OK;
        }

        Xtra_NextServerNumber(&server_number,AMOUNT_NTP_SERVERS);
        attempts++;
    }

   XTRA_TRACE_ERROR("sntp_client: SNTP update failed on all servers\n");

   return FAIL;
}



/*======
FUNCTION _download

DESCRIPTION
    Attempt XTRA data download from available servers. Try each server once and
    start from latest succefull server first.

RETURN VALUE
    OK
    FAIL

DEPENDENCIES

======*/

static int _download()
{
    int server_number=0;
    int attempts=0;


    //use last succesfull server first
    server_number=Xtra_GetLastXtraServer();

    //try each server only once
    while(attempts<AMOUNT_XTRA_SERVERS)
    {

        if(globals.xtra_data)
        {
            free(globals.xtra_data);
            globals.xtra_data = NULL;
        }

        globals.xtra_data = Xtra_HttpGet(globals.config.data.xtra_server_url[server_number],
                                         globals.config.data.user_agent_string,
                                        &globals.xtra_datalen);

        if(globals.xtra_datalen == 0)
        {
            XTRA_TRACE("http_client: XTRA_HttpGet() failed on server %s\n",globals.config.data.xtra_server_url[server_number]);
        }
        else
        {
            //TODO: check downloaded size against real XTRA2.BIN file size
            XTRA_TRACE("http_client: XTRA_HttpGet() successfull on server %s, size=%lx\n",globals.config.data.xtra_server_url[server_number],globals.xtra_datalen);
            Xtra_SetLastXtraServer(server_number);
            return OK;
        }

        Xtra_NextServerNumber(&server_number,AMOUNT_XTRA_SERVERS);
        attempts++;
    }

   XTRA_TRACE_ERROR("http_client: XTRA_HttpGet() failed on all servers\n");

   return FAIL;
}

/** Thread to poll incoming data, time and stop messages **/
static void* xtra_msg_q_poller_thread(void *arg) {

    msq_q_err_type rv = 0;
    char *xtra_msg_buffer;
    int retval = OK;
    struct timeval t;
    int64_t timeReference = 0;
    t.tv_sec = t.tv_usec = 0;

    while(globals.bTerminate == 0) {

        /* Block until we receive a message */
        XTRA_TRACE_INFO("In the xtra_msq_q_poller_thread");
        rv = msg_q_rcv(g_handle, (void**)&xtra_msg_buffer);
        if(rv != eMSG_Q_SUCCESS) {
            XTRA_TRACE_ERROR("xtra_msg_q_poller_thread: msq_q_rcv failed \n");
            break;
        }
        XTRA_TRACE("Message received by the msg_q_poller_thread is: %s\n",xtra_msg_buffer);
        /* Check message sender's priority and signal the appropriate thread */
        switch(xtra_msg_buffer[0]) {

            case 'T':
                 /* Get time from SNTP server */
                 if(OK == _sntpUpdate())
                 {

                    if(sLocalTimeCallbacks.xtraTimeCallback != NULL)
                    {
                        gettimeofday(&t, NULL);
                        timeReference = (int64_t)((int64_t)t.tv_sec*1000 + (int64_t)t.tv_usec/1000);
                        sLocalTimeCallbacks.xtraTimeCallback(globals.assist_time.time_utc,timeReference,globals.assist_time.uncertainty);
                        timeReference = t.tv_sec = t.tv_usec = 0;
                    }

                 }
                 break;
            case 'D':
                 /* Download XTRA data */
                 if(OK == _download()) {

                    if(sLocalDataCallbacks.xtraDataCallback != NULL)
                    {
                        sLocalDataCallbacks.xtraDataCallback((char*)globals.xtra_data, (int)globals.xtra_datalen);
                    }

                 }
                 break;

             case 'S':
                 /* Cleanup and stop */
                 if(globals.xtra_data) {

                     free(globals.xtra_data);
                     globals.xtra_data = NULL;
                 }
                 globals.bTerminate = 1;
                 break;

            default:
                 /* Ignore message */
                 break;
        }
        free(xtra_msg_buffer);
    }
    pthread_exit(&retval);
    return (void*)0;
}

/** Starts all the Xtra system of threads **/
static int start_threads() {
    int rc = 0;

    XTRA_TRACE( "start_threads: Starting thread... \n");

    pthread_attr_init(&g_thread_attr);

    pthread_attr_setdetachstate(&g_thread_attr, PTHREAD_CREATE_JOINABLE);

    /* create the message poller thread and pass in the message queue handle */
    rc = pthread_create(&g_thread_id,NULL,xtra_msg_q_poller_thread,&g_handle);
    if(rc != 0)
    {
        XTRA_TRACE_ERROR("start_threads: pthread_create failed %d\n", rc);
        return FAIL;
    }

    XTRA_TRACE( "start_threads: Thread Started... \n");

    return OK;
}


/** Xtra System initialization **/
static int xtra_init(XtraClientDataCallbacksType * dataCallbacks, XtraClientTimeCallbacksType * timeCallbacks, XtraClientConfigType *pConfig) {

    msq_q_err_type rv = 0;
    XTRA_TRACE( "xtra_init: Xtra System Initialization Started \n");

    if(!isSystemInitialized) {

        XTRA_TRACE( "xtra_init: Xtra System Initializing ... \n");

        mutex_init();

        /* Set configuration */
        xtra_set_config(pConfig);

        /* Create Message queue to handle incoming requests */
        if((rv = create_msg_q()) != eMSG_Q_SUCCESS)
        {
            XTRA_TRACE("xtra_init: create message queue failed and returned %d\n",rv);
            mutex_destroy();
            return FAIL;
        }

        /* Start threads */
        start_threads();
        register_callbacks(dataCallbacks,timeCallbacks);
        isSystemInitialized = 1;
    }

    XTRA_TRACE( "xtra_init: Xtra System Initialization complete. \n");

    return OK;
}

static int on_xtra_data_request(void) {

    char * data_buffer;

    XTRA_TRACE( "on_xtra_data: Incoming Xtra data request ... \n");
    pthread_mutex_lock (&q_wait_mutex);

    if(!isSystemInitialized) {
        pthread_mutex_unlock(&q_wait_mutex);
        XTRA_TRACE_ERROR("on_xtra_data_request: Received data request before initializing the Xtra system \n");
        return FAIL;
    }
    data_buffer = create_message(XTRA_MSG_SIZE,"D");
    if(eMSG_Q_SUCCESS != msg_q_snd(g_handle,(void*)data_buffer,free_message)) {

        msg_q_destroy(&g_handle);
        pthread_mutex_unlock(&q_wait_mutex);
        XTRA_TRACE_ERROR("on_xtra_data_request: send message failed \n");
        return FAIL;
    }
    XTRA_TRACE("Sent Data Request %s\n",data_buffer);
    pthread_mutex_unlock(&q_wait_mutex);

    XTRA_TRACE( "on_xtra_data: Xtra Data Request Sent \n");
    return OK;
}

static int on_xtra_time_request(void) {

    char * time_buffer;

    XTRA_TRACE( "on_xtra_time: Incoming time request ... \n");
    pthread_mutex_lock (&q_wait_mutex);

    if(!isSystemInitialized) {
        pthread_mutex_unlock(&q_wait_mutex);
        XTRA_TRACE_ERROR("on_xtra_time_request: Received time request before Xtra System Initialization \n");
        return FAIL;
    }
    time_buffer = create_message(XTRA_MSG_SIZE,"T");
    if(eMSG_Q_SUCCESS != msg_q_snd(g_handle,(void*)time_buffer,free_message)) {

        msg_q_destroy(&g_handle);
        pthread_mutex_unlock(&q_wait_mutex);
        XTRA_TRACE_ERROR("on_xtra_time_request: send message failed \n");
        return FAIL;
    }
    XTRA_TRACE("Sent Time Request %s\n",time_buffer);
    pthread_mutex_unlock(&q_wait_mutex);

    XTRA_TRACE( "on_xtra_time: Time request sent... \n");

    return OK;
}

static int on_stop_internal_request(void) {

    char * stop_buffer;
    XTRA_TRACE( "on_stop_internal_request: Incoming Stop message \n");
    pthread_mutex_lock (&q_wait_mutex);

    if(!isSystemInitialized) {
        pthread_mutex_unlock(&q_wait_mutex);
        XTRA_TRACE_ERROR("on_xtra_stop_request: Received stop request before Xtra System Initialization \n");
        return FAIL;
    }
    stop_buffer = create_message(XTRA_MSG_SIZE,"S");
    if(eMSG_Q_SUCCESS != msg_q_snd(g_handle,(void*)stop_buffer,free_message)) {

        msg_q_destroy(&g_handle);
        pthread_mutex_unlock(&q_wait_mutex);
        XTRA_TRACE_ERROR("on_xtra_stop_request: send message failed \n");
        return FAIL;
    }

    XTRA_TRACE("Sent Stop Request %s\n",stop_buffer);

    pthread_mutex_unlock(&q_wait_mutex);

    XTRA_TRACE( "on_stop_internal_request: Stop message sent \n");

    return OK;
}


static void stop() {

    int retval = 0;
    XTRA_TRACE( "stop: Incoming Stop request \n");
    on_stop_internal_request();
    XTRA_TRACE("Waiting for Xtra Message Queue Poller thread to exit...");
    pthread_join(g_thread_id,&retval);
    mutex_destroy();
    destroy_msg_q();
    pthread_attr_destroy(&g_thread_attr);
    XTRA_TRACE( "stop: Xtra System Stopped gracefully and returned: %d \n",retval);
}

#ifdef __cplusplus
}
#endif
