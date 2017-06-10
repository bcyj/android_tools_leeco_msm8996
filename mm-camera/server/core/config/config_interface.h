/*============================================================================
   Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#ifndef __CONFIG_INTERFACE_H__

struct config_interface_t {
  int (*config_request)(void *parm1, void *parm2, int *cmdPending);
  int (*config_proc_event_message)(void *parm1, void *parm2);
};

struct config_interface_t* create_config_intf(uint8_t has_ispif);
void destroy_config_intf(struct config_interface_t *intf);

#endif /* __CONFIG_INTERFACE_H__ */
