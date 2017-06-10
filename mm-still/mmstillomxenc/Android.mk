OMX_FUZZ_PATH := $(call my-dir)

# -------------------------------------------------------
#                      Make the shared library (mmstillomxenc)
# -------------------------------------------------------
#include $(CLEAR_VARS)
#LOCAL_PREBUILT_LIBS := libfuzzlog.so
#LOCAL_MODULE_TAGS := eng
#include $(BUILD_MULTI_PREBUILT)

include $(CLEAR_VARS)
LOCAL_PATH := $(OMX_FUZZ_PATH)
LOCAL_MODULE_TAGS := eng

LOCAL_CFLAGS := -g -O3
LOCAL_CFLAGS += $(KERNEL_HEADERS:%=-I%)

LOCAL_CFLAGS += -fno-short-enums
LOCAL_CFLAGS += -D_ANDROID_
LOCAL_CFLAGS += -D_DEBUG

ifeq ($(strip $(USES_ARMV7)),true)
LOCAL_CFLAGS += -DARM_ARCH_7A
endif

OMX_HEADER_DIR := frameworks/native/include/media/openmax
OMX_CORE_DIR := hardware/qcom/camera/mm-image-codec

LOCAL_C_INCLUDES := $(TARGET_OUT_HEADERS)/mm-core/omxcore
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../omx/inc/
LOCAL_C_INCLUDES += $(LOCAL_PATH)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../jpeg2/inc
LOCAL_C_INCLUDES += hardware/qcom/camera
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include

LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

#LOCAL_CFLAGS     += -include $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/linux/ion.h

LOCAL_CFLAGS     += -DANDROID_BUILD
#LOCAL_CFLAGS     += -fexceptions
#LOCAL_CFLAGS     += -error-unresolved-symbols
#LOCAL_CFLAGS    += $(common_libmmmpo_cflags)
LOCAL_CFLAGS     += $(common_libmmjpeg_cflags)
LOCAL_CFLAGS     += $(libmmomx_enc_test_defines)

ifeq ($(call is-board-platform-in-list,msm8974),true)
  LOCAL_C_INCLUDES += $(OMX_HEADER_DIR)
  LOCAL_C_INCLUDES += $(OMX_CORE_DIR)/qomx_core
  LOCAL_C_INCLUDES += $(OMX_CORE_DIR)/qexif

  LOCAL_CFLAGS += -DOMX_CODEC_V1_WRAPPER
  LOCAL_SRC_FILES  := mmstill_jpeg_omx_enc_codec_v1.cpp
else
  LOCAL_SRC_FILES  := mmstill_jpeg_omx_enc.cpp
endif

LOCAL_SRC_FILES  += mmstillomxenc.cpp
LOCAL_SRC_FILES  += mmstillomxencHelper.cpp
LOCAL_SRC_FILES  += FuzzerUtil.cpp

LOCAL_MODULE     := libFuzzmmstillomxenc

# Shared Libraries
LOCAL_SHARED_LIBRARIES := liblog libmmjpeg libcutils libqomx_jpegenc

ifeq ($(call is-board-platform-in-list,msm8974),true)
  LOCAL_SHARED_LIBRARIES += libqomx_core
else
  LOCAL_SHARED_LIBRARIES += libmmstillomx
endif


LOCAL_PRELINK_MODULE   := false

LOCAL_MODULE_OWNER := qcom
LOCAL_32_BIT_ONLY := true
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)


