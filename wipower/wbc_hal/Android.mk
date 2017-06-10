LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := wbc_hal.default

ifdef TARGET_2ND_ARCH
LOCAL_MODULE_RELATIVE_PATH := hw
else
LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)/hw
endif

LOCAL_C_INCLUDES := $(TARGET_OUT_HEADERS)/common/inc
LOCAL_SRC_FILES += file_access.c wbc_uevent.c wbc_hal_interface.c
LOCAL_PROPRIETARY_MODULE := true

LOCAL_SHARED_LIBRARIES := \
        libhardware \
        libcutils \
        libutils

LOCAL_CFLAGS := -Wall
LOCAL_MODULE_TAGS := optional
include $(BUILD_SHARED_LIBRARY)
