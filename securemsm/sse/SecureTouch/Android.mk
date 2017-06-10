ifeq ($(call is-board-platform-in-list, msm8974 apq8084 msm8994 msm8916),true)
  include $(call all-subdir-makefiles)
endif

