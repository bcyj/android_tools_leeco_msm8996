/*===========================================================================

 Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 Qualcomm Technologies Proprietary and Confidential.

===========================================================================*/
#ifndef __THERMAL_IOCTL_INTERFACE_H__
#define __THERMAL_IOCTL_INTERFACE_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef ENABLE_THERMAL_IOCTL
int thermal_ioctl_init(void);
int thermal_ioctl_defined(void);
int thermal_ioctl_set_frequency(uint32_t cpu_num, uint32_t freq_request,
	uint8_t max_freq);
int thermal_ioctl_set_cluster_frequency(uint32_t cluster_num,
	uint32_t freq_request, uint8_t max_freq);
int thermal_ioctl_get_cluster_freq_plan(int cluster_num, uint32_t *arr,
	uint32_t *len);
void thermal_ioctl_release(void);
#else
static inline int thermal_ioctl_init(void)
{
	return -ENOSYS;
};
static inline int thermal_ioctl_defined(void)
{
	return -ENOSYS;
};
static inline int thermal_ioctl_set_frequency(uint32_t cpu_num, uint32_t freq_request,
	uint8_t max_freq)
{
	return -ENOSYS;
};
static inline int thermal_ioctl_set_cluster_frequency(uint32_t cluster_num,
	uint32_t freq_request, uint8_t max_freq)
{
	return -ENOSYS;
};
static inline int thermal_ioctl_get_cluster_freq_plan(int cluster_num, uint32_t *arr,
        uint32_t *len)
{
	return -ENOSYS;
};

static inline void thermal_ioctl_release(void)
{
};
#endif

#ifdef __cplusplus
}
#endif

#endif /* __THERMAL_IOCTL_INTERFACE_H__ */
