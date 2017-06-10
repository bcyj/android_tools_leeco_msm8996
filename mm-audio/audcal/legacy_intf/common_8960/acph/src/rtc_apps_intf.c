/** 
\file **************************************************************************
*
*  A U D I O   C A L I B R A T I O N   P A C K E T   H A N D L E R   
*
*DESCRIPTION
* This file contains the implementation of ACPH 
*
*REFERENCES
* None.
*
* Copyright (c) 2010-2013 by Qualcomm Technologies, Inc.
* All Rights Reserved.
* Qualcomm Technologies Proprietary and Confidential.
*******************************************************************************
*/
/*
-------------------------------
|Include Files                |
-------------------------------
*/
#include "acph.h"
#include <dirent.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include "acdb-id-mapper.h"
#include <linux/msm_audio_acdb.h>

extern uint32_t ACPH_BUFFER_LENGTH;

/*
-------------------------------
|Macros                       |
-------------------------------
*/

extern uint32_t ACPH_HALF_BUF_LEN;
/*
---------------------------------
|Static Variable Definitions    |
---------------------------------
*/
extern char_t * acph_main_buffer;
extern char_t * acph_sub_buffer;

#define FILE_NAME_LENGTH	200
static char MsmAdieCodecPoke[FILE_NAME_LENGTH] = "/sys/kernel/debug/asoc";
static char MsmAdieCodecPeek[FILE_NAME_LENGTH] = "/sys/kernel/debug/asoc";
static uint32_t found_codec_path = 0;
#define NUMBER_OF_SUBSTRING	3

static char* RtacDev = "/dev/msm_rtac";

#define RTC_MAX_ACTIVE_AUD_DEVICES 4
#define RTC_MAX_ACTIVE_AUD_STREAMS 8
#define RTC_MAX_ACTIVE_VOC_DEVICES 2

/** RTC adm data */
typedef struct _rtc_adm_data
{
	uint32_t topology_id;
	uint32_t afe_port;
	uint32_t	copp;
	uint32_t num_of_popp;
	uint32_t	popp[RTC_MAX_ACTIVE_AUD_STREAMS];
} rtc_adm_data;

/** RTC ADM info as an array of adm data */
typedef struct _rtc_adm
{
    uint32_t	num_of_handles;
	rtc_adm_data handle[RTC_MAX_ACTIVE_AUD_DEVICES];
} rtc_adm;

/** RTC voice data */
typedef struct _rtc_voice_data
{
	uint32_t	tx_topology_id;
	uint32_t	rx_topology_id;
	uint32_t	tx_afe_port;
	uint32_t	rx_afe_port;
	uint16_t	cvs_handle;
	uint16_t	cvp_handle;
} rtc_voice_data;

/** RTC voice info as an array of voice data */
typedef struct _rtc_voice
{
	uint32_t	num_of_voice_combos;
	rtc_voice_data	voice_combo[RTC_MAX_ACTIVE_VOC_DEVICES];
} rtc_voice;

#define RTC_IO_BUF_SIZE 5000
static char rtc_io_buf[RTC_IO_BUF_SIZE];

static rtc_adm active_copp_info;
static rtc_voice active_voice_info;

/*
--------------------------------------------------
|Static Function Declarations and Definitions    |
--------------------------------------------------
*/
int32_t adie_execute_command(void *input_buf_ptr,
							 uint32_t *resp_buf_length_ptr
							 );

extern void create_error_resp (uint32_t error_code,
							   char_t *req_buf_ptr,
							   char_t **resp_buf_ptr,
							   uint32_t *resp_buf_length
							   );

extern void create_suc_resp (uint32_t data_length,
							 char_t *req_buf_ptr,
							 char_t **resp_buf_ptr,
							 uint32_t *resp_buf_length
							 );

