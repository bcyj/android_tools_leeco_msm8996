#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include "qmi_client.h"
#include "wireless_data_service_v01.h"
#include "qmi.h"
#include <pthread.h>


#define WDS_MSG_DEFAULT_TIMEOUT 5000
#define QMI_WDS_PROFILE_TECH_3GPP 0x01
#define QMI_HEADER_SIZE QMI_MAX_HDR_SIZE
#define MESSAGE_RECEIVED 0
#define MESSAGE_NOT_RECEIVED 1
#define DECODE_ERROR 2
#define MESSAGE_RECEIVED_TRANSPORT_ERROR 3


/* Macro for signaling reader thread */
#define SIGNAL_READER(MSG) \
        pthread_mutex_lock(&async_resp);\
        async_msg_recv_flag = MSG;\
        pthread_cond_signal(&async_resp_cond);\
        pthread_mutex_unlock(&async_resp);\



/* Callback that needs to be defined by the user for asynchronous message sending */
static void start_network_interface_callback ( qmi_client_type              user_handle,
                                               unsigned long                msg_id,
                                               void                         *resp_c_struct,
                                               int                          resp_c_struct_len,
                                               void                         *resp_cb_data,
                                               qmi_client_error_type        tranp_err
                                              );


static void start_nw_if_thread(void* user_handle);
static void read_nw_if_resp_thread(void* user_handle);
static int  stop_nw_if(qmi_client_type user_handle,int pkt_handle);

/* Global data */
wds_start_network_interface_req_msg_v01  request_msg;
wds_start_network_interface_resp_msg_v01 response_msg;

/* Global  Mutex data */
pthread_mutex_t async_resp;
pthread_cond_t async_resp_cond;
pthread_t async_resp_handler;

/* Global async message flag */
int async_msg_recv_flag = -1;

/*qmi message library handle*/
static int qmi_handle = QMI_INVALID_CLIENT_HANDLE;

static void start_network_interface_callback ( qmi_client_type              user_handle,
                                               unsigned long                msg_id,
                                               void                         *resp_c_struct,
                                               int                          resp_c_struct_len,
                                               void                         *resp_cb_data,
                                               qmi_client_error_type        transp_err
                                              )
{


    if (transp_err !=  QMI_NO_ERR && transp_err != QMI_IDL_MISSING_TLV ) {
        printf("Transport Error ERROR CODE:%d \n",transp_err );
        SIGNAL_READER(MESSAGE_RECEIVED_TRANSPORT_ERROR);
    }
    else
    {
        printf("Signaling the reader thread\n");
        SIGNAL_READER(MESSAGE_RECEIVED);
    }
}


static void start_nw_if_thread( void* user_handle )
{
   qmi_client_error_type rc;
   qmi_txn_handle txn_handle;
   int thread_rc;

   /* Setting the request/response message structure to zero */
   memset(&request_msg,0,sizeof(wds_start_network_interface_req_msg_v01));
   memset(&response_msg,0,sizeof(wds_start_network_interface_resp_msg_v01));

  /* Create a thread to handle the signal received when we get the response */
   thread_rc = pthread_create(&async_resp_handler,NULL,read_nw_if_resp_thread,user_handle);

   if (thread_rc) {
       printf("pthread_create failed \n");
       return;
   }


   /* Start network interface */
   rc =  qmi_client_send_msg_async(user_handle,
                                   QMI_WDS_START_NETWORK_INTERFACE_REQ,
                                   &request_msg,
                                   sizeof(wds_start_network_interface_req_msg_v01),
                                   &response_msg,
                                   sizeof(wds_start_network_interface_resp_msg_v01),
                                   &start_network_interface_callback,
                                   NULL,
                                   &txn_handle);

   if (rc == QMI_NO_ERR ) {
       printf("Starting network interface asyncronously.......... \n");
       /* Here we can try cancelling the transaction */
   }
   else {
        printf("ERROR in sending the start n/w interface message, ERROR CODE:%d",rc);
        SIGNAL_READER(MESSAGE_NOT_RECEIVED);
   }
 pthread_exit(NULL);
}


