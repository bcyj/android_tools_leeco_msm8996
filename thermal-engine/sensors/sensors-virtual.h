/*===========================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

===========================================================================*/
#ifndef __SENSORS_VIRTUAL_H__
#define __SENSORS_VIRTUAL_H__
#include "sensors_manager_internal.h"

int vs_setup(struct sensors_mgr_sensor_info *sensor);
void vs_shutdown(struct sensors_mgr_sensor_info *sensor);
int vs_get_temperature(struct sensors_mgr_sensor_info *sensor);
void vs_interrupt_wait(struct sensors_mgr_sensor_info *sensor);
void vs_update_thresholds(struct sensors_mgr_sensor_info *sensor,
		struct thresholds_req_t *thresh);
#endif  /* __SENSORS_VIRTUAL_H__ */
