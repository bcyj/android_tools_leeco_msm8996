/*============================================================================
@file sensor_thresh.c

@brief
Native implementation of sensor threshold API.  Provides access to the SAM
threshold algorithm.

@attention
Threshold JNI does not support SSR and Sensors Daemon recovery.  Active
Threshold algorithm instances in such situations must be manually restarted.

Copyright (c) 2012-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

/*============================================================================
  Macros
  ==========================================================================*/
#define FX_QFACTOR           (16)
#define FX_FLTTOFIX(f,q)     ((int32_t)( (f)*(1<<(q))+(((f)>(0.0))?(0.5):(-0.5))))
#define FX_FLTTOFIX_Q16(d)   (FX_FLTTOFIX(d,FX_QFACTOR))

#define FX_FIXTOFLT(i,q)     (((double)(i))/((double)(1<<(q))))
#define FX_FIXTOFLT_Q16(a)   (FX_FIXTOFLT(a,FX_QFACTOR))

#define UNIT_CONVERT_Q16     (1.0/65536.0)

#define LOG_TAG "sensor_thresh"

/*============================================================================
  INCLUDE FILES
  ==========================================================================*/

#include "sensor_thresh.h"
#include "sns_sam_sensor_thresh_v01.h"
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
  Type Declarations
  ==========================================================================*/

/* Data associated with each threshold request */
typedef struct _thresh_control
{
  sensor1_handle_s *sensor1_handle;
  bool is_cb_arrived;
  pthread_mutex_t cb_mutex;
  pthread_cond_t cb_cond;
  jobject cb_obj;   /* onSensorChanged callback object */
  int instance_id;  /* Instance ID returned from SAM and returned to Android */

  /* Linked list fields */
  struct _thresh_control *next;
  struct _thresh_control *prev;
} thresh_control_s;

/*============================================================================
  Static Variable Definitions
  ==========================================================================*/

/** Mutex protecting the control queue */
static pthread_mutex_t thresh_control_queue_mutex;

/* First and last entries into the list */
static thresh_control_s *thresh_control_queue_first;
static thresh_control_s *thresh_control_queue_last;

static JavaVM *java_vm_g;

/*
 * Array of methods.
 *
 * Each entry has three fields: the name of the method, the method
 * signature, and a pointer to the native implementation.
 */
static const JNINativeMethod gMethods[] = {
   { "registerThresholdNative",
     "(IIIFFFLcom/qualcomm/sensors/qsensortest/QSensorEventListener;)I",
     (void*)Java_com_qualcomm_sensors_sensortest_SensorThresh_registerThresholdNative
   },
   { "unregisterThresholdNative",
     "(I)I",
     (void*)Java_com_qualcomm_sensors_sensortest_SensorThresh_unregisterThresholdNative
   }
};

/*============================================================================
                    FORWARD FUNCTION DECLARATIONS
  ==========================================================================*/

void thresh_sensor1_cb( intptr_t data, sensor1_msg_header_s *msg_hdr,
                sensor1_msg_type_e msg_type, void *msg_ptr );

/*============================================================================
  Static Function Definitions and Documentation
  ===========================================================================*/

/**
 *  Send a sensor1 request, and wait for the response.
 *
 *  @param[i] thresh_control
 *  @param[i] msg_hdr
 *  @param[i] msg_ptr
 *
 *  @return -1 Upon error, all other values indicate success.
 */
static int
send_req( thresh_control_s *thresh_control, sensor1_msg_header_s *msg_hdr, void *msg_ptr )
{
  sensor1_error_e sensor1_error;
  struct timespec ts;       // timestamp to use with timed_wait
  int instance_id = -1;     // Instance ID contained in response
  int result_status;        // Result of timed_wait

  sensor1_error = sensor1_write( thresh_control->sensor1_handle,
                                 msg_hdr, msg_ptr );
  if( SENSOR1_SUCCESS != sensor1_error )
  {
    LOGE( "%s: sensor1_write returned %d", __FUNCTION__, sensor1_error );
    sensor1_free_msg_buf( thresh_control->sensor1_handle, msg_ptr );
  }
  else
  {
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += 10;

    /* wait for response */
    while( !thresh_control->is_cb_arrived )
    {
      result_status = pthread_cond_timedwait( &thresh_control->cb_cond, &thresh_control->cb_mutex, &ts );
      if( ETIMEDOUT == result_status )
      {
        LOGE( "%s: Request timed-out", __FUNCTION__ );
        thresh_control->instance_id = -1;
        break;
      }
    }

    thresh_control->is_cb_arrived = false;
    instance_id = thresh_control->instance_id;
  }
  return instance_id;
}

