# Copyright (C) 2013 Mimer Information Technology AB, info@mimer.com
  LOCAL_PATH := $(call my-dir)
  include $(CLEAR_VARS)
   
  # Module name should match apk name to be installed.
  LOCAL_MODULE := MimerService
  LOCAL_SRC_FILES := $(LOCAL_MODULE).apk
  LOCAL_MODULE_CLASS := APPS
  LOCAL_REQUIRED_MODULES := MimerFramework
  LOCAL_MODULE_SUFFIX := $(COMMON_ANDROID_PACKAGE_SUFFIX)
  LOCAL_CERTIFICATE := platform
   
  include $(BUILD_PREBUILT)
