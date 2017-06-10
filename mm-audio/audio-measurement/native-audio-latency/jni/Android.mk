ifneq ($(BUILD_TINY_ANDROID),true)
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE    := libnative_audio_latency_jni

LOCAL_SRC_FILES := native-audio-latency-jni.c

LOCAL_C_INCLUDES := $(TOP)/system/media/wilhelm/include/
LOCAL_C_INCLUDES += $(TOP)/frameworks/wilhelm/include/
LOCAL_C_INCLUDES += $(TOP)/system/media/opensles/include/

LOCAL_SHARED_LIBRARIES := \
       libutils \
       libOpenSLES \
       libandroid \
       libcutils

LOCAL_MODULE_OWNER := qcom
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MULTILIB := 32

include $(BUILD_SHARED_LIBRARY)
endif #BUILD_TINY_ANDROID
