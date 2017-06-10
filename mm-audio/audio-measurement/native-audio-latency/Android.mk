ifneq ($(BUILD_TINY_ANDROID),true)

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := $(call all-java-files-under, src)

LOCAL_JNI_SHARED_LIBRARIES := libnative_audio_latency_jni

LOCAL_PACKAGE_NAME := NativeAudioLatency

LOCAL_MODULE_OWNER := qcom

LOCAL_PROPRIETARY_MODULE := true

LOCAL_MULTILIB := 32

include $(BUILD_PACKAGE)

include $(call all-makefiles-under, $(LOCAL_PATH))
endif #BUILD_TINY_ANDROID
