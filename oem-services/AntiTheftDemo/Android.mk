#/******************************************************************************
#*@file Android.mk
#*brief Rules for copiling the source files
#*******************************************************************************/

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional
LOCAL_PROGUARD_ENABLED := disabled

LOCAL_SRC_FILES := $(call all-subdir-java-files)

LOCAL_PACKAGE_NAME := AntiTheftDemo
LOCAL_CERTIFICATE := platform

LOCAL_JAVA_LIBRARIES += telephony-common
LOCAL_MODULE_OWNER := qti

include $(BUILD_PACKAGE)

include $(call all-makefiles-under,$(LOCAL_PATH))

