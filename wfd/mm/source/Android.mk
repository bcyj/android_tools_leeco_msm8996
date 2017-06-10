ifeq ($(call is-vendor-board-platform,QCOM),true)
ifneq ($(call is-board-platform,copper),true)

include $(call all-subdir-makefiles)

endif
endif
