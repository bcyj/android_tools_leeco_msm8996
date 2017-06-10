LOCAL_PATH := $(call my-dir)


ifndef NFC_ANTENNA
NFC_ANTENNA := unspecified

ifeq ($(TARGET_BOARD_PLATFORM),msm8974)
NFC_ANTENNA:=8974_30x50_0001
endif

ifeq ($(TARGET_BOARD_PLATFORM),msm8994)
NFC_ANTENNA:=8994_27.8x27.8_0000
endif

endif

#ensure the antenna nvm selected replaces whatever is in the build
$(shell touch $(LOCAL_PATH)/nvm-files/nfc_nvm*$(NFC_ANTENNA).bin)

include $(call all-subdir-makefiles)

include $(CLEAR_VARS)
LOCAL_MODULE       := Signedrompatch_v20.bin
LOCAL_MODULE_TAGS  := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH  := $(TARGET_OUT_ETC)/firmware
LOCAL_SRC_FILES    := $(LOCAL_MODULE)
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE       := Signedrompatch_v21.bin
LOCAL_MODULE_TAGS  := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH  := $(TARGET_OUT_ETC)/firmware
LOCAL_SRC_FILES    := $(LOCAL_MODULE)
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE       := Signedrompatch_v24.bin
LOCAL_MODULE_TAGS  := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH  := $(TARGET_OUT_ETC)/firmware
LOCAL_SRC_FILES    := $(LOCAL_MODULE)
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE       := Signedrompatch_v30.bin
LOCAL_MODULE_TAGS  := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH  := $(TARGET_OUT_ETC)/firmware
LOCAL_SRC_FILES    := $(LOCAL_MODULE)
include $(BUILD_PREBUILT)

ifeq ($(NFC_ANTENNA),unspecified)
include $(CLEAR_VARS)
LOCAL_MODULE       := nfc_test.bin
LOCAL_MODULE_TAGS  := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH  := $(TARGET_OUT_ETC)/firmware
LOCAL_SRC_FILES    := $(LOCAL_MODULE)
include $(BUILD_PREBUILT)


else #NFC_ANTENNA specified
include $(CLEAR_VARS)
LOCAL_MODULE       := nfc_test.bin
LOCAL_MODULE_TAGS  := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH  := $(TARGET_OUT_ETC)/firmware
LOCAL_SRC_FILES    := nvm-files/nfc_nvm_unfused_$(NFC_ANTENNA).bin
$(info nfc_test.bin  sourced from nvm-files/nfc_nvm_unfused_$(NFC_ANTENNA).bin);
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE       := fused_nvm.bin
LOCAL_MODULE_TAGS  := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH  := $(TARGET_OUT_ETC)/firmware
LOCAL_SRC_FILES    := nvm-files/nfc_nvm_fused_$(NFC_ANTENNA).bin
$(info fused_nvm.bin sourced from nvm-files/nfc_nvm_fused_$(NFC_ANTENNA).bin);
include $(BUILD_PREBUILT)
endif #NFC_ANTENNA specified
