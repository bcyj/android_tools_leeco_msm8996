/*============================================================================

FILE:       rtc_q6_intf.c

DESCRIPTION: abstract transport layer interface file.

PUBLIC CLASSES:  Not Applicable

INITIALIZATION AND SEQUENCING REQUIREMENTS:  N/A

Copyright 2010-2012 Qualcomm Technologies, Inc.
All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
===========================================================================*/

#include <stddef.h>
#include <string.h> /* memcpy */
#include "acph.h"
#include "rtc_q6_intf.h"
#include <linux/msm_audio_acdb.h>

static uint8_t *rtc_pmem = NULL;
static uint8_t *rtc_pmem_aligned = NULL;

#define RTC_PAGE_SIZE            0x1000
#define RTC_SHMEM_SIZE           0x2000
#define RTC_SHMEM_PAYLOAD_LENGTH (3*sizeof(uint32_t))

static char* RtacDev = "/dev/msm_rtac";

extern char_t * acph_main_buffer;
extern char_t * acph_sub_buffer;

extern void create_error_resp (
							   uint32_t error_code,
							   char_t *req_buf_ptr,
							   char_t **resp_buf_ptr,
							   uint32_t *resp_buf_length
							   );

extern void create_suc_resp (
							 uint32_t data_length,
							 char_t *req_buf_ptr,
							 char_t **resp_buf_ptr,
							 uint32_t *resp_buf_length
							 );

typedef enum _RtcOperationMode {
	RTC_GET_PARAM_TYPE = 0,
	RTC_SET_PARAM_TYPE = 1
} RtcOperationMode;

/*===========================================================================
STATIC VARIABLES
===========================================================================*/

/*===========================================================================
INTERNAL FUNCTIONS
===========================================================================*/

/*===========================================================================
FUNCTION rtc_q6_intf_malloc

DESCRIPTION

PARAMS:
size - the size of the buffer to be allocated

RETURN VALUE
None

SIDE EFFECTS
===========================================================================*/
static void* rtc_q6_intf_malloc(uint32_t size
								)
{
	void* tmp;
	tmp = malloc(size);
	if( !tmp)
	{
		return NULL;
	}
	memset(tmp, 0, size);
	return tmp;
}

/*===========================================================================
FUNCTION rtc_q6_intf_init

DESCRIPTION

RETURN VALUE
None

SIDE EFFECTS
===========================================================================*/
int32_t rtc_q6_intf_init(void)
{
	int32_t rc = RTC_INTF_SUCCESS;
	/* pre create the pmem pool */
	rtc_pmem = rtc_q6_intf_malloc(RTC_SHMEM_SIZE + RTC_PAGE_SIZE);
	if( rtc_pmem == NULL ) {
		rc = RTC_INTF_ENORESOURCE;
		goto end;
	}
	rtc_pmem_aligned = (uint8_t *)((((uint32_t) rtc_pmem) + RTC_PAGE_SIZE)
		& 0xFFFFF000);
end:
	return rc;
}

/*===========================================================================
FUNCTION rtc_q6_intf_deinit

DESCRIPTION

RETURN VALUE
None

SIDE EFFECTS
===========================================================================*/
int32_t rtc_q6_intf_deinit(void)
{
	if( rtc_pmem )
	{
		free(rtc_pmem);
		rtc_pmem = NULL;
		rtc_pmem_aligned = NULL;
	}
	return RTC_INTF_SUCCESS;
}

