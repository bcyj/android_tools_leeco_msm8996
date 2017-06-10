/*============================================================================
  @file sns_file_test.c

  @brief
    To test the file service's ability to handles file open, write, and
    close requests.
  @author gju

  Modify build to enable testing:
  - Copy sns_file_internal_v01.h from /dsps/sensorsdaemon/common/idl/inc to /dsps/api

  - Add to sensors/Android.mk
    LOCAL_COPY_HEADERS      += dsps/api/sns_file_internal_v01.h

  - Modify entry #36 in libsensor1.c::svc_map_g:
    SNS_GET_SVC_OBJ( FILE_INTERNAL, 01)

  - Add include to libsensor1.c:
    #include "sns_file_internal_v01.h"

  Copyright (c) 2012-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ==========================================================================*/

/*============================================================================
  INCLUDE FILES
  ==========================================================================*/

#include "sensor1.h"
#include "sns_common_v01.h"
#include "sns_file_internal_v01.h"
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

typedef struct
{
  sensor1_handle_s *sensor1_handle;
  bool is_cb_arrived; /* If a RESP has been received yet */
  pthread_mutex_t cb_mutex; /* Wait for RESP */
  pthread_cond_t cb_cond; /* Wait for RESP */
  int32_t resp_result;
} test_control_s;

/*============================================================================
  Static Variable Definitions
  ==========================================================================*/

static test_control_s *test_control_g;

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

   printf( "%s\n", __FUNCTION__ );

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
 * Indicate to the waiting thread that a response has been received
 *
 * @param result Which cb function received the response
 */
static void
signal_response( int32_t result, test_control_s *test_control )
{
  test_control->resp_result = result;
  test_control->is_cb_arrived = true;
  pthread_cond_signal( &test_control->cb_cond );
}

/**
 *  Callback handler for all sensor1 connections.
 */
static void
sensor_rcv_msg( intptr_t cb_data, sensor1_msg_header_s *msg_hdr,
                     sensor1_msg_type_e msg_type, void *msg_ptr )
{
  if( NULL == msg_hdr ) {
    printf( "%s: received NULL msg_hdr_ptr! type: %i\n", __FUNCTION__, msg_type );
    return ;
  }

  printf( "%s: received message; type: %i; svc num: %i; id: %i; cb_data: %i\n",
          __FUNCTION__, msg_type, msg_hdr->service_number, msg_hdr->msg_id, cb_data );

  if( SNS_FILE_INTERNAL_OPEN_RESP_V01 == msg_hdr->msg_id )
  {
    sns_file_open_resp_msg_v01 *open_resp = (sns_file_open_resp_msg_v01*)msg_ptr;
    signal_response( open_resp->fildes_valid ? open_resp->fildes : -1, test_control_g );
  }
  else if( SNS_FILE_INTERNAL_WRITE_RESP_V01 == msg_hdr->msg_id )
  {
    sns_file_write_resp_msg_v01 *write_resp = (sns_file_write_resp_msg_v01*)msg_ptr;
    signal_response( write_resp->bytes_written_valid ? (int32_t)write_resp->bytes_written : -1,
                     test_control_g );
  }
  else if( SNS_FILE_INTERNAL_CLOSE_RESP_V01 == msg_hdr->msg_id )
  {
    sns_file_close_resp_msg_v01 *close_resp = (sns_file_close_resp_msg_v01*)msg_ptr;
    signal_response( close_resp->resp.sns_err_t, test_control_g );
  }
  else
  {
    printf( "******%s: Unexpected message type: %i, %i (cb data %i)\n",
            __FUNCTION__, msg_type, msg_hdr->service_number, *(uint16_t*)cb_data );
  }

  if( NULL != msg_ptr )
  {
    sensor1_free_msg_buf( test_control_g->sensor1_handle, msg_ptr );
  }
}

/**
 * Initializes a test_control object.
 *
 * @param control
 *
 * @return 0 on success, <0 for an error
 */
