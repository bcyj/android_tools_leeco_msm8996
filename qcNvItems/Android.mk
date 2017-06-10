################################################################################
# @file  Android.mk
# @brief Rules for compiling the source files
################################################################################

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional debug

LOCAL_MODULE := qcnvitems
LOCAL_SRC_FILES := $(call all-subdir-java-files)

LOCAL_JAVA_LIBRARIES := qcrilhook

include $(BUILD_JAVA_LIBRARY)

# ==========================================================================
include $(CLEAR_VARS)

LOCAL_MODULE := qcnvitems.xml

LOCAL_MODULE_TAGS := optional debug

LOCAL_MODULE_CLASS := ETC

# This will install the file in /system/etc/permissions
#
LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)/permissions

LOCAL_SRC_FILES := $(LOCAL_MODULE)

include $(BUILD_PREBUILT)
