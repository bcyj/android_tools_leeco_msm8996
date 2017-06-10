#/******************************************************************************
#*@file Android.mk
#*brief Rules for copiling the source files
#*******************************************************************************/
LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional debug

LOCAL_SRC_FILES := $(call all-subdir-java-files)

#LOCAL_STATIC_JAVA_LIBRARIES := embmslibrary
LOCAL_JAVA_LIBRARIES := embmslibrary
LOCAL_PACKAGE_NAME := EmbmsTestApp
LOCAL_CERTIFICATE := platform

include $(BUILD_PACKAGE)
