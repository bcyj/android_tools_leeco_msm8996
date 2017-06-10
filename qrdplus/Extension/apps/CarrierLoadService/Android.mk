LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := $(call all-java-files-under, src) \
        src/com/qualcomm/qti/loadcarrier/ILoadCarrierService.aidl

LOCAL_JAVA_LIBRARIES := telephony-common

LOCAL_PACKAGE_NAME := CarrierLoadService

LOCAL_CERTIFICATE := platform

include $(BUILD_PACKAGE)
