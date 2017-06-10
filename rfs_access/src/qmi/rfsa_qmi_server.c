/*
 * Copyright (c) 2012-2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#include <stdio.h>
#include "rfsa_qmi_server.h"

#include "qmi_csi.h"
#include "qmi_csi_common.h"

/*****************************************************************************
* Defines                                                                   *
****************************************************************************/

#define RFSA_APR_SERVER_CMD_QUEUE_SIZE (10)

/** Operation code for the start remote FS tests. */
#define ADSP_CMD_START_REMOTEFS_TESTS    0x00011287

static char rfsa_qmi_server_my_thread_name[] = "RFSAQMISERVER";
static rfsa_thread_t rfsa_qmi_server_thread;
extern qmi_csi_xport_ops_type qcsi_ipc_router_ops;

/*****************************************************************************
* Definitions                                                               *
****************************************************************************/

#define RFSA_APR_SERVER_ON_PANIC( ret ) \
{ if ( ret ) { ERR_FATAL( "Error[%d]", ret, 0, 0 ); } }

struct rfsa_qmi_server_cinfo {
	uint32_t is_client_ready;
	qmi_client_handle clnt;
};

struct rfsa_qmi_server_svc {
	qmi_csi_service_handle service_handle;
};

struct rfsa_qmi_server_response {
	qmi_req_handle req_handle;
	int msg_id;
};

/*****************************************************************************
* Variables                                                                 *
****************************************************************************/

static struct rfsa_qmi_server_svc svc;

/*****************************************************************************
* Implementations                                                           *
****************************************************************************/

static qmi_csi_cb_error rfsa_qmi_server_cb(struct rfsa_qmi_server_cinfo *clnt_info,
						qmi_req_handle req_handle,
						unsigned int msg_id,
						void *req_c_struct,
						unsigned int req_c_struct_len,
						void *service_cookie)
{
	rfsa_packet_t rfsa_packet;
	qmi_csi_error resp_err;
	int32_t ret = RFSA_EOK;
	qmi_csi_cb_error qmi_rc = QMI_CSI_NO_ERR;
	struct rfsa_qmi_server_response *response;
	rfsa_server_work_item_t *item;

	(void)clnt_info;
	(void)service_cookie;

	LOGV("rfsa_qmi_server_cb started.\n");

	if (rfsa_server_check_received_data(msg_id, req_c_struct_len) == 0) {
		LOGE("Error: msg_id:%d data_len: %d.\n", msg_id, req_c_struct_len);
		return QMI_CSI_CB_INTERNAL_ERR;
	}

	ret = rfsa_server_get_free_packet(&item);

	if (ret == RFSA_EOK) {
		item->rfsa_packet.opcode = msg_id;

		memcpy(&(item->rfsa_packet.rfsa_req.file_stat_req), req_c_struct, req_c_struct_len);

		item->rfsa_packet.data_ptr = &(item->rfsa_packet.rfsa_req.file_stat_req);
		item->rfsa_packet.data_size = req_c_struct_len;

		response = (struct rfsa_qmi_server_response *)item->rfsa_packet.transport_payload;
		response->req_handle = req_handle;
		response->msg_id = msg_id;

		rfsa_server_add_to_queue(item);
	} else {
		LOGE("Error in rfsa_qmi_server_cb %d", ret);
		qmi_rc = QMI_CSI_CB_INTERNAL_ERR;
	}
	return qmi_rc;
}

int32_t rfsa_qmi_server_response(rfsa_server_work_item_t *item)
{
	struct rfsa_qmi_server_response *response;
	qmi_csi_error qmi_rc;

	response = (struct rfsa_qmi_server_response *)item->rfsa_packet.transport_payload;

	qmi_rc = qmi_csi_send_resp(response->req_handle, response->msg_id,
					item->rfsa_packet.data_ptr,
					item->rfsa_packet.data_size);
	if (qmi_rc != QMI_CSI_NO_ERR) {
		LOGE("qmi_csi_send_resp returned error: %d\n", qmi_rc);
		return RFSA_EFAILED;
	}

	return RFSA_EOK;
}

static qmi_csi_cb_error rfsa_qmi_server_connect_cb(qmi_client_handle client_handle,
							void *service_cookie,
							void **connection_handle)
{
	struct rfsa_qmi_server_cinfo *cinfo;

	(void)service_cookie;

	LOGV("rfsa_qmi_server_connect_cb started.\n");

	cinfo = malloc(sizeof(struct rfsa_qmi_server_cinfo));
	if (!cinfo) {
		return QMI_CSI_CB_NO_MEM;
	}

	cinfo->clnt = client_handle;
	cinfo->is_client_ready = 1;
	*connection_handle = cinfo;
	return QMI_CSI_CB_NO_ERR;
}

static void rfsa_qmi_server_disconnect_cb(void *connection_handle, void *service_cookie)
{
	(void)service_cookie;

	LOGV("rfsa_qmi_server_disconnect_cb started.\n");

	if (connection_handle) {
		free(connection_handle);
	}
}

static qmi_csi_cb_error rfsa_qmi_server_handle_req_cb(void *connection_handle,
							qmi_req_handle req_handle,
							unsigned int msg_id,
							void *req_c_struct,
							unsigned int req_c_struct_len,
							void *service_cookie)
{
	qmi_csi_cb_error qmi_rc = QMI_CSI_CB_INTERNAL_ERR;
	struct rfsa_qmi_server_cinfo *cinfo = (struct rfsa_qmi_server_cinfo*)connection_handle;
	struct rfsa_qmi_server_svc *svc = (struct rfsa_qmi_server_svc*) service_cookie;

	LOGV("rfsa_qmi_server_handle_req_cb started.\n");

	if (!cinfo || !cinfo->is_client_ready) {
		LOGE("Invalid client\n");
		return qmi_rc;
	}

	qmi_rc = rfsa_qmi_server_cb(cinfo, req_handle, msg_id, req_c_struct, req_c_struct_len, service_cookie);

	return qmi_rc;
}

int32_t rfsa_qmi_server_init()
{
	qmi_csi_os_params os_params, os_params_in;
	fd_set fds;

	qmi_csi_xport_start(&qcsi_ipc_router_ops, NULL);

	qmi_idl_service_object_type rmt_storage_service_object = rfsa_get_service_object_v01();
	qmi_csi_error qmi_rc = QMI_CSI_INTERNAL_ERR;

	qmi_rc = qmi_csi_register(rmt_storage_service_object, rfsa_qmi_server_connect_cb,
				rfsa_qmi_server_disconnect_cb, rfsa_qmi_server_handle_req_cb,
				&svc, &os_params, &svc.service_handle);

	if(qmi_rc != QMI_NO_ERR) {
		LOGE("Error in qmi_csi_register, qmi_rc=%d\n", qmi_rc);
		return RFSA_EFAILED;
	}

	while (1) {
		fds = os_params.fds;
		select(os_params.max_fd+1, &fds, NULL, NULL, NULL);
		os_params_in.fds = fds;
		qmi_csi_handle_event(svc.service_handle, &os_params_in);
	}

	qmi_csi_unregister(svc.service_handle);

	return RFSA_EOK;
}

/* TBD */
int32_t rfsa_qmi_server_deinit()
{
	return RFSA_EOK;
}
