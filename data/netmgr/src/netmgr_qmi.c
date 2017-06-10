/******************************************************************************

                          N E T M G R _ Q M I . C

******************************************************************************/

/******************************************************************************

  @file    netmgr_qmi.c
  @brief   Network Manager QMI Driver Interface

  DESCRIPTION
  Implementation of NetMgr QMI Driver interface.

******************************************************************************/
/*===========================================================================

  Copyright (c) 2010-2014 Qualcomm Technologies, Inc. All Rights Reserved

  Qualcomm Technologies Proprietary and Confidential.

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
02/11/10   ar         Initial version (derived from DSC file)

******************************************************************************/

/*===========================================================================
                              INCLUDE FILES
===========================================================================*/

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "ds_list.h"
#include "ds_cmdq.h"
#include "ds_trace.h"
#include "qmi_qos_srvc.h"
#include "qmi_platform.h"
#include "netmgr_defs.h"
#include "netmgr_exec.h"
#include "netmgr_platform.h"
#include "netmgr_util.h"
#include "netmgr_tc.h"
#include "netmgr_qmi.h"
#include "netmgr_main.h"
#include "wireless_data_administrative_service_v01.h"

#include "netmgr_qmi_wda.h"
#include "netmgr_rmnet.h"
#include "netmgr_qmi_dfs.h"

#include <linux/rmnet_data.h>

#ifdef FEATURE_DS_LINUX_ANDROID
#include <cutils/properties.h>

#ifndef NETMGR_OFFTARGET
  #include <private/android_filesystem_config.h>
#endif

#endif

#define NETMGR_QMI_MAX_RETRY_COUNT               10
#define NETMGR_QMI_WAIT_TIME_BEFORE_NEXT_RETRY   500000 /* usec */
#define NETMGR_QMI_CLIENT_INVALID (-1)


#define NETMGR_QMI_CLIENT_INVALID            (-1)

#ifdef FEATURE_DATA_IWLAN
  #define NETMGR_IS_REV_IP_TRANS_CONN_ID(cid)                     \
    ((0 == strncmp((cid), "rev_rmnet", strlen("rev_rmnet"))) ||   \
     (0 == strncmp((cid), "r_rmnet_data", strlen("r_rmnet_data"))))

  #define NETMGR_IS_REV_IP_TRANS_LOCAL_BRK_OUT_CONN_ID(cid)         \
   ((0 == strncmp((cid), "rev_rmnet0", strlen("rev_rmnet0"))) ||   \
     (0 == strncmp((cid), "r_rmnet_data0", strlen("r_rmnet_data0"))))

  #define NETMGR_QMI_INVALID_TXN_ID            (0)

  static pthread_mutex_t  txn_list_mutex = PTHREAD_MUTEX_INITIALIZER;

  #define NETMGR_QMI_IS_BEARER_TECH_EX_IWLAN(bt)                  \
    ((QMI_WDS_TECHNOLOGY_3GPP == (bt)->technology) &&             \
     (QMI_WDS_BEARER_TECH_RAT_EX_3GPP_WLAN == (bt)->rat_mask) &&  \
     (QMI_WDS_3GPP_SO_MASK_S2B == (bt)->so_mask))

#endif /* FEATURE_DATA_IWLAN */

#define NETMGR_QMI_QOS_HEADER_6_BYTE         (1<<0)
#define NETMGR_QMI_QOS_HEADER_8_BYTE         (1<<1)
#define NETMGR_QMI_QOS_HEADER_DEFAULT        (0)

LOCAL int
netmgr_qmi_verify_link (int link);

/*qmi message library handle*/
LOCAL int qmi_handle = QMI_INVALID_CLIENT_HANDLE;

/*===========================================================================
                     LOCAL DEFINITIONS AND DECLARATIONS
===========================================================================*/

struct netmgr_qmi_cfg_s netmgr_qmi_cfg;

/*---------------------------------------------------------------------------
   Forward declaration of the function used to process QMI WDS Ind Command
---------------------------------------------------------------------------*/
LOCAL void
netmgr_wds_ind
(
  netmgr_qmi_wds_ind_t  *netmgr_qmi_wds_ind
);

/*---------------------------------------------------------------------------
   Inline accessor for getting the WDS Client ID for a given QMI Link
---------------------------------------------------------------------------*/
LOCAL __inline__ int
netmgr_qmi_wds_get_clnt_id (int link, netmgr_qmi_client_type_t type)
{
  if( (unsigned int)type < (unsigned int)NETMGR_QMI_CLIENT_MAX ) {
    return netmgr_qmi_cfg.links[link].wds_info.clnt_hdl[type];
  } else {
    return NETMGR_QMI_CLIENT_INVALID;
  }
}

/*---------------------------------------------------------------------------
   Inline accessor for getting the QOS Client ID for a given QMI Link
---------------------------------------------------------------------------*/
LOCAL __inline__ int
netmgr_qmi_qos_get_clnt_id (int link, netmgr_qmi_client_type_t type)
{
  if( (unsigned int)type < (unsigned int)NETMGR_QMI_CLIENT_MAX ) {
    return netmgr_qmi_cfg.links[link].qos_info.clnt_hdl[type];
  } else {
    return -1;
  }
}

/*---------------------------------------------------------------------------
   Inline accessor for getting a pointer to the QMI Driver Control Info for
   a given QMI Link
---------------------------------------------------------------------------*/
LOCAL __inline__ netmgr_wds_qmi_drv_info_t *
netmgr_qmi_get_wds_qmi_drv_info (int link)
{
  return &netmgr_qmi_cfg.links[link].wds_info;
}

LOCAL netmgr_qmi_client_type_t
netmgr_qmi_convert_ip_pref_to_client_type(qmi_ip_family_pref_type ip_family)
{
  netmgr_qmi_client_type_t client_type;

  switch( ip_family )
  {
    case QMI_IP_FAMILY_PREF_IPV6:
      client_type = NETMGR_QMI_CLIENT_IPV6;
      break;
    case QMI_IP_FAMILY_PREF_IPV4:
    case QMI_IP_FAMILY_PREF_ANY:
    default:
      client_type = NETMGR_QMI_CLIENT_IPV4;
      break;
  }

  return client_type;
}


/*===========================================================================
  FUNCTION  netmgr_qmi_wds_get_tech_name
===========================================================================*/
/*!
@brief
  Retrives the technology name for Modem network interface

@return
 unsigned int - technology name

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
unsigned short netmgr_qmi_wds_get_tech_name(int link)
{
  return netmgr_qmi_cfg.links[link].wds_info.tech_name;
}


/*===========================================================================
  FUNCTION  netmgr_qmi_map_dev_conn_instance
===========================================================================*/
/*!
@brief
  maps qmi connection instance to that of the qmi framework

@arg *dev_id Name of device to get the connection instance
@arg *conn_id populated with the frameworks connection instance

@return
  int - NETMGR_SUCCESS on successful operation,
        NETMGR_FAILURE if there was an error sending QMI message
*/
/*=========================================================================*/
int netmgr_qmi_map_dev_conn_instance
(
  const char                      *dev_id,
  qmi_client_qmux_instance_type   *conn_id
)
{
  int rc = NETMGR_SUCCESS;
  int mux_id = -1;
  int ep_type = -1;
  int epid = -1;

  if ( !dev_id || !conn_id )
  {
    netmgr_log_err("%s(): Called with null pointers!\n", __func__);
    return NETMGR_FAILURE;
  }

  switch (QMI_PLATFORM_DEV_NAME_TO_CONN_ID_EX(dev_id, &ep_type, &epid, &mux_id))
  {
    case QMI_CONN_ID_RMNET_0:
      *conn_id = QMI_CLIENT_QMUX_RMNET_INSTANCE_0;
      break;
    case QMI_CONN_ID_RMNET_1:
      *conn_id = QMI_CLIENT_QMUX_RMNET_INSTANCE_1;
      break;
    case QMI_CONN_ID_RMNET_2:
      *conn_id = QMI_CLIENT_QMUX_RMNET_INSTANCE_2;
      break;
    case QMI_CONN_ID_RMNET_3:
      *conn_id = QMI_CLIENT_QMUX_RMNET_INSTANCE_3;
      break;
    case QMI_CONN_ID_RMNET_4:
      *conn_id = QMI_CLIENT_QMUX_RMNET_INSTANCE_4;
      break;
    case QMI_CONN_ID_RMNET_5:
      *conn_id = QMI_CLIENT_QMUX_RMNET_INSTANCE_5;
      break;
    case QMI_CONN_ID_RMNET_6:
      *conn_id = QMI_CLIENT_QMUX_RMNET_INSTANCE_6;
      break;
    case QMI_CONN_ID_RMNET_7:
      *conn_id = QMI_CLIENT_QMUX_RMNET_INSTANCE_7;
      break;

    case QMI_CONN_ID_RMNET_MDM_0:
      *conn_id = QMI_CLIENT_QMUX_RMNET_USB_INSTANCE_0;
      break;
    case QMI_CONN_ID_RMNET_MDM_1:
      *conn_id = QMI_CLIENT_QMUX_RMNET_USB_INSTANCE_1;
      break;
    case QMI_CONN_ID_RMNET_MDM_2:
      *conn_id = QMI_CLIENT_QMUX_RMNET_USB_INSTANCE_2;
      break;
    case QMI_CONN_ID_RMNET_MDM_3:
      *conn_id = QMI_CLIENT_QMUX_RMNET_USB_INSTANCE_3;
      break;
    case QMI_CONN_ID_RMNET_MDM_4:
      *conn_id = QMI_CLIENT_QMUX_RMNET_USB_INSTANCE_4;
      break;
    case QMI_CONN_ID_RMNET_MDM_5:
      *conn_id = QMI_CLIENT_QMUX_RMNET_USB_INSTANCE_5;
      break;
    case QMI_CONN_ID_RMNET_MDM_6:
      *conn_id = QMI_CLIENT_QMUX_RMNET_USB_INSTANCE_6;
      break;
    case QMI_CONN_ID_RMNET_MDM_7:
      *conn_id = QMI_CLIENT_QMUX_RMNET_USB_INSTANCE_7;
      break;

    case QMI_CONN_ID_RMNET_MHI_0:
      *conn_id = QMI_CLIENT_QMUX_RMNET_MHI_INSTANCE_0;
      break;
    case QMI_CONN_ID_RMNET_MHI_1:
      *conn_id = QMI_CLIENT_QMUX_RMNET_MHI_INSTANCE_1;
      break;

    default:
      netmgr_log_err("Invalid connection ID %s passed no match received", dev_id);
      rc = NETMGR_FAILURE;
    break;
  }

  return rc;
}

/*===========================================================================
                            LOCAL FUNCTION DEFINITIONS
===========================================================================*/

#ifdef FEATURE_DATA_IWLAN
/*===========================================================================
  FUNCTION  netmgr_qmi_wds_util_enq_txn
===========================================================================*/
/*!
@brief
  Enqueues the reverse IP txn for the given IP client type and link

@return
  int - NETMGR_SUCCESS on operation success, NETMGR_FAILURE otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
LOCAL int netmgr_qmi_wds_util_enq_txn
(
  netmgr_qmi_client_type_t  clnt,
  int                       link,
  qmi_wds_txn_id_type       txn_id
)
{
  netmgr_util_circ_list_type  *txn_list = NULL;
  qmi_wds_txn_id_type         *txn_ptr = NULL;
  int rc = NETMGR_FAILURE;

  NETMGR_LOG_FUNC_ENTRY;

  NETMGR_LOCK_MUTEX(txn_list_mutex);

  if (clnt >= NETMGR_QMI_CLIENT_MAX)
  {
    netmgr_log_err("invalid clnt=%d\n", clnt);
    goto bail;
  }

  if (NETMGR_SUCCESS != netmgr_qmi_verify_link(link))
  {
    netmgr_log_err("invalid link=%d\n", link);
    goto bail;
  }

  txn_ptr = ds_malloc(sizeof(qmi_wds_txn_id_type));

  if (!txn_ptr)
  {
    netmgr_log_err("failed to alloc memory for txn_id=%ld clnt=%d link=%d\n",
                   txn_id, clnt, link);
    goto bail;
  }

  *txn_ptr = txn_id;

  txn_list = &netmgr_qmi_cfg.links[link].wds_info.rev_ip_txns[clnt];

  if (NETMGR_SUCCESS != netmgr_util_enq_circ_list(txn_list, (void *)txn_ptr))
  {
    netmgr_log_err("failed to enqueue txn_id=%ld, clnt=%d link=%d\n",
                   txn_id, clnt, link);
    ds_free(txn_ptr);
    goto bail;
  }

  rc = NETMGR_SUCCESS;

bail:
  NETMGR_UNLOCK_MUTEX(txn_list_mutex);
  NETMGR_LOG_FUNC_EXIT;
  return rc;
}


/*===========================================================================
  FUNCTION  netmgr_qmi_wds_util_deq_txn
===========================================================================*/
/*!
@brief
  Dequeues and returns the next reverse IP txn for the given IP client type and
  link

@return
  int - NETMGR_SUCCESS on operation success, NETMGR_FAILURE otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
LOCAL int netmgr_qmi_wds_util_deq_txn
(
  netmgr_qmi_client_type_t  clnt,
  int                       link,
  qmi_wds_txn_id_type       *txn_id
)
{
  netmgr_util_circ_list_type  *txn_list = NULL;
  qmi_wds_txn_id_type  *txn_ptr = NULL;
  int rc = NETMGR_FAILURE;

  NETMGR_LOG_FUNC_ENTRY;

  NETMGR_LOCK_MUTEX(txn_list_mutex);

  if (!txn_id)
  {
    netmgr_log_err("invalid input\n");
    goto bail;
  }

  if (clnt >= NETMGR_QMI_CLIENT_MAX)
  {
    netmgr_log_err("invalid clnt=%d\n", clnt);
    goto bail;
  }

  if (NETMGR_SUCCESS != netmgr_qmi_verify_link(link))
  {
    netmgr_log_err("invalid link=%d\n", link);
    goto bail;
  }

  txn_list = &netmgr_qmi_cfg.links[link].wds_info.rev_ip_txns[clnt];

  if (NETMGR_SUCCESS != netmgr_util_deq_circ_list(txn_list, (void **)&txn_ptr))
  {
    netmgr_log_err("txn dequeue failed clnt=%d, link=%d\n", clnt, link);
    goto bail;
  }

  *txn_id = *txn_ptr;

  ds_free(txn_ptr);

  rc = NETMGR_SUCCESS;

bail:
  NETMGR_UNLOCK_MUTEX(txn_list_mutex);
  NETMGR_LOG_FUNC_EXIT;
  return rc;
}
#endif /* FEATURE_DATA_IWLAN */

/*===========================================================================
  FUNCTION  netmgr_qmi_reset_link_info
===========================================================================*/
/*!
@brief
 Helper function to reset state information for a given QMI link.

@return
  void

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
LOCAL void
netmgr_qmi_reset_link_info (int link)
{
  /* Reset interface info */
  netmgr_qmi_cfg.links[link].wds_info.clnt_hdl[NETMGR_QMI_CLIENT_IPV4] = NETMGR_QMI_CLIENT_INVALID;
  netmgr_qmi_cfg.links[link].wds_info.clnt_hdl[NETMGR_QMI_CLIENT_IPV6] = NETMGR_QMI_CLIENT_INVALID;
  netmgr_qmi_cfg.links[link].qos_info.clnt_hdl[NETMGR_QMI_CLIENT_IPV4] = NETMGR_QMI_CLIENT_INVALID;
  netmgr_qmi_cfg.links[link].qos_info.clnt_hdl[NETMGR_QMI_CLIENT_IPV6] = NETMGR_QMI_CLIENT_INVALID;
  netmgr_qmi_cfg.links[link].wds_info.mtu = NETMGR_MTU_INVALID;

  memset( &netmgr_qmi_cfg.links[link].wds_info.addr_info, 0x0,
          sizeof(netmgr_qmi_cfg.links[link].wds_info.addr_info) );

#ifdef FEATURE_DATA_IWLAN
  netmgr_qmi_cfg.links[link].dfs_info.clnt_hdl[NETMGR_QMI_CLIENT_IPV4] = NULL;
  netmgr_qmi_cfg.links[link].dfs_info.clnt_hdl[NETMGR_QMI_CLIENT_IPV6] = NULL;
  netmgr_qmi_cfg.links[link].assoc_link = NETMGR_LINK_MAX;

  netmgr_util_circ_list_destroy(&netmgr_qmi_cfg.links[link].wds_info.rev_ip_txns[NETMGR_QMI_CLIENT_IPV4]);
  netmgr_util_circ_list_destroy(&netmgr_qmi_cfg.links[link].wds_info.rev_ip_txns[NETMGR_QMI_CLIENT_IPV6]);
#endif /* FEATURE_DATA_IWLAN */

  netmgr_qmi_cfg.links[link].wds_info.first_conn_clnt = NETMGR_QMI_CLIENT_MAX;
  return;
}

/*===========================================================================
  FUNCTION  netmgr_qmi_driver_cleanup
===========================================================================*/
/*!
@brief
  Performs cleanup of QMI client connections.

@return
  void

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
LOCAL void
netmgr_qmi_driver_cleanup
(
  int link
)
{
  int qmi_err;
  netmgr_link_info_t * link_info;
  int clnt;


  if (qmi_handle < 0)
  {
    netmgr_log_err("QMI message library was never initialized. "
                   "invalid qmi handle. \n");
    return;
  }

  if( !netmgr_qmi_cfg.link_array[link].enabled )
  {
    netmgr_log_err("%s(): QMI link=%d is disabled\n",
                   __func__,link);
    return;
  }

  if( FALSE == netmgr_qmi_cfg.link_array[link].initialized )
  {
    netmgr_log_err("%s(): QMI link=%d is not initialized\n",
                   __func__, link);
    return;
  }

  link_info = &netmgr_qmi_cfg.links[link];

  if (NETMGR_QMI_CLIENT_INVALID != link_info->wds_info.clnt_hdl[NETMGR_QMI_CLIENT_IPV4])
  {
    netmgr_log_high("Releasing the V4 WDS qmi_client_handle 0x%08x \n",
                    link_info->wds_info.clnt_hdl[NETMGR_QMI_CLIENT_IPV4]);
    qmi_wds_srvc_release_client( link_info->wds_info.clnt_hdl[NETMGR_QMI_CLIENT_IPV4], &qmi_err );
  }
  else
  {
    netmgr_log_high("Invalid V4 WDS qmi_client_handle 0x%08x \n",
                    link_info->wds_info.clnt_hdl[NETMGR_QMI_CLIENT_IPV4]);
  }

  if (NETMGR_QMI_CLIENT_INVALID != link_info->wds_info.clnt_hdl[NETMGR_QMI_CLIENT_IPV6])
  {
    netmgr_log_high("Releasing the V6 WDS qmi_client_handle 0x%08x \n",
                    link_info->wds_info.clnt_hdl[NETMGR_QMI_CLIENT_IPV6]);
    qmi_wds_srvc_release_client( link_info->wds_info.clnt_hdl[NETMGR_QMI_CLIENT_IPV6], &qmi_err );
  }
  else
  {
    netmgr_log_high("Invalid V6 WDS qmi_client_handle 0x%08x \n",
                    link_info->wds_info.clnt_hdl[NETMGR_QMI_CLIENT_IPV6]);
  }

#ifdef NETMGR_QOS_ENABLED
  if (NETMGR_QMI_CLIENT_INVALID != link_info->qos_info.clnt_hdl[NETMGR_QMI_CLIENT_IPV4])
  {
    qmi_qos_srvc_release_client( link_info->qos_info.clnt_hdl[NETMGR_QMI_CLIENT_IPV4], &qmi_err );
    netmgr_log_high("Releasing the QOS qmi_client_handle 0x%08x \n",
                    link_info->qos_info.clnt_hdl[NETMGR_QMI_CLIENT_IPV4]);
  }
  else
  {
    netmgr_log_high("Invalid QOS qmi_client_handle 0x%08x \n",
                    link_info->qos_info.clnt_hdl[NETMGR_QMI_CLIENT_IPV4]);
  }

  if (NETMGR_QMI_CLIENT_INVALID != link_info->qos_info.clnt_hdl[NETMGR_QMI_CLIENT_IPV6])
  {
    qmi_qos_srvc_release_client( link_info->qos_info.clnt_hdl[NETMGR_QMI_CLIENT_IPV6], &qmi_err );
    netmgr_log_high("Releasing the QOS qmi_client_handle 0x%08x \n",
                    link_info->qos_info.clnt_hdl[NETMGR_QMI_CLIENT_IPV6]);
  }
  else
  {
    netmgr_log_high("Invalid QOS qmi_client_handle 0x%08x \n",
                    link_info->qos_info.clnt_hdl[NETMGR_QMI_CLIENT_IPV6]);
  }
#endif /* NETMGR_QOS_ENABLED */

#ifdef FEATURE_DATA_IWLAN
  if (NULL != link_info->dfs_info.clnt_hdl[NETMGR_QMI_CLIENT_IPV4])
  {
    netmgr_log_high("Releasing the IPv4 DFS qmi_client_handle %p\n",
                    link_info->dfs_info.clnt_hdl[NETMGR_QMI_CLIENT_IPV4]);
    qmi_client_release( link_info->dfs_info.clnt_hdl[NETMGR_QMI_CLIENT_IPV4] );
  }

  if (NULL != link_info->dfs_info.clnt_hdl[NETMGR_QMI_CLIENT_IPV6])
  {
    netmgr_log_high("Releasing the IPv6 DFS qmi_client_handle %p\n",
                    link_info->dfs_info.clnt_hdl[NETMGR_QMI_CLIENT_IPV6]);
    qmi_client_release( link_info->dfs_info.clnt_hdl[NETMGR_QMI_CLIENT_IPV6] );
  }
#endif /* FEATURE_DATA_IWLAN */

  /* reset link info to prevent accidental usage */
  netmgr_qmi_reset_link_info(link);

  /* Mark the link as uninitialized */
  netmgr_log_high("%s(): Marking link=%d as uninitialized\n",
                  __func__, link);
  netmgr_qmi_cfg.link_array[link].initialized = FALSE;
}

/*===========================================================================
  FUNCTION  netmgr_qmi_cleanup
===========================================================================*/
/*!
@brief
  Disconnects completely from QMI.

@return
  void

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
LOCAL void
netmgr_qmi_cleanup
(
  void
)
{
  int i=0;
  /* Release QMI connections */
  /* Release client handles */
  for( i=0; i < NETMGR_MAX_LINK; i++ ) {
    netmgr_qmi_driver_cleanup(i);
  }

  /* Release QMI library connection */
  qmi_release(qmi_handle);
}


