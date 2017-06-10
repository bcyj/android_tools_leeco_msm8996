/********************************************************************
---------------------------------------------------------------------
 Copyright (c) 2012 Qualcomm Technologies, Inc.
 All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
----------------------------------------------------------------------
QSEECom Sample/Test Client app.
*********************************************************************/
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <linux/msm_ion.h>
#include <utils/Log.h>
#include "QSEEComAPI.h"
#include "common_log.h"
#include <sys/mman.h>
#include <getopt.h>


/** adb log */
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "QSEECOM_SAMPLE_CLIENT: "
#ifdef LOG_NDDEBUG
#undef LOG_NDDEBUG
#endif
#define LOG_NDDEBUG 0 //Define to enable LOGD
#ifdef LOG_NDEBUG
#undef LOG_NDEBUG
#endif
#define LOG_NDEBUG  0 //Define to enable LOGV
#define NON_LEGACY_CMD_LINE_ARGS 1


#define RUN_TESTS 0
#define ION_BUFFER_LENGTH (64*1024)


/* commands supported from sample client */
#define CLIENT_CMD0_GET_VERSION             0
#define CLIENT_CMD1_BASIC_DATA              1
#define CLIENT_CMD2_REGISTER_SB_PTR         2
#define CLIENT_CMD3_RUN_CRYTPO_TEST         3
#define CLIENT_CMD4_RUN_CRYTPO_PERF         4
#define CLIENT_CMD5_RUN_SFS_TEST            5
#define CLIENT_CMD6_RUN_BUF_ALIGNMENT_TEST  6
#define CLIENT_CMD7_RUN_CRYPTO_RSA_TEST     7
#define CLIENT_CMD8_RUN_RSA_PERF_TEST       8
#define CLIENT_CMD9_RUN_CMNLIB_TEST         9
#define CLIENT_CMD10_RUN_CORE_TEST          10
#define CLIENT_CMD11_RUN_SECURECHANNEL_TEST 11
#define CLIENT_CMD12_RUN_SERVICES_TEST      12
#define CLIENT_CMD13_RUN_MISC_TEST          13
#define CLIENT_CMD17_RUN_STORAGE_TEST       17

#define __64KB__ 0x10000
#define __32KB__ 0x8000
#define __16KB__ 0x4000
#define __8KB__ 0x2000
#define __4KB__ 0x1000
#define __2KB__ 0x800
#define __1KB__ 0x400

/* Error code: status sent as response to command from sample client*/

#define SUCCESS 0
#define FAILED -1

#define FAIL_REGISTER_SB    -2
#define FAIL_DEREGISTER_SB  -3
#define FAIL_PREP_SB_NON_SECURE_READ -4
#define FAIL_PREP_SB_SECURE_READ     -5
#define FAIL_UNSUPPORTED_COMMAND     -6

static uint32_t test_iterations  = 1000;

#define NUM_CLIENT_THREADS 2

#define MAX_APP_NAME 25

static char legacy_app_name[MAX_APP_NAME] = "sampleapp";
static char *appname;


#define SAMPLE_CLIENT_VERSION 0x1
/* Name of File Services */
static const char *FILESYSTEM_LIB = "libdrmfs.so";
static void *g_FSHandle            = NULL;

struct qsc_send_cmd {
     uint32_t cmd_id;
     uint32_t data;
     uint32_t data2;
     uint32_t len;
     uint32_t start_pkt;
     uint32_t end_pkt;
     uint32_t test_buf_size;
};

struct qsc_ion_info {
	int32_t ion_fd;
	int32_t ifd_data_fd;
	struct ion_handle_data ion_alloc_handle;
	unsigned char * ion_sbuffer;
	uint32_t sbuf_len;
};

struct qsc_send_cmd_rsp {
  uint32_t data;
  int32_t status;
};

enum test_types {
	BASIC,
	REGMEM,
	CRYPTO,
	PERFORMANCE,
	STRESS,
	RSA,
        LAST_TEST,
	INTERNAL= LAST_TEST
};

struct option qsc_testopts[] = {
	{"Basic", no_argument, NULL, 'b'},
	{"Regmem", no_argument, NULL, 'r'},
	{"Crypto", no_argument, NULL, 'c'},
	{"performance", no_argument, NULL, 'p'},
	{"stress", no_argument, NULL, 's'},
	{"rsa", no_argument, NULL, 'r'},
	{"Help", no_argument, NULL, 'h'},
	{NULL, 0, NULL, 0},
};

FILE *fp;

static int verbose;

pthread_mutex_t plock;

