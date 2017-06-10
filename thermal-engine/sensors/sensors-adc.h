/*===========================================================================

  Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

===========================================================================*/

#ifndef __ADC_SENSOR_H__
#define __ADC_SENSOR_H__

#include "sensors_manager_internal.h"

int adc_sensors_setup(struct sensor_info *sensor);
void adc_sensors_shutdown(struct sensor_info *sensor);
int adc_sensor_get_temperature(struct sensor_info *sensor);

#endif  /* __ADC_SENSOR_H__ */
