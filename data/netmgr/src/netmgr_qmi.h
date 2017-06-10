/******************************************************************************

                          N E T M G R _ Q M I . H

******************************************************************************/

/******************************************************************************

  @file    netmgr_qmi.h
  @brief   Network Manager QMI Driver Interface header file

  DESCRIPTION
  Header file for NetMgr QMI Driver interface.

******************************************************************************/
/*===========================================================================

  Copyright (c) 2010-2014 Qualcomm Technologies, Inc. All Rights Reserved

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
02/11/10   ar         Initial version (derived from DSC file)

******************************************************************************/

#ifndef __NETMGR_QMI_H__
#define __NETMGR_QMI_H__

/*===========================================================================
                              INCLUDE FILES
===========================================================================*/
#include "comdef.h"
#ifdef FEATURE_DATA_IWLAN

#ifndef QMI_CLIENT_H
  #undef QMI_INTERNAL_ERR
  #undef QMI_SERVICE_ERR
  #undef QMI_TIMEOUT_ERR
  #undef QMI_EXTENDED_ERR
  #undef QMI_PORT_NOT_OPEN_ERR
  #undef QMI_MEMCOPY_ERROR
  #undef QMI_INVALID_TXN
  #undef QMI_CLIENT_ALLOC_FAILURE
#endif

  #include "qmi_client.h"
#endif /* FEATURE_DATA_IWLAN */
#include "data_filter_service_v01.h"
#include "qmi.h"
#include "qmi_wds_srvc.h"
#include "qmi_qos_srvc.h"
#include "netmgr_tc.h"
#include "netmgr.h"
#include "netmgr_util.h"
#include "qmi_client.h"
#include "qmi_client_instance_defs.h"
#include "netmgr_config.h"

/*===========================================================================
                     GLOBAL DEFINITIONS AND DECLARATIONS
===========================================================================*/

/*---------------------------------------------------------------------------
   Modem handle for primary flow
---------------------------------------------------------------------------*/
#define NETMGR_QMI_PRIMARY_FLOW_ID        (0)
#define NETMGR_IS_DEFAULT_FLOW(id)                           \
        ((NETMGR_QMI_PRIMARY_FLOW_ID == id)? TRUE : FALSE)
#define NETMGR_QMI_TIMEOUT                (10000)

typedef enum {
  NETMGR_QMI_CLIENT_IPV4,
  NETMGR_QMI_CLIENT_IPV6,
  NETMGR_QMI_CLIENT_MAX
} netmgr_qmi_client_type_t;

/*---------------------------------------------------------------------------
  Type representing enumeration of QMI WDS Interface Commands
---------------------------------------------------------------------------*/
typedef enum {
  NETMGR_QMI_CMD_MIN,
  NETMGR_QMI_SYS_IND_CMD,
  NETMGR_QMI_WDS_IND_CMD,
  NETMGR_QMI_QOS_EVENT_REPORT_IND_CMD,
  NETMGR_QMI_DFS_IND_CMD,
  NETMGR_QMI_CMD_MAX
} netmgr_qmi_cmd_type_t;

/*---------------------------------------------------------------------------
  Type of Command data for a QMI SYS Indication
---------------------------------------------------------------------------*/
typedef struct {
  qmi_sys_event_type event_id;            /* Event id */
  qmi_sys_event_info_type event_info;     /* Event info */
  void * user_data;                       /* User data */
} netmgr_qmi_sys_ind_t;

/*---------------------------------------------------------------------------
  Type of Command data for a QMI WDS Indication
---------------------------------------------------------------------------*/
typedef struct {
  int                             link;      /* Link id */
  int                             user_hndl; /* The handle on which indication
                                                was received */
  qmi_wds_indication_id_type      ind_id;    /* Type of indication */
  qmi_wds_indication_data_type    ind_data;  /* Message data */
} netmgr_qmi_wds_ind_t;

/*---------------------------------------------------------------------------
  Type representing collection of QoS flow attributes
---------------------------------------------------------------------------*/
typedef struct netmgr_qmi_qos_flow_info_s {
  uint32                            flow_id;
  netmgr_tc_flow_state_t            state;
  boolean                           is_new;
  uint8                             flow_type;
  uint8                             priority;
  uint32                            datarate;
  uint8                             num_filter;
  qmi_qos_granted_filter_data_type  filter_list[QMI_QOS_MAX_FLOW_FILTER];
} netmgr_qmi_qos_flow_info_t;

/*---------------------------------------------------------------------------
  Type of Command data for a QMI QOS Indication
  ---------------------------------------------------------------------------*/
typedef struct {
  int                            link;        /* Link id */
  qmi_qos_indication_id_type     ind_id;      /* Indication ID */
  qmi_qos_indication_data_type   info;        /* Indication information */
} netmgr_qmi_qos_ind_t;

typedef union {
  dfs_reverse_ip_transport_filters_updated_ind_msg_v01  filter_ind;
} netmgr_qmi_dfs_ind_data_type;

typedef struct {
  int                            link;        /* Link id */
  unsigned int                   ind_id;      /* Indication ID */
  netmgr_qmi_dfs_ind_data_type   info;
} netmgr_qmi_dfs_ind_t;

