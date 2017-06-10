#======================================================================
#makefile for libmmcamera2_pproc_modules.so form mm-camera2
#======================================================================
ifeq ($(call is-vendor-board-platform,QCOM),true)
ifeq ($(call is-board-platform-in-list,msm8974 msm8916 msm8960 msm7627a msm8660 msm8226 msm8610),true)


LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_CFLAGS  := -D_ANDROID_
LOCAL_MMCAMERA_PATH  := $(LOCAL_PATH)/../../../../../mm-camera2

LOCAL_C_INCLUDES += $(LOCAL_PATH)

LOCAL_C_INCLUDES += $(LOCAL_PATH)/includes/
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
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/includes/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/pproc/includes/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/pproc/cpp/
LOCAL_C_INCLUDES += $(LOCAL_MMCAMERA_PATH)/media-controller/modules/sensors/chromatix/$(CHROMATIX_VERSION)

LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
LOCAL_C_INCLUDES += \
 $(LOCAL_PATH)/../../../../../../../../../hardware/qcom/camera/QCamera2/stack/common

#LOCAL_CFLAGS  += -Werror

LOCAL_SRC_DIR := $(LOCAL_PATH)
LOCAL_SRC_FILES += $(shell find $(LOCAL_SRC_DIR) -name '*.c' | sed s:^$(LOCAL_PATH)::g )

LOCAL_MODULE           := libmmcamera2_pproc_modules
LOCAL_32_BIT_ONLY := true

LOCAL_SHARED_LIBRARIES := liblog libcutils libmmcamera_cpp liboemcamera libdl
LOCAL_MODULE_TAGS      := optional eng
LOCAL_ADDITIONAL_DEPENDENCIES  := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

LOCAL_MODULE_OWNER := qcom 
LOCAL_32_BIT_ONLY := true
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)

#************* cpp libs start ************#
LOCAL_PATH := $(LOCAL_DIR_PATH)
include $(LOCAL_PATH)/modules/pproc/cpp/Android.mk
#************* cpp libs end ************#

#************* cpp firmware start ************#
LOCAL_PATH:= $(LOCAL_DIR_PATH)
include $(LOCAL_PATH)/modules/pproc/cpp/firmware/Android.mk
include $(call all-subdir-makefiles)
#************* cpp firmware start end ************#

endif # if 8960
endif # is-vendor-board-platform,QCOM
