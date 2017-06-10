/*
 *
 * This library contains the API to carry out Real Time Audio Calibration
 * which receives/sends data from QACT and sends/receives it to the RTAC driver
 *
 * Copyright (c) 2012-2014 QUALCOMM Technologies, Inc. All Rights Reserved.
 * QUALCOMM Technologies Proprietary and Confidential.
 *
 */

#include <stdint.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/msm_audio_calibration.h>
#include "acdb-rtac.h"
#include "acdb.h"
#include "acph.h"
#include "acph_update_for_rtc.h"
#include <unistd.h>

#ifndef __packed
#define __packed __attribute__((packed))
#endif /* __packed */

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

#define ADSP_CVS_SERVICE                    0x05
#define ADSP_CVP_SERVICE                    0x06
#define ADSP_ASM_SERVICE                    0x07
#define ADSP_ADM_SERVICE                    0x08

#define RTAC_MAJOR_VERSION	1
#define RTAC_MINOR_VERSION	2

#define RTAC_MAX_ACTIVE_DEVICES		4
#define RTAC_MAX_ACTIVE_VOICE_COMBOS	2
#define RTAC_MAX_ACTIVE_POPP		8

/* Makes size of internal buffer 8192 */
#define RTAC_BUFF_SIZE			2048
/* Shared buffer sent to DSP is size 8192 */
#define MAX_PARAM_SIZE			8192
/* size of basic param returned from ADSP */
#define MIN_PARAM_SIZE			12

/* ADM rtac info */
struct rtac_popp_data {
	uint32_t	popp;
	uint32_t	popp_topology;
};

struct rtac_adm_data {
	uint32_t		topology_id;
	uint32_t		afe_port;
	uint32_t		copp;
	uint32_t		num_of_popp;
	struct rtac_popp_data	popp[RTAC_MAX_ACTIVE_POPP];
};

struct rtac_adm {
	uint32_t			num_of_dev;
	struct rtac_adm_data		device[RTAC_MAX_ACTIVE_DEVICES];
};

/* Voice rtac info */
struct rtac_voice_data {
	uint32_t	tx_topology_id;
	uint32_t	rx_topology_id;
	uint32_t	tx_afe_port;
	uint32_t	rx_afe_port;
	uint16_t	cvs_handle;
	uint16_t	cvp_handle;
};

struct rtac_voice {
	uint32_t		num_of_voice_combos;
	struct rtac_voice_data	voice[RTAC_MAX_ACTIVE_VOICE_COMBOS];
};

/* SET/GET PARAM ADSP formats */
struct audio_get_param_payload {
	uint32_t	addr_lsw;
	uint32_t	addr_msw;
	uint32_t	mem_handle;
	uint32_t	module_id;
	uint32_t	param_id;
	uint16_t	param_max_size;
	uint16_t	reserved;
} __packed;

struct audio_set_param_payload {
	uint32_t	addr_lsw;
	uint32_t	addr_msw;
	uint32_t	mem_handle;
	uint32_t	payload_size;
	uint32_t	module_id;
	uint32_t	param_id;
	uint16_t	param_max_size;
	uint16_t	reserved;
} __packed;

struct afe_get_param_payload {
	uint16_t	port_id;
	uint16_t	payload_size;
	uint32_t	addr_lsw;
	uint32_t	addr_msw;
	uint32_t	mem_handle;
	uint32_t	module_id;
	uint32_t	param_id;
} __packed;

struct afe_get_param_resp_payload {
	uint32_t	module_id;
	uint32_t	param_id;
	uint16_t	param_max_size;
	uint16_t	reserved;
} __packed;

struct afe_set_param_payload {
	uint16_t	port_id;
	uint16_t	payload_size;
	uint32_t	addr_lsw;
	uint32_t	addr_msw;
	uint32_t	mem_handle;
	uint32_t	module_id;
	uint32_t	param_id;
	uint16_t	param_max_size;
	uint16_t	reserved;
} __packed;

struct voice_get_param_payload {
	uint32_t	mem_handle;
	uint32_t	addr_lsw;
	uint32_t	addr_msw;
	uint16_t	mem_size;
	uint32_t	module_id;
	uint32_t	param_id;
} __packed;

