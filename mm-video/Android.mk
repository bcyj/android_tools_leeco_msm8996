VIDEO_DIR := $(call my-dir)
LOCAL_PATH := $(VIDEO_DIR)
include $(CLEAR_VARS)

ifeq ($(call is-board-platform-in-list, msm8610),true)
    include $(VIDEO_DIR)/ittiam_vidc/vdec/Android.mk
    include $(VIDEO_DIR)/ittiam_vidc/venc/Android.mk
endif

ifeq ($(call is-board-platform-in-list, msm8909),true)
    include $(VIDEO_DIR)/ittiam_vidc/vdec/8909/Android.mk
endif
