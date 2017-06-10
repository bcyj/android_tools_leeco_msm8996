/*============================================================================
  @file sns_sam_test.c

  @brief
    Test file for SAM.

  Copyright (c) 2012,2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ==========================================================================*/

/*===========================================================================
  Include Files
  ===========================================================================*/
#include "sensor1.h"
#include "sns_sam_ped_v01.h"
#include "sns_sam_amd_v01.h"
#include "sns_sam_quaternion_v01.h"
#include "sns_sam_filtered_mag_v01.h"
#include "sns_sam_gravity_vector_v01.h"
#include "sns_sam_rotation_vector_v01.h"
#include "sns_sam_smd_v01.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>

/*============================================================================
  Static Data
  ===========================================================================*/

/* generic fixed point macros */
#define FX_FLTTOFIX(f,q)     ((int32_t)( (f)*(1<<(q))+(((f)>(0.0))?(0.5):(-0.5))))
#define FX_FIXTOFLT(i,q)     (((double)(i))/((double)(1<<(q))))
#define FX_TRUNC(a,q)        ((a)&(0xFFFFFFFF<<(q)))
#define FX_ROUND(a,q)        (FX_TRUNC((a)+FX_HALF(q),q))

/* convert a from q1 format to q2 format */
#define FX_CONV(a,q1,q2)     (((q2)>(q1))?(a)<<((q2)-(q1)):(a)>>((q1)-(q2)))

#define FX_ADD(a,b,q1,q2,q3) (FX_CONV((a),(q1),(q3))+FX_CONV((b),(q2),(q3)))
#define FX_SUB(a,b,q1,q2,q3) (FX_CONV((a),(q1),(q3))-FX_CONV((b),(q2),(q3)))
#define FX_MUL(a,b,q1,q2,q3) ((int32_t)(FX_CONV((((int64_t)(int32_t)(a))*((int64_t)(int32_t)(b))),((q1)+(q2)),(q3))))
#define FX_DIV(a,b,q1,q2,q3) ((int32_t)(FX_CONV(((int64_t)(int32_t)(a)),(q1),(q2)+(q3))/((int64_t)(int32_t)(b))))

/* Q16 fixed point macros */
#define FX_QFACTOR           (16)
#define FX_MAX_Q16           ((int32_t) 0x7FFFFFFF)
#define FX_MIN_Q16           ((int32_t) 0x80000000)

#define FX_FLTTOFIX_Q16(d)   (FX_FLTTOFIX(d,FX_QFACTOR))
#define FX_FIXTOFLT_Q16(a)   (FX_FIXTOFLT(a,FX_QFACTOR))

#ifndef UNREFERENCED_PARAMETER
#define UNREFERENCED_PARAMETER(x) (void)x;
#endif /* UNREFERENCED_PARAMETER */

/*============================================================================
  Type Definitions
  ===========================================================================*/

typedef enum {
  SAM_ENABLE,
  SAM_DISABLE,
  SAM_UPDATE,
  SAM_BATCH,
  SAM_GET_REPORT,
  SAM_CANCEL,
  SAM_MSG_CNT
} sam_message_type;

typedef enum {
  ALGO_AMD,
  ALGO_PED,
  ALGO_QUAT,
  ALGO_FMV,
  ALGO_GRAV,
  ALGO_SMD,
  ALGO_ROTV,
  ALGO_CNT
} sam_algo;

typedef void (*msg_func)( void* );

typedef struct {
  sensor1_msg_header_s header[ SAM_MSG_CNT ];
  msg_func request[ SAM_MSG_CNT ];
} sam_messages;

/*============================================================================
  Global Data
  ===========================================================================*/

sensor1_handle_s *my_handle;
pthread_mutex_t my_mutex;
pthread_cond_t my_cond;
int my_predicate = 0;

sigset_t set;

uint8_t algo_instance_id;
int rcv_ind_count = 0;
int sample_rate = 20;
int report_period = 0;

static const char usage_fmt[] =
  "\nUsage: %s [-a algorithm] [-c ind_count] [-s sample_rate_hz] [-r report_period_sec]\n"
  "AMD: 0\n"
  "Pedometer: 1\n"
  "Quaternion: 2\n"
  "FMV: 3\n"
  "Gravity: 4\n"
  "SMD: 5\n"
  "Rotation Vector: 6\n"
  "Default setting: -a 0 -s 20 -r 0\n";

/*============================================================================
  Pedometer
  ===========================================================================*/

void handle_ped_resp( sensor1_msg_header_s *msg_hdr_ptr, void *msg_ptr )
{
  if( SNS_SAM_PED_ENABLE_RESP_V01 == msg_hdr_ptr->msg_id ) {
    sns_sam_ped_enable_resp_msg_v01 *resp_ptr = msg_ptr;
    printf( "Got ENABLE response, status = %d, id = %d\n",
            resp_ptr->resp.sns_result_t, resp_ptr->instance_id );
    algo_instance_id = resp_ptr->instance_id;
  } else if( SNS_SAM_PED_GET_REPORT_RESP_V01 == msg_hdr_ptr->msg_id ) {
    sns_sam_ped_get_report_resp_msg_v01 *resp_ptr = msg_ptr;
    printf( "Got GET_REPORT response, status = %d, id = %d, "
            "timestamp = %u, state = %d\n",
            resp_ptr->resp.sns_result_t, resp_ptr->instance_id,
            resp_ptr->timestamp, resp_ptr->report_data.step_count );
  } else if( SNS_SAM_PED_DISABLE_RESP_V01 == msg_hdr_ptr->msg_id ) {
    sns_sam_ped_disable_resp_msg_v01 *resp_ptr = msg_ptr;
    printf( "Got DISABLE response, status = %d, id = %d\n",
            resp_ptr->resp.sns_result_t, resp_ptr->instance_id );
  } else if( SNS_SAM_PED_CANCEL_RESP_V01 == msg_hdr_ptr->msg_id ) {
    sns_common_cancel_resp_msg_v01 *resp_ptr = msg_ptr;
    printf( "Got CANCEL response, status = %d\n", resp_ptr->resp.sns_result_t );
  } else if( SNS_SAM_PED_RESET_RESP_V01 == msg_hdr_ptr->msg_id ) {
    sns_sam_ped_reset_resp_msg_v01 *resp_ptr = msg_ptr;
    printf( "Got RESET response, status = %d\n", resp_ptr->resp.sns_result_t );
  } else if( SNS_SAM_PED_BATCH_RESP_V01 == msg_hdr_ptr->msg_id ) {
    sns_sam_ped_batch_resp_msg_v01 *resp_ptr = msg_ptr;
    printf( "Got BATCH response, status = %d\n", resp_ptr->resp.sns_result_t );
  } else {
    printf( "sam_test: Got INVALID response!!!\n" );
  }
}

void handle_ped_ind( sensor1_msg_header_s *msg_hdr_ptr, void *msg_ptr )
{
  if ( SNS_SAM_PED_REPORT_IND_V01 == msg_hdr_ptr->msg_id ) {
    sns_sam_ped_report_ind_msg_v01 *ind_ptr = msg_ptr;
    rcv_ind_count++;
    printf( "Got REPORT indication, id = %d, timestamp = %u, state = %d\n",
            ind_ptr->instance_id, ind_ptr->timestamp, ind_ptr->report_data.step_count );
  } else if ( SNS_SAM_PED_BATCH_IND_V01 == msg_hdr_ptr->msg_id ) {
    sns_sam_ped_batch_ind_msg_v01 *ind_ptr = msg_ptr;
    uint32_t i;

    for( i = 0; i < ind_ptr->items_len; i++ ) {
      rcv_ind_count++;
      printf( "Got BATCH indication, id = %d, timestamp = %u, state = %d\n",
              ind_ptr->instance_id, ind_ptr->items[i].timestamp, ind_ptr->items[i].report.step_count );
    }
  } else {
    printf( "sam_test: Got INVALID indication!!!\n" );
  }
}