struct voice_set_param_payload {
	uint32_t	mem_handle;
	uint32_t	addr_lsw;
	uint32_t	addr_msw;
	uint32_t	mem_size;
	uint32_t	module_id;
	uint32_t	param_id;
	uint16_t	param_max_size;
	uint16_t	reserved;
} __packed;

struct send_rtac_data {
	uint32_t	buf_size;
	uint32_t	cmd_size;
	uint32_t	port_id;
	union {
	struct audio_get_param_payload	aud_get;
	struct audio_set_param_payload	aud_set;
	struct afe_get_param_payload	afe_get;
	struct afe_set_param_payload	afe_set;
	struct voice_get_param_payload	voc_get;
	struct voice_set_param_payload	voc_set;
	};
}  __packed;

static int32_t			rtac_driver_handle;
static struct rtac_adm		rtac_adm_data;
static struct rtac_adm		rtac_adm_data;
static struct rtac_voice	rtac_voice_data;

static uint32_t	rtac_buf[RTAC_BUFF_SIZE];

int32_t get_adm_info()
{
	int ret = 0;

	ret = ioctl(rtac_driver_handle, AUDIO_GET_RTAC_ADM_INFO, &rtac_adm_data);
	if (ret < 0) {
		LOGE("ACDB RTAC -> ERROR: AUDIO_GET_RTAC_ADM_INFO errno: %d\n", errno);
		ret = ACPH_ERR_CSD_AUD_CMD_FAILURE;
		goto done;
	}

	ret = ACPH_SUCCESS;
done:
	return ret;
}

int32_t get_voice_info()
{
	int ret = 0;

	ret = ioctl(rtac_driver_handle, AUDIO_GET_RTAC_VOICE_INFO, &rtac_voice_data);
	if (ret < 0) {
		LOGE("ACDB RTAC -> ERROR: AUDIO_GET_RTAC_ADM_INFO errno: %d\n", errno);
		ret = ACPH_ERR_CSD_VOC_CMD_FAILURE;
		goto done;
	}

	ret = ACPH_SUCCESS;
done:
	return ret;
}

int32_t get_aud_topology(uint32_t *aud_top,
			uint32_t *bytes_filled)
{
	int ret = 0;
	uint32_t i;

	ret = get_adm_info();
	if (ret < 0) {
		goto done;
	}

	aud_top[0] = rtac_adm_data.num_of_dev;
	for(i=0; i < rtac_adm_data.num_of_dev; i++) {
		aud_top[(i*4+1)] = rtac_adm_data.device[i].topology_id;
		aud_top[(i*4+2)] = rtac_adm_data.device[i].copp;
		aud_top[(i*4+3)] = rtac_adm_data.device[i].copp;
		aud_top[(i*4+4)] = rtac_adm_data.device[i].num_of_popp;
	}

	*bytes_filled = (i*4+1) * sizeof(uint32_t);
done:
	return ret;
}

int32_t get_aud_streams(ACPH_CMD_QUERY_AUD_COPP_STREAM_HANDLES_req *aud_strm_info,
			uint32_t *aud_strm,
			uint32_t *bytes_filled)
{
	int ret = 0;
	uint32_t i, j;

	ret = get_adm_info();
	if (ret < 0) {
		goto done;
	}

	for(i=0; i < rtac_adm_data.num_of_dev; i++) {
		if (aud_strm_info->copp_handle == rtac_adm_data.device[i].copp) {
			break;
		} else if (i == rtac_adm_data.num_of_dev-1) {
			LOGE("ACDB RTAC -> ERROR: get_aud_streams, copp 0x%x does not match 0x%x in rtac driver\n",
				aud_strm_info->copp_handle, rtac_adm_data.device[i].copp);
			/* default to 0 */
			i = 0;
			break;
		}
	}

	aud_strm[0] = rtac_adm_data.device[i].num_of_popp;
	for(j=0; j < rtac_adm_data.device[i].num_of_popp; j++) {
		aud_strm[j*2+1] = rtac_adm_data.device[i].popp[j].popp;
		aud_strm[j*2+2] = rtac_adm_data.device[i].popp[j].popp_topology;
	}

	*bytes_filled = (j*2+1) * sizeof(uint32_t);
done:
	return ret;
}

