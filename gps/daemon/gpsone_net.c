/* Copyright (c) 2010 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * Copyright (c) 2012 Qualcomm Atheros, Inc.
 * All Rights Reserved.
 * Qualcomm Atheros Confidential and Proprietary
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "gpsone_daemon_dbg.h"
#include "gpsone_glue_msg.h"
#include "gpsone_net.h"
#include "gpsone_thread_helper.h"
#include "gpsone_ctrl_msg.h"

static int loc_api_server_msgqid;
static int loc_api_resp_msgqid;
extern const char * global_gpsone_loc_api_q_path;
extern const char * global_gpsone_loc_api_resp_q_path;
static struct gpsone_thelper thelper_net;

#ifndef DEBUG_X86
#include "gpsone_bit_forward.h"
#endif

#if !defined(DEBUG_X86) && !defined(FEATURE_DORMANCY_DISABLE)
#include "qmi_wds_srvc.h"
#include "dsi_netctrl.h"

static int qmi_client_handle = -1;
static dsi_hndl_t dsi_tech_hndl = 0;
static int is_data_call_up = 0;

struct event_strings_s
{
  dsi_net_evt_t evt;
  char * str;
};

static struct event_strings_s event_string_tbl[DSI_EVT_MAX] =
{
  { DSI_EVT_INVALID, "DSI_EVT_INVALID" },
  { DSI_EVT_NET_IS_CONN, "DSI_EVT_NET_IS_CONN" },
  { DSI_EVT_NET_NO_NET, "DSI_EVT_NET_NO_NET" },
  { DSI_EVT_PHYSLINK_DOWN_STATE, "DSI_EVT_PHYSLINK_DOWN_STATE" },
  { DSI_EVT_PHYSLINK_UP_STATE, "DSI_EVT_PHYSLINK_UP_STATE" },
  { DSI_EVT_NET_RECONFIGURED, "DSI_EVT_NET_RECONFIGURED" }
};

/*===========================================================================
FUNCTION    gpsone_net_dsi_ev_cb

DESCRIPTION
   This is the call back from data service.  gpsone_net_dsi_init has to wait for
   this callback after the initialization

DEPENDENCIES
   None

RETURN VALUE

SIDE EFFECTS
   N/A

===========================================================================*/
static void gpsone_net_dsi_ev_cb( dsi_hndl_t hndl,
                                  void * user_data,
                                  dsi_net_evt_t evt,
                                  dsi_evt_payload_t *payload_ptr )
{
    int i;
    (void)payload_ptr;
    GPSONE_DMN_DBG("%s:%d] evt = %d\n", __func__, __LINE__, evt);
    for (i=0; i<DSI_EVT_MAX; i++)
    {
      if (event_string_tbl[i].evt == evt)
        GPSONE_DMN_DBG("%s:%d] %s\n", __func__, __LINE__, event_string_tbl[i].str);
        break;
    }

    switch (evt) {
        case DSI_EVT_NET_NO_NET:
            GPSONE_DMN_DBG("%s:%d] dsi event DSI_EVT_NET_NO_NET %d\n", __func__, __LINE__, evt);
            break;

        case DSI_EVT_PHYSLINK_DOWN_STATE:
        case DSI_EVT_PHYSLINK_UP_STATE:
        case DSI_EVT_NET_RECONFIGURED:
            GPSONE_DMN_DBG("%s:%d] dsi event %d\n", __func__, __LINE__, evt);
            break;

        case DSI_EVT_NET_IS_CONN:
            GPSONE_DMN_DBG("%s:%d] dsi event DSI_EVT_NET_IS_CONN %d\n", __func__, __LINE__, evt);
            thelper_signal_ready(&thelper_net);
            break;

        case DSI_EVT_INVALID:
        default:
            GPSONE_DMN_PR_ERR("%s:%d] error - unknown dsi event %d\n", __func__, __LINE__, evt);
            break;
    }
}

