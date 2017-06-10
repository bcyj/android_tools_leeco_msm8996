/******************************************************************************
  @file:  SlimSensor1Provider.c
  @brief: GNSS / Sensor Interface Framework Support

  DESCRIPTION
    This file defines the implementation for sensor provider interface
    using the Qualcomm proprietary DSPS Sensor1 API.

  INITIALIZATION AND SEQUENCING REQUIREMENTS

  -----------------------------------------------------------------------------
  Copyright (c) 2011,2014 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  -----------------------------------------------------------------------------

******************************************************************************/

/*
  * Copyright (c) 2012 Qualcomm Atheros, Inc..
  * All Rights Reserved.
  * Qualcomm Atheros Confidential and Proprietary.
  */

/*=====================================================================
                        EDIT HISTORY FOR MODULE

  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.

when       who      what, where, why
--------   ---      -------------------------------------------------------
05/01/11   jb       Initial version
08/01/11   jb       1. Misc typedef changes
                    2. Fixed bug with out-of-order sensor samples when
                       more than 1 sample is received at a time. Samples
                       are sorted into monotonically increasing order based
                       on timestamp and then processed.
10/24/12   vr       1. Adding temperature streaming.
                    2. Adding motion data streming.
                    3. Adding pedometer streaming.
======================================================================*/

#include <sys/ioctl.h>
#include <sys/time.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>

#include "sensor1.h"
#include "SlimSensor1Provider.h"
#include "SlimSensor1ProviderWrapper.h"
#include "SlimDaemonMsg.h"
#include "slim_client_types.h"

#include "log_util.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif /* LOG_TAG */
#define LOG_TAG "slim_sensor1"

#include "sns_sam_qmd_v01.h"
#include "sns_sam_amd_v01.h"
#include "sns_sam_rmd_v01.h"
#include "sns_sam_vmd_v01.h"
#include "sns_sam_ped_v01.h"
#include "sns_smgr_api_v01.h"
#include "sns_time_api_v02.h"
#include "sns_reg_api_v02.h"

#define MOTION_DATA_TLV_AGE 100
#define MOTION_DATA_TLV_TIMEOUT 65535

#define UNIT_CONVERT_Q16             (1.0 / 65536.0)
#define UNIT_CONVERT_MAGNETIC_FIELD  (100 * UNIT_CONVERT_Q16)  /* uTesla */
#define SENSOR_TYPE_UNDEFINED -1

typedef struct sensor1_sensor_state_t {
   bool                          running;            /* Is reporting currently? */
   uint32_t                      reporting_rate;     /* Current sampling rate in Hz */
   uint32_t                      batching_rate;      /* Current batching rate in Hz */
   float                         reporting_interval; /* Current sampling interval in ms */
   float                         batching_interval;  /* Current batching interval in ms */
} sensor1_sensor_state_t;

typedef struct sensor1_motion_state_t {
   bool                        running;                  /* Motion data updates coming from Sensor1? */
   uint8_t                     amd_algo_instance_id;     /* AMD algorithm instance id. Needed to disable motion data reporting */
   uint8_t                     rmd_algo_instance_id;     /* RMD algorithm instance id. Needed to disable motion data reporting */
   sns_sam_motion_state_e_v01  amd;                      /* AMD algorithm state */
   sns_sam_motion_state_e_v01  rmd;                      /* RMD algorithm state */
} sensor1_motion_state_t;

typedef struct sensor1_pedometer_state_t {
   bool                        running;                  /* Pedometer updates coming from Sensor1? */
   uint8_t                     algo_instance_id;         /* Pedometer algorithm instance id. Needed to disable motion data reporting */
   uint32_t                    step_count_threshold;     /* Step count threshold. */
   struct timeval              reset_time;               /* The latest reset time */
} sensor1_pedometer_state_t;

typedef struct sensor1_control_t {
    sensor1_handle_s*       sensor1_client_handle;    /* Opaque Sensor1 Client Handle */
    bool                    spi_running;              /* SPI updates coming from Sensor1? */
    int32_t                 spi_algo_instance_id;     /* SPI Algorithm instance id. Needed to disable SPI reporting */
    void*                   p_msg_q;                  /* Message Queue to add messages to. */
    sensor1_motion_state_t  motion_state;             /* All necessary for the motion state */
    sensor1_pedometer_state_t  pedometer_state;       /* All necessary for the pedometer */
    sensor1_sensor_state_t  gyro_state;               /* All necessary state for accel sensor */
    sensor1_sensor_state_t  accel_state;              /* All necessary state for gyro sensor */
    sensor1_sensor_state_t  baro_state;               /* All necessary state for gyro sensor */
    sensor1_sensor_state_t  gyro_temperature_state;   /* All necessary state for gyro temperature sensor */
    sensor1_sensor_state_t  accel_temperature_state;  /* All necessary state for accel temperature sensor */
    sensor1_sensor_state_t  mag_calib_state;          /* All necessary state for calib mag sensor */
    sensor1_sensor_state_t  mag_uncalib_state;        /* All necessary state for uncalib mag sensor */
} sensor1_control_t;

static sensor1_control_t* g_sensor1_control = NULL;

/* Static function declarations */
static void process_time_resp( sensor1_msg_header_s *msg_hdr, void *msg_ptr );

static sensor1_access_control_t* g_sensor1_access_control = NULL;
static sensor1_handle_s *g_p_sensor1_hndl = NULL;
static sensor1_error_e g_sensor1_open_result = SENSOR1_SUCCESS;

/*===========================================================================
FUNCTION    slim_sensor1_init_defaults

DESCRIPTION
  Initialization function for sensor1 internal state.

DEPENDENCIES
   N/A

RETURN VALUE
   N/A

SIDE EFFECTS
   N/A

===========================================================================*/
static bool slim_sensor1_init_defaults()
{
    if( g_sensor1_control != NULL )
    {
        free(g_sensor1_control);
        g_sensor1_control = NULL;
    }

    /* Fill with default values */
    g_sensor1_control = (sensor1_control_t*)calloc(1, sizeof(*g_sensor1_control));

    if( NULL == g_sensor1_control )
    {
        LOC_LOGE("%s: Error: calloc error", __FUNCTION__);
        return false;
    }

    g_sensor1_control->spi_algo_instance_id = -1;
    g_sensor1_control->p_msg_q = NULL;

    return true;
}

/*===========================================================================

  FUNCTION:   slim_sensor1_update_motion_data

  ===========================================================================*/
