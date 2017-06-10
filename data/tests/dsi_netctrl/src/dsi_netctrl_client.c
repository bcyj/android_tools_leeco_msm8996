/******************************************************************************

                    D S I _ N E T C T R L _ C L I E N T . C

******************************************************************************/

/******************************************************************************

  @file    dsi_netctrl_client.c
  @brief   dsi_netctrl test client application

  DESCRIPTION
  Client application that allows making data calls using dsi_netctrl

  ---------------------------------------------------------------------------
  Copyright (c) 2010,2013-2014 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------

******************************************************************************/

/******************************************************************************

                      EDIT HISTORY FOR FILE

  $Id: $

when       who        what, where, why
--------   ---        -------------------------------------------------------
09/20/10   jf         created

******************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "dsi_netctrl_test.h"
#include "ds_string.h"

#define SUCCESS (0)
#define FAILURE (-1)
#define ASSERT_USAGE(x) if (!(x)) goto usage

/* Server control commands */
#define DSI_NETCTRL_TEST_SERVER_START "start"
#define DSI_NETCTRL_TEST_SERVER_STOP "stop"

/* data call commands */
#define DSI_NETCTRL_TEST_CALL_CREATE "create"
#define DSI_NETCTRL_TEST_CALL_RELEASE "release"
#define DSI_NETCTRL_TEST_CALL_UP "up"
#define DSI_NETCTRL_TEST_CALL_DOWN "down"
#define DSI_NETCTRL_TEST_CALL_DEVICE "device"
#define DSI_NETCTRL_TEST_CALL_GET_TECH "tech"
#define DSI_NETCTRL_TEST_CALL_PARAM "param"

/* list all data call handles */
#define DSI_NETCTRL_TEST_LIST "list"

/* parameters that can be set with DSI_NETCTRL_TEST_CALL_PARAM command */
#define DSI_NETCTRL_TEST_PARAM_UMTS_PROFILE "umts_profile"
#define DSI_NETCTRL_TEST_PARAM_APN "apn"
#define DSI_NETCTRL_TEST_PARAM_USERNAME "username"
#define DSI_NETCTRL_TEST_PARAM_PASSWORD "password"
#define DSI_NETCTRL_TEST_PARAM_AUTH "auth"
#define DSI_NETCTRL_TEST_PARAM_CDMA_PROFILE "cdma_profile"
#define DSI_NETCTRL_TEST_PARAM_IP_ADDR "ip_addr"
#define DSI_NETCTRL_TEST_PARAM_DEVICE_NAME "device_name"
#define DSI_NETCTRL_TEST_PARAM_IPV "ipv"
#define DSI_NETCTRL_TEST_PARAM_TECH "tech"
#define DSI_NETCTRL_TEST_PARAM_CALL_TYPE "call_type"

/* values for DSI_NETCTRL_TEST_PARAM_TECH set parameter operation */
#define DSI_NETCTRL_TEST_TECH_UMTS "umts"
#define DSI_NETCTRL_TEST_TECH_CDMA "cdma"
#define DSI_NETCTRL_TEST_TECH_1X "1x"
#define DSI_NETCTRL_TEST_TECH_EVDO "evdo"
#define DSI_NETCTRL_TEST_TECH_LTE "lte"

/* values for DSI_NETCTRL_TEST_PARAM_CALL_TYPE set parameter operation */
#define DSI_NETCTRL_TEST_CALL_TYPE_TETHERED "tethered"
#define DSI_NETCTRL_TEST_CALL_TYPE_EMBEDDED "embedded"

/* profile related commands */
#define DSI_NETCTRL_TEST_PROFILE "profile"
#define DSI_NETCTRL_TEST_PROFILE_INIT "init"
#define DSI_NETCTRL_TEST_PROFILE_DEINIT "deinit"
#define DSI_NETCTRL_TEST_PROFILE_CREATE "create"
#define DSI_NETCTRL_TEST_PROFILE_RELEASE "release"
#define DSI_NETCTRL_TEST_PROFILE_SET "set"
#define DSI_NETCTRL_TEST_PROFILE_LOOKUP "lookup"

/* profile parameter names that can be set */
#define DSI_NETCTRL_TEST_PROFILE_SET_TECH "tech"
#define DSI_NETCTRL_TEST_PROFILE_SET_ID "id"
#define DSI_NETCTRL_TEST_PROFILE_SET_APN "apn"
#define DSI_NETCTRL_TEST_PROFILE_SET_NAI "nai"
#define DSI_NETCTRL_TEST_PROFILE_SET_PASSWORD "password"
#define DSI_NETCTRL_TEST_PROFILE_SET_AUTH "auth"
#define DSI_NETCTRL_TEST_PROFILE_SET_IP "ip"

