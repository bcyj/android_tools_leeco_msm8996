#ifndef __RFSA_SERVER_H__
#define __RFSA_SERVER_H__

/*
 * Copyright (c) 2012 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#include "rfsa_common.h"
#include "rfsa_event.h"
#include "rfsa_list.h"
#include "rfsa_lock.h"
#include "rfsa_thread.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct rfsa_server_work_item_t {
	rfsa_list_node_t		link;
	rfsa_packet_t		rfsa_packet;
} rfsa_server_work_item_t;

/**
* Initialize the server
*/
int32_t rfsa_server_init(void);
int32_t rfsa_server_deinit(void);

void rfsa_server_add_to_queue(rfsa_server_work_item_t *item);
int32_t rfsa_server_get_free_packet(rfsa_server_work_item_t **item);
int rfsa_server_check_received_data(int msg_id, int req_c_struct_len);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif /* __RFSA_SERVER_H__ */
