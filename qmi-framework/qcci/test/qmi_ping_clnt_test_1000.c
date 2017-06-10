/******************************************************************************
  -----------------------------------------------------------------------------
  Copyright (c) 2011-2012, 2014 Qualcomm Technologies, Inc.
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
static int qmi_ping_test_main(qmi_client_type *clnt, int test_length, int data_size_bytes, int test_duration_ms)
{
  uint32_t rc;
  qmi_ping_clnt_results_type results;

  printf("\n\n\nPING TEST STARTED...\n");
  rc = qmi_ping_clnt_common_sync_data_test(clnt, test_length, data_size_bytes, test_duration_ms, 1, &results);
  if(rc == 0)
  {
    printf("Test Passed\n");
    printf("Performance: \n");
    qmi_ping_clnt_common_print_stats(&results, QMI_PING_STANDARD_PRINT_FORMAT);
  }
  else
  {
    printf("Test Failed, \n");
  }
  printf("PING TEST COMPLETE...\n");

  return(rc );
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
  uint32_t data_size_bytes;
  uint32_t test_duration_ms;
  qmi_txn_handle txn;
  uint32_t num_services, num_entries=0;
  int rc,service_connect;
  qmi_client_type clnt, notifier;
  qmi_cci_os_signal_type os_params;
  qmi_service_info info[10];

  printf("Usage: qmi_ping_clnt_test_0000 test_duration_ms(2000 msec) data_size_bytes(1000) test_length(0) server(0)\n");
  printf("- To run for a time period, specify test_duration_ms and omit test_length \n");
  printf("- To run for a number of QMI txns, specify test_duration_ms = 0 \n   and specify test_length \n");

 /* Setup defaults */
  test_length = 0;
  data_size_bytes = (rand() % PING_MAX_DATA_SIZE_V01);
  test_duration_ms=500;

  /* Get the service object for the ping API */
  qmi_idl_service_object_type ping_service_object = ping_get_service_object_v01();
  if (!ping_service_object)
  {
    printf("PING: ping_get_serivce_object failed, verify qmi_ping_api_v01.h and .c match.\n");
    return -1;
  }
  rc = qmi_client_notifier_init(ping_service_object, &os_params, &notifier);

  /* Check if the service is up, if not wait on a signal */
  while(1)
  {
    rc = qmi_client_get_service_list( ping_service_object, NULL, NULL, &num_services);
    printf("PING: qmi_client_get_service_list() returned %d num_services = %d\n", rc, num_services);
    if(rc == QMI_NO_ERR)
      break;
    /* wait for server to come up */
    QMI_CCI_OS_SIGNAL_WAIT(&os_params, 0);
  }

  num_entries = num_services;
  /* The server has come up, store the information in info variable */
  rc = qmi_client_get_service_list( ping_service_object, info, &num_entries, &num_services);
  printf("PING: qmi_client_get_service_list() returned %d num_entries = %d num_services = %d\n", rc, num_entries, num_services);

  service_connect = 0;
  if (num_services > 1)
  {
    /* if the service was specified in the command line arguement process it */
    if (argc >= 5) {
      service_connect = atoi (argv[4]);
      /* If the input argument is out of range then ignore it */
      if (service_connect >= num_services) {
        service_connect = (rand() % num_services);
      }
    }
    else {
      service_connect = (rand() % num_services);
    }

    printf("%d Ping Services found: Connecting to %i\n",num_services, service_connect);
  }

  rc = qmi_client_init(&info[service_connect], ping_service_object, NULL, NULL, NULL, &clnt);
  printf("PING: qmi_client_init returned %d\n", rc);

  switch(argc)
  {
    case 4:
      test_length = atoi(argv[3]);
    case 3:
      data_size_bytes = atoi(argv[2]);
    case 2:
      test_duration_ms = atoi(argv[1]);
      break;
    default:
      printf("Using defaults test_length:%d, data_size_bytes:%d, test_duration_ms:%d\n",
             (unsigned int)test_length ,(unsigned int)data_size_bytes,(unsigned int)test_duration_ms);
  }

  if(data_size_bytes >=  PING_MAX_DATA_SIZE_V01)
    data_size_bytes = PING_MAX_DATA_SIZE_V01 - 1;

  printf("\n\n\nPING TEST 1000 STARTED...\n");
  printf("Test parameters\n");
  printf("   Duration     : %6d ms\n",(int)test_duration_ms);
  printf("   Data Size    : %6d bytes\n",(int)data_size_bytes);
  if(test_length)
  {
    printf("   Test Length  : %6d \n",(int)test_length);
  }
  printf("\n");

  rc =  qmi_ping_test_main(&clnt, test_length, data_size_bytes, test_duration_ms);
  if( rc == 0 )
  {
    printf("PASS\n");
  }
  else
  {
    printf("FAIL\n");
  }

  rc = qmi_client_release(clnt);
  printf("PING: qmi_client_release of clnt returned %d\n", rc);

  rc = qmi_client_release(notifier);
  printf("PING: qmi_client_release of notifier returned %d\n", rc);
  printf("Sleeping before exit ...\n");
  sleep(1);
  return(rc);
}
