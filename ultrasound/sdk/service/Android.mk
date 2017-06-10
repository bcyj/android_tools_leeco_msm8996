LOCAL_PATH:= $(call my-dir)

#=============================================
#  DigitalPenService lib for service
#=============================================
include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(call all-java-files-under, src)

LOCAL_STATIC_JAVA_LIBRARIES := simple DigitalPenSDK
LOCAL_MODULE_TAGS := optional

LOCAL_MODULE := DigitalPenService

include $(BUILD_JAVA_LIBRARY)

# ====  digitalpenservice.xml lib def  ========================
include $(CLEAR_VARS)

LOCAL_MODULE := digitalpenservice.xml
LOCAL_MODULE_TAGS := optional
LOCAL_PROGUARD_ENABLED := disabled
LOCAL_MODULE_CLASS := ETC

# This will install the file in /system/etc/permissions
LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)/permissions

LOCAL_SRC_FILES := $(LOCAL_MODULE)

LOCAL_MODULE_OWNER := qcom

include $(BUILD_PREBUILT)
