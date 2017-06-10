/*===========================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

===========================================================================*/

#ifndef __SPEAKER_CAL_H__
#define __SPEAKER_CAL_H__
#include "thermal_config.h"

#ifdef ENABLE_SPEAKER_CAL
void speaker_cal_init_data(struct thermal_setting_t *setting);
int speaker_cal_init(struct thermal_setting_t *setting);
#else
static inline void speaker_cal_init_data(struct thermal_setting_t *setting) {}
static inline int speaker_cal_init(struct thermal_setting_t *setting) {return 0;}
#endif /* ENABLE_SPEAKER_CAL */


#endif /* __SPEAKER_CAL_H__ */
