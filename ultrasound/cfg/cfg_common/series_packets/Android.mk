ifneq ($(BUILD_TINY_ANDROID),true)

ROOT_DIR := $(call my-dir)
LOCAL_PATH := $(ROOT_DIR)
include $(CLEAR_VARS)

# Build all of the sub-targets
include $(call all-makefiles-under, $(LOCAL_PATH))

endif #BUILD_TINY_ANDROID

