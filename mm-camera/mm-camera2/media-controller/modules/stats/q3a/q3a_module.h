/* q3a_module.h
 *
 * Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __Q3A_MODULE_H__
#define __Q3A_MODULE_H__

#include "mct_list.h"
#include "mct_module.h"
#include "mct_pipeline.h"

#include "mct_port.h"
#include "aec_module.h"
#include "awb_module.h"
#include "af_module.h"

mct_module_t *q3a_module_init(const char *name);
void         q3a_module_deinit(mct_module_t *module);
mct_port_t   *q3a_module_get_port(mct_module_t *module, unsigned int sessionid);

#endif /* __Q3A_MODULE_H__ */
