OMX_ADEC_EVRC := $(call my-dir)
ifeq ($(MM_AUDIO_ENABLED_ADEC_EVRC_TESTAPP),true)
include $(call all-subdir-makefiles)
endif