/**
* FUNCTION : query_aud_topology_copp_handles
*
* DESCRIPTION : query for audio copp handles with a given device
*
* DEPENDENCIES : CSD needs to be available and initialized
*
* PARAMS:
*   req_buf_ptr - pointer to request buffer
*   resp_buf_ptr - pointer to response buffer
*   resp_buf_length - length of the response buffer
*
* RETURN VALUE : None
*
* SIDE EFFECTS : None
*
* Expected data input: None
*
* Expected data output:
*              number of audio copp handles (32-bit)
*              audio_copp_handle_array[n]
*      Each element of audio_copp_handle_array is:
*              topology_id (32-bit)
*              ac_handle (32-bit)
*              audproc_copp_id (32-bit)
*              num_as_handles (32-bit)
*
*/
void query_aud_topology_copp_handles (char_t *req_buf_ptr,
									  char_t **resp_buf_ptr,
									  uint32_t *resp_buf_length
									  )
{
	uint8_t* nBufferPointer = (uint8_t *)(acph_main_buffer + ACPH_ACDB_BUFFER_POSITION);
	size_t bytes_read = 0;
	uint32_t num_as_handles = 0;
	int i, fd, result;
	uint32_t real_acdb_id = 0;
	if (NULL == acph_main_buffer)
	{
		/**not initilized*/
		create_error_resp(ACPH_ERR_UNKNOWN_REASON, req_buf_ptr,
			resp_buf_ptr, resp_buf_length);
		return;
	}

	/** open adm debugfs */
	fd = open(RtacDev, O_RDWR);
	if (fd < 0)
	{
		create_error_resp(ACPH_ERR_CSD_OPEN_HANDLE, req_buf_ptr,
			resp_buf_ptr, resp_buf_length);
#ifdef ACDB_RTC_DEBUG
		ACDB_DEBUG_LOG("[ACDB RTC]->(get aud copp handles)->open device control, response [%d]\n", fd);
#endif
		return;
	}

	/* get the copp info from debugfs */
#ifdef ACDB_RTC_DEBUG
	ACDB_DEBUG_LOG("[ACDB RTC]->(get aud copp info)->enter ioctl\n");
#endif
	bytes_read = ioctl(fd, AUDIO_GET_RTAC_ADM_INFO, &active_copp_info);
#ifdef ACDB_RTC_DEBUG
	ACDB_DEBUG_LOG("[ACDB RTC]->(get aud copp info)->exit ioctl, actual byte read[%d]\n",bytes_read);
#endif
	/* close debugfs after read */
	close(fd);
	if (bytes_read <= 0 || active_copp_info.num_of_handles > RTC_MAX_ACTIVE_AUD_DEVICES)
	{
#ifdef DEBUG_PRINT_ERROR
		DEBUG_PRINT_ERROR("[ACDB RTC ERROR]->(get aud copp handles)->bytes read less than 0 or number of active dev > %d\n",
			RTC_MAX_ACTIVE_AUD_DEVICES);
#endif
		create_error_resp(ACPH_ERR_CSD_AUD_CMD_FAILURE, req_buf_ptr,
			resp_buf_ptr, resp_buf_length);
		return;
	}

	/* prepare response buffer */
	for (i=0; i<(int)active_copp_info.num_of_handles; i++)
	{
		num_as_handles = active_copp_info.handle[i].num_of_popp;

		nBufferPointer += sizeof(uint32_t);
		memcpy(nBufferPointer,
			&(active_copp_info.handle[i].topology_id),
			sizeof(uint32_t));
		nBufferPointer += sizeof(uint32_t);
		memcpy(nBufferPointer,
			&(active_copp_info.handle[i].copp),
			sizeof(uint32_t));
		nBufferPointer += sizeof(uint32_t);
		memcpy(nBufferPointer,
			&(active_copp_info.handle[i].copp),
			sizeof(uint32_t));
		/* find the number of as handles */
		nBufferPointer += sizeof(uint32_t);
		memcpy(nBufferPointer, &num_as_handles, sizeof(uint32_t));
	}
	memcpy(acph_main_buffer + ACPH_ACDB_BUFFER_POSITION,
		&(active_copp_info.num_of_handles), sizeof(uint32_t));
	create_suc_resp(((active_copp_info.num_of_handles)*4)*sizeof(uint32_t)
		+ sizeof(uint32_t), req_buf_ptr, resp_buf_ptr, resp_buf_length);
}

/**
* FUNCTION : query_aud_copp_stream_handles
*
* DESCRIPTION : query for audio stream handles with a given copp handle
*
* DEPENDENCIES : CSD needs to be available and initialized
*
* PARAMS:
*   req_buf_ptr - pointer to request buffer
*   resp_buf_ptr - pointer to response buffer
*   resp_buf_length - length of the response buffer
*
* RETURN VALUE : None
*
* SIDE EFFECTS : None
*
* Expected data input:
*              audio_copp_handle (32-bit)
*              num_as_handles (32-bit)
*
* Expected data output:
*              number of audio stream handles (32-bit)
*              audio_stream_handle_array[n]
*      Each element of audio_stream_handle_array is:
*              audio_stream_handle (32-bit) (including both session ID and stream ID)
*
*/
void query_aud_copp_stream_handles (char_t *req_buf_ptr,
									char_t **resp_buf_ptr,
									uint32_t *resp_buf_length
									)
{
	uint32_t ac_handle = 0;
	uint32_t num_as_handles = 0;
	uint8_t* nBufferPointer = (uint8_t *)(acph_main_buffer + ACPH_ACDB_BUFFER_POSITION);
	int i, j;
	uint32_t ulData_Length;
	memcpy(&ulData_Length,req_buf_ptr + ACPH_DATA_LENGTH_POSITION,
		ACPH_DATA_LENGTH_LENGTH);
	if (2 * ACPH_CAL_DATA_UNIT_LENGTH > ulData_Length)
	{
		/**command parameter missing*/
		create_error_resp(ACPH_ERR_INVALID_COMMAND, req_buf_ptr,
			resp_buf_ptr, resp_buf_length);
		return;
	}
	if (NULL == acph_main_buffer)
	{
		/**not initilized*/
		create_error_resp(ACPH_ERR_UNKNOWN_REASON, req_buf_ptr,
			resp_buf_ptr, resp_buf_length);
		return;
	}

	/**copy value to csd command*/
	memcpy(&ac_handle, 
		req_buf_ptr + ACPH_HEADER_LENGTH, 
		ACPH_CAL_DATA_UNIT_LENGTH);
	for (i=0; i<(int)active_copp_info.num_of_handles; i++)
	{
		if (ac_handle == active_copp_info.handle[i].copp)
		{
			num_as_handles = active_copp_info.handle[i].num_of_popp;
			for (j=0; j<(int)num_as_handles; j++)
			{
				nBufferPointer += sizeof(uint32_t);
				memcpy(nBufferPointer,
					&(active_copp_info.handle[i].popp[j]),
					sizeof(uint32_t));
			}
			break;
		}
	}
	memcpy(acph_main_buffer + ACPH_ACDB_BUFFER_POSITION,
		&num_as_handles, sizeof(uint32_t));
	create_suc_resp((num_as_handles+1)*sizeof(uint32_t),
		req_buf_ptr, resp_buf_ptr, resp_buf_length);
}

