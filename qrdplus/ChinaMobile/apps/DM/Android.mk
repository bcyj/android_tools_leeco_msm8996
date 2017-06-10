LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_JAVA_LIBRARIES := telephony-common

LOCAL_MODULE_TAGS := optional

# Only compile source java files in this apk.
LOCAL_SRC_FILES := $(call all-java-files-under, src)

LOCAL_PACKAGE_NAME := DM

#LOCAL_SDK_VERSION := current

LOCAL_CERTIFICATE := platform

LOCAL_JNI_SHARED_LIBRARIES := libdmjni
LOCAL_MULTILIB := 32

LOCAL_PROGUARD_ENABLED := disabled

# This will install the file in /system/vendor/ChinaMobile
LOCAL_MODULE_RELATIVE_PATH := ../ChinaMobile/system/app
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := qti

include $(BUILD_PACKAGE)

# Use the following include to make our test apk.
include $(call all-makefiles-under,$(LOCAL_PATH))
