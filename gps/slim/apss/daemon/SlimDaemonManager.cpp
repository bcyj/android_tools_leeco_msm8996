/******************************************************************************
  @file:  SlimDaemonManager.cpp
  @brief: Sensor Interface Framework Daemon

  DESCRIPTION
    This file creates slim daemon that will interface with sensor clients and
    Sensor Provider (DSPS or Android API). It routes sensor data to SLIM CORE
    using a dedicated dat handling thread.

  INITIALIZATION AND SEQUENCING REQUIREMENTS

  -----------------------------------------------------------------------------
  Copyright (c) 2012-2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  -----------------------------------------------------------------------------

******************************************************************************/
/*
  * Copyright (c) 2012 Qualcomm Atheros, Inc..
  * All Rights Reserved.
  * Qualcomm Atheros Confidential and Proprietary.
  */

/******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdbool.h>

#define LOG_TAG "SlimDmn"

#include "log_util.h"
#include "gpsone_thread_helper.h"
#include "msg_q.h"
#include "loc_cfg.h"
#include "SlimDaemonManager.h"
#include "slim_utils.h"

using namespace ql_client_listener;
using namespace socket_client_listener;


#ifdef FEATURE_GSIFF_DSPS
#define DEFAULT_SENSOR_PROVIDER SENSOR1_SENSOR_PROVIDER
#else
#define DEFAULT_SENSOR_PROVIDER ANDROID_NDK_SENSOR_PROVIDER
#endif

#define SAP_CONF_FILE "/etc/sap.conf"

#ifndef GPS_CONF_FILE
#define GPS_CONF_FILE "/etc/gps.conf"
#endif

typedef struct slim_ap_control_t {
   void*                 data_msg_q;            /* Data Message Queue (memory-based) */
   struct gpsone_thelper data_task_helper;      /* Data Task thread state */
} slim_ap_control_t;

static slim_ap_control_t* g_slim_ap_control = NULL;
static uint32_t g_sensor_provider = (uint32_t)DEFAULT_SENSOR_PROVIDER;
static uint32_t g_sensor_usage = (uint32_t)0;

static loc_param_s_type conf_parameter_table[] =
{
  {"SENSOR_PROVIDER",  &g_sensor_provider, NULL,  'n'},
  {"SENSOR_USAGE",     &g_sensor_usage, NULL,     'n'},
};

pthread_mutex_t lock;
static QLClientListener *nQLClientListener = NULL;
static SocketClientListener *nSocketClientListener = NULL;

/*===========================================================================
FUNCTION   SlimDaemonOpenConnectionRequestHandler

DESCRIPTION
   Handler for opening connection for sensor clients.

   Note: Function called from the data task thread context opening client
   connection may take a while complete.

===========================================================================*/
static int SlimDaemonOpenConnectionRequestHandler(slim_OpenTxnStructType* pOpenRequest)
{
   LOC_LOGV("%s: Recv Sensor open request for client-%p"
            ,__FUNCTION__,(pOpenRequest->p_Handle));

   //pOpenRequest->fn_Callback = NotifyDataCallback;//TODO remove
   slim_CoreOpenClientConnection(pOpenRequest);

   return 0;
}

/*===========================================================================
FUNCTION    SlimDaemonCloseConnectionRequestHandler

DESCRIPTION
   Handler for opening connection for sensor clients.

   Note: Function called from the data task thread context closing client
   connection may take a while complete.

===========================================================================*/
static int SlimDaemonCloseConnectionRequestHandler(slim_GenericTxnStructType* pCloseRequest)
{

   LOC_LOGV("%s: Recv Sensor open request for client-%p"
            ,__FUNCTION__,pCloseRequest->z_TxnData.p_Handle);

   slim_CoreCloseClientConnection(&pCloseRequest->z_TxnData.p_Handle);

   return 0;
}

