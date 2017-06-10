/*===========================================================================

  Copyright (c) 2012-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

===========================================================================*/

#ifndef __DEVICE_ACTIONS_H__
#define __DEVICE_ACTIONS_H__

#include "thermal.h"

#define MAX_WLAN_MITIGATION_LEVEL (4)
#define MAX_BATTERY_MITIGATION_LEVEL  (3)
#define MAX_CAMERA_MITIGATION_LEVEL  (10)
#define MAX_CAMCORDER_MITIGATION_LEVEL  (10)
#define MAX_MDP_MITIGATION_LEVEL  (3)
#define MAX_VENUS_MITIGATION_LEVEL (3)

int cpufreq_init(void);
void clusterfreq_init(void);
int gpufreq_init(void);
void hotplug_init(void);
void vdd_dig_sw_mode_init(void);
void opt_curr_req_init(void);
void vdd_rstr_init(void);

int report_action(const char *msg);
int shutdown_action(int delay);
int cpufreq_request(int cpu, int frequency);
int clusterfreq_request(int cluster, int frequency);
int lcd_brightness_request(int value);
int battery_request(int level);
int gpufreq_request(int gpu, int level);
int wlan_request(int level);
int hotplug_request(int cpu, int offline);
int vdd_restriction_request(int request);
int kernel_mitigation_request(int request);
int camera_request(int request);
int camcorder_request(int request);
int vdd_dig_sw_mode_request(int mode);
int optimum_current_request(int mode);
int mdp_request(int level);
int venus_request(int level);

int get_cluster_id(int cluster_id);
int get_num_clusters(void);
int is_cluster_sync(int cluster_id);
int get_cores_in_cluster(int cluster_id);
#endif  /* __DEVICE_ACTIONS_H__ */
