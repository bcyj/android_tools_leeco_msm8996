LOCAL_PATH := $(call my-dir)

define find-subdir-cacerts-files
    $(patsubst ./%,%,$(shell cd $(LOCAL_PATH) ; find -L $(1) -type f))
endef

define add-prebuilt-cacerts
    $(eval $(prebuilt-cacerts))
endef

define prebuilt-cacerts
    include $$(CLEAR_VARS)

    LOCAL_MODULE := $$(carrier)_$$(shell basename $(basename $(1)))
    LOCAL_MODULE_CLASS := ETC
    LOCAL_MODULE_STEM := $$(shell basename $(1))
    LOCAL_SRC_FILES := $(1)
    LOCAL_MODULE_PATH := $$(TARGET_OUT)/vendor/ChinaTelecom/system/etc/security/cacerts

    include $$(BUILD_PREBUILT)
endef

internal_path := ../../../InternalUseOnly/Carrier/Preload/cacerts
absolute_path := vendor/qcom/proprietary/qrdplus/InternalUseOnly/Carrier/Preload/cacerts
carrier := ct

ifeq ($(absolute_path)/$(carrier), $(wildcard $(absolute_path)/$(carrier)))
    $(foreach f,$(call find-subdir-cacerts-files,$(internal_path)/$(carrier)),$(call add-prebuilt-cacerts,$f))
endif