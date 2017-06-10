/*
 *
 * This library contains the API to carry out Real Time Audio Calibration
 * which receives/sends data from QACT and sends/receives it to the RTAC driver
 *
 * Copyright (c) 2013-2014 QUALCOMM Technologies, Inc. All Rights Reserved.
 * QUALCOMM Technologies Proprietary and Confidential.
 *
 */

#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <limits.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include "adie-rtac.h"
#include "acdb.h"
#include "acph.h"
#include "acph_update_for_rtc.h"
#include <unistd.h>

#define __packed __attribute__((packed))
#ifdef _ANDROID_
/* definitions for Android logging */
#include <utils/Log.h>
#include "common_log.h"
#else /* _ANDROID_ */
#define LOGI(...)      fprintf(stdout,__VA_ARGS__)
#define LOGE(...)      fprintf(stderr,__VA_ARGS__)
#define LOGV(...)      fprintf(stderr,__VA_ARGS__)
#define LOGD(...)      fprintf(stderr,__VA_ARGS__)
#endif /* _ANDROID_ */

#define ADIE_RTAC_MAJOR_VERSION	1
#define ADIE_RTAC_MINOR_VERSION	0

#define FILE_NAME_LENGTH	200
static char MsmAdieCodecPoke[FILE_NAME_LENGTH] = "/sys/kernel/debug/asoc";
static char MsmAdieCodecPeek[FILE_NAME_LENGTH] = "/sys/kernel/debug/asoc";
static uint32_t found_codec_path = 0;

#define NUMBER_OF_SUBSTRING	3
#define READ_STEP_SIZE 100
#define ADIE_RTC_HEADER_SIZE 2
static int get_adie_codec_file_names(void)
{
	DIR *dir;
	int i;
	struct dirent *dirent;
	char *file_names[NUMBER_OF_SUBSTRING] = {"-snd-card", "_codec", "reg"};

	for(i = 0; i < NUMBER_OF_SUBSTRING; i++) {
		dir = opendir(MsmAdieCodecPeek);
		if (dir == NULL) {
			LOGE("%d (%s) opendir %s failed\n", errno, strerror(errno), MsmAdieCodecPeek);
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
				if (i == NUMBER_OF_SUBSTRING - 1)
					found_codec_path = 1;
			}
		}
		closedir(dir);
	}
	return 0;
}

