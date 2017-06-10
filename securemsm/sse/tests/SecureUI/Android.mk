ifeq ($(call is-board-platform-in-list,msm8974 apq8084 msm8994 msm8916 msm8916_32),true)

include $(call all-subdir-makefiles)

endif # end filter
