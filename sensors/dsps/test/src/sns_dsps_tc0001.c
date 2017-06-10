/*============================================================================
  @file sns_dsps_tc0001.c

  @brief
    Tests basic functionality of all sensor services.

  Copyright (c) 2012-2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Confidential and Proprietary
  ==========================================================================*/

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

/* Receive this many sensor indications before declaring the test successful */
#define NUM_INDICATIONS_TO_TEST 10

pthread_mutex_t my_mutex;
pthread_cond_t my_cond;
uint8_t  my_txn_id;
uint8_t  num_indications;

sensor1_error_e error;
struct timespec ts;

sns_smgr_all_sensor_info_resp_msg_v01 all_sensor_info;
sns_smgr_single_sensor_info_resp_msg_v01 single_sensor_info[SNS_SMGR_MAX_SENSOR_NUM_V01];

void handle_smgr_all_sensor_info_resp( sensor1_handle_s *hndl, void *msg_ptr )
{
  uint32_t i;
  all_sensor_info = *((sns_smgr_all_sensor_info_resp_msg_v01*) msg_ptr);

  printf("Retrieved all sensor info\n");
  for( i = 0; i < all_sensor_info.SensorInfo_len; i ++ ) {
    char name[SNS_SMGR_SHORT_SENSOR_NAME_SIZE_V01 +1];
    memcpy(name, all_sensor_info.SensorInfo[i].SensorShortName, all_sensor_info.SensorInfo[i].SensorShortName_len );
    name[all_sensor_info.SensorInfo[i].SensorShortName_len] = 0;
    /*
    printf("\tSensorID %s ShortName: %s\n",
           (all_sensor_info.SensorInfo[i].SensorID == SNS_SMGR_ID_ACCEL_V01) ? "ACCEL":
           (all_sensor_info.SensorInfo[i].SensorID == SNS_SMGR_ID_GYRO_V01) ? "GYRO":
           (all_sensor_info.SensorInfo[i].SensorID == SNS_SMGR_ID_MAG_V01) ? "MAG":
           (all_sensor_info.SensorInfo[i].SensorID == SNS_SMGR_ID_PRESSURE_V01) ? "PRESSURE":
           (all_sensor_info.SensorInfo[i].SensorID == SNS_SMGR_ID_PROX_LIGHT_V01) ? "PROX_LIGHT":"OTHER",
           name );
    */
  }
}

void
handle_smgr_single_sensor_info_resp( void *msg_ptr )
{
  static int num_sensors = 0;
  uint32_t i;
  sns_smgr_single_sensor_info_resp_msg_v01 *info =
    (sns_smgr_single_sensor_info_resp_msg_v01*) msg_ptr;

  single_sensor_info[num_sensors] = *info;
  for( i = 0; i < info->SensorInfo.data_type_info_len; i++ ) {
    char sensor_name[SNS_SMGR_MAX_SENSOR_NAME_SIZE_V01+1];
    char vendor_name[SNS_SMGR_MAX_VENDOR_NAME_SIZE_V01+1];

    memcpy(sensor_name,
           info->SensorInfo.data_type_info[i].SensorName,
           info->SensorInfo.data_type_info[i].SensorName_len);
    sensor_name[info->SensorInfo.data_type_info[i].SensorName_len] = 0;

    memcpy(vendor_name,
           info->SensorInfo.data_type_info[i].VendorName,
           info->SensorInfo.data_type_info[i].VendorName_len);
    vendor_name[info->SensorInfo.data_type_info[i].VendorName_len] = 0;

    printf("Sensor Name %s\nVendor Name %s\nSensor id %d\tDataType %d\nVersion %u\t"
           "MaxSampleRate %u\tIdlePower %u\nMax Power %u\tMax Range %u\tResolution %u\n",
           sensor_name, vendor_name,
           info->SensorInfo.data_type_info[i].SensorID,
           info->SensorInfo.data_type_info[i].DataType,
           info->SensorInfo.data_type_info[i].Version,
           info->SensorInfo.data_type_info[i].MaxSampleRate,
           info->SensorInfo.data_type_info[i].IdlePower,
           info->SensorInfo.data_type_info[i].MaxPower,
           info->SensorInfo.data_type_info[i].MaxRange,
           info->SensorInfo.data_type_info[i].Resolution );
  }

  num_sensors++;
}

