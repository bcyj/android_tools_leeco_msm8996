LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

# Only compile source java files in this apk.
LOCAL_SRC_FILES := $(call all-java-files-under, src) \
       src/com/qualcomm/qti/loadcarrier/ILoadCarrierService.aidl \
       src/com/qualcomm/qti/accesscache/ICarrierAccessCacheService.aidl

LOCAL_PACKAGE_NAME := CarrierConfigure

LOCAL_JAVA_LIBRARIES := qcrilhook

LOCAL_CERTIFICATE := platform

LOCAL_PROGUARD_ENABLED := disabled

include $(BUILD_PACKAGE)
