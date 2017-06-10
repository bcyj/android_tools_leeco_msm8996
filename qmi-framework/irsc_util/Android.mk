LOCAL_PATH := $(call my-dir)

commonIncludes := $(TARGET_OUT_HEADERS)/common/inc

include $(CLEAR_VARS)
LOCAL_MODULE := irsc_util
LOCAL_C_INCLUDES := $(commonIncludes)
LOCAL_CFLAGS := -DDEBUG_ANDROID
LOCAL_SHARED_LIBRARIES := liblog
LOCAL_SRC_FILES  := irsc_util.c
LOCAL_MODULE_TAGS := optional
include $(BUILD_EXECUTABLE)

