#/******************************************************************************
#*@file Android.mk
#*brief Rules for copiling the source files
#*******************************************************************************/

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional debug
LOCAL_PROGUARD_ENABLED := disabled

LOCAL_SRC_FILES := $(call all-subdir-java-files)
LOCAL_SRC_FILES += src/com/qualcomm/qualcommsettings/IDun.aidl

LOCAL_PACKAGE_NAME := QualcommSettings
LOCAL_CERTIFICATE := platform
LOCAL_JAVA_LIBRARIES := qcnvitems qcrilhook

LOCAL_MODULE_OWNER := qcom

include $(BUILD_PACKAGE)
