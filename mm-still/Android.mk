ifeq ($(call is-vendor-board-platform,QCOM),true)
  STILL_ROOT := $(call my-dir)
  MM_STILL_OMX_COMP := false
  MM_STILL_OMX_FUZZ := false
  MM_STILL_OMX_COMP_LIST := msm7627a
  MM_STILL_OMX_COMP_LIST += msm8660
  MM_STILL_OMX_COMP_LIST += msm8960
  MM_STILL_OMX_COMP_LIST += msm8610
  CODECV1_LIST := msm8974
  CODECV1_LIST += msm8960
  CODECV1_LIST += msm8226
  CODECV1_LIST += msm8916
  CODECV1_LIST += msm8610
  CODECV1_LIST += msm_bronze
  CODECV1_LIST += msm8916
  CODECV1_LIST += msm8909
  ifeq ($(call is-board-platform-in-list,$(MM_STILL_OMX_COMP_LIST)),true)
    MM_STILL_OMX_COMP := true
    #MM_STILL_OMX_FUZZ := true
  endif

  ifeq ($(call is-android-codename-in-list,JELLY_BEAN),true)
    NEW_LOG_API := true
    OMX_HEADER_DIR := frameworks/native/include/media/openmax
    USE_BIONIC_HEADER := false
  else
    NEW_LOG_API := false
    OMX_HEADER_DIR := $(TARGET_OUT_HEADERS)/mm-core/omxcore
    USE_BIONIC_HEADER := false
  endif

  ifeq ($(call is-board-platform-in-list,$(CODECV1_LIST)),true)
    ifeq ($(call is-board-platform-in-list,msm8974 msm8916 msm8226 msm8610 msm_bronze msm8916 msm8909),true)
      JPEGD_VER := jpeg10
      JPEGE_VER := jpeg10
      FACT_VER := codecB
      ifeq ($(call is-board-platform-in-list,msm8974),true)
        MM_STILL_OMX_FUZZ := true
      endif
    else
      JPEGD_VER := none
      JPEGE_VER := gemini
      FACT_VER := codecA
    endif
    include $(STILL_ROOT)/codec_v1/Android.mk
  else ifeq ($(call is-chipset-prefix-in-board-platform,msm7627),true)
    JPEG1_LIB := true
    JPEG_DEC := q5_sw
    JPEG_ENC := q5_sw
    HEAP_ID := ION_CAMERA_HEAP_ID # Only for Makefile compatibility

    include $(STILL_ROOT)/jpeg/Android.mk
    include $(STILL_ROOT)/gemini/Android.mk
  else
    USES_ARMV7 := true
    USES_GEMINI := true
    USES_MERCURY := true
    JPEG_DEC := q5_sw
    JPEG_ENC := hw_sw

    ifeq ($(call is-chipset-prefix-in-board-platform,msm8974 msm8610 msm_bronze msm8916 msm8909),true)
      USES_GEMINI := false
      USES_MERCURY := false
      SMIPOOL_AVAILABLE := false

      include $(STILL_ROOT)/jpeg2/Android.mk
      include $(STILL_ROOT)/jpeg_hw_10/Android.mk
    else ifeq ($(call is-chipset-prefix-in-board-platform,msm8226),true)
      USES_GEMINI := false
      USES_MERCURY := false
      SMIPOOL_AVAILABLE := false

      include $(STILL_ROOT)/jpeg2/Android.mk
      include $(STILL_ROOT)/jpeg_hw_10/Android.mk

    else ifeq ($(call is-chipset-prefix-in-board-platform,msm8916),true)
      USES_GEMINI := false
      USES_MERCURY := false
      SMIPOOL_AVAILABLE := false

      include $(STILL_ROOT)/jpeg2/Android.mk
      include $(STILL_ROOT)/jpeg_hw_10/Android.mk

    else
      ifeq ($(call is-chipset-in-board-platform,msm7630),true)
        SMIPOOL_AVAILABLE := false
        HEAP_ID := ION_CAMERA_HEAP_ID # Only for Makefile compatibility
      else ifeq ($(call is-chipset-prefix-in-board-platform,msm8660),true)
        SMIPOOL_AVAILABLE := true
        HEAP_ID := ION_CAMERA_HEAP_ID
      else ifeq ($(call is-chipset-prefix-in-board-platform,msm8960),true)
        SMIPOOL_AVAILABLE := false
      else # This is for any newer targets after 8960. i.e. copper (8074)
        SMIPOOL_AVAILABLE := false
      endif

      include $(STILL_ROOT)/jpeg2/Android.mk
      include $(STILL_ROOT)/gemini/Android.mk
    endif

    include $(STILL_ROOT)/mpo/Android.mk
    include $(STILL_ROOT)/jps/Android.mk
    include $(STILL_ROOT)/mpod/Android.mk
    include $(STILL_ROOT)/mercury/Android.mk

  endif

  include $(STILL_ROOT)/ipl/Android.mk

  ifeq ($(MM_STILL_OMX_COMP),true)
    include $(STILL_ROOT)/omx/Android.mk
  endif
  ifeq ($(MM_STILL_OMX_FUZZ),true)
    include $(STILL_ROOT)/mmstillomxenc/Android.mk
  endif
endif
