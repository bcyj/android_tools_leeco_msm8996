ifneq ($(BUILD_TINY_ANDROID),true)

ROOT_DIR := $(call my-dir)
LOCAL_PATH := $(ROOT_DIR)
include $(CLEAR_VARS)

# ------------------------------------------------------------------------------
#         Common definitons
# ------------------------------------------------------------------------------

# ------------------------------------------------------------------------------
#       Make the Shared library (libual)
# ------------------------------------------------------------------------------
include $(CLEAR_VARS)
LOCAL_MODULE            := libual
LOCAL_MODULE_TAGS       := optional
LOCAL_CFLAGS            := -DLOG_NIDEBUG=0 -DLOG_NDDEBUG=0
LOCAL_C_INCLUDES        :=                            \
  $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include  \
  $(LOCAL_PATH)/../ual_util

ifeq ($(call is-board-platform-in-list,msm8974 apq8084 plutonium),true)
LOCAL_C_INCLUDES        +=                            \
        external/tinyalsa/include \
        $(LOCAL_PATH)/../usnd_route

LOCAL_SHARED_LIBRARIES  :=  \
        libtinyalsa \
        libusndroute
endif

ifeq ($(call is-board-platform,msm8960),true)
LOCAL_C_INCLUDES        +=                            \
  $(TARGET_OUT_HEADERS)/mm-audio/libalsa-intf         \
  $(TOP)/hardware/qcom/audio/legacy/alsa_sound
LOCAL_SHARED_LIBRARIES  :=  \
    libalsa-intf
endif

LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

LOCAL_SHARED_LIBRARIES  +=  \
  libcutils                 \
  libutils                  \
  liblog                    \

LOCAL_SRC_FILES         :=  \
      ual.cpp               \
      ual_alsa.cpp

LOCAL_MODULE_OWNER := qcom
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)

endif #BUILD_TINY_ANDROID

