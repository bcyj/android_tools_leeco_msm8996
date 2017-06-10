/******************************************************************************

                          N E T M G R _ D E F S . H

******************************************************************************/

/******************************************************************************

  @file    netmgr_defs.h
  @brief   Network Manager common definitions header file

  DESCRIPTION
  Header file containing common definitions for NetMgr modules

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
02/08/10   ar         Initial version

******************************************************************************/

#ifndef __NETMGR_DEFS_H__
#define __NETMGR_DEFS_H__

#include "comdef.h"
#include "netmgr.h"

/*===========================================================================
                                 TYPE DEFINITIONS
===========================================================================*/

#define NETMGR_TRUE  "true"
#define NETMGR_FALSE "false"


/* Macro for maximum number of links to Modem */
#define NETMGR_MAX_MODEMS 2
/* following values are used in conjuction with
 * ro.baseband property to configure the
 * netmgr_ctl_port_array[] links */
#define NETMGR_MODEM_MSM 0
#define NETMGR_MODEM_MDM 1

#ifdef FEATURE_DATA_IWLAN
 #define NETMGR_MAX_LINKS_PER_MODEM (8+9) /* 9 Additional ports for reverse rmnet */
#else
 #define NETMGR_MAX_LINKS_PER_MODEM (8)
#endif /* FEATURE_DATA_IWLAN */

#define NETMGR_MAX_LINK (NETMGR_MAX_MODEMS*NETMGR_MAX_LINKS_PER_MODEM)

/* Macro for state of link/interface configuration */
#define NETMGR_STATE_ENABLED   (1)
#define NETMGR_STATE_DISABLED  (0)

/* Macros for link MTU */
#define NETMGR_MTU_INVALID (0)
#define NETMGR_MTU_MAX     (2000)

#define NETMGR_MAX_COMMAND_LENGTH  (512)
#define NETMGR_MAX_STR_LENGTH      (150)

#define  NETMGR_MAIN_RMNET_SMD_PREFIX      "rmnet"          /* Prefix for smd ports */
#define  NETMGR_MAIN_RMNET_SDIO_PREFIX     "rmnet_sdio"     /* Prefix for sdio ports */
#define  NETMGR_MAIN_RMNET_USB_PREFIX      "rmnet_usb"      /* Prefix for usb ports */
#define  NETMGR_MAIN_RMNET_DATA_PREFIX     "rmnet_data"     /* Prefix for data ports */
#define  NETMGR_MAIN_RMNET_SMUX_PREFIX     "rmnet_smux"     /* Prefix for smux ports */
#define  NETMGR_MAIN_RMNET2_USB_PREFIX     "rmnet2_usb"     /* Prefix for second MDM USB ports */

#ifdef FEATURE_DATA_IWLAN
  #define  NETMGR_MAIN_REV_RMNET_SMD_PREFIX  "rev_rmnet"      /* Prefix for reverse smd ports */
  #define  NETMGR_MAIN_REV_RMNET_USB_PREFIX  "rev_rmnet_usb"  /* Prefix for reverse usb ports */
  #define  NETMGR_MAIN_REV_RMNET_DATA_PREFIX "r_rmnet_data"   /* Prefix for rmnet data reverse ports */
#endif /* FEATURE_DATA_IWLAN */

/* Start index of Rmnet data devices */
#define NETMGR_RMNET_START 0

/* Table of transport device name prefix per Modem */
typedef struct netmgr_main_dev_prefix_s
{
  char    prefix[NETMGR_IF_NAME_MAX_LEN];
  byte    inst_min;
  byte    inst_max;
} netmgr_main_dev_prefix_type;

#ifdef FEATURE_DATA_IWLAN
  typedef enum
  {
    NETMGR_FWD_LINK,
    NETMGR_REV_LINK,
    NETMGR_MAX_LINK_TYPES
  } netmgr_main_link_type;

  extern netmgr_main_dev_prefix_type netmgr_main_dev_prefix_tbl[NETMGR_MAX_MODEMS][NETMGR_MAX_LINK_TYPES];
#else
  extern netmgr_main_dev_prefix_type netmgr_main_dev_prefix_tbl[];
#endif /* FEATURE_DATA_IWLAN */

/*---------------------------------------------------------------------------
  Type representing network interface IP address
---------------------------------------------------------------------------*/
typedef enum netmgr_ip_addr_e {
  NETMGR_IP_ANY_ADDR     = 0x00,
  NETMGR_IPV4_ADDR       = 0x04,
  NETMGR_IPV6_ADDR       = 0x06,
  NETMGR_IPV4V6_ADDR     = 0x0A,
  NETMGR_IP_ADDR_INVALID = 0xFF
} netmgr_ip_addr_t;

/*---------------------------------------------------------------------------
  Type representing network interface IP address
---------------------------------------------------------------------------*/
typedef struct netmgr_ip_address_s {
  netmgr_ip_addr_t type;
  union netmgr_ip_address_u
  {
    uint32 v4;
    uint64 v6_addr64[2];
    uint32 v6_addr32[4];
    uint16 v6_addr16[8];
    uint8  v6_addr8[16];
  } addr;
} netmgr_ip_address_t;

