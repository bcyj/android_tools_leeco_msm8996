/*==========================================================================
@file sensor_test.c

@brief
JNI Native code containing several functions to be called from a java class.
Run sensor tests, or toggle whether raw data is sent to Android.

Copyright (c) 2011-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
==========================================================================*/

#define LOG_TAG "sensor_test"

#include "sensor_test.h"
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

/* Global variables.  Purpose: communication between runSensorTest and
 * handleRcvMsg
 */
static sensor1_handle_s *test_sensor1_handle;
static int test_result = 0; /* Pass result from rcv() to get() */
static int test_predicate = 0;         /* Waiting for RESP message */
static pthread_mutex_t test_mutex;     /* Only one request at a time */
static pthread_cond_t test_cond;       /* Waiting for RESP message */
static uint8_t transaction_id;

/**
 * Indicates that the appropriate message has been received,
 * and sets the message result, so that the primary function
 * can continue.
 *
 * @param cond The condition variable that the
 *             primary function is waiting on.
 * @param predicate Boolean; indicated whether the message has been
 *             received.
 * @param return_result Variable the primary function will read to
 *             determine the result.
 * @param result The result of the messages received.
 */
static void
rcv_complete(pthread_cond_t *cond, int *predicate,
                   int *return_result, int result)
{
   pthread_mutex_lock( &test_mutex );
   *return_result = result;
   *predicate = 1;
   pthread_cond_signal( cond );
   pthread_mutex_unlock( &test_mutex );
}

/**
 * Calls sensor1_close.  Prints an error message upon error.
 *
 * @param sensor1_handle Handle as returned from sensor1_open.
 * @return 0 upon success; <0 indicates an error.
 */
static int
close_sensor1(sensor1_handle_s *sensor1_handle)
{
   sensor1_error_e error;
   transaction_id++;

   error = sensor1_close( sensor1_handle );
   if( SENSOR1_SUCCESS != error )
   {
      LOGE( "%s: sensor1_close returned %d", __FUNCTION__, error );
      return -1;
   }
   return 0;
}

/**
 * Call-back functions from sensor1, for when a message has been received.
 * See sensor1.h for parameter details.
 */
void
rcv_test_msg(intptr_t data, sensor1_msg_header_s *msg_hdr_ptr,
                  sensor1_msg_type_e msg_type, void *msg_ptr)
{
   if( NULL == msg_hdr_ptr ) {
      LOGD( "%s: received NULL msg_hdr_ptr! type: %i",
            __FUNCTION__, msg_type );
      return ;
   }

   // LOGE( "%s: Received msg w/ txn_id=%i", __FUNCTION__, msg_hdr_ptr->txn_id );

   if( SENSOR1_MSG_TYPE_RESP == msg_type ) // SNS_SMGR_SINGLE_SENSOR_TEST_RESP_V01
   {
      sns_smgr_single_sensor_test_resp_msg_v01 *msgPtr =
         (sns_smgr_single_sensor_test_resp_msg_v01*) msg_ptr;

      if( msg_hdr_ptr->txn_id != transaction_id )
      {
         LOGE( "%s: Wrong transaction ID: %i, %i",
            __FUNCTION__, msg_hdr_ptr->txn_id, transaction_id );
      }
      else if( SNS_RESULT_FAILURE_V01 == msgPtr->Resp.sns_result_t )
      {
         /* If there is an error is the common response,
          * do not expect an indication */
         LOGE( "%s: Received error in common response: %i",
               __FUNCTION__, msgPtr->Resp.sns_err_t );
         rcv_complete( &test_cond, &test_predicate, &test_result, -1 );
      }
      else if( SNS_SMGR_TEST_STATUS_SUCCESS_V01 == msgPtr->TestStatus )
      {
         rcv_complete( &test_cond, &test_predicate, &test_result, 0 );
      }
      else if( SNS_SMGR_TEST_STATUS_PENDING_V01 != msgPtr->TestStatus )
      {
         // Instead of returning this error code, let us wait to see
         // if the IND msg returns a more detailed one...
         test_result = -msgPtr->TestStatus - 10;
      }
   }
   else if( SENSOR1_MSG_TYPE_IND == msg_type ) // SNS_SMGR_SINGLE_SENSOR_TEST_IND_V01
   {
      sns_smgr_single_sensor_test_ind_msg_v01 *msgPtr =
         (sns_smgr_single_sensor_test_ind_msg_v01*) msg_ptr;

      if( SNS_SMGR_TEST_RESULT_PASS_V01 == msgPtr->TestResult )
      {
         rcv_complete( &test_cond, &test_predicate, &test_result, 0 );
      }
      else if( true == msgPtr->ErrorCode_valid )
      {
         LOGE( "%s: Received error in IND message: %i",
               __FUNCTION__, msgPtr->ErrorCode );
         rcv_complete( &test_cond, &test_predicate, &test_result,
                       msgPtr->ErrorCode );
      }
      else
      {
        LOGE( "%s: Message indicates error w/o code (%i)",
            __FUNCTION__, test_result );
        rcv_complete( &test_cond, &test_predicate, &test_result,
            test_result );
      }
   }
   else if( SENSOR1_MSG_TYPE_BROKEN_PIPE == msg_type )
   {
      LOGE( "%s: Broken message pipe", __FUNCTION__ );
      rcv_complete( &test_cond, &test_predicate, &test_result, -21 );
   }
   else if( SENSOR1_MSG_TYPE_RESP_INT_ERR == msg_type )
   {
      LOGE( "%s: Response - Internal Error", __FUNCTION__ );
      rcv_complete( &test_cond, &test_predicate, &test_result, -22 );
   }
   else if( SENSOR1_MSG_TYPE_RETRY_OPEN == msg_type )
   {
      LOGE( "%s: Received retry open", __FUNCTION__ );
      rcv_complete( &test_cond, &test_predicate, &test_result, -21 );
   }
   else
   {
      LOGW( "%s: Received unknown message type: %d",
            __FUNCTION__, msg_type );
   }

   if( NULL != msg_ptr ) {
      sensor1_free_msg_buf( *((sensor1_handle_s**)data), msg_ptr );
   }
}

