/*!
  @file
  dsi_netctrl.c

  @brief
  This file provides the functionality to control the embedded data call by
  talking to the modem via QMI service(s).

*/

/*===========================================================================

  Copyright (c) 2010-2014 Qualcomm Technologies, Inc. All Rights Reserved
  Qualcomm Technologies Proprietary and Confidential.
===========================================================================*/

/*===========================================================================

                        EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.
Notice that changes are listed in reverse chronological order.

$Header: //depot/restricted/linux/android/ril/qcril_dsi.c $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
03/11/10   js      Initial Creation (derived from original qcril_datai.c)

===========================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include "ds_string.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#ifdef DSI_NETCTRL_OFFTARGET
#include <string.h>
#include "ds_sl_list.h"
#include <sys/socket.h>
#else
#include <linux/socket.h>
#endif
#include <linux/sockios.h>
#include <linux/if.h>
#include <netinet/in.h>

#include <netdb.h> /* struct addrinfo */

#include "dsi_netctrl.h"
#include "assert.h"
#include "qmi_wds_srvc.h"
#include "dsi_netctrli.h" /* declarations shared internally */
#include "dsi_netctrl_cb_thrd.h"

#define DSI_UMTS_DEFAULT_PDP_PROF 1
static int dsi_ril_instance;

static int dsi_modem_subs_id;

/* dsi_getaddrinfo() #defines */
#define DSI_DNS_PORT 53
#define DSI_GETADDRINFO_NS_COUNT 2

#define DSI_NETMGR_GET_NEXT_TXN_ID(txn) \
  do { \
     pthread_mutex_lock(&dsi_netmgr_txn_mutex); \
     txn.pid = getpid(); \
     txn.txn_id = ++dsi_netmgr_txn_cnt; \
     pthread_mutex_unlock(&dsi_netmgr_txn_mutex); \
  }while(0)

/*---------------------------------------------------------------------------
                           DECLARATIONS
---------------------------------------------------------------------------*/
/*

dsi_store_tbl[] -- contains one entry per each CALL
dsi_iface_tbl[] -- contains one entry per each iface (QMI Port)
dsi_modem_tbl[] -- contains one entry per each modem

There are 1..n dsi_store_tbl[] entries associated with each iface
There are 1..n dsi_iface_tbl[] entries associated with each modem

Design Comments:

multi-thread access:
- External API functions are *NOT* serialized and use a global
  lock to prevent multithreaded concurrent access at any time.

callback processing contract:
- User of this module is expected to post an event to themselves
  and let go immediately without any delay.

Local data protection:
Following mechanism is used to protect local data from
concurrent access (user API, callback processing):
- interface states can be modified by multiple PDs such as
  callback thread and/or user API execution and hence is
  protected by a mutex local to each interface.
- store handle to the interface association can change
  dynamically and needs to be protected by a mutex in order
  to allow multiple concurrent access to such association by
  callbacks and/or user API execution.

DSI interface Design:
reference count:
- local reference count is required on each interface object
  in order to manage multiple call bringup and multipe call
  bring down on the same interface in single PD (wds client handle)
  - lifetime of such a reference count by a given call object
    (i.e. store table entry) would be starting from when the call
    is moved to CONNECTED state to the point where the client
    explicitely let go the call (release, stop etc.) OR if the
    underlying interface itself goes down for some reason.
Interface State: (not a state machine)
- a very basic state information is required on each interface
  object in order to arbitrate the interface usage among
  various call objects (i.e. store table entries)
- lifetime of such a state variable would be starting from
  when a client has successfully issued start_nw_if() on that
  interface to the point where the interface was brought down.
  Remember, even if a given client has let go of the interface,
  the state on that interface may or may not change depending
  on whether other client(s) are associated with the given
  interface or not.

 */


/*===========================================================================
  FUNCTION:  dsi_get_curr_call_info
===========================================================================*/
/*!
    @brief
    This function is a wrapper for qmi_wds_get_curr_call_info().
    The function uses the qdi_hndl to get the corresponding qmi_hndl based
    on the ip_family parameter. req_mask determines what parameters will
    be returned in the call_info structure.

    @return
    DSI_ERROR
    DSI_SUCCESS
*/
/*=========================================================================*/
static int dsi_get_curr_call_info
(
   qdi_client_handle_t qdi_hndl,
   qmi_wds_req_runtime_settings_params_type req_mask,
   int ip_family,
   qmi_wds_curr_call_info_type *call_info
)
{
  qmi_wds_profile_id_type            profile_id;
  qmi_wds_profile_params_type        profile_params;
  qmi_client_handle_type             qmi_hndl;
  int err_code;
  int result;
  int ret = DSI_ERROR;

  DSI_LOG_DEBUG("%s","dsi_get_curr_call_info() : ENTRY" );

  if(AF_INET == ip_family)
  {
    /* Get IPV4 qmi wds handle */
    qmi_hndl = qdi_get_qmi_wds_handle_for_ip(qdi_hndl, QMI_IP_FAMILY_PREF_IPV4);
  }
  else if (AF_INET6 == ip_family)
  {
    /* Get IPV6 qmi wds handle*/
    qmi_hndl = qdi_get_qmi_wds_handle_for_ip(qdi_hndl, QMI_IP_FAMILY_PREF_IPV6);
  }
  else
  {
    DSI_LOG_ERROR("%s","dsi_get_curr_call_info(): Invalid IP Family");
    goto err_lbl;
  }

  /* Query Modem for requested parameters */
  result = qmi_wds_get_curr_call_info( qmi_hndl,
                                       req_mask,
                                       &profile_id,
                                       &profile_params,
                                       call_info,
                                       &err_code);
  if( QMI_NO_ERR != result )
  {
    DSI_LOG_INFO("dsi_get_curr_call_info(): failed on qmi_wds_get_curr_call_info: "
                  "ret=%d err=%d\n",
                  result, err_code);
    goto err_lbl;
  }

  ret = DSI_SUCCESS;
err_lbl:
  DSI_LOG_DEBUG("%s","dsi_get_curr_call_info(): EXIT" );
  return ret;
}

/*===========================================================================
  FUNCTION:  dsi_iface_ioctl_get_pcscf_address
===========================================================================*/
/*!
    @brief
    Get the PCSCF_SERV_ADDR for both the IP families.

    @return
    DSI_ERROR
    DSI_SUCCESS
*/
/*=========================================================================*/
static int dsi_iface_ioctl_get_pcscf_address
(
  qdi_client_handle_t qdi_hndl,
  int ip_version,
  void *argval_ptr
)
{
  qmi_wds_req_runtime_settings_params_type  req_mask;
  qmi_wds_curr_call_info_type               call_info_ipv4;
  qmi_wds_curr_call_info_type               call_info_ipv6;
  dsi_pcscf_addr_info_t*                    pcscf_addr;
  unsigned int index = 0;
  unsigned int cnt = 0;
  struct sockaddr_in *sin   = NULL;
  struct sockaddr_in6 *sin6 = NULL;
  char addr_buf[INET6_ADDRSTRLEN];
  struct sockaddr_storage* __attribute__((__may_alias__)) addr_ptr;

  DSI_LOG_DEBUG("dsi_iface_ioctl_get_pcscf_address(): ENTRY QDI handle [%d]", qdi_hndl);

  if( NULL == argval_ptr )
  {
    DSI_LOG_ERROR("%s","dsi_iface_ioctl_get_pcscf_address(): Invalid parameters passed [NULL]");
    DSI_LOG_DEBUG("%s","dsi_iface_ioctl_get_pcscf_address(): EXIT with error" );
    return DSI_ERROR;
  }

  pcscf_addr = (dsi_pcscf_addr_info_t*) argval_ptr;
  memset(pcscf_addr, 0, sizeof(dsi_pcscf_addr_info_t));
  memset(&call_info_ipv4, 0, sizeof(qmi_wds_curr_call_info_type));
  memset(&call_info_ipv6, 0, sizeof(qmi_wds_curr_call_info_type));

  /* Set appropriate mask to get the PCSCF server address */
  req_mask = QMI_WDS_GET_CURR_CALL_INFO_PCSCF_SERV_ADDR_PARAM_MASK;

  if(DSI_IP_VERSION_4 == ip_version ||
     DSI_IP_VERSION_4_6 == ip_version)
  {
    /* Get IPV4 PCSCF addresses */
    dsi_get_curr_call_info(qdi_hndl, req_mask, AF_INET, &call_info_ipv4);

    /* Store pcscf address */
    if (QMI_WDS_CURR_CALL_INFO_PCSCF_IPV4_ADDR_LIST & call_info_ipv4.mask)
    {
      /* Populate pcscf addresses */
      for(index = 0; index < call_info_ipv4.p_cscf_ipv4_addrs.num_instances && cnt < DSI_PCSCF_ADDR_LIST_MAX; index++)
      {
        uint32 addr = htonl( call_info_ipv4.p_cscf_ipv4_addrs.p_cscf_ipv4_addr[index] );

        addr_ptr = &pcscf_addr->pcscf_address[cnt].addr;
        sin = (struct sockaddr_in*)addr_ptr;

        pcscf_addr->pcscf_address[cnt].addr.ss_family = AF_INET;
        pcscf_addr->pcscf_address[cnt].valid_addr = TRUE;
        memcpy(&sin->sin_addr, &addr,sizeof(sin->sin_addr));
        ++cnt;
      }
     }
  }

  if(DSI_IP_VERSION_6 == ip_version ||
     DSI_IP_VERSION_4_6 == ip_version)
  {
    /* Get IPV6 PCSCF addresses */
    dsi_get_curr_call_info(qdi_hndl, req_mask, AF_INET6, &call_info_ipv6);

    if (QMI_WDS_CURR_CALL_INFO_PCSCF_IPV6_ADDR_LIST & call_info_ipv6.mask)
    {
      /* Populate pcscf addresses */
      for(index = 0; index < call_info_ipv6.p_cscf_ipv6_addrs.num_instances && cnt < DSI_PCSCF_ADDR_LIST_MAX; index++)
      {
        addr_ptr = &pcscf_addr->pcscf_address[cnt].addr;
        sin6 = (struct sockaddr_in6*)addr_ptr;

        pcscf_addr->pcscf_address[cnt].addr.ss_family = AF_INET6;
        pcscf_addr->pcscf_address[cnt].valid_addr = TRUE;
        memcpy(&sin6->sin6_addr, call_info_ipv6.p_cscf_ipv6_addrs.p_cscf_ipv6_addr[index], sizeof(sin6->sin6_addr));
        ++cnt;
      }
    }
  }

  pcscf_addr->addr_count = cnt;
  DSI_LOG_INFO("dsi_iface_ioctl_get_pcscf_address(): Received [%d] PCSCF addresses", cnt);
  for(index = 0; index < cnt ; index++)
  {
    memset(addr_buf, 0, sizeof(INET6_ADDRSTRLEN));
    if( pcscf_addr->pcscf_address[index].addr.ss_family == AF_INET)
    {
      addr_ptr = &pcscf_addr->pcscf_address[index].addr;
      sin = (struct sockaddr_in*)addr_ptr;
      inet_ntop(AF_INET, &sin->sin_addr, addr_buf, INET_ADDRSTRLEN);
    }
    else if( pcscf_addr->pcscf_address[index].addr.ss_family == AF_INET6)
    {
      addr_ptr = &pcscf_addr->pcscf_address[index].addr;
      sin6 = (struct sockaddr_in6*)addr_ptr;
      inet_ntop(AF_INET6, &sin6->sin6_addr, addr_buf, INET6_ADDRSTRLEN);
    }
    DSI_LOG_INFO("dsi_iface_ioctl_get_pcscf_address(): PCSCF Addr - %s", addr_buf);
  }

  DSI_LOG_DEBUG("%s","dsi_iface_ioctl_get_pcscf_address(): EXIT success" );
  return DSI_SUCCESS;
}

/*===========================================================================
  FUNCTION:  dsi_iface_ioctl_get_pcscf_fqdn_list
===========================================================================*/
/*!
    @brief
    Get the PCSCF_FQDN_LIST for both the IP families.

    @return
    DSI_ERROR
    DSI_SUCCESS
*/
/*=========================================================================*/
static int dsi_iface_ioctl_get_pcscf_fqdn_list
(
  qdi_client_handle_t qdi_hndl,
  int ip_version,
  void *argval_ptr
)
{
  qmi_wds_req_runtime_settings_params_type  req_mask;
  qmi_wds_curr_call_info_type               call_info_v4;
  qmi_wds_curr_call_info_type               call_info_v6;
  dsi_pcscf_fqdn_list_t*                    pcscf_fqdn;
  unsigned int index = 0;
  unsigned int cnt = 0;

  DSI_LOG_DEBUG("%s","dsi_iface_ioctl_get_pcscf_fqdn_list(): ENTRY" );

  if( NULL == argval_ptr )
  {
    DSI_LOG_ERROR("%s","dsi_iface_ioctl_get_pcscf_fqdn_list(): Invalid parameters passed [NULL]");
    DSI_LOG_DEBUG("%s","dsi_iface_ioctl_get_pcscf_fqdn_list(): EXIT err" );
    return DSI_ERROR;
  }

  pcscf_fqdn = (dsi_pcscf_fqdn_list_t*) argval_ptr;
  memset(pcscf_fqdn, 0, sizeof(dsi_pcscf_fqdn_list_t));
  memset(&call_info_v4, 0, sizeof(qmi_wds_curr_call_info_type));
  memset(&call_info_v6, 0, sizeof(qmi_wds_curr_call_info_type));

  /* Set appropriate mask to get the FQDN */
  req_mask = QMI_WDS_GET_CURR_CALL_INFO_PCSCF_DOMAIN_NAME_LIST_PARAM_MASK;

  if( DSI_IP_VERSION_4 == ip_version ||
      DSI_IP_VERSION_4_6 == ip_version)
  {
    /* Get IPV4 PCSCF FQDN */
    dsi_get_curr_call_info(qdi_hndl, req_mask, AF_INET, &call_info_v4);

    /* Store PCSCF FQDN */
    if (QMI_WDS_CURR_CALL_INFO_PCSCF_FQDN_LIST & call_info_v4.mask)
    {
      /* Populate pcscf domain list */
      for(index = 0; index < call_info_v4.fqdn_list.num_instances && cnt < DSI_PCSCF_FQDN_LIST_MAX ; index++)
      {
        memcpy(&pcscf_fqdn->pcscf_domain_list[cnt],
               &call_info_v4.fqdn_list.fqdn_strings[index],
               sizeof(qmi_wds_fqdn_strings));
        ++cnt;
      }
    }
  }

  if( DSI_IP_VERSION_6 == ip_version ||
      DSI_IP_VERSION_4_6 == ip_version)
  {
    /* Get IPV6 PCSCF FQDN */
    dsi_get_curr_call_info(qdi_hndl, req_mask, AF_INET6, &call_info_v6);

    if (QMI_WDS_CURR_CALL_INFO_PCSCF_FQDN_LIST & call_info_v6.mask)
    {
      /* Populate pcscf domain list */
      for(index = 0; index < call_info_v6.fqdn_list.num_instances && cnt < DSI_PCSCF_FQDN_LIST_MAX; index++)
      {
        memcpy(&pcscf_fqdn->pcscf_domain_list[cnt],
               &call_info_v6.fqdn_list.fqdn_strings[index],
               sizeof(qmi_wds_fqdn_strings));
        ++cnt;
      }
    }
  }

  pcscf_fqdn->fqdn_count = cnt;
  DSI_LOG_INFO("dsi_iface_ioctl_get_pcscf_fqdn_list(): Received [%d] PCSCF domain names", cnt);
  for(index = 0; index<cnt; index++)
  {
    DSI_LOG_INFO("dsi_iface_ioctl_get_pcscf_fqdn_list() : PCSCF domain name - %s",
                 pcscf_fqdn->pcscf_domain_list[index].fqdn_string);
  }

  DSI_LOG_DEBUG("%s","dsi_iface_ioctl_get_pcscf_fqdn_list(): EXIT success" );
  return DSI_SUCCESS;
}

