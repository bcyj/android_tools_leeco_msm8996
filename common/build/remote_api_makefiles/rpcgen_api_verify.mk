LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

include vendor/qcom/proprietary/common/build/remote_api_makefiles/target_api_enables.mk
include $(LOCAL_PATH)/rpcgen_api_defines.mk

ifeq ($(call is-board-platform,msm8660),true)
    MODEM_APIS_DIR := msm8660_surf
else
    MODEM_APIS_DIR := $(TARGET_BOARD_PLATFORM)
endif

API_SRCDIR := vendor/qcom/proprietary/modem-apis/$(MODEM_APIS_DIR)/api
TARGET_MODEM_APIS_PATH := $(LOCAL_PATH)/../../../modem-apis/$(MODEM_APIS_DIR)

RPCGEN_APIS_PATH := $(TARGET_OUT_INTERMEDIATES)/EXECUTABLES/rpcgen_apis_verify_intermediates
TARGET_REMOTE_APIS_PATH := $(API_SRCDIR)/libs/remote_apis
RPCGEN := $(LOCAL_PATH)/../../scripts/rpcgen_apis.pl

#-----------------------------------------------------------------------------
# Include path for all APIS supported in this test
#-----------------------------------------------------------------------------
LOCAL_C_INCLUDES := $(TARGET_OUT_INTERMEDIATES)/SHARED_LIBRARIES/libcommondefs_intermediates/inc

LOCAL_CFLAGS := $(defines_api_enable)
LOCAL_CFLAGS += -I hardware/msm7k/librpc/rpc
LOCAL_CFLAGS += -I hardware/msm7k/librpc
LOCAL_CFLAGS += -I $(RPCGEN_APIS_PATH)/src

LOCAL_SHARED_LIBRARIES := librpc libcommondefs

LOCAL_GENERATED_SOURCES := $(RPCGEN_APIS_PATH)/src/rpcgen_apis_verify.c

$(RPCGEN_APIS_PATH)/src/rpcgen_apis_verify.c : \
$(TARGET_MODEM_APIS_PATH)/remote_api_enables.mk $(API_SRCDIR)/ApiReport.txt
	mkdir -p $(RPCGEN_APIS_PATH)/src/
	perl $(RPCGEN) -td=$(RPCGEN_APIS_PATH)/src -tf=rpcgen_apis_verify.c -sd=$(TARGET_MODEM_APIS_PATH)

#function
#$(call rpcgen-api-verify-variable,api-dir,api-src,hascallback)
define rpcgen-api-verify-variable

LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/STATIC_LIBRARIES/lib$1_rpcgen_intermediates/inc

ifeq ($(strip $(3)),1)
LOCAL_GENERATED_SOURCES += $(RPCGEN_APIS_PATH)/app/$(strip $(2))_cb_sample.c

$(RPCGEN_APIS_PATH)/app/$(strip $(2))_cb_sample.c : \
$(TARGET_REMOTE_APIS_PATH)/$(strip $(1))/xdr/$(strip $(2))_cb.xdr
	mkdir -p $(RPCGEN_APIS_PATH)/app
	perl $(RPCGEN) -api=$2 -cb -td=$(RPCGEN_APIS_PATH)/app -tf=$2_cb_sample.c \
	               -sd=$(TARGET_REMOTE_APIS_PATH)/$1/xdr -sf=$2_cb.xdr

endif

LOCAL_STATIC_LIBRARIES += lib$1_rpcgen

endef
#end rpcgen-api-verify-variable

ifeq ($(ADC_RPCGEN_ENABLE),1)
$(eval $(call rpcgen-api-verify-variable,adc,adc,0))
endif

ifeq ($(AUTH_RPCGEN_ENABLE),1)
$(eval $(call rpcgen-api-verify-variable,auth,auth,0))
endif

ifeq ($(CM_RPCGEN_ENABLE),1)
$(eval $(call rpcgen-api-verify-variable,cm,cm,1))
endif

ifeq ($(DOG_KEEPALIVE_MODEM_RPCGEN_ENABLE),1)
$(eval $(call rpcgen-api-verify-variable,dog_keepalive_modem,dog_keepalive_modem,0))
endif

ifeq ($(DSUCSDAPPIF_APIS_RPCGEN_ENABLE),1)
$(eval $(call rpcgen-api-verify-variable,dsucsd,dsucsdappif_apis,1))
endif

ifeq ($(FM_WAN_API_RPCGEN_ENABLE),1)
$(eval $(call rpcgen-api-verify-variable,fm_wan_api,fm_wan_api,0))
endif

ifeq ($(GPSONE_BIT_API_RPCGEN_ENABLE),1)
$(eval $(call rpcgen-api-verify-variable,gpsone_bit_api,gpsone_bit_api,1))
endif

