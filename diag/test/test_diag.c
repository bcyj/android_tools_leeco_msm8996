/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
Copyright (c) 2007-2014 by Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.


              Test Application for Diag Interface

GENERAL DESCRIPTION
  Contains main implementation of Diagnostic Services Test Application.

EXTERNALIZED FUNCTIONS
  None

INITIALIZATION AND SEQUENCING REQUIREMENTS
  

Copyright (c) 2007-2014 Qualcomm Technologies, Inc.
All Rights Reserved.
Qualcomm Technologies Confidential and Proprietary

*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/*===========================================================================

                        EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.
Notice that changes are listed in reverse chronological order.

$Header: 

when       who    what, where, why
--------   ---     ----------------------------------------------------------
10/01/08   SJ     Changes for CBSP2.0
03/26/08   JV     Added calls to test packet request/response
12/2/07    jv     Added test calls for log APIs
11/19/07   mad    Created      
===========================================================================*/
#include "event.h"
#include "msg.h"
#include "log.h"

#include "diag_lsm.h"
#include "stdio.h"
#include <unistd.h>
#include <pthread.h>
#include <limits.h>

#include "diagpkt.h"
#include "diagcmd.h"
#include "diagdiag.h"
#include "diag.h"

/* Global Data */
/* defined this temp struct just for testing logs */
typedef PACK(struct)
{
	log_hdr_type hdr;	/* Log header (length, code, timestamp) */
	uint32 log_payload[122];
} shim_log;

/* Subsystem command codes for the test app */
#define DIAG_TEST_APP_MT_NO_SUBSYS	143
#define DIAG_SUBSYS_TEST_CLIENT_MT	69
#define DIAG_TEST_APP_F_75		0x0000
#define DIAG_TEST_APP_F_75_test		0x0003
#define DIAG_TEST_APP_F_128		0x0001
#define DIAG_TEST_APP_F_128_DELAY	0x0002
#define DIAG_TEST_APP_F_75_test_1	0x0004
#define DIAG_TEST_APP_F_75_test_2	0x0005
#define DIAG_SUBSYS_TEST_CLIENT_1	67

/* static data */
pthread_t fd_testapp_thread_hdl;	/* Diag task thread handle */

/*=============================================================================*/
/* Local Function declarations */
/*=============================================================================*/

typedef boolean (*ptr_Diag_LSM_Init)(byte* pIEnv);
typedef boolean (*ptr_Diag_LSM_DeInit)(void);

static void *StartSecondApp(void* param);

PACK(void *) dummy_func_no_subsys(PACK(void *)req_pkt, uint16 pkt_len);

PACK(void *) dummy_func_75(PACK(void *)req_pkt, uint16 pkt_len);

PACK(void *) dummy_func_128(PACK(void *)req_pkt, uint16 pkt_len);

PACK(void *) dummy_func_128_delay(PACK(void *)req_pkt, uint16 pkt_len);

PACK(void *) dummy_func_75_test(PACK(void *)req_pkt, uint16 pkt_len);

/*=============================================================================*/
/* User tables for this client(test app) */
/*=============================================================================*/
static const diagpkt_user_table_entry_type test_tbl_1[] =
{
	{DIAG_TEST_APP_MT_NO_SUBSYS, DIAG_TEST_APP_MT_NO_SUBSYS, dummy_func_no_subsys},
};

static const diagpkt_user_table_entry_type test_tbl_2[] =
{
	{DIAG_TEST_APP_F_75, DIAG_TEST_APP_F_75, dummy_func_75},
	{DIAG_TEST_APP_F_75_test, DIAG_TEST_APP_F_75_test, dummy_func_75_test},
};

static const diagpkt_user_table_entry_type test_tbl_3[] =
{
	{DIAG_TEST_APP_F_128, DIAG_TEST_APP_F_128, dummy_func_128},
};

static const diagpkt_user_table_entry_type test_tbl_4[] =
{
	{DIAG_TEST_APP_F_128_DELAY, DIAG_TEST_APP_F_128_DELAY, dummy_func_128_delay},
};

/*=============================================================================*/

