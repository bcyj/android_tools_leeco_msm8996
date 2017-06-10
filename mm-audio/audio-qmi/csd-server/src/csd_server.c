/* Copyright (c) 2012-2014 Qualcomm Technologies Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#include <stdbool.h>
#include <csd_server.h>
#include "csd_alsa.h"

extern pthread_mutex_t cond_mutex;
extern pthread_cond_t  condition;
extern struct csd_common_data csd_common_data;

static qmi_csi_cb_error csd_server_connect_cb(qmi_client_handle client_handle,
                                              void *service_cookie,
                                              void **connection_handle)
{
    int i = 0;
    qmi_csi_cb_error rc = QMI_CSI_CB_NO_ERR;
    LOGD("qmi_audio_connect_cb() IN\n");

    if (get_new_client_handle_from_pool(client_handle,
                                        connection_handle) != 0) {
        LOGE("Failed to get free client handle, Max no of clients reached\n");
        rc = QMI_CSI_CB_CONN_REFUSED;
    }

    LOGD("qmi_audio_connect_cb() OUT rc=%d\n", rc);
    return rc;
}

static void csd_server_disconnect_cb(void *connection_handle,
                                     void *service_cookie)
{
    struct client_data *clnt_data = (struct client_data *)connection_handle;
    LOGD("qmi_audio_disconnect_cb called on client 0x%08X", clnt_data);
    csd_alsa_cleanup_client(clnt_data);
    memset(clnt_data, 0, sizeof(struct client_data));
}

static qmi_csi_cb_error csd_server_init(struct client_data *clnt_data,
                                        qmi_req_handle req_handle,
                                        int32_t msg_id,
                                        void *req_c_struct)
{
    qmi_csi_cb_error qmi_rc = QMI_CSI_CB_NO_ERR;
    qmi_csi_error resp_err;
    qmi_csd_init_resp_msg_v01 resp = {0};
    uint32_t  rc = 0;

    if (qmi_csd_init  != QMI_CSD_INIT_DONE) {
        rc = csd_alsa_init();
    } else {
        LOGD("csd_server: CSD already initialized\n");
    }
EXIT:
    LOGD("csd_server:csd_init_req_handler rc=%d\n", rc);

    if (rc != 0) {
        resp.qmi_csd_status_code_valid = true;
        resp.qmi_csd_status_code =  QMI_CSD_EFAILED_V01;
        resp.resp.result = QMI_RESULT_FAILURE_V01;
        resp.resp.error = QMI_ERR_GENERAL_V01;
        qmi_csd_init  = QMI_CSD_INIT_FAILED; /* CSD init failed  */
    } else {
        qmi_csd_init  = QMI_CSD_INIT_DONE;
    }
    resp_err = qmi_csi_send_resp(req_handle, msg_id, &resp, sizeof(resp));

    if (resp_err != QMI_CSI_NO_ERR) {
        LOGD("csd_server:csd_init_req_handler => qmi_csi_send_resp returned\n"
             "error=0x%x\n", resp_err);
        rc = QMI_CSI_CB_INTERNAL_ERR;
    } else {
        rc = QMI_CSI_CB_NO_ERR;
    }

    LOGD("csd_server:csd_init_req_handler => msg_id=0x%x OUT rc=%d\n",
         msg_id, rc);
    return rc;
}

static qmi_csi_cb_error csd_server_deinit(struct client_data *clnt_data,
                                          qmi_req_handle req_handle,
                                          int32_t msg_id,
                                          void *req_c_struct)
{
    qmi_csi_cb_error qmi_rc = QMI_CSI_CB_NO_ERR;
    qmi_csi_error resp_err;
    qmi_csd_deinit_resp_msg_v01 resp;
    uint32_t  rc;

    rc = csd_alsa_deinit();
    if (rc != 0) {
        qmi_csd_init  = QMI_CSD_DINIT_FAILED;
        LOGE("csd_alsa_deinit() failed rc=%d\n",rc);
    }

    qmi_csd_init  = QMI_CSD_DINIT_DONE;

EXIT:
    LOGD("csd_deinit_req_handler() error=0x%x\n", rc);
    memset(&resp, 0, sizeof(resp));

    if (rc != 0) {
        resp.qmi_csd_status_code_valid = true;
        resp.qmi_csd_status_code = QMI_CSD_EFAILED_V01;
        resp.resp.result = QMI_RESULT_FAILURE_V01;
        resp.resp.error = QMI_ERR_GENERAL_V01;
        qmi_csd_init  = QMI_CSD_DINIT_FAILED;  /* CSD De Init failed  */
    } else {
        qmi_csd_init  = QMI_CSD_DINIT_DONE;
    }

    resp_err = qmi_csi_send_resp(req_handle,
                                 msg_id,
                                 &resp,
                                 sizeof(qmi_csd_deinit_resp_msg_v01));

    if (resp_err != QMI_CSI_NO_ERR) {
        LOGD("csd_server:csd_deinit_req_handler => qmi_csi_send_resp returned\n"
             "error=0x%x\n", resp_err);
        rc = QMI_CSI_CB_INTERNAL_ERR;
    }

    return rc;
}

static qmi_csi_cb_error csd_server_close_handle(struct client_data *clnt_data,
                                                qmi_req_handle req_handle,
                                                int32_t msg_id,
                                                void *req_c_struct)
{
    qmi_csi_error resp_err;
    qmi_csd_close_resp_msg_v01 resp;
    uint32_t handle;
    int rc = 0;

    handle = ((qmi_csd_close_req_msg_v01*)(req_c_struct))->handle;
    LOGD("csd_server:csd_close_req_handler CSD_CLOSE handle =0x%08X\n", handle);

    rc = csd_alsa_close_handle(clnt_data, handle);
    if (rc != 0) {
        LOGE("Failed to close handle=0x%08X rc=%d\n",handle, rc);
    }

EXIT:
    LOGD("csd_server:csd_close_req_handler CSD_CLOSE rc =0x%x\n", rc);
    memset(&resp, 0, sizeof(qmi_csd_close_resp_msg_v01));

    if (rc != 0) {
        resp.resp.result = QMI_RESULT_FAILURE_V01;
        resp.resp.error = QMI_ERR_GENERAL_V01;
        resp.qmi_csd_status_code_valid = true;
        resp.qmi_csd_status_code = QMI_CSD_EFAILED_V01;
        resp.handle_valid = false;
    } else {
        resp.qmi_csd_status_code_valid = false;
        resp.handle_valid = true;
        resp.handle = handle;
    }

    resp_err = qmi_csi_send_resp(req_handle, msg_id, &resp, sizeof(resp));
    if (resp_err != QMI_CSI_NO_ERR) {
        LOGD("csd_close_req_handler qmi_csi_send_resp error=0x%x\n", resp_err);
        rc = QMI_CSI_CB_INTERNAL_ERR;
    }

    return rc;
}

qmi_csi_cb_error csd_server_ioctl_dev_cmd_enable(struct client_data *clnt_data,
                                                 qmi_req_handle req_handle,
                                                 int32_t msg_id,
                                                 void *req_c_struct)
{
    uint32_t j;
    qmi_csd_dev_cmd_resp_msg resp_msg;
    uint32_t respmsg_size;
    uint32_t rc = 0;
    qmi_csi_error resp_err;
    struct device_data *device_data = NULL;
    struct csd_dev_entry *dev_entries = NULL;
    uint32_t device_handle = 0;
    int32_t no_of_devices = 0;
    int32_t curr_no_of_devices = 0;
    int32_t new_no_of_devices = 0;
    qmi_csd_dev_cmd_req_msg *qmi_csd_dev_req =
                                        (qmi_csd_dev_cmd_req_msg *)req_c_struct;
    qmi_csd_dev_enable_v01 *req_dev_en = (qmi_csd_dev_enable_v01 *)
                        (&(qmi_csd_dev_req->u.qmi_csd_enable_dev_req.qmi_csd_dev_enable_cmd_payload));

    LOGD("csd_server_ioctl_dev_cmd_enable() IN\n");

    device_handle = qmi_csd_dev_req->u.qmi_csd_enable_dev_req.handle;
    if (!is_valid_device_handle(clnt_data, device_handle)) {
        rc = 1;
        goto EXIT;
    }

    device_data =  (struct device_data *)device_handle;
    no_of_devices = req_dev_en->devs_len;

    if (no_of_devices != MAX_NO_ACTIVE_DEVICES) {
        rc = 1;
        LOGE("Cannot set devices, parameter list of devices=%d doesnt match"
             "required number=%d\n", no_of_devices, MAX_NO_ACTIVE_DEVICES);
        goto EXIT;
    }

    for (j = 0; j < no_of_devices; j++) {
        if (device_data->devices[j].dev_id != 0) {
            rc = 1;
            LOGE("Disable old devices before enabling new ones\n");
            goto EXIT;
        }
        device_data->devices[j].dev_id = req_dev_en->devs[j].dev_id;
        if (req_dev_en->devs[j].dev_attrib.sample_rate == NB_SAMPLE_RATE ||
           req_dev_en->devs[j].dev_attrib.sample_rate == WB_SAMPLE_RATE) {
            device_data->devices[j].dev_attrib.sample_rate =
                                     req_dev_en->devs[j].dev_attrib.sample_rate;
        } else {
            device_data->devices[j].dev_attrib.sample_rate = NB_SAMPLE_RATE;
        }
        device_data->devices[j].dev_attrib.bits_per_sample =
                                 req_dev_en->devs[j].dev_attrib.bits_per_sample;
        LOGD("dev_id[%08X] sr[%08X] bps[%08X]\n",
             device_data->devices[j].dev_id,
             device_data->devices[j].dev_attrib.sample_rate,
             device_data->devices[j].dev_attrib.bits_per_sample);
    }

    rc = csd_alsa_enable_device(clnt_data, device_data->devices[0].dev_id,
                                device_data->devices[1].dev_id);
    if (rc != 0)
        LOGE("csd_alsa_enable_device() failed rc=%d\n", rc);

EXIT:
    LOGD(" CSD_DEV_CMD_ENABLE rc =0x%x\n", rc);
    memset(&resp_msg, 0, sizeof(qmi_csd_dev_cmd_resp_msg));
    if (rc != 0) {
        resp_msg.u.qmi_csd_enable_dev_resp.resp.result = QMI_RESULT_FAILURE_V01;
        resp_msg.u.qmi_csd_enable_dev_resp.resp.error = QMI_ERR_GENERAL_V01;
        resp_msg.u.qmi_csd_enable_dev_resp.qmi_csd_status_code_valid = true;
        resp_msg.u.qmi_csd_enable_dev_resp.qmi_csd_status_code =
                                       (qmi_csd_status_v01) QMI_CSD_EFAILED_V01;
    } else {
        LOGD("CSD_DEV_CMD_ENABLE SUCCESS => rc =0x%x\n", rc);
        resp_msg.u.qmi_csd_enable_dev_resp.qmi_csd_status_code_valid = false;
    }

    respmsg_size = sizeof(resp_msg.u.qmi_csd_enable_dev_resp);
    resp_err = qmi_csi_send_resp(req_handle, msg_id, &resp_msg, respmsg_size);
    if (resp_err != QMI_CSI_NO_ERR) {
        LOGE("Error qmi_csd_ioctl_dev_cmd_req_handler resp_err=%d\n", resp_err);
        rc = QMI_CSI_CB_INTERNAL_ERR;
    } else {
        rc = QMI_CSI_CB_NO_ERR;
    }

    LOGD("csd_server_ioctl_dev_cmd_enable() OUT rc=%d\n", rc);
    return rc;
}

