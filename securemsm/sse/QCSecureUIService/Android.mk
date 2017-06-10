ifeq ($(call is-board-platform-in-list, msm8974 apq8084 msm8994 msm8916 msm8916_32),true)
ifneq ($(BUILD_TINY_ANDROID),true)

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_PACKAGE_NAME := com.qualcomm.qti.services.secureui

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := $(call all-java-files-under, src)

LOCAL_PROGUARD_FLAG_FILES := proguard.flags
#LOCAL_PROGUARD_ENABLED := disabled

LOCAL_CERTIFICATE := platform

LOCAL_JAVA_LIBRARIES := WfdCommon telephony-common

include $(BUILD_PACKAGE)

endif
endif