LOCAL_PATH := $(call my-dir)

define find-subdir-mbn-files
    $(patsubst ./%,%,$(shell cd $(LOCAL_PATH) ; find ./ -type f -name "*.mbn"))
endef

define add-prebuilt-mbn-files
    $(eval $(prebuilt-mbn-files))
endef

define prebuilt-mbn-files
    include $$(CLEAR_VARS)
    LOCAL_MODULE := $(1)
    LOCAL_SRC_FILES := $$(LOCAL_MODULE)
    LOCAL_MODULE_TAGS := optional
    LOCAL_MODULE_CLASS := ETC
    LOCAL_MODULE_PATH := $$(TARGET_OUT)/vendor/ChinaMobile/data/modem_config
    include $$(BUILD_PREBUILT)
endef

$(foreach f,$(call find-subdir-mbn-files),$(call add-prebuilt-mbn-files,$f))

CONFIG_FILE := config_modem_mode.xml
$(shell mkdir -p $(TARGET_OUT)/vendor/ChinaMobile/data/modem_config)
$(shell cp -r $(LOCAL_PATH)/$(CONFIG_FILE) \
    $(TARGET_OUT)/vendor/ChinaMobile/data/modem_config/$(CONFIG_FILE))
