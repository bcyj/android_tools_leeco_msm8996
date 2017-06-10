#======================================================================
#makefile for libmmcamera2_pproc_modules.so form mm-camera2
#======================================================================
ifeq ($(call is-vendor-board-platform,QCOM),true)
ifeq ($(call is-board-platform-in-list,msm8974 msm8916 msm8960 msm8226 msm7627a msm8660 msm8610 msm8909),true)

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_CFLAGS  := -D_ANDROID_
LOCAL_PPROC_PATH := $(LOCAL_PATH)
LOCAL_MMCAMERA_PATH  := $(LOCAL_PATH)/../../../../mm-camera2

ifeq ($(call is-board-platform-in-list, msm8909),true)
LOCAL_CFLAGS  += -DAF_2X13_FILTER_SUPPORT
endif

LOCAL_C_INCLUDES += $(LOCAL_PATH)

LOCAL_C_INCLUDES += $(LOCAL_PATH)/includes/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/includes/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/server-tuning/tuning/
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
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/pproc-new/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/pproc-new/cpp/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/pproc-new/c2d/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/imglib/modules/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/imglib/modules/cac/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/imglib/modules/module_imglib/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/pproc-new/vpe/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/pproc-new/wnr/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/sensors/chromatix/$(CHROMATIX_VERSION)
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/include/adreno/
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/include/adreno200/
LOCAL_C_INCLUDES += \
 $(LOCAL_PATH)/../../../../../../../../hardware/qcom/camera/QCamera2/stack/common

#LOCAL_CFLAGS  += -Werror

ifeq ($(MSM_VERSION), 8974)
  LOCAL_CFLAGS += -DCAMERA_FEATURE_CAC
else ifeq ($(MSM_VERSION), 8226)
  LOCAL_CFLAGS += -DCAMERA_FEATURE_WNR_SW
endif

ifeq ($(MSM_VERSION), 8610, 8909)
  LOCAL_CFLAGS += -DCAMERA_FEATURE_WNR_SW
endif

ifeq ($(MSM_VERSION), 8909)
  LOCAL_CFLAGS += -DCAMERA_FEATURE_WNR_SW
endif

ifeq ($(MSM_VERSION), 8916)
  LOCAL_CFLAGS += -DPPROC_METADATA_HEADER_VERSION=0x2
else
  LOCAL_CFLAGS += -DPPROC_METADATA_HEADER_VERSION=0x0
endif

LOCAL_SRC_DIR := $(LOCAL_PATH)

LOCAL_SRC_FILES := pproc_module.c
LOCAL_SRC_FILES += pproc_port.c

LOCAL_MODULE           := libmmcamera2_pproc_modules
LOCAL_32_BIT_ONLY := true

LOCAL_SHARED_LIBRARIES := liblog libcutils \
 liboemcamera libdl libmmcamera2_c2d_module libmmcamera2_imglib_modules \
 libmmcamera2_wnr_module

ifneq ($(MSM_VERSION), 8909)
LOCAL_SHARED_LIBRARIES += libmmcamera2_vpe_module
LOCAL_CFLAGS += -DCAMERA_FEATURE_VPE
endif

ifeq ($(call is-board-platform-in-list,msm8974 msm8916 msm8226), true)
LOCAL_SHARED_LIBRARIES += libmmcamera2_cpp_module
LOCAL_CFLAGS += -DCAMERA_FEATURE_CPP
endif

LOCAL_MODULE_TAGS      := optional eng
LOCAL_ADDITIONAL_DEPENDENCIES  := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
LOCAL_MODULE_OWNER := qcom 
LOCAL_32_BIT_ONLY := true
LOCAL_PROPRIETARY_MODULE := true

#temp disable
include $(BUILD_SHARED_LIBRARY)

#************* cpp module start ************#
#LOCAL_PATH := $(LOCAL_DIR_PATH)
include $(LOCAL_PATH)/cpp/Android.mk
#************* cpp module end ************#

#************* c2d module start ************#
LOCAL_PATH := $(LOCAL_PPROC_PATH)
include $(LOCAL_PATH)/c2d/Android.mk
#************* c2d module end ************#

#************* vpe module start ************#
LOCAL_PATH := $(LOCAL_PPROC_PATH)
include $(LOCAL_PATH)/vpe/Android.mk
#************* vpe module end ************#

#************* wnr module start ************#
LOCAL_PATH := $(LOCAL_PPROC_PATH)
include $(LOCAL_PATH)/wnr/Android.mk
#************* wnr module end ************#

#************* cpp firmware start ************#
#LOCAL_PATH:= $(LOCAL_DIR_PATH)
#include $(LOCAL_PATH)/cpp/firmware/Android.mk
#include $(call all-subdir-makefiles)
#************* cpp firmware end ************#

endif # if 8960
endif # is-vendor-board-platform,QCOM