qmi_csi_cb_error csd_server_open_device_control(struct client_data *clnt_data,
                                                qmi_req_handle req_handle,
                                                int32_t msg_id,
                                                void *req_c_struct)
{
    qmi_csi_error resp_err;
    uint32_t dev_handle;
    qmi_csd_open_device_control_resp_msg_v01 resp;
    int rc = 0;

    LOGD("csd_server:csd_open_device_control_req_handler IN\n");
    rc = csd_create_device_handle(clnt_data, &dev_handle);
    if (rc != 0)
        LOGE("Failed csd_create_device_handle handle rc=%d\n",rc);

EXIT:
    memset(&resp, 0, sizeof(qmi_csd_open_device_control_resp_msg_v01));
    LOGD("csd_create_device_handle() rc:%d dev_handle:%08X\n", rc, dev_handle);

    if (rc != 0) {
        LOGD("csd_server:csd_open_device_control_req_handler Error\n");
        resp.resp.result = QMI_RESULT_FAILURE_V01;
        resp.resp.error = QMI_ERR_GENERAL_V01;
        resp.open_status_valid = true;
        resp.open_status = (qmi_csd_status_v01) (QMI_CSD_EFAILED_V01);
        resp.qmi_csd_device_handle_valid = false;
    } else {
        /* open succeed  */
        resp.open_status_valid = false;
        resp.qmi_csd_device_handle_valid = true;
        resp.qmi_csd_device_handle = dev_handle;
    }

    resp_err = qmi_csi_send_resp(req_handle, msg_id, &resp, sizeof(resp));
    if (resp_err != QMI_CSI_NO_ERR) {
        LOGE("csd_server:csd_open_device_control qmi_csi_send_resp error=%d\n",
             resp_err);
        rc = QMI_CSI_CB_INTERNAL_ERR;
    } else {
        rc = QMI_CSI_CB_NO_ERR;
    }

    return rc;
}

qmi_csi_cb_error csd_server_open_passive_control_voice_stream(struct client_data *clnt_data,
                                                              qmi_req_handle req_handle,
                                                              int32_t msg_id,
                                                              void *req_c_struct)
{
    qmi_csi_error resp_err;
    qmi_csd_open_passive_control_voice_stream_req_msg_v01 *req_ptr;
    qmi_csd_open_passive_control_voice_stream_resp_msg_v01 resp;
    uint32_t size, vs_handle;
    int32_t rc = 0;

    LOGD("csd_server:csd_server_open_passive_control_voice_stream IN\n");
    req_ptr = (qmi_csd_open_passive_control_voice_stream_req_msg_v01 *)(req_c_struct);

    rc = csd_create_voice_stream_handle(clnt_data, req_ptr->session_name, &vs_handle);
    if (rc != 0)
        LOGE("FAILED csd_create_voice_stream_handle() rc=%d\n", rc);

EXIT:
    memset(&resp, 0,
           sizeof(qmi_csd_open_passive_control_voice_stream_resp_msg_v01));
    LOGD(" CSD_OPEN_VOICE_STREAM rc=%d, vs_handle=0x%x\n", rc, vs_handle);
    if (rc != 0) {
        resp.resp.result = QMI_RESULT_FAILURE_V01;
        resp.resp.error = QMI_ERR_GENERAL_V01;
        resp.open_status_valid = true;
        resp.open_status = QMI_CSD_EFAILED_V01;
        resp.qmi_csd_vs_passive_control_handle_valid = false;
    } else {
        resp.open_status_valid = false;
        resp.qmi_csd_vs_passive_control_handle_valid = true;
        resp.qmi_csd_vs_passive_control_handle = vs_handle;
    }

    resp_err = qmi_csi_send_resp(req_handle, msg_id, &resp, sizeof(resp));
    if (resp_err != QMI_CSI_NO_ERR) {
        LOGD("Failed csd_server:qmi_csi_send_resp error=%d\n", resp_err);
        rc = QMI_CSI_CB_INTERNAL_ERR;
    } else {
        rc = QMI_CSI_CB_NO_ERR;
    }

    return rc;
}

qmi_csi_cb_error csd_server_open_voice_context(struct client_data *clnt_data,
                                               qmi_req_handle req_handle,
                                               int32_t msg_id,
                                               void *req_c_struct)
{
    qmi_csi_error resp_err;
    qmi_csd_vc_open_full_control_type_v01 *req_ptr;
    qmi_csd_open_voice_context_resp_msg_v01 resp;
    uint32_t size, vc_handle;
    struct voice_context_type *voice_context = NULL;
    int32_t rc = 0;

    LOGD("csd_server_open_voice_context IN\n", msg_id);
    req_ptr = &(((qmi_csd_open_voice_context_req_msg_v01 *)(req_c_struct))->qmi_csd_vc_open_payload);

    rc = csd_create_voice_context_handle(clnt_data, req_ptr->session_name,
                                         &vc_handle);
    if (rc != 0) {
        LOGE("Failed csd_create_voice_context_handle() rc=%d\n", rc);
        goto EXIT;
    }

    voice_context = (struct voice_context_type *)vc_handle;
    voice_context->direction = (uint16_t)(req_ptr->direction);
    voice_context->network_id = (uint32_t)(req_ptr->network_id);

EXIT:
    LOGD(" rc=%d, vc_handle=0x%x\n", rc, vc_handle);
    memset(&resp, 0, sizeof(qmi_csd_open_voice_context_resp_msg_v01));

    if (rc != 0) {
        // error case, invalid handle
        LOGD("CSD_OPEN_VOICE_CONTEXT FAILED => rc =0x%x\n", rc);
        resp.resp.result = QMI_RESULT_FAILURE_V01;
        resp.resp.error = QMI_ERR_GENERAL_V01;
        resp.open_status_valid = true;
        resp.open_status = QMI_CSD_EFAILED_V01;
        resp.qmi_csd_vc_handle_valid = false;
    } else {
        LOGD("CSD_OPEN_VOICE_CONTEXT SUCCESS => rc =0x%x\n", rc);
        resp.open_status_valid = false;
        resp.qmi_csd_vc_handle_valid = true;
        resp.qmi_csd_vc_handle = vc_handle;
    }

    resp_err = qmi_csi_send_resp(req_handle, msg_id, &resp, sizeof(resp));
    if (resp_err != QMI_CSI_NO_ERR) {
        LOGD("csd_open_voice_context send_resp error=%d\n", resp_err);
        rc = QMI_CSI_CB_INTERNAL_ERR;
    } else {
        rc = QMI_CSI_CB_NO_ERR;
    }

    return rc;
}

qmi_csi_cb_error csd_server_open_voice_manage(struct client_data *clnt_data,
                                              qmi_req_handle req_handle,
                                              int32_t msg_id,
                                              void *req_c_struct)
{
    qmi_csi_error resp_err;
    qmi_csd_vm_open_type_v01 *req_ptr;
    qmi_csd_open_voice_manager_resp_msg_v01 resp;
    uint32_t vm_handle;
    int32_t rc = 0;

    LOGD("csd_server_open_voice_manage => msg_id=0x%x\n", msg_id);

    req_ptr = &(((qmi_csd_open_voice_manager_req_msg_v01 *)(req_c_struct))->qmi_csd_vm_open_payload);

    rc = csd_create_voice_manager_handle(clnt_data, req_ptr->session_name,
                                         &vm_handle);
    if (rc != 0) {
        LOGE("FAILED csd_create_voice_manager_handle() rc=%d\n");
        goto EXIT;
    }

EXIT:
    LOGD("csd_server_open_voice_manage() rc=%d, vm_handle=%d\n", rc, vm_handle);
    memset(&resp, 0, sizeof(qmi_csd_open_voice_manager_resp_msg_v01));
    if (rc != 0) {
        LOGD("CSD_OPEN_VOICE_MANAGER FAILED => rc =0x%x\n", rc);
        resp.resp.result = QMI_RESULT_FAILURE_V01;
        resp.resp.error = QMI_ERR_GENERAL_V01;
        resp.open_status_valid = true;
        resp.open_status = QMI_CSD_EFAILED_V01;
        resp.qmi_csd_vm_handle_valid = false;
    } else {
        // open succeed
        LOGD("CSD_OPEN_VOICE_MANAGER SUCCESS => rc =0x%x\n", rc);
        resp.open_status_valid = false;
        resp.qmi_csd_vm_handle_valid = true;
        resp.qmi_csd_vm_handle = vm_handle;
    }

    resp_err = qmi_csi_send_resp(req_handle, msg_id, &resp, sizeof(resp));
    if (resp_err != QMI_CSI_NO_ERR) {
        LOGD("csd_server_open_voice_manage send_resp error=%d\n", resp_err);
        rc = QMI_CSI_CB_INTERNAL_ERR;
    } else {
        rc = QMI_CSI_CB_NO_ERR;
    }

    return rc;
}