/*===========================================================================
  FUNCTION  netmgr_qmi_verify_link
===========================================================================*/
/*!
@brief
 Helper function to verify that a given QMI link is valid.

@return
  int - NETMGR_SUCCESS if the link is valid, NETMGR_FAILURE otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
LOCAL int
netmgr_qmi_verify_link (int link)
{
  /* Range check */
  if ((link < 0) || (link >= NETMGR_MAX_LINK)) {
    return NETMGR_FAILURE;
  }

  if( !netmgr_qmi_cfg.link_array[link].enabled ) {
    return NETMGR_FAILURE;
  }

  return NETMGR_SUCCESS;
}

/*===========================================================================
  FUNCTION  netmgr_qmi_get_dev_str_for_link
===========================================================================*/
/*!
@brief
  Helper function to return the device string associated for the link.

@return
  Device name associated with the link (rmnet_data0 etc)
*/
/*=========================================================================*/
LOCAL char*
netmgr_qmi_get_dev_str_for_link (int link)
{
  static char dev_name[NETMGR_IF_NAME_MAX_LEN];

  /* Verify that link id is valid */
  if (netmgr_qmi_verify_link(link) == NETMGR_SUCCESS &&
      (unsigned int)link >= netmgr_main_cfg.def_link)
  {
    snprintf(dev_name, NETMGR_IF_NAME_MAX_LEN, "%s%d",
             NETMGR_MAIN_RMNET_DATA_PREFIX, (unsigned int)link - netmgr_main_cfg.def_link);
    return dev_name;
  }
  return NULL;
}

/*===========================================================================
  FUNCTION  netmgr_qmi_get_conn_id_for_link
===========================================================================*/
/*!
@brief
 Helper function to return the QMI connection ID (port number) for a given
 QMI link.

@return
  qmi_connection_id_type - QMI Connection ID for the link

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
LOCAL const char*
netmgr_qmi_get_conn_id_for_link (int link)
{
  /* Verify that link id is valid */
  if (netmgr_qmi_verify_link(link) == NETMGR_SUCCESS) {
    /* Return qmi connection id for the link */
    return netmgr_qmi_cfg.link_array[link].qmi_conn_id;
  }
  return NULL;
}

/*===========================================================================
  FUNCTION  netmgr_qmi_link_init
===========================================================================*/
/*!
@brief
 Initializes link state information.

@return
  void

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
LOCAL void
netmgr_qmi_link_init( void )
{
  int i;

  NETMGR_LOG_FUNC_ENTRY;

  /* Iterate over the array of interfaces, initializing each one */
  for (i = 0; i < NETMGR_MAX_LINK; ++i) {
    /* Check for link enabled status */
    if( netmgr_qmi_cfg.link_array[i].enabled )
    {
      /* Reset interface info */
      /* Following interface is unreserved, hence can be used for
         subsequent data calls */
      netmgr_qmi_reset_link_info(i);
    }
  }

  NETMGR_LOG_FUNC_EXIT;
  return;
}

/*===========================================================================
  FUNCTION:  netmgr_qmi_sys_cb
===========================================================================*/
/*!
    @brief
    Processes an incoming QMI system indication. Posts the indication
    for further processing to the executive thread.

    @return
    none
*/
/*=========================================================================*/
void
netmgr_qmi_sys_cb
(
  qmi_sys_event_type event_id,
  const qmi_sys_event_info_type * event_info,
  void * user_data
)
{
  const char *dev_id;
  netmgr_exec_cmd_t * cmd = NULL;
  netmgr_sm_events_t sm_ev = NETMGR_INVALID_EV;
  int i = 0;
  ds_target_t target = ds_get_target();

  if( NULL == event_info)
  {
    netmgr_log_err("netmgr_qmi_sys_cb: Ignoring event_id[%d] with NULL event_info", event_id);
    return;
  }
  if(event_id != QMI_SYS_EVENT_MODEM_IN_SERVICE_IND &&
     event_id != QMI_SYS_EVENT_MODEM_OUT_OF_SERVICE_IND)
  {
    netmgr_log_err("netmgr_qmi_sys_cb: Ignoring system event_id[%d]", (int)event_id);
    return;
  }

  dev_id = event_info->qmi_modem_service_ind.dev_id;
  /* Currently LE targets don't have SSR enabled, ignore these events */
  if (DS_TARGET_LE_MDM9X35 == target
      || DS_TARGET_LE_MDM9X25 == target
      || DS_TARGET_LE_MDM9X15 == target
      || DS_TARGET_LE_LEGACY == target)
  {
    netmgr_log_med("%s(): Ignoring SSR event phys port: %s", __func__, dev_id);
    return;
  }

  /* We need to check if the incoming indication is
   * on the correct control channel */
  if (TRUE == netmgr_main_cfg.single_qmux_ch_enabled)
  {
    if (strcmp(netmgr_main_cfg.qmux_ch_name, dev_id))
    {
      /* For targets with single QMUX control channel flag enabled
       * we need to compare with only one control channel. If the
       * names do not match then we ignore the SSR indication */
      netmgr_log_med("%s(): Ignoring SSR event on phys port: %s", __func__, dev_id);
      return;
    }
    netmgr_log_low("%s(): Processing SSR event phys port: %s", __func__, dev_id);

    /* If the last event indication is the same as the newly received indication
       we don't process the event. As it has been processed for all ifaces already */
    if(netmgr_main_cfg.modem_ssr_state == (int)event_id )
    {
      netmgr_log_low("netmgr_qmi_sys_cb: event_id[%d] already processed, skipping", (int) event_id);
      return;
    }
    /* Received a different event indication. Store new event indication
       to prevent duplicate event handling. */
    netmgr_main_cfg.modem_ssr_state = (int)event_id;

    /* Post siys_ind cmd to do rmnet global clean up */
    /* Posting QMI Indication command on the first active link */
    for( i=0; i<NETMGR_MAX_LINK; ++i)
    {
      if( TRUE == netmgr_qmi_cfg.link_array[i].enabled)
      {
        break;
      }
    }
    if( i == NETMGR_MAX_LINK)
    {
      netmgr_log_err("Cannot find any active link");
      return;
    }

    /* allocate a command object */
    cmd = netmgr_exec_get_cmd();
    NETMGR_ASSERT(cmd);
    /* set command object parameters */
    cmd->data.type = NETMGR_QMI_MSG_CMD;
    cmd->data.link = i;
    cmd->data.info.qmi_msg.type = NETMGR_QMI_SYS_IND_CMD;
    cmd->data.info.qmi_msg.data.sys_ind.event_id = event_id;
    /* there are no embedded pointers inside event_info structure, so
     * memcpy should be enough to copy everything */
    memcpy(&(cmd->data.info.qmi_msg.data.sys_ind.event_info),
           event_info,
           sizeof(cmd->data.info.qmi_msg.data.sys_ind.event_info));
    cmd->data.info.qmi_msg.data.sys_ind.user_data = user_data;

    /* Post command for processing in the command thread context */
    if( NETMGR_SUCCESS != netmgr_exec_put_cmd( cmd ) ) {
        netmgr_log_err("netmgr_qmi_sys_ind: failed to put commmand\n");
        netmgr_exec_release_cmd( cmd );
    }
    /* End - global cleanup command posting */

    /* Search for active links and post sm cmd for each link */
    switch(event_id)
    {
      case QMI_SYS_EVENT_MODEM_OUT_OF_SERVICE_IND:
        netmgr_log_high("rcvd QMI_SYS_EVENT_MODEM_OUT_OF_SERVICE_IND\n");
        sm_ev = NETMGR_MODEM_OOS_EV;
        break;
      case QMI_SYS_EVENT_MODEM_IN_SERVICE_IND:
        netmgr_log_high("rcvd QMI_SYS_EVENT_MODEM_IN_SERVICE_IND\n");
        sm_ev = NETMGR_MODEM_IS_EV;
        break;
      default:
        netmgr_log_low("ignore qmi sys ind [%d]\n", event_id);
        return;
    }

    for(i=0; i<NETMGR_MAX_LINK; i++)
    {
      if (FALSE == netmgr_qmi_cfg.link_array[i].enabled)
      {
        /* Link not enabled don't post cmd */
        continue;
      }
      netmgr_log_high("netmgr_qmi_sys_cb: link[%d] active. Posting cmd to netmgr_exec\n",i);
      /* allocate a command object */
      cmd = netmgr_exec_get_cmd();
      NETMGR_ASSERT(cmd);

      /* set command object parameters */
      cmd->data.type = sm_ev;
      cmd->data.link = i;

      cmd->data.info.qmi_msg.type = NETMGR_QMI_SYS_IND_CMD;
      cmd->data.info.qmi_msg.data.sys_ind.event_id = event_id;
      /* there are no embedded pointers inside event_info structure, so
       * memcpy should be enough to copy everything */
      memcpy(&(cmd->data.info.qmi_msg.data.sys_ind.event_info),
             event_info,
             sizeof(cmd->data.info.qmi_msg.data.sys_ind.event_info));
      cmd->data.info.qmi_msg.data.sys_ind.user_data = user_data;

      /* Post command for processing in the command thread context */
      if( NETMGR_SUCCESS != netmgr_exec_put_cmd( cmd ) )
      {
        netmgr_log_err("failed to put commmand\n");
        netmgr_exec_release_cmd( cmd );
      }
    }
  }
  else /* Not a single qmux ctl channel target */
  {
    /* For other targets we need to compare the incoming device id with
     * all active QMI channels. If there the incoming device id does not
     * match any of the active qmi links then we will ignore the indication */
    for(i=0; i<NETMGR_MAX_LINK; i++)
    {
      if( TRUE == netmgr_qmi_cfg.link_array[i].enabled
          && !strcmp(netmgr_qmi_cfg.link_array[i].qmi_conn_id, dev_id))
      {
        break;
      }
    }

    /* We searched all the netmgr qmi conn_ids and could not
       * match it with the incoming device id, ignore the indication */
    if (NETMGR_MAX_LINK == i)
    {
      netmgr_log_med("%s(): Ignoring SSR event on phys port: %s", __func__, dev_id);
      return;
    }

    switch(event_id)
    {
      case QMI_SYS_EVENT_MODEM_OUT_OF_SERVICE_IND:
        netmgr_log_high("rcvd QMI_SYS_EVENT_MODEM_OUT_OF_SERVICE_IND\n");
        sm_ev = NETMGR_MODEM_OOS_EV;
        break;
      case QMI_SYS_EVENT_MODEM_IN_SERVICE_IND:
        netmgr_log_high("rcvd QMI_SYS_EVENT_MODEM_IS_SERVICE_IND\n");
        sm_ev = NETMGR_MODEM_IS_EV;
        break;
      default:
        netmgr_log_low("ignore qmi sys ind [%d]\n", event_id);
        break;
    }

    if(NETMGR_INVALID_EV == sm_ev)
    {
      return;
    }
    /* allocate a command object */
    cmd = netmgr_exec_get_cmd();
    NETMGR_ASSERT(cmd);

    /* set command object parameters */
    cmd->data.type = sm_ev;
    cmd->data.link = i;
    cmd->data.info.qmi_msg.type = NETMGR_QMI_SYS_IND_CMD;
    cmd->data.info.qmi_msg.data.sys_ind.event_id = event_id;
    /* there are no embedded pointers inside event_info structure, so
     * memcpy should be enough to copy everything */
    if (NULL != event_info)
    {
        memcpy(&(cmd->data.info.qmi_msg.data.sys_ind.event_info),
               event_info,
               sizeof(cmd->data.info.qmi_msg.data.sys_ind.event_info));
    }
    cmd->data.info.qmi_msg.data.sys_ind.user_data = user_data;

    /* Post command for processing in the command thread context */
    if( NETMGR_SUCCESS != netmgr_exec_put_cmd( cmd ) ) {
        netmgr_log_err("failed to put commmand\n");
        netmgr_exec_release_cmd( cmd );
    }
  }
  return;
}

/*===========================================================================
  FUNCTION  netmgr_qmi_wds_ind
===========================================================================*/
/*!
@brief
 Processes an incoming QMI WDS Indication. It posts a command to do the
 required processing in the Command Thread context.

@return
  void

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
LOCAL void
netmgr_qmi_wds_ind
(
  int                            user_handle,
  qmi_service_id_type            service_id,
  void                         * user_data,
  qmi_wds_indication_id_type     ind_id,
  qmi_wds_indication_data_type * ind_data
)
{
  int link;
  netmgr_exec_cmd_t * cmd = NULL;
  (void)user_handle;

  /* Verify service id before proceeding */
  if( QMI_WDS_SERVICE != service_id ) {
    netmgr_log_err("Received non-WDS indication, ignoring\n");
    return;
  }

  /* Get link id from user data ptr */
  link = (int)(uintptr_t)user_data;

  /* Verify link id */
  NETMGR_ASSERT(netmgr_qmi_verify_link(link) == NETMGR_SUCCESS);

  /* Allocate a command object */
  cmd = netmgr_exec_get_cmd();
  NETMGR_ASSERT(cmd);

  /* Set command object parameters */
  cmd->data.type                                = NETMGR_QMI_MSG_CMD;
  cmd->data.link                                = link;

  cmd->data.info.qmi_msg.type                   = NETMGR_QMI_WDS_IND_CMD;
  cmd->data.info.qmi_msg.data.wds_ind.link      = link;
  cmd->data.info.qmi_msg.data.wds_ind.user_hndl = user_handle;
  cmd->data.info.qmi_msg.data.wds_ind.ind_id    = ind_id;
  cmd->data.info.qmi_msg.data.wds_ind.ind_data  = *ind_data;

  /* Post command for processing in the command thread context */
  if( NETMGR_SUCCESS != netmgr_exec_put_cmd( cmd ) ) {
     NETMGR_ABORT("netmgr_qmi_wds_ind: failed to put commmand\n");
     netmgr_exec_release_cmd(cmd);
  }

  return;
}

/*===========================================================================
  FUNCTION  netmgr_qmi_qos_ind
===========================================================================*/
/*!
@brief
 Processes an incoming QMI QOS Indication. It posts a command to do the
 required processing in the Command Thread context.

@return
  void

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
LOCAL void
netmgr_qmi_qos_ind
(
  int                            user_handle,
  qmi_service_id_type            service_id,
  void                         * user_data,
  qmi_qos_indication_id_type     ind_id,
  qmi_qos_indication_data_type * ind_data
)
{
  int link;
  netmgr_exec_cmd_t * cmd;
  (void)user_handle;

  NETMGR_LOG_FUNC_ENTRY;

  /* Verify service id before proceeding */
  if( QMI_QOS_SERVICE != service_id ) {
    netmgr_log_err("Received non-QOS indication, ignoring\n");
    return;
  }
  NETMGR_ASSERT(ind_data != NULL);

  /* Get link id from user data ptr */
  link = (int)(uintptr_t)user_data;

  /* Verify link id */
  NETMGR_ASSERT(netmgr_qmi_verify_link(link) == NETMGR_SUCCESS);

  /* Allocate a command object */
  cmd = netmgr_exec_get_cmd();
  NETMGR_ASSERT(cmd);

  switch( ind_id ) {
    case  QMI_QOS_SRVC_EVENT_REPORT_IND_MSG:
      /* Set command object parameters */
      cmd->data.type                     = NETMGR_QMI_MSG_CMD;
      cmd->data.link                     = (int)(uintptr_t)user_data;
      cmd->data.info.qmi_msg.type        = NETMGR_QMI_QOS_EVENT_REPORT_IND_CMD;
      cmd->data.info.qmi_msg.data.qos_ind.link   = link;
      cmd->data.info.qmi_msg.data.qos_ind.ind_id = ind_id;
      cmd->data.info.qmi_msg.data.qos_ind.info   = *ind_data;

      if (ind_data->event_report.flow_info[0].qos_flow_state.report_flow_state_chng ==
          QMI_QOS_FLOW_ENABLED) {
        DS_MARK_TRACE_PARAMS("netmgr QOS IND Flow Enabled received for link %d\n", link);
      } else if (ind_data->event_report.flow_info[0].qos_flow_state.report_flow_state_chng ==
                 QMI_QOS_FLOW_DISABLED) {
        DS_MARK_TRACE_PARAMS("netmgr QOS IND Flow Disabled received for link %d\n", link);
      }
      break;

    default:
      netmgr_log_err("received unsupported indication type %d\n", ind_id);
      netmgr_exec_release_cmd( cmd );
      return;
  }

  /* Post command for processing in the command thread context */
  if( NETMGR_SUCCESS != netmgr_exec_put_cmd( cmd ) ) {
    NETMGR_ABORT("netmgr_qmi_qos_ind: failed to put commmand\n");
    netmgr_exec_release_cmd(cmd);
  }

  NETMGR_LOG_FUNC_EXIT;
  return;
}


/*===========================================================================
  FUNCTION  netmgr_qmi_get_modem_link_info
===========================================================================*/
/*!
@brief
  Function to query Modem WDS information for specific link.

@return
  int - NETMGR_SUCCESS on successful operation, NETMGR_FAILURE otherwise

@note

  - Dependencies
    - netmgr_qmi_init() must have been called

  - Side Effects
    - None
*/
/*=========================================================================*/
int netmgr_qmi_get_modem_link_info
(
  uint8                        link,
  qmi_ip_family_pref_type      evt_ip_family
)
{
  qmi_client_handle_type client_hndl;
  qmi_wds_req_runtime_settings_params_type req_mask;
  qmi_wds_profile_id_type            profile_id;
  qmi_wds_profile_params_type        profile_params;
  qmi_wds_curr_call_info_type        call_info;
  netmgr_address_set_t             * addr_info_ptr = NULL;
  int err_code;
  int result;
  netmgr_qmi_client_type_t clnt;

  NETMGR_LOG_FUNC_ENTRY;

  clnt = netmgr_qmi_convert_ip_pref_to_client_type(evt_ip_family);
  client_hndl = netmgr_qmi_cfg.links[link].wds_info.clnt_hdl[clnt];

  req_mask =
    (
      QMI_WDS_GET_CURR_CALL_INFO_IP_FAMILY_PARAM_MASK         |
      QMI_WDS_GET_CURR_CALL_INFO_IP_ADDRESS_PARAM_MASK        |
      QMI_WDS_GET_CURR_CALL_INFO_GATEWAY_INFO_PARAM_MASK      |
      QMI_WDS_GET_CURR_CALL_INFO_DNS_ADDR_PARAM_MASK          |
      QMI_WDS_GET_CURR_CALL_INFO_MTU_PARAM_MASK
    );

  /* Query Modem for requested parameters */
  result = qmi_wds_get_curr_call_info( client_hndl,
                                       req_mask,
                                       &profile_id,
                                       &profile_params,
                                       &call_info,
                                       &err_code);

  if( QMI_NO_ERR != result ) {
    netmgr_log_err("failed on qmi_wds_get_curr_call_info: "
                   "ret=%d err=%d\n",
                   result, err_code);
    return NETMGR_FAILURE;
  }

  if( QMI_IP_FAMILY_PREF_IPV4 == evt_ip_family )
  {
    addr_info_ptr = &netmgr_qmi_cfg.links[ link ].wds_info.addr_info.ipv4;
    netmgr_qmi_cfg.links[ link ].wds_info.addr_info.valid_mask |= NETMGR_ADDRSET_MASK_IPV4;
  }
  else if(QMI_IP_FAMILY_PREF_IPV6 == evt_ip_family )
  {
    addr_info_ptr = &netmgr_qmi_cfg.links[ link ].wds_info.addr_info.ipv6;
    netmgr_qmi_cfg.links[ link ].wds_info.addr_info.valid_mask |= NETMGR_ADDRSET_MASK_IPV6;
  }
  else
  {
    netmgr_log_err("unsupported IP family value[%d]\n", evt_ip_family );
    return NETMGR_FAILURE;
  }

  /* Cache results in WDS state info */
  if( QMI_WDS_CURR_CALL_INFO_MTU & call_info.mask ) {
    netmgr_qmi_cfg.links[ link ].wds_info.mtu =
      (NETMGR_MTU_MAX < call_info.mtu)? NETMGR_MTU_MAX : call_info.mtu;
    netmgr_log_med("Link MTU: %d\n", call_info.mtu );
  }
  else {
    netmgr_log_err("Link MTU undefined\n");
    netmgr_qmi_cfg.links[ link ].wds_info.mtu = NETMGR_MTU_INVALID;
    return NETMGR_FAILURE;
  }

  if( QMI_WDS_CURR_CALL_INFO_IP_FAMILY & call_info.mask ) {
    addr_info_ptr->if_addr.type = call_info.ip_family;
    netmgr_log_med("IP address family: 0x%02x\n", addr_info_ptr->if_addr.type );
  }
  else {
    netmgr_log_err("IP address family undefined\n");
    return NETMGR_FAILURE;
  }

  /* Process Um interface address */
  if( QMI_WDS_CURR_CALL_INFO_IPV4_ADDR & call_info.mask ) {
    /* Update the address only if an address purge is not pending */
    if (FALSE == addr_info_ptr->is_addr_purge_pend)
    {
      addr_info_ptr->if_addr.addr.v4 = htonl( (uint32_t)call_info.ipv4_addr );
      NETMGR_LOG_IPV4_ADDR( med, "Modem iface", addr_info_ptr->if_addr.addr.v4 );
    }
  }
  else
  {
    /* Update the address only if an address purge is not pending */
    if (FALSE == addr_info_ptr->is_addr_purge_pend)
    {
      if( QMI_WDS_CURR_CALL_INFO_IPV6_ADDR & call_info.mask ) {
        memcpy( addr_info_ptr->if_addr.addr.v6_addr8,
                call_info.ipv6_addr_info.ipv6_addr,
                sizeof(addr_info_ptr->if_addr.addr.v6_addr8) );
        NETMGR_LOG_IPV6_ADDR( med, "Modem iface",
                              addr_info_ptr->if_addr.addr.v6_addr64 );
      }
      else {
        netmgr_log_err("Modem iface IP address undefined\n");
        return NETMGR_FAILURE;
      }
    }

    /* Process IPv6 prefix len */
    addr_info_ptr->if_mask = (unsigned int)call_info.ipv6_addr_info.ipv6_prefix_len;
    netmgr_log_med( "Modem iface IPV6 Prefix len = %d\n", addr_info_ptr->if_mask );
  }

  /* Process IPV4 subnet mask */
  if( QMI_WDS_CURR_CALL_INFO_SUBNET_MASK & call_info.mask ) {
    addr_info_ptr->if_mask = htonl( (uint32_t)call_info.ipv4_subnet_mask );
    NETMGR_LOG_IPV4_ADDR( med, "Subnet mask", addr_info_ptr->if_mask );
  }


  /* Process Gateway  address */
  if( QMI_WDS_CURR_CALL_INFO_IPV4_GATEWAY_ADDR & call_info.mask ) {
    addr_info_ptr->gateway.addr.v4 = htonl( (uint32_t)call_info.ipv4_gateway_addr );
    addr_info_ptr->gateway.type = NETMGR_IPV4_ADDR;
    NETMGR_LOG_IPV4_ADDR( med, "Gateway", addr_info_ptr->gateway.addr.v4 );
  }
  else
  {
    if( QMI_WDS_CURR_CALL_INFO_IPV6_GTWY_ADDR & call_info.mask ) {
      memcpy( addr_info_ptr->gateway.addr.v6_addr8,
              call_info.ipv6_gateway_addr_info.ipv6_addr,
              sizeof(addr_info_ptr->gateway.addr.v6_addr8) );
      addr_info_ptr->gateway.type = NETMGR_IPV6_ADDR;
      NETMGR_LOG_IPV6_ADDR( med, "Gateway", addr_info_ptr->gateway.addr.v6_addr64 );

      addr_info_ptr->gw_mask = (unsigned int)call_info.ipv6_addr_info.ipv6_prefix_len;
      netmgr_log_med( "Gateway IPV6 Prefix len = %d\n", addr_info_ptr->gw_mask );
    }
    else {
      netmgr_log_err("Gateway IP address undefined\n");
      return NETMGR_FAILURE;
    }
  }

  /* Process DNS addresses (primary & secondary) */
  memset( &addr_info_ptr->dns_primary, 0x0, sizeof(addr_info_ptr->dns_primary) );
  memset( &addr_info_ptr->dns_secondary, 0x0, sizeof(addr_info_ptr->dns_secondary) );

  if( QMI_WDS_CURR_CALL_INFO_PRIMARY_DNS_IPV4_ADDR & call_info.mask ) {
    addr_info_ptr->dns_primary.type = NETMGR_IPV4_ADDR;
    addr_info_ptr->dns_primary.addr.v4 =
      htonl( (uint32_t)call_info.primary_dns_ipv4_addr );
    NETMGR_LOG_IPV4_ADDR( med, "DNS Primary",
                          addr_info_ptr->dns_primary.addr.v4 );
  } else if( QMI_WDS_CURR_CALL_INFO_PRIMARY_DNS_IPV6_ADDR & call_info.mask ) {
    addr_info_ptr->dns_primary.type = NETMGR_IPV6_ADDR;
    memcpy( addr_info_ptr->dns_primary.addr.v6_addr8,
            call_info.primary_dns_ipv6_addr,
            sizeof(addr_info_ptr->dns_primary.addr.v6_addr8) );
    NETMGR_LOG_IPV6_ADDR( med, "DNS Primary",
                          addr_info_ptr->dns_primary.addr.v6_addr64 );
  } else {
    netmgr_log_low("DNS primary address undefined\n");
  }

  if( QMI_WDS_CURR_CALL_INFO_SECONDARY_DNS_IPV4_ADDR & call_info.mask ) {
    addr_info_ptr->dns_secondary.type = NETMGR_IPV4_ADDR;
    addr_info_ptr->dns_secondary.addr.v4 =
      htonl( (uint32_t)call_info.secondary_dns_ipv4_addr );
    NETMGR_LOG_IPV4_ADDR( med, "DNS Secondary",
                          addr_info_ptr->dns_secondary.addr.v4 );
  } else if( QMI_WDS_CURR_CALL_INFO_SECONDARY_DNS_IPV6_ADDR & call_info.mask ) {
    addr_info_ptr->dns_secondary.type = NETMGR_IPV6_ADDR;
    memcpy( addr_info_ptr->dns_secondary.addr.v6_addr8,
            call_info.secondary_dns_ipv6_addr,
            sizeof(addr_info_ptr->dns_secondary.addr.v6_addr8) );
    NETMGR_LOG_IPV6_ADDR( med, "DNS Secondary",
                          addr_info_ptr->dns_secondary.addr.v6_addr64 );
  } else {
    netmgr_log_low("DNS secondary address undefined\n");
  }

  NETMGR_LOG_FUNC_EXIT;
  return NETMGR_SUCCESS;
}


