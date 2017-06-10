LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_JAVA_LIBRARIES := telephony-common \
                        qcrilhook


LOCAL_SRC_FILES := $(call all-java-files-under, src)

LOCAL_PACKAGE_NAME := LanixTelcelMexicoTelephonyApp

LOCAL_MODULE_PATH := $(TARGET_OUT)/vendor/LanixTelcelMexico/system/app

LOCAL_CERTIFICATE := platform

LOCAL_PRIVILEGED_MODULE := true

#LOCAL_PROGUARD_FLAG_FILES := proguard.flags

include $(BUILD_PACKAGE)

