LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

relative_path := ../../../InternalUseOnly/Carrier/Resource/Wallpaper
absolute_path := vendor/qcom/proprietary/qrdplus/InternalUseOnly/Carrier/Resource/Wallpaper
EXTRA_RES := ct/res

# we need to use the absolute path to judge if the extra res exist.
ifeq ($(absolute_path)/$(EXTRA_RES), $(wildcard $(absolute_path)/$(EXTRA_RES)))
    # we need to use the relative path to set as the res dir.
    res_dir := $(relative_path)/$(EXTRA_RES) res
else
    res_dir := res
endif

LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := $(call all-java-files-under, src)
LOCAL_RESOURCE_DIR := $(addprefix $(LOCAL_PATH)/, $(res_dir))
LOCAL_AAPT_FLAGS := --auto-add-overlay
LOCAL_PACKAGE_NAME := CtWallpaper
LOCAL_CERTIFICATE := shared

# This will install the file in /system/vendor/ChinaTelecom
LOCAL_MODULE_PATH := $(TARGET_OUT)/vendor/ChinaTelecom/system/app

include $(BUILD_PACKAGE)
