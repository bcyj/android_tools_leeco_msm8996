/*============================================================================
  @file sns_cm_test.c

  @brief
    To test basic functionality of the client manager through it's sensor1
    interface.

  Copyright (c) 2012, 2014 QUALCOMM Technologies, Inc.  All Rights Reserved
  Qualcomm Technologies Confidential and Proprietary
  ==========================================================================*/
#define __STDC_FORMAT_MACROS

#include "sensor1.h"
#include "sns_common_v01.h"
#include "sns_smgr_api_v01.h"
#include <errno.h>
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
#include <utils/SystemClock.h>

#ifdef SNS_LA
#include <linux/msm_dsps.h>
#endif /* SNS_LA */

static const char usage_fmt[] =
  "\nUsage: %s [-r sample_rate] [-d duration_in_seconds] [-s sensor_id]\n"
  "Default setting: -r 30 -d 1 -s 0\n";

#define NOTIFY_DATA 0xBEEFCAFE
#define WRITABLE_DATA 0xDEADBEEF

//#define SNS_LATENCY_MEASUREMENT

#define DSPS_TICK_TIME_USEC(clkval) (((int64_t)clkval*(int64_t)1000000LL)/(int64_t)dsps_clk_rate)

#ifndef UNIX_PATH_MAX
#define UNIX_PATH_MAX 108
#endif /* UNIX_MAX_PATH */
pthread_mutex_t my_mutex;
pthread_cond_t my_cond;
int my_predicate = 0;
sensor1_error_e error;
sensor1_msg_header_s msg_hdr;
struct timespec ts;
sns_smgr_periodic_report_req_msg_v01 *smgr_req;

int dsps_fd = -1;
int64_t dsps_clk_offset_usec = 0;
unsigned int dsps_clk_ticks = 0;
int64_t dsps_clk_rate = 32768;

void get_dsps_clk_offset( void )
{
#ifdef SNS_LA
  struct sched_param sched_param;
  int err = 0;
  static bool err_printed = false;

  if (dsps_fd == -1)
    return;

  err = sched_getparam( 0, &sched_param );
  if( err != 0 /* && err_printed == false */ ) {
    err_printed = true;
    perror("sched_getparam: ");
  }
  sched_param.sched_priority = sched_get_priority_max( SCHED_FIFO );
  if( sched_param.sched_priority == -1 /* && err_printed == false */ ) {
    err_printed = true;
    perror("sched_get_priority_max(SCHED_FIFO): ");
  }
  err = sched_setscheduler( 0, SCHED_FIFO, &sched_param );
  if( err != 0 /* && err_printed == false */ ) {
    err_printed = true;
    perror("sched_setscheduler(SCHED_FIFO): ");
  }

#ifdef SNS_LATENCY_MEASUREMENT
  /**
   * Attempt to read DSPS time from /dev/msm_dsps
   */
  err = ioctl( dsps_fd, DSPS_IOCTL_READ_SLOW_TIMER, &dsps_clk_ticks );
  if( err == -1 ) {
    perror("ioctl(/dev/msm_dsps, READ_SLOW_TIMER): ");
  }
#endif

  clock_gettime( CLOCK_REALTIME, &ts);

  sched_param.sched_priority = 0;
  err = sched_setscheduler( 0, SCHED_OTHER, &sched_param );
  if( err != 0 /* && err_printed == false */ ) {
    err_printed = true;
    perror("sched_setscheduler(SCHED_OTHER): ");
  }

  dsps_clk_offset_usec = ( + (int64_t)ts.tv_sec*1000000LL
                           - (int64_t)DSPS_TICK_TIME_USEC(dsps_clk_ticks)
                           + (int64_t)ts.tv_nsec/1000LL );

  printf("DSPS clock offset hr:min:sec.ms.usec %02" PRId64":%02" PRId64":%02" PRId64".%03" PRId64".%03" PRId64"\n",
         dsps_clk_offset_usec/1000000/60/60, /* hours */
         dsps_clk_offset_usec/1000000/60 % 60, /* Minutes */
         dsps_clk_offset_usec/1000000 % 60, /* Seconds */
         dsps_clk_offset_usec/1000 % 1000 /* ms */,
         dsps_clk_offset_usec % 1000 /* usec */);
#endif /* SNS_LA */
}