/**
* FUNCTION : query_voc_all_active_streams
*
* DESCRIPTION : query for all active voice streams
*
* DEPENDENCIES : CSD needs to be available and initialized
*
* PARAMS:
*   req_buf_ptr - pointer to request buffer
*   resp_buf_ptr - pointer to response buffer
*   resp_buf_length - length of the response buffer
*
* RETURN VALUE : None
*
* SIDE EFFECTS : None
*
* Expected data input: None
*
* Expected data output:
*              number of streams (32-bit)
*              voice_stream_array[n]
*     Each element of voice_stream_array is:
*              cvs_handle (32-bit)
*              voc_vs_handle (32-bit)
*
*/
void query_voc_all_active_streams (char_t *req_buf_ptr,
								   char_t **resp_buf_ptr,
								   uint32_t *resp_buf_length
								   )
{
	/* assumption is there will only be 1 stream, so it will at most
	* have 1 cvs_handle, 1 cvp_handle
	*/
	uint8_t* nBufferPointer = (uint8_t *)(acph_main_buffer + ACPH_ACDB_BUFFER_POSITION);
	size_t bytes_read = 0;
	uint32_t num_streams = 0;
	uint32_t my_cvs_handle = 0;
	int i,fd, result;
	uint32_t real_tx_acdb_id = 0, real_rx_acdb_id = 0;
	if (NULL == acph_main_buffer)
	{
#ifdef DEBUG_PRINT_ERROR
		DEBUG_PRINT_ERROR("[ACDB RTC ERROR]->(get voc all active strm)->null acph_main_buffer\n");
#endif
		/**not initilized*/
		create_error_resp(ACPH_ERR_UNKNOWN_REASON, req_buf_ptr,
			resp_buf_ptr, resp_buf_length);
		return;
	}
	/** open device control debugfs */
	fd = open(RtacDev, O_RDWR);
	if (fd < 0)
	{
		create_error_resp(ACPH_ERR_CSD_OPEN_HANDLE, req_buf_ptr,
			resp_buf_ptr, resp_buf_length);
#ifdef DEBUG_PRINT_ERROR
		DEBUG_PRINT_ERROR("[ACDB RTC ERROR]->(get voc all active strm)->open device control, response [%d]\n", fd);
#endif
		return;
	}

	/* get the voice info from debugfs */
#ifdef ACDB_RTC_DEBUG
	ACDB_DEBUG_LOG("[ACDB RTC]->(get voc all active strm)->enter ioctl\n");
#endif
	bytes_read = ioctl(fd, AUDIO_GET_RTAC_VOICE_INFO, &active_voice_info);
#ifdef ACDB_RTC_DEBUG
	ACDB_DEBUG_LOG("[ACDB RTC]->(get voc all active strm)->exit ioctl, actual byte read[%d]\n",bytes_read);
#endif
	/* close debugfs after read */
	close(fd);

	if (bytes_read <= 0 || active_voice_info.num_of_voice_combos > RTC_MAX_ACTIVE_VOC_DEVICES)
	{
#ifdef ACDB_RTC_DEBUG
		ACDB_DEBUG_LOG("[ACDB RTC]->(get voc all active strm)->bytes read less than 0 or number of active dev pair > %d\n", 
			RTC_MAX_ACTIVE_VOC_DEVICES);
#endif
		create_error_resp(ACPH_ERR_CSD_VOC_CMD_FAILURE,
			req_buf_ptr, resp_buf_ptr, resp_buf_length);
		return;
	}

	/* prepare response buffer */
	for (i=0; i<(int)active_voice_info.num_of_voice_combos; i++)
	{
		num_streams++;
		nBufferPointer += sizeof(uint32_t);
		/* QACT is expecting a 32-bit number */
		my_cvs_handle = (uint32_t)active_voice_info.voice_combo[i].cvs_handle;
		memcpy(nBufferPointer, &my_cvs_handle, sizeof(uint32_t));
		nBufferPointer += sizeof(uint32_t);
		memcpy(nBufferPointer, &my_cvs_handle, sizeof(uint32_t));
	}
	memcpy(acph_main_buffer + ACPH_ACDB_BUFFER_POSITION,
		&num_streams, sizeof(uint32_t));
	create_suc_resp((num_streams*2+1)*sizeof(uint32_t),
		req_buf_ptr, resp_buf_ptr, resp_buf_length);
}