static int32_t qsc_ION_memalloc(struct qsc_ion_info *handle,
                                uint32_t size)
{
        int32_t ret = 0;
        int32_t iret = 0;
        int32_t fd = 0;
        unsigned char *v_addr;
        struct ion_allocation_data ion_alloc_data;
        int32_t ion_fd;
        int32_t rc;
        struct ion_fd_data ifd_data;
        struct ion_handle_data handle_data;

        /* open ION device for memory management
         * O_DSYNC -> uncached memory
        */

        if(handle == NULL){
	  LOGE("Error:: null handle received");
	  printf("%s: Error:: null handle received\n", __func__);
	  return -1;
	}
        ion_fd  = open("/dev/ion", O_RDONLY);
        if (ion_fd < 0) {
           LOGE("Error::Cannot open ION device\n");
           printf("%s: Error:: Cannot open ION device\n", __func__);
           return -1;
        }
        handle->ion_sbuffer = NULL;
	handle->ifd_data_fd = 0;

        /* Size of allocation */
        ion_alloc_data.len = (size + 4095) & (~4095);

        /* 4K aligned */
        ion_alloc_data.align = 4096;

        /* memory is allocated from EBI heap */
        ion_alloc_data.heap_id_mask= ION_HEAP(ION_QSECOM_HEAP_ID);

	/* Set the memory to be uncached */
        ion_alloc_data.flags = 0;

        /* IOCTL call to ION for memory request */
        rc = ioctl(ion_fd, ION_IOC_ALLOC, &ion_alloc_data);
        if (rc) {
           LOGE("Error::Error while trying to allocate data\n");
           printf("%s: Error while trying to allocate data\n", __func__);
           goto alloc_fail;
        }

        if (ion_alloc_data.handle) {
           ifd_data.handle = ion_alloc_data.handle;
        } else {
           LOGE("Error::ION alloc data returned a NULL\n");
           printf("%s: Error:: ION alloc data returned a NULL\n", __func__);
           goto alloc_fail;
        }
       /* Call MAP ioctl to retrieve the ifd_data.fd file descriptor */
        rc = ioctl(ion_fd, ION_IOC_MAP, &ifd_data);
        if (rc) {
           LOGE("Error::Failed doing ION_IOC_MAP call\n");
           printf("%s: Error:: Failed doing ION_IOC_MAP call\n",__func__);
           goto ioctl_fail;
        }

        /* Make the ion mmap call */
        v_addr = (unsigned char *)mmap(NULL, ion_alloc_data.len,
                                        PROT_READ | PROT_WRITE,
                                        MAP_SHARED, ifd_data.fd, 0);
        if (v_addr == MAP_FAILED) {
           LOGE("Error::ION MMAP failed\n");
	   printf("%s: Error:: ION MMAP failed\n",__func__);
           ret = -1;
           goto map_fail;
        }
        handle->ion_fd = ion_fd;
        handle->ifd_data_fd = ifd_data.fd;
        handle->ion_sbuffer = v_addr;
        handle->ion_alloc_handle.handle = ion_alloc_data.handle;
        handle->sbuf_len = size;
        return ret;

map_fail:
	if(handle->ion_sbuffer != NULL)
	{
          ret = munmap(handle->ion_sbuffer, ion_alloc_data.len);
          if (ret)
            LOGE("Error::Failed to unmap memory for load image. ret = %d\n",
                  ret);
            printf("%s:Error:Failed to unmap memory for load image.ret = %d\n",
				  __func__,ret);
	}

ioctl_fail:
        handle_data.handle = ion_alloc_data.handle;
        if (handle->ifd_data_fd)
           close(handle->ifd_data_fd);
        iret = ioctl(ion_fd, ION_IOC_FREE, &handle_data);
        if (iret){
           LOGE("Error::ION FREE ioctl returned error = %d\n",iret);
	   printf("%s: Error::ION FREE ioctl returned error = %d\n",
		   	      __func__,iret);
	}

alloc_fail:
        if (ion_fd)
          close(ion_fd);
        return ret;

}

/** @brief: Deallocate ION memory
 *
 *
 */
static int32_t qsc_ion_dealloc(struct qsc_ion_info *handle)
{
        struct ion_handle_data handle_data;
        int32_t ret = 0;

        /* Deallocate the memory for the listener */
        ret = munmap(handle->ion_sbuffer,
                        (handle->sbuf_len + 4095) & (~4095));
        if (ret){
          LOGE("Error::Unmapping ION Buffer failed with ret = %d\n", 
               ret);
	  printf("%s: Error:: Unmapping ION Buffer failed with ret = %d\n",
		  	     __func__,ret);
	}

        handle_data.handle = handle->ion_alloc_handle.handle;
        close(handle->ifd_data_fd);
        ret = ioctl(handle->ion_fd, ION_IOC_FREE, &handle_data);
        if (ret){
          LOGE("Error::ION Memory FREE ioctl failed with ret = %d\n", 
                     ret);
          printf("%s: Error:: ION Memory FREE ioctl failed with ret = %d\n",__func__,ret);
	}
        close(handle->ion_fd);
	return ret;

}

/**@brief:  Implement simple application start
 *
 * @param[in/out]	handle.
 * @param[in]		appname.
 * @param[in]		buffer size.
 * @return	zero on success or error count on failure.
 */
int32_t qsc_start_app(struct QSEECom_handle **l_QSEEComHandle, 
                        const char *appname, int32_t buf_size)
{
	int32_t ret = 0;

	/* start the application */
	ret = QSEECom_start_app(l_QSEEComHandle, "/firmware/image",
					appname, buf_size);
	if (ret)
		ret = QSEECom_start_app(l_QSEEComHandle, "/system/etc/firmware",
					appname, buf_size);
	if (ret) {
	   LOGE("Loading app -%s failed",appname);
	   printf("%s: Loading app -%s failed\n",__func__,appname);
	} else {
	   LOGD("Loading app -%s succeded",appname);
	}

	return ret;
}

/**@brief:  Implement simple shutdown app
 * @param[in]	handle.
 * @return	zero on success or error count on failure.
 */
int32_t qsc_shutdown_app(struct QSEECom_handle **l_QSEEComHandle)
{
	int32_t ret = 0;

	LOGD("qsc_shutdown_app: start");
	/* shutdown the application */
	if (*l_QSEEComHandle != NULL) {
	   ret = QSEECom_shutdown_app(l_QSEEComHandle);
	   if (ret) {
	      LOGE("Shutdown app failed with ret = %d", ret);
	      printf("%s: Shutdown app failed with ret = %d\n",__func__,ret);
	   } else {
	      LOGD("shutdown app: pass");
	   }
	} else {
		LOGE("cannot shutdown as the handle is NULL");
		printf("%s:cannot shutdown as the handle is NULL\n",__func__);
	}
	return ret;
}

/**@brief:  Exercise send command
 * @param[in]	handle.
 * @param[in]	data to be send to secure app.
 * @return	zero on success or error count on failure.
 */	
