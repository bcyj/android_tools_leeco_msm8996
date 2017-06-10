/******************************************************************************
  ---------------------------------------------------------------------------
  Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
*******************************************************************************/

#include "qmi_client.h"
#include "qmi_idl_lib.h"
#include "qmi_cci_target_ext.h"
#include <string.h>
#include <getopt.h>
#include "../test_service_v01.h"


static int pending_inds;


#define REMOTE_SERVICE_FIRST  10
#define REMOTE_SERVICE_LAST   14
#define QMI_TEST_MAX_IND_SZ   8192

static void usage(const char *fname)
{
  printf("\n*************************************************************\n");
  printf("Usage: %s PARAMS can be:\n"
    "  -i,          number of indications: default 10\n"
    "  -n,          number of bytes for each indication:\n"
    "               default 256: max %i\n"
    "  -h,          Help Option to print Usage\n"
    "  -d,          Delay for each indication: default 100 \n",
    "  -r  --remote_service The remote server to connect to\n"
    "                       10 - HEX0 service\n"
    "                       11 - HEX1 service\n"
    "                       12 - HEX2 service\n"
    "                       13 - HEX3 service\n"
    "                       14 - SCI service\n"
    "                       default - HEX0\n",
    fname, QMI_TEST_MAX_IND_SZ);
  printf("\n*************************************************************\n");
}

static void parse_command
(
  int   argc,
  char *const argv[],
  int  *indications,
  int  *num_bytes,
  int  *delay,
  int  *remote_service
)
{
  int command;

  struct option longopts[] = {
    {"indications", required_argument, NULL, 'i'},
    {"num_bytes", required_argument, NULL, 'n'},
    {"help", no_argument, NULL, 'h'},
    {"delay", required_argument, NULL, 'd'},
    {"remote_service", required_argument, NULL, 'r'},
    {NULL, 0, NULL, 0},
  };

  while ((command = getopt_long(argc, argv, "hn:d:i:r:", longopts,
         NULL)) != -1) {
    switch (command) {
      case 'i':
        *indications = atoi(optarg);
        break;
      case 'h':
        usage(argv[0]);
        exit(0);
      case 'n':
        *num_bytes = atoi(optarg);
        if (*num_bytes > QMI_TEST_MAX_IND_SZ) {
          printf ("TEST: invalid inication size of %i: max %i\n",
                  *num_bytes, QMI_TEST_MAX_IND_SZ);
          exit (-1);
        }
        break;
      case 'd':
        *delay = atoi(optarg);
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
      default:
        usage(argv[0]);
        exit(-1);
    }
  }
}

/*=============================================================================
  CALLBACK FUNCTION test_service_ind_cb
=============================================================================*/
/*!
@brief
  This callback function is called by the QCCI infrastructure when
  infrastructure receives an indication for this client

@param[in]   user_handle         Opaque handle used by the infrastructure to
         identify different services.

@param[in]   msg_id              Message ID of the indication

@param[in]  ind_buf              Buffer holding the encoded indication

@param[in]  ind_buf_len          Length of the encoded indication

@param[in]  ind_cb_data          Cookie value supplied by the client during registration

*/
/*=========================================================================*/
void test_service_ind_cb
(
 qmi_client_type                user_handle,
 unsigned long                  msg_id,
 unsigned char                  *ind_buf,
 int                            ind_buf_len,
 void                           *ind_cb_data
)
{
  pending_inds--;

  if (!(pending_inds % 1000))
    printf("TEST_SERVICE: Indication: msg_id=0x%x buf_len=%d pending_ind=%d\n",
          (unsigned int)msg_id, ind_buf_len, pending_inds);

}

/*=============================================================================
  CALLBACK FUNCTION test_service_rx_cb
=============================================================================*/
/*!
@brief
         TBD

@param[in]   user_handle         Opaque handle used by the infrastructure to
         identify different services.

@param[in]   msg_id              Message ID of the response

@param[in]   buf                 Buffer holding the decoded response

@param[in]   len                 Length of the decoded response

@param[in]   resp_cb_data        Cookie value supplied by the client

@param[in]   transp_err          Error value

*/
/*=========================================================================*/
void test_service_rx_cb
(
 qmi_client_type                user_handle,
 unsigned long                  msg_id,
 void                           *buf,
 int                            len,
 void                           *resp_cb_data,
 qmi_client_error_type          transp_err
 )
{
  /* Print the appropriate message based on the message ID */
  switch (msg_id)
  {
    case QMI_TEST_RESP_V01:
      printf("TEST: Async Test Response: %s\n",((test_ping_resp_msg_v01 *)buf)->pong);
      break;
    case QMI_TEST_DATA_RESP_V01:
      printf("TEST: Async Test Data Length: %d\n",((test_data_resp_msg_v01 *)buf)->data_len);
      break;
    case QMI_TEST_DATA_IND_REG_RESP_V01:
      printf("TEST: Async Test Data Ind Registration Success.\n");
      break;
    default:
      break;
  }
}


