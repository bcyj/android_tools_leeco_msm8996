# Copyright (C) 2013 Mimer Information Technology AB, info@mimer.com
LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := libmimcomm
LOCAL_MODULE_SUFFIX := .so
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := libmimcomm.so

include $(BUILD_PREBUILT)