void ped_enable_req( sns_sam_ped_enable_req_msg_v01 *sam_ped_enable_req )
{
  memset( sam_ped_enable_req, sizeof(sns_sam_ped_enable_req_msg_v01), 0 );
  //sam_ped_enable_req->report_period = FX_FLTTOFIX_Q16(1);
  //sam_ped_enable_req->step_count_threshold_valid = 1;
  //sam_ped_enable_req->step_count_threshold = 5;
}

void ped_disable_req( sns_sam_ped_disable_req_msg_v01 *sam_ped_disable_req )
{
  sam_ped_disable_req->instance_id = algo_instance_id;
}

void ped_get_report_req( sns_sam_ped_get_report_req_msg_v01 *sam_ped_get_report_req )
{
  sam_ped_get_report_req->instance_id = algo_instance_id;
}

void cancel_req( sns_common_cancel_req_msg_v01 *sam_cancel_req )
{
  UNREFERENCED_PARAMETER(sam_cancel_req);
}

void ped_batch_req( sns_sam_ped_batch_req_msg_v01 *sam_ped_batch_req )
{
  sam_ped_batch_req->instance_id = algo_instance_id;
  sam_ped_batch_req->batch_period = FX_FLTTOFIX_Q16(3);
}

void ped_reset_req( sns_sam_ped_reset_req_msg_v01 *sam_ped_reset_req )
{
  sam_ped_reset_req->instance_id = algo_instance_id;
}

void create_ped_messages( sam_messages *ped_messages )
{
  ped_messages->request[ SAM_ENABLE ] = (msg_func)&ped_enable_req;
  ped_messages->request[ SAM_DISABLE ] = (msg_func)&ped_disable_req;
  ped_messages->request[ SAM_GET_REPORT ] = (msg_func)&ped_get_report_req;
  ped_messages->request[ SAM_CANCEL ] = (msg_func)&cancel_req;
  ped_messages->request[ SAM_BATCH ] = (msg_func)&ped_batch_req;
  ped_messages->request[ SAM_UPDATE ] = (msg_func)&ped_reset_req;

  /***********************/
  ped_messages->header[ SAM_ENABLE ].service_number = SNS_SAM_PED_SVC_ID_V01;
  ped_messages->header[ SAM_ENABLE ].msg_id = SNS_SAM_PED_ENABLE_REQ_V01;
  ped_messages->header[ SAM_ENABLE ].msg_size = sizeof(sns_sam_ped_enable_req_msg_v01);
  ped_messages->header[ SAM_ENABLE ].txn_id = 1;
  /***********************/
  ped_messages->header[ SAM_GET_REPORT ].service_number = SNS_SAM_PED_SVC_ID_V01;
  ped_messages->header[ SAM_GET_REPORT ].msg_id = SNS_SAM_PED_GET_REPORT_REQ_V01;
  ped_messages->header[ SAM_GET_REPORT ].msg_size = sizeof(sns_sam_ped_get_report_req_msg_v01);
  ped_messages->header[ SAM_GET_REPORT ].txn_id = 2;
  /***********************/
  ped_messages->header[ SAM_DISABLE ].service_number = SNS_SAM_PED_SVC_ID_V01;
  ped_messages->header[ SAM_DISABLE ].msg_id = SNS_SAM_PED_DISABLE_REQ_V01;
  ped_messages->header[ SAM_DISABLE ].msg_size = sizeof(sns_sam_ped_disable_req_msg_v01);
  ped_messages->header[ SAM_DISABLE ].txn_id = 3;
  /***********************/
  ped_messages->header[ SAM_CANCEL ].service_number = SNS_SAM_PED_SVC_ID_V01;
  ped_messages->header[ SAM_CANCEL ].msg_id = SNS_SAM_PED_CANCEL_REQ_V01;
  ped_messages->header[ SAM_CANCEL ].msg_size = sizeof(sns_common_cancel_req_msg_v01);
  ped_messages->header[ SAM_CANCEL ].txn_id = 4;
  /***********************/
  ped_messages->header[ SAM_UPDATE ].service_number = SNS_SAM_PED_SVC_ID_V01;
  ped_messages->header[ SAM_UPDATE ].msg_id = SNS_SAM_PED_RESET_REQ_V01;
  ped_messages->header[ SAM_UPDATE ].msg_size = sizeof(sns_sam_ped_reset_req_msg_v01);
  ped_messages->header[ SAM_UPDATE ].txn_id = 5;
  /***********************/
  ped_messages->header[ SAM_BATCH ].service_number = SNS_SAM_PED_SVC_ID_V01;
  ped_messages->header[ SAM_BATCH ].msg_id = SNS_SAM_PED_BATCH_REQ_V01;
  ped_messages->header[ SAM_BATCH ].msg_size = sizeof(sns_sam_ped_batch_req_msg_v01);
  ped_messages->header[ SAM_BATCH ].txn_id = 6;
}

/*============================================================================
  AMD
  ===========================================================================*/

void handle_amd_resp( sensor1_msg_header_s *msg_hdr_ptr, void *msg_ptr )
{
  if( SNS_SAM_AMD_ENABLE_RESP_V01 == msg_hdr_ptr->msg_id ) {
    sns_sam_qmd_enable_resp_msg_v01 *resp_ptr = msg_ptr;
    printf( "Got ENABLE response, status = %d, id = %d\n",
            resp_ptr->resp.sns_result_t, resp_ptr->instance_id );
    algo_instance_id = resp_ptr->instance_id;
  } else if( SNS_SAM_AMD_GET_REPORT_RESP_V01 == msg_hdr_ptr->msg_id ) {
    sns_sam_qmd_get_report_resp_msg_v01 *resp_ptr = msg_ptr;
    printf( "Got GET_REPORT response, status = %d, id = %d, "
             "timestamp = %u, state = %d\n",
             resp_ptr->resp.sns_result_t, resp_ptr->instance_id,
             resp_ptr->timestamp, resp_ptr->state );
  } else if( SNS_SAM_AMD_DISABLE_RESP_V01 == msg_hdr_ptr->msg_id ) {
    sns_sam_qmd_disable_resp_msg_v01 *resp_ptr = msg_ptr;
    printf( "Got DISABLE response, status = %d, id = %d\n",
            resp_ptr->resp.sns_result_t, resp_ptr->instance_id );
  } else if( msg_hdr_ptr->msg_id == SNS_SAM_AMD_CANCEL_RESP_V01 ) {
    sns_common_cancel_resp_msg_v01 *resp_ptr = msg_ptr;
    printf( "Got CANCEL response, status = %d\n", resp_ptr->resp.sns_result_t );
  } else {
    printf( "sam_test: Got INVALID response!!!\n" );
  }
}

void handle_amd_ind( sensor1_msg_header_s *msg_hdr_ptr, void *msg_ptr )
{
  if ( msg_hdr_ptr->msg_id == SNS_SAM_AMD_REPORT_IND_V01 ) {
    sns_sam_qmd_report_ind_msg_v01 *ind_ptr = msg_ptr;
    rcv_ind_count++;
    printf( "Got REPORT indication, id = %d, timestamp = %u, state = %d\n",
            ind_ptr->instance_id, ind_ptr->timestamp, ind_ptr->state );
  } else {
    printf( "sam_test: Got INVALID indication!!!\n" );
  }
}

void amd_enable_req( sns_sam_qmd_enable_req_msg_v01 *sam_qmd_enable_req )
{
  memset( sam_qmd_enable_req, sizeof(sns_sam_qmd_enable_req_msg_v01), 0 );
  //sam_qmd_enable_req->report_period = (1<<16);  /* 1 second, Q16 format */
}