#if (defined(NETMGR_OFFTARGET) || defined(FEATURE_DATA_LINUX_LE))
  #define NETMGR_CONV_TO_SOCKADDR(src,dest)                                       \
    (dest)->ss_family = (NETMGR_IPV6_ADDR == (src)->type)? AF_INET6 : AF_INET;    \
    memcpy( &(dest)->__ss_padding, &(src)->addr,                                  \
            MIN(sizeof((dest)->__ss_padding), sizeof((src)->addr)) );
  #define SA_FAMILY(addr)         (addr).sa_family
  #define SA_DATA(addr)           (addr).sa_data
  #define SASTORAGE_FAMILY(addr)  (addr).ss_family
  #define SASTORAGE_DATA(addr)    (addr).__ss_padding
#else
  #define NETMGR_CONV_TO_SOCKADDR(src,dest)                                       \
    (dest)->ss_family = (NETMGR_IPV6_ADDR == (src)->type)? AF_INET6 : AF_INET;    \
    memcpy( &(dest)->__data, &(src)->addr,                                        \
            MIN(sizeof((dest)->__data), sizeof((src)->addr)) );
  #define SA_FAMILY(addr)         (addr).sa_family
  #define SA_DATA(addr)           (addr).sa_data
  #define SASTORAGE_FAMILY(addr)  (addr).ss_family
  #define SASTORAGE_DATA(addr)    (addr).__data
#endif /* FEATURE_DS_LINUX_ANDROID */


/*---------------------------------------------------------------------------
  Types representing network interface IP address information
---------------------------------------------------------------------------*/
#ifdef FEATURE_DATA_IWLAN
  typedef enum
  {
    NETMGR_IPSEC_ALGO_INVALID = -1,
    NETMGR_IPSEC_ALGO_HASH,
    NETMGR_IPSEC_ALGO_CRYPTO,
  } netmgr_ipsec_algo_type;

  typedef enum
  {
    NETMGR_SA_DIR_TX,
    NETMGR_SA_DIR_RX,
    NETMGR_SA_DIR_MAX
  } netmgr_ipsec_sa_dir_t;

  /* IPSec SA (Security Association) associated with an address family */
  typedef struct
  {
    boolean        is_sa_valid;  /* Flag indication whether SA
                                   configuration is required   */
    boolean        is_sa_shared; /* Flag indicating whether the
                                   SA (Security Association) is shared
                                   between V4 and V6 calls     */
    const char     *mode;
    char           sa_state_id[NETMGR_SA_DIR_MAX][NETMGR_MAX_STR_LENGTH];
    char           sa_policy_sel[NETMGR_SA_DIR_MAX][NETMGR_MAX_STR_LENGTH];
    unsigned long  esp_spi_v4;   /* SPI for detecting sequence no. rollovers */
    unsigned long  esp_spi_v6;

    struct {
      boolean      is_valid;
      char         local_addr[NETMGR_MAX_STR_LENGTH];
      char         dest_addr[NETMGR_MAX_STR_LENGTH];
      int          ip_family;
    } tunnel_ep;
  } netmgr_ipsec_sa_t;
#endif /* FEATURE_DATA_IWLAN */

typedef struct netmgr_address_set_s {
  netmgr_ip_address_t if_addr;             /* Network interface address    */
  unsigned int        if_mask;             /* V4 subnet mask|V6 Prefix Len */
  netmgr_ip_address_t gateway;             /* Gateway address              */
  unsigned int        gw_mask;             /* V6 Prefix Len                */
  netmgr_ip_address_t dns_primary;         /* Primary DNS server address   */
  netmgr_ip_address_t dns_secondary;       /* Secondary DNS server address */
  boolean             is_addr_purge_pend;  /* Set to TRUE if an address purge
                                              is pending */
  netmgr_ip_address_t ll_addr;             /* Link Local interface address  */
#ifdef FEATURE_DATA_IWLAN
  netmgr_ipsec_sa_t   sa;                  /* Security Association (SA) info */
#endif /* FEATURE_DATA_IWLAN */

} netmgr_address_set_t;

#define NETMGR_ADDRSET_MASK_INVALID (0x00)
#define NETMGR_ADDRSET_MASK_IPV4    (0x01)
#define NETMGR_ADDRSET_MASK_IPV6    (0x02)
#define NETMGR_ADDRSET_MASK_IPV4V6  (NETMGR_ADDRSET_MASK_IPV4|NETMGR_ADDRSET_MASK_IPV6)
#define NETMGR_STRING_LEN_MAX       (256)

typedef struct netmgr_address_info_s {
  unsigned int           valid_mask;
  netmgr_address_set_t   ipv4;
  netmgr_address_set_t   ipv6;
} netmgr_address_info_t;

/*---------------------------------------------------------------------------
  Type representing generic message buffer
---------------------------------------------------------------------------*/
typedef struct netmgr_msg_s {
  unsigned int     msglen;
  void           * msg;
} netmgr_msg_t;

typedef struct
{
  uint32 consumer_pipe_num;
  uint32 producer_pipe_num;
} netmgr_ipa_ep_pair_type;


#endif /* __NETMGR_DEFS_H__ */
