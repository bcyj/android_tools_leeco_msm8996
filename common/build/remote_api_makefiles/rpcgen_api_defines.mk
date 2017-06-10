#api enable defines
include vendor/qcom/proprietary/common/build/remote_api_makefiles/target_api_enables.mk

ifeq ($(ADC_RPCGEN_ENABLE),1)
defines_api_enable += -DFEATURE_EXPORT_ADC_RPCGEN
endif

ifeq ($(AUTH_RPCGEN_ENABLE),1)
defines_api_enable += -DFEATURE_EXPORT_AUTH_RPCGEN
endif

ifeq ($(CM_RPCGEN_ENABLE),1)
defines_api_enable += -DFEATURE_EXPORT_CM_RPCGEN
endif

ifeq ($(DOG_KEEPALIVE_MODEM_RPCGEN_ENABLE),1)
defines_api_enable += -DFEATURE_EXPORT_DOG_KEEPALIVE_MODEM_RPCGEN
endif

ifeq ($(DSUCSDAPPIF_APIS_RPCGEN_ENABLE),1)
defines_api_enable += -DFEATURE_EXPORT_DSUCSDAPPIF_APIS_RPCGEN
endif

ifeq ($(FM_WAN_API_RPCGEN_ENABLE),1)
defines_api_enable += -DFEATURE_EXPORT_FM_WAN_API_RPCGEN
endif

ifeq ($(GPSONE_BIT_API_RPCGEN_ENABLE),1)
defines_api_enable += -DFEATURE_EXPORT_GPSONE_BIT_API_RPCGEN
endif

ifeq ($(GSDI_EXP_RPCGEN_ENABLE),1)
defines_api_enable += -DFEATURE_EXPORT_GSDI_EXP_RPCGEN
endif

ifeq ($(GSTK_EXP_RPCGEN_ENABLE),1)
defines_api_enable += -DFEATURE_EXPORT_GSTK_EXP_RPCGEN
endif

ifeq ($(ISENSE_RPCGEN_ENABLE),1)
defines_api_enable += -DFEATURE_EXPORT_ISENSE_RPCGEN
endif

ifeq ($(MMGSDILIB_RPCGEN_ENABLE),1)
defines_api_enable += -DFEATURE_EXPORT_MMGSDILIB_RPCGEN
endif

ifeq ($(MMGSDISESSIONLIB_RPCGEN_ENABLE),1)
defines_api_enable += -DFEATURE_EXPORT_MMGSDISESSIONLIB_RPCGEN
endif

ifeq ($(MVS_RPCGEN_ENABLE),1)
defines_api_enable += -DFEATURE_EXPORT_MVS_RPCGEN
endif

ifeq ($(NV_RPCGEN_ENABLE),1)
defines_api_enable += -DFEATURE_EXPORT_NV_RPCGEN
endif

ifeq ($(OEM_RAPI_RPCGEN_ENABLE),1)
defines_api_enable += -DFEATURE_EXPORT_OEM_RAPI_RPCGEN
endif

ifeq ($(PBMLIB_RPCGEN_ENABLE),1)
defines_api_enable += -DFEATURE_EXPORT_PBMLIB_RPCGEN
endif

ifeq ($(PDAPI_RPCGEN_ENABLE),1)
defines_api_enable += -DFEATURE_EXPORT_PDAPI_RPCGEN
endif

ifeq ($(PDSM_ATL_RPCGEN_ENABLE),1)
defines_api_enable += -DFEATURE_EXPORT_PDSM_ATL_RPCGEN
endif

ifeq ($(PING_MDM_RPC_RPCGEN_ENABLE),1)
defines_api_enable += -DFEATURE_EXPORT_PING_MDM_RPC_RPCGEN
endif

ifeq ($(PLAYREADY_RPCGEN_ENABLE),1)
defines_api_enable += -DFEATURE_EXPORT_PLAYREADY_RPCGEN
endif

ifeq ($(QCHAT_RPCGEN_ENABLE),1)
defines_api_enable += -DFEATURE_EXPORT_QCHAT_RPCGEN
endif

ifeq ($(REMOTEFS_RPCGEN_ENABLE),1)
defines_api_enable += -DFEATURE_EXPORT_REMOTEFS_RPCGEN
endif

ifeq ($(RFM_SAR_RPCGEN_ENABLE),1)
defines_api_enable += -DFEATURE_EXPORT_RFM_SAR_RPCGEN
endif

ifeq ($(SECUTIL_RPCGEN_ENABLE),1)
defines_api_enable += -DFEATURE_EXPORT_SECUTIL_RPCGEN
endif

ifeq ($(SND_RPCGEN_ENABLE),1)
defines_api_enable += -DFEATURE_EXPORT_SND_RPCGEN
endif

ifeq ($(TEST_API_RPCGEN_ENABLE),1)
defines_api_enable += -DFEATURE_EXPORT_TEST_API_RPCGEN
endif

ifeq ($(THERMAL_MITIGATION_RPCGEN_ENABLE),1)
defines_api_enable += -DFEATURE_EXPORT_THERMAL_MITIGATION_RPCGEN
endif

ifeq ($(TIME_REMOTE_ATOM_RPCGEN_ENABLE),1)
defines_api_enable += -DFEATURE_EXPORT_TIME_REMOTE_ATOM_RPCGEN
endif

ifeq ($(UIM_RPCGEN_ENABLE),1)
defines_api_enable += -DFEATURE_EXPORT_UIM_RPCGEN
endif

ifeq ($(VOEM_IF_RPCGEN_ENABLE),1)
defines_api_enable += -DFEATURE_EXPORT_VOEM_IF_RPCGEN
endif

ifeq ($(WIDEVINE_RPCGEN_ENABLE),1)
defines_api_enable += -DFEATURE_EXPORT_WIDEVINE_RPCGEN
endif

ifeq ($(WMS_RPCGEN_ENABLE),1)
defines_api_enable += -DFEATURE_EXPORT_WMS_RPCGEN
endif

remote_api_defines := $(defines_api_enable)