void amd_disable_req( sns_sam_qmd_disable_req_msg_v01 *sam_qmd_disable_req )
{
  sam_qmd_disable_req->instance_id = algo_instance_id;
}

void amd_get_report_req( sns_sam_qmd_get_report_req_msg_v01 *sam_qmd_get_report_req )
{
  sam_qmd_get_report_req->instance_id = algo_instance_id;
}

void create_amd_messages( sam_messages *amd_messages )
{
  amd_messages->request[ SAM_ENABLE ] = (msg_func)&amd_enable_req;
  amd_messages->request[ SAM_DISABLE ] = (msg_func)&amd_disable_req;
  amd_messages->request[ SAM_GET_REPORT ] = (msg_func)&amd_get_report_req;
  amd_messages->request[ SAM_CANCEL ] = (msg_func)&cancel_req;
  amd_messages->request[ SAM_BATCH ] = NULL;
  amd_messages->request[ SAM_UPDATE ] = NULL;

  /***********************/
  amd_messages->header[ SAM_ENABLE ].service_number = SNS_SAM_AMD_SVC_ID_V01;
  amd_messages->header[ SAM_ENABLE ].msg_id = SNS_SAM_AMD_ENABLE_REQ_V01;
  amd_messages->header[ SAM_ENABLE ].msg_size = sizeof(sns_sam_qmd_enable_req_msg_v01);
  amd_messages->header[ SAM_ENABLE ].txn_id = 1;
  /***********************/
  amd_messages->header[ SAM_GET_REPORT ].service_number = SNS_SAM_AMD_SVC_ID_V01;
  amd_messages->header[ SAM_GET_REPORT ].msg_id = SNS_SAM_PED_GET_REPORT_REQ_V01;
  amd_messages->header[ SAM_GET_REPORT ].msg_size = sizeof(sns_sam_qmd_get_report_req_msg_v01);
  amd_messages->header[ SAM_GET_REPORT ].txn_id = 2;
  /***********************/
  amd_messages->header[ SAM_DISABLE ].service_number = SNS_SAM_AMD_SVC_ID_V01;
  amd_messages->header[ SAM_DISABLE ].msg_id = SNS_SAM_AMD_DISABLE_REQ_V01;
  amd_messages->header[ SAM_DISABLE ].msg_size = sizeof(sns_sam_qmd_disable_req_msg_v01);
  amd_messages->header[ SAM_DISABLE ].txn_id = 3;
  /***********************/
  amd_messages->header[ SAM_CANCEL ].service_number = SNS_SAM_AMD_SVC_ID_V01;
  amd_messages->header[ SAM_CANCEL ].msg_id = SNS_SAM_AMD_CANCEL_REQ_V01;
  amd_messages->header[ SAM_CANCEL ].msg_size = sizeof(sns_common_cancel_req_msg_v01);
  amd_messages->header[ SAM_CANCEL ].txn_id = 4;
}

/*============================================================================
  Quaternion
  ===========================================================================*/

void handle_quat_resp( sensor1_msg_header_s *msg_hdr_ptr, void *msg_ptr )
{
  if( SNS_SAM_QUAT_ENABLE_RESP_V01 == msg_hdr_ptr->msg_id ) {
    sns_sam_quat_enable_resp_msg_v01 *resp_ptr = msg_ptr;
    printf( "Got ENABLE response, status = %d, id = %d\n",
            resp_ptr->resp.sns_result_t, resp_ptr->instance_id );
    algo_instance_id = resp_ptr->instance_id;
  } else if( SNS_SAM_QUAT_GET_REPORT_RESP_V01 == msg_hdr_ptr->msg_id ) {
    sns_sam_quat_get_report_resp_msg_v01 *resp_ptr = msg_ptr;
    printf( "Got GET_REPORT response, status = %d, id = %d, "
             "timestamp = %u, result = (%f, %f, %f, %f)\n",
             resp_ptr->resp.sns_result_t, resp_ptr->instance_id,
             resp_ptr->timestamp, resp_ptr->result.quaternion[0],
             resp_ptr->result.quaternion[1], resp_ptr->result.quaternion[2],
             resp_ptr->result.quaternion[3] );
  } else if( SNS_SAM_QUAT_DISABLE_RESP_V01 == msg_hdr_ptr->msg_id ) {
    sns_sam_quat_disable_resp_msg_v01 *resp_ptr = msg_ptr;
    printf( "Got DISABLE response, status = %d, id = %d\n",
            resp_ptr->resp.sns_result_t, resp_ptr->instance_id );
  } else if( msg_hdr_ptr->msg_id == SNS_SAM_QUAT_CANCEL_RESP_V01 ) {
    sns_common_cancel_resp_msg_v01 *resp_ptr = msg_ptr;
    printf( "Got CANCEL response, status = %d\n", resp_ptr->resp.sns_result_t );
  } else {
    printf( "sam_test: Got INVALID response!!!\n" );
  }
}

void handle_quat_ind( sensor1_msg_header_s *msg_hdr_ptr, void *msg_ptr )
{
  if ( msg_hdr_ptr->msg_id == SNS_SAM_QUAT_REPORT_IND_V01 ) {
    sns_sam_quat_report_ind_msg_v01 *ind_ptr = msg_ptr;
    rcv_ind_count++;
    printf( "Got REPORT indication, id = %d, timestamp = %u, result = (%f, %f, %f, %f)\n",
            ind_ptr->instance_id, ind_ptr->timestamp, ind_ptr->result.quaternion[0],
            ind_ptr->result.quaternion[1], ind_ptr->result.quaternion[2],
            ind_ptr->result.quaternion[3] );
  } else {
    printf( "sam_test: Got INVALID indication!!!\n" );
  }
}

void quat_enable_req( sns_sam_quat_enable_req_msg_v01 *sam_quat_enable_req )
{
  memset( sam_quat_enable_req, sizeof(sns_sam_quat_enable_req_msg_v01), 0 );
  sam_quat_enable_req->report_period = FX_FLTTOFIX_Q16(report_period);
  sam_quat_enable_req->sample_rate_valid = 1;
  sam_quat_enable_req->sample_rate = FX_FLTTOFIX_Q16(sample_rate);
}

void quat_disable_req( sns_sam_quat_disable_req_msg_v01 *sam_quat_disable_req )
{
  sam_quat_disable_req->instance_id = algo_instance_id;
}

void quat_get_report_req( sns_sam_quat_get_report_req_msg_v01 *sam_quat_get_report_req )
{
  sam_quat_get_report_req->instance_id = algo_instance_id;
}

