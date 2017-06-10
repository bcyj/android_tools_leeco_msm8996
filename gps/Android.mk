ifneq ($(BUILD_TINY_ANDROID),true)
ifneq ($(BOARD_VENDOR_QCOM_GPS_LOC_API_HARDWARE),)
ifneq ($(call is-board-platform,copper),true)

LOCAL_PATH := $(call my-dir)

# This property is used by GPS HAL and Wiper to cooperate with QC NLP
# note: this property must be aligned with persist.loc.nlp_name
# 1: QC Network Location Provider is in use. Note persist.loc.nlp_name must be set.
# 0: otherwise. Note persist.loc.nlp_name must be cleared/commented out.
ADDITIONAL_BUILD_PROPERTIES += persist.gps.qc_nlp_in_use=1

# package name of QC NLP, if so chosen in persist.gps.qc_nlp_in_use
# note: this property must be aligned with persist.gps.qc_nlp_in_use,
#       for LocationManagerService.java is controlled by this property only
# note: the length limit for value is 92 characters
ADDITIONAL_BUILD_PROPERTIES += persist.loc.nlp_name=com.qualcomm.location

# This property will decouple the "WiFi & Mobile Network Location" from AGPS database setting(Settings.Global.ASSISTED_GPS_ENABLED)
# 1: AGPS operation is controlled by Settings.Global.ASSISTED_GPS_ENABLED.
#      Recommended for all OEMs who don't use Google SUPL servers.
# 0: AGPS operation is controlled by "WiFi & Mobile Network Location" setting on Android UI
#      Recommended for everybody who use Google SUPL servers.
ADDITIONAL_BUILD_PROPERTIES += ro.gps.agps_provider=1

# This property controls whether PIP is gated by LciService.
# By default, it is set to 0. Set to 1 to only allow LciService to enable/disable PIP.
ADDITIONAL_BUILD_PROPERTIES += ro.pip.gated=0

# Select which RPC lib to link to
LOC_API_USE_LOCAL_RPC:=0
LOC_API_USE_QCOM_AUTO_RPC:=1

# Target-specific makefile
GPS_BUILD_DIR:=$(LOCAL_PATH)/build
GPS_MAKE_INC:=$(TARGET_BOARD_PLATFORM).in
ifeq (, $(wildcard $(GPS_BUILD_DIR)/$(TARGET_BOARD_PLATFORM)*))
   GPS_MAKE_INC=unsupported.in
endif

ifeq ($(TARGET_DEVICE),apq8026_lw)
LW_FEATURE_SET := true
endif

ifeq ($(LW_FEATURE_SET),true)
include $(GPS_BUILD_DIR)/$(GPS_MAKE_INC)
DIR_LIST := $(LOCAL_PATH)/etc/
DIR_LIST += $(LOCAL_PATH)/framework/native/core/
include $(addsuffix Android.mk, $(DIR_LIST))
else
include $(GPS_BUILD_DIR)/$(GPS_MAKE_INC) $(call all-subdir-makefiles)
endif

endif # is-board-platform
endif # BOARD_VENDOR_QCOM_GPS_LOC_API_HARDWARE
endif # BUILD_TINY_ANDROID
