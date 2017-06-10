/******************************************************************************
  ---------------------------------------------------------------------------
  Copyright (c) 2011-2012 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
*******************************************************************************/

#include "qmi_cci_target.h"
#include "qmi_client.h"
#include "qmi_idl_lib.h"
#include "qmi_cci_common.h"
#include <string.h>
#include "qmi_ping_api_v01.h"
static int pending_inds = 0;
static int pending_async = 0;

/*=============================================================================
  CALLBACK FUNCTION ping_ind_cb
=============================================================================*/
/*!
@brief
  This callback function is called by the QCCI infrastructure when
  infrastructure receives an indication for this client

@param[in]  user_handle          Opaque handle used by the infrastructure to
 				 identify different services.

@param[in]  msg_id               Message ID of the indication

@param[in]  ind_buf              Buffer holding the encoded indication

@param[in]  ind_buf_len          Length of the encoded indication

@param[in]  ind_cb_data          Cookie value supplied by the client during registration

*/
/*=========================================================================*/
void ping_ind_cb
(
 qmi_client_type                user_handle,
 unsigned long                  msg_id,
 unsigned char                  *ind_buf,
 int                            ind_buf_len,
 void                           *ind_cb_data
)
{
	--pending_inds;
	printf("PING: Indication: msg_id=0x%x buf_len=%d pending_ind=%d\n", (unsigned int)msg_id,
		   ind_buf_len, pending_inds);
}

/*=============================================================================
  CALLBACK FUNCTION ping_rx_cb
=============================================================================*/
/*!
@brief
  This callback function is called by the QCCI infrastructure when
  infrastructure receives an asynchronous response for this client

@param[in]   user_handle         Opaque handle used by the infrastructure to
 								 identify different services.
 
@param[in]   msg_id              Message ID of the response

@param[in]   buf                 Buffer holding the decoded response

@param[in]   len                 Length of the decoded response

@param[in]   resp_cb_data        Cookie value supplied by the client

@param[in]   transp_err          Error value

*/
/*=========================================================================*/
void ping_rx_cb
(
 qmi_client_type                user_handle,
 unsigned long                  msg_id,
 void                           *buf,
 int                            len,
 void                           *resp_cb_data,
 qmi_client_error_type          transp_err
 )
{
	--pending_async;
	/* Print the appropriate message based on the message ID */
	switch (msg_id)
	{
		case QMI_PING_RESP_V01:
			printf("PING: Async Ping Response: %s\n",((ping_resp_msg_v01 *)buf)->pong);
			break;
		case QMI_PING_DATA_RESP_V01:
			printf("PING: Async Ping Data Length: %d\n",((ping_data_resp_msg_v01 *)buf)->data_len);
			break;
		case QMI_PING_DATA_IND_REG_RESP_V01:
			printf("PING: Async Ping Data Ind Registration Success.\n");
			break;
		default:
			break;
	}
}

/*=============================================================================
  FUNCTION ping_basic_test
=============================================================================*/
/*!
@brief
  This function sends a number of basic ping messages asynchronously

@param[in]   clnt                Client handle needed to send messages

@param[in]   txn                 Transaction handle

@param[in]   num_pings           Number of pings to send

*/
/*=========================================================================*/
void ping_basic_test
(
	qmi_client_type *clnt,
	qmi_txn_handle *txn,
	int num_pings
)
{
	int i,rc;
	ping_req_msg_v01 req;
	ping_resp_msg_v01 resp;
	/* Set the value of the basic ping request */
	req.client_name_valid = 0;
	memcpy(&req, "ping", 4);
	printf("PING: Basic Ping Test with %d async ping messages.\n",num_pings);
	for (i=0;i<num_pings;++i)
	{
		rc = qmi_client_send_msg_async(*clnt, QMI_PING_REQ_V01, &req, sizeof(req),
									   &resp, sizeof(resp), ping_rx_cb, 2, txn);
		printf("PING: qmi_client_send_msg_async returned %d on loop %d\n", rc,i);
		if (rc != 0){
			printf("PING: send_msg_async error: %d\n",rc);
		  exit(1);
		}
		++pending_async;
		usleep(200);
	}
	/* Wait until all pending async messages have been received */
	while (pending_async != 0)
	{
		usleep(500);
	}
}

/*=============================================================================
  FUNCTION ping_data_test
=============================================================================*/
/*!
@brief
  This function sends a number of data ping messages asynchronously

@param[in]   clnt                Client handle needed to send messages

@param[in]   txn                 Transaction handle

@param[in]   num_pings           Number of data messages to send

@param[in]   msg_size            Size of data messages to send

*/
/*=========================================================================*/
void ping_data_test
(
	qmi_client_type *clnt,
	qmi_txn_handle *txn,
	int num_msgs,
	int msg_size
)
{
	int i,rc;
	ping_data_req_msg_v01 *data_req;
	ping_data_resp_msg_v01 *data_resp;
	data_req = (ping_data_req_msg_v01*)malloc(sizeof(ping_data_req_msg_v01));
	if (!data_req)
	{
		printf("data_req memory alloc failed\n");
		return;
	}
	data_resp = (ping_data_resp_msg_v01*)malloc(sizeof(ping_data_resp_msg_v01));
	if (!data_resp)
	{
		printf("data_resp memory alloc failed\n");
		free(data_req);
		return;
	}

	memset( data_req, 0, sizeof(ping_data_req_msg_v01) );
	memset( data_resp, 0, sizeof(ping_data_resp_msg_v01) );
	data_req->data_len = msg_size;
	printf("PING: Data Ping Test with %d async data messages of size %d.\n",num_msgs,msg_size);
	for (i=0;i<num_msgs;++i)
	{
		rc = qmi_client_send_msg_async(*clnt, QMI_PING_DATA_REQ_V01, data_req, sizeof(ping_data_req_msg_v01),
									   data_resp, sizeof(ping_data_resp_msg_v01), ping_rx_cb, 2, txn);
		printf("PING: qmi_client_send_msg_async returned %d on loop %d\n", rc,i);
		if (rc != 0){
			printf("PING: send_msg_async error: %d\n",rc);
		  exit(1);
		}
		++pending_async;
		usleep(500);
	}
	/* Wait until all pending async messages have been received */
	while (pending_async != 0)
	{
		usleep(500);
	}
	free(data_req);
	free(data_resp);
}

