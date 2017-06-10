LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := $(call all-java-files-under,src)

LOCAL_PACKAGE_NAME := 10086cn
# This will install the file in /system/vendor/ChinaMobile
LOCAL_MODULE_PATH := $(TARGET_OUT)/vendor/ChinaMobile/system/app

include $(BUILD_PACKAGE)

# Use the following include to make our test apk.
include $(call all-makefiles-under,$(LOCAL_PATH))

