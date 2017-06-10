###############################################################################
# Copyright (c) 2012 Qualcomm Technologies, Inc. All Rights
# Reserved. Qualcomm Technologies Proprietary and Confidential
###############################################################################

LOCAL_PATH := $(call my-dir)

ifdef QCHAT_DLC_ROOT
###########################
# QChat Permissions
###########################
include $(QCHAT_DLC_ROOT)/AccessQcomQMI/Android.mk
endif



