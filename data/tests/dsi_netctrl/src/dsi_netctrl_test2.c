/******************************************************************************

                      D S I _ N E T C T R L _ T E S T 2 . C

******************************************************************************/

/******************************************************************************

  @file    dsi_netctrl_test2.c
  @brief   dsi_netctrl test

  DESCRIPTION
  Interactive test that allows making data calls using dsi_netctrl

  ---------------------------------------------------------------------------
  Copyright (c) 2010-2014 Qualcomm Technologies, Inc.
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
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <unistd.h>
#ifdef FEATURE_DS_LINUX_ANDROID
#include <string.h>
#include <cutils/properties.h>

#include "data_system_determination_v01.h"
#include "qmi_client.h"
#include "qmi.h"
#endif

#include "dsi_netctrl.h"
#include "dsi_netctrl_qos.h"


#define DSI_LOG_DEBUG(...) \
  printf(__VA_ARGS__); \
  printf("\n")
#define DSI_LOG_ERROR(...) \
  printf(__VA_ARGS__); \
  printf("\n")
#define DSI_LOG_FATAL(...) \
  printf(__VA_ARGS__); \
  printf("\n")
#define DSI_LOG_VERBOSE(...) \
  printf( __VA_ARGS__); \
  printf("\n")
#define DSI_LOG_INFO(...) \
  printf(__VA_ARGS__); \
  printf("\n")

/* demo apn */
char apn[] = "a";
/* demo apn 1 */
char apn_1[] = "b";

#define DSI_PROFILE_3GPP2_OFFSET (1000)
#define DSI_PROFILE_NUM_MAX      (9999)

#define MAX_CALLS  8

#define DSI_IP_FAMILY_4   "IP"
#define DSI_IP_FAMILY_6   "IPV6"
#define DSI_IP_FAMILY_4_6 "IPV4V6"

#define DSI_INET4_NTOP(prefix,data)                                                  \
  DSI_LOG_VERBOSE("%s [%d.%d.%d.%d]", prefix,                                        \
               data[3], data[2], data[1], data[0])
#define DSI_INET6_NTOP(prefix,data)                                                  \
  DSI_LOG_VERBOSE("%s [%.2x%.2x:%.2x%.2x:%.2x%.2x:%.2x%.2x:"                         \
                      "%.2x%.2x:%.2x%.2x:%.2x%.2x:%.2x%.2x]", prefix,                \
               data[0],data[1],data[2],data[3],data[4],data[5],data[6],data[7],      \
               data[8],data[9],data[10],data[11],data[12],data[13],data[14],data[15])


enum dsi_test_action_e {
  dsi_test_action_quit,
  dsi_test_action_create_call,
  dsi_test_action_release_call,
  dsi_test_action_net_up,
  dsi_test_action_net_down,
  dsi_test_action_cycles,
  dsi_test_action_list,
  dsi_test_action_manage_qosflow,
  dsi_test_action_get_channel_rate,
  dsi_test_action_get_bearer_tech,
  dsi_test_action_get_packet_stats,
  dsi_test_action_reset_packet_stats,
#ifdef FEATURE_DS_LINUX_ANDROID
  dsi_test_init_qmi_dsd_client,
  dsi_test_action_reg_handoff,
  dsi_test_action_unreg_handoff,
#endif
  dsi_test_action_ip_table,
  dsi_test_action_get_ioctl
};

enum dsi_test_tech_e {
  dsi_test_tech_min = 0,
  dsi_test_tech_umts = dsi_test_tech_min,
  dsi_test_tech_cdma,
  dsi_test_tech_1x,
  dsi_test_tech_do,
  dsi_test_tech_lte,
  dsi_test_tech_modem_link_local,
  dsi_test_tech_auto,
  dsi_test_tech_max
};

typedef struct {
  const char* name;
  unsigned int val;
} dsi_test_tech_map_t;

dsi_test_tech_map_t tech_map[] = {
  {"UMTS", DSI_RADIO_TECH_UMTS},
  {"CDMA", DSI_RADIO_TECH_CDMA},
  {"1X", DSI_RADIO_TECH_1X},
  {"DO", DSI_RADIO_TECH_DO},
  {"LTE", DSI_RADIO_TECH_LTE},
  {"MODEM_LINK_LOCAL", DSI_EXT_TECH_MODEM_LINK_LOCAL},
  {"AUTOMATIC", DSI_RADIO_TECH_UNKNOWN}
};


enum dsi_test_call_status_e
{
  dsi_test_call_status_idle,
  dsi_test_call_status_connecting,
  dsi_test_call_status_connected,
  dsi_test_call_status_disconnecting
};

struct status_strings_s
{
  enum dsi_test_call_status_e status;
  const char* str;
};

struct status_strings_s status_string_tbl[] =
{
  {dsi_test_call_status_idle, "idle"},
  {dsi_test_call_status_connecting, "connecting"},
  {dsi_test_call_status_connected, "connected"},
  {dsi_test_call_status_disconnecting, "disconnecting"}
};

typedef struct {
  dsi_hndl_t handle;
  const char* tech;
  const char* family;
  int profile;
  enum dsi_test_call_status_e call_status;
  pthread_mutex_t mutex;
  pthread_cond_t cond;
  struct {
    dsi_qos_id_type flow_id;
    dsi_qos_granted_info_type qos_granted_info;
  } qos_info;
} dsi_test_call_info_t;

dsi_test_call_info_t dsi_net_hndl[MAX_CALLS];

#define WAIT_ON_STATE(st,state)                             \
    (void)pthread_mutex_lock( &(st)->mutex );               \
    while( state != (st)->call_status ) {                   \
      (void)pthread_cond_wait( &(st)->cond, &(st)->mutex ); \
    }                                                       \
    (void)pthread_mutex_unlock( &(st)->mutex );

#define SET_STATE(st,state)                                 \
    (void)pthread_mutex_lock( &(st)->mutex );               \
    (st)->call_status = state;                              \
    (void)pthread_cond_signal( &(st)->cond );               \
    (void)pthread_mutex_unlock( &(st)->mutex );

struct event_strings_s
{
  dsi_net_evt_t evt;
  char * str;
};

struct event_strings_s event_string_tbl[DSI_EVT_MAX] =
{
  { DSI_EVT_INVALID, "DSI_EVT_INVALID" },
  { DSI_EVT_NET_IS_CONN, "DSI_EVT_NET_IS_CONN" },
  { DSI_EVT_NET_NO_NET, "DSI_EVT_NET_NO_NET" },
  { DSI_EVT_PHYSLINK_DOWN_STATE, "DSI_EVT_PHYSLINK_DOWN_STATE" },
  { DSI_EVT_PHYSLINK_UP_STATE, "DSI_EVT_PHYSLINK_UP_STATE" },
  { DSI_EVT_NET_RECONFIGURED, "DSI_EVT_NET_RECONFIGURED" },
  { DSI_EVT_QOS_STATUS_IND, "DSI_EVT_QOS_STATUS_IND" },
  { DSI_EVT_NET_NEWADDR, "DSI_EVT_NET_NEWADDR" },
  { DSI_EVT_NET_DELADDR, "DSI_EVT_NET_DELADDR" },
  { DSI_NET_TMGI_ACTIVATED, "DSI_NET_TMGI_ACTIVATED" },
  { DSI_NET_TMGI_DEACTIVATED, "DSI_NET_TMGI_DEACTIVATED" },
  { DSI_NET_TMGI_LIST_CHANGED, "DSI_NET_TMGI_LIST_CHANGED" },
#ifdef FEATURE_DS_LINUX_ANDROID
  { DSI_EVT_NET_HANDOFF, "DSI_NET_HANDOFF" }
#endif
};


enum dsi_qos_op_e {
  dsi_test_qos_op_min = 0,
  dsi_test_qos_op_request = dsi_test_qos_op_min,
  dsi_test_qos_op_release,
  dsi_test_qos_op_suspend,
  dsi_test_qos_op_resume,
  dsi_test_qos_op_modify,
  dsi_test_qos_flow_info,
  dsi_test_qos_op_max
};

struct {
  const char* name;
  enum dsi_qos_op_e val;
} qos_op_map[] =
{
  {"Flow Requst",  dsi_test_qos_op_request},
  {"Flow Release", dsi_test_qos_op_release},
  {"Flow Suspend", dsi_test_qos_op_suspend},
  {"Flow Resume",  dsi_test_qos_op_resume},
  {"Flow Modify",  dsi_test_qos_op_modify},
  {"Flow Info",    dsi_test_qos_flow_info},
};