void create_quat_messages( sam_messages *quat_messages )
{
  quat_messages->request[ SAM_ENABLE ] = (msg_func)&quat_enable_req;
  quat_messages->request[ SAM_DISABLE ] = (msg_func)&quat_disable_req;
  quat_messages->request[ SAM_GET_REPORT ] = (msg_func)&quat_get_report_req;
  quat_messages->request[ SAM_CANCEL ] = (msg_func)&cancel_req;
  quat_messages->request[ SAM_BATCH ] = NULL;
  quat_messages->request[ SAM_UPDATE ] = NULL;

  /***********************/
  quat_messages->header[ SAM_ENABLE ].service_number = SNS_SAM_QUATERNION_SVC_ID_V01;
  quat_messages->header[ SAM_ENABLE ].msg_id = SNS_SAM_QUAT_ENABLE_REQ_V01;
  quat_messages->header[ SAM_ENABLE ].msg_size = sizeof(sns_sam_quat_enable_req_msg_v01);
  quat_messages->header[ SAM_ENABLE ].txn_id = 1;
  /***********************/
  quat_messages->header[ SAM_GET_REPORT ].service_number = SNS_SAM_QUATERNION_SVC_ID_V01;
  quat_messages->header[ SAM_GET_REPORT ].msg_id = SNS_SAM_QUAT_GET_REPORT_REQ_V01;
  quat_messages->header[ SAM_GET_REPORT ].msg_size = sizeof(sns_sam_quat_get_report_req_msg_v01);
  quat_messages->header[ SAM_GET_REPORT ].txn_id = 2;
  /***********************/
  quat_messages->header[ SAM_DISABLE ].service_number = SNS_SAM_QUATERNION_SVC_ID_V01;
  quat_messages->header[ SAM_DISABLE ].msg_id = SNS_SAM_QUAT_DISABLE_REQ_V01;
  quat_messages->header[ SAM_DISABLE ].msg_size = sizeof(sns_sam_quat_disable_req_msg_v01);
  quat_messages->header[ SAM_DISABLE ].txn_id = 3;
  /***********************/
  quat_messages->header[ SAM_CANCEL ].service_number = SNS_SAM_QUATERNION_SVC_ID_V01;
  quat_messages->header[ SAM_CANCEL ].msg_id = SNS_SAM_QUAT_CANCEL_REQ_V01;
  quat_messages->header[ SAM_CANCEL ].msg_size = sizeof(sns_common_cancel_req_msg_v01);
  quat_messages->header[ SAM_CANCEL ].txn_id = 4;
}

/*============================================================================
  FMV
  ===========================================================================*/

void handle_fmv_resp( sensor1_msg_header_s *msg_hdr_ptr, void *msg_ptr )
{
  if( SNS_SAM_FILTERED_MAG_ENABLE_RESP_V01 == msg_hdr_ptr->msg_id ) {
    sns_sam_filtered_mag_enable_resp_msg_v01 *resp_ptr = msg_ptr;
    printf( "Got ENABLE response, status = %d, id = %d\n",
            resp_ptr->resp.sns_result_t, resp_ptr->instance_id );
    algo_instance_id = resp_ptr->instance_id;
  } else if( SNS_SAM_FILTERED_MAG_GET_REPORT_RESP_V01 == msg_hdr_ptr->msg_id ) {
    sns_sam_filtered_mag_get_report_resp_msg_v01 *resp_ptr = msg_ptr;
    printf( "Got GET_REPORT response, status = %d, id = %d, "
             "timestamp = %u, result = (%f, %f, %f)\n",
             resp_ptr->resp.sns_result_t, resp_ptr->instance_id,
             resp_ptr->timestamp, resp_ptr->result.filtered_mag[0],
             resp_ptr->result.filtered_mag[1], resp_ptr->result.filtered_mag[2] );
  } else if( SNS_SAM_FILTERED_MAG_DISABLE_RESP_V01 == msg_hdr_ptr->msg_id ) {
    sns_sam_filtered_mag_disable_resp_msg_v01 *resp_ptr = msg_ptr;
    printf( "Got DISABLE response, status = %d, id = %d\n",
            resp_ptr->resp.sns_result_t, resp_ptr->instance_id );
  } else if( msg_hdr_ptr->msg_id == SNS_SAM_FILTERED_MAG_CANCEL_RESP_V01 ) {
    sns_common_cancel_resp_msg_v01 *resp_ptr = msg_ptr;
    printf( "Got CANCEL response, status = %d\n", resp_ptr->resp.sns_result_t );
  } else {
    printf( "sam_test: Got INVALID response!!!\n" );
  }
}

void handle_fmv_ind( sensor1_msg_header_s *msg_hdr_ptr, void *msg_ptr )
{
  if ( msg_hdr_ptr->msg_id == SNS_SAM_FILTERED_MAG_REPORT_IND_V01 ) {
    sns_sam_filtered_mag_report_ind_msg_v01 *ind_ptr = msg_ptr;
    rcv_ind_count++;
    printf( "Got REPORT indication, id = %d, timestamp = %u, result = (%f, %f, %f)\n",
            ind_ptr->instance_id, ind_ptr->timestamp, ind_ptr->result.filtered_mag[0],
            ind_ptr->result.filtered_mag[1], ind_ptr->result.filtered_mag[2] );
  } else {
    printf( "sam_test: Got INVALID indication!!!\n" );
  }
}

void fmv_enable_req( sns_sam_filtered_mag_enable_req_msg_v01 *sam_fmv_enable_req )
{
  memset( sam_fmv_enable_req, sizeof(sns_sam_filtered_mag_enable_req_msg_v01), 0 );
  sam_fmv_enable_req->report_period = FX_FLTTOFIX_Q16(report_period);
  sam_fmv_enable_req->sample_rate_valid = 1;
  sam_fmv_enable_req->sample_rate = FX_FLTTOFIX_Q16(sample_rate);
}

void fmv_disable_req( sns_sam_filtered_mag_disable_req_msg_v01 *sam_fmv_disable_req )
{
  sam_fmv_disable_req->instance_id = algo_instance_id;
}

void fmv_get_report_req( sns_sam_filtered_mag_get_report_req_msg_v01 *sam_fmv_get_report_req )
{
  sam_fmv_get_report_req->instance_id = algo_instance_id;
}

void create_fmv_messages( sam_messages *fmv_messages )
{
  fmv_messages->request[ SAM_ENABLE ] = (msg_func)&fmv_enable_req;
  fmv_messages->request[ SAM_DISABLE ] = (msg_func)&fmv_disable_req;
  fmv_messages->request[ SAM_GET_REPORT ] = (msg_func)&fmv_get_report_req;
  fmv_messages->request[ SAM_CANCEL ] = (msg_func)&cancel_req;
  fmv_messages->request[ SAM_BATCH ] = NULL;
  fmv_messages->request[ SAM_UPDATE ] = NULL;

  /***********************/
  fmv_messages->header[ SAM_ENABLE ].service_number = SNS_SAM_FILTERED_MAG_SVC_ID_V01;
  fmv_messages->header[ SAM_ENABLE ].msg_id = SNS_SAM_FILTERED_MAG_ENABLE_REQ_V01;
  fmv_messages->header[ SAM_ENABLE ].msg_size = sizeof(sns_sam_filtered_mag_enable_req_msg_v01);
  fmv_messages->header[ SAM_ENABLE ].txn_id = 1;
  /***********************/
  fmv_messages->header[ SAM_GET_REPORT ].service_number = SNS_SAM_FILTERED_MAG_SVC_ID_V01;
  fmv_messages->header[ SAM_GET_REPORT ].msg_id = SNS_SAM_FILTERED_MAG_GET_REPORT_REQ_V01;
  fmv_messages->header[ SAM_GET_REPORT ].msg_size = sizeof(sns_sam_filtered_mag_get_report_req_msg_v01);
  fmv_messages->header[ SAM_GET_REPORT ].txn_id = 2;
  /***********************/
  fmv_messages->header[ SAM_DISABLE ].service_number = SNS_SAM_FILTERED_MAG_SVC_ID_V01;
  fmv_messages->header[ SAM_DISABLE ].msg_id = SNS_SAM_FILTERED_MAG_DISABLE_REQ_V01;
  fmv_messages->header[ SAM_DISABLE ].msg_size = sizeof(sns_sam_filtered_mag_disable_req_msg_v01);
  fmv_messages->header[ SAM_DISABLE ].txn_id = 3;
  /***********************/
  fmv_messages->header[ SAM_CANCEL ].service_number = SNS_SAM_FILTERED_MAG_SVC_ID_V01;
  fmv_messages->header[ SAM_CANCEL ].msg_id = SNS_SAM_FILTERED_MAG_CANCEL_REQ_V01;
  fmv_messages->header[ SAM_CANCEL ].msg_size = sizeof(sns_common_cancel_req_msg_v01);
  fmv_messages->header[ SAM_CANCEL ].txn_id = 4;
}

