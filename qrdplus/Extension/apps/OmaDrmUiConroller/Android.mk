LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := user

LOCAL_SRC_FILES := $(call all-java-files-under, src)

LOCAL_PACKAGE_NAME := OmaDrmUiController

LOCAL_CERTIFICATE := platform

LOCAL_PROGUARD_ENABLED := disabled

LOCAL_MODULE_TAGS := optional

include $(BUILD_PACKAGE)
include $(call all-makefiles-under,$(LOCAL_PATH))
