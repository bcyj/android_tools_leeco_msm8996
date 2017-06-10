src_java := src/com/qualcomm/qti/imstestrunner
src_proto := src

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional eng

LOCAL_PACKAGE_NAME := imstestrunner

LOCAL_PROTOC_OPTIMIZE_TYPE := micro

LOCAL_CERTIFICATE := platform

LOCAL_MODULE_OWNER := qcom

LOCAL_SRC_FILES := $(call all-subdir-java-files) \
    $(call all-proto-files-under, $(src_proto))

LOCAL_PROGUARD_ENABLED := disabled

#include $(BUILD_STATIC_JAVA_LIBRARY)
include $(BUILD_PACKAGE)


# ==========================================================================
include $(CLEAR_VARS)

LOCAL_MODULE := imstestrunner.xml

54LOCAL_MODULE_TAGS := optional

LOCAL_MODULE_CLASS := ETC
LOCAL_PROGUARD_ENABLED := disabled

# This will install the file in /system/etc/permissions

LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)/permissions

LOCAL_SRC_FILES := $(LOCAL_MODULE)

include $(BUILD_PREBUILT)

