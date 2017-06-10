LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

relative_path := ../../../InternalUseOnly/Carrier/Resource/Stk
absolute_path := vendor/qcom/proprietary/qrdplus/InternalUseOnly/Carrier/Resource/Stk
EXTRA_RES := ct/res

# we need to use the absolute path to judge if the extra res exist.
ifeq ($(absolute_path)/$(EXTRA_RES), $(wildcard $(absolute_path)/$(EXTRA_RES)))
    # we need to use the relative path to set as the res dir.
    res_dir := $(relative_path)/$(EXTRA_RES) res
else
    res_dir := res
endif

LOCAL_RESOURCE_DIR := $(addprefix $(LOCAL_PATH)/, $(res_dir))
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := $(call all-subdir-java-files)
LOCAL_SDK_VERSION := current
LOCAL_PACKAGE_NAME := CtStkRes
LOCAL_MODULE_PATH := $(TARGET_OUT)/vendor/ChinaTelecom/system/vendor/overlay
LOCAL_CERTIFICATE := shared

include $(BUILD_PACKAGE)