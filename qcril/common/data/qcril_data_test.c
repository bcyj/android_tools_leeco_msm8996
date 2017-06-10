/******************************************************************************

                  Q C R I L _ D A T A _ T E S T . C

******************************************************************************/

/******************************************************************************

  @file    qcril_data_test.c
  @brief   QCRIL Data tests 

  DESCRIPTION
  QCRIL_Data test functions.

******************************************************************************/
/*===========================================================================

  Copyright (c) 2011 - 2012 Qualcomm Technologies, Inc. All Rights Reserved

  Qualcomm Technologies Proprietary

  Export of this technology or software is regulated by the U.S. Government.
  Diversion contrary to U.S. law prohibited.

  All ideas, data and information contained in or disclosed by
  this document are confidential and proprietary information of
  Qualcomm Technologies, Inc. and all rights therein are expressly reserved.
  By accepting this material the recipient agrees that this material
  and the information contained therein are held in confidence and in
  trust and will not be used, copied, reproduced in whole or in part,
  nor its contents revealed in any manner to others without the express
  written permission of Qualcomm Technologies, Inc.

===========================================================================*/

/******************************************************************************

                      EDIT HISTORY FOR FILE
  $Id:$

when       who        what, where, why
--------   ---        -------------------------------------------------------
04/22/11   ar         Initial version

******************************************************************************/

#ifdef QCRIL_DATA_TEST 

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#ifndef QCRIL_DATA_OFFTARGET
#include <netinet/in.h>
#include <errno.h>
#endif
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <asm/types.h>
#include <pthread.h>
#include <linux/if.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>

#include "comdef.h"
#include "qmi.h"
#include "qmi_wds_srvc.h"
#include "qmi_qos_srvc.h"
#include "qmi_qos_srvc_i.h"
#include "qcrili.h"
#include "qcril_data.h"
#include "qcril_data_defs.h"
#include "qcril_data_client.h"
#include "qcril_data_stubs.h"
#include "qcril_data_test.h"

/* Current test info */
LOCAL qcril_data_test_info_t qcril_data_test_info;

qcril_data_test_client_info_t qcril_data_client_info;

#define QOS_FLOW_ID (0x01020304)

extern dsi_qos_granted_info_type dummy_qos;
extern qcril_data_dsi_cb_tbl_type dsi_cb_tbl[];

/*--------------------------------------------------------------------------- 
   Utility routines
---------------------------------------------------------------------------*/

#define QCRIL_DATA_CMD_WAIT()                                                      \
        pthread_mutex_lock(&qcril_data_test_info.mutx);                            \
        pthread_cond_wait(&qcril_data_test_info.cond, &qcril_data_test_info.mutx); \
        pthread_mutex_unlock(&qcril_data_test_info.mutx);


#define QCRIL_DATA_CMD_SIGNAL()                                                    \
        pthread_mutex_lock(&qcril_data_test_info.mutx);                            \
        pthread_cond_signal(&qcril_data_test_info.cond);                           \
        pthread_mutex_unlock(&qcril_data_test_info.mutx);

