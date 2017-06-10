ifneq ($(BUILD_TINY_ANDROID),true)

ROOT_DIR := $(call my-dir)
LOCAL_PATH := $(ROOT_DIR)
include $(CLEAR_VARS)


# ------------------------------------------------------------------------------
#       TSC device configuration file
# ------------------------------------------------------------------------------
include $(CLEAR_VARS)
LOCAL_MODULE:= usf_tsc.idc
LOCAL_MODULE_CLASS := DATA
LOCAL_SRC_FILES := $(LOCAL_MODULE)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT)/usr/idc
LOCAL_MODULE_OWNER := qcom
include $(BUILD_PREBUILT)

# ------------------------------------------------------------------------------
#       TSC device configuration file
# ------------------------------------------------------------------------------
include $(CLEAR_VARS)
LOCAL_MODULE:= usf_tsc_ext.idc
LOCAL_MODULE_CLASS := DATA
LOCAL_SRC_FILES := $(LOCAL_MODULE)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT)/usr/idc
LOCAL_MODULE_OWNER := qcom
include $(BUILD_PREBUILT)

# ------------------------------------------------------------------------------
#       TSC device configuration file
# ------------------------------------------------------------------------------
include $(CLEAR_VARS)
LOCAL_MODULE:= usf_tsc_ptr.idc
LOCAL_MODULE_CLASS := DATA
LOCAL_SRC_FILES := $(LOCAL_MODULE)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT)/usr/idc
LOCAL_MODULE_OWNER := qcom
include $(BUILD_PREBUILT)

# ------------------------------------------------------------------------------
#       Scope Debugger configuration file
# ------------------------------------------------------------------------------
include $(CLEAR_VARS)
LOCAL_MODULE:= ASDConf.sdc
LOCAL_MODULE_CLASS := DATA
LOCAL_SRC_FILES := $(LOCAL_MODULE)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)/usf/epos
LOCAL_MODULE_OWNER := qcom
include $(BUILD_PREBUILT)

# ------------------------------------------------------------------------------
#       Create directories for daemons pattern files
# ------------------------------------------------------------------------------
$(shell mkdir -p --mode=755 $(TARGET_OUT_ETC)/usf/tester/pattern;)

$(shell mkdir -p --mode=755 $(TARGET_OUT_ETC)/usf/epos/pattern;)

$(shell mkdir -p --mode=755 $(TARGET_OUT_ETC)/usf/p2p/pattern;)

$(shell mkdir -p --mode=755 $(TARGET_OUT_ETC)/usf/hovering/pattern;)

$(shell mkdir -p --mode=755 $(TARGET_OUT_ETC)/usf/gesture/pattern;)

$(shell mkdir -p --mode=755 $(TARGET_OUT_ETC)/usf/proximity/pattern;)

# ------------------------------------------------------------------------------
#       Create directories for daemons record files
# ------------------------------------------------------------------------------
$(shell mkdir -p --mode=755 $(TARGET_OUT_ETC)/usf/tester/rec;)

$(shell mkdir -p --mode=755 $(TARGET_OUT_ETC)/usf/epos/rec;)

$(shell mkdir -p --mode=755 $(TARGET_OUT_ETC)/usf/p2p/rec;)

$(shell mkdir -p --mode=755 $(TARGET_OUT_ETC)/usf/hovering/rec;)

$(shell mkdir -p --mode=755 $(TARGET_OUT_ETC)/usf/gesture/rec;)

$(shell mkdir -p --mode=755 $(TARGET_OUT_ETC)/usf/sync_gesture/rec;)

$(shell mkdir -p --mode=755 $(TARGET_OUT_ETC)/usf/proximity/rec;)

$(shell mkdir -p --mode=755 $(TARGET_OUT_ETC)/usf/pairing/rec;)

$(shell mkdir -p --mode=755 $(TARGET_OUT_ETC)/usf/sw_calib/rec;)

# ------------------------------------------------------------------------------
#       Create directory for Epos tuning files
# ------------------------------------------------------------------------------
$(shell mkdir -p --mode=755 $(PRODUCT_OUT)/persist/usf/epos;)

# ------------------------------------------------------------------------------
#       Create directory for Mixer profiles directory
# ------------------------------------------------------------------------------
$(shell mkdir -p --mode=755 $(PRODUCT_OUT)/persist/usf/mixer;)

# ------------------------------------------------------------------------------
#       Create directory for Pairing series files
# ------------------------------------------------------------------------------
$(shell mkdir -p --mode=755 $(PRODUCT_OUT)/persist/usf/pen_pairing;)

# ------------------------------------------------------------------------------
#       Create directory for SW calib calibration file
# ------------------------------------------------------------------------------
$(shell mkdir -p --mode=755 $(PRODUCT_OUT)/persist/usf/sw_calib;)

# Build all of the sub-targets
include $(call all-makefiles-under, $(LOCAL_PATH))

endif #BUILD_TINY_ANDROID