/*===========================================================================
  FUNCTION  netmgr_sys_ind
===========================================================================*/
/*!
@brief
 Performs processing of an incoming QMI SYS Indication message. This function
 is executed in the Command Thread context.

@return
  void

@note

  - Dependencies
    - None

  - Side Effects
    - Depending on the system indication, appropriate action is
      taken to release/acquire QMI resources
*/
/*=========================================================================*/
LOCAL void
netmgr_sys_ind
(
  qmi_sys_event_type event_id,
  const qmi_sys_event_info_type * event_info
)
{
  netmgr_sm_events_t sm_ev = NETMGR_INVALID_EV;
  netmgr_exec_cmd_t * cmd = NULL;
  int i=0;

  /* Process based on indication type */
  switch (event_id) {
  case QMI_SYS_EVENT_MODEM_OUT_OF_SERVICE_IND:
    netmgr_log_high("rcvd QMI_SYS_EVENT_MODEM_OUT_OF_SERVICE_IND on "
                    "conn_id=%d dev_id=%s\n",
                    event_info->qmi_modem_service_ind.conn_id,
                    event_info->qmi_modem_service_ind.dev_id);
    netmgr_rmnet_remove_all_configuration();
    if (netmgr_main_get_low_latency_filters_enabled())
    {
      netmgr_kif_ifioctl_change_sleep_state((const char *)netmgr_main_get_phys_net_dev(),
                                            NETMGR_KIF_RESET_SLEEP_STATE);
      rmnet_qmi_dfs_release_qmi_client((const char *)netmgr_main_get_phys_net_dev());
    }
    break;
  case QMI_SYS_EVENT_MODEM_IN_SERVICE_IND:
    netmgr_log_high("rcvd QMI_SYS_EVENT_MODEM_IS_SERVICE_IND on "
                    "conn_id=%d dev_id=%s\n",
                    event_info->qmi_modem_service_ind.conn_id,
                    event_info->qmi_modem_service_ind.dev_id);
    if (netmgr_main_get_rmnet_data_enabled())
    {
      netmgr_rmnet_configure_embedded_data();
    }
    netmgr_kif_set_qos_header_format(NETMGR_RMNET_START,
                                     &(netmgr_main_cfg.data_format));
    if (netmgr_main_get_low_latency_filters_enabled())
    {
      netmgr_qmi_dfs_enable_low_latency_filters((const char *)netmgr_main_get_phys_net_dev());
    }
    break;
  default:
    netmgr_log_low("ignore qmi sys ind [%d]\n", event_id);
    break;
  }

  return;
}

/*===========================================================================
  FUNCTION  netmgr_wds_pkt_srvc_status_ind
===========================================================================*/
/*!
@brief
 Performs processing of an incoming WDS Packet Service Status Indication
 message. This function is executed in the Command Thread context.

@return
  void

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
LOCAL void
netmgr_wds_pkt_srvc_status_ind
(
  int                                  link,
  int                                  user_handle,
  const qmi_wds_indication_data_type * ind_data
)
{
  qmi_wds_link_status_type link_status;
  qmi_wds_call_end_reason_type  call_end_reason;
  qmi_ip_family_pref_type ip_family = QMI_IP_FAMILY_PREF_IPV4; /* assume IPV4 */
  netmgr_exec_cmd_t * cmd;
  netmgr_address_set_t * address_set_ptr = NULL;
  netmgr_qmi_client_type_t clnt;

  NETMGR_LOG_FUNC_ENTRY;

  /* Verify link id */
  NETMGR_ASSERT(netmgr_qmi_verify_link(link) == NETMGR_SUCCESS);

  /* verify indication data */
  NETMGR_ASSERT(ind_data);

  link_status = ind_data->pkt_srvc_status.link_status;
  if( QMI_WDS_PKT_SRVC_IND_IP_FAMILY & ind_data->pkt_srvc_status.param_mask )
  {
    ip_family = ind_data->pkt_srvc_status.ip_family;
  }

  clnt = netmgr_qmi_convert_ip_pref_to_client_type(ip_family);

  /* Ignore the indication on the different QMI WDS handle */
  if (user_handle != netmgr_qmi_cfg.links[link].wds_info.clnt_hdl[clnt])
  {
    netmgr_log_low("ignoring IP[%d] type pkt_srvc_ind on WDS handle[0x%x] for link[%d]\n",
                   ip_family,
                   user_handle,
                   link);
    return;
  }

  /* Check Modem interface link status */
  switch( link_status ) {
    case QMI_WDS_PACKET_DATA_CONNECTED:
    {
      /* Allocate address buffer; released in SM module */
      address_set_ptr = ds_malloc( sizeof(netmgr_address_set_t) );
      if( NULL == address_set_ptr )
      {
        netmgr_log_err("failed to allocate address_set\n");
        return;
      }
      memset(address_set_ptr, 0, sizeof(netmgr_address_set_t));

      /* Update the 'first_conn_clnt' after first PACKET_DATA_CONNECTED event */
      if (NETMGR_QMI_CLIENT_MAX == netmgr_qmi_cfg.links[link].wds_info.first_conn_clnt)
      {
        netmgr_qmi_cfg.links[link].wds_info.first_conn_clnt = clnt;
      }

      /*-------------------------------------------------------------------------
        For reconfiguraiton, capture the current address before its updated
      -------------------------------------------------------------------------*/
      if( ind_data->pkt_srvc_status.reconfig_required )
      {
        if( NETMGR_ADDRSET_MASK_INVALID !=
            netmgr_qmi_cfg.links[ link ].wds_info.addr_info.valid_mask )
        {
          /* Copy current address into buffer */
          *address_set_ptr = (QMI_WDS_IP_FAMILY_PREF_IPV4 == ip_family)?
                             netmgr_qmi_cfg.links[ link ].wds_info.addr_info.ipv4 :
                             netmgr_qmi_cfg.links[ link ].wds_info.addr_info.ipv6;
        }
        else
        {
          netmgr_log_err("no previously cached address for link[%d]\n", link);
        }
      }

      /*-------------------------------------------------------------------------
        Query Modem network interface for link info
      -------------------------------------------------------------------------*/
      if (
#ifndef FEATURE_DATA_PREFIX_DELEGATION
          (ip_family == QMI_IP_FAMILY_PREF_IPV4) &&
#endif /* FEATURE_DATA_PREFIX_DELEGATION */
          (NETMGR_SUCCESS != netmgr_qmi_get_modem_link_info( link, ip_family ))
         )
      {
        /* Just report error and continue */
        netmgr_log_err("failed on netmgr_qmi_get_modem_link_info\n");
      }

      if( !ind_data->pkt_srvc_status.reconfig_required )
      {
        if (ip_family == QMI_IP_FAMILY_PREF_IPV4)
        {
          *address_set_ptr = netmgr_qmi_cfg.links[ link ].wds_info.addr_info.ipv4;
        }
        else if (ip_family == QMI_IP_FAMILY_PREF_IPV6)
        {
          netmgr_log_high("setting valid_mask for ip_family %d for link[%d]",ip_family,link);

          /* Update necessary fields */
          netmgr_qmi_cfg.links[ link ].wds_info.addr_info.valid_mask |= NETMGR_ADDRSET_MASK_IPV6;
          netmgr_qmi_cfg.links[ link ].wds_info.addr_info.ipv6.if_addr.type = NETMGR_IPV6_ADDR;

          *address_set_ptr = netmgr_qmi_cfg.links[ link ].wds_info.addr_info.ipv6;
        }
        else
        {
          netmgr_log_err("no previously cached address for link[%d]\n", link);
        }
      }

      /*-------------------------------------------------------------------------
        Post message to executive
      -------------------------------------------------------------------------*/
      cmd = netmgr_exec_get_cmd();
      NETMGR_ASSERT(cmd);

      cmd->data.type                   = NETMGR_WDS_CONNECTED_EV;
      cmd->data.link                   = link;
      cmd->data.info.connect_msg.reconfig_required =
        ind_data->pkt_srvc_status.reconfig_required;
      cmd->data.info.connect_msg.addr_info_ptr = address_set_ptr;

      if(ind_data->pkt_srvc_status.param_mask & QMI_WDS_PKT_SRVC_IND_TECH_NAME)
      {
        netmgr_qmi_cfg.links[ link ].wds_info.tech_name =
          ind_data->pkt_srvc_status.tech_name;
      }
      else
      {
        netmgr_qmi_cfg.links[ link ].wds_info.tech_name =
          QMI_WDS_IFACE_NAME_NOT_REPORTED;
      }

      netmgr_log_med("Posting connect event - family[%d/%d] reconfig[%d]\n",
                     (QMI_WDS_PKT_SRVC_IND_IP_FAMILY==(QMI_WDS_PKT_SRVC_IND_IP_FAMILY &
                                                       ind_data->pkt_srvc_status.param_mask)),
                     ip_family, cmd->data.info.connect_msg.reconfig_required );

      if( NETMGR_SUCCESS != netmgr_exec_put_cmd( cmd ) ) {
        NETMGR_ABORT("netmgr_wds_pkt_srvc_status_ind: failed to put commmand\n");
        netmgr_exec_release_cmd(cmd);
        ds_free( address_set_ptr );
        return;
      }
    }
    break;

    case QMI_WDS_PACKET_DATA_DISCONNECTED:
    {
      /* Modem interface has disconnected. */

      /* Check call end reason code if present */
      if( ind_data->pkt_srvc_status.param_mask &
          QMI_WDS_PKT_SRVC_IND_CALL_END_REASON ) {
        call_end_reason.legacy_reason =
          ind_data->pkt_srvc_status.call_end_reason.legacy_reason;

        if( call_end_reason.legacy_reason < QMI_WDS_CE_REASON_UNSPECIFIED ||
            call_end_reason.legacy_reason > QMI_WDS_CE_REASON_MAX )
        {
          netmgr_log_high("Invalid CALL_END_CODE received : %d\n",
                          call_end_reason.legacy_reason);
        }
      }

      /*-------------------------------------------------------------------------
        Post message to executive
      -------------------------------------------------------------------------*/
      cmd = netmgr_exec_get_cmd();
      NETMGR_ASSERT(cmd);

      cmd->data.type                                = NETMGR_WDS_DISCONNECTED_EV;
      cmd->data.link                                = link;

      /* Set the interface teardown flag.  For single-IP calls, the
       * interface will teardown when disconnect indication arrives.
       * For dual-IP calls, it may be two IP addresses are on a single
       * interface.  So the first disconnect must purge the specified
       * IP addreess, yet the other address lives on and network
       * interface state remains unchanged.  We clear the valid_mask
       * below for this address family to indicate the interface is
       * now effectively a single-IP call.  The next disconnect
       * indication will then teardown interface as normal.
       */
      cmd->data.info.disconnect_msg.teardown_iface       =
        (NETMGR_ADDRSET_MASK_IPV4V6 != netmgr_qmi_cfg.links[ link ].wds_info.addr_info.valid_mask);
      cmd->data.info.disconnect_msg.addr_info_ptr   =
        (QMI_IP_FAMILY_PREF_IPV6 == ip_family)?
        &netmgr_qmi_cfg.links[ link ].wds_info.addr_info.ipv6 :
        &netmgr_qmi_cfg.links[ link ].wds_info.addr_info.ipv4;

      /* Clear the address purge pending flags */
      netmgr_qmi_cfg.links[ link ].wds_info.addr_info.ipv4.is_addr_purge_pend = FALSE;
      netmgr_qmi_cfg.links[ link ].wds_info.addr_info.ipv6.is_addr_purge_pend = FALSE;

      /* If an address purge is being initiated, set the corresponding flag */
      if (FALSE == cmd->data.info.disconnect_msg.teardown_iface)
      {
        if (QMI_WDS_IP_FAMILY_PREF_IPV4 == ip_family)
        {
          netmgr_qmi_cfg.links[ link ].wds_info.addr_info.ipv4.is_addr_purge_pend = TRUE;
        }
        else
        {
          netmgr_qmi_cfg.links[ link ].wds_info.addr_info.ipv6.is_addr_purge_pend = TRUE;
        }
      }

      netmgr_log_med("Posting disconnect event - family[%d/%d] teardown[%d]\n",
                     (QMI_WDS_PKT_SRVC_IND_IP_FAMILY==(QMI_WDS_PKT_SRVC_IND_IP_FAMILY & ind_data->pkt_srvc_status.param_mask)),
                     ip_family, cmd->data.info.disconnect_msg.teardown_iface );

      if( NETMGR_SUCCESS != netmgr_exec_put_cmd( cmd ) ) {
        NETMGR_ABORT("netmgr_wds_pkt_srvc_status_ind: failed to put commmand\n");
        netmgr_exec_release_cmd(cmd);
        return;
      }

      /* Clear disconnected address from cached valid mask if specified */
      if( QMI_WDS_PKT_SRVC_IND_IP_FAMILY & ind_data->pkt_srvc_status.param_mask )
      {
        netmgr_qmi_cfg.links[ link ].wds_info.addr_info.valid_mask &=
          (QMI_IP_FAMILY_PREF_IPV4 == ip_family)?
          (unsigned int)~NETMGR_ADDRSET_MASK_IPV4 : (unsigned int)~NETMGR_ADDRSET_MASK_IPV6;
      }
    }
    break;

    case QMI_WDS_PACKET_DATA_SUSPENDED:
    case QMI_WDS_PACKET_DATA_AUTHENTICATING:
    default:
      /* Ignore in all other as this message is unsupported */
      netmgr_log_err("netmgr_wds_pkt_srvc_status_ind called, ignoring\n");
      break;
  }

  NETMGR_LOG_FUNC_EXIT;
  return;
}

#ifdef FEATURE_DATA_IWLAN
/*===========================================================================
  FUNCTION  netmgr_qmi_query_modem_mtu
===========================================================================*/
/*!
@brief
  Function to query Modem MTU information for specific link.

@return
  int - NETMGR_SUCCESS on successful operation, NETMGR_FAILURE otherwise

@note

  - Dependencies
    - netmgr_qmi_init() must have been called

  - Side Effects
    - None
*/
/*=========================================================================*/
LOCAL int netmgr_qmi_query_modem_mtu
(
  uint8  link
)
{
  qmi_wds_profile_id_type      profile_id;
  qmi_wds_profile_params_type  profile_params;
  qmi_wds_curr_call_info_type  call_info;
  qmi_wds_req_runtime_settings_params_type  req_mask = QMI_WDS_GET_CURR_CALL_INFO_MTU_PARAM_MASK;
  int client_hndl;
  int ret = NETMGR_FAILURE;
  int result, err_code;

  NETMGR_LOG_FUNC_ENTRY;

  /* Validate input */
  if (NETMGR_SUCCESS != netmgr_qmi_verify_link(link))
  {
    netmgr_log_err("netmgr_qmi_query_modem_mtu: invalid input param\n");
    goto bail;
  }

  /* Start with the IPv4 client handle */
  client_hndl = netmgr_qmi_cfg.links[link].wds_info.clnt_hdl[NETMGR_QMI_CLIENT_IPV4];

  netmgr_log_med("netmgr_qmi_query_modem_mtu: trying with IPv4 handle\n");

  /* Query Modem for requested parameters */
  result = qmi_wds_get_curr_call_info( client_hndl,
                                       req_mask,
                                       &profile_id,
                                       &profile_params,
                                       &call_info,
                                       &err_code);

  /* If the WDS client says that there is not an active call then try using the IPv6 handle */
  if (result == QMI_SERVICE_ERR && err_code == QMI_SERVICE_ERR_OUT_OF_CALL)
  {
    netmgr_log_med("netmgr_qmi_query_modem_mtu: trying with IPv6 handle\n");

    client_hndl = netmgr_qmi_cfg.links[link].wds_info.clnt_hdl[NETMGR_QMI_CLIENT_IPV6];

    result = qmi_wds_get_curr_call_info(client_hndl,
                                        req_mask,
                                        &profile_id,
                                        &profile_params,
                                        &call_info,
                                        &err_code);
  }

  if (QMI_NO_ERR != result)
  {
    netmgr_log_err("netmgr_qmi_query_modem_mtu: failed on qmi_wds_get_curr_call_info: "
                   "ret=%d err=%d\n",
                   result, err_code);
    goto bail;
  }

  /* Cache results in WDS state info */
  if (QMI_WDS_CURR_CALL_INFO_MTU & call_info.mask)
  {
    netmgr_qmi_cfg.links[ link ].wds_info.mtu =
      (NETMGR_MTU_MAX < call_info.mtu)? NETMGR_MTU_MAX : call_info.mtu;
    netmgr_log_med("netmgr_qmi_query_modem_mtu: received link MTU: %d from modem\n", call_info.mtu );
  }
  else
  {
    netmgr_log_err("netmgr_qmi_query_modem_mtu: didn't receive link MTU from modem\n");
    goto bail;
  }

  ret = NETMGR_SUCCESS;

bail:
  NETMGR_LOG_FUNC_EXIT;
  return ret;
}
#endif /* FEATURE_DATA_IWLAN */