qmi_csi_cb_error csd_server_ioctl_vc_set_device_config(struct client_data *clnt_data,
                                                       qmi_req_handle req_handle,
                                                       int32_t msg_id,
                                                       void *req_c_struct)
{
    int32_t rc = 0;
    qmi_csd_status_v01 qmi_rc = QMI_CSD_EOK_V01;
    qmi_csd_vc_cmd_req_msg *qmi_csd_vc_req =
                                         (qmi_csd_vc_cmd_req_msg *)req_c_struct;
    qmi_csd_vc_ioctl_set_device_config_v01 *req_info_set_dev_config = (qmi_csd_vc_ioctl_set_device_config_v01*)(&(qmi_csd_vc_req->u.qmi_csd_set_device_config_req.qmi_csd_vc_ioctl_set_device_config_payload));
    qmi_csd_vc_cmd_resp_msg resp_msg = {0};
    struct voice_context_type *voice_context_data  = NULL;
    uint32_t resp_msg_size = 0;
    qmi_csi_error resp_err;
    uint32_t vc_handle = 0;

    LOGD("csd_server_ioctl_vc_set_device_config() IN\n");
    vc_handle = qmi_csd_vc_req->u.qmi_csd_set_device_config_req.handle;
    if (!is_valid_voice_context_handle(clnt_data, vc_handle)) {
        LOGE("Failed is_valid_voice_context_handle() vc_handle=%08x",
             vc_handle);
        rc = 1;
        goto EXIT;
    }

    if (qmi_csd_vc_req->u.qmi_csd_set_device_config_req.ec_ref_dev_num_valid) {
        LOGE("CSD doesnt support setting EC\n");
        rc = 1;
        qmi_rc = QMI_CSD_EUNSUPPORTED_V01;
        goto EXIT;
    }

    pthread_mutex_lock(&cond_mutex);
    voice_context_data = (struct voice_context_type *)vc_handle;
    voice_context_data->rx_dev_id = req_info_set_dev_config->rx_dev_num;
    voice_context_data->tx_dev_id = req_info_set_dev_config->tx_dev_num;
    voice_context_data->rx_dev_sr = req_info_set_dev_config->rx_dev_sr;
    voice_context_data->tx_dev_sr = req_info_set_dev_config->tx_dev_sr;
    LOGD("csd_server:qmi_csd_ioctl_vc_cmd_req_handler cmd_token[%08X]\n"
         "rx_dev_num[%08X] tx_dev_num[%08X] rx_sr[%08X] tx_sr[%08X]\n",
         req_info_set_dev_config->cmd_token,
         req_info_set_dev_config->rx_dev_num,
         req_info_set_dev_config->tx_dev_num,
         req_info_set_dev_config->rx_dev_sr,
         req_info_set_dev_config->tx_dev_sr);

    memset(&csd_common_data.command, 0, sizeof(struct command_data));
    csd_common_data.command.token = qmi_csd_vc_req->u.qmi_csd_set_device_config_req.qmi_csd_vc_ioctl_set_device_config_payload.cmd_token;
    csd_common_data.command.msg_id = msg_id;
    csd_common_data.command.req_handle = req_handle;
    csd_common_data.thread_state = STATE_PROCESS_CMD;
    csd_common_data.command.command_handle = vc_handle;
    csd_common_data.command.command_client_data = clnt_data;
    pthread_cond_signal(&condition);
    pthread_mutex_unlock(&cond_mutex);

EXIT:
    LOGD("csd_server_ioctl_vc_set_device_config() rc=%d\n",rc);
    if (rc != 0) {
        LOGE("CSD_VC_IOCTL_SET_DEVICE_CONFIG FAILED rc [%08X]\n", rc);
        resp_msg.u.qmi_csd_set_device_config_resp.resp.result =
                                                         QMI_RESULT_FAILURE_V01;
        resp_msg.u.qmi_csd_set_device_config_resp.resp.error =
                                                            QMI_ERR_GENERAL_V01;
        resp_msg.u.qmi_csd_set_device_config_resp.qmi_csd_status_code_valid =
                                                                           true;
        resp_msg.u.qmi_csd_set_device_config_resp.qmi_csd_status_code = qmi_rc;
        resp_msg.u.qmi_csd_set_device_config_resp.handle_valid = false;
        resp_msg.u.qmi_csd_set_device_config_resp.cmd_token_valid = false;
        resp_msg_size = sizeof(resp_msg.u.qmi_csd_set_device_config_resp);
        resp_err = qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                                      resp_msg_size);
        if (resp_err != QMI_CSI_NO_ERR) {
            LOGD("csd_server:csd_server_ioctl_vc_set_device_config()\n"
                 "send_resp error=%d\n", resp_err);
            rc = QMI_CSI_CB_INTERNAL_ERR;
        }
    }

    return rc;
}

qmi_csi_cb_error csd_server_ioctl_vc_cmd_enable(struct client_data *clnt_data,
                                                qmi_req_handle req_handle,
                                                int32_t msg_id,
                                                void *req_c_struct)
{
    qmi_csi_cb_error qmi_rc = QMI_CSI_CB_NO_ERR;
    qmi_csd_vc_cmd_req_msg *qmi_csd_vc_req =
                                        (qmi_csd_vc_cmd_req_msg *)req_c_struct;
    qmi_csd_vc_cmd_resp_msg resp_msg = {0};
    uint32_t resp_msg_size = 0;
    qmi_csi_error resp_err;
    uint32_t vc_handle = 0;
    int rc = 0;

    LOGD("csd_server_ioctl_vc_cmd_enable() IN\n");

    vc_handle = qmi_csd_vc_req->u.qmi_csd_enable_req.handle;
    if (!is_valid_voice_context_handle(clnt_data, vc_handle)) {
        LOGE("Failed is_valid_voice_context_handle vc_handle=0x%08x\n",
             vc_handle);
        rc = 1;
        goto EXIT;
    }

    pthread_mutex_lock(&cond_mutex);
    memset(&csd_common_data.command, 0, sizeof(struct command_data));
    csd_common_data.command.token =
                                 qmi_csd_vc_req->u.qmi_csd_enable_req.cmd_token;
    csd_common_data.command.msg_id = msg_id;
    csd_common_data.command.req_handle = req_handle;
    csd_common_data.thread_state = STATE_PROCESS_CMD;
    csd_common_data.command.command_handle = vc_handle;
    csd_common_data.command.command_client_data = clnt_data;
    pthread_cond_signal(&condition);
    pthread_mutex_unlock(&cond_mutex);

EXIT:
    LOGD("csd_server_ioctl_vc_cmd_enable() rc=%d\n",rc);
    if (rc != 0) {
        LOGE("CSD_VC_IOCTL_ENABLE FAILED\n");
        resp_msg.u.qmi_csd_enable_resp.resp.result = QMI_RESULT_FAILURE_V01;
        resp_msg.u.qmi_csd_enable_resp.resp.error = QMI_ERR_GENERAL_V01;
        resp_msg.u.qmi_csd_enable_resp.qmi_csd_status_code_valid = true;
        resp_msg.u.qmi_csd_enable_resp.qmi_csd_status_code =
                                                           QMI_CSD_EFAILED_V01;
        resp_msg.u.qmi_csd_enable_resp.handle_valid = false;
        resp_msg.u.qmi_csd_enable_resp.cmd_token_valid = false;
        resp_msg_size = sizeof(resp_msg.u.qmi_csd_enable_resp);
        resp_err = qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                                     resp_msg_size);
        if (resp_err != QMI_CSI_NO_ERR) {
            LOGE("csd_server_ioctl_vc_cmd_enable() send_resp error=0x%x\n",
                 resp_err);
            rc = QMI_CSI_CB_INTERNAL_ERR;
        } else {
            rc = QMI_CSI_CB_NO_ERR;
        }
        return rc;
    }
    return QMI_CSI_CB_NO_ERR;
}

qmi_csi_cb_error csd_server_ioctl_vm_cmd_attach_context(struct client_data *clnt_data,
                                                        qmi_req_handle req_handle,
                                                        int32_t msg_id,
                                                        void *req_c_struct)
{
    qmi_csi_cb_error qmi_rc = QMI_CSI_CB_NO_ERR;
    qmi_csd_vm_cmd_req_msg *qmi_csd_vm_req =
                                        (qmi_csd_vm_cmd_req_msg *)req_c_struct;
    qmi_csd_vm_cmd_resp_msg resp_msg = {0};
    uint32_t resp_msg_size = 0;
    qmi_csi_error resp_err;
    uint32_t vm_handle = 0;
    int rc = 0;

    LOGD("csd_server_ioctl_vm_cmd_attach_context IN\n");

    vm_handle = qmi_csd_vm_req->u.qmi_csd_attach_context_req.handle;
    if (!is_valid_voice_manager_handle(clnt_data, vm_handle)) {
        LOGE("FAILED is_valid_voice_manager_handle vm_handle=0x%08x\n",
              vm_handle);
        rc = 1;
        goto EXIT;
    }

    pthread_mutex_lock(&cond_mutex);
    memset(&csd_common_data.command, 0, sizeof(struct command_data));
    csd_common_data.command.token = qmi_csd_vm_req->u.qmi_csd_attach_context_req.qmi_csd_vm_ioctl_attach_context_payload.cmd_token;
    csd_common_data.command.msg_id = msg_id;
    csd_common_data.command.req_handle = req_handle;
    csd_common_data.thread_state = STATE_PROCESS_CMD;
    csd_common_data.command.command_handle = vm_handle;
    csd_common_data.command.command_client_data = clnt_data;
    pthread_cond_signal(&condition);
    pthread_mutex_unlock(&cond_mutex);

EXIT:
    LOGD("csd_server_ioctl_vm_cmd_attach_context() rc=%d\n",rc);
    if (rc != 0) {
        LOGE("IOCTL_VM_CMD_ATTACH_CONTEXT_REQ_V01 FAILED rc=%d\n", rc);
        resp_msg.u.qmi_csd_attach_context_resp.resp.result =
                                                         QMI_RESULT_FAILURE_V01;
        resp_msg.u.qmi_csd_attach_context_resp.resp.error = QMI_ERR_GENERAL_V01;
        resp_msg.u.qmi_csd_attach_context_resp.qmi_csd_status_code_valid = true;
        resp_msg.u.qmi_csd_attach_context_resp.qmi_csd_status_code =
                                                            QMI_CSD_EFAILED_V01;
        resp_msg.u.qmi_csd_attach_context_resp.handle_valid = false;
        resp_msg.u.qmi_csd_attach_context_resp.cmd_token_valid = false;
        resp_msg_size = sizeof(resp_msg.u.qmi_csd_attach_context_resp);
        resp_err = qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                                     resp_msg_size);
        if (resp_err != QMI_CSI_NO_ERR) {
            LOGE("csd_server_ioctl_vm_cmd_attach_context()=%d\n", resp_err);
            rc = QMI_CSI_CB_INTERNAL_ERR;
        } else {
            rc = QMI_CSI_CB_NO_ERR;
        }
        return rc;
    }

    return QMI_CSI_CB_NO_ERR;
}

