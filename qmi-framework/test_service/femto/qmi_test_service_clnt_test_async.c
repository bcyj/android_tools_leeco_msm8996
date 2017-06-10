/******************************************************************************
  -----------------------------------------------------------------------------
  Copyright (c) 2014 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  -----------------------------------------------------------------------------

******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include "qmi_client.h"
#include "qmi_idl_lib.h"
#include "qmi_cci_target_ext.h"
#include "../qmi_test_service_clnt_common_stats.h"
#include "../qmi_test_service_clnt_common.h"
#include "../test_service_v01.h"

#define REMOTE_SERVICE_FIRST  10
#define REMOTE_SERVICE_LAST   14
/*===========================================================================
  FUNCTION  qmi_test_main
===========================================================================*/
/*!
@brief
  Main body of qmi clnt test

@return
  0 for pass, -1 for fail

@note
  - Dependencies
    - N/A
  - Side Effects
*/
/*=========================================================================*/
static int qmi_test_main(qmi_client_type *clnt, qmi_txn_handle *txn, int test_length, int data_size_bytes, int test_duration_ms)
{
  uint32_t rc;
  qmi_test_clnt_results_type results;

  printf("\n\n\nQMI TEST STARTED...\n");
  rc = qmi_test_clnt_common_async_data_test(clnt, txn, test_length, data_size_bytes, test_duration_ms, 1, &results);
  if(rc == 0)
  {
    printf("Test Passed\n");
    printf("Performance: \n");
    qmi_test_clnt_common_print_stats(&results, QMI_TEST_STANDARD_PRINT_FORMAT);
  }
  else
  {
    printf("Test Failed, \n");
  }
  printf("QMI TEST COMPLETE...\n");

  return(rc );
}

/*=============================================================================
  Main functions
=============================================================================*/
static void usage(int ret)
{
  printf("Usage: [[-d][-t]] [-p] [-r] [-i] [-h]\n"
      "Runs a asynchronous QMI ping\n"
      "OPTIONS can be:\n"
      "  -d, --duration       Duration of the test in on ms, default 500ms\n"
      "  -t, --test_length    The amount of messages to send\n"
      "  -p, --packet_size    The size of the transmit unit in bytes,\n"
      "                       default 1000 \n"
      "  -r  --remote_service The remote server to connect to\n"
      "                       10 - HEX0 service\n"
      "                       11 - HEX1 service\n"
      "                       12 - HEX2 service\n"
      "                       13 - HEX3 service\n"
      "                       14 - SCI service\n"
      "                       default - HEX0\n"
      "  -i, --itterations    The amount of tests to run, default 1\n"
      "  -h, --help           print this help message and exit\n"
      "NOTE: -d and -t are incompatible with each other.  When one is set\n"
      "      to a non-zero value then the other has to be 0.\n"
  );
  exit(ret);
}

