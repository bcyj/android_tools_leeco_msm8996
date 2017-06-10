LOCAL_PATH:= $(call my-dir)

relative_path := ../../../InternalUseOnly/Carrier/Resource/Mail189
absolute_path := vendor/qcom/proprietary/qrdplus/InternalUseOnly/Carrier/Resource/Mail189

# we need to use the absolute path to judge if the extra res exist.
ifeq ($(absolute_path)/res, $(wildcard $(absolute_path)/res))
    # we need to use the relative path to set as the res dir.
    res_dir := $(relative_path)/res res

    include $(CLEAR_VARS)

    LOCAL_MODULE_TAGS := optional
    LOCAL_SRC_FILES := $(call all-subdir-java-files)
    LOCAL_RESOURCE_DIR := $(addprefix $(LOCAL_PATH)/, $(res_dir))
    LOCAL_AAPT_FLAGS := --auto-add-overlay
    LOCAL_PACKAGE_NAME := Mail189

    # This will install the file in /system/vendor/ChinaTelecom
    LOCAL_MODULE_PATH := $(TARGET_OUT)/vendor/ChinaTelecom/system/app

    include $(BUILD_PACKAGE)
endif
