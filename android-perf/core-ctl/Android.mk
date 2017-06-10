ifneq (,$(filter msm8994 msm8909, $(TARGET_BOARD_PLATFORM)))
ifneq (,$(filter arm aarch64 arm64, $(TARGET_ARCH)))

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_SRC_FILES := core_ctl.c
DLKM_DIR := device/qcom/common/dlkm
LOCAL_MODULE := core_ctl.ko
LOCAL_MODULE_TAGS := optional
include $(DLKM_DIR)/AndroidKernelModule.mk

endif
endif
