###############################################################################
# Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights
# Reserved. Qualcomm Technologies Proprietary and Confidential
###############################################################################

LOCAL_PATH := $(call my-dir)

ifdef INTERFACE_PERMISSIONS
###########################
# Permission Mapping
###########################
include $(INTERFACE_PERMISSIONS)/PermissionMapping/Android.mk
endif