/*===========================================================================
  FUNCTION:  dsi_iface_ioctl_remove_delegated_ipv6_prefix
===========================================================================*/
/*!
    @brief
    Remove IPV6 prefix from the interface

    @return
    DSI_ERROR
    DSI_SUCCESS
*/
/*=========================================================================*/
static int dsi_iface_ioctl_remove_delegated_ipv6_prefix
(
  qdi_client_handle_t qdi_hndl,
  void *argval_ptr
)
{
  qmi_client_handle_type              qmi_hndl;
  qmi_wds_delegated_ipv6_prefix_type  *req;
  int                                 err_code;
  int                                 result;

  DSI_LOG_DEBUG("%s","dsi_iface_ioctl_remove_delegated_ipv6_prefix(): ENTRY" );

  if ( NULL == argval_ptr )
  {
    DSI_LOG_ERROR("%s","dsi_iface_ioctl_remove_delegated_ipv6_prefix(): "
                       "Invalid parameters passed [NULL]");
    return DSI_ERROR;
  }

  req = (qmi_wds_delegated_ipv6_prefix_type*) argval_ptr;

  /* Get IPv6 handle */
  qmi_hndl = qdi_get_qmi_wds_handle_for_ip(qdi_hndl, QMI_IP_FAMILY_PREF_IPV6);

  if (DSI_INVALID_WDS_HNDL == qmi_hndl)
  {
    DSI_LOG_ERROR("%s", "dsi_iface_ioctl_remove_delegated_ipv6_prefix(): "
                        "Invalid WDS handle!");
    return DSI_ERROR;
  }

  result = qmi_wds_remove_delegated_ipv6_prefix(qmi_hndl,
                                                req,
                                                &err_code);

  if (QMI_NO_ERR != result)
  {
    DSI_LOG_ERROR("dsi_iface_ioctl_remove_delegated_ipv6_prefix(): "
                  "Failed to delete IPV6 prefix [%d][%d]",
                  result, err_code);
    return DSI_ERROR;
  }

  DSI_LOG_DEBUG("%s","dsi_iface_ioctl_remove_delegated_ipv6_prefix(): "
                     "EXIT success" );
  return DSI_SUCCESS;
}

/*===========================================================================
                    PUBLIC FUNCTIONS - TO BE IMPLEMENTED LATER
===========================================================================*/
/* TODO: not implemented yet */
#if 0
static int get_data_call_param(
  dsi_hndl_t hndl,
  dsi_call_param_identifier_t identifier,
  dsi_call_param_value_t * info
)
{
  int ret = DSI_ERROR;
  int reti = DSI_SUCCESS;
  dsi_store_t * st = NULL;

  DSI_LOG_DEBUG( "%s", "get_data_call_param: ENTRY" );

  st = ( (dsi_store_t *) hndl );

  do
  {

    if ( !( DSI_IS_HNDL_VALID( st ) ) ||
         !( DSI_IS_IDENT_VALID ( identifier ) ) ||
          ( info == NULL ) )
    {
      DSI_LOG_ERROR( "cannot get, inval arg, dsi_hndl [%x], ident [%d]",
                       (unsigned int)hndl, identifier );
      break;
    }

    reti = DSI_SUCCESS;
    switch( identifier )
    {
    case DSI_CALL_INFO_DEVICE_NAME:
    case DSI_CALL_INFO_APN_NAME:
    case DSI_CALL_INFO_IP_ADDR:
    default:
      DSI_LOG_ERROR( "get call param not supported, " \
                     " identifier [%d]", identifier );
      reti = DSI_ERROR;
      break;
    }
    if(DSI_ERROR == reti)
    {
      break;
    }

    ret = DSI_SUCCESS;
  } while (0);

  if (DSI_SUCCESS == ret)
  {
    DSI_LOG_DEBUG( "%s", "get_data_call_attr: EXIT with suc" );
  }
  else
  {
    DSI_LOG_DEBUG( "%s", "get_data_call_attr: EXIT with err" );
  }

  return ret;
}

/* TODO: not implemented yet */
static int get_last_call_fail_cause(
  dsi_hndl_t       hndl,
  int             *fail_cause
)
{
  DSI_LOG_DEBUG( "last call fail cause is [%u]", *fail_cause );
  DSI_LOG_DEBUG( "%s", "get_last_call_fail_cause: EXIT with err" );
  return DSI_ERROR;
}/* get_last_call_fail_cause() */

/*
    This routine registers physlink events ACTIVE/DORMANT
    on the specified iface.
*/
int dsi_reg_physlink_up_down_events(
  dsi_hndl_t            *handle
)
{
  DSI_LOG_ERROR( "%s", "dsi_reg_physlink_up_down_events: NOT IMPLEMENTED \n" );

  return DSI_ERROR;
}

/*
    This routine is a generic iface ioctl handler for qcril.
    Currently this routine wraps the dss iface ioctl.
*/
int dsi_iface_go_dormant
(
  dsi_hndl_t * handle
)
{
  DSI_LOG_ERROR("%s","dsi_iface_go_dormant: not implemented");

  return DSI_ERROR;
}
#endif /* if 0 */
/*===========================================================================
                    PUBLIC VTABLE IMPLEMENTATION
===========================================================================*/
/*==========================================================================
  FUNCTION: dsi_ind_registration
===========================================================================*/
/*!
  @brief
  This function can be used to register/unregister for WDS handoff
  indications

  @return
  DSI_SUCCESS
  DSI_FAILURE

  @note
  This function must be called only after SNI is done and we have
  an iface association. The WDS client binding will happen only at
  that point in time. This can be called when DSI_NET_IS_CONN event
  is received
*/
/*=========================================================================*/
int dsi_ind_registration
(
  dsi_hndl_t              hndl,
  dsi_ind_reg_type        ind_type,
  dsi_ind_registration_t  reg_unreg
)
{
  dsi_store_t * st_hndl = NULL;
  int ret = DSI_ERROR;
  int reti = DSI_SUCCESS;
  int i;

  DSI_LOG_DEBUG( "%s", "dsi_ind_registration: ENTRY" );

  DSI_GLOBAL_LOCK;

  do
  {
    ret = DSI_ERROR;

    st_hndl = ((dsi_store_t *)hndl);

    if (!dsi_inited)
    {
      DSI_LOG_ERROR("%s","dsi_ind_registration: dsi not inited");
      break;
    }

    if(!(DSI_IS_HNDL_VALID(st_hndl)))
    {
      DSI_LOG_ERROR( "cannot proceed with reg/unreg, inval arg, st_hndl [%p]",
                       (unsigned int*)st_hndl );
      break;
    }

    DSI_LOCK_MUTEX(&(st_hndl->priv.mutex));
    i = st_hndl->priv.dsi_iface_id;
    DSI_UNLOCK_MUTEX(&(st_hndl->priv.mutex));

    if(!DSI_IS_ID_VALID(i))
    {
      DSI_LOG_ERROR("dsi_ind_registration: st_hndl contains" \
                      "invalid id [%d]", i);
      break;
    }

    switch (ind_type)
    {
    case QMI_WDS_HANDOFF_IND:
      {
        if (DSI_IND_REGISTER == reg_unreg)
        {
          DSI_LOG_DEBUG("%s", "Register for Handoff indications");
          reti = dsi_mni_vtbl.mni_reg_unreg_handoff_ind_f(i, st_hndl, TRUE);
        }
        else
        {
          DSI_LOG_DEBUG("%s", "Unregister for Handoff indications");
          reti = dsi_mni_vtbl.mni_reg_unreg_handoff_ind_f(i, st_hndl, FALSE);
        }
      }
      break;

    default:
      {
        DSI_LOG_ERROR("%s", "dsi_ind_registration: Unknown indication");
        reti = DSI_ERROR;
        break;
      }
    }

    if (DSI_SUCCESS != reti)
    {
      DSI_LOG_ERROR("%s", "Handoff ind registeration/unregisteration failed!");
      break;
    }

    ret = DSI_SUCCESS;
  } while(0);

  if (DSI_SUCCESS == ret)
  {
    DSI_LOG_DEBUG( "%s", "dsi_ind_registration: EXIT with suc" );
  }
  else
  {
    DSI_LOG_DEBUG( "%s", "dsi_ind_registration: EXIT with err" );
  }

  DSI_GLOBAL_UNLOCK;

  return ret;
}

/*===========================================================================
  FUNCTION:  dsi_start_data_call
===========================================================================*/
/*!
    @brief
    This function can be used to start a data call.

    @return
    DSI_ERROR
    DSI_SUCCESS
*/
/*=========================================================================*/
int dsi_start_data_call
(
  dsi_hndl_t hndl
)
{
  dsi_store_t * st_hndl = NULL;
  int ret = DSI_ERROR, i = 0;

  DSI_L2S_ENTRY_PARAMS("dsi_hndl=%p", (unsigned int*)hndl);
  DSI_LOG_DEBUG( "%s", "start_data_call: ENTRY" );

  DSI_GLOBAL_LOCK;

  st_hndl = ( (dsi_store_t *) hndl );

#ifdef DSI_TEST_MODE_FAIL_CALL_CONTINUOUS
  DSI_LOG_VERBOSE( "%s", "test mode, fail data call" );
  goto err_label;
#endif

  /* this do..while loop decides the overall return value.
     set ret to ERROR at the beginning. set ret to SUCCESS
     at the end. If there was an error in the middle, we break out*/
  do
  {
    ret = DSI_ERROR;

    if (!dsi_inited)
    {
      DSI_LOG_ERROR("%s","start_data_call: dsi not inited");
      break;
    }

    if (!(DSI_IS_HNDL_VALID(st_hndl)))
    {
      DSI_LOG_ERROR( "start_data_call invalid arg, st_hndl [%p]",
                       (unsigned int*)st_hndl );
      break;
    }

    if (st_hndl->priv.call_tech == QMI_WDS_IFACE_NAME_EMBMS &&
        st_hndl->priv.ip_version == DSI_IP_VERSION_4_6)
    {
      DSI_LOG_ERROR("%s","start_data_call: Dual IP not supported for EMBMS");
      break;
    }
    /* If this is not a partial retry, perform route look-up */
    if (FALSE == st_hndl->priv.partial_retry)
    {
      ret = dsi_mni_vtbl.mni_look_up_f(st_hndl, &i);
      if (DSI_SUCCESS != ret)
      {
        DSI_LOG_ERROR("%s", "dsi_route_look_up failed");
        break;
      }
    }
    else
    {
      DSI_LOG_INFO( "Dual-IP partial retry: skipping route look-up, using iface=%d",
                    st_hndl->priv.dsi_iface_id );
      i = st_hndl->priv.dsi_iface_id;

      /* Clear the rl_qmi_inst during partial retry to avoid unnecessary
         ref_count increments */
      st_hndl->priv.rl_qmi_inst = DSI_INVALID_QMI_INST;
    }

    if( !DSI_IS_WDS_HNDL_VALID(i) && ( !DSI_IS_QOS_HNDL_VALID(i) ))
    {
      dsi_mni_init_client(i);
    }
    DSI_LOG_DEBUG("issuing start_nw_if on dsi_iface [%d]", i);

    /* at this point, we know which dsi iface (i.e. qmi port) - i
       to issue wds_start_nw_iface command on */
    ret = dsi_mni_vtbl.mni_start_f(i, st_hndl);
    if(ret == DSI_ERROR)
    {
      dsi_ip_family_t ipf = (DSI_IP_VERSION_6 == st_hndl->priv.ip_version) ?
                             DSI_IP_FAMILY_V6 : DSI_IP_FAMILY_V4;

      DSI_LOG_ERROR("dsi_mni_vtbl.mni_start_f [%p] returned err",
                    (unsigned int*)dsi_mni_vtbl.mni_start_f);
      DSI_LOG_ERROR("dsi call end reason code/type is [%d/%d]",
                    st_hndl->priv.ce_reason[ipf].reason_code,
                    st_hndl->priv.ce_reason[ipf].reason_type);

      dsi_mni_release_client(i);

      break;
    }
    else
    {
      /*
         now that we have successfully issued start_nw_if()
         on this interface for the given st_hndl, attach
         this interface to the st_hndl and put it's state
         to IN_USE, so doesn't get stepped on inadvertently
         by another call (store handle)
      */
      DSI_LOCK_MUTEX(&(st_hndl->priv.mutex));
      dsi_attach_dsi_iface(i, st_hndl);
      DSI_UPDATE_CALL_STATE(st_hndl, DSI_STATE_CALL_CONNECTING);
      DSI_UNLOCK_MUTEX(&(st_hndl->priv.mutex));
    }

    ret = DSI_SUCCESS;
  } while(0);

  if (DSI_SUCCESS == ret)
  {
    DSI_LOG_DEBUG( "%s", "start_data_call: EXIT with suc" );
  }
  else
  {
    DSI_LOG_DEBUG( "%s", "start_data_call: EXIT with err" );
  }

  DSI_GLOBAL_UNLOCK;
  DSI_L2S_EXIT_WITH_STATUS("dsi_hndl=%p", (unsigned int*)hndl);

  return ret;
}

