/*============================================================================
  @file sns_cm_conc_test.c

  @brief
    To test how the client manager handles concurrent threads and sensor1
    connections.
  @author gju
  @date 02/22/2012

  Copyright (c) 2012-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ==========================================================================*/

/*============================================================================
  INCLUDE FILES
  ==========================================================================*/

#include "sensor1.h"
#include "sns_common_v01.h"
#include "sns_smgr_api_v01.h"
#include <fcntl.h>
#include <inttypes.h>
#include <pthread.h>
#include <sched.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <pthread.h>
#include <errno.h> // For ETIMEDOUT

/*============================================================================
  Type Declarations
  ==========================================================================*/

#define SENSOR_ID_COUNT 5
#define DATA_TYPE_COUNT 2

typedef struct _sensor_report_s sensor_report_s;
struct _sensor_report_s
{
  uint16_t sensor_rate;
  uint16_t report_id;
  sensor_report_s *next;
};

typedef struct
{
  sensor1_handle_s *sensor1_handle;
  bool is_cb_arrived; /* If a RESP has been received yet */
  pthread_mutex_t cb_mutex; /* Wait for RESP */
  pthread_cond_t cb_cond; /* Wait for RESP */
  uint16_t resp_result; /* ID of sensor1 cb function */
  sensor_report_s *sensor_reports;
  pthread_mutex_t reports_mutex; /* For sensor_reports access */
  uint32_t report_id_cnt; /* Id of next unused report ID. */
  uint16_t control_id;  /* Unique identifier of this struct */
} test_control_s;

/*============================================================================
  Static Variable Definitions
  ==========================================================================*/

static uint16_t sensor_ids[ SENSOR_ID_COUNT ] =
{
  SNS_SMGR_ID_ACCEL_V01, SNS_SMGR_ID_GYRO_V01, SNS_SMGR_ID_MAG_V01,
  SNS_SMGR_ID_PRESSURE_V01, SNS_SMGR_ID_PROX_LIGHT_V01
};
static uint16_t data_types[ DATA_TYPE_COUNT ] =
{
  SNS_SMGR_DATA_TYPE_PRIMARY_V01,
  SNS_SMGR_DATA_TYPE_SECONDARY_V01
};

/* Default Values - May change through command-line */
#define SENSORS_PER_THREAD 4
#define THREAD_COUNT 3
#define ACTIONS_PER_THREAD 50

#define MAX_CNT 10000000

/* How many sensor1 connections should be made per thread */
static uint16_t sensors_per_thread_g = SENSORS_PER_THREAD;
/* How many threads to spawn */
static uint16_t thread_count_g = THREAD_COUNT;
/* How many actions (report request, delete, or cancel) to make in each thread */
static uint16_t actions_per_thread_g = ACTIONS_PER_THREAD;

/* One thread should never access the control of another thread,
 * so no mutex should be needed */
static test_control_s *test_controls_g;

/*============================================================================
  Static Function Definitions and Documentation
  ===========================================================================*/

/**
 * Causes the current thread to wait until a response has
 * been received, as signalled by sensor_reg_signal_response.
 *
 * cb_mutex must be locked prior to calling this function.
 *
 * @return False if there was an error waiting, true upon success.
 */
static bool
wait_for_response( test_control_s *test_control ){
   bool result = true;
   struct timespec ts;

   printf( "%s: %i \n", __FUNCTION__, test_control->control_id );

   clock_gettime(CLOCK_REALTIME, &ts);
   ts.tv_sec += 10;

   /* In case the response has already been received */
   while( test_control->is_cb_arrived == false )
   {
      int result_status = pthread_cond_timedwait( &test_control->cb_cond,
                                                  &test_control->cb_mutex, &ts );
      if( result_status == ETIMEDOUT )
      {
         printf( "******%s: Sensor Test request timed-out\n", __FUNCTION__ );
         result = false;
         test_control->is_cb_arrived = true;
      }
   }
   test_control->is_cb_arrived = false;

   return result;
}