qmi_csi_cb_error csd_server_ioctl_vm_cmd_set_hdvoice_mode(struct client_data *clnt_data,
                                                          qmi_req_handle req_handle,
                                                          int32_t msg_id,
                                                          void *req_c_struct)
{
    qmi_csi_cb_error qmi_rc = QMI_CSI_CB_NO_ERR;
    qmi_csd_vm_cmd_req_msg *qmi_csd_vm_req =
                                         (qmi_csd_vm_cmd_req_msg *)req_c_struct;
    qmi_csd_vm_cmd_resp_msg resp_msg = {0};
    uint32_t resp_msg_size = 0;
    qmi_csi_error resp_err;
    uint32_t vm_handle = 0;
    uint32_t mode, token;
    int rc = 0;

    LOGD("%s: IN\n", __func__);

    vm_handle = qmi_csd_vm_req->u.qmi_csd_set_hdvoice_mode_req.handle;
    if (!is_valid_voice_manager_handle(clnt_data, vm_handle)) {
        LOGE("FAILED is_valid_voice_manager_handle() vm_handle=0x%08x\n",
             vm_handle);

        rc = 1;
        goto EXIT;
    }

    pthread_mutex_lock(&cond_mutex);
    mode = qmi_csd_vm_req->u.qmi_csd_set_hdvoice_mode_req.qmi_csd_vm_ioctl_set_hdvoice_mode_payload.mode;
    token = qmi_csd_vm_req->u.qmi_csd_set_hdvoice_mode_req.qmi_csd_vm_ioctl_set_hdvoice_mode_payload.cmd_token;
    ((struct voice_stream_manager *)vm_handle)->hd_voice = mode;
    csd_common_data.command.token = token;
    csd_common_data.command.msg_id = msg_id;
    csd_common_data.command.req_handle = req_handle;
    csd_common_data.thread_state = STATE_PROCESS_CMD;
    csd_common_data.command.command_handle = vm_handle;
    csd_common_data.command.command_client_data = clnt_data;
    pthread_cond_signal(&condition);
    pthread_mutex_unlock(&cond_mutex);

EXIT:
    LOGD("%s: rc=%d\n", __func__, rc);

    if (rc != 0) {
        LOGE(" CSD_VC_IOCTL_SET_HDVOICE FAILED rc=%d\n", rc);

        resp_msg.u.qmi_csd_set_hdvoice_mode_resp.resp.result =
                                                         QMI_RESULT_FAILURE_V01;
        resp_msg.u.qmi_csd_set_hdvoice_mode_resp.resp.error = QMI_ERR_GENERAL_V01;
        resp_msg.u.qmi_csd_set_hdvoice_mode_resp.qmi_csd_status_code_valid = true;
        resp_msg.u.qmi_csd_set_hdvoice_mode_resp.qmi_csd_status_code =
                                                            QMI_CSD_EFAILED_V01;
        resp_msg.u.qmi_csd_set_hdvoice_mode_resp.handle_valid = false;
        resp_msg.u.qmi_csd_set_hdvoice_mode_resp.cmd_token_valid = false;
        resp_msg_size = sizeof(resp_msg.u.qmi_csd_set_hdvoice_mode_resp);
        resp_err = qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                                                                resp_msg_size);
        if (resp_err != QMI_CSI_NO_ERR) {
            LOGD("csd_server_ioctl_vm_cmd_set_hdvoice_mode qmi_csi_send_resp\n"
                 "resp_err=%d\n", resp_err);

            rc = QMI_CSI_CB_INTERNAL_ERR;
        } else {
            rc = QMI_CSI_CB_NO_ERR;
        }
        return rc;
    }

    return QMI_CSI_CB_NO_ERR;
}

qmi_csi_cb_error csd_server_ioctl_vm_cmd_set_widevoice(struct client_data *clnt_data,
                                                       qmi_req_handle req_handle,
                                                       int32_t msg_id,
                                                       void *req_c_struct)
{
    qmi_csi_cb_error qmi_rc = QMI_CSI_CB_NO_ERR;
    qmi_csd_vm_cmd_req_msg *qmi_csd_vm_req =
                                         (qmi_csd_vm_cmd_req_msg *)req_c_struct;
    qmi_csd_vm_cmd_resp_msg resp_msg = {0};
    uint32_t resp_msg_size = 0;
    qmi_csi_error resp_err;
    uint32_t vm_handle = 0;
    int rc = 0;

    LOGD("csd_server_ioctl_vm_cmd_set_widevoice() IN\n");

    vm_handle = qmi_csd_vm_req->u.qmi_csd_set_widevoice_req.handle;
    if (!is_valid_voice_manager_handle(clnt_data, vm_handle)) {
        LOGE("FAILED is_valid_voice_manager_handle() vm_handle=0x%08x\n",
             vm_handle);
        rc = 1;
        goto EXIT;
    }

    pthread_mutex_lock(&cond_mutex);
    ((struct voice_stream_manager *)vm_handle)->wide_voice = qmi_csd_vm_req->u.qmi_csd_set_widevoice_req.qmi_csd_vm_ioctl_set_widevoice_payload.enable;
    csd_common_data.command.token = qmi_csd_vm_req->u.qmi_csd_set_widevoice_req.qmi_csd_vm_ioctl_set_widevoice_payload.cmd_token;
    csd_common_data.command.msg_id = msg_id;
    csd_common_data.command.req_handle = req_handle;
    csd_common_data.thread_state = STATE_PROCESS_CMD;
    csd_common_data.command.command_handle = vm_handle;
    csd_common_data.command.command_client_data = clnt_data;
    pthread_cond_signal(&condition);
    pthread_mutex_unlock(&cond_mutex);

EXIT:
    LOGD("csd_server_ioctl_vm_cmd_set_widevoice() rc=%d\n",rc);

    if (rc != 0) {
        LOGE(" CSD_VC_IOCTL_SET_WIDE_VOICE FAILED rc=%d\n", rc);
        resp_msg.u.qmi_csd_set_widevoice_resp.resp.result =
                                                         QMI_RESULT_FAILURE_V01;
        resp_msg.u.qmi_csd_set_widevoice_resp.resp.error = QMI_ERR_GENERAL_V01;
        resp_msg.u.qmi_csd_set_widevoice_resp.qmi_csd_status_code_valid = true;
        resp_msg.u.qmi_csd_set_widevoice_resp.qmi_csd_status_code =
                                                            QMI_CSD_EFAILED_V01;
        resp_msg.u.qmi_csd_set_widevoice_resp.handle_valid = false;
        resp_msg.u.qmi_csd_set_widevoice_resp.cmd_token_valid = false;
        resp_msg_size = sizeof(resp_msg.u.qmi_csd_set_widevoice_resp);
        resp_err = qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                                                                resp_msg_size);
        if (resp_err != QMI_CSI_NO_ERR) {
            LOGD("csd_server_ioctl_vm_cmd_set_widevoice qmi_csi_send_resp\n"
                 "resp_err=%d\n", resp_err);
            rc = QMI_CSI_CB_INTERNAL_ERR;
        } else {
            rc = QMI_CSI_CB_NO_ERR;
        }
        return rc;
    }

    return QMI_CSI_CB_NO_ERR;
}

qmi_csi_cb_error csd_server_ioctl_vm_cmd_set_tty_mode(struct client_data *clnt_data,
                                                      qmi_req_handle req_handle,
                                                      int32_t msg_id,
                                                      void *req_c_struct)
{
    qmi_csi_cb_error qmi_rc = QMI_CSI_CB_NO_ERR;
    qmi_csd_vm_cmd_req_msg *qmi_csd_vm_req =
                                         (qmi_csd_vm_cmd_req_msg *)req_c_struct;
    qmi_csd_vm_cmd_resp_msg resp_msg = {0};
    uint32_t resp_msg_size = 0;
    qmi_csi_error resp_err;
    uint32_t vm_handle = 0;
    int rc = 0;

    LOGD("csd_server_ioctl_vm_cmd_set_tty_mode()\n");

    vm_handle = qmi_csd_vm_req->u.qmi_csd_set_tty_mode_req.handle;
    if (!is_valid_voice_manager_handle(clnt_data, vm_handle)) {
        LOGE("FAILED is_valid_voice_manager_handle() vm_handle=0x%08x\n",
             vm_handle);
        rc = 1;
        goto EXIT;
    }

    pthread_mutex_lock(&cond_mutex);
    ((struct voice_stream_manager *)vm_handle)->tty_mode = qmi_csd_vm_req->u.qmi_csd_set_tty_mode_req.qmi_csd_vm_ioctl_set_tty_mode_payload.mode;
    csd_common_data.command.token = qmi_csd_vm_req->u.qmi_csd_set_tty_mode_req.qmi_csd_vm_ioctl_set_tty_mode_payload.cmd_token;
    csd_common_data.command.msg_id = msg_id;
    csd_common_data.command.req_handle = req_handle;
    csd_common_data.thread_state = STATE_PROCESS_CMD;
    csd_common_data.command.command_handle = vm_handle;
    csd_common_data.command.command_client_data = clnt_data;
    pthread_cond_signal(&condition);
    pthread_mutex_unlock(&cond_mutex);

EXIT:
    LOGD("csd_server_ioctl_vm_cmd_set_tty_mode() rc=%d\n",rc);
    if (rc != 0) {
        LOGE(" CSD_VM_IOCTL_SET_TTY FAILED rc [%08X]\n", rc);
        resp_msg.u.qmi_csd_set_tty_mode_resp.resp.result =
                                                         QMI_RESULT_FAILURE_V01;
        resp_msg.u.qmi_csd_set_tty_mode_resp.resp.error = QMI_ERR_GENERAL_V01;
        resp_msg.u.qmi_csd_set_tty_mode_resp.qmi_csd_status_code_valid = true;
        resp_msg.u.qmi_csd_set_tty_mode_resp.qmi_csd_status_code =
                                                            QMI_CSD_EFAILED_V01;
        resp_msg.u.qmi_csd_set_tty_mode_resp.handle_valid = false;
        resp_msg.u.qmi_csd_set_tty_mode_resp.cmd_token_valid = false;
        resp_msg_size = sizeof(resp_msg.u.qmi_csd_set_tty_mode_resp);
        resp_err = qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                                     resp_msg_size);
        if (resp_err != QMI_CSI_NO_ERR){
            LOGE("csd_server_ioctl_vm_cmd_set_tty_mode() send_resp error=%d\n",
                  resp_err);
            rc = QMI_CSI_CB_INTERNAL_ERR;
        } else {
            rc = QMI_CSI_CB_NO_ERR;
        }
        return rc;
    }

    return QMI_CSI_CB_NO_ERR;
}