void handle_smgr_report_resp( void *msg_ptr )
{
  /* Starting a new sampling session. Since this test
     only tests one sensor at a time, set the number
     of received indications to 0 now that a new stream
     has just been started. */
  num_indications = 0;
}

void notify_data_cb( intptr_t data,
                     sensor1_msg_header_s *msg_hdr_ptr,
                     sensor1_msg_type_e msg_type,
                     void *msg_ptr )
{
  if( NULL == msg_hdr_ptr ) {
    printf("\nreceived NULL msg_hdr_ptr!\n");
  } else {
    /*
    printf("hdr.service_number: %u\n\thdr.msg_id: %d\n\t"
           "hdr.msg_type: %d\n\thdr.msg_size: %d\n\thdr.txn_id: %d\n",
           msg_hdr_ptr->service_number,
           msg_hdr_ptr->msg_id,
           msg_type,
           msg_hdr_ptr->msg_size,
           msg_hdr_ptr->txn_id );
    */
  }

  if( msg_type == SENSOR1_MSG_TYPE_RESP && NULL != msg_hdr_ptr ) {
    //printf("received RESP\n");

    if( msg_hdr_ptr->service_number == SNS_SMGR_SVC_ID_V01 ) {
      switch (msg_hdr_ptr->msg_id) {
        case SNS_SMGR_ALL_SENSOR_INFO_RESP_V01:
          handle_smgr_all_sensor_info_resp( *((sensor1_handle_s**)data),
                                            msg_ptr );
          break;
        case SNS_SMGR_SINGLE_SENSOR_INFO_RESP_V01:
          handle_smgr_single_sensor_info_resp( msg_ptr );
          break;
        case SNS_SMGR_REPORT_RESP_V01:
          handle_smgr_report_resp( msg_ptr );
          break;
        default:
          break;
      }
    }

    pthread_mutex_lock( &my_mutex );
    my_txn_id = msg_hdr_ptr->txn_id;
    pthread_cond_signal( &my_cond );
    pthread_mutex_unlock( &my_mutex );
  } else if( msg_type == SENSOR1_MSG_TYPE_IND ) {
    printf("*");
    fflush(NULL);
    pthread_mutex_lock( &my_mutex );
    num_indications++;
    pthread_cond_signal( &my_cond );
    pthread_mutex_unlock( &my_mutex );
  } else if( msg_type == SENSOR1_MSG_TYPE_BROKEN_PIPE ) {
    printf("\nreceived BROKEN_PIPE!!!\n");
  } else {
    printf( "\nreceived INVALID MSG type: %i!!!\n", msg_type );
  }
  if( NULL != msg_ptr ) {
    sensor1_free_msg_buf( *((sensor1_handle_s**)data), msg_ptr );
  }
}

void send_req( sensor1_handle_s *hndl, void *req, sensor1_msg_header_s msg_hdr )
{
  error = sensor1_write( hndl,
                         &msg_hdr,
                         req );

  if( SENSOR1_SUCCESS != error ) {
    printf("\nsensor1_write returned %d\n", error);
    if( SENSOR1_EWOULDBLOCK != error ) {
      exit(error);
    }
  }
  // Make sure we get a response
  error = 0;
  pthread_mutex_lock( &my_mutex );
  clock_gettime( CLOCK_REALTIME, &ts );
  while( (my_txn_id != msg_hdr.txn_id) && error == 0 ) {
    ts.tv_sec += 100000000; // wait 100msec
    error = pthread_cond_timedwait( &my_cond, &my_mutex, &ts );
  }
  my_txn_id = 0;
  pthread_mutex_unlock( &my_mutex );
  if( 0 != error ) {
    printf("\nError while waiting for response message %d\n", error);
    exit(error);
  } else {
    //printf("\nGot response to message\n");
  }
}

void alloc_cancel( uint8_t svc_num, sensor1_msg_header_s *msg_hdr )
{
  msg_hdr->service_number = svc_num;
  msg_hdr->msg_id = SNS_SMGR_CANCEL_REQ_V01;
  msg_hdr->msg_size = 0;
  msg_hdr->txn_id = 28;
}