int32_t get_voc_streams(uint32_t *voc_strm,
			uint32_t *bytes_filled)
{
	int ret = 0;
	uint32_t i;

	ret = get_voice_info();
	if (ret < 0) {
		goto done;
	}

	voc_strm[0] = rtac_voice_data.num_of_voice_combos;
	for (i=0; i<rtac_voice_data.num_of_voice_combos; i++) {
		voc_strm[i*2+1] = rtac_voice_data.voice[i].cvs_handle;
		voc_strm[i*2+2] = rtac_voice_data.voice[i].cvs_handle;
	}

	*bytes_filled = (i*2+1) * sizeof(uint32_t);
done:
	return ret;
}

int32_t get_voc_copp_handles(ACPH_CMD_QUERY_VOC_VS_COPP_HANDLES_req *voc_copp_info,
			uint32_t *voc_copp,
			uint32_t *bytes_filled)
{
	int ret = 0;
	uint32_t i;

	ret = get_voice_info();
	if (ret < 0) {
		goto done;
	}

	for(i=0; i < rtac_voice_data.num_of_voice_combos; i++) {
		if (voc_copp_info->popp_handle == rtac_voice_data.voice[i].cvs_handle) {
			break;
		} else if (i == rtac_voice_data.num_of_voice_combos-1) {
			LOGE("ACDB RTAC -> ERROR: get_voc_copp_handles, popp 0x%x does not match 0x%x in rtac driver\n",
				voc_copp_info->popp_handle, rtac_voice_data.voice[i].cvs_handle);
			/* default to 0 */
			i = 0;
			break;
		}
	}

	voc_copp[0] = 1;
	voc_copp[1] = rtac_voice_data.voice[i].cvp_handle;
	voc_copp[2] = rtac_voice_data.voice[i].cvp_handle;

	*bytes_filled = 3 * sizeof(uint32_t);
done:
	return ret;
}

int32_t get_voc_topology(ACPH_CMD_QUERY_VOC_VC_TOPOLOGIES_req *voc_top_info,
			uint32_t *voc_top,
			uint32_t *bytes_filled)
{
	int ret = 0;
	uint32_t i;

	ret = get_voice_info();
	if (ret < 0) {
		goto done;
	}

	for(i=0; i < rtac_voice_data.num_of_voice_combos; i++) {
		if (voc_top_info->copp_handle == rtac_voice_data.voice[i].cvp_handle) {
			break;
		} else if (i == rtac_voice_data.num_of_voice_combos-1) {
			LOGE("ACDB RTAC -> ERROR: get_voc_topology, copp 0x%x does not match 0x%x in rtac driver\n",
				voc_top_info->copp_handle, rtac_voice_data.voice[i].cvp_handle);
			/* default to 0 */
			i = 0;
			break;
		}
	}

	voc_top[0] = 1;
	voc_top[1] = rtac_voice_data.voice[i].rx_topology_id;
	voc_top[2] = rtac_voice_data.voice[i].tx_topology_id;

	*bytes_filled = 3 * sizeof(uint32_t);
done:
	return ret;
}

int32_t get_voc_afe_port_id(ACPH_CMD_QUERY_VOC_AFE_ACTIVE_PORT_IDS_req *voc_afe_info,
			uint32_t *voc_afe,
			uint32_t *bytes_filled)
{
	int ret = 0;
	uint32_t i;

	ret = get_voice_info();
	if (ret < 0) {
		goto done;
	}

	for(i=0; i < rtac_voice_data.num_of_voice_combos; i++) {
		if (voc_afe_info->voc_copp_handle == rtac_voice_data.voice[i].cvp_handle) {
			break;
		} else if (i == rtac_voice_data.num_of_voice_combos-1) {
			LOGE("ACDB RTAC -> ERROR: get_voc_afe_port_id, copp 0x%x does not match 0x%x in rtac driver\n",
				voc_afe_info->voc_copp_handle, rtac_voice_data.voice[i].cvp_handle);
			/* default to 0 */
			i = 0;
			break;
		}
	}
	voc_afe[0] = rtac_voice_data.voice[i].tx_afe_port;
	voc_afe[1] = rtac_voice_data.voice[i].rx_afe_port;

	*bytes_filled = 2 * sizeof(uint32_t);
done:
	return ret;
}

