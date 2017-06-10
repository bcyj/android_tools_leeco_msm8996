/*==========================================================================
@file sensor_reg.c

@brief
Native code to write or read a value from the sensors registry.

Copyright (c) 2011-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
==========================================================================*/

#define LOG_TAG "sensor_reg"

#include "sensor_reg.h"
#include "sensor1.h"
#include "sns_reg_api_v02.h"
#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include <stdlib.h>
#include <utils/Log.h>
#include <common_log.h>
#include <stdbool.h>

/*============================================================================
  Internal Definitions
  ==========================================================================*/
#define SENSOR_REG_TIMEOUT 10

/*============================================================================
  Type Declarations
  ==========================================================================*/

typedef struct sensor_reg_req_s{
  bool process_error; /* Error processing RESP */
  pthread_mutex_t cb_mutex; /* Waiting for the RESP */
  pthread_cond_t cb_cond; /* Waiting for the RESP */
  bool is_cb_arrived; /* Whether the response has been received */
  uint8_t txn_id;  /* Transaction number of the request message */

  uint8_t *read_data; /* Data received following a read request */
  uint8_t read_data_len; /* Size of *read_data */

  struct sensor_reg_req_s *next;
} sensor_reg_req_s;

/*============================================================================
  Static Variable Definitions
  ===========================================================================*/
static sensor1_handle_s *sensor1_handle = NULL; /* NULL if uninitialized, -1 if WOULDBLOCK */
static sensor_reg_req_s *sensor_reg_reqs = NULL;  /* Head of list for all ongoing sensor registry requests */
static uint8_t txn_id_last = 0;  /* Last transaction ID used */

static pthread_mutex_t sensor_reg_reqs_mutex; /* mutex for sensor_reg_reqs and txn_id_last */
static sensor_reg_req_s sensor1_data; /* Callback variables for sensor1 handle */

/*============================================================================
                    FUNCTION DECLARATIONS
============================================================================*/
void sensor_reg_rcv_msg( intptr_t data, sensor1_msg_header_s *msg_hdr_ptr,
                         sensor1_msg_type_e msg_type, void *msg_ptr );

/*============================================================================
  Static Function Definitions and Documentation
  ==========================================================================*/

/**
 * Destroys and frees an instance of a sensor_reg_req_s struct
 *
 * @param sensor_reg_req Object to clean-up and deallocate
 */
static void
sensor_reg_req_destroy( sensor_reg_req_s *sensor_reg_req )
{
  pthread_mutex_lock( &sensor_reg_reqs_mutex );
  pthread_mutex_destroy( &sensor_reg_req->cb_mutex );
  pthread_cond_destroy( &sensor_reg_req->cb_cond );
  free( sensor_reg_req );
  pthread_mutex_unlock( &sensor_reg_reqs_mutex );
}

/**
 * Creates a new registry request object.
 *
 * @return New object, or NULL if allocation failed
 */
static sensor_reg_req_s*
sensor_reg_req_add()
{
  sensor_reg_req_s *sensor_reg_req = malloc( sizeof(sensor_reg_req_s) );

  pthread_mutex_lock( &sensor_reg_reqs_mutex );
  if( NULL != sensor_reg_req )
  {
    sensor_reg_req->process_error = false;
    sensor_reg_req->txn_id = txn_id_last++;
    sensor_reg_req->read_data = NULL;
    sensor_reg_req->is_cb_arrived = false;
    pthread_mutex_init( &sensor_reg_req->cb_mutex, NULL );
    pthread_cond_init( &sensor_reg_req->cb_cond, NULL );

    sensor_reg_req->next = sensor_reg_reqs;
    sensor_reg_reqs = sensor_reg_req;
  }
  pthread_mutex_unlock( &sensor_reg_reqs_mutex );

  return sensor_reg_req;
}

/** Lookup a request object from the list
 *
 * @param[i] txn_id Transaction ID associated with the request
 * @param[i] remove Whether to remove the entry from the list if found
 * @return The object if found, NULL otherwise
 */