void test_open( sensor1_handle_s **hndl_ptr )
{
  error = sensor1_open( hndl_ptr,
                        notify_data_cb,
                        (intptr_t)hndl_ptr );

  if( SENSOR1_SUCCESS != error ) {
    printf("\nsensor1_open returned %d\n", error);
    exit(error);
  }
}

void
alloc_smgr_all_sensor_info_req( sensor1_handle_s *hndl, sensor1_msg_header_s *msg_hdr )
{
  msg_hdr->service_number = SNS_SMGR_SVC_ID_V01;
  msg_hdr->msg_id = SNS_SMGR_ALL_SENSOR_INFO_REQ_V01;
  msg_hdr->msg_size = 0;
  msg_hdr->txn_id = 7;
}

sns_smgr_single_sensor_info_req_msg_v01*
alloc_smgr_single_sensor_info_req( sensor1_handle_s *hndl, sensor1_msg_header_s *msg_hdr, uint8_t SensorID)
{
  sensor1_error_e error;
  sns_smgr_single_sensor_info_req_msg_v01 *smgr_req;

  msg_hdr->service_number = SNS_SMGR_SVC_ID_V01;
  msg_hdr->msg_id = SNS_SMGR_SINGLE_SENSOR_INFO_REQ_V01;
  msg_hdr->msg_size = sizeof(sns_smgr_single_sensor_info_req_msg_v01);
  msg_hdr->txn_id = 12+SensorID;

  error = sensor1_alloc_msg_buf( hndl,
                                 sizeof(sns_smgr_single_sensor_info_req_msg_v01),
                                 (void**)&smgr_req );
  if( SENSOR1_SUCCESS != error ) {
    printf("\nsensor1_alloc_msg_buf returned %d\n", error);
    return NULL;
  }
  smgr_req->SensorID = SensorID;
  return smgr_req;
}


sns_smgr_periodic_report_req_msg_v01*
alloc_smgr_report_req( sensor1_handle_s *hndl, sensor1_msg_header_s *msg_hdr,
                       uint8_t SensorId, uint8_t DataType, uint8_t Action, uint16_t ReportRate )
{
  sensor1_error_e error;
  sns_smgr_periodic_report_req_msg_v01 *smgr_req;
  //printf("Testing alloc\n");
  error = sensor1_alloc_msg_buf( hndl,
                                 sizeof(sns_smgr_periodic_report_req_msg_v01),
                                 (void**)&smgr_req );
  if( SENSOR1_SUCCESS != error ) {
    printf("\nsensor1_alloc_msg_buf returned %d\n", error);
    return NULL;
  }
  msg_hdr->service_number = SNS_SMGR_SVC_ID_V01;
  msg_hdr->msg_id = SNS_SMGR_REPORT_REQ_V01;
  msg_hdr->msg_size = sizeof(sns_smgr_periodic_report_req_msg_v01);
  msg_hdr->txn_id = 62;

  smgr_req->ReportId = 0;
  smgr_req->Action = Action;
  smgr_req->ReportRate = ReportRate;
  smgr_req->BufferFactor = 1;
  smgr_req->Item_len = 1;
  smgr_req->Item[0].SensorId = SensorId;
  smgr_req->Item[0].DataType = DataType;
  smgr_req->Item[0].Sensitivity = 0; // Default
  smgr_req->Item[0].Decimation = SNS_SMGR_DECIMATION_RECENT_SAMPLE_V01;
  smgr_req->Item[0].MinSampleRate = 0;
  smgr_req->Item[0].StationaryOption = SNS_SMGR_REST_OPTION_NO_REPORT_V01;
  smgr_req->Item[0].DoThresholdTest = 0;
  smgr_req->Item[0].ThresholdOutsideMinMax = 0;
  smgr_req->Item[0].ThresholdDelta = 0;
  smgr_req->Item[0].ThresholdAllAxes = 0;
  smgr_req->Item[0].ThresholdMinMax[0] = 0;
  smgr_req->Item[0].ThresholdMinMax[1] = 0;

  return smgr_req;
}

void init( void )
{
  sensor1_init();

  pthread_mutex_init( &my_mutex, NULL );
  pthread_cond_init( &my_cond, NULL );
}

