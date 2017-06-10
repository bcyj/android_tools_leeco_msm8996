LOCAL_PATH := $(call my-dir)

ifeq ($(call is-board-platform,msm8660),true)
    MODEM_APIS_DIR := msm8660_surf
else
    MODEM_APIS_DIR := $(TARGET_BOARD_PLATFORM)
endif

REMOTE_APIS_PATH := ../../../modem-apis/$(MODEM_APIS_DIR)/api/libs/remote_apis
TARGET_REMOTE_APIS_PATH := $(LOCAL_PATH)/$(REMOTE_APIS_PATH)

define rpcgen_apis_variable

ifneq ($(filter commondefs,$2),)
RPCGEN_APIS_PATH := $(TARGET_OUT_INTERMEDIATES)/$1/lib$2_intermediates
RPCGEN_APIS_PATH_FL := ../../../../../../$(TARGET_OUT_INTERMEDIATES)/$1/lib$2_intermediates
else
RPCGEN_APIS_PATH := $(TARGET_OUT_INTERMEDIATES)/$1/lib$2_rpcgen_intermediates
RPCGEN_APIS_PATH_FL := ../../../../../../$(TARGET_OUT_INTERMEDIATES)/$1/lib$2_rpcgen_intermediates
endif

endef

RPCGEN := $(LOCAL_PATH)/../../scripts/rpcgen_apis.pl

#$(call library-variables-commondef,library-prefix)
define library-variables-commondefs

include $(CLEAR_VARS)

include vendor/qcom/proprietary/common/build/remote_api_makefiles/target_api_enables.mk
include $(LOCAL_PATH)/rpcgen_api_defines.mk

$(shell rm -rf $(TARGET_RPCGEN_APIS_PATH)/$1/xdr)

LOCAL_C_INCLUDES := $(RPCGEN_APIS_PATH)/inc
LOCAL_C_INCLUDES += hardware/msm7k/librpc
LOCAL_C_INCLUDES += hardware/msm7k/librpc/rpc

LOCAL_GENERATED_SOURCES += $(RPCGEN_APIS_PATH)/src/$2_rpcgen_xdr.c
LOCAL_GENERATED_SOURCES += $(RPCGEN_APIS_PATH)/src/$2_rpcgen_clnt.c

LOCAL_SHARED_LIBRARIES := librpc
LOCAL_COPY_HEADERS_TO := libcommondefs/rpcgen/inc
LOCAL_COPY_HEADERS := $(RPCGEN_APIS_PATH_FL)/inc/commondefs_rpcgen_rpc.h

LOCAL_MODULE := libcommondefs

LOCAL_MODULE_TAGS := optional

LOCAL_PRELINK_MODULE := false

LOCAL_MODULE_OWNER := qcom
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)

$(LOCAL_PATH)/$(RPCGEN_APIS_PATH_FL)/inc/$2_rpcgen_rpc.h : \
$(RPCGEN_APIS_PATH)/xdr/$2.xdr
	mkdir -p $(RPCGEN_APIS_PATH)/inc
	perl $(RPCGEN) -api=$2 -has_cb=$3 -add_copyright=$4 -td=$(RPCGEN_APIS_PATH)/inc \
                       -tf=$2_rpcgen_rpc.h -sd=$(RPCGEN_APIS_PATH)/xdr -sf=$2.xdr

$(RPCGEN_APIS_PATH)/src/$2_rpcgen_xdr.c : $(RPCGEN_APIS_PATH)/xdr/$2.xdr
	mkdir -p $(RPCGEN_APIS_PATH)/src
	perl $(RPCGEN) -api=$2 -has_cb=$3 -add_copyright=$4 -td=$(RPCGEN_APIS_PATH)/src \
                       -tf=$2_rpcgen_xdr.c -sd=$(RPCGEN_APIS_PATH)/xdr -sf=$2.xdr

$(RPCGEN_APIS_PATH)/src/$2_rpcgen_clnt.c : $(RPCGEN_APIS_PATH)/xdr/$2.xdr
	mkdir -p $(RPCGEN_APIS_PATH)/src
	perl $(RPCGEN) -api=$2 -has_cb=$3 -add_copyright=$4 -td=$(RPCGEN_APIS_PATH)/src \
                       -tf=$2_rpcgen_clnt.c -sd=$(RPCGEN_APIS_PATH)/xdr -sf=$2.xdr

