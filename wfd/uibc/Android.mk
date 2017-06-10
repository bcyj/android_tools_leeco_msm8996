ifeq ($(call is-vendor-board-platform,QCOM),true)
ifneq ($(call is-board-platform,copper),true)

MM_WFD_UIBC_SRC := $(call my-dir)
include $(call all-subdir-makefiles)

endif
endif