int main( int argc, char * const argv[])
{
  sensor1_handle_s *hndl1;
  int               opt;
  uint32_t          i,j;

  printf( "Starting %s\n", argv[0] );

  init();

  test_open( &hndl1 );

  /* Get the sensor list */
  {
    sensor1_msg_header_s smgr_all_info_req_hdr;
    alloc_smgr_all_sensor_info_req( hndl1, &smgr_all_info_req_hdr );
    send_req( hndl1, NULL, smgr_all_info_req_hdr );
  }
  /* Get info about each sensor */
  for( i = 0; i < all_sensor_info.SensorInfo_len; i ++ ) {
    sns_smgr_single_sensor_info_req_msg_v01 *smgr_single_info_req_msg;
    sensor1_msg_header_s smgr_single_info_req_hdr;
    smgr_single_info_req_msg =
      alloc_smgr_single_sensor_info_req(hndl1, &smgr_single_info_req_hdr, all_sensor_info.SensorInfo[i].SensorID);
    send_req( hndl1, smgr_single_info_req_msg, smgr_single_info_req_hdr );
  }
  /* Verify list of sensors */
  {
    bool accel_pres = false;
    bool gyro_pres = false;
    bool mag_pres = false;
    bool prox_pres = false;
    bool light_pres = false;

    for( i = 0; i < all_sensor_info.SensorInfo_len; i ++ ) {
      switch(single_sensor_info[i].SensorInfo.data_type_info[0].SensorID) {
        case SNS_SMGR_ID_ACCEL_V01:
          accel_pres = true;
          break;
        case SNS_SMGR_ID_GYRO_V01:
          gyro_pres = true;
          break;
        case SNS_SMGR_ID_MAG_V01:
          mag_pres = true;
          break;
        case SNS_SMGR_ID_PROX_LIGHT_V01:
          prox_pres = true;
          light_pres = true;
          break;
      }
    }
    if( accel_pres && gyro_pres && mag_pres && prox_pres && light_pres ) {
      printf("All sensors found\n");
    } else {
      printf("Not all sensors present: accel:%d gyro:%d mag:%d prox:%d light:%d\n",
             accel_pres, gyro_pres, mag_pres, prox_pres, light_pres );
      exit(2);
    }
  }

  /* Request data from each sensor */
  /* Get NUM_INDICATIONS_TO_TEST samples from each sensor, 
     then cancel the request */
  for( i = 0; i < all_sensor_info.SensorInfo_len; i ++ ) {
    sns_smgr_periodic_report_req_msg_v01 *smgr_report_req;
    sensor1_msg_header_s req_hdr;
    /* For prox/als, only  only check for one indication */
    uint8_t num_ind_to_test =
      (all_sensor_info.SensorInfo[i].SensorID == SNS_SMGR_ID_PROX_LIGHT_V01) ?
        1 : NUM_INDICATIONS_TO_TEST;

    for( j = 0; j < single_sensor_info[i].SensorInfo.data_type_info_len; j++ ) {

      smgr_report_req =
        alloc_smgr_report_req( hndl1, &req_hdr,
                               single_sensor_info[i].SensorInfo.data_type_info[j].SensorID,
                               j, SNS_SMGR_REPORT_ACTION_ADD_V01,
                               single_sensor_info[i].SensorInfo.data_type_info[j].MaxSampleRate);
      printf("\nSending request for sensor %s\n",
             single_sensor_info[i].SensorInfo.data_type_info[j].SensorName);
      send_req( hndl1, smgr_report_req, req_hdr );
      /* The handling of the report response will set num_indications
         to 0 */
      pthread_mutex_lock( &my_mutex );
      error = 0;
      while( num_indications < num_ind_to_test && error == 0) {
        clock_gettime( CLOCK_REALTIME, &ts );
        /* Wait for 1 second for each indication */
        ts.tv_sec++;
        error = pthread_cond_timedwait( &my_cond, &my_mutex, &ts );
      }

      if( error != 0 || num_indications < num_ind_to_test ) {
        printf("\nExiting due to error %s(%d). num ind %d. sensor id %d\n",
               strerror(error), error, num_indications,
               single_sensor_info[i].SensorInfo.data_type_info[j].SensorID );
        exit(1);
      }
      pthread_mutex_unlock( &my_mutex );
      printf("\n\tCanceling all requests\n" );
      alloc_cancel( SNS_SMGR_SVC_ID_V01, &req_hdr );
      send_req( hndl1, NULL, req_hdr );
    }
  }

  printf("All tests passed\n");

  return 0;
}