int32_t qsc_issue_send_cmd_req(struct QSEECom_handle *l_QSEEComHandle,
                                         struct qsc_send_cmd *send_cmd)
{
	int32_t ret = 0;
	int32_t req_len = 0;
	int32_t rsp_len = 0;
	struct qsc_send_cmd *msgreq;	/* request data sent to QSEE */
	struct qsc_send_cmd_rsp *msgrsp;	/* response data sent from QSEE */

	LOGD("send modified cmd: start");
	/* populate the data in shared buffer */
	msgreq=(struct qsc_send_cmd *)l_QSEEComHandle->ion_sbuffer;
	msgreq->cmd_id = send_cmd->cmd_id;
	msgreq->data = 100;
	req_len = sizeof(struct qsc_send_cmd);
	rsp_len = sizeof(struct qsc_send_cmd_rsp);

	if (req_len & QSEECOM_ALIGN_MASK)
		req_len = QSEECOM_ALIGN(req_len);

	if (rsp_len & QSEECOM_ALIGN_MASK)
		rsp_len = QSEECOM_ALIGN(rsp_len);

	msgrsp=(struct qsc_send_cmd_rsp *)l_QSEEComHandle->ion_sbuffer;

	LOGD("req data = %d",msgreq->data);
	LOGD("req len = %d bytes",req_len);
	LOGD("rsp len = %d bytes",rsp_len);
	/* send request from HLOS to QSEApp */
	ret = QSEECom_send_cmd(l_QSEEComHandle,
				msgreq,
				req_len,
				msgrsp,
				rsp_len);
	if (ret) {
	   LOGE("send command failed with ret = %d\n", ret);
	   printf("%s: Send command failed with ret = %d\n",__func__,ret);
	}
	LOGD("rsp data = %d", msgrsp->data);
	if(msgreq->cmd_id == CLIENT_CMD1_BASIC_DATA)
	{
		/* The response data should be req_data * 10 */
		if ( ret || (((msgrsp->data)/100) != 10) ) {
			LOGE("FAILED Response data = %x", msgrsp->data);
			printf("%s:FAILED Response data = %x\n",__func__, msgrsp->data);
		} else {
			LOGD("send command: PASS");
		}
	}
	return ret;

}

/**@brief:  Exercise send modified command
 * @param[in]	handle.
 * @param[in]	crypto_test flag.
 * @return	zero on success or error count on failure.
 */	
int32_t qsc_issue_send_modified_cmd_req(struct QSEECom_handle *l_QSEEComHandle,
                                               struct qsc_send_cmd *send_cmd)
{
	int32_t ret = 0;
	int32_t err = 0;
	int32_t size = 0;
	int32_t j = 0;
	int32_t req_len = 0;
	int32_t rsp_len = 0;
	struct QSEECom_ion_fd_info  ion_fd_info;
	struct qsc_send_cmd *msg_req;
	struct qsc_send_cmd_rsp* msg_rsp;
	struct qsc_ion_info ihandle;
	char * verify;

	LOGD("send modified cmd: start");
	/* allocate 64KB memory */

	ihandle.ion_fd = 0;
	ihandle.ion_alloc_handle.handle = 0;
	ret = qsc_ION_memalloc(&ihandle, ION_BUFFER_LENGTH);
	if (ret) {
		LOGD("Error allocating memory in ion\n");
		return -1;
	}
	memset(&ion_fd_info, 0, sizeof(struct QSEECom_ion_fd_info));

	/* Populate the send data structure */
	msg_req=(struct qsc_send_cmd *)l_QSEEComHandle->ion_sbuffer;
	msg_req->cmd_id = send_cmd->cmd_id;
	msg_req->len = ION_BUFFER_LENGTH;
	msg_req->start_pkt = send_cmd->start_pkt;
	msg_req->end_pkt = send_cmd->end_pkt;
	msg_req->test_buf_size = send_cmd->test_buf_size;

	ion_fd_info.data[0].fd = ihandle.ifd_data_fd;
	ion_fd_info.data[0].cmd_buf_offset = sizeof(uint32_t) ;
	ion_fd_info.data[1].fd = ihandle.ifd_data_fd;
	ion_fd_info.data[1].cmd_buf_offset = sizeof(uint32_t) * 2 ;

        verify = malloc(ION_BUFFER_LENGTH);
        if(verify == NULL) {
                LOGE("Malloc failed for SEND_MODIFIED_CMD test, exiting\n");
                printf("Malloc failed for Send Modified CMD test, exiting\n");
				ret = qsc_ion_dealloc(&ihandle);
				if(ret) {
				  err++;
				  LOGE("return value of dealloc is %d",ret);
				  printf("return value of dealloc is %d\n",ret);
				}
                return -1;
        }

	for (j = 0; j < ION_BUFFER_LENGTH; j++){
	   *(ihandle.ion_sbuffer+j) = (char)(j%255);
	   *(verify+j) = (char)(j%255);
	}

	req_len = sizeof(struct qsc_send_cmd);
	rsp_len = sizeof(struct qsc_send_cmd_rsp);

	if (req_len & QSEECOM_ALIGN_MASK)
    		req_len = QSEECOM_ALIGN(req_len);

	if (rsp_len & QSEECOM_ALIGN_MASK)
         	rsp_len = QSEECOM_ALIGN(rsp_len);

	msg_rsp = (struct qsc_send_cmd_rsp *)l_QSEEComHandle->ion_sbuffer + req_len;
	msg_rsp->status = 0;
	/* send request from HLOS to QSEApp */
        ret = QSEECom_send_modified_cmd(l_QSEEComHandle, msg_req,
					req_len, msg_rsp,
					rsp_len, &ion_fd_info);
	if (send_cmd->cmd_id != CLIENT_CMD2_REGISTER_SB_PTR) {
          if (memcmp(verify, ihandle.ion_sbuffer, ION_BUFFER_LENGTH)) {
	    err++;
	    LOGD("qsc_issue_send_modified_cmd_req: failed verification");
	    printf("qsc_issue_send_modified_cmd_req: FAILED verification\n");
          }
	}
	if(ret) {
	   err++;
	   LOGD("qsc_issue_send_modified_cmd_req: fail cmd = %d ret = %d", 
		   send_cmd->cmd_id, ret);
	   printf("qsc_issue_send_modified_cmd_req: fail cmd = %d ret = %d\n", 
		    send_cmd->cmd_id, ret);
	}

