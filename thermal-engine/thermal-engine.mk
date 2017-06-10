################################################################################
# -----------------------------------------------------------------------------
# Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
# Qualcomm Technologies Proprietary and Confidential.
# -----------------------------------------------------------------------------
################################################################################

vpath %.c $(SRCDIR)

APP_NAME := thermal-engine

SRCLIST :=thermal.c thermal_monitor.c \
	thermal_monitor-data.c \
	thermal_util.c thermal_config_v2.c \
	./sensors/sensors-tsens.c ./sensors/sensors-fsm9900.c \
	./sensors/sensors-thermal.c ./sensors/sensors_manager.c \
	./sensors/sensors-gen.c ./sensors/sensors-adc.c \
	./devices/devices.c ./devices/devices_actions.c \
	./devices/devices_manager.c

CFLAGS  +=-DENABLE_PID
SRCLIST +=pid_algorithm.c pid-data.c

CFLAGS  +=-DENABLE_SS
SRCLIST +=ss_algorithm.c ss-data.c

CFLAGS  +=-DENABLE_THERMAL_SERVER
SRCLIST +=./server/thermal_server.c ./server/thermal_lib_common.c

CFLAGS  +=-DENABLE_VIRTUAL_SENSORS
SRCLIST +=./sensors/sensors-virtual.c

CFLAGS  +=$(QCT_CFLAGS) -I$(SRCDIR) -I$(SRCDIR)/inc -I$(SRCDIR)/server
CFLAGS  +=-Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -Werror
CFLAGS  +=-g -O0 -fno-inline -fno-short-enums -fpic

CFLAGS  +=-DSENSORS_FSM9900 -DENABLE_TSENS_INTERRUPT -D_GNU_SOURCE
CFLAGS  +=-DCONFIG_FILE_DEFAULT='"/mnt/flash/etc/thermal.conf"'

LIBS    +=-lpthread -lstringl -lm -lrt

all: $(APP_NAME)

$(APP_NAME): $(SRCLIST)
	@ pwd
	$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) $(LIBS) -o $@ $^ $(LDLIBS)

