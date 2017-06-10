COMPILE_CAMERA := true

#default BUILD_CAM_FD to 0 (off)
BUILD_CAM_FD := 0

#define BUILD_SERVER
BUILD_SERVER := false
BUILD_SERVER_BOARD_PLATFORM_LIST := msm7627a
#BUILD_SERVER_BOARD_PLATFORM_LIST += msm7630_surf
#BUILD_SERVER_BOARD_PLATFORM_LIST += msm7630_fusion
BUILD_SERVER_BOARD_PLATFORM_LIST += msm8660
#BUILD_SERVER_BOARD_PLATFORM_LIST += msm8960
#BUILD_SERVER_BOARD_PLATFORM_LIST += msm8974
#BUILD_SERVER_BOARD_PLATFORM_LIST += msm8226
#BUILD_SERVER_BOARD_PLATFORM_LIST += msm8610
ifeq ($(call is-board-platform-in-list,$(BUILD_SERVER_BOARD_PLATFORM_LIST)),true)
  BUILD_SERVER := true
  USE_SERVER_TREE := -D_V4L2_BASED_CAM_
endif

BUILD_MM_CAMERA2 := false
BUILD_MM_CAMERA2_BOARD_PLATFORM_LIST := msm8960
BUILD_MM_CAMERA2_BOARD_PLATFORM_LIST += msm8974
BUILD_MM_CAMERA2_BOARD_PLATFORM_LIST += msm8226
BUILD_MM_CAMERA2_BOARD_PLATFORM_LIST += msm8610
BUILD_MM_CAMERA2_BOARD_PLATFORM_LIST += msm8916
BUILD_MM_CAMERA2_BOARD_PLATFORM_LIST += msm8909
ifeq ($(call is-board-platform-in-list,$(BUILD_MM_CAMERA2_BOARD_PLATFORM_LIST)),true)
  BUILD_MM_CAMERA2 := true
endif


mmcamera_debug_defines := -D_ANDROID_

ifeq ($(call is-android-codename-in-list,JELLY_BEAN),true)
  FEATURE_GYRO := false
else
  FEATURE_GYRO := true
endif
ifeq ($(call is-board-platform-in-list, msm8909),true)
mmcamera_debug_defines  += -DAF_2X13_FILTER_SUPPORT
endif

ifeq ($(call is-board-platform,msm7627a),true)
  VFE_VERS := vfe2x
  MSM_VERSION := 7x27A
  FEATURE_WAVELET_DENOISE := true
  TARGET_NEON_ENABLED := true
  mmcamera_debug_defines += -DHW_ENCODE
  mmcamera_debug_defines += -DUSE_GEMINI
  mmcamera_debug_defines += -DUSE_PREVIEW_TABLE2
  mmcamera_debug_defines += -DUSE_HFR_TABLE2
  ifeq ($(BUILD_SERVER), true)
    FEATURE_FACE_PROC := true
    FEATURE_VFE_TEST_VEC := false
  endif
else ifeq ($(call is-chipset-prefix-in-board-platform,msm7627),true)
  VFE_VERS := vfe2x
  MSM_VERSION := 7x2x
  mmcamera_debug_defines += -DHW_ENCODE
  mmcamera_debug_defines += -DUSE_GEMINI
  mmcamera_debug_defines += -DUSE_PREVIEW_TABLE2
else ifeq ($(call is-chipset-in-board-platform,msm7630),true)
  VFE_VERS := vfe31
  MSM_VERSION := 7x30
  mmcamera_debug_defines += -DHW_ENCODE
  mmcamera_debug_defines += -DUSE_GEMINI
  ifeq ($(BUILD_SERVER), true)
    mmcamera_debug_defines += -DCONFIG_MSG_THESHOLD=350
    FEATURE_ZSL := true
    FEATURE_WAVELET_DENOISE := true
    TARGET_NEON_ENABLED := true
    mmcamera_debug_defines += -DFEATURE_ZSL_SUPPORTED
    FEATURE_FACE_PROC := true
  endif
else ifeq ($(call is-board-platform,msm8660),true)
  VFE_VERS := vfe31
  MSM_VERSION := 8x60
  mmcamera_debug_defines += -DHW_ENCODE
  mmcamera_debug_defines += -DUSE_GEMINI
  mmcamera_debug_defines += -DCONFIG_MSG_THESHOLD=350
  FEATURE_ZSL := true
  FEATURE_WAVELET_DENOISE := true
  TARGET_NEON_ENABLED := true
  mmcamera_debug_defines += -DFEATURE_ZSL_SUPPORTED
  FEATURE_FACE_PROC := true
  FEATURE_VFE_TEST_VEC := false
