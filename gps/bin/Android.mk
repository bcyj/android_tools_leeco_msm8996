ifneq ($(BUILD_TINY_ANDROID),true)
ifneq ($(QCA_1530_ANDROID_CONFIG_SHIP),)

LOCAL_PATH := $(call my-dir)

include $(LOCAL_PATH)/common/Android.mk $(LOCAL_PATH)/$(QCA_1530_ANDROID_CONFIG_SHIP)/Android.mk

endif
endif # not BUILD_TINY_ANDROID