/* QOS related commands */
#define DSI_NETCTRL_TEST_QOS_REQUEST      "qosrequest"
#define DSI_NETCTRL_TEST_QOS_RELEASE      "qosrelease"
#define DSI_NETCTRL_TEST_QOS_SUSPEND      "qossuspend"
#define DSI_NETCTRL_TEST_QOS_RESUME       "qosresume"
#define DSI_NETCTRL_TEST_QOS_MODIFY       "qosmodify"
#define DSI_NETCTRL_TEST_QOS_GETGRANTED   "qosgetgranted"

/* parameters that can be set with QOS commands */
#define DSI_NETCTRL_TEST_QOS_FLOWID        "flowid"
#define DSI_NETCTRL_TEST_QOS_DIRECTION     "dir"
#define DSI_NETCTRL_TEST_QOS_DATARATE      "datarate"
#define DSI_NETCTRL_TEST_QOS_TRAFFIC_CLASS "tclass"
#define DSI_NETCTRL_TEST_QOS_PROFILEID     "profile"
#define DSI_NETCTRL_TEST_QOS_IPV4_SRCADDR  "srcaddrv4"
#define DSI_NETCTRL_TEST_QOS_IPV4_DSTADDR  "dstaddrv4"
#define DSI_NETCTRL_TEST_QOS_TCP_SRCPORT   "tcpsrcport"
#define DSI_NETCTRL_TEST_QOS_UDP_SRCPORT   "udpsrcport"
#define DSI_NETCTRL_TEST_QOS_TCP_DSTPORT   "tcpdstport"
#define DSI_NETCTRL_TEST_QOS_UDP_DSTPORT   "udpdstport"

#define DSI_NETCTRL_TEST_QOS_DIRECTION_TX  "tx"
#define DSI_NETCTRL_TEST_QOS_DIRECTION_RX  "rx"