void parent_notify_data_cb( intptr_t data,
                            sensor1_msg_header_s *msg_hdr_ptr,
                            sensor1_msg_type_e msg_type,
                            void *msg_ptr )
{
#ifdef SNS_LA
  struct timespec now;
  static int i = 0;
#endif /* SNS_LA */
  if( NULL == msg_hdr_ptr ) {
    //printf("\ncm_test: received NULL msg_hdr_ptr!\n");
  } else {
    /*
    printf("cm_test: hdr.service_number: %u\n\thdr.msg_id: %d\n\t"
           "hdr.msg_type: %d\n\thdr.msg_size: %d\n\thdr.txn_id: %d\n",
           msg_hdr_ptr->service_number,
           msg_hdr_ptr->msg_id,
           msg_type,
           msg_hdr_ptr->msg_size,
           msg_hdr_ptr->txn_id );
    */
    //printf("*");
    fflush(NULL);
  }

  if( msg_type == SENSOR1_MSG_TYPE_RESP ) {
    //printf("cm_test: received RESP\n");
    pthread_mutex_lock( &my_mutex );
    my_predicate = 1;
    pthread_cond_signal( &my_cond );
    pthread_mutex_unlock( &my_mutex );
  } else if( msg_type == SENSOR1_MSG_TYPE_IND ) {
    //printf("cm_test: received IND\n");
    uint8_t id = ((sns_smgr_periodic_report_ind_msg_v01*)msg_ptr)->Item[0].SensorId;
    uint8_t type = ((sns_smgr_periodic_report_ind_msg_v01*)msg_ptr)->Item[0].DataType;
    uint8_t flag = ((sns_smgr_periodic_report_ind_msg_v01*)msg_ptr)->Item[0].ItemFlags;
    uint8_t quality =  ((sns_smgr_periodic_report_ind_msg_v01*)msg_ptr)->Item[0].ItemQuality;
    int32_t *data = ((sns_smgr_periodic_report_ind_msg_v01*)msg_ptr)->Item[0].ItemData;
    printf("ID=%d,Type=%d,Val=%d,%d,%d", id, type, data[0], data[1], data[2]);
    if (quality != SNS_SMGR_ITEM_QUALITY_FILTERED_V01)
      printf("   Quality=%d,Flag=%d\n", quality, flag);
    printf("\n");
  } else if( msg_type == SENSOR1_MSG_TYPE_BROKEN_PIPE ) {
    printf("\ncm_test: received BROKEN_PIPE!!!\n");
  } else if( msg_type == SENSOR1_MSG_TYPE_RETRY_OPEN ) {
    printf("\ncm_test: received RETRY_OPEN!!!\n");
    error = sensor1_open( (sensor1_handle_s **)data,
                          parent_notify_data_cb,
                          (intptr_t)((sensor1_handle_s **)data) );
    if( SENSOR1_SUCCESS != error ) {
      printf("\ncm_test: retry opened failed with %d\n", error);
    } else {
      pthread_mutex_lock( &my_mutex );
      my_predicate = 4;
      pthread_cond_signal( &my_cond );
      pthread_mutex_unlock( &my_mutex );
    }
  } else {
    printf("\ncm_test: received INVALID MSG type!!!\n");
  }
#if defined(SNS_LA) && defined(SNS_LATENCY_MEASUREMENT)
  if( ((i % 10) == 0) &&
      msg_type == SENSOR1_MSG_TYPE_IND &&
      msg_hdr_ptr != NULL &&
      msg_hdr_ptr->msg_id == SNS_SMGR_REPORT_IND_V01 ) {
    int64_t delta;
    sns_smgr_periodic_report_ind_msg_v01 *rpt = msg_ptr;

    get_dsps_clk_offset();

    clock_gettime( CLOCK_REALTIME, &now);
    delta = ( (int64_t)now.tv_sec*1000000LL
              - (int64_t)dsps_clk_offset_usec
              + (int64_t)now.tv_nsec/1000LL
              - (int64_t)DSPS_TICK_TIME_USEC(rpt->Item[0].TimeStamp) );

    printf("Message one-way delay usec: %" PRId64 "\n", delta);
    printf("msg delay usec using ticks: %u\n", dsps_clk_ticks - rpt->Item[0].TimeStamp );
  }
  i++;
#endif /* SNS_LA && SNS_LATENCY_MEASUREMENT */
  if( NULL != msg_ptr ) {
    sensor1_free_msg_buf( *((sensor1_handle_s**)data), msg_ptr );
  }
}

