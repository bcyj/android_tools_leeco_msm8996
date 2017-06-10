
# mm-qcamera-app
bin_PROGRAMS := mm-qcamera-app

mm_qcamera_app_CFLAGS := $(DEBUG_CFLAGS)
mm_qcamera_app_CFLAGS += -DAMSS_VERSION=$(AMSS_VERSION)
mm_qcamera_app_CFLAGS += -I ../appslib
mm_qcamera_app_CFLAGS += -I ../../common
#for jpeg
#mm_qcamera_app_CFLAGS += -I ../../../mm-still/ipl/inc
#mm_qcamera_app_CFLAGS += -I ../../../mm-still/jpeg/inc

mm_qcamera_app_CFLAGS += -DMSM_CAMERA_GCC
mm_qcamera_app_CFLAGS += -DLINUX_ENABLED
mm_qcamera_app_CFLAGS += -D_V4L2_BASED_CAM_
#remove when snapshot is supported
mm_qcamera_app_CFLAGS += -DDISABLE_JPEG_ENCODING
mm_qcamera_app_CFLAGS += -DCAMERA_ION_HEAP_ID=ION_HEAP_ADSP_ID

if MSM7630
mm_qcamera_app_CFLAGS += -D_MSM7630_
else
if MSM7X27A
mm_qcamera_app_CFLAGS += -D_MSM7627A_
else
mm_qcamera_app_CFLAGS += -D_MSM8960_
endif
endif

mm_qcamera_app_SOURCES := mm_qcamera_app.c
mm_qcamera_app_SOURCES += mm_qcamera_display.c
mm_qcamera_app_SOURCES += mm_qcamera_main_menu.c
mm_qcamera_app_SOURCES += mm_qcamera_preview.c
mm_qcamera_app_SOURCES += mm_qcamera_snapshot.c
#mm_qcamera_app_SOURCES += mm_qcamera_unit_test.c
mm_qcamera_app_SOURCES += mm_qcamera_video.c

mm_qcamera_app_LDFLAGS := $(DEBUG_LDFLAGS)
mm_qcamera_app_LDADD := ../appslib/liboemcamera.la
mm_qcamera_app_LDADD += -ldl

