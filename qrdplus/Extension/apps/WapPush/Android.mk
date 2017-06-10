LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(call all-subdir-java-files)

LOCAL_MODULE:= com.qrd.wappush
LOCAL_MODULE_TAGS := optional
LOCAL_JAVA_LIBRARIES += telephony-common  mms-common
include $(BUILD_JAVA_LIBRARY)

#MAKE_XML
include $(CLEAR_VARS)
LOCAL_MODULE := com.qrd.wappush.xml
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)/permissions
LOCAL_SRC_FILES := $(LOCAL_MODULE)
LOCAL_MODULE_TAGS := optional
LOCAL_JAVA_LIBRARIES += telephony-common  mms-common
include $(BUILD_PREBUILT)