/**
 * Given an ID, find the corresponding struct in test_controls_g.
 *
 * ID X should correspond to index X, but in case it doesn't, or
 * if the implementation changes.
 *
 * @param control_id ID as set in test_control_s.control_id
 * @return Index into test_controls_g.
 */
static int
find_control( uint16_t control_id )
{
  int rv = -1;

  int i;
  for( i = 0; i < (sensors_per_thread_g * thread_count_g); i++ )
  {
    if( test_controls_g[i].control_id == control_id )
    {
      rv = i;
      break;
    }
  }

  return rv;
}

/**
 * Indicate to the waiting thread that a response has been received
 *
 * @param result Which cb function received the response
 * @param cb_data The callback data from sensor1_open, AKA the test control ID.
 * @param msg_ptr
 */
static void
signal_response( uint16_t result, uint16_t cb_data, void *msg_ptr )
{
  int control_index = find_control( cb_data );

  if( -1 != control_index )
  {
    test_controls_g[ control_index ].resp_result = result;
    test_controls_g[ control_index ].is_cb_arrived = true;
    pthread_cond_signal( &test_controls_g[ control_index ].cb_cond );

    // TODO: Save registered callback function and check here
  }
  else
  {
    printf( "******%s: Unable to find associated control: %i\n", __FUNCTION__, cb_data );
  }
}

/**
 * Processes an incoming indication message from SMGR.  Will only print a message
 * if an error is found.
 *
 * @param cb_data The callback data from sensor1_open, AKA the test control ID.
 * @param msg_ptr
 */
static void
process_ind( uint16_t cb_data, sns_smgr_periodic_report_ind_msg_v01 *msg_ptr )
{
  int control_id = find_control( cb_data );
  uint8_t data_type = msg_ptr->Item[0].DataType, sensor_id = msg_ptr->Item[0].SensorId;
  uint16_t current_rate = msg_ptr->CurrentRate, report_id = msg_ptr->ReportId;
  sensor_report_s *curr;

  if( -1 != control_id )
  {
    pthread_mutex_lock( &test_controls_g[ control_id ].reports_mutex );

    curr = test_controls_g[ control_id ].sensor_reports;
    while( NULL != curr )
    {
      if( curr->report_id == report_id )
        break;

      curr = curr->next;
    }

    if( NULL == curr )
    {
      printf( "******%s: Unexpected indication received.  Control ID: %i, report ID: %i, sensor ID: %i, data type: %i, rate: %i\n",
              __FUNCTION__, control_id, report_id, sensor_id, data_type, current_rate );
    }
    pthread_mutex_unlock( &test_controls_g[ control_id ].reports_mutex );
  }
  else
  {
    printf( "******%s: Unable to find associated control: %i\n", __FUNCTION__, cb_data );
  }
}

/**
 *  Callback handler for all sensor1 connections.  See sensor_rcv_msg1/2 for the
 *  callback function.
 */
static void
sensor_rcv_msg( intptr_t cb_data, sensor1_msg_header_s *msg_hdr,
                     sensor1_msg_type_e msg_type, void *msg_ptr, uint8_t cb_func_num )
{
  if( NULL == msg_hdr ) {
    printf( "%s: received NULL msg_hdr_ptr! type: %i\n", __FUNCTION__, msg_type );
    return ;
  }

  printf( "%s: received message; type: %i; svc num: %i; id: %i; cb_data/control id: %i\n",
      __FUNCTION__, msg_type, msg_hdr->service_number, msg_hdr->msg_id, *(uint16_t*)cb_data );

  if( SENSOR1_MSG_TYPE_RESP == msg_type )
  {
    // TODO: If response is an error, remove sensor report (so that any future indications are flagged as an error)
    signal_response( cb_func_num, *(uint16_t*)cb_data, msg_hdr );
  }
  else if( SENSOR1_MSG_TYPE_IND == msg_type &&
           SNS_SMGR_SVC_ID_V01 == msg_hdr->service_number )
  {
    process_ind( *(uint16_t*)cb_data, (sns_smgr_periodic_report_ind_msg_v01*)msg_ptr );
  }
  else
  {
    printf( "******%s: Unexpected message type: %i, %i (control id %i)\n",
            __FUNCTION__, msg_type, msg_hdr->service_number, *(uint16_t*)cb_data );
  }

  if( NULL != msg_ptr )
  {
    int control_id = find_control( cb_data );
    if( -1 != control_id )
    {
      sensor1_free_msg_buf(  test_controls_g[ control_id ].sensor1_handle, msg_ptr );
    }
  }
}

