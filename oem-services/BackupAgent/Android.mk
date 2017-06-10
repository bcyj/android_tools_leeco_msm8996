#/******************************************************************************
#*@file Android.mk
#*******************************************************************************/
ifneq ($(TARGET_BUILD_PDK), true)
LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional
LOCAL_PROGUARD_ENABLED := disabled

LOCAL_SRC_FILES := $(call all-subdir-java-files)

LOCAL_JAVA_LIBRARIES := vcard
LOCAL_PACKAGE_NAME := QtiBackupAgent
LOCAL_CERTIFICATE := platform

LOCAL_MODULE_OWNER := qti

include $(BUILD_PACKAGE)

include $(call all-makefiles-under,$(LOCAL_PATH))
endif
