IMX135_CHROMATIX_VIDEO_PATH := $(call my-dir)

# ---------------------------------------------------------------------------
#                   Make the shared library (libchromatix_imx135_default_video)
# ---------------------------------------------------------------------------

include $(CLEAR_VARS)
LOCAL_PATH := $(IMX135_CHROMATIX_VIDEO_PATH)
LOCAL_MODULE_TAGS := optional

LOCAL_CFLAGS:= \
        -DAMSS_VERSION=$(AMSS_VERSION) \
        $(mmcamera_debug_defines) \
        $(mmcamera_debug_cflags) \
        -include camera_defs_i.h

LOCAL_C_INCLUDES := $(LOCAL_PATH)/../../../
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../../../
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../../../include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../../../hardware/sensor
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../../../../common
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../../../../../../../../hardware/qcom/camera
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/mm-still/jpeg
LOCAL_C_INCLUDES += chromatix_imx135_video.h

LOCAL_SRC_FILES:= chromatix_imx135_video.c

LOCAL_MODULE           := libchromatix_imx135_default_video
LOCAL_SHARED_LIBRARIES := libcutils
include $(LOCAL_PATH)/../../../../../../local_additional_dependency.mk

ifeq ($(MM_DEBUG),true)
LOCAL_SHARED_LIBRARIES += liblog
endif

LOCAL_MODULE_OWNER := qcom 
LOCAL_32_BIT_ONLY := true
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)
