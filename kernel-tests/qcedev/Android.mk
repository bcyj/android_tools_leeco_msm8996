ifneq (,$(filter arm aarch64 arm64, $(TARGET_ARCH)))

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

ifeq ($(call is-board-platform,msm7627a),true)
	QCEDEV_CFLAGS := -DQCEDEV_TARGET_MSM7x27A
else ifeq ($(call is-board-platform,msm7627_surf),true)
	QCEDEV_CFLAGS := -DCEDEV_TARGET_MSM7x27
else ifeq ($(call is-board-platform,msm7630_surf),true)
	QCEDEV_CFLAGS := -DQCEDEV_TARGET_MSM7x30_SURF
else ifeq ($(call is-board-platform,msm7630_fusion),true)
	QCEDEV_CFLAGS := -DQCEDEV_TARGET_MSM7x30_FUSION
else ifeq ($(call is-board-platform,msm8660),true)
	QCEDEV_CFLAGS := -DQCEDEV_TARGET_MSM8660
else ifeq ($(call is-board-platform,msm8960),true)
	QCEDEV_CFLAGS := -DQCEDEV_TARGET_MSM8960
else ifeq ($(call is-board-platform,msm8974),true)
	QCEDEV_CFLAGS := -DQCEDEV_TARGET_MSM8974
else ifeq ($(call is-board-platform,msm8226),true)
	QCEDEV_CFLAGS := -DQCEDEV_TARGET_MSM8226
else ifeq ($(call is-board-platform,msm8610),true)
        QCEDEV_CFLAGS := -DQCEDEV_TARGET_MSM8610
else ifeq ($(call is-board-platform,apq8084),true)
        QCEDEV_CFLAGS := -DQCEDEV_TARGET_APQ8084
else ifeq ($(call is-board-platform,msm8916),true)
	QCEDEV_CFLAGS := -DQCEDEV_TARGET_MSM8916
else ifeq ($(call is-board-platform,msm8994),true)
	QCEDEV_CFLAGS := -DQCEDEV_TARGET_MSM8994
else ifeq ($(call is-board-platform,msm8909),true)
	QCEDEV_CFLAGS := -DQCEDEV_TARGET_MSM8909
endif

LOCAL_C_INCLUDES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

LOCAL_SHARED_LIBRARIES := \
        libc \
        libcutils \
        libutils

LOCAL_MODULE := qcedev_test
LOCAL_SRC_FILES := qcedev_test.c
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_DATA)/kernel-tests
LOCAL_CFLAGS := $(QCEDEV_CFLAGS)
LOCAL_PRELINK_MODULE := false
LOCAL_MODULE_OWNER := qcom
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE := qcedev_test.sh
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_SRC_FILES := qcedev_test.sh
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_DATA)/kernel-tests
LOCAL_MODULE_OWNER := qcom
include $(BUILD_PREBUILT)
endif
