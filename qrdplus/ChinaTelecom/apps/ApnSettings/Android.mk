LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := $(call all-subdir-java-files)

LOCAL_JAVA_LIBRARIES := telephony-common telephony-msim
LOCAL_PACKAGE_NAME := ApnSettings

LOCAL_CERTIFICATE := platform

# This will install the file in /system/vendor/ChinaTelecom
LOCAL_MODULE_PATH := $(TARGET_OUT)/vendor/ChinaTelecom/system/app
include $(BUILD_PACKAGE)