/*===========================================================================
FUNCTION    SlimDaemonSensorDataRequestHandler

DESCRIPTION
   Handler for sensor data request received from sensor clients.

   Note: Function called from the data task thread context as starting/stopping
         sensor provider may take a while complete.

===========================================================================*/
static int SlimDaemonSensorDataRequestHandler(slim_EnableSensorDataTxnStructType* pSensorDataRequest)
{
   LOC_LOGV("%s: Recv sensor data request for sensor-%d, enable-%d, "
            "sample count-%d, report rate-%d " ,__FUNCTION__,
            pSensorDataRequest->z_Request.sensor,
            pSensorDataRequest->z_Request.enableConf.enable,
            pSensorDataRequest->z_Request.sampleCount,
            pSensorDataRequest->z_Request.reportRate);

   slim_CoreEnableSensorDataRequest(pSensorDataRequest);

   return 0;
}

/*===========================================================================
FUNCTION    SlimDaemonTimeSyncDataRequestHandler

DESCRIPTION
   Handler for Time sync data request received from sensor clients.

   Note: Function called from the data thread context as reading the sensor
         provider clock may take a while to complete.

===========================================================================*/
static int SlimDaemonTimeSyncDataRequestHandler(slim_GetProviderTimeRequestTxnStructType* pTimeRequest)
{
     LOC_LOGV("%s: Recv time sync request" ,__func__);

     slim_CoreGetProviderTimeRequest(pTimeRequest);
     return 0;
}

/*===========================================================================
FUNCTION    SlimDaemonMotionDataRequestHandler

DESCRIPTION
   Handler for motion data request received from sensor clients.

   Note: Function called from the data task thread context as starting/stopping
         sensor provider may take a while complete.

===========================================================================*/
static int SlimDaemonMotionDataRequestHandler(slim_EnableMotionDataTxnStructType* pMotionDataRequest)
{
    LOC_LOGV("%s: Recv Motion Data request with enable=%d",
             __FUNCTION__, pMotionDataRequest->z_Request.enableConf.enable);

    slim_CoreEnableMotionDataRequest(pMotionDataRequest);

    return 0;
}

/*===========================================================================
FUNCTION    SlimDaemonPedometerRequestHandler

DESCRIPTION
   Handler for pedometer request received from sensor clients.

   Note: Function called from the data task thread context as starting/stopping
         sensor provider may take a while complete.

===========================================================================*/
static int SlimDaemonPedometerRequestHandler(slim_EnablePedometerTxnStructType* pPedometerDataRequest)
{
    LOC_LOGV("%s: Recv Pedometer with enable=%d, resetStepCount_valid=%u,"
             " resetStepCount=%d, stepCountThreshold_valid=%d, stepCountThreshold=%d"
             ,__FUNCTION__,
             pPedometerDataRequest->z_Request.enableConf.enable,
             pPedometerDataRequest->z_Request.resetStepCount_valid,
             pPedometerDataRequest->z_Request.resetStepCount,
             pPedometerDataRequest->z_Request.stepCountThreshold_valid,
             pPedometerDataRequest->z_Request.stepCountThreshold);

    slim_CoreEnablePedometerRequest(pPedometerDataRequest);

    return 0;
}

/*===========================================================================
FUNCTION    SlimDaemonProviderMessageHandler

DESCRIPTION
   Handler for routing data received from sensor providers.

DEPENDENCIES
   N/A

RETURN VALUE
   < 0  : Failure
   0 >= : Success

SIDE EFFECTS
   N/A

===========================================================================*/
static int SlimDaemonProviderMessageHandler(SlimDaemonMessage* pMsg)
{

    ENTRY_LOG();

    slim_CoreHandleProviderMessage(
            pMsg->msgData.ind.eProvider,
            pMsg->msgHeader.msgType,
            sizeof(*pMsg),
            pMsg);

    return 0;
}

