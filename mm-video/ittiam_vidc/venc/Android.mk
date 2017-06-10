ifneq ($(BUILD_TINY_ANDROID),true)

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

# ---------------------------------------------------------------------------------
# 				Common definitons
# ---------------------------------------------------------------------------------

libOmxVenc-def += -g -O2
libOmxVenc-def += -DICS
ifeq ($(call is-android-codename,JELLY_BEAN),true)
libOmxVenc-def += -DJELLY_BEAN
endif
# ---------------------------------------------------------------------------------
# 			Make the Shared library (libOmxVenc)
# ---------------------------------------------------------------------------------

libmm-venc-inc          := $(LOCAL_PATH)/inc
libmm-venc-inc          += $(TARGET_OUT_HEADERS)/mm-core/omxcore
libmm-venc-inc          += $(TOP)/hardware/qcom/display/libgralloc/
libmm-venc-inc          += $(TOP)/hardware/qcom/media/libstagefrighthw
ifeq ($(call is-android-codename,JELLY_BEAN),true)
libmm-venc-inc          += $(TOP)/frameworks/av/include/media/stagefright
libmm-venc-inc          += $(TOP)/frameworks/native/include/media/hardware
else
libmm-venc-inc          += $(TOP)/frameworks/base/include/media/stagefright
libmm-venc-inc          += $(TOP)/frameworks/native/include/media/hardware
endif

LOCAL_MODULE                    := libOmxIttiamVenc
LOCAL_MODULE_TAGS               := optional
LOCAL_CFLAGS                    := $(libOmxVenc-def)
LOCAL_C_INCLUDES                := $(libmm-venc-inc)
ifeq ($(call is-android-codename,JELLY_BEAN),true)
LOCAL_C_INCLUDES                += $(TARGET_OUT_HEADERS)/common/inc
endif

LOCAL_CFLAGS                    += -include bionic/libc/kernel/arch-arm/asm/posix_types.h
LOCAL_LDFLAGS                   += -Wl,--no-fatal-warnings
LOCAL_C_INCLUDES                += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
LOCAL_ADDITIONAL_DEPENDENCIES   += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

LOCAL_SHARED_LIBRARIES  := liblog
LOCAL_SHARED_LIBRARIES  += libutils
LOCAL_SHARED_LIBRARIES  += libbinder
LOCAL_SHARED_LIBRARIES  += libcutils
LOCAL_SHARED_LIBRARIES  += libdl
LOCAL_SHARED_LIBRARIES  += libc
LOCAL_STATIC_LIBRARIES  := iv_h264_enc_lib
LOCAL_LDLIBS            +=  -lpthread
LOCAL_LDLIBS            +=  -ldl
LOCAL_LDLIBS            +=  -lsdl
LOCAL_SRC_FILES         := src/OMX_Ittiam_Venc_API.c
LOCAL_SRC_FILES         += src/OMX_Ittiam_Venc_Process.c
LOCAL_SRC_FILES         += src/omx_venc.cpp
LOCAL_SRC_FILES         += src/ithread.c

LOCAL_MODULE_OWNER := qcom
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)

endif #BUILD_TINY_ANDROID

# ---------------------------------------------------------------------------------
#                END
# ---------------------------------------------------------------------------------
