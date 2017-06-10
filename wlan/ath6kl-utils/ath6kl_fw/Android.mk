ifneq ($(TARGET_SIMULATOR),true)

LOCAL_PATH:= $(call my-dir)
define include-ar6k-prebuilt
    include $$(CLEAR_VARS)
    LOCAL_MODULE := $(4)
    LOCAL_MODULE_STEM := $(3)
    LOCAL_MODULE_TAGS := optional
    LOCAL_MODULE_CLASS := ETC
    LOCAL_MODULE_PATH := $(2)
    LOCAL_SRC_FILES := $(1)
    include $$(BUILD_PREBUILT)
endef

define add-ar6k-prebuilt-file
    $(eval $(include-ar6k-prebuilt))
endef
# HW2.1.1 firmware

ar6k_hw21_dst_dir := $(TARGET_OUT)/etc/firmware/ath6k/AR6003/hw2.1.1
$(call add-ar6k-prebuilt-file,hw2.1.1/fw-4.bin,$(ar6k_hw21_dst_dir),fw-4.bin,fw-4)
$(call add-ar6k-prebuilt-file,hw2.1.1/softmac,$(ar6k_hw21_dst_dir),softmac,softmac)
$(call add-ar6k-prebuilt-file,hw2.1.1/athtcmd_ram.bin,$(ar6k_hw21_dst_dir),athtcmd_ram.bin,athtcmd_ram)
$(call add-ar6k-prebuilt-file,hw2.1.1/nullTestFlow.bin,$(ar6k_hw21_dst_dir),nullTestFlow.bin,nullTestFlow)
$(call add-ar6k-prebuilt-file,hw2.1.1/athwlan.bin,$(ar6k_hw21_dst_dir),athwlan.bin,athwlan)
$(call add-ar6k-prebuilt-file,hw2.1.1/utf.bin,$(ar6k_hw21_dst_dir),utf.bin,utf)
$(call add-ar6k-prebuilt-file,hw2.1.1/bdata.bin,$(TARGET_OUT)/../persist,bdata.bin,bdata)
ar6k_hw21_dst_dir :=


$(shell mkdir -p $(TARGET_OUT)/etc/firmware/ath6k/AR6003/hw2.1.1; \
        ln -sf /persist/bdata.bin \
        $(TARGET_OUT)/etc/firmware/ath6k/AR6003/hw2.1.1/bdata.bin)
endif
