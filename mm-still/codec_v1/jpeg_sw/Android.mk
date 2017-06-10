LOCAL_PATH := $(call my-dir)
LOCAL_JPEG_PATH := $(LOCAL_PATH)
# ---------------------------------------------------------------------------------
#      Make the prebuilt library (libmmjpeg-enc-rvct, libmmjpeg-dec-rvct)
# ---------------------------------------------------------------------------------

include $(CLEAR_VARS)

MM_DEBUG :=true
MM_DECODER := true
NEW_LOG_API := true
USE_BIONIC_HEADER := false
USES_ARMV7 := true
JPEG_DEC := q5_sw
JPEG_ENC := sw_only
JPEG_PATH := vendor/qcom/proprietary/mm-still/jpeg2

ifeq ($(strip $(USES_ARMV7)),true)
LOCAL_PATH := $(JPEG_PATH)/lib/os/armv7
else
LOCAL_PATH := $(JPEG_PATH)/lib/os/armv6
endif

#LOCAL_PREBUILT_LIBS := libmmjpeg-enc-rvct.a
#LOCAL_PREBUILT_LIBS += libmmjpeg-dec-rvct.a

include $(BUILD_MULTI_PREBUILT)

# ---------------------------------------------------------------------------------
#                       Common definitons
# ---------------------------------------------------------------------------------

#CPU := -mcpu=arm1136j-s

libmmjpeg_defines := -g -O3

common_libmmjpeg_cflags := -fno-short-enums
common_libmmjpeg_cflags += $(libmmjpeg_defines)
common_libmmjpeg_cflags += -D_ANDROID_
common_libmmjpeg_cflags += -D_DEBUG
common_libmmjpeg_cflags += -DCODEC_V1
common_libmmjpeg_cflags += -DNEW_LOG_API

ifeq ($(strip $(USES_ARMV7)),true)
common_libmmjpeg_cflags += -DARM_ARCH_7A
endif

ifeq ($(strip $(TARGET_USES_ION)),true)
common_libmmjpeg_cflags += -DUSE_ION
endif

# ---------------------------------------------------------------------------------
#           Make the shared library (libmmjpeg)
# ---------------------------------------------------------------------------------

include $(CLEAR_VARS)
JPEG_PATH := vendor/qcom/proprietary/mm-still/jpeg2
LOCAL_PATH := $(JPEG_PATH)

libmmjpeg_includes := $(JPEG_PATH)/inc
libmmjpeg_includes += $(JPEG_PATH)/src
libmmjpeg_includes += $(JPEG_PATH)/src/os
libmmjpeg_includes += hardware/qcom/camera/mm-image-codec/qexif
libmmjpeg_includes += $(TARGET_OUT_INTERMEDIATES)/include/fastcv/

libmmjpeg_includes += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/
LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr


LOCAL_COPY_HEADERS_TO := mm-still/jpeg

LOCAL_MODULE           := libmmjpeg
LOCAL_32_BIT_ONLY       := true
LOCAL_MODULE_TAGS      := optional
LOCAL_CFLAGS           := $(common_libmmjpeg_cflags)
LOCAL_C_INCLUDES       := $(libmmjpeg_includes)
LOCAL_C_INCLUDES       += $(TARGET_OUT_HEADERS)/adsprpc/inc
LOCAL_LDFLAGS          := -Wl,--no-fatal-warnings
LOCAL_PRELINK_MODULE   := false
LOCAL_SHARED_LIBRARIES := libcutils libfastcvopt

ifeq ($(strip $(USES_GEMINI)),true)
LOCAL_SHARED_LIBRARIES += libgemini
endif

ifeq ($(strip $(USES_MERCURY)),true)
LOCAL_SHARED_LIBRARIES += libmercury
endif

