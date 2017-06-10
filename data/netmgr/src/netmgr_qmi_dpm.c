/**
  @file
  netmgr_qmi_dpm.c

  @brief
  Netmgr QMI DPM (QMI Data Port Mapper service) implementation.

  @details
  Netmgr QMI DPM component is used to configure embedded ports on the modem.
  The configuration specifies how many ports to open, what are the control
  ports and what are the data ports (hw-accel and software data ports). It
  can also specify the mapping between control and data ports. The configuration
  is choosen through configdb or hard-coded.

  The following functions are exported:
  netmgr_qmi_dpm_init()
  netmgr_qmi_dpm_open_port()
  netmgr_qmi_dpm_close_port()

  The ports are opened using QCCI APIs (they must go over IPC-router since
  QMUXD transport would not be ready yet).

*/
/*===========================================================================

  Copyright (c) 2013-2015 Qualcomm Technologies, Inc. All Rights Reserved
  Qualcomm Technologies Proprietary and Confidential

===========================================================================*/

/*===========================================================================

                        EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.
Notice that changes are listed in reverse chronological order.

when       who     what, where, why
--------   ---     ----------------------------------------------------------
10/03/13   hm/nd   Initial version

===========================================================================*/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <pthread.h>
#include "ds_util.h"
#include "ds_cmdq.h"
#include <linux/msm_rmnet.h>
#include "data_port_mapper_v01.h"
#include "qmi_client.h"
#include "qmi_client_instance_defs.h"
#include "netmgr_main.h"
#include "netmgr_qmi_dpm.h"
#include "netmgr_netlink.h"
#include "netmgr_util.h"
#include "netmgr_cmdq.h"
#include "ds_string.h"

/*===========================================================================

                               DATA DEFINITIONS

===========================================================================*/

#define NETMGR_QMI_DPM_TIMEOUT        10000
#define NETMGR_QMI_DPM_MAX_RETRIES    5

typedef struct
{
  /* configdb related information */
  boolean use_configdb;
  boolean configdb_inited;

  /* QMI portmapper service info */
  qmi_client_type             dpm_clid;
  boolean                     dpm_inited;

  /* Last sent open_req/close_reqs (for debug purposes) */
  dpm_open_port_req_msg_v01   open_req;
  boolean                     open_req_sent;
  dpm_close_port_req_msg_v01  close_req;

} netmgr_qmi_dpm_state_t;

netmgr_qmi_dpm_state_t netmgr_qmi_dpm_state;

qmi_client_os_params         dpm_os_params;
qmi_cci_os_signal_type       dpm_notifier_os_params;
qmi_client_type              dpm_notifier;

void netmgr_qmi_dpm_notify_cb
(
 qmi_client_type                user_handle,
 qmi_idl_service_object_type    service_obj,
 qmi_client_notify_event_type   service_event,
 void                           *notify_cb_data
);

void netmgr_qmi_dpm_err_cb
(
  qmi_client_type                   user_handle,
  qmi_client_error_type             error,
  void                              *err_cb_data
);

/*===========================================================================
  FUNCTION:  netmgr_qmi_dpm_process_cmdq_event
===========================================================================*/
/** @ingroup netmgr_dpm

    @brief
    Called in response to DPM events as part of command executor module

    @params None.
    @return None.
*/
void netmgr_qmi_dpm_process_cmdq_event
(
  netmgr_dpm_event_t   evt
)
{
  qmi_idl_service_object_type dpm_svc_obj = dpm_get_service_object_v01();

  if (NULL == dpm_svc_obj)
  {
    netmgr_log_err("netmgr_qmi_dpm_process_cmdq_event(): Could not get DPM object!");
    return;
  }

  switch (evt)
  {
  case NETMGR_QMI_DPM_OOS_EV:
    /* We have received SSR notification, clean up existing DPM
     * handle. Initialize a notifier client and register for notifier
     * callback so that we know when DPM service becomes available again
     * on modem which means SSR recovery happened */

    netmgr_log_med("netmgr_qmi_dpm_process_cmdq_event(): Releasing dpm client");
    (void) qmi_client_release(netmgr_qmi_dpm_state.dpm_clid);
    (void) qmi_client_notifier_init(dpm_svc_obj, &dpm_os_params, &dpm_notifier);
    (void) qmi_client_register_notify_cb(dpm_notifier, netmgr_qmi_dpm_notify_cb, NULL);

    /* Reset internal flags */
    netmgr_qmi_dpm_state.dpm_inited = FALSE;
    netmgr_qmi_dpm_state.open_req_sent = FALSE;
    break;

  case NETMGR_QMI_DPM_IS_EV:
    /* We have received SSR recovery message */
    netmgr_log_med("netmgr_qmi_dpm_process_cmdq_event():"
                   " Initializing DPM client and opening port...");
    (void) qmi_client_release(dpm_notifier);
    (void) netmgr_qmi_dpm_init();
    (void) netmgr_qmi_dpm_port_open();
    break;

  default:
    netmgr_log_err("netmgr_qmi_dpm_process_cmdq_event(): Unsupported operation");
    break;
  }
}