void pack_audio_get_rtac_data(ACPH_CMD_RTC_GET_CAL_DATA_req *rtac_info, uint32_t out_buf_size)
{
	struct send_rtac_data	*buf = (struct send_rtac_data *)rtac_buf;

	buf->buf_size = out_buf_size;
	buf->cmd_size = sizeof(struct audio_get_param_payload);
	buf->port_id = rtac_info->pp_id;
	buf->aud_get.addr_lsw = 0;
	buf->aud_get.addr_msw = 0;
	buf->aud_get.mem_handle = 0;
	buf->aud_get.module_id = rtac_info->module_id;
	buf->aud_get.param_id = rtac_info->parameter_id;
	buf->aud_get.param_max_size = MAX_PARAM_SIZE;
	buf->aud_get.reserved = 0;
}

void pack_afe_get_rtac_data(ACPH_GET_AFE_DATA_req *rtac_info, uint32_t out_buf_size)
{
	struct send_rtac_data	*buf = (struct send_rtac_data *)rtac_buf;

	buf->buf_size = out_buf_size;
	buf->cmd_size = sizeof(struct afe_get_param_payload);
	buf->port_id = rtac_info->afe_port_id;
	buf->afe_get.port_id = rtac_info->afe_port_id;
	buf->afe_get.addr_lsw = 0;
	buf->afe_get.addr_msw = 0;
	buf->afe_get.mem_handle = 0;
	buf->afe_get.module_id = rtac_info->module_id;
	buf->afe_get.param_id = rtac_info->parameter_id;
	buf->afe_get.payload_size = MAX_PARAM_SIZE;
}

void pack_voice_get_rtac_data(ACPH_CMD_RTC_GET_CAL_DATA_req *rtac_info, uint32_t out_buf_size)
{
	struct send_rtac_data	*buf = (struct send_rtac_data *)rtac_buf;

	buf->buf_size = out_buf_size;
	buf->cmd_size = sizeof(struct voice_get_param_payload);
	buf->port_id = rtac_info->pp_id;
	buf->voc_get.mem_handle = 0;
	buf->voc_get.addr_lsw = 0;
	buf->voc_get.addr_msw = 0;
	buf->voc_get.mem_size = MAX_PARAM_SIZE;
	buf->voc_get.module_id = rtac_info->module_id;
	buf->voc_get.param_id = rtac_info->parameter_id;
}

void pack_audio_set_rtac_data(ACPH_CMD_RTC_SET_CAL_DATA_req *rtac_info, uint32_t in_buf_size, uint32_t out_buf_size)
{
	struct send_rtac_data	*buf = (struct send_rtac_data *)rtac_buf;
	int payload_size = in_buf_size - 3 * sizeof(uint32_t);

	buf->buf_size = out_buf_size;
	/* taking 3 params out of input adding 4 */
	buf->cmd_size = in_buf_size + sizeof(uint32_t);
	buf->port_id = rtac_info->pp_id;
	buf->aud_set.addr_lsw = 0;
	buf->aud_set.addr_msw = 0;
	buf->aud_set.mem_handle = 0;
	buf->aud_set.payload_size = payload_size;

	memcpy(&buf->aud_set.module_id, &rtac_info->module_id, payload_size);
}

void pack_afe_set_rtac_data(ACPH_SET_AFE_DATA_req *rtac_info, uint32_t in_buf_size, uint32_t out_buf_size)
{
	struct send_rtac_data	*buf = (struct send_rtac_data *)rtac_buf;
	int payload_size = in_buf_size - (sizeof(rtac_info->op_mode)
									  + sizeof(rtac_info->tx_afe_port_id)
									  + sizeof(rtac_info->rx_afe_port_id));

	buf->buf_size = out_buf_size;
	buf->cmd_size = in_buf_size + (sizeof(struct afe_set_param_payload)
								   - sizeof(struct ACPH_SET_AFE_DATA_req));

	if (rtac_info->op_mode != 2)
		buf->afe_set.port_id = rtac_info->tx_afe_port_id;
	else
		buf->afe_set.port_id = rtac_info->rx_afe_port_id;
	buf->port_id = buf->afe_set.port_id;

	buf->afe_set.payload_size = payload_size;
	buf->afe_set.addr_lsw = 0;
	buf->afe_set.addr_msw = 0;
	buf->afe_set.mem_handle = 0;
	memcpy(&buf->afe_set.module_id, &rtac_info->module_id, payload_size);
}

