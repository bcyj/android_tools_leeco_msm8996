LOCAL_PATH:= $(call my-dir)
#VT_TOP:=$(LOCAL_PATH)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := $(call all-java-files-under, src)
LOCAL_SRC_FILES += src/com/qualcomm/qti/vtremoteservice/IRemoteService.aidl
LOCAL_MODULE := videocallapi

#LOCAL_PACKAGE_NAME := VideoCallApi
#LOCAL_PROGUARD_FLAGS := -include $(LOCAL_PATH)/proguard.flags

#OMS_RESOURCES_LIBRARIES = true

LOCAL_JAVA_LIBRARIES := telephony-common
#include $(BUILD_STATIC_JAVA_LIBRARY)
include $(BUILD_JAVA_LIBRARY)

#MAKE_XML
include $(CLEAR_VARS)
LOCAL_MODULE := videocallapi.xml
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)/permissions
LOCAL_SRC_FILES := $(LOCAL_MODULE)
include $(BUILD_PREBUILT)