LOCAL_SHARED_LIBRARIES += liblog libutils
ifeq ($(call is-board-platform-in-list, msm8610),true)
LOCAL_SHARED_LIBRARIES += libadsprpc
endif
#LOCAL_STATIC_LIBRARIES := libmmjpeg-enc-rvct
#LOCAL_STATIC_LIBRARIES += libmmjpeg-dec-rvct

ifeq ($(call is-board-platform-in-list, msm8610),true)
LOCAL_CFLAGS+= -DHYBRID_QDSP6_ENCODER_8x10
endif

#LOCAL_SRC_FILES := src/jpegd.c
LOCAL_SRC_FILES += src/jpege_engine_sw.c
LOCAL_SRC_FILES += src/jpege_engine_sw_huff.c
LOCAL_SRC_FILES += src/jpege_engine_sw_scale_up.c
LOCAL_SRC_FILES += src/jpege_engine_sw_scale_down.c
LOCAL_SRC_FILES += src/jpege_engine_sw_scale.c
LOCAL_SRC_FILES += src/jpege_engine_bs.c
#LOCAL_SRC_FILES += src/jpegd_engine_utils.c
#LOCAL_SRC_FILES += src/jpegd_engine_sw.c
#LOCAL_SRC_FILES += src/jpegd_engine_sw_progressive.c
#LOCAL_SRC_FILES += src/jpegd_engine_sw_utils.c
#LOCAL_SRC_FILES += src/jpegd_engine_sw_idct.c
#LOCAL_SRC_FILES += src/jpegd_engine_sw_huff.c
LOCAL_SRC_FILES += src/jpeg_q5_helper_sp.c
LOCAL_SRC_FILES += src/jpeg_writer.c
LOCAL_SRC_FILES += src/jpeg_file_size_control.c
LOCAL_SRC_FILES += src/jpeg_reader.c
LOCAL_SRC_FILES += src/jpeg_buffer.c
LOCAL_SRC_FILES += src/jpeg_header.c
LOCAL_SRC_FILES += src/jpeg_debug.c
#LOCAL_SRC_FILES += src/jpeg_postprocessor.c
#LOCAL_SRC_FILES += src/jpeg_postprocess_config.c
#LOCAL_SRC_FILES += src/jpeg_postprocess_dm.c
#LOCAL_SRC_FILES += src/jpeg_postprocess_cc.c
#LOCAL_SRC_FILES += src/jpeg_postprocess_ds.c
#LOCAL_SRC_FILES += src/jpeg_postprocess_yuv2rgb.c
LOCAL_SRC_FILES += src/jpeg_queue.c
LOCAL_SRC_FILES += src/exif.c
LOCAL_SRC_FILES += src/exif_defaults.c
LOCAL_SRC_FILES += src/os/os_pmem_sp.c
LOCAL_SRC_FILES += src/os/os_thread_sp.c
LOCAL_SRC_FILES += src/os/os_timer_sp.c
LOCAL_SRC_FILES += src/writer_utility.c
#LOCAL_SRC_FILES += src/jpegd_englist_sw_only.c
LOCAL_SRC_FILES += src/jpsd.c
LOCAL_SRC_FILES += src/jpege.c
LOCAL_SRC_FILES += src/jpege_engine_hybrid.c
LOCAL_SRC_FILES += src/jpege_englist_sw_only.c
ifeq ($(call is-board-platform-in-list, msm8610),true)
LOCAL_SRC_FILES += src/adsp_jpege_stub.c
LOCAL_SRC_FILES += src/jpege_engine_q6.c
LOCAL_SRC_FILES += src/apr_pmem.c
endif
ifeq ($(LOCAL_32_BIT_ONLY), true)
LOCAL_SRC_FILES += src/asm/armv7/jpege_engine_sw_dct_fetch_dct_armv7_gcc.S
LOCAL_SRC_FILES += src/asm/armv7/jpege_engine_sw_huff_bs_arm_gcc.S
LOCAL_SRC_FILES += src/asm/armv7/jpege_engine_sw_quant_zigzag_arm_gcc.S
else
LOCAL_SRC_FILES += src/jpege_engine_sw_fetch_dct.c
LOCAL_SRC_FILES += src/jpege_engine_sw_dct.c
LOCAL_SRC_FILES += src/jpege_engine_sw_quant_zigzag.c
LOCAL_SRC_FILES += src/jpege_engine_sw_huff_bs.c
endif

