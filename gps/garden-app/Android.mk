#/******************************************************************************
#*@file Android.mk
#*brief Rules for compiling all code related to garden-app
#*******************************************************************************/
ifneq ($(BUILD_TINY_ANDROID),true)
ifneq (true, $(strip $(GPS_NOT_SUPPORTED)))

include $(call all-subdir-makefiles)

endif # GPS_NOT_SUPPORTED
endif # BUILD_TINY_ANDROID