/*===========================================================================
FUNCTION    gpsone_net_qmi_wds_indication_hdlr

DESCRIPTION
   This is the handler of the qmi request response

   user_handle - user handle
   service_id - no use
   user_data - no use
   ind_id - no use
   ind_data - no use

DEPENDENCIES
   None

RETURN VALUE

SIDE EFFECTS
   N/A

===========================================================================*/
static void gpsone_net_qmi_wds_indication_hdlr( int user_handle, qmi_service_id_type  service_id,
    void *user_data, qmi_wds_indication_id_type ind_id, qmi_wds_indication_data_type *ind_data)
{
    struct ctrl_msgbuf cmsgbuf;
    extern int daemon_manager_msgqid;

    GPSONE_DMN_DBG("%s:%d] user_handle = %d, qmi_client_handle = %d ind_id = %d\n",
        __func__, __LINE__, user_handle, qmi_client_handle, ind_id);
    if (user_handle != qmi_client_handle) {
        GPSONE_DMN_PR_ERR("%s:%d] user_handle does not match qmi_client_handle\n", __func__, __LINE__);
        return;
    }

    if (ind_id != QMI_WDS_SRVC_EVENT_REPORT_IND_MSG) {
        GPSONE_DMN_PR_ERR("%s:%d] not QMI_WDS_SRVC_EVENT_REPORT_IND_MSG %d\n", __func__, __LINE__,
            QMI_WDS_SRVC_EVENT_REPORT_IND_MSG);
        return;
    }

    cmsgbuf.ctrl_type = GPSONE_FORCE_DORMANCY_STATUS;
    gpsone_glue_msgsnd(daemon_manager_msgqid, & cmsgbuf, sizeof(cmsgbuf));

    return;
}
#endif

/*===========================================================================
FUNCTION gpsone_net_check_dormancy_status

DESCRIPTION
   qmi_wds_get_dormancy_status cannot be called from gpsone_net_qmi_wds_indication_hdlr
   context, thus we deliver a message to daemon manager and then check
   dormancy status from daemon manager thread

DEPENDENCIES
   None

RETURN VALUE
   0: dormant; 1: active; -1: error

SIDE EFFECTS
   N/A

===========================================================================*/
int gpsone_net_check_dormancy_status(void)
{
    int result = -1;
#if !defined(DEBUG_X86) && !defined(FEATURE_DORMANCY_DISABLE)
    qmi_wds_dorm_status_type dorm_status;
    int qmi_err_code;

    GPSONE_DMN_DBG("%s:%d]\n", __func__, __LINE__);
    result = qmi_wds_get_dormancy_status ( qmi_client_handle, &dorm_status, &qmi_err_code);
    if (result != QMI_NO_ERR) {
        GPSONE_DMN_PR_ERR("%s:%d] result = %d, err code = %d\n", __func__, __LINE__, result, qmi_err_code);
        result = -1;
    } else {
        if (dorm_status == QMI_WDS_DORM_STATUS_DORMANT) {
            result = 0;
            GPSONE_DMN_DBG("%s:%d] DORM DORMANT\n", __func__, __LINE__);
        } else if ( dorm_status == QMI_WDS_DORM_STATUS_ACTIVE) {
            result = 1;
            GPSONE_DMN_DBG("%s:%d] DORM ACTIVE\n", __func__, __LINE__);
        } else {
            GPSONE_DMN_PR_ERR("%s:%d] result = %d, err code = %d, unknown dorm_status = %d\n",
                __func__, __LINE__, result, qmi_err_code, dorm_status);
            result = -1;
        }
    }
#endif
    return result;
}