/*===========================================================================
  FUNCTION:  dsi_stop_data_call
===========================================================================*/
/*!
    @brief
    This function can be used to stop a data call.

    @return
    DSI_ERROR
    DSI_SUCCESS
*/
/*=========================================================================*/
int dsi_stop_data_call(
  dsi_hndl_t hndl
)
{
  dsi_store_t * st_hndl;
  int ret = DSI_ERROR;
  int i;
  boolean result = FALSE;
  boolean send_no_net = FALSE;

  DSI_L2S_ENTRY_PARAMS("dsi_hndl=%p", (unsigned int*)hndl);
  DSI_GLOBAL_LOCK;

  /* this do..while loop decides the overall return value
     set ret to ERROR at the beginning. set ret to SUCCESS
     at the end. If there was an error in the middle, we break out*/
  do
  {
    ret = DSI_ERROR;

    DSI_LOG_DEBUG( "%s", "stop_data_call: ENTRY" );

    st_hndl = ((dsi_store_t *)hndl);

    if (!dsi_inited)
    {
      DSI_LOG_ERROR("%s","stop_data_call: dsi not inited");
      break;
    }

    if(!(DSI_IS_HNDL_VALID(st_hndl)))
    {
      DSI_LOG_ERROR( "cannot stop, inval arg, st_hndl [%p]",
                       (unsigned int*)st_hndl );
      break;
    }

    DSI_LOCK_MUTEX(&(st_hndl->priv.mutex));
    i = st_hndl->priv.dsi_iface_id;
    DSI_UNLOCK_MUTEX(&(st_hndl->priv.mutex));

    if(!DSI_IS_ID_VALID(i))
    {
      DSI_LOG_ERROR("stop_data_call: st_hndl contains" \
                      "invalid id [%d]", i);
      break;
    }
    switch(st_hndl->priv.ip_version)
    {
      case DSI_IP_VERSION_4:
        if (DSI_INVALID_WDS_TXN != DSI_GET_WDS_V4_TXN(i))
        {
          result = TRUE;
        }
      break;
      case DSI_IP_VERSION_6:
        if (DSI_INVALID_WDS_TXN != DSI_GET_WDS_V6_TXN(i))
        {
          result = TRUE;
        }
      break;
      case DSI_IP_VERSION_4_6:
        if (DSI_INVALID_WDS_TXN != DSI_GET_WDS_V4_TXN(i)||DSI_INVALID_WDS_TXN != DSI_GET_WDS_V6_TXN(i))
        {
          result = TRUE;
        }
      break;
    }
    /* if wds txn exists, abort */
    if (result)
    {
      DSI_LOCK_MUTEX(&(st_hndl->priv.mutex));
      DSI_UPDATE_CALL_STATE(st_hndl, DSI_STATE_CALL_DISCONNECTING);
      DSI_UNLOCK_MUTEX(&(st_hndl->priv.mutex));

      ret = dsi_mni_vtbl.mni_abort_start_f(i, st_hndl);
      if (DSI_ERROR == ret)
      {
        break;
      }
    }
    else
    {

      /* issue stop nw if cmd if local ref_count goes to 0 */
      DSI_LOCK_MUTEX(&(st_hndl->priv.mutex));
      if( DSI_IP_VERSION_4_6 == st_hndl->priv.ip_version )
      {

       /* Dont send Local NO_NET if the current interface ref count is
          reached one. We need to call Stop n.w i/f.
       */

        if(((1 == DSI_GET_V4_COUNT(i)) && (1 == DSI_GET_V6_COUNT(i))) ||
             (1 == (DSI_GET_V4_COUNT(i) + DSI_GET_V6_COUNT(i))))
        {
          send_no_net = FALSE;
          DSI_DECR_COUNT(i, st_hndl,DSI_IP_VERSION_4);
          DSI_DECR_COUNT(i, st_hndl,DSI_IP_VERSION_6);
        }
        else
        {
          send_no_net = TRUE;
          DSI_DECR_COUNT(i, st_hndl,DSI_IP_VERSION_4);
          DSI_DECR_COUNT(i, st_hndl,DSI_IP_VERSION_6);
          DSI_UNLOCK_MUTEX(&(st_hndl->priv.mutex));
          break;
        }
      }
      else
      {
        DSI_DECR_COUNT(i, st_hndl,st_hndl->priv.ip_version);
      }
      DSI_UNLOCK_MUTEX(&(st_hndl->priv.mutex));

      /* check interface ref count in local state */
      if ( (( DSI_IP_VERSION_4 == st_hndl->priv.ip_version ) && (0 == DSI_GET_V4_COUNT(i)))||
           (( DSI_IP_VERSION_6 == st_hndl->priv.ip_version ) && (0 == DSI_GET_V6_COUNT(i))) ||
           (( DSI_IP_VERSION_4_6 == st_hndl->priv.ip_version ) && ((0 == DSI_GET_V4_COUNT(i))
             || (0 == DSI_GET_V6_COUNT(i)))) )
      {
        DSI_LOG_DEBUG("ref count on interface [%d] reached zero",
                      i);

        DSI_LOCK_MUTEX(&(st_hndl->priv.mutex));
        DSI_UPDATE_CALL_STATE(st_hndl, DSI_STATE_CALL_DISCONNECTING);
        DSI_UNLOCK_MUTEX(&(st_hndl->priv.mutex));

        /* call the lower layer to stop modem network iface */
        ret = dsi_mni_vtbl.mni_stop_f(i,st_hndl);
        if (DSI_ERROR == ret)
        {
          break;
        }
      }
      else
      {
        send_no_net = TRUE;
      }

    }
    ret = DSI_SUCCESS;
  } while (0);

  if(send_no_net)
  {
    DSI_LOG_DEBUG("interface [%d] has ref count v4/v6 [%d/%d] on it",
                  i, DSI_GET_V4_COUNT(i),DSI_GET_V6_COUNT(i));

    /* notify user that network is down (even though it's not) */
    if( NULL != st_hndl->net_ev_cb )
    {
      DSI_LOG_DEBUG("notifying user of st_hndl [%p] about NET_NO_NET",
                    st_hndl);
      st_hndl->net_ev_cb((dsi_hndl_t *)st_hndl,
                         st_hndl->user_data,
                         DSI_EVT_NET_NO_NET,
                         NULL);
    }
    else
    {
      DSI_LOG_ERROR("callback on store pointer [%p] is NULL", st_hndl);
    }

    /* detach this interface from the client store handle */
    DSI_LOG_DEBUG("st_hndl [%p] detached from interface [%d]",
                  st_hndl, i);
    DSI_LOCK_MUTEX(&(st_hndl->priv.mutex));
    dsi_detach_dsi_iface(st_hndl);
    DSI_UNLOCK_MUTEX(&(st_hndl->priv.mutex));

  }

  if (DSI_SUCCESS == ret)
  {
    DSI_LOG_DEBUG( "%s", "stop_data_call: EXIT with succ" );
  }
  else
  {
    DSI_LOG_DEBUG( "%s", "stop_data_call: EXIT with err" );
  }

  DSI_GLOBAL_UNLOCK;
  DSI_L2S_EXIT_WITH_STATUS("dsi_hndl=%p", (unsigned int*)hndl);
  return ret;
}

/*===========================================================================
  FUNCTION:  dsi_set_data_call_param
===========================================================================*/
/*!
    @brief
    User can use this function to set the data call parameter
    before trying to start a data call.

    @return
    DSI_ERROR
    DSI_SUCCESS

*/
/*=========================================================================*/
int dsi_set_data_call_param(
  dsi_hndl_t hndl,
  dsi_call_param_identifier_t identifier,
  dsi_call_param_value_t * info
)
{
  dsi_store_t * st = NULL;
  unsigned int count;
  int profile_id_num = 0;
  int auth_pref = -1;
  int dsi_tech = DSI_RADIO_TECH_UNKNOWN, temp_tech;
  int ret = DSI_ERROR;
  int reti = DSI_SUCCESS;
  boolean tech_found = DSI_FALSE;

  DSI_LOG_DEBUG( "%s", "set_data_call_param: ENTRY" );

  DSI_GLOBAL_LOCK;

  st = ( (dsi_store_t *) hndl );

  /* this do..while loop decides the overall return value.
     set ret to ERROR at the beginning. set ret to SUCCESS
     at the end. If there was an error in the middle, we break out*/
  do
  {
    ret = DSI_ERROR;

    if (!dsi_inited)
    {
      DSI_LOG_ERROR("%s","set_data_call_param: dsi not inited");
      break;
    }

    if ( !DSI_IS_HNDL_VALID( st ) || !DSI_IS_IDENT_VALID ( identifier ) || ( info == NULL ) )
    {
      DSI_LOG_ERROR( "cannot set, inval arg, st_hndl [%p], ident [%u], info [%p]",
                     (unsigned int*)st, identifier, (unsigned int*)info );
      break;
    }

    DSI_LOG_VERBOSE( "set param for dsi_hndl [%p], ident [%d], num_val [%d]",
                     (unsigned int*)st, identifier, info->num_val );

    reti = DSI_SUCCESS;
    switch( identifier )
    {
    case DSI_CALL_INFO_TECH_PREF:
      /* sanity check the value received */
      dsi_tech = info->num_val;
      temp_tech = DSI_RADIO_TECH_MIN;
      tech_found = DSI_FALSE;
      for(count=0; count<DSI_RADIO_TECH_MAX; count++)
      {
        /* if XORing results in all 0's
           i.e. we found an exact match of bitmap */
        if (!(temp_tech ^ dsi_tech))
        {
          if (DSI_TRUE == tech_found)
          {
            /* we had already found a technology match
            * use provided more than one t echnology */
            DSI_LOG_ERROR("multiple technologies provided by user " \
                          "in [%d]", (int)dsi_tech);
            tech_found = DSI_FALSE;
            break;
          }
          else
          {
            /* we found a technology match */
            tech_found = DSI_TRUE;
            DSI_LOG_DEBUG("tech match found [%d]", temp_tech);
            /* even if a match is found here, continue
            * this loop to see if user provided more than
            * one technology by mistake */
          }
        }
        /* other wise go to the next bitmap */
        temp_tech = temp_tech << 1;
      }

      /* validate dsi_tech */
      if(tech_found == DSI_FALSE)
      {
        DSI_LOG_ERROR("received invalid technology [%d]",
                      dsi_tech);
        reti = DSI_ERROR;
        break;
      }
      else if(DSI_RADIO_TECH_CDMA != dsi_tech &&
              DSI_RADIO_TECH_UMTS != dsi_tech)
      {
        /* QMI SNI only support two broad technologies
        * 1. cdma 2. umts. For the rest, set the dsi
        * tech directly in st only to determine the
        * corresponding modem */
        st->dsi_tech = dsi_tech;
        break;
      }
      else if (-1 ==
               (temp_tech = (int)dsi_map_dsi_to_qmi_tech(dsi_tech)))
      {
        DSI_LOG_ERROR("received valid dsi technology [%d] " \
                      "that won't match with a valid qmi tech",
                      dsi_tech);
        reti = DSI_ERROR;
        break;
      }
      else
      {
        /* use static table to convert dsi_tech to qmi_tech */
        DSI_LOG_DEBUG("setting qmi tech pref %d in start_nw_params",
                      temp_tech);
        st->priv.start_nw_params.params_mask |=
          QMI_WDS_START_NW_TECH_PREF_PARAM;
        st->priv.start_nw_params.tech_pref = (unsigned char)temp_tech;
        DSI_L2S_FORMAT_MSG(DSI_L2S_INVALID_IFACE, "dsi_hndl=%p|TECH_PREF=%d",
                           (unsigned int*)st, temp_tech);
      }
      break;
    case DSI_CALL_INFO_CDMA_PROFILE_IDX:
    case DSI_CALL_INFO_UMTS_PROFILE_IDX:
      profile_id_num = info->num_val;

      switch( identifier )
      {
      case DSI_CALL_INFO_CDMA_PROFILE_IDX:
        st->priv.start_nw_params.params_mask |=
          QMI_WDS_START_NW_PROFILE_IDX_3GPP2_PARAM;
        st->priv.start_nw_params.profile_index_3gpp2 =
          (unsigned char)profile_id_num;
        DSI_L2S_FORMAT_MSG(DSI_L2S_INVALID_IFACE, "dsi_hndl=%p|3GPP2_PROFILE_ID=%d",
                           (unsigned int*)st, profile_id_num);
        break;
      case DSI_CALL_INFO_UMTS_PROFILE_IDX:
        st->priv.start_nw_params.params_mask |=
          QMI_WDS_START_NW_PROFILE_IDX_PARAM;
        st->priv.start_nw_params.profile_index =
          (unsigned char)profile_id_num;
        DSI_L2S_FORMAT_MSG(DSI_L2S_INVALID_IFACE, "dsi_hndl=%p|PROFILE_ID=%d",
                           (unsigned int*)st, profile_id_num);
        break;
      default:
        reti = DSI_ERROR;
        DSI_ASSERT(0, "memory corruption");
        break;
      }

      DSI_LOG_VERBOSE("dsnet_policy profile_id [%d]",
                      profile_id_num);
      break;
    case DSI_CALL_INFO_APN_NAME:

      if (info->num_val > QMI_WDS_MAX_APN_STR_SIZE-1 ||
          info->num_val < 0)
      {
        DSI_LOG_ERROR("cannot set APN, invalid length, num_val [%d]",
                      info->num_val );
        reti = DSI_ERROR;
        break;
      }

      st->priv.start_nw_params.params_mask |=
        QMI_WDS_START_NW_APN_NAME_PARAM;
      memcpy
      (
        st->priv.start_nw_params.apn_name,
        info->buf_val,
        (size_t)info->num_val
      );
      st->priv.start_nw_params.apn_name[info->num_val] = '\0';

      DSI_L2S_FORMAT_MSG(DSI_L2S_INVALID_IFACE, "dsi_hndl=%p|APN_NAME=%s" ,
                         (unsigned int*)st ,
                         st->priv.start_nw_params.apn_name );
      DSI_LOG_VERBOSE("apn name set to [%s]",
                      st->priv.start_nw_params.apn_name);
      break;
    case DSI_CALL_INFO_USERNAME:
      if (info->num_val > QMI_WDS_MAX_USERNAME_PASS_STR_SIZE-1 ||
          info->num_val <= 0)
      {
        DSI_LOG_ERROR( "cannot set UserName, invalid length, num_val [%d]",
                       info->num_val );
        reti = DSI_ERROR;
        break;
      }
      memcpy
      (
        st->priv.start_nw_params.username,
        info->buf_val,
        (size_t)info->num_val
      );
      st->priv.start_nw_params.username[info->num_val] = '\0';
      st->priv.start_nw_params.params_mask |=
        QMI_WDS_START_NW_USERNAME_PARAM;

      DSI_LOG_VERBOSE("start_nw_params user name set to [%s]",
                      st->priv.start_nw_params.username);
      break;
    case DSI_CALL_INFO_PASSWORD:
      if ( info->num_val > QMI_WDS_MAX_USERNAME_PASS_STR_SIZE-1 ||
           info->num_val <= 0)
      {
        DSI_LOG_ERROR( "cannot set Password, invalid length, num_val [%d]",
                       info->num_val );
        reti = DSI_ERROR;
        break;
      }

      memcpy
      (
        st->priv.start_nw_params.password,
        info->buf_val,
        (size_t)info->num_val
      );

      st->priv.start_nw_params.password[info->num_val] = '\0';
      st->priv.start_nw_params.params_mask |=
        QMI_WDS_START_NW_PASSWORD_PARAM;

      DSI_LOG_VERBOSE( "start_nw_params password set to [%s]",
                       st->priv.start_nw_params.password);
      break;
    case DSI_CALL_INFO_AUTH_PREF:
      auth_pref = info->num_val;

      switch(auth_pref)
      {
      case DSI_AUTH_PREF_PAP_CHAP_NOT_ALLOWED:
        st->priv.start_nw_params.auth_pref =
          QMI_WDS_AUTH_PREF_PAP_CHAP_NOT_ALLOWED;
        break;
      case DSI_AUTH_PREF_PAP_ONLY_ALLOWED:
        st->priv.start_nw_params.auth_pref =
          QMI_WDS_AUTH_PREF_PAP_ONLY_ALLOWED;
        break;
      case DSI_AUTH_PREF_CHAP_ONLY_ALLOWED:
        st->priv.start_nw_params.auth_pref =
          QMI_WDS_AUTH_PREF_CHAP_ONLY_ALLOWED;
        break;
      case DSI_AUTH_PREF_PAP_CHAP_BOTH_ALLOWED:
        st->priv.start_nw_params.auth_pref =
          QMI_WDS_AUTH_PREF_PAP_CHAP_BOTH_ALLOWED;
        break;
      default:
        DSI_LOG_ERROR("Invalid Authentication type specified [%d]",
                      auth_pref);
        reti = DSI_ERROR;
        break;
      }

      if (reti != DSI_ERROR)
      {
        st->priv.start_nw_params.params_mask |=
          QMI_WDS_START_NW_AUTH_PREF_PARAM;
      }

     break;
    case DSI_CALL_INFO_CALL_TYPE:
      reti = DSI_SUCCESS;
      switch(info->num_val)
      {
      case DSI_CALL_TYPE_TETHERED:
        st->priv.start_nw_params.params_mask |=
          QMI_WDS_START_NW_DATA_CALL_ORIGIN_PARAM;
        st->priv.start_nw_params.data_call_origin =
          QMI_WDS_DATA_CALL_ORIGIN_LAPTOP;
        DSI_LOG_DEBUG("%s","data call explicitly set in tethered mode");
        DSI_L2S_FORMAT_MSG(DSI_L2S_INVALID_IFACE, "dsi_hndl=%p call_type=TETHERED", st);
        break;
      case DSI_CALL_TYPE_EMBEDDED:
        st->priv.start_nw_params.params_mask |=
          QMI_WDS_START_NW_DATA_CALL_ORIGIN_PARAM;
        st->priv.start_nw_params.data_call_origin =
          QMI_WDS_DATA_CALL_ORIGIN_EMBEDDED;
        DSI_LOG_DEBUG("%s","data call explicitly set in embedded mode");
        DSI_L2S_FORMAT_MSG(DSI_L2S_INVALID_IFACE, "dsi_hndl=%p call_type=EMBEDDED", st);
        break;
      default:
        DSI_LOG_ERROR("invalid call type specified [%d]",
                      info->num_val);
        reti = DSI_ERROR;
        break;
      }
      break;
    case DSI_CALL_INFO_IP_VERSION:
      reti = DSI_SUCCESS;
      st->priv.ip_version = info->num_val;

      switch(info->num_val)
      {
      case DSI_IP_VERSION_4:
      st->priv.start_nw_params.params_mask |=
        QMI_WDS_START_NW_IP_FAMILY_PREF_PARAM;

      st->priv.start_nw_params.ip_family_pref =
        QMI_WDS_IP_FAMILY_PREF_IPV4;
      DSI_L2S_FORMAT_MSG(DSI_L2S_INVALID_IFACE, "dsi_hndl=%p ip_version=IPV4", st);
        break;

      case DSI_IP_VERSION_6:
      st->priv.start_nw_params.params_mask |=
        QMI_WDS_START_NW_IP_FAMILY_PREF_PARAM;

      st->priv.start_nw_params.ip_family_pref =
        QMI_WDS_IP_FAMILY_PREF_IPV6;
      DSI_L2S_FORMAT_MSG(DSI_L2S_INVALID_IFACE, "dsi_hndl=%p ip_version=IPV6", st);
        break;

      case DSI_IP_VERSION_4_6:
      st->priv.start_nw_params.params_mask |=
        QMI_WDS_START_NW_IP_FAMILY_PREF_PARAM;

      /* For a Dual-IP call ip_family_pref should be
         QMI_WDS_IP_FAMILY_PREF_UNSPECIFIED per QMI WDS spec */
      st->priv.start_nw_params.ip_family_pref =
        QMI_WDS_IP_FAMILY_PREF_UNSPECIFIED;
      DSI_L2S_FORMAT_MSG(DSI_L2S_INVALID_IFACE, "dsi_hndl=%p ip_version=IPV4V6", st);
        break;

      default:
        DSI_LOG_ERROR("invalid IP family specified [%d]",
                      info->num_val);
        reti = DSI_ERROR;
        break;
      }
      break;
    case DSI_CALL_INFO_EXT_TECH_PREF:
      reti = DSI_SUCCESS;
      switch(info->num_val)
      {
      case DSI_EXT_TECH_EMBMS:
        st->priv.start_nw_params.params_mask |=
          QMI_WDS_START_NW_XTENDED_TECH_PREF_PARAM;
        st->priv.start_nw_params.xtended_tech_pref =
          QMI_WDS_IFACE_NAME_EMBMS;
        DSI_L2S_FORMAT_MSG(DSI_L2S_INVALID_IFACE, "dsi_hndl=%p EXTENDED_TECH=EMBMS", st);
        break;

      case DSI_EXT_TECH_MODEM_LINK_LOCAL:
        st->priv.start_nw_params.params_mask |=
          QMI_WDS_START_NW_XTENDED_TECH_PREF_PARAM;
        st->priv.start_nw_params.xtended_tech_pref =
          QMI_WDS_IFACE_NAME_MODEM_LINK_LOCAL;
        DSI_L2S_FORMAT_MSG(DSI_L2S_INVALID_IFACE, "dsi_hndl=%p EXTENDED_TECH=LINK_LOCAL", st);
        break;

      default:
        DSI_LOG_ERROR("no processing for xtended tech pref [%d]",
                      info->num_val);
        reti = DSI_ERROR;
        break;
      }
      break;

    case DSI_CALL_INFO_PARTIAL_RETRY:
      st->priv.partial_retry = (boolean) info->num_val;
      DSI_LOG_VERBOSE("partial retry [%d]",
                      info->num_val);
      DSI_L2S_FORMAT_MSG(DSI_L2S_INVALID_IFACE, "dsi_hndl=%p PARTIAL_RETRY=%d",
                         st, info->num_val);
      break;

    case DSI_CALL_INFO_APP_TYPE:
      st->priv.app_type = info->num_val;
      DSI_LOG_VERBOSE("app_type [%d]",
                      info->num_val);
      DSI_L2S_FORMAT_MSG(DSI_L2S_INVALID_IFACE, "dsi_hndl=%p APP_TYPE=%d",
                         st,info->num_val);
      break;

    default:
      DSI_LOG_ERROR( "cannot set, unknown ident [%d]", identifier );
      reti = DSI_ERROR;
      break;
    }/* switch() */
    if (reti == DSI_ERROR)
    {
      break;
    }

    ret = DSI_SUCCESS;
  } while (0);

  if (ret == DSI_SUCCESS)
  {
    DSI_LOG_DEBUG( "%s", "set_data_call_param: EXIT with suc" );
  }
  else
  {
    DSI_LOG_DEBUG( "%s", "set_data_call_param: EXIT with err" );
  }

  DSI_GLOBAL_UNLOCK;

  return ret;
}/* set_data_call_param() */

