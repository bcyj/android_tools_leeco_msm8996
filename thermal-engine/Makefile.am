if ENABLE_THERMAL_ENGINE
AM_CFLAGS = -Wall \
		-Wundef \
		-Wstrict-prototypes \
		-Wno-trigraphs \
		-Werror \
		-g -O0 \
		-fno-inline \
		-fno-short-enums \
		-fpic

AM_CPPFLAGS = -D_GNU_SOURCE \
	      -DCONFIG_FILE_DEFAULT='"/etc/thermal-engine.conf"' \
	      -I./inc

sources = \
	 ./thermal.c \
	 ./thermal_util.c \
	 ./thermal_monitor.c \
	 ./thermal_monitor-data.c \
	 ./sensors/sensors-tsens.c \
	 ./sensors/sensors_manager.c \
	 ./devices/devices_manager.c \
	 ./devices/devices.c \
	 ./devices/devices_actions.c

if IS_MDM9625
sources += ./sensors/sensors-9625.c
AM_CPPFLAGS += -DENABLE_TSENS_INTERRUPT \
	      -DSENSORS_9625
endif

if IS_MDM9635
sources += ./sensors/sensors-9625.c
AM_CPPFLAGS += -DENABLE_TSENS_INTERRUPT \
	      -DSENSORS_9635
endif

if IS_MSM8974
sources += \
	 ./sensors/sensors-adc.c \
	 ./sensors/sensors-gen.c \
	 ./sensors/sensors-bcl.c \
	 ./sensors/sensors-thermal.c \
	 ./sensors/sensors-8974.c \
	 ./pid_algorithm.c \
	 ./pid-data.c \
	 ./ss_algorithm.c \
	 ./ss-data.c \
	 ./server/thermal_server.c \
	 ./server/thermal_lib_common.c \
	 ./speaker_cal.c \
	 ./speaker_cal-data.c \
	 ./sensors/sensors-virtual.c

AM_CPPFLAGS += -DSENSORS_8974 \
	      -DENABLE_TSENS_INTERRUPT \
	      -DENABLE_PID \
	      -DENABLE_SS \
	      -DENABLE_THERMAL_SERVER \
	      -DENABLE_SPEAKER_CAL \
	      -DENABLE_VIRTUAL_SENSORS
endif


if IS_MSM8610
sources += \
	 ./sensors/sensors-adc.c \
	 ./sensors/sensors-gen.c \
	 ./sensors/sensors-bcl.c \
	 ./sensors/sensors-8610.c \
	 ./pid_algorithm.c \
	 ./pid-data.c \
	 ./ss_algorithm.c \
	 ./ss-data.c \
	 ./server/thermal_server.c \
	 ./server/thermal_lib_common.c \
	 ./sensors/sensors-virtual.c

AM_CPPFLAGS += -DSENSORS_8610 \
	      -DENABLE_TSENS_INTERRUPT \
	      -DENABLE_PID \
	      -DENABLE_SS \
	      -DENABLE_THERMAL_SERVER \
	      -DENABLE_VIRTUAL_SENSORS
endif

if IS_MSM8226
sources += \
	 ./sensors/sensors-adc.c \
	 ./sensors/sensors-gen.c \
	 ./sensors/sensors-bcl.c \
	 ./sensors/sensors-8226.c \
	 ./pid_algorithm.c \
	 ./pid-data.c \
	 ./ss_algorithm.c \
	 ./ss-data.c \
	 ./server/thermal_server.c \
	 ./server/thermal_lib_common.c \
	 ./sensors/sensors-virtual.c

AM_CPPFLAGS += -DSENSORS_8226 \
	      -DENABLE_TSENS_INTERRUPT \
	      -DENABLE_PID \
	      -DENABLE_SS \
	      -DENABLE_THERMAL_SERVER \
	      -DENABLE_VIRTUAL_SENSORS
endif

if ENABLE_OLD_PARSER
sources += ./thermal_config.c
AM_CPPFLAGS += -DENABLE_OLD_PARSER
else
sources += ./thermal_config_v2.c
endif

if USE_QMI
AM_CPPFLAGS += $(QMIF_CFLAGS) \
	       $(QMI_CFLAGS)

local_LDFLAGS = $(QMIF_LIBS) $(QMI_LIBS)

if QMI_TMD
AM_CPPFLAGS += -DENABLE_MODEM_MITIGATION
sources += ./devices/devices-qmi.c \
	   ./devices/thermal_mitigation_device_service_v01.c
endif

if QMI_TS
AM_CPPFLAGS += -DENABLE_MODEM_TS
sources += ./sensors/sensors-qmi.c \
	   ./sensors/thermal_sensor_service_v01.c
endif
endif

if USE_GLIB
common_CFLAGS =  -DUSE_GLIB @GLIB_CFLAGS@
common_LDFLAGS = -lpthread -lrt @GLIB_LIBS@
else
common_LDFLAGS = -lpthread -lrt
endif

thermal_engine_SOURCES = $(sources)

thermal_engine_CFLAGS = $(AM_CFLAGS) $(AM_CPPFLAGS) $(common_CFLAGS)
thermal_engine_LDFLAGS = -lpthread -lrt $(common_LDFLAGS) $(local_LDFLAGS)

bin_PROGRAMS = thermal-engine

#Export config file to /etc/
if IS_MSM8974
sysconf_DATA = thermal-engine-8974.conf
endif

if IS_MSM8610
sysconf_DATA = thermal-engine-8610.conf
endif

if IS_MSM8226
sysconf_DATA = thermal-engine-8226.conf
endif

#Install thermal client library
if ENABLE_THERMAL_CLIENT_LIB
lib_LTLIBRARIES:= libthermalclient.la

libthermalclient_la_CFLAGS := $(AM_CFLAGS) $(common_CFLAGS)
libthermalclient_la_CFLAGS += -I$(WORKSPACE)/common/inc
libthermalclient_la_CFLAGS += -I./inc -I./server

libthermalclient_la_SOURCES := ./client/thermal_client.c ./server/thermal_lib_common.c

libthermalclient_la_LDFLAGS := -shared $(common_LDFLAGS)

libthermalclient_la_LIBADD := -lcutils
endif
endif

