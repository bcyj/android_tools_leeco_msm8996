OV8825_CHROMATIX_PREVIEW_PATH := $(call my-dir)

# ---------------------------------------------------------------------------
#                      Make the shared library (libchromatix_ov8820_preview)
# ---------------------------------------------------------------------------

include $(CLEAR_VARS)
LOCAL_PATH := $(OV8825_CHROMATIX_PREVIEW_PATH)
LOCAL_MODULE_TAGS := optional

BUILD_OV8825_7853F := false
BUILD_MM_CAMERA2_BOARD_PLATFORM_7853F_LIST := msm8610
ifeq ($(call is-board-platform-in-list,$(BUILD_MM_CAMERA2_BOARD_PLATFORM_7853F_LIST)),true)
  BUILD_OV8825_7853F := true
endif

LOCAL_CFLAGS:= -DAMSS_VERSION=$(AMSS_VERSION) \
        $(mmcamera_debug_defines) \
        $(mmcamera_debug_cflags) \
        -include camera_defs_i.h

LOCAL_C_INCLUDES := $(LOCAL_PATH)/../../../
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../../../module/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../../../../../../../common/
ifeq ($(BUILD_OV8825_7853F), true)
  LOCAL_C_INCLUDES += chromatix_ov8825_7853f_preview.h
  LOCAL_SRC_FILES:= chromatix_ov8825_7853f_preview.c
  LOCAL_MODULE           := libchromatix_ov8825_7853f_preview
else
  LOCAL_C_INCLUDES += chromatix_ov8825_preview.h
  LOCAL_SRC_FILES:= chromatix_ov8825_preview.c
  LOCAL_MODULE           := libchromatix_ov8825_preview
endif


LOCAL_SHARED_LIBRARIES := libcutils
include $(LOCAL_PATH)/../../../../../../../../../local_additional_dependency.mk

ifeq ($(MM_DEBUG),true)
LOCAL_SHARED_LIBRARIES += liblog
endif

LOCAL_MODULE_OWNER := qcom 
LOCAL_32_BIT_ONLY := true
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)
