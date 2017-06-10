/**
  @file
  port_bridge.h

  @brief
  Main port_bridge application which bridges communication
  between an external host port and a modem port.
*/

/*===========================================================================

  Copyright (c) 2011-2014 Qualcomm Technologies, Inc. All Rights Reserved

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

#ifndef PORT_BRIDGE_H
#define PORT_BRIDGE_H
#include <errno.h>

#ifdef LE_PORT_BRIDGE_DBG
#ifndef LOGE
#define LOGE(...) fprintf(stderr, "E/PORT-BRIDGE" __VA_ARGS__)
#endif
#ifndef LOGI
#define LOGI(...) fprintf(stderr, "I/PORT-BRIDGE" __VA_ARGS__)
#endif
#ifndef LOGD
#define LOGD(...) fprintf(stderr, "D/PORT-BRIDGE" __VA_ARGS__)
#endif
#else
#define LOG_NIDEBUG 0
#include <utils/Log.h>
#include "common_log.h"
#endif

#ifdef LOG_TAG
#undef LOG_TAG
#define LOG_TAG  "PORT-BRIDGE"
#endif

#define DUN_MAXBUFSIZE 1024
#define SLEEP_INTERVAL  (1)
#define DUN_SUCCESS  (0)
#define DUN_ERROR    (-1)

typedef enum
{
  DUN_ATCMD_INVALID = -1,
  DUN_ATCMD_START,
  DUN_ATCMD_STOP,
  DUN_ATCMD_MAX
} dun_atcmd_t;

typedef struct
{
  char modem[100];
  char host[100];
  int modem_fd;
  int host_fd;
  pthread_t portsmonitor_thread;
  pthread_t portdataxfr_thread;
} dun_portparams_t;

typedef enum
{
  DUN_STATE_ERROR = -1,
  DUN_STATE_USB_UNPLUG,
  DUN_STATE_IDLE,
  DUN_STATE_CONNECTED,
  DUN_STATE_MAX
} dun_state_t;

typedef enum
{
  DUN_EVENT_ERROR = -1,
  DUN_EVENT_USB_UNPLUG,
  DUN_EVENT_USB_PLUG,
  DUN_EVENT_START,
  DUN_EVENT_STOP,
  DUN_EVENT_MAX
} dun_event_t;

typedef struct
{
  dun_event_t event;
  char        *unused;
} dun_event_msg_t;

/* USB states */
typedef enum
{
  DUN_USBSTATE_ERROR = -1,
  DUN_USBSTATE_UNPLUG,
  DUN_USBSTATE_PLUG,
  DUN_USBSTATE_MAX
} dun_usbstate_t;

typedef enum
{
  DUN_USBMODE_INVALID = -1,
  DUN_USBMODE_NOT_SERIAL,
  DUN_USBMODE_SERIAL,
  DUN_USBMODE_MAX
} dun_usbmode_t;

typedef struct
{
  dun_usbstate_t  usb_state;
  dun_usbmode_t   usb_mode;
} dun_usb_t;

extern int dun_ctrl_pipefds[2];

/* for debugging use */
extern char *DUN_EVENT_STR[DUN_EVENT_MAX + 1];
extern char *DUN_STATE_STR[DUN_STATE_MAX + 1];

extern dun_portparams_t dun_portparams;
extern pthread_mutex_t  dun_post_event_mutex;

/*===========================================================================
  FUNCTION  dun_monitor_usbkevents
============================================================================*/
/*!
@brief
  Thread to monitor kernel events to detect USB plug/unplug events

@param
  arg  Argument passed as part of thread creation

@return
  None
*/
/*===========================================================================*/
extern void* dun_monitor_usbkevents(void *arg);

/*===========================================================================
  FUNCTION  dun_start_ports_threads
============================================================================*/
/*!
@brief
  Starts the ports monitor and data transfer threads

@param
  pportparams  Structure holding information on the devices

@return
  int - DUN_SUCCESS if successful
        DUN_ERROR if there was an error
*/
/*==========================================================================*/
extern int dun_start_ports_threads(dun_portparams_t *);

/*===========================================================================
  FUNCTION  dun_stop_ports_threads
============================================================================*/
/*!
@brief
  Stops the threads

@param
  pportparams  Structure holding information on the devices

@return
  int - DUN_SUCCESS if successful
        DUN_ERROR if there was an error
*/
/*===========================================================================*/
extern int dun_stop_ports_threads(dun_portparams_t *);

/*===========================================================================
  FUNCTION  dun_post_event
============================================================================*/
/*!
@brief
  Posts event to the DUN state machine

@param
  event  The event to be posted

@return
  None
*/
/*===========================================================================*/
extern void dun_post_event(dun_event_t event);

#endif /* PORT_BRIDGE_H */


