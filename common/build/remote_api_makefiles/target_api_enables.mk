ifeq ($(call is-board-platform,msm8660),true)
    MODEM_APIS_DIR := msm8660_surf
else
    MODEM_APIS_DIR := $(TARGET_BOARD_PLATFORM)
endif

-include vendor/qcom/proprietary/modem-apis/$(MODEM_APIS_DIR)/remote_api_enables.mk

ifeq ($(call is-board-platform,msm7630_fusion),true)
    FUSION_APIS_DIR :=  mdm9600_fusion
    #Include Fusion Target if applicable
   -include vendor/qcom/proprietary/modem-apis/$(FUSION_APIS_DIR)/remote_api_enables.mk
endif

ifeq ($(call is-board-platform,msm8660),true)
    FUSION_APIS_DIR :=  msm8660_csfb_mdm9600
    #Include Fusion Target if applicable
   -include vendor/qcom/proprietary/modem-apis/$(FUSION_APIS_DIR)/remote_api_enables.mk
endif