bool slim_sensor1_update_motion_data(bool running)
{
    sns_sam_qmd_enable_req_msg_v01* ena_amd_sam_req = NULL;
    sns_sam_qmd_disable_req_msg_v01* dis_amd_sam_req = NULL;

    void* rmd_sam_req = NULL;
    sensor1_error_e error;
    bool messages_sent = true;

    /* Message headers */
    sensor1_msg_header_s amd_req_hdr;
    sensor1_msg_header_s rmd_req_hdr;

    if( g_sensor1_control == NULL )
    {
        LOC_LOGE("%s: Initialization required to perform operation", __FUNCTION__);
        return false;
    }

    /* Turn on AMD and RMD reporting */
    if( running && !g_sensor1_control->motion_state.running )
    {
        LOC_LOGI("%s: Turning on AMD reporting", __FUNCTION__);

        amd_req_hdr.service_number = SNS_SAM_AMD_SVC_ID_V01;
        amd_req_hdr.txn_id = 0;
        amd_req_hdr.msg_id = SNS_SAM_AMD_ENABLE_REQ_V01;
        amd_req_hdr.msg_size = sizeof( sns_sam_qmd_enable_req_msg_v01 );

        error = sensor1_alloc_msg_buf( g_sensor1_control->sensor1_client_handle,
                                       sizeof(sns_sam_qmd_enable_req_msg_v01),
                                       (void**)&ena_amd_sam_req );
        if ( SENSOR1_SUCCESS != error )
        {
            LOC_LOGE("%s:%u] sensor1_alloc_msg_buf add returned %d", __FUNCTION__,__LINE__, error);
            return false;
        }

        sns_sam_qmd_enable_req_msg_v01* enable_amd_sam_req = (sns_sam_qmd_enable_req_msg_v01*)ena_amd_sam_req;
        /* Only report on new AMD events */
        enable_amd_sam_req->report_period = 0;

        /* Send across Sensor1 Interface */
        if ( (error = sensor1_write( g_sensor1_control->sensor1_client_handle, &amd_req_hdr,
                                     ena_amd_sam_req )) != SENSOR1_SUCCESS )
        {
            /* Error so we deallocate QMI message */
            sensor1_free_msg_buf( g_sensor1_control->sensor1_client_handle, ena_amd_sam_req );
            LOC_LOGE("%s:%u] sensor1_write() error: %u", __FUNCTION__,__LINE__, error );
            messages_sent = false;
            if (SENSOR1_EINVALID_CLIENT == error) {
                LOC_LOGE("sensor daemon has died. Exit slim to recover");
                exit(1);
            }
        }

        LOC_LOGI("%s: Turning on RMD reporting", __FUNCTION__);

        rmd_req_hdr.service_number = SNS_SAM_RMD_SVC_ID_V01;
        rmd_req_hdr.txn_id = 0;
        rmd_req_hdr.msg_id = SNS_SAM_RMD_ENABLE_REQ_V01;
        rmd_req_hdr.msg_size = sizeof( sns_sam_qmd_enable_req_msg_v01 );

        error = sensor1_alloc_msg_buf( g_sensor1_control->sensor1_client_handle,
                                       sizeof(sns_sam_qmd_enable_req_msg_v01),
                                       (void**)&rmd_sam_req );
        if ( SENSOR1_SUCCESS != error )
        {
            LOC_LOGE("%s:%u] sensor1_alloc_msg_buf add returned %d", __FUNCTION__,__LINE__, error);
            return false;
        }

        sns_sam_qmd_enable_req_msg_v01* enable_rmd_sam_req = (sns_sam_qmd_enable_req_msg_v01*)rmd_sam_req;
        /* Only report on new RMD events */
        enable_rmd_sam_req->report_period = 0;

        /* Send across Sensor1 Interface */
        if ( (error = sensor1_write( g_sensor1_control->sensor1_client_handle, &rmd_req_hdr,
                                     rmd_sam_req )) != SENSOR1_SUCCESS )
        {
            /* Error so we deallocate QMI message */
            sensor1_free_msg_buf( g_sensor1_control->sensor1_client_handle, rmd_sam_req );
            LOC_LOGE("%s:%u] sensor1_write() error: %u", __FUNCTION__,__LINE__, error );
            messages_sent = false;
            if (SENSOR1_EINVALID_CLIENT == error) {
                LOC_LOGE("sensor daemon has died. Exit slim to recover");
                exit(1);
            }
        }
    }
    /* Turn off AMD amd RMD reporting */
    else if( !running && g_sensor1_control->motion_state.running )
    {
        LOC_LOGI("%s: Turning off AMD reporting", __FUNCTION__);

        amd_req_hdr.service_number = SNS_SAM_AMD_SVC_ID_V01;
        amd_req_hdr.txn_id = 0;
        amd_req_hdr.msg_id = SNS_SAM_AMD_DISABLE_REQ_V01;
        amd_req_hdr.msg_size = sizeof( sns_sam_qmd_disable_req_msg_v01 );
        error = sensor1_alloc_msg_buf( g_sensor1_control->sensor1_client_handle,
                                       sizeof(sns_sam_qmd_disable_req_msg_v01),
                                       (void**)&dis_amd_sam_req );

        if ( SENSOR1_SUCCESS != error )
        {
            LOC_LOGE("%s:%u] sensor1_alloc_msg_buf add returned %d", __FUNCTION__,__LINE__, error);
            return false;
        }

        sns_sam_qmd_disable_req_msg_v01* disable_amd_sam_req = (sns_sam_qmd_disable_req_msg_v01*)dis_amd_sam_req;
        disable_amd_sam_req->instance_id = g_sensor1_control->motion_state.amd_algo_instance_id;

        /* Send across Sensor1 Interface */
        if ( (error = sensor1_write( g_sensor1_control->sensor1_client_handle, &amd_req_hdr,
                                     dis_amd_sam_req )) != SENSOR1_SUCCESS )
        {
            /* Error so we deallocate QMI message */
            sensor1_free_msg_buf( g_sensor1_control->sensor1_client_handle, dis_amd_sam_req );
            LOC_LOGE("%s:%u] sensor1_write() error: %u", __FUNCTION__,__LINE__, error );
            messages_sent = false;
            if (SENSOR1_EINVALID_CLIENT == error) {
                LOC_LOGE("sensor daemon has died. Exit slim to recover");
                exit(1);
            }
        }

        LOC_LOGI("%s: Turning off RMD reporting", __FUNCTION__);

        rmd_req_hdr.service_number = SNS_SAM_RMD_SVC_ID_V01;
        rmd_req_hdr.txn_id = 0;
        rmd_req_hdr.msg_id = SNS_SAM_RMD_DISABLE_REQ_V01;
        rmd_req_hdr.msg_size = sizeof( sns_sam_qmd_disable_req_msg_v01 );
        error = sensor1_alloc_msg_buf( g_sensor1_control->sensor1_client_handle,
                                       sizeof(sns_sam_qmd_disable_req_msg_v01),
                                       (void**)&rmd_sam_req );

        if ( SENSOR1_SUCCESS != error )
        {
            LOC_LOGE("%s:%u] sensor1_alloc_msg_buf add returned %d", __FUNCTION__,__LINE__, error);
            return false;
        }

        sns_sam_qmd_disable_req_msg_v01* disable_rmd_sam_req = (sns_sam_qmd_disable_req_msg_v01*)rmd_sam_req;
        disable_rmd_sam_req->instance_id = g_sensor1_control->motion_state.rmd_algo_instance_id;

        /* Send across Sensor1 Interface */
        if ( (error = sensor1_write( g_sensor1_control->sensor1_client_handle, &rmd_req_hdr,
                                     rmd_sam_req )) != SENSOR1_SUCCESS )
        {
            /* Error so we deallocate QMI message */
            sensor1_free_msg_buf( g_sensor1_control->sensor1_client_handle, rmd_sam_req );
            LOC_LOGE("%s:%u] sensor1_write() error: %u", __FUNCTION__,__LINE__, error );
            messages_sent = false;
            if (SENSOR1_EINVALID_CLIENT == error) {
                LOC_LOGE("sensor daemon has died. Exit slim to recover");
                exit(1);
            }
        }
    }
    /* Enabling if already enabled or disabling while already disabled. */
    else
    {
        return true;
    }

    if (messages_sent)
    {
        g_sensor1_control->motion_state.running = running;
    }
    g_sensor1_control->motion_state.amd = SNS_SAM_MOTION_UNKNOWN_V01;
    g_sensor1_control->motion_state.rmd = SNS_SAM_MOTION_UNKNOWN_V01;

    return true;
}

/*===============================================================i============

FUNCTION    slim_sensor1_update_pedometer

===========================================================================*/
bool slim_sensor1_update_pedometer(bool running, uint8_t reset_step_count, uint32_t step_count_threshold)
{
    if( g_sensor1_control == NULL )
    {
        LOC_LOGE("%s: Initialization required to perform operation", __FUNCTION__);
        return false;
    }

    void* sam_req = NULL;
    sensor1_error_e error;

    /* Message header */
    sensor1_msg_header_s sam_req_hdr;
    sam_req_hdr.service_number = SNS_SAM_PED_SVC_ID_V01;
    sam_req_hdr.txn_id = 0;

    /* Turn off Pedometer reporting */
    if( (!running && g_sensor1_control->pedometer_state.running) ||
        (running && g_sensor1_control->pedometer_state.running && step_count_threshold != g_sensor1_control->pedometer_state.step_count_threshold))
    {
        LOC_LOGI("%s: Turning off Pedometer reporting", __FUNCTION__);

        sam_req_hdr.msg_id = SNS_SAM_PED_DISABLE_REQ_V01;
        sam_req_hdr.msg_size = sizeof( sns_sam_ped_disable_req_msg_v01 );
        error = sensor1_alloc_msg_buf( g_sensor1_control->sensor1_client_handle,
                                       sizeof(sns_sam_ped_disable_req_msg_v01),
                                       (void**)&sam_req );

        if ( SENSOR1_SUCCESS != error )
        {
            LOC_LOGE("%s: sensor1_alloc_msg_buf add returned %d", __FUNCTION__, error);
            return false;
        }

        sns_sam_ped_disable_req_msg_v01* disable_ped_sam_req = (sns_sam_ped_disable_req_msg_v01*)sam_req;
        disable_ped_sam_req->instance_id = g_sensor1_control->pedometer_state.algo_instance_id;

        /* Send across Sensor1 Interface */
        if ( (error = sensor1_write( g_sensor1_control->sensor1_client_handle, &sam_req_hdr,
                                     sam_req )) != SENSOR1_SUCCESS )
        {
            /* Error so we deallocate QMI message */
            sensor1_free_msg_buf( g_sensor1_control->sensor1_client_handle, sam_req );
            LOC_LOGE("%s: sensor1_write() error: %u", __FUNCTION__, error );
            if (SENSOR1_EINVALID_CLIENT == error) {
                LOC_LOGE("sensor daemon has died. Exit slim to recover");
                exit(1);
            }
            return false;
        }
        else
        {
            g_sensor1_control->pedometer_state.running = false;
        }
    }
    /* Resetting the step count. */
    else if( running && g_sensor1_control->pedometer_state.running && reset_step_count )
    {
        LOC_LOGI("%s: Resetting the Pedometer step count.", __FUNCTION__);

        sam_req_hdr.msg_id = SNS_SAM_PED_RESET_REQ_V01;
        sam_req_hdr.msg_size = sizeof( sns_sam_ped_reset_req_msg_v01 );
        error = sensor1_alloc_msg_buf( g_sensor1_control->sensor1_client_handle,
                                       sizeof(sns_sam_ped_reset_req_msg_v01),
                                       (void**)&sam_req );

        if ( SENSOR1_SUCCESS != error )
        {
            LOC_LOGE("%s: sensor1_alloc_msg_buf add returned %d", __FUNCTION__, error);
            return false;
        }

        sns_sam_ped_reset_req_msg_v01* reset_ped_sam_req = (sns_sam_ped_reset_req_msg_v01*)sam_req;
        reset_ped_sam_req->instance_id = g_sensor1_control->pedometer_state.algo_instance_id;

        /* Send across Sensor1 Interface */
        if ( (error = sensor1_write( g_sensor1_control->sensor1_client_handle, &sam_req_hdr,
                                     sam_req )) != SENSOR1_SUCCESS )
        {
            /* Error so we deallocate QMI message */
            sensor1_free_msg_buf( g_sensor1_control->sensor1_client_handle, sam_req );
            LOC_LOGE("%s: sensor1_write() error: %u", __FUNCTION__, error );
            if (SENSOR1_EINVALID_CLIENT == error) {
                LOC_LOGE("sensor daemon has died. Exit slim to recover");
                exit(1);
            }
            return false;
        }
    }
    /* Turn on Pedometer reporting */
    if( running && !g_sensor1_control->pedometer_state.running )
    {
        LOC_LOGI("%s: Turning on Pedometer reporting", __FUNCTION__);

        sam_req_hdr.msg_id = SNS_SAM_PED_ENABLE_REQ_V01;
        sam_req_hdr.msg_size = sizeof( sns_sam_ped_enable_req_msg_v01 );
        error = sensor1_alloc_msg_buf( g_sensor1_control->sensor1_client_handle,
                                       sizeof(sns_sam_ped_enable_req_msg_v01),
                                       (void**)&sam_req );
        if ( SENSOR1_SUCCESS != error )
        {
            LOC_LOGE("%s: sensor1_alloc_msg_buf add returned %d", __FUNCTION__, error);
            return false;
        }

        sns_sam_ped_enable_req_msg_v01* enable_ped_sam_req = (sns_sam_ped_enable_req_msg_v01*)sam_req;
        /* SLIM supports only event based pedometer reports. */
        enable_ped_sam_req->report_period = 0;
        if (step_count_threshold > 0)
        {
            enable_ped_sam_req->step_count_threshold_valid = true;
            enable_ped_sam_req->step_count_threshold = step_count_threshold;
        }

        /* Send across Sensor1 Interface */
        if ( (error = sensor1_write( g_sensor1_control->sensor1_client_handle, &sam_req_hdr,
                                     sam_req )) != SENSOR1_SUCCESS )
        {
            /* Error so we deallocate QMI message */
            sensor1_free_msg_buf( g_sensor1_control->sensor1_client_handle, sam_req );
            LOC_LOGE("%s: sensor1_write() error: %u", __FUNCTION__, error );
            if (SENSOR1_EINVALID_CLIENT == error) {
                LOC_LOGE("sensor daemon has died. Exit slim to recover");
                exit(1);
            }
            return false;
        }
        else
        {
            g_sensor1_control->pedometer_state.running = running;
        }
    }

    return true;
}