qmi_csi_cb_error csd_server_ioctl_vs_cmd_set_ui_property(struct client_data *clnt_data,
                                                         qmi_req_handle req_handle,
                                                         int32_t msg_id,
                                                         void *req_c_struct)
{
    qmi_csd_vs_cmd_req_msg *qmi_csd_vs_req =
                                         (qmi_csd_vs_cmd_req_msg *)req_c_struct;
    qmi_csd_vs_ioctl_set_ui_property_v01 *req_info_set_ui_prop = (qmi_csd_vs_ioctl_set_ui_property_v01 *)(&(qmi_csd_vs_req->u.qmi_csd_set_ui_prop_req.qmi_csd_vs_ioctl_set_ui_property_payload));
    qmi_csd_vs_cmd_resp_msg resp_msg = {0};
    qmi_csi_error resp_err;
    struct voice_stream *voice_stream_data = NULL;
    uint32_t vs_handle = 0;
    uint32_t resp_msg_size = 0;
    int rc = 0;

    LOGD("csd_server_ioctl_vs_cmd_set_ui_property() IN\n");

    vs_handle = qmi_csd_vs_req->u.qmi_csd_get_ui_prop_req.handle;
    if (!is_valid_voice_stream_handle(clnt_data, vs_handle)) {
        LOGE("Failed is_valid_voice_stream_handle() vs_handle=0x%08x\n",
             vs_handle);
        rc = 1;
        goto EXIT;
    }
    voice_stream_data = (struct voice_stream *)vs_handle;

    pthread_mutex_lock(&cond_mutex);
    if (req_info_set_ui_prop->module_id == MODULE_ID_VOICE_MODULE_FENS) {
        voice_stream_data->ui_prop_mask = UI_PROP_FENS;
        voice_stream_data->fens =   req_info_set_ui_prop->param_data[0];
    } else {
        voice_stream_data->ui_prop_mask = UI_PROP_SLOW_TALK;
        voice_stream_data->slow_talk =  req_info_set_ui_prop->param_data[0];
    }

    memset(&csd_common_data.command, 0, sizeof(struct command_data));
    csd_common_data.command.token = req_info_set_ui_prop->cmd_token;
    csd_common_data.command.msg_id = msg_id;
    csd_common_data.command.req_handle = req_handle;
    csd_common_data.thread_state = STATE_PROCESS_CMD;
    csd_common_data.command.command_handle = vs_handle;
    csd_common_data.command.command_client_data = clnt_data;
    pthread_cond_signal(&condition);
    pthread_mutex_unlock(&cond_mutex);

EXIT:
    LOGD("csd_server_ioctl_vs_cmd_set_ui_property() rc=%d\n",rc);
    if (rc != 0) {
        LOGE("CSD_VS_IOCTL_SET_UI_PROPERTY FAILED rc=%d\n",rc);
        resp_msg.u.qmi_csd_set_ui_prop_resp.resp.result =
                                                         QMI_RESULT_FAILURE_V01;
        resp_msg.u.qmi_csd_set_ui_prop_resp.resp.error = QMI_ERR_GENERAL_V01;
        resp_msg.u.qmi_csd_set_ui_prop_resp.qmi_csd_status_code_valid = true;
        resp_msg.u.qmi_csd_set_ui_prop_resp.qmi_csd_status_code =
                                                            QMI_CSD_EFAILED_V01;
        resp_msg.u.qmi_csd_set_ui_prop_resp.handle_valid = false;
        resp_msg.u.qmi_csd_set_ui_prop_resp.cmd_token_valid = false;
        resp_msg_size = sizeof(resp_msg.u.qmi_csd_set_ui_prop_resp);
        resp_err = qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                                     resp_msg_size);
        if (resp_err != QMI_CSI_NO_ERR) {
            LOGE("csd_server_ioctl_vs_cmd_set_ui_property send_resp error=%d\n",
                 resp_err);
            rc = QMI_CSI_CB_INTERNAL_ERR;
        } else {
            rc = QMI_CSI_CB_NO_ERR;
        }
        return rc;
    }

    return QMI_CSI_CB_NO_ERR;
}

qmi_csi_cb_error csd_server_ioctl_vm_cmd_start_voice(struct client_data *clnt_data,
                                                     qmi_req_handle req_handle,
                                                     int32_t msg_id,
                                                     void *req_c_struct)
{
    qmi_csi_cb_error qmi_rc = QMI_CSI_CB_NO_ERR;
    qmi_csd_vm_cmd_req_msg *qmi_csd_vm_req =
                                         (qmi_csd_vm_cmd_req_msg *)req_c_struct;
    uint32_t vm_handle = 0;
    qmi_csd_vm_cmd_resp_msg resp_msg = {0};
    uint32_t resp_msg_size = 0;
    qmi_csi_error resp_err;
    int rc = 0;

    LOGD("csd_server_ioctl_vm_cmd_start_voice IN\n");
    vm_handle = qmi_csd_vm_req->u.qmi_csd_start_voice_req.handle;
    if (!is_valid_voice_manager_handle(clnt_data, vm_handle)) {
        LOGE("FAILED is_valid_voice_manager_handle() vm_handle=0x%08x\n",
             vm_handle);
        rc = 1;
        goto EXIT;
    }

    pthread_mutex_lock(&cond_mutex);
    memset(&csd_common_data.command, 0, sizeof(struct command_data));
    csd_common_data.command.token =
                           qmi_csd_vm_req->u.qmi_csd_start_voice_req.cmd_token;
    csd_common_data.command.msg_id = msg_id;
    csd_common_data.command.req_handle = req_handle;
    csd_common_data.thread_state = STATE_PROCESS_CMD;
    csd_common_data.command.command_handle = vm_handle;
    csd_common_data.command.command_client_data = clnt_data;
    pthread_cond_signal(&condition);
    pthread_mutex_unlock(&cond_mutex);

EXIT:
    LOGD("csd_server_ioctl_vm_cmd_start_voice() rc=%d\n",rc);
    if (rc != 0) {
        LOGE("CSD_VM_IOCTL_START_VOICE FAILED rc=%d\n",rc);
        resp_msg.u.qmi_csd_start_voice_resp.resp.result =
                                                         QMI_RESULT_FAILURE_V01;
        resp_msg.u.qmi_csd_start_voice_resp.resp.error = QMI_ERR_GENERAL_V01;
        resp_msg.u.qmi_csd_start_voice_resp.qmi_csd_status_code_valid = true;
        resp_msg.u.qmi_csd_start_voice_resp.qmi_csd_status_code =
                                                            QMI_CSD_EFAILED_V01;
        resp_msg.u.qmi_csd_start_voice_resp.handle_valid = false;
        resp_msg.u.qmi_csd_start_voice_resp.cmd_token_valid = false;
        resp_msg_size = sizeof(resp_msg.u.qmi_csd_start_voice_resp);
        resp_err = qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                                     resp_msg_size);
        if (resp_err != QMI_CSI_NO_ERR) {
            LOGD("csd_server_ioctl_vm_cmd_start_voice() send_resp error=%d\n",
                 resp_err);
            rc = QMI_CSI_CB_INTERNAL_ERR;
        } else {
            rc = QMI_CSI_CB_NO_ERR;
        }
        return rc;
    }

    return QMI_CSI_CB_NO_ERR;
}

qmi_csi_cb_error csd_server_ioctl_vm_cmd_stop_voice(struct client_data *clnt_data,
                                                    qmi_req_handle req_handle,
                                                    int32_t msg_id,
                                                    void *req_c_struct)
{
    qmi_csi_cb_error qmi_rc = QMI_CSI_CB_NO_ERR;
    qmi_csd_vm_cmd_req_msg *qmi_csd_vm_req =
                                         (qmi_csd_vm_cmd_req_msg *)req_c_struct;
    uint32_t vm_handle = 0;
    qmi_csd_vm_cmd_resp_msg resp_msg = {0};
    uint32_t resp_msg_size = 0;
    qmi_csi_error resp_err;
    int rc = 0;

    LOGD("csd_server_ioctl_vm_cmd_stop_voice() IN\n");
    vm_handle = qmi_csd_vm_req->u.qmi_csd_start_voice_req.handle;
    if (!is_valid_voice_manager_handle(clnt_data, vm_handle)) {
        LOGE("is_valid_voice_manager_handle() vm_handle=0x%08x\n",vm_handle);
        rc = 1;
        goto EXIT;
    }

    pthread_mutex_lock(&cond_mutex);
    memset(&csd_common_data.command, 0, sizeof(struct command_data));
    csd_common_data.command.token =
                            qmi_csd_vm_req->u.qmi_csd_start_voice_req.cmd_token;
    csd_common_data.command.msg_id = msg_id;
    csd_common_data.command.req_handle = req_handle;
    csd_common_data.thread_state = STATE_PROCESS_CMD;
    csd_common_data.command.command_handle = vm_handle;
    csd_common_data.command.command_client_data = clnt_data;
    pthread_cond_signal(&condition);
    pthread_mutex_unlock(&cond_mutex);

EXIT:
    LOGD("csd_server_ioctl_vm_cmd_stop_voice() rc=%d\n",rc);
    if (rc != 0) {
        LOGE("CSD_VM_IOCTL_STOP_VOICE rc=%d\n", rc);
        resp_msg.u.qmi_csd_stop_voice_resp.resp.result = QMI_RESULT_FAILURE_V01;
        resp_msg.u.qmi_csd_stop_voice_resp.resp.error = QMI_ERR_GENERAL_V01;
        resp_msg.u.qmi_csd_stop_voice_resp.qmi_csd_status_code_valid = true;
        resp_msg.u.qmi_csd_stop_voice_resp.qmi_csd_status_code =
                                                            QMI_CSD_EFAILED_V01;
        resp_msg.u.qmi_csd_stop_voice_resp.handle_valid = false;
        resp_msg.u.qmi_csd_stop_voice_resp.cmd_token_valid = false;
        resp_msg_size = sizeof(resp_msg.u.qmi_csd_stop_voice_resp);
        resp_err = qmi_csi_send_resp(req_handle, msg_id,
                                     &resp_msg, resp_msg_size);
        if (resp_err != QMI_CSI_NO_ERR) {
            LOGE("csd_server_ioctl_vm_cmd_stop_voice() send_resp error=0x%x\n",
                 resp_err);
            rc = QMI_CSI_CB_INTERNAL_ERR;
        } else {
            rc = QMI_CSI_CB_NO_ERR;
        }
        return rc;
    }

    return QMI_CSI_CB_NO_ERR;
}