void pack_voice_set_rtac_data(ACPH_CMD_RTC_SET_CAL_DATA_req *rtac_info, uint32_t in_buf_size, uint32_t out_buf_size)
{
	struct send_rtac_data	*buf = (struct send_rtac_data *)rtac_buf;
	int payload_size = in_buf_size - 3 * sizeof(uint32_t);

	buf->buf_size = out_buf_size;
	/* taking 3 params out of input adding 4 */
	buf->cmd_size = in_buf_size + sizeof(uint32_t);
	buf->port_id = rtac_info->pp_id;
	buf->voc_set.mem_handle = 0;
	buf->voc_set.addr_lsw = 0;
	buf->voc_set.addr_msw = 0;
	buf->voc_set.mem_size = payload_size;

	memcpy(&buf->voc_set.module_id, &rtac_info->module_id, payload_size);
}

int32_t send_rtac(uint32_t opcode)
{
	int ret = 0;

	ret = ioctl(rtac_driver_handle, opcode, rtac_buf);
	if (ret <= 0) {
		LOGE("ACDB RTAC -> ERROR: send_rtac opcode 0x%x, port 0x%x errno: %d\n",
			opcode, rtac_buf[2], errno);
		ret = ACPH_FAILURE;
		goto done;
	}

	ret = ACPH_SUCCESS;
done:
	return ret;
}

int32_t send_get_rtac(uint32_t *rtac_data, uint32_t out_buf_size,
			uint32_t *bytes_filled, uint32_t opcode)
{
	int ret = 0;
	uint32_t i;

	ret = send_rtac(opcode);
	if (ret < 0) {
		goto done;
	}

	if (opcode == AUDIO_GET_RTAC_AFE_CAL) {
		struct afe_get_param_resp_payload * resp;
		resp = (struct afe_get_param_resp_payload *) &rtac_buf;
		*bytes_filled = resp->param_max_size + sizeof (struct afe_get_param_resp_payload);
	} else {
		/* param len is 3rd */
		/* module, param, & param len fields */
		*bytes_filled = rtac_buf[2] + 3*sizeof(uint32_t);
	}
	if (*bytes_filled > out_buf_size) {
		LOGE("ACDB RTAC -> ERROR: send_get_rtac bytes filled = %d greater than buf_size = %d\n",
			*bytes_filled, out_buf_size);
		*bytes_filled = 0;
		goto done;
	}

	memcpy(rtac_data, &rtac_buf, *bytes_filled);
done:
	return ret;
}
static bool is_afe_port_exist(uint32_t afe_port_id)
{
	uint32_t i;
	bool found = false;

	if (get_voice_info() == ACPH_SUCCESS) {
		for(i=0; i < rtac_voice_data.num_of_voice_combos; i++) {
			if ((afe_port_id == rtac_voice_data.voice[i].rx_afe_port) ||
				(afe_port_id == rtac_voice_data.voice[i].tx_afe_port)){
				found = true;
				break;
			}
		}
	}

	if (!found) {
		if (get_adm_info() == ACPH_SUCCESS) {
			for(i=0; i < rtac_adm_data.num_of_dev; i++) {
				if (afe_port_id == rtac_adm_data.device[i].afe_port) {
					found = true;
					break;
				}
			}
		}
	}

	return found;
}

static int32_t get_afe_rtac(struct ACPH_GET_AFE_DATA_req *rtac_info, uint32_t in_buf_size,
		uint32_t *rtac_data, uint32_t out_buf_size, uint32_t *bytes_filled)
{
	int32_t ret;

	/* Check if AFE port exit */
	if (is_afe_port_exist(rtac_info->afe_port_id)) {
		pack_afe_get_rtac_data(rtac_info, out_buf_size);
		ret = send_get_rtac(rtac_data, out_buf_size, bytes_filled, AUDIO_GET_RTAC_AFE_CAL);
	} else {
		LOGE("ACDB RTAC -> Error: set_afe_rtac, AFE port ID %d not found!\n", rtac_info->afe_port_id);
		ret = ACPH_FAILURE;
	}
	return ret;
}