static sensor_reg_req_s*
sensor_reg_req_lookup( uint8_t txn_id, bool remove )
{
  sensor_reg_req_s *prev = NULL, *curr;

  pthread_mutex_lock( &sensor_reg_reqs_mutex );
  curr = sensor_reg_reqs;
  while( NULL != curr )
  {
    if( txn_id == curr->txn_id )
    {
      if( remove && NULL != prev )
      {
        prev->next = curr->next;
      }
      if( remove && sensor_reg_reqs == curr )
      {
        sensor_reg_reqs = curr->next;
      }
      break;
    }
    prev = curr;
    curr = curr->next;
  }
  pthread_mutex_unlock( &sensor_reg_reqs_mutex );
  return curr;
}

/**
 * Signal all pending registry requests with error status.
 */
static void
sensor_reg_sig_all()
{
  sensor_reg_req_s *curr = NULL;

  pthread_mutex_lock( &sensor_reg_reqs_mutex );
  curr = sensor_reg_reqs;
  while( NULL != curr )
  {
    pthread_mutex_lock( &curr->cb_mutex );
    curr->is_cb_arrived = true;
    curr->process_error = true;
    pthread_cond_signal( &curr->cb_cond );
    pthread_mutex_unlock( &curr->cb_mutex );

    curr = curr->next;
  }
  pthread_mutex_unlock( &sensor_reg_reqs_mutex );

  return curr;
}

/**
 * Causes the current thread to wait until a response has
 * been received, as signalled by sensor_reg_signal_response.
 *
 * cb_mutex must be locked prior to calling this function.
 *
 * @param timeout # of seconds to wait for response
 * @return False if there was an error waiting, true upon success.
 */
static bool
sensor_reg_wait_for_response( uint16_t timeout, sensor_reg_req_s *sensor_reg_req )
{
   bool result = true;
   int result_status;
   struct timespec ts;

   clock_gettime(CLOCK_REALTIME, &ts);
   ts.tv_sec += timeout;

   while( !sensor_reg_req->is_cb_arrived )
   {
      result_status = pthread_cond_timedwait( &sensor_reg_req->cb_cond,
                                              &sensor_reg_req->cb_mutex, &ts );
      if( sensor_reg_req->is_cb_arrived )
      {
        break;
      }
      if( ETIMEDOUT == result_status )
      {
          LOGE( "%s: Registry request timeout", __FUNCTION__ );
          result = false;
          sensor_reg_req->is_cb_arrived = true;
      }
   }

   return result;
}

/**
 * Processes the response message caused by either a read or
 * write request to the sensors registry.
 */
static void
sensor_reg_process_resp( sensor1_msg_header_s *msg_hdr, void *msg_ptr )
{
   bool error = false;
   uint32_t i = 0;
   sensor_reg_req_s *sensor_reg_req = sensor_reg_req_lookup( msg_hdr->txn_id, false );
   if( NULL == sensor_reg_req )
   {
      LOGE( "%s: Unable to find transaction id: %i",
            __FUNCTION__, msg_hdr->txn_id );
      return;
   }
   pthread_mutex_lock( &sensor_reg_req->cb_mutex );

   if( SNS_REG_SINGLE_READ_REQ_V02 == msg_hdr->msg_id )
   {
      sns_reg_single_read_resp_msg_v02 *msgPtr =
            (sns_reg_single_read_resp_msg_v02*) msg_ptr;

      if( 0 == msgPtr->resp.sns_result_t )
      {
         sensor_reg_req->read_data = malloc( sizeof(uint8_t) * msgPtr->data_len );
         if( NULL == sensor_reg_req->read_data )
         {
            LOGE( "%s: Unable to malloc space for read_data",
                  __FUNCTION__ );
            error = true;
         }
         else
         {
            for( i = 0; i < msgPtr->data_len; i++)
            {
               sensor_reg_req->read_data[i] = msgPtr->data[i];
            }
            sensor_reg_req->read_data_len = msgPtr->data_len;
         }
      }
      else
      {
         LOGE( "%s: Error in RESP; result: %i",
               __FUNCTION__, msgPtr->resp.sns_err_t );
         error = true;
      }
   }
   else if( SNS_REG_SINGLE_WRITE_REQ_V02 == msg_hdr->msg_id )
   {
      sns_reg_single_write_resp_msg_v02 *msgPtr =
         (sns_reg_single_write_resp_msg_v02*) msg_ptr;

      if( 0 != msgPtr->resp.sns_result_t )
      {
         error = true;
         LOGE( "%s: Error in RESP result: %i",
               __FUNCTION__, msgPtr->resp.sns_result_t );
      }
   }
   else
   {
      error = true;
      LOGE( "%s: Unknown message id received: %i",
            __FUNCTION__, msg_hdr->msg_id);
   }

   sensor_reg_req->is_cb_arrived = true;
   sensor_reg_req->process_error = error;
   pthread_cond_signal( &sensor_reg_req->cb_cond );
   pthread_mutex_unlock( &sensor_reg_req->cb_mutex );
}