/*===========================================================================
FUNCTION    gpsone_net_dsi_init

DESCRIPTION
   This function will initialize the net ctrl resources

DEPENDENCIES
   None

RETURN VALUE
   0: success or negative value for failure

SIDE EFFECTS
   N/A

===========================================================================*/
int gpsone_net_dsi_init(void)
{
#if !defined(DEBUG_X86) && !defined(FEATURE_DORMANCY_DISABLE)
    int result;

    dsi_call_param_value_t param_info;
    char qmi_port_name[DSI_CALL_INFO_DEVICE_NAME_MAX_LEN + 1];
    int qmi_err_code;
    qmi_wds_event_report_params_type event_report_params;

    GPSONE_DMN_DBG("%s:%d]\n", __func__, __LINE__);

    if (!dsi_tech_hndl) {

        if(DSI_SUCCESS != dsi_init(DSI_MODE_GENERAL))
        {
            GPSONE_DMN_DBG("%s:%d] dsi_init failed\n",__func__, __LINE__);
            return -1;
        }

        /* obtain data service handle */
        dsi_tech_hndl = dsi_get_data_srvc_hndl(gpsone_net_dsi_ev_cb, NULL);

        if (!dsi_tech_hndl) {
            GPSONE_DMN_DBG("%s:%d] failed\n", __func__, __LINE__);
            return -1;
        }

    }

    /* set data call param */
    param_info.buf_val = NULL;
    param_info.num_val = DSI_RADIO_TECH_1X;
    result = dsi_set_data_call_param(dsi_tech_hndl, DSI_CALL_INFO_TECH_PREF, &param_info);

    if (result != DSI_SUCCESS) {
        GPSONE_DMN_DBG("%s:%d] failed\n", __func__, __LINE__);
        return -1;
    }

    /* start data call */
    GPSONE_DMN_DBG("%s:%d] Start data call\n", __func__, __LINE__);
    result = dsi_start_data_call(dsi_tech_hndl);
    if (result != DSI_SUCCESS) {
        GPSONE_DMN_DBG("%s:%d] failed\n", __func__, __LINE__);
        return -1;
    } else {
        result = thelper_signal_timedwait(&thelper_net, 5);
        if (result) {
            GPSONE_DMN_PR_ERR("%s:%d] result = %d\n", __func__, __LINE__, result);
            return -1;
        } else {
            is_data_call_up = 1;
            GPSONE_DMN_DBG("%s:%d] data call is up\n", __func__, __LINE__);
        }
    }

    result = dsi_get_device_name(dsi_tech_hndl, qmi_port_name, DSI_CALL_INFO_DEVICE_NAME_MAX_LEN + 1);
    if (result != DSI_SUCCESS) {
        GPSONE_DMN_DBG("%s:%d] failed\n", __func__, __LINE__);
        return -1;
    }

    qmi_client_handle = qmi_wds_srvc_init_client(qmi_port_name, gpsone_net_qmi_wds_indication_hdlr, NULL, &qmi_err_code);
    if (qmi_client_handle < 0) {
        GPSONE_DMN_PR_ERR("%s:%d] handle = %d, err code = %d\n", __func__, __LINE__, qmi_client_handle, qmi_err_code);
        return -1;
    }

    /* now register callback */
    event_report_params.param_mask = 0;
    event_report_params.param_mask |= QMI_WDS_EVENT_DORM_STATUS_IND;
    event_report_params.report_dorm_status = 1;

    result = qmi_wds_set_event_report(qmi_client_handle, &event_report_params,&qmi_err_code);
    if (result < 0) {
        GPSONE_DMN_PR_ERR("%s:%d] result = %d, err code = %d\n", __func__, __LINE__, result, qmi_err_code);
        return -1;
    }

    GPSONE_DMN_DBG("%s:%d]\n", __func__, __LINE__);
#endif
    return 0;
}

/*===========================================================================
FUNCTION    gpsone_net_dsi_release

DESCRIPTION
   This function will release the resources for net ctrl

DEPENDENCIES
   None

RETURN VALUE

SIDE EFFECTS
   N/A

===========================================================================*/
int gpsone_net_dsi_release(void)
{
    int result = 0;
#if !defined(DEBUG_X86) && !defined(FEATURE_DORMANCY_DISABLE)
    int qmi_err_code;

    GPSONE_DMN_DBG("%s:%d]\n", __func__, __LINE__);

    result = qmi_wds_srvc_release_client(qmi_client_handle, &qmi_err_code);
    if (result < 0) {
        GPSONE_DMN_PR_ERR("%s:%d] result = %d, err code %d\n", __func__, __LINE__, result, qmi_err_code);
    }

    qmi_client_handle = -1;

    if (is_data_call_up) {
        dsi_stop_data_call(dsi_tech_hndl);
        is_data_call_up = 0;
        GPSONE_DMN_DBG("%s:%d] data call is stopped", __func__, __LINE__);
    } else {
        GPSONE_DMN_PR_ERR("%s:%d] why is this called when data call is not up\n", __func__, __LINE__);
    }
#endif
    return result;
}

/*===========================================================================
FUNCTION    gpsone_net_init

DESCRIPTION
   This function will initialize the message queue for net and call dsi_init

DEPENDENCIES
   None

RETURN VALUE
   0: success

SIDE EFFECTS
   N/A

===========================================================================*/
int gpsone_net_init(void)
{
    int result = 0;
    GPSONE_DMN_DBG("%s:%d]\n", __func__, __LINE__);
    loc_api_server_msgqid = gpsone_glue_msgget(global_gpsone_loc_api_q_path, O_RDWR);

    loc_api_resp_msgqid = gpsone_glue_msgget(global_gpsone_loc_api_resp_q_path, O_RDWR);

    result = thelper_signal_init(&thelper_net);
    if (result) {
        return -1;
    }

    return 0;
}

