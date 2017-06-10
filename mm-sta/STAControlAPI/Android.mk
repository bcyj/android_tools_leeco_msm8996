LOCAL_PATH:= $(call my-dir)

#
# STA Control API client
#

include $(CLEAR_VARS)

LOCAL_CFLAGS:= -D_ANDROID_

LOCAL_SRC_FILES:= src/control_api.cpp

LOCAL_C_INCLUDES:= $(LOCAL_PATH)/inc

LOCAL_SHARED_LIBRARIES := liblog
LOCAL_MODULE := libstaapi

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_OWNER := qcom

LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)
