ifneq ($(TARGET_SIMULATOR),true)

LOCAL_PATH:= $(call my-dir)
define include-ar6004-prebuilt
    include $$(CLEAR_VARS)
    LOCAL_MODULE := $(4)
    LOCAL_MODULE_STEM := $(3)
    LOCAL_MODULE_TAGS := optional
    LOCAL_MODULE_CLASS := ETC
    LOCAL_MODULE_PATH := $(2)
    LOCAL_SRC_FILES := $(1)
    LOCAL_MODULE_OWNER := qcom
    include $$(BUILD_PREBUILT)
endef

define add-ar6004-prebuilt-file
    $(eval $(include-ar6004-prebuilt))
endef

# HW1.2 firmware
ar6004_hw12_dst_dir := $(TARGET_OUT)/etc/firmware/ath6k/AR6004/hw1.2
$(call add-ar6004-prebuilt-file,hw1.2/fw.ram.bin,$(ar6004_hw12_dst_dir),fw.ram.bin,ar6004_fw_12)
$(call add-ar6004-prebuilt-file,hw1.2/bdata.bin,$(ar6004_hw12_dst_dir),bdata.bin,ar6004_bdata_12)
ar6004_hw12_dst_dir :=

# HW1.3 firmware
ar6004_hw13_dst_dir := $(TARGET_OUT)/etc/firmware/ath6k/AR6004/hw1.3
$(call add-ar6004-prebuilt-file,hw1.3/fw.ram.bin_usb,$(ar6004_hw13_dst_dir),fw.ram.bin_usb,ar6004_usb_fw_13)
$(call add-ar6004-prebuilt-file,hw1.3/bdata.bin_usb,$(ar6004_hw13_dst_dir),bdata.bin_usb,ar6004_usb_bdata_13)
$(call add-ar6004-prebuilt-file,hw1.3/fw.ram.bin_sdio,$(ar6004_hw13_dst_dir),fw.ram.bin_sdio,ar6004_sdio_fw_13)
$(call add-ar6004-prebuilt-file,hw1.3/bdata.bin_sdio,$(ar6004_hw13_dst_dir),bdata.bin_sdio,ar6004_sdio_bdata_13)
$(call add-ar6004-prebuilt-file,hw1.3/fw_ext.ram.bin,$(ar6004_hw13_dst_dir),fw_ext.ram.bin,ar6004_usb_fw_ext_13)
ar6004_hw13_dst_dir :=

$(shell mkdir -p $(TARGET_OUT)/etc/firmware/ath6k/AR6004/hw1.3; \
        ln -sf /system/etc/firmware/ath6k/AR6004/hw1.3/fw.ram.bin_usb \
               $(TARGET_OUT)/etc/firmware/ath6k/AR6004/hw1.3/fw.ram.bin; \
        ln -sf /system/etc/firmware/ath6k/AR6004/hw1.3/bdata.bin_usb \
               $(TARGET_OUT)/etc/firmware/ath6k/AR6004/hw1.3/bdata.bin)

# HW2.0.4/3.0 firmware
ar6004_hw30_dst_dir := $(TARGET_OUT)/etc/firmware/ath6k/AR6004/hw3.0
$(call add-ar6004-prebuilt-file,hw3.0/fw.ram.bin,$(ar6004_hw30_dst_dir),fw.ram.bin,ar6004_fw_30)
$(call add-ar6004-prebuilt-file,hw3.0/bdata.bin_usb,$(ar6004_hw30_dst_dir),bdata.bin_usb,ar6004_usb_bdata_30)
$(call add-ar6004-prebuilt-file,hw3.0/bdata.bin_sdio,$(ar6004_hw30_dst_dir),bdata.bin_sdio,ar6004_sdio_bdata_30)
ar6004_hw30_dst_dir :=

$(shell mkdir -p $(TARGET_OUT)/etc/firmware/ath6k/AR6004/hw3.0; \
        ln -sf /system/etc/firmware/ath6k/AR6004/hw3.0/bdata.bin_usb \
               $(TARGET_OUT)/etc/firmware/ath6k/AR6004/hw3.0/bdata.bin)

ifeq ($(BOARD_FORCE_ATH_WLAN_AR6004), true)
include $(CLEAR_VARS)
LOCAL_MODULE       := ar6004_wlan.conf
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH  := $(TARGET_OUT)/etc/firmware/ath6k/AR6004
LOCAL_SRC_FILES    := $(LOCAL_MODULE)
LOCAL_MODULE_OWNER := qcom
include $(BUILD_PREBUILT)
endif

endif