/**
 * Initialize the fields associated with thresh_control_s
 * object.
 *
 * @param[i] env
 * @param[i] cb_obj
 *
 * @return Created object, or NULL upon failure
 */
static thresh_control_s*
init_thresh_control( JNIEnv *env, jobject cb_obj )
{
  thresh_control_s* thresh_control = malloc( sizeof(thresh_control_s) );

  if( NULL != thresh_control )
  {
    thresh_control->instance_id = -1;
    thresh_control->is_cb_arrived = false;
    thresh_control->sensor1_handle = NULL;
    pthread_mutex_init( &thresh_control->cb_mutex, NULL );
    pthread_cond_init( &thresh_control->cb_cond, NULL );
    thresh_control->cb_obj = (*env)->NewGlobalRef( env, cb_obj );
  }

  return thresh_control;
}

/**
 * Look-up an entry in the control queue based on instance ID.
 *
 * @param[i] instance_id
 *
 * @return thresh_control object or NULL if not found.
 */
static thresh_control_s*
lookup_control( int instance_id )
{
  thresh_control_s *curr = NULL;

  pthread_mutex_lock( &thresh_control_queue_mutex );
  if( NULL == thresh_control_queue_first )
  {
    LOGE( "%s: queue empty %i", __FUNCTION__, instance_id );
  }
  else
  {
    do
    {
      curr = ( NULL == curr ) ? thresh_control_queue_first : curr->next;

      if( instance_id == curr->instance_id )
      {
        pthread_mutex_unlock( &thresh_control_queue_mutex );
        return curr;
      }
    } while( curr != thresh_control_queue_last );
  }

  pthread_mutex_unlock( &thresh_control_queue_mutex );
  return NULL;
}

/**
 * Remove control from queue and clean-up data and memory
 *
 * @param[i] env
 * @param[io] thresh_control
 */
static void
cleanup_control( JNIEnv *env, thresh_control_s *thresh_control )
{
  pthread_mutex_lock( &thresh_control_queue_mutex );

  if( thresh_control == thresh_control_queue_first )
  {
    thresh_control_queue_first = thresh_control->next;
  }
  else
  {
    thresh_control->prev->next = thresh_control->next;
  }

  if( thresh_control == thresh_control_queue_last )
  {
    thresh_control_queue_last = thresh_control->prev;
  }
  pthread_mutex_unlock( &thresh_control_queue_mutex );

  pthread_mutex_destroy( &thresh_control->cb_mutex );
  pthread_cond_destroy( &thresh_control->cb_cond );

  (*env)->DeleteGlobalRef( env, thresh_control->cb_obj );
  free( thresh_control );
}

/**
 * Add a control object to the queue.
 *
 * @param[io] thresh_control
 */
static void
add_control( thresh_control_s *thresh_control )
{
  pthread_mutex_lock( &thresh_control_queue_mutex );

  thresh_control->next = thresh_control_queue_first;
  thresh_control->prev = NULL;
  if( NULL != thresh_control_queue_first )
  {
    thresh_control_queue_first->prev = thresh_control;
  }
  thresh_control_queue_first = thresh_control;

  if( NULL == thresh_control_queue_last )
  {
    thresh_control_queue_last = thresh_control_queue_first;
  }

  pthread_mutex_unlock( &thresh_control_queue_mutex );
}

