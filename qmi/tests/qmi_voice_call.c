/* This file contains the source code for testing of QMI APIs for making
   voice calls and receiving RSSI indications from the modem.

   Copyright (c) 2010 Qualcomm Technologies, Inc.  All Rights Reserved.*/
/*===========================================================================

                           INCLUDE FILES

===========================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "log.h"
#include "qmi_client.h"
#include "device_management_service_v01.h"
#include "voice_service_v01.h"
#include "network_access_service_v01.h"
#include "wireless_messaging_service_v01.h"
#include "qmi.h"

/*qmi message library handle*/
static int qmi_handle = QMI_INVALID_CLIENT_HANDLE;
/*===========================================================================

                    INTERNAL DEFINITIONS AND TYPES

===========================================================================*/
/*===========================================================================
                    MACRO DEFINITIONS
===========================================================================*/

/* Macro Definitions of constants*/
#define VOICE_MSG_DEFAULT_TIMEOUT 5000
#define QMI_VOICE_MAX_TLV_SIZE (512 - 200)
#define QMI_HEADER_SIZE QMI_MAX_HDR_SIZE

#define MESSAGE_RECEIVED 0
#define MESSAGE_NOT_RECEIVED 1
#define DECODE_ERROR 2
#define MESSAGE_RECEIVED_TRANSPORT_ERROR 3

/* Modem bringup constants */
#define ONLINE_MODE 0x00
#define LPM_MODE 0x01

/* Call status indicators from network */
#define CALL_STATUS_ORIGINATION 0x00
#define CALL_STATUS_ANSWER 0x01
#define CALL_STATUS_END 0x03
#define CALL_STATUS_ALERTING 0x05
#define CALL_STATUS_CONNECT 0x06

/*RSSI indication type */
#define QMI_NAS_EVENT_REPORT_IND_MSG_ID 0x0002 

/* Timeout for accepting or rejecting an incoming voice call */
#define SELECT_TIMEOUT 2

/* Print statement for exiting from the state which receives RSSI notifications from the modem */
#define EXIT_IND_MODE_PRINTF printf("Press 'e' to exit\n")

#define USER_INPUT_MAX 10
// The number that MO voice call test would dial. Change this number to call a different number during MO voice call
#define TEST_DIALING_NUMBER "12345678"

#define SMS_RAW_DATA_LENGTH 28

/* Macro to signal a waiting thread*/
#define SIGNAL_MAIN(MSG) \
        pthread_mutex_lock(&async_resp);\
        g_status = MSG;\
        pthread_cond_signal(&async_resp_cond);\
        pthread_mutex_unlock(&async_resp);

/*===========================================================================
                    INTERNAL FUNCTION PROTOTYPES
===========================================================================*/
/*--------------------------------------------------------------------------
                               REQUEST FUNCTIONS
---------------------------------------------------------------------------*/
/* For modem bringup */
int dms_request_set_operating_mode(qmi_client_type user_handle, int op_mode);
int dms_request_get_operating_mode(qmi_client_type user_handle);

/* For voice */
void test_mo_voice_call_req (qmi_client_type user_handle);
void test_mt_voice_call_req (qmi_client_type user_handle);
void test_voice_call_hold_req (qmi_client_type user_handle);
void voice_call_dial_req ( qmi_client_type user_handle);
void voice_call_answer_req (qmi_client_type user_handle, uint8_t call_id);
void voice_call_end_req (qmi_client_type user_handle, uint8_t call_id);

/* For RSSI */
void test_nas_rssi_req(qmi_client_type user_handle);

/* For MO SMS */
void test_mo_sms(qmi_client_type user_handle);

/* For MT SMS */
static void test_mt_sms(qmi_client_type wms_user_handle);
/*--------------------------------------------------------------------------
                               CALLBACK FUNCTIONS
---------------------------------------------------------------------------*/
/* For modem bringup */
static void dms_cb_set_operating_mode ( qmi_client_type              user_handle,
                           unsigned long               msg_id,
                           void                        *resp_c_struct,
                           int                         resp_c_struct_len,
                           void                        *resp_cb_data,
                           qmi_client_error_type       tranp_err
                          );
static void dms_cb_get_operating_mode ( qmi_client_type              user_handle,
						   unsigned long               msg_id,
						   void                        *resp_c_struct,
						   int                         resp_c_struct_len,
						   void                        *resp_cb_data,
						   qmi_client_error_type       tranp_err
						  );

/* For Voice*/
static void cb_dial_call ( qmi_client_type              user_handle,
                           unsigned long                msg_id,
                           void                         *resp_c_struct,
                           int                          resp_c_struct_len,
                           void                         *resp_cb_data,
                           qmi_client_error_type        tranp_err
                          );
static void cb_answer_call ( qmi_client_type            user_handle,
                           unsigned long                msg_id,
                           void                         *resp_c_struct,
                           int                          resp_c_struct_len,
                           void                         *resp_cb_data,
                           qmi_client_error_type        tranp_err
                          );

static void cb_end_call ( qmi_client_type               user_handle,
                           unsigned long                msg_id,
                           void                         *resp_c_struct,
                           int                          resp_c_struct_len,
                           void                         *resp_cb_data,
                           qmi_client_error_type        tranp_err
                          );
/* For RSSI */
static void cb_nas_set_event_report ( qmi_client_type   user_handle,
                           unsigned long                msg_id,
                           void                         *resp_c_struct,
                           int                          resp_c_struct_len,
                           void                         *resp_cb_data,
                           qmi_client_error_type        tranp_err
                          );

/* For MO SMS*/
static void cb_mo_sms(qmi_client_type		wms_user_handle,
						   unsigned long				msg_id,
						   void							*resp_c_struct,
						   int 							resp_c_struct_len,
						   void							*resp_cb_data,
						   qmi_client_error_type		transp_err);
						   
/* For MT SMS */
static void cb_mt_sms(qmi_client_type			wms_user_handle,
							unsigned long				msg_id,
							void						*resp_c_struct,
							int 						resp_c_struct_len,
							void						*resp_cb_data,
							qmi_client_error_type		transp_err);

/*--------------------------------------------------------------------------
                               UNSOLICITED INDICATIONS
---------------------------------------------------------------------------*/
/* For modem bringup */
static void dms_unsolicited_event_ind (qmi_client_type  user_handle,
                                   unsigned long        msg_id,
                                   unsigned char        *ind_buf,
                                   int                  ind_buf_len,
                                   void                 *ind_cb_data
                                   );

/* For voice */
static void unsolicited_voice_ind ( qmi_client_type     user_handle,
                                     unsigned long      msg_id,
                                     unsigned char      *ind_buf,
                                     int                ind_buf_len,
                                     void               *ind_cb_data
								  );
/* For RSSI */
static void unsolicited_nas_ind (qmi_client_type 		user_handle,
                                   unsigned long        msg_id,
                                   unsigned char        *ind_buf,
                                   int                  ind_buf_len,
                                   void                 *ind_cb_data
                                );

/* For MT SMS */
static void sms_ind (qmi_client_type user_handle,
                                   unsigned long                  msg_id,
                                   unsigned char                  *ind_buf,
                                   int                            ind_buf_len,
                                   void                           *ind_cb_data
                                   );

/* Function starts the QMI VOICE test as a separate thread */
static void* voice_thread_func(void* user_choice);
								
/* Function starts the QMI RSSI test as a separate thread */
static void* nas_thread_func(void* user_choice);

/* Function starts the QMI MO/MT SMS tests as a separate thread */
static void* sms_thread_func(void* user_choice);

/*===========================================================================
                         GLOBAL VARIABLES
===========================================================================*/
/* For modem bringup */
dms_set_operating_mode_resp_msg_v01 g_dms_op_mode_req_response;	
dms_get_operating_mode_resp_msg_v01 g_dms_op_mode_get_req_response;	

/* For Voice */
voice_dial_call_req_msg_v01	 dial_call_req_msg;
voice_dial_call_resp_msg_v01 dial_call_resp_msg;
voice_answer_call_req_msg_v01 answer_call_req_msg;
voice_answer_call_resp_msg_v01 answer_call_resp_msg;
voice_end_call_req_msg_v01 end_call_req_msg;
voice_end_call_resp_msg_v01 end_call_resp_msg;