/**
* FUNCTION : query_voc_vs_copp_handles
*
* DESCRIPTION : query for voice copp handles with a given voc_vs_handle
*
* DEPENDENCIES : CSD needs to be available and initialized
*
* PARAMS:
*   req_buf_ptr - pointer to request buffer
*   resp_buf_ptr - pointer to response buffer
*   resp_buf_length - length of the response buffer
*
* RETURN VALUE : None
*
* SIDE EFFECTS : None
*
* Expected data input: voc_vs_handle (32-bit)
*
* Expected data output:
*              number of voice copp handles (32-bit)
*              voice_copp_handle_array[n]
*      Each element of voice_copp_handle_array is:
*              cvp_handle (32-bit)
*              voc_vc_handle (32-bit)
*
*/
void query_voc_vs_copp_handles (char_t *req_buf_ptr,
								char_t **resp_buf_ptr,
								uint32_t *resp_buf_length
								)
{
	/* assumption is for each stream (cvs), there will only be 1 copp (cvp)
	*/
	uint8_t* nBufferPointer = (uint8_t *)(acph_main_buffer + ACPH_ACDB_BUFFER_POSITION);
	uint32_t num_copps = 0;
	int i;
	uint32_t ulData_Length = 0;
	uint32_t cvs_handle = 0;
	uint16_t my_cvs_handle = 0;
	uint32_t my_cvp_handle = 0;

	memcpy(&ulData_Length,req_buf_ptr + ACPH_DATA_LENGTH_POSITION,
		ACPH_DATA_LENGTH_LENGTH);
	if (ACPH_CAL_DATA_UNIT_LENGTH > ulData_Length)
	{
		/**command parameter missing*/
		create_error_resp(ACPH_ERR_INVALID_COMMAND, req_buf_ptr,
			resp_buf_ptr, resp_buf_length);
		return;
	}
	if (NULL == acph_main_buffer)
	{
		/**not initilized*/
		create_error_resp(ACPH_ERR_UNKNOWN_REASON, req_buf_ptr,
			resp_buf_ptr, resp_buf_length);
		return;
	}

	memcpy(&cvs_handle, req_buf_ptr + ACPH_HEADER_LENGTH, 
		ACPH_CAL_DATA_UNIT_LENGTH);
	/* QACT is sending down a 32-bit number */
	my_cvs_handle = (uint16_t) (cvs_handle & 0x0000FFFF);
	/* prepare response buffer */
	for (i=0; i<(int)active_voice_info.num_of_voice_combos; i++)
	{
		if (my_cvs_handle == active_voice_info.voice_combo[i].cvs_handle)
		{
			num_copps++;
			nBufferPointer += sizeof(uint32_t);
			/* QACT is expecting a 32-bit number */
			my_cvp_handle = (uint32_t)active_voice_info.voice_combo[i].cvp_handle;
			memcpy(nBufferPointer, &my_cvp_handle, sizeof(uint32_t));
			nBufferPointer += sizeof(uint32_t);
			memcpy(nBufferPointer, &my_cvp_handle, sizeof(uint32_t));
		}
	}
	memcpy(acph_main_buffer + ACPH_ACDB_BUFFER_POSITION,
		&num_copps, sizeof(uint32_t));
	create_suc_resp((num_copps*2+1)*sizeof(uint32_t),
		req_buf_ptr, resp_buf_ptr, resp_buf_length);
}

/**
* FUNCTION : query_voc_vc_topology
*
* DESCRIPTION : query for voice devices with a given voc_vc_handle
*
* DEPENDENCIES : CSD needs to be available and initialized
*
* PARAMS:
*   req_buf_ptr - pointer to request buffer
*   resp_buf_ptr - pointer to response buffer
*   resp_buf_length - length of the response buffer
*
* RETURN VALUE : None
*
* SIDE EFFECTS : None
*
* Expected data input: voc_vc_handle (32-bit)
*
* Expected data output:
*              number of voice device pairs (32-bit)
*              voice_device_array[n]
*      Each element of voice_device_array is:
*              voice_rx_topology (32-bit)
*              voice_tx_topology (32-bit)
*
*/
void query_voc_vc_topology (char_t *req_buf_ptr,
							char_t **resp_buf_ptr,
							uint32_t *resp_buf_length
							)
{
	/* assumption is there will only be 1 stream, so it will at most
	* have 1 cvs_handle, 1 cvp_handle
	*/
	uint8_t* nBufferPointer = (uint8_t *)(acph_main_buffer + ACPH_ACDB_BUFFER_POSITION);
	uint32_t num_topology_pairs = 0;
	int i;
	uint32_t ulData_Length = 0;
	uint32_t cvp_handle = 0;
	uint16_t my_cvp_handle = 0;

	memcpy(&ulData_Length,req_buf_ptr + ACPH_DATA_LENGTH_POSITION,
		ACPH_DATA_LENGTH_LENGTH);
	if (ACPH_CAL_DATA_UNIT_LENGTH > ulData_Length)
	{
		/**command parameter missing*/
		create_error_resp(ACPH_ERR_INVALID_COMMAND, req_buf_ptr,
			resp_buf_ptr, resp_buf_length);
		return;
	}
	if (NULL == acph_main_buffer)
	{
		/**not initilized*/
		create_error_resp(ACPH_ERR_UNKNOWN_REASON, req_buf_ptr, resp_buf_ptr, resp_buf_length);
		return;
	}

	memcpy(&cvp_handle, req_buf_ptr + ACPH_HEADER_LENGTH, 
		ACPH_CAL_DATA_UNIT_LENGTH);
	/* QACT is sending down a 32-bit number */
	my_cvp_handle = (uint16_t) (cvp_handle & 0x0000FFFF);
	/* prepare response buffer */
	for (i=0; i<(int)active_voice_info.num_of_voice_combos; i++)
	{
		if (my_cvp_handle == active_voice_info.voice_combo[i].cvp_handle)
		{
			num_topology_pairs++;
			nBufferPointer += sizeof(uint32_t);
			memcpy(nBufferPointer, &active_voice_info.voice_combo[i].rx_topology_id,
				sizeof(uint32_t));
			nBufferPointer += sizeof(uint32_t);
			memcpy(nBufferPointer, &active_voice_info.voice_combo[i].tx_topology_id,
				sizeof(uint32_t));
		}
	}
	memcpy(acph_main_buffer + ACPH_ACDB_BUFFER_POSITION,
		&num_topology_pairs, sizeof(uint32_t));
	create_suc_resp((num_topology_pairs*2+1)*sizeof(uint32_t),
		req_buf_ptr, resp_buf_ptr, resp_buf_length);
}

