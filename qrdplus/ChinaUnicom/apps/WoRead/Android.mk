LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

RELATIVE_PATH := ../../../InternalUseOnly/Carrier/Resource/WoRead
INTERNAL_RES_PATH := vendor/qcom/proprietary/qrdplus/InternalUseOnly/Carrier/Resource/WoRead
EXTRA_RES := res

ifeq ($(INTERNAL_RES_PATH)/$(EXTRA_RES), $(wildcard $(INTERNAL_RES_PATH)/$(EXTRA_RES)))
    res_dir := $(RELATIVE_PATH)/$(EXTRA_RES) res
else
    res_dir := res
endif

LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := $(call all-subdir-java-files)
LOCAL_RESOURCE_DIR := $(addprefix $(LOCAL_PATH)/, $(res_dir))
LOCAL_AAPT_FLAGS := --auto-add-overlay
LOCAL_PACKAGE_NAME := WoRead

# This will install the file in /system/vendor/ChinaUnicom
LOCAL_MODULE_PATH := $(TARGET_OUT)/vendor/ChinaUnicom/system/app

include $(BUILD_PACKAGE)
