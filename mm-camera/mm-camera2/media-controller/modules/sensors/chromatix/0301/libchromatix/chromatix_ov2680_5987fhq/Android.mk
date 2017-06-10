ifeq ($(call is-vendor-board-platform,QCOM),true)
include $(call all-subdir-makefiles)
endif
