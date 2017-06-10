ifeq ($(call is-board-platform-in-list, msm8960 apq8084),true)

ifneq ($(BUILD_TINY_ANDROID),true)

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

# ---------------------------------------------------------------------------------
#                 Common definitons
# ---------------------------------------------------------------------------------

csd-client-def := -g -O3
csd-client-def += -D_ANDROID_
# csd-client-def += -D_ENABLE_QC_MSG_LOG_
# ---------------------------------------------------------------------------------
#             Make the Shared library (libcsd-client)
# ---------------------------------------------------------------------------------

csd-client-inc          := $(LOCAL_PATH)/inc

LOCAL_MODULE            := libcsd-client
LOCAL_MODULE_TAGS       := optional
LOCAL_CFLAGS            := $(csd-client-def)
LOCAL_C_INCLUDES        := $(csd-client-inc)
LOCAL_C_INCLUDES        += $(TARGET_OUT_HEADERS)/qmi-framework/inc
LOCAL_C_INCLUDES        += $(TARGET_OUT_HEADERS)/qmi/inc
LOCAL_C_INCLUDES        += $(TARGET_OUT_HEADERS)/mm-audio/audcal
LOCAL_C_INCLUDES        += $(TARGET_OUT_HEADERS)/mm-audio/audio-acdb-util
LOCAL_C_INCLUDES        += $(TARGET_OUT_HEADERS)/common/inc
LOCAL_C_INCLUDES        += $(TARGET_OUT_HEADERS)/libmdmdetect/inc
LOCAL_C_INCLUDES        += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
LOCAL_ADDITIONAL_DEPENDENCIES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

LOCAL_SHARED_LIBRARIES  := libutils libqmi_cci libqmi_common_so libacdbloader libcutils libmdmdetect
LOCAL_COPY_HEADERS_TO   := mm-audio/audio-qmi/csd-client
LOCAL_COPY_HEADERS      := inc/csd_client.h

LOCAL_SRC_FILES         := src/csd_client.c \
                           src/core_sound_driver_v01.c \

LOCAL_MODULE_OWNER := qcom
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)

endif # BUILD_TINY_ANDROID
endif # is-board-platform-in-list

# ---------------------------------------------------------------------------------
#                     END
# ---------------------------------------------------------------------------------