/*---------------------------------------------------------------------------
  Type of QMI internal commands
---------------------------------------------------------------------------*/
typedef struct {
  netmgr_qmi_cmd_type_t                type;
  union {
    netmgr_qmi_sys_ind_t               sys_ind;
    netmgr_qmi_wds_ind_t               wds_ind;
    netmgr_qmi_qos_ind_t               qos_ind;
    netmgr_qmi_dfs_ind_t               dfs_ind;
  } data;
} netmgr_qmi_cmd_t;

/*---------------------------------------------------------------------------
  Type representing collection of WDS service info
---------------------------------------------------------------------------*/
typedef struct {
  boolean                           is_valid;
  qmi_wds_data_bearer_tech_type_ex  bt;
} netmgr_qmi_bearer_tech_t;

typedef struct {
  int clnt_hdl[NETMGR_QMI_CLIENT_MAX];      /* QMI WDS client handles      */
  netmgr_qmi_client_type_t  first_conn_clnt;/* First connnected client     */
  netmgr_address_info_t addr_info;          /* Network interface addr info */
  unsigned long         mtu;                /* Negotiated network link MTU */
  unsigned short        tech_name;          /* technology name             */
#ifdef FEATURE_DATA_IWLAN
  netmgr_util_circ_list_type  rev_ip_txns[NETMGR_QMI_CLIENT_MAX];
  struct
  {
    boolean         is_valid;
    int             ip_family;
    int             status;
  } rev_ip_config_status;
  netmgr_qmi_bearer_tech_t    bearer_tech_ex;
#endif /* FEATURE_DATA_IWLAN */
} netmgr_wds_qmi_drv_info_t;

/*---------------------------------------------------------------------------
  Type representing collection of QOS service info
---------------------------------------------------------------------------*/
typedef struct {
  int     clnt_hdl[NETMGR_QMI_CLIENT_MAX];        /* QMI QOS client handles */
  netmgr_qmi_qos_flow_info_t   *qos_flows[QMI_QOS_MAX_FLOW_FILTER];
} netmgr_qos_qmi_drv_info_t;

#ifdef FEATURE_DATA_IWLAN
typedef struct {
  qmi_client_type  clnt_hdl[NETMGR_QMI_CLIENT_MAX];        /* QMI QOS client handles */
} netmgr_dfs_qmi_drv_info_t;
#endif /* FEATURE_DATA_IWLAN */

/*---------------------------------------------------------------------------
  Type representing collection of a link's state information
---------------------------------------------------------------------------*/
typedef struct {
  netmgr_wds_qmi_drv_info_t  wds_info;  /* WDS service info */
  netmgr_qos_qmi_drv_info_t  qos_info;  /* QOS service info */

#ifdef FEATURE_DATA_IWLAN
  netmgr_dfs_qmi_drv_info_t  dfs_info;  /* DFS service info */
  int                        assoc_link; /* Associated fwd or rev link if in
                                            an iWLAN/ePDG call */
#endif /* FEATURE_DATA_IWLAN */
} netmgr_link_info_t;

/*---------------------------------------------------------------------------
  Collection of configuration information for the module
---------------------------------------------------------------------------*/
struct netmgr_qmi_cfg_s {
  int nlink;                                        /* number of qmi links */
  netmgr_ctl_port_config_type * link_array;         /* link enabled array  */
  netmgr_data_format_t data_format;
  netmgr_link_info_t links[NETMGR_MAX_LINK];        /* link state info     */
};

extern struct netmgr_qmi_cfg_s netmgr_qmi_cfg;

#ifdef FEATURE_DATA_IWLAN
typedef enum
{
  NETMGR_QMI_IWLAN_CALL_BRINGUP,
  NETMGR_QMI_IWLAN_CALL_CLEANUP
} netmgr_qmi_iwlan_call_mode_t;
#endif /* FEATURE_DATA_IWLAN */

/*===========================================================================
                            GLOBAL FUNCTION DECLARATIONS
===========================================================================*/

/*===========================================================================
  FUNCTION  netmgr_qmi_get_ip_addr_type
===========================================================================*/
/*!
@brief
  Retrives the IP address type for Modem netwok interface

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
netmgr_ip_addr_t netmgr_qmi_get_ip_addr_type_first_conn_clnt( int link );

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
netmgr_address_info_t * netmgr_qmi_get_addr_info( int link );

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
unsigned int netmgr_qmi_get_mtu( int link );

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
unsigned short netmgr_qmi_wds_get_tech_name( int link );

/*===========================================================================
  FUNCTION  netmgr_qmi_init
===========================================================================*/
/*!
@brief
 Main initialization routine of the QMI Interface module.

@return
  void

@note

  - Dependencies
    - None

  - Side Effects
    - Initializes the QMI Driver
*/
/*=========================================================================*/
void netmgr_qmi_init (int nlink, netmgr_ctl_port_config_type links[], netmgr_data_format_t data_format);

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
);

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
);

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
);

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
);

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
);

/*===========================================================================
  FUNCTION  netmgr_qmi_iwlan_get_link_assoc
===========================================================================*/
/*!
@brief
  Returns the corresponding link associated with the given forward or reverse
  iWLAN links.

@return
  A valid link on success
  NETMGR_LINK_MAX on error

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
);

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
);

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
);


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
);

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
);

/*===========================================================================
  FUNCTION  netmgr_qmi_get_ipsec_tunnel_endpoints
===========================================================================*/
/*!
@brief
  Retrives the tunnel endpoint info for a given link and ip family

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
);

#endif /* FEATURE_DATA_IWLAN */

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
);

#endif /* __NETMGR_QMI_H__ */
