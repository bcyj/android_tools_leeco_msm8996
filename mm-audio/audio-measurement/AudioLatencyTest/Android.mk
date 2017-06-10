
#/******************************************************************************
#*@file Android.mk
#*brief Rules for copiling the source files
#*******************************************************************************/

ifeq ($(call is-board-platform-in-list,msm7630_surf msm8660 msm8960),true)
ifneq ($(BUILD_TINY_ANDROID),true)

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := $(call all-subdir-java-files)

LOCAL_PACKAGE_NAME := AudioLatencyTest
LOCAL_CERTIFICATE := platform
include $(BUILD_PACKAGE)

endif #BUILD_TINY_ANDROID
endif #is-board-platform-in-list