/*===========================================================================
FUNCTION    SlimDaemonDataTask

DESCRIPTION
   Data Task Thread used by daemon process. Reads from a data message queue
   (memory-based)and calls appropriate message handlers based on received
   message. Thread runs forever unless killed by user. Used for low latency
   operations.

===========================================================================*/
static int SlimDaemonDataTask(void* context)
{
    ENTRY_LOG();
    void* p_data_msg_q = (void*)context;
    int rv = 0;

    SlimDaemonMessage* msg_received = NULL;

    msq_q_err_type msg_q_rv = msg_q_rcv(p_data_msg_q, (void**)&msg_received);
    if ( msg_q_rv == eMSG_Q_SUCCESS && msg_received != NULL )
    {
        LOC_LOGI("%s: Handling message type = %u", __FUNCTION__, msg_received->msgHeader.msgType);

        switch ( msg_received->msgHeader.msgType )
        {
        case  SLIM_DAEMON_TIME_SYNC_DATA_REQUEST:
            rv = SlimDaemonTimeSyncDataRequestHandler(
                    &msg_received->msgData.request.data.timeRequest);
            break;
        case SLIM_DAEMON_CLIENT_OPEN_REQUEST:
            rv = SlimDaemonOpenConnectionRequestHandler(
                    &msg_received->msgData.request.data.openRequest);
            break;
        case SLIM_DAEMON_CLIENT_CLOSE_REQUEST:
            rv = SlimDaemonCloseConnectionRequestHandler(
                    &msg_received->msgData.request.data.closeRequest);
            break;
        case SLIM_DAEMON_SENSOR_DATA_REQUEST:
            rv = SlimDaemonSensorDataRequestHandler(
                    &msg_received->msgData.request.data.sensorDataRequest);
            break;
        case SLIM_DAEMON_MOTION_DATA_REQUEST:
            rv = SlimDaemonMotionDataRequestHandler(
                    &msg_received->msgData.request.data.motionDataRequest);
            break;
        case SLIM_DAEMON_PEDOMETER_REQUEST:
            rv = SlimDaemonPedometerRequestHandler(
                    &msg_received->msgData.request.data.pedometerDataRequest);
            break;
        case SLIM_DAEMON_SENSOR_DATA_INJECT:
        case SLIM_DAEMON_MOTION_DATA_INJECT:
        case SLIM_DAEMON_PEDOMETER_INJECT:
        case SLIM_DAEMON_TIME_SYNC_DATA_INJECT:
        case SLIM_DAEMON_GENERIC_SERVICE_RESPONSE:
            rv = SlimDaemonProviderMessageHandler(msg_received);
            break;
        default:
            LOC_LOGE("%s: Unexpected Message Type = %u in slim_ap_data_task msg queue",
                     __FUNCTION__, msg_received->msgHeader.msgType);
            break;
        }
    }
    else
    {
        LOC_LOGE("%s: Bad read from data message queue", __FUNCTION__);
    }

    if( msg_received != NULL )
    {
        free(msg_received);
    }

    return rv;
}

/*===========================================================================
FUNCTION    SlimDaemonDestroy

DESCRIPTION
   Destroys resources used by the SLIM AP daemon
   process. If any of these fail an error message is printed. However, all
   memory will be deallocated.

===========================================================================*/
static int SlimDaemonDestroy(void)
{
    //TODO: coreDeinit should call this function as well as respective provider
    //deinit/destroy functions
    ENTRY_LOG();

    if( g_slim_ap_control != NULL )
    {
        /* Remove data message queue */
        int msg_rc = msg_q_destroy(&g_slim_ap_control->data_msg_q);
        if( msg_rc != eMSG_Q_SUCCESS )
        {
            LOC_LOGE("%s: Could not destroy data msg queue rc = %d errno = %d!", __FUNCTION__, msg_rc, errno);
        }

        /* Free memory resources */
        free(g_slim_ap_control);
        g_slim_ap_control = NULL;
    }

    // TODO: provider deinitialization through SLIM core
#ifdef FEATURE_GSIFF_DSPS
    slim_sensor1_close();
#endif
    return 0;
}

