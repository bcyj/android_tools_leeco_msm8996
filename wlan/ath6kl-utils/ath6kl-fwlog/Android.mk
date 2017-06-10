LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := ath6kl-fwlog-record
LOCAL_MODULE_TAGS := optional
LOCAL_C_INCLUDES := $(LOCAL_PATH)
LOCAL_SHARED_LIBRARIES := libc libcutils
LOCAL_SRC_FILES := ath6kl-fwlog-record.c
LOCAL_CFLAGS := $(CFLAGS)
LOCAL_MODULE_OWNER := qcom
include $(BUILD_EXECUTABLE)