struct qos_event_strings_s
{
  dsi_qos_status_event_type evt;
  char * str;
} qos_event_string_tbl[] =
{
  { DSI_QOS_ACTIVATED_EV,         "DSI_QOS_ACTIVATED_EV" },
  { DSI_QOS_SUSPENDED_EV,         "DSI_QOS_SUSPENDED_EV" },
  { DSI_QOS_GONE_EV,              "DSI_QOS_GONE_EV" },
  { DSI_QOS_MODIFY_ACCEPTED_EV,   "DSI_QOS_MODIFY_ACCEPTED_EV" },
  { DSI_QOS_MODIFY_REJECTED_EV,   "DSI_QOS_MODIFY_REJECTED_EV" },
  { DSI_QOS_INFO_CODE_UPDATED_EV, "DSI_QOS_INFO_CODE_UPDATED_EV" },
  { DSI_QOS_FLOW_ENABLED_EV,      "DSI_QOS_FLOW_ENABLED_EV" },
  { DSI_QOS_FLOW_DISABLED_EV,     "DSI_QOS_FLOW_DISABLED_EV" }
};

#ifdef FEATURE_DS_LINUX_ANDROID
/* iWLAN test definitions */
#define DSD_APN_PREF_SYS_WWAN           (0)
#define DSD_APN_PREF_SYS_WLAN           (1)
#define DSD_APN_PREF_SYS_IWLAN          (2)
#define DSD_APN_PREF_SYS_INVALID        (-1)

#define DSI_QMI_DSD_SYNC_MSG_TIMEOUT    (10000)

#define DSI_QMI_MSM_CONN_ID             QMI_PORT_RMNET_1
#define DSI_QMI_MDM_CONN_ID             QMI_PORT_RMNET_USB_0
#define DSI_QMI_DEFAULT_CONN_ID         DSI_QMI_MSM_CONN_ID

#define DSI_QMI_PROPERTY_VALUE_SIZE     PROPERTY_VALUE_MAX
#define DSI_QMI_BASEBAND_PROPERTY       "ro.baseband"
#define DSI_QMI_BASEBAND_VALUE_MSM      "msm"
#define DSI_QMI_BASEBAND_VALUE_MDM      "mdm"

static char * default_qmi_port = DSI_QMI_DEFAULT_CONN_ID;
static qmi_client_type  global_qmi_dsd_hndl = NULL;
#endif

void test_net_ev_cb( dsi_hndl_t hndl,
                     void * user_data,
                     dsi_net_evt_t evt,
                     dsi_evt_payload_t *payload_ptr )
{
  int i;
  (void)hndl; (void)payload_ptr;
  char suffix[256];

  if (evt > DSI_EVT_INVALID && evt < DSI_EVT_MAX)
  {
    dsi_test_call_info_t* call = (dsi_test_call_info_t*) user_data;
    memset( suffix, 0x0, sizeof(suffix) );

    /* Update call state */
    switch( evt )
    {
      case DSI_EVT_NET_NO_NET:
        SET_STATE( call, dsi_test_call_status_idle );
        break;

      case DSI_EVT_NET_IS_CONN:
        SET_STATE( call, dsi_test_call_status_connected );
        break;

      case DSI_EVT_QOS_STATUS_IND:
        if( payload_ptr )
        {
          switch( payload_ptr->qos_info.status_evt )
          {
            case DSI_QOS_ACTIVATED_EV:
              call->qos_info.flow_id = payload_ptr->qos_info.flow_id;
              snprintf( suffix, sizeof(suffix),
                        " ACTIVATED - QOS Flow ID [0x%lx]",
                        call->qos_info.flow_id);
              break;
            case DSI_QOS_GONE_EV:
              call->qos_info.flow_id = 0;
              snprintf( suffix, sizeof(suffix),
                        " RELEASED" );
              break;

            case DSI_QOS_SUSPENDED_EV:
              snprintf( suffix, sizeof(suffix),
                        " SUSPENDED - QOS Flow ID [0x%lx]",
                        call->qos_info.flow_id);
              break;

            case DSI_QOS_MODIFY_ACCEPTED_EV:
            case DSI_QOS_MODIFY_REJECTED_EV:
            case DSI_QOS_INFO_CODE_UPDATED_EV:
            case DSI_QOS_FLOW_ENABLED_EV:
            case DSI_QOS_FLOW_DISABLED_EV:
              /* Do nothing */
              break;

            default:
              DSI_LOG_DEBUG("Received unsupported event [%d]", evt);
              return;
          }
        }
        break;

      case DSI_EVT_PHYSLINK_DOWN_STATE:
      case DSI_EVT_PHYSLINK_UP_STATE:
      case DSI_EVT_NET_RECONFIGURED:
      case DSI_EVT_NET_NEWADDR:
      case DSI_EVT_NET_DELADDR:
      case DSI_NET_TMGI_ACTIVATED:
      case DSI_NET_TMGI_DEACTIVATED:
      case DSI_NET_TMGI_LIST_CHANGED:
        /* Do nothing */
        break;

#ifdef FEATURE_DS_LINUX_ANDROID
      case DSI_EVT_NET_HANDOFF:
        if (payload_ptr)
        {
          switch(payload_ptr->handoff_info.handoff_type)
          {
          case QMI_WDS_HANDOFF_INIT:
            DSI_LOG_DEBUG("%s", "Handoff initialized");
            break;
          case QMI_WDS_HANDOFF_SUCCESS:
            DSI_LOG_DEBUG("%s", "Handoff successful");
            break;
          case QMI_WDS_HANDOFF_FAILURE:
            DSI_LOG_DEBUG("%s", "Handoff failed");
            break;
          }
          DSI_LOG_DEBUG("IP type [%d]", payload_ptr->handoff_info.ip_type);
          DSI_LOG_VERBOSE("Source RAT [%d] Target RAT [%d]",
                          payload_ptr->handoff_info.handoff_data.source_rat,
                          payload_ptr->handoff_info.handoff_data.target_rat);
        }
        break;
#endif
      default:
        DSI_LOG_DEBUG("Received unsupported event [%d]", evt);
        return;
    }

    /* Debug message */
    for (i=0; i<DSI_EVT_MAX; i++)
    {
      if (event_string_tbl[i].evt == evt)
      {
        DSI_LOG_DEBUG("<<< callback received event [%s]%s",
                      event_string_tbl[i].str, suffix);
        break;
      }
    }
  }
}

char dsi_test_prompt()
{
  char ch;
  putchar('>');
  while ((ch = (char)getchar()) == '\n');

  return ch;
}

int dsi_test_prompt_string( char * buff, unsigned int len )
{
  putchar('>');
  (void)fgets( buff, (int)len, stdin );

  /* Return length of input string */
  return (int)strlen( buff );
}

int get_action()
{
  char ch;
  int action = dsi_test_action_quit;
  int done = 0;

  do
  {
    DSI_LOG_VERBOSE("\n==== Select action ====");
    DSI_LOG_VERBOSE("c : Create call handle");
    DSI_LOG_VERBOSE("r : Release call handle");
    DSI_LOG_VERBOSE("u : Net up");
    DSI_LOG_VERBOSE("d : Net down");
    DSI_LOG_VERBOSE("n : Net up/down cycles");
    DSI_LOG_VERBOSE("l : List calls");
    DSI_LOG_VERBOSE("z : Get Channel rates");
    DSI_LOG_VERBOSE("b : Get Bearer Tech");
    DSI_LOG_VERBOSE("s : Get packet stats");
    DSI_LOG_VERBOSE("k : reset packet stats");
    DSI_LOG_VERBOSE("o : QOS operation");
#ifdef FEATURE_DS_LINUX_ANDROID
    DSI_LOG_VERBOSE("i : Init QMI-DSD client");
    DSI_LOG_VERBOSE("h : Register for handoff indications");
    DSI_LOG_VERBOSE("e : Unregister for handoff indications");
#endif
    DSI_LOG_VERBOSE("p : IOCTL operation");
    DSI_LOG_VERBOSE("t : IP table operations");
    DSI_LOG_VERBOSE("q : Quit");
    DSI_LOG_VERBOSE("=======================");

    ch = dsi_test_prompt();

    switch (ch)
    {
    case 'q':
      action = dsi_test_action_quit;
      break;

    case 'c':
      action = dsi_test_action_create_call;
      break;

    case 'r':
      action = dsi_test_action_release_call;
      break;

    case 'u':
      action = dsi_test_action_net_up;
      break;

    case 'd':
      action = dsi_test_action_net_down;
      break;

    case 'l':
      action = dsi_test_action_list;
      break;

    case 'n':
      action = dsi_test_action_cycles;
      break;

    case 'o':
      action = dsi_test_action_manage_qosflow;
      break;

    case 'p':
      action = dsi_test_action_get_ioctl;
      break;

    case 'z':
      action = dsi_test_action_get_channel_rate;
      break;

    case 'b':
      action = dsi_test_action_get_bearer_tech;
      break;

    case 's':
      action = dsi_test_action_get_packet_stats;
      break;

    case 'k':
      action = dsi_test_action_reset_packet_stats;
      break;
#ifdef FEATURE_DS_LINUX_ANDROID
    case 'i':
      action = dsi_test_init_qmi_dsd_client;
      break;

    case 'h':
      action = dsi_test_action_reg_handoff;
      break;

    case 'e':
      action = dsi_test_action_unreg_handoff;
      break;
#endif
    case 't':
      action = dsi_test_action_ip_table;
      break;
    default:
      /* try again */
      continue;
    }

    done = 1;
  } while (!done);

  return action;
}

