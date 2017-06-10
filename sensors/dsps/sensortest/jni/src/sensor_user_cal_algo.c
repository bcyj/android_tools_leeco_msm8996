/*============================================================================
@file sensor_user_cal_algo.c

@brief
Provides the accel user cal algorithm.

Copyright (c) 2011-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

/*============================================================================
  INCLUDE FILES
  ==========================================================================*/
#define LOG_TAG "user_cal_algo"

#include "sns_smgr_api_v01.h"
#include "sensor1.h"
#include <signal.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <time.h>
#include <stdlib.h>
#include <utils/Log.h>
#include <common_log.h>
#include <stdbool.h>
#include <inttypes.h>

/*============================================================================
  Type Declarations
  ==========================================================================*/

#define USER_CAL_COL 3
#define REPORT_RATE 20

typedef int32_t q16_t;
#define G                    (9.80665) // accel due to gravity at sea level
#define FX_ABS(x)            ((x<0)?(-(x)):(x))
#define FX_CONV(a,q1,q2)     (((q2)>(q1))?(a)<<((q2)-(q1)):(a)>>((q1)-(q2)))
#define FX_QFACTOR           (16)
#define FX_FLTTOFIX(f,q)     ((int32_t)( (f)*(1<<(q))+(((f)>(0.0))?(0.5):(-0.5))))
#define FX_FLTTOFIX_Q16(d)   (FX_FLTTOFIX(d,FX_QFACTOR))

typedef enum user_cal_error_e {
   ERROR_SUCCESS = 0,  /* No error */
   ERROR_TIMEOUT = -1, /* Response error - timeout */
   ERROR_SENSOR1 = -2, /* Sensor1 Error */
   ERROR_BIAS    = -3, /* Error using bias */
   ERROR_STYPE   = -4, /* Invalid Sensor Type */
   ERROR_RESP    = -5 /* Error in report resp msg */
} user_cal_error_e;

/* calibration motion state */
typedef enum
{
  USER_CAL_MOT_STATE_UNKNOWN = 0,
  USER_CAL_MOT_STATE_REST    = 1,
  USER_CAL_MOT_STATE_MOVING  = 2,
  USER_CAL_MOT_STATE_NO_VAR  = 3,
  USER_CAL_MOT_STATE_HI_BIAS = 4
} user_cal_mot_state_e;

/* calibration configuration */
typedef struct
{
   int32_t sampleRate;         // sample rate in Hz, Q16
   int32_t numSamples;         // number of samples
   int64_t varianceThold;      // stationary detect variance threshold
   q16_t   bias_thresholds[3];
} user_cal_config_s;

/* calibration input */
typedef struct
{
   int32_t data[USER_CAL_COL]; // user measurement, rad/s, Q16
} user_cal_input_s;

/* calibration output */
typedef struct
{
   int32_t bias[USER_CAL_COL]; // estimated user bias
   user_cal_mot_state_e motionState;
} user_cal_output_s;

/* calibration state */
typedef struct
{
   user_cal_config_s config;           // user calibration configuration
   int64_t variance[USER_CAL_COL];     // sensor variance
   int64_t sampleSqSum[USER_CAL_COL];  // sum of square of sensor sample data
   int32_t sampleSum[USER_CAL_COL];    // sum of sensor sample data
   uint8_t sampleCount;                // count of sensor samples
} user_cal_state_s;

typedef struct
{
  sensor1_handle_s *sensor1_handle;
  bool is_cb_arrived; /* If a RESP has been received yet */
  pthread_mutex_t cb_mutex; /* Wait for RESP */
  pthread_cond_t cb_cond; /* Wait for RESP */
  int32_t *bias_result;       /* result of algorithm */
  int8_t error;   /* Result of algorithm: user_cal_error_e */
  uint32_t report_id; /* ReportID as per sns_smgr_periodic_report_req_msg_v01 */
  user_cal_state_s state;
} user_cal_control_s;

/*============================================================================
  Static Variable Definitions
  ==========================================================================*/

static user_cal_control_s *g_user_cal_control;