/**
* FUNCTION : rtc_get_q6_cal_data
*
* DESCRIPTION : Get real time calibration data
*
* DEPENDENCIES : DSP must be active
*
* PARAMS:
*   req_buf_ptr - pointer to request buffer
*   resp_buf_ptr - pointer to response buffer
*   resp_buf_length - length of the response buffer
*   mode - indicating set/get operation
*          0 - get_param
*          1 - set_param
*
* RETURN VALUE : None
*
* SIDE EFFECTS : None
*/
static void rtc_transfer_q6_cal_data(char_t *req_buf_ptr,
									 char_t **resp_buf_ptr,
									 uint32_t *resp_buf_length,
									 RtcOperationMode mode
									 )
{
	/*
	*  The request buffer should be in following format:
	*  <CMD_ID: 2 bytes, DATA_LENGTH:4 bytes, DATA>
	*  The format of DATA is:
	*  DST_ADDR is a combination of destination domain and service id (in APR)
	*  DST_DOMAIN: 1-byte value in 4-byte data
	*  DST_SERVICE_ID: 1-byte value in 4-byte data
	*  DST_PORT_ID can be either a copp id or session id + stream id
	*  DST_PORT_ID: 2-byte value in 4-byte data
	*  The rest of DATA should be in the format specified in Elite ISOD docs    
	*/
	uint32_t nPageSize = RTC_PAGE_SIZE;
	int32_t result_flag;
	uint32_t ulData_Length = 0;
	uint32_t service_id = 0;
	uint32_t port_id = 0;
	int32_t payload_size = 0;
	int32_t cal_type = -1;
	int fd;
	size_t bytes_read = 0;
	uint32_t total_size = 0;
	uint32_t payload_addr = 0;
	uint16_t max_param_size = RTC_PAGE_SIZE - 40; 
	uint32_t status_code = 0;

	memcpy(&ulData_Length,req_buf_ptr + ACPH_DATA_LENGTH_POSITION,
		ACPH_DATA_LENGTH_LENGTH);
	if (3 * ACPH_CAL_DATA_UNIT_LENGTH >= ulData_Length)
	{
		/**command parameter missing*/
		create_error_resp(ACPH_ERR_INVALID_COMMAND, req_buf_ptr, resp_buf_ptr, resp_buf_length);
#ifdef LOGE
		LOGE("[ACDB RTC ERROR]->ACPH_ERR_INVALID_COMMAND, data length [%d]\n",resp_buf_length);
#endif
		return;
	}
	if (NULL == acph_main_buffer || NULL == rtc_pmem)
	{
		/**not initilized*/
		create_error_resp(ACPH_ERR_UNKNOWN_REASON, req_buf_ptr, resp_buf_ptr, resp_buf_length);
		if (NULL == acph_main_buffer)
		{
#ifdef LOGE
			LOGE("[ACDB RTC ERROR]->[ACPH buffer]->NULL buffer\n");
#endif
		}
		else
		{
#ifdef LOGE
			LOGE("[ACDB RTC ERROR]->[RTC_PMEM]->NULL buffer\n");
#endif
		}
		return;
	}
	/* get service id */
	memcpy(&service_id,
		req_buf_ptr + ACPH_HEADER_LENGTH + ACPH_CAL_DATA_UNIT_LENGTH, 
		ACPH_CAL_DATA_UNIT_LENGTH);
	/* get port id */
	memcpy(&port_id,
		req_buf_ptr + ACPH_HEADER_LENGTH + 2*ACPH_CAL_DATA_UNIT_LENGTH, 
		ACPH_CAL_DATA_UNIT_LENGTH);
	/* make sure the service id is correct */
	switch (service_id)
	{
	case ADSP_VSM_SERVICE:
		if (mode == RTC_GET_PARAM_TYPE)
		{
			cal_type = AUDIO_GET_RTAC_CVS_CAL;
#ifdef ACDB_RTC_DEBUG
			LOGE("[ACDB RTC]->Q6 get param [AUDIO_GET_RTAC_CVS_CAL]\n");
#endif
		}
		else
		{
			cal_type = AUDIO_SET_RTAC_CVS_CAL;
#ifdef ACDB_RTC_DEBUG
			LOGE("[ACDB RTC]->Q6 set param [AUDIO_SET_RTAC_CVS_CAL]\n");
#endif
		}
		break;
	case ADSP_VPM_SERVICE:
		if (mode == RTC_GET_PARAM_TYPE)
		{
			cal_type = AUDIO_GET_RTAC_CVP_CAL;
#ifdef ACDB_RTC_DEBUG
			LOGE("[ACDB RTC]->Q6 get param [AUDIO_GET_RTAC_CVP_CAL]\n");
#endif
		}
		else
		{
			cal_type = AUDIO_SET_RTAC_CVP_CAL;
#ifdef ACDB_RTC_DEBUG
			LOGE("[ACDB RTC]->Q6 set param [AUDIO_SET_RTAC_CVP_CAL]\n");
#endif
		}
		break;
	case ADSP_ASM_SERVICE:
		if (mode == RTC_GET_PARAM_TYPE)
		{
			cal_type = AUDIO_GET_RTAC_ASM_CAL;
#ifdef ACDB_RTC_DEBUG
			LOGE("[ACDB RTC]->Q6 get param [AUDIO_GET_RTAC_ASM_CAL]\n");
#endif
		}
		else
		{
			cal_type = AUDIO_SET_RTAC_ASM_CAL;
#ifdef ACDB_RTC_DEBUG
			LOGE("[ACDB RTC]->Q6 set param [AUDIO_SET_RTAC_ASM_CAL]\n");
#endif
		}
		break;
	case ADSP_ADM_SERVICE:
		if (mode == RTC_GET_PARAM_TYPE)
		{
			cal_type = AUDIO_GET_RTAC_ADM_CAL;
#ifdef ACDB_RTC_DEBUG
			LOGE("[ACDB RTC]->Q6 get param [AUDIO_GET_RTAC_ADM_CAL]\n");
#endif
		}
		else
		{
			cal_type = AUDIO_SET_RTAC_ADM_CAL;
#ifdef ACDB_RTC_DEBUG
			LOGE("[ACDB RTC]->Q6 set param [AUDIO_SET_RTAC_ADM_CAL]\n");
#endif
		}
		break;
	default:
#ifdef LOGE
		LOGE("[ACDB RTC ERROR]->Unexpected serviceID\n");
#endif
		break;
	}
	if (-1 == cal_type)
	{
		create_error_resp(ACPH_ERR_INVALID_COMMAND, req_buf_ptr, resp_buf_ptr, resp_buf_length);
		return;
	}
	/* read/write rtac device file to get/set param */
	payload_size = ulData_Length - (3*sizeof(uint32_t));
	fd = open(RtacDev, O_RDWR);
	if (fd < 0)
	{
#ifdef LOGE
		LOGE("[ACDB RTC ERROR]->failed to open file %s\n", RtacDev);
#endif
		create_error_resp(ACPH_ERR_CSD_OPEN_HANDLE, req_buf_ptr,
			resp_buf_ptr, resp_buf_length);
		return;
	}
	total_size = payload_size + (2*sizeof(uint32_t));
	memcpy(rtc_pmem, &nPageSize, sizeof(uint32_t));
	memcpy(rtc_pmem + sizeof(uint32_t), &total_size, sizeof(uint32_t));
	memcpy(rtc_pmem + 2*sizeof(uint32_t), &port_id, sizeof(uint32_t));
	memcpy(rtc_pmem + 3*sizeof(uint32_t), &payload_addr, sizeof(uint32_t));

	if (mode == RTC_GET_PARAM_TYPE)
	{
		memcpy(rtc_pmem + 4*sizeof(uint32_t), 
			req_buf_ptr + ACPH_HEADER_LENGTH + 3 * ACPH_CAL_DATA_UNIT_LENGTH, 
			payload_size);
		memcpy(rtc_pmem + 4*sizeof(uint32_t) + payload_size, 
			&max_param_size, 
			sizeof(uint16_t));
#ifdef ACDB_RTC_DEBUG
		LOGE("[ACDB RTC]->[APR send packet format]->[getparam]->page size[%d]\n",*((uint32_t*)rtc_pmem));
		LOGE("[ACDB RTC]->[APR send packet format]->[getparam]->total size[%d]\n",*((uint32_t*)rtc_pmem+1));
		LOGE("[ACDB RTC]->[APR send packet format]->[getparam]->port ID[%08X]\n",*((uint32_t*)rtc_pmem+2)); 
		LOGE("[ACDB RTC]->[APR send packet format]->[getparam]->payload address[%08X]\n",*((uint32_t*)rtc_pmem+3));
		LOGE("[ACDB RTC]->[APR send packet format]->[getparam]->MID[%08X]\n",*((uint32_t*)rtc_pmem+4));
		LOGE("[ACDB RTC]->[APR send packet format]->[getparam]->PID[%08X]\n",*((uint32_t*)rtc_pmem+5));
		LOGE("[ACDB RTC]->[APR send packet format]->[getparam]->max param size[%d]\n",max_param_size);
#endif

#ifdef ACDB_RTC_DEBUG
		LOGE("[ACDB RTC]->(get param)->enter ioctl\n");
#endif
		bytes_read = ioctl(fd, (uint32_t)cal_type, rtc_pmem);
#ifdef ACDB_RTC_DEBUG
		LOGE("[ACDB RTC]->(get param)->exit ioctl, actual byte read[%d]\n",bytes_read);
#endif
	}
	else /* Set_Param */
	{
		memcpy(rtc_pmem + 4*sizeof(uint32_t),
			&payload_size, 
			sizeof(uint32_t));
		memcpy(rtc_pmem + 5*sizeof(uint32_t), 
			req_buf_ptr + ACPH_HEADER_LENGTH + 3 * ACPH_CAL_DATA_UNIT_LENGTH, 
			payload_size);
#ifdef ACDB_RTC_DEBUG
		LOGE("[ACDB RTC]->[APR send packet format]->[setparam]->page size[%d]\n",*((uint32_t*)rtc_pmem));
		LOGE("[ACDB RTC]->[APR send packet format]->[setparam]->total size[%d]\n",*((uint32_t*)rtc_pmem+1));   
		LOGE("[ACDB RTC]->[APR send packet format]->[setparam]->port ID[%08X]\n",*((uint32_t*)rtc_pmem+2)); 
		LOGE("[ACDB RTC]->[APR send packet format]->[setparam]->payload address[%08X]\n",*((uint32_t*)rtc_pmem+3));
		LOGE("[ACDB RTC]->[APR send packet format]->[setparam]->payload size[%d]\n",*((uint32_t*)rtc_pmem+4)); 
		LOGE("[ACDB RTC]->[APR send packet format]->[setparam]->MID[%08X]\n",*((uint32_t*)rtc_pmem+5));
		LOGE("[ACDB RTC]->[APR send packet format]->[setparam]->PID[%08X]\n",*((uint32_t*)rtc_pmem+6));
		LOGE("[ACDB RTC]->[APR send packet format]->[setparam]->param size[%d]\n",*((uint32_t*)rtc_pmem+7));
#endif

#ifdef ACDB_RTC_DEBUG
		LOGE("[ACDB RTC]->(set param)->enter ioctl\n"); 
#endif
		bytes_read = ioctl(fd, (uint32_t)cal_type, rtc_pmem);
#ifdef ACDB_RTC_DEBUG
		LOGE("[ACDB RTC]->(set param)->exit ioctl, actual byte read[%d]\n",bytes_read);
#endif
	}
	close(fd);
	if (bytes_read <= 0)
	{
		create_error_resp(ACPH_ERR_CSD_AUD_CMD_FAILURE, req_buf_ptr,
			resp_buf_ptr, resp_buf_length);
#ifdef LOGE
		LOGE("[ACDB RTC ERROR]->(ACPH_ERR_CSD_AUD_CMD_FAILURE)->failed->Byte read[%d\n", bytes_read);
#endif
		return;
	}
	memcpy(&payload_size, rtc_pmem, sizeof(uint32_t));

#ifdef ACDB_RTC_DEBUG
	if (cal_type == AUDIO_GET_RTAC_CVS_CAL || cal_type == AUDIO_GET_RTAC_CVP_CAL ||
		cal_type == AUDIO_GET_RTAC_ASM_CAL || cal_type == AUDIO_GET_RTAC_ADM_CAL)
	{
		if (*((uint32_t*)rtc_pmem) <= 0)
		{
			LOGE("[ACDB RTC]->[APR return packet format]->[getparam]->payload size is equal to 0\n");
		}
		else
		{
			LOGE("[ACDB RTC]->[APR return packet format]->[getparam]->payload size[%d]\n",*((uint32_t*)rtc_pmem));
			LOGE("[ACDB RTC]->[APR return packet format]->[getparam]->status code[%d]\n",*((uint32_t*)rtc_pmem+1));          
			LOGE("[ACDB RTC]->[APR return packet format]->[getparam]->MID[%08X]\n",*((uint32_t*)rtc_pmem+2));
			LOGE("[ACDB RTC]->[APR return packet format]->[getparam]->PID[%08X]\n",*((uint32_t*)rtc_pmem+3));
			LOGE("[ACDB RTC]->[APR return packet format]->[getparam]->param size[%d]\n",*((uint32_t*)rtc_pmem+4));
			if (*((uint32_t*)rtc_pmem+4) == 0)
			{
				LOGE("[ACDB RTC]->[APR return packet format]->[getparam]->param size is equal to 0\n");
			}
			else
			{
				LOGE("[ACDB RTC]->[APR return packet format]->[getparam]->first 8 byte of data [%08X]\n",
					(uint32_t*)(*((uint8_t*)rtc_pmem+4*sizeof(uint32_t)+1)));
			}
		}
	}
	else if (cal_type == AUDIO_SET_RTAC_CVS_CAL || cal_type == AUDIO_SET_RTAC_CVP_CAL ||
		cal_type == AUDIO_SET_RTAC_ASM_CAL || cal_type == AUDIO_SET_RTAC_ADM_CAL)
	{
		if (*((uint32_t*)rtc_pmem) <= 0)
		{
			LOGE("[ACDB RTC]->[APR return packet format]->[setparam]->payload size is equal to 0\n");
		}
		else
		{
			LOGE("[ACDB RTC]->[APR return packet format]->[setparam]->payload size[%d]\n",*(uint32_t*)rtc_pmem);
			LOGE("[ACDB RTC]->[APR return packet format]->[setparam]->opcode[%08X]\n",*((uint32_t*)rtc_pmem+1));          
			LOGE("[ACDB RTC]->[APR return packet format]->[sgetparam]->status code[%08X]\n",*((uint32_t*)rtc_pmem+2));
		}
	}
#endif

	/* the response buffer is in format:
	* uint32_t payload_size;
	* byte data[payload_size];
	*/
	if (payload_size <= 0 || payload_size >= (int)nPageSize)
	{
#ifdef LOGE
		LOGE("[ACDB RTC ERROR]->response payload size is incorrect: %d\n", payload_size);
#endif
		create_error_resp(ACPH_ERR_CSD_AUD_CMD_FAILURE, req_buf_ptr,
			resp_buf_ptr, resp_buf_length);
		return;
	}
	/* For Get_Param, first 4-byte of payload will be status code
	* Values: 0 success, 1 module not found, 2 param not found
	* For Set_Param, it might be opcode followed by status code
	*/
	/* For Get_Param, the response is size followed by status code
	* For Set_Param, the response is size followed by opcode then status code
	*/
	if (mode == RTC_GET_PARAM_TYPE)
	{
		memcpy(&status_code, rtc_pmem+1*sizeof(uint32_t), sizeof(uint32_t));
	}
	else
	{
		memcpy(&status_code, rtc_pmem+2*sizeof(uint32_t), sizeof(uint32_t));
	}
	if (status_code != 0)
	{
#ifdef LOGE
		LOGE("[ACDB RTC ERROR]->APR response status code: %d\n", status_code);
#endif
		create_error_resp(ACPH_ERR_CSD_AUD_CMD_FAILURE, req_buf_ptr,
			resp_buf_ptr, resp_buf_length);
		return;
	}

	if (mode == RTC_GET_PARAM_TYPE)
	{
		/* for get_param, the payload is not the real payload,
		* it needs to parse the payload to get the real ones
		* payload_size, status_code, module_id, param_id, param_size (2-byte)
		*/
		uint16_t real_param_size = 0;
		memcpy(&real_param_size,
			rtc_pmem + 4*sizeof(uint32_t),
			sizeof(uint16_t));
		if (real_param_size == 0 || payload_size < (int32_t)(real_param_size + 3*sizeof(uint32_t)))
		{
#ifdef LOGE
			LOGE("[ACDB RTC]->returned param size is not correct: %d %d\n",
				payload_size, real_param_size);
#endif
			create_error_resp(ACPH_ERR_CSD_AUD_CMD_FAILURE, req_buf_ptr,
				resp_buf_ptr, resp_buf_length);
			return;
		}
		/* QACT only needs the payload as part of ACPH packet,
		* the size can be calculated based on the total size of ACPH packet.
		* This is the reason to skip payload_size here
		*/
		memcpy(acph_main_buffer + ACPH_ACDB_BUFFER_POSITION,
			rtc_pmem + 2*sizeof(uint32_t),
			real_param_size + 3*sizeof(uint32_t));
		create_suc_resp(real_param_size + 3*sizeof(uint32_t),
			req_buf_ptr, resp_buf_ptr, resp_buf_length);
	}
	else
	{
		create_suc_resp(0, req_buf_ptr, resp_buf_ptr, resp_buf_length);
	}
}

