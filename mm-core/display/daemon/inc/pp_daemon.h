/*
 * Copyright (c) 2012-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef _PP_DAEMON_H
#define _PP_DAEMON_H

#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <cutils/properties.h>
#include <cutils/log.h>

#include "common_log.h"
#include <cutils/sockets.h>
#include <linux/msm_mdp.h>
#include <linux/fb.h>

#include "DummySensor.h"
#include "Calib.h"
#include "ConfigParameters.h"
#include "AD.h"
#include "ScreenRefresher.h"
#include "AbaContext.h"
#include "CABL.h"
#include "DaemonContext.h"
#include "PPDaemonUtils.h"

const char* SYS_BRIGHTNESS = "/sys/class/leds/lcd-backlight/brightness";
const char* SYS_BRIGHTNESS_ALT = "/sys/devices/virtual/graphics/fb0/msmfb_bl0/brightness";

#define LEVEL_CHANGE_SLEEP_DURATION 5
#define PARTIAL_UPDATE_BYPASS "persist.hwc.pubypass"

#define DAEMON_SOCKET "pps"

#endif /* _PP_DAEMON_H */
