/*============================================================================
@file sensor_user_cal.c

@brief
JNI Native code which permits an Android app to send user calibration
messages to SMGR.

Copyright (c) 2011-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

/*============================================================================
  INCLUDE FILES
  ==========================================================================*/

#define LOG_TAG "sensor_user_cal"

#include "sensor_user_cal.h"
#include "sensor_reg.h"
#include "sns_reg_api_v02.h"
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

/*============================================================================
  Useful macros
  ==========================================================================*/
#define FX_QFACTOR           (16)
#define FX_FLTTOFIX(f,q)     ((int32_t)( (f)*(1<<(q))+(((f)>(0.0))?(0.5):(-0.5))))
#define FX_FLTTOFIX_Q16(d)   (FX_FLTTOFIX(d,FX_QFACTOR))

/*============================================================================
  Type Declarations
  ==========================================================================*/

typedef struct
{
  sensor1_handle_s *sensor1_handle;
  bool is_cb_arrived; /* If a RESP has been received yet */
  pthread_mutex_t cb_mutex; /* Wait for RESP */
  pthread_cond_t cb_cond; /* Wait for RESP */
  int resp_result;  /* Result of sensor cal message */
  uint32_t txn_id;  /* transaction id as per sensor1_msg_header_s */
} user_cal_control_s;

typedef enum user_cal_error_e {
   USER_CAL_SUCCESS = 0,  /* No error */
   USER_CAL_TIMEOUT = -1, /* Response error - timeout */
   USER_CAL_SENSOR1 = -2, /* Sensor1 Error */
   USER_CAL_BIAS    = -3, /* Error using bias */
   USER_CAL_REG     = -4 /* Error writing to registry */
} user_cal_error_e;

/*============================================================================
  Static Variable Definitions
  ==========================================================================*/

static user_cal_control_s *g_user_cal_control;

/*
 * Array of methods.
 *
 * Each entry has three fields: the name of the method, the method
 * signature, and a pointer to the native implementation.
 */
static const JNINativeMethod gMethods[] = {
  { "performUserCal",
    "(BB)I",
    (void*)Java_com_qualcomm_sensors_sensortest_SensorUserCal_performUserCal
  }
};

/*============================================================================
                    FORWARD FUNCTION DECLARATIONS
  ==========================================================================*/
void
cal_sensor1_cb(intptr_t data, sensor1_msg_header_s *msg_hdr,
                  sensor1_msg_type_e msg_type, void *msg_ptr);

void
user_cal_init();

int
calc_sensor_bias( uint32_t, uint32_t, int32_t** );

/*============================================================================
  Static Function Definitions and Documentation
  ===========================================================================*/

/**
 * Calls sensor1_close.  Prints an error message upon error.
 *
 * @return 0 upon success; <0 indicates an error.
 */
static user_cal_error_e
close_sensor1()
{
  sensor1_error_e error;
  g_user_cal_control->txn_id++;

  error = sensor1_close( g_user_cal_control->sensor1_handle );
  if( SENSOR1_SUCCESS != error )
  {
    LOGE( "%s: sensor1_close returned %d", __FUNCTION__, error );
    return USER_CAL_SENSOR1;
  }
  return USER_CAL_SUCCESS;
}

/*
 * Explicitly register all methods for our class.
 *
 * @return 0 on success.
 */
static int
registerMethods(JNIEnv* env)
{
  static const char* const kClassName =
      "com/qualcomm/sensors/sensortest/SensorUserCal";
  jclass class;

  /* look up the class */
  class = (*env)->FindClass( env, kClassName );
  if(class == NULL)
  {
    LOGE( "%s: Can't find class %s", __FUNCTION__, kClassName );
    return -1;
  }

  /* register all the methods */
  if( (*env)->RegisterNatives( env, class, gMethods,
      sizeof(gMethods) / sizeof(gMethods[0]) ) != JNI_OK )
  {
    LOGE( "%s: Failed registering methods for %s",
          __FUNCTION__, kClassName );
    return -1;
  }

  return 0;
}

/**
 * Indicates to a waiting thread that a response message has been
 * received.b  g_user_cal_control->cb_mutex should be held prior
 * to calling this function.
 *
 * @param[i] result >=0: Response code contained in the response
 *                <0: user_cal_error_e
 */
static void
signal_response( int result )
{
  pthread_mutex_lock( &g_user_cal_control->cb_mutex );
  g_user_cal_control->resp_result = result;
  g_user_cal_control->is_cb_arrived = true;
  pthread_cond_signal( &g_user_cal_control->cb_cond );
  pthread_mutex_unlock( &g_user_cal_control->cb_mutex );
}

