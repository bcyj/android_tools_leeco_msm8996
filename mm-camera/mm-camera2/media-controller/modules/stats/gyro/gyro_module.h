/* gyro_module.h
 *
 * Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __GYRO_MODULE_H__
#define __GYRO_MODULE_H__
#include "mct_module.h"

mct_module_t* gyro_module_init(const char *name);
void gyro_module_deinit(mct_module_t *mod);
mct_port_t *gyro_module_get_port(mct_module_t *mod, unsigned int sessionid);
#endif /* __GYRO_MODULE_H__ */