int32_t get_rtac(uint32_t service_id, uint32_t *rtac_info, uint32_t in_buf_size,
		uint32_t *rtac_data, uint32_t out_buf_size, uint32_t *bytes_filled)
{
	int ret = 0;
	uint32_t i, j;

	if (out_buf_size < MIN_PARAM_SIZE) {
		LOGE("ACDB RTAC -> ERROR: get_rtac buffer size of %d is too small\n", out_buf_size);
		ret = ACPH_FAILURE;
		goto done;
	}

	switch (service_id) {
	case ADSP_CVP_SERVICE:
		for (i=0; i < rtac_voice_data.num_of_voice_combos; i++) {
			if (rtac_info[2]== rtac_voice_data.voice[i].cvp_handle) {
				pack_voice_get_rtac_data((ACPH_CMD_RTC_GET_CAL_DATA_req *)rtac_info, out_buf_size);
				ret = send_get_rtac(rtac_data, out_buf_size, bytes_filled,
							AUDIO_GET_RTAC_CVP_CAL);
				goto done;
			}
		}
		LOGE("ACDB RTAC -> Error: get_rtac, ADSP_CVP_SERVICE handle 0x%x not found!\n", rtac_info[2]);
		ret = ACPH_FAILURE;
		goto done;
	case ADSP_CVS_SERVICE:
		for (i=0; i < rtac_voice_data.num_of_voice_combos; i++) {
			if (rtac_info[2] == rtac_voice_data.voice[i].cvs_handle) {
				pack_voice_get_rtac_data((ACPH_CMD_RTC_GET_CAL_DATA_req *)rtac_info, out_buf_size);
				ret = send_get_rtac(rtac_data, out_buf_size, bytes_filled,
							AUDIO_GET_RTAC_CVS_CAL);
				goto done;
			}
		}
		LOGE("ACDB RTAC -> Error: get_rtac, ADSP_CVS_SERVICE handle 0x%x not found!\n", rtac_info[2]);
		ret = ACPH_FAILURE;
		goto done;
	case ADSP_ADM_SERVICE:
		for (i=0; i < rtac_adm_data.num_of_dev; i++) {
			if (rtac_info[2] == rtac_adm_data.device[i].copp) {
				pack_audio_get_rtac_data((ACPH_CMD_RTC_GET_CAL_DATA_req *)rtac_info, out_buf_size);
				ret = send_get_rtac(rtac_data, out_buf_size, bytes_filled,
							AUDIO_GET_RTAC_ADM_CAL);
				goto done;
			}
		}
		LOGE("ACDB RTAC -> Error: get_rtac, ADSP_ADM_SERVICE handle 0x%x not found!\n", rtac_info[2]);
		ret = ACPH_FAILURE;
		goto done;
	case ADSP_ASM_SERVICE:
		for (i=0; i < rtac_adm_data.num_of_dev; i++) {
			for (j=0; j < rtac_adm_data.num_of_dev; j++) {
				if (rtac_info[2] == rtac_adm_data.device[i].popp[j].popp) {
					pack_audio_get_rtac_data((ACPH_CMD_RTC_GET_CAL_DATA_req *)rtac_info, out_buf_size);
					ret = send_get_rtac(rtac_data, out_buf_size, bytes_filled,
								AUDIO_GET_RTAC_ASM_CAL);
					goto done;
				}
			}
		}
		LOGE("ACDB RTAC -> Error: get_rtac, ADSP_ASM_SERVICE handle 0x%x not found!\n", rtac_info[2]);
		ret = ACPH_FAILURE;
		goto done;
	default:
		LOGE("ACDB RTAC -> Error: get_rtac, unrecognized service %d\n", service_id);
		ret = ACPH_FAILURE;
		goto done;
	}

done:
	return ret;
}

static int32_t set_afe_rtac(struct ACPH_SET_AFE_DATA_req *rtac_info, uint32_t in_buf_size,
		uint32_t *rtac_data, uint32_t out_buf_size )
{
	uint32_t afe_port_id;
	int32_t ret;

	afe_port_id = (rtac_info->op_mode != 2) ? rtac_info->tx_afe_port_id :
											rtac_info->rx_afe_port_id;

	/* Check if AFE port exit */
	if (is_afe_port_exist(afe_port_id)) {
		pack_afe_set_rtac_data(rtac_info, in_buf_size, out_buf_size);
		ret = send_rtac(AUDIO_SET_RTAC_AFE_CAL);
	} else {
		LOGE("ACDB RTAC -> Error: set_afe_rtac, AFE port ID %d not found!\n", afe_port_id);
		ret = ACPH_FAILURE;
	}
	return ret;
}

