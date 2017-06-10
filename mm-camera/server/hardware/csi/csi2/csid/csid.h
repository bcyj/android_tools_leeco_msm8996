/*============================================================================

   Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __CSID_H__
#define __CSID_H__

#include "csi.h"

int csid_process_init(csi_t *csi_obj, uint32_t *csid_version);
int csid_set_cfg(csi_t *csi_obj, enum msm_sensor_resolution_t res);

#endif
