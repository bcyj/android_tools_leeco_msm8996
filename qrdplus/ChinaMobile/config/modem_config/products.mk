LOCAL_PATH := vendor/qcom/proprietary/qrdplus/ChinaMobile/config/modem_config

define find-subdir-mbn-files
    $(patsubst ./%,%,$(shell cd $(LOCAL_PATH) ; find ./ -type f -name "*.mbn"))
endef

define add-to-product
    $(eval PRODUCT_PACKAGES += $(1))
endef

$(foreach f,$(call find-subdir-mbn-files),$(call add-to-product,$f))
