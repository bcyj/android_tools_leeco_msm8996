LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := $(call all-java-files-under, src) \
        src/com/qualcomm/qti/accesscache/ICarrierAccessCacheService.aidl

LOCAL_PACKAGE_NAME := CarrierCacheService

LOCAL_CERTIFICATE := platform

include $(BUILD_PACKAGE)
