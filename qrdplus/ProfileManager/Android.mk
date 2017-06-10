#     
# Copyright (c) 2011-2012, Qualcomm Technologies, Inc. 
# All Rights Reserved. 
# Qualcomm Technologies Proprietary and Confidential.
# Developed by QRD Engineering team.
#
ifeq ($(strip $(TARGET_USES_QTIC_PROFILE_MANAGER)),true)
LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional eng

LOCAL_JAVA_LIBRARIES := telephony-common

LOCAL_SRC_FILES := $(call all-subdir-java-files)

LOCAL_PACKAGE_NAME := ProfileMgr

LOCAL_CERTIFICATE := platform

include $(BUILD_PACKAGE)
endif