	/* No Crypto test option. Verify the first 16 bytes of response data.
	   It should be req_data + 10
	*/
	if(send_cmd->cmd_id == CLIENT_CMD2_REGISTER_SB_PTR) {
	   for (j = 0; j <16; j++) {
	      *(ihandle.ion_sbuffer+j) = *(ihandle.ion_sbuffer+j)-10;
	      if(*(ihandle.ion_sbuffer + j) != (char)(j%255)) {
	        LOGE("Modified buffer check @ %d = %x", j, *(ihandle.ion_sbuffer +j));
		printf("%s: Modified buffer check @ %d = %x\n",
		     __func__,j, *(ihandle.ion_sbuffer +j));
			err++;
		break;
		}
	   }
	}
	if(msg_rsp->status < 0) {
	   err++;
	   LOGE("qsc_issue_send_modified_cmd_req:: failed msg_rsp->status =%d", 
		   msg_rsp->status);
	   printf("qsc_issue_send_modified_cmd_req:: failed msg_rsp->status =%d\n", 
		    msg_rsp->status);
	}

	/* De-allocate 64KB memory */
	ret = qsc_ion_dealloc(&ihandle);
	if(ret) {
	  err++;
	  LOGE("return value of dealloc is %d",ret);
	  printf("qsc_issue_send_modified_cmd_req::fail dealloc is %d\n",ret);
	}
	if (verify)
		free(verify);
	return err;

}


int32_t qsc_run_crypto_test(uint32_t cmd, int dummy_thread)
{
	uint32_t i = 0;
	int32_t ret = 0;			/* return value */
	int32_t err = 0;			/* return value */
	struct qsc_send_cmd send_cmd;
	uint32_t pkt_size = __32KB__;
	struct QSEECom_handle *l_QSEEComHandle = NULL;

	printf("Starting qsc_run_crypto_test: cmd = %d, thread = %d\n",
		cmd, dummy_thread + 1);
        printf("This may take a few minutes please wait (running 10000 cycles)\n");
	LOGD("Starting qsc_run_crypto_test");
        ret = qsc_start_app(&l_QSEEComHandle,appname, 1024);
	if (ret) {
	   LOGE("Start app: fail");
	   printf("Start app: fail\n");
	   return -1;
        } else {
	    LOGD("Start app: pass");
	    printf("Start app: pass\n");
        }
	if(l_QSEEComHandle == NULL) {
		LOGE("Failed to get QSEECOM handle\n");
		printf("Failed to get QSEECOM handle\n");
		return -1;
	}
	send_cmd.cmd_id = cmd;
	ret = QSEECom_set_bandwidth(l_QSEEComHandle, true);
	for(i = 0; i < test_iterations; i++) {
	   if (pkt_size == __64KB__)
	     pkt_size = __1KB__;
	   send_cmd.start_pkt = pkt_size;
	   send_cmd.end_pkt = pkt_size;
	   send_cmd.test_buf_size = pkt_size;
	   /* Send data using send modified command to QSEE application. crypto test. */
	   ret = qsc_issue_send_modified_cmd_req(l_QSEEComHandle, &send_cmd);
	   if (ret) {
             err++;
	     LOGE("iteration::%d::send modified cmd with crypto test:fail",i);
	     printf("iteration::%d::send modified cmd with crypto test:fail\n",i);
	     break;
	   }
	   usleep(30000);
	   pkt_size = pkt_size * 2;
	}
	printf("qsc_run_crypto_test ran %d iterations\n", i);
	
	if (!ret)
	   printf("   qsc_run_crypto_test PASSED\n");
	else
	   printf("   qsc_run_crypto_test FAILED\n");
	ret = QSEECom_set_bandwidth(l_QSEEComHandle, false);
	/* Shutdown the application */
	ret = qsc_shutdown_app(&l_QSEEComHandle);
	if (ret) {
	   err++;
	   LOGE("   Failed to shutdown app: %d",ret);
	   printf("   Failed to shutdown app: %d",ret);
	}
	return err;
}

/**@brief:  Run CRYPTO_PERFORMANCE_TEST for 'test_iterations' times
 * @param[in]	handle.
 * @return	zero on success or error count on failure.
 */