/*===========================================================================
FUNCTION    update_spi_reporting_status

DESCRIPTION
  Function updates the current running status of spi updates from this sensor
  provider

  running:       TRUE - start spi reporting, FALSE - stop sensor reporting

  Note: Function should not be called from a time sensitive thread as this
        function may block for some time to start up spi reporting.

DEPENDENCIES
   N/A

RETURN VALUE
   0/FALSE : Failure
   1/TRUE  : Successful

SIDE EFFECTS
   N/A


===========================================================================*/
static bool update_spi_reporting_status(bool running)
{
    void* sam_req = NULL;
    sensor1_error_e error;

    /* Message header */
    sensor1_msg_header_s req_hdr;
    req_hdr.service_number = SNS_SAM_VMD_SVC_ID_V01;
    req_hdr.txn_id = 0;

    /* Turn on spi reporting */
    if( running && !g_sensor1_control->spi_running )
    {
        LOC_LOGI("%s: Turning on spi reporting", __FUNCTION__);

        req_hdr.msg_id = SNS_SAM_VMD_ENABLE_REQ_V01;
        req_hdr.msg_size = sizeof( sns_sam_qmd_enable_req_msg_v01 );

        error = sensor1_alloc_msg_buf( g_sensor1_control->sensor1_client_handle,
                                       sizeof(sns_sam_qmd_enable_req_msg_v01),
                                       (void**)&sam_req );
        if ( SENSOR1_SUCCESS != error )
        {
            LOC_LOGE("%s: sensor1_alloc_msg_buf add returned %d", __FUNCTION__, error);
            return false;
        }

        sns_sam_qmd_enable_req_msg_v01* enable_sam_req = (sns_sam_qmd_enable_req_msg_v01*)sam_req;
        enable_sam_req->report_period = 0;               /* Only report on new spi events */
    }
    /* Turn off spi reporting */
    else if( !running && g_sensor1_control->spi_running )
    {
        LOC_LOGI("%s: Turning off spi reporting", __FUNCTION__);

        req_hdr.msg_id = SNS_SAM_VMD_DISABLE_REQ_V01;
        req_hdr.msg_size = sizeof( sns_sam_qmd_disable_req_msg_v01 );
        error = sensor1_alloc_msg_buf( g_sensor1_control->sensor1_client_handle,
                                       sizeof(sns_sam_qmd_disable_req_msg_v01),
                                       (void**)&sam_req );

        if ( SENSOR1_SUCCESS != error )
        {
            LOC_LOGE("%s: sensor1_alloc_msg_buf add returned %d", __FUNCTION__, error);
            return false;
        }

        sns_sam_qmd_disable_req_msg_v01* disable_sam_req = (sns_sam_qmd_disable_req_msg_v01*)sam_req;
        disable_sam_req->instance_id = g_sensor1_control->spi_algo_instance_id;
    }
    /* Enabling if already enabled or disabling while already disabled. */
    else
    {
        return true;
    }

    /* Send across Sensor1 Interface */
    if ( (error = sensor1_write( g_sensor1_control->sensor1_client_handle, &req_hdr,
                                 sam_req )) != SENSOR1_SUCCESS )
    {
        /* Error so we deallocate QMI message */
        sensor1_free_msg_buf( g_sensor1_control->sensor1_client_handle, sam_req );
        LOC_LOGE("%s: sensor1_write() error: %u", __FUNCTION__, error );
        if (SENSOR1_EINVALID_CLIENT == error) {
            LOC_LOGE("sensor daemon has died. Exit slim to recover");
            exit(1);
        }
        return false;
    }
    else
    {
        g_sensor1_control->spi_running = running;
    }

    return true;
}

/*===========================================================================
FUNCTION    slim_sensor1_update_sensor_status_buffering

DESCRIPTION
  Generic function to start/stop a sensor based on provided sampling rate,
  batching rate, mounted state, and sensor information using Sensor1 Buffering API.

  running:       true - start sensor reporting, false - stop sensor reporting
  sample_count:  Batch sample count
  report_rate:   Batch reporting rate
  sensor_type:   What kind of sensor are we updating

DEPENDENCIES
   TODO: The sampling frequency is a mere suggestion to the sensor1 daemon. Sensor 1
   will stream at the requested minimum sampling frequency requested by all AP clients
   combined. So we do see cases where buffering API does not help and we get single
   sensor data for every indication. In that case should SLIM AP do the batching?

RETURN VALUE
   0/false : Failure
   1/true  : Successful

SIDE EFFECTS
   N/A

===========================================================================*/
 bool slim_sensor1_update_sensor_status_buffering
(
   bool running,
   uint32_t sample_count,
   uint32_t report_rate,
   slimServiceEnumT sensor_type
)
 {
     sensor1_sensor_state_t* sensor_state;
     uint32_t sampling_rate;

     const char* sensor_str;
     uint8_t sns1_sensor_id;
     uint8_t sns1_data_type;

     if ( g_sensor1_control == NULL )
     {
         LOC_LOGE("%s: Initialization required to perform operation", __FUNCTION__);
         return false;
     }
     //check correct? TODO:
     sampling_rate = sample_count * report_rate;
     switch (sensor_type)
     {
     case eSLIM_SERVICE_SENSOR_ACCEL:
         sensor_str = "accel";
         sns1_sensor_id = SNS_SMGR_ID_ACCEL_V01;
         sns1_data_type = SNS_SMGR_DATA_TYPE_PRIMARY_V01;
         sensor_state = &g_sensor1_control->accel_state;
         break;

     case eSLIM_SERVICE_SENSOR_GYRO:
         sensor_str = "gyro";
         sns1_sensor_id = SNS_SMGR_ID_GYRO_V01;
         sns1_data_type = SNS_SMGR_DATA_TYPE_PRIMARY_V01;
         sensor_state = &g_sensor1_control->gyro_state;
         break;

     case eSLIM_SERVICE_SENSOR_ACCEL_TEMP:
         sensor_str = "accel_temperature";
         sns1_sensor_id = SNS_SMGR_ID_ACCEL_V01;
         sns1_data_type = SNS_SMGR_DATA_TYPE_SECONDARY_V01;
         sensor_state = &g_sensor1_control->accel_temperature_state;
         break;

     case eSLIM_SERVICE_SENSOR_GYRO_TEMP:
         sensor_str = "gyro_temperature";
         sns1_sensor_id = SNS_SMGR_ID_GYRO_V01;
         sns1_data_type = SNS_SMGR_DATA_TYPE_SECONDARY_V01;
         sensor_state = &g_sensor1_control->gyro_temperature_state;
         break;

     case eSLIM_SERVICE_SENSOR_BARO:
         sensor_str = "pressure";
         sns1_sensor_id = SNS_SMGR_ID_PRESSURE_V01;
         sns1_data_type = SNS_SMGR_DATA_TYPE_PRIMARY_V01;
         sensor_state = &g_sensor1_control->baro_state;
         break;

     case eSLIM_SERVICE_SENSOR_MAG_CALIB:
         sensor_str = "mag_calib";
         sns1_sensor_id = SNS_SMGR_ID_MAG_V01;
         sns1_data_type = SNS_SMGR_DATA_TYPE_PRIMARY_V01;
         sensor_state = &g_sensor1_control->mag_calib_state;
         break;

     case eSLIM_SERVICE_SENSOR_MAG_UNCALIB:
         sensor_str = "mag_uncalib";
         sns1_sensor_id = SNS_SMGR_ID_MAG_V01;
         sns1_data_type = SNS_SMGR_DATA_TYPE_PRIMARY_V01;
         sensor_state = &g_sensor1_control->mag_uncalib_state;
         break;

     default:
         LOC_LOGE("%s: Unknown Sensor Type %d", __FUNCTION__, sensor_type);
         return false;
         break;
     }

     /* No sensor state to use. */
     if( sensor_state == NULL )
     {
         LOC_LOGE("%s: No %s sensor state provided to start/stop", __FUNCTION__, sensor_str);
         return false;
     }

     /* No State Change */
     if( !running && !sensor_state->running )
     {
         /* Sensors stay Off */
         return true;
     }
     /* Sensors stay On but no change in sensor sampling rate */
     else if ( (running && sensor_state->running) && (sampling_rate == sensor_state->reporting_rate) )
     {
         return true;
     }

     sensor1_msg_header_s req_hdr;
     sns_smgr_buffering_req_msg_v01* smgr_req = NULL;

     sensor1_error_e error = sensor1_alloc_msg_buf( g_sensor1_control->sensor1_client_handle,
                                                    sizeof(sns_smgr_buffering_req_msg_v01),
                                                    (void**)&smgr_req );
     if ( SENSOR1_SUCCESS != error )
     {
         LOC_LOGE("%s: sensor1_alloc_msg_buf add returned %d", __FUNCTION__, error);
         return false;
     }

     /* Message header */
     req_hdr.service_number = SNS_SMGR_SVC_ID_V01;
     req_hdr.msg_id = SNS_SMGR_BUFFERING_REQ_V01;
     req_hdr.msg_size = sizeof( sns_smgr_buffering_req_msg_v01 );
     req_hdr.txn_id = 0;

     if( running )
     {
         uint32_t sensor1_reporting_rate;
         SENSOR1_Q16_FROM_FLOAT( sensor1_reporting_rate, (float)report_rate );

         /* Turn on sensor */
         if ( !sensor_state->running )
         {
             LOC_LOGI("%s: Turning on %s reporting, reporting-rate %3.5f (q16 %u), sampling-rate %u",
                      __FUNCTION__, sensor_str, (float)report_rate, sensor1_reporting_rate, sampling_rate);
         }
         /* Sensor sampling rate change */
         else if ( sensor_state->running )
         {
             LOC_LOGI("%s: Changing sampling rate on %s reporting", __FUNCTION__, sensor_str);
         }
         sensor_state->reporting_rate = sampling_rate;
         sensor_state->batching_rate = report_rate;
         sensor_state->reporting_interval = 1000.0/sampling_rate;
         sensor_state->batching_interval = 1000.0/report_rate;

         smgr_req->ReportId = sensor_type;
         smgr_req->Action = SNS_SMGR_BUFFERING_ACTION_ADD_V01;

         //Default behavior is to stream sensor data independent of whether screen is on/off
         smgr_req->notify_suspend_valid = true;
         smgr_req->notify_suspend.proc_type = SNS_PROC_APPS_V01;
         smgr_req->notify_suspend.send_indications_during_suspend = true;

         smgr_req->ReportRate = sensor1_reporting_rate;
         smgr_req->Item_len = 1;
         smgr_req->Item[0].SensorId = sns1_sensor_id;
         smgr_req->Item[0].DataType = sns1_data_type;
         smgr_req->Item[0].Decimation = SNS_SMGR_DECIMATION_RECENT_SAMPLE_V01;
         smgr_req->Item[0].SampleQuality = 0;
         smgr_req->Item[0].SamplingRate = sampling_rate;

         /* Full calibration for calibrated magnetometer service */
         if (eSLIM_SERVICE_SENSOR_MAG_CALIB == sensor_type)
         {
             smgr_req->Item[0].Calibration = SNS_SMGR_CAL_SEL_FULL_CAL_V01;
         }
         else
         {
             smgr_req->Item[0].Calibration = SNS_SMGR_CAL_SEL_FACTORY_CAL_V01;
         }
     }
     /* Turn off sensor */
     else if( !running && sensor_state->running )
     {
         LOC_LOGI("%s: Turning off %s reporting", __FUNCTION__, sensor_str);
         smgr_req->ReportId = sensor_type;
         smgr_req->Action = SNS_SMGR_BUFFERING_ACTION_DELETE_V01;
         smgr_req->ReportRate = 0;
         smgr_req->Item_len = 0;
     }

     /* Clear out any old samples that may be lingering from previous starts/stops. */

     /* Send across Sensor1 Interface */
     if ( (error = sensor1_write( g_sensor1_control->sensor1_client_handle, &req_hdr,
                                  smgr_req )) != SENSOR1_SUCCESS )
     {
         /* Error so we deallocate QMI message */
         sensor1_free_msg_buf( g_sensor1_control->sensor1_client_handle, smgr_req );
         LOC_LOGE("%s: sensor1_write() error: %u", __FUNCTION__, error );
         if (SENSOR1_EINVALID_CLIENT == error) {
             LOC_LOGE("sensor daemon has died. Exit slim to recover");
             exit(1);
         }
         return false;
     }
     else
     {
         /* Update running state after performing MSI to make the function generic */
         sensor_state->running = running;
     }
     return true;
 }

