ifneq ($(BUILD_TINY_ANDROID),true)

ROOT_DIR := $(call my-dir)
LOCAL_PATH := $(ROOT_DIR)
include $(CLEAR_VARS)

# ------------------------------------------------------------------------------
#       FORM_FACTOR liquid
# ------------------------------------------------------------------------------

include $(CLEAR_VARS)
LOCAL_MODULE:= form_factor_liquid.cfg
LOCAL_MODULE_CLASS := DATA
LOCAL_SRC_FILES := $(LOCAL_MODULE)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)/usf
LOCAL_MODULE_OWNER := qcom
include $(BUILD_PREBUILT)

# ------------------------------------------------------------------------------
#       Mixer profiles
# ------------------------------------------------------------------------------

include $(CLEAR_VARS)
LOCAL_MODULE:= mixer_paths_liquid.xml
LOCAL_MODULE_CLASS := DATA
LOCAL_SRC_FILES := $(LOCAL_MODULE)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/persist/usf/mixer
LOCAL_MODULE_OWNER := qcom
include $(BUILD_PREBUILT)

# ------------------------------------------------------------------------------
#       Tester daemon configuration files
# ------------------------------------------------------------------------------
include $(CLEAR_VARS)
LOCAL_MODULE:= usf_tester_epos_liquid.cfg
LOCAL_MODULE_CLASS := DATA
LOCAL_SRC_FILES := $(LOCAL_MODULE)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)/usf/tester/cfg_liquid
LOCAL_MODULE_OWNER := qcom
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE:= usf_tester_p2p_liquid.cfg
LOCAL_MODULE_CLASS := DATA
LOCAL_SRC_FILES := $(LOCAL_MODULE)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)/usf/tester/cfg_liquid
LOCAL_MODULE_OWNER := qcom
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE:= usf_tester_hovering_liquid.cfg
LOCAL_MODULE_CLASS := DATA
LOCAL_SRC_FILES := $(LOCAL_MODULE)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)/usf/tester/cfg_liquid
LOCAL_MODULE_OWNER := qcom
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE:= usf_tester_gesture_liquid.cfg
LOCAL_MODULE_CLASS := DATA
LOCAL_SRC_FILES := $(LOCAL_MODULE)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)/usf/tester/cfg_liquid
LOCAL_MODULE_OWNER := qcom
include $(BUILD_PREBUILT)

# ------------------------------------------------------------------------------
#       Tester daemon transparent data files
# ------------------------------------------------------------------------------
include $(CLEAR_VARS)
LOCAL_MODULE:= usf_tester_epos_liquid_tx_transparent_data.bin
LOCAL_MODULE_CLASS := DATA
LOCAL_SRC_FILES := $(LOCAL_MODULE)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)/usf/tester/cfg_liquid
LOCAL_MODULE_OWNER := qcom
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE:= usf_tester_gesture_liquid_tx_transparent_data.bin
LOCAL_MODULE_CLASS := DATA
LOCAL_SRC_FILES := $(LOCAL_MODULE)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)/usf/tester/cfg_liquid
LOCAL_MODULE_OWNER := qcom
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE:= usf_tester_gesture_liquid_rx_transparent_data.bin
LOCAL_MODULE_CLASS := DATA
LOCAL_SRC_FILES := $(LOCAL_MODULE)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)/usf/tester/cfg_liquid
LOCAL_MODULE_OWNER := qcom
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE:= usf_tester_hovering_liquid_tx_transparent_data.bin
LOCAL_MODULE_CLASS := DATA
LOCAL_SRC_FILES := $(LOCAL_MODULE)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)/usf/tester/cfg_liquid
LOCAL_MODULE_OWNER := qcom
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE:= usf_tester_hovering_liquid_rx_transparent_data.bin
LOCAL_MODULE_CLASS := DATA
LOCAL_SRC_FILES := $(LOCAL_MODULE)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)/usf/tester/cfg_liquid
LOCAL_MODULE_OWNER := qcom
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE:= usf_tester_p2p_liquid_tx_transparent_data.bin
LOCAL_MODULE_CLASS := DATA
LOCAL_SRC_FILES := $(LOCAL_MODULE)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)/usf/tester/cfg_liquid
LOCAL_MODULE_OWNER := qcom
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE:= usf_tester_p2p_liquid_rx_transparent_data.bin
LOCAL_MODULE_CLASS := DATA
LOCAL_SRC_FILES := $(LOCAL_MODULE)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)/usf/tester/cfg_liquid
LOCAL_MODULE_OWNER := qcom
include $(BUILD_PREBUILT)

