LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := $(call all-subdir-java-files)

LOCAL_PACKAGE_NAME := LanixClaroColombiaSTKProxy
LOCAL_AAPT_FLAGS := --rename-manifest-package com.qualcomm.qti.stkproxy

# This will install the file in /system/vendor/LanixClaroColombia
LOCAL_MODULE_PATH := $(TARGET_OUT)/vendor/LanixClaroColombia/system/app

LOCAL_CERTIFICATE := platform
include $(BUILD_PACKAGE)
