LOCAL_PATH := $(call my-dir)

ifeq ($(HAVE_MXT_CFG),true)
include $(CLEAR_VARS)
LOCAL_MODULE       := mxt1386e_apq8064_liquid.cfg
LOCAL_MODULE_TAGS  := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH  := $(TARGET_OUT_ETC)/firmware
LOCAL_SRC_FILES    := $(LOCAL_MODULE)
include $(BUILD_PREBUILT)
endif