/*===========================================================================
  FUNCTION:  netmgr_qmi_dpm_notify_cb
===========================================================================*/
/** @ingroup netmgr_dpm

    @brief
    Callback to receive DPM service events

    @details
    This will inform netmgr when DPM service comes up.
    This is needed to handle SSR.

    @params
    user_handle    QCCI client ID
    service_obj    DPM service object
    service_event  DPM service event

    @return None.
*/
void netmgr_qmi_dpm_notify_cb
(
 qmi_client_type                user_handle,
 qmi_idl_service_object_type    service_obj,
 qmi_client_notify_event_type   service_event,
 void                           *notify_cb_data
)
{
  netmgr_cmdq_cb_cmd_t *cmd_buf = NULL;
  netmgr_log_med("netmgr_qmi_dpm_notify_cb(): Modem is IN_SERVICE");

  cmd_buf = netmgr_cmdq_get_cmd();
  if (NULL == cmd_buf)
  {
    netmgr_log_err("netmgr_qmi_dpm_notify_cb(): Malloc failed for cmd_buf");
    return;
  }

  switch(service_event)
  {
  case QMI_CLIENT_SERVICE_COUNT_INC:
    /* Service is back up, post IN_SERVICE
     * to command thread*/
    netmgr_log_med("netmgr_qmi_dpm_notify_cb(): DPM service is up!");
    cmd_buf->cmd_data.cmd_type               = NETMGR_CB_CMD_TYPE_DPM;
    cmd_buf->cmd_data.netmgr_union.dpm_event = NETMGR_QMI_DPM_IS_EV;

    if ( NETMGR_SUCCESS != netmgr_cmdq_put_cmd(cmd_buf) ) {
      netmgr_log_err("netmgr_qmi_dpm_notify_cb(): Failed to put command!");
      netmgr_cmdq_release_cmd(cmd_buf);
    }

    break;
  case QMI_CLIENT_SERVICE_COUNT_DEC:
    netmgr_log_med("netmgr_qmi_dpm_notify_cb(): DPM service is down!");
    break;
  default:
    netmgr_log_err("netmgr_qmi_dpm_notify_cb(): Unsupported service event!");
  }
}

/*===========================================================================
  FUNCTION:  netmgr_qmi_dpm_err_cb
===========================================================================*/
/** @ingroup netmgr_dpm

    @brief
    Callback to receive DPM error events

    @details
    This will inform netmgr when DPM service goes down on the modem so that
    we can cleanup DPM state. This is needed to handle SSR.

    @params
    user_handle  QCCI client ID
    error
    @return None.
*/
void netmgr_qmi_dpm_err_cb
(
  qmi_client_type                   user_handle,
  qmi_client_error_type             error,
  void                              *err_cb_data
)
{
  netmgr_cmdq_cb_cmd_t* cmd_buf = NULL;

  cmd_buf = netmgr_cmdq_get_cmd();
  netmgr_log_err("netmgr_qmi_dpm_err_cb(): MODEM is OUT_OF_SERVICE");

  if (NULL == cmd_buf)
  {
    netmgr_log_err("netmgr_qmi_dpm_err_cb(): Malloc failed for cmd_buf");
    return;
  }

  cmd_buf->cmd_data.cmd_type               = NETMGR_CB_CMD_TYPE_DPM;
  cmd_buf->cmd_data.netmgr_union.dpm_event = NETMGR_QMI_DPM_OOS_EV;

  if ( NETMGR_SUCCESS != netmgr_cmdq_put_cmd(cmd_buf) ) {
    netmgr_log_err("netmgr_qmi_dpm_err_cb(): Failed to put command!");
    netmgr_cmdq_release_cmd(cmd_buf);
  }
}