/*===========================================================================
FUNCTION    SlimDaemonInit

DESCRIPTION
   Initializes sensor clients, and resources used by the SLIM_DAEMON
   process. If any of these fail the process exits with an error code.

===========================================================================*/
static int SlimDaemonInit(e_sensor_provider_type sensor_provider)
{
    ENTRY_LOG();
    slimServiceProviderEnumT defaultProvider;

    /* Fill with default values */
    g_slim_ap_control = (slim_ap_control_t*)calloc(1, sizeof(*g_slim_ap_control));
    if( NULL == g_slim_ap_control )
    {
        LOC_LOGE("%s: Error: calloc error", __FUNCTION__);
        return -1;
    }
    //Initialize the data messgae queue
    if( msg_q_init(&g_slim_ap_control->data_msg_q) != eMSG_Q_SUCCESS )
    {
        LOC_LOGE("%s: Unable to initialize Data Message Queue!", __FUNCTION__);
        SlimDaemonDestroy();
        return -1;
    }

    slim_message_queue_init(g_slim_ap_control->data_msg_q);

    slim_CoreInit();//Will call sensor provider init functions

    if(SENSOR1_SENSOR_PROVIDER  == sensor_provider)
    {
        defaultProvider = eSLIM_SERVICE_PROVIDER_SSC;

    }
    else
    {
        defaultProvider = eSLIM_SERVICE_PROVIDER_NATIVE;
    }

    //Initialize client listeners
    if(nQLClientListener == NULL)
    {
        LOC_LOGD("%s: Creating nQLClientListener!", __FUNCTION__);
        nQLClientListener = QLClientListener::get(g_slim_ap_control->data_msg_q,defaultProvider);
    }
    if(nSocketClientListener == NULL)
    {
        LOC_LOGD("%s: Creating nSocketClientListener!", __FUNCTION__);
        nSocketClientListener = SocketClientListener::get(g_slim_ap_control->data_msg_q);
    }

    return 0;
}


/*===========================================================================
FUNCTION    main

DESCRIPTION
   Startup of daemon process. Simply initializes daemon and starts up data
   thread to process message queue requests. Then waits until data thread
   exits (which should be never).

========================================================*/
int main(int argc, char *argv[])
{
    ENTRY_LOG();

    SLIM_UNUSED(argc);
    SLIM_UNUSED(argv);

    /* Read logging configuration and sensor provider */
    UTIL_READ_CONF(SAP_CONF_FILE, conf_parameter_table);
    UTIL_READ_CONF_DEFAULT(GPS_CONF_FILE);

    //TODO: we don't need to set the default provider based on sap.conf anymore.
    //It was always meant for SAP client, SLIM_AP serves more than that now. TBD

    /* If a bad sensor provider is given then revert to the default. */
    if( g_sensor_provider <= MIN_SENSOR_PROVIDER || g_sensor_provider >= MAX_SENSOR_PROVIDER )
    {
        g_sensor_provider = (uint32_t)DEFAULT_SENSOR_PROVIDER;
    }

    LOC_LOGI("%s:g_sensor_provider is %d", __func__, g_sensor_provider);

    bool sensor1Found = false;

#ifdef FEATURE_GSIFF_DSPS
    if ((e_sensor_provider_type)g_sensor_provider == SENSOR1_SENSOR_PROVIDER &&
            slim_sensor1_open())
    {
        sensor1Found = true;
    }
#endif

    if (!sensor1Found)
    {
        LOC_LOGI("%s: Fall back to Android NDK as Sensor Core is not on this target(FEATURE_GSIFF_DSPS)!", __FUNCTION__);
        g_sensor_provider = (uint32_t)ANDROID_NDK_SENSOR_PROVIDER;
    }

    /* Initialize module structures */
    if( SlimDaemonInit((e_sensor_provider_type)g_sensor_provider) < 0 )
    {
        LOC_LOGE("%s: Unable to initialize SLIM_DAEMON!", __FUNCTION__);
        return -1;
    }

    /* Create task to listen for message queue updates */
    gpsone_launch_thelper(&g_slim_ap_control->data_task_helper,
                          NULL,            /* Initialize func */
                          NULL,            /* Pre-Process func */
                          SlimDaemonDataTask, /* Process func */
                          NULL,            /* Post-Process func */
                          g_slim_ap_control->data_msg_q);

    /* Wait until the Data Task exits. This should be never */
    gpsone_join_thelper(&g_slim_ap_control->data_task_helper);

    /* Clean up all resources */
    SlimDaemonDestroy();
    return 0;
}