int main(void)
{
	//shim_log test_log;
	boolean bInit_Success = FALSE;
	// test_log.log_payload = 0x230987;
	printf(" calling LSM init \n");
	bInit_Success = Diag_LSM_Init(NULL);

	if(!bInit_Success)
	{
		printf("TestApp_MultiThread: Diag_LSM_Init() failed.");
		return -1;
	}
	printf("TestApp_MultiThread: Diag_LSM_Init succeeded. \n");
	pthread_create( &fd_testapp_thread_hdl, NULL, StartSecondApp, NULL );

	if (fd_testapp_thread_hdl == 0)
	{
		printf("TestApp_MultiThread: Failed to create second test app thread");
		return -1;
	}

	// send_data 143 0 0 0 0 0
	DIAGPKT_DISPATCH_TABLE_REGISTER (DIAGPKT_NO_SUBSYS_ID, test_tbl_1);

	// send_data 75 67 0 0 0 0 0
	DIAGPKT_DISPATCH_TABLE_REGISTER (DIAG_SUBSYS_TEST_CLIENT_1, test_tbl_2);

	// send_data 75 69 0 0 0 0 0 0
	DIAGPKT_DISPATCH_TABLE_REGISTER (DIAG_SUBSYS_TEST_CLIENT_MT, test_tbl_2);

	// send_data 75 66 0 0 0 0 0
	DIAGPKT_DISPATCH_TABLE_REGISTER (DIAG_SUBSYS_ANALOG, test_tbl_2);

	// send_data 128 67 0 0 0 0 0 0
	DIAGPKT_DISPATCH_TABLE_REGISTER_V2 (DIAG_SUBSYS_CMD_VER_2_F, DIAG_SUBSYS_TEST_CLIENT_1, test_tbl_2);

	 // send_data 128 67 1 0 0 0 0 0
	DIAGPKT_DISPATCH_TABLE_REGISTER_V2 (DIAG_SUBSYS_CMD_VER_2_F,DIAG_SUBSYS_TEST_CLIENT_1, test_tbl_3);

	//send_data 128 69 2 0 0 0 0 0 0
	DIAGPKT_DISPATCH_TABLE_REGISTER_V2_DELAY(DIAG_SUBSYS_CMD_VER_2_F, DIAG_SUBSYS_TEST_CLIENT_MT, test_tbl_4);

	// send_data 128 67 2 0 0 0 0
	DIAGPKT_DISPATCH_TABLE_REGISTER_V2_DELAY(DIAG_SUBSYS_CMD_VER_2_F, DIAG_SUBSYS_TEST_CLIENT_1, test_tbl_4);

	// char event_payload[40] = "TestApp_MultiThread Event with Payload1";
	// shim_log* pLog_Test = NULL;

	/* Sleep for 5 seconds to give second thread a chance to show its stuff */
	sleep(5);

	do
	{
		MSG_1(MSG_SSID_DIAG,MSG_LVL_HIGH,"HELLO HIGH1 %d",209387);

		MSG_MED("This is a msg_med from ARM11", 1,2,3);
		MSG_HIGH("HELLO WORLDHIGH HELLO WORLDHIGH HELLO WORLDHIGH HELLO WORLDHIGH HELLO WORLDHIGH HELLO WORLDHIGH HELLO WORLDHIGH HELLO WORLDHIGH HELLO WORLDHIGH HELLO WORLDHIGH HELLO WORLDHIGH HELLO WORLDHIGH HELLO WORLDHIGH HELLO WORLDHIGH HELLO WORLDHIGH HELLO WORLDHIGH HELLO WORLDHIGH HELLO WORLDHIGH HELLO WORLDHIGH HELLO WORLDHIGH HELLO WORLDHIGH HELLO WORLDHIGH HELLO WORLDHIGH HELLO WORLDHIGH HELLO WORLDHIGH HELLO WORLDHIGH HELLO %d %d %d",1,2,3);
		MSG_2(MSG_SSID_DIAG,MSG_LVL_HIGH,"HELLO HIGH2 %d %d",209387, 4957);
		MSG_5(MSG_SSID_DIAG,MSG_LVL_HIGH,"HELLO HIGH5 %d %d %d %d %d",1,6,1000,34,209387);
		MSG(MSG_SSID_DIAG, MSG_LEGACY_HIGH, "Test MSG with no arg");

		MSG_TS(MSG_SSID_DIAG, MSG_LEGACY_HIGH, "TEST MSG WITH TIMESTAMP", 473040000000ULL);

		event_report(EVENT_CAMERA_INVALID_STATE);
		event_report(EVENT_DIAG_STRESS_TEST_NO_PAYLOAD);

		char event_payload[40] = "TestApp_MultiThread Event with Payload1";
		event_report_payload(EVENT_DIAG_STRESS_TEST_WITH_PAYLOAD,sizeof(event_payload), event_payload);

		char event_payload2[1] = "T";
		event_report_payload(EVENT_DIAG_STRESS_TEST_WITH_PAYLOAD,sizeof(event_payload2), event_payload2);

		usleep(1000);

		char event_payload3[2] = "Te";
		event_report_payload(EVENT_DIAG_STRESS_TEST_WITH_PAYLOAD,sizeof(event_payload3), event_payload3);

		usleep(1000);

		char event_payload4[3] = "Tes";
		event_report_payload(EVENT_DIAG_STRESS_TEST_WITH_PAYLOAD,sizeof(event_payload4), event_payload4);

		/* Important Note: This is just to test QSR_MSG* macros. These macros should
		 * not be called directly, by diag client modules. MSG* macros are replaced by
		 * QSR_MSG* macros, by text-replacement before build. */
		QSR_MSG(1100110010, MSG_SSID_DIAG, MSG_LEGACY_HIGH, "QSR Message string from TestApp");
		QSR_MSG_1(1100110011, MSG_SSID_DIAG, MSG_LEGACY_HIGH, "QSR Message string from TestApp", 220022);
		QSR_MSG_2(1100110012, MSG_SSID_DIAG, MSG_LEGACY_HIGH, "QSR Message string from TestApp", 220022, 330033);
		QSR_MSG_3(1100110013, MSG_SSID_DIAG, MSG_LEGACY_HIGH, "QSR Message string from TestApp", 220022, 330033, 440044);
		QSR_MSG_4(1100110014, MSG_SSID_DIAG, MSG_LEGACY_HIGH, "QSR Message string from TestApp", 220022, 330033, 440044, 550055);
		QSR_MSG_9(1100110019, MSG_SSID_DIAG, MSG_LEGACY_HIGH, "QSR Message string from TestApp", 110011, 220022, 330033, 440044, 550055, 660066, 770077, 880088, 990099);

		/*
		log_set_code(&test_log, LOG_DIAG_STRESS_TEST_C);
		log_set_length(&test_log, sizeof(shim_log));

		log_set_timestamp(&test_log);
		log_submit(&test_log);

		pLog_Test = (shim_log*)log_alloc(LOG_WMS_READ_C,sizeof(shim_log));
		log_set_timestamp(pLog_Test);
		log_commit(pLog_Test);
		*/

	} while(1);

/* Now find the DeInit function and call it. */
	// Clean up before exiting
	Diag_LSM_DeInit();

	return 0;
}