void
sensor_rcv_msg1( intptr_t cb_data, sensor1_msg_header_s *msg_hdr,
                     sensor1_msg_type_e msg_type, void *msg_ptr )
{
   sensor_rcv_msg( cb_data, msg_hdr, msg_type, msg_ptr, 1 );
}

void
sensor_rcv_msg2( intptr_t cb_data, sensor1_msg_header_s *msg_hdr,
                     sensor1_msg_type_e msg_type, void *msg_ptr )
{
   sensor_rcv_msg( cb_data, msg_hdr, msg_type, msg_ptr, 2 );
}

/**
 * Initializes a test_control object.
 *
 * @param test_control
 * @param use_func_one Determines which sensor_rcv function to register as a callback
 * @param The unique id that will be assigned to this object
 *
 * @return 0 on success, <0 for an error
 */
static int
init_test_ctrl( test_control_s *test_control, bool use_func_one, uint16_t control_id )
{
  printf( "%s: control id: %i, function one: %i\n", __FUNCTION__, control_id, use_func_one );

  test_control->control_id = control_id;
  sensor1_error_e error = sensor1_open( &test_control->sensor1_handle,
                        use_func_one ? sensor_rcv_msg1 : sensor_rcv_msg2,
                        (intptr_t)(&test_control->control_id) );
  if( SENSOR1_SUCCESS != error ) {
     printf( "******%s: sensor1_open returned %d\n", __FUNCTION__, error );
     return -1;
  }

  test_control->sensor_reports = NULL;
  test_control->report_id_cnt = 0;

  test_control->resp_result = 0;
  pthread_mutex_init( &test_control->cb_mutex, NULL );
  pthread_cond_init( &test_control->cb_cond, NULL );
  pthread_mutex_init( &test_control->reports_mutex, NULL );

  printf( "%s: %i complete\n", __FUNCTION__, test_control->control_id );

  return 0;
}

/**
 * Clears and frees and sensor reports associated with the given sensor1 handle
 *
 * @param test_control
 */
static void
clear_sensor_reports( test_control_s *test_control )
{
  sensor_report_s *temp;

  pthread_mutex_lock( &test_control->reports_mutex );
  while( NULL != test_control->sensor_reports )
  {
    temp = test_control->sensor_reports;
    test_control->sensor_reports = test_control->sensor_reports->next;
    free( temp );
  }
  test_control->sensor_reports = NULL;
  pthread_mutex_unlock( &test_control->reports_mutex );
}

/**
 * Destroys the contents of the test_control object
 *
 * @param test_control
 *
 * @return Result of sensor1_close
 */
static sensor1_error_e
destroy_test_ctrl( test_control_s *test_control )
{
  sensor_report_s *temp;
  printf( "%s: control id: %i\n", __FUNCTION__, test_control->control_id );

  sensor1_error_e error = sensor1_close( test_control->sensor1_handle );
  if( SENSOR1_SUCCESS != error )
  {
    printf( "******%s: sensor1_close returned %d\n", __FUNCTION__, error );
  }

  clear_sensor_reports( test_control );

  test_control->sensor1_handle = NULL;
  test_control->resp_result = 0;
  pthread_mutex_destroy( &test_control->cb_mutex );
  pthread_cond_destroy( &test_control->cb_cond );
  pthread_mutex_destroy( &test_control->reports_mutex );

  printf( "%s: %i complete\n", __FUNCTION__, test_control->control_id );

  return error;
}

