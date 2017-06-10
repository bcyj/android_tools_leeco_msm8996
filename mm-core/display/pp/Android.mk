ifneq ($(BUILD_TINY_ANDROID),true)

ROOT_DIR := $(call my-dir)
LOCAL_PATH:= $(ROOT_DIR)
# ---------------------------------------------------------------------------------
#                       Make the PP app (PPPreference)
# ---------------------------------------------------------------------------------
include $(CLEAR_VARS)
LOCAL_PATH:= $(ROOT_DIR)/app/PPPreference

LOCAL_SRC_FILES := $(call all-subdir-java-files)

LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES += \
  /src/com/android/display/IPPService.aidl

LOCAL_PACKAGE_NAME := PPPreference

LOCAL_CERTIFICATE := platform

include $(BUILD_PACKAGE)

endif #TINY_ANDROID
