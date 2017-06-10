/*
 * ----------------------------------------------------------------------------
 *  Copyright (c) 2010 Qualcomm Technologies, Inc.
 *  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
 *  ---------------------------------------------------------------------------
 *
 */

#define LOG_TAG "usbhub"
#include <cutils/log.h>

#include <cutils/properties.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <sys/wait.h>
#include <pthread.h>
#include <termios.h>
#include <unistd.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <net/if.h>
#include <termios.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <signal.h>
#include "common_log.h"

int main (int argc, char **argv)
{
    const char* gpio_value = "0";
    char current_pin_value[3];

    if (argc == 2 && 0 == strncmp(argv[1], "--on", 4)) {
        gpio_value = "1";
        property_set("persist.sys.usbhubstate","true");
        LOGE("usbhub --on command called");
    }
    else if (argc == 2 && 0 == strncmp(argv[1], "--off", 5)) {
        gpio_value = "0";
        property_set("persist.sys.usbhubstate","false");
        LOGE("usbhub --off command called");
    }
    else {
        LOGE("Incorrect usbhub option selected. Select --on or --off");
        exit(0);
    }

    int fd = open("/sys/class/gpio/gpio56/value",O_RDWR);
    if(fd < 0) {
        LOGE("Error in opening file /sys/class/gpio/gpio56/value");
        exit(0);
    }
    (void)read(fd, current_pin_value, 1);
    //Write only if previous value of pin is different
    if(strncmp(current_pin_value, gpio_value, 1)) {
        write(fd, gpio_value, strlen(gpio_value));
    }
    close(fd);

    return 0;
}