static int get_adie_codec_file_names(void)
{
	DIR *dir;
	int i;
	struct dirent *dirent;
	char *file_names[NUMBER_OF_SUBSTRING] = {"-snd-card", "_codec", "reg"};

	for(i = 0; i < NUMBER_OF_SUBSTRING; i++) {
		dir = opendir(MsmAdieCodecPeek);
		if (dir == NULL) {
#ifdef DEBUG_PRINT_ERROR
			DEBUG_PRINT_ERROR("%d (%s) opendir %s failed\n", errno, strerror(errno), MsmAdieCodecPeek);
#endif
			return 2;
		}

		while (NULL != (dirent = readdir(dir))) {
			if (strstr (dirent->d_name,file_names[i]))
			{
				strlcat(MsmAdieCodecPeek, "/", sizeof(MsmAdieCodecPeek));
				strlcat(MsmAdieCodecPeek, dirent->d_name, sizeof(MsmAdieCodecPeek));
				strlcat(MsmAdieCodecPoke, "/", sizeof(MsmAdieCodecPoke));
				strlcat(MsmAdieCodecPoke, dirent->d_name, sizeof(MsmAdieCodecPoke));

				/* If "reg" found don't search anymore */
				if (i == (NUMBER_OF_SUBSTRING - 1))
					found_codec_path = 1;
			}
		}
		closedir(dir);
	}
	return 0;
}