/*===========================================================================

  FUNCTION:   slim_sensor1_update_spi_status

  ===========================================================================*/
bool slim_sensor1_update_spi_status(bool running)
{
    if( g_sensor1_control == NULL )
    {
        LOC_LOGE("%s: Initialization required to perform operation", __FUNCTION__);
        return false;
    }

    return update_spi_reporting_status(running);
}

/*===========================================================================
FUNCTION    process_smgr_resp

DESCRIPTION
  Handler for Sensor1 SMGR (Sensor Manager) Service Responses.

DEPENDENCIES
   N/A

RETURN VALUE
   N/A

SIDE EFFECTS
   N/A

===========================================================================*/
static void process_smgr_resp(sensor1_msg_header_s *msg_hdr, void* msg_ptr)
{
    LOC_LOGI("%s: msg_id = %d txn_id = %d", __FUNCTION__, msg_hdr->msg_id, msg_hdr->txn_id );
    bool error = false;

    if ( msg_hdr->msg_id == SNS_SMGR_BUFFERING_RESP_V01 )
    {
        sns_smgr_buffering_resp_msg_v01* smgr_resp = (sns_smgr_buffering_resp_msg_v01*)msg_ptr;
        slimErrorEnumT e_Error = eSLIM_SUCCESS;
        slimServiceEnumT e_Service = eSLIM_SERVICE_NONE;

        if ( smgr_resp->Resp.sns_result_t != SNS_RESULT_SUCCESS_V01 || smgr_resp->Resp.sns_err_t != SENSOR1_SUCCESS )
        {
            LOC_LOGE("%s: Result: %u, Error: %u", __FUNCTION__,
                     smgr_resp->Resp.sns_result_t, smgr_resp->Resp.sns_err_t );
            error = true;
        }

        if ( smgr_resp->AckNak_valid &&
             smgr_resp->AckNak != SNS_SMGR_RESPONSE_ACK_SUCCESS_V01 &&
             smgr_resp->AckNak != SNS_SMGR_RESPONSE_ACK_MODIFIED_V01 )
        {
            LOC_LOGE("%s: %d Error: %u Reason: %u", __FUNCTION__, smgr_resp->ReportId,
                     smgr_resp->AckNak, smgr_resp->ReasonPair[0].Reason );
            error = true;
        }
        LOC_LOGI("%s: ReportId: %u Resp: %u", __FUNCTION__, smgr_resp->ReportId,
                 smgr_resp->AckNak  );

        if(error)
        {
            e_Error = eSLIM_ERROR_INTERNAL;
            e_Service = smgr_resp->ReportId;
        }
        //TODO: notify response TO CORE
    }
    else
    {
        LOC_LOGE("%s: Received unexpected smgr resp msg_id = %d",
                 __FUNCTION__, msg_hdr->msg_id);
    }
}

/*===========================================================================
FUNCTION    check_sample_quality

DESCRIPTION
  Check the input sample quality.

DEPENDENCIES
   N/A

RETURN VALUE
   true if valid sample, false otherwise

SIDE EFFECTS
   N/A

===========================================================================*/
static bool check_sample_quality
(
  uint8_t u_sample_quality
)
{
    switch (u_sample_quality)
    {
    case SNS_SMGR_ITEM_QUALITY_INVALID_FAILED_SENSOR_V01:
    case SNS_SMGR_ITEM_QUALITY_INVALID_NOT_READY_V01:
    case SNS_SMGR_ITEM_QUALITY_INVALID_SUSPENDED_V01:
        return false;
    default:
        break;
    }
    return true;
}

/*===========================================================================
FUNCTION    convert_sensor_samples

DESCRIPTION
  Converts and dispatches barometer samples in this batch to the client.

DEPENDENCIES
   N/A

RETURN VALUE
   true if success, false otherwise

SIDE EFFECTS
   N/A

===========================================================================*/

static void convert_sensor_samples(
  const sns_smgr_buffering_sample_index_s_v01 *pz_IndexData,
  const sns_smgr_buffering_sample_s_v01 *pz_Samples,
  slimSensorTypeEnumT e_SensorType,
  slimServiceEnumT e_Service
)
{
    //TODO:Memory optimization

    SlimDaemonMessage slimApMessage;
    memset(&slimApMessage, 0, sizeof(slimApMessage));
    SlimDaemonMessage* pSlimDaemonMessage = &slimApMessage;

    slimSensorDataStructT sensorData;
    memset(&sensorData, 0, sizeof(sensorData));

    /* We support maximum of 50 samples */
    uint8 u_SampleCount = pz_IndexData->SampleCount > SLIM_SENSOR_MAX_SAMPLE_SETS ?
        SLIM_SENSOR_MAX_SAMPLE_SETS : pz_IndexData->SampleCount;
    uint32 q_LastSampleIndex = pz_IndexData->FirstSampleIdx + u_SampleCount - 1;

    uint32 q_IndexSample = 0;
    uint32 q_FillIndexSample = 0;
    int32 l_TotalTimeOffset = 0;

    LOC_LOGI("%s: rcvd sensor-samples, first-idx = %d sample-cnt = %d",
             __FUNCTION__, pz_IndexData->FirstSampleIdx, u_SampleCount);

    if (pz_IndexData->FirstSampleIdx > q_LastSampleIndex)
    {
        SLIM_MSG_ERROR0("[SLIM] Send3AxisSamples: No sample data\n");
        return;
    }

    sensorData.sensorType = e_SensorType;

    /* Store the received timestamp in milliseconds */
    sensorData.timeBase =
            slim_TimeToMillisecondsFloor(pz_IndexData->FirstSampleTimestamp);
    sensorData.timeSource = eSLIM_TIME_SOURCE_UNSPECIFIED;

    /* Calculate offset in clock ticks between time base and first sensor sample */
    l_TotalTimeOffset =
            pz_IndexData->FirstSampleTimestamp - slim_TimeToClockTicks(sensorData.timeBase);
    sensorData.samples_len = u_SampleCount;

    LOC_LOGI("%s: Received sensor-%d data with sample count-%d", __func__,
             sensorData.sensorType,
             u_SampleCount);

    for(q_IndexSample = pz_IndexData->FirstSampleIdx; q_IndexSample <= q_LastSampleIndex; q_IndexSample++)
    {
        /* Report contains the offset from timestamps of previous sample in report.
           We need to report the offset to first timestamp of report. */
        l_TotalTimeOffset += pz_Samples[q_IndexSample].TimeStampOffset;

        if (check_sample_quality(pz_Samples[q_IndexSample].Quality))
        {
            sensorData.samples[q_FillIndexSample].sampleTimeOffset = l_TotalTimeOffset;

            /* ACCEL: 3 axes, each in meter/second squared (m/s2)
               GYRO: 3 axes, each in radian/second (rad/s)
            */
            if (eSLIM_SENSOR_TYPE_ACCEL == sensorData.sensorType ||
                eSLIM_SENSOR_TYPE_GYRO == sensorData.sensorType)
            {
                /* Convert from SAE to Android co-ordinates and scale
                   x' = y; y' = x; z' = -z; sample indexes 0: X, 1: Y, 2: Z */
                sensorData.samples[q_FillIndexSample].sample[0] =
                    (float)((float)pz_Samples[q_IndexSample].Data[1] * UNIT_CONVERT_Q16);
                sensorData.samples[q_FillIndexSample].sample[1] =
                    (float)((float)pz_Samples[q_IndexSample].Data[0] * UNIT_CONVERT_Q16);
                sensorData.samples[q_FillIndexSample].sample[2] =
                    (float)((float)-pz_Samples[q_IndexSample].Data[2] * UNIT_CONVERT_Q16);

                LOC_LOGI("%s: sensor type -%d with data- %f,%f,%f", __func__,
                         sensorData.sensorType,
                         (float)pz_Samples[q_IndexSample].Data[1] * UNIT_CONVERT_Q16,
                         (float)pz_Samples[q_IndexSample].Data[0] * UNIT_CONVERT_Q16,
                         (float)-pz_Samples[q_IndexSample].Data[2] * UNIT_CONVERT_Q16);


            }
            /* MAG: 3 axes, each in uTesla */
            else if (eSLIM_SENSOR_TYPE_MAGNETOMETER == sensorData.sensorType)
            {
              /* Convert from SAE to Android co-ordinates and scale
                 x' = y; y' = x; z' = -z; sample indexes 0: X, 1: Y, 2: Z */
                sensorData.samples[q_FillIndexSample].sample[0] =
                        (float)pz_Samples[q_IndexSample].Data[1] * UNIT_CONVERT_MAGNETIC_FIELD;
                sensorData.samples[q_FillIndexSample].sample[1] =
                        (float)pz_Samples[q_IndexSample].Data[0] * UNIT_CONVERT_MAGNETIC_FIELD;
                sensorData.samples[q_FillIndexSample].sample[2] =
                        (float)-pz_Samples[q_IndexSample].Data[2] * UNIT_CONVERT_MAGNETIC_FIELD;
            }
            /* ACCEL TEMP: 1 axis, in Celsius
               GYRO TEMP: 1 axis, in Celsius
               PRESSURE (BARO): 1 axis, in hectopascal (hPa)
            */
            else
            {
                sensorData.samples[q_FillIndexSample].sample[0] =
                    (float)((float)pz_Samples[q_IndexSample].Data[0] * UNIT_CONVERT_Q16);
            }
            q_FillIndexSample++;
        }
    }

    LOC_LOGI("%s: Sensor1 data of sensor %d with data len-%d", __func__,
            sensorData.sensorType, sensorData.samples_len);
    IF_LOC_LOGV {
        int i;
        for(i=0;i<sensorData.samples_len;i++)
        {
            LOC_LOGV("%s: Sensor1 data with time offset-%d, data-(%f,%f,%f)", __func__,
                    sensorData.samples[i].sampleTimeOffset,
                    sensorData.samples[i].sample[0],
                    sensorData.samples[i].sample[1],
                    sensorData.samples[i].sample[2]);
        }
    }

    pSlimDaemonMessage->msgData.ind.data.sensorData = sensorData;

    slim_Sensor1RouteIndication(e_Service,
                                eSLIM_SUCCESS,
                                eSLIM_MESSAGE_ID_SENSOR_DATA_IND,
                                pSlimDaemonMessage);
}

