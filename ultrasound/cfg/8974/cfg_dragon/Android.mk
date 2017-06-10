ifneq ($(BUILD_TINY_ANDROID),true)

ROOT_DIR := $(call my-dir)
LOCAL_PATH := $(ROOT_DIR)
include $(CLEAR_VARS)

# ------------------------------------------------------------------------------
#       FORM_FACTOR dragon
# ------------------------------------------------------------------------------

include $(CLEAR_VARS)
LOCAL_MODULE:= form_factor_dragon.cfg
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
LOCAL_MODULE:= mixer_paths_dragon.xml
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
LOCAL_MODULE:= usf_tester_epos_dragon.cfg
LOCAL_MODULE_CLASS := DATA
LOCAL_SRC_FILES := $(LOCAL_MODULE)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)/usf/tester/cfg_dragon
LOCAL_MODULE_OWNER := qcom
include $(BUILD_PREBUILT)

# ------------------------------------------------------------------------------
#       Tester daemon transparent data files
# ------------------------------------------------------------------------------
include $(CLEAR_VARS)
LOCAL_MODULE:= usf_tester_epos_dragon_tx_transparent_data.bin
LOCAL_MODULE_CLASS := DATA
LOCAL_SRC_FILES := $(LOCAL_MODULE)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)/usf/tester/cfg_dragon
LOCAL_MODULE_OWNER := qcom
include $(BUILD_PREBUILT)

# ------------------------------------------------------------------------------
#       EPOS daemon configuration files
# ------------------------------------------------------------------------------
include $(CLEAR_VARS)
LOCAL_MODULE:= usf_epos_dragon.cfg
LOCAL_MODULE_CLASS := DATA
LOCAL_SRC_FILES := $(LOCAL_MODULE)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)/usf/epos/cfg_dragon
LOCAL_MODULE_OWNER := qcom
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE:= usf_epos_dragon_ps_disabled.cfg
LOCAL_MODULE_CLASS := DATA
LOCAL_SRC_FILES := $(LOCAL_MODULE)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)/usf/epos/cfg_dragon
LOCAL_MODULE_OWNER := qcom
include $(BUILD_PREBUILT)

# ------------------------------------------------------------------------------
#       Power save tuning parameters (EPOS) files
# ------------------------------------------------------------------------------
include $(CLEAR_VARS)
LOCAL_MODULE:= ps_tuning1_dragon.bin
LOCAL_MODULE_CLASS := DATA
LOCAL_SRC_FILES := ps_tuning1.bin
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)/usf/epos/cfg_dragon
LOCAL_MODULE_OWNER := qcom
include $(BUILD_PREBUILT)

# ------------------------------------------------------------------------------
#       Pairing daemon configuration files
# ------------------------------------------------------------------------------
include $(CLEAR_VARS)
LOCAL_MODULE:= usf_pairing_dragon.cfg
LOCAL_MODULE_CLASS := DATA
LOCAL_SRC_FILES := $(LOCAL_MODULE)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)/usf/pairing/cfg_dragon
LOCAL_MODULE_OWNER := qcom
include $(BUILD_PREBUILT)

# ------------------------------------------------------------------------------
#       SW Calib daemon configuration files
# ------------------------------------------------------------------------------
include $(CLEAR_VARS)
LOCAL_MODULE:= usf_sw_calib_dragon.cfg
LOCAL_MODULE_CLASS := DATA
LOCAL_SRC_FILES := $(LOCAL_MODULE)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)/usf/sw_calib/cfg_dragon
LOCAL_MODULE_OWNER := qcom
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE:= usf_sw_calib_tester_dragon.cfg
LOCAL_MODULE_CLASS := DATA
LOCAL_SRC_FILES := $(LOCAL_MODULE)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)/usf/sw_calib/cfg_dragon
LOCAL_MODULE_OWNER := qcom
include $(BUILD_PREBUILT)

# ------------------------------------------------------------------------------
#       DigitalPen Service configuration files
# ------------------------------------------------------------------------------
include $(CLEAR_VARS)
LOCAL_MODULE:= service_settings_dragon.xml
LOCAL_MODULE_CLASS := DATA
LOCAL_SRC_FILES := $(LOCAL_MODULE)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)/usf/epos/cfg_dragon
LOCAL_MODULE_OWNER := qcom
include $(BUILD_PREBUILT)

# Build all of the sub-targets
include $(call all-makefiles-under, $(LOCAL_PATH))

endif #BUILD_TINY_ANDROID