void
writable_cb( intptr_t cb_data, uint32_t service_id )
{
  printf( "%s: received writable response; control id: %i; svc id: %i\n",
          __FUNCTION__, *(uint16_t*)cb_data, service_id );
  signal_response( 0, *(uint16_t*)cb_data, NULL );
}

/**
 * Registers a writable callback for service ID 12
 *
 * @param test_control
 */
static void
test_writable( test_control_s *test_control )
{
  sensor1_error_e error;
  printf( "%s: begin \n", __FUNCTION__ );

  pthread_mutex_lock( &test_control->cb_mutex );
  error = sensor1_writable( test_control->sensor1_handle,
                            writable_cb,
                            (intptr_t)(&test_control->control_id),
                            12 );

  // Potential Enhancement:
  // Randomly choose a service ID to register with
  // Record registered service ID and confirm in writable_cb

  if( SENSOR1_SUCCESS != error ) {
    printf( "******%s: sensor1_writable returned %i\n", __FUNCTION__, error );
  }
  if( wait_for_response( test_control ) == false )
  {
    printf( "******%s: wait_for_response failed\n", __FUNCTION__);
  }

  pthread_mutex_unlock( &test_control->cb_mutex );
}

/**
 * Make a new report request for SMGR
 *
 * @param test_control
 * @param sensor_id ID of the sensor as specified in sns_smgr_v01.h
 * @param data_type Data type (primary, secondary) of the request
 * @param report_rate
 * @param report_id Unique ID per sensor1 connection for this report
 */
static void
test_report_add( test_control_s *test_control, uint16_t sensor_id,
    uint16_t data_type, uint16_t report_rate, uint16_t report_id )
{
  sensor1_msg_header_s msg_hdr;
  sns_smgr_periodic_report_req_msg_v01 *smgr_req;
  sensor1_error_e error;
  sensor_report_s *new_report;

  printf( "%s: control id: %i, sensor id: %i, datatype: %i, report rate: %i, report id: %i\n",
      __FUNCTION__, test_control->control_id, sensor_id, data_type, report_rate, report_id );

  msg_hdr.service_number = SNS_SMGR_SVC_ID_V01;
  msg_hdr.msg_id = SNS_SMGR_REPORT_REQ_V01;
  msg_hdr.msg_size = sizeof(sns_smgr_periodic_report_req_msg_v01);
  msg_hdr.txn_id = 123;

  error = sensor1_alloc_msg_buf( test_control->sensor1_handle,
                                 sizeof(sns_smgr_periodic_report_req_msg_v01),
                                 (void**)&smgr_req );
  if( SENSOR1_SUCCESS != error ) {
    printf( "******%s: sensor1_alloc_msg_buf returned %i\n", __FUNCTION__, error );
    return ;
  }

  smgr_req->ReportId = report_id;
  smgr_req->Action = SNS_SMGR_REPORT_ACTION_ADD_V01;
  smgr_req->ReportRate = report_rate; // 30 Hz -- 33 ms
  smgr_req->BufferFactor = 1;
  smgr_req->Item_len = 1;
  smgr_req->Item[0].SensorId = sensor_id;
  smgr_req->Item[0].DataType = data_type;
  smgr_req->Item[0].Sensitivity = 0;
  smgr_req->Item[0].Decimation = SNS_SMGR_DECIMATION_RECENT_SAMPLE_V01;
  smgr_req->Item[0].MinSampleRate = 0;
  smgr_req->Item[0].StationaryOption = SNS_SMGR_REST_OPTION_NO_REPORT_V01;
  smgr_req->Item[0].DoThresholdTest = 0;
  smgr_req->Item[0].ThresholdOutsideMinMax = 0;
  smgr_req->Item[0].ThresholdDelta = 0;
  smgr_req->Item[0].ThresholdAllAxes = 0;
  smgr_req->Item[0].ThresholdMinMax[0] = 0;
  smgr_req->Item[0].ThresholdMinMax[1] = 0;

  pthread_mutex_lock( &test_control->cb_mutex );
  error = sensor1_write( test_control->sensor1_handle,
                         &msg_hdr,
                         smgr_req );

  if( SENSOR1_SUCCESS != error ) {
    printf( "******%s: sensor1_write returned %d\n", __FUNCTION__, error );
  }
  if( wait_for_response( test_control ) == false )
  {
    printf( "******%s: wait_for_response failed\n", __FUNCTION__);
  }

  pthread_mutex_unlock( &test_control->cb_mutex );

  new_report = malloc( sizeof(sensor_report_s) );
  if( NULL == new_report )
  {
    printf( "%s: Malloc Error\n", __FUNCTION__ );
  }
  else
  {
    new_report->report_id = report_id;
    new_report->next = test_control->sensor_reports;
    new_report->sensor_rate = report_rate;
    test_control->sensor_reports = new_report;
  }

  printf( "%s: %i complete\n", __FUNCTION__, test_control->control_id );
}

