ifeq ($(call is-vendor-board-platform,QCOM),true)

ifneq ($(strip $(TARGET_NO_RPC)),true)

   include $(QC_PROP_ROOT)/common/build/remote_api_makefiles/remote_apis.mk

   ifneq ($(BUILD_TINY_ANDROID),true)

      include $(QC_PROP_ROOT)/common/build/remote_api_makefiles/rpcgen_apis.mk

      ifneq ($(call is-board-platform-in-list,msm7625_surf msm7625_ffa),true)
         include $(QC_PROP_ROOT)/common/build/remote_api_makefiles/rpcgen_api_verify.mk
      endif

   endif #!BUILD_TINY_ANDROID

endif

endif
