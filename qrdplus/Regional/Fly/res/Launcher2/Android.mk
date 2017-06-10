LOCAL_PATH := $(call my-dir)
res_dir := res
include $(CLEAR_VARS)

LOCAL_RESOURCE_DIR := $(addprefix $(LOCAL_PATH)/, $(res_dir))

LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := $(call all-subdir-java-files)
LOCAL_SDK_VERSION := current
LOCAL_CERTIFICATE := shared

LOCAL_PACKAGE_NAME := FlyLauncher2Res
LOCAL_MODULE_PATH := $(TARGET_OUT)/vendor/Fly/system/vendor/overlay

include $(BUILD_PACKAGE)
