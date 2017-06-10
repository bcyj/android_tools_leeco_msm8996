/******************************************************************************

                        D S I _ N E T C T R L _ T E S T . H

******************************************************************************/

/******************************************************************************

  @file    dsi_netctrl_test.j
  @brief   dsi_netctrl test header

  DESCRIPTION
  Common header between dsi_netctrl test daemon and client.

  ---------------------------------------------------------------------------
  Copyright (c) 2010 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------

******************************************************************************/

/******************************************************************************

                      EDIT HISTORY FOR FILE

  $Id: $

when       who        what, where, why
--------   ---        -------------------------------------------------------
10/07/10   jf         created

******************************************************************************/

#ifndef DSI_NETCTRL_TEST_H
#define DSI_NETCTRL_TEST_H

#include "qdp.h"
#include "dsi_netctrl.h"

#define DSI_NETCTRL_TEST_PARAM_MAX_LENGTH 128
#define DSI_NETCTRL_TEST_NAME_MAX_LENGTH 32
#define DSI_NETCTRL_TEST_SOCKET "/data/data_test/dsi_netctrl_test_socket"

typedef enum
{
  DSI_TEST_CALL_STATUS_IDLE,
  DSI_TEST_CALL_STATUS_CONNECTING,
  DSI_TEST_CALL_STATUS_CONNECTED,
  DSI_TEST_CALL_STATUS_DISCONNECTING
} dsi_test_call_status_e;

typedef enum
{
  DSI_NETCTRL_TEST_CMD_STOP_SERVER,
  DSI_NETCTRL_TEST_CMD_CREATE_CALL,
  DSI_NETCTRL_TEST_CMD_RELEASE_CALL,
  DSI_NETCTRL_TEST_CMD_SET_CALL_PARAM,
  DSI_NETCTRL_TEST_CMD_NET_UP,
  DSI_NETCTRL_TEST_CMD_NET_DOWN,
  DSI_NETCTRL_TEST_CMD_LIST_CALLS,
  DSI_NETCTRL_TEST_CMD_GET_DEVICE,
  DSI_NETCTRL_TEST_CMD_GET_TECH,
  DSI_NETCTRL_TEST_CMD_PROFILE_INIT,
  DSI_NETCTRL_TEST_CMD_PROFILE_DEINIT,
  DSI_NETCTRL_TEST_CMD_CREATE_PROFILE,
  DSI_NETCTRL_TEST_CMD_RELEASE_PROFILE,
  DSI_NETCTRL_TEST_CMD_SET_PROFILE_PARAM,
  DSI_NETCTRL_TEST_CMD_LOOKUP_PROFILE,
  DSI_NETCTRL_TEST_CMD_QOS_REQUEST,
  DSI_NETCTRL_TEST_CMD_QOS_RELEASE,
  DSI_NETCTRL_TEST_CMD_QOS_SUSPEND,
  DSI_NETCTRL_TEST_CMD_QOS_RESUME,
  DSI_NETCTRL_TEST_CMD_QOS_MODIFY,
  DSI_NETCTRL_TEST_CMD_QOS_GETGRANTED
} dsi_netctrl_test_cmd;

typedef struct
{
  dsi_netctrl_test_cmd cmd;
  char name[DSI_NETCTRL_TEST_NAME_MAX_LENGTH];

  union
  {
    struct
    {
      dsi_call_param_identifier_t identifier;
      int num_val;
      char buf_val[DSI_NETCTRL_TEST_PARAM_MAX_LENGTH];
    } call_param;

    struct
    {
      qdp_ril_param_idx_t identifier;
      char val[DSI_NETCTRL_TEST_PARAM_MAX_LENGTH];
    } profile_param;
    
    struct
    {
      dsi_qos_id_type   flowid;
      struct
      {
        qmi_qos_flow_req_type    tx_flow_req;
        qmi_qos_flow_req_type    rx_flow_req;
        qmi_qos_filter_req_type  tx_filter_req;
        qmi_qos_filter_req_type  rx_filter_req;
      } spec;
    } qos_param;
  };

} dsi_netctrl_test_cmd_info;

#endif //DSI_NETCTRL_TEST_H
