ifeq ($(strip $(AUDIO_FEATURE_ENABLED_DS2_DOLBY_DAP)),true)
DOLBY_ROOT := $(call my-dir)
include $(call all-subdir-makefiles)
endif
