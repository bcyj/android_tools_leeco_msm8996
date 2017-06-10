LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := debug
LOCAL_PROGUARD_ENABLED := disabled

LOCAL_SRC_FILES := $(call all-java-files-under, src)


LOCAL_PACKAGE_NAME := PresenceApp
LOCAL_CERTIFICATE := platform
LOCAL_JAVA_LIBRARIES := rcsservice
LOCAL_JAVA_LIBRARIES += rcsimssettings


include $(BUILD_PACKAGE)