qmi_csi_cb_error csd_server_ioctl_vm_cmd_standby_voice(struct client_data *clnt_data,
                                                       qmi_req_handle req_handle,
                                                       int32_t msg_id,
                                                       void *req_c_struct)
{
    qmi_csi_cb_error qmi_rc = QMI_CSI_CB_NO_ERR;
    qmi_csd_vm_cmd_req_msg *qmi_csd_vm_req =
                                        (qmi_csd_vm_cmd_req_msg *) req_c_struct;
    uint32_t vm_handle = 0;
    qmi_csd_vm_cmd_resp_msg resp_msg = {0};
    uint32_t resp_msg_size = 0;
    qmi_csi_error resp_err;
    int rc = 0;

    LOGD("csd_server_ioctl_vm_cmd_standby_voice() IN\n");
    vm_handle = qmi_csd_vm_req->u.qmi_csd_standby_voice_req.handle;
    if (!is_valid_voice_manager_handle(clnt_data, vm_handle)) {
        LOGE("is_valid_voice_manager_handle() FAILED vm_handle=0x%08x\n",
             vm_handle);
        rc = 1;
        goto EXIT;
    }

    pthread_mutex_lock(&cond_mutex);
    memset(&csd_common_data.command, 0, sizeof(struct command_data));
    csd_common_data.command.token =
                          qmi_csd_vm_req->u.qmi_csd_standby_voice_req.cmd_token;
    csd_common_data.command.msg_id = msg_id;
    csd_common_data.command.req_handle = req_handle;
    csd_common_data.thread_state = STATE_PROCESS_CMD;
    csd_common_data.command.command_handle = vm_handle;
    csd_common_data.command.command_client_data = clnt_data;
    pthread_cond_signal(&condition);
    pthread_mutex_unlock(&cond_mutex);

EXIT:
    LOGD(" csd_server_ioctl_vm_cmd_standby_voice() rc=%d\n",rc);
    if ((rc != 0)) {
        LOGE("CSD_VM_IOCTL_STANDBY_VOICE rc=%d\n",rc);
        resp_msg.u.qmi_csd_standby_voice_resp.resp.result =
                                                         QMI_RESULT_FAILURE_V01;
        resp_msg.u.qmi_csd_standby_voice_resp.resp.error = QMI_ERR_GENERAL_V01;
        resp_msg.u.qmi_csd_standby_voice_resp.qmi_csd_status_code_valid = true;
        resp_msg.u.qmi_csd_standby_voice_resp.qmi_csd_status_code =
                                                            QMI_CSD_EFAILED_V01;
        resp_msg.u.qmi_csd_standby_voice_resp.handle_valid = false;
        resp_msg.u.qmi_csd_standby_voice_resp.cmd_token_valid = false;
        resp_msg_size = sizeof(resp_msg.u.qmi_csd_standby_voice_resp);
        resp_err = qmi_csi_send_resp(req_handle, msg_id,
                                      &resp_msg, resp_msg_size);
        if (resp_err != QMI_CSI_NO_ERR) {
            LOGE("csd_server_ioctl_vm_cmd_standby_voice() send_resp err=0x%x\n",
                 resp_err);
            rc = QMI_CSI_CB_INTERNAL_ERR;
        } else {
            rc = QMI_CSI_CB_NO_ERR;
        }
        return rc;
    }

    return QMI_CSI_CB_NO_ERR;
}

qmi_csi_cb_error csd_server_ioctl_vc_cmd_set_rx_volume_index(struct client_data *clnt_data,
                                                             qmi_req_handle req_handle,
                                                             int32_t msg_id,
                                                             void *req_c_struct)
{
    qmi_csi_cb_error qmi_rc = QMI_CSI_CB_NO_ERR;
    qmi_csd_vc_cmd_req_msg *qmi_csd_vc_req =
                                         (qmi_csd_vc_cmd_req_msg *)req_c_struct;
    qmi_csd_vc_cmd_resp_msg resp_msg = {0};
    struct voice_context_type *voice_context_data  = NULL;
    uint32_t resp_msg_size = 0;
    qmi_csi_error resp_err;
    uint32_t vc_handle = 0;
    int rc = 0;

    LOGD("csd_server_ioctl_vc_cmd_set_rx_volume_index() IN\n");
    vc_handle = qmi_csd_vc_req->u.qmi_csd_set_device_config_req.handle;
    if (!is_valid_voice_context_handle(clnt_data, vc_handle)) {
        LOGE("is_valid_voice_context_handle FAILED vc_handle=0x%08x\n",
              vc_handle);
        rc = 1;
        goto EXIT;
    }

    pthread_mutex_lock(&cond_mutex);
    voice_context_data = (struct voice_context_type *)vc_handle;
    voice_context_data->volume = qmi_csd_vc_req->u.qmi_csd_set_rx_volume_req.qmi_csd_vc_ioctl_set_rx_volume_index_payload.vol_index;
    memset(&csd_common_data.command, 0, sizeof(struct command_data));
    csd_common_data.command.token = qmi_csd_vc_req->u.qmi_csd_set_rx_volume_req.qmi_csd_vc_ioctl_set_rx_volume_index_payload.cmd_token;
    csd_common_data.command.msg_id = msg_id;
    csd_common_data.command.req_handle = req_handle;
    csd_common_data.thread_state = STATE_PROCESS_CMD;
    csd_common_data.command.command_handle = vc_handle;
    csd_common_data.command.command_client_data = clnt_data;
    pthread_cond_signal(&condition);
    pthread_mutex_unlock(&cond_mutex);

EXIT:
    LOGD("csd_server_ioctl_vc_cmd_set_rx_volume_index() rc=%d\n",rc);
    if (rc != 0) {
        LOGE("CSD_VC_IOCTL_SET_RX_VOLUME FAILED rc=%d\n", rc);
        resp_msg.u.qmi_csd_set_rx_volume_resp.resp.result =
                                                         QMI_RESULT_FAILURE_V01;
        resp_msg.u.qmi_csd_set_rx_volume_resp.resp.error = QMI_ERR_GENERAL_V01;
        resp_msg.u.qmi_csd_set_rx_volume_resp.qmi_csd_status_code_valid = true;
        resp_msg.u.qmi_csd_set_rx_volume_resp.qmi_csd_status_code =
                                                            QMI_CSD_EFAILED_V01;
        resp_msg.u.qmi_csd_set_rx_volume_resp.handle_valid = false;
        resp_msg.u.qmi_csd_set_rx_volume_resp.cmd_token_valid = false;
        resp_msg_size = sizeof(resp_msg.u.qmi_csd_set_rx_volume_resp);
        resp_err = qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                                     resp_msg_size);
        if (resp_err != QMI_CSI_NO_ERR) {
            LOGE("csd_server_ioctl_vc_cmd_set_rx_volume_index()\n"
                 "send_resp error=%d\n", resp_err);
            rc = QMI_CSI_CB_INTERNAL_ERR;
        } else {
            rc = QMI_CSI_CB_NO_ERR;
        }
        return rc;
    }

    return QMI_CSI_CB_NO_ERR;
}

qmi_csi_cb_error csd_server_ioctl_vs_cmd_set_mute(struct client_data *clnt_data,
                                                  qmi_req_handle req_handle,
                                                  int32_t msg_id,
                                                  void *req_c_struct)
{
    qmi_csi_cb_error qmi_rc = QMI_CSI_CB_NO_ERR;
    qmi_csd_vs_cmd_req_msg *qmi_csd_vs_req =
                                         (qmi_csd_vs_cmd_req_msg *)req_c_struct;
    qmi_csd_vs_cmd_resp_msg resp_msg = {0};
    qmi_csi_error resp_err;
    struct voice_stream *voice_stream_data = NULL;
    uint32_t vs_handle = 0;
    uint32_t resp_msg_size = 0;
    int rc = 0;

    LOGD("csd_server_ioctl_vs_cmd_set_mute() IN\n");
    vs_handle = qmi_csd_vs_req->u.qmi_csd_get_ui_prop_req.handle;
    if (!is_valid_voice_stream_handle(clnt_data, vs_handle)) {
        rc = 1;
        goto EXIT;
    }

    pthread_mutex_lock(&cond_mutex);
    voice_stream_data = (struct voice_stream *)vs_handle;
    voice_stream_data->mute.direction = qmi_csd_vs_req->u.qmi_csd_set_mute_req.qmi_csd_vs_ioctl_set_mute_payload.direction;
    voice_stream_data->mute.mute_flag = qmi_csd_vs_req->u.qmi_csd_set_mute_req.qmi_csd_vs_ioctl_set_mute_payload.mute_flag;
    memset(&csd_common_data.command, 0, sizeof(struct command_data));
    csd_common_data.command.token = qmi_csd_vs_req->u.qmi_csd_set_mute_req.qmi_csd_vs_ioctl_set_mute_payload.cmd_token;
    csd_common_data.command.msg_id = msg_id;
    csd_common_data.command.req_handle = req_handle;
    csd_common_data.thread_state = STATE_PROCESS_CMD;
    csd_common_data.command.command_handle = vs_handle;
    csd_common_data.command.command_client_data = clnt_data;
    pthread_cond_signal(&condition);
    pthread_mutex_unlock(&cond_mutex);

EXIT:
    LOGD("csd_server_ioctl_vs_cmd_set_mute() rc=%d\n",rc);
    if (rc != 0) {
        LOGE("CSD_VS_IOCTL_SET_MUTE FAILED rc=%d\n", rc);
        resp_msg.u.qmi_csd_set_ui_prop_resp.resp.result =
                                                        QMI_RESULT_FAILURE_V01;
        resp_msg.u.qmi_csd_set_ui_prop_resp.resp.error = QMI_ERR_GENERAL_V01;
        resp_msg.u.qmi_csd_set_ui_prop_resp.qmi_csd_status_code_valid = true;
        resp_msg.u.qmi_csd_set_ui_prop_resp.qmi_csd_status_code =
                                                           QMI_CSD_EFAILED_V01;
        resp_msg.u.qmi_csd_set_ui_prop_resp.handle_valid = false;
        resp_msg.u.qmi_csd_set_ui_prop_resp.cmd_token_valid = false;
        resp_msg_size = sizeof(resp_msg.u.qmi_csd_set_ui_prop_resp);
        resp_err = qmi_csi_send_resp(req_handle, msg_id,
                                     &resp_msg, resp_msg_size);
        if (resp_err != QMI_CSI_NO_ERR) {
            LOGE("csd_server_ioctl_vs_cmd_set_mute() send_resp error=%d\n",
                  resp_err);
            rc = QMI_CSI_CB_INTERNAL_ERR;
        } else {
            rc = QMI_CSI_CB_NO_ERR;
        }
        return rc;

    }
    return QMI_CSI_CB_NO_ERR;
}

