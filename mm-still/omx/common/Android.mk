OMX_COMPONENT_PATH := $(call my-dir)
# ---------------------------------------------------------------------------------
#                       Common definitons
# ---------------------------------------------------------------------------------

#CPU := -mcpu=arm1136j-s
MM_DEBUG :=false

#libmmjpeg_defines := -g -O3 $(CPU)
libmmomx_defines := -g -O3
libmmomx_defines += $(KERNEL_HEADERS:%=-I%)

ifeq ($(MM_DEBUG),true)
libmmomx_defines += -DVERBOSE -DMM_DEBUG
endif

common_libmmomx_cflags := -fno-short-enums
common_libmmomx_cflags += $(libmmomx_defines)
common_libmmomx_cflags += -D_ANDROID_
common_libmmomx_cflags += -D_DEBUG


ifeq ($(strip $(TARGET_USES_ION)),true)
common_libmmomx_cflags += -DUSE_ION
endif

# ---------------------------------------------------------------------------
#                Make the shared library (libmt9p012)
# ---------------------------------------------------------------------------

include $(CLEAR_VARS)
LOCAL_PATH := $(OMX_COMPONENT_PATH)
LOCAL_MODULE_TAGS := optional

LOCAL_C_INCLUDES := $(OMX_HEADER_DIR)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../inc/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../jpeg2/src
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../jpeg2/inc/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../jpeg2/src/os
LOCAL_C_INCLUDES += hardware/qcom/camera
ifneq ($(strip $(USE_BIONIC_HEADER)),true)
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
LOCAL_CFLAGS += -include $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/linux/msm_ion.h
endif

LOCAL_MODULE           := libimage-omx-common
LOCAL_32_BIT_ONLY       := true
LOCAL_CFLAGS           := $(common_libmmomx_cflags)
LOCAL_PRELINK_MODULE   := false
LOCAL_SHARED_LIBRARIES := libcutils libmmjpeg

LOCAL_SRC_FILES:= omx_jpeg_common.c

ifeq ($(MM_DEBUG),true)
LOCAL_SHARED_LIBRARIES += liblog
endif

LOCAL_MODULE_OWNER := qcom
LOCAL_32_BIT_ONLY := true
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)
