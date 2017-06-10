IMS_DIR := $(call my-dir)
include $(CLEAR_VARS)
ifneq ($(TARGET_USES_IMS),false)
ifeq ($(call is-board-platform-in-list,msm8960 msm8974 msm8226 msm8994 apq8084 msm8916 msm8909 msm8909_512),true)
    include $(IMS_DIR)/imscamera/Android.mk
endif
endif