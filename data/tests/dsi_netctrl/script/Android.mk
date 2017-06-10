################################################################################
# @file tests/dsi_netctrl/script/Android.mk
# @brief Makefile for building dsi_netctrl test scripts
################################################################################

LOCAL_PATH:= $(call my-dir)

TEST_CASES := \
    dsi_netctrl_common.sh \
    ipv6.sh \
    tethered.sh \
    umts.sh \
    cdma.sh \
    evdo.sh \
    1x.sh \
    lte.sh \
    auto.sh \
    multipdp.sh

define build-testcase
  $(eval include $(CLEAR_VARS)) \
  $(eval LOCAL_MODULE:= $(1)) \
  $(eval LOCAL_MODULE_CLASS := EXECUTABLES) \
  $(eval LOCAL_SRC_FILES := $(1)) \
  $(eval LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/qmi/inc ) \
  $(eval LOCAL_MODULE_TAGS := optional) \
  $(eval LOCAL_MODULE_PATH := $(TARGET_OUT_DATA)/data_test) \
  $(eval LOCAL_MODULE_OWNER := qcom) \
  $(eval include $(BUILD_PREBUILT))
endef

$(eval $(foreach testcase,$(TEST_CASES),$(call build-testcase,$(testcase))))

