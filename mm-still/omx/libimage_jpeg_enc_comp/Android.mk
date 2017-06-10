OMX_COMPONENT_PATH := $(call my-dir)

# ---------------------------------------------------------------------------------
#                       Common definitons
# ---------------------------------------------------------------------------------

#CPU := -mcpu=arm1136j-s
MM_DEBUG :=false

#libmmjpeg_defines := -g -O3 $(CPU)
libmmomx_enc_defines := -g -O3
libmmomx_enc_defines += $(KERNEL_HEADERS:%=-I%)

ifeq ($(MM_DEBUG),true)
libmmomx_enc_defines += -DVERBOSE -DMM_DEBUG
endif

ifeq ($(JPEG1_LIB),true)
libmmomx_enc_defines += -DUSE_JPEG1_LIB
endif

common_libmmmpo_cflags := -fno-short-enums
common_libmmmpo_cflags += $(libmmomx_enc_defines)
common_libmmmpo_cflags += -D_ANDROID_
common_libmmmpo_cflags += -D_DEBUG


ifeq ($(strip $(TARGET_USES_ION)),true)
common_libmmmpo_cflags += -DUSE_ION
endif

ifeq ($(strip $(NEW_LOG_API)),true)
common_libmmmpo_cflags += -DNEW_LOG_API
endif
# ---------------------------------------------------------------------------
#                Make the shared library (libimage-jpeg-enc-omx-comp)
# ---------------------------------------------------------------------------

include $(CLEAR_VARS)
LOCAL_PATH := $(OMX_COMPONENT_PATH)


LOCAL_C_INCLUDES := $(OMX_HEADER_DIR)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../inc/

ifeq ($(JPEG1_LIB),true)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../jpeg/inc/
else
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../jpeg2/inc/
endif
LOCAL_C_INCLUDES += omx_component.h
LOCAL_C_INCLUDES += hardware/qcom/camera
ifneq ($(strip $(USE_BIONIC_HEADER)),true)
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
LOCAL_CFLAGS += -include $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include/linux/msm_ion.h
endif

LOCAL_COPY_HEADERS_TO := mm-still/mm-enc-omx-comp
LOCAL_COPY_HEADERS := omx_component.h

LOCAL_MODULE           := libimage-jpeg-enc-omx-comp
LOCAL_32_BIT_ONLY       := true
LOCAL_MODULE_TAGS      := optional
LOCAL_CFLAGS           := $(common_libmmmpo_cflags)
LOCAL_PRELINK_MODULE   := false
LOCAL_SHARED_LIBRARIES := libcutils libmmjpeg libimage-omx-common

LOCAL_SRC_FILES:= omx_component.c

ifeq ($(MM_DEBUG),true)
LOCAL_SHARED_LIBRARIES += liblog
endif

LOCAL_MODULE_OWNER := qcom
LOCAL_32_BIT_ONLY := true
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)