static int get_adie_register(uint8_t *req_buf_ptr, uint32_t req_buf_len,
			  uint8_t *resp_buf_ptr, uint32_t resp_buf_len,
			  uint32_t *resp_buf_bytes_filled)
{
	int result = ACPH_SUCCESS;
	int     fd = -1;
	char_t *rtc_io_buf = NULL;

	if((resp_buf_ptr != NULL) && (resp_buf_len >= sizeof(ACPH_CMD_GET_ADIE_REGISTER_rsp)))
	{
		int found = 0;
		int32_t nReqBufLen = 0;
		int32_t nRespBufLen = 0;
		uint32_t ultempRegAddr = 0;
		int32_t lRegValue = 0;
		uint32_t regAddr = 0;
		uint32_t regMask = 0;
		size_t numBytes = 0;
		uint32_t *nOutputBufPos = resp_buf_ptr;
		char_t *pInputBuf = (char_t *)req_buf_ptr;
		int32_t rtc_io_buf_size;
		char temp[READ_STEP_SIZE];

		nReqBufLen = req_buf_len;
		/* Req buffer contains 4 bytes of Reg addr and 4 bytes of Mask Value */
		if(nReqBufLen < sizeof(ACPH_CMD_GET_ADIE_REGISTER_req))
		{
			LOGE("[ACPH_CMD_GET_ADIE_REGISTER]->insufficient length of req buffer to get data\n");
			result = ACPH_ERR_LENGTH_NOT_MATCH;
			goto done;
		}
		else
		{
			char_t reg[5], val[3];
			reg[4] = '\0', val[2] = '\0';
			uint32_t offset = 0;

			memcpy(&regAddr, pInputBuf, ACPH_CAL_DATA_UNIT_LENGTH);
			memcpy(&regMask,
				(pInputBuf+ACPH_CAL_DATA_UNIT_LENGTH),
				ACPH_CAL_DATA_UNIT_LENGTH);
			fd = open(MsmAdieCodecPeek, O_RDWR);
			if(fd < 0)
			{
				result = ACPH_ERR_ADIE_INIT_FAILURE;
				LOGE("[ACPH_CMD_GET_ADIE_REGISTER]->ERROR! cannot open adie peek error: %d, path: %s", fd, MsmAdieCodecPeek);
				goto done;
			}
			/* read till end of the file to calculate the filesize, codec_reg is not a regular file so stat operations wont work*/
			while(read(fd,temp,READ_STEP_SIZE));
			/*add 2 bytes for header information like no.of registers etc.*/
			rtc_io_buf_size = lseek(fd,0,SEEK_CUR)+ADIE_RTC_HEADER_SIZE;
			lseek(fd,0,SEEK_SET);/*set the cur position to begining*/
			LOGE("[ACPH_CMD_GET_ADIE_REGISTER]->codec_reg file size =%d",rtc_io_buf_size);

			rtc_io_buf = (char_t *)malloc(rtc_io_buf_size);
			if (rtc_io_buf == NULL)
			{
				result = ACPH_ERR_ADIE_INIT_FAILURE;
				LOGE("[ACPH_CMD_GET_ADIE_REGISTER]->ERROR! cannot allocate memory: %d, path: %s", rtc_io_buf_size, MsmAdieCodecPeek);
				goto done;
			}
			/* First four bytes is register address */
			numBytes = read(fd, rtc_io_buf, rtc_io_buf_size);
			close(fd);
			LOGE("[rtc_apps_intf]->ACPH_CMD_GET_ADIE_REGISTER->byte read[%d]\n",numBytes);
			if (numBytes <= 0)
			{
				result = ACPH_ERR_ADIE_GET_CMD_FAILURE;
				LOGE("[ACPH_CMD_GET_ADIE_REGISTER]->ERROR! length of written bytes does not match expected value %d", numBytes);
				goto done;
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
				LOGE("[ACPH_CMD_GET_ADIE_REGISTER]->reg[%08X],val[%08X]\n",
							   ultempRegAddr, lRegValue);

				if (ultempRegAddr >= CDC_REG_DIG_BASE_READ)
					ultempRegAddr -= CDC_REG_DIG_OFFSET;

				if(ultempRegAddr == regAddr)
				{
					found = 1;
					LOGE("[ACPH_CMD_GET_ADIE_REGISTER]->register[%08X] found from the file!\n",regAddr);
					break;
				}
			}

			/* make sure the conversion is successful */
			if (found == 0)
			{
				result = ACPH_ERR_ADIE_GET_CMD_FAILURE;
				LOGE("[ACPH_CMD_GET_ADIE_REGISTER]->ERROR! get adie register[0x%x] failed Peek(%s) Poke(%s)",
					regAddr, MsmAdieCodecPeek, MsmAdieCodecPoke);
				goto done;
			}
			else
			{
				LOGE("ACPH_CMD_GET_ADIE_REGISTER command success. Found the value for register = 0x%X, value = 0x%X\n",regAddr,lRegValue);
			}
			/* return a masked value */
			lRegValue &= regMask;
			memcpy((void *)nOutputBufPos,(const void *)&lRegValue, sizeof(uint32_t));
			*resp_buf_bytes_filled = ACPH_CAL_DATA_UNIT_LENGTH;
		}
	}
	else
	{
		result = ACPH_FAILURE;
	}
done:
	free(rtc_io_buf);
	return result;
}

