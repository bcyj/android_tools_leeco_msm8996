/*===========================================================================

  Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

===========================================================================*/

#ifndef __GEN_SENSOR_H__
#define __GEN_SENSOR_H__

#include "sensors_manager_internal.h"

int gen_sensors_setup(struct sensor_info * sensor);
void gen_sensors_shutdown(struct sensor_info * sensor);
int gen_sensor_get_temperature(struct sensor_info * sensor);

#endif  /* __GEN_SENSOR_H__ */