/*===========================================================================
  FUNCTION:  dsi_get_device_name
===========================================================================*/
/*!
    @brief
    used to release data service handle.

    @return
    DSI_ERROR
    DSI_SUCCESS

    @note
    len should be at least DSI_CALL_INFO_DEVICE_NAME_MAX_LEN+1
    long
*/
/*=========================================================================*/
int dsi_get_device_name(dsi_hndl_t hndl, char * buf, int len)
{
  int ret = DSI_ERROR;
  dsi_store_t * st = NULL;
  int i = 0;

  do
  {
    ret = DSI_ERROR;

    DSI_LOG_INFO("%s","dsi_get_device_name: ENTRY");

    st = (dsi_store_t *)hndl;

    if (!DSI_IS_HNDL_VALID(st))
    {
      DSI_LOG_ERROR("%s", "dsi_get_device_name: received invalid hndl");
      break;
    }

    i = st->priv.dsi_iface_id;

    if (!DSI_IS_ID_VALID(i))
    {
      DSI_LOG_ERROR("%s", "dsi_get_device_name: received invalid hndl");
      break;
    }

    /* a non-NULL buffer with at least DSI_CALL_INFO_DEVICE_NAME_MAX_LEN
     *  size is required (add 1 for NULL char) */
    if (NULL == buf || len < DSI_CALL_INFO_DEVICE_NAME_MAX_LEN+1)
    {
      DSI_LOG_ERROR("%s", "dsi_get_device_name: received invalid buf");
      break;
    }

    /* copy rmnetxx value into the buffer */
    DSI_LOG_DEBUG("copying value [%s] at user provided location [%p]",
                  DSI_GET_DEV_STR(i), buf);
    strlcpy(buf, DSI_GET_DEV_STR(i), (size_t)len);

   ret = DSI_SUCCESS;
  } while (0);

  if (ret == DSI_ERROR)
  {
    DSI_LOG_ERROR("%s","dsi_get_device_name: EXIT with err");
  }
  else
  {
    DSI_LOG_INFO("%s","dsi_get_device_name: EXIT with suc");
  }

  return ret;
}

/*===========================================================================
  FUNCTION:  dsi_get_call_end_reason
===========================================================================*/
/*!
    @brief
    used to get call end reason.

    @return
    DSI_ERROR
    DSI_SUCCESS

*/
/*=========================================================================*/
int dsi_get_call_end_reason
(
  dsi_hndl_t hndl,
  dsi_ce_reason_t * ce_reason,
  dsi_ip_family_t ipf
)
{
  dsi_store_t * st = (dsi_store_t *)hndl;

  DSI_GLOBAL_LOCK;
  DSI_LOG_DEBUG("%s","dsi_get_call_end_reason ENTRY");

  if (DSI_IS_HNDL_VALID(st)   &&
      NULL != ce_reason       &&
      DSI_IS_IP_FAMILY_VALID(ipf))
  {
    DSI_LOG_DEBUG("returning reason code/type [%d/%d] on"
                  " handle [%p]", st->priv.ce_reason[ipf].reason_code,
                  st->priv.ce_reason[ipf].reason_type, st);
    ce_reason->reason_code = st->priv.ce_reason[ipf].reason_code;
    ce_reason->reason_type = st->priv.ce_reason[ipf].reason_type;

    DSI_GLOBAL_UNLOCK;
    DSI_LOG_DEBUG("%s","dsi_get_call_end_reason EXIT success");
    return DSI_SUCCESS;
  }

  DSI_LOG_DEBUG("%s","dsi_get_call_end_reason EXIT error");
  DSI_GLOBAL_UNLOCK;
  return DSI_ERROR;
}

/*===========================================================================
  FUNCTION:  dsi_get_call_tech
===========================================================================*/
/*!
    @brief
    This API used to get techcology on which the call was up. This API can be
    called anywhere after the client receives DSI_EVT_NET_IS_CONN event and
    before the client releases the dsi handle.

    on Successful return, call_tech param will be set to a valid call tech.

    @return
    DSI_ERROR
    DSI_SUCCESS

*/
/*=========================================================================*/
int dsi_get_call_tech
(
  dsi_hndl_t hndl,
  dsi_call_tech_type *call_tech
)
{
  dsi_store_t * st = (dsi_store_t *)hndl;

  DSI_GLOBAL_LOCK;
  DSI_LOG_DEBUG("%s","dsi_get_call_tech ENTRY");

  if (DSI_IS_HNDL_VALID(st) && NULL != call_tech)
  {
    *call_tech = DSI_EXT_TECH_INVALID;

    switch(st->priv.call_tech)
    {
      case QMI_WDS_IFACE_NAME_CDMA:
        *call_tech = DSI_EXT_TECH_CDMA;
      break;

      case QMI_WDS_IFACE_NAME_UTMS:
        *call_tech = DSI_EXT_TECH_UMTS;
      break;

      case QMI_WDS_IFACE_NAME_EMBMS:
        *call_tech = DSI_EXT_TECH_EMBMS;
      break;

      case QMI_WDS_IFACE_NAME_MODEM_LINK_LOCAL:
        *call_tech = DSI_EXT_TECH_MODEM_LINK_LOCAL;
      break;

      default:
        DSI_LOG_ERROR("call tech [%x] unknown", st->priv.call_tech);
      break;
    }

    DSI_GLOBAL_UNLOCK;
    DSI_LOG_DEBUG("%s","dsi_get_call_tech EXIT success");
    return DSI_SUCCESS;
  }

  DSI_LOG_DEBUG("%s","dsi_get_call_tech EXIT error");
  DSI_GLOBAL_UNLOCK;
  return DSI_ERROR;
}

/*===========================================================================
  FUNCTION:  dsi_get_ip_addr_count
===========================================================================*/
/*!
    @brief
    Get then number of IP addresses (IPv4 and global IPv6) associated with
    the DSI interface.

    @param
    hndl - dsi interface handle

    @return
    The number of IP addresses associated with the DSI interface.
*/
/*=========================================================================*/
unsigned int dsi_get_ip_addr_count
(
  dsi_hndl_t hndl
)
{
  list_type* list = NULL;
  unsigned int count = 0;
  dsi_store_t* st = (dsi_store_t *) hndl;

  DSI_GLOBAL_LOCK;
  DSI_LOG_DEBUG("%s","dsi_get_ip_addr_count: ENTRY");

  if (!DSI_IS_HNDL_VALID(st))
  {
    DSI_LOG_ERROR("invalid dsi handle [%p]d rcvd", hndl);
  }
  else if (!DSI_IS_ID_VALID(st->priv.dsi_iface_id))
  {
    DSI_LOG_ERROR("invalid dsi_iface_id=%d", st->priv.dsi_iface_id);
  }
  else
  {
    dsi_ip_family_t ipf;

    for (ipf = DSI_IP_FAMILY_V4; ipf < DSI_NUM_IP_FAMILIES; ++ipf)
    {
      if (TRUE == DSI_IS_ADDR_VALID(st->priv.dsi_iface_id, ipf, iface_addr))
      {
        ++count;
      }
    }
  }

  DSI_LOG_DEBUG("dsi_get_ip_addr_count: found [%d] valid addresses", count);

  DSI_LOG_DEBUG("%s","dsi_get_ip_addr_count: EXIT");

  DSI_GLOBAL_UNLOCK;
  return count;
}

