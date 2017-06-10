LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
# Logging Features. Turn any one ON at any time

LOCAL_C_INCLUDES := $(LOCAL_PATH)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../core/lib/inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../inc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../platform
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/common/inc

LOCAL_SRC_FILES += qmi_idl_lib.c
LOCAL_SRC_FILES += qmi_idl_accessor.c

LOCAL_MODULE := libidl

LOCAL_MODULE_TAGS := optional

LOCAL_PRELINK_MODULE := false

LOCAL_MODULE_OWNER := qcom
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)
