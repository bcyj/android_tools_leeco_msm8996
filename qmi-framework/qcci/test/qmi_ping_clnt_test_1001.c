/******************************************************************************
  -----------------------------------------------------------------------------
  Copyright (c) 2011-2012 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  -----------------------------------------------------------------------------

******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "qmi_cci_target.h"
#include "qmi_client.h"
#include "qmi_idl_lib.h"
#include "qmi_cci_common.h"
#include "qmi_ping_clnt_common_stats.h"
#include "qmi_ping_clnt_common.h"
#include "qmi_ping_api_v01.h"

extern ping_error_cb(qmi_client_type clnt, qmi_client_error_type error, void *error_cb_data);
/*===========================================================================
  FUNCTION  qmi_ping_test_main
===========================================================================*/
/*!
@brief
  Main body of qmi ping clnt test

@return
  0 for pass, -1 for fail

@note
  - Dependencies
    - N/A
  - Side Effects
*/
/*=========================================================================*/
static int qmi_ping_test_main(int test_length, int test_duration_ms)
{
  qmi_txn_handle txn;
  uint32_t num_services, num_entries=0;
  int rc, service_connect;
  qmi_client_type clnt, notifier;
  qmi_cci_os_signal_type os_params;
  qmi_service_info info[10];
  int data_size_bytes = 1;
  qmi_ping_clnt_results_type results;

  qmi_ping_clnt_common_print_stats(&results, QMI_PING_SINGLE_LINE_HEADER_PRINT_FORMAT);
  /* Get the service object for the ping API */
  qmi_idl_service_object_type ping_service_object = ping_get_service_object_v01();
  if (!ping_service_object)
  {
    printf("PING: ping_get_serivce_object failed, verify qmi_ping_api_v01.h and .c match.\n");
    return -1;
  }

ssr_retry:
  rc = qmi_client_notifier_init(ping_service_object, &os_params, &notifier);
  /* Check if the service is up, if not wait on a signal */
  while(1)
  {
    rc = qmi_client_get_service_list( ping_service_object, NULL, NULL, &num_services);
    if(rc == QMI_NO_ERR)
      break;
    /* wait for server to come up */
    QMI_CCI_OS_SIGNAL_WAIT(&os_params, 0);
  }

  num_entries = num_services;
  /* The server has come up, store the information in info variable */
  rc = qmi_client_get_service_list( ping_service_object, info, &num_entries, &num_services);
  service_connect = 0;
  if (num_services > 1)
    service_connect = (rand() % num_services);

  rc = qmi_client_init(&info[service_connect], ping_service_object, NULL, NULL, NULL, &clnt);
  qmi_client_register_error_cb(clnt, ping_error_cb, NULL);

  for (;data_size_bytes < PING_MAX_DATA_SIZE_V01; data_size_bytes = (data_size_bytes << 1))
  {
    rc |= qmi_ping_clnt_common_sync_data_test(&clnt, test_length, data_size_bytes, test_duration_ms, 0, &results);
    if (rc)
    {
      qmi_client_release(clnt);
      goto ssr_retry;
    }
    qmi_ping_clnt_common_print_stats(&results, QMI_PING_SINGLE_LINE_PRINT_FORMAT);
  }

  rc = qmi_client_release(clnt);
  printf("PING: qmi_client_release of clnt returned %d\n", rc);

  rc = qmi_client_release(notifier);
  printf("PING: qmi_client_release of notifier returned %d\n", rc);
  return(rc);
}

/*===========================================================================
  FUNCTION  main
===========================================================================*/
/*!
@brief
  program enty

@return
  rc

@note
  - Dependencies
    - N/A
  - Side Effects
*/
/*=========================================================================*/
int main(int argc, char *argv[])
{
  uint32_t test_length;
  uint32_t test_duration_ms;
  int rc;
  printf("Usage: qmi_ping_clnt_test_0000 test_duration_ms(2000 msec) test_length(0) \n");
  printf("- To run for a time period, specify test_duration_ms and omit test_length \n");
  printf("- To run for a number of QMI txns, specify test_duration_ms = 0 \n   and specify test_length \n");

 /* Setup defaults */
  test_length = 0;
  test_duration_ms=500;

  switch(argc)
  {
    case 3:
      test_length = atoi(argv[2]);
    case 2:
      test_duration_ms = atoi(argv[1]);
      break;
    default:
      printf("Using defaults test_length:%d, test_duration_ms:%d\n",
             (unsigned int)test_length , (unsigned int)test_duration_ms);
  }

  printf("\n\n\nPING TEST 1001 STARTED...\n");
  printf("Test parameters\n");
  printf("   Duration     : %6d ms\n",(int)test_duration_ms);
  if(test_length)
  {
    printf("   Test Length  : %6d \n",(int)test_length);
  }
  printf("\n");

  rc =  qmi_ping_test_main(test_length, test_duration_ms);
  if( rc == 0 )
  {
    printf("PASS\n");
  }
  else
  {
    printf("FAIL\n");
  }

  printf("Sleeping before exit ...\n");
  sleep(1);
  return(rc);
}
