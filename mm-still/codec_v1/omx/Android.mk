ifeq ($(TARGET_ARCH),$(filter $(TARGET_ARCH),arm arm64))

LOCAL_PATH := $(call my-dir)
OMX_COMMON := $(LOCAL_PATH)/common
JPEG_PATH := $(LOCAL_PATH)/../../jpeg2

include $(CLEAR_VARS)

omx_jpeg_defines:= -DAMSS_VERSION=$(AMSS_VERSION) \
       -g -O0 \
       -D_ANDROID_ \
        -include QIDbg.h

ifeq ($(PLATFORM_SDK_VERSION), 19)
omx_jpeg_defines += -Werror
endif

omx_jpeg_defines += -DCODEC_V1

ifeq ($(strip $(FACT_VER)),codecB)
omx_jpeg_defines += -DCODEC_B
endif

LOCAL_CFLAGS := $(omx_jpeg_defines)

LOCAL_SRC_FILES := common/QOMX_Buffer.cpp
LOCAL_SRC_FILES += common/qomx_core_component.cpp
LOCAL_SRC_FILES += common/QOMXImageCodec.cpp
LOCAL_SRC_FILES += common/QIMessage.cpp

LOCAL_MODULE := qomx_core_helper
LOCAL_MODULE_TAGS := optional

OMX_HEADER_DIR := hardware/qcom/media/mm-core/inc
OMX_CORE_DIR := hardware/qcom/camera/mm-image-codec

LOCAL_C_INCLUDES := frameworks/native/include/media/openmax
LOCAL_C_INCLUDES += $(OMX_COMMON)
LOCAL_C_INCLUDES += $(OMX_COMMON)/../../common
LOCAL_C_INCLUDES += $(OMX_CORE_DIR)/qomx_core
LOCAL_C_INCLUDES += $(OMX_CORE_DIR)/qexif
LOCAL_C_INCLUDES += $(OMX_HEADER_DIR)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../utils
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../exif
LOCAL_C_INCLUDES += $(JPEG_PATH)/inc
LOCAL_C_INCLUDES += $(JPEG_PATH)/src
LOCAL_C_INCLUDES += $(JPEG_PATH)/src/os
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../decoder
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../encoder
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../dma


ifneq ($(strip $(USE_BIONIC_HEADER)),true)
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
endif

LOCAL_MODULE_OWNER := qcom
LOCAL_PROPRIETARY_MODULE := true

LOCAL_32_BIT_ONLY := true

include $(BUILD_STATIC_LIBRARY)

include $(call all-makefiles-under,$(LOCAL_PATH))
endif
