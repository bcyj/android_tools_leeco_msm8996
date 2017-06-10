QCRIL_BOARD_PLATFORM_LIST := msm7627_surf
QCRIL_BOARD_PLATFORM_LIST += msm7627a
QCRIL_BOARD_PLATFORM_LIST += msm7630_surf
QCRIL_BOARD_PLATFORM_LIST += msm7630_fusion
QCRIL_BOARD_PLATFORM_LIST += msm8660
QCRIL_BOARD_PLATFORM_LIST += msm8960
QCRIL_BOARD_PLATFORM_LIST += msm8974
QCRIL_BOARD_PLATFORM_LIST += msm8226
QCRIL_BOARD_PLATFORM_LIST += msm8610
QCRIL_BOARD_PLATFORM_LIST += apq8084
QCRIL_BOARD_PLATFORM_LIST += msm8916
QCRIL_BOARD_PLATFORM_LIST += msm8994
QCRIL_BOARD_PLATFORM_LIST += msm8909

QCRIL_FUSION_BOARD_PLATFORM_LIST = msm7627_surf
QCRIL_FUSION_BOARD_PLATFORM_LIST += msm7627_6x
QCRIL_FUSION_BOARD_PLATFORM_LIST += msm7627a
QCRIL_FUSION_BOARD_PLATFORM_LIST += msm7630_fusion
QCRIL_FUSION_BOARD_PLATFORM_LIST += msm7630_surf
QCRIL_DIR := $(call my-dir)

ifeq ($(call is-board-platform-in-list,msm7627_6x $(QCRIL_BOARD_PLATFORM_LIST)),true)
  include $(QCRIL_DIR)/qcrilhook_oem/Android.mk
endif

ifeq ($(call is-board-platform-in-list,$(QCRIL_BOARD_PLATFORM_LIST)),true)

  ifdef DSDA_BUILD_SECOND_RIL
    ## Compile a 2nd library for some DSDA targets
    QCRIL_DSDA_INSTANCE=2
    include $(QCRIL_DIR)/qcril_qmi/qcril_qmi.mk

    QCRIL_DSDA_INSTANCE=1
  endif

  include $(QCRIL_DIR)/qcril_qmi/qcril_qmi.mk

endif

ifeq ($(call is-board-platform-in-list,$(QCRIL_FUSION_BOARD_PLATFORM_LIST)),true)
  include $(QCRIL_DIR)/wmsts/Android.mk
  include $(QC_PROP_ROOT)/common/build/remote_api_makefiles/remote_api_defines.mk
  include $(QCRIL_DIR)/qcril_fusion/qcril_fusion.mk
endif