ifeq ($(GSDI_EXP_RPCGEN_ENABLE),1)
$(eval $(call rpcgen-api-verify-variable,gsdi_exp,gsdi_exp,1))
endif

ifeq ($(GSTK_EXP_RPCGEN_ENABLE),1)
$(eval $(call rpcgen-api-verify-variable,gstk_exp,gstk_exp,1))
endif

ifeq ($(ISENSE_RPCGEN_ENABLE),1)
$(eval $(call rpcgen-api-verify-variable,isense,isense,1))
endif

ifeq ($(LOC_API_RPCGEN_ENABLE),1)
$(eval $(call rpcgen-api-verify-variable,loc_api,loc_api,1))
endif

ifeq ($(MMGSDILIB_RPCGEN_ENABLE),1)
$(eval $(call rpcgen-api-verify-variable,mmgsdilib,mmgsdilib,1))
endif

ifeq ($(MMGSDISESSIONLIB_RPCGEN_ENABLE),1)
$(eval $(call rpcgen-api-verify-variable,mmgsdisessionlib,mmgsdisessionlib,0))
endif

ifeq ($(MVS_RPCGEN_ENABLE),1)
$(eval $(call rpcgen-api-verify-variable,mvs,mvs,1))
endif

ifeq ($(NV_RPCGEN_ENABLE),1)
$(eval $(call rpcgen-api-verify-variable,nv,nv,0))
endif

ifeq ($(OEM_RAPI_RPCGEN_ENABLE),1)
$(eval $(call rpcgen-api-verify-variable,oem_rapi,oem_rapi,1))
endif

ifeq ($(PBMLIB_RPCGEN_ENABLE),1)
$(eval $(call rpcgen-api-verify-variable,pbmlib,pbmlib,1))
endif

ifeq ($(PDAPI_RPCGEN_ENABLE),1)
$(eval $(call rpcgen-api-verify-variable,pdapi,pdapi,1))
endif

ifeq ($(PDSM_ATL_RPCGEN_ENABLE),1)
$(eval $(call rpcgen-api-verify-variable,pdsm_atl,pdsm_atl,1))
endif

ifeq ($(PING_MDM_RPC_RPCGEN_ENABLE),1)
$(eval $(call rpcgen-api-verify-variable,ping_mdm,ping_mdm_rpc,1))
endif

ifeq ($(PLAYREADY_RPCGEN_ENABLE),1)
$(eval $(call rpcgen-api-verify-variable,playready,playready,0))
endif

ifeq ($(QCHAT_RPCGEN_ENABLE),1)
$(eval $(call rpcgen-api-verify-variable,qchat,qchat,0))
endif

ifeq ($(REMOTEFS_RPCGEN_ENABLE),1)
$(eval $(call rpcgen-api-verify-variable,remotefs,remotefs,0))
endif

ifeq ($(RFM_SAR_RPCGEN_ENABLE),1)
$(eval $(call rpcgen-api-verify-variable,rfm_sar,rfm_sar,0))
endif

ifeq ($(SECUTIL_RPCGEN_ENABLE),1)
$(eval $(call rpcgen-api-verify-variable,secutil,secutil,0))
endif

ifeq ($(SND_RPCGEN_ENABLE),1)
$(eval $(call rpcgen-api-verify-variable,snd,snd,1))
endif

ifeq ($(TEST_API_RPCGEN_ENABLE),1)
$(eval $(call rpcgen-api-verify-variable,test_api,test_api,0))
endif

ifeq ($(THERMAL_MITIGATION_RPCGEN_ENABLE),1)
$(eval $(call rpcgen-api-verify-variable,thermal_mitigation,thermal_mitigation,0))
endif

ifeq ($(TIME_REMOTE_ATOM_RPCGEN_ENABLE),1)
$(eval $(call rpcgen-api-verify-variable,time_remote_atom,time_remote_atom,0))
endif

ifeq ($(UIM_RPCGEN_ENABLE),1)
$(eval $(call rpcgen-api-verify-variable,uim,uim,0))
endif

ifeq ($(VOEM_IF_RPCGEN_ENABLE),1)
$(eval $(call rpcgen-api-verify-variable,voem_if,voem_if,0))
endif

ifeq ($(WIDEVINE_RPCGEN_ENABLE),1)
$(eval $(call rpcgen-api-verify-variable,widevine,widevine,0))
endif

ifeq ($(WMS_RPCGEN_ENABLE),1)
$(eval $(call rpcgen-api-verify-variable,wms,wms,1))
endif

LOCAL_MODULE_TAGS := optional

LOCAL_MODULE_PATH := $(TARGET_OUT_DATA)/remote-api-tests/librpc

LOCAL_PRELINK_MODULE := false

LOCAL_MODULE := rpcgen_apis_verify

LOCAL_MODULE_OWNER := qcom

include $(BUILD_EXECUTABLE)