static int
init_test_ctrl( test_control_s **test_control )
{
  test_control_s *control = malloc( sizeof(test_control_s) );
  sensor1_error_e error =
    sensor1_open( &control->sensor1_handle, sensor_rcv_msg, 1 );
  if( SENSOR1_SUCCESS != error )
  {
     printf( "******%s: sensor1_open returned %d\n", __FUNCTION__, error );
     return -1;
  }

  control->resp_result = -1;
  pthread_mutex_init( &control->cb_mutex, NULL );
  pthread_cond_init( &control->cb_cond, NULL );

  *test_control = control;
  return 0;
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
  sensor1_error_e error = sensor1_close( test_control->sensor1_handle );
  if( SENSOR1_SUCCESS != error )
  {
    printf( "******%s: sensor1_close returned %d\n", __FUNCTION__, error );
  }

  test_control->sensor1_handle = NULL;
  test_control->resp_result = 0;
  pthread_mutex_destroy( &test_control->cb_mutex );
  pthread_cond_destroy( &test_control->cb_cond );

  free( test_control );

  return error;
}

/**
 * Make a new report request for SMGR
 *
 * @param test_control
 * @param file_name
 * @param mode
 *
 * @return File descriptor
 */
static int
test_req_open( test_control_s *test_control, char *file_name, char *mode )
{
  sensor1_msg_header_s msg_hdr;
  sns_file_open_req_msg_v01 *sensor1_req;
  sensor1_error_e error;
  int32_t rv;

  printf( "%s: Open request\n", __FUNCTION__ );

  msg_hdr.service_number = SNS_FILE_INTERNAL_SVC_ID_V01;
  msg_hdr.msg_id = SNS_FILE_INTERNAL_OPEN_REQ_V01;
  msg_hdr.msg_size = sizeof(sns_file_open_req_msg_v01);
  msg_hdr.txn_id = 1;

  error = sensor1_alloc_msg_buf( test_control->sensor1_handle,
                                 sizeof(sns_file_open_req_msg_v01),
                                 (void**)&sensor1_req );
  if( SENSOR1_SUCCESS != error ) {
    printf( "******%s: sensor1_alloc_msg_buf returned %i\n", __FUNCTION__, error );
    return -1;
  }

  sensor1_req->path_name_len = strlen( file_name );
  strncpy( sensor1_req->path_name, file_name, SNS_FILE_MAX_FILENAME_SIZE_V01 );
  sensor1_req->mode_len = strlen( mode );
  strncpy( sensor1_req->mode, mode, SNS_FILE_MAX_MODE_SIZE_V01 );

  pthread_mutex_lock( &test_control->cb_mutex );
  error = sensor1_write( test_control->sensor1_handle,
                         &msg_hdr,
                         sensor1_req );
  if( SENSOR1_SUCCESS != error ) {
    printf( "******%s: sensor1_write returned %d\n", __FUNCTION__, error );
  }

  if( wait_for_response( test_control ) == false )
  {
     printf( "******%s: wait_for_response failed\n", __FUNCTION__);
  }

  rv = test_control->resp_result;
  pthread_mutex_unlock( &test_control->cb_mutex );

  printf( "%s: Open req complete: %i \n", __FUNCTION__, rv );
  return rv;
}

/**
 * Make a new report request for SMGR
 *
 * @param test_control
 * @param file_name
 * @param mode
 *
 * @return File descriptor
 */
static int
test_req_write( test_control_s *test_control, int32_t fildesc, char const *buf )
{
  sensor1_msg_header_s msg_hdr;
  sns_file_write_req_msg_v01 *sensor1_req;
  sensor1_error_e error;
  int32_t rv;

  printf( "%s: Write request\n", __FUNCTION__ );

  msg_hdr.service_number = SNS_FILE_INTERNAL_SVC_ID_V01;
  msg_hdr.msg_id = SNS_FILE_INTERNAL_WRITE_REQ_V01;
  msg_hdr.msg_size = sizeof(sns_file_write_req_msg_v01);
  msg_hdr.txn_id = 2;

  error = sensor1_alloc_msg_buf( test_control->sensor1_handle,
                                 sizeof(sns_file_write_req_msg_v01),
                                 (void**)&sensor1_req );
  if( SENSOR1_SUCCESS != error ) {
    printf( "******%s: sensor1_alloc_msg_buf returned %i\n", __FUNCTION__, error );
    return -1;
  }

  sensor1_req->fildes = fildesc;
  sensor1_req->buf_len = strlen( buf );
  strncpy( (char*)sensor1_req->buf, buf, SNS_FILE_MAX_FILENAME_SIZE_V01 );

  pthread_mutex_lock( &test_control->cb_mutex );
  error = sensor1_write( test_control->sensor1_handle,
                         &msg_hdr,
                         sensor1_req );
  if( SENSOR1_SUCCESS != error ) {
    printf( "******%s: sensor1_write returned %d\n", __FUNCTION__, error );
  }

  if( wait_for_response( test_control ) == false )
  {
     printf( "******%s: wait_for_response failed\n", __FUNCTION__);
  }

  rv = test_control->resp_result;
  pthread_mutex_unlock( &test_control->cb_mutex );

  printf( "%s: Write req complete: %i \n", __FUNCTION__, rv );
  return rv;
}

