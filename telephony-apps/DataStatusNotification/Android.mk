LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional eng

LOCAL_PROGUARD_ENABLED := disabled

LOCAL_SRC_FILES := $(call all-subdir-java-files)

LOCAL_PACKAGE_NAME := datastatusnotification
LOCAL_CERTIFICATE := platform
LOCAL_JAVA_LIBRARIES := qcrilhook telephony-common

include $(BUILD_PACKAGE)

include $(call all-makefiles-under,$(LOCAL_PATH))
