
#libchromatix_vx6953_preview.so
lib_LTLIBRARIES := libchromatix_vx6953_preview.la

libchromatix_vx6953_preview_la_CFLAGS := $(DEBUG_CFLAGS)
libchromatix_vx6953_preview_la_CFLAGS += -DMSM_CAMERA_GCC
libchromatix_vx6953_preview_la_CFLAGS += -D_GNU_SOURCE
libchromatix_vx6953_preview_la_CFLAGS += -DLINUX_ENABLED
libchromatix_vx6953_preview_la_CFLAGS += -fPIC

libchromatix_vx6953_preview_la_CFLAGS += -I ../../..
libchromatix_vx6953_preview_la_CFLAGS += -I ../../../../..
libchromatix_vx6953_preview_la_CFLAGS += -I ../../../../../hardware/sensor
libchromatix_vx6953_preview_la_CFLAGS += -I ../../../../../hardware/flash
libchromatix_vx6953_preview_la_CFLAGS += -I ../../../../../hardware/flash/xenon
libchromatix_vx6953_preview_la_CFLAGS += -I ../../../../../isp3a
libchromatix_vx6953_preview_la_CFLAGS += -I ../../../../../../common
#for jpeg
libchromatix_vx6953_preview_la_CFLAGS += -I ../../../../../../../mm-still/jpeg/inc

libchromatix_vx6953_preview_la_SOURCES := chromatix_vx6953_preview.c

libchromatix_vx6953_preview_la_LDFLAGS := $(DEBUG_LDFLAGS)
libchromatix_vx6953_preview_la_LDFLAGS += -shared