int32_t set_rtac(uint32_t service_id, uint32_t *rtac_info, uint32_t in_buf_size,
		uint32_t *rtac_data, uint32_t out_buf_size )
{
	int ret = 0;
	uint32_t i, j;

	switch (service_id) {
	case ADSP_CVP_SERVICE:
		for (i=0; i < rtac_voice_data.num_of_voice_combos; i++) {
			if (rtac_info[2] == rtac_voice_data.voice[i].cvp_handle) {
				pack_voice_set_rtac_data((ACPH_CMD_RTC_SET_CAL_DATA_req *)rtac_info, in_buf_size, out_buf_size);
				ret = send_rtac(AUDIO_SET_RTAC_CVP_CAL);
				goto done;
			}
		}
		LOGE("ACDB RTAC -> Error: set_rtac, ADSP_CVP_SERVICE handle 0x%x not found!\n", rtac_info[2]);
		ret = ACPH_FAILURE;
		goto done;
	case ADSP_CVS_SERVICE:
		for (i=0; i < rtac_voice_data.num_of_voice_combos; i++) {
			if (rtac_info[2] == rtac_voice_data.voice[i].cvs_handle) {
				pack_voice_set_rtac_data((ACPH_CMD_RTC_SET_CAL_DATA_req *)rtac_info, in_buf_size, out_buf_size);
				ret = send_rtac(AUDIO_SET_RTAC_CVS_CAL);
				goto done;
			}
		}
		LOGE("ACDB RTAC -> Error: set_rtac, ADSP_CVS_SERVICE handle 0x%x not found!\n", rtac_info[2]);
		ret = ACPH_FAILURE;
		goto done;
	case ADSP_ADM_SERVICE:
		for (i=0; i < rtac_adm_data.num_of_dev; i++) {
			if (rtac_info[2] == rtac_adm_data.device[i].copp) {
				pack_audio_set_rtac_data((ACPH_CMD_RTC_SET_CAL_DATA_req *)rtac_info, in_buf_size, out_buf_size);
				ret = send_rtac(AUDIO_SET_RTAC_ADM_CAL);
				goto done;
			}
		}
		LOGE("ACDB RTAC -> Error: set_rtac, ADSP_ADM_SERVICE handle 0x%x not found!\n", rtac_info[2]);
		ret = ACPH_FAILURE;
		goto done;
	case ADSP_ASM_SERVICE:
		for (i=0; i < rtac_adm_data.num_of_dev; i++) {
			for (j=0; j < rtac_adm_data.num_of_dev; j++) {
				if (rtac_info[2] == rtac_adm_data.device[i].popp[j].popp) {
					pack_audio_set_rtac_data((ACPH_CMD_RTC_SET_CAL_DATA_req *)rtac_info, in_buf_size, out_buf_size);
					ret = send_rtac(AUDIO_SET_RTAC_ASM_CAL);
					goto done;
				}
			}
		}
		LOGE("ACDB RTAC -> Error: set_rtac, ADSP_ASM_SERVICE handle 0x%x not found!\n", rtac_info[2]);
		ret = ACPH_FAILURE;
		goto done;
	default:
		LOGE("ACDB RTAC -> Error: set_rtac, unrecognized service %d\n", service_id);
		ret = ACPH_FAILURE;
		goto done;
	}

done:
	return ret;
}