/*===========================================================================
  FUNCTION  netmgr_wds_event_report_ind
===========================================================================*/
/*!
@brief
 Performs processing of an incoming WDS Event Report Indication message.
 This function is executed in the Command Thread context. This function
 currently reports 'physlink events' to clients.

@return
  void

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
LOCAL void
netmgr_wds_event_report_ind
(
  int                                  link,
  const qmi_wds_indication_data_type * ind_data
)
{
  netmgr_exec_cmd_t * cmd;
  qmi_wds_data_bearer_tech_type bearer_tech;

  NETMGR_LOG_FUNC_ENTRY;

  /* Verify link id */
  NETMGR_ASSERT( netmgr_qmi_verify_link(link) == NETMGR_SUCCESS );

  /* Verify indication data */
  NETMGR_ASSERT( ind_data != NULL );

  /* Check event mask, process acoordingly... */

  if ((QMI_WDS_EVENT_BEARER_TECH_EX_IND & ind_data->event_report.event_mask) ||
     (QMI_WDS_EVENT_BEARER_TECH_IND & ind_data->event_report.event_mask))
  {
    if ((QMI_WDS_EVENT_BEARER_TECH_EX_IND & ind_data->event_report.event_mask))
    {
#ifdef FEATURE_DATA_IWLAN
      qmi_wds_data_bearer_tech_type_ex  *prev_bt = NULL;
      prev_bt = &netmgr_qmi_cfg.links[link].wds_info.bearer_tech_ex.bt;
#endif /* FEATURE_DATA_IWLAN */

      netmgr_log_high("bearer_tech_ex_ind: link=%d technology=0x%x rat_mask=0x%08x so_mask=0x%016llx\n",
                      link,
                      ind_data->event_report.data_bearer_tech_type_ex.technology,
                      ind_data->event_report.data_bearer_tech_type_ex.rat_mask,
                      ind_data->event_report.data_bearer_tech_type_ex.so_mask);

#ifdef FEATURE_DATA_IWLAN
      /* Consider only valid bearer techs */
      if (QMI_WDS_BEARER_TECH_RAT_EX_NULL_BEARER == ind_data->event_report.data_bearer_tech_type_ex.rat_mask)
      {
        netmgr_log_med("bearer_tech_ex_ind: NULL bearer for link=%d, ignoring\n",
                       link);
        goto bail;
      }

      if (TRUE != netmgr_qmi_cfg.links[link].wds_info.bearer_tech_ex.is_valid)
      {
        netmgr_log_med("bearer_tech_ex_ind: prev bearer_tech invalid for link=%d, ignoring\n",
                       link);
      }
      else
      {
        const qmi_wds_data_bearer_tech_type_ex  *curr_bt = NULL;
        boolean is_prev_bt_iwlan = FALSE, is_curr_bt_iwlan = FALSE;

        curr_bt = &ind_data->event_report.data_bearer_tech_type_ex;

        is_prev_bt_iwlan = NETMGR_QMI_IS_BEARER_TECH_EX_IWLAN(prev_bt);
        is_curr_bt_iwlan = NETMGR_QMI_IS_BEARER_TECH_EX_IWLAN(curr_bt);

        netmgr_log_med("bearer_tech_ex_ind: link=%d is_prev_bt_iwlan=%d is_curr_bt_iwlan=%d\n",
                       link,
                       is_prev_bt_iwlan,
                       is_curr_bt_iwlan);

        /* Update the interface MTU if moving between iWLAN and WWAN */
        if ((FALSE == is_prev_bt_iwlan && TRUE == is_curr_bt_iwlan) ||
            (TRUE == is_prev_bt_iwlan && FALSE == is_curr_bt_iwlan))
        {
           if (NETMGR_SUCCESS == netmgr_qmi_query_modem_mtu((uint8)link))
           {
             netmgr_log_med("bearer_tech_ex_ind: link=%d transition=%s updating Modem MTU\n",
                            link,
                            (TRUE == is_prev_bt_iwlan) ? "IWLAN -> WWAN" : "WWAN -> IWLAN");
             (void)netmgr_kif_set_mtu(link, TRUE);
           }
           else
           {
             netmgr_log_err("bearer_tech_ex_ind: link=%d transition=%s MTU query failed\n",
                            link,
                            (TRUE == is_prev_bt_iwlan) ? "IWLAN -> WWAN" : "WWAN -> IWLAN");
           }
        }
      }

      /* Save the previous bearer tech */
      *prev_bt = ind_data->event_report.data_bearer_tech_type_ex;
      netmgr_qmi_cfg.links[link].wds_info.bearer_tech_ex.is_valid = TRUE;
#endif /* FEATURE_DATA_IWLAN */
    }
    else /* (QMI_WDS_EVENT_BEARER_TECH_IND & ind_data->event_report.event_mask)*/
    {
      netmgr_log_high("Bearer tech change current NW:%dx, SO mask: %d, rat mask:%d\n",
                      ind_data->event_report.data_bearer_tech_type.current_db_nw,
                      ind_data->event_report.data_bearer_tech_type.db_so_mask,
                      ind_data->event_report.data_bearer_tech_type.rat_mask);
      /* When Bearer tech changes, network service might adopt different configuration
       need to query and reset MTU in the kernel for the new network service */
      if( NETMGR_SUCCESS ==
          netmgr_qmi_get_modem_link_info((uint8)link, QMI_IP_FAMILY_PREF_IPV4))
      {
        (void)netmgr_kif_set_mtu(link, TRUE);
      }
      else
      {
        netmgr_log_err("qmi query modem link info failed!\n ");
      }
    }
  }

bail:
  NETMGR_LOG_FUNC_EXIT;
  return;
}

/*===========================================================================
  FUNCTION  netmgr_wds_ext_ip_config_ind
===========================================================================*/
/*!
@brief
 Performs processing of an incoming WDS Extended IP Indication message.
 This function is executed in the Command Thread context. This function
 currently reports DNS changes for IPv6 call to clients.

@return
  void

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
LOCAL void
    netmgr_wds_ext_ip_config_ind
(
  int                                 link,
  const qmi_wds_indication_data_type  *ind_data
)
{
  NETMGR_LOG_FUNC_ENTRY;
  netmgr_nl_event_info_t * event_info = NULL;
  netmgr_kif_state_t kif_state = netmgr_kif_get_state(link);
  const char * iface_name = NULL;

  /* If kernel iface is not completely configured, we can drop the indication as DNS info will
  be fetched anyways after kernel iface is configured, using get runtime settings */
  if( kif_state != NETMGR_KIF_OPEN )
  {
    netmgr_log_high("netmgr_wds_ext_ip_config_ind: netmgr kif is not configured yet."
                    "Dropping the EXT_IP indication");
    goto bail;
  }

  iface_name = netmgr_kif_get_name(link);
  if( NULL == iface_name)
  {
    netmgr_log_err("%s(): unable to determine name for link=%d\n", __func__, link);
    goto bail;
  }

  /* If indication is for DNS info, send RECONFIGURED event to cleints */
  if( ind_data->ext_ip_ind & QMI_WDS_GET_CURR_CALL_INFO_DNS_ADDR_PARAM_MASK ) {
    /* Post reconfigured indication to clients */
    event_info = netmgr_malloc( sizeof(netmgr_nl_event_info_t) );
    if( NULL == event_info ) {
      netmgr_log_err("Failed to allocate event buffer. Discarding EXT_IP ind");
      goto bail;
    }
    else {
      memset( event_info, 0x0, sizeof(netmgr_nl_event_info_t) );
      event_info->event = NET_PLATFORM_RECONFIGURED_EV;

      event_info->addr_info.addr.ip_addr.ss_family = AF_INET6;
      event_info->param_mask |= NETMGR_EVT_PARAM_IPADDR;

      event_info->link = link;
      event_info->param_mask |= NETMGR_EVT_PARAM_LINK;
      strlcpy( event_info->dev_name,
               iface_name,
               sizeof(event_info->dev_name) );
      event_info->param_mask |= NETMGR_EVT_PARAM_DEVNAME;

      if( NETMGR_SUCCESS != netmgr_kif_send_event_msg( event_info ) ) {
        netmgr_log_err("Failed on kif_send_event RECONFIGURED\n");
      }
      else {
        netmgr_log_high("kif_send_event[RECONFIGURED]: Success\n");
      }
    }
    netmgr_free( event_info );
  }
bail:
  NETMGR_LOG_FUNC_EXIT;
}

#ifdef FEATURE_DATA_IWLAN
/*===========================================================================
  FUNCTION  netmgr_wds_handle_rev_ip_connect_ind
===========================================================================*/
/*!
@brief
  Handler for the reverse IP "connected" transport connection indication

@return
  void

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
LOCAL void
netmgr_wds_handle_rev_ip_connect_ind
(
  int                                  link,
  int                                  user_handle,
  const qmi_wds_rev_ip_trans_ind_type  *rev_ip_ind
)
{
  netmgr_exec_cmd_t  *cmd = NULL;
  netmgr_address_info_t  *addr_info_ptr = NULL;
  netmgr_address_set_t  *addr_set_ptr = NULL;
  netmgr_wds_qmi_drv_info_t  *wds_info_ptr = NULL;
  int ip_type;
  netmgr_qmi_client_type_t  clnt;
  int rc = NETMGR_FAILURE;

  NETMGR_ASSERT( rev_ip_ind != NULL );

  NETMGR_LOG_FUNC_ENTRY;

  /* Ensure that we got the indication on the correctly bound WDS client */
  if (user_handle == netmgr_qmi_cfg.links[link].wds_info.clnt_hdl[NETMGR_QMI_CLIENT_IPV4])
  {
    if (!(QMI_WDS_REV_IP_TRANS_IND_IPV4_ADDR_PARAM_MASK & rev_ip_ind->param_mask))
    {
      netmgr_log_err("netmgr_wds_handle_rev_ip_connect_ind: indication on V4 WDS client without address\n");
      goto bail;
    }
    clnt = NETMGR_QMI_CLIENT_IPV4;
  }
  else if (user_handle == netmgr_qmi_cfg.links[link].wds_info.clnt_hdl[NETMGR_QMI_CLIENT_IPV6])
  {
    if (!(QMI_WDS_REV_IP_TRANS_IND_IPV6_ADDR_PARAM_MASK & rev_ip_ind->param_mask))
    {
      netmgr_log_err("netmgr_wds_handle_rev_ip_connect_ind: indication on V6 WDS client without address\n");
      goto bail;
    }
    clnt = NETMGR_QMI_CLIENT_IPV6;
  }
  else
  {
    netmgr_log_err("netmgr_wds_handle_rev_ip_connect_ind: invalid handle=0x%x received\n",
                   user_handle);
    goto bail;
  }

  if (!(rev_ip_ind->param_mask & QMI_WDS_REV_IP_TRANS_IND_TECH_NAME_PARAM_MASK))
  {
    netmgr_log_err("netmgr_wds_handle_rev_ip_connect_ind: missing technology\n");
    goto bail;
  }
  else if (QMI_WDS_IFACE_NAME_WLAN_LOCAL_BRKOUT != rev_ip_ind->tech_name &&
           QMI_WDS_IFACE_NAME_IWLAN_EPDG != rev_ip_ind->tech_name)
  {
    netmgr_log_err("netmgr_wds_handle_rev_ip_connect_ind: unexpected technology=0x%x\n",
                   rev_ip_ind->tech_name);
    goto bail;
  }

  /* Allocate address buffer; released in SM module */
  addr_set_ptr = ds_malloc( sizeof(netmgr_address_set_t) );
  if (NULL == addr_set_ptr)
  {
    netmgr_log_err("failed to allocate address_set\n");
    goto bail;
  }

  memset(addr_set_ptr, 0, sizeof(*addr_set_ptr));

  /* Enqueue the reverse ip txn id */
  if (NETMGR_SUCCESS != netmgr_qmi_wds_util_enq_txn(clnt, link, rev_ip_ind->txn_id))
  {
    netmgr_log_err("failed to enqueue txn=%ld, clnt=%d, link=%d\n",
                   rev_ip_ind->txn_id,
                   clnt,
                   link);
    goto bail;
  }

  wds_info_ptr = &netmgr_qmi_cfg.links[ link ].wds_info;
  addr_info_ptr = &wds_info_ptr->addr_info;

  netmgr_log_low("netmgr_wds_handle_rev_ip_connect_ind: tech_name=0x%x, txn_id=%ld, clnt=%d, link=%d\n",
                 rev_ip_ind->tech_name,
                 rev_ip_ind->txn_id,
                 clnt,
                 link);

  wds_info_ptr->tech_name = rev_ip_ind->tech_name;

  if (NETMGR_QMI_CLIENT_MAX == netmgr_qmi_cfg.links[link].wds_info.first_conn_clnt)
  {
    wds_info_ptr->first_conn_clnt = clnt;
  }

  /* Store the address and send a NETMGR_WDS_CONNECTED_EV */
  if (QMI_WDS_REV_IP_TRANS_IND_IPV4_ADDR_PARAM_MASK & rev_ip_ind->param_mask)
  {
    addr_info_ptr->valid_mask |= NETMGR_ADDRSET_MASK_IPV4;
    addr_info_ptr->ipv4.if_addr.type = NETMGR_IPV4_ADDR;
    addr_info_ptr->ipv4.if_addr.addr.v4 = htonl((uint32_t)rev_ip_ind->ipv4_addr);
    addr_info_ptr->ipv4.sa.is_sa_valid = FALSE;
    addr_info_ptr->ipv4.sa.is_sa_shared = FALSE;

    if (QMI_WDS_REV_IP_TRANS_IND_SA_CONFIG_SHARED_PARAM_MASK & rev_ip_ind->param_mask)
    {
      addr_info_ptr->ipv4.sa.is_sa_shared = rev_ip_ind->is_sa_shared;
    }

    NETMGR_LOG_IPV4_ADDR( med, "Modem iface", addr_info_ptr->ipv4.if_addr.addr.v4 );

    *addr_set_ptr = addr_info_ptr->ipv4;
  }
  else if (QMI_WDS_REV_IP_TRANS_IND_IPV6_ADDR_PARAM_MASK & rev_ip_ind->param_mask)
  {
    addr_info_ptr->valid_mask |= NETMGR_ADDRSET_MASK_IPV6;
    addr_info_ptr->ipv6.if_addr.type = NETMGR_IPV6_ADDR;
    addr_info_ptr->ipv6.if_mask = (unsigned int)rev_ip_ind->ipv6_addr_info.ipv6_prefix_len;
    memcpy(addr_info_ptr->ipv6.if_addr.addr.v6_addr8,
           rev_ip_ind->ipv6_addr_info.ipv6_addr,
           sizeof(addr_info_ptr->ipv6.if_addr.addr.v6_addr8));

    addr_info_ptr->ipv6.sa.is_sa_valid = FALSE;
    addr_info_ptr->ipv6.sa.is_sa_shared = FALSE;

    if (QMI_WDS_REV_IP_TRANS_IND_SA_CONFIG_SHARED_PARAM_MASK & rev_ip_ind->param_mask)
    {
      addr_info_ptr->ipv6.sa.is_sa_shared = rev_ip_ind->is_sa_shared;
    }

    NETMGR_LOG_IPV6_ADDR( med, "Modem iface",
                          addr_info_ptr->ipv6.if_addr.addr.v6_addr64 );

    *addr_set_ptr = addr_info_ptr->ipv6;
  }

  /*-------------------------------------------------------------------------
    Post message to executive
  -------------------------------------------------------------------------*/
  cmd = netmgr_exec_get_cmd();
  NETMGR_ASSERT(cmd);

  cmd->data.type = NETMGR_WDS_CONNECTED_EV;
  cmd->data.link = link;
  cmd->data.info.connect_msg.reconfig_required = FALSE;
  cmd->data.info.connect_msg.addr_info_ptr = addr_set_ptr;

  netmgr_log_med("Posting reverse connect event - family[%d] \n",
                 clnt);

  if( NETMGR_SUCCESS != netmgr_exec_put_cmd( cmd ) ) {
    NETMGR_ABORT("netmgr_wds_handle_rev_ip_connect_ind: failed to put commmand\n");
    netmgr_exec_release_cmd(cmd);
    goto bail;
  }

  rc = NETMGR_SUCCESS;

bail:
  if (NETMGR_SUCCESS != rc)
  {
    if (NULL != addr_set_ptr)
    {
      ds_free( addr_set_ptr );
    }
  }

  NETMGR_LOG_FUNC_EXIT;
}

/*===========================================================================
  FUNCTION  netmgr_wds_handle_rev_ip_disconnect_ind
===========================================================================*/
/*!
@brief
  Handler for the reverse IP "disconnected" transport connection indication
@return
  void

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
LOCAL void
netmgr_wds_handle_rev_ip_disconnect_ind
(
  int                                  link,
  int                                  user_handle,
  const qmi_wds_rev_ip_trans_ind_type  *rev_ip_ind
)
{
  netmgr_exec_cmd_t  *cmd = NULL;
  netmgr_qmi_client_type_t  clnt;

  NETMGR_ASSERT( rev_ip_ind != NULL );

  NETMGR_LOG_FUNC_ENTRY;

  /* Modem interface has disconnected. */

  /* Determine the IP type of the call that got disconnected */
  if (user_handle == netmgr_qmi_cfg.links[link].wds_info.clnt_hdl[NETMGR_QMI_CLIENT_IPV4])
  {
    clnt = NETMGR_QMI_CLIENT_IPV4;
  }
  else if (user_handle == netmgr_qmi_cfg.links[link].wds_info.clnt_hdl[NETMGR_QMI_CLIENT_IPV6])
  {
    clnt = NETMGR_QMI_CLIENT_IPV6;
  }
  else
  {
    netmgr_log_err("netmgr_wds_handle_rev_ip_disconnect_ind: invalid handle=0x%x received\n",
                   user_handle);
    goto bail;
  }

  if (NETMGR_SUCCESS != netmgr_qmi_wds_util_enq_txn(clnt, link, rev_ip_ind->txn_id))
  {
    netmgr_log_err("failed to enqueue txn=%ld, clnt=%d, link=%d\n",
                   rev_ip_ind->txn_id,
                   clnt,
                   link);
    goto bail;
  }

  /*-------------------------------------------------------------------------
    Post message to executive
  -------------------------------------------------------------------------*/
  cmd = netmgr_exec_get_cmd();
  NETMGR_ASSERT(cmd);

  cmd->data.type = NETMGR_WDS_DISCONNECTED_EV;
  cmd->data.link = link;

  /* Set the interface teardown flag.  For single-IP calls, the
   * interface will teardown when disconnect indication arrives.
   * For dual-IP calls, it may be two IP addresses are on a single
   * interface.  So the first disconnect must purge the specified
   * IP addreess, yet the other address lives on and network
   * interface state remains unchanged.  We clear the valid_mask
   * below for this address family to indicate the interface is
   * now effectively a single-IP call.  The next disconnect
   * indication will then teardown interface as normal.
   */
  cmd->data.info.disconnect_msg.teardown_iface       =
    (NETMGR_ADDRSET_MASK_IPV4V6 != netmgr_qmi_cfg.links[ link ].wds_info.addr_info.valid_mask);
  cmd->data.info.disconnect_msg.addr_info_ptr   =
    (NETMGR_QMI_CLIENT_IPV6 == clnt)?
    &netmgr_qmi_cfg.links[ link ].wds_info.addr_info.ipv6 :
    &netmgr_qmi_cfg.links[ link ].wds_info.addr_info.ipv4;

  /* Clear the address purge pending flags */
  netmgr_qmi_cfg.links[ link ].wds_info.addr_info.ipv4.is_addr_purge_pend = FALSE;
  netmgr_qmi_cfg.links[ link ].wds_info.addr_info.ipv6.is_addr_purge_pend = FALSE;

  /* If an address purge is being initiated, set the corresponding flag */
  if (FALSE == cmd->data.info.disconnect_msg.teardown_iface)
  {
    if (NETMGR_QMI_CLIENT_IPV4 == clnt)
    {
      netmgr_qmi_cfg.links[ link ].wds_info.addr_info.ipv4.is_addr_purge_pend = TRUE;
    }
    else
    {
      netmgr_qmi_cfg.links[ link ].wds_info.addr_info.ipv6.is_addr_purge_pend = TRUE;
    }
  }

  /* Clear disconnected address from cached valid mask if specified */
  netmgr_qmi_cfg.links[ link ].wds_info.addr_info.valid_mask &= (NETMGR_QMI_CLIENT_IPV4 == clnt) ?
                                                                (unsigned int)~NETMGR_ADDRSET_MASK_IPV4 :
                                                                (unsigned int)~NETMGR_ADDRSET_MASK_IPV6;

  netmgr_log_med("Posting reverse disconnect event - family[%d] \n",
                 clnt);

  if (NETMGR_SUCCESS != netmgr_exec_put_cmd(cmd))
  {
    NETMGR_ABORT("netmgr_wds_pkt_srvc_status_ind: failed to put commmand\n");
    netmgr_exec_release_cmd(cmd);
    return;
  }

bail:
  NETMGR_LOG_FUNC_EXIT;
}

/*===========================================================================
  FUNCTION  netmgr_wds_rev_ip_trans_ind
===========================================================================*/
/*!
@brief
 Performs processing of an incoming WDS reverse IP transport connection
 indication. This function is executed in the Command Thread context.

@return
  void

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
LOCAL void
netmgr_wds_rev_ip_trans_conn_ind
(
  int                                 link,
  int                                 user_handle,
  const qmi_wds_indication_data_type  *ind_data
)
{
  netmgr_exec_cmd_t * cmd;
  const qmi_wds_rev_ip_trans_ind_type  *rev_ip_ind = NULL;

  NETMGR_LOG_FUNC_ENTRY;

  /* Verify link id */
  NETMGR_ASSERT( netmgr_qmi_verify_link(link) == NETMGR_SUCCESS );

  /* Verify indication data */
  NETMGR_ASSERT( ind_data != NULL );

  rev_ip_ind = &ind_data->rev_ip_conn_ind;

  switch (rev_ip_ind->conn_status)
  {
     case QMI_WDS_REV_IP_TRANS_CONN_STATUS_CONNECTED:
      netmgr_wds_handle_rev_ip_connect_ind(link, user_handle, rev_ip_ind);
      break;

    case QMI_WDS_REV_IP_TRANS_CONN_STATUS_DISCONNECTED:
      netmgr_wds_handle_rev_ip_disconnect_ind(link, user_handle, rev_ip_ind);
      break;

    default:
      netmgr_log_err("netmgr_wds_rev_ip_trans_conn_ind: unknown conn_status=%d\n",
                     rev_ip_ind->conn_status);
      break;
  }

  NETMGR_LOG_FUNC_EXIT;
  return;
}
#endif /* FEATURE_DATA_IWLAN */

/*===========================================================================
  FUNCTION  netmgr_wds_ind
===========================================================================*/
/*!
@brief
 Performs processing of an incoming WDS Indication message. This function
 is executed in the Command Thread context.

@return
  void

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
LOCAL void
netmgr_wds_ind
(
  netmgr_qmi_wds_ind_t  *netmgr_qmi_wds_ind
)
{
  /* Verify input parameter */
  NETMGR_ASSERT( netmgr_qmi_wds_ind != NULL );

  /* Verify link id is valid before proceeding */
  NETMGR_ASSERT(netmgr_qmi_verify_link(netmgr_qmi_wds_ind->link) == NETMGR_SUCCESS);

  /* Process based on indication type */
  switch (netmgr_qmi_wds_ind->ind_id) {
    case QMI_WDS_SRVC_PKT_SRVC_STATUS_IND_MSG:
      /* Process packet service status indication */
      netmgr_wds_pkt_srvc_status_ind(netmgr_qmi_wds_ind->link,
                                     netmgr_qmi_wds_ind->user_hndl,
                                     &netmgr_qmi_wds_ind->ind_data);
      break;

    case QMI_WDS_SRVC_EVENT_REPORT_IND_MSG:
      /* Process Event Report indication */
      netmgr_wds_event_report_ind(netmgr_qmi_wds_ind->link,
                                  &netmgr_qmi_wds_ind->ind_data);
      break;

#ifdef FEATURE_DATA_IWLAN
    case QMI_WDS_SRVC_REV_IP_TRANSPORT_IND_MSG:
      /* Process Event Report indication */
      netmgr_wds_rev_ip_trans_conn_ind(netmgr_qmi_wds_ind->link,
                                       netmgr_qmi_wds_ind->user_hndl,
                                       &netmgr_qmi_wds_ind->ind_data);
      break;
#endif /* FEATURE_DATA_IWLAN */

    case QMI_WDS_SRVC_EXT_IP_CONFIG_IND_MSG:
      /* Process Extended IP indication */
      netmgr_log_high("Processing Extended IP indication");
      netmgr_wds_ext_ip_config_ind(netmgr_qmi_wds_ind->link,
                                       &netmgr_qmi_wds_ind->ind_data);
      break;

    default:
      /* Ignore all other indications */
      netmgr_log_high("Ignoring QMI WDS IND of type %d\n", netmgr_qmi_wds_ind->ind_id);
  }

  return;
}