static int set_adie_register(uint8_t *req_buf_ptr, uint32_t req_buf_len,
			  uint32_t *resp_buf_bytes_filled)
{
	int result = ACPH_SUCCESS;
	int     fd = -1;

	if(resp_buf_bytes_filled != NULL)
	{
		int32_t nReqBufLen = 0;
		int32_t nRespBufLen = 0;
		uint32_t ulRegValue = 0;
		uint32_t regAddr = 0;
		uint32_t regMask = 0;
		size_t numBytes1 = 0;
		size_t numBytes2 = 0;
		char *temp = NULL;
		char_t *pInputBuf = (char_t *)req_buf_ptr;

		nReqBufLen = req_buf_len;
		if(nReqBufLen < sizeof(ACPH_CMD_SET_ADIE_REGISTER_req))
		{
			LOGE("[ACPH_CMD_SET_ADIE_REGISTER]->insufficient length of req buffer to get data\n");
			result = ACPH_ERR_LENGTH_NOT_MATCH;

			goto done;
		}
		else
		{
			memcpy(&regAddr, pInputBuf, ACPH_CAL_DATA_UNIT_LENGTH);
			memcpy(&regMask,
				(pInputBuf + ACPH_CAL_DATA_UNIT_LENGTH),
				ACPH_CAL_DATA_UNIT_LENGTH);
			memcpy(&ulRegValue,
				(pInputBuf + 2*ACPH_CAL_DATA_UNIT_LENGTH),
				ACPH_CAL_DATA_UNIT_LENGTH);
			/* set the value as masked one*/
			ulRegValue &= regMask;

			if (regAddr >= CDC_REG_DIG_BASE_WRITE)
				regAddr += CDC_REG_DIG_OFFSET;

			numBytes1 = asprintf(&temp, "0x%x 0x%x", regAddr, ulRegValue);
			LOGE("set register request received for ==> reg[%X], val[%X], bytes[%d]\n",
				 regAddr, ulRegValue, numBytes1);
			fd = open(MsmAdieCodecPoke, O_RDWR);
			if(fd < 0)
			{
				result = ACPH_ERR_ADIE_INIT_FAILURE;
				LOGE("[ACPH_CMD_SET_ADIE_REGISTER]->ERROR! cannot open adie poke error: %d, path: %s", fd, MsmAdieCodecPoke);
				if (temp != NULL)
					{
						free(temp);
						temp=NULL;
					}

				goto done;
			}
			numBytes2 = write(fd, temp, numBytes1);
			LOGE("set register ==> actual bytes written[%d]\n", numBytes2);
			if (temp != NULL)
				{
					free(temp);
					temp=NULL;
				}
			close(fd);
			/* make sure the write is successful */
			if (numBytes1 != numBytes2)
			{
				result = ACPH_ERR_ADIE_SET_CMD_FAILURE;
				LOGE("[ACPH_CMD_SET_ADIE_REGISTER]->ERROR! set adie register failed for Register[0x%X], numBytes[%d]",regAddr ,numBytes1);
				goto done;
			}
			*resp_buf_bytes_filled = 0;
				LOGE("[ACPH_CMD_SET_ADIE_REGISTER]->Success\n");
		}
	}
	else
	{
		result = ACPH_FAILURE;
	}
done:
	return result;
}