/**
* FUNCTION : rtc_get_q6_cal_data
*
* DESCRIPTION : Get real time calibration data
*
* DEPENDENCIES : DSP must be active
*
* PARAMS:
*   req_buf_ptr - pointer to request buffer
*   resp_buf_ptr - pointer to response buffer
*   resp_buf_length - length of the response buffer
*
* RETURN VALUE : None
*
* SIDE EFFECTS : None
*/
void rtc_get_q6_cal_data (char_t *req_buf_ptr,
						  char_t **resp_buf_ptr,
						  uint32_t *resp_buf_length
						  )
{
	rtc_transfer_q6_cal_data(req_buf_ptr, resp_buf_ptr, resp_buf_length,
		RTC_GET_PARAM_TYPE);
}

/**
* FUNCTION : rtc_set_q6_cal_data
*
* DESCRIPTION : Set real time calibration data
*
* DEPENDENCIES : DSP must be active
*
* PARAMS:
*   req_buf_ptr - pointer to request buffer
*   resp_buf_ptr - pointer to response buffer
*   resp_buf_length - length of the response buffer
*
* RETURN VALUE : None
*
* SIDE EFFECTS : None
*/
void rtc_set_q6_cal_data (char_t *req_buf_ptr,
						  char_t **resp_buf_ptr,
						  uint32_t *resp_buf_length
						  )
{
	rtc_transfer_q6_cal_data(req_buf_ptr, resp_buf_ptr, resp_buf_length,
		RTC_SET_PARAM_TYPE);
}

