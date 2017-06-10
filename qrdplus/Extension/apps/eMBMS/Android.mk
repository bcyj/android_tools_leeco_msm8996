LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := MSDC_UI
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := $(LOCAL_MODULE).apk
LOCAL_MODULE_CLASS := APPS
LOCAL_MODULE_SUFFIX := $(COMMON_ANDROID_PACKAGE_SUFFIX)
LOCAL_CERTIFICATE := platform

include $(BUILD_PREBUILT)


EMBMS_MIDDLEWARE := vendor/qcom/proprietary/qrdplus/InternalUseOnly/Carrier/Preload/apps/Common/libmsp.so
BUILD_EMBMS_MIDDLEWARE=$(shell if [ -f $(EMBMS_MIDDLEWARE) ]; then echo "true"; else echo "false"; fi;)

ifeq ($(BUILD_EMBMS_MIDDLEWARE),true)

LOCAL_PATH :=  vendor/qcom/proprietary/qrdplus/InternalUseOnly/Carrier/Preload/apps/Common/

include $(CLEAR_VARS)

LOCAL_MODULE := QAS_DVC_MSP
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := $(LOCAL_MODULE).apk
LOCAL_MODULE_CLASS := APPS
LOCAL_MODULE_SUFFIX := $(COMMON_ANDROID_PACKAGE_SUFFIX)
LOCAL_CERTIFICATE := platform

include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libmsp
LOCAL_SRC_FILES := libmsp.so
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_SUFFIX := .so
LOCAL_MODULE_PATH := $(TARGET_OUT)/app/QAS_DVC_MSP/lib/arm
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_REQUIRED_MODULES := libmsp

include $(BUILD_PREBUILT)

endif