$(RPCGEN_APIS_PATH)/xdr/$2.xdr :
	mkdir -p $(RPCGEN_APIS_PATH)/xdr
	perl $(RPCGEN) -api=$2 -has_cb=$3 -add_copyright=$4 -td=$(RPCGEN_APIS_PATH)/xdr \
                       -tf=$2.xdr

endef
#end library-variables-commondef


#$(call library-variables-rpcgen,library-prefix)
define library-variables-rpcgen

include $(CLEAR_VARS)

LOCAL_C_INCLUDES += hardware/msm7k/librpc
LOCAL_C_INCLUDES += $(RPCGEN_APIS_PATH)/../../SHARED_LIBRARIES/libcommondefs_intermediates/inc
LOCAL_C_INCLUDES += $(RPCGEN_APIS_PATH)/inc

# If API has callback
ifeq ($3,1)
LOCAL_GENERATED_SOURCES += $(RPCGEN_APIS_PATH)/src/$2_rpcgen_cb_xdr.c
LOCAL_GENERATED_SOURCES += $(RPCGEN_APIS_PATH)/src/$2_rpcgen_common_xdr.c
LOCAL_GENERATED_SOURCES += $(RPCGEN_APIS_PATH)/src/$2_rpcgen_cb_svc.c
LOCAL_GENERATED_SOURCES += $(RPCGEN_APIS_PATH)/src/$2cb_appinit.c

endif

LOCAL_GENERATED_SOURCES += $(RPCGEN_APIS_PATH)/src/$2_rpcgen_clnt.c
LOCAL_GENERATED_SOURCES += $(RPCGEN_APIS_PATH)/src/$2_rpcgen_xdr.c

LOCAL_SHARED_LIBRARIES := librpc
LOCAL_SHARED_LIBRARIES += libcommondefs

LOCAL_COPY_HEADERS_TO := $1/rpcgen/inc
LOCAL_COPY_HEADERS := $(RPCGEN_APIS_PATH_FL)/inc/$2_rpcgen_rpc.h

# If API has callback
ifeq ($3,1)
LOCAL_COPY_HEADERS += $(RPCGEN_APIS_PATH_FL)/inc/$2_rpcgen_common_rpc.h
LOCAL_COPY_HEADERS += $(RPCGEN_APIS_PATH_FL)/inc/$2_rpcgen_cb_rpc.h
LOCAL_COPY_HEADERS += $(RPCGEN_APIS_PATH_FL)/inc/$2cb_appinit.h

endif

LOCAL_MODULE := lib$1_rpcgen
LOCAL_LDLIBS += -lpthread
LOCAL_PRELINK_MODULE := false
LOCAL_MODULE_OWNER := qcom
include $(BUILD_STATIC_LIBRARY)

$(LOCAL_PATH)/$(RPCGEN_APIS_PATH_FL)/inc/$2_rpcgen_rpc.h : \
$(TARGET_REMOTE_APIS_PATH)/$1/xdr/$2.xdr
	mkdir -p $(RPCGEN_APIS_PATH)/inc
	perl $(RPCGEN) -api=$2 -has_cb=$3 -add_copyright=$4 -td=$(RPCGEN_APIS_PATH)/inc \
                       -tf=$2_rpcgen_rpc.h -sd=$(TARGET_REMOTE_APIS_PATH)/$1/xdr -sf=$2.xdr

$(RPCGEN_APIS_PATH)/src/$2_rpcgen_xdr.c : $(TARGET_REMOTE_APIS_PATH)/$1/xdr/$2.xdr
	mkdir -p $(RPCGEN_APIS_PATH)/src
	perl $(RPCGEN) -api=$2 -has_cb=$3 -add_copyright=$4 -td=$(RPCGEN_APIS_PATH)/src \
                       -tf=$2_rpcgen_xdr.c -sd=$(TARGET_REMOTE_APIS_PATH)/$1/xdr -sf=$2.xdr

$(RPCGEN_APIS_PATH)/src/$2_rpcgen_clnt.c : $(TARGET_REMOTE_APIS_PATH)/$1/xdr/$2.xdr
	mkdir -p $(RPCGEN_APIS_PATH)/src
	perl $(RPCGEN) -api=$2 -has_cb=$3 -add_copyright=$4 -td=$(RPCGEN_APIS_PATH)/src \
                       -tf=$2_rpcgen_clnt.c -sd=$(TARGET_REMOTE_APIS_PATH)/$1/xdr -sf=$2.xdr