qmi_csi_cb_error csd_server_ioctl_vm_cmd_detach_context(struct client_data *clnt_data,
                                                       qmi_req_handle req_handle,
                                                       int32_t msg_id,
                                                       void *req_c_struct)
{
    qmi_csi_cb_error qmi_rc = QMI_CSI_CB_NO_ERR;
    qmi_csd_vm_cmd_req_msg *qmi_csd_vm_req =
                                         (qmi_csd_vm_cmd_req_msg *)req_c_struct;
    qmi_csd_vm_cmd_resp_msg resp_msg = {0};
    uint32_t resp_msg_size = 0;
    qmi_csi_error resp_err;
    uint32_t vm_handle = 0;
    int rc = 0;

    LOGD("csd_server_ioctl_vm_cmd_detach_context() IN\n");
    vm_handle = qmi_csd_vm_req->u.qmi_csd_detach_context_req.handle;
    if (!is_valid_voice_manager_handle(clnt_data, vm_handle)) {
        LOGE("FAILED is_valid_voice_manager_handle() vm_handle=0x%08x\n",
             vm_handle);
        rc = 1;
        goto EXIT;
    }

    pthread_mutex_lock(&cond_mutex);
    memset(&csd_common_data.command, 0, sizeof(struct command_data));
    csd_common_data.command.token = qmi_csd_vm_req->u.qmi_csd_detach_context_req.qmi_csd_vm_ioctl_detach_context_payload.cmd_token;
    csd_common_data.command.msg_id = msg_id;
    csd_common_data.command.req_handle = req_handle;
    csd_common_data.thread_state = STATE_PROCESS_CMD;
    csd_common_data.command.command_handle = vm_handle;
    csd_common_data.command.command_client_data = clnt_data;
    pthread_cond_signal(&condition);
    pthread_mutex_unlock(&cond_mutex);

EXIT:
    LOGD("csd_server_ioctl_vm_cmd_detach_context() rc=%d\n",rc);
    if (rc != 0) {
        LOGE("IOCTL_VM_CMD_DETACH_CONTEXT_REQ_V01 FAILED rc %d\n", rc);
        resp_msg.u.qmi_csd_detach_context_resp.resp.result =
                                                         QMI_RESULT_FAILURE_V01;
        resp_msg.u.qmi_csd_detach_context_resp.resp.error = QMI_ERR_GENERAL_V01;
        resp_msg.u.qmi_csd_detach_context_resp.qmi_csd_status_code_valid = true;
        resp_msg.u.qmi_csd_detach_context_resp.qmi_csd_status_code =
                                                            QMI_CSD_EFAILED_V01;
        resp_msg.u.qmi_csd_detach_context_resp.handle_valid = false;
        resp_msg.u.qmi_csd_detach_context_resp.cmd_token_valid = false;
        resp_msg_size = sizeof(resp_msg.u.qmi_csd_detach_context_resp);
        resp_err = qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                                     resp_msg_size);
        if (resp_err != QMI_CSI_NO_ERR) {
            LOGE("csd_server_ioctl_vm_cmd_detach_context send_resp err=0x%x\n",
                 resp_err);
            rc = QMI_CSI_CB_INTERNAL_ERR;
        } else {
            rc = QMI_CSI_CB_NO_ERR;
        }
        return rc;
    }

    return QMI_CSI_CB_NO_ERR;
}

qmi_csi_cb_error csd_server_ioctl_vc_cmd_disable(struct client_data *clnt_data,
                                                 qmi_req_handle req_handle,
                                                 int32_t msg_id,
                                                 void *req_c_struct)
{
    qmi_csi_cb_error qmi_rc = QMI_CSI_CB_NO_ERR;
    qmi_csd_vc_cmd_req_msg *qmi_csd_vc_req =
                                         (qmi_csd_vc_cmd_req_msg *)req_c_struct;
    qmi_csd_vc_cmd_resp_msg resp_msg = {0};
    uint32_t resp_msg_size = 0;
    qmi_csi_error resp_err;
    uint32_t vc_handle = 0;
    int rc = 0;

    LOGD("csd_server_ioctl_vc_cmd_disable() IN\n");

    vc_handle = qmi_csd_vc_req->u.qmi_csd_disable_req.handle;
    if (!is_valid_voice_context_handle(clnt_data, vc_handle)) {
        LOGE("FAILED is_valid_voice_context_handle() vc_handle=0x%08x\n",
              vc_handle);
        rc = 1;
        goto EXIT;
    }

    pthread_mutex_lock(&cond_mutex);
    memset(&csd_common_data.command, 0, sizeof(struct command_data));
    csd_common_data.command.token =
                               qmi_csd_vc_req->u.qmi_csd_disable_req.cmd_token;
    csd_common_data.command.msg_id = msg_id;
    csd_common_data.command.req_handle = req_handle;
    csd_common_data.thread_state = STATE_PROCESS_CMD;
    csd_common_data.command.command_handle = vc_handle;
    csd_common_data.command.command_client_data = clnt_data;
    pthread_cond_signal(&condition);
    pthread_mutex_unlock(&cond_mutex);

EXIT:
    LOGD("csd_server_ioctl_vc_cmd_disable() rc=%d\n",rc);
    if ((rc != 0)) {
        LOGD("CSD_VC_IOCTL_DISABLE FAILED rc=%d\n", rc);
        resp_msg.u.qmi_csd_disable_resp.resp.result = QMI_RESULT_FAILURE_V01;
        resp_msg.u.qmi_csd_disable_resp.resp.error = QMI_ERR_GENERAL_V01;
        resp_msg.u.qmi_csd_disable_resp.qmi_csd_status_code_valid = true;
        resp_msg.u.qmi_csd_disable_resp.qmi_csd_status_code = QMI_CSD_EFAILED_V01;
        resp_msg.u.qmi_csd_disable_resp.handle_valid = false;
        resp_msg.u.qmi_csd_disable_resp.cmd_token_valid = false;
        resp_msg_size = sizeof(resp_msg.u.qmi_csd_disable_resp);
        resp_err = qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                                     resp_msg_size);
        if (resp_err != QMI_CSI_NO_ERR) {
            LOGD("csd_server_ioctl_vc_cmd_disable send_resp error=%d\n",
                 resp_err);
            rc = QMI_CSI_CB_INTERNAL_ERR;
        } else {
            rc = QMI_CSI_CB_NO_ERR;
        }
        return rc;
    }

    return QMI_CSI_CB_NO_ERR;
}

qmi_csi_cb_error csd_server_ioctl_dev_cmd_disable(struct client_data *clnt_data,
                                                  qmi_req_handle req_handle,
                                                  int32_t msg_id,
                                                  void *req_c_struct)
{
    uint32_t  i, j;
    qmi_csd_dev_cmd_resp_msg resp_msg;
    uint32_t resp_msg_size;
    uint32_t rc = 0;
    qmi_csi_error resp_err;
    struct device_data *device_data;
    uint32_t device_handle = 0;
    int32_t no_of_devices = 0;
    qmi_csd_dev_cmd_req_msg *qmi_csd_dev_req = (qmi_csd_dev_cmd_req_msg *)req_c_struct;
    qmi_csd_dev_disable_v01 *req_dev_dis = (qmi_csd_dev_disable_v01 *)
                        (&(qmi_csd_dev_req->u.qmi_csd_disable_dev_req.qmi_csd_dev_disable_cmd_payload));

    LOGD("csd_server_ioctl_dev_cmd_disable() IN\n");
    device_handle = qmi_csd_dev_req->u.qmi_csd_disable_dev_req.handle;
    if (!is_valid_device_handle(clnt_data, device_handle)) {
        rc = 1;
        goto EXIT;
    }
    device_data =  (struct device_data *)device_handle;

    no_of_devices = req_dev_dis->dev_ids_len;

    if (no_of_devices != MAX_NO_ACTIVE_DEVICES) {
        rc = 1;
        LOGE("Cannot set devices, parameter list of devices=%d doesnt match\n"
             "required number=%d\n", no_of_devices, MAX_NO_ACTIVE_DEVICES);
        goto EXIT;
    }

    rc = csd_alsa_disable_device(clnt_data, req_dev_dis->dev_ids[0],
                                 req_dev_dis->dev_ids[1]);

    /* Clear devices and tty_modes from all sessions */
    for (i = 0; i < no_of_devices; i++)
         csd_common_data.device_data.devices[i].dev_id = 0;

    for (i = 0; i < MAX_CSD_QMI_CLIENTS; i++)
        for (j = 0; j < MAX_SESSIONS_PER_CLIENT; j++)
             csd_common_data.client_data[i].session[j].voice_stream_manager_data.tty_mode = 0;

    if (rc != 0)
        LOGE("csd_alsa_disable_device() failed rc=%d\n", rc);

EXIT:
    LOGD("csd_server_ioctl_dev_cmd_disable() rc=%d\n",rc);
    memset(&resp_msg, 0, sizeof(qmi_csd_dev_cmd_resp_msg));
    if (rc != 0) {
        LOGE("CSD_DEV_CMD_DISABLE FAILED rc=%d\n", rc);
        resp_msg.u.qmi_csd_disable_dev_resp.resp.result= QMI_RESULT_FAILURE_V01;
        resp_msg.u.qmi_csd_disable_dev_resp.resp.error = QMI_ERR_GENERAL_V01;
        resp_msg.u.qmi_csd_disable_dev_resp.qmi_csd_status_code_valid = true;
        resp_msg.u.qmi_csd_disable_dev_resp.qmi_csd_status_code =
                                       (qmi_csd_status_v01) QMI_CSD_EFAILED_V01;
    } else {
        resp_msg.u.qmi_csd_disable_dev_resp.qmi_csd_status_code_valid = false;
    }

    resp_msg_size = sizeof(resp_msg.u.qmi_csd_disable_dev_resp);
    resp_err = qmi_csi_send_resp(req_handle, msg_id, &resp_msg, resp_msg_size);
    if (resp_err != QMI_CSI_NO_ERR) {
        LOGE("csd_server_ioctl_dev_cmd_disable() send_resp err=%d\n",resp_err);
        rc = QMI_CSI_CB_INTERNAL_ERR;
    } else {
        rc = QMI_CSI_CB_NO_ERR;
    }
    LOGD("audio_csd_ioctl_dev_cmd_enable_req_handler() OUT rc=%d\n", rc);

    return rc;
}