int send_command( dsi_netctrl_test_cmd_info *cmd,
                  char *rsp, int rsp_len, int *out_len )
{
  int sockfd;
  struct sockaddr_un serv_addr;
  int servlen;
  int length = 0;
  struct sockaddr_un addr;
  struct sockaddr_un* __attribute__((__may_alias__)) addr_ptr;

  memset(&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sun_family = AF_UNIX;
  strlcpy(serv_addr.sun_path, DSI_NETCTRL_TEST_SOCKET, sizeof(serv_addr.sun_path));
  servlen = (int)(std_strlen(serv_addr.sun_path) + sizeof(serv_addr.sun_family));

  /* Send command to daemon */
  sockfd = socket(AF_UNIX, SOCK_STREAM, 0);

  if (sockfd < 0)
  {
    fprintf(stderr, "error creating socket\n");
    return FAILURE;
  }

  memcpy(&addr, &serv_addr, sizeof(addr));
  addr_ptr = &addr;
  if (connect(sockfd, (struct sockaddr*)addr_ptr, servlen) < 0)
  {
    fprintf(stderr, "error connecting socket\n");
    return FAILURE;
  }

  write(sockfd, cmd, sizeof(*cmd));

  memset( rsp, 0x0, (size_t)rsp_len );
  length = (int)read(sockfd, rsp, (size_t)(rsp_len-1));

  close(sockfd);

  if (length < 0)
  {
    fprintf(stderr, "read() failed\n");
    return FAILURE;
  }

  *out_len = length;
  return SUCCESS;
}


int main(int argc, char** argv)
{
  char** args;
  char* end;
  dsi_netctrl_test_cmd_info cmd;

  char buf[256];
  int length = 0;

  ASSERT_USAGE(argc > 1);

  memset(&cmd, 0x0, sizeof(cmd));
  args = argv + 1;

  /* Prepare command buffer */
  if (std_stricmp(DSI_NETCTRL_TEST_CALL_UP, *args) == 0)
  {
    ASSERT_USAGE(argc == 3);
    args++;

    strlcpy(cmd.name, *args, DSI_NETCTRL_TEST_NAME_MAX_LENGTH);
    cmd.name[DSI_NETCTRL_TEST_NAME_MAX_LENGTH-1] = '\0';
    cmd.cmd = DSI_NETCTRL_TEST_CMD_NET_UP;
  }
  else if (std_stricmp(DSI_NETCTRL_TEST_CALL_DOWN, *args) == 0)
  {
    ASSERT_USAGE(argc == 3);
    args++;

    strlcpy(cmd.name, *args, DSI_NETCTRL_TEST_NAME_MAX_LENGTH);
    cmd.name[DSI_NETCTRL_TEST_NAME_MAX_LENGTH-1] = '\0';
    cmd.cmd = DSI_NETCTRL_TEST_CMD_NET_DOWN;
  }
  else if (std_stricmp(DSI_NETCTRL_TEST_CALL_CREATE, *args) == 0)
  {
    ASSERT_USAGE(argc == 3);
    args++;

    strlcpy(cmd.name, *args, DSI_NETCTRL_TEST_NAME_MAX_LENGTH);
    cmd.name[DSI_NETCTRL_TEST_NAME_MAX_LENGTH-1] = '\0';
    cmd.cmd = DSI_NETCTRL_TEST_CMD_CREATE_CALL;
  }
  else if (std_stricmp(DSI_NETCTRL_TEST_CALL_RELEASE, *args) == 0)
  {
    ASSERT_USAGE(argc == 3);
    args++;

    strlcpy(cmd.name, *args, DSI_NETCTRL_TEST_NAME_MAX_LENGTH);
    cmd.name[DSI_NETCTRL_TEST_NAME_MAX_LENGTH-1] = '\0';
    cmd.cmd = DSI_NETCTRL_TEST_CMD_RELEASE_CALL;
  }
  else if (std_stricmp(DSI_NETCTRL_TEST_CALL_DEVICE, *args) == 0)
  {
    ASSERT_USAGE(argc == 3);
    args++;

    strlcpy(cmd.name, *args, DSI_NETCTRL_TEST_NAME_MAX_LENGTH);
    cmd.name[DSI_NETCTRL_TEST_NAME_MAX_LENGTH-1] = '\0';
    cmd.cmd = DSI_NETCTRL_TEST_CMD_GET_DEVICE;
  }
  else if (std_stricmp(DSI_NETCTRL_TEST_CALL_PARAM, *args) == 0)
  {
    ASSERT_USAGE(argc == 5);
    args++;

    strlcpy(cmd.name, *args, DSI_NETCTRL_TEST_NAME_MAX_LENGTH);
    cmd.name[DSI_NETCTRL_TEST_NAME_MAX_LENGTH-1] = '\0';

    args++;

    if (std_stricmp(DSI_NETCTRL_TEST_PARAM_UMTS_PROFILE, *args) == 0)
    {
      cmd.call_param.identifier = DSI_CALL_INFO_UMTS_PROFILE_IDX;
    }
    else if (std_stricmp(DSI_NETCTRL_TEST_PARAM_APN, *args) == 0)
    {
      cmd.call_param.identifier = DSI_CALL_INFO_APN_NAME;
    }
    else if (std_stricmp(DSI_NETCTRL_TEST_PARAM_USERNAME, *args) == 0)
    {
      cmd.call_param.identifier = DSI_CALL_INFO_USERNAME;
    }
    else if (std_stricmp(DSI_NETCTRL_TEST_PARAM_PASSWORD, *args) == 0)
    {
      cmd.call_param.identifier = DSI_CALL_INFO_PASSWORD;
    }
    else if (std_stricmp(DSI_NETCTRL_TEST_PARAM_AUTH, *args) == 0)
    {
      cmd.call_param.identifier = DSI_CALL_INFO_AUTH_PREF;
    }
    else if (std_stricmp(DSI_NETCTRL_TEST_PARAM_CDMA_PROFILE, *args) == 0)
    {
      cmd.call_param.identifier = DSI_CALL_INFO_CDMA_PROFILE_IDX;
    }
    else if (std_stricmp(DSI_NETCTRL_TEST_PARAM_IP_ADDR, *args) == 0)
    {
      fprintf(stderr, DSI_NETCTRL_TEST_PARAM_IP_ADDR " not implemented\n\n");
      goto usage;
    }
    else if (std_stricmp(DSI_NETCTRL_TEST_PARAM_DEVICE_NAME, *args) == 0)
    {
      fprintf(stderr, DSI_NETCTRL_TEST_PARAM_DEVICE_NAME " not implemented\n\n");
      goto usage;
    }
    else if (std_stricmp(DSI_NETCTRL_TEST_PARAM_TECH, *args) == 0)
    {
      cmd.call_param.identifier = DSI_CALL_INFO_TECH_PREF;

      args++;

      if (std_stricmp(DSI_NETCTRL_TEST_TECH_UMTS, *args) == 0)
      {
        cmd.call_param.num_val = DSI_RADIO_TECH_UMTS;
      }
      else if (std_stricmp(DSI_NETCTRL_TEST_TECH_CDMA, *args) == 0)
      {
        cmd.call_param.num_val = DSI_RADIO_TECH_CDMA;
      }
      else if (std_stricmp(DSI_NETCTRL_TEST_TECH_1X, *args) == 0)
      {
        cmd.call_param.num_val = DSI_RADIO_TECH_1X;
      }
      else if (std_stricmp(DSI_NETCTRL_TEST_TECH_EVDO, *args) == 0)
      {
        cmd.call_param.num_val = DSI_RADIO_TECH_DO;
      }
      else if (std_stricmp(DSI_NETCTRL_TEST_TECH_LTE, *args) == 0)
      {
        cmd.call_param.num_val = DSI_RADIO_TECH_LTE;
      }
      else
      {
        goto usage;
      }
    }
    else if (std_stricmp(DSI_NETCTRL_TEST_PARAM_CALL_TYPE, *args) == 0)
    {
      cmd.call_param.identifier = DSI_CALL_INFO_CALL_TYPE;

      args++;

      if (std_stricmp(DSI_NETCTRL_TEST_CALL_TYPE_TETHERED, *args) == 0)
      {
        cmd.call_param.num_val = DSI_CALL_TYPE_TETHERED;
      }
      else if (std_stricmp(DSI_NETCTRL_TEST_CALL_TYPE_EMBEDDED, *args) == 0)
      {
        cmd.call_param.num_val = DSI_CALL_TYPE_EMBEDDED;
      }
      else
      {
        goto usage;
      }
    }
    else if (std_stricmp(DSI_NETCTRL_TEST_PARAM_IPV, *args) == 0)
    {
      cmd.call_param.identifier = DSI_CALL_INFO_IP_VERSION;
    }
    else
    {
      goto usage;
    }

    args++;

    switch (cmd.call_param.identifier)
    {
    /* numeric parameters */
    case DSI_CALL_INFO_UMTS_PROFILE_IDX:
    case DSI_CALL_INFO_CDMA_PROFILE_IDX:
    case DSI_CALL_INFO_AUTH_PREF:
    case DSI_CALL_INFO_IP_VERSION:
      cmd.call_param.num_val = (int)strtol(*args, &end, 10);
      if (*end != '\0')
        goto usage;
      break;

    /* string parameters */
    case DSI_CALL_INFO_APN_NAME:
    case DSI_CALL_INFO_USERNAME:
    case DSI_CALL_INFO_PASSWORD:
      strlcpy(cmd.call_param.buf_val, *args, DSI_NETCTRL_TEST_PARAM_MAX_LENGTH);
      cmd.call_param.num_val = (int)std_strlen(*args);
      break;

    /* already parsed */
    case DSI_CALL_INFO_TECH_PREF:
    case DSI_CALL_INFO_CALL_TYPE:
      break;

    default:
      goto usage;
    }

    cmd.cmd = DSI_NETCTRL_TEST_CMD_SET_CALL_PARAM;
  }
  else if (std_stricmp(DSI_NETCTRL_TEST_LIST, *args) == 0)
  {
    cmd.cmd = DSI_NETCTRL_TEST_CMD_LIST_CALLS;
  }
  else if (std_stricmp(DSI_NETCTRL_TEST_CALL_GET_TECH, *args) == 0)
  {
    ASSERT_USAGE(argc == 3);
    args++;

    strlcpy(cmd.name, *args, DSI_NETCTRL_TEST_NAME_MAX_LENGTH);
    cmd.name[DSI_NETCTRL_TEST_NAME_MAX_LENGTH-1] = '\0';

    cmd.cmd = DSI_NETCTRL_TEST_CMD_GET_TECH;
  }
  else if (std_stricmp(DSI_NETCTRL_TEST_SERVER_STOP, *args) == 0)
  {
    cmd.cmd = DSI_NETCTRL_TEST_CMD_STOP_SERVER;
  }
  else if (std_stricmp(DSI_NETCTRL_TEST_SERVER_START, *args) == 0)
  {
    if (system("/data/data_test/dsi_netctrl_daemon") == 0)
    {
      return 0;
    }
    else
    {
      fprintf(stderr, "failed to start daemon\n");
      exit(EXIT_FAILURE);
    }
  }
  else if (std_stricmp(DSI_NETCTRL_TEST_PROFILE, *args) == 0)
  {
    ASSERT_USAGE(argc >= 3);
    args++;

    if (std_stricmp(DSI_NETCTRL_TEST_PROFILE_INIT, *args) == 0)
    {
      ASSERT_USAGE(argc == 4);

      args++;
      strlcpy(cmd.name, *args, DSI_NETCTRL_TEST_NAME_MAX_LENGTH);
      cmd.name[DSI_NETCTRL_TEST_NAME_MAX_LENGTH-1] = '\0';

      cmd.cmd = DSI_NETCTRL_TEST_CMD_PROFILE_INIT;
    }
    else if (std_stricmp(DSI_NETCTRL_TEST_PROFILE_DEINIT, *args) == 0)
    {
      ASSERT_USAGE(argc == 3);
      cmd.cmd = DSI_NETCTRL_TEST_CMD_PROFILE_DEINIT;
    }
    else if (std_stricmp(DSI_NETCTRL_TEST_PROFILE_CREATE, *args) == 0)
    {
      ASSERT_USAGE(argc == 4);

      args++;
      strlcpy(cmd.name, *args, DSI_NETCTRL_TEST_NAME_MAX_LENGTH);
      cmd.name[DSI_NETCTRL_TEST_NAME_MAX_LENGTH-1] = '\0';

      cmd.cmd = DSI_NETCTRL_TEST_CMD_CREATE_PROFILE;
    }
    else if (std_stricmp(DSI_NETCTRL_TEST_PROFILE_RELEASE, *args) == 0)
    {
      ASSERT_USAGE(argc == 4);

      args++;
      strlcpy(cmd.name, *args, DSI_NETCTRL_TEST_NAME_MAX_LENGTH);
      cmd.name[DSI_NETCTRL_TEST_NAME_MAX_LENGTH-1] = '\0';

      cmd.cmd = DSI_NETCTRL_TEST_CMD_RELEASE_PROFILE;
    }
    else if (std_stricmp(DSI_NETCTRL_TEST_PROFILE_LOOKUP, *args) == 0)
    {
      ASSERT_USAGE(argc == 4);

      args++;
      strlcpy(cmd.name, *args, DSI_NETCTRL_TEST_NAME_MAX_LENGTH);
      cmd.name[DSI_NETCTRL_TEST_NAME_MAX_LENGTH-1] = '\0';

      cmd.cmd = DSI_NETCTRL_TEST_CMD_LOOKUP_PROFILE;
    }
    else if (std_stricmp(DSI_NETCTRL_TEST_PROFILE_SET, *args) == 0)
    {
      ASSERT_USAGE(argc == 6);

      args++;
      strlcpy(cmd.name, *args, DSI_NETCTRL_TEST_NAME_MAX_LENGTH);
      cmd.name[DSI_NETCTRL_TEST_NAME_MAX_LENGTH-1] = '\0';

      args++;
      if (std_stricmp(DSI_NETCTRL_TEST_PROFILE_SET_TECH, *args) == 0)
      {
        cmd.profile_param.identifier = QDP_RIL_TECH;

        args++;
        strlcpy(cmd.profile_param.val, *args, DSI_NETCTRL_TEST_PARAM_MAX_LENGTH);
        cmd.profile_param.val[DSI_NETCTRL_TEST_PARAM_MAX_LENGTH-1] = '\0';
      }
      else if (std_stricmp(DSI_NETCTRL_TEST_PROFILE_SET_ID, *args) == 0)
      {
        cmd.profile_param.identifier = QDP_RIL_PROFILE_ID;

        args++;
        strlcpy(cmd.profile_param.val, *args, DSI_NETCTRL_TEST_PARAM_MAX_LENGTH);
        cmd.profile_param.val[DSI_NETCTRL_TEST_PARAM_MAX_LENGTH-1] = '\0';
      }
      else if (std_stricmp(DSI_NETCTRL_TEST_PROFILE_SET_APN, *args) == 0)
      {
        cmd.profile_param.identifier = QDP_RIL_APN;

        args++;
        strlcpy(cmd.profile_param.val, *args, DSI_NETCTRL_TEST_PARAM_MAX_LENGTH);
        cmd.profile_param.val[DSI_NETCTRL_TEST_PARAM_MAX_LENGTH-1] = '\0';
      }
      else if (std_stricmp(DSI_NETCTRL_TEST_PROFILE_SET_NAI, *args) == 0)
      {
        cmd.profile_param.identifier = QDP_RIL_NAI;

        args++;
        strlcpy(cmd.profile_param.val, *args, DSI_NETCTRL_TEST_PARAM_MAX_LENGTH);
        cmd.profile_param.val[DSI_NETCTRL_TEST_PARAM_MAX_LENGTH-1] = '\0';
      }
      else if (std_stricmp(DSI_NETCTRL_TEST_PROFILE_SET_PASSWORD, *args) == 0)
      {
        cmd.profile_param.identifier = QDP_RIL_PASSWORD;

        args++;
        strlcpy(cmd.profile_param.val, *args, DSI_NETCTRL_TEST_PARAM_MAX_LENGTH);
        cmd.profile_param.val[DSI_NETCTRL_TEST_PARAM_MAX_LENGTH-1] = '\0';
      }
      else if (std_stricmp(DSI_NETCTRL_TEST_PROFILE_SET_IP, *args) == 0)
      {
        cmd.profile_param.identifier = QDP_RIL_IP_FAMILY;

        args++;
        strlcpy(cmd.profile_param.val, *args, DSI_NETCTRL_TEST_PARAM_MAX_LENGTH);
        cmd.profile_param.val[DSI_NETCTRL_TEST_PARAM_MAX_LENGTH-1] = '\0';
      }

      cmd.cmd = DSI_NETCTRL_TEST_CMD_SET_PROFILE_PARAM;
    }
  }
  else if (std_stricmp(DSI_NETCTRL_TEST_QOS_REQUEST, *args) == 0)
  {
    qmi_qos_flow_req_type    *flow_ptr = NULL;
    qmi_qos_filter_req_type  *filter_ptr = NULL;

    ASSERT_USAGE(argc >= 4);

    /* Mandatory parameters */
    args++;
    strlcpy(cmd.name, *args, DSI_NETCTRL_TEST_NAME_MAX_LENGTH);
    cmd.name[DSI_NETCTRL_TEST_NAME_MAX_LENGTH-1] = '\0';

    args++;
    if (std_stricmp(DSI_NETCTRL_TEST_QOS_DIRECTION, *args) == 0)
    {
      args++;
      if (std_stricmp(DSI_NETCTRL_TEST_QOS_DIRECTION_TX, *args) == 0)
      {
	flow_ptr   = &cmd.qos_param.spec.tx_flow_req;
	filter_ptr = &cmd.qos_param.spec.tx_filter_req;
      }
      else if (std_stricmp(DSI_NETCTRL_TEST_QOS_DIRECTION_RX, *args) == 0)
      {
	flow_ptr   = &cmd.qos_param.spec.rx_flow_req;
	filter_ptr = &cmd.qos_param.spec.rx_filter_req;
      }
      else
      {
	fprintf(stderr, "Error: cannot decode direction {tx|rx}\n");
	exit(EXIT_FAILURE);
      }
    }
    else
    {
      fprintf(stderr, "Error: direction must be specified before other paramaters\n");
      exit(EXIT_FAILURE);
    }

    /* Optional parameters */
    args++;
    while( args )
    {
      if (std_stricmp(DSI_NETCTRL_TEST_QOS_DIRECTION, *args) == 0)
      {
	/* multiple direction tokens permitted */
	args++;
	if (std_stricmp(DSI_NETCTRL_TEST_QOS_DIRECTION_TX, *args) == 0)
	{
	  flow_ptr   = &cmd.qos_param.spec.tx_flow_req;
	  filter_ptr = &cmd.qos_param.spec.tx_filter_req;
	}
	else if (std_stricmp(DSI_NETCTRL_TEST_QOS_DIRECTION_RX, *args) == 0)
	{
	  flow_ptr   = &cmd.qos_param.spec.rx_flow_req;
	  filter_ptr = &cmd.qos_param.spec.rx_filter_req;
	}
	else
	{
	  fprintf(stderr, "Error: cannot decode direction {tx|rx}\n");
	  exit(EXIT_FAILURE);
	}
      }
      else if (std_stricmp(DSI_NETCTRL_TEST_QOS_DATARATE, *args) == 0)
      {
	args++;
        flow_ptr->umts_flow_desc.data_rate.guaranteed_rate = (long unsigned int)atoi( *args );
	args++;
        flow_ptr->umts_flow_desc.data_rate.max_rate = (long unsigned int)atoi( *args );
	flow_ptr->umts_flow_desc.param_mask |= QMI_QOS_UMTS_FLOW_PARAM_DATA_RATE;
      }
      else if (std_stricmp(DSI_NETCTRL_TEST_QOS_TRAFFIC_CLASS, *args) == 0)
      {
	args++;
        flow_ptr->umts_flow_desc.traffic_class = atoi( *args ); // TODO
	flow_ptr->umts_flow_desc.param_mask |= QMI_QOS_UMTS_FLOW_PARAM_TRAFFIC_CLASS;
      }
      else if (std_stricmp(DSI_NETCTRL_TEST_QOS_PROFILEID, *args) == 0)
      {
	args++;
        flow_ptr->cdma_flow_desc.profile_id = (long unsigned int)atoi( *args );
	flow_ptr->cdma_flow_desc.param_mask |= QMI_QOS_CDMA_FLOW_PARAM_PROFILE_ID;
      }
      else if (std_stricmp(DSI_NETCTRL_TEST_QOS_IPV4_SRCADDR, *args) == 0)
      {
	args++;
        filter_ptr->filter_desc.src_addr.ipv4_ip_addr = ntohl( 0 ); // TODO
	args++;
        filter_ptr->filter_desc.src_addr.ipv4_subnet_mask = ntohl( 0xFFFFFFFF ); // TODO
	filter_ptr->filter_desc.param_mask |= QMI_QOS_FILTER_PARAM_SRC_ADDR;
	filter_ptr->ip_version = QMI_QOS_IP_VERSION_4;
      }
      else if (std_stricmp(DSI_NETCTRL_TEST_QOS_IPV4_DSTADDR, *args) == 0)
      {
	args++;
        filter_ptr->filter_desc.dest_addr.ipv4_ip_addr = ntohl( 0 ); // TODO
	args++;
        filter_ptr->filter_desc.src_addr.ipv4_subnet_mask = ntohl( 0xFFFFFFFF ); // TODO
	filter_ptr->filter_desc.param_mask |= QMI_QOS_FILTER_PARAM_DEST_ADDR;
	filter_ptr->ip_version = QMI_QOS_IP_VERSION_4;
      }
      else if (std_stricmp(DSI_NETCTRL_TEST_QOS_TCP_SRCPORT, *args) == 0)
      {
	args++;
        filter_ptr->filter_desc.tcp_src_ports.start_port = (uint16_t)ntohl( 0 ); // TODO
	args++;
        filter_ptr->filter_desc.tcp_src_ports.range = (uint16_t)ntohl( 1 ); // TODO
	filter_ptr->filter_desc.param_mask |= QMI_QOS_FILTER_PARAM_TCP_SRC_PORTS;
      }
      else if (std_stricmp(DSI_NETCTRL_TEST_QOS_TCP_DSTPORT, *args) == 0)
      {
	args++;
        filter_ptr->filter_desc.tcp_dest_ports.start_port = (uint16_t)ntohl( 0 ); // TODO
	args++;
        filter_ptr->filter_desc.tcp_dest_ports.range = (uint16_t)ntohl( 1 ); // TODO
	filter_ptr->filter_desc.param_mask |= QMI_QOS_FILTER_PARAM_TCP_DEST_PORTS;
      }
      else if (std_stricmp(DSI_NETCTRL_TEST_QOS_UDP_SRCPORT, *args) == 0)
      {
	args++;
        filter_ptr->filter_desc.udp_src_ports.start_port = (uint16_t)ntohl( 0 ); // TODO
	args++;
        filter_ptr->filter_desc.udp_src_ports.range = (uint16_t)ntohl( 1 ); // TODO
	filter_ptr->filter_desc.param_mask |= QMI_QOS_FILTER_PARAM_UDP_SRC_PORTS;
      }
      else if (std_stricmp(DSI_NETCTRL_TEST_QOS_UDP_DSTPORT, *args) == 0)
      {
	args++;
        filter_ptr->filter_desc.udp_dest_ports.start_port = (uint16_t)ntohl( 0 ); // TODO
	args++;
        filter_ptr->filter_desc.udp_dest_ports.range = (uint16_t)ntohl( 1 ); // TODO
	filter_ptr->filter_desc.param_mask |= QMI_QOS_FILTER_PARAM_UDP_DEST_PORTS;
      }
      else
      {
        fprintf(stderr, "Warning: unrecognized paramater, ignoring [%s]\n", *args);
      }
      args++;
    }

    cmd.cmd = DSI_NETCTRL_TEST_CMD_QOS_REQUEST;
  }
  else
  {
    fprintf(stderr, "Unrecognized command %s\n\n", *args);
    goto usage;
  }

  
  /* Exchange command/response with daemon */
  if( SUCCESS != send_command( &cmd, buf, sizeof(buf), &length ) )
  {
    exit(EXIT_FAILURE); 
  }

  /* Process response buffer */
  switch (cmd.cmd)
  {
    case DSI_NETCTRL_TEST_CMD_STOP_SERVER:
    case DSI_NETCTRL_TEST_CMD_CREATE_CALL:
    case DSI_NETCTRL_TEST_CMD_RELEASE_CALL:
    case DSI_NETCTRL_TEST_CMD_SET_CALL_PARAM:
    case DSI_NETCTRL_TEST_CMD_NET_UP:
    case DSI_NETCTRL_TEST_CMD_NET_DOWN:
    case DSI_NETCTRL_TEST_CMD_PROFILE_INIT:
    case DSI_NETCTRL_TEST_CMD_PROFILE_DEINIT:
    case DSI_NETCTRL_TEST_CMD_CREATE_PROFILE:
    case DSI_NETCTRL_TEST_CMD_RELEASE_PROFILE:
    case DSI_NETCTRL_TEST_CMD_SET_PROFILE_PARAM:
      /* data is error message */
      if (length > 0)
      {
	fprintf(stderr, "%s", buf);
	exit(EXIT_FAILURE);
      }
      break;

    case DSI_NETCTRL_TEST_CMD_LIST_CALLS:
    case DSI_NETCTRL_TEST_CMD_GET_DEVICE:
    case DSI_NETCTRL_TEST_CMD_GET_TECH:
    case DSI_NETCTRL_TEST_CMD_LOOKUP_PROFILE:
      /* no data indicates error */
      if (length == 0)
      {
	fprintf(stderr, "Operation failed.\n");
	exit(EXIT_FAILURE);
      }
      else
      {
	printf("%s", buf);
      }
      break;
      
    default:
      break;
  }

  return 0;

usage:
  fprintf(stderr, "usage: %s <command>\n", argv[0]);
  fprintf(stderr, "where <command> is:\n"
          "\t\"" DSI_NETCTRL_TEST_SERVER_START "\" - start the test daemon\n"
          "\t\"" DSI_NETCTRL_TEST_SERVER_STOP "\"  - stop the test daemon\n"
          "\t\"" DSI_NETCTRL_TEST_LIST "\" - list all calls and their state\n"
          "\t\"" DSI_NETCTRL_TEST_CALL_CREATE " <name>\" - create a call with name <name>\n"
          "\t\"" DSI_NETCTRL_TEST_CALL_RELEASE " <name>\" - release call with name <name>\n"
          "\t\"" DSI_NETCTRL_TEST_CALL_UP " <name>\" - bring up data call with name <name>\n"
          "\t\"" DSI_NETCTRL_TEST_CALL_DOWN " <name>\" - bring down data call with name <name>\n"
          "\t\"" DSI_NETCTRL_TEST_CALL_DEVICE " <name>\" - print name of interface for call <name>\n"
          "\t\"" DSI_NETCTRL_TEST_CALL_PARAM " <name> <type> <value>\" - set call parameter\n\n"
          "\t\"" DSI_NETCTRL_TEST_QOS_REQUEST " <value-set>\" - request QOS flow/filter\n\n"
          "\t\"" DSI_NETCTRL_TEST_QOS_RELEASE " <value>\" - release QOS flow\n\n"
          "\t\"" DSI_NETCTRL_TEST_QOS_SUSPEND " <value>\" - suspend QOS flow\n\n"
          "\t\"" DSI_NETCTRL_TEST_QOS_RESUME " <value>\" - resume QOS flow\n\n"
          "\t\"" DSI_NETCTRL_TEST_QOS_MODIFY " <value-set>\" - modify QOS flow\n\n"
          "\t\"" DSI_NETCTRL_TEST_QOS_GETGRANTED " <value>\" - query QOS flow\n\n"
          "Possible values for set call parameter <type> <value> pairs are:\n"
          "\t" DSI_NETCTRL_TEST_PARAM_UMTS_PROFILE " <integer>\n"
          "\t" DSI_NETCTRL_TEST_PARAM_CDMA_PROFILE " <integer>\n"
          "\t" DSI_NETCTRL_TEST_PARAM_APN " <string>\n"
          "\t" DSI_NETCTRL_TEST_PARAM_USERNAME " <string>\n"
          "\t" DSI_NETCTRL_TEST_PARAM_PASSWORD " <string>\n"
          "\t" DSI_NETCTRL_TEST_PARAM_AUTH " <integer>\n"
          "\t" DSI_NETCTRL_TEST_PARAM_TECH " ("
            DSI_NETCTRL_TEST_TECH_UMTS " | "
            DSI_NETCTRL_TEST_TECH_CDMA " | "
            DSI_NETCTRL_TEST_TECH_1X   " | "
            DSI_NETCTRL_TEST_TECH_EVDO " | "
            DSI_NETCTRL_TEST_TECH_LTE  ")\n"
          "\t" DSI_NETCTRL_TEST_PARAM_CALL_TYPE " ("
            DSI_NETCTRL_TEST_CALL_TYPE_EMBEDDED " | "
            DSI_NETCTRL_TEST_CALL_TYPE_TETHERED ")\n"
          "\t" DSI_NETCTRL_TEST_PARAM_IPV " <integer>\n"
          "Possible values for QOS request <type> <value> pairs are:\n"
          "\t" DSI_NETCTRL_TEST_QOS_DIRECTION " [" DSI_NETCTRL_TEST_QOS_DIRECTION_TX "|" DSI_NETCTRL_TEST_QOS_DIRECTION_RX "]\n"
          "\t" DSI_NETCTRL_TEST_QOS_DATARATE " <integer> <integer>\n"
          "\t" DSI_NETCTRL_TEST_QOS_TRAFFIC_CLASS " <integer>\n"
          "\t" DSI_NETCTRL_TEST_QOS_PROFILEID " <integer>\n"
          "\t" DSI_NETCTRL_TEST_QOS_IPV4_SRCADDR " <address> <mask>\n"
          "\t" DSI_NETCTRL_TEST_QOS_IPV4_DSTADDR " <address> <mask>\n"
          "\t" DSI_NETCTRL_TEST_QOS_TCP_SRCPORT " <integer> <integer>\n"
          "\t" DSI_NETCTRL_TEST_QOS_TCP_DSTPORT " <integer> <integer>\n"
          "\t" DSI_NETCTRL_TEST_QOS_UDP_SRCPORT " <integer> <integer>\n"
          "\t" DSI_NETCTRL_TEST_QOS_UDP_DSTPORT " <integer> <integer>\n"
    );

  return -1;
}