int get_tech()
{
  char ch;
  int i;
  int done = 0;
  int tech = -1;

  do
  {
    DSI_LOG_VERBOSE("\n==== Select tech ====");
    for (i = dsi_test_tech_min; i < dsi_test_tech_max; i++)
    {
      DSI_LOG_VERBOSE("%d : %s", i, tech_map[i].name);
    }
    DSI_LOG_VERBOSE("q : Cancel");
    DSI_LOG_VERBOSE("=====================");

    ch = dsi_test_prompt();

    if (ch == 'q')
    {
      done = 1;
      tech = -1;
    }
    else
    {
      tech = ch - '0';

      if (tech >= dsi_test_tech_min && tech < dsi_test_tech_max)
      {
        done = 1;
      }
    }
  } while (!done);

  return tech;
}

int get_ip_family()
{
  char ch;
  int ip = -1;
  int done = 0;

  do
  {
    DSI_LOG_VERBOSE("\n==== Select IP Family ====");
    DSI_LOG_VERBOSE("0 : IPv4");
    DSI_LOG_VERBOSE("1 : IPv6");
    DSI_LOG_VERBOSE("2 : IPv4v6");
    DSI_LOG_VERBOSE("q : Cancel");
    DSI_LOG_VERBOSE("==========================");

    ch = dsi_test_prompt();

    if (ch == 'q')
    {
      done = 1;
      ip = -1;
    }
    else
    {
      switch (ch)
      {
      case '0':
        ip = DSI_IP_VERSION_4;
        done = 1;
        break;

      case '1':
        ip = DSI_IP_VERSION_6;
        done = 1;
        break;

      case '2':
        ip = DSI_IP_VERSION_4_6;
        done = 1;
        break;

      default:
        break;
      }
    }
  } while (!done);

  return ip;
}

int get_profile()
{
  int profile = -1;
  int done = 0;
  char buff[8];
  int len;

  do
  {
    DSI_LOG_VERBOSE("\n==== Select Profile ====");
    DSI_LOG_VERBOSE("1..%d : profile n (3GPP2 profiles use offset %d)",
		    DSI_PROFILE_NUM_MAX, DSI_PROFILE_3GPP2_OFFSET);
    DSI_LOG_VERBOSE("a    : automatic");
    DSI_LOG_VERBOSE("q    : Cancel");
    DSI_LOG_VERBOSE("========================");


    memset( buff, 0x0, sizeof(buff) );
    len = dsi_test_prompt_string( buff, sizeof(buff) );

    if (*buff == 'q')
    {
      done = 1;
      profile = -1;
    }
    else if (*buff == 'a')
    {
      done = 1;
      profile = 0;
    }
    else
    {
      profile = (int)strtol(buff, NULL, 10);

      if (profile >= 1 && profile < DSI_PROFILE_NUM_MAX)
      {
        done = 1;
      }
    }
  } while (!done);

  return profile;
}

int get_call_cycles()
{
  char buff[8];
  int len;
  int cycles = 0;
  int done = 0;

  do
  {
    DSI_LOG_VERBOSE("\n==== Specify Cycles ====");
    DSI_LOG_VERBOSE("1..n : 'n' up/down cycles");
    DSI_LOG_VERBOSE("q    : Cancel");
    DSI_LOG_VERBOSE("========================");

    memset( buff, 0x0, sizeof(buff) );
    len = dsi_test_prompt_string( buff, sizeof(buff) );

    if (*buff == 'q')
    {
      done = 1;
      cycles = -1;
    }
    else
    {
      cycles = (int)strtol(buff, NULL, 10);

      if (cycles > 0 && cycles < 1000)
      {
        done = 1;
      }
    }
  } while (!done);

  return cycles;
}

void get_channel_rate()
{
  int i;

  for (i = 0;i < MAX_CALLS; i++)
  {
    if (dsi_net_hndl[i].handle)
    {
      /* Test print channel rate */
      dsi_data_channel_rate_t rate;
      memset(&rate, 0, sizeof(dsi_data_channel_rate_t));
      int ret = dsi_get_current_data_channel_rate(dsi_net_hndl[i].handle, &rate);

      if (ret == 0) {
        DSI_LOG_VERBOSE("\n=====Channel Rates [Call %d]======\n", i);
        DSI_LOG_VERBOSE("recvd cur_channel_rate cur_tx_rate=%lu, cur_rx_rate=%lu, max_tx_rate=%lu, max_rx_rate=%lu",
                    rate.current_tx_rate,
                    rate.current_rx_rate,
                    rate.max_tx_rate,
                    rate.max_rx_rate);
        DSI_LOG_VERBOSE("\n==================================");
      } else {
        DSI_LOG_VERBOSE("ERROR: Could not get channel rate!!\n");
      }
    }
  }
}

int print_calls()
{
  int i;
  int has_call = 0;

  for (i = 0; i < MAX_CALLS; i++)
  {
    if (dsi_net_hndl[i].handle)
    {
      DSI_LOG_VERBOSE("%d : Call %s %s on profile %d [%s]", i,
                      dsi_net_hndl[i].tech,
                      dsi_net_hndl[i].family,
                      dsi_net_hndl[i].profile,
                      status_string_tbl[dsi_net_hndl[i].call_status].str);
      has_call = 1;
    }
  }

  return has_call;
}

int get_call()
{
  char ch;
  int call;
  int done = 0;
  int has_call = 0;

  do
  {
    DSI_LOG_VERBOSE("\n==== Select Call ID ====");
    has_call = print_calls();

    if (has_call)
    {
      DSI_LOG_VERBOSE("q : Cancel");
      DSI_LOG_VERBOSE("========================");
    }
    else
    {
      DSI_LOG_VERBOSE("No call available!");
      DSI_LOG_VERBOSE("========================");
      call = -1;
      done = 1;
      break;
    }

    ch = dsi_test_prompt();

    if (ch == 'q')
    {
      call = -1;
      done = 1;
    }
    else
    {
      call = ch - '0';

      if (call >= 0 && call < MAX_CALLS && dsi_net_hndl[call].handle)
      {
        done = 1;
      }
    }

  } while (!done);

  return call;
}


int get_qos_operation()
{
  char ch;
  int i;
  int done = 0;
  int qos_op = -1;

  do
  {
    DSI_LOG_VERBOSE("\n==== Select QOS operation ====");
    for (i = dsi_test_qos_op_min; i < dsi_test_qos_op_max; i++)
    {
      DSI_LOG_VERBOSE("%d : %s", i, qos_op_map[i].name);
    }
    DSI_LOG_VERBOSE("q : Cancel");
    DSI_LOG_VERBOSE("=====================");

    ch = dsi_test_prompt();

    if (ch == 'q')
    {
      done = 1;
      qos_op = -1;
    }
    else
    {
      qos_op = ch - '0';

      if (qos_op >= dsi_test_qos_op_min && qos_op < dsi_test_qos_op_max)
      {
        done = 1;
      }
    }
  } while (!done);

  return qos_op;
}