/*=============================================================================
  FUNCTION test_service_ind_test
=============================================================================*/
/*!
@brief
  This function tells the service to send a specified number of indication messages

@param[in]   clnt                Client handle needed to send messages

@param[in]   txn                 Transaction handle

@param[in]   num_inds            Number of indications for the service to send

@param[in]   ind_size            Size of indications for the service to send

@param[in]   delay               Amount of time the server should wait between indications

*/
/*=========================================================================*/
int test_service_ind_test
(
  qmi_client_type *clnt,
  qmi_txn_handle *txn,
  int num_inds,
  int ind_size,
  int delay
)
{
  int i,rc;
  test_data_ind_reg_req_msg_v01 data_ind_reg_req;
  test_data_ind_reg_resp_msg_v01 data_ind_reg_resp;

  pending_inds = num_inds;

  memset( &data_ind_reg_req, 0, sizeof(test_data_ind_reg_req_msg_v01) );
  memset( &data_ind_reg_resp, 0, sizeof(test_data_ind_reg_resp_msg_v01) );
  data_ind_reg_req.num_inds_valid = 1;
  data_ind_reg_req.num_inds = num_inds;
  /* Send the optional TLVs if these values are passed as arguments */
  if (delay > 0)
  {
    data_ind_reg_req.ms_delay_valid = 1;
    data_ind_reg_req.ms_delay = delay;
  }
  if (ind_size > 0)
  {
    data_ind_reg_req.ind_size_valid = 1;
    data_ind_reg_req.ind_size = ind_size;
  }

  printf("TEST: Data Indication Test with %d indications of size %d.\n",num_inds,ind_size);
  rc = qmi_client_send_msg_async(*clnt, QMI_TEST_DATA_IND_REG_REQ_V01, &data_ind_reg_req,
                   sizeof(data_ind_reg_req),&data_ind_reg_resp,
                   sizeof(data_ind_reg_resp), test_service_rx_cb, NULL, txn);
  printf("TEST: qmi_client_send_msg_async returned %d\n", rc);
  if (rc != 0){
    printf("TEST: send_msg_async error: %d\n",rc);
    exit(1);
  }
  usleep(200);

  /* Wait until all pending indications have been received */
  while (pending_inds != 0)
  {
    usleep(500);
  }

  printf ("TEST: PASS num_inds %i\n", num_inds);

  return 0;
}

/*=============================================================================
  FUNCTION main
=============================================================================*/
int main(int argc, char **argv)
{
  int remote_service = REMOTE_SERVICE_FIRST;
  qmi_txn_handle txn;
  uint32_t num_services;
  int rc;
  int num_bytes = 256;
  int delay = 100;
  int indications = 10;
  qmi_client_type clnt, notifier;
  qmi_cci_os_signal_type os_params;
  qmi_service_info service_info;
  const char *remote_names[] = {"HEX0", "HEX1", "HEX2", "HEX3", "SCI"};

  parse_command (argc, argv, &indications, &num_bytes, &delay, &remote_service);
  /* Get the service object for the test API */
  qmi_idl_service_object_type test_service_object = test_get_service_object_v01();
  if (!test_service_object)
  {
    printf("TEST: FAIL: test_get_serivce_object failed, verify test_service_v01.h and .c match.\n");
    goto handle_ind_test_failure;
  }

  rc = qmi_client_notifier_init(test_service_object, &os_params, &notifier);
  if (rc != QMI_NO_ERR) {
    printf ("TEST: FAIL: qmi_client_notifier_init returned %i\n", rc);
    goto handle_ind_test_failure;
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
    goto handle_ind_test_failure_free;
  }

  printf ("\nTEST: Testing remote_service %s\n",
          remote_names[remote_service - REMOTE_SERVICE_FIRST]);

  printf("\nTEST: Starting test\n");
  rc = qmi_client_init(&service_info, test_service_object, test_service_ind_cb, NULL, NULL, &clnt);
  if (rc != QMI_NO_ERR) {
    printf("TEST: FAIL: qmi_client_init returned %d\n", rc);
    goto handle_ind_test_failure_free;
  }

  printf("\n\n\nQMI TEST IND STARTED...\n");

  printf("Test parameters\n");
  printf("   num_bytes: %i Bytes\n", num_bytes);
  printf("   delay: %i ms\n", delay);
  printf("   indications: %i\n", indications);
  printf("   remote_processor: %s\n",
         remote_names [remote_service-REMOTE_SERVICE_FIRST]);
  printf("\n");

  rc =  test_service_ind_test(&clnt, &txn, indications, num_bytes, delay);
  if( rc == 0 )
  {
    printf("PASS\n");
  }
  else
  {
    printf("FAIL\n");
    rc = qmi_client_release(clnt);
    goto handle_ind_test_failure_free;
  }

  rc = qmi_client_release(clnt);
  if (rc != QMI_NO_ERR) {
    printf("TEST: FAIL: qmi_client_release of clnt returned %d\n", rc);
    goto handle_ind_test_failure_free;
  }

  sleep(1);

  rc = qmi_client_release(notifier);
  if (rc != QMI_NO_ERR) {
    printf("TEST: FAIL: qmi_client_release of notifier returned %d\n", rc);
    goto handle_ind_test_failure;
  }

  printf("Sleeping before exit ...\n");
  sleep(1);
  return  (0);

handle_ind_test_failure_free:
  qmi_client_release(notifier);

handle_ind_test_failure:
  return (-1);

}