/*===========================================================================
  FUNCTION:  dsi_get_ip_addr
===========================================================================*/
/*!
    @brief
    used to get IP address information structure (network order)

    @return
    DSI_ERROR
    DSI_SUCCESS

*/
/*=========================================================================*/
int dsi_get_ip_addr
(
  dsi_hndl_t hndl,
  dsi_addr_info_t * info_ptr,
  int len
)
{
  dsi_store_t * st = NULL;
  int ret = DSI_ERROR;
  int reti = DSI_SUCCESS;
  int i, j;
  int ip_family;

  DSI_GLOBAL_LOCK;
  DSI_LOG_DEBUG("%s","dsi_get_ip_addr ENTRY");

  ret = DSI_ERROR;
  do
  {
    st = (dsi_store_t *)hndl;

    if (NULL == info_ptr || !DSI_IS_HNDL_VALID(st) || len < 1)
    {
      DSI_LOG_ERROR("%s","invalid params rcvd");
      break;
    }

    if (!DSI_IS_ID_VALID(st->priv.dsi_iface_id))
    {
      DSI_LOG_ERROR("invalid dsi_iface_id=%d", st->priv.dsi_iface_id);
      break;
    }

    if((len == 1) && (st->priv.ip_version != DSI_IP_VERSION_4_6))
    {
      ip_family = (st->priv.ip_version == DSI_IP_VERSION_4) ?
                      (DSI_IP_FAMILY_V4):(DSI_IP_FAMILY_V6);

      info_ptr[0] = DSI_GET_ADDRINFO(st->priv.dsi_iface_id, ip_family);

      ret = DSI_SUCCESS;

    }
    else
    {
      for (i = 0, j = 0; i < DSI_NUM_IP_FAMILIES && j < len; ++i)
      {
        if (!DSI_IS_ADDR_VALID(st->priv.dsi_iface_id, i, iface_addr))
        {
          DSI_LOG_DEBUG("found no valid address for st [%p], ip_family [%d]", st, i);
          continue;
        }

        DSI_LOG_DEBUG("found valid address for st [%p], ip_family [%d]", st, i);

    #if 0 /* Suppressed to reduce redundant log messages */
        reti = DSI_SUCCESS;

        /* Interface address */
        if( DSI_IS_ADDR_FAMILY_VALID( &DSI_GET_ADDR(st->priv.dsi_iface_id,iface_addr) ) ) {
          DSI_INET_NTOP( "Interface ", &DSI_GET_ADDR(st->priv.dsi_iface_id,iface_addr) );
        }
        else
        {
          DSI_LOG_ERROR("programming err: invalid iface address ss_family [%d]",
                        DSI_GET_ADDR(st->priv.dsi_iface_id,iface_addr).ss_family);
          reti = DSI_ERROR;
        }

        /* Gateway server address */
        if( DSI_IS_ADDR_FAMILY_VALID( &DSI_GET_ADDR(st->priv.dsi_iface_id,gtwy_addr) ) ) {
          DSI_INET_NTOP( "Gateway ", &DSI_GET_ADDR(st->priv.dsi_iface_id,gtwy_addr) );
        }
        else
        {
          DSI_LOG_ERROR("programming err: invalid gateway server ss_family [%d]",
                        DSI_GET_ADDR(st->priv.dsi_iface_id,gtwy_addr).ss_family);
          reti = DSI_ERROR;
        }

        /* DNS Primary server address */
        if( DSI_IS_ADDR_FAMILY_VALID( &DSI_GET_ADDR(st->priv.dsi_iface_id,dnsp_addr) ) ) {
          DSI_INET_NTOP( "DNS Primary ", &DSI_GET_ADDR(st->priv.dsi_iface_id,dnsp_addr) );
        }
        else
        {
          DSI_LOG_ERROR("programming err: invalid DNS primary server ss_family [%d]",
                        DSI_GET_ADDR(st->priv.dsi_iface_id,dnsp_addr).ss_family);
          reti = DSI_ERROR;
        }

        /* DNS Secondary server address */
        if( DSI_IS_ADDR_FAMILY_VALID( &DSI_GET_ADDR(st->priv.dsi_iface_id,dnss_addr) ) ) {
          DSI_INET_NTOP( "DNS Secondary ", &DSI_GET_ADDR(st->priv.dsi_iface_id,dnss_addr) );
        }
        else
        {
          DSI_LOG_ERROR("programming err: invalid DNS secondary server ss_family [%d]",
                        DSI_GET_ADDR(st->priv.dsi_iface_id,dnss_addr).ss_family);
          reti = DSI_ERROR;
        }

        if (DSI_SUCCESS != reti)
        {
          break;
        }
    #endif /* 0*/

        /* copy address info into user buffer */
        info_ptr[j++] = DSI_GET_ADDRINFO(st->priv.dsi_iface_id, i);
      }

      if (j > 0)
      {
        ret = DSI_SUCCESS;
      }

    }


  } while(0);

  if (DSI_SUCCESS == ret)
  {
    DSI_LOG_DEBUG("%s","dsi_get_ip_addr EXIT success");
  }
  else
  {
    DSI_LOG_DEBUG("%s","dsi_get_ip_addr EXIT error");
  }

  DSI_GLOBAL_UNLOCK;

  return ret;
}


/*===========================================================================
  FUNCTION:  dsi_iface_ioctl
===========================================================================*/
/*!
    @brief
    This function is in interface to get IOCTL values.

    @return
    DSI_ERROR
    DSI_SUCCESS
*/
/*=========================================================================*/
int dsi_iface_ioctl
(
  dsi_hndl_t dsi_hndl,
  dsi_iface_ioctl_enum_t ioctl_name,
  void *argval_ptr,
  int  *err_code
)
{
  dsi_store_t* st = NULL;
  int ret = DSI_ERROR;

  *err_code = DSI_SUCCESS;

  DSI_LOG_DEBUG("%s", "dsi_iface_ioctl(): ENTRY" );

  st = (dsi_store_t *)dsi_hndl;

  if (!DSI_IS_HNDL_VALID(st))
  {
    DSI_LOG_ERROR("%s","dsi_iface_ioctl(): invalid params rcvd");
    *err_code = DSI_EBADF;
    goto err_lbl;
  }

  if (!DSI_IS_ID_VALID(st->priv.dsi_iface_id))
  {
    DSI_LOG_ERROR("dsi_iface_ioctl(): invalid dsi_iface_id=%d", st->priv.dsi_iface_id);
    *err_code = DSI_EBADF;
    goto err_lbl;
  }

  DSI_LOG_DEBUG("dsi_iface_ioctl(): hndl:[0x%x] iface_id:[%d] ioctl:[%d]", dsi_hndl, st->priv.dsi_iface_id, ioctl_name);

  switch (ioctl_name)
  {
    case DSI_IFACE_IOCTL_GET_PCSCF_SERV_ADDRESS:
      ret = dsi_iface_ioctl_get_pcscf_address(DSI_GET_QDI_HNDL(st->priv.dsi_iface_id),
                                              st->priv.ip_version,
                                              argval_ptr);
      break;
    case DSI_IFACE_IOCTL_GET_PCSCF_DOMAIN_NAME_LIST:
      ret = dsi_iface_ioctl_get_pcscf_fqdn_list(DSI_GET_QDI_HNDL(st->priv.dsi_iface_id),
                                                st->priv.ip_version,
                                                argval_ptr);
      break;
    case DSI_IFACE_IOCTL_REMOVE_DELEGATED_IPV6_PREFIX:
      ret = dsi_iface_ioctl_remove_delegated_ipv6_prefix(DSI_GET_QDI_HNDL(st->priv.dsi_iface_id),
                                                         argval_ptr);
      break;
    default:
      DSI_LOG_ERROR("dsi_iface_ioctl(): Invalid IOCTL name [%d]", ioctl_name);
      *err_code = DSI_EFAULT;
      break;
  }

err_lbl:
  DSI_LOG_DEBUG("%s","dsi_iface_ioctl(): EXIT" );
  return ret;
}

/*===========================================================================
  FUNCTION:  dsi_rel_data_srvc_hndl
===========================================================================*/
/*!
    @brief
    used to release data service handle.

    @return
    None

    @note
    If user ever started an interface with this hndl,
    the corresponding interface would be stopped before
    the dsi hndl is released
*/
/*=========================================================================*/
void dsi_rel_data_srvc_hndl
(
  dsi_hndl_t hndl
)
{
  int ret = DSI_ERROR;
  dsi_store_t *st_hndl = NULL;
  short int index = DSI_INVALID_IFACE;

  DSI_LOG_DEBUG( "%s", "rel_data_srvc_hndl: ENTRY" );

  DSI_GLOBAL_LOCK;

  st_hndl = ((dsi_store_t *)(hndl) );

  do
  {
    if (!dsi_inited)
    {
      DSI_LOG_ERROR("%s","rel_data_srvc_hndl: dsi not inited");
      break;
    }

    if (!(DSI_IS_HNDL_VALID(st_hndl)))
    {
      DSI_LOG_ERROR( "inval arg, store hndl [%p]", (unsigned int*)st_hndl );
      break;
    }

    for (index=0; index < DSI_MAX_DATA_CALLS; index ++)
    {
      if (dsi_store_table[index].dsi_store_ptr == hndl)
      {
        break;
      }
    }

    if (index == DSI_MAX_DATA_CALLS)
    {
      /* User error? Gave us a bad handle? */
      DSI_LOG_ERROR("rel_data_srvc_hndl: PANIC:Could not find " \
                    "the handle [%p] in the store table",
                    (unsigned int*)hndl);
      DSI_ASSERT(0, "rel_data_srvc_hndl received a valid handle "
                 "that could not be located in our table");
      break;
    }

    DSI_LOG_DEBUG("rel_data_srvc_hndl: Found the index containing " \
                  "the store handle, %d", index );

    /* if the dsi hndl still refers to a valid dsi interface,
       stop the corresponding interface */
    if( (DSI_INVALID_IFACE != st_hndl->priv.dsi_iface_id) &&
        (DSI_STATE_CALL_DISCONNECTING != st_hndl->priv.call_state) )
    {
      DSI_LOG_INFO("now stopping dsi interface [%d] as it's referred " \
                   "by dsi hndl [%p]",
                   st_hndl->priv.dsi_iface_id,(unsigned int*)st_hndl);
      DSI_GLOBAL_UNLOCK;
      dsi_stop_data_call(st_hndl);
      DSI_GLOBAL_LOCK;
      /* detach this interface from the store handle
       * further notifications on this interface may not be
       * processed now that dsi handle is detached from the
       * iface */
      dsi_detach_dsi_iface(st_hndl);
    }

    ret = DSI_SUCCESS;
  } while (0);

  /* Release the QDI call handle */
  if (DSI_SUCCESS == ret &&
      QDI_INVALID_CALL_HANDLE != st_hndl->priv.qdi_call_hndl)
  {
    qdi_release_call_handle(st_hndl->priv.qdi_call_hndl);
    st_hndl->priv.qdi_call_hndl = QDI_INVALID_CALL_HANDLE;
  }

  /* clean up the dsi_store_table entry */
  DSI_LOG_DEBUG( "Try to de-allocate dsi store table"
                 " entry at index[%d]",
                 index);
  dsi_cleanup_store_tbl( index );

  if (DSI_SUCCESS == ret)
  {
    DSI_LOG_DEBUG("%s", "rel_data_srvc_hndl: EXIT with suc");
  }
  else
  {
    DSI_LOG_DEBUG("%s", "rel_data_srvc_hndl: EXIT with err");
  }

  DSI_GLOBAL_UNLOCK;

  return;
}

/*===========================================================================
  FUNCTION:  dsi_get_data_srvc_hndl
===========================================================================*/
/*!
    @brief
    Used to get data service handle. This handle contains a vtable (set of
    function pointers) that can be used by the user to exercise data service
    functionalities.

    @return
    dsi_hndl_t if successfull, NULL other wise
*/
/*=========================================================================*/
dsi_hndl_t dsi_get_data_srvc_hndl(
  dsi_net_ev_cb   user_cb_fn,
  void           *user_data
)
{
  dsi_store_t * st = NULL;
  int index  = 0;
  int ret = DSI_ERROR;
  dsi_ip_family_t ipf;

  DSI_L2S_ENTRY();
  DSI_LOG_DEBUG( "%s", "dsi_get_data_srvc_hndl: ENTRY" );

  DSI_GLOBAL_LOCK;

  /* this do..while loop decides the overall return value.
     set ret to ERROR at the beginning. set ret to SUCCESS
     at the end. If there was an error in the middle, we break out*/
  do
  {
    ret = DSI_ERROR;

    if (!dsi_inited)
    {
      DSI_LOG_ERROR("%s","dsi_get_data_srvc_hndl: " \
                    "dsi not inited");
      break;
    }

    /* this memory gets freed by dsi_cleanup_store_tbl function
       typically called by rel_data_srvc_hndl user API function */
    if ((st =(dsi_store_t*)malloc(sizeof(dsi_store_t)))== NULL )
    {
      DSI_LOG_ERROR( "%s", "alloc dsi obj FAILED" );
      break;
    }

    memset( st, 0, sizeof( dsi_store_t ) );
    DSI_LOG_DEBUG( "%s", "alloc dsi store successful" );

    /*
      Find a free table entry
    */
    for (index = 0; index < DSI_MAX_DATA_CALLS; index++)
    {
      if (!dsi_store_table[index].is_valid)
      {
        DSI_LOG_VERBOSE("found an un-used index [%d], " \
                        "store pointer is [%p]",index,
                        (unsigned int*)st );
        break;
      }
    }
    if (index == DSI_MAX_DATA_CALLS )
    {
      DSI_LOG_ERROR( "%s", "dsi_get_data_srvc_hndl: Couldnt " \
                     "find a free store table slot" );
      break;
    }
    else
    {
      dsi_store_table[index].dsi_store_ptr = st;
      dsi_store_table[index].is_valid = DSI_TRUE;
    }

    /* reset dsi_tech in st */
    st->dsi_tech = DSI_RADIO_TECH_UNKNOWN;
    /* reset param mask in start_nw_params */
    st->priv.start_nw_params.params_mask = 0;
    /* we don't have valid interface assigned to this
       dsi_hndl yet */
    st->priv.dsi_iface_id = DSI_INVALID_IFACE;
    DSI_UPDATE_CALL_STATE(st, DSI_STATE_CALL_IDLE);
    st->priv.partial_retry = FALSE;
    st->priv.qdi_call_hndl = QDI_INVALID_CALL_HANDLE;
    st->priv.ref_count = 0;

    /* set call end reason to undefined */
    for (ipf = DSI_IP_FAMILY_V4; ipf < DSI_NUM_IP_FAMILIES; ++ipf)
    {
      st->priv.ce_reason[ipf].reason_type = DSI_CE_TYPE_UNINIT;
      st->priv.ce_reason[ipf].reason_code = DSI_CE_REASON_UNKNOWN;
    }

    st->priv.call_tech = QMI_WDS_IFACE_NAME_NOT_REPORTED;

    /* reset route lookup QMI
       ]instance in st */
    st->priv.rl_qmi_inst = DSI_INVALID_QMI_INST;

    /* reset EMBMS related entry */
    memset(&(st->priv.embms_tmgi_list_info), 0, sizeof(st->priv.embms_tmgi_list_info));

    st->priv.embms_tmgi_list_info.list_type    = QMI_WDS_EMBMS_TMGI_LIST_INVALID;

    /* initialize mutex for this store handle */
    pthread_mutex_init(&(st->priv.mutex), NULL);

    /* Debug info for net_policy */
    DSI_LOG_VERBOSE( "%s", "start_nw_params reset to 0" );

    /* Populate */
    st->net_ev_cb                 = user_cb_fn;
    st->user_data                 = user_data;

    /* Mark as valid */
    st->self                      = st;

    DSI_LOG_VERBOSE( "data store is at [%p]", (unsigned int*)st );

    ret = DSI_SUCCESS;
  } while (0);

  if (ret == DSI_ERROR)
  {
    DSI_LOG_DEBUG("%s", "dsi_get_data_srvc_hndl: EXIT with err");
    if (st != NULL)
    {
      free(st);
      st = NULL;
    }
  }
  else
  {
    DSI_LOG_DEBUG("%s", "dsi_get_data_srvc_hndl: EXIT with suc");
  }

  DSI_GLOBAL_UNLOCK;
  DSI_L2S_EXIT_WITH_STATUS("dsi_hndl=%p", (unsigned int*)st);

  return (dsi_hndl_t)st;
}/* dsi_get_data_srvc_hndl() */

