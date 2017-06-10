#
# Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
# Qualcomm Technologies Proprietary and Confidential.
#
ifneq (, $(filter aarch64, $(TARGET_ARCH)))
    $(info TODOAArch64: $(LOCAL_PATH)/Android.mk: Enable build support for 64 bit)
else
ifeq ($(call is-vendor-board-platform,QCOM),true)
LOCAL_PATH:= $(call my-dir)

Enable_SimContacts:=yes

ifeq ($(strip $(Enable_SimContacts)),yes)

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := $(call all-subdir-java-files)

LOCAL_PACKAGE_NAME := SimContacts
LOCAL_CERTIFICATE := platform

include $(BUILD_PACKAGE)

endif
endif
endif
