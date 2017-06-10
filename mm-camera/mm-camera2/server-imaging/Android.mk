ifeq ($(call is-vendor-board-platform,QCOM),true)
#
# mm-qcamera-daemon
#
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_CFLAGS := \
  -DAMSS_VERSION=$(AMSS_VERSION) \
  $(mmcamera_debug_defines) \
  $(mmcamera_debug_cflags) \
  -DMSM_CAMERA_BIONIC

LOCAL_CFLAGS  += -D_ANDROID_

ifneq ($(call is-platform-sdk-version-at-least,17),true)
  LOCAL_CFLAGS += -include bionic/libc/kernel/common/linux/types.h
  LOCAL_CFLAGS += -include bionic/libc/kernel/common/linux/socket.h
  LOCAL_CFLAGS += -include bionic/libc/kernel/common/linux/in.h
  LOCAL_CFLAGS += -include bionic/libc/kernel/common/linux/un.h
endif

LOCAL_SRC_FILES:= server.c \
  server_process.c

LOCAL_C_INCLUDES:=$(LOCAL_PATH)
LOCAL_C_INCLUDES+= $(LOCAL_PATH)/../includes/
LOCAL_C_INCLUDES+= $(LOCAL_PATH)/../media-controller/mct/bus/
LOCAL_C_INCLUDES+= $(LOCAL_PATH)/../media-controller/mct/controller/
LOCAL_C_INCLUDES+= $(LOCAL_PATH)/../media-controller/mct/object/
LOCAL_C_INCLUDES+= $(LOCAL_PATH)/../media-controller/includes/
LOCAL_C_INCLUDES+= $(LOCAL_PATH)/../media-controller/mct/tools/
LOCAL_C_INCLUDES+= $(LOCAL_PATH)/../media-controller/mct/event/
LOCAL_C_INCLUDES+= $(LOCAL_PATH)/../media-controller/mct/pipeline/
LOCAL_C_INCLUDES+= $(LOCAL_PATH)/../media-controller/mct/stream/
LOCAL_C_INCLUDES+= $(LOCAL_PATH)/../media-controller/mct/module/
LOCAL_C_INCLUDES+= $(LOCAL_PATH)/../media-controller/mct/port/
LOCAL_C_INCLUDES+= $(LOCAL_PATH)/../media-controller/mct/debug/
LOCAL_C_INCLUDES+= \
 $(LOCAL_PATH)/../../../../../../hardware/qcom/camera/QCamera2/stack/common

include $(LOCAL_PATH)/../../local_additional_dependency.mk

LOCAL_SHARED_LIBRARIES:= libcutils libdl liboemcamera \
                                   libmmcamera2_stats_modules \
                                   libmmcamera2_iface_modules \
                                   libmmcamera2_isp_modules \
                                   libmmcamera2_sensor_modules \
                                   libmmcamera2_pproc_modules \
                                   libmmcamera2_imglib_modules

LOCAL_MODULE:= mm-qcamera-daemon
LOCAL_32_BIT_ONLY := true
LOCAL_MODULE_TAGS := optional

include $(BUILD_EXECUTABLE)
endif
