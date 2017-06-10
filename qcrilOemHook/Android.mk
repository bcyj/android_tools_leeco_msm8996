################################################################################
# @file  Android.mk
# @brief Rules for compiling the source files
################################################################################

LOCAL_PATH:= $(call my-dir)
# ==========================================================================
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional debug
LOCAL_JAVA_LIBRARIES := telephony-common

LOCAL_MODULE := qcrilhook
LOCAL_SRC_FILES := $(call all-java-files-under,src/com/qualcomm/qcrilhook)
LOCAL_SRC_FILES += src/com/qualcomm/qcrilhook/IOemHookCallback.aidl
LOCAL_SRC_FILES += src/com/qualcomm/qcrilmsgtunnel/IQcrilMsgTunnel.aidl

include $(BUILD_JAVA_LIBRARY)

# ==========================================================================
include $(CLEAR_VARS)

LOCAL_MODULE := qcrilhook.xml

LOCAL_MODULE_TAGS := optional debug

LOCAL_MODULE_CLASS := ETC

# This will install the file in /system/etc/permissions
#
LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)/permissions

LOCAL_SRC_FILES := $(LOCAL_MODULE)

include $(BUILD_PREBUILT)

# ==========================================================================
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional debug
LOCAL_JAVA_LIBRARIES := telephony-common

LOCAL_SRC_FILES := $(call all-java-files-under,src/com/qualcomm/qcrilmsgtunnel)
LOCAL_SRC_FILES += src/com/qualcomm/qcrilhook/IOemHookCallback.aidl
LOCAL_SRC_FILES += src/com/qualcomm/qcrilmsgtunnel/IQcrilMsgTunnel.aidl

LOCAL_PACKAGE_NAME := qcrilmsgtunnel
LOCAL_CERTIFICATE := platform

LOCAL_PROGUARD_ENABLED := disabled

include $(BUILD_PACKAGE)