/**
 * Call sensor1_open and set global sensor1_handle based on result.
 * Note: Must hold sensor1_mutex prior to calling.
 *
 * @return sensor1 error from sensor1_open
 */
static sensor1_error_e
sensor_reg_sensor1_open()
{
   sensor1_error_e error = sensor1_open( &sensor1_handle,
                        sensor_reg_rcv_msg,
                        (intptr_t)(&sensor1_handle) );
   if( SENSOR1_SUCCESS != error )
   {
     LOGE( "%s: sensor1_open returned %d", __FUNCTION__, error );
     sensor1_handle = (SENSOR1_EWOULDBLOCK == error) ? (sensor1_handle_s*)-1 : (sensor1_handle_s*)NULL;
   }

   return error;
}

/*============================================================================
  Externalized Function Definitions
  ==========================================================================*/

/* For receiving responses to sensor registry requests */
void
sensor_reg_rcv_msg( intptr_t data, sensor1_msg_header_s *msg_hdr_ptr,
                     sensor1_msg_type_e msg_type, void *msg_ptr )
{
   sensor1_error_e error;

   if( NULL == msg_hdr_ptr &&
       SENSOR1_MSG_TYPE_BROKEN_PIPE != msg_type &&
       SENSOR1_MSG_TYPE_RETRY_OPEN != msg_type )
   {
      LOGW( "%s: received NULL msg_hdr_ptr! type: %i",
            __FUNCTION__, msg_type );
      return ;
   }

   if( SENSOR1_MSG_TYPE_RESP == msg_type )
   {
      if( SNS_REG2_SVC_ID_V01 == msg_hdr_ptr->service_number )
      {
         sensor_reg_process_resp( msg_hdr_ptr, msg_ptr );
      }
      else
      {
         LOGE( "%s: Invalid service number type: %i",
               __FUNCTION__, msg_hdr_ptr->service_number );
      }
   }
   else if( SENSOR1_MSG_TYPE_BROKEN_PIPE == msg_type )
   {
      LOGW( "%s: received BROKEN_PIPE", __FUNCTION__ );
      pthread_mutex_lock( &sensor1_data.cb_mutex );
      sensor1_close( sensor1_handle );
      sensor_reg_sensor1_open();
      pthread_mutex_unlock( &sensor1_data.cb_mutex );

      sensor_reg_sig_all();
   }
   else if( SENSOR1_MSG_TYPE_RETRY_OPEN == msg_type )
   {
      LOGW( "%s: received RETRY_OPEN", __FUNCTION__ );
      pthread_mutex_lock( &sensor1_data.cb_mutex );

      error = sensor1_open( (sensor1_handle_s **)data,
                            sensor_reg_rcv_msg,
                            (intptr_t)((sensor1_handle_s **)data) );

      LOGW( "%s: sensor1_open returned %d", __FUNCTION__, error );
      if( SENSOR1_EWOULDBLOCK != error )
      {
         sensor1_data.is_cb_arrived = true;
         sensor1_data.process_error = ( SENSOR1_SUCCESS != error );
         pthread_cond_signal( &sensor1_data.cb_cond );
      }
      pthread_mutex_unlock( &sensor1_data.cb_mutex );
   }
   else
   {
      LOGE( "%s: Invalid msg type: %u", __FUNCTION__, msg_type );
   }

   if( NULL != msg_ptr ) {
      sensor1_free_msg_buf( *((sensor1_handle_s**)data), msg_ptr );
   }
}

