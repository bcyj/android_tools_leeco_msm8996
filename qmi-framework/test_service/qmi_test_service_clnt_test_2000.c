/******************************************************************************
  -----------------------------------------------------------------------------
  Copyright (c) 2012-2014 Qualcomm Technologies, Inc.
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
#include "qmi_client_instance_defs.h"
#include "qmi_test_service_clnt_common_stats.h"
#include "qmi_test_service_clnt_common.h"
#include "test_service_v01.h"

static uint32_t test_length;
static uint32_t data_size_bytes;
static uint32_t test_duration_ms = 500;
static int service_connect;
static int use_qmuxd;

static void usage(const char *fname)
{
  printf("\n*************************************************************\n");
  printf("Usage: %s PARAMS can be:\n"
	  "  -q,          Use this option to enable QMUX\n"
	  "  -t <value>,  Time duration for the entire test\n"
	  "  -i <value>,  Number of iterations to run\n"
          "  -s <value>,  Service Instance to connect to"
	"\n", fname);
  printf("- To run multiple QMI txns, specify option t=0 and specify option i= no. of txns\n");
  printf("- To run for a specific time duration, specify option 't' and omit option 'i'\n");
  printf("\n*************************************************************\n");
}

static void parse_command(int argc, char *const argv[])
{
  int command;

  struct option longopts[] = {
	{"use_qmux", no_argument, NULL, 'q'},
	{"duration", required_argument, NULL, 't'},
	{"iterations", required_argument, NULL, 'i'},
	{"service_connect", required_argument, NULL, 's'},
	{"help", no_argument, NULL, 'h'},
	{NULL, 0, NULL, 0},
  };

  while ((command = getopt_long(argc, argv, "qht:i:s:", longopts,
				NULL)) != -1) {
    switch (command) {
      case 'q':
	use_qmuxd = 1;
	break;
      case 'h':
        usage(argv[0]);
	exit(0);
      case 't':
	test_duration_ms = atoi(optarg);
	break;
      case 'i':
	test_length = atoi(optarg);
	break;
      case 's':
	service_connect = atoi(optarg);
	break;
      default:
        usage(argv[0]);
	exit(-1);
    }
  }
}
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
static int qmi_test_main(qmi_client_type *clnt, int test_length, int test_duration_ms)
{
  uint32_t rc;
  qmi_test_clnt_results_type results;

  printf("\n\n\nQMI TEST STARTED...\n");
  rc = qmi_test_clnt_common_sync_basic_test(clnt, test_length, test_duration_ms, 1, &results);
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
  qmi_txn_handle txn;
  uint32_t num_services, num_entries=0;
  int rc;
  qmi_client_type clnt, notifier;
  qmi_cci_os_signal_type os_params;
  qmi_service_info info[10];

  printf("\n To show usage: %s -h\n", argv[0]);
  parse_command(argc, argv);

  if (!use_qmuxd)
  {
    qmi_cci_qmux_xport_unregister(QMI_CLIENT_QMUX_RMNET_INSTANCE_0);
    qmi_cci_qmux_xport_unregister(QMI_CLIENT_QMUX_RMNET_USB_INSTANCE_0);
    qmi_cci_qmux_xport_unregister(QMI_CLIENT_QMUX_RMNET_SMUX_INSTANCE_0);
    qmi_cci_qmux_xport_unregister(QMI_CLIENT_QMUX_RMNET_MHI_INSTANCE_0);
  }

 /* Get the service object for the test API */
  qmi_idl_service_object_type test_service_object = test_get_service_object_v01();
  if (!test_service_object)
  {
    printf("TEST: test_get_serivce_object failed, verify test_service_v01.h and .c match.\n");
    return -1;
  }
  rc = qmi_client_notifier_init(test_service_object, &os_params, &notifier);

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

  num_entries = num_services;
  /* The server has come up, store the information in info variable */
  rc = qmi_client_get_service_list( test_service_object, info, &num_entries, &num_services);
  printf("TEST: qmi_client_get_service_list() returned %d num_entries = %d num_services = %d\n", rc, num_entries, num_services);
  if (argc >= 5)
    service_connect = atoi(argv[4]);

  if (service_connect >= num_services)
  {
    printf("Only %d Test Services found: Choosing the default one)\n",num_services);
    service_connect = 0;
  }

  rc = qmi_client_init(&info[service_connect], test_service_object, NULL, NULL, NULL, &clnt);
  printf("TEST: qmi_client_init returned %d\n", rc);

  printf("Test Attributes :- test_length:%d, test_duration_ms:%d\n",
             (unsigned int)test_length ,(unsigned int)test_duration_ms);

  printf("\n\n\nQMI TEST 2000 STARTED...\n");
  printf("Test parameters\n");
  printf("   Duration     : %6d ms\n",(int)test_duration_ms);
  if(test_length)
  {
    printf("   Test Length  : %6d \n",(int)test_length);
  }
  printf("\n");

  rc =  qmi_test_main(&clnt, test_length, test_duration_ms);
  if( rc == 0 )
  {
    printf("PASS\n");
  }
  else
  {
    printf("FAIL\n");
  }

  rc = qmi_client_release(clnt);
  printf("TEST: qmi_client_release of clnt returned %d\n", rc);

  rc = qmi_client_release(notifier);
  printf("TEST: qmi_client_release of notifier returned %d\n", rc);
  printf("Sleeping before exit ...\n");
  sleep(1);
  return(rc);
}
