ifeq ($(call is-board-platform-in-list, msm8960 msm8974 msm8226 msm8610 copper apq8084 msm8994 msm8909),true)
ifneq ($(BUILD_TINY_ANDROID),true)


LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)


# ---------------------------------------------------------------------------------
#               Common definitons
# ---------------------------------------------------------------------------------

mm-alsa-utils-def := -g -O3
mm-alsa-utils-def += -D_ANDROID_
mm-alsa-utils-def += -DQDSP6V2


# ---------------------------------------------------------------------------------
#             Make the alsa-utils (libalsautils)
# ---------------------------------------------------------------------------------

include $(CLEAR_VARS)

mm-alsa-utils-inc     := $(LOCAL_PATH)/inc
mm-alsa-utils-inc     += $(TARGET_OUT_HEADERS)/mm-audio/audio-alsa
mm-alsa-utils-inc     += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
mm-alsa-utils-inc     += $(TARGET_OUT_HEADERS)/mm-audio/libalsa-intf
mm-alsa-utils-inc     += $(TARGET_OUT_HEADERS)/mm-core/omxcore


LOCAL_MODULE            := libalsautils
LOCAL_MODULE_TAGS       := optional
LOCAL_CFLAGS            := $(mm-alsa-utils-def)
LOCAL_C_INCLUDES        := $(mm-alsa-utils-inc)
LOCAL_C_INCLUDES        += $(TARGET_OUT_HEADERS)/common/inc
LOCAL_SHARED_LIBRARIES  := libaudioalsa
LOCAL_SHARED_LIBRARIES  += libalsa-intf
LOCAL_SHARED_LIBRARIES  += libmm-omxcore
LOCAL_LDLIBS            := -lalsa-intf

LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
LOCAL_SRC_FILES         := src/omx_alsa_utils.c
LOCAL_COPY_HEADERS_TO   := mm-audio/omx/alsa-utils/qdsp6/inc/
LOCAL_COPY_HEADERS      := inc/omx_alsa_utils.h
LOCAL_MODULE_OWNER := qcom
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)

endif #BUILD_TINY_ANDROID
endif #TARGET_BOARD_PLATFORM
# ---------------------------------------------------------------------------------
#                     END
# ---------------------------------------------------------------------------------