int32_t qsc_run_crypto_performance_test(uint32_t dummy_cmd, int dummy_thread)
{
	uint32_t i = 0;
	int32_t ret = 0;			/* return value */
	int32_t err = 0;			/* return value */
	struct qsc_send_cmd send_cmd;
	struct QSEECom_handle *l_QSEEComHandle = NULL;

	LOGD("Starting qsc_run_crypto_performance_test");
	printf("Starting qsc_run_crypto_performance_test: cmd = %d, thread = %d\n",
		dummy_cmd, dummy_thread + 1);
        printf("(This may take a few minutes please wait....)\n");



        ret = qsc_start_app(&l_QSEEComHandle,appname, 1024);
	if (ret) {
	    LOGE("   Start app: fail");
	    printf("   Start app: fail\n");
	    return -1;
        } else {
	    LOGD("   Start app: pass");
	    printf("   Start app: pass\n");
        }

	ret = QSEECom_set_bandwidth(l_QSEEComHandle, true);
	if ( ret != 0 )
	{
	   LOGE("Error: set QSEECom_set_bandwidth=true returned %d, shutting down the app", ret);
	   printf("%s: Error: set QSEECom_set_bandwidth=true returned %d, shutting down the app\n",__func__,ret);
	   ret = qsc_shutdown_app(&l_QSEEComHandle);
       if (ret) {
	     err++;
	     LOGE("Failed to shutdown app: %d",ret);
	     printf("Failed to shutdown app: %d",ret);
       }
	   return -1;
	}

	send_cmd.cmd_id = CLIENT_CMD4_RUN_CRYTPO_PERF;
	for(i = 0; i < test_iterations; i++)
	{
	   send_cmd.start_pkt = __1KB__;
	   send_cmd.end_pkt = __64KB__;
	   send_cmd.test_buf_size = __64KB__;
	   /* Send data using send modified command to QSEE application. crypto test. */
	   ret = qsc_issue_send_modified_cmd_req(l_QSEEComHandle, &send_cmd);
	   if(ret) {
	     LOGE("iteration::%d:send modified cmd with crypto perf test:fail",i);
	     printf("%s:iteration:%d:send modified cmd for crypto perf fail\n",__func__,i);
	     break;
	   }
	   usleep(30000);
	}
	if (!ret)
           printf("qsc_run_crypto_performance_test PASSED\n");
	else
	   printf("qsc_run_crypto_performance_test FAILED\n");

        ret = QSEECom_set_bandwidth(l_QSEEComHandle, false);
	if ( ret != 0 )
	{
	   LOGE("Error: set QSEECom_set_bandwidth=false returned %d", ret);
	   printf("%s: Error: set QSEECom_set_bandwidth=false returned %d\n",
	   	      __func__,ret);
	  return -1;
	}

        /* Shutdown the application */
	ret = qsc_shutdown_app(&l_QSEEComHandle);
	if (ret) {
	   err++;
	   LOGE("Failed to shutdown app: %d",ret);
	   printf("Failed to shutdown app: %d",ret);
	}


	return err;
}


int32_t qsc_run_start_stop_app_basic_test(uint32_t dummy_cmd,int thread)
{
	uint32_t i = 0;
	int32_t ret = 0;			/* return value */
	int32_t err = 0;			/* return value */
	struct qsc_send_cmd send_cmd;
	struct QSEECom_handle *l_QSEEComHandle = NULL;

        printf("Basic_start_stop_test: cmd %d, thread %d\n", dummy_cmd, thread + 1);
        LOGD("Starting qsc_run_start_stop_app_basic_test: thread %d",thread + 1);
        printf("(This may take a few minutes please wait....)\n");
	/* Start the application */
        for  (i = 0;  i < test_iterations;  i++) {
	  LOGD("iteration %d", i);
	  ret = qsc_start_app(&l_QSEEComHandle,appname, 1024);
	   if (ret) {
	     LOGE("Start app: fail");
	   printf("Start app: fail\n");
	   err++;
	   break;
    	   }
	   /* Send data using send command to QSEE application */
	   send_cmd.cmd_id = CLIENT_CMD1_BASIC_DATA;
           send_cmd.start_pkt = 0;
           send_cmd.end_pkt = 0;
           send_cmd.test_buf_size = 0;
	   ret = qsc_issue_send_cmd_req(l_QSEEComHandle, &send_cmd);
    	   /* Shutdown the application */
	   ret = qsc_shutdown_app(&l_QSEEComHandle);
	   if (ret) {
             LOGE("Failed to shutdown app: %d",ret);
             printf("Failed to shutdown app: %d\n",ret);
	   }
        }
	printf("qsc_run_start_stop_app_basic_test PASSED for thread %d\n", 
		    thread + 1);	
        return err;
}


int32_t qsc_run_get_version(uint32_t dummy_cmd,int thread)
{
	uint32_t i = 0;
	int32_t ret = 0;			/* return value */
	int32_t err = 0;			/* return value */
	struct qsc_send_cmd send_cmd;
	struct QSEECom_handle *l_QSEEComHandle = NULL;


	printf("Starting qsc_run_get_version test: cmd = %d, thread = %d\n",
		dummy_cmd, thread + 1);
	/* Start the application */
        for  (i = 0;  i < test_iterations;  i++) {
		LOGD("iteration %d", i);
		ret = qsc_start_app(&l_QSEEComHandle,appname, 1024);
		if (ret) {
			LOGE("Start app: fail");
			printf("Start app: fail\n");
			err++;
			break;
		}
		/* Send data using send command to QSEE application */
		send_cmd.cmd_id = CLIENT_CMD0_GET_VERSION;
		ret = qsc_issue_send_cmd_req(l_QSEEComHandle, &send_cmd);
		/* Shutdown the application */
		ret = qsc_shutdown_app(&l_QSEEComHandle);
		if (ret) {
			LOGE("Failed to shutdown app: %d",ret);
			printf("Failed to shutdown app: %d\n",ret);
		}
	}
	printf("qsc_run_get_version PASSED \n");
	return err;
}

static int _qsc_basic_thread_test(int thread)
{
	int errors = 0;
	int i;
	int32_t dummy_cmd = 0;

	errors += qsc_run_start_stop_app_basic_test(dummy_cmd,thread);

	if (errors) {
	   printf("Number of failures in thread %d stress test: 0x%x\n",
					thread, errors);
	} else {
	   printf("Thread %d stress test: PASSED\n", thread + 1);
	}
	return errors;

}

static void *_qsc_test_thread(void *arg)
{
	uint32_t thread_num = *(uint32_t *)arg;

	uint32_t  errors = 0;

	printf("QSC:    Thread %d starting...\n", thread_num + 1);

	errors =  _qsc_basic_thread_test(thread_num);
	if (errors)
	   pthread_exit(&errors);

	if (errors)
	   printf("Thread %d failed \n",
				thread_num);

	pthread_exit(&errors);

	return NULL;

}

