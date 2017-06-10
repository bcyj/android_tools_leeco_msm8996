/*===========================================================================

  Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

===========================================================================*/
#ifndef __SENSOR_PM8821_H__
#define __SENSOR_PM8821_H__

#include "sensors_manager_internal.h"

int pm8821_setup(struct sensor_info *sensor);
void pm8821_shutdown(struct sensor_info *sensor);
int pm8821_get_temperature(struct sensor_info *sensor);
void pm8821_interrupt_wait(struct sensor_info *sensor);

#endif  /* __SENSOR_PM8821_H__ */
