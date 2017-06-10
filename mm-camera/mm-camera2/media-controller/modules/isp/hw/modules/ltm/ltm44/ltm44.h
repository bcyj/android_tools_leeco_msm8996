/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __LTM44_H__
#define __LTM44_H__

#include "camera_dbg.h"
#include "isp_event.h"
#include "ltm44_reg.h"
#include "isp_hw_module_ops.h"
#include "isp_pipeline.h"
#include "isp_pipeline_util.h"
#include "chromatix.h"

typedef struct {
  int8_t table_updated;
} ltm_parms_t;

typedef struct {
  isp_ops_t ops;
  isp_notify_ops_t *notify_ops;

  /* Module Control */
  uint8_t hw_update_pending;
  uint8_t trigger_enable; /* enable trigger update feature flag from PIX*/
  uint8_t skip_trigger; /* skip the trigger update */
  uint8_t enable;         /* enable flag from PIX */

} isp_ltm_mod_t;

#endif //__LTM44_H__
