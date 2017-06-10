/*===========================================================================

  Copyright (c) 2012 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

===========================================================================*/
#ifndef __BCL_SENSOR_H__
#define __BCL_SENSOR_H__

#include "sensors_manager_internal.h"

int bcl_setup(struct sensor_info *sensor);
void bcl_shutdown(struct sensor_info *sensor);
int bcl_get_iavail(struct sensor_info *sensor);
int bcl_get_soc(struct sensor_info *sensor);
void bcl_enable(struct sensor_info *sensor, int enabled);
void bcl_interrupt_wait(struct sensor_info *sensor);
void bcl_update_thresholds(struct sensor_info *sensor, struct thresholds_req_t *thresh);

#endif  /* __BCL_SENSOR_H__ */
