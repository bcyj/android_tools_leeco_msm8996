/*==========================================================================

                     FTM WLAN Header File

Description
  The header file includes enums, struct definitions for WLAN FTM packets

# Copyright (c) 2010-2011, 2014 by Qualcomm Technologies, Inc.
# All Rights Reserved.
# Qualcomm Technologies Proprietary and Confidential.

===========================================================================*/

/*===========================================================================

                         Edit History


when       who       what, where, why
--------   ---       ----------------------------------------------------------
07/11/11   karthikm  Created header file to include enums, struct for WLAN FTM
                     for Atheros support
========================================================================*/

#ifndef  FTM_WLAN_H_
#define  FTM_WLAN_H_

#ifdef CONFIG_FTM_WLAN

#include "diagpkt.h"
#include <sys/types.h>

#define FTM_WLAN_CMD_CODE 22

extern char g_ifname[];

/* Various ERROR CODES supported by the FTM WLAN module*/
typedef enum {
    FTM_ERR_CODE_PASS = 0,
    FTM_ERR_CODE_IOCTL_FAIL,
    FTM_ERR_CODE_SOCK_FAIL,
    FTM_ERR_CODE_UNRECOG_FTM
}FTM_WLAN_ERROR_CODES;


#define CONFIG_HOST_TCMD_SUPPORT  1
#define AR6000_IOCTL_SUPPORTED    1

#define ATH_MAC_LEN               6
/* FTM WLAN request type */
typedef PACKED struct
{
  diagpkt_cmd_code_type              cmd_code;
  diagpkt_subsys_id_type             subsys_id;
  diagpkt_subsys_cmd_code_type       subsys_cmd_code;
  uint16                             cmd_id; /* command id (required) */
  uint16                             cmd_data_len;
  uint16                             cmd_rsp_pkt_size;
  uint16                             rsvd;
  byte                               rsvd1;
  byte                               rsvd2;
  byte                               rsvd3;
  byte                               wlandeviceno;
  byte                               data[1];
}__attribute__((packed))ftm_wlan_pkt_type;

/* FTM WLAN response types */

typedef PACKED struct
{
  diagpkt_subsys_header_v2_type  header ; /*Diag header*/
  uint32                         result ;/* result */
}__attribute__((packed))ftm_wlan_gen_rsp_type;

typedef PACKED struct
{
  diagpkt_subsys_header_v2_type  header ; /*Diag header*/
  uint32                         result ;/* result */
  byte                              data[1]; /*rxReport*/
}__attribute__((packed))ftm_wlan_rx_rsp_type;

typedef PACKED struct
{
  diagpkt_subsys_header_v2_type  header ; /*Diag header*/
  uint32                         result ;/* result */
  byte                              data[1]; /*ThermValReport*/
}__attribute__((packed))ftm_wlan_thermval_rsp_type;

void* ftm_wlan_dispatch(ftm_wlan_pkt_type *wlan_ftm_pkt, int pkt_len);

#endif /* CONFIG_FTM_WLAN */
#endif /* FTM_WLAN_H_ */
