LOCAL_PATH := $(call my-dir)

define find-subdir-animation-files
    $(patsubst ./%,%,$(shell cd $(LOCAL_PATH) ; find -L $(1) -type f))
endef

define add-prebuilt-animation
    $(eval $(prebuilt-animation))
endef

define prebuilt-animation
    include $$(CLEAR_VARS)

    LOCAL_MODULE := $$(carrier)_$$(shell basename $(basename $(1)))
    LOCAL_MODULE_CLASS := ETC
    LOCAL_MODULE_STEM := $$(shell basename $(1))
    LOCAL_SRC_FILES := $(1)
    LOCAL_MODULE_PATH := $$(TARGET_OUT)/vendor/ChinaMobile/system/media

    include $$(BUILD_PREBUILT)
endef

internal_path := ../../../InternalUseOnly/Carrier/Resource/BootAnimation
absolute_path := vendor/qcom/proprietary/qrdplus/InternalUseOnly/Carrier/Resource/BootAnimation
carrier := cmcc

ifeq ($(absolute_path)/$(carrier), $(wildcard $(absolute_path)/$(carrier)))
    $(foreach f,$(call find-subdir-animation-files,$(internal_path)/$(carrier)),$(call add-prebuilt-animation,$f))
endif