/* For RSSI*/
nas_set_event_report_req_msg_v01 nas_set_event_report_req_msg;
nas_set_event_report_resp_msg_v01 nas_set_event_report_resp_msg;

/* For MO SMS */
wms_raw_send_req_msg_v01			request_msg;
wms_raw_send_resp_msg_v01			response_msg;

/* For MT SMS */
wms_set_event_report_req_msg_v01 req_mt_msg;
wms_set_event_report_resp_msg_v01 resp_mt_msg;

/* Call ID - assigns an unique ID for each number dialled */
volatile uint8_t g_call_id = 255;
volatile uint8_t g_call_ended = 0;
volatile uint8_t g_nas_flag = 0;

/* Global  Mutex data */
pthread_mutex_t async_resp = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t async_resp_cond = PTHREAD_COND_INITIALIZER;

pthread_t voice_thread_handler;
pthread_t nas_thread_handler;
pthread_t sms_thread_handler;

qmi_client_type user_handle_voice=NULL;
qmi_client_type user_handle_dms=NULL;
qmi_client_type user_handle_nas=NULL;
qmi_client_type user_handle_wms=NULL;
qmi_idl_service_object_type wms_service_object;
qmi_idl_service_object_type nas_service_object;
qmi_idl_service_object_type voice_service_object;
qmi_idl_service_object_type dms_service_object;

/*===========================================================================
                         ENUMS
===========================================================================*/
volatile enum voice_call_status
{
    E_VOICE_NO_ERROR,
    E_VOICE_ERROR_IN_DIAL,
    E_VOICE_ERROR_IN_ANSWER,
    E_VOICE_ERROR_IN_END,
    E_VOICE_CONNECTED,
    E_VOICE_ALERTING,
    E_VOICE_ORIGINATING,
    E_VOICE_END,
    E_VOICE_ANSWER,
    E_VOICE_CALL_WAITING,
    E_VOICE_ERROR_UNKNOWN
}g_status;

volatile enum result_indicators
{
    E_SUCCESS,
    E_FAILURE
}g_result;


/*===========================================================================

                                FUNCTIONS

===========================================================================*/

/*=========================================================================
  FUNCTION:  main

===========================================================================*/
/*!
    @brief
    Starts and Handles the whole testing framework for MO voice.

    @return
    Zero on success or Appropriate error code on failure
*/
/*=========================================================================*/
int main(int argc, char *argv[])
{
    ENTRYTRACE;
	int user_response = 0;
	char user_response_string[USER_INPUT_MAX] = "0";
    qmi_client_error_type rc;
	qmi_client_type user_handle_dms;
    qmi_idl_service_object_type dms_service_object;
	pthread_condattr_t condAttr;
    pthread_mutexattr_t mutexAttr;
	int nas_thread = -1;
	int sms_thread = -1;
	int voice_thread = -1;
	
    /* Mutexes and condition variables are made process shared
       So that Call backs running in context of other process
       can signal the current process */
    //pthread_condattr_init(&condAttr);
    //pthread_condattr_setpshared(&condAttr, PTHREAD_PROCESS_SHARED); 

    pthread_cond_init (&async_resp_cond, &condAttr);
    pthread_mutexattr_init(&mutexAttr);
    pthread_mutexattr_setpshared(&mutexAttr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&async_resp, &mutexAttr);

    QMI_TEST_LOG1("Initializing QMI\n");
    /* Initialize the qmi datastructure(Once per process ) */
    qmi_handle = qmi_init(NULL, NULL);
    if (qmi_handle < 0)
    {
      QMI_TEST_LOG1("qmi message library not initialzed.");
      return qmi_handle;
    }

    nas_service_object = nas_get_service_object();

	
	/* Get the DMS service object */
    dms_service_object = dms_get_service_object();

    /* Initialize a DMS connection to first QMI control port */
    rc = qmi_client_init("rmnet0",
                        dms_service_object,
                        dms_unsolicited_event_ind,
                        dms_service_object,
                        &user_handle_dms );

    if (rc != QMI_NO_ERR)
    {
        if (qmi_handle >=0 )
        { 
          qmi_release(qmi_handle);
        }
		QMI_TEST_LOG2("Error: connection not Initialized. Error Code:%d\n",rc);
		return rc;
    }
    else
    {
		QMI_TEST_LOG1("Fetching current operating mode\n\n");
		dms_request_get_operating_mode(user_handle_dms);
		QMI_TEST_LOG1("Setting modem to LPM mode \n\n");
		dms_request_set_operating_mode(user_handle_dms, LPM_MODE);
		dms_request_get_operating_mode(user_handle_dms);
		QMI_TEST_LOG1("Setting modem to online mode \n\n");
		dms_request_set_operating_mode(user_handle_dms, ONLINE_MODE);
		dms_request_get_operating_mode(user_handle_dms);
    }
	
	while(1)
	{
		printf("************************************Qualcomm QMI TEST****************************\n");
		printf("Menu\n");
		printf("----\n");
		printf("[QmiTest] Press '1' to make MO voice Call\n");
		printf("[QmiTest] Press '2' to answer MT voice Call\n");
		printf("[QmiTest] Press '3' to view RSSI notifications from NAS\n");
		printf("[QmiTest] Press '4' to send a MO SMS\n");
		printf("[QmiTest] Press '5' to receive a MT SMS\n");
        printf("[Qmi Test] Press '6' to exit\n");
		scanf("%s", user_response_string);
		user_response = user_response_string[0];
		if ('1' == user_response || '2' == user_response)
		{
            voice_thread = pthread_create(&voice_thread_handler, NULL, voice_thread_func, (void*)user_response);
            if(0 == voice_thread)
            {
                rc = pthread_join(voice_thread_handler,NULL);
                if (rc != 0)
                {
                    QMI_TEST_LOG1("pthread_join failed\n");
                    exit(-1);
                }
            }
            else
            {
                QMI_TEST_LOG1("pthread_create failed\n");
            }
		}
		else if ('3' == user_response)
		{

//			printf(" This feature is temporary disabled\n");
///*
			// Create a thread to do the RSSI test 
			nas_thread = pthread_create(&nas_thread_handler, NULL, nas_thread_func, NULL);
            if(0 == nas_thread)
            {
				rc = pthread_join(nas_thread_handler,NULL);
                if (rc == -1)
                {
                    QMI_TEST_LOG1("pthread_join failed\n");
                    exit(-1);
                }

            }
            else
            {
                QMI_TEST_LOG1("pthread_create failed\n");
            }
//*/
		}
		else if ('4' == user_response)
		{
			/* Create a thread to perform the MO SMS test */
			sms_thread = pthread_create(&sms_thread_handler, NULL, sms_thread_func, (void*)user_response);
			if(0 == sms_thread)
            {
                rc = pthread_join(sms_thread_handler,NULL);
                if (rc == -1)
                {
                    QMI_TEST_LOG1("pthread_join failed\n");
                    exit(-1);
                }
            }
            else
            {
                QMI_TEST_LOG1("pthread_create failed\n");
            }
		}
		else if ('5' == user_response)
		{
			/* 	After modem send ACK to a MTSMS, modem gets into not functional state
			comment out the MTSMS test. To test MTSMS, uncomment below statement for 
			mt_sms_thread. After MTSMS is received, reboot the phone to start test again
			*/
			printf("\n\nMT SMS is currently comment out!! \n\n");
		}
        else if ('6' == user_response)
        {
            break;
        }
		else
		{
			printf("[Qmi Test] Invalid Choice\n");
		}
	}
	/* Release the Voice client in case the Voice thread was created*/
    if(NULL != user_handle_voice)
    {
        rc = qmi_client_release(user_handle_voice);
        if (rc < 0 )
        {
           QMI_TEST_LOG1("Qmi voice client release not successful\n");
        }
        else
        {
           QMI_TEST_LOG1("Qmi voice client release successful\n");
        }
    }
	
	/* Release the NAS client in case the NAS thread was created*/
    if(NULL != user_handle_nas)
    {
        rc = qmi_client_release(user_handle_nas);
        if (rc < 0 )
        {
           QMI_TEST_LOG1("Qmi nas client release not successful\n");
        }
        else
        {
           QMI_TEST_LOG1("Qmi nas client release successful\n");
        }
    }
	
	/* Release the SMS client in case the SMS thread was created*/
    if(NULL != user_handle_wms)
    {
        rc = qmi_client_release(user_handle_wms);
        if (rc < 0 )
        {
           QMI_TEST_LOG1("Qmi sms client release not successful\n");
        }
        else
        {
           QMI_TEST_LOG1("Qmi sms client release successful\n");
        }
    }

    if (qmi_handle >=0 )
    { 
      qmi_release(qmi_handle);
    }
	QMI_TEST_LOG1("Qmi release done\n");
	EXITTRACE;
	return 0;
}

