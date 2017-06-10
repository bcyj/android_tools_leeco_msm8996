JPEG_PATH := $(call my-dir)

# ---------------------------------------------------------------------------------
#      Make the prebuilt library (libmmjpeg-enc-rvct, libmmjpeg-dec-rvct)
# ---------------------------------------------------------------------------------

include $(CLEAR_VARS)

ifeq ($(strip $(USES_ARMV7)),true)
LOCAL_PATH := $(JPEG_PATH)/lib/os/armv7
else
LOCAL_PATH := $(JPEG_PATH)/lib/os/armv6
endif

LOCAL_PREBUILT_LIBS := libmmjpeg-enc-rvct.a
LOCAL_PREBUILT_LIBS += libmmjpeg-dec-rvct.a

include $(BUILD_MULTI_PREBUILT)

# ---------------------------------------------------------------------------------
#                       Common definitons
# ---------------------------------------------------------------------------------

#CPU := -mcpu=arm1136j-s
MM_DEBUG :=false
MM_DECODER :=true

#libmmjpeg_defines := -g -O3 $(CPU)
libmmjpeg_defines := -g -O3
libmmjpeg_defines += $(KERNEL_HEADERS:%=-I%)

ifeq ($(MM_DEBUG),true)
libmmjpeg_defines += -DVERBOSE -DMM_DEBUG
endif

ifeq ($(strip $(USES_GEMINI)),true)
libmmjpeg_defines += -DUSE_GEMINI
endif

ifeq ($(strip $(USES_MERCURY)),true)
libmmjpeg_defines += -DUSE_MERCURY
endif

common_libmmjpeg_cflags := -fno-short-enums
common_libmmjpeg_cflags += $(libmmjpeg_defines)
common_libmmjpeg_cflags += -D_ANDROID_
common_libmmjpeg_cflags += -D_DEBUG

ifeq ($(strip $(USES_ARMV7)),true)
common_libmmjpeg_cflags += -DARM_ARCH_7A
endif

ifeq ($(strip $(SMIPOOL_AVAILABLE)),true)
common_libmmjpeg_cflags += -DSMIPOOL_AVAILABLE
endif

ifeq ($(strip $(TARGET_USES_ION)),true)
common_libmmjpeg_cflags += -DUSE_ION
endif

ifeq ($(strip $(NEW_LOG_API)),true)
common_libmmjpeg_cflags += -DNEW_LOG_API
endif

# ---------------------------------------------------------------------------------
# 			Make the shared library (libmmjpeg)
# ---------------------------------------------------------------------------------

include $(CLEAR_VARS)
LOCAL_PATH := $(JPEG_PATH)

libmmjpeg_includes += $(LOCAL_PATH)/inc
libmmjpeg_includes += $(LOCAL_PATH)/src
libmmjpeg_includes += $(LOCAL_PATH)/src/os

ifeq ($(strip $(USES_GEMINI)),false)
libmmjpeg_includes += $(LOCAL_PATH)/../jpeg_hw_10/jpege_hw/inc
endif

ifeq ($(strip $(USES_MERCURY)),false)
libmmjpeg_includes += $(LOCAL_PATH)/../jpeg_hw_10/jpegd_hw/inc
endif

ifneq ($(strip $(USE_BIONIC_HEADER)),true)
libmmjpeg_includes += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/

LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
else
libmmjpeg_includes+= bionic/libc/kernel/common/media
endif

LOCAL_COPY_HEADERS_TO := mm-still/jpeg
LOCAL_COPY_HEADERS := inc/exif.h
LOCAL_COPY_HEADERS += inc/jpeg_buffer.h
LOCAL_COPY_HEADERS += inc/jpeg_common.h
LOCAL_COPY_HEADERS += inc/jpegd.h
LOCAL_COPY_HEADERS += inc/jpege.h
LOCAL_COPY_HEADERS += inc/jpegerr.h
LOCAL_COPY_HEADERS += inc/jps.h
LOCAL_COPY_HEADERS += inc/jpsd.h
LOCAL_COPY_HEADERS += inc/os_int.h
LOCAL_COPY_HEADERS += inc/os_int_sp.h
LOCAL_COPY_HEADERS += inc/os_types.h
LOCAL_COPY_HEADERS += inc/os_types_sp.h
LOCAL_COPY_HEADERS += inc/os_pmem.h
LOCAL_COPY_HEADERS += inc/os_pmem_sp.h

