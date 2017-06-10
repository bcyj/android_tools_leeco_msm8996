LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := wbc_hal_test
LOCAL_C_INCLUDES := $(TARGET_OUT_HEADERS)/common/inc
LOCAL_SRC_FILES += wbc_hal_test.c

LOCAL_SHARED_LIBRARIES := \
        libhardware \
        libwbc_hal \
        libc \
        libcutils \
        libutils

LOCAL_MODULE_TAGS := optional
include $(BUILD_EXECUTABLE)

