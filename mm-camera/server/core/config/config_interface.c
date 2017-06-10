/*============================================================================
   Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

#include "config_interface.h"
#include "camera.h"
#include "config_proc.h"

int config_proc_event_message_0(void *parm1,  void *parm2);
int config_v2_proc_event_message_0(void *parm1,  void *parm2);
int config_v4l2_request(void *parm1, void *parm2, int *cmdPending);
int config_v2_v4l2_request(void *parm1, void *parm2, int *cmdPending);

struct config_interface_t* create_config_intf(uint8_t has_ispif)
{
  struct config_interface_t *config_intf =
    malloc(sizeof(struct config_interface_t));
  if (config_intf == NULL)
    return NULL;
  if (has_ispif) {
    config_intf->config_request = config_v2_v4l2_request;
    config_intf->config_proc_event_message = config_v2_proc_event_message_0;
  } else {
    config_intf->config_request = config_v4l2_request;
    config_intf->config_proc_event_message = config_proc_event_message_0;
  }
  return config_intf;
}

void destroy_config_intf(struct config_interface_t *intf)
{
  free(intf);
}