LOCAL_MODULE           := libmmjpeg
LOCAL_32_BIT_ONLY       := true
LOCAL_MODULE_TAGS      := optional
LOCAL_CFLAGS           := $(common_libmmjpeg_cflags)
LOCAL_C_INCLUDES       := $(libmmjpeg_includes)
LOCAL_C_INCLUDES       += hardware/qcom/camera
LOCAL_C_INCLUDES       += $(TARGET_OUT_HEADERS)/adsprpc/inc
LOCAL_LDFLAGS          += -Wl,--no-fatal-warnings
LOCAL_PRELINK_MODULE   := false
LOCAL_SHARED_LIBRARIES := libcutils
ifeq ($(strip $(USES_GEMINI)),true)
LOCAL_SHARED_LIBRARIES += libgemini
else
LOCAL_SHARED_LIBRARIES += libjpegehw
endif

ifeq ($(strip $(USES_MERCURY)),true)
LOCAL_SHARED_LIBRARIES += libmercury
else
LOCAL_SHARED_LIBRARIES += libjpegdhw
endif

ifeq ($(MM_DEBUG),true)
LOCAL_SHARED_LIBRARIES += liblog libutils
endif

LOCAL_STATIC_LIBRARIES := libmmjpeg-enc-rvct
LOCAL_STATIC_LIBRARIES += libmmjpeg-dec-rvct

LOCAL_SRC_FILES := src/jpege.c
LOCAL_SRC_FILES += src/jpegd.c
LOCAL_SRC_FILES += src/jpege_engine_hybrid.c
LOCAL_SRC_FILES += src/jpege_engine_q6.c
LOCAL_SRC_FILES += src/apr_pmem.c
LOCAL_SRC_FILES += src/jpege_engine_sw.c
LOCAL_SRC_FILES += src/jpege_engine_sw_fetch_dct.c
LOCAL_SRC_FILES += src/jpege_engine_sw_huff.c
LOCAL_SRC_FILES += src/jpege_engine_sw_scale_up.c
LOCAL_SRC_FILES += src/jpege_engine_sw_scale_down.c
LOCAL_SRC_FILES += src/jpege_engine_sw_scale.c
LOCAL_SRC_FILES += src/jpege_engine_bs.c
#LOCAL_SRC_FILES += src/jpege_engine_q5.c
LOCAL_SRC_FILES += src/jpegd_engine_utils.c
LOCAL_SRC_FILES += src/jpegd_engine_hw_utils.c
LOCAL_SRC_FILES += src/jpegd_engine_sw.c
LOCAL_SRC_FILES += src/jpegd_engine_sw_progressive.c
LOCAL_SRC_FILES += src/jpegd_engine_sw_utils.c
LOCAL_SRC_FILES += src/jpegd_engine_sw_idct.c
LOCAL_SRC_FILES += src/jpegd_engine_sw_huff.c
#LOCAL_SRC_FILES += src/jpegd_engine_q5.c
LOCAL_SRC_FILES += src/jpeg_q5_helper_sp.c
LOCAL_SRC_FILES += src/jpeg_writer.c
LOCAL_SRC_FILES += src/jpeg_file_size_control.c
LOCAL_SRC_FILES += src/jpeg_reader.c
LOCAL_SRC_FILES += src/jpeg_buffer.c
LOCAL_SRC_FILES += src/jpeg_header.c
LOCAL_SRC_FILES += src/jpeg_debug.c
LOCAL_SRC_FILES += src/jpeg_postprocessor.c
LOCAL_SRC_FILES += src/jpeg_postprocess_config.c
LOCAL_SRC_FILES += src/jpeg_postprocess_dm.c
LOCAL_SRC_FILES += src/jpeg_postprocess_cc.c
LOCAL_SRC_FILES += src/jpeg_postprocess_ds.c
LOCAL_SRC_FILES += src/jpeg_postprocess_yuv2rgb.c
LOCAL_SRC_FILES += src/jpeg_queue.c
LOCAL_SRC_FILES += src/exif.c
LOCAL_SRC_FILES += src/exif_defaults.c
LOCAL_SRC_FILES += src/os/os_pmem_sp.c
LOCAL_SRC_FILES += src/os/os_thread_sp.c
LOCAL_SRC_FILES += src/os/os_timer_sp.c
LOCAL_SRC_FILES += src/writer_utility.c
LOCAL_SRC_FILES += src/jpegd_englist_sw_only.c
LOCAL_SRC_FILES += src/jpsd.c

