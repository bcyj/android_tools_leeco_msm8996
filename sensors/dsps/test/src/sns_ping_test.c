/*============================================================================
  @file sns_ping_test.c

  @brief
    Sends a version request to all sensor services via sensor1 API

  Copyright (c) 2013 QUALCOMM Technologies, Inc.  All Rights Reserved
  Qualcomm Technologies Confidential and Proprietary
  ==========================================================================*/

#include "sensor1.h"
#include "sns_common_v01.h"

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

#define UNREFERENCED_PARAMETER(x) (void)x;

pthread_mutex_t mutex;
pthread_cond_t cond;
int predicate = 0;
sensor1_error_e error;
sensor1_msg_header_s msg_hdr;
struct timespec ts;

typedef struct svc_list_s {
  char* name;
  int   svc_num;
} svc_list_t;


svc_list_t svc_list[] = { {"Sensor Manager  ", 0},
                          {"Power Manager  ", 1},
                          {"Sensor Message Router on the DSPS  ", 2},
                          {"Sensor Registry -- legacy  ", 3},
                          {"Algorithm: Absolute Motion Detection  ", 4},
                          {"Algorithm: Relative Motion Detection  ", 5},
                          {"Algorithm: Vehicle Motion Detection  ", 6},
                          {"Sensors Debug Interface on DSPS  ", 7},
                          {"Sensors Diag Interface on DSPS  ", 8},
                          {"Face and Shake service on the Apps Processor  ", 9},
                          {"Algorithm: Bring to Ear  ", 10},
                          {"Algorithm: Quaternion  ", 11},
                          {"Algorithm: Gravity  ", 12},
                          {"SMGR internal service  ", 13},
                          {"Debug internal service  ", 14},
                          {"Sensor Registry V02  ", 15},
                          {"Algorithm: Mag Calibration  ", 16},
                          {"Algorithm: Filtered Magnetic Vector  ", 17},
                          {"Algorithm: Rotation Vector  ", 18},
                          {"Algorithm: Gyro based Quaternion  ", 19},
                          {"Algorithm: Gravity Vector  ", 20},
                          {"Algorithm: Sensor Threshold  ", 21},
                          {"Sensors Time Service  ", 22},
                          {"Algorithm: Orientation  ", 23},
                          {"Sensors Time Service V02  ", 24},
                          {"Algorithm: Basic Gestures  ", 25},
                          {"Algorithm: Tap  ", 26},
                          {"Algorithm: Facing  ", 27},
                          {"Algorithm: Integrated Angle  ", 28},
                          {"Algorithm: Gyro assisted Tap ", 29},
                          {"Algorithm: Gyro assisted Tap V2 ", 30},
                          {"Reserved for OEM use ", 31},
                          {"Reserved for OEM use ", 32},
                          {"Reserved for OEM use ", 33},
                          {"Algorithm: Gyrobuf algorithm (EIS)  ", 34},
                          {"Algorithm: Gyroint algorithm (EIS)  ", 35},
                          {"Sensors file internal interface  ", 36},
                          {"Algorithm: Pedometer  ", 37},
                          {"Algorithm: Pedestrian Activity Monitor  ", 38},
                          {"Algorithm: Detect modem scenarios  ", 39},
                          {"Algorithm: Significant Motion Detector  ", 40},
                          {"Algorithm: Coarse Motion Classifier  ", 41},
                          {"Algorithm: Distance Bound  ", 42},
};


void notify_data_cb( intptr_t data,
                     sensor1_msg_header_s *msg_hdr_ptr,
                     sensor1_msg_type_e msg_type,
                     void *msg_ptr )
{
  sns_common_version_resp_msg_v01 *ver_resp = (sns_common_version_resp_msg_v01*)msg_ptr;

  UNREFERENCED_PARAMETER( data );

  if( NULL == msg_hdr_ptr ) {
    return;
  }
  if( msg_type == SENSOR1_MSG_TYPE_RESP && msg_hdr_ptr->msg_id == 1 ) {
    if( ver_resp->resp.sns_result_t != 0 ) {
      printf("resp ERROR %d\n", ver_resp->resp.sns_err_t);
    } else {
      printf("ver resp. version %d, max msg %d\n",
             ver_resp->interface_version_number,
             ver_resp->max_message_id);
    }
    pthread_mutex_lock( &mutex );
    predicate = msg_hdr_ptr->service_number;
    pthread_cond_signal( &cond );
    pthread_mutex_unlock( &mutex );
  }
}


void write_ver_req( sensor1_handle_s *hndl, int svc_id )
{
  int error = 0;
  void* ver_req;
  sensor1_msg_header_s ver_req_hdr = { .service_number = svc_id,
                                       /* Ver req is always msg ID 1 */ .msg_id = 1,
                                       .msg_size = 1,
                                       /* set txn id to svc id */ .txn_id = svc_id };
  printf("Sending ver request to %s(%d)... ", svc_list[svc_id].name, svc_list[svc_id].svc_num );
  fflush(stdout);

  error = sensor1_alloc_msg_buf( hndl,
                                 1,
                                 &ver_req );

  error = sensor1_write( hndl, &ver_req_hdr, ver_req );

  if( error == SENSOR1_EBAD_MSG_SZ ) {
    ver_req_hdr.msg_size = 0;
    error = sensor1_write( hndl, &ver_req_hdr, ver_req );
  }

  if( error != SENSOR1_SUCCESS ) {
    printf("sensor1_write error %d!\n", error );
    return;
  }

  pthread_mutex_lock( &mutex );
  predicate = -1;
  error = 0;
  clock_gettime( CLOCK_REALTIME, &ts );
  while( predicate == -1 && error == 0 ) {
    ts.tv_nsec += 500000000; // wait 500msec
    error = pthread_cond_timedwait( &cond, &mutex, &ts );
  }
  predicate = -1;
  pthread_mutex_unlock( &mutex );

  if( error == ETIMEDOUT ) {
    printf("Request timed out\n");
  };
}


int main( int argc, char * const argv[])
{
  sensor1_handle_s *hndl_ptr;
  sensor1_error_e error;
  int i;
  UNREFERENCED_PARAMETER(argc);
  UNREFERENCED_PARAMETER(argv);

  pthread_mutex_init( &mutex, NULL );
  pthread_cond_init( &cond, NULL );

  sensor1_init();
  error = sensor1_open( &hndl_ptr,
                        notify_data_cb,
                        (intptr_t)(&hndl_ptr) );
  if( SENSOR1_SUCCESS != error ) {
    printf("\n%s: sensor1_open returned %d\n", argv[0], error);
    exit(error);
  }

  for( i = 0; i < (int)(sizeof(svc_list) / sizeof(svc_list_t)); i++ ) {
    write_ver_req( hndl_ptr, i );
  }
  return 0;
}