/*=========================================================================
  FUNCTION:  dms_request_set_operating_mode

===========================================================================*/
/*!
    @brief
    Set the modem operating mode via QMI DMS

    @return
    None
*/
/*=========================================================================*/
int dms_request_set_operating_mode(qmi_client_type user_handle, int op_mode)
{
	ENTRYTRACE;
	dms_set_operating_mode_req_msg_v01 request_msg;
	qmi_client_error_type rc;
	qmi_txn_handle txn_handle;

	memset(&request_msg, 0, sizeof(request_msg) );
	memset(&g_dms_op_mode_req_response, 0, sizeof(g_dms_op_mode_req_response) );

	request_msg.operating_mode = op_mode;


	QMI_TEST_LOG2(".. user handle - %d", (int) user_handle);


	/* Send Async message */
	/* 	call QMI_DMS_SET_OPERATING_MODE_REQ, corresponding resp QMI_DMS_SET_OPERATING_MODE_RESP
		will be sent through callback dms_cb_set_operating_mode   */

	rc = qmi_client_send_msg_async(user_handle,
		QMI_DMS_SET_OPERATING_MODE_REQ,
		&request_msg,
		sizeof(request_msg),
		&g_dms_op_mode_req_response,
		sizeof(g_dms_op_mode_req_response),
		&dms_cb_set_operating_mode,
		NULL,
		&txn_handle
	);
	QMI_TEST_LOG2(".. QMI_DMS_SET_OPERATING_MODE_REQ issued - %d", (int) rc);

	EXITTRACE;
	return (int) rc;

}
/*=========================================================================
  FUNCTION:  dms_request_get_operating_mode

===========================================================================*/
/*!
    @brief
    Get the current modem operating mode via QMI DMS

    @return
    None
*/
/*=========================================================================*/
int dms_request_get_operating_mode(qmi_client_type user_handle)
{
	ENTRYTRACE;
	qmi_client_error_type rc;
	qmi_txn_handle txn_handle;

	memset(&g_dms_op_mode_get_req_response, 0, sizeof(g_dms_op_mode_get_req_response) );
	QMI_TEST_LOG2(".. user handle - %d", (int) user_handle);

	/* Send Async message */
	/* 	call QMI_DMS_GET_OPERATING_MODE_REQ, corresponding resp QMI_DMS_GET_OPERATING_MODE_RESP
		will be sent through callback dms_cb_get_operating_mode   */
	rc = qmi_client_send_msg_async(user_handle,
		QMI_DMS_GET_OPERATING_MODE_REQ,
		NULL,
		0,
		&g_dms_op_mode_get_req_response,
		sizeof(g_dms_op_mode_get_req_response),
		&dms_cb_get_operating_mode,
		NULL,
		&txn_handle
	);


	QMI_TEST_LOG2(".. QMI_DMS_GET_OPERATING_MODE_REQ issued - %d\n", (int) rc);
	EXITTRACE;
	return (int) rc;
}

/*=========================================================================
  FUNCTION:  test_mo_voice_call_req

===========================================================================*/
/*!
    @brief
    Make a MO voice call and end it.

    @return
    None
*/
/*=========================================================================*/
void test_mo_voice_call_req(qmi_client_type user_handle)
{
    ENTRYTRACE;
	struct timeval timeout;
	fd_set readfds;
    g_status = E_VOICE_NO_ERROR;
    g_call_id = 255;
    g_call_ended = 0;


    FD_ZERO(&readfds);	
    FD_SET(0, &readfds);
	
    pthread_mutex_lock(&async_resp);
    /*Dial Function is called here*/
    voice_call_dial_req(user_handle);

    /*Check whether the dial request was sent successfully or not*/
    if(E_VOICE_ERROR_IN_DIAL != g_status)
    {
       /*wait for the call back or modem events to signal the main thread*/

       pthread_cond_wait(&async_resp_cond, &async_resp);
       printf("[voiceTest] The MO Call is connected now\n");

       if(E_VOICE_CONNECTED == g_status)
       {
			int ret = 0; 	
		   printf("[voiceTest] Press any key to end MO call\n");
	
		   while(1)
		   {
			timeout.tv_sec = SELECT_TIMEOUT;
			timeout.tv_usec = 0;
			
			ret = select(1, &readfds, NULL, NULL, &timeout);
			if(FD_ISSET(0, &readfds))
			{
            char temp;
            temp = getchar();
            getchar();

				g_call_ended = 2;
				FD_ZERO(&readfds);
				break;
			}
			else if(0 == ret)
			{
				if(1 == g_call_ended)
				{
				   FD_ZERO(&readfds);
				   printf("[voiceTest] MO call ended by Called Party\n");			
				   break;
				}
			}
			else
			{
				QMI_TEST_LOG1("SELECT ERROR\n");
			}
			FD_SET(0, &readfds);		
		   }	

		   if(2 == g_call_ended)
		   {
				/*Call is ended here*/
				voice_call_end_req(user_handle, g_call_id);

			   if(E_VOICE_ERROR_IN_END != g_status)
			   {
				   pthread_cond_wait(&async_resp_cond, &async_resp);
				   pthread_mutex_unlock(&async_resp);
				   if(E_VOICE_END == g_status)
				   {
					  printf("Call Ended\n");
				   }
				   else
				   {
						QMI_TEST_LOG1("Call End failed\n");
				   }
			   }
			   else
			   {
				   pthread_mutex_unlock(&async_resp);
				   QMI_TEST_LOG1("Error while Ending\n");
			   }
			}
			else
			{
				pthread_mutex_unlock(&async_resp);
			}
		}
		else if(E_VOICE_ERROR_UNKNOWN == g_status)
		{
			pthread_mutex_unlock(&async_resp);
			QMI_TEST_LOG1("Error while making call\n");
		}
    }
    else
    {
       QMI_TEST_LOG1("Error While dialling\n");
       pthread_mutex_unlock(&async_resp);
    }
    EXITTRACE;
}

