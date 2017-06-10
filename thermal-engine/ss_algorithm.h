/*===========================================================================

Copyright (c) 2012-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.

===========================================================================*/

#ifndef __SS_ALGORITHM_H__
#define __SS_ALGORITHM_H__
#include "thermal_config.h"

#ifdef ENABLE_SS
void ss_init_data(struct thermal_setting_t *setting);
int ss_algo_init(struct thermal_setting_t *setting);
#else
static inline void ss_init_data(struct thermal_setting_t *setting){}
static inline int ss_algo_init(struct thermal_setting_t *setting) {return 0;}
#endif /* ENABLE_SS */

#endif /* __SS_ALGORITHM_H__ */