/*===========================================================================
FUNCTION    gpsone_net_release

DESCRIPTION
   This function will release the message queue for net

DEPENDENCIES
   None

RETURN VALUE
   0: success

SIDE EFFECTS
   N/A

===========================================================================*/
int gpsone_net_release(void)
{
    gpsone_glue_msgremove(NULL, loc_api_server_msgqid);
    gpsone_glue_msgremove(NULL, loc_api_resp_msgqid);
    GPSONE_DMN_DBG("%s:%d]\n", __func__, __LINE__);

#if !defined(DEBUG_X86) && !defined(FEATURE_DORMANCY_DISABLE)
    if(dsi_tech_hndl) {
        dsi_rel_data_srvc_hndl(dsi_tech_hndl);
        dsi_tech_hndl = 0;
    }
#endif
    thelper_signal_destroy(&thelper_net);
    return 0;
}

/*===========================================================================
FUNCTION    gpsone_net_if_request

DESCRIPTION
   This function sends network interface request to connectivity manager
   through loc_api

DEPENDENCIES
   None

RETURN VALUE
   0: success
   -1: failure

SIDE EFFECTS
   N/A

===========================================================================*/
int gpsone_net_if_request(unsigned is_supl, unsigned long ipv4_addr, unsigned char ipv6_addr[16])
{
    struct ctrl_msgbuf cmsg;
    cmsg.ctrl_type = GPSONE_LOC_API_IF_REQUEST;
    cmsg.cmsg.cmsg_if_request.type = is_supl?
        IF_REQUEST_TYPE_SUPL : IF_REQUEST_TYPE_ANY;
    cmsg.cmsg.cmsg_if_request.sender_id = IF_REQUEST_SENDER_ID_GPSONE_DAEMON;
    cmsg.cmsg.cmsg_if_request.ipv4_addr = ipv4_addr;
    if (ipv6_addr) memcpy( cmsg.cmsg.cmsg_if_request.ipv6_addr, ipv6_addr, 16);
    gpsone_glue_msgsnd(loc_api_server_msgqid, &cmsg, sizeof(cmsg));

    GPSONE_DMN_DBG("%s:%d] is_supl: %d, ipv4: %d.%d.%d.%d, ipv6: %s", __func__, __LINE__, is_supl,
                 (unsigned char)(ipv4_addr>>24),
                 (unsigned char)(ipv4_addr>>16),
                 (unsigned char)(ipv4_addr>>8),
                 (unsigned char)ipv4_addr,
                 (char *) NULL != ipv6_addr? ipv6_addr: "");

    gpsone_glue_msgrcv(loc_api_resp_msgqid, &cmsg, sizeof(cmsg));
    int result = cmsg.cmsg.cmsg_response.result;
    if (result == GPSONE_LOC_API_IF_REQUEST_SUCCESS) {
      GPSONE_DMN_DBG("%s:%d] network i/f up received from HAL!\n", __func__, __LINE__);
      return 0;
    } else if ((result == GPSONE_LOC_API_IF_RELEASE_SUCCESS)||
               (result == GPSONE_LOC_API_IF_FAILURE)) {
      //0 means closed
      //-1 means failed. in either case I/F is down. we will bring down the conn bridges
      GPSONE_DMN_PR_ERR("%s:%d] result is %d. i/f request failed\n", __func__, __LINE__, result);
      return -1;
    } else {
      GPSONE_DMN_PR_ERR("%s:%d] result is %d. unexpected response from HAL!\n", __func__, __LINE__, result);
      return -1;
    }
}

/*===========================================================================
FUNCTION    gpsone_net_if_release

DESCRIPTION
   This function sends network interface release request to connectivity manager
   through loc_api

DEPENDENCIES
   None

RETURN VALUE
   0: success

SIDE EFFECTS
   N/A

===========================================================================*/
int gpsone_net_if_release(unsigned is_supl, unsigned long ipv4_addr, unsigned char ipv6_addr[16])
{
    struct ctrl_msgbuf cmsg;
    cmsg.ctrl_type = GPSONE_LOC_API_IF_RELEASE;
    cmsg.cmsg.cmsg_if_request.type = is_supl?
        IF_REQUEST_TYPE_SUPL : IF_REQUEST_TYPE_ANY;
    cmsg.cmsg.cmsg_if_request.sender_id = IF_REQUEST_SENDER_ID_GPSONE_DAEMON;
    cmsg.cmsg.cmsg_if_request.ipv4_addr = ipv4_addr;
    if (ipv6_addr) memcpy( cmsg.cmsg.cmsg_if_request.ipv6_addr, ipv6_addr, 16);
    gpsone_glue_msgsnd(loc_api_server_msgqid, &cmsg, sizeof(cmsg));

    GPSONE_DMN_DBG("%s:%d] about to block on receive from HAL\n", __func__, __LINE__);
    gpsone_glue_msgrcv(loc_api_resp_msgqid, &cmsg, sizeof(cmsg));
    int result = cmsg.cmsg.cmsg_response.result;
    //disregard the return from release
    GPSONE_DMN_DBG("%s:%d] network i/f result = %d\n", __func__, __LINE__, result);
    return 0;
}