/*=========================================================================
  FUNCTION:  test_mt_voice_call_req

===========================================================================*/
/*!
    @brief
    Receive a MT voice call and end it.

    @return
    None
*/
/*=========================================================================*/
void test_mt_voice_call_req(qmi_client_type user_handle)
{
    ENTRYTRACE;
    char user_str[USER_INPUT_MAX] = "a";
	char user_response;
    struct timeval timeout;
    fd_set readfds;

    g_status = E_VOICE_NO_ERROR;
    g_call_id = 255;
    g_call_ended = 0;

	FD_SET(0, &readfds);
	FD_SET(0, &readfds);
    timeout.tv_sec = SELECT_TIMEOUT;
    timeout.tv_usec = 0;		
    
    pthread_mutex_lock(&async_resp);
    pthread_cond_wait(&async_resp_cond, &async_resp);

    printf("[voiceTest] Incoming Call is detected, Call ID: %d\n", g_call_id);
   	while(1)
   	{
		QMI_TEST_LOG2("[voiceTest] Incoming Call is detected, Call ID: %d\n", g_call_id);
		printf("[voiceTest] Press 'a' to answer\n");
		printf("[voiceTest] Press 'r' to reject\n");
		scanf("%s",user_str);
        user_response = user_str[0];
		if('a' == user_response || 'r' == user_response)
		{
			break;
		}
		else
		{
			printf("[voiceTest] Invalid Choice\n");
		}
	}
   
     if('a' == user_response)
    {
        QMI_TEST_LOG2("User pressed Answer for %d call_id\n", g_call_id); 
		/*Answer Function is called here*/
		voice_call_answer_req(user_handle, g_call_id);

		/*Check whether the answer request was sent successfully or not*/
		if(E_VOICE_ERROR_IN_ANSWER != g_status)
		{
			int ret = 0;
			/*wait for the call back or modem events to signal the main thread*/
			QMI_TEST_LOG1("\nWaiting for INDICATION **************\n");
			pthread_cond_wait(&async_resp_cond, &async_resp);
			printf("[voiceTest] The MT Call is connected\n");
			printf("[voiceTest] Press any key to end MT call\n");		
			while(1)
			{
				timeout.tv_sec = 5;
				timeout.tv_usec = 0;   

				ret = select(1, &readfds, NULL, NULL, &timeout);        
				if(FD_ISSET(0, &readfds))
				{
                char temp;
                temp = getchar();
                getchar();   
				g_call_ended = 2;
				FD_ZERO(&readfds);
				break;
				}
				else if(0 == ret)
				{
					if(1 == g_call_ended)
					{
					   FD_ZERO(&readfds);
					   printf("[voiceTest] MT call ended by Calling Party\n");			
					   break;
					}
				}		
				else
				{
					QMI_TEST_LOG1("SELECT ERROR\n");
				}
				FD_SET(0, &readfds);	
			}	


		   if(E_VOICE_CONNECTED == g_status || E_VOICE_END == g_status)
		   {	
			   if(2 == g_call_ended)
			   {
				   /*Call is ended here*/
				   voice_call_end_req(user_handle, g_call_id);

				   if(E_VOICE_ERROR_IN_END != g_status)
				   {
					   /*wait for the call back or modem events to signal*/
					   pthread_cond_wait(&async_resp_cond, &async_resp);
					   pthread_mutex_unlock(&async_resp);
					   if(E_VOICE_END == g_status)
					   {
						  printf("Call Ended\n");
					   }
					   else
					   {
						  QMI_TEST_LOG1("Call End failed\n");
					   }
				   }
				   else
				   {
					   pthread_mutex_unlock(&async_resp);
               QMI_TEST_LOG1("Error while Ending\n");
           }
				   }
       else
       {
           pthread_mutex_unlock(&async_resp);
			   }
		   }
		   else if(E_VOICE_ERROR_UNKNOWN == g_status)
		   {
			   pthread_mutex_unlock(&async_resp);
			   QMI_TEST_LOG1("Error while making call\n");
		   }

		}
		else
		{
		   QMI_TEST_LOG1("Error While sending answer request\n");
		}
    }
    EXITTRACE;
}

/*=========================================================================
  FUNCTION:  voice_call_dial_req

===========================================================================*/
/*!
    @brief
    Dial a MO voice call with a pre-set number.

    @return
    None
*/
/*=========================================================================*/
void voice_call_dial_req( qmi_client_type user_handle )
{
   ENTRYTRACE;
   qmi_client_error_type rc;
   qmi_txn_handle txn_handle;

   /* Setting the request/response message structure to zero */
   memset(&dial_call_req_msg, 0, sizeof(voice_dial_call_req_msg_v01));
   memset(&dial_call_resp_msg, 0, sizeof(voice_dial_call_resp_msg_v01));

   /*Set the number to be dialler here*/
   strcpy(dial_call_req_msg.calling_number, TEST_DIALING_NUMBER);

   /* Send Async message */
   rc =  qmi_client_send_msg_async(user_handle,
                                   QMI_VOICE_DIAL_CALL_REQ,
                                   &dial_call_req_msg,
                                   sizeof(voice_dial_call_req_msg_v01),
                                   &dial_call_resp_msg,
                                   sizeof(voice_dial_call_resp_msg_v01),
                                   &cb_dial_call,
                                   NULL,
                                   &txn_handle);

   if (rc == QMI_NO_ERR )
   {
       QMI_TEST_LOG1("Dial Request sent\n");
   }
   else
   {
       QMI_TEST_LOG2("ERROR in sending the Dial Request, ERROR CODE:%d",rc);
       g_status = E_VOICE_ERROR_IN_DIAL;
   }
   EXITTRACE;
}

/*=========================================================================
  FUNCTION:  voice_call_answer_req

===========================================================================*/
/*!
    @brief
    Answer a MT voice call.

    @return
    None
*/
/*=========================================================================*/
void voice_call_answer_req( qmi_client_type user_handle, uint8_t call_id )
{
	ENTRYTRACE;
	qmi_client_error_type rc;
	qmi_txn_handle txn_handle;

	/* Setting the request/response message structure to zero */
	memset(&answer_call_req_msg,0,sizeof(voice_answer_call_req_msg_v01));
	memset(&answer_call_resp_msg,0,sizeof(voice_answer_call_resp_msg_v01));

	QMI_TEST_LOG2("Sending Answer Call request to modem for call ID: %d\n",call_id);
	/*Set the call-id to be answered here*/
	answer_call_req_msg.call_id = call_id;

	/* Send Async message */
	rc =  qmi_client_send_msg_async(user_handle,
								   QMI_VOICE_ANSWER_CALL_REQ,
								   &answer_call_req_msg,
								   sizeof(voice_answer_call_req_msg_v01),
								   &answer_call_resp_msg,
								   sizeof(voice_answer_call_resp_msg_v01),
								   &cb_answer_call,
								   NULL,
								   &txn_handle);
	QMI_TEST_LOG2("Answer Call request SENT for call ID: %d\n",call_id);

	if (rc == QMI_NO_ERR )
	{
	   QMI_TEST_LOG1("Answer Call Request sent\n");
	}
	else
	{
	   QMI_TEST_LOG2("ERROR in sending the Answer Request, ERROR CODE:%d",rc);
	   g_status = E_VOICE_ERROR_IN_ANSWER;
	}

	EXITTRACE;
}

/*=========================================================================
  FUNCTION:  voice_call_end_req

===========================================================================*/
/*!
    @brief
    End a MO voice call with a pre-set Call ID

    @return
    None
*/
/*=========================================================================*/
void voice_call_end_req (qmi_client_type user_handle, uint8_t call_id)
{
	ENTRYTRACE;
	qmi_client_error_type rc;
	qmi_txn_handle txn_handle;

	/* Setting the request/response message structure to zero */
	memset(&end_call_req_msg, 0, sizeof(voice_end_call_req_msg_v01));
	memset(&end_call_resp_msg, 0, sizeof(voice_end_call_resp_msg_v01));

	/* Set the call ID to end the call*/
	end_call_req_msg.call_id = call_id;

	/* Send Async message */
	rc = qmi_client_send_msg_async(user_handle,
		QMI_VOICE_END_CALL_REQ,
		&end_call_req_msg,
		sizeof(voice_end_call_req_msg_v01),
		&end_call_resp_msg,
		sizeof(voice_end_call_resp_msg_v01),
		&cb_end_call,
		NULL,
		&txn_handle
	);

	if (rc == QMI_NO_ERR )
	{
		QMI_TEST_LOG1("End Call Request sent\n");
	}
	else
	{
		QMI_TEST_LOG2("ERROR in sending the End Call Request, ERROR CODE: %d",rc);
		g_status = E_VOICE_ERROR_IN_END;
	}
	EXITTRACE;
}

/*=========================================================================
  FUNCTION:  handle_nas_rssi_subscription

===========================================================================*/
/*!
    @brief
    Subscribes for RSSI indications from NAS
	
    @return
    None
*/
/*=========================================================================*/
void handle_nas_rssi_subscription(qmi_client_type user_handle)
{
	ENTRYTRACE;
    qmi_txn_handle txn_handle;
	
	/* Setting the request/response message structure to zero */
	memset(&nas_set_event_report_req_msg,0,sizeof(nas_set_event_report_req_msg));
	memset(&nas_set_event_report_resp_msg,0,sizeof(nas_set_event_report_resp_msg));
	
	QMI_TEST_LOG1("Subscribing for NAS indications\n");
	/* Tell NAS that RSSI TLV is valid */
	nas_set_event_report_req_msg.rssi_indicator_valid = 1;
	/* Tell NAS that we want to receive NAS indications */
	nas_set_event_report_req_msg.rssi_indicator.report_rssi = 1;
	/* Tell NAS that send us an indication when the signal strength changes by 10 dbm */
	nas_set_event_report_req_msg.rssi_indicator.rssi_delta = 10;

	/* Send Async message */
	/* 	call QMI_NAS_SET_EVENT_REPORT_REQ_MSG, corresponding resp QMI_NAS_SET_EVENT_REPORT_RESP_MSG
		will be sent through callback cb_nas_set_event_report   */


	qmi_client_error_type rc =  qmi_client_send_msg_async(user_handle,
                                   QMI_NAS_SET_EVENT_REPORT_REQ_MSG,
                                   &nas_set_event_report_req_msg,
                                   sizeof(nas_set_event_report_req_msg),
                                   &nas_set_event_report_resp_msg,
                                   sizeof(nas_set_event_report_resp_msg),
                                   &cb_nas_set_event_report,
                                   NULL,
                                   &txn_handle);
		
	QMI_TEST_LOG2("Asynchronous message sent to NAS returned %d\n", rc);

	rc = rc;

	EXITTRACE;
}

