LOCAL_PATH:= $(call my-dir)

#
# QtiDdsSwitchService service
#
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional debug
LOCAL_PACKAGE_NAME := QtiDdsSwitchService

LOCAL_JAVA_LIBRARIES := qcrilhook \
        telephony-common

LOCAL_SRC_FILES := $(call all-java-files-under, src)

LOCAL_PROGUARD_ENABLED := disabled

LOCAL_MODULE_OWNER := qti

LOCAL_CERTIFICATE := platform

include $(BUILD_PACKAGE)
