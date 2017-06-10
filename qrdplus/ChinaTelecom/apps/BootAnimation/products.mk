absolute_path := vendor/qcom/proprietary/qrdplus/InternalUseOnly/Carrier/Resource/BootAnimation
carrier := ct

define find-subdir-animation-files
    $(patsubst ./%,%,$(shell find -L $(1) -type f))
endef

define add-to-product
    $(eval PRODUCT_PACKAGES += $$(carrier)_$$(shell basename $(basename $(1))))
endef

file_path := $(absolute_path)/$(carrier)
ifeq ($(file_path), $(wildcard $(file_path)))
    $(foreach f,$(call find-subdir-animation-files,$(file_path)),$(call add-to-product,$f))
endif