/*=========================================================================
  FUNCTION:  test_nas_rssi_req

===========================================================================*/
/*!
    @brief
    Set up testing of RSSI notifications from NAS (Network Access Service)

    @return
    None
*/
/*=========================================================================*/
void test_nas_rssi_req(qmi_client_type user_handle)
{
	ENTRYTRACE;
	char user_response[USER_INPUT_MAX] = "0";
    g_nas_flag = 1;
	handle_nas_rssi_subscription(user_handle);
	while (1)
	{
		scanf("%s", user_response);
		
		if ('e' == user_response[0])
		{
			g_nas_flag = 0;
			break;
		}
		else
		{
			EXIT_IND_MODE_PRINTF;
		}
	}
    EXITTRACE;
}

/*=========================================================================
  FUNCTION:  test_mo_sms

===========================================================================*/
/*!
    @brief
    Test sending of a MO SMS

    @return
    None
*/
/*=========================================================================*/
void test_mo_sms(qmi_client_type wms_user_handle)
{
	ENTRYTRACE;
	qmi_client_error_type	rc;
	qmi_txn_handle			txn_handle;
	int i ;
	/* zero out request/response message structure */
	memset(&request_msg, 0, sizeof(wms_raw_send_req_msg_v01));
	memset(&response_msg, 0, sizeof(wms_raw_send_resp_msg_v01));
	/* mandatory */
	request_msg.raw_message_data.format = 0x00; /* CDMA */
	request_msg.raw_message_data.raw_message_len = SMS_RAW_DATA_LENGTH;
	
	/* CDMA PDU hardcoded */
	uint8_t arr[SMS_RAW_DATA_LENGTH] = {0x00,/* Point-to-Point message type */
						 0x00,0x02,0x10,0x02, /* TeleService ID 4098*/
                         0x04,0x03,0x00,0xC4,0x8C,	/* Destination Address */                         
						 0x08,0x10,0x00,0x03,0x20,0x56,0x10,0x01,0x03,0x10,0x0C,0x10,0x08,0x01,0x00,0x0C,0x01,0x00};/* Bearer Data */
	QMI_TEST_LOG1("INSIDE TEST_MO_SMS\n");
	for (i = 0; i < SMS_RAW_DATA_LENGTH; i++)
	{
		QMI_TEST_LOG3("%d %d\n", i, arr[i]);
		request_msg.raw_message_data.raw_message[i] = arr[i];
	}
	QMI_TEST_LOG1(" BEFORE qmi_client_send_msg_async \n");
	/* call QMI_WMS_RAW_SEND_REQ, corresponding resp QMI_WMS_RAW_SEND_RESP will be sent
	through callback cb_mo_sms	*/
	
	rc = qmi_client_send_msg_async(wms_user_handle,
									QMI_WMS_RAW_SEND_REQ,
									&request_msg,
									sizeof(wms_raw_send_req_msg_v01),
									&response_msg,
									sizeof(wms_raw_send_resp_msg_v01),
									&cb_mo_sms,
									NULL,
									&txn_handle);
	QMI_TEST_LOG1("AFTER qmi_client_send_msg_async \n");									
	
	if(rc == QMI_NO_ERR)
	{
		QMI_TEST_LOG1(" send sms....\n");
	}
	else
	{
		QMI_TEST_LOG2("send sms failed ERROR CODE: %d", rc);
	}
	EXITTRACE;
}
/*=========================================================================
  FUNCTION:  mt_sms_req

===========================================================================*/
/*!
    @brief
    Test receiving of a MT SMS

    @return
    None
*/
/*=========================================================================*/
static void test_mt_sms(qmi_client_type wms_user_handle)
{
	ENTRYTRACE;
	qmi_client_error_type	rc;
	qmi_txn_handle			txn_handle;
	char user_response[USER_INPUT_MAX];
	
	memset(&req_mt_msg, 0, sizeof(wms_set_event_report_req_msg_v01));
	memset(&resp_mt_msg, 0, sizeof(wms_set_event_report_resp_msg_v01));
	
	req_mt_msg.report_mt_message_valid = 1; /* request to report mt sms */
	req_mt_msg.report_mt_message = 1; /*  Report new MT messages
	 0x00 - Disable
	 0x01 - Enable*/
		
	/*  call QMI_WMS_SET_EVENT_RESPORT_REQ to get event report indication
		corresponding QMI_WMS_SET_EVENT_REPORT_RESP is sent through 
		mt_sms_req_callback
	 */
	rc = qmi_client_send_msg_async(wms_user_handle,
									QMI_WMS_SET_EVENT_REPORT_REQ, 
									&req_mt_msg, 
									sizeof(wms_set_event_report_req_msg_v01),
									&resp_mt_msg,
									sizeof(wms_set_event_report_resp_msg_v01),
									&cb_mt_sms,
									NULL,
									&txn_handle);
	if(rc == QMI_NO_ERR)
	{
		QMI_TEST_LOG1(" request for mt sms....\n");
	}
	else
	{
		QMI_TEST_LOG2(" request for mt sms failed ERROR CODE: %d\n", rc);
	}
	EXIT_IND_MODE_PRINTF;
	while (1)
	{
		scanf("%s", user_response);
		
		if ('e' == user_response[0])
		{
			break;
		}
		else
		{
			EXIT_IND_MODE_PRINTF;
		}
	}
	EXITTRACE;
}

/*=========================================================================
  FUNCTION:  voice_thread_func

===========================================================================*/
/*!
    @brief
    Function called on VOICE thread initiaion

    @return
    None
*/
/*=========================================================================*/
static void* voice_thread_func (void *user_choice)		
{
    ENTRYTRACE;
	char response = (char) (int)user_choice;

    if(NULL == user_handle_voice)
    {
        /* Get the NAS service object */
        voice_service_object = voice_get_service_object();

        /* Initialize a NAS connection to first QMI control port */
        qmi_client_error_type rc = qmi_client_init("rmnet0",
                            voice_service_object,
                            unsolicited_voice_ind,
                            voice_service_object,
                            &user_handle_voice );

        if (rc != QMI_NO_ERR)
        {
           if (qmi_handle >=0 )
           { 
             qmi_release(qmi_handle);
           }
           QMI_TEST_LOG2("Error: Voice Connection not Initialized. Error Code:%d\n", rc);
           EXITTRACE;
		   return NULL;
        }
        QMI_TEST_LOG2("Voice Connection Initialized. User Handle: %d\n", (int) user_handle_voice);
    }

    if('1' == response)
    {
        test_mo_voice_call_req (user_handle_voice);
    }
	else if('2' == response)
    {
        test_mt_voice_call_req (user_handle_voice);    
    }
	EXITTRACE;
	return NULL;
}

/*=========================================================================
  FUNCTION:  nas_thread_func

===========================================================================*/
/*!
    @brief
    Function called on NAS thread initiaion

    @return
    None
*/
/*=========================================================================*/
static void* nas_thread_func (void* user_choice)		
{
	if(NULL == user_handle_nas)
	{
		// Get the NAS service object 
		nas_service_object = nas_get_service_object();

		// Initialize a NAS connection to first QMI control port 
		qmi_client_error_type rc = qmi_client_init("rmnet0",
							nas_service_object,
							unsolicited_nas_ind,
							nas_service_object,
							&user_handle_nas );

		if (rc != QMI_NO_ERR)
		{
           if (qmi_handle >=0 )
           { 
             qmi_release(qmi_handle);
           }
		   QMI_TEST_LOG2("Error: NAS Connection not Initialized. Error Code:%d\n", rc);
		   return NULL;
		}
		QMI_TEST_LOG2("NAS Connection Initialized. User Handle: %d\n", (int) user_handle_nas);
	}
	test_nas_rssi_req(user_handle_nas);
	return NULL;
}