/*============================================================================
                    FORWARD FUNCTION DECLARATIONS
  ==========================================================================*/

void
cal_algo_sensor1_cb(intptr_t data, sensor1_msg_header_s *msg_hdr,
                  sensor1_msg_type_e msg_type, void *msg_ptr);

/*============================================================================
  Static Function Definitions and Documentation
  ==========================================================================*/

/**
 * Calls sensor1_close.  Prints an error message upon error.
 *
 * @return 0 upon success; <0 indicates an error.
 */
static user_cal_error_e
close_sensor1()
{
  sensor1_error_e error;
  g_user_cal_control->report_id++;

  error = sensor1_close( g_user_cal_control->sensor1_handle );
  if( SENSOR1_SUCCESS != error )
  {
    LOGE( "%s: sensor1_close returned %d", __FUNCTION__, error );
    return ERROR_SENSOR1;
  }
  return ERROR_SUCCESS;
}

/**
  Execute the algorithm in the specified state using specified input
  to generate the output at the specified location

  @param[i] state: pointer to algorithm state
  @param[i] input: pointer to algorithm input
  @param[i] output: pointer to algorithm output
*/
static void
user_cal_scm_update( user_cal_state_s *state, user_cal_input_s *input,
   user_cal_output_s *output)
{
  uint8_t i;
  user_cal_config_s *config = &state->config;
  output->motionState = USER_CAL_MOT_STATE_UNKNOWN;

  if( state->sampleCount == 0 )
  {
    for( i = 0; i < USER_CAL_COL; i++ )
    {
        state->sampleSum[i] = 0;
        state->sampleSqSum[i] = 0;
        state->variance[i] = 0;
    }
  }

  state->sampleCount++;

  for( i = 0; i < USER_CAL_COL; i++ )
  {
    state->sampleSum[i] += input->data[i];
    state->sampleSqSum[i] +=
        ((int64_t)(input->data[i]) * (int64_t)(input->data[i]));
  }

  if( state->sampleCount == state->config.numSamples )
  {
    int64_t varT;

    for( i = 0; i < USER_CAL_COL; i++ )
    {
      varT = (int64_t)(state->sampleSum[i]) * (int64_t)(state->sampleSum[i]);

      state->variance[i] = ( state->sampleSqSum[i] -
          ( varT / config->numSamples ) )  / config->numSamples;

      if( state->variance[i] > state->config.varianceThold )
      {
        LOGW( "%s: Variance exceeded, restarting algorithm. index: %i; variance: %"PRIi64"; variance threshold: %"PRIi64,
            __FUNCTION__, i, state->variance[i], state->config.varianceThold );

        //indicate motion state detected, reset algorithm state
        state->sampleCount = 0;
        output->motionState = USER_CAL_MOT_STATE_MOVING;
        return;
      }
      else if( 0 == state->variance[i] )
      {
        LOGW( "%s: Zero variance found, restarting algorithm. index: %i; variance: %"PRIi64,
            __FUNCTION__, i, state->variance[i] );

        state->sampleCount = 0;
        output->motionState = USER_CAL_MOT_STATE_NO_VAR;
        return;
      }
      else if( FX_ABS( state->sampleSum[i] / config->numSamples ) > state->config.bias_thresholds[i]) {
        LOGW( "%s: High BIAS found, restarting algorithm. index: %i; bias: %i, threshold: %i",
            __FUNCTION__, i,state->sampleSum[i] / config->numSamples, state->config.bias_thresholds[i] );

        state->sampleCount = 0;
        output->motionState = USER_CAL_MOT_STATE_HI_BIAS;
        return;
      }
    }

    output->motionState = USER_CAL_MOT_STATE_REST;
    output->bias[0] = state->sampleSum[0] / config->numSamples;
    output->bias[1] = state->sampleSum[1] / config->numSamples;
    output->bias[2] = state->sampleSum[2] / config->numSamples;
  }
}

/**
 * Adds a request to start streaming from the specified sensor.
 *
 * @param[i] sensor_id ID of the sensor per sns_smgr_api_v01.h
 * @param[i] data_type 0 for PRIMARY, 1 for SECONDARY
 *
 * @return Error code: user_cal_error_e
 *
 * @side-effect g_user_cal_control->cb_mutex will be locked if no error
 *  has occurred.
 */