JNIEXPORT jint JNICALL
Java_com_qualcomm_sensors_sensortest_SensorThresh_registerThresholdNative
  ( JNIEnv *env, jclass class, jint sensor_id, jint data_type, jint sample_rate,
    jfloat thresh_x, jfloat thresh_y, jfloat thresh_z, jobject cb_obj )
{
  sensor1_msg_header_s msg_hdr;
  sns_sam_sensor_thresh_enable_req_msg_v01 *msg_ptr;
  sensor1_error_e sensor1_error;
  int rv = -1;
  thresh_control_s *thresh_control = init_thresh_control( env, cb_obj );

  if( NULL != thresh_control )
  {
    sensor1_error = sensor1_open( &thresh_control->sensor1_handle, thresh_sensor1_cb, (intptr_t)thresh_control );
    if( SENSOR1_SUCCESS != sensor1_error ) {
      LOGE( "%s: sensor1_open returned %d", __FUNCTION__, sensor1_error );
      cleanup_control( env, thresh_control );
      rv = -1;
    }
    else
    {
      msg_hdr.service_number = SNS_SAM_SENSOR_THRESH_SVC_ID_V01;
      msg_hdr.msg_id = SNS_SAM_SENSOR_THRESH_ENABLE_REQ_V01;
      msg_hdr.msg_size = sizeof(sns_sam_sensor_thresh_enable_req_msg_v01);
      msg_hdr.txn_id = 0;

      sensor1_error = sensor1_alloc_msg_buf( thresh_control->sensor1_handle, msg_hdr.msg_size, (void**)&msg_ptr );
      if( SENSOR1_SUCCESS != sensor1_error )
      {
        LOGE( "%s: sensor1_alloc_msg_buf returned %d", __FUNCTION__, sensor1_error );
        rv = -1;
      }
      else
      {
        LOGE( "%s: Threshold sample rate: %i (Q16: %i)", __FUNCTION__, sample_rate, FX_FLTTOFIX_Q16( sample_rate * 1.0 ) );
        msg_ptr->sensor_id = sensor_id;
        msg_ptr->data_type = data_type;
        msg_ptr->sample_rate = FX_FLTTOFIX_Q16( sample_rate * 1.0 );
        msg_ptr->threshold[0] = FX_FLTTOFIX_Q16( thresh_x );
        msg_ptr->threshold[1] = FX_FLTTOFIX_Q16( thresh_y );
        msg_ptr->threshold[2] = FX_FLTTOFIX_Q16( thresh_z );

        pthread_mutex_lock( &thresh_control->cb_mutex );
        rv = send_req( thresh_control, &msg_hdr, msg_ptr );
        pthread_mutex_unlock( &thresh_control->cb_mutex );

        if( -1 != rv )
        {
          add_control( thresh_control );
        }
      }

      if( -1 == rv )
      {
        sensor1_close( thresh_control->sensor1_handle );
        cleanup_control( env, thresh_control );
      }
    }
  }
  else
  {
    LOGE( "%s: malloc error", __FUNCTION__ );
  }

  return rv;
}

JNIEXPORT jint
JNICALL Java_com_qualcomm_sensors_sensortest_SensorThresh_unregisterThresholdNative
  ( JNIEnv *env, jclass class, jint instance_id )
{
  sensor1_msg_header_s msg_hdr;
  sns_sam_sensor_thresh_disable_req_msg_v01 *msg_ptr;
  sensor1_error_e sensor1_error;
  int rv = -1;
  LOGD( "%s: Sending disable request %i", __FUNCTION__, instance_id );
  thresh_control_s *thresh_control = lookup_control( instance_id );

  if( NULL == thresh_control )
  {
    LOGE( "%s: Unable to find instance %i", __FUNCTION__, instance_id );
    rv = -1;
  }
  else
  {
    msg_hdr.service_number = SNS_SAM_SENSOR_THRESH_SVC_ID_V01;
    msg_hdr.msg_id = SNS_SAM_SENSOR_THRESH_DISABLE_REQ_V01;
    msg_hdr.msg_size = sizeof(sns_sam_sensor_thresh_disable_req_msg_v01);
    msg_hdr.txn_id = 0;

    sensor1_error = sensor1_alloc_msg_buf( thresh_control->sensor1_handle, msg_hdr.msg_size, (void**)&msg_ptr );
    if( SENSOR1_SUCCESS != sensor1_error )
    {
      LOGE( "%s: sensor1_alloc_msg_buf returned %d", __FUNCTION__, sensor1_error );
      rv = -1;
    }
    else
    {
      msg_ptr->instance_id = instance_id;

      pthread_mutex_lock( &thresh_control->cb_mutex );
      rv = send_req( thresh_control, &msg_hdr, msg_ptr );
      pthread_mutex_unlock( &thresh_control->cb_mutex );
    }

    sensor1_close( thresh_control->sensor1_handle );
    cleanup_control( env, thresh_control );
  }

  return rv;
}

/**
 * Indicates to a waiting thread that a response message has been
 * received.b  g_thresh_control->cb_mutex should be held prior
 * to calling this function.
 *
 * @param[i] result >=0: Response code contained in the response
 *                <0: user_cal_error_e
 */