/*=========================================================================
  FUNCTION:  sms_thread_func

===========================================================================*/
/*!
    @brief
    Function called on SMS thread initiaion

    @return
    None
*/
/*=========================================================================*/
static void* sms_thread_func (void* user_choice)		
{
    char response = (char) (int)user_choice;
    if(NULL == user_handle_wms)
{
	/* Get the WMS service object */
    wms_service_object = wms_get_service_object();

    /* Initialize a SMS connection to first QMI control port */
    qmi_client_error_type rc = qmi_client_init("rmnet0",
                        wms_service_object,
                        sms_ind,
                        wms_service_object,
                        &user_handle_wms );

	if (rc != QMI_NO_ERR)
    {
       if (qmi_handle >=0 )
       { 
         qmi_release(qmi_handle);
       }
       QMI_TEST_LOG2("Error: WMS Connection not Initialized. Error Code:%d\n", rc);
       return NULL;
    }
	QMI_TEST_LOG2("WMS Connection Initialized. User Handle: %d\n", (int) user_handle_wms);
    }
	if('4' == response)
    {
        test_mo_sms(user_handle_wms);
    }    
    else if('5' == response)
    {
		test_mt_sms(user_handle_wms);
    }
	return NULL;
}
/*--------------------------------------------------------------------------
                               CALLBACK FUNCTIONS
---------------------------------------------------------------------------*/
/*=========================================================================
  FUNCTION:  dms_cb_set_operating_mode
===========================================================================*/
/*!
    @brief
    Call Back function for set_operating_mode request to DMS

    @return
    None
*/
/*=========================================================================*/
static void dms_cb_set_operating_mode ( qmi_client_type              user_handle,
                           unsigned long               msg_id,
                           void                        *resp_c_struct,
                           int                         resp_c_struct_len,
                           void                        *resp_cb_data,
                           qmi_client_error_type       tranp_err
                          )
{
	ENTRYTRACE;
	dms_set_operating_mode_resp_msg_v01* resp = (dms_set_operating_mode_resp_msg_v01*)resp_c_struct;

	QMI_TEST_LOG2("result = %d\n", resp->resp.result);
	QMI_TEST_LOG2("Error = %d\n", resp->resp.error);
	QMI_TEST_LOG2("Error = %d\n", (int)tranp_err);

	resp = resp;

	EXITTRACE;
}

/*=========================================================================
  FUNCTION:  dms_cb_get_operating_mode
===========================================================================*/
/*!
    @brief
    Call Back function for get_operating_mode request to DMS

    @return
    None
*/
/*=========================================================================*/
static void dms_cb_get_operating_mode ( qmi_client_type              user_handle,
                           unsigned long               msg_id,
                           void                        *resp_c_struct,
                           int                         resp_c_struct_len,
                           void                        *resp_cb_data,
                           qmi_client_error_type       tranp_err
                          )
{
	ENTRYTRACE;
		dms_get_operating_mode_resp_msg_v01* resp = (dms_get_operating_mode_resp_msg_v01*)resp_c_struct;
        QMI_TEST_LOG2(".. reported operating mode - %d\n", resp->operating_mode );
	resp = resp;
        EXITTRACE;
}

/*=========================================================================
  FUNCTION:  cb_dial_call
===========================================================================*/
/*!
    @brief
    Call Back function for dial call req

    @return
    None
*/
/*=========================================================================*/
void cb_dial_call (qmi_client_type              user_handle,
				   unsigned long                msg_id,
                   void                         *resp_c_struct,
				   int                          resp_c_struct_len,
				   void                         *resp_cb_data,
				   qmi_client_error_type        tranp_err )
{
	ENTRYTRACE;
	/*  Temp fix: Accept QMI_IDL_MISSING_TLV to tolerate QMI bug returning such error
		even when TLV is present and intepreted correctly
	 */
    if (1 || (tranp_err ==  QMI_NO_ERR || tranp_err == QMI_IDL_MISSING_TLV ))
    {
        if(1 || QMI_RESULT_SUCCESS ==
            (((voice_dial_call_resp_msg_v01*)resp_c_struct)->resp).result)
        {
            g_call_id = ((voice_dial_call_resp_msg_v01*)resp_c_struct)->call_id;
			QMI_TEST_LOG2("Dial Call reponse received from modem. Call ID: %d\n",g_call_id);
            return;
        }
    }
    QMI_TEST_LOG2("Transport Error in call back for dial. Error code: %d\n", tranp_err);
    SIGNAL_MAIN(E_VOICE_ERROR_IN_DIAL);
	EXITTRACE;
}

/*=========================================================================
  FUNCTION:  cb_answer_call
===========================================================================*/
/*!
    @brief
    Call Back function for answer call req

    @return
    None
*/
/*=========================================================================*/
void cb_answer_call (qmi_client_type              user_handle,
					unsigned long                msg_id,
					void                         *resp_c_struct,
					int                          resp_c_struct_len,
					void                         *resp_cb_data,
					qmi_client_error_type        tranp_err )
{
	ENTRYTRACE;
    if (1 || (tranp_err ==  QMI_NO_ERR ))
    {
        if(QMI_RESULT_SUCCESS ==
            (((voice_answer_call_resp_msg_v01*)resp_c_struct)->resp).result )
        {
            EXITTRACE;
			return;
        }
    }
    QMI_TEST_LOG2("Transport Error in call back for answer. Error code: %d\n",tranp_err);
    SIGNAL_MAIN(E_VOICE_ERROR_IN_ANSWER);
	EXITTRACE;
}

/*=========================================================================
  FUNCTION:  cb_end_call
===========================================================================*/
/*!
    @brief
    Call Back function for end call req

    @return
    None
*/
/*=========================================================================*/
void cb_end_call (qmi_client_type              user_handle,
					unsigned long               msg_id,
					void                        *resp_c_struct,
					int                         resp_c_struct_len,
					void                        *resp_cb_data,
					qmi_client_error_type       tranp_err
				  )
{
    if (1 || (tranp_err ==  QMI_NO_ERR ))
    {
        if(QMI_RESULT_SUCCESS ==
            (((voice_end_call_resp_msg_v01*)resp_c_struct)->resp).result )
        {
			/* g_call_id = ((voice_end_call_resp_msg_v01*)resp_c_struct)->call_id; */
            return;
        }
    }
    QMI_TEST_LOG2("Transport Error in call back for end. Error code: %d\n",tranp_err);
    SIGNAL_MAIN(E_VOICE_ERROR_IN_END);
}