int
sensor_reg_read( uint16_t id, uint8_t time_out, uint8_t **data, uint8_t *data_len )
{
   sensor1_error_e      error;
   sensor1_msg_header_s msgHdr;
   sns_reg_single_read_req_msg_v02* msgPtr = NULL;
   int rv = SENSOR_REG_SUCCESS;

   pthread_mutex_lock( &sensor1_data.cb_mutex );

   if( NULL == sensor1_handle )
   {
      LOGE( "%s: Sensor1 handle not opened", __FUNCTION__ );
      pthread_mutex_unlock( &sensor1_data.cb_mutex );
      return SENSOR_REG_OPEN;
   }
   else if( (intptr_t)-1 == (intptr_t)sensor1_handle &&
            ( false == sensor_reg_wait_for_response( SENSOR_REG_TIMEOUT, &sensor1_data )
              || sensor1_data.process_error ) )
   {
      LOGE( "%s: sensor1 not available", __FUNCTION__ );
      pthread_mutex_unlock( &sensor1_data.cb_mutex );
      return SENSOR_REG_SENSOR1;
   }

   error = sensor1_alloc_msg_buf( sensor1_handle,
              sizeof(sns_reg_single_read_req_msg_v02),
             (void**)&msgPtr );
   if( SENSOR1_SUCCESS != error )
   {
      LOGE( "%s: sensor1_alloc_msg_buf returned(get) %d",
            __FUNCTION__, error );
      pthread_mutex_unlock( &sensor1_data.cb_mutex );
      return SENSOR_REG_SENSOR1;
   }

   sensor_reg_req_s *sensor_reg_req = sensor_reg_req_add();
   if( NULL == sensor_reg_req )
   {
      LOGE( "%s: sensor_reg_req_add failed", __FUNCTION__ );
      sensor1_free_msg_buf( sensor1_handle, msgPtr );
      pthread_mutex_unlock( &sensor1_data.cb_mutex );
      return SENSOR_REG_INTERNAL;
   }

   msgHdr.service_number = SNS_REG2_SVC_ID_V01;
   msgHdr.msg_id = SNS_REG_SINGLE_READ_REQ_V02;
   msgHdr.msg_size = sizeof(sns_reg_single_read_req_msg_v02);
   msgHdr.txn_id = sensor_reg_req->txn_id;
   msgPtr->item_id = id;

   pthread_mutex_lock( &sensor_reg_req->cb_mutex );
   error = sensor1_write( sensor1_handle, &msgHdr, msgPtr );
   if( SENSOR1_SUCCESS != error )
   {
      LOGE( "%s: sensor1_write returned %d", __FUNCTION__, error );
      sensor1_free_msg_buf( sensor1_handle, msgPtr );
      pthread_mutex_unlock( &sensor1_data.cb_mutex );
      pthread_mutex_unlock( &sensor_reg_req->cb_mutex );
      return SENSOR_REG_SENSOR1;
   }

   pthread_mutex_unlock( &sensor1_data.cb_mutex );

   if( sensor_reg_wait_for_response( time_out, sensor_reg_req ) == false )
   {
      LOGE( "%s: sensor_reg_wait_for_response failed", __FUNCTION__);
      rv = SENSOR_REG_RESP;
   }
   else if( sensor_reg_req->process_error )
   {
      LOGE( "%s: error processing message", __FUNCTION__);
      rv = SENSOR_REG_PROCESS;
   }
   else
   {
      *data = sensor_reg_req->read_data;
      *data_len = sensor_reg_req->read_data_len;
   }
   pthread_mutex_unlock( &sensor_reg_req->cb_mutex );
   sensor_reg_req_lookup( sensor_reg_req->txn_id, true );
   sensor_reg_req_destroy( sensor_reg_req );

   return rv;
}