/*============================================================================
  Gravity Vector
  ===========================================================================*/

void handle_grav_resp( sensor1_msg_header_s *msg_hdr_ptr, void *msg_ptr )
{
  if( SNS_SAM_GRAVITY_ENABLE_RESP_V01 == msg_hdr_ptr->msg_id ) {
    sns_sam_gravity_enable_resp_msg_v01 *resp_ptr = msg_ptr;
    printf( "Got ENABLE response, status = %d, id = %d\n",
            resp_ptr->resp.sns_result_t, resp_ptr->instance_id );
    algo_instance_id = resp_ptr->instance_id;
  } else if( SNS_SAM_GRAVITY_GET_REPORT_RESP_V01 == msg_hdr_ptr->msg_id ) {
    sns_sam_gravity_get_report_resp_msg_v01 *resp_ptr = msg_ptr;
    printf( "Got GET_REPORT response, status = %d, id = %d, "
            "timestamp = %u, state = %d\n",
            resp_ptr->resp.sns_result_t, resp_ptr->instance_id,
            resp_ptr->timestamp, resp_ptr->device_motion_state );
  } else if( SNS_SAM_GRAVITY_DISABLE_RESP_V01 == msg_hdr_ptr->msg_id ) {
    sns_sam_gravity_disable_resp_msg_v01 *resp_ptr = msg_ptr;
    printf( "Got DISABLE response, status = %d, id = %d\n",
            resp_ptr->resp.sns_result_t, resp_ptr->instance_id );
  } else if( SNS_SAM_GRAVITY_CANCEL_RESP_V01 == msg_hdr_ptr->msg_id ) {
    sns_common_cancel_resp_msg_v01 *resp_ptr = msg_ptr;
    printf( "Got CANCEL response, status = %d\n", resp_ptr->resp.sns_result_t );
  } else if( SNS_SAM_GRAVITY_BATCH_RESP_V01 == msg_hdr_ptr->msg_id ) {
    sns_sam_gravity_batch_resp_msg_v01 *resp_ptr = msg_ptr;
    printf( "Got BATCH response, status = %d\n", resp_ptr->resp.sns_result_t );
  } else {
    printf( "sam_test: Got INVALID response!!!\n" );
  }
}

void handle_grav_ind( sensor1_msg_header_s *msg_hdr_ptr, void *msg_ptr )
{
  if ( SNS_SAM_GRAVITY_REPORT_IND_V01 == msg_hdr_ptr->msg_id ) {
    sns_sam_gravity_report_ind_msg_v01 *ind_ptr = msg_ptr;
    rcv_ind_count++;
    printf( "Got REPORT indication, id = %d, timestamp = %u, state = %d (%f, %f, %f)\n",
            ind_ptr->instance_id, ind_ptr->timestamp, ind_ptr->device_motion_state,
            ind_ptr->result.gravity[0], ind_ptr->result.gravity[1], ind_ptr->result.gravity[2]);
  } else if ( SNS_SAM_GRAVITY_BATCH_IND_V01 == msg_hdr_ptr->msg_id ) {
    sns_sam_gravity_batch_ind_msg_v01 *ind_ptr = msg_ptr;
    uint32_t i;

    for( i = 0; i < ind_ptr->reports_len; i++ ) {
      rcv_ind_count++;
    printf( "Got REPORT indication, id = %d, timestamp = %u, state = %d (%f, %f, %f)\n",
            ind_ptr->instance_id, ind_ptr->first_report_timestamp, ind_ptr->reports[i].device_motion_state,
            ind_ptr->reports[i].result.gravity[0], ind_ptr->reports[i].result.gravity[1],
            ind_ptr->reports[i].result.gravity[2]);
    }
  } else {
    printf( "sam_test: Got INVALID indication!!!\n" );
  }
}

void grav_enable_req( sns_sam_gravity_enable_req_msg_v01 *sam_grav_enable_req )
{
  memset( sam_grav_enable_req, sizeof(sns_sam_gravity_enable_req_msg_v01), 0 );
  sam_grav_enable_req->report_period = FX_FLTTOFIX_Q16(report_period);
  sam_grav_enable_req->sample_rate_valid = 1;
  sam_grav_enable_req->sample_rate = FX_FLTTOFIX_Q16(sample_rate);
}

void grav_disable_req( sns_sam_gravity_disable_req_msg_v01 *sam_grav_disable_req )
{
  sam_grav_disable_req->instance_id = algo_instance_id;
}

void grav_get_report_req( sns_sam_gravity_get_report_req_msg_v01 *sam_grav_get_report_req )
{
  sam_grav_get_report_req->instance_id = algo_instance_id;
}

void grav_batch_req( sns_sam_gravity_batch_req_msg_v01 *sam_grav_batch_req )
{
  sam_grav_batch_req->instance_id = algo_instance_id;
  sam_grav_batch_req->batch_period = FX_FLTTOFIX_Q16(3);
}

void create_grav_messages( sam_messages *grav_messages )
{
  grav_messages->request[ SAM_ENABLE ] = (msg_func)&grav_enable_req;
  grav_messages->request[ SAM_DISABLE ] = (msg_func)&grav_disable_req;
  grav_messages->request[ SAM_GET_REPORT ] = (msg_func)&grav_get_report_req;
  grav_messages->request[ SAM_CANCEL ] = (msg_func)&cancel_req;
  grav_messages->request[ SAM_BATCH ] = (msg_func)&grav_batch_req;
  grav_messages->request[ SAM_UPDATE ] = NULL;

  /***********************/
  grav_messages->header[ SAM_ENABLE ].service_number = SNS_SAM_GRAVITY_VECTOR_SVC_ID_V01;
  grav_messages->header[ SAM_ENABLE ].msg_id = SNS_SAM_GRAVITY_ENABLE_REQ_V01;
  grav_messages->header[ SAM_ENABLE ].msg_size = sizeof(sns_sam_gravity_enable_req_msg_v01);
  grav_messages->header[ SAM_ENABLE ].txn_id = 1;
  /***********************/
  grav_messages->header[ SAM_GET_REPORT ].service_number = SNS_SAM_GRAVITY_VECTOR_SVC_ID_V01;
  grav_messages->header[ SAM_GET_REPORT ].msg_id = SNS_SAM_GRAVITY_GET_REPORT_REQ_V01;
  grav_messages->header[ SAM_GET_REPORT ].msg_size = sizeof(sns_sam_gravity_get_report_req_msg_v01);
  grav_messages->header[ SAM_GET_REPORT ].txn_id = 2;
  /***********************/
  grav_messages->header[ SAM_DISABLE ].service_number = SNS_SAM_GRAVITY_VECTOR_SVC_ID_V01;
  grav_messages->header[ SAM_DISABLE ].msg_id = SNS_SAM_GRAVITY_DISABLE_REQ_V01;
  grav_messages->header[ SAM_DISABLE ].msg_size = sizeof(sns_sam_gravity_disable_req_msg_v01);
  grav_messages->header[ SAM_DISABLE ].txn_id = 3;
  /***********************/
  grav_messages->header[ SAM_CANCEL ].service_number = SNS_SAM_GRAVITY_VECTOR_SVC_ID_V01;
  grav_messages->header[ SAM_CANCEL ].msg_id = SNS_SAM_GRAVITY_CANCEL_REQ_V01;
  grav_messages->header[ SAM_CANCEL ].msg_size = sizeof(sns_common_cancel_req_msg_v01);
  grav_messages->header[ SAM_CANCEL ].txn_id = 4;
  /***********************/
  grav_messages->header[ SAM_BATCH ].service_number = SNS_SAM_GRAVITY_VECTOR_SVC_ID_V01;
  grav_messages->header[ SAM_BATCH ].msg_id = SNS_SAM_GRAVITY_BATCH_REQ_V01;
  grav_messages->header[ SAM_BATCH ].msg_size = sizeof(sns_sam_gravity_batch_req_msg_v01);
  grav_messages->header[ SAM_BATCH ].txn_id = 6;
}

