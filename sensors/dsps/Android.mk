ifeq ($(call is-board-platform-in-list,msm8660 msm8960 copper msm8974 msm8226 apq8084 msm8962 msm8994),true)
BUILD_DSPS:=true
endif

ifeq ($(BUILD_DSPS),true)
include $(call all-subdir-makefiles)
endif