# If API has callback
ifeq ($3,1)
$(LOCAL_PATH)/$(RPCGEN_APIS_PATH_FL)/inc/$2_rpcgen_cb_rpc.h : \
$(TARGET_REMOTE_APIS_PATH)/$1/xdr/$2_cb.xdr
	mkdir -p $(RPCGEN_APIS_PATH)/inc
	perl $(RPCGEN) -api=$2 -cb -add_copyright=$4 -td=$(RPCGEN_APIS_PATH)/inc \
                       -tf=$2_rpcgen_cb_rpc.h -sd=$(TARGET_REMOTE_APIS_PATH)/$1/xdr -sf=$2_cb.xdr

$(LOCAL_PATH)/$(RPCGEN_APIS_PATH_FL)/inc/$2cb_appinit.h : \
$(TARGET_REMOTE_APIS_PATH)/$1/xdr/$2_cb.xdr
	mkdir -p $(RPCGEN_APIS_PATH)/inc
	perl $(RPCGEN) -api=$2 -cb -add_copyright=$4 -td=$(RPCGEN_APIS_PATH)/inc \
                       -tf=$2cb_appinit.h -sd=$(TARGET_REMOTE_APIS_PATH)/$1/xdr -sf=$2_cb.xdr

$(RPCGEN_APIS_PATH)/src/$2_rpcgen_cb_xdr.c : $(TARGET_REMOTE_APIS_PATH)/$1/xdr/$2_cb.xdr
	mkdir -p $(RPCGEN_APIS_PATH)/src
	perl $(RPCGEN) -api=$2 -cb -add_copyright=$4 -td=$(RPCGEN_APIS_PATH)/src \
                       -tf=$2_rpcgen_cb_xdr.c -sd=$(TARGET_REMOTE_APIS_PATH)/$1/xdr -sf=$2_cb.xdr

$(RPCGEN_APIS_PATH)/src/$2_rpcgen_cb_svc.c : $(TARGET_REMOTE_APIS_PATH)/$1/xdr/$2_cb.xdr
	mkdir -p $(RPCGEN_APIS_PATH)/src
	perl $(RPCGEN) -api=$2 -cb -add_copyright=$4 -td=$(RPCGEN_APIS_PATH)/src \
                       -tf=$2_rpcgen_cb_svc.c -sd=$(TARGET_REMOTE_APIS_PATH)/$1/xdr -sf=$2_cb.xdr

$(RPCGEN_APIS_PATH)/src/$2cb_appinit.c : $(TARGET_REMOTE_APIS_PATH)/$1/xdr/$2_cb.xdr
	mkdir -p $(RPCGEN_APIS_PATH)/src
	perl $(RPCGEN) -api=$2 -cb -add_copyright=$4 -td=$(RPCGEN_APIS_PATH)/src \
                       -tf=$2cb_appinit.c -sd=$(TARGET_REMOTE_APIS_PATH)/$1/xdr -sf=$2_cb.xdr

$(LOCAL_PATH)/$(RPCGEN_APIS_PATH_FL)/inc/$2_rpcgen_common_rpc.h : \
$(TARGET_REMOTE_APIS_PATH)/$1/xdr/$2_common.xdr
	mkdir -p $(RPCGEN_APIS_PATH)/inc
	perl $(RPCGEN) -api=$2 -cb -add_copyright=$4 -td=$(RPCGEN_APIS_PATH)/inc \
                       -tf=$2_rpcgen_common_rpc.h -sd=$(TARGET_REMOTE_APIS_PATH)/$1/xdr -sf=$2_common.xdr

$(RPCGEN_APIS_PATH)/src/$2_rpcgen_common_xdr.c : $(TARGET_REMOTE_APIS_PATH)/$1/xdr/$2_common.xdr
	mkdir -p $(RPCGEN_APIS_PATH)/src
	perl $(RPCGEN) -api=$2 -cb -add_copyright=$4 -td=$(RPCGEN_APIS_PATH)/src \
                       -tf=$2_rpcgen_common_xdr.c -sd=$(TARGET_REMOTE_APIS_PATH)/$1/xdr -sf=$2_common.xdr

endif

endef
#end library-variables-rpcgen

# Compiling commondefs
$(eval $(call rpcgen_apis_variable,SHARED_LIBRARIES,commondefs))
$(eval $(call library-variables-commondefs,commondefs,commondefs,0,1))

# Compiling rpcgen Remote Apis
# For the following calls, specify
# <api> <api_directory_name> <has callback 0/1>