JNIEXPORT jint JNICALL
Java_com_qualcomm_sensors_sensortest_SensorTest_runNativeSensorTest__IIIZZ
  ( JNIEnv *ENV, jclass class, jint sensorID, jint dataType,
   jint testType, jboolean saveToRegistry, jboolean applyCalNow)
{
   sensor1_msg_header_s msgHdr;
   sns_smgr_single_sensor_test_req_msg_v01 *msgPtr;
   sensor1_error_e error;
   struct timespec ts;
   int resultStatus;
   int testResult;

   test_result = -1;
   error = sensor1_open( &test_sensor1_handle,
                         rcv_test_msg,
                         (intptr_t)(&test_sensor1_handle) );
   if( SENSOR1_SUCCESS != error ) {
      LOGE( "%s: sensor1_open returned %d", __FUNCTION__, error );
      return -1;
   }

   error = sensor1_alloc_msg_buf( test_sensor1_handle,
              sizeof(sns_smgr_single_sensor_test_req_msg_v01),
              (void**)&msgPtr );
   if( SENSOR1_SUCCESS != error )
   {
      LOGE( "%s: sensor1_alloc_msg_buf returned %d",
            __FUNCTION__, error );
      close_sensor1(test_sensor1_handle);
      return -1;
   }

   msgHdr.service_number = SNS_SMGR_SVC_ID_V01;
   msgHdr.msg_id = SNS_SMGR_SINGLE_SENSOR_TEST_REQ_V01;
   msgHdr.msg_size = sizeof(sns_smgr_single_sensor_test_req_msg_v01);
   msgHdr.txn_id = transaction_id;

   /* Create the message */
   msgPtr->SensorID = sensorID;
   msgPtr->DataType = dataType;
   msgPtr->TestType = testType;
   msgPtr->SaveToRegistry_valid = true;
   msgPtr->SaveToRegistry = saveToRegistry;
   msgPtr->ApplyCalNow_valid = true;
   msgPtr->ApplyCalNow = applyCalNow;

   error = sensor1_write( test_sensor1_handle, &msgHdr, msgPtr );
   if( SENSOR1_SUCCESS != error )
   {
      LOGE( "%s: sensor1_write returned %d", __FUNCTION__, error );
      sensor1_free_msg_buf( test_sensor1_handle, msgPtr );
      close_sensor1( test_sensor1_handle );
      return -1;
   }

   clock_gettime(CLOCK_REALTIME, &ts);
   ts.tv_sec += 10;

   pthread_mutex_lock( &test_mutex );
   /* wait for response */
   while( 1 != test_predicate )
   {
      resultStatus = pthread_cond_timedwait( &test_cond, &test_mutex, &ts );
      if( resultStatus == ETIMEDOUT )
      {
         if( 0 != test_result && -1 != test_result )
         {
            // Being here means the RESP message included an error code,
            // but we never received an IND to confirm it.
            LOGE( "%s: Time-out: %i", __FUNCTION__, test_result );
            break;
         }
         else
         {
            LOGE( "%s: Sensor Test request timed-out", __FUNCTION__ );
            rcv_complete( &test_cond, &test_predicate, &test_result, -3 );
         }
      }
   }
   test_predicate = 0;
   testResult = test_result;
   close_sensor1(test_sensor1_handle);
   pthread_mutex_unlock( &test_mutex );

   return testResult;
}