static int32_t qsc_stress_test_thread(uint32_t dummy_cmd, int dummy_thread)
{
	int i, rc;
	uint32_t *thread_errs;
	uint32_t errors = 0;
	static pthread_t threads[2];

	printf("\n\n");
	printf("----------------------------------------------\n");
	printf(" Running Multi-Client BASIC Test: cmd = %d, thread = %d\n",
		dummy_cmd, dummy_thread + 1);
	printf("----------------------------------------------\n");

	pthread_mutex_init(&plock, 0);

	for (i = 0; i < NUM_CLIENT_THREADS; i++) {
	   rc = pthread_create(&threads[i], NULL, _qsc_test_thread,
				(void *)&i);
           if(rc) {
	      printf("QSC:Couldn't create thread %d\n", i + 1);
	      return 1;
	   }else{
               printf("QSC: created thread %d\n",i+1);
	   }
	}

	for (i = 0; i < NUM_CLIENT_THREADS; i++) {
	   rc = pthread_join(threads[i], (void **)&thread_errs);
	   if(rc){
	      printf("QSC:Couldn't join thread %d\n", i + 1);
	      return 1;
	   }else{
               printf("QSC: joined the thread %d\n", i + 1);
	   }
	   errors += *thread_errs;
	}
	return errors;
}

static int32_t qsc_regmem_test()
{
	struct QSEECom_handle *l_QSEEComHandle = NULL;
	struct qsc_send_cmd send_cmd;
        int32_t err = 0;/* error count  */

	int32_t ret = qsc_start_app(&l_QSEEComHandle,appname, 1024);
        if(ret) {
	  LOGE("Start app: fail");
	  printf("Start app: fail\n");
	  err++;
	  return ret;
	 }else{
	    LOGD("Start app: pass");
	    printf("Start app: pass\n");
	 }
	 /* Send data using send modified command to QSEE application. */
	 send_cmd.cmd_id = CLIENT_CMD2_REGISTER_SB_PTR;
	 send_cmd.start_pkt = 0;
	 send_cmd.end_pkt = 0;
	 send_cmd.test_buf_size = 0;
	 ret = qsc_issue_send_modified_cmd_req(l_QSEEComHandle, &send_cmd);
	 if (ret){
	   LOGE("send modified cmd : fail");
	   printf("%s: Send modified cmd fail\n",__func__);
         }else{
	   LOGD("send modified cmd CLIENT_CMD2_REGISTER_SB_PTR: pass");
         }
	 /* Shutdown the application */
	 ret = qsc_shutdown_app(&l_QSEEComHandle);
	 if (ret) {
	   LOGE("   Failed to shutdown app: %d",ret);
	   printf("	Failed to shutdown app: %d\n",ret);
	 }
     return ret;
}


int32_t qsc_run_crypto_rsa_test(uint32_t dummy_cmd,int thread)
{
	uint32_t i = 0;
	int32_t ret = 0;			/* return value */
	int32_t err = 0;			/* return value */
	struct qsc_send_cmd send_cmd;
	struct QSEECom_handle *l_QSEEComHandle = NULL;

	printf("Running RSA test: cmd = %d, thread = %d\n",
		dummy_cmd, thread + 1);

	/* Start the application */
	ret = qsc_start_app(&l_QSEEComHandle,appname, 1024);
	if (ret) {
		LOGE("Start app: fail");
		printf("Start app: fail\n");
		return -1;
	}
	if(l_QSEEComHandle == NULL) {
		LOGE("Failed to get QSEECOM handle, exiting RSA test\n");
		printf("Failed to get QSEECOM handle, exiting RSA test\n");
		return -1;
	}
	/* Send data using send command to QSEE application */
	send_cmd.cmd_id = CLIENT_CMD7_RUN_CRYPTO_RSA_TEST;
        send_cmd.start_pkt = 0;
        send_cmd.end_pkt = 0;
        send_cmd.test_buf_size = 0;

	ret = QSEECom_set_bandwidth(l_QSEEComHandle, true);
	if (ret) {
		LOGE("Set_bandwidth(true) failed\n");
		return -1;
	}
	for(i = 0; i < test_iterations; i++) {
		ret = qsc_issue_send_cmd_req(l_QSEEComHandle, &send_cmd);
		if (ret) {
			LOGE("RSA: semd_cmd_req failed: iteration %d", i+1);
			printf("RSA: semd_cmd_req failed: iteration %d\n", i+1);
			err++;
			break;
		}
	}
	ret = QSEECom_set_bandwidth(l_QSEEComHandle, false);
	if (ret)
		LOGE("Set_bandwidth(false) failed");
	/* Shutdown the application */
	ret = qsc_shutdown_app(&l_QSEEComHandle);
	if (ret) {
		LOGE("Failed to shutdown app: %d",ret);
		printf("Failed to shutdown app: %d\n",ret);
		err ++;
	}
	if (err)
		printf("qsc_run_crypto_rsa_test FAILED \n");
	else
		printf("qsc_run_crypto_rsa_test PASSED \n");
	return err;
}

int32_t qsc_rpmb_provision_key() {
	int ret = 0;
	struct qseecom_rpmb_provision_key send_buf = {0};

	printf("\t-------------------------------------------------------\n");
	printf("\t WARNING!!! You are about to provision the RPMB key.\n");
	printf("\t This is a ONE time operation and CANNOT be reversed.\n");
	printf("\t-------------------------------------------------------\n");
	printf("\t 0 -> Provision Production key\n");
	printf("\t 1 -> Provision Test key\n");
	printf("\t-------------------------------------------------------\n");
	printf("\t Select an option to proceed: ");

	scanf("%d", &send_buf.key_type);
	if (send_buf.key_type != 0 && send_buf.key_type != 1) {
		printf("Invalid RPMB provision keytype (%d)\n", send_buf.key_type);
		return -1;
	}

	ret = QSEECom_send_service_cmd((void*) &send_buf, sizeof(send_buf),
					NULL, 0, QSEECOM_RPMB_PROVISION_KEY_COMMAND);
	if(!ret)
		printf("RPMB key provisioning completed\n");
	else
		printf("RPMB key provisioning failed (%d)\n", ret);

	return ret;
}