int get_ioctl()
{
  char ch;
  int ioctl = -1;
  int done = 0;

  do
  {
    DSI_LOG_VERBOSE("\n==== Select IOCTL ====");
    DSI_LOG_VERBOSE("0 : Get PCSCF Server Address");
    DSI_LOG_VERBOSE("1 : Get PCSCF Domain Name List");
    DSI_LOG_VERBOSE("q : Cancel");
    DSI_LOG_VERBOSE("==========================");

    ch = dsi_test_prompt();

    if (ch == 'q')
    {
      done = 1;
      ioctl = -1;
    }
    else
    {
      switch (ch)
      {
      case '0':
        ioctl = DSI_IFACE_IOCTL_GET_PCSCF_SERV_ADDRESS;
        done = 1;
        break;

      case '1':
        ioctl = DSI_IFACE_IOCTL_GET_PCSCF_DOMAIN_NAME_LIST;
        done = 1;
        break;

      default:
        break;
      }
    }
  } while (!done);

  return ioctl;
}

void get_bearer_tech()
{
  int i;

  for (i = 0;i < MAX_CALLS; i++)
  {
    if (dsi_net_hndl[i].handle)
    {
      /* Test print bearer tech */
      dsi_data_bearer_tech_t ret = DSI_DATA_BEARER_TECH_UNKNOWN;
      ret = dsi_get_current_data_bearer_tech(dsi_net_hndl[i].handle);

      if (ret != DSI_DATA_BEARER_TECH_UNKNOWN) {
        DSI_LOG_VERBOSE("\n=====Bearer Tech [Call %d]======\n", i);
        DSI_LOG_VERBOSE("Current Bearer Tech = %d",ret);
        DSI_LOG_VERBOSE("\n==================================");
      } else {
        DSI_LOG_VERBOSE("ERROR: Could not get bearer tech!!\n");
      }
    }
  }
}
void get_packet_stats()
{
  char ch;
  char* ip = NULL;
  int done = 0;
  int i,ret,has_call,calls = -1;
  dsi_data_pkt_stats pkt_stats;
  do
  {
    calls = get_call();
    if (calls >= 0)
    {
      done = 1;
    }
    else
    {
      break;
    }
  } while (!done);

  if (dsi_net_hndl[calls].handle)
  {
    if((ret = dsi_get_pkt_stats(dsi_net_hndl[calls].handle,&pkt_stats)) == 0)
    {
      DSI_LOG_VERBOSE("pkt stats for bytes_rx = %llu bytes_tx = %llu pkts_dropped_rx = %lu pkts_dropped_tx = %lu pkts_rx = %lu pkts_tx = %lu",
                pkt_stats.bytes_rx,
                pkt_stats.bytes_tx,
                pkt_stats.pkts_dropped_rx,
                pkt_stats.pkts_dropped_tx,
                pkt_stats.pkts_rx,
                pkt_stats.pkts_tx
                );
    }
    else
    {
      DSI_LOG_VERBOSE("error while getting packet stats");
    }
  }
}

void reset_packet_stats()
{
  char ch;
  char* ip = NULL;
  int done = 0;
  int i,ret,has_call,calls = -1;
  dsi_data_pkt_stats pkt_stats;
  do
  {
    calls = get_call();
    if (calls >= 0)
    {
      done = 1;
    }
    else
    {
      break;
    }
  } while (!done);

  if (dsi_net_hndl[calls].handle)
  {
    if((ret = dsi_reset_pkt_stats(dsi_net_hndl[calls].handle)) == 0)
    {
      DSI_LOG_VERBOSE("reset successful");
    }
    else
    {
      DSI_LOG_VERBOSE("error while resetting packet stats");
    }
  }
}

void dsi_test_create_call(int tech, int ip_family, int profile)
{
  dsi_call_param_value_t param_info;
  int index = -1;
  int i;

  for (i = 0; i < MAX_CALLS; i++)
  {
    if (dsi_net_hndl[i].handle == NULL)
    {
      index = i;
      break;
    }
  }

  if (index < 0 || index >= MAX_CALLS)
  {
    DSI_LOG_ERROR("Reached maximum call limit");
    return;
  }

  /* obtain data service handle */
  dsi_net_hndl[index].handle = dsi_get_data_srvc_hndl(test_net_ev_cb, (void*) &dsi_net_hndl[index]);
  dsi_net_hndl[index].tech = tech_map[tech].name;
  dsi_net_hndl[index].profile = profile;
  dsi_net_hndl[index].call_status = dsi_test_call_status_idle;

  /* set data call param */
  param_info.buf_val = NULL;
  param_info.num_val = (int)tech_map[tech].val;
  DSI_LOG_DEBUG("Setting tech to %s", tech_map[tech].name);
  if( DSI_EXT_TECH_MODEM_LINK_LOCAL == param_info.num_val )
  {
    dsi_set_data_call_param(dsi_net_hndl[index].handle, DSI_CALL_INFO_EXT_TECH_PREF, &param_info);
  }
  else
  {
    dsi_set_data_call_param(dsi_net_hndl[index].handle, DSI_CALL_INFO_TECH_PREF, &param_info);
  }

  switch (tech)
  {
  case dsi_test_tech_umts:
  case dsi_test_tech_lte:
    param_info.buf_val = apn;
    param_info.num_val = 1;
    DSI_LOG_DEBUG("Setting APN to %s", apn);
    dsi_set_data_call_param(dsi_net_hndl[index].handle, DSI_CALL_INFO_APN_NAME, &param_info);
    break;

  case dsi_test_tech_cdma:
    param_info.buf_val = NULL;
    param_info.num_val = DSI_AUTH_PREF_PAP_CHAP_BOTH_ALLOWED;
    DSI_LOG_DEBUG("%s","Setting auth pref to both allowed");
    dsi_set_data_call_param(dsi_net_hndl[index].handle, DSI_CALL_INFO_AUTH_PREF, &param_info);
    break;

  default:
    break;
  }

  switch (ip_family)
  {
  case DSI_IP_VERSION_4:
    dsi_net_hndl[index].family = DSI_IP_FAMILY_4;
    param_info.buf_val = NULL;
    param_info.num_val = ip_family;
    DSI_LOG_DEBUG("Setting family to %s", DSI_IP_FAMILY_4);
    dsi_set_data_call_param(dsi_net_hndl[index].handle, DSI_CALL_INFO_IP_VERSION, &param_info);
    break;

  case DSI_IP_VERSION_6:
    dsi_net_hndl[index].family = DSI_IP_FAMILY_6;
    param_info.buf_val = NULL;
    param_info.num_val = ip_family;
    DSI_LOG_DEBUG("Setting family to %s", DSI_IP_FAMILY_6);
    dsi_set_data_call_param(dsi_net_hndl[index].handle, DSI_CALL_INFO_IP_VERSION, &param_info);
    break;

  case DSI_IP_VERSION_4_6:
    dsi_net_hndl[index].family = DSI_IP_FAMILY_4_6;
    param_info.buf_val = NULL;
    param_info.num_val = ip_family;
    DSI_LOG_DEBUG("Setting family to %s", DSI_IP_FAMILY_4_6);
    dsi_set_data_call_param(dsi_net_hndl[index].handle, DSI_CALL_INFO_IP_VERSION, &param_info);
    break;

  default:
    /* don't set anything for IPv4 */
    break;
  }

  if (profile > 0)
  {
    if( profile > DSI_PROFILE_3GPP2_OFFSET )
    {
      param_info.num_val = (profile - DSI_PROFILE_3GPP2_OFFSET);
      DSI_LOG_DEBUG("Setting 3GPP2 PROFILE to %d", param_info.num_val);
      dsi_set_data_call_param(dsi_net_hndl[index].handle, DSI_CALL_INFO_CDMA_PROFILE_IDX, &param_info);
    }
    else
    {
      param_info.num_val = profile;
      DSI_LOG_DEBUG("Setting 3GPP PROFILE to %d", param_info.num_val);
      dsi_set_data_call_param(dsi_net_hndl[index].handle, DSI_CALL_INFO_UMTS_PROFILE_IDX, &param_info);
    }
  }
}

void dsi_test_release_call(int index)
{
  DSI_LOG_DEBUG("Releasing call handle");

  dsi_rel_data_srvc_hndl(dsi_net_hndl[index].handle);
  dsi_net_hndl[index].handle = NULL;
}

