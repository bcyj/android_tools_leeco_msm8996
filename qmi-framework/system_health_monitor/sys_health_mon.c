/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
 Copyright (c) 2014 Qualcomm Technologies, Inc.
 All Rights Reserved.  Qualcomm Technologies Proprietary and Confidential.
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/ioctl.h>

#ifdef SHM_ANDROID
#include <utils/Log.h>
#include "common_log.h"
#define LOG_TAG "SHM"

#define SHM_LOG_DEBUG(x...) do { \
    SLOGD(x); \
} while(0)

#define SHM_LOG_ERR(x...) do { \
    LOGE(x); \
} while (0)

#else

#define SHM_LOG_DEBUG(x...) {}
#define SHM_LOG_ERR(x...) {}

#endif

#define SYSTEM_HEALTH_MONITOR_IOCTL_MAGIC (0xC3)

#define CHECK_SYSTEM_HEALTH_IOCTL \
        _IOR(SYSTEM_HEALTH_MONITOR_IOCTL_MAGIC, 0, unsigned int)

#define DEV_PATH "/dev/system_health_monitor"

/**
 * check_system_health() - Check the system health
 *
 * @return: 0 on success, -1 on failure.
 *
 * This function is used by the user-space applications to initiate the
 * system health check. This function in turn trigger SHM to send QMI
 * message to all the HMAs connected to it.
 */
int check_system_health(void)
{
    int fd;
    int rc;

    fd = open(DEV_PATH, O_RDONLY | O_CLOEXEC | O_NONBLOCK);
    if (fd <= 0) {
        SHM_LOG_ERR("%s: Failed to open %s - errno %d\n",
                    __func__, DEV_PATH, errno);
        return -1;
    }

    rc = ioctl(fd, CHECK_SYSTEM_HEALTH_IOCTL, NULL);
    if (rc < 0)
        SHM_LOG_ERR("%s: Failed to initiate health check - errno %d\n",
                    __func__, errno);
    else
        SHM_LOG_DEBUG("%s: Successfully initiated health check\n", __func__);

    close(fd);
    return 0;
}