int32_t qsc_rpmb_erase() {
	int ret = 0;
	char input;

	printf("\t---------------------------------------------------------------\n");
	printf("\t WARNING!!! You are about to erase the entire RPMB partition.\n");
	printf("\t----------------------------------------------------------------\n");
	printf("\t Do you want to proceed (y/n)? ");

	input = getchar();
	if (input != 'y')
		return 0;

	ret = QSEECom_send_service_cmd(NULL, 0, NULL , 0, QSEECOM_RPMB_ERASE_COMMAND);
	if(!ret)
		printf("RPMB partition erase completed\n");
	else
		printf("RPMB partition erase failed (%d)\n", ret);

	return ret;
}

static int32_t (*qsc_test_func[]) (uint32_t, int) = {
	[BASIC] = qsc_run_start_stop_app_basic_test,
	[CRYPTO] = qsc_run_crypto_test,
	[PERFORMANCE] = qsc_run_crypto_performance_test,
	[STRESS] = qsc_stress_test_thread,
        [RSA]  = qsc_run_crypto_rsa_test,
};

static void qsc_legacy_usage(int ret)
{

	printf("*************************************************************\n");
	printf("************       QSC LEGACY TEST APP             **********\n");	
	printf("*************************************************************\n");	
	printf("\n"
		"Usage: qseecom_sample_client -[OPTION] -[TEST_TYPE0]..-[TEST_TYPEn]\n"
		"Runs the user space tests specified by the TEST_TYPE\n"
		"\n parameter(s).\n"
		"\n\n"
		"OPTION can be:\n"
		"  -v, --verbose       Tests with debug messages on\n\n"
		"TEST_TYPE can be:\n"
		"  -b, --basic         Test CLIENT_CMD1_BASIC_DATA command.\n"
		"  -c, --crypto        Test crypto operations (1KB-32KB packets) \n"
		"  -p, --cryptoperf    Test crypto performance (1KB-64KB packets)\n"
		"  -r, --crypto_rsa    Test crypto RSA test \n"
		"  -e, --exit          Exit\n"
		"  -h, --help          Print this help message and exit\n\n\n"
		" - 'adb push <BUILD>out/target/product/<TARGET>/obj/EXECUTABLES"
		" /qseecom_sample_client_intermediates/qseecom_sample_client /data'\n"
		" - Connect to device: From command shell, do 'adb shell'\n"
		" - Once in the shell: do 'cd /data' \n"
		" - Change permission for qseecom_sample_client: "
		"   do  chmod 777 qseecom_sample_client\n"
		" - Run qseecom_sample_client:\n"
		"  do './qseecom_sample_client -<OPTION> -<TEST_TYPE0> -<TEST_TYPE1>'\n"
		"---------------------------------------------------------\n\n\n");

   exit(ret);

}

static void qsc_usage(void)
{

	printf("*************************************************************\n");
	printf("************       QSC LATEST TEST APP           ************\n");	
	printf("*************************************************************\n");
	printf("\n"
		"Runs the user space tests specified by the TEST_TYPE\n"
		"OPTION can be:\n"
		" v,             : Tests with debug messages on\n\n"
		" <sampleappname>: Name of the 'sampleapp' that needs to be tested with\n"
		" \t Ex: sampleapp, sampleapp2...\n"
		" <cmd_id>   cmd_id of the test(below)\n"
		" \t\t 0: CLIENT_CMD0_GET_VERSION\n"
		" \t\t 1: CLIENT_CMD1_BASIC_DATA\n"
 		" \t\t 2: CLIENT_CMD2_REGISTER_SB_PTR\n"
		" \t\t 3: CLIENT_CMD3_RUN_CRYTPO_TEST \n"
		" \t\t 4: CLIENT_CMD4_RUN_CRYTPO_PERF\n"
		" \t\t 5: CLIENT_CMD5_RUN_SFS_TEST\n"
		" \t\t 6: CLIENT_CMD6_RUN_BUF_ALIGNMENT_TEST\n"
		" \t\t 7: CLIENT_CMD7_RUN_CRYPTO_RSA_TEST\n"
		" \t\t 8: CLIENT_CMD8_RUN_RSA_PERF_TEST\n"
		" \t\t 9: CLIENT_CMD9_RUN_CMNLIB_TEST\n"
		" \t\t 10: CLIENT_CMD10_RUN_CORE_TEST\n"
		" \t\t 11: CLIENT_CMD11_RUN_SECURECHANNEL_TEST\n"
		" \t\t 12: CLIENT_CMD12_RUN_SERVICES_TEST\n"
		" \t\t 13: CLIENT_CMD13_RUN_MISC_TEST\n"
		" \t\t 14: PROVISION RPMB KEY\n"
		" \t\t 15: ERASE RPMB PARTITION\n"
		" \t\t 17: CLIENT_CMD17_RUN_STORAGE_TEST\n"
		"<Iterations>	 : Test iterations (> 0) \n"
		" e          	 :Exit\n"
		" h              :Print this help message and exit\n\n\n"
		" - 'adb push <BUILD>out/target/product/<TARGET>/obj/EXECUTABLES"
		" /qseecom_sample_client_intermediates/qseecom_sample_client /data'\n"
		" - Connect to device: From command shell, do 'adb shell'\n"
		" - Once in the shell: do 'cd /data' \n"
		" - Change permission for qseecom_sample_client: \n"
		"   do  chmod 777 qseecom_sample_client\n"
		" - Run qseecom_sample_client:\n"
		"  do './qseecom_sample_client v <sampleappname> <CMD_ID> <Iterations>'\n"
		"---------------------------------------------------------\n\n\n");

}

