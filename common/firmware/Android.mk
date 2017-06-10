TARGET_LIST := msm8610
TARGET_LIST += msm8916

ifeq ($(call is-board-platform-in-list,$(TARGET_LIST)),true)

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE       := ice40.bin
LOCAL_MODULE_TAGS  := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH  := $(TARGET_OUT_ETC)/firmware
LOCAL_SRC_FILES    := $(LOCAL_MODULE)

include $(BUILD_PREBUILT)

endif
