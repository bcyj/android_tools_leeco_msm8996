LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_JAVA_LIBRARIES := telephony-common

LOCAL_MODULE_TAGS := optional eng

LOCAL_SRC_FILES := $(call all-java-files-under, src)

LOCAL_PACKAGE_NAME := AutoRegistration
LOCAL_CERTIFICATE := platform

# This will install the file in /system/vendor/ChinaTelecom
LOCAL_MODULE_PATH := $(TARGET_OUT)/vendor/ChinaTelecom/system/app
include $(BUILD_PACKAGE)