LOCAL_MODULE_OWNER := qcom
LOCAL_32_BIT_ONLY := true
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)

# ---------------------------------------------------------------------------------
#       Make the tests (mm-jpeg-enc-test)
# ---------------------------------------------------------------------------------

ifeq ($(call is-board-platform-in-list, msm8610),true)
include $(CLEAR_VARS)
LOCAL_PATH := $(JPEG_PATH)

mm-jpeg-inc := $(JPEG_PATH)/inc
mm-jpeg-inc += $(JPEG_PATH)/src
mm-jpeg-inc += $(JPEG_PATH)/src/os
ifneq ($(strip $(USE_BIONIC_HEADER)),true)
mm-jpeg-inc += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/
LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
endif

LOCAL_MODULE        := mm-jpeg-enc-test
LOCAL_MODULE_TAGS   := optional
LOCAL_CFLAGS        := $(common_libmmjpeg_cflags)
LOCAL_C_INCLUDES    := $(mm-jpeg-inc)
LOCAL_C_INCLUDES    += hardware/qcom/camera/mm-image-codec/qexif

LOCAL_PRELINK_MODULE    := false
LOCAL_SHARED_LIBRARIES  := libcutils libmmjpeg

ifeq ($(strip $(USES_GEMINI)),true)
LOCAL_CFLAGS+= -DGEMINI_HW_ENCODE
endif

ifeq ($(strip $(USES_MERCURY)),true)
LOCAL_CFLAGS+= -DMERCURY_HW_DECODE
endif

ifeq ($(MM_DEBUG),true)
LOCAL_SHARED_LIBRARIES  += liblog libutils
endif

LOCAL_SRC_FILES := test/encoder_test.c
LOCAL_SRC_FILES += test/ppf_jpeg_header.c

include $(BUILD_EXECUTABLE)
endif

# ---------------------------------------------------------------------------------
ifeq ($(MM_DECODER),true)
# ---------------------------------------------------------------------------------
#           Make the tests (mm-jpeg-dec-test)
# ---------------------------------------------------------------------------------

include $(CLEAR_VARS)
LOCAL_PATH := $(JPEG_PATH)

mm-jpeg-inc := $(JPEG_PATH)/inc
mm-jpeg-inc += $(JPEG_PATH)/src
mm-jpeg-inc += $(JPEG_PATH)/src/os

LOCAL_MODULE            := mm-jpeg-dec-test
LOCAL_32_BIT_ONLY       := true
LOCAL_MODULE_TAGS       := optional
LOCAL_CFLAGS            := $(common_libmmjpeg_cflags)
LOCAL_CFLAGS            += -DSCREEN_DUMP_SUPPORTED
LOCAL_C_INCLUDES        := $(mm-jpeg-inc)
LOCAL_C_INCLUDES        += hardware/qcom/camera/mm-image-codec/qexif
LOCAL_PRELINK_MODULE    := false
LOCAL_SHARED_LIBRARIES  := libcutils libmmjpeg

ifneq ($(strip $(USE_BIONIC_HEADER)),true)
LOCAL_C_INCLUDES              += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
LOCAL_ADDITIONAL_DEPENDENCIES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
endif

ifeq ($(MM_DEBUG),true)
LOCAL_SHARED_LIBRARIES  += liblog libutils
endif

#LOCAL_SRC_FILES := test/decoder_test.c

#include $(BUILD_EXECUTABLE)
endif
# ---------------------------------------------------------------------------------
#                                      END
# ---------------------------------------------------------------------------------

