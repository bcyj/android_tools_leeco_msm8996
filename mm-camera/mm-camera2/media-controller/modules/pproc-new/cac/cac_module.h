/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __CAC_MODULE_H__
#define __CAC_MODULE_H__

#include <pthread.h>
#include "mct_queue.h"
#include "mct_list.h"
#include "media_controller.h"
#include "mct_port.h"
#include "mct_object.h"
#include "cam_types.h"
#include "mct_module.h"
#include "mct_pipeline.h"
#include "mct_stream.h"
#include "camera_dbg.h"

mct_module_t* cac_module_init(const char *name);
void cac_module_deinit(mct_module_t *mod);

#endif//__CAC_MODULE_H__