static int
report_req_add( uint8_t sensor_id, uint8_t data_type )
{
  sensor1_msg_header_s msgHdr;
  sns_smgr_periodic_report_req_msg_v01 *smgr_req;
  sensor1_error_e sensor1_error;
  struct timespec ts; // timestamp to use with timed_wait
  int resp_result;    // Result contained in response
  int result_status;  // Result of timed_wait

  sensor1_error = sensor1_open( &g_user_cal_control->sensor1_handle,
                        cal_algo_sensor1_cb,
                        (intptr_t)(&g_user_cal_control->sensor1_handle) );
  if( SENSOR1_SUCCESS != sensor1_error ) {
    LOGE( "%s: sensor1_open returned %d", __FUNCTION__, sensor1_error );
    return ERROR_SENSOR1;
  }

  sensor1_error = sensor1_alloc_msg_buf( g_user_cal_control->sensor1_handle,
      sizeof(sns_smgr_periodic_report_req_msg_v01),(void**)&smgr_req );
  if( SENSOR1_SUCCESS != sensor1_error )
  {
    LOGE( "%s: sensor1_alloc_msg_buf returned %d",
          __FUNCTION__, sensor1_error );
    close_sensor1( g_user_cal_control->sensor1_handle );
    return ERROR_SENSOR1;
  }

  msgHdr.service_number = SNS_SMGR_SVC_ID_V01;
  msgHdr.msg_id = SNS_SMGR_REPORT_REQ_V01;
  msgHdr.msg_size = sizeof(sns_smgr_periodic_report_req_msg_v01);
  msgHdr.txn_id = g_user_cal_control->report_id;

  /* Create the message */
  smgr_req->Item_len = 1;
  smgr_req->ReportId = g_user_cal_control->report_id;
  smgr_req->Action = SNS_SMGR_REPORT_ACTION_ADD_V01;
  smgr_req->ReportRate = 20;
  smgr_req->BufferFactor = 0;
  smgr_req->Item[0].DataType = data_type;
  smgr_req->Item[0].SensorId = sensor_id;
  smgr_req->Item[0].Sensitivity = 0;
  smgr_req->Item[0].Decimation = SNS_SMGR_DECIMATION_RECENT_SAMPLE_V01;
  smgr_req->Item[0].MinSampleRate = REPORT_RATE;
  smgr_req->Item[0].StationaryOption = SNS_SMGR_REST_OPTION_NO_REPORT_V01;
  smgr_req->Item[0].DoThresholdTest = false;
  smgr_req->Item[0].ThresholdOutsideMinMax = 0;
  smgr_req->Item[0].ThresholdDelta = 0;
  smgr_req->Item[0].ThresholdAllAxes = 0;
  smgr_req->Item[0].ThresholdMinMax[0] = 0;
  smgr_req->Item[0].ThresholdMinMax[1] = 0;

  smgr_req->cal_sel_valid = true;
  smgr_req->cal_sel_len = 1;
  smgr_req->cal_sel[0] = SNS_SMGR_CAL_SEL_RAW_V01;

  pthread_mutex_lock( &g_user_cal_control->cb_mutex );
  g_user_cal_control->state.sampleCount = 0;
  sensor1_error = sensor1_write( g_user_cal_control->sensor1_handle,
                                 &msgHdr, smgr_req );
  if( SENSOR1_SUCCESS != sensor1_error )
  {
    LOGE( "%s: sensor1_write returned %d", __FUNCTION__, sensor1_error );
    sensor1_free_msg_buf( g_user_cal_control->sensor1_handle, smgr_req );
    close_sensor1( g_user_cal_control->sensor1_handle );
    pthread_mutex_unlock( &g_user_cal_control->cb_mutex );
    return ERROR_SENSOR1;
  }

  return ERROR_SUCCESS;
}