/*===========================================================================
FUNCTION    process_smgr_ind

DESCRIPTION
  Handler for Sensor1 SMGR (Sensor Manager) Service Indications. Sensor data
  input is handled here.

DEPENDENCIES
   N/A

RETURN VALUE
   N/A

SIDE EFFECTS
   N/A

===========================================================================*/
void process_smgr_ind(sensor1_msg_header_s *msg_hdr, void* msg_ptr)
{
    LOC_LOGD("%s: msg_id = %d", __FUNCTION__, msg_hdr->msg_id );

    if ( msg_hdr->msg_id == SNS_SMGR_BUFFERING_IND_V01 )
    {
        uint32_t q_index = 0;
        sns_smgr_buffering_ind_msg_v01 *smgr_ind = (sns_smgr_buffering_ind_msg_v01 *)msg_ptr;

        LOC_LOGI("%s: ReportId: %d Indices: %d, Samples: %d", __FUNCTION__,
                 smgr_ind->ReportId, smgr_ind->Indices_len, smgr_ind->Samples_len);

        /* Don't want the sensor data anymore */
        if ( !g_sensor1_control->accel_state.running &&
                !g_sensor1_control->accel_temperature_state.running &&
                !g_sensor1_control->gyro_state.running &&
                !g_sensor1_control->gyro_temperature_state.running &&
                !g_sensor1_control->baro_state.running &&
                !g_sensor1_control->mag_calib_state.running &&
                !g_sensor1_control->mag_uncalib_state.running)
        {
            LOC_LOGE("%s: all accel/gyro/baro/mag turned-off.. returning\n", __FUNCTION__);
            return;
        }
        /* Don't want the sensor data anymore */
        if ( smgr_ind->Indices_len <= 0 )
        {
            LOC_LOGE("%s: indices_len <= 0, returning\n", __FUNCTION__);
            return;
        }

        /* Indication should contain only one type of sensor data per report id
           but we go through all of the indixes to make sure.
        */
        for ( q_index = 0; q_index < smgr_ind->Indices_len; q_index++ )
        {
            const sns_smgr_buffering_sample_index_s_v01 *pz_IndexData = &smgr_ind->Indices[q_index];
            int l_last_sample_index = pz_IndexData->FirstSampleIdx + pz_IndexData->SampleCount - 1;
            slimSensorTypeEnumT e_SensorType = SENSOR_TYPE_UNDEFINED;

            LOC_LOGI("%s: buffering-index = %d sensor-id = %d first-sample-idx = %d sample-cnt = %d\n first sample time stamp = %d",
                     __FUNCTION__, q_index, pz_IndexData->SensorId, pz_IndexData->FirstSampleIdx, pz_IndexData->SampleCount, pz_IndexData->FirstSampleTimestamp);

            if ( (pz_IndexData->FirstSampleIdx <= l_last_sample_index) &&
                 (l_last_sample_index < (int)smgr_ind->Samples_len) )
            {
                if (SNS_SMGR_ID_ACCEL_V01 == pz_IndexData->SensorId)
                {
                    if (g_sensor1_control->accel_state.running &&
                            SNS_SMGR_DATA_TYPE_PRIMARY_V01 == pz_IndexData->DataType)
                    {
                        LOC_LOGV("%s: Got sensor - accel\n", __FUNCTION__);
                        e_SensorType = eSLIM_SENSOR_TYPE_ACCEL;

                    }
                    else if (g_sensor1_control->accel_temperature_state.running &&
                            SNS_SMGR_DATA_TYPE_SECONDARY_V01 == pz_IndexData->DataType)
                    {

                        LOC_LOGV("%s: Got sensor - accel temp\n", __FUNCTION__);
                        e_SensorType = eSLIM_SENSOR_TYPE_ACCEL_TEMP;
                    }
                }
                else if (SNS_SMGR_ID_GYRO_V01 == pz_IndexData->SensorId)
                {
                    if (g_sensor1_control->gyro_state.running &&
                            SNS_SMGR_DATA_TYPE_PRIMARY_V01 == pz_IndexData->DataType)
                    {
                        LOC_LOGV("%s: Got sensor - gyro\n", __FUNCTION__);
                        e_SensorType = eSLIM_SENSOR_TYPE_GYRO;
                    }
                    else if (g_sensor1_control->gyro_temperature_state.running &&
                            SNS_SMGR_DATA_TYPE_SECONDARY_V01 == pz_IndexData->DataType)
                    {
                        LOC_LOGV("%s: Got sensor - gyro temp\n", __FUNCTION__);
                        e_SensorType = eSLIM_SENSOR_TYPE_GYRO_TEMP;
                    }
                }
                else if (SNS_SMGR_ID_PRESSURE_V01 == pz_IndexData->SensorId)
                {
                    if (g_sensor1_control->baro_state.running &&
                            SNS_SMGR_DATA_TYPE_PRIMARY_V01 == pz_IndexData->DataType)
                    {
                        e_SensorType = eSLIM_SENSOR_TYPE_BAROMETER;
                    }
                }
                else if (SNS_SMGR_ID_MAG_V01 == pz_IndexData->SensorId &&
                        ((g_sensor1_control->mag_calib_state.running &&
                        eSLIM_SERVICE_SENSOR_MAG_CALIB == smgr_ind->ReportId) ||
                        (g_sensor1_control->mag_uncalib_state.running &&
                        eSLIM_SERVICE_SENSOR_MAG_UNCALIB == smgr_ind->ReportId)))
                {
                    if (SNS_SMGR_DATA_TYPE_PRIMARY_V01 == pz_IndexData->DataType)
                    {
                        e_SensorType = eSLIM_SENSOR_TYPE_MAGNETOMETER;
                    }
                }
                /* If sensor type is supported, convert samples and send them to client */
                if (e_SensorType != (slimSensorTypeEnumT)SENSOR_TYPE_UNDEFINED)
                {

                    LOC_LOGV("%s: sensor-type is %d\n",
                             __FUNCTION__, e_SensorType);

                    convert_sensor_samples(pz_IndexData,
                                           &smgr_ind->Samples[0],
                                           e_SensorType,
                                           smgr_ind->ReportId) ;
                }
                else
                {
                    LOC_LOGE("%s: buffering api not supported/enabled for this sensor-id %d, sensor-type %d\n",
                             __FUNCTION__, pz_IndexData->SensorId, pz_IndexData->DataType);
                }
            }
        }
    }
    else
    {
        LOC_LOGE("%s: Received invalid indication, msg_id = %d",
                 __FUNCTION__, msg_hdr->msg_id);
    }


}