#ifdef NETMGR_QOS_ENABLED
/*===========================================================================
  FUNCTION  netmgr_qos_flow_decode_umts_params
===========================================================================*/
/*!
@brief
  Decodes the QoS specificaiton UMTS parameters for priority and datarate

@return
  int - NETMGR_SUCCESS on successful operation, NETMGR_FAILURE otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
LOCAL int
netmgr_qos_flow_decode_umts_params
(
  const qmi_qos_umts_flow_desc_type * umts_desc,
  uint8                             * priority,
  uint32                            * datarate
)
{
  NETMGR_ASSERT( umts_desc );
  NETMGR_ASSERT( priority );
  NETMGR_ASSERT( datarate );

  NETMGR_LOG_FUNC_ENTRY;

  /* Decode datarate paramater */
  if( umts_desc->param_mask & QMI_QOS_UMTS_FLOW_PARAM_DATA_RATE ) {
    /* Use guaranteed datarate value */
    *datarate = (uint32)umts_desc->data_rate.guaranteed_rate;
  }
  else {
    /* Assume default datarate */
    netmgr_log_err("undefined data rate, assuming default\n");
    *datarate = NETMGR_TC_DEFAULT_DATARATE;
  }

  /* Decode priority paramater.  This is derived from traffic class */
  if( umts_desc->param_mask & QMI_QOS_UMTS_FLOW_PARAM_TRAFFIC_CLASS ) {
    switch( umts_desc->traffic_class ) {
      case QMI_QOS_UMTS_TC_CONVERSATIONAL:
        *priority = NETMGR_TC_CLASS_PRIO_CONVERSATIONAL;
        break;
      case QMI_QOS_UMTS_TC_STREAMING:
        *priority = NETMGR_TC_CLASS_PRIO_STREAMING;
        break;
      case QMI_QOS_UMTS_TC_INTERACTIVE:
        *priority = NETMGR_TC_CLASS_PRIO_INTERACTIVE;
        break;
      case QMI_QOS_UMTS_TC_BACKGROUND:
        *priority = NETMGR_TC_CLASS_PRIO_BACKGROUND;
        break;
      default:
        /* Assume default datarate */
        netmgr_log_err("unknown traffic class, assuming default\n");
        *priority = NETMGR_TC_DEFAULT_PRIORITY;
        break;
    }
  }
  else {
    /* Assume default datarate */
    netmgr_log_err("undefined traffic class, assuming default\n");
    *priority = NETMGR_TC_DEFAULT_PRIORITY;
  }

  NETMGR_LOG_FUNC_EXIT;

  return NETMGR_SUCCESS;
}


/*===========================================================================
  FUNCTION  netmgr_qos_flow_decode_cdma_params
===========================================================================*/
/*!
@brief
  Decodes the QoS specificaiton CDMA parameters for priority and datarate

@return
  int - NETMGR_SUCCESS on successful operation, NETMGR_FAILURE otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
LOCAL int
netmgr_qos_flow_decode_cdma_params
(
  const qmi_qos_cdma_flow_desc_type * cdma_desc,
  uint8                             * priority,
  uint32                            * datarate
)
{
  NETMGR_ASSERT( cdma_desc );
  NETMGR_ASSERT( priority );
  NETMGR_ASSERT( datarate );

  /* Lookup the parameters based on the profile ID */
  if( cdma_desc->param_mask & QMI_QOS_CDMA_FLOW_PARAM_PROFILE_ID ) {
    if( NETMGR_SUCCESS !=
        netmgr_tc_get_qos_params_by_profile_id( (uint16)cdma_desc->profile_id,
                                                datarate,
                                                priority ) ) {

      netmgr_log_err( "Error decoding CDMA TX flow data!\n");
      return NETMGR_FAILURE;
    }

  }
  else {
    netmgr_log_err("undefined profile ID, assuming defaults\n");
    *priority = NETMGR_TC_DEFAULT_PRIORITY;
    *datarate = NETMGR_TC_DEFAULT_DATARATE;
  }
  return NETMGR_SUCCESS;
}

/*===========================================================================
  FUNCTION  netmgr_qos_flow_activated
===========================================================================*/
/*!
@brief
  Performs processing of an QoS flow activated notification.

@return
  void

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
LOCAL void
netmgr_qos_flow_activated
(
  int                                   link,
  qmi_qos_event_report_flow_info_type * flow_ptr
)
{
  netmgr_exec_cmd_t * cmd = NULL;
  uint8     priority = NETMGR_TC_DEFAULT_PRIORITY;
  uint32    datarate = NETMGR_TC_DEFAULT_DATARATE;
  const qmi_qos_flow_req_type * flow = NULL;
  qmi_qos_granted_filter_data_type * filter = NULL;
  unsigned int i;

  NETMGR_ASSERT( flow_ptr );

  NETMGR_LOG_FUNC_ENTRY;

  /* Check if this activation is for a new flow.  For new flows, need
   * to decode QoS specification parameters.  For exisiting flows,
   * this data has previously been received. */
  if( flow_ptr->qos_flow_state.new_flow ) {

    /* Check for QoS TX flow specification */
    if( flow_ptr->tx_granted_flow_data_is_valid )
    {
      flow = &flow_ptr->tx_granted_flow_data.qos_flow_granted;

      /* Check for UMTS parameter set */
      if( flow->umts_flow_desc.param_mask ) {
        if( NETMGR_SUCCESS !=
            netmgr_qos_flow_decode_umts_params( &flow->umts_flow_desc,
                                                &priority,
                                                &datarate ) ) {

          netmgr_log_err( "Error decoding UMTS TX flow data!\n");
          return;
        }
      }
      /* Check for CDMA parameter set */
      else if( flow->cdma_flow_desc.param_mask ) {
        if( NETMGR_SUCCESS !=
            netmgr_qos_flow_decode_cdma_params( &flow->cdma_flow_desc,
                                                &priority,
                                                &datarate ) ) {

          netmgr_log_err( "Error decoding CDMA TX flow data!\n");
          return;
        }
      }
      else {
        netmgr_log_err( "TX flow data undefined, using defaults!\n");
      }
    }
    else {
      /* No specification available, assume default values */
      netmgr_log_err( "TX flow data undefined, using defaults!\n");
    }
  }

  /* Allocate a command object */
  cmd = netmgr_exec_get_cmd();
  NETMGR_ASSERT(cmd);

  /* Set command object parameters */
  cmd->data.type                                = NETMGR_QOS_ACTIVATE_EV;
  cmd->data.link                                = link;

  cmd->data.info.qos_flow.flow_id   = (uint32)flow_ptr->qos_flow_state.qos_identifier;
  cmd->data.info.qos_flow.is_new    = flow_ptr->qos_flow_state.new_flow;
  cmd->data.info.qos_flow.flow_type = (uint8)((QMI_QOS_EVENT_REPORT_FLOW_INFO_FLOW_TYPE_PARAM &
                                       flow_ptr->param_mask)?
                                       flow_ptr->flow_type : NETMGR_FLOW_TYPE_INVALID);
  cmd->data.info.qos_flow.priority  = priority;
  cmd->data.info.qos_flow.datarate  = datarate;

  cmd->data.info.qos_flow.num_filter = (uint8)flow_ptr->tx_filter_count;
  memcpy( (void*)cmd->data.info.qos_flow.filter_list,
          flow_ptr->tx_granted_filter_data,
          sizeof(cmd->data.info.qos_flow.filter_list) );

  /* Post command for processing in the command thread context */
  if( NETMGR_SUCCESS != netmgr_exec_put_cmd( cmd ) ) {
    NETMGR_ABORT("netmgr_qos_flow_activated: failed to put commmand\n");
    netmgr_exec_release_cmd(cmd);
  }

  NETMGR_LOG_FUNC_EXIT;
  return;
}

/*===========================================================================
  FUNCTION  netmgr_qos_flow_modified
===========================================================================*/
/*!
@brief
  Performs processing of an QoS flow modified notification.

@return
  void

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
LOCAL void
netmgr_qos_flow_modified
(
  int                                  link,
  qmi_qos_event_report_flow_info_type * flow_ptr
)
{
  netmgr_exec_cmd_t* cmd = NULL;
  uint8 priority = NETMGR_TC_DEFAULT_PRIORITY;
  uint32 datarate = NETMGR_TC_DEFAULT_DATARATE;
  const qmi_qos_flow_req_type* flow = NULL;
  int err;

  NETMGR_LOG_FUNC_ENTRY;

  NETMGR_ASSERT( flow_ptr );

  // Check for Qos TX flow specification
  if (flow_ptr->tx_granted_flow_data_is_valid)
  {
    flow = &flow_ptr->tx_granted_flow_data.qos_flow_granted;

    if (flow->umts_flow_desc.param_mask)
    {
      err = netmgr_qos_flow_decode_umts_params(&flow->umts_flow_desc,
                                               &priority,
                                               &datarate);

      if (err != NETMGR_SUCCESS)
      {
        netmgr_log_err("Error decoding UMTS TX flow data\n");
        return;
      }
    }
    else if (flow->cdma_flow_desc.param_mask)
    {
      err = netmgr_qos_flow_decode_cdma_params(&flow->cdma_flow_desc,
                                               &priority,
                                               &datarate);

      if (err != NETMGR_SUCCESS)
      {
        netmgr_log_err("Error decoding CDMA TX flow data\n");
        return;
      }
    }
    else
    {
      // no spec provided
      netmgr_log_err("TX flow data undefined, using defaults\n");
    }
  }

  // Allocate cmd object
  cmd = netmgr_exec_get_cmd();
  NETMGR_ASSERT(cmd);

  // Set command data
  cmd->data.type = NETMGR_QOS_MODIFY_EV;
  cmd->data.link = link;
  cmd->data.info.qos_flow.flow_id = (uint32)flow_ptr->qos_flow_state.qos_identifier;
  cmd->data.info.qos_flow.is_new = flow_ptr->qos_flow_state.new_flow;
  cmd->data.info.qos_flow.flow_type = (uint8)((QMI_QOS_EVENT_REPORT_FLOW_INFO_FLOW_TYPE_PARAM &
                                       flow_ptr->param_mask)?
                                      flow_ptr->flow_type : NETMGR_FLOW_TYPE_INVALID);
  cmd->data.info.qos_flow.priority = priority;
  cmd->data.info.qos_flow.datarate = datarate;

  cmd->data.info.qos_flow.num_filter = (uint8)flow_ptr->tx_filter_count;
  memcpy(cmd->data.info.qos_flow.filter_list,
         flow_ptr->tx_granted_filter_data,
         sizeof(cmd->data.info.qos_flow.filter_list));

  // Post command for processing in the command thread context
  if ( NETMGR_SUCCESS != netmgr_exec_put_cmd( cmd ) )
  {
    NETMGR_ABORT("netmgr_qos_flow_modified: failed to put commmand\n");
    netmgr_exec_release_cmd(cmd);
  }

  NETMGR_LOG_FUNC_EXIT;
  return;
}

/*===========================================================================
  FUNCTION  netmgr_qos_flow_deleted
===========================================================================*/
/*!
@brief
  Performs processing of an QoS flow deleted notification.

@return
  void

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
LOCAL void
netmgr_qos_flow_deleted
(
  int                                   link,
  qmi_qos_event_report_flow_info_type * flow_ptr
)
{
  netmgr_exec_cmd_t * cmd = NULL;
  NETMGR_LOG_FUNC_ENTRY;

  NETMGR_ASSERT( flow_ptr );

  // Allocate cmd object
  cmd = netmgr_exec_get_cmd();
  NETMGR_ASSERT(cmd);
  cmd->data.type = NETMGR_QOS_DELETE_EV;
  cmd->data.link = link;
  cmd->data.info.qos_flow.flow_id = (uint32)flow_ptr->qos_flow_state.qos_identifier;
  cmd->data.info.qos_flow.flow_type = (uint8)((QMI_QOS_EVENT_REPORT_FLOW_INFO_FLOW_TYPE_PARAM &
                                       flow_ptr->param_mask)?
                                      flow_ptr->flow_type : NETMGR_FLOW_TYPE_INVALID);

  // Post command
  if (NETMGR_SUCCESS != netmgr_exec_put_cmd(cmd))
  {
    NETMGR_ABORT("netmgr_qos_flow_delete: failed to put command");
    netmgr_exec_release_cmd(cmd);
  }

  NETMGR_LOG_FUNC_EXIT;
  return;
}

/*===========================================================================
  FUNCTION  netmgr_qos_flow_suspended
===========================================================================*/
/*!
@brief
  Performs processing of an QoS flow suspended notification.

@return
  void

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
LOCAL void
netmgr_qos_flow_suspended
(
  int                                         link,
  const qmi_qos_event_report_flow_info_type   *flow_ptr
)
{
  netmgr_exec_cmd_t * cmd = NULL;
  uint8     priority = NETMGR_TC_DEFAULT_PRIORITY;
  uint32    datarate = NETMGR_TC_DEFAULT_DATARATE;
  const qmi_qos_flow_req_type * flow = NULL;
  qmi_qos_granted_filter_data_type * filter = NULL;
  unsigned int i;

  NETMGR_ASSERT( flow_ptr );

  NETMGR_LOG_FUNC_ENTRY;

  /* Check if this suspend is for a new flow.  For new flows, need
   * to decode QoS specification parameters.  For existing flows,
   * this data has previously been received. */
  if( flow_ptr->qos_flow_state.new_flow )
  {

    /* Check for QoS TX flow specification */
    if( flow_ptr->tx_granted_flow_data_is_valid )
    {
      flow = &flow_ptr->tx_granted_flow_data.qos_flow_granted;

      /* Check for UMTS parameter set */
      if( flow->umts_flow_desc.param_mask )
      {
        if( NETMGR_SUCCESS != netmgr_qos_flow_decode_umts_params(&flow->umts_flow_desc,
                                                                 &priority,
                                                                 &datarate))
        {

          netmgr_log_err( "Error decoding UMTS TX flow data!\n");
          return;
        }
      }
      /* Check for CDMA parameter set */
      else if( flow->cdma_flow_desc.param_mask )
      {
        if( NETMGR_SUCCESS != netmgr_qos_flow_decode_cdma_params(&flow->cdma_flow_desc,
                                                                 &priority,
                                                                 &datarate))
        {

          netmgr_log_err( "Error decoding CDMA TX flow data!\n");
          return;
        }
      }
      else
      {
        netmgr_log_err( "TX flow data undefined, using defaults!\n");
      }
    }
    else
    {
      /* No specification available, assume default values */
      netmgr_log_err( "TX flow data undefined, using defaults!\n");
    }
  }

  /* Allocate a command object */
  cmd = netmgr_exec_get_cmd();
  NETMGR_ASSERT(cmd);

  /* Set command object parameters */
  cmd->data.type                                = NETMGR_QOS_SUSPEND_EV;
  cmd->data.link                                = link;

  cmd->data.info.qos_flow.flow_id   = (uint32)flow_ptr->qos_flow_state.qos_identifier;
  cmd->data.info.qos_flow.is_new    = flow_ptr->qos_flow_state.new_flow;
  cmd->data.info.qos_flow.flow_type = (uint8)((QMI_QOS_EVENT_REPORT_FLOW_INFO_FLOW_TYPE_PARAM &
                                       flow_ptr->param_mask)?
                                       flow_ptr->flow_type : NETMGR_FLOW_TYPE_INVALID);
  cmd->data.info.qos_flow.priority  = priority;
  cmd->data.info.qos_flow.datarate  = datarate;

  cmd->data.info.qos_flow.num_filter = (uint8)flow_ptr->tx_filter_count;
  memcpy((void*)cmd->data.info.qos_flow.filter_list,
         flow_ptr->tx_granted_filter_data,
         sizeof(cmd->data.info.qos_flow.filter_list));

  /* Post command for processing in the command thread context */
  if(NETMGR_SUCCESS != netmgr_exec_put_cmd(cmd))
  {
    NETMGR_ABORT("netmgr_qos_flow_activated: failed to put command\n");
    netmgr_exec_release_cmd(cmd);
  }

  NETMGR_LOG_FUNC_EXIT;
  return;
}

/*===========================================================================
  FUNCTION  netmgr_qos_flow_control
===========================================================================*/
/*!
@brief
  Performs processing of an QoS flow control notification.

@return
  void

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
LOCAL void
netmgr_qos_flow_control
(
  int                                   link,
  qmi_qos_event_report_flow_info_type * flow_ptr,
  global_flow_state_change              state_change
)
{
  netmgr_exec_cmd_t * cmd = NULL;
  NETMGR_LOG_FUNC_ENTRY;

  NETMGR_ASSERT( flow_ptr );

  // Allocate cmd object
  cmd = netmgr_exec_get_cmd();
  NETMGR_ASSERT(cmd);
  cmd->data.type = NETMGR_QOS_FLOCNTRL_EV;
  cmd->data.link = link;
  cmd->data.info.qos_flow.flow_id = (uint32)flow_ptr->qos_flow_state.qos_identifier;
  cmd->data.info.qos_flow.flow_type = (uint8)((QMI_QOS_EVENT_REPORT_FLOW_INFO_FLOW_TYPE_PARAM &
                                       flow_ptr->param_mask)?
                                      flow_ptr->flow_type : NETMGR_FLOW_TYPE_INVALID);
  cmd->data.info.qos_flow.state = (QMI_QOS_FLOW_DISABLED == state_change)?
                                  NETMGR_TC_FLOW_DISABLED : NETMGR_TC_FLOW_ACTIVE;

  // Post command
  if (NETMGR_SUCCESS != netmgr_exec_put_cmd(cmd))
  {
    NETMGR_ABORT("netmgr_qos_flow_suspended: failed to put command");
    netmgr_exec_release_cmd(cmd);
  }

  NETMGR_LOG_FUNC_EXIT;
  return;
}
#endif /* NETMGR_QOS_ENABLED */

/*===========================================================================
  FUNCTION  netmgr_qos_event_report_ind
===========================================================================*/
/*!
@brief
  Performs processing of an incoming QOS Event Report Indication message.

@return
  void

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
LOCAL void
netmgr_qos_event_report_ind
(
  int                            link,
  qmi_qos_indication_data_type * ind_data
)
{
  qmi_qos_event_report_type    *event = NULL;
  int i=0;

  NETMGR_ASSERT( ind_data );
  event = &ind_data->event_report;
  (void)link;

  NETMGR_LOG_FUNC_ENTRY;

#ifdef NETMGR_QOS_ENABLED
  /* Verify the flow info is present */
  if( event->param_mask & QMI_QOS_EVENT_REPORT_GLOBAL_FLOW_INFO_PARAM )
  {
    /* Loop over list of flows */
    netmgr_log_med( "Processing event reports - num_flows[%d]", event->num_flows );

    for( i=0; i < event->num_flows; i++ )
    {
      if( 0 ==
          (QMI_QOS_EVENT_REPORT_FLOW_INFO_FLOW_STATE_PARAM & event->flow_info[i].param_mask) )
      {
        netmgr_log_err( "Flow state not present, skipping[%d]\n", i);
        continue;
      }

      /* Process the QoS flow event */
      switch( event->flow_info[i].qos_flow_state.report_flow_state_chng ) {

        case QMI_QOS_FLOW_ACTIVATED:
          netmgr_qos_flow_activated( link,
                                     &event->flow_info[i]);
          break;
        case QMI_QOS_FLOW_MODIFIED:
          netmgr_qos_flow_modified( link, &event->flow_info[i] );
          break;
        case QMI_QOS_FLOW_DELETED:
          netmgr_qos_flow_deleted( link, &event->flow_info[i] );
          break;
        case QMI_QOS_FLOW_SUSPENDED:
          netmgr_qos_flow_suspended( link, &event->flow_info[i] );
          break;

        case QMI_QOS_FLOW_ENABLED:
        case QMI_QOS_FLOW_DISABLED:
          if (!netmgr_main_get_ibfc_enabled())
          {
             netmgr_qos_flow_control( link, &event->flow_info[i],
                                      event->flow_info[i].qos_flow_state.report_flow_state_chng );
          }
          break;

        default:
          netmgr_log_err("unsupported QoS flow event: %d\n",
                         event->flow_info[i].qos_flow_state.report_flow_state_chng );
          break;
      }
    }
  }
#endif /* NETMGR_QOS_ENABLED */

  NETMGR_LOG_FUNC_EXIT;
  return;
}

/*===========================================================================
  FUNCTION  netmgr_qos_ind
===========================================================================*/
/*!
@brief
 Performs processing of an incoming QOS Indication message. This function
 is executed in the Command Thread context.

@return
  void

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
LOCAL void
netmgr_qos_ind
(
  int                                  link,
  qmi_qos_indication_id_type           ind_id,
  qmi_qos_indication_data_type       * ind_data
)
{
  NETMGR_LOG_FUNC_ENTRY;

  /* Verify link id is valid before proceeding */
  NETMGR_ASSERT(netmgr_qmi_verify_link(link) == NETMGR_SUCCESS);

  /* Process based on indication type */
  switch (ind_id) {
    case QMI_QOS_SRVC_EVENT_REPORT_IND_MSG:
      /* Process Event Report indication */
      netmgr_qos_event_report_ind( link, ind_data );
      break;

    default:
      /* Ignore all other indications */
      netmgr_log_high("Ignoring QMI QOS IND of type %d\n", ind_id);
      break;
  }

  NETMGR_LOG_FUNC_EXIT;
  return;
}


/*===========================================================================
  FUNCTION  netmgr_qmi_cmd_exec
===========================================================================*/
/*!
@brief
 Virtual function registered with the Command Thread to executes a QMI WDS
 Interface command.

@return
  void

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
LOCAL int
netmgr_qmi_cmd_exec (ds_cmd_t * cmd, void * data)
{
  netmgr_qmi_cmd_t * qmi_cmd;
  (void)cmd;

  /* Get qmi cmd ptr from user data ptr */
  qmi_cmd = (netmgr_qmi_cmd_t *)data;
  NETMGR_ASSERT( qmi_cmd );

  /* Process based on command type */
  switch( qmi_cmd->type ) {
    case NETMGR_QMI_SYS_IND_CMD:
      netmgr_sys_ind( qmi_cmd->data.sys_ind.event_id,
                      &qmi_cmd->data.sys_ind.event_info);
      break;
    case NETMGR_QMI_WDS_IND_CMD:
      netmgr_wds_ind( &qmi_cmd->data.wds_ind );
      break;

    case NETMGR_QMI_QOS_EVENT_REPORT_IND_CMD:
      netmgr_qos_ind( qmi_cmd->data.qos_ind.link,
                      qmi_cmd->data.qos_ind.ind_id,
                      &qmi_cmd->data.qos_ind.info );
      break;

  case NETMGR_QMI_DFS_IND_CMD:
      netmgr_qmi_dfs_process_ind( qmi_cmd->data.dfs_ind.link,
                                  qmi_cmd->data.dfs_ind.ind_id,
                                  &qmi_cmd->data.dfs_ind.info );
      break;

    default:
      netmgr_log_err("received unknown command type %d\n", qmi_cmd->type);
      return NETMGR_FAILURE;
  }

  return NETMGR_SUCCESS;
}

