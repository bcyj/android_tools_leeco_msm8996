absolute_path := vendor/qcom/proprietary/qrdplus/InternalUseOnly/Carrier/Preload/cacerts
carrier := ct

define find-subdir-cacert-files
    $(patsubst ./%,%,$(shell find -L $(1) -type f))
endef

define add-to-product
    $(eval PRODUCT_PACKAGES += $$(carrier)_$$(shell basename $(basename $(1))))
endef

file_path := $(absolute_path)/$(carrier)
ifeq ($(file_path), $(wildcard $(file_path)))
    $(foreach f,$(call find-subdir-cacert-files,$(file_path)),$(call add-to-product,$f))
endif