else ifeq ($(call is-board-platform,msm8960),true)
  VFE_VERS := vfe32
  MSM_VERSION := 8960
  mmcamera_debug_defines += -DHW_ENCODE
  mmcamera_debug_defines += -DCONFIG_MSG_THESHOLD=350
  mmcamera_debug_defines += -DUSE_GEMINI
  FEATURE_WAVELET_DENOISE := true
  TARGET_NEON_ENABLED := true
  FEATURE_ZSL := true
  FEATURE_FACE_PROC := true
  FEATURE_VFE_TEST_VEC := false
else ifeq ($(call is-board-platform,msm8974),true)
  VFE_VERS := vfe40
  MSM_VERSION := 8974
  mmcamera_debug_defines += -DHW_ENCODE
  mmcamera_debug_defines += -DCONFIG_MSG_THESHOLD=350
  #FEATURE_WAVELET_DENOISE := true
  TARGET_NEON_ENABLED := false
  FEATURE_ZSL := true
  FEATURE_FACE_PROC := true
  FEATURE_VFE_TEST_VEC := false
else ifeq ($(call is-board-platform,msm8226),true)
  VFE_VERS := vfe40
  MSM_VERSION := 8226
  mmcamera_debug_defines += -DHW_ENCODE
  mmcamera_debug_defines += -DCONFIG_MSG_THESHOLD=350
  #FEATURE_WAVELET_DENOISE := true
  TARGET_NEON_ENABLED := false
  FEATURE_ZSL := true
  FEATURE_FACE_PROC := true
  FEATURE_VFE_TEST_VEC := false
else ifeq ($(call is-board-platform,msm8916),true)
  VFE_VERS := vfe40
  MSM_VERSION := 8916
  mmcamera_debug_defines += -DHW_ENCODE
  mmcamera_debug_defines += -DCONFIG_MSG_THESHOLD=350
  #FEATURE_WAVELET_DENOISE := true
  TARGET_NEON_ENABLED := false
  FEATURE_ZSL := true
  FEATURE_FACE_PROC := true
  FEATURE_VFE_TEST_VEC := false
else ifeq ($(call is-board-platform,msm8610),true)
  VFE_VERS := vfe32
  MSM_VERSION := 8610
  mmcamera_debug_defines += -DHW_ENCODE
  mmcamera_debug_defines += -DCONFIG_MSG_THESHOLD=350
  #FEATURE_WAVELET_DENOISE := true
  TARGET_NEON_ENABLED := false
  FEATURE_ZSL := true
  FEATURE_FACE_PROC := true
  FEATURE_VFE_TEST_VEC := false
else ifeq ($(call is-board-platform,msm8909),true)
  VFE_VERS := vfe32
  MSM_VERSION := 8909
  mmcamera_debug_defines += -DHW_ENCODE
  mmcamera_debug_defines += -DCONFIG_MSG_THESHOLD=350
  #FEATURE_WAVELET_DENOISE := true
  TARGET_NEON_ENABLED := false
  FEATURE_ZSL := true
  FEATURE_FACE_PROC := true
  FEATURE_VFE_TEST_VEC := false
else
  COMPILE_CAMERA := false
endif

ifeq ($(BUILD_SERVER), true)
  CHROMATIX_VERSION := 0208
else ifeq ($(BUILD_MM_CAMERA2), true)
  CHROMATIX_VERSION := 0301
else
  ifeq ($(MSM_VERSION),7x27A)
    CHROMATIX_VERSION := 0207
  else
    CHROMATIX_VERSION := 0205
  endif
endif

#MM_DEBUG:=true
ifeq ($(call is-android-codename-in-list,JELLY_BEAN),true)
  mmcamera_debug_defines += -DLOGE=ALOGE
endif

ifeq ($(MM_DEBUG),true)
  mmcamera_debug_defines += -DLOG_DEBUG -DLOG_TAG=\"CameraService\"
  mmcamera_debug_cflags += -g -O0 
  mmcamera_debug_libs := liblog libutils
endif #MM_DEBUG

# select different camera code tree for newer targets
MY_PATH := $(call my-dir)

ifeq ($(strip $(COMPILE_CAMERA)),true)

  ifeq ($(BUILD_SERVER), true)
    include $(MY_PATH)/apps/Android.mk
    include $(MY_PATH)/server/Android.mk
  else ifeq ($(BUILD_MM_CAMERA2), true)
    include $(MY_PATH)/mm-camera2/Android.mk
  else
    include $(MY_PATH)/apps/Android.mk
    include $(MY_PATH)/targets/Android.mk
  endif
endif #COMPILE_CAMERA