/*=========================================================================
  FUNCTION:  cb_nas_set_event_report
===========================================================================*/
/*!
    @brief
    Call Back function for Set Event Report Req to NAS

    @return
    None
*/
/*=========================================================================*/
void cb_nas_set_event_report (qmi_client_type            user_handle,
									unsigned long               msg_id,
									void                        *resp_c_struct,
									int                         resp_c_struct_len,
									void                        *resp_cb_data,
									qmi_client_error_type       tranp_err
									)
{
    ENTRYTRACE;
	if ((tranp_err ==  QMI_NO_ERR )||(tranp_err == QMI_IDL_MISSING_TLV))
    {
        if(QMI_RESULT_SUCCESS == (((nas_set_event_report_resp_msg_v01*)resp_c_struct)->resp).result )
        {
            QMI_TEST_LOG1("NAS Set Event Report Response from Modem is success\n");			
        }
		else
		{ 
			QMI_TEST_LOG2("NAS Set Event Report Response from Modem is Failure with error code %d\n",
			(((nas_set_event_report_resp_msg_v01*)resp_c_struct)->resp).result);
		}
    }
    else
	{
		QMI_TEST_LOG2("NAS Set Event Report Response from Modem received transport error = %d\n", tranp_err);
	}
	EXITTRACE;
} 
/*=========================================================================
  FUNCTION:  cb_mo_sms
===========================================================================*/
/*!
    @brief
    Call Back function for SMS RAW SEND Request
	The callback block should contain Response for SMS RAW SEND Req


    @return
    None
*/
/*=========================================================================*/
void cb_mo_sms(qmi_client_type	wms_user_handle,
									unsigned long	msg_id,
									void			*resp_c_struct,
									int 			resp_c_struct_len,
									void			*resp_cb_data,
									qmi_client_error_type	transp_err)
{
	if (transp_err ==  QMI_NO_ERR )
	{
		QMI_TEST_LOG1("QMI CLIENT NO ERROR \n");		
    }
	else
	{
		QMI_TEST_LOG2("QMI ERROR %d\n\n\n", transp_err);
	}
	wms_raw_send_resp_msg_v01	*resp_c_ptr;
	resp_c_ptr = (wms_raw_send_resp_msg_v01*)resp_c_struct;
	QMI_TEST_LOG2("MESSAGE ID %d\n", resp_c_ptr->message_id);
	QMI_TEST_LOG2("RESULT CODE %d\n", resp_c_ptr->resp.error);
	QMI_TEST_LOG2("CAUSE CODE VALID %d\n", resp_c_ptr->cause_code_valid);
	QMI_TEST_LOG2("CAUSE_CODE %d\n", resp_c_ptr->cause_code);
	QMI_TEST_LOG2("ERROR_CLASS_VALID %d\n", resp_c_ptr->error_class_valid);
	QMI_TEST_LOG2("ERROR_CLASS %d\n", resp_c_ptr->error_class);
	if(resp_c_ptr->cause_code_valid == 1)
	{
		QMI_TEST_LOG2("CAUSE CODE %d\n", resp_c_ptr->cause_code);
	}
	if(resp_c_ptr->error_class_valid == 1)
	{
		QMI_TEST_LOG2("ERROR CLASS %d\n", resp_c_ptr->error_class);
	}
}
/*=========================================================================
  FUNCTION:  mt_sms_req_callback
===========================================================================*/
/*!
    @brief
    Call Back function for Event Report Request
	Event Report will be sent through sms_ind after this callback gets
	SUCCESS results

    @return
    None
*/
/*=========================================================================*/
static void cb_mt_sms(qmi_client_type	wms_user_handle,
									unsigned long	msg_id,
									void			*resp_c_struct,
									int 			resp_c_struct_len,
									void			*resp_cb_data,
									qmi_client_error_type	transp_err)							
{
	QMI_TEST_LOG1("\n\n\nMT_SMS_REQUEST_CALLBACK\n\n");
	if(transp_err == QMI_NO_ERR)
	{
		QMI_TEST_LOG1("QMI NO ERROR\n");
	}
	else
	{
		/* no mandatory field */
		QMI_TEST_LOG2("QMI ERROR %d\n\n", transp_err);		
	}
	
}

/*--------------------------------------------------------------------------
                               MODEM INDICATIONS
---------------------------------------------------------------------------*/
/*=========================================================================
  FUNCTION:  dms_unsolicited_event_ind
===========================================================================*/
/*!
    @brief
    Unsolicited modem indications from DMS are handled here.

    @return
    None
*/
/*=========================================================================*/
static void dms_unsolicited_event_ind (qmi_client_type user_handle,
                                   unsigned long                  msg_id,
                                   unsigned char                  *ind_buf,
                                   int                            ind_buf_len,
                                   void                           *ind_cb_data
                                   )
{
	ENTRYTRACE;

       qmi_client_error_type rc;

       char ind_msg[256]; 
	QMI_TEST_LOG2(".. message ID:%ld\n",msg_id);

        rc = qmi_client_message_decode(user_handle,
                          QMI_IDL_INDICATION,
                          QMI_DMS_EVENT_REPORT_IND,
                          (void*)ind_buf,
                          ind_buf_len,
                          (void *)&ind_msg,
                          sizeof(ind_msg));

       QMI_TEST_LOG2("..decode res %d \n", rc);


     EXITTRACE;
}

/*=========================================================================
  FUNCTION:  unsolicited_voice_ind
===========================================================================*/
/*!
    @brief
    Unsolicited modem indications related to voice calls are handled here.

    @return
    None
*/
/*=========================================================================*/
void unsolicited_voice_ind (qmi_client_type user_handle,
							   unsigned long                  msg_id,
							   unsigned char                  *ind_buf,
							   int                            ind_buf_len,
							   void                           *ind_cb_data
							   )
{
	ENTRYTRACE;

    
	voice_call_status_ind_msg_v01 *call_status_ind;
	voice_call_status_ind_msg_v01 ind_msg;
    call_status_ind = &ind_msg;
    qmi_client_error_type rc;
	
	memset(&ind_msg,0,sizeof(voice_call_status_ind_msg_v01));

    /*This is the API used to decode the the unsolicited events from the modem*/
    rc = qmi_client_message_decode(user_handle,
                          QMI_IDL_INDICATION,
                          msg_id,
                          (void*)ind_buf,
                          ind_buf_len,
                          (void *)&ind_msg,
                          sizeof(voice_call_status_ind_msg_v01));

    /* If there is any issue with Decode return from here */
    if(QMI_NO_ERR != rc)
    {
		QMI_TEST_LOG2("Decode of Indication message returned error: %d\n",rc);
		EXITTRACE;
        return;
    }

    /* Handle the various cases based on the message ID*/
    switch(msg_id)
    {
    case QMI_VOICE_CALL_STATUS_IND:
        /* Ind_msg has the result, cast and read the result */
		QMI_TEST_LOG2("Indication received for Call ID: %d\n", call_status_ind->call_status.call_id);

        /*Make sure the events are for the same call ID that is set in global*/
        if((call_status_ind->call_status.call_id == g_call_id)
            || (255==g_call_id))
        {
            switch(call_status_ind->call_status.call_event)
            {
            case CALL_STATUS_ANSWER:
				QMI_TEST_LOG1("Answer Indication received\n");
                break;

            case CALL_STATUS_ORIGINATION:
				QMI_TEST_LOG1("Originating call Indication received\n");
                QMI_TEST_LOG1("Signaling the Main thread\n");
                g_status = E_VOICE_ORIGINATING;
                break;

            case CALL_STATUS_CONNECT:
                if(1 || E_VOICE_ORIGINATING == g_status ||
                   E_VOICE_CALL_WAITING == g_status)
                {
					QMI_TEST_LOG1("Call connected Indication received\n");
                    QMI_TEST_LOG1("Signaling the Main thread\n");
                    SIGNAL_MAIN(E_VOICE_CONNECTED);
                }
                else
                {
                    QMI_TEST_LOG1("Unexpected State Transition\n");
                    SIGNAL_MAIN(E_VOICE_ERROR_IN_DIAL);
                }
                break;

            case CALL_STATUS_END:
                if(E_VOICE_CONNECTED == g_status)
                {
					QMI_TEST_LOG1("Call end Indication received\n");
                    if(2 == g_call_ended)
                    {
						QMI_TEST_LOG1("Signaling the Main thread\n");                    
						SIGNAL_MAIN(E_VOICE_END);
                    }
                    else
                    {
                        g_call_ended=1;
                    }
                    
                }
                else
                {
                    QMI_TEST_LOG1("Unexpected State Transition\n");
                    SIGNAL_MAIN(E_VOICE_ERROR_IN_END);
                }
                break;

			case CALL_STATUS_ALERTING:
				g_call_id = call_status_ind->call_status.call_id;
        QMI_TEST_LOG2("Alert Indication Received %d\n",g_call_id);
				SIGNAL_MAIN(E_VOICE_ALERTING);
				break;

            default:
                QMI_TEST_LOG2("Unexpected Call Event %d\n", call_status_ind->call_status.call_event);
                break;
            }
        }
        else
        {
            QMI_TEST_LOG2("Unexpected Call ID: %d\n", call_status_ind->call_status.call_id);
            SIGNAL_MAIN(E_VOICE_ERROR_UNKNOWN);
        }
        break;

    default:
        QMI_TEST_LOG2("Unexpected Indication: %ld\n", msg_id);
		EXITTRACE;
        return;
    }
	EXITTRACE;
}
/*=========================================================================
  FUNCTION:  unsolicited_nas_ind
===========================================================================*/
/*!
    @brief
    Unsolicited modem Indications from NAS are handled here.

    @return
    None
*/
/*=========================================================================*/

