/*
 * ----------------------------------------------------------------------------
 *  Copyright (c) 2010 Qualcomm Technologies, Inc.
 *  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
 *  ---------------------------------------------------------------------------
 *
 */

#define LOG_TAG "usbhub_init"
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
    char target_type[PROPERTY_VALUE_MAX];
    char current_hub_state[PROPERTY_VALUE_MAX];
    char current_pin_value[3];
    (void)argc; (void)argv;

    property_get("ro.board.platform",target_type,"msmdefault");
    if(!strncmp(target_type,"msmdefault",10)) {
        fprintf(stderr, "Target type not known before exporting gpio");
        exit(0);
    }

    //Export GPIO56 corresponding to USBHUB to userspace for fusion targets
    //For non fusion targets just return without exporting
    if(strncmp(target_type,"msm7630_fusion",14))
        return 0;

    int fd = open("/sys/class/gpio/export",O_WRONLY);
    const char* gpio_number = "56";
    if(fd>=0){
        write(fd,gpio_number,strlen(gpio_number));
    }
    close(fd);

    //Set the GPIO56 direction to out
    int fd_dir = open("/sys/class/gpio/gpio56/direction",O_WRONLY);
    if(fd_dir<0){
        LOGE("Error in opening file /sys/class/gpio/gpio56/direction");
        exit(0);
    }

    //Writing high is same as output-high. Writing out is equivalent to output-low
    write(fd_dir,"high",4);
    close(fd_dir);

    //Reboot with hub state based on UI option selected before shutdown.
    property_get("persist.sys.usbhubstate",current_hub_state,"--");
    LOGE("persist.sys.usbhubstate property value during usb hub initialization = %s", current_hub_state);
    fd = open("/sys/class/gpio/gpio56/value",O_RDWR);
    if(fd<0){
        LOGE("Error in opening file /sys/class/gpio/gpio56/value");
        exit(0);
    }

    int status = read(fd, current_pin_value, 1);
    if(status>0)
        LOGE("UsbHub: current_pin_value=%c",current_pin_value[0]);
    //Write only if previous value of pin is different
    if(!strncmp(current_hub_state, "true", 4) && strncmp(current_pin_value, "1", 1)){
        write(fd, "1", 1);
        LOGE("UsbHub: Value 1 written");
    }
    else if(!strncmp(current_hub_state, "false", 5) && strncmp(current_pin_value, "0", 1)){
        write(fd, "0", 1);
        LOGE("UsbHub: Value 0 written");
    }
    else {
        if(!strncmp(current_hub_state, "true", 4) || !strncmp(current_hub_state, "false", 5))
            LOGE("UsbHub: No change in USB Hub state is needed");
        else
            LOGE("UsbHub: persist.sys.usbhubstate is in undefined state");
    }
    close(fd);

    return 0;
}