static void
signal_response( thresh_control_s *thresh_control, int instance_id )
{
  pthread_mutex_lock( &thresh_control->cb_mutex );
  thresh_control->instance_id = instance_id;
  thresh_control->is_cb_arrived = true;
  pthread_cond_signal( &thresh_control->cb_cond );
  pthread_mutex_unlock( &thresh_control->cb_mutex );
}

/**
 * Create an instance of the sensor event class, and populate it
 * from the received message.
 *
 * @param[i] env
 * @param[i] msg_ptr
 * @param[o] obj_ptr
 *
 * @return -1 upon error, 0 on success.
 */
static int
create_sensor_event( JNIEnv *env, sns_sam_sensor_thresh_report_ind_msg_v01 *msg_ptr, jobject *obj_ptr )
{
  const char *class_path = "android/hardware/SensorEvent";
  jclass class;
  jmethodID constructor;
  jobject object;
  jfloatArray values_array;
  jfloat *values;
  jfieldID values_field, accuracy_field, timestamp_field;

  class = (*env)->FindClass( env, class_path );
  if( !class )
  {
    LOGE( "%s: Unable to find class: %s", __FUNCTION__, class_path );
    return -1;
  }

  constructor = (*env)->GetMethodID( env, class, "<init>", "(I)V" );
  if( !constructor )
  {
    LOGE( "%s: Unable to find constructor: %s", __FUNCTION__, class_path );
    return -1;
  }

  object = (*env)->NewObject( env, class, constructor, 3 );
  if( !object )
  {
    LOGE( "%s: Unable to create object: %s", __FUNCTION__, class_path );
    return -1;
  }

  accuracy_field = (*env)->GetFieldID( env, class, "accuracy", "I" );
  if( !accuracy_field )
  {
    LOGE( "%s: Unable to find accuracy field", __FUNCTION__ );
    return -1;
  }
  (*env)->SetIntField( env, object, accuracy_field, 3 );

  timestamp_field = (*env)->GetFieldID( env, class, "timestamp", "J" );
  if( !timestamp_field )
  {
    LOGE( "%s: Unable to find timestamp field", __FUNCTION__ );
    return -1;
  }
  (*env)->SetLongField( env, object, timestamp_field, msg_ptr->timestamp );

  values_array = (*env)->NewFloatArray( env, 3 );
  if( !values_array )
  {
    LOGE( "%s: Unable to create integer array", __FUNCTION__ );
    return -1;
  }

  values = (*env)->GetFloatArrayElements( env, values_array, NULL );
  values[0] = msg_ptr->sample_value[0] * UNIT_CONVERT_Q16;
  values[1] = msg_ptr->sample_value[1] * UNIT_CONVERT_Q16;
  values[2] = msg_ptr->sample_value[2] * UNIT_CONVERT_Q16;
  (*env)->ReleaseFloatArrayElements( env, values_array, values, 0 );

  values_field = (*env)->GetFieldID( env, class, "values", "[F" );
  if( !values_field )
  {
    LOGE( "%s: Unable to find values field", __FUNCTION__ );
    return -1;
  }
  (*env)->SetObjectField( env, object, values_field, values_array );

  *obj_ptr = (*env)->NewGlobalRef( env, object );
  return 0;
}

/**
 * Process an indication received from the sensor1 callback.
 *
 * @param[i] thresh_control
 * @param[i] msg_ptr
 */
static void
handle_ind( thresh_control_s *thresh_control, sns_sam_sensor_thresh_report_ind_msg_v01 *msg_ptr )
{
  JNIEnv *env;
  bool attached = false;
  int err;
  jobject object;
  jclass class;
  jmethodID methodID;

  err = (*java_vm_g)->GetEnv( java_vm_g, (void**)&env, JNI_VERSION_1_4 );
  if( 0 > err )
  {
    err = (*java_vm_g)->AttachCurrentThread( java_vm_g, &env, NULL );
    if( 0 > err )
    {
      LOGE( "%s: Error attaching thread: %i", __FUNCTION__, err );
      return ;
    }
    attached = true;
  }

  class = (*env)->GetObjectClass( env, thresh_control->cb_obj );
  if( !class )
  {
    LOGE( "%s: Unable to get class reference", __FUNCTION__ );
  }
  else
  {
    methodID = (*env)->GetMethodID( env, class, "onSensorChanged", "(Landroid/hardware/SensorEvent;)V" );
    if( !methodID )
    {
      LOGE( "%s: Unable to get method ID", __FUNCTION__ );
    }
    else
    {
      err = create_sensor_event( env, msg_ptr, &object );

      if( -1 != err )
      {
        (*env)->CallVoidMethod( env, thresh_control->cb_obj, methodID, object );
        (*env)->DeleteGlobalRef( env, object );
      }
    }
  }

  if( attached )
  {
    (*java_vm_g)->DetachCurrentThread( java_vm_g );
  }
}

