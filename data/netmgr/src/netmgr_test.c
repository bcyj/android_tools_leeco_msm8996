/******************************************************************************

                        N E T M G R _ T E S T . C

******************************************************************************/

/******************************************************************************

  @file    netmgr_test.c
  @brief   Network Manager tests 

  DESCRIPTION
  NetMgr test functions.

******************************************************************************/
/*===========================================================================

  Copyright (c) 2010-2012 Qualcomm Technologies, Inc. All Rights Reserved

  Qualcomm Technologies Proprietary

  Export of this technology or software is regulated by the U.S. Government.
  Diversion contrary to U.S. law prohibited.

  All ideas, data and information contained in or disclosed by
  this document are confidential and proprietary information of
  Qualcomm Technologies, Inc. and all rights therein are expressly reserved.
  By accepting this material the recipient agrees that this material
  and the information contained therein are held in confidence and in
  trust and will not be used, copied, reproduced in whole or in part,
  nor its contents revealed in any manner to others without the express
  written permission of Qualcomm Technologies, Inc.

===========================================================================*/

/******************************************************************************

                      EDIT HISTORY FOR FILE

  $Id:$

when       who        what, where, why
--------   ---        -------------------------------------------------------
02/10/10   ar         Initial version

******************************************************************************/

#ifdef NETMGR_TEST 

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#ifndef NETMGR_OFFTARGET
#include <netinet/in.h>
#endif
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <asm/types.h>
#include <pthread.h>
#include <linux/if.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>

#include "qmi.h"
#include "qmi_wds_srvc.h"
#include "qmi_qos_srvc.h"
#include "qmi_qos_srvc_i.h"

#include "netmgr_defs.h"
#include "netmgr_exec.h"
#include "netmgr_kif.h" 
#include "netmgr_netlink.h" 
#include "netmgr_qmi.h"
#include "netmgr_util.h"
#include "netmgr_sm_int.h"
#include "netmgr_test.h"
#ifdef NETMGR_QOS_ENABLED
#include "netmgr_tc_i.h"
#endif

#include "netmgr_stubs.h"

extern int test_mtu;

extern void
netmgr_kif_nl_msg_recv_cmd_free (ds_cmd_t * cmd, void * data);

/* Current test info */
LOCAL netmgr_test_info_t netmgr_test_info;

extern struct netmgr_qmi_cfg_s netmgr_qmi_cfg;
extern struct netmgr_tc_cfg_s netmgr_tc_cfg;
extern struct netmgr_exec_state_s  netmgr_exec_state_info;
extern netmgr_kif_info_t netmgr_kif_info[];

/* Forward declarations */
void
netmgr_qmi_sys_cb
(
  qmi_sys_event_type event_id,
  const qmi_sys_event_info_type * event_info,
  void * user_data
);

void netmgr_qmi_wds_ind
( 
  int                            user_handle,
  qmi_service_id_type            service_id,
  void                         * user_data,
  qmi_wds_indication_id_type     ind_id,
  qmi_wds_indication_data_type * ind_data
);

void netmgr_qmi_qos_ind
( 
  int                            user_handle,
  qmi_service_id_type            service_id,
  void                         * user_data,
  qmi_qos_indication_id_type     ind_id,
  qmi_qos_indication_data_type * ind_data
);

/*--------------------------------------------------------------------------- 
   Utility routines
---------------------------------------------------------------------------*/

#define NETMGR_CMD_WAIT()                                                  \
        pthread_mutex_lock(&netmgr_test_info.mutx);                        \
        pthread_cond_wait(&netmgr_test_info.cond, &netmgr_test_info.mutx); \
        pthread_mutex_unlock(&netmgr_test_info.mutx);

