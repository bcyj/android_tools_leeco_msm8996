#/******************************************************************************
#*@file Android.mk
#*brief Rules for copiling the source files
#*******************************************************************************/

LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := $(call all-subdir-java-files)

LOCAL_PACKAGE_NAME := CustomerService

LOCAL_JAVA_LIBRARIES += telephony-common mms-common

# This will install the file in /system/vendor/ChinaTelecom
LOCAL_MODULE_PATH := $(TARGET_OUT)/vendor/ChinaTelecom/system/app

include $(BUILD_PACKAGE)

include $(call all-makefiles-under,$(LOCAL_PATH))