void write_cb( intptr_t cb_data,
               uint32_t service_id )
{
  if( WRITABLE_DATA != (uintptr_t)cb_data ) {
    printf( "\ncm_test: write_cb with wrong data %x\n", (int)cb_data );
  }
  printf( "\ncm_test: write_cb data\n" );
  pthread_mutex_lock( &my_mutex );
  my_predicate = 2;
  pthread_cond_signal( &my_cond );
  pthread_mutex_unlock( &my_mutex );
}

void test_writable( sensor1_handle_s *hndl )
{
  printf("Testing writable\n");
  error = sensor1_writable( hndl,
                            write_cb,
                            WRITABLE_DATA,
                            (intptr_t)12 );
  if( SENSOR1_SUCCESS != error ) {
    printf("\ncm_test: sensor1_writable returned %d\n", error);
    exit(error);
  }
  // Make sure we get a response
  pthread_mutex_lock( &my_mutex );
  error = SENSOR1_SUCCESS;
  clock_gettime( CLOCK_REALTIME, &ts );
  while( my_predicate != 2 && error == SENSOR1_SUCCESS ) {
    ts.tv_sec += 1; // wait 1 nsecond
    error = (sensor1_error_e)pthread_cond_timedwait( &my_cond, &my_mutex, &ts );
  }
  my_predicate = 0;
  pthread_mutex_unlock( &my_mutex );
  if( SENSOR1_SUCCESS != error ) {
    printf("\nError while waiting for writable callback %d\n", error);
    exit(error);
  } else {
    printf("\nGot writable callback\n");
  }

}

void test_write( sensor1_handle_s *hndl )
{
  printf("Testing write\n");
  error = sensor1_write( hndl,
                         &msg_hdr,
                         smgr_req );

  if( SENSOR1_SUCCESS != error ) {
    printf("\ncm_test: sensor1_write returned %d\n", error);
    if( SENSOR1_EWOULDBLOCK != error ) {
      exit(error);
    }
  }
  // Make sure we get a response
  error = SENSOR1_SUCCESS;
  pthread_mutex_lock( &my_mutex );
  clock_gettime( CLOCK_REALTIME, &ts );
  while( my_predicate != 1 && error == SENSOR1_SUCCESS ) {
    ts.tv_sec += 100000000; // wait 100msec
    error = (sensor1_error_e)pthread_cond_timedwait( &my_cond, &my_mutex, &ts );
  }
  my_predicate = 0;
  pthread_mutex_unlock( &my_mutex );
  if( SENSOR1_SUCCESS != error ) {
    printf("\nError while waiting for response message %d\n", error);
    exit(error);
  } else {
    printf("\nGot response to message\n");
  }
}

void test_cancel( sensor1_handle_s *hndl )
{
  printf("\nTesting cancel\n");
  msg_hdr.service_number = SNS_SMGR_SVC_ID_V01;
  msg_hdr.msg_id = SNS_SMGR_CANCEL_REQ_V01;
  msg_hdr.msg_size = 0;
  msg_hdr.txn_id = 5;

  error = sensor1_write( hndl,
                         &msg_hdr,
                         NULL );

  if( SENSOR1_SUCCESS != error ) {
    printf("\ncm_test: sensor1_write returned %d\n", error);
    if( SENSOR1_EWOULDBLOCK != error ) {
      //exit(error);
    }
  }
  // Make sure we get a response
  error = SENSOR1_SUCCESS;
  pthread_mutex_lock( &my_mutex );
  clock_gettime( CLOCK_REALTIME, &ts );
  while( my_predicate != 1 && error == 0 ) {
    ts.tv_nsec += 100000000; // wait 100msec
    error = (sensor1_error_e)pthread_cond_timedwait( &my_cond, &my_mutex, &ts );
  }
  my_predicate = 0;
  pthread_mutex_unlock( &my_mutex );
  if( SENSOR1_SUCCESS != error ) {
    printf("\nError while waiting for response message %d\n", error);
    //exit(error);
  } else {
    printf("\nGot response to cancel\n");
  }
}

void test_invalid_writes( sensor1_handle_s *hndl)
{
  printf("Testing write with NULL sensor handle\n");
  error = sensor1_write( (sensor1_handle_s*)NULL,
                         &msg_hdr,
                         smgr_req );

  if( SENSOR1_SUCCESS == error ) {
    printf("\ncm_test: sensor1_write succeeded on NULL handle!\n");
    if( SENSOR1_EWOULDBLOCK != error ) {
      exit(2);
    }
  }

  printf("Testing write with BAD sensor handle\n");
  error = sensor1_write( (sensor1_handle_s*)0xBADADD,
                         &msg_hdr,
                         smgr_req );

  if( SENSOR1_SUCCESS == error ) {
    printf("\ncm_test: sensor1_write succeeded on bad handle!\n");
    exit(2);
  }
}

