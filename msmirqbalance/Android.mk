BOARD_PLATFORM_LIST := msm8916
BOARD_PLATFORM_LIST += apq8084
BOARD_PLATFORM_LIST += msm8994
BOARD_PLATFORM_LIST += msm8992

ifeq ($(call is-board-platform-in-list,$(BOARD_PLATFORM_LIST)),true)

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := msm_irqbalance.c

LOCAL_SHARED_LIBRARIES := libcutils
LOCAL_SHARED_LIBRARIES += libutils

LOCAL_MODULE := msm_irqbalance

LOCAL_MODULE_TAGS := optional

LOCAL_CFLAGS += \
	-DUSE_ANDROID_LOG \

LOCAL_MODULE_OWNER := qti
include $(BUILD_EXECUTABLE)

endif
