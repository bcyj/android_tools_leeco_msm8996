CNE_PATH := $(call my-dir)

ifeq ($(BOARD_USES_QCNE),true)
ifneq ($(BUILD_TINY_ANDROID),true)
ifneq ($(call is-board-platform,copper),true)
ifeq ($(call is-vendor-board-platform,QCOM),true)
ifeq ($(call is-platform-sdk-version-at-least,18),true)

ifneq ($(TARGET_PRODUCT),full)
  include $(call first-makefiles-under, $(CNE_PATH))
endif

endif
endif
endif
endif
endif