/*============================================================================
  SMD
  ===========================================================================*/

void handle_smd_resp( sensor1_msg_header_s *msg_hdr_ptr, void *msg_ptr )
{
  if( SNS_SAM_SMD_ENABLE_RESP_V01 == msg_hdr_ptr->msg_id ) {
    sns_sam_smd_enable_resp_msg_v01 *resp_ptr = msg_ptr;
    printf( "Got ENABLE response, status = %d, id = %d\n",
            resp_ptr->resp.sns_result_t, resp_ptr->instance_id );
    algo_instance_id = resp_ptr->instance_id;
  } else if( SNS_SAM_SMD_GET_REPORT_RESP_V01 == msg_hdr_ptr->msg_id ) {
    sns_sam_smd_get_report_resp_msg_v01 *resp_ptr = msg_ptr;
    printf( "Got GET_REPORT response, status = %d, id = %d, "
             "timestamp = %u\n",
             resp_ptr->resp.sns_result_t, resp_ptr->instance_id,
             resp_ptr->timestamp);
  } else if( SNS_SAM_SMD_DISABLE_RESP_V01 == msg_hdr_ptr->msg_id ) {
    sns_sam_smd_disable_resp_msg_v01 *resp_ptr = msg_ptr;
    printf( "Got DISABLE response, status = %d, id = %d\n",
            resp_ptr->resp.sns_result_t, resp_ptr->instance_id );
  } else if( msg_hdr_ptr->msg_id == SNS_SAM_SMD_CANCEL_RESP_V01 ) {
    sns_common_cancel_resp_msg_v01 *resp_ptr = msg_ptr;
    printf( "Got CANCEL response, status = %d\n", resp_ptr->resp.sns_result_t );
  } else {
    printf( "sam_test: Got INVALID response!!!\n" );
  }
}

void handle_smd_ind( sensor1_msg_header_s *msg_hdr_ptr, void *msg_ptr )
{
  if ( msg_hdr_ptr->msg_id == SNS_SAM_SMD_REPORT_IND_V01 ) {
    sns_sam_smd_report_ind_msg_v01 *ind_ptr = msg_ptr;
    rcv_ind_count++;
    printf( "Got REPORT indication, id = %d, timestamp = %u, state = %d\n",
            ind_ptr->instance_id, ind_ptr->timestamp, ind_ptr->report_data.motion_state );
  } else {
    printf( "sam_test: Got INVALID indication!!!\n" );
  }
}

void smd_enable_req( sns_sam_smd_enable_req_msg_v01 *sam_smd_enable_req )
{
  memset( sam_smd_enable_req, sizeof(sns_sam_smd_enable_req_msg_v01), 0 );
  //sam_smd_enable_req->report_period = (1<<16);  /* 1 second, Q16 format */
}

void smd_disable_req( sns_sam_smd_disable_req_msg_v01 *sam_smd_disable_req )
{
  sam_smd_disable_req->instance_id = algo_instance_id;
}

void smd_get_report_req( sns_sam_smd_get_report_req_msg_v01 *sam_smd_get_report_req )
{
  sam_smd_get_report_req->instance_id = algo_instance_id;
}

void create_smd_messages( sam_messages *smd_messages )
{
  smd_messages->request[ SAM_ENABLE ] = (msg_func)&smd_enable_req;
  smd_messages->request[ SAM_DISABLE ] = (msg_func)&smd_disable_req;
  smd_messages->request[ SAM_GET_REPORT ] = (msg_func)&smd_get_report_req;
  smd_messages->request[ SAM_CANCEL ] = (msg_func)&cancel_req;
  smd_messages->request[ SAM_BATCH ] = NULL;
  smd_messages->request[ SAM_UPDATE ] = NULL;

  /***********************/
  smd_messages->header[ SAM_ENABLE ].service_number = SNS_SAM_SMD_SVC_ID_V01;
  smd_messages->header[ SAM_ENABLE ].msg_id = SNS_SAM_SMD_ENABLE_REQ_V01;
  smd_messages->header[ SAM_ENABLE ].msg_size = sizeof(sns_sam_smd_enable_req_msg_v01);
  smd_messages->header[ SAM_ENABLE ].txn_id = 1;
  /***********************/
  smd_messages->header[ SAM_GET_REPORT ].service_number = SNS_SAM_SMD_SVC_ID_V01;
  smd_messages->header[ SAM_GET_REPORT ].msg_id = SNS_SAM_PED_GET_REPORT_REQ_V01;
  smd_messages->header[ SAM_GET_REPORT ].msg_size = sizeof(sns_sam_smd_get_report_req_msg_v01);
  smd_messages->header[ SAM_GET_REPORT ].txn_id = 2;
  /***********************/
  smd_messages->header[ SAM_DISABLE ].service_number = SNS_SAM_SMD_SVC_ID_V01;
  smd_messages->header[ SAM_DISABLE ].msg_id = SNS_SAM_SMD_DISABLE_REQ_V01;
  smd_messages->header[ SAM_DISABLE ].msg_size = sizeof(sns_sam_smd_disable_req_msg_v01);
  smd_messages->header[ SAM_DISABLE ].txn_id = 3;
  /***********************/
  smd_messages->header[ SAM_CANCEL ].service_number = SNS_SAM_SMD_SVC_ID_V01;
  smd_messages->header[ SAM_CANCEL ].msg_id = SNS_SAM_SMD_CANCEL_REQ_V01;
  smd_messages->header[ SAM_CANCEL ].msg_size = sizeof(sns_common_cancel_req_msg_v01);
  smd_messages->header[ SAM_CANCEL ].txn_id = 4;
}

/*============================================================================
  Rotation Vector
  ===========================================================================*/

void handle_rotv_resp( sensor1_msg_header_s *msg_hdr_ptr, void *msg_ptr )
{
  if( SNS_SAM_ROTATION_VECTOR_ENABLE_RESP_V01 == msg_hdr_ptr->msg_id ) {
    sns_sam_rotation_vector_enable_resp_msg_v01 *resp_ptr = msg_ptr;
    printf( "Got ENABLE response, status = %d, id = %d\n",
            resp_ptr->resp.sns_result_t, resp_ptr->instance_id );
    algo_instance_id = resp_ptr->instance_id;
  } else if( SNS_SAM_ROTATION_VECTOR_GET_REPORT_RESP_V01 == msg_hdr_ptr->msg_id ) {
    sns_sam_rotation_vector_get_report_resp_msg_v01 *resp_ptr = msg_ptr;
    printf( "Got GET_REPORT response, status = %d, id = %d, "
            "timestamp = %u, state = (%f, %f, %f)\n",
            resp_ptr->resp.sns_result_t, resp_ptr->instance_id,
            resp_ptr->timestamp, resp_ptr->result.rotation_vector[0],
            resp_ptr->result.rotation_vector[1], resp_ptr->result.rotation_vector[2] );
  } else if( SNS_SAM_ROTATION_VECTOR_DISABLE_RESP_V01 == msg_hdr_ptr->msg_id ) {
    sns_sam_rotation_vector_disable_resp_msg_v01 *resp_ptr = msg_ptr;
    printf( "Got DISABLE response, status = %d, id = %d\n",
            resp_ptr->resp.sns_result_t, resp_ptr->instance_id );
  } else if( SNS_SAM_ROTATION_VECTOR_CANCEL_RESP_V01 == msg_hdr_ptr->msg_id ) {
    sns_common_cancel_resp_msg_v01 *resp_ptr = msg_ptr;
    printf( "Got CANCEL response, status = %d\n", resp_ptr->resp.sns_result_t );
  } else if( SNS_SAM_ROTATION_VECTOR_BATCH_RESP_V01 == msg_hdr_ptr->msg_id ) {
    sns_sam_rotation_vector_batch_resp_msg_v01 *resp_ptr = msg_ptr;
    printf( "Got BATCH response, status = %d\n", resp_ptr->resp.sns_result_t );
  } else {
    printf( "sam_test: Got INVALID response!!!\n" );
  }
}

