#
# Copyright (c) 2012 - 2013, Qualcomm Technologies, Inc.
# All Rights Reserved.
# Qualcomm Technologies Proprietary and Confidential.
#

ifeq ($(strip $(TARGET_USES_QTIC_DATA_MONITOR)),true)
LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional eng

LOCAL_SRC_FILES := $(call all-java-files-under, src)

$(warning $(LOCAL_SRC_FILES))
LOCAL_PACKAGE_NAME := DataMonitor
LOCAL_CERTIFICATE := platform

include $(BUILD_PACKAGE)
endif
