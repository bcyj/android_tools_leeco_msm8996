LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := $(call all-subdir-java-files)

LOCAL_PACKAGE_NAME := SetupWizard
LOCAL_CERTIFICATE := platform

LOCAL_MODULE_PATH := $(TARGET_OUT)/vendor/Smartfren/system/app
LOCAL_STATIC_JAVA_LIBRARIES := \
    android-support-v4 \
    android-support-v13 \

include $(BUILD_PACKAGE)
