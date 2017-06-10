include vendor/qcom/proprietary/common/build/remote_api_makefiles/target_api_enables.mk

ifeq ($(AUTH_FUSION_ENABLE),1)
    defines_api_enable += -DFEATURE_EXPORT_AUTH_FUSION
endif

ifeq ($(CM_FUSION_ENABLE),1)
    defines_api_enable += -DFEATURE_EXPORT_CM_FUSION
endif

ifeq ($(DSUCSDAPPIF_APIS_FUSION_ENABLE),1)
    defines_api_enable += -DFEATURE_EXPORT_DSUCSDAPPIF_APIS_FUSION
endif

ifeq ($(OEM_RAPI_FUSION_ENABLE),1)
    defines_api_enable += -DFEATURE_EXPORT_OEM_RAPI_FUSION
endif

ifeq ($(PBMLIB_FUSION_ENABLE),1)
    defines_api_enable += -DFEATURE_EXPORT_PBMLIB_FUSION
endif

ifeq ($(PING_LTE_RPC_ENABLE),1)
    defines_api_enable += -DFEATURE_EXPORT_PING_LTE_RPC
endif

ifeq ($(TEST_API_FUSION_ENABLE),1)
    defines_api_enable += -DFEATURE_EXPORT_TEST_API_FUSION
endif

ifeq ($(THERMAL_MITIGATION_FUSION_ENABLE),1)
    defines_api_enable += -DFEATURE_EXPORT_THERMAL_MITIGATION_FUSION
endif

ifeq ($(TIME_REMOTE_ATOM_FUSION_ENABLE),1)
    defines_api_enable += -DFEATURE_EXPORT_TIME_REMOTE_ATOM_FUSION
endif

ifeq ($(WMS_FUSION_ENABLE),1)
    defines_api_enable += -DFEATURE_EXPORT_WMS_FUSION
endif

defines_api_features += -DAUTH_A_KEY_DIGITS=26
defines_api_features += -DREX_TASK_NAME_LEN=127
defines_api_features += -DFEATURE_WCDMA
defines_api_features += -DFEATURE_GSM

fusion_api_defines := $(defines_api_enable)
fusion_api_defines += $(defines_api_features)
