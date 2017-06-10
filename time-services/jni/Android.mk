LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
        TimeServiceNative.cpp

LOCAL_C_INCLUDES += \
        $(JNI_H_INCLUDE) \
        $(TARGET_OUT_HEADERS)/common/inc \
        $(TARGET_OUT_HEADERS)/time-services

LOCAL_SHARED_LIBRARIES += libtime_genoff libutils liblog

LOCAL_PRELINK_MODULE := false
LOCAL_MODULE:= libTimeService
LOCAL_MODULE_TAGS := optional

LOCAL_MODULE_OWNER := qcom
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)
