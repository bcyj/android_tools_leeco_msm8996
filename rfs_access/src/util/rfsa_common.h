#ifndef __RFSA_COMMON_H__
#define __RFSA_COMMON_H__

/*
 * Copyright (c) 2012-2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#define LOG_TAG "rfs_access"

#include "cutils/log.h"
#include "common_log.h"
#include "rfsa_v01.h"

/***************************************************************************
* The definitions.                                                         *
****************************************************************************/

/**
* Return codes to the client/server layers
*/
#define RFSA_EOK			0x000
#define RFSA_EFAILED			0x001
#define RFSA_EEOS			0x002

/**
* Flags used for open ret
*/
#define RFSA_READ_FLAG_EOF		0x01

#define SIZE_TRANSPORT_PAYLOAD		50

/***************************************************************************
* The type definitions.                                                    *
****************************************************************************/

/**
*
*/
typedef struct
{
	/* Opcode of the command */
	int32_t			opcode;
	/* The payload associated with the command */
	void			*data_ptr;
	/* The payload size */
	int32_t			data_size;
	/* The structures returned by the command */
	union {
		rfsa_file_stat_req_msg_v01		file_stat_req;
		rfsa_file_create_req_msg_v01		file_create_req;
		rfsa_file_read_req_msg_v01		file_read_req;
		rfsa_get_buff_addr_req_msg_v01		get_buff_addr_req;
		rfsa_release_buff_addr_req_msg_v01	free_buff_addr_req;
		rfsa_iovec_file_read_req_msg_v01	iovec_read_req;
		rfsa_iovec_file_write_req_msg_v01	iovec_write_req;
	} rfsa_req;
	union {
		rfsa_file_stat_resp_msg_v01		file_stat_ret;
		rfsa_file_create_resp_msg_v01		file_create_ret;
		rfsa_file_read_resp_msg_v01		file_read_ret;
		rfsa_get_buff_addr_resp_msg_v01		get_buff_addr_ret;
		rfsa_release_buff_addr_resp_msg_v01	free_buff_addr_ret;
		rfsa_iovec_file_read_resp_msg_v01	iovec_read_ret;
		rfsa_iovec_file_write_resp_msg_v01	iovec_write_ret;
	} rfsa_ret;
	char			transport_payload[SIZE_TRANSPORT_PAYLOAD];
} rfsa_packet_t;

/**
* The callback between the client/server layer and and the one below
*/
typedef int32_t (*rfsa_callback) (rfsa_packet_t *packet);

#endif /* __RFSA_COMMON_H__ */