/*===========================================================================
  FUNCTION  netmgr_qmi_get_flow_info
===========================================================================*/
/*!
@brief
  Function to query QoS flow info via QMI Message Library

@return
  int - NETMGR_SUCCESS on successful operation, NETMGR_FAILURE otherwise

@note

  - Dependencies
    - netmgr_qmi_init() must have been called

  - Side Effects
    - None
*/
/*=========================================================================*/
int netmgr_qmi_get_flow_info
(
  uint8     link,
  uint32    flow_id,
  uint8   * priority,
  uint32  * datarate
)
{
#ifdef NETMGR_QOS_ENABLED
  qmi_qos_granted_info_rsp_type granted_info;
  qmi_client_handle_type client_hndl;
  qmi_qos_flow_req_type *flow;
  int err_code;
  int result;

  NETMGR_LOG_FUNC_ENTRY;

  /* Use IPV4 family handle for flow query */
  if( NETMGR_QMI_CLIENT_INVALID ==
      netmgr_qmi_cfg.links[link].qos_info.clnt_hdl[NETMGR_QMI_CLIENT_IPV4] )
  {
    netmgr_log_err( "QMI QOS client not initalized \n");
    return NETMGR_FAILURE;
  }
  client_hndl = netmgr_qmi_cfg.links[link].qos_info.clnt_hdl[NETMGR_QMI_CLIENT_IPV4];

  /* Check for primary or secondary flow */
  if( NETMGR_IS_DEFAULT_FLOW( flow_id ) ) {
    result = qmi_qos_get_primary_granted_qos_info( client_hndl,
                                                   flow_id,
                                                   &granted_info,
                                                   &err_code );
    /* Query primary flow */
    if( QMI_NO_ERR != result ) {
      netmgr_log_err("failed on qmi_qos_get_"
                     "primary_granted_qos_info, ret=%d err=%d\n",
                     result, err_code);
      return NETMGR_FAILURE;
    }
  }
  else {
    /* Query secondary flow */
    result = qmi_qos_get_secondary_granted_qos_info( client_hndl,
                                                     flow_id,
                                                     &granted_info,
                                                     &err_code );
    if( QMI_NO_ERR != result ) {
      netmgr_log_err("failed on qmi_qos_get_"
                     "secondary_granted_qos_info, ret=%d err=%d\n",
                     result, err_code);
      return NETMGR_FAILURE;
    }
  }

  /* Check for QoS TX flow specification */
  if( granted_info.tx_granted_flow_data_is_valid ) {
    flow = &granted_info.tx_granted_flow_data.qos_flow_granted;

    /* Check for UMTS parameter set */
    if( flow->umts_flow_desc.param_mask ) {
      if( NETMGR_SUCCESS !=
          netmgr_qos_flow_decode_umts_params( &flow->umts_flow_desc,
                                              priority,
                                              datarate ) ) {

        netmgr_log_err( "Error decoding UMTS TX flow data!\n");
        return NETMGR_FAILURE;
      }
    }
    /* Check for CDMA parameter set */
    else if( flow->cdma_flow_desc.param_mask ) {
      if( NETMGR_SUCCESS !=
          netmgr_qos_flow_decode_cdma_params( &flow->cdma_flow_desc,
                                              priority,
                                              datarate ) ) {

        netmgr_log_err( "Error decoding CDMA TX flow data!\n");
        return NETMGR_FAILURE;
      }
    }
    else {
      netmgr_log_err( "TX flow data undefined!\n");
      return NETMGR_FAILURE;
    }
  }

  NETMGR_LOG_FUNC_EXIT;

  return NETMGR_SUCCESS;

#else
  (void)link; (void)flow_id; (void)priority; (void)datarate;
  netmgr_log_err( "Operation not supported!\n");
  return NETMGR_FAILURE;
#endif /* NETMGR_QOS_ENABLED */
}


/*===========================================================================
  FUNCTION  netmgr_qmi_wds_service_init
===========================================================================*/
/*!
@brief
  Initializes the QMI WDS services for a specific link.

@return
  int - NETMGR_SUCCESS on successful operation, NETMGR_FAILURE otherwise.

@note

  - Dependencies
    - netmgr_qmi_driver_init() must have been invoked during powerup.

  - Side Effects
    - None
*/
/*=========================================================================*/
LOCAL int
netmgr_qmi_wds_service_init
(
  int                     link,
  char                  * qmi_cid,
  qmi_ip_family_pref_type ip_family
)
{
  int                                     ret = NETMGR_FAILURE;
  int                                     qmi_err;
  int                                     wds_clnt_id;
  netmgr_qmi_client_type_t                clnt;
  qmi_wds_bind_mux_data_port_params_type  bind_params;
  qmi_wds_indication_reg_req_type         ext_ip_req;
  int                                     rc = QMI_NO_ERR;


  NETMGR_LOG_FUNC_ENTRY;

  /* Initialize WDS service client */
  if ((wds_clnt_id =
       qmi_wds_srvc_init_client( qmi_cid,
                                 netmgr_qmi_wds_ind,
                                 (void *)(intptr_t)link,
                                 &qmi_err ) ) < 0) {
    netmgr_log_err("netmgr_qmi_wds_service_init: qmi_wds_srvc_init_client failed "
                   "for link %d qmi_cid %s with error %ld,\n",
                   link, qmi_cid, (long int)qmi_err);
    goto error;
  }
  else
  {
    if( QMI_NO_ERR !=
        qmi_wds_set_client_ip_pref( wds_clnt_id,
                                    ip_family,
                                    &qmi_err ) ) {
      netmgr_log_err("netmgr_qmi_wds_service_init: qmi_wds_set_client_ip_pref failed "
                     "for link %d qmi_cid %s with error %ld,\n",
                     link, qmi_cid, (long int)qmi_err);
    }

/* TODO: Implement qmi_wds_indication_register on off-target then remove this check */
#ifndef NETMGR_OFFTARGET
    /* Register for extended IP config change indication */
    memset(&ext_ip_req, 0, sizeof(ext_ip_req));

    ext_ip_req.param_mask = QMI_WDS_EXT_IP_CONFIG_CHANGE_IND_REG_LIST_PARAM_MASK;
    ext_ip_req.ext_ip_pref = QMI_WDS_EXT_IP_CONFIG_CHANGE_REPORT;

    if (QMI_NO_ERR != (rc = qmi_wds_indication_register(wds_clnt_id,
                                                        ext_ip_req,
                                                        &qmi_err)))
    {
      netmgr_log_high( "Failed to register for ext IP config ind on "
                       "IPv%d wds_handle 0x%08x, rc=%d, qmi_err=%d",
                       ip_family,
                       wds_clnt_id,
                       rc,
                       qmi_err );
    }
    else
    {
      netmgr_log_high("Successfully registered for ext IP config "
                      "ind on IPv%d wds_handle 0x%08x",
                      ip_family,
                      wds_clnt_id );
    }
#endif
  /* Always bind NETMGR wds client to DONT CARE Subs */
    if (QMI_NO_ERR != qmi_wds_bind_subscription ( wds_clnt_id,
                                                  0x00FF,
                                                  &qmi_err))
    {
      netmgr_log_err("netmgr_qmi_wds_service_init: subs_id binding failed");
    }

#ifdef FEATURE_DATA_IWLAN
    /* Register for reverse IP transport indications. */
    if (NETMGR_IS_REV_IP_TRANS_CONN_ID(qmi_cid))
    {
      if (QMI_NO_ERR !=
          qmi_wds_rev_ip_transport_conn_ind_registration(wds_clnt_id,
                                                         QMI_WDS_REV_IP_TRANSPORT_CONN_IND_REGISTER,
                                                         &qmi_err))
      {
        netmgr_log_err("netmgr_qmi_wds_service_init: qmi_wds_rev_ip_transport_conn_ind_registration failed "
                       "for link %d qmi_cid %s with error %ld,\n",
                       link, qmi_cid, (long int)qmi_err);
        goto error;
      }
    }
    else if(TRUE == netmgr_main_get_iwlan_enabled())
    {
      qmi_wds_event_report_params_type  wds_event_report;

      /* Register for WDS bearer_tech_ex events. */
      memset(&wds_event_report, 0, sizeof(wds_event_report));

      wds_event_report.param_mask = QMI_WDS_EVENT_BEARER_TECH_EX_IND;
      wds_event_report.report_bearer_tech_ex = TRUE;

      if( QMI_NO_ERR !=
          qmi_wds_set_event_report( wds_clnt_id,
                                    &wds_event_report,
                                    &qmi_err ) )
      {
        netmgr_log_high("netmgr_qmi_wds_service_init: qmi_wds_set_event_report failed "
                        "for link %d ip_family %d qmi_cid %s with error %ld,\n",
                        link, ip_family, qmi_cid, (long int)qmi_err);
      }
    }
#endif /* FEATURE_DATA_IWLAN */

    clnt = netmgr_qmi_convert_ip_pref_to_client_type(ip_family);

    /* Save WDS client ID in internal state */
    netmgr_qmi_cfg.links[ link ].wds_info.clnt_hdl[clnt] = wds_clnt_id;
    ret = NETMGR_SUCCESS;
  }

error:
  NETMGR_LOG_FUNC_EXIT;

  return ret;
}

/*===========================================================================
  FUNCTION  netmgr_qmi_qos_service_init
===========================================================================*/
/*!
@brief
  Initializes the QMI QOS services for a specific link.

@return
  int - NETMGR_SUCCESS on successful operation, NETMGR_FAILURE otherwise.

@note

  - Dependencies
    - netmgr_qmi_driver_init() must have been invoked during powerup.

  - Side Effects
    - None
*/
/*=========================================================================*/
LOCAL int
netmgr_qmi_qos_service_init
(
  int                     link,
  char                  * qmi_cid,
  qmi_ip_family_pref_type ip_family
)
{
  int ret = NETMGR_FAILURE;
#ifdef NETMGR_QOS_ENABLED
  int qmi_err;
  int qos_clnt_id;
  netmgr_qmi_client_type_t clnt = netmgr_qmi_convert_ip_pref_to_client_type(ip_family);
  qmi_qos_event_report_state_type   qos_report_state;

  NETMGR_LOG_FUNC_ENTRY;

  /* Initialize QOS service clients */
  if ((qos_clnt_id =
       qmi_qos_srvc_init_client( qmi_cid,
                                 netmgr_qmi_qos_ind,
                                 (void *)(intptr_t)link,
                                 &qmi_err ) ) < 0) {
    if( (QMI_SERVICE_ERR == qos_clnt_id) &&
        (QMI_SERVICE_ERR_INVALID_SERVICE_TYPE == qmi_err) ) {
      netmgr_log_err("netmgr_qmi_qos_service_init: QOS not supported on Modem, suppressing\n");
      /* Clear QOS state variables */
      netmgr_main_cfg.runmode &= ~NETMGR_MAIN_RUNMODE_QOSHDR;
      netmgr_qmi_cfg.links[ link ].qos_info.clnt_hdl[clnt] = NETMGR_QMI_CLIENT_INVALID;
      /* Treat as warning to keep link active */
      ret = NETMGR_SUCCESS;
    } else {
      netmgr_log_err("netmgr_qmi_qos_service_init: qmi_qos_srvc_init_client failed "
                     "for link %d qmi_cid %s with error %ld,\n",
                     link, qmi_cid, (long int)qmi_err);
    }
    goto error;
  }
  else
  {
    if( QMI_NO_ERR !=
        qmi_qos_set_client_ip_pref( qos_clnt_id,
                                    ip_family,
                                    &qmi_err ) ) {
      netmgr_log_err("netmgr_qmi_qos_service_init: qmi_qos_set_client_ip_pref failed "
                     "for link %d qmi_cid %s with error %ld,\n",
                     link, qmi_cid, (long int)qmi_err);
      goto error;
    }

  /* Always bind NETMGR wds client to DONT CARE Subs */
    if (QMI_NO_ERR != qmi_qos_bind_subscription ( qos_clnt_id,
                                                  0x00FF,
                                                  &qmi_err))
    {
      netmgr_log_err("netmgr_qmi_wds_service_init: subs_id binding failed");
    }

    /* Register for QoS flow event reporting. */
    qos_report_state.param_mask = QMI_QOS_GLOBAL_FLOW_REPORTING_STATE_PARAM;
    qos_report_state.flow_state = QMI_QOS_EVENT_REPORTING_ON;
    if( QMI_NO_ERR !=
        qmi_qos_set_event_report_state( qos_clnt_id,
                                        &qos_report_state,
                                        &qmi_err ) ) {
      netmgr_log_err("netmgr_qmi_qos_service_init: qmi_qos_set_event_report_state failed "
                     "for link %d qmi_cid %s with error %ld,\n",
                     link, qmi_cid, (long int)qmi_err);
      goto error;
    }

    /* Save QOS client ID in internal state */
    netmgr_qmi_cfg.links[ link ].qos_info.clnt_hdl[clnt] = qos_clnt_id;
    ret = NETMGR_SUCCESS;
  }

error:
  NETMGR_LOG_FUNC_EXIT;
#endif /* NETMGR_QOS_ENABLED */

  return ret;
}

#ifdef FEATURE_DATA_IWLAN
/*===========================================================================
  FUNCTION  netmgr_qmi_dfs_service_init
===========================================================================*/
/*!
@brief
  Initializes the QMI DFS services for a specific link.

@return
  int - NETMGR_SUCCESS on successful operation, NETMGR_FAILURE otherwise.

@note

  - Dependencies
    - netmgr_qmi_driver_init() must have been invoked during powerup.

  - Side Effects
    - None
*/
/*=========================================================================*/
LOCAL int
netmgr_qmi_dfs_service_init
(
  int                     link,
  char                  * qmi_cid,
  qmi_ip_family_pref_type ip_family
)
{
  int ret = NETMGR_FAILURE;
  netmgr_qmi_client_type_t clnt;

  clnt = netmgr_qmi_convert_ip_pref_to_client_type(ip_family);

  if (NETMGR_SUCCESS == netmgr_qmi_dfs_init_qmi_client(link,
                                                       qmi_cid,
                                                       ip_family,
                                                       &netmgr_qmi_cfg.links[ link ].dfs_info.clnt_hdl[clnt]))
  {
    dfs_indication_register_req_msg_v01 req;
    dfs_indication_register_resp_msg_v01 resp;
    int rc;

    memset (&req, 0, sizeof(req));
    memset (&resp, 0, sizeof(resp));

    req.report_reverse_ip_transport_filters_update_valid = TRUE;
    req.report_reverse_ip_transport_filters_update = TRUE;

    /* Send indication registration request */
    rc = qmi_client_send_msg_sync(netmgr_qmi_cfg.links[ link ].dfs_info.clnt_hdl[clnt],
                                  QMI_DFS_INDICATION_REGISTER_REQ_V01,
                                  (void *)&req,
                                  sizeof(req),
                                  (void*)&resp,
                                  sizeof(resp),
                                  NETMGR_QMI_TIMEOUT);

    netmgr_log_high("The rev ip filter update registration link[%d] ip_family[%d] status[%s]\n",
                    link,
                    ip_family,
                    (rc == QMI_NO_ERR && resp.resp.result == QMI_RESULT_SUCCESS_V01) ? "SUCCESS" : "FAIL");
  }
  else
  {
    netmgr_log_err("failed to allocate dfs client on link[%d]\n",
                   link);
    goto bail;
  }

  ret = NETMGR_SUCCESS;

bail:
  return ret;
}
#endif /* FEATURE_DATA_IWLAN */

/*===========================================================================
  FUNCTION  netmgr_qmi_set_data_format
===========================================================================*/
/*!
@brief
  Sets the WDA data format for the link


@return
  NETMGR_SUCCESS - if successful
  NETMGR_ERROR - if any errors occurred during sending WDA message.

*/
/*=========================================================================*/
LOCAL int
netmgr_qmi_set_data_format
(
  char *dev_name,
  netmgr_data_format_t data_format
)
{
  unsigned int ingress_flags = 0;
  unsigned int dl_minimum_padding = 0;
  wda_set_data_format_req_msg_v01 wda_request;
  wda_set_data_format_resp_msg_v01 wda_response;
  int ret_val = NETMGR_WDA_BAD_ARGUMENTS;

  memset(&wda_request, 0, sizeof(wda_set_data_format_req_msg_v01));

  if (netmgr_main_get_qos_enabled() & (data_format.qos_header_format_valid))
  {
    wda_request.qos_format_valid = TRUE;
    wda_request.qos_format = (uint8_t)data_format.qos_format;

    wda_request.qos_header_format_valid = (uint8_t)data_format.qos_header_format_valid;
    wda_request.qos_header_format = data_format.qos_header_format;
  }

  if ((netmgr_main_get_qos_enabled()) &&
      ((wda_request.qos_format_valid == FALSE) ||
      (wda_request.qos_format == 0)) &&
      (!netmgr_main_get_ibfc_enabled()))
  {
    wda_request.flow_control_valid = TRUE;
    wda_request.flow_control = TRUE;
  }

  wda_request.link_prot_valid = TRUE;
  wda_request.link_prot = data_format.link_prot;

  if (data_format.ul_data_aggregation_protocol == NETMGR_WDA_UL_QMAP)
  {
    wda_request.ul_data_aggregation_protocol_valid = TRUE;
    wda_request.ul_data_aggregation_protocol =
      data_format.ul_data_aggregation_protocol;
  }

  if (data_format.dl_data_aggregation_protocol == NETMGR_WDA_DL_QMAP
        || data_format.dl_data_aggregation_protocol == NETMGR_WDA_DL_QMAPV3)
  {
    wda_request.dl_data_aggregation_protocol_valid = TRUE;
    wda_request.dl_data_aggregation_protocol =
      data_format.dl_data_aggregation_protocol;

    wda_request.dl_data_aggregation_max_size_valid = TRUE;
    wda_request.dl_data_aggregation_max_size = (uint32_t)data_format.dl_agg_size;

    wda_request.dl_data_aggregation_max_datagrams_valid = TRUE;
    wda_request.dl_data_aggregation_max_datagrams = (uint32_t)data_format.dl_agg_cnt;

    if (data_format.dl_agg_cnt > 1)
    {
      wda_request.dl_minimum_padding_valid = TRUE;
      wda_request.dl_minimum_padding = (uint8_t)data_format.dl_agg_pad;
    }
  }

  if ((ret_val = netmgr_wda_set_data_format(dev_name,
                                            &wda_request,
                                            &wda_response)) != QMI_NO_ERR)
  {
    netmgr_log_err("%s() WDA Set Data Format Failed on (%s)", __func__, dev_name);

    if ((wda_request.flow_control == TRUE) &&
        (wda_response.flow_control != wda_request.flow_control))
    {
      netmgr_log_err("%s() Modem does not support flow_control on (%s)",
                      __func__,
                      dev_name);
    }

    if ((netmgr_main_get_qos_enabled()) &&
        (netmgr_main_get_rmnet_data_enabled()) &&
        (ret_val == NETMGR_WDA_MODEM_REJECTED) &&
        (wda_request.qos_header_format_valid))
    {
      netmgr_log_high("%s() request.qos_header_format is %d, response.qos_header_format is %d",
                        __func__,
                        wda_request.qos_header_format,
                        wda_response.qos_header_format);
      if ((wda_request.qos_header_format == NETMGR_QMI_QOS_HEADER_8_BYTE) &&
          (wda_response.qos_header_format == NETMGR_QMI_QOS_HEADER_6_BYTE))
      {
        netmgr_log_high("%s() Modem is using qos_header_format size 6 on (%s)",
                        __func__,
                        dev_name);
      }
      /* If modem fails does not support qos_header_format to set 8 byte QoS, revert to
         6 byte QoS which is the default */
      if ((wda_request.qos_header_format_valid) && (!(wda_response.qos_header_format_valid)))
      {
        netmgr_log_high("%s() Modem does not support qos_header_format on (%s)",
                        __func__,
                        dev_name);
        data_format.qos_header_format = NETMGR_QMI_QOS_HEADER_6_BYTE;
      }
      if ((wda_request.qos_header_format == NETMGR_QMI_QOS_HEADER_6_BYTE) &&
          (wda_response.qos_header_format == NETMGR_QMI_QOS_HEADER_8_BYTE))
      {
        netmgr_log_high("%s() Modem is using qos_header_format size 6 ,%s on (%s)",
                        __func__,
                        "qos_header_format to be used is size 8",
                        dev_name);
        return NETMGR_FAILURE;
      }

    }
  }

  if (wda_response.dl_minimum_padding == wda_request.dl_minimum_padding)
  {
    dl_minimum_padding = wda_response.dl_minimum_padding;
    netmgr_log_high("%s() dl_minimum_padding on (%s) is %u",
                    __func__,
                    dev_name,
                    dl_minimum_padding);
  }
  else
  {
    netmgr_log_high("%s() Modem rejected dl_minimum_padding on (%s)",__func__,dev_name);
  }

  if ((wda_request.qos_header_format_valid) && (wda_response.qos_header_format_valid))
  {
    netmgr_log_high("%s() request qos_header_format is %d, response.qos_header_format is %d",
                    __func__,
                    wda_request.qos_header_format,
                    wda_response.qos_header_format);
    data_format.qos_header_format = wda_response.qos_header_format;
  }


  if (data_format.dl_agg_cnt > 1)
  {
    if (netmgr_rmnet_get_ingress_data_format((const char *)netmgr_main_get_phys_net_dev(),
                                             &ingress_flags) != NETMGR_SUCCESS)
    {
      netmgr_log_err("%s() Unable to get the ingress data format for (%s)", __func__, dev_name);
      return NETMGR_FAILURE;
    }

    ingress_flags = ingress_flags | RMNET_INGRESS_FORMAT_DEAGGREGATION;

    if (NETMGR_SUCCESS !=  netmgr_rmnet_set_feature_flags((const char *)netmgr_main_get_phys_net_dev(),
                                                          ingress_flags,
                                                          (unsigned int)NETMGR_RMNET_NO_FEATURE_FLAGS,
                                                          dl_minimum_padding))
    {
      netmgr_log_err("%s() setting of feature flags failed", __func__, dev_name);
      return NETMGR_FAILURE;
    }
  }

  return NETMGR_SUCCESS;
}

