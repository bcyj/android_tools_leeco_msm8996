ifeq ($(strip $(FEATURE_Independent)),yes)

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_PROGUARD_ENABLED := disabled

LOCAL_SRC_FILES := $(call all-java-files-under, src)

LOCAL_PACKAGE_NAME := GsmTuneAway

LOCAL_CERTIFICATE := platform

LOCAL_JAVA_LIBRARIES := telephony-common telephony-msim

# This will install the file in /system/vendor/ChinaUnicom
LOCAL_MODULE_PATH := $(TARGET_OUT)/vendor/ChinaUnicom/system/app
include $(BUILD_PACKAGE)

endif
