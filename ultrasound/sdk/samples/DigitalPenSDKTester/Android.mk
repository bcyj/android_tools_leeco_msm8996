#/******************************************************************************
#*@file Android.mk
# Rule for making Digital Pen SDK Tester
#*******************************************************************************/

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_PROGUARD_ENABLED := disabled

LOCAL_STATIC_JAVA_LIBRARIES := DigitalPenSDK

LOCAL_SRC_FILES := $(call all-java-files-under, src)

LOCAL_PACKAGE_NAME := DigitalPenSDKTester
LOCAL_SDK_VERSION := current
LOCAL_CERTIFICATE := platform

include $(BUILD_PACKAGE)