/*===========================================================================
  FUNCTION  netmgr_qmi_set_qmap_settings
===========================================================================*/
/*!
@brief
  Sets the QMAP settings for the link


@return
  NETMGR_SUCCESS - if successful
  NETMGR_ERROR - if any errors occurred during sending WDA message.

*/
/*=========================================================================*/
LOCAL int
netmgr_qmi_set_qmap_settings
(
  char *dev_name
)
{
  wda_set_qmap_settings_req_msg_v01 qmap_request;
  wda_set_qmap_settings_resp_msg_v01 qmap_response;
  memset(&qmap_request, 0, sizeof(wda_set_qmap_settings_req_msg_v01));
  memset(&qmap_response, 0, sizeof(wda_set_qmap_settings_resp_msg_v01));

  if (netmgr_main_get_ibfc_enabled())
  {
    qmap_request.in_band_flow_control_valid = TRUE;
    qmap_request.in_band_flow_control = TRUE;
  }

  if (QMI_NO_ERR != netmgr_wda_set_qmap_settings(dev_name, &qmap_request, &qmap_response))
  {
    netmgr_log_err("%s() WDA Set QMAP Settings Failed on (%s)", __func__, dev_name);
    return NETMGR_FAILURE;
  }

  return NETMGR_SUCCESS;
}

/*===========================================================================
  FUNCTION  netmgr_qmi_service_init
===========================================================================*/
/*!
@brief
  Initializes the QMI services for a specific link.

@return
  int - NETMGR_SUCCESS on successful operation, NETMGR_FAILURE otherwise.

@note

  - Dependencies
    - netmgr_qmi_driver_init() must have been invoked during powerup.

  - Side Effects
    - None
*/
/*=========================================================================*/
LOCAL int
netmgr_qmi_service_init( int link, netmgr_data_format_t data_format )
{
  char                               *qmi_cid;
  char                               *dev_str;
  int                                 qmi_err;
  int                                 wds_clnt_id;
  qmi_link_layer_protocol_type        ll_prot;
  qmi_data_format_qos_hdr_state_type  qos_hdr;

  NETMGR_LOG_FUNC_ENTRY;

  /* Get qmi connection id for the interface */
  qmi_cid = (char *)netmgr_qmi_get_conn_id_for_link( link );
  dev_str = (char *)netmgr_qmi_get_dev_str_for_link( link );

  if (qmi_cid == NULL)
  {
    netmgr_log_err("netmgr_qmi_service_init: failed. netmgr_qmi_get_conn_id_for_link returned qmi_cid as NULL\n");
    goto error;
  }

  if (dev_str == NULL)
  {
    netmgr_log_err("netmgr_qmi_service_init: failed. netmgr_qmi_get_dev_str_for_link returned dev_str as NULL\n");
    goto error;
  }

  if( !netmgr_qmi_cfg.link_array[link].enabled )
  {
    netmgr_log_err("%s(): QMI link=%d is disabled\n",
                   __func__,link);
    goto error;
  }

  if( netmgr_qmi_cfg.link_array[link].initialized == TRUE )
  {
    netmgr_log_err("%s(): QMI link=%d is already initialized\n",
                   __func__, link);
    return NETMGR_SUCCESS;
  }

  netmgr_log_med("%s", "initializing IPv4 WDS client");

  if( NETMGR_SUCCESS != netmgr_qmi_wds_service_init( link, qmi_cid, QMI_IP_FAMILY_PREF_IPV4 ) )
  {
    netmgr_log_err("netmgr_qmi_service_init: failed on WDS init, link[%d] IP family[%d]\n",
                   link, QMI_IP_FAMILY_PREF_IPV4);
    goto error;
  }

  /* Check for successful service registration */
  netmgr_log_med("%s", "initializing IPv6 WDS client");

  if( NETMGR_QMI_CLIENT_INVALID !=
      netmgr_qmi_cfg.links[ link ].wds_info.clnt_hdl[NETMGR_QMI_CLIENT_IPV4] )
  {
    if( NETMGR_SUCCESS != netmgr_qmi_wds_service_init( link, qmi_cid, QMI_IP_FAMILY_PREF_IPV6 ) )
    {
      netmgr_log_err("netmgr_qmi_service_init: failed on WDS init, link[%d] IP family[%d]\n",
                     link, QMI_IP_FAMILY_PREF_IPV6);
      goto error;
    }

    netmgr_log_high("The wds client ids [0x%08x/0x%08x]\n",
                    netmgr_qmi_cfg.links[ link ].wds_info.clnt_hdl[NETMGR_QMI_CLIENT_IPV4],
                    netmgr_qmi_cfg.links[ link ].wds_info.clnt_hdl[NETMGR_QMI_CLIENT_IPV6]);
  }

#ifdef FEATURE_DATA_IWLAN
  /* Initialize QoS and data format on non-reverse Rmnet ports */
  if (NETMGR_IS_REV_IP_TRANS_CONN_ID(qmi_cid))
  {
    if (NETMGR_SUCCESS != netmgr_qmi_dfs_service_init(link, qmi_cid, QMI_IP_FAMILY_PREF_IPV4))
    {
      netmgr_log_err("netmgr_qmi_service_init: failed on DFS init, link[%d] IP family[%d]\n",
                     link, QMI_IP_FAMILY_PREF_IPV4);
    }

    if (NETMGR_SUCCESS != netmgr_qmi_dfs_service_init(link, qmi_cid, QMI_IP_FAMILY_PREF_IPV6))
    {
      netmgr_log_err("netmgr_qmi_service_init: failed on DFS init, link[%d] IP family[%d]\n",
                     link, QMI_IP_FAMILY_PREF_IPV6);
    }

    netmgr_log_high("The dfs client ids [%p/%p]\n",
                    netmgr_qmi_cfg.links[ link ].dfs_info.clnt_hdl[NETMGR_QMI_CLIENT_IPV4],
                    netmgr_qmi_cfg.links[ link ].dfs_info.clnt_hdl[NETMGR_QMI_CLIENT_IPV6]);

    netmgr_log_low("netmgr_qmi_service_init: skipping data format and QoS init on cid=%s",
                   qmi_cid);
  }
  else
#endif /* FEATURE_DATA_IWLAN */
  {
    /* Initialize QOS service clients */
    if( netmgr_main_get_qos_enabled() )
    {
      if( NETMGR_SUCCESS != netmgr_qmi_qos_service_init( link, dev_str, QMI_IP_FAMILY_PREF_IPV4 ) )
      {
        netmgr_log_err("netmgr_qmi_service_init: failed on QOS init, link[%d] IP family[%d]\n",
                       link, QMI_IP_FAMILY_PREF_IPV4);
        goto error;
      }

      /* Check for successful service registration */
      if( NETMGR_QMI_CLIENT_INVALID !=
          netmgr_qmi_cfg.links[ link ].qos_info.clnt_hdl[NETMGR_QMI_CLIENT_IPV4] )
      {
        if( NETMGR_SUCCESS != netmgr_qmi_qos_service_init( link, dev_str, QMI_IP_FAMILY_PREF_IPV6 ) )
        {
          netmgr_log_err("netmgr_qmi_service_init: failed on QOS init, link[%d] IP family[%d]\n",
                         link, QMI_IP_FAMILY_PREF_IPV6);
          goto error;
        }

        netmgr_log_high("The qos client ids [0x%08x/0x%08x]\n",
                        netmgr_qmi_cfg.links[ link ].qos_info.clnt_hdl[NETMGR_QMI_CLIENT_IPV4],
                        netmgr_qmi_cfg.links[ link ].qos_info.clnt_hdl[NETMGR_QMI_CLIENT_IPV6]);
      }
    }
    /* Configure data format using either CTL or WDA message */
#ifdef NETMGR_OFFTARGET
  netmgr_main_cfg.wda_data_format = FALSE; // For now WDA data format is disabled.
#endif
    if (!netmgr_main_cfg.wda_data_format)
    {
      /* Configure data channel framing mode and QoS header state.
       * This must be done at device powerup. */
      ll_prot = (netmgr_main_cfg.runmode & NETMGR_MAIN_RUNMODE_ETHERNET)?
                QMI_DATA_FORMAT_LINK_PROTOCOL_ETHERNET : QMI_DATA_FORMAT_LINK_PROTOCOL_IP;
      qos_hdr = (netmgr_main_cfg.runmode & NETMGR_MAIN_RUNMODE_QOSHDR)?
                QMI_DATA_FORMAT_WITH_QOS_HDR : QMI_DATA_FORMAT_WITHOUT_QOS_HDR;

      netmgr_log_low("netmgr_qmi_service_init: setting data format: "
                     "qos_hdr:%d, ll_prot:%d\n", qos_hdr, ll_prot);

      if( QMI_NO_ERR !=
          qmi_set_port_data_format( qmi_cid,
                                    qos_hdr,
                                    &ll_prot,
                                    &qmi_err ) )
      {
        netmgr_log_err("netmgr_qmi_service_init: qmi_set_port_data_format failed "
                       "for link %d qmi_cid %s with error %ld,\n",
                       link, qmi_cid, (long int)qmi_err);
      }
    }
    else
    {
      if (NETMGR_SUCCESS != netmgr_qmi_set_data_format(qmi_cid, data_format))
      {
        netmgr_log_err("netmgr_qmi_service_init: netmgr_qmi_set_data_format failed"
                       "for link %d qmi_cid %s\n", link, qmi_cid);
      }
      if (netmgr_main_get_ibfc_enabled())
      {
        if (NETMGR_SUCCESS != netmgr_qmi_set_qmap_settings(qmi_cid))
        {
          netmgr_log_err("netmgr_qmi_service_init: netmgr_wda_set_qmap_setings failed"
                         "for link %d qmi_cid %s\n", link, qmi_cid);
        }
      }
    }
  }

#ifdef FEATURE_DATA_IWLAN
  netmgr_util_circ_list_init(&netmgr_qmi_cfg.links[ link ].wds_info.rev_ip_txns[NETMGR_QMI_CLIENT_IPV4]);
  netmgr_util_circ_list_init(&netmgr_qmi_cfg.links[ link ].wds_info.rev_ip_txns[NETMGR_QMI_CLIENT_IPV6]);
#endif /* FEATURE_DATA_IWLAN */

  /* Mark the link as initialized */
  netmgr_log_high("%s(): Marking link=%d as initialized\n",
                  __func__, link);
  netmgr_qmi_cfg.link_array[link].initialized = TRUE;

  NETMGR_LOG_FUNC_EXIT;
  return NETMGR_SUCCESS;

error:
  if (NETMGR_QMI_CLIENT_INVALID != netmgr_qmi_cfg.links[ link ].wds_info.clnt_hdl[NETMGR_QMI_CLIENT_IPV4])
  {
    qmi_wds_srvc_release_client( netmgr_qmi_cfg.links[ link ].wds_info.clnt_hdl[NETMGR_QMI_CLIENT_IPV4],
                                 &qmi_err );
    netmgr_qmi_cfg.links[ link ].wds_info.clnt_hdl[NETMGR_QMI_CLIENT_IPV4] = NETMGR_QMI_CLIENT_INVALID;
  }

  if (NETMGR_QMI_CLIENT_INVALID != netmgr_qmi_cfg.links[ link ].wds_info.clnt_hdl[NETMGR_QMI_CLIENT_IPV6])
  {
    qmi_wds_srvc_release_client( netmgr_qmi_cfg.links[ link ].wds_info.clnt_hdl[NETMGR_QMI_CLIENT_IPV6],
                                 &qmi_err );
    netmgr_qmi_cfg.links[ link ].wds_info.clnt_hdl[NETMGR_QMI_CLIENT_IPV6] = NETMGR_QMI_CLIENT_INVALID;
  }

  if (NETMGR_QMI_CLIENT_INVALID != netmgr_qmi_cfg.links[ link ].qos_info.clnt_hdl[NETMGR_QMI_CLIENT_IPV4])
  {
    qmi_qos_srvc_release_client( netmgr_qmi_cfg.links[ link ].qos_info.clnt_hdl[NETMGR_QMI_CLIENT_IPV4],
                                 &qmi_err );
    netmgr_qmi_cfg.links[ link ].qos_info.clnt_hdl[NETMGR_QMI_CLIENT_IPV4] = NETMGR_QMI_CLIENT_INVALID;
  }

  if (NETMGR_QMI_CLIENT_INVALID != netmgr_qmi_cfg.links[ link ].qos_info.clnt_hdl[NETMGR_QMI_CLIENT_IPV6])
  {
    qmi_qos_srvc_release_client( netmgr_qmi_cfg.links[ link ].qos_info.clnt_hdl[NETMGR_QMI_CLIENT_IPV6],
                                 &qmi_err );
    netmgr_qmi_cfg.links[ link ].qos_info.clnt_hdl[NETMGR_QMI_CLIENT_IPV6] = NETMGR_QMI_CLIENT_INVALID;
  }

  NETMGR_LOG_FUNC_EXIT;
  return NETMGR_FAILURE;
}

/*===========================================================================
  FUNCTION  netmgr_qmi_driver_init
===========================================================================*/
/*!
@brief
 Initializes the QMI Driver for list of specified links.

@return
  void

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
LOCAL void
netmgr_qmi_driver_init( netmgr_data_format_t data_format )
{
  int i;
  char * qmi_cid;
  int qmi_err, retry = 0;

  NETMGR_LOG_FUNC_ENTRY;

  do
  {
    qmi_handle = qmi_init( netmgr_qmi_sys_cb, NULL );

    if (qmi_handle < 0)
    {
      netmgr_log_err("Error initializing qmi message library... retry=%d", retry + 1);
      ++retry;
      usleep(NETMGR_QMI_WAIT_TIME_BEFORE_NEXT_RETRY);
    }
  } while (qmi_handle < 0 && retry < NETMGR_QMI_MAX_RETRY_COUNT);

  if (qmi_handle < 0)
  {
    NETMGR_STOP("Error initializing qmi message library");
  }

  /* Iterate over the array of links, initializing QMI service clients
   * for each */
  for (i = 0; i < NETMGR_MAX_LINK; ++i) {

    /* continue to next link if this link is not enabled
     *  i.e. we are not using default cfg, and dsc.cfg has
     *  not enabled this link */
    if( netmgr_qmi_cfg.link_array[i].enabled == FALSE ) {
      netmgr_log_med( "ignoring link[%d]\n", i );
      continue;
    }

    netmgr_log_high( "initing QMI link[%d]\n", i );

    /* Get qmi connection id for the interface */
    qmi_cid = (char *)netmgr_qmi_get_conn_id_for_link(i);
    netmgr_log_high("qmi_cid = %s\n", qmi_cid);
    NETMGR_ASSERT(qmi_cid != NULL);

    /* Initialize qmi connection */
    if (qmi_connection_init(qmi_cid, &qmi_err) < 0) {
      /* Suspend further processing on this link */
      netmgr_qmi_cfg.link_array[i].enabled = FALSE;
      netmgr_log_err( "Error on QMI connection init, "
                      "suppressing link[%d]\n", i );
      continue;
    }

    /* Perform QMI service init */
    if( NETMGR_SUCCESS != netmgr_qmi_service_init( i, data_format ) )
    {
      /* Mark this link uninitialized */
      netmgr_qmi_cfg.link_array[i].initialized = FALSE;
      netmgr_log_err( "Error on QMI service init, "
                      "marking link[%d] uninitialized\n", i );
      continue;
    }
  }

  /* Register process exit cleanup handler */
  atexit(netmgr_qmi_cleanup);

  NETMGR_LOG_FUNC_EXIT;
  return;
}

/*===========================================================================
  FUNCTION  netmgr_qmi_reset_connection
===========================================================================*/
/*!
@brief
  Reinitialize QMI connections on reset command.

@return
  int - NETMGR_SUCCESS on successful operation, NETMGR_FAILURE otherwise.

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
LOCAL int
netmgr_qmi_reset_connection
(
  int                 link,
  netmgr_sm_events_t  evt
)
{
  NETMGR_LOG_FUNC_ENTRY;

  netmgr_log_high( "initing QMI link[%d]\n", link );

  /* Perform QMI service init */
  if( NETMGR_SUCCESS != netmgr_qmi_service_init( link, netmgr_qmi_cfg.data_format ) )
  {
    /* Mark this link uninitialized */
    netmgr_qmi_cfg.link_array[link].initialized = FALSE;
    netmgr_log_err( "Error on QMI service init, "
                    "marking link[%d] uninitialized\n", link );
  }

  NETMGR_LOG_FUNC_EXIT;
  return NETMGR_SUCCESS;
}

/*===========================================================================
                            GLOBAL FUNCTION DEFINITIONS
===========================================================================*/


/*===========================================================================
  FUNCTION  netmgr_wds_query_profile
===========================================================================*/
/*!
@brief
  Queries Data Profile parameters for the specified profile ID.

@return
  void

@note

  - Dependencies
    - None

  - Side Effects
    - Blocks execution of thread until the QMI message exchange to get the
      profile parameters is complete.
*/
/*=========================================================================*/
int
netmgr_wds_query_profile
(
    qmi_wds_profile_id_type     * qmi_prof_id,
    qmi_wds_profile_params_type * qmi_prof_params
)
{
  int wds_clnt_id;
  int qmi_err;

  /* Get WDS client ID for the default link, i.e. link ID 0 */
  wds_clnt_id = netmgr_qmi_wds_get_clnt_id(0, NETMGR_QMI_CLIENT_IPV4);

  netmgr_log_high("Calling qmi_wds_query_profile!\n");

  /* Query profile parameters using the default link */
  if (qmi_wds_query_profile( wds_clnt_id,
                             qmi_prof_id,
                             qmi_prof_params,
                             &qmi_err ) != 0)
  {
    netmgr_log_err("qmi_wds_query_profile failed with error %ld\n", (long int)qmi_err);
    return -1;
  }
  else {
    netmgr_log_err("Returned from qmi_wds_query_profile!\n");
  }

  return 0;
}

/*===========================================================================
  FUNCTION  netmgr_qmi_get_ip_addr_type_first_conn_clnt
===========================================================================*/
/*!
@brief
  Retrives the IP address type for Modem network interface

@return
  netmgr_ip_addr_t - IP address type,
                     NETMGR_IP_ADDR_INVALID if undefined

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
netmgr_ip_addr_t netmgr_qmi_get_ip_addr_type_first_conn_clnt( int link )
{
  netmgr_ip_addr_t rc = NETMGR_IP_ADDR_INVALID;

  if( NETMGR_SUCCESS == netmgr_qmi_verify_link( link ) )
  {
    switch( netmgr_qmi_cfg.links[ link ].wds_info.first_conn_clnt )
    {
      case NETMGR_QMI_CLIENT_IPV4:
        rc = NETMGR_IPV4_ADDR;
        break;
      case NETMGR_QMI_CLIENT_IPV6:
        rc = NETMGR_IPV6_ADDR;
        break;
      default:
        netmgr_log_err( "netmgr_qmi_get_ip_addr_type_first_conn_clnt unsupported value[%d]\n",
                        netmgr_qmi_cfg.links[ link ].wds_info.first_conn_clnt );
        break;
    }
  }
  return rc;
}

/*===========================================================================
  FUNCTION  netmgr_qmi_get_addr_info
===========================================================================*/
/*!
@brief
  Retrives the network address info for Modem netwok interface

@return
  netmgr_address_info_t * - IP address info struct pointer,
                            NULL if undefined

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
netmgr_address_info_t * netmgr_qmi_get_addr_info( int link )
{
  if( NETMGR_SUCCESS == netmgr_qmi_verify_link( link ) )
    return &netmgr_qmi_cfg.links[ link ].wds_info.addr_info;
  else
    return NULL;
}

#ifdef FEATURE_DATA_IWLAN
/*===========================================================================
  FUNCTION  netmgr_qmi_get_ipsec_tunnel_endpoints
===========================================================================*/
/*!
@brief
  Retrives the tunnel endpoint addresses for a given link and ip family

@return
  netmgr_address_info_t * - IP address info struct pointer,
                            NULL if undefined

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int netmgr_qmi_get_ipsec_tunnel_endpoints
(
  int link,
  int ip_family,
  const char **local,
  const char **dest,
  int *tunnel_family
)
{
  int ret = NETMGR_FAILURE;

  if( NETMGR_SUCCESS == netmgr_qmi_verify_link( link ) )
  {
    netmgr_ipsec_sa_t *sa = (AF_INET == ip_family) ? &netmgr_qmi_cfg.links[ link ].wds_info.addr_info.ipv4.sa :
                                                     &netmgr_qmi_cfg.links[ link ].wds_info.addr_info.ipv6.sa;

    if (TRUE == sa->tunnel_ep.is_valid)
    {
      *local = sa->tunnel_ep.local_addr;
      *dest = sa->tunnel_ep.dest_addr;
      *tunnel_family = sa->tunnel_ep.ip_family;
      ret = NETMGR_SUCCESS;
    }
    else
    {
      netmgr_log_err("%s(): tunnel endpoint addresses invalid for link=%d, ip_family=%d\n",
                     __func__,
                     link,
                     ip_family);
    }
  }

  return ret;
}
#endif /* FEATURE_DATA_IWLAN */

/*===========================================================================
  FUNCTION  netmgr_qmi_get_mtu
===========================================================================*/
/*!
@brief
  Retrives the link MTU for Modem network interface

@return
 unsigned int - MTU value, NETMGR_MTU_INVALID if undefined

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
unsigned int netmgr_qmi_get_mtu( int link )
{
  if( NETMGR_SUCCESS == netmgr_qmi_verify_link( link ) )
    return (unsigned int)netmgr_qmi_cfg.links[ link ].wds_info.mtu;
  else
    return NETMGR_MTU_INVALID;
}


/*===========================================================================
  FUNCTION  netmgr_qmi_reset_link_wds_data
===========================================================================*/
/*!
@brief
  Reset the QMI WDS state information associated with the given link

@return
  None

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
void
netmgr_qmi_reset_link_wds_data
(
  int link
)
{
  if (NETMGR_SUCCESS == netmgr_qmi_verify_link( link ))
  {
    memset(&netmgr_qmi_cfg.links[ link ].wds_info.addr_info,
           0,
           sizeof(netmgr_qmi_cfg.links[ link ].wds_info.addr_info));

    netmgr_qmi_cfg.links[ link ].wds_info.mtu = NETMGR_MTU_INVALID;
    netmgr_qmi_cfg.links[ link ].wds_info.tech_name = QMI_WDS_IFACE_NAME_NOT_REPORTED;
    netmgr_qmi_cfg.links[ link ].wds_info.first_conn_clnt = NETMGR_QMI_CLIENT_MAX;

#ifdef FEATURE_DATA_IWLAN
    netmgr_util_circ_list_destroy(&netmgr_qmi_cfg.links[ link ].wds_info.rev_ip_txns[NETMGR_QMI_CLIENT_IPV4]);
    netmgr_util_circ_list_destroy(&netmgr_qmi_cfg.links[ link ].wds_info.rev_ip_txns[NETMGR_QMI_CLIENT_IPV6]);

    memset (&netmgr_qmi_cfg.links[ link ].wds_info.rev_ip_config_status,
            0,
            sizeof (netmgr_qmi_cfg.links[ link ].wds_info.rev_ip_config_status));
    memset (&netmgr_qmi_cfg.links[ link ].wds_info.bearer_tech_ex,
            0,
            sizeof (netmgr_qmi_cfg.links[ link ].wds_info.bearer_tech_ex));
#endif /* FEATURE_DATA_IWLAN */
  }
}


