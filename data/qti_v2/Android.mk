BOARD_PLATFORM_LIST := apq8084
BOARD_PLATFORM_LIST += msm8994
BOARD_PLATFORM_LIST += msm8909
ifeq ($(call is-board-platform-in-list,$(BOARD_PLATFORM_LIST)),true)
include $(call all-subdir-makefiles)
endif # QCOM_TARGET_PRODUCT