/**
 * Call-back functions from sensor1, for when a message has been received.
 * See sensor1.h for parameter details.
 */
void
thresh_sensor1_cb( intptr_t data, sensor1_msg_header_s *msg_hdr,
                   sensor1_msg_type_e msg_type, void *msg_ptr )
{
  thresh_control_s *thresh_control = (thresh_control_s*)data;

  if( NULL == msg_hdr ) {
    LOGD( "%s: received NULL msg_hdr_ptr. type: %i", __FUNCTION__, msg_type );
    return ;
  }

  if( SENSOR1_MSG_TYPE_RESP == msg_type &&
      SNS_SAM_SENSOR_THRESH_ENABLE_RESP_V01 == msg_hdr->msg_id )
  {
    LOGD( "%s: Received enable response", __FUNCTION__ );
    sns_sam_sensor_thresh_enable_resp_msg_v01 *msgPtr =
        (sns_sam_sensor_thresh_enable_resp_msg_v01*) msg_ptr;

    signal_response( thresh_control,
                     msgPtr->resp.sns_result_t == 0 && msgPtr->instance_id_valid
                      ? msgPtr->instance_id : -1 );
  }
  else if( SENSOR1_MSG_TYPE_RESP == msg_type &&
           SNS_SAM_SENSOR_THRESH_DISABLE_RESP_V01 == msg_hdr->msg_id )
  {
    LOGD( "%s: Received disable response", __FUNCTION__ );
    sns_sam_sensor_thresh_disable_resp_msg_v01 *msgPtr =
        (sns_sam_sensor_thresh_disable_resp_msg_v01*) msg_ptr;

    signal_response( thresh_control, msgPtr->resp.sns_result_t == 0 ? 0 : -1 );
  }
  else if( SENSOR1_MSG_TYPE_IND == msg_type )
  {
    handle_ind( thresh_control, msg_ptr );
  }
  else
  {
    LOGW( "%s: Unsupported message.  type: %i; svc_id: %i; msg_id: %i",
          __FUNCTION__, msg_type, msg_hdr->service_number, msg_hdr->msg_id );
  }

  if( NULL != msg_ptr )
  {
    sensor1_free_msg_buf( thresh_control->sensor1_handle, msg_ptr );
  }
}

/*
 * Explicitly register all methods for our class.
 *
 * @param[i] env
 *
 * @return 0 on success.
 */
static int
registerMethods(JNIEnv* env)
{
  static const char* const kClassName =
    "com/qualcomm/sensors/sensortest/SensorThresh";
  jclass class;

  /* look up the class */
  class = (*env)->FindClass( env, kClassName );
  if( NULL == class)
  {
    LOGI( "%s: Can't find class %s", __FUNCTION__, kClassName );
    return -1;
  }

  /* register all the methods */
  if( (*env)->RegisterNatives( env, class, gMethods,
      sizeof(gMethods) / sizeof(gMethods[0]) ) != JNI_OK )
  {
    LOGE( "%s: Failed registering methods for %s", __FUNCTION__, kClassName );
    return -1;
  }

  return 0;
}

/*============================================================================
  Externalized Function Definitions
  ==========================================================================*/

/*
 * This is called by the VM when the shared library is first loaded.
 */
jint
JNI_OnLoad(JavaVM *vm, void *reserved)
{
  JNIEnv* env = NULL;
  if( (*vm)->GetEnv( vm, (void**)&env, JNI_VERSION_1_4 ) != JNI_OK )
  {
    LOGE( "%s: GetEnv failed", __FUNCTION__ );
    return -1;
  }

  if( registerMethods( env ) != 0 )
  {
    LOGE( "%s: native registration failed", __FUNCTION__ );
    return -1;
  }

  /* Initalize the global queue and mutex */
  pthread_mutex_init( &thresh_control_queue_mutex, NULL );
  thresh_control_queue_first = NULL;
  thresh_control_queue_last = NULL;
  java_vm_g = vm;

  return JNI_VERSION_1_4;
}
