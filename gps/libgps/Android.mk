LOCAL_PATH:= $(call my-dir)

# we should not be using this post-Froyo
ifeq (false, )
ifneq ($(call is-board-platform-in-list,msm7627_surf msm7627a msm7627_6x),true)
  # For 7201 and 8x50
  PDAPICBVERS     :=0x95b5cd25
  PDSM_ATLCBVERS  :=0xd5bbf3b9
else
  # For 7627
  PDAPICBVERS     :=0xab221027
  PDSM_ATLCBVERS  :=0xd5bbf3b9
endif

include $(LOCAL_PATH)/pdapi_$(PDAPICBVERS)_pdsm_$(PDSM_ATLCBVERS)/Android.mk
endif
