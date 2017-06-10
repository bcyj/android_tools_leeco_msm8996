OMX_ADEC_QCELP13 := $(call my-dir)
ifeq ($(MM_AUDIO_ENABLED_ADEC_QCELP13_TESTAPP),true)
include $(call all-subdir-makefiles)
endif