static uint32_t parse_command
(
  int argc,
  char *const argv[],
  int *duration,
  int *test_length,
  int *packet_size,
  int *remote_service,
  int *itterations
)
{
  int command;
  unsigned ret = 0;

  struct option longopts[] = {
    {"duration", required_argument, NULL, 'd'},
    {"test_length", required_argument, NULL, 't'},
    {"packet_size", required_argument, NULL, 'p'},
    {"remote_service", required_argument, NULL, 'r'},
    {"itterations", required_argument, NULL, 'i'},
    {"help", no_argument, NULL, 'h'},
  };

  while ((command = getopt_long(argc, argv, "d:t:p:r:i:h", longopts,
          NULL)) != -1) {
    switch (command) {
    case 'd':
      *duration = atoi (optarg);
      *test_length = 0;
      break;
    case 't':
      *test_length = (unsigned int)atoi (optarg);
      *duration = 0;
      break;
    case 'p':
      *packet_size = atoi (optarg);
      break;
    case 'r':
      *remote_service = atoi (optarg);
      if ((*remote_service < REMOTE_SERVICE_FIRST) ||
          (*remote_service > REMOTE_SERVICE_LAST)) {
        printf ("TEST: invalid remote_service %i\n",
                *remote_service);
        exit (-1);
      }
      break;
    case 'i':
      *itterations = atoi (optarg);
      break;
    case 'h':
      usage (0);
      break;
    default:
      usage(-1);
    }
  }

  return ret;
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
  int duration = 500;
  int test_length = 0;
  int packet_size = 1000;
  int remote_service = REMOTE_SERVICE_FIRST;
  int itterations = 1;
  int idx = 0;
  qmi_txn_handle txn;
  uint32_t num_services;
  int rc;
  qmi_client_type clnt, notifier;
  qmi_cci_os_signal_type os_params;
  qmi_service_info service_info;
  const char *remote_names[] = {"HEX0", "HEX1", "HEX2", "HEX3", "SCI"};

  parse_command (argc, argv, &duration, &test_length, &packet_size,
                 &remote_service, &itterations);

  /* Get the service object for the test API */
  qmi_idl_service_object_type test_service_object = test_get_service_object_v01();
  if (!test_service_object)
  {
    printf("TEST: FAIL: test_get_serivce_object failed, verify test_service_v01.h and .c match.\n");
    goto handle_async_test_failure;
  }

  rc = qmi_client_notifier_init(test_service_object, &os_params, &notifier);
  if (rc != QMI_NO_ERR) {
    printf ("TEST: FAIL: qmi_client_notifier_init returned %i\n", rc);
    goto handle_async_test_failure;
  }

  /* Check if the service is up, if not wait on a signal */
  while(1)
  {

    rc = qmi_client_get_service_list( test_service_object, NULL, NULL, &num_services);
    printf("TEST: qmi_client_get_service_list() returned %d num_services = %d\n", rc, num_services);
    if(rc == QMI_NO_ERR)
      break;
    /* wait for server to come up */
    QMI_CCI_OS_SIGNAL_WAIT(&os_params, 0);
  }

  /* The server has come up, store the information in info variable */
  rc = qmi_client_get_service_instance (test_service_object, remote_service, &service_info);
  if (rc != QMI_NO_ERR) {
    printf("TEST: FAIL: remote_service %i not registered\n", remote_service);
    goto handle_async_test_failure_free;
  }

  printf ("\nTEST: Testing remote_service %s\n",
          remote_names[remote_service - REMOTE_SERVICE_FIRST]);

  if(packet_size >=  TEST_MED_DATA_SIZE_V01)
    packet_size= TEST_MED_DATA_SIZE_V01 - 1;

  for (idx=0; idx < itterations; idx++)
  {
    printf("\nTEST: Starting test %i\n", idx);
    rc = qmi_client_init(&service_info, test_service_object, NULL, NULL, NULL, &clnt);
    if (rc != QMI_NO_ERR) {
      printf("TEST: FAIL: qmi_client_init returned %d\n", rc);
      goto handle_async_test_failure_free;
    }

    printf("\n\n\nQMI TEST ASYNC STARTED...\n");

    printf("Test parameters\n");
    printf("   Duration     : %6d ms\n", duration);
    printf("   Data Size    : %6d bytes\n", packet_size);
    if (test_length)
    {
      printf("   Test Length  : %6d \n",(int)test_length);
    }
    printf("\n");

    rc =  qmi_test_main(&clnt, &txn, test_length, packet_size, duration);
    if( rc == 0 )
    {
      printf("PASS\n");
    }
    else
    {
      printf("FAIL\n");
      rc = qmi_client_release(clnt);
      goto handle_async_test_failure_free;
    }

    rc = qmi_client_release(clnt);
    if (rc != QMI_NO_ERR) {
      printf("TEST: FAIL: qmi_client_release of clnt returned %d\n", rc);
      goto handle_async_test_failure_free;
    }

    sleep(1);
  }
  rc = qmi_client_release(notifier);
  if (rc != QMI_NO_ERR) {
    printf("TEST: FAIL: qmi_client_release of notifier returned %d\n", rc);
    goto handle_async_test_failure;
  }

  printf("Sleeping before exit ...\n");
  sleep(1);
  return  (0);

handle_async_test_failure_free:
  qmi_client_release(notifier);

handle_async_test_failure:
  return (-1);
}
