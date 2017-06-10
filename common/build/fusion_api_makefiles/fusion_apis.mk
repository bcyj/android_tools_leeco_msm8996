LOCAL_PATH := $(call my-dir)

FUSION_APIS_PATH := ../../../modem-apis/$(FUSION_APIS_DIR)/api/libs/remote_apis
ONCRPC_PATH := vendor/qcom/proprietary/oncrpc

include vendor/qcom/proprietary/common/build/remote_api_makefiles/target_api_enables.mk
include vendor/qcom/proprietary/common/build/remote_api_makefiles/remote_api_defines.mk
include vendor/qcom/proprietary/oncrpc/oncrpc_defines.mk

# functions
#$(call fusion-library-variables,library-prefix)
define fusion-library-variables

    include $(CLEAR_VARS)

    LOCAL_CFLAGS := $(oncrpc_defines)
    LOCAL_CFLAGS += $(oncrpc_common_defines)
    LOCAL_CFLAGS += $(defines_api_enable)
    LOCAL_CFLAGS += $(defines_api_features)

    LOCAL_C_INCLUDES := $(FUSION_APIS_PATH)/$(strip $(1))/src
    LOCAL_C_INCLUDES += $(LOCAL_PATH)/$(FUSION_APIS_PATH)/$(strip $(1))/inc
    LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/oncrpc/inc
    LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/common/inc
    LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/diag/include

    LOCAL_SRC_FILES := $(FUSION_APIS_PATH)/$1/src/$2_clnt.c
    LOCAL_SRC_FILES += $(FUSION_APIS_PATH)/$1/src/$2_xdr.c

    LOCAL_SHARED_LIBRARIES := liboncrpc
    LOCAL_SHARED_LIBRARIES += libdiag

    LOCAL_COPY_HEADERS_TO := $1/inc
    LOCAL_COPY_HEADERS := $(FUSION_APIS_PATH)/$1/inc/$2.h
    LOCAL_COPY_HEADERS += $(FUSION_APIS_PATH)/$1/inc/$2_rpc.h

    LOCAL_MODULE := lib$1

    LOCAL_MODULE_TAGS := optional

    LOCAL_LDLIBS += -lpthread

    LOCAL_PRELINK_MODULE := false

    LOCAL_MODULE_OWNER := qcom
    LOCAL_PROPRIETARY_MODULE := true

    include $(BUILD_SHARED_LIBRARY)

endef

#end library-variable

ifeq ($(AUTH_FUSION_ENABLE),1)
$(eval $(call fusion-library-variables,auth_fusion,auth_fusion))
endif

ifeq ($(CM_FUSION_ENABLE),1)
$(eval $(call fusion-library-variables,cm_fusion,cm_fusion))
endif

ifeq ($(DSUCSDAPPIF_APIS_FUSION_ENABLE),1)
$(eval $(call fusion-library-variables,dsucsdappif_apis_fusion,dsucsdappif_apis_fusion))
endif

ifeq ($(OEM_RAPI_FUSION_ENABLE),1)
$(eval $(call fusion-library-variables,oem_rapi_fusion,oem_rapi_fusion))
endif

ifeq ($(PBMLIB_FUSION_ENABLE),1)
$(eval $(call fusion-library-variables,pbmlib_fusion,pbmlib_fusion))
endif

#ifeq ($(PING_LTE_RPC_ENABLE),1)
$(eval $(call fusion-library-variables,ping_lte_rpc,ping_lte_rpc))
#endif

ifeq ($(TEST_API_FUSION_ENABLE),1)
$(eval $(call fusion-library-variables,test_api_fusion,test_api_fusion))
endif

ifeq ($(THERMAL_MITIGATION_FUSION_ENABLE),1)
$(eval $(call fusion-library-variables,thermal_mitigation_fusion,thermal_mitigation_fusion))
endif

ifeq ($(TIME_REMOTE_ATOM_FUSION_ENABLE),1)
$(eval $(call fusion-library-variables,time_remote_atom_fusion,time_remote_atom_fusion))
endif

ifeq ($(WMS_FUSION_ENABLE),1)
$(eval $(call fusion-library-variables,wms_fusion,wms_fusion))
endif
