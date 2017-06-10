OMX_JPEG_PATH := $(call my-dir)

# ------------------------------------------------------------------------------
#                Make the shared library (libqomx_encoder)
# ------------------------------------------------------------------------------

include $(CLEAR_VARS)
LOCAL_PATH := $(OMX_JPEG_PATH)
JPEG_PATH := $(LOCAL_PATH)/../../../jpeg2

LOCAL_MODULE_TAGS := optional

omx_jpeg_defines:=  \
       -DAMSS_VERSION=$(AMSS_VERSION) \
       -g -O0 \
       -D_ANDROID_ \
        -include QIDbg.h

ifneq (,$(findstring $(PLATFORM_VERSION), 4.4 4.4.1 4.4.2 4.4.3 4.4.4))
omx_jpeg_defines += -Werror
endif

omx_jpeg_defines += -DCODEC_V1

ifeq ($(strip $(FACT_VER)),codecB)
omx_jpeg_defines += -DCODEC_B
endif

LOCAL_CFLAGS := $(omx_jpeg_defines)

OMX_HEADER_DIR := frameworks/native/include/media/openmax
OMX_CORE_DIR := hardware/qcom/camera/mm-image-codec

ifeq ($(call is-board-platform-in-list, msm8610),true)
LOCAL_CFLAGS+= -DJPEG_USE_QDSP6_ENCODER
endif

LOCAL_C_INCLUDES := $(OMX_HEADER_DIR)
LOCAL_C_INCLUDES += $(OMX_CORE_DIR)/qomx_core
LOCAL_C_INCLUDES += $(OMX_CORE_DIR)/qexif
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../common
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../utils
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../exif
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../mobicat
LOCAL_C_INCLUDES += $(JPEG_PATH)/inc
LOCAL_C_INCLUDES += $(JPEG_PATH)/src
LOCAL_C_INCLUDES += $(JPEG_PATH)/src/os
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../encoder
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../decoder
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../common
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../qcrypt
LOCAL_C_INCLUDES += external/openssl/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../../adsprpc/inc


LOCAL_C_INCLUDES += vendor/qcom/proprietary/mm-camera/mm-camera2/media-controller/mct/stream
LOCAL_C_INCLUDES += vendor/qcom/proprietary/mm-camera/mm-camera2/media-controller/mct/object
LOCAL_C_INCLUDES += vendor/qcom/proprietary/mm-camera/mm-camera2/media-controller/includes
LOCAL_C_INCLUDES += vendor/qcom/proprietary/mm-camera/mm-camera2/media-controller/mct/tools
LOCAL_C_INCLUDES += vendor/qcom/proprietary/mm-camera/mm-camera2/includes
LOCAL_C_INCLUDES += hardware/qcom/camera/QCamera2/stack/common
LOCAL_C_INCLUDES += vendor/qcom/proprietary/mm-camera/mm-camera2/media-controller/mct/event
LOCAL_C_INCLUDES += vendor/qcom/proprietary/mm-camera/mm-camera2/media-controller/mct/module
LOCAL_C_INCLUDES += vendor/qcom/proprietary/mm-camera/mm-camera2/media-controller/mct/bus
LOCAL_C_INCLUDES += vendor/qcom/proprietary/mm-camera/mm-camera2/media-controller/mct/port
LOCAL_C_INCLUDES += vendor/qcom/proprietary/mm-camera/mm-camera2/server-tuning/tuning

ifneq ($(strip $(USE_BIONIC_HEADER)),true)
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
LOCAL_ADDITIONAL_DEPENDENCIES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
endif

LOCAL_SRC_FILES := OMXImageEncoder.cpp
LOCAL_SRC_FILES += OMXJpegEncoder.cpp
LOCAL_SRC_FILES += ../mobicat/QMobicatComposer.cpp

LOCAL_MODULE           := libqomx_jpegenc
LOCAL_PRELINK_MODULE   := false
LOCAL_SHARED_LIBRARIES := libcutils libdl libmmqjpeg_codec libmmjpeg
LOCAL_WHOLE_STATIC_LIBRARIES := qomx_core_helper
#LOCAL_ADDITIONAL_DEPENDENCIES := libqomx_jpegdec
LOCAL_ADDITIONAL_DEPENDENCIES_32 := libqomx_jpegdec
ifeq ($(call is-board-platform-in-list, msm8610),true)
LOCAL_SHARED_LIBRARIES += libadsprpc
endif

LOCAL_MODULE_OWNER := qcom
LOCAL_32_BIT_ONLY := true
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)
