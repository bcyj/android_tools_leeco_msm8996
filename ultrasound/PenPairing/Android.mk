LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

# Build all java files in the java subdirectory
LOCAL_SRC_FILES := $(call all-subdir-java-files)

LOCAL_JAVA_LIBRARIES := DigitalPenService

# Name of the APK to build
LOCAL_PACKAGE_NAME := PenPairingApp
LOCAL_CERTIFICATE := platform

# Tell it to build an APK
include $(BUILD_PACKAGE)