static qmi_csi_cb_error csd_server_handle_req_cb(void *connection_handle,
                                                 qmi_req_handle req_handle,
                                                 int msg_id,
                                                 void *req_c_struct,
                                                 int req_c_struct_len,
                                                 void *service_cookie)
{
    qmi_csi_cb_error rc = QMI_CSI_CB_NO_ERR;
    qmi_csi_error resp_err;
    struct client_data *clnt_data = (struct client_data *)connection_handle;
    /*
     * A request is being handled, increment the service_cookie num_requests
     */
    ((service_context_type *)service_cookie)->num_requests++;

    LOGD("csd_server_handle_req_cb() IN\n");

    if (((msg_id != QMI_CSD_INIT_REQ_V01) &&
         (msg_id != QMI_CSD_DEINIT_REQ_V01) &&
         (msg_id != QMI_CSD_QUERY_DRIVER_VERSION_REQ_V01)) &&
        QMI_CSD_INIT_DONE != qmi_csd_init) {
        LOGE("Failed CSD_QMI Not Initialized\n");
        return QMI_CSI_CB_INTERNAL_ERR;
    }

    switch (msg_id) {
       case QMI_CSD_INIT_REQ_V01:
            rc = csd_server_init(clnt_data, req_handle, msg_id, req_c_struct);
            break;

        case QMI_CSD_DEINIT_REQ_V01:
             rc = csd_server_deinit(clnt_data, req_handle, msg_id,
                                    req_c_struct);
             break;

        case QMI_CSD_CLOSE_REQ_V01:
             rc = csd_server_close_handle(clnt_data,req_handle, msg_id,
                                          req_c_struct);
             break;

        case QMI_CSD_OPEN_DEVICE_CONTROL_REQ_V01:
             rc = csd_server_open_device_control(clnt_data, req_handle,
                                                 msg_id, req_c_struct);
             break;

        case QMI_CSD_IOCTL_DEV_CMD_ENABLE_REQ_V01:
             rc = csd_server_ioctl_dev_cmd_enable(clnt_data, req_handle, msg_id,
                                                  req_c_struct);
             break;

        case QMI_CSD_OPEN_PASSIVE_CONTROL_VOICE_STREAM_REQ_V01:
             rc = csd_server_open_passive_control_voice_stream(clnt_data,
                                                               req_handle,
                                                               msg_id,
                                                               req_c_struct);
             break;

        case QMI_CSD_OPEN_VOICE_CONTEXT_REQ_V01:
             rc = csd_server_open_voice_context(clnt_data, req_handle, msg_id,
                                                req_c_struct);
             break;

        case QMI_CSD_OPEN_VOICE_MANAGER_REQ_V01:
             rc = csd_server_open_voice_manage(clnt_data,req_handle, msg_id,
                                               req_c_struct);
             break;

        case QMI_CSD_IOCTL_VC_CMD_SET_DEVICE_CONFIG_REQ_V01:
            rc = csd_server_ioctl_vc_set_device_config(clnt_data, req_handle,
                                                       msg_id, req_c_struct);
            break;

        case QMI_CSD_IOCTL_VC_CMD_ENABLE_REQ_V01:
             rc = csd_server_ioctl_vc_cmd_enable(clnt_data, req_handle, msg_id,
                                                 req_c_struct);
             break;

        case QMI_CSD_IOCTL_VM_CMD_ATTACH_CONTEXT_REQ_V01:
             rc = csd_server_ioctl_vm_cmd_attach_context(clnt_data, req_handle,
                                                         msg_id, req_c_struct);
             break;

        case QMI_CSD_IOCTL_VM_CMD_SET_WIDEVOICE_REQ_V01:
             rc = csd_server_ioctl_vm_cmd_set_widevoice(clnt_data, req_handle,
                                                        msg_id, req_c_struct);
             break;

        case QMI_CSD_IOCTL_VM_CMD_SET_HDVOICE_MODE_REQ_V01:
             rc = csd_server_ioctl_vm_cmd_set_hdvoice_mode(clnt_data, req_handle,
                                                           msg_id, req_c_struct);
             break;

        case QMI_CSD_IOCTL_VM_CMD_SET_TTY_MODE_REQ_V01:
             rc = csd_server_ioctl_vm_cmd_set_tty_mode(clnt_data, req_handle,
                                                       msg_id, req_c_struct);
             break;

        case QMI_CSD_IOCTL_VS_CMD_SET_UI_PROPERTY_REQ_V01:
             rc = csd_server_ioctl_vs_cmd_set_ui_property(clnt_data,req_handle,
                                                          msg_id, req_c_struct);
            break;

        case QMI_CSD_IOCTL_VM_CMD_START_VOICE_REQ_V01:
             rc = csd_server_ioctl_vm_cmd_start_voice(clnt_data, req_handle,
                                                      msg_id, req_c_struct);
            break;

        case QMI_CSD_IOCTL_VC_CMD_SET_RX_VOLUME_INDEX_REQ_V01:
             rc = csd_server_ioctl_vc_cmd_set_rx_volume_index(clnt_data,
                                                              req_handle,
                                                              msg_id,
                                                              req_c_struct);
             break;

        case QMI_CSD_IOCTL_VS_CMD_SET_MUTE_REQ_V01:
              rc = csd_server_ioctl_vs_cmd_set_mute(clnt_data, req_handle,
                                                    msg_id,
                                                    req_c_struct);
              break;

        case QMI_CSD_IOCTL_VM_CMD_STOP_VOICE_REQ_V01:
             rc = csd_server_ioctl_vm_cmd_stop_voice(clnt_data, req_handle,
                                                     msg_id, req_c_struct);
             break;

        case QMI_CSD_IOCTL_VM_CMD_DETACH_CONTEXT_REQ_V01:
             rc = csd_server_ioctl_vm_cmd_detach_context(clnt_data, req_handle,
                                                         msg_id, req_c_struct);
             break;

        case QMI_CSD_IOCTL_VC_CMD_DISABLE_REQ_V01:
              rc = csd_server_ioctl_vc_cmd_disable(clnt_data, req_handle,
                                                   msg_id, req_c_struct);
              break;

        case QMI_CSD_IOCTL_DEV_CMD_DISABLE_REQ_V01:
               rc = csd_server_ioctl_dev_cmd_disable(clnt_data,req_handle,
                                                     msg_id,
                                                     req_c_struct);
               break;

        case QMI_CSD_IOCTL_VM_CMD_STANDBY_VOICE_REQ_V01:
             rc = csd_server_ioctl_vm_cmd_standby_voice(clnt_data,req_handle,
                                                        msg_id, req_c_struct);
             break;

        default:
            LOGE("Message ID:[%08X] not implemented yet.\n", msg_id);
            rc = QMI_CSI_CB_INTERNAL_ERR;
    }
    LOGD("Message ID:[%08X] rc=[%08X]\n", msg_id, rc);
    return rc;
}

void *csd_server_register_service(qmi_csi_os_params *os_params)
{
    qmi_idl_service_object_type csd_service_object =
                                                  csd_get_service_object_v01();
    qmi_client_os_params qsap_os_params;
    qmi_csi_error rc = QMI_CSI_INTERNAL_ERR;

    rc = qmi_csi_register(csd_service_object, csd_server_connect_cb,
                          csd_server_disconnect_cb, csd_server_handle_req_cb,
                          &service_cookie, os_params,
                          &service_cookie.service_handle);

    LOGD("qmi_csi_register() rc=%d\n",rc);

    rc |= qmi_sap_register(csd_service_object, &qsap_os_params,
                           &service_cookie.qsap_handle);

    LOGD("qmi_sap_register() rc=%d\n",rc);

    if (rc != QMI_NO_ERR)
        return NULL;

    return service_cookie.service_handle;
}

int main(int argc, char **argv)
{
    /* on-target intiialization code */
    qmi_csi_os_params os_params, os_params_in;
    fd_set fds;
    void *sp;
    int rc = 0;

    rc = csd_alsa_preinit();
    if (rc != 0) {
        LOGE("csd_alsa_preinit failed error rc=%d\n", rc);
        exit(1);
    }
    sp = csd_server_register_service(&os_params);

    if (!sp) {
        LOGE("Unable to register service!\n");
        exit(1);
    } else {
        LOGD("csd-server is up\n");
    }

    while(1) {
        fds = os_params.fds;
        select(os_params.max_fd + 1, &fds, NULL, NULL, NULL);
        os_params_in.fds = fds;
        qmi_csi_handle_event(sp, &os_params_in);
    }
    qmi_csi_unregister(sp);

    return 0;
}/* main */
