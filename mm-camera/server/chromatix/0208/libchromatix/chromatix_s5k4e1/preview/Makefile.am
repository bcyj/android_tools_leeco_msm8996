
#libchromatix_s5k4e1_preview.so
lib_LTLIBRARIES := libchromatix_s5k4e1_preview.la

libchromatix_s5k4e1_preview_la_CFLAGS := $(DEBUG_CFLAGS)
libchromatix_s5k4e1_preview_la_CFLAGS += -DMSM_CAMERA_GCC
libchromatix_s5k4e1_preview_la_CFLAGS += -D_GNU_SOURCE
libchromatix_s5k4e1_preview_la_CFLAGS += -DLINUX_ENABLED
libchromatix_s5k4e1_preview_la_CFLAGS += -fPIC

libchromatix_s5k4e1_preview_la_CFLAGS += -I ../../..
libchromatix_s5k4e1_preview_la_CFLAGS += -I ../../../../..
libchromatix_s5k4e1_preview_la_CFLAGS += -I ../../../../../hardware/sensor
libchromatix_s5k4e1_preview_la_CFLAGS += -I ../../../../../hardware/flash
libchromatix_s5k4e1_preview_la_CFLAGS += -I ../../../../../hardware/flash/xenon
libchromatix_s5k4e1_preview_la_CFLAGS += -I ../../../../../isp3a
libchromatix_s5k4e1_preview_la_CFLAGS += -I ../../../../../../common
#for jpeg
libchromatix_s5k4e1_preview_la_CFLAGS += -I ../../../../../../../mm-still/jpeg/inc

libchromatix_s5k4e1_preview_la_SOURCES := chromatix_s5k4e1_preview.c

libchromatix_s5k4e1_preview_la_LDFLAGS := $(DEBUG_LDFLAGS)
libchromatix_s5k4e1_preview_la_LDFLAGS += -shared

