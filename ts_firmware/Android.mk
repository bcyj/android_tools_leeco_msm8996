TS_FW_LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

ifeq ($(call is-vendor-board-platform,QCOM),true)
    include $(TS_FW_LOCAL_PATH)/ts_tools/evt-sniff/Android.mk
endif

ifeq ($(call is-chipset-in-board-platform,msm7630),true)
    include $(TS_FW_LOCAL_PATH)/msm7630_fw/Android.mk
endif

ifeq ($(call is-board-platform,msm8660),true)
    include $(TS_FW_LOCAL_PATH)/msm8660_fw/Android.mk
endif

ifeq ($(call is-board-platform,msm8960),true)
    include $(TS_FW_LOCAL_PATH)/msm8960_fw/Android.mk
endif

ifeq ($(call is-board-platform,msm7627a),true)
    include $(TS_FW_LOCAL_PATH)/msm7627a_fw/Android.mk
endif
