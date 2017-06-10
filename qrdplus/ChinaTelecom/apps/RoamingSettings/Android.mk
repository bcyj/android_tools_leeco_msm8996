LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional eng

# Only compile source java files in this apk.
LOCAL_SRC_FILES := $(call all-java-files-under, src)

LOCAL_PACKAGE_NAME := CtRoamingSettings

LOCAL_CERTIFICATE := platform

LOCAL_JAVA_LIBRARIES += telephony-common qcrilhook

LOCAL_MODULE_OWNER := qcom

# This will install the file in /system/vendor/ChinaTelecom
LOCAL_MODULE_PATH := $(TARGET_OUT)/vendor/ChinaTelecom/system/app

include $(BUILD_PACKAGE)

# Use the following include to make our test apk.
include $(call all-makefiles-under,$(LOCAL_PATH))
