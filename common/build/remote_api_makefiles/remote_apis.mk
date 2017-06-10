LOCAL_PATH := $(call my-dir)

ifeq ($(call is-board-platform,msm8660),true)
  MODEM_APIS_DIR := msm8660_surf
else
  MODEM_APIS_DIR := $(TARGET_BOARD_PLATFORM)
endif

REMOTE_APIS_PATH := ../../../modem-apis/$(MODEM_APIS_DIR)/api/libs/remote_apis
ONCRPC_PATH := vendor/qcom/proprietary/oncrpc

include vendor/qcom/proprietary/common/build/remote_api_makefiles/target_api_enables.mk
include $(LOCAL_PATH)/remote_api_defines.mk
include vendor/qcom/proprietary/oncrpc/oncrpc_defines.mk

# functions
#$(call library-variables,library-prefix)
define library-variables

    include $(CLEAR_VARS)

    LOCAL_CFLAGS := $(oncrpc_defines)
    LOCAL_CFLAGS += $(oncrpc_common_defines)
    LOCAL_CFLAGS += $(defines_api_enable)
    LOCAL_CFLAGS += $(defines_api_features)

    LOCAL_C_INCLUDES := $(REMOTE_APIS_PATH)/$(strip $(1))/src
    LOCAL_C_INCLUDES += $(LOCAL_PATH)/$(REMOTE_APIS_PATH)/$(strip $(1))/inc
    LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/oncrpc/inc
    LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/common/inc
    LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/diag/include

    LOCAL_SRC_FILES := $(REMOTE_APIS_PATH)/$1/src/$2_clnt.c
    LOCAL_SRC_FILES += $(REMOTE_APIS_PATH)/$1/src/$2_xdr.c

    LOCAL_SHARED_LIBRARIES := liboncrpc
    LOCAL_SHARED_LIBRARIES += libdiag

    LOCAL_COPY_HEADERS_TO := $1/inc
    LOCAL_COPY_HEADERS := $(REMOTE_APIS_PATH)/$1/inc/$2.h
    LOCAL_COPY_HEADERS += $(REMOTE_APIS_PATH)/$1/inc/$2_rpc.h

    LOCAL_MODULE := lib$1

    LOCAL_MODULE_TAGS := optional

    LOCAL_LDLIBS += -lpthread

    LOCAL_PRELINK_MODULE := false

    LOCAL_MODULE_OWNER := qcom
    LOCAL_PROPRIETARY_MODULE := true

    include $(BUILD_SHARED_LIBRARY)

endef

#end library-variable

ifeq ($(ADC_ENABLE),1)
$(eval $(call library-variables,adc,adc))
endif

ifeq ($(AUTH_ENABLE),1)
$(eval $(call library-variables,auth,auth))
endif

ifeq ($(CM_ENABLE),1)
$(eval $(call library-variables,cm,cm))
endif

ifeq ($(DOG_KEEPALIVE_MODEM_ENABLE),1)
$(eval $(call library-variables,dog_keepalive_modem,dog_keepalive_modem))
endif

ifeq ($(DSUCSDAPPIF_APIS_ENABLE),1)
$(eval $(call library-variables,dsucsd,dsucsdappif_apis))
endif

ifeq ($(FM_WAN_API_ENABLE),1)
$(eval $(call library-variables,fm_wan_api,fm_wan_api))
endif

ifeq ($(GPSONE_BIT_API_ENABLE),1)
$(eval $(call library-variables,gpsone_bit_api,gpsone_bit_api))
endif

ifeq ($(GSDI_EXP_ENABLE),1)
$(eval $(call library-variables,gsdi_exp,gsdi_exp))
endif

ifeq ($(GSTK_EXP_ENABLE),1)
$(eval $(call library-variables,gstk_exp,gstk_exp))
endif

ifeq ($(ISENSE_ENABLE),1)
$(eval $(call library-variables,isense,isense))
endif

ifeq ($(LOC_API_ENABLE),1)
$(eval $(call library-variables,loc_api,loc_api))
endif

ifeq ($(MMGSDILIB_ENABLE),1)
$(eval $(call library-variables,mmgsdilib,mmgsdilib))
endif

ifeq ($(MMGSDISESSIONLIB_ENABLE),1)
$(eval $(call library-variables,mmgsdisessionlib,mmgsdisessionlib))
endif

ifeq ($(MVS_ENABLE),1)
$(eval $(call library-variables,mvs,mvs))
endif

ifeq ($(NV_ENABLE),1)
$(eval $(call library-variables,nv,nv))
endif

ifeq ($(OEM_RAPI_ENABLE),1)
$(eval $(call library-variables,oem_rapi,oem_rapi))
endif

ifeq ($(PBMLIB_ENABLE),1)
$(eval $(call library-variables,pbmlib,pbmlib))
endif

ifeq ($(PDAPI_ENABLE),1)
$(eval $(call library-variables,pdapi,pdapi))
endif

ifeq ($(PDSM_ATL_ENABLE),1)
$(eval $(call library-variables,pdsm_atl,pdsm_atl))
endif

ifeq ($(PING_MDM_RPC_ENABLE),1)
$(eval $(call library-variables,ping_mdm,ping_mdm_rpc))
endif

ifeq ($(PLAYREADY_ENABLE),1)
$(eval $(call library-variables,playready,playready))
endif

ifeq ($(QCHAT_ENABLE),1)
$(eval $(call library-variables,qchat,qchat))
endif

ifeq ($(REMOTEFS_ENABLE),1)
$(eval $(call library-variables,remotefs,remotefs))
endif

ifeq ($(RFM_SAR_ENABLE),1)
$(eval $(call library-variables,rfm_sar,rfm_sar))
endif

ifeq ($(SECUTIL_ENABLE),1)
$(eval $(call library-variables,secutil,secutil))
endif

ifeq ($(SND_ENABLE),1)
$(eval $(call library-variables,snd,snd))
endif

ifeq ($(TEST_API_ENABLE),1)
$(eval $(call library-variables,test_api,test_api))
endif

ifeq ($(THERMAL_MITIGATION_ENABLE),1)
$(eval $(call library-variables,thermal_mitigation,thermal_mitigation))
endif

ifeq ($(TIME_REMOTE_ATOM_ENABLE),1)
$(eval $(call library-variables,time_remote_atom,time_remote_atom))
endif

ifeq ($(UIM_ENABLE),1)
$(eval $(call library-variables,uim,uim))
endif

ifeq ($(VOEM_IF_ENABLE),1)
$(eval $(call library-variables,voem_if,voem_if))
endif

ifeq ($(WIDEVINE_ENABLE),1)
$(eval $(call library-variables,widevine,widevine))
endif

ifeq ($(WMS_ENABLE),1)
$(eval $(call library-variables,wms,wms))
endif
