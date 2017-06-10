ifeq ($(call is-board-platform,msm8660),true)

ifneq ($(BUILD_TINY_ANDROID),true)

LOCAL_PATH:= $(call my-dir)
ROOT_DIR := $(LOCAL_PATH)
include $(CLEAR_VARS)

# ---------------------------------------------------------------------------------
#                 Common definitons
# ---------------------------------------------------------------------------------

lib_iec_60958_61937-def := -g -O3
lib_iec_60958_61937-def += -D_ANDROID_

# ---------------------------------------------------------------------------------
#             Make the component library (lib_iec_60958_61937)
# ---------------------------------------------------------------------------------


lib_iec_60958_61937-inc       := $(LOCAL_PATH)/inc
lib_iec_60958_61937-inc       += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include

LOCAL_MODULE            := lib_iec_60958_61937
LOCAL_MODULE_TAGS       := optional
LOCAL_CFLAGS            := $(lib_iec_60958_61937-def)
LOCAL_C_INCLUDES        := $(lib_iec_60958_61937-inc)

LOCAL_ADDITIONAL_DEPENDENCIES  += $(KERNEL_HEADERS_INSTALL)

LOCAL_PRELINK_MODULE    := false
LOCAL_SHARED_LIBRARIES  := libutils liblog

LOCAL_COPY_HEADERS_TO   := mm-audio/audio-iec-60958-61937
LOCAL_COPY_HEADERS      := inc/iec_60958_61937.h


LOCAL_SRC_FILES += src/iec_60958_61937.c

LOCAL_MODULE_OWNER := qcom
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)

endif #BUILD_TINY_ANDROID
endif # is-board-platform
