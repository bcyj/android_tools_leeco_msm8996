LOCAL_PATH:= $(call my-dir)
VT_TOP:=$(LOCAL_PATH)
include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := $(call all-java-files-under, src)
LOCAL_SRC_FILES += src/com/qualcomm/qti/vtremoteservice/IRemoteService.aidl

LOCAL_PACKAGE_NAME := vtremoteservice
LOCAL_CERTIFICATE := platform

LOCAL_PROPRIETARY_MODULE := true

LOCAL_PROGUARD_FLAGS := -include $(LOCAL_PATH)/proguard.flags

OMS_RESOURCES_LIBRARIES = true
include $(BUILD_PACKAGE)
#include $(BUILD_MULTI_PREBUILT)
