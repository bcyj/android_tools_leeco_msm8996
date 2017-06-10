
ifneq ($(BUILD_TINY_ANDROID),true)

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

# ---------------------------------------------------------------------------------
#                 Common definitons
# ---------------------------------------------------------------------------------

voice-svc-def := -g -O3
voice-svc-def += -D_ANDROID_
# ---------------------------------------------------------------------------------
#             Make the Shared library (libvoice-svc)
# ---------------------------------------------------------------------------------

voice-svc-inc          := $(LOCAL_PATH)/inc

LOCAL_MODULE            := libvoice-svc
LOCAL_MODULE_TAGS       := optional
LOCAL_CFLAGS            := $(voice-svc-def)
LOCAL_C_INCLUDES        := $(voice-svc-inc)
LOCAL_C_INCLUDES        += $(TARGET_OUT_HEADERS)/common/inc
LOCAL_C_INCLUDES        += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

LOCAL_SHARED_LIBRARIES  := libutils libcutils
LOCAL_COPY_HEADERS_TO   := mm-audio/voice-svc
LOCAL_COPY_HEADERS      := inc/voice_svc_client.h

LOCAL_SRC_FILES         := src/voice_svc_client.c \

LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)

endif # BUILD_TINY_ANDROID

# ---------------------------------------------------------------------------------
#                     END
# ---------------------------------------------------------------------------------
