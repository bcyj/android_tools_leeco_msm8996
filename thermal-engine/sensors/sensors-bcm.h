/*===========================================================================

  Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

===========================================================================*/
#ifndef __BCM_SENSOR_H__
#define __BCM_SENSOR_H__

#include "sensors_manager_internal.h"

int bcm_setup(struct sensor_info *sensor);
void bcm_shutdown(struct sensor_info *sensor);
int bcm_get_current(struct sensor_info *sensor);
void bcm_interrupt_wait(struct sensor_info *sensor);
void bcm_update_thresholds(struct sensor_info *sensor, struct thresholds_req_t *thresh);

#endif  /* __BCM_SENSOR_H__ */