/*===========================================================================
FUNCTION    process_sam_resp

DESCRIPTION
  Handler for Sensor1 SAM Service Responses.

DEPENDENCIES
   N/A

RETURN VALUE
   N/A

SIDE EFFECTS
   N/A

===========================================================================*/
static void process_sam_resp(sensor1_msg_header_s *msg_hdr, void* msg_ptr)
{
    LOC_LOGD("%s: msg_id = %d", __FUNCTION__, msg_hdr->msg_id );
    //TODO: Send resonse to slim core
    switch ( msg_hdr->service_number )
    {
    case SNS_SAM_AMD_SVC_ID_V01:
        if ( msg_hdr->msg_id == SNS_SAM_AMD_ENABLE_RESP_V01)
        {
            sns_sam_qmd_enable_resp_msg_v01* resp_ptr = (sns_sam_qmd_enable_resp_msg_v01*)msg_ptr;

            if ( resp_ptr->resp.sns_result_t != SNS_RESULT_SUCCESS_V01 || resp_ptr->resp.sns_err_t != SENSOR1_SUCCESS )
            {
                LOC_LOGE("%s: Bad Enable response result = %u, error = %u", __FUNCTION__,
                         resp_ptr->resp.sns_result_t, resp_ptr->resp.sns_err_t  );
            }
            else
            {
                LOC_LOGI("%s: Got ENABLE response, status = %d, instance_id = %d",
                         __FUNCTION__, resp_ptr->resp.sns_result_t, resp_ptr->instance_id);
                g_sensor1_control->motion_state.amd_algo_instance_id = resp_ptr->instance_id;
            }
        }
        else if ( msg_hdr->msg_id == SNS_SAM_AMD_DISABLE_RESP_V01)
        {
            sns_sam_qmd_disable_resp_msg_v01* resp_ptr = (sns_sam_qmd_disable_resp_msg_v01*)msg_ptr;

            if ( resp_ptr->resp.sns_result_t != SNS_RESULT_SUCCESS_V01 || resp_ptr->resp.sns_err_t != SENSOR1_SUCCESS )
            {
                LOC_LOGE("%s: Bad Disable response result = %u, error = %u", __FUNCTION__,
                         resp_ptr->resp.sns_result_t, resp_ptr->resp.sns_err_t  );
            }
            else
            {
                LOC_LOGI("%s: Got DISABLE response, status = %d, instance_id = %d",
                         __FUNCTION__, resp_ptr->resp.sns_result_t, resp_ptr->instance_id);
            }
        }
        else
        {
            LOC_LOGE("%s: Received invalid sam response, msg_id = %d",
                     __FUNCTION__, msg_hdr->msg_id);
        }
        break;

    case SNS_SAM_RMD_SVC_ID_V01:
        if ( msg_hdr->msg_id == SNS_SAM_RMD_ENABLE_RESP_V01)
        {
            sns_sam_qmd_enable_resp_msg_v01* resp_ptr = (sns_sam_qmd_enable_resp_msg_v01*)msg_ptr;

            if ( resp_ptr->resp.sns_result_t != SNS_RESULT_SUCCESS_V01 || resp_ptr->resp.sns_err_t != SENSOR1_SUCCESS )
            {
                LOC_LOGE("%s: Bad Enable response result = %u, error = %u", __FUNCTION__,
                         resp_ptr->resp.sns_result_t, resp_ptr->resp.sns_err_t  );
            }
            else
            {
                LOC_LOGI("%s: Got ENABLE response, status = %d, instance_id = %d",
                         __FUNCTION__, resp_ptr->resp.sns_result_t, resp_ptr->instance_id);
                g_sensor1_control->motion_state.rmd_algo_instance_id = resp_ptr->instance_id;
            }
        }
        else if ( msg_hdr->msg_id == SNS_SAM_RMD_DISABLE_RESP_V01)
        {
            sns_sam_qmd_disable_resp_msg_v01* resp_ptr = (sns_sam_qmd_disable_resp_msg_v01*)msg_ptr;

            if ( resp_ptr->resp.sns_result_t != SNS_RESULT_SUCCESS_V01 || resp_ptr->resp.sns_err_t != SENSOR1_SUCCESS )
            {
                LOC_LOGE("%s: Bad Disable response result = %u, error = %u", __FUNCTION__,
                         resp_ptr->resp.sns_result_t, resp_ptr->resp.sns_err_t  );
            }
            else
            {
                LOC_LOGI("%s: Got DISABLE response, status = %d, instance_id = %d",
                         __FUNCTION__, resp_ptr->resp.sns_result_t, resp_ptr->instance_id);
            }
        }
        else
        {
            LOC_LOGE("%s: Received invalid sam response, msg_id = %d",
                     __FUNCTION__, msg_hdr->msg_id);
        }
        break;

    case SNS_SAM_VMD_SVC_ID_V01:
        if ( msg_hdr->msg_id == SNS_SAM_VMD_ENABLE_RESP_V01)
        {
            sns_sam_qmd_enable_resp_msg_v01* resp_ptr = (sns_sam_qmd_enable_resp_msg_v01*)msg_ptr;

            if ( resp_ptr->resp.sns_result_t != SNS_RESULT_SUCCESS_V01 || resp_ptr->resp.sns_err_t != SENSOR1_SUCCESS )
            {
                LOC_LOGE("%s: Bad Enable response result = %u, error = %u", __FUNCTION__,
                         resp_ptr->resp.sns_result_t, resp_ptr->resp.sns_err_t  );
            }
            else
            {
                LOC_LOGI("%s: Got ENABLE response, status = %d, instance_id = %d",
                         __FUNCTION__, resp_ptr->resp.sns_result_t, resp_ptr->instance_id);
                g_sensor1_control->spi_algo_instance_id = resp_ptr->instance_id;
            }
        }
        else if ( msg_hdr->msg_id == SNS_SAM_VMD_DISABLE_RESP_V01)
        {
            sns_sam_qmd_disable_resp_msg_v01* resp_ptr = (sns_sam_qmd_disable_resp_msg_v01*)msg_ptr;

            if ( resp_ptr->resp.sns_result_t != SNS_RESULT_SUCCESS_V01 || resp_ptr->resp.sns_err_t != SENSOR1_SUCCESS )
            {
                LOC_LOGE("%s: Bad Disable response result = %u, error = %u", __FUNCTION__,
                         resp_ptr->resp.sns_result_t, resp_ptr->resp.sns_err_t  );
            }
            else
            {
                LOC_LOGI("%s: Got DISABLE response, status = %d, instance_id = %d",
                         __FUNCTION__, resp_ptr->resp.sns_result_t, resp_ptr->instance_id);
            }
        }
        else
        {
            LOC_LOGE("%s: Received invalid sam response, msg_id = %d",
                     __FUNCTION__, msg_hdr->msg_id);
        }
        break;

    case SNS_SAM_PED_SVC_ID_V01:
        if ( msg_hdr->msg_id == SNS_SAM_PED_ENABLE_RESP_V01)
        {
            sns_sam_ped_enable_resp_msg_v01* resp_ptr = (sns_sam_ped_enable_resp_msg_v01*)msg_ptr;

            if ( resp_ptr->resp.sns_result_t != SNS_RESULT_SUCCESS_V01 || resp_ptr->resp.sns_err_t != SENSOR1_SUCCESS )
            {
                LOC_LOGE("%s: Bad ENABLE response result = %u, error = %u", __FUNCTION__,
                         resp_ptr->resp.sns_result_t, resp_ptr->resp.sns_err_t  );
                g_sensor1_control->pedometer_state.running = false;
            }
            else
            {
                LOC_LOGI("%s: Got ENABLE response, status = %d, instance_id = %d",
                         __FUNCTION__, resp_ptr->resp.sns_result_t, resp_ptr->instance_id);
                gettimeofday(&(g_sensor1_control->pedometer_state.reset_time),NULL);
                g_sensor1_control->pedometer_state.algo_instance_id = resp_ptr->instance_id;

            }
        }
        else if ( msg_hdr->msg_id == SNS_SAM_PED_DISABLE_RESP_V01)
        {
            sns_sam_ped_disable_resp_msg_v01* resp_ptr = (sns_sam_ped_disable_resp_msg_v01*)msg_ptr;

            if ( resp_ptr->resp.sns_result_t != SNS_RESULT_SUCCESS_V01 || resp_ptr->resp.sns_err_t != SENSOR1_SUCCESS )
            {
                LOC_LOGE("%s: Bad DISABLE response result = %u, error = %u", __FUNCTION__,
                         resp_ptr->resp.sns_result_t, resp_ptr->resp.sns_err_t  );
            }
            else
            {
                LOC_LOGI("%s: Got DISABLE response, status = %d, instance_id = %d",
                         __FUNCTION__, resp_ptr->resp.sns_result_t, resp_ptr->instance_id);
            }
        }
        else if ( msg_hdr->msg_id == SNS_SAM_PED_RESET_RESP_V01)
        {
            sns_sam_ped_reset_resp_msg_v01* resp_ptr = (sns_sam_ped_reset_resp_msg_v01*)msg_ptr;

            if ( resp_ptr->resp.sns_result_t != SNS_RESULT_SUCCESS_V01 || resp_ptr->resp.sns_err_t != SENSOR1_SUCCESS )
            {
                LOC_LOGE("%s: Bad RESET response result = %u, error = %u", __FUNCTION__,
                         resp_ptr->resp.sns_result_t, resp_ptr->resp.sns_err_t  );
            }
            else
            {
                LOC_LOGI("%s: Got RESET response, status = %d, instance_id = %d",
                         __FUNCTION__, resp_ptr->resp.sns_result_t, resp_ptr->instance_id);
                gettimeofday(&(g_sensor1_control->pedometer_state.reset_time),NULL);
            }
        }
        else
        {
            LOC_LOGE("%s: Received invalid sam response, msg_id = %d",
                     __FUNCTION__, msg_hdr->msg_id);
        }
        break;

    default:
        break;
    }

    return;
}

/*===========================================================================
FUNCTION    process_amd_and_rmd_ind

DESCRIPTION
  Handler for Sensor1 SAM Service Indications.

DEPENDENCIES
   N/A

RETURN VALUE
   N/A

SIDE EFFECTS
   N/A

===========================================================================*/
static void process_amd_and_rmd_ind(sensor1_msg_header_s *msg_hdr, void* msg_ptr, bool error)
{
    if( NULL == msg_ptr || NULL == msg_hdr)
    {
        LOC_LOGD("%s:msg_hdr(%p) or msg_ptr(%p) is NULL",__FUNCTION__, msg_hdr,msg_ptr );
        return;
    }

    LOC_LOGD("%s: msg_id = %d", __FUNCTION__, msg_hdr->msg_id );

    switch (msg_hdr->msg_id)
    {
    /* Handler for both AMD and RMD (same message id value) */
    case SNS_SAM_AMD_REPORT_IND_V01:
    //case SNS_SAM_RMD_REPORT_IND_V01:
    {
        sns_sam_qmd_report_ind_msg_v01* ind_ptr = (sns_sam_qmd_report_ind_msg_v01*)msg_ptr;
        uint32_t dsps_time_ms = slim_TimeToMilliseconds(ind_ptr->timestamp);

        if (error)
        {
            g_sensor1_control->motion_state.amd = SNS_SAM_MOTION_UNKNOWN_V01;
            g_sensor1_control->motion_state.rmd = SNS_SAM_MOTION_UNKNOWN_V01;
        }
        else if (msg_hdr->service_number == SNS_SAM_AMD_SVC_ID_V01)
        {
            LOC_LOGI("%s: Received AMD state indication, instance_id = %d, timestamp = %" PRIu32 ", state = %d",
                     __FUNCTION__, ind_ptr->instance_id, dsps_time_ms, ind_ptr->state);
            g_sensor1_control->motion_state.amd = ind_ptr->state;
        }
        else if (msg_hdr->service_number == SNS_SAM_RMD_SVC_ID_V01)
        {
            LOC_LOGI("%s: Received RMD state indication, instance_id = %d, timestamp = %" PRIu32 ", state = %d",
                     __FUNCTION__, ind_ptr->instance_id, dsps_time_ms, ind_ptr->state);
            g_sensor1_control->motion_state.rmd = ind_ptr->state;
        }

        SlimDaemonMessage slimApMessage;
        memset(&slimApMessage, 0, sizeof(slimApMessage));
        SlimDaemonMessage* pSlimDaemonMessage = &slimApMessage;

        slimMotionDataStructT motionData;
        memset(&motionData, 0, sizeof(motionData));

        motionData.timeSource = eSLIM_TIME_SOURCE_UNSPECIFIED;
        motionData.age = MOTION_DATA_TLV_AGE;
        motionData.motionMode = eSLIM_MOTION_MODE_UNKNOWN;
        motionData.timeout = MOTION_DATA_TLV_TIMEOUT;

        if (g_sensor1_control->motion_state.amd == SNS_SAM_MOTION_UNKNOWN_V01)
        {
            motionData.motionState = eSLIM_MOTION_STATE_UNKNOWN;
            motionData.probabilityOfState = 99.9;
        }
        else if (g_sensor1_control->motion_state.amd == SNS_SAM_MOTION_REST_V01)
        {
            motionData.motionState = eSLIM_MOTION_STATE_STATIONARY;
            motionData.probabilityOfState = 99.9;
        }
        else if (g_sensor1_control->motion_state.rmd == SNS_SAM_MOTION_UNKNOWN_V01)
        {
            motionData.motionState = eSLIM_MOTION_STATE_UNKNOWN;
            motionData.probabilityOfState = 99.9;
        }
        else if (g_sensor1_control->motion_state.rmd == SNS_SAM_MOTION_REST_V01)
        {
            motionData.motionState = eSLIM_MOTION_STATE_STATIONARY;
            motionData.probabilityOfState = 90.0;
        }
        else if (g_sensor1_control->motion_state.rmd == SNS_SAM_MOTION_MOVE_V01)
        {
            motionData.motionState = eSLIM_MOTION_STATE_IN_MOTION;
            motionData.probabilityOfState = 99.9;
        }

        if (0 != dsps_time_ms )
        {
            motionData.timestamp = dsps_time_ms;
            motionData.age = 0;//TODO: TBD is reset is needed
        }

        LOC_LOGI("%s: Motion data update sent with state = %d and probability = %f", __FUNCTION__,motionData.motionState, motionData.probabilityOfState);

        pSlimDaemonMessage->msgData.ind.data.motionData = motionData;

        slim_Sensor1RouteIndication(eSLIM_SERVICE_MOTION_DATA,
                                    eSLIM_SUCCESS,
                                    eSLIM_MESSAGE_ID_MOTION_DATA_IND,
                                    pSlimDaemonMessage);
    }
    break;
    default:
    {
        LOC_LOGE("%s: Received invalid indication, msg_id = %d",
                 __FUNCTION__, msg_hdr->msg_id);
    }
    break;
    }
}

/*===========================================================================
FUNCTION    process_vmd_ind

DESCRIPTION
  Handler for Sensor1 SAM Service Indications.

DEPENDENCIES
   N/A

RETURN VALUE
   N/A

SIDE EFFECTS
   N/A

===========================================================================*/
static void process_vmd_ind(sensor1_msg_header_s *msg_hdr, void* msg_ptr)
{
    LOC_LOGD("%s: msg_id = %d", __FUNCTION__, msg_hdr->msg_id );

    switch (msg_hdr->msg_id)
    {
    case SNS_SAM_VMD_REPORT_IND_V01:
    {
        sns_sam_qmd_report_ind_msg_v01* ind_ptr = (sns_sam_qmd_report_ind_msg_v01*)msg_ptr;
        uint32_t dsps_time_ms = slim_TimeToMilliseconds(ind_ptr->timestamp);

        LOC_LOGI("%s: Received SPI indication, instance_id = %d, timestamp = %" PRIu32 ", state = %d",
                 __FUNCTION__, ind_ptr->instance_id, dsps_time_ms, ind_ptr->state);

        g_sensor1_control->spi_algo_instance_id = ind_ptr->instance_id;

        /* Only send SPI updates for moving or stationary */
        if( ind_ptr->state == SNS_SAM_MOTION_REST_V01 || ind_ptr->state == SNS_SAM_MOTION_MOVE_V01 )
        {
            uint8_t is_stationary = 0xFF;
            if( ind_ptr->state == SNS_SAM_MOTION_REST_V01 )
            {
                is_stationary = 1;
            }
            else if( ind_ptr->state == SNS_SAM_MOTION_MOVE_V01 )
            {
                is_stationary = 0;
            }
            //TODO:
            /* Send to data task through message queue. */
        }
    }
    break;

    default:
    {
        LOC_LOGE("%s: Received invalid indication, msg_id = %d",
                 __FUNCTION__, msg_hdr->msg_id);
    }
    break;
    }
}


/*===========================================================================
FUNCTION    process_ped_ind

DESCRIPTION
  Handler for Sensor1 SAM Service Indications.

DEPENDENCIES
   N/A

RETURN VALUE
   N/A

SIDE EFFECTS
   N/A

===========================================================================*/
static void process_ped_ind(sensor1_msg_header_s *msg_hdr, void* msg_ptr)
{
    LOC_LOGD("%s: msg_id = %d", __FUNCTION__, msg_hdr->msg_id );

    switch (msg_hdr->msg_id)
    {
    case SNS_SAM_PED_REPORT_IND_V01:
    {
        sns_sam_ped_report_ind_msg_v01* ind_ptr = (sns_sam_ped_report_ind_msg_v01*)msg_ptr;
        uint32_t dsps_time_ms = slim_TimeToMilliseconds(ind_ptr->timestamp);
        struct timeval current_time;
        struct timeval diff;
        gettimeofday(&current_time,NULL);
        timersub(&current_time,&(g_sensor1_control->pedometer_state.reset_time),&diff);
        uint32_t time_interval = (uint32_t)(diff.tv_sec*1000 + (float)diff.tv_usec/1000);

        LOC_LOGI("%s: Received Pedometer indication, instance_id = %d, timestamp = %" PRIu32
                ", time_interval = %" PRIu32 " ms, step_event = %u, step_confidence = %u, "
                "step_count = %" PRIu32 ", step_count_error = %" PRId32 ", step_rate = %4.2f Hz",
                 __FUNCTION__, ind_ptr->instance_id, dsps_time_ms, time_interval,
                 ind_ptr->report_data.step_event,
                 ind_ptr->report_data.step_confidence,
                 ind_ptr->report_data.step_count,
                 ind_ptr->report_data.step_count_error,
                 ind_ptr->report_data.step_rate);

        /* SLIM does not support periodic pedometer reports. A report will be
           generated only when a step event is detected. */
        if (ind_ptr->report_data.step_event)
        {
            SlimDaemonMessage slimApMessage;
            memset(&slimApMessage, 0, sizeof(slimApMessage));
            SlimDaemonMessage* pSlimDaemonMessage = &slimApMessage;

            slimPedometerDataStructT pedometerData;
            memset(&pedometerData, 0, sizeof(pedometerData));

            /* Update SSC time and store the received timestamp in milliseconds */
            pedometerData.timestamp = dsps_time_ms;
            pedometerData.timeSource = eSLIM_TIME_SOURCE_UNSPECIFIED;

            /* Calculate interval from the last reset */
            pedometerData.timeInterval = time_interval;

            pedometerData.stepCount = ind_ptr->report_data.step_count;
            pedometerData.stepConfidence_valid = TRUE;
            pedometerData.stepConfidence = ind_ptr->report_data.step_confidence;
            pedometerData.stepCountUncertainty_valid = TRUE;
            pedometerData.stepCountUncertainty = (float)ind_ptr->report_data.step_count_error;
            pedometerData.stepRate_valid = TRUE;
            pedometerData.stepRate = ind_ptr->report_data.step_rate;
            pedometerData.stepCountFractional_valid = FALSE;
            pedometerData.strideLength_valid = FALSE;
            pedometerData.strideLengthUncertainty_valid = FALSE;

            pedometerData.reportType_valid = TRUE;
            pedometerData.reportType = eSLIM_INJECT_PEDOMETER_REPORT_ON_STEP_EVENT;

            pSlimDaemonMessage->msgData.ind.data.pedometerData = pedometerData;

            slim_Sensor1RouteIndication(eSLIM_SERVICE_PEDOMETER,
                                        eSLIM_SUCCESS,
                                        eSLIM_MESSAGE_ID_PEDOMETER_IND,
                                        pSlimDaemonMessage);
        }
        else
        {
            LOC_LOGI("%s: Pedometer indication ignored.", __FUNCTION__);
        }
    }
    break;

    default:
    {
        LOC_LOGE("%s: Received invalid indication, msg_id = %d",
                 __FUNCTION__, msg_hdr->msg_id);
    }
    break;
    }
}

/*===========================================================================
FUNCTION    process_time_resp

DESCRIPTION
  Handler for Sensor1 SAM Service Time Indications.

DEPENDENCIES
   N/A

RETURN VALUE
   N/A

SIDE EFFECTS
   N/A

===========================================================================*/
void process_time_resp( sensor1_msg_header_s *msg_hdr, void *msg_ptr )
{
    bool error = false;

    if( SNS_TIME_TIMESTAMP_RESP_V02 == msg_hdr->msg_id )
    {
        sns_time_timestamp_resp_msg_v02 *time_msg_ptr =
            (sns_time_timestamp_resp_msg_v02*) msg_ptr;

        if( 0 == time_msg_ptr->resp.sns_result_t )
        {
            if( true == time_msg_ptr->timestamp_dsps_valid )
            {
                uint32_t dsps_clk_ticks = time_msg_ptr->timestamp_dsps;
                uint32_t provider_time = slim_TimeToMilliseconds(dsps_clk_ticks);

                LOC_LOGI( "%s: Apps: %" PRIu64 "; DSPS: %u; Ms: %" PRIu32,
                        __FUNCTION__,
                        time_msg_ptr->timestamp_apps,
                        time_msg_ptr->timestamp_dsps,
                        provider_time);

                slim_ProviderRouteTimeResponse(
                        msg_hdr->txn_id, SLIM_PROVIDER_SENSOR1, eSLIM_SUCCESS, provider_time);
            }
            else if( true == time_msg_ptr->error_code_valid )
            {
                LOC_LOGE( "%s: Error in time response: %i", __FUNCTION__, time_msg_ptr->error_code );
                error = true;
            }
            else
            {
                LOC_LOGE( "%s: Unknown error in time response", __FUNCTION__ );
                error = true;
            }
        }
        else
        {
            LOC_LOGE( "%s: Received 'Failed' in time response result", __FUNCTION__ );
            error = true;
        }
    }
    else
    {
        error = true;
        LOC_LOGE( "%s: Unhandled message id received: %i",
                  __FUNCTION__, msg_hdr->msg_id );
    }
}

