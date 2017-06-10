ifeq ($(call is-board-platform-in-list,msm8909 msm8916),true)
include $(call all-subdir-makefiles)
endif