# ------------------------------------------------------------------------------
#       EPOS daemon configuration files
# ------------------------------------------------------------------------------
include $(CLEAR_VARS)
LOCAL_MODULE:= usf_epos_liquid.cfg
LOCAL_MODULE_CLASS := DATA
LOCAL_SRC_FILES := $(LOCAL_MODULE)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)/usf/epos/cfg_liquid
LOCAL_MODULE_OWNER := qcom
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE:= usf_epos_liquid_ps_disabled.cfg
LOCAL_MODULE_CLASS := DATA
LOCAL_SRC_FILES := $(LOCAL_MODULE)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)/usf/epos/cfg_liquid
LOCAL_MODULE_OWNER := qcom
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE:= usf_epos_liquid_6_channels.cfg
LOCAL_MODULE_CLASS := DATA
LOCAL_SRC_FILES := $(LOCAL_MODULE)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)/usf/epos/cfg_liquid
LOCAL_MODULE_OWNER := qcom
include $(BUILD_PREBUILT)

# ------------------------------------------------------------------------------
#       Power save standy mode tuning parameters (EPOS) file
# ------------------------------------------------------------------------------
include $(CLEAR_VARS)
LOCAL_MODULE:= ps_tuning1_standby_liquid.bin
LOCAL_MODULE_CLASS := DATA
LOCAL_SRC_FILES := ps_tuning1_standby.bin
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)/usf/epos/cfg_liquid
LOCAL_MODULE_OWNER := qcom
include $(BUILD_PREBUILT)

# ------------------------------------------------------------------------------
#       Power save idle mode tuning parameters (EPOS) file
# ------------------------------------------------------------------------------
include $(CLEAR_VARS)
LOCAL_MODULE:= ps_tuning1_idle_liquid.bin
LOCAL_MODULE_CLASS := DATA
LOCAL_SRC_FILES := ps_tuning1_idle.bin
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)/usf/epos/cfg_liquid
LOCAL_MODULE_OWNER := qcom
include $(BUILD_PREBUILT)


# ------------------------------------------------------------------------------
#       P2P daemon configuration files
# ------------------------------------------------------------------------------
include $(CLEAR_VARS)
LOCAL_MODULE:= usf_p2p_liquid.cfg
LOCAL_MODULE_CLASS := DATA
LOCAL_SRC_FILES := $(LOCAL_MODULE)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)/usf/p2p/cfg_liquid
LOCAL_MODULE_OWNER := qcom
include $(BUILD_PREBUILT)

# ------------------------------------------------------------------------------
#       Hovering daemon configuration files
# ------------------------------------------------------------------------------
include $(CLEAR_VARS)
LOCAL_MODULE:= usf_hovering_liquid.cfg
LOCAL_MODULE_CLASS := DATA
LOCAL_SRC_FILES := $(LOCAL_MODULE)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)/usf/hovering/cfg_liquid
LOCAL_MODULE_OWNER := qcom
include $(BUILD_PREBUILT)

# 2 speakers
include $(CLEAR_VARS)
LOCAL_MODULE:= usf_hovering_liquid_2_speaker.cfg
LOCAL_MODULE_CLASS := DATA
LOCAL_SRC_FILES := $(LOCAL_MODULE)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)/usf/hovering/cfg_liquid
LOCAL_MODULE_OWNER := qcom
include $(BUILD_PREBUILT)

# ------------------------------------------------------------------------------
#       Gesture daemon configuration files
# ------------------------------------------------------------------------------
include $(CLEAR_VARS)
LOCAL_MODULE:= usf_gesture_liquid.cfg
LOCAL_MODULE_CLASS := DATA
LOCAL_SRC_FILES := $(LOCAL_MODULE)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)/usf/gesture/cfg_liquid
LOCAL_MODULE_OWNER := qcom
include $(BUILD_PREBUILT)