#define TEST_ASSERT(a)                                   \
        if(!(a)) {                                       \
          qcril_data_log_high("...FAILED: " #a "\n" );   \
          assert(a);                                     \
        } else {                                         \
          qcril_data_log_high("...Verified: " #a "\n" ); \
        }



LOCAL void stub_response_cb( qcril_stub_response_type type,
                             void *param_ptr,
                             void *user_data )
{
  qcril_request_resp_params_type *resp = NULL;
  qcril_unsol_resp_params_type *unsol = NULL;
  qcril_request_params_type params;
  qcril_data_event_data_t * evt_ptr = NULL;
  evt_ptr = (qcril_data_event_data_t*)malloc( sizeof(qcril_data_event_data_t) );
  evt_ptr->self = evt_ptr;
  evt_ptr->data = &info_tbl[0];
  evt_ptr->evt = DSI_EVT_QOS_STATUS_IND;
  evt_ptr->payload.qos_info.flow_id = QOS_FLOW_ID;
    
  /* Validate common attributes */
  TEST_ASSERT( qcril_data_client_info.exp_info.resp_type        == type );
  switch( type )
  {
    case QCRIL_STUB_RESPONSE_REQ:
      resp = (qcril_request_resp_params_type*)param_ptr;
      TEST_ASSERT( qcril_data_client_info.exp_info.resp.instance_id == resp->instance_id );
      TEST_ASSERT( qcril_data_client_info.exp_info.resp.t           == resp->t );
      TEST_ASSERT( qcril_data_client_info.exp_info.resp.request_id  == resp->request_id );
      TEST_ASSERT( qcril_data_client_info.exp_info.resp.ril_err_no  == resp->ril_err_no );
      params.instance_id = resp->instance_id;

      if( NULL != qcril_data_client_info.exp_info.resp.resp_pkt )
      {
        TEST_ASSERT( NULL != resp->resp_pkt );
    
        qcril_qos_resp_t *exp_ptr = &((qcril_data_call_response_type*)qcril_data_client_info.exp_info.resp.resp_pkt)->qos_resp;
        qcril_qos_resp_t *rcv_ptr = ((qcril_qos_resp_t*)resp->resp_pkt);

        switch( resp->request_id )
        {
          case RIL_REQUEST_SETUP_QOS:
            TEST_ASSERT( 0 == memcmp( exp_ptr->setup.return_code,
                                      rcv_ptr->setup.return_code,
                                      strlen(exp_ptr->setup.return_code)) );
            if( exp_ptr->setup.qos_flow_id[0] ) {
              TEST_ASSERT( 0 == memcmp( exp_ptr->setup.qos_flow_id[0],
                                        rcv_ptr->setup.qos_flow_id[0],
                                        strlen(exp_ptr->setup.qos_flow_id[0])) );
            }
            if( exp_ptr->setup.qos_flow_id[1] ) {
              TEST_ASSERT( 0 == memcmp( exp_ptr->setup.qos_flow_id[1],
                                        rcv_ptr->setup.qos_flow_id[1],
                                        strlen(exp_ptr->setup.qos_flow_id[1])) );
            }
            evt_ptr->payload.qos_info.status_evt  = QMI_QOS_ACTIVATED_EV;
            break;

          case RIL_REQUEST_RELEASE_QOS:
            TEST_ASSERT( 0 == memcmp( exp_ptr->result.return_code,
                                      rcv_ptr->result.return_code,
                                      strlen(exp_ptr->result.return_code)) );
            evt_ptr->payload.qos_info.status_evt  = QMI_QOS_GONE_EV;
            break;

          case RIL_REQUEST_SUSPEND_QOS:
            TEST_ASSERT( 0 == memcmp( exp_ptr->result.return_code,
                                      rcv_ptr->result.return_code,
                                      strlen(exp_ptr->result.return_code)) );
            evt_ptr->payload.qos_info.status_evt  = QMI_QOS_SUSPENDED_EV;
            break;

          case RIL_REQUEST_RESUME_QOS:
            TEST_ASSERT( 0 == memcmp( exp_ptr->result.return_code,
                                      rcv_ptr->result.return_code,
                                      strlen(exp_ptr->result.return_code)) );
            evt_ptr->payload.qos_info.status_evt  = QMI_QOS_ACTIVATED_EV;
            break;

          case RIL_REQUEST_MODIFY_QOS:
            TEST_ASSERT( 0 == memcmp( exp_ptr->result.return_code,
                                      rcv_ptr->result.return_code,
                                      strlen(exp_ptr->result.return_code)) );
            evt_ptr->payload.qos_info.status_evt  = QMI_QOS_MODIFY_ACCEPTED_EV;
            break;
        
          case RIL_REQUEST_GET_QOS_STATUS:
            TEST_ASSERT( 0 == memcmp( exp_ptr->get_status.return_code,
                                      rcv_ptr->get_status.return_code,
                                      strlen(exp_ptr->get_status.return_code)) );
            if( exp_ptr->get_status.status ) {
              TEST_ASSERT( 0 == memcmp( exp_ptr->get_status.status,
                                        rcv_ptr->get_status.status,
                                        strlen(exp_ptr->get_status.status)) );
            }
            if( exp_ptr->get_status.qos_spec1 ) {
              TEST_ASSERT( strlen(exp_ptr->get_status.qos_spec1) == strlen(rcv_ptr->get_status.qos_spec1) );
              TEST_ASSERT( 0 == memcmp( exp_ptr->get_status.qos_spec1,
                                        rcv_ptr->get_status.qos_spec1,
                                        strlen(exp_ptr->get_status.qos_spec1)) );
            }
            if( exp_ptr->get_status.qos_spec2 ) {
              TEST_ASSERT( strlen(exp_ptr->get_status.qos_spec2) == strlen(rcv_ptr->get_status.qos_spec2) );
              TEST_ASSERT( 0 == memcmp( exp_ptr->get_status.qos_spec2,
                                        rcv_ptr->get_status.qos_spec2,
                                        strlen(exp_ptr->get_status.qos_spec2)) );
            }
            evt_ptr->payload.qos_info.status_evt = 0;
            break;

          default:
            TEST_ASSERT(0);
        }
      }
      break;
      
    case QCRIL_STUB_RESPONSE_UNS:
      unsol = (qcril_unsol_resp_params_type*)param_ptr;
      TEST_ASSERT( qcril_data_client_info.exp_info.resp.instance_id == unsol->instance_id );
      evt_ptr->payload.qos_info.status_evt = 0;
      break;
  }

  params.data = evt_ptr;
  info_tbl[0].pend_tok   = NULL;
  info_tbl[0].pend_req   = DS_RIL_REQ_INVALID;  /* is there a RIL_REQUEST_INVALID */
  

#if 0 // Suppress for now, memory problem in test script
/* Conditionally trigger asynch handler */
  if( 0 != evt_ptr->payload.qos_info.status_evt )
  {
    QCRIL_DATA_MUTEX_UNLOCK(&info_tbl_mutex);
    
    qcril_data_client_info.exp_info.resp_type = QCRIL_STUB_RESPONSE_UNS;
    qcril_data_event_hdlr( &params, param_ptr );
  }
#endif
  
  qcril_data_client_info.testcase_status = QCRIL_DATA_SUCCESS;
}

/* Callback to validate the input to DSI_NetCtrl, and trigger QCRIL response */
LOCAL void dsi_cb( dsi_hndl_t hndl,
                   void *user_data,
                   dsi_net_evt_t evt,
                   dsi_evt_payload_t *payload_ptr )
{
  qcril_data_log_high("%s: received hndl[%#x] user_data[%#x] event[%d] payload[%p]\n",
                      __func__, (unsigned int)hndl, (unsigned int)user_data, evt, payload_ptr );

  TEST_ASSERT( evt == qcril_data_client_info.exp_info.evt );
  
  /* Compare payload elements */
  if( payload_ptr )
  {
    TEST_ASSERT( qcril_data_client_info.exp_info.flow_ID == payload_ptr->qos_info.flow_id  );
    TEST_ASSERT( qcril_data_client_info.exp_info.status_evt == payload_ptr->qos_info.status_evt );
    
    /* Compare the expected and received QOS specs */
    if( (DSI_QOS_ACTIVATED_EV       == payload_ptr->qos_info.status_evt ||
         DSI_QOS_MODIFY_ACCEPTED_EV == payload_ptr->qos_info.status_evt ) )
    {
      dsi_qos_spec_type  *exp_spec = &qcril_data_client_info.exp_info.qos_spec;
      dsi_qos_spec_type  *rcv_spec = (dsi_qos_spec_type*)user_data;

      if( exp_spec && rcv_spec )
      {
        TEST_ASSERT( exp_spec->num_tx_flow_req == rcv_spec->num_tx_flow_req );
        TEST_ASSERT( exp_spec->num_tx_filter_req == rcv_spec->num_tx_filter_req );
        if( exp_spec->num_tx_flow_req )
          TEST_ASSERT( (0==memcmp( exp_spec->tx_flow_req_array,
                                   rcv_spec->tx_flow_req_array,
                                   sizeof(qmi_qos_flow_req_type) )));
        if( exp_spec->num_tx_filter_req ) 
          TEST_ASSERT( (0==memcmp( exp_spec->tx_filter_req_array,
                                   rcv_spec->tx_filter_req_array,
                                   sizeof(qmi_qos_filter_req_type) )));
      
        TEST_ASSERT( exp_spec->num_rx_flow_req == rcv_spec->num_rx_flow_req );
        TEST_ASSERT( exp_spec->num_rx_filter_req == rcv_spec->num_rx_filter_req );
        if( exp_spec->num_rx_flow_req )
          TEST_ASSERT( (0==memcmp( exp_spec->rx_flow_req_array,
                                   rcv_spec->rx_flow_req_array,
                                   sizeof(qmi_qos_flow_req_type) )));
        if( exp_spec->num_rx_filter_req ) 
          TEST_ASSERT( (0==memcmp( exp_spec->rx_filter_req_array,
                                   rcv_spec->rx_filter_req_array,
                                   sizeof(qmi_qos_filter_req_type) )));
      }
    }
    else if( DSI_QOS_GONE_EV  == payload_ptr->qos_info.status_evt )
    {
    }
  }

  qcril_data_client_info.testcase_status = QCRIL_DATA_SUCCESS;
}

int qcril_data_test_reset()
{
  return 0;
}

void * qcril_data_test_make_flow( void )
{
  return NULL;
}

void * qcril_data_test_make_filter( void )
{
  return NULL;
}

/*--------------------------------------------------------------------------- 
   Testcases
---------------------------------------------------------------------------*/
#define RIL_INSTANCE_ID  (0)
#define RIL_MODEM_ID  (0)
#define RIL_TOKEN_ID  (0xFFFF)
#define RIL_CALL_ID  (1)

int qcril_data_test_qos_create_umts_sucessful1( void* arg )
{
  qcril_request_params_type *params_ptr = NULL;
  qcril_request_return_type *response_ptr = NULL;
  char * data_array[3];
  unsigned int  index = 0;
  RIL_Token   token = RIL_TOKEN_ID;
  dsi_qos_spec_type  *exp_spec = NULL;
  static qcril_data_call_response_type  call_resp;
  
  /* Setup initial state */
  memset( info_tbl, 0x0, sizeof(info_tbl) );
  info_tbl[ index ].instance_id = RIL_INSTANCE_ID;
  info_tbl[ index ].modem_id = RIL_MODEM_ID;
  info_tbl[ index ].index = index;
  info_tbl[ index ].cid = RIL_CALL_ID;
  info_tbl[ index ].self = &info_tbl[ index ];
  dsi_cb_tbl[0].info_tbl_ptr = &info_tbl[ index ];
    
  /* Allocate RIL command and response buffers */
  params_ptr = malloc( sizeof(qcril_request_params_type) );
  assert( params_ptr );
  memset( params_ptr, 0x0, sizeof(qcril_request_params_type) );
  response_ptr = malloc( sizeof(qcril_request_return_type) );
  assert( response_ptr ); 
  memset( response_ptr, 0x0, sizeof(qcril_request_return_type) );
  
  qcril_data_client_info.params_ptr = params_ptr;
  qcril_data_client_info.response_ptr = response_ptr;
  qcril_data_client_info.testcase_status = QCRIL_DATA_FAILURE;

  /* Generate RIL QOS specification */
  asprintf( &data_array[0], "%d", RIL_CALL_ID );
  asprintf( &data_array[1],
            "RIL_QOS_SPEC_INDEX=0,RIL_QOS_FLOW_DIRECTION=0,RIL_QOS_FLOW_DATA_RATE_MIN=64000,RIL_QOS_FLOW_DATA_RATE_MAX=64000,"
            "RIL_QOS_FILTER_INDEX=0,RIL_QOS_FILTER_DIRECTION=0,RIL_QOS_FILTER_IPVERSION=IP,"
            "RIL_QOS_FILTER_TCP_SOURCE_PORT_START=4000,RIL_QOS_FILTER_TCP_SOURCE_PORT_RANGE=1" );
  asprintf( &data_array[2],
            "RIL_QOS_SPEC_INDEX=0,RIL_QOS_FLOW_DIRECTION=1,RIL_QOS_FLOW_DATA_RATE_MIN=64000,RIL_QOS_FLOW_DATA_RATE_MAX=64000,"
            "RIL_QOS_FILTER_INDEX=0,RIL_QOS_FILTER_DIRECTION=1,RIL_QOS_FILTER_IPVERSION=IP,"
            "RIL_QOS_FILTER_TCP_SOURCE_PORT_START=4000,RIL_QOS_FILTER_TCP_SOURCE_PORT_RANGE=1" );

  /* Setup expected info */
  qcril_data_client_info.exp_info.resp_type = QCRIL_STUB_RESPONSE_REQ;
  qcril_data_client_info.exp_info.resp.t = RIL_TOKEN_ID;
  qcril_data_client_info.exp_info.resp.request_id = RIL_REQUEST_SETUP_QOS;
  qcril_data_client_info.exp_info.resp.ril_err_no = RIL_E_SUCCESS;
  qcril_data_client_info.exp_info.evt = DSI_EVT_QOS_STATUS_IND;
  qcril_data_client_info.exp_info.status_evt = DSI_QOS_ACTIVATED_EV;
  qcril_data_client_info.exp_info.flow_ID = QOS_FLOW_ID;
  qcril_data_client_info.exp_info.resp.resp_pkt = &call_resp;
  qcril_data_client_info.exp_info.resp.resp_len = sizeof(call_resp.qos_resp);
    
  asprintf( &call_resp.qos_resp.setup.return_code, "%d", RIL_E_SUCCESS );

  exp_spec = &qcril_data_client_info.exp_info.qos_spec;
  memset( exp_spec, 0x0, sizeof(dsi_qos_spec_type) );
  exp_spec->tx_flow_req_array = malloc( sizeof(qmi_qos_flow_req_type) );
  exp_spec->rx_flow_req_array = malloc( sizeof(qmi_qos_flow_req_type) );
  exp_spec->tx_filter_req_array = malloc( sizeof(qmi_qos_filter_req_type) );
  exp_spec->rx_filter_req_array = malloc( sizeof(qmi_qos_filter_req_type) );
  memset( exp_spec->tx_flow_req_array, 0x0, sizeof(qmi_qos_flow_req_type) );
  memset( exp_spec->rx_flow_req_array, 0x0, sizeof(qmi_qos_flow_req_type) );
  memset( exp_spec->tx_filter_req_array, 0x0, sizeof(qmi_qos_filter_req_type) );
  memset( exp_spec->rx_filter_req_array, 0x0, sizeof(qmi_qos_filter_req_type) );

  exp_spec->num_tx_flow_req = 1;
  exp_spec->tx_flow_req_array->umts_flow_desc.param_mask |= QMI_QOS_UMTS_FLOW_PARAM_DATA_RATE;
  exp_spec->tx_flow_req_array->umts_flow_desc.data_rate.guaranteed_rate = 64000; 
  exp_spec->tx_flow_req_array->umts_flow_desc.data_rate.max_rate = 64000; 
  exp_spec->num_tx_filter_req = 1;
  exp_spec->tx_filter_req_array->ip_version = QMI_QOS_IP_VERSION_4;
  exp_spec->tx_filter_req_array->filter_desc.param_mask |= QMI_QOS_FILTER_PARAM_TCP_SRC_PORTS;
  exp_spec->tx_filter_req_array->filter_desc.tcp_src_ports.start_port = ntohs(4000);
  exp_spec->tx_filter_req_array->filter_desc.tcp_src_ports.range = 1;
  exp_spec->tx_filter_req_array->filter_desc.param_mask |= QMI_QOS_FILTER_PARAM_TRANS_PROTOCOL;
  exp_spec->tx_filter_req_array->filter_desc.protocol = QMI_QOS_TRANS_PROT_TCP;
    
  exp_spec->num_rx_flow_req = 1;
  exp_spec->rx_flow_req_array->umts_flow_desc.param_mask |= QMI_QOS_UMTS_FLOW_PARAM_DATA_RATE;
  exp_spec->rx_flow_req_array->umts_flow_desc.data_rate.guaranteed_rate = 64000; 
  exp_spec->rx_flow_req_array->umts_flow_desc.data_rate.max_rate = 64000; 
  exp_spec->num_rx_filter_req = 1;
  exp_spec->rx_filter_req_array->ip_version = QMI_QOS_IP_VERSION_4;
  exp_spec->rx_filter_req_array->filter_desc.param_mask |= QMI_QOS_FILTER_PARAM_TCP_SRC_PORTS;
  exp_spec->rx_filter_req_array->filter_desc.tcp_src_ports.start_port = ntohs(4000);
  exp_spec->rx_filter_req_array->filter_desc.tcp_src_ports.range = 1;
  exp_spec->rx_filter_req_array->filter_desc.param_mask |= QMI_QOS_FILTER_PARAM_TRANS_PROTOCOL;
  exp_spec->rx_filter_req_array->filter_desc.protocol = QMI_QOS_TRANS_PROT_TCP;
  
  /* Setup RIL command buffer */
  params_ptr->instance_id = RIL_INSTANCE_ID;
  params_ptr->modem_id = RIL_MODEM_ID;
  params_ptr->t = RIL_TOKEN_ID;
  params_ptr->event_id = RIL_REQUEST_SETUP_QOS;
  params_ptr->data = (void*)data_array;
  params_ptr->datalen = sizeof(data_array);

  /* SETUP: Register with DSI_NetCtrl */
  dsi_init( DSI_MODE_GENERAL );
  info_tbl[ index ].dsi_hndl = dsi_cb_tbl[0].dsi_hndl = dsi_get_data_srvc_hndl( dsi_cb, (void*)&qcril_data_client_info );
    
  /* Process QOS command, validation is done in callback */
  qcril_data_request_setup_qos( params_ptr, response_ptr );

  /* SETUP: Release DSI_NetCtrl */
  dsi_rel_data_srvc_hndl( info_tbl[ index ].dsi_hndl );

  /* Release dynamic memory */
  free( exp_spec->tx_flow_req_array );
  free( exp_spec->rx_flow_req_array );
  free( exp_spec->tx_filter_req_array );
  free( exp_spec->rx_filter_req_array );
  free( data_array[0] );
  free( data_array[1] );
  free( data_array[2] );
  free( response_ptr );
  free( params_ptr );
  
  return qcril_data_client_info.testcase_status;
} /* qcril_data_test_qos_create_umts_sucessful1 */

int qcril_data_test_qos_create_umts_sucessful2( void* arg )
{
  qcril_request_params_type *params_ptr = NULL;
  qcril_request_return_type *response_ptr = NULL;
  char * data_array[3];
  unsigned int  index = 0;
  RIL_Token   token = RIL_TOKEN_ID;
  dsi_qos_spec_type  *exp_spec = NULL;
  static qcril_data_call_response_type  call_resp;
  
  /* Setup initial state */
  memset( info_tbl, 0x0, sizeof(info_tbl) );
  info_tbl[ index ].instance_id = RIL_INSTANCE_ID;
  info_tbl[ index ].modem_id = RIL_MODEM_ID;
  info_tbl[ index ].index = index;
  info_tbl[ index ].cid = RIL_CALL_ID;
  info_tbl[ index ].self = &info_tbl[ index ];

  qcril_data_client_info.testcase_status = QCRIL_DATA_FAILURE;
  qcril_data_client_info.exp_info.resp_type = QCRIL_STUB_RESPONSE_UNS;
    
  /* Allocate RIL command and response buffers */
  params_ptr = malloc( sizeof(qcril_request_params_type) );
  assert( params_ptr );
  memset( params_ptr, 0x0, sizeof(qcril_request_params_type) );
  response_ptr = malloc( sizeof(qcril_request_return_type) );
  assert( response_ptr ); 
  memset( response_ptr, 0x0, sizeof(qcril_request_return_type) );
  
  /* Generate RIL request parameters */
  asprintf( &data_array[0], "%d", RIL_CALL_ID );
  asprintf( &data_array[1],
            "RIL_QOS_SPEC_INDEX=0,RIL_QOS_FLOW_DIRECTION=0,RIL_QOS_FLOW_DATA_RATE_MIN=64000,RIL_QOS_FLOW_DATA_RATE_MAX=64000,"
            "RIL_QOS_FILTER_INDEX=0,RIL_QOS_FILTER_DIRECTION=0,RIL_QOS_FILTER_IPVERSION=IP,"
            "RIL_QOS_FILTER_TCP_SOURCE_PORT_START=4000,RIL_QOS_FILTER_TCP_SOURCE_PORT_RANGE=1,"
            "RIL_QOS_FILTER_IPV4_SOURCE_ADDR=10.2.5.111/16" );
  asprintf( &data_array[2],
            "RIL_QOS_SPEC_INDEX=0,RIL_QOS_FLOW_DIRECTION=1,RIL_QOS_FLOW_DATA_RATE_MIN=64000,RIL_QOS_FLOW_DATA_RATE_MAX=384000,"
            "RIL_QOS_FILTER_INDEX=0,RIL_QOS_FILTER_DIRECTION=1,RIL_QOS_FILTER_IPVERSION=IP,"
            "RIL_QOS_FILTER_TCP_DESTINATION_PORT_START=4000,RIL_QOS_FILTER_TCP_DESTINATION_PORT_RANGE=1,"
            "RIL_QOS_FILTER_IPV4_DESTINATION_ADDR=10.2.5.42/30" );

  /* Setup expected info */
  qcril_data_client_info.exp_info.resp_type = QCRIL_STUB_RESPONSE_REQ;
  qcril_data_client_info.exp_info.resp.t = RIL_TOKEN_ID;
  qcril_data_client_info.exp_info.resp.request_id = RIL_REQUEST_SETUP_QOS;
  qcril_data_client_info.exp_info.resp.ril_err_no = RIL_E_SUCCESS;
  qcril_data_client_info.exp_info.evt = DSI_EVT_QOS_STATUS_IND;
  qcril_data_client_info.exp_info.status_evt = DSI_QOS_ACTIVATED_EV;
  qcril_data_client_info.exp_info.flow_ID = QOS_FLOW_ID;
  qcril_data_client_info.exp_info.resp.resp_pkt = &call_resp;
  qcril_data_client_info.exp_info.resp.resp_len = sizeof(call_resp.qos_resp);
    
  asprintf( &call_resp.qos_resp.setup.return_code, "%d", RIL_E_SUCCESS );
  
  exp_spec = &qcril_data_client_info.exp_info.qos_spec;
  memset( exp_spec, 0x0, sizeof(dsi_qos_spec_type) );
  exp_spec->tx_flow_req_array = malloc( sizeof(qmi_qos_flow_req_type) );
  exp_spec->rx_flow_req_array = malloc( sizeof(qmi_qos_flow_req_type) );
  exp_spec->tx_filter_req_array = malloc( sizeof(qmi_qos_filter_req_type) );
  exp_spec->rx_filter_req_array = malloc( sizeof(qmi_qos_filter_req_type) );
  memset( exp_spec->tx_flow_req_array, 0x0, sizeof(qmi_qos_flow_req_type) );
  memset( exp_spec->rx_flow_req_array, 0x0, sizeof(qmi_qos_flow_req_type) );
  memset( exp_spec->tx_filter_req_array, 0x0, sizeof(qmi_qos_filter_req_type) );
  memset( exp_spec->rx_filter_req_array, 0x0, sizeof(qmi_qos_filter_req_type) );

  exp_spec->num_tx_flow_req = 1;
  exp_spec->tx_flow_req_array->umts_flow_desc.param_mask |= QMI_QOS_UMTS_FLOW_PARAM_DATA_RATE;
  exp_spec->tx_flow_req_array->umts_flow_desc.data_rate.guaranteed_rate = 64000; 
  exp_spec->tx_flow_req_array->umts_flow_desc.data_rate.max_rate = 64000; 
  exp_spec->num_tx_filter_req = 1;
  exp_spec->tx_filter_req_array->ip_version = QMI_QOS_IP_VERSION_4;
  exp_spec->tx_filter_req_array->filter_desc.param_mask |= QMI_QOS_FILTER_PARAM_TCP_SRC_PORTS;
  exp_spec->tx_filter_req_array->filter_desc.tcp_src_ports.start_port = ntohs(4000);
  exp_spec->tx_filter_req_array->filter_desc.tcp_src_ports.range = 1;
  exp_spec->tx_filter_req_array->filter_desc.param_mask |= QMI_QOS_FILTER_PARAM_TRANS_PROTOCOL;
  exp_spec->tx_filter_req_array->filter_desc.protocol = QMI_QOS_TRANS_PROT_TCP;
  exp_spec->tx_filter_req_array->filter_desc.param_mask |= QMI_QOS_FILTER_PARAM_SRC_ADDR;
  exp_spec->tx_filter_req_array->filter_desc.src_addr.ipv4_ip_addr = 0x0A02056F;
  exp_spec->tx_filter_req_array->filter_desc.src_addr.ipv4_subnet_mask = 0xFFFF0000;
    
  exp_spec->num_rx_flow_req = 1;
  exp_spec->rx_flow_req_array->umts_flow_desc.param_mask |= QMI_QOS_UMTS_FLOW_PARAM_DATA_RATE;
  exp_spec->rx_flow_req_array->umts_flow_desc.data_rate.guaranteed_rate = 64000; 
  exp_spec->rx_flow_req_array->umts_flow_desc.data_rate.max_rate = 384000; 
  exp_spec->num_rx_filter_req = 1;
  exp_spec->rx_filter_req_array->ip_version = QMI_QOS_IP_VERSION_4;
  exp_spec->rx_filter_req_array->filter_desc.param_mask |= QMI_QOS_FILTER_PARAM_TCP_DEST_PORTS;
  exp_spec->rx_filter_req_array->filter_desc.tcp_dest_ports.start_port = ntohs(4000);
  exp_spec->rx_filter_req_array->filter_desc.tcp_dest_ports.range = 1;
  exp_spec->rx_filter_req_array->filter_desc.param_mask |= QMI_QOS_FILTER_PARAM_TRANS_PROTOCOL;
  exp_spec->rx_filter_req_array->filter_desc.protocol = QMI_QOS_TRANS_PROT_TCP;
  exp_spec->rx_filter_req_array->filter_desc.param_mask |= QMI_QOS_FILTER_PARAM_DEST_ADDR;
  exp_spec->rx_filter_req_array->filter_desc.dest_addr.ipv4_ip_addr = 0x0A02052A;
  exp_spec->rx_filter_req_array->filter_desc.dest_addr.ipv4_subnet_mask = 0xFFFFFFFC;
  
  /* Setup RIL command buffer */
  params_ptr->instance_id = RIL_INSTANCE_ID;
  params_ptr->modem_id = RIL_MODEM_ID;
  params_ptr->t = RIL_TOKEN_ID;
  params_ptr->event_id = RIL_REQUEST_SETUP_QOS;
  params_ptr->data = (void*)data_array;
  params_ptr->datalen = sizeof(data_array);

  /* SETUP: Register with DSI_NetCtrl */
  dsi_init( DSI_MODE_GENERAL );
  info_tbl[ index ].dsi_hndl = dsi_cb_tbl[0].dsi_hndl = dsi_get_data_srvc_hndl( dsi_cb, (void*)&qcril_data_client_info );
    
  /* Process QOS command, validation is done in callback */
  qcril_data_request_setup_qos( params_ptr, response_ptr );

  /* SETUP: Release DSI_NetCtrl */
  dsi_rel_data_srvc_hndl( info_tbl[ index ].dsi_hndl );

  /* Release dynamic memory */
  free( exp_spec->tx_flow_req_array );
  free( exp_spec->rx_flow_req_array );
  free( exp_spec->tx_filter_req_array );
  free( exp_spec->rx_filter_req_array );
  free( data_array[0] );
  free( data_array[1] );
  free( data_array[2] );
  free( response_ptr );
  free( params_ptr );
  
  return qcril_data_client_info.testcase_status;
} /* qcril_data_test_qos_create_umts_sucessful2 */

int qcril_data_test_qos_create_umts_sucessful3( void* arg )
{
  qcril_request_params_type *params_ptr = NULL;
  qcril_request_return_type *response_ptr = NULL;
  char * data_array[2];
  unsigned int  index = 0;
  RIL_Token   token = RIL_TOKEN_ID;
  dsi_qos_spec_type  *exp_spec = NULL;
  static qcril_data_call_response_type  call_resp;
  
  /* Setup initial state */
  memset( info_tbl, 0x0, sizeof(info_tbl) );
  info_tbl[ index ].instance_id = RIL_INSTANCE_ID;
  info_tbl[ index ].modem_id = RIL_MODEM_ID;
  info_tbl[ index ].index = index;
  info_tbl[ index ].cid = RIL_CALL_ID;
  info_tbl[ index ].self = &info_tbl[ index ];
  dsi_cb_tbl[0].info_tbl_ptr = &info_tbl[ index ];
    
  /* Allocate RIL command and response buffers */
  params_ptr = malloc( sizeof(qcril_request_params_type) );
  assert( params_ptr );
  memset( params_ptr, 0x0, sizeof(qcril_request_params_type) );
  response_ptr = malloc( sizeof(qcril_request_return_type) );
  assert( response_ptr ); 
  memset( response_ptr, 0x0, sizeof(qcril_request_return_type) );
  
  qcril_data_client_info.params_ptr = params_ptr;
  qcril_data_client_info.response_ptr = response_ptr;
  qcril_data_client_info.testcase_status = QCRIL_DATA_FAILURE;

  /* Generate RIL QOS specification */
  asprintf( &data_array[0], "%d", RIL_CALL_ID );
  asprintf( &data_array[1],
            "RIL_QOS_SPEC_INDEX=0,RIL_QOS_FLOW_DIRECTION=0,RIL_QOS_FLOW_DATA_RATE_MIN=64000,"
            "RIL_QOS_FLOW_DATA_RATE_MAX=128000,RIL_QOS_FLOW_LATENCY=50,"
            "RIL_QOS_FILTER_INDEX=0,RIL_QOS_FILTER_IPVERSION=IP,RIL_QOS_FILTER_DIRECTION=0,"
            "RIL_QOS_FILTER_IPV4_DESTINATION_ADDR=10.42.12.13,"
            "RIL_QOS_FILTER_UDP_DESTINATION_PORT_START=4040,RIL_QOS_FILTER_UDP_DESTINATION_PORT_RANGE=20," 
            "RIL_QOS_FILTER_INDEX=1,RIL_QOS_FILTER_IPVERSION=IP,RIL_QOS_FILTER_DIRECTION=0,"
            "RIL_QOS_FILTER_IPV4_DESTINATION_ADDR=111.42.12.222,"
            "RIL_QOS_FILTER_UDP_DESTINATION_PORT_START=2222,RIL_QOS_FILTER_UDP_DESTINATION_PORT_RANGE=20" );

  /* Setup expected info */
  qcril_data_client_info.exp_info.resp_type = QCRIL_STUB_RESPONSE_REQ;
  qcril_data_client_info.exp_info.resp.t = RIL_TOKEN_ID;
  qcril_data_client_info.exp_info.resp.request_id = RIL_REQUEST_SETUP_QOS;
  qcril_data_client_info.exp_info.resp.ril_err_no = RIL_E_SUCCESS;
  qcril_data_client_info.exp_info.evt = DSI_EVT_QOS_STATUS_IND;
  qcril_data_client_info.exp_info.status_evt = DSI_QOS_ACTIVATED_EV;
  qcril_data_client_info.exp_info.flow_ID = QOS_FLOW_ID;
  qcril_data_client_info.exp_info.resp.resp_pkt = &call_resp;
  qcril_data_client_info.exp_info.resp.resp_len = sizeof(call_resp.qos_resp);
    
  asprintf( &call_resp.qos_resp.setup.return_code, "%d", RIL_E_SUCCESS );

  exp_spec = &qcril_data_client_info.exp_info.qos_spec;
  memset( exp_spec, 0x0, sizeof(dsi_qos_spec_type) );
  exp_spec->tx_flow_req_array = malloc( sizeof(qmi_qos_flow_req_type) );
  exp_spec->rx_flow_req_array = malloc( sizeof(qmi_qos_flow_req_type) );
  exp_spec->tx_filter_req_array = malloc( sizeof(qmi_qos_filter_req_type)*2 );
  exp_spec->rx_filter_req_array = malloc( sizeof(qmi_qos_filter_req_type) );
  memset( exp_spec->tx_flow_req_array, 0x0, sizeof(qmi_qos_flow_req_type) );
  memset( exp_spec->rx_flow_req_array, 0x0, sizeof(qmi_qos_flow_req_type) );
  memset( exp_spec->tx_filter_req_array, 0x0, sizeof(qmi_qos_filter_req_type)*2 );
  memset( exp_spec->rx_filter_req_array, 0x0, sizeof(qmi_qos_filter_req_type) );

  exp_spec->num_tx_flow_req = 1;
  exp_spec->tx_flow_req_array->umts_flow_desc.param_mask |= QMI_QOS_UMTS_FLOW_PARAM_DATA_RATE;
  exp_spec->tx_flow_req_array->umts_flow_desc.data_rate.guaranteed_rate = 64000; 
  exp_spec->tx_flow_req_array->umts_flow_desc.data_rate.max_rate = 128000; 
  exp_spec->tx_flow_req_array->umts_flow_desc.param_mask |= QMI_QOS_UMTS_FLOW_PARAM_MAX_DELAY;
  exp_spec->tx_flow_req_array->umts_flow_desc.max_delay = 50; 
  exp_spec->num_tx_filter_req = 2;
  exp_spec->tx_filter_req_array[0].ip_version = QMI_QOS_IP_VERSION_4;
  exp_spec->tx_filter_req_array[0].filter_desc.param_mask |= QMI_QOS_FILTER_PARAM_DEST_ADDR;
  exp_spec->tx_filter_req_array[0].filter_desc.dest_addr.ipv4_ip_addr =  0x0A2A0C0D;
  exp_spec->tx_filter_req_array[0].filter_desc.dest_addr.ipv4_subnet_mask = 0xFFFFFFFF;
  exp_spec->tx_filter_req_array[0].filter_desc.param_mask |= QMI_QOS_FILTER_PARAM_UDP_DEST_PORTS;
  exp_spec->tx_filter_req_array[0].filter_desc.udp_dest_ports.start_port = ntohs(4040);
  exp_spec->tx_filter_req_array[0].filter_desc.udp_dest_ports.range = 20;
  exp_spec->tx_filter_req_array[0].filter_desc.param_mask |= QMI_QOS_FILTER_PARAM_TRANS_PROTOCOL;
  exp_spec->tx_filter_req_array[0].filter_desc.protocol = QMI_QOS_TRANS_PROT_UDP;
  exp_spec->tx_filter_req_array[1].ip_version = QMI_QOS_IP_VERSION_4;
  exp_spec->tx_filter_req_array[1].filter_desc.param_mask |= QMI_QOS_FILTER_PARAM_DEST_ADDR;
  exp_spec->tx_filter_req_array[1].filter_desc.dest_addr.ipv4_ip_addr = 0x6F2A0CDE;
  exp_spec->tx_filter_req_array[1].filter_desc.dest_addr.ipv4_subnet_mask = 0xFFFFFFFF;
  exp_spec->tx_filter_req_array[1].filter_desc.param_mask |= QMI_QOS_FILTER_PARAM_UDP_DEST_PORTS;
  exp_spec->tx_filter_req_array[1].filter_desc.udp_dest_ports.start_port = ntohs(2222);
  exp_spec->tx_filter_req_array[1].filter_desc.udp_dest_ports.range = 20;
  exp_spec->tx_filter_req_array[1].filter_desc.param_mask |= QMI_QOS_FILTER_PARAM_TRANS_PROTOCOL;
  exp_spec->tx_filter_req_array[1].filter_desc.protocol = QMI_QOS_TRANS_PROT_UDP;
    
  
  /* Setup RIL command buffer */
  params_ptr->instance_id = RIL_INSTANCE_ID;
  params_ptr->modem_id = RIL_MODEM_ID;
  params_ptr->t = RIL_TOKEN_ID;
  params_ptr->event_id = RIL_REQUEST_SETUP_QOS;
  params_ptr->data = (void*)data_array;
  params_ptr->datalen = sizeof(data_array);

  /* SETUP: Register with DSI_NetCtrl */
  dsi_init( DSI_MODE_GENERAL );
  info_tbl[ index ].dsi_hndl = dsi_cb_tbl[0].dsi_hndl = dsi_get_data_srvc_hndl( dsi_cb, (void*)&qcril_data_client_info );
    
  /* Process QOS command, validation is done in callback */
  qcril_data_request_setup_qos( params_ptr, response_ptr );

  /* SETUP: Release DSI_NetCtrl */
  dsi_rel_data_srvc_hndl( info_tbl[ index ].dsi_hndl );

  /* Release dynamic memory */
  free( exp_spec->tx_flow_req_array );
  free( exp_spec->rx_flow_req_array );
  free( exp_spec->tx_filter_req_array );
  free( exp_spec->rx_filter_req_array );
  free( data_array[0] );
  free( data_array[1] );
  free( response_ptr );
  free( params_ptr );
  
  return qcril_data_client_info.testcase_status;
} /* qcril_data_test_qos_create_umts_sucessful3 */


int qcril_data_test_qos_create_umts_ipv6_sucessful( void* arg )
{
  qcril_request_params_type *params_ptr = NULL;
  qcril_request_return_type *response_ptr = NULL;
  char * data_array[3];
  unsigned int  index = 0;
  RIL_Token   token = RIL_TOKEN_ID;
  dsi_qos_spec_type  *exp_spec = NULL;
  static qcril_data_call_response_type  call_resp;
  
  /* Setup initial state */
  memset( info_tbl, 0x0, sizeof(info_tbl) );
  info_tbl[ index ].instance_id = RIL_INSTANCE_ID;
  info_tbl[ index ].modem_id = RIL_MODEM_ID;
  info_tbl[ index ].index = index;
  info_tbl[ index ].cid = RIL_CALL_ID;
  info_tbl[ index ].self = &info_tbl[ index ];

  qcril_data_client_info.testcase_status = QCRIL_DATA_FAILURE;
  qcril_data_client_info.exp_info.resp_type = QCRIL_STUB_RESPONSE_UNS;
    
  /* Allocate RIL command and response buffers */
  params_ptr = malloc( sizeof(qcril_request_params_type) );
  assert( params_ptr );
  memset( params_ptr, 0x0, sizeof(qcril_request_params_type) );
  response_ptr = malloc( sizeof(qcril_request_return_type) );
  assert( response_ptr ); 
  memset( response_ptr, 0x0, sizeof(qcril_request_return_type) );
  
  /* Generate RIL request parameters */
  asprintf( &data_array[0], "%d", RIL_CALL_ID );
  asprintf( &data_array[1],
            "RIL_QOS_SPEC_INDEX=0,RIL_QOS_FLOW_DIRECTION=0,RIL_QOS_FLOW_DATA_RATE_MIN=64000,RIL_QOS_FLOW_DATA_RATE_MAX=64000,"
            "RIL_QOS_FILTER_INDEX=0,RIL_QOS_FILTER_DIRECTION=0,RIL_QOS_FILTER_IPVERSION=IPV6,"
            "RIL_QOS_FILTER_TCP_SOURCE_PORT_START=4000,RIL_QOS_FILTER_TCP_SOURCE_PORT_RANGE=1,"
            "RIL_QOS_FILTER_IPV6_SOURCE_ADDR=2002:1ab3:3cb0:0:1234:5678:9abc:def0/64" );
  asprintf( &data_array[2],
            "RIL_QOS_SPEC_INDEX=0,RIL_QOS_FLOW_DIRECTION=1,RIL_QOS_FLOW_DATA_RATE_MIN=64000,RIL_QOS_FLOW_DATA_RATE_MAX=384000,"
            "RIL_QOS_FILTER_INDEX=0,RIL_QOS_FILTER_DIRECTION=1,RIL_QOS_FILTER_IPVERSION=IPV6,"
            "RIL_QOS_FILTER_TCP_DESTINATION_PORT_START=4000,RIL_QOS_FILTER_TCP_DESTINATION_PORT_RANGE=1,"
            "RIL_QOS_FILTER_IPV6_DESTINATION_ADDR=2002::ab00/128" );
  char ipv6_addr1[] = { 0x20, 0x02, 0x1a, 0xb3, 0x3c, 0xb0, 0x00, 0x00, 0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0 };
  char ipv6_addr2[] = { 0x20, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xab, 0x00 };
  
  /* Setup expected info */
  qcril_data_client_info.exp_info.resp_type = QCRIL_STUB_RESPONSE_REQ;
  qcril_data_client_info.exp_info.resp.t = RIL_TOKEN_ID;
  qcril_data_client_info.exp_info.resp.request_id = RIL_REQUEST_SETUP_QOS;
  qcril_data_client_info.exp_info.resp.ril_err_no = RIL_E_SUCCESS;
  qcril_data_client_info.exp_info.evt = DSI_EVT_QOS_STATUS_IND;
  qcril_data_client_info.exp_info.status_evt = DSI_QOS_ACTIVATED_EV;
  qcril_data_client_info.exp_info.flow_ID = QOS_FLOW_ID;
  qcril_data_client_info.exp_info.resp.resp_pkt = &call_resp;
  qcril_data_client_info.exp_info.resp.resp_len = sizeof(call_resp.qos_resp);
    
  asprintf( &call_resp.qos_resp.setup.return_code, "%d", RIL_E_SUCCESS );
  
  exp_spec = &qcril_data_client_info.exp_info.qos_spec;
  memset( exp_spec, 0x0, sizeof(dsi_qos_spec_type) );
  exp_spec->tx_flow_req_array = malloc( sizeof(qmi_qos_flow_req_type) );
  exp_spec->rx_flow_req_array = malloc( sizeof(qmi_qos_flow_req_type) );
  exp_spec->tx_filter_req_array = malloc( sizeof(qmi_qos_filter_req_type) );
  exp_spec->rx_filter_req_array = malloc( sizeof(qmi_qos_filter_req_type) );
  memset( exp_spec->tx_flow_req_array, 0x0, sizeof(qmi_qos_flow_req_type) );
  memset( exp_spec->rx_flow_req_array, 0x0, sizeof(qmi_qos_flow_req_type) );
  memset( exp_spec->tx_filter_req_array, 0x0, sizeof(qmi_qos_filter_req_type) );
  memset( exp_spec->rx_filter_req_array, 0x0, sizeof(qmi_qos_filter_req_type) );

  exp_spec->num_tx_flow_req = 1;
  exp_spec->tx_flow_req_array->umts_flow_desc.param_mask |= QMI_QOS_UMTS_FLOW_PARAM_DATA_RATE;
  exp_spec->tx_flow_req_array->umts_flow_desc.data_rate.guaranteed_rate = 64000; 
  exp_spec->tx_flow_req_array->umts_flow_desc.data_rate.max_rate = 64000; 
  exp_spec->num_tx_filter_req = 1;
  exp_spec->tx_filter_req_array->ip_version = QMI_QOS_IP_VERSION_6;
  exp_spec->tx_filter_req_array->filter_desc.param_mask |= QMI_QOS_FILTER_PARAM_TCP_SRC_PORTS;
  exp_spec->tx_filter_req_array->filter_desc.tcp_src_ports.start_port = ntohs(4000);
  exp_spec->tx_filter_req_array->filter_desc.tcp_src_ports.range = 1;
  exp_spec->tx_filter_req_array->filter_desc.param_mask |= QMI_QOS_FILTER_PARAM_TRANS_PROTOCOL;
  exp_spec->tx_filter_req_array->filter_desc.protocol = QMI_QOS_TRANS_PROT_TCP;
  exp_spec->tx_filter_req_array->filter_desc.param_mask |= QMI_QOS_FILTER_PARAM_IPV6_SRC_ADDR;
  memcpy(exp_spec->tx_filter_req_array->filter_desc.ipv6_src_addr.ipv6_ip_addr, ipv6_addr1, QMI_QOS_IPV6_ADDR_SIZE_IN_BYTES);
  exp_spec->tx_filter_req_array->filter_desc.ipv6_src_addr.ipv6_filter_prefix_len = 64;
    
  exp_spec->num_rx_flow_req = 1;
  exp_spec->rx_flow_req_array->umts_flow_desc.param_mask |= QMI_QOS_UMTS_FLOW_PARAM_DATA_RATE;
  exp_spec->rx_flow_req_array->umts_flow_desc.data_rate.guaranteed_rate = 64000; 
  exp_spec->rx_flow_req_array->umts_flow_desc.data_rate.max_rate = 384000; 
  exp_spec->num_rx_filter_req = 1;
  exp_spec->rx_filter_req_array->ip_version = QMI_QOS_IP_VERSION_6;
  exp_spec->rx_filter_req_array->filter_desc.param_mask |= QMI_QOS_FILTER_PARAM_TCP_DEST_PORTS;
  exp_spec->rx_filter_req_array->filter_desc.tcp_dest_ports.start_port = ntohs(4000);
  exp_spec->rx_filter_req_array->filter_desc.tcp_dest_ports.range = 1;
  exp_spec->rx_filter_req_array->filter_desc.param_mask |= QMI_QOS_FILTER_PARAM_TRANS_PROTOCOL;
  exp_spec->rx_filter_req_array->filter_desc.protocol = QMI_QOS_TRANS_PROT_TCP;
  exp_spec->rx_filter_req_array->filter_desc.param_mask |= QMI_QOS_FILTER_PARAM_IPV6_DEST_ADDR;
  memcpy(exp_spec->rx_filter_req_array->filter_desc.ipv6_dest_addr.ipv6_ip_addr, ipv6_addr2, QMI_QOS_IPV6_ADDR_SIZE_IN_BYTES);
  exp_spec->rx_filter_req_array->filter_desc.ipv6_dest_addr.ipv6_filter_prefix_len = 128;
  
  /* Setup RIL command buffer */
  params_ptr->instance_id = RIL_INSTANCE_ID;
  params_ptr->modem_id = RIL_MODEM_ID;
  params_ptr->t = RIL_TOKEN_ID;
  params_ptr->event_id = RIL_REQUEST_SETUP_QOS;
  params_ptr->data = (void*)data_array;
  params_ptr->datalen = sizeof(data_array);

  /* SETUP: Register with DSI_NetCtrl */
  dsi_init( DSI_MODE_GENERAL );
  info_tbl[ index ].dsi_hndl = dsi_cb_tbl[0].dsi_hndl = dsi_get_data_srvc_hndl( dsi_cb, (void*)&qcril_data_client_info );
    
  /* Process QOS command, validation is done in callback */
  qcril_data_request_setup_qos( params_ptr, response_ptr );

  /* SETUP: Release DSI_NetCtrl */
  dsi_rel_data_srvc_hndl( info_tbl[ index ].dsi_hndl );

  /* Release dynamic memory */
  free( exp_spec->tx_flow_req_array );
  free( exp_spec->rx_flow_req_array );
  free( exp_spec->tx_filter_req_array );
  free( exp_spec->rx_filter_req_array );
  free( data_array[0] );
  free( data_array[1] );
  free( data_array[2] );
  free( response_ptr );
  free( params_ptr );
  
  return qcril_data_client_info.testcase_status;
} /* qcril_data_test_qos_create_umts_ipv6_sucessful */


int qcril_data_test_qos_create_umts_failure1( void* arg )
{
  qcril_request_params_type *params_ptr = NULL;
  qcril_request_return_type *response_ptr = NULL;
  char * data_array[3];
  unsigned int  index = 0;
  RIL_Token   token = RIL_TOKEN_ID;
  
  /* Setup initial state */
  memset( info_tbl, 0x0, sizeof(info_tbl) );
  info_tbl[ index ].instance_id = RIL_INSTANCE_ID;
  info_tbl[ index ].modem_id = RIL_MODEM_ID;
  info_tbl[ index ].index = index;
  info_tbl[ index ].cid = RIL_CALL_ID;
  info_tbl[ index ].self = &info_tbl[ index ];

  qcril_data_client_info.testcase_status = QCRIL_DATA_FAILURE;
  qcril_data_client_info.exp_info.resp_type = QCRIL_STUB_RESPONSE_UNS;

  /* Allocate RIL command and response buffers */
  params_ptr = malloc( sizeof(qcril_request_params_type) );
  assert( params_ptr );
  memset( params_ptr, 0x0, sizeof(qcril_request_params_type) );

  response_ptr = malloc( sizeof(qcril_request_return_type) );
  assert( response_ptr ); 
  memset( response_ptr, 0x0, sizeof(qcril_request_return_type) );
  
  /* Generate RIL request parameters */
  asprintf( &data_array[0], "%d", RIL_CALL_ID );
  asprintf( &data_array[1],
            "RIL_QOS_SPEC_INDEX=0,RIL_QOS_FLOW_DIRECTION=0,RIL_QOS_FLOW_DATA_RATE_MIN=64000,RIL_QOS_FLOW_DATA_RATE_MAX=64000,"
            "RIL_QOS_FILTER_INDEX=0,RIL_QOS_FILTER_DIRECTION=0,RIL_QOS_FILTER_IPVERSION=IP,"
            "RIL_QOS_FILTER_TCP_SOURCE_PORT_START=4000,RIL_QOS_FILTER_TCP_SOURCE_PORT_RANGE=1" );
  asprintf( &data_array[2],
            "RIL_QOS_SPEC_INDEX=0,RIL_QOS_FLOW_DIRECTION=1,RIL_QOS_FLOW_DATA_RATE_MIN=64000,RIL_QOS_FLOW_DATA_RATE_MAX=64000,"
            "RIL_QOS_FILTER_INDEX=0,RIL_QOS_FILTER_DIRECTION=1,RIL_QOS_FILTER_IPVERSION=IP,"
            "RIL_QOS_FILTER_TCP_SOURCE_PORT_START=4000,RIL_QOS_FILTER_TCP_SOURCE_PORT_RANGE=1" );

  /* Setup expected info */
  qcril_data_client_info.exp_info.resp_type = QCRIL_STUB_RESPONSE_REQ;
  qcril_data_client_info.exp_info.resp.t = RIL_TOKEN_ID;
  qcril_data_client_info.exp_info.resp.request_id = RIL_REQUEST_SETUP_QOS;
  qcril_data_client_info.exp_info.resp.ril_err_no = RIL_E_SUCCESS;
  qcril_data_client_info.exp_info.evt = DSI_EVT_QOS_STATUS_IND;
  qcril_data_client_info.exp_info.status_evt = DSI_QOS_GONE_EV;
  qcril_data_client_info.exp_info.flow_ID = QOS_FLOW_ID;
  
  /* Setup RIL command buffer */
  params_ptr->instance_id = RIL_INSTANCE_ID;
  params_ptr->modem_id = RIL_MODEM_ID;
  params_ptr->t = RIL_TOKEN_ID;
  params_ptr->event_id = RIL_REQUEST_SETUP_QOS;
  params_ptr->data = (void*)data_array;
  params_ptr->datalen = sizeof(data_array);

  /* SETUP: Register with DSI_NetCtrl */
  dsi_init( DSI_MODE_GENERAL );
  info_tbl[ index ].dsi_hndl = dsi_cb_tbl[0].dsi_hndl = dsi_get_data_srvc_hndl( dsi_cb, (void*)&qcril_data_client_info );
  qcril_data_client_info.handle = -2;  /* key for stub */
  
  /* Process QOS command, validation is done in callback */
  qcril_data_request_setup_qos( params_ptr, response_ptr );

  /* SETUP: Release DSI_NetCtrl */
  dsi_rel_data_srvc_hndl( info_tbl[ index ].dsi_hndl );
  qcril_data_client_info.handle = NULL;

  /* Release dynamic memory */
  free( data_array[2] );
  free( data_array[1] );
  free( data_array[0] );
  free( response_ptr );
  free( params_ptr );
  
  return qcril_data_client_info.testcase_status;
} /* qcril_data_test_qos_create_umts_failure1 */

int qcril_data_test_qos_create_umts_failure2( void* arg )
{
  qcril_request_params_type *params_ptr = NULL;
  qcril_request_return_type *response_ptr = NULL;
  static qcril_data_call_response_type  call_resp;
  char * data_array[3];
  unsigned int  index = 0;
  RIL_Token   token = RIL_TOKEN_ID;
  
  /* Setup initial state */
  memset( info_tbl, 0x0, sizeof(info_tbl) );
  info_tbl[ index ].instance_id = RIL_INSTANCE_ID;
  info_tbl[ index ].modem_id = RIL_MODEM_ID;
  info_tbl[ index ].index = index;
  info_tbl[ index ].cid = RIL_CALL_ID;
  info_tbl[ index ].self = &info_tbl[ index ];

  qcril_data_client_info.testcase_status = QCRIL_DATA_FAILURE;
  qcril_data_client_info.exp_info.resp_type = QCRIL_STUB_RESPONSE_UNS;

  /* Allocate RIL command and response buffers */
  params_ptr = malloc( sizeof(qcril_request_params_type) );
  assert( params_ptr );
  memset( params_ptr, 0x0, sizeof(qcril_request_params_type) );

  response_ptr = malloc( sizeof(qcril_request_return_type) );
  assert( response_ptr ); 
  memset( response_ptr, 0x0, sizeof(qcril_request_return_type) );
  
  /* Generate RIL request parameters */
  asprintf( &data_array[0], "%d", RIL_CALL_ID );
  asprintf( &data_array[1],
            "RIL_QOS_SPEC_INDEX=0,RIL_QOS_FLOW_DIRECTION=0,RIL_QOS_FLOW_DATA_RATE_MIN=64000,RIL_QOS_FLOW_DATA_RATE_MAX=64000,"
            "RIL_QOS_FILTER_INDEX=0,RIL_QOS_FILTER_DIRECTION=0,RIL_QOS_FILTER_IPVERSION=IP,"
            "RIL_QOS_FILTER_TCP_SOURCE_PORT_START=4000,RIL_QOS_FILTER_TCP_SOURCE_PORT_RANGE=1" );
  asprintf( &data_array[2],
            "RIL_QOS_SPEC_INDEX=0,RIL_QOS_FLOW_DIRECTION=1,RIL_QOS_FLOW_DATA_RATE_MIN=64000,RIL_QOS_FLOW_DATA_RATE_MAX=64000,"
            "RIL_QOS_FILTER_INDEX=0,RIL_QOS_FILTER_DIRECTION=1,RIL_QOS_FILTER_IPVERSION=IP,"
            "RIL_QOS_FILTER_TCP_SOURCE_PORT_START=4000,RIL_QOS_FILTER_TCP_SOURCE_PORT_RANGE=1" );

  /* Setup expected info */
  qcril_data_client_info.exp_info.resp_type = QCRIL_STUB_RESPONSE_REQ;
  qcril_data_client_info.exp_info.resp.t = RIL_TOKEN_ID;
  qcril_data_client_info.exp_info.resp.request_id = RIL_REQUEST_SETUP_QOS;
  qcril_data_client_info.exp_info.resp.ril_err_no = RIL_E_SUCCESS;
  qcril_data_client_info.exp_info.evt = DSI_EVT_QOS_STATUS_IND;
  qcril_data_client_info.exp_info.resp.resp_pkt = &call_resp;
  qcril_data_client_info.exp_info.resp.resp_len = sizeof(call_resp.qos_resp);
  qcril_data_client_info.exp_info.status_evt = DSI_QOS_GONE_EV;
  qcril_data_client_info.exp_info.flow_ID = QOS_FLOW_ID;

  asprintf( &call_resp.qos_resp.setup.return_code, "%d", RIL_E_GENERIC_FAILURE );
  
  /* Setup RIL command buffer */
  params_ptr->instance_id = RIL_INSTANCE_ID;
  params_ptr->modem_id = RIL_MODEM_ID;
  params_ptr->t = RIL_TOKEN_ID;
  params_ptr->event_id = RIL_REQUEST_SETUP_QOS;
  params_ptr->data = (void*)data_array;
  params_ptr->datalen = sizeof(data_array);

  /* SETUP: Register with DSI_NetCtrl */
  dsi_init( DSI_MODE_GENERAL );
  info_tbl[ index ].dsi_hndl = dsi_cb_tbl[0].dsi_hndl = dsi_get_data_srvc_hndl( dsi_cb, (void*)&qcril_data_client_info );
  qcril_data_client_info.handle = -1;  /* key for stub */
  
  /* Process QOS command, validation is done in callback */
  qcril_data_request_setup_qos( params_ptr, response_ptr );

  /* SETUP: Release DSI_NetCtrl */
  dsi_rel_data_srvc_hndl( info_tbl[ index ].dsi_hndl );
  qcril_data_client_info.handle = NULL;

  /* Release dynamic memory */
  free( data_array[2] );
  free( data_array[1] );
  free( data_array[0] );
  free( response_ptr );
  free( params_ptr );
  
  return qcril_data_client_info.testcase_status;
} /* qcril_data_test_qos_create_umts_failure2 */

int qcril_data_test_qos_release_sucessful1( void* arg )
{
  qcril_request_params_type *params_ptr = NULL;
  qcril_request_return_type *response_ptr = NULL;
  static qcril_data_call_response_type  call_resp;
  char * data_array[1];
  unsigned int  index = 0;
  RIL_Token   token = RIL_TOKEN_ID;
  qcril_data_qos_state_type *flow_ptr = NULL;
  
  /* Setup initial state */
  memset( info_tbl, 0x0, sizeof(info_tbl) );
  info_tbl[ index ].instance_id = RIL_INSTANCE_ID;
  info_tbl[ index ].modem_id = RIL_MODEM_ID;
  info_tbl[ index ].index = index;
  info_tbl[ index ].cid = RIL_CALL_ID;
  info_tbl[ index ].self = &info_tbl[ index ];

  flow_ptr = (qcril_data_qos_state_type*)malloc( sizeof(qcril_data_qos_state_type) );
  flow_ptr->flow_id = QOS_FLOW_ID;
  list_push_back( &info_tbl[ index ].qos_flow_list, &flow_ptr->link );
  
  qcril_data_client_info.exp_info.resp_type = QCRIL_STUB_RESPONSE_UNS;

  /* Allocate RIL command and response buffers */
  params_ptr = malloc( sizeof(qcril_request_params_type) );
  assert( params_ptr );
  memset( params_ptr, 0x0, sizeof(qcril_request_params_type) );
  response_ptr = malloc( sizeof(qcril_request_return_type) );
  assert( response_ptr ); 
  memset( response_ptr, 0x0, sizeof(qcril_request_return_type) );
  
  /* Setup expected info */
  qcril_data_client_info.exp_info.resp_type = QCRIL_STUB_RESPONSE_REQ;
  qcril_data_client_info.exp_info.resp.t = RIL_TOKEN_ID;
  qcril_data_client_info.exp_info.resp.request_id = RIL_REQUEST_RELEASE_QOS;
  qcril_data_client_info.exp_info.resp.ril_err_no = RIL_E_SUCCESS;
  qcril_data_client_info.exp_info.evt = DSI_EVT_QOS_STATUS_IND;
  qcril_data_client_info.exp_info.resp.resp_pkt = &call_resp;
  qcril_data_client_info.exp_info.resp.resp_len = sizeof(call_resp.qos_resp);
  qcril_data_client_info.exp_info.status_evt = DSI_QOS_GONE_EV;

  asprintf( &call_resp.qos_resp.result.return_code, "%d", RIL_E_SUCCESS );
  
  /* Generate RIL request parameters */
  asprintf( &data_array[0], "0x%08x", QOS_FLOW_ID );
  
  /* Setup RIL command buffer */
  params_ptr->instance_id = RIL_INSTANCE_ID;
  params_ptr->modem_id = RIL_MODEM_ID;
  params_ptr->t = RIL_TOKEN_ID;
  params_ptr->event_id = RIL_REQUEST_RELEASE_QOS;
  params_ptr->data = (void*)data_array;
  params_ptr->datalen = sizeof(data_array);

  /* SETUP: Register with DSI_NetCtrl */
  dsi_init( DSI_MODE_GENERAL );
  info_tbl[ index ].dsi_hndl = dsi_cb_tbl[0].dsi_hndl = dsi_get_data_srvc_hndl( dsi_cb, (void*)&qcril_data_client_info );
    
  /* Process QOS command, validation is done in callback */
  qcril_data_request_release_qos( params_ptr, response_ptr );

  /* SETUP: Release DSI_NetCtrl */
  dsi_rel_data_srvc_hndl( info_tbl[ index ].dsi_hndl );
  
  /* Release dynamic memory */
  free( call_resp.qos_resp.result.return_code );
  free( flow_ptr );
  free( data_array[0] );
  free( response_ptr );
  free( params_ptr );
  
  return qcril_data_client_info.testcase_status;
} /* qcril_data_test_qos_release_sucessful1 */

int qcril_data_test_qos_release_failure1( void* arg )
{
  qcril_request_params_type *params_ptr = NULL;
  qcril_request_return_type *response_ptr = NULL;
  char * data_array[1];
  unsigned int  index = 0;
  RIL_Token   token = RIL_TOKEN_ID;
  qcril_data_qos_state_type *flow_ptr = NULL;
  
  /* Setup initial state */
  memset( info_tbl, 0x0, sizeof(info_tbl) );
  info_tbl[ index ].instance_id = RIL_INSTANCE_ID;
  info_tbl[ index ].modem_id = RIL_MODEM_ID;
  info_tbl[ index ].index = index;
  info_tbl[ index ].cid = RIL_CALL_ID;
  info_tbl[ index ].self = &info_tbl[ index ];
  flow_ptr = (qcril_data_qos_state_type*)malloc( sizeof(qcril_data_qos_state_type) );
  flow_ptr->flow_id = QOS_FLOW_ID;
  list_push_back( &info_tbl[ index ].qos_flow_list, &flow_ptr->link );
  
  /* Allocate RIL command and response buffers */
  params_ptr = malloc( sizeof(qcril_request_params_type) );
  assert( params_ptr );
  memset( params_ptr, 0x0, sizeof(qcril_request_params_type) );
  response_ptr = malloc( sizeof(qcril_request_return_type) );
  assert( response_ptr ); 
  memset( response_ptr, 0x0, sizeof(qcril_request_return_type) );
  
  /* Generate RIL request parameters */
  asprintf( &data_array[0], "%s", "0x123zz" );
  
  /* Setup RIL command buffer */
  params_ptr->instance_id = RIL_INSTANCE_ID;
  params_ptr->modem_id = RIL_MODEM_ID;
  params_ptr->t = RIL_TOKEN_ID;
  params_ptr->event_id = RIL_REQUEST_RELEASE_QOS;
  params_ptr->data = (void*)data_array;
  params_ptr->datalen = sizeof(data_array);

  /* SETUP: Register with DSI_NetCtrl */
  dsi_init( DSI_MODE_GENERAL );
  info_tbl[ index ].dsi_hndl = dsi_cb_tbl[0].dsi_hndl = dsi_get_data_srvc_hndl( dsi_cb, (void*)&qcril_data_client_info );
    
  /* Process QOS command, validation is done in callback */
  qcril_data_request_release_qos( params_ptr, response_ptr );

  /* SETUP: Release DSI_NetCtrl */
  dsi_rel_data_srvc_hndl( info_tbl[ index ].dsi_hndl );
  
  /* Release dynamic memory */
  free( flow_ptr );
  free( data_array[0] );
  free( response_ptr );
  free( params_ptr );
  
  return qcril_data_client_info.testcase_status;
} /* qcril_data_test_qos_release_failure1 */


int qcril_data_test_qos_getstatus_sucessful1( void* arg )
{
  qcril_request_params_type *params_ptr = NULL;
  qcril_request_return_type *response_ptr = NULL;
  static qcril_data_call_response_type  call_resp;
  char * data_array[1];
  unsigned int  index = 0;
  RIL_Token   token = RIL_TOKEN_ID;
  qcril_data_qos_state_type *flow_ptr = NULL;
  
  /* Setup initial state */
  memset( info_tbl, 0x0, sizeof(info_tbl) );
  info_tbl[ index ].instance_id = RIL_INSTANCE_ID;
  info_tbl[ index ].modem_id = RIL_MODEM_ID;
  info_tbl[ index ].index = index;
  info_tbl[ index ].cid = RIL_CALL_ID;
  info_tbl[ index ].self = &info_tbl[ index ];
  flow_ptr = (qcril_data_qos_state_type*)malloc( sizeof(qcril_data_qos_state_type) );
  flow_ptr->flow_id = QOS_FLOW_ID;
  list_push_back( &info_tbl[ index ].qos_flow_list, &flow_ptr->link );
  
  /* Allocate RIL command and response buffers */
  params_ptr = malloc( sizeof(qcril_request_params_type) );
  assert( params_ptr );
  memset( params_ptr, 0x0, sizeof(qcril_request_params_type) );
  response_ptr = malloc( sizeof(qcril_request_return_type) );
  assert( response_ptr ); 
  memset( response_ptr, 0x0, sizeof(qcril_request_return_type) );
  
  /* Generate RIL request parameters */
  asprintf( &data_array[0], "0x%08x", QOS_FLOW_ID );
  
  /* Setup RIL command buffer */
  params_ptr->instance_id = RIL_INSTANCE_ID;
  params_ptr->modem_id = RIL_MODEM_ID;
  params_ptr->t = RIL_TOKEN_ID;
  params_ptr->event_id = RIL_REQUEST_GET_QOS_STATUS;
  params_ptr->data = (void*)data_array;
  params_ptr->datalen = sizeof(data_array);

  /* Setup expected info */
  qcril_data_client_info.exp_info.resp.instance_id = params_ptr->instance_id;
  qcril_data_client_info.exp_info.resp.t = params_ptr->t;
  qcril_data_client_info.exp_info.resp.request_id = params_ptr->event_id;
  qcril_data_client_info.exp_info.resp.ril_err_no = RIL_E_SUCCESS;
  qcril_data_client_info.exp_info.resp.resp_pkt = &call_resp;
  qcril_data_client_info.exp_info.resp.resp_len = sizeof(call_resp.qos_resp);
  qcril_data_client_info.exp_info.resp_type = QCRIL_STUB_RESPONSE_REQ;
  qcril_data_client_info.exp_info.flow_ID = QOS_FLOW_ID;

  asprintf( &call_resp.qos_resp.get_status.return_code, "%d", RIL_E_SUCCESS );
  asprintf( &call_resp.qos_resp.get_status.status, "%d", RIL_QOS_STATUS_ACTIVATED );
  asprintf( &call_resp.qos_resp.get_status.qos_spec1,
            "RIL_QOS_SPEC_INDEX=%d,RIL_QOS_FLOW_DIRECTION=%d,"
            "RIL_QOS_FLOW_TRAFFIC_CLASS=%d,"
            "RIL_QOS_FLOW_DATA_RATE_MIN=%d,RIL_QOS_FLOW_DATA_RATE_MAX=%d,"
            "RIL_QOS_FLOW_LATENCY=%d,"
            "RIL_QOS_FLOW_3GPP2_PROFILE_ID=%d,RIL_QOS_FLOW_3GPP2_PRIORITY=%d",
            dummy_qos.tx_granted_flow_data.ip_flow_index,
            RIL_QOS_TX,
            dummy_qos.tx_granted_flow_data.qos_flow_granted.umts_flow_desc.traffic_class,
            dummy_qos.tx_granted_flow_data.qos_flow_granted.umts_flow_desc.data_rate.guaranteed_rate,
            dummy_qos.tx_granted_flow_data.qos_flow_granted.umts_flow_desc.data_rate.max_rate,
            dummy_qos.tx_granted_flow_data.qos_flow_granted.umts_flow_desc.max_delay,
            dummy_qos.tx_granted_flow_data.qos_flow_granted.cdma_flow_desc.profile_id,
            dummy_qos.tx_granted_flow_data.qos_flow_granted.umts_flow_desc.flow_priority_3gpp2);
  asprintf( &call_resp.qos_resp.get_status.qos_spec2,
            "RIL_QOS_SPEC_INDEX=%d,RIL_QOS_FLOW_DIRECTION=%d,"
            "RIL_QOS_FLOW_DATA_RATE_MIN=%d,RIL_QOS_FLOW_DATA_RATE_MAX=%d",
            dummy_qos.rx_granted_flow_data.ip_flow_index,
            RIL_QOS_RX,
            dummy_qos.rx_granted_flow_data.qos_flow_granted.umts_flow_desc.data_rate.guaranteed_rate,
            dummy_qos.rx_granted_flow_data.qos_flow_granted.umts_flow_desc.data_rate.max_rate );
            
  /* SETUP: Register with DSI_NetCtrl */
  dsi_init( DSI_MODE_GENERAL );
  info_tbl[ index ].dsi_hndl = dsi_cb_tbl[0].dsi_hndl = dsi_get_data_srvc_hndl( dsi_cb, (void*)&qcril_data_client_info );
    
  /* Process QOS command, validation is done in callback */
  qcril_data_request_get_qos_status( params_ptr, response_ptr );

  /* SETUP: Release DSI_NetCtrl */
  dsi_rel_data_srvc_hndl( info_tbl[ index ].dsi_hndl );
  qcril_data_client_info.handle = NULL;

  
  /* Release dynamic memory */
  free( call_resp.qos_resp.get_status.return_code );
  free( call_resp.qos_resp.get_status.status );
  free( call_resp.qos_resp.get_status.qos_spec1 );
  free( call_resp.qos_resp.get_status.qos_spec2 );
  free( flow_ptr );
  free( data_array[0] );
  free( response_ptr );
  free( params_ptr );
  
  return qcril_data_client_info.testcase_status;
} /* qcril_data_test_qos_getstatus_sucessful1 */

int qcril_data_test_qos_getstatus_failure1( void* arg )
{
  qcril_request_params_type *params_ptr = NULL;
  qcril_request_return_type *response_ptr = NULL;
  static qcril_data_call_response_type  call_resp;
  char * data_array[1];
  unsigned int  index = 0;
  RIL_Token   token = RIL_TOKEN_ID;
  qcril_data_qos_state_type *flow_ptr = NULL;
  
  /* Setup initial state */
  memset( info_tbl, 0x0, sizeof(info_tbl) );
  info_tbl[ index ].instance_id = RIL_INSTANCE_ID;
  info_tbl[ index ].modem_id = RIL_MODEM_ID;
  info_tbl[ index ].index = index;
  info_tbl[ index ].cid = RIL_CALL_ID;
  info_tbl[ index ].self = &info_tbl[ index ];
  flow_ptr = (qcril_data_qos_state_type*)malloc( sizeof(qcril_data_qos_state_type) );
  flow_ptr->flow_id = QOS_FLOW_ID;
  list_push_back( &info_tbl[ index ].qos_flow_list, &flow_ptr->link );
  
  /* Allocate RIL command and response buffers */
  params_ptr = malloc( sizeof(qcril_request_params_type) );
  assert( params_ptr );
  memset( params_ptr, 0x0, sizeof(qcril_request_params_type) );
  response_ptr = malloc( sizeof(qcril_request_return_type) );
  assert( response_ptr ); 
  memset( response_ptr, 0x0, sizeof(qcril_request_return_type) );
  
  /* Generate RIL request parameters - invalid */
  asprintf( &data_array[0], "0x%08x", 999 );
  
  /* Setup RIL command buffer */
  params_ptr->instance_id = RIL_INSTANCE_ID;
  params_ptr->modem_id = RIL_MODEM_ID;
  params_ptr->t = RIL_TOKEN_ID;
  params_ptr->event_id = RIL_REQUEST_GET_QOS_STATUS;
  params_ptr->data = (void*)data_array;
  params_ptr->datalen = sizeof(data_array);

  qcril_data_client_info.exp_info.resp.instance_id = params_ptr->instance_id;
  qcril_data_client_info.exp_info.resp.t = params_ptr->t;
  qcril_data_client_info.exp_info.resp.request_id = params_ptr->event_id;
  qcril_data_client_info.exp_info.resp.ril_err_no = RIL_E_SUCCESS;
  qcril_data_client_info.exp_info.resp.resp_pkt = &call_resp;
  qcril_data_client_info.exp_info.resp.resp_len = sizeof(call_resp.qos_resp);
  qcril_data_client_info.exp_info.resp_type = QCRIL_STUB_RESPONSE_REQ;
  qcril_data_client_info.exp_info.flow_ID = QOS_FLOW_ID;

  asprintf( &call_resp.qos_resp.get_status.return_code, "%d", RIL_E_GENERIC_FAILURE );
  
  /* SETUP: Register with DSI_NetCtrl */
  dsi_init( DSI_MODE_GENERAL );
  info_tbl[ index ].dsi_hndl = dsi_cb_tbl[0].dsi_hndl = dsi_get_data_srvc_hndl( dsi_cb, (void*)&qcril_data_client_info );
    
  /* Process QOS command, validation is done in callback */
  qcril_data_request_get_qos_status( params_ptr, response_ptr );

  /* SETUP: Release DSI_NetCtrl */
  dsi_rel_data_srvc_hndl( info_tbl[ index ].dsi_hndl );
  
  /* Release dynamic memory */
  free( flow_ptr );
  free( data_array[0] );
  free( response_ptr );
  free( params_ptr );
  
  return qcril_data_client_info.testcase_status;
} /* qcril_data_test_qos_getgranted_failure1 */

int qcril_data_test_qos_suspend_sucessful1( void* arg )
{
  qcril_request_params_type *params_ptr = NULL;
  qcril_request_return_type *response_ptr = NULL;
  static qcril_data_call_response_type  call_resp;
  char * data_array[1];
  unsigned int  index = 0;
  RIL_Token   token = RIL_TOKEN_ID;
  qcril_data_qos_state_type *flow_ptr = NULL;
  
  /* Setup initial state */
  memset( info_tbl, 0x0, sizeof(info_tbl) );
  info_tbl[ index ].instance_id = RIL_INSTANCE_ID;
  info_tbl[ index ].modem_id = RIL_MODEM_ID;
  info_tbl[ index ].index = index;
  info_tbl[ index ].cid = RIL_CALL_ID;
  info_tbl[ index ].self = &info_tbl[ index ];
  flow_ptr = (qcril_data_qos_state_type*)malloc( sizeof(qcril_data_qos_state_type) );
  flow_ptr->flow_id = QOS_FLOW_ID;
  list_push_back( &info_tbl[ index ].qos_flow_list, &flow_ptr->link );
  
  qcril_data_client_info.exp_info.resp_type = QCRIL_STUB_RESPONSE_UNS;

  /* Allocate RIL command and response buffers */
  params_ptr = malloc( sizeof(qcril_request_params_type) );
  assert( params_ptr );
  memset( params_ptr, 0x0, sizeof(qcril_request_params_type) );
  response_ptr = malloc( sizeof(qcril_request_return_type) );
  assert( response_ptr ); 
  memset( response_ptr, 0x0, sizeof(qcril_request_return_type) );
  
  /* Setup expected info */
  qcril_data_client_info.exp_info.resp_type = QCRIL_STUB_RESPONSE_REQ;
  qcril_data_client_info.exp_info.resp.t = RIL_TOKEN_ID;
  qcril_data_client_info.exp_info.resp.request_id = RIL_REQUEST_SUSPEND_QOS;
  qcril_data_client_info.exp_info.resp.ril_err_no = RIL_E_SUCCESS;
  qcril_data_client_info.exp_info.resp.resp_pkt = &call_resp;
  qcril_data_client_info.exp_info.resp.resp_len = sizeof(call_resp.qos_resp);
  qcril_data_client_info.exp_info.evt = DSI_EVT_QOS_STATUS_IND;
  qcril_data_client_info.exp_info.status_evt = DSI_QOS_SUSPENDED_EV;
  qcril_data_client_info.exp_info.flow_ID = QOS_FLOW_ID;

  asprintf( &call_resp.qos_resp.result.return_code, "%d", RIL_E_SUCCESS );
  
  /* Generate RIL request parameters */
  asprintf( &data_array[0], "0x%08x", QOS_FLOW_ID );
  
  /* Setup RIL command buffer */
  params_ptr->instance_id = RIL_INSTANCE_ID;
  params_ptr->modem_id = RIL_MODEM_ID;
  params_ptr->t = RIL_TOKEN_ID;
  params_ptr->event_id = RIL_REQUEST_SUSPEND_QOS;
  params_ptr->data = (void*)data_array;
  params_ptr->datalen = sizeof(data_array);

  /* SETUP: Register with DSI_NetCtrl */
  dsi_init( DSI_MODE_GENERAL );
  info_tbl[ index ].dsi_hndl = dsi_cb_tbl[0].dsi_hndl = dsi_get_data_srvc_hndl( dsi_cb, (void*)&qcril_data_client_info );
    
  /* Process QOS command, validation is done in callback */
  qcril_data_request_suspend_qos( params_ptr, response_ptr );

  /* SETUP: Release DSI_NetCtrl */
  dsi_rel_data_srvc_hndl( info_tbl[ index ].dsi_hndl );
  
  /* Release dynamic memory */
  free( call_resp.qos_resp.result.return_code );
  free( flow_ptr );
  free( data_array[0] );
  free( response_ptr );
  free( params_ptr );
  
  return qcril_data_client_info.testcase_status;
} /* qcril_data_test_qos_suspend_sucessful1 */

int qcril_data_test_qos_suspend_failure1( void* arg )
{
  qcril_request_params_type *params_ptr = NULL;
  qcril_request_return_type *response_ptr = NULL;
  static qcril_data_call_response_type  call_resp;
  char * data_array[1];
  unsigned int  index = 0;
  RIL_Token   token = RIL_TOKEN_ID;
  qcril_data_qos_state_type *flow_ptr = NULL;
  
  /* Setup initial state */
  memset( info_tbl, 0x0, sizeof(info_tbl) );
  info_tbl[ index ].instance_id = RIL_INSTANCE_ID;
  info_tbl[ index ].modem_id = RIL_MODEM_ID;
  info_tbl[ index ].index = index;
  info_tbl[ index ].cid = RIL_CALL_ID;
  info_tbl[ index ].self = &info_tbl[ index ];
  flow_ptr = (qcril_data_qos_state_type*)malloc( sizeof(qcril_data_qos_state_type) );
  flow_ptr->flow_id = QOS_FLOW_ID;
  list_push_back( &info_tbl[ index ].qos_flow_list, &flow_ptr->link );
  
  qcril_data_client_info.exp_info.resp_type = QCRIL_STUB_RESPONSE_UNS;

  /* Allocate RIL command and response buffers */
  params_ptr = malloc( sizeof(qcril_request_params_type) );
  assert( params_ptr );
  memset( params_ptr, 0x0, sizeof(qcril_request_params_type) );
  response_ptr = malloc( sizeof(qcril_request_return_type) );
  assert( response_ptr ); 
  memset( response_ptr, 0x0, sizeof(qcril_request_return_type) );
  
  /* Setup expected info */
  qcril_data_client_info.exp_info.resp_type = QCRIL_STUB_RESPONSE_REQ;
  qcril_data_client_info.exp_info.resp.t = RIL_TOKEN_ID;
  qcril_data_client_info.exp_info.resp.request_id = RIL_REQUEST_SUSPEND_QOS;
  qcril_data_client_info.exp_info.resp.ril_err_no = RIL_E_SUCCESS;
  qcril_data_client_info.exp_info.resp.resp_pkt = &call_resp;
  qcril_data_client_info.exp_info.resp.resp_len = sizeof(call_resp.qos_resp);
  qcril_data_client_info.exp_info.evt = DSI_EVT_QOS_STATUS_IND;
  qcril_data_client_info.exp_info.status_evt = DSI_QOS_SUSPENDED_EV;
  qcril_data_client_info.exp_info.flow_ID = QOS_FLOW_ID;

  asprintf( &call_resp.qos_resp.get_status.return_code, "%d", RIL_E_GENERIC_FAILURE );
  
  /* Generate RIL request parameters */
  asprintf( &data_array[0], "0x%08x", QOS_FLOW_ID );
  
  /* Setup RIL command buffer */
  params_ptr->instance_id = RIL_INSTANCE_ID;
  params_ptr->modem_id = RIL_MODEM_ID;
  params_ptr->t = RIL_TOKEN_ID;
  params_ptr->event_id = RIL_REQUEST_SUSPEND_QOS;
  params_ptr->data = (void*)data_array;
  params_ptr->datalen = sizeof(data_array);

  /* SETUP: Register with DSI_NetCtrl */
  dsi_init( DSI_MODE_GENERAL );
  info_tbl[ index ].dsi_hndl = dsi_cb_tbl[0].dsi_hndl = dsi_get_data_srvc_hndl( dsi_cb, (void*)&qcril_data_client_info );
  qcril_data_client_info.handle = -1; /* key for stub */
  
  /* Process QOS command, validation is done in callback */
  qcril_data_request_suspend_qos( params_ptr, response_ptr );

  /* SETUP: Release DSI_NetCtrl */
  dsi_rel_data_srvc_hndl( info_tbl[ index ].dsi_hndl );
  
  /* Release dynamic memory */
  free( flow_ptr );
  free( data_array[0] );
  free( response_ptr );
  free( params_ptr );
  
  return qcril_data_client_info.testcase_status;
} /* qcril_data_test_qos_suspend_failure1 */

int qcril_data_test_qos_resume_sucessful1( void* arg )
{
  qcril_request_params_type *params_ptr = NULL;
  qcril_request_return_type *response_ptr = NULL;
  static qcril_data_call_response_type  call_resp;
  char * data_array[1];
  unsigned int  index = 0;
  RIL_Token   token = RIL_TOKEN_ID;
  qcril_data_qos_state_type *flow_ptr = NULL;
  
  /* Setup initial state */
  memset( info_tbl, 0x0, sizeof(info_tbl) );
  info_tbl[ index ].instance_id = RIL_INSTANCE_ID;
  info_tbl[ index ].modem_id = RIL_MODEM_ID;
  info_tbl[ index ].index = index;
  info_tbl[ index ].cid = RIL_CALL_ID;
  info_tbl[ index ].self = &info_tbl[ index ];
  flow_ptr = (qcril_data_qos_state_type*)malloc( sizeof(qcril_data_qos_state_type) );
  flow_ptr->flow_id = QOS_FLOW_ID;
  list_push_back( &info_tbl[ index ].qos_flow_list, &flow_ptr->link );
  
  qcril_data_client_info.exp_info.resp_type = QCRIL_STUB_RESPONSE_UNS;

  /* Allocate RIL command and response buffers */
  params_ptr = malloc( sizeof(qcril_request_params_type) );
  assert( params_ptr );
  memset( params_ptr, 0x0, sizeof(qcril_request_params_type) );
  response_ptr = malloc( sizeof(qcril_request_return_type) );
  assert( response_ptr ); 
  memset( response_ptr, 0x0, sizeof(qcril_request_return_type) );
  
  /* Setup expected info */
  qcril_data_client_info.exp_info.resp_type = QCRIL_STUB_RESPONSE_REQ;
  qcril_data_client_info.exp_info.resp.t = RIL_TOKEN_ID;
  qcril_data_client_info.exp_info.resp.request_id = RIL_REQUEST_RESUME_QOS;
  qcril_data_client_info.exp_info.resp.ril_err_no = RIL_E_SUCCESS;
  qcril_data_client_info.exp_info.resp.resp_pkt = &call_resp;
  qcril_data_client_info.exp_info.resp.resp_len = sizeof(call_resp.qos_resp);
  qcril_data_client_info.exp_info.evt = DSI_EVT_QOS_STATUS_IND;
  qcril_data_client_info.exp_info.status_evt = DSI_QOS_ACTIVATED_EV;
  qcril_data_client_info.exp_info.flow_ID = QOS_FLOW_ID;

  asprintf( &call_resp.qos_resp.result.return_code, "%d", RIL_E_SUCCESS );
  
  /* Generate RIL request parameters */
  asprintf( &data_array[0], "0x%08x", QOS_FLOW_ID );
  
  /* Setup RIL command buffer */
  params_ptr->instance_id = RIL_INSTANCE_ID;
  params_ptr->modem_id = RIL_MODEM_ID;
  params_ptr->t = RIL_TOKEN_ID;
  params_ptr->event_id = RIL_REQUEST_RESUME_QOS;
  params_ptr->data = (void*)data_array;
  params_ptr->datalen = sizeof(data_array);

  /* SETUP: Register with DSI_NetCtrl */
  dsi_init( DSI_MODE_GENERAL );
  info_tbl[ index ].dsi_hndl = dsi_cb_tbl[0].dsi_hndl = dsi_get_data_srvc_hndl( dsi_cb, NULL );
  qcril_data_client_info.handle = 0; /* key for stub */
    
  /* Process QOS command, validation is done in callback */
  qcril_data_request_resume_qos( params_ptr, response_ptr );

  /* SETUP: Release DSI_NetCtrl */
  dsi_rel_data_srvc_hndl( info_tbl[ index ].dsi_hndl );
  
  /* Release dynamic memory */
  free( call_resp.qos_resp.result.return_code );
  free( flow_ptr );
  free( data_array[0] );
  free( response_ptr );
  free( params_ptr );
  
  return qcril_data_client_info.testcase_status;
} /* qcril_data_test_qos_resume_sucessful1 */

int qcril_data_test_qos_modify_umts_sucessful1( void* arg )
{
  qcril_request_params_type *params_ptr = NULL;
  qcril_request_return_type *response_ptr = NULL;
  static qcril_data_call_response_type  call_resp;
  char * data_array[3];
  unsigned int  index = 0;
  RIL_Token   token = RIL_TOKEN_ID;
  dsi_qos_spec_type  *exp_spec = NULL;
  qcril_data_qos_state_type *flow_ptr = NULL;
  
  /* Setup initial state */
  memset( info_tbl, 0x0, sizeof(info_tbl) );
  info_tbl[ index ].instance_id = RIL_INSTANCE_ID;
  info_tbl[ index ].modem_id = RIL_MODEM_ID;
  info_tbl[ index ].index = index;
  info_tbl[ index ].cid = RIL_CALL_ID;
  info_tbl[ index ].self = &info_tbl[ index ];
  dsi_cb_tbl[0].info_tbl_ptr = &info_tbl[ index ];
  flow_ptr = (qcril_data_qos_state_type*)malloc( sizeof(qcril_data_qos_state_type) );
  flow_ptr->flow_id = QOS_FLOW_ID;
  list_push_back( &info_tbl[ index ].qos_flow_list, &flow_ptr->link );
    
  /* Allocate RIL command and response buffers */
  params_ptr = malloc( sizeof(qcril_request_params_type) );
  assert( params_ptr );
  memset( params_ptr, 0x0, sizeof(qcril_request_params_type) );
  response_ptr = malloc( sizeof(qcril_request_return_type) );
  assert( response_ptr ); 
  memset( response_ptr, 0x0, sizeof(qcril_request_return_type) );
  
  qcril_data_client_info.params_ptr = params_ptr;
  qcril_data_client_info.response_ptr = response_ptr;
  qcril_data_client_info.testcase_status = QCRIL_DATA_FAILURE;

  /* Generate RIL QOS specification */
  asprintf( &data_array[0], "0x%08x", QOS_FLOW_ID );
  asprintf( &data_array[1],
            "RIL_QOS_SPEC_INDEX=0,RIL_QOS_FLOW_DIRECTION=0,RIL_QOS_FLOW_DATA_RATE_MIN=64000,RIL_QOS_FLOW_DATA_RATE_MAX=64000,"
            "RIL_QOS_FILTER_INDEX=0,RIL_QOS_FILTER_DIRECTION=0,RIL_QOS_FILTER_IPVERSION=IP,"
            "RIL_QOS_FILTER_TCP_SOURCE_PORT_START=4000,RIL_QOS_FILTER_TCP_SOURCE_PORT_RANGE=1" );
  asprintf( &data_array[2],
            "RIL_QOS_SPEC_INDEX=0,RIL_QOS_FLOW_DIRECTION=1,RIL_QOS_FLOW_DATA_RATE_MIN=64000,RIL_QOS_FLOW_DATA_RATE_MAX=64000,"
            "RIL_QOS_FILTER_INDEX=0,RIL_QOS_FILTER_DIRECTION=1,RIL_QOS_FILTER_IPVERSION=IP,"
            "RIL_QOS_FILTER_TCP_SOURCE_PORT_START=4000,RIL_QOS_FILTER_TCP_SOURCE_PORT_RANGE=1" );

  /* Setup expected info */
  qcril_data_client_info.exp_info.resp_type = QCRIL_STUB_RESPONSE_REQ;
  qcril_data_client_info.exp_info.resp.t = RIL_TOKEN_ID;
  qcril_data_client_info.exp_info.resp.request_id = RIL_REQUEST_MODIFY_QOS;
  qcril_data_client_info.exp_info.resp.ril_err_no = RIL_E_SUCCESS;
  qcril_data_client_info.exp_info.resp.resp_pkt = &call_resp;
  qcril_data_client_info.exp_info.resp.resp_len = sizeof(call_resp.qos_resp);
  qcril_data_client_info.exp_info.evt = DSI_EVT_QOS_STATUS_IND;
  qcril_data_client_info.exp_info.status_evt = DSI_QOS_MODIFY_ACCEPTED_EV;
  qcril_data_client_info.exp_info.flow_ID = QOS_FLOW_ID;
    
  asprintf( &call_resp.qos_resp.result.return_code, "%d", RIL_E_SUCCESS );
  
  exp_spec = &qcril_data_client_info.exp_info.qos_spec;
  memset( exp_spec, 0x0, sizeof(dsi_qos_spec_type) );
  exp_spec->tx_flow_req_array = malloc( sizeof(qmi_qos_flow_req_type) );
  exp_spec->rx_flow_req_array = malloc( sizeof(qmi_qos_flow_req_type) );
  exp_spec->tx_filter_req_array = malloc( sizeof(qmi_qos_filter_req_type) );
  exp_spec->rx_filter_req_array = malloc( sizeof(qmi_qos_filter_req_type) );
  memset( exp_spec->tx_flow_req_array, 0x0, sizeof(qmi_qos_flow_req_type) );
  memset( exp_spec->rx_flow_req_array, 0x0, sizeof(qmi_qos_flow_req_type) );
  memset( exp_spec->tx_filter_req_array, 0x0, sizeof(qmi_qos_filter_req_type) );
  memset( exp_spec->rx_filter_req_array, 0x0, sizeof(qmi_qos_filter_req_type) );

  exp_spec->num_tx_flow_req = 1;
  exp_spec->tx_flow_req_array->umts_flow_desc.param_mask |= QMI_QOS_UMTS_FLOW_PARAM_DATA_RATE;
  exp_spec->tx_flow_req_array->umts_flow_desc.data_rate.guaranteed_rate = 64000; 
  exp_spec->tx_flow_req_array->umts_flow_desc.data_rate.max_rate = 64000; 
  exp_spec->num_tx_filter_req = 1;
  exp_spec->tx_filter_req_array->ip_version = QMI_QOS_IP_VERSION_4;
  exp_spec->tx_filter_req_array->filter_desc.param_mask |= QMI_QOS_FILTER_PARAM_TCP_SRC_PORTS;
  exp_spec->tx_filter_req_array->filter_desc.tcp_src_ports.start_port = ntohs(4000);
  exp_spec->tx_filter_req_array->filter_desc.tcp_src_ports.range = 1;
  exp_spec->tx_filter_req_array->filter_desc.param_mask |= QMI_QOS_FILTER_PARAM_TRANS_PROTOCOL;
  exp_spec->tx_filter_req_array->filter_desc.protocol = QMI_QOS_TRANS_PROT_TCP;
    
  exp_spec->num_rx_flow_req = 1;
  exp_spec->rx_flow_req_array->umts_flow_desc.param_mask |= QMI_QOS_UMTS_FLOW_PARAM_DATA_RATE;
  exp_spec->rx_flow_req_array->umts_flow_desc.data_rate.guaranteed_rate = 64000; 
  exp_spec->rx_flow_req_array->umts_flow_desc.data_rate.max_rate = 64000; 
  exp_spec->num_rx_filter_req = 1;
  exp_spec->rx_filter_req_array->ip_version = QMI_QOS_IP_VERSION_4;
  exp_spec->rx_filter_req_array->filter_desc.param_mask |= QMI_QOS_FILTER_PARAM_TCP_SRC_PORTS;
  exp_spec->rx_filter_req_array->filter_desc.tcp_src_ports.start_port = ntohs(4000);
  exp_spec->rx_filter_req_array->filter_desc.tcp_src_ports.range = 1;
  exp_spec->rx_filter_req_array->filter_desc.param_mask |= QMI_QOS_FILTER_PARAM_TRANS_PROTOCOL;
  exp_spec->rx_filter_req_array->filter_desc.protocol = QMI_QOS_TRANS_PROT_TCP;
  
  /* Setup RIL command buffer */
  params_ptr->instance_id = RIL_INSTANCE_ID;
  params_ptr->modem_id = RIL_MODEM_ID;
  params_ptr->t = RIL_TOKEN_ID;
  params_ptr->event_id = RIL_REQUEST_MODIFY_QOS;
  params_ptr->data = (void*)data_array;
  params_ptr->datalen = sizeof(data_array);

  /* SETUP: Register with DSI_NetCtrl */
  dsi_init( DSI_MODE_GENERAL );
  info_tbl[ index ].dsi_hndl = dsi_cb_tbl[0].dsi_hndl = dsi_get_data_srvc_hndl( dsi_cb, (void*)&qcril_data_client_info );
    
  /* Process QOS command, validation is done in callback */
  qcril_data_request_modify_qos( params_ptr, response_ptr );

  /* SETUP: Release DSI_NetCtrl */
  dsi_rel_data_srvc_hndl( info_tbl[ index ].dsi_hndl );

  /* Release dynamic memory */
  free( flow_ptr );
  free( call_resp.qos_resp.result.return_code );
  free( exp_spec->tx_flow_req_array );
  free( exp_spec->rx_flow_req_array );
  free( exp_spec->tx_filter_req_array );
  free( exp_spec->rx_filter_req_array );
  free( data_array[0] );
  free( data_array[1] );
  free( data_array[2] );
  free( response_ptr );
  free( params_ptr );
  
  return qcril_data_client_info.testcase_status;
} /* qcril_data_test_qos_modify_umts_sucessful1 */

int qcril_data_test_qos_modify_umts_failure1( void* arg )
{
  qcril_request_params_type *params_ptr = NULL;
  qcril_request_return_type *response_ptr = NULL;
  static qcril_data_call_response_type  call_resp;
  char * data_array[3];
  unsigned int  index = 0;
  RIL_Token   token = RIL_TOKEN_ID;
  dsi_qos_spec_type  *exp_spec = NULL;
  qcril_data_qos_state_type *flow_ptr = NULL;
  
  /* Setup initial state */
  memset( info_tbl, 0x0, sizeof(info_tbl) );
  info_tbl[ index ].instance_id = RIL_INSTANCE_ID;
  info_tbl[ index ].modem_id = RIL_MODEM_ID;
  info_tbl[ index ].index = index;
  info_tbl[ index ].cid = RIL_CALL_ID;
  info_tbl[ index ].self = &info_tbl[ index ];
  dsi_cb_tbl[0].info_tbl_ptr = &info_tbl[ index ];
  flow_ptr = (qcril_data_qos_state_type*)malloc( sizeof(qcril_data_qos_state_type) );
  flow_ptr->flow_id = QOS_FLOW_ID;
  list_push_back( &info_tbl[ index ].qos_flow_list, &flow_ptr->link );
    
  /* Allocate RIL command and response buffers */
  params_ptr = malloc( sizeof(qcril_request_params_type) );
  assert( params_ptr );
  memset( params_ptr, 0x0, sizeof(qcril_request_params_type) );
  response_ptr = malloc( sizeof(qcril_request_return_type) );
  assert( response_ptr ); 
  memset( response_ptr, 0x0, sizeof(qcril_request_return_type) );
  
  qcril_data_client_info.params_ptr = params_ptr;
  qcril_data_client_info.response_ptr = response_ptr;
  qcril_data_client_info.testcase_status = QCRIL_DATA_FAILURE;

  /* Generate RIL QOS specification */
  asprintf( &data_array[0], "0x%08x", QOS_FLOW_ID );
  asprintf( &data_array[1],
            "RIL_QOS_SPEC_INDEX=0,RIL_QOS_FLOW_DIRECTION=0,RIL_QOS_FLOW_DATA_RATE_MIN=64000,RIL_QOS_FLOW_DATA_RATE_MAX=64000,"
            "RIL_QOS_FILTER_INDEX=0,RIL_QOS_FILTER_DIRECTION=0,RIL_QOS_FILTER_IPVERSION=IP,"
            "RIL_QOS_FILTER_TCP_SOURCE_PORT_START=4000,RIL_QOS_FILTER_TCP_SOURCE_PORT_RANGE=1" );
  asprintf( &data_array[2],
            "RIL_QOS_SPEC_INDEX=0,RIL_QOS_FLOW_DIRECTION=1,RIL_QOS_FLOW_DATA_RATE_MIN=64000,RIL_QOS_FLOW_DATA_RATE_MAX=64000,"
            "RIL_QOS_FILTER_INDEX=0,RIL_QOS_FILTER_DIRECTION=1,RIL_QOS_FILTER_IPVERSION=IP,"
            "RIL_QOS_FILTER_TCP_SOURCE_PORT_START=4000,RIL_QOS_FILTER_TCP_SOURCE_PORT_RANGE=1" );

  /* Setup expected info */
  qcril_data_client_info.exp_info.resp_type = QCRIL_STUB_RESPONSE_REQ;
  qcril_data_client_info.exp_info.resp.t = RIL_TOKEN_ID;
  qcril_data_client_info.exp_info.resp.request_id = RIL_REQUEST_MODIFY_QOS;
  qcril_data_client_info.exp_info.resp.ril_err_no = RIL_E_SUCCESS;
  qcril_data_client_info.exp_info.resp.resp_pkt = &call_resp;
  qcril_data_client_info.exp_info.resp.resp_len = sizeof(call_resp.qos_resp);
  qcril_data_client_info.exp_info.evt = DSI_EVT_QOS_STATUS_IND;
  qcril_data_client_info.exp_info.status_evt = DSI_QOS_MODIFY_REJECTED_EV;
  qcril_data_client_info.exp_info.flow_ID = QOS_FLOW_ID;
    
  asprintf( &call_resp.qos_resp.result.return_code, "%d", RIL_E_SUCCESS );
  
  exp_spec = &qcril_data_client_info.exp_info.qos_spec;
  memset( exp_spec, 0x0, sizeof(dsi_qos_spec_type) );
  exp_spec->tx_flow_req_array = malloc( sizeof(qmi_qos_flow_req_type) );
  exp_spec->rx_flow_req_array = malloc( sizeof(qmi_qos_flow_req_type) );
  exp_spec->tx_filter_req_array = malloc( sizeof(qmi_qos_filter_req_type) );
  exp_spec->rx_filter_req_array = malloc( sizeof(qmi_qos_filter_req_type) );
  memset( exp_spec->tx_flow_req_array, 0x0, sizeof(qmi_qos_flow_req_type) );
  memset( exp_spec->rx_flow_req_array, 0x0, sizeof(qmi_qos_flow_req_type) );
  memset( exp_spec->tx_filter_req_array, 0x0, sizeof(qmi_qos_filter_req_type) );
  memset( exp_spec->rx_filter_req_array, 0x0, sizeof(qmi_qos_filter_req_type) );

  exp_spec->num_tx_flow_req = 1;
  exp_spec->tx_flow_req_array->umts_flow_desc.param_mask |= QMI_QOS_UMTS_FLOW_PARAM_DATA_RATE;
  exp_spec->tx_flow_req_array->umts_flow_desc.data_rate.guaranteed_rate = 64000; 
  exp_spec->tx_flow_req_array->umts_flow_desc.data_rate.max_rate = 64000; 
  exp_spec->num_tx_filter_req = 1;
  exp_spec->tx_filter_req_array->ip_version = QMI_QOS_IP_VERSION_4;
  exp_spec->tx_filter_req_array->filter_desc.param_mask |= QMI_QOS_FILTER_PARAM_TCP_SRC_PORTS;
  exp_spec->tx_filter_req_array->filter_desc.tcp_src_ports.start_port = ntohs(4000);
  exp_spec->tx_filter_req_array->filter_desc.tcp_src_ports.range = 1;
  exp_spec->tx_filter_req_array->filter_desc.param_mask |= QMI_QOS_FILTER_PARAM_TRANS_PROTOCOL;
  exp_spec->tx_filter_req_array->filter_desc.protocol = QMI_QOS_TRANS_PROT_TCP;
    
  exp_spec->num_rx_flow_req = 1;
  exp_spec->rx_flow_req_array->umts_flow_desc.param_mask |= QMI_QOS_UMTS_FLOW_PARAM_DATA_RATE;
  exp_spec->rx_flow_req_array->umts_flow_desc.data_rate.guaranteed_rate = 64000; 
  exp_spec->rx_flow_req_array->umts_flow_desc.data_rate.max_rate = 64000; 
  exp_spec->num_rx_filter_req = 1;
  exp_spec->rx_filter_req_array->ip_version = QMI_QOS_IP_VERSION_4;
  exp_spec->rx_filter_req_array->filter_desc.param_mask |= QMI_QOS_FILTER_PARAM_TCP_SRC_PORTS;
  exp_spec->rx_filter_req_array->filter_desc.tcp_src_ports.start_port = ntohs(4000);
  exp_spec->rx_filter_req_array->filter_desc.tcp_src_ports.range = 1;
  exp_spec->rx_filter_req_array->filter_desc.param_mask |= QMI_QOS_FILTER_PARAM_TRANS_PROTOCOL;
  exp_spec->rx_filter_req_array->filter_desc.protocol = QMI_QOS_TRANS_PROT_TCP;
  
  /* Setup RIL command buffer */
  params_ptr->instance_id = RIL_INSTANCE_ID;
  params_ptr->modem_id = RIL_MODEM_ID;
  params_ptr->t = RIL_TOKEN_ID;
  params_ptr->event_id = RIL_REQUEST_MODIFY_QOS;
  params_ptr->data = (void*)data_array;
  params_ptr->datalen = sizeof(data_array);

  /* SETUP: Register with DSI_NetCtrl */
  dsi_init( DSI_MODE_GENERAL );
  info_tbl[ index ].dsi_hndl = dsi_cb_tbl[0].dsi_hndl = dsi_get_data_srvc_hndl( dsi_cb, (void*)&qcril_data_client_info );
  qcril_data_client_info.handle = -3; /* key for stub */
  
  /* Process QOS command, validation is done in callback */
  qcril_data_request_modify_qos( params_ptr, response_ptr );

  /* SETUP: Release DSI_NetCtrl */
  dsi_rel_data_srvc_hndl( info_tbl[ index ].dsi_hndl );

  /* Release dynamic memory */
  free( flow_ptr );
  free( call_resp.qos_resp.result.return_code );
  free( exp_spec->tx_flow_req_array );
  free( exp_spec->rx_flow_req_array );
  free( exp_spec->tx_filter_req_array );
  free( exp_spec->rx_filter_req_array );
  free( data_array[0] );
  free( data_array[1] );
  free( data_array[2] );
  free( response_ptr );
  free( params_ptr );
  
  return qcril_data_client_info.testcase_status;
} /* qcril_data_test_qos_modify_umts_failure1 */

int qcril_data_test_qos_create_umts_multi_filter_sucessful1( void* arg )
{
  qcril_request_params_type *params_ptr = NULL;
  qcril_request_return_type *response_ptr = NULL;
  char * data_array[3];
  unsigned int  index = 0;
  RIL_Token   token = RIL_TOKEN_ID;
  dsi_qos_spec_type  *exp_spec = NULL;
  static qcril_data_call_response_type  call_resp;

  /* Setup initial state */
  memset( info_tbl, 0x0, sizeof(info_tbl) );
  info_tbl[ index ].instance_id = RIL_INSTANCE_ID;
  info_tbl[ index ].modem_id = RIL_MODEM_ID;
  info_tbl[ index ].index = index;
  info_tbl[ index ].cid = RIL_CALL_ID;
  info_tbl[ index ].self = &info_tbl[ index ];
  dsi_cb_tbl[0].info_tbl_ptr = &info_tbl[ index ];

  /* Allocate RIL command and response buffers */
  params_ptr = malloc( sizeof(qcril_request_params_type) );
  assert( params_ptr );
  memset( params_ptr, 0x0, sizeof(qcril_request_params_type) );
  response_ptr = malloc( sizeof(qcril_request_return_type) );
  assert( response_ptr ); 
  memset( response_ptr, 0x0, sizeof(qcril_request_return_type) );

  qcril_data_client_info.params_ptr = params_ptr;
  qcril_data_client_info.response_ptr = response_ptr;
  qcril_data_client_info.testcase_status = QCRIL_DATA_FAILURE;

  /* Generate RIL QOS specification */
  asprintf( &data_array[0], "%d", RIL_CALL_ID );
  asprintf( &data_array[1],
            "RIL_QOS_SPEC_INDEX=0,RIL_QOS_FLOW_DIRECTION=0,RIL_QOS_FLOW_DATA_RATE_MIN=64000,RIL_QOS_FLOW_DATA_RATE_MAX=64000,"
            "RIL_QOS_FILTER_INDEX=0,RIL_QOS_FILTER_DIRECTION=0,RIL_QOS_FILTER_IPVERSION=IP,"
            "RIL_QOS_FILTER_TCP_SOURCE_PORT_START=4000,RIL_QOS_FILTER_TCP_SOURCE_PORT_RANGE=1,"
            "RIL_QOS_FILTER_INDEX=1,RIL_QOS_FILTER_DIRECTION=0,RIL_QOS_FILTER_IPVERSION=IP,"
            "RIL_QOS_FILTER_TCP_SOURCE_PORT_START=6000,RIL_QOS_FILTER_TCP_SOURCE_PORT_RANGE=10,"
            "RIL_QOS_FILTER_INDEX=2,RIL_QOS_FILTER_DIRECTION=0,RIL_QOS_FILTER_IPVERSION=IP,"
            "RIL_QOS_FILTER_TCP_SOURCE_PORT_START=8000,RIL_QOS_FILTER_TCP_SOURCE_PORT_RANGE=20" );
#define NUM_TX_FILTER (3)
  asprintf( &data_array[2],
            "RIL_QOS_SPEC_INDEX=0,RIL_QOS_FLOW_DIRECTION=1,RIL_QOS_FLOW_DATA_RATE_MIN=64000,RIL_QOS_FLOW_DATA_RATE_MAX=64000,"
            "RIL_QOS_FILTER_INDEX=0,RIL_QOS_FILTER_DIRECTION=1,RIL_QOS_FILTER_IPVERSION=IP,"
            "RIL_QOS_FILTER_TCP_SOURCE_PORT_START=4000,RIL_QOS_FILTER_TCP_SOURCE_PORT_RANGE=1" );
#define NUM_RX_FILTER (1)

  /* Setup expected info */
  qcril_data_client_info.exp_info.resp_type = QCRIL_STUB_RESPONSE_REQ;
  qcril_data_client_info.exp_info.resp.t = RIL_TOKEN_ID;
  qcril_data_client_info.exp_info.resp.request_id = RIL_REQUEST_SETUP_QOS;
  qcril_data_client_info.exp_info.resp.ril_err_no = RIL_E_SUCCESS;
  qcril_data_client_info.exp_info.evt = DSI_EVT_QOS_STATUS_IND;
  qcril_data_client_info.exp_info.status_evt = DSI_QOS_ACTIVATED_EV;
  qcril_data_client_info.exp_info.flow_ID = QOS_FLOW_ID;
  qcril_data_client_info.exp_info.resp.resp_pkt = &call_resp;
  qcril_data_client_info.exp_info.resp.resp_len = sizeof(call_resp.qos_resp);

  asprintf( &call_resp.qos_resp.setup.return_code, "%d", RIL_E_SUCCESS );

  exp_spec = &qcril_data_client_info.exp_info.qos_spec;
  memset( exp_spec, 0x0, sizeof(dsi_qos_spec_type) );
  exp_spec->tx_flow_req_array = malloc( sizeof(qmi_qos_flow_req_type) );
  exp_spec->rx_flow_req_array = malloc( sizeof(qmi_qos_flow_req_type) );
  exp_spec->tx_filter_req_array = malloc( sizeof(qmi_qos_filter_req_type)*NUM_TX_FILTER );
  exp_spec->rx_filter_req_array = malloc( sizeof(qmi_qos_filter_req_type)*NUM_RX_FILTER );
  memset( exp_spec->tx_flow_req_array, 0x0, sizeof(qmi_qos_flow_req_type) );
  memset( exp_spec->rx_flow_req_array, 0x0, sizeof(qmi_qos_flow_req_type) );
  memset( exp_spec->tx_filter_req_array, 0x0, sizeof(qmi_qos_filter_req_type) );
  memset( exp_spec->rx_filter_req_array, 0x0, sizeof(qmi_qos_filter_req_type) );

  exp_spec->num_tx_flow_req = 1;
  exp_spec->tx_flow_req_array->umts_flow_desc.param_mask |= QMI_QOS_UMTS_FLOW_PARAM_DATA_RATE;
  exp_spec->tx_flow_req_array->umts_flow_desc.data_rate.guaranteed_rate = 64000; 
  exp_spec->tx_flow_req_array->umts_flow_desc.data_rate.max_rate = 64000; 
  exp_spec->num_tx_filter_req = NUM_TX_FILTER;
  exp_spec->tx_filter_req_array[0].ip_version = QMI_QOS_IP_VERSION_4;
  exp_spec->tx_filter_req_array[0].filter_desc.param_mask |= QMI_QOS_FILTER_PARAM_TCP_SRC_PORTS;
  exp_spec->tx_filter_req_array[0].filter_desc.tcp_src_ports.start_port = ntohs(4000);
  exp_spec->tx_filter_req_array[0].filter_desc.tcp_src_ports.range = 1;
  exp_spec->tx_filter_req_array[0].filter_desc.param_mask |= QMI_QOS_FILTER_PARAM_TRANS_PROTOCOL;
  exp_spec->tx_filter_req_array[0].filter_desc.protocol = QMI_QOS_TRANS_PROT_TCP;
  exp_spec->tx_filter_req_array[1].ip_version = QMI_QOS_IP_VERSION_4;
  exp_spec->tx_filter_req_array[1].filter_desc.param_mask |= QMI_QOS_FILTER_PARAM_TCP_SRC_PORTS;
  exp_spec->tx_filter_req_array[1].filter_desc.tcp_src_ports.start_port = ntohs(6000);
  exp_spec->tx_filter_req_array[1].filter_desc.tcp_src_ports.range = 10;
  exp_spec->tx_filter_req_array[1].filter_desc.param_mask |= QMI_QOS_FILTER_PARAM_TRANS_PROTOCOL;
  exp_spec->tx_filter_req_array[1].filter_desc.protocol = QMI_QOS_TRANS_PROT_TCP;
  exp_spec->tx_filter_req_array[2].ip_version = QMI_QOS_IP_VERSION_4;
  exp_spec->tx_filter_req_array[2].filter_desc.param_mask |= QMI_QOS_FILTER_PARAM_TCP_SRC_PORTS;
  exp_spec->tx_filter_req_array[2].filter_desc.tcp_src_ports.start_port = ntohs(8000);
  exp_spec->tx_filter_req_array[2].filter_desc.tcp_src_ports.range = 20;
  exp_spec->tx_filter_req_array[2].filter_desc.param_mask |= QMI_QOS_FILTER_PARAM_TRANS_PROTOCOL;
  exp_spec->tx_filter_req_array[2].filter_desc.protocol = QMI_QOS_TRANS_PROT_TCP;

  exp_spec->num_rx_flow_req = 1;
  exp_spec->rx_flow_req_array->umts_flow_desc.param_mask |= QMI_QOS_UMTS_FLOW_PARAM_DATA_RATE;
  exp_spec->rx_flow_req_array->umts_flow_desc.data_rate.guaranteed_rate = 64000; 
  exp_spec->rx_flow_req_array->umts_flow_desc.data_rate.max_rate = 64000; 
  exp_spec->num_rx_filter_req = NUM_RX_FILTER;
  exp_spec->rx_filter_req_array->ip_version = QMI_QOS_IP_VERSION_4;
  exp_spec->rx_filter_req_array->filter_desc.param_mask |= QMI_QOS_FILTER_PARAM_TCP_SRC_PORTS;
  exp_spec->rx_filter_req_array->filter_desc.tcp_src_ports.start_port = ntohs(4000);
  exp_spec->rx_filter_req_array->filter_desc.tcp_src_ports.range = 1;
  exp_spec->rx_filter_req_array->filter_desc.param_mask |= QMI_QOS_FILTER_PARAM_TRANS_PROTOCOL;
  exp_spec->rx_filter_req_array->filter_desc.protocol = QMI_QOS_TRANS_PROT_TCP;

  /* Setup RIL command buffer */
  params_ptr->instance_id = RIL_INSTANCE_ID;
  params_ptr->modem_id = RIL_MODEM_ID;
  params_ptr->t = RIL_TOKEN_ID;
  params_ptr->event_id = RIL_REQUEST_SETUP_QOS;
  params_ptr->data = (void*)data_array;
  params_ptr->datalen = sizeof(data_array);

  /* SETUP: Register with DSI_NetCtrl */
  dsi_init( DSI_MODE_GENERAL );
  info_tbl[ index ].dsi_hndl = dsi_cb_tbl[0].dsi_hndl = dsi_get_data_srvc_hndl( dsi_cb, (void*)&qcril_data_client_info );

  /* Process QOS command, validation is done in callback */
  qcril_data_request_setup_qos( params_ptr, response_ptr );

  /* SETUP: Release DSI_NetCtrl */
  dsi_rel_data_srvc_hndl( info_tbl[ index ].dsi_hndl );

  /* Release dynamic memory */
  free( exp_spec->tx_flow_req_array );
  free( exp_spec->rx_flow_req_array );
  free( exp_spec->tx_filter_req_array );
  free( exp_spec->rx_filter_req_array );
  free( data_array[0] );
  free( data_array[1] );
  free( data_array[2] );
  free( response_ptr );
  free( params_ptr );

  return qcril_data_client_info.testcase_status;
} /* qcril_data_test_qos_create_umts_multi_filter_sucessful1 */


void client_net_ev_cb( qcril_data_hndl_t         hndl,
                       void                     *user_data,
                       qcril_data_net_evt_t      evt,
                       qcril_data_evt_payload_t *payload )
{
  fprintf(stderr, "%s: hndl[%08x] user_data[%p] evt[%d] payload[%p]\n",
          __func__, hndl, user_data, evt, payload);
}

int qcril_data_test_client_reg_dereg( void* arg )
{
  qcril_data_hndl_t hndl = QCRIL_DATA_CLIENT_HNDL_INVALID;

  /* Initialize client module */
  qcril_data_client_init();
  
  /* Verify NULL callback results in error */
  hndl = qcril_data_client_register( NULL, NULL );
  if( QCRIL_DATA_CLIENT_HNDL_INVALID != hndl )
  {
    fprintf(stderr, "Failed on client_register for NULL callback\n");
    return QCRIL_DATA_FAILURE;
  }

  hndl = qcril_data_client_register( client_net_ev_cb, 0x01020304 );
  if( QCRIL_DATA_CLIENT_HNDL_INVALID == hndl )
  {
    fprintf(stderr, "Failed on client_register for NULL callback\n");
    return QCRIL_DATA_FAILURE;
  }
  
  qcril_data_client_release( hndl );

  return QCRIL_DATA_SUCCESS;
} /* qcril_data_test_client_reg_dereg */


int qcril_data_test_client_get_calls( void* arg )
{
  pthread_mutexattr_t info_tbl_mutex_attr;
  qcril_data_hndl_t hndl = QCRIL_DATA_CLIENT_HNDL_INVALID;
  qcril_data_active_call_info_t call_list[QCRIL_DATA_MAX_CALL_RECORDS];
  unsigned int  num_calls = 0;
  int ret, i;
#define CALLID0 (1)
#define CALLID1 (2)
  
  memset( call_list, 0x0, sizeof(call_list) );
  
  /* Initialize client module */
  qcril_data_client_init();

  /* Setup a few calls */
  fprintf(stderr, "Configuring test data\n");
  pthread_mutexattr_init(&info_tbl_mutex_attr);
  pthread_mutexattr_settype(&info_tbl_mutex_attr, PTHREAD_MUTEX_RECURSIVE);
  pthread_mutex_init(&info_tbl_mutex, &info_tbl_mutex_attr);
  memset( info_tbl, -1, sizeof(info_tbl) );
  info_tbl[0].index       = CALLID0;
  info_tbl[0].cid         = CALLID0;
  info_tbl[0].self        = &info_tbl[0];
  strncpy(info_tbl[0].call_info.apn, "APN-0", sizeof(info_tbl[0].call_info.apn));
  
  info_tbl[2].index       = CALLID1;
  info_tbl[2].cid         = CALLID1;
  info_tbl[2].self        = &info_tbl[2];
  strncpy(info_tbl[2].call_info.apn, "APN-2", sizeof(info_tbl[2].call_info.apn));
  
  /* Verify NULL callback results in error */
  hndl = qcril_data_client_register( client_net_ev_cb, 0x01020304 );
  if( QCRIL_DATA_CLIENT_HNDL_INVALID == hndl )
  {
    fprintf(stderr, "Failed on client_register for NULL callback\n");
    return QCRIL_DATA_FAILURE;
  }

  fprintf(stderr, "Query active calls\n");
  ret = qcril_data_get_active_calls( hndl, &num_calls, call_list );
  if( QCRIL_DATA_FAILURE == ret )
  {
    fprintf(stderr, "Failed on get_active_calls\n");
    return QCRIL_DATA_FAILURE;
  }

  /* Check results */
  if( (call_list[0].call_id != CALLID0) || (call_list[1].call_id != CALLID1) )
  {
    fprintf(stderr, "Failed on results validation\n");
    return QCRIL_DATA_FAILURE;
  }
  else
  {
    for(i=0; i<num_calls; i++)
    {
      fprintf(stderr, "\tCallID[%d] APN[%s]\n",call_list[i].call_id, call_list[i].apn );
    }
  }
  qcril_data_client_release( hndl );

  return QCRIL_DATA_SUCCESS;
} /* qcril_data_test_client_get_calls */


/*--------------------------------------------------------------------------- 
  Test Framework
---------------------------------------------------------------------------*/


qcril_data_testcase_t qcril_data_tests[] =
{
  { "Register & deregister client",
    qcril_data_test_client_reg_dereg },

  { "Request simple QOS flow/filter for UMTS successful",
    qcril_data_test_qos_create_umts_sucessful1 },

  { "Request full QOS flow/filter for UMTS with IPv4 address successful",
    qcril_data_test_qos_create_umts_sucessful2 },
  
  { "Request simple QOS flow/filter for UMTS failure",
    qcril_data_test_qos_create_umts_failure1 },

  { "Request QOS release successful",
    qcril_data_test_qos_release_sucessful1 },
  
  { "Request QOS release with bad flowID failure",
    qcril_data_test_qos_release_failure1 },

  { "Get status & granted QOS successful",
    qcril_data_test_qos_getstatus_sucessful1 },

  { "Get status & granted QOS failure",
    qcril_data_test_qos_getstatus_failure1 },

  { "Request QOS suspend successful",
    qcril_data_test_qos_suspend_sucessful1 },
  
  { "Request QOS suspend failure",
    qcril_data_test_qos_suspend_failure1 },

  { "Request QOS resume successful",
    qcril_data_test_qos_resume_sucessful1 },

  { "Request modify QOS flow for UMTS successful",
    qcril_data_test_qos_modify_umts_sucessful1 },

  { "Request modify QOS flow for UMTS failure",
    qcril_data_test_qos_modify_umts_failure1 },

  { "Request full QOS flow/filter for UMTS with IPv6 address successful",
    qcril_data_test_qos_create_umts_ipv6_sucessful },

  { "Request QOS flow with multiple filters for UMTS successful",
    qcril_data_test_qos_create_umts_multi_filter_sucessful1 },
  
  { "Request QOS flow/multi TX filter for UMTS successful",
    qcril_data_test_qos_create_umts_sucessful3 },

  { "Query active calls",
    qcril_data_test_client_get_calls },
};

int qcril_data_test_execute( void )
{
  boolean result = QCRIL_DATA_FAILURE;
  int i, cnt, cnt_pass = 0;

  qcril_stubs_init( stub_response_cb, NULL );
    
  cnt = ds_arrsize( qcril_data_tests );
  fprintf(stderr, "=== Testcases: %d ===\n", cnt );

  for( i=0; i<cnt; i++) {
    qcril_data_test_info.current = &qcril_data_tests[i];
    fprintf(stderr, "***Executing test[%d]: %s\n",
                    i, qcril_data_tests[i].description );

    if( qcril_data_tests[i].testcase ) {
      result = qcril_data_tests[i].testcase( NULL );
    }
    /* Ensure dynamic memory cleanup */
    qcril_data_test_reset();
    
    cnt_pass += (QCRIL_DATA_SUCCESS==result)? 1 : 0;
    fprintf(stderr, "***Result[%d]: %s\n",
                    i, (QCRIL_DATA_SUCCESS==result)? "PASS" : "FAIL" );

    fprintf(stderr, "==========================================\n");
  }
  
  fprintf(stderr, "=== Testcases: %d  PASS: %d(%5.1f%%) FAIL: %d(%5.1f%%) ===\n",
                  cnt, cnt_pass, (cnt_pass*1.0/cnt)*100, (cnt-cnt_pass), (1-(cnt_pass*1.0/cnt))*100 );

  qcril_stubs_release();
  
  return result;
}

void qcril_data_test_init( void )
{
  pthread_mutexattr_t  mattr;

  /* Initialize the mutex and condition variables */
  (void)pthread_mutexattr_init(&mattr);
  (void)pthread_mutexattr_settype(&mattr,PTHREAD_MUTEX_RECURSIVE_NP);
  (void)pthread_mutex_init(&qcril_data_test_info.mutx, &mattr);
  (void)pthread_cond_init(&qcril_data_test_info.cond, NULL);
}

void qcril_data_test_teardown( void )
{
}

int main( int argc, const char* argv[] )
{
  int rc = 0;
  
  /* Initialize test framemwork */
  qcril_data_test_init();

  /* Launch test suite */
  rc = qcril_data_test_execute();
    
  /* Teardown test framemwork */
  qcril_data_test_teardown();
  
  fprintf(stderr, "=== Done: rc=%d ===\n", rc );
  return rc;
}

#endif /* QCRIL_DATA_TEST */
