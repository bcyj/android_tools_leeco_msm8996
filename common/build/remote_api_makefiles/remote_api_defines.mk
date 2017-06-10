include vendor/qcom/proprietary/common/build/remote_api_makefiles/target_api_enables.mk

#api enable defines
ifeq ($(ADC_ENABLE),1)
    defines_api_enable += -DFEATURE_EXPORT_ADC
endif

ifeq ($(AUTH_ENABLE),1)
    defines_api_enable += -DFEATURE_EXPORT_AUTH
endif

ifeq ($(CM_ENABLE),1)
    defines_api_enable += -DFEATURE_EXPORT_CM
endif

ifeq ($(DOG_KEEPALIVE_MODEM_ENABLE),1)
    defines_api_enable += -DFEATURE_EXPORT_DOG_KEEPALIVE_MODEM
endif

ifeq ($(DSUCSDAPPIF_APIS_ENABLE),1)
    defines_api_enable += -DFEATURE_EXPORT_DSUCSDAPPIF_APIS
endif

ifeq ($(FM_WAN_API_ENABLE),1)
    defines_api_enable += -DFEATURE_EXPORT_FM_WAN_API
endif

ifeq ($(GPSONE_BIT_API_ENABLE),1)
    defines_api_enable += -DFEATURE_EXPORT_GPSONE_BIT_API
endif

ifeq ($(GSDI_EXP_ENABLE),1)
    defines_api_enable += -DFEATURE_EXPORT_GSDI_EXP
endif

ifeq ($(GSTK_EXP_ENABLE),1)
    defines_api_enable += -DFEATURE_EXPORT_GSTK_EXP
endif

ifeq ($(ISENSE_ENABLE),1)
    defines_api_enable += -DFEATURE_EXPORT_ISENSE
endif

ifeq ($(MMGSDILIB_ENABLE),1)
    defines_api_enable += -DFEATURE_EXPORT_MMGSDILIB
endif

ifeq ($(MMGSDISESSIONLIB_ENABLE),1)
    defines_api_enable += -DFEATURE_EXPORT_MMGSDISESSIONLIB
endif

ifeq ($(MVS_ENABLE),1)
    defines_api_enable += -DFEATURE_EXPORT_MVS
endif

ifeq ($(NV_ENABLE),1)
    defines_api_enable += -DFEATURE_EXPORT_NV
endif

ifeq ($(OEM_RAPI_ENABLE),1)
    defines_api_enable += -DFEATURE_EXPORT_OEM_RAPI
endif

ifeq ($(PBMLIB_ENABLE),1)
    defines_api_enable += -DFEATURE_EXPORT_PBMLIB
endif

ifeq ($(PDAPI_ENABLE),1)
    defines_api_enable += -DFEATURE_EXPORT_PDAPI
endif

ifeq ($(PDSM_ATL_ENABLE),1)
    defines_api_enable += -DFEATURE_EXPORT_PDSM_ATL
endif

ifeq ($(PING_MDM_RPC_ENABLE),1)
    defines_api_enable += -DFEATURE_EXPORT_PING_MDM_RPC
endif

ifeq ($(PLAYREADY_ENABLE),1)
    defines_api_enable += -DFEATURE_EXPORT_PLAYREADY
endif

ifeq ($(QCHAT_ENABLE),1)
    defines_api_enable += -DFEATURE_EXPORT_QCHAT
endif

ifeq ($(REMOTEFS_ENABLE),1)
    defines_api_enable += -DFEATURE_EXPORT_REMOTEFS
endif

ifeq ($(RFM_SAR_ENABLE),1)
    defines_api_enable += -DFEATURE_EXPORT_RFM_SAR
endif

ifeq ($(SECUTIL_ENABLE),1)
    defines_api_enable += -DFEATURE_EXPORT_SECUTIL
endif

ifeq ($(SND_ENABLE),1)
    defines_api_enable += -DFEATURE_EXPORT_SND
endif

ifeq ($(TEST_API_ENABLE),1)
    defines_api_enable += -DFEATURE_EXPORT_TEST_API
endif

ifeq ($(THERMAL_MITIGATION_ENABLE),1)
    defines_api_enable += -DFEATURE_EXPORT_THERMAL_MITIGATION
endif

ifeq ($(TIME_REMOTE_ATOM_ENABLE),1)
    defines_api_enable += -DFEATURE_EXPORT_TIME_REMOTE_ATOM
endif

ifeq ($(UIM_ENABLE),1)
    defines_api_enable += -DFEATURE_EXPORT_UIM
endif

ifeq ($(VOEM_IF_ENABLE),1)
    defines_api_enable += -DFEATURE_EXPORT_VOEM_IF
endif

ifeq ($(WIDEVINE_ENABLE),1)
    defines_api_enable += -DFEATURE_EXPORT_WIDEVINE
endif

ifeq ($(WMS_ENABLE),1)
    defines_api_enable += -DFEATURE_EXPORT_WMS
endif

