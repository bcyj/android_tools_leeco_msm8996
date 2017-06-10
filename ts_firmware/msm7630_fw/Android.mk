LOCAL_PATH := $(call my-dir)

ifeq ($(HAVE_CYTTSP_FW_UPGRADE),true)
include $(CLEAR_VARS)
LOCAL_MODULE       := cyttsp_7630_fluid.hex
LOCAL_MODULE_TAGS  := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH  := $(TARGET_OUT_ETC)/firmware
LOCAL_SRC_FILES    := $(LOCAL_MODULE)
include $(BUILD_PREBUILT)
endif