/*=============================================================================
  FUNCTION ping_ind_test
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
void ping_ind_test
(
	qmi_client_type *clnt,
	qmi_txn_handle *txn,
	int num_inds,
	int ind_size,
	int delay
)
{
	int i,rc;
	ping_data_ind_reg_req_msg_v01 data_ind_reg_req;
	ping_data_ind_reg_resp_msg_v01 data_ind_reg_resp;

	/* Set the number of pending indications */
	pending_inds = num_inds;
	memset( &data_ind_reg_req, 0, sizeof(ping_data_ind_reg_req_msg_v01) );
	memset( &data_ind_reg_resp, 0, sizeof(ping_data_ind_reg_resp_msg_v01) );
	data_ind_reg_req.num_inds_valid = 1;
	data_ind_reg_req.num_inds = num_inds;
	/* Send the optional TLVs if these values are passed as arguments */
	if (delay > 0)
	{
		data_ind_reg_req.ind_delay_valid = 1;
		data_ind_reg_req.ind_delay = delay;
	}
	if (ind_size > 0)
	{
		data_ind_reg_req.ind_size_valid = 1;
		data_ind_reg_req.ind_size = ind_size;
	}
	printf("PING: Data Indication Test with %d indications of size %d.\n",num_inds,ind_size);
	rc = qmi_client_send_msg_async(*clnt, QMI_PING_DATA_IND_REG_REQ_V01, &data_ind_reg_req,
								   sizeof(data_ind_reg_req),&data_ind_reg_resp,
								   sizeof(data_ind_reg_resp), ping_rx_cb, 2, txn);
	printf("PING: qmi_client_send_msg_async returned %d\n", rc);
	if (rc != 0){
		printf("PING: send_msg_async error: %d\n",rc);
		exit(1);
	}
	++pending_async;
	usleep(200);
	/* Wait until all pending async messages have been received */
	while (pending_async != 0)
	{
		usleep(500);
	}
	/* Wait until all pending indications have been received */
	while (pending_inds != 0)
	{
		usleep(500);
	}
}

/*=============================================================================
  FUNCTION main
=============================================================================*/
int main(int argc, char **argv)
{
  qmi_txn_handle txn;
  uint32_t num_services, num_entries=0;
  int rc,service_connect;
  enum ping_tests{
	  PING_BASIC_TEST = 1,
	  PING_DATA_TEST,
	  PING_IND_TEST
  }ping_tests_type;
  int i;
  /* Set the defaults for the ping test arguments */
  int ping_test_args[3] = {1,0,0};
  qmi_client_type clnt, notifier;
  qmi_cci_os_signal_type os_params;

  qmi_service_info info[10];
  uint32_t port = 10001;

  /* Get the service object for the ping API */
  qmi_idl_service_object_type ping_service_object = ping_get_service_object_v01();
  /* Verify that ping_get_service_object did not return NULL */
  if (!ping_service_object)
  {
	  printf("PING: ping_get_serivce_object failed, verify qmi_ping_api_v01.h and .c match.\n");
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
  };

  num_entries = num_services;
  /* The server has come up, store the information in info variable */
  rc = qmi_client_get_service_list( ping_service_object, info, &num_entries, &num_services);
  printf("PING: qmi_client_get_service_list() returned %d num_entries = %d num_services = %d\n", rc, num_entries, num_services);
  service_connect = 0;
  if (num_services > 1)
  {
	printf("%d Ping Services found: Choose which one to connect to (numbered starting at 0)\n",num_services);
	do
        {
	  scanf("%d",&service_connect);
        } while(service_connect >= num_services);
  }

  rc = qmi_client_init(&info[service_connect], ping_service_object, ping_ind_cb, NULL, NULL, &clnt);

  printf("PING: qmi_client_init returned %d\n", rc);
  /* If additional arguments have beens upplied to choose a specific test */
  if (argc > 1)
  {
	  ping_tests_type = atoi(argv[1]);
	  for (i=2; i<argc; ++i)
	  {
		  ping_test_args[i-2] = atoi(argv[i]);
	  }
	  switch(ping_tests_type)
	  {
		  case PING_BASIC_TEST:
			  ping_basic_test(&clnt,&txn,ping_test_args[0]);
			  break;
		  case PING_DATA_TEST:
			  ping_data_test(&clnt,&txn,ping_test_args[0],ping_test_args[1]);
			  break;
		  case PING_IND_TEST:
			  ping_ind_test(&clnt,&txn,ping_test_args[0],ping_test_args[1],ping_test_args[2]);
			  break;
		  default:
			  printf("PING: Unrecognized test number: %d\n",ping_tests_type);
	  }
  }else{
	  /* No args passed, perform basic single ping test */
	  ping_basic_test(&clnt,&txn,1);
  }

  rc = qmi_client_release(clnt);
  printf("PING: qmi_client_release of clnt returned %d\n", rc);

  rc = qmi_client_release(notifier);
  printf("PING: qmi_client_release of notifier returned %d\n", rc);
  return 0;
}