/**
* FUNCTION : rtc_get_q6_cal_data_shmem
*
* DESCRIPTION : Get real time calibration data using shared memory
*
* DEPENDENCIES : DSP must be active
*
* PARAMS:
*   req_buf_ptr - pointer to request buffer
*   resp_buf_ptr - pointer to response buffer
*   resp_buf_length - length of the response buffer
*
* RETURN VALUE : None
*
* SIDE EFFECTS : None
*/
void rtc_get_q6_cal_data_shmem (char_t *req_buf_ptr,
								char_t **resp_buf_ptr,
								uint32_t *resp_buf_length
								)
{
	rtc_transfer_q6_cal_data(req_buf_ptr, resp_buf_ptr, resp_buf_length,
		RTC_GET_PARAM_TYPE);
}

/**
* FUNCTION : rtc_set_q6_cal_data_shmem
*
* DESCRIPTION : Set real time calibration data using shared memory
*
* DEPENDENCIES : DSP must be active
*
* PARAMS:
*   req_buf_ptr - pointer to request buffer
*   resp_buf_ptr - pointer to response buffer
*   resp_buf_length - length of the response buffer
*
* RETURN VALUE : None
*
* SIDE EFFECTS : None
*/
void rtc_set_q6_cal_data_shmem (char_t *req_buf_ptr,
								char_t **resp_buf_ptr,
								uint32_t *resp_buf_length
								)
{
	rtc_transfer_q6_cal_data(req_buf_ptr, resp_buf_ptr, resp_buf_length,
		RTC_SET_PARAM_TYPE);
}