static void read_nw_if_resp_thread(void* user_handle)
{
    int rc;

    /* wait for the signal until the data arrives */
    pthread_mutex_lock(&async_resp);
    pthread_cond_wait(&async_resp_cond,&async_resp);

    if  (async_msg_recv_flag == MESSAGE_RECEIVED ) {
          /* Read the message */
        printf(" Reading asynchronous response \n");
        printf(" Packet data handle : %d\n", response_msg.pkt_data_handle);
        /* Handle call end reason */
        if (response_msg.call_end_reason_valid) {
            printf("Call end reason : %d\n",response_msg.call_end_reason);
        }
        /* we can do some work here on the data received */
        /* We can stop the network interface here or in the main thread */
    }
    else if (async_msg_recv_flag == MESSAGE_RECEIVED_TRANSPORT_ERROR) {
        printf(" Message received but there was a transport error \n");
    }
    else if (async_msg_recv_flag == MESSAGE_NOT_RECEIVED) {
        printf(" The asynchronous message sending failed...so no need to wait for reading it \n");
    }

    pthread_mutex_unlock(&async_resp);
    pthread_exit(NULL);
}


static int stop_network_if(qmi_client_type user_handle,int pkt_handle)
{
    wds_stop_network_interface_req_msg_v01  stop_nw_req;
    wds_stop_network_interface_resp_msg_v01 stop_nw_res;

    int rc,qmi_err_code;

    /* memset the request response structure to zero */
    memset(&stop_nw_req,0,sizeof(wds_stop_network_interface_req_msg_v01));
    memset(&stop_nw_res,0,sizeof(wds_stop_network_interface_resp_msg_v01));

    stop_nw_req.pkt_data_handle = pkt_handle;

    printf("The user handle used to stop the network interface is:%p\n",user_handle);
    printf("The packet handle used to stop the network interface is:%d\n",pkt_handle);


    rc = qmi_client_send_msg_sync(user_handle,
                                  QMI_WDS_STOP_NETWORK_INTERFACE_REQ,
                                  &stop_nw_req,
                                  sizeof(wds_stop_network_interface_req_msg_v01),
                                  &stop_nw_res,
                                  sizeof(wds_stop_network_interface_resp_msg_v01),
                                  WDS_MSG_DEFAULT_TIMEOUT);

   /* we can check for standard response here */
    return rc;

}

int main(int argc, char *argv[])
{
    qmi_client_error_type rc;
    qmi_client_type user_handle;
	pthread_t thread_start_nw_if;
    void *status;
    qmi_idl_service_object_type wds_service_obj;


    /* Initialize the thread variable */
    pthread_mutex_init(&async_resp, NULL);
    pthread_cond_init (&async_resp_cond, NULL);

   /* Initialize the qmi datastructure(Once per process ) */
    qmi_handle = qmi_init(NULL,NULL);

    if (qmi_handle < 0)
    {
      printf("qmi message library not initialized.");
      return qmi_handle;
    }

   /*Get the wds service object */
    wds_service_obj = wds_get_service_object();


   /* Initialize a connection to first QMI control port */
    rc = qmi_client_init("rmnet0",
                        wds_service_obj,
                        NULL,
                        NULL,
                        &user_handle
                        );


    if (rc != QMI_NO_ERR ) {
         printf("Error: connection not Initialized...Error Code:%d\n",rc);
         rc = qmi_client_release(user_handle);
         if (rc < 0 ) {
            printf("Release not successful \n");
         }else
         {
            printf("Qmi client release successful \n");
         }
    }
    else{
        printf("Connection Initialized....User Handle:%d\n",user_handle);

        rc = pthread_create(&thread_start_nw_if,NULL,start_nw_if_thread,(void *)user_handle);

	   if (rc) {
            printf("Error in creating the thread_start_nw_if thread \n");
            exit(-1);
        }

	   /* Join thread thread_start_nw_if */
	   rc = pthread_join(thread_start_nw_if,&status);
        if (rc == -1 ) {
            exit(-1);
        }
	   printf("Joined thread_start_nw_if \n");

	   /* Join thread async_resp_handler thread */
        rc = pthread_join(async_resp_handler,&status);
        if (rc == -1 ) {
            exit(-1);
        }
	   printf("Joined async_resp_handler \n");

	   /*Stop the network interface */
         rc = stop_network_if(user_handle,response_msg.pkt_data_handle);
         if (rc == QMI_NO_ERR ) {
             printf(" Network interface stopped successfully \n");
         }
         else
         {
             printf(" Error in stopping network interface, Error Code:%d\n", rc );
         }
        /* Release the client */

        rc = qmi_client_release(user_handle);
        if (rc < 0 ) {
            printf("Release not successful \n");
        }else
        {
            printf("Qmi client release successful \n");
        }
        if (qmi_handle >= 0)
        {
          qmi_release(qmi_handle);
        }
        printf("Qmi release done.........\n");

    }
   return rc;
}
