LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_JAVA_LIBRARIES += telephony-common

LOCAL_SRC_FILES := $(call all-subdir-java-files)

LOCAL_PACKAGE_NAME := SalesTracker

LOCAL_MODULE_PATH := $(TARGET_OUT)/vendor/Micromax/system/app

include $(BUILD_PACKAGE)
