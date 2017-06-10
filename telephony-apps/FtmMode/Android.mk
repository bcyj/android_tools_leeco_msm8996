#
# Copyright (c) 2013, Qualcomm Technologies, Inc.
# All Rights Reserved.
# Qualcomm Technologies Proprietary and Confidential.
#

LOCAL_PATH:= $(call my-dir)


include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := eng

LOCAL_SRC_FILES := $(call all-subdir-java-files)

LOCAL_PACKAGE_NAME := FTMMode
LOCAL_CERTIFICATE := platform
LOCAL_JAVA_LIBRARIES += qcrilhook

include $(BUILD_PACKAGE)

include $(call all-makefiles-under,$(LOCAL_PATH))