/*===========================================================================
FUNCTION    gpsone_net_unforce_dormancy

DESCRIPTION
   This function will use qmi API to unforce dormancy

   transaction_id - transaction id of the request

DEPENDENCIES
   None

RETURN VALUE
   0: success or negative value for failure

SIDE EFFECTS
   N/A

===========================================================================*/
int gpsone_net_unforce_dormancy(void)
{
    int result = 0;
#if !defined(DEBUG_X86) && !defined(FEATURE_DORMANCY_DISABLE)
    int qmi_err_code;

    /* force_dormancy has to be called first */
    if (qmi_client_handle < 0) {
        GPSONE_DMN_PR_ERR("%s:%d] failed\n", __func__, __LINE__);
        return -1;
    }

    /* start data call */
    result = qmi_wds_go_active_req ( qmi_client_handle, &qmi_err_code);

    if (result != QMI_NO_ERR) {
        GPSONE_DMN_PR_ERR("%s:%d] result = %d, err code = %d\n", __func__, __LINE__, result, qmi_err_code);
        GPSONE_DMN_DBG("%s:%d] failed\n", __func__, __LINE__);
        return -1;
    } else {
        if (result) {
            GPSONE_DMN_PR_ERR("%s:%d] result = %d\n", __func__, __LINE__, result);
            return -1;
        } else {
            GPSONE_DMN_DBG("%s:%d] go_active_req is done\n", __func__, __LINE__);
        }
    }

#endif
    GPSONE_DMN_DBG("%s:%d] result = %d\n", __func__, __LINE__, result);
    return result;
}

/*===========================================================================
FUNCTION    gpsone_net_disable_dormancy_timer

DESCRIPTION
   This function will use qmi interface to disable dormancy timer

DEPENDENCIES
   None

RETURN VALUE
   0: success or negative value for failure

SIDE EFFECTS
   N/A

===========================================================================*/
int gpsone_net_disable_dormancy_timer(void)
{
    int result = 0;

#if !defined(DEBUG_X86) && !defined(FEATURE_DORMANCY_DISABLE)
    int qmi_err_code;
    qmi_wds_set_internal_runtime_settings_params_type params;
    qmi_wds_set_internal_runtime_settings_rsp_type rsp_data;

    if (qmi_client_handle < 0) {
        GPSONE_DMN_PR_ERR("%s:%d]\n", __func__, __LINE__);
        return -1;
    }

    params.params_mask = QMI_WDS_SET_INTERNAL_RUNTIME_ENABLE_HOLDDOWN_PARAM;
    params.holddown_enable = ENABLE;
    result = qmi_wds_set_internal_runtime_settings ( qmi_client_handle,
         & params, & rsp_data, & qmi_err_code);
    if (result != QMI_NO_ERR) {
        GPSONE_DMN_PR_ERR("%s:%d] result = %d, err code = %d\n", __func__, __LINE__, result, qmi_err_code);
    }
#endif
    GPSONE_DMN_DBG("%s:%d] result = %d\n", __func__, __LINE__, result);
    return result;
}

/*===========================================================================
FUNCTION    gpsone_net_force_dormancy

DESCRIPTION
   This function will use qmi interface to force dormancy

    transaction_id - transaction id for this request
DEPENDENCIES
   None

RETURN VALUE
   0: success or negative value for failure

SIDE EFFECTS
   N/A

===========================================================================*/
int gpsone_net_force_dormancy(void)
{
    int result = 0;

#if !defined(DEBUG_X86) && !defined(FEATURE_DORMANCY_DISABLE)
    int qmi_err_code;

    GPSONE_DMN_DBG("%s:%d]\n", __func__, __LINE__);

    result = qmi_wds_go_dormant_req ( qmi_client_handle, &qmi_err_code);
    if (result != QMI_NO_ERR) {
        GPSONE_DMN_PR_ERR("%s:%d] result = %d, err code = %d\n", __func__, __LINE__, result, qmi_err_code);
    }

    result = gpsone_net_disable_dormancy_timer();
    if (result != QMI_NO_ERR) {
        GPSONE_DMN_PR_ERR("%s:%d]\n", __func__, __LINE__);
    }

#endif
    GPSONE_DMN_DBG("%s:%d] result = %d\n", __func__, __LINE__, result);
    return result;
}

