/*
 * Copyright (c) 2014, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#ifndef __MMI_AGENT_H__
#define __MMI_AGENT_H__

#include "mmi_module.h"

#ifdef LOG_TAG
#undef LOG_TAG
#define LOG_TAG   "mmia"
#endif

/**handle socket command*/
int handle_init(mmi_module_t * module, msg_t * req, msg_t * resp);
int handle_deinit(mmi_module_t * module, msg_t * req, msg_t * resp);
int handle_query(mmi_module_t * module, msg_t * req, msg_t * resp);
int handle_run(mmi_module_t * module, msg_t * req, msg_t * resp);
int handle_stop(mmi_module_t * module, msg_t * req, msg_t * resp);
int handle_ctrl(mmi_module_t * module, msg_t * req, msg_t * resp);
#endif