void test_open( sensor1_handle_s **hndl_ptr )
{
  printf("Testing open\n");
  error = sensor1_open( hndl_ptr,
                        parent_notify_data_cb,
                        (intptr_t)hndl_ptr );

  if( SENSOR1_EWOULDBLOCK == error ) {
    printf("\ncm_test: sensor1_open returned EWOULDBLOCK. Waiting 60 sec for sensor availability\n");
    pthread_mutex_lock( &my_mutex );
    error = SENSOR1_SUCCESS;
    clock_gettime( CLOCK_REALTIME, &ts );
    while( my_predicate != 4 && error == 0 ) {
      ts.tv_sec += 60; // wait 60 seconds
      error = (sensor1_error_e)pthread_cond_timedwait( &my_cond, &my_mutex, &ts );
    }
    if( my_predicate &= 4 ) {
      printf("sensor1 now available\n");
    } else {
      printf("error waiting for sensor1\n");
      exit(1);
    }
    my_predicate = 0;
    pthread_mutex_unlock( &my_mutex );
  } else if( SENSOR1_SUCCESS != error ) {
    printf("\ncm_test: sensor1_open returned %d\n", error);
    exit(error);
  }
}

void test_alloc( sensor1_handle_s *hndl)
{
  printf("Testing alloc\n");
  error = sensor1_alloc_msg_buf( hndl,
                                 sizeof(sns_smgr_periodic_report_req_msg_v01),
                                 (void**)&smgr_req );
  if( SENSOR1_SUCCESS != error ) {
    printf("\ncm_test: sensor1_alloc_msg_buf returned %d\n", error);
    exit(error);
  }
  smgr_req->ReportId = 0;
  smgr_req->Action = SNS_SMGR_REPORT_ACTION_ADD_V01;
  smgr_req->ReportRate = 30; // 30 Hz -- 33 ms
  smgr_req->BufferFactor = 1;
  smgr_req->Item_len = 1;
  smgr_req->Item[0].SensorId = SNS_SMGR_ID_ACCEL_V01;
  smgr_req->Item[0].DataType = SNS_SMGR_DATA_TYPE_PRIMARY_V01;
  smgr_req->Item[0].Sensitivity = 0; // Default
  smgr_req->Item[0].Decimation = SNS_SMGR_DECIMATION_FILTER_V01;
  smgr_req->Item[0].MinSampleRate = 0;
  smgr_req->Item[0].StationaryOption = SNS_SMGR_REST_OPTION_NO_REPORT_V01;
  smgr_req->Item[0].DoThresholdTest = 0;
  smgr_req->Item[0].ThresholdOutsideMinMax = 0;
  smgr_req->Item[0].ThresholdDelta = 0;
  smgr_req->Item[0].ThresholdAllAxes = 0;
  smgr_req->Item[0].ThresholdMinMax[0] = 0;
  smgr_req->Item[0].ThresholdMinMax[1] = 0;
}

void test_delete( sensor1_handle_s *hndl )
{
  test_alloc( hndl );
  smgr_req->Action = SNS_SMGR_REPORT_ACTION_DELETE_V01;
  printf("Testing delete\n");
  error = sensor1_write( hndl,
                         &msg_hdr,
                         smgr_req );

  if( SENSOR1_SUCCESS != error ) {
    printf("\ncm_test: sensor1_write returned %d\n", error);
    if( SENSOR1_EWOULDBLOCK != error ) {
      exit(error);
    }
  }
  // Make sure we get a response
  error = SENSOR1_SUCCESS;
  pthread_mutex_lock( &my_mutex );
  clock_gettime( CLOCK_REALTIME, &ts );
  while( my_predicate != 1 && error == 0 ) {
    ts.tv_sec += 100000000; // wait 100msec
    error = (sensor1_error_e)pthread_cond_timedwait( &my_cond, &my_mutex, &ts );
  }
  my_predicate = 0;
  pthread_mutex_unlock( &my_mutex );
  if( 0 != error ) {
    printf("\nError while waiting for response message %d\n", error);
    exit(error);
  } else {
    printf("\nGot response to message\n");
  }
}

