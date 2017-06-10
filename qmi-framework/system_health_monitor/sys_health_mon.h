/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
 Copyright (c) 2014 Qualcomm Technologies, Inc.
 All Rights Reserved.  Qualcomm Technologies Proprietary and Confidential.
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
#ifndef _SYS_HEALTH_MON_H_
#define _SYS_HEALTH_MON_H_

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * check_system_health() - Check the system health
 *
 * @return: 0 on success, -1 on failure.
 *
 * This function is used by the user-space applications to initiate the
 * system health check. This function in turn trigger SHM to send QMI
 * message to all the HMAs connected to it.
 */
int check_system_health(void);

#ifdef __cplusplus
}
#endif

#endif
