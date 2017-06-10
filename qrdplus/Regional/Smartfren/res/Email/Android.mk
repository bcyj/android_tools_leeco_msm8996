LOCAL_PATH := $(call my-dir)

res_dir := res
include $(CLEAR_VARS)

LOCAL_RESOURCE_DIR := $(addprefix $(LOCAL_PATH)/, $(res_dir))
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := $(call all-subdir-java-files)
LOCAL_SDK_VERSION := current
LOCAL_PACKAGE_NAME := SmartfrenEmailRes

LOCAL_MODULE_PATH := $(TARGET_OUT)/vendor/Smartfren/system/vendor/overlay
LOCAL_CERTIFICATE := shared

include $(BUILD_PACKAGE)

