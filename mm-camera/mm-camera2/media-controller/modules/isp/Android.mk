#======================================================================
#makefile for libmmcamera2_isp_modules.so form mm-camera2
#======================================================================
ifeq ($(call is-vendor-board-platform,QCOM),true)
ifeq ($(call is-board-platform-in-list,msm8974 msm8916 msm8960 msm7627a msm8660 msm8226 msm8610 msm8909),true)


LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_CFLAGS  := -D_ANDROID_

#LOCAL_CFLAGS += -Werror


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


ifeq ($(call is-board-platform-in-list, msm8909),true)
LOCAL_CFLAGS  += -DAF_2X13_FILTER_SUPPORT
endif

LOCAL_MMCAMERA_PATH  := $(LOCAL_PATH)/../../../../mm-camera2

LOCAL_C_INCLUDES += $(LOCAL_PATH)
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/includes/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/includes/


LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/mct/includes/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/mct/tools/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/mct/bus/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/mct/controller/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/mct/event/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/mct/module/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/mct/object/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/mct/pipeline/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/mct/port/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/mct/stream/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/mct/debug/

LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/includes/

LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/sensors/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/sensors/includes/

LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/sensors/chromatix/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/sensors/chromatix/$(CHROMATIX_VERSION)

LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/stats/q3a/

LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/isp/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/isp/algo/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/isp/algo/interfaces/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/includes/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/isp/core/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/isp/hw/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/isp/hw/modules/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/isp/hw/axi/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules//isp/hw/pix/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/isp/hw/pix/pix40/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/isp/hw/pix/modules/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/server-tuning/tuning/

LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
LOCAL_C_INCLUDES += \
 $(LOCAL_PATH)/../../../../../../../../hardware/qcom/camera/QCamera2/stack/common

#LOCAL_CFLAGS  += -Werror

LOCAL_SRC_DIR := $(LOCAL_PATH)
LOCAL_SRC_FILES += $(shell find $(LOCAL_SRC_DIR) -name '*.c' | sed s:^$(LOCAL_PATH)::g )

LOCAL_MODULE           := libmmcamera2_isp_modules
LOCAL_32_BIT_ONLY := true
LOCAL_SHARED_LIBRARIES := liblog libcutils liboemcamera libdl
LOCAL_MODULE_TAGS      := optional eng
LOCAL_ADDITIONAL_DEPENDENCIES  := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

LOCAL_MODULE_OWNER := qcom 
LOCAL_32_BIT_ONLY := true
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)

endif # if 8960
endif # is-vendor-board-platform,QCOM
