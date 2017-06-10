
# v4l2-qcamera-app
bin_PROGRAMS := v4l2-qcamera-app

v4l2_qcamera_app_CFLAGS := $(DEBUG_CFLAGS)
v4l2_qcamera_app_CFLAGS += -DAMSS_VERSION=$(AMSS_VERSION)
v4l2_qcamera_app_CFLAGS += -I ../appslib
v4l2_qcamera_app_CFLAGS += -I ../../common
#for jpeg
#v4l2_qcamera_app_CFLAGS += -I ../../../mm-still/ipl/inc
#v4l2_qcamera_app_CFLAGS += -I ../../../mm-still/jpeg/inc

v4l2_qcamera_app_CFLAGS += -DMSM_CAMERA_GCC
v4l2_qcamera_app_CFLAGS += -DLINUX_ENABLED
#remove when snapshot is supported
v4l2_qcamera_app_CFLAGS += -DDISABLE_JPEG_ENCODING
v4l2_qcamera_app_CFLAGS += -D_V4L2_BASED_CAM_

v4l2_qcamera_app_SOURCES := v4l2-qcamera-app.c

v4l2_qcamera_app_LDFLAGS := $(DEBUG_LDFLAGS)
v4l2_qcamera_app_LDADD := ../appslib/liboemcamera.la
v4l2_qcamera_app_LDADD += -ldl