/**
 * Indicates to a waiting thread that a response message has been
 * received.  g_user_cal_control->cb_mutex should be held prior
 * to calling this function.
 *
 * @param[i] error >0: Response code contained in the response
 *                  0: Success
 *                 <0: user_cal_error_e
 * @param[i] bias_result Output of the algorithm; NULL if error != 0
 */
static void
signal_response( int error, int32_t *bias_result )
{
  pthread_mutex_lock( &g_user_cal_control->cb_mutex );
  g_user_cal_control->error = error;
  g_user_cal_control->bias_result = bias_result;
  g_user_cal_control->is_cb_arrived = true;
  pthread_cond_signal( &g_user_cal_control->cb_cond );
  pthread_mutex_unlock( &g_user_cal_control->cb_mutex );
}

/*============================================================================
  Externalized Function Definitions
  ==========================================================================*/

/**
 * Call-back functions from sensor1, for when a message has been received.
 * See sensor1.h for parameter details.
 */
void
cal_algo_sensor1_cb(intptr_t data, sensor1_msg_header_s *msg_hdr,
                  sensor1_msg_type_e msg_type, void *msg_ptr)
{
  if( NULL == msg_hdr ) {
    LOGD( "%s: received NULL msg_hdr_ptr. type: %i",
        __FUNCTION__, msg_type );
    return ;
  }

  if( SENSOR1_MSG_TYPE_RESP == msg_type &&
      SNS_SMGR_SVC_ID_V01 == msg_hdr->service_number &&
      SNS_SMGR_REPORT_RESP_V01 == msg_hdr->msg_id )
  {
    sns_smgr_periodic_report_resp_msg_v01 *msgPtr =
        (sns_smgr_periodic_report_resp_msg_v01*) msg_ptr;

    if( msg_hdr->txn_id != g_user_cal_control->report_id )
    {
      LOGE( "%s: Wrong transaction ID: %i, %i",
            __FUNCTION__, msg_hdr->txn_id, g_user_cal_control->report_id );
    }
    if( msgPtr->ReportId != g_user_cal_control->report_id )
    {
      LOGE( "%s: Wrong report ID: %i, %i",
            __FUNCTION__, msgPtr->ReportId, g_user_cal_control->report_id );
    }
    else if( SNS_RESULT_FAILURE_V01 == msgPtr->Resp.sns_result_t )
    {
      LOGE( "%s: Response error: %i",
            __FUNCTION__, msgPtr->Resp.sns_err_t );
      signal_response( ERROR_RESP, NULL );
    }
    else
    {
      // No errors in response
    }
  }
  else if( SENSOR1_MSG_TYPE_IND == msg_type &&
           SNS_SMGR_SVC_ID_V01 == msg_hdr->service_number &&
           SNS_SMGR_REPORT_IND_V01 == msg_hdr->msg_id )
  {
    sns_smgr_periodic_report_ind_msg_v01 *msgPtr =
        (sns_smgr_periodic_report_ind_msg_v01*) msg_ptr;
    sns_smgr_data_item_s_v01 *dataItem = &msgPtr->Item[0];

    if( msgPtr->ReportId != g_user_cal_control->report_id )
    {
      LOGE( "%s: Wrong report ID: %i, %i",
            __FUNCTION__, msgPtr->ReportId, g_user_cal_control->report_id );
    }
    else if( 0 != (dataItem->ItemFlags & SNS_SMGR_ITEM_FLAG_INVALID_V01) )
    {
      LOGE( "%s: Item flag error: %i",
            __FUNCTION__, dataItem->ItemFlags );
    }
    else
    {
      user_cal_input_s input;
      user_cal_output_s output;

      input.data[0] = dataItem->ItemData[0];
      input.data[1] = dataItem->ItemData[1];
      // PENDING: Adding G only applies to ACCEL,should make more general
      input.data[2] = dataItem->ItemData[2] + FX_FLTTOFIX_Q16(G);
      user_cal_scm_update( &g_user_cal_control->state, &input, &output );

      if( USER_CAL_MOT_STATE_REST == output.motionState )
      {
        int32_t *bias_result = malloc( sizeof(int32_t) * 3 );
        if( NULL == bias_result )
        {
          LOGE( "%s: Malloc error", __FUNCTION__ );
          signal_response( ERROR_BIAS, NULL );
        }
        else
        {
          bias_result[0] = -output.bias[0];
          bias_result[1] = -output.bias[1];
          bias_result[2] = -output.bias[2];
          signal_response( ERROR_SUCCESS, bias_result );
        }
      }
    }
  }
  else
  {
    LOGW( "%s: Unsupported message.  type: %i; svc_id: %i; msg_id: %i",
          __FUNCTION__, msg_type, msg_hdr->service_number,
          msg_hdr->msg_id );
  }

  if( NULL != msg_ptr ) {
    sensor1_free_msg_buf( *((sensor1_handle_s**)data), msg_ptr );
  }
}

