ifneq ($(BUILD_TINY_ANDROID),true)

ROOT_DIR := $(call my-dir)
LOCAL_PATH := $(ROOT_DIR)
include $(CLEAR_VARS)

# ------------------------------------------------------------------------------
#       Copy pairing series files
# ------------------------------------------------------------------------------
define add_series_packets
    include $(CLEAR_VARS)
    LOCAL_MODULE:= $1
    LOCAL_MODULE_CLASS := DATA
    LOCAL_SRC_FILES := $$(LOCAL_MODULE)
    LOCAL_MODULE_TAGS := optional
    LOCAL_MODULE_PATH := $(PRODUCT_OUT)/persist/usf/pen_pairing/ref1
    LOCAL_MODULE_OWNER := qcom
    include $(BUILD_PREBUILT)
endef
FILE_LIST := $(wildcard $(LOCAL_PATH)/*.dat)
SERIES_FILES := $(FILE_LIST:$(LOCAL_PATH)/%=%)
$(foreach item,$(SERIES_FILES),$(eval $(call add_series_packets,$(item))))

endif #BUILD_TINY_ANDROID