/*===========================================================================
  FUNTION:   dsi_get_current_channel_rate
===========================================================================*/
/*!
    @brief
    Returns the current channel rate of the call

    @param
    hndl - dsi interface handle

    @return
    DSI_ERROR
    DSI_SUCCESS
*/
/*=========================================================================*/
int dsi_get_current_data_channel_rate
(
  dsi_hndl_t               hndl,
  dsi_data_channel_rate_t *data_rate
)
{
  dsi_store_t*              st = (dsi_store_t *) hndl;
  int                       rc, qmi_err, qmi_wds_hndl = DSI_INVALID_WDS_HNDL;
  qmi_wds_channel_rate_type qmi_channel_rate;
  int                       ret = DSI_ERROR;

  DSI_LOG_DEBUG("%s","dsi_get_current_data_channel_rate: ENTRY");

  DSI_GLOBAL_LOCK;

  do
  {
    /* validate input parameter */
    if (NULL == data_rate) {
        DSI_LOG_ERROR("%s", "NULL input parameter");
        break;
    }

    if (!DSI_IS_HNDL_VALID(st))
    {
      DSI_LOG_ERROR("invalid dsi handle [%p]d rcvd", hndl);
      break;
    }

    if (!DSI_IS_ID_VALID(st->priv.dsi_iface_id))
    {
      DSI_LOG_ERROR("invalid dsi_iface_id=%d", st->priv.dsi_iface_id);
      break;
    }

    memset(&qmi_channel_rate, 0, sizeof(qmi_channel_rate));

    /* Directly obtain the QMI WDS handle from QDP. This will allow us to use
       the correct handle even for an IPv6 only call */
    if(st->priv.ip_version == DSI_IP_VERSION_4)
    {
      qmi_wds_hndl = qdi_get_qmi_wds_handle_for_ip(DSI_GET_QDI_HNDL(st->priv.dsi_iface_id),
                                                   QMI_IP_FAMILY_PREF_IPV4);
      DSI_LOG_DEBUG("qmi_wds_hndl for v4 = %d",qmi_wds_hndl);
    }
    else if(st->priv.ip_version == DSI_IP_VERSION_6)
    {
      qmi_wds_hndl = qdi_get_qmi_wds_handle_for_ip(DSI_GET_QDI_HNDL(st->priv.dsi_iface_id),
                                                   QMI_IP_FAMILY_PREF_IPV6);
      DSI_LOG_DEBUG("qmi_wds_hndl for v6 =%d", qmi_wds_hndl);
    }
    else if(st->priv.ip_version == DSI_IP_VERSION_4_6)
    {
      DSI_LOG_ERROR("Ip_version v4v6 not supported =%d", st->priv.ip_version);
      break;
    }
    if (DSI_INVALID_WDS_HNDL == qmi_wds_hndl)
    {
      DSI_LOG_ERROR("invalid WDS handle for iface=%d", st->priv.dsi_iface_id);
      break;
    }

    rc = qmi_wds_get_current_channel_rate(qmi_wds_hndl,
                                         &qmi_channel_rate,
                                         &qmi_err);

    if (rc < 0)
    {
      DSI_LOG_ERROR("qmi_wds_get_current_data_rate() iface=%d, rc=%d, qmi_err=%d",
                    st->priv.dsi_iface_id,
                    rc,
                    qmi_err);
      break;
    }

    DSI_LOG_DEBUG("recvd cur_channel_rate cur_tx_rate=%lu, cur_rx_rate=%lu, max_tx_rate=%lu, max_rx_rate=%lu",
                  qmi_channel_rate.current_channel_tx_rate,
                  qmi_channel_rate.current_channel_rx_rate,
                  qmi_channel_rate.max_channel_tx_rate,
                  qmi_channel_rate.max_channel_rx_rate);

    data_rate->current_tx_rate = qmi_channel_rate.current_channel_tx_rate;
    data_rate->current_rx_rate = qmi_channel_rate.current_channel_rx_rate;
    data_rate->max_tx_rate     = qmi_channel_rate.max_channel_tx_rate;
    data_rate->max_rx_rate     = qmi_channel_rate.max_channel_rx_rate;

    ret = DSI_SUCCESS;
  }
  while (0);

  DSI_GLOBAL_UNLOCK;

  DSI_LOG_DEBUG("%s","dsi_get_current_data_channel_rate: EXIT");

  return ret;
}

/*===========================================================================
  FUNCTION:  dsi_get_current_data_bearer_tech
===========================================================================*/
/*!
    @brief
    Returns the current data bearer technology on which call was brought up

    @param
    hndl - dsi interface handle

    @return
    data bearer technology
*/
/*=========================================================================*/
dsi_data_bearer_tech_t
dsi_get_current_data_bearer_tech
(
  dsi_hndl_t hndl
)
{
  dsi_data_bearer_tech_t  data_tech = DSI_DATA_BEARER_TECH_UNKNOWN;
  dsi_store_t* st = (dsi_store_t *) hndl;
  int rc, qmi_err, qmi_wds_hndl = DSI_INVALID_WDS_HNDL;
  qmi_wds_data_bearer_tech_type_ex qmi_bearer_tech_ex;
  qmi_wds_data_bearer_tech_type qmi_bearer_tech;

  DSI_LOG_DEBUG("%s","dsi_get_current_data_bearer_tech: ENTRY");

  DSI_GLOBAL_LOCK;

  do
  {
    if (!DSI_IS_HNDL_VALID(st))
    {
      DSI_LOG_ERROR("invalid dsi handle [%p]d rcvd", hndl);
      break;
    }

    if (!DSI_IS_ID_VALID(st->priv.dsi_iface_id))
    {
      DSI_LOG_ERROR("invalid dsi_iface_id=%d", st->priv.dsi_iface_id);
      break;
    }

    /* Directly obtain the QMI WDS handle from QDP. This will allow us to use
       the correct handle even for an IPv6 only call */
    if(st->priv.ip_version == DSI_IP_VERSION_4)
    {
      qmi_wds_hndl = qdi_get_qmi_wds_handle_for_ip(DSI_GET_QDI_HNDL(st->priv.dsi_iface_id),
                                                   QMI_IP_FAMILY_PREF_IPV4);
      DSI_LOG_DEBUG("qmi_wds_hndl for v4 = %d",qmi_wds_hndl);
    }
    else if(st->priv.ip_version == DSI_IP_VERSION_6)
    {
      qmi_wds_hndl = qdi_get_qmi_wds_handle_for_ip(DSI_GET_QDI_HNDL(st->priv.dsi_iface_id),
                                                   QMI_IP_FAMILY_PREF_IPV6);
      DSI_LOG_DEBUG("qmi_wds_hndl for v6 =%d", qmi_wds_hndl);
    }
    else if(st->priv.ip_version == DSI_IP_VERSION_4_6)
    {
      /* For dual-ip calls we need to get the correct handle based on the
       * packet data counters for that specific IP type since one call type
       * may come up before the other */
      qmi_wds_hndl = qdi_get_qmi_wds_handle(DSI_GET_QDI_HNDL(st->priv.dsi_iface_id));
      DSI_LOG_DEBUG("qmi_wds_hndl for v4v6 =%d", qmi_wds_hndl);
    }

    if (DSI_INVALID_WDS_HNDL == qmi_wds_hndl)
    {
      DSI_LOG_ERROR("invalid WDS handle for iface=%d", st->priv.dsi_iface_id);
      break;
    }

    memset(&qmi_bearer_tech_ex, 0, sizeof(qmi_bearer_tech_ex));
    memset(&qmi_bearer_tech, 0, sizeof(qmi_bearer_tech));

    rc = qmi_wds_get_current_bearer_tech_ex(qmi_wds_hndl,
                                            &qmi_bearer_tech_ex,
                                            &qmi_err);
    if(rc < 0)
    {
      DSI_LOG_ERROR("qmi_wds_get_current_bearer_tech_ex() iface=%d, rc=%d, qmi_err=%d",
                    st->priv.dsi_iface_id,
                    rc,
                    qmi_err);
      DSI_LOG_DEBUG("%s", "Fallback to qmi_wds_get_current_bearer_tech");

      rc = qmi_wds_get_current_bearer_tech(qmi_wds_hndl,
                                           &qmi_bearer_tech,
                                           &qmi_err);

      if(rc < 0)
      {
        DSI_LOG_ERROR("qmi_wds_get_current_bearer_tech() iface=%d, rc=%d, qmi_err=%d",
                      st->priv.dsi_iface_id,
                      rc,
                      qmi_err);
        break;
      }

      DSI_LOG_DEBUG("recvd cur_bearer_tech nw=0x%x, rat_mask=0x%x, so_mask=0x%x",
                    qmi_bearer_tech.current_db_nw,
                    qmi_bearer_tech.rat_mask,
                    qmi_bearer_tech.db_so_mask);

      data_tech = dsi_translate_qmi_to_dsi_bearer_tech(&qmi_bearer_tech);
    }
    else
    {
      DSI_LOG_DEBUG("recvd cur_bearer_tech_ex technology=0x%x, rat_mask=0x%x, so_mask=0x%x",
                    qmi_bearer_tech_ex.technology,
                    qmi_bearer_tech_ex.rat_mask,
                    qmi_bearer_tech_ex.so_mask);

      data_tech = dsi_translate_qmi_to_dsi_bearer_tech_ex(&qmi_bearer_tech_ex);
    }
  }
  while (0);

  DSI_GLOBAL_UNLOCK;

  DSI_LOG_DEBUG("data bearer tech=%d", data_tech);
  DSI_LOG_DEBUG("%s","dsi_get_current_data_bearer_tech: EXIT");

  return data_tech;
}

/*===========================================================================
  FUNCTION:  dsi_reset_pkt_stats
===========================================================================*/
/*!
    @brief
    Resets the packet data transfer statistics

    @param
    hndl - dsi interface handle

    @return
    DSI_ERROR
    DSI_SUCCESS
*/
/*=========================================================================*/
int dsi_reset_pkt_stats
(
  dsi_hndl_t  hndl
)
{
  dsi_store_t *st = (dsi_store_t *) hndl;
  int          rc, qmi_err, qmi_wds_hndl = DSI_INVALID_WDS_HNDL;
  int          ret = DSI_ERROR;
  int          rc_v6,qmi_err_v6,qmi_wds_v6_hndl = DSI_INVALID_WDS_HNDL;

  DSI_LOG_DEBUG("%s","dsi_reset_pkt_stats: ENTRY");

  DSI_GLOBAL_LOCK;

  do
  {
    if (!DSI_IS_HNDL_VALID(st))
    {
      DSI_LOG_ERROR("invalid dsi handle [%p] rcvd", hndl);
      break;
    }

    if (!DSI_IS_ID_VALID(st->priv.dsi_iface_id))
    {
      DSI_LOG_ERROR("invalid dsi_iface_id=%d", st->priv.dsi_iface_id);
      break;
    }

    /* Directly obtain the QMI WDS handle from QDI. This will allow us to use
       the correct handle even for an IPv6 only call */
    switch(st->priv.ip_version)
    {
      case DSI_IP_VERSION_4_6:
       /*Get QMI handle for ipv4 */
        qmi_wds_hndl = qdi_get_qmi_wds_handle_for_ip(DSI_GET_QDI_HNDL(st->priv.dsi_iface_id),
                                                     QMI_IP_FAMILY_PREF_IPV4);
       /*Get QMI handle for ipv6 */
        qmi_wds_v6_hndl = qdi_get_qmi_wds_handle_for_ip(DSI_GET_QDI_HNDL(st->priv.dsi_iface_id),
                                                     QMI_IP_FAMILY_PREF_IPV6);
       /* Resetting Packet stats for ipv4 first followed by ipv6 */
        if (DSI_INVALID_WDS_HNDL == qmi_wds_hndl && DSI_INVALID_WDS_HNDL == qmi_wds_v6_hndl)
        {
          DSI_LOG_ERROR("invalid WDS handle for iface=%d", st->priv.dsi_iface_id);
          break;
        }
        /* Resetting Packet stats for ipv4 first followed by ipv6 */
        rc    = qmi_wds_reset_pkt_statistics( qmi_wds_hndl, &qmi_err );
        rc_v6 = qmi_wds_reset_pkt_statistics( qmi_wds_v6_hndl, &qmi_err_v6 );

        if(0 > rc && 0 > rc_v6)
        {
          DSI_LOG_DEBUG( "dsi_reset_pkt_stats: reset stats err,iface=%d, rc=%d, qmi_err=%d",
                        st->priv.dsi_iface_id,
                        rc,
                        qmi_err);
          DSI_LOG_DEBUG( "dsi_reset_pkt_stats: reset stats err,iface=%d, rc=%d, qmi_err=%d",
                        st->priv.dsi_iface_id,
                        rc_v6,
                        qmi_err_v6);
          break;
        }
        ret = DSI_SUCCESS;
        break;
       /*-------------------------------------------------------------------------------------------------------------
             If ip version is ipv4 then clear ipv4  stats .
          --------------------------------------------------------------------------------------------------------------*/
      case DSI_IP_VERSION_4:
        /*Get QMI handle for ipv4 */
        qmi_wds_hndl = qdi_get_qmi_wds_handle_for_ip(DSI_GET_QDI_HNDL(st->priv.dsi_iface_id),
                                                     QMI_IP_FAMILY_PREF_IPV4);
        if (DSI_INVALID_WDS_HNDL == qmi_wds_hndl)
        {
          DSI_LOG_ERROR("invalid WDS handle for iface=%d", st->priv.dsi_iface_id);
          break;
        }
        /* Resetting Packet stats for ipv4  */
        rc = qmi_wds_reset_pkt_statistics( qmi_wds_hndl, &qmi_err );
        DSI_LOG_ERROR("Value v4  = %d" ,rc);

        if(0 > rc )
        {
          DSI_LOG_DEBUG( "dsi_reset_pkt_stats: reset stats err,iface=%d, rc=%d, qmi_err=%d",
                        st->priv.dsi_iface_id,
                        rc,
                        qmi_err);
          break;
        }
        ret = DSI_SUCCESS;
        break;
      /*-------------------------------------------------------------------------------------------------------------
             If ip version is ipv6 then clear ipv6 stats .
          --------------------------------------------------------------------------------------------------------------*/
      case DSI_IP_VERSION_6:
        /*Get QMI handle for ipv6 */
        qmi_wds_v6_hndl = qdi_get_qmi_wds_handle_for_ip(DSI_GET_QDI_HNDL(st->priv.dsi_iface_id),
                                                        QMI_IP_FAMILY_PREF_IPV6);
        if(DSI_INVALID_WDS_HNDL == qmi_wds_v6_hndl)
        {
          DSI_LOG_ERROR("invalid WDS handle for iface=%d", st->priv.dsi_iface_id);
          break;
        }
        /* Resetting Packet stats for ipv6 */
        rc_v6 = qmi_wds_reset_pkt_statistics( qmi_wds_v6_hndl, &qmi_err_v6 );
        DSI_LOG_ERROR("Value v6 = %d",rc_v6);

        if(0 > rc_v6 )
        {
          DSI_LOG_DEBUG( "dsi_reset_pkt_stats: reset stats err,iface=%d, rc=%d, qmi_err=%d",
                        st->priv.dsi_iface_id,
                        rc_v6,
                        qmi_err_v6);
          break;
        }
        ret = DSI_SUCCESS;
        break;

      default :
        DSI_LOG_ERROR("invalid IP version in dsi_reset_pkt_stats = %d", st->priv.ip_version);
    }
  }
  while (0);

  DSI_GLOBAL_UNLOCK;

  DSI_LOG_DEBUG("%s","dsi_reset_pkt_stats: EXIT");

  return ret;
}/* dsi_reset_pkt_stats */


