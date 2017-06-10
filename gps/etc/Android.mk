LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

ifneq ($(LW_FEATURE_SET),true)
# files that live under /system/etc/...
LOCAL_MODULE := izat.conf
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH  := $(TARGET_OUT_ETC)/
LOCAL_SRC_FILES := izat.conf
include $(BUILD_PREBUILT)

#Copy sap.conf to system img
include $(CLEAR_VARS)
LOCAL_MODULE := sap.conf
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_SRC_FILES := sap.conf
LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)
include $(BUILD_PREBUILT)
endif

ifeq ($(call is-platform-sdk-version-at-least,19),true)
#Copy flp.conf to /system/etc/
include $(CLEAR_VARS)
LOCAL_MODULE := flp.conf
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_SRC_FILES := flp.conf
LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)
include $(BUILD_PREBUILT)
endif # include for after jelly bean