JNIEXPORT jint JNICALL
Java_com_qualcomm_sensors_sensortest_SensorTest_runNativeSensorTest__III
  (JNIEnv *ENV, jclass class, jint sensorID, jint dataType, jint testType)
{
  return Java_com_qualcomm_sensors_sensortest_SensorTest_runNativeSensorTest__IIIZZ(
      ENV, class, sensorID, dataType, testType, true, true );
}

JNIEXPORT jint JNICALL
Java_com_qualcomm_sensors_sensortest_SensorTest_getNativeRawDataMode
  (JNIEnv *ENV, jclass class)
{
   int error = 0;
   uint8_t *data, data_len = NULL;
   int result = 0;

   error = sensor_reg_open();
   if( 0 != error )
   {
      LOGE( "%s: Error in sensor_reg_open(): %i",
            __FUNCTION__, error );
      return error;
   }

   error = sensor_reg_read( (uint16_t)50, 10, &data, &data_len );
   if( 0 != error )
   {
      LOGE( "%s: Error in sensor_reg_read(): %i",
            __FUNCTION__, error );
      return error;
   }
   else if( NULL == data || 0 == data_len )
   {
      LOGE( "%s: Invalid output from sensor_reg_read", __FUNCTION__ );
      return -1;
   }
   else
   {
      result = data[0];
   }
   free( data );

   error = sensor_reg_close();
   if( 0 != error )
   {
      LOGE( "%s: Error in sensor_reg_close(): %i",
            __FUNCTION__, error );
      return error;
   }

   return result;
}

JNIEXPORT jint JNICALL
Java_com_qualcomm_sensors_sensortest_SensorTest_setNativeRawDataMode
  (JNIEnv *ENV, jclass class, jboolean enabled)
{
   int error = 0;
   uint8_t rdm_value = (JNI_TRUE == enabled) ? 1 : 0;
   uint8_t cal_value = (JNI_TRUE == enabled) ? 0 : 1;

   error = sensor_reg_open();
   if( 0 != error )
   {
      LOGE( "%s: Error in sensor_reg_open(): %i",
            __FUNCTION__, error );
      return error;
   }

   error = sensor_reg_write( SNS_REG_ITEM_RAW_DATA_MODE_V02, &rdm_value, 1, 10 );
   if( 0 != error )
   {
      LOGE( "%s: Error in sensor_reg_write(): %i",
            __FUNCTION__, error );
      return error;
   }

   error = sensor_reg_write( SNS_REG_ITEM_GYRO_CAL_DEF_ENABLE_ALGO_V02, &cal_value, 1, 10 );
   if( 0 != error )
   {
      LOGE( "%s: Error in sensor_reg_write(): %i",
            __FUNCTION__, error );
      return error;
   }

   error = sensor_reg_close();
   if( 0 != error )
   {
      LOGE( "%s: Error in sensor_reg_close(): %i",
            __FUNCTION__, error );
      return error;
   }

   return 0;
}

/*
 * Array of methods.
 *
 * Each entry has three fields: the name of the method, the method
 * signature, and a pointer to the native implementation.
 */
static const JNINativeMethod gMethods[] = {
   { "runNativeSensorTest",
     "(III)I",
     (void*)Java_com_qualcomm_sensors_sensortest_SensorTest_runNativeSensorTest__III
   },
   { "runNativeSensorTest",
     "(IIIZZ)I",
     (void*)Java_com_qualcomm_sensors_sensortest_SensorTest_runNativeSensorTest__IIIZZ
   },
   { "setNativeRawDataMode",
     "(Z)I",
     (void*)Java_com_qualcomm_sensors_sensortest_SensorTest_setNativeRawDataMode
   },
   { "getNativeRawDataMode",
     "()I",
     (void*)Java_com_qualcomm_sensors_sensortest_SensorTest_getNativeRawDataMode
   }
};

/*
 * Explicitly register all methods for our class.
 *
 * Returns 0 on success.
 */
static int
registerMethods(JNIEnv* env)
{
   static const char* const kClassName =
      "com/qualcomm/sensors/sensortest/SensorTest";
   jclass class;

   /* look up the class */
   class = (*env)->FindClass( env, kClassName );
   if(class == NULL)
   {
      LOGI( "%s: Can't find class %s", __FUNCTION__, kClassName );
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

/*
 * This is called by the VM when the shared library is first loaded.
 */
jint
JNI_OnLoad(JavaVM* vm, void* reserved)
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
   test_result = 0;
   pthread_mutex_init( &test_mutex, NULL );
   pthread_cond_init( &test_cond, NULL );
   transaction_id = 0;

   return JNI_VERSION_1_4;
}