/*===========================================================================
  FUNCTION:  netmgr_qmi_dpm_init
===========================================================================*/
/** @ingroup netmgr_dpm

    @brief
    Initializes the QMI DPM module.

    @details
    Performs the QCCI client library initialization to be able to send
    and receive DPM messages.

    @params None.
    @return NETMGR_SUCCESS: in case of success.
    @return NETMGR_FAILURE: in case of error.
    @dependencies None.
*/
int netmgr_qmi_dpm_init(void)
{
  qmi_idl_service_object_type  dpm_svc_obj;
  qmi_client_error_type        rc;
  int retry_counter = 0;

  /* Check if dpm client was already initialized */
  if (TRUE == netmgr_qmi_dpm_state.dpm_inited)
  {
    netmgr_log_med("netmgr_qmi_dpm_init(): DPM client is already initialized!\n");
    return NETMGR_SUCCESS;
  }

  /* Initialize QCCI client to send the open_port request */
  dpm_svc_obj = dpm_get_service_object_v01();
  if (dpm_svc_obj == NULL)
  {
    netmgr_log_err("netmgr_qmi_dpm_init(): DPM service not available");
    return NETMGR_FAILURE;
  }

  /* Unregister QMUX transport since QMUX would not be up at this point */
  netmgr_log_med("netmgr_qmi_dpm_init(): Unregistering QMUX transport");
  qmi_cci_qmux_xport_unregister(QMI_CLIENT_QMUX_RMNET_INSTANCE_0);
  qmi_cci_qmux_xport_unregister(QMI_CLIENT_QMUX_RMNET_USB_INSTANCE_0);
  qmi_cci_qmux_xport_unregister(QMI_CLIENT_QMUX_RMNET_SMUX_INSTANCE_0);
  qmi_cci_qmux_xport_unregister(QMI_CLIENT_QMUX_RMNET_MHI_INSTANCE_0);

  do
  {
    memset(&dpm_os_params, 0, sizeof(dpm_os_params));
    rc = qmi_client_init_instance(dpm_svc_obj,
                                  QMI_CLIENT_INSTANCE_ANY,
                                  NULL,
                                  NULL,
                                  &dpm_os_params,
                                  NETMGR_QMI_DPM_TIMEOUT,
                                  &netmgr_qmi_dpm_state.dpm_clid);

    if (QMI_TIMEOUT_ERR == rc)
    {
      /* There may be some delays in services getting reported to QMI FW. This delay may be more
       * than the timeout we have set internally. We will retry to initialize DPM client till
       * we keep getting timeout errors */
      netmgr_log_err("netmgr_qmi_dpm_init(): DPM client init has failed due to timeout, retrying");
      retry_counter++;
    }
  } while (QMI_TIMEOUT_ERR == rc && NETMGR_QMI_DPM_MAX_RETRIES > retry_counter);

  if (NETMGR_QMI_DPM_MAX_RETRIES == retry_counter)
  {
    /* If max retries have been reached, bail */
    netmgr_log_err("netmgr_qmi_dpm_init(): qmi_client_init_instance failed,"
                   " maximum retries reached!");
    return NETMGR_FAILURE;
  }

  if (rc != QMI_NO_ERR)
  {
    netmgr_log_err("netmgr_qmi_dpm_init(): qmi_client_init_instance failed, err %d\n", rc);
    return NETMGR_FAILURE;
  }

  /* Register for err callback */
  (void) qmi_client_register_error_cb(netmgr_qmi_dpm_state.dpm_clid,
                                      netmgr_qmi_dpm_err_cb,
                                      NULL);

  netmgr_qmi_dpm_state.dpm_inited = TRUE;
  netmgr_qmi_dpm_state.open_req_sent = FALSE;
  memset(&netmgr_qmi_dpm_state.open_req, 0, sizeof(netmgr_qmi_dpm_state.open_req));
  memset(&netmgr_qmi_dpm_state.close_req, 0, sizeof(netmgr_qmi_dpm_state.close_req));

  netmgr_log_med("netmgr_qmi_dpm_init(): qmi_client_init_instance SUCCESS, "
      "client: 0x%x\n", netmgr_qmi_dpm_state.dpm_clid);

  return NETMGR_SUCCESS;
}