#ifdef FEATURE_DATA_IWLAN
/*===========================================================================
  FUNCTION  netmgr_qmi_send_rev_ip_config_complete
===========================================================================*/
/*!
@brief
  Send the reverse IP configuration status to the modem

@return
  None

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
void
netmgr_qmi_send_rev_ip_config_complete
(
  netmgr_qmi_iwlan_call_mode_t  mode,
  int                           link,
  int                           ip_family,
  int                           status
)
{
  int clnt_hndl;
  qmi_wds_rev_ip_transport_config_result_type res;
  int rc, qmi_err;
  netmgr_qmi_client_type_t clnt;
  ds_dll_el_t  *node = NULL;
  const void *data = NULL;
  qmi_wds_txn_id_type  txn_id;

  NETMGR_LOG_FUNC_ENTRY;

  if (ip_family != AF_INET && ip_family != AF_INET6)
  {
    netmgr_log_err("netmgr_qmi_send_rev_ip_config_complete: invalid ip_family=%d\n",
                   ip_family);
    goto bail;
  }

  if (NETMGR_SUCCESS != netmgr_qmi_verify_link( link ))
  {
    netmgr_log_err("netmgr_qmi_send_rev_ip_config_complete: invalid link=%d\n",
                   link);
    goto bail;
  }

  clnt = (AF_INET == ip_family) ? NETMGR_QMI_CLIENT_IPV4 : NETMGR_QMI_CLIENT_IPV6 ;

  if (NETMGR_QMI_IWLAN_CALL_BRINGUP == mode)
  {
    netmgr_kif_iwlan_update_dynamic_config(link, ip_family);

    /* Query and install rev ip filters (if any) */
    if (NULL == netmgr_qmi_cfg.links[ link ].dfs_info.clnt_hdl[clnt] ||
        NETMGR_SUCCESS != netmgr_qmi_dfs_query_and_process_rev_ip_filters(link,
                                                                          netmgr_qmi_cfg.links[ link ].dfs_info.clnt_hdl[clnt]))
    {
      netmgr_log_err("netmgr_kif_install_sa_and_routing_rules: failed to install rev ip filters, "
                     "link=%d, clnt_hdl=%p",
                     link,
                     netmgr_qmi_cfg.links[ link ].dfs_info.clnt_hdl[clnt]);
    }
  }
  else if (NETMGR_QMI_IWLAN_CALL_CLEANUP == mode)
  {
    netmgr_kif_iwlan_cleanup_dynamic_config(link, ip_family);
  }

  if ((clnt_hndl = netmgr_qmi_cfg.links[ link ].wds_info.clnt_hdl[clnt]) < 0)
  {
    netmgr_log_err("netmgr_qmi_send_rev_ip_config_complete: invalid clnt_hndl=%d, link=%d, ip_family=%d\n",
                   clnt_hndl,
                   link,
                   clnt);
    goto bail;
  }

  if (NETMGR_SUCCESS != netmgr_qmi_wds_util_deq_txn(clnt, link, &txn_id))
  {
    netmgr_log_err("netmgr_qmi_send_rev_ip_config_complete: failed dequeue txn clnt=%d, link=%d\n",
                   clnt,
                   link);
    goto bail;
  }

  netmgr_log_med("netmgr_qmi_send_rev_ip_config_complete: mode=%s, dequeued txn=%ld, clnt=%d, link=%d, status=%d\n",
                 (NETMGR_QMI_IWLAN_CALL_BRINGUP == mode) ? "BRINGUP" : "CLEANUP",
                 txn_id,
                 clnt,
                 link,
                 status);

  res = (status == NETMGR_SUCCESS) ? QMI_WDS_REV_IP_TRANSPORT_CONFIG_SUCCESS :
                                     QMI_WDS_REV_IP_TRANSPORT_CONFIG_FAIL;


  if (QMI_NO_ERR != (rc = qmi_wds_rev_ip_transport_config_complete(clnt_hndl,
                                                                   txn_id,
                                                                   res,
                                                                   &qmi_err)))
  {
    netmgr_log_err("netmgr_qmi_send_rev_ip_config_complete: failed rc=%d, qmi_err=%d\n",
                   rc,
                   qmi_err);
  }

bail:
  NETMGR_LOG_FUNC_EXIT;
  return;
}


/*===========================================================================
  FUNCTION  netmgr_qmi_query_ipsec_sa_config
===========================================================================*/
/*!
@brief
  Query the Security Associations (SA) for the given family and link

@return
  int - NETMGR_SUCCESS on operation success, NETMGR_FAILURE otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int
netmgr_qmi_query_ipsec_sa_config
(
  int                           ip_family,
  int                           link,
  qmi_wds_ipsec_sa_config_type  *sa_config
)
{
  int rc, ret = NETMGR_FAILURE, qmi_err;
  netmgr_qmi_client_type_t clnt;
  int  clnt_hndl;

  NETMGR_LOG_FUNC_ENTRY;

  if (!sa_config)
  {
    netmgr_log_err("netmgr_qmi_query_ipsec_sa_config: invalid input\n");
    goto bail;
  }

  if (AF_INET != ip_family && AF_INET6 != ip_family)
  {
    netmgr_log_err("netmgr_qmi_query_ipsec_sa_config: invalid ip_family=%d\n",
                   ip_family);
    goto bail;
  }

  if (NETMGR_SUCCESS != netmgr_qmi_verify_link( link ))
  {
    netmgr_log_err("netmgr_qmi_query_ipsec_sa_config: invalid link=%d\n",
                   link);
    goto bail;
  }

  clnt = (ip_family == AF_INET) ? NETMGR_QMI_CLIENT_IPV4 : NETMGR_QMI_CLIENT_IPV6;

  if ((clnt_hndl = netmgr_qmi_cfg.links[ link ].wds_info.clnt_hdl[clnt]) < 0)
  {
    netmgr_log_err("netmgr_qmi_query_ipsec_sa_config: invalid clnt_hndl=%d, link=%d, ip_family=%d\n",
                   clnt_hndl,
                   link,
                   clnt);
    goto bail;
  }

  if (QMI_NO_ERR != (rc = qmi_wds_get_ipsec_static_sa_config(clnt_hndl,
                                                             sa_config,
                                                             &qmi_err)))
  {
    netmgr_log_err("netmgr_qmi_query_ipsec_sa_config: SA query failed rc=%d, qmi_err=%d\n",
                   rc,
                   qmi_err);
    goto bail;
  }

  ret = NETMGR_SUCCESS;

bail:
  NETMGR_LOG_FUNC_EXIT;
  return ret;
}

/*===========================================================================
  FUNCTION  netmgr_qmi_iwlan_update_link_assoc
===========================================================================*/
/*!
@brief
  Updates the association for a given link with the corresponding forward or
  reverse link with matching address or prefix

@return
  int - NETMGR_SUCCESS on operation success, NETMGR_FAILURE otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int
netmgr_qmi_iwlan_update_link_assoc
(
  int  link,
  int  *assoc_link
)
{
  int  ret = NETMGR_FAILURE;
  int  start, end;
  int  i, j;
  netmgr_main_link_type  type;
  netmgr_address_info_t  *addr_info = NULL;
  netmgr_address_info_t  *tmp_addr_info = NULL;
  int cur_assoc = NETMGR_LINK_MAX;

  NETMGR_LOG_FUNC_ENTRY;

  addr_info = netmgr_qmi_get_addr_info(link);

  if (NULL == addr_info)
  {
    netmgr_log_err("netmgr_qmi_iwlan_update_link_assoc: invalid link=%d\n",
                   link);
    goto bail;
  }

  if (NULL != assoc_link)
  {
    *assoc_link = NETMGR_LINK_MAX;
  }

  type = (NETMGR_KIF_IS_REV_RMNET_LINK(link) ? NETMGR_REV_LINK : NETMGR_FWD_LINK);

  for (i = 0; i < NETMGR_MAX_MODEMS; ++i)
  {
    if (link >= netmgr_main_dev_prefix_tbl[i][type].inst_min &&
        link <= netmgr_main_dev_prefix_tbl[i][type].inst_max)
    {
      break;
    }
  }

  if (i >= NETMGR_MAX_MODEMS)
  {
    netmgr_log_err("netmgr_qmi_iwlan_update_link_assoc: unable to determine modem for link=%d\n",
                   link);
    goto bail;
  }

  /* Clear any prior associations for this link */
  if (NETMGR_LINK_MAX != (cur_assoc = netmgr_qmi_iwlan_get_link_assoc(link)))
  {
    (void) netmgr_qmi_iwlan_clear_link_assoc(link, cur_assoc);

    if (NETMGR_FWD_LINK == type &&
        NETMGR_STATE_GOING_DOWN == stm_get_state(&NETMGR_SM[cur_assoc]))
    {
      netmgr_log_med("netmgr_qmi_iwlan_update_link_assoc: updating link=%d, bringing down prior associated rev_link=%d\n",
                     link,
                     cur_assoc);
      netmgr_kif_iface_close((uint8)cur_assoc, NULL, TRUE);
    }
  }

  /* Search for a iface with mtaching address in the other link set */
  if (NETMGR_FWD_LINK == type)
  {
    start = netmgr_main_dev_prefix_tbl[i][NETMGR_REV_LINK].inst_min;
    end   = netmgr_main_dev_prefix_tbl[i][NETMGR_REV_LINK].inst_max;
  }
  else
  {
    start = netmgr_main_dev_prefix_tbl[i][NETMGR_FWD_LINK].inst_min;
    end   = netmgr_main_dev_prefix_tbl[i][NETMGR_FWD_LINK].inst_max;
  }

  netmgr_log_med("netmgr_qmi_iwlan_update_link_assoc: finding assoc for link=%d [v4=0x%x, v6=0x%llx], type=%d, modem=%d in [%d, %d]\n",
                 link,
                 (addr_info->valid_mask & NETMGR_ADDRSET_MASK_IPV4) ? addr_info->ipv4.if_addr.addr.v4 : 0u,
                 (addr_info->valid_mask & NETMGR_ADDRSET_MASK_IPV6) ? addr_info->ipv6.if_addr.addr.v6_addr64[0] : 0llu,
                 type,
                 i,
                 start,
                 end);

  for (j = start; j <= end; ++j)
  {
    tmp_addr_info = netmgr_qmi_get_addr_info(j);

    if (NULL == tmp_addr_info)
    {
      continue;
    }

    if ((addr_info->valid_mask & NETMGR_ADDRSET_MASK_IPV4) &&
        (tmp_addr_info->valid_mask & NETMGR_ADDRSET_MASK_IPV4))
    {
      netmgr_log_med("netmgr_qmi_iwlan_update_link_assoc: valid V4 addr link=%d [0x%x], link=%d [0x%x]\n",
                     link,
                     addr_info->ipv4.if_addr.addr.v4,
                     j,
                     tmp_addr_info->ipv4.if_addr.addr.v4);


      /* If a matching IPv4 address is found */
      if (addr_info->ipv4.if_addr.addr.v4 == tmp_addr_info->ipv4.if_addr.addr.v4)
      {
        break;
      }
    }
    else if ((addr_info->valid_mask & NETMGR_ADDRSET_MASK_IPV6) &&
             (tmp_addr_info->valid_mask & NETMGR_ADDRSET_MASK_IPV6))
    {
      netmgr_log_med("netmgr_qmi_iwlan_update_link_assoc: valid V6 addr link=%d [0x%llx], link=%d [0x%llx]\n",
                     link,
                     addr_info->ipv6.if_addr.addr.v6_addr64[0],
                     j,
                     tmp_addr_info->ipv6.if_addr.addr.v6_addr64[0]);

      /* If a matching IPv6 prefix is found */
      if (addr_info->ipv6.if_addr.addr.v6_addr64[0] == tmp_addr_info->ipv6.if_addr.addr.v6_addr64[0])
      {
        break;
      }
    }
  }

  if (j > end)
  {
    netmgr_log_med("netmgr_qmi_iwlan_update_link_assoc: unable to find a matching association for link=%d\n",
                   link);

    goto bail;
  }

  netmgr_log_med("netmgr_qmi_iwlan_update_link_assoc: associating link=%d with link=%d\n",
                 link,
                 j);

  netmgr_qmi_cfg.links[link].assoc_link = j;
  netmgr_qmi_cfg.links[j].assoc_link = link;

  if (NULL != assoc_link)
  {
    *assoc_link = j;
  }

  ret = NETMGR_SUCCESS;

bail:
  NETMGR_LOG_FUNC_EXIT;
  return ret;
}

/*===========================================================================
  FUNCTION  netmgr_qmi_iwlan_clear_link_assoc
===========================================================================*/
/*!
@brief
  Removes the association between the given forward and reverse links.

@return
  int - NETMGR_SUCCESS on operation success, NETMGR_FAILURE otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int
netmgr_qmi_iwlan_clear_link_assoc
(
  int  fwd_link,
  int  rev_link
)
{
  int ret_fwd = NETMGR_FAILURE;
  int ret_rev = NETMGR_FAILURE;
  int ret;

  NETMGR_LOG_FUNC_ENTRY;

  if (NETMGR_SUCCESS != netmgr_qmi_verify_link (fwd_link))
  {
    netmgr_log_med("netmgr_qmi_iwlan_clear_link_assoc: invalid fwd_link=%d\n",
                   fwd_link);
  }
  else
  {
    netmgr_qmi_cfg.links[fwd_link].assoc_link = NETMGR_LINK_MAX;
    ret_fwd = NETMGR_SUCCESS;
  }

  if (NETMGR_SUCCESS != netmgr_qmi_verify_link (rev_link))
  {
    netmgr_log_med("netmgr_qmi_iwlan_clear_link_assoc: invalid rev_link=%d\n",
                   rev_link);
  }
  else
  {
    netmgr_qmi_cfg.links[rev_link].assoc_link = NETMGR_LINK_MAX;
    ret_rev = NETMGR_SUCCESS;
  }

  ret = (NETMGR_SUCCESS == ret_fwd || NETMGR_SUCCESS == ret_rev) ?
        NETMGR_SUCCESS :
        NETMGR_FAILURE;

bail:
  NETMGR_LOG_FUNC_EXIT;
  return ret;
}

/*===========================================================================
  FUNCTION  netmgr_qmi_iwlan_get_link_assoc
===========================================================================*/
/*!
@brief
  Returns the corresponding link associated with the given forward or reverse
  iWLAN links.

@return
  A valid link on success, NETMGR_LINK_MAX on error

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int
netmgr_qmi_iwlan_get_link_assoc
(
  int  link
)
{
  int ret = NETMGR_LINK_MAX;

  NETMGR_LOG_FUNC_ENTRY;

  if (NETMGR_SUCCESS != netmgr_qmi_verify_link (link))
  {
    netmgr_log_err("netmgr_qmi_iwlan_get_link_assoc: invalid link=%d\n",
                   link);
    goto bail;
  }

  ret = netmgr_qmi_cfg.links[link].assoc_link;

bail:
  NETMGR_LOG_FUNC_EXIT;
  return ret;
}

/*===========================================================================
  FUNCTION  netmgr_qmi_save_rev_ip_config_status
===========================================================================*/
/*!
@brief
  Saves the reverse IP config status for the given link

@return
  int - NETMGR_SUCCESS on operation success, NETMGR_FAILURE otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int
netmgr_qmi_save_rev_ip_config_status
(
  int  link,
  int  ip_family,
  int  status
)
{
  int ret = NETMGR_FAILURE;

  NETMGR_LOG_FUNC_ENTRY;

  if (NETMGR_SUCCESS != netmgr_qmi_verify_link( link ))
  {
    netmgr_log_err("netmgr_qmi_save_rev_ip_config_status: invalid link=%d\n",
                   link);
    goto bail;
  }

  if (AF_INET != ip_family && AF_INET6 != ip_family)
  {
    netmgr_log_err("netmgr_qmi_save_rev_ip_config_status: invalid ip_family=%d\n",
                   ip_family);
    goto bail;
  }

  if (NETMGR_SUCCESS != status && NETMGR_FAILURE != status)
  {
    netmgr_log_err("netmgr_qmi_save_rev_ip_config_status: invalid status=%d\n",
                   status);
    goto bail;
  }

  netmgr_qmi_cfg.links[ link ].wds_info.rev_ip_config_status.ip_family = ip_family;
  netmgr_qmi_cfg.links[ link ].wds_info.rev_ip_config_status.status = status;

  /* Set the state info as valid */
  netmgr_qmi_cfg.links[link].wds_info.rev_ip_config_status.is_valid = TRUE;

  ret = NETMGR_SUCCESS;

bail:
  NETMGR_LOG_FUNC_EXIT;
  return ret;
}

/*===========================================================================
  FUNCTION  netmgr_qmi_retrieve_rev_ip_config_status
===========================================================================*/
/*!
@brief
  Retrieves the reverse IP config status for the given link

@return
  int - NETMGR_SUCCESS on operation success, NETMGR_FAILURE otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int
netmgr_qmi_retrieve_rev_ip_config_status
(
  int  link,
  int  *ip_family,
  int  *status
)
{
  int ret = NETMGR_FAILURE;

  NETMGR_LOG_FUNC_ENTRY;

  if (NETMGR_SUCCESS != netmgr_qmi_verify_link( link ))
  {
    netmgr_log_err("netmgr_qmi_retrieve_rev_ip_config_status: invalid link=%d\n",
                   link);
    goto bail;
  }

  if (!ip_family || !status)
  {
    netmgr_log_err("netmgr_qmi_retrieve_rev_ip_config_status: invalid input params\n");
    goto bail;
  }

  if (FALSE == netmgr_qmi_cfg.links[link].wds_info.rev_ip_config_status.is_valid)
  {
    netmgr_log_med("netmgr_qmi_retrieve_rev_ip_config_status: invalid state info\n");
    goto bail;
  }

  *ip_family = netmgr_qmi_cfg.links[ link ].wds_info.rev_ip_config_status.ip_family;
  *status = netmgr_qmi_cfg.links[ link ].wds_info.rev_ip_config_status.status;

  /* Invalidate the status info upon retrieval */
  netmgr_qmi_cfg.links[link].wds_info.rev_ip_config_status.is_valid = FALSE;

  ret = NETMGR_SUCCESS;

bail:
  NETMGR_LOG_FUNC_EXIT;
  return ret;
}

/*===========================================================================
  FUNCTION  netmgr_qmi_initiate_esp_rekey
===========================================================================*/
/*!
@brief
  Sends ESP rekey message

@return
  NETMGR_SUCCESS on operation success
  NETMGR_FAILURE otherwise

@note
  - Dependencies
    - ESP sequence number has overflown

  - Side Effects
    - None
*/
/*=========================================================================*/
int
netmgr_qmi_initiate_esp_rekey
(
  int link,
  netmgr_qmi_client_type_t qmi_client
)
{
  int rc, qmi_err_code;
  int ret = NETMGR_FAILURE;
  int wds_hndl = NETMGR_QMI_CLIENT_INVALID;

  NETMGR_LOG_FUNC_ENTRY;

  do
  {
    /* Validate input parameters */
    if (NETMGR_QMI_CLIENT_MAX <= qmi_client ||
        (NETMGR_QMI_CLIENT_IPV4 != qmi_client &&
        NETMGR_QMI_CLIENT_IPV6 != qmi_client))
    {
      netmgr_log_err("%s", "netmgr_qmi_initiate_esp_rekey: Invalid QMI client");
      break;
    }

    wds_hndl = netmgr_qmi_wds_get_clnt_id(link, qmi_client);

    if (NETMGR_QMI_CLIENT_INVALID == wds_hndl)
    {
      netmgr_log_err("netmgr_qmi_initiate_esp_rekey: Unable to retrive valid qmi client for link [%d]",
                     link);
      break;
    }

    /* Send QMI message */
    rc = qmi_wds_initiate_esp_rekey(wds_hndl, &qmi_err_code);

    if (QMI_NO_ERR != rc)
    {
      netmgr_log_err("netmgr_qmi_initiate_esp_rekey: qmi_wds_initiate_esp_rekey failed [%d][%d] for link [%d]",
                     rc, qmi_err_code, link);
      break;
    }

    /* Rekey succeeded */
    ret = NETMGR_SUCCESS;
  } while (0);

  return ret;
}
#endif /* FEATURE_DATA_IWLAN */

/*===========================================================================
  FUNCTION  netmgr_qmi_init
===========================================================================*/
/*!
@brief
 Main initialization routine of the QMI WDS Interface module.

@return
  void

@note

  - Dependencies
    - None

  - Side Effects
    - Initializes the QMI Driver
*/
/*=========================================================================*/
void
netmgr_qmi_init (int nlink, netmgr_ctl_port_config_type links[], netmgr_data_format_t data_format)
{
  struct qmi_vtbl vtable;

  NETMGR_LOG_FUNC_ENTRY;

  /* Set number of links in the configuration blob */
  netmgr_qmi_cfg.nlink = nlink;
  netmgr_qmi_cfg.link_array = links;   /* Needed for reset */
  netmgr_qmi_cfg.data_format = data_format;   /* Needed for reset */

  /* Register with Platform layer */
  vtable.out_of_service    = netmgr_qmi_driver_cleanup;
  vtable.reset             = netmgr_qmi_reset_connection;
  vtable.verify            = netmgr_qmi_verify_link;
  vtable.dispatch          = netmgr_qmi_cmd_exec;
  vtable.qos_get_flow_info = netmgr_qmi_get_flow_info;

  if( NETMGR_SUCCESS !=
      netmgr_platform_register_vtbl( NETMGR_PLATFORM_VTBL_QMI,
                                     (void*)&vtable ) )
  {
    NETMGR_ABORT("netmgr_qmi_init: cannot register vtable with platform layer");
    return;
  }

  /* Initialize link data structures */
  netmgr_qmi_link_init();

  /* Initialize the QMI Client Driver and start WDS clients for each
  ** interface.
  */
  netmgr_qmi_driver_init(data_format);

  NETMGR_LOG_FUNC_EXIT;
  return;
}