# Add platform-specific source files
ifeq ($(strip $(JPEG_ENC)),q5_sw)
LOCAL_SRC_FILES += src/jpege_englist_q5_sw.c
else ifeq ($(strip $(JPEG_ENC)),sw_only)
LOCAL_SRC_FILES += src/jpege_englist_sw_only.c
else ifeq ($(strip $(JPEG_ENC)),hw_sw)
 ifeq ($(strip $(USES_GEMINI)),true)
  LOCAL_SRC_FILES += src/jpege_engine_hw.c
 else
  LOCAL_SRC_FILES += src/jpege_engine_hw_10.c
 endif
  ifeq ($(strip $(USES_MERCURY)),true)
  LOCAL_SRC_FILES += src/jpegd_engine_hw.c
 else
  LOCAL_SRC_FILES += src/jpegd_engine_hw_10.c
 endif
LOCAL_SRC_FILES += src/jpege_englist_hw_sw.c
LOCAL_SRC_FILES += src/jpegd_englist_hw_sw.c
endif

LOCAL_MODULE_OWNER := qcom
LOCAL_32_BIT_ONLY := true
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)

# ---------------------------------------------------------------------------------
# 			Make the tests (mm-jpeg-enc-test)
# ---------------------------------------------------------------------------------

include $(CLEAR_VARS)
LOCAL_PATH := $(JPEG_PATH)

mm-jpeg-inc += $(LOCAL_PATH)/inc
mm-jpeg-inc += $(LOCAL_PATH)/src
mm-jpeg-inc += $(LOCAL_PATH)/src/os
ifneq ($(strip $(USE_BIONIC_HEADER)),true)
mm-jpeg-inc += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/
LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
endif

LOCAL_MODULE		:= mm-jpeg-enc-test
LOCAL_32_BIT_ONLY       := true
LOCAL_MODULE_TAGS       := optional
LOCAL_CFLAGS            := $(common_libmmjpeg_cflags)
LOCAL_C_INCLUDES  	:= $(mm-jpeg-inc)
LOCAL_C_INCLUDES        += hardware/qcom/camera
LOCAL_C_INCLUDES       += $(TARGET_OUT_HEADERS)/adsprpc/inc
LOCAL_PRELINK_MODULE	:= false
LOCAL_SHARED_LIBRARIES	:= libcutils libmmjpeg

ifeq ($(call is-board-platform-in-list, msm8610),true)
LOCAL_SHARED_LIBRARIES += libadsprpc
endif

ifeq ($(strip $(USES_GEMINI)),true)
LOCAL_CFLAGS+= -DGEMINI_HW_ENCODE
endif

ifeq ($(strip $(USES_MERCURY)),true)
LOCAL_CFLAGS+= -DMERCURY_HW_DECODE
endif

ifeq ($(MM_DEBUG),true)
LOCAL_SHARED_LIBRARIES 	+= liblog libutils
endif

LOCAL_SRC_FILES	:= test/encoder_test.c
LOCAL_SRC_FILES += test/ppf_jpeg_header.c

include $(BUILD_EXECUTABLE)

# ---------------------------------------------------------------------------------
ifeq ($(MM_DECODER),true)
# ---------------------------------------------------------------------------------
# 			Make the tests (mm-jpeg-dec-test)
# ---------------------------------------------------------------------------------

include $(CLEAR_VARS)
LOCAL_PATH := $(JPEG_PATH)

mm-jpeg-inc	+= $(LOCAL_PATH)/inc
mm-jpeg-inc	+= $(LOCAL_PATH)/src
mm-jpeg-inc	+= $(LOCAL_PATH)/src/os

ifneq ($(strip $(USE_BIONIC_HEADER)),true)
LOCAL_CFLAGS += -include $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/linux/msm_ion.h

LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
endif

LOCAL_MODULE 		:= mm-jpeg-dec-test
LOCAL_32_BIT_ONLY       := true
LOCAL_MODULE_TAGS       := optional
LOCAL_CFLAGS		:= $(common_libmmjpeg_cflags)
LOCAL_CFLAGS		+= -DSCREEN_DUMP_SUPPORTED
LOCAL_C_INCLUDES	:= $(mm-jpeg-inc)
LOCAL_C_INCLUDES       += hardware/qcom/camera
LOCAL_PRELINK_MODULE	:= false
LOCAL_SHARED_LIBRARIES	:= libcutils libmmjpeg

ifeq ($(MM_DEBUG),true)
LOCAL_SHARED_LIBRARIES 	+= liblog libutils
endif

LOCAL_SRC_FILES	:= test/decoder_test.c

include $(BUILD_EXECUTABLE)
endif
# ---------------------------------------------------------------------------------
#                                      END
# ---------------------------------------------------------------------------------
