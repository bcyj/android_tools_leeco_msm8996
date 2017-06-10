/*===========================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

===========================================================================*/

#ifndef __VIRTUAL_SENSORS_H__
#define __VIRTUAL_SENSORS_H__

#ifdef ENABLE_VIRTUAL_SENSORS
int virtual_sensors_init(struct thermal_setting_t *thermal_setting, const char *pFName);
#else
static inline int virtual_sensors_init(struct thermal_setting_t *thermal_setting, const char *pFName) {return 0;}
#endif /* ENABLE_VIRTUAL_SENSORS */

#endif /* __VIRTUAL_SENSORS_H__ */
