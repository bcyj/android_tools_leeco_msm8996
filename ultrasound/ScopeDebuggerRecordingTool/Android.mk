LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := eng

# Build all java files in the java subdirectory
LOCAL_SRC_FILES := $(call all-subdir-java-files)

LOCAL_PROGUARD_ENABLED := disabled

# Name of the APK to build
LOCAL_PACKAGE_NAME := ScopeDebuggerRecordingTool
LOCAL_CERTIFICATE := platform

# Tell it to build an APK
include $(BUILD_PACKAGE)
