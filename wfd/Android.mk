ifeq ($(call is-vendor-board-platform,QCOM),true)
WFD_DISABLE_PLATFORM_LIST := msm8610 mpq8092 msm_bronze

ifneq ($(call is-board-platform-in-list,$(WFD_DISABLE_PLATFORM_LIST)),true)

include $(call all-subdir-makefiles)

endif
endif # TARGET_USES_WFD