void handle_rotv_ind( sensor1_msg_header_s *msg_hdr_ptr, void *msg_ptr )
{
  if ( SNS_SAM_ROTATION_VECTOR_REPORT_IND_V01 == msg_hdr_ptr->msg_id ) {
    sns_sam_rotation_vector_report_ind_msg_v01 *ind_ptr = msg_ptr;
    rcv_ind_count++;
    printf( "Got REPORT indication, id = %d, timestamp = %u, state = %d (%f, %f, %f)\n",
            ind_ptr->instance_id, ind_ptr->timestamp, ind_ptr->result.accuracy,
            ind_ptr->result.rotation_vector[0], ind_ptr->result.rotation_vector[1], ind_ptr->result.rotation_vector[2]);
  } else if ( SNS_SAM_ROTATION_VECTOR_BATCH_IND_V01 == msg_hdr_ptr->msg_id ) {
    sns_sam_rotation_vector_batch_ind_msg_v01 *ind_ptr = msg_ptr;
    uint32_t i;

    for( i = 0; i < ind_ptr->reports_len; i++ ) {
      rcv_ind_count++;
    printf( "Got REPORT indication, id = %d, timestamp = %u, state = %d (%f, %f, %f)\n",
            ind_ptr->instance_id, ind_ptr->first_report_timestamp, ind_ptr->reports[i].accuracy,
            ind_ptr->reports[i].rotation_vector[0], ind_ptr->reports[i].rotation_vector[1],
            ind_ptr->reports[i].rotation_vector[2]);
    }
  } else {
    printf( "sam_test: Got INVALID indication!!!\n" );
  }
}

void rotv_enable_req( sns_sam_rotation_vector_enable_req_msg_v01 *sam_rotv_enable_req )
{
  memset( sam_rotv_enable_req, sizeof(sns_sam_rotation_vector_enable_req_msg_v01), 0 );
  sam_rotv_enable_req->report_period = FX_FLTTOFIX_Q16(report_period);
  sam_rotv_enable_req->sample_rate_valid = 1;
  sam_rotv_enable_req->sample_rate = FX_FLTTOFIX_Q16(sample_rate);
}

void rotv_disable_req( sns_sam_rotation_vector_disable_req_msg_v01 *sam_rotv_disable_req )
{
  sam_rotv_disable_req->instance_id = algo_instance_id;
}

void rotv_get_report_req( sns_sam_rotation_vector_get_report_req_msg_v01 *sam_rotv_get_report_req )
{
  sam_rotv_get_report_req->instance_id = algo_instance_id;
}

void rotv_batch_req( sns_sam_rotation_vector_batch_req_msg_v01 *sam_rotv_batch_req )
{
  sam_rotv_batch_req->instance_id = algo_instance_id;
  sam_rotv_batch_req->batch_period = FX_FLTTOFIX_Q16(3);
}

void create_rotv_messages( sam_messages *rotv_messages )
{
  rotv_messages->request[ SAM_ENABLE ] = (msg_func)&rotv_enable_req;
  rotv_messages->request[ SAM_DISABLE ] = (msg_func)&rotv_disable_req;
  rotv_messages->request[ SAM_GET_REPORT ] = (msg_func)&rotv_get_report_req;
  rotv_messages->request[ SAM_CANCEL ] = (msg_func)&cancel_req;
  rotv_messages->request[ SAM_BATCH ] = (msg_func)&rotv_batch_req;
  rotv_messages->request[ SAM_UPDATE ] = NULL;

  /***********************/
  rotv_messages->header[ SAM_ENABLE ].service_number = SNS_SAM_ROTATION_VECTOR_SVC_ID_V01;
  rotv_messages->header[ SAM_ENABLE ].msg_id = SNS_SAM_ROTATION_VECTOR_ENABLE_REQ_V01;
  rotv_messages->header[ SAM_ENABLE ].msg_size = sizeof(sns_sam_rotation_vector_enable_req_msg_v01);
  rotv_messages->header[ SAM_ENABLE ].txn_id = 1;
  /***********************/
  rotv_messages->header[ SAM_GET_REPORT ].service_number = SNS_SAM_ROTATION_VECTOR_SVC_ID_V01;
  rotv_messages->header[ SAM_GET_REPORT ].msg_id = SNS_SAM_ROTATION_VECTOR_GET_REPORT_REQ_V01;
  rotv_messages->header[ SAM_GET_REPORT ].msg_size = sizeof(sns_sam_rotation_vector_get_report_req_msg_v01);
  rotv_messages->header[ SAM_GET_REPORT ].txn_id = 2;
  /***********************/
  rotv_messages->header[ SAM_DISABLE ].service_number = SNS_SAM_ROTATION_VECTOR_SVC_ID_V01;
  rotv_messages->header[ SAM_DISABLE ].msg_id = SNS_SAM_ROTATION_VECTOR_DISABLE_REQ_V01;
  rotv_messages->header[ SAM_DISABLE ].msg_size = sizeof(sns_sam_rotation_vector_disable_req_msg_v01);
  rotv_messages->header[ SAM_DISABLE ].txn_id = 3;
  /***********************/
  rotv_messages->header[ SAM_CANCEL ].service_number = SNS_SAM_ROTATION_VECTOR_SVC_ID_V01;
  rotv_messages->header[ SAM_CANCEL ].msg_id = SNS_SAM_ROTATION_VECTOR_CANCEL_REQ_V01;
  rotv_messages->header[ SAM_CANCEL ].msg_size = sizeof(sns_common_cancel_req_msg_v01);
  rotv_messages->header[ SAM_CANCEL ].txn_id = 4;
  /***********************/
  rotv_messages->header[ SAM_BATCH ].service_number = SNS_SAM_ROTATION_VECTOR_SVC_ID_V01;
  rotv_messages->header[ SAM_BATCH ].msg_id = SNS_SAM_ROTATION_VECTOR_BATCH_REQ_V01;
  rotv_messages->header[ SAM_BATCH ].msg_size = sizeof(sns_sam_rotation_vector_batch_req_msg_v01);
  rotv_messages->header[ SAM_BATCH ].txn_id = 6;
}

/*============================================================================
  Common
  ===========================================================================*/
