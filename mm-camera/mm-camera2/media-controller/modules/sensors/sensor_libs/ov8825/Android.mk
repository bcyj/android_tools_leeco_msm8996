OV8825_SENSOR_LIBS_PATH := $(call my-dir)

# ---------------------------------------------------------------------------
#                      Make the shared library allen@20130123 added for test
# ---------------------------------------------------------------------------

include $(CLEAR_VARS)
LOCAL_PATH := $(OV8825_SENSOR_LIBS_PATH)
LOCAL_MODULE_TAGS := optional

LOCAL_CFLAGS:= \
        -DAMSS_VERSION=$(AMSS_VERSION) \
        $(mmcamera_debug_defines) \
        $(mmcamera_debug_cflags)

ifeq ($(VFE_VERS),vfe40)
  LOCAL_CFLAGS += -DVFE_40
else ifeq ($(VFE_VERS),vfe32)
  LOCAL_CFLAGS += -DVFE_32
  ifeq ($(FEATURE_GYRO), true)
    LOCAL_CFLAGS += -DFEATURE_GYRO
  endif
else ifeq ($(VFE_VERS),vfe31)
  LOCAL_CFLAGS += -DVFE_31
  ifeq ($(MSM_VERSION), 7x30)
    LOCAL_CFLAGS += -DVFE_31_7x30
  else
    LOCAL_CFLAGS += -DVFE_31_8x60
  endif
else ifeq ($(VFE_VERS),vfe2x)
  LOCAL_CFLAGS += -DVFE_2X
endif

ifeq ($(MSM_VERSION),8916)
LOCAL_CFLAGS += -D_MSM_BEAR
endif

LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../includes
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

BUILD_OV8825_7853F := false
BUILD_MM_CAMERA2_BOARD_PLATFORM_7853F_LIST := msm8610
ifeq ($(call is-board-platform-in-list,$(BUILD_MM_CAMERA2_BOARD_PLATFORM_7853F_LIST)),true)
  BUILD_OV8825_7853F := true
endif

ifeq ($(BUILD_OV8825_7853F), true)
  LOCAL_SRC_FILES:= ov8825_7853f_lib.c
else
  LOCAL_SRC_FILES:= ov8825_lib.c
endif

LOCAL_MODULE           := libmmcamera_ov8825
LOCAL_SHARED_LIBRARIES := libcutils
#include $(LOCAL_PATH)/../../../../local_additional_dependency.mk

ifeq ($(MM_DEBUG),true)
LOCAL_SHARED_LIBRARIES += liblog
endif

LOCAL_MODULE_OWNER := qcom 
LOCAL_32_BIT_ONLY := true
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)
