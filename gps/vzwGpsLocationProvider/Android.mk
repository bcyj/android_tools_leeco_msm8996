#/******************************************************************************
#*@file Android.mk
#* brief Rules for building the VZW GPS Location Provider and
#* VZW Native GPS Location Provider
#*******************************************************************************/

#Temporarily disbale VzW stack in this release
LOCAL_PATH := $(call my-dir)


# build the jar library, using all java files under java directory
include $(CLEAR_VARS)

LOCAL_MODULE:= com.qualcomm.location.vzw_library

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := $(call all-java-files-under, VzwGpsLocationProvider)

include $(BUILD_JAVA_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE := com.qualcomm.location.vzw_library.xml

LOCAL_MODULE_TAGS := optional

LOCAL_MODULE_CLASS := ETC

LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)/permissions

LOCAL_SRC_FILES := $(LOCAL_MODULE)

include $(BUILD_PREBUILT)

include $(call all-makefiles-under,$(LOCAL_PATH))