void notify_data_cb( intptr_t data,
                     sensor1_msg_header_s *msg_hdr_ptr,
                     sensor1_msg_type_e msg_type,
                     void *msg_ptr )
{
  if( NULL == msg_hdr_ptr ) {
    printf("sam_test: received NULL msg_hdr_ptr!\n");
  } else {
    /*
    printf("sam_test: hdr.service_number: %u\n\thdr.msg_id: %d\n\t"
           "hdr.msg_type: %d\n\thdr.msg_size: %d\n\thdr.txn_id: %d\n",
           msg_hdr_ptr->service_number,
           msg_hdr_ptr->msg_id,
           msg_type,
           msg_hdr_ptr->msg_size,
           msg_hdr_ptr->txn_id );
    */
  }

  if( NULL == msg_ptr ) {
    printf("sam_test: received NULL msg_body!\n");
  }

  if( msg_type == SENSOR1_MSG_TYPE_RESP ) {
    // printf("sam_test: received RESP\n");
    pthread_mutex_lock( &my_mutex );
    my_predicate = 1;
    pthread_cond_signal( &my_cond );
    pthread_mutex_unlock( &my_mutex );

    if( SNS_SAM_PED_SVC_ID_V01 == msg_hdr_ptr->service_number )
      handle_ped_resp( msg_hdr_ptr, msg_ptr );
    if( SNS_SAM_AMD_SVC_ID_V01 == msg_hdr_ptr->service_number )
      handle_amd_resp( msg_hdr_ptr, msg_ptr );
    if( SNS_SAM_QUATERNION_SVC_ID_V01 == msg_hdr_ptr->service_number )
      handle_quat_resp( msg_hdr_ptr, msg_ptr );
    if( SNS_SAM_FILTERED_MAG_SVC_ID_V01 == msg_hdr_ptr->service_number )
      handle_fmv_resp( msg_hdr_ptr, msg_ptr );
    if( SNS_SAM_GRAVITY_VECTOR_SVC_ID_V01 == msg_hdr_ptr->service_number )
      handle_grav_resp( msg_hdr_ptr, msg_ptr );
    if( SNS_SAM_SMD_SVC_ID_V01 == msg_hdr_ptr->service_number )
      handle_smd_resp( msg_hdr_ptr, msg_ptr );
    if( SNS_SAM_ROTATION_VECTOR_SVC_ID_V01 == msg_hdr_ptr->service_number )
      handle_rotv_resp( msg_hdr_ptr, msg_ptr );
  } else if( msg_type == SENSOR1_MSG_TYPE_IND ) {
    // printf("sam_test: received IND\n");
    if( SNS_SAM_PED_SVC_ID_V01 == msg_hdr_ptr->service_number )
      handle_ped_ind( msg_hdr_ptr, msg_ptr );
    if( SNS_SAM_AMD_SVC_ID_V01 == msg_hdr_ptr->service_number )
      handle_amd_ind( msg_hdr_ptr, msg_ptr );
    if( SNS_SAM_QUATERNION_SVC_ID_V01 == msg_hdr_ptr->service_number )
      handle_quat_ind( msg_hdr_ptr, msg_ptr );
    if( SNS_SAM_FILTERED_MAG_SVC_ID_V01 == msg_hdr_ptr->service_number )
      handle_fmv_ind( msg_hdr_ptr, msg_ptr );
    if( SNS_SAM_GRAVITY_VECTOR_SVC_ID_V01 == msg_hdr_ptr->service_number )
      handle_grav_ind( msg_hdr_ptr, msg_ptr );
    if( SNS_SAM_SMD_SVC_ID_V01 == msg_hdr_ptr->service_number )
      handle_smd_ind( msg_hdr_ptr, msg_ptr );
    if( SNS_SAM_ROTATION_VECTOR_SVC_ID_V01 == msg_hdr_ptr->service_number )
      handle_rotv_ind( msg_hdr_ptr, msg_ptr );
  } else if( SENSOR1_MSG_TYPE_BROKEN_PIPE == msg_type  ) {
    printf( "sam_test: Got broken pipe!\n" );
  } else {
    printf( "sam_test: received INVALID MSG type!!!\n" );
  }
  if( NULL != msg_ptr ) {
    sensor1_free_msg_buf( *((sensor1_handle_s**)data), msg_ptr );
  }
}

void test_init()
{
  sensor1_error_e err = SENSOR1_SUCCESS;

  sigemptyset( &set );
  sigaddset( &set, SIGALRM );
  sigprocmask( SIG_SETMASK, &set, NULL );

  err = sensor1_init();
  if( SENSOR1_SUCCESS != err )
  {
    printf( "sam_test: sensor1_init returned %d\n", err );
    exit(err);
  }

  pthread_mutex_init( &my_mutex, NULL );
  pthread_cond_init( &my_cond, NULL );
}

void test_open()
{
  sensor1_error_e err = SENSOR1_SUCCESS;

  err = sensor1_open( &my_handle,
                        notify_data_cb,
                        (intptr_t)(&my_handle) );
  if( SENSOR1_SUCCESS != err )
  {
    printf( "sam_test: sensor1_open returned %d\n", err );
    exit(err);
  }
}

void test_close()
{
  sensor1_error_e err = SENSOR1_SUCCESS;

  err = sensor1_close( my_handle );
  if( SENSOR1_SUCCESS != err )
  {
    printf( "sam_test: sensor1_close returned %d\n", err );
    exit(err);
  }
}

void send_req( sam_messages *message, sam_message_type message_type )
{
  sensor1_error_e err;
  void *tempMsg;

  if( NULL == message->request[ message_type ] )
  {
    printf( "send_req: Message %d not supported\n", message_type );
    return;
  }

  err = sensor1_alloc_msg_buf( my_handle,
                               message->header[ message_type ].msg_size,
                               (void**)&tempMsg );
  if( SENSOR1_SUCCESS != err )
  {
    printf( "sam_test: sensor1_alloc_msg_buf returned %d\n", err );
    exit(err);
  }
  message->request[ message_type ]( tempMsg );

  err = sensor1_write( my_handle, &message->header[ message_type ], tempMsg );
  if( SENSOR1_SUCCESS != err )
  {
    printf( "send_req: sensor1_write returned %d\n", err );
    exit(err);
  }

  // wait for response
  pthread_mutex_lock( &my_mutex );
  while( my_predicate != 1 )
  {
    err = pthread_cond_wait( &my_cond, &my_mutex );
  }
  my_predicate = 0;
  pthread_mutex_unlock( &my_mutex );
}

int main( int argc, char * const argv[])
{
  int algoIdx = 0, indCount = 0;
  int opt;
  sam_messages messages[ ALGO_CNT ];

  while( (opt = getopt(argc, argv, "a:c:s:r:" ) )!= -1 ) {
    switch(opt) {
      case 'a':
        algoIdx = atoi( optarg );
        break;
      case 'c':
        indCount = atoi( optarg );
        break;
      case 's':
        sample_rate = atoi( optarg );
        break;
      case 'r':
        report_period = atoi( optarg );
        break;
      case '?':
        fprintf( stderr, usage_fmt, argv[0] );
        exit(0);
      default:
        break;
    }
  }

  test_init();
  test_open();

  create_amd_messages( &messages[ ALGO_AMD ] );
  create_ped_messages( &messages[ ALGO_PED ] );
  create_quat_messages( &messages[ ALGO_QUAT ] );
  create_fmv_messages( &messages[ ALGO_FMV ] );
  create_grav_messages( &messages[ ALGO_GRAV ] );
  create_smd_messages( &messages[ ALGO_SMD ]);
  create_rotv_messages( &messages[ ALGO_ROTV ]);

  send_req( &messages[ algoIdx ], SAM_ENABLE );
  send_req( &messages[ algoIdx ], SAM_GET_REPORT );

  // wait for some number of reports from SAM
  while( 0 != indCount && rcv_ind_count < indCount ) {
    sleep(1);
  }

  sleep(5);
  send_req( &messages[ algoIdx ], SAM_GET_REPORT );
  sleep(10);
  send_req( &messages[ algoIdx ], SAM_UPDATE );
  sleep(10);
  send_req( &messages[ algoIdx ], SAM_BATCH );
  sleep(20);
  send_req( &messages[ algoIdx ], SAM_DISABLE );
  sleep(5);
  send_req( &messages[ algoIdx ], SAM_CANCEL );

  sleep(1);

  test_close();

  printf( "All tests passed\n" );
  return 0;
}