void dsi_test_netup(int index)
{
  DSI_LOG_DEBUG("Attempting to bring call up on technology %s with IP family %s",
                dsi_net_hndl[index].tech,
                dsi_net_hndl[index].family);

  /* start data call */
  dsi_start_data_call(dsi_net_hndl[index].handle);
  dsi_net_hndl[index].call_status = dsi_test_call_status_connecting;
}

void dsi_test_netdown(int index)
{
  DSI_LOG_DEBUG("Attempting to bring down %s %s call on profile %d",
                dsi_net_hndl[index].tech,
                dsi_net_hndl[index].family,
                dsi_net_hndl[index].profile);

  /* end data call */
  dsi_stop_data_call(dsi_net_hndl[index].handle);
  dsi_net_hndl[index].call_status = dsi_test_call_status_disconnecting;
}

#ifdef FEATURE_DS_LINUX_ANDROID
void dsi_reg_wds_handoff_ind(int index)
{
  int rc = DSI_ERROR;

  DSI_LOG_DEBUG("Registering for WDS handoff indications for call %d for ip %s",
                index,
                dsi_net_hndl[index].family);
  do
  {
    /* Register for WDS handoff ind. This happens after SNI is complete */
    rc = dsi_ind_registration(dsi_net_hndl[index].handle,
                              QMI_WDS_HANDOFF_IND,
                              DSI_IND_REGISTER);

    if (DSI_SUCCESS != rc)
    {
      DSI_LOG_ERROR("%s", "WDS handoff registration failed!");
      break;
    }

    rc = DSI_SUCCESS;
  } while (0);

  if (DSI_SUCCESS != rc)
  {
    DSI_LOG_ERROR("%s", "WDS Handoff test failed!");
  }
  else
  {
    DSI_LOG_ERROR("%s", "WDS Handoff test success!");
  }
}

void dsi_unreg_handoff_ind(int index)
{
  int rc = DSI_ERROR;

  do
  {
    if (global_qmi_dsd_hndl != NULL)
    {
      rc = qmi_client_release(global_qmi_dsd_hndl);

      if (QMI_NO_ERR != rc)
      {
        DSI_LOG_ERROR("%s", "Error releasing DSD client");
        break;
      }
    }

    rc = dsi_ind_registration(dsi_net_hndl[index].handle,
                              QMI_WDS_HANDOFF_IND,
                              DSI_IND_UNREGISTER);

    if (DSI_SUCCESS != rc)
    {
      DSI_LOG_ERROR("%s", "WDS Handoff unregistration failed!");
      break;
    }

    rc = DSI_SUCCESS;
  } while (0);

  if (DSI_SUCCESS != rc)
  {
    DSI_LOG_ERROR("%s", "WDS Handoff test failed!");
  }
  else
  {
    DSI_LOG_ERROR("%s", "WDS Handoff test success!");
  }
}

void get_apn_name(char* buffer)
{
  unsigned int done = 0;
  int len = 0;

  do
  {
    DSI_LOG_VERBOSE("\n==== Enter APN name ====");
    DSI_LOG_VERBOSE("q: cancel");
    DSI_LOG_VERBOSE("========================");

    len = dsi_test_prompt_string( buffer,
+                                  (QMI_DSD_MAX_APN_LEN_V01 + 1) );

    if (*buffer == 'q')
    {
      done = 1;
    }
    else if (len > QMI_DSD_MAX_APN_LEN_V01)
    {
      DSI_LOG_VERBOSE("\nAPN name too long...please re-enter\n");
      done = 0;
    }
    else
    {
      done = 1;
    }
  } while (!done);

  /* Remove trailing newline character if it exists */
  if (buffer[len - 1] == '\n')
  {
    buffer[len - 1] = '\0';
  }
}

int get_pref_system()
{
  char ch;
  unsigned int done = 0;
  char buffer[1];
  int ret = DSD_APN_PREF_SYS_INVALID;

  do
  {
    DSI_LOG_VERBOSE("\n==== Enter preferred system =====");
    DSI_LOG_VERBOSE("0:  WWAN");
    DSI_LOG_VERBOSE("1:  WLAN");
    DSI_LOG_VERBOSE("2:  IWWLAN");
    DSI_LOG_VERBOSE("q:  quit");
    DSI_LOG_VERBOSE("========================");

    ch = dsi_test_prompt();

    switch (ch)
    {
    case '0':
      done = 1;
      ret = DSD_APN_PREF_SYS_WWAN;
      break;
    case '1':
      done = 1;
      ret = DSD_APN_PREF_SYS_WLAN;
      break;
    case '2':
      done = 1;
      ret = DSD_APN_PREF_SYS_IWLAN;
      break;
    case 'q':
      done = 1;
      break;
    default:
      DSI_LOG_VERBOSE("\nPreferred system not found. Please re-enter\n");
      done = 0;
    }
  } while (!done);

  return ret;
}

void dsi_register_qmi_dsd_client(int index)
{
  char ch;
  int  done = 0, rc = DSI_ERROR;
  int  len, qmi_err_code;
  int  pref_sys = DSD_APN_PREF_SYS_INVALID;
  char buff[QMI_DSD_MAX_APN_LEN_V01 + 1];
  char baseband[DSI_QMI_PROPERTY_VALUE_SIZE];
  dsd_set_apn_preferred_system_req_msg_v01  apn_req_msg;
  dsd_set_apn_preferred_system_resp_msg_v01 apn_resp_msg;

  do
  {
    memset(baseband, 0, sizeof(baseband));
    property_get(DSI_QMI_BASEBAND_PROPERTY, baseband, "");

    if (strlen(baseband) > 0)
    {
      DSI_LOG_VERBOSE("Android baseband: %s", baseband);
    }
    else
    {
      DSI_LOG_VERBOSE("Error reading baseband value...exiting!");
      break;
    }

    if (!strcmp(baseband, DSI_QMI_BASEBAND_VALUE_MSM))
    {
      default_qmi_port = DSI_QMI_MSM_CONN_ID;
    }
    else if (!strcmp(baseband, DSI_QMI_BASEBAND_VALUE_MDM))
    {
      default_qmi_port = DSI_QMI_MDM_CONN_ID;
    }

    /* Register for APN preferred messages */
    memset(&apn_req_msg, 0, sizeof(apn_req_msg));
    memset(&apn_resp_msg, 0, sizeof(apn_resp_msg));

    /* Get Pref System */
    pref_sys = get_pref_system();

    if (DSD_APN_PREF_SYS_INVALID == pref_sys)
    {
      break;
    }

    apn_req_msg.apn_pref_sys.pref_sys = pref_sys;

    /* Flushing input stream */
    while ((ch = getchar()) != '\n' && ch != EOF);

    memset( buff, 0x0, sizeof(buff) );
    get_apn_name(buff);

    /* Flushing input stream */
    while ((ch = getchar()) != '\n' && ch != EOF);

    (void) strlcpy(apn_req_msg.apn_pref_sys.apn_name,
                   buff,
                   sizeof(apn_req_msg.apn_pref_sys.apn_name));

    /* Open a qmi connection */
    rc = qmi_connection_init(default_qmi_port,
                             &qmi_err_code);
    if (QMI_NO_ERR != rc)
    {
      DSI_LOG_ERROR("qmi_connection_init for [%s] failed "
                    "with error [%d]", default_qmi_port,
                    qmi_err_code);
      break;
    }

    rc = qmi_client_init(default_qmi_port,
                         dsd_get_service_object_v01(),
                         NULL,
                         (void *)NULL,
                         &global_qmi_dsd_hndl);

    if (QMI_NO_ERR != rc)
    {
      DSI_LOG_VERBOSE("%s", "Failed to initialize qmi DSD client!");
      break;
    }
    else
    {
      DSI_LOG_DEBUG("Modem supports QMI-DSD service");
      DSI_LOG_DEBUG("qmi_hndl: %p", global_qmi_dsd_hndl);
      rc = qmi_client_send_msg_sync(global_qmi_dsd_hndl,
                                    QMI_DSD_SET_APN_PREFERRED_SYSTEM_REQ_V01,
                                    &apn_req_msg,
                                    sizeof(apn_req_msg),
                                    &apn_resp_msg,
                                    sizeof(apn_resp_msg),
                                    DSI_QMI_DSD_SYNC_MSG_TIMEOUT);

      if (QMI_NO_ERR != rc)
      {
        DSI_LOG_DEBUG("Failed to register for set APN pref system, err=%d",
                      rc);
        DSI_LOG_VERBOSE("Response code: %d", apn_resp_msg.resp.result);
        DSI_LOG_VERBOSE("Error code   : %d", apn_resp_msg.resp.error);
        break;
      }
    }

    rc = DSI_SUCCESS;
  } while (0);

  if (DSI_SUCCESS != rc)
  {
    DSI_LOG_ERROR("%s", "DSD client init failed!");
  }
  else
  {
    DSI_LOG_ERROR("%s", "DSD client init succeeded!");
  }
}
#endif

