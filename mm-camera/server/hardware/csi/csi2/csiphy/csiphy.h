/*============================================================================

   Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __CSIPHY_H__
#define __CSIPHY_H__

#include "csi.h"

int csiphy_process_init(csi_t *csi_obj);
int csiphy_set_cfg(csi_t *csi_obj, enum msm_sensor_resolution_t res);

#endif