#define TEST_ASSERT(a)                               \
        if(!(a)) {                                   \
          netmgr_log_high("...FAILED: " #a "\n" );   \
          NETMGR_ASSERT(a);                          \
        } else {                                     \
          netmgr_log_high("...Verified: " #a "\n" ); \
        }


int netmgr_test_reset()
{
  int i=0;
  netmgr_exec_cmd_t cmd;

  netmgr_log_high("\n\n****END OF TEST*****\n\n");

  for( i=0; i<NETMGR_MAX_LINK; i++ ) {
    netmgr_log_high("\n\n>>>>>>>resetting link %d\n\n", i);    
    /* Inject RESET command to state machine */
    cmd.data.link = i;
    stm_instance_process_input( NULL,
                                NETMGR_SM,
                                i,
                                NETMGR_MODEM_OOS_EV,
                                &cmd);
    stm_instance_process_input( NULL,
                                NETMGR_SM,
                                i,
                                NETMGR_MODEM_IS_EV,
                                &cmd );
  }
  return 0;
}

struct msghdr * netmgr_test_make_kif_opened( int link )
{
  struct msghdr * msgh = NULL;
  netmgr_exec_cmd_t * cmd_buf = NULL;
  struct ifinfomsg * ifinfo = NULL;
  
  msgh = netmgr_nl_alloc_msg( NL_MSG_MAX_LEN );
  ((struct nlmsghdr *)msgh->msg_iov->iov_base)->nlmsg_type = RTM_NEWLINK;
  ((struct nlmsghdr *)msgh->msg_iov->iov_base)->nlmsg_len = NLMSG_SPACE(sizeof (struct nlmsghdr));
  ifinfo = (struct ifinfomsg *) NLMSG_DATA(((struct nlmsghdr *)msgh->msg_iov->iov_base));
  ifinfo->ifi_index = (link+3);
  ifinfo->ifi_change = IFF_UP;
  ifinfo->ifi_flags  = IFF_UP;
  return msgh;
}

struct msghdr * netmgr_test_make_kif_configured( int link )
{
  struct msghdr * msgh = NULL;
  netmgr_exec_cmd_t * cmd_buf = NULL;
  struct ifaddrmsg * ifaddr = NULL;
  
  /* Simulate KIF_CONFIGURE */
  msgh = netmgr_nl_alloc_msg( NL_MSG_MAX_LEN );
  ((struct nlmsghdr *)msgh->msg_iov->iov_base)->nlmsg_type = RTM_NEWADDR;
  ((struct nlmsghdr *)msgh->msg_iov->iov_base)->nlmsg_len = NL_MSG_MAX_LEN;

  ifaddr = (struct ifaddrmsg *) NLMSG_DATA(((struct nlmsghdr *)msgh->msg_iov->iov_base));
  ifaddr->ifa_index = (link+3);
  ifaddr->ifa_family = AF_INET;  // 4
  ifaddr->ifa_prefixlen = 0;
  ifaddr->ifa_flags = 0;
  ifaddr->ifa_scope = 0;

  struct nlmsghdr * nlh = (struct nlmsghdr*)msgh->msg_iov->iov_base;  /* NL message header */
  struct rtattr *rtah = NULL;
  struct sockaddr_storage addr_info;

  memset( &addr_info, 0x0, sizeof(addr_info));
  addr_info.ss_family = AF_INET;
  *((uint32*)addr_info.__ss_padding) = 0x01020304;
  
  rtah = IFA_RTA( NLMSG_DATA(nlh) );
  rtah->rta_type = IFA_ADDRESS;
  rtah->rta_len = sizeof(addr_info);
  memcpy( RTA_DATA(rtah), &addr_info.__ss_padding, sizeof(addr_info.__ss_padding));

  return msgh;
}

struct msghdr * netmgr_test_make_kif_configured_v6( int link )
{
  struct msghdr * msgh = NULL;
  netmgr_exec_cmd_t * cmd_buf = NULL;
  struct ifaddrmsg * ifaddr = NULL;
  netmgr_nl_msg_t * nlmsg_info = NULL;

  /* Simulate KIF_CONFIGURE */
  msgh = netmgr_nl_alloc_msg( NL_MSG_MAX_LEN );
  ((struct nlmsghdr *)msgh->msg_iov->iov_base)->nlmsg_type = RTM_NEWADDR;
  ((struct nlmsghdr *)msgh->msg_iov->iov_base)->nlmsg_len = NL_MSG_MAX_LEN;

//  memset( ((struct nlmsghdr *)msgh->msg_iov->iov_base), 0x06, NL_MSG_MAX_LEN);
  ifaddr = (struct ifaddrmsg *) NLMSG_DATA(((struct nlmsghdr *)msgh->msg_iov->iov_base));
  ifaddr->ifa_index = (link+3);
  ifaddr->ifa_family = AF_INET6;  // 6;
  ifaddr->ifa_prefixlen = 64;
  ifaddr->ifa_flags = 0;
  ifaddr->ifa_scope = 0;
  return msgh;
}

struct msghdr * netmgr_test_make_kif_closed( int link )
{
  struct msghdr * msgh = NULL;
  netmgr_exec_cmd_t * cmd_buf = NULL;
  struct ifinfomsg * ifinfo = NULL;
  
  msgh = netmgr_nl_alloc_msg( NL_MSG_MAX_LEN );
  ((struct nlmsghdr *)msgh->msg_iov->iov_base)->nlmsg_type = RTM_DELLINK;
  ((struct nlmsghdr *)msgh->msg_iov->iov_base)->nlmsg_len = NL_MSG_MAX_LEN;

  ifinfo = (struct ifinfomsg *) NLMSG_DATA(((struct nlmsghdr *)msgh->msg_iov->iov_base));
  ifinfo->ifi_index = (link+3);
  ifinfo->ifi_change = IFF_UP;
  ifinfo->ifi_flags  = ~IFF_UP;
  return msgh;
}

void netmgr_test_make_qmi_connected(int INSTANCE)
{
  qmi_wds_indication_data_type ind_data;

  /* Generate QMI Modem network interface UP indication */
  ind_data.pkt_srvc_status.param_mask =
    (QMI_WDS_PKT_SRVC_IND_IP_FAMILY|QMI_WDS_PKT_SRVC_IND_TECH_NAME); /* No call end reason */
  ind_data.pkt_srvc_status.link_status = QMI_WDS_PACKET_DATA_CONNECTED;
  ind_data.pkt_srvc_status.reconfig_required = FALSE;
  ind_data.pkt_srvc_status.ip_family = QMI_WDS_IP_FAMILY_PREF_IPV4;
  ind_data.pkt_srvc_status.tech_name = QMI_WDS_IFACE_NAME_UTMS;
  
  netmgr_log_high("Injecting message: QMI_WDS_SRVC_PKT_SRVC_STATUS_IND_MSG\n");
  netmgr_qmi_wds_ind( INSTANCE, QMI_WDS_SERVICE, (void*)INSTANCE,
                      QMI_WDS_SRVC_PKT_SRVC_STATUS_IND_MSG, &ind_data );


}

void netmgr_test_kif_opened(int INSTANCE)
{
  netmgr_exec_cmd_t * cmd_buf = NULL;
  /* Simulate KIF_OPEN */
  cmd_buf = netmgr_exec_get_cmd();
  cmd_buf->cmd.free_f = netmgr_kif_nl_msg_recv_cmd_free;
  cmd_buf->data.type = NETMGR_KIF_MSG_CMD;
  cmd_buf->data.link = INSTANCE;
  cmd_buf->data.info.kif_msg.msg = netmgr_test_make_kif_opened( INSTANCE );
  cmd_buf->data.info.kif_msg.msglen = NL_MSG_MAX_LEN;
  netmgr_log_high("Injecting message: NETMGR_KIF_MSG_CMD - OPENED\n");
  netmgr_exec_put_cmd( cmd_buf );
}

void netmgr_test_kif_configured(int INSTANCE)
{
  netmgr_exec_cmd_t * cmd_buf = NULL;
  /* Simulate KIF_CONFIGURE */
  cmd_buf = netmgr_exec_get_cmd();
  cmd_buf->cmd.free_f = netmgr_kif_nl_msg_recv_cmd_free;
  cmd_buf->data.type = NETMGR_KIF_MSG_CMD;
  cmd_buf->data.info.kif_msg.msg = netmgr_test_make_kif_configured( INSTANCE );
  cmd_buf->data.info.kif_msg.msglen = NL_MSG_MAX_LEN;

  netmgr_log_high("Injecting message: NETMGR_KIF_MSG_CMD - CONFIGURED\n");
  netmgr_exec_put_cmd( cmd_buf );
}
/*--------------------------------------------------------------------------- 
   Testcases
---------------------------------------------------------------------------*/



int netmgr_test_iface_bringup_successful( void* arg )
{
  qmi_wds_indication_data_type ind_data;
  struct msghdr * msgh = NULL;
  netmgr_exec_cmd_t * cmd_buf = NULL;
#ifdef NETMGR_QOS_ENABLED  
  netmgr_tc_flow_info_t * flow_buf = NULL;
#endif  
  #define INSTANCE 1

  sleep(1);
  
  TEST_ASSERT( NETMGR_STATE_INITED == 
               stm_get_state( netmgr_exec_state_info.links[INSTANCE].sm ) );

  
  /* Setup initial state */
  netmgr_kif_info[INSTANCE].ifi_index = 4;  // match ontarget
  netmgr_kif_info[INSTANCE].state = NETMGR_KIF_CLOSED;
  netmgr_exec_state_info.links[INSTANCE].sm->current_state = NETMGR_STATE_INITED;

  /* Generate QMI Modem network interface UP indication */
  ind_data.pkt_srvc_status.param_mask =
    (QMI_WDS_PKT_SRVC_IND_IP_FAMILY|QMI_WDS_PKT_SRVC_IND_TECH_NAME); /* No call end reason */
  ind_data.pkt_srvc_status.link_status = QMI_WDS_PACKET_DATA_CONNECTED;
  ind_data.pkt_srvc_status.reconfig_required = FALSE;
  ind_data.pkt_srvc_status.ip_family = QMI_WDS_IP_FAMILY_PREF_IPV6;
  ind_data.pkt_srvc_status.tech_name = QMI_WDS_IFACE_NAME_UTMS;
  
  netmgr_log_high("Injecting message: QMI_WDS_SRVC_PKT_SRVC_STATUS_IND_MSG\n");
  netmgr_qmi_wds_ind( INSTANCE, QMI_WDS_SERVICE, (void*)INSTANCE,
                      QMI_WDS_SRVC_PKT_SRVC_STATUS_IND_MSG, &ind_data );
  sleep(2);

  /* Check state of objects */
  TEST_ASSERT( NETMGR_KIF_OPENING == netmgr_kif_info[INSTANCE].state );
  TEST_ASSERT( NETMGR_STATE_COMING_UP ==
               stm_get_state( netmgr_exec_state_info.links[INSTANCE].sm ) );

  /* Simulate KIF_OPEN */
  cmd_buf = netmgr_exec_get_cmd();
  cmd_buf->cmd.free_f = netmgr_kif_nl_msg_recv_cmd_free;
  cmd_buf->data.type = NETMGR_KIF_MSG_CMD;
  cmd_buf->data.link = INSTANCE;
  cmd_buf->data.info.kif_msg.msg = netmgr_test_make_kif_opened( INSTANCE );
  cmd_buf->data.info.kif_msg.msglen = NL_MSG_MAX_LEN;
  netmgr_log_high("Injecting message: NETMGR_KIF_MSG_CMD - OPENED\n");
  netmgr_exec_put_cmd( cmd_buf );
  sleep(1);
  
  /* Check state of objects */
  TEST_ASSERT( NETMGR_KIF_CONFIGURING == netmgr_kif_info[INSTANCE].state );
  TEST_ASSERT( NETMGR_STATE_CONFIGURING ==
               stm_get_state( netmgr_exec_state_info.links[INSTANCE].sm ) );
  
  /* Simulate KIF_CONFIGURE */
  cmd_buf = netmgr_exec_get_cmd();
  cmd_buf->cmd.free_f = netmgr_kif_nl_msg_recv_cmd_free;
  cmd_buf->data.type = NETMGR_KIF_MSG_CMD;
  cmd_buf->data.info.kif_msg.msg = netmgr_test_make_kif_configured( INSTANCE );
  cmd_buf->data.info.kif_msg.msglen = NL_MSG_MAX_LEN;

  netmgr_log_high("Injecting message: NETMGR_KIF_MSG_CMD - CONFIGURED\n");
  netmgr_exec_put_cmd( cmd_buf );
  sleep(1);
  
  /* Check state of objects */
  TEST_ASSERT( NETMGR_KIF_OPEN == netmgr_kif_info[INSTANCE].state );
  TEST_ASSERT( NETMGR_STATE_UP ==
               stm_get_state( netmgr_exec_state_info.links[INSTANCE].sm ) );

#ifdef NETMGR_QOS_ENABLED  
  /* Validate primary flow */
  TEST_ASSERT( NULL != netmgr_tc_cfg.links[INSTANCE].flow_list );
  TEST_ASSERT( NULL != ds_dll_deq( netmgr_tc_cfg.links[INSTANCE].flow_list, NULL, (const void**)&flow_buf) );
  TEST_ASSERT( 0 == flow_buf->qos_flow.flow_id );
  TEST_ASSERT( NETMGR_TC_FLOW_ACTIVE == flow_buf->qos_flow.state );
#endif
  
  return NETMGR_SUCCESS;
}

int netmgr_test_iface_teardown_successful( void* arg )
{
  qmi_wds_indication_data_type ind_data;
  struct msghdr * msgh = NULL;
  netmgr_exec_cmd_t * cmd_buf = NULL;
  #define INSTANCE 1
  
  /* Setup initial state */
  netmgr_kif_info[INSTANCE].ifi_index = 4;
  netmgr_kif_info[INSTANCE].state = NETMGR_KIF_OPEN;
  netmgr_qmi_cfg.links[INSTANCE].wds_info.addr_info.valid_mask = NETMGR_ADDRSET_MASK_IPV4;
  netmgr_exec_state_info.links[INSTANCE].sm->current_state = NETMGR_STATE_UP;
  
  /* Generate QMI Modem network interface DOWN indication */
  ind_data.pkt_srvc_status.param_mask =
    (QMI_WDS_PKT_SRVC_IND_IP_FAMILY|QMI_WDS_PKT_SRVC_IND_TECH_NAME); /* No call end reason */
  ind_data.pkt_srvc_status.link_status = QMI_WDS_PACKET_DATA_DISCONNECTED;
  ind_data.pkt_srvc_status.reconfig_required = FALSE;
  ind_data.pkt_srvc_status.ip_family = QMI_WDS_IP_FAMILY_PREF_IPV4;
  ind_data.pkt_srvc_status.tech_name = QMI_WDS_IFACE_NAME_UTMS;
  
  netmgr_qmi_wds_ind( INSTANCE, QMI_WDS_SERVICE, (void*)INSTANCE,
                      QMI_WDS_SRVC_PKT_SRVC_STATUS_IND_MSG, &ind_data );
  sleep(1);

  /* Check state of objects */
  TEST_ASSERT( NETMGR_STATE_GOING_DOWN ==
               stm_get_state( netmgr_exec_state_info.links[INSTANCE].sm ) );
#ifdef FEATURE_DATA_LINUX_LE
  TEST_ASSERT( NETMGR_KIF_DECONFIGURING == netmgr_kif_info[INSTANCE].state );
#else
  TEST_ASSERT( NETMGR_KIF_CLOSING == netmgr_kif_info[INSTANCE].state );
#endif

  /* Simulate KIF_CLOSE */
  cmd_buf = netmgr_exec_get_cmd();
  cmd_buf->cmd.free_f = netmgr_kif_nl_msg_recv_cmd_free;
  cmd_buf->data.type = NETMGR_KIF_MSG_CMD;
  cmd_buf->data.info.kif_msg.msg = netmgr_test_make_kif_closed( INSTANCE );
  cmd_buf->data.info.kif_msg.msglen = NL_MSG_MAX_LEN;
  netmgr_exec_put_cmd( cmd_buf );
  sleep(2);

  /* Check state of objects */
  TEST_ASSERT( NETMGR_KIF_CLOSED == netmgr_kif_info[INSTANCE].state );
  TEST_ASSERT( NETMGR_STATE_INITED ==
               stm_get_state( netmgr_exec_state_info.links[INSTANCE].sm ) );

  return NETMGR_SUCCESS;
}


int netmgr_test_iface_reconfigure_successful( void* arg )
{
  qmi_wds_indication_data_type ind_data;
  struct msghdr * msgh = NULL;
  netmgr_exec_cmd_t * cmd_buf = NULL;
  #define INSTANCE 1

  /* Setup initial state */
  netmgr_kif_info[INSTANCE].ifi_index = 4;
  netmgr_kif_info[INSTANCE].state = NETMGR_KIF_OPEN;
  netmgr_qmi_cfg.links[INSTANCE].wds_info.addr_info.valid_mask = NETMGR_ADDRSET_MASK_IPV4;
  netmgr_qmi_cfg.links[INSTANCE].wds_info.addr_info.ipv4.if_addr.addr.v4 = 0x01020304;
  netmgr_qmi_cfg.links[INSTANCE].wds_info.addr_info.ipv4.if_mask = 30;
  netmgr_exec_state_info.links[INSTANCE].sm->current_state = NETMGR_STATE_UP;

  /* Generate QMI Modem network interface reconfig required */
  ind_data.pkt_srvc_status.param_mask =
    (QMI_WDS_PKT_SRVC_IND_IP_FAMILY|QMI_WDS_PKT_SRVC_IND_TECH_NAME); /* No call end reason */
  ind_data.pkt_srvc_status.link_status = QMI_WDS_PACKET_DATA_CONNECTED;
  ind_data.pkt_srvc_status.reconfig_required = TRUE;
  ind_data.pkt_srvc_status.ip_family = QMI_WDS_IP_FAMILY_PREF_IPV4;
  ind_data.pkt_srvc_status.tech_name = QMI_WDS_IFACE_NAME_UTMS;

  netmgr_qmi_wds_ind( INSTANCE, QMI_WDS_SERVICE, (void*)INSTANCE,
                      QMI_WDS_SRVC_PKT_SRVC_STATUS_IND_MSG, &ind_data );
  sleep(1);

  /* Check state of objects */
  TEST_ASSERT( NETMGR_STATE_RECONFIGURING ==
               stm_get_state( netmgr_exec_state_info.links[INSTANCE].sm ) );
  TEST_ASSERT( NETMGR_KIF_RECONFIGURING == netmgr_kif_info[INSTANCE].state );

  /* Simulate KIF_CONFIGURE */
  cmd_buf = netmgr_exec_get_cmd();
  cmd_buf->cmd.free_f = netmgr_kif_nl_msg_recv_cmd_free;
  cmd_buf->data.type = NETMGR_KIF_MSG_CMD;
  cmd_buf->data.info.kif_msg.msg = netmgr_test_make_kif_configured( INSTANCE );
  cmd_buf->data.info.kif_msg.msglen = NL_MSG_MAX_LEN;
  netmgr_exec_put_cmd( cmd_buf );
  sleep(1);
  
  /* Check state of objects */
  TEST_ASSERT( NETMGR_KIF_OPEN == netmgr_kif_info[INSTANCE].state );
  TEST_ASSERT( NETMGR_STATE_UP ==
               stm_get_state( netmgr_exec_state_info.links[INSTANCE].sm ) );

  return NETMGR_SUCCESS;
}

int netmgr_test_kif_close_when_iface_up_successful( void* arg )
{
  qmi_wds_indication_data_type ind_data;
  struct msghdr * msgh = NULL;
  netmgr_exec_cmd_t * cmd_buf = NULL;
  #define INSTANCE 1
  
  /* Setup initial state */
  netmgr_kif_info[INSTANCE].ifi_index = 4;
  netmgr_kif_info[INSTANCE].state = NETMGR_KIF_OPEN;
  netmgr_exec_state_info.links[INSTANCE].sm->current_state = NETMGR_STATE_UP;
  
  /* Simulate KIF_CLOSE */
  cmd_buf = netmgr_exec_get_cmd();
  cmd_buf->cmd.free_f = netmgr_kif_nl_msg_recv_cmd_free;
  cmd_buf->data.type = NETMGR_KIF_MSG_CMD;
  cmd_buf->data.info.kif_msg.msg = netmgr_test_make_kif_closed( INSTANCE );
  cmd_buf->data.info.kif_msg.msglen = NL_MSG_MAX_LEN;
  netmgr_exec_put_cmd( cmd_buf );
  sleep(1);
  
  /* Check state of objects */
// Assuming KIF CLOSE will close SMD port, triggering Modem to teardown WDS link.
  TEST_ASSERT( NETMGR_KIF_CLOSED == netmgr_kif_info[INSTANCE].state );
  TEST_ASSERT( NETMGR_STATE_INITED ==
               stm_get_state( netmgr_exec_state_info.links[INSTANCE].sm ) );


  return NETMGR_SUCCESS;
}

int netmgr_test_kif_close_when_iface_comingup_successful( void* arg )
{
  qmi_wds_indication_data_type ind_data;
  struct msghdr * msgh = NULL;
  netmgr_exec_cmd_t * cmd_buf = NULL;
  #define INSTANCE 1
  
  /* Setup initial state */
  netmgr_kif_info[INSTANCE].ifi_index = 4;
  netmgr_kif_info[INSTANCE].state = NETMGR_KIF_OPENING;
  netmgr_exec_state_info.links[INSTANCE].sm->current_state = NETMGR_STATE_COMING_UP;
  
  /* Simulate KIF_CLOSE */
  cmd_buf = netmgr_exec_get_cmd();
  cmd_buf->cmd.free_f = netmgr_kif_nl_msg_recv_cmd_free;
  cmd_buf->data.type = NETMGR_KIF_MSG_CMD;
  cmd_buf->data.info.kif_msg.msg = netmgr_test_make_kif_closed( INSTANCE );
  cmd_buf->data.info.kif_msg.msglen = NL_MSG_MAX_LEN;
  netmgr_exec_put_cmd( cmd_buf );
  sleep(1);
  
  /* Check state of objects */
// Assuming KIF CLOSE will close SMD port, triggering Modem to teardown WDS link.
  TEST_ASSERT( NETMGR_KIF_CLOSED == netmgr_kif_info[INSTANCE].state );
  TEST_ASSERT( NETMGR_STATE_INITED ==
               stm_get_state( netmgr_exec_state_info.links[INSTANCE].sm ) );

  return NETMGR_SUCCESS;
}

int netmgr_test_kif_close_when_iface_configuring_successful( void* arg )
{
  qmi_wds_indication_data_type ind_data;
  struct msghdr * msgh = NULL;
  netmgr_exec_cmd_t * cmd_buf = NULL;
  #define INSTANCE 1
  
  /* Setup initial state */
  netmgr_kif_info[INSTANCE].ifi_index = 4;
  netmgr_kif_info[INSTANCE].state = NETMGR_KIF_CONFIGURING;
  netmgr_exec_state_info.links[INSTANCE].sm->current_state = NETMGR_STATE_CONFIGURING;
  
  /* Simulate KIF_CLOSE */
  cmd_buf = netmgr_exec_get_cmd();
  cmd_buf->cmd.free_f = netmgr_kif_nl_msg_recv_cmd_free;
  cmd_buf->data.type = NETMGR_KIF_MSG_CMD;
  cmd_buf->data.info.kif_msg.msg = netmgr_test_make_kif_closed( INSTANCE );
  cmd_buf->data.info.kif_msg.msglen = NL_MSG_MAX_LEN;
  netmgr_exec_put_cmd( cmd_buf );
  sleep(1);
  
  /* Check state of objects */
// Assuming KIF CLOSE will close SMD port, triggering Modem to teardown WDS link.
  TEST_ASSERT( NETMGR_KIF_CLOSED == netmgr_kif_info[INSTANCE].state );
  TEST_ASSERT( NETMGR_STATE_INITED ==
               stm_get_state( netmgr_exec_state_info.links[INSTANCE].sm ) );

  return NETMGR_SUCCESS;
}

int netmgr_test_kif_close_when_iface_reconfiguring_successful( void* arg )
{
  qmi_wds_indication_data_type ind_data;
  struct msghdr * msgh = NULL;
  netmgr_exec_cmd_t * cmd_buf = NULL;
  #define INSTANCE 1
  
  /* Setup initial state */
  netmgr_kif_info[INSTANCE].ifi_index = 4;
  netmgr_kif_info[INSTANCE].state = NETMGR_KIF_RECONFIGURING;
  netmgr_exec_state_info.links[INSTANCE].sm->current_state = NETMGR_STATE_RECONFIGURING;
  
  /* Simulate KIF_CLOSE */
  cmd_buf = netmgr_exec_get_cmd();
  cmd_buf->cmd.free_f = netmgr_kif_nl_msg_recv_cmd_free;
  cmd_buf->data.type = NETMGR_KIF_MSG_CMD;
  cmd_buf->data.info.kif_msg.msg = netmgr_test_make_kif_closed( INSTANCE );
  cmd_buf->data.info.kif_msg.msglen = NL_MSG_MAX_LEN;
  netmgr_exec_put_cmd( cmd_buf );
  sleep(1);
  
  /* Check state of objects */
// Assuming KIF CLOSE will close SMD port, triggering Modem to teardown WDS link.
  TEST_ASSERT( NETMGR_KIF_CLOSED == netmgr_kif_info[INSTANCE].state );
  TEST_ASSERT( NETMGR_STATE_INITED ==
               stm_get_state( netmgr_exec_state_info.links[INSTANCE].sm ) );

  return NETMGR_SUCCESS;
}

int netmgr_test_wds_disconnect_when_iface_comingup_successful( void* arg )
{
  qmi_wds_indication_data_type ind_data;
  struct msghdr * msgh = NULL;
  netmgr_exec_cmd_t * cmd_buf = NULL;
  #define INSTANCE 1
  
  /* Setup initial state */
  netmgr_kif_info[INSTANCE].ifi_index = 4;
  netmgr_kif_info[INSTANCE].state = NETMGR_KIF_OPENING;
  netmgr_exec_state_info.links[INSTANCE].sm->current_state = NETMGR_STATE_COMING_UP;
  
  /* Generate QMI Modem network interface DOWN indication */
  ind_data.pkt_srvc_status.param_mask =
    (QMI_WDS_PKT_SRVC_IND_IP_FAMILY|QMI_WDS_PKT_SRVC_IND_TECH_NAME); /* No call end reason */
  ind_data.pkt_srvc_status.link_status = QMI_WDS_PACKET_DATA_DISCONNECTED;
  ind_data.pkt_srvc_status.reconfig_required = FALSE;
  ind_data.pkt_srvc_status.ip_family = QMI_WDS_IP_FAMILY_PREF_IPV4;
  ind_data.pkt_srvc_status.tech_name = QMI_WDS_IFACE_NAME_UTMS;
  
  netmgr_qmi_wds_ind( INSTANCE, QMI_WDS_SERVICE, (void*)INSTANCE,
                      QMI_WDS_SRVC_PKT_SRVC_STATUS_IND_MSG, &ind_data );
  sleep(1);

  /* Check state of objects */
  TEST_ASSERT( NETMGR_KIF_CLOSING == netmgr_kif_info[INSTANCE].state );
  TEST_ASSERT( NETMGR_STATE_GOING_DOWN ==
               stm_get_state( netmgr_exec_state_info.links[INSTANCE].sm ) );

  return NETMGR_SUCCESS;
}

int netmgr_test_wds_disconnect_when_iface_configuring_successful( void* arg )
{
  qmi_wds_indication_data_type ind_data;
  struct msghdr * msgh = NULL;
  netmgr_exec_cmd_t * cmd_buf = NULL;
  #define INSTANCE 1
  
  /* Setup initial state */
  netmgr_kif_info[INSTANCE].ifi_index = 4;
  netmgr_kif_info[INSTANCE].state = NETMGR_KIF_CONFIGURING;
  netmgr_exec_state_info.links[INSTANCE].sm->current_state = NETMGR_STATE_CONFIGURING;
  
  /* Generate QMI Modem network interface DOWN indication */
  ind_data.pkt_srvc_status.param_mask =
    (QMI_WDS_PKT_SRVC_IND_IP_FAMILY|QMI_WDS_PKT_SRVC_IND_TECH_NAME); /* No call end reason */
  ind_data.pkt_srvc_status.link_status = QMI_WDS_PACKET_DATA_DISCONNECTED;
  ind_data.pkt_srvc_status.reconfig_required = FALSE;
  ind_data.pkt_srvc_status.ip_family = QMI_WDS_IP_FAMILY_PREF_IPV4;
  ind_data.pkt_srvc_status.tech_name = QMI_WDS_IFACE_NAME_UTMS;
  
  netmgr_qmi_wds_ind( INSTANCE, QMI_WDS_SERVICE, (void*)INSTANCE,
                      QMI_WDS_SRVC_PKT_SRVC_STATUS_IND_MSG, &ind_data );
  sleep(1);

  /* Check state of objects */
  TEST_ASSERT( NETMGR_KIF_CLOSING == netmgr_kif_info[INSTANCE].state );
  TEST_ASSERT( NETMGR_STATE_GOING_DOWN ==
               stm_get_state( netmgr_exec_state_info.links[INSTANCE].sm ) );

  return NETMGR_SUCCESS;
}

int netmgr_test_wds_disconnect_when_iface_reconfiguring_successful( void* arg )
{
  qmi_wds_indication_data_type ind_data;
  struct msghdr * msgh = NULL;
  netmgr_exec_cmd_t * cmd_buf = NULL;
  #define INSTANCE 1
  
  /* Setup initial state */
  netmgr_kif_info[INSTANCE].ifi_index = 4;
  netmgr_kif_info[INSTANCE].state = NETMGR_KIF_RECONFIGURING;
  netmgr_exec_state_info.links[INSTANCE].sm->current_state = NETMGR_STATE_RECONFIGURING;
  
  /* Generate QMI Modem network interface DOWN indication */
  ind_data.pkt_srvc_status.param_mask =
    (QMI_WDS_PKT_SRVC_IND_IP_FAMILY|QMI_WDS_PKT_SRVC_IND_TECH_NAME); /* No call end reason */
  ind_data.pkt_srvc_status.link_status = QMI_WDS_PACKET_DATA_DISCONNECTED;
  ind_data.pkt_srvc_status.reconfig_required = FALSE;
  ind_data.pkt_srvc_status.ip_family = QMI_WDS_IP_FAMILY_PREF_IPV4;
  ind_data.pkt_srvc_status.tech_name = QMI_WDS_IFACE_NAME_UTMS;
  
  netmgr_qmi_wds_ind( INSTANCE, QMI_WDS_SERVICE, (void*)INSTANCE,
                      QMI_WDS_SRVC_PKT_SRVC_STATUS_IND_MSG, &ind_data );
  sleep(1);

  /* Check state of objects */
  TEST_ASSERT( NETMGR_KIF_CLOSING == netmgr_kif_info[INSTANCE].state );
  TEST_ASSERT( NETMGR_STATE_GOING_DOWN ==
               stm_get_state( netmgr_exec_state_info.links[INSTANCE].sm ) );

  return NETMGR_SUCCESS;
}

int netmgr_test_wds_connect_when_iface_goingdown_successful(void * arg )
{
  qmi_wds_indication_data_type ind_data;
  struct msghdr * msgh = NULL;
  netmgr_exec_cmd_t * cmd_buf = NULL;
  #define INSTANCE 1
  
  /* Setup initial state */
  netmgr_kif_info[INSTANCE].ifi_index = 4;
  netmgr_kif_info[INSTANCE].state = NETMGR_KIF_CLOSING;
  netmgr_exec_state_info.links[INSTANCE].sm->current_state = NETMGR_STATE_GOING_DOWN;
  
  /* Generate QMI Modem network interface DOWN indication */
  ind_data.pkt_srvc_status.param_mask =
    (QMI_WDS_PKT_SRVC_IND_IP_FAMILY|QMI_WDS_PKT_SRVC_IND_TECH_NAME); /* No call end reason */
  ind_data.pkt_srvc_status.link_status = QMI_WDS_PACKET_DATA_CONNECTED;
  ind_data.pkt_srvc_status.reconfig_required = FALSE;
  ind_data.pkt_srvc_status.ip_family = QMI_WDS_IP_FAMILY_PREF_IPV4;
  ind_data.pkt_srvc_status.tech_name = QMI_WDS_IFACE_NAME_UTMS;
  
  netmgr_qmi_wds_ind( INSTANCE, QMI_WDS_SERVICE, (void*)INSTANCE,
                      QMI_WDS_SRVC_PKT_SRVC_STATUS_IND_MSG, &ind_data );
  sleep(1);

  /* Check state of objects */
  TEST_ASSERT( NETMGR_KIF_CLOSING == netmgr_kif_info[INSTANCE].state );
  TEST_ASSERT( NETMGR_STATE_GOING_DOWN_TO_COME_UP ==
               stm_get_state( netmgr_exec_state_info.links[INSTANCE].sm ) );

  return NETMGR_SUCCESS;
}

int netmgr_test_iface_dualIP_firstIPremoval_successful( void* arg )
{
  qmi_wds_indication_data_type ind_data;
  struct msghdr * msgh = NULL;
  netmgr_exec_cmd_t * cmd_buf = NULL;
  #define INSTANCE 1

  /* Setup initial state */
  netmgr_kif_info[INSTANCE].ifi_index = 4;
  netmgr_kif_info[INSTANCE].state = NETMGR_KIF_OPEN;
  /* dual-IP call active */
  netmgr_qmi_cfg.links[INSTANCE].wds_info.addr_info.valid_mask = NETMGR_ADDRSET_MASK_IPV4V6;
  netmgr_exec_state_info.links[INSTANCE].sm->current_state = NETMGR_STATE_UP;

  /* Generate QMI Modem network interface DOWN indication */
  ind_data.pkt_srvc_status.param_mask =
    (QMI_WDS_PKT_SRVC_IND_IP_FAMILY|QMI_WDS_PKT_SRVC_IND_TECH_NAME); /* No call end reason */
  ind_data.pkt_srvc_status.link_status = QMI_WDS_PACKET_DATA_DISCONNECTED;
  ind_data.pkt_srvc_status.reconfig_required = FALSE;
  ind_data.pkt_srvc_status.ip_family = QMI_WDS_IP_FAMILY_PREF_IPV4;
  ind_data.pkt_srvc_status.tech_name = QMI_WDS_IFACE_NAME_UTMS;
  
  netmgr_qmi_wds_ind( INSTANCE, QMI_WDS_SERVICE, (void*)INSTANCE,
                      QMI_WDS_SRVC_PKT_SRVC_STATUS_IND_MSG, &ind_data );
  sleep(1);

  /* Check state of objects */
  TEST_ASSERT( NETMGR_STATE_UP ==
               stm_get_state( netmgr_exec_state_info.links[INSTANCE].sm ) );
  TEST_ASSERT( NETMGR_KIF_OPEN == netmgr_kif_info[INSTANCE].state );

  return NETMGR_SUCCESS;
}

int netmgr_test_iface_dualIP_secondIPremoval_successful( void* arg )
{
  qmi_wds_indication_data_type ind_data;
  struct msghdr * msgh = NULL;
  netmgr_exec_cmd_t * cmd_buf = NULL;
  #define INSTANCE 1

  /* Setup initial state */
  netmgr_kif_info[INSTANCE].ifi_index = 4;
  netmgr_kif_info[INSTANCE].state = NETMGR_KIF_OPEN;
  /* dual-IP call active */
  netmgr_qmi_cfg.links[INSTANCE].wds_info.addr_info.valid_mask = NETMGR_ADDRSET_MASK_IPV4V6;
  netmgr_exec_state_info.links[INSTANCE].sm->current_state = NETMGR_STATE_UP;

  /* Generate QMI Modem network interface DOWN indication */
  ind_data.pkt_srvc_status.param_mask =
    (QMI_WDS_PKT_SRVC_IND_IP_FAMILY|QMI_WDS_PKT_SRVC_IND_TECH_NAME); /* No call end reason */
  ind_data.pkt_srvc_status.link_status = QMI_WDS_PACKET_DATA_DISCONNECTED;
  ind_data.pkt_srvc_status.reconfig_required = FALSE;
  ind_data.pkt_srvc_status.ip_family = QMI_WDS_IP_FAMILY_PREF_IPV4;
  ind_data.pkt_srvc_status.tech_name = QMI_WDS_IFACE_NAME_UTMS;

  netmgr_qmi_wds_ind( INSTANCE, QMI_WDS_SERVICE, (void*)INSTANCE,
                      QMI_WDS_SRVC_PKT_SRVC_STATUS_IND_MSG, &ind_data );
  sleep(1);

  /* Check state of objects */
  TEST_ASSERT( NETMGR_STATE_UP ==
               stm_get_state( netmgr_exec_state_info.links[INSTANCE].sm ) );
  TEST_ASSERT( NETMGR_KIF_OPEN == netmgr_kif_info[INSTANCE].state );

  /* Generate QMI Modem network interface DOWN indication */
  ind_data.pkt_srvc_status.param_mask =
    (QMI_WDS_PKT_SRVC_IND_IP_FAMILY|QMI_WDS_PKT_SRVC_IND_TECH_NAME); /* No call end reason */
  ind_data.pkt_srvc_status.link_status = QMI_WDS_PACKET_DATA_DISCONNECTED;
  ind_data.pkt_srvc_status.reconfig_required = FALSE;
  ind_data.pkt_srvc_status.ip_family = QMI_WDS_IP_FAMILY_PREF_IPV6;
  ind_data.pkt_srvc_status.tech_name = QMI_WDS_IFACE_NAME_UTMS;
  
  netmgr_qmi_wds_ind( INSTANCE, QMI_WDS_SERVICE, (void*)INSTANCE,
                      QMI_WDS_SRVC_PKT_SRVC_STATUS_IND_MSG, &ind_data );
  sleep(1);

  /* Check state of objects */
  TEST_ASSERT( NETMGR_STATE_GOING_DOWN ==
               stm_get_state( netmgr_exec_state_info.links[INSTANCE].sm ) );
#ifdef FEATURE_DATA_LINUX_LE
  TEST_ASSERT( NETMGR_KIF_DECONFIGURING == netmgr_kif_info[INSTANCE].state );
#else
  TEST_ASSERT( NETMGR_KIF_CLOSING == netmgr_kif_info[INSTANCE].state );
#endif

  /* Simulate KIF_CLOSE */
  cmd_buf = netmgr_exec_get_cmd();
  cmd_buf->cmd.free_f = netmgr_kif_nl_msg_recv_cmd_free;
  cmd_buf->data.type = NETMGR_KIF_MSG_CMD;
  cmd_buf->data.info.kif_msg.msg = netmgr_test_make_kif_closed( INSTANCE );
  cmd_buf->data.info.kif_msg.msglen = NL_MSG_MAX_LEN;
  netmgr_exec_put_cmd( cmd_buf );
  sleep(2);

  /* Check state of objects */
  TEST_ASSERT( NETMGR_KIF_CLOSED == netmgr_kif_info[INSTANCE].state );
  TEST_ASSERT( NETMGR_STATE_INITED ==
               stm_get_state( netmgr_exec_state_info.links[INSTANCE].sm ) );

  return NETMGR_SUCCESS;
}

int netmgr_test_iface_dualIP_late_ipv4_connect_successful( void* arg )
{
  qmi_wds_indication_data_type ind_data;
  struct msghdr * msgh = NULL;
  netmgr_exec_cmd_t * cmd_buf = NULL;
#ifdef NETMGR_QOS_ENABLED
  netmgr_tc_flow_info_t * flow_buf = NULL;
#endif
  #define INSTANCE 1

  /* Setup initial state */
  netmgr_kif_info[INSTANCE].ifi_index = 4;  // match ontarget
  netmgr_kif_info[INSTANCE].state = NETMGR_KIF_CLOSED;
  netmgr_exec_state_info.links[INSTANCE].sm->current_state = NETMGR_STATE_INITED;

  /* Generate QMI Modem IPV6 network interface UP indication */
  netmgr_qmi_cfg.links[INSTANCE].wds_info.clnt_hdl = 6;
  ind_data.pkt_srvc_status.param_mask =
    (QMI_WDS_PKT_SRVC_IND_IP_FAMILY|QMI_WDS_PKT_SRVC_IND_TECH_NAME); /* No call end reason */
  ind_data.pkt_srvc_status.link_status = QMI_WDS_PACKET_DATA_CONNECTED;
  ind_data.pkt_srvc_status.reconfig_required = FALSE;
  ind_data.pkt_srvc_status.ip_family = QMI_WDS_IP_FAMILY_PREF_IPV6;
  ind_data.pkt_srvc_status.tech_name = QMI_WDS_IFACE_NAME_UTMS;

  netmgr_log_high("Injecting message: QMI_WDS_SRVC_PKT_SRVC_STATUS_IND_MSG [IPV6]\n");
  netmgr_qmi_wds_ind( INSTANCE, QMI_WDS_SERVICE, (void*)INSTANCE,
                      QMI_WDS_SRVC_PKT_SRVC_STATUS_IND_MSG, &ind_data );
  sleep(1);

  /* Check state of objects */
  TEST_ASSERT( NETMGR_KIF_OPENING == netmgr_kif_info[INSTANCE].state );
  TEST_ASSERT( NETMGR_STATE_COMING_UP ==
               stm_get_state( netmgr_exec_state_info.links[INSTANCE].sm ) );

  /* Simulate KIF_OPEN */
  cmd_buf = netmgr_exec_get_cmd();
  cmd_buf->cmd.free_f = netmgr_kif_nl_msg_recv_cmd_free;
  cmd_buf->data.type = NETMGR_KIF_MSG_CMD;
  cmd_buf->data.link = INSTANCE;
  cmd_buf->data.info.kif_msg.msg = netmgr_test_make_kif_opened( INSTANCE );
  cmd_buf->data.info.kif_msg.msglen = NL_MSG_MAX_LEN;
  netmgr_log_high("Injecting message: NETMGR_KIF_MSG_CMD - OPENED\n");
  netmgr_exec_put_cmd( cmd_buf );
  sleep(1);

  /* Check state of objects */
  TEST_ASSERT( NETMGR_KIF_CONFIGURING == netmgr_kif_info[INSTANCE].state );
  TEST_ASSERT( NETMGR_STATE_CONFIGURING ==
               stm_get_state( netmgr_exec_state_info.links[INSTANCE].sm ) );

  /* Generate QMI Modem IPV4 network interface UP indication */
  netmgr_qmi_cfg.links[INSTANCE].wds_info.clnt_hdl = 4;
  ind_data.pkt_srvc_status.param_mask =
    (QMI_WDS_PKT_SRVC_IND_IP_FAMILY|QMI_WDS_PKT_SRVC_IND_TECH_NAME); /* No call end reason */
  ind_data.pkt_srvc_status.link_status = QMI_WDS_PACKET_DATA_CONNECTED;
  ind_data.pkt_srvc_status.reconfig_required = FALSE;
  ind_data.pkt_srvc_status.ip_family = QMI_WDS_IP_FAMILY_PREF_IPV4;
  ind_data.pkt_srvc_status.tech_name = QMI_WDS_IFACE_NAME_UTMS;

  netmgr_log_high("Injecting message: QMI_WDS_SRVC_PKT_SRVC_STATUS_IND_MSG [IPV4]\n");
  netmgr_qmi_wds_ind( INSTANCE, QMI_WDS_SERVICE, (void*)INSTANCE,
                      QMI_WDS_SRVC_PKT_SRVC_STATUS_IND_MSG, &ind_data );
  sleep(1);

  /* Check state of objects */
  TEST_ASSERT( NETMGR_KIF_CONFIGURING == netmgr_kif_info[INSTANCE].state );
  TEST_ASSERT( NETMGR_STATE_CONFIGURING ==
               stm_get_state( netmgr_exec_state_info.links[INSTANCE].sm ) );

  /* Simulate KIF_CONFIGURE */
  cmd_buf = netmgr_exec_get_cmd();
  cmd_buf->cmd.free_f = netmgr_kif_nl_msg_recv_cmd_free;
  cmd_buf->data.type = NETMGR_KIF_MSG_CMD;
  cmd_buf->data.info.kif_msg.msg = netmgr_test_make_kif_configured( INSTANCE );
  cmd_buf->data.info.kif_msg.msglen = NL_MSG_MAX_LEN;

  netmgr_log_high("Injecting message: NETMGR_KIF_MSG_CMD - CONFIGURED\n");
  netmgr_exec_put_cmd( cmd_buf );
  sleep(1);

  /* Check state of objects */
  TEST_ASSERT( NETMGR_KIF_OPEN == netmgr_kif_info[INSTANCE].state );
  TEST_ASSERT( NETMGR_STATE_UP ==
               stm_get_state( netmgr_exec_state_info.links[INSTANCE].sm ) );

#ifdef NETMGR_QOS_ENABLED
  /* Validate primary flow */
  TEST_ASSERT( NULL != netmgr_tc_cfg.links[INSTANCE].flow_list );
  TEST_ASSERT( NULL != ds_dll_deq( netmgr_tc_cfg.links[INSTANCE].flow_list, NULL, (const void**)&flow_buf) );
  TEST_ASSERT( 0 == flow_buf->qos_flow.flow_id );
  TEST_ASSERT( NETMGR_TC_FLOW_ACTIVE == flow_buf->qos_flow.state );
#endif

  return NETMGR_SUCCESS;
}

int netmgr_test_iface_dualIP_late_ipv6_connect_successful( void* arg )
{
  qmi_wds_indication_data_type ind_data;
  struct msghdr * msgh = NULL;
  netmgr_exec_cmd_t * cmd_buf = NULL;
#ifdef NETMGR_QOS_ENABLED
  netmgr_tc_flow_info_t * flow_buf = NULL;
#endif
  #define INSTANCE 1

  /* Setup initial state */
  netmgr_kif_info[INSTANCE].ifi_index = 4;  // match ontarget
  netmgr_kif_info[INSTANCE].state = NETMGR_KIF_CLOSED;
  netmgr_exec_state_info.links[INSTANCE].sm->current_state = NETMGR_STATE_INITED;

  /* Generate QMI Modem IPV4 network interface UP indication */
  netmgr_qmi_cfg.links[INSTANCE].wds_info.clnt_hdl = 4;
  ind_data.pkt_srvc_status.param_mask =
    (QMI_WDS_PKT_SRVC_IND_IP_FAMILY|QMI_WDS_PKT_SRVC_IND_TECH_NAME); /* No call end reason */
  ind_data.pkt_srvc_status.link_status = QMI_WDS_PACKET_DATA_CONNECTED;
  ind_data.pkt_srvc_status.reconfig_required = FALSE;
  ind_data.pkt_srvc_status.ip_family = QMI_WDS_IP_FAMILY_PREF_IPV4;
  ind_data.pkt_srvc_status.tech_name = QMI_WDS_IFACE_NAME_UTMS;

  netmgr_log_high("Injecting message: QMI_WDS_SRVC_PKT_SRVC_STATUS_IND_MSG [IPV4]\n");
  netmgr_qmi_wds_ind( INSTANCE, QMI_WDS_SERVICE, (void*)INSTANCE,
                      QMI_WDS_SRVC_PKT_SRVC_STATUS_IND_MSG, &ind_data );
  sleep(1);

  /* Check state of objects */
  TEST_ASSERT( NETMGR_KIF_OPENING == netmgr_kif_info[INSTANCE].state );
  TEST_ASSERT( NETMGR_STATE_COMING_UP ==
               stm_get_state( netmgr_exec_state_info.links[INSTANCE].sm ) );

  /* Simulate KIF_OPEN */
  cmd_buf = netmgr_exec_get_cmd();
  cmd_buf->cmd.free_f = netmgr_kif_nl_msg_recv_cmd_free;
  cmd_buf->data.type = NETMGR_KIF_MSG_CMD;
  cmd_buf->data.link = INSTANCE;
  cmd_buf->data.info.kif_msg.msg = netmgr_test_make_kif_opened( INSTANCE );
  cmd_buf->data.info.kif_msg.msglen = NL_MSG_MAX_LEN;
  netmgr_log_high("Injecting message: NETMGR_KIF_MSG_CMD - OPENED\n");
  netmgr_exec_put_cmd( cmd_buf );
  sleep(1);

  /* Check state of objects */
  TEST_ASSERT( NETMGR_KIF_CONFIGURING == netmgr_kif_info[INSTANCE].state );
  TEST_ASSERT( NETMGR_STATE_CONFIGURING ==
               stm_get_state( netmgr_exec_state_info.links[INSTANCE].sm ) );

    /* Generate QMI Modem IPV6 network interface UP indication */
  netmgr_qmi_cfg.links[INSTANCE].wds_info.clnt_hdl = 6;
  ind_data.pkt_srvc_status.param_mask =
    (QMI_WDS_PKT_SRVC_IND_IP_FAMILY|QMI_WDS_PKT_SRVC_IND_TECH_NAME); /* No call end reason */
  ind_data.pkt_srvc_status.link_status = QMI_WDS_PACKET_DATA_CONNECTED;
  ind_data.pkt_srvc_status.reconfig_required = FALSE;
  ind_data.pkt_srvc_status.ip_family = QMI_WDS_IP_FAMILY_PREF_IPV6;
  ind_data.pkt_srvc_status.tech_name = QMI_WDS_IFACE_NAME_UTMS;

  netmgr_log_high("Injecting message: QMI_WDS_SRVC_PKT_SRVC_STATUS_IND_MSG [IPV6]\n");
  netmgr_qmi_wds_ind( INSTANCE, QMI_WDS_SERVICE, (void*)INSTANCE,
                      QMI_WDS_SRVC_PKT_SRVC_STATUS_IND_MSG, &ind_data );
  sleep(1);

  /* Check state of objects */
  TEST_ASSERT( NETMGR_KIF_CONFIGURING == netmgr_kif_info[INSTANCE].state );
  TEST_ASSERT( NETMGR_STATE_CONFIGURING ==
               stm_get_state( netmgr_exec_state_info.links[INSTANCE].sm ) );

  /* Simulate KIF_CONFIGURE */
  cmd_buf = netmgr_exec_get_cmd();
  cmd_buf->cmd.free_f = netmgr_kif_nl_msg_recv_cmd_free;
  cmd_buf->data.type = NETMGR_KIF_MSG_CMD;
  cmd_buf->data.info.kif_msg.msg = netmgr_test_make_kif_configured( INSTANCE );
  cmd_buf->data.info.kif_msg.msglen = NL_MSG_MAX_LEN;

  netmgr_log_high("Injecting message: NETMGR_KIF_MSG_CMD - CONFIGURED\n");
  netmgr_exec_put_cmd( cmd_buf );
  sleep(1);

  /* Check state of objects */
  TEST_ASSERT( NETMGR_KIF_OPEN == netmgr_kif_info[INSTANCE].state );
  TEST_ASSERT( NETMGR_STATE_UP ==
               stm_get_state( netmgr_exec_state_info.links[INSTANCE].sm ) );

#ifdef NETMGR_QOS_ENABLED
  /* Validate primary flow */
  TEST_ASSERT( NULL != netmgr_tc_cfg.links[INSTANCE].flow_list );
  TEST_ASSERT( NULL != ds_dll_deq( netmgr_tc_cfg.links[INSTANCE].flow_list, NULL, (const void**)&flow_buf) );
  TEST_ASSERT( 0 == flow_buf->qos_flow.flow_id );
  TEST_ASSERT( NETMGR_TC_FLOW_ACTIVE == flow_buf->qos_flow.state );
#endif

  return NETMGR_SUCCESS;
}


int netmgr_test_modem_oos_is_various_stages( void* arg )
{
  sleep(1);
  #define INSTANCE 1

  netmgr_log_med("current nemgr state is %d\n", (int)stm_get_state( netmgr_exec_state_info.links[INSTANCE].sm));
  
  TEST_ASSERT( NETMGR_STATE_INITED == 
               stm_get_state( netmgr_exec_state_info.links[INSTANCE].sm ) );

  /* Setup initial state */
  netmgr_kif_info[INSTANCE].ifi_index = 4;
  netmgr_kif_info[INSTANCE].state = NETMGR_KIF_CLOSED;
  netmgr_exec_state_info.links[INSTANCE].sm->current_state = NETMGR_STATE_INITED;
  
  netmgr_qmi_sys_cb( QMI_SYS_EVENT_MODEM_OUT_OF_SERVICE_IND,
                     NULL,
                     NULL);

  sleep(1);

  netmgr_log_med("current nemgr state is %d\n", (int)stm_get_state( netmgr_exec_state_info.links[INSTANCE].sm));

  /* Check state of objects */
  TEST_ASSERT( NETMGR_KIF_CLOSED == netmgr_kif_info[INSTANCE].state );
  TEST_ASSERT( NETMGR_STATE_DOWN ==
               stm_get_state( netmgr_exec_state_info.links[INSTANCE].sm ) );

  /* make sure packet srvc ind connected is ignored in down state */
  netmgr_test_make_qmi_connected(INSTANCE);

  /* wait for above event to be processed by exec */
  sleep(1);

  /* kif must be in closed state */
  TEST_ASSERT( NETMGR_KIF_CLOSED == netmgr_kif_info[INSTANCE].state );
  TEST_ASSERT( NETMGR_STATE_DOWN ==
               stm_get_state( netmgr_exec_state_info.links[INSTANCE].sm ) );

  /* bring modem back in server */
  netmgr_qmi_sys_cb( QMI_SYS_EVENT_MODEM_IN_SERVICE_IND,
                     NULL,
                     NULL);
  sleep(1);

  /* Check state of objects */
  TEST_ASSERT( NETMGR_KIF_CLOSED == netmgr_kif_info[INSTANCE].state );
  TEST_ASSERT( NETMGR_STATE_INITED ==
               stm_get_state( netmgr_exec_state_info.links[INSTANCE].sm ) );

  /* make sure packet srvc ind connected is ignored in down state */
  netmgr_test_make_qmi_connected(INSTANCE);

  /* wait for above event to be processed by exec */
  sleep(2);

  /* kif must be in opening state */
  TEST_ASSERT( NETMGR_KIF_OPENING == netmgr_kif_info[INSTANCE].state );
  TEST_ASSERT( NETMGR_STATE_COMING_UP ==
               stm_get_state( netmgr_exec_state_info.links[INSTANCE].sm ) );


  /* put modem down again */
  netmgr_qmi_sys_cb( QMI_SYS_EVENT_MODEM_OUT_OF_SERVICE_IND,
                     NULL,
                     NULL);

  sleep(2);

  /* verify state transition */
  TEST_ASSERT( NETMGR_KIF_CLOSED == netmgr_kif_info[INSTANCE].state );
  TEST_ASSERT( NETMGR_STATE_DOWN ==
               stm_get_state( netmgr_exec_state_info.links[INSTANCE].sm ) );

  /* bring modem back in server */
  netmgr_qmi_sys_cb( QMI_SYS_EVENT_MODEM_IN_SERVICE_IND,
                     NULL,
                     NULL);
  sleep(1);

  /* Check state of objects */
  TEST_ASSERT( NETMGR_KIF_CLOSED == netmgr_kif_info[INSTANCE].state );
  TEST_ASSERT( NETMGR_STATE_INITED ==
               stm_get_state( netmgr_exec_state_info.links[INSTANCE].sm ) );


  /* make sure packet srvc ind connected is ignored in down state */
  netmgr_test_make_qmi_connected(INSTANCE);

  /* wait for above event to be processed by exec */
  sleep(2);

  /* kif must be in opening state */
  TEST_ASSERT( NETMGR_KIF_OPENING == netmgr_kif_info[INSTANCE].state );
  TEST_ASSERT( NETMGR_STATE_COMING_UP ==
               stm_get_state( netmgr_exec_state_info.links[INSTANCE].sm ) );

  /* make kif opened */
  netmgr_test_kif_opened(INSTANCE);
  sleep(2);

  /* Check state of objects */
  TEST_ASSERT( NETMGR_KIF_CONFIGURING == netmgr_kif_info[INSTANCE].state );
  TEST_ASSERT( NETMGR_STATE_CONFIGURING ==
               stm_get_state( netmgr_exec_state_info.links[INSTANCE].sm ) );
  

  /* put modem down again */
  netmgr_qmi_sys_cb( QMI_SYS_EVENT_MODEM_OUT_OF_SERVICE_IND,
                     NULL,
                     NULL);

  sleep(2);

  /* verify state transition */
  TEST_ASSERT( NETMGR_KIF_CLOSED == netmgr_kif_info[INSTANCE].state );
  TEST_ASSERT( NETMGR_STATE_DOWN ==
               stm_get_state( netmgr_exec_state_info.links[INSTANCE].sm ) );

  /* bring modem back in server */
  netmgr_qmi_sys_cb( QMI_SYS_EVENT_MODEM_IN_SERVICE_IND,
                     NULL,
                     NULL);
  sleep(1);

  /* Check state of objects */
  TEST_ASSERT( NETMGR_KIF_CLOSED == netmgr_kif_info[INSTANCE].state );
  TEST_ASSERT( NETMGR_STATE_INITED ==
               stm_get_state( netmgr_exec_state_info.links[INSTANCE].sm ) );

  /* make sure packet srvc ind connected is ignored in down state */
  netmgr_test_make_qmi_connected(INSTANCE);

  /* wait for above event to be processed by exec */
  sleep(2);

  /* kif must be in opening state */
  TEST_ASSERT( NETMGR_KIF_OPENING == netmgr_kif_info[INSTANCE].state );
  TEST_ASSERT( NETMGR_STATE_COMING_UP ==
               stm_get_state( netmgr_exec_state_info.links[INSTANCE].sm ) );

  /* make kif opened */
  netmgr_test_kif_opened(INSTANCE);
  sleep(2);

  /* Check state of objects */
  TEST_ASSERT( NETMGR_KIF_CONFIGURING == netmgr_kif_info[INSTANCE].state );
  TEST_ASSERT( NETMGR_STATE_CONFIGURING ==
               stm_get_state( netmgr_exec_state_info.links[INSTANCE].sm ) );
  

  /* make kif configured */
  netmgr_test_kif_configured(INSTANCE);
  sleep(2);

  /* Check state of objects */
  TEST_ASSERT( NETMGR_KIF_OPEN == netmgr_kif_info[INSTANCE].state );
  TEST_ASSERT( NETMGR_STATE_UP ==
               stm_get_state( netmgr_exec_state_info.links[INSTANCE].sm ) );
  

  return NETMGR_SUCCESS;
}


int netmgr_test_kif_worker_thread_stop_successful( void* arg )
{
  qmi_wds_indication_data_type ind_data;
  struct msghdr * msgh = NULL;
  netmgr_exec_cmd_t * cmd_buf = NULL;
#ifdef NETMGR_QOS_ENABLED  
  netmgr_tc_flow_info_t * flow_buf = NULL;
#endif  
  #define INSTANCE 1

  sleep(1);
  
  TEST_ASSERT( NETMGR_STATE_INITED == 
               stm_get_state( netmgr_exec_state_info.links[INSTANCE].sm ) );

  
  /* Setup initial state */
  netmgr_kif_info[INSTANCE].ifi_index = 4;  // match ontarget
  netmgr_kif_info[INSTANCE].state = NETMGR_KIF_CLOSED;
  netmgr_exec_state_info.links[INSTANCE].sm->current_state = NETMGR_STATE_INITED;

  /* Generate QMI Modem network interface UP indication */
  ind_data.pkt_srvc_status.param_mask =
    (QMI_WDS_PKT_SRVC_IND_IP_FAMILY|QMI_WDS_PKT_SRVC_IND_TECH_NAME); /* No call end reason */
  ind_data.pkt_srvc_status.link_status = QMI_WDS_PACKET_DATA_CONNECTED;
  ind_data.pkt_srvc_status.reconfig_required = FALSE;
  ind_data.pkt_srvc_status.ip_family = QMI_WDS_IP_FAMILY_PREF_IPV4;
  ind_data.pkt_srvc_status.tech_name = QMI_WDS_IFACE_NAME_UTMS;
  
  netmgr_log_high("Injecting message: QMI_WDS_SRVC_PKT_SRVC_STATUS_IND_MSG\n");
  netmgr_qmi_wds_ind( INSTANCE, QMI_WDS_SERVICE, (void*)INSTANCE,
                      QMI_WDS_SRVC_PKT_SRVC_STATUS_IND_MSG, &ind_data );
  sleep(2);

  /* Check state of objects */
  TEST_ASSERT( NETMGR_KIF_OPENING == netmgr_kif_info[INSTANCE].state );
  TEST_ASSERT( NETMGR_STATE_COMING_UP ==
               stm_get_state( netmgr_exec_state_info.links[INSTANCE].sm ) );

  /* Simulate KIF_OPEN */
  cmd_buf = netmgr_exec_get_cmd();
  cmd_buf->cmd.free_f = netmgr_kif_nl_msg_recv_cmd_free;
  cmd_buf->data.type = NETMGR_KIF_MSG_CMD;
  cmd_buf->data.link = INSTANCE;
  cmd_buf->data.info.kif_msg.msg = netmgr_test_make_kif_opened( INSTANCE );
  cmd_buf->data.info.kif_msg.msglen = NL_MSG_MAX_LEN;
  netmgr_log_high("Injecting message: NETMGR_KIF_MSG_CMD - OPENED\n");
  netmgr_exec_put_cmd( cmd_buf );
  sleep(1);
  
  /* Check state of objects */
  TEST_ASSERT( NETMGR_KIF_CONFIGURING == netmgr_kif_info[INSTANCE].state );
  TEST_ASSERT( NETMGR_STATE_CONFIGURING ==
               stm_get_state( netmgr_exec_state_info.links[INSTANCE].sm ) );
  
  /* Simulate KIF_CONFIGURE */
  cmd_buf = netmgr_exec_get_cmd();
  cmd_buf->cmd.free_f = netmgr_kif_nl_msg_recv_cmd_free;
  cmd_buf->data.type = NETMGR_KIF_MSG_CMD;
  cmd_buf->data.info.kif_msg.msg = netmgr_test_make_kif_configured( INSTANCE );
  cmd_buf->data.info.kif_msg.msglen = NL_MSG_MAX_LEN;

  netmgr_log_high("Injecting message: NETMGR_KIF_MSG_CMD - CONFIGURED\n");
  netmgr_exec_put_cmd( cmd_buf );
  sleep(1);
  
  /* Check state of objects (assume DHCP thread hung) */
  TEST_ASSERT( NETMGR_KIF_OPEN == netmgr_kif_info[INSTANCE].state );
  TEST_ASSERT( NETMGR_STATE_CONFIGURING ==
               stm_get_state( netmgr_exec_state_info.links[INSTANCE].sm ) );

  TEST_ASSERT( NETMGR_KIF_WTHREAD_STATE_RUNNING == netmgr_kif_info[INSTANCE].wtlist.workers[NETMGR_KIF_WTHREAD_DHCP].state );
  TEST_ASSERT( NETMGR_KIF_THRDID_NULL != netmgr_kif_info[INSTANCE].wtlist.workers[NETMGR_KIF_WTHREAD_DHCP].thrd_id );
  
  return NETMGR_SUCCESS;
}


int netmgr_test_kif_update_mtu( void* arg )
{

  qmi_wds_indication_data_type ind_data;
  struct msghdr * msgh = NULL;
  netmgr_exec_cmd_t * cmd_buf = NULL;
  #define INSTANCE 1

  /* Setup initial state */
  netmgr_kif_info[INSTANCE].ifi_index = 4;
  netmgr_kif_info[INSTANCE].state = NETMGR_KIF_OPEN;
  netmgr_exec_state_info.links[INSTANCE].sm->current_state = NETMGR_STATE_UP;

  /* Generate QMI Modem network interface UP indication */
  ind_data.event_report.event_mask = QMI_WDS_EVENT_BEARER_TECH_IND;

  ind_data.event_report.data_bearer_tech_type.current_db_nw == QMI_WDS_CDMA_TYPE;
  ind_data.event_report.data_bearer_tech_type.db_so_mask.so_mask_1x = CDMA_1X_IS2000_REL_A;
  ind_data.event_report.data_bearer_tech_type.rat_mask.cdma_rat_mask = CDMA_EVDO_REVA;

  netmgr_log_high( "Injecting event message: QMI_WDS_SRVC_EVENT_REPORT_IND_MSG\n" );
  netmgr_qmi_wds_ind( INSTANCE, QMI_WDS_SERVICE, (void*)INSTANCE,
                      QMI_WDS_SRVC_EVENT_REPORT_IND_MSG, &ind_data );

  sleep(2);

  TEST_ASSERT(MTU == test_mtu);

  return NETMGR_SUCCESS;
}

int netmgr_test_wds_when_iface_is_embms(void* arg )
{
  qmi_wds_indication_data_type ind_data;
  struct msghdr *msgh = NULL;
  netmgr_exec_cmd_t * cmd_buf = NULL;
  #define INSTANCE 1

  /* Setup initial state */
#if 0
  netmgr_kif_info[INSTANCE].ifi_index = 4;
  netmgr_kif_info[INSTANCE].state = NETMGR_KIF_OPENING;
  netmgr_exec_state_info.links[INSTANCE].sm->current_state = NETMGR_STATE_COMING_UP;
#else
  netmgr_kif_info[INSTANCE].ifi_index = 4;  // match ontarget
  netmgr_kif_info[INSTANCE].state = NETMGR_KIF_CLOSED;
  netmgr_exec_state_info.links[INSTANCE].sm->current_state = NETMGR_STATE_INITED;
#endif

  /* Generate QMI Modem PKT_SRV_IND indication */
  ind_data.pkt_srvc_status.param_mask = QMI_WDS_PKT_SRVC_IND_TECH_NAME;
  ind_data.pkt_srvc_status.tech_name = QMI_WDS_IFACE_NAME_EMBMS;
  ind_data.pkt_srvc_status.link_status = QMI_WDS_PACKET_DATA_CONNECTED;
  ind_data.pkt_srvc_status.reconfig_required = FALSE;
  
  netmgr_log_high("Injecting message: QMI_WDS_SRVC_PKT_SRVC_STATUS_IND_MSG\n");
  netmgr_qmi_wds_ind( INSTANCE, QMI_WDS_SERVICE, (void*)INSTANCE,
                      QMI_WDS_SRVC_PKT_SRVC_STATUS_IND_MSG, &ind_data );

  sleep(2);

  /* Simulate KIF_OPEN */
  cmd_buf = netmgr_exec_get_cmd();
  cmd_buf->cmd.free_f = netmgr_kif_nl_msg_recv_cmd_free;
  cmd_buf->data.type = NETMGR_KIF_MSG_CMD;
  cmd_buf->data.link = INSTANCE;
  cmd_buf->data.info.kif_msg.msg = netmgr_test_make_kif_opened( INSTANCE );
  cmd_buf->data.info.kif_msg.msglen = NL_MSG_MAX_LEN;
  netmgr_log_high("Injecting message: NETMGR_KIF_MSG_CMD - OPENED\n");
  netmgr_exec_put_cmd( cmd_buf );
  sleep(1);

  /* Check state of objects */
  TEST_ASSERT( NETMGR_KIF_OPENING == netmgr_kif_info[INSTANCE].state );
  TEST_ASSERT( NETMGR_STATE_UP ==
               stm_get_state( netmgr_exec_state_info.links[INSTANCE].sm ) );

  return NETMGR_SUCCESS;
}

#ifdef NETMGR_QOS_ENABLED  

int netmgr_test_qos_create_umts_sucessful( void* arg )
{
  qmi_qos_indication_data_type ind_data;
  qmi_qos_event_report_flow_info_type *flow_info = &ind_data.event_report.flow_info;
  struct msghdr * msgh = NULL;
  netmgr_exec_cmd_t * cmd_buf = NULL;
  netmgr_tc_flow_info_t * flow_buf = NULL;
//  netmgr_tc_filter_info_t *filter_buf = NULL;
  #define INSTANCE 1
  
  /* Setup initial state */
  netmgr_kif_info[INSTANCE].ifi_index = 4;
  netmgr_kif_info[INSTANCE].state = NETMGR_KIF_OPEN;
  netmgr_exec_state_info.links[INSTANCE].sm->current_state = NETMGR_STATE_UP;
  netmgr_tc_cfg.links[INSTANCE].root_qdisc = NULL;
    
  /* Generate QMI QoS activate indication (UMTS)*/
  ind_data.event_report.param_mask = 0; // TODO - Sharat to add value
  flow_info->qos_flow_state.qos_identifier = 0xABCDEF21;
  flow_info->qos_flow_state.new_flow = TRUE;
  flow_info->qos_flow_state.report_flow_state_chng = QMI_QOS_FLOW_ACTIVATED;

  flow_info->tx_granted_flow_data_is_valid = TRUE;
  flow_info->tx_granted_flow_data.ip_flow_index = 0;
  flow_info->tx_granted_flow_data.qos_flow_granted.cdma_flow_desc.param_mask = 0;
  flow_info->tx_granted_flow_data.qos_flow_granted.umts_flow_desc.param_mask = (QMI_QOS_UMTS_FLOW_PARAM_DATA_RATE| QMI_QOS_UMTS_FLOW_PARAM_TRAFFIC_CLASS);
  flow_info->tx_granted_flow_data.qos_flow_granted.umts_flow_desc.data_rate.max_rate        = 1024; //kbps
  flow_info->tx_granted_flow_data.qos_flow_granted.umts_flow_desc.data_rate.guaranteed_rate =  800; //kbps
  flow_info->tx_granted_flow_data.qos_flow_granted.umts_flow_desc.traffic_class = QMI_QOS_UMTS_TC_STREAMING;

  flow_info->tx_filter_count = 2;
  flow_info->tx_granted_filter_data[0].filter_index = 0;
  flow_info->tx_granted_filter_data[0].qos_filter.ip_version = QMI_QOS_IP_VERSION_4; /* IPv4 */
  flow_info->tx_granted_filter_data[0].qos_filter.filter_desc.param_mask = (QMI_QOS_FILTER_PARAM_SRC_ADDR |
                                                                         QMI_QOS_FILTER_PARAM_DEST_ADDR |
                                                                         QMI_QOS_FILTER_PARAM_TRANS_PROTOCOL |
                                                                         QMI_QOS_FILTER_PARAM_TOS |
                                                                         QMI_QOS_FILTER_PARAM_TCP_DEST_PORTS);
  flow_info->tx_granted_filter_data[0].qos_filter.filter_desc.src_addr.ipv4_ip_addr = ntohl(0x01020304);
  flow_info->tx_granted_filter_data[0].qos_filter.filter_desc.src_addr.ipv4_subnet_mask = ntohl(0xFFFFFFFF);
  flow_info->tx_granted_filter_data[0].qos_filter.filter_desc.dest_addr.ipv4_ip_addr = ntohl(0x09080706);
  flow_info->tx_granted_filter_data[0].qos_filter.filter_desc.dest_addr.ipv4_subnet_mask = ntohl(0xFFFFFFFF);;
  flow_info->tx_granted_filter_data[0].qos_filter.filter_desc.protocol = QMI_QOS_TRANS_PROT_TCP;
  flow_info->tx_granted_filter_data[0].qos_filter.filter_desc.tos.tos_value = 0x08;
  flow_info->tx_granted_filter_data[0].qos_filter.filter_desc.tos.tos_mask = 0x0F;
  flow_info->tx_granted_filter_data[0].qos_filter.filter_desc.tcp_src_ports.start_port = 1000;
  flow_info->tx_granted_filter_data[0].qos_filter.filter_desc.tcp_src_ports.range = 2;
  flow_info->tx_granted_filter_data[1].filter_index = 1;
  flow_info->tx_granted_filter_data[1].qos_filter.ip_version = QMI_QOS_IP_VERSION_4; /* IPv4 */
  flow_info->tx_granted_filter_data[1].qos_filter.filter_desc.param_mask = (QMI_QOS_FILTER_PARAM_SRC_ADDR |
                                                                         QMI_QOS_FILTER_PARAM_DEST_ADDR |
                                                                         QMI_QOS_FILTER_PARAM_TRANS_PROTOCOL |
                                                                         QMI_QOS_FILTER_PARAM_TOS |
                                                                         QMI_QOS_FILTER_PARAM_TCP_DEST_PORTS);
  flow_info->tx_granted_filter_data[1].qos_filter.filter_desc.src_addr.ipv4_ip_addr = ntohl(0x01020304);
  flow_info->tx_granted_filter_data[1].qos_filter.filter_desc.src_addr.ipv4_subnet_mask = ntohl(0xFFFFFFFF);
  flow_info->tx_granted_filter_data[1].qos_filter.filter_desc.dest_addr.ipv4_ip_addr = ntohl(0x09080706);
  flow_info->tx_granted_filter_data[1].qos_filter.filter_desc.dest_addr.ipv4_subnet_mask = ntohl(0xFFFFFFFF);;
  flow_info->tx_granted_filter_data[1].qos_filter.filter_desc.protocol = QMI_QOS_TRANS_PROT_UDP;
  flow_info->tx_granted_filter_data[1].qos_filter.filter_desc.udp_src_ports.start_port = 4000;
  flow_info->tx_granted_filter_data[1].qos_filter.filter_desc.udp_src_ports.range = 5;
    
  netmgr_qmi_qos_ind( INSTANCE, QMI_QOS_SERVICE, (void*)INSTANCE,
                      QMI_QOS_SRVC_EVENT_REPORT_IND_MSG, &ind_data );
  sleep(1);

  /* Check state of objects */
  TEST_ASSERT( NETMGR_KIF_OPEN == netmgr_kif_info[INSTANCE].state );
  TEST_ASSERT( NETMGR_STATE_UP ==
               stm_get_state( netmgr_exec_state_info.links[INSTANCE].sm ) );

  /* Validate flow */
  TEST_ASSERT( NULL != netmgr_tc_cfg.links[INSTANCE].flow_list );
  TEST_ASSERT( NULL != ds_dll_deq( netmgr_tc_cfg.links[INSTANCE].flow_list, NULL, (const void**)&flow_buf) );
  TEST_ASSERT( flow_info->qos_flow_state.qos_identifier == flow_buf->qos_flow.flow_id );
  TEST_ASSERT( NETMGR_TC_FLOW_ACTIVE == flow_buf->qos_flow.state );
  
  /* Validate class  */
  TEST_ASSERT( NULL != netmgr_tc_cfg.links[INSTANCE].default_class );

  /* Validate filter */
//  TEST_ASSERT( NULL != flow_buf->filter );

  return NETMGR_SUCCESS;
}

int netmgr_test_qos_create_cdma_sucessful( void* arg )
{
  qmi_qos_indication_data_type ind_data;
  qmi_qos_event_report_flow_info_type *flow_info = &ind_data.event_report.flow_info;
  struct msghdr * msgh = NULL;
  netmgr_exec_cmd_t * cmd_buf = NULL;
  netmgr_tc_flow_info_t * flow_buf = NULL;
  #define INSTANCE 1
  
  /* Setup initial state */
  netmgr_kif_info[INSTANCE].ifi_index = 4;
  netmgr_kif_info[INSTANCE].state = NETMGR_KIF_OPEN;
  netmgr_exec_state_info.links[INSTANCE].sm->current_state = NETMGR_STATE_UP;
  netmgr_tc_cfg.links[INSTANCE].root_qdisc = NULL;
  
  /* Generate QMI QoS activate indication (CDMA)*/
  ind_data.event_report.param_mask = 0; // TODO - Sharat to add value
  flow_info->qos_flow_state.qos_identifier = 0xFEDCBA89;
  flow_info->qos_flow_state.new_flow = TRUE;
  flow_info->qos_flow_state.report_flow_state_chng = QMI_QOS_FLOW_ACTIVATED;
  flow_info->tx_granted_flow_data_is_valid = TRUE;
  flow_info->tx_granted_flow_data.ip_flow_index = 0;
  flow_info->tx_granted_flow_data.qos_flow_granted.umts_flow_desc.param_mask = 0;
  flow_info->tx_granted_flow_data.qos_flow_granted.cdma_flow_desc.param_mask = (QMI_QOS_CDMA_FLOW_PARAM_PROFILE_ID);
  flow_info->tx_granted_flow_data.qos_flow_granted.cdma_flow_desc.profile_id = 5;
  flow_info->tx_filter_count = 0;
  
    
  netmgr_qmi_qos_ind( INSTANCE, QMI_QOS_SERVICE, (void*)INSTANCE,
                      QMI_QOS_SRVC_EVENT_REPORT_IND_MSG, &ind_data );
  sleep(1);

  /* Check state of objects */
  TEST_ASSERT( NETMGR_KIF_OPEN == netmgr_kif_info[INSTANCE].state );
  TEST_ASSERT( NETMGR_STATE_UP ==
               stm_get_state( netmgr_exec_state_info.links[INSTANCE].sm ) );
  
  /* Validate flow */
  TEST_ASSERT( NULL != netmgr_tc_cfg.links[INSTANCE].flow_list );
  TEST_ASSERT( NULL != ds_dll_deq( netmgr_tc_cfg.links[INSTANCE].flow_list, NULL, (const void**)&flow_buf) );
  TEST_ASSERT( flow_info->qos_flow_state.qos_identifier == flow_buf->qos_flow.flow_id );
  TEST_ASSERT( NETMGR_TC_FLOW_ACTIVE == flow_buf->qos_flow.state );
  
  /* Validate class  */
  TEST_ASSERT( NULL != netmgr_tc_cfg.links[INSTANCE].default_class );
  
  return NETMGR_SUCCESS;
}

#endif // NETMGR_QOS_ENABLED  

/*--------------------------------------------------------------------------- 
  Test Framework
---------------------------------------------------------------------------*/

netmgr_testcase_t netmgr_tests[] =
{
  { "Generate QMI packet services indication for successful bringup",
    netmgr_test_iface_bringup_successful },
  
  { "Generate QMI packet services indication for successful teardown",
    netmgr_test_iface_teardown_successful },

  { "Generate QMI packet services indication for successful reconfigure",
    netmgr_test_iface_reconfigure_successful },

  { "Generate abnormal KIF close while Modem interface UP",
    netmgr_test_kif_close_when_iface_up_successful },
  
  { "Generate abnormal KIF close while Modem interface CONFIGURING",
    netmgr_test_kif_close_when_iface_configuring_successful },

  { "Generate abnormal KIF close while Modem interface RECONFIGURING",
    netmgr_test_kif_close_when_iface_reconfiguring_successful },

  { "Generate abnormal KIF close while Modem interface COMING_UP",
    netmgr_test_kif_close_when_iface_comingup_successful },

  { "Generate abnormal WDS disconnect while Modem interface COMING_UP",
    netmgr_test_wds_disconnect_when_iface_comingup_successful },

  { "Generate abnormal WDS disconnect while Modem interface CONFIGURING",
    netmgr_test_wds_disconnect_when_iface_configuring_successful },

  { "Generate abnormal WDS disconnect while Modem interface RECONFIGURING",
    netmgr_test_wds_disconnect_when_iface_reconfiguring_successful },
  
  { "Generate abnormal WDS connect while Modem interface GOING_DOWN",
    netmgr_test_wds_connect_when_iface_goingdown_successful },

  { "Generate QMI packet services indication for first IP family removal [dual-IP]",
    netmgr_test_iface_dualIP_firstIPremoval_successful },

  { "Generate QMI packet services indication for second IP family removal [dual-IP]",
    netmgr_test_iface_dualIP_secondIPremoval_successful },
  
  { "Generate QMI packet services indication late for IPV4 call [dual-IP]",
    netmgr_test_iface_dualIP_late_ipv4_connect_successful },

  { "Generate QMI packet services indication late for IPV6 call [dual-IP]",
    netmgr_test_iface_dualIP_late_ipv6_connect_successful },
  
#if 0 // Needs work */  
  { "Generate KIF address update [dual-IP]",
    netmgr_test_iface_address_update_successful },
#endif
  
#if 0  
  { "Generate modem oos when modem is INITED",
    netmgr_test_modem_oos_is_various_stages },

  { "Verify worker thread termination",
    netmgr_test_kif_worker_thread_stop_successful },

#ifdef NETMGR_QOS_ENABLED  
  { "Generate QoS event report for flow activation on UMTS",
    netmgr_test_qos_create_umts_sucessful },

  { "Generate QoS event report for flow activation on CDMA",
    netmgr_test_qos_create_cdma_sucessful },
#endif // NETMGR_QOS_ENABLED  
#endif // 0

  { "Generate MODEM event to update MTU",
    netmgr_test_kif_update_mtu },

  { "Generate EMBMS pkt_src_ind MODEM event ",
    netmgr_test_wds_when_iface_is_embms },
};


int netmgr_test_execute( void )
{
  boolean result = NETMGR_FAILURE;
  int i, cnt, cnt_pass = 0;

  cnt = ds_arrsize( netmgr_tests );
  netmgr_log_high("=== Testcases: %d ===\n", cnt );

  for( i=0; i<cnt; i++) {
    netmgr_test_info.current = &netmgr_tests[i];
    netmgr_log_high("***Executing test[%d]: %s\n",
                    i, netmgr_tests[i].description );
    
    result = netmgr_tests[i].testcase( NULL );
    /* Ensure dynamic memory cleanup */
    netmgr_test_reset();
    
    cnt_pass += (NETMGR_SUCCESS==result)? 1 : 0;
    netmgr_log_high("***Result[%d]: %s\n",
                    i, (NETMGR_SUCCESS==result)? "PASS" : "FAIL" );

  }
  
  netmgr_log_high("=== Testcases: %d  PASS: %d(%5.1f%%) FAIL: %d(%5.1f%%) ===\n",
                  cnt, cnt_pass, (cnt_pass*1.0/cnt)*100, (cnt-cnt_pass), (1-(cnt_pass*1.0/cnt))*100 );
  return result;
}

void netmgr_test_init( void )
{
  /* Initialize the mutex and condition variables */
  (void)pthread_mutex_init(&netmgr_test_info.mutx, NULL);
  (void)pthread_cond_init(&netmgr_test_info.cond, NULL);
  
}

#endif /* NETMGR_TEST */
