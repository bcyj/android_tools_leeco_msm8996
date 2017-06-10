ifeq ($(strip $(TARGET_USES_QTIC_WORLD_CLOCK)),true)
LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional eng

LOCAL_SRC_FILES := $(call all-subdir-java-files)

LOCAL_PACKAGE_NAME := WorldClock

LOCAL_CERTIFICATE := platform

include $(BUILD_PACKAGE)
endif