int
sensor_reg_write( uint16_t id, uint8_t const *data, int len, uint8_t time_out )
{
   sensor1_error_e      error;
   sensor1_msg_header_s msgHdr;
   sns_reg_single_write_req_msg_v02 *msgPtr = NULL;
   int                  i = 0;
   int                  rv = SENSOR_REG_SUCCESS;

   if( len < 1 )
   {
      LOGE( "%s: The value of len must be > 1: %i",
            __FUNCTION__, len );
      return SENSOR_REG_LEN;
   }

   pthread_mutex_lock( &sensor1_data.cb_mutex );

   if( NULL == sensor1_handle )
   {
      LOGE( "%s: Sensor1 handle not opened", __FUNCTION__ );
      pthread_mutex_unlock( &sensor1_data.cb_mutex );
      return SENSOR_REG_OPEN;
   }
   else if( (intptr_t)-1 == (intptr_t)sensor1_handle &&
            ( false == sensor_reg_wait_for_response( time_out, &sensor1_data )
              || sensor1_data.process_error ) )
   {
      LOGE( "%s: sensor1 not available", __FUNCTION__ );
      pthread_mutex_unlock( &sensor1_data.cb_mutex );
      return SENSOR_REG_SENSOR1;
   }

   error = sensor1_alloc_msg_buf( sensor1_handle,
                                  sizeof(sns_reg_single_write_req_msg_v02),
                                  (void**)&msgPtr );
   if( SENSOR1_SUCCESS != error )
   {
      LOGE( "%s: sensor1_alloc_msg_buf returned %d",
            __FUNCTION__, error );
      pthread_mutex_unlock( &sensor1_data.cb_mutex );
      return SENSOR_REG_SENSOR1;
   }

   sensor_reg_req_s *sensor_reg_req = sensor_reg_req_add();
   if( NULL == sensor_reg_req )
   {
      LOGE( "%s: sensor_reg_req_add failed", __FUNCTION__ );
      sensor1_free_msg_buf( sensor1_handle, msgPtr );
      pthread_mutex_unlock( &sensor1_data.cb_mutex );
      return SENSOR_REG_INTERNAL;
   }

   msgHdr.service_number = SNS_REG2_SVC_ID_V01;
   msgHdr.msg_id = SNS_REG_SINGLE_WRITE_REQ_V02;
   msgHdr.msg_size = sizeof(sns_reg_single_write_req_msg_v02);
   msgHdr.txn_id = sensor_reg_req->txn_id;
   msgPtr->item_id = id;
   msgPtr->data_len = len;

   for(i = 0; i < len; i++)
   {
      msgPtr->data[i] = data[i];
   }

   pthread_mutex_lock( &sensor_reg_req->cb_mutex );
   error = sensor1_write( sensor1_handle, &msgHdr, msgPtr );

   if( SENSOR1_SUCCESS != error )
   {
      LOGE( "%s: sensor1_write returned %d", __FUNCTION__, error );
      sensor1_free_msg_buf( sensor1_handle, msgPtr );
      pthread_mutex_unlock( &sensor1_data.cb_mutex );
      pthread_mutex_unlock( &sensor_reg_req->cb_mutex );
      return SENSOR_REG_SENSOR1;
   }
   pthread_mutex_unlock( &sensor1_data.cb_mutex );

   if( sensor_reg_wait_for_response( time_out, sensor_reg_req ) == false )
   {
      LOGE( "%s: sensor_reg_wait_for_response failed", __FUNCTION__);
      rv = SENSOR_REG_RESP;
   }
   else if( sensor_reg_req->process_error )
   {
      LOGE( "%s: error in message", __FUNCTION__);
      rv = SENSOR_REG_PROCESS;
   }
   pthread_mutex_unlock( &sensor_reg_req->cb_mutex );
   sensor_reg_req_lookup( sensor_reg_req->txn_id, true );
   sensor_reg_req_destroy( sensor_reg_req );

   return rv;
}

