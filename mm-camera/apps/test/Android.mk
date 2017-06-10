LOCAL_PATH:=$(call my-dir)
include $(CLEAR_VARS)

mmcamera_debug_defines += -DENABLE_DISPLAY

LOCAL_CFLAGS:= -Werror \
	-DAMSS_VERSION=$(AMSS_VERSION) \
	$(mmcamera_debug_defines) \
	$(mmcamera_debug_cflags) \
  -include camera_defs_i.h

ifeq ($(MSM_VERSION),7x2x)
  LOCAL_CFLAGS += -D_TARGET_7X27_
endif

ifeq ($(VFE_VERS),vfe31)
  LOCAL_CFLAGS += -D_VFE_31_
endif

ifeq ($(MSM_VERSION), 7x27A)
  LOCAL_CFLAGS += -D_TARGET_7X27A_
endif
LOCAL_CFLAGS += -include bionic/libc/kernel/common/linux/types.h
LOCAL_CFLAGS += -include bionic/libc/kernel/common/linux/socket.h
LOCAL_CFLAGS += -include bionic/libc/kernel/common/linux/in.h
LOCAL_CFLAGS += -include bionic/libc/kernel/common/linux/un.h

LOCAL_SRC_FILES:= \
	main.c \
	native_cam.c \
	video_cam.c \
	camera_testsuite.c \
	testsuite_server.c \
	testsuite_utilities.c

LOCAL_C_INCLUDES:=$(LOCAL_PATH)
LOCAL_C_INCLUDES+= \
	$(LOCAL_PATH)/../appslib \
	$(LOCAL_PATH)/../../common \
	$(LOCAL_PATH)/../../../mm-still/jpeg \
	$(LOCAL_PATH)/../../../mm-still/jpeg/inc \
	$(LOCAL_PATH)/../../../mm-still/jpeg/src
LOCAL_C_INCLUDES+= $(LOCAL_PATH)/../../../../../../hardware/qcom/camera

include $(LOCAL_PATH)/../../local_additional_dependency.mk

ifeq ($(MSM_VERSION), 8960)
LOCAL_CFLAGS += -DCAMERA_ION_HEAP_ID=ION_CP_MM_HEAP_ID
else
LOCAL_CFLAGS += -DCAMERA_ION_HEAP_ID=ION_CAMERA_HEAP_ID
endif

ifeq ($(strip $(TARGET_USES_ION)),true)
LOCAL_CFLAGS += -DUSE_ION
endif

LOCAL_SHARED_LIBRARIES:= \
	liboemcamera libcutils $(mmcamera_debug_libs) libdl

LOCAL_MODULE:=mm-qcamera-test

LOCAL_MODULE_TAGS := optional

include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES:=$(LOCAL_PATH)
LOCAL_C_INCLUDES+= \
	$(LOCAL_PATH)/../appslib
LOCAL_C_INCLUDES+= $(LOCAL_PATH)/../../../../../../hardware/qcom/camera

LOCAL_SRC_FILES:= \
        testsuite_client.c \
	testsuite_utilities.c

LOCAL_MODULE:=mm-qcamera-testsuite-client

LOCAL_MODULE_TAGS := optional

include $(BUILD_EXECUTABLE)
