ifneq ($(TARGET_SIMULATOR),true)

LOCAL_PATH:= $(call my-dir)
define include-ar3k-prebuilt
    include $$(CLEAR_VARS)
    LOCAL_MODULE := $(4)
    LOCAL_MODULE_STEM := $(3)
    LOCAL_MODULE_TAGS := optional
    LOCAL_MODULE_CLASS := ETC
    LOCAL_MODULE_PATH := $(2)
    LOCAL_SRC_FILES := $(1)
    include $$(BUILD_PREBUILT)
endef

define add-ar3k-prebuilt-file
    $(eval $(include-ar3k-prebuilt))
endef
# AR3002,2.2.1 firmware

ar3k_egret221_dst_dir := $(TARGET_OUT_ETC)/firmware/ar3k/1020201
$(call add-ar3k-prebuilt-file,01020201/RamPatch.txt,$(ar3k_egret221_dst_dir),RamPatch.txt,AR3002_RamPatch)
$(call add-ar3k-prebuilt-file,01020201/PS_ASIC.pst,$(ar3k_egret221_dst_dir),PS_ASIC.pst,AR3002_PS_ASIC)
ar3k_egret221_dst_dir :=

endif

