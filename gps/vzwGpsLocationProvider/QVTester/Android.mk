#/******************************************************************************
#*@file Android.mk
#*brief Rules for compiling QVTester04 Application
#*******************************************************************************/

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

#this tag means this package will be available for all eng builds
#note if the OEM decides to include this permission and group assignment in the final build, the
#tag should be changed to 'user'
LOCAL_MODULE_TAGS := optional

LOCAL_JAVA_LIBRARIES := com.qualcomm.location.vzw_library

LOCAL_SRC_FILES := $(call all-subdir-java-files)

LOCAL_PACKAGE_NAME := QVTester

#note this is necessary for the test app to acquire the permission to access LocAPI in etc/oncrpc
LOCAL_CERTIFICATE := platform

include $(BUILD_PACKAGE)
