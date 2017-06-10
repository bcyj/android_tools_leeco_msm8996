#     
# Copyright (c) 2011 Qualcomm Technologies, Inc. 
# All Rights Reserved. 
# Qualcomm Technologies Confidential and Proprietary. 
# Developed by QRD Engineering team.
#
ifeq ($(strip $(TARGET_USES_QTIC_STOP_TIMER)),true)
LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional eng

LOCAL_SRC_FILES := $(call all-subdir-java-files)

LOCAL_PACKAGE_NAME := StopTimer

LOCAL_CERTIFICATE := platform

include $(BUILD_PACKAGE)
endif
