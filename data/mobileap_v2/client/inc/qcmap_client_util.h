#ifndef _QCMAP_CLIENT_UTIL_H_ 
#define _QCMAP_CLIENT_UTIL_H_

/******************************************************************************

                         qcmap_client_util.h 

******************************************************************************/

/******************************************************************************

  @file    qcmap_firewall.h

  DESCRIPTION
  Header file for firewall data structure.

  ---------------------------------------------------------------------------
  Copyright (c) 2011-2013 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------

******************************************************************************/

/******************************************************************************

                      EDIT HISTORY FOR FILE

when       who        what, where, why
--------   ---        -------------------------------------------------------
07/11/12   bnn         9x25 

******************************************************************************/

/*===========================================================================

                          INCLUDE FILES FOR MODULE

===========================================================================*/
#include "comdef.h"
#include "qcmap_firewall_util.h"

/*===========================================================================
MACRO IPV4_ADDR_MSG()

DESCRIPTION
  This macro prints an IPV4 address to F3.

PARAMETERS
  ip_addr: The IPV4 address in host byte order.

RETURN VALUE
  none
===========================================================================*/
#define IPV4_ADDR_MSG(ip_addr) MSG_4(MSG_SSID_DS, \
                        MSG_LEGACY_HIGH, \
                        "IPV4 Address is %d.%d.%d.%d", \
                        (unsigned char)(ip_addr), \
                        (unsigned char)(ip_addr >> 8), \
                        (unsigned char)(ip_addr >> 16) , \
                        (unsigned char)(ip_addr >> 24))

/*===========================================================================
MACRO IPV6_ADDR_MSG()

DESCRIPTION
  This macro prints an IPV6 address to F3.

PARAMETERS
  ip_addr: The IPV6 address in network byte order.

RETURN VALUE
  none
===========================================================================*/
#define IPV6_ADDR_MSG(ip_addr) MSG_8(MSG_SSID_DS, \
                        MSG_LEGACY_HIGH, \
                        "IPV6 Address %x:%x:%x:%x:%x:%x:%x:%x", \
                        (uint16)(ps_ntohs(ip_addr[0])), \
                        (uint16)(ps_ntohs(ip_addr[0] >> 16)), \
                        (uint16)(ps_ntohs(ip_addr[0] >> 32)) , \
                        (uint16)(ps_ntohs(ip_addr[0] >> 48)), \
                        (uint16)(ps_ntohs(ip_addr[1])), \
                        (uint16)(ps_ntohs(ip_addr[1] >> 16)), \
                        (uint16)(ps_ntohs(ip_addr[1] >> 32)) , \
                        (uint16)(ps_ntohs(ip_addr[1] >> 48)))

#define MAX_COMMAND_STR_LEN 200
#define IPV4_ADDR_LEN 4
#define IPV6_ADDR_LEN 16
#define IPTABLE_CHAIN 10
/*---------------------------------------------------------------------------
           FireWall Entry Configuration.
-----------------------------------------------------------------------------*/



typedef struct
{
  ip_filter_type filter_spec;
  uint32         firewall_handle;
} qcmap_msgr_firewall_entry_conf_t;

/*---------------------------------------------------------------------------
            FireWall handle list configuration.
---------------------------------------------------------------------------*/
typedef struct
{
  uint32 handle_list[QCMAP_MSGR_MAX_FIREWALL_ENTRIES_V01];
  ip_version_enum_type ip_family;
  int num_of_entries;
} qcmap_msgr_get_firewall_handle_list_conf_t;

/*---------------------------------------------------------------------------
            FireWall handle configuration.
---------------------------------------------------------------------------*/
typedef struct
{
  uint32 handle;
  ip_version_enum_type ip_family;
} qcmap_msgr_firewall_handle_conf_t;

/*---------------------------------------------------------------------------
            FireWall configuration.
---------------------------------------------------------------------------*/
typedef union
{
  qcmap_msgr_firewall_entry_conf_t extd_firewall_entry;
  qcmap_msgr_get_firewall_handle_list_conf_t extd_firewall_handle_list;
  qcmap_msgr_firewall_handle_conf_t extd_firewall_handle;
} qcmap_msgr_firewall_conf_t;

#endif