/**
 * Sends to SMGR a sensor calibration message
 *
 * @param[i] usage See sns_smgr_api_v01.h
 * @param[i] sensor_id See sns_smgr_api_v01.h
 * @param[i] data_type See sns_smgr_api_v01.h
 * @param[i] zero_bias_len See sns_smgr_api_v01.h
 * @param[i] zero_bias See sns_smgr_api_v01.h
 * @param[i] scale_factor_len See sns_smgr_api_v01.h
 * @param[i] scale_factor See sns_smgr_api_v01.h
 *
 * @return Error code according to user_cal_error_e.
 */
static int
req_sensor_cal( uint8_t usage, uint8_t sensor_id, uint8_t data_type,
    uint8_t zero_bias_len, int32_t *zero_bias,
    uint8_t scale_factor_len, uint32_t *scale_factor)
{
  sensor1_msg_header_s msgHdr;
  sns_smgr_sensor_cal_req_msg_v01 *msgPtr;
  sensor1_error_e sensor1_error;
  struct timespec ts; // timestamp to use with timed_wait
  int resp_result;    // Result contained in response
  int result_status;  // Result of timed_wait

  sensor1_error = sensor1_open( &g_user_cal_control->sensor1_handle,
                        cal_sensor1_cb,
                        (intptr_t)(&g_user_cal_control->sensor1_handle) );
  if( SENSOR1_SUCCESS != sensor1_error ) {
    LOGE( "%s: sensor1_open returned %d", __FUNCTION__, sensor1_error );
    return USER_CAL_SENSOR1;
  }

  sensor1_error = sensor1_alloc_msg_buf( g_user_cal_control->sensor1_handle,
      sizeof(sns_smgr_sensor_cal_req_msg_v01),(void**)&msgPtr );
  if( SENSOR1_SUCCESS != sensor1_error )
  {
    LOGE( "%s: sensor1_alloc_msg_buf returned %d",
          __FUNCTION__, sensor1_error );
    close_sensor1( g_user_cal_control->sensor1_handle );
    return USER_CAL_SENSOR1;
  }

  msgHdr.service_number = SNS_SMGR_SVC_ID_V01;
  msgHdr.msg_id = SNS_SMGR_CAL_REQ_V01;
  msgHdr.msg_size = sizeof(sns_smgr_sensor_cal_req_msg_v01);
  msgHdr.txn_id = g_user_cal_control->txn_id;

  /* Create the message */
  msgPtr->usage = usage;
  msgPtr->SensorId = sensor_id;
  msgPtr->DataType = data_type;
  msgPtr->ZeroBias_len = zero_bias_len;
  msgPtr->ScaleFactor_len = scale_factor_len;

  memcpy( msgPtr->ZeroBias, zero_bias, zero_bias_len * sizeof(int32_t) );
  memcpy( msgPtr->ScaleFactor, scale_factor, scale_factor_len * sizeof(uint32_t) );

  pthread_mutex_lock( &g_user_cal_control->cb_mutex );
  sensor1_error = sensor1_write( g_user_cal_control->sensor1_handle,
                                 &msgHdr, msgPtr );
  if( SENSOR1_SUCCESS != sensor1_error )
  {
    LOGE( "%s: sensor1_write returned %d", __FUNCTION__, sensor1_error );
    sensor1_free_msg_buf( g_user_cal_control->sensor1_handle, msgPtr );
    close_sensor1( g_user_cal_control->sensor1_handle );
    pthread_mutex_unlock( &g_user_cal_control->cb_mutex );
    return USER_CAL_SENSOR1;
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
      g_user_cal_control->resp_result = USER_CAL_TIMEOUT;
      break;
    }
  }

  g_user_cal_control->is_cb_arrived = 0;
  resp_result = g_user_cal_control->resp_result;
  close_sensor1( g_user_cal_control->sensor1_handle );

  pthread_mutex_unlock( &g_user_cal_control->cb_mutex );
  return resp_result;
}

/*============================================================================
  Externalized Function Definitions
  ==========================================================================*/

/**
 * Call-back functions from sensor1, for when a message has been received.
 * See sensor1.h for parameter details.
 */