/*=============================================================================*/
/* dummy registered functions */
/*=============================================================================*/

PACK(void *) dummy_func_no_subsys(PACK(void *)req_pkt, uint16 pkt_len)
{
	PACK(void *)rsp = NULL;
	printf("\n ####### TestApp_MultiThread: Inside dummy_func_no_subsys #######\n");

	// Allocate the same length as the request.
	rsp = diagpkt_alloc (DIAG_TEST_APP_MT_NO_SUBSYS, pkt_len);

	if (rsp != NULL) {
		memcpy ((void *) rsp, (void *) req_pkt, pkt_len);
		printf("TestApp_MultiThread: diagpkt_alloc succeeded");
	}

	return (rsp);
}

/*=============================================================================*/

PACK(void *) dummy_func_75(PACK(void *)req_pkt, uint16 pkt_len)
{
	PACK(void *)rsp = NULL;
	printf("\n TestApp_MultiThread: Inside dummy_func_75");

	// Allocate the same length as the request.
	rsp = diagpkt_subsys_alloc (DIAG_TEST_APP_MT_NO_SUBSYS,DIAG_TEST_APP_F_75, pkt_len);

	if (rsp != NULL) {
		memcpy ((void *) rsp, (void *) req_pkt, pkt_len);
		printf("TestApp_MultiThread: diagpkt_subsys_alloc succeeded");
	} else {
		printf("TestApp_MultiThread: diagpkt_subsys_alloc failed");
	}

	return (rsp);
}

/*=============================================================================*/

PACK(void *) dummy_func_128(PACK(void *)req_pkt, uint16 pkt_len)
{
	PACK(void *)rsp = NULL;
	printf("TestApp_MultiThread: Inside dummy_func_128");

	// Allocate the same length as the request.
	rsp = diagpkt_subsys_alloc_v2 (DIAG_TEST_APP_MT_NO_SUBSYS,DIAG_TEST_APP_F_128, pkt_len);

	if (rsp != NULL) {
		memcpy ((void *) rsp, (void *) req_pkt, pkt_len);
		printf("TestApp_MultiThread: diagpkt_subsys_alloc_v2 succeeded");
	}

	return (rsp);
}

/*=============================================================================*/

typedef PACK(struct)
{
	uint8 command_code;
	uint8 subsys_id;
	uint16 subsys_cmd_code;
	uint32 status;
	uint16 delayed_rsp_id;
	uint16 rsp_cnt;		/* 0, means one response and 1, means two responses */
} diagtestapp_subsys_hdr_v2_type;

typedef PACK(struct)
{
	diagtestapp_subsys_hdr_v2_type hdr;
	char my_rsp[20];
}diagtestapp_delayed_rsp_type;

