LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

# Only compile source java files in this apk.
LOCAL_SRC_FILES := $(call all-java-files-under, src)

LOCAL_PACKAGE_NAME := NotePad2

LOCAL_SDK_VERSION := current
# This will install the file in /system/vendor/ChinaMobile
LOCAL_MODULE_PATH := $(TARGET_OUT)/vendor/ChinaMobile/system/app

include $(BUILD_PACKAGE)

# Use the following include to make our test apk.
include $(call all-makefiles-under,$(LOCAL_PATH))