static int get_multiple_adie_registers(uint8_t *req_buf_ptr,
			uint32_t req_buf_len, uint8_t *resp_buf_ptr,
			uint32_t *resp_buf_bytes_filled)
{
	int result = ACPH_SUCCESS;
	int     fd = -1;
	char_t *rtc_io_buf = NULL;

	if(NULL != resp_buf_bytes_filled)
	{
		int32_t nReqBufLen = 0;
		int32_t nRespBufLen = 0;
		uint8_t *nOutputBufPos = resp_buf_ptr;
		int32_t nTotalRegisters = 0;
		int32_t lRegValue = 0;
		uint32_t ultempRegAddr = 0;
		size_t numBytes = 0;
		int32_t i=0;
		char_t *pInputBuf = (char_t *)req_buf_ptr;
		char_t *pCurInputBuf = NULL;
		int32_t rtc_io_buf_size;
		char temp[READ_STEP_SIZE];

		nReqBufLen = req_buf_len;
		if(nReqBufLen < ACPH_CAL_DATA_UNIT_LENGTH)
		{
			LOGE("[ACPH_CMD_GET_MULTIPLE_ADIE_REGISTERS]->insufficient length of req buffer to get data\n");
			result = ACPH_ERR_LENGTH_NOT_MATCH;
			goto done;
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

			pCurInputBuf = pInputBuf;
			memcpy(&nTotalRegisters,pCurInputBuf,ACPH_CAL_DATA_UNIT_LENGTH);
			pCurInputBuf += ACPH_CAL_DATA_UNIT_LENGTH;

			if((0<nTotalRegisters)&&
			   ((nTotalRegisters*2*ACPH_CAL_DATA_UNIT_LENGTH) +ACPH_CAL_DATA_UNIT_LENGTH == nReqBufLen))
			{
				fd = open(MsmAdieCodecPeek, O_RDWR);
				if(fd < 0)
				{
					result = ACPH_ERR_ADIE_INIT_FAILURE;
					LOGE("[ACPH_CMD_GET_MULTIPLE_ADIE_REGISTERS]->ERROR! cannot open adie peek error: %d, path: %s", fd, MsmAdieCodecPeek);
					goto done;
				}
				/* read till end of the file to calculate the filesize, codec_reg is not a regular file so stat operations wont work*/
				while(read(fd,temp,READ_STEP_SIZE));
				/*add 2 bytes for header information like no.of registers etc.*/
				rtc_io_buf_size = lseek(fd,0,SEEK_CUR)+ADIE_RTC_HEADER_SIZE;
				lseek(fd,0,SEEK_SET);/*set the cur position to begining*/
				LOGE("[ACPH_CMD_GET_MULTIPLE_ADIE_REGISTERS]->codec_reg file size =%d",rtc_io_buf_size);

				rtc_io_buf = (char_t *)malloc(rtc_io_buf_size);
				if (rtc_io_buf == NULL)
				{
					result = ACPH_ERR_ADIE_INIT_FAILURE;
					LOGE("[ACPH_CMD_GET_MULTIPLE_ADIE_REGISTERS]->ERROR! cannot allocate memory: %d, path: %s",
						rtc_io_buf_size, MsmAdieCodecPeek);
					goto done;
				}
				/* First four bytes is register address */
				numBytes = read(fd, rtc_io_buf, rtc_io_buf_size);
				if (numBytes <= 0)
				{
					/* read failure error */
					result = ACPH_ERR_ADIE_GET_CMD_FAILURE;
					LOGE("[ACPH_CMD_GET_ADIE_REGISTER]->ERROR! length of written bytes does not match expected value %d", numBytes);
					close(fd);
					goto done;
				}
				close(fd);
			}
			/* Perform search for get registers */
			for(i=0; i<nTotalRegisters && found; i++)
			{
				memcpy(&regAddr,pCurInputBuf,ACPH_CAL_DATA_UNIT_LENGTH);
				pCurInputBuf += ACPH_CAL_DATA_UNIT_LENGTH;
				memcpy(&regMask,pCurInputBuf,ACPH_CAL_DATA_UNIT_LENGTH);
				pCurInputBuf += ACPH_CAL_DATA_UNIT_LENGTH;
				LOGE("GET_MULTI-Reg ==> Reg[%08X],Mask[%08X]\n",
						 regAddr, regMask);
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
					if (ultempRegAddr >= CDC_REG_DIG_BASE_READ)
						ultempRegAddr -= CDC_REG_DIG_OFFSET;

					if(ultempRegAddr == regAddr)
					{
						count++;
						found = 1;
						/* return a masked value */
						lRegValue &= regMask;
						memcpy((void *)nOutputBufPos,(const void *)&lRegValue, sizeof(uint32_t));
							   nOutputBufPos += ACPH_CAL_DATA_UNIT_LENGTH;
						usleep(30);
						LOGE("[ACPH_CMD_GET_ADIE_REGISTER]->reg[%08X],val[%08X], count[%d]\n",
								   ultempRegAddr, lRegValue,count);
						break;
					}
				}
			}
			if (found == 0)
			{
				LOGE("GetMultipleAdieReg failed because reg[%08x] is not found\n",regAddr);
				result = ACPH_ERR_ADIE_GET_CMD_FAILURE;
				goto done;
			}
			if(ACPH_SUCCESS==result && nTotalRegisters == count)
			{
				nRespBufLen = sizeof(uint32_t)*(count);
				memcpy((void *)resp_buf_bytes_filled,(const void *)&nRespBufLen,sizeof(int32_t));
				LOGE("[ACPH_CMD_GET_MULTIPLE_ADIE_REGISTERS]->Success\n");
			}
			else
			{
				LOGE("[ACPH_CMD_GET_MULTIPLE_ADIE_REGISTERS]->Error in lengths of input or output buffers or total registers\n");
				result = ACPH_ERR_UNKNOWN_REASON;
				goto done;
			}
		}
	}
	else
	{
		result = ACPH_FAILURE;
	}
done:
	free(rtc_io_buf);
	return result;

}