PACK(void *) dummy_func_128_delay(PACK(void *) req_pkt, uint16 pkt_len)
{
	PACK(void *) rsp = NULL;
	diagtestapp_delayed_rsp_type *p_my_rsp = NULL;
	diagpkt_subsys_delayed_rsp_id_type delay_rsp_id = 0;
	unsigned char *temp = (unsigned char *)req_pkt + 1;
	int rsp_ssid = (int)(*(char *)temp);

	printf("\n DiagSvcTestApp1: Inside dummy_func_128_delay \n");

	/* Allocate the length of response. */
	rsp = diagpkt_subsys_alloc_v2(rsp_ssid, DIAG_TEST_APP_F_128_DELAY, sizeof(diagtestapp_delayed_rsp_type));

	/* Get the delayed_rsp_id that was allocated by diag to
	 * use for the delayed response we're going to send next.
	 * This id is unique in the system.
	 */
	delay_rsp_id = diagpkt_subsys_get_delayed_rsp_id(rsp);

	/* We cannot just do a memcpy of the request packet to the response
	 * packets for loop-back, because the size/content of the response
	 * packet is different from the request packet */
	if (rsp != NULL) {
		char first_resp[]="First response.";
		printf(" \n DiagSvcTestApp1: diagpkt_subsys_alloc succeeded \n");

		p_my_rsp = (diagtestapp_delayed_rsp_type*)rsp;
		/* Fill in the dummy immediate response. */
		memcpy(p_my_rsp->my_rsp, first_resp, strlen(first_resp));
	}

	diagpkt_commit(rsp);

	/* Now work on the dummy delayed response */
	rsp = diagpkt_subsys_alloc_v2_delay(rsp_ssid, DIAG_TEST_APP_F_128_DELAY,
				delay_rsp_id, sizeof(diagtestapp_delayed_rsp_type));

	if (rsp != NULL) {
		/* We cannot just do a memcpy here. */
		char sec_resp[] = "Delayed response.";
		printf("\n DiagSvcTestApp1: diagpkt_subsys_alloc_v2_delay succeeded \n");

		/* ICD says this means this is the second/final response.
		 * Note that multiple delayed response are supported.
		 * Please see the ICD */
		diagpkt_subsys_set_rsp_cnt(rsp,1);
		p_my_rsp = (diagtestapp_delayed_rsp_type*) rsp;
		/* Fill in my dummy delayed response */
		memcpy(p_my_rsp->my_rsp,sec_resp,strlen(sec_resp));
	}

	diagpkt_delay_commit(rsp);

	return NULL;
}

/*=============================================================================*/

PACK(void *) dummy_func_75_test (PACK(void *)req_pkt, uint16 pkt_len)
{
	PACK(void *)rsp = NULL;
	printf("TestApp_MultiThread: Inside dummy_func_75_test**");

	// Allocate the same length as the request.
	rsp = diagpkt_subsys_alloc(DIAG_TEST_APP_MT_NO_SUBSYS,DIAG_TEST_APP_F_75_test, pkt_len);

	if (rsp != NULL) {
		memcpy ((void *)rsp, (void *)req_pkt, pkt_len);
		printf("TestApp_MultiThread: diagpkt_subsys_alloc succeeded");
	}
	return (rsp);
}

/*=============================================================================*/
//static DWORD StartSecondApp(LPVOID param)

static void *StartSecondApp(void* param)
{
	shim_log test_log;
	shim_log* pLog_Test1 = NULL;
	shim_log* pLog_Test2 = NULL;
	shim_log* pLog_Test3 = NULL;
	boolean result1 = FALSE;
	boolean result2 = FALSE;
	unsigned int count;

	for (count = 0; count < UINT_MAX; count++)
	{
		MSG_HIGH("second thread %d %d %d",1,2,3);

		MSG_SPRINTF_1(MSG_SSID_DIAG, MSG_LEGACY_ERROR,"HELLO SPRINTF_1 ERROR %d",1);
		MSG_SPRINTF_1(MSG_SSID_DIAG, MSG_LEGACY_HIGH,"HELLO SPRINTF_1 ERROR %d",1);
		sleep(1);

		MSG(MSG_SSID_DIAG, MSG_LEGACY_HIGH, "Test MSG with no arg");

		log_set_code(&test_log,LOG_DIAG_STRESS_TEST_C);
		log_set_length(&test_log, sizeof(shim_log));
		log_set_timestamp(&test_log);
		log_submit(&test_log);

		result1 = log_status(LOG_WMS_SET_ROUTES_C);
		result2 = log_status(LOG_DIAG_STRESS_TEST_C);

		pLog_Test1 = (shim_log*)log_alloc(LOG_WMS_SET_ROUTES_C ,sizeof(shim_log));
		pLog_Test2 = (shim_log*)log_alloc(LOG_DATA_PROTOCOL_LOGGING_C ,sizeof(shim_log));
		pLog_Test3 = (shim_log*)log_alloc(LOG_WMS_READ_C,sizeof(shim_log));

		log_commit(pLog_Test1);
		log_commit(pLog_Test2);
		log_commit(pLog_Test3);

		sleep(.001);
	}

	return 0;
}
