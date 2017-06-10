LOCAL_PATH := $(call my-dir)

# Build all of the sub-targets
include $(CLEAR_VARS)

include $(call all-makefiles-under, $(LOCAL_PATH))