/* is_module.h
 *
 * Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __IS_MODULE_H__
#define __IS_MODULE_H__
#include "mct_module.h"

mct_module_t* is_module_init(const char *name);
void is_module_deinit(mct_module_t *mod);
mct_port_t *is_module_get_port(mct_module_t *mod, unsigned int sessionid);
#endif /* __IS_MODULE_H__ */