# ------------------------------------------------------------------------------
#       Sync Gesture daemon configuration files
# ------------------------------------------------------------------------------
include $(CLEAR_VARS)
LOCAL_MODULE:= usf_sync_gesture_liquid.cfg
LOCAL_MODULE_CLASS := DATA
LOCAL_SRC_FILES := $(LOCAL_MODULE)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)/usf/sync_gesture/cfg_liquid
LOCAL_MODULE_OWNER := qcom
include $(BUILD_PREBUILT)

# ------------------------------------------------------------------------------
#       Sync Gesture daemon transparent data binary files
# ------------------------------------------------------------------------------
include $(CLEAR_VARS)
LOCAL_MODULE:= usf_sync_gesture_liquid_tx_transparent_data.bin
LOCAL_MODULE_CLASS := DATA
LOCAL_SRC_FILES := $(LOCAL_MODULE)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)/usf/sync_gesture/cfg_liquid
LOCAL_MODULE_OWNER := qcom
include $(BUILD_PREBUILT)

# ------------------------------------------------------------------------------
#       Proximity daemon configuration files
# ------------------------------------------------------------------------------
# liquid digital

include $(CLEAR_VARS)
LOCAL_MODULE:= usf_proximity_liquid.cfg
LOCAL_MODULE_CLASS := DATA
LOCAL_SRC_FILES := $(LOCAL_MODULE)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)/usf/proximity/cfg_liquid
LOCAL_MODULE_OWNER := qcom
include $(BUILD_PREBUILT)

# ------------------------------------------------------------------------------
#       Proximity daemon transparent data files
# ------------------------------------------------------------------------------

include $(CLEAR_VARS)
LOCAL_MODULE:= usf_proximity_liquid_rx_transparent_data.bin
LOCAL_MODULE_CLASS := DATA
LOCAL_SRC_FILES := $(LOCAL_MODULE)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)/usf/proximity/cfg_liquid
LOCAL_MODULE_OWNER := qcom
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE:= usf_proximity_liquid_tx_transparent_data.bin
LOCAL_MODULE_CLASS := DATA
LOCAL_SRC_FILES := $(LOCAL_MODULE)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)/usf/proximity/cfg_liquid
LOCAL_MODULE_OWNER := qcom
include $(BUILD_PREBUILT)

# ------------------------------------------------------------------------------
#       Pairing daemon configuration files
# ------------------------------------------------------------------------------
include $(CLEAR_VARS)
LOCAL_MODULE:= usf_pairing_liquid.cfg
LOCAL_MODULE_CLASS := DATA
LOCAL_SRC_FILES := $(LOCAL_MODULE)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)/usf/pairing/cfg_liquid
LOCAL_MODULE_OWNER := qcom
include $(BUILD_PREBUILT)

# ------------------------------------------------------------------------------
#       SW Calib daemon configuration files
# ------------------------------------------------------------------------------
include $(CLEAR_VARS)
LOCAL_MODULE:= usf_sw_calib_liquid.cfg
LOCAL_MODULE_CLASS := DATA
LOCAL_SRC_FILES := $(LOCAL_MODULE)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)/usf/sw_calib/cfg_liquid
LOCAL_MODULE_OWNER := qcom
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE:= usf_sw_calib_tester_liquid.cfg
LOCAL_MODULE_CLASS := DATA
LOCAL_SRC_FILES := $(LOCAL_MODULE)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)/usf/sw_calib/cfg_liquid
LOCAL_MODULE_OWNER := qcom
include $(BUILD_PREBUILT)

# ------------------------------------------------------------------------------
#       DigitalPen Service configuration files
# ------------------------------------------------------------------------------
include $(CLEAR_VARS)
LOCAL_MODULE:= service_settings_liquid.xml
LOCAL_MODULE_CLASS := DATA
LOCAL_SRC_FILES := $(LOCAL_MODULE)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)/usf/epos/cfg_liquid
LOCAL_MODULE_OWNER := qcom
include $(BUILD_PREBUILT)

# Build all of the sub-targets
include $(call all-makefiles-under, $(LOCAL_PATH))

endif #BUILD_TINY_ANDROID