/*===========================================================================
  FUNCTION:  dsi_get_pkt_stats
===========================================================================*/
/*!
    @brief
    Queries the packet data transfer statistics from the current packet
    data session

    @param
    hndl - dsi interface handle
    dsi_data_stats - memory to hold the queried statistics details

    @return
    DSI_ERROR
    DSI_SUCCESS
*/
/*=========================================================================*/
int dsi_get_pkt_stats
(
  dsi_hndl_t          hndl,
  dsi_data_pkt_stats *dsi_data_stats
)
{
  dsi_store_t       *st = (dsi_store_t *) hndl;
  int                rc, qmi_err, qmi_wds_hndl = DSI_INVALID_WDS_HNDL;
  int                rc_v6,qmi_err_v6,qmi_wds_v6_hndl = DSI_INVALID_WDS_HNDL;
  unsigned long      stats_mask;
  qmi_wds_xfer_stats stats_response,stats_response_v6;
  int                ret = DSI_ERROR;

  DSI_LOG_DEBUG("%s","dsi_get_pkt_stats: ENTRY");

  DSI_GLOBAL_LOCK;

  do
  {
    /* validate input parameter */
    if(NULL == dsi_data_stats)
    {
      DSI_LOG_ERROR("%s", "NULL input parameter");
      break;
    }

    if (!DSI_IS_HNDL_VALID(st))
    {
      DSI_LOG_ERROR("invalid dsi handle [%p] rcvd", hndl);
      break;
    }

    if (!DSI_IS_ID_VALID(st->priv.dsi_iface_id))
    {
      DSI_LOG_ERROR("invalid dsi_iface_id=%d", st->priv.dsi_iface_id);
      break;
    }

    /* Directly obtain the QMI WDS handle from QDI. This will allow us to use
       the correct handle even for an IPv6 only call */
    if(st->priv.ip_version == DSI_IP_VERSION_4)
    {
      qmi_wds_hndl = qdi_get_qmi_wds_handle_for_ip(DSI_GET_QDI_HNDL(st->priv.dsi_iface_id),
                                                   QMI_IP_FAMILY_PREF_IPV4);
      DSI_LOG_DEBUG("qmi_wds_hndl for v4 = %d",qmi_wds_hndl);
    }
    else if(st->priv.ip_version == DSI_IP_VERSION_6)
    {
      qmi_wds_hndl = qdi_get_qmi_wds_handle_for_ip(DSI_GET_QDI_HNDL(st->priv.dsi_iface_id),
                                                   QMI_IP_FAMILY_PREF_IPV6);
      DSI_LOG_DEBUG("qmi_wds_hndl for v6 =%d", qmi_wds_hndl);
    }
    else if(st->priv.ip_version == DSI_IP_VERSION_4_6)
    {
      qmi_wds_hndl    = qdi_get_qmi_wds_handle_for_ip(DSI_GET_QDI_HNDL(st->priv.dsi_iface_id),
                                                   QMI_IP_FAMILY_PREF_IPV4);
      qmi_wds_v6_hndl = qdi_get_qmi_wds_handle_for_ip(DSI_GET_QDI_HNDL(st->priv.dsi_iface_id),
                                                   QMI_IP_FAMILY_PREF_IPV6);
      /* validate the qmi_wds handle */
      if (DSI_INVALID_WDS_HNDL == qmi_wds_hndl && DSI_INVALID_WDS_HNDL == qmi_wds_v6_hndl)
      {
        DSI_LOG_ERROR("invalid WDS handle for iface=%d", st->priv.dsi_iface_id);
        break;
      }
    }
    if (DSI_INVALID_WDS_HNDL == qmi_wds_hndl)
    {
      DSI_LOG_ERROR("invalid WDS handle for iface=%d", st->priv.dsi_iface_id);
      break;
    }

    /* requst packet statistics */
    memset(&stats_response, 0, sizeof(stats_response));

    stats_mask = QMI_WDS_XFER_STATS_TX_PKTS_GOOD |
                 QMI_WDS_XFER_STATS_RX_PKTS_GOOD |
                 QMI_WDS_XFER_STATS_TX_BYTES_OK |
                 QMI_WDS_XFER_STATS_RX_BYTES_OK |
                 QMI_WDS_XFER_STATS_TX_PACKETS_DROPPED |
                 QMI_WDS_XFER_STATS_RX_PACKETS_DROPPED;

    rc = qmi_wds_get_pkt_statistics( qmi_wds_hndl,
                                     stats_mask,
                                     &stats_response,
                                     &qmi_err );
    if(0 > rc)
    {
      DSI_LOG_ERROR( "dsi_get_pkt_stats: get stats err,iface=%d, rc=%d, qmi_err=%d",
                    st->priv.dsi_iface_id,
                    rc,
                    qmi_err);
      break;
    }

    /* QMI message returned successfully.Fill in the statistics if requested for ipv4 or ipv6 */
    if(st->priv.ip_version == DSI_IP_VERSION_4 || st->priv.ip_version == DSI_IP_VERSION_6)
    {
      dsi_data_stats->bytes_rx        = stats_response.rx_good_byte_cnt;
      dsi_data_stats->bytes_tx        = stats_response.tx_good_byte_cnt;
      dsi_data_stats->pkts_dropped_rx = stats_response.rx_pkts_dropped;
      dsi_data_stats->pkts_dropped_tx = stats_response.tx_pkts_dropped;
      dsi_data_stats->pkts_rx         = stats_response.rx_good_pkt_cnt;
      dsi_data_stats->pkts_tx         = stats_response.tx_good_pkt_cnt;
    }
    /* QMI message returned successfully.Fill in the statistics if requested for ipv4v6 */
    if(st->priv.ip_version == DSI_IP_VERSION_4_6)
    {
      rc_v6 = qmi_wds_get_pkt_statistics(qmi_wds_v6_hndl,
                                       stats_mask,
                                       &stats_response_v6,
                                       &qmi_err_v6);
      if(0 > rc_v6)
      {
        DSI_LOG_ERROR( "dsi_get_pkt_stats: get stats err,iface=%d, rc=%d, qmi_err=%d",
                      st->priv.dsi_iface_id,
                      rc_v6,
                      qmi_err_v6);
        break;
      }
    /* IPv4 statistics plus IPv6 statistics for ipv4v6*/
      dsi_data_stats->bytes_rx        = stats_response.rx_good_byte_cnt +
                                        stats_response_v6.rx_good_byte_cnt;
      dsi_data_stats->bytes_tx        = stats_response.tx_good_byte_cnt +
                                        stats_response_v6.tx_good_byte_cnt;
      dsi_data_stats->pkts_dropped_rx = stats_response.rx_pkts_dropped +
                                        stats_response_v6.rx_pkts_dropped;
      dsi_data_stats->pkts_dropped_tx = stats_response.tx_pkts_dropped +
                                        stats_response_v6.tx_pkts_dropped;
      dsi_data_stats->pkts_rx         = stats_response.rx_good_pkt_cnt +
                                        stats_response_v6.rx_good_pkt_cnt;
      dsi_data_stats->pkts_tx         = stats_response.tx_good_pkt_cnt +
                                        stats_response_v6.tx_good_pkt_cnt;
    }
    ret = DSI_SUCCESS;
  }
  while (0);

  DSI_GLOBAL_UNLOCK;

  DSI_LOG_DEBUG("%s","dsi_get_pkt_stats: EXIT");

  return ret;

}/* dsi_get_pkt_stats */

/*===========================================================================
  FUNCTION:  dsi_set_ril_instance
===========================================================================*/
/*!
    @brief
    Sets the RIL instance for the dsi_netctrl library. Used in
    DSDS/DSDA/TSTS type of targets where there can be multiple RIL instances.

    @param
    instance - indicates the instance index. (First instance is 0, second
    instance is 1 etc).

    @dependencies
    Needs to be called prior to dsi_init()

    @return
    DSI_ERROR
    DSI_SUCCESS
*/
/*=========================================================================*/
int dsi_set_ril_instance
(
  int instance
)
{
  DSI_LOG_DEBUG("dsi_set_ril_instance: instance %d", instance);

  dsi_ril_instance = instance;

  return DSI_SUCCESS;

} /* dsi_set_ril_instance() */


/*===========================================================================
  FUNCTION:  dsi_get_ril_instance
===========================================================================*/
/*!
    @brief
    Gets the RIL instance for the dsi_netctrl library. Used in
    DSDS/DSDA/TSTS type of targets where there can be multiple RIL instances.

    @return
    RIL instance that was set through dsi_set_ril_instance
*/
/*=========================================================================*/
int dsi_get_ril_instance
(
  void
)
{
  DSI_LOG_DEBUG("dsi_get_ril_instance: instance %d", dsi_ril_instance);

  return dsi_ril_instance;

} /* dsi_set_ril_instance() */

/*===========================================================================
  FUNCTION:  dsi_set_modem_subs_id
===========================================================================*/
/*!
    @brief
    Sets the modem subscription id for the dsi_netctrl library. Used in
    Multi SIM targets targets for binding the subscription.

    @param
    subs_id - subscription ID the process is intrested in

    @dependencies
    Needs to be called prior to dsi_init()

    @return
    DSI_ERROR
    DSI_SUCCESS
*/
/*=========================================================================*/
int dsi_set_modem_subs_id
(
  int subs_id
)
{
  DSI_LOG_DEBUG("dsi_set_modem_subs_id: subs_id %d", subs_id);

  dsi_modem_subs_id = subs_id;

  return DSI_SUCCESS;

} /* dsi_set_modem_subs_id() */


/*===========================================================================
  FUNCTION:  dsi_get_modem_subs_id
===========================================================================*/
/*!
    @brief
    Gets the modem subscription id for the dsi_netctrl library. Used in
    Multi SIM targets where there can be multiple RIL instances making a calls

    @return
    Subscription ID that was set using dsi_set_modem_subs_id
*/
/*=========================================================================*/
int dsi_get_modem_subs_id
(
  void
)
{
  DSI_LOG_DEBUG("dsi_get_modem_subs_id: subs_id %d", dsi_modem_subs_id);

  return dsi_modem_subs_id;

} /* dsi_get_modem_subs_id() */

/*===========================================================================
  FUNCTION:  dsi_get_qmi_port_name
===========================================================================*/
/*!
    @brief
    Used to retrieve the QMI port associated with the data service handle.

    @return
    DSI_ERROR
    DSI_SUCCESS

    @note
    len should be at least DSI_CALL_INFO_DEVICE_NAME_MAX_LEN+1
    long
*/
/*=========================================================================*/
int dsi_get_qmi_port_name(dsi_hndl_t hndl, char * buf, int len)
{
  int ret = DSI_ERROR;
  dsi_store_t * st = NULL;
  int i = 0;

  do
  {
    ret = DSI_ERROR;

    DSI_LOG_INFO("%s","dsi_get_qmi_port_name: ENTRY");

    st = (dsi_store_t *)hndl;

    if (!DSI_IS_HNDL_VALID(st))
    {
      DSI_LOG_ERROR("%s", "dsi_get_qmi_port_name: received invalid hndl");
      break;
    }

    i = st->priv.dsi_iface_id;

    if (!DSI_IS_ID_VALID(i))
    {
      DSI_LOG_ERROR("%s", "dsi_get_qmi_port_name: received invalid hndl");
      break;
    }

    /* a non-NULL buffer with at least DSI_CALL_INFO_DEVICE_NAME_MAX_LEN
     *  size is required (add 1 for NULL char) */
    if (NULL == buf || len < DSI_CALL_INFO_DEVICE_NAME_MAX_LEN+1)
    {
      DSI_LOG_ERROR("%s", "dsi_get_qmi_port_name: received invalid buf");
      break;
    }

    /* copy rmnetxx value into the buffer */
    DSI_LOG_DEBUG("copying value [%s] at user provided location [%p]",
                  DSI_GET_WDS_STR(i), buf);
    strlcpy(buf, DSI_GET_WDS_STR(i), (size_t)len);

   ret = DSI_SUCCESS;
  } while (0);

  if (ret == DSI_ERROR)
  {
    DSI_LOG_ERROR("%s","dsi_get_qmi_port_name: EXIT with err");
  }
  else
  {
    DSI_LOG_INFO("%s","dsi_get_qmi_port_name: EXIT with suc");
  }

  return ret;
}

/*===========================================================================
  FUNCTION:  dsi_process_screen_state_change
===========================================================================*/
/** @ingroup dsi_process_screen_state_change

    Performs any optimization processing when the screen state is turned off.

    @param[in] screen_state Screen state. 0 - OFF, 1 - ON

    @return
    DSI_SUCCESS -- The API returned success.
    DSI_ERROR -- The API returned failure.

    @dependencies
    dsi_init() must be called.
*/
/*=========================================================================*/
int dsi_process_screen_state_change(int screen_state)
{
  int result = DSI_ERROR;

  DSI_LOG_INFO("Screen state changed: %s", screen_state ? "ON" : "OFF");

  if (screen_state == 0)
  {
    result = netmgr_client_send_user_cmd(NETMGR_USER_CMD_SCREEN_OFF, NULL);
  }
  else if (screen_state == 1)
  {
    result = netmgr_client_send_user_cmd(NETMGR_USER_CMD_SCREEN_ON, NULL);
  }

  return result;
}