static void print_qos_filter_params
(
  dsi_qos_granted_filter_data_type  filter_params,
  char*                             filter_type_str
)
{
  DSI_LOG_VERBOSE("%s: protocol [0x%x] ", filter_type_str, (unsigned int) filter_params.qos_filter.filter_desc.protocol );
  DSI_LOG_VERBOSE("%s: tos_value [%d] ", filter_type_str, (unsigned int) filter_params.qos_filter.filter_desc.tos.tos_value );
  DSI_LOG_VERBOSE("%s: tos_mask  [%d] ", filter_type_str, (unsigned int) filter_params.qos_filter.filter_desc.tos.tos_mask );

  DSI_LOG_VERBOSE("%s: tcp_src_start_port [%d] ", filter_type_str, (unsigned int) filter_params.qos_filter.filter_desc.tcp_src_ports.start_port);
  DSI_LOG_VERBOSE("%s: tcp_src_range [%d] ", filter_type_str, (unsigned int) filter_params.qos_filter.filter_desc.tcp_src_ports.range);
  DSI_LOG_VERBOSE("%s: tcp_dest_start_port [%d] ", filter_type_str, (unsigned int) filter_params.qos_filter.filter_desc.tcp_dest_ports.start_port);
  DSI_LOG_VERBOSE("%s: tcp_dest_range [%d] ", filter_type_str, (unsigned int) filter_params.qos_filter.filter_desc.tcp_dest_ports.range);

  DSI_LOG_VERBOSE("%s: udp_src_start_port [%d] ", filter_type_str, (unsigned int) filter_params.qos_filter.filter_desc.udp_src_ports.start_port);
  DSI_LOG_VERBOSE("%s: udp_src_range [%d] ", filter_type_str, (unsigned int) filter_params.qos_filter.filter_desc.udp_src_ports.range);
  DSI_LOG_VERBOSE("%s: udp_dest_start_port [%d] ", filter_type_str, (unsigned int) filter_params.qos_filter.filter_desc.udp_dest_ports.start_port);
  DSI_LOG_VERBOSE("%s: udp_dest_range [%d] ", filter_type_str, (unsigned int) filter_params.qos_filter.filter_desc.udp_dest_ports.range);

  DSI_LOG_VERBOSE("%s: esp_security_policy_index [%d] ", filter_type_str, filter_params.qos_filter.filter_desc.esp_security_policy_index);
  DSI_LOG_VERBOSE("%s: precedence [%d] ", filter_type_str, (unsigned int) filter_params.qos_filter.filter_desc.precedence);
  DSI_LOG_VERBOSE("%s: filter_id [%d] ", filter_type_str, (unsigned int) filter_params.qos_filter.filter_desc.filter_id );

  DSI_LOG_VERBOSE("%s: transport_src_start_port [%d] ", filter_type_str, (unsigned int) filter_params.qos_filter.filter_desc.transport_src_ports.start_port);
  DSI_LOG_VERBOSE("%s: transport_src_range [%d] ", filter_type_str, (unsigned int) filter_params.qos_filter.filter_desc.transport_src_ports.range);
  DSI_LOG_VERBOSE("%s: transport_dest_start_port [%d] ", filter_type_str, (unsigned int) filter_params.qos_filter.filter_desc.transport_dest_ports.start_port);
  DSI_LOG_VERBOSE("%s: transport_dest_range [%d] ", filter_type_str, (unsigned int) filter_params.qos_filter.filter_desc.transport_dest_ports.range);

  if(filter_params.qos_filter.ip_version == QMI_QOS_IP_VERSION_4)
  {
    char str[256] ={0};

    snprintf(str,256,"%s: %s",filter_type_str, "src_addr.ipv4_ip_addr" );
    DSI_INET4_NTOP(str, ((unsigned char*)&filter_params.qos_filter.filter_desc.src_addr.ipv4_ip_addr));
    snprintf(str,256,"%s: %s",filter_type_str, "src_addr.ipv4_subnet_mask" );
    DSI_INET4_NTOP(str, ((unsigned char*)&filter_params.qos_filter.filter_desc.src_addr.ipv4_subnet_mask));

    snprintf(str,256,"%s: %s",filter_type_str, "dest_addr.ipv4_ip_addr" );
    DSI_INET4_NTOP(str, ((unsigned char*)&filter_params.qos_filter.filter_desc.dest_addr.ipv4_ip_addr));
    snprintf(str,256,"%s: %s",filter_type_str, "dest_addr.ipv4_subnet_mask" );
    DSI_INET4_NTOP(str, ((unsigned char*)&filter_params.qos_filter.filter_desc.dest_addr.ipv4_subnet_mask));
  }
  else if(filter_params.qos_filter.ip_version == QMI_QOS_IP_VERSION_6)
  {
    char str[256] = {0};

    DSI_LOG_VERBOSE("%s: ipv6_src_addr.ipv6_filter_prefix_len [%d]", filter_type_str, (unsigned short)filter_params.qos_filter.filter_desc.ipv6_src_addr.ipv6_filter_prefix_len);
    snprintf(str,256,"%s: %s",filter_type_str, "ipv6_src_addr.ipv6_ip_addr" );
    DSI_INET6_NTOP(str, filter_params.qos_filter.filter_desc.ipv6_src_addr.ipv6_ip_addr);

    DSI_LOG_VERBOSE("%s: ipv6_dest_addr.ipv6_filter_prefix_len [%d]", filter_type_str, (unsigned short)filter_params.qos_filter.filter_desc.ipv6_dest_addr.ipv6_filter_prefix_len);
    snprintf(str,256,"%s: %s",filter_type_str, "ipv6_dest_addr.ipv6_ip_addr" );
    DSI_INET6_NTOP(str, filter_params.qos_filter.filter_desc.ipv6_dest_addr.ipv6_ip_addr);

    DSI_LOG_VERBOSE("%s: ipv6_traffic_class.traffic_class_value [%d]", filter_type_str, (unsigned short)filter_params.qos_filter.filter_desc.ipv6_traffic_class.traffic_class_value);
    DSI_LOG_VERBOSE("%s: ipv6_traffic_class.traffic_class_mask [%d]", filter_type_str, (unsigned short)filter_params.qos_filter.filter_desc.ipv6_traffic_class.traffic_class_mask);

    DSI_LOG_VERBOSE("%s: ipv6_flow_label [%ld]", filter_type_str, filter_params.qos_filter.filter_desc.ipv6_flow_label);
  }
}