void test_close( sensor1_handle_s *hndl )
{
  printf("Testing close\n");
  error = sensor1_close( hndl );
  if( SENSOR1_SUCCESS != error ) {
    printf("\ncm_test: sensor1_close returned %d\n", error);
    exit(error);
  }
}

void init( void )
{

  sensor1_init();

  msg_hdr.service_number = SNS_SMGR_SVC_ID_V01;
  msg_hdr.msg_id = SNS_SMGR_REPORT_REQ_V01;
  msg_hdr.msg_size = sizeof(sns_smgr_periodic_report_req_msg_v01);
  msg_hdr.txn_id = 123;

  pthread_mutex_init( &my_mutex, NULL );
  pthread_cond_init( &my_cond, NULL );

#ifdef SNS_LA
  dsps_fd = open("/dev/msm_dsps", O_RDONLY );
  if( dsps_fd == -1 ) {
    //perror("open(/dev/msm_dsps, O_RDONLY): ");
  }

  get_dsps_clk_offset();
#endif /* SNS_LA */
}

int main( int argc, char * const argv[])
{
  sensor1_handle_s *hndl1;
  int               rate = 30;
  int               duration = 1;
  int               sensor_id =  SNS_SMGR_ID_ACCEL_V01;
  int               opt;
  struct timespec ts_mono, ts_real, ts_boot_start, ts_boot_end, ts_boot;
  uint64_t        elapsed_realtime;

  //  socketpair( AF_UNIX, SOCK_SEQPACKET, 0, socket_pair );

  while( (opt = getopt(argc, argv, "r:h:d:s:" ))!= -1 ) {
    switch(opt) {
      case 'r':
        rate = atoi(optarg);
        break;
      case 'h':
        dsps_clk_rate = atoi(optarg);
        break;
      case 'd':
        duration = atoi(optarg);
        break;
      case 's':
        sensor_id = atoi(optarg);
        break;
      case '?':
        fprintf(stderr, usage_fmt, argv[0]);
        exit(0);
      default:
        break;
    }
  }

  clock_gettime( CLOCK_MONOTONIC, &ts_mono );
  clock_gettime( CLOCK_BOOTTIME, &ts_boot_start );
  elapsed_realtime = android::elapsedRealtimeNano();
  clock_gettime( CLOCK_BOOTTIME, &ts_boot_end );

  ts_boot.tv_sec = (ts_boot_start.tv_sec + ts_boot_end.tv_sec) / 2;
  ts_boot.tv_nsec = (ts_boot_start.tv_nsec + ts_boot_end.tv_nsec) / 2;

  clock_gettime( CLOCK_REALTIME, &ts_real );
  printf("mono: %ld.%ld\nboot: %ld.%ld\nreal: %ld.%ld\n",
         ts_mono.tv_sec,ts_mono.tv_nsec,
         ts_boot.tv_sec,ts_boot.tv_nsec,
         ts_real.tv_sec,ts_real.tv_nsec );
  printf("\nmono: %lld\nboot: %lld\nreal: %lld\n",
         ((uint64_t)ts_mono.tv_sec * 1000000000) + ts_mono.tv_nsec,
         ((uint64_t)ts_boot.tv_sec * 1000000000) + ts_boot.tv_nsec,
         ((uint64_t)ts_real.tv_sec * 1000000000) + ts_real.tv_nsec );
  printf("\nelapsedRealtimeNano: %lld\n", elapsed_realtime );
  printf("Elapsed Realtime - clock_boottime: %lld\n",
         ((uint64_t)ts_boot.tv_sec * 1000000000) + ts_boot.tv_nsec - elapsed_realtime );
  init();

  test_open( &hndl1 );
  test_alloc( hndl1 );
  test_writable( hndl1 );
  printf("Setting report rate to %d\n", rate);
  smgr_req->ReportRate = rate;
  printf("Asking sensor data with id %d\n", sensor_id);
  smgr_req->Item[0].SensorId = sensor_id;
  test_write( hndl1 );
  printf("returned from test_write\n");

  while (duration == 0)
  { // wait forever
    sleep(1);
  }
  sleep(duration);
  test_delete( hndl1 );

  test_invalid_writes( hndl1 );
  sleep(2);

  usleep(100);
  test_cancel( hndl1 );
  sleep(5);
  printf("Parent calling close\n");
  test_close( hndl1 );
  printf("Parent close returned\n");

  wait(NULL);
  printf("All tests passed\n");

  return 0;
}
