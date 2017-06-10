LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := $(call all-java-files-under, src)

LOCAL_CERTIFICATE := platform

LOCAL_PACKAGE_NAME := WfdClient

LOCAL_CERTIFICATE := platform

LOCAL_JAVA_LIBRARIES := WfdCommon

include $(BUILD_PACKAGE)

include $(call all-makefiles-under,$(LOCAL_PATH))