/*===========================================================================
  FUNCTION:  netmgr_qmi_dpm_port_open
===========================================================================*/
/** @ingroup netmgr_dpm

    @brief
    Forms a dpm_open_port_req_msg_v01 message and sends to the modem.

    @details
    This method forms a dpm_open_port_req_msg_v01 and sends it to Q6 modem.

    This method needs to be sent over IPC router transport since QMUX would
    not be available during boot time. Once this message is received by Q6
    modem, it is going to open the ports on its ends and QMUXD boot-up is
    unblocked.

    The configuration used is from configdb XML file if it exists. Otherwise
    default hardcoded configuration is utilized.

    @params None.
    @return NETMGR_SUCCESS: in case of success.
    @return NETMGR_FAILURE: in case of error.

    @dependencies
    netmgr_qmi_dpm_init should be complete.
*/
int netmgr_qmi_dpm_port_open(void)
{
  dpm_open_port_req_msg_v01* open_req = NULL;
  dpm_open_port_resp_msg_v01 open_resp;
  int rc = NETMGR_FAILURE;
  qmi_client_error_type qmi_err = QMI_NO_ERR;

  netmgr_log_med("netmgr_qmi_dpm_port_open(): ENTRY\n");

#ifdef NETMGR_DPM_CONFIG_DB
  open_req = netmgr_qmi_dpm_port_open_config_db();
#else

  open_req = (dpm_open_port_req_msg_v01 *)
    netmgr_malloc(sizeof(dpm_open_port_req_msg_v01));
  if (NULL == open_req)
  {
    netmgr_log_err("failed to allocate memory for port_open_msg\n");
    goto bail;
  }

  if (netmgr_qmi_dpm_state.open_req_sent)
  {
    netmgr_log_med("Ports already opened, closing them first\n");
    if (NETMGR_SUCCESS != netmgr_qmi_dpm_port_close())
    {
      netmgr_log_med("Close port failed, exiting!\n");
      goto bail;
    }
    netmgr_qmi_dpm_state.open_req_sent = FALSE;
  }

  memset(open_req, 0, sizeof(dpm_open_port_req_msg_v01));
  open_req->control_port_list_valid = TRUE;
  open_req->control_port_list_len = 1;
  strlcpy (open_req->control_port_list[0].port_name, netmgr_main_cfg.smd_ch_name,QMI_DPM_PORT_NAME_MAX_V01);
  open_req->control_port_list[0].default_ep_id.ep_type = netmgr_main_cfg.ep_type;
  open_req->control_port_list[0].default_ep_id.iface_id = (uint32_t)netmgr_main_cfg.epid;

  open_req->hardware_data_port_list_valid = TRUE;
  open_req->hardware_data_port_list_len = 1;
  open_req->hardware_data_port_list[0].ep_id.ep_type =  netmgr_main_cfg.ep_type;
  open_req->hardware_data_port_list[0].ep_id.iface_id = (uint32_t)netmgr_main_cfg.epid;
  open_req->hardware_data_port_list[0].hardware_ep_pair.consumer_pipe_num = (uint32_t)netmgr_main_cfg.consumer_pipe_num;
  open_req->hardware_data_port_list[0].hardware_ep_pair.producer_pipe_num = (uint32_t)netmgr_main_cfg.producer_pipe_num;

  open_req->software_data_port_list_valid = FALSE;
#endif

  /* Send open_req */
  qmi_err = qmi_client_send_msg_sync(netmgr_qmi_dpm_state.dpm_clid,
                                     QMI_DPM_OPEN_PORT_REQ_V01,
                                     (void *)open_req,
                                     sizeof(dpm_open_port_req_msg_v01),
                                     (void*)&open_resp,
                                     sizeof(dpm_open_port_resp_msg_v01),
                                     NETMGR_QMI_DPM_TIMEOUT);
  if (QMI_NO_ERR != qmi_err)
  {
    netmgr_log_err("Send open_req failed, err %d\n", qmi_err);

    /* Open request failed */
    netmgr_qmi_dpm_state.open_req_sent = FALSE;
    goto bail;
  }

  /* Open request was sent successfully */
  netmgr_qmi_dpm_state.open_req_sent = TRUE;
  rc = NETMGR_SUCCESS;

bail:
  /* Store open_req for debug purposes */
  if (rc == NETMGR_SUCCESS && NULL != open_req)
  {
    netmgr_qmi_dpm_state.open_req = *open_req;
  }

  if (NULL != open_req)
  {
    netmgr_free(open_req);
  }

  /* Check whether open request was sent successfully before registering back transports
   * It is possible for modem to go down in between dpm_init and dpm_port_open and in that
   * case we cannot register back the transports because it will block the notifier thread
   * within qmi framework */
  if (TRUE == netmgr_qmi_dpm_state.open_req_sent)
  {
    netmgr_log_med("netmgr_qmi_dpm_init(): Open request successful,"
                   " registering back QMUX transports");
    qmi_cci_qmux_xport_register(QMI_CLIENT_QMUX_RMNET_INSTANCE_0);
    qmi_cci_qmux_xport_register(QMI_CLIENT_QMUX_RMNET_USB_INSTANCE_0);
    qmi_cci_qmux_xport_register(QMI_CLIENT_QMUX_RMNET_SMUX_INSTANCE_0);
    qmi_cci_qmux_xport_register(QMI_CLIENT_QMUX_RMNET_MHI_INSTANCE_0);
  }

  netmgr_log_med("netmgr_qmi_dpm_port_open(): EXIT, rc %d\n", rc);
  return rc;
}

