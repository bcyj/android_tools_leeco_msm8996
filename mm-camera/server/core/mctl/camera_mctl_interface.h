/*============================================================================
   Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential                         .
                                                                 .
   This file implements the camera mctl client and observer logic for the.
   mm-camera server.
============================================================================*/

#ifndef __MCTL_INTERFACE_H__
#define __MCTL_INTERFACE_H__

#include "camera.h"
#include "camera_plugin_intf.h"

typedef struct {
  /* Launch MCTL thread, pipe for communication is sent through
   * config_thread_arguments
   */
  void* (*launch_mctl_thread)(struct config_thread_arguments* arg);
  /* Release mctl thread */
  int (*release_mctl_thread)(void *);
  /* check if the camera resources are available */
  int (*camera_available) (int resource_id);
  /* initialize plugin*/
  int (*init_camera_plugin)(void* ,camera_plugin_ops_t *);
  /* server fd */
  int server_fd;
  /* plugin ops */
  camera_plugin_ops_t *p_plugin_ops;
} camera_mctl_svr_ops_t;

typedef struct {
  /* Operations performed on server side on behalf of client */
  camera_mctl_svr_ops_t svr_ops;
  /* Operations performed on client side on behalf of server */
  camera_mctl_client_ops_t client_ops;
} camera_mctl_client_t;

typedef enum {
  CAM_MCTL_CB_NOTIFY_FRAME,
  CAM_MCTL_CB_NOTIFY_CTRL,
} camera_mctl_event_type_t;

typedef struct {
  camera_mctl_event_type_t type;
  cam_ctrl_type camctrl_type;
  void *p_data;
  int len;
} camera_mctl_event_t;

typedef struct {
  int (*notify) (camera_mctl_event_t *p_evt);
} camera_mctl_observer_t;

#endif //__MCTL_INTERFACE_H__