int sensor_reg_open()
{
  sensor1_error_e error;

  if( NULL == sensor1_handle )
  {
    pthread_mutex_init( &sensor1_data.cb_mutex, NULL );
    pthread_cond_init( &sensor1_data.cb_cond, NULL );
    pthread_mutex_init( &sensor_reg_reqs_mutex, NULL );

    error = sensor_reg_sensor1_open();
    if( SENSOR1_SUCCESS != error && SENSOR1_EWOULDBLOCK != error )
    {
      pthread_mutex_destroy( &sensor1_data.cb_mutex );
      pthread_cond_destroy( &sensor1_data.cb_cond );
      pthread_mutex_destroy( &sensor_reg_reqs_mutex );
      return SENSOR_REG_INTERNAL;
    }
  }

  return SENSOR_REG_SUCCESS;
}

int sensor_reg_close() { return SENSOR_REG_SUCCESS; }

JNIEXPORT jint JNICALL
Java_com_qualcomm_sensors_sensortest_SensorsReg_setRegistryValue
   ( JNIEnv *env, jclass class, jint reg_id, jbyteArray data, jbyte len )
{
   uint8_t *cData;
   int return_value = 0;

   cData  = (uint8_t *) (*env)->GetByteArrayElements( env, data, NULL );
   return_value = sensor_reg_write( reg_id, cData, len, SENSOR_REG_TIMEOUT );
   (*env)->ReleaseByteArrayElements( env, data, (jbyte *) cData, 0 );

   return return_value;
}

JNIEXPORT jbyteArray JNICALL
Java_com_qualcomm_sensors_sensortest_SensorsReg_getRegistryValue
   ( JNIEnv *env, jclass class, jint reg_id )
{
   jbyteArray byteArray = NULL;
   int return_value = 0;
   uint8_t *result_data = NULL;
   uint8_t data_len = 0;

   if ((*env)->EnsureLocalCapacity( env, 2 ) < 0) {
      return NULL; /* out of memory error */
   }

   return_value = sensor_reg_read( reg_id, SENSOR_REG_TIMEOUT, &result_data, &data_len );

   /* Handle errors */
   if( SENSOR_REG_SUCCESS != return_value )
   {
      char error_string[16];

      snprintf( error_string, 16, "Error code: %i", return_value );
      jclass Exception = (*env)->FindClass( env, "java/lang/Exception" );
      (*env)->ThrowNew( env, Exception, error_string );
      return NULL;
   }

   byteArray = (*env)->NewByteArray( env, data_len );
   if( byteArray != NULL )
   {
      (*env)->SetByteArrayRegion( env, byteArray, 0, data_len, (jbyte*) result_data );
   }

   free(result_data);
   return byteArray;
}

JNIEXPORT jint JNICALL
Java_com_qualcomm_sensors_sensortest_SensorsReg_open( JNIEnv *env, jclass class )
{
   return sensor_reg_open();
}

JNIEXPORT jint JNICALL
Java_com_qualcomm_sensors_sensortest_SensorsReg_close( JNIEnv *env, jclass class )
{
   return sensor_reg_close();
}

/*
 * Array of methods.
 *
 * Each entry has three fields: the name of the method, the method
 * signature, and a pointer to the native implementation.
 */
static const JNINativeMethod gMethods[] = {
   { "setRegistryValue",
     "(I[BB)I",
     (void*)Java_com_qualcomm_sensors_sensortest_SensorsReg_setRegistryValue
   },
   { "getRegistryValue",
     "(I)[B",
     (void*)Java_com_qualcomm_sensors_sensortest_SensorsReg_getRegistryValue
   },
   { "open",
     "()I",
     (void*)Java_com_qualcomm_sensors_sensortest_SensorsReg_open
   },
   { "close",
     "()I",
     (void*)Java_com_qualcomm_sensors_sensortest_SensorsReg_close
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
                        "com/qualcomm/sensors/sensortest/SensorsReg";
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

   return JNI_VERSION_1_4;
}