void
cal_sensor1_cb(intptr_t data, sensor1_msg_header_s *msg_hdr,
                  sensor1_msg_type_e msg_type, void *msg_ptr)
{
  if( NULL == msg_hdr ) {
    LOGD( "%s: received NULL msg_hdr_ptr. type: %i",
        __FUNCTION__, msg_type );
    return ;
  }

  if( SENSOR1_MSG_TYPE_RESP == msg_type &&
      SNS_SMGR_SVC_ID_V01 == msg_hdr->service_number &&
      SNS_SMGR_CAL_RESP_V01 == msg_hdr->msg_id )
  {
    sns_smgr_sensor_cal_resp_msg_v01 *msgPtr =
        (sns_smgr_sensor_cal_resp_msg_v01*) msg_ptr;

    if( msg_hdr->txn_id != g_user_cal_control->txn_id )
    {
      LOGE( "%s: Wrong transaction ID: %i, %i",
            __FUNCTION__, msg_hdr->txn_id, g_user_cal_control->txn_id );
    }
    else
    {
      signal_response( msgPtr->Resp.sns_result_t == 0 ? 0 :
          msgPtr->Resp.sns_err_t );
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

JNIEXPORT jint
JNICALL Java_com_qualcomm_sensors_sensortest_SensorUserCal_performUserCal
  ( JNIEnv *env, jclass class, jbyte sensor_type, jbyte data_type )
{
  int32_t *zero_bias;
  int bias_len = calc_sensor_bias( sensor_type, data_type, &zero_bias );
  uint32_t scale_factor[] = { FX_FLTTOFIX_Q16(1.0), FX_FLTTOFIX_Q16(1.0),
                              FX_FLTTOFIX_Q16(1.0)} ;
  int result;

  if( 0 > bias_len )
  {
    LOGE( "%s: Error occurred when calculating bias: %i",
          __FUNCTION__, bias_len );
    return USER_CAL_BIAS;
  }

   result = sensor_reg_open();
   if( 0 != result )
   {
      LOGE( "%s: Error in sensor_reg_open: %i", __FUNCTION__, result );
      return USER_CAL_REG;
   }

   result = sensor_reg_write( SNS_REG_ITEM_ACC_X_BIAS_V02, (uint8_t*)&zero_bias[0], 4, 10 );
   if( 0 != result )
   {
      LOGE( "%s: Error in sensor_reg_write: %i", __FUNCTION__, result );
      return USER_CAL_REG;
   }
   result = sensor_reg_write( SNS_REG_ITEM_ACC_Y_BIAS_V02, (uint8_t*)&zero_bias[1], 4, 10 );
   if( 0 != result )
   {
      LOGE( "%s: Error in sensor_reg_write: %i", __FUNCTION__, result );
      return USER_CAL_REG;
   }
   result = sensor_reg_write( SNS_REG_ITEM_ACC_Z_BIAS_V02, (uint8_t*)&zero_bias[2], 4, 10 );
   if( 0 != result )
   {
      LOGE( "%s: Error in sensor_reg_write: %i", __FUNCTION__, result );
      return USER_CAL_REG;
   }

   result = sensor_reg_close();
   if( 0 != result )
   {
      LOGE( "%s: Error in sensor_reg_close: %i", __FUNCTION__, result );
      return USER_CAL_REG;
   }

  result = req_sensor_cal( 2, sensor_type, data_type, bias_len,
      zero_bias, 3, scale_factor);

  free( zero_bias );
  return result;
}

JNIEXPORT jint JNICALL
Java_com_qualcomm_sensors_sensortest_SensorUserCal_reqSensorCal
( JNIEnv *env, jclass class, jbyte usage, jbyte sensor_id, jbyte data_type,
 jintArray zero_bias, jintArray scale_factor )
{
  uint32_t *scale_factor_temp;
  int32_t *zero_bias_temp;
  int result;

  zero_bias_temp = (int32_t*)(*env)->GetByteArrayElements( env, zero_bias, NULL );
  scale_factor_temp = (uint32_t*)(*env)->GetByteArrayElements( env, scale_factor, NULL );

  result = req_sensor_cal( (uint8_t)usage, (uint8_t)sensor_id, (uint8_t)data_type,
      (uint8_t)(*env)->GetArrayLength( env, zero_bias ), zero_bias_temp,
      (uint8_t)(*env)->GetArrayLength( env, scale_factor ), scale_factor_temp);

  (*env)->ReleaseByteArrayElements( env, zero_bias, (jbyte *)zero_bias_temp, JNI_ABORT );
  (*env)->ReleaseByteArrayElements( env, scale_factor, (jbyte *)scale_factor_temp, JNI_ABORT );

  return result;
}

/*
 * This is called by the VM when the shared library is first loaded.
 */
jint
JNI_OnLoad( JavaVM* vm, void* reserved )
{
   JNIEnv* env = NULL;
   if( (*vm)->GetEnv( vm, (void**) &env, JNI_VERSION_1_4 ) != JNI_OK )
   {
      LOGE( "%s: GetEnv failed", __FUNCTION__ );
      return -1;
   }

   if( registerMethods( env ) != 0 )
   {
      LOGE( "%s: native registration failed", __FUNCTION__ );
      return -1;
   }

   /* Initalize the global variables */
   g_user_cal_control = malloc( sizeof(user_cal_control_s) );
   if( NULL == g_user_cal_control )
   {
      LOGE( "%s: Malloc failure", __FUNCTION__ );
      return -1;
   }

   g_user_cal_control->resp_result = 0;
   g_user_cal_control->is_cb_arrived = false;
   g_user_cal_control->sensor1_handle = NULL;
   pthread_mutex_init( &g_user_cal_control->cb_mutex, NULL );
   pthread_cond_init( &g_user_cal_control->cb_cond, NULL );
   g_user_cal_control->txn_id = 0;

   user_cal_init();

   return JNI_VERSION_1_4;
}