static unsigned int qsc_parse_legacy_command(int argc, char *const argv[])
{
	int command = 0;
	unsigned int ret = 0;

	verbose = 0;

	while ((command = getopt_long(argc, argv, "vbcpsrih", qsc_testopts,
				      NULL)) != -1) {
		switch (command) {
		case 'v':
			verbose = 1;
			break;
		case 'b':
			ret |= 1 << BASIC;
			break;
		case 'c':
			ret |= 1 << CRYPTO;
			break;
		case 'p':
			ret |= 1 << PERFORMANCE;
			break;
		case 's':
			ret = 1 << STRESS;
			break;
		case 'r':
			ret = 1 << RSA;
			break;
		case 'i':
			ret = 1 << INTERNAL;
			break;
		case 'h':
		default:
			return 0;
			break;
		}
	}
	return ret;
}



void qsc_internal_test_menu(void)
{
	printf("\n\n");
	printf("Currently implemented tests are:\n");
	printf("\n");
	printf("  Input    1	TEST CASE\n");
	printf(" -----------	----------\n");
	printf("0 --->  Display Menu\n");
	printf("1 --->  Buffer align -- Test addr of unaligned buffer by"
		    "sample app\n");
	printf("2 --->  Register Memory -- Test QSEE APIs for registering memory"
		    "pointed to withina command structure\n");
        printf("3: ---> Run Basic test with multiple clients\n");
}

void qsc_internal_test(int argc, char **argv)
{
	int test_number = 0, dummy_thread = 0, dummy_cmd = 0;
	int i = 0;

	if (argc != 1 || !argv[0]) {
	   qsc_internal_test_menu();
           return;
	}
	fp = fopen("test.txt", "w+");
	test_number = atoi(argv[0]);

	switch (test_number) {
	  case 0:
             qsc_internal_test_menu();
             break;
	  case 1:
             qsc_run_crypto_test(CLIENT_CMD6_RUN_BUF_ALIGNMENT_TEST, dummy_thread);
             break;
          case 2:
	     qsc_regmem_test();
             break;
          case 3:
             qsc_stress_test_thread(dummy_cmd,dummy_thread);
             break;
	  default:
	     break;
	}
}

int qsc_run_legacy_test_app(int argc, char *argv[])
{
       int32_t i = 0,num_tests_failed = 0, rc = 0;
       uint32_t dummy_cmd = 0;
       int dummy_thread = 0;

       unsigned int j = 0;
	unsigned int test_mask = qsc_parse_legacy_command(argc, argv);

	appname = legacy_app_name; /* Default appname is 'sampleapp' */

	if(test_mask == 0) {
		LOGD("Note: Command line arguments do notbelong to legacy test\n");
		printf("Note: Command line arguments do not belong to legacy test\n");
		return NON_LEGACY_CMD_LINE_ARGS;
	}
       printf("Starting qsecom sample client v1.3\n");
       LOGD("Starting qsecom sample client v1.3");

       if((test_mask & (1 << INTERNAL)) == (1U << INTERNAL)) {
         qsc_internal_test(argc-2, &argv[2]);
       }else{
         for (i = 0; i < LAST_TEST; i++){
           /* Look for the test that was selected */
       	   if(!(test_mask & (1U << i)))
             continue;
           /* This test was selected, so run it */
           rc = qsc_test_func[i](dummy_cmd,dummy_thread);
           if(rc){
             printf("%s test case FAILED! rc:%d\n",
             qsc_testopts[i].name, rc);
             num_tests_failed++;
           }
         }
       }
       return 0;

}

int qsc_run_latest_test_app(char *argv[])
{
	int i =0;
	int ret = 0;
	int dummy_cmd_id = 0, dummy_thread = 0;

	if(argv == NULL) {
		printf("No arguments to process, exiting! \n");
		LOGE("No arguments to process, exiting!");
		return -1;
	}
	/* Verbose */
	verbose = atoi(argv[1]);
	/* APP_NAME */
	appname = argv[2];
	/* Iterations */
	test_iterations = atoi(argv[4]);
	/* CMD_ID */
	switch(atoi(argv[3])) {
	case 0:
		ret = qsc_run_get_version(dummy_cmd_id,dummy_thread);
		break;
	case 1:
		ret = qsc_run_start_stop_app_basic_test(dummy_cmd_id,dummy_thread);
		break;
	case 2:
		ret = qsc_regmem_test();
		break;
	case 3:
		ret = qsc_run_crypto_test(CLIENT_CMD3_RUN_CRYTPO_TEST,dummy_thread);
		break;
	case 4:
		ret = qsc_run_crypto_performance_test(dummy_cmd_id,dummy_thread);
		break;
	case 6:
		ret = qsc_run_crypto_test(CLIENT_CMD6_RUN_BUF_ALIGNMENT_TEST,dummy_thread);
		break;
	case 7:
		ret = qsc_run_crypto_rsa_test(dummy_cmd_id,dummy_thread);
		break;
	case 14:
		ret = qsc_rpmb_provision_key();
		break;
	case 15:
		ret = qsc_rpmb_erase();
		break;
	default:
		ret = qsc_run_crypto_test(atoi(argv[3]),dummy_thread);
		break;
	}
	return ret;
}

void qsc_reset_legacy_params(void)
{
	appname = legacy_app_name;
	test_iterations  = 1000;
}

int main(int argc, char *argv[])
{
	int32_t i = 0,num_tests_failed = 0, rc = 0;
	uint32_t dummy_cmd = 0;
	int dummy_thread = 0;
	unsigned int j = 0;

	if (NON_LEGACY_CMD_LINE_ARGS ==
			qsc_run_legacy_test_app(argc, argv)) {
		if((argc <= 1) || (!strcmp(argv[1],"e")) || (!strcmp(argv[1],"h"))) {
			qsc_usage();
			qsc_legacy_usage(-1);
		}
		qsc_run_latest_test_app(argv);
	}
	/* Reset legacy params & retain legacy test behavior for sometime*/
	qsc_reset_legacy_params();
	return 0;
}