ifeq ($(ADC_RPCGEN_ENABLE),1)
$(eval $(call rpcgen_apis_variable,STATIC_LIBRARIES,adc))
$(eval $(call library-variables-rpcgen,adc,adc,0,1))
endif

ifeq ($(AUTH_RPCGEN_ENABLE),1)
$(eval $(call rpcgen_apis_variable,STATIC_LIBRARIES,auth))
$(eval $(call library-variables-rpcgen,auth,auth,0,1))
endif

ifeq ($(CM_RPCGEN_ENABLE),1)
$(eval $(call rpcgen_apis_variable,STATIC_LIBRARIES,cm))
$(eval $(call library-variables-rpcgen,cm,cm,1,1))
endif

ifeq ($(DOG_KEEPALIVE_MODEM_RPCGEN_ENABLE),1)
$(eval $(call rpcgen_apis_variable,STATIC_LIBRARIES,dog_keepalive_modem))
$(eval $(call library-variables-rpcgen,dog_keepalive_modem,dog_keepalive_modem,0,1))
endif

ifeq ($(DSUCSDAPPIF_APIS_RPCGEN_ENABLE),1)
$(eval $(call rpcgen_apis_variable,STATIC_LIBRARIES,dsucsd))
$(eval $(call library-variables-rpcgen,dsucsd,dsucsd,0,1))
endif

ifeq ($(FM_WAN_API_RPCGEN_ENABLE),1)
$(eval $(call rpcgen_apis_variable,STATIC_LIBRARIES,fm_wan_api))
$(eval $(call library-variables-rpcgen,fm_wan_api,fm_wan_api,0,1))
endif

ifeq ($(GPSONE_BIT_API_RPCGEN_ENABLE),1)
$(eval $(call rpcgen_apis_variable,STATIC_LIBRARIES,gpsone_bit_api))
$(eval $(call library-variables-rpcgen,gpsone_bit_api,gpsone_bit_api,1,1))
endif

ifeq ($(GSDI_EXP_RPCGEN_ENABLE),1)
$(eval $(call rpcgen_apis_variable,STATIC_LIBRARIES,gsdi_exp))
$(eval $(call library-variables-rpcgen,gsdi_exp,gsdi_exp,1,1))
endif

ifeq ($(GSTK_EXP_RPCGEN_ENABLE),1)
$(eval $(call rpcgen_apis_variable,STATIC_LIBRARIES,gstk_exp))
$(eval $(call library-variables-rpcgen,gstk_exp,gstk_exp,1,1))
endif

ifeq ($(ISENSE_RPCGEN_ENABLE),1)
$(eval $(call rpcgen_apis_variable,STATIC_LIBRARIES,isense))
$(eval $(call library-variables-rpcgen,isense,isense,1,1))
endif

ifeq ($(LOC_API_RPCGEN_ENABLE),1)
$(eval $(call rpcgen_apis_variable,STATIC_LIBRARIES,loc_api))
$(eval $(call library-variables-rpcgen,loc_api,loc_api,1,0))
endif

ifeq ($(MMGSDILIB_RPCGEN_ENABLE),1)
$(eval $(call rpcgen_apis_variable,STATIC_LIBRARIES,mmgsdilib))
$(eval $(call library-variables-rpcgen,mmgsdilib,mmgsdilib,1,1))
endif

ifeq ($(MMGSDISESSIONLIB_RPCGEN_ENABLE),1)
$(eval $(call rpcgen_apis_variable,STATIC_LIBRARIES,mmgsdisessionlib))
$(eval $(call library-variables-rpcgen,mmgsdisessionlib,mmgsdisessionlib,0,1))
endif

ifeq ($(MVS_RPCGEN_ENABLE),1)
$(eval $(call rpcgen_apis_variable,STATIC_LIBRARIES,mvs))
$(eval $(call library-variables-rpcgen,mvs,mvs,1,1))
endif

ifeq ($(NV_RPCGEN_ENABLE),1)
$(eval $(call rpcgen_apis_variable,STATIC_LIBRARIES,nv))
$(eval $(call library-variables-rpcgen,nv,nv,0,1))
endif

ifeq ($(OEM_RAPI_RPCGEN_ENABLE),1)
$(eval $(call rpcgen_apis_variable,STATIC_LIBRARIES,oem_rapi))
$(eval $(call library-variables-rpcgen,oem_rapi,oem_rapi,1,1))
endif

