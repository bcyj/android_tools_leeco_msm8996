OMX_CORE_PATH := $(call my-dir)

# ------------------------------------------------------------------------------
#                Make the shared library (libimageomxcore)
# ------------------------------------------------------------------------------

include $(CLEAR_VARS)
LOCAL_PATH := $(OMX_CORE_PATH)
LOCAL_MODULE_TAGS := optional

ifeq ($(strip $(TARGET_USES_ION)),true)
LOCAL_CFLAGS += -DUSE_ION
endif

ifeq ($(strip $(NEW_LOG_API)),true)
LOCAL_CFLAGS += -DNEW_LOG_API
endif

LOCAL_C_INCLUDES := $(OMX_HEADER_DIR)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../inc/
LOCAL_C_INCLUDES += omx_core.h

LOCAL_COPY_HEADERS_TO := mm-still/mm-omx
LOCAL_COPY_HEADERS := ../inc/omx_debug.h
LOCAL_COPY_HEADERS += ../inc/omx_jpeg_common.h
LOCAL_COPY_HEADERS += ../inc/omx_jpeg_ext.h
LOCAL_COPY_HEADERS += omx_core.h

LOCAL_SRC_FILES:= omx_core.c

LOCAL_MODULE           := libmmstillomx
LOCAL_PRELINK_MODULE   := false
LOCAL_SHARED_LIBRARIES := libcutils libdl

LOCAL_MODULE_OWNER := qcom
LOCAL_32_BIT_ONLY := true
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)