/*===========================================================================
FUNCTION rtc_q6_intf_send

DESCRIPTION

PARAMS:
send_buf - sending buf ptr (APR packet)
rsp_buf - rsp buf ptr for sync send only
rsp_buf_len - rsp buf length in bytes

RETURN VALUE
None

SIDE EFFECTS
===========================================================================*/
int32_t rtc_q6_intf_send(void* send_buf,
						 void* rsp_buf,
						 uint32_t* rsp_buf_len
						 )
{
	return RTC_INTF_EFAILURE;
}

/*===========================================================================
FUNCTION rtc_q6_intf_send_get_shmem

DESCRIPTION

PARAMS:
send_buf - sending buf ptr (APR packet)
rsp_buf - rsp buf ptr for sync send only
rsp_buf_len - rsp buf length in bytes

RETURN VALUE
None

SIDE EFFECTS
===========================================================================*/
int32_t rtc_q6_intf_send_get_shmem(void* send_buf,
								   void* rsp_buf,
								   uint32_t* rsp_buf_len
								   )
{
	return RTC_INTF_EFAILURE;
}

/*===========================================================================
FUNCTION rtc_q6_intf_send_set_shmem

DESCRIPTION

PARAMS:
send_buf - sending buf ptr (APR packet)
rsp_buf - rsp buf ptr for sync send only
rsp_buf_len - rsp buf length in bytes

RETURN VALUE
None

SIDE EFFECTS
===========================================================================*/
int32_t rtc_q6_intf_send_set_shmem(void* send_buf,
								   void* rsp_buf,
								   uint32_t* rsp_buf_len
								   )
{
	return RTC_INTF_EFAILURE;
}