static void
test_cancel( test_control_s *test_control )
{
  sensor1_msg_header_s msg_hdr;
  sns_smgr_periodic_report_req_msg_v01 *smgr_req;
  sensor1_error_e error;

  printf( "%s: control id: %i\n", __FUNCTION__, test_control->control_id );

  msg_hdr.service_number = SNS_SMGR_SVC_ID_V01;
  msg_hdr.msg_id = SNS_SMGR_CANCEL_REQ_V01;
  msg_hdr.msg_size = 0;
  msg_hdr.txn_id = 123;

  pthread_mutex_lock( &test_control->cb_mutex );

  error = sensor1_write( test_control->sensor1_handle,
                         &msg_hdr,
                         NULL );

  if( SENSOR1_SUCCESS != error ) {
    printf( "******%s: sensor1_write returned %d\n", __FUNCTION__, error );
  }

  if( wait_for_response( test_control ) == false )
  {
     printf( "******%s: wait_for_response failed\n", __FUNCTION__);
  }

   pthread_mutex_unlock( &test_control->cb_mutex );

   clear_sensor_reports( test_control );

   printf( "%s: %i complete\n", __FUNCTION__, test_control->control_id );
}

/**
 * Deletes the specified report from the sensor1 connection
 *
 * @param test_control
 * @param report_id
 */