/*
ADIE execute command
*/
int32_t adie_execute_command(void *input_buf_ptr,
							 uint32_t *resp_buf_length_ptr)
{
	int16_t nCommand = 0;
	int32_t result = ACPH_SUCCESS;
	int     fd = -1;

	if(!found_codec_path) {
		if (get_adie_codec_file_names())
			LOGE("failed to get Peek and Poke names\n");
	}

	memcpy(&nCommand,input_buf_ptr, ACPH_COMMAND_ID_LENGTH);
	switch(nCommand)
	{
	/** this is the init on LA */
	case ACPH_CMD_ADIE_DAL_ATTACH:
		{
			break;
		}
		/** this is the deinit on LA */
	case ACPH_CMD_ADIE_DAL_DETACH:
		{
			break;
		}
	case ACPH_CMD_GET_ADIE_REGISTER:
		{
			if(resp_buf_length_ptr!=NULL)
			{
				int found = 0;
				int32_t nReqBufLen = 0;
				int32_t nRespBufLen = 0;
				uint32_t ultempRegAddr = 0;
				int32_t lRegValue = 0;
				uint32_t regAddr = 0;
				uint32_t regMask = 0;
				size_t numBytes = 0;
				uint32_t nOutputBufPos = (uint32_t)acph_main_buffer + ACPH_ACDB_BUFFER_POSITION;
				char_t *pInputBuf = (char_t *)input_buf_ptr;

				memcpy(&nReqBufLen,
					   (pInputBuf+ACPH_DATA_LENGTH_POSITION),
					   ACPH_DATA_LENGTH_LENGTH);
				if(2*ACPH_CAL_DATA_UNIT_LENGTH > nReqBufLen)
				{
#ifdef ACDB_RTC_DEBUG
					DEBUG_PRINT_ERROR("[ACPH_CMD_GET_ADIE_REGISTER]->insufficient length of req buffer to get data\n");
#endif
					result = ACPH_ERR_LENGTH_NOT_MATCH;
					goto end;
				}
				else
				{
					char_t reg[5], val[3];	
					reg[4] = '\0', val[2] = '\0';
					uint32_t offset = 0;
					
					memcpy(&regAddr,
						(pInputBuf+ACPH_HEADER_LENGTH),
						ACPH_CAL_DATA_UNIT_LENGTH);
					memcpy(&regMask,
						(pInputBuf+ACPH_HEADER_LENGTH+ACPH_CAL_DATA_UNIT_LENGTH),
						ACPH_CAL_DATA_UNIT_LENGTH);

					fd = open(MsmAdieCodecPeek, O_RDWR);
					if(fd < 0)
					{
						result = ACPH_ERR_ADIE_INIT_FAILURE;
#ifdef DEBUG_PRINT_ERROR
						DEBUG_PRINT_ERROR("[ACPH_CMD_GET_ADIE_REGISTER]->ERROR! cannot open adie peek error: %d, path %s", fd, MsmAdieCodecPeek);
#endif						
						goto end;
					}
					
					// First four bytes is register address
					numBytes = read(fd, rtc_io_buf, RTC_IO_BUF_SIZE);
					close(fd);					
#ifdef ACDB_RTC_DEBUG
					ACDB_DEBUG_LOG("[rtc_apps_intf]->ACPH_CMD_GET_ADIE_REGISTER->byte read[%d]\n",numBytes);
#endif
					if (numBytes <= 0)
					{
						// This is an error because if the
						result = ACPH_ERR_ADIE_GET_CMD_FAILURE;
#ifdef DEBUG_PRINT_ERROR
						DEBUG_PRINT_ERROR("[ACPH_CMD_GET_ADIE_REGISTER]->ERROR! length of written bytes does not match expected value %d", numBytes);
#endif
						//close(fd);
						goto end;
					}
					
					while(numBytes>offset)
					{				
						memcpy((void*)reg, (void*)rtc_io_buf+offset, sizeof(uint32_t));
						offset += sizeof(uint32_t);
												
						ultempRegAddr = strtoul(reg, NULL, 16);
						offset += 2;
						memcpy((void*)val, (void*)rtc_io_buf+offset, sizeof(uint16_t));						
						lRegValue = strtol(val, NULL, 16);
						offset += 3;						
#ifdef ACDB_RTC_DEBUG
						ACDB_DEBUG_LOG("[ACPH_CMD_GET_ADIE_REGISTER]->reg[%08X],val[%08X]\n",
									   ultempRegAddr, lRegValue);
#endif						
						if(ultempRegAddr == regAddr)
						{
							found = 1;
#ifdef ACDB_RTC_DEBUG
							ACDB_DEBUG_LOG("[ACPH_CMD_GET_ADIE_REGISTER]->register[%08X] found from the file!\n",regAddr);
#endif
							break;
						}
					}

					/** make sure the conversion is successful */
					if (found == 0)
					{
						result = ACPH_ERR_ADIE_GET_CMD_FAILURE;
#ifdef DEBUG_PRINT_ERROR
						DEBUG_PRINT_ERROR("[ACPH_CMD_GET_ADIE_REGISTER]->ERROR! get adie register[0x%x] failed",regAddr);
#endif
						goto end;
					}
					else
					{
#ifdef ACDB_RTC_DEBUG
						ACDB_DEBUG_LOG("ACPH_CMD_GET_ADIE_REGISTER command success. Found the value for register = 0x%X, value = 0x%X\n",regAddr,lRegValue);
#endif
					}
					/* return a masked value */
					lRegValue &= regMask;
					memcpy((void *)nOutputBufPos,(const void *)&lRegValue, sizeof(uint32_t));
					nRespBufLen = ACPH_CAL_DATA_UNIT_LENGTH;
					memcpy((void *)resp_buf_length_ptr, (const void *)&nRespBufLen, sizeof(int32_t));
				}
			}//check for resp_buf_length_ptr
			break;
		}
	case ACPH_CMD_SET_ADIE_REGISTER:
		{
			if(NULL!=resp_buf_length_ptr)
			{
				int32_t nReqBufLen = 0;
				int32_t nRespBufLen = 0;
				uint32_t ulRegValue = 0;
				uint32_t regAddr = 0;
				uint32_t regMask = 0;
				size_t numBytes1 = 0;
				size_t numBytes2 = 0;
				char *temp = NULL;
				char_t *pInputBuf = (char_t *)input_buf_ptr;

				memcpy(&nReqBufLen,
					(pInputBuf+ACPH_DATA_LENGTH_POSITION),
					ACPH_DATA_LENGTH_LENGTH);
				if(3*ACPH_CAL_DATA_UNIT_LENGTH > nReqBufLen)
				{
#ifdef ACDB_RTC_DEBUG
					ACDB_DEBUG_LOG("[ACPH_CMD_SET_ADIE_REGISTER]->insufficient length of req buffer to get data\n");
#endif
					result = ACPH_ERR_LENGTH_NOT_MATCH;

					goto end;
				}
				else
				{
					memcpy(&regAddr,
						(pInputBuf+ACPH_HEADER_LENGTH),
						ACPH_CAL_DATA_UNIT_LENGTH);
					memcpy(&regMask,
						(pInputBuf+ACPH_HEADER_LENGTH+ACPH_CAL_DATA_UNIT_LENGTH),
						ACPH_CAL_DATA_UNIT_LENGTH);
					memcpy(&ulRegValue,
						(pInputBuf+ACPH_HEADER_LENGTH + 2*ACPH_CAL_DATA_UNIT_LENGTH),
						ACPH_CAL_DATA_UNIT_LENGTH);
					/* set the value as masked one*/
					ulRegValue &= regMask;
					numBytes1 = asprintf(&temp, "0x%x 0x%x", regAddr, ulRegValue);
#ifdef ACDB_RTC_DEBUG
					ACDB_DEBUG_LOG("set register request received for ==> reg[%X], val[%X], bytes[%d]\n",
						 regAddr, ulRegValue, numBytes1);
#endif
					fd = open(MsmAdieCodecPoke, O_RDWR);
					if(fd < 0)
					{
						result = ACPH_ERR_ADIE_INIT_FAILURE;
#ifdef DEBUG_PRINT_ERROR
						DEBUG_PRINT_ERROR("[ACPH_CMD_SET_ADIE_REGISTER]->ERROR! cannot open adie poke error: %d, path: %s", fd, MsmAdieCodecPoke);
#endif
						if (temp != NULL)
							{
								free(temp);
								temp=NULL;
							}

						goto end;
					}
					numBytes2 = write(fd, temp, numBytes1);
#ifdef ACDB_RTC_DEBUG
					ACDB_DEBUG_LOG("set register ==> actual bytes written[%d]\n", numBytes2);
#endif				
					if (temp != NULL)
						{
							free(temp);
							temp=NULL;
						}
					close(fd);
					/** make sure the write is successful */
					if (numBytes1 != numBytes2)
					{
						result = ACPH_ERR_ADIE_SET_CMD_FAILURE;
						ACDB_DEBUG_LOG("[ACPH_CMD_SET_ADIE_REGISTER]->ERROR! set adie register failed for Register[0x%X], numBytes[%d]",regAddr ,numBytes1);
						goto end;
					}
					memcpy((void *)resp_buf_length_ptr, (const void *)&nRespBufLen, sizeof(uint32_t));
#ifdef ACDB_RTC_DEBUG
						ACDB_DEBUG_LOG("[ACPH_CMD_SET_ADIE_REGISTER]->Success\n");
#endif
				}
			}//end of checking for resp_buf_length_ptr
			break;
		}
	case ACPH_CMD_GET_MULTIPLE_ADIE_REGISTERS:
		{
			if(NULL!=resp_buf_length_ptr)
			{
				int32_t nReqBufLen = 0;
				int32_t nRespBufLen = 0;
				uint32_t nOutputBufPos = (uint32_t) acph_main_buffer + ACPH_ACDB_BUFFER_POSITION;
				int32_t nTotalRegisters = 0;
				int32_t lRegValue = 0;
				uint32_t ultempRegAddr = 0;
				size_t numBytes = 0;
				int32_t i=0;
				char_t *pInputBuf = (char_t *)input_buf_ptr;
				char_t *pCurInputBuf = NULL;
				
				memcpy(&nReqBufLen,
					  (pInputBuf+ACPH_DATA_LENGTH_POSITION),
					  ACPH_DATA_LENGTH_LENGTH);
				if(ACPH_CAL_DATA_UNIT_LENGTH>nReqBufLen)
				{
#ifdef ACDB_RTC_DEBUG
					ACDB_DEBUG_LOG("[ACPH_CMD_GET_MULTIPLE_ADIE_REGISTERS]->insufficient length of req buffer to get data\n");
#endif
					result = ACPH_ERR_LENGTH_NOT_MATCH;
					goto end;
				}
				else
				{
					char_t reg[5], val[3];	
					reg[4] = '\0', val[2] = '\0';
					uint32_t offset = 0;
					uint32_t count = 0;
					uint32_t regAddr = 0;
					uint32_t regMask = 0;
					uint32_t found = 1;					
				
					pCurInputBuf = pInputBuf + ACPH_HEADER_LENGTH;
					memcpy(&nTotalRegisters,pCurInputBuf,ACPH_CAL_DATA_UNIT_LENGTH);
					pCurInputBuf += ACPH_CAL_DATA_UNIT_LENGTH;

					if((0<nTotalRegisters)&&
					   ((nTotalRegisters*2*ACPH_CAL_DATA_UNIT_LENGTH) +ACPH_CAL_DATA_UNIT_LENGTH ==nReqBufLen)&&
					   (ACPH_BUFFER_LENGTH >= nReqBufLen)
					   )
					{
						fd = open(MsmAdieCodecPeek, O_RDWR);
						if(fd < 0)
						{
							result = ACPH_ERR_ADIE_INIT_FAILURE;
#ifdef DEBUG_PRINT_ERROR
							DEBUG_PRINT_ERROR("[ACPH_CMD_GET_MULTIPLE_ADIE_REGISTERS]->ERROR! cannot open adie peek error: %d, path %s", fd, MsmAdieCodecPeek);
#endif
							goto end;
						}
						// First four bytes is register address
						numBytes = read(fd, rtc_io_buf, RTC_IO_BUF_SIZE);
						if (numBytes <= 0)
						{
							// read failure error
							result = ACPH_ERR_ADIE_GET_CMD_FAILURE;									
#ifdef DEBUG_PRINT_ERROR
							DEBUG_PRINT_ERROR("[ACPH_CMD_GET_ADIE_REGISTER]->ERROR! length of written bytes does not match expected value %d", numBytes);
#endif
							close(fd);
							goto end;
						}
						close(fd);
					}
					//Perform search for get registers
					for(i=0; i<nTotalRegisters && found; i++)
					{
						memcpy(&regAddr,pCurInputBuf,ACPH_CAL_DATA_UNIT_LENGTH);
						pCurInputBuf += ACPH_CAL_DATA_UNIT_LENGTH;
						memcpy(&regMask,pCurInputBuf,ACPH_CAL_DATA_UNIT_LENGTH);
						pCurInputBuf += ACPH_CAL_DATA_UNIT_LENGTH;
#ifdef ACDB_RTC_DEBUG								
						ACDB_DEBUG_LOG("GET_MULTI-Reg ==> Reg[%08X],Mask[%08X]\n",
								 regAddr, regMask);	
#endif
   					    offset = 0;
						found = 0;
						while(numBytes>offset)
						{				
							memcpy((void*)reg, (void*)rtc_io_buf+offset, sizeof(uint32_t));
							offset += sizeof(uint32_t);											
							ultempRegAddr = strtoul(reg, NULL, 16);
							offset += 2;
							memcpy((void*)val, (void*)rtc_io_buf+offset, sizeof(uint16_t));						
							lRegValue = strtol(val, NULL, 16);
							offset += 3;								
							if(ultempRegAddr == regAddr)
							{
								count++;
								found = 1;
								/* return a masked value */
								lRegValue &= regMask;
								memcpy((void *)nOutputBufPos,(const void *)&lRegValue, sizeof(uint32_t));
									   nOutputBufPos += ACPH_CAL_DATA_UNIT_LENGTH;
 							    usleep(30);
#ifdef ACDB_RTC_DEBUG
								ACDB_DEBUG_LOG("[ACPH_CMD_GET_ADIE_REGISTER]->reg[%08X],val[%08X], count[%d]\n",
										   ultempRegAddr, lRegValue,count);
#endif
								break;
							}
						}
					}
					if (found == 0)
					{
						ACDB_DEBUG_LOG("GetMultipleAdieReg failed because reg[%08x] is not found\n",regAddr);
						result = ACPH_ERR_ADIE_GET_CMD_FAILURE;
						goto end;
					}				
					if(ACPH_SUCCESS==result && nTotalRegisters == count)
					{
						nRespBufLen = sizeof(uint32_t)*(count);
						memcpy((void *)resp_buf_length_ptr,(const void *)&nRespBufLen,sizeof(int32_t));
#ifdef ACDB_RTC_DEBUG
						ACDB_DEBUG_LOG("[ACPH_CMD_GET_MULTIPLE_ADIE_REGISTERS]->Success\n");
#endif
					}
					else
					{
						ACDB_DEBUG_LOG("[ACPH_CMD_GET_MULTIPLE_ADIE_REGISTERS]->Error in lengths of input or output buffers or total registers\n");
						result = ACPH_ERR_UNKNOWN_REASON;
						goto end;
					}
				}
			}//end of checking for resp_buf_length_ptr
			break;
		}
	case ACPH_CMD_SET_MULTIPLE_ADIE_REGISTERS:
		{
			if(NULL!=resp_buf_length_ptr)
			{
				int32_t nReqBufLen = 0;
				int32_t nRespBufLen = 0;
				char_t *pInputBuf = (char_t *)input_buf_ptr;
				char_t *pCurInputBuf = NULL;
				int32_t nTotalRegisters = 0;
				uint32_t ulRegValue = 0;
				uint32_t regAddr = 0;
				uint32_t regMask = 0;
				size_t numBytes1 = 0;
				size_t numBytes2 = 0;
				uint32_t i=0;

				memcpy(&nReqBufLen,
					   (pInputBuf+ACPH_DATA_LENGTH_POSITION),
					   ACPH_DATA_LENGTH_LENGTH);
				if(ACPH_CAL_DATA_UNIT_LENGTH>nReqBufLen)
				{
#ifdef ACDB_RTC_DEBUG
					ACDB_DEBUG_LOG("[ACPH_CMD_SET_MULTIPLE_ADIE_REGISTERS]->insufficient length of req buffer to get data\n");
#endif
					result = ACPH_ERR_LENGTH_NOT_MATCH;
					goto end;
				}
				else
				{
					pCurInputBuf = pInputBuf + ACPH_HEADER_LENGTH;
					memcpy(&nTotalRegisters,pCurInputBuf,ACPH_CAL_DATA_UNIT_LENGTH);
					pCurInputBuf += ACPH_CAL_DATA_UNIT_LENGTH;

					if((0<nTotalRegisters)&&
					   (nReqBufLen==(nTotalRegisters*3*ACPH_CAL_DATA_UNIT_LENGTH)+ ACPH_CAL_DATA_UNIT_LENGTH)
					   )
					{											
						for(i=0;i<nTotalRegisters;i++)
						{
							char *temp = NULL;					
							memcpy(&regAddr,pCurInputBuf,ACPH_CAL_DATA_UNIT_LENGTH);
							pCurInputBuf += ACPH_CAL_DATA_UNIT_LENGTH;
							memcpy(&regMask,pCurInputBuf,ACPH_CAL_DATA_UNIT_LENGTH);
							pCurInputBuf += ACPH_CAL_DATA_UNIT_LENGTH;
							memcpy(&ulRegValue, pCurInputBuf, ACPH_CAL_DATA_UNIT_LENGTH);
							pCurInputBuf += ACPH_CAL_DATA_UNIT_LENGTH;
#ifdef ACDB_RTC_DEBUG							
							ACDB_DEBUG_LOG("SET_MULTI-Reg ==> Reg[%08X],Val[%08X], Mask[%08X]\n",
								 regAddr, ulRegValue, regMask);
#endif
							/* set the value as masked one*/
							ulRegValue &= regMask;

							numBytes1 = asprintf(&temp, "0x%x 0x%x", regAddr, ulRegValue);

							fd = open(MsmAdieCodecPoke, O_RDWR);
							if(fd < 0)
							{								
#ifdef DEBUG_PRINT_ERROR
								ACDB_DEBUG_LOG("[ACPH_CMD_GET_ADIE_REGISTER]->ERROR! cannot open adie poke error: %d, path %s", fd, MsmAdieCodecPoke);
#endif
								result = ACPH_ERR_ADIE_INIT_FAILURE;
								if (temp != NULL)
								{
									free(temp);
									temp=NULL;
								}
								goto end;
							}
							numBytes2 = write(fd, temp, numBytes1);
							if (temp != NULL)
							{
								free(temp);
								temp=NULL;
							}
							close(fd);
							/** make sure the write is successful */
							if (numBytes1 != numBytes2)
							{
								ACDB_DEBUG_LOG("[ACPH_CMD_SET_MULTIPLE_ADIE_REGISTERS]->set multi register failed, numBytes1[%d],numBytes2[%d]\n",
											   numBytes1, numBytes2);
								result = ACPH_ERR_ADIE_GET_CMD_FAILURE;
								goto end;
							}
							usleep(30);
						}//end of for loop
						memcpy((void *)resp_buf_length_ptr, (const void *)&nRespBufLen, sizeof(int32_t));
#ifdef ACDB_RTC_DEBUG
						ACDB_DEBUG_LOG("[ACPH_CMD_SET_MULTIPLE_ADIE_REGISTERS]->success\n");
#endif
					}//end of checking for input buf length and total registers
					else
					{
						ACDB_DEBUG_LOG("[ACPH_CMD_SET_MULTIPLE_ADIE_REGISTERS]->Error in lengths of input or output buffers or total registers\n");
						result = ACPH_ERR_UNKNOWN_REASON;
						goto end;
					}
				}
			}//end of checking for resp_buf_length_ptr
			break;
		}//end of case ACPH_CMD_SET_MULTIPLE_ADIE_REGISTERS
	default:
		{
#ifdef ACDB_RTC_DEBUG
			ACDB_DEBUG_LOG("Cannot recognize the ACPH_ADIE command\n");
			result = ACPH_ERR_INVALID_COMMAND;
#endif
			break;
		}
	}
end:
	return result;
}