defines_api_features := -DAUTH_A_KEY_DIGITS=26
defines_api_features += -DREX_TASK_NAME_LEN=127
#cm api specific defines
ifeq ($(call is-board-platform-in-list,$(QSD8K_BOARD_PLATFORMS)),true)
    defines_api_features += -DFEATURE_MODEM_STATISTICS
endif

ifeq ($(call is-board-platform,msm7627_6x),true)
    defines_api_features += -DFEATURE_MODEM_STATISTICS
endif
defines_api_features += -DFEATURE_MMGPS
defines_api_features += -DFEATURE_PDAPI
defines_api_features += -DFEATURE_CCBS
defines_api_features += -DFEATURE_REL5
defines_api_features += -DFEATURE_TTY
defines_api_features += -DFEATURE_GSM_EXT_SPEECH_PREF_LIST
defines_api_features += -DFEATURE_UUS
defines_api_features += -DFEATURE_MM_SUPERSET
defines_api_features += -DFEATURE_GSM_AMR_WB
#cm target specific
ifeq ($(call is-board-platform-in-list,msm7627_surf msm7627a msm7627_6x),true)
defines_api_features += -DFEATURE_GSM_AMR
defines_api_features += -DFEATURE_GSM_HALF_RATE
endif
ifeq ($(call is-board-platform-in-list,msm7625_surf msm7625_ffa),true)
defines_api_features += -DFEATURE_GSM_AMR
defines_api_features += -DFEATURE_GSM_HALF_RATE
endif
#end cm

#dsucsd api feature defines
defines_api_features += -DFEATURE_DATA
defines_api_features += -DFEATURE_DATA_GCSD
defines_api_features += -DFEATURE_DATA_WCDMA_CS
defines_api_features += -DFEATURE_DATA_UCSD_API_SIOPORT
defines_api_features += -DFEATURE_DATA_UCSD_SCUDIF_API
defines_api_features += -DFEATURE_DATA_UCSD_UNIFIED_API
defines_api_features += -DFEATURE_CM_MMGPS
defines_api_features += -DFEATURE_USB_CDC_ACM
defines_api_features += -DFEATURE_SMD
defines_api_features += -DFEATURE_SMEM_DS
#end dsucsd

#gsdi api specific defines
defines_api_features += -DFEATURE_MMGSDI_UMTS
defines_api_features += -DFEATURE_MMGSDI_NO_TCB_PTR_OR_CRIT_SEC
defines_api_features += -DASSERT
#end gsdi

#mvs api specific defines
defines_api_features += -DFEATURE_MVS
#end mvs

# NV defines (ported from the NV Makefile)
defines_api_features += -DFEATURE_NV_RPC_SUPPORT

#pdapi, pdsm_atl specific defines
defines_api_features += -DFEATURE_PDAPI
#end pdapi, pdsm_atl

#pmapp_gen api specific defines
defines_api_features += -DFEATURE_PMIC_RTC
defines_api_features += -DFEATURE_EXTERNAL_ERR_FATAL
#end pmapp_gen

#pmapp_otg api specific defines
defines_api_features += -DFEATURE_PMIC_USB_OTG
#end pmapp_otg

#snd api specific defines
defines_api_features += -DFEATURE_VOC_PCM_INTERFACE
defines_api_features += -DFEATURE_VOICE_PLAYBACK
defines_api_features += -DFEATURE_VOICE_RECORD
#end snd

#wms api specific defines
defines_api_features += -DFEATURE_GWSMS
defines_api_features += -DFEATURE_SMS_UDH
defines_api_features += -DFEATURE_BROADCAST_SMS_MULTIMODE
#end wms

defines_api_features += -DFEATURE_ADIE_SVC

#begin add_clkregim
ifeq ($(call is-board-platform-in-list,msm7627_surf msm7627a msm7627_6x),true)
defines_api_features += -DT_CLKREGIM=7625
endif
ifeq ($(call is-board-platform-in-list,msm7625_surf msm7625_ffa),true)
defines_api_features += -DT_CLKREGIM=7625
endif

ifeq ($(call is-board-platform-in-list,$(QSD8K_BOARD_PLATFORMS)),true)
defines_api_features += -DT_CLKREGIM=7600
endif
ifeq ($(call is-board-platform,msm7501a_surf),true)
defines_api_features += -DT_CLKREGIM=7600
endif
defines_api_features += -DFEATURE_CLKREGIM_RM
#end add_clkregim

ifeq ($(call is-board-platform-in-list,msm7627_surf msm7627a msm7627_6x),true)
defines_api_features += -DFEATURE_GWSMS
endif
ifeq ($(call is-board-platform-in-list,msm7625_surf msm7625_ffa),true)
defines_api_features += -DFEATURE_GWSMS
endif

defines_api_features += -DFEATURE_WCDMA
defines_api_features += -DFEATURE_GSM

remote_api_defines := $(defines_api_enable)
remote_api_defines += $(defines_api_features)
