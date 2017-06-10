#/******************************************************************************
#*@file Android.mk
#*brief Rules for copiling the source files
#*******************************************************************************/
LOCAL_PATH:= $(call my-dir)

# Build the auto generated files into a library to be used by both the
# app and the service
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := debug

LOCAL_MODULE := embmslibrary
LOCAL_SRC_FILES += src/com/qualcomm/embms/IEmbmsService.aidl
LOCAL_SRC_FILES += src/com/qualcomm/embms/IEmbmsServiceCallback.aidl

include $(BUILD_JAVA_LIBRARY)
# ==========================================================================
# Build the service
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := debug

LOCAL_SRC_FILES := $(call all-subdir-java-files)

LOCAL_PACKAGE_NAME := embms
LOCAL_CERTIFICATE := platform
LOCAL_PROGUARD_ENABLED := disabled

LOCAL_JAVA_LIBRARIES := embmslibrary qcrilhook

include $(BUILD_PACKAGE)

# ==========================================================================
include $(CLEAR_VARS)

LOCAL_MODULE := embms.xml

LOCAL_MODULE_TAGS := debug

LOCAL_MODULE_CLASS := ETC

# This will install the file in /system/etc/permissions
#
LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)/permissions

LOCAL_SRC_FILES := $(LOCAL_MODULE)

include $(BUILD_PREBUILT)