void dsi_test_manage_qosflow(int index,  int qos_op)
{
  dsi_qos_spec_type  qos_spec;
  dsi_qos_id_type    qos_id_list;
  dsi_qos_err_rsp_type   qos_spec_err_list;
  int count = 0;

  int status;

  switch( qos_op )
  {
    case dsi_test_qos_op_request:
      /* Create a QOS spec */
      memset( &qos_spec, 0x0, sizeof(qos_spec));
      qos_spec.tx_flow_req_array = malloc( sizeof(qmi_qos_flow_req_type) );
      qos_spec.rx_flow_req_array = malloc( sizeof(qmi_qos_flow_req_type) );
      qos_spec.tx_filter_req_array = malloc( sizeof(qmi_qos_filter_req_type) );
      qos_spec.rx_filter_req_array = malloc( sizeof(qmi_qos_filter_req_type) );

      if (!qos_spec.tx_flow_req_array || !qos_spec.rx_flow_req_array
          || !qos_spec.tx_filter_req_array || !qos_spec.rx_filter_req_array) {
          DSI_LOG_ERROR("Unable to process specified QOS operation [%d]", qos_op);
          break;
      }

      memset( qos_spec.tx_flow_req_array, 0x0, sizeof(qmi_qos_flow_req_type) );
      memset( qos_spec.rx_flow_req_array, 0x0, sizeof(qmi_qos_flow_req_type) );
      memset( qos_spec.tx_filter_req_array, 0x0, sizeof(qmi_qos_filter_req_type) );
      memset( qos_spec.rx_filter_req_array, 0x0, sizeof(qmi_qos_filter_req_type) );

      qos_spec.num_tx_flow_req = 1;
      qos_spec.tx_flow_req_array->umts_flow_desc.param_mask |= QMI_QOS_UMTS_FLOW_PARAM_DATA_RATE;
      qos_spec.tx_flow_req_array->umts_flow_desc.data_rate.guaranteed_rate = 1000000;
      qos_spec.tx_flow_req_array->umts_flow_desc.data_rate.max_rate = 1500000;
      qos_spec.num_tx_filter_req = 1;
      qos_spec.tx_filter_req_array->ip_version = QMI_QOS_IP_VERSION_4;
      qos_spec.tx_filter_req_array->filter_desc.param_mask |= QMI_QOS_FILTER_PARAM_TCP_SRC_PORTS;
      qos_spec.tx_filter_req_array->filter_desc.tcp_src_ports.start_port = (uint16_t)ntohs(4000);
      qos_spec.tx_filter_req_array->filter_desc.tcp_src_ports.range = 1;
      qos_spec.tx_filter_req_array->filter_desc.param_mask |= QMI_QOS_FILTER_PARAM_TRANS_PROTOCOL;
      qos_spec.tx_filter_req_array->filter_desc.protocol = QMI_QOS_TRANS_PROT_TCP;
      qos_spec.tx_filter_req_array->filter_desc.param_mask |= QMI_QOS_FILTER_PARAM_DEST_ADDR;
      qos_spec.tx_filter_req_array->filter_desc.dest_addr.ipv4_ip_addr = 0x0AE22E24; // solarflare 10.226.46.36
      qos_spec.tx_filter_req_array->filter_desc.dest_addr.ipv4_subnet_mask = 0xFFFFFFFF;

      /* Issue request for QOS spec */
      status = dsi_request_qos( dsi_net_hndl[index].handle,
                                1, &qos_spec, QMI_QOS_REQUEST,
                                &qos_id_list, &qos_spec_err_list );

      free( qos_spec.tx_flow_req_array );
      free( qos_spec.rx_flow_req_array );
      free( qos_spec.tx_filter_req_array );
      free( qos_spec.rx_filter_req_array );
      break;

    case dsi_test_qos_op_release:
      /* Issue request */
      status = dsi_release_qos( dsi_net_hndl[index].handle,
                                1, &dsi_net_hndl[index].qos_info.flow_id );
      break;

    case dsi_test_qos_op_suspend:
      /* Issue request */
      status = dsi_suspend_qos( dsi_net_hndl[index].handle,
                                1, &dsi_net_hndl[index].qos_info.flow_id );
      break;

    case dsi_test_qos_op_resume:
      /* Issue request */
      status = dsi_resume_qos( dsi_net_hndl[index].handle,
                               1, &dsi_net_hndl[index].qos_info.flow_id );
      break;
    case dsi_test_qos_flow_info:
    {
      dsi_qos_granted_info_type *qos_granted_info_ptr = &dsi_net_hndl[index].qos_info.qos_granted_info;

      int ip_family = AF_INET;

      if(strncmp(dsi_net_hndl[index].family,DSI_IP_FAMILY_4_6,6) == 0)
      {
        DSI_LOG_ERROR("DUAL IP QoS retrieval not supported via test app");
        break;
      }
      else if(strncmp(dsi_net_hndl[index].family,DSI_IP_FAMILY_6,4) == 0)
      {
	ip_family = AF_INET6;
      }
      else if (strncmp(dsi_net_hndl[index].family,DSI_IP_FAMILY_4,2) == 0)
      {
	ip_family = AF_INET;
      }
      else
      {
        DSI_LOG_ERROR("IP family unknown");
        break;
      }

      memset(qos_granted_info_ptr,0x0,sizeof(dsi_test_qos_flow_info));

      dsi_get_granted_qos(dsi_net_hndl[index].handle,
                          dsi_net_hndl[index].qos_info.flow_id,
                          ip_family,
                          qos_granted_info_ptr);

      if( qos_granted_info_ptr->rx_filter_count > 0)
      {

        for(count =0; count < qos_granted_info_ptr->rx_filter_count ; count++)
        {
          DSI_LOG_VERBOSE("\nfilter_count %d\n",count+1);
          print_qos_filter_params(dsi_net_hndl[index].qos_info.qos_granted_info.rx_granted_filter_data[count],"Rx");
        }
      }

      if(qos_granted_info_ptr->tx_filter_count > 0)
      {
        for(count =0; count < qos_granted_info_ptr->tx_filter_count ; count++)
	{
           DSI_LOG_VERBOSE("\nfilter_count %d\n",count+1);
           print_qos_filter_params(dsi_net_hndl[count].qos_info.qos_granted_info.tx_granted_filter_data[count],"Tx");
        }
      }

      break;
    }

    default:
      DSI_LOG_ERROR("Unsupported QOS operation [%d]", qos_op);
  }
}


void dsi_test_ip_table_ops(dsi_hndl_t hndl)
{
  char ch = 0;
  dsi_port_forwarding_status_t status;
  int ip = (get_ip_family()== DSI_IP_VERSION_4? AF_INET:AF_INET6);
  int done = 0;

  do
  {
    DSI_LOG_VERBOSE("\n==== Select port operation ====");
    DSI_LOG_VERBOSE("0 : Enable port forwarding");
    DSI_LOG_VERBOSE("1 : Disable port forwarding ");
    DSI_LOG_VERBOSE("2 : Query port forwarding status ");
    DSI_LOG_VERBOSE("q : Cancel");
    DSI_LOG_VERBOSE("==========================");

    ch = dsi_test_prompt();

    if (ch == 'q')
    {
      done = 1;
    }
    else
    {
      switch (ch)
      {
      case '0':
        if (dsi_enable_port_forwarding(hndl,ip) == DSI_SUCCESS)
        {
          DSI_LOG_VERBOSE("enable successful");
        }
        else
        {
          DSI_LOG_VERBOSE("enable failed");
        }

        done = 1;
        break;

      case '1':
        if(dsi_disable_port_forwarding(hndl, ip) == DSI_SUCCESS)
        {
            DSI_LOG_VERBOSE("disable successful");
        }
        else
        {
            DSI_LOG_VERBOSE("disable failed");
        }
        done = 1;
        break;
      case '2':
        if(dsi_query_port_forwarding_status(hndl,ip,&status)== DSI_SUCCESS)
        {
           DSI_LOG_VERBOSE("IP [%d] status [%s]", ip, ((int)status==DSI_PORT_FORWARDING_ENABLED)?"ENABLED":"DISABLED");
        }
        else
        {
            DSI_LOG_VERBOSE("query failed");
        }
        done = 1;
        break;

      default:

        break;
      }
    }
  } while (!done);
}


