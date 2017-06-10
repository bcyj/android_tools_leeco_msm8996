#Fusion Target Support
ifeq ($(call is-board-platform,msm7630_fusion),true)

#   FUSION_APIS_DIR :=  mdm9600_fusion

#   include $(QC_PROP_ROOT)/common/build/fusion_api_makefiles/fusion_apis.mk

endif
# Fusion Target support for 8660 variants
ifeq ($(call is-board-platform,msm8660),true)

   FUSION_APIS_DIR := msm8660_csfb_mdm9600

   include $(QC_PROP_ROOT)/common/build/fusion_api_makefiles/fusion_apis.mk

endif