static void
test_delete( test_control_s *test_control, uint16_t report_id )
{
  sensor1_msg_header_s msg_hdr;
  sns_smgr_periodic_report_req_msg_v01 *smgr_req;
  sensor1_error_e error;
  sensor_report_s *curr, *prev = NULL;

  printf( "%s: control id: %i, report_id: %i\n",
          __FUNCTION__, test_control->control_id, report_id );

  msg_hdr.service_number = SNS_SMGR_SVC_ID_V01;
  msg_hdr.msg_id = SNS_SMGR_REPORT_REQ_V01;
  msg_hdr.msg_size = sizeof(sns_smgr_periodic_report_req_msg_v01);
  msg_hdr.txn_id = 123;

  error = sensor1_alloc_msg_buf( test_control->sensor1_handle,
                                 sizeof(sns_smgr_periodic_report_req_msg_v01),
                                 (void**)&smgr_req );
  if( SENSOR1_SUCCESS != error ) {
    printf( "******%s: sensor1_alloc_msg_buf returned %i\n", __FUNCTION__, error );
    return ;
  }

  smgr_req->ReportId = report_id;
  smgr_req->Action = SNS_SMGR_REPORT_ACTION_DELETE_V01;
  smgr_req->ReportRate = 0;
  smgr_req->BufferFactor = 0;
  smgr_req->Item_len = 0;

  pthread_mutex_lock( &test_control->cb_mutex );

  error = sensor1_write( test_control->sensor1_handle,
                         &msg_hdr,
                         smgr_req );

  if( SENSOR1_SUCCESS != error ) {
    printf( "******%s: sensor1_write returned %d\n", __FUNCTION__, error );
  }

  if( wait_for_response( test_control ) == false )
  {
     printf( "******%s: wait_for_response failed\n", __FUNCTION__);
  }

  pthread_mutex_unlock( &test_control->cb_mutex );

  pthread_mutex_lock( &test_control->reports_mutex );
  curr = test_control->sensor_reports;
  while( NULL != curr )
  {
    if( curr->report_id == report_id )
    {
      if( prev == NULL )
      {
        test_control->sensor_reports = curr->next;
      }
      else
      {
        prev->next = curr->next;
      }

      free( curr );
      break;
    }
    prev = curr;
    curr = curr->next;
  }
  pthread_mutex_unlock( &test_control->reports_mutex );

  printf( "%s: %i complete\n", __FUNCTION__, test_control->control_id );
}

/**
 * @return a random active report ID for the test control
 */
static int
get_used_report_id( test_control_s *test_control )
{
  pthread_mutex_lock( &test_control->reports_mutex );
  uint8_t index = rand() % (test_control->report_id_cnt + 1);
  sensor_report_s *curr, *prev;
  uint16_t i;
  int rv = -1;

  curr = test_control->sensor_reports;
  for( i = 0; curr != NULL && i < index; i++ )
  {
    curr = (NULL == curr->next) ? test_control->sensor_reports : curr->next;
    rv = curr->report_id;
  }
  pthread_mutex_unlock( &test_control->reports_mutex );

  return rv;
}

/**
 *
 * @param thread_cnt The index of this thread
 */
static void
test_thread( void *args )
{
  uint8_t thread_cnt = *(uint8_t*)args;
  int i, result;
  uint32_t sleep_us;

  printf( "%s: Starting thread %i\n", __FUNCTION__, thread_cnt );

  for( i = 0; i < sensors_per_thread_g; i++ )
  {
    uint16_t control_index = thread_cnt * sensors_per_thread_g + i;
    result = init_test_ctrl( &test_controls_g[ control_index ], i % 2, control_index );

    if( 0 != result )
    {
      exit(EXIT_FAILURE);
    }
  }

  if( 0 == sensors_per_thread_g )
  {
    return ;
  }

  for( i = 0; i < actions_per_thread_g; i++ )
  {
    uint8_t action = rand() % 4;
    int16_t report_id;
    test_control_s *test_control;
    uint16_t test_control_index = thread_cnt * sensors_per_thread_g +
                                  rand() % sensors_per_thread_g;

    printf( "%s: Starting action %i for test control index: %i\n", __FUNCTION__, action, test_control_index );
    test_control = &test_controls_g[ test_control_index ];

    if( 0 == action )
    {
      uint16_t sensor_id = rand() % SENSOR_ID_COUNT;
      uint16_t data_type = rand() % DATA_TYPE_COUNT;
      pthread_mutex_lock( &test_control->reports_mutex );
      report_id = test_control->report_id_cnt++;
      pthread_mutex_unlock( &test_control->reports_mutex );
      uint16_t report_rate = (rand() % 45) + 5;

      test_report_add( test_control, sensor_id, data_type, report_rate, report_id );
    }
    else if( 1 == action )
    {
      test_cancel( test_control );
    }
    else if( 2 == action )
    {
      report_id = get_used_report_id( test_control );

      if( -1 == report_id )
      {
        printf( "%s: No report to close.  Thread: %i, control index: %i\n",
                __FUNCTION__, thread_cnt, test_control_index );
      }
      else
      {
        test_delete( test_control, report_id );
      }
    }
    else if( 3 == action )
    {
      test_writable( test_control );
    }

    sleep_us = (rand() % 1000) * 1000;
    printf( "%s: Thread %i sleeping for %i us\n", __FUNCTION__, thread_cnt, sleep_us );
    usleep( sleep_us );
  }

  for( i = 0; i < sensors_per_thread_g; i++ )
  {
    destroy_test_ctrl( &test_controls_g[ thread_cnt * sensors_per_thread_g + i ] );
  }

  free( args );
}