/*===========================================================================
  FUNCTION:  netmgr_qmi_dpm_port_close
===========================================================================*/
/** @ingroup netmgr_dpm

    @brief
    Forms a dpm_close_port_req_msg_v01 message and sends to the modem.

    @details
    This method forms a dpm_close_port_req_msg_v01 and sends it to Q6 modem.
    Message is sent using IPC router transport.

    @params None.
    @return NETMGR_SUCCESS: in case of success.
    @return NETMGR_FAILURE: in case of error.

    @dependencies
    netmgr_qmi_dpm_init should be complete.
    netmgr_qmi_dpm_port_open should be complete, otherwise this is a NOP.
*/
int netmgr_qmi_dpm_port_close(void)
{
  netmgr_log_err("netmgr_qmi_dpm_port_close(): Not supported currently\n");
  return NETMGR_SUCCESS;
}

/*===========================================================================
  FUNCTION:  netmgr_qmi_dpm_log_state
===========================================================================*/
/** @ingroup netmgr_dpm

    @brief
    Prints debug messages about QMI DPM state in netmgr.

    @details
    Prints information on whether QMI DPM is inited, what was the last
    message sent etc.

    @params None.
    @return None.
*/
void netmgr_qmi_dpm_log_state(void)
{

  dpm_open_port_req_msg_v01  *open_req;
  unsigned int i;

  netmgr_log_med("====netmgr_qmi_dpm_log_state====\n");
  netmgr_log_med("....use_config_db: %d\n", netmgr_qmi_dpm_state.use_configdb);
  netmgr_log_med("....configdb_inited: %d\n", netmgr_qmi_dpm_state.configdb_inited);
  netmgr_log_med("....dpm_clid: %d\n", netmgr_qmi_dpm_state.dpm_clid);
  netmgr_log_med("....dpm_inited: %d\n", netmgr_qmi_dpm_state.dpm_inited);
  netmgr_log_med("....close_req: 0x%x\n", netmgr_qmi_dpm_state.close_req);
  netmgr_log_med("....open_req_sent: %d\n", netmgr_qmi_dpm_state.open_req_sent);
  netmgr_log_med("....open_req:\n");

  open_req = &netmgr_qmi_dpm_state.open_req;
  netmgr_log_med("......control_port_list_valid: %d\n", open_req->control_port_list_valid);
  netmgr_log_med("......control_port_list_len: %d\n", open_req->control_port_list_len);
  netmgr_log_med("......control_port_list:\n");
  for (i = 0; i < open_req->control_port_list_len && i < QMI_DPM_PORT_MAX_NUM_V01; i++)
  {
    netmgr_log_med("........control_port_list[%d].port_name: %s\n", i, open_req->control_port_list[i].port_name);
    netmgr_log_med("........control_port_list[%d].default_ep_id.ep_type: %d\n", i, open_req->control_port_list[i].default_ep_id.ep_type);
    netmgr_log_med("........control_port_list[%d].default_ep_id.iface_id: 0x%x\n", i, open_req->control_port_list[i].default_ep_id.iface_id);
  }

  netmgr_log_med("......hardware_data_port_list_valid: %d\n", open_req->hardware_data_port_list_valid);
  netmgr_log_med("......hardware_data_port_list_len: %d\n", open_req->hardware_data_port_list_len);
  for (i = 0; i < open_req->hardware_data_port_list_len && i < QMI_DPM_PORT_MAX_NUM_V01; i++)
  {
    netmgr_log_med("........hardware_data_port_list[%d].ep_id.ep_type: %d\n", i, open_req->hardware_data_port_list[i].ep_id.ep_type);
    netmgr_log_med("........hardware_data_port_list[%d].ep_id.iface_id: 0x%x\n", i, open_req->hardware_data_port_list[i].ep_id.iface_id);
    netmgr_log_med("........hardware_data_port_list[%d].hardware_ep_pair.consumer_pipe_num: %d\n", i, open_req->hardware_data_port_list[i].hardware_ep_pair.consumer_pipe_num);
    netmgr_log_med("........hardware_data_port_list[%d].hardware_ep_pair.producer_pipe_num: %d\n", i, open_req->hardware_data_port_list[i].hardware_ep_pair.producer_pipe_num);
  }

  netmgr_log_med("......software_data_port_list_valid: %d\n", open_req->software_data_port_list_valid);
  netmgr_log_med("====netmgr_qmi_dmp_log_state====\n");
}

