/******************************************************************************

                  Q C R I L _ D A T A _ T E S T . H
 
******************************************************************************/

/******************************************************************************

  @file    qcril_data_test.h
  @brief   QCRIL Data tests Header File

  DESCRIPTION
  Header file for QCRIL_Data test functions.

******************************************************************************/
/*===========================================================================

  Copyright (c) 2011 Qualcomm Technologies, Inc. All Rights Reserved

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

#ifndef __QCRIL_DATA_TEST_H__
#define __QCRIL_DATA_TEST_H__

#include <pthread.h>
#include "ds_util.h"
#include "qcril_data_stubs.h"

/*===========================================================================
                     GLOBAL DEFINITIONS AND DECLARATIONS
===========================================================================*/
#define QCRIL_DATA_SUCCESS (0)
#define QCRIL_DATA_FAILURE (-1)

#define qcril_data_log_err   ds_log_err
#define qcril_data_log_high  ds_log_high
#define qcril_data_log_med   ds_log_med
#define qcril_data_log_low   ds_log_low



#define MAX_DESCRIPTION_SIZ  80

typedef int (*qcril_data_testcase_cb)(void*);

typedef struct qcril_data_testcase_s {
  char                description[MAX_DESCRIPTION_SIZ];
  qcril_data_testcase_cb  testcase;
} qcril_data_testcase_t;


typedef struct qcril_data_test_info_s {
  qcril_data_testcase_t  *current;
  pthread_cond_t      cond;   /* Condition variable for signaling */
  pthread_mutex_t     mutx;   /* Mutex required by condition var */
} qcril_data_test_info_t;

typedef struct client_info_s
{
  dsi_hndl_t          handle;
  qcril_request_params_type *params_ptr;
  qcril_request_return_type *response_ptr;
  struct {
    dsi_net_evt_t       evt;
    dsi_qos_status_event_type status_evt;
    dsi_qos_id_type     flow_ID;
    RIL_QosIndStates    qos_result;
    dsi_qos_spec_type   qos_spec;
    qcril_stub_response_type resp_type;
    qcril_request_resp_params_type resp;    
  } exp_info;
  int   testcase_status;
} qcril_data_test_client_info_t;


int qcril_data_test_execute( void );

void qcril_data_test_init( void );


/*===========================================================================
  FUNCTION  qcril_data_client_thread_wait
===========================================================================*/
/*!
@brief
  Function to wait on client event listener thread.
  THIS IS FOR TEST PURPOSES ONLY.

@return
  int - QCRIL_DATA_SUCCESS on successful operation, QCRIL_DATA_FAILURE otherwise

@note

  - Dependencies
    - None  

  - Side Effects
    - None
*/
/*=========================================================================*/
int qcril_data_client_thread_wait( void );

#endif /* __QCRIL_DATA_TEST_H__ */
