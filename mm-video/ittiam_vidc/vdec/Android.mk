ifneq ($(BUILD_TINY_ANDROID),true)

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

# ---------------------------------------------------------------------------------
# 				Common definitons
# ---------------------------------------------------------------------------------

libOmxVdec-def += -g -O2
libOmxVdec-def += -DICS
libOmxVdec-def += -DUSE_ION
libOmxVdec-def += -DENABLE_H264
libOmxVdec-def += -DENABLE_MPEG4
#libOmxVdec-def += -DENABLE_MPEG2
#libOmxVdec-def += -DENABLE_HEVC

#ifeq ($(call is-android-codename,JELLY_BEAN),true)
libOmxVdec-def += -DJELLY_BEAN
#endif
# ---------------------------------------------------------------------------------
# 			Make the Shared library (libOmxVdec)
# ---------------------------------------------------------------------------------

libmm-vdec-inc          := $(LOCAL_PATH)/inc

#ifeq ($(call is-android-codename,JELLY_BEAN),true)
libmm-vdec-inc          += $(TOP)/frameworks/av/include/media/stagefright
libmm-vdec-inc          += $(TOP)/frameworks/native/include/media/hardware
#libmm-vdec-inc          += $(TOP)/frameworks/native/include/media/openmax
#else
#libmm-vdec-inc          += $(TOP)/frameworks/base/include/media/stagefright
#endif

ifeq ($(call is-android-codename,ICECREAM_SANDWICH),true)
libmm-vdec-inc          += $(TOP)/hardware/qcom/display/libgralloc/
endif

ifeq ($(TARGET_BOARD_PLATFORM),exynos5)
libmm-vdec-inc          += $(TOP)/hardware/samsung_slsi/exynos5/include
libmm-vdec-inc          += $(LOCAL_PATH)/../../../include/khronos
libOmxVdec-def += -DISExynos
else
libmm-vdec-inc          += $(TARGET_OUT_HEADERS)/mm-core/omxcore
libmm-vdec-inc          += $(TOP)/hardware/qcom/display/libgralloc/
LOCAL_C_INCLUDES        += $(TARGET_OUT_HEADERS)/common/inc
endif


LOCAL_MODULE                    := libOmxIttiamVdec
LOCAL_MODULE_TAGS               := optional
LOCAL_CFLAGS                    := $(libOmxVdec-def)
LOCAL_C_INCLUDES                := $(libmm-vdec-inc)

LOCAL_CFLAGS                    += -include bionic/libc/kernel/arch-arm/asm/posix_types.h
LOCAL_LDFLAGS                   += -Wl,--no-fatal-warnings
LOCAL_C_INCLUDES                += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
LOCAL_ADDITIONAL_DEPENDENCIES   += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

LOCAL_PRELINK_MODULE    := false
LOCAL_SHARED_LIBRARIES  := liblog libutils libbinder libcutils libdl

LOCAL_STATIC_LIBRARIES  := iv_h264_dec_lib
#LOCAL_STATIC_LIBRARIES  += iv_hevc_dec_lib
LOCAL_STATIC_LIBRARIES  += iv_mpeg4_dec_lib
#LOCAL_STATIC_LIBRARIES  += iv_mpeg2_dec_lib
LOCAL_LDLIBS += \
	-lpthread \
	-ldl \
	-lsdl
LOCAL_SRC_FILES         := src/OMX_Ittiam_Vdec_API.c
LOCAL_SRC_FILES         += src/OMX_Ittiam_Vdec_Process.c
ifeq ($(TARGET_BOARD_PLATFORM),msm8960)
LOCAL_SRC_FILES         += src/omx_vdec.cpp
endif
ifeq ($(TARGET_BOARD_PLATFORM),msm8660)
LOCAL_SRC_FILES         += src/omx_vdec.cpp
endif
ifeq ($(TARGET_BOARD_PLATFORM),msm8974)
LOCAL_SRC_FILES         += src/omx_vdec.cpp
endif
ifeq ($(TARGET_BOARD_PLATFORM),msm7627a)
LOCAL_SRC_FILES         += src/omx_vdec.cpp
endif
ifeq ($(TARGET_BOARD_PLATFORM),msm8625)
LOCAL_SRC_FILES         += src/omx_vdec.cpp
endif
ifeq ($(TARGET_BOARD_PLATFORM),msm8610)
LOCAL_SRC_FILES         += src/omx_vdec.cpp
endif
ifeq ($(TARGET_BOARD_PLATFORM),exynos5)
LOCAL_SRC_FILES         += src/omx_vdec_ittiam_wrapper.cpp
endif
LOCAL_SRC_FILES         += src/ithread.c

LOCAL_MODULE_OWNER := qcom
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)

endif #BUILD_TINY_ANDROID

# ---------------------------------------------------------------------------------
#                END
# ---------------------------------------------------------------------------------