static void sensor1_notify_data_callback(intptr_t cb_data,
                                  sensor1_msg_header_s *msg_hdr,
                                  sensor1_msg_type_e msg_type,
                                  void* msg_ptr)
{
    SLIM_UNUSED(cb_data);
    LOC_LOGD("%s: Message Type = %d", __FUNCTION__, msg_type );

    if ( msg_hdr != NULL )
    {
        LOC_LOGD("%s: Service Num = %d, Message id = %d", __FUNCTION__,
                 msg_hdr->service_number, msg_hdr->msg_id );
    }

    switch ( msg_type )
    {
    case SENSOR1_MSG_TYPE_RETRY_OPEN:
        pthread_mutex_lock( &g_sensor1_access_control->cb_mutex );
        g_sensor1_open_result = sensor1_open(&g_p_sensor1_hndl,sensor1_notify_data_callback,(intptr_t)NULL);
        LOC_LOGV("%s: Sensor1 open: %d", __FUNCTION__, g_sensor1_open_result);

        pthread_cond_signal( &g_sensor1_access_control->cb_arrived_cond );
        pthread_mutex_unlock( &g_sensor1_access_control->cb_mutex );
        break;
    case SENSOR1_MSG_TYPE_RESP:
        if( NULL == msg_ptr || NULL == msg_hdr)
        {
            LOC_LOGE("%s: Invalid msg_ptr/msg_hdr for resp", __FUNCTION__);
        }
        else if ( SNS_SMGR_SVC_ID_V01 == msg_hdr->service_number )
        {
            process_smgr_resp(msg_hdr, msg_ptr);
        }
        else if ( SNS_SAM_AMD_SVC_ID_V01 == msg_hdr->service_number ||
                  SNS_SAM_RMD_SVC_ID_V01 == msg_hdr->service_number ||
                  SNS_SAM_VMD_SVC_ID_V01 == msg_hdr->service_number ||
                  SNS_SAM_PED_SVC_ID_V01 == msg_hdr->service_number )
        {
            process_sam_resp(msg_hdr, msg_ptr);
        }
        else if ( SNS_TIME2_SVC_ID_V01 == msg_hdr->service_number )
        {
            process_time_resp( msg_hdr, msg_ptr );
        }
        else
        {
            LOC_LOGE("%s: Unexpected resp service_number = %u", __FUNCTION__, msg_hdr->service_number);
        }
        break;

    case SENSOR1_MSG_TYPE_IND:
        if ( NULL == msg_ptr || NULL == msg_hdr)
        {
            LOC_LOGE("%s: Invalid msg_ptr/msg_hdr for ind", __FUNCTION__);
        }
        else if ( SNS_SMGR_SVC_ID_V01 == msg_hdr->service_number )
        {
            process_smgr_ind(msg_hdr, msg_ptr);
        }
        else if ( SNS_SAM_AMD_SVC_ID_V01 == msg_hdr->service_number ||
                  SNS_SAM_RMD_SVC_ID_V01 == msg_hdr->service_number )
        {
            process_amd_and_rmd_ind(msg_hdr, msg_ptr,false);
        }
        else if ( SNS_SAM_VMD_SVC_ID_V01 == msg_hdr->service_number )
        {
            process_vmd_ind(msg_hdr, msg_ptr);
        }
        else if ( SNS_SAM_PED_SVC_ID_V01 == msg_hdr->service_number )
        {
            process_ped_ind(msg_hdr, msg_ptr);
        }
        else
        {
            LOC_LOGE("%s: Unexpected ind service_number = %u", __FUNCTION__, msg_hdr->service_number);
        }
        break;

    case SENSOR1_MSG_TYPE_BROKEN_PIPE:
        LOC_LOGE("%s: Sensor1 Broken Pipe msg type in cb. Closing Sensor1", __FUNCTION__);
        process_amd_and_rmd_ind(msg_hdr, msg_ptr, true);
        slim_sensor1_destroy();
        LOC_LOGE("sensor daemon has died. Exit slim to recover");
        exit(1);
        break;

    case SENSOR1_MSG_TYPE_RESP_INT_ERR:
    case SENSOR1_MSG_TYPE_REQ:
    default:
        LOC_LOGE("%s: Invalid msg type in cb = %u", __FUNCTION__, msg_type );
        break;
    }

    /* Free message received as we are done with it and to comply with Sensor1 Documentation */
    if( NULL != msg_ptr )
    {
        sensor1_free_msg_buf(g_sensor1_control->sensor1_client_handle, msg_ptr);
    }
}

/* ----------------------- END INTERNAL FUNCTIONS ---------------------------------------- */

/*===========================================================================

  FUNCTION:   slim_sensor1_open

  ===========================================================================*/
bool slim_sensor1_open()
{
    bool b_Success = false;

    /* TODO: remove the dependency from SlimDaemonManager to here
     * Instead the available services should be set dynamically depending on
     * whether this succeeds in provider initialization.
     */
#ifdef FEATURE_GSIFF_DSPS
    /* Fill with default values */
    g_sensor1_access_control =
            (sensor1_access_control_t*)calloc(1, sizeof(*g_sensor1_access_control));

    if( NULL == g_sensor1_access_control )
    {
        LOC_LOGE("%s: Error: calloc error", __FUNCTION__);
        return false;
    }

    /*initialize mutex/cv to synchronize sensor1 api calls */
    pthread_mutex_init(&(g_sensor1_access_control->cb_mutex), NULL);
    pthread_cond_init(&(g_sensor1_access_control->cb_arrived_cond), NULL);

    g_sensor1_open_result =
            sensor1_open(&g_p_sensor1_hndl,sensor1_notify_data_callback,(intptr_t)NULL);

    switch (g_sensor1_open_result)
    {
    case SENSOR1_SUCCESS:
        LOC_LOGI("%s: Sensor1 connection opened successfully!", __FUNCTION__);
        b_Success = true;
        break;

    case SENSOR1_EWOULDBLOCK:
        LOC_LOGI("%s: Pending Sensor1 connection opening!", __FUNCTION__);
        b_Success = true;
        break;

    default:
        LOC_LOGI("%s: Sensor1 open: %d."
                "Fall back to Android NDK as Sensor Core is not available!",
                __FUNCTION__, g_sensor1_open_result);
        break;
    }
#endif /* FEATURE_GSIFF_DSPS */

    return b_Success;
}

/*===========================================================================

  FUNCTION:   slim_sensor1_close

  ===========================================================================*/
void slim_sensor1_close()
{
    if (NULL != g_sensor1_access_control)
    {
        /* Remove sensor1 mutex/CV */
        int mutex_rc = pthread_mutex_destroy(&g_sensor1_access_control->cb_mutex);
        if ( mutex_rc != 0 )
        {
            LOC_LOGE("%s: Could not destroy sensor1 cb mutex rc = %d errno = %d!",
                     __FUNCTION__, mutex_rc, errno);
        }

        int cv_rc = pthread_cond_destroy(&g_sensor1_access_control->cb_arrived_cond);
        if ( cv_rc != 0 )
        {
            LOC_LOGE("%s: Could not destroy sensor1 cb CV rc = %d errno = %d!",
                     __FUNCTION__, cv_rc, errno);
        }

        free(g_sensor1_access_control);
        g_sensor1_access_control = NULL;
    }
}

/*===========================================================================

  FUNCTION:   slim_sensor1_init

  ===========================================================================*/
bool slim_sensor1_init()
{
    LOC_LOGI("%s: Initializing Sensor1", __FUNCTION__);

    /* Initialize Sensor1 API */
    if( slim_sensor1_init_defaults() == false )
    {
        LOC_LOGE("%s: Unable to initialize with default values!", __FUNCTION__);
        return false;
    }

    if (g_sensor1_open_result == SENSOR1_EWOULDBLOCK)
    {
        LOC_LOGW("sensor process is not up and running yet!");

        pthread_mutex_lock( &g_sensor1_access_control->cb_mutex );
        pthread_cond_wait( &g_sensor1_access_control->cb_arrived_cond, &g_sensor1_access_control->cb_mutex );
        pthread_mutex_unlock( &g_sensor1_access_control->cb_mutex );
    }

    // Check that sensor1 client was successfully created
    if(g_sensor1_open_result != SENSOR1_SUCCESS)
    {
        LOC_LOGE("Fatal: Could not open Sensor1 Client sensor1_open = %d!", g_sensor1_open_result);
        slim_sensor1_destroy();
        return false;
    }

    g_sensor1_control->sensor1_client_handle = g_p_sensor1_hndl;

    return true;
}


/*===========================================================================

  FUNCTION:   slim_sensor1_destroy

  ===========================================================================*/
bool slim_sensor1_destroy()
{
    if( g_sensor1_control != NULL )
    {
        //TODO:
        /* Turn off sensor data reporting if it is still on */
        if( g_sensor1_control->sensor1_client_handle != NULL && g_sensor1_open_result == SENSOR1_SUCCESS)
        {
            sensor1_error_e error = sensor1_close(g_sensor1_control->sensor1_client_handle);
            if ( SENSOR1_SUCCESS != error )
            {
                LOC_LOGE("%s: sensor1_close() error: %u", __FUNCTION__, error);
                return false;
            }
        }

        /* Deallocate memory */
        free(g_sensor1_control);
        g_sensor1_control = NULL;

        return true;
    }
    else
    {
        LOC_LOGE("%s: Initialization required to perform operation", __FUNCTION__);
        return false;
    }
}

/*===========================================================================

  FUNCTION:   slim_sensor1_get_sensor_time

  ===========================================================================*/
bool slim_sensor1_get_sensor_time(int32 l_ServiceTxnId)
{
    sensor1_error_e                 error;
    sensor1_msg_header_s            msg_hdr;
    sns_reg_single_read_req_msg_v02 *msg_ptr = NULL;

    if( g_sensor1_control == NULL )
    {
        LOC_LOGE("%s: Initialization required to perform operation", __FUNCTION__);
        return false;
    }

    error = sensor1_alloc_msg_buf( g_sensor1_control->sensor1_client_handle, 0, (void**)&msg_ptr );
    if( SENSOR1_SUCCESS != error )
    {
        LOC_LOGE( "%s: sensor1_alloc_msg_buf returned(get) %d", __FUNCTION__, error );
        return false;
    }

    /* Create the message header */
    msg_hdr.service_number = SNS_TIME2_SVC_ID_V01;
    msg_hdr.msg_id = SNS_TIME_TIMESTAMP_REQ_V02;
    msg_hdr.msg_size = 0;
    msg_hdr.txn_id = l_ServiceTxnId;

    error = sensor1_write( g_sensor1_control->sensor1_client_handle, &msg_hdr, msg_ptr );
    if( SENSOR1_SUCCESS != error )
    {
        LOC_LOGE( "%s: sensor1_write returned %d", __FUNCTION__, error );
        sensor1_free_msg_buf( g_sensor1_control->sensor1_client_handle, msg_ptr );
        if (SENSOR1_EINVALID_CLIENT == error) {
            LOC_LOGE("sensor daemon has died. Exit slim to recover");
            exit(1);
        }
        return false;
    }

    return true;
}
