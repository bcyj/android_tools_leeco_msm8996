/*===========================================================================

  Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

===========================================================================*/

#ifndef __PID_ALGORITHM_H__
#define __PID_ALGORITHM_H__
#include "thermal_config.h"

#ifdef ENABLE_PID
void pid_init_data(struct thermal_setting_t *setting);
int pid_algo_init(struct thermal_setting_t *setting);
#else
static inline void pid_init_data(struct thermal_setting_t *setting){}
static inline int pid_algo_init(struct thermal_setting_t *setting) {return 0;}
#endif /* ENABLE_PID */

#endif /* __PID_ALGORITHM_H__ */
