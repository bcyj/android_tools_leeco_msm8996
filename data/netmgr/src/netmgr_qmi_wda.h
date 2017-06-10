/******************************************************************************

                       N E T M G R _ Q M I _ W D A . H

******************************************************************************/

/******************************************************************************

  @file    netmgr_qmi_wda.h
  @brief   Netmanager QMI Wireless Data Administrative helper

  DESCRIPTION
  Netmanager QMI Wireless Data Administrative helper

******************************************************************************/
/*===========================================================================

  Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved

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

#ifndef __NETMGR_QMI_WDA_H__
#define __NETMGR_QMI_WDA_H__

/* Need below include for wda_set_data_format_req_msg_v01 */
#include "wireless_data_administrative_service_v01.h"

#define NETMGR_WDA_SUCCESS        0
#define NETMGR_WDA_BAD_ARGUMENTS  1
#define NETMGR_WDA_QMI_FAILURE    2
#define NETMGR_WDA_MODEM_REJECTED 3

#define NETMGR_WDA_UL_QMAP      0x05
#define NETMGR_WDA_DL_QMAP      0x05
#define NETMGR_WDA_DL_QMAPV3    0x07

int netmgr_wda_set_data_format(const char *dev_id,
                               wda_set_data_format_req_msg_v01 *request,
                               wda_set_data_format_resp_msg_v01 *response);

int netmgr_wda_set_qmap_settings(const char *dev_id,
                                 wda_set_qmap_settings_req_msg_v01  *request,
                                 wda_set_qmap_settings_resp_msg_v01 *response);

#endif /* __NETMGR_QMI_WDA_H__ */