static void unsolicited_nas_ind (qmi_client_type 			   user_handle,
								unsigned long                  msg_id,
								unsigned char                  *ind_buf,
								int                            ind_buf_len,
								void                           *ind_cb_data
								)
{
    ENTRYTRACE;
	if(1 == g_nas_flag)
    {
		EXIT_IND_MODE_PRINTF;
		QMI_TEST_LOG2("Received message ID:%ld\n from NAS\n", msg_id);
		// We only decode and analyze the indication if it's QMI_NAS_EVENT_REPORT_IND_MSG_ID
		// If not, we just ignore it
		
		if (msg_id == QMI_NAS_EVENT_REPORT_IND_MSG_ID)
		{
			nas_event_report_ind_msg_v01 decoded_ind;
			qmi_client_error_type rc;

			memset(&decoded_ind, 0, sizeof(decoded_ind));
		 
			//This is the API used to decode the the unsolicited events from the modem
			rc = qmi_client_message_decode(user_handle,
								  QMI_IDL_INDICATION,
								  msg_id,
								  (void*)ind_buf,
								  ind_buf_len,
								  (void *)&decoded_ind,
								  sizeof(decoded_ind));

			// If there is any issue with Decode return from here 
					
			//Temp fix: Accept QMI_IDL_MISSING_TLV to tolerate QMI bug returning such error
			//even when TLV is present and intepreted correctly	

			if(QMI_NO_ERR != rc && QMI_IDL_MISSING_TLV != rc)
			{
				QMI_TEST_LOG2("Decode of NAS Indication message returned error: %d\n",rc);
				EXITTRACE;
				return;
			}
			QMI_TEST_LOG1("NAS Indication message decode OK \n");

			// Now decoded_ind has been populated
			// Since we only set flag for RSSI, we should make sure that it's enabled in the indication response
			if (decoded_ind.rssi_valid == 1)
			{
				printf("RSSI value is -%d dbm\n", decoded_ind.rssi.rssi);
				switch (decoded_ind.rssi.radio_if)
				{
				case 0:
					printf("Phone in No Service Mode\n");
					break;
				case 1:
					printf("Phone in CDMA 2000 (1X) Mode\n");
					break;
				case 2:
					printf("Phone in CDMA 2000 HRPD (1xEV-DO) Mode\n");
					break;
				case 3:
					printf("Phone in AMPS Mode\n");
					break;
				case 4:
					printf("Phone in GSM Mode\n");
					break;
				case 5:
					printf("Phone in UMTS Mode\n");
					break;
				default:
					printf("Invalid RF Mode!!!\n");
				}
			}
			else
			{
				QMI_TEST_LOG1("Expected RSSI info to be valid but didn't find it so!\n");
			}
		}
		else
		{
			QMI_TEST_LOG1("NAS message ignored\n");
		}
	}
	else
	{
		QMI_TEST_LOG1("Incoming NAS indication supressed\n");
	}
	EXITTRACE;
}

/*=========================================================================
  FUNCTION:  sms_ind
===========================================================================*/
/*!
    @brief
    Unsolicited modem indications for MT SMS are handled here

    @return
    None
*/
/*=========================================================================*/
static void sms_ind (qmi_client_type user_handle,
                                   unsigned long                  msg_id,
                                   unsigned char                  *ind_buf,
                                   int                            ind_buf_len,
                                   void                           *ind_cb_data
                                   )

{
	wms_event_report_ind_msg_v01* mt_sms;
	qmi_client_error_type	rc;
	uint32_t i, j;
	int k;
	
	QMI_TEST_LOG1("\n\n\n SMS_INDICATION_MESSAGE\n\n\n");
	mt_sms = malloc(sizeof(wms_event_report_ind_msg_v01));
	memset(mt_sms, 0, sizeof(wms_event_report_ind_msg_v01));
    
	QMI_TEST_LOG1("\n\n MEMSET  DONE\n");
	QMI_TEST_LOG2("\n\n msg_id:%d\n", (int)msg_id);
	
	for(k = 0; k < ind_buf_len; k++)
	{
		QMI_TEST_LOG2("%d\t", ind_buf[k]);
	}
	QMI_TEST_LOG1("\n\n");
    rc = qmi_client_message_decode(user_handle,
									QMI_IDL_INDICATION,
									QMI_WMS_EVENT_REPORT_IND, 
									(void*)ind_buf,
									ind_buf_len,
									(void*)mt_sms,
									sizeof(wms_event_report_ind_msg_v01)
									);
	QMI_TEST_LOG1("\n\n AFTER QMI_CLIENT_MESSAGE_DECODE\n");	
	QMI_TEST_LOG2("\n\n rc:%d\n", rc);
									
	switch(msg_id)
	{
		case QMI_WMS_EVENT_REPORT_IND:		
			if(mt_sms->message_mode == 0x00)/* CDMA */
			{
				QMI_TEST_LOG1("\nMESSAGE MODE CDMA\n");
				QMI_TEST_LOG2("\nmt_message_valid:%d\n", mt_sms->mt_message_valid);
				QMI_TEST_LOG2("\nstorage_type:%d\n", mt_sms->mt_message.storage_type);
				QMI_TEST_LOG2("\nstorage_index:%d\n", mt_sms->mt_message.storage_index);
				QMI_TEST_LOG2("\ntransfer_route_mt_message_valid:%d\n", mt_sms->transfer_route_mt_message_valid);
				QMI_TEST_LOG2("\nmt_sms.transfer_route_mt_message.ack_indicator:%d\n", mt_sms->transfer_route_mt_message.ack_indicator);
				QMI_TEST_LOG2("\ntransaction_id: %d\n", mt_sms->transfer_route_mt_message.transaction_id);
				QMI_TEST_LOG2("\nformat:%d\n", mt_sms->transfer_route_mt_message.format);
				for (j = 0; j < mt_sms->transfer_route_mt_message.data_len; j++)
					{
						printf("%d\t", mt_sms->transfer_route_mt_message.data[j]);
					}
				QMI_TEST_LOG1("\n\n");	
				if(mt_sms->mt_message_valid == 1)
				{
					QMI_TEST_LOG1("\nMT_MESSAGE IS VALID\n");
					QMI_TEST_LOG2("storage_type:%d\n", mt_sms->mt_message.storage_type);
					QMI_TEST_LOG2("storage_index:%d\n", mt_sms->mt_message.storage_index);
				}
				else
				{
					QMI_TEST_LOG1("MT_MESSAGE IS INVALID\n");
				}
				if(mt_sms->transfer_route_mt_message_valid == 1)
				{
					QMI_TEST_LOG1("TRANSFER_ROUTE_MT_MESSAGE IS VALID\n");
					QMI_TEST_LOG2("ack_indicator: %d\n", mt_sms->transfer_route_mt_message.ack_indicator);
					QMI_TEST_LOG2("transaction_id: %d\n", mt_sms->transfer_route_mt_message.transaction_id);
					QMI_TEST_LOG2("format:%d\n", mt_sms->transfer_route_mt_message.format);
					for (i = 0; i < mt_sms->transfer_route_mt_message.data_len; i++)
					{
						printf("%d\t", mt_sms->transfer_route_mt_message.data[i]);
					}
					QMI_TEST_LOG1("\n\n");
			    }
				else
				{
					QMI_TEST_LOG1("TRANSFER_ROUTE_MT_MESSAGE IS INVALID\n");
				}
				if(mt_sms->message_mode_valid == 1)
				{
					QMI_TEST_LOG2("MESSAGE MODE VALID: %d\n", mt_sms->message_mode);
				}
				else
				{
					QMI_TEST_LOG1("MESSAGE MODE INVALID\n");
				}
			}
			else if (mt_sms->message_mode == 0x01) /* GW */
			{
				QMI_TEST_LOG1("MESSAGE MODE GW, NO PROCESS\n");
			}
			else
			{
				QMI_TEST_LOG1("UNKNOWN MESSAGE MODE\n");				
			}
			break;
		default:
			QMI_TEST_LOG2("NOT PROCESSED MSG_ID: %d\n", (int)msg_id);
			break;
	}
}	
