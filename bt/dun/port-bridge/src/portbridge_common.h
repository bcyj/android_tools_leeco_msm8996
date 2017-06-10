/******************************************************************************

  @file    portbridge_common.h
  @brief   Main Portbridge Module Header

  DESCRIPTION
  Header for the the main thread of the port-bridge process.

 ******************************************************************************/
/*===========================================================================

  Copyright (c) 2010,2013 Qualcomm Technologies, Inc. All Rights Reserved

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

#ifndef __PORTBRIDGE_COMMON_H__
#define __PORTBRIDGE_COMMON_H__

#include <utils/Log.h>
#include "comdef.h"
//#include "ds_util.h"

#define LOG_TAG "BT_DUN"

/*Logging macros*/
#define port_log_dflt  LOGD
#define port_log_err   LOGE
#define port_log_high  LOGV
#define port_log_low   LOGI

/*File Descriptor Number for the Pipes*/
#define READ_FD_FOR_PIPE  0
#define WRITE_FD_FOR_PIPE 1

#define MAX_PHY_NET_DEV_NAME_LEN 20

/* Physical network device name */
extern char pb_dun_phys_net_dev_name[MAX_PHY_NET_DEV_NAME_LEN];

/*Core Events*/
typedef enum {
    DUN_EVENT_ERROR = 0,
    DUN_EVENT_EXT_HOST_CON,
    DUN_EVENT_EXT_HOST_DISCON,
    DUN_EVENT_START_CALL,
    DUN_EVENT_STOP_CALL,
    DUN_EVENT_READY_TO_CONNECT,
    DUN_EVENT_MAX,
} DUN_EVENT_E;

/*Platform Events*/
typedef enum {
    PLATFORM_EVENT_ERROR = 0,
    PLATFORM_EVENT_RMNET_UP,
    PLATFORM_EVENT_RMNET_DOWN,
    PLATFORM_EVENT_DUN_INITIATED,
    PLATFORM_EVENT_DUN_TERMINATED,
    PLATFORM_EVENT_MAX,
} PLATFORM_EVENT_E;

/*Core Event message structure*/
typedef struct {
    DUN_EVENT_E event;
} dun_event_msg_m;

/*Platform Event Message Structure*/
typedef struct {
    PLATFORM_EVENT_E event;
} platform_event_msg_m;

/*Inter-Process Event Message Structure*/
typedef struct {
    union{
        PLATFORM_EVENT_E p_event;
        DUN_EVENT_E d_event;
    } event;
} ipc_event_msg_m;

int pipe_lock(int fildes[2]);

#endif /* __PORTBRIDGE_COMMON_H__ */