/*===========================================================================
  FUNCTION:  dsi_enable_port_forwarding
===========================================================================*/
/*!
    @brief
    This function can be used to enable port forwarding by installing
    the iptable rules appropriately. If no IWLAN calls are active then
    the client preference is saved and rules are installed on bring up
    of the first IWLAN call.

    @param[in] dsi_hndl Handqqle received from dsi_get_data_srvc_hndl().
    @param[in] ip_family - AF_INET/AF_INET6

    @return
    DSI_ERROR
    DSI_SUCCESS

    @dependencies
    There has to be at least one iwlan call up for the specified family
    for the API to take effect.
*/
/*=========================================================================*/
int dsi_enable_port_forwarding
(
   dsi_hndl_t dsi_hndl,
   int        ip_family
)
{
  dsi_store_t *st = NULL;
  netmgr_user_cmd_data_t *cmd_data;
  int ret = DSI_ERROR;

  DSI_LOG_ENTRY;
  st = (dsi_store_t*)dsi_hndl;
  if(!DSI_IS_HNDL_VALID(st))
  {
    DSI_LOG_ERROR("%s: Invalid dsi handle [%d]",__func__, dsi_hndl);
    ret = DSI_ERROR;
    goto bail;
  }

  if(ip_family != AF_INET && ip_family != AF_INET6 )
  {
    DSI_LOG_ERROR("%s: Invalid ip_family value specified[%d]",__func__, ip_family);
    ret = DSI_ERROR;
    goto bail;
  }

  cmd_data = (netmgr_user_cmd_data_t *)malloc(sizeof(netmgr_user_cmd_data_t));
  if(NULL == cmd_data)
  {
    DSI_LOG_ERROR("%s: Insufficient space to allocate cmd_data",__func__);
    ret = DSI_ERROR;
    goto bail;
  }
  memset(cmd_data, 0x0, sizeof(netmgr_user_cmd_data_t));

  /* update txn info in the cmd_data */
  DSI_NETMGR_GET_NEXT_TXN_ID(cmd_data->txn);
  cmd_data->cmd_id = NETMGR_USER_CMD_ENABLE_PORT_FORWARDING;
  cmd_data->data.port_forwarding_data.ip_family = ip_family;

  /* store command data in dsi store handle for validation later */
  memcpy(&st->priv.user_cmd_data, cmd_data, sizeof(netmgr_user_cmd_data_t));

  DSI_LOG_DEBUG("%s: Sending user cmd NETMGR_USER_CMD_ENABLE_PORT_FORWARDING "
                "for pid[%d] using txn_id[%d]",
                __func__, cmd_data->txn.pid, cmd_data->txn.txn_id);
  DSI_INIT_SIGNAL_DATA(&st->priv.signal_data);
  DSI_INIT_SIGNAL_FOR_WAIT(&st->priv.signal_data);
  netmgr_client_send_user_cmd(NETMGR_USER_CMD_ENABLE_PORT_FORWARDING, cmd_data);

  if(DSI_WAIT_FOR_SIGNAL_WITH_TIMEOUT(&st->priv.signal_data, 5000 ) == DSI_ERROR)
  {
    DSI_LOG_ERROR("%s: dsi_enable_port_forwarding: User command processing timedout in netmgr", __func__);
    st->priv.user_cmd_data.txn.txn_status = DSI_ERROR;
    ret = DSI_ERROR;
  }
  else
  {
    ret = DSI_SUCCESS;
  }
  DSI_DESTROY_SIGNAL_DATA(&st->priv.signal_data);

  if(st->priv.user_cmd_data.txn.txn_status != DSI_SUCCESS)
  {
    ret = DSI_ERROR;
  }

bail:
  if(NULL != cmd_data )
  {
    free(cmd_data);
    cmd_data = NULL;
  }

  DSI_LOG_EXIT;
  return ret;
}

/*===========================================================================
  FUNCTION:  dsi_disable_port_forwarding
===========================================================================*/
/*!
    @brief
    This function can be used to disable port forwarding by uninstalling
    the iptable rules appropriately. If no IWLAN calls are active then
    the client preference is saved and rules will not be installed on
    bring up of the first IWLAN call.

    @param[in] dsi_hndl Handle received from dsi_get_data_srvc_hndl().
    @param[in] ip_family - AF_INET/AF_INET6

    @return
    DSI_ERROR
    DSI_SUCCESS
*/
/*=========================================================================*/
extern int dsi_disable_port_forwarding
(
   dsi_hndl_t dsi_hndl,
   int        ip_family
)
{
  dsi_store_t * st = NULL;
  netmgr_user_cmd_data_t *cmd_data = NULL;
  int ret = DSI_ERROR;

  DSI_LOG_ENTRY;
  st = (dsi_store_t*)dsi_hndl;
  if(!DSI_IS_HNDL_VALID(st))
  {
    DSI_LOG_ERROR("%s: Invalid dsi handle [%d]", __func__, dsi_hndl);
    ret = DSI_ERROR;
    goto bail;
  }

  if( AF_INET != ip_family && AF_INET6 != ip_family)
  {
    DSI_LOG_ERROR("%s: Invalid ip_family value specified [%d]",__func__, ip_family);
    ret = DSI_ERROR;
    goto bail;
  }

  cmd_data = (netmgr_user_cmd_data_t *)malloc(sizeof(netmgr_user_cmd_data_t));
  if(NULL == cmd_data)
  {
    DSI_LOG_ERROR("%s: Insufficient space to allocate cmd_data", __func__);
    ret = DSI_ERROR;
    goto bail;
  }
  memset(cmd_data, 0x0, sizeof(netmgr_user_cmd_data_t));

  /* update txn info in the cmd_data */
  DSI_NETMGR_GET_NEXT_TXN_ID(cmd_data->txn);
  cmd_data->cmd_id = NETMGR_USER_CMD_DISABLE_PORT_FORWARDING;
  cmd_data->data.port_forwarding_data.ip_family = ip_family;

  /* store command data in dsi store handle for validation later */
  memcpy(&st->priv.user_cmd_data, cmd_data, sizeof(netmgr_user_cmd_data_t));

  DSI_LOG_DEBUG("%s: Sending user cmd NETMGR_USER_CMD_DISABLE_PORT_FORWARDING "
                "for pid[%d] using txn_id[%d]",
                __func__,cmd_data->txn.pid, cmd_data->txn.txn_id);
  DSI_INIT_SIGNAL_DATA(&st->priv.signal_data);
  DSI_INIT_SIGNAL_FOR_WAIT(&st->priv.signal_data);
  netmgr_client_send_user_cmd(NETMGR_USER_CMD_DISABLE_PORT_FORWARDING, cmd_data);

  if(DSI_WAIT_FOR_SIGNAL_WITH_TIMEOUT(&st->priv.signal_data, 5000 ) == DSI_ERROR)
  {
    DSI_LOG_ERROR("%s: User command processing timedout in netmgr", __func__);
    st->priv.user_cmd_data.txn.txn_status = DSI_ERROR;
    ret = DSI_ERROR;
  }
  else
  {
    ret = DSI_SUCCESS;
  }
  DSI_DESTROY_SIGNAL_DATA(&st->priv.signal_data);

  if(st->priv.user_cmd_data.txn.txn_status != DSI_SUCCESS)
  {
    ret = DSI_ERROR;
  }

bail:
  if(NULL != cmd_data )
  {
    free(cmd_data);
    cmd_data = NULL;
  }

  DSI_LOG_EXIT;
  return ret;
}

/*===========================================================================
  FUNCTION:  dsi_query_port_forwarding
===========================================================================*/
/*!
    @brief
    This function can be used to query the current port forwarding preference
    set by the client.

    @param[in] dsi_hndl Handle received from dsi_get_data_srvc_hndl().
    @param[in] ip_family - AF_INET/AF_INET6
    @param[out] forwarding_status return the current port forwarding status.

    @return
    DSI_ERROR
    DSI_SUCCESS
*/
/*=========================================================================*/
extern int dsi_query_port_forwarding_status
(
   dsi_hndl_t                    dsi_hndl,
   int                           ip_family,
   dsi_port_forwarding_status_t* forwarding_status
)
{
  dsi_store_t * st = NULL;
  netmgr_user_cmd_data_t *cmd_data = NULL;
  int ret = DSI_ERROR;

  DSI_LOG_ENTRY;

  st = (dsi_store_t*)dsi_hndl;
  if(!DSI_IS_HNDL_VALID(st))
  {
    DSI_LOG_ERROR("%s: Invalid dsi handle [%d]", __func__, dsi_hndl);
    ret = DSI_ERROR;
    goto bail;
  }

  if( AF_INET != ip_family && AF_INET6 != ip_family)
  {
    DSI_LOG_ERROR("%s: Invalid ip_family [%d]",__func__, ip_family);
    ret = DSI_ERROR;
    goto bail;
  }

  if(NULL == forwarding_status)
  {
    DSI_LOG_ERROR("%s: invalid input parameter", __func__);
    ret = DSI_ERROR;
    goto bail;
  }
  *forwarding_status = DSI_PORT_FORWARDING_INVALID;

  cmd_data = (netmgr_user_cmd_data_t *)malloc(sizeof(netmgr_user_cmd_data_t));
  if(NULL == cmd_data)
  {
    DSI_LOG_ERROR("%s: Insufficient space to allocate cmd_data", __func__);
    ret = DSI_ERROR;
    goto bail;
  }
  memset(cmd_data, 0x0, sizeof(netmgr_user_cmd_data_t));

  /* update txn info in the cmd_data */
  DSI_NETMGR_GET_NEXT_TXN_ID(cmd_data->txn);
  cmd_data->cmd_id = NETMGR_USER_CMD_QUERY_PORT_FORWARDING;
  cmd_data->data.port_forwarding_data.ip_family = ip_family;

  /* store command data in dsi store handle for validation later */
  memcpy(&st->priv.user_cmd_data, cmd_data, sizeof(netmgr_user_cmd_data_t));

  /* Clear ip_family for query as we need to get this information from netmgr */
  st->priv.user_cmd_data.data.port_forwarding_data.ip_family = 0;

  DSI_LOG_DEBUG("%s: Sending user cmd NETMGR_USER_CMD_QUERY_PORT_FORWARDING "
                "for pid[%d] using txn_id[%d]",
                __func__,cmd_data->txn.pid, cmd_data->txn.txn_id);
  DSI_INIT_SIGNAL_DATA(&st->priv.signal_data);
  DSI_INIT_SIGNAL_FOR_WAIT(&st->priv.signal_data);
  netmgr_client_send_user_cmd(NETMGR_USER_CMD_QUERY_PORT_FORWARDING, cmd_data);

  if(DSI_WAIT_FOR_SIGNAL_WITH_TIMEOUT(&st->priv.signal_data, 5000 ) == DSI_ERROR)
  {
    DSI_LOG_ERROR("%s: User command processing timedout in netmgr", __func__);
    st->priv.user_cmd_data.txn.txn_status = DSI_ERROR;
    ret = DSI_ERROR;
  }
  else
  {
    ret = DSI_SUCCESS;
  }
  DSI_DESTROY_SIGNAL_DATA(&st->priv.signal_data);

  if(st->priv.user_cmd_data.txn.txn_status == DSI_SUCCESS)
  {
    if( ip_family == st->priv.user_cmd_data.data.port_forwarding_data.ip_family )
    {
      *forwarding_status = DSI_PORT_FORWARDING_ENABLED;
      DSI_LOG_DEBUG("%s: forwarding [ENABLED] for ip_family[%s]",
                    __func__, ip_family == AF_INET? "AF_INET":"AF_INET6");
    }
    else
    {
      *forwarding_status = DSI_PORT_FORWARDING_DISABLED;
      DSI_LOG_DEBUG("%s: forwarding [DISABLED] for ip_family[%s]",
                    __func__, ip_family == AF_INET? "AF_INET":"AF_INET6");
    }
  }
  else
  {
    ret = DSI_ERROR;
  }

bail:
  if(NULL != cmd_data )
  {
    free(cmd_data);
    cmd_data = NULL;
  }
  DSI_LOG_EXIT;
  return ret;
}

/*===========================================================================
  FUNCTION:  dsi_get_link_mtu
===========================================================================*/
/*!
    @brief
    Used to get the MTU of the corresponding link

    @return
    DSI_ERROR
    DSI_SUCCESS

*/
/*=========================================================================*/
int dsi_get_link_mtu
(
  dsi_hndl_t    hndl,
  unsigned int  *mtu
)
{
  dsi_store_t * st = NULL;
  int ret = DSI_ERROR;
  int fd = -1;
  struct ifreq if_mtu;
  const char *ifname = NULL;

  DSI_GLOBAL_LOCK;

  DSI_LOG_DEBUG("%s","dsi_get_link_mtu ENTRY");

  ret = DSI_ERROR;
  do
  {
    st = (dsi_store_t *)hndl;

    if (NULL == mtu || !DSI_IS_HNDL_VALID(st))
    {
      DSI_LOG_ERROR("%s","invalid params rcvd");
      break;
    }

    if (!DSI_IS_ID_VALID(st->priv.dsi_iface_id))
    {
      DSI_LOG_ERROR("invalid dsi_iface_id=%d", st->priv.dsi_iface_id);
      break;
    }

    if (DSI_GET_MTU(st->priv.dsi_iface_id) != DSI_INVALID_MTU)
    {
      *mtu = DSI_GET_MTU(st->priv.dsi_iface_id);
    }
    else
    {
      memset(&if_mtu, 0, sizeof(if_mtu));

      ifname = DSI_GET_DEV_STR(st->priv.dsi_iface_id);

      if ( ifname == NULL )
      {
        DSI_LOG_ERROR("unable to find device name for iface=%d", st->priv.dsi_iface_id);
        break;
      }

      DSI_LOG_DEBUG("invalid cache, querying MTU from iface=%s", ifname);

      fd = socket(AF_INET, SOCK_STREAM, 0);
      if ( fd < 0 )
      {
        DSI_LOG_ERROR("%s", "failed to create socket");
        break;
      }

      strlcpy(if_mtu.ifr_name, ifname, IFNAMSIZ);
      if_mtu.ifr_name[IFNAMSIZ - 1] = '\0';

      if ( ioctl(fd, SIOCGIFMTU, &if_mtu) < 0 )
      {
        DSI_LOG_ERROR("%s", "failed to create socket");
        break;
      }

      *mtu = if_mtu.ifr_mtu;
      DSI_SET_MTU(st->priv.dsi_iface_id, if_mtu.ifr_mtu);
    }

    DSI_LOG_DEBUG("returning mtu=%d", *mtu);
    ret = DSI_SUCCESS;
  }
  while (0);

  if (fd != -1)
  {
    close(fd);
  }

  DSI_GLOBAL_UNLOCK;

  DSI_LOG_DEBUG("%s","dsi_get_link_mtu: EXIT");

  return ret;
}
