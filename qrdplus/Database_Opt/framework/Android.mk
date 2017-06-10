# Copyright (C) 2013 Mimer Information Technology AB, info@mimer.com
LOCAL_PATH := $(call my-dir)

#MAKE runtime library

include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := MimerFramework.jar
LOCAL_MODULE_CLASS := JAVA_LIBRARIES
LOCAL_SRC_FILES := $(LOCAL_MODULE)
include $(BUILD_PREBUILT)

#MAKE host library
# the custom dex'ed emma library ready to put on a device.
# ============================================================
include $(CLEAR_VARS)

LOCAL_PREBUILT_STATIC_JAVA_LIBRARIES := MimerFramework:MimerFramework_host.jar
include $(BUILD_MULTI_PREBUILT)

#MAKE_XML
include $(CLEAR_VARS)
LOCAL_MODULE := MimerFramework.xml
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)/permissions
LOCAL_SRC_FILES := $(LOCAL_MODULE)
include $(BUILD_PREBUILT)