ifeq ($(PBMLIB_RPCGEN_ENABLE),1)
$(eval $(call rpcgen_apis_variable,STATIC_LIBRARIES,pbmlib))
$(eval $(call library-variables-rpcgen,pbmlib,pbmlib,1,1))
endif

ifeq ($(PDAPI_RPCGEN_ENABLE),1)
$(eval $(call rpcgen_apis_variable,STATIC_LIBRARIES,pdapi))
$(eval $(call library-variables-rpcgen,pdapi,pdapi,1,1))
endif

ifeq ($(PDSM_ATL_RPCGEN_ENABLE),1)
$(eval $(call rpcgen_apis_variable,STATIC_LIBRARIES,pdsm_atl))
$(eval $(call library-variables-rpcgen,pdsm_atl,pdsm_atl,1,1))
endif

ifeq ($(PING_MDM_RPC_RPCGEN_ENABLE),1)
$(eval $(call rpcgen_apis_variable,STATIC_LIBRARIES,ping_mdm))
$(eval $(call library-variables-rpcgen,ping_mdm,ping_mdm_rpc,1,1))
endif

ifeq ($(PLAYREADY_RPCGEN_ENABLE),1)
$(eval $(call rpcgen_apis_variable,STATIC_LIBRARIES,playready))
$(eval $(call library-variables-rpcgen,playready,playready,0,1))
endif

ifeq ($(QCHAT_RPCGEN_ENABLE),1)
$(eval $(call rpcgen_apis_variable,STATIC_LIBRARIES,qchat))
$(eval $(call library-variables-rpcgen,qchat,qchat,0,1))
endif

ifeq ($(REMOTEFS_RPCGEN_ENABLE),1)
$(eval $(call rpcgen_apis_variable,STATIC_LIBRARIES,remotefs))
$(eval $(call library-variables-rpcgen,remotefs,remotefs,0,1))
endif

ifeq ($(RFM_SAR_RPCGEN_ENABLE),1)
$(eval $(call rpcgen_apis_variable,STATIC_LIBRARIES,rfm_sar))
$(eval $(call library-variables-rpcgen,rfm_sar,rfm_sar,0,1))
endif

ifeq ($(SECUTIL_RPCGEN_ENABLE),1)
$(eval $(call rpcgen_apis_variable,STATIC_LIBRARIES,secutil))
$(eval $(call library-variables-rpcgen,secutil,secutil,0,1))
endif

ifeq ($(SND_RPCGEN_ENABLE),1)
$(eval $(call rpcgen_apis_variable,STATIC_LIBRARIES,snd))
$(eval $(call library-variables-rpcgen,snd,snd,1,1))
endif

ifeq ($(TEST_API_RPCGEN_ENABLE),1)
$(eval $(call rpcgen_apis_variable,STATIC_LIBRARIES,test_api))
$(eval $(call library-variables-rpcgen,test_api,test_api,0,1))
endif

ifeq ($(THERMAL_MITIGATION_RPCGEN_ENABLE),1)
$(eval $(call rpcgen_apis_variable,STATIC_LIBRARIES,thermal_mitigation))
$(eval $(call library-variables-rpcgen,thermal_mitigation,thermal_mitigation,0,1))
endif

ifeq ($(TIME_REMOTE_ATOM_RPCGEN_ENABLE),1)
$(eval $(call rpcgen_apis_variable,STATIC_LIBRARIES,time_remote_atom))
$(eval $(call library-variables-rpcgen,time_remote_atom,time_remote_atom,0,1))
endif

ifeq ($(UIM_RPCGEN_ENABLE),1)
$(eval $(call rpcgen_apis_variable,STATIC_LIBRARIES,uim))
$(eval $(call library-variables-rpcgen,uim,uim,1,1))
endif

ifeq ($(VOEM_IF_RPCGEN_ENABLE),1)
$(eval $(call rpcgen_apis_variable,STATIC_LIBRARIES,voem_if))
$(eval $(call library-variables-rpcgen,voem_if,voem_if,0,1))
endif

ifeq ($(WIDEVINE_RPCGEN_ENABLE),1)
$(eval $(call rpcgen_apis_variable,STATIC_LIBRARIES,widevine))
$(eval $(call library-variables-rpcgen,widevine,widevine,0,1))
endif

ifeq ($(WMS_RPCGEN_ENABLE),1)
$(eval $(call rpcgen_apis_variable,STATIC_LIBRARIES,wms))
$(eval $(call library-variables-rpcgen,wms,wms,1,1))
endif