/**
 * Calculates and returns the sensor bias.
 *
 * @param[i] sensor_id ID of the sensor per sns_smgr_api_v01.h
 * @param[i] data_type 0 for PRIMARY, 1 for SECONDARY
 * @param[o] bias_result Bias calculated
 *
 * @return >=0: Length of bias array
 *          <0: Error code: user_cal_error_e.
 *  If error occurred, do not use bias_zero
 */
int
calc_sensor_bias( uint8_t sensor_type, uint8_t data_type, int32_t **bias_result )
{
  int result = ERROR_SUCCESS;
  struct timespec ts; // timestamp to use with timed_wait
  int result_status;  // Result of timed_wait

  if( 0 != sensor_type ) // Only ACCEL presently supported
  {
    return ERROR_STYPE;
  }

  result = report_req_add( sensor_type, data_type );
  if( ERROR_SUCCESS != result )
  {
    return result;
  }

  clock_gettime(CLOCK_REALTIME, &ts);
  ts.tv_sec += 10;

  /* wait for response */
  while( 1 != g_user_cal_control->is_cb_arrived )
  {
    result_status = pthread_cond_timedwait( &g_user_cal_control->cb_cond,
        &g_user_cal_control->cb_mutex, &ts );
    if( ETIMEDOUT == result_status )
    {
      LOGE( "%s: Request timed-out", __FUNCTION__ );
      g_user_cal_control->error = ERROR_TIMEOUT;
      break;
    }
  }

  result = g_user_cal_control->error;
  g_user_cal_control->is_cb_arrived = 0;
  *bias_result = g_user_cal_control->bias_result;
  close_sensor1( g_user_cal_control->sensor1_handle );

  pthread_mutex_unlock( &g_user_cal_control->cb_mutex );

  return ERROR_SUCCESS == result ? 3 : result;
}

/**
 *  Initializes variables necessary for calc_sensor_bias.
 */
void
user_cal_init()
{
  /* Initalize the global variables */
  g_user_cal_control = malloc( sizeof(user_cal_control_s) );
  if( NULL == g_user_cal_control )
  {
    LOGE( "%s: Malloc failure", __FUNCTION__ );
    return -1;
  }

  g_user_cal_control->bias_result = NULL;
  g_user_cal_control->is_cb_arrived = false;
  g_user_cal_control->sensor1_handle = NULL;
  pthread_mutex_init( &g_user_cal_control->cb_mutex, NULL );
  pthread_cond_init( &g_user_cal_control->cb_cond, NULL );
  g_user_cal_control->report_id = 0;
  g_user_cal_control->error = 0;

  g_user_cal_control->state.config.sampleRate = REPORT_RATE;
  g_user_cal_control->state.config.numSamples = 64;

  // Threshold values only correct for 6050 Accel
  g_user_cal_control->state.config.varianceThold = (int64_t)(FX_CONV(FX_FLTTOFIX_Q16(0.000944),16,32));
  g_user_cal_control->state.config.bias_thresholds[0] = FX_FLTTOFIX_Q16(0.15*G);  // x-axis bias
  g_user_cal_control->state.config.bias_thresholds[1] = FX_FLTTOFIX_Q16(0.15*G);  // y-axis bias
  g_user_cal_control->state.config.bias_thresholds[2] = FX_FLTTOFIX_Q16(0.15*G);  // z-axis bias
}
