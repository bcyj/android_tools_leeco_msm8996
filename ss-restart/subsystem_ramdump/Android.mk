LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := subsystem_ramdump.c

LOCAL_CFLAGS := -DANDROID_BUILD

LOCAL_SHARED_LIBRARIES += libmdmdetect libcutils libutils

LOCAL_MODULE := subsystem_ramdump

LOCAL_MODULE_TAGS := optional

ifeq ($(call is-vendor-board-platform,QCOM),true)
  LOCAL_C_INCLUDES := $(TARGET_OUT_HEADERS)/common/inc
endif

LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/libmdmdetect/inc/

LOCAL_MODULE_OWNER := qcom

include $(BUILD_EXECUTABLE)