void dsi_test_get_ioctl(dsi_hndl_t hndl )
{
  dsi_pcscf_addr_info_t* pcscf_addr_list = NULL;
  dsi_pcscf_fqdn_list_t* pcscf_fqdn_list = NULL;
  struct sockaddr_storage sa;
  struct sockaddr_storage* __attribute__((__may_alias__)) sa_ptr;
  struct sockaddr_in* sa_in = NULL;
  struct sockaddr_in6* sa_in6 = NULL;
  char ip_str[INET6_ADDRSTRLEN];
  int err_code;
  int result;
  unsigned int index = 0;
  int ioctl = get_ioctl();

  switch(ioctl)
  {
    case DSI_IFACE_IOCTL_GET_PCSCF_SERV_ADDRESS:
    {
      pcscf_addr_list = (dsi_pcscf_addr_info_t*) malloc(sizeof(dsi_pcscf_addr_info_t));
      if( NULL == pcscf_addr_list)
      {
        DSI_LOG_VERBOSE("Error allocating memory");
        return;
      }

      result  = dsi_iface_ioctl(hndl,
                                DSI_IFACE_IOCTL_GET_PCSCF_SERV_ADDRESS,
                                pcscf_addr_list,
                                &err_code);
      if(DSI_ERROR == result)
      {
        DSI_LOG_VERBOSE("Error fetching IOCTL - DSI_IFACE_IOCTL_GET_PCSCF_SERV_ADDRESS");
        return;
      }
      else
      {
        DSI_LOG_VERBOSE("PCSCF Address count : [%d] ", pcscf_addr_list->addr_count);

        for(index = 0; index < pcscf_addr_list->addr_count; index++)
        {
          /* Print IP Address */
          if (pcscf_addr_list->pcscf_address[index].valid_addr)
          {
            if(pcscf_addr_list->pcscf_address[index].addr.ss_family == AF_INET)
            {
                sa = pcscf_addr_list->pcscf_address[index].addr;
                sa_ptr = &sa;
                sa_in = (struct sockaddr_in*)sa_ptr;
                memset(ip_str, 0, INET_ADDRSTRLEN);
                inet_ntop(AF_INET,
                          &(sa_in->sin_addr),
                          ip_str,
                          INET_ADDRSTRLEN);
                DSI_LOG_VERBOSE("IPv4 PCSCF : %s ", ip_str);
            }
            else if ( pcscf_addr_list->pcscf_address[index].addr.ss_family == AF_INET6)
            {
                sa = pcscf_addr_list->pcscf_address[index].addr;
                sa_ptr = &sa;
                sa_in6 = (struct sockaddr_in6*)sa_ptr;
                memset(ip_str, 0, INET6_ADDRSTRLEN);
                inet_ntop(AF_INET6,
                          &(sa_in6->sin6_addr),
                          ip_str,
                          INET6_ADDRSTRLEN);
                DSI_LOG_VERBOSE("IPv6 PCSCF : %s ", ip_str);
            }
          }
        }
      }
      break;
    }
    case DSI_IFACE_IOCTL_GET_PCSCF_DOMAIN_NAME_LIST:
    {
      pcscf_fqdn_list = (dsi_pcscf_fqdn_list_t*) malloc(sizeof(dsi_pcscf_fqdn_list_t));
      if( NULL == pcscf_fqdn_list)
      {
        DSI_LOG_VERBOSE("Error allocating memory");
        return;
      }

      result  = dsi_iface_ioctl(hndl,
                                DSI_IFACE_IOCTL_GET_PCSCF_DOMAIN_NAME_LIST,
                                pcscf_fqdn_list,
                                &err_code);
      if(DSI_ERROR == result)
      {
        DSI_LOG_VERBOSE("Error fetching IOCTL - DSI_IFACE_IOCTL_GET_PCSCF_DOMAIN_NAME_LIST");
        return;
      }
      else
      {
        DSI_LOG_VERBOSE("PCSCF Domain Name count : [%d] ", pcscf_fqdn_list->fqdn_count);

        for(index = 0; index < pcscf_fqdn_list->fqdn_count; index++)
        {
          /* Print FQDN */
          DSI_LOG_VERBOSE("PCSCF Domain address : [ %s ] ", pcscf_fqdn_list->pcscf_domain_list[index].fqdn_string);
        }
      }
      break;
    }
    default:
      DSI_LOG_VERBOSE("Invalid IOCTL [%d]", ioctl);
      break;
  }
}

void dsi_test_exit(void)
{
  int i;

  /* release data service */
  for (i = 0; i < MAX_CALLS; i++)
  {
    if (dsi_net_hndl[i].handle)
    {
      dsi_rel_data_srvc_hndl(dsi_net_hndl[i].handle);
    }
  }
}

void dsi_test_call_cycles( int call, int cycles )
{
  int i;

  for( i=1; i<=cycles; i++ )
  {
    DSI_LOG_DEBUG("=== Start Cycle #%d ===", i);

    /* Bringup interface */
    dsi_test_netup( call );

    /* Wait for connect event */
    DSI_LOG_DEBUG("Waiting for CONNECT...");
    WAIT_ON_STATE( &dsi_net_hndl[call], dsi_test_call_status_connected );

    /* Teardown interface */
    dsi_test_netdown( call );

    /* Wait for disconnect event */
    DSI_LOG_DEBUG("Waiting for DISCONNECT...");
    WAIT_ON_STATE( &dsi_net_hndl[call], dsi_test_call_status_idle );

    DSI_LOG_DEBUG("=== End Cycle #%d ===", i);
  }
}

void dsi_test_init()
{
  int i;

  memset( dsi_net_hndl, 0x0, sizeof(dsi_net_hndl) );

  for (i = 0; i < MAX_CALLS; i++)
  {
    (void)pthread_mutex_init( &dsi_net_hndl[i].mutex, NULL );
    (void)pthread_cond_init( &dsi_net_hndl[i].cond, NULL );
  }
}

void dsi_init_cb( void *cb_data){
  printf("dsi_init callback: Start\n");
  printf("dsi_netctrl: inited. Receieved data %p\n", cb_data);
  printf("dsi_init callback: End\n");
}

int main(int argc, char *argv[])
{
  char _exit = 0;
  int action;
  int family;
  int tech;
  int profile;
  int call;
  int cycles;
  int qos_op;
  void* qos_spec = NULL;
  unsigned int qos_flowid = 0;

  /* Initialize internal data structs */
  dsi_test_init();

  /* initilize qcril library  */
  if (DSI_SUCCESS !=  dsi_init(DSI_MODE_GENERAL))
  {
    DSI_LOG_ERROR("%s","dsi_init failed !!");
    return -1;
  }

  atexit(dsi_test_exit);
  if(argc > 1){
  if(strncmp(argv[1],"background",9) == 0){
	/* For power profiling, we need to run dsi_netctrl_test
	in the background, the following code helps connect to
	a backhaul. */
	sleep(2);
	dsi_test_create_call(6,DSI_IP_VERSION_4, 0);
	DSI_LOG_DEBUG("Doing DSI Net UP\n");
	dsi_test_netup(0);
	do{
		sleep (10000000);
	}while(1);
	}
  }
  do
  {
    action = get_action();

    switch (action)
    {
    case dsi_test_action_quit:
      _exit = 1;
      break;;

    case dsi_test_action_create_call:
      tech = get_tech();
      if (tech < 0)
        break;

      family = get_ip_family();
      if (family < 0)
        break;

      profile = get_profile();
      if (profile < 0)
        break;

      dsi_test_create_call(tech, family, profile);
      break;

    case dsi_test_action_release_call:
      call = get_call();
      if (call < 0)
        break;

      dsi_test_release_call(call);
      break;

    case dsi_test_action_net_up:
      call = get_call();
      if (call < 0)
        break;

      dsi_test_netup(call);
      break;

    case dsi_test_action_net_down:
      call = get_call();
      if (call < 0)
        break;

      dsi_test_netdown(call);
      break;

    case dsi_test_action_cycles:
      call = get_call();
      if (call < 0)
        break;

      cycles = get_call_cycles();
      if (cycles <= 0)
        break;

      dsi_test_call_cycles( call, cycles );
      break;

    case dsi_test_action_list:
      if (!print_calls())
      {
        DSI_LOG_VERBOSE("No Calls");
      }
      break;

    case dsi_test_action_manage_qosflow:
      call = get_call();
      if (call < 0)
        break;

      if( dsi_test_call_status_connected == dsi_net_hndl[call].call_status )
      {
        qos_op = get_qos_operation();
        if (qos_op < 0)
          break;

        dsi_test_manage_qosflow(call, qos_op);
      }
      else
      {
        DSI_LOG_ERROR("%s","Call not connected, rejected");
      }
      break;

    case dsi_test_action_get_channel_rate:
      get_channel_rate();
      break;

    case dsi_test_action_get_bearer_tech:
      get_bearer_tech();
      break;

    case dsi_test_action_get_packet_stats:
      get_packet_stats();
      break;

    case dsi_test_action_reset_packet_stats:
      reset_packet_stats();
      break;

#ifdef FEATURE_DS_LINUX_ANDROID
    case dsi_test_init_qmi_dsd_client:
      call = get_call();
      if (call < 0)
        break;

      dsi_register_qmi_dsd_client(call);
      break;

    case dsi_test_action_reg_handoff:
      call = get_call();
      if (call < 0)
        break;

      dsi_reg_wds_handoff_ind(call);
      break;

    case dsi_test_action_unreg_handoff:
      call = get_call();
      if (call < 0)
        break;

      dsi_unreg_handoff_ind(call);
      break;
#endif

    case dsi_test_action_get_ioctl:
      call = get_call();
      if( call < 0 )
        break;

      dsi_test_get_ioctl(dsi_net_hndl[call].handle);
      break;
    case dsi_test_action_ip_table:
      call = get_call();
      if(call < 0 )
          break;
      dsi_test_ip_table_ops(dsi_net_hndl[call].handle);
      break;

    default:
      DSI_LOG_ERROR("%s","unrecognized command");
    }
  } while (!_exit);

  return 0;
}