int32_t acdb_rtac_callback(uint16_t cmd, uint8_t *req_buf_ptr,
				uint32_t req_buf_len, uint8_t *resp_buf_ptr,
				uint32_t resp_buf_len, uint32_t *resp_buf_bytes_filled)
{
	int ret = 0;

	switch(cmd) {
	case ACPH_CMD_QUERY_DSP_RTC_VERSION:
		((ACPH_CMD_QUERY_DSP_RTC_VERSION_rsp *)resp_buf_ptr)->dsp_rtc_major_version = RTAC_MAJOR_VERSION;
		((ACPH_CMD_QUERY_DSP_RTC_VERSION_rsp *)resp_buf_ptr)->dsp_rtc_minor_version = RTAC_MINOR_VERSION;
		*resp_buf_bytes_filled = sizeof (ACPH_CMD_QUERY_DSP_RTC_VERSION_rsp);
		goto done;
	case ACPH_CMD_QUERY_AUD_TOPOLOGY_COPP_HANDLES:
		ret = get_aud_topology((uint32_t *)resp_buf_ptr,
					resp_buf_bytes_filled);
		goto done;
	case ACPH_CMD_QUERY_AUD_COPP_STREAM_HANDLES_V2:
		ret = get_aud_streams((ACPH_CMD_QUERY_AUD_COPP_STREAM_HANDLES_req *)req_buf_ptr,
					(uint32_t *)resp_buf_ptr,
					resp_buf_bytes_filled);
		goto done;
	case ACPH_CMD_QUERY_VOC_ALL_ACTIVE_STREAMS:
		ret = get_voc_streams((uint32_t *)resp_buf_ptr,
					resp_buf_bytes_filled);
		goto done;
	case ACPH_CMD_QUERY_VOC_VS_COPP_HANDLES:
		ret = get_voc_copp_handles((ACPH_CMD_QUERY_VOC_VS_COPP_HANDLES_req *)req_buf_ptr,
					(uint32_t *)resp_buf_ptr,
					resp_buf_bytes_filled);
		goto done;
	case ACPH_CMD_QUERY_VOC_VC_TOPOLOGIES:
		ret = get_voc_topology((ACPH_CMD_QUERY_VOC_VC_TOPOLOGIES_req *)req_buf_ptr,
					(uint32_t *)resp_buf_ptr,
					resp_buf_bytes_filled);
		goto done;
	case ACPH_CMD_RTC_GET_CAL_DATA:
		ret = get_rtac(((ACPH_CMD_RTC_GET_CAL_DATA_req *)req_buf_ptr)->service_id,
				(uint32_t *)req_buf_ptr,
				req_buf_len,
				(uint32_t *)resp_buf_ptr,
				resp_buf_len,
				resp_buf_bytes_filled);
		goto done;
	case ACPH_CMD_RTC_SET_CAL_DATA:
		ret = set_rtac(((ACPH_CMD_RTC_SET_CAL_DATA_req *)req_buf_ptr)->service_id,
				(uint32_t *)req_buf_ptr,
				req_buf_len,
				(uint32_t *)resp_buf_ptr,
				resp_buf_len);
		goto done;
	case ACPH_GET_AFE_DATA:
		ret = get_afe_rtac((ACPH_GET_AFE_DATA_req *)req_buf_ptr,
				req_buf_len,
				(uint32_t *)resp_buf_ptr,
				resp_buf_len,
				resp_buf_bytes_filled);
		goto done;
	case ACPH_SET_AFE_DATA:
		ret = set_afe_rtac((ACPH_SET_AFE_DATA_req *)req_buf_ptr,
				req_buf_len,
				(uint32_t *)resp_buf_ptr,
				resp_buf_len);
		goto done;
	case ACPH_CMD_QUERY_VOC_AFE_ACTIVE_PORT_IDS:
		ret = get_voc_afe_port_id((ACPH_CMD_QUERY_VOC_AFE_ACTIVE_PORT_IDS_req *)req_buf_ptr,
					(uint32_t *)resp_buf_ptr,
					resp_buf_bytes_filled);
		goto done;

	default:
		if ((cmd > ACPH_DSP_RTC_CMD_ID_END) || ((cmd < ACPH_DSP_RTC_CMD_ID_START)))
			LOGE("ACDB RTAC -> ERROR: ioctl not recognized: 0x%x\n", cmd);
		ret = ACPH_ERR_INVALID_COMMAND;
		goto done;
	}

done:
	return ret;
}

void acdb_rtac_init(void)
{
	int ret;

	ret = acph_register_command(ACPH_DSP_RTC_REG_SERVICEID, acdb_rtac_callback);
	if (ret < 0) {
		LOGE("ACDB RTAC -> acph register failed error = %d\n", ret);
		goto done;
	}

	rtac_driver_handle = open("/dev/msm_rtac", O_RDWR);
	if (rtac_driver_handle < 0) {
		LOGE("Cannot open /dev/msm_rtac errno: %d\n", errno);
		goto done;
	}
done:
	return;
}

void acdb_rtac_exit(void)
{
	int ret;

	ret = acph_deregister_command(ACPH_DSP_RTC_REG_SERVICEID);
	if (ret < 0)
		LOGE("ACDB RTAC -> ERROR: acdb_rtac_exit, acph_deregister_command failed err =%d\n", ret);

	close(rtac_driver_handle);
}
