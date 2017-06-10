/*===========================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

===========================================================================*/
#ifndef __SENSOR_THERMAL_SYS_H__
#define __SENSOR_THERMAL_SYS_H__

#include "sensors_manager_internal.h"

int thermal_sensor_setup(struct sensor_info *sensor);
void thermal_sensor_shutdown(struct sensor_info *sensor);
int thermal_sensor_get_temperature(struct sensor_info *sensor);
void thermal_sensor_interrupt_wait(struct sensor_info *sensor);

#endif  /* __SENSOR_THERMAL_SYS_H__ */
