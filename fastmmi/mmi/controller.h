/*
 * Copyright (c) 2014, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#ifndef __CONTROLLER_H
#define __CONTROLLER_H

int handle_ctr_msg(msg_t * msg, module_info * mod);
void notify_controller(module_info * mod);

#endif