int
main( int argc, char * const argv[])
{
  pthread_t *threads;
  int rc;
  uint8_t i;
  time_t curr_time = time( NULL );

  sensor1_init();
  printf( "%s: Starting cm_conc_test.  Seeding srand with: %lli\n",
          __FUNCTION__, (int64_t)curr_time );
  srand( curr_time );

  for( i = 1; i < argc; i++ )
  {
    if( argv[i][0] == '-' )
    {
      switch( argv[i][1] )
      {
        case 's':
          sensors_per_thread_g = atoi( argv[ ++i ] );
          printf( "%s: Settings sensors per thread: %i\n",
                  __FUNCTION__, sensors_per_thread_g );
          break;
        case 't':
          thread_count_g = atoi( argv[ ++i ] );
          printf( "%s: Settings thread count: %i\n",
                  __FUNCTION__, thread_count_g );
          break;
        case 'a':
          actions_per_thread_g = atoi( argv[ ++i ] );
          printf( "%s: Settings actions per thread: %i\n",
                  __FUNCTION__, actions_per_thread_g );
          break;
        case 'h':
          printf( "\n-s <#> : # of sensor1 connections per thread\n-t <#> : # of threads to create\n-a <#> : How many actions each thread should take before finishing \n\n" );
          return -1;
        default:
          printf( "******%s: Unknown parameter flag: %c\n", __FUNCTION__, argv[i][1] );
          break;
      }
    }
  }

  if( MAX_CNT < actions_per_thread_g ||
      MAX_CNT < thread_count_g ||
      MAX_CNT < sensors_per_thread_g )
  {
    printf( "%s: Invalid input parameters: %i, %i, %i\n", __FUNCTION__,
            actions_per_thread_g, thread_count_g, sensors_per_thread_g );
    return 0;
  }

  test_controls_g = malloc( thread_count_g * sensors_per_thread_g * sizeof(test_control_s) );
  if( NULL == test_controls_g )
  {
    printf( "%s: Malloc Error for test_controls_g\n", __FUNCTION__ );
    return 0;
  }

  threads = malloc( thread_count_g * sizeof(pthread_t) );
  if( NULL == threads )
  {
    printf( "%s: Malloc Error for threads\n", __FUNCTION__ );
    free( test_controls_g );
    return 0;
  }

  for( i = 0; i < thread_count_g; i++ )
  {
    uint8_t *thread_cnt = (uint8_t*)malloc( sizeof(uint8_t) );
    if( NULL == thread_cnt )
    {
      printf( "%s: Malloc Error for thread %i\n", __FUNCTION__, i );
    }
    else
    {
      memcpy( thread_cnt, &i, 1 );
      printf( "%s: creating thread %i\n", __FUNCTION__, i );
      rc = pthread_create(&threads[i], NULL, (void *) &test_thread, (void *)thread_cnt);

      if( 0 != rc )
        printf( "******%s: Error creating thread %i\n", __FUNCTION__, i );
    }
  }

  for( i = 0; i < thread_count_g; i++ )
  {
    rc = pthread_join(threads[i], NULL);

    if( 0 != rc )
      printf( "******%s: Error waiting for thread %i\n", __FUNCTION__, i );
  }

  free( threads );
  free( test_controls_g );

  return 0;
}