static int set_multiple_adie_registers(uint8_t *req_buf_ptr, uint32_t req_buf_len,
				uint32_t *resp_buf_bytes_filled)
{
	int result = ACPH_SUCCESS;
	int     fd = -1;

	if(NULL != resp_buf_bytes_filled)
	{
		int32_t nReqBufLen = 0;
		int32_t nRespBufLen = 0;
		char_t *pInputBuf = (char_t *)req_buf_ptr;
		char_t *pCurInputBuf = NULL;
		int32_t nTotalRegisters = 0;
		uint32_t ulRegValue = 0;
		uint32_t regAddr = 0;
		uint32_t regMask = 0;
		size_t numBytes1 = 0;
		size_t numBytes2 = 0;
		uint32_t i=0;

		nReqBufLen = req_buf_len;
		if(nReqBufLen < ACPH_CAL_DATA_UNIT_LENGTH)
		{
			LOGE("[ACPH_CMD_SET_MULTIPLE_ADIE_REGISTERS]->insufficient length of req buffer to get data\n");
			result = ACPH_ERR_LENGTH_NOT_MATCH;
			goto done;
		}
		else
		{
			pCurInputBuf = pInputBuf;
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
					LOGE("SET_MULTI-Reg ==> Reg[%08X],Val[%08X], Mask[%08X]\n",
						 regAddr, ulRegValue, regMask);
					/* set the value as masked one*/
					ulRegValue &= regMask;

					if (regAddr >= CDC_REG_DIG_BASE_WRITE)
						regAddr += CDC_REG_DIG_OFFSET;

					numBytes1 = asprintf(&temp, "0x%x 0x%x", regAddr, ulRegValue);

					fd = open(MsmAdieCodecPoke, O_RDWR);
					if(fd < 0)
					{
						LOGE("[ACPH_CMD_GET_ADIE_REGISTER]->ERROR! cannot open adie poke error: %d, path: %s", fd, MsmAdieCodecPoke);
						result = ACPH_ERR_ADIE_INIT_FAILURE;
						if (temp != NULL)
						{
							free(temp);
							temp=NULL;
						}
						goto done;
					}
					numBytes2 = write(fd, temp, numBytes1);
					if (temp != NULL)
					{
						free(temp);
						temp=NULL;
					}
					close(fd);
					/* make sure the write is successful */
					if (numBytes1 != numBytes2)
					{
						LOGE("[ACPH_CMD_SET_MULTIPLE_ADIE_REGISTERS]->set multi register failed, numBytes1[%d],numBytes2[%d]\n",
									   numBytes1, numBytes2);
						result = ACPH_ERR_ADIE_GET_CMD_FAILURE;
						goto done;
					}
					usleep(30);
				}
				*resp_buf_bytes_filled = 0;
				LOGE("[ACPH_CMD_SET_MULTIPLE_ADIE_REGISTERS]->success\n");
			}
			else
			{
				LOGE("[ACPH_CMD_SET_MULTIPLE_ADIE_REGISTERS]->Error in lengths of input or output buffers or total registers\n");
				result = ACPH_ERR_UNKNOWN_REASON;
				goto done;
			}
		}
	}
	else
	{
		result = ACPH_FAILURE;
	}
done:
	return result;
}

int32_t adie_rtac_callback(uint16_t cmd, uint8_t *req_buf_ptr,
				uint32_t req_buf_len, uint8_t *resp_buf_ptr,
				uint32_t resp_buf_len, uint32_t *resp_buf_bytes_filled)
{
	int result = ACPH_SUCCESS;

	if(!found_codec_path) {
		if (get_adie_codec_file_names())
			LOGE("failed to get Peek and Poke names\n");
	}

	switch(cmd) {
	case ACPH_CMD_QUERY_ADIE_RTC_VERSION:
		((ACPH_CMD_QUERY_ADIE_RTC_VERSION_rsp *)resp_buf_ptr)->adie_rtc_major_version = ADIE_RTAC_MAJOR_VERSION;
		((ACPH_CMD_QUERY_ADIE_RTC_VERSION_rsp *)resp_buf_ptr)->adie_rtc_minor_version = ADIE_RTAC_MINOR_VERSION;
		*resp_buf_bytes_filled = sizeof (ACPH_CMD_QUERY_ADIE_RTC_VERSION_rsp);
		break;
	case ACPH_CMD_GET_ADIE_REGISTER:
		result = get_adie_register(req_buf_ptr, req_buf_len, resp_buf_ptr,
					   resp_buf_len, resp_buf_bytes_filled);
		break;
	case ACPH_CMD_SET_ADIE_REGISTER:
		result = set_adie_register(req_buf_ptr, req_buf_len, resp_buf_bytes_filled);
		break;
	case ACPH_CMD_GET_MULTIPLE_ADIE_REGISTERS:
		result = get_multiple_adie_registers(req_buf_ptr, req_buf_len, resp_buf_ptr,
					   resp_buf_bytes_filled);
		break;
	case ACPH_CMD_SET_MULTIPLE_ADIE_REGISTERS:
		result = set_multiple_adie_registers(req_buf_ptr, req_buf_len,
					   resp_buf_bytes_filled);
		break;
	default:
		LOGE("Cannot recognize the ACPH_ADIE command\n");
		result = ACPH_ERR_INVALID_COMMAND;
		break;
	}
done:
	return result;
}

void adie_rtac_init(void)
{
	int ret;

	ret = acph_register_command(ACPH_ADIE_RTC_REG_SERVICEID, adie_rtac_callback);
	if (ret < 0) {
		LOGE("ACDB RTAC -> acph register failed error = %d\n", ret);
	}

	return;
}

void adie_rtac_exit(void)
{
	int ret;

	ret = acph_deregister_command(ACPH_DSP_RTC_REG_SERVICEID);
	if (ret < 0)
		LOGE("ACDB RTAC -> ERROR: adie_rtac_exit, acph_deregister_command failed err =\n", ret);
}