static int
test_req_close( test_control_s *test_control, int32_t fildes )
{
  sensor1_msg_header_s msg_hdr;
  sns_file_close_req_msg_v01 *sensor1_req;
  sensor1_error_e error;
  int32_t rv;

  printf( "%s: Close request\n", __FUNCTION__ );

  msg_hdr.service_number = SNS_FILE_INTERNAL_SVC_ID_V01;
  msg_hdr.msg_id = SNS_FILE_INTERNAL_CLOSE_REQ_V01;
  msg_hdr.msg_size = sizeof(sns_file_close_req_msg_v01);
  msg_hdr.txn_id = 3;

  error = sensor1_alloc_msg_buf( test_control->sensor1_handle,
                                 sizeof(sns_file_close_req_msg_v01),
                                 (void**)&sensor1_req );
  if( SENSOR1_SUCCESS != error ) {
    printf( "******%s: sensor1_alloc_msg_buf returned %i\n", __FUNCTION__, error );
    return -1;
  }

  sensor1_req->fildes = fildes;

  pthread_mutex_lock( &test_control->cb_mutex );
  error = sensor1_write( test_control->sensor1_handle,
                         &msg_hdr,
                         sensor1_req );
  if( SENSOR1_SUCCESS != error ) {
    printf( "******%s: sensor1_write returned %d\n", __FUNCTION__, error );
  }

  if( wait_for_response( test_control ) == false )
  {
     printf( "******%s: wait_for_response failed\n", __FUNCTION__);
  }

  rv = test_control->resp_result;
  pthread_mutex_unlock( &test_control->cb_mutex );

  printf( "%s: Close req complete: %i \n", __FUNCTION__, rv );
  return rv;
}

int
main( int argc, char * const argv[] )
{
  pthread_t *threads;
  int rc;
  char choice[3] = "1\n";
  char file_name[256], mode[256], fd_name[256], buf[512];
  int32_t fildesc, bytes_written, close_result;

  sensor1_init();
  init_test_ctrl( &test_control_g );
  printf( "%s: Starting sns_file_test.\n", __FUNCTION__ );
  printf( "1:Open file\n2:Write to file\n3:Close file\n" );

  while( '0' != choice[0] )
  {
    printf( "Make choice: " );
    fgets( choice, 3, stdin );
    switch( choice[0] )
    {
      case '1':
        printf( "Enter file name to open: " );
        fgets( file_name, 256, stdin );
        file_name[ strlen( file_name ) - 1 ] = '\0';
        printf( "Enter file mode: " );
        fgets( mode, 256, stdin );
        mode[ strlen( mode ) - 1 ] = '\0';
        fildesc = test_req_open( test_control_g, file_name, mode );
        printf( "New file descriptor: %i\n", fildesc );
        break;
      case '2':
        printf( "Enter file descriptor: " );
        fgets( fd_name, 256, stdin );
        fildesc = atoi ( fd_name );
        printf( "Enter data to write: " );
        fgets( buf, 256, stdin );
        buf[ strlen( buf ) - 1 ] = '\0';
        bytes_written = test_req_write( test_control_g,
                                        fildesc, buf );
        printf( "Bytes written: %i\n", bytes_written );
        break;
      case '3':
        printf( "Enter file descriptor to close: " );
        fgets( fd_name, 256, stdin );
        fildesc = atoi ( fd_name );
        close_result = test_req_close( test_control_g, fildesc );
        printf( "Close result: %i\n", close_result );
        break;
      default:
        printf( "******%s: Unknown option: %c\n", __FUNCTION__, choice[0] );
        break;
    }
  }

  destroy_test_ctrl( test_control_g );

  return 0;
}
