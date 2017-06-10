
# libmmcamera-core.a
noinst_LTLIBRARIES := libmmcamera-core.la

libmmcamera_core_la_CFLAGS := $(DEBUG_CFLAGS)
libmmcamera_core_la_CFLAGS += -DMSM_CAMERA_GCC
libmmcamera_core_la_CFLAGS += -D_GNU_SOURCE
libmmcamera_core_la_CFLAGS += -DLINUX_ENABLED
libmmcamera_core_la_CFLAGS += -fPIC
libmmcamera_core_la_CFLAGS += -D__ARM_HAVE_NEON
libmmcamera_core_la_CFLAGS += -DDISABLE_JPEG_ENCODING
libmmcamera_core_la_CFLAGS += -D_V4L2_BASED_CAM_

libmmcamera_core_la_CFLAGS += -I ../../common
#for jpeg
if MSM7X2X
libmmcamera_core_la_CFLAGS += -I ../../../mm-still/jpeg
libmmcamera_core_la_CFLAGS += -I ../../../mm-still/jpeg/inc
endif
if MSM7X27A
libmmcamera_core_la_CFLAGS += -I ../../../mm-still/jpeg
libmmcamera_core_la_CFLAGS += -I ../../../mm-still/jpeg/inc
else
libmmcamera_core_la_CFLAGS += -I ../../../mm-still/jpeg2
libmmcamera_core_la_CFLAGS += -I ../../../mm-still/jpeg2/inc
endif
if MSM8660
libmmcamera_core_la_CFLAGS += -I ../../../mm-still/mpo/src
libmmcamera_core_la_CFLAGS += -I ../../../mm-still/mpo/inc
endif

libmmcamera_core_la_SOURCES := cam_frame_q.c
libmmcamera_core_la_SOURCES += camframe.c
libmmcamera_core_la_SOURCES += camaf_ctrl.c
libmmcamera_core_la_SOURCES += mm_camera_interface.c
libmmcamera_core_la_SOURCES += cam_display.c
libmmcamera_core_la_SOURCES += cam_stats.c
#enable when snapshot is supported
libmmcamera_core_la_SOURCES += snapshot.c
#libmmcamera_core_la_SOURCES += jpeg_encoder.c
libmmcamera_core_la_SOURCES += liveshot.c
libmmcamera_core_la_SOURCES += yv12conversionroutines.c
if MSM8660
libmmcamera_core_la_SOURCES += mpo_encoder.c
endif

libmmcamera_core_la_LDFLAGS := $(DEBUG_LDFLAGS)
libmmcamera_core_la_LDFLAGS += -static
if MSM7X2X
libmmcamera_core_la_LDFLAGS += -D_TARGET_7x2x_
endif
if MSM7X27A
libmmcamera_core_la_LDFLAGS += -D_TARGET_7x27A_
endif
if MSM8660
libmmcamera_core_la_LDFLAGS += -D_TARGET_8660_
endif

include ../../override_mem_alloc_dealloc.am
libmmcamera_core_la_DEPENDENCIES := $(call override_mem_alloc_dealloc,$(libmmcamera_core_la_OBJECTS))


# liboemcamera.so
lib_LTLIBRARIES := liboemcamera.la

liboemcamera_la_CFLAGS := $(DEBUG_CFLAGS)
liboemcamera_la_CFLAGS += -DMSM_CAMERA_GCC
liboemcamera_la_CFLAGS += -DLINUX_ENABLED

liboemcamera_la_SOURCES := override_malloc.c

liboemcamera_la_LDFLAGS := $(DEBUG_LDFLAGS)
liboemcamera_la_LDFLAGS += -shared
liboemcamera_la_LDFLAGS += -version-info 1:0:0
liboemcamera_la_LIBADD = libmmcamera-core.la
if WAVELET_DENOISE
if MSM8960
liboemcamera_la_LIBADD += ../../server/libmmcamera_target.la
else
if MSM7630
liboemcamera_la_LIBADD += ../../server/libmmcamera_target.la
else
if MSM7X27A
liboemcamera_la_LIBADD += ../../server/libmmcamera_target.la
else
liboemcamera_la_LIBADD += ../../server/libmmcamera_target.la
liboemcamera_la_LIBADD += ../../server/libmmcamera_server_assembly.la
endif
endif
endif
endif
liboemcamera_la_LIBADD += -ldl
if MSM8960
liboemcamera_la_LIBADD += -lmmjpeg
else
#add when snapshot is supported
#liboemcamera_la_LIBADD += -lmmjpeg
#liboemcamera_la_LIBADD += -lmmipl
endif
if MSM8660
liboemcamera_la_LIBADD += -lmmmpo
liboemcamera_la_LIBADD += -lC2D2
liboemcamera_la_LIBADD += -lmmstereo
endif

# mm-qcamera-daemon
bin_PROGRAMS := mm-qcamera-daemon

mm_qcamera_daemon_CFLAGS := $(DEBUG_CFLAGS)
mm_qcamera_daemon_CFLAGS += -DMSM_CAMERA_GCC
mm_qcamera_daemon_CFLAGS += -DLINUX_ENABLED
mm_qcamera_daemon_CFLAGS += -DDISABLE_JPEG_ENCODING

mm_qcamera_daemon_CFLAGS += -I ../appslib
mm_qcamera_daemon_CFLAGS += -I ../../common

#for jpeg
if MSM7X2X
mm_qcamera_daemon_CFLAGS += -I ../../../mm-still/jpeg
mm_qcamera_daemon_CFLAGS += -I ../../../mm-still/jpeg/inc
else
mm_qcamera_daemon_CFLAGS += -I ../../../mm-still/jpeg2
mm_qcamera_daemon_CFLAGS += -I ../../../mm-still/jpeg2/inc
endif
if MSM8660
mm_qcamera_daemon_CFLAGS += -I ../../../mm-still/mpo/src
mm_qcamera_daemon_CFLAGS += -I ../../../mm-still/mpo/inc
endif

mm_qcamera_daemon_SOURCES := override_malloc.c
mm_qcamera_daemon_SOURCES += ../../common/stacktrace.c
mm_qcamera_daemon_SOURCES += camdaemon.c

mm_qcamera_daemon_LDFLAGS := $(DEBUG_LDFLAGS)
mm_qcamera_daemon_LDADD = liboemcamera.la
#add when snapshot is supported
#mm_qcamera_daemon_LDADD += -lmmipl
mm_qcamera_daemon_LDADD += -lglib-2.0

