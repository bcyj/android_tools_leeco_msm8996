LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

US_PRODUCT_LIST := msm8960
US_PRODUCT_LIST += msm8974
US_PRODUCT_LIST += apq8084
US_PRODUCT_LIST += plutonium

# Build all of the sub-targets
ifeq ($(call is-board-platform-in-list,$(US_PRODUCT_LIST)),true)
  include $(call all-makefiles-under, $(LOCAL_PATH))
endif

