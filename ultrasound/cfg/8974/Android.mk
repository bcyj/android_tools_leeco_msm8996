LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

# Build all of the sub-targets
ifeq ($(call is-board-platform,msm8974),true)
  include $(call all-makefiles-under, $(LOCAL_PATH